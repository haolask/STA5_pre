#ifndef RADIOMANAGERMW_H
#define RADIOMANAGERMW_H

#include <QObject>

#include "cmd_mngr_base.h"
#include "radiomanagerbase.h"
#include "observer.h"
#include "event_manager.h"
#include "protocollayer.h"
#include "state_machine.h"
#include "shellwindow.h"

extern "C"
{
#include <hmi_if_app_request.h>
#include <hmi_if_extern.h>
}

#define TUNER_0 0
#define TUNER_1 1
#define RADIOMNGR_COLD_START
#define RADIOMNGR_COMMAND_TIMEOUT 20000 // in ms ~ 20sec.

typedef enum
{
    UP = 0,
    DOWN = 1,
} DirectioTy;

class RadioManagerJasmin : public QObject, public RadioManagerBase, public EventManagerObserver<eventsList, eventDataInterface>
{
    Q_OBJECT

    public:
        explicit RadioManagerJasmin(QObject* parent = nullptr);

        ~RadioManagerJasmin();

        void RadioLastStatus();

        StatusCodeTy BootOn();

        StatusCodeTy PowerOn();

        StatusCodeTy PowerOff();

        StatusCodeTy Source(BandTy newBand);

        StatusCodeTy MuteToggle();

        void VolumeUp();

        void VolumeDown();

        StatusCodeTy NextFrequency(unsigned int newFreq, BandTy newBand, SystemMode newMode);

        StatusCodeTy Seek(bool upDirection);

        void List();

        StatusCodeTy SetQualityMode(bool enable);

        bool ProcessEvent(Events event, QVariant data);

        void EventRaised(eventsList eventType, int size, eventDataInterface* eventData) override;

        void UserEventPending() { m_userEventPending = true; }
        void UserEventNotPending() { m_userEventPending = false; }

        bool IsRadioActive() { return true; }

        //static void radioNotificationHdlr(HMIIF_IDataObject* pData);
        static RadioManagerJasmin* getInstance();
        static void EndOfEvent(quint32 event);

        friend void radioNotificationHdlr(HMIIF_IDataObject* pData);

    public slots:
        void CommandForRadioManagerSlot(Events event, QVariant data);

    signals:
        void EventFromDeviceSignal(Events event);
        void EndOfEventSignal();
        void MessageSignal(QString eventStr, QString dataStr, int typeData);

    private:
        StatusCodeTy sendRequest(void (*handler)(), QList<quint32> events, quint32 errorevent, quint32 timeout = RADIOMNGR_COMMAND_TIMEOUT);
        void displayActivityStatus(quint32 data);
        void displayBandInfo(quint32 data);
        HMIIF_IDataObject sendRequest_(HMIIF_IDataObject (*handler)());
        void dataObjParser(HMIIF_IDataObject* pData);
        QString ret2string(quint32 in);
        StatusCodeTy checkEvents(QList<quint32>* eventList, QEventLoop* loop, quint32 fail);
        void clearEvents();

        // requests
        StatusCodeTy RequestBoot();
        StatusCodeTy RequestDismiss();
        StatusCodeTy RequestStart();
        StatusCodeTy RequestStop();
        StatusCodeTy RequestFreq(quint32 freq);
        StatusCodeTy RequestSeek(DirectioTy direction);
        StatusCodeTy RequestSource(BandTy band);

        // set section
        void SetFreq(quint32 freq)
        {
            //m_lock.lock();

            m_freq = freq;

            // Feedback RadioPanel with the current Frequency (1 = new frequency used instead of bool)
            eventDataInterface eventData;

            eventData.dataPtr = reinterpret_cast<unsigned char *> (&m_freq);
            eventData.size = sizeof (unsigned int);
            eventData.eventType = EVENTS_RX_FREQ;

            eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);

            //m_lock.unlock();
        }

        void SetBand(MODE_TYPE band)
        {
            m_lock.lock();
            radioStatusGlobal->radioStatus.persistentData.band = (BandTy)band;
            m_lock.unlock();
        }

        void SetServiceList(quint32 numb)
        {
            //            eventDataInterface eventData;

            //            eventData.dataPtr = (unsigned char *)&m_serviceTable;
            //            eventData.size = sizeof (EnsembleTableTy);

            //            eventData.eventType = EVENTS_UPDATE_SERVICE_LIST;

            //            eventManager->PushEventOnlyExternal(eventData.eventType, eventData.size, &eventData);

            m_serviceTable.ensFrequency = numb;
        }

        void SetProgramServiceName(qint8* ps);
        void SetServiceName(QString servStr);
        void SetRadioText(QString servStr);
        void SetRDS(QString PI, QString pty_str, bool TA, bool tp);

        ProtocolLayer* m_protocolLayer;
        bool m_eventOngoing;
        bool m_seekOngoing;
        ShellWindow* shellWindow;
        Storage<PersistentData> radioStorage;
        EventManager<eventsList, eventDataInterface>* eventManager;

        static QList<quint32> m_event;
        static RadioManagerJasmin* radioManager;

        bool m_userEventPending;
        quint32 m_freq;
        EnsembleTableTy m_serviceTable;

        QString m_rt;
        QString m_ps;
        QString m_pi;
        bool m_ta;
        bool m_tp;

        quint32 m_cntr;

        QMutex                       m_lock;
};

#endif // RADIOMANAGERMW_H
