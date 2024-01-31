//
// Created by wxl on 23-11-19.
//

#ifndef CRASH_CATCHER_CRASHDUMPER_H
#define CRASH_CATCHER_CRASHDUMPER_H

#include <ctime>
#include <cstdio>
#include <list>
#include <iostream>

namespace MAI {
    class CrashDumper {
    public:
        static void dumpThreadBt(uint32_t tid, std::ostream &s= std::cout);

        static void dumpProcessBt(uint32_t pid, std::ostream &s= std::cout, uint32_t filter= 0);

        static void getThreadList(uint32_t pid, std::list<pid_t>& list);

        static void readProcNode(uint32_t pid, const char* node, std::ostream &os= std::cout);

        static int wait4stop(uint32_t pid);

        static pid_t getThreadPid(uint32_t tid);

    };
};




#endif //CRASH_CATCHER_CRASHDUMPER_H
