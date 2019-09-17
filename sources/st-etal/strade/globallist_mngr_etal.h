#ifndef GLOBALLIST_MNGR_ETAL_H
#define GLOBALLIST_MNGR_ETAL_H

#include <QObject>
#include <QDebug>
#include "common.h"
#include "cmd_mngr_base.h"
#include "globallist_mngr_base.h"

extern "C" {
#include <etal_target_config.h>
#include <etal_api.h>
}

// to have FG
//#define ETAL_GLOBAL_LIST_DEBUG
//#define ETAL_GLOBAL_LIST_INIT_ETAL
#define GLOABLE_LIST_VERB

#define ETAL_GLOBAL_LIST_FM_START_FREQ  87500
//#define ETAL_GLOBAL_LIST_DAB_START_FREQ 174928
#define ETAL_GLOBAL_LIST_DAB_START_FREQ 239200

#define ETAL_GLOBAL_LIST_LOG_COLOR 1
#define ETAL_GLOBAL_LIST_MES_COLOR 2

#define ETAL_GLOBAL_LIST_RDS_EMPTY_MSG "not detected"

typedef struct
{
    quint32 freq;
    QString pi;
    QString ps;
} RDSData;

class ETAL_globalList_mngr : public QObject, public GlobalList_mngr_base, public CmdMngrBase
{
    Q_OBJECT

    public:
        explicit ETAL_globalList_mngr(QObject* parent = 0);
        ~ETAL_globalList_mngr();

        void on(bool fm = true, bool dab = true, quint32 fm_fe = ETAL_FE_HANDLE_1, quint32 dab_fe = ETAL_FE_HANDLE_1);
        void off();
        void term();
        ServiceListTy handlerFM();
        ServiceListTy handlerDAB();

        void seekHandler(void* state);

        ETAL_HANDLE receiver() const {  return m_etal_handler_receiver_bg;  }

        friend void     etalRDSCallbackFg(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
        friend void     etalRDSCallbackBg(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy* status, void* pvContext);
#if defined (ETAL_GLOBAL_LIST_INIT_ETAL)
        friend void     etalUserNotificationHandler(void* context, ETAL_EVENTS dwEvent, void* pstatus);
#endif // #if defined(ETAL_GLOBAL_LIST_INIT_ETAL)

    signals:
        void message(QString s1, QString s2, qint32 a);
        void logging(QString s1, QString s2, qint32 a);
        void seekFreqFoundSignal();
        void seekFullCycleSignal();
        void rdsAcqDoneSignal();
        void terminate();
        void terminated();
        void receiverUpdate(ETAL_HANDLE receiver);

    public slots:
    private:
        // methods
        void freqFound(quint32 freq);
        void fullcycleFound(quint32 freq);

        void setMode(Mode mode, bool fg = false);
        bool setFreq(quint32 freq, bool fg = false);

        void start();
        bool cont();
        void stop();

        void setRDS(bool en, bool fg = false);
        void handleRDS(EtalRDSData* prds);

        void getEnsembleInfo(EnsembleTableTy* ensTable);
        quint32 updateServiceList(ServiceListTy* list, bool fg = false);

        quint32 getCurrentEnsemble(bool fg = false);
        QList<quint32> getServiceList(qint32 ens, bool audioServ, bool dataServ, bool fg = true);

#if defined (ETAL_GLOBAL_LIST_DEBUG)
        void selectAudioService(qint32 ens, quint16 id);
#endif // #if defined (ETAL_GLOBAL_LIST_DEBUG)

        // variables
        bool        m_mode_done;
        bool        m_found_freq;
        Mode        m_mode;
        quint32     m_freq;
        bool        m_fm_enable;
        bool        m_dab_enable;

        quint32     m_fm_fe;
        quint32     m_dab_fe;
        RDSData     m_rds_data;

        quint32     m_last_freq;

        // handler to etal receiver
#if defined (ETAL_GLOBAL_LIST_DEBUG)
        ETAL_HANDLE m_etal_handler_receiver_fg;
#endif // #if defined(ETAL_GLOBAL_LIST_DEBUG)
        ETAL_HANDLE m_etal_handler_receiver_bg;
#if defined (ETAL_GLOBAL_LIST_DEBUG)
        ETAL_HANDLE m_etal_handler_rds_fg;
#endif // #if defined(ETAL_GLOBAL_LIST_DEBUG)
        ETAL_HANDLE m_etal_handler_rds_bg;
        QMutex      m_lock;

        //   QEventLoop  m_loop;
        //    QTimer      timer;
};

#endif // GLOBALLIST_MNGR_ETAL_H
