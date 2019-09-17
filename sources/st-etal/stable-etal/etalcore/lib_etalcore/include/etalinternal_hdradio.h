//!
//!  \file 		etalinternal_hdradio.h
//!  \brief 	<i><b>ETAL communication, private header</b></i>
//!  \details	ETAL internal and external communications, HDRADIO-specific definitions
//!

/**************************************
 * Source configuration
 *************************************/
#define	 HD_MONITORING

/**************************************
 * Defines
 *************************************/
/*!
 * \def		END_ARG_MARKER
 * 			Reserved value used in the construction of the Command
 * 			Packet to indicate the end of the variable arguments
 * 			list (see #ETAL_initCPbuffer_HDRADIO)
 */
#define END_ARG_MARKER                             (-1)
/*!
 * \def		HDRADIO_CP_FULL_SIZE
 * 			Max size of the Command Layer message supported in ETAL.
 * 			This may not be the absolute maximum size of messages deriving
 * 			from the HD Radio specs but it is based on command 'PSD decode'
 *                  biggest possible answer.
 */
#define HDRADIO_CP_FULL_SIZE                       (ETAL_HD_MAX_RESPONSE_LEN + HDRADIO_CP_OVERHEAD)
#define HDRADIO_CP_OVERHEAD                        8
#define HDRADIO_CP_PAYLOAD_INDEX                   5
#define HDRADIO_CP_CMDTYPESTATUS_LEN               3
#define HDRADIO_FAKE_RECEIVER                      100
#define HDRADIO_FAKE_RECEIVER2                     101


/****** Command Packet defines ******/
#define RESERVED                             0
#define HC_COMMAND_PACKET_GET_OPCODE(_buf_)  ((_buf_)[0])
#define HC_COMMAND_PACKET_GET_DATALEN(_buf_) ((((tU32)(_buf_)[4])<<24)|(((tU32)(_buf_)[3])<<16)|(((tU32)(_buf_)[2])<<8)|((tU32)(_buf_)[1]))
#define HC_COMMAND_PACKET_GET_FUNCODE(_buf_) ((_buf_)[5])
/**** Command Packet defines end ****/

/****** CP ReturnData defines ******/
#define ETAL_HDRADIO_CMD_FUNCTION_OFFSET           (HDRADIO_CP_PAYLOAD_INDEX + 0)

/* Get_Ext_SIS_Data Command Function (opcode 0x47) */
#define ETAL_HDRADIO_GETBASICSIS_MIN_FIELD_LEN     (HDRADIO_CP_OVERHEAD + 0x0B)
#define ETAL_HDRADIO_GETBASICSIS_NUMTYPES_OFFSET   (HDRADIO_CP_PAYLOAD_INDEX + 0x0A)
#define ETAL_HDRADIO_GETBASICSIS_TYPEID_REL_OFFSET 0x00
#define ETAL_HDRADIO_GETBASICSIS_STATUS_REL_OFFSET 0x01
#define ETAL_HDRADIO_GETBASICSIS_LENGTH_REL_OFFSET 0x02
#define ETAL_HDRADIO_GETBASICSIS_DATA_REL_OFFSET   0x03

/* Sys_Version Command Function (opcode 0x80) */
/*!
 * \def		ETAL_HDRADIO_SWVER_MAX_STRING_LEN
 * 			Max length of the string returned by the Sys_Version (0x80)
 * 			command, function Get_SW_Version (0x01).
 * 			The HD Radio specification defines this lenght as 36 in section 9.6.1,
 * 			ETAL extends with a null terminator
 */
#define ETAL_HDRADIO_SWVER_MAX_STRING_LEN          37
#define ETAL_HDRADIO_SWVER_MIN_FIELD_LEN           (HDRADIO_CP_OVERHEAD + ETAL_HDRADIO_SWVER_MAX_STRING_LEN)
#define ETAL_HDRADIO_SWVER_STRING_OFFSET           (HDRADIO_CP_PAYLOAD_INDEX + 1)
/*!
 * \def		ETAL_HDRADIO_STA680_SWVER_STRING
 * 			The STA680 software version sub-string used
 * 			to verify the device is alive.
 * 			Depending on th embedded software version,
 * 			there are at least three strings returned by the STA680:
 * 			- "  STA680-5100156C-0D000033-C0005.000"
 * 			- "  STA680-5100156C-0D000033-C0006.102"
 * 			- "  STA680-51001569-0D000033-C0006.300"
 *
 * 			To avoid future problems ETAL only checks the first part of the string
 */
#define ETAL_HDRADIO_STA680_SWVER_STRING           "  STA680"
#define ETAL_HDRADIO_SWVER_STRING_LEN              (sizeof(ETAL_HDRADIO_STA680_SWVER_STRING) - 1)

/* Sys_Tune Command Function (opcode 0x82) */
#define ETAL_HDRADIO_TUNESEL_MIN_FIELD_LEN                (HDRADIO_CP_OVERHEAD + 2)
#define ETAL_HDRADIO_TUNESTATUS_PENDING_DIG_MIN_FIELD_LEN (HDRADIO_CP_OVERHEAD + 2)
#define ETAL_HDRADIO_TUNESTATUS_DIG_MIN_FIELD_LEN         (HDRADIO_CP_OVERHEAD + 32)
#define ETAL_HDRADIO_TUNESTATUS_DIG_SPECIALCASE_FIELD_LEN (HDRADIO_CP_OVERHEAD + 5)
#define ETAL_HDRADIO_TUNESTATUS_DSQM_MIN_FIELD_LEN        (HDRADIO_CP_OVERHEAD + 4)
#define ETAL_HDRADIO_VALID_INDICATION_OFFSET              (HDRADIO_CP_PAYLOAD_INDEX + 1)
#define ETAL_HDRADIO_TUNE_OPERATION_OFFSET                (HDRADIO_CP_PAYLOAD_INDEX + 1)
#define ETAL_HDRADIO_TUNE_OPERATION_GETSTATUS_BAND_OFFSET (HDRADIO_CP_PAYLOAD_INDEX + 2)
#define ETAL_HDRADIO_TUNE_OPERATION_GETSTATUS_FREQUENCY_OFFSET (HDRADIO_CP_PAYLOAD_INDEX + 3)
#define ETAL_HDRADIO_DSQM_OFFSET                          (HDRADIO_CP_PAYLOAD_INDEX + 2)
#define ETAL_HDRADIO_TUNEALIGNSTATUS_MIN_FIELD_LEN        (HDRADIO_CP_OVERHEAD + 3)
#define ETAL_HDRADIO_TUNEALIGNSTATUS_OFFSET               (HDRADIO_CP_PAYLOAD_INDEX + 1)

/* Sys_Cntrl_Cnfg (opcode 0x83) */
#define ETAL_HDRADIO_SYSCNTRL_GET_MIN_FIELD_LEN           (HDRADIO_CP_OVERHEAD + 5)
#define ETAL_HDRADIO_SW_CONFIG_OFFSET                     (HDRADIO_CP_PAYLOAD_INDEX + 1)
#define ETAL_HDRADIO_SYSCNTRL_SET_MIN_FIELD_LEN           (HDRADIO_CP_OVERHEAD + 2)
#define ETAL_HDRADIO_SW_CONFIG_STATUS_OFFSET              (HDRADIO_CP_PAYLOAD_INDEX + 1)
#define ETAL_HDRADIO_SYSCNTRL_GET_SUPPORTED_SERVICES_MIN_FIELD_LEN      (HDRADIO_CP_OVERHEAD + 5)
#define ETAL_HDRADIO_SYSCNTRL_GET_SUPPORTED_SERVICES_RX_SW_CNFG_OFFSET  (HDRADIO_CP_PAYLOAD_INDEX + 1)

/* IBOC_Cntrl_Cnfg (opcode 0x91) */
#define ETAL_HDRADIO_IBOC_CNTRL_CNFG_SET_MRC_CNFG_MIN_FIELD_LEN     (HDRADIO_CP_OVERHEAD + 2)
#define ETAL_HDRADIO_IBOC_CNTRL_CNFG_MRC_CNFG_STATUS_OFFSET         (ETAL_HDRADIO_CMD_FUNCTION_OFFSET + 1)
#define ETAL_HDRADIO_IBOC_CNTRL_CNFG_GET_MRC_CNFG_MIN_FIELD_LEN     (HDRADIO_CP_OVERHEAD + 2)
#define ETAL_HDRADIO_MRC_CNFG_MRC_ENABLE_STATUS_MASK                0x01
#define ETAL_HDRADIO_MRC_CNFG_FREQUENCY_MISMATCH_STATUS_MASK        0x02
#define ETAL_HDRADIO_MRC_CNFG_BAND_MISMATCH_STATUS_MASK             0x04

/* PSD_Decode Command Function (opcode 0x93) */
#define ETAL_HDRADIO_GETPSD_MIN_FIELD_LEN          (HDRADIO_CP_OVERHEAD + 1)
#define ETAL_HDRADIO_GETPSD_MIN_INFO_LEN           7
#define ETAL_HDRADIO_GETPSD_FIRST_FIELD_OFFSET     (HDRADIO_CP_PAYLOAD_INDEX + 1)
#define ETAL_HDRADIO_GETPSD_PROGRAM_REL_OFFSET     0x00
#define ETAL_HDRADIO_GETPSD_FIELD_REL_OFFSET       0x01
#define ETAL_HDRADIO_GETPSD_LENGTH_REL_OFFSET      0x06
#define ETAL_HDRADIO_GETPSD_DATA_REL_OFFSET        0x07

#define ETAL_HDRADIO_SETPSD_MIN_FIELD_LEN          (HDRADIO_CP_OVERHEAD + 2)
#define ETAL_HDRADIO_CONF_STATUS_OFFSET            (HDRADIO_CP_PAYLOAD_INDEX + 1)

#define ETAL_HDRADIO_GETPSD_CFG_MIN_RESP_LEN       (HDRADIO_CP_OVERHEAD + 2)
#define ETAL_HDRADIO_GETPSD_CFG_LEN_OFFSET         (HDRADIO_CP_PAYLOAD_INDEX + 1)

/**** CP ReturnData defines end ****/

/**************************************
 * Types
 *************************************/
/*!	enum	etalTuneFSMIndexTy
 * 			Identifies the concurrent users of the #ETAL_tuneFSM_HDRADIO
 * 			state machine.
 */
typedef enum
{
	/*! For calls from the external API use this identifier */
	ETAL_HDRADIO_TUNEFSM_API_USER      = 0,
	/*! For calls from #ETAL_CommunicationLayer_ThreadEntry_HDRADIO use this identifier */
	ETAL_HDRADIO_TUNEFSM_INTERNAL_USER = 1,
	/*! The total number of identifiers, used to allocate arrays and sanity checks */
	ETAL_HDRADIO_TUNEFSM_MAX_USER      = 2
} etalTuneFSMIndexTy;

/*!
 * \enum	etalTuneFSMHDActionTy
 * 			Define the type of action required to the #ETAL_tuneFSM_HDRADIO
 * 			function.
 */
typedef enum
{
	/*! The function performs all the actions in one shot,
	 * thus it returns after at most #ETAL_HD_TUNE_TIMEOUT msec */
	tuneFSMHDNormalResponse,
	/*! The function operates as a state machine that exits after the main states;
	 * the function exits only if the STA680 is left in a known state */
	tuneFSMHDImmediateResponse,
	/*! The function releases its semaphore and returns immediately;
	 * on next invocation it will restart from the Init state;
	 * parameter *waitAudio* is ignored in this case */
	tuneFSMHDRestart,
	/*! The function update the audio sync status 
	 * so that on next status check, an Event Update can be sent
	 * on interface to client
	 * else client may not know what happen 
	 * in case of seek in SPS typically
	 */
	tuneFSMHDNewService
} etalTuneFSMHDActionTy;

/* Commands structs */
/*!
 * \struct	etalToHDRADIOCpTy
 * 			Generic header for HD Radio Command Packet.
 * 			For details refer to the HD Radio spec (see etalcmd_hdradio.c file description)
 */
typedef struct
{
	/*! The Command Opcode, as defined in #tyHDRADIOCmdID */
	tU8  m_Opcode;
	/*! The Command Packet length (4 bytes), defined in section 5.1.2 */
	tU32 m_DataLength;
	/*! The command parameters: this is a variable length field.
	 *  The ETAL implementation uses a unique global buffer #CP_buf to
	 *  store these parameters.
	 */
	tU8 *m_CpData;
	/*! The Command function code. This field is used for convenience,
	 *  in the HD Radio spec the function code is part of the paramters.
	 *  In ETAL the function #ETAL_initCPbuffer_HDRADIO copies this
	 *  field to the command parameters field
	 */
	tU8  m_FunctionCode;
	/*! The Command Type as defined in #tyHDRADIOCmdType */
	tU8  m_CmdType;
	/*! The Command Status, normally used only for responses */
	tU8  m_CmdStatus;
} etalToHDRADIOCpTy;

typedef union {
	tU8 B;
	struct {
		tU8 Success           :1;//lsb
		tU8 LMCountMismatch   :1;
		tU8 HeaderNotFound    :1;
		tU8 ChecksumFailed    :1;
		tU8 OverflowCondition :1;
		tU8 ErrorCodeField    :3;//msb
	} bf;
} etalToHDRADIOStatusTy;
/* Commands structs end */

/* CmdTypes enum */
typedef enum {
	ARG_1B_LEN = 1,
	ARG_2B_LEN = 2,
	ARG_3B_LEN = 3,
	ARG_4B_LEN = 4,
} tyHDRADIOArgByteLen;

/*!
 * \enum	tyHDRADIOCmdType
 * 			The Command Type, as defined in the HD Radio
 * 			spec (see etalcmd_hdradio.c file description), section 5.1.4. 
 */
typedef enum {
	/*! Error or uninitialized entry */
	UNDEF_TYPE = -1,
	/*! The Host is reading data from the DCOP */
	READ_TYPE  =  0,
	/*! The Host is writing data to the DCOP */
	WRITE_TYPE =  1,
} tyHDRADIOCmdType;
/* CmdTypes enums end */

/*!
 * \enum	tyHDRADIOCmdID
 * 			Lists the HD Radio command Opcode
 * 			as defined in the HD Radio specification,
 * 			section 5.2.
 * 			The enum contains also some codes defined
 * 			for internal ETAL use.
 */
typedef enum {
	HDRADIO_IBOC_ACQUIRE_CMD            = 0x01, // requires 46ms delay, see RX_TN_4068
	HDRADIO_BAND_SELECT_CMD             = 0x02, // requires 46ms delay, see RX_TN_4068
	HDRADIO_GET_STATUS_CMD              = 0x20,
	HDRADIO_GET_QI_CMD                  = 0x21,
	HDRADIO_SPS_CNTRL_CMD               = 0x22, // requires 46ms delay, see RX_TN_4068
	HDRADIO_GET_ACQ_STATUS_CMD          = 0x23,
	HDRADIO_AAS_ENABLE_PORTS_CMD        = 0x41,
	HDRADIO_AAS_GET_ENABLED_PORTS_CMD   = 0x42,
	HDRADIO_GET_PSD_CMD                 = 0x43,
	HDRADIO_FLUSH_PSD_QUEUE_CMD         = 0x44,
	HDRADIO_SET_SIS_CNFG_CMD            = 0x45,
	HDRADIO_GET_SIS_CNFG_CMD            = 0x46,
	HDRADIO_GET_EXT_SIS_DATA_CMD        = 0x47,
	HDRADIO_FLUSH_SIS_DATA_CMD          = 0x48,
	HDRADIO_AAS_GET_DATA_CMD            = 0x49,
	HDRADIO_AAS_FLUSH_QUEUE_CMD         = 0x4A,
	HDRADIO_GET_SERVICE_INFO_CMD        = 0x4B,
	HDRADIO_AAS_PROC_LOT_CMD            = 0x4C,
	HDRADIO_SIG_GET_DATA_CMD            = 0x4D,
	HDRADIO_AAS_CNTRL_CNFG_CMD          = 0x4E,
	HDRADIO_ACTIVE_ALERTS_CMD           = 0x60,
	HDRADIO_CA_CNTRL_CMD                = 0x75,
	HDRADIO_SYS_VERSION_CMD             = 0x80,
	HDRADIO_SYS_TUNE_CMD                = 0x82, // requires 46ms delay, see RX_TN_4068
	HDRADIO_SYS_CNTRL_CNFG_CMD          = 0x83,
	HDRADIO_SYS_NVM_ACCESS_CMD          = 0x84,
	HDRADIO_SYS_AUDIO_CNTRL_CMD         = 0x8B,
	HDRADIO_SYS_DIAGNOSTICS_CMD         = 0x8D,
	HDRADIO_IBOC_CNTRL_CNFG_CMD         = 0x91, // requires 46ms delay, see RX_TN_4068
	HDRADIO_PSD_DECODE_CMD              = 0x93,
	HDRADIO_TOKEN_CMD                   = 0x94,
	HDRADIO_ADVANCED_AUDIO_BLENDING_CMD = 0x99,
	HDRADIO_IBOC_DIAGNOSTICS_CMD        = 0x9D,
	HDRADIO_ANALOG_AM_DEMOD_CMD         = 0xA0,
	HDRADIO_ANALOG_FM_DEMOD_CMD         = 0xA1,
	HDRADIO_RBDS_CNTRL_CMD              = 0xA2,
	/*! Start a background tune operation for a specific Receiver */
	HDRADIO_SEEK_START_SPECIAL          = 0xFE,
	/*! Stop a background tune operation for a specific Receiver */
	HDRADIO_SEEK_STOP_SPECIAL           = 0xFF,
} tyHDRADIOCmdID;
/* OpCodes enum end */

/* FunctionCodes enums */
typedef enum {
	TUNE_SELECT           = 0x01,
	TUNE_STEP             = 0x02,
	SEEK                  = 0x03,
	SCAN                  = 0x04,
	TUNE_GET_STATUS       = 0x06,
	SET_TUNING_PARAMETERS = 0x0A,
	GET_TUNING_PARAMETERS = 0x0B,
	GET_DSQM_STATUS       = 0x0C,
	GET_ALIGN_STATUS	  = 0x0D,
} tyHDRADIOTuneCmdFunc;

typedef enum {
	GET_STATION_MESSAGE           = 0x01,
	GET_TIME_ZONE                 = 0x02,
	GET_LEAP_SECONDS              = 0x03,
	GET_UNIVERSAL_SHORT_NAME      = 0x04,
	GET_BASIC_SIS_DATA            = 0x05,
	GET_SLOGAN                    = 0x06,
	GET_TX_EXCITER_CORE_VERSION   = 0x07,
	GET_TX_EXCITER_MANUF_VERSION  = 0x08,
	GET_TX_IMPORTER_CORE_VERSION  = 0x09,
	GET_TX_IMPORTER_MANUF_VERSION = 0x0A,
} tyHDRADIOSISDataCommand;

typedef enum {
	GET_PSD_DECODE     = 0x02,
	SET_PSD_CNFG_PARAM = 0x03,
	GET_PSD_CNFG_PARAM = 0x04,
} tyHDRADIOPSDDecode;

typedef enum {
	GET_SW_VERSION = 0x01,
} tyHDRADIOVersionCmdFunc;

typedef enum {
	SET_MRC_CNFG    = 0x13,
	GET_MRC_CNFG    = 0x14,
	SET_ALIGN_PARAM = 0x18,
	GET_ALIGN_PARAM = 0x19,	
} tyHDRADIOIbocCntrlCnfgCmdFunc;

typedef enum {
	GET_ACTIVATED_SERVICES = 0x01,
	SET_ACTIVATED_SERVICES = 0x02,
	GET_SUPPORTED_SERVICES = 0x03,
	GET_CNFG_INFO          = 0x08,
	GET_SYS_CNFG           = 0x09,
	SET_SYS_CNFG           = 0x0A,
	SET_SYS_CNFG_PARAMETER = 0x0C
} tyHDRADIOSysCtrlCnfgFunc;

/* FunctionCodes enums end */

/* SubsequentData enums */
typedef enum {
	TUNE_ALL_TYPES    = 0x00,
	TUNE_ANALOG_ONLY  = 0x01,
	TUNE_DIGITAL_ONLY = 0x02,
} tyHDRADIOTuneType;

typedef enum {
	AM_BAND   = 0x00,
	FM_BAND   = 0x01,
	IDLE_MODE = 0x63,
} tyHDRADIOBandID;

typedef enum {
	MPS_AUDIO_HD_1  = 0x00,
	SPS1_AUDIO_HD_2 = 0x01,
	SPS2_AUDIO_HD_3 = 0x02,
	SPS3_AUDIO_HD_4 = 0x03,
	SPS4_AUDIO_HD_5 = 0x04,
	SPS5_AUDIO_HD_6 = 0x05,
	SPS6_AUDIO_HD_7 = 0x06,
	SPS7_AUDIO_HD_8 = 0x07,
} tyHDRADIOAudioPrgID;

typedef enum {
	MPS_AUDIO_MASK  = 0x01,
	SPS1_AUDIO_MASK = 0x02,
	SPS2_AUDIO_MASK = 0x04,
	SPS3_AUDIO_MASK = 0x08,
	SPS4_AUDIO_MASK = 0x10,
	SPS5_AUDIO_MASK = 0x20,
	SPS6_AUDIO_MASK = 0x40,
	SPS7_AUDIO_MASK = 0x80,
} tyHDRADIOPSDProgramBitMask;

typedef enum {
	TITLE      = 0x0001,
	ARTIST     = 0x0002,
	ALBUM      = 0x0004,
	GENRE      = 0x0008,
	COMMENT    = 0x0010,
	UFID       = 0x0020,
	COMMERCIAL = 0x0040,
	XHDR       = 0x0080,
} tyHDRADIOPSDFieldBitMask;

typedef enum {
	NONE_COMMENT              = 0x00,
	LANGUAGE                  = 0x01,
	SHORT_CONTENT_DESCRIPTION = 0x02,
	THE_ACTUAL_TEXT           = 0x04,
} tyHDRADIOPSDCommentsBitMask;

typedef enum {
	NONE_UFID        = 0x00,
	OWNER_IDENTIFIER = 0x01,
	IDENTIFIER       = 0x02,
} tyHDRADIOPSDUFIDBitMask;

typedef enum {
	NONE_COMMERCIAL = 0x00,
	PRICE_STRING    = 0x01,
	VALID_UNTIL     = 0x02,
	CONTACT_URL     = 0x04,
	RECEIVED_AS     = 0x08,
	NAME_OF_SELLER  = 0x10,
	DESCRIPTION     = 0x20,
} tyHDRADIOPSDCommercialBitMask;

typedef enum {
	REINIT_CONF_PARAM = 0x00,
	SET_CONF_PARAM    = 0x01,
} tyHDRADIOPSDConfigParamSetting;

typedef enum {
	PSD_FIELDS_ENABLED                           = 0x01,
	PSD_TITLE_LENGTH                             = 0x02,
	PSD_ARTIST_LENGTH                            = 0x03,
	PSD_ALBUM_LENGTH                             = 0x04,
	PSD_GENRE_LENGTH                             = 0x05,
	PSD_COMMENT_SHORT_CONTENT_DESCRIPTION_LENGTH = 0x06,
	PSD_COMMENT_ACTUAL_TEXT_LENGTH               = 0x07,
	PSD_UFID_OWNER_IDENTIFIER_LENGTH             = 0x08,
	PSD_COMMERCIAL_PRICE_STRING_LENGTH           = 0x09,
	PSD_COMMERCIAL_CONTACT_URL_LENGTH            = 0x0A,
	PSD_COMMERCIAL_NAME_OF_SELLER_LENGTH         = 0x0B,
	PSD_COMMERCIAL_DESCRIPTION_LENGTH            = 0x0C,
	PSD_XHDR_LENGTH                              = 0x0D,
} tyHDRADIOPSDConfigParam;
/* SubsequentData enums end */

/* ReturnData enums */
typedef enum {
	INVALID = 0x00,
	VALID   = 0x01,
} tyHDRADIOValidIndication;

/*!
 * \enum	tyHDRADIOTuneOperationType
 * 			Indicates the type of operation for Sys_Tune (0x82)
 * 			command, function Tune_Get_Status (0x06).
 * 			The same code is used for the command and for
 * 			the response (except for one value as noted below).
 */
typedef enum {
	/*! Both analogue and digital information, command or response */
	ALL_TYPES_TUNED        = 0x00,
	/*! Only analogue information, command or response */
	ONLY_ANALOG_TUNED      = 0x01,
	/*! Only digital information, command or response */
	ONLY_DIGITAL_TUNED     = 0x02,
	/*! Tune operation not complete thus no information available, response only */
	TUNE_OPERATION_PENDING = 0xFF,
} tyHDRADIOTuneOperationType;

typedef enum {
	HD_ACQUIRED            = 0x01,
	SIS_ACQUIRED           = 0x02,
	DIGITAL_AUDIO_ACQUIRED = 0x04,
} tyHDRADIODigitalAcquisitionStatus;

typedef enum {
	PARAMETER_NOT_SET          = 0x00,
	PARAMETER_SUCCESSFULLY_SET = 0x01,
} tyHDRADIOConfigurationStatus;

typedef enum {
	STATION_ID         = 0x00,
	STATION_SHORT_NAME = 0x01,
	STATION_LONG_NAME  = 0x02,
	STATION_LOCATION   = 0x04,
} tyHDRADIOSISDataType;

typedef enum {
	NO_DATA_AVAILABLE       = 0x00,
	ONLY_OLD_DATA_AVAILABLE = 0x01,
	NEW_DATA_DISPLAYABLE    = 0x02,
	NEW_DATA_PARTIAL        = 0x03,
} tyHDRADIOSISStatus;
/* ReturnData enums end */

/* ReturnData structs */

/*!
 * \struct	etalToHDRADIODigiAcqStatusTy
 * 			This struct maps directly on the response
 * 			provided by the HD Radio DCOP to a Sys_Tune (0x82) command,
 * 			Tune_Get_Status (0x06) function, for Digital only mode,
 * 			as shown in Table 5-10 of the HD Radio spec (see etalcmd_hdradio.c file description)
 */
typedef struct
{
	tU8 m_TuneOperationType;
	tU8 m_Band;
	tU8 m_LSBRfChFreq;
	tU8 m_MSBRfChFreq;
	tU8 m_CurrentProg;
	tU8 m_DigiAcquisitionStatus;
	tU8 m_AudioQI;
	tU8 m_CdNo;
	tU8 m_TXDigitalAudioGain;
	tU8 m_TXBlendControlStatus;
	tU8 m_ProgTypeCurrentlyProg;
	tU8 m_AudioProgramsAvailable;
	tU8 m_Reserved13;
	tU8 m_Reserved14;
	tU8 m_Reserved15;
	tU8 m_AudioProgConditionalAccInfo;
	tU8 m_Reserved17;
	tU8 m_Reserved18;
	tU8 m_Reserved19;
	tU8 m_PSDChangeFlag;
	tU8 m_Reserved21;
	tU8 m_Reserved22;
	tU8 m_Reserved23;
	tU8 m_TokenFlags;
	tU8 m_CurrentSampleRateSelection;
	tU8 m_SelectedSampleRateValue;
	tU8 m_PrimaryServiceMode;
	tU8 m_CodecMode;
	tU8 m_FilteredDSQMStatus;
	tU8 m_LSBFilteredDSQMValue;
	tU8 m_MSBFilteredDSQMValue;
} etalToHDRADIODigiAcqStatusTy;

typedef union
{
	tU8 value;
	struct 
	{
		tU8 m_AlignmentIndicator:1;
		tU8 m_AudioAvailableIndicator:1;
		tU8 m_PhaseIndicator:1;
		tU8 m_reserved_1:5;
	} BitField;
} AudioIndicatorTy;

/*!
 * \struct	etalToHDRADIOAlignStatusTy
 * 			This struct maps directly on the response
 * 			provided by the HD Radio DCOP to a Sys_Tune (0x82) command,
 * 			Get_Align_Status (0x0D) function, for Digital only mode,
 * 			as shown in Table 5-10 of the HD Radio spec (see etalcmd_hdradio.c file description)
 */
typedef struct
{
	AudioIndicatorTy m_AudioIndicator;
	tU32 m_AlignmentOffset;
	tS8 m_LevelAdjustment;
} etalToHDRADIOAlignStatusTy;

/*!
 * \struct	etalToHDRADIOMRCCnfgStatusTy
 * 			This struct maps directly on the response
 * 			provided by the HD Radio DCOP to a IBOC_Cntrl_Cnfg (0x91) command,
 * 			Set_MRC_Cnfg (0x13) Get_MRC_Cnfg (0x14) functions,
 * 			as shown in Table 5-17 of the HD Radio spec (see etalcmd_hdradio.c file description)
 */
typedef union
{
	tU8 mrc_cnfg_status;
	struct 
	{
		tU8 m_MRCEnableStatus:1;
		tU8 m_FrequencyMismatchStatus:1;
		tU8 m_BandMismatchStatus:1;
		tU8 m_reserved_1:5;
	} BitField;
} HDRADIOToEtalMRCCnfgStatusTy;

/* ReturnData structs end */

/**************************************
 * Functions prototypes
 *************************************/
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
tSInt       ETAL_initCommunication_HDRADIO(tBool isBootMode, tBool vI_manageReset);
tSInt       ETAL_deinitCommunication_HDRADIO(tVoid);
tVoid       ETAL_getHDRADIODevLock(tVoid);
tVoid       ETAL_releaseHDRADIODevLock(tVoid);
tSInt       ETAL_cmdSysVersion_HDRADIO(tChar *str);
tSInt       ETAL_cmdPing_HDRADIO(tVoid);
tSInt       ETAL_getTuneFSMLock_HDRADIO(tyHDRADIOInstanceID instanceId);
tVoid       ETAL_releaseTuneFSMLock_HDRADIO(tyHDRADIOInstanceID instanceId);
tSInt       ETAL_configRadiotext_HDRADIO(tVoid);
tSInt       ETAL_cmdTuneSelect_HDRADIO(ETAL_HANDLE hReceiver, tU32 dwKHzFreq, tyHDRADIOAudioPrgID prg);
tSInt       ETAL_cmdGetStatusDigital_HDRADIO(ETAL_HANDLE hReceiver, etalToHDRADIODigiAcqStatusTy *ptDigiAcqStatus);
tSInt       ETAL_cmdGetSIS_HDRADIO(ETAL_HANDLE hReceiver, tChar *name, tU16 max_name, tBool *is_new);
tSInt       ETAL_cmdGetSISAsync_HDRADIO(ETAL_HANDLE hDatapath);
tSInt       ETAL_cmdPSDCnfgFieldsEn_HDRADIO(ETAL_HANDLE hReceiver, tU16 *PSDFieldsEnableBitmap);
tSInt       ETAL_cmdPSDCnfgLen_HDRADIO(ETAL_HANDLE hReceiver, tyHDRADIOPSDConfigParam field, tU8 fieldLen);
tSInt       ETAL_cmdPSDCnfgLenGet_HDRADIO(ETAL_HANDLE hReceiver, tyHDRADIOPSDConfigParam field, tU8 *fieldLen);
tSInt       ETAL_cmdPSDDecodeGet_HDRADIO(ETAL_HANDLE hReceiver, tS8 vI_Service, tChar *title, tU16 max_title, tChar *artist, tU16 max_artist);
tSInt       ETAL_cmdPSDDecodeGetAsync_HDRADIO(ETAL_HANDLE hDatapath);
tSInt       ETAL_SetMRCCnfg_HDRADIO(ETAL_HANDLE hReceiver, tU32 freq, tBool set_mrc_enable);
tSInt       ETAL_cmdSetMRCCnfg_HDRADIO(ETAL_HANDLE hReceiver, tBool set_mrc_enable, HDRADIOToEtalMRCCnfgStatusTy *hd_mrc_cnfg_status);
tSInt       ETAL_cmdGetMRCCnfg_HDRADIO(ETAL_HANDLE hReceiver, HDRADIOToEtalMRCCnfgStatusTy *hd_mrc_cnfg_status);
tSInt       ETAL_cmdGetSupportedServices_HDRADIO(etalHDRXSWCnfgTy *rx_sw_cnfg);
tSInt       ETAL_cmdGetQuality_HDRADIO(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
tSInt 		ETAL_cmdSetAlignParam_HDRADIO(tBool  vI_alignmentActiveForAM, tBool vI_alignmentActiveForFM, tBool vI_enableAMGain, tBool vI_enableFMGain);
tSInt       ETAL_cmdDisableAAA_HDRADIO(tVoid);
ETAL_STATUS ETAL_cmdStartPSD_HDRADIO(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_cmdStopPSD_HDRADIO(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_cmdStartPSDDataServ_HDRADIO(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_cmdStopPSDDataServ_HDRADIO(ETAL_HANDLE hReceiver);
tVoid       ETAL_pollPSDChange_HDRADIO(ETAL_HANDLE hDatapath);
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
tVoid       ETAL_stringToRadiotext_HDRADIO(EtalTextInfo *pRadiotext, tChar *title, tChar *artist);
#endif
tSInt       ETAL_sendCommandTo_HDRADIO(ETAL_HANDLE hReceiver, tU8 *cmd, tU32 clen, tU8 **resp, tSInt *rlen);
tSInt       ETAL_sendCommandToInstance_HDRADIO(tyHDRADIOInstanceID instIdCmd, tU8 *cmd, tU32 clen, tU8 **resp, tSInt *rlen);
tSInt /*@alt void@*/ETAL_queueCommand_HDRADIO(ETAL_HANDLE hDatapath, tU8 *cmd, tU32 clen);
tSInt       ETAL_parseSIS_HDRADIO(tU8 *resp, tU32 len, tChar *name, const tU32 max_len, tBool *is_new);
tSInt       ETAL_parsePSDDecode_HDRADIO(tU8 *resp, tU32 len, tU8 audioPrgs, tChar *title, tU32 max_title, tChar *artist, tU32 max_artist);
ETAL_STATUS ETAL_tuneFSM_HDRADIO(ETAL_HANDLE hReceiver, tU32 freq, etalTuneFSMHDActionTy actionType, tBool waitAudio, etalTuneFSMIndexTy index);
ETAL_STATUS ETAL_cmdStartMonitor_HDRADIO(ETAL_HANDLE hMonitor);
ETAL_STATUS ETAL_cmdStopMonitor_HDRADIO(ETAL_HANDLE hMonitor);
tSInt 		ETAL_pollSISChangeAsync_HDRADIO(ETAL_HANDLE hDatapath);
tVoid		ETAL_forceResendRadioInfo(tVoid);
tBool       ETAL_isDoFlashOrDownloadOrDumpHDR(tVoid);
tBool       ETAL_isDoDownloadHDR(tVoid);
tSInt       ETAL_doFlashOrDownloadOrDumpHDR(tVoid);

#if defined(CONFIG_COMM_DRIVER_EMBEDDED) && defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
tSInt ETAL_cmdSpecialReset_HDR(tVoid);
tSInt ETAL_cmdSpecialFlash_HDR(HDR_flashModeEnumTy op_mode, HDR_targetMemOrInstanceIDTy trg_mem_or_instId, HDR_accessModeEnumTy access_mode, const EtalDCOPAttr *dcop_attr);
#endif // #if defined(CONFIG_COMM_DRIVER_EMBEDDED) && defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

#endif
