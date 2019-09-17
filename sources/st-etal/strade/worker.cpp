#include "worker.h"

// Worker::Worker (void *parForRadioManager) : radioManager(this), stateMachineRadioManager(nullptr), stateMachineRadioThread(nullptr)
Worker::Worker(void* parForRadioManager)
{
    // Decalre tmpTable as not valid: it will become valid onece for all
    // when first is copied into a valid entry
    tmpTableIsValid = false;

    // Ensure delayed event list is empty
    delayedEventsList.clear();

    // Delayed timer setup (initialize with this to make it belonging to the new thread)
    delayedEventsTimer = new QTimer(this);
    delayedEventsTimer->setSingleShot(true);
    delayedEventsTimer->setInterval(50); // Set interval to 50ms

    QObject::connect(delayedEventsTimer, &QTimer::timeout, this, &Worker::onInternalTimeout);

    // Start observing events (this event belongs to main thread not to working one)
    // because it has been instantiated on constructor that is executed in main thread
    eventManager = new EventManager<eventsList, eventDataInterface> (this);

    // Pass parameters to radio manager (it is created as belonging to the caller thread: necessary to have TCP/IP working)
    // If the RadioManager is called with 'this' parameters it will go in the thread with all sub-classes (e.g. Middleware, CmdCis)
    // otherwise it will be moved alone on the thread afterwards with 'move to the thread' by the caller
#if defined (CONFIG_USE_JASMIN)
    radioManager =  new RadioManagerJasmin();
#endif

#if (defined (CONFIG_USE_ETAL) || defined (CONFIG_USE_STANDALONE))
    radioManager = new RadioManager();
#endif

    radioManager->Initialize(parForRadioManager);

    // We can now start radio manager state machine
    stateMachineRadioThread = new StateMachineRadioManagerThread();

    stateMachineRadioManager = new StateMachineRadioManager(10);

    // Connections for the state machine
    QObject::connect(stateMachineRadioThread, &StateMachineRadioManagerThread::started, stateMachineRadioManager, &StateMachineRadioManager::process);
    QObject::connect(stateMachineRadioManager, &StateMachineRadioManager::finished, stateMachineRadioThread, &StateMachineRadioManagerThread::quit);
    QObject::connect(stateMachineRadioManager, &StateMachineRadioManager::finished, stateMachineRadioManager, &StateMachineRadioManager::deleteLater);
    QObject::connect(stateMachineRadioThread, &StateMachineRadioManagerThread::finished, stateMachineRadioThread, &StateMachineRadioManagerThread::deleteLater);

    // Connections between the radio manager and the state machine:
    // - Connection to feedback from RadioManager to StateMachineRadioManager
    // - Connection to send commands from StateMachineRadioManager to RadioManager

#if defined (CONFIG_USE_JASMIN)
    QObject::connect((RadioManagerJasmin *)radioManager,
                     static_cast<void (RadioManagerJasmin::*)(Events)> (&RadioManagerJasmin::EventFromDeviceSignal),
                     stateMachineRadioManager,
                     static_cast<void (StateMachineRadioManager::*)(Events)> (&StateMachineRadioManager::EventFromDeviceSlot));
#endif // #if defined(CONFIG_USE_JASMIN)
#if (defined (CONFIG_USE_ETAL) || defined (CONFIG_USE_STANDALONE))
    QObject::connect(radioManager,
                     static_cast<void (RadioManager::*)(Events)> (&RadioManager::EventFromDeviceSignal),
                     stateMachineRadioManager,
                     static_cast<void (StateMachineRadioManager::*)(Events)> (&StateMachineRadioManager::EventFromDeviceSlot));
#endif // #if (defined(CONFIG_USE_ETAL) || defined(CONFIG_USE_STANDALONE))

#if (defined CONFIG_USE_ETAL)
    // signal for bg seek
    QObject::connect(radioManager, &RadioManager::seek_gl, this, &Worker::WorkerEvent);
#endif // #if (defined CONFIG_USE_ETAL)

#if defined (CONFIG_USE_JASMIN)
    // We need a direct connection in order to execute the radio manager slot on state machine thread
    // (or we need to revise all place where stuff is executed and TCP/IP instance between them)
    QObject::connect(stateMachineRadioManager,
                     static_cast<void (StateMachineRadioManager::*)(Events, QVariant)> (&StateMachineRadioManager::CommandForRadioManagerSignal),
                     (RadioManagerJasmin *)radioManager,
                     static_cast<void (RadioManagerJasmin::*)(Events, QVariant)> (&RadioManagerJasmin::CommandForRadioManagerSlot),
                     Qt::DirectConnection);
#endif // #if defined(CONFIG_USE_JASMIN)

#if (defined (CONFIG_USE_ETAL) || defined (CONFIG_USE_STANDALONE))
    QObject::connect(stateMachineRadioManager,
                     static_cast<void (StateMachineRadioManager::*)(Events, QVariant)> (&StateMachineRadioManager::CommandForRadioManagerSignal),
                     radioManager,
                     static_cast<void (RadioManager::*)(Events, QVariant)> (&RadioManager::CommandForRadioManagerSlot),
                     Qt::DirectConnection);
#endif // #if (defined(CONFIG_USE_ETAL) || defined(CONFIG_USE_STANDALONE))

    stateMachineRadioManager->moveToThread(stateMachineRadioThread);

    // Start the state machine
    stateMachineRadioThread->start();
}

Worker::~Worker()
{
    delete eventManager;
    delete radioManager;
}

void Worker::EventRaised(eventsList eventType, int size, eventDataInterface* eventDataPtr)
{
    if (EVENTS_RX_SLS == eventType)
    {
        QByteArray tmpArray = QByteArray(reinterpret_cast<char *> (eventDataPtr->dataPtr), size);

        PostalOffice<QByteArray>* postalOffice = new PostalOffice<QByteArray> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_SLS, tmpArray);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);
    }
    else if (EVENTS_RX_DLS == eventType)
    {
        QString tmpArray = QString(reinterpret_cast<char *> (eventDataPtr->dataPtr));

        PostalOffice<QString>* postalOffice = new PostalOffice<QString> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_DLS, tmpArray);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);

        // Emit signal to radio state> machine
#if 0
        if (stateMachineRadioManager->IsRunning())
        {
            emit stateMachineRadioManager->externalEventTransition_signal();
        }
#endif // #if 0
    }
    else if (EVENTS_RX_SYNC_LEVEL == eventType)
    {
        qualityInfoTy* qualInfo = reinterpret_cast<qualityInfoTy *> (eventDataPtr->dataPtr);

        PostalOffice<qualityInfoTy>* postalOffice = new PostalOffice<qualityInfoTy> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_SYNC_LEVEL, *qualInfo);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);

        // TODO: Inform the state machine regarding this event
#if 0
        if (true == qualInfo->sync)
        {
            stateMachineRadioManager->DoDataCheckOnPolling();
        }
#endif // # if 0
    }
    else if (EVENTS_RX_QUALITY_LEVEL == eventType)
    {
        qualityInfoTy* qualInfo = reinterpret_cast<qualityInfoTy *> (eventDataPtr->dataPtr);

        PostalOffice<qualityInfoTy>* postalOffice = new PostalOffice<qualityInfoTy> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_QUALITY_LEVEL, *qualInfo);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);

        // TODO: Inform the state machine regarding this event
    }
    else if (EVENTS_UPDATE_RDS_DATA == eventType)
    {
        RdsDataSetTableTy* rdsData = reinterpret_cast<RdsDataSetTableTy *> (eventDataPtr->dataPtr);

        PostalOffice<RdsDataSetTableTy>* postalOffice = new PostalOffice<RdsDataSetTableTy> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_RDS, *rdsData);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);
    }
    else if (EVENTS_UPDATE_AUDIO_PLAYS == eventType)
    {
        bool* audio = reinterpret_cast<bool *> (eventDataPtr->dataPtr);

        PostalOffice<bool>* postalOffice = new PostalOffice<bool> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_AUDIO_PLAYS, *audio);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);
    }
    else if (EVENTS_RX_FREQ == eventType)
    {
        RadioTuneAnswer* newFreqAndBand = reinterpret_cast<RadioTuneAnswer *> (eventDataPtr->dataPtr);

        PostalOffice<RadioTuneAnswer>* postalOffice = new PostalOffice<RadioTuneAnswer> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_FREQ, *newFreqAndBand);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);
    }
    else if (EVENTS_RX_ENSEMBLE_NAME == eventType)
    {
        // Copy new data
        memcpy(&tmpTable, eventDataPtr->dataPtr, sizeof (EnsembleTableTy));

        PostalOffice<EnsembleTableTy>* postalOffice = new PostalOffice<EnsembleTableTy> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_ENSEMBLE_NAME, tmpTable);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);
    }
    else if (EVENTS_RX_SERVICE_LIST == eventType)
    {
        // We do not send the list here: we would like to wait for all
        // get specific service data commands to be send and responses to
        // be decoded: we delay the send to HMI when list is verified
    }
    else if (EVENTS_UPDATE_SERVICE_LIST == eventType)
    {
        // Copy new data
        memcpy(&tmpList, eventDataPtr->dataPtr, sizeof (ServiceListTy));
        tmpTableIsValid = true;

        SendServiceListWithPostal(tmpList);
    }
    else if (EVENTS_RX_SERVICE_NAME == eventType)
    {
        QString tmpArray = QString(reinterpret_cast<char *> (eventDataPtr->dataPtr));

        PostalOffice<QString>* postalOffice = new PostalOffice<QString> ();
        PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

        postalOffice->PutPacket(POSTAL_TYPE_IS_SERVICE_NAME, tmpArray);

        postalOffice->Lock();

        // Emit signal to HMI
        emit WorkerEventFromDeviceSignal(postalOfficeBase);
    }
}

void Worker::SendServiceListWithPostal(ServiceListTy serviceList)
{
    PostalOffice<ServiceListTy>* postalOffice = new PostalOffice<ServiceListTy> ();
    PostalOfficeBase* postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

    postalOffice->PutPacket(POSTAL_TYPE_IS_SERVICE_LIST, serviceList);

    postalOffice->Lock();

    // Emit signal to HMI
    emit WorkerEventFromDeviceSignal(postalOfficeBase);
}

void Worker::onExternalTimeout()
{
    qDebug() << "Worker invoked on external timeout by thread : " << QThread::currentThreadId() << ", object name : " << this->objectName();

    // Invoke a slot back to the manager
    emit WorkerSignal();
}

void Worker::WorkerSlot()
{
    qDebug() << "Worker called by thread : " << QThread::currentThreadId() << ", object name : " << this->objectName();
}

void Worker::WorkerEvent(void* state)
{
    emit WorkerEventSignal(state);
}

void Worker::onInternalTimeout()
{
    if (!delayedEventsList.isEmpty())
    {
        EventPackage eventPackage;

        eventPackage = delayedEventsList.takeFirst();

        CheckEvent(eventPackage.event, eventPackage.dataContainer);
    }
}

void Worker::EventFromHmiSlot(Events event)
{
    CheckEvent(event);
}

void Worker::EventFromHmiSlot(Events event, DataContainer data)
{
    CheckEvent(event, data);
}

void Worker::EnqueueDelayedEvent(Events event, const DataContainer* data)
{
    EventPackage eventPackage;

    QList<EventPackage>::const_iterator iter;
    bool found = false;

    eventPackage.event = event;
    if (nullptr != data)
    {
        eventPackage.dataContainer = *data;
    }

    // Before to enqueue the new event check if it is already present, in that case nothing to do
    for (iter = delayedEventsList.constBegin(); iter != delayedEventsList.constEnd(); ++iter)
    {
        const EventPackage tmpPackage = *iter;

        if (tmpPackage.event == event)
        {
            found = true;

            break;
        }
    }

    // Event must be delayed, we do not send response to event generator
    if (false == found)
    {
        delayedEventsList.append(eventPackage);

        // Signal to radio manager about the user pending event
        radioManager->UserEventPending();

        // Start timer to process event
        delayedEventsTimer->start();

        // Debug info
        if (nullptr != data)
        {
            qDebug() << "User event " << event << " delayed" << " - " << "User data.content = " << data->content;
        }
        else
        {
            qDebug() << "User event " << event << " delayed";
        }
    }
    else
    {
        // Debug info
        if (nullptr != data)
        {
            qDebug() << "User event " << event << " not delayed (duplicate)" << " - " << "User data.content = " << data->content;
        }
        else
        {
            qDebug() << "User event " << event << " not delayed (duplicate)";
        }
    }
}

void Worker::CheckEvent(Events event, const DataContainer& data)
{
    if (true == radioManager->ProcessEvent(event, data.content))
    {
        PostalOfficeBase* postalOfficeBase;

        // Retrieve operation result
        StatusCodeTy statusCode = radioManager->GetEventData(event, postalOfficeBase);

        // Event has been processed, send the response now
        emit WorkerAnswerSignal(event, statusCode, postalOfficeBase);

        if (true == delayedEventsList.isEmpty())
        {
            radioManager->UserEventNotPending();
        }
        else
        {
            // Start timer to process event
            delayedEventsTimer->start();
        }
    }
    else
    {
        // Before to enqueue the event we check if we ar ein power down, in that case we throw it away
        if (true == radioManager->IsRadioActive())
        {
            EnqueueDelayedEvent(event, &data);
        }
    }
}

void Worker::CheckEvent(Events event)
{
    if (true == radioManager->ProcessEvent(event, QVariant()))
    {
        PostalOfficeBase* postalOfficeBase;

        // Retrieve operation result
        StatusCodeTy statusCode = radioManager->GetEventData(event, postalOfficeBase);

        // Event has been processed, send the response now
        emit WorkerAnswerSignal(event, statusCode, postalOfficeBase);

        if (true == delayedEventsList.isEmpty())
        {
            radioManager->UserEventNotPending();
        }
        else
        {
            // Start timer to process next event
            delayedEventsTimer->start();
        }
    }
    else
    {
        // Before to enqueue the event we check if we ar ein power down, in that case we throw it away
        if (true == radioManager->IsRadioActive())
        {
            EnqueueDelayedEvent(event, nullptr);
        }
    }
}

// End of file
