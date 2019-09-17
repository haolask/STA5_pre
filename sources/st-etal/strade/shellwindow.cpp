#include "shellwindow.h"
#include "ui_shellwindow.h"

ShellWindow::ShellWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShellWindow)
{
    ui->setupUi(this);
    RadioMonitorSetup();
}

ShellWindow::~ShellWindow()
{
    delete ui;
}

void ShellWindow::RadioMonitorSetup()
{
    // Connect clrButton signal to appropriate slot
    connect(ui->clear_pushButton, SIGNAL(clicked()), this, SLOT(handleClrButton()));

    this->showMinimized();
}

QString ShellWindow::getTunerFromI2cAddress(unsigned int tunerAddr)
{
    QString retTuner;

    if (0xC2 == tunerAddr)
    {
        retTuner = "  //T1";
    }
    else if (0xC8 == tunerAddr)
    {
        retTuner = "  //T2";
    }
    else if (0xC0 == tunerAddr)
    {
        retTuner = "  //T3";
    }
    else
    {
        retTuner = "";
    }
    return retTuner;
}

void ShellWindow::DisplayMonitorData(QString dataStr, int typeData, unsigned int tunerAddr)
{
    QTextCursor cursor;

    dataStr = dataStr + getTunerFromI2cAddress(tunerAddr);

    cursor = ui->monitor_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->monitor_textEdit->setTextCursor(cursor);
    cursor = ui->monitor_textEdit->textCursor();

    switch (typeData)
    {
        case RADIO_DATA_COMMAND:
            ui->monitor_textEdit->setTextColor(Qt::black);
            break;

        case RADIO_DATA_NOTIFY:
            ui->monitor_textEdit->setTextColor(Qt::red);
            break;

        case RADIO_DATA_RESPONSE:
            ui->monitor_textEdit->setTextColor(Qt::blue);
            break;

        case RADIO_DATA_AUTONOTIFY:
            ui->monitor_textEdit->setTextColor(Qt::darkRed);
            break;

        default:
            ui->monitor_textEdit->setTextColor(Qt::black);
            break;
    }

    cursor = ui->monitor_textEdit->textCursor();
    cursor.insertText(dataStr);
    ui->monitor_textEdit->setTextCursor(cursor);
}

bool ShellWindow::DisplayErrorRadio(RadioSequencesStatusTy seqStatus, QString notifyStr, QString notifyStatusStr)
{
    bool noRadioConnect = false;
    QString infoStr;

    if (seqStatus == RADIO_NO_CONNECTION_AVAILABLE_ERROR)
    {
        infoStr.append(QString("\n[i] ERROR in Command Connection status!"));
        DisplayMonitorData(infoStr, RADIO_DATA_COMMAND);

        noRadioConnect = true;
    }
    else
    {
        // Error
        infoStr.append(QString("\n[n] ") + notifyStr);
        infoStr.append(QString("\n[i] ERROR in Notification Status: ") + notifyStatusStr);
        DisplayMonitorData(infoStr, RADIO_DATA_NOTIFY);
    }

    return noRadioConnect;
}

void ShellWindow::handleClrButton()
{
    ui->monitor_textEdit->clear();
}

void ShellWindow::inShellCmostData_slot(QString eventStr, QString dataStr, int typeData, unsigned int tunerAddr)
{
    DisplayMonitorData(eventStr + dataStr, typeData, tunerAddr);
}

void ShellWindow::inShellMwData_slot(QString eventStr, QString dataStr, int typeData)
{
    DisplayMonitorData(eventStr + dataStr, typeData);
}

void ShellWindow::inShellEtalData_slot(QString eventStr, QString dataStr, int typeData)
{
    DisplayMonitorData(eventStr + dataStr, typeData);
}

void ShellWindow::inShellCmostError_slot(RadioSequencesStatusTy status, QString eventStr, QString dataStr)
{
    DisplayErrorRadio(status, eventStr, dataStr);
}

void ShellWindow::inShellMwError_slot(RadioSequencesStatusTy status, QString eventStr, QString dataStr)
{
    DisplayErrorRadio(status, eventStr, dataStr);
}

void ShellWindow::inShellEtalError_slot(RadioSequencesStatusTy status, QString eventStr, QString dataStr)
{
    DisplayErrorRadio(status, eventStr, dataStr);
}
