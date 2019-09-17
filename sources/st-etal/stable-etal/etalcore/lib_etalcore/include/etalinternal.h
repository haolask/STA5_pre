//!
//!  \file 		etalinternal.h
//!  \brief 	<i><b>ETAL private header</b></i>
//!  \details   ETAL definitions to be used only within the library
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "etal_cust_param.h"
#include "etaldefs.h"
#include "etal_api.h"
#include "common_fifo.h"
#include "common_trace.h"
#include "etal_trace.h"
#include "dabmw_import.h"
#include "tunerdriver.h"

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	#include "HDRADIO_Protocol.h"
	#include "hdr_boot.h"
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
    #include "DAB_Protocol.h"
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR


#ifdef __cplusplus
extern "C" {
#endif

/***********************************
 *
 * Defines
 *
 **********************************/
/*!
 * 	\def	ETAL_CMOST_TUNER_ID_1
 * 			ID value of tuner 1
 */
#define ETAL_CMOST_TUNER_1_ID	0

/*!
 * 	\def	ETAL_CMOST_TUNER_ID_2
 * 			ID value of tuner 2
 */
#define ETAL_CMOST_TUNER_2_ID	1


/*!
 * 	\def	ETAL_BCAST_STD_NUMBER
 * 			Total number of entries of the #EtalBcastStandard enum
 */
#define ETAL_BCAST_STD_NUMBER    7

/*!
 * \def		ETAL_MAX_RECEIVERS
 * 			Max number of concurrent receivers 
 */
#define ETAL_MAX_RECEIVERS               4
/*!
 * \def		ETAL_MAX_DATAPATH_PER_RECEIVER
 * 			Max number of concurrent Datapaths
 */
#define ETAL_MAX_DATAPATH_PER_RECEIVER   9
/*!
 * \def		ETAL_MAX_MONITORS
 * 			Max number of concurrent Monitors
 */
#define ETAL_MAX_MONITORS                4
/*!
 * \def		ETAL_MAX_HDINSTANCE
 * 			Max instances (channels) supported by HD DCOP.
 * 			This value depends on the hardware capabilities
 * 			so it should not normally be modified.
 */
#define ETAL_MAX_HDINSTANCE              2
/*!
 * \def		ETAL_MAX_DATASERVICE_CACHE
 * 			Max concurrent data services for RAW data extraction.
 * 			The DAB DCOP supports 4 subch, each with 1024 packet addresses.
 * 			Each packet address can carry one Data Service.
 */
#define ETAL_MAX_DATASERVICE_CACHE       8 

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	/* ETAL_MAX_FILTERS reflects DABMW implementation, do not change! */
	/* assumed to fit in one byte, checked in ETAL_sanityCheck */
	#define ETAL_MAX_FILTERS            16
#endif

/*!
 * \def ETAL_MAX_DIVERSITY
 */
#if (defined(CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)) && (! defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL))
	#define ETAL_MAX_DIVERSITY              1
#elif defined (CONFIG_MODULE_INTEGRATED) && defined (CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707)
	#define ETAL_MAX_DIVERSITY              6
#else
	#define ETAL_MAX_DIVERSITY              3
#endif


/*!
 * \def		ETAL_UNDEF_SLOT
 * 			Identifier for uninitialized RDS slot. RDS slots are
 * 			used to store RDS decoder state.
 * \see		ETAL_receiverGetRDSSlot
 */
#define ETAL_UNDEF_SLOT                 ((tU8)0xFF)
/*!
 * \def		ETAL_VALUE_NOT_AVAILABLE
 * 			Given a 'C' language type, returns a number that
 * 			represents an unavaliable value. This value is used
 * 			for quality containers.
 * \todo	This value could be confused with proper data
 */
#define ETAL_VALUE_NOT_AVAILABLE(_type_) ((_type_)0)
/*!
 * \def		ETAL_INVALID_PROG
 * 			Identifies an invalid HD Radio program
 */
#define ETAL_INVALID_PROG               ((tS8)-1)
/*!
 * \def		ETAL_INVALID_PROG
 * 			Identifies an invalid HD Radio program
 */
#define ETAL_INVALID_MUTE_STATUS        ((tU8)0xFF)
/*!
 * \def		ETAL_MAX_INTCB
 * 			Max number of non-periodic callbacks that can be registered
 * 			concurrently.
 */
 // with SF activated we have already register for
 // Tune/Destroy/SerivceSelect/AudioSourceSelect/SSresponse/SSestimation/SeekStatus/
 // So we need more callback
#define ETAL_MAX_INTCB                   20
/*!
 * \def		ETAL_MAX_INTCB_PERIODIC
 * 			Max number of periodic callbacks that can be registered
 * 			concurrently.
 */
#define ETAL_MAX_INTCB_PERIODIC          8

/*
 * Frequency bands, derived from the CMOST documentation
 * but should be valid system-wide
 */
#define ETAL_BAND_AM_MIN         144
#define ETAL_BAND_AM_MAX       30000

#define ETAL_BAND_FM_MIN       65000
#define ETAL_BAND_FM_MAX      108000
#define ETAL_BAND_FM_STEP		 100


#define ETAL_BAND_FMEU_MIN     87500
#define ETAL_BAND_FMEU_MAX    108000
#define ETAL_BAND_FMEU_STEP		 100

#define ETAL_BAND_FMEEU_MIN    65000
#define ETAL_BAND_FMEEU_MAX    74000
#define ETAL_BAND_FMEEU_STEP     100


#define ETAL_BAND_FMUS_MIN     87900
#define ETAL_BAND_FMUS_MAX    107900
#define ETAL_BAND_FMUS_STEP		 200


#define ETAL_BAND_FMJP_MIN     76000
#define ETAL_BAND_FMJP_MAX     95000
#define ETAL_BAND_FMJP_STEP		 100


#define ETAL_BAND_WB_MIN      162400
#define ETAL_BAND_WB_MAX      162550
#define ETAL_BAND_DAB3_MIN    174928
#define ETAL_BAND_DAB3_MAX    239200
#define ETAL_BAND_DABL_MIN   1452960
#define ETAL_BAND_DABL_MAX   1478640
#define ETAL_BAND_DRM3_MIN       144 
#define ETAL_BAND_DRM3_MAX     30000
#define ETAL_BAND_DRMP_MIN    120000
#define ETAL_BAND_DRMP_MAX    250000

/*!
 * \def		ETAL_SEEK_MAX_FM_STEP
 * 			The maximum allowed seek step, in KHz, for
 * 			seek/scan/learn operations in FM.
 * 			It is used for a rough check on parameter
 * 			validity, the limit is not enforced
 * 			by the CMOST firmware.
 */
#define ETAL_SEEK_MAX_FM_STEP     500
/*!
 * \def		ETAL_SEEK_MAX_AM_STEP
 * 			The maximum allowed seek step, in KHz, for
 * 			seek/scan/learn operations in AM.
 * 			It is used for a rough check on parameter
 * 			validity, the limit is not enforced
 * 			by the CMOST firmware.
 */
#define ETAL_SEEK_MAX_AM_STEP     50

#define ETAL_SEEK_GOODSIGNAL      0x04
#define ETAL_SEEK_FULLCYCLE       0x02
#define ETAL_SEEK_WRAPPED         0x01
#define ETAL_SEEK_NOGOODSIGNAL    0x00
#define NEW_DAB_SEEK              1

#define NOISEFLOOR_INIT           -106 	//dBm
#define SEEK_TH_INC               4 	//dBm
#define SEEK_TH_INC_NO_SYNC       2 	//dBm
#define TUNE_SYNC_TIMEOUT         2500	//ms
#define WAIT_FOR_ENSEMBLE         1000 	//ms
#define NOISEFLOOR_MAX            -60 	//dBm
/*
 * Maximum number of alternate frequencies for AF_Search_Manual()
 */
#define ETAL_AF_SEARCH_MANUAL_MAX_AF_LIST     25

/*!
 * \def		ETAL_INVALID_HINDEX
 * 			Used as error return or uninitialized entry for the
 * 			'index' field of the #ETAL_HANDLE.
 */
#define ETAL_INVALID_HINDEX        ((ETAL_HINDEX)0xFF)

/*!
 * \def		ETAL_INTCB_CONTEXT_UNUSED
 * 			Indicate that the *context* parameter of the #ETAL_intCbRegister is not needed.
 */
#define ETAL_INTCB_CONTEXT_UNUSED 0x00

/*!
 * \def		ETAL_UNDEFINED_ADDRESS
 * 			Indicates unused location in a CMOST index-to-parameter address conversion
 * 			table.
 */
#define ETAL_UNDEFINED_ADDRESS 						0x000000
/*!
 * \def		ETAL_MAX_TU8_VALUE
 * 			Used in checks that verify if a value fits in a single byte.
 */
#define ETAL_MAX_TU8_VALUE                          255

/*
 * Protocol Layer LUNs - Logical Unit Number
 */
/*!
 * \def		ETAL_AUTONOTIF_LUN
 * 			The DAB DCOP uses virtual communication channels called LUN.
 * 			Since the LUN is part of the command/response its decoding
 * 			must be known to ETAL.
 *
 * 			This defines the LUN used for autonotifications, that is
 * 			messages autonomously generated by the DAB DCOP.
 */
#define ETAL_AUTONOTIF_LUN	                    ((tU8)0x29)
/*!
 * \def		ETAL_CONTROL_LUN
 * 			Defines the LUN used for commands and command responses.
 *
 * \see		ETAL_AUTONOTIF_LUN
 */
#define ETAL_CONTROL_LUN	                    ((tU8)0x30)
/*!
 * \def		ETAL_DATA_LUN
 * 			Defines the LUN used for data channel.
 *
 * \see		ETAL_AUTONOTIF_LUN
 */
#define ETAL_DATA_LUN		                    ((tU8)0x31)
/*!
 * \def		ETAL_BROADCAST_LUN
 * 			Defines the LUN used for special commands (e.g. DCOP memory flashing).
 *
 * \see		ETAL_AUTONOTIF_LUN
 */
#define ETAL_BROADCAST_LUN                      ((tU8)0xFF)

/*!
 * \def		deviceFEmask
 * 			Broad Tuner category: bitmap used to
 * 			extract the number of frontends from
 * 			an #EtalDeviceType
 */
#define deviceFEmask             0x0003

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	#define ETAL_EXTERNAL_DRIVER_TYPE_COMMAND    (tU8)0
	#define ETAL_EXTERNAL_DRIVER_TYPE_RESET      (tU8)7
	#define ETAL_EXTERNAL_DRIVER_RESERVED        (tU8)0
#endif

// NB MAX Tuner option Per Path
#define ETAL_MAX_PATH						9

/*!
 * \def		ETAL_DEF_MAX_USER_APPLICATION_DATA_TYPE
 * 			A maximum of 4 user application types are allowed for a service component. 
 * 			Each User Application is reported MSB first and uses 2 bytes. 
 * 			A value of 1 means that no valid user application has been found. 
 * 			User application types are defined in ETSI TS 101 756 Table 16 or Digital Radio Middleware Tables par. 4.8. 
 * 			This field is referred also as UAType in the DAB specifications
 */
#define ETAL_DEF_MAX_USER_APPLICATION_DATA_TYPE     4

/***********************************
 *
 * Macros
 *
 **********************************/
/*!
 * \def		ETAL_CMDROUTE_TO_CMOST
 * 			Returns true if the argument contains a generic CMOST device.
 *
 * 			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType 
 */
#define ETAL_CMDROUTE_TO_CMOST(_x_) (((_x_) & deviceCMOST) != 0)
/*!
 * \def		ETAL_CMDROUTE_TO_STAR
 * 			Returns true if the argument contains a generic STAR device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_CMDROUTE_TO_STAR(_x_)  (((_x_) & deviceSTAR) != 0)
/*!
 * \def		ETAL_CMDROUTE_TO_DCOP
 * 			Returns true if the argument contains a generic DCOP device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_CMDROUTE_TO_DCOP(_x_)  (((_x_) & deviceDCOP) != 0)
/*!
 * \def		ETAL_CMDROUTE_TO_MDR
 * 			Returns true if the argument contains a DAB DCOP device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_CMDROUTE_TO_MDR(_x_)   (((_x_) & deviceMDR) != 0)
/*!
 * \def		ETAL_CMDROUTE_TO_HD
 * 			Returns true if the argument contains a HD DCOP device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_CMDROUTE_TO_HD(_x_)    (((_x_) & deviceHD) != 0)

/*!
 * \def		ETAL_DEVICE_IS_STAR
 * 			Returns true if the argument is a generic STAR device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_DEVICE_IS_STAR(_x_)   (((_x_) & deviceSTAR) == deviceSTAR)
/*!
 * \def		ETAL_DEVICE_IS_STAR
 * 			Returns true if the argument is a generic STAR device.
 *
 *			The argument should be a bitmap of etalDeviceTy values.
 * \see		etalDeviceTy
 */
#define ETAL_DEVICE_IS_START(_x_)   (((_x_) & deviceSTART) == deviceSTART)
/*!
 * \def		ETAL_DEVICE_IS_DOT
 * 			Returns true if the argument is a generic DOT device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_DEVICE_IS_DOT(_x_)    (((_x_) & deviceDOT)  == deviceDOT)
/*!
 * \def		ETAL_DEVICE_IS_DCOP
 * 			Returns true if the argument is a generic DCOP device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_DEVICE_IS_DCOP(_x_)   (((_x_) & deviceDCOP) == deviceDCOP)
/*!
 * \def		ETAL_DEVICE_IS_MDR
 * 			Returns true if the argument is a DAB DCOP device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_DEVICE_IS_MDR(_x_)    (((_x_) & deviceMDR)  == deviceMDR)
/*!
 * \def		ETAL_DEVICE_IS_HD
 * 			Returns true if the argument is a HD DCOP device.
 *
 *			The argument should be a bitmap of EtalDeviceType values.
 * \see		EtalDeviceType
 */
#define ETAL_DEVICE_IS_HD(_x_)     (((_x_) & deviceHD)   == deviceHD)

/*!
 * \def		ETAL_WRITE_PARAM_GET_ADDRESS
 * 			Given a paramValueArray parameter of the #etal_write_parameter
 * 			it returns the address.
 */
#define ETAL_WRITE_PARAM_GET_ADDRESS(_buf) (((tU32 *)(_buf))[0])
/*!
 * \def		ETAL_WRITE_PARAM_GET_INDEX
 * 			Given a paramValueArray parameter of the #etal_write_parameter
 * 			it returns the index.
 */
#define ETAL_WRITE_PARAM_GET_INDEX(_buf)   (ETAL_WRITE_PARAM_GET_ADDRESS(_buf))
/*!
 * \def		ETAL_WRITE_PARAM_GET_VALUE
 * 			Given a paramValueArray parameter of the #etal_write_parameter
 * 			it returns the value.
 */
#define ETAL_WRITE_PARAM_GET_VALUE(_buf)   (((tU32 *)(_buf))[1])

/*!
 * \def		ETAL_WRITE_PARAM_ENTRY_SIZE
 * 			Size of each entry in the paramValueArray parameter of #etal_write_parameter
 * 			Each entry is composed of 2 integers, the size is expressed in integers
 */
#define ETAL_WRITE_PARAM_ENTRY_SIZE 2

/*!
 * \def		ETAL_IS_HDRADIO_STANDARD
 * 			True if the parameter is of type ETAL_BCAST_STD_HD_FM or ETAL_BCAST_STD_HD_AM
 */
#define ETAL_IS_HDRADIO_STANDARD(_std_)   (((_std_) == ETAL_BCAST_STD_HD_FM) || ((_std_) == ETAL_BCAST_STD_HD_AM))

#undef DEBUG_SEEK

/*!
 * \def		The maximum number of subchannels in  a DAB ensemble 
 */
#define ETAL_MAX_NUM_SUBCH_PER_ENSEMBLE  64u

/*!
 * \def		invalid (inactive) TDI and VIT params
 */
#define ETAL_INVALID_SUBCH_ID   64u

/*!
 * \def		definitions for TMID
 */
#define ETAL_TMID_MSC_STREAM_AUDIO 0x00 /* TMID */
#define ETAL_TMID_MSC_STREAM_DATA  0x01
#define ETAL_TMID_FIDC_DATA_CHANNEL_RESERVED  0x02
#define ETAL_TMID_MSC_PACKET_DATA  0x03

/*!
 * \def		definitions for ASCTY & DSCTY
 */
#define ETAL_ASCTY_MPEG1_FOREGROUND   0
#define ETAL_ASCTY_MPEG1_BACKGROUND   1
#define ETAL_ASCTY_MPEG2_MULTICHANNEL 2

#define ETAL_ASCTY_DAB_PLUS          0x3F    /* ETSI TS 102 563 V1.1.1 (2007-02), 7.1: ASCTY_DAB_PLUS =111111*/

#define ETAL_DSCTY_MPEG2TS           0x18    /* DMB: data channel with DSCTY = MPEG2-TS */

/*!
 * \def		definitions for MSC mode
 */
#define ETAL_MSC_MODE_DAB			1
#define ETAL_MSC_MODE_DAB_PLUS		2
#define ETAL_MSC_MODE_DMB			3
#define ETAL_MSC_MODE_PAD			4
#define ETAL_MSC_MODE_PACKET		5
#define ETAL_MSC_MODE_DATA			6
#define ETAL_MSC_MODE_UNDEFINED		0xfu

#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_AUTOSEEK)
// we need to have a non empty flag for waiting else it is failing in t-kernel at least
#define ETAL_DAB_SEEK_NO_EVENT_FLAG        0x00

#define ETAL_DAB_SEEK_EVENT_SLEEP			30
#define ETAL_DAB_SEEK_EVENT_SLEEP_FLAG		((tU32)0x01 << ETAL_DAB_SEEK_EVENT_SLEEP)

#define ETAL_DAB_SEEK_EVENT_FLAGS          ETAL_DAB_SEEK_EVENT_SLEEP_FLAG
#define ETAL_DAB_SEEK_EVENT_WAIT_MASK      (ETAL_DAB_SEEK_EVENT_FLAGS)

// Kill event
#define ETAL_DAB_SEEK_EVENT_KILL					0
// WAKEUP EVENT
#define ETAL_DAB_SEEK_EVENT_WAKE_UP					31

#define ETAL_DAB_SEEK_EVENT_KILL_FLAG				((tU32)0x01 << ETAL_DAB_SEEK_EVENT_KILL)
#define ETAL_DAB_SEEK_EVENT_WAKEUP_FLAG				((tU32)0x01 << ETAL_DAB_SEEK_EVENT_WAKE_UP)

#define ETAL_DAB_SEEK_WAKEUP_FLAGS					(ETAL_DAB_SEEK_EVENT_KILL_FLAG | ETAL_DAB_SEEK_EVENT_WAKEUP_FLAG)
#define ETAL_DAB_SEEK_EVENT_WAIT_WAKEUP_MASK		(ETAL_DAB_SEEK_WAKEUP_FLAGS)

#define ETAL_DAB_SEEK_EVENT_ALL_FLAG				(0xFFFFFFFF)

#define ETAL_DAB_SEEK_THREAD_PRIORITY      OSAL_C_U32_THREAD_PRIORITY_NORMAL
#define ETAL_DAB_SEEK_STACK_SIZE           4096
#define ETAL_DAB_SEEK_EVENT_WAIT_TIME_MS   10U
#endif //#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_AUTOSEEK)

/***********************************
 *
 * Types
 *
 **********************************/
typedef tSInt (*etalBoardInitFunctionPtrTy)(tVoid);
typedef tU16    etalFilterMaskTy;

/*!
 * \enum	EtalQualityReportingType
 */
typedef enum
{
	receptionQuality,
	channelQuality,
	channelQualityForAF
} EtalQualityReportingType;


/*!
 * \enum	etalCmdActionTy
 * 			Generic action specifier for function parameter.
 */
typedef enum
{
	/*! Stop the action */
	cmdActionStop,
	/*! Start the action */
	cmdActionStart
} etalCmdActionTy;

/*!
 * \enum	etalCmdTuneActionTy
 * 			Define the type of action required to the #ETAL_tuneReceiverInternal
 * 			function.
 */
typedef enum
{
	/*! Use the regular Tune Frequency command 0x060. The DCOP sends
	 *  a response after the sync is achieved, or after a timeout */
	cmdTuneNormalResponse,
	/*! Use the special version of tune (Tune Frequency Immediate, 0x07A)
	 *  only available in the DCOP version of the STA662, which sends
	 *  a response immediately after receiving the command. The response
	 *  does not contain body, only header, and is needed to support
	 *  the DCOP state machines in combination with the Tune Request
	 *  Autonotification */
	cmdTuneImmediateResponse,
	/*! Send a special version of the Tune Frequency command 0x060
	 *  used only for Service Following that has a different behaviour
	 *  in case of no sync condition. The regular Tune Frequency command
	 *  (cmdTuneNormalResponse), if the sync is not achieved on the new
	 *  frequency waits some time before sending the response,
	 *  to allow for sync recover in low signal condition. This version
	 *  requests an immediate response from the DCOP as soon as the new
	 *  frequency is tuned and the sync status is known, without waiting
	 *  for the timeout. */
	cmdTuneDoNotWaitResponseDcopDirectResponseOnStatus
} etalCmdTuneActionTy;

/*!
 * \enum	etalCmdSpecialTy
 * 			Special commands are those requiring a state machine action.
 * 			More than one may be set for a Receiver at the same time.
 */
typedef enum
{
	/*! Manual AF Check */
	cmdSpecialManualAFCheck,
	/*! Manual Seek */
	cmdSpecialManualSeek,
	/*! Automatic Seek */
	cmdSpecialSeek,
	/*! Scan */
	cmdSpecialScan,
	/*! Learn */
	cmdSpecialLearn,
	/*! Seamless estimation */
	cmdSpecialSeamlessEstimation,
	/*! Seamless switching */
	cmdSpecialSeamlessSwitching,
	/*! Seek operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalSeekRequestInProgress,
	/*! Learn operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalLearnRequestInProgress,
	/*! Scan operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalScanRequestInProgress,
	/*! RDS operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalRDSRequestInProgress,
	/*! Tune operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalTuneRequestInProgress,
	/*! DAB Announcement operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalDABAnnouncementRequestInProgress,
	/*! DAB Reconfiguration operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalDABReconfigurationRequestInProgress,
	/*! DAB Status operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalDABStatusRequestInProgress,
	/*! DAB Data Status operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalDABDataStatusRequestInProgress,
	/*! Seamless switching operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalSeamlessSwitchingRequestInProgress,
	/*! Seamless estimation operation that requires sending feedback to the ETAL API user (e.g. events) */
	cmdSpecialExternalSeamlessEstimationRequestInProgress,
	/*! RDS Extraction */
	cmdSpecialRDS,
	/*! Decoded RDS extraction */
	cmdSpecialDecodedRDS,
	/*! RDS strategy */
	cmdSpecialRDSStrategy,
	/*! Rds Automatic Seek */
	cmdSpecialRdsSeek,
	/*! TextInformation extraction */
	cmdSpecialTextInfo,
	/*! Tune operation */
	cmdSpecialTune,
	/*! Wildcard for any operation that involves changing the
	 * receiver frequency, or that would be affected by a change in the receiver frequency. */
	cmdSpecialAnyChangingFrequency,
	/*! event FM stereo notification */
	cmdSpecialEventFmStero
} etalCmdSpecialTy;

/*!
 * \enum	etalSeekModeTy
 * 			Defines the type of Seek operation
 */
typedef enum
{
	/*! Start Manual Seek mode */
	cmdManualModeStart    = 0,
	/*! Continue a Seek (automatic or manual) command */
	cmdContinue           = 1,
	/*! Start Automatic Seek mode */
	cmdAutomaticModeStart = 2
} etalSeekModeTy;

/*!
 * \enum	etalCommandTy
 * 			Command type for #ETAL_cmdRoutingCheck function parameter
 */
typedef enum
{
	/*! Tune command */
	commandTune         = 0,
	/*! Command routing depends on the Broadcast Standard */
	commandBandSpecific = 1,
	/*! RDS commands need special processing */
	commandRDS          = 2,
	/*! Quality command */
	commandQuality      = 3,
	/*! Internal use */
	commandSize         = 4
} etalCommandTy;

/*!
 * \struct	etalFrequencyBandInfoTy
 * 			Description of a Frequency band, mainly used
 * 			for seek operations.
 */
typedef struct
{
	/*! The frequency band */
	EtalFrequencyBand band;
	/*! The lower frequency band limit in Hz */
	tU32 bandMin;
	/*! The upper frequency band limit in Hz */
	tU32 bandMax;
	/*! The idefault seek step in Hz */
	tU32 step;
} etalFrequencyBandInfoTy;

/*!
 * \enum    etalCallbackTy
 *          Broad category of callback type. ETAL defines
 *          event callbacks and quality callbacks.
 * \see     ETAL_callbackInvoke
 */
typedef enum
{
    /*! Invoke an Event callback */
	cbTypeEvent,
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	/*! Invoke a quality callback for DAB DCOP */
	cbTypeQuality_MDR,
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	/*! Invoke a quality callback for AM/FM/HD Radio */
	cbTypeQuality
#endif
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API
} etalCallbackTy;

/*
 * etalFrontendDescTy
 */
typedef struct
{
	tU8             RDSSlotIndex;
	tU32            standards; // bitmap of EtalBcastStandard values
	tU32            dataTypes; // bitmap of EtalBcastDataType values
} etalFrontendDescTy;

/*
 * etalStatusFSMTy
 */
typedef enum
{
	ETAL_FSM_TUNE_REQUEST,
} etalStatusFSMTy;


#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/*
 * etalDCOPFilterTy
 */
typedef struct
{
	tBool isValid;
	tBool isValidForDcop;

	tU16  paramOffset;
	tU8   paramSize;
} etalDCOPFilterTy;

/*!
 * \struct	etalDABTuneStatusTy
 * 			Defines the main DAB Tune status information.
 */
typedef struct
{
	/*! Unique Ensemble ID (ECC + Country Id + Ensemble Id) */
	tU32              UEId;
	/*! Service ID, 16 bits or 32 bits */
	tU32              Service;
	/*! Service Label (optional) */
	tChar             ServiceLabel[ETAL_DEF_MAX_SERVICENAME];
	/*! Service Label charset */
	tU8				  ServiceLabelCharSet;
	/*! Program Associated Data, i.e. the meta data transmitted
	 * with the Audio program. The field stores the last
	 * received PAD DLS data */
	etalPADDLSTy     PADDLS;
	/*! TRUE if the *UEId* and/or the *Service* have never been
	 * read since they were received from the DAB DCOP */
	tU8               InfoIsNew;
	/*! TRUE if the *ServiceLabel* has never been
	 * read since it was received from the DAB DCOP */
	tU8               ServiceLabelIsNew;
	/*! TRUE if the *PADData* has never been
	 * read since it was received from the DAB DCOP */
	tU8               PADDataIsNew;
} etalDABTuneStatusTy;
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

/*
 * EtalTuneInfoInternal
 */
typedef struct
{
	ETAL_HANDLE              m_receiverHandle;
	tU32                     m_Frequency;
	tU32                     m_syncInternal; // this is for seek and DAB : synchro status
	tBool					 m_externalRequestInfo;
	tS8						 m_serviceId;
} EtalTuneInfoInternal;

/*
 * EtalAudioSourceSelectInternal
 */
typedef struct
{
	EtalAudioSourceTy        m_audioSourceSelected;
	tBool					 m_externalRequestInfo;
} EtalAudioSourceSelectInternal;

/*
 * EtalServiceSelectionStatus
 */
typedef struct
{
	ETAL_HANDLE              m_receiverHandle;
	tU32                     m_Frequency;
	tU32                     m_Ueid;
	tU32					 m_Sid;
	EtalServiceSelectMode	 m_mode;
	EtalServiceSelectSubFunction	m_subFunction;
} EtalServiceSelectionStatus;

/*
 * Internal callback types
 */
/*!
 * \enum	etalIntCbCallTy
 * 			Defines the type of internal callback, i.e. when it will be invoked.
 * 			To add a new internal callback mode:
 * 			1. define a new value for etalIntCbCallTy (e.g. callAtEndOfSeek)
 * 			2. identify the point in ETAL where the callback should be invoked (e.g. #ETALTML_Report_Seek_Info)
 * 			3. in that point add a call to #ETAL_intCbScheduleCallbacks(new mode)
 *
 * \see		ETAL_intCbRegister
 */
typedef enum
{
	callNoEvent,
	/*! The callback will be invoked when ETAL detects that a CMOST seek procedure
		is concluded with a good station found. The callback could be registered
		at any time but it may be invoked only after a seek command to the CMOST */
	callAtEndOfSeek,
	callAtSeekResponse,
	callAtSeekStatus,
    callAtEndOfLearn,
    callAtEndOfScan,
    callAtSeamlessEstimationResponse,
	callAtSeamlessSwitchingResponse,
	callAtLearnResponse,
	callAtServiceSelection,
    callBeforeTuneFrequency,
	callAtTuneFrequency,
    callAtHDTuneFrequency,
	callAtAudioSourceSelect,
	/*! Call at Receiver destruction, that is after stopping HD Radio Tune
	 *  function, clearing the DAB status variables and setting the Tuner
	 *  to Idle mode; see #ETAL_destroyReceiverInternal */
	callAtReceiverDestroy,
	callAtDABAutonotification,
	callAtDABAnnouncement,
    callAtEndOfRdsSeek,
} etalIntCbCallTy;

typedef tVoid (*etalIntCbPeriodicFuncPtrTy)(ETAL_HANDLE hGeneric);
typedef tVoid (*etalIntCbFuncPtrTy)(ETAL_HANDLE hGeneric, tVoid *param, tU32 param_len, tU32 context);

/*!
 * \union	etalAudioIntfStatusTy
 * 			This union is defined only to enable access to variables of
 * 			type #EtalAudioInterfTy as a tU8, without modifying
 * 			directly #EtalAudioInterfTy which is exported.
 */
typedef union
{
	tU8               all;
	EtalAudioInterfTy bitfield;
} etalAudioIntfStatusTy;

/*
 * etaltmlLearnStateTy
 */
typedef enum
{
    ETALTML_LEARN_NULL,
    ETALTML_LEARN_START,
    ETALTML_LEARN_STARTED,
    ETALTML_LEARN_STOP,
    ETALTML_LEARN_FINISHED,
    ETALTML_LEARN_ERROR,
    ETALTML_LEARN_CONTINUE,
    ETALTML_LEARN_GET_HD_SERVICE,
} etaltmlLearnStateTy;

/*
 * etaltmlScanStateTy
 */
typedef enum
{
    ETALTML_SCAN_NULL,
    ETALTML_SCAN_START,
    ETALTML_SCAN_WAIT,
    ETALTML_SCAN_STOP,
    ETALTML_SCAN_FINISHED,
    ETALTML_SCAN_ERROR,
    ETALTML_SCAN_CONTINUE,
} etaltmlScanStateTy;

/*
 * etalSeekStateTy
 */
typedef enum
{
    ETAL_DABSEEK_NULL,
    ETAL_DABSEEK_START,
    ETAL_DABSEEK_FST,
    ETAL_DABSEEK_WAIT_FOR_SYNC,
    ETAL_DABSEEK_WAIT_FOR_ENSEMBLE,
	ETAL_DABSEEK_SET_THRESHOLD,
    ETAL_DABSEEK_STOP
} etalSeekStateTy;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
typedef enum
{
	ETALTML_RDS_AUTO_SEEK_IDLE,
    ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK,
    ETALTML_RDS_AUTO_SEEK_WAITRDS,
    ETALTML_RDS_AUTO_SEEK_WAITPI,
    ETALTML_RDS_AUTO_SEEK_WAITTP,
    ETALTML_RDS_AUTO_SEEK_WAITPTY,
    ETALTML_RDS_AUTO_SEEK_FINISHED,
} etalRdsSeekStateTy;

#endif

/*
 * etalSeekConfigTy
 */
typedef struct
{
    etalSeekStateTy    			      state;
	tS32 							  noiseFloor;
	tS32 							  seekThr;
	tBool							  isNoiseFloorSet;
	OSAL_tMSecond                     time;
	tU32						      startFrequency;
	tU32						      step;
	etalSeekAudioTy    		          exitSeekAction;
	EtalSeekTerminationModeTy 	      terminationMode;
	etalSeekDirectionTy			      direction;
	EtalSeekThreshold                 seekThreshold;
	etalSeekHdModeTy                  seekHDSPS;
	tBool                             updateStopFrequency;
	EtalSeekStatus              	  autoSeekStatus;
} etalSeekConfigTy;

#ifdef CONFIG_ETAL_HAVE_ETALTML
/*
 * Cannot move to etaltml_internal.h because it is included
 * at the end of this file, but needed for etalReceiverStatusTy
 */

/*
 * etalLearnConfigTy
 */
typedef struct
{
    etaltmlLearnStateTy    			  state;
    tU32                              initialFrequency;
    tU32                              step;
    EtalSeekTerminationModeTy         terminationMode;
    tU32                              maxNbOfFrequency;
    EtalLearnFrequencyTy*             frequencyList;
    etalLearnReportingModeStatusTy    mode;
    tU32                              nbOfFreq;
    EtalFrequencyBand                 bandIndex;
    EtalLearnFrequencyTy              frequencyListTmp[ETAL_LEARN_MAX_NB_FREQ];
    EtalLearnStatusTy                 learnStatus;
} etalLearnConfigTy;

/*
 * etalScanConfigTy
 */
typedef struct
{
    etaltmlScanStateTy                state;
    tU32                              initialFrequency;
    tU32                              step;
    etalSeekDirectionTy               direction;
    EtalSeekTerminationModeTy         terminationMode;
    tU32                              audioPlayTime;
    EtalScanStatusTy                  scanStatus;
} etalScanConfigTy;
#endif // CONFIG_ETAL_HAVE_ETALTML


#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
typedef struct
{
  etalRdsSeekStateTy    			state;
  tBool								autoSeek_complete;		
  EtalSeekStatus					seekStatus;					

} etalRdsSeekConfigTy;
#endif

/*
 * etalRDSAttrModeTy
 */
typedef enum
{
	ETAL_RDS_MODE_NORMAL,
	ETAL_RDS_MODE_TEMPORARY_FAST_PI,
	ETAL_RDS_MODE_PERMANENT_FAST_PI
} etalRDSAttrModeTy;

/*
 * etalDatapathRDSAttr
 */
typedef struct
{
	etalRDSAttrModeTy   rdsMode;
	tU8     nbRdsBlockforInteruptFastPI;        /* for mode Fast PI, indicate the number of decoded PI which will generate an interrupt for buffer read */
	tU8     nbRdsBlockforInteruptNormalMode;    /* for normal mode, indicate the number of decoded blocks which will generate an interrupt for buffer read */
	EtalRDSRBDSModeTy   rdsRbdsMode;
} etalRDSAttr;

/*
 * etalDatapathRDSAttr
 */
typedef struct
{
    /* public parameters */
    etalRDSAttr rdsAttr;

    /* private parameter */
#ifndef CONFIG_COMM_ENABLE_RDS_IRQ
    tU8     numPICnt;       /* the number of A and C' RDS blocks that will be received in fast PI mode and polling rds configuration */
#endif
} etalRDSAttrInternal;

/*
 * EtalDISSStatusTy
 */
typedef struct
{
    EtalDISSMode    m_mode;
    tU8             m_filter_index;
} EtalDISSStatusTy;

/*
 * EtalDebugVPAModeEnumTy
 */
typedef enum
{
    ETAL_DEBUG_VPA_MODE_NONE    = 0,
    ETAL_DEBUG_VPA_MODE_ON      = 1,
    ETAL_DEBUG_VPA_MODE_OFF     = 2
} EtalDebugVPAModeEnumTy;


/*
 * STAR blending mode input enum
 */
typedef enum
{
	ETAL_STAR_BLENDING_AUTO_HD                  = 0x00, /* Automatic HD blending controlled via GPIO */
    ETAL_STAR_BLENDING_STAR_ANALOG_AMFM         = 0x01, /* Select STAR analogue input (AM/FM) */
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
    ETAL_STAR_BLENDING_STA660_DIGITAL_DABDRM    = 0x02  /* DAB/DRM mode (digital input from STA660) */
#else   // CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BF, CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BG or higher
    ETAL_STAR_BLENDING_STA680_DIGITAL_AMFMHD    = 0x02, /* Select STA680 digital input (AM/FM HD) */
    ETAL_STAR_BLENDING_HD_ALIGN                 = 0x03, /* HD alignment mode (left = analogue, right = digital) */
    ETAL_STAR_BLENDING_STA660_DIGITAL_DABDRM    = 0x04  /* DAB/DRM mode (digital input from STA660) */
#endif
} etalStarBlendingModeEnumTy;

/*
 * etalAudioChannelTy
 */
typedef enum
{
	ETAL_AUDIO_CHN_UNDEFINED    = 0,
	ETAL_AUDIO_CHN_STEREO       = 1,
	ETAL_AUDIO_CHN_MONO         = 2
} etalAudioChannelTy;

/*
 * etalDatapathTy
 */
typedef struct
{
	EtalSink            m_sink;
} etalDatapathTy;

/*
 * etalDiversTy
 */
typedef struct
{
	tU8               m_DiversityMode; // a.k.a. the number of front-ends used in this config
	ETAL_HANDLE       m_FeConfig[ETAL_CAPA_MAX_FRONTEND];
} etalDiversTy;

/*!
 * \struct	etalHDRXSWCnfgTy
 * 			This struct maps directly on the response
 * 			provided by the HD Radio DCOP to a IBOC_Cntrl_Cnfg (0x83) command,
 * 			Get_Supported_Services (0x03),
 * 			as shown in Table 5-12 of the HD Radio spec (see etalcmd_hdradio.c file description)
 */
typedef union
{
	tU8 RX_SW_Cnfg[4];
	struct 
	{
		tU8 m_Digital_FM_Instance_1_Available:1;
		tU8 m_Digital_AM_Instance_1_Available:1;
		tU8 m_Analog_FM                      :1;
		tU8 m_Analog_AM                      :1;
		tU8 m_Digital_FM_Instance_2_Available:1;
		tU8 m_Digital_AM_Instance_2_Available:1;
		tU8 m_Digital_FM_Instance_3_Available:1;
		tU8 m_Digital_AM_Instance_3_Available:1;

		tU8 m_RBDS            :1;
		tU8 m_RBDS_Decode     :1;
		tU8 m_AIM             :1;
		tU8 m_Reserved1_Byte1 :1;
		tU8 m_AUX1_Audio_Input:1;
		tU8 m_Reserved2_Byte1 :3;

		tU8 m_Instance_1_capability:2;
		tU8 m_Instance_2_capability:2;
		tU8 m_Instance_3_capability:2;
		tU8 m_Conditional_Access   :1;
		tU8 m_reserved_byte2       :1;

		tU8 m_PSD_Decode       :1;
		tU8 m_reserved1_byte3  :1;
		tU8 m_Tagging          :1;
		tU8 m_Active_Alerts    :1;
		tU8 m_LOT_Off_Chip_Only:1;
		tU8 m_Reserved2_byte3  :3;
	} BitField;
} etalHDRXSWCnfgTy;

/*
 * etalReceiverStatusTy
 */
typedef struct
{
	tBool isValid;

	EtalBcastStandard currentStandard;
	tU32              supportedStandards;
	etalDiversTy      diversity;
	tU32              frequency;
	EtalDeviceType    tunerType;
	etalFrequencyBandInfoTy bandInfo;
	tBool             receiverMuted;
	etalDatapathTy    datapaths[ETAL_MAX_DATAPATH_PER_RECEIVER];
	etalSeekConfigTy  seekCfg;
	tBool			  isTuneInProgress;
	tBool             isEventFmSteroInProgress;
	tBool             isManualAFCheckInProgress;
	tBool             isManualSeekInProgress;
	tBool             isRDSInProgress;
	tBool			  isExternalRequestInProgress;
	tBool			  isExternalSeekRequestInProgress;
	tBool			  isExternalSeamlessEstimationRequestInProgress;
	tBool			  isExternalSeamlessSwitchingRequestInProgress;
	tBool			  isExternalTuneRequestInProgress;
	tBool             isExternalDABAnnouncementRequestInProgress;
	tBool             isExternalDABStatusRequestInProgress;
	tBool             isExternalDABDataStatusRequestInProgress;
	tBool             isExternalDABReconfigurationRequestInProgress;
	tBool             isSeekInProgress;
	tBool	   		  isSeamlessEstimationInProgress;
	tBool             isSeamlessSwitchingInProgress;

#ifdef CONFIG_ETAL_HAVE_ETALTML
	tBool			  isExternalLearnRequestInProgress;
	tBool			  isExternalScanRequestInProgress;
	tBool			  isExternalRDSRequestInProgress;
	tBool             isDecodedRDSInProgress;
	tBool             isRDSStrategyInProgress;	
	tBool             isTextInfoInProgress;
	tBool             isScanInProgress;
	tBool             isLearnInProgress;
	etalLearnConfigTy learnCfg;
	etalScanConfigTy  scanCfg;
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
	etalRdsSeekConfigTy	RdsSeekCfg;
	tBool			  isRdsSeekInProgress;
#endif
#endif
	// after a change band 
	// a tune would be required
	// put a flag to monitor is tuned is required
	// 
	tBool			  isTunedRequiredAfterChangeBand;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	struct
	{
		DABMW_mwAppTy application;
		tU32          enabledDataServiceBitmap;
		tU16          enabledAutonotificationEventBitmap;
		tU8           subch;
	} MDRConfig;
	tU8 autoNotificationbitmap;
	etalAutoNotificationStatusTy DABStatusNotif;
	etalAutoNotificationStatusTy DABAnnouncementNotif;
	etalAutoNotificationStatusTy DABAnnouncementRawNotif;
	etalAutoNotificationStatusTy DABReconfigurationNotif;
	etalAutoNotificationStatusTy DABDataStatusNotif;
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	struct
	{
		ETAL_HANDLE             tunerId;
		etalChannelTy           channel;
		etalRDSAttrInternal     rdsAttrInt;
		EtalProcessingFeatures  processingFeatures;
		EtalDISSStatusTy        dissStatus;
		EtalDebugVPAModeEnumTy  m_DebugVPAMode;
		etalAudioChannelTy      audioChannel;
	} CMOSTConfig;
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	struct
	{
		tyHDRADIOInstanceID instanceId;
		tU8                 acquisitionStatus;
		tU8 				audioInformation;
		tS8                 currentProgram;  // ranges 0 (MSC) to 7 (SPS7), or ETAL_INVALID_PROG
		tS8                 availablePrograms[ETAL_HD_MAX_PROGRAM_NUM];
		tU8                 programNum;
	} HDRADIOConfig;
#endif
} etalReceiverStatusTy;

/*
 * etalDCOPTy
 */
typedef struct
{
	EtalDeviceDesc     deviceDescr;
	/*! The tunerIndex of the STAR Device connected to the DCOP's
	 *  audio ouput, or ETAL_INVALID_HINDEX if the audio is routed
	 *  directly to the Host (in this case the audio commands are ignored).
	 *  This field is used to decide to which Tuner address the audio commands for an
	 *  ETAL_DATA_TYPE_DCOP_AUDIO Receiver.
	 *  The tunerIndex is the index in the etalTuner array, also
	 *  reported by the #etal_get_capabilities API */
	tU8                tunerIndexForAudioCommands;
} etalDCOPTy;

/*
 * etalTunerTy
 */
typedef struct
{
	EtalDeviceDesc     deviceDescr;
	etalFrontendDescTy frontEndList[ETAL_CAPA_MAX_FRONTEND_PER_TUNER];
} etalTunerTy;

/*
 * etalMonitorTy
 */
typedef struct
{
	tBool isValid;

	EtalBcastStandard           standard;
	EtalBcastQualityMonitorAttr requested;
	union
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		struct
		{
			/* indexes into the etalStatusTy.filters array */
			tU8 filterIndex[ETAL_MAX_QUALITY_PER_MONITOR];
			tS8 filterCountMonitor;
			etalFilterMaskTy filterMask;
			OSAL_tMSecond nextExpiration[ETAL_MAX_QUALITY_PER_MONITOR];
			tBool isDCOPMonitoringStarted;
		} MDR;
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		struct
		{
			/* one nextExpiration per each m_monitoredIndicators[] in requested */
			OSAL_tMSecond nextExpiration[ETAL_MAX_QUALITY_PER_MONITOR];
		} STARHD;
#endif
	} monitorConfig;
} etalMonitorTy;

/*
 *
 */
typedef struct
{
	ETAL_HANDLE hDatapath;
	tU16        packetAddress;
	tU8         subchId;
} etalDataServiceInfoTy;

/*!
 * \struct	etalStatusTy
 * 			Description of the ETAL Status
 */
typedef struct
{
	/*! TRUE if ETAL has been initialized */
	tBool isInitialized;

	/*! The attributes used by the ETAL API user to initialize ETAL */
	EtalHardwareAttr     hardwareAttr;
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	/*! The Tuner audio status.
	 *
	 *  STAR tuners include DAC and ADC
	 *  so potentially could process audio; whether a STAR in the
	 *  system actually produces audio depends on the application.
	 *
	 *  This information is not used for DOT tuners. */
	etalAudioIntfStatusTy audioInterfaceStatus[ETAL_CAPA_MAX_TUNER];
	ETAL_HANDLE audioSourceReceiver;
	EtalAudioSourceTy audioSource;
#endif
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
	/*! The Monitor descriptions */
	etalMonitorTy        monitors[ETAL_MAX_MONITORS];
	/*! The number of Monitors currently present in *monitors* */
	tU8                  monitorsCount;
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	/*! The main DAB Tune information; this field is also protected by #etalTuneStatusSem */
	etalDABTuneStatusTy  DABTuneStatus;
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
	/*! The Filters, needed to implement Monitors in non-DAB devices */
	etalDCOPFilterTy     filters[ETAL_MAX_FILTERS];
	/*! The number of Filters currently present in *filters* */
	tS8                  filtersCount;
#endif
	/*! The Data Service Info, used to associate (packet address, subch) to services
	 * for DAB Broadcast. See #ETAL_statusAddDataServiceInfo */
	etalDataServiceInfoTy dataServiceCache[ETAL_MAX_DATASERVICE_CACHE];
	/*! TRUE if PAD was requested to the DAB DCOP */
	tBool                PADActive;
#endif
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
	/*! Handle of Receivers configured for RDS reception
	 * through interrupt (instead of polling) */
    ETAL_HANDLE          hReceiverRDSIRQ[ETAL_MAX_RECEIVERS];
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	/*! The main DAB Tune information; this field is also protected by #etalTuneStatusSem */
	etalHDRXSWCnfgTy     HDSupportedServices;
#endif
} etalStatusTy;

/*!
 * \enum	etalReadWriteIndexInternalTy
 * 			The list of CMOST registers internally accessible though the
 * 			etal_read_parameter/etal_write_parameter interfaces.
 * 			Actual availability of some registers depends on the CMOST
 * 			flavour (single vs dual channel) and also on the
 * 			CMOST silicon and firmware version.
 */
typedef enum
{
	/* The first value in this enum continues from the last value
	 * of the exported index list, that is etalReadWriteIndexTy.
	 * This way all indexe-to-address conversions can be done
	 * by accessing a single array */

	/* XTAL alignment */
	IDX_CMT_tunApp0_tm_outSwitch = ETAL_IDX_CMT_MAX_EXTERNAL,
	IDX_CMT_tunApp0_tm_iqShift,
	IDX_CMT_bbpX_detFlags,
	IDX_CMT_bbpX_y1High	,
	IDX_CMT_systemConfig_tuneDetCompCoeff,
	/* etal_get_version command */
	IDX_CMT_mainY_st_version_info__0__,
	/* service following */
	IDX_CMT_tunApp0_fm_qd_quality,   /* combined Quality indicator computed by STAR */
	IDX_CMT_tunApp0_fm_qdAf_quality, /* combined Quality indicator computed by STAR for AF */
	IDX_CMT_tunApp1_fm_qd_quality,   /* combined Quality indicator computed by STAR */
	IDX_CMT_tunApp1_fm_qdAf_quality, /* combined Quality indicator computed by STAR for AF */

	ETAL_IDX_CMT_MAX_INTERNAL
} etalReadWriteIndexInternalTy;

#if defined (CONFIG_ETAL_HAVE_ETALTML)
// Etal Path information
// this is used to manage the Front End selection for each path
// by ranking in priority order the FE.
//
typedef struct
{
	EtalPathName 	  m_PathName;
	EtalBcastStandard m_PathStandard;
	tU8 			  m_NbTunerCombinaison;
	etalDiversTy      m_Diversity[ETAL_MAX_DIVERSITY];	
} etalPathTy;

/*
 * etalPathCapabilitiesTy
 */
typedef struct
{
	tU8				 m_maxPath;
	etalPathTy       m_Path[ETAL_MAX_PATH];
} etalPathCapabilitiesTy;
#endif

/*
 * etalSeekThresholdSettingModeTy
 */
typedef enum
{
    settingFromIndex = 0,
    settingFromValue = 1
} etalSeekThresholdSettingModeTy;

/*
 * EtalSCInfo
 */
typedef struct
{
	tU8         m_scIndex;
	tU16        m_scNumber;
	tU8         m_scids;
	tU8         m_subchId;
	tU16        m_packetAddress;
	tU16        m_userApplicationDataType[ETAL_DEF_MAX_USER_APPLICATION_DATA_TYPE];
	tU8         m_dataServiceType;
	tU8         m_scType;
	tU8         m_scLabelCharset;
	tChar       m_scLabel[ETAL_DEF_MAX_LABEL_LEN];
	tU16        m_scLabelCharflag;
} EtalSCExtendedInfo;

/*
 * EtalServiceComponentList
 */
typedef struct
{
	tU8                 m_scCount;
	EtalSCExtendedInfo  m_scInfo[ETAL_DEF_MAX_SC_PER_SERVICE];
} EtalServiceComponentExtendedList;

/***********************************
 *
 * Variables
 *
 **********************************/
extern OSAL_tSemHandle          etalApiStatusSem;
extern const EtalHwCapabilities etalCapabilities;
extern etalTunerTy              etalTuner[]; // cannot be const, RDSSlot is read/write
extern const etalDCOPTy         etalDCOP;
extern const tU32               etalFrontendCommandRouting_Tune[ETAL_BCAST_STD_NUMBER];
extern const tU32               etalFrontendCommandRouting_Quality[ETAL_BCAST_STD_NUMBER];
extern const EtalDeviceType     etalFrontendCommandRouting_BandSpecific[ETAL_BCAST_STD_NUMBER];
extern const EtalDeviceType     etalFrontendCommandRouting_RDS;
extern const etalBoardInitFunctionPtrTy etalBoardInitFunctionPtr;

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
extern OSAL_tSemHandle    etalDABSeekSem;
#endif //#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
#endif //#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR


#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
extern const tyHDRADIODeviceConfiguration etalHDDeviceConfig;
#endif

#if defined (CONFIG_ETAL_HAVE_ETALTML)
// add on path mgt
extern etalPathCapabilitiesTy etalPathPreferences;
#endif

/***********************************
 *
 * Function prototypes
 *
 **********************************/

/*
 * Global initialization
 */
tSInt ETAL_init_Light(const EtalHardwareAttr *hardware_attr, tBool power_up);
tVoid		ETAL_deinit_Light(tVoid);
tSInt       ETAL_init(const EtalHardwareAttr *init_params, tBool power_up);
tSInt 		ETAL_tuner_init(tU32 deviceID, const EtalTunerAttr *tuner_hardware_attr, tBool tunerIsAlreadyStarted);
tSInt 		ETAL_Dcop_init(const EtalDCOPAttr *dcop_hardware_attr, EtalDcopInitTypeEnum InitType);
#if defined(CONFIG_HOST_OS_FREERTOS)
tSInt		ETAL_Dcop_init_Light(const EtalDCOPAttr *dcop_hardware_attr, EtalDcopInitTypeEnum InitType);
#endif
tVoid       ETAL_initRollback(tBool power_up);
tSInt       ETAL_statusInitLock(tBool power_up);
tSInt       ETAL_statusDeinitLock(tBool power_down);
tSInt       ETAL_statusGetLock(tVoid);
tVoid       ETAL_statusReleaseLock(tVoid);
tVoid       ETAL_statusGetInternalLock(tVoid);
tVoid       ETAL_statusReleaseInternalLock(tVoid);
ETAL_STATUS ETAL_statusGetReceiverLockFromDatapath(ETAL_HANDLE hDatapath, ETAL_HANDLE *hReceiver);
tVoid       ETAL_configValidate(tVoid);
tVoid       ETAL_setBootFlashProgramModeMdr(tBool doFlashProgram, tChar *mdr_bootstrap_filename, tChar *mdr_program_firmware_filename);
tVoid       ETAL_setBootFlashDumpModeMdr(tBool doFlashDump);
tBool       ETAL_isAllFrontendFree(const ETAL_HANDLE *list, tU32 size);
tVoid 		ETAL_getDeviceDescription_HDRADIO(EtalDeviceDesc *deviceDescription);
tVoid       ETAL_getDeviceDescription_DAB(EtalDeviceDesc *deviceDescription);
tVoid       ETAL_configGetDCOPAudioTunerIndex(tU8 *tuner_index);
tVoid       ETAL_restart(tVoid); /* only used in test/, not needed for delvered library */

/* Reset*/
tVoid       ETAL_resetQualityContainer(EtalBcastStandard standard, EtalBcastQualityContainer *d);
tVoid       ETAL_resetHDQualityContainer_HDRADIO(EtalHdQualityEntries *d);
tVoid       ETAL_resetDABQualityContainer_MDR(EtalDabQualityEntries *d);
tVoid       ETAL_resetAmFmQualityContainer_CMOST(EtalFmQualityEntries *d);

/*
 * HANDLE processing
 */
ETAL_HANDLE ETAL_handleMakeTuner(ETAL_HINDEX index);
ETAL_HANDLE ETAL_handleMakeReceiver(ETAL_HINDEX index);
ETAL_HANDLE ETAL_handleMakeDatapath(ETAL_HINDEX receiverIndex, ETAL_HINDEX index);
ETAL_HANDLE ETAL_handleMakeMonitor(ETAL_HINDEX index);
ETAL_HANDLE ETAL_handleMakeFrontend(ETAL_HINDEX tunerIndex, ETAL_HINDEX channel);
tBool       ETAL_handleIsValid(ETAL_HANDLE hGeneric);
tBool       ETAL_handleIsReceiver(ETAL_HANDLE hGeneric);
ETAL_HINDEX ETAL_handleReceiverGetIndex(ETAL_HANDLE hReceiver);
tBool       ETAL_handleIsTuner(ETAL_HANDLE hGeneric);
ETAL_HINDEX ETAL_handleTunerGetIndex(ETAL_HANDLE hTuner);
tBool       ETAL_handleIsDatapath(ETAL_HANDLE hGeneric);
ETAL_HINDEX ETAL_handleDatapathGetIndex(ETAL_HANDLE hDatapath);
ETAL_HINDEX ETAL_handleDatapathGetReceiverIndex(ETAL_HANDLE hDatapath);
tBool       ETAL_handleIsMonitor(ETAL_HANDLE hGeneric);
ETAL_HINDEX ETAL_handleMonitorGetIndex(ETAL_HANDLE hDatapath);
ETAL_HANDLE ETAL_handleDatapathGetReceiver(ETAL_HANDLE hDatapath);
tBool       ETAL_handleIsFrontend(ETAL_HANDLE hGeneric);
ETAL_HANDLE ETAL_handleFrontendGetTuner(ETAL_HANDLE hFrontend);
ETAL_HINDEX ETAL_handleFrontendGetTunerIndex(ETAL_HANDLE hFrontend);
ETAL_HINDEX ETAL_handleFrontendGetChannel(ETAL_HANDLE hFrontend);

/*
 * Capabilities
 */
tU32         ETAL_cmdRoutingCheck(ETAL_HANDLE hReceiver, etalCommandTy cmd);
tVoid        ETAL_statusFillCapabilities(EtalHwCapabilities *capa);

/*
 * Thread entries
 */
#ifdef CONFIG_HOST_OS_TKERNEL
tVoid		ETAL_CallbackHandler_ThreadEntry(tSInt ,tPVoid);
tVoid		ETAL_DataHandler_ThreadEntry(tSInt ,tPVoid);
tVoid		ETAL_Control_ThreadEntry(tSInt ,tPVoid);
tVoid		ETAL_IRQ_ThreadEntry(tSInt ,tPVoid);
#else
tVoid       ETAL_CallbackHandler_ThreadEntry(tPVoid);
tVoid       ETAL_DataHandler_ThreadEntry(tPVoid);
tVoid       ETAL_Control_ThreadEntry(tPVoid);
tVoid       ETAL_IRQ_ThreadEntry(tPVoid);
#endif
/*
 * Callback processing
 */
tSInt       ETAL_callbackInit(tVoid);
tSInt       ETAL_callbackDeinit(tVoid);
tVoid       ETAL_callbackInvoke(tU32 handler_index, etalCallbackTy msg_type, ETAL_EVENTS event, tVoid *param, tU32 param_len);
/*
 * Data handler processing
 */
tSInt       ETAL_datahandlerInit(tVoid);
tSInt       ETAL_datahandlerDeinit(tVoid);
tSInt /*@alt void@*/ETAL_datahandlerInvoke(tU32 handler_index, ETAL_HANDLE hDatapath, tVoid *param, tU32 param_len, EtalDataBlockStatusTy *status);
/*
 * Internal interfaces
 */
tBool       ETAL_isValidFrequency(ETAL_HANDLE hReceiver, tU32 f);
ETAL_STATUS ETAL_tuneReceiverInternal(ETAL_HANDLE hReceiver, tU32 Frequency, etalCmdTuneActionTy dcop_action);
ETAL_STATUS ETAL_changeBandInternal(ETAL_HANDLE hGeneric, EtalFrequencyBand band, tU32 fmin, tU32 fmax, EtalProcessingFeatures processingFeatures, tBool isInternalOnly);
ETAL_STATUS ETAL_configReceiverInternal(ETAL_HANDLE *pReceiver, EtalReceiverAttr *pRecvCfg);
ETAL_STATUS ETAL_setTunerIdleInternal(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_configDatapathInternal(ETAL_HANDLE *pDatapath, const EtalDataPathAttr *pDatapathAttr);
ETAL_STATUS ETAL_destroyDatapathInternal(ETAL_HANDLE *pDatapath);


/*
 * ETAL internal status
 */
tBool             ETAL_statusIsInitialized(tVoid);
tVoid             ETAL_statusSetInitialized(tBool state);
tVoid             ETAL_statusDestroyReceiver(ETAL_HANDLE hReceiver);
ETAL_STATUS		  ETAL_destroyReceiverInternal(ETAL_HANDLE hReceiver);
tVoid             ETAL_statusInternalInit(const EtalHardwareAttr *init_params);
tSInt             ETAL_statusExternalInit(tVoid);
ETAL_HANDLE       ETAL_statusAddReceiverHandle(tVoid);
tVoid             ETAL_statusStopTuneRequest(tVoid);
tSInt             ETAL_statusStartRDSIrq(ETAL_HANDLE hReceiver);
tVoid             ETAL_statusStopRDSIrq(ETAL_HANDLE hReceiver);
tBool             ETAL_statusIsRDSIrqThreadActive(ETAL_HANDLE *hReceiver);
tBool             ETAL_statusGetRDSIrqhReceiver(ETAL_HANDLE tunerId, etalChannelTy channel, ETAL_HANDLE *phReceiver);
EtalCountryVariant ETAL_statusGetCountry(tVoid);
EtalNVMLoadConfig  ETAL_statusGetNvmLoadCfg(tVoid);
EtalNVMSaveConfig  ETAL_statusGetNvmSaveCfg(tVoid);

#ifdef CONFIG_ETAL_SUPPORT_CMOST
tVoid             ETAL_statusSetTunerAudioStatus(ETAL_HANDLE hTuner, etalAudioIntfStatusTy status);
etalAudioIntfStatusTy ETAL_statusGetTunerAudioStatus(ETAL_HANDLE hTuner);
tVoid             ETAL_statusSetAudioSource(ETAL_HANDLE hReceiver, EtalAudioSourceTy source);
tVoid             ETAL_statusGetAudioSource(ETAL_HANDLE *phReceiver, EtalAudioSourceTy *psource);
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
tVoid             ETAL_statusClearDABTuneStatus(tBool clear_all);
tVoid             ETAL_statusSetDABPAD(etalPADDLSTy *pad);
tBool             ETAL_statusGetDABPAD(etalPADDLSTy *pad, tBool *is_new);
tVoid             ETAL_statusSetDABService(tU32 UEId, tU32 service);
tSInt /*@alt void@*/ETAL_statusGetDABService(tU32 *UEId, tU32 *service, tBool *is_new);
tVoid             ETAL_statusSetDABServiceLabel(tChar *label, tU8 charSet);
tSInt /*@alt void@*/ETAL_statusGetDABServiceLabel(tChar *label, tU8 *charSet, tU32 max_len, tBool *is_new);
tSInt             ETAL_statusAddDataServiceInfo(ETAL_HANDLE hDatapath, tU16 packet_address, tU8 subchid);
tSInt             ETAL_statusRemoveDataServiceInfo(ETAL_HANDLE hDatapath, tU16 packet_address, tU8 subchid);
tVoid             ETAL_statusRemoveDataServiceInfoForDatapath(ETAL_HANDLE hDatapath);
ETAL_HANDLE       ETAL_statusGetDataServiceInfo(tU16 packet_address, tU8 subchid);
tBool             ETAL_statusIsPADActive(tVoid);
tVoid             ETAL_statusSetPADActive(tVoid);
tVoid             ETAL_statusResetPADActive(tVoid);
tVoid             ETAL_getDeviceConfig_DAB(tyDABDeviceConfiguration *deviceConfiguration);
tVoid 			  ETAL_ResetDabAutonotification_MDR(etalReceiverStatusTy *recvp);
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
tVoid             ETAL_statusSetHDSupportedServices(etalHDRXSWCnfgTy *rx_sw_cnfg);
tVoid             ETAL_statusGetHDSupportedServices(etalHDRXSWCnfgTy *rx_sw_cnfg);
tBool             ETAL_statusIsHDMRCSupportedServices(tVoid);
tVoid 			  ETAL_getDeviceConfig_HDRADIO(tyHDRADIODeviceConfiguration *deviceConfiguration);
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
EtalCbNotify      ETAL_statusHardwareAttrGetNotifycb(tVoid);
tVoid            *ETAL_statusHardwareAttrGetNotifyContext(tVoid);
tVoid             ETAL_statusHardwareAttrInit(const EtalHardwareAttr *hardware_attr);
tVoid			  ETAL_statusHardwareAttrTunerInit( tU32 deviceID, const EtalTunerAttr *tuner_hardware_attr);
tVoid 			  ETAL_statusHardwareAttrDcopInit(const EtalDCOPAttr *dcop_hardware_attr);
tVoid             ETAL_statusHardwareAttrBackup(EtalHardwareAttr *hardware_attr);
tBool             ETAL_statusHardwareAttrUseXTALAlignment(ETAL_HANDLE hTuner);
tU32              ETAL_statusHardwareAttrGetXTALAlignment(ETAL_HANDLE hTuner);
tBool             ETAL_statusHardwareAttrUseTunerImage(ETAL_HANDLE hTuner);
tVoid             ETAL_statusHardwareAttrGetTunerImage(ETAL_HANDLE hTuner, tU8 **firmware, tU32 *firmware_size);
tBool             ETAL_statusHardwareAttrUseDefaultParams(ETAL_HANDLE hTuner);
tBool             ETAL_statusHardwareAttrUseCustomParams(ETAL_HANDLE hTuner);
tVoid             ETAL_statusHardwareAttrGetCustomParams(ETAL_HANDLE hTuner, tU32 **params, tU32 *params_size);
EtalTraceConfig  *ETAL_statusHardwareAttrGetTraceConfig(tVoid);
tBool             ETAL_statusHardwareAttrIsDCOPActive(tVoid);
tBool             ETAL_statusHardwareAttrIsValidDCOPAttr(tVoid);
EtalDCOPAttr     *ETAL_statusHardwareAttrGetDCOPAttr(tVoid);
tBool             ETAL_statusHardwareAttrGetDCOPAttrDoFlashProgram(tVoid);
tBool             ETAL_statusHardwareAttrIsTunerActive(ETAL_HANDLE hTuner);
tVoid             ETAL_initStatusSetState(EtalInitState state);
EtalInitState     ETAL_initStatusGetState(tVoid);
tVoid             ETAL_initStatusSetDCOPStatus(EtalDeviceStatus status);
tVoid             ETAL_initStatusGetDCOPStatus(EtalDeviceStatus *status);
tVoid             ETAL_initStatusSetTunerStatus(tU32 i, EtalDeviceStatus status);
tSInt             ETAL_initStatusGetTunerStatus(tU32 i, EtalDeviceStatus *status);
tBool             ETAL_initStatusIsTunerStatusError(tU32 index);
tBool 			  ETAL_initStatusIsTunerStatusReadyToUse(tU32 index);
tVoid             ETAL_initStatusSetNonFatal(EtalNonFatalError warn);
tVoid             ETAL_initStatusSetTunerVersion(tU32 i, tChar *vers);
tBool             ETAL_initStatusIsCompatibleTunerVersion(tU32 i);
tVoid             ETAL_initStatusGet(EtalInitStatus *dst);

/*
 * Internal callback management
 */
tVoid             ETAL_intCbInit(tVoid);
ETAL_STATUS       ETAL_intCbRegister(etalIntCbCallTy mode, etalIntCbFuncPtrTy fptr, ETAL_HANDLE hGeneric, tU32 context);
ETAL_STATUS       ETAL_intCbRegisterPeriodic(etalIntCbPeriodicFuncPtrTy fptr, ETAL_HANDLE hGeneric, tU32 delay_ms);
tBool             ETAL_intCbIsRegisteredPeriodic(etalIntCbPeriodicFuncPtrTy fptr, ETAL_HANDLE hGeneric);
tBool             ETAL_intCbIsRegistered(etalIntCbCallTy mode, etalIntCbFuncPtrTy fptr, ETAL_HANDLE hGeneric);
ETAL_STATUS /*@alt void@*/ETAL_intCbDeregister(etalIntCbCallTy mode, etalIntCbFuncPtrTy fptr, ETAL_HANDLE hGeneric);
tVoid             ETAL_intCbDeregisterDatapath(ETAL_HANDLE hDatapath);
ETAL_STATUS /*@alt void@*/ETAL_intCbDeregisterPeriodic(etalIntCbPeriodicFuncPtrTy fptr, ETAL_HANDLE hGeneric);
tVoid /*@alt void@*/ETAL_intCbDeregisterPeriodicReceiver(ETAL_HANDLE hReceiver);
tVoid             ETAL_intCbDeregisterPeriodicDatapath(ETAL_HANDLE hDatapath);
tVoid             ETAL_intCbScheduleCallbacks(ETAL_HANDLE hReceiver, etalIntCbCallTy mode, tVoid *param, tU32 param_len);

tVoid             ETAL_intCbSchedulePeriodicCallbacks(tVoid);

/*
 * Monitor
 */
etalMonitorTy    *ETAL_statusGetMonitor(ETAL_HANDLE hMonitor);
tU32              ETAL_statusCountIndicatorsForMonitor(etalMonitorTy *monp);
tBool             ETAL_statusIsValidMonitorHandle(ETAL_HANDLE *pMonitor);
tBool             ETAL_statusIsValidMonitor(const EtalBcastQualityMonitorAttr *q);
tSInt             ETAL_statusDestroyMonitor(ETAL_HANDLE hMonitor);
tSInt             ETAL_statusCreateModifyMonitor(ETAL_HANDLE* pMonitor, const EtalBcastQualityMonitorAttr* pMonitorAttr);
ETAL_HANDLE       ETAL_statusGetReceiverFromMonitor(etalMonitorTy *pmon);
tU32              ETAL_statusGetCountMonitor(tVoid);

/*
 * Filters
 */
tVoid             ETAL_statusFilterCountIncrement(tVoid);
tVoid             ETAL_statusFilterCountDecrement(tVoid);
tS32              ETAL_statusFilterCountGet(tVoid);
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
etalDCOPFilterTy *ETAL_statusFilterGet(tU32 filter_index);
#endif
#endif

/*
 * Tuner
 */
tSInt            ETAL_tunerInitLock(tVoid);
tSInt            ETAL_tunerDeinitLock(tVoid);
ETAL_STATUS      ETAL_tunerGetLock(ETAL_HANDLE hTuner);
tVoid            ETAL_tunerReleaseLock(ETAL_HANDLE hTuner);
tVoid            ETAL_tunerInitRDSSlot(tVoid);
tU32             ETAL_tunerGetNumberOfTuners(tVoid);
tU32             ETAL_tunerGetFEsupportedStandards(ETAL_HANDLE hFrontend);
tU32             ETAL_tunerGetChannelSupportedDataTypes(ETAL_HANDLE hTuner, etalChannelTy channel);
tU32             ETAL_tunerGetFEsupportedDataTypes(ETAL_HANDLE hFrontend);
tSInt 			 ETAL_tunerGetAddress(ETAL_HANDLE hTuner, EtalDeviceDesc *deviceDescription);
#ifndef CONFIG_COMM_DRIVER_EXTERNAL
tSInt 			 ETAL_getDeviceConfig_CMOST(ETAL_HANDLE hTuner, tyCMOSTDeviceConfiguration *deviceConfiguration);
#else
tSInt            ETAL_getDeviceConfigExternal_CMOST(ETAL_HANDLE hTuner, tyCMOSTDeviceConfiguration *deviceConfiguration);
#endif
EtalDeviceType   ETAL_tunerGetType(ETAL_HANDLE hTuner);
EtalDeviceType   ETAL_tunerDeviceStringToType(tChar *desc);
tSInt            ETAL_tunerSearchAllFrontend(const ETAL_HANDLE *list, tU32 size, ETAL_HANDLE *hTuner);
tSInt            ETAL_tunerSearchFrontend(ETAL_HANDLE hFrontend);
#ifdef CONFIG_ETAL_SUPPORT_CMOST
ETAL_HANDLE      ETAL_tunerGetFirstSTAR_CMOST(tVoid);
ETAL_HANDLE      ETAL_tunerGetFirstDOT_CMOST(tVoid);
etalChannelTy    ETAL_FEHandleToChannel_CMOST(ETAL_HANDLE hFrontend);
#endif
ETAL_HANDLE      ETAL_tunerGetFirstTuner_CMOST(tVoid);
#ifdef CONFIG_MODULE_INTEGRATED
ETAL_HANDLE      ETAL_tunerGetSecondTuner_CMOST(tVoid);
#endif
etalFrontendDescTy *ETAL_tunerGetFrontend(ETAL_HANDLE hFrontend);
tBool            ETAL_tunerIsValidHandle(ETAL_HANDLE hTuner);
tVoid 			 ETAL_IRQCallbackFunction_CMOST_TUNER_ID_0(tVoid);
tVoid 			 ETAL_IRQCallbackFunction_CMOST_TUNER_ID_1(tVoid);

/*
 * Receiver
 */
tVoid       ETAL_receiverInit(tVoid);
tSInt       ETAL_receiverInitLock(tVoid);
tSInt       ETAL_receiverDeinitLock(tVoid);
ETAL_STATUS ETAL_receiverGetLock(ETAL_HANDLE hReceiver);
tVoid       ETAL_receiverReleaseLock(ETAL_HANDLE hReceiver);
tVoid       ETAL_receiverResetStatus(ETAL_HANDLE hReceiver);
tVoid       ETAL_receiverInitPtr(etalReceiverStatusTy *recvp);
tBool       ETAL_receiverIsValidHandle(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverIsValidRDSHandle(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverIsCompatible(etalReceiverStatusTy *newp, etalReceiverStatusTy *old);
tVoid       ETAL_receiverAddInternal(ETAL_HANDLE *phReceiverReconf, etalReceiverStatusTy *stat);
tVoid       ETAL_receiverSetFrequency(ETAL_HANDLE hReceiver, tU32 freq, tBool resetRDS);
tU32        ETAL_receiverGetFrequency(ETAL_HANDLE hReceiver);
tVoid       ETAL_receiverSetBandInfo(ETAL_HANDLE hReceiver, EtalFrequencyBand band, tU32 bandMin, tU32 bandMax, tU32 step);
tSInt       ETAL_receiverGetBandInfo(ETAL_HANDLE hReceiver, etalFrequencyBandInfoTy *band_info);
tU32        ETAL_receiverGetSupportedStandard(ETAL_HANDLE hReceiver);
tSInt       ETAL_receiverGetTunerId(ETAL_HANDLE hReceiver, ETAL_HANDLE *phTuner);
tSInt       ETAL_receiverGetChannel(ETAL_HANDLE hReceiver, etalChannelTy *pchannel);
tVoid       ETAL_receiverSetMute(ETAL_HANDLE hReceiver, tBool mute);
tBool       ETAL_receiverIsMute(ETAL_HANDLE hReceiver);
tVoid       ETAL_receiverInitFMAudioStereo(ETAL_HANDLE hReceiver);
tVoid       ETAL_receiverSetFMAudioStereo(ETAL_HANDLE hReceiver, tBool stereo);
tBool       ETAL_receiverIsFMAudioStereo(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverIsFMAudioMono(ETAL_HANDLE hReceiver);
tVoid       ETAL_receiverSetSpecial(ETAL_HANDLE hReceiver, etalCmdSpecialTy cmd, etalCmdActionTy action);
tBool       ETAL_receiverIsFrontendUsed(etalReceiverStatusTy *recvp, ETAL_HANDLE hFrontend);
ETAL_STATUS ETAL_receiverFreeAllFrontend(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_receiverStop(ETAL_HANDLE hReceiver);
tBool		ETAL_receiverIsUsed(ETAL_HANDLE hReceiver);
tU32 		ETAL_receiverLowerBandLimit(ETAL_HANDLE hReceiver);
tU32 		ETAL_receiverUpperBandLimit(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverIsRDSCapable(ETAL_HANDLE hReceiver);
ETAL_HANDLE ETAL_receiverSearchActiveSpecial(etalCmdSpecialTy cmd);
ETAL_HANDLE ETAL_receiverSearchActiveSpecialAndStd(etalCmdSpecialTy cmd, EtalBcastStandard std);
tBool       ETAL_receiverIsSpecialInProgress(ETAL_HANDLE hReceiver, etalCmdSpecialTy cmd);
tBool       ETAL_receiverGetInProgress(etalReceiverStatusTy *recvp, etalCmdSpecialTy cmd);
ETAL_HANDLE ETAL_receiverSearchFromApplication(tU8 app);
ETAL_HANDLE ETAL_receiverSearchFromTunerId(ETAL_HANDLE hTuner, ETAL_HANDLE hReceiver_start);
tBool       ETAL_receiverAllocateIfFree(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverHaveFreeSpace(tVoid);
tVoid       ETAL_receiverStopAllSpecial(etalCmdSpecialTy cmd);
ETAL_STATUS ETAL_receiverSetStandard(ETAL_HANDLE hReceiver, EtalBcastStandard std);
tSInt       ETAL_receiverGetRDSSlot(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverSupportsQuality(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverSupportsRDS(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverSupportsAudio(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_receiverGetRDSAttrInt(ETAL_HANDLE hReceiver, etalRDSAttrInternal **pRDSAttrInt);
ETAL_STATUS ETAL_receiverGetRDSAttr(ETAL_HANDLE hReceiver, etalRDSAttr **pRDSAttr);
ETAL_STATUS ETAL_receiverSetRDSAttr(ETAL_HANDLE hReceiver, etalRDSAttr *RDSAttr);
tSInt       ETAL_receiverSetDataService_MDR(ETAL_HANDLE hReceiver, EtalDataServiceType service);
tSInt       ETAL_receiverClearDataService_MDR(ETAL_HANDLE hReceiver, EtalDataServiceType service);
tSInt       ETAL_receiverGetDataServices_MDR(ETAL_HANDLE hReceiver, tU32 *serviceBitmap);
tSInt       ETAL_receiverSetAutonotification_MDR(ETAL_HANDLE hReceiver, EtalAutonotificationEventType eventBitMap);
tSInt       ETAL_receiverGetAutonotification_MDR(ETAL_HANDLE hReceiver, tU16 *eventBitMap);
ETAL_STATUS ETAL_receiverGetProcessingFeatures(ETAL_HANDLE hReceiver, EtalProcessingFeatures *proc_features);
ETAL_STATUS ETAL_receiverSetProcessingFeatures(ETAL_HANDLE hReceiver, EtalProcessingFeatures proc_features);
tVoid       ETAL_receiverSetDefaultProcessingFeatures(EtalBcastStandard std, tU8 frontendsz, EtalProcessingFeatures *proc_features);
ETAL_STATUS ETAL_receiverGetConfigVPAMode(ETAL_HANDLE hReceiver, tBool *configVpaMode);
ETAL_STATUS ETAL_receiverGetDebugVPAMode(ETAL_HANDLE hReceiver, EtalDebugVPAModeEnumTy *debugVpaMode);
ETAL_STATUS ETAL_receiverSetConfigVPAMode(ETAL_HANDLE hReceiver, tBool configVpaMode);
ETAL_STATUS ETAL_receiverSetDebugVPAMode(ETAL_HANDLE hReceiver, EtalDebugVPAModeEnumTy debugVpaMode);
ETAL_STATUS ETAL_receiverGetDISSStatus(ETAL_HANDLE hReceiver, EtalDISSStatusTy *diss_status);
ETAL_STATUS ETAL_receiverSetDISSStatus(ETAL_HANDLE hReceiver, EtalDISSStatusTy *diss_status);
tVoid       ETAL_receiver_check_state_periodic_callback(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_get_service_list(ETAL_HANDLE hReceiver, tU32 eid, tBool bGetAudioServices, tBool bGetDataServices, EtalServiceList *pServiceList);
tSInt       ETAL_receiverSetSubch_MDR(ETAL_HANDLE hReceiver, tU8 subch);
tSInt       ETAL_receiverClearSubch_MDR(ETAL_HANDLE hReceiver);
tSInt       ETAL_receiverGetSubch_MDR(ETAL_HANDLE hReceiver, tU8 *psubch);

/*
 * Datapath
 */
ETAL_STATUS       ETAL_enable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap, tU32 *EnabledServiceBitmap, EtalDataServiceParam ServiceParameters);
ETAL_STATUS       ETAL_disable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap);
ETAL_STATUS       ETAL_config_datapath(ETAL_HANDLE *pDatapath, const EtalDataPathAttr *pDatapathAttr);
ETAL_STATUS       ETAL_destroy_datapath(ETAL_HANDLE *pDatapath);
ETAL_HANDLE       ETAL_receiverAddDatapath(const EtalDataPathAttr *pDatapathAttr);
ETAL_HANDLE       ETAL_receiverGetDatapathFromDataType(ETAL_HANDLE hReceiver, EtalBcastDataType type);
ETAL_HANDLE       ETAL_receiverGetFromDatapath(ETAL_HANDLE hDatapath);
EtalBcastDataType ETAL_receiverGetDataTypeForDatapath(ETAL_HANDLE hDatapath);
EtalSink         *ETAL_receiverDatapathGetSink(ETAL_HANDLE hDatapath);
tVoid             ETAL_receiverDatapathDestroy(ETAL_HANDLE hDatapath);
tSInt             ETAL_receiverDatapathSetType(ETAL_HANDLE hDatapath, EtalDataServiceType type);
tSInt             ETAL_receiverDatapathRemoveType(ETAL_HANDLE hDatapath, EtalDataServiceType type);
tBool             ETAL_receiverDatapathIsRawRDS(ETAL_HANDLE hDatapath);
tBool             ETAL_receiverDatapathIsValid(ETAL_HANDLE hDatapath);
tBool             ETAL_receiverDatapathIsValidAndEmpty(ETAL_HANDLE hDatapath);
tBool             ETAL_receiverDatapathTypeIs(ETAL_HANDLE hDatapath, EtalBcastDataType type);
tVoid             ETAL_receiverDestroyAllDatapathsForReceiver(ETAL_HANDLE hReceiver);

/*
 * auto notification
 */
ETAL_STATUS       ETAL_setup_autonotification(ETAL_HANDLE hReceiver, tU16 eventBitmap, tU16 *enabledEventBitmap);
ETAL_STATUS       ETAL_updateAutonotification(ETAL_HANDLE hReceiver, EtalAutonotificationEventType eventBitMap);

#if defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
tSInt       ETAL_getManualSeekFSMLock(tVoid);
tSInt       ETAL_releaseManualSeekFSMLock(tVoid);
#endif //CONFIG_ETAL_HAVE_MANUAL_SEEK || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_ALTERNATE_FREQUENCY) || defined (CONFIG_ETAL_HAVE_ALL_API)
ETAL_STATUS ETAL_AF_check(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p);
ETAL_STATUS ETAL_AF_switch(ETAL_HANDLE hReceiver, tU32 alternateFrequency);
ETAL_STATUS ETAL_AF_start(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p);
ETAL_STATUS ETAL_AF_end(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* p);
ETAL_STATUS ETAL_get_AF_quality(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
ETAL_STATUS ETAL_AF_check_and_get_AF_quality(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p);
ETAL_STATUS ETAL_AF_search_manual(ETAL_HANDLE hReceiver, tU32 antennaSelection, tU32 *AFList, tU32 nbOfAF, EtalBcastQualityContainer* AFQualityList);
#endif // defined (CONFIG_ETAL_HAVE_ALTERNATE_FREQUENCY) || defined (CONFIG_ETAL_HAVE_ALL_API)

/*
 * Seamless estimation and seamless switching
 */
ETAL_STATUS ETAL_seamless_estimation_start_internal(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig_ptr);
ETAL_STATUS ETAL_seamless_estimation_stop_internal(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS);
ETAL_STATUS ETAL_seamless_switching_internal(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig_ptr);

/*
 * Seek interfaces
 */
ETAL_STATUS ETALTML_get_internal_seek_thresholds(ETAL_HANDLE hReceiver, EtalSeekThreshold* seekThreshold);
ETAL_STATUS ETALTML_seek_start_internal(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, etalSeekHdModeTy seekHDSPS, tBool updateStopFrequency);
ETAL_STATUS ETALTML_seek_stop_internal(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode);
ETAL_STATUS ETALTML_seek_continue_internal(ETAL_HANDLE hReceiver);
tVoid ETALTML_InitAutoSeekStatus(EtalBcastStandard standard, EtalSeekStatus* extAutoSeekStatus);
tVoid ETALTML_Report_Seek_Info(ETAL_HANDLE hReceiver, EtalSeekStatus* extAutoSeekStatus);
tVoid ETALTML_stop_and_send_event(ETAL_HANDLE hReceiver, etalSeekConfigTy *seekCfgp, etalSeekStatusTy status);
ETAL_STATUS ETALTML_checkSeekStartParameters(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, etalSeekHdModeTy seekHDSPS);
ETAL_STATUS ETALTML_checkSeekStopParameters(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode);
ETAL_STATUS ETALTML_checkSeekContinueParameters(ETAL_HANDLE hReceiver);
ETAL_STATUS ETALTML_checkSetSeekThresholdsParameters(ETAL_HANDLE hReceiver, EtalSeekThreshold* seekThreshold);
tVoid ETALTML_SeekStatusPeriodicFuncInternal(ETAL_HANDLE hGeneric);
tVoid ETAL_SeekIntCbFuncDabSync(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
tSInt ETAL_DABSeekTaskInit(tVoid);
tSInt ETAL_DABSeekTaskDeinit(tVoid);
tVoid ETAL_DABSeekStopTask(tVoid);
#endif //#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
#endif //#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
tVoid       ETAL_receiverSetRadioInfo_HDRADIO(ETAL_HANDLE hReceiver, tU8 acq_status, tS8 curr_prog, tU8 avail_prog, tU8 audio_information);
tVoid       ETAL_receiverGetRadioInfo_HDRADIO(ETAL_HANDLE hReceiver, tU8 *acq_status, tS8 *curr_prog, tU32 *avail_prog_num, tU8 *pO_audioInformation);
tVoid       ETAL_receiverSetCurrentProgram_HDRADIO(ETAL_HANDLE hReceiver, tS8 prog);
tS8         ETAL_receiverGetService_HDRADIO(ETAL_HANDLE hReceiver, tU32 index);
tBool       ETAL_receiverHasService_HDRADIO(ETAL_HANDLE hReceiver, tS8 prog);
tBool       ETAL_receiverHasDigitalAudio_HDRADIO(ETAL_HANDLE hReceiver);
tBool       ETAL_receiverHasSystemData_HDRADIO(ETAL_HANDLE hReceiver);
tSInt       ETAL_receiverSelectProgram_HDRADIO(ETAL_HANDLE hReceiver, tS8 prog);
tBool       ETAL_seekHDNeedsTune_HDRADIO(ETAL_HANDLE hReceiver, etalSeekDirectionTy dir);
tSInt       ETAL_receiverGetHdInstance(ETAL_HANDLE hReceiver, tyHDRADIOInstanceID *pinstanceId);
#endif
EtalDeviceType        ETAL_receiverGetTunerType(ETAL_HANDLE hReceiver);
EtalBcastStandard     ETAL_receiverGetStandard(ETAL_HANDLE hReceiver);
etalReceiverStatusTy *ETAL_receiverGet(ETAL_HANDLE hReceiver);

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
ETAL_STATUS ETAL_start_RDS(ETAL_HANDLE hReceiver, etalRDSAttr *pRDSAttr);
ETAL_STATUS ETAL_stop_RDS(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_resume_RDS(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_suspend_RDS(ETAL_HANDLE hReceiver);

tVoid       ETAL_RDSRawPeriodicFunc(ETAL_HANDLE hGeneric);
#endif /* CONFIG_ETAL_SUPPORT_CMOST_STAR */

/*
 * Misc
 */
tVoid       ETAL_utilitySetU8(tU8 *buf, tU32 off, tU8 val);
tU8         ETAL_utilityGetU8(tU8 *buf, tU32 off);
tVoid       ETAL_utilitySetU16(tU8 *buf, tU32 off, tU16 val);
tU16        ETAL_utilityGetU16(tU8 *buf, tU32 off);
tVoid       ETAL_utilitySetU24(tU8 *buf, tU32 off, tU32 val);
tU32        ETAL_utilityGetU24(tU8 *buf, tU32 off);
tVoid       ETAL_utilitySetU32(tU8 *buf, tU32 off, tU32 val);
tU32        ETAL_utilityGetU32(tU8 *buf, tU32 off);
ETAL_STATUS ETAL_utilityGetDefaultBandLimits(EtalFrequencyBand band, tU32 *bandMin, tU32 *bandMax, tU32 *step);
/*
 * Print utilities
 */
tSInt       ETAL_tracePrintAsciiLUN(tU8 *buf);
tSInt       ETAL_tracePrintInit(tVoid);
tVoid       ETAL_tracePrintDeInit(tVoid);
tVoid       ETAL_tracePrint(tU32 u32Level, tU32 u32Class, tCString coszFormat);
tVoid       ETAL_tracePrintVersion(EtalVersion *version);
tVoid       ETAL_traceEvent(etalCallbackTy msg_type, ETAL_EVENTS event, tVoid *param);
tVoid       ETAL_traceDataPath(ETAL_HANDLE hDatapath, tVoid *param, tU32 param_size, EtalDataBlockStatusTy *status);
tVoid       ETAL_tracePrintQuality(tU32 level, EtalBcastQualityContainer *q);
tVoid       ETAL_tracePrintBuffer(tU32 u32Level, tU32 u32Class, tChar *prefix, tU8* pBuffer, tU32 len); // normally under #if 0
tCString    ETAL_MsgTypeToString(etalCallbackTy type);
tCString    ETAL_EventToString(ETAL_EVENTS ev);
tCString    ETAL_STATUS_toString(ETAL_STATUS s);
tCString    ETAL_commErrToString(EtalCommErr err);
ETAL_STATUS ETAL_traceConfig(EtalTraceConfig *config);


/*
 * Error handling
 */
tVoid       ETAL_sendCommunicationErrorEvent(ETAL_HANDLE hReceiver, EtalCommErr err, tU32 err_raw, tU8 *buf, tU32 buf_len);

/*
 * Board configuration, not to be called directly
 */
tSInt       ETAL_InitCMOSTtoMDRInterface(tVoid);
tSInt       ETAL_InitCMOSTtoMDRInterface_MTD(tVoid);
tSInt       ETAL_InitCMOSTtoHDRADIOInterface(tVoid);
tSInt       ETAL_InitCMOSTtoHDRADIOInterface_MTD(tVoid);
tSInt       ETAL_InitCMOSTOnlyAudioInterface(tVoid);
tSInt       ETAL_getTunerIdForAudioCommands(ETAL_HANDLE hReceiver, ETAL_HANDLE *hTuner);

// audio 
ETAL_STATUS ETAL_audioSelectInternal(ETAL_HANDLE hReceiver, EtalAudioSourceTy src);

// mute/unmute
ETAL_STATUS ETAL_mute(ETAL_HANDLE hReceiver, tBool muteFlag);

// even FM stereo start/stop
ETAL_STATUS ETAL_event_FM_stereo_start(ETAL_HANDLE hReceiver);
ETAL_STATUS ETAL_event_FM_stereo_stop(ETAL_HANDLE hReceiver);

// Read - Write parameters
ETAL_STATUS ETAL_read_parameter_internal(ETAL_HANDLE hReceiver, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength);
ETAL_STATUS ETAL_read_parameter_nolock(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength);
ETAL_STATUS ETAL_write_parameter_internal(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *paramValueArray, tU16 length);

// Quality 
ETAL_STATUS ETAL_get_reception_quality_internal(ETAL_HANDLE hReceiver, EtalBcastQualityContainer *pBcastQuality);
ETAL_STATUS ETAL_get_channel_quality_internal(ETAL_HANDLE hReceiver, EtalBcastQualityContainer *pBcastQuality);

tVoid ETAL_runtimecheck(tVoid);

// utils 

DABMW_mwCountryTy DABMW_GetCountry (tVoid);

tVoid DABMW_SetCountry (DABMW_mwCountryTy country);

DABMW_systemBandsTy DABMW_GetReceiverTableSystemBand (ETAL_HANDLE hReceiver, tU32 frequency, DABMW_mwCountryTy countryId);

tU32 DABMW_GetNextFrequencyFromFreq (tU32 frequency, DABMW_systemBandsTy systemBand, tBool up);


tU32 DABMW_GetSystemBandMinFreq (DABMW_systemBandsTy bandValue);

tU32 DABMW_GetSystemBandMaxFreq (DABMW_systemBandsTy bandValue);


tU32 DABMW_GetBandInfoTable_step (DABMW_systemBandsTy bandValue, DABMW_mwCountryTy countryId);

tBool DABMW_IsFMBand (DABMW_systemBandsTy band);

tBool DABMW_IsAMBand (DABMW_systemBandsTy band);

tBool DABMW_IsDABBand (DABMW_systemBandsTy band);

EtalFrequencyBand DABMW_TranslateDabmwBandToEtalBand(DABMW_systemBandsTy vI_dabmwBand);

DABMW_systemBandsTy DABMW_TranslateEtalBandToDabmwBand(EtalFrequencyBand vI_etalBand);

tChar* ETAL_getIPAddressForExternalDriver(tVoid);

// debug
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
ETAL_STATUS ETAL_debugSetDISSInternal(ETAL_HANDLE hReceiver, etalChannelTy tuner_channel, EtalDISSMode mode, tU8 filter_index);
ETAL_STATUS ETAL_debugGetWSPStatusInternal(ETAL_HANDLE hReceiver, EtalWSPStatus *WSPStatus);
ETAL_STATUS ETAL_debugVPAControlInternal(ETAL_HANDLE hReceiver, tBool status, ETAL_HANDLE *hReceiver_bg);
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

#if defined(CONFIG_HOST_OS_TKERNEL)  
// IRQ entry
extern tVoid ETAL_IRQ_EntryTuner1(void);
#endif


// 

#if defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
#include "etalinternal_mdr.h"
#endif
#if defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#include "etalinternal_hdradio.h"
#endif
#if defined(CONFIG_ETAL_SUPPORT_CMOST)
#include "etalinternal_cmost.h"
#endif
#ifdef CONFIG_ETAL_HAVE_ETALTML
	#include "etaltmlinternal.h"
#endif

#ifdef __cplusplus
}
#endif
