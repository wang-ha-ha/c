//
// Created by wxl on 23-11-21.
//

#ifndef CRASH_CATCHER_CRASH_CATCHER_TYPE_H
#define CRASH_CATCHER_CRASH_CATCHER_TYPE_H

#include <stdint.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRASH_CATCHER_DIR      "/userfs/tombstone"
#define TOMBSTONE_DIR_MAX 5

typedef enum {
    CRASH,
    ABORT,
    DUMP,
    NONE,
} crash_catcher_type_e;

#define CRASH_CATCHER_TYPE_CRASH    "crash"
#define CRASH_CATCHER_TYPE_ABORT    "abort"
#define CRASH_CATCHER_TYPE_DUMP    "dump"

#define DUMP_FLAG_BT        (0x1)
#define DUMP_FLAG_PROC      (0x2)
#define DUMP_FLAG_COMMON    (0x4)

#define DUMP_FLAG_ALL       (0xffffffff)

typedef struct {
    uint32_t target;
    union {
        struct {
            siginfo_t info;
            ucontext_t ctx;
        } crash_info;
    } info;
} crash_catcher_payload_t;


#ifdef __cplusplus
};
#endif

#endif //CRASH_CATCHER_CRASH_CATCHER_TYPE_H
