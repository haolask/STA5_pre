
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

#ifndef LAUNCHWINDOW_H_
#define LAUNCHWINDOW_H_

#include "target_config.h"

#include <QMainWindow>
#include <QLocale>
#include <QEventLoop>
#include <QEvent>

#include "common.h"
#include "radio_storage_types.h"
#include "tcptransportlayer.h"
#include "filemanager.h"
#include "dabradio.h"
#include "cmd_mngr_mw.h"
#include "cmd_mngr_cis.h"
//#include "protocollayer.h"
#include "stationlistmanager.h"
#include "worker.h"

namespace Ui {
    class LaunchWindow;
}

class LaunchWindow : public QMainWindow, public FilesManager
{
    Q_OBJECT

public:
    explicit LaunchWindow(QMainWindow *parent = 0, QString m_InstancerName = "0");
    ~LaunchWindow();

private:
    Ui::LaunchWindow *ui;
    QEventLoop *eventLoop;
    QTimer *delayTimer;

    RadioStatusGlobal *radioGlobalData;

    DabRadio *dabradio;

    WorkerThread workerThread;
    Worker *worker;

    ShellWindow *shellWindow;

    StationListManagerThread *stationListManagerThread;
    StationListManager *stationListManager;

    int processNumber;

    void ExitThreads ();
    void ApplicationInit();

    void StationListManagement();

private slots:
    void delayTimerTimeout_slot();
    void CloseApplication_Slot();
    void WorkerProcessFinished();
    void stationListError(QString errorStr);

public slots:
    void disp_slot();

signals:
    void processStart_signal();
    void processStarted_signal();
    void processTerminate_signal();
    void processTerminated_signal(int, QProcess::ExitStatus);
    void terminateStationList();

    void MySignal();
};

#endif // LAUNCHWINDOW_H_
