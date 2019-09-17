#ifndef SHELLWINDOW_H
#define SHELLWINDOW_H

#include <QMainWindow>

#include "common.h"

namespace Ui {
class ShellWindow;
}

class ShellWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShellWindow(QWidget *parent = 0);
    ~ShellWindow();

    void DisplayMonitorData(QString dataStr, int typeData, unsigned int tunerAddr = 0);
    bool DisplayErrorRadio( RadioSequencesStatusTy , QString, QString);

private:
    Ui::ShellWindow *ui;
    void RadioMonitorSetup();
    void ClearMonitor();
    void inShellCmostBootFileName(QString cmostBootFile);
    QString getTunerFromI2cAddress(unsigned int tunerAddr);

private slots:
    void handleClrButton();

public slots:
    void inShellCmostData_slot(QString, QString,int, unsigned int);
    void inShellMwData_slot(QString, QString,int);
    void inShellCmostError_slot(RadioSequencesStatusTy , QString, QString);
    void inShellMwError_slot(RadioSequencesStatusTy , QString, QString);
    void inShellEtalData_slot(QString eventStr, QString dataStr,int typeData);
    void inShellEtalError_slot(RadioSequencesStatusTy status, QString eventStr, QString dataStr);

};

#endif // SHELLWINDOW_H
