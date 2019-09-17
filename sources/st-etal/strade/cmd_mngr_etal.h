#ifndef ETAL_H
#define ETAL_H

#include "target_config.h"
#include "common.h"
#include "event_manager.h"
#include "cmd_mngr_base.h"

#if (defined CONFIG_USE_ETAL)

#include <QObject>
#include <QFile>
#include <mutex>

extern "C" {
#include <etal_target_config.h>
#include <etal_api.h>
}

#define ETAL_MNGR_LEARN_SIZE 50
#define ETAL_MNGR_DAB_SYNCHRONIZED 3
#define ETAL_MNGR_DAB_READY 7
#define ETAL_MNGR_DAB_UPDATE_SWITCH_OFF_CNTR 2
#define ETAK_MNGR_DELAY 1

typedef union
{
    struct
    {
        quint32 ens : 1;
        quint32 services : 1;
        quint32 info : 1;
    }  b;

    quint32 w;
} DABUpdateState;

class EtalContent
{
    //    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    //    Q_PROPERTY(Band band READ band WRITE setBand NOTIFY bandChanged)

    public:
        Mode                        m_mode;
        Band                        m_band;
        quint32                     m_frequency;

        quint16                     m_id;   // program identification
        QString                     m_name; // program name
        quint32                     m_fend; // front end to be connected
};

// Ensemble info
class EtalDABEnsInfo
{
    public:
        quint32                     m_ueid;     //Ensemble Identifier
        quint8                      m_charset;
        QString                     m_label;    //Ensemble Name
        quint16                     m_charFlag;
};

// service info
class EtalDABServiceInfo
{
    public:
        quint16                     m_id;
        quint16                     m_bitrate;
        QString                     m_label;
};

class EtalDABInfo
{
    public:
        qint32                      m_idx;   // index of selected service
        EtalDABEnsInfo              m_ens;
        QList<EtalDABServiceInfo>   m_service;
};

class Etal : public QObject, public CmdMngrBase
{
    Q_OBJECT
    Q_ENUMS(Error)

    public:
        enum Error  { NoError, ResourceError, OpenError, OutOfRangeError };

        explicit Etal(QObject* parent = 0);

        ~Etal();

        // Get section
        Band            band()              const {   return m_cont.m_band;            }
        Mode            mode()              const {   return m_cont.m_mode;            }
        quint32         frequency()         const {   return m_cont.m_frequency;       }
        qint32          signalStrength()    const {   return m_signalStrength;         }
        quint16         id()                const {   return m_cont.m_id;              }
        QString         name()              const {   return m_cont.m_name;            }
        bool            booted()            const {   return m_boot;                   }

        QString         GetRadioText()      const { return m_radio_text; }

        quint8          pty()               const {   return m_pty;                    }
        bool            isEnsAvailable()    const {   return m_ens_available;          }
        QString         pty_str()           const;
        quint32         getIndexFromFreq(quint32 freq);
        void            getEnsembleList();
        quint32         getCurrentEnsemble();
        QList<quint32>  getServiceList(qint32 ens, bool audioServ = true, bool dataServ = true);
        bool            getEnsembleInfo();
        bool            getServiceListInfo() {  return getServicesInfo(m_dabInfo.m_ens.m_ueid); }
        bool            getServicesInfo(qint32 ens);
        EnsembleTableTy getEnsTable();
        ServiceListTy   getServices();

        EtalDABInfo& getEnsembleContent();
        EtalContent& getContent();

        // Enable/disable section
        void            enableSLS(bool en);
        void            enableDLS(bool en);
        void            enableQualMonitor(bool en);
        void            enableRDS(bool en);
        void            enableSF(bool en);

        // Set section
        void            setRadioText(QString dlsStr);
        void            setSLS(QByteArray slsArray);
        void            setEnsembleName();
        void            setServiceName(QString servStr);
        void            setServiceList();
        void            setVolume(qint32 vol);
        void            setSyncStatus(qualityInfoTy* qualInfo);
        void            setFrequencyUpdate(quint32 frequency);
        void            setRdsData(EtalRDSData* prds);
        void            setPhaseDiversity(bool pd) {    m_phase_diversity = pd;     }

        void            selectService();
        bool            selectAudioService(qint32 ens, quint16 id);
        void            selectDataService(qint32 ens, qint32 id);
        void            monitorUpdate(EtalBcastQualityContainer* pQuality);
        void            ensembleInfo();
        void            seekHandler(void* status);

        friend void     etalRDSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
        friend void     etalRadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
        friend void     etalSLSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
        friend void     etalUserNotificationHandler(void* context, ETAL_EVENTS dwEvent, void* pstatus);
        friend void     etalQualityMonitor(EtalBcastQualityContainer* pQuality, void* vpContext);

    signals:
        void bandChanged(Band);
        void frequencyChanged(int);
        void signalStrengthChanged(int);
        void seekFreqFoundSignal();
        void seekFullCycleSignal();
        void seekStationList(void* status);

        void error(Etal::Error error, QString msg);
        void logging(QString, QString, qint32);
        void message(QString, QString, qint32);

    public slots:
        void boot();

        //    void getContent(EtalContent* cntnt);
        void setContent(EtalContent const* cntnt);

        void setBand(Band band);
        void setFrequency(quint32 frequency);
        void setMute(bool mute);
        void powerOn();
        void powerOff();
        void source();
        void updateFrequency(quint32 frequency);
        void setSelectService(); // takes valule from content
        bool updateServiceList();
        void seekUp(qint32 step);
        void seekDown(qint32 step);
        bool serviceSelect(int servIndex);
        void learn();
        qualityInfoTy getQuality(EtalBcastQualityContainer* pQuality = NULL);

    private:
        void sendSeekFreqFoundSignal()
        {
            // Clear persistant
            m_dabInfo.m_service.clear();
            m_cont.m_id = 0;
            m_update_cntr = 0;
            m_cont.m_name.clear();
            m_radio_text.clear();
            m_seek = false;

            // stop auto seek state machine
            seekStop();

            emit seekFreqFoundSignal();
        }

        void sendSeekFullCycleSignal()
        {
            // Clear persistant
            m_dabInfo.m_service.clear();
            m_cont.m_id = 0;
            m_update_cntr = 0;
            m_cont.m_name.clear();
            m_radio_text.clear();
            m_seek = false;

            // stop etal state machine
            seekStop();

            emit seekFullCycleSignal();
        }

        void monitorUpdateDAB(EtalBcastQualityContainer* pQuality);
        void monitorUpdateFM(EtalBcastQualityContainer* pQuality);
        void monitorUpdateAM(EtalBcastQualityContainer* pQuality);
        void updateDAB(EtalTuneStatus* status);
        void updateDABState();
        void seekStop();

        bool                        m_boot;
        bool                        m_seek;
        bool                        m_phase_diversity;
        bool                        m_ens_available;
        bool                        m_update_flag;
        quint32                     m_update_cntr;

        EtalContent                 m_cont;

        qint32                      m_signalStrength;
        quint8                      m_pty;
        bool                        m_tp;
        bool                        m_ta;
        bool                        m_ms;
        QString                     m_radio_text;
        quint8                      m_rdsAFlist[26];
        quint8                      m_rdsAFlistLength;

        alignas(32) ETAL_HANDLE     m_etal_handler;
        alignas(32) ETAL_HANDLE     m_etal_handler_data;
        alignas(32) ETAL_HANDLE     m_etal_handler_data_sls;
        alignas(32) ETAL_HANDLE     m_etal_handler_qual_mon;

        Error                       m_error;
        const EtalHwCapabilities* m_cap;
        static quint32              m_cntr;

        EtalLearnFrequencyTy        m_freqList[ETAL_MNGR_LEARN_SIZE];
        qualityInfoTy               m_qual;
        EtalDABInfo                 m_dabInfo;
        DABUpdateState              m_dabState;

        EtalSeekStatus              m_seek_bg;

        QList<quint32>              m_services;
        ServiceListTy               m_serviceList;
        EventManager<eventsList, eventDataInterface>* eventManager;
        QMutex                       m_lock;
};

#endif // #if (defined CONFIG_USE_ETAL)

#endif // ETAL_H
