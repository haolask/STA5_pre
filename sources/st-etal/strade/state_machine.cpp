#include <QThread>

#include "state_machine.h"

StateMachine::StateMachine(QObject *parent) : QObject(parent), machine(nullptr), timer(this), initDone(false)
{

}

StateMachine::~StateMachine()
{

}

void StateMachine::InternalTimeout_slot ()
{
    qDebug() << "Internal timeout";
}

void StateMachine::Exec(int pollingTimeMSec)
{
    qDebug() << "State Machine thread ID: " << QThread::currentThreadId() << "(" << QThread::currentThread() << ")";

#if 0 // TODO: This test code show that a timer initialized with 'timer(this)' belongs to this thread
    timer.setInterval(10000);
    timer.setSingleShot(false);

    connect (&timer, &QTimer::timeout, this, &StateMachine::InternalTimeout_slot); // New syntax

    timer.start();
#endif // #if 0

    machine = new QStateMachine ();

    QState *stateS1_top = new QState(); // (QState::ParallelStates);
    QState *stateS1_idle= new QState(stateS1_top);
    QState *stateS1_polling = new QState(stateS1_top);
    QState *stateS1_event = new QState(stateS1_top);
    QFinalState *stateS2_top = new QFinalState();

    stateS1_top->setInitialState(stateS1_idle);

    machine->addState(stateS1_top);
    machine->addState(stateS2_top);
    machine->setInitialState(stateS1_top);

    transitionTimer = new QTimer(stateS1_top);

    qDebug() << "set Interval transitionTimer";

    transitionTimer->setInterval(pollingTimeMSec);
    transitionTimer->setSingleShot(true);

    stateS1_top->addTransition(transitionTimer, SIGNAL(timeout()), stateS1_polling);
    stateS1_top->addTransition(this, SIGNAL(externalEventTransition_signal()), stateS1_event);
    stateS1_top->addTransition(this, SIGNAL(terminateStateTransition_signal()), stateS2_top);
    stateS1_polling->addTransition(this, SIGNAL(idleStateTransition_signal()), stateS1_idle);
    stateS1_event->addTransition(this, SIGNAL(idleStateTransition_signal()), stateS1_idle);

    connect (stateS1_idle, &QState::entered, this, &StateMachine::StateMachineStateS1_idle_slot); // New syntax
    connect (stateS1_polling, &QState::entered, this, &StateMachine::StateMachineStateS1_polling_slot); // New syntax
    connect (stateS1_event, &QState::entered, this, &StateMachine::StateMachineStateS1_event_slot); // New syntax
    connect (stateS2_top, &QState::entered, this, &StateMachine::StateMachineStateS2_slot); // New syntax

#if 0 // Proprietary event loop
    QEventLoop loop;

    QObject::connect(machine, SIGNAL(finished()), &loop, SLOT(quit()), Qt::QueuedConnection);

    machine->start();

    loop.exec();

    qDebug() << "Really finished";
#else
    machine->start();
#endif // #if 0

    initDone = true;
}
