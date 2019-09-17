#if 0
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h> // For usleep()
#include <cstring>  // 'memset'
#include <cstdio>   // 'fopen', 'fwrite'
#include <string>   // std::str

#include <QtGlobal> // For Q_UNUSED
#include <QThread>  // For sleep(), msleep(), usleep()
#endif

#include "hw_init_thread.h"


HwInit::HwInit (QObject *parent) : QObject(parent)
{

}


#if 0
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
/// C++ <HwInitThread> Interface                                       ///
////////////////////////////////////////////////////////////////////////////////
HwInitThread::HwInitThread (tSInt threadWaitTime) : threadWaitTime(threadWaitTime)
{
    frequency = DABMW_INVALID_FREQUENCY;
    isCisInitCalled = false;

#if (defined CONFIG_TARGET_TEST_ENABLE)
    testExec = new (TestExec);
#endif // #if (defined CONFIG_TARGET_TEST_ENABLE)
}

tVoid HwInitThread::ThreadMain()
{
    // Dynamic instances of objects belonging to this thread
    // (cannot be done in the init because they would belong to caller)
    //cisExec = new CisCmds(0, QString("Cis_msg"), QString("Cis_ch"));

    // Infinite loop
    while (false == stopThread)
    {
        // Wait requested wait time between data availability checks
        std::this_thread::sleep_for (std::chrono::milliseconds (threadWaitTime));
        if(false == isCisInitCalled)
        {
            isCisInitCalled = true;

            // emit sendStartUpCmost_signal();
            //cisExec->startUpCmostDevice();
        }
#if (defined CONFIG_TARGET_TEST_ENABLE)
        testExec->Execute (frequency, dbManager);
#endif // #if (defined CONFIG_TARGET_TEST_ENABLE)
    }
}

tVoid HwInitThread::SetCurrentFrequency (tU32 freq)
{
    frequency = freq;
}

////////////////////////////////////////////////////////////////////////////////
/// C++ <HwInit> Interface                                             ///
////////////////////////////////////////////////////////////////////////////////
HwInit::InitializeHwSetup ()
{
    // Dynamic allocation done here belongs to the thread

    // Connect all needed signals

#if 0

    connect (mainProcCis, SIGNAL (cis_sendCmdToDispatcher_signal(QByteArray)),
             mainProcDispatcher, SLOT (dispatcher_receivedCmdFromCis_slot(QByteArray)));

    connect (mainProcDispatcher, SIGNAL (dispatcher_sendWrittenBytesNbToCis_signal(qint64)),
             mainProcCis, SLOT (cis_receivedWrittenBytesNbFromCis_slot(qint64)));

    connect (mainProcCis, SIGNAL (inShellCmostData_signal(QString, QString, int)),
             mainProcShell, SLOT (inShellCmostData_slot(QString, QString, int)));

    connect (mainProcDispatcher, SIGNAL (dispatcher_sendDataToCis_signal(QByteArray)),
             mainProcCis, SLOT (cis_receivedDataFromDispatcher_slot(QByteArray)));

    connect (mainProcRadio, SIGNAL (dabradio_getCisFwVersion_signal(void)),
             mainProcCis, SLOT (getCisFwVersion_slot(void)));

    connect (mainProcRadio, SIGNAL (sendCisDabFrequency_signal(QString)),
             mainProcCis, SLOT (sendCisDabFrequency_slot(QString)));

    connect (mainProcCis, SIGNAL (initCmostComplete_signal(bool)),
             mainProcRadio, SLOT (initCmostComplete_slot(bool)));

    connect (mainProcRadio, SIGNAL (cisTuneFrequency_signal(quint32, quint8, QString)),
             mainProcCis, SLOT (cisTuneFrequency_slot(quint32, quint8, QString)));

    connect (mainProcRadio, SIGNAL (cisMute_signal(int)),
             mainProcCis, SLOT (cisMute_slot(int)));

    connect (mainProcRadio, SIGNAL (cisVolume_signal(int, int)),
             mainProcCis, SLOT (cisVolume_slot(int, int)));

    connect (mainProcCis, SIGNAL(rdsDataUpdate_signal(RdsDataSetTableTy)),
             mainProcRadio, SLOT(rdsDataUpdate_slot(RdsDataSetTableTy)));
#endif // #if 0

//    mainProcCis->startUpCmostDevice();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// C <HwInitThread> class Interface                                   ///
////////////////////////////////////////////////////////////////////////////////
struct HwInitThread* New_HwInitThread (tU64 waitTime)
{
    // Instantiate HwInit with a wait time between run of 1ms
    return new HwInitThread ((tSInt)waitTime);
}

tVoid Start_HwInitThread (struct HwInitThread* d)
{
    Q_UNUSED(d);

    // Start LoggerThread
    //d->ThreadStart ();
}

tVoid Delete_HwInitThread (struct HwInitThread* d)
{
    // Delete HwInitThread
    delete d;
}

tVoid SetCurrentFrequency_HwInitThread (struct HwInitThread *d, tU32 freq)
{
    d->SetCurrentFrequency (freq);
}

////////////////////////////////////////////////////////////////////////////////
/// Pure C thread                                                            ///
////////////////////////////////////////////////////////////////////////////////
static tBool RC_stopThread = false;

tPVoid RC_HwInitThread (tPVoid args)
{
    HwInit *hwInit;

    Q_UNUSED(args);
    Q_UNUSED(hwInit);

#if 0
    // Set to stay in the loop
    RC_stopThread = false;

    // Create a new logger (executing thread every 1ms)
    hwInit = new HwInit ();

    while (false == RC_stopThread)
    {
        // Wait 1ms
        //OSAL_MicroSleep (1000L);
        usleep (1000L);
    }
#endif

    return NULL;
}

tVoid RC_ExitHwInitThread (tVoid)
{
    RC_stopThread = true;
}
#endif
// End of file
