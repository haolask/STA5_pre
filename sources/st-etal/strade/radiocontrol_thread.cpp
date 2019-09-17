///
//! \file          radiocontrol_thread.cpp
//! \brief
//! \author        Alberto Saviotti
//!
//! Project        HelloLinux
//! Sw component   logger
//!
//! Copyright (c)  STMicroelectronics
//!
//! History
//! Date           | Modification               | Author
//! 20161207       | Initial version            | Alberto Saviotti
///

//#include "osal.h"

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h> // For usleep()
#include <cstring>  // 'memset'
#include <cstdio>   // 'fopen', 'fwrite'
#include <string>   // std::str

#include <QtGlobal> // For Q_UNUSED
#include <QThread>  // For sleep(), msleep(), usleep()

//#include "db_manager.hpp"

#include "radiocontrol_thread.h"

#if (defined CONFIG_TARGET_TEST_ENABLE)
    #include "test_common.hpp"
#endif // #if (defined CONFIG_TARGET_TEST_ENABLE)

////////////////////////////////////////////////////////////////////////////////
/// Global declarations                                                      ///
////////////////////////////////////////////////////////////////////////////////
#if (defined CONFIG_TARGET_TEST_ENABLE)
    static TestExec* testExec;
#endif // #if (defined CONFIG_TARGET_TEST_ENABLE)

////////////////////////////////////////////////////////////////////////////////
/// C++ <RadioControlThread> Interface                                       ///
////////////////////////////////////////////////////////////////////////////////
RadioControlThread::RadioControlThread (tSInt threadWaitTime) : threadWaitTime(threadWaitTime), radioControl (threadWaitTime)
{
    frequency = DABMW_INVALID_FREQUENCY;

#if (defined CONFIG_TARGET_TEST_ENABLE)
    testExec = new (TestExec);
#endif // #if (defined CONFIG_TARGET_TEST_ENABLE)
}

tVoid RadioControlThread::ThreadMain ()
{
    // Infinite loop
    while (false == stopThread)
    {
        // Wait requested wait time between data availability checks
        std::this_thread::sleep_for (std::chrono::milliseconds (threadWaitTime));

        // Check if we have some new event to check
        //dbManager.CheckEvents ();

        // Call the algorithm to check the station list and update it for the user
        //dbManager.CheckStationList (frequency);

#if (defined CONFIG_TARGET_TEST_ENABLE)
        testExec->Execute (frequency, dbManager);
#endif // #if (defined CONFIG_TARGET_TEST_ENABLE)
    }
}

tVoid RadioControlThread::SetCurrentFrequency (tU32 freq)
{
    frequency = freq;
}

////////////////////////////////////////////////////////////////////////////////
/// C++ <RadioControl> Interface                                             ///
////////////////////////////////////////////////////////////////////////////////
RadioControl::RadioControl (tSInt waitTime) : waitTime(waitTime)
{

}

////////////////////////////////////////////////////////////////////////////////
/// C <RadioControlThread> class Interface                                   ///
////////////////////////////////////////////////////////////////////////////////
struct RadioControlThread* New_RadioControlThread (tU64 waitTime)
{
    // Instantiate RadioControl with a wait time between run of 1ms
    return new RadioControlThread (waitTime);
}

tVoid Start_RadioControlThread (struct RadioControlThread* d)
{
    // Start LoggerThread
    d->ThreadStart ();
}

tVoid Delete_RadioControlThread (struct RadioControlThread* d)
{
    // Delete RadioControlThread
    delete d;
}

tVoid SetCurrentFrequency_RadioControlThread (struct RadioControlThread *d, tU32 freq)
{
    d->SetCurrentFrequency (freq);
}

////////////////////////////////////////////////////////////////////////////////
/// Pure C thread                                                            ///
////////////////////////////////////////////////////////////////////////////////
static tBool RC_stopThread = false;

tPVoid RC_RadioControlThread (tPVoid args)
{
    RadioControl *radioControl;

    Q_UNUSED(args);
    Q_UNUSED(radioControl);

    // Set to stay in the loop
    RC_stopThread = false;

    // Create a new logger (executing thread every 1ms)
    radioControl = new RadioControl (1L);

    while (false == RC_stopThread)
    {
        // Wait 1ms
        //OSAL_MicroSleep (1000L);
        usleep (1000L);
    }

    return NULL;
}

tVoid RC_ExitRadioControlThread (tVoid)
{
    RC_stopThread = true;
}

// End of file
