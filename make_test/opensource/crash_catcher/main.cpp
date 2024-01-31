#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sstream>

#include <sys/shm.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <fstream>
#include <dirent.h>

#include "crash_catcher_type.h"
#include "CrashDumper.h"

#define TOMBSTONE_DIR_MAX 5

static const uint32_t lineLen = 1024;
static char line[1024];
const char* simple_line(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, lineLen, fmt, ap);
    va_end(ap);
    return line;
}

bool is_digit(const char* s) {
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

std::string get_free_dir() {
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
    if (indexList.size() > 0) {
        std::sort(indexList.begin(), indexList.end());
        latestIndex = indexList.back() + 1;
    }

    res = simple_line("%s/%d", CRASH_CATCHER_DIR, latestIndex);
    mkdir(res.c_str(), 0666);

    indexList.push_back(latestIndex);
    if (indexList.size() > TOMBSTONE_DIR_MAX) {
        const int endIndex = indexList.size() - TOMBSTONE_DIR_MAX;
        for (int i = 0; i < endIndex; i++) {
            system(simple_line("rm -rf %s/%d", CRASH_CATCHER_DIR, indexList[i]));
        }
    }

    return res;
}

void do_crash_dump(const std::string &dir, uint32_t pid, int32_t flag, crash_catcher_type_e cct,
                   crash_catcher_payload_t &payload) {
    printf("%s: %d 0x%x -> %s\n", __FUNCTION__, pid, flag, dir.c_str());

    std::ofstream ofs(simple_line("%s/tombstone", dir.c_str()));

    std::stringstream ss;
    MAI::CrashDumper::readProcNode(pid, "cmdline", ss);
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    ofs << simple_line("%s%s %d", asctime(timeinfo), ss.str().c_str(), pid) << std::endl;
    uint32_t tid = 0;
    switch (cct) {
        case CRASH: {
            tid = payload.target;
            snprintf(line, lineLen, "process %d crashed in thread %d as sig %d fault_addr: 0x%x",
                     pid, tid, payload.info.crash_info.info.si_signo,
                     payload.info.crash_info.info.si_addr);
            ofs << line << std::endl;
            // TODO dump register
            break;
        }
        case ABORT: {
            tid = payload.target;
            ofs << simple_line("process %d raised abort in thread %d", pid, tid) << std::endl;
            break;
        }
        default:
            break;
    }

    if (flag & DUMP_FLAG_BT) {
        if (tid) {
            MAI::CrashDumper::dumpThreadBt(tid, ofs);
        }

        if (pid) {
            MAI::CrashDumper::dumpProcessBt(pid, ofs, tid);
        }
    }

    if (flag & DUMP_FLAG_PROC) {
        system(simple_line("ls -l /proc/%d/fd > %s/fd", pid, dir.c_str()));
        system(simple_line("cat /proc/%d/maps > %s/maps", pid, dir.c_str()));
    }

    if (flag & DUMP_FLAG_COMMON) {
        // TODO system version
        // TODO logcat
        system(simple_line("cat /proc/meminfo > %s/meminfo", dir.c_str()));
        system(simple_line("dmesg > %s/dmesg", dir.c_str()));
        system(simple_line("cat /proc/uptime > %s/uptime", dir.c_str()));
        system(simple_line("cat /proc/cmdline > %s/cmdline", dir.c_str()));
        system(simple_line("cat /proc/version > %s/version", dir.c_str()));
        system(simple_line("ps T > %s/ps", dir.c_str()));
    }
}

void printfUsage() {
    printf("Usage: crash_catcher pid type flag\n"
           "\ttype: one of crash abort kill dump\n"
           "\tflag: BT - 0x1, PROC - 0x2, COMMON - 0x4\n"
           "example: crash_catcher 3784 dump 0x7\n");
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printfUsage();
        exit(0);
    }

    uint32_t pid = atoi(argv[1]);
    const char* type = argv[2];
    int32_t flag = 0;
    char* endptr = nullptr;
    flag = strtol(argv[3], &endptr, 16);
    if (endptr[0] != '\0') {
        flag = 0;
    }

    if (!pid || !flag) {
        printf("invalid pid(%d) or flag(0x%x)\n", pid, flag);
        printfUsage();
        exit(0);
    }

    struct {
        const char *s;
        crash_catcher_type_e t;
    } maps[] = {
            {CRASH_CATCHER_TYPE_CRASH, CRASH},
            {CRASH_CATCHER_TYPE_ABORT, ABORT},
            {CRASH_CATCHER_TYPE_DUMP,  DUMP},
    };

    crash_catcher_type_e cct = NONE;
    for (const auto m: maps) {
        if (!strcmp(m.s, type)) {
            cct = m.t;
            break;
        }
    }

    if (cct == NONE) {
        printf("invalid cct %s\n", type);
        printfUsage();
        exit(0);
    }

    crash_catcher_payload_t payload;
    bool validPayload = false;
    if ((CRASH == cct) || (ABORT == cct)) {
        const size_t shm_size = 4096;

        int shmid = shmget(pid, shm_size, 0666);
        if (shmid != -1) {
            void *shm_ptr = shmat(shmid, NULL, 0);
            if (shm_ptr) {
                memcpy(&payload, shm_ptr, sizeof(payload));
                shmdt(shm_ptr);
                validPayload = true;
            } else {
                perror("shmat");
            }
            shmctl(shmid, IPC_RMID, nullptr);
        } else {
            perror("shmget");
        }


        if (!validPayload) {
            printf("invalid payload\n");
            printfUsage();
            exit(0);
        }
    }

    const std::string dir = get_free_dir();
    do_crash_dump(dir, pid, flag, cct, payload);
    return 0;
}
