#include <iostream>
#include <fstream>
#include <QString>

#include "cmd_mngr_cis.h"

namespace
{
    enum ProtocolCommandHeaderFormat
    {
        Sync_Byte              = 0,
        Lun_Byte               = 1,
        CommandType_Byte       = 2,
        FormatType_Byte        = 3,
        I2cAddress_Byte        = 4,
        Spare_Byte             = 5,
        PayloadLenghtMsb_Byte  = 6,
        PayloadLenghtLsb_Byte  = 7
    };

    #define BOARD_SUPPORTED_CONF_NUM        3

    const QString BOOTFILE_T1 = "BOOTFILE_T1";
    const QString BOOTFILE_T2 = "BOOTFILE_T2";
    const QString BOOTFILE_T3 = "BOOTFILE_T3";
    const QString BOARD = "BOARD";
    const QString COUNTRY = "COUNTRY";

    const QString BOARD_STAR_DCOP = "BOARD_STAR_DCOP";
    const QString BOARD_MTD_2_CHANNELS = "BOARD_MTD_2_CHANNELS";
    const QString BOARD_MTD_4_CHANNELS = "BOARD_MTD_4_CHANNELS";

    #define INVALID_TUNER                   0xFF
    #define INVALID_CHANNEL                 0xFF

    #define TUNER1_I2C_ADDR                 0xC2
    #define TUNER2_I2C_ADDR                 0xC8
    #define TUNER3_I2C_ADDR                 0xC0

    #define INVALID_BAND                    0xFF
    #define INVALID_FREQUENCY               1

    struct CurrentChBandFrequencyStatus
    {
        QString name;

        unsigned int chAtuner1Band;
        unsigned int chAtuner1Frequency;

        unsigned int chBtuner1Band;
        unsigned int chBtuner1Frequency;

        unsigned int chAtuner2Band;
        unsigned int chAtuner2Frequency;

        unsigned int chBtuner2Band;
        unsigned int chBtuner2Frequency;
    };

    CurrentChBandFrequencyStatus currentChBandFrequencyStatus[BOARD_SUPPORTED_CONF_NUM] =
    {
        { BOARD_STAR_DCOP, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY },
        { BOARD_MTD_2_CHANNELS, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY },
        { BOARD_MTD_4_CHANNELS, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY, INVALID_BAND, INVALID_FREQUENCY }
    };

    struct BoardDescriptor
    {
        QString name;

        unsigned int numTuners;
        int numChannels;

        unsigned int fmAmPrimaryTunerAddress;
        unsigned int fmAmPrimaryChannelNumber;

        unsigned int dabPrimaryTunerAddress;
        unsigned int dabPrimaryChannelNumber;

        unsigned int fmAmSecondaryTunerAddress;
        unsigned int fmAmSecondaryChannelNumber;

        unsigned int dabSecondaryTunerAddress;
        unsigned int dabSecondaryChannelNumber;
    };

    const BoardDescriptor boardDescriptor[BOARD_SUPPORTED_CONF_NUM] =
    {
        { BOARD_STAR_DCOP, 1, 1, 0xC2, 1, 0xC2, 1, INVALID_TUNER, INVALID_CHANNEL, INVALID_TUNER, INVALID_CHANNEL },
        { BOARD_MTD_2_CHANNELS, 2, 2, 0xC2, 1, 0xC8, 1, 0xC2, 2, INVALID_TUNER, INVALID_CHANNEL },
        //  { BOARD_MTD_2_CHANNELS, 2, 2, 0xC2, 1, 0xC8, 1, INVALID_TUNER, INVALID_CHANNEL, INVALID_TUNER, INVALID_CHANNEL },
        { BOARD_MTD_4_CHANNELS, 2, 4, 0xC2, 1, 0xC8, 1, 0xC2, 2, 0xC8, 2 }
    };

    const unsigned long CIS_WAIT_AFTER_CMD_MS = 100;
}

CisCmds::CisCmds(QObject* parent, QString _dataMsgInstName, QString _dataChInstName) :
    QObject(parent), DataMessages(_dataMsgInstName), DataChannel(_dataChInstName)
{
    isRdsOn = false;
    rdsBuffersList.clear();

    cmostInit();
}

CisCmds::~CisCmds()
{ }

void CisCmds::cisConnectionTimerTimeout_slot()
{
    isCisConnectionTimeout    = true;
    isCisNotifStatusReceived  = true;
}

StatusCodeTy CisCmds::ExecuteActionOnAllTuners(CisActionsTy action, QString command, unsigned int waitTime)
{
    StatusCodeTy retValue;
    int numTuners;

    numTuners = boardDescriptor[cmostParams.cmostBoardType].numTuners;

    if (1 == numTuners)
    {
        retValue = cisExecute(action, TUNER1_I2C_ADDR, command, waitTime);
    }
    else if (2 == numTuners)
    {
        retValue = cisExecute(action, TUNER1_I2C_ADDR, command, waitTime);

        if (CMD_OK == retValue)
        {
            retValue = cisExecute(action, TUNER2_I2C_ADDR, command, waitTime);
        }
    }
    else if (3 == numTuners)
    {
        retValue = cisExecute(action, TUNER1_I2C_ADDR, command, waitTime);

        if (CMD_OK == retValue)
        {
            retValue = cisExecute(action, TUNER2_I2C_ADDR, command, waitTime);

            if (CMD_OK == retValue)
            {
                retValue = cisExecute(action, TUNER3_I2C_ADDR, command, waitTime);
            }
        }
    }
    else
    {
        retValue = WRONG_PARAMETERS;
    }

    return retValue;
}

StatusCodeTy CisCmds::cmostDeviceReset()
{
    return ExecuteActionOnAllTuners(CIS_RESET_ACTION, "RST");
}

StatusCodeTy CisCmds::cisReadFirmwareVersion(unsigned int tunerAddress)
{
    StatusCodeTy retValue = CMD_OK;

    retValue = cisExecute(CIS_READ_FW_VERSION_ACTION, tunerAddress, "R 01401E 2");

    if (retValue != CMD_OK)
    {
        emit inShellCmostData_signal("\n[r] Failed CMOST Read, HW or connection problem", " ", RADIO_DATA_NOTIFY, tunerAddress);
    }

    return retValue;
}

StatusCodeTy CisCmds::ReadBootFile(QString bootName, int* lines)
{
    bootCmdList.clear();

    if (true == loadScriptFile(bootName, &bootCmdList))
    {
        *lines = bootCmdList.size();

        return CMD_OK;
    }

    *lines = 0;

    return OPENFILE_ERROR;
}

bool CisCmds::loadScriptFile(QString fileName, QStringList* list)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "ERROR: loadScriptFile...";

        return false;
    }

    QTextStream in(&file);
    int n = 0;
    QStringList cmdList;

    while (!in.atEnd())
    {
        QString line = in.readLine(); // Read one line at a time

        if (line.startsWith("//"))
        {
            qDebug() << "Skipped line[" << n << "] : " << line;
        }
        else
        {
            line.simplified();
            cmdList.append(line);
        }

        n++;
    }

    file.close();

    list->clear();
    *list =  cmdList;

    return true;
}

StatusCodeTy CisCmds::cisExecute(CisActionsTy cisAction, unsigned int tunerAddr,
                                 QString commandStr, unsigned long waitTime)
{
    StatusCodeTy retValue = CMD_OK;

    switch (cisAction)
    {
        case CIS_RESET_ACTION:
        {
            activeCisDataType = RESET_IC;

            emit inShellCmostData_signal("\n[c] Reset CMOST",
                                         " " + commandStr, RADIO_DATA_COMMAND, tunerAddr);
        }
        break;

        case CIS_RESET_DCOP_ACTION:
        {
            activeCisDataType = RESET_OTHER_DEVICE;

            emit inShellCmostData_signal("\n[c] Reset DCOP device",
                                         " " + commandStr, RADIO_DATA_COMMAND, tunerAddr);
        }
        break;

        case CIS_READ_FW_VERSION_ACTION:
        {
            activeCisDataType = READ_TYPE;

            emit inShellCmostData_signal("\n[c] CIS Read Firmware Version",
                                         " " + commandStr, RADIO_DATA_COMMAND, tunerAddr);
        }
        break;

        case CIS_CMD_ACTION:
        case CIS_SETBAND_ACTION:
        case CIS_SETFREQUENCY_ACTION:
        case CIS_GET_RDS_ACTION:
        case CIS_SEEK_START_ACTION:
        case CIS_SEEK_RESULT_ACTION:
        {
            activeCisDataType = CMD_TYPE;

            emit inShellCmostData_signal("\n[c] CIS Cmd: ", commandStr, RADIO_DATA_COMMAND, tunerAddr);
        }
        break;

        case CIS_READ_ACTION:
        {
            activeCisDataType = READ_TYPE;

            emit inShellCmostData_signal("\n[i] CIS Read: ", "\n[c] " + commandStr, RADIO_DATA_COMMAND, tunerAddr);
        }
        break;

        case CIS_WRITE_ACTION:
        {
            activeCisDataType = WRITE_TYPE;

            emit inShellCmostData_signal("\n[c] CIS ", commandStr, RADIO_DATA_COMMAND, tunerAddr);
        }
        break;

        case CIS_BOOT_ACTION:
        {
            activeCisDataType = WRITE_TYPE;
        }
        break;

        default:
        {
            activeCisDataType = WRITE_TYPE;
        }
        break;
    }

    activeCisActionType = cisAction;

    // Send command
    retValue = SendCmdToDevice(commandStr, activeCisDataType, tunerAddr);

    // If requested wait after command is send
    if (waitTime > 0)
    {
        QThread::msleep(waitTime);
    }

    return retValue;
}

StatusCodeTy CisCmds::SendCmdToDevice(QString command, quint8 dataType, unsigned int tunerAddr)
{
    StatusCodeTy retValue = CMD_OK;

    // Parse cis command and check sintax errors
    QByteArray parsed_command;

    cisCommandEncoder((CisCommandCodeTy)dataType, command, &parsed_command, tunerAddr);

    // Transmit cis command
    retValue = CisCmdTransmitter(parsed_command);

    return retValue;
}

StatusCodeTy CisCmds::CisCmdTransmitter(QByteArray parsed_command)
{
    if (parsed_command.size() == 0)
    {
        return NO_DATA_TO_WRITE;
    }

    isCisConnectionTimeout  = false;
    isCisNotifStatusReceived   = false;
    isCisRespReceived          = false;

    QTimer cisConnectionTimer;

    // Create the cisConnectionTimer object
    cisConnectionTimer.setInterval(3000);
    cisConnectionTimer.setSingleShot(true);

    // Connect the connectionTimer  timeout event to proper slot
    QObject::connect(&cisConnectionTimer, &QTimer::timeout, this, &CisCmds::cisConnectionTimerTimeout_slot);

    cisConnectionTimer.start();

    emit cis_sendCmdToDispatcher_signal(parsed_command);

    // Wait the notification
    while (false  == isCisNotifStatusReceived)
    {
        QApplication::processEvents();
    }

    cisConnectionTimer.stop();

    if (true == isCisConnectionTimeout)
    {
        return CONNECTION_ERRORS;
    }

    return CMD_OK;
}

void CisCmds::cisInitQualityInfo()
{
    cisQualityInfo.qualFicBer = 0;
    cisQualityInfo.qualFstRf = 0;
    cisQualityInfo.qualFstBb = 0;
    cisQualityInfo.fmQualityInfo.qualDetune = 0;
    cisQualityInfo.fmQualityInfo.qualMultiPath = 0;
    cisQualityInfo.fmQualityInfo.qualMpxNoise = 0;
    cisQualityInfo.fmQualityInfo.qualSNR = 0;
    cisQualityInfo.fmQualityInfo.qualAdj = 0;
    cisQualityInfo.fmQualityInfo.qualCoChannel = 0;
    cisQualityInfo.fmQualityInfo.qualDeviation = 0;
    cisQualityInfo.fmQualityInfo.qualStereo = 0;
    cisQualityInfo.dabQualityInfo.audioBer = 0;
    cisQualityInfo.dabQualityInfo.audioCRCError = 0;
    cisQualityInfo.dabQualityInfo.audioCRCTotal = 0;
    cisQualityInfo.dabQualityInfo.ficBer = 0;
    cisQualityInfo.dabQualityInfo.mscBer = 0;
    cisQualityInfo.dabQualityInfo.qualDabTxMode = 0;
    cisQualityInfo.dabQualityInfo.qualServiceBitRate = 0;
    cisQualityInfo.dabQualityInfo.serviceComponent = 0;
    cisQualityInfo.dabQualityInfo.serviceSubCh = 0;
}

void CisCmds::cmostInit(void)
{
    utils = new Utilities();
    fileMngr = new FilesManager();

    cmostParams = GetCmostParameters();
    cisInitQualityInfo();

    vpaOn = true;
}

// To be decomposed in multiple single functions
StatusCodeTy CisCmds::cmostBoot()
{
    StatusCodeTy retValue = CMD_OK;
    int nbLines = 0;

    bootCompleted = false;
    unsigned int tunerAddr;

    for (unsigned int cnt = 0; cnt < (boardDescriptor[cmostParams.cmostBoardType].numTuners); cnt++)
    {
        if (0 == cnt)
        {
            tunerAddr = TUNER1_I2C_ADDR;
        }
        else if (1 == cnt)
        {
            tunerAddr = TUNER2_I2C_ADDR;
        }
        else if (2 == cnt)
        {
            tunerAddr = TUNER3_I2C_ADDR;
        }

        // Read firmware version
        retValue = cisReadFirmwareVersion(tunerAddr);

        if (CMD_OK != retValue)
        {
            // Return errror
            return retValue;
        }

        if (0 == cnt)
        {
            retValue = ReadBootFile(cmostParams.bootFileName_t1, &nbLines);
        }
        else if (1 == cnt)
        {
            retValue = ReadBootFile(cmostParams.bootFileName_t2, &nbLines);
        }
        else if (2 == cnt)
        {
            retValue = ReadBootFile(cmostParams.bootFileName_t3, &nbLines);
        }

        if (CMD_OK != retValue)
        {
            return retValue;
        }

        // Send boot code line by line
        if (CMD_OK != executeBoot(nbLines, true, tunerAddr))
        {
            return retValue;
        }
        else
        {
            bootCompleted = true;
            boot_isOk = false;

            // Check for max 2sec if always wrong exit as wrong
            //  for (int i = 0; i < 20; i++)
            for (int i = 0; i < 50; i++)
            {
                sendDspOutReset(tunerAddr);

                if (true == boot_isOk)
                {
                    retValue = CMD_OK;

                    break;
                }
            }

            if (false == boot_isOk)
            {
                retValue = OPERATION_FAILED;

                emit inShellCmostData_signal("\n[c] Failed CMOST answer to Boot sequence", " ", RADIO_DATA_COMMAND, tunerAddr);
            }
            else
            {
                emit inShellCmostData_signal("\n[c] Correct CMOST answer to Boot sequence", " ", RADIO_DATA_COMMAND, tunerAddr);
            }
        }
    }

    return retValue;
}

bool CisCmds::isUseModeBOARD_MTD_2_CHANNELS()
{
    if (0 == boardDescriptor[cmostParams.cmostBoardType].name.compare(BOARD_MTD_2_CHANNELS))
    {
        return true;
    }
    return false;
}

StatusCodeTy CisCmds::initNotSelectedBand(BandTy currentBand)
{
    StatusCodeTy retValue = CMD_OK;

    unsigned int tunerAddress, tunerChannel;
    unsigned int lastBand, lastFrequency;

    if (BAND_DAB3 == currentBand)
    {
        retValue = GetTunerAddressAndChannel(MODE_FM, &tunerAddress, &tunerChannel);
    }
    else
    {
        retValue = GetTunerAddressAndChannel(MODE_DAB, &tunerAddress, &tunerChannel);
    }

    if (CMD_OK == retValue)
    {
        if (BAND_DAB3 == currentBand)
        {
            retValue = GetTunerCurrentBandAndFrequency(MODE_FM, &lastBand, &lastFrequency);
        }
        else
        {
            retValue = GetTunerCurrentBandAndFrequency(MODE_DAB, &lastBand, &lastFrequency);
        }
    }

    if (CMD_OK == retValue)
    {
        if (BAND_DAB3 == currentBand)
        {
            if (true == vpaOn)
            {
                // Init Tuner1 to Band FM Vpa on ( DAB band is now active)
                retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 01 01 155cc 1a5e0 64");
            }
            else
            {
                // Init Tuner1 to Band FM Vpa off ( DAB band is now active)
                retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 01 00 155cc 1a5e0 64");
            }

            lastBand = (BandTy)BAND_FM;
            SetTunerCurrentBandAndFrequency(MODE_FM, &lastBand, &lastFrequency);
        }
        else
        {
            // Init Tuner2 to Band DAB3 ( FM band Vpa on is now active)
            retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 03 0 2ab50 3a660 1");
            lastBand = (BandTy)BAND_DAB3;
            SetTunerCurrentBandAndFrequency(MODE_DAB, &lastBand, &lastFrequency);
        }
    }

    return retValue;
}

StatusCodeTy CisCmds::cmostPowerOn()
{
    StatusCodeTy retValue;

    if (0 == boardDescriptor[cmostParams.cmostBoardType].name.compare(BOARD_STAR_DCOP))
    {
        // Set SDM and audio interfaces
        retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 04 010002 00", CIS_WAIT_AFTER_CMD_MS);

        if (CMD_OK == retValue)
        {
            // Reset DCOP
            retValue = cisExecute(CIS_RESET_DCOP_ACTION, TUNER1_I2C_ADDR, "RST_DCOP", CIS_WAIT_AFTER_CMD_MS);
        }

        if (CMD_OK == retValue)
        {
            // Set audio interface (05 = analog DAC on and SAI input interface)
            retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 0D 05", CIS_WAIT_AFTER_CMD_MS);
        }

        if (CMD_OK == retValue)
        {
            // Set volume
            retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 15 20c4 20c4 7fffff 4026e7", CIS_WAIT_AFTER_CMD_MS);
        }
    }
    else if (0 == boardDescriptor[cmostParams.cmostBoardType].name.compare(BOARD_MTD_2_CHANNELS) ||
             0 == boardDescriptor[cmostParams.cmostBoardType].name.compare(BOARD_MTD_4_CHANNELS))
    {
        // Enable SAI audio
        retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 04 010000 00", CIS_WAIT_AFTER_CMD_MS);

        if (CMD_OK == retValue)
        {
            // Set audio interface (05 = analog DAC on and SAI input interface)
            retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 0D 05", CIS_WAIT_AFTER_CMD_MS);
        }

        if (CMD_OK == retValue)
        {
            // Set volume
            retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 15 20c4 20c4 7fffff 4026e7", CIS_WAIT_AFTER_CMD_MS);
        }

        if (CMD_OK == retValue)
        {
            // Turn on SDM interface (necessary to give clock to DCOP)
            retValue = cisExecute(CIS_CMD_ACTION, TUNER2_I2C_ADDR, "C 04 000002 00", CIS_WAIT_AFTER_CMD_MS);
        }

        if (CMD_OK == retValue)
        {
            // Reset DCOP
            retValue = cisExecute(CIS_RESET_DCOP_ACTION, TUNER2_I2C_ADDR, "RST_DCOP", CIS_WAIT_AFTER_CMD_MS);
        }
    }
    else
    {
        // Configuration is not supported
        retValue = APPLICATION_NOT_SUPPORTED;
    }

    return retValue;
}

StatusCodeTy CisCmds::cmostPowerOff()
{
    StatusCodeTy retValue;
    unsigned int tunerAddr;
    QString tmpCmd;

    for (unsigned int cnt = 0; cnt < boardDescriptor[cmostParams.cmostBoardType].numTuners; cnt++)
    {
        if (0 == cnt)
        {
            tunerAddr = TUNER1_I2C_ADDR;
        }
        else if (1 == cnt)
        {
            tunerAddr = TUNER2_I2C_ADDR;
        }
        else if (2 == cnt)
        {
            tunerAddr = TUNER3_I2C_ADDR;
        }

        tmpCmd = "C 0A " + QString::number(1, 10) + " 00 0 155cc 1a5e0 64";
        retValue = cisExecute(CIS_CMD_ACTION, tunerAddr, tmpCmd);

        tmpCmd = "C 0A " + QString::number(2, 10) + " 00 0 155cc 1a5e0 64";
        retValue = cisExecute(CIS_CMD_ACTION, tunerAddr, tmpCmd);

        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Band = INVALID_BAND;
        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Frequency = INVALID_FREQUENCY;
        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner1Band = INVALID_BAND;
        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner1Frequency = INVALID_FREQUENCY;
        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner2Band = INVALID_BAND;
        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner2Frequency = INVALID_FREQUENCY;
        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner2Band = INVALID_BAND;
        currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner2Frequency = INVALID_FREQUENCY;

        if (CMD_OK != retValue)
        {
            // Do not loop on other tuners
            break;
        }
    }

    return retValue;
}

StatusCodeTy CisCmds::setBand(BandTy currentBand, SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;

    unsigned int tunerAddress, tunerChannel;
    unsigned int lastBand, lastFrequency;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        retValue = GetTunerCurrentBandAndFrequency(usedMode, &lastBand, &lastFrequency);
        switch (currentBand)
        {
            case BAND_FM:

                // DAC audio ouput is present only on Tuner1, so we redirect the FM signal to it
                retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 14 " + QString::number(tunerChannel, 10), CIS_WAIT_AFTER_CMD_MS);

                if (CMD_OK != retValue)
                {
                    return retValue;
                }

                if (BAND_FM != lastBand)
                {
                    if (true == vpaOn)
                    {
                        retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 01 01 155cc 1a5e0 64");
                    }
                    else
                    {
                        retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 01 0 155cc 1a5e0 64");
                    }

                    lastBand = BAND_FM;
                    SetTunerCurrentBandAndFrequency(usedMode, &lastBand, &lastFrequency);
                }
                break;

            case BAND_AM:
                // DAC audio ouput is present only on Tuner1, so we redirect the AM signal to it
                retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 14 " + QString::number(tunerChannel, 10), CIS_WAIT_AFTER_CMD_MS);

                if (CMD_OK != retValue)
                {
                    return retValue;
                }

                if (BAND_MW != lastBand)
                {
                    if (USA == cmostParams.countryName)
                    {
                        retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 05 0 212 6AE 10");
                    }
                    else
                    {
                        retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 05 0 20a 65d 9");
                    }
                    lastBand = BAND_MW;
                    SetTunerCurrentBandAndFrequency(usedMode, &lastBand, &lastFrequency);
                }

                break;

            case BAND_DAB3:
                // DAC audio ouput is present only on Tuner1, so we redirect the DAB signal to it
                retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 14 04", CIS_WAIT_AFTER_CMD_MS);

                if (CMD_OK != retValue)
                {
                    return retValue;
                }

                if (BAND_DAB3  != lastBand)
                {
                    retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 0A " + QString::number(tunerChannel, 10) + " 03 0 2ab50 3a660 1");
                    lastBand = BAND_DAB3;
                    SetTunerCurrentBandAndFrequency(usedMode, &lastBand, &lastFrequency);
                }
                break;

            default:
                // No code
                break;
        }
    }

    return retValue;
}

CountryNameTy CisCmds::getCmostCountry()
{
    return cmostParams.countryName;
}

StatusCodeTy CisCmds::GetTunerAddressAndChannel(SystemMode usedMode, unsigned int* tunerAddress,
                                                unsigned int* tunerChannel, bool primary)
{
    StatusCodeTy retValue = WRONG_PARAMETERS;

    if (MODE_FM == usedMode || MODE_AM == usedMode)
    {
        if (true == primary)
        {
            *tunerAddress = boardDescriptor[cmostParams.cmostBoardType].fmAmPrimaryTunerAddress;
            *tunerChannel = boardDescriptor[cmostParams.cmostBoardType].fmAmPrimaryChannelNumber;

            retValue = CMD_OK;
        }
    }
    else if (MODE_DAB == usedMode)
    {
        if (true == primary)
        {
            *tunerAddress = boardDescriptor[cmostParams.cmostBoardType].dabPrimaryTunerAddress;
            *tunerChannel = boardDescriptor[cmostParams.cmostBoardType].dabPrimaryChannelNumber;

            retValue = CMD_OK;
        }
    }

    return retValue;
}

StatusCodeTy CisCmds::GetTunerCurrentBandAndFrequency(SystemMode usedMode,
                                                      unsigned int* currentBand,
                                                      unsigned int* currentFrequency, bool primary)
{
    StatusCodeTy retValue = WRONG_PARAMETERS;

    if (MODE_FM == usedMode || MODE_AM == usedMode)
    {
        if (true == primary)
        {
            *currentBand = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Band;
            *currentFrequency = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Frequency;

            retValue = CMD_OK;
        }
    }
    else if (MODE_DAB == usedMode)
    {
        if (true == primary)
        {
            if (TUNER1_I2C_ADDR == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryTunerAddress)
            {
                if (1 == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryChannelNumber)
                {
                    *currentBand = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Band;
                    *currentFrequency = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Frequency;
                }
                else
                {
                    *currentBand = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner1Band;
                    *currentFrequency = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner1Frequency;
                }
            }
            else if (TUNER2_I2C_ADDR == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryTunerAddress)
            {
                if (1 == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryChannelNumber)
                {
                    *currentBand = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner2Band;
                    *currentFrequency = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner2Frequency;
                }
                else
                {
                    *currentBand = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner2Band;
                    *currentFrequency = currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner2Frequency;
                }
            }
            retValue = CMD_OK;
        }
    }

    return retValue;
}

StatusCodeTy CisCmds::SetTunerCurrentBandAndFrequency(SystemMode usedMode,
                                                      unsigned int* currentBand,
                                                      unsigned int* currentFrequency, bool primary)
{
    StatusCodeTy retValue = WRONG_PARAMETERS;

    if (MODE_FM == usedMode || MODE_AM == usedMode)
    {
        if (true == primary)
        {
            currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Band = *currentBand;
            currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Frequency = *currentFrequency;

            retValue = CMD_OK;
        }
    }
    else if (MODE_DAB == usedMode)
    {
        if (true == primary)
        {
            if (TUNER1_I2C_ADDR == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryTunerAddress)
            {
                if (1 == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryChannelNumber)
                {
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Band = *currentBand;
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner1Frequency = *currentFrequency;
                }
                else
                {
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner1Band = *currentBand;
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner1Frequency = *currentFrequency;
                }
            }
            else if (TUNER2_I2C_ADDR == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryTunerAddress)
            {
                if (1 == boardDescriptor[cmostParams.cmostBoardType].dabPrimaryChannelNumber)
                {
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner2Band = *currentBand;
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chAtuner2Frequency = *currentFrequency;
                }
                else
                {
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner2Band = *currentBand;
                    currentChBandFrequencyStatus[cmostParams.cmostBoardType].chBtuner2Frequency = *currentFrequency;
                }
            }
            retValue = CMD_OK;
        }
    }

    return retValue;
}

StatusCodeTy CisCmds::getCurrentFrequency(SystemMode usedMode, unsigned int* activeFrequency)
{
    unsigned int activeBand;
    StatusCodeTy retValue = CMD_OK;

    retValue = GetTunerCurrentBandAndFrequency(usedMode, &activeBand, activeFrequency);

    return retValue;
}

StatusCodeTy CisCmds::setCurrentFrequency(SystemMode usedMode, unsigned int* activeBand, unsigned int* activeFrequency)
{
    StatusCodeTy retValue = CMD_OK;

    retValue = SetTunerCurrentBandAndFrequency(usedMode, activeBand, activeFrequency);

    return retValue;
}

StatusCodeTy CisCmds::setFrequency(quint32 currentFreq, SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 08 " + QString::number(tunerChannel, 10) + " " + QString::number(currentFreq, 16) + " 0");
    }

    return retValue;
}

StatusCodeTy CisCmds::executeBoot(int listSize, bool isCmosBoot, unsigned int tunerAddr)
{
    Q_UNUSED(isCmosBoot);

    StatusCodeTy retValue = CMD_OK;

    if (listSize == 0)
    {
        qDebug() << "ERROR: Empty script";

        return OPERATION_FAILED;
    }

    // Go throught command list
    int lineIndex = 0;

    emit inShellCmostData_signal("\n[c] Cis BOOT...", " ", RADIO_DATA_COMMAND, tunerAddr);

    while ((lineIndex < listSize) && (retValue == CMD_OK))
    {
        QString command = bootCmdList.at(lineIndex);

        CisCommandCodeTy cmdType = getCommandType(command);

        activeCisDataType = cmdType;

        retValue = cisExecute(CIS_BOOT_ACTION, tunerAddr, command, 1); // Wait 1ms after command

        lineIndex++;
    }

    emit inShellCmostData_signal("\n[c] BOOT OK", " ", RADIO_DATA_COMMAND, tunerAddr);

    return retValue;
}

CisCommandCodeTy CisCmds::getCommandType(QString command)
{
    CisCommandCodeTy type;

    command = command.remove(QChar(' '), Qt::CaseInsensitive);
    command = command.simplified();
    command = command.toUpper();

    if (command.simplified() == (QString("RST")))
    {
        type = RESET_IC;
    }
    else if (command.simplified() == (QString("ABN")))
    {
        type = ABNORMAL;
    }
    else if (command.simplified() == QString("BOOT"))
    {
        type = BOOT_SEQ;
    }
    else if (command.startsWith(QString("C")))
    {
        type = CMD_TYPE;
    }
    else if (command.startsWith(QString("R")))
    {
        type =  READ_TYPE;
    }
    else if (command.startsWith(QString("W")))
    {
        type = WRITE_TYPE;
    }
    else
    {
        type = UNKNOWN_TYPE;
    }

    return type;
}

QByteArray CisCmds::processCisCommand(QString command, CisCommandCodeTy type)
{
    QString cmd = command.simplified(); // Remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '

    cmd = cmd.right(cmd.size() - 2);
    QStringList arguments = cmd.split(" ", QString::SkipEmptyParts);
    int cntIni = 0;

    if (type == CMD_TYPE)
    {
        arguments[0] = "00" + arguments[0] + utils->formatDataNoSpace((quint8)(arguments.size() - 1));
        cntIni = 1;
    }

    for (int cnt = cntIni; cnt < arguments.size(); cnt++)
    {
        while (arguments[cnt].length() < 6)
        {
            arguments[cnt] = "0" + arguments[cnt];
        }
    }

    cmd.clear();

    for (int cnt = 0; cnt < arguments.size(); cnt++)
    {
        cmd = cmd + arguments[cnt];
    }

    cmd = utils->insertSpaces(cmd, 2); // Insert spaces every two characters
    arguments = cmd.split(" ", QString::SkipEmptyParts);

    // Encoding command payload
    bool oK;
    QByteArray payload;

    for (int i = 0; i < arguments.size(); i++)
    {
        payload.append((quint8)arguments.at(i).toInt(&oK, 16));
    }

    return payload;
}

// Parser of CIS command
StatusCodeTy CisCmds::cisCommandEncoder(CisCommandCodeTy type, QString command,
                                        QByteArray* encodedCmd, unsigned int tunerAddr)
{
    #define CMOST_DIRECT_LUN                0xFF
    #define CMOST_0xFE_LUN                  0xFE

    QByteArray encoded = 0;

    // Check if command line is empty then skip it
    if (command.isEmpty() || command.startsWith("\\"))
    {
        *encodedCmd = 0;

        return CMD_SINTAX_ERROR;
    }

    // Parse the command string
    command = command.toLower();
    command = command.simplified(); // Remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '

    // Prepare HEADER
    QByteArray header;
    header[Sync_Byte] = TARGET_MESSAGE_SYNC_BYTE;
    header[Lun_Byte] = CMOST_DIRECT_LUN;

    switch (type)
    {
        // CMOST API command type
        case CMD_TYPE:
        {
            QByteArray payload = processCisCommand(command, CMD_TYPE);

            // Calculate command payload lenght (bytes) for the header
            quint32 size = (quint32)payload.size();
            quint16 payLenght = (quint16)(size & 0x0000FFFF);

            header[CommandType_Byte]       = CMD_TYPE;
            header[FormatType_Byte]        = FORMAT_24bit;
            header[I2cAddress_Byte]        = tunerAddr;
            header[Spare_Byte]             = 0x00;
            header[PayloadLenghtMsb_Byte]  = (quint8)(payLenght >> 8);
            header[PayloadLenghtLsb_Byte]  = (quint8)(payLenght & 0xFF);

            encoded = header + payload;
        }
        break;

        // Read Direct command type
        case READ_TYPE:
        {
            QByteArray payload = processCisCommand(command, READ_TYPE);

            // Calculate command payload lenght in bytes for the header
            quint32 size = (quint32)payload.size();
            quint16 payLenght = (quint16)(size & 0x0000FFFF);

            header[CommandType_Byte]       = READ_TYPE;
            header[FormatType_Byte]        = FORMAT_32bit;
            header[I2cAddress_Byte]        = tunerAddr;
            header[Spare_Byte]             = 0x00;
            header[PayloadLenghtMsb_Byte]  = (quint8)(payLenght >> 8);
            header[PayloadLenghtLsb_Byte]  = (quint8)(payLenght & 0xFF);

            encoded = header + payload;
        }
        break;

        // Write Direct command type
        case WRITE_TYPE:
        {
            QByteArray payload = processCisCommand(command, WRITE_TYPE);

            // Calculate command payload lenght (bytes) for the header
            quint32 size = (quint32)payload.size();
            quint16 payLenght = (quint16)(size & 0x0000FFFF);

            header[CommandType_Byte]       = WRITE_TYPE;          // 0x06
            header[FormatType_Byte]        = FORMAT_32bit;        // 0x00
            header[I2cAddress_Byte]        = tunerAddr;
            header[Spare_Byte]             = 0x00;
            header[PayloadLenghtMsb_Byte]  = (quint8)(payLenght >> 8);
            header[PayloadLenghtLsb_Byte]  = (quint8)(payLenght & 0xFF);

            // Encoded command
            encoded = header + payload;
        }
        break;

        case RESET_IC:
        {
            // RESET Command has Only HEADER, no Payload
            header[CommandType_Byte]       = RESET_IC;          // [0x07]
            header[FormatType_Byte]        = FORMAT_24bit;      // [0x01]
            header[I2cAddress_Byte]        = tunerAddr;
            header[Spare_Byte]             = 0x00;              // [0x00]
            header[PayloadLenghtMsb_Byte]  = 0x00;              // [0x00]
            header[PayloadLenghtLsb_Byte]  = 0x00;              // [0x00]

            // Encoded command
            encoded = header;
        }
        break;

        case RESET_OTHER_DEVICE:
        {
            header[Sync_Byte]              = PROTOCOL_LAYER_SYNC_BYTE;
            header[Lun_Byte]               = CMOST_0xFE_LUN;     // Lun = 0xFE
            header[CommandType_Byte]       = RESET_OTHER_DEVICE; // 0x09
            header[FormatType_Byte]        = 0x59;   //(quint8)((plDcopPort & 0xFF00)>>8);                     //23000 MSB
            header[I2cAddress_Byte]        = 0xD8;   //(quint8)((plDcopPort & 0xFF)>>0); // 23000 LSB

            header[Spare_Byte]             = 0x00;               // [0x00]
            header[Spare_Byte]             = 0x00;
            header[PayloadLenghtMsb_Byte]  = 0x00;               // [0x00]
            header[PayloadLenghtLsb_Byte]  = 0x00;               // [0x00]

            // Encoded command
            encoded = header;
        }
        break;

        case ABNORMAL:
        {
            header[Sync_Byte]              = 0x2D;
            header[Lun_Byte]               = 0xFE;
            header[CommandType_Byte]       = 0x07;
            header[FormatType_Byte]        = 0x5D;
            header[I2cAddress_Byte]        = 0xC0;
            header[Spare_Byte]             = 0x00;
            header[PayloadLenghtMsb_Byte]  = 0x00;
            header[PayloadLenghtLsb_Byte]  = 0x00;

            // Encoded command
            encoded = header;
        }
        break;

        default:
            // No code
            break;
    }

    *encodedCmd = encoded;

    return CMD_OK;
}

// Read boot fileName and Cmost Board type to use
cmostParametersTy CisCmds::GetCmostParameters()
{
    // Get only Boot File name for the moment
    cmostParametersTy cmostParams;
    QStringList lst;

    lst = fileMngr->LoadFile(CMOST_PARAMS_FILE_NAME);

    // Remove comments
    for (int cnt = 0; cnt < lst.length(); cnt++)
    {
        if (true == lst.at(cnt).startsWith("#"))
        {
            lst.removeAt(cnt);
        }
    }

    // Clear variables
    cmostParams.bootFileName_t1.clear();
    cmostParams.bootFileName_t2.clear();
    cmostParams.bootFileName_t3.clear();
    cmostParams.cmostBoardType = 0;
    cmostParams.countryName = Europe;

    // Check if we have elements
    if (false == lst.isEmpty())
    {
        fileMngr->TokenizeFile("=", lst);

        //ChunkTy chunk;
        QList<ChunkTy> chunkList = fileMngr->GetChunkListObj();
        QListIterator<ChunkTy> it(chunkList);

        while (it.hasNext())
        {
            // Load next element
            ChunkTy chunk = it.next();

            if (0 == chunk.key.compare(BOOTFILE_T1, Qt::CaseInsensitive))
            {
                //Set boot file name and path
                cmostParams.bootFileName_t1 = chunk.value;
            }
            else if (0 == chunk.key.compare(BOOTFILE_T2, Qt::CaseInsensitive))
            {
                //Set boot file name and path
                cmostParams.bootFileName_t2 = chunk.value;
            }
            else if (0 == chunk.key.compare(BOOTFILE_T3, Qt::CaseInsensitive))
            {
                //Set boot file name and path
                cmostParams.bootFileName_t3 = chunk.value;
            }
            else if (true == chunk.key.contains(BOARD, Qt::CaseInsensitive) &&
                     (0 == chunk.value.compare("Y", Qt::CaseInsensitive)))
            {
                for (int cnt = 0; cnt < BOARD_SUPPORTED_CONF_NUM; cnt++)
                {
                    if (0 == chunk.key.compare(boardDescriptor[cnt].name, Qt::CaseInsensitive))
                    {
                        // Set current board type
                        cmostParams.cmostBoardType = cnt;

                        // Exit for loop
                        break;
                    }
                }
            }
            else if (0 == chunk.key.compare(COUNTRY, Qt::CaseInsensitive))
            {
                // Get current country
                if (0 == chunk.value.compare(tr("EUROPE"), Qt::CaseInsensitive))
                {
                    cmostParams.countryName = Europe;
                }
                else if (0 == chunk.value.compare(tr("USA"), Qt::CaseInsensitive))
                {
                    cmostParams.countryName = USA;
                }
                else if (0 == chunk.value.compare(tr("JAPAN"), Qt::CaseInsensitive))
                {
                    cmostParams.countryName = Japan;
                }
                else if (0 == chunk.value.compare(tr("EASTEUROPE"), Qt::CaseInsensitive))
                {
                    cmostParams.countryName = EastEurope;
                }
                else if (0 == chunk.value.compare(tr("CHINA"), Qt::CaseInsensitive))
                {
                    cmostParams.countryName = China;
                }
                else if (0 == chunk.value.compare(tr("CANADA"), Qt::CaseInsensitive))
                {
                    cmostParams.countryName = Canada;
                }
                else if (0 == chunk.value.compare(tr("KOREA"), Qt::CaseInsensitive))
                {
                    cmostParams.countryName = Korea;
                }
            }
        }
    }

    return cmostParams;
}

void CisCmds::cis_receivedDataFromDispatcher_slot(QByteArray data)
{
    switch (activeCisDataType)
    {
        case RESET_IC:
            if (((quint8)data[7] == 0x01) && ((quint8)data[8] == 0x07) &&
                ((quint8)data[2] == 0x00) && ((quint8)data[3] == 0x00) &&
                ((quint8)data[4] == 0x00) && ((quint8)data[5] == 0x00))
            {
                // Correct Answer to Reset Cmost
                emit inShellCmostData_signal("\n[i] CIS Reset Response OK:", utils->processTextFormat(data, HEX_FORMAT), RADIO_DATA_RESPONSE, 0);
            }
            break;

        case READ_TYPE:
            emit inShellCmostData_signal("\n[r] CIS Read Response: ", utils->processTextFormat(data, HEX_FORMAT), RADIO_DATA_RESPONSE, 0);

            // TODO: Save info of specific commands (get firmware version, get quality etc.)
            SaveCisSpecificCmdData(data, activeCisActionType);
            break;

        case CMD_TYPE:
            emit inShellCmostData_signal("\n[r] CIS Cmd Response: ", utils->processTextFormat(data, HEX_FORMAT), RADIO_DATA_RESPONSE, 0);

            // TODO: Save info of specific commands (get firmware version, get quality etc.)
            SaveCisSpecificCmdData(data, activeCisActionType);
            break;

        case WRITE_TYPE:
            break;

        default:
            // No code
            break;
    }

    isCisNotifStatusReceived = true;
    isCisRespReceived = true;
}

void CisCmds::SaveCisSpecificCmdData(QByteArray data, quint8 actionType)
{
    switch (actionType)
    {
        case CIS_GET_RDS_ACTION:
            if (data.length() >= 9)
            {
                rdsBuffersList.append(data.mid(8, -1));
            }
            break;

        case CIS_READ_FW_VERSION_ACTION:
            break;

        case CIS_READ_ACTION:
            if (data.length() >= 11)
            {
                if (((quint8)data[7] == 0x04) && ((quint8)data[8] == 0xAF) &&
                    ((quint8)data[9] == 0xFE) && ((quint8)data[10] == 0x42) &&
                    ((quint8)data[2] == 0x00) && ((quint8)data[3] == 0x00) &&
                    ((quint8)data[4] == 0x00) && ((quint8)data[5] == 0x00))
                {
                    boot_isOk = true;
                }
            }
            break;

        case CIS_CMD_ACTION:
            if (data.length() >= 9)
            {
                getCisCmdAnswer(data.mid(8, -1));
            }
            break;

        case CIS_SETBAND_ACTION:
            break;

        case CIS_SETFREQUENCY_ACTION:
            break;

        case CIS_SEEK_RESULT_ACTION:
            if (data.length() >= 9)
            {
                getCisSeekResult(data.mid(8, -1));
            }
            break;

        default:
            // No code
            break;
    }
}

void CisCmds::getCisCmdAnswer(QByteArray data)
{
    if (data.length() < 12)
    {
        return;
    }

    if ((((quint8)data[1] == 0x12) || ((quint8)data[1] == 0x13)) && ((quint8)data[2] == 0x03))
    {
        // If it is answer to get DAB quality
        if ((quint8)data[1] == 0x13)
        {
            qDebug() << "cisQualityInfo.qualFstRf = " << cisQualityInfo.qualFstRf;

            cisQualityInfo.qualFstRf = (quint8)data[3];
        }
        else if ((quint8)data[1] == 0x12)
        {
            // Save all quality infos
            cisQualityInfo.qualFstRf = (quint8)data[3];
            cisQualityInfo.qualFstBb = (quint8)data[4];
            cisQualityInfo.fmQualityInfo.qualDetune = (quint8)data[5];
            cisQualityInfo.fmQualityInfo.qualMultiPath = (quint8)data[6];
            cisQualityInfo.fmQualityInfo.qualMpxNoise = (quint8)data[7];
            cisQualityInfo.fmQualityInfo.qualSNR = (quint8)(0.56 + 0.39 * (quint8)data[8]);
            cisQualityInfo.fmQualityInfo.qualAdj = (quint8)data[9];
            cisQualityInfo.fmQualityInfo.qualCoChannel = (quint8)data[10];
            cisQualityInfo.fmQualityInfo.qualDeviation = (quint8)((data[11] >> 1) & 0x7F);
            cisQualityInfo.fmQualityInfo.qualStereo = (quint8)(data[11] & 0x01);
        }
    }
}

void CisCmds::getCisSeekResult(QByteArray data)
{
    // Check if the command length is valid
    if (data.length() < 6)
    {
        return;
    }

    // Refresh tune frequency with the returned freq value (TUNER_AMFM_GET_SEEK_STATUS)
    if (((quint8)data[1] == 0x28) && (((quint8)data[2] == 0x03) || ((quint8)data[2] == 0x04)))
    {
        // Set answer received
        seekfmParams.isAnswerReceived = true;

        // Save new frequency
        seekfmParams.reachedFreq = (((data[3] << 16) & 0x1F0000) | ((data[4] << 8) & 0xFF00) | ((data[5]) & 0xFF));

        // Check if good station found
        if ((quint8)data[3] & 0x80)
        {
            seekfmParams.isValidStation = true;
            activeCisActionType = CIS_GET_RDS_ACTION;
        }
        else if ((quint8)(data[3] & 0x40) == 0x40)
        {
            // If reached fullSeekCycle
            seekfmParams.isFullSeekCycle = true;
        }
    }
}

void CisCmds::sendCmdReservedForProtocolLayer(quint8 cmdtype, quint8 specialVal, quint32 portVal)
{
    QByteArray parseData;

    parseData[0] = (quint8)0x2D;
    parseData[1] = (quint8)0xFE;
    parseData[2] =  cmdtype;
    parseData[3] = (quint8)((portVal & 0xFF00) >> 8);
    parseData[4] = (quint8)((portVal & 0xFF) >> 0);

    if (cmdtype == PROT_LAYER_SPECIALIZE_DEV_CONN)
    {
        parseData[5] = specialVal;
    }
    else
    {
        parseData[5] = (quint8)0x00;
    }

    parseData[6] = (quint8)0x00;
    parseData[7] = (quint8)0x00;

    emit cis_sendCmdToDispatcher_signal(parseData);
}

void CisCmds::sendDspOutReset(unsigned int tunerAddress)
{
    cisExecute(CIS_READ_ACTION, tunerAddress, "R 20180 1");
}

StatusCodeTy CisCmds::cisUnMute(int muteStatus, SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        retValue = cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 16 " + QString::number(muteStatus, 10));
        //  retValue = cisExecute (CIS_CMD_ACTION, tunerAddress, "C 16 "+ QString::number(muteStatus, 10));
    }

    return retValue;
}

void CisCmds::cisSetVolume(int rVolume, SystemMode usedMode)
{
    int rVolumeAnalogInt, rVolumeDigInt;
    double dccc3, dccc4;

    // Displayed Volume Level range 0 to 24
    // Effective Normalized Volume level (range -142 to + 0 dB) using lockup table via CIS command 0x15
    QList<int> volumeTableList;
    volumeTableList << -100 << -50 << -40 << -35 << -30
                    << -25 << -20 << -18 << -16 << -15
                    << -14 << -13 << -12 << -11 << -10
                    << -9 << -8 << -7 << -6 << -5
                    << -4 << -3 << -2 << -1 << 0;

    rVolumeAnalogInt = volumeTableList[rVolume];
    rVolumeDigInt = rVolumeAnalogInt - 6;

    // Cmd 0x15: For the moment Analog level (FM-AM) kept equal to
    // Digital level (DAB). The other two parameters (Time Analog to Dig and Time Dig to Analog) are kept fix = 21.93ms
    dccc3 = pow(10, double (rVolumeAnalogInt) / 20.0);
    dccc4 = pow(10, double (rVolumeDigInt) / 20.0);

    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        // For the moment control only volume level of Tuner 1 DAC audio
        cisExecute(CIS_CMD_ACTION, TUNER1_I2C_ADDR, "C 15 20C4 20C4 " +  QString::number(utils->ConvFloat2Hex(dccc3), 16) + " " +
                   QString::number(utils->ConvFloat2Hex(dccc4), 16));
    }
}

quint32 CisCmds::getFmFreqFromIndex(int freqIdx)
{
    quint32 fmFr = 0;

    fmFr = 87500 + 100 * freqIdx;

    return fmFr;
}

quint32 CisCmds::getMwFreqFromIndex(int freqIdx)
{
    quint32 mwFr = 0;

    if (cmostParams.countryName == USA)
    {
        mwFr =  530 + 10 * freqIdx;
    }
    else
    {
        mwFr =  522 + 9 * freqIdx;
    }

    return mwFr;
}

CisSeekParamsTy CisCmds::GetSeekFmStatus(int readCmd28DlyValue, SystemMode usedMode)
{
    quint8 instID = 1;
    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    if (readCmd28DlyValue > 0)
    {
        QThread::msleep(readCmd28DlyValue);
    }

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        cisExecute(CIS_SEEK_RESULT_ACTION, tunerAddress, "c 28 " + QString::number(instID, 10));
    }
    while (seekfmParams.isAnswerReceived == false)
    { }

    return seekfmParams;
}

void CisCmds::SeekFmAuto(bool updirection, int nbSeekMeasures, int stepjump, SystemMode usedMode, quint32 currSeekFreq)
{
    quint8 param2;
    quint8 instID = 1;

    seekfmParams.isAnswerReceived = false;
    seekfmParams.isFullSeekCycle = false;
    seekfmParams.isValidStation = false;
    seekfmParams.reachedFreq = currSeekFreq;

    if (false == updirection)
    {
        param2 = 3; // Seek down bit1 = 1
    }
    else
    {
        param2 = 1; // Seekup bit1 = 0
    }

    param2 |= ((nbSeekMeasures - 1) & 0x0F) << 4; // Add (nb_samples-1) to measure

    param2 |= 0x04;  // Stay in Mute

    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        cisExecute(CIS_SEEK_START_ACTION, tunerAddress, "C 26 " + QString::number(instID, 10) + " " +
                   QString::number(param2, 16) + " " +
                   QString::number(stepjump, 16));
    }
}

StatusCodeTy CisCmds::sendSeekEndCmd(SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);
    if (CMD_OK == retValue)
    {
        retValue = cisExecute(CIS_CMD_ACTION, tunerAddress, "C 27 " + QString::number(tunerChannel, 10) + " 00");
    }
    return retValue;
}

StatusCodeTy CisCmds::SeekDabManual(quint32 currentFreq, SystemMode usedMode, bool updirection)
{
    QString cmd;

    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        if (true == updirection)
        {
            cmd = "C 26 " + QString::number(tunerChannel, 10) + " 0c " + QString::number(currentFreq, 16);
        }
        else
        {
            cmd = "C 26 " + QString::number(tunerChannel, 10) + " 0e " + QString::number(currentFreq, 16);
        }
        retValue = cisExecute(CIS_SEEK_START_ACTION, tunerAddress, cmd);
    }

    return retValue;
}

qualityInfoTy CisCmds::getFmQualityParameters(SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;

    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        cisExecute(CIS_CMD_ACTION, tunerAddress, "C 12 " + QString::number(tunerChannel, 10), 0);
    }

    return cisQualityInfo;
}

qualityInfoTy CisCmds::getDabFieldStrength(SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

    if (CMD_OK == retValue)
    {
        cisExecute(CIS_CMD_ACTION, tunerAddress, "C 13 " + QString::number(tunerChannel, 10), 0);
    }

    return cisQualityInfo;
}

bool CisCmds::StopRdsAcquisition()
{
    isRdsOn = false;

    return true;
}

bool CisCmds::RunRdsAcquisition(SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    if (false == isRdsOn)
    {
        isRdsOn = true;

        retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);
        if (CMD_OK == retValue)
        {
            // Accept only 0 Error RDS data blocks
            cisExecute(CIS_CMD_ACTION, tunerAddress, "C 1A " + QString::number(tunerChannel, 10) + " 000201");
        }
    }

    return true;
}

QByteArray CisCmds::GetRdsData()
{
    // Return the RDS raw data (NULLPTR if List is empty)
    if (true == rdsBuffersList.isEmpty())
    {
        return nullptr;
    }
    else
    {
        return rdsBuffersList.takeFirst();
    }
}

void CisCmds::RdsDataPolling(SystemMode usedMode)
{
    StatusCodeTy retValue = CMD_OK;
    unsigned int tunerAddress, tunerChannel;

    if (true == isRdsOn)
    {
        retValue = GetTunerAddressAndChannel(usedMode, &tunerAddress, &tunerChannel);

        if (CMD_OK == retValue)
        {
            if ((activeCisActionType != CIS_SEEK_START_ACTION) &&
                (activeCisActionType != CIS_SEEK_RESULT_ACTION))
            {
                cisExecute(CIS_GET_RDS_ACTION, tunerAddress, "C 1B " + QString::number(tunerChannel, 10));
            }
        }
    }
}

void CisCmds::Learn(BandTy currentBand, quint32 currentFreq, SystemMode usedMode)
{
    Q_UNUSED(currentBand);
    Q_UNUSED(currentFreq);
    Q_UNUSED(usedMode);
}

// End of file
