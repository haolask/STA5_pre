#ifndef WORKER_H
#define WORKER_H

#include "target_config.h"

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include <vector>
#include <list>

#include "cmd_mngr_base.h"
#include "observer.h"
#include "event_manager.h"
#include "target_config.h"

#if defined (CONFIG_USE_JASMIN)
#include "radiomanager_jasmin.h"
#else
#include "radiomanager.h"
#endif

#include "state_machine_radiomanager.h"
#include "postaloffice.h"

class WorkerThread : public QThread
{
    Q_OBJECT

    public:
        void run(void)
        {
            // Worker thread ID
            qDebug() << "Worker thread ID: " << QThread::currentThreadId();

            exec();
        }
};

class WorkerSignalsSlots : public QObject
{
    Q_OBJECT

    public:
        explicit WorkerSignalsSlots(QObject* parent = nullptr) : QObject(parent) { }

        virtual ~WorkerSignalsSlots() { }

    public slots:
        virtual void EventFromHmiSlot(Events event) = 0;

        virtual void onInternalTimeout() = 0;

        virtual void onExternalTimeout() = 0;
        virtual void WorkerSlot() = 0;
        virtual void WorkerEvent(void* state) = 0;

    signals:
        void WorkerSignal();
        void WorkerAnswerSignal(Events event, StatusCodeTy statusCode, PostalOfficeBase* data);
        void WorkerEventFromDeviceSignal(eventDataInterface* eventData);
        void WorkerEventFromDeviceSignal(PostalOfficeBase* eventData);
        void WorkerEventSignal(void* state);
};

class Worker : public WorkerSignalsSlots, public EventManagerObserver<eventsList, eventDataInterface>
{
    Q_OBJECT

    public:
        Worker(void* parForRadioManager);

        ~Worker();

        void EventRaised(eventsList eventType, int size, eventDataInterface* eventDataPtr) override;

        void EventFromHmiSlot(Events event) override;
        void EventFromHmiSlot(Events event, DataContainer data);

        void onInternalTimeout() override;

        void onExternalTimeout() override;
        void WorkerSlot() override;
        void WorkerEvent(void* state) override;

    private:
        void CheckEvent(Events event);
        void CheckEvent(Events event, const DataContainer& data);
        void SendServiceListWithPostal(ServiceListTy serviceList);

        void EnqueueDelayedEvent(Events event, const DataContainer* data);

#if defined (CONFIG_USE_JASMIN)
        RadioManagerJasmin* radioManager;
#endif // #if defined(CONFIG_USE_JASMIM)

#if (defined (CONFIG_USE_ETAL) || defined (CONFIG_USE_STANDALONE))
        RadioManager* radioManager;
#endif // #if (defined(CONFIG_USE_ETAL) || defined(CONFIG_USE_STANDALONE))

        StateMachineRadioManager* stateMachineRadioManager;
        StateMachineRadioManagerThread* stateMachineRadioThread;
        EventManager<eventsList, eventDataInterface>* eventManager;
        EnsembleTableTy tmpTable;
        ServiceListTy tmpList;
        bool tmpTableIsValid;
        QTimer* delayedEventsTimer;
        QList<EventPackage> delayedEventsList;
};

#endif // WORKER_H
