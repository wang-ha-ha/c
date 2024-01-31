//
// Created by wxl on 24-1-14.
//

#include "crash_catcher_collect_info.h"
#include "crash_catcher_type.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <syscall.h>

pid_t gettid() {
    return (pid_t) syscall(SYS_gettid);
}

static const uint32_t lineLen = 1024;
static char line[1024];
const char* simple_line(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, lineLen, fmt, ap);
    va_end(ap);
    return line;
}

static bool is_digit(const char* s) {
    bool res = true;
    if (s) {
        const uint32_t len = strlen(s);
        for (int i = 0; i < len; i++) {
            if (s[i] >= '0' && s[i] <= '9') continue;
            else {
                res = false;
                break;
            }
        }
    } else {
        res = false;
    }
    return res;
}

static std::string get_free_dir() {
    std::string res;
    if (access(CRASH_CATCHER_DIR, F_OK)) {
        mkdir(CRASH_CATCHER_DIR, 0666);
    }

    DIR* p = opendir(CRASH_CATCHER_DIR);
    std::vector<uint32_t> indexList;
    indexList.reserve(10);
    if (p) {
        dirent* dir = nullptr;
        while ((dir = readdir(p)) != nullptr) {
            if (is_digit(dir->d_name)) {
                uint32_t index;
                sscanf(dir->d_name, "%u", &index);
                indexList.push_back(index);
            }
        }
        closedir(p);
    }

    uint32_t latestIndex = 0;
    if (!indexList.empty()) {
        std::sort(indexList.begin(), indexList.end());
        latestIndex = indexList.back() + 1;
    }

    res = simple_line("%s/%d", CRASH_CATCHER_DIR, latestIndex);
    mkdir(res.c_str(), 0755);

    indexList.push_back(latestIndex);
    if (indexList.size() > TOMBSTONE_DIR_MAX) {
        const auto endIndex = indexList.size() - TOMBSTONE_DIR_MAX;
        for (int i = 0; i < endIndex; i++) {
            system(simple_line("rm -rf %s/%d", CRASH_CATCHER_DIR, indexList[i]));
        }
    }

    return res;
}

static void simple_system_cmd(const char *cmd, const char *o_dir, const char *o_name) {
    auto final_cmd = simple_line("%s > %s/%s", cmd, o_dir, o_name);
    system(final_cmd);
}

static const uint32_t node_buf_size = 256;
static char node_buf[node_buf_size];

void write_tombstone(int sig, const siginfo_t *info, const std::string &dir, const __pid_t pid, const pid_t tid);

static const char *read_node(const char *node) {
    memset(node_buf, 0, node_buf_size);
    FILE *file = fopen(node, "r");
    if (file) {
        fread(node_buf, node_buf_size, 1, file);
        fclose(file);
    }
    return node_buf;
}


void collect_info_local(int sig, siginfo_t *info, void *ctx) {
    const auto dir = get_free_dir();
    const auto pid = getpid();
    const auto tid = gettid();

    write_tombstone(sig, info, dir, pid, tid);

    const struct {
        const char *cmd;
        const char *o_name;
    } node_list[] = {
            {simple_line("cat /proc/%d/fd", pid),   "fd"},
            {simple_line("cat /proc/%d/maps", pid), "maps"},
            {"cat /proc/meminfo",                      "meminfo"},
            {"dmesg",                                 "dmesg"},
            {"cat /proc/uptime",                      "uptime"},
            {"cat /proc/cmdline",                     "cmdline"},
            {"cat /proc/version",                     "version"},
            {"ps T",                                  "threads"},

    };

    for (const auto n: node_list) {
        simple_system_cmd(n.cmd, dir.c_str(), n.o_name);
    }

    exit(1);
}

void write_tombstone(int sig, const siginfo_t *info, const std::string &dir, const __pid_t pid, const pid_t tid) {
    const auto tombstone_path = simple_line("%s/tombstone", dir.c_str());
    FILE *f_tombstone = fopen(tombstone_path, "w");
    if (!f_tombstone) {
        printf("failed to open %s, %s\n", tombstone_path, strerror(errno));
        return;
    }
    const auto process = read_node(simple_line("/proc/%d/cmdline", pid));
    time_t now;
    time(&now);
    const char *header = nullptr;

    switch (sig) {
        case SIGSEGV:
        case SIGILL:
        case SIGFPE:
        case SIGBUS: {
            header = simple_line("time: %s process: %d pid: %d tid: %d receive %s, fault_addr %x \n",
                                 ctime(&now), process, pid, tid, strsignal(sig), info->si_addr);
            break;
        }
        case SIGABRT: {
            header = simple_line("time: %s process: %s pid: %d tid: %d receive %s\n",
                                 ctime(&now), process, pid, tid, strsignal(sig));
            break;
        }
        default:
            break;
    }
    fwrite(header, strlen(header), 1, f_tombstone);
    // TODO dump reg

    do {
        unw_cursor_t cursor;
        unw_context_t context;

        // grab the machine context and initialize the cursor
        if (unw_getcontext(&context) < 0)
            break;
        if (unw_init_local(&cursor, &context) < 0)
            break;

        // skip write_tombstone and collect_info_local
        unw_step(&cursor);
        unw_step(&cursor);
        while (unw_step(&cursor) > 0) {
            unw_word_t offset, pc;
            unw_get_reg(&cursor, UNW_REG_IP, &pc);
            const uint32_t sym_size = 1024;
            char sym_buf[sym_size];
            const char* bt = nullptr;
            if (unw_get_proc_name(&cursor, sym_buf, sym_size, &offset) == 0) {
                Dl_info d_info;
                const char* lib_name = "unknown";
                uint32_t lib_off = -1;
                if (dladdr((void*)pc, &d_info)) {
                    lib_name = d_info.dli_fname;
                    lib_off = (unw_word_t) d_info.dli_fbase == 0x400000 ? pc : pc - (unw_word_t) d_info.dli_fbase;
                }

                int status = 0;
                const auto symbol = abi::__cxa_demangle(sym_buf, nullptr, nullptr, &status);
                bt = simple_line("%8x: %s in %s + %x\n", pc, symbol ? symbol : sym_buf, lib_name, lib_off);
                if (symbol) free(symbol);
            } else {
                bt = simple_line("%8x: no symbol\n", pc);
            }
            fwrite(bt, strlen(bt), 1, f_tombstone);
        }
    } while (false);

    fclose(f_tombstone);
}
