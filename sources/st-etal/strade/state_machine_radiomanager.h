#ifndef STATE_MACHINE_RADIOMANAGER_H
#define STATE_MACHINE_RADIOMANAGER_H

#include <QDateTime>
#include <QThread>

#include "state_machine.h"
#include "observer.h"
#include "target_config.h"
#include "radiomanagerbase.h"

class StateMachineRadioManagerThread : public QThread
{
    Q_OBJECT

    public:
        void run(void)
        {
            qDebug() << "StateMachineRadioManager thread ID: " << QThread::currentThreadId();

            // Starts the event loop
            exec();
        }
};

class StateMachineRadioManager : public StateMachine, public Observed <Events>
{
    Q_OBJECT

    public:
        //StateMachineRadioManager (RadioManager *rm) : StateMachine(nullptr) //, radioManagerPtr(rm)
        StateMachineRadioManager(unsigned int pollingTimeMs = 10) : StateMachine(nullptr), pollingTime(pollingTimeMs) //, radioManagerPtr(rm)
        {
            dataCheck = false;
            rdsCheck = false;
            qualityCheck = false;

            globalEventsEnable = true;

            lastTimeCheckAlive = INVALID_LONGLONG_DATA;
            lastTimeGetRds = INVALID_LONGLONG_DATA;
            lastTimeGetQuality = INVALID_LONGLONG_DATA;
            lastTimeDabDataCheck = INVALID_LONGLONG_DATA;
        }

    public slots:
        void process()
        {
            // Allocate resources using new here
            qDebug() << "SMRM thread ID: " << QThread::currentThreadId() << "(" << QThread::currentThread() << ")";

            // Make the state machine polling time quite high, 10ms
            Exec(pollingTime);
        }

        void EventFromDeviceSlot(Events event);

    signals:
        void finished();

        void CommandForRadioManagerSignal(Events event, QVariant data);

    private:
        void IdleStateProcess() override
        {
            // If you need to exit the state machine:
            //emit terminateStateTransition_signal();
        }

        void ActiveStatePollingProcess() override
        {
            ExecutePollingActions();
        }

        void ActiveStateEventProcess() override
        {
            qDebug() << "Active state event executed in thread: " << QThread::currentThreadId();
        }

        void ExitStateProcess() override
        {
            qDebug() << "Exit state executed in thread: " << QThread::currentThreadId();
        }

        void ExecutePollingActions();

        bool DoCheckAlive(qint64 curTime);
        bool DoGetRds(qint64 curTime);
        bool DoGetQuality(qint64 curTime);
        bool DoDabDataCheck(qint64 curTime);

        bool globalEventsEnable;

        bool dataCheck;
        bool rdsCheck;
        bool qualityCheck;

        qint64 lastTimeCheckAlive;
        qint64 lastTimeGetRds;
        qint64 lastTimeGetQuality;
        qint64 lastTimeDabDataCheck;

        unsigned int pollingTime;
};

#endif // STATE_MACHINE_RADIOMANAGER_H
