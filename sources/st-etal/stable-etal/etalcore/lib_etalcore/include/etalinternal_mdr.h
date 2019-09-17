//!
//!  \file 		etalinternal_mdr.h
//!  \brief 	<i><b>ETAL communication, private header</b></i>
//!  \details	ETAL internal and external communications, MDR-specific definitions
//!

#ifdef CONFIG_COMM_DRIVER_EMBEDDED
#include "connection_modes.h"
#include "steci_defines.h"
#include "steci_helpers.h"
#include "steci_protocol.h"
#endif // CONFIG_COMM_DRIVER_EMBEDDED

#ifndef CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD
#ifdef CONFIG_COMM_DRIVER_EMBEDDED
#include "steci_lld.h"
#include "dcop_boot.h"
#endif // CONFIG_COMM_DRIVER_EMBEDDED
#endif // !CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD


/***********************************
 *
 * Defines
 *
 **********************************/
/* DABMW Notification and Response status */
#define DABMW_CMD_STATUS_OK                     0x00
#define DABMW_CMD_STATUS_ERR_CHECKSUM           0x04
#define DABMW_CMD_STATUS_ERR_TIMEOUT            0x05
#define DABMW_CMD_STATUS_ERR_HEADER0_FORMAT     0x07
#define DABMW_CMD_STATUS_ERR_QUEUE_FULL         0x08
#define DABMW_CMD_STATUS_ERR_PARAM_WRONG        0x09
#define DABMW_CMD_STATUS_ERR_ILLEGAL_CMDNUM     0x0A
#define DABMW_CMD_STATUS_ERR_RESERVED_CMDNUM    0x0B
#define DABMW_CMD_STATUS_ERR_DUPLICATED_AUTO    0x0C
#define DABMW_CMD_STATUS_ERR_PAYLOAD_TOO_LONG   0x0D
#define DABMW_CMD_STATUS_ERR_UNAVAILABLE_FNCT   0x0E
#define DABMW_CMD_STATUS_ERR_WRONG_CMD_LENGHT   0x0F
#define DABMW_CMD_STATUS_ERR_GENERIC_FAILURE    0x10
#define DABMW_CMD_STATUS_ERR_FNCT_DISABLED      0x11
#define DABMW_CMD_STATUS_ERR_APP_NOT_SUPPORTED  0x12
#define DABMW_CMD_STATUS_ERR_WRONG_STOP_BIT     0x13
#define DABMW_CMD_STATUS_ERR_CMD_IS_NOT_ONGOING 0x14
#define DABMW_CMD_STATUS_ERR_RADIO_IS_NOT_ON    0x15
#define DABMW_CMD_STATUS_ERR_NO_MEMORY_AVAIL    0x16
#define DABMW_CMD_STATUS_ERR_NVM_FAILED         0x17
#define DABMW_CMD_STATUS_ERR_LAST_ERROR         0x1F
#define DABMW_CMD_STATUS_RSP_WITHOUT_PAYLOAD    0x20
#define DABMW_CMD_STATUS_RSP_NO_DATA_AVAILABLE  0x21
#define DABMW_CMD_STATUS_NO_RSP_TO_SEND         0x22
#define DABMW_CMD_STATUS_OPERATION_ONGOING      0x23
#define DABMW_CMD_STATUS_OPERATION_FAILED       0x24
#define DABMW_CMD_STATUS_ONGOING_NO_RSP_TO_SEND 0x25
#define DABMW_CMD_STATUS_OTHER_CMD_WAITING      0x26
#define DABMW_CMD_STATUS_LASTOPERATION_STOPPED  0x27

#define ETAL_DABMW_RESPONSE_WITH_PAYLOAD_LEN       4
#define ETAL_DABMW_RESPONSE_NO_PAYLOAD_LEN         3
#define ETAL_DABMW_HEADER_HAS_LONG_PAYLOAD(_d_) (((_d_)[0] & 0x08) == (tU8)0x08)

#define ETAL_MDR_COMMAND_ID(_buf)    ((((tU16)(_buf)[1] & 0x03) << 8) | ((_buf)[2] & 0xFF))
#define ETAL_MDR_IS_NOTIFICATION(_buf)     ((((_buf)[0] & 0x40) >> 6) == (tU8)0)
#define ETAL_MDR_IS_FAST_NOTIFICATION(_buf) (((_buf)[0] & 0x04) == (tU8)0x04)
#define ETAL_MDR_FAST_NOTIF_HAS_DATA(_buf)  (((_buf)[0] & 0x02) == (tU8)0x02)
#define ETAL_MDR_IS_RESPONSE(_buf)         ((((_buf)[0] & 0x40) >> 6) == (tU8)1)
#define ETAL_MDR_NOTIFICATION_STATUS(_buf) (tU32)((((_buf)[1]) & 0xFC) >> 2)
#define ETAL_MDR_RESPONSE_STATUS(_buf)     (ETAL_MDR_NOTIFICATION_STATUS(_buf))
#define ETAL_MDR_RESPONSE_PAYLOAD_OFFSET           4

#define ETAL_MDR_GETMONITORDABINFO             0x003
#define ETAL_MDR_GETMONITORDABINFO_LEN            53
#define ETAL_MDR_GETMONITORAMFMINFO            0x005
#define ETAL_MDR_GETMONITORAMFMINFO_LEN           62
// offset of the first payload byte starting from HEADER BYTE0
#define ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET     4
#define ETAL_MDR_GETMONITORINFO_SKIP_COUNT         0

#define ETAL_DABMW_GET_DABMON_APP(_d_)                  (tU8) ((_d_)[0])
#define ETAL_DABMW_GET_DABMON_FICBER(_d_)               (tU32)(((((tU32)(_d_)[13]) << 24) & 0xFF000000) | ((((tU32)(_d_)[14]) << 16) & 0x00FF0000) | ((((tU32)(_d_)[15]) << 8) & 0x0000FF00) | ((((tU32)(_d_)[16]) << 0) & 0x000000FF))
#define ETAL_DABMW_GET_DABMON_MSCBER(_d_)               (tU32)(((((tU32)(_d_)[25]) << 24) & 0xFF000000) | ((((tU32)(_d_)[26]) << 16) & 0x00FF0000) | ((((tU32)(_d_)[27]) << 8) & 0x0000FF00) | ((((tU32)(_d_)[28]) << 0) & 0x000000FF))
#define ETAL_DABMW_GET_DABMON_AUDIOSUBCHBER(_d_)        (tU32)(((((tU32)(_d_)[17]) << 24) & 0xFF000000) | ((((tU32)(_d_)[18]) << 16) & 0x00FF0000) | ((((tU32)(_d_)[19]) << 8) & 0x0000FF00) | ((((tU32)(_d_)[20]) << 0) & 0x000000FF))
#define ETAL_DABMW_GET_DABMON_DATASUBCHBER(_d_)         (tU32)(((((tU32)(_d_)[21]) << 24) & 0xFF000000) | ((((tU32)(_d_)[22]) << 16) & 0x00FF0000) | ((((tU32)(_d_)[23]) << 8) & 0x0000FF00) | ((((tU32)(_d_)[24]) << 0) & 0x000000FF))
#define ETAL_DABMW_GET_DABMON_BBLEVEL(_d_)              (tS32)(((((tS32)(_d_)[41]) << 8)  & 0x0000FF00) | ((((tS32)(_d_)[42]) << 0)  & 0x000000FF))

#define ETAL_MDR_GETPADDLS_PAYLOAD_OFFSET     	   4
#define ETAL_DABMW_GET_DABPADDLS_APP(_d_)          (tU8) ((_d_)[0] >> 4)

#define ETAL_MDR_EVENTNOTIFICATION_BITMAP_OFFSET   4

#define ETAL_MDR_SEEK_PAYLOAD_LEN              6

/*
 * special command IDs
 */
#define ETAL_MDR_GETPAD                        0x051
#define ETAL_MDR_GETPADPLUS                    0x053
#define ETAL_MDR_SEEK                          0x062
#define ETAL_MDR_LEARN                         0x064
#define ETAL_MDR_SEAMLESS_SWITCHING            0x066
#define ETAL_MDR_SEAMLESS_ESTIMATION           0x06F
#define ETAL_MDR_TUNE						   0x060

#define ETAL_MDR_FSM_STATUS_RUNNING            0x80

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
/*
 *  Boot mode define
 */
#ifdef CONFIG_HOST_OS_TKERNEL
// File name path : /sda = SDCard 0
#define ETAL_MDR_FLASH_DUMP_FILENAME        "/sda/tuner/firmware/dcop/flashDump.bin"
#define ETAL_MDR_FLASH_BOOTSTRAP_FILENAME   "/sda/tuner/firmware/dcop/flasher.bin"
#define ETAL_MDR_FLASH_PROGRAM_FILENAME     "/sda/tuner/firmware/dcop/dcop_fw.bin"
#else
#define ETAL_MDR_FLASH_DUMP_FILENAME        "flashDump.bin"
#define ETAL_MDR_FLASH_BOOTSTRAP_FILENAME   "flasher.bin"
#define ETAL_MDR_FLASH_PROGRAM_FILENAME     "dcop_fw.bin"
#endif // CONFIG_HOST_OS_TKERNEL

#define ETAL_MDR_BOOT_FILENAME_MAX_SIZE      256
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE

/* NVM configuration */
#define ETAL_DAB_NVM_ACCESS_TUNER_TABLE     	(tU8)0x04
#define ETAL_DAB_NVM_ACCESS_DAB_LANDSCAPE   	(tU8)0x08
#define ETAL_DAB_NVM_SAVE_DAB_LANDSCAPE     	(tU8)0x04
#define ETAL_DAB_NVM_CLEAR_DAB_DATABASE         (tU8)0x04
#define ETAL_DAB_NVM_CLEAR_VOLATILE_DATABASE    (tU8)0x80

#ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
#define ETAL_DABMWOH_PAD_PERIOD 250
#endif

/***********************************
 *
 * Types
 *
 **********************************/
typedef struct
{
	tU8  cmdStatus;
    tU8 LunId;
    tU8 specific0;
    tU8 specific1;
    tU8 specific2;
    tU8 specific3;
	tU32 cmdLen;
	tU8 *cmd;
	tBool cmdHasResponse;
	tU32 rspLen;
	tU8  rsp[ETAL_MAX_RESPONSE_LEN_CONTROL_LUN];
} etalToMDRCmdTy;

typedef enum
{
	cmdStatusInitMDR             = 0x0,
	cmdStatusWaitNotificationMDR = ETAL_MDR_FSM_STATUS_RUNNING | 0x01,
	cmdStatusWaitResponseMDR     = ETAL_MDR_FSM_STATUS_RUNNING | 0x02,
	cmdStatusCompleteMDR         = 0x03,
	cmdStatusErrorMDR            = 0x04
} etalToMDRCmdStatusTy;

typedef enum
{
	/*
	 * values used by the DCOP commands, do not change!
	 */
	etalMOTServiceEPGraw      = 0x01,
	etalMOTServiceSLS         = 0x02,
	etalMOTServiceSLSoverXPAD = 0x04,
} etalMOTserviceTy;

/*
 * Data Protocol payload type for DECODED mode
 * (from TUNER_MODULE dabmw.h)
 *
 * assumed to fit in one byte, checked in ETAL_sanityCheck
 */
typedef enum
{
	DABMW_DATACHANNEL_DECODED_TYPE_UNDEF =              0x00,
	DABMW_DATACHANNEL_DECODED_TYPE_EPG_RAW =            0x01,
	DABMW_DATACHANNEL_DECODED_TYPE_SLS =                0x02,
	DABMW_DATACHANNEL_DECODED_TYPE_SLS_XPAD =           0x03,
	DABMW_DATACHANNEL_DECODED_TYPE_BWS_RAW =            0x04,
	DABMW_DATACHANNEL_DECODED_TYPE_TPEG_RAW =           0x05,
	DABMW_DATACHANNEL_DECODED_TYPE_TPEG_SNI =           0x06,
	DABMW_DATACHANNEL_DECODED_TYPE_SLI =                0x07,
	DABMW_DATACHANNEL_DECODED_TYPE_EPG_BIN =            0x10,
	DABMW_DATACHANNEL_DECODED_TYPE_EPG_SRV =            0x12,
	DABMW_DATACHANNEL_DECODED_TYPE_EPG_PRG =            0x13,
	DABMW_DATACHANNEL_DECODED_TYPE_EPG_LOGO =           0x14,
	DABMW_DATACHANNEL_DECODED_TYPE_JML_OBJ =            0x20,
	DABMW_DATACHANNEL_DECODED_TYPE_DRM_OBJ =            0x40,
	DABMW_DATACHANNEL_DECODED_TYPE_DRM_TM =             0x41,
	DABMW_DATACHANNEL_DECODED_TYPE_DRM_QUALITY =        0x42,
	DABMW_DATACHANNEL_DECODED_TYPE_DRM_FAC =            0x43,
	DABMW_DATACHANNEL_DECODED_TYPE_DRM_SDC =            0x44,
	DABMW_DATACHANNEL_DECODED_TYPE_DRM_PCMAUDIO =       0x45,
	DABMW_DATACHANNEL_DECODED_TYPE_DRM_MDI =            0x46,
	DABMW_DATACHANNEL_DECODED_TYPE_DEBUGDUMP =          0x80,
} DABMWDataServiceType;

/*!
 * \enum	etalCmdMdrTypeTy
 */
typedef enum
{
	Cmd_Mdr_Type_Normal         = 0x00,
	Cmd_Mdr_Type_Special        = 0x01,
	Cmd_Mdr_Type_Direct         = 0x02
} etalCmdMdrTypeTy;

/***********************************
 *
 * Variables
 *
 **********************************/
#if defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE)
extern tChar ETAL_MDR_bootstrap_name_found[ETAL_MDR_BOOT_FILENAME_MAX_SIZE], ETAL_MDR_fw_name_found[ETAL_MDR_BOOT_FILENAME_MAX_SIZE];
extern tBool ETAL_doFlashDump;
extern tBool ETAL_doFlashProgram;
#endif /* CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE */


/***********************************
 *
 * Function protptypes
 *
 **********************************/
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
tSInt 			  ETAL_initCommunication_MDR(EtalDcopBootType isBootMode, tBool vI_manageReset);
tSInt 			  ETAL_Reset_MDR(tVoid);
tSInt             ETAL_deinitCommunication_MDR(tVoid);
tVoid             ETAL_invokeEventCallback_MDR(tU8 *data, tU32 len);

tSInt             ETAL_cmdPing_MDR(tVoid);
tSInt             ETAL_cmdPowerUp_MDR(const DABMW_mwCountryTy country, tU8 nvmFlags);
tSInt             ETAL_cmdPowerDown_MDR(tU8 saveNvm);
tSInt             ETAL_cmdClearDatabase_MDR(tU8 clearDatabase);
tSInt             ETAL_cmdTune_MDR(ETAL_HANDLE hReceiver, tU32 dwFrequency, etalCmdTuneActionTy type, tU16 tuneTimeout);
tSInt             ETAL_cmdGetQuality_MDR(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
tSInt             ETAL_cmdGetAudioQuality_MDR(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
tSInt             ETAL_cmdGetEnsembleList_MDR(EtalEnsembleList *list, tBool *have_data);
tSInt             ETAL_cmdGetEnsembleData_MDR(tU32 eid, tU8 *charset, tChar *label, tU16 *bitmap, tBool *have_data);
tSInt 			  ETAL_cmdGetFIC_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy action);
tSInt             ETAL_cmdGetServiceList_MDR(tU32 eid, tBool bGetAudioServices, tBool bGetDataServices, EtalServiceList *list, tBool *have_data);
tSInt             ETAL_cmdGetSpecificServiceData_MDR(tU32 eid, tU32 sid, EtalServiceInfo *serv_info, EtalServiceComponentList *sclist, tBool *have_data);
tSInt             ETAL_cmdGetSpecificServiceDataExtended_MDR(ETAL_HANDLE hGeneric, tU32 eid, tU32 sid, EtalServiceInfo *serv_info, EtalServiceComponentExtendedList *sclist, tBool *have_data);
tSInt             ETAL_cmdGetCurrentEnsemble_MDR(ETAL_HANDLE hReceiver, tU32 *pUEId);
tSInt             ETAL_cmdServiceSelect_MDR(ETAL_HANDLE hDataPath, ETAL_HANDLE hReceiver, EtalServiceSelectMode mode, EtalServiceSelectSubFunction type, tU32 UEId, tU32 service, tU32 sc, tU32 subch, tBool *noData);
tSInt 			  ETAL_cmdServiceSelect_MDR_internal(ETAL_HANDLE hReceiver, EtalServiceSelectMode mode, EtalServiceSelectSubFunction type, tU32 eid, tU32 sid, tU32 sc, tU32 subch, tBool *no_data, EtalBcastDataType dataType);
tSInt             ETAL_cmdGetPAD_MDR(ETAL_HANDLE hReceiver, etalPADDLSTy *data, tBool *no_data);
tSInt             ETAL_cmdGetPADDLSEvent_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd);
tSInt             ETAL_cmdGetPADDLPlusEvent_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd);
tSInt             ETAL_cmdSetPADDLPlusEvent_MDR(ETAL_HANDLE hReceiver);
#ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
tSInt             ETAL_cmdGetPAD_DABMWoH(ETAL_HANDLE hReceiver, etalPADDLSTy *data, tBool *no_data);
tSInt             ETAL_cmdGetPADevent_DABMWoH(ETAL_HANDLE hReceiver, etalCmdActionTy cmd);
#endif
tSInt             ETAL_cmdGetTPEGRAW_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd);
tSInt             ETAL_cmdGetTPEGSNI_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd);
tSInt             ETAL_cmdGetEPGBIN_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd);
tSInt             ETAL_cmdGetEPGSRV_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU8 ecc, tU16 eid);
tSInt             ETAL_cmdGetEPGPRG_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU8 ecc, tU16 eid, tU32 sid);
tSInt             ETAL_cmdGetEPGLogo_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU8 ecc, tU16 eid, tU32 sid, tU8 logoType);
tSInt             ETAL_cmdGetJMLOBJ_MDR(ETAL_HANDLE hReceiver, etalCmdActionTy cmd, tU16 objectId);
tSInt 			  ETAL_cmdLearn_MDR(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, etalCmdActionTy cmd);
tSInt             ETAL_cmdStartMonitor_MDR(ETAL_HANDLE hMonitor);
tSInt             ETAL_cmdRemoveFiltersForMonitor_MDR(ETAL_HANDLE hMonitor);
tSInt             ETAL_cmdResetMonitorAndFilters_MDR(tVoid);
tSInt 		      ETAL_cmdSeamlessEstimation_MDR(etalSeamlessEstimationConfigTy *seamlessEstimationConfig);
tSInt             ETAL_cmdSeamlessSwitching_MDR(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig);
tSInt             ETAL_cmdSelectAudioOutput_MDR(ETAL_HANDLE hReceiver);
tSInt 			  ETAL_cmdMute_MDR(ETAL_HANDLE hReceiver, tBool muteFlag);
tSInt             ETAL_cmdStartStopMOT(ETAL_HANDLE hReceiver, etalMOTserviceTy service, etalCmdActionTy action);
tSInt             ETAL_cmdSetupEventNotification_MDR(ETAL_HANDLE hReceiver, tU16 serviceBitMap);
tSInt             ETAL_cmdGetFirmwareVersion_MDR(tU32 *year, tU32 *month, tU32 *day, tU32 *major, tU32 *minor, tU32 *internal, tU32 *build);
tSInt             ETAL_cmdGetTime_MDR(tBool getTimeWithLTO, EtalTime *DABTime);
tVoid             ETAL_resetFilter_MDR(etalMonitorTy *pmon);
tSInt             ETAL_checkFilter_MDR(ETAL_HANDLE hMonitor, const EtalBcastQualityMonitorAttr* pMonitorAttr);
tVoid             ETAL_initFilterMask_MDR(etalFilterMaskTy *mask);
etalDCOPFilterTy *ETAL_getFilter_MDR(tU32 index);
tSInt             ETAL_extractPADDLS_MDR(tU8 *data, tU32 len, etalPADDLSTy *pad, tU8 *app);
tSInt 			  ETAL_extractPADDLPLUS_MDR(tU8 *data, tU32 len, etalPADDLPLUSTy *pad, tU8 *app);
#if defined(CONFIG_ETAL_HAVE_ETALTML)
tSInt             ETAL_extractLearn_MDR(tU8 *buf, tU32 len, etalLearnConfigTy *learn);
#endif
#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
tSInt             ETAL_extractSeamlessEstimation_MDR(tU8* buf, tU32 len, EtalSeamlessEstimationStatus *seamlessEstimation);
tSInt             ETAL_extractSeamlessSwitching_MDR(tU8* buf, tU32 len, EtalSeamlessSwitchingStatus *seamlessSwitching);
#endif
tSInt 			  ETAL_extractTune_MDR(tU8* buf, tU32 len, EtalTuneStatus *tune);

tVoid             ETAL_extractDabQualityFromNotification_MDR(EtalDabQualityEntries *d, tU8 *payload);
ETAL_HANDLE       ETAL_receiverGetDatapathFromDABType(ETAL_HANDLE hReceiver, DABMWDataServiceType dabmw_dstype, tVoid **context);

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && !defined (CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD)
tSInt             ETAL_cmdSpecialBootMode_MDR(STECI_cmdModeEnumTy boot_mode);
tSInt             ETAL_cmdSpecialFlashBootstrap_MDR(DCOP_accessModeEnumTy data_mode, tU16 payload_len, tU8 *payload_ptr);
tSInt             ETAL_cmdSpecialFlashCheckFlasher_MDR(DCOP_targetMemEnumTy bootstrap_type);
tSInt             ETAL_cmdSpecialFlashErase_MDR(tVoid);
tSInt             ETAL_cmdSpecialFlashProgramDownload_MDR(DCOP_targetMemEnumTy bootstrap_type, DCOP_accessModeEnumTy data_mode, tU16 payload_len, tU8 *payload_ptr);
tSInt             ETAL_cmdSpecialFlashDump_MDR(DCOP_targetMemEnumTy bootstrap_type, DCOP_accessModeEnumTy data_mode, tU16 payload_len, tU8 *payload_ptr);
#endif // CONFIG_COMM_DRIVER_EMBEDDED && !CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
tBool             ETAL_isBootstrapFileFoundMDR(tChar *mdr_bootstrap_name_found);
tBool             ETAL_isProgramFileFoundMDR(tChar *mdr_fw_name_found);
tSInt             ETAL_renameBootFileMDR(tChar *mdr_fw_name_found);
EtalDcopBootType  ETAL_isDoFlashOrDownloadOrDumpMDR(tVoid);
tSInt             ETAL_doFlashOrDownloadOrDumpMDR(tVoid);
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
#endif

#ifdef CONFIG_COMM_DRIVER_DIRECT
tSInt ETAL_sendDirectCommandTo_MDR(tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen);
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT
tSInt ETAL_sendCommandSpecialTo_MDR(tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen);
tSInt ETAL_sendCommandTo_MDR(tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen);
tSInt ETAL_sendCommandWithRetryTo_MDR(etalCmdMdrTypeTy cmdType, tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen, tU32 retry_delay);

tCString ETAL_cmdStatusToASCII_MDR(etalToMDRCmdStatusTy cmd);
tCString ETAL_MDRStatusToString(tU32 status);

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
tSInt ETAL_cmdGetAudioQuality_MDR(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);
#endif

