#ifndef HW_INIT_THREAD_H
#define HW_INIT_THREAD_H

//#include <QtConcurrent/QtConcurrent>
//#include <QLocale>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <iostream>

#include "common.h"
#include "stm_types.h"
#include "cmd_mngr_cis.h"
#include "dabradio.h"
#include "dabradio_shell.h"
#include "dispatcher.h"

class HwInit : public QObject
{
    Q_OBJECT

public:
    explicit HwInit (QObject *parent = 0);

    void Process (void)
    {
        std::cout << "HwInit executing and emitting signal" << std::endl;
     //   qDebug () << InitializeHwSetup ();
    }

signals:
    void HwInitSignal (void);

public slots:

};
#if 0
    Q_SLOT void aSlot ()
    {
        qDebug () << QThread::currentThread ();

        QThread::currentThread ()->quit ();
    }

    Q_SLOT void Process ()
    {
        qDebug () << QThread::currentThread ();

        qDebug () << InitializeHwSetup ();

        QThread::currentThread ()->quit ();
    }

    public:
        Q_SIGNAL void aSignal ();

        HwInit (DabRadio& mpdr,
                DabRadio_Shell& mps,
                CisCmds& mpc,
                CommandDispatcher& mpcd) :
            mainProcRadio(&mpdr), mainProcShell(&mps), mainProcCis(&mpc), mainProcDispatcher(&mpcd)
        {
            // Dynamic memory allocationdone here belongs to main
        }

    private:
        int InitializeHwSetup ();

        DabRadio            *mainProcRadio;
        DabRadio_Shell      *mainProcShell;
        CisCmds             *mainProcCis;
        CommandDispatcher   *mainProcDispatcher;
};
#endif

class HwInitThread : public QThread
{
    Q_OBJECT

    public:
        HwInitThread () { timerStarted = false; }

        void run (void)
        {
            std::cout << "HwInit thread started" << std::endl;
            qDebug () <<  QThread::currentThread() << ", object name : " << this->objectName ();

            if (true == timerStarted)
            {
                hwInitThreadTimer.start (1000);
            }

            exec();
        }

        void LinkManagerTimer (QObject *obj, const char* slot)
        {
            connect (&hwInitThreadTimer, SIGNAL (timeout ()), obj, slot);

            timerStarted = true;
        }

    public slots:
        void HwInitThreadSlot (void)
        {
            std::cout << "HwInit thread slot called" << std::endl;
            qDebug () <<  QThread::currentThread() << ", object name : " << this->objectName ();

            emit HwInitThreadSignal ();
        }

    signals:
        void HwInitThreadSignal (void);

    private:
        HwInit t_hwInit;

        QTimer hwInitThreadTimer;

        bool timerStarted;
    };

#endif // HW_INIT_THREAD_H

// End of file

#if 0
    private:
        tSInt   threadWaitTime;

        //HwInit  hwInit;

        std::thread hwInitThread;

        tBool   stopThread = false;

        tU32    frequency;
        tBool   isCisInitCalled;


        tVoid ThreadMain ();

    public:
        HwInitThread (tSInt threadWaitTime);

        ~HwInitThread ()
        {
            stopThread = true;

            if (hwInitThread.joinable ())
            {
                hwInitThread.join ();
            }
        }

        tVoid ThreadStart (DabRadio& dr, DabRadio_Shell& rs, CisCmds& cc, CommandDispatcher& cd)
        {
            HwInit hi (dr, rs, cc,cd);
            QThread hiThread;

            hiThread.setObjectName ("thread_hwinit");

            hi.moveToThread (&hiThread);

            QObject::connect (&hiThread, SIGNAL(started ()), &hi, SLOT(Process ()));
//            QObject::connect (&hiThread, SIGNAL(started (DabRadio,DabRadio_Shell,CisCmds,CommandDispatcher)),
//                              &hi, SLOT(Process (DabRadio,DabRadio_Shell,CisCmds,CommandDispatcher)));

            hiThread.start ();

            hiThread.wait ();
        }

        tVoid ThreadStop ()
        {
            stopThread = true;
        }

        tVoid SetCurrentFrequency (tU32 freq);

private:
        DabRadio            *t_dabRadio;
        DabRadio_Shell      *t_dabRadioShell;
        CisCmds             *t_ciscmds;
        CommandDispatcher   *t_dispatcher;

};

#endif
