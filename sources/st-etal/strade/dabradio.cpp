#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QScrollBar>
#include <QTapAndHoldGesture>
#include <QSound>

#include "elistwidget.h"
#include "echeckbox.h"
#include "pty.h"
#include "dab.h"
#include "dabradio.h"

#include "ui_dabradio.h"

namespace
{
    // Defines to be used to capture mouse events
    #define LBL_RADIO_MOVE_BAR              QString("lbl_radioMoveBar")
    #define LBL_PAD_ON_DLS_LABEL            QString("padDls_Label")
    #define LBL_SLS_ON_XPAD_IMAGE           QString("lbl_slsOnXpadImage")

    #define MAXDAB_INDEX                    40

    #define DELAY_ACTION_500_MSEC           500
    #define DELAY_ACTION_1_SEC              1000
    #define DELAY_ACTION_2_SEC              2000
    #define DELAY_ACTION_2_5_SEC            2500
    #define PRESET_LONG_DURATION_MS         1500
    #define DELAY_ACTION_4_SEC              4000
    #define DELAY_ACTION_MS                 8000

    #define TIMEOUT_DISPLAY_SLS_AGAIN       DELAY_ACTION_MS
    #define DELAY_NOSIGNAL_INDICATION       DELAY_ACTION_2_SEC

    #define SERVICE_LIST_NUMBER_OF_DISPLAYED_ELEMENTS   9
    #define SERVICE_LIST_SELECTED_ENTRY                 5

    const unsigned int DAB_MIN_FREQ = 168160;    //!< Absolute maximum frequency for FM worldwide
    const unsigned int DAB_MAX_FREQ = 239200;    //!< Absolute maximum frequency for AM worldwide

    const unsigned int FM_MIN_FREQ = 64000;     //!< Absolute minimum frequency for FM worldwide
    const unsigned int FM_MAX_FREQ = 109000;    //!< Absolute maximum frequency for FM worldwide
    const unsigned int AM_MIN_FREQ = 522;       //!< Absolute minimum frequency for AM worldwide
    const unsigned int AM_MAX_FREQ = 30000;     //!< Absolute maximum frequency for AM worldwide

    const unsigned int FM_EU_MIN_FREQ = 87500;  //!< European FM minimum frequency
    const unsigned int FM_EU_MAX_FREQ = 108000; //!< European FM maximum frequency
    const unsigned int FM_US_MIN_FREQ = 87900;  //!< US FM minimum frequency
    const unsigned int FM_US_MAX_FREQ = 107900; //!< US FM maximum frequency

    const unsigned int AM_EU_MIN_FREQ = 522;    //!< European AM minimum frequency
    const unsigned int AM_EU_MAX_FREQ = 1629;   //!< European AM maximum frequency
    const unsigned int AM_US_MIN_FREQ = 530;    //!< US AM minimum frequency
    const unsigned int AM_US_MAX_FREQ = 1710;   //!< US AM maximum frequency

    const unsigned int FM_EU_STEP = 100;         //!< European FM step
    const unsigned int FM_US_STEP = 200;         //!< US FM step

    const unsigned int AM_EU_STEP = 9;           //!< European AM step
    const unsigned int AM_US_STEP = 10;          //!< US FM step

    // Display states and actions
    const QString STATE_SERVICELIST = "STATE: service list";        //!< The service list is displayed
    const QString STATE_PRESETS = "STATE: presets";                 //!< Presets are displayed (in DAB use case with the SLS)
    const QString STATE_SLS_PTY = "STATE: sls/pty";                 //!< In DAB SLS is displayed, in case of FM it is the PTY image
    const QString STATE_QUALITY = "STATE: quality";                 //!< Quality screen is displayed
    const QString STATE_SETUP = "STATE: setup";                     //!< Setup screen is displayed (in this screen the QUALITY can be enabled and DISABLED)
    const QString STATE_OFF = "STATE: off";                         //!< The radio is turn off
    const QString STATE_PREVIOUS = nullptr;                         //!< For transitions from current to previous state the nullptr is used to indicate the
    //!< "unknown" previous state

    const QString ACTION_PRESS_ONOFF = "ACTION: press on/off";      //!< Action caused by the user pressing of the on/off button
    const QString ACTION_PRESS_PRESETS = "ACTION: press presets";   //!< Action caused by the user pressing of the presets button
    const QString ACTION_PRESS_LIST = "ACTION: press list";         //!< Action caused by the user pressing of the list button
    const QString ACTION_PRESS_SCREEN = "ACTION: press screen";     //!< Action caused by the user pressing the display (in the SLS space)
    const QString ACTION_ON_TIMER = "ACTION: on timer";             //!< Action caused by timer to expire
    const QString ACTION_NEW_SLS = "ACTION: new sls";               //!< Action caused by a new SLS image  available for display
    const QString ACTION_PRESS_SETUP = "ACTION: press setup";       //!< Action caused by the user pressing the setup button
    const QString ACTION_TUNE = "ACTION: tune";                     //!< Action caused by the user performing one of the action related to tune: seek/scan/tune
    const QString ACTION_NONE = nullptr;                            //!< No action took place, this entry is used to check for a state change without any action
    //!< occurred (i.e. the current state became disabled)

    enum BerQualityTy
    {
        BER_RESERVED                          = 0,                  //!< Not used
        BER_LEVEL_ZERO                        = 1,                  //!< BER level 0 (quality is considered poor)
        BER_LESS_5E_4                         = 2,                  //!< BER level 4 (quality is considered good)
        BER_LESS_5E_3                         = 3,                  //!< BER level 3 (quality is considered good)
        BER_LESS_5E_2                         = 4,                  //!< BER level 2 (quality is considered poor)
        BER_LESS_5E_1                         = 5,                  //!< BER level 1 (quality is considered poor)
        BER_GE_5E_1                           = 6,                  //!< BER level below level 1 limit (quality is considered poor)
        BER_NOT_AVAILABLE                     = 7                   //!< BER is not available
    };
}

DabRadio::DabRadio(RadioStatusGlobal* radioGlobalData, QWidget* parent, QString _dataMsgInstName) :
    QMainWindow(parent),
    DataMessages(_dataMsgInstName),
    ui(new Ui::DabRadio)
{
    ui->setupUi(this);

    // Set fonts
    SetFonts();

    // Remove corners setting translucent background
    setAttribute(Qt::WA_TranslucentBackground, true);

    // Remove border and title bar
    setWindowFlags(Qt::FramelessWindowHint);

    lastPIid.clear();

    radioStatusGlobal = radioGlobalData;

    isSeekMode = NO_SEEK_MODE;

    isPresetListOn = false;

    isClosing = false;

    actionExecuted = RADIO_ACTION_NONE;

    lastLabelDisplayed.clear();

    powerOffStatus = true;

    tuneIsOngoing = false;

    isSetupScreenOn = false;
    isLearnActive = false;
    vpa_on = true;

    sliderCurrentlyMoved = false;

    fmFreqkHz = INVALID_FREQUENCY;
    mwFreqkHz = INVALID_FREQUENCY;
    dabFreqMHz = INVALID_FREQUENCY;

    // Default to BAND DAB
    radioPersistentData.band = BAND_DAB3;

    // Default values to europe ones
    CalculateCountryDependentValues(COUNTRY_EU);

    displayStatus.currentView = VIEW_ACTIVE_NONE;
    displayStatus.nextView = VIEW_ACTIVE_NONE;

    // Get an instance of the preset list
    presetList = presetList->Instance();

    // Fill widget lists
    SetupWidgetLists();

    // Setup radio (timers and visuals)
    DabRadioSetup();

    // Init Radio status to OFF
    radioIsOn = false;

    // Calculate move bar geometry
    QRect moveBarGeometry = ui->lbl_radioMoveBar->geometry();

    moveOffsetX = moveBarGeometry.x();
    moveOffsetY = moveBarGeometry.y();

    // Initialize the views that changes depending on current radio status
    if (false == InitializeDisplayViewStateMachine())
    {
        qDebug() << "ERROR: failure to initialize the display view state machine";
    }
}

void DabRadio::SetFonts()
{
    // Load fonts
    QFontDatabase::addApplicationFont(":/fonts/cambria");

    QFont font = QFont("Cambria", 11, QFont::Bold, false);

    // Set fonts for all labels and button
    QList<QPushButton *> listButtons = this->findChildren<QPushButton *> ();

    for (auto button : listButtons)
    {
        button->setFont(font);
    }

    QList<QLabel *> listLabels = this->findChildren<QLabel *> ();

    for (auto label : listLabels)
    {
        label->setFont(font);
    }

    // Set special fonts: labels or buttons with different size
    QFont specialFont = QFont("Cambria", 9, QFont::Bold, false);

    ui->lbl_activeServiceBitrate->setFont(specialFont);
    ui->lbl_bandBottom->setFont(specialFont);
    ui->lbl_bandTop->setFont(specialFont);
}

DabRadio::~DabRadio()
{
    waitTimer->stop();
    delayActionTimer->stop();
    mousePressureTimer->stop();
    signalIndicationTimer->stop();

    delete smMachineDab;
    delete smMachineFm;

    delete waitTimer;
    delete delayActionTimer;
    delete mousePressureTimer;
    delete signalIndicationTimer;

    delete ui;
}

// Permanent
// Have an object living in another thread and let it perform different tasks upon request.
// This means communication to and from the worker thread is required.
// 1) Derive a class from QObject and implement the necessary slots and signals
// 2) Move the object to a thread with a running event loop
// 3) Communicate with the object over queued signal/slot connections
bool DabRadio::eventFilter(QObject* obj, QEvent* event)
{
    QDateTime now = QDateTime::currentDateTime();

    if (event->type() == QEvent::MouseButtonPress && obj == ui->lstWidget_currPresetList->viewport())
    {
        checkMousePressureTime.msFromEpoch = now.toMSecsSinceEpoch();

        lastSlsIsValid = false;

        // Start timer
        mousePressureTimer->start(PRESET_LONG_DURATION_MS);
    }
    else if (event->type() == QEvent::MouseButtonRelease && obj == ui->lstWidget_currPresetList->viewport())
    {
        qint64 tmpVal = now.toMSecsSinceEpoch();

        checkMousePressureTime.deltaFromLatestEvent = tmpVal - checkMousePressureTime.msFromEpoch;

        if (checkMousePressureTime.deltaFromLatestEvent < PRESET_LONG_DURATION_MS)
        {
            mousePressureTimer->stop();

            // Set the preset selected by the user
            ui->lstWidget_currPresetList->setCurrentRow(presetCurrentRow);
            ui->padDls_Label->clear();

            qDebug() << " Preset Index = " << presetCurrentRow;

            radioStatusGlobal->panelData.currPresetIndex = presetCurrentRow;

            // Execute event action (LOAD)
            PresetEventAction(false);
        }
    }
    else if (event->type() == QEvent::Gesture && obj == ui->btn_onoff)
    {
        QGestureEvent* gestevent = static_cast<QGestureEvent *> (event);

        if (QGesture* gest = gestevent->gesture(Qt::TapAndHoldGesture))
        {
            QTapAndHoldGesture* tapgest = static_cast<QTapAndHoldGesture *> (gestevent->gesture(Qt::TapAndHoldGesture));

            Q_UNUSED(gest);
            Q_UNUSED(tapgest);

            // Avoid to get a second event
            if ((false == isClosing && false == radioIsOn) ||
                (false == isClosing && true == bootCompleted))
            {
                qDebug() << "Grabbed a gesture event";

                isClosing = true;

                if (false == radioIsOn)
                {
                    emit CloseApplication_Signal();
                }
                else
                {
                    PowerOffAction();
                }
            }
            else
            {
                qDebug() << "Gesture with: 'isClosing' = " << isClosing << " 'isRadioOn' = " << radioIsOn;
            }
        }
    }
    else if ((obj->objectName() == LBL_RADIO_MOVE_BAR) && (event->type() == QEvent::MouseButtonPress))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent *> (event);

        m_nMouseClick_X_Coordinate = mouseEvent->x() + moveOffsetX;
        m_nMouseClick_Y_Coordinate = mouseEvent->y() + moveOffsetY;
    }
    else if ((obj->objectName() == LBL_RADIO_MOVE_BAR) && (event->type() == QEvent::MouseMove))
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent *> (event);

        if (mouseEvent->buttons() & Qt::LeftButton)
        {
            move(mouseEvent->globalX() - m_nMouseClick_X_Coordinate, mouseEvent->globalY() - m_nMouseClick_Y_Coordinate);
        }
    }
    else if (obj->objectName() == LBL_PAD_ON_DLS_LABEL || obj->objectName() == LBL_SLS_ON_XPAD_IMAGE)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent *> (event);

            if (mouseEvent->buttons() & Qt::LeftButton)
            {
                OnScreenPressViewUpdate();
            }
        }
    }

    // Allow standard event processing
    return QMainWindow::eventFilter(obj, event);
}

bool DabRadio::CheckIfButtonActionCanBeDone()
{
    // Get the sender object
    QObject* senderObj = sender();

    // Get the sender name
    QString senderObjName = senderObj->objectName();

    // If the pressed button is the ON/OFF it is always active
    if (0 == senderObjName.compare("btn_onoff"))
    {
        // This slot is called also on button release
        if (true == isClosing || false == bootCompleted)
        {
            return false;
        }
    }
    else if (0 == senderObjName.compare("btn_setup"))
    {
        // If the radio is not in power on status then we need to return here
        if (false == radioIsOn || true == powerOffStatus)
        {
            return false;
        }
    }
    else
    {
        // If the setup screen is active we do not do any action: the user needs to close the setup screen
        if (true == isSetupScreenOn || false == radioIsOn || true == powerOffStatus)
        {
            return false;
        }
    }

    return true;
}

void DabRadio::waitTimer_timeout_slot()
{
    isWaiting = false;
}

void DabRadio::SignalQualityText_slot()
{
    ui->lbl_answer->setText(signalTextToDisplay);
}

void DabRadio::DelayTimerSetupAndStart(RadioActionsTy action, int timeout)
{
    if (RADIO_ACTION_NONE != actionExecuted)
    {
        delayActionTimer->stop();
    }

    actionExecuted = action;

    // Set timer with executed action in order to do closing stuff on timer
    delayActionTimer->setInterval(timeout);
    delayActionTimer->start();
}

void DabRadio::DelayActionTimer_slot()
{
    switch (actionExecuted)
    {
        case RADIO_POWER_ON_ACTION:
            AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_STOP);
            break;

        case RADIO_POWER_OFF_ACTION:
            AnimationsMachine(RADIO_OFF_ANIMATION_GROUP, (AnimationActionsTy)0);

            // Enable button signals
            ButtonBlockSignals(false);
            break;

        case RADIO_ACTION_VOLUME:
            ui->lbl_ensFreqNumber->setText(lastLabelDisplayed);
            break;

        case RADIO_PRESET_ACTION:
            // The preset timer started on button click has ended, we display old screen again
            isPresetListOn = false;

            // Enable the signal
            ui->lstWidget_currPresetList->blockSignals(false);
            break;

        default:
            // No code
            break;
    }

    // Update the screen
    if (RADIO_DISPLAY_TESTSCREEN == actionExecuted ||
        RADIO_SERVICELIST_ACTION == actionExecuted ||
        RADIO_PRESET_ACTION == actionExecuted)
    {
        QString oldState = currentSmMachine->GetCurrentState();
        QString newState = currentSmMachine->CalculateNewState(ACTION_ON_TIMER);

        qDebug() << "On ON TIMER new state is: " << newState;

        ModifyDisplayView(newState, oldState, ACTION_ON_TIMER);
    }

    // Clear executed action
    actionExecuted = RADIO_ACTION_NONE;
}

void DabRadio::SetSignalStrengthIcon(bool visible, quint32 qualityValue)
{
    // Set visible status
    if (true == powerOffStatus)
    {
        return;
    }

    ui->lbl_signalStatus->setVisible(visible);

    // Set icon based on quality value
    if (true == visible)
    {
        switch (qualityValue)
        {
            case BER_RESERVED:                      // 0
            case BER_NOT_AVAILABLE:                 // 7
                ui->lbl_signalStatus->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/signal_assent_ico.png);"));
                break;

            case BER_LEVEL_ZERO:                   // 1
            case BER_LESS_5E_4:                    // 2
                ui->lbl_signalStatus->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/signal_high_ico.png);"));
                break;

            case BER_LESS_5E_3:                    // 3
            case BER_LESS_5E_2:                    // 4
                ui->lbl_signalStatus->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/signal_medium_ico.png);"));
                break;

            case BER_LESS_5E_1:                    // 5
                ui->lbl_signalStatus->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/signal_low_ico.png);"));
                break;

            case BER_GE_5E_1:                      // 6
                ui->lbl_signalStatus->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/signal_verylow_ico.png);"));
                break;

            default:
                qDebug() << "ERROR: unknown signal quality received";
                break;
        }
    }
}

void DabRadio::hide_ensembleRadioLabels()
{
    // Hide service radio labels
    ui->lbl_activeServiceBitrate->setVisible(false);

    // Hide radio status label
    ui->lbl_ensLabel->setVisible(false);
}

void DabRadio::waitAnimationTimer_timeout_slot()
{ }

void DabRadio::tick_timeout_slot()
{
    ui->lbl_watch->setText(QTime::currentTime().toString(QString("hh:mm:ss")));
}

void DabRadio::DabRadioSetup()
{
    utils = new Utilities();

    // Wait timer setup
    waitTimer = new QTimer(this);
    waitTimer->setSingleShot(true);

    QObject::connect(waitTimer, &QTimer::timeout, this, &DabRadio::waitTimer_timeout_slot);

    // 'mousePressureTimer' setup
    mousePressureTimer = new QTimer(this);
    QObject::connect(mousePressureTimer, &QTimer::timeout, this, &DabRadio::mousePressureTimer_timeout_slot);

    // Action timeout timer
    delayActionTimer = new QTimer(this);
    delayActionTimer->setSingleShot(true);

    QObject::connect(delayActionTimer, &QTimer::timeout, this, &DabRadio::DelayActionTimer_slot);

    // Signal indication timer (used to delay the signal/no signal indication to take account of
    // the audio delay (user experience)
    signalIndicationTimer = new QTimer(this);
    signalIndicationTimer->setSingleShot(true);
    signalIndicationTimer->setInterval(DELAY_NOSIGNAL_INDICATION);

    QObject::connect(signalIndicationTimer, &QTimer::timeout, this, &DabRadio::SignalQualityText_slot);

    // Setup radio time indication
    RadioClockSetup();

    // Initialize visuals
    GraphicsItemsSetup();

    configureTestScreenDisplay();

    setPanel(RADIO_INIT_ACTION);
}

void DabRadio::AnimationsStopOnEventResponse(Events event)
{
    switch (event)
    {
        case EVENT_POWER_UP:
        case EVENT_FREQUENCY_CHANGE:
        case EVENT_SEEK_UP:
        case EVENT_SEEK_DOWN:
            AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_STOP);
            break;

        default:
            // No code
            break;
    }
}

void DabRadio::setPanel(RadioActionsTy action)
{
    ui->testscreen_widget->setVisible(false);
    ui->setup_widget->setVisible(false);
    ui->btn_clearPresets->setVisible(false);
    ui->btn_learn->setVisible(false);

    switch (action)
    {
        case RADIO_INIT_ACTION:
        {
            ViewsSwitcher(RADIO_HW_INIT_VIEW);

            // Clear data
            SetClearProperty(RADIO_INIT_ACTION);
        }
        break;

        case RADIO_POWER_ON_ACTION:
        {
            // Signal that a poweron has been given
            powerOffStatus = false;

            // Set panel according to ON status
            ViewsSwitcher(RADIO_HW_ON_VIEW);

            AnimationsMachine(LABEL_RADIO_BACKGROUND_OPACITY_ANIMATION_ON, ANIMATION_START);
            wait(100);
            ViewsSwitcher(RADIO_POWER_ON_VIEW);

            // Display DAB or FM last status setting
            displaySelectedBand();

            // Clear data
            SetClearProperty(RADIO_POWER_ON_ACTION);

            // Start animations
            AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_START);

            // Update information on screen
            ServiceFollowingFlagsUpdate();

            // Set timer with executed action in order to do closing stuff on timer
            if (RADIO_ACTION_NONE == actionExecuted)
            {
                DelayTimerSetupAndStart(RADIO_POWER_ON_ACTION, DELAY_ACTION_MS);
            }
        }
        break;

        case RADIO_POWER_OFF_ACTION:
        {
            // Indicate that a power off action is ongoing (avoid to update panel labels and visuals)
            powerOffStatus = true;

            // Set panel according to OFF status
            ViewsSwitcher(RADIO_HW_OFF_VIEW);

            // Clear data
            SetClearProperty(RADIO_POWER_OFF_ACTION);

            // Start animations
            AnimationsMachine(LABEL_ST_LOGO_OPACITY_ANIMATION_ON, ANIMATION_START);

            // Set timer with executed action in order to do closing stuff on timer
            // Set timer to 700 + 500 + minimum delta (animation time)
            DelayTimerSetupAndStart(RADIO_POWER_OFF_ACTION, 1300);
        }
        break;

        case RADIO_TUNE_FREQUENCY_ACTION:
        case RADIO_SEEK_UP_ACTION:
        case RADIO_SEEK_DOWN_ACTION:
        {
            // Clear data (RADIO_TUNE_FREQUENCY_ACTION used for all)
            SetClearProperty(RADIO_TUNE_FREQUENCY_ACTION);

            // Set visible property on ACTION
            SetVisibleProperty(RADIO_TUNE_FREQUENCY_ACTION);

            // Start animation (for DAB always for analog only for seek)
            if (BAND_DAB3 == radioPersistentData.band)
            {
                AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_START);
            }
            else
            {
                if (RADIO_SEEK_UP_ACTION == action || RADIO_SEEK_DOWN_ACTION == action)
                {
                    AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_START);
                }
            }
        }
        break;

        case RADIO_SOURCE_ACTION:
        {
            // Clear labels
            SetClearProperty(RADIO_SOURCE_ACTION);

            // Set visible property on ACTION
            SetVisibleProperty(RADIO_SOURCE_ACTION);

            // Signal strength icon set to signal absent
            SetSignalStrengthIcon(true, BER_NOT_AVAILABLE);

            // Update information on screen
            ServiceFollowingFlagsUpdate();

            // Start animations
            AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_START);
        }
        break;

        case RADIO_SETUP_ACTION:
        {
            #define CHECKBOX_SEPARATION_SIZE            40
            #define BUTTONS_SEPARATION_SIZE             60

            int currentCheckBoxPosition = 30;

            // Create Setup various labels at runtime
            ui->setup_widget->setGeometry(QRect(155, 44, 597, 382));

            ui->setup_widget->setParent(this);

            // 'Z' order
            ui->setup_widget->raise();

            ui->lbl_radioBackground->lower();

            ui->setupTitle_label->setText("SETUP CONFIGURATION");
            ui->setupTitle_label->setGeometry(QRect(200, 5, 300, 25));

            // First column settings
            ui->slsdisplay_checkBox->setText("SLS display Enable");
            ui->slsdisplay_checkBox->setChecked(radioPersistentData.slsDisplayEn);
            ui->slsdisplay_checkBox->setGeometry(QRect(10, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->fmfmServiceFollowingEn_checkBox->setText("FM-FM SF Enable");
            ui->fmfmServiceFollowingEn_checkBox->setChecked(radioPersistentData.afCheckEn);
            ui->fmfmServiceFollowingEn_checkBox->setGeometry(QRect(10, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->dabfmServiceFollowingEn_checkBox->setText("DAB-FM SF Enable");
            ui->dabfmServiceFollowingEn_checkBox->setChecked(radioPersistentData.dabFmServiceFollowingEn);
            ui->dabfmServiceFollowingEn_checkBox->setGeometry(QRect(10, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->dabdabServiceFollowingEn_checkBox->setText("DAB-DAB SF Enable");
            ui->dabdabServiceFollowingEn_checkBox->setChecked(radioPersistentData.dabDabServiceFollowingEn);
            ui->dabdabServiceFollowingEn_checkBox->setGeometry(QRect(10, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->globalServiceListEn_checkBox->setText("Global Service List Enable");
            ui->globalServiceListEn_checkBox->setChecked(radioPersistentData.globalServiceListEn);
            ui->globalServiceListEn_checkBox->setGeometry(QRect(10, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->ptyEn_checkBox->setText("Program Type");
            ui->ptyEn_checkBox->setChecked(false);
            ui->ptyEn_checkBox->setGeometry(QRect(10, currentCheckBoxPosition, 280, 40));
            ui->ptyEn_checkBox->setMinimumSize(QSize(280, 40));

            // Do connections
            QObject::connect(ui->ptyEn_checkBox,
                             static_cast<void (ECheckBox::*)(bool, bool)> (&ECheckBox::Click_signal),
                             this,
                             static_cast<void (DabRadio::*)(bool, bool)> (&DabRadio::CheckBoxClick_slot),
                             Qt::DirectConnection);

            currentCheckBoxPosition = 30;

            // Second column settings
            ui->taEn_checkBox->setText("TA Enable");
            ui->taEn_checkBox->setChecked(radioPersistentData.rdsTaEnabled);
            ui->taEn_checkBox->setGeometry(QRect(310, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->regionalRdsEn_checkBox->setText("RDS Regional mode Enable");
            ui->regionalRdsEn_checkBox->setChecked(radioPersistentData.rdsRegionalEn);
            ui->regionalRdsEn_checkBox->setGeometry(QRect(310, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->rdsEonEn_checkBox->setText("RDS EON Enable");
            ui->rdsEonEn_checkBox->setChecked(radioPersistentData.rdsEonEn);
            ui->rdsEonEn_checkBox->setGeometry(QRect(310, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->unsupportedServicesDisplayEn_checkBox->setText("Unsupported Services Enable");
            ui->unsupportedServicesDisplayEn_checkBox->setChecked(radioPersistentData.displayUnsupportedServiceEn);
            ui->unsupportedServicesDisplayEn_checkBox->setGeometry(QRect(310, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->alphabeticalServicesEn_checkBox->setText("Services Ordering");
            ui->alphabeticalServicesEn_checkBox->setChecked(radioPersistentData.alphabeticOrder);
            ui->alphabeticalServicesEn_checkBox->setGeometry(QRect(310, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += CHECKBOX_SEPARATION_SIZE;

            ui->testScreenEn_checkBox->setText("Test Screen Enable");
            ui->testScreenEn_checkBox->setChecked(radioPersistentData.testScreenIsActive);
            ui->testScreenEn_checkBox->setGeometry(QRect(310, currentCheckBoxPosition, 280, 40));
            currentCheckBoxPosition += BUTTONS_SEPARATION_SIZE;

            // Buttons
            ui->btn_clearPresets->setText("Clear Presets");
            ui->btn_clearPresets->setGeometry(QRect(20, currentCheckBoxPosition, 141, 61));
            ui->btn_clearPresets->setVisible(true);

            ui->btn_learn->setText("Learn");
            ui->btn_learn->setGeometry(QRect(310, currentCheckBoxPosition, 141, 61));
            ui->btn_learn->setVisible(true);

            // Put setup_label visible
            ui->setup_widget->setVisible(true);
        }
        break;

        default:
        {
            // No code
        }
        break;
    }
}

bool DabRadio::InitializeDisplayViewStateMachine()
{
    QList<QString> stateList;
    QList<QString> eventList;
    smMachineDab = new SmMachine("SM: DAB");
    smMachineFm = new SmMachine("SM: FM");
    currentSmMachine = nullptr;
    QList<QString>::const_iterator iter;
    bool opRes = true;

    // Just for better debug
    qSetMessagePattern("%{file}(%{line}): %{message}");

    stateList.append(STATE_SERVICELIST);
    stateList.append(STATE_PRESETS);
    stateList.append(STATE_SLS_PTY);
    stateList.append(STATE_QUALITY);
    stateList.append(STATE_SETUP);
    stateList.append(STATE_OFF);

    eventList.append(ACTION_PRESS_PRESETS); // User press presets button
    eventList.append(ACTION_PRESS_LIST);    // User press list button
    eventList.append(ACTION_PRESS_SCREEN);  // User press screen
    eventList.append(ACTION_ON_TIMER);      // The timer expires
    eventList.append(ACTION_NEW_SLS);       // New sls arrives
    eventList.append(ACTION_PRESS_SETUP);   // User press setup
    eventList.append(ACTION_TUNE);          // Gather all actions resulting in a tune: seek, scan, tune, source
    eventList.append(ACTION_PRESS_ONOFF);   // USer press power on/off button

    ///
    // SETUP DAB STATE MACHINE
    ///
    // Insert states in DAB state machine
    for (iter = stateList.constBegin(); iter != stateList.constEnd(); ++iter)
    {
        const QString& tmpStateName = *iter;

        smMachineDab->InsertNewState(tmpStateName);
    }

    // Insert events in DAB state machine
    for (iter = eventList.constBegin(); iter != eventList.constEnd(); ++iter)
    {
        const QString& tmpEventName = *iter;

        smMachineDab->InsertNewEvent(tmpEventName);
    }

    // Set default state
    //smMachineDab->SetDefaultState(STATE_SERVICELIST);
    //smMachineDab->EnterDefaultState();

    // Setup transitions for DAB state machine
    // From state off
    smMachineDab->InsertStateChange(STATE_OFF, ACTION_PRESS_ONOFF, STATE_SERVICELIST, 0);

    // From state service list
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_PRESETS, STATE_PRESETS, 0);
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_LIST, STATE_SLS_PTY, 0);    // Alternate choice with lower priority
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_LIST, STATE_QUALITY, 1);    // Alternate choice with higher priority
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_ON_TIMER, STATE_SLS_PTY, 0);      // Alternate choice with lower priority
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_ON_TIMER, STATE_QUALITY, 1);      // Alternate choice with higher priority
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_NEW_SLS, STATE_SLS_PTY, 0);
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_SETUP, STATE_SETUP, 2);     // Setup is always top priority
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_TUNE, STATE_SERVICELIST, 0);
    smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_ONOFF, STATE_OFF, 3);

    // From state presets
    smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_PRESS_PRESETS, STATE_PREVIOUS, 0);
    smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_PRESS_LIST, STATE_SERVICELIST, 0);
    smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_NEW_SLS, STATE_SLS_PTY, 0);
    smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_ON_TIMER, STATE_PREVIOUS, 0);
    smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_PRESS_SETUP, STATE_SETUP, 2);          // Setup is always high priority
    smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_TUNE, STATE_SERVICELIST, 0);
    smMachineDab->InsertStateChange(STATE_PRESETS, ACTION_PRESS_ONOFF, STATE_OFF, 3);            // Off state is alwasy top priority

    // From state sls
    smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0);
    smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_LIST, STATE_SERVICELIST, 0);     // Alternate choice with lower priority
    smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_LIST, STATE_QUALITY, 1);         // Alternate choice with higher priority
    smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_SCREEN, STATE_SERVICELIST, 0);
    smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_SETUP, STATE_SETUP, 2);          // Setup is always high priority
    smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_TUNE, STATE_SERVICELIST, 0);
    smMachineDab->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_ONOFF, STATE_OFF, 3);            // Off state is alwasy top priority

    smMachineDab->SetFallbackState(STATE_SLS_PTY, STATE_SERVICELIST);

    // From state quality
    smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0);
    smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_PRESS_LIST, STATE_SERVICELIST, 0);
    smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_PRESS_SETUP, STATE_SETUP, 2);         // Setup is always top priority
    smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_TUNE, STATE_SERVICELIST, 0);
    smMachineDab->InsertStateChange(STATE_QUALITY, ACTION_PRESS_ONOFF, STATE_OFF, 3);           // Off state is alwasy top priority

    // From state setup
    //smMachineDab->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_PREVIOUS, 0);
    smMachineDab->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_SERVICELIST, 0);     // Alternate choice with lower priority
    smMachineDab->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_SLS_PTY, 1);         // Alternate choice with medium priority
    smMachineDab->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_QUALITY, 2);         // Alternate choice with higher priority
    smMachineDab->InsertStateChange(STATE_SETUP, ACTION_PRESS_ONOFF, STATE_OFF, 3);             // Off state is alwasy top priority

    // Disable not active state
    smMachineDab->DisableState(STATE_QUALITY);
    smMachineDab->DisableState(STATE_SLS_PTY);

    // Set current state
    smMachineDab->SetCurrentState(STATE_SERVICELIST);

    ///
    // SETUP FM STATE MACHINE
    ///
    // Insert states in FM state machine
    for (iter = stateList.constBegin(); iter != stateList.constEnd(); ++iter)
    {
        const QString& tmpStateName = *iter;

        smMachineFm->InsertNewState(tmpStateName);
    }

    // Insert events in FM state machine
    for (iter = eventList.constBegin(); iter != eventList.constEnd(); ++iter)
    {
        const QString& tmpEventName = *iter;

        smMachineFm->InsertNewEvent(tmpEventName);
    }

    // Set default state
    //smMachineFm->SetDefaultState(STATE_SLS_PTY);
    //smMachineFm->EnterDefaultState();

    // Setup transitions for FM state machine
    // From state off
    smMachineFm->InsertStateChange(STATE_OFF, ACTION_PRESS_ONOFF, STATE_SLS_PTY, 0);

    // From state presets
    smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_PRESETS, nullptr, 0);
    smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_LIST, STATE_SLS_PTY, 0);
    smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_LIST, STATE_QUALITY, 1);
    smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_ON_TIMER, STATE_PREVIOUS, 0);
    smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_SETUP, STATE_SETUP, 2); // Setup is always top priority
    smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_TUNE, STATE_SLS_PTY, 0);
    smMachineFm->InsertStateChange(STATE_PRESETS, ACTION_PRESS_ONOFF, STATE_OFF, 3);

    // From state pty
    smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0);
    smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_LIST, STATE_QUALITY, 0);
    smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_ON_TIMER, STATE_QUALITY, 0);
    smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_SETUP, STATE_SETUP, 2); // Setup is always top priority
    smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_TUNE, STATE_SLS_PTY, 0);
    smMachineFm->InsertStateChange(STATE_SLS_PTY, ACTION_PRESS_ONOFF, STATE_OFF, 3);

    // From state quality
    smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_PRESS_PRESETS, STATE_PRESETS, 0);
    smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_PRESS_LIST, STATE_SLS_PTY, 0);
    smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_PRESS_SETUP, STATE_SETUP, 2); // Setup is always top priority
    smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_TUNE, STATE_QUALITY, 0);
    smMachineFm->InsertStateChange(STATE_QUALITY, ACTION_PRESS_ONOFF, STATE_OFF, 3);

    smMachineFm->SetFallbackState(STATE_QUALITY, STATE_SLS_PTY);

    // From state setup
    smMachineFm->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_PREVIOUS, 0);
    smMachineFm->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_SLS_PTY, 1); // Alternate choice with medium priority
    smMachineFm->InsertStateChange(STATE_SETUP, ACTION_PRESS_SETUP, STATE_QUALITY, 2); // Alternate choice with higher priority
    smMachineFm->InsertStateChange(STATE_SETUP, ACTION_PRESS_ONOFF, STATE_OFF, 0);

    // Disable not active state
    smMachineFm->DisableState(STATE_QUALITY);
    smMachineFm->DisableState(STATE_SERVICELIST);

    // Set current state
    smMachineFm->SetCurrentState(STATE_SLS_PTY);

#if 0
    ///
    // Start TESTS
    ///
    // Test error conditions
    if (true == smMachineDab->InsertNewState(STATE_SERVICELIST))
    {
        qDebug() << "ERROR: insertion of duplicated state name " << STATE_SERVICELIST;

        opRes = false;
    }

    if (true == smMachineDab->InsertStateChange(STATE_SERVICELIST, ACTION_PRESS_PRESETS, "STATE: fake", 0))
    {
        qDebug() << "ERROR: state change called with invalid parameters was wrongly registered";

        opRes = false;
    }

    // TEST 1: DAB state machine
    QString currentState;

    // Turn on the radio
    currentSmMachine = smMachineDab;
    qDebug() << "Initial state for DAB is: " << currentSmMachine->GetCurrentState() << " (expected SERVICE LIST)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_ONOFF << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_ONOFF) << "(expected SERVICE LIST)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected SETUP)";

    currentState = currentSmMachine->GetCurrentState();
    smMachineDab->EnableState(STATE_SLS_PTY);
    qDebug() << ACTION_NEW_SLS << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_NEW_SLS) << "(expected SETUP)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected SLS/PTY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_LIST << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_LIST) << "(expected SERVICE LIST)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_ON_TIMER << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_ON_TIMER) << "(expected SLS/PTY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_PRESETS << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_PRESETS) << "(expected PRESETS)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_NEW_SLS << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_NEW_SLS) << "(expected SLS/PTY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected SETUP)";

    currentSmMachine->EnableState(STATE_QUALITY);

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected QUALITY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_LIST << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_LIST) << "(expected SERVICE LIST)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_ON_TIMER << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_ON_TIMER) << "(expected QUALITY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_PRESETS << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_PRESETS) << "(expected PRESETS)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_ON_TIMER << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_ON_TIMER) << "(expected QUALITY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected SETUP)";

    currentSmMachine->DisableState(STATE_QUALITY);

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected SLS/PTY)";

    currentSmMachine->DisableState(STATE_SLS_PTY);

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_NONE << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_NONE) << "(expected SERVICE LIST)";

    // TEST 2: FM state machine
    currentSmMachine = smMachineFm;
    qDebug() << "Initial state for FM is: " << currentSmMachine->GetCurrentState() << " (expected SLS/PTY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_PRESETS << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_PRESETS) << "(expected PRESETS)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_ON_TIMER << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_ON_TIMER) << "(expected SLS/PTY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected SETUP)";

    currentSmMachine->EnableState(STATE_QUALITY);

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_SETUP << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP) << "(expected QUALITY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_LIST << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_LIST) << "(expected SLS/PTY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_LIST << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_LIST) << "(expected QUALITY)";

    currentState = currentSmMachine->GetCurrentState();
    qDebug() << ACTION_PRESS_ONOFF << " : " << currentState << " -> " << currentSmMachine->CalculateNewState(ACTION_PRESS_ONOFF) << "(expected OFF)";
#endif // #if 0

    return opRes;
}

void DabRadio::DisplayNewView(QString action)
{
    QList<QWidget *>::iterator iter;

    // Calculate the new view
    QString newState = currentSmMachine->CalculateNewState(action);

    qDebug() << "New display view state is: " << newState;

    // Clear display
    for (iter = viewAllWidgetList.begin(); iter != viewAllWidgetList.end(); ++iter)
    {
        QWidget* tmpWidget = *iter;

        qDebug() << tmpWidget->objectName();

        tmpWidget->setVisible(false);
    }
}

void DabRadio::ServiceFollowingFlagsUpdate()
{
    ui->af_label->setVisible(radioPersistentData.afCheckEn);
    ui->dab_fm_label->setVisible(radioPersistentData.dabFmServiceFollowingEn);
    ui->dab_dab_label->setVisible(radioPersistentData.dabDabServiceFollowingEn);
}

void DabRadio::updateTaRds_slot()
{
    radioPersistentData.rdsTaEnabled = taEn_checkbox->isChecked();
}

void DabRadio::updateRegionalRds_slot()
{
    radioPersistentData.rdsRegionalEn = regionalRdsEn_checkbox->isChecked();
}

void DabRadio::updateRdsEon_slot()
{
    radioPersistentData.rdsEonEn = rdsEonEn_checkbox->isChecked();
}

void DabRadio::updateGlobalServiceList_slot()
{
    radioPersistentData.globalServiceListEn = globalServiceListEn_checkbox->isChecked();
}

void DabRadio::updateUnsupportedServicesDisplay_slot()
{
    radioPersistentData.displayUnsupportedServiceEn = unsupportedServicesDisplayEn_checkbox->isChecked();
}

void DabRadio::RadioClockSetup()
{
    // Tick timer setup
    tickTimer = new QTimer(this);

    QObject::connect(tickTimer, &QTimer::timeout, this, &DabRadio::tick_timeout_slot);

    tickTimer->start(1000);
}

void DabRadio::GraphicsItemsSetup()
{
    // Setup items
    ui->lbl_ensLabel->clear();
    ui->lbl_tuneFreq->clear();
    ui->lbl_ensFreqNumber->clear();
    ui->lbl_PIid->clear();
    ui->lbl_activeServiceLabel->clear();
    ui->lbl_answer->clear();

    // Clear SLS, DLS and LOGOS
    ui->lbl_slsOnXpadImage->clear();
    ui->padDls_Label->clear();
    ui->lbl_epgLogo->clear();
    ui->lbl_epgLogo->setVisible(true);

    ui->lbl_ensFreqNumber->setVisible(true);

    ui->tp_label->setVisible(false);
    ui->ta_label->setVisible(false);
    ui->af_label->setVisible(false);
    ui->stereoOn_label->setVisible(false);
    ui->dab_fm_label->setVisible(false);
    ui->dab_dab_label->setVisible(false);

    ui->lbl_audio_off_status->setVisible(false);

    // Virtual buttons shall be all invisible (currently not used)
    ui->btn_option->setVisible(false);
    ui->btn_setting->setVisible(false);
    ui->btn_radio->setVisible(false);
    ui->btn_audio->setVisible(false);
    ui->btn_up->setVisible(false);

    // Set styles
    ui->padDls_Label->setStyleSheet(QString::fromUtf8("QLabel#padDls_Label{\n"
                                                      "color: rgba(208, 209, 209, 230);\n"
                                                      "border-image:transparent;\n"
                                                      "background-color: transparent;\n"
                                                      "border-width: 0px;\n"
                                                      "}\n"
                                                      ""));

    ui->lbl_slsOnXpadImage->setStyleSheet(QString::fromUtf8("QLabel#lbl_slsOnXpadImage{\n"
                                                            "color: rgba(208, 209, 209, 230);\n"
                                                            "border-image:transparent;\n"
                                                            "background-color: transparent;\n"
                                                            "border-width: 0px;\n"
                                                            "}\n"
                                                            ""));

    ui->lbl_epgLogo->setStyleSheet(QString::fromUtf8("QLabel#lbl_epgLogo{\n"
                                                     "color: rgba(208, 209, 209, 230);\n"
                                                     "border-image:transparent;\n"
                                                     "background-color: transparent;\n"
                                                     "border-width: 0px;\n"
                                                     "}\n"
                                                     ""));

    ui->lbl_ensLabel->setStyleSheet(QString::fromUtf8("QLabel#lbl_ensLabel{\n"
                                                      "color: rgba(208, 209, 209, 230);\n"
                                                      "border-image:transparent;\n"
                                                      "background-color: transparent;\n"
                                                      "border-width: 0px;\n"
                                                      "}\n"
                                                      ""));

    ui->lbl_tuneFreq->setStyleSheet(QString::fromUtf8("QLabel#lbl_tuneFreq{\n"
                                                      "color: rgba(208, 209, 209, 230);\n"
                                                      "border-image:transparent;\n"
                                                      "background-color: transparent;\n"
                                                      "border-width: 0px;\n"
                                                      "}\n"
                                                      ""));

    ui->lbl_watch->setStyleSheet(QString::fromUtf8("QLabel#lbl_watch{\n"
                                                   "color: rgba(208, 209, 209, 230);\n"
                                                   "border-image:transparent;\n"
                                                   "background-color: transparent;\n"
                                                   "border-width: 0px;\n"
                                                   "}\n"
                                                   ""));

    ui->lbl_activeServiceLabel->setStyleSheet(QString::fromUtf8("QLabel#lbl_activeServiceLabel{\n"
                                                                "color: rgba(208, 209, 209, 230);\n"
                                                                "border-image:transparent;\n"
                                                                "background-color: transparent;\n"
                                                                "border-width: 0px;\n"
                                                                "}\n"
                                                                ""));

    ui->ptyLabel->setStyleSheet(QString::fromUtf8("QLabel#ptyLabel{\n"
                                                  "color: rgba(208, 209, 209, 230);\n"
                                                  "border-image:transparent;\n"
                                                  "background-color: transparent;\n"
                                                  "border-width: 0px;\n"
                                                  "}\n"
                                                  ""));

    ui->lbl_answer->setStyleSheet(QString::fromUtf8("QLabel#lbl_answer{\n"
                                                    "color: rgba(208, 209, 209, 230);\n"
                                                    "border-image:transparent;\n"
                                                    "background-color: transparent;\n"
                                                    "border-width: 0px;\n"
                                                    "}\n"
                                                    ""));

    ui->lbl_stationTracking->setStyleSheet(QString::fromUtf8("QLabel#lbl_stationTracking{\n"
                                                             "color: rgba(208, 209, 209, 230);\n"
                                                             "border-image:transparent;\n"
                                                             "background-color: transparent;\n"
                                                             "border-width: 0px;\n"
                                                             "}\n"
                                                             ""));

    // Make blinking text not visible
    ui->lbl_stationTracking->setVisible(false);

    // Set service list geometry
    ui->lstWidget_currServiceList->setGeometry(QRect(368, 89, 381, 274));
    ui->lstWidget_currServiceList->setParent(this);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(ui->lstWidget_currServiceList);

    // ui->lstWidget_currServiceList->setStylesheet("*:hover {background:gray;}");
    // setStyleSheet("WidgetItem:pressed { background-color: #444444; }");
    ui->lstWidget_currServiceList->setItemDelegate(new Delegate());

    // Set Presets list geometry
    ui->lstWidget_currPresetList->setGeometry(QRect(380, 92, 190, 274));
    ui->lstWidget_currPresetList->setParent(this);
    QHBoxLayout* layoutFm = new QHBoxLayout();
    layoutFm->addWidget(ui->lstWidget_currPresetList);

    // Install event filters
    ui->lbl_radioMoveBar->installEventFilter(this);
    ui->lbl_ensFreqNumber->installEventFilter(this);
    ui->lbl_slsOnXpadImage->installEventFilter(this);
    ui->padDls_Label->installEventFilter(this);
    ui->btn_seekdw->installEventFilter(this);
    ui->btn_seekup->installEventFilter(this);
    ui->btn_minimize->installEventFilter(this);

    ui->lstWidget_currServiceList->installEventFilter(this);

    ui->btn_onoff->grabGesture(Qt::TapAndHoldGesture);
    ui->btn_onoff->installEventFilter(this);

    ui->lstWidget_currPresetList->grabGesture(Qt::TapGesture);
    ui->lstWidget_currPresetList->grabGesture(Qt::TapAndHoldGesture);
    ui->lstWidget_currPresetList->viewport()->installEventFilter(this);
    ui->lstWidget_currPresetList->installEventFilter(this);

    // 'Z' order
    ui->lbl_radioMoveBar->raise();
    ui->lbl_radioBackground->lower();

    // Setup animation
    AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_SETUP);

    // Setup blinking text
    movie = new QMovie(":/station_tracking");
}

void DabRadio::on_btn_mute_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    // Block buttons to avoid multi inputs
    ButtonBlockSignals(true);

    // Send mute/unmute signal
    emit EventFromHmiSignal(EVENT_MUTE_TOGGLE);
}

void DabRadio::ButtonBlockSignals(bool enableBlock)
{
    // Block all buttons but seek up & down because we can use them to stop an ongoing seek action
    ui->btn_mute->blockSignals(enableBlock);
    ui->btn_volup->blockSignals(enableBlock);
    ui->btn_voldw->blockSignals(enableBlock);
    ui->btn_tunePrevious->blockSignals(enableBlock);
    ui->btn_tuneNext->blockSignals(enableBlock);
    ui->btn_list->blockSignals(enableBlock);
    ui->btn_preset->blockSignals(enableBlock);
    ui->btn_setup->blockSignals(enableBlock);
}

void DabRadio::on_btn_volup_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    // Block buttons to avoid multi inputs
    ButtonBlockSignals(true);

    emit EventFromHmiSignal(EVENT_VOLUME_UP);
}

void DabRadio::on_btn_voldw_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    // Block buttons to avoid multi inputs
    ButtonBlockSignals(true);

    emit EventFromHmiSignal(EVENT_VOLUME_DOWN);
}

void DabRadio::on_btn_minimize_clicked()
{
    this->setWindowState(Qt::WindowMinimized);
}

void DabRadio::on_btn_tuneNext_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    // Calculate next index and frequency
    GoToNextFrequency();

    // Do tune
    StartTuneAction();
}

void DabRadio::on_btn_tunePrevious_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    // Calculate previous index and frequency
    GoToPreviousFrequency();

    // Do tune
    StartTuneAction();
}

void DabRadio::on_lstWidget_currServiceList_pressed(const QModelIndex& index)
{
    // BLock signals
    ui->lstWidget_currServiceList->blockSignals(true);

    QString oldState = currentSmMachine->GetCurrentState();
    QString newState = currentSmMachine->CalculateNewState(ACTION_PRESS_SCREEN);

    qDebug() << "On PRESS SCREEN new state is: " << newState;

    ModifyDisplayView(newState, oldState, ACTION_PRESS_SCREEN);

    QString serviceName = onScreenServiceList.at(index.row()); //pressedItem->text();

    if (0 != selectedService.compare(serviceName))
    {
        // Save new selected name
        selectedService = serviceName;

        // Invalid SLS indication
        lastSlsIsValid = false;

        // Signal SLS/PTY status is disabled (use DAB machien because FM never disable PTY)
        smMachineDab->DisableState(STATE_SLS_PTY);

        // Clear fields
        SetClearProperty(RADIO_SERVICE_SELECT_ACTION);

        // Emit signal to radio manager
        DataContainer data;

        // Calculate new service ID
        unsigned int selectedServiceId;
        bool found = false;

        foreach(auto elem, onScreenServiceDatabase)
        {
            if (0 == elem.ServiceLabel.compare(serviceName))
            {
                selectedServiceId = elem.serviceUniqueId;

                found = true;

                break;
            }
        }

        if (true == found)
        {
            data.content = QVariant(selectedServiceId);

            emit EventFromHmiSignal(EVENT_SERVICE_SELECT, data);

            // Now we need to apply a little trick: if the test screen is enabled we would like to return
            // back to quality screen after a while
            if (true == currentSmMachine->CheckEnabledStatus(STATE_QUALITY))
            {
                DelayTimerSetupAndStart(RADIO_DISPLAY_TESTSCREEN, DELAY_ACTION_4_SEC);
            }
        }
    }

    // Enable signals again
    ui->lstWidget_currServiceList->blockSignals(false);
}

void DabRadio::wait(int msec)
{
    waitTimer->setInterval(msec);
    waitTimer->start();

    isWaiting = true;

    while (true == isWaiting)
    {
        QApplication::processEvents(QEventLoop::AllEvents);
    }
}

void DabRadio::GoToNextFrequency()
{
    switch (radioPersistentData.band)
    {
        case BAND_FM:
        {
            // Calculate new frequency
            quint32 tmpFmFreqkHz = fmFreqkHz + fmCountryDependentValues.step;

            // If frequecy is above band limit then set to the minimum value
            if (tmpFmFreqkHz > fmCountryDependentValues.maxBandValue)
            {
                fmFreqkHz = fmCountryDependentValues.minBandValue;
            }
            else
            {
                fmFreqkHz = tmpFmFreqkHz;
            }
        }
        break;

        case BAND_AM:
        {
            // Calculate new frequency
            quint32 tmpMwFreqkHz = mwFreqkHz + mwCountryDependentValues.step;

            // If frequecy is above band limit then set to the minimum value
            if (tmpMwFreqkHz > mwCountryDependentValues.maxBandValue)
            {
                mwFreqkHz = mwCountryDependentValues.minBandValue;
            }
            else
            {
                mwFreqkHz = tmpMwFreqkHz;
            }
        }
        break;

        case BAND_DAB3:
            CalculateNextDabFrequency(dabFreqMHz);

            radioStatusGlobal->panelData.ensembleIndex = getDab3EnsembleIndex(dabFreqMHz);
            break;

        default:
            // No code
            break;
    }
}

void DabRadio::GoToPreviousFrequency()
{
    switch (radioPersistentData.band)
    {
        case BAND_FM:
        {
            // Calculate new frequency
            quint32 tmpFmFreqkHz = fmFreqkHz - fmCountryDependentValues.step;

            // If frequecy is above band limit then set to the minimum value
            if (tmpFmFreqkHz < fmCountryDependentValues.minBandValue)
            {
                fmFreqkHz = fmCountryDependentValues.maxBandValue;
            }
            else
            {
                fmFreqkHz = tmpFmFreqkHz;
            }
        }
        break;

        case BAND_AM:
        {
            // Calculate new frequency
            quint32 tmpMwFreqkHz = mwFreqkHz - mwCountryDependentValues.step;

            // If frequecy is above band limit then set to the minimum value
            if (tmpMwFreqkHz < mwCountryDependentValues.minBandValue)
            {
                mwFreqkHz = mwCountryDependentValues.maxBandValue;
            }
            else
            {
                mwFreqkHz = tmpMwFreqkHz;
            }
        }
        break;

        case BAND_DAB3:
            CalculatePrevDabFrequency(dabFreqMHz);

            radioStatusGlobal->panelData.ensembleIndex = getDab3EnsembleIndex(dabFreqMHz);
            break;

        default:
            // No code
            break;
    }
}

void DabRadio::LoadServicesList(bool display)
{
    int sizeOfServiceList;

    // Clear service list
    onScreenServiceList.clear();
    onScreenServiceDatabase.clear();

    // Get the ensemble table
    ensTable = radioStatusGlobal->panelData.ensembleTable;
    list = radioStatusGlobal->panelData.list;

    // Add services data to 'lstWidget_currServiceList'
    sizeOfServiceList = list.serviceList.size();

    ui->lstWidget_currServiceList->clear();

    // Display services
    for (int i = 0; i < sizeOfServiceList; i++)
    {
        ServiceTy service = list.serviceList.at(i);

        // Item to add (if it has a valid string and if it is an audio service ("0000xxxx")
        if (false == service.ServiceLabel.isEmpty() &&
            true == service.ServiceID.startsWith("0000"))
        {
            onScreenServiceList.append(service.ServiceLabel);
            onScreenServiceDatabase.append(service);
        }
    }

    // Sort the service list
    if (true == radioPersistentData.alphabeticOrder)
    {
        qSort(onScreenServiceList.begin(), onScreenServiceList.end());
    }

    foreach(auto serviceName, onScreenServiceList)
    {
        QString serv = "   " + serviceName;
        QListWidgetItem* item = new QListWidgetItem(QIcon(":/icons/star_ico.png"), serv);

        // To show Services specific small icons instead of fixed STAR icon uncomment next line
        //   item->setIcon(QIcon (getRadioLogo(service.ServiceID)));
        item->setForeground(Qt::white);
        item->setBackground(Qt::transparent);

        ui->lstWidget_currServiceList->addItem(item);
    }

    // Make service list visible
    ui->lstWidget_currServiceList->setVisible(display);

    // Update screen to the current active service (scrollbar, logo, highlighted entry)
    DisplayActiveService();
}

void DabRadio::SetStyleSheetProperty(RadioViewsTy viewName)
{
    QList<QPushButton *> hwButtonsList;
    QString styleSheetStrCustom;
    QList<QPushButton *>::const_iterator iter;

    // Create button list
    hwButtonsList.append(ui->btn_src);
    hwButtonsList.append(ui->btn_volup);
    hwButtonsList.append(ui->btn_voldw);
    hwButtonsList.append(ui->btn_list);
    hwButtonsList.append(ui->btn_preset);
    hwButtonsList.append(ui->btn_setup);
    hwButtonsList.append(ui->btn_mute);
    hwButtonsList.append(ui->btn_seekdw);
    hwButtonsList.append(ui->btn_seekup);

    if (RADIO_HW_INIT_VIEW == viewName || RADIO_HW_OFF_VIEW == viewName)
    {
        const QString styleSheetStr = "QPushButton#<button name>{\n"
                                      "border-image: url(:/images/btn_hardware.png);\n"
                                      "color: rgba(208, 209, 209, 230);\n"
                                      "border-style: groove;\n"
                                      "border-width: 0px;\n"
                                      "}\n"
                                      "\n"
                                      "QPushButton#<button name>:pressed{\n"
                                      "border-style: groove;\n"
                                      "border-width: 0px;\n"
                                      "}\n"
                                      "";

        // Apply style sheet
        for (iter = hwButtonsList.begin(); iter != hwButtonsList.end(); ++iter)
        {
            QPushButton* tmpPushButton = *iter;

            styleSheetStrCustom = styleSheetStr;
            styleSheetStrCustom = styleSheetStrCustom.replace("<button name>", tmpPushButton->objectName());

            tmpPushButton->setStyleSheet(styleSheetStrCustom);
        }

        ui->btn_minimize->setStyleSheet(QString::fromUtf8("QPushButton#btn_minimize{\n"
                                                          "border-image: url(:/images/minimize_gray.png);\n"
                                                          "color: rgb(144, 151, 255);\n"
                                                          "border-style: groove;\n"
                                                          "border-width: 0px;\n"
                                                          "}\n"
                                                          "\n"
                                                          "QPushButton#btn_minimize:pressed{\n"
                                                          "border-style: groove;\n"
                                                          "border-width: 3px;\n"
                                                          "}\n"
                                                          ""));

        if (RADIO_HW_INIT_VIEW == viewName)
        {
            ui->lbl_on_off->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/on_off_gray_ico.png);"));
        }
        else
        {
            ui->lbl_on_off->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/on_off_red_ico.png);"));
        }

        ui->btn_closeAppz->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/exit_ico_off.png);\n" "\n" ""));
    }
    else if (RADIO_POWER_ON_VIEW == viewName || RADIO_HW_ON_VIEW == viewName)
    {
        const QString styleSheetStr = "QPushButton#<button name>{\n"
                                      "border-image: url(:/images/btn_hardware.png);\n"
                                      "color: rgb(144, 151, 255);\n"
                                      "border-style: groove;\n"
                                      "border-width: 0px;\n"
                                      "}\n"
                                      "\n"
                                      "QPushButton#<button name>:pressed{\n"
                                      "border-style: groove;\n"
                                      "border-width: 3px;\n"
                                      "}\n"
                                      "";

        // Apply style sheet
        for (iter = hwButtonsList.begin(); iter != hwButtonsList.end(); ++iter)
        {
            QPushButton* tmpPushButton = *iter;

            styleSheetStrCustom = styleSheetStr;
            styleSheetStrCustom = styleSheetStrCustom.replace("<button name>", tmpPushButton->objectName());

            tmpPushButton->setStyleSheet(styleSheetStrCustom);
        }

        ui->btn_minimize->setStyleSheet(QString::fromUtf8("QPushButton#btn_minimize{\n"
                                                          "border-image: url(:/images/minimize_cyan.png);\n"
                                                          "color: rgb(144, 151, 255);\n"
                                                          "border-style: groove;\n"
                                                          "border-width: 0px;\n"
                                                          "}\n"
                                                          "\n"
                                                          "QPushButton#btn_minimize:pressed{\n"
                                                          "border-style: groove;\n"
                                                          "border-width: 3px;\n"
                                                          "}\n"
                                                          ""));

        ui->lbl_on_off->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/on_off_cyan_ico.png);"));
        ui->btn_closeAppz->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/exit_ico_on.png);\n" "\n" ""));
    }
}

void DabRadio::SetClearProperty(RadioActionsTy actionName)
{
    // Clear service name label
    ui->lbl_activeServiceLabel->clear();

    // Clear program type label
    ui->ptyLabel->clear();

    // Set logo
    ui->lbl_epgLogo->clear();

    // Clear SLS
    ui->lbl_slsOnXpadImage->clear();

    // Clear DLS
    ui->padDls_Label->clear();

    // Clear service ID label
    ui->lbl_PIid->clear();

    // Force Good Signal indication
    ui->lbl_answer->clear();

    if (RADIO_POWER_OFF_ACTION == actionName)
    {
        // Clear tuned frequency
        ui->lbl_tuneFreq->clear();

        // Clear ensemble frequency name
        ui->lbl_ensFreqNumber->clear();

        // Clear 'wait' indicator
        ui->lbl_wait->clear();

        // Clear ensemble label
        ui->lbl_ensLabel->clear();

        // Clear current service list
        ui->lstWidget_currServiceList->clear();
    }

    if (RADIO_TUNE_FREQUENCY_ACTION == actionName ||
        RADIO_SEEK_UP_ACTION == actionName ||
        RADIO_SEEK_DOWN_ACTION == actionName)
    {
        // Clear ensemble label
        ui->lbl_ensLabel->clear();

        signalTextToDisplay.clear();

        ui->lbl_ensLabel->clear();
        ui->rdsAfList_label->clear();
        ui->lbl_quality_1->clear();
        ui->lbl_quality_2->clear();
        ui->lbl_quality_3->clear();
        ui->lbl_quality_4->clear();
        ui->lbl_quality_5->clear();
        ui->lbl_quality_6->clear();
        ui->lbl_quality_7->clear();
        ui->lbl_quality_8->clear();
        ui->lbl_quality_9->clear();
        ui->lbl_quality_10->clear();
        ui->lbl_quality_11->clear();

        ui->lstWidget_currServiceList->clear();

        if (RADIO_SEEK_UP_ACTION == actionName || RADIO_SEEK_DOWN_ACTION == actionName)
        {
            ui->lbl_tuneFreq->clear();
            ui->lbl_ensFreqNumber->clear();
        }
    }

    if (RADIO_SOURCE_ACTION == actionName)
    {
        // Clear ensemble label
        ui->lbl_ensLabel->clear();

        signalTextToDisplay.clear();
        ui->lbl_radioBand->clear();
        ui->lbl_bandTop->clear();
        ui->lbl_bandBottom->clear();
        ui->lbl_tuneFreq->clear();
        ui->lbl_activeServiceBitrate->clear();
        ui->lbl_ensLabel->clear();

        ui->lbl_ensFreqNumber->clear();
        ui->lbl_wait->clear();

        ui->rdsAfList_label->clear();
        ui->lbl_quality_1->clear();
        ui->lbl_quality_2->clear();
        ui->lbl_quality_3->clear();
        ui->lbl_quality_4->clear();
        ui->lbl_quality_5->clear();
        ui->lbl_quality_6->clear();
        ui->lbl_quality_7->clear();
        ui->lbl_quality_8->clear();
        ui->lbl_quality_9->clear();
        ui->lbl_quality_10->clear();
        ui->lbl_quality_11->clear();

        ui->lstWidget_currServiceList->clear();
    }

    if (RADIO_PRESET_ACTION == actionName)
    {
        // Clear ensemble label
        ui->lbl_ensLabel->clear();

        ui->lbl_ensLabel->clear();
        ui->lbl_radioBand->clear();
        ui->lbl_activeServiceBitrate->clear();
        ui->lbl_tuneFreq->clear();
        ui->lbl_ensFreqNumber->clear();
        ui->lbl_bandBottom->clear();
        ui->lbl_bandTop->clear();
    }

    if (RADIO_SERVICE_SELECT_ACTION == actionName)
    {
        // All clears are done in the common part
    }
}

void DabRadio::SetVisibleProperty(RadioActionsTy actionName)
{
    if (RADIO_TUNE_FREQUENCY_ACTION == actionName)
    {
        // Hide stero and TA/TP indicators
        ui->stereoOn_label->setVisible(false);
        ui->ta_label->setVisible(false);
        ui->tp_label->setVisible(false);

        // Hide ensemble radio labels
        hide_ensembleRadioLabels();
    }
    else if (RADIO_SOURCE_ACTION == actionName)
    {
        ui->lstWidget_currPresetList->setVisible(false);
        ui->lstWidget_currServiceList->setVisible(false);
        ui->ptyLabel->setVisible(false);
        ui->lbl_PIid->setVisible(true);
        ui->lbl_wait->setVisible(false);
        ui->tp_label->setVisible(false);
        ui->ta_label->setVisible(false);
        ui->stereoOn_label->setVisible(false);
        ui->af_label->setVisible(false);
        ui->dab_fm_label->setVisible(false);
        ui->dab_dab_label->setVisible(false);
        ui->padDls_Label->setVisible(false);

        // Hide ensemble radio labels
        hide_ensembleRadioLabels();
    }
    else if (RADIO_PRESET_ACTION == actionName)
    {
        ui->tp_label->setVisible(false);
        ui->ta_label->setVisible(false);
        ui->stereoOn_label->setVisible(false);
        ui->af_label->setVisible(false);
    }
}

void DabRadio::SetVisibleProperty(RadioViewsTy viewName)
{
    if (RADIO_HW_OFF_VIEW == viewName || RADIO_HW_INIT_VIEW == viewName)
    {
        // Make all lists invisible
        QList<QLabel *> listLabels = this->findChildren<QLabel *> ();

        for (auto label : listLabels)
        {
            label->setVisible(false);
        }

        // Make all buttons visible...
        QList<QPushButton *> listButtons = this->findChildren<QPushButton *> ();

        for (auto button : listButtons)
        {
            button->setVisible(true);
        }

        //...but the unused ones
        for (auto unusedButton : unusedButtonsWidgetList)
        {
            unusedButton->setVisible(false);
        }

        // Edges
        ui->lbl_edge_x1->setVisible(true);
        ui->lbl_edge_x1_1->setVisible(true);
        ui->lbl_edge_x1_2->setVisible(true);
        ui->lbl_edge_x2->setVisible(true);
        ui->lbl_edge_y1->setVisible(true);
        ui->lbl_edge_y2->setVisible(true);
        ui->lbl_edge_y3->setVisible(true);

        // Make specific items visible/invisible state different from default
        ui->lbl_bandBottom->setVisible(false);
        ui->lbl_bandTop->setVisible(false);

        ui->btn_tunePrevious->setVisible(false);
        ui->btn_tuneNext->setVisible(false);
        ui->lbl_radioMoveBar->setVisible(true);
        ui->lbl_radioBackground->setVisible(true);
        ui->lstWidget_currServiceList->setVisible(false);
        ui->hFreqSlider->setVisible(false);
        ui->lbl_on_off->setVisible(true);

        ui->lstWidget_currPresetList->setVisible(false);
    }
    else if (RADIO_POWER_ON_VIEW == viewName || RADIO_HW_ON_VIEW == viewName)
    {
        // Set all labels to visible
        QList<QLabel *> listLabels = this->findChildren<QLabel *> ();

        for (auto label : listLabels)
        {
            label->setVisible(true);
        }

        // Make all buttons visible...
        QList<QPushButton *> listButtons = this->findChildren<QPushButton *> ();

        for (auto button : listButtons)
        {
            button->setVisible(true);
        }

        //...but the unused ones
        for (auto unusedButton : unusedButtonsWidgetList)
        {
            unusedButton->setVisible(false);
        }

        // Make visible or invisible specific items
        ui->lstWidget_currServiceList->setVisible(true);
        ui->lbl_audio_off_status->setVisible(false);

        ui->tp_label->setVisible(false);
        ui->ta_label->setVisible(false);

        ui->hFreqSlider->setVisible(true);
        ui->lbl_wait->setVisible(false);
        ui->lbl_slsOnXpadImage->setVisible(false);
        ui->padDls_Label->setVisible(false);

        ui->lbl_stationTracking->setVisible(false);

        hide_ensembleRadioLabels();

        // Stereo flag update
        updateStereoFlag(0);

        // Signal strength icon set to signal absent
        SetSignalStrengthIcon(true, BER_NOT_AVAILABLE);
    }
}

void DabRadio::SetZOrderProperty(RadioViewsTy viewName)
{
    if (RADIO_HW_OFF_VIEW == viewName || RADIO_HW_ON_VIEW == viewName ||
        RADIO_POWER_ON_VIEW == viewName || RADIO_HW_INIT_VIEW == viewName)
    {
        ui->lbl_radioMoveBar->raise();
        ui->lbl_radioBackground->lower();
    }
}

void DabRadio::BackgroundLblOpacityAnimation_ON(AnimationActionsTy animationAction, bool status)
{
    static QGraphicsOpacityEffect* graphicsOpacityEffect;
    static QPropertyAnimation* propertyAnimation;

    if (animationAction == ANIMATION_START)
    {
        graphicsOpacityEffect = new QGraphicsOpacityEffect(this);
        propertyAnimation = new QPropertyAnimation(this);

        // 'lbl_radioBackground' settings
        if (true == status)
        {
            graphicsOpacityEffect->setOpacity(1);
        }
        else
        {
            graphicsOpacityEffect->setOpacity(0);
        }

        ui->lbl_radioBackground->setGraphicsEffect(graphicsOpacityEffect);
        ui->lbl_radioBackground->setStyleSheet(QString::fromUtf8("border-image: url(:/images/radio_window_background_on.png);"));
        ui->lbl_radioBackground->setGeometry(QRect(150, 40, 601, 391));
        ui->lbl_radioBackground->setVisible(true);
        ui->lbl_radioBackground->raise();

        propertyAnimation->setTargetObject(graphicsOpacityEffect);
        propertyAnimation->setPropertyName("opacity");

        if (true == status)
        {
            propertyAnimation->setDuration(3000);
            propertyAnimation->setStartValue(0);
            propertyAnimation->setEndValue(1);
        }
        else
        {
            propertyAnimation->setDuration(2500);
            propertyAnimation->setStartValue(1);
            propertyAnimation->setEndValue(0);
        }

        propertyAnimation->setLoopCount(1);
        propertyAnimation->setEasingCurve(QEasingCurve::Linear);

        // Start animation
        propertyAnimation->start(QAbstractAnimation::DeleteWhenStopped);

        if (false == status)
        {
            // Wait
            while (propertyAnimation->state() == QAbstractAnimation::Running)
            {
                QApplication::processEvents(QEventLoop::AllEvents);
            }
        }
    }
    else if (animationAction == ANIMATION_STOP)
    {
        // Stop animation
        propertyAnimation->stop();

        // Wait
        while (propertyAnimation->state() != QAbstractAnimation::Stopped)
        {
            QApplication::processEvents(QEventLoop::AllEvents);
        }
    }
}

void DabRadio::STLogoLblOpacityAnimation_ON(AnimationActionsTy animationAction, bool status)
{
    static QGraphicsOpacityEffect* graphicsOpacityEffect;
    static QPropertyAnimation* propertyAnimation;

    if (animationAction == ANIMATION_START)
    {
        graphicsOpacityEffect = new QGraphicsOpacityEffect(this);
        propertyAnimation = new QPropertyAnimation(this);

        // Settings for 'lbl_radioBackground'
        graphicsOpacityEffect->setOpacity(1);
        ui->lbl_radioBackground->setGraphicsEffect(graphicsOpacityEffect);
        ui->lbl_radioBackground->setStyleSheet(QString::fromUtf8("border-image: url(:/images/ST_Logo.png);"));
        ui->lbl_radioBackground->setGeometry(QRect(150, 40, 601, 391));

        ui->lbl_radioBackground->setVisible(true);
        ui->lbl_radioBackground->raise();

        propertyAnimation->setTargetObject(graphicsOpacityEffect);
        propertyAnimation->setPropertyName("opacity");

        if (true == status)
        {
            propertyAnimation->setDuration(700);
            propertyAnimation->setStartValue(0);
            propertyAnimation->setEndValue(1);
        }
        else
        {
            propertyAnimation->setDuration(500);
            propertyAnimation->setStartValue(1);
            propertyAnimation->setEndValue(0);
        }

        propertyAnimation->setLoopCount(1);
        propertyAnimation->setEasingCurve(QEasingCurve::Linear);

        // Start animation
        propertyAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else if (animationAction == ANIMATION_STOP)
    {
        // Stop animation
        propertyAnimation->stop();

        // Wait
        while (propertyAnimation->state() != QAbstractAnimation::Stopped)
        {
            QApplication::processEvents(QEventLoop::AllEvents);
        }
    }
}

void DabRadio::RadioOffViewAnimationGroup()
{
    // Opacity animation
    lbl_radioBackground_opacity = new QGraphicsOpacityEffect(this);
    lbl_radioBackground_animation = new QPropertyAnimation(this);
    ui->lbl_radioBackground->setGraphicsEffect(lbl_radioBackground_opacity);
    ui->lbl_radioBackground->setVisible(true);
    ui->lbl_radioBackground->raise();
    lbl_radioBackground_animation->setTargetObject(lbl_radioBackground_opacity);
    lbl_radioBackground_animation->setPropertyName("opacity");
    lbl_radioBackground_animation->setDuration(1000);  // 2900
    lbl_radioBackground_animation->setStartValue(1);
    lbl_radioBackground_animation->setEndValue(0);
    lbl_radioBackground_animation->setLoopCount(1);
    lbl_radioBackground_animation->setEasingCurve(QEasingCurve::Linear);

    // Create animations group
    radioOffAnimationGroup = new QParallelAnimationGroup(this);
    radioOffAnimationGroup->addAnimation(lbl_radioBackground_animation);
    radioOffAnimationGroup->start(QAbstractAnimation::DeleteWhenStopped);

    // Wait
    while (radioOffAnimationGroup->state() != QAbstractAnimation::Stopped)
    {
        QApplication::processEvents(QEventLoop::AllEvents);
    }

    qDebug() << "Ended RadioOff Animation Group";

    // Check if we requested to close the application or turn-off the radio
    if (true == isClosing)
    {
        emit CloseApplication_Signal();
    }

    // Enable power-on/off button again
    ui->btn_onoff->blockSignals(false);
}

void DabRadio::IcoWaitAnimation(AnimationActionsTy animationAction)
{
    static QTimer* waitAnimationTimer;
    static QStateMachine* stateMachine;

    static QState* state1;
    static QState* state2;
    static QState* state3;
    static QState* state4;

    if (animationAction == ANIMATION_SETUP)
    {
        // SETUP TIMER
        waitAnimationTimer = new QTimer(this);
        waitAnimationTimer->setInterval(200);

        connect(waitAnimationTimer, SIGNAL(timeout()), this, SLOT(waitAnimationTimer_timeout_slot()));

        // STATE MACHINE SETUP
        stateMachine = new QStateMachine(this);

        // Create graphics states
        state1 = new QState();
        state2 = new QState();
        state3 = new QState();
        state4 = new QState();

        // Assign states proprieties
        state1->assignProperty(ui->lbl_wait, "styleSheet", (QString::fromUtf8("border-image: url(:/icons/loading_ico_1.png);")));
        state2->assignProperty(ui->lbl_wait, "styleSheet", (QString::fromUtf8("border-image: url(:/icons/loading_ico_2.png);")));
        state3->assignProperty(ui->lbl_wait, "styleSheet", (QString::fromUtf8("border-image: url(:/icons/loading_ico_3.png);")));
        state4->assignProperty(ui->lbl_wait, "styleSheet", (QString::fromUtf8("border-image: url(:/icons/loading_ico_4.png);")));

        // Assign states transitions
        state1->addTransition(waitAnimationTimer, SIGNAL(timeout()), state2);
        state2->addTransition(waitAnimationTimer, SIGNAL(timeout()), state3);
        state3->addTransition(waitAnimationTimer, SIGNAL(timeout()), state4);
        state4->addTransition(waitAnimationTimer, SIGNAL(timeout()), state1);

        // Add state to state machine
        stateMachine->addState(state1);
        stateMachine->addState(state2);
        stateMachine->addState(state3);
        stateMachine->addState(state4);

        stateMachine->setInitialState(state1);
    }
    else if (animationAction == ANIMATION_START)
    {
        if (radioPersistentData.band == BAND_DAB3)
        {
            ui->lbl_wait->setVisible(true);
            ui->lbl_wait->raise();
        }

        stateMachine->start();
        waitAnimationTimer->start();
    }
    else if (animationAction == ANIMATION_STOP)
    {
        waitAnimationTimer->stop();
        stateMachine->stop();

        ui->lbl_wait->setVisible(false);
        ui->lbl_wait->lower();
    }
}

void DabRadio::ViewsSwitcher(RadioViewsTy viewName)
{
    SetStyleSheetProperty(viewName);

    SetVisibleProperty(viewName);

    SetZOrderProperty(viewName);
}

void DabRadio::AnimationsMachine(AnimationNameTy animationName, AnimationActionsTy animationAction)
{
    switch (animationName)
    {
        case LABEL_RADIO_BACKGROUND_OPACITY_ANIMATION_OFF:
            BackgroundLblOpacityAnimation_ON(animationAction, false);
            break;

        case LABEL_RADIO_BACKGROUND_OPACITY_ANIMATION_ON:
            BackgroundLblOpacityAnimation_ON(animationAction, true);
            break;

        case LABEL_ST_LOGO_OPACITY_ANIMATION_OFF:
            STLogoLblOpacityAnimation_ON(animationAction, false);
            break;

        case LABEL_ST_LOGO_OPACITY_ANIMATION_ON:
            STLogoLblOpacityAnimation_ON(animationAction, true);
            break;

        case RADIO_OFF_ANIMATION_GROUP:
            RadioOffViewAnimationGroup();
            break;

        case LABEL_WAIT_ANIMATION:
            if (animationAction == ANIMATION_START)
            {
                ui->lbl_wait->setVisible(true);
                ui->lbl_wait->raise();
            }
            else if (animationAction == ANIMATION_STOP)
            {
                ui->lbl_wait->setVisible(false);
                ui->lbl_wait->lower();
            }

            IcoWaitAnimation(animationAction);
            break;

        default:
            // No code
            break;
    }
}

void DabRadio::setMwFreqLabels()
{
    int currentStep = (mwFreqkHz - mwCountryDependentValues.minBandValue) / mwCountryDependentValues.step;

    ui->hFreqSlider->setValue(currentStep);

    ui->lbl_ensFreqNumber->setText("  AM  ");
    ui->lbl_tuneFreq->setText(QString::number(mwFreqkHz) + " kHz");
    ui->lbl_activeServiceLabel->setText(QString::number(mwFreqkHz) + " kHz");

    ui->ptyLabel->clear();
    ui->ptyLabel->setVisible(true);
    ui->lbl_ensLabel->setVisible(false);
    ui->lbl_slsOnXpadImage->setVisible(false);

    // Clear SLS, DLS and LOGOS
    ui->lbl_slsOnXpadImage->clear();
    ui->padDls_Label->clear();
    ui->lbl_epgLogo->clear();
}

void DabRadio::setFmFreqLabels()
{
    QString freqMHz = QString::number(((float)fmFreqkHz / (float)1000), 'f', 1);

    int currentStep = (fmFreqkHz - fmCountryDependentValues.minBandValue) / fmCountryDependentValues.step;

    ui->hFreqSlider->setValue(currentStep);

    ui->lbl_ensFreqNumber->setText("  FM  ");

    ui->lbl_tuneFreq->setText(freqMHz + " MHz");

    ui->lbl_activeServiceLabel->setText(freqMHz + " MHz");

    ui->lbl_PIid->clear();
    ui->lbl_PIid->setVisible(true);
    ui->ptyLabel->clear();
    ui->ptyLabel->setVisible(true);

    ui->lbl_ensLabel->setVisible(false);
    ui->lbl_slsOnXpadImage->setVisible(false);

    // Clear SLS, DLS and LOGOS
    ui->lbl_slsOnXpadImage->clear();
    ui->padDls_Label->clear();
    ui->lbl_epgLogo->clear();
}

void DabRadio::setDabFreqLabels(void)
{
    int tmpIndex;

    // Clear messages
    ui->ptyLabel->clear();
    ui->lbl_ensLabel->clear();
    ui->lbl_PIid->clear();
    ui->lbl_activeServiceLabel->clear();
    ui->lstWidget_currServiceList->clear();
    ui->lbl_answer->clear();

    // Clear SLS, DLS and LOGOS
    ui->lbl_slsOnXpadImage->clear();
    ui->padDls_Label->clear();
    ui->lbl_epgLogo->clear();

    tmpIndex = getDab3EnsembleIndex(dabFreqMHz);

    if (INVALID_SDATA != tmpIndex)
    {
        radioStatusGlobal->panelData.ensembleIndex = tmpIndex;

        ui->hFreqSlider->setValue(tmpIndex);

        ui->lbl_ensLabel->setText(radioStatusGlobal->panelData.ensembleTable.EnsChLabel);

        ui->lbl_tuneFreq->setText(dabFreq2String(dabFreqMHz));

        ui->lbl_ensFreqNumber->setText(getDab3Channel(tmpIndex));
    }
}

void DabRadio::initSlider(quint8 band)
{
    if (BAND_AM == band)
    {
        ui->lbl_radioBand->setText("AM");

        // Init slider number of steps
        int numSteps = (mwCountryDependentValues.maxBandValue - mwCountryDependentValues.minBandValue) / mwCountryDependentValues.step;
        ui->hFreqSlider->setMaximum(numSteps);

        ui->lbl_bandTop->setText(QString::number(mwCountryDependentValues.maxBandValue));
        ui->lbl_bandBottom->setText(QString::number(mwCountryDependentValues.minBandValue));
    }
    else if (BAND_FM == band)
    {
        int numSteps = (fmCountryDependentValues.maxBandValue - fmCountryDependentValues.minBandValue) / fmCountryDependentValues.step;
        ui->hFreqSlider->setMaximum(numSteps);

        ui->lbl_radioBand->setText("FM");

        if (COUNTRY_US == radioPersistentData.country)
        {
            // US settings
            ui->lbl_bandBottom->setText(QString::number(fmCountryDependentValues.minBandValue / 1000) + ".9");
            ui->lbl_bandTop->setText(QString::number(fmCountryDependentValues.maxBandValue / 1000) + ".9");
        }
        else
        {
            // Default to Europe
            ui->lbl_bandBottom->setText(QString::number(fmCountryDependentValues.minBandValue / 1000) + ".5");
            ui->lbl_bandTop->setText(QString::number(fmCountryDependentValues.maxBandValue / 1000) + ".0");
        }
    }
    else if (BAND_DAB3 == band)
    {
        ui->hFreqSlider->setMaximum(MAXDAB_INDEX);

        ui->lbl_radioBand->setText("DAB");
        ui->lbl_bandTop->setText("13F");
        ui->lbl_bandBottom->setText("5A");
    }
}

void DabRadio::displaySelectedBand()
{
    ui->lbl_slsOnXpadImage->setVisible(false);

    ui->lstWidget_currServiceList->setVisible(false);
    ui->lstWidget_currPresetList->setVisible(false);
    ui->lbl_activeServiceBitrate->setVisible(false);

    initSlider(radioPersistentData.band);

    switch (radioPersistentData.band)
    {
        case BAND_AM:
        case BAND_LW:
        case BAND_MW:
        case BAND_SW:
            setMwFreqLabels();
            break;

        case BAND_DAB3:
            setDabFreqLabels();
            break;

        case BAND_FM:
            setFmFreqLabels();
            break;

        case BAND_DRM30:
        case BAND_DRM_PLUS:
        default:
            // No code
            break;
    }
}

void DabRadio::on_btn_src_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    isPresetListOn = false;
    lastSlsIsValid = false;

    setPanel(RADIO_SOURCE_ACTION);

    // Block buttons to avoid multi inputs
    ButtonBlockSignals(true);

    // Signal command ongoing to block onscreen information like DLS
    tuneIsOngoing = true;

    // Send signal to global list application
    emit GlobalListApplicationProlog_Signal(EVENT_SOURCE, radioPersistentData.band);
}

void DabRadio::on_hFreqSlider_valueChanged(int value)
{
    int freq;

    switch (radioPersistentData.band)
    {
        case BAND_FM:
        {
            freq = (fmCountryDependentValues.minBandValue + fmCountryDependentValues.step * value);

            ui->lbl_tuneFreq->setText(fmFreq2String(freq));
            ui->lbl_activeServiceLabel->setText(fmFreq2String(freq));
        }
        break;

        case BAND_AM:
        {
            freq = (mwCountryDependentValues.minBandValue + mwCountryDependentValues.step * value);

            ui->lbl_tuneFreq->setText(QString::number(freq, 10) + "kHz");
            ui->lbl_activeServiceLabel->setText(QString::number(freq, 10) + "kHz");
        }
        break;

        case BAND_DAB3:
        {
            QString tmpEnsembleNumber = getDab3Channel(value);

            freq = getDab3FrequencyVal(value);

            ui->lbl_ensFreqNumber->setText(tmpEnsembleNumber);
            ui->lbl_tuneFreq->setText(dabFreq2String(freq));
        }
        break;

        default:
            // No code
            break;
    }
}

void DabRadio::on_hFreqSlider_sliderReleased()
{
    int value;

    if (false == CheckIfButtonActionCanBeDone())
    {
        // Return if radio status is not the proper one
        return;
    }

    // Get the sender object
    QObject* senderObj = sender();
    QSlider* tmpObj = dynamic_cast<QSlider *> (senderObj);

    value = tmpObj->value();

    // Signal slider is no more moved
    sliderCurrentlyMoved = false;

    switch (radioPersistentData.band)
    {
        case BAND_FM:
        {
            // Calculate new frequency
            quint32 tmpNewFmFreqkHz = fmCountryDependentValues.minBandValue + (fmCountryDependentValues.step * value);

            if (tmpNewFmFreqkHz == fmFreqkHz)
            {
                // If the same frequency is tuner then just return, no action to be done
                return;
            }

            // Adjust frequency
            fmFreqkHz = tmpNewFmFreqkHz;
        }
        break;

        case BAND_AM:
        {
            // Calculate new frequency
            quint32 tmpNewAmFreqkHz = mwCountryDependentValues.minBandValue + (mwCountryDependentValues.step * value);

            if (tmpNewAmFreqkHz == mwFreqkHz)
            {
                // If the same frequency is tuner then just return, no action to be done
                return;
            }

            // Adjust frequency
            mwFreqkHz = tmpNewAmFreqkHz;
        }
        break;

        case BAND_DAB3:
        {
            quint32 newFreq = getDab3FrequencyVal(value);

            if (dabFreqMHz == newFreq)
            {
                // If the same frequency is tuner then just return, no action to be done
                return;
            }

            // Adjust frequency
            dabFreqMHz = getDab3FrequencyVal(value);

            // Save frequency information in the global variable
            radioStatusGlobal->panelData.ensembleIndex = value;
        }
        break;

        default:
            // No code
            break;
    }

    // Do tune
    StartTuneAction();
}

void DabRadio::BlinkingText(bool start)
{
    if (true == start)
    {
        ui->lbl_stationTracking->setMovie(movie);

        ui->lbl_stationTracking->setVisible(true);

        movie->start();
    }
    else
    {
        ui->lbl_stationTracking->setVisible(false);

        movie->stop();
    }
}

void DabRadio::StartTuneAction()
{
    // Signal command ongoing to block onscreen information like DLS
    tuneIsOngoing = true;

    // Block slider signals before display redraw
    ui->hFreqSlider->blockSignals(true);

    // Signal SLS/PTY status is disabled
    smMachineDab->DisableState(STATE_SLS_PTY);

    QString oldState = currentSmMachine->GetCurrentState();
    QString newState = currentSmMachine->CalculateNewState(ACTION_TUNE);

    qDebug() << "On TUNE new state is: " << newState;

    ModifyDisplayView(newState, oldState, ACTION_TUNE);

    // SLS is no more valid
    lastSlsIsValid = false;

    // Disable buttons
    ButtonBlockSignals(true);

    // Emit signal for frequency change
    DataContainer data;

    // We need to avoid to expire the preset timer because we start a tune action
    // maybe meanwhile presets are visible and timer to return to old view has been started
    if (true == delayActionTimer->isActive() && RADIO_PRESET_ACTION == actionExecuted)
    {
        delayActionTimer->stop();
        actionExecuted = RADIO_ACTION_NONE;
    }

    // Set slider
    switch (radioPersistentData.band)
    {
        case BAND_FM:
            // Set labels
            setFmFreqLabels();

            qDebug() << "--- USER tune command   ---> " << QString::number(fmFreqkHz) << " <---";

            // Send the preset selected by the user
            data.content = QVariant(fmFreqkHz);
            break;

        case BAND_AM:
            // Set labels
            setMwFreqLabels();

            qDebug() << "--- USER tune command   ---> " << QString::number(mwFreqkHz) << " <---";

            // Send the preset selected by the user
            data.content = QVariant(mwFreqkHz);
            break;

        case BAND_DAB3:
            // Set labels
            setDabFreqLabels();

            // Display blinking information until the service is extracted
            BlinkingText(true);

            qDebug() << "--- USER tune command   ---> " << QString::number(dabFreqMHz) << " <---";

            // Send the preset selected by the user
            data.content = QVariant(dabFreqMHz);
            break;

        default:
            // No code
            break;
    }

    // Emit signal for the radio manager
    emit EventFromHmiSignal(EVENT_FREQUENCY_CHANGE, data);

    // Enable slider signals again
    ui->hFreqSlider->blockSignals(false);
}

void DabRadio::rdsPanelUpdateData(RdsDataSetTableTy rdsdataset)
{
    bool ok;

    if (BAND_FM == radioPersistentData.band)
    {
        currentPIid = rdsdataset.PIid;

        ui->lbl_PIid->setText(rdsdataset.PIid);

        // Show the radio logo
        radioStatusGlobal->radioStatus.persistentData.serviceId = currentPIid.toUInt(&ok, 16);

        if ((true == radioPersistentData.rdsRegionalEn) &&
            (currentPIid.mid(1, 1).toUInt(&ok, 10) >= 5))
        {
            QString basePIid  = currentPIid.left(1) + "2" + currentPIid.right(2);
            DisplayRadioLogo("0000" + basePIid);
        }
        else
        {
            DisplayRadioLogo("0000" + currentPIid);
        }

        if (false == ui->testscreen_widget->isVisible())
        {
            // Display PTY image
            if (rdsdataset.ptyVal >= 0)
            {
                // Stop the timer to display default PTY image if running
                if (RADIO_DISPLAY_SLSPTY == actionExecuted)
                {
                    delayActionTimer->stop();
                }

                // Display the PTY image
                Display_SlsPtyImage(rdsdataset.ptyVal);
            }
            else
            {
                // Set timer with executed action in order to display PTY image on timer
                // (this is done to avoid image flickering due to default image immediately followed by pty related one)
                if (RADIO_ACTION_NONE == actionExecuted)
                {
                    DelayTimerSetupAndStart(RADIO_DISPLAY_SLSPTY, DELAY_ACTION_500_MSEC);
                }
            }
        }

        ui->ptyLabel->setText(rdsdataset.ptyName);
        ui->tp_label->setVisible(rdsdataset.tpFlag);
        ui->ta_label->setVisible(rdsdataset.taFlag);

        if (true == rdsdataset.taFlag)
        {
            ui->lbl_activeServiceLabel->setText("TRAFFIC");
        }
        else
        {
            if (false == sliderCurrentlyMoved)
            {
                if (false == rdsdataset.PSname.isEmpty())
                {
                    ui->lbl_activeServiceLabel->setText(rdsdataset.PSname.trimmed());

                    //qDebug() << "rdsdataset.PSname = " << rdsdataset.PSname.trimmed();
                }
                else
                {
                    QString PsDefaultFile = "./radio_logos/defaultPsName.txt";

                    ui->lbl_activeServiceLabel->setText((DisplayDefaultPSname(PsDefaultFile, "0000" + currentPIid)).trimmed());
                }
            }
        }

        ui->lbl_activeServiceLabel->setVisible(true);

        ui->padDls_Label->setText(rdsdataset.rtText.trimmed());
        ui->padDls_Label->setVisible(true);

        QString tmpAf;
        double freqV;
        int cnt;

        tmpAf.clear();

        for (cnt = 0; cnt < 26; cnt++)
        {
            if (rdsdataset.RdsAFList[cnt] != 205)
            {
                if (ui->hexAf_checkBox->isChecked())
                {
                    if (rdsdataset.RdsAFList[cnt] > 15)
                    {
                        tmpAf = tmpAf + QString::number(rdsdataset.RdsAFList[cnt], 16) + " ";
                    }
                    else
                    {
                        tmpAf = tmpAf + "0" + QString::number(rdsdataset.RdsAFList[cnt], 16) + " ";
                    }
                }
                else
                {
                    freqV = (87500.0 + 100.0 * rdsdataset.RdsAFList[cnt]) / 1000.0;

                    if (rdsdataset.RdsAFList[cnt] > 124)
                    {
                        tmpAf = tmpAf + QString::number(freqV, 'f', 1) + " ";
                    }
                    else
                    {
                        tmpAf = tmpAf + "_" + QString::number(freqV, 'f', 1) + " ";
                    }
                }
            }
            else
            {
                break;
            }
        }

        ui->rdsAfList_label->setText("AF List:   Nb AFs = " + QString::number(cnt, 10) + "\n" + tmpAf);
    }
    else
    {
        qDebug() << "RDS: 'rdsPanelUpdateData' called not in FM";
    }
}

void DabRadio::on_btn_onoff_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    // Block buttons to avoid multi inputs
    ui->btn_onoff->blockSignals(true);

    // Action depends on current radio status
    if (false == radioIsOn)
    {
        PowerOnAction();
    }
    else
    {
        PowerOffAction();
    }
}

void DabRadio::CalculateCountryDependentValues(CountryTy country)
{
    if (COUNTRY_US == country)
    {
        // Set for US
        mwCountryDependentValues.minBandValue = AM_US_MIN_FREQ;
        mwCountryDependentValues.maxBandValue = AM_US_MAX_FREQ;
        mwCountryDependentValues.step = AM_US_STEP;
        mwCountryDependentValues.ptyStr = "_US_";

        fmCountryDependentValues.minBandValue = FM_US_MIN_FREQ;
        fmCountryDependentValues.maxBandValue = FM_US_MAX_FREQ;
        fmCountryDependentValues.step = FM_US_STEP;
        fmCountryDependentValues.ptyStr = "_US_";
    }
    else
    {
        // Default to EU
        mwCountryDependentValues.minBandValue = AM_EU_MIN_FREQ;
        mwCountryDependentValues.maxBandValue = AM_EU_MAX_FREQ;
        mwCountryDependentValues.step = AM_EU_STEP;
        mwCountryDependentValues.ptyStr = "_EU_";

        fmCountryDependentValues.minBandValue = FM_EU_MIN_FREQ;
        fmCountryDependentValues.maxBandValue = FM_EU_MAX_FREQ;
        fmCountryDependentValues.step = FM_EU_STEP;
        fmCountryDependentValues.ptyStr = "_EU_";
    }
}

void DabRadio::SetupInitialStateForDisplayView()
{
    // Setup state machine
    switch (radioPersistentData.band)
    {
        case BAND_DAB3:
            currentSmMachine = smMachineDab;

            // Signal SLS/PTY status is disabled
            smMachineDab->DisableState(STATE_SLS_PTY);

            currentSmMachine->SetCurrentState(STATE_SERVICELIST);

            // If the quality state is enabled start timer and then display it
            if (true == currentSmMachine->CheckEnabledStatus(STATE_QUALITY))
            {
                // Set timer with executed action in order to do closing stuff on timer
                if (RADIO_ACTION_NONE == actionExecuted)
                {
                    DelayTimerSetupAndStart(RADIO_SERVICELIST_ACTION, TIMEOUT_DISPLAY_SLS_AGAIN);
                }
            }

#if 0
            // Set initial view
            DisplayNewView(STATE_SERVICELIST);
#endif // #if 0

            qDebug() << "Initial state for DAB is: " << currentSmMachine->GetCurrentState() << " (expected SERVICE LIST)";
            break;

        case BAND_FM:
        case BAND_AM:
        case BAND_LW:
        case BAND_MW:
        case BAND_SW:
        case BAND_WX:
            currentSmMachine = smMachineFm;

            // Signal SLS/PTY status is enabled
            currentSmMachine->EnableState(STATE_SLS_PTY);

            // Inital state can be quality (if enabled) or SLS/PTY
            if (true == currentSmMachine->CheckEnabledStatus(STATE_QUALITY))
            {
                currentSmMachine->SetCurrentState(STATE_QUALITY);
            }
            else
            {
                currentSmMachine->SetCurrentState(STATE_SLS_PTY);
            }

#if 0
            // Set initial view
            DisplayNewView(STATE_SLS_PTY);
#endif // #if 0

            qDebug() << "Initial state for FM is: " << currentSmMachine->GetCurrentState() << " (expected SERVICE LIST)";
            break;

        case BAND_DRM30:
        case BAND_DRM_PLUS:
        default:
            currentSmMachine = smMachineDab;

            if (false == radioPersistentData.slsDisplayEn)
            {
                // Signal SLS/PTY status is disabled
                currentSmMachine->DisableState(STATE_SLS_PTY);
            }
            else
            {
                // Signal SLS/PTY status is enabled
                currentSmMachine->EnableState(STATE_SLS_PTY);
            }

            currentSmMachine->SetCurrentState(STATE_SERVICELIST);

#if 0
            // Set initial view
            DisplayNewView(STATE_SERVICELIST);
#endif // #if 0

            qDebug() << "Initial state for DRM is: " << currentSmMachine->GetCurrentState() << " (expected SERVICE LIST)";
            break;
    }
}

void DabRadio::SetupWidgetLists()
{
    // Physical buttons list
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_mute));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_voldw));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_volup));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_seekdw));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_seekup));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_list));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_preset));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_src));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_setup));
    physicalButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_onoff));

    // Unused buttons list
    unusedButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_option));
    unusedButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_up));
    unusedButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_audio));
    unusedButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_radio));
    unusedButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_setting));
    unusedButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_minimize));
    unusedButtonsWidgetList.append(dynamic_cast<QPushButton *> (ui->btn_closeAppz));

    // All widgets list
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lstWidget_currPresetList));   // View presets
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lstWidget_currServiceList));  // View service list
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_slsOnXpadImage));         // View SLS/PTY
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->testscreen_widget));          // View quality
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->setup_widget));               // View setup
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_ensFreqNumber));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceBitrate));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->tp_label));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->ta_label));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->stereoOn_label));             // Stereo label defaulted
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_PIid));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceLabel));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_epgLogo));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->ptyLabel));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_answer));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_radioBand));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandBottom));
    viewAllWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandTop));

    // Presets widgets list
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lstWidget_currPresetList));   // View presets
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_ensFreqNumber));
    //  viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceBitrate));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_PIid));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceLabel));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_epgLogo));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->ptyLabel));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_answer));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_radioBand));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandBottom));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandTop));
    viewPresetsWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_slsOnXpadImage)); // Presets view present small SLS/PTY image

    // Service list widgets list
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lstWidget_currServiceList));  // View service list
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_ensFreqNumber));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceBitrate));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_PIid));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceLabel));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_epgLogo));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->ptyLabel));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_answer));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_radioBand));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandBottom));
    viewServiceListWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandTop));

    // SLS/PTY widgets list
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_slsOnXpadImage));         // View SLS/PTY
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_ensFreqNumber));
    //  viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceBitrate));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_PIid));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceLabel));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_epgLogo));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->ptyLabel));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_answer));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_radioBand));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandBottom));
    viewSlsPtyWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandTop));

    // Quality widgets list
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->testscreen_widget));          // View quality
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_ensFreqNumber));
    //  viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceBitrate));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_PIid));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_activeServiceLabel));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_epgLogo));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->ptyLabel));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_answer));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_radioBand));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandBottom));
    viewQualityWidgetList.append(dynamic_cast<QWidget *> (ui->lbl_bandTop));

    // Setup widgets list
    viewSetupWidgetList.append(dynamic_cast<QWidget *> (ui->setup_widget));               // View setup

    // Power off widget list
    viewPowerOffWidgetList.clear();
}

void DabRadio::PowerOnAction()
{
    // Setup initial state for the display view
    SetupInitialStateForDisplayView();

    // Set Radio Panel on
    setPanel(RADIO_POWER_ON_ACTION);

    emit EventFromHmiSignal(EVENT_POWER_UP);

    // Send signal to global list application
    emit GlobalListApplicationProlog_Signal(EVENT_POWER_UP, radioPersistentData.band);
}

void DabRadio::PowerOffAction()
{
    // Block buttons to avoid multi inputs
    ButtonBlockSignals(true);

    // Invalidate data
    isPresetListOn = false;
    lastSlsIsValid = false;

    // Force quality screen to be disabled: we do not want to preserve the status between power cycles
    radioPersistentData.testScreenIsActive = false;
    ui->testScreenEn_checkBox->setCheckState(Qt::Unchecked);

    QString oldState = currentSmMachine->GetCurrentState();
    QString newState = currentSmMachine->CalculateNewState(ACTION_PRESS_ONOFF);

    qDebug() << "On PRESS POWER ON/OFF new state is: " << newState;

    ModifyDisplayView(newState, oldState, ACTION_PRESS_ONOFF);

    // Emit event for power down passing the current settings
    DataContainer data;

    // Send the preset selected by the user
    data.content = QVariant::fromValue(radioPersistentData);

    emit EventFromHmiSignal(EVENT_POWER_DOWN, data);

    // Send signal to global list application
    emit GlobalListApplicationProlog_Signal(EVENT_POWER_DOWN, radioPersistentData.band);
}

void DabRadio::on_btn_list_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    QString oldState = currentSmMachine->GetCurrentState();
    QString newState = currentSmMachine->CalculateNewState(ACTION_PRESS_LIST);

    qDebug() << "On PRESS LIST new state is: " << newState;

    ModifyDisplayView(newState, oldState, ACTION_PRESS_LIST);
}

void DabRadio::on_btn_preset_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    QString oldState = currentSmMachine->GetCurrentState();
    QString newState = currentSmMachine->CalculateNewState(ACTION_PRESS_PRESETS);

    qDebug() << "On PRESS PRESETS new state is: " << newState;

    ModifyDisplayView(newState, oldState, ACTION_PRESS_PRESETS);

    // Signal the current display status
    isPresetListOn = true;
}

bool DabRadio::RenderSlsPtyImage(slsPtyRenderSize size)
{
    // Check if we have a valid SLS image
    if (false == lastSlsIsValid)
    {
        // Invalid image make it invisible anyway
        ui->lbl_slsOnXpadImage->setVisible(false);

        // We do not have a valid image just return
        return false;
    }

    // Determine size
    if (IMAGE_RENDER_SIZE_SMALL == size)
    {
        QPixmap pix = lastSlsImage;

        ui->lbl_slsOnXpadImage->setGeometry(QRect(589, 107, 131, 131));
        QPixmap scaledImage = pix.scaled(ui->lbl_slsOnXpadImage->width(), ui->lbl_slsOnXpadImage->height(),
                                         Qt::KeepAspectRatio, Qt::SmoothTransformation);

        ui->lbl_slsOnXpadImage->setPixmap(scaledImage);
    }
    else if (IMAGE_RENDER_SIZE_NORMAL == size || IMAGE_RENDER_SIZE_BIG == size)
    {
        ui->lbl_slsOnXpadImage->setGeometry(QRect(375, 97, 361, 265));
        ui->lbl_slsOnXpadImage->setPixmap(lastSlsImage);
    }
    else
    {
        // Invalid image make it invisible anyway
        ui->lbl_slsOnXpadImage->setVisible(false);

        // Invalid size, silently return
        return false;
    }

    // Hide Service List
    ui->lstWidget_currServiceList->setVisible(false);

    // Display the image
    ui->lbl_slsOnXpadImage->setVisible(true);
    return true;
}

void DabRadio::on_btn_up_clicked()
{
    // Unused it is a virtual button currently always hidden
}

void DabRadio::on_btn_audio_clicked()
{
    // Unused: it is a virtual button currently always hidden
}

void DabRadio::on_btn_radio_clicked()
{
    // Unused: it is a virtual button currently always hidden
}

void DabRadio::on_btn_setting_clicked()
{
    // Unused: it is a virtual button currently always hidden
}

void DabRadio::on_btn_setup_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    QString oldState = currentSmMachine->GetCurrentState();
    QString newState = currentSmMachine->CalculateNewState(ACTION_PRESS_SETUP);

    qDebug() << "On PRESS SETUP new state is: " << newState;

    ModifyDisplayView(newState, oldState, ACTION_PRESS_SETUP);

    // Check if we are entering or exiting the setup screen
    //if (true == isSetupScreenOn)
    if (0 != newState.compare(STATE_SETUP))
    {
        isSetupScreenOn = false;
    }
    else
    {
        // We entered setup
        isSetupScreenOn = true;
    }
}

void DabRadio::StoreLastStatusData(PersistentData& data)
{
    // Save data in class member variable
    radioPersistentData = data;

    // Save last frequencies
    fmFreqkHz = radioPersistentData.frequencyFm;
    mwFreqkHz = radioPersistentData.frequencyAm;
    dabFreqMHz = radioPersistentData.frequencyDab;

    // Setup correct values for bands
    CalculateCountryDependentValues(radioPersistentData.country);
}

void DabRadio::WorkerAnswerSlot(Events event, StatusCodeTy statusCode, PostalOfficeBase* eventData)
{
    // TODO: use the status code to understand if the command completed successfully
    Q_UNUSED(statusCode);

    switch (event)
    {
        case EVENT_RADIO_LASTSTATUS:
        {
            PostalOffice<PersistentData>* data = dynamic_cast<PostalOffice<PersistentData> *> (eventData);
            PersistentData dataPacket = data->GetPacket();

            // Save the status information
            StoreLastStatusData(dataPacket);

            // Update display
            //displayRadioBand();
            initSlider(radioPersistentData.band);

            // Data has been used, unlock it
            eventData->Unlock();
        }
        break;

        case EVENT_BOOT_ON:
        {
            PostalOffice<bool>* data = dynamic_cast<PostalOffice<bool> *> (eventData);

            bool connection = data->GetPacket();

            if (true == connection)
            {
                bootCompleted = true;

                ui->lbl_on_off->setStyleSheet(QString::fromUtf8("border-image: url(:/icons/on_off_red_ico.png);"));
            }
            else
            {
                bootCompleted = false;

                ui->lbl_answer->setText("Hardware or connection error");
                ui->lbl_answer->setVisible(true);
            }

            // Data has been used, unlock it
            eventData->Unlock();
        }
        break;

        case EVENT_SERVICE_SELECT:
            DisplayActiveService();
            break;

        case EVENT_SOURCE:
        {
            PostalOffice<BandTy>* data = dynamic_cast<PostalOffice<BandTy> *> (eventData);
            BandTy dataPacket = data->GetPacket();
            BandTy tmpBand;

            // Save the band
            tmpBand = dataPacket;

            if (tmpBand == radioPersistentData.band)
            {
                // Update the frequency slider
                displaySelectedBand();
            }

            // Save band information
            radioPersistentData.band = dataPacket;

            AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_STOP);

            // Block slider signals before display redraw
            ui->hFreqSlider->blockSignals(true);

            // Setup state machine
            SetupInitialStateForDisplayView();

            QString oldState = currentSmMachine->GetCurrentState();
            QString newState = currentSmMachine->GetCurrentState();

            qDebug() << "On PRESS SOURCE new state is: " << newState;

            ModifyDisplayView(newState, oldState, nullptr);

            // Signal command ended (re-enable onscreen information like DLS)
            tuneIsOngoing = false;

            // Signals active again
            ui->hFreqSlider->blockSignals(false);

            // Enable buttons again
            ButtonBlockSignals(false);

            // Send signal to global list application
            emit GlobalListApplicationEpilog_Signal(event, radioPersistentData.band);
        }
        break;

        case EVENT_MUTE_TOGGLE:
            // Update graphic
            ui->lbl_audio_off_status->setVisible(radioStatusGlobal->radioStatus.dynamicData.radioMute);

            // Enable buttons again
            ButtonBlockSignals(false);
            break;

        case EVENT_VOLUME_UP:
        case EVENT_VOLUME_DOWN:
        {
            qDebug() << "Button volume pressed";

            QString tmpStr = ui->lbl_ensFreqNumber->text();

            // We do not want to save a volume label but the frequency one
            if (false == tmpStr.contains("Volume"))
            {
                // Save the current label in order to display it again at timer timeout
                lastLabelDisplayed = ui->lbl_ensFreqNumber->text();
            }

            // Set volume label
            if (VOLUME_MAX_VALUE == radioStatusGlobal->radioStatus.persistentData.radioVolume)
            {
                ui->lbl_ensFreqNumber->setText("Volume MAX");
            }
            else
            {
                ui->lbl_ensFreqNumber->setText("Volume " + QString::number(radioStatusGlobal->radioStatus.persistentData.radioVolume, 10));
            }

            // Set timer with executed action in order to do closing stuff on timer
            if ((RADIO_ACTION_NONE == actionExecuted) || (RADIO_ACTION_VOLUME == actionExecuted))
            {
                DelayTimerSetupAndStart(RADIO_ACTION_VOLUME, PRESET_LONG_DURATION_MS);
            }

            // Enable buttons again
            ButtonBlockSignals(false);
        }
        break;

        case EVENT_POWER_DOWN:
            // Set radio status to off
            radioIsOn = false;

            qDebug() << "Power off executed";
            break;

        case EVENT_POWER_UP:
            // Signal radio is on (do as first thing to let below operations to act
            // with information that radio is on)
            qDebug() << "Power on executed";

            radioIsOn = true;

            if ((BAND_FM == radioPersistentData.band) || (BAND_AM == radioPersistentData.band))
            {
                // Force Good Signal indication
                ui->lbl_answer->clear();
            }

            // AnimationsMachine(LABEL_WAIT_ANIMATION, ANIMATION_STOP);

            // Stop timer
            delayActionTimer->stop();

            // Clear executed action
            actionExecuted = RADIO_ACTION_NONE;

            // Enable button again
            ui->btn_onoff->blockSignals(false);

            // Stop animations
            AnimationsStopOnEventResponse(event);

            // Update sync quality
            //updateSyncQuality();

            // Enable buttons again
            ButtonBlockSignals(false);

            // Send signal to global list application
            emit GlobalListApplicationEpilog_Signal(event, radioPersistentData.band);
            break;

        case EVENT_FREQUENCY_CHANGE:
            qDebug() << "Frequency tuned";

            // Stop animations
            AnimationsStopOnEventResponse(event);

            // Signal command ended (re-enable onscreen information like DLS)
            tuneIsOngoing = false;

            // Enable buttons again
            ButtonBlockSignals(false);

            // Send signal to global list application
            emit GlobalListApplicationEpilog_Signal(event, radioPersistentData.band);
            break;

        case EVENT_PRESET_SAVE:
        {
            // Emit beep
            QSound::play(QString(":/beep.wav"));

            // Save preset
            int presetToReplace = radioStatusGlobal->panelData.currPresetIndex;
            QString serviceLabel = ui->lbl_activeServiceLabel->text();
            QString serviceId = "0000" + QString::number(radioStatusGlobal->radioStatus.persistentData.serviceId, 16);

            qDebug() << "string serviceId = " << serviceId;

            presetList->SetPreset(presetToReplace,
                                  serviceLabel.toStdString().c_str(),
                                  radioStatusGlobal->radioStatus.persistentData.serviceId,
                                  radioStatusGlobal->radioStatus.persistentData.frequency,
                                  radioPersistentData.band);

            ui->lstWidget_currPresetList->item(presetToReplace)->setText(serviceLabel);
            ui->lstWidget_currPresetList->item(presetToReplace)->setIcon(QIcon(getRadioLogo(serviceId)));

            // Set timer with executed action in order to do closing stuff on timer
            delayActionTimer->start(DELAY_ACTION_MS);

            // Signal command ended (re-enable onscreen information like DLS)
            tuneIsOngoing = false;

            // Enable buttons again
            ButtonBlockSignals(false);
        }
        break;

        case EVENT_PRESET_SELECT:
        {
            // Preset has been selected
            ui->hFreqSlider->blockSignals(true);

            QString oldState = currentSmMachine->GetCurrentState();

            // We had a preset selected, we need to act as the band is changed
            // Setup state machine
            SetupInitialStateForDisplayView();

            QString newState = currentSmMachine->GetCurrentState();

            qDebug() << "On SELECT PRESET new state is: " << newState << " - From: " << oldState;

            ModifyDisplayView(newState, oldState, nullptr);

            // Get the frequency
            unsigned int frequency = radioStatusGlobal->radioStatus.persistentData.frequency;

            if (BAND_FM == radioPersistentData.band)
            {
                int currentStep = (frequency - fmCountryDependentValues.minBandValue) / fmCountryDependentValues.step;

                ui->hFreqSlider->setValue(currentStep);

                ui->lbl_activeServiceLabel->setText(radioStatusGlobal->radioStatus.persistentData.serviceName);

                // Show the radio logo
                DisplayRadioLogo("0000" + QString::number(radioStatusGlobal->radioStatus.persistentData.serviceId, 16));

                ui->lbl_PIid->setText(QString::number(radioStatusGlobal->radioStatus.persistentData.serviceId, 16));

                ui->lbl_tuneFreq->setText(fmFreq2String(frequency));
            }
            else if (BAND_DAB3 == radioPersistentData.band)
            {
                int tmpIndex = getDab3EnsembleIndex(frequency);

                radioStatusGlobal->panelData.ensembleIndex = tmpIndex;

                if (ui->hFreqSlider->value() != tmpIndex)
                {
                    ui->hFreqSlider->setValue(tmpIndex);

                    ui->lbl_ensFreqNumber->setText(getDab3Channel(tmpIndex));

                    lastSlsIsValid = false;

                    ui->lstWidget_currServiceList->clear();
                }

                // For a while display service list then on a timer the quality again
                if ((0 == newState.compare(STATE_SERVICELIST)) ||
                    (0 == newState.compare(STATE_QUALITY)))
                {
                    // Load service list on screen
                    LoadServicesList(true);

                    if (0 == newState.compare(STATE_QUALITY))
                    {
                        // Make the test screen invisible
                        ui->testscreen_widget->setVisible(false);

                        // Now we need to apply a little trick: if the test screen is left enable we would like to return
                        // back to quality after having displayed the list for a few seconds
                        DelayTimerSetupAndStart(RADIO_DISPLAY_TESTSCREEN, DELAY_ACTION_4_SEC);
                    }
                }

                ui->lbl_tuneFreq->setText(dabFreq2String(frequency));
            }
            else if (BAND_AM == radioPersistentData.band)
            {
                // Nothing to be done
                ui->lbl_tuneFreq->setText(QString::number(frequency, 10) + "kHz");
            }

            // Force Good Signal indication
            ui->lbl_answer->clear();

            ui->hFreqSlider->blockSignals(false);

            // Set timer with executed action in order to do closing stuff on timer
            delayActionTimer->start(DELAY_ACTION_MS);

            // Signal command ended (re-enable onscreen information like DLS)
            tuneIsOngoing = false;

            // Enable buttons again
            ButtonBlockSignals(false);
        }
        break;

        case EVENT_PRESET_CLEAR:
            ui->btn_clearPresets->setEnabled(false);

            // Emit beep
            QSound::play(QString(":/beep.wav"));
            break;

        case EVENT_SEEK_UP:
        case EVENT_SEEK_DOWN:
            // Stop animations
            AnimationsStopOnEventResponse(event);

            SetCurrSeekModeStatus(NO_SEEK_MODE);

            ui->lstWidget_currPresetList->setVisible(false);

            // Block slider signals before display redraw
            ui->hFreqSlider->blockSignals(true);

            // Update local Frequency variable
            if (BAND_FM == radioPersistentData.band)
            {
                // If testscreen checkbox is enabled, then show immediately Quality screen
                if (true == ui->testScreenEn_checkBox->isChecked())
                {
                    ui->testscreen_widget->setVisible(true);
                }
            }
            else if (BAND_AM == radioPersistentData.band)
            {
                // If testscreen checkbox is enabled, then show immediately Quality screen
                if (true == ui->testScreenEn_checkBox->isChecked())
                {
                    ui->testscreen_widget->setVisible(true);
                }
            }

            // Signals active again
            ui->hFreqSlider->blockSignals(false);

            // Signal command ended (re-enable onscreen information like DLS)
            tuneIsOngoing = false;

            // Enable buttons again
            ButtonBlockSignals(false);
            break;

        default:
            // No code
            break;
    }
}

void DabRadio::updateSyncQuality(bool isGoodLevel, unsigned int qualityLevel, unsigned int signalStrength, bool force)
{
    unsigned int qualVal = 0;

    // Radio is ON: we need to check quality
    if (BAND_DAB3 == radioPersistentData.band)
    {
        if (true == isGoodLevel || true == force)
        {
            // Stop the timer
            if (true == signalIndicationTimer->isActive())
            {
                signalIndicationTimer->stop();
            }

            // Display good signal indication
            signalTextToDisplay.clear();

            // Clear answer label
            ui->lbl_answer->clear();
        }
        else
        {
            // Check if we are in seek mode
            if (NO_SEEK_MODE == GetCurrSeekModeStatus())
            {
                // Display no signal information
                signalTextToDisplay = "No signal";

                if (false == signalIndicationTimer->isActive())
                {
                    signalIndicationTimer->start();
                }
            }
            else
            {
                // Stop the timer
                if (true == signalIndicationTimer->isActive())
                {
                    signalIndicationTimer->stop();
                }

                // In seek mode clear information
                signalTextToDisplay.clear();

                ui->lbl_answer->clear();
            }
        }
    }

    if (BAND_DAB3 == radioPersistentData.band)
    {
        qualVal = qualityLevel;
    }
    else if ((BAND_FM == radioPersistentData.band) ||
             (BAND_AM == radioPersistentData.band))
    {
        if (signalStrength & 0x80)
        {
            // If field strength < 0
            qualVal = BER_NOT_AVAILABLE;
        }
        else
        {
            // SNR got with CIS command 0x12 is limited to min about 60% !!
            // Convert FM Quality SNR (range 0 to 100) in FIC_BER DAB equivalent
            if (qualityLevel < 20)
            {
                qualVal = BER_NOT_AVAILABLE;
            }
            else if (qualityLevel < 40)
            {
                qualVal = BER_GE_5E_1;
            }
            else if (qualityLevel < 60)
            {
                qualVal = BER_LESS_5E_1;
            }
            else if (qualityLevel < 80)
            {
                qualVal = BER_LESS_5E_2;
            }
            else
            {
                qualVal = BER_LESS_5E_4;
            }
        }
    }

    // Update signal strenght icon
    SetSignalStrengthIcon(true, qualVal);
}

void DabRadio::WorkerEventFromDeviceSlot(PostalOfficeBase* eventData)
{
    PostalType what = eventData->GetType();

    // If we reach this point before the dabRadio is fully started we could have
    // not initialized values so we check and un case just return
    if (nullptr == currentSmMachine)
    {
        return;
    }

    // Check if we are in power off state
    QString currentState = currentSmMachine->GetCurrentState();

    if (0 != currentState.compare(STATE_OFF))
    {
        // We need to process the tune because it happens before we have the response to
        // power on that is signalled by Worker after the tune has been done
        if (true == radioIsOn || POSTAL_TYPE_IS_FREQ == what)
        {
            switch (what)
            {
                case POSTAL_TYPE_IS_DLS:
                {
                    PostalOffice<QString>* data = dynamic_cast<PostalOffice<QString> *> (eventData);

                    QString dlsText = data->GetPacket();

                    dlsText = dlsText.simplified(); // Remove leading and trailing spaces

                    if (true == DisplayReceivedInformation())
                    {
                        //qDebug() << "DLS received & displayed: " << dlsText;

                        ui->padDls_Label->setVisible(true);
                        ui->padDls_Label->setText(dlsText);
                    }
                    else
                    {
                        qDebug() << "DLS received: " << dlsText;
                    }
                }
                break;

                case POSTAL_TYPE_IS_SLS:
                {
                    QPixmap image;
                    PostalOffice<QByteArray>* data = dynamic_cast<PostalOffice<QByteArray> *> (eventData);

                    //qDebug() << "SLS received";

                    // Get data
                    if (image.loadFromData(data->GetPacket()))
                    {
                        // We display the information only if a tune command is ongoing
                        if (true == DisplayReceivedInformation())
                        {
                            // We have a valid image let's save information about it
                            lastSlsImage = image;
                            lastSlsIsValid = true;

                            if (true == radioPersistentData.slsDisplayEn)
                            {
                                // Signal SLS/PTY status is enabled
                                currentSmMachine->EnableState(STATE_SLS_PTY);
                            }

                            QString oldState = currentSmMachine->GetCurrentState();
                            QString newState = currentSmMachine->CalculateNewState(ACTION_NEW_SLS);

                            qDebug() << "On NEW SLS new state is: " << newState;

                            ModifyDisplayView(newState, oldState, ACTION_NEW_SLS);
                        }
                    }
                }
                break;

                case POSTAL_TYPE_IS_SYNC_LEVEL:
                {
                    PostalOffice<qualityInfoTy>* data = dynamic_cast<PostalOffice<qualityInfoTy> *> (eventData);
                    qualityInfoTy qualInfo;

                    qualInfo = data->GetPacket();

                    radioStatusGlobal->panelData.isGoodLevel = qualInfo.sync;

                    radioStatusGlobal->radioStatus.dynamicData.qualityLevel = qualInfo.qualFicBer;

                    // Update quality information
                    if (0 == currentSmMachine->GetCurrentState().compare(STATE_QUALITY))
                    {
                        updateQualityTestScreen(qualInfo);
                    }

                    if (true == DisplayReceivedInformation())
                    {
                        updateSyncQuality(qualInfo.sync, qualInfo.qualFicBer, radioStatusGlobal->radioStatus.dynamicData.signalStrength);
                    }
                }
                break;

                case POSTAL_TYPE_IS_QUALITY_LEVEL:
                {
                    PostalOffice<qualityInfoTy>* data = dynamic_cast<PostalOffice<qualityInfoTy> *> (eventData);
                    qualityInfoTy qualInfo;

                    if (NULL != data)
                    {
                        qualInfo = data->GetPacket();

                        radioStatusGlobal->panelData.isGoodLevel = qualInfo.sync;

                        if (BAND_DAB3 == radioPersistentData.band)
                        {
                            radioStatusGlobal->radioStatus.dynamicData.qualityLevel = qualInfo.qualFicBer;

                            // On SYNC event we need to update the information on screen (only if we are not in a tune)
                            if (true == DisplayReceivedInformation())
                            {
                                updateSyncQuality(qualInfo.sync, qualInfo.qualFicBer, radioStatusGlobal->radioStatus.dynamicData.signalStrength);
                            }
                        }
                        else
                        {
                            // Save SNR FM quality parameter range 0 to 100 value %
                            radioStatusGlobal->radioStatus.dynamicData.qualityLevel = (quint8)qualInfo.fmQualityInfo.qualSNR;

                            radioStatusGlobal->radioStatus.dynamicData.signalStrength = qualInfo.qualFstRf;

                            updateStereoFlag(qualInfo.fmQualityInfo.qualStereo);

                            // On SYNC event we need to update the information on screen (only if we are not in a tune)
                            if (true == DisplayReceivedInformation())
                            {
                                // In FM first parameter not used, forced = TRUE
                                updateSyncQuality(true, qualInfo.fmQualityInfo.qualSNR, qualInfo.qualFstRf);
                            }
                        }

                        // Update quality information
                        if (0 == currentSmMachine->GetCurrentState().compare(STATE_QUALITY))
                        {
                            updateQualityTestScreen(qualInfo);
                        }
                    }
                }
                break;

                case POSTAL_TYPE_IS_FREQ:
                {
                    PostalOffice<RadioTuneAnswer>* data = dynamic_cast<PostalOffice<RadioTuneAnswer> *> (eventData);
                    RadioTuneAnswer tuneAnswer = data->GetPacket();
                    unsigned int frequency = tuneAnswer.freq;
                    radioPersistentData.band = tuneAnswer.band;

                    ui->hFreqSlider->blockSignals(true);

                    if (BAND_AM == radioPersistentData.band)
                    {
                        mwFreqkHz = frequency;
                    }
                    else if (BAND_FM == radioPersistentData.band)
                    {
                        fmFreqkHz = frequency;
                    }
                    else if (BAND_DAB3 == radioPersistentData.band)
                    {
                        dabFreqMHz = frequency;

                        // Let the "No signal" timer to start so in case we do not have good signal we
                        // are able to display the indication
                        signalTextToDisplay = "No signal";

                        if (false == signalIndicationTimer->isActive())
                        {
                            signalIndicationTimer->start();
                        }
                    }

                    // Update display information
                    displaySelectedBand();

                    ui->hFreqSlider->blockSignals(false);
                }
                break;

                case POSTAL_TYPE_IS_ENSEMBLE_NAME:
                {
                    PostalOffice<EnsembleTableTy>* data = dynamic_cast<PostalOffice<EnsembleTableTy> *> (eventData);

                    radioStatusGlobal->panelData.ensembleTable = data->GetPacket();

                    if (true == DisplayReceivedInformation())
                    {
                        ui->lbl_ensLabel->setVisible(true);
                        ui->lbl_ensLabel->setText(radioStatusGlobal->panelData.ensembleTable.EnsChLabel);
                    }

                    // On the tune frequency the "No signal" information is started and the quality could be
                    // disabled so we would like to stop this information to appear if we get a valid ensemble name
                    if (true == signalIndicationTimer->isActive())
                    {
                        signalIndicationTimer->stop();
                    }

                    // Display good signal indication
                    signalTextToDisplay.clear();

                    // Clear answer label
                    ui->lbl_answer->clear();

                    qDebug() << "Ensemble Name: " << radioStatusGlobal->panelData.ensembleTable.EnsChLabel;
                }
                break;

                case POSTAL_TYPE_IS_SERVICE_LIST:
                {
                    PostalOffice<ServiceListTy>* data = dynamic_cast<PostalOffice<ServiceListTy> *> (eventData);

                    radioStatusGlobal->panelData.list = data->GetPacket();

                    //                    ServiceTy service;
                    //                    service.frequency = 199999;
                    //                    service.ServiceBitrate = 200;
                    //                    service.ServiceCharset = 0;
                    //                    service.ServiceID = QString("0x232f");
                    //                    service.ServiceLabel = "Vltavin";
                    //                    service.serviceUniqueId = 0x232f;

                    //                    radioStatusGlobal->panelData.list.serviceList.append(service);

                    //                    service.frequency = 199999;
                    //                    service.ServiceBitrate = 201;
                    //                    service.ServiceCharset = 0;
                    //                    service.ServiceID = QString("0x232d");
                    //                    service.ServiceLabel = "Hermelin";
                    //                    service.serviceUniqueId = 0x232d;

                    //                    radioStatusGlobal->panelData.list.serviceList.append(service);

                    //                    service.frequency = 199999;
                    //                    service.ServiceBitrate = 202;
                    //                    service.ServiceCharset = 0;
                    //                    service.ServiceID = QString("0x232e");
                    //                    service.ServiceLabel = "Romadur";
                    //                    service.serviceUniqueId = 0x232e;

                    //                    radioStatusGlobal->panelData.list.serviceList.append(service);

#if 0
                    qDebug() << "= " << radioStatusGlobal->panelData.ensembleTable.EnsChLabel << " =";

                    for (int cnt = 0; cnt < radioStatusGlobal->panelData.ensembleTable.ServicesTableList.size(); cnt++)
                    {
                        ServiceTy service = radioStatusGlobal->panelData.ensembleTable.ServicesTableList.at(cnt);

                        qDebug() << "=== " << service.ServiceLabel << " (" << service.ServiceID << ")" << " ===";
                    }
#endif // #if 0

                    // Protect the display of information: only if the radio
                    if (true == DisplayReceivedInformation())
                    {
                        // TODO: complete
                        QString tmpCurrentState = currentSmMachine->GetCurrentState();

                        if ((0 == tmpCurrentState.compare(STATE_SERVICELIST)) ||
                            (0 == tmpCurrentState.compare(STATE_QUALITY)))
                        {
                            // Load service list on screen
                            LoadServicesList(true);

                            if (0 == tmpCurrentState.compare(STATE_QUALITY))
                            {
                                // Make the test screen invisible
                                ui->testscreen_widget->setVisible(false);

                                // Now we need to apply a little trick: if the test screen is left enable we would like to return
                                // back to quality after having displayed the list for a few seconds
                                DelayTimerSetupAndStart(RADIO_DISPLAY_TESTSCREEN, DELAY_ACTION_4_SEC);
                            }
                        }
                    }
                }
                break;

                case POSTAL_TYPE_IS_SERVICE_NAME:
                {
                    PostalOffice<QString>* data = dynamic_cast<PostalOffice<QString> *> (eventData);

                    qDebug() << "Service Name: " << data->GetPacket().trimmed();

                    if (true == DisplayReceivedInformation())
                    {
                        DisplayActiveService();
                    }
                }
                break;

                case POSTAL_TYPE_IS_RDS:
                {
                    PostalOffice<RdsDataSetTableTy>* data = dynamic_cast<PostalOffice<RdsDataSetTableTy> *> (eventData);

                    if (true == DisplayReceivedInformation())
                    {
                        rdsPanelUpdateData(data->GetPacket());
                    }
                }
                break;

                case POSTAL_TYPE_IS_AUDIO_PLAYS:
                {
                    PostalOffice<bool>* data = dynamic_cast<PostalOffice<bool> *> (eventData);

                    bool audioIsPlaying = data->GetPacket();

                    if (true == audioIsPlaying)
                    {
                        qDebug() << "Audio is playing";
                    }

                    // We stop the service tracking information
                    BlinkingText(false);
                }
                break;

                default:
                    // No code
                    break;
            }
        }
    }

    // Data has been used, unlock it
    eventData->Unlock();
}

void  DabRadio::SetRdsAfDisplay(bool active)
{
    if (BAND_FM == radioPersistentData.band)
    {
        ui->rdsAfList_label->setVisible(active);
        ui->hexAf_checkBox->setVisible(active);
    }
    else
    {
        ui->rdsAfList_label->setVisible(false);
        ui->hexAf_checkBox->setVisible(false);
    }
}

void DabRadio::SetTestScreenMode(bool enable)
{
    // Setup RDS/AF fields
    SetRdsAfDisplay(enable);

    // Enable or disable quality measurements
    if (true == enable)
    {
        // Activate QualitySNR read
        emit EventFromHmiSignal(EVENT_QUALITY_ENABLE);
    }
    else
    {
        // Stop Quality SNR read
        emit EventFromHmiSignal(EVENT_QUALITY_DISABLE);
    }
}

void DabRadio::ModifyDisplayView(QString newState, QString oldState, QString action)
{
    QList<QWidget *> listToUse;
    QList<QWidget *>::iterator iter;

    // Actions for the old state
    if (nullptr != oldState)
    {
        if (0 == oldState.compare(STATE_SERVICELIST))
        {
            listToUse = viewServiceListWidgetList;
        }
        else if (0 == oldState.compare(STATE_PRESETS))
        {
            listToUse = viewPresetsWidgetList;
        }
        else if (0 == oldState.compare(STATE_SLS_PTY))
        {
            listToUse = viewSlsPtyWidgetList;
        }
        else if (0 == oldState.compare(STATE_QUALITY))
        {
            listToUse = viewQualityWidgetList;
        }
        else if (0 == oldState.compare(STATE_SETUP))
        {
            listToUse = viewSetupWidgetList;
        }

        for (iter = listToUse.begin(); iter != listToUse.end(); ++iter)
        {
            QWidget* tmpWidget = *iter;

            tmpWidget->setVisible(false);
        }
    }

    // Actions for the new state
    if (nullptr != newState)
    {
        if (0 == newState.compare(STATE_SERVICELIST))
        {
            listToUse = viewServiceListWidgetList;
        }
        else if (0 == newState.compare(STATE_PRESETS))
        {
            listToUse = viewPresetsWidgetList;

            // Load presets list
            LoadPresetsList();

            // Display small image
            RenderSlsPtyImage(IMAGE_RENDER_SIZE_SMALL);
        }
        else if (0 == newState.compare(STATE_SLS_PTY))
        {
            listToUse = viewSlsPtyWidgetList;

            // Render SLS/PTY image
            RenderSlsPtyImage(IMAGE_RENDER_SIZE_NORMAL);
        }
        else if (0 == newState.compare(STATE_QUALITY))
        {
            listToUse = viewQualityWidgetList;

            // Display or not element depending on FM/DAB
            SetRdsAfDisplay(true);
        }
        else if (0 == newState.compare(STATE_SETUP))
        {
            listToUse = viewSetupWidgetList;

            // Display setup
            setPanel(RADIO_SETUP_ACTION);
        }
        else if (0 == newState.compare(STATE_OFF))
        {
            listToUse = viewPowerOffWidgetList;

            // Set Radio Panel off
            setPanel(RADIO_POWER_OFF_ACTION);
        }

        if (false == listToUse.isEmpty())
        {
            for (iter = listToUse.begin(); iter != listToUse.end(); ++iter)
            {
                QWidget* tmpWidget = *iter;

                tmpWidget->setVisible(true);
            }
        }
    }

    // Modifications due to the action
    if (nullptr != action)
    {
        if (0 == action.compare(ACTION_TUNE))
        {
            // Set radio panel
            setPanel(RADIO_TUNE_FREQUENCY_ACTION);
        }
    }
}

void DabRadio::OnScreenPressViewUpdate()
{
    QString oldState = currentSmMachine->GetCurrentState();
    QString newState = currentSmMachine->CalculateNewState(ACTION_PRESS_SCREEN);

    qDebug() << "On PRESS SCREEN new state is: " << newState;

    ModifyDisplayView(newState, oldState, ACTION_PRESS_SCREEN);

    // Set timer with executed action in order to do closing stuff on timer
    if (0 == oldState.compare(STATE_SLS_PTY))
    {
        if (RADIO_ACTION_NONE == actionExecuted)
        {
            DelayTimerSetupAndStart(RADIO_SERVICELIST_ACTION, TIMEOUT_DISPLAY_SLS_AGAIN);
        }
    }
}

bool DabRadio::DisplayReceivedInformation()
{
    // If we have a tune ongoing the screen shall not update some field
    if (false == tuneIsOngoing)
    {
        return true;
    }

    return false;
}

void DabRadio::DisplayRadioLogo(QString curServiceId)
{
    bool found = false;
    QStringList nameFilter;

    nameFilter << "*.png" << "*.jpg" << "*.gif";

    QDir directory("./radio_logos");
    QFileInfoList list = directory.entryInfoList(nameFilter, QDir::Files);
    QListIterator<QFileInfo> it(list);

    curServiceId = curServiceId.mid(4, 4); // We use logos only for audio services so we can analyze
    QString serviceIdNoSpaces = curServiceId.remove(QChar(' '), Qt::CaseInsensitive);

    while (it.hasNext())
    {
        QFileInfo fileInfo = it.next();
        QString tmpStr = fileInfo.fileName();

        if (true == tmpStr.contains(serviceIdNoSpaces, Qt::CaseInsensitive))
        {
            QPixmap pix(fileInfo.absoluteFilePath());
            currentRadioLogo = fileInfo.absoluteFilePath();
            QPixmap scaled = pix.scaled(ui->lbl_epgLogo->width(), ui->lbl_epgLogo->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

            ui->lbl_epgLogo->setPixmap(scaled);

            found = true;

            break;
        }
    }

    if (false == found)
    {
        if (BAND_FM == radioPersistentData.band)
        {
            QPixmap pix("./radio_logos/0000.fmStation.png");
            QPixmap scaled = pix.scaled(ui->lbl_epgLogo->width(), ui->lbl_epgLogo->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->lbl_epgLogo->setPixmap(scaled);
        }
        else
        {
            ui->lbl_epgLogo->clear();
        }
    }
}

void DabRadio::Display_SlsPtyImage(quint8 ptyValue)
{
    QStringList nameFilter;
    QString ptyId;

    nameFilter << "*.png" << "*.jpg" << "*.gif";

    QDir directory("./image_sls");
    QFileInfoList list = directory.entryInfoList(nameFilter, QDir::Files);
    QListIterator<QFileInfo> it(list);

    if (ptyValue < 10)
    {
        ptyId = "pty0" + QString::number(ptyValue, 10) + mwCountryDependentValues.ptyStr;
    }
    else
    {
        ptyId = "pty" + QString::number(ptyValue, 10) + mwCountryDependentValues.ptyStr;
    }

    while (it.hasNext())
    {
        QFileInfo fileInfo = it.next();
        QString tmpStr = fileInfo.fileName();

        if (true == tmpStr.contains(ptyId, Qt::CaseInsensitive))
        {
            QPixmap pix(fileInfo.absoluteFilePath());

            QPixmap scaled = pix.scaled(361, 265, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            ui->lbl_slsOnXpadImage->setPixmap(scaled);

            // We have a valid image set the flag and store the image
            lastSlsImage = scaled;
            lastSlsIsValid = true;

            // Check how big the image shall be displayed
            if (true == ui->lstWidget_currPresetList->isVisible())
            {
                RenderSlsPtyImage(IMAGE_RENDER_SIZE_SMALL);
            }
            else
            {
                RenderSlsPtyImage(IMAGE_RENDER_SIZE_NORMAL);
            }

            break;
        }
    }
}

QString DabRadio::DisplayDefaultPSname(QString psFileName, QString serviceId)
{
    bool found = false;
    QString retPs;
    QFile FileSorgPs;
    QString str_ps;
    bool ok;

    if (serviceId.toUInt(&ok, 16) == 0)
    {
        retPs.clear();
        return retPs;
    }

    serviceId = serviceId.mid(4, 4); // We use logos only for audio services so we can analyze
    QString serviceIdNoSpaces = serviceId.remove(QChar(' '), Qt::CaseInsensitive);

    FileSorgPs.setFileName(psFileName);

    // Open file with list of Default PS names to find  the one belonging to passed PI
    FileSorgPs.open(QIODevice::ReadOnly);

    do
    {
        // Read line by line
        str_ps = FileSorgPs.readLine();

        if (true == str_ps.contains(serviceIdNoSpaces, Qt::CaseInsensitive))
        {
            retPs = str_ps.right(str_ps.length() - 5);

            found = true;
        }
    }
    while (!FileSorgPs.atEnd() && (false == found));

    FileSorgPs.close();

    return retPs;
}

void DabRadio::ServiceListOnScreen(int currentIndex)
{
    // Move "cursor" to currently selected service
    ui->lstWidget_currServiceList->item(currentIndex)->setSelected(true);

    // TODO: implement in order to have currently selected item always in the center
    //       and to make it selectable with the wheel
#if 0
    QStringList displayedList;

    // We enter here with alphabetical ordered list
    // We need to place selected item at the screen center

    // Get number of elements
    int numberOfElements = onScreenServiceList.count();

    if (numberOfElements)
    {
        // Move "cursor" to currently selected service
        //ui->lstWidget_currServiceList->item(currentServiceIndex)->setSelected(true);
        onScreenServiceList.at(currentIndex);
    }
#endif // #if 0
}

void DabRadio::DisplayActiveService()
{
    ServiceTy activeService;
    int elementOnScreen = 0;
    unsigned int currentServiceId;
    int currentServiceIndex = 0;

    // Get the currently selected item
    currentServiceId = radioStatusGlobal->radioStatus.persistentData.serviceId;

    foreach(auto service, onScreenServiceDatabase)
    {
        if (currentServiceId == service.serviceUniqueId)
        {
            // Set active service
            activeService = service;

            break;
        }
    }

    foreach(auto serviceName, onScreenServiceList)
    {
        if (0 == serviceName.compare(activeService.ServiceLabel))
        {
            break;
        }
        else
        {
            // Find current index
            currentServiceIndex++;
        }
    }

    // We need to be sure we have the selected element avilable
    if (ui->lstWidget_currServiceList->count() > currentServiceIndex)
    {
        ServiceListOnScreen(currentServiceIndex);

        QScrollBar* verticalScrollBar = ui->lstWidget_currServiceList->verticalScrollBar();

        // We have services on screen: in case service unavailable information is present remove it
        signalTextToDisplay.clear();

        // Force immediate clear of the label
        ui->lbl_answer->clear();

        // Move "cursor" to currently selected service
        //ui->lstWidget_currServiceList->item(currentServiceIndex)->setSelected(true);

        // Show the service label
        ui->lbl_activeServiceLabel->setText(activeService.ServiceLabel);
        ui->lbl_activeServiceLabel->setVisible(true);

        // Show the PTY name label  (no display PTY = 0 = "None")
        if (activeService.ServicePty > 0)
        {
            ui->ptyLabel->setText(GetCurrentPtyLabel(activeService.ServicePty));
            ui->ptyLabel->setVisible(true);
        }

        // Show the radio logo
        DisplayRadioLogo(activeService.ServiceID);

        // If the current item is not displayed then we need to bring it into visibility (this is true
        // at startup when the service is not selected by the user but recovered by saved ones)
        int currentSliderPos = verticalScrollBar->value();

        if (currentServiceIndex > currentSliderPos + SERVICE_LIST_NUMBER_OF_DISPLAYED_ELEMENTS)
        {
            int newValue = currentServiceIndex - SERVICE_LIST_NUMBER_OF_DISPLAYED_ELEMENTS + 1;

            verticalScrollBar->setValue(newValue);
        }

        ui->lbl_PIid->setText(activeService.ServiceID.right(4));

        // Display active service bitrate
        if (INVALID_DATA_U16 != activeService.ServiceBitrate)
        {
            ui->lbl_activeServiceBitrate->setText(QString::number(activeService.ServiceBitrate, 10) + " Kbps");
            ui->lbl_activeServiceBitrate->setVisible(true);
        }
    }
    else
    {
        //Debug information
        qDebug() << "No enough items yet in the graphic Service List (or selected service has a higher index than available services)";
        qDebug() << "Elements on screen: " << elementOnScreen << " - Service index: " << currentServiceIndex;

        // Display service unavailable only if the signal quality is good, otherwise let "No signal" to be displayed
        if (true == radioStatusGlobal->panelData.isGoodLevel)
        {
            // Clear the screen info
            ui->lbl_activeServiceLabel->setVisible(true);

            if (radioPersistentData.band == BAND_DAB3)
            {
                // Clear the screen info
                ui->lbl_activeServiceLabel->clear();

                // Display the information for the user that no service is available
                signalTextToDisplay = "Service unavailable";

                signalIndicationTimer->start();
            }
        }
    }
}

QString DabRadio::fmFreq2String(quint32 cFreq)
{
    QString freqMHz = QString::number(((float)cFreq / (float)1000), 'f', 1) + " MHz";

    return freqMHz;
}

QString DabRadio::dabFreq2String(quint32 cFreq)
{
    QString ftext;

    return ftext.sprintf("%3d.%.3d MHz", (cFreq / 1000), (cFreq % 1000));
}

void DabRadio::PanelStartUp()
{
    bootCompleted = false;

    emit EventFromHmiSignal(EVENT_RADIO_LASTSTATUS);

    emit EventFromHmiSignal(EVENT_BOOT_ON);
}

QString DabRadio::getDab3Channel(int index)
{
    QString dabChName;

    QList<QString> channelList = dab::dabEuChannelList;

    if (INVALID_SDATA == index)
    {
        return "NA";
    }

    channelList << "0";

    if (index < channelList.size() && index >= 0)
    {
        dabChName = channelList[index];
    }
    else
    {
        dabChName = "NA";
    }

    return dabChName;
}

quint32 DabRadio::getDab3FrequencyVal(int index)
{
    quint32 dabFreqVal;

    QList<quint32> Dab3FrequencyList = dab::dabEuFreqList;

    // Add a terminator
    Dab3FrequencyList << 0;

    if (index < Dab3FrequencyList.size() && index >= 0)
    {
        dabFreqVal = Dab3FrequencyList[index];
    }
    else
    {
        dabFreqVal = 0;
    }

    return dabFreqVal;
}

void DabRadio::CalculateNextDabFrequency(quint32& dabFreqVal)
{
    int len = dab::dabEuFreqList.size();
    int loopCnt = 0;

    if (dabFreqVal == dab::dabEuFreqList.at(len - 1))
    {
        dabFreqVal = dab::dabEuFreqList.at(0);
    }
    else
    {
        foreach(auto freq, dab::dabEuFreqList)
        {
            if (freq == dabFreqVal && loopCnt < len)
            {
                dabFreqVal = dab::dabEuFreqList.at(loopCnt + 1);

                break;
            }

            loopCnt++;
        }
    }
}

void DabRadio::CalculatePrevDabFrequency(quint32& dabFreqVal)
{
    int len = dab::dabEuFreqList.size();
    int loopCnt = 0;

    if (dabFreqVal == dab::dabEuFreqList.at(0))
    {
        dabFreqVal = dab::dabEuFreqList.at(len - 1);
    }
    else
    {
        foreach(auto freq, dab::dabEuFreqList)
        {
            if (freq == dabFreqVal && loopCnt > 0)
            {
                dabFreqVal = dab::dabEuFreqList.at(loopCnt - 1);

                break;
            }

            loopCnt++;
        }
    }
}

int DabRadio::getDab3EnsembleIndex(quint32 dabFreqVal)
{
    int ensIdx = INVALID_SDATA;

    QList<quint32> Dab3FrequencyList = dab::dabEuFreqList;

    // Add a terminator
    Dab3FrequencyList << 0;

    for (int cntIdx = 0; cntIdx < (MAX_NB_DAB_FREQUENCIES + 1); cntIdx++)
    {
        if (dabFreqVal == Dab3FrequencyList[cntIdx])
        {
            ensIdx = cntIdx;

            break;
        }
    }

    return ensIdx;
}

void DabRadio::on_btn_seekup_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    isPresetListOn = false;
    lastSlsIsValid = false;

    // Block buttons to avoid multi inputs
    ButtonBlockSignals(true);

    // Signal command ongoing to block onscreen information like DLS
    tuneIsOngoing = true;

    // Signal SLS/PTY status is disabled
    smMachineDab->DisableState(STATE_SLS_PTY);

    // We ne4ed to avoid to expire the preset timer because we start a tune action
    // maybe meanwhile presets are visible and timer to return to old view has been started
    if (true == delayActionTimer->isActive() && RADIO_PRESET_ACTION == actionExecuted)
    {
        delayActionTimer->stop();
        actionExecuted = RADIO_ACTION_NONE;
    }

    lastPIid = currentPIid;

    SetCurrSeekModeStatus(SEEK_UP_MODE);

    setPanel(RADIO_SEEK_UP_ACTION);

    emit EventFromHmiSignal(EVENT_SEEK_UP);
}

void DabRadio::on_btn_seekdw_clicked()
{
    // Check if action can be done in current status
    if (false == CheckIfButtonActionCanBeDone())
    {
        return;
    }

    isPresetListOn = false;
    lastSlsIsValid = false;

    // Block buttons to avoid multi inputs
    ButtonBlockSignals(true);

    // Signal command ongoing to block onscreen information like DLS
    tuneIsOngoing = true;

    // Signal SLS/PTY status is disabled
    smMachineDab->DisableState(STATE_SLS_PTY);

    // We need to avoid to expire the preset timer because we start a tune action
    // maybe meanwhile presets are visible and timer to return to old view has been started
    if (true == delayActionTimer->isActive() && RADIO_PRESET_ACTION == actionExecuted)
    {
        delayActionTimer->stop();
        actionExecuted = RADIO_ACTION_NONE;
    }

    lastPIid = currentPIid;

    SetCurrSeekModeStatus(SEEK_DOWN_MODE);

    setPanel(RADIO_SEEK_DOWN_ACTION);

    emit EventFromHmiSignal(EVENT_SEEK_DOWN);
}

void DabRadio::on_hFreqSlider_actionTriggered(int action)
{
    Q_UNUSED(action);
}

QString DabRadio::getRadioLogo(QString serviceId)
{
    bool found = false;
    QString currLogo;
    QStringList nameFilter;

    nameFilter << "*.png" << "*.jpg" << "*.gif";

    QDir directory("./radio_logos");
    QFileInfoList list = directory.entryInfoList(nameFilter, QDir::Files);
    QListIterator<QFileInfo> it(list);

    serviceId = serviceId.mid(4, 4); // We use logos only for audio services so we can analyze
    QString serviceIdNoSpaces = serviceId.remove(QChar(' '), Qt::CaseInsensitive);

    while (it.hasNext())
    {
        QFileInfo fileInfo = it.next();
        QString tmpStr = fileInfo.fileName();

        if (true == tmpStr.contains(serviceIdNoSpaces, Qt::CaseInsensitive))
        {
            currLogo = fileInfo.absoluteFilePath();
            found = true;

            break;
        }
    }

    if (false == found)
    {
        if (BAND_FM == radioPersistentData.band)
        {
            currLogo = "./radio_logos/0000.fmStation.png";
        }
    }

    return currLogo;
}

void DabRadio::LoadPresetsList()
{
    #define PRESET_LIST_SIZE     6

    // Display presets
    QString serviceName;
    QString serviceId;
    QString currLogoName;

    // Rebuild preset list
    ui->lstWidget_currPresetList->clear();

    for (int cnt = 0; cnt < PRESET_LIST_SIZE; cnt++)
    {
        RadioServicePreset preset = presetList->GetPreset(cnt);

        serviceName = QString::fromStdString(preset.GetServiceName()).left(16).trimmed();

        if (0 == serviceName.compare("Empty", Qt::CaseInsensitive))
        {
            // We have an empty slot
            serviceId = "00000000";
        }
        else
        {
            // We do have a valid service
            serviceId = "0000" + QString::number(preset.GetId(), 16);
        }

        currLogoName = getRadioLogo(serviceId);

        QListWidgetItem* item = new QListWidgetItem(QIcon(currLogoName), serviceName);

        item->setSizeHint(QSize(45, 45));
        item->setForeground(Qt::white);
        item->setBackground(Qt::transparent);

        ui->lstWidget_currPresetList->setIconSize(QSize(42, 42));
        ui->lstWidget_currPresetList->addItem(item);
    }

    // We have a list now display it
    ui->lstWidget_currPresetList->setVisible(true);

    // Set timer with executed action in order to do closing stuff on timer
    DelayTimerSetupAndStart(RADIO_PRESET_ACTION, DELAY_ACTION_MS);
}

void DabRadio::PresetEventAction(bool presetSaveMode)
{
    ui->lstWidget_currPresetList->blockSignals(true);

    if (true == radioIsOn)
    {
        ui->hFreqSlider->blockSignals(true);

        // Signal command ongoing to block onscreen information like DLS
        tuneIsOngoing = true;

        if (true == presetSaveMode)
        {
            // Execute STORE preset action
            emit EventFromHmiSignal(EVENT_PRESET_SAVE);

            // Send signal to global list application
            emit GlobalListApplicationProlog_Signal(EVENT_PRESET_SAVE, radioPersistentData.band);
        }
        else
        {
            DataContainer data;

            // Send the preset selected by the user
            data.content = QVariant(radioStatusGlobal->panelData.currPresetIndex);

            // Clear labels
            SetClearProperty(RADIO_PRESET_ACTION);

            // make labels invisible
            SetVisibleProperty(RADIO_PRESET_ACTION);

            // Signal strength icon set to signal absent
            SetSignalStrengthIcon(true, BER_NOT_AVAILABLE);

            if (BAND_DAB3 == radioPersistentData.band)
            {
                ui->lbl_slsOnXpadImage->setVisible(false);
            }

            // Execute LOAD preset action
            emit EventFromHmiSignal(EVENT_PRESET_SELECT, data);

            // Send signal to global list application
            emit GlobalListApplicationProlog_Signal(EVENT_PRESET_SELECT, radioPersistentData.band);
        }

        // Enable slider signals again
        ui->hFreqSlider->blockSignals(false);
    }

    ui->lstWidget_currPresetList->blockSignals(false);
}

void DabRadio::mousePressureTimer_timeout_slot()
{
    mousePressureTimer->stop();

    qDebug() << "TIMEOUT SLOT Preset Index = " << presetCurrentRow;

    // Set the preset selected by the user
    ui->lstWidget_currPresetList->setCurrentRow(presetCurrentRow);

    radioStatusGlobal->panelData.currPresetIndex = presetCurrentRow;

    // Execute preset action (STORE)
    PresetEventAction(true);

    delayActionTimer->start(DELAY_ACTION_MS);
}

void DabRadio::on_lstWidget_currPresetList_entered(const QModelIndex& index)
{
    presetCurrentRow = index.row();

    delayActionTimer->start(DELAY_ACTION_MS);
}

void DabRadio::on_btn_clearPresets_clicked()
{
    qDebug() << "Clear Presets requested";

    if (true == ui->btn_clearPresets->isEnabled())
    {
        emit EventFromHmiSignal(EVENT_PRESET_CLEAR);
    }
}

void DabRadio::on_btn_learn_clicked()
{
    qDebug() << "Learn requested";

    if (true == ui->btn_learn->isEnabled())
    {
        emit EventFromHmiSignal(EVENT_LEARN);
    }
}

void DabRadio::configureTestScreenDisplay()
{
    #define TEXT_BOX_FIRST_COL_START    10
    #define TEXT_BOX_SECOND_COL_START   180
    #define TEST_BOX_WIDTH              160

    // Create test screen various labels at runtime
    ui->testscreen_widget->setGeometry(QRect(365, 85, 385, 280));

    ui->testscreen_widget->setParent(this);

    // 'Z' order
    ui->testscreen_widget->raise();

    // Geometry
    ui->lbl_quality_1->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 5, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_2->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 35, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_3->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 65, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_4->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 95, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_5->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 125, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_6->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 155, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_7->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 185, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_8->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 215, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_9->setGeometry(QRect(TEXT_BOX_FIRST_COL_START, 245, TEST_BOX_WIDTH, 30));
    ui->rdsAfList_label->setGeometry(QRect(TEXT_BOX_SECOND_COL_START, 5, 210, 270));
    ui->hexAf_checkBox->setGeometry(QRect((TEXT_BOX_SECOND_COL_START + 10), 240, 131, 32));
    ui->lbl_quality_10->setGeometry(QRect(TEXT_BOX_SECOND_COL_START, 5, TEST_BOX_WIDTH, 30));
    ui->lbl_quality_11->setGeometry(QRect(TEXT_BOX_SECOND_COL_START, 35, TEST_BOX_WIDTH, 30));
}

void DabRadio::on_slsdisplay_checkBox_clicked()
{
    radioPersistentData.slsDisplayEn = ui->slsdisplay_checkBox->isChecked();

    if (false == radioPersistentData.slsDisplayEn)
    {
        // Signal SLS/PTY status is disabled
        smMachineDab->DisableState(STATE_SLS_PTY);
    }
    else
    {
        // Signal SLS/PTY status is enabled
        smMachineDab->EnableState(STATE_SLS_PTY);
    }
}

void DabRadio::on_fmfmServiceFollowingEn_checkBox_clicked()
{
    radioPersistentData.afCheckEn = ui->fmfmServiceFollowingEn_checkBox->isChecked();

    // Update information on screen
    ServiceFollowingFlagsUpdate();
}

void DabRadio::on_dabfmServiceFollowingEn_checkBox_clicked()
{
    radioPersistentData.dabFmServiceFollowingEn = ui->dabfmServiceFollowingEn_checkBox->isChecked();

    // Update information on screen
    ServiceFollowingFlagsUpdate();
}

void DabRadio::on_dabdabServiceFollowingEn_checkBox_clicked()
{
    radioPersistentData.dabDabServiceFollowingEn = ui->dabdabServiceFollowingEn_checkBox->isChecked();

    // Update information on screen
    ServiceFollowingFlagsUpdate();
}

void DabRadio::on_taEn_checkBox_clicked()
{
    radioPersistentData.rdsTaEnabled = ui->taEn_checkBox->isChecked();
}

void DabRadio::on_regionalRdsEn_checkBox_clicked()
{
    radioPersistentData.rdsRegionalEn = ui->regionalRdsEn_checkBox->isChecked();
}

void DabRadio::on_rdsEonEn_checkBox_clicked()
{
    radioPersistentData.rdsEonEn = ui->rdsEonEn_checkBox->isChecked();
}

void DabRadio::on_globalServiceListEn_checkBox_clicked()
{
    radioPersistentData.globalServiceListEn = ui->globalServiceListEn_checkBox->isChecked();
}

void DabRadio::on_unsupportedServicesDisplayEn_checkBox_clicked()
{
    radioPersistentData.displayUnsupportedServiceEn = ui->unsupportedServicesDisplayEn_checkBox->isChecked();
}

void DabRadio::on_alphabeticalServicesEn_checkBox_clicked()
{
    radioPersistentData.alphabeticOrder = ui->alphabeticalServicesEn_checkBox->isChecked();

    // Load service list on screen
    LoadServicesList(true);
}

void DabRadio::updateQualityTestScreen(qualityInfoTy qualInfo)
{
    #define INVALID_BER_NUMBER    "4294967295"

    double di2;

    if (BAND_DAB3 == radioPersistentData.band)
    {
        if (true == radioStatusGlobal->panelData.isGoodLevel)
        {
            ui->lbl_quality_3->setText("SYNC");
        }
        else
        {
            ui->lbl_quality_3->setText("No Sync");
        }

        ui->lbl_quality_4->setText("Ber = " + QString::number(radioStatusGlobal->radioStatus.dynamicData.qualityLevel, 10));

        if ((qualInfo.dabQualityInfo.qualDabTxMode < 5) && (qualInfo.dabQualityInfo.qualDabTxMode > 0))
        {
            ui->lbl_quality_5->setText("Tx Mode = " + QString::number(qualInfo.dabQualityInfo.qualDabTxMode, 10));
        }
        else
        {
            ui->lbl_quality_5->setText("No Tx Mode");
        }

        ui->lbl_quality_6->setText("BitRate = " + QString::number(qualInfo.dabQualityInfo.qualServiceBitRate, 10));

        QString tmpString = QString::number(qualInfo.dabQualityInfo.ficBer);

        if (tmpString == INVALID_BER_NUMBER)
        {
            ui->lbl_quality_7->setText("FicBer = N.A.");
        }
        else
        {
            ui->lbl_quality_7->setText("FicBer = " + QString::number(qualInfo.dabQualityInfo.ficBer, 10));
        }

        tmpString = QString::number(qualInfo.dabQualityInfo.mscBer);

        if (tmpString == INVALID_BER_NUMBER)
        {
            ui->lbl_quality_8->setText("MscBer = N.A.");
        }
        else
        {
            ui->lbl_quality_8->setText("MscBer = " +
                                       QString::number(qualInfo.dabQualityInfo.mscBer, 10));
        }

        tmpString = QString::number(qualInfo.dabQualityInfo.audioBer);

        if (tmpString == INVALID_BER_NUMBER)
        {
            ui->lbl_quality_9->setText("AudioBer = N.A.");
        }
        else
        {
            ui->lbl_quality_9->setText("AudioBer = " + QString::number(qualInfo.dabQualityInfo.audioBer, 10));
        }

        ui->lbl_quality_10->setText("CrcErr = " + QString::number(qualInfo.dabQualityInfo.audioCRCError, 10) + "/" +
                                    QString::number(qualInfo.dabQualityInfo.audioCRCTotal, 10));

        ui->lbl_quality_11->setText("SubCh = " + QString::number(qualInfo.dabQualityInfo.serviceSubCh, 10));
        ui->lbl_quality_10->setVisible(true);
        ui->lbl_quality_11->setVisible(true);

        // fstRF   It is displayed only if was read ( value != 0) during last quality measure
        if (0 != (qualInfo.qualFstRf & 0x7F))
        {
            radioStatusGlobal->radioStatus.dynamicData.signalStrength = qualInfo.qualFstRf;
            if (qualInfo.qualFstRf & 0x80)
            {
                QString rflev_str = QString::number(0x80 - (qualInfo.qualFstRf & 0x7F), 10);

                ui->lbl_quality_1->setText("fstRF:-" + rflev_str + " dBm");
            }
            else
            {
                QString rflev_str = QString::number((qualInfo.qualFstRf & 0x7F), 10);
                ui->lbl_quality_1->setText("fstRF: " + rflev_str + " dBm");
            }
        }

        // fstBB   It is displayed only if was read ( value != 0) during last quality measure
        if (0 != (qualInfo.qualFstBb & 0x7F))
        {
            if (qualInfo.qualFstBb & 0x80)
            {
                ui->lbl_quality_2->setText("fstBB: " + QString::number(qualInfo.qualFstBb, 10));
            }
            else
            {
                ui->lbl_quality_2->setText("fstBB: " + QString::number(qualInfo.qualFstBb, 10));
            }
        }

        // Detune
        // Do nothing

        ui->lbl_quality_8->setVisible(true);
    }
    else if (BAND_FM == radioPersistentData.band)
    {
        ui->lbl_quality_10->setVisible(false);
        ui->lbl_quality_11->setVisible(false);

        // fstRF
        radioStatusGlobal->radioStatus.dynamicData.signalStrength = qualInfo.qualFstRf;
        if (qualInfo.qualFstRf & 0x80)
        {
            ui->lbl_quality_1->setText("fstRF:-" + QString::number(0x80 - (qualInfo.qualFstRf & 0x7F), 10) + " dBuV");
        }
        else
        {
            ui->lbl_quality_1->setText("fstRF: " + QString::number((qualInfo.qualFstRf & 0x7F), 10) + " dBuV");
        }

        // fstBB
        if (qualInfo.qualFstBb & 0x80)
        {
            ui->lbl_quality_2->setText("fstBB:-" + QString::number(0x80 - (qualInfo.qualFstBb & 0x7F), 10) + " dBuV");
        }
        else
        {
            ui->lbl_quality_2->setText("fstBB: " + QString::number((qualInfo.qualFstRf & 0x7F), 10) + " dBuV");
        }

        // Detune
        int ii;

        di2 = (195.0 * qualInfo.fmQualityInfo.qualDetune) / 1000;

        if (di2 < 0)
        {
            ii = static_cast<int> (50.0 + di2 + 0.5);
        }
        else
        {
            ii = static_cast<int> (di2 + 0.5);
        }

        ui->lbl_quality_3->setVisible(true);
        ui->lbl_quality_3->setText("Det: " + QString::number(ii, 10) + " kHz");

        // MP
        if (qualInfo.fmQualityInfo.qualMultiPath == 0)
        {
            ui->lbl_quality_4->setText("MP : " + QString::number(0, 10) + " %");
        }
        else
        {
            quint8 mpval1;

            if (qualInfo.fmQualityInfo.qualMultiPath == 0xFF)
            {
                mpval1 = qualInfo.fmQualityInfo.qualMultiPath;
            }
            else
            {
                mpval1 = qualInfo.fmQualityInfo.qualMultiPath + 1;
            }

            ui->lbl_quality_4->setText("MP : " + QString::number((quint8)(0.39 * mpval1), 10) + " %");
        }
        ui->lbl_quality_4->setVisible(true);

        // FM MPX noise / AM not used
        if (qualInfo.fmQualityInfo.qualMpxNoise == 0)
        {
            ui->lbl_quality_5->setText("MpxN: " + QString::number(0, 10) + " %");
        }
        else
        {
            quint8 adjval1;

            if (qualInfo.fmQualityInfo.qualMpxNoise == 0xFF)
            {
                adjval1 = qualInfo.fmQualityInfo.qualMpxNoise;
            }
            else
            {
                adjval1 = qualInfo.fmQualityInfo.qualMpxNoise + 1;
            }

            ui->lbl_quality_5->setText("MpxN: " + QString::number((quint8)(0.39 * adjval1), 10) + " %");
        }
        ui->lbl_quality_5->setVisible(true);

        // SNR (FM/AM)
        ui->lbl_quality_6->setVisible(true);
        ui->lbl_quality_6->setText("SNR: " + QString::number(qualInfo.fmQualityInfo.qualSNR, 10) + " %");

        // ADJACENT (FM/AM)
        if (qualInfo.fmQualityInfo.qualAdj > 0x7F)
        {
            if (qualInfo.fmQualityInfo.qualAdj == 0x80)
            {
                ui->lbl_quality_7->setText("Adj: 0 %");
            }
            else
            {
                ui->lbl_quality_7->setText("Adj: " + QString::number((int)(-0.39 * (0x80 - (qualInfo.fmQualityInfo.qualAdj & 0x7F)) + 50.5f), 10) + " %");
            }
        }
        else
        {
            ui->lbl_quality_7->setText("Adj: " + QString::number((int)(0.39 * qualInfo.fmQualityInfo.qualAdj + 50.5f), 10) + " %");
        }
        ui->lbl_quality_7->setVisible(true);

        // VPA Co-Channel (only FM)
        if (false == vpa_on)
        {
            ui->lbl_quality_8->setVisible(false);
        }
        else
        {
            ui->lbl_quality_8->setText("CoCh: " + QString::number((int)(0.39 * qualInfo.fmQualityInfo.qualCoChannel + 0.5f), 10) + " %");
            ui->lbl_quality_8->setVisible(true);
        }

        // Deviation (FM)
        // FM: 0x7F corresponds to 200  kHz deviation resolution 200kHz/127 = 1.575 kHz
        ui->lbl_quality_9->setText("Dev: " + QString::number((int)(1.575 * (qualInfo.fmQualityInfo.qualDeviation) + 0.5f), 10) + " kHz");
        ui->lbl_quality_9->setVisible(true);
    }
    else if (BAND_AM == radioPersistentData.band)
    {
        ui->lbl_quality_10->setVisible(false);
        ui->lbl_quality_11->setVisible(false);

        // fstRF
        radioStatusGlobal->radioStatus.dynamicData.signalStrength = qualInfo.qualFstRf;
        if (qualInfo.qualFstRf & 0x80)
        {
            ui->lbl_quality_1->setText("fstRF:-" + QString::number(0x80 - (qualInfo.qualFstRf & 0x7F), 10) + " dBuV");
        }
        else
        {
            ui->lbl_quality_1->setText("fstRF: " + QString::number((qualInfo.qualFstRf & 0x7F), 10) + " dBuV");
        }

        // fstBB
        if (qualInfo.qualFstBb & 0x80)
        {
            ui->lbl_quality_2->setText("fstBB:-" + QString::number(0x80 - (qualInfo.qualFstBb & 0x7F), 10) + " dBuV");
        }
        else
        {
            ui->lbl_quality_2->setText("fstBB: " + QString::number((qualInfo.qualFstRf & 0x7F), 10) + " dBuV");
        }

        // Detune
        int ii;
        di2 = (195.0 * qualInfo.fmQualityInfo.qualDetune) / 10;

        if (di2 < 0)
        {
            ii = static_cast<int> (4993.0 + di2 + 0.5);
        }
        else
        {
            ii = static_cast<int> (di2 + 0.5);
        }

        ui->lbl_quality_3->setText("Det: " + QString::number(ii, 10) + " Hz");
        ui->lbl_quality_3->setVisible(true);

        // MP
        ui->lbl_quality_4->setVisible(false);

        // AM MPX noise not used
        ui->lbl_quality_5->setVisible(false);

        // SNR (FM/AM)
        ui->lbl_quality_6->setText("SNR: " + QString::number(qualInfo.fmQualityInfo.qualSNR, 10) + " %");
        ui->lbl_quality_6->setVisible(true);

        // ADJACENT (FM/AM)
        ui->lbl_quality_8->setVisible(false);
        if (qualInfo.fmQualityInfo.qualAdj > 0x7F)
        {
            if (qualInfo.fmQualityInfo.qualAdj == 0x80)
            {
                ui->lbl_quality_7->setText("Adj: 0 %");
            }
            else
            {
                ui->lbl_quality_7->setText("Adj: " + QString::number((int)(-0.39 * (0x80 - (qualInfo.fmQualityInfo.qualAdj & 0x7F)) + 50.5f), 10) + " %");
            }
        }
        else
        {
            ui->lbl_quality_7->setText("Adj: " + QString::number((int)(0.39 * qualInfo.fmQualityInfo.qualAdj + 50.5f), 10) + " %");
        }
        ui->lbl_quality_7->setVisible(true);

        // Mod (AM)
        // AM: 0x7F corresponds to 100% modulation    resolution 100% / 127 = 0.787 %
        ui->lbl_quality_9->setText("Mod: " + QString::number((int)(0.787 * (qualInfo.fmQualityInfo.qualDeviation) + 0.5f), 10) + " %");
        ui->lbl_quality_9->setVisible(true);
    }
}

void DabRadio::on_testScreenEn_checkBox_clicked()
{
    radioPersistentData.testScreenIsActive = ui->testScreenEn_checkBox->isChecked();

    // TODO: remove, it is an hack because it is used by radio manager
    radioStatusGlobal->radioStatus.persistentData.testScreenIsActive = radioPersistentData.testScreenIsActive;

    if (true == radioPersistentData.testScreenIsActive)
    {
        smMachineFm->EnableState(STATE_QUALITY);
        smMachineDab->EnableState(STATE_QUALITY);

        qDebug() << "QUALITY status is enabled";
    }
    else
    {
        smMachineFm->DisableState(STATE_QUALITY);
        smMachineDab->DisableState(STATE_QUALITY);

        qDebug() << "QUALITY status is disabled";
    }

    // In case we do not have an active test screen we make it immediately invisible
    SetTestScreenMode(radioPersistentData.testScreenIsActive);
}

void DabRadio::updateStereoFlag(unsigned int stereoFlag)
{
    // Set information visible only if radio is on, band is FM and stereo flag indication is active
    if ((BAND_FM == radioPersistentData.band) && (stereoFlag == 1))
    {
        ui->stereoOn_label->setVisible(true);
    }
    else
    {
        ui->stereoOn_label->setVisible(false);
    }
}

void DabRadio::on_hFreqSlider_sliderPressed()
{
    // Signal that slider is moved we do not want incoming information to replace the frequency that is changing
    sliderCurrentlyMoved = true;
}

void DabRadio::on_ptyEn_checkBox_clicked()
{
    // Here we do nothing, we operate on installed event filter and callback
}

QString DabRadio::ClearPty(unsigned char& currentPty)
{
    QString retVal;

    const QList<QString>* curListPtr;

    if (COUNTRY_EU == radioPersistentData.country)
    {
        curListPtr = &pty::ptyEuList;
    }
    else
    {
        curListPtr = &pty::ptyUsList;
    }

    currentPty = 0;

    retVal = curListPtr->at(currentPty);

    return retVal;
}

QString DabRadio::GetNextPty(unsigned char& currentPty)
{
    QString retVal;

    const QList<QString>* curListPtr;

    if (COUNTRY_EU == radioPersistentData.country)
    {
        curListPtr = &pty::ptyEuList;
    }
    else
    {
        curListPtr = &pty::ptyUsList;
    }

    if (currentPty < (curListPtr->size() - 1))
    {
        currentPty++;
    }
    else
    {
        currentPty = 0;
    }

    retVal = curListPtr->at(currentPty);

    return retVal;
}

void DabRadio::CheckBoxClick_slot(bool labelClick, bool checkBoxClick)
{
    qDebug() << "My event filter has been hit (1)";

    if (true == labelClick)
    {
        QString newPty = GetNextPty(radioPersistentData.ptySelected);

        qDebug() << newPty;

        ui->ptyEn_checkBox->setText(newPty);

        if (0 == radioPersistentData.ptySelected)
        {
            ui->ptyEn_checkBox->setChecked(false);
        }
        else
        {
            ui->ptyEn_checkBox->setChecked(true);
        }
    }
    else if (true == checkBoxClick)
    {
        QString tmpStr = ClearPty(radioPersistentData.ptySelected);

        ui->ptyEn_checkBox->setText(tmpStr);

        ui->ptyEn_checkBox->setChecked(false);
    }
}

QString DabRadio::GetCurrentPtyLabel(unsigned char& currentPty)
{
    QString retVal;

    const QList<QString>* curListPtr;

    if (COUNTRY_EU == radioPersistentData.country)
    {
        curListPtr = &pty::ptyEuList;
    }
    else
    {
        curListPtr = &pty::ptyUsList;
    }

    retVal = curListPtr->at(currentPty);

    return retVal;
}

// End of file
