#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <QTimer>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QEventTransition>
#include <QEventLoop>
#include <QDebug>

class StateMachine : public QObject
{
    Q_OBJECT

    public:
        StateMachine(QObject *parent = nullptr);

        void Exec(int pollingTimeMSec = 1000);

        virtual ~StateMachine ();

        bool IsRunning ()
        {
            if (true == initDone)
            {
                return machine->isRunning ();
            }
            else
            {
                return false;
            }
        }

    protected:
        virtual void IdleStateProcess () = 0;

        virtual void ActiveStatePollingProcess () = 0;

        virtual void ActiveStateEventProcess () = 0;

        virtual void ExitStateProcess () = 0;

    protected:
        QTimer *transitionTimer;

    private:
        QStateMachine *machine;

        QTimer timer;

        bool initDone;

    signals:
        void idleStateTransition_signal();

        void externalEventTransition_signal();

        void terminateStateTransition_signal();

    private slots:
        void StateMachineStateS1_idle_slot ()
        {
            IdleStateProcess();

            // qDebug() << "start transitionTimer";

            transitionTimer->start();
        }

        void StateMachineStateS1_polling_slot ()
        {
            ActiveStatePollingProcess();

            emit idleStateTransition_signal();
        }

        void StateMachineStateS1_event_slot ()
        {
            ActiveStateEventProcess();

            emit idleStateTransition_signal();
        }

        void StateMachineStateS2_slot ()
        {
            ExitStateProcess();
        }

        void InternalTimeout_slot ();
};

#endif // STATE_MACHINE_H
