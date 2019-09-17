#include "stationlistmanager.h"

StationListManager::StationListManager(RadioStatusGlobal* radioGlobalData, ShellWindow* shell) : globalStationTestExecuted(false)
{
    // Instance of the scheduling timer (with 'this' parameter in order to make it belogning to this thread)
    schedulingTimer = new QTimer(this);

    // Instance data here
    stationList = stationList->Instance();

    // global state pointer initialization
    radioStatusGlobal = radioGlobalData;

#if (defined CONFIG_USE_STANDALONE)
    Q_UNUSED(shell);
#endif // #if (defined CONFIG_USE_STANDALONE)

    // Initialize a new shell for feedback
    qDebug() << "StationListMngr Init shellWindow Currently active thread ID: " << QThread::currentThreadId() << "(" << QThread::currentThread() << ")";

    shellWindow = shell;

    // Show the shell
    shellWindow->show();
    shellWindow->setWindowTitle("Debug Monitor");

    // Instance of etal interface
#if defined (CONFIG_USE_ETAL)
    m_mngr = new ETAL_globalList_mngr();
#endif // #if defined (CONFIG_USE_ETAL)

#if defined (CONFIG_USE_STANDALONE)
    m_mngr = new MW_globalList_mngr();
#endif // #if defined (CONFIG_USE_ETAL)

    m_stopping = false;
#if defined (CONFIG_USE_ETAL)
    m_fm_fe = RADIO_MNGR_ETAL_BG_FM_PATH;
    m_dab_fe = RADIO_MNGR_ETAL_BG_DAB_PATH;
#endif // #if defined(CONFIG_USE_ETAL)

#if defined (CONFIG_USE_STANDALONE)
    m_fm_fe = FE_HANDLE_3;
    m_dab_fe = FE_HANDLE_3;
#endif // #if defined(CONFIG_USE_STANDALONE)

    QObject::connect(this, &StationListManager::terminated, this, &StationListManager::terminate);
#if defined (CONFIG_USE_ETAL)
    QObject::connect(m_mngr, &ETAL_globalList_mngr::message, shellWindow, &ShellWindow::inShellEtalData_slot);
#endif // #if defined(CONFIG_USE_ETAL)

    QObject::connect(schedulingTimer, &QTimer::timeout, this, &StationListManager::onSchedule);
}

StationListManager::~StationListManager()
{
#if (defined (CONFIG_USE_ETAL) || defined (CONFIG_USE_STANDALONE))
    delete m_mngr;
#endif // #if (defined(CONFIG_USE_ETAL) || defined(CONFIG_USE_STANDALONE))

    delete schedulingTimer;
}

void StationListManager::onSchedule()
{
    //qDebug() << "Executed onSchedule()";

    m_lock.lock();

#if (defined (CONFIG_USE_ETAL) || defined (CONFIG_USE_STANDALONE))
    if (radioStatusGlobal->radioStatus.persistentData.globalServiceListEn)
    {
        //qDebug() << "Scan selected bands ... ";

        // enable and configure etal layer
#if (defined CONFIG_USE_ETAL)
        m_mngr->on(m_enable_fm, m_enable_dab, RADIO_MNGR_ETAL_BG_FM_PATH, RADIO_MNGR_ETAL_BG_DAB_PATH);
#endif // #if (defined CONFIG_USE_ETAL)

#if (defined CONFIG_USE_STANDALONE)
        m_mngr->on(m_enable_fm, m_enable_dab, FE_HANDLE_4, FE_HANDLE_4);
#endif // #if (defined CONFIG_USE_STANDALONE)

        // learn FM band
        ServiceListTy fmlist = m_mngr->handlerFM();

        // data to fm service list
        foreach(ServiceTy item, fmlist.serviceList)
        {
            // ignore stations without PI
            if (0 != item.serviceUniqueId)
            {
                if (item.ServiceLabel.isEmpty())
                {
                    item.ServiceLabel = QString::number(item.frequency / 1000., 'f', 2);
                }

                qDebug() << "Adding service " << item.ServiceLabel << " PI " <<  item.ServiceID;

                // add service to fm staioin list if not already there
                if (false == stationList->CheckIfServiceIsPresentInFm(item.serviceUniqueId))
                {
                    bool ret;

                    // stationList->stationListFm->InsertFmService(item.ps, item.pi, item.freq, item.preset);
                    //  !TODO check band param
                    ret = stationList->stationListFm->InsertFmService(item.ServiceLabel.toStdString(), item.serviceUniqueId, item.frequency, 0);

                    Q_UNUSED(ret);
                    // qDebug() << "\t" << ((true == ret) ? "done" : "failed");
                }
            }
        }

        if (m_stopping)
        {
            qDebug() << "timer is off leaving scheduler";
            m_mngr->off();
            m_lock.unlock();
            emit terminated();
            return;
        }

        // learn DAB
        ServiceListTy dablist = m_mngr->handlerDAB();

        foreach(ServiceTy item, dablist.serviceList)
        {
            qDebug() << "Adding service " << item.ServiceLabel << "from " << "TO BE ADDED"; // item.EnsChLabel;

            if (false == stationList->CheckIfServiceIsPresentInDab(item.serviceUniqueId, 0)) // !TODO check this item.ensembleUniqueId))
            {
                bool ret;

                // Insert a new entry in the service list
                ret = stationList->stationListDab->InsertDabService(item.ServiceLabel.toStdString(),
                                                                    item.serviceUniqueId,
                                                                    item.frequency,
                                                                    0,
                                                                    0, // !TODO item.ensembleUniqueId,
                                                                    " ", // !TODO item.EnsChLabel.toStdString(),
                                                                    -1);

                Q_UNUSED(ret);
                // qDebug() << "\t" << ((true == ret) ? "done" : "failed");
            }
        }

        // off
        m_mngr->off();

        if (m_stopping)
        {
            qDebug() << "timer is off leaving scheduler";

            emit terminated();
        }
    }

    if (!m_stopping)
    {
        schedulingTimer->setInterval(STATION_LIST_MNGR_START_DELAY);
    }
#endif // #if (defined(CONFIG_USE_ETAL) || defined(CONFIG_USE_STANDALONE))

    m_lock.unlock();
}

void StationListManager::process()
{
    // Welcome message
    qDebug() << "Station list manager started";

#if 0
    GlobalStationListTest();
#else
    // Load data for FM
    ifstream inFileFm("fm.bin", ios::in | ios::binary);

    if (inFileFm.is_open())
    {
        inFileFm >> *stationList->stationListFm;
    }

    std::list<RadioServiceFm>::iterator itFm;

#if 0
    qDebug() << "Loaded AMFM services:";

    // Print available services for FM
    for (itFm = stationList->stationListFm->begin(); itFm != stationList->stationListFm->end(); itFm++)
    {
        qDebug() << "Service (AM FM) : " << QString::fromStdString(itFm->GetServiceName()) \
                 << " - PI : "   << QString("%1").arg(itFm->GetId(), 0, 16) \
                 << " - freq : " << itFm->GetFrequency();
    }
#endif

    // Load data for DAB
    ifstream inFileDab("dab.bin", ios::in | ios::binary);

    if (inFileDab.is_open())
    {
        inFileDab >> *stationList->stationListDab;
    }

    std::list<RadioServiceDab>::iterator itDab;

    qDebug() << "Loaded DAB services:";

    //    // Print available services for DAB
    //    for (itDab = stationList->stationListDab->begin (); itDab != stationList->stationListDab->end (); itDab++)
    //    {
    //        qDebug() << "Service (DAB) : " <<  QString::fromStdString(itDab->GetServiceName()) << " - Ensemble : " <<
    // QString::fromStdString(itDab->GetEnsembleName());
    //        cout << "Service (DAB) : " << itDab->GetServiceName() << " - Ensemble : " << itDab->GetEnsembleName() << endl;
    //    }

    // Thread scheduling period (ms)
    // 120000ms = 120s = 2 minutes
    schedulingTimer->setInterval(STATION_LIST_MNGR_START_DELAY);
    schedulingTimer->start();
#endif
}

void StationListManager::terminate()
{
    qDebug() << "Station list manager saving ... ";
    // Save data from the FM list if any
    if (stationList->stationListFm->GetNumberOfServices())
    {
        ofstream outFileFm("fm.bin", ios::out | ios::binary);
        std::list<RadioServiceFm>::iterator itFm;

        qDebug() << "Saved AMFM services:";

        // Print available services for FM
        //        for (itFm = stationList->stationListFm->begin (); itFm != stationList->stationListFm->end (); itFm++)
        //        {
        //            qDebug() << "Service (AM FM) : " << QString::fromStdString(itFm->GetServiceName()) << " - freq : " << itFm->GetFrequency();
        //        }

        outFileFm << *stationList->stationListFm;
        outFileFm.close();
    }

    m_stopping = false;

    // Save data from the DAB list if any
    if (stationList->stationListDab->GetNumberOfServices())
    {
        ofstream outFileDab("dab.bin", ios::out | ios::binary);
        std::list<RadioServiceDab>::iterator itDab;

        qDebug() << "Saved DAB services:";

        // Print available services for DAB
        //        for (itDab = stationList->stationListDab->begin (); itDab != stationList->stationListDab->end (); itDab++)
        //        {
        //            qDebug() << "Service (DAB) : " <<  QString::fromStdString(itDab->GetServiceName()) << " - Ensemble : " <<
        // QString::fromStdString(itDab->GetEnsembleName());
        //        }

        outFileDab << *stationList->stationListDab;
        outFileDab.close();
    }

    if (EVENT_POWER_DOWN == m_event)
    {
        // do not emit any event
        m_event = EVENT_NONE;
    }
    else if (EVENT_SOURCE == m_event)
    {
        m_event = EVENT_NONE;
        emit eventToRadioManager(EVENT_SOURCE);
    }
    else
    {
        // emit finished to destroy global list manager and thread
        m_event = EVENT_NONE;
        emit finished();
    }
}

void StationListManager::stop()
{
    qDebug() << "Station list manager stopping";
    schedulingTimer->stop();
    m_stopping = true;
#if (defined CONFIG_USE_ETAL)
    m_mngr->term();
#endif // #if (defined CONFIG_USE_ETAL)
}

#if (defined CONFIG_USE_ETAL)
void StationListManager::seek_gl(void* state)
{
    m_mngr->seekHandler(state);
}

#endif // #if (defined CONFIG_USE_ETAL)

void StationListManager::eventFromHmiProlog(Events event, quint32 band)
{
    qDebug() << "StationListManager::eventFromHmiProlog event " << event << " band " << band;

    switch (event)
    {
        case EVENT_SOURCE:
        case EVENT_POWER_DOWN:
            // here we need to terminate main loop
            stop();
            m_event = event;

            // if lock possible call terminate directlly
            // since it will not be done automatically
            if (m_lock.tryLock())
            {
                terminate();
                m_lock.unlock();
            }

            break;

        default:
            break;
    }
}

void StationListManager::eventFromHmiEpilog(Events event, quint32 band)
{
    qDebug() << "StationListManager::eventFromHmiEpilog event " << event << " band " << band;
#if 1
    if (band == BAND_AM)
    {
        m_enable_fm = true; m_enable_dab = true;
    }

    if (band == BAND_FM)
    {
        m_enable_fm = false; m_enable_dab = true;
    }

    if (band == BAND_DAB3)
    {
        m_enable_fm = true; m_enable_dab = false;
    }
#else
    // all disabled in this branch
    if (band == BAND_AM)
    {
        m_enable_fm = false; m_enable_dab = false;
    }

    if (band == BAND_FM)
    {
        m_enable_fm = false; m_enable_dab = false;
    }

    if (band == BAND_DAB3)
    {
        m_enable_fm = false; m_enable_dab = false;
    }
#endif

    switch (event)
    {
        case EVENT_POWER_UP:
        case EVENT_SOURCE:
            qDebug() << "Station list start timer ";
            process(); // start timer
            break;

        default:
            break;
    }
}

void StationListManager::GlobalStationListTest()
{
    if (false == globalStationTestExecuted)
    {
        //EnsembleTableTy ensTable;
        ServiceListTy  listService;
        // Signal that the test has been executed
        globalStationTestExecuted = true;

        // Execute test
        stationList->Test();

        qDebug() << "GlobalStationListTest() executed";

        UpdateGlobalStationList(listService);
    }
}

void StationListManager::UpdateGlobalStationList(const ServiceListTy  listService)
{
    if (false != listService.serviceList.isEmpty())
    {
        ServiceTy service = listService.serviceList.at(0);
        //        bool ok;
        //        unsigned int ensembleUniqueId, serviceUniqueId;
        //        QString tmpEnsEcc = inputEnsembleTable.EnsECCID;

        //        // Retrieve the ensemble ID as number
        //        ensembleUniqueId = tmpEnsEcc.remove(" ").toUInt(&ok, 16);

        //        if (false == ok)
        //        {
        //            // Parsing failed, handle error here
        //            qDebug() << "ERROR: StationListUpdater ensemble id conversion failed";
        //        }

        //        // Retrieve the service ID as number
        //        serviceUniqueId = service.ServiceID.remove(" ").toUInt(&ok, 16);

        //        if (false == ok)
        //        {
        //            // Parsing failed, handle error here
        //            qDebug() << "ERROR: StationListUpdater service id conversion failed";
        //        }

        //        stationList->CheckIfServiceIsPresentInDab(serviceUniqueId, ensembleUniqueId);

        // Insert new data

        // Mark services no more present but do not remove them
    }
}
