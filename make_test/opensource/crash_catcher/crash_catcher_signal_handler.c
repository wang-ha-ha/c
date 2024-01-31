//
// Created by wxl on 23-11-21.
//

#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "ucontext.h"
#include "crash_catcher_collect_info.h"
#include "crash_catcher_type.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <syscall.h>

pid_t gettid() {
    return (pid_t) syscall(SYS_gettid);
}

#ifdef COLLECT_LOCAL
#else
static void *shm_ptr = NULL;
static void collect_info_remote(int sig, siginfo_t *info, void *ctx) {
    crash_catcher_payload_t payload;
    memset(&payload, 0, sizeof(crash_catcher_payload_t));
    const char *cct = NULL;
    switch (sig) {
        case SIGSEGV:
        case SIGILL:
        case SIGFPE:
        case SIGBUS: {
            cct = CRASH_CATCHER_TYPE_CRASH;
            payload.target = gettid();
            if (info) {
                memcpy(&(payload.info.crash_info.info), info, sizeof(siginfo_t));
            }

            if (ctx) {
                memcpy(&(payload.info.crash_info.ctx), ctx, sizeof(ucontext_t));
            }

            break;
        }
        case SIGABRT: {
            cct = CRASH_CATCHER_TYPE_ABORT;
            payload.target = gettid();
            break;
        }
        default:
            break;
    }

    uint32_t pid = getpid();
    if (shm_ptr) {
        memcpy(shm_ptr, &payload, sizeof(payload));
        pid_t fork_pid = fork();
        if (fork_pid == 0) {
            const uint32_t cmdSize = 1024;
            char cmd_pid[cmdSize];
            snprintf(cmd_pid, cmdSize, "%d", pid);
            const char *cmd = "/bin/crash_catcher";
            execl(cmd, "crash_catcher", cmd_pid, cct, "0x7", NULL);
            perror("exec failed");
            exit(0);
        }
        waitpid(fork_pid, NULL, 0);
        shmdt(shm_ptr);
    }

    exit(1);
}

void init_shm() {
    const size_t shm_size = 4096;
    assert(shm_size >= sizeof(crash_catcher_payload_t));
    int shmid = shmget(getpid(), shm_size, IPC_CREAT | 0666);
    if (shmid >= 0) {
        shm_ptr = shmat(shmid, NULL, 0);
        if (!shm_ptr) {
            perror("shmat");
        }
    } else {
        perror("shmget");
    }
}
#endif

void register_handler() {
    struct sigaction sa;
    sigset_t mask;
    printf("WCQ==TEST register_handler\n");
    sigemptyset(&mask);
#ifdef COLLECT_LOCAL
    sa.sa_sigaction = collect_info_local;
#else
    sa.sa_sigaction = collect_info_remote;
#endif

    sa.sa_mask = mask;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);

    sigaction(SIGABRT, &sa, NULL);
}

void __attribute__((constructor)) init_crash_catcher() {
// void init_crash_catcher() {
    printf("WCQ==TEST init_crash_catcher\n");
#ifdef COLLECT_LOCAL
#else
    init_shm();
#endif
    register_handler();
}