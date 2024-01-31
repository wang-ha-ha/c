//
// Created by wxl on 23-11-19.
//

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <cxxabi.h>
#include <libunwind.h>
#include <libunwind-ptrace.h>
#include "CrashDumper.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define ERROR(fmt, ...) printf(fmt, ##__VA_ARGS__);

void MAI::CrashDumper::getThreadList(uint32_t pid, std::list<pid_t> &list) {
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "/proc/%d/task", pid);
    DIR *entry = opendir(path);
    if (entry) {
        struct dirent* dir = nullptr;
        while ((dir = readdir(entry)) != nullptr) {
            if (!strcmp(".", dir->d_name) || !strcmp("..", dir->d_name)) continue;
            list.push_back(std::atoi(dir->d_name));
        }
    } else {
        ERROR("failed to open %s: %s\n", path, strerror(errno));
    }
}

void MAI::CrashDumper::dumpThreadBt(uint32_t tid, std::ostream &s) {
    std::stringstream ss;
    readProcNode(tid, "comm", ss);
    auto res = ss.str();
    res.erase(std::remove(res.begin(), res.end(), '\n'), res.end());
    // write header
    const uint32_t lineLen = 1024;
    char line[lineLen];
    snprintf(line, lineLen, "==== backtrace pid: %d, comm: %s ====", tid, res.c_str());
    s << line << std::endl;
    do {
        unw_cursor_t cursor;
        unw_context_t context;
        unw_word_t ip, sp, off;

        unw_addr_space_t addr_space = unw_create_addr_space(&_UPT_accessors, 0);
        if (!addr_space) {
            ERROR("Failed to create address space\n");
        }


        if (-1 == ptrace(PTRACE_ATTACH, tid, nullptr, nullptr)) {
            ERROR(" failed to ptrace attach\n");
        }

        if (!wait4stop(tid)) {
            ERROR("wait SIGSTOP of ptrace failed\n");
        }

        void *rctx = _UPT_create(tid);

        if (rctx == nullptr)
            ERROR("Failed to _UPT_create\n");

        if (int res = unw_init_remote(&cursor, addr_space, rctx))
            ERROR("unw_init_remote failed: %d\n", res);



        const size_t buffLen = 256;
        char buff[buffLen];
        const char* nosym = "no symbol";
        do {
            char* sym = nullptr;
            char *demangle = nullptr;
            int res = unw_get_reg(&cursor, UNW_REG_IP, &ip);
            if (res) ERROR("failed to get ip %d\n", res);
            if (0 == unw_get_proc_name(&cursor, buff, buffLen, &off)) {
                sym = buff;
                int status = 0;
                demangle = abi::__cxa_demangle(buff, nullptr, nullptr, &status);
                if (demangle) sym = demangle;
            } else {
                sym = (char*)nosym;
            }
            snprintf(line, lineLen, "0x%x: %s+0x%x", ip, sym, off);
            printf("%s\n", line);
            s << line << std::endl;
            if (demangle) free(demangle);
        } while (unw_step(&cursor) > 0);
        
        _UPT_destroy(rctx);
        ptrace(PTRACE_DETACH, tid, nullptr, nullptr);
    } while (0);
}

void MAI::CrashDumper::dumpProcessBt(uint32_t pid, std::ostream &s, uint32_t filter) {
    std::list<pid_t> list;
    getThreadList(pid, list);
    for (const auto tid: list) {
        if (filter == tid) continue;
        dumpThreadBt(tid, s);
    }
}

void MAI::CrashDumper::readProcNode(uint32_t pid, const char* node, std::ostream &os) {
    const uint32_t buffLen = 1024;
    char buff[buffLen];
    snprintf(buff, buffLen, "/proc/%d/%s", pid, node);
    std::ifstream ifs(buff);
    if (ifs.is_open()) {
        os << ifs.rdbuf();
        ifs.close();
    }
}

int MAI::CrashDumper::wait4stop(uint32_t pid) {
    int status = 99;
    do {
        if (waitpid(pid, &status, 0) == -1 || WIFEXITED(status) || WIFSIGNALED(status))
            return 0;
    } while(!WIFSTOPPED(status));
    return 1;
}

pid_t MAI::CrashDumper::getThreadPid(uint32_t tid) {
    pid_t pid = 0;
    const uint32_t buffLen = 1024;
    char buff[buffLen];
    snprintf(buff, buffLen, "/proc/%d/status", tid);
    std::ifstream ifs(buff);
    const uint32_t lineSize = 1024;
    char line[lineSize];
    while (!ifs.eof() && ifs.getline(line, lineSize)) {
        if (sscanf(line, "Tgid:\t%d", &pid) == 1) {
            break;
        }
    }
    return pid;
}
