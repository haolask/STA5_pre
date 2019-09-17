#include "radiomanager_jasmin.h"
#include <QDateTime>
#include <QThread>
#include <QString>
#include <QHash>
#include <fstream>      // std::ifstream

void radioNotificationHdlr(HMIIF_IDataObject* pData);

RadioManagerJasmin::RadioManagerJasmin(QObject* parent)
{
    Q_UNUSED(parent);

    qDebug() << Q_FUNC_INFO;

    // Initialize members
    m_eventOngoing = false;

    m_userEventPending = false;

    m_protocolLayer = new ProtocolLayer();
    m_protocolLayer->ProtocolLayerStart();

    radioManager = this;

    // shell window for logging
    shellWindow = new ShellWindow();

    // Initialize event manager
    eventManager = new EventManager<eventsList, eventDataInterface> (this);

    // connection with message signal
    QObject::connect(this, &RadioManagerJasmin::MessageSignal, shellWindow, &ShellWindow::inShellEtalData_slot);

    m_serviceTable.ensFrequency = 0;
    m_cntr = 0;

    /* Callback function to receive notifications from RadioLib */
    _pfnNotifyHdlr = radioNotificationHdlr;
}

void RadioManagerJasmin::EventRaised(eventsList eventType, int size, eventDataInterface* eventData)
{
    Q_UNUSED(eventType);
    Q_UNUSED(size);
    Q_UNUSED(eventData);
}

RadioManagerJasmin::~RadioManagerJasmin()
{
    if (COMMAND_COMPLETED_CORRECTLY != RequestDismiss())
    {
        qDebug() << Q_FUNC_INFO << "ERROR in Jasmin HM deinit";
    }

    delete shellWindow;
}

void RadioManagerJasmin::RadioLastStatus()
{
#if !defined (RADIOMNGR_COLD_START)
    // Load Persistent data
    std::ifstream inFileRadioStatus("radiostatus.bin", std::ios::in | std::ios::binary);

    // Check if we have a valid file
    if (inFileRadioStatus)
    {
        // Load data
        inFileRadioStatus >> radioStorage;

        // Get loaded data and store in the global variable
        radioStatusGlobal->radioStatus.persistentData = radioStorage.GetStatus();

        // Initialize dynamic data
        radioStatusGlobal->radioStatus.dynamicData.radioMute = true;

        if (radioStatusGlobal->radioStatus.persistentData.radioVolume > VOLUME_MAX_VALUE)
        {
            radioStatusGlobal->radioStatus.persistentData.radioVolume =  VOLUME_MAX_VALUE;
        }
    }
    else
    {
#endif // #if !defined(RADIOMNGR_COLD_START)
    // Set radio volume to a default initial value
    radioStatusGlobal->radioStatus.persistentData.radioVolume = VOLUME_MAX_VALUE / 2; // TODO: use the correct start value

    qDebug() << "Default hardcoded init. FM";

    // Default initialization FM
    radioStatusGlobal->radioStatus.persistentData.radioMode = MODE_FM;
    radioStatusGlobal->radioStatus.persistentData.band = BAND_FM;
    radioStatusGlobal->radioStatus.persistentData.serviceId = 0x232f;

    radioStatusGlobal->radioStatus.persistentData.frequency = 87500;
    radioStatusGlobal->radioStatus.persistentData.frequencyDab = 174928;
    radioStatusGlobal->radioStatus.persistentData.frequencyFm = 87500;
    radioStatusGlobal->radioStatus.persistentData.frequencyAm = 639;

    radioStatusGlobal->radioStatus.dynamicData.radioMute = true;
    radioStatusGlobal->radioStatus.dynamicData.radioState = STOPPED_STATE;
#if !defined (RADIOMNGR_COLD_START)
}
#endif // #if !defined(RADIOMNGR_COLD_START)

    radioStatusGlobal->radioStatus.persistentData.country = COUNTRY_EU;

#if 0
    // Load Presets data
    std::ifstream inFilePresetsTable("presets.bin", std::ios::in | std::ios::binary);

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
#endif
}

StatusCodeTy RadioManagerJasmin::BootOn()
{
    QString msg;

    qDebug() << Q_FUNC_INFO;
    msg = Q_FUNC_INFO;

#if !defined (RADIO_MNGR_HW_EMULATION)
    StatusCodeTy ret = RequestBoot();
#else
    StatusCodeTy ret = COMMAND_COMPLETED_CORRECTLY;
#endif // # if !defined (RADIO_MNGR_HW_EMULATION)

    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    return ret;
}

//#define RADIO_MNGR_HW_EMULATION

StatusCodeTy RadioManagerJasmin::PowerOn()
{
    QString msg;

#if !defined (RADIO_MNGR_HW_EMULATION)
    StatusCodeTy ret; //= RequestBoot();
    //StatusCodeTy
    ret = RequestStart();
#else
    StatusCodeTy ret = COMMAND_COMPLETED_CORRECTLY;
#endif // # if !defined (RADIO_MNGR_HW_EMULATION)

    msg = Q_FUNC_INFO;
    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    qDebug() << Q_FUNC_INFO;
    qDebug() << "SF DAB/DAB          " \
             << ((radioStatusGlobal->radioStatus.persistentData.dabDabServiceFollowingEn) ? QString("On") : QString("Off"));
    qDebug() << "SF DAB/FM           " \
             << ((radioStatusGlobal->radioStatus.persistentData.dabFmServiceFollowingEn) ? QString("On") : QString("Off"));
    qDebug() << "SF FM/FM            " \
             << ((radioStatusGlobal->radioStatus.persistentData.afCheckEn) ? QString("On") : QString("Off"));
    qDebug() << "Global Service list " \
             << ((radioStatusGlobal->radioStatus.persistentData.globalServiceListEn) ? QString("On") : QString("Off"));

#if !defined (RADIO_MNGR_HW_EMULATION)
    // !TBD make connection with setup screen
    //    Request_EnableAnnouncement_Info(RADIO_SWITCH_REQUEST_ENABLE);
    //    Request_EnableAnnouncement(RADIO_SWITCH_REQUEST_ENABLE);
    Request_EnableDABFMLinking(RADIO_SWITCH_REQUEST_DISABLE);
    Request_RDSFollowing(RADIO_SWITCH_REQUEST_DISABLE);
    Request_MultiplexList_Switch(RADIO_SWITCH_REQUEST_DISABLE);

    QThread::sleep(2);

    sendRequest_(Request_RadioSettings);
#endif // #endif // #if !defined (RADIO_MNGR_HW_EMULATION)

    return ret;
}

StatusCodeTy RadioManagerJasmin::PowerOff()
{
    QString msg;

    qDebug() << Q_FUNC_INFO;

    // Save Persistent data
    radioStorage.SetStatus(radioStatusGlobal->radioStatus.persistentData);
    std::ofstream outFileRadioStatus("radiostatus.bin", std::ios::out | std::ios::binary);
    outFileRadioStatus << radioStorage;

#if !defined (RADIO_MNGR_HW_EMULATION)
    // stop library
    return RequestStop();
#else
    msg = Q_FUNC_INFO;
    msg += ret2string(COMMAND_COMPLETED_CORRECTLY);
    emit MessageSignal(msg, QString('\n'), 1);

    return COMMAND_COMPLETED_CORRECTLY;
#endif
}

StatusCodeTy RadioManagerJasmin::Source(BandTy newBand)
{
    QString msg;

    qDebug() << Q_FUNC_INFO;
    msg = Q_FUNC_INFO;
#if !defined (RADIO_MNGR_HW_EMULATION)
    RequestSource(newBand);
#endif

    radioStatusGlobal->radioStatus.persistentData.band = newBand;

    msg += " " + QString::number(newBand) + " " + QString::number(radioStatusGlobal->radioStatus.persistentData.band);
    emit MessageSignal(msg, QString('\n'), 1);

    return COMMAND_COMPLETED_CORRECTLY;
}

StatusCodeTy RadioManagerJasmin::MuteToggle()
{
    Request_PlaySelectStation(m_cntr++);

    if (m_cntr >= m_serviceTable.ensFrequency)
    {
        m_cntr = 0;
    }

    //    Request_RDSFollowing(RADIO_SWITCH_REQUEST_ENABLE);

    return COMMAND_COMPLETED_CORRECTLY;
}

void RadioManagerJasmin::VolumeUp()
{ }

void RadioManagerJasmin::VolumeDown()
{ }

StatusCodeTy RadioManagerJasmin::NextFrequency(unsigned int newFreq, BandTy newBand, SystemMode newMode)
{
    QString msg;
    StatusCodeTy ret;

    Q_UNUSED(newBand);
    Q_UNUSED(newMode);

    // On frequency change we would like to play the first service because we
    // do not store last played service per each ensemble but only one
    radioStatusGlobal->panelData.serviceIndex = 0;
    memset(&radioStatusGlobal->radioStatus.persistentData.serviceName, 0, STATION_NAME_LEN);

    qDebug() << Q_FUNC_INFO << " freq: " << newFreq << " " << radioStatusGlobal->radioStatus.persistentData.frequency;

#if !defined (RADIO_MNGR_HW_EMULATION)
    ret = RequestFreq(newFreq);
#else
    // virtual tune to next freq
    ret = COMMAND_COMPLETED_CORRECTLY;
#endif // #if !defined (RADIO_MNGR_HW_EMULATION)

    return ret;
}

StatusCodeTy RadioManagerJasmin::Seek(bool upDirection)
{
    qDebug() << Q_FUNC_INFO;

#if !defined (RADIO_MNGR_HW_EMULATION)
    DirectioTy dir = (true == upDirection) ? UP : DOWN;
    StatusCodeTy ret = RequestSeek(dir);
    return ret;
#else
    Q_UNUSED(upDirection);
    return COMMAND_COMPLETED_CORRECTLY;
#endif // #if !defined (RADIO_MNGR_HW_EMULATION)
}

void RadioManagerJasmin::List()
{
    qDebug() << Q_FUNC_INFO;
}

// events from HMI
bool RadioManagerJasmin::ProcessEvent(Events event, QVariant data)
{
    QByteArray curRdsBuffer;
    BandTy newSource;
    qualityInfoTy qualInfo;

    qDebug() << Q_FUNC_INFO << " event " << event;

    // Avoid executing a command meanwhile another one is under execution
    if (true == m_eventOngoing)
    {
        if ((BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band) &&
            ((EVENT_SEEK_DOWN == event) || (EVENT_SEEK_UP == event) ||
             (EVENT_SOURCE == event)))
        //  (EVENT_SOURCE == event) || (EVENT_SETUP == event)))
        {
            // Stop currently active Seek procedure
            m_seekOngoing = false;

            // Force Exit from Seek algorythm
            return true;
        }
        else
        {
            // Return indication that event hasn't been executed
            return false;
        }
    }

    // Flag the event as ONGOING
    m_eventOngoing = true;

    switch (event)
    {
        case EVENT_RADIO_LASTSTATUS:
            RadioLastStatus();
            break;

        case EVENT_BOOT_ON:
            if (CMD_OK != BootOn())
            {
                qDebug() << "ERROR: boot failed";

                radioStatusGlobal->panelData.isHwConnectionOk = false;
            }
            break;

        case EVENT_POWER_UP:
            if (CMD_OK != PowerOn())
            {
                qDebug() << "ERROR: power-on command failed";

                // We have no service list: schedule a request
                //emit EventFromDeviceSignal(EVENT_AUTO_CHECK_DATA);
            }

            // RequestAnotherCheckForServiceData();
            break;

        case EVENT_POWER_DOWN:
            PowerOff();
            break;

        case EVENT_MUTE_TOGGLE:
            if (CMD_OK != MuteToggle())
            {
                qDebug() << "ERROR: mute command failed";
            }

            // radioStatusGlobal->radioStatus.dynamicData.radioMute =
            //         radioStatusGlobal->radioStatus.dynamicData.radioMute;
            break;

        case EVENT_SERVICE_SELECT:
            //if (data.isValid())
            //{
            //    int selectionNumber = data.ToInt();
            //
            //    ServiceSelect(selectionNumber);
            //}
            break;

        case EVENT_PRESET_SELECT:
        {
            if (data.isValid())
            {
                //int presetNumber = data.toInt();

                //      PresetSelect(presetNumber);
            }
        }
        break;

        case EVENT_PRESET_SAVE:
            //PresetSave();
            break;

        case EVENT_SOURCE:
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
                //RequestAnotherCheckForServiceData();
            }
            break;

        //  case EVENT_SETUP:
        //   // SetUp();
        //  break;

        //  case EVENT_STOP_SETUP:
        //   // StopSetupMode();
        //  break;

        case EVENT_QUALITY_ENABLE:
            SetQualityMode(true);
            break;

        case EVENT_QUALITY_DISABLE:
            SetQualityMode(false);
            break;

        case EVENT_PRESET_CLEAR:
            // ClearAllPresets();
            break;

        case EVENT_LEARN:
            // LearnProcedure();
            break;

        case EVENT_SEEK_UP:
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
            break;

        case EVENT_SEEK_DOWN:
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
            break;

        case EVENT_VOLUME_UP:
            VolumeUp();
            break;

        case EVENT_VOLUME_DOWN:
            VolumeDown();
            break;

        case EVENT_FREQUENCY_CHANGE:
            if (data.isValid())
            {
                unsigned int frequency = data.toInt();

                if (CMD_OK != NextFrequency(frequency,
                                            radioStatusGlobal->radioStatus.persistentData.band,
                                            radioStatusGlobal->radioStatus.persistentData.radioMode))
                {
                    qDebug() << "ERROR: frequency step failed";

                    // We have no service list: scheduele a request
                    //emit EventFromDeviceSignal(EVENT_AUTO_CHECK_DATA);
                }
                else
                {
                    radioStatusGlobal->radioStatus.persistentData.frequency = frequency;
                }

                qDebug() << "@@@@@@@@ band " << radioStatusGlobal->radioStatus.persistentData.band;
                qDebug() << "@@@@@@@@ freq " << radioStatusGlobal->radioStatus.persistentData.frequency;

                if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
                {
                    // RequestAnotherCheckForServiceData();
                }
            }
            break;

        case EVENT_CHECK_ALIVE:
            // DcopCheckAlive();
            qDebug() << "EVENT_CHECK_ALIVE";
            break;

        case EVENT_AUTO_CHECK_DATA:
        {
#if (defined CONFIG_USE_STANDALONE)
            StatusCodeTy retValue;

            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                if (false == radioManagerStatus.serviceListConfirmed)
                {
                    retValue = GetInformationForEnsemble();

                    if (CMD_OK == retValue)
                    {
                        // Try to extract the service
                        retValue = ExtractAudioService();
                    }

                    // Check service list status
                    if (true == radioManagerStatus.serviceListConfirmed)
                    {
                        // We have all information we need, ask middleware to send information also
                        // to worker for HMI
                        cmdMw->SendLastServiceList();
                    }
                }
                else if (false == radioManagerStatus.audioServiceIsPlaying)
                {
                    // Command audio service extraction
                    retValue = ExtractAudioService();
                }

                // Check if we need to do another request
                RequestAnotherCheckForServiceData();
#endif // #if (defined CONFIG_USE_STANDALONE)
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

        case EVENT_LIST:
            List();
            break;

        case EVENT_QUALITY_SNR:
            // Read Quality SNR data
#if (defined CONFIG_USE_STANDALONE)
            //            qualInfo.quality = cmdCis->getFmFieldStrength(radioStatusGlobal->radioStatus.persistentData.radioMode);
            if (BAND_DAB3 == radioStatusGlobal->radioStatus.persistentData.band)
            {
                qualInfo = cmdCis->getDabFieldStrength(radioStatusGlobal->radioStatus.persistentData.radioMode);
                qualInfo.dabQualityInfo = geDabQualInfo(true);
                qualInfo.qualFstBb = qualInfo.dabQualityInfo.baseband_level;       // Use oly 8LSB of BaseBand Level

                dabQualIsRunning = true;
            }
            else
            {
                qualInfo = cmdCis->getFmQualityParameters(radioStatusGlobal->radioStatus.persistentData.radioMode);
            }
            qualInfo.qualFicBer = radioStatusGlobal->radioStatus.dynamicData.qualityLevel;
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
            qDebug() << "Radio mngr";
            qualInfo = m_etal->getQuality();
#endif //#if (defined CONFIG_USE_ETAL)

            // if (qualInfo.quality > 20)
            if (qualInfo.fmQualityInfo.qualSNR > 20)
            {
                qualInfo.sync = true;
            }
            qDebug() << "qualInfo.sync  = " << qualInfo.sync;

            //SendFmQualityData (&qualInfo);

            break;

        default:
            qDebug() << "RADIO MANAGER received unknown event";
            break;
    }

    // Flag the event as completed
    m_eventOngoing = false;

    return true;
}

StatusCodeTy RadioManagerJasmin::SetQualityMode(bool enable)
{
    StatusCodeTy retValue = CMD_OK;

    Q_UNUSED(enable);

    // TODO: implement

    return retValue;
}

void RadioManagerJasmin::CommandForRadioManagerSlot(Events event, QVariant data)
{
    Q_UNUSED(event);
    Q_UNUSED(data);
    QString msg;

    qDebug() << Q_FUNC_INFO << " " << event;

    switch (event)
    {
        case EVENT_CHECK_ALIVE:
            //Request_ManualUpdateSTL();
            msg = Q_FUNC_INFO + QString(" band ") + QString::number(Request_GetRadioMode());
            emit MessageSignal(msg, QString('\n'), 1);

            sendRequest_(Request_GetRadioStationListData);

            break;

        default:
            break;
    }
}

RadioManagerJasmin * RadioManagerJasmin::radioManager = nullptr;
QList<quint32> RadioManagerJasmin::m_event;

RadioManagerJasmin * RadioManagerJasmin::getInstance()
{
    return radioManager;
}

void RadioManagerJasmin::EndOfEvent(quint32 event)
{
    RadioManagerJasmin::getInstance()->m_lock.lock();

    qDebug() << Q_FUNC_INFO << " start";

    if ((RadioManagerJasmin::getInstance() != nullptr) && (event != 0xffff))
    {
        RadioManagerJasmin::m_event.append(event);

        qDebug() << " adding " << QString::number(event);

        RadioManagerJasmin::getInstance()->EndOfEventSignal();
    }
    qDebug() << Q_FUNC_INFO << " stop";
    RadioManagerJasmin::getInstance()->m_lock.unlock();
}

// private members
StatusCodeTy RadioManagerJasmin::RequestBoot()
{
#if 1
    QTimer timer;
    QEventLoop loop;
    StatusCodeTy ret = COMMAND_COMPLETED_CORRECTLY;
    QString msg;
    Radio_EtalHardwareAttr R_Etal_Initparams;

    QList<quint32> eventList;

    qDebug() << Q_FUNC_INFO;
    msg = Q_FUNC_INFO;

    // define requested event order to finish
    eventList.clear();
    eventList << RADIO_ETALHWCONFIG_SUCCESS;

    timer.setSingleShot(true);

    QObject::connect(this,
                     static_cast<void (RadioManagerJasmin::*)()> (&RadioManagerJasmin::EndOfEventSignal),
                     &loop,
                     static_cast<void (QEventLoop::*)()> (&QEventLoop::quit));

    QObject::connect(this,  RadioManagerJasmin::EndOfEventSignal, &loop, QEventLoop::quit);
    connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);

    timer.start(10000);

    clearEvents();

#if defined (RADIOMNGR_COLD_START)
    // Remove the LSM & NVM files to make it a Cold start
    remove("D:\\ST_NVM.bin");
    remove("D:\\ST_LSM.bin");
#endif // #if !defined (RADIOMNGR_COLD_START)

    // Enable CMOST device
    R_Etal_Initparams.m_DCOPAttr.m_isDisabled = FALSE;      /* To be revisited for DAB enabled in market																	*/
    R_Etal_Initparams.m_DCOPAttr.m_doFlashDump = FALSE;     /* ETAL reads back the current DCOP firmware image to verify, if program flashed correctly						*/
    R_Etal_Initparams.m_DCOPAttr.m_doFlashProgram = FALSE;      /* ETAL writes firmware image into flash memory attached to DCOP												*/
    R_Etal_Initparams.m_DCOPAttr.m_doDownload = 0;      /* ETAL writes the DCOP image to the DCOP volatile memory. If m_doFlashProgram is TRUE, m_doDownload  is
                                                           FALSE	*/
    R_Etal_Initparams.m_DCOPAttr.m_cbGetImage = NULL;           /* Pointer to a function that ETAL calls to obtain a DCOP Firmware chunk										*/
    R_Etal_Initparams.m_DCOPAttr.m_pvGetImageContext = NULL;            /* ETAL copies this parameter as - is to the pvContext parameter of the m_cbGetImage
                                                                           function					*/
    R_Etal_Initparams.m_DCOPAttr.m_cbPutImage = NULL;           /* Pointer to a function that ETAL calls to provide to the application a DCOP Firmware block					*/
    R_Etal_Initparams.m_DCOPAttr.m_pvPutImageContext = NULL;            /* ETAL copies this parameter as - is to the pvContext parameter of the m_cbPutImage
                                                                           function					*/

    // STAR Tuner - 1 Attribute Configuration
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_isDisabled = FALSE;        /* This CMOST device is initialized and  used in the active ETAL configuration					*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_useXTALalignment = FALSE;      /* To be revisited Crystal Value alignment														*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_XTALalignment = 0;         /* XTAL adjustment value, considered only if the m_useXTALalignment flag is set to TRUE			*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_useCustomParam = 0xFF;         /* ETAL does not write any parameter to the CMOST device										*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_CustomParamSize = 0;           /* Number of entries in the m_CustomParam array													*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_CustomParam = NULL;            /* Pointer to an array of integers containing the custom parameters for this CMOST
                                                                               device		*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_useDownloadImage = 0;          /* ETAL uses the default images built in the TUNER_DRIVER										*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_DownloadImageSize = 500;           /* The size in bytes of the m_DownaloadImage array. ignored if
                                                                                   m_useDownaloadImage is set to 0	*/
    R_Etal_Initparams.m_tunerAttr[TUNER_0].m_DownloadImage = NULL;          /* Pointer to a one-dimensional array containing the CMOST firmware image or patches			*/

    // STAR Tuner - 2 Attribute Configuration

    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_isDisabled = FALSE;        /* This CMOST device is initialized and  used in the active ETAL configuration					*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_useXTALalignment = FALSE;      /* To be revisited Crystal Value alignment														*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_XTALalignment = 0;         /* XTAL adjustment value, considered only if the m_useXTALalignment flag is set to TRUE			*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_useCustomParam = 0xFF;         /* ETAL does not write any parameter to the CMOST device										*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_CustomParamSize = 0;           /* Number of entries in the m_CustomParam array													*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_CustomParam = NULL;            /* Pointer to an array of integers containing the custom parameters for this CMOST
                                                                               device		*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_useDownloadImage = 0;          /* ETAL uses the default images built in the TUNER_DRIVER										*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_DownloadImageSize = 200;           /* The size in bytes of the m_DownaloadImage array. ignored if
                                                                                   m_useDownaloadImage is set to 0	*/
    R_Etal_Initparams.m_tunerAttr[TUNER_1].m_DownloadImage = NULL;          /* Pointer to a one-dimensional array containing the CMOST firmware image or patches			*/

    Request_EtalHWConfig(R_Etal_Initparams);

    ret = checkEvents(&eventList, &loop, RADIO_ETALHWCONFIG_FAILURE);

    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    return ret;
#else
    return COMMAND_COMPLETED_CORRECTLY;
#endif
}

StatusCodeTy RadioManagerJasmin::RequestDismiss()
{
#if 1
    QTimer timer;
    QEventLoop loop;
    StatusCodeTy ret = COMMAND_COMPLETED_CORRECTLY;
    QString msg;

    QList<quint32> eventList;

    qDebug() << Q_FUNC_INFO;
    msg = Q_FUNC_INFO;

    // define requested event order to finish
    eventList.clear();
    eventList << RADIO_ETALHWDECONFIG_SUCCESS;

    timer.setSingleShot(true);

    connect(this,  RadioManagerJasmin::EndOfEventSignal, &loop, QEventLoop::quit);
    connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);

    timer.start(4000);

    clearEvents();

    Request_EtalHWDeconfig();

    ret = checkEvents(&eventList, &loop, RADIO_ETALHWDECONFIG_FAILURE);

    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    return ret;
#else
    return COMMAND_COMPLETED_CORRECTLY;
#endif
}

StatusCodeTy RadioManagerJasmin::RequestStart()
{
    QTimer timer;
    QEventLoop loop;
    StatusCodeTy ret = COMMAND_COMPLETED_CORRECTLY;
    QString msg;

    QList<quint32> eventList;

    qDebug() << Q_FUNC_INFO;
    msg = Q_FUNC_INFO;

    // define requested event order to finish
    eventList.clear();
    eventList << RADIO_STARTUP_SUCCESS << RADIO_DOID_STATION_INFO;

    timer.setSingleShot(true);

    connect(this,  RadioManagerJasmin::EndOfEventSignal, &loop, QEventLoop::quit);
    connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);

    timer.start(60000);

    clearEvents();

    Request_StartRadio(RADIO_VARIANT_A2, RADIO_MARKET_WESTERN_EUROPE, 7);

    ret = checkEvents(&eventList, &loop, RADIO_STARTUP_FAILURE);

    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    return ret;
}

StatusCodeTy RadioManagerJasmin::RequestStop()
{
    StatusCodeTy ret;
    QString msg = Q_FUNC_INFO;

    QList<quint32> eventList;

    eventList << RADIO_SHUTDOWN_SUCCESS;

    // shut down radio
    ret = sendRequest(Request_ShutDownTuner, eventList, RADIO_SHUTDOWN_FAILURE, RADIOMNGR_COMMAND_TIMEOUT * 2);

    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    return ret;
}

StatusCodeTy RadioManagerJasmin::RequestFreq(quint32 freq)
{
    QTimer timer;
    QEventLoop loop;
    QString msg;

    QList<quint32> eventList;

    Request_TuneByFrequency(freq);

    return COMMAND_COMPLETED_CORRECTLY;
}

StatusCodeTy RadioManagerJasmin::RequestSeek(DirectioTy direction)
{
    QTimer timer;
    QEventLoop loop;
    StatusCodeTy ret;
    QString msg;
    RADIO_DIRECTION dir = ((direction == UP) ? RADIO_DIRECTION_UP : RADIO_DIRECTION_DOWN);

    QList<quint32> eventList;

    qDebug() << Q_FUNC_INFO;
    msg = Q_FUNC_INFO;

    // define requested event order to finish
    eventList.clear();
    eventList << RADIO_SEEK_REQ_SUCCESS;

    timer.setSingleShot(true);

    connect(this,  RadioManagerJasmin::EndOfEventSignal, &loop, QEventLoop::quit);
    connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);

    timer.start(30000);
    clearEvents();

    Request_SeekStation(dir);

    ret = checkEvents(&eventList, &loop, RADIO_SEEK_REQ_FAILURE);

    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    return ret;
}

StatusCodeTy RadioManagerJasmin::RequestSource(BandTy band)
{
    MODE_TYPE mode;
    QTimer timer;
    QEventLoop loop;
    StatusCodeTy ret;
    QString msg;

    QList<quint32> eventList;

    qDebug() << Q_FUNC_INFO;
    msg = Q_FUNC_INFO;

    // define requested event order to finish
    eventList.clear();
    // sequence to wait for
    eventList << RADIO_SELECTBAND_SUCCESS;

    timer.setSingleShot(true);

    connect(this,  RadioManagerJasmin::EndOfEventSignal, &loop, QEventLoop::quit);
    connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);

    timer.start(60000);
    clearEvents();

    switch (band)
    {
        case BAND_AM:
            mode = RADIO_MODE_AM;
            break;

        case BAND_DAB3:
            mode =  RADIO_MODE_DAB;
            break;

        default:
            mode = RADIO_MODE_FM;
            break;
    }

    Request_ChangeRadioMode(mode);

    ret = checkEvents(&eventList, &loop, RADIO_SELECTBAND_FAILURE);

    msg += ret2string(ret);
    emit MessageSignal(msg, QString('\n'), 1);

    return COMMAND_COMPLETED_CORRECTLY;
}

// sends command waiting for confirmation
// in[0]: pointer to handler
// in[1]: sequence of events to exit command
// in[2]: error event to exit
// in[3]: timeout in ms
// return: status code
StatusCodeTy RadioManagerJasmin::sendRequest(void (*handler)(), QList<quint32> events, quint32 errorevent, quint32 timeout)
{
    QTimer timer;
    QEventLoop loop;
    StatusCodeTy ret;

    qDebug() << Q_FUNC_INFO;

    timer.setSingleShot(true);

    connect(this,  RadioManagerJasmin::EndOfEventSignal, &loop, QEventLoop::quit);
    connect(&timer,  QTimer::timeout, &loop, QEventLoop::quit);

    timer.start(timeout);
    clearEvents();

    (*handler)();  // call handler

    // wait for correct sequence || error || leave on timeout
    ret = checkEvents(&events, &loop, errorevent);

    return ret;
}

StatusCodeTy RadioManagerJasmin::checkEvents(QList<quint32>* eventList, QEventLoop* loop, quint32 fail)
{
    StatusCodeTy ret = COMMAND_COMPLETED_CORRECTLY;
    bool found = false;

    qDebug() << Q_FUNC_INFO;

    do
    {
        // Check if the wanted event is already present and in that case do not enter loop:
        // the answer has already been delivered
        for (auto eventsWaitingFor : *eventList)
        {
            for (auto eventsAlreadyGot : RadioManagerJasmin::m_event)
            {
                if (eventsAlreadyGot == eventsWaitingFor)
                {
                    found = true;

                    break;
                }
            }
        }

        if (nullptr != loop && false == found)
        {
            loop->exec();
        }

        m_lock.lock();

        qDebug() << "########## Wake UP ###################";
        qDebug() << "Events";

        for (int i = 0; i < m_event.count(); i++)
        {
            qDebug() << "   * " << m_event[i];
        }

        qDebug() << "List" << eventList->count();
        for (int i = 0; i < eventList->count(); i++)
        {
            qDebug() << "   * " << eventList->at(i);
        }
        qDebug() << "#######################################";

        if (m_event.isEmpty())
        {
            eventList->clear();
            ret = OPERATION_TIMEOUT;
        }
        else
        {
            do
            {
                if (m_event[0] == eventList->at(0))
                {
                    eventList->removeFirst();
                    if (eventList->isEmpty())
                    {
                        m_event.clear();
                        break;
                    }
                }
                if (fail == m_event[0])
                {
                    eventList->clear();
                    m_event.clear();
                    ret = GENERIC_FAILURE;
                    break;
                }
                m_event.removeFirst();
            }
            while (!m_event.isEmpty());
        }
        m_lock.unlock();
    }
    while (!eventList->isEmpty());

    qDebug() << "expected sequence received freq " << radioStatusGlobal->radioStatus.persistentData.frequency;

    return ret;
}

HMIIF_IDataObject RadioManagerJasmin::sendRequest_(HMIIF_IDataObject (*handler)())
{
    HMIIF_IDataObject obj = (*handler)();

    dataObjParser(&obj);

    return obj;
}

void RadioManagerJasmin::dataObjParser(HMIIF_IDataObject* pData)
{
    int dataObjStatus = 0;
    DWORD activeBand;
    DWORD data = 0;
    IObjectList* StnList;
    HMIIF_IDataObject* StnListObj;
    int StnListCount;
    int i;
    QString msg;

    dataObjStatus = pData->GetId(pData->thiss);

    switch (dataObjStatus)
    {
        case RADIO_DOID_STATUS:

            msg = "RADIO_DOID_STATUS\n";

            pData->Get(pData->thiss, RADIO_DOSID_STATUS, &data);
            switch (data)
            {
                case RADIO_STARTUP_SUCCESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_STARTUP_SUCCESS";
                    break;

                case RADIO_STARTUP_FAILURE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_STARTUP_FAILURE";
                    break;

                case RADIO_SELECTBAND_SUCCESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SELECTBAND_SUCCESS";
                    break;

                case RADIO_SELECTBAND_FAILURE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SELECTBAND_FAILURE";
                    break;

                case RADIO_STNLISTSELECT_REQ_SUCCESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_STNLISTSELECT_REQ_SUCCESS";
                    break;

                case RADIO_STNLISTSELECT_REQ_FAILURE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_STNLISTSELECT_REQ_FAILURE";
                    break;

                case RADIO_SEEK_REQ_SUCCESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SEEK_REQ_SUCCESS";
                    break;

                case RADIO_SEEK_NO_SIGNAL:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SEEK_NO_SIGNAL";
                    break;

                case RADIO_SEEK_REQ_FAILURE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SEEK_REQ_FAILURE";
                    break;

                case RADIO_SCAN_STARTED:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SCAN_STARTED";
                    break;

                case RADIO_SCAN_INPROGRESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SCAN_INPROGRESS";
                    break;

                case RADIO_SCAN_COMPLETE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SCAN_COMPLETE";
                    break;

                case RADIO_SHUTDOWN_SUCCESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SHUTDOWN_SUCCESS";
                    break;

                case RADIO_SHUTDOWN_FAILURE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_SHUTDOWN_FAILURE";
                    break;

                case RADIO_MANUAL_UPDATE_STL_SUCCESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_MANUAL_UPDATE_STL_SUCCESS";
                    break;

                case RADIO_MANUAL_UPDATE_STL_FAILURE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_MANUAL_UPDATE_STL_FAILURE";
                    break;

                case RADIO_TUNE_UP_DOWN_SUCCESS:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_TUNE_UP_DOWN_SUCCESS";
                    break;

                case RADIO_TUNE_UP_DOWN_FAILURE:
                    msg += " [HMI_TEST_APP] HMI APP Status: RADIO_TUNE_UP_DOWN_FAILURE";
                    break;

                default:
                    msg += " [HMI_TEST_APP] HMI APP Status Received: " + QString::number(data);
            }
            msg += "\n";
            break;

        case RADIO_DOID_ACTIVITY_STATE:

            msg = "RADIO_DOID_ACTIVITY_STATE\n";
            emit MessageSignal(msg, "\n", 1);
            msg.clear();

            pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE_BAND, &data);
            displayBandInfo(data);

            pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE, &data);
            displayActivityStatus(data);
            break;

        case RADIO_DOID_SETTINGS:

            msg = "RADIO_DOID_SETTINGS\n";

            /* Read DAB-FM Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_DABFM_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    msg += "[HMI_TEST_APP] DABFM Switch is Enabled\n";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    msg += "[HMI_TEST_APP] DABFM Switch is Disabled\n";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    msg += "[HMI_TEST_APP] DABFM Switch is Disabled\n";
                    break;
            }

            /* Read Announcement Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_ANNO_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    msg += "[HMI_TEST_APP] Announcement Switch is Enabled\n";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    msg += "[HMI_TEST_APP] Announcement Switch is Disabled\n";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    msg += "[HMI_TEST_APP] Announcement Switch is Disabled\n";
                    break;
            }

            /* Read RDS Followup Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_RDS_FOLLOWUP_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    msg += "[HMI_TEST_APP] RDS Followup Switch is Enabled\n";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    msg += "[HMI_TEST_APP] RDS Followup Switch is Disabled\n";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    msg += "[HMI_TEST_APP] RDS Followup Switch is Disabled\n";
                    break;
            }

            /* Read Info Announcement Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_INFO_ANNO_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    msg += "[HMI_TEST_APP] Info Announcement Switch is Enabled\n";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    msg += "[HMI_TEST_APP] Info Announcement Switch is Disabled\n";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    msg += "[HMI_TEST_APP] Info Announcement Switch is Disabled\n";
                    break;
            }

            /* Read Multiplex Emsemble list Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_MULTIPLEX_SWITCH_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    msg += "[HMI_TEST_APP] Multiplex Ensemble List Switch is Enabled\n";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    msg += "[HMI_TEST_APP] Multiplex Ensemble List Switch is Disabled\n";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    msg += "[HMI_TEST_APP] Multiplex Ensemble List Switch is Disabled\n";
                    break;
            }
            break;

        case RADIO_DOID_STATION_INFO:

            msg = "RADIO_DOID_STATION_INFO";
            emit MessageSignal(msg, QString('\n'), 1);
            msg.clear();

            /* Read Band Information */
            pData->Get(pData->thiss, RADIO_DOSID_BAND, &data);
            displayBandInfo(data);
            activeBand = data;

            /* Read Frequency Information */
            pData->Get(pData->thiss, RADIO_DOSID_FREQUENCY, &data);
            msg = "[HMI_TEST_APP] Frequency: " + QString::number(data) + "\n";

            /* Read Service Name Information */
            pData->Get(pData->thiss, RADIO_DOSID_SERVICENAME, &data);
            msg += "[HMI_TEST_APP] Service Name: " + QString((char *)data) + "\n";

            if (activeBand == RADIO_MODE_DAB)
            {
                /* Read Channel Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_CHANNELNAME, &data);
                msg += "[HMI_TEST_APP] Channel Name: " + QString::number(data) + "\n";

                /* Read Ensemble Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_ENSEMBLENAME, &data);
                msg += "[HMI_TEST_APP] Ensemble Name: " + QString((char *)data) + "\n";

                /* Read Current Service Number Information */
                pData->Get(pData->thiss, RADIO_DOSID_CURRENTSERVICENUMBER, &data);
                msg += "[HMI_TEST_APP] Current Service Number: " + QString::number(data) + "\n";

                /* Read information on total no. of services */
                pData->Get(pData->thiss, RADIO_DOSID_TOTALNUMBEROFSERVICE, &data);
                msg += "[HMI_TEST_APP] Total Services: " + QString::number(data) + "\n";
            }

            /* Read Radio Text Information */
            pData->Get(pData->thiss, RADIO_DOSID_RADIOTEXT, &data);
            msg += "[HMI_TEST_APP] Radio Text: " + QString((char *)data) + "\n";

            break;

        case RADIO_DOID_STATION_LIST_DISPLAY:

            StnList = ((IObjectList *)pData->thiss);
            pData->Get(StnList->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_BAND, &data);
            displayBandInfo(data);

            StnListCount = StnList->GetCount(StnList->thiss);

            msg += "[HMI_TEST_APP] Station List Information\n";
            msg += "**********************************************\n";
            msg += "Index\tFrequency\tPI/Service\tMatched Index\n";

            for (i = 0; i < StnListCount; i++)
            {
                StnListObj = StnList->GetAt(StnList->thiss, i);
                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_INDEX, &data);
                msg += "[" + QString::number(data) + "]\t";

                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_FREQUENCY, &data);
                msg += QString::number(data) + "\t";

                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_SERVICENAME, &data);
                msg += QString((char *)data) + "\t";

                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_TUNEDSTN_INDEX, &data);
                msg += "\t " + QString::number(data) + "\n";
            }
            msg += "**********************************************\n";

            break;

        case RADIO_DOID_BESTPI_INFO:
            msg += "Best PI\n";
            pData->Get(pData->thiss, RADIO_DOSID_BESTPI_FREQUENCY, &data);
            msg += "Tuner Frequency " + QString::number(data) + "\n";
            pData->Get(pData->thiss, RADIO_DOSID_BESTPI_PI, &data);
            msg += "Tuner PI " + QString("%1").arg(data, 0, 16) + "\n";
            pData->Get(pData->thiss, RADIO_DOSID_BESTPI_QUALITY, &data);
            msg += "Tuner Quality " + QString::number(data) + "\n";
            break;

        case RADIO_DOID_FIRMWARE_VERSION_INFO:
            msg += "FW version\n";
            pData->Get(pData->thiss, RADIO_DOSID_AMFMTUNER_FIRMWARE_VERSION, &data);
            msg += "Tuner AMFM FW " + QString((char *)data) + "\n";
            pData->Get(pData->thiss, RADIO_DOSID_DABTUNER_HARDWARE_VERSION, &data);
            msg += "Tuner DAB HW " + QString((char *)data) + "\n";
            pData->Get(pData->thiss, RADIO_DOSID_DABTUNER_FIRMWARE_VERSION, &data);
            msg += "Tuner DAB FW " + QString((char *)data) + "\n";
            break;

        default:
            msg += "[HMI_TEST_APP] Invalid ID received\n";
            break;
    }

    emit MessageSignal(msg, QString('\n'), 1);
}

void RadioManagerJasmin::displayActivityStatus(quint32 data)
{
    QString msg;

    switch (data)
    {
        case RADIO_STATION_NOT_AVAILABLE:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_STATION_NOT_AVAILABLE";
        }
        break;

        case RADIO_FM_AF_PROCESSING:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_FM_AF_PROCESSING";
        }
        break;

        case RADIO_FM_INTERNAL_SCAN_PROCESS:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_FM_INTERNAL_SCAN_PROCESS";
        }
        break;

        case RADIO_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING";
        }
        break;

        case RADIO_DAB_AF_PROCESSING:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DAB_AF_PROCESSING";
        }
        break;

        case RADIO_DAB_INTERNAL_SCAN_PROCESS:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DAB_INTERNAL_SCAN_PROCESS";
        }
        break;

        case RADIO_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING";
        }
        break;

        case RADIO_FM_LEARNMEM_AF_PROCESSING:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_FM_LEARNMEM_AF_PROCESSING";
        }
        break;

        case RADIO_DAB_LEARNMEM_AF_PROCESSING:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DAB_LEARNMEM_AF_PROCESSING";
        }
        break;

        case RADIO_SIGNAL_LOSS:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_SIGNAL_LOSS";
        }
        break;

        case RADIO_DAB_DAB_STARTED:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DAB_DAB_STARTED";
        }
        break;

        case RADIO_DAB_DAB_LINKING_DONE:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DAB_DAB_LINKING_DONE";
        }
        break;

        case RADIO_LISTENING:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_LISTENING";
        }
        break;

        case RADIO_IN_SCAN:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_IN_SCAN";
        }
        break;

        case RADIO_TUNE_UPDOWN:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_TUNE_UPDOWN";
        }
        break;

        case RADIO_ANNOUNCEMENT:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_ANNOUNCEMENT";
        }
        break;

        case RADIO_DABFMLINKING_DONE:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DABFMLINKING_DONE";
        }
        break;

        case RADIO_IMPLICIT_LINKING_DONE:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_IMPLICIT_LINKING_DONE";
        }
        break;

        case RADIO_AF_SWITCHING_ESTABLISHED:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_AF_SWITCHING_ESTABLISHED";
        }
        break;

        case RADIO_DABTUNER_ABNORMAL:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_DABTUNER_ABNORMAL";
        }
        break;

        case RADIO_STATUS_INVALID:
        {
            msg = "[HMI_TEST_APP] Activity Status = RADIO_STATUS_INVALID";
        }
        break;
    }

    qDebug() << msg;
    emit MessageSignal(msg, QString('\n'), 1);
}

void RadioManagerJasmin::displayBandInfo(quint32 data)
{
    QString msg;

    switch (data)
    {
        case RADIO_MODE_AM:
            msg = "[HMI_TEST_APP] Current Band: AM";
            break;

        case RADIO_MODE_FM:
            msg = "[HMI_TEST_APP] Current Band: FM";
            break;

        case RADIO_MODE_DAB:
            msg = "[HMI_TEST_APP] Current Band: DAB";
            break;

        case RADIO_MODE_INVALID:
            msg = "[HMI_TEST_APP] Current Band: INVALID";
            break;
    }

    emit MessageSignal(msg, QString('\n'), 1);
}

QString RadioManagerJasmin::ret2string(quint32 in)
{
    QHash<int, QString> hash;

    hash.insert(OPERATION_TIMEOUT, " timeout");
    hash.insert(COMMAND_COMPLETED_CORRECTLY, " done");
    hash.insert(RADIO_STARTUP_FAILURE, " fail");
    hash.insert(OPERATION_FAILED, " operation fail");

    return hash[in];
}

void RadioManagerJasmin::clearEvents()
{
    m_lock.lock();
    m_event.clear();
    m_lock.unlock();
}

void RadioManagerJasmin::SetServiceName(QString servStr)
{
    eventDataInterface eventData;
    QByteArray a;

    m_ps = servStr;

    qDebug() << "service name "  << servStr;

    a.append(servStr);
    eventData.dataPtr = (unsigned char *)a.data();
    eventData.size = a.size();

    eventData.eventType = EVENTS_RX_SERVICE_NAME;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void RadioManagerJasmin::SetRadioText(QString rt)
{
    eventDataInterface eventData;
    QByteArray a;

    m_rt = rt;

    qDebug() << "Radio Text " << rt;

    a.append(rt);
    eventData.dataPtr = (unsigned char *)a.data();
    eventData.size = a.size();
    eventData.eventType = EVENTS_RX_DLS;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void RadioManagerJasmin::SetRDS(QString PI, QString pty_str, bool ta, bool tp)
{
    eventDataInterface eventData;
    RdsDataSetTableTy  rdsData;

    // fill export structure with current content
    rdsData.PIid = PI;
    rdsData.PSname = m_ps;
    rdsData.rtText = m_rt;
    if ("Pop M" == pty_str)
    {
        rdsData.ptyVal = 10;
    }
    else if ("Other M" == pty_str)
    {
        rdsData.ptyVal = 15;
    }
    else if ("Country" == pty_str)
    {
        rdsData.ptyVal = 25;
    }
    else if ("Varied" == pty_str)
    {
        rdsData.ptyVal = 9;
    }
    else if ("News" == pty_str)
    {
        rdsData.ptyVal = 1;
    }
    else
    {
        rdsData.ptyVal = 0;
    }

    rdsData.ptyName = pty_str;
    rdsData.tpFlag = tp;
    rdsData.taFlag = ta;
    rdsData.msFlag = 0;
    for (quint32 i = 0; i < 26; i++)
    {
        rdsData.RdsAFList[i] = 0;
    }

    eventData.dataPtr = (unsigned char *)&rdsData;
    eventData.size = sizeof (RdsDataSetTableTy);
    eventData.eventType = EVENTS_UPDATE_RDS_DATA;

    eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);
}

void radioNotificationHdlr(HMIIF_IDataObject* pData)
{
    int dataObjStatus = 0;
    DWORD activeBand;
    DWORD data = 0;
    IObjectList* StnList;
    HMIIF_IDataObject* StnListObj;
    int StnListCount;
    int i;

    dataObjStatus = pData->GetId(pData->thiss);

    switch (dataObjStatus)
    {
        case RADIO_DOID_STATUS:

            qDebug() << "@ RADIO_DOID_STATUS";

            pData->Get(pData->thiss, RADIO_DOSID_STATUS, &data);
            switch (data)
            {
                case RADIO_ETALHWCONFIG_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWCONFIG_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_ETALHWCONFIG_SUCCESS);
                    break;

                case RADIO_ETALHWCONFIG_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWCONFIG_FAILURE";
                    RadioManagerJasmin::EndOfEvent(RADIO_ETALHWCONFIG_FAILURE);
                    break;

                case RADIO_STARTUP_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_STARTUP_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_STARTUP_SUCCESS);
                    break;

                case RADIO_STARTUP_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_STARTUP_FAILURE";
                    RadioManagerJasmin::EndOfEvent(RADIO_STARTUP_FAILURE);
                    break;

                case RADIO_SELECTBAND_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SELECTBAND_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_SELECTBAND_SUCCESS);
                    break;

                case RADIO_SELECTBAND_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SELECTBAND_FAILURE";
                    RadioManagerJasmin::EndOfEvent(RADIO_SELECTBAND_FAILURE);
                    break;

                case RADIO_STNLISTSELECT_REQ_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_STNLISTSELECT_REQ_SUCCESS";
                    break;

                case RADIO_STNLISTSELECT_REQ_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_STNLISTSELECT_REQ_FAILURE";
                    break;

                case RADIO_SEEK_REQ_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SEEK_REQ_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_SEEK_REQ_SUCCESS);
                    break;

                case RADIO_SEEK_NO_SIGNAL:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SEEK_NO_SIGNAL";

                //                break;
                case RADIO_SEEK_REQ_FAILURE:
                    RadioManagerJasmin::EndOfEvent(RADIO_SEEK_REQ_FAILURE);
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SEEK_REQ_FAILURE";
                    break;

                case RADIO_SCAN_STARTED:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SCAN_STARTED";
                    break;

                case RADIO_SCAN_INPROGRESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SCAN_INPROGRESS";
                    break;

                case RADIO_SCAN_COMPLETE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SCAN_COMPLETE";
                    break;

                case RADIO_SHUTDOWN_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SHUTDOWN_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_SHUTDOWN_SUCCESS);
                    break;

                case RADIO_SHUTDOWN_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_SHUTDOWN_FAILURE";
                    RadioManagerJasmin::EndOfEvent(RADIO_SHUTDOWN_FAILURE);
                    break;

                case RADIO_MANUAL_UPDATE_STL_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_MANUAL_UPDATE_STL_SUCCESS";
                    break;

                case RADIO_MANUAL_UPDATE_STL_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_MANUAL_UPDATE_STL_FAILURE";
                    break;

                case RADIO_TUNE_UP_DOWN_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_TUNE_UP_DOWN_SUCCESS";
                    break;

                case RADIO_TUNE_UP_DOWN_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_TUNE_UP_DOWN_FAILURE";
                    break;

                case RADIO_POWERON_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_POWERON_FAILURE";
                    RadioManagerJasmin::EndOfEvent(RADIO_POWERON_FAILURE);
                    break;

                case RADIO_POWERON_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_POWERON_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_POWERON_SUCCESS);
                    break;

                case RADIO_POWEROFF_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_POWEROFF_FAILURE";
                    RadioManagerJasmin::EndOfEvent(RADIO_POWEROFF_FAILURE);
                    break;

                case RADIO_POWEROFF_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_POWEROFF_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_POWEROFF_SUCCESS);
                    break;

                case RADIO_ETALHWDECONFIG_SUCCESS:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWDECONFIG_SUCCESS";
                    RadioManagerJasmin::EndOfEvent(RADIO_ETALHWDECONFIG_SUCCESS);
                    break;

                case RADIO_ETALHWDECONFIG_FAILURE:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWDECONFIG_FAILURE";
                    RadioManagerJasmin::EndOfEvent(RADIO_ETALHWDECONFIG_FAILURE);
                    break;

                default:
                    qDebug() << "[HMI_TEST_APP] HMI APP Status Received: " << data;
            }
            break;

        case RADIO_DOID_ACTIVITY_STATE:

            qDebug() << "@ RADIO_DOID_ACTIVITY_STATE";

            pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE_BAND, &data);
            //displayBandInfo(data);

            pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE, &data);
            //RadioManagerJasmin::getInstance()->displayActivityStatus(data);
            break;

        case RADIO_DOID_SETTINGS:

            qDebug() << "@ RADIO_DOID_SETTINGS";

            /* Read DAB-FM Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_DABFM_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    qDebug() << "[HMI_TEST_APP] DABFM Switch is Enabled";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    qDebug() << "[HMI_TEST_APP] DABFM Switch is Disabled";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    qDebug() << "[HMI_TEST_APP] DABFM Switch is Disabled";
                    break;
            }

            /* Read Announcement Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_ANNO_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    qDebug() << "[HMI_TEST_APP] Announcement Switch is Enabled";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    qDebug() << "[HMI_TEST_APP] Announcement Switch is Disabled";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    qDebug() << "[HMI_TEST_APP] Announcement Switch is Disabled";
                    break;
            }

            /* Read RDS Followup Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_RDS_FOLLOWUP_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    qDebug() << "[HMI_TEST_APP] RDS Followup Switch is Enabled";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    qDebug() << "[HMI_TEST_APP] RDS Followup Switch is Disabled";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    qDebug() << "[HMI_TEST_APP] RDS Followup Switch is Disabled";
                    break;
            }

            /* Read Info Announcement Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_INFO_ANNO_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    qDebug() << "[HMI_TEST_APP] Info Announcement Switch is Enabled";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    qDebug() << "[HMI_TEST_APP] Info Announcement Switch is Disabled";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    qDebug() << "[HMI_TEST_APP] Info Announcement Switch is Disabled";
                    break;
            }

            /* Read Multiplex Emsemble list Switch Settings */
            pData->Get(pData->thiss, RADIO_DOSID_MULTIPLEX_SWITCH_SETTING_STATUS, &data);
            switch (data)
            {
                case RADIO_SWITCH_SETTING_ON:
                    qDebug() << "[HMI_TEST_APP] Multiplex Ensemble List Switch is Enabled";
                    break;

                case RADIO_SWITCH_SETTING_OFF:
                    qDebug() << "[HMI_TEST_APP] Multiplex Ensemble List Switch is Disabled";
                    break;

                case RADIO_SWITCH_SETTING_INVALID:
                    qDebug() << "[HMI_TEST_APP] Multiplex Ensemble List Switch is Disabled";
                    break;
            }
            break;

        case RADIO_DOID_STATION_INFO:

            qDebug() << "@ RADIO_DOID_STATION_INFO";

            /* Read Band Information */
            pData->Get(pData->thiss, RADIO_DOSID_BAND, &data);
            RadioManagerJasmin::getInstance()->SetBand((MODE_TYPE)data);

            activeBand = data;

            /* Read Frequency Information */
            pData->Get(pData->thiss, RADIO_DOSID_FREQUENCY, &data);
            qDebug() << "[HMI_TEST_APP] Frequency: " << data;
            RadioManagerJasmin::getInstance()->SetFreq(data);

            /* Read Service Name Information */
            pData->Get(pData->thiss, RADIO_DOSID_SERVICENAME, &data);
            qDebug() << "[HMI_TEST_APP] Service Name: " << (char *)data;
            RadioManagerJasmin::getInstance()->SetServiceName(QString((char *)data));

            if (activeBand == RADIO_MODE_DAB)
            {
                /* Read Channel Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_CHANNELNAME, &data);
                qDebug() << "[HMI_TEST_APP] Channel Name: " << data;

                /* Read Ensemble Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_ENSEMBLENAME, &data);
                qDebug() << "[HMI_TEST_APP] Ensemble Name: " << (char *)data;

                /* Read Current Service Number Information */
                pData->Get(pData->thiss, RADIO_DOSID_CURRENTSERVICENUMBER, &data);
                qDebug() << "[HMI_TEST_APP] Current Service Number: " << data;

                /* Read information on total no. of services */
                pData->Get(pData->thiss, RADIO_DOSID_TOTALNUMBEROFSERVICE, &data);
                qDebug() << "[HMI_TEST_APP] Total Services: " << data;
            }
            else if (activeBand == RADIO_MODE_FM)
            {
                bool ta, tp;
                QString pty_str;

                /* Read Channel Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_PROGRAMME_TYPE, &data);

                qDebug() << "[HMI_TEST_APP] Programme Type: " << (char *)data;

                /* Read Channel Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_TA, &data);
                ta = data;
                qDebug() << "[HMI_TEST_APP] TA: " << QString::number(data);

                /* Read Channel Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_TP, &data);
                tp = data;
                qDebug() << "[HMI_TEST_APP] TP: " <<  QString::number(data);

                /* Read Channel Name Information */
                pData->Get(pData->thiss, RADIO_DOSID_PI, &data);
                qDebug() << "[HMI_TEST_APP] PI: " <<  QString("%1").arg(data, 0, 16);

                RadioManagerJasmin::getInstance()->SetRDS(QString("%1").arg(data, 0, 16), pty_str, ta, tp);
            }

            /* Read Radio Text Information */
            pData->Get(pData->thiss, RADIO_DOSID_RADIOTEXT, &data);
            qDebug() << "[HMI_TEST_APP] Radio Text: " << (char *)data;
            RadioManagerJasmin::getInstance()->SetRadioText(QString((char *)data));
            RadioManagerJasmin::EndOfEvent(RADIO_DOID_STATION_INFO);
            break;

        case RADIO_DOID_STATION_LIST_DISPLAY:

            StnList = ((IObjectList *)pData->thiss);
            pData->Get(StnList->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_BAND, &data);

            switch (data)
            {
                case RADIO_MODE_AM:
                    qDebug() << "[HMI_TEST_APP] Current Band: AM";
                    break;

                case RADIO_MODE_FM:
                    qDebug() << "[HMI_TEST_APP] Current Band: FM";
                    break;

                case RADIO_MODE_DAB:
                    qDebug() <<  "[HMI_TEST_APP] Current Band: DAB";
                    break;

                case RADIO_MODE_INVALID:
                    qDebug() <<  "[HMI_TEST_APP] Current Band: INVALID";
                    break;
            }

            StnListCount = StnList->GetCount(StnList->thiss);

            qDebug() << "[HMI_TEST_APP] Station List Information";
            qDebug() << "**********************************************";
            qDebug() << "Index\tFrequency\tPI/Service\tMatched Index";

            RadioManagerJasmin::getInstance()->SetServiceList(StnListCount);

            for (i = 0; i < StnListCount; i++)
            {
                StnListObj = StnList->GetAt(StnList->thiss, i);
                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_INDEX, &data);
                qDebug() << "[" << data << "]\t";

                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_FREQUENCY, &data);
                qDebug() << data << "\t";

                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_SERVICENAME, &data);
                qDebug() << (char *)data << "\t";

                StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_TUNEDSTN_INDEX, &data);
                qDebug() << "\t " << data;
            }
            qDebug() << "**********************************************";

            break;

        default:
            qDebug() << "[HMI_TEST_APP] Invalid ID received";
            break;
    }
}

// End of file
