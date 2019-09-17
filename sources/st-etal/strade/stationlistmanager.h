#ifndef STATIONLISTMANAGER_H
#define STATIONLISTMANAGER_H

#include <QThread>
#include <QTimer>
#include <QDebug>

#include "common.h"
#include "target_config.h"
#include "radiomanagerbase.h"
#include "station_list_global.h"

#if defined (CONFIG_USE_ETAL)
#include "globallist_mngr_etal.h"
#endif // #if defined(Etal)

#if defined (CONFIG_USE_STANDALONE)
#include "globallist_mngr_mw.h"
#endif // #if defined(Standalone)

#define STATION_LIST_MNGR_START_DELAY       10000 // ms
#define STATION_LIST_MNGR_PERIOD            10000 // ms

class StationListManagerThread : public QThread
{
    Q_OBJECT

    public:
        void run(void)
        {
            qDebug() << "StationListManager thread ID: " << QThread::currentThreadId();

            // Starts the event loop
            exec();
        }

        void test();
};

class StationListManagerSignalsSlots : public QObject
{
    Q_OBJECT

    public:
        explicit StationListManagerSignalsSlots(QObject* parent = nullptr) : QObject(parent) { }

        virtual ~StationListManagerSignalsSlots() { }

    public slots:
        virtual void process() = 0;
        virtual void onSchedule() = 0;
        virtual void terminate() = 0;

    signals:
        void eventToRadioManager(Events event);
        void terminated();
        void finished();
        void stop();
        void error(QString err);
};

class StationListManager : public StationListManagerSignalsSlots
{
    Q_OBJECT

    public:
        StationListManager(RadioStatusGlobal* radioGlobalData, ShellWindow* shell);
        ~StationListManager();

        // Slots
        void process() override;
        void onSchedule() override;
        void terminate() override;
        void stop();
        void seek_gl(void* state);

        void eventFromHmiProlog(Events event, quint32 band);
        void eventFromHmiEpilog(Events event, quint32 band);

    private:
        // Test function
        void GlobalStationListTest();

        // Update station list
        void UpdateGlobalStationList(const ServiceListTy listService);

        // Execute test only once
        bool globalStationTestExecuted;

        // Scheduling timer
        QTimer* schedulingTimer;

        // Station list global class
        StationListGlobal* stationList;

        RadioStatusGlobal* radioStatusGlobal;

        // shell window for logging
        ShellWindow* shellWindow;

#if defined (CONFIG_USE_ETAL)
        ETAL_globalList_mngr* m_mngr;
#endif // #if defined(Etal)

#if defined (CONFIG_USE_STANDALONE)
        MW_globalList_mngr* m_mngr;
#endif // #if defined(Etal)

        bool m_stopping;

        bool m_enable_fm;
        bool m_enable_dab;

        quint32 m_fm_fe;
        quint32 m_dab_fe;

        Events m_event;
        QMutex m_lock;
};

#endif // STATIONLISTMANAGER_H
