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
#ifndef COMMON_H
#define COMMON_H

#include <QtGui>
#include <QDebug>
#include <QtDebug>
#include <QtCore/QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QGraphicsOpacityEffect>

#include "utilities.h"

// =============================
//     APPLICATION DEFINES
// =============================
#define SEPARATOR_STRING                " "
#define SPACE_STRING                    " "
#define NEWLINE_CHARACTER               '\n'
#define DOCS_FOLDER_NAME                "docs/"
#define LOGS_FOLDER_NAME                "logs/"
#define TOOLS_FOLDER_NAME               "tools/"
#define SETTINGS_FOLDER_NAME            "settings/"
#define SCRIPTS_FOLDER_NAME             "scripts/"

#define MAX_FILE_LINE_LENGTH            255

// ================================
//     PROTOCOL LAYER DEFINES
// ================================
// FILES
#define PROTOCOL_LAYER_FILE_NAME        "MDR_Protocol.exe"
#define TASK_KILLER_FILE_NAME           "taskkill.exe"
#define PROTOCOL_CONFIG_FILE_NAME       "settings/protocol_layer_config.cfg"
#define PARAMETERS_FILE_NAME            "settings/parameters.cfg"
#define CMOST_PARAMS_FILE_NAME          "settings/cmost_config.cfg"

// TIMEOUTs
#define TCP_PROTOCOL_LAYER_RESPONSE_TIMEOUT     500
#define TCP_PROTOCOL_LAYER_CONNECTION_TIMEOUT   1000
// #define TCP_PROTOCOL_LAYER_ADDRESS              "127.0.0.1"
// #define TCP_DCOP_PROTOCOL_LAYER_PORT            23000
// #define TCP_CMOST_PROTOCOL_LAYER_PORT           24000
#define TCP_CTRL_PORT                           25000
#define RECONNECTION_RETRIES                    2
#define DEFAULT_PROCESS_START_TIMEOUT           2000

#define PROT_LAYER_EXIT                         1
#define PROT_LAYER_CABLE_CHECK                  7
#define PROT_LAYER_SPECIALIZE_DEV_CONN          8
#define PROT_LAYER_RESET_OTHER_DEVICE           9

// =============================
//     MIDDLEWARE
// =============================
#define COMMAND_COMMON_HEADER_LENGTH        8
#define TARGET_MESSAGE_SYNC_BYTE            0x1D
#define PROTOCOL_LAYER_SYNC_BYTE            0x2D
#define COMMAND_SYNC_FIELD                  0
#define COMMAND_PROTOCOL_FAMILY_FIELD       1
#define COMMAND_PROTOCOL_LUN_FIELD          1
#define COMMAND_MESSAGE_ID_FIELD            2
#define COMMAND_DATA_LENGTH_FIELD           6
#define COMMAND_PAYLOAD_FIELD               8
#define MDW_COMMAND_TIMEOUT                 5000 // ms

enum MdwCommandCodeTy
{
    MIDW_GET_ALIVE_CHK                  = 0x000,
    MIDW_POWER_UP                       = 0x001,
    MIDW_GET_FIRMW_VER                  = 0x002,
    MIDW_GET_DAB_INFO                   = 0x003,
    MIDW_POWER_DOWN                     = 0x004,
    MIDW_GET_AMFM_INFO                  = 0x005,
    MIDW_GET_DAB_BER_MONITORING         = 0x006,

    MIDW_SETUP_EVENT_NOTIFICATIONS      = 0x010,

    MIDW_DUMP_DATABASE                  = 0x021,
    MIDW_TUNER_TABLE_WRITE              = 0x022,
    MIDW_TUNER_TABLE_DUMP               = 0x023,
    MIDW_TUNER_TABLE_CLEAR              = 0x024,
    MIDW_ALL_TUNER_TABLE_CLEAR          = 0x025,
    MIDW_CLEAR_DATABASE                 = 0x026,
    MIDW_CLEAR_ENSEMBLE_DATA            = 0x027,
    ERASE_NVM_DATA                      = 0x028,

    MIDW_GET_RAW_RDS                    = 0x030,
    MIDW_GET_RDS_PS                     = 0x031,
    MIDW_GET_RDS_DI                     = 0x032,
    MIDW_GET_RDS_PI                     = 0x033,
    MIDW_GET_RDS_PTY                    = 0x034,
    MIDW_GET_RDS_TP_TA                  = 0x035,
    MIDW_GET_RDS_PTY_TP_TA_MS           = 0x036,
    MIDW_GET_RDS_TIME                   = 0x037,
    MIDW_GET_RDS_RT                     = 0x038,
    MIDW_GET_RDS_AF_LIST                = 0x039,
    MIDW_SET_RDS_TH                     = 0x03A,
    MIDW_GET_RDS_EON                    = 0x03B,

    MIDW_GET_RDS_OFF                    = 0x03F,
    MIDW_GET_ENSEMBLE_LIST              = 0x040,
    MIDW_GET_ENSEMBLE_DATA              = 0x041,
    MIDW_GET_FIC                        = 0x042,
    MIDW_GET_SERVICE_LIST               = 0x043,
    MIDW_GET_SPECIFIC_SERVICE_DATA      = 0x044,
    MIDW_GET_TIME                       = 0x045,
    MIDW_GET_TMC_SERVICE_LIST           = 0x046,
    MIDW_GET_TMC_SERVICE_DATA           = 0x047,
    MIDW_GET_FIDC_DATA                  = 0x048,
    MIDW_GET_USER_DATA                  = 0x049,

    MIDW_GET_PAD_DLS                    = 0x051,
    MIDW_SETUP_PAD_DL_PLUS              = 0x052,
    MIDW_GET_PAD_DL_PLUS                = 0x053,

    MIDW_TUNE_FREQUENCY                 = 0x060,
    MIDW_SERVICE_SELECT                 = 0x061,
    MIDW_SEEK                           = 0x062,
    MIDW_SCAN                           = 0x063,
    MIDW_LEARN                          = 0x064,
    MIDW_SEL_AUDIO_OUT                  = 0x065,
    MIDW_DABFM_SWITCHING                = 0x066,
    MIDW_TUNE_DAB_CHANNEL               = 0x067,
    MIDW_GET_SERVICE_QUALITY            = 0x068,
    MIDW_SETUP_AM_FM_TH                 = 0x069,
    MIDW_SETUP_SERVICE_FOLLOWING        = 0x06A,
    MIDW_TUNE_NEXT_FREQ                 = 0x06B,
    MIDW_SETUP_AM_FM_BAND_DATA          = 0x06C,
    MIDW_GET_CURR_ENSEMBLE              = 0x06D,
    MIDW_SET_TUNE_TIMEOUT               = 0x06E,

    MIDW_MUTE                           = 0x082,

    MIDW_SET_ERROR_CONCEALMENT_PARAMS   = 0x085,
    MIDW_GET_DYN_RANGE_CORRECTION       = 0x086,
    MIDW_SET_DYN_RANGE_CORRECTION       = 0x087,

    MIDW_GET_FM_STEREO_INDICATION       = 0x089,

    MIDW_GET_MOT                        = 0x100,
    MIDW_STOP_MOT                       = 0x101,
    MIDW_GET_RAW_TPEG                   = 0x102,
    MIDW_STOP_RAW_TPEG                  = 0x103,
    MIDW_GET_TPEG_SNI                   = 0x104,
    MIDW_STOP_TPEG_SNI                  = 0x105,
    MIDW_GET_SERVICE_LINKING_INFO       = 0x106,
    MIDW_STOP_SERVICE_LINKING_INFO      = 0x107,

    MIDW_GET_EPG                            = 0x110,
    MIDW_STOP_EPG                           = 0x111,
    MIDW_GET_EPG_SEVICES_INFO_FOR_ENSEMBLE  = 0x112,
    MIDW_GET_EPG_PROGRAMME_INFO             = 0x113,

    // EXPERT MODE COMMAND
    MIDW_SET_INFO_LEVEL                 = 0x2F0,
    MIDW_GET_TASK_INFO                  = 0x2F1,
    MIDW_ENABLE_DATALOGGER              = 0x2F2,
    MIDW_WRITE_LOGGED_DATA              = 0x2F3,
    MIDW_WRITE_LOGGED_DATA_DELAYED      = 0x2F4,
    MIDW_LANDSCAPE_BROWSER              = 0x2F5,
    MIDW_GET_CURRENT_FRAME              = 0x2F6,
    MIDW_SET_AMFM_PROCESSING_MODE       = 0x2F7,
    MIDW_GET_AVAILABLE_APPL             = 0x2F8,
    MIDW_SETUP_APPLICATION_PATH         = 0x2F9,

    MIDW_SET_AM_FM_DISS_MODE            = 0x2FB
};

enum CisCommandCodeTy
{
    UNKNOWN_TYPE                     = (quint8)0x00,
    READ_TYPE                        = (quint8)0x01, // read direct
    WRITE_TYPE                       = (quint8)0x06, // write direct
    CMD_TYPE                         = (quint8)0x05, // generic decoded command(API command)
    // WRITE_BOOT                       = (quint8)0x06, //write direct
    RESET_IC                         = (quint8)0x07, // reset
    A2B_INIT_CMD                     = (quint8)0x08, // A2B init
    RESET_OTHER_DEVICE               = (quint8)0x09,
    BOOT_SEQ                         = (quint8)0x77, // boot sequence
    ABNORMAL                         = (quint8)0x03, // AbNormal
    SCRIPT_SEQ                       = (quint8)0x0A  // script sequence
        //    READ_FIRMWARE_VERSION           = 0x01,
};

enum cmdFormatT
{
    FORMAT_32bit    = 0,
    FORMAT_24bit    = 1
};

enum StatusCodeTy
{
    COMMAND_COMPLETED_CORRECTLY = 0x00,
    RFU_0x01                    = 0x01, // Reserved for future use
    RFU_0x02                    = 0x02, // Reserved for future use
    RFU_0x03                    = 0x03, // Reserved for future use
    RFU_0x04                    = 0x04, // Reserved for future use
    OPERATION_TIMEOUT           = 0x05, // The operation ended with a timeout
    RFU_0x06                    = 0x06, // Reserved for future use
    HEADER_0_FORMAT_WRONG       = 0x07, // Header 0 format wrong (rfu for Cis command)
    QUEUE_FULL                  = 0x08, // The command has been rejected due to command queue full
    WRONG_PARAMETERS            = 0x09, // The command has been rejected due to a wrong parameter
    ILLEGAL_COMMAND_NUMBER      = 0x0A, // The command has been rejected because the command number is illegal
    RESERVED_COMMAND_NUMBER     = 0x0B, // The command has been rejected because the command number is reserved
    DUPLICATED_AUTO_COMMAND     = 0x0C, // An auto command has been rejected because the same command has been previously issued and it is still active
    PAYLOAD_TOO_LONG            = 0x0D, // The payload is too long and is not currently supported
    UNAVAILABLE_FUNCTIONALITY   = 0x0E, // Requested command number is not implemented
    WRONG_COMMAND_LENGTH        = 0x0F, // The command has been issued with wrong length
    GENERIC_FAILURE             = 0x10, // The command failed for unknown reason
    FUNCTIONALITY_DISABLED      = 0x11, // The requested functionality is disabled. A command to enable this functionality must be issued to enable it
    APPLICATION_NOT_SUPPORTED   = 0x12, // The command target is an application not supported by the current firmware
    STOP_BIT_ILLEGALLY_SET      = 0x13, // The stop bit has been illegally set
    COMMAND_IS_NOT_ONGOING      = 0x14, // A stop (commands with specific stop command version) has been issued without command ongoing
    RADIO_UNCORRECT_SEQUENCE    = 0x15, // A command has been issued without prior mandatory command: sequence illegal
    NO_MEMORY_AVAILABLE         = 0x16, // A command requires too much memory to be executed
    NVM_MEMORY_ERROR            = 0x17, // An error occurred during access to NVM for data read or write
    RESPONSE_WITHOUT_PAYLOAD    = 0x20, // The response is available but no payload is available
    RESPONSE_NO_DATA_AVAILABLE  = 0x21, // The command has been responded but the requested data is not available
    RESERVED_0x22               = 0x22, // This error code is used internally and will never show-up to the host
    OPERATION_ONGOING           = 0x23, // A tune related command is still ongoing
    OPERATION_FAILED            = 0x24, // A tune related command is ended with error
    RESERVED_0x25               = 0x25, // This error code is used internally and will never show-up to the host
    OTHER_COMMAND_WAITING       = 0x26, // The command has been refused because another command is waiting and no buffer is available
    RFU_0x27                    = 0x27, // Reserved for future use
    CMD_SINTAX_ERROR            = 0x53,
    NO_DATA_TO_WRITE            = 0x54,
    CONNECTION_ERRORS           = 0x55,
    CMD_TIMEOUT_ERROR           = 0x57,
    RESP_FORMAT_UNKNOW          = 0x58,
    CMD_STATUS_RUNNING          = 0x59,
    RESPONSE_TIMEOUT_ERROR      = 0x5A,
    OPENFILE_ERROR              = 0x5B,

    CMD_OK = COMMAND_COMPLETED_CORRECTLY
};

enum MessageTypeTy
{
    MDW_COMMAND             = (quint8)0x07,
    MDW_NOTIFICATION        = (quint8)0x08,
    MDW_RESPONSE            = (quint8)0x09,
    MDW_AUTORESPONSE        = (quint8)0x0A,
    MDW_AUTONOTIFICATION    = (quint8)0x0B,

    MDW_FIC_BINARY_DATA_ON_DATACHANNEL      = (quint8)0x00,
    MDW_RAW_BINARY_DATA_ON_DATACHANNEL      = (quint8)0x02,
    MDW_ASCII_DATA_ON_DATACHANNEL           = (quint8)0x04,
    MDW_DECODED_BINARY_DATA_ON_DATACHANNEL  = (quint8)0x03,
    MDW_AUDIO_BINARY_DATA_ON_DATACHANNEL    = (quint8)0x06,

    SFX_COMMAND     = (quint8)0x10,
    SFX_RESPONSE    = (quint8)0x11,
    ERR_MESSAGE     = (quint8)0x12,

    MDW_RESP_TYPE_UNKNOW    = (quint8)0x0FF
};

enum CisMessageTypeTy
{
    CIS_UNVALID             = (quint8)0x00,
    CIS_COMMAND             = (quint8)0x01,
    CIS_NOTIFICATION        = (quint8)0x02,
    CIS_RESPONSE            = (quint8)0x03,
    CIS_ERR_MESSAGE         = (quint8)0x04,
    CIS_RESP_UNKNOW         = (quint8)0x0FF
};

enum DecodedPayloadFormatTy
{
    EPG_RAW                 = 0x01, // MOT Body objects containing EPG. The payload follows the “Raw MOT Object payload” format
    SLS                     = 0x02, // MOT Body objects containing SLS extracted from a data stream. The payload follows the “Raw MOT Object payload” format
    SLS_OVER_XPAD           = 0x03, // MOT Body objects containing SLS extracted from the X-PAD of an audio stream.The payload follows the “Raw MOT Object
                                    // payload” format
                                    // NOTE: the PacketAddress field of the Binary Application Header is set to 0 in this case.
    BWS_RAW                 = 0x04, // Not yet supported
    TPEG_RAW                = 0x05, // DataGroup object containing TPEG extracted from a TDC data stream. The payload follows the “Raw DataGroup object” format
    TPEG_SNI                = 0x06, // A binary object containing the TPEG Service and Network Information (SNI).The payload follows the “SNI Table” format
    SERVICE_LINKING_INFO    = 0x07, // A binary object describing the service linking Information.The payload follows the “Service Linking Information format”
    EPG_BIN                 = 0x10, // A complete set of EPG objects.The payload follows the format described in chapter 25.6.5
    RFU                     = 0x11,
    EPG_SRV                 = 0x12, // EPG Service Information decoded from XML, filtered and re-encoded as a plain table of strings.
    EPG_PRG                 = 0x13, // EPG Programme Information decoded from XML, filtered and re-encoded as a plain table of strings.
    DEBUG_DUMP              = 0x80  // Reserved for Debug data. The payload follows the format described in  chapter 25.6.8
};

enum StatusCodeActionsTy
{
    MW_STATUS_NO_ERROR          = 0,
    MW_STATUS_ERROR_CONTINUE    = 1,
    MW_STATUS_ERROR_ABORT       = 2
};

struct AnnounceSwitchTy
{
    quint32     ensembleId; // 0,1,2
    quint16     annSwitch; // 3,4
    quint32     serviceId_1; // 5,6,7,8
    quint32     serviceId_2; // 9,10,11,12
    quint8      subChId; // 13
    quint8      newFlag; // 14
    quint8      regionFlag; // 15
};

struct ReconfigOngoingTy
{
    quint8      reconfigType; // 0
    quint32     occurrenceTime; // 1,2,3,4
    quint32     cifCounter; // 5,6,7,8
    quint32     ensembleId; // 9,10,11
};

struct DabStatusTy
{
    quint8     notifReason; // 0
    quint8     search; // 1
    quint8     tranMode; // 2
    quint8     berFic; // 3
    quint8     mute; // 4
    quint32    tunedFreq; // 5,6,7,8
    quint8     reconfig; // 9
    quint8     sync; // 10
};

struct AutoNotifDataFieldsTy
{
    AnnounceSwitchTy    announceSwitch;
    ReconfigOngoingTy   reconfigOngoing;
    DabStatusTy         dabStatus;
    int id;
};

struct PadOnDlsTy
{
    quint8          PadType;
    quint8          Charset;
    QString         DecodedLabel;
};

typedef union
{
    struct
    {
        quint8   Stop :  1;
        quint8   Data :  1;
        quint8   Fast :  1;
        quint8   Len :   1;
        quint8   Sto :   1;
        quint8   Auto :  1;
        quint8   Reply : 1;
        quint8   Host :  1;
    } bits;

    quint8 byte;
} HeaderZeroTy;

typedef union
{
    struct
    {
        quint8   CmdCodeUp_0 :   1;
        quint8   CmdCodeUp_1 :   1;
        quint8   MainDAB :       1;
        quint8   SecondaryDab :  1;
        quint8   MainAMFM :      1;
        quint8   BackAMFM :      1;
        quint8   rfu_0 :         1;
        quint8   rfu_1 :         1;
    } bits;

    quint8 byte;
} HeaderOneTy;

struct FwVersionTy
{
    QString   dateYear;
    QString   dateMonth;
    QString   dateDay;
    QString   version;
    QString   build;
    QString   capabilities;
};

struct ChunkTy
{
    QString key;
    QString value;
};

typedef union
{
    struct
    {
        quint8   tmp4 :  1;
        quint8   tmp5 :  1;
        quint8   tmp6 :  1;
        quint8   tmp7 :  1;
    } bytes;

    quint32 h;
} FourBytesTy;

struct DataChannelProtHeaderTy
{
    MessageTypeTy    MessageType;
    quint8           CurrAppId;
};

struct AsciiAppHeaderTy
{
    QString  HeaderZero;
    QString  HeaderOne;
};

struct BinaryDecodedAppHeaderTy
{
    quint8                      SubChId;
    DecodedPayloadFormatTy      PayloadFormat;
    quint16                     PacketAdd;
    quint8                      ContinuityIndex; // size: 4 bits
    quint32                     Size;
};

struct MotHeaderTy
{
    quint16 MOTBodyInfoLen;      // size: 2 bytes - The sum of the sizes in bytes of all the following fields, excluding MOTBodyInfoLen, up to and excluding
                                 // MOTBody
    quint8  ContentType;        // size: 1 bytes - From the MOT Directory entry for this object
    quint8  ContentSubType;      // size: 2 bytes - From the MOT Directory entry for this object
    quint8  ContentNameCharSet;  // size: 1 bytes - Character set indicator byte for ContentName
    quint8  ContentNameLen;      // size: 1 bytes - Length in bytes of the ContentName field
    quint8  ContentName;         // size: ContentNameLen bytes - ContentName string without the leading ‘character set indicator byte’
    quint8  MimeTypeCharSet;     // size: 1 bytes - Character set indicator byte for MimeType
    quint8  MimeTypeLen;         // size: 1 bytes - Length in bytes of the MimeType field
    quint8  MimeType;           // size: MimeTypeLen bytes - MimeType string without the leading ‘character set indicator byte’
    quint8  AppParamLen;        // size: 1 bytes - Length in bytes of the AppParam field
    quint8  AppParam;           // size: AppParamLen bytes - Object-specific Application Parameters
    quint8  GlobalAppParamLen;   // size: 1 bytes - Length in bytes of GlobalAppParam field
    quint8  GlobalAppParam;      // size: GlobalAppParamLen bytes - Carousel-wide Application Parameters
    quint8  GlobalUADataLen;     // size: 1 bytes - Length in bytes of the GlobalUAData field
    quint8  GlobalUAData;        // size: GlobalUADataLen bytes - Carousel-wide User Application data, from FIG 0/13
};

struct MotObjTy
{
    BinaryDecodedAppHeaderTy   BinDecodedAppHeader;
    MotHeaderTy                MotHeader;
    QByteArray                 MotBody;
};

struct MdwVerifyParamTy
{
    QString     checkMaskStr;
    QString     respTypeStr;
    int         checkMaskSize;
    QString     mismatchAction;
};

struct MdwSendCmdParamTy
{
    MdwCommandCodeTy    cmdCode;
    QString             cmdStr;
    QString             cmdName;
    StatusCodeTy        cmdStatusCode;
    QString             cmdStatusCodeName;
    int                 timeOut;
    bool                cmdNeedResponse;
    bool                cmdNeedAutoResponse;
};

struct MdwCommandTy
{
    MdwCommandCodeTy        cmdCode;
    QString                 cmdName;
    QString                 cmdStr;
    StatusCodeTy            cmdStatusCode;
    QString                 cmdStatusCodeName;
    StatusCodeActionsTy     cmdStatusCodeAction;
    bool                    cmdNeedResponse;
    bool                    cmdNeedAutoResponse;
    int                     writtenBytes;
    int                     timeOut;
};

struct MdwResponseTy
{
    QByteArray              respData;
    QString                 respDataStr;
    quint8                  respHeaderLenght;
    quint8                  respPayloadLenght;
    StatusCodeTy            respStatusCode;
    QString                 respStatusCodeName;
    MdwCommandCodeTy        respCmdCode;
    QString                 respCmdCodeName;
    StatusCodeActionsTy     respStatusCodeAction;
};

struct MdwNotificationTy
{
    QByteArray          notifData;
    QString             notifDataStr;
    quint8              notifHeaderLenght;
    quint8              notifPayloadLenght;
    MdwCommandCodeTy    notifCmdCode;
    QString             notifCmdCodeName;
    StatusCodeTy        notifStatusCode;
    QString             notifStatusCodeName;
    StatusCodeActionsTy notifStatusCodeAction;
};

struct CisSeekParamsTy
{
    bool isAnswerReceived;
    bool isValidStation;
    bool isFullSeekCycle;
    quint32 reachedFreq;
};

struct CisNotificationTy
{
    QByteArray          notifData;
    QString             notifDataStr;
    quint8              notifHeaderLenght;
    quint8              notifPayloadLenght;
    CisCommandCodeTy    notifCmdCode;
    QString             notifCmdCodeName;
    StatusCodeTy        notifStatusCode;
    QString             notifStatusCodeName;
    StatusCodeActionsTy notifStatusCodeAction;
};

struct MdwAutoNotificationTy
{
    QByteArray              autoNotifData;
    QString                 autoNotifDataStr;
    quint8                  autoNotifHeaderLenght;
    quint8                  autoNotifPayloadLenght;
    StatusCodeTy            autoNotifStatusCode;
    QString                 autoNotifStatusCodeName;
    AutoNotifDataFieldsTy   autoNotifDataFields;
    StatusCodeActionsTy     autoNotifStatusCodeAction;
};

struct MdwDataPacketTy
{
    MdwCommandTy                    mdwCommand;
    MdwNotificationTy               mdwNotif;
    MdwResponseTy                   mdwResponse;
    MdwResponseTy                   mdwAutoResponse;
    MdwAutoNotificationTy           mdwAutoNotif;
    MotObjTy                        mot;
    MessageTypeTy                   msgType;
};

enum
{
    RADIO_DATA_COMMAND = 0,
    RADIO_DATA_NOTIFY = 1,
    RADIO_DATA_RESPONSE = 2,
    RADIO_DATA_AUTONOTIFY = 3
};

enum RadioViewsTy
{
    RADIO_HW_INIT_VIEW      = 0,
    RADIO_HW_OFF_VIEW       = 1,
    RADIO_HW_ON_VIEW        = 2,
    RADIO_POWER_ON_VIEW     = 3
};

enum AnimationNameTy
{
    NO_ANIMATION                = 0,
    BUTTON_AUDIO_GEOMETRY_ANIMATION,
    BUTTON_RADIO_GEOMETRY_ANIMATION,
    BUTTON_OPTION_GEOMETRY_ANIMATION,
    BUTTON_SETTING_GEOMETRY_ANIMATION,
    BUTTON_UP_GEOMETRY_ANIMATION,
    BUTTON_OPTION_OPACITY_ANIMATION,
    BUTTON_SETTING_OPACITY_ANIMATION,
    BUTTON_UP_OPACITY_ANIMATION,
    BUTTON_AUDIO_OPACITY_ANIMATION,
    BUTTON_RADIO_OPACITY_ANIMATION,
    LIST_WIDGET_OPACITY_ANIMATION_ON,
    LIST_WIDGET_OPACITY_ANIMATION_OFF,
    LABEL_PAD_ON_DLS_BACKGROUND_OPACITY_ANIMATION_OFF,
    LABEL_PAD_ON_DLS_BACKGROUND_OPACITY_ANIMATION_ON,
    LABEL_WAIT_ANIMATION,
    RADIO_OFF_ANIMATION_GROUP,
    LABEL_ST_LOGO_OPACITY_ANIMATION_OFF,
    LABEL_ST_LOGO_OPACITY_ANIMATION_ON,
    LABEL_RADIO_BACKGROUND_OPACITY_ANIMATION_OFF,
    LABEL_RADIO_BACKGROUND_OPACITY_ANIMATION_ON
};

enum AnimationActionsTy
{
    NO_ACTIONS          = 0,
    ANIMATION_SETUP     = 1,
    ANIMATION_START     = 2,
    ANIMATION_STOP      = 3,
    ANIMATION_DELETE    = 4,
    ANIMATION_RESTART   = 5,
    ANIMATION_PAUSE     = 6
};

enum RadioActionsTy
{
    // Initialization actions
    RADIO_ACTION_NONE                     = 0,
    RADIO_INIT_ACTION                     = 1,

    // Buttons actions
    RADIO_POWER_ON_ACTION                 = 2,
    RADIO_POWER_OFF_ACTION                = 3,
    RADIO_SEEK_UP_ACTION                  = 4,
    RADIO_SEEK_DOWN_ACTION                = 5,
    RADIO_TUNE_FREQUENCY_ACTION           = 6,
    RADIO_SOURCE_ACTION                   = 7,
    RADIO_ACTION_VOLUME                   = 8,
    RADIO_PRESET_ACTION                   = 9,
    RADIO_SETUP_ACTION                    = 10,
    RADIO_SERVICELIST_ACTION              = 11,
    RADIO_SERVICE_SELECT_ACTION           = 12,

    // Automatic actions
    RADIO_DISPLAY_SLSPTY                  = 20,
    RADIO_TIMER_EXPIRED                   = 21,
    RADIO_DISPLAY_TESTSCREEN              = 22
};

enum RadioSequencesStatusTy
{
    RADIO_SEQUENCE_OK                     = 0,
    RADIO_GENERIC_SEQUENCE_ERROR          = 2,
    RADIO_NO_CONNECTION_AVAILABLE_ERROR   = 3,
    RADIO_NO_SIGNAL_ERROR                 = 4,
    RADIO_FREQ_NO_SYNC_ERROR              = 5,
    RADIO_SEQUENCE_ERROR_ABORT            = 6,
    RADIO_SEQUENCE_INTERNAL_TIMEOUT_ERROR = 7
};

struct RdsDataSetTableTy
{
    QString             rdsTuneFreq;
    QString             PSname;
    QString             PIid;
    QString             countryName;
    QString             ECC_label;
    QString             rtText;
    bool                tpFlag;
    bool                taFlag;
    bool                msFlag;
    quint8              ptyVal;
    QString             ptyName;
    quint8              diVal;
    quint8              RdsAFList[26];
};

struct ServiceTy
{
    QString     ServiceID;
    unsigned int serviceUniqueId;
    unsigned int frequency;
    quint16     ServiceBitrate;
    quint8      SubChID;
    QString     ServiceLabel;
    quint8      ServiceCharset;
    quint8      ServicePty;
};

struct ServiceListTy
{
    QList<ServiceTy>    serviceList;
};

struct EnsembleTableTy
{
    QString             EnsECCID;

    unsigned int        ensembleUniqueId;

    quint8              EnsTune_Ber_Fic;
    quint8              EnsTuneSync;
    quint8              EnsTxMode;
    quint32             ensFrequency;
    QString             EnsChLabel;
    quint8              EnsCharset;

    //QList<ServiceTy>    ServicesTableList;
};

class DataMessages
{
    public:
        DataMessages(QString instancerName = " No istancer")
        {
            Q_UNUSED(instancerName)

            mdwCommandObj       = new MdwCommandTy;
            mdwNotifObj         = new MdwNotificationTy;
            mdwResponseObj      = new MdwResponseTy;
            mdwAutoResponseObj  = new MdwResponseTy;
            mdwAutoNotifListObj = new QList<MdwAutoNotificationTy> ();
            mdwAutoRespListObj  = new QList<MdwResponseTy> ();
        }

        MdwCommandTy* GetMdwCommandObj()     { return mdwCommandObj; }
        MdwResponseTy* GetMdwResponseObj()    { return mdwResponseObj; }
        MdwResponseTy* GetMdwAutoResponseObj() { return mdwAutoResponseObj; }
        MdwNotificationTy* GetMdwNotificationObj() { return mdwNotifObj; }
        QList<MdwAutoNotificationTy>* GetAutoNotifListObj()  { return mdwAutoNotifListObj; }
        QList<MdwResponseTy>* GetAutoRespListObj()   { return mdwAutoRespListObj; }

    public:
        MessageTypeTy   currMsgType;
        int             scriptLineIndex;

        // QList<PresetTableTy>    PresetsTableList;

    public:
        inline QString GetMdwCommandName(MdwCommandCodeTy cmdCode)
        {
            QString cmdName;

            switch (cmdCode)
            {
                case MIDW_GET_ALIVE_CHK: // = 0x000,
                {
                    cmdName = "MIDW_GET_ALIVE_CHK";
                }
                break;

                case MIDW_POWER_UP: // = 0x001,
                {
                    cmdName = "MIDW_POWER_UP";
                }
                break;

                case MIDW_GET_FIRMW_VER: // = 0x002,
                {
                    cmdName = "MIDW_GET_FIRMW_VER";
                }
                break;

                case MIDW_GET_DAB_INFO: // = 0x003,
                {
                    cmdName = "MIDW_GET_DAB_INFO";
                }
                break;

                case MIDW_POWER_DOWN: // = 0x004,
                {
                    cmdName = "MIDW_POWER_DOWN";
                }
                break;

                case MIDW_GET_AMFM_INFO: // = 0x005,
                {
                    cmdName = "MIDW_GET_AMFM_INFO";
                }
                break;

                case MIDW_GET_DAB_BER_MONITORING: // = 0x006,
                {
                    cmdName = "MIDW_GET_DAB_BER_MONITORING";
                }
                break;

                case MIDW_SETUP_EVENT_NOTIFICATIONS: // = 0x010,
                {
                    cmdName = "MIDW_SETUP_EVENT_NOTIFICATIONS";
                }
                break;

                case MIDW_DUMP_DATABASE: // = 0x021,
                {
                    cmdName = "MIDW_DUMP_DATABASE";
                }
                break;

                case MIDW_TUNER_TABLE_WRITE: // = 0x022,
                {
                    cmdName = "MIDW_TUNER_TABLE_WRITE";
                }
                break;

                case MIDW_TUNER_TABLE_DUMP: // = 0x023,
                {
                    cmdName = "MIDW_TUNER_TABLE_DUMP";
                }
                break;

                case MIDW_TUNER_TABLE_CLEAR: // = 0x024,
                {
                    cmdName = "MIDW_TUNER_TABLE_CLEAR";
                }
                break;

                case MIDW_ALL_TUNER_TABLE_CLEAR: // = 0x025,
                {
                    cmdName = "MIDW_ALL_TUNER_TABLE_CLEAR";
                }
                break;

                case MIDW_CLEAR_DATABASE: // = 0x026,
                {
                    cmdName = "MIDW_CLEAR_DATABASE";
                }
                break;

                case MIDW_CLEAR_ENSEMBLE_DATA: // = 0x027,
                {
                    cmdName = "MIDW_CLEAR_ENSEMBLE_DATA";
                }
                break;

                case ERASE_NVM_DATA: // = 0x028,
                {
                    cmdName = "ERASE_NVM_DATA";
                }
                break;

                case MIDW_GET_RAW_RDS: // = 0x030,
                {
                    cmdName = "MIDW_GET_RAW_RDS";
                }
                break;

                case MIDW_GET_RDS_PS: // = 0x031,
                {
                    cmdName = "MIDW_GET_RDS_PS";
                }
                break;

                case MIDW_GET_RDS_DI: // = 0x032,
                {
                    cmdName = "MIDW_GET_RDS_DI";
                }
                break;

                case MIDW_GET_RDS_PI: // = 0x033,
                {
                    cmdName = "MIDW_GET_RDS_PI";
                }
                break;

                case MIDW_GET_RDS_PTY: // = 0x034,
                {
                    cmdName = "MIDW_GET_RDS_PTY";
                }
                break;

                case MIDW_GET_RDS_TP_TA: // = 0x035,
                {
                    cmdName = "MIDW_GET_RDS_TP_TA";
                }
                break;

                case MIDW_GET_RDS_PTY_TP_TA_MS: // = 0x036,
                {
                    cmdName = "MIDW_GET_RDS_PTY_TP_TA_MS";
                }
                break;

                case MIDW_GET_RDS_TIME: // = 0x037,
                {
                    cmdName = "MIDW_GET_RDS_TIME";
                }
                break;

                case MIDW_GET_RDS_RT: // = 0x038,
                {
                    cmdName = "MIDW_GET_RDS_RT";
                }
                break;

                case MIDW_GET_RDS_AF_LIST: // = 0x039,
                {
                    cmdName = "MIDW_GET_RDS_AF_LIST";
                }
                break;

                case MIDW_SET_RDS_TH: // = 0x03A,
                {
                    cmdName = "MIDW_SET_RDS_TH";
                }
                break;

                case MIDW_GET_RDS_EON: // = 0x03B,
                {
                    cmdName = "MIDW_GET_RDS_EON";
                }
                break;

                case MIDW_GET_RDS_OFF: // = 0x03F,
                {
                    cmdName = "MIDW_GET_RDS_OFF";
                }
                break;

                case MIDW_GET_ENSEMBLE_LIST: // = 0x040,
                {
                    cmdName = "MIDW_GET_ENSEMBLE_LIST";
                }
                break;

                case MIDW_GET_ENSEMBLE_DATA: // = 0x041,
                {
                    cmdName = "MIDW_GET_ENSEMBLE_DATA";
                }
                break;

                case MIDW_GET_FIC: // = 0x042,
                {
                    cmdName = "MIDW_GET_FIC";
                }
                break;

                case MIDW_GET_SERVICE_LIST: // = 0x043,
                {
                    cmdName = "MIDW_GET_SERVICE_LIST";
                }
                break;

                case MIDW_GET_SPECIFIC_SERVICE_DATA: // = 0x044,
                {
                    cmdName = "MIDW_GET_SPECIFIC_SERVICE_DATA";
                }
                break;

                case MIDW_GET_TIME: // = 0x045,
                {
                    cmdName = "MIDW_GET_TIME";
                }
                break;

                case MIDW_GET_TMC_SERVICE_LIST: // = 0x046,
                {
                    cmdName = "MIDW_GET_TMC_SERVICE_LIST";
                }
                break;

                case MIDW_GET_TMC_SERVICE_DATA: // = 0x047,
                {
                    cmdName = "MIDW_GET_TMC_SERVICE_DATA";
                }
                break;

                case MIDW_GET_FIDC_DATA: // = 0x048,
                {
                    cmdName = "MIDW_GET_FIDC_DATA";
                }
                break;

                case MIDW_GET_USER_DATA: // = 0x049,
                {
                    cmdName = "MIDW_GET_USER_DATA";
                }
                break;

                case MIDW_GET_PAD_DLS: // = 0x051,
                {
                    cmdName = "MIDW_GET_PAD_DLS";
                }
                break;

                case MIDW_SETUP_PAD_DL_PLUS: // = 0x052,
                {
                    cmdName = "MIDW_SETUP_PAD_DL_PLUS";
                }
                break;

                case MIDW_GET_PAD_DL_PLUS: // = 0x053,
                {
                    cmdName = "MIDW_GET_PAD_DL_PLUS";
                }
                break;

                case MIDW_TUNE_FREQUENCY: // = 0x060,
                {
                    cmdName = "MIDW_TUNE_FREQUENCY";
                }
                break;

                case MIDW_SERVICE_SELECT: // = 0x061,
                {
                    cmdName = "MIDW_SERVICE_SELECT";
                }
                break;

                case MIDW_SEEK: // = 0x062,
                {
                    cmdName = "MIDW_SEEK";
                }
                break;

                case MIDW_SCAN: // = 0x063,
                {
                    cmdName = "MIDW_TUNER_SET_DISS";
                }
                break;

                case MIDW_LEARN: // = 0x064,
                {
                    cmdName = "MIDW_LEARN";
                }
                break;

                case MIDW_SEL_AUDIO_OUT: // = 0x065,
                {
                    cmdName = "MIDW_SEL_AUDIO_OUT";
                }
                break;

                case MIDW_DABFM_SWITCHING: // = 0x066,
                {
                    cmdName = "MIDW_DABFM_SWITCHING";
                }
                break;

                case MIDW_TUNE_DAB_CHANNEL: // = 0x067,
                {
                    cmdName = "MIDW_TUNE_DAB_CHANNEL";
                }
                break;

                case MIDW_GET_SERVICE_QUALITY: // = 0x068,
                {
                    cmdName = "MIDW_GET_SERVICE_QUALITY";
                }
                break;

                case MIDW_SETUP_AM_FM_TH: // = 0x069,
                {
                    cmdName = "MIDW_SETUP_AM_FM_TH";
                }
                break;

                case MIDW_SETUP_SERVICE_FOLLOWING: // = 0x06A,
                {
                    cmdName = "MIDW_SETUP_SERVICE_FOLLOWING";
                }
                break;

                case MIDW_TUNE_NEXT_FREQ: // = 0x06B,
                {
                    cmdName = "MIDW_TUNE_NEXT_FREQ";
                }
                break;

                case MIDW_SETUP_AM_FM_BAND_DATA: // = 0x06C,
                {
                    cmdName = "MIDW_SETUP_AM_FM_BAND_DATA";
                }
                break;

                case MIDW_GET_CURR_ENSEMBLE: // = 0x06D,
                {
                    cmdName = "MIDW_GET_CURR_ENSEMBLE";
                }
                break;

                case MIDW_SET_TUNE_TIMEOUT: // = 0x06E,
                {
                    cmdName = "MIDW_SET_TUNE_TIMEOUT";
                }
                break;

                case MIDW_MUTE: // = 0x082,
                {
                    cmdName = "MIDW_MUTE";
                }
                break;

                case MIDW_SET_ERROR_CONCEALMENT_PARAMS: // = 0x085
                {
                    cmdName = "MIDW_SET_ERROR_CONCEALMENT_PARAMS";
                }
                break;

                case MIDW_GET_DYN_RANGE_CORRECTION: // = 0x086,
                {
                    cmdName = "MIDW_GET_DYN_RANGE_CORRECTION";
                }
                break;

                case MIDW_SET_DYN_RANGE_CORRECTION: // = 0x087,
                {
                    cmdName = "MIDW_SET_DYN_RANGE_CORRECTION";
                }
                break;

                case MIDW_GET_FM_STEREO_INDICATION: // = 0x089,
                {
                    cmdName = "MIDW_GET_FM_STEREO_INDICATION";
                }
                break;

                case MIDW_GET_MOT: // = 0x100,
                {
                    cmdName = "MIDW_GET_MOT";
                }
                break;

                case MIDW_STOP_MOT: // = 0x101,
                {
                    cmdName = "MIDW_STOP_MOT";
                }
                break;

                case MIDW_GET_RAW_TPEG: // = 0x102,
                {
                    cmdName = "MIDW_GET_RAW_TPEG";
                }
                break;

                case MIDW_STOP_RAW_TPEG: // = 0x103,
                {
                    cmdName = "MIDW_STOP_RAW_TPEG";
                }
                break;

                case MIDW_GET_TPEG_SNI: // = 0x104,
                {
                    cmdName = "MIDW_GET_TPEG_SNI";
                }
                break;

                case MIDW_STOP_TPEG_SNI: // = 0x105,
                {
                    cmdName = "MIDW_STOP_TPEG_SNI";
                }
                break;

                case MIDW_GET_SERVICE_LINKING_INFO: // = 0x106,
                {
                    cmdName = "MIDW_GET_SERVICE_LINKING_INFO";
                }
                break;

                case MIDW_STOP_SERVICE_LINKING_INFO: // = 0x107,
                {
                    cmdName = "MIDW_STOP_SERVICE_LINKING_INFO";
                }
                break;

                case MIDW_GET_EPG: // = 0x110,
                {
                    cmdName = "MIDW_GET_EPG";
                }
                break;

                case MIDW_STOP_EPG: // = 0x111,
                {
                    cmdName = "MIDW_STOP_EPG";
                }
                break;

                case MIDW_GET_EPG_SEVICES_INFO_FOR_ENSEMBLE: // = 0x112,
                {
                    cmdName = "MIDW_GET_EPG_SEVICES_INFO_FOR_ENSEMBLE";
                }
                break;

                case MIDW_GET_EPG_PROGRAMME_INFO: // = 0x113,
                {
                    cmdName = "MIDW_GET_EPG_PROGRAMME_INFO";
                }
                break;

                case MIDW_SET_INFO_LEVEL: // = 0x2F0,
                {
                    cmdName = "MIDW_SET_INFO_LEVEL";
                }
                break;

                case MIDW_GET_TASK_INFO: // = 0x2F1,
                {
                    cmdName = "MIDW_GET_TASK_INFO";
                }
                break;

                case MIDW_ENABLE_DATALOGGER: // = 0x2F2,
                {
                    cmdName = "MIDW_ENABLE_DATALOGGER";
                }
                break;

                case MIDW_WRITE_LOGGED_DATA: // = 0x2F3,
                {
                    cmdName = "MIDW_WRITE_LOGGED_DATA";
                }
                break;

                case MIDW_WRITE_LOGGED_DATA_DELAYED: // = 0x2F4,
                {
                    cmdName = "MIDW_WRITE_LOGGED_DATA_DELAYED";
                }
                break;

                case MIDW_LANDSCAPE_BROWSER: // = 0x2F5,
                {
                    cmdName = "MIDW_LANDSCAPE_BROWSER";
                }
                break;

                case MIDW_GET_CURRENT_FRAME: // = 0x2F6,
                {
                    cmdName = "MIDW_GET_CURRENT_FRAME";
                }
                break;

                case MIDW_SET_AMFM_PROCESSING_MODE: // = 0x2F7,
                {
                    cmdName = "MIDW_SET_AMFM_PROCESSING_MODE";
                }
                break;

                case MIDW_GET_AVAILABLE_APPL: // = 0x2F8,
                {
                    cmdName = "MIDW_GET_AVAILABLE_APPL";
                }
                break;

                case MIDW_SETUP_APPLICATION_PATH: // = 0x2F9,
                {
                    cmdName = "MIDW_SETUP_APPLICATION_PATH";
                }
                break;

                case MIDW_SET_AM_FM_DISS_MODE: // = 0x2FB
                {
                    cmdName = "MIDW_SET_AM_FM_DISS_MODE";
                }
                break;

                default:
                {
                    // No code
                }
                break;
            }

            return cmdName;
        }

        inline MessageTypeTy GetResponseType(QString respType)
        {
            if (respType == "notification")
            {
                return MDW_NOTIFICATION;
            }
            else if (respType == "response")
            {
                return MDW_RESPONSE;
            }
            else if (respType == "autoresponse")
            {
                return MDW_AUTORESPONSE;
            }
            else if (respType == "autonotification")
            {
                return MDW_AUTONOTIFICATION;
            }

            return ERR_MESSAGE;
        }

        inline QString GetMdwStatusCodeName(StatusCodeTy statusCode)
        {
            QString CurrStatusCodeName;

            switch (statusCode)
            {
                case COMMAND_COMPLETED_CORRECTLY:
                {
                    CurrStatusCodeName = "MDW_COMMAND_COMPLETED_CORRECTLY";
                }
                break;

                case RFU_0x01:
                case RFU_0x02:
                case RFU_0x03:
                case RFU_0x04:
                case RFU_0x06:
                case RESERVED_0x22:
                case RFU_0x27:
                case RESERVED_0x25:
                {
                    CurrStatusCodeName = "MDW_RFU_COMMAND";
                }
                break;

                case OPERATION_TIMEOUT:
                {
                    CurrStatusCodeName = "MDW_OPERATION_TIMEOUT";
                }
                break;

                case HEADER_0_FORMAT_WRONG:
                {
                    CurrStatusCodeName = "MDW_HEADER_0_FORMAT_WRONG";
                }
                break;

                case QUEUE_FULL:
                {
                    CurrStatusCodeName = "MDW_QUEUE_FULL";
                }
                break;

                case WRONG_PARAMETERS:
                {
                    CurrStatusCodeName = "MDW_WRONG_PARAMETERs";
                }
                break;

                case ILLEGAL_COMMAND_NUMBER:
                {
                    CurrStatusCodeName = "MDW_ILLEGAL_COMMAND_NUMBER";
                }
                break;

                case RESERVED_COMMAND_NUMBER:
                {
                    CurrStatusCodeName = "MDW_RESERVED_COMMAND_NUMBER";
                }
                break;

                case DUPLICATED_AUTO_COMMAND:
                {
                    CurrStatusCodeName = "MDW_DUPLICATED_AUTO_COMMAND";
                }
                break;

                case PAYLOAD_TOO_LONG:
                {
                    CurrStatusCodeName = "MDW_PAYLOAD_TOO_LONG";
                }
                break;

                case UNAVAILABLE_FUNCTIONALITY:
                {
                    CurrStatusCodeName = "MDW_UNAVAILABLE_FUNCTIONALITY";
                }
                break;

                case WRONG_COMMAND_LENGTH:
                {
                    CurrStatusCodeName = "MDW_WRONG_COMMAND_LENGTH";
                }
                break;

                case GENERIC_FAILURE:
                {
                    CurrStatusCodeName = "MDW_GENERIC_FAILURE";
                }
                break;

                case FUNCTIONALITY_DISABLED:
                {
                    CurrStatusCodeName = "FUNCTIONALITY_DISABLED";
                }
                break;

                case APPLICATION_NOT_SUPPORTED:
                {
                    CurrStatusCodeName = "APPLICATION_NOT_SUPPORTED";
                }
                break;

                case STOP_BIT_ILLEGALLY_SET:
                {
                    CurrStatusCodeName = "STOP_BIT_ILLEGALLY_SET";
                }
                break;

                case COMMAND_IS_NOT_ONGOING:
                {
                    CurrStatusCodeName = "COMMAND_IS_NOT_ONGOING";
                }
                break;

                case RADIO_UNCORRECT_SEQUENCE:
                {
                    CurrStatusCodeName = "RADIO_UNCORRECT_SEQUENCE";
                }
                break;

                case NO_MEMORY_AVAILABLE:
                {
                    CurrStatusCodeName = "NO_MEMORY_AVAILABLE";
                }
                break;

                case NVM_MEMORY_ERROR:
                {
                    CurrStatusCodeName = "NVM_MEMORY_ERROR";
                }
                break;

                case RESPONSE_WITHOUT_PAYLOAD:
                {
                    CurrStatusCodeName = "MDW_RESPONSE_WITHOUT_PAYLOAD";
                }
                break;

                case RESPONSE_NO_DATA_AVAILABLE:
                {
                    CurrStatusCodeName = "MDW_RESPONSE_NO_DATA_AVAILABLE";
                }
                break;

                case OPERATION_ONGOING:
                {
                    CurrStatusCodeName = "MDW_OPERATION_ONGOING";
                }
                break;

                case OPERATION_FAILED:
                {
                    CurrStatusCodeName = "MDW_OPERATION_FAILED";
                }
                break;

                case OTHER_COMMAND_WAITING:
                {
                    CurrStatusCodeName = "MDW_OTHER_COMMAND_WAITING";
                }
                break;

                case NO_DATA_TO_WRITE:
                {
                    CurrStatusCodeName = "MDW_RFU_COMMAND";
                }
                break;

                case CONNECTION_ERRORS:
                {
                    CurrStatusCodeName = "MDW_CONNECTION_ERRORS";
                }
                break;

                case CMD_SINTAX_ERROR:
                {
                    CurrStatusCodeName = "MDW_SINTAX_ERROR";
                }
                break;

                case CMD_TIMEOUT_ERROR:
                {
                    CurrStatusCodeName = "CMD_TIMEOUT_ERROR";
                }
                break;

                case CMD_STATUS_RUNNING:
                {
                    CurrStatusCodeName = "CMD_STATUS_RUNNING";
                }
                break;

                case RESP_FORMAT_UNKNOW:
                {
                    CurrStatusCodeName = "MDW_RESP_FORMAT_UNKNOW";
                }
                break;

                default:
                {
                    // No code
                }
                break;
            }

            return CurrStatusCodeName;
        }

    public:
        void wait(quint32 msec)
        {
            QEventLoop eventLoop;
            QTimer timer;

            timer.setSingleShot(true);
            QObject::connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
            timer.start(msec);
            eventLoop.exec();
        }

        QByteArray ByteArrayToStr(QByteArray& data)
        {
            QByteArray commandTextEdit_tmp;
            QByteArray dec_values = data;
            QString temp_string;
            int i = 0, len = dec_values.length();

            while (i < len)
            {
                int value = (unsigned char)dec_values[i++];

                temp_string.setNum(value, 16);
                QString tmp;
                tmp.fill(QChar('0'), (2 - temp_string.length())).append(temp_string);
                temp_string = tmp;

                commandTextEdit_tmp.append(temp_string.toUpper());
            }

            return commandTextEdit_tmp;
        }

        QString InsertSpaces(QString& data, qint32 every_x_chars)
        {
            QString data_tmp;
            qint32  data_size = data.size();

            if ((data_size > every_x_chars) && (data_size > 0))
            {
                while (data_size >= every_x_chars)
                {
                    data_tmp = data.right(every_x_chars) + SEPARATOR_STRING + data_tmp;
                    data.remove(data.length() - every_x_chars, every_x_chars);
                    data_size -= every_x_chars;
                }

                data_tmp = data_tmp.trimmed();

                return data_tmp;
            }

            return data;
        }

        QString FormatMdwCommandString(QString str)
        {
            str = str.toUpper();
            str = str.simplified(); // remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '.
            str = str.remove(QChar(' '), Qt::CaseInsensitive); // remove spaces
            str = InsertSpaces(str, 2);

            return str;
        }

        MdwDataPacketTy ExtractCommandInfo(QString command)
        {
            MdwDataPacketTy decodedDataObj;

            // Save current message type
            decodedDataObj.msgType = MDW_COMMAND;

            // Save command string
            decodedDataObj.mdwCommand.cmdStr = FormatMdwCommandString(command);

            // Set the current CmdStatusCode
            decodedDataObj.mdwCommand.cmdStatusCode = CMD_STATUS_RUNNING;

            // Save current CmdStatusCodeName
            decodedDataObj.mdwCommand.cmdStatusCodeName = GetMdwStatusCodeName(CMD_STATUS_RUNNING);

            // Save command code
            command = command.simplified(); // remove ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '.
            command = command.remove(QChar(' '), Qt::CaseInsensitive); // remove spaces
            command = InsertSpaces(command, 2); // insert spaces every two characters

            QStringList tokens = command.split(" ", QString::SkipEmptyParts);

            bool ok;
            HeaderZeroTy headZero;
            headZero.byte   = (quint8)tokens.at(0).toInt(&ok, 16);
            quint8 headOne  = (quint8)tokens.at(1).toInt(&ok, 16);
            quint8 headTwo  = (quint8)tokens.at(2).toInt(&ok, 16);

            quint16 cmdCode = 0x000;
            quint8 cmdMsb = headOne & 0x03;
            quint8 cmdLsb = headTwo;
            cmdCode |= (cmdMsb << 8);
            cmdCode |= (cmdLsb);

            decodedDataObj.mdwCommand.cmdCode = (MdwCommandCodeTy)cmdCode;

            // Save command name
            decodedDataObj.mdwCommand.cmdName = GetMdwCommandName((MdwCommandCodeTy)cmdCode);

            // Does command wait a response?
            bool cmdNeedResponse = true;

            if (headZero.bits.Stop || headZero.bits.Auto || headZero.bits.Fast)
            {
                cmdNeedResponse = false;
            }

            // Need the command response?
            decodedDataObj.mdwCommand.cmdNeedResponse = cmdNeedResponse;

            return decodedDataObj;
        }

    private:
        MdwCommandTy* mdwCommandObj;
        MdwNotificationTy* mdwNotifObj;
        MdwResponseTy* mdwResponseObj;
        MdwResponseTy* mdwAutoResponseObj;
        QList<MdwAutoNotificationTy>* mdwAutoNotifListObj;
        QList<MdwResponseTy>* mdwAutoRespListObj;
};

class DataChannel
{
    public:
        DataChannel(QString _instancerName = " No istancer")
        {
            Q_UNUSED(_instancerName);

            motObj  = new MotObjTy;

            DataChProtocolHeader = new DataChannelProtHeaderTy;
        }

        DataChannelProtHeaderTy* DataChProtocolHeader;

        MotObjTy* motObj;
};

#endif // COMMON_H
