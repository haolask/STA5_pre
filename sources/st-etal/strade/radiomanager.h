#ifndef RADIOMANAGER_H
#define RADIOMANAGER_H

#include "target_config.h"

#include <QObject>
#include <QThread>
#include <QDebug>

#if (defined CONFIG_USE_ETAL)
#include "cmd_mngr_etal.h"
#endif // #if (defined CONFIG_USE_ETAL)

#if (defined CONFIG_USE_STANDALONE)
#include "common.h"
#include "cmd_mngr_cis.h"
#include "cmd_mngr_mw.h"
#include "rds.h"
#endif  // #if (defined CONFIG_USE_STANDALONE)

#include "tcptransportlayer.h"

#include "defines.h"
#include "shellwindow.h"
#include "filemanager.h"
#include "station_list_global.h"
#include "observer.h"
#include "event_manager.h"

#include "state_machine.h"
#include "radiomanagerbase.h"
#include "presets.h"

#include "protocollayer.h"

#include "postal_types.h"
#include "postaloffice.h"

#if (defined CONFIG_USE_ETAL)
#define RadioMngrContent                EtalContent

enum RadioMngrState  { Init, Playing, ChangeMode, Scan, Learn };

#endif // #if (defined CONFIG_USE_ETAL)

struct RadioManagerStatus
{
    // Service list related data
    bool serviceListConfirmed;
    int numberOfAlreadyStoredServices;
    int numberOfRetry;

    // Audio service status
    bool audioServiceIsPlaying;

    // Syncronization status
    bool dabSync;
};

struct CheckAliveData
{
    qint64 msFromEpoch;
    qint64 deltaFromLatestEvent;
};

class RadioManager : public QObject, public EventManagerObserver<eventsList, eventDataInterface>, public RadioManagerBase
{
    Q_OBJECT

    public:
        explicit RadioManager(QObject* parent = nullptr);

        ~RadioManager();

        void Initialize(void* par)
        {
            // Initialize class member to point to the passed address
            radioStatusGlobal = static_cast<RadioStatusGlobal *> (par);

            // Initialize service table list
            radioStatusGlobal->panelData.list.serviceList.clear();
        }

        void EventRaised(eventsList eventType, int size, eventDataInterface* eventData) override;

        void RadioLastStatus();
        void SetBand();
        void StartEvents();
        void StopEvents();
        void VolumeUp();
        void VolumeDown();
        void List() { }

        bool ProcessEvent(Events event, QVariant data);

        StatusCodeTy GetEventData(Events event, PostalOfficeBase *& postalOfficeBase);

        void SendBootResponse(PostalOfficeBase *& postalOfficeBase);
        void SendRadioLastStatus(PostalOfficeBase *& postalOfficeBase);
        void SendSourceResponse(PostalOfficeBase *& postalOfficeBase);

        StatusCodeTy Seek(bool upDirection);
        StatusCodeTy SetUp();
        StatusCodeTy SetQualityMode(bool enable);
        StatusCodeTy ClearAllPresets();

        StatusCodeTy NextFrequency(unsigned int newFreq, BandTy newBand, SystemMode newMode);
        StatusCodeTy BootOn();
        StatusCodeTy PowerOn();
        StatusCodeTy PowerOff();
        StatusCodeTy MuteToggle();
        StatusCodeTy ServiceSelect(unsigned int idValue = INVALID_SERVICE_ID);
        StatusCodeTy ServiceRemove();
        StatusCodeTy PresetSelect(int currentPresetIndex);
        StatusCodeTy PresetSave();
        StatusCodeTy EventsNotificationStop();
        StatusCodeTy EventsNotificationStart();
        StatusCodeTy PadOnDlsStop();
        StatusCodeTy SlsFromXpadStop();
        StatusCodeTy PadOnDlsStart();
        StatusCodeTy SlsFromXpadStart();
        StatusCodeTy Source(BandTy newBand);
        StatusCodeTy SetDcopFrequency(quint32 currFreq);
        StatusCodeTy setPadOnDls(bool padEn);
        StatusCodeTy setSlsFromXpad(bool slsEn);
        StatusCodeTy EnableNotification(bool notifyEn);
        StatusCodeTy GetEnsembleList();
        StatusCodeTy GetEnsembleData();
        StatusCodeTy GetServiceList();
        StatusCodeTy GetSpecificServiceData();
        StatusCodeTy SetFrequency(unsigned int newFreq, BandTy newBand, SystemMode newMode);
        StatusCodeTy DcopCheckAlive();

        bool UpdateStationList(EnsembleTableTy& tmpTable);

        bool CheckEventOngoing() { return eventOngoing; }
        void SetEventOngoing() { eventOngoing = true; }
        void ClearEventOngoing() { eventOngoing = false; }
        void UserEventPending() { userEventPending = true; }
        void UserEventNotPending() { userEventPending = false; }

        bool IsRadioActive();

    signals:
        void EventFromDeviceSignal(Events event);

#if (defined CONFIG_USE_ETAL)
        void error(Etal::Error error, QString msg);
        void logging(QString, QString, qint32);
        void seek_gl(void* state);
#endif // #if (defined CONFIG_USE_ETAL)

    public slots:
        void CommandForRadioManagerSlot(Events event, QVariant data);

#if (defined CONFIG_USE_ETAL)
        void seek_gl_slot(void* state)
        {
            emit seek_gl(state);
        }

        void error_slot(Etal::Error err, QString msg)
        {
            emit error(err,  msg);
        }

        void logging_slot(QString str1, QString str2, qint32 i)
        {
            emit logging(str1, str2, i);
        }
#endif // #if (defined CONFIG_USE_ETAL)

    private:
        void DisconnectSockets();
        bool ConnectSockets();

        void RadioMngrApplicationInit();
        StatusCodeTy NewTuneSequence(unsigned int newFreq, BandTy newBand, SystemMode newMode);
        void CheckAliveManagement();

        void SeekSequence(bool updirection);

#if (defined CONFIG_USE_STANDALONE)
        StatusCodeTy DabSeekTuneSequence();

        void SeekDabSequence(bool updirection);

        quint32  SeekFmSequence(bool updirection, quint32 currSeekFreq, SystemMode usedMode);
#endif  // #if (defined CONFIG_USE_STANDALONE)

        bool updateEnsembleIndex(bool updirection);   // Returns true if wrapped over band limits (useful for seek)

        //bool IdentifyServiceToExtract();

        StatusCodeTy GetInformationForEnsemble(BandTy band);

        void RequestAnotherCheckForServiceData();

        void CheckDabStatusResponse(qualityInfoTy& info);

        void UpdateGlobalStructures(bool onSuccess);

        StatusCodeTy ExtractAudioService(unsigned int serviceId);

        void SendTuneResponse(RadioTuneAnswer& tuneData);
        void SendAudioPlaysUpdate(bool audioPlays);
        void SendRdsData(RdsDataSetTableTy* rdsData);
        void SendQualityData(qualityInfoTy* qualInfo);

        unsigned int GetFirstServiceId();
        int getServiceIndex(unsigned int serviceId);
        void SaveCurrentBandData(BandTy cBand);
        void SetNewBandData(BandTy cBand);
        dabQualityInfoTy GetDabQualInfo(bool active);
        void initNotSelectedSource();
        StatusCodeTy LearnProcedure();

        void UpdateRadioStatusAndSendFeedback(unsigned int newFreq, BandTy newBand);

        RadioManagerStatus radioManagerStatus;

        CheckAliveData checkAliveData;

        bool eventOngoing;
        bool seekOngoing;  // Set to true, during Seek procedure
        qint8 dabSeekSyncLevel;  // Used by seek for DCOP request

        bool userEventPending;

        ShellWindow* shellWindow;
        Storage<PersistentData> radioStorage;

        Presets* presetList;
        std::list<RadioService>::iterator itPresets;
        RadioService* presetService;

        EventManager<eventsList, eventDataInterface>* eventManager;

        ProtocolLayer* protocolLayer;

        parametersProtLayerTy plParameters;

#if (defined CONFIG_USE_STANDALONE)
        TcpTransportLayer* tcpCmostTLayer;
        TcpTransportLayer* tcpDcopTLayer;

        CisCmds* cmdCis;
        Middleware* cmdMw;
        RdsDecoder* rdsDecoder;
#endif // #if (defined CONFIG_USE_STANDALONE)

#if (defined CONFIG_USE_ETAL)
        Etal* m_etal;
        RadioMngrContent m_cont[MODE_MAX];
        Mode m_mode;
        RadioMngrState m_state;

        static quint32 m_DABIIIFreqTable[MAX_NB_DAB_FREQUENCIES];
#endif // #if (defined CONFIG_USE_ETAL)
};

#endif // RADIOMANAGER_H
