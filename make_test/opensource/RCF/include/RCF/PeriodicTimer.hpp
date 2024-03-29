
//******************************************************************************
// RCF - Remote Call Framework
//
// Copyright (c) 2005 - 2018, Delta V Software. All rights reserved.
// http://www.deltavsoft.com
//
// RCF is distributed under dual licenses - closed source or GPL.
// Consult your particular license for conditions of use.
//
// If you have not purchased a commercial license, you are using RCF 
// under GPL terms.
//
// Version: 3.0
// Contact: support <at> deltavsoft.com 
//
//******************************************************************************

#ifndef INCLUDE_RCF_PERIODICTIMER_HPP
#define INCLUDE_RCF_PERIODICTIMER_HPP

#include <RCF/AsioFwd.hpp>
#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/Timer.hpp>

namespace RCF {

    class AsioTimer;
    class I_Service;
    class PeriodicTimer;

    class TimerControlBlock
    {
    public:
        TimerControlBlock(PeriodicTimer * pPeriodicTimer);

        Mutex mMutex;
        PeriodicTimer * mpPeriodicTimer;
    };

    typedef std::shared_ptr<TimerControlBlock> TimerControlBlockPtr;
    typedef std::shared_ptr<AsioTimer> AsioTimerPtr;

    class RCF_EXPORT PeriodicTimer
    {
    public:
        PeriodicTimer(I_Service & service, std::uint32_t intervalMs);
        ~PeriodicTimer();

        void start();
        void stop();

        void setIntervalMs(std::uint32_t intervalMs);
        std::uint32_t getIntervalMs();

    private:

        friend class PeriodicTimerSentry;

        void setTimer();
        void onTimer();

        static void sOnTimer(
            const AsioErrorCode & ec, 
            TimerControlBlockPtr tcbPtr);

        TimerControlBlockPtr    mTcbPtr;
        I_Service &             mService;
        std::uint32_t           mIntervalMs;
        Timer                   mLastRunTimer;
        AsioTimerPtr            mAsioTimerPtr;
    };

} // namespace RCF

#endif // ! INCLUDE_RCF_PERIODICTIMER_HPP
