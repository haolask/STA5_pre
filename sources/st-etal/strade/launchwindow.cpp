/****************************************************************************
**
** Copyright (C) 2013 STMicroelectronics. All rights reserved.
**
** This file is part of DAB Test Application
**
** Author: Marco Barboni (marco.barboni@st.com)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation or under the
** terms of the Qt Commercial License Agreement. The respective license
** texts for these are provided with the open source and commercial
** editions of Qt.
**
****************************************************************************/
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QTimer>

#include "postaloffice.h"
#include "launchwindow.h"
#include "ui_launchwindow.h"

LaunchWindow::LaunchWindow(QMainWindow* parent,
                           QString m_InstancerName) :
    QMainWindow(parent),
    FilesManager(m_InstancerName),
    ui(new Ui::LaunchWindow)
{
    ui->setupUi(this);
    this->setObjectName("main_thread");

    ApplicationInit();

    setAttribute(Qt::WA_DeleteOnClose, true);
}

LaunchWindow::~LaunchWindow()
{
    delayTimer->stop();

    delete delayTimer;
    delete eventLoop;
    delete worker;
    qDebug() << "Deleted: worker";

    delete dabradio;
    qDebug() << "Deleted: dabradio";

    delete radioGlobalData;
    qDebug() << "Deleted: radioGlobalData";

    delete ui;
    qDebug() << "Deleted: LaunchWindow";
}

void LaunchWindow::CloseApplication_Slot()
{
    // TODO: stop RDS and other active tasks

    ExitThreads();
    qDebug() << "Exit threads";

    QWidget::close();
    qDebug() << "Application Close";

    delete shellWindow;
}

#if 0
#include "test_threads.h"
#endif // #if 0

#if 0
#include "presets.h"
#endif // #if 0

#if 0
#include "sm.h"
#endif // #if 0

void LaunchWindow::ApplicationInit()
{
    // Main thread ID
    qDebug() << "Main thread ID: " << QThread::currentThreadId() << "(" << QThread::currentThread() << ")";

    //===
    // TEST state machine
    //vvv
#if 0
    const QString STATE_SERVICELIST = "STATE: service list";
    const QString STATE_PRESETS = "STATE: presets";
    const QString STATE_SLS_PTY = "STATE: sls/pty";
    const QString STATE_QUALITY = "STATE: quality";
    const QString STATE_SETUP = "STATE: setup";
    const QString STATE_PREVIOUS = nullptr;

    const QString ACTION_PRESS_PRESETS = "ACTION: press presets";
    const QString ACTION_PRESS_LIST = "ACTION: press list";
    const QString ACTION_PRESS_SCREEN = "ACTION: press screen";
    const QString ACTION_ON_TIMER = "ACTION: on timer";
    const QString ACTION_NEW_SLS = "ACTION: new sls";
    const QString ACTION_PRESS_SETUP = "ACTION: press setup";
    const QString ACTION_TUNE = "ACTION: tune";

    QList<QString> stateList;
    QList<QString> eventList;
    SmMachine* smMachineDab = new SmMachine("SM: DAB");
    SmMachine* smMachineFm = new SmMachine("SM: FM");
    SmMachine* currentSmMachine = nullptr;
    QList<QString>::const_iterator iter;

    // Just for better debug
    qSetMessagePattern("%{file}(%{line}): %{message}");

    stateList.append(STATE_SERVICELIST);
    stateList.append(STATE_PRESETS);
    stateList.append(STATE_SLS_PTY);
    stateList.append(STATE_QUALITY);
    stateList.append(STATE_SETUP);

    eventList.append(ACTION_PRESS_PRESETS); // User press presets button
    eventList.append(ACTION_PRESS_LIST);    // User press list button
    eventList.append(ACTION_PRESS_SCREEN);  // User press screen
    eventList.append(ACTION_ON_TIMER);      // The timer expires
    eventList.append(ACTION_NEW_SLS);       // New sls arrives
    eventList.append(ACTION_PRESS_SETUP);   // USer press setup
    eventList.append(ACTION_TUNE);          // Gather all actions resulting in a tune: seek, scan, tune, source

    ///
    // SETUP DAB STATE MACHINE
    ///
    // Insert states in DAB state machine
    for (iter = stateList.constBegin(); iter != stateList.constEnd(); ++iter)
    {
        const QString& tmpStateName = *iter;

        if (false == smMachineDab->InsertNewState(tmpStateName))
        {
            qDebug() << "ERROR: insertion of duplicated state name " << tmpStateName;
        }
    }

    // Setup fallback state for SETUP (necessary when no enabled alternatives are available)
    smMachineDab->SetFallbackState(STATE_SETUP, STATE_SERVICELIST);

    // Set default state
    if (false == smMachineDab->SetDefaultState(STATE_SERVICELIST))
    {
        qDebug() << "ERROR: default state was not set as default correctly";
    }

    if (false == smMachineDab->EnterDefaultState())
    {
        qDebug() << "ERROR: default state was not entered";
    }

    // Insert events in DAB state machine
    for (iter = eventList.constBegin(); iter != eventList.constEnd(); ++iter)
    {
        const QString& tmpEventName = *iter;

        if (false == smMachineDab->InsertNewEvent(tmpEventName))
        {
            qDebug() << "ERROR: insertion of duplicated event name " << tmpEventName;
        }
    }

    // Test error conditions
    if (true == smMachineDab->InsertNewState(STATE_SERVICELIST))
    {
        qDebug() << "ERROR: insertion of duplicated state name " << STATE_SERVICELIST;
    }

    if (true == smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_PRESETS, "STATE: fake", 0))
    {
        qDebug() << "ERROR: state change called with invalid parameters was wrongly registered";
    }

    // Setup transitions for DAB state machine
    // From state service list
    if (false == smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_PRESETS, STATE_PRESETS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_LIST, STATE_SLS_PTY, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_ON_TIMER, STATE_QUALITY, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_NEW_SLS, STATE_SLS_PTY, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_SETUP, STATE_SETUP, 2)) // Setup is always top priority
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // From state presets
    if (false == smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_PRESS_PRESETS, STATE_PREVIOUS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_PRESS_LIST, STATE_SERVICELIST, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_ON_TIMER, STATE_PREVIOUS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_PRESS_SETUP, STATE_SETUP, 2)) // Setup is always top priority
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // From state sls
    if (false == smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_LIST, STATE_SERVICELIST, 0)) // Alternate choice with lower priority
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_LIST, STATE_QUALITY, 1)) // Alternate choice with higher priority
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_SCREEN, STATE_SERVICELIST, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_SETUP, STATE_SETUP, 2)) // Setup is always top priority
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_TUNE, STATE_SERVICELIST, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // From state quality
    if (false == smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_PRESS_LIST, STATE_SERVICELIST, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_PRESS_SETUP, STATE_SETUP, 2)) // Setup is always to p priority
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_TUNE, STATE_SERVICELIST, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // From state setup
    if (false == smMachineDab->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_PREVIOUS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineDab->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_QUALITY, 1)) // Alternate choice with higher priority
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // Disable not active state
    if (false == smMachineDab->DisableState(STATE_QUALITY))
    {
        qDebug() << "ERROR: disabling state failed";
    }

    // Set current state
    if (false == smMachineDab->SetCurrentState(STATE_SERVICELIST))
    {
        qDebug() << "ERROR: setting current state failed";
    }

    ///
    // SETUP FM STATE MACHINE
    ///
    // Insert states in FM state machine
    for (iter = stateList.constBegin(); iter != stateList.constEnd(); ++iter)
    {
        const QString& tmpStateName = *iter;

        if (false == smMachineFm->InsertNewState(tmpStateName))
        {
            qDebug() << "ERROR: insertion of duplicated state name " << tmpStateName;
        }
    }

    // Setup fallback state for SETUP (necessary when no enabled alternatives are available)
    smMachineFm->SetFallbackState(STATE_SETUP, STATE_SLS_PTY);

    // Set default state
    if (false == smMachineFm->SetDefaultState(STATE_SLS_PTY))
    {
        qDebug() << "ERROR: default state was not set as default correctly";
    }

    if (false == smMachineFm->EnterDefaultState())
    {
        qDebug() << "ERROR: default state was not entered";
    }

    // Insert events in FM state machine
    for (iter = eventList.constBegin(); iter != eventList.constEnd(); ++iter)
    {
        const QString& tmpEventName = *iter;

        if (false == smMachineFm->InsertNewEvent(tmpEventName))
        {
            qDebug() << "ERROR: insertion of duplicated event name " << tmpEventName;
        }
    }

    // Disable not active state
    if (false == smMachineFm->DisableState(STATE_QUALITY))
    {
        qDebug() << "ERROR: disabling state failed";
    }

    if (false == smMachineFm->DisableState(STATE_SERVICELIST))
    {
        qDebug() << "ERROR: disabling state failed";
    }

    // Set current state
    if (false == smMachineFm->SetCurrentState(STATE_SLS_PTY))
    {
        qDebug() << "ERROR: setting current state failed";
    }

    // Setup transitions for FM state machine
    // From state presets
    if (false == smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_PRESETS, nullptr, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_LIST, STATE_SLS_PTY, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_LIST, STATE_QUALITY, 1))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_ON_TIMER, STATE_PREVIOUS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_SETUP, STATE_SETUP, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // From state pty
    if (false == smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_LIST, STATE_QUALITY, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_ON_TIMER, STATE_QUALITY, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_SETUP, STATE_SETUP, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // From state quality
    if (false == smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_PRESS_LIST, STATE_SLS_PTY, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_PRESS_SETUP, STATE_SETUP, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    // From state setup
    if (false == smMachineFm->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_PREVIOUS, 0))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    if (false == smMachineFm->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_QUALITY, 1))
    {
        qDebug() << "ERROR: state change called with not valid parameters";
    }

    ///
    // Start TESTS
    ///
    // Setup current used state machine (it changes on source button press)
    currentSmMachine = smMachineDab;

    // TEST 1: test an event
    QString newState1 = currentSmMachine->CalculateNewState(ACTION_PRESS_PRESETS);

    qDebug() << "Transition occurred from " << STATE_SERVICELIST << " to state " << newState1 << " (expected PRESETS)";

    QString newState2 = currentSmMachine->CalculateNewState(ACTION_PRESS_PRESETS);

    qDebug() << "Transition occurred from " << newState1 << " to state " << newState2 << " (expected SERVICE LIST)";

    // TEST 2: test a sequence
    QString newState3 = currentSmMachine->CalculateNewState(ACTION_NEW_SLS);

    qDebug() << "Transition occurred from " << STATE_SERVICELIST << " to state " << newState3 << " (expected SLS)";

    QString newState4 = currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP);

    qDebug() << "Transition occurred from " << newState3 << " to state " << newState4 << " (expected SETUP)";

    if (false == currentSmMachine->EnableState(STATE_QUALITY))
    {
        qDebug() << "ERROR: enabling state failed";
    }

    QString newState5 = currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP);

    qDebug() << "Transition occurred from " << newState4 << " to state " << newState5 << " (expected QUALITY because enabled)";

    QString newState6 = currentSmMachine->CalculateNewState(ACTION_PRESS_LIST);

    qDebug() << "Transition occurred from " << newState5 << " to state " << newState6 << " (expected SERVICE LIST)";

    QString newState7 = currentSmMachine->CalculateNewState(ACTION_ON_TIMER);

    qDebug() << "Transition occurred from " << newState6 << " to state " << newState7  << " (expected QUALITY)";

    QString newState8 = currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP);

    qDebug() << "Transition occurred from " << newState7 << " to state " << newState8 << " (expected SETUP)";

    if (false == currentSmMachine->DisableState(STATE_QUALITY))
    {
        qDebug() << "ERROR: enabling state failed";
    }

    QString newState9 = currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP);

    qDebug() << "Transition occurred from " << newState8 << " to state " << newState9 << " (expected old status but QUALITY is disabled, fallback to SERVICE LIST)";

    // TEST 3: pressing source we enter FM state machine and then we press presets and check on timer event
    currentSmMachine = smMachineFm;

    QString newState10 = currentSmMachine->GetCurrentState();

    qDebug() << "User pressed source: entering FM. Current status is default for FM. New state is " << newState10 << " (expected is PTY)";

    QString newState11 = currentSmMachine->CalculateNewState(ACTION_PRESS_PRESETS);

    qDebug() << "Transition occurred from " << newState10 << " to state " << newState11  << " (expected PRESETS)";

    QString newState12 = currentSmMachine->CalculateNewState(ACTION_ON_TIMER);

    qDebug() << "Transition occurred from " << newState11 << " to state " << newState12  << " (expected PTY)";
#endif // #if 0

    //===
    // TEST service lists
    //vvv
#if 0
    StationListGlobal* stationListGlobal = stationListGlobal->Instance();

    stationListGlobal->Test();
    //^^^
#endif // #if 0

    //===
    // TEST presets
    //vvv
#if 0
    Presets* presetList = presetList->Instance();

    ///
    // Step 1: Create the list and check services inside the list, 2 FM and 3 DAB services shall be reported
    ///
    presetList->SetPreset(6, "Radio Rai 7", 0x5207, 105500);
    presetList->SetPreset(5, "Radio Rai 5", 0x5205, 103500);
    presetList->SetPreset(4, "Radio Rai 4", 0x5204, 102500);
    presetList->SetPreset(1, "Radio Rai 1", 0x5201, 88000);
    presetList->SetPreset(2, "Radio Rai 2", 0x5202, 89000);
    presetList->SetPreset(3, "Radio Rai 3", 0x5203, 90000);

    presetList->SetPreset(6, "Radio Rai 6", 0x5206, 104500);

    // Check number of FM services
    cout << "---------------------------- " <<  endl;
    cout << "STEP 1 - Number of presets: " << presetList->GetNumberOfServices() << endl;
    cout << "---------------------------- " <<  endl;

    // Print preset 6
    RadioServicePreset preset(presetList->GetPreset(6));

    cout << "---------------------------- " << endl;
    cout << "STEP 1 - Preset 6 name      : " << preset.GetServiceName() << endl;
    cout << "STEP 1 - Preset 6 frequency : " << preset.GetFrequency() << endl;
    cout << "STEP 1 - Preset 6 ID        : " << preset.GetId() << endl;
    cout << "---------------------------- " << endl;

    ///
    // Step 2: Save the data and clear memory content, 0 services shall be reported
    ///
    // Save data from the PRESET list
    ofstream outFilePresets("presets.bin", ios::out | ios::binary);

    outFilePresets << *presetList;

    outFilePresets.close();

    presetList->ClearServiceList();

    // Check number of services
    cout << "---------------------------- " <<  endl;
    cout << "STEP 2 - Number of PRESETS after clear (must be 0): " << presetList->GetNumberOfServices() << endl;
    cout << "---------------------------- " <<  endl;

    ///
    // Step 3: Load data, 2 FM and 3 DAB services shall be reported
    ///
    // Load data
    ifstream inFilePresets("presets.bin", ios::in | ios::binary);

    inFilePresets >> *presetList;

    // Check number of FM services
    cout << "STEP 3 - Number of PRESETS after load: " << presetList->GetNumberOfServices() << endl;
#endif // #if 0
       //^^^

    //===
    // TEST QThread
    //vvv
#if 0
    TestStateMachine* testStateMachine1 = new TestStateMachine(2000);
    TestThread* testThread1 = new TestThread();

    //#if 0 // Use method 1
    //    QTimer *timerPtr = testStateMachine1->GetTimerAddress();
    //    timerPtr->moveToThread(testThread1);
    //#else
    //    testStateMachine1->SetTargetThread(testThread1);
    //#endif // #if 0

    testStateMachine1->moveToThread(testThread1);

    //connect(testStateMachine1, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(testThread1, &TestThread::started, testStateMachine1, &TestStateMachine::process);
    connect(testStateMachine1, &TestStateMachine::finished, testThread1, &TestThread::quit);
    connect(testStateMachine1, &TestStateMachine::finished, testStateMachine1, &TestStateMachine::deleteLater);
    connect(testThread1, &TestThread::finished, testThread1, &TestThread::deleteLater);

    testThread1->start();

#if 0 // Start second thread
    TestStateMachine* testStateMachine2 = new TestStateMachine(3000);
    TestThread* testThread2 = new TestThread();

    //#if 0 // Use method 1
    //    QTimer *timerPtr = testStateMachine2->GetTimerAddress();
    //    timerPtr->moveToThread(testThread2);
    //#else
    //    testStateMachine2->SetTargetThread(testThread2);
    //#endif // #if 0

    testStateMachine2->moveToThread(testThread2);

    //connect(testStateMachine2, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(testThread2, &TestThread::started, testStateMachine2, &TestStateMachine::process);
    connect(testStateMachine2, &TestStateMachine::finished, testThread2, &TestThread::quit);
    connect(testStateMachine2, &TestStateMachine::finished, testStateMachine2, &TestStateMachine::deleteLater);
    connect(testThread2, &TestThread::finished, testThread2, &TestThread::deleteLater);

    testThread2->start();
#endif

    while (1)
    {
        QApplication::processEvents(QEventLoop::AllEvents);
    }
#endif // #if 0
       //^^^

    // Allocate memory for the radio panel status data
    radioGlobalData = new RadioStatusGlobal();

    shellWindow = new ShellWindow();

    // Instantiate the Delay Timer
    delayTimer = new QTimer();

    // Instantiate the Event Loop
    eventLoop = new QEventLoop();

    // Instantiate the dabradio
    dabradio = new DabRadio(radioGlobalData, 0, QString("DabRadio"));

    // Instantiate objects on the worker thread
    worker = new Worker(radioGlobalData);

    worker->setObjectName("worker");

    // Register type in order to use it in queued connections
    qRegisterMetaType<Events> ("Events");
    qRegisterMetaType<StatusCodeTy> ("StatusCodeTy");
    qRegisterMetaType<PostalOfficeBase *> ("PostalOfficeBase");
    qRegisterMetaType<DataContainer> ("DataContainer");

    QObject::connect(dabradio, &DabRadio::CloseApplication_Signal, this, &LaunchWindow::CloseApplication_Slot);

    QObject::connect(worker,
                     static_cast<void (Worker::*)(Events, StatusCodeTy, PostalOfficeBase *)> (&Worker::WorkerAnswerSignal),
                     dabradio,
                     static_cast<void (DabRadio::*)(Events, StatusCodeTy, PostalOfficeBase *)> (&DabRadio::WorkerAnswerSlot));

    QObject::connect(dabradio,
                     static_cast<void (DabRadio::*)(Events)> (&DabRadio::EventFromHmiSignal),
                     worker,
                     static_cast<void (Worker::*)(Events)> (&Worker::EventFromHmiSlot));

    QObject::connect(dabradio,
                     static_cast<void (DabRadio::*)(Events, DataContainer)> (&DabRadio::EventFromHmiSignal),
                     worker,
                     static_cast<void (Worker::*)(Events, DataContainer)> (&Worker::EventFromHmiSlot));

    QObject::connect(worker,
                     static_cast<void (Worker::*)(PostalOfficeBase *)> (&Worker::WorkerEventFromDeviceSignal),
                     dabradio,
                     static_cast<void (DabRadio::*)(PostalOfficeBase *)> (&DabRadio::WorkerEventFromDeviceSlot));

    // Timer to timeout the EventLoop of delay method
    QObject::connect(delayTimer, &QTimer::timeout, this, &LaunchWindow::delayTimerTimeout_slot);

    // Move worker to workerThread
    worker->moveToThread(&workerThread);
    workerThread.start();

    // TODO: remove comment to start station manager thread and execute a simple test
    StationListManagement();

    // We do wait for threads to execute and then we can send commands (CMOST boot is the first)
    QThread::msleep(300);

    // Show main window
    dabradio->show();

    // Update panel with startup status
    dabradio->PanelStartUp();
}

void LaunchWindow::StationListManagement()
{
    stationListManager = new StationListManager(radioGlobalData, shellWindow);

    stationListManagerThread = new StationListManagerThread();

    // Move station list class to the thread using the thread wrapper
    stationListManager->moveToThread(stationListManagerThread);

    // Do connections
    QObject::connect(stationListManager,
                     static_cast<void (StationListManager::*)(QString)> (&StationListManager::error),
                     this,
                     static_cast<void (LaunchWindow::*)(QString)> (&LaunchWindow::stationListError));

    QObject::connect(stationListManager,
                     static_cast<void (StationListManager::*)(Events)> (&StationListManager::eventToRadioManager),
                     worker,
                     static_cast<void (Worker::*)(Events)> (&Worker::EventFromHmiSlot));

    QObject::connect(dabradio,
                     static_cast<void (DabRadio::*)(Events, quint32)> (&DabRadio::GlobalListApplicationProlog_Signal),
                     stationListManager,
                     static_cast<void (StationListManager::*)(Events, quint32)> (&StationListManager::eventFromHmiProlog));

    qDebug() << "StationListMngr connect Currently active thread ID: " << QThread::currentThreadId() << "(" << QThread::currentThread() << ")";

    QObject::connect(dabradio,
                     static_cast<void (DabRadio::*)(Events, quint32)> (&DabRadio::GlobalListApplicationEpilog_Signal),
                     stationListManager,
                     static_cast<void (StationListManager::*)(Events, quint32)> (&StationListManager::eventFromHmiEpilog));

    QObject::connect(stationListManager, &StationListManager::finished, stationListManagerThread, &StationListManagerThread::quit);
    QObject::connect(stationListManager, &StationListManager::finished, stationListManager, &StationListManager::deleteLater);
    QObject::connect(stationListManagerThread, &StationListManagerThread::finished, stationListManagerThread, &StationListManagerThread::deleteLater);
    QObject::connect(this, &LaunchWindow::terminateStationList, stationListManager, &StationListManager::stop);
    QObject::connect(this, &LaunchWindow::terminateStationList, stationListManager, &StationListManager::terminate);

#if (defined CONFIG_USE_ETAL)
    QObject::connect(worker, &Worker::WorkerEventSignal, stationListManager, &StationListManager::seek_gl);
#endif // #if (defined CONFIG_USE_ETAL)

    // Start the station list manager thread
    stationListManagerThread->start();

    // Wait 1 second before asking thread to terminate
    // QThread::sleep(1);

    // Terminate station list manager thread (delete later is called for the class and the thread to let them gracefully end)
    // On terminateStationList()-> StationListManager::terminate() -> StationListManager::finished()
    //                                                                        |             |
    //                                                                        v             v
    //                                             StationListManagerThread::quit()     StationListManager::deleteLater()
    //                                                                        |
    //                                                                        v
    //                                      StationListManagerThread::deleteLater()
    // emit terminateStationList();
}

void LaunchWindow::stationListError(QString errorStr)
{
    qDebug() << "Error from station list manager: " << errorStr;
}

void LaunchWindow::delayTimerTimeout_slot()
{
    eventLoop->quit();
}

void LaunchWindow::ExitThreads()
{
    workerThread.quit();

    // Commented next line for test
    emit terminateStationList();

    while (false == workerThread.isFinished())
    {
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(100);
    }

    if (false == workerThread.isRunning())
    {
        qDebug() << "Worker thread is stopped";
    }
}

void LaunchWindow::disp_slot()
{ }

void LaunchWindow::WorkerProcessFinished()
{
    QString tmpStr = "Received process #";

    processNumber++;
    tmpStr.append(QString::number(processNumber, 16));
}
