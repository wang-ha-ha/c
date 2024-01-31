//
// Created by wxl on 24-1-14.
//

#ifndef CRASH_CATCHER_CRASH_CATCHER_COLLECT_INFO_H
#define CRASH_CATCHER_CRASH_CATCHER_COLLECT_INFO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <signal.h>
void collect_info_local(int sig, siginfo_t *info, void *ctx);
#ifdef __cplusplus
};
#endif



#endif //CRASH_CATCHER_CRASH_CATCHER_COLLECT_INFO_H
