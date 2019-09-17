// TODO: another hack, do it properly
#include <QDateTime>

#include "target_config.h"

#include "radiomanager.h"
#include "shellwindow.h"
#include "presets.h"

#define MAX_NB_PRESETS     6

namespace
{
    const int SERVICE_LIST_RETRY_NUM = 10;

    const unsigned int FM_MIN_FREQ = 64000;       //!< Absolute minimum frequency for FM worldwide
    const unsigned int FM_MAX_FREQ = 109000;      //!< Absolute maximum frequency for FM worldwide
    const unsigned int AM_MIN_FREQ = 522;         //!< Absolute minimum frequency for AM worldwide
    const unsigned int AM_MAX_FREQ = 30000;       //!< Absolute maximum frequency for AM worldwide
    const unsigned int DAB_MIN_FREQ = 168160;     //!< Absolute maximum frequency for FM worldwide
    const unsigned int DAB_MAX_FREQ = 239200;     //!< Absolute maximum frequency for AM worldwide

    const unsigned int FM_DEFAULT_FREQ = 87500;
    const unsigned int AM_DEFAULT_FREQ = 900;
    const unsigned int DAB_DEFAULT_FREQ = 225648;

    const unsigned int SIGNATURE = 0x80AA5501;    //!< Value used to recognize a valid settings file

#if (defined CONFIG_USE_STANDALONE)
    qint64 CurrentEpochTimeMs(qint64 latest)
    {
        qint64 deltaFromLatestEvent;
        QDateTime now = QDateTime::currentDateTime();

        deltaFromLatestEvent = now.toMSecsSinceEpoch() - latest;

        return deltaFromLatestEvent;
    }
#endif // #if (defined CONFIG_USE_STANDALONE)
}

// RadioManager::RadioManager (QObject *parent) : QObject(parent), eventOngoing(false),
//                                               tcpCmostTLayer(nullptr), tcpDcopTLayer(nullptr), cmdCis(nullptr), cmdMw(nullptr)
RadioManager::RadioManager(QObject* parent) : QObject(parent), eventOngoing(false)
{
    // Initializa global variables
    memset(&radioManagerStatus, 0, sizeof (radioManagerStatus));
    userEventPending = false;
    seekOngoing = false;

    // Get an instance of the preset list
    presetList = presetList->Instance();

    // Initialize a new shell for feedback
    shellWindow = new ShellWindow();

    // Show the shell
    shellWindow->show();
    shellWindow->setWindowTitle("Radio Shell Monitor");

    // Initialize event manager
    eventManager = new EventManager<eventsList, eventDataInterface> (this);

#if (defined CONFIG_USE_ETAL)
    m_etal = new Etal();
#endif // #if (defined CONFIG_USE_ETAL)

    // Init radio manager application
    RadioMngrApplicationInit();
}

RadioManager::~RadioManager()
{
#if (defined CONFIG_USE_STANDALONE)
    qDebug() << "Disconnect Sockets";

    // Send command to close Protocol Layer
    DisconnectSockets();

    // Disconnect from protocol layer
    tcpCmostTLayer->disconnect();
    tcpDcopTLayer->disconnect();

    delete protocolLayer;
    qDebug() << "Deleted: protocolLayer";

    // Delete members
    delete tcpCmostTLayer;
    delete tcpDcopTLayer;
    delete cmdMw;
    delete cmdCis;
    delete shellWindow;
    delete rdsDecoder;
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    delete m_etal;
    qDebug() << "Deleted: Etal";

    delete shellWindow;
    qDebug() << "Deleted: shellWindow";

    // Send command to close Protocol Layer
    qDebug() << "Disconnect Sockets";
    DisconnectSockets();

    delete protocolLayer;
    qDebug() << "Deleted: protocolLayer";
#endif // #if (defined CONFIG_USE_ETAL)
}

void RadioManager::RadioLastStatus()
{
    bool initDone = false;

    // Load Persistent data
    ifstream inFileRadioStatus("radiostatus.bin", ios::in | ios::binary);

    // Check if we have a valid file
    if (inFileRadioStatus)
    {
        // Load data
        inFileRadioStatus >> radioStorage;

        // Get loaded data and store in the global variable
        radioStatusGlobal->radioStatus.persistentData = radioStorage.GetStatus();

        if (SIGNATURE == radioStatusGlobal->radioStatus.persistentData.signature)
        {
            // The test screen always starts disabled, the user has to enable it each session
            radioStatusGlobal->radioStatus.persistentData.testScreenIsActive = false;

            // Initialize dynamic data
            radioStatusGlobal->radioStatus.dynamicData.radioMute = true;

            if (radioStatusGlobal->radioStatus.persistentData.radioVolume > VOLUME_MAX_VALUE)
            {
                radioStatusGlobal->radioStatus.persistentData.radioVolume = VOLUME_MAX_VALUE;
            }

            // Sanity check on frequencies
            if (radioStatusGlobal->radioStatus.persistentData.frequencyFm <= FM_MIN_FREQ ||
                radioStatusGlobal->radioStatus.persistentData.frequencyFm >= FM_MAX_FREQ)
            {
                radioStatusGlobal->radioStatus.persistentData.frequencyFm = FM_DEFAULT_FREQ;
            }

            if (radioStatusGlobal->radioStatus.persistentData.frequencyAm <= AM_MIN_FREQ ||
                radioStatusGlobal->radioStatus.persistentData.frequencyAm >= AM_MAX_FREQ)
            {
                radioStatusGlobal->radioStatus.persistentData.frequencyAm = AM_DEFAULT_FREQ;
            }

            if (radioStatusGlobal->radioStatus.persistentData.frequencyDab <= DAB_MIN_FREQ ||
                radioStatusGlobal->radioStatus.persistentData.frequencyDab >= DAB_MAX_FREQ)
            {
                radioStatusGlobal->radioStatus.persistentData.frequencyDab = DAB_DEFAULT_FREQ;
            }

            initDone = true;
        }
    }

    // If we did not init properly we default to some predefined value
    if (false == initDone)
    {
        // Set radio volume to a default initial value
        radioStatusGlobal->radioStatus.persistentData.radioVolume = VOLUME_MAX_VALUE / 2; // TODO: use the correct start value

        // Dafault values
        radioStatusGlobal->radioStatus.persistentData.band = BAND_DAB3;
        radioStatusGlobal->radioStatus.persistentData.slsDisplayEn = true;
        radioStatusGlobal->radioStatus.persistentData.afCheckEn = false;
        radioStatusGlobal->radioStatus.persistentData.dabFmServiceFollowingEn = false;
        radioStatusGlobal->radioStatus.persistentData.dabDabServiceFollowingEn = false;
        radioStatusGlobal->radioStatus.persistentData.rdsTaEnabled = false;
        radioStatusGlobal->radioStatus.persistentData.rdsRegionalEn = false;
        radioStatusGlobal->radioStatus.persistentData.rdsEonEn = false;
        radioStatusGlobal->radioStatus.persistentData.globalServiceListEn = true;
        radioStatusGlobal->radioStatus.persistentData.displayUnsupportedServiceEn = false;
        radioStatusGlobal->radioStatus.persistentData.alphabeticOrder = false;

        radioStatusGlobal->radioStatus.persistentData.signature = SIGNATURE;
        radioStatusGlobal->radioStatus.persistentData.ptySelected = 0; // Set to 0 means using no PTY

        // Default initialization DAB
        radioStatusGlobal->radioStatus.persistentData.radioMode = MODE_DAB;
        radioStatusGlobal->radioStatus.persistentData.band = BAND_DAB3;
        radioStatusGlobal->radioStatus.persistentData.serviceId = 0x232f;

        radioStatusGlobal->radioStatus.persistentData.frequency = 227360;
        radioStatusGlobal->radioStatus.persistentData.frequencyDab = 227360;
        radioStatusGlobal->radioStatus.persistentData.frequencyFm = 91900;
        radioStatusGlobal->radioStatus.persistentData.frequencyAm = 639;

        radioStatusGlobal->radioStatus.dynamicData.radioMute = true;
        radioStatusGlobal->radioStatus.dynamicData.radioState = STOPPED_STATE;
    }

#if (defined CONFIG_USE_ETAL)
    // TODO: In Eal case it shall be retireved from Etal API
    radioStatusGlobal->radioStatus.persistentData.country = COUNTRY_EU;
#endif // #if (defined CONFIG_USE_ETAL)

#if (defined CONFIG_USE_STANDALONE)
    radioStatusGlobal->radioStatus.persistentData.country = (CountryTy)cmdCis->getCmostCountry();
#endif // #if (defined CONFIG_USE_STANDALONE)

    // Load Presets data
    ifstream inFilePresetsTable("presets.bin", ios::in | ios::binary);

    // Check if we have a valid file
    if (inFilePresetsTable)
    {
        // Load Presets spreviously stored data
        inFilePresetsTable >> *presetList;
    }
    else
    {
        ClearAllPresets();
    }
}

StatusCodeTy RadioManager::ClearAllPresets()
{
    StatusCodeTy retValue = CMD_OK;

    qDebug() << "Radio Manager Clear All Presets";

    for (int cnt = 0; cnt < MAX_NB_PRESETS; cnt++)
    {
        presetList->SetPreset(cnt, "Empty", INVALID_SERVICE_ID, INVALID_FREQUENCY, INVALID_BAND);
    }

    return retValue;
}

StatusCodeTy RadioManager::BootOn()
{
    StatusCodeTy retValue = CMD_OK;

    // Set radio status
    radioStatusGlobal->radioStatus.dynamicData.radioState = STOPPED_STATE;

#if (defined CONFIG_USE_STANDALONE)
    // Reset the device
    retValue = cmdCis->cmostDeviceReset();

    if (retValue != CMD_OK)
    {
        return retValue;
    }

    QThread::msleep(200);

    retValue = cmdCis->cmostBoot();

    if (retValue != CMD_OK)
    {
        return retValue;
    }

    retValue = cmdCis->cmostPowerOn();

    if (retValue != CMD_OK)
    {
        return retValue;
    }

    QThread::msleep(2000);

    if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_GET_ALIVE_CHK, false))
    {
        retValue = GENERIC_FAILURE;
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    qDebug() << Q_FUNC_INFO;

    QThread::msleep(1000);

    m_etal->boot();

    retValue = (true == m_etal->booted()) ? CMD_OK : CONNECTION_ERRORS;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

#if (defined CONFIG_USE_STANDALONE)
void RadioManager::initNotSelectedSource()
{
    StatusCodeTy retValue = CMD_OK;
    unsigned int tmpBand, tmpFrequency;
    SystemMode useMode;

    retValue = cmdCis->initNotSelectedBand(radioStatusGlobal->radioStatus.persistentData.band);

    // if Band not active source is Dab
    // Invalidate ServiceTable
    radioStatusGlobal->panelData.list.serviceList.clear();

    if (CMD_OK == retValue)
    {
        if (BAND_DAB3 != radioStatusGlobal->radioStatus.persistentData.band)
        {
            retValue = SetDcopFrequency(0);
        }
    }

    if (CMD_OK == retValue)
    {
        // TODO Clear Rds Data

        // Tune the TUNER to the new frequency
        if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
        {
            retValue = cmdCis->setFrequency(radioStatusGlobal->radioStatus.persistentData.frequencyFm, MODE_FM);
            useMode = MODE_FM;
            tmpBand = BAND_FM;
            tmpFrequency = radioStatusGlobal->radioStatus.persistentData.frequencyFm;
        }
        else
        {
            retValue = cmdCis->setFrequency(radioStatusGlobal->radioStatus.persistentData.frequencyDab, MODE_DAB);
            useMode = MODE_DAB;
            tmpBand = BAND_DAB3;
            tmpFrequency = radioStatusGlobal->radioStatus.persistentData.frequencyDab;
        }

        cmdCis->setCurrentFrequency(useMode, &tmpBand, &tmpFrequency);

        // TODO run rds acquisition

        if (CMD_OK == retValue)
        {
            // In case DAB is not selected source
            if (BAND_DAB3 != radioStatusGlobal->radioStatus.persistentData.band)
            {
                // Set Dcop Dab frequency
                retValue = SetDcopFrequency(radioStatusGlobal->radioStatus.persistentData.frequencyDab);
            }
        }
    }
}

#endif // #if (defined CONFIG_USE_STANDALONE)

StatusCodeTy RadioManager::PowerOn()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    // If User case BOARD_MTD_2_CHANNELS then set band/freq of both tuners
    if (true == cmdCis->isUseModeBOARD_MTD_2_CHANNELS())
    {
        // Set Band/Frequency of not active source
        initNotSelectedSource();
    }

    // Set band
    SetBand();

    if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
    {
        // Start DCOP sending events
        StartEvents();
    }

    // On power-up we wait a little bit
    QThread::msleep(300);

    // Execute a tune sequence
    retValue = NewTuneSequence(radioStatusGlobal->radioStatus.persistentData.frequency,
                               radioStatusGlobal->radioStatus.persistentData.band,
                               radioStatusGlobal->radioStatus.persistentData.radioMode);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    bool pd = false;

#if (defined RADIO_MNGR_ETAL_FG_FM_PD)
    pd = true;
#endif // #if (defined CONFIG_BOARD_DUAL_TUNER)

    m_etal->setPhaseDiversity(pd);

    // conf. for all supported modes
    m_cont[FM].m_band = B_FM;
    m_cont[FM].m_fend = RADIO_MNGR_ETAL_FG_FM_PATH;

    m_cont[AM].m_band = B_AM;
    m_cont[AM].m_fend = RADIO_MNGR_ETAL_FG_AM_PATH;

    m_cont[DAB].m_band = B_DAB3;
    m_cont[DAB].m_fend = RADIO_MNGR_ETAL_FG_DAB_PATH;

    switch (radioStatusGlobal->radioStatus.persistentData.radioMode)
    {
        case MODE_DAB:
        {
            m_etal->setContent(&m_cont[DAB]);
        }
        break;

        case MODE_AM:
        {
            m_etal->setContent(&m_cont[AM]);
        }
        break;

        case MODE_FM:
        default:
        {
            m_etal->setContent(&m_cont[FM]);
        }
        break;
    }

    NewTuneSequence(radioStatusGlobal->radioStatus.persistentData.frequency,
                    radioStatusGlobal->radioStatus.persistentData.band,
                    radioStatusGlobal->radioStatus.persistentData.radioMode);
#endif

#if (defined CONFIG_USE_STANDALONE)
    cmdCis->cisSetVolume(radioStatusGlobal->radioStatus.persistentData.radioVolume,
                         radioStatusGlobal->radioStatus.persistentData.radioMode);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    m_etal->setVolume(radioStatusGlobal->radioStatus.persistentData.radioVolume);
#endif // #if (defined CONFIG_USE_ETAL)

    // We need to force an unmute, the toggle ogf teh global is done internally
    // so here we initialize the global value to mute status
    radioStatusGlobal->radioStatus.dynamicData.radioMute = true;

    // Call the mute toggle function
    if (CMD_OK != MuteToggle())
    {
        qDebug() << "ERROR: mute command error";
    }

    // The radio is now in active state (if we had error we still set active state)
    radioStatusGlobal->radioStatus.dynamicData.radioState = ACTIVE_STATE;

    return retValue;
}

StatusCodeTy RadioManager::PowerOff()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    if (BAND_FM == radioStatusGlobal->radioStatus.persistentData.band)
    {
        // Stop RDS
        emit EventFromDeviceSignal(EVENT_STOP_RDS);
    }

    if (BAND_FM == radioStatusGlobal->radioStatus.persistentData.band)
    {
        radioStatusGlobal->radioStatus.persistentData.frequencyFm =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }
    else if (BAND_AM == radioStatusGlobal->radioStatus.persistentData.band)
    {
        radioStatusGlobal->radioStatus.persistentData.frequencyAm =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }
    else if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
    {
        radioStatusGlobal->radioStatus.persistentData.frequencyDab =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

    GetDabQualInfo(false);

    // Stop state machine to request for Quality SNR
    emit EventFromDeviceSignal(EVENT_STOP_QUALITY_SNR);

#if (defined CONFIG_USE_STANDALONE)
    // Mute the radio
    if (false == radioStatusGlobal->radioStatus.dynamicData.radioMute)
    {
        retValue = MuteToggle();

        if (CMD_OK != retValue)
        {
            // TODO: Manage error
        }
    }

    StopEvents();

    // PowerOff --> Tuner in StandBy
    retValue = cmdCis->cmostPowerOff();

    if (CMD_OK != retValue)
    {
        // TODO: Manage error
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    m_etal->powerOff();
#endif // #if (defined CONFIG_USE_ETAL)

    if (BAND_FM == radioStatusGlobal->radioStatus.persistentData.band)
    {
        radioStatusGlobal->radioStatus.persistentData.frequencyFm =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }
    else if (BAND_AM == radioStatusGlobal->radioStatus.persistentData.band)
    {
        radioStatusGlobal->radioStatus.persistentData.frequencyAm =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }
    else if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
    {
        radioStatusGlobal->radioStatus.persistentData.frequencyDab =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }

    // Save Last Radio status, copy ensemble and service names
    if (false == radioStatusGlobal->panelData.ensembleTable.EnsChLabel.isEmpty())
    {
        memcpy(radioStatusGlobal->radioStatus.persistentData.ensembleName,
               radioStatusGlobal->panelData.ensembleTable.EnsChLabel.toStdString().c_str(),
               ENSEMBLE_NAME_LEN);
    }
    else
    {
        memset(radioStatusGlobal->radioStatus.persistentData.ensembleName, 0, STATION_NAME_LEN);
    }

    if (false == radioStatusGlobal->panelData.list.serviceList.isEmpty())
    {
        if (false == radioStatusGlobal->panelData.list.serviceList[radioStatusGlobal->panelData.serviceIndex].ServiceLabel.isEmpty())
        {
            memcpy(radioStatusGlobal->radioStatus.persistentData.serviceName,
                   radioStatusGlobal->panelData.list.serviceList[radioStatusGlobal->panelData.serviceIndex].ServiceLabel.toStdString().c_str(),
                   STATION_NAME_LEN);
        }
        else
        {
            memset(radioStatusGlobal->radioStatus.persistentData.serviceName, 0, STATION_NAME_LEN);
        }
    }
    else
    {
        memset(radioStatusGlobal->radioStatus.persistentData.serviceName, 0, STATION_NAME_LEN);
    }

    // Save Persistent data
    radioStorage.SetStatus(radioStatusGlobal->radioStatus.persistentData);

    ofstream outFileRadioStatus("radiostatus.bin", ios::out | ios::binary);
    outFileRadioStatus << radioStorage;

    // Save Presets data and clear memory content, 0 services shall be reported
    ofstream outFilePresets("presets.bin", ios::out | ios::binary);

    outFilePresets << *presetList;

    outFilePresets.close();

    qDebug() << "Radio is off";

    return retValue;
}

void RadioManager::StopEvents()
{
    // Stop DLS
    PadOnDlsStop();

    // Stop SLS
    SlsFromXpadStop();

    // Stop events
    EventsNotificationStop();
}

void RadioManager::StartEvents()
{
    // Start events
    EventsNotificationStart();

    // Start DLS
    PadOnDlsStart();

    // Start SLS
    SlsFromXpadStart();
}

void RadioManager::SetBand()
{
#if (defined CONFIG_USE_STANDALONE)
    cmdCis->setBand(radioStatusGlobal->radioStatus.persistentData.band,
                    radioStatusGlobal->radioStatus.persistentData.radioMode);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    switch (radioStatusGlobal->radioStatus.persistentData.band)
    {
        case BAND_AM:
        {
            m_etal->setContent(&m_cont[AM]);
        }
        break;

        case BAND_DAB3:
        {
            m_etal->setContent(&m_cont[DAB]);
        }
        break;

        case BAND_FM:
        default:
        {
            m_etal->setContent(&m_cont[FM]);
        }
        break;
    }
#endif // #if (defined CONFIG_USE_ETAL)
}

StatusCodeTy RadioManager::SetDcopFrequency(quint32 currFreq)
{
#if (defined CONFIG_USE_STANDALONE)
    // Check if we are tuning to 0 or a valid frequency
    if (currFreq != 0)
    {
        cmdMw->SetActiveFrequency(currFreq);

        cmdMw->SetCurrServiceId(INVALID_SERVICE_ID);
    }

    // Execute command
    if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_TUNE_FREQUENCY, true))
    {
        return GENERIC_FAILURE;
    }

    return CMD_OK;
#else
    Q_UNUSED(currFreq);

    return UNAVAILABLE_FUNCTIONALITY;
#endif // #if (defined CONFIG_USE_STANDALONE)
}

StatusCodeTy RadioManager::SetFrequency(unsigned int newFreq, BandTy newBand, SystemMode newMode)
{
    StatusCodeTy retValue = CMD_OK;

    qDebug() << "--- RM tune command     ---> " << QString::number(newFreq) << " <---";

#if (defined CONFIG_USE_STANDALONE)
    unsigned int curFreq;
    unsigned int curBand;

    // When we tune to a new frequency we need to invalidate some data
    radioStatusGlobal->panelData.list.serviceList.clear();

    retValue = cmdCis->getCurrentFrequency(newMode, &curFreq);

    if (CMD_OK == retValue)
    {
        if (curFreq != newFreq)
        {
            // In case of DAB we need to tell the DCOP that we are going to change frequency,
            // we do that tuning to 0
            if (BAND_DAB3 == newBand)
            {
                retValue = SetDcopFrequency(0);
            }

            if (CMD_OK == retValue)
            {
                if (BAND_FM == newBand)
                {
                    rdsDecoder->ClearRdsData();
                }

                // Tune the TUNER to the new frequency
                retValue = cmdCis->setFrequency(newFreq, newMode);

                if (BAND_FM == newBand)
                {
                    rdsDecoder->setRdsFmTuneFrequency(newFreq);
                    cmdCis->RunRdsAcquisition(newMode);
                }

                // In case of DAB we tune to the new frequency
                if (BAND_DAB3 == newBand)
                {
                    if (CMD_OK == retValue)
                    {
                        retValue = SetDcopFrequency(newFreq);
                    }
                }
            }

            curBand = static_cast<unsigned int> (newBand);
            curFreq = newFreq;

            cmdCis->setCurrentFrequency(newMode, &curBand, &curFreq);
        }
    }

    if (CMD_OK != retValue)
    {
        qDebug() << "ERROR: set frequency failed";
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    Q_UNUSED(newMode);
    // TODO type align

    switch (newBand)
    {
        case BAND_AM:
        {   m_etal->setBand(B_AM); }
        break;

        case BAND_DAB3:
        {   m_etal->setBand(B_DAB3); }
        break;

        case BAND_FM:
        default:
        {   m_etal->setBand(B_FM); }
        break;
    }

    m_etal->setFrequency(newFreq);
#endif  // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::DcopCheckAlive()
{
#if (defined CONFIG_USE_STANDALONE)
    StatusCodeTy retValue = CMD_OK;

    // Execute command
    if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_GET_ALIVE_CHK, false))
    {
        retValue = GENERIC_FAILURE;
    }

    return retValue;
#else
    return UNAVAILABLE_FUNCTIONALITY;
#endif // #if (defined CONFIG_USE_STANDALONE)
}

StatusCodeTy RadioManager::GetEnsembleList()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    // Execute command
    if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_GET_ENSEMBLE_LIST, true))
#endif // #if (defined CONFIG_USE_STANDALONE)
#if (defined CONFIG_USE_ETAL)
    if (true != m_etal->isEnsAvailable())
#endif // #if (defined CONFIG_USE_ETAL)
    {
        retValue = GENERIC_FAILURE;
    }

    return retValue;
}

StatusCodeTy RadioManager::GetEnsembleData()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    // Execute command
    if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_GET_ENSEMBLE_DATA, true))
#endif // #if (defined CONFIG_USE_STANDALONE)
#if (defined CONFIG_USE_ETAL)
    if (true != m_etal->getEnsembleInfo())
#endif
    {
        retValue = GENERIC_FAILURE;
    }

    return retValue;
}

StatusCodeTy RadioManager::GetServiceList()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    // Execute command
    if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_GET_SERVICE_LIST, true))
#endif // #if (defined CONFIG_USE_STANDALONE)
#if (defined CONFIG_USE_ETAL)
    //if (true != m_etal->getServiceListInfo)
    if (false)
#endif // #if (defined CONFIG_USE_ETAL)
    {
        retValue = GENERIC_FAILURE;
    }

    return retValue;
}

StatusCodeTy RadioManager::GetSpecificServiceData()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    // Execute command
    if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_GET_SPECIFIC_SERVICE_DATA, true))
#endif // #if (defined CONFIG_USE_STANDALONE)
#if (defined CONFIG_USE_ETAL)
    if (true != m_etal->getServiceListInfo())
#endif // #if (defined CONFIG_USE_ETAL)
    {
        retValue = GENERIC_FAILURE;
    }

    //#if (defined CONFIG_USE_ETAL)
    //    radioStatusGlobal->panelData.ensembleTable   =   m_etal->getEnsTable();
    //    radioStatusGlobal->panelData.list            =   m_etal->getServices();

    //    return COMMAND_COMPLETED_CORRECTLY;
    //#endif // #if (define CONFIG_USE_ETAL)

    return retValue;
}

void RadioManager::UpdateGlobalStructures(bool onSuccess)
{
    if (true == onSuccess)
    {
        int validStations = 0;
#if (defined CONFIG_USE_STANDALONE)
        // Save current ensemble index
        radioStatusGlobal->panelData.ensembleIndex = cmdMw->GetCurrEnsembleIndex();

        // Copy current esemble table inside the global structure
        radioStatusGlobal->panelData.ensembleTable = cmdMw->GetCurrEnsembleTable();

        // Copy current service list
        radioStatusGlobal->panelData.list = cmdMw->GetCurrServiceList();
#endif
#if (defined CONFIG_USE_ETAL)
        // Save current ensemble index
        // radioStatusGlobal->panelData.ensembleIndex = getDab3IndexFromFrequency();

        // Copy current esemble table inside the global structure
        radioStatusGlobal->panelData.ensembleTable = m_etal->getEnsTable();

        // Copy current service list
        radioStatusGlobal->panelData.list = m_etal->getServices();
#endif

        // Get the number of new services stored and check if we can consider the list confirmed
        int numberOfRetrievedServices = radioStatusGlobal->panelData.list.serviceList.size();

        // To declare a list confirmed we need to have:
        // - Same number of stations twice
        // - All labels shall be present (this is dangerous so shall be "relazed" in some way,
        //   maybe creating a fake label)
        if (numberOfRetrievedServices == radioManagerStatus.numberOfAlreadyStoredServices &&
            radioManagerStatus.numberOfAlreadyStoredServices > 0)
        {
            // Calculate elements on screen (considering valid labels)
            QListIterator<ServiceTy> itValidStations(radioStatusGlobal->panelData.list.serviceList);

            while (itValidStations.hasNext())
            {
                ServiceTy tmp = itValidStations.next();

                if (false == tmp.ServiceLabel.isEmpty())
                {
                    validStations++;
                }
            }

            if (validStations == numberOfRetrievedServices)
            {
                qDebug() << "SERVICE LIST confirmed (1)";

                radioManagerStatus.serviceListConfirmed = true;
            }
        }

        // In case service list is not confirmed increment number of trials
        if (false == radioManagerStatus.serviceListConfirmed)
        {
            // Increment number of re-try
            radioManagerStatus.numberOfRetry++;

            // If we reach a certain number of retry then give up
            if (radioManagerStatus.numberOfRetry >= 15)
            {
                qDebug() << "SERVICE LIST confirmed due to excessive iterations (1)";

                radioManagerStatus.serviceListConfirmed = true;
            }
        }

        // Update stored information
        radioManagerStatus.numberOfAlreadyStoredServices = numberOfRetrievedServices;
    }
    else
    {
        // We update data on command failure
        if (false == radioManagerStatus.serviceListConfirmed)
        {
            // Increment number of re-try
            radioManagerStatus.numberOfRetry++;

            // If we reach a certain number of retry then give up
            if (radioManagerStatus.numberOfRetry >= SERVICE_LIST_RETRY_NUM)
            {
                qDebug() << "SERVICE LIST confirmed due to excessive iterations (2)";

                radioManagerStatus.serviceListConfirmed = true;
            }
        }
    }
}

StatusCodeTy RadioManager::MuteToggle()
{
    StatusCodeTy retValue;
    bool newMuteStatus;

#if 0
    // For test only to be removed
    cmdMw->SetQualAcquireStatus(false);
    cmdMw->CommandExecute(MIDW_GET_DAB_INFO, true);
#endif

    newMuteStatus = !radioStatusGlobal->radioStatus.dynamicData.radioMute;

#if (defined CONFIG_USE_STANDALONE)
    if (true == newMuteStatus)
    {
        // Make mute
        retValue = cmdCis->cisUnMute(false, radioStatusGlobal->radioStatus.persistentData.radioMode);
    }
    else
    {
        // Make unmute
        retValue = cmdCis->cisUnMute(true, radioStatusGlobal->radioStatus.persistentData.radioMode);
    }

    if (CMD_OK == retValue)
    {
        // Update global variable
        cmdMw->SetMuteFlag(newMuteStatus);

        // Execute command
        if (RADIO_SEQUENCE_OK != cmdMw->CommandExecute(MIDW_MUTE, false))
        {
            retValue = GENERIC_FAILURE;
        }

        if (CMD_OK == retValue)
        {
            radioStatusGlobal->radioStatus.dynamicData.radioMute = newMuteStatus;
        }
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    m_etal->setMute(newMuteStatus);
    radioStatusGlobal->radioStatus.dynamicData.radioMute = newMuteStatus;
    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::ServiceSelect(unsigned int idValue)
{
    StatusCodeTy retValue = CMD_OK;

    // Calculate index from PI value if we got a valid one, otherwise use current one
    if (INVALID_SERVICE_ID != idValue)
    {
        // Calculate new service index
        int serviceIndex = getServiceIndex(idValue);

        if (serviceIndex >= 0)
        {
            // Save infromation in global variables
            radioStatusGlobal->radioStatus.persistentData.serviceId = idValue;
            radioStatusGlobal->panelData.serviceIndex = serviceIndex;
        }
        else
        {
            idValue = GetFirstServiceId();

            // Save infromation in global variables
            radioStatusGlobal->radioStatus.persistentData.serviceId = idValue;
            radioStatusGlobal->panelData.serviceIndex = 0;
        }
    }
    else
    {
        idValue = GetFirstServiceId();

        // Save infromation in global variables
        radioStatusGlobal->radioStatus.persistentData.serviceId = idValue;
        radioStatusGlobal->panelData.serviceIndex = 0;
    }

#if (defined CONFIG_USE_STANDALONE)
    RadioSequencesStatusTy retSeq;

    //cmdMw->SetCurrServiceIndex(radioStatusGlobal->panelData.serviceIndex);
    cmdMw->SetCurrServiceId(radioStatusGlobal->radioStatus.persistentData.serviceId);

    retSeq = cmdMw->CommandExecute(MIDW_SERVICE_SELECT, true);

    if (RADIO_SEQUENCE_OK != retSeq)
    {
        retValue = GENERIC_FAILURE;
    }
    else
    {
        ServiceTy service = radioStatusGlobal->panelData.list.serviceList.at(radioStatusGlobal->panelData.serviceIndex);

        radioStatusGlobal->radioStatus.persistentData.serviceId = service.serviceUniqueId;
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    // !TODO check return
    m_etal->serviceSelect(radioStatusGlobal->radioStatus.persistentData.serviceId);

    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::ServiceRemove()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    RadioSequencesStatusTy retSeq;

    cmdMw->SetServiceRemoveMode(true);
    retSeq = cmdMw->CommandExecute(MIDW_SERVICE_SELECT, true);
    cmdMw->SetServiceRemoveMode(false);

    if (RADIO_SEQUENCE_OK != retSeq)
    {
        retValue = GENERIC_FAILURE;
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    // TO DO
    // m_etal->serviceRemove(radioStatusGlobal->panelData.serviceIndex);

    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::EnableNotification(bool notifyEn)
{
#if (defined CONFIG_USE_STANDALONE)
    StatusCodeTy retValue = CMD_OK;

    RadioSequencesStatusTy retSeq;

    cmdMw->SetEventAutonotificationRunning(notifyEn);

    retSeq = cmdMw->CommandExecute(MIDW_SETUP_EVENT_NOTIFICATIONS, false);

    if (RADIO_SEQUENCE_OK != retSeq)
    {
        retValue = GENERIC_FAILURE;
    }

    return retValue;
#else
    Q_UNUSED(notifyEn);

    return UNAVAILABLE_FUNCTIONALITY;
#endif // #if (defined CONFIG_USE_STANDALONE)
}

StatusCodeTy RadioManager::EventsNotificationStop()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    retValue = EnableNotification(false);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    retValue = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::EventsNotificationStart()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    retValue = EnableNotification(true);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::setPadOnDls(bool dlsEn)
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    RadioSequencesStatusTy retSeq;

    // Check if it is a PAD start or STOP
    cmdMw->SelectPadOnDlsState(dlsEn);

    retSeq = cmdMw->CommandExecute(MIDW_GET_PAD_DLS, false);

    if (RADIO_SEQUENCE_OK != retSeq)
    {
        retValue = GENERIC_FAILURE;
    }
#else
    Q_UNUSED(dlsEn);
#endif // #if (defined CONFIG_USE_STANDALONE)

    return retValue;
}

StatusCodeTy RadioManager::PadOnDlsStop()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    retValue = setPadOnDls(false);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::PadOnDlsStart()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    retValue = setPadOnDls(true);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    // !TODO impelement DLS on/off
    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::setSlsFromXpad(bool slsEn)
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    RadioSequencesStatusTy retSeq;

    // Check if it is an SLS start or STOP
    cmdMw->SelectSlsOnXpadState(slsEn);

    if (true == slsEn)
    {
        retSeq = cmdMw->CommandExecute(MIDW_GET_MOT, false);
    }
    else
    {
        retSeq = cmdMw->CommandExecute(MIDW_STOP_MOT, false);
    }

    if (RADIO_SEQUENCE_OK != retSeq)
    {
        retValue = GENERIC_FAILURE;
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    // !TODO sls on/off
    Q_UNUSED(slsEn);
#endif

    return retValue;
}

StatusCodeTy RadioManager::SlsFromXpadStop()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    retValue = setSlsFromXpad(false);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    // !TODO sls on/off
    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::SlsFromXpadStart()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    retValue = setSlsFromXpad(true);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    // !TODO sls on/off
    retValue  = COMMAND_COMPLETED_CORRECTLY;
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

void RadioManager::SaveCurrentBandData(BandTy cBand)
{
    if (BAND_FM == cBand)
    {
        // Stop RDS
        emit EventFromDeviceSignal(EVENT_STOP_RDS);

        // Stop Quality SNR read
        emit EventFromDeviceSignal(EVENT_STOP_QUALITY_SNR);

        // Save last FM frequency
        radioStatusGlobal->radioStatus.persistentData.frequencyFm =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }
    else if (BAND_AM == cBand)
    {
        // Stop Quality SNR read
        emit EventFromDeviceSignal(EVENT_STOP_QUALITY_SNR);

        // Save last AM frequency
        radioStatusGlobal->radioStatus.persistentData.frequencyAm =
            radioStatusGlobal->radioStatus.persistentData.frequency;
    }
    else
    {
        GetDabQualInfo(false);

        // Stop DAB Events autonotifications
        StopEvents();

        // Stop Quality SNR read
        emit EventFromDeviceSignal(EVENT_STOP_QUALITY_SNR);

        // Save last DAB frequency
        radioStatusGlobal->radioStatus.persistentData.frequencyDab =
            radioStatusGlobal->radioStatus.persistentData.frequency;

        // Remove Service currently played
        if (true == radioManagerStatus.audioServiceIsPlaying)
        {
            // Remove the service
            ServiceRemove();

            // Update flag
            radioManagerStatus.audioServiceIsPlaying = false;

            // Update information to the HMI
            SendAudioPlaysUpdate(radioManagerStatus.audioServiceIsPlaying);
        }
    }
}

void RadioManager::SetNewBandData(BandTy cBand)
{
    if (BAND_AM == cBand)
    {
        // Retrieve last AM frequency
        radioStatusGlobal->radioStatus.persistentData.frequency =
            radioStatusGlobal->radioStatus.persistentData.frequencyAm;

        // We enter AM mode
        radioStatusGlobal->radioStatus.persistentData.band = BAND_AM;
        radioStatusGlobal->radioStatus.persistentData.radioMode = MODE_AM;
    }
    else if (BAND_DAB3 == cBand)
    {
        // Retrieve last DAB frequency
        radioStatusGlobal->radioStatus.persistentData.frequency =
            radioStatusGlobal->radioStatus.persistentData.frequencyDab;

        radioStatusGlobal->panelData.ensembleIndex = getDab3IndexFromFrequency(radioStatusGlobal->radioStatus.persistentData.frequency);

        // Store new frequency
        radioStatusGlobal->panelData.ensembleTable.ensFrequency =
            radioStatusGlobal->radioStatus.persistentData.frequency;

        // We enter DAB mode
        radioStatusGlobal->radioStatus.persistentData.band = BAND_DAB3;
        radioStatusGlobal->radioStatus.persistentData.radioMode = MODE_DAB;
    }
    else
    {
        // Retrieve last FM frequency
        radioStatusGlobal->radioStatus.persistentData.frequency =
            radioStatusGlobal->radioStatus.persistentData.frequencyFm;

        // We enter FM mode
        radioStatusGlobal->radioStatus.persistentData.band = BAND_FM;
        radioStatusGlobal->radioStatus.persistentData.radioMode = MODE_FM;
    }
}

StatusCodeTy RadioManager::Source(BandTy newBand)
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_ETAL)
    // Save current content
    m_cont[m_etal->mode()] = m_etal->getContent();
#endif // #if (defined CONFIG_USE_ETAL)

    SaveCurrentBandData(radioStatusGlobal->radioStatus.persistentData.band);

    radioStatusGlobal->radioStatus.persistentData.band = newBand;

    SetNewBandData(newBand);

    SetBand();

    if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
    {
        // Start DCOP sending events
        StartEvents();
    }

    retValue = NewTuneSequence(radioStatusGlobal->radioStatus.persistentData.frequency,
                               radioStatusGlobal->radioStatus.persistentData.band,
                               radioStatusGlobal->radioStatus.persistentData.radioMode);

    // Update global variables and inform the HMI about the new frequency and the band
    UpdateRadioStatusAndSendFeedback(radioStatusGlobal->radioStatus.persistentData.frequency, newBand);

    return retValue;
}

StatusCodeTy RadioManager::SetUp()
{
    StatusCodeTy retValue = CMD_OK;

    qDebug() << "Radio Manager Setup (nothing to do)";

    return retValue;
}

StatusCodeTy RadioManager::SetQualityMode(bool enable)
{
    StatusCodeTy retValue = CMD_OK;

    if (false == enable)
    {
        GetDabQualInfo(false);

        // Stop Quality SNR read
        emit EventFromDeviceSignal(EVENT_STOP_QUALITY_SNR);
    }
    else
    {
        GetDabQualInfo(true);

        // Activate Quality SNR scheduling (signal to StateMachineRadioManager)
        emit EventFromDeviceSignal(EVENT_QUALITY_SNR);
    }

    return retValue;
}

StatusCodeTy RadioManager::NextFrequency(unsigned int newFreq, BandTy newBand, SystemMode newMode)
{
    StatusCodeTy retValue = CMD_OK;

    // On frequency change we would like to play the first service because we
    // do not store last played service per each ensemble but only one
    radioStatusGlobal->panelData.serviceIndex = 0;
    memset(&radioStatusGlobal->radioStatus.persistentData.serviceName, 0, STATION_NAME_LEN);

    retValue =  NewTuneSequence(newFreq, newBand, newMode);

    // Update global variables and inform the HMI about the new frequency and the band
    UpdateRadioStatusAndSendFeedback(newFreq, newBand);

    return retValue;
}

void RadioManager::UpdateRadioStatusAndSendFeedback(unsigned int newFreq, BandTy newBand)
{
    // Update global structure with the new frequency values
    radioStatusGlobal->radioStatus.persistentData.frequency = newFreq;
    radioStatusGlobal->radioStatus.persistentData.band = newBand;

    // Feedback RadioPanel with the current Frequency and band
    RadioTuneAnswer tuneAnswer;

    tuneAnswer.freq = radioStatusGlobal->radioStatus.persistentData.frequency;
    tuneAnswer.band = radioStatusGlobal->radioStatus.persistentData.band;

    SendTuneResponse(tuneAnswer);
}

StatusCodeTy RadioManager::NewTuneSequence(unsigned int newFreq, BandTy newBand, SystemMode newMode)
{
    StatusCodeTy retValue = CMD_OK;

    // Set information that no service is playing and clear service list
    memset(&radioManagerStatus, 0, sizeof (radioManagerStatus));

    // Invalidate service list data
    radioManagerStatus.serviceListConfirmed = false;
    radioManagerStatus.numberOfRetry = 0;

#if (defined CONFIG_USE_STANDALONE)
    // Tune new frequency
    retValue = SetFrequency(newFreq, newBand, newMode);

    if (CMD_OK == retValue)
    {
        // In case of DAB we have to update global structure and get ensemble information
        if (BAND_DAB3 == newBand)
        {
            // Update global variables
            radioStatusGlobal->panelData.ensembleIndex = cmdMw->GetCurrEnsembleIndex();
            radioStatusGlobal->panelData.ensembleTable = cmdMw->GetCurrEnsembleTable();
            radioStatusGlobal->panelData.isGoodLevel = false;

            // We need to wait in order to gather information
            QThread::msleep(200);

            // Get information for the tuned ensemble
            retValue = GetInformationForEnsemble(newBand);

            if (CMD_OK == retValue)
            {
                // We have good signal and information should be available let's try to extract the audio service
                retValue = ExtractAudioService(radioStatusGlobal->radioStatus.persistentData.serviceId);

                if (CMD_OK != retValue)
                {
                    qDebug() << "ERROR: ExtractAudioServce() failed";
                }
            }
            else
            {
                qDebug() << "ERROR: command GetInformationForEnsemble() failed";
            }

            if (true == radioStatusGlobal->radioStatus.persistentData.testScreenIsActive)
            {
                // Activate Quality SNR scheduling (signal to StateMachineRadioManager)
                emit EventFromDeviceSignal(EVENT_QUALITY_SNR);
            }
        }
        else
        {
            if (BAND_FM == newBand)
            {
                // Activate RDS scheduling (signal to StateMachineRadioManager)
                emit EventFromDeviceSignal(EVENT_DATA_RDS_IN);
            }

            if ((BAND_FM == newBand) || (BAND_AM == newBand))
            {
                // Activate QualitySNR scheduling (signal to StateMachineRadioManager)
                emit EventFromDeviceSignal(EVENT_QUALITY_SNR);
            }
        }
    }
    else
    {
        qDebug() << "WARNING: new tune sequence reported bad signal level";

        // Update global variables
        radioStatusGlobal->panelData.isGoodLevel = false;
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    // Tune new frequency
    retValue = SetFrequency(newFreq, newBand, newMode);

    if (BAND_DAB3 == newBand)
    {
        // Update global variables
        radioStatusGlobal->panelData.ensembleIndex = getDab3IndexFromFrequency(newFreq);
        //radioStatusGlobal->panelData.ensembleTable = cmdMw->GetCurrEnsembleTable();
        radioStatusGlobal->panelData.isGoodLevel = false;

        QThread::msleep(200);

        // Get information for the tuned ensemble
        retValue = GetInformationForEnsemble(newBand);

        if (CMD_OK == retValue)
        {
            // We have good signal and information should be available let's try to extract the audio service
            retValue = ExtractAudioService(radioStatusGlobal->radioStatus.persistentData.serviceId);

            if (CMD_OK != retValue)
            {
                qDebug() << "ERROR: ExtractAudioServce() failed";
            }
        }
        else
        {
            qDebug() << "ERROR: command GetInformationForEnsemble() failed";
        }

        if (true == radioStatusGlobal->radioStatus.persistentData.testScreenIsActive)
        {
            // Activate QualitySNR scheduling (signal to StateMachineRadioManager)
            emit EventFromDeviceSignal(EVENT_QUALITY_SNR);
        }
    }
    else
    {
        if (BAND_FM == newBand)
        {
            // Activate RDS scheduling (signal to StateMachineRadioManager)
            emit EventFromDeviceSignal(EVENT_DATA_RDS_IN);
        }

        if ((BAND_FM == newBand) || (BAND_AM == newBand))
        {
            // Activate QualitySNR scheduling (signal to StateMachineRadioManager)
            emit EventFromDeviceSignal(EVENT_QUALITY_SNR);
        }
    }
#endif // #if (defined CONFIG_USE_ETAL)

    return retValue;
}

StatusCodeTy RadioManager::Seek(bool upDirection)
{
    StatusCodeTy retValue = CMD_OK;

    if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
    {
        // On frequency change we would like to play the first service because we
        // do not store last played service per each ensemble but only one
        radioStatusGlobal->panelData.serviceIndex = 0;
        memset(&radioStatusGlobal->radioStatus.persistentData.serviceName, 0, STATION_NAME_LEN);

        // Invalidate service list data
        radioManagerStatus.serviceListConfirmed = false;
        radioManagerStatus.numberOfRetry = 0;
    }

#if (defined CONFIG_USE_STANDALONE)
    rdsDecoder->ClearRdsData();
#endif // #if (defined CONFIG_USE_STANDALONE)

    seekOngoing = true;

    SeekSequence(upDirection);

    seekOngoing = false;

    return retValue;
}

void RadioManager::SeekSequence(bool updirection)
{
#if (defined CONFIG_USE_STANDALONE)
    if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
    {
        // Start seek sequence
        SeekDabSequence(updirection);
    }
    else
    {
        radioStatusGlobal->radioStatus.persistentData.frequency =
            SeekFmSequence(updirection, radioStatusGlobal->radioStatus.persistentData.frequency,
                           radioStatusGlobal->radioStatus.persistentData.radioMode);

        rdsDecoder->setRdsFmTuneFrequency(radioStatusGlobal->radioStatus.persistentData.frequency);

        cmdCis->getFmQualityParameters(radioStatusGlobal->radioStatus.persistentData.radioMode);
    }
#else
    quint32 step;
    QTimer timer;
    QEventLoop loop;

    timer.setSingleShot(true);

    QObject::connect(m_etal,
                     static_cast<void (Etal::*)()> (&Etal::seekFreqFoundSignal),
                     &loop,
                     static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

    QObject::connect(m_etal,
                     static_cast<void (Etal::*)()> (&Etal::seekFullCycleSignal),
                     &loop,
                     static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

    QObject::connect(&timer,
                     &QTimer::timeout,
                     &loop,
                     static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

    timer.start(60000);

    step = (BAND_AM == radioStatusGlobal->radioStatus.persistentData.band) ? 9 : 100;

    if (true == updirection)
    {
        m_etal->seekUp(step);
    }
    else
    {
        m_etal->seekDown(step);
    }

    loop.exec();

    radioStatusGlobal->radioStatus.persistentData.frequency = m_etal->frequency();

    // Feedback RadioPanel with the current Frequency and band on exiting the seek sequence
    UpdateRadioStatusAndSendFeedback(radioStatusGlobal->radioStatus.persistentData.frequency,
                                     radioStatusGlobal->radioStatus.persistentData.band);

    if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
    {
        RequestAnotherCheckForServiceData();
    }
#endif // #if (defined CONFIG_USE_STANDALONE)
}

bool RadioManager::updateEnsembleIndex(bool updirection)
{
    if (true == updirection)
    {
        if (radioStatusGlobal->panelData.ensembleIndex < (MAX_NB_DAB_FREQUENCIES - 1))
        {
            radioStatusGlobal->panelData.ensembleIndex++;
        }
        else
        {
            radioStatusGlobal->panelData.ensembleIndex = 0;

            return true;
        }
    }
    else
    {
        if (radioStatusGlobal->panelData.ensembleIndex > 0)
        {
            radioStatusGlobal->panelData.ensembleIndex--;
        }
        else
        {
            radioStatusGlobal->panelData.ensembleIndex = (MAX_NB_DAB_FREQUENCIES - 1);

            return true;
        }
    }

    return false;
}

#if (defined CONFIG_USE_STANDALONE)
void RadioManager::SeekDabSequence(bool updirection)
{
    #define DAB_SEEK_NOISEFLOOR_INIT     ((qint8) - 106)
    #define DAB_SEEK_NOISEFLOOR_MAX      ((qint8) - 60)
    #define DAB_SEEK_TH_INC              ((qint8)4)
    #define DAB_SEEK_TH_INC_NO_SYNC      ((qint8)2)
    #define DAB_SEEK_WAIT_PERIOD         (50)               // 50 ms is basic period

    // Wait time for sync is (DAB_SEEK_WAIT_FOR_SYNC+DAB_SEEK_CHECK_SYNC)*DAB_SEEK_WAIT_PERIOD
    #define DAB_SEEK_WAIT_FOR_SYNC       (25 * 2)           // 2500 ms wait for sync (initial wait time)

    // Wait for valid ensemble is DAB_SEEK_WAIT_FOR_ENSEMBLE*DAB_SEEK_WAIT_PERIOD
    #define DAB_SEEK_WAIT_FOR_ENSEMBLE   (10 * 2)           // 1000 ms

    int initIndex;
    bool isValidStation = false;
    bool isNoiseFloorSet = false;
    StatusCodeTy retValue;

    qint8 noiseFloor = DAB_SEEK_NOISEFLOOR_INIT;
    qint8 seekThr = noiseFloor + 0;

    qint64 startTime = CurrentEpochTimeMs(0);

    qDebug() << "DABSEEK: " << "start";

    // Update to next frequency
    updateEnsembleIndex(updirection);

    // Stop frequency is the new selected one in order to allow to return to the
    // original frequency
    initIndex = radioStatusGlobal->panelData.ensembleIndex;

    isValidStation = false;
    radioStatusGlobal->panelData.isGoodLevel = false;

    // Before to tune to the new frequency we need to tune DCOP to 0
    // in order to avoid that data is mixup between different ensemble.
    // It seems not to be necessary when command 0x7A is used
    qDebug() << "DABSEEK: Tune to 0";

    retValue = SetDcopFrequency(0);

    if (CMD_OK != retValue)
    {
        qDebug() << "DABSEEK: SetDcopFrequency(0) failed";
    }

    do
    {
        radioStatusGlobal->radioStatus.persistentData.frequency =
            cmdMw->getDab3FreqFromIndex(radioStatusGlobal->panelData.ensembleIndex);

        radioStatusGlobal->panelData.ensembleTable.ensFrequency =
            radioStatusGlobal->radioStatus.persistentData.frequency;

        // Tune the TUNER to the new frequency
        cmdCis->SeekDabManual(radioStatusGlobal->radioStatus.persistentData.frequency,
                              radioStatusGlobal->radioStatus.persistentData.radioMode,
                              updirection);

        // Feedback RadioPanel with the current Frequency and band
        UpdateRadioStatusAndSendFeedback(radioStatusGlobal->radioStatus.persistentData.frequency,
                                         radioStatusGlobal->radioStatus.persistentData.band);

        // We need to wait in order to gather information
        // if all channels are tuned then tune command gives enought delay
        QThread::msleep(10);

        qualityInfoTy qualInfo = cmdCis->getDabFieldStrength(radioStatusGlobal->radioStatus.persistentData.radioMode);

        qDebug() << "DABSEEK:" << (qint8)qualInfo.qualFstRf << "dBm";

        if ((qint8)qualInfo.qualFstRf < noiseFloor)
        {
            noiseFloor = (qint8)qualInfo.qualFstRf;
            seekThr = noiseFloor + DAB_SEEK_TH_INC;

            qDebug() << "DABSEEK: " << "... NO SIGNAL ==> noise floor: " << noiseFloor << " -> thr = " << seekThr;
        }
        else
        {
            if ((qint8)qualInfo.qualFstRf > seekThr)
            {
                // Potentially good station -> going to tune and sync
                qint64 tuneTime = CurrentEpochTimeMs(0);

                qDebug() << "DABSEEK: Tuning" << radioStatusGlobal->radioStatus.persistentData.frequency << "kHz";

                retValue = DabSeekTuneSequence();

                if (CMD_OK != retValue)
                {
                    qDebug() << "DABSEEK: DabSeekTuneSequence() failed";
                }

                int n = CurrentEpochTimeMs(tuneTime) / 50;

                while ((0 != dabSeekSyncLevel) && (false == radioManagerStatus.dabSync) && (n++ < (DAB_SEEK_WAIT_FOR_SYNC)))
                {
                    QThread::msleep(DAB_SEEK_WAIT_PERIOD);
                }

                qDebug() << "DABSEEK: time elapsed" << CurrentEpochTimeMs(tuneTime) << "ms";

                // Get information for the tuned ensemble
                //if (radioManagerStatus.dabSync && (dabSeekSyncLevel > 0))
                if (radioManagerStatus.dabSync)
                {
                    // Wait for ensemble data here
                    // Update global variables
                    radioStatusGlobal->panelData.ensembleIndex = cmdMw->GetCurrEnsembleIndex();
                    radioStatusGlobal->panelData.ensembleTable = cmdMw->GetCurrEnsembleTable();

                    retValue = GetInformationForEnsemble(radioStatusGlobal->radioStatus.persistentData.band);

                    n = 0;

                    while ((CMD_OK != retValue) && (n++ < DAB_SEEK_WAIT_FOR_ENSEMBLE))
                    {
                        QThread::msleep(DAB_SEEK_WAIT_PERIOD);

                        retValue = GetInformationForEnsemble(radioStatusGlobal->radioStatus.persistentData.band);
                    }

                    qDebug() << "DABSEEK: " << CurrentEpochTimeMs(tuneTime) << "ms -> GetInformationForEnsemble() = " << retValue;

                    if (CMD_OK == retValue)
                    {
                        // We have good signal and information should be available let's try to extract the audio service
                        retValue = ExtractAudioService(INVALID_SERVICE_ID);

                        if (CMD_OK != retValue)
                        {
                            qDebug() << "DABSEEK: ExtractAudioService() failed";
                        }

                        // Valid station found
                        isValidStation = true;
                        radioStatusGlobal->panelData.isGoodLevel = true;

                        qDebug() << "DABSEEK: Found DAB valid station (ensemble and service infos maybe not yet valid)";
                    }
                    else
                    {
                        qDebug() << "DABSEEK: command GetInformationForEnsemble() failed => no station, continue with seek";
                    }
                }

                if (!isValidStation)
                {
                    // Sync not reached -> continue with seek

                    // Before to tune to the new frequency we need to tune DCOP to 0
                    // in order to avoid that data is mixup between different ensemble
                    qDebug() << "DABSEEK: Tuning 0, current SYNC =" << dabSeekSyncLevel;

                    retValue = SetDcopFrequency(0);

                    if (CMD_OK != retValue)
                    {
                        qDebug() << "DABSEEK: SetDcopFrequency() failed";
                    }

                    if ((qint8)qualInfo.qualFstRf < DAB_SEEK_NOISEFLOOR_MAX)
                    {
                        if ((radioStatusGlobal->radioStatus.persistentData.frequency != 210096) && // 10N
                            (radioStatusGlobal->radioStatus.persistentData.frequency != 217088) && // 11N
                            (radioStatusGlobal->radioStatus.persistentData.frequency != 224096))   // 12N
                        {
                            if (isNoiseFloorSet)
                            {
                                noiseFloor += DAB_SEEK_TH_INC_NO_SYNC;
                                seekThr += DAB_SEEK_TH_INC_NO_SYNC;
                            }
                            else
                            {
                                noiseFloor = (qint8)qualInfo.qualFstRf;
                                seekThr = noiseFloor + DAB_SEEK_TH_INC_NO_SYNC;
                                isNoiseFloorSet = true;
                            }
                        }
                    }

                    qDebug() << "DABSEEK: " << "... NO SYNC ==> noise floor: " << noiseFloor << " -> thr = " << seekThr;
                }
            }
        }

        if (!isValidStation)
        {
            if (updateEnsembleIndex(updirection))
            {
                // reset noise floor and threshold when wrapping over band limits -> this forces sync
                noiseFloor = DAB_SEEK_NOISEFLOOR_INIT;
                seekThr = noiseFloor + 0;
                isNoiseFloorSet = false;
            }
        }
    }
    while ((false == isValidStation) &&
           (radioStatusGlobal->panelData.ensembleIndex != initIndex) &&
           (true == seekOngoing));

    // If seek exited without valid station for one full band cycle, reload initial index that was incremented(decremented) by 1
    // in updirection (downdirection)
    if ((false == isValidStation) &&
        (radioStatusGlobal->panelData.ensembleIndex == initIndex))
    {
        if (true == updirection)
        {
            radioStatusGlobal->panelData.ensembleIndex--;
        }
        else
        {
            radioStatusGlobal->panelData.ensembleIndex++;
        }
    }

    // Unmute
    cmdCis->sendSeekEndCmd(radioStatusGlobal->radioStatus.persistentData.radioMode);

    if (true == radioStatusGlobal->radioStatus.persistentData.testScreenIsActive)
    {
        // Activate QualitySNR scheduling (signal to StateMachineRadioManager)
        emit EventFromDeviceSignal(EVENT_QUALITY_SNR);
    }

    qDebug() << "DABSEEK: elapsed time " << QString::number(CurrentEpochTimeMs(startTime));
}

StatusCodeTy RadioManager::DabSeekTuneSequence()
{
    StatusCodeTy retValue = CMD_OK;

    // Set information that no service is playing and clear service list
    memset(&radioManagerStatus, 0, sizeof (radioManagerStatus));

    // Invalidate service list data
    radioManagerStatus.serviceListConfirmed = false;
    radioManagerStatus.numberOfRetry = 0;

    // When we tune to a new frequency we need to invalidate some data
    radioStatusGlobal->panelData.list.serviceList.clear();
    radioStatusGlobal->radioStatus.persistentData.frequency = radioStatusGlobal->panelData.ensembleTable.ensFrequency;

    radioStatusGlobal->radioStatus.persistentData.frequencyDab = radioStatusGlobal->radioStatus.persistentData.frequency;

    // Clear tune request
    dabSeekSyncLevel = -1;

    retValue = SetDcopFrequency(radioStatusGlobal->radioStatus.persistentData.frequency);

    if (CMD_OK != retValue)
    {
        qDebug() << "ERROR: DABSEEK failed to tune";
    }

    return retValue;
}

quint32 RadioManager::SeekFmSequence(bool updirection, quint32 currSeekFreq, SystemMode usedMode)
{
    CisSeekParamsTy cisSeekFm;
    int nbSeekMeas = 1;
    int freqStepInt = 100;
    int readCmd28DlyValue = 10;

    //quint32 newFreq;

    if (MODE_AM == usedMode)
    {
        if (COUNTRY_US == radioStatusGlobal->radioStatus.persistentData.country)
        {
            freqStepInt = 10;
        }
        else
        {
            freqStepInt = 9;
        }
    }

    // FM Auto seek procedure, device return first detected valid frequency
    // with possible indication that a fullSeekCycle was reached
    cmdCis->SeekFmAuto(updirection, nbSeekMeas, freqStepInt, usedMode, currSeekFreq);

    // Set the starting frequency
    //newFreq = radioStatusGlobal->radioStatus.persistentData.frequency;

    do
    {
        cisSeekFm = cmdCis->GetSeekFmStatus(readCmd28DlyValue, usedMode);

        // Feedback RadioPanel with the current Frequency and band
        RadioTuneAnswer tuneAnswer;

        tuneAnswer.freq = cisSeekFm.reachedFreq; // += freqStepInt;
        tuneAnswer.band = radioStatusGlobal->radioStatus.persistentData.band;

        SendTuneResponse(tuneAnswer);

        //        //quint32 isNewFreq = 1;
        //        eventDataInterface eventData;
        //        newFreq = cisSeekFm.reachedFreq; //+= freqStepInt;

        //        eventData.dataPtr = reinterpret_cast<unsigned char *> (&newFreq); // (unsigned char *)&isNewFreq;
        //        //eventData.size = sizeof (isNewFreq);
        //        eventData.size = sizeof (newFreq);
        //        eventData.eventType = EVENTS_RX_FREQ;

        //        eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
    }
    while ((cisSeekFm.isValidStation == false) &&
           (cisSeekFm.isFullSeekCycle == false) &&
           (true == seekOngoing));

    cmdCis->sendSeekEndCmd(usedMode);

    return cisSeekFm.reachedFreq;
}

#endif // #if (defined CONFIG_USE_STANDALONE)

void RadioManager::VolumeUp()
{
    if (radioStatusGlobal->radioStatus.persistentData.radioVolume < VOLUME_MAX_VALUE)
    {
        radioStatusGlobal->radioStatus.persistentData.radioVolume++;
    }
    else
    {
        radioStatusGlobal->radioStatus.persistentData.radioVolume = VOLUME_MAX_VALUE;
    }

#if (defined CONFIG_USE_STANDALONE)
    cmdCis->cisSetVolume(radioStatusGlobal->radioStatus.persistentData.radioVolume,
                         radioStatusGlobal->radioStatus.persistentData.radioMode);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    m_etal->setVolume(radioStatusGlobal->radioStatus.persistentData.radioVolume);
#endif // #if (defined CONFIG_USE_ETAL)
}

void RadioManager::VolumeDown()
{
    if (radioStatusGlobal->radioStatus.persistentData.radioVolume > 0)
    {
        radioStatusGlobal->radioStatus.persistentData.radioVolume--;
    }
    else
    {
        radioStatusGlobal->radioStatus.persistentData.radioVolume = 0;
    }

#if (defined CONFIG_USE_STANDALONE)
    cmdCis->cisSetVolume(radioStatusGlobal->radioStatus.persistentData.radioVolume,
                         radioStatusGlobal->radioStatus.persistentData.radioMode);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    m_etal->setVolume(radioStatusGlobal->radioStatus.persistentData.radioVolume);
#endif // #if (defined CONFIG_USE_ETAL)
}

bool RadioManager::IsRadioActive()
{
    return radioStatusGlobal->panelData.isRadioOn;
}

void RadioManager::CommandForRadioManagerSlot(Events event, QVariant data)
{
    // If an user event is waiting we delay any automatic check
    if (true == userEventPending)
    {
        return;
    }

    // We just use the event receiver function also for events from state machines
    ProcessEvent(event, data);
}

void RadioManager::SendBootResponse(PostalOfficeBase *& postalOfficeBase)
{
    PostalOffice<bool>* postalOffice = new PostalOffice<bool> ();
    postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

    postalOffice->PutPacket(POSTAL_TYPE_IS_EVENT_ANSWER, radioStatusGlobal->panelData.isHwConnectionOk);

    postalOffice->Lock();
}

void RadioManager::SendRadioLastStatus(PostalOfficeBase *& postalOfficeBase)
{
    PostalOffice<PersistentData>* postalOffice = new PostalOffice<PersistentData> ();
    postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

    postalOffice->PutPacket(POSTAL_TYPE_IS_EVENT_ANSWER, radioStatusGlobal->radioStatus.persistentData);

    postalOffice->Lock();
}

void RadioManager::SendSourceResponse(PostalOfficeBase *& postalOfficeBase)
{
    PostalOffice<BandTy>* postalOffice = new PostalOffice<BandTy> ();
    postalOfficeBase = dynamic_cast<PostalOfficeBase *> (postalOffice);

    postalOffice->PutPacket(POSTAL_TYPE_IS_EVENT_ANSWER, radioStatusGlobal->radioStatus.persistentData.band);

    postalOffice->Lock();
}

StatusCodeTy RadioManager::GetEventData(Events event, PostalOfficeBase *& postalOfficeBase)
{
    StatusCodeTy statusCode = CMD_OK;

    if (EVENT_BOOT_ON == event)
    {
        SendBootResponse(postalOfficeBase);
    }
    else if (EVENT_RADIO_LASTSTATUS == event)
    {
        SendRadioLastStatus(postalOfficeBase);
    }
    else if (EVENT_SOURCE == event)
    {
        SendSourceResponse(postalOfficeBase);
    }
    else
    {
        // Set the data pointer to null
        postalOfficeBase = nullptr;
    }

    return statusCode;
}

bool RadioManager::ProcessEvent(Events event, QVariant data)
{
    BandTy newSource;
    qualityInfoTy qualInfo;

#if (defined CONFIG_USE_STANDALONE)
    QByteArray curRdsBuffer;
#endif // #if (defined CONFIG_USE_STANDALONE)

    // Avoid executing a command meanwhile another one is under execution
    if (true == eventOngoing)
    {
        if ((EVENT_SEEK_DOWN == event) ||
            (EVENT_SEEK_UP == event) ||
            (EVENT_SOURCE == event))
        {
            // Stop currently active Seek procedure (StandAlone)
            if (true == seekOngoing)
            {
                seekOngoing = false;

                if (EVENT_SOURCE == event)
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }

            return false;
        }

        // Return indication that event hasn't been executed
        return false;
    }

    // Check if the radio is in the correct status to receive commands
    if (false == radioStatusGlobal->panelData.isRadioOn)
    {
        if (EVENT_RADIO_LASTSTATUS != event &&
            EVENT_BOOT_ON != event &&
            EVENT_POWER_UP != event &&
            EVENT_POWER_DOWN != event)
        {
            return false;
        }
    }

    // Flag the event as ONGOING
    eventOngoing = true;

    switch (event)
    {
        case EVENT_RADIO_LASTSTATUS:
        {
            RadioLastStatus();
        }
        break;

        case EVENT_BOOT_ON:
        {
            if (CMD_OK != BootOn())
            {
                qDebug() << "ERROR: boot failed";

                radioStatusGlobal->panelData.isHwConnectionOk = false;
            }
            else
            {
                radioStatusGlobal->panelData.isHwConnectionOk = true;
            }
        }
        break;

        case EVENT_POWER_UP:
        {
            // Do power on without checking the return value: it is a sequence and it can
            // fail due to lack of information requested
            PowerOn();

            // Power on occurred
            radioStatusGlobal->panelData.isRadioOn = true;

            // Do anothger check: it is anyway necessary to get teh proper confirmation of the services list
            RequestAnotherCheckForServiceData();
        }
        break;

        case EVENT_POWER_DOWN:
        {
            if (data.isValid())
            {
                // Setup variables
                radioStatusGlobal->radioStatus.persistentData.slsDisplayEn = data.value<PersistentData>().slsDisplayEn;
                radioStatusGlobal->radioStatus.persistentData.afCheckEn = data.value<PersistentData>().afCheckEn;
                radioStatusGlobal->radioStatus.persistentData.dabFmServiceFollowingEn = data.value<PersistentData>().dabFmServiceFollowingEn;
                radioStatusGlobal->radioStatus.persistentData.dabDabServiceFollowingEn = data.value<PersistentData>().dabDabServiceFollowingEn;
                radioStatusGlobal->radioStatus.persistentData.rdsTaEnabled = data.value<PersistentData>().rdsTaEnabled;
                radioStatusGlobal->radioStatus.persistentData.rdsRegionalEn = data.value<PersistentData>().rdsRegionalEn;
                radioStatusGlobal->radioStatus.persistentData.rdsEonEn = data.value<PersistentData>().rdsEonEn;
                radioStatusGlobal->radioStatus.persistentData.globalServiceListEn = data.value<PersistentData>().globalServiceListEn;
                radioStatusGlobal->radioStatus.persistentData.displayUnsupportedServiceEn = data.value<PersistentData>().displayUnsupportedServiceEn;
                radioStatusGlobal->radioStatus.persistentData.alphabeticOrder = data.value<PersistentData>().alphabeticOrder;
                radioStatusGlobal->radioStatus.persistentData.testScreenIsActive = data.value<PersistentData>().testScreenIsActive;
            }

            PowerOff();

            radioStatusGlobal->panelData.isRadioOn = false;
        }
        break;

        case EVENT_MUTE_TOGGLE:
        {
            if (CMD_OK != MuteToggle())
            {
                qDebug() << "ERROR: mute command failed";
            }
        }
        break;

        case EVENT_SERVICE_SELECT:
        {
            if (data.isValid())
            {
                unsigned int selectionId = data.toInt();

                ServiceSelect(selectionId);
            }
        }
        break;

        case EVENT_PRESET_SELECT:
        {
            if (data.isValid())
            {
                int presetNumber = data.toInt();

                PresetSelect(presetNumber);
            }
        }
        break;

        case EVENT_PRESET_SAVE:
        {
            PresetSave();
        }
        break;

        case EVENT_SOURCE:
        {
            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                newSource = BAND_AM;
            }
            else
            {
                newSource = (BandTy)(1 + radioStatusGlobal->radioStatus.persistentData.band);
            }

            Source(newSource);

            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                RequestAnotherCheckForServiceData();
            }
        }
        break;

        case EVENT_QUALITY_ENABLE:
        {
            SetQualityMode(true);
        }
        break;

        case EVENT_QUALITY_DISABLE:
        {
            SetQualityMode(false);
        }
        break;

        case EVENT_PRESET_CLEAR:
        {
            ClearAllPresets();
        }
        break;

        case EVENT_LEARN:
        {
            LearnProcedure();
        }
        break;

        case EVENT_SEEK_UP:
        {
            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                // Before start seek stop automatic events
                emit EventFromDeviceSignal(EVENT_STOP_ALL_EVENTS);
            }

            Seek(true);

            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                // Enable automatic events again
                emit EventFromDeviceSignal(EVENT_START_ALL_EVENTS);
            }
        }
        break;

        case EVENT_SEEK_DOWN:
        {
            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                // Before start seek stop automatic events
                emit EventFromDeviceSignal(EVENT_STOP_ALL_EVENTS);
            }

            Seek(false);

            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                // Enable automatic events again
                emit EventFromDeviceSignal(EVENT_START_ALL_EVENTS);
            }
        }
        break;

        case EVENT_VOLUME_UP:
        {
            VolumeUp();
        }
        break;

        case EVENT_VOLUME_DOWN:
        {
            VolumeDown();
        }
        break;

        case EVENT_FREQUENCY_CHANGE:
        {
            if (data.isValid())
            {
                unsigned int frequency = data.toInt();

                if (CMD_OK != NextFrequency(frequency,
                                            radioStatusGlobal->radioStatus.persistentData.band,
                                            radioStatusGlobal->radioStatus.persistentData.radioMode))
                {
                    qDebug() << "ERROR: frequency step failed";
                }

                if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
                {
                    RequestAnotherCheckForServiceData();
                }
            }
        }
        break;

        case EVENT_CHECK_ALIVE:
        {
            DcopCheckAlive();
        }
        break;

        case EVENT_AUTO_CHECK_DATA:
        {
            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                if (false == radioManagerStatus.serviceListConfirmed)
                {
                    GetInformationForEnsemble(radioStatusGlobal->radioStatus.persistentData.band);

                    // Check service list status
                    if (true == radioManagerStatus.serviceListConfirmed)
                    {
                        // Try to extract the service
                        ExtractAudioService(radioStatusGlobal->radioStatus.persistentData.serviceId);

                        // We have all information we need, ask middleware to send information also
                        // to worker for HMI
#if (defined CONFIG_USE_STANDALONE)
                        cmdMw->SendLastServiceList();
#endif // #if (defined CONFIG_USE_STANDALONE)
#if (defined CONFIG_USE_ETAL)
                        m_etal->setServiceList();
#endif // #if (defined CONFIG_USE_ETAL)
                    }
                }
                else if (false == radioManagerStatus.audioServiceIsPlaying)
                {
                    // Command audio service extraction
                    ExtractAudioService(radioStatusGlobal->radioStatus.persistentData.serviceId);

                    // If we do have an audio service playing we need to inform the HMI about that so the
                    // proper display action can be taken
                    if (true == radioManagerStatus.audioServiceIsPlaying)
                    {
#if (defined CONFIG_USE_STANDALONE)
                        cmdMw->SendLastServiceList();
#endif // #if (defined CONFIG_USE_STANDALONE)
#if (defined CONFIG_USE_ETAL)
                        m_etal->setServiceList();
#endif // #if (defined CONFIG_USE_ETAL)
                    }
                }

                // Check if we need to do another request
                RequestAnotherCheckForServiceData();
            }
        }
        break;

        case EVENT_DATA_RDS_IN:
#if (defined CONFIG_USE_STANDALONE)
            {
                // Read RDS buffer data
                cmdCis->RdsDataPolling(radioStatusGlobal->radioStatus.persistentData.radioMode);

                // Wait a little bit the storing of RDS data
                QThread::msleep(3);

                // Get RdsRaw data from rdsBuffersList
                curRdsBuffer = cmdCis->GetRdsData();

                if (curRdsBuffer != nullptr)
                {
                    rdsDecoder->ProcessRawData(curRdsBuffer);

                    if (true == rdsDecoder->NewDataAvailable())
                    {
                        RdsDataSetTableTy rdsDataSetTableTy;

                        rdsDecoder->GetNewData(rdsDataSetTableTy);

                        SendRdsData(&rdsDataSetTableTy);
                    }
                }
            }
#endif // #if (defined CONFIG_USE_STANDALONE)

            break;

        case EVENT_QUALITY_SNR:
        {
            // Read Quality SNR data
#if (defined CONFIG_USE_STANDALONE)
            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                // This call will fill the entire structure, also part that is from DCOP: we will update this part
                // with a call to the DCOP
                qualInfo = cmdCis->getDabFieldStrength(radioStatusGlobal->radioStatus.persistentData.radioMode);

                // Fill DAB quality part
                qualInfo.dabQualityInfo = GetDabQualInfo(true);

                // Get sync status
                qualInfo.sync = cmdMw->GetSyncStatus();

                // Fill FSB
                qualInfo.qualFstBb = qualInfo.dabQualityInfo.baseband_level; // Use oly 8 LSB of BaseBand Level

                qualInfo.qualFicBer = qualInfo.dabQualityInfo.ficBer; // radioStatusGlobal->radioStatus.dynamicData.qualityLevel;
            }
            else
            {
                qualInfo = cmdCis->getFmQualityParameters(radioStatusGlobal->radioStatus.persistentData.radioMode);

                if (qualInfo.fmQualityInfo.qualSNR > 20)
                {
                    qualInfo.sync = true;
                }
                else
                {
                    qualInfo.sync = false;
                }
            }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
            qDebug() << "Radio mngr";

            qualInfo = m_etal->getQuality();
#endif // #if (defined CONFIG_USE_ETAL)

            SendQualityData(&qualInfo);
        }
        break;

        default:
        {
            qDebug() << "RADIO MANAGER received unknown event";
        }
        break;
    }

    // Flag the event as completed
    eventOngoing = false;

    return true;
}

void RadioManager::RequestAnotherCheckForServiceData()
{
    // Check if we have a confirmed service list and we are playing audio
    if (false == radioManagerStatus.serviceListConfirmed ||
        false == radioManagerStatus.audioServiceIsPlaying)
    {
        // We have no service list: schedule a request
        emit EventFromDeviceSignal(EVENT_AUTO_CHECK_DATA);
    }
}

void RadioManager::DisconnectSockets()
{
    parametersProtLayerTy plParameters = protocolLayer->GetPLParameters();

#if (defined CONFIG_USE_STANDALONE)
    cmdCis->sendCmdReservedForProtocolLayer(PROT_LAYER_EXIT, 0, plParameters.dcopPort);
    cmdCis->sendCmdReservedForProtocolLayer(PROT_LAYER_EXIT, 0, plParameters.cmostPort);
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    //    m_etal->etalSendCmdReservedForProtocolLayer(PROT_LAYER_EXIT,0,TCP_DCOP_PROTOCOL_LAYER_PORT);
    //    m_etal->etalSendCmdReservedForProtocolLayer(PROT_LAYER_EXIT,0,TCP_CMOST_PROTOCOL_LAYER_PORT);
#endif // #if (defined CONFIG_USE_ETAL)
}

bool RadioManager::ConnectSockets()
{
#if (defined CONFIG_USE_STANDALONE)
    // Open the cmost tcp socket
    if (false == tcpCmostTLayer->OpenTcpLayerSocket())
    {
        qDebug() << "ERROR: CMOST socket doesn`t open!";

        return false;
    }

    // Open the dcop tcp socket
    if (false == tcpDcopTLayer->OpenTcpLayerSocket())
    {
        qDebug() << "ERROR: DCOP socket doesn`t open!";

        return false;
    }

    // Try to connect the tcpCmostTransportLayer to the protocolLayerProcess
    if (true == tcpCmostTLayer->connect())
    {
        if (true == tcpDcopTLayer->connect())
        {
            return true;
        }
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

    return false;
}

void RadioManager::RadioMngrApplicationInit()
{
#if (defined CONFIG_USE_STANDALONE)
    protocolLayer = new ProtocolLayer();

    protocolLayer->ProtocolLayerStart();

    parametersProtLayerTy plParameters = protocolLayer->GetPLParameters();

    tcpCmostTLayer = new TcpTransportLayer(this, plParameters.ipPortAddress, plParameters.cmostPort);
    tcpDcopTLayer = new TcpTransportLayer(this, plParameters.ipPortAddress, plParameters.dcopPort);

    // Connect to protocol layer
    ConnectSockets();

    // Instantiate the Middleware obj
    cmdMw = new Middleware(this);

    // Instantiate the Cis cmds obj
    cmdCis = new CisCmds(this);

    // Instantiate RDS decoder
    rdsDecoder = new RdsDecoder();

    // Do connections
    QObject::connect(tcpCmostTLayer,
                     static_cast<void (TcpTransportLayer::*)(QByteArray)> (&TcpTransportLayer::tcpLayer_sendDataToDispatcher_signal),
                     cmdCis,
                     static_cast<void (CisCmds::*)(QByteArray)> (&CisCmds::cis_receivedDataFromDispatcher_slot));

    QObject::connect(tcpDcopTLayer,
                     static_cast<void (TcpTransportLayer::*)(QByteArray)> (&TcpTransportLayer::tcpLayer_sendDataToDispatcher_signal),
                     cmdMw,
                     static_cast<void (Middleware::*)(QByteArray)> (&Middleware::mdw_receivedDataFromDcopTcpLayer_slot));

    QObject::connect(cmdMw,
                     static_cast<void (Middleware::*)(QByteArray)> (&Middleware::mdw_sendCmdToDispatcher_signal),
                     tcpDcopTLayer,
                     static_cast<void (TcpTransportLayer::*)(QByteArray)> (&TcpTransportLayer::receivedCmdForDevice_slot));

    QObject::connect(cmdCis,
                     static_cast<void (CisCmds::*)(QByteArray)> (&CisCmds::cis_sendCmdToDispatcher_signal),
                     tcpCmostTLayer,
                     static_cast<void (TcpTransportLayer::*)(QByteArray)> (&TcpTransportLayer::receivedCmdForDevice_slot));

    QObject::connect(cmdCis,
                     static_cast<void (CisCmds::*)(QString, QString, int, unsigned int)> (&CisCmds::inShellCmostData_signal),
                     shellWindow,
                     static_cast<void (ShellWindow::*)(QString, QString, int, unsigned int)> (&ShellWindow::inShellCmostData_slot));

    QObject::connect(cmdMw,
                     static_cast<void (Middleware::*)(QString, QString, int)> (&Middleware::inShellMwData_signal),
                     shellWindow,
                     static_cast<void (ShellWindow::*)(QString, QString, int)> (&ShellWindow::inShellMwData_slot));
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    protocolLayer = new ProtocolLayer();
    protocolLayer->ProtocolLayerStart();

    //QObject::connect(m_etal, &Etal::logging, shellWindow, &ShellWindow::inShellEtalData_slot);
    //QObject::connect(m_etal, &Etal::message, shellWindow, &ShellWindow::inShellEtalData_slot);
    //QObject::connect(m_etal, &Etal::seekStationList, this, &RadioManager::seek_gl_slot);
    QObject::connect(m_etal,
                     static_cast<void (Etal::*)(QString, QString, qint32)> (&Etal::logging),
                     shellWindow,
                     static_cast<void (ShellWindow::*)(QString, QString, int)> (&ShellWindow::inShellEtalData_slot));

    QObject::connect(m_etal,
                     static_cast<void (Etal::*)(QString, QString, qint32)> (&Etal::message),
                     shellWindow,
                     static_cast<void (ShellWindow::*)(QString, QString, int)> (&ShellWindow::inShellEtalData_slot));

    QObject::connect(m_etal,
                     static_cast<void (Etal::*)(void *)> (&Etal::seekStationList),
                     this,
                     static_cast<void (RadioManager::*)(void *)> (&RadioManager::seek_gl_slot));
#endif // #if (defined CONFIG_USE_ETAL)
}

//bool RadioManager::IdentifyServiceToExtract()
//{
//    QString tmpStr = QString(radioStatusGlobal->radioStatus.persistentData.serviceName);

//    QListIterator<ServiceTy> it(radioStatusGlobal->panelData.ensembleTable.ServicesTableList);
//    int index = 0;
//    bool found = false;

//    if (-1 != radioStatusGlobal->panelData.serviceIndex)
//    {
//        return true;
//    }

//    if (false == tmpStr.isEmpty())
//    {
//        while (it.hasNext())
//        {
//            ServiceTy service = it.next();

//            if (tmpStr == service.ServiceLabel)
//            {
//                found = true;

//                break;
//            }
//            else
//            {
//                index++;
//            }
//        }

//        if (true == found)
//        {
//            radioStatusGlobal->panelData.serviceIndex = index;
//        }
//    }
//    else
//    {
//        // We do not have a service label string than we select first service if we have any
//        if (false == radioStatusGlobal->panelData.ensembleTable.ServicesTableList.isEmpty())
//        {
//            ServiceTy service = radioStatusGlobal->panelData.ensembleTable.ServicesTableList.at(0);

//            if (false == service.ServiceLabel.isEmpty())
//            {
//                radioStatusGlobal->panelData.serviceIndex = 0;

//                found = true;
//            }
//        }
//    }

//    return found;
//}

StatusCodeTy RadioManager::GetInformationForEnsemble(BandTy band)
{
    StatusCodeTy retValue = CMD_OK;

    if (BAND_DAB3 == band)
    {
        // Get ensemble list
        retValue = GetEnsembleList();

        if (CMD_OK != retValue)
        {
            //qDebug() << "ERROR: GetEnsembleList() failed";
        }
        else
        {
            // On good ensemble list ask for ensemble data
            retValue = GetEnsembleData();

            if (CMD_OK != retValue)
            {
                //qDebug() << "ERROR: GetEnsembleData() failed";
            }
            else
            {
                // We have ensemble data, proceed with service list
                retValue = GetServiceList();

                if (CMD_OK != retValue)
                {
                    //qDebug() << "ERROR: GetServiceList() failed";
                }
                else
                {
                    // Wait a little bit the availability of service label
                    QThread::msleep(5);

                    // Finally get services data
                    retValue = GetSpecificServiceData();

                    if (CMD_OK != retValue)
                    {
                        //qDebug() << "ERROR: GetSpecificServiceData() failed";
                    }
                }
            }
        }

        // Now update global data
        if (CMD_OK != retValue)
        {
            // Update on command failure
            UpdateGlobalStructures(false);
        }
        else
        {
            // We need to update global structures in order to have
            // updated data available to the HMI and other users
            UpdateGlobalStructures(true);
        }
    }

    return retValue;
}

StatusCodeTy RadioManager::ExtractAudioService(unsigned int serviceId)
{
#if 1
    StatusCodeTy retValue;

    // Extract the selected service
    retValue = ServiceSelect(serviceId);

    if (CMD_OK == retValue)
    {
        // If we succeed there's a service that is playing music
        radioManagerStatus.audioServiceIsPlaying = true;

        // Update information to the HMI
        SendAudioPlaysUpdate(radioManagerStatus.audioServiceIsPlaying);
    }
    else
    {
        // We do not have a service playing
        radioManagerStatus.audioServiceIsPlaying = false;

        // Update information to the HMI
        SendAudioPlaysUpdate(radioManagerStatus.audioServiceIsPlaying);
    }
#else
    StatusCodeTy retValue = OPERATION_FAILED;

    if (false == radioManagerStatus.audioServiceIsPlaying)
    {
        // TODO: remove, this is an hack
        if (true == IdentifyServiceToExtract())
        {
            // Extract the selected service
            retValue = ServiceSelect();

            if (CMD_OK == retValue)
            {
                // If we succeed there's a service that is playing music
                radioManagerStatus.audioServiceIsPlaying = true;

                qDebug() << "RADIO MANAGEMENT: service is now playing";
            }
            else
            {
                // We do not have a service playing
                radioManagerStatus.audioServiceIsPlaying = false;

                //qDebug() << "ERROR: ServiceSelect() failed";
            }
        }
        else
        {
            // We do not have a service playing
            radioManagerStatus.audioServiceIsPlaying = false;

            // If we do not have a service here and the service list is already confirmed
            // could be that the we switched ensemble on same frequency then extract first service next request
            if (true == radioManagerStatus.serviceListConfirmed)
            {
                memset(&radioStatusGlobal->radioStatus.persistentData.serviceName[0], 0x00, STATION_NAME_LEN);
            }

            qDebug() << "ERROR: No valid service to play identified";
        }
    }
    else
    {
        retValue = CMD_OK;
    }
#endif // #if 1

    return retValue;
}

void RadioManager::CheckDabStatusResponse(qualityInfoTy& info)
{
    // If we transition from no sync to sync let's check if we have new services
    if (false == radioManagerStatus.dabSync && true == info.sync)
    {
        // Ask for data
        if (false == radioManagerStatus.serviceListConfirmed)
        {
            // We do not have a confirmed service list: schedule a request
            emit EventFromDeviceSignal(EVENT_DATA_ON_SYNC);
        }
        else if (0 == radioManagerStatus.numberOfAlreadyStoredServices)
        {
            radioManagerStatus.serviceListConfirmed = false;
            radioManagerStatus.numberOfRetry = 0;

            // We have no service list: schedule a request
            emit EventFromDeviceSignal(EVENT_DATA_ON_SYNC);
        }
    }

    // Save new information
    radioManagerStatus.dabSync = info.sync;
}

void RadioManager::EventRaised(eventsList eventType, int size, eventDataInterface* eventData)
{
    Q_UNUSED(size);

    // We react to event only after power-up is complete
    if ((EVENTS_RX_SYNC_LEVEL == eventType) || (EVENTS_RX_QUALITY_LEVEL == eventType))
    {
        // We need to check if an event is ongoing, in that case we cannot send a new command
        // If we execute radio manager and state machine on same thread we need to be sure that
        // we are not executing any command, add condition:
        // - false == eventOngoing
        if (ACTIVE_STATE == radioStatusGlobal->radioStatus.dynamicData.radioState)
        {
            qualityInfoTy qualityInfo;

            memcpy(&qualityInfo, eventData->dataPtr, sizeof (qualityInfoTy));

            if (EVENTS_RX_SYNC_LEVEL == eventType)
            {
                // Analyze the data and take the decision on react on it
                CheckDabStatusResponse(qualityInfo);

                // This is used for seek only -> using maximum to give better chance to catch sync
                if (qualityInfo.syncLevel > dabSeekSyncLevel)
                {
                    dabSeekSyncLevel = qualityInfo.syncLevel;
                }
            }
        }
    }
    else if (EVENTS_RX_SERVICE_LIST == eventType)
    {
        RequestAnotherCheckForServiceData();
    }
    else if (EVENTS_RX_PING == eventType)
    {
        // Check ping data in order to verify if the device is still alive (sw watchdog)
        CheckAliveManagement();
    }
    else if (EVENTS_RX_SERVICE_NAME == eventType)
    {
#if defined (CONFIG_USE_ETAL)
        //radioStatusGlobal->panelData.serviceIndex = m_etal->idx();
#endif // #if defined(CONFIG_USE_ETAL)
    }
    else if (EVENTS_RX_RDS_BLOCKS == eventType)
    {
        // TODO
    }
}

void RadioManager::SendTuneResponse(RadioTuneAnswer& tuneData)
{
    eventDataInterface eventData;

    eventData.dataPtr = reinterpret_cast<unsigned char *> (&tuneData);
    eventData.size = sizeof (RadioTuneAnswer);
    eventData.eventType = EVENTS_RX_FREQ;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void RadioManager::SendAudioPlaysUpdate(bool audioPlays)
{
    eventDataInterface eventData;

    eventData.dataPtr = reinterpret_cast<unsigned char *> (&audioPlays);
    eventData.size = sizeof (bool);
    eventData.eventType = EVENTS_UPDATE_AUDIO_PLAYS;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void RadioManager::SendRdsData(RdsDataSetTableTy* rdsData)
{
    eventDataInterface eventData;

    eventData.dataPtr = reinterpret_cast<unsigned char *> (rdsData);
    eventData.size = sizeof (RdsDataSetTableTy);
    eventData.eventType = EVENTS_UPDATE_RDS_DATA;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void RadioManager::SendQualityData(qualityInfoTy* qualInfo)
{
    eventDataInterface eventData;

    eventData.dataPtr = reinterpret_cast<unsigned char *> (qualInfo);
    eventData.size = sizeof (qualityInfoTy);
    eventData.eventType = EVENTS_RX_QUALITY_LEVEL;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void RadioManager::CheckAliveManagement()
{
    qint64 tmpVal;
    QDateTime now = QDateTime::currentDateTime();

    // Check alive management
    tmpVal = checkAliveData.msFromEpoch;

    checkAliveData.msFromEpoch = now.toMSecsSinceEpoch();

    checkAliveData.deltaFromLatestEvent = checkAliveData.msFromEpoch - tmpVal;
}

bool RadioManager::UpdateStationList(EnsembleTableTy& tmpTable)
{
    Q_UNUSED(tmpTable);

    // StationListGlobal *stationListGlobal = stationListGlobal->Instance ();

#if 0
    stationListGlobal->Test();
#endif // #if 0

    // TODO: implement
    // stationListUpdater->UpdateGlobalStationList(tmpTable, stationListGlobal);

    return true;
}

StatusCodeTy RadioManager::PresetSelect(int currentPresetIndex)
{
    StatusCodeTy retValue = CMD_OK;

    RadioServicePreset preset = presetList->GetPreset(currentPresetIndex);

    BandTy prsBand = (BandTy)preset.GetBand();

    unsigned int curFreq = preset.GetFrequency();

    unsigned int orgFreq = radioStatusGlobal->radioStatus.persistentData.frequency;

    radioStatusGlobal->radioStatus.persistentData.serviceId = preset.GetId();

    if ((preset.GetId() == INVALID_SERVICE_ID) && (preset.GetServiceName() == "Empty"))
    {
        return retValue;
    }

    memcpy(radioStatusGlobal->radioStatus.persistentData.serviceName,
           (preset.GetServiceName()).c_str(),
           STATION_NAME_LEN);

#if (defined CONFIG_USE_ETAL)
    // Save current content
    m_cont[m_etal->mode()] = m_etal->getContent();

    if (BAND_FM == prsBand)
    {
        m_cont[FM].m_id = radioStatusGlobal->radioStatus.persistentData.serviceId;
        m_etal->setContent(&m_cont[FM]);
    }
    else if (BAND_AM == prsBand)
    {
        m_cont[AM].m_id = radioStatusGlobal->radioStatus.persistentData.serviceId;
        m_etal->setContent(&m_cont[AM]);
    }
    else
    {
        m_cont[DAB].m_id = radioStatusGlobal->radioStatus.persistentData.serviceId;
        m_etal->setContent(&m_cont[DAB]);
    }
#endif // #if (defined CONFIG_USE_ETAL)

    if (radioStatusGlobal->radioStatus.persistentData.band != prsBand)
    {
        SaveCurrentBandData(radioStatusGlobal->radioStatus.persistentData.band);

        radioStatusGlobal->radioStatus.persistentData.band = prsBand;

        SetNewBandData(prsBand);

#if (defined CONFIG_USE_STANDALONE)
        SetBand();
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
        // Band change command
        m_etal->setBand((Band)radioStatusGlobal->radioStatus.persistentData.band);    // Set band B_DAB3
#endif // #if (defined CONFIG_USE_ETAL)

        if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
        {
            // Start DCOP sending events
            StartEvents();
        }

        retValue = NewTuneSequence(radioStatusGlobal->radioStatus.persistentData.frequency,
                                   radioStatusGlobal->radioStatus.persistentData.band,
                                   radioStatusGlobal->radioStatus.persistentData.radioMode);
    }
    else
    {
        radioStatusGlobal->radioStatus.persistentData.band = prsBand;

        SetNewBandData(prsBand);
    }

    if (BAND_DAB3 == prsBand)
    {
        if (curFreq != orgFreq)
        {
            // Preset Frequency is different from current frequency, means
            // it must be recalled a service belonging to another ensemble
            // Retune to wanted ensemble frequency
            radioStatusGlobal->radioStatus.persistentData.frequency = curFreq;
            radioStatusGlobal->radioStatus.persistentData.frequencyDab = curFreq;
            radioStatusGlobal->panelData.ensembleIndex = getDab3IndexFromFrequency(curFreq);

            radioStatusGlobal->panelData.ensembleTable.ensFrequency =
                radioStatusGlobal->radioStatus.persistentData.frequency;

            if (CMD_OK != NextFrequency(radioStatusGlobal->radioStatus.persistentData.frequency,
                                        radioStatusGlobal->radioStatus.persistentData.band,
                                        radioStatusGlobal->radioStatus.persistentData.radioMode))
            {
                qDebug() << "ERROR: frequency step failed";
            }

            RequestAnotherCheckForServiceData();
        }
#if (defined CONFIG_USE_ETAL)
        else
#endif // (defined CONFIG_USE_ETAL)
        {
            // Select Service without retune since Ensemble is the current one

            int curServiceId = radioStatusGlobal->radioStatus.persistentData.serviceId;
            ServiceSelect(curServiceId);

            // Update global variables and inform the HMI about the new frequency and the band
            UpdateRadioStatusAndSendFeedback(radioStatusGlobal->radioStatus.persistentData.frequency,
                                             radioStatusGlobal->radioStatus.persistentData.band);
        }
    }
    else
    {
        if (BAND_FM == prsBand)
        {
            radioStatusGlobal->radioStatus.persistentData.frequency = curFreq;
            radioStatusGlobal->radioStatus.persistentData.frequencyFm = curFreq;

#if (defined CONFIG_USE_STANDALONE)
            rdsDecoder->ClearRdsData();
#endif // #if (defined CONFIG_USE_STANDALONE)
        }
        else if (BAND_AM == prsBand)
        {
            radioStatusGlobal->radioStatus.persistentData.frequencyAm =
                radioStatusGlobal->radioStatus.persistentData.frequency;
        }

#if (defined CONFIG_USE_STANDALONE)
        retValue = cmdCis->setFrequency(radioStatusGlobal->radioStatus.persistentData.frequency,
                                        radioStatusGlobal->radioStatus.persistentData.radioMode);

        if (BAND_FM == radioStatusGlobal->radioStatus.persistentData.band)
        {
            rdsDecoder->setRdsFmTuneFrequency(radioStatusGlobal->radioStatus.persistentData.frequency);
            cmdCis->RunRdsAcquisition(radioStatusGlobal->radioStatus.persistentData.radioMode);
        }
#endif // #if (defined CONFIG_USE_STANDALONE)

        // Update global variables and inform the HMI about the new frequency and the band
        UpdateRadioStatusAndSendFeedback(radioStatusGlobal->radioStatus.persistentData.frequency,
                                         radioStatusGlobal->radioStatus.persistentData.band);

#if (defined CONFIG_USE_ETAL)
        m_etal->setFrequency(radioStatusGlobal->radioStatus.persistentData.frequency);
#endif // #if (defined CONFIG_USE_ETAL)
    }

    return retValue;
}

unsigned int RadioManager::GetFirstServiceId()
{
    bool ok;
    unsigned int res;

    if (radioStatusGlobal->panelData.list.serviceList.size() > 0)
    {
        res = (radioStatusGlobal->panelData.list.serviceList.at(0).ServiceID.right(4)).toUInt(&ok, 16);

        return res;
    }

    return INVALID_SERVICE_ID;
}

int RadioManager::getServiceIndex(unsigned int serviceId)
{
    bool ok;
    int retServiceIdx = -1;
    int sizeServiceList = radioStatusGlobal->panelData.list.serviceList.size();

    // We have a valid service ID, serach for it
    for (int cnt = 0; cnt < sizeServiceList; cnt++)
    {
        if (serviceId == (radioStatusGlobal->panelData.list.serviceList.at(cnt).ServiceID.right(4)).toUInt(&ok, 16))
        {
            retServiceIdx = cnt;

            break;
        }
    }

    return retServiceIdx;
}

StatusCodeTy RadioManager::LearnProcedure()
{
    StatusCodeTy retValue = CMD_OK;

#if (defined CONFIG_USE_STANDALONE)
    cmdCis->Learn(radioStatusGlobal->radioStatus.persistentData.band,
                  radioStatusGlobal->radioStatus.persistentData.frequency,
                  radioStatusGlobal->radioStatus.persistentData.radioMode);
#endif // #if (defined CONFIG_USE_STANDALONE)

    return retValue;
}

StatusCodeTy RadioManager::PresetSave()
{
    StatusCodeTy retValue = CMD_OK;

    qDebug() << Q_FUNC_INFO;

    int presetToReplace = radioStatusGlobal->panelData.currPresetIndex;

    // Remove the Preset service at current index
    presetList->SetPreset(presetToReplace,
                          radioStatusGlobal->radioStatus.persistentData.serviceName,
                          radioStatusGlobal->radioStatus.persistentData.serviceId,
                          radioStatusGlobal->radioStatus.persistentData.frequency,
                          radioStatusGlobal->radioStatus.persistentData.band);

    return retValue;
}

dabQualityInfoTy RadioManager::GetDabQualInfo(bool active)
{
    dabQualityInfoTy qualDabInfo = { };

#if (defined CONFIG_USE_STANDALONE)
    cmdMw->SetQualAcquireStatus(active);
    cmdMw->CommandExecute(MIDW_GET_DAB_INFO, true);

    if (true == active)
    {
        // Get quality structure containing current quality values
        qualDabInfo = cmdMw->getQualityInfo();
    }
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
    Q_UNUSED(active);

    // TODO  Get Dab quality parameters
    // ?? qualDabInfo = m_etal->getQualityInfo();
#endif // #if (defined CONFIG_USE_ETAL)

    return qualDabInfo;
}

// End of file
