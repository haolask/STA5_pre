#ifndef CMD_MNGR_CIS_H_
#define CMD_MNGR_CIS_H_

#include <QLocale>
#include <QEvent>
#include "common.h"
#include "radio_storage_types.h"
#include "filemanager.h"
#include "utilities.h"
#include "cmd_mngr_base.h"

#include "tcptransportlayer.h"
#include "rds.h"

enum CisActionsTy
{
    CIS_RESET_ACTION             =   0,
    CIS_READ_FW_VERSION_ACTION   =   1,
    CIS_WRITE_ACTION             =   2,
    CIS_READ_ACTION              =   3,
    CIS_CMD_ACTION               =   4,
    CIS_SETBAND_ACTION           =   5,
    CIS_SETFREQUENCY_ACTION      =   6,
    CIS_BOOT_ACTION              =   7,
    CIS_RESET_DCOP_ACTION        =   8,
    CIS_GET_RDS_ACTION           =   9,
    CIS_SEEK_START_ACTION        =   10,
    CIS_SEEK_RESULT_ACTION       =   11
};

enum CountryNameTy
{
    Europe                 = 0,
    USA                    = 1,
    Japan                  = 2,
    EastEurope             = 3,
    China                  = 4,
    Canada                 = 5,
    Korea                  = 6
};

struct cmostParametersTy
{
    QString bootFileName_t1;    // CMOST Boot File name
    QString bootFileName_t2;    // CMOST Boot File name
    QString bootFileName_t3;    // CMOST Boot File name
    int cmostBoardType;         // CMOST Board type
    CountryNameTy countryName;  // CMOST Country selected
};

class CisCmds : public QObject, public DataMessages, public DataChannel
{
    Q_OBJECT

    public:
        CisCmds(QObject* parent, QString _dataMsgInstName = "CIS_msg", QString _dataChInstName = "CIS_ch");

        ~CisCmds();

        FilesManager* fileMngr;

        StatusCodeTy cmostDeviceReset();
        StatusCodeTy cmostBoot();
        StatusCodeTy cmostPowerOn();
        StatusCodeTy cmostPowerOff();
        bool         isUseModeBOARD_MTD_2_CHANNELS();
        StatusCodeTy initNotSelectedBand(BandTy currentBand);

        CountryNameTy getCmostCountry();

        StatusCodeTy SeekDabManual(quint32 currentFreq, SystemMode usedMode, bool updirection);
        StatusCodeTy sendSeekEndCmd(SystemMode usedMode);
        StatusCodeTy setBand(BandTy currentBand, SystemMode usedMode);
        StatusCodeTy setFrequency(quint32 currentFreq, SystemMode usedMode);
        void sendCmdReservedForProtocolLayer(quint8 cmdtype, quint8 specialVal, quint32 portVal);
        StatusCodeTy cisExecute(CisActionsTy cisAction, unsigned int tunerAddr,
                                QString commandStr, unsigned long waitTime = 0);
        StatusCodeTy ExecuteActionOnAllTuners(CisActionsTy action, QString command, unsigned int waitTime = 0);
        StatusCodeTy cisUnMute(int muteStatus, SystemMode usedMode);
        void cisSetVolume(int rVolume, SystemMode usedMode);
        quint32  getFmFreqFromIndex(int freqIdx);
        quint32  getMwFreqFromIndex(int freqIdx);
        //   quint32  SeekFmSequence(bool updirection, quint32 currSeekFreq, SystemMode usedMode);
        //quint8 getFmFieldStrength( SystemMode usedMode);
        qualityInfoTy getFmQualityParameters(SystemMode usedMode);
        qualityInfoTy getDabFieldStrength(SystemMode usedMode);
        bool RunRdsAcquisition(SystemMode usedMode);
        bool StopRdsAcquisition();
        QByteArray GetRdsData();
        void RdsDataPolling(SystemMode usedMode);
        StatusCodeTy getCurrentFrequency(SystemMode usedMode, unsigned int* activeFrequency);
        StatusCodeTy setCurrentFrequency(SystemMode usedMode,
                                         unsigned int* lastBand,
                                         unsigned int* lastFrequency);
        void SeekFmAuto(bool updirection, int nbSeekMeasures, int stepjump, SystemMode usedMode, quint32 currSeekFreq);
        CisSeekParamsTy  GetSeekFmStatus(int readCmd28DlyValue, SystemMode usedMode);
        void Learn(BandTy currentBand, quint32 currentFreq, SystemMode usedMode);

    private:
        bool isCisNotifStatusReceived;
        bool isCisRespReceived;
        bool isCisConnectionTimeout;

        bool boot_isOk;
        bool bootCompleted;
        int  WrittenCisByte;

        bool vpaOn;

        bool isValidDabStation;
        //  quint8 cisQualityLevel;
        qualityInfoTy  cisQualityInfo;

        CisSeekParamsTy seekfmParams;

        QTimer* rdsTimer;

        Utilities* utils;

        quint8              activeCisDataType;
        quint8              activeCisActionType;

        cmostParametersTy   cmostParams;

        QStringList         bootCmdList;

        quint8              cisBand;

        bool                isRdsOn;
        QList<QByteArray>   rdsBuffersList;

        void cisInitQualityInfo();
        void cmostInit();
        StatusCodeTy cisReadFirmwareVersion(unsigned int tunerAddress);
        cmostParametersTy GetCmostParameters();
        StatusCodeTy SendCmdToDevice(QString command, quint8 dataType, unsigned int tunerAddr);
        StatusCodeTy CisCmdTransmitter(QByteArray parsed_command);
        void SaveCisSpecificCmdData(QByteArray data, quint8 actionType);
        void getCisCmdAnswer(QByteArray data);
        void getCisSeekResult(QByteArray data);
        StatusCodeTy ReadBootFile(QString bootName, int* lines);
        StatusCodeTy executeBoot(int listSize, bool isCmosBoot, unsigned int tunerAddr);
        bool loadScriptFile(QString fileName, QStringList* list);
        QByteArray processCisCommand(QString command, CisCommandCodeTy type);
        CisCommandCodeTy getCommandType(QString command);
        StatusCodeTy cisCommandEncoder(CisCommandCodeTy type, QString command,
                                       QByteArray* encodedCmd, unsigned int tunerAddr);
        void sendDspOutReset(unsigned int tunerAddress);
        StatusCodeTy GetTunerAddressAndChannel(SystemMode usedMode, unsigned int* tunerAddress,
                                               unsigned int* tunerChannel, bool primary = true);
        StatusCodeTy GetTunerCurrentBandAndFrequency(SystemMode usedMode,
                                                     unsigned int* currentBand,
                                                     unsigned int* currentFrequency, bool primary = true);
        StatusCodeTy SetTunerCurrentBandAndFrequency(SystemMode usedMode,
                                                     unsigned int* currentBand,
                                                     unsigned int* currentFrequency, bool primary = true);

    public slots:
        void cisConnectionTimerTimeout_slot();
        void cis_receivedDataFromDispatcher_slot(QByteArray data);

    signals:
        void inShellCmostData_signal(QString, QString, int, unsigned int);
        void inShellCmostError_signal(RadioSequencesStatusTy, QString, QString);
        void cis_sendCmdToDispatcher_signal(QByteArray);
        void initCmostComplete_signal(bool);
        void updateFreq_signal(int);
};

#endif // CMD_MNGR_CIS_H_
