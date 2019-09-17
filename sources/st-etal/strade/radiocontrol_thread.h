///
//! \file          radiocontrol_thread.hpp
//! \brief         Radio Control Thread interface
//! \author        Alberto Saviotti
//!
//! Project        MSR1
//! Sw component   Radio Control
//!
//! Copyright (c)  STMicroelectronics
//!
//! History
//! Date           | Modification               | Author
//! 20161207       | Initial version            | Alberto Saviotti
///

#ifndef RADIOCONTROL_THREAD_HPP_
#define RADIOCONTROL_THREAD_HPP_

////////////////////////////////////////////////////////////////////////////////
/// C/C++ Common interface                                                   ///
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// C++ <RadioControl> class                                                 ///
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
#define INCLUDE_CLASS
#else
#undef INCLUDE_CLASS
#endif

#ifdef INCLUDE_CLASS

#include <thread>
#include <chrono>

#include "common.h"
#include "stm_types.h"
//#include "dabmw.h"

//#include "db_manager.hpp"


class RadioControl
{
    private:
        tSInt waitTime;

    public:
        RadioControl (tSInt waitTime);

        ~RadioControl () { }
};

class RadioControlThread
{
    private:
        tSInt threadWaitTime;

        RadioControl radioControl;

        std::thread radioControlThread;

        tBool stopThread = false;

        //DbManager dbManager;

        tU32 frequency;

        tVoid ThreadMain ();

    public:
        RadioControlThread (tSInt threadWaitTime);

        ~RadioControlThread ()
        {
            stopThread = true;

            if (radioControlThread.joinable ())
            {
                radioControlThread.join ();
            }
        }        

        tVoid ThreadStart ()
        {
            // This will start the thread. Notice move semantics!
            radioControlThread = std::thread (&RadioControlThread::ThreadMain, this);
        }

        tVoid ThreadStop ()
        {
            stopThread = true;
        }

        tVoid SetCurrentFrequency (tU32 freq);
};

#endif // INCLUDE_CLASS

////////////////////////////////////////////////////////////////////////////////
/// C <RadioControlThread> and <RadioControl> classes interface              ///
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
/// C interface for creating a C++ Radio Control thread                      ///
////////////////////////////////////////////////////////////////////////////////
struct RadioControlThread;

extern struct RadioControlThread* New_RadioControlThread (tU64 waitTime);

extern tVoid Start_RadioControlThread (struct RadioControlThread *d);

extern tVoid Delete_RadioControlThread (struct RadioControlThread *d);

extern tVoid SetCurrentFrequency_RadioControlThread (struct RadioControlThread *d, tU32 freq);

////////////////////////////////////////////////////////////////////////////////
/// C interface for creating a C Radio Control thread                        ///
////////////////////////////////////////////////////////////////////////////////
struct RadioControl;

extern tPVoid RC_RadioControlThread (tPVoid args);

extern tVoid RC_ExitRadioControlThread (tVoid);

#ifdef __cplusplus
}
#endif

#endif  // RADIOCONTROL_THREAD_HPP_

// End of file
