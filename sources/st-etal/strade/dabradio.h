#ifndef DABRADIO_H
#define DABRADIO_H

#include <QMainWindow>
#include <QDebug>
#include <QImage>
#include <QPoint>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QVariant>
#include <QMainWindow>

#include <QIcon>
#include <QLabel>
#include <QCheckBox>
#include <QBitmap>
#include <qvector.h>
#include <QMessageBox>
#include <QThread>
#include "qdatetime.h"
#include "qdir.h"
#include <stack>
#include <QList>
#include <QTimer>
#include <QtCore>
#include <qfile.h>
#include <QTime>
#include <QDateTime>

#include "common.h"
#include "defines.h"
#include "target_config.h"
#include "radiomanagerbase.h"
#include "station_list_global.h"
#include "cmd_mngr_cis.h"
#include "tcptransportlayer.h"
#include "postaloffice.h"
#include "worker.h"
#include "presets.h"
#include "sm.h"

namespace Ui
{
    class DabRadio;
}

enum SeekStatusTy
{
    SEEK_DOWN_MODE         = -1,
    NO_SEEK_MODE           = 0,
    SEEK_UP_MODE           = 1
};

enum ListCntMode
{
    LIST_IN_TESTSCREEN     = 0,
    LIST_IN_SERVICELIST    = 1,
    LIST_IN_SLSSCREEN      = 2
};

enum DisplayView
{
    VIEW_ACTIVE_NONE        = 0,
    VIEW_ACTIVE_SERVICELIST = 1,
    VIEW_ACTIVE_PRESETS     = 2,
    VIEW_ACTIVE_QUALITY     = 3,
    VIEW_ACTIVE_SETUP       = 4
};

struct DisplayStatus
{
    DisplayView currentView;
    DisplayView nextView;
};

struct CheckMousePressureTime
{
    qint64 msFromEpoch;
    qint64 deltaFromLatestEvent;
};

struct CountryDependentValues
{
    quint32 minBandValue;
    quint32 maxBandValue;

    quint32 step;

    QString ptyStr;
};

class DabRadio : public QMainWindow,
    public FilesManager,
    public DataMessages,
    public DataChannel
{
    Q_OBJECT

    public:
        explicit DabRadio(RadioStatusGlobal* radioGlobalData, QWidget* parent = 0,
                          QString _dataMsgInstName = "DabRadio");

        ~DabRadio();

        SeekStatusTy GetCurrSeekModeStatus() { return isSeekMode; }
        void SetCurrSeekModeStatus(SeekStatusTy m_isSeekMode) { isSeekMode = m_isSeekMode; }

        RadioViewsTy GetCurrViewName() { return currViewName; }
        void SetCurrViewName(RadioViewsTy m_currViewName) { currViewName = m_currViewName; }

        void PanelStartUp();

    private:
        int m_nMouseClick_X_Coordinate;
        int m_nMouseClick_Y_Coordinate;

        RadioStatusGlobal* radioStatusGlobal;

        Ui::DabRadio* ui;

        Utilities* utils;
        QTimer* waitTimer;
        QTimer* tickTimer;
        QTimer* delayActionTimer;

        QTimer* signalIndicationTimer;
        QString signalTextToDisplay;

        QTimer* mousePressureTimer;
        CheckMousePressureTime checkMousePressureTime;
        int presetCurrentRow;

        CountryDependentValues mwCountryDependentValues;
        CountryDependentValues fmCountryDependentValues;

        QPropertyAnimation* lbl_P_animation;
        QPropertyAnimation* lbl_O_animation;
        QPropertyAnimation* lbl_W_animation;
        QPropertyAnimation* lbl_E_animation;
        QPropertyAnimation* lbl_R_animation;
        QPropertyAnimation* lbl_O_1_animation;
        QPropertyAnimation* lbl_F_animation;
        QPropertyAnimation* lbl_F_1_animation;
        QGraphicsOpacityEffect* lbl_radioBackground_opacity;
        QPropertyAnimation* lbl_radioBackground_animation;
        QParallelAnimationGroup* radioOffAnimationGroup;

        QTimer* lblWaitAnimationTimer;
        QTimer* opacityAnimationsTimer_2;
        QTimer* waitLblAnimationTimer;

        QGraphicsOpacityEffect* sls_image_opacity;
        QPropertyAnimation* sls_image_animation;

        QGraphicsOpacityEffect* sls_background_opacity;

        QGraphicsOpacityEffect* listWidget_opacity;
        QPropertyAnimation* listWidget_animation;

        QGraphicsOpacityEffect* logo_opacity;
        QPropertyAnimation* logo_animation;

        QGraphicsOpacityEffect* background_off_opacity;
        QPropertyAnimation* background_off_animation;

        QGraphicsOpacityEffect* background_opacity;
        QPropertyAnimation* background_animation;

        QGraphicsOpacityEffect* searching_lbl_opacity;
        QPropertyAnimation* searching_lbl_animation;

        QGraphicsOpacityEffect* lbl_searching_OpacityEffect;
        QPropertyAnimation* lbl_searching_propertyAnimation;

        QCheckBox* fmfmServiceFollowingEn_checkbox;
        QCheckBox* dabfmServiceFollowingEn_checkbox;
        QCheckBox* dabdabServiceFollowingEn_checkbox;
        QCheckBox* taEn_checkbox;
        QCheckBox* regionalRdsEn_checkbox;
        QCheckBox* rdsEonEn_checkbox;
        QCheckBox* globalServiceListEn_checkbox;
        QCheckBox* unsupportedServicesDisplayEn_checkbox;

        SeekStatusTy isSeekMode;
        bool bootCompleted;
        bool isWaiting;     // Wait methode flag
        bool isClosing;
        bool tuneIsOngoing;
        bool radioIsOn;

        bool isPresetListOn;
        bool isSetupScreenOn;
        bool isLearnActive;

        bool vpa_on;

        QFont           font;
        QPoint          offset;
        QSize           btnMinimizeSize;
        QRect           btnMinimizeGeom;
        RadioViewsTy    currViewName;

        RadioActionsTy actionExecuted;
        QString lastLabelDisplayed;

        EnsembleTableTy ensTable;
        ServiceListTy list;

        QStringList onScreenServiceList;
        QList<ServiceTy> onScreenServiceDatabase;
        QString selectedService;

        bool sliderCurrentlyMoved;

        QString  currentRadioLogo;

        bool powerOffStatus;

        QString lastPIid;
        QString currentPIid;

        int moveOffsetX;
        int moveOffsetY;

        QPixmap lastSlsImage;
        bool    lastSlsIsValid;

        Presets* presetList;

        DisplayStatus displayStatus;

        quint32 fmFreqkHz;
        quint32 mwFreqkHz;
        quint32 dabFreqMHz;

        //BandTy currentBand;

        SmMachine* smMachineDab;
        SmMachine* smMachineFm;
        SmMachine* currentSmMachine;

        PersistentData radioPersistentData;

        // Buttons list
        QList<QPushButton *> physicalButtonsWidgetList;
        QList<QPushButton *> unusedButtonsWidgetList;

        // Visualize the display according to the new view
        QList<QWidget *> viewAllWidgetList;
        QList<QWidget *> viewPresetsWidgetList;
        QList<QWidget *> viewServiceListWidgetList;
        QList<QWidget *> viewSlsPtyWidgetList;
        QList<QWidget *> viewQualityWidgetList;
        QList<QWidget *> viewSetupWidgetList;
        QList<QWidget *> viewPowerOffWidgetList;

        QMovie* movie;

        void SetFonts();

        bool InitializeDisplayViewStateMachine();
        void DisplayNewView(QString action);
        void SetupInitialStateForDisplayView();
        void SetupWidgetLists();
        void ModifyDisplayView(QString newState, QString oldState, QString action);

        void DelayTimerSetupAndStart(RadioActionsTy action, int timeout);

        void SetSignalStrengthIcon(bool visible, quint32 qualityValue);

        //void StoreInitialFrequencies();
        void CalculateCountryDependentValues(CountryTy country);

        bool DisplayReceivedInformation();
        void putInTestScreenMode();
        void OnScreenPressViewUpdate();

        StatusCodeTy GetMdwResponsesStatus();
        StatusCodeActionsTy HandleMdwStatusCodes(MdwCommandCodeTy mdwCmdCode, StatusCodeTy status);

        void DlsButtonsAnimGeometrySetup();
        void SaveDefaultButtonsGeometry(QRect geometry);

        // Setup
        void DabRadioSetup();
        void GraphicsItemsSetup();
        void RadioClockSetup();
        void setPanel(RadioActionsTy action);
        void LoadServicesList(bool display);
        QString getRadioLogo(QString serviceId);
        void LoadPresetsList();
        void LoadRadioOffSettings();
        void LoadRadioOnSettings();
        void displaySelectedBand();

        void ButtonBlockSignals(bool enableBlock);

        // Comunication
        void RadioGetResponses(QByteArray data, MdwCommandCodeTy m_commandCode);
        QStringList RadioSendCommand(MdwCommandCodeTy m_commandCode);

        // State machines
        void LoadAnimationData();
        void GoToPreviousFrequency();
        void timeout_slot();
        void GoToNextFrequency();
        void wait(int msec);
        void CalculatePrevDabFrequency(quint32& dabFreqVal);
        void CalculateNextDabFrequency(quint32& dabFreqVal);

        void AnimationsMachine(AnimationNameTy animationName, AnimationActionsTy animationAction);
        void updateSyncQuality(bool isGoodLevel, unsigned int qualityLevel, unsigned int signalStrength, bool force = false);

        // Animations
        void WaitLabelAnimation(AnimationActionsTy animationAction);
        void LblSearchingAnimation(AnimationActionsTy animAction);
        void BackgroundLblOpacityAnimation_ON(AnimationActionsTy animationAction, bool status);
        void STLogoLblOpacityAnimation_ON(AnimationActionsTy animationAction, bool status);
        void RadioOffViewAnimationGroup();
        void IcoWaitAnimation(AnimationActionsTy animationAction);

        // Set properties on action
        void SetClearProperty(RadioActionsTy actionName);
        void SetVisibleProperty(RadioActionsTy actionName);

        // Set properties on view
        void SetStyleSheetProperty(RadioViewsTy view);
        void SetVisibleProperty(RadioViewsTy view);
        void SetZOrderProperty(RadioViewsTy view);
        void ViewsSwitcher(RadioViewsTy view);

        void initSlider(quint8 band);
        void setMwFreqLabels();
        void setFmFreqLabels();
        void setDabFreqLabels(void);
        void hide_ensembleRadioLabels();
        void DisplayActiveService();
        void ServiceListOnScreen(int currentIndex);

        QString fmFreq2String(quint32 cFreq);
        QString dabFreq2String(quint32 cFreq);
        QString getDab3Channel(int index);
        quint32 getDab3FrequencyVal(int index);
        int getDab3EnsembleIndex(quint32 dabFreqVal);

        void AnimationsStopOnEventResponse(Events event);
        void Display_SlsPtyImage(quint8 ptyValue);
        void DisplayRadioLogo(QString curServiceId);
        QString DisplayDefaultPSname(QString psFileName, QString serviceId);

        void BlinkingText(bool start);

        void StartTuneAction();
        void rdsPanelUpdateData(RdsDataSetTableTy rdsdataset);

        void PresetEventAction(bool presetSaveMode);
        void ServiceFollowingFlagsUpdate();
        void updateQualityTestScreen(qualityInfoTy qualInfo);
        void updateStereoFlag(unsigned int stereoFlag);

        void SetRdsAfDisplay(bool active);
        void SetTestScreenMode(bool enable);

        bool CheckIfButtonActionCanBeDone();

        bool RenderSlsPtyImage(slsPtyRenderSize size = IMAGE_RENDER_SIZE_NORMAL);

        void PowerOnAction();
        void PowerOffAction();

        void StoreLastStatusData(PersistentData& data);

        QString ClearPty(unsigned char& currentPty);
        QString GetNextPty(unsigned char& currentPty);
        QString GetCurrentPtyLabel(unsigned char& currentPty);

    private slots:
        void on_btn_minimize_clicked();
        void on_btn_mute_clicked();
        void on_btn_voldw_clicked();
        void on_btn_volup_clicked();
        void on_btn_onoff_clicked();
        void on_lstWidget_currServiceList_pressed(const QModelIndex& index);
        void on_btn_src_clicked();
        void on_hFreqSlider_valueChanged(int value);
        void on_hFreqSlider_sliderReleased();
        void on_btn_list_clicked();
        void on_btn_preset_clicked();
        void on_btn_up_clicked();
        void on_btn_audio_clicked();
        void on_btn_radio_clicked();
        void on_btn_setting_clicked();
        void on_btn_setup_clicked();
        void on_btn_seekup_clicked();
        void on_btn_seekdw_clicked();
        void on_btn_tuneNext_clicked();
        void on_btn_tunePrevious_clicked();
        void on_hFreqSlider_actionTriggered(int action);

        void on_lstWidget_currPresetList_entered(const QModelIndex& index);

        void on_btn_clearPresets_clicked();

        void updateTaRds_slot();
        void updateRegionalRds_slot();
        void updateRdsEon_slot();
        void updateGlobalServiceList_slot();
        void updateUnsupportedServicesDisplay_slot();

        void on_btn_learn_clicked();
        void configureTestScreenDisplay();
        void on_slsdisplay_checkBox_clicked();
        void on_fmfmServiceFollowingEn_checkBox_clicked();
        void on_dabfmServiceFollowingEn_checkBox_clicked();
        void on_dabdabServiceFollowingEn_checkBox_clicked();
        void on_taEn_checkBox_clicked();
        void on_regionalRdsEn_checkBox_clicked();
        void on_rdsEonEn_checkBox_clicked();
        void on_globalServiceListEn_checkBox_clicked();
        void on_unsupportedServicesDisplayEn_checkBox_clicked();
        void on_testScreenEn_checkBox_clicked();

        void on_hFreqSlider_sliderPressed();

        void on_alphabeticalServicesEn_checkBox_clicked();

        void on_ptyEn_checkBox_clicked();

    public slots:
        void waitTimer_timeout_slot();
        void waitAnimationTimer_timeout_slot();
        void DelayActionTimer_slot();
        void SignalQualityText_slot();

        void tick_timeout_slot();
        void mousePressureTimer_timeout_slot();

        void WorkerAnswerSlot(Events event, StatusCodeTy statusCode, PostalOfficeBase* eventData);
        void WorkerEventFromDeviceSlot(PostalOfficeBase* eventData);

        void CheckBoxClick_slot(bool labelClick, bool checkBoxClick);

    signals:
        void CloseApplication_Signal();
        void EventFromHmiSignal(Events event, DataContainer data);
        void EventFromHmiSignal(Events event);
        void GlobalListApplicationProlog_Signal(Events event, quint32 band);
        void GlobalListApplicationEpilog_Signal(Events event, quint32 band);

    protected:
        bool eventFilter(QObject* obj, QEvent* event);
};

#endif

// End of file
