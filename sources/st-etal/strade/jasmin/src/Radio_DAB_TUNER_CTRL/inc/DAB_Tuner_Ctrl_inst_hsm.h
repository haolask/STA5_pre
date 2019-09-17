/*==================================================================================================
    start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_Tuner_Ctrl_inst_hsm.h 															   *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.											   *
*  All rights reserved. Reproduction in whole or part is prohibited								   *
*  without the written permission of the copyright owner.										   *
*																								   *
*  Project              : ST_Radio_Middleware																	   *
*  Organization			: Jasmin Infotech Pvt. Ltd.												   *
*  Module				: Radio DAB Tuner Control												   *
*  Description			: This file contains API declarations related to instance HSM.			   *
*																								   *
*																								   *
***************************************************************************************************/

#ifndef DAB_TUNER_CTRL_INST_HSM_
#define DAB_TUNER_CTRL_INST_HSM_

/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Types.h"
#include "DAB_Tuner_Ctrl_Request.h"
#include "hsm_api.h"
#include "sys_task.h"
#include "cfg_types.h"
/*#include "hsm_api.h"

*/

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/
#define DAB_MAX_NUM_OE_SERVICES    0x03
#define DAB_MAX_NUM_FREQ_INFO      0X0A
#define DAB_TUNER_CTRL_MAX_NO_OF_SERVICES_PER_RECEIVER  0x35
#define DAB_TUNER_CTRL_MAX_COMPONENTS_PER_LIST        0x2
#define DAB_SETFREQTABLE_MAX_NUM_FREQ                 100
#define MAX_LINKAGE_SETS								5
#define DAB_FM_LINKING_MAX_THRESHOULD					25   //72
#define DAB_FM_LINKING_MIN_THRESHOULD					10   //36
#define DAB_FM_LINKING_THRESHOULD						20    //60
#define	DAB_QUALITY_THRESHOULD							30 
#define ESD_THRESHOLD									((Ts8)-50)
#define	MAX_FMDAB_SID									5
#define DAB_TUNER_CTRL_ANNO_START			((Tu8) 0x00)	
#define DAB_TUNER_CTRL_ANNO_STOP			((Tu8) 0x01)
#define DAB_TUNER_CTRL_SET_ANNO_ON			0x07FF
#define	CURRENT_SID_LINKING_DATA			0x10
#define	CHECK_HARDLINK						((Tu8)0x00)
#define	CHECK_IMPLICIT						((Tu8)0x01)
#define	CHECK_HARDLINK_AND_IMPLICIT			((Tu8)0x02)

#define	NO_BLENDING							((Tu8)0x00)
#define	BLENDING_IN_PROGRESS				((Tu8)0x01)
#define	STOP_BLENDING_IN_PROGRESS			((Tu8)0x02)
#define	DAB2DAB_ALTERNATE_NOSIGNAL			((Ts8)-128)
#define DAB2DAB_ALTERNATE_THRESHOULD	 	((Ts8)-102)
#define	SERVICE_AVAILABLE					0x00
#define	BER_LINKING_THRESHOULD				((Ts8)-2)
#define	ENABLE_TUNESTATUS_NOTIFICATION		(Tu8)0x03
#define	DISABLE_TUNESTATUS_NOTIFICATION		(Tu8)0x00


#define	FM_HARDLINKS				(Tu8)0x01
#define	DAB_ALTERNATE_FREQUENCY		(Tu8)0x02
#define	DAB_ALTERNATE_ENSEMBLE		(Tu8)0x03
#define	DAB_DAB_HARDLINKS			(Tu8)0x04
#define	QUALITY_PARAMETERS			(Tu8)0x05
#define	BLENDING_INFO				(Tu8)0x06
#define	IMPLICIT_SID				(Tu8)0x07
#define	BLENDED_TO_FM				(Tu8)0x08
#define	BLEND_BACK_TO_DAB			(Tu8)0x09
#define	FM_DAB_LINKING_STATUS		(Tu8)0x0A
#define	FM_DAB_LINKING_DATA			(Tu8)0x0B
#define	DAB_DAB_LINKING				(Tu8)0x0C

//#define DAB_TUNER_CTRL_ENABLE_FMDAB_REGIONAL_CHECK /* FM-DAB followup regional PI check is disabled */
#define DAB_GET_VERSION_CMD_MAX_REPEAT_COUNT		((Tu8)0x02) /* No of times GetVersionNumbers_req to be retriggered */
#define DAB_GET_VERSION_CMD_MAX_REPEAT_COUNT_FOR_SYSMONITOR		((Tu8)0x05) /* No of times GetVersionNumbers_req to be retriggered */
#define DAB_CREATE_RECEIVER_CMD_MAX_REPEAT_COUNT	((Tu8)0x02) /* No of times CreateReceiver_cmd to be retriggered */
#define DAB_SET_TUNE_STATUS_CMD_MAX_REPEAT_COUNT	((Tu8)0x02) /* No of times SetTuneStatusNot_cmd to be retriggered */
#define DAB_AUDIO_STATUS_CMD_MAX_REPEAT_COUNT		((Tu8)0x02) /* No of times SetAudioStatusNotifier_cmd to be retriggered */
#define DAB_SET_SYNCHRONISATION_CMD_MAX_REPEAT_COUNT	((Tu8)0x02) /* No of times SetSynchronisationNotifier_cmd to be retriggered */
#define DAB_SERV_LIST_CHANGED_CMD_MAX_REPEAT_COUNT		((Tu8)0x02) /* No of times SetServListChangedNotifier_cmd to be retriggered */
#define DAB_SERV_PROPERTY_CHANGED_CMD_MAX_REPEAT_COUNT	((Tu8)0x02) /* No of times SetServPropsChangedNotifier_cmd to be retriggered */	
#define DAB_REGISTER_SINK_TO_SERVCOMP_CMD_MAX_REPEAT_COUNT	((Tu8)0x02) /* No of times RegisterSinkToServComp_cmd to be retriggered */	
#define DAB_ABORT_SCAN_CMD_MAX_REPEAT_COUNT					((Tu8)0x02) /* No of times AbortScan2_Cmd to be retriggered */	
#define DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT			((Tu8)0x02) /* No of times GetComponentListReq_Cmd to be retriggered */		
#define DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT	((Tu8)0x02) /* No of times GetEnsembleProperties_req to be retriggered */
#define DAB_GET_PROGRAMLIST_CMD_MAX_REPEAT_COUNT			((Tu8)0x02) /* No of times GetProgrammeServiceList_req to be retriggered */
#define DAB_SELECT_SERVICE_CMD_MAX_REPEAT_COUNT				((Tu8)0x02) /* No of times SelectService_cmd to be retriggered */
#define DAB_ANNOUNCEMENT_SWITCHING_CMD_MAX_REPEAT_COUNT		((Tu8)0x02) /* No of times AnnouncementSwitching_cmd to be retriggered */
#define DAB_PREPARE_FOR_BLENDING_CMD_MAX_REPEAT_COUNT		((Tu8)0x02) /* No of times PrepareForBlending_cmd to be retriggered */
#define DAB_START_TIME_ALLIGNMENT_CMD_MAX_REPEAT_COUNT		((Tu8)0x02) /* No of times StartTimeAlignmentForBlending_cmd to be retriggered */
#define DAB_START_BLENDING_CMD_MAX_REPEAT_COUNT				((Tu8)0x02) /* No of times StartBlending_cmd to be retriggered */
#define DAB_TUNE_CMD_MAX_REPEAT_COUNT						((Tu8)0x02) /* No of times TuneTo_cmd to be retriggered */
#define DAB_SCAN_NOTIFY_CMD_MAX_REPEAT_COUNT				((Tu8)0x02) /* No of times ScanStatus2_not to be retriggered */
#define DAB_DESTROY_RECEIVER_CMD_MAX_REPEAT_COUNT			((Tu8)0x02) /* No of times DestroyReciver_cmd to be retriggered */
#define DAB_RESET_BLENDING_CMD_MAX_REPEAT_COUNT				((Tu8)0x02) /* No of times ResetBlending_cmd to be retriggered */
#define DAB_SELECT_COMPONENT_CMD_MAX_REPEAT_COUNT			((Tu8)0x02) /* No of times SelectComponent_cmd to be retriggered */
#define DAB_DABTUNER_RST_OFF_CMD_MAX_REPEAT_COUNT			((Tu8)0x02) /* No of times System_reset cmd to be retriggered */
#define DAB_SYSTEM_CONTORL_CMD_MAX_REPEAT_COUNT				((Tu8)0x02)	/* No of times SystemControl_cmd to be retriggered */
#define DAB_SELECT_AUDIO_CMD_MAX_REPEAT_COUNT				((Tu8)0x02)	/* No of times SelectAudio_cmd to be retriggered */
#define DAB_GET_ENSEMBLE_CMD_MAX_REPEAT_COUNT				((Tu8)0x02)	/* No of times Getensemble_cmd to be retriggered */
#define DAB_GET_AUDIO_STATUS_MAX_REPEAT_COUNT				((Tu8)0x02)	/* No of times SelectAudio_cmd to be retriggered */
#define DAB_START_TIME_STRETCH_CMD_MAX_REPEAT_COUNT			((Tu8)0x02)	/* No of times StartTimeStretch_cmd to be retriggered */
#define DAB_TUNE_TIMEOUT_MAX_WAIT_COUNT						((Tu8)0x02)	/* Tune timeout maximum wait time */
#define DAB_AUTOSEEK_NOTIFY_CMD_MAX_REPEAT_COUNT			((Tu8)0x02)	/* AUTOSEEK timeout maximum wait time */
/*********************************/
#define SYSTEM_MONITOR_NOTIFICATION_TIME					((Tu16)10000)
#define DAB_CREATE_RECEIVER_CMD_TIMEOUT_TIME				((Tu8)100) /* Timeout time for CreateReceiver_cmd is 100ms */
#define DAB_GET_VERSION_CMD_TIMEOUT_TIME					((Tu8)150) /* Timeout time for GetVersionNumbers_req is 100ms */
#define DAB_SET_SYNCHRONISATION_CMD_TIMEOUT_TIME			((Tu8)100) /* Timeout time for SetSynchronisationNotifier_cmd is 100ms */
#define DAB_REGISTER_SINK_TO_SERVCOMP_CMD_TIMEOUT_TIME		((Tu8)100) /* Timeout time for RegisterSinkToServComp_cmd is 100ms */
#define DAB_SERV_LIST_CHANGED_CMD_TIMEOUT_TIME				((Tu8)100) /* Timeout time for SetServListChangedNotifier_cmd is 100ms */
#define DAB_SERV_PROPERTY_CHANGED_CMD_TIMEOUT_TIME			((Tu8)100) /* Timeout time for SetServPropsChangedNotifier_cmd is 100ms */
#define DAB_AUDIO_STATUS_CMD_TIMEOUT_TIME					((Tu8)100) /* Timeout time for SetAudioStatusNotifier_cmd is 100ms */
#define DAB_SET_TUNE_STATUS_CMD_TIMEOUT_TIME				((Tu8)100) /* Timeout time for SetTuneStatusNot_cmd is 100ms */
#define DAB_SYSTEM_CONTORL_CMD_TIMEOUT_TIME					((Tu8)100) /* Timeout time for SystemControl_cmd is 100ms */
#define DAB_GET_ENSEMBLE_PROPERTIES_CMD_TIMEOUT_TIME		((Tu16)1150) /* Timeout time for GetEnsembleProperties_req is 150ms */
#define DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME					((Tu16)1100) /* Timeout time for SelectService_cmd is 1100ms */
#define DAB_SELECT_COMPONENT_CMD_TIMEOUT_TIME				((Tu16)1000) /* Timeout time for SelectComponent_cmd is 1000ms */
#define DAB_SCAN_NOTIFY_CMD_TIMEOUT_TIME					((Tu16)54400) /* Timeout time for ScanStatus2_not is 54400ms */
#define DAB_ABORT_SCAN_CMD_TIMEOUT_TIME						((Tu8)100) /* Timeout time for AbortScan2_Cmd is 100ms */
#define DAB_GET_PROGRAMLIST_CMD_TIMEOUT_TIME				((Tu8)150) /* Timeout time for GetProgrammeServiceList_req is 150ms */
#define DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME				((Tu8)150) /* Timeout time for GetComponentListReq_Cmd is 150ms */
#define DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME			((Tu16)4054) /* Timeout time for PrepareForBlending_cmd is 3150ms */
#define DAB_START_BLENDING_CMD_TIMEOUT_TIME					((Tu8)100) /* Timeout time for StartBlending_cmd is 100ms */
#define DAB_ANNOUNCEMENT_SWITCHING_CMD_TIMEOUT_TIME			((Tu16)1100) /* Timeout time for AnnouncementSwitching_cmd is 1100ms */
#define DAB_RESET_BLENDING_CMD_TIMEOUT_TIME					((Tu8)100) /* Timeout time for ResetBlending_cmd is 100ms */
#define DAB_START_TIME_ALLIGNMENT_CMD_TIMEOUT_TIME			((Tu8)100) /* Timeout time for StartTimeAlignmentForBlending_cmd is 100ms */
#define DAB_DESTROY_RECEIVER_CMD_TIMEOUT_TIME				((Tu8)100) /* Timeout time for DestroyReciver_cmd is 100ms */
#define DAB_SIGNAL_STABLE_CHECK_TIME 						((Tu16)10000) /* As per requirement 10seconds hold time to validate DAB signal stability */
#define DAB_BG_SCAN_START_TWO_MIN_DELAY_TIME 				(120000u) /* As per requirement background scan is started every 2minutes */
#define DAB_BG_SCAN_START_THREE_SECS_DELAY_TIME 			((Tu16)3000) /* Background scan is started after 3Secs time delay */
#define DAB_BG_SCAN_START_TWO_SECS_DELAY_TIME 				((Tu16)2000) /* Background scan is started after 2Secs time delay */
#define DAB_BG_SCAN_START_HALF_SEC_DELAY_TIME 				((Tu16)500) /* Background scan is started after 500ms time delay */
#define	TIME_TO_CHECK_HARDLINKS_FOR_SID						((Tu8)300)
#define DAB_GET_AUDIO_PROPERTIES_REPLY_TIME					((Tu8)100)
#define DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME			((Tu8)150)
#define DAB_TUNER_CTRL_GET_SYNC_STATE_REPLY_TIMEOUT_TIME	((Tu8)200)
#define DAB_INIT_ALL_LINKING_DELAY_TIME						((Tu16)10000) /* Linking start disable time-10secs after seek operation */
#define DAB_START_TIME_STRETCH_CMD_TIMEOUT_TIME				((Tu8)100) /* Timeout time for DestroyReciver_cmd is 100ms */
#define DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME	((Tu16)5000) /* Timeout time for Autoseek event is 5000ms */
#define DAB_SCAN_AUDIO_PLAY_TIME 							((Tu16)5000)

#define DAB_TUNER_CTRL_REQID                           DAB_TUNER_CTRL_INST_HSM_STARTUP:   \
												   case DAB_TUNER_CTRL_INST_HSM_ABNORMAL_STATE: \
								                   case DAB_TUNER_CTRL_ACTIVATE_REQID:     \
								                   case DAB_TUNER_CTRL_TUNEBYFREQ_REQID:   \
								                   case DAB_TUNER_CTRL_SELSERV_REQID :     \
								                   case DAB_TUNER_CTRL_DESELBAND_REQID:    \
								                   case DAB_TUNER_CTRL_CREATE_RCVER_REQID: \
												   case DAB_TUNER_CTRL_CANCEL_REQID:	\
								                   case DAB_TUNER_CTRL_INST_SHUTDOWN:      \
								                   case DAB_TUNER_CTRL_SCAN_REQID:			\
												   case	DAB_TUNER_CTRL_FM_DAB_PI:			\
												   case	START_BACKGROUND_SCAN:              \
												   case DAB_TUNER_CTRL_ANNOUNCEMENT_START_SWITCHING_REQID:\
												   case DAB_TUNER_CTRL_ANNOUNCEMENT_CANCEL_REQID:	\
												   case DAB_TUNER_CTRL_INTERNAL_ANNO_MSG: \
												   case DAB_TUNER_CTRL_ANNOUNCEMENT_STOP_SWITCHING_REQID:	\
												   case DAB_TUNER_CTRL_CANCEL_ANNOUNCEMENT_REQID:	\
												   case DAB_TUNER_CTRL_ANNO_CONFIG_REQID:\
												   case TRIGGER_GET_AUDIO_STATUS_REQ: \
												   case DAB_TUNER_CTRL_DAB_FM_LINKING_ENABLE_REQID:\
												   case DAB_TUNER_CTRL_ENG_MODE_REQID:\
												   case	DAB_TUNER_CTRL_INST_HSM_ACTIVATE_DEACTIVATE_REQ:\
												   case DAB_TUNER_CTRL_FM_DAB_STOP_LINKING:\
												   case DAB_TUNER_CTRL_DAB_AF_SETTINGS_REQID:\
												   case	DAB_TUNER_CTRL_DAB_FACTORY_RESET_REQID: \
												   case DAB_TUNER_CTRL_SEEK_REQID


#define DAB_TUNER_CTRL_RESID		                    DAB_TUNER_CTRL_TUNE_STATUS_NOTIFICATION: \
												   case DAB_TUNER_CTRL_RECEIVER_CONFIG_DONE_RESID: \
												   case DAB_TUNER_CTRL_DATAPATH_CONFIG_DONE_RESID: \
												   case DAB_TUNER_CTRL_AUDIO_SOURCE_SELECT_DONE_RESID: \
												   case DAB_TUNER_CTRL_ENABLE_DATASERVICE_DONE_RESID: \
												   case DAB_TUNER_CTRL_DISABLE_DATA_SERVICE_DONE_RESID: \
							                       case DAB_TUNER_GET_VERSION_NUMBER_REPLY:      \
							                       case DAB_TUNER_CTRL_CREATE_RECEIVER_REPLY:    \
							                       case DAB_TUNER_CTRL_CREATE_CONTEXT_REPLY:     \
							                       case DAB_TUNER_CTRL_SET_TUNE_STATUS_NOTIFICATION_REPLY:\
							                       case DAB_TUNER_CTRL_SCAN_REPLY:               \
							                       case DAB_TUNER_CTRL_SCAN_NOTIFICATION:        \
							                       case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY: \
							                       case DAB_TUNER_CTRL_TUNE_REPLY:               \
							                       case DAB_TUNER_CTRL_SELECT_SERVICE_REPLY:     \
							                       case DAB_TUNER_CTRL_SELECT_COMPONENT_REPLY:	\
												   case GET_PROGRAM_SERVLIST_REPLY:            \
												   case AUDIO_STATUS_REPLY:						\
												   case AUDIO_STATUS_NOTIFICATION:				\
												   case	BER_STATUS_REPLY:						\
												   case	BER_STATUS_NOTIFICATION:				\
												   case RSSI_REPLY:								\
												   case RSSI_NOTIFICATION:						\
												   case	SNR_REPLY:								\
												   case	SNR_NOTIFICATION:						\
												   case DAB_TUNER_CTRL_SCAN_FAILURE:			\
												   case DAB_TUNER_CTRL_TUNE_FAILURE:			\
												   case DAB_TUNER_CTRL_SEARCHNEXT_REPLY:        \
												   case DESTROY_RECEIVER_REPLY:					\
												   case FIG_DATA_NOTIFICATION:					\
												   case FIG_REPLY:								\
												   case DAB_TUNER_CTRL_AUDIO_STATUS_REPLY:		\
												   case DAB_TUNER_CTRL_AUDIO_STATUS_NOTIFICATION:\
												   case STATUS_TIME_OUT:						\
												   case	DAB_TUNER_CTRL_SYNCHRONISATION_REPLY:	\
												   case	DAB_TUNER_CTRL_SYNCHRONISATION_NOTIFICATION:\
												   case DAB_TUNER_CTRL_DECODING_STATUS:			\
												   case PREPARE_FOR_BLENDING_REPLY:				\
												   case	PREPARE_FOR_BLENDING_NOTIFICATION:		\
												   case START_TIME_ALLIGNMENT_REPLY:			\
												   case START_TIME_ALLIGNMENT_NOTIFICATION:		\
												   case START_BLENDING_REPLY:					\
												   case BEST_PI_RECEIVED_NOTIFICATION:			\
												   case DAB_TUNER_CTRL_DAB_FM_LINKING_STATUS_NOTIFYID:\
												   case	DAB_TUNER_CTRL_PI_QUALITY_NOTIFYID:		\
												   case STATUS_CHECKING_MSG :\
												   case DAB_TUNER_CTRL_FREQUENCY_CHANGE_NOTIFICATION:\
												   case GET_ENSEMBLE_PROPERTIES_REPLY :\
												   case AUDIO_TIMER_NOTIFICATION:				\
												   case GET_PROGRAMLIST_TIMEOUT:				\
												   case	GET_SEARCHNEXT_TIMEOUT:					\
												   case	SET_DRIFTTRACKING_REPLY :				\
												   case REGISTER_SINK_TO_SERVCOMP_REPLY:	\
												   case DLS_DATA_NOTIFICATION:				\
												   case SLS_DATA_NOTIFICATION:              \
												   case	TUNE_TIME_OUT:						\
												   case	SELECT_COMPONENT_TIMEOUT:			\
												   case SELECT_SERVICE_TIMEOUT	:			\
												   case SCAN_NOTIFY_TIMEOUT :               \
												   case SCAN2SEEK_NOTIFY_TIMEOUT :          \
												   case DAB_TUNER_CTRL_START_BACKGRND_SCAN :   \
												   case SERV_LIST_CHANGED_NOTIFIER_REPLY: \
												   case SERV_PROPERTY_CHANGED_NOTIFIER_REPLY: \
												   case PROGRAMME_SERV_LIST_CHANGED_NOTIFICATION: \
												   case SERVICE_PROPERTY_CHANGED_NOTIFICATION: \
												   case DAB_TUNER_CTRL_ANNOUNCEMENT_SWITCHING_REPLY:\
												   case	DAB_TUNER_CTRL_TUNE_SIGNAL_PRESENT:	\
												   case DAB_TUNER_CTRL_TUNE_MCI_PRESENT: \
												   case	GET_TUNESTATUS_TIMEOUT:\
												   case ABORT_SCAN2_REPLY: \
												   case SYSTEM_CONTORL_REPLY:\
												   case SYSTEM_CONTORL_NOTIFICATION: \
												   case RESET_BLENDING_REPLY: \
												   case DAB_TUNER_CTRL_AMFMTUNER_STATUS_NOTIFYID: \
												   case DAB_TUNER_CTRL_DAB_SAMECHANNELANNO_NOTIFYID:  \
												   case CREATE_RECEIVER_REPLY_TIMEOUT:	\
												   case DESTROY_RECEIVER_REPLY_TIMEOUT:	\
												   case GET_VERSION_NUMBER_REPLY_TIMEOUT:	\
												   case REGISTER_SINK_TO_SERVCOMP_REPLY_TIMEOUT:	\
												   case SERV_LIST_CHANGED_NOTIFIER_REPLY_TIMEOUT:	\
												   case SERV_PROPERTY_CHANGED_NOTIFIER_REPLY_TIMEOUT:	\
												   case SET_TUNE_STATUS_NOTIFICATION_REPLY_TIMEOUT:	\
												   case SET_AUDIO_STATUS_REPLY_TIMEOUT: \
												   case SET_SYNCHRONISATION_REPLY_TIMEOUT: \
												   case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:	\
												   case ABORT_SCAN_TIMEOUT:	\
												   case GET_COMPONENTLIST_TIMEOUT:	\
												   case PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT:	\
												   case START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT:	\
												   case START_BLENDING_REPLY_TIMEOUT:	\
												   case ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT:	\
												   case RESET_BLENDING_REPLY_TIMEOUT:	\
												   case SYSTEM_CONTORL_REPLY_TIMEOUT:	\
												   case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY:\
												   case DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY: \
												   case START_DAB_DAB_LINKING:	\
												   case DAB_SIGNAL_STABLE_CHECK:\
												   case	CHECK_HARDLINKS_FOR_TUNED_SID:\
												   case SYSTEM_MONITOR_NOTIFICATION_TIMEOUT : \
												   case DAB_TUNER_CTRL_DRIFT_TRACKING_NOTIFICATION:\
													case DAB_TUNER_CTRL_BACKGROUND_SCAN_START_NOTIFYID : \
													case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT : \
													case DAB_TUNER_CTRL_GET_SYNC_STATE_REPLY : \
													case DAB_TUNER_CTRL_GET_SYNC_STATE_REPLY_TIME_OUT : \
													case DAB_TUNER_CTRL_AUDIO_ERROR_CONCEALMENT_REPLY: \
													case DAB_TUNER_CTRL_SET_AUDIO_ERROR_CONCEALMENT_REPLY: \
													case DAB_TUNER_CTRL_STRATERGY_STATUS_NOTIFYID:\
													case DAB_TUNER_CTRL_TUNE_TO_SERVICE:\
													case DAB_INIT_ALL_LINKING_PROCESS:\
													case DAB_TUNER_CTRL_START_TIMESTRETCH_REPLY:\
													case DAB_TUNER_CTRL_START_TIMESTRETCH_NOTIFICATION:\
												   	case DAB_TUNER_CTRL_INIT_FMDAB_LINKING_NOTIFYID:\
													case START_TIME_STRETCH_REPLY_TIMEOUT:\
													case DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT: \
													case DAB_TUNER_CTRL_COMP_LIST_SORT_RESID: \
													case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY: \
													case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT: \
													case DAB_TUNER_CTRL_SELECT_AUDIO_REPLY: \
													case DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT:\
													case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION:\
													case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT:\
													case DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT:\
													case DAB_TUNER_CTRL_STOP_SCAN_AUDIO:\
													case AUTOSEEK_CMD_REPLY_TIMEOUT:\
													case ABORT_AUTOSEEK_TIMEOUT:\
													case DAB_QUALITY_NOTIFICATION_MSGID:\
													case DAB_TUNER_CTRL_DESTROY_QUALITYMONITOR_DONE_RESID:\
													case DAB_TUNER_CTRL_ENABLE_QUALITY_MONITOR_DONE_RESID:\
													case DAB_TUNER_CTRL_DESTROY_DATAPATH_DONE_RESID

#define  DAB_TUNER_CTRL_INST_HSM_STARTUP                        (0x17)
#define  DAB_TUNER_CTRL_INST_SHUTDOWN                           (0x22)				
#define  DAB_TUNER_CTRL_ERROR                                   (0x24)
#define  REPLY_STATUS_OK                                        (Tu8)0x00
#define  LINKING_DAB_SIDS									    0
#define  LINKING_FM_PIS										    1
#define	 UN_RELIABLE						                    0X00
#define	 PROBABLE							                    0X01
#define  RELIABLE							                    0X02
#define	 ALLIGNMENT_LOST						                0x03
#define	 PREPARE_BLENDING_DELTA_DELAY		                    500
#define  DAB_TUNER_CTRL_MAX_NUM_OE_SERVICES          			20u
#define  FIC_BER_THRESHOLD										50000
/*--------------------------------------------------------------------------------------------------
    type definitions
--------------------------------------------------------------------------------------------------*/


typedef struct
{
  Ts_hsm								  			st_hsm;                /* the base HSM object (handles state transitions) */
  Tu8 							 					str_state[100];
  const Tu8*							  			u8_curr_state_str;     /* Pointer to store the current state handler name */
  
  /*Structures*/
  Ts_Announcement_Support 							Announcement_data[MAX_ENSEMBLE_SERVICES];
  Ts_Sys_Msg 										Blending_Process;
  Ts_DabTunerMsg_R_CreateReceiver		  			CreateReceiverReply;                                                          
  Ts_DabTunerMsg_R_ScanStatus_Not		  			DAB_Tuner_Ctrl_ScanNotification;
  Ts_DabTunerMsg_GetComponentList_Reply   			DAB_Tuner_Ctrl_GetComponent_Reply;
  Ts_DAB_Tuner_Ctrl_Set_FIG_Reply					FIG_Reply;
  Ts_dab_freqInfo             						freqInfo[DAB_MAX_NUM_FREQ_INFO];
  Ts_Lsn_Info 										hardlinkinfo[MAX_LINKAGE_SETS];
  Ts_PI_Freq										Hardlink_FM_PI;
  Ts_Linking_info									linkagedata;
  Ts_dab_oeServices           						oeServices[DAB_TUNER_CTRL_MAX_NUM_OE_SERVICES];
  Ts_Tuner_Ctrl_CurrentEnsembleInfo       			requestedinfo;
  Ts_DabTuner_ScanInput 							sScanInput;
  Ts_DabTunerMsg_R_StartScan_Cmd		  			ScanReply;
  Ts_DabTunerMsg_R_ScanStatus_Not		  			ScanStatusNotify;

  Ts_AbortScan2Reply								st_AbortScan2Reply ;
  Ts_AFList											st_AFList;
  Ts_Anno_Swtch_Info								st_Anno_Swtch_Info;
  Ts_Anno_Swtch_Notify_Info							st_Anno_Swtch_Notify_Info[12];
  Ts_DabTunerMsg_R_AnnouncementSwitching   			st_AnnouncementSwitching_reply;
  Ts_AudioStatus2_not								st_Audiostatus;
  Ts_DabTunerMsg_R_AudioStatus_notify				st_Audio_Status_notification;
  Ts_DabTunerMsg_R_SetAudioStatusReply				st_Audio_Status_reply;
  Ts_DabTunerMsg_R_PeriodicalBERQual_Notify			st_BER_notification;
  Ts_DabTunerMsg_R_PeriodicalBERQual_reply			st_BER_reply;
  Ts_Blending_info									st_Blending_info;
  Ts_Tuner_Ctrl_CurrentEnsembleInfo		  			st_currentEnsembleData;
  Ts_Announcement_Support  							st_CurrentSIDAnno_Info;
  Ts_CurrentSidLinkingInfo 							st_CurrentSidLinkingInfo ;
  Ts_Service_Info									st_CurrentSIDService_Info;
  Ts_Tuner_Ctrl_CurrentEnsembleInfo		  			st_CurrentTunedAnnoInfo;	/* Structure for storing tuned announcement station information */
  Ts_DAB_Tuner_Anno_Swtch_Info 						st_DAB_Tuner_Anno_Swtch_Info;
  Ts_DabTunerMsg_R_AudioErrorConcealment2_repl		st_DabTunerMsg_R_AudioErrorConcealment2_repl;
  Ts_DabTunerMsg_GetAudioErrorConcealment2SettingsReply st_DabTunerMsg_GetAudioErrorConcealment2SettingsReply;  
  Ts_DabTunerMsg_R_DestroyReceiver_Cmd	 			st_destroyreceiver;
  Ts_dab_DLS_data									st_Dls_data;
  Ts_Drift_Tracking_Notification					st_Drift_Tracking_Notification; 
  Ts_dab_tuner_ctrl_DLS_data                        st_Dynamic_Label_Data; 
  Ts_Tuner_Ctrl_BasicEnsembleInfo		  			st_ensembleProperties; /* Ensemble properties structure */
  Ts_Ensemble_Info									st_Ensemble_Info;
  Ts_FIG_Data_available								st_FIG_data_available;
  Ts_dab_tuner_ctrl_fmdab_linkinfo					st_fmdab_linkinfo[MAX_FMDAB_SID];
  Ts_GetAudioProperties_repl						st_GetAudioProperties_repl ;
  Ts_GetAudioStatus_repl							st_GetAudioStatus_repl ;
  Ts_DabTunerMsg_GetAudioStatus2_reply				Ts_GetAudioStatus2_reply;
  Ts_DabTunerMsg_GetCurrEnsembleProgListReply       st_GetCurrEnsembleProgListReply;
  Ts_DabTunerGetEnsembleProperties_reply            st_GetEnsembleProperties_reply;
  Ts_GetSynchronisationState_repl					st_GetSynchronisationState_repl ;
  Ts_DabTunerMsg_R_GetVersion            			st_GetVersionReply; 
  Ts_LinkingStatus									st_Linkingstatus;
  Ts_Tuner_Ctrl_CurrentEnsembleInfo					st_lsmdata;
  Ts_PI_Data										st_PI_data_available;
  Ts_PrepareForBlending_Notify						st_PrepareForBlending_Notify;
  Ts_PrepareForBlending_Reply						st_PrepareForBlending_Reply; 
  Ts_DabTunerMsg_GetCurrEnsembleProgListReply		st_ProgrammeServListChanged_not; /*	ProgrammeServListChanged_not notification information */
  Ts_DabTunerMsg_RegisterSinkToServComp_reply       st_RegisterSinkToServComp_reply;
  Ts_Dab_app_Tuner_Requests                         st_requestId;
  Ts_ResetBlending_Reply							st_ResetBlending_Reply ;
  Ts_DabTunerMsg_R_RSSI_notify						st_RSSI_notification;
  Ts_DabTunerMsg_R_RSSINotifierSettings_reply		st_RSSI_reply;
  Ts_SearchNext_Reply                               st_searchNextReply;
  Ts_Tuner_Ctrl_CurrentEnsembleInfo                 st_seekInfo;
  Ts_Select_ComponentReply				 			st_selectcompreply;
  Ts_DabTunerMsg_SelectServiceReply       			st_selectServiceReply;
  Ts_Set_Audio_Status_Notifier_Reply				st_Set_Audio_Status_reply; 
  Ts_DabTunerMsg_SetServListChangedNotifier_repl	st_SetServListChangedNotifier_reply; /* SetServListChangedNotifier_repl reply status */
  Ts_Service_Info									st_Service_Temp_Info;
  Ts_Service_Info									st_Service_Info[MAX_ENSEMBLE_SERVICES];
  Ts_DabTunerMsg_SetServPropsChangedNotifier_repl 	st_SetServPropsChangedNotifier_reply; /* SetServPropsChangedNotifier_repl reply status */
  Ts_DabTunerMsg_ServPropsChanged_not				st_ServPropsChanged_not;  /* ServPropsChanged_not notification infromation */
  Ts_Set_DriftTracking_Reply						st_Set_DriftTracking_Reply;
  Ts_DabTunerMsg_R_SNRNotifier						st_SNR_notification;
  Ts_DabTunerMsg_R_SetSNRNotifier_reply				st_SNR_reply;
  Ts_StartBlending_Reply							st_Start_Blending_Reply; 
  Ts_TimeAlignmentForBlending_Notify				st_Start_Timeallignment_notify;
  Ts_StartTimeAlignmentForBlending_repl				st_StartTimeAlignmentForBlending_repl;
  Ts_StartTimeStretch_not							st_StartTimeStretch_not;
  Ts_StartTimeStretch_repl							st_StartTimeStretch_repl;
  Ts_DabTunerMsg_R_SynchNotification                st_synchNotification;
  Ts_DabTunerMsg_R_SynchReply                       st_synchReply;
  Ts_SystemControl_repl								st_SystemControl_repl ;
  Ts_SystemMonitoring_not							st_SystemMonitoring_not ;
  Ts_Announcement_Support							st_Temp_Anno_Info;		/*temporary structure for one time saving and after will copy the data into main data base*/
  Ts_DabTunerMsg_R_TuneTo                 			st_tuneReply;
  Ts_DabTunerMsg_R_SetTuneStatusNot       			st_tuneStatus;
  Ts_DabTunerMsg_R_SetTuneStatus		 			st_tunestatusnot;
  Ts_Tuner_Status_Notification						st_Tuner_Status_Notification;
  Ts_DabTunerMsg_R_SetTuneStatusNot                 st_tuneStatus_SearchNext;
  EtalSeekStatus									st_SeekStatus;
  EtalBcastQualityContainer							st_EtalBcastQualityContainer;
  EtalTuneStatus									st_Tunestatus;
  
  /*Enum*/
  
  Tbool 											b_FirstOccurFlag;
  Te_RADIO_ReplyStatus								e_ReplyStatus;
  
  Tbool												b_Anno_Onstate;
  Tbool												b_AnnoSigLow_sent;
  Te_DAB_Tuner_Ctrl_announcement_type				e_announcement_type;

  Te_Tuner_Ctrl_BestPI_Type							e_BestPI_Type;
  Tbool												b_ActivateRequest;
  Ts_OE_Anno_Support								st_OE_Anno_Support_Info;
  Ts_OE_Anno_Switching								st_OE_Anno_Switching_Info;

  Tbool												b_CheckAudioNotification;
  Tbool		  										b_ReConfigurationFlag ;
  Tbool												b_ScanInProgress;
  Te_DAB_Tuner_Ctrl_CancelType 						e_CancelType;
  Tbool									  			b_componentSelected;   /* Flag for selectServiceHandler that indicates if a component has been selected */
  Te_RADIO_Comp_Status								e_ComponentStatus;
  Te_DAB_Tuner_Ctrl_Eng_Mode_Request				e_DAB_Tuner_Ctrl_Eng_Mode_Request;
  Te_DAB_Tuner_Ctrl_LearnMemAFStatus				e_DAB_Tuner_Ctrl_LearnMemAFStatus;
  Te_DAB_Tuner_Ctrl_DAB_AF_Settings					e_DAB_Tuner_Ctrl_DAB_AF_Settings;
  Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus	e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus;
  Te_DAB_Tuner_Ctrl_State							e_DAB_Tuner_Ctrl_State;
  Tbool												b_DAB2DAB_Alternates_Sorted;
  Te_DAB2DAB_Linking_Status							e_DAB2DAB_Linking_Status;
  Tbool									  			b_Direction;
  Tbool												b_FIG02_Disabled;
  Te_dab_tuner_ctrl_fmdab_linkstatus				e_fmdab_linkstatus; 
  Te_FmtoDAB_Reqstatus 								e_FmtoDAB_Reqstatus;
  Te_RADIO_Comp_Status								e_AMFMTUNERStatus;
  Te_RADIO_DABFM_LinkingStatus 						e_LinkingStatus;
  Te_DAB_Tuner_DABFMLinking_Switch					e_Linking_Switch_Status;
  Tbool												b_Notifications_Disabled;
  Te_Tuner_Ctrl_ReConfigType						e_ReConfigType ;
  Te_DAB_Tuner_Ctrl_RequestCmd						e_RequestCmd ;
  Te_RADIO_DirectionType							e_SeekDirection;
  Te_Tuner_Ctrl_ScanStatus							e_Scannstatus;
  Tbool												b_SamePIFoundNotifySent;
  Te_DAB_Tuner_Ctrl_DABTUNERRestartCmd				e_DABTUNERRestartCmd ;
  Te_RADIO_DirectionType							e_seekDirection;
  Te_Tuner_Ctrl_SelectService_Request_status		e_SelectService_Status;
  Tbool												b_StartAllLinking;
  Te_SystemMonitoringNotReceived					e_SystemMonitoringNotReceived ;
  Te_DAB_Tuner_Ctrl_announcement_type				e_tuned_announcement_type;
  Te_Tuner_Ctrl_AnnoIndication						e_Tuner_Ctrl_AnnoIndication;
  Tbool												b_TunedtoNewEnsemble;

  /*Variables*/
  Tu32 												Avg_Blending_delay;
  Ts32												BlendingDelay_differene;
  Ts32												Blending_Delay[5];	
  Tu32												Blending_delay_To_Chk_Allignment;
  Tu32 												Frequency;
  Tu32 												Index;
  Tu32 												nvm_read;
  Tu32 												nvm_write;
  Tu32 												Sid;
  
  Tu32												u32_SeekStartFrequency ;
  Tu32												u32_AutoSeekStartFrequency;
  Tbool												b_AutoSeekStart; /* Flag for Updatestopfreq in autoseek_start cmd*/
  
  Tu16 												CompIndex;
  Tu16 												currentserviceindx;
  Tu16 												Eid;
  Tu16												Level_data[5];
  Tu16 												serviceindx;
  
  Tu16												u16_AnnoConfig;
  Tu16												u16_FM_DAB_SCID;
  Tu16												u16_FM_DAB_SID;
  Tu16												u16_LevelData;

  Tu8 												ALternateEid;
  Tu8 												ALternateSid;
  Tu8 												Best_PI_notification;
  Tu8												CIFCountDiff;
  Tu8												Confidence_level[5];
  Tbool												b_DAB_DAB_Linking_Timer_start;
  Tbool												b_Eid_Found;
  Tu8 												EnsembleFreq;
  Tbool 											b_Frequency_Found;
  Tbool												b_Implicit_Linking;
  Tu8												Level_data_count;
  Tbool 											b_Linking;
  Tbool												b_Linking_Handler;
  Tu8 												Max_Quality;
  Tu8 												Min_Quality;
  Tbool												b_Prepare_Blending;
  Tbool												b_SameSidFound;
  Tbool 											b_SameSidFreq_Found;
  Tbool												b_scanstarted;
  Tbool												b_Sid_Found;
  Tu8 												startup;
  Tu8												Start_Blending;
  Tu8												Timer_On;
  Tbool												b_Timer_start;
    
  Tu8 												u8_Anno_index ;
  Tu8												u8_BlendingProcess;
  Tu8												u8_cmd_recall_count ; /* Variable to store cmd retrigger count */
  Tu8												u8_Cmd_ReTrigger ;
  Tu8 												u8_CurrEnsembleSerIndx;
  Tbool												b_DAB_Alternatecheck_Ongoing;
  Tu8												u8_DiffChannelClusterid;	
  Tu8												u8_DiffChannelSubChId;
  Tu8												u8_ENGMODEStatus;
  Tu8 												u8_FigMinLength ; /* Variable to store minimum bytes of all FIG's */
  Tbool												b_InitialStartUpFlag;
  Tu8												u8_NoOfSamePIs ;
  Tu8												u8_NoOfServicesIn_ServiceInfo ;
  Tu8 												u8_QualityMax;
  Tu8 												u8_QualityMin;
  Tu8												u8_SameChannelClusterid;
  Tu8         										u8_SameChannelSubChId; 
  Tu8												u8_DABTuner_Abnormal_RSSI_Check_Count;
  Tu8												u8_SeekFrequencyIndex ;
  Tu8 												u8_SettingStatus;
  Tu8 												u8_Skip_ReConfiguration ;
  Tu8 												u8_StartType;
  Tu8												u8_Subchannelid; 
  
  
} Ts_dab_tuner_ctrl_inst_hsm;

/* struct for TimerId*/

typedef struct
{
	
	Tu32	u32_StartBlending_Timer;
	Tu32	u32_ActiveStart_Timer;
	Tu32	u32_ScanNotify_ScanStart_Timer;
	Tu32	u32_AbortScan_ScanStart_Timer;
	Tu32	u32_ScanStateGetComponentlist_Timer;
	Tu32	u32_NosignalListen_Timer;
	Tu32	u32_DecodingStatus_Timer;
	Tu32	u32_DAB_DABLinking_Timer;
	Tu32	u32_Listen_Timer;
	Tu32	u32_AnnouncementSwitching_Timer;
	Tu32	u32_Linking_Timer;
	Tu32	u32_DAB_FM_blending_Timer;
	Tu32	u32_TuneTimeOut_DABDABBlending_Timer;
	Tu32	u32_DABDABBlending_Timer;
	Tu32	u32_TuneEnsemble_Timer;
	Tu32	u32_ProgramList_Timer;
	Tu32	u32_CompList_Timer;
	Tu32	u32_SelectService_Timer;
	Tu32	u32_SelectComponentTimeout_SelComp_Timer;
	Tu32	u32_Selcomp_Timer;
	Tu32	u32_TuneTimeOut_SearchNext_Timer;
	Tu32	u32_SearchNext_Timer;
	Tu32	u32_StartBackGrndScan_Timer;
	Tu32	u32_ResetBlending_BgScan_Timer;
	Tu32	u32_AbortScan_BackgroundScan_Timer;
	Tu32	u32_BackgrndScanState_Timer;	
	Tu32	u32_TuneTimeOut_FMDABLinking_Timer;
	Tu32	u32_SelService_FMDABLinking_Timer;
	Tu32	u32_FMDABLinking_Timer;
	Tu32	u32_DestroyReceiver_Timer;
	Tu32	u32_ESD_AllBandTune_Timer;
	Tu32	u32_AutoseekNotify_Timer;
	Tu32	u32_AutoseekStopNotify_Timer;
} Ts_dab_tuner_ctrl_inst_timer;
/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM top handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control instance HSM is initialised.
*   \details                  When DAB Tuner Control is initialised, it enters the instance top 
                              state that is, instance top handler function and processes the valid 
							  messages received in the handler. 
*   \post-condition			  DAB Tuner Control is in top state.
*   \ErrorHandling    		  When the message received by the instance top handler cannot be 
                              processed, it throws an error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_TopHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM inactive handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in instance top state handler.
*   \details                  When DAB Tuner Control is in instance top state, the top handler 
                              transits to instance inactive handler via HSM_MSGID_START message and 
							  processes the valid messages received in the handler. 
*   \post-condition			  DAB Tuner Control is in instance inactive state.
*   \ErrorHandling    		  When the message received by the instance inactive handler cannot be 
                              processed, the parent handles the message, when the message is not 
							  handled by the parent state then HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_InactiveHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM active handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in instance inactive state handler.
*   \details                  When DAB Tuner Control is in instance inactive state, inactive handler 
                              transits to instance active handler via start up message received from 
							  upper layer. 
*   \post-condition			  DAB Tuner Control is in instance active state.
*   \ErrorHandling    		  When the message received by the active handler cannot be processed, 
                              the parent handles the message, when the message is not handled by the 
                              parent state then HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM active start handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in instance active state handler.
*   \details                  When DAB Tuner Control is in active state, active handler transits 
                              to active start handler via start up message received from DAB Tuner 
							  Control main HSM. 
*   \post-condition			  DAB Tuner Control is in instance active start state.
*   \ErrorHandling    		  When the message received by the instance active start state handler 
                              cannot be processed, the parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveStartHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM active start up scan handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in instance active start state handler.
*   \details                  When DAB Tuner Control is in active start state, active start handler 
                              transits to active start handler via start scan message received from 
							  upper layer. 
*   \post-condition			  DAB Tuner Control is in instance active start state.
*   \ErrorHandling    		  When the message received by the instance active start state handler 
                              cannot be processed, the parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ScanStateHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM active idle handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in instance active start state handler.
*   \details                  When DAB Tuner Control is in instance active start state, active start 
                              handler transits to instance idle state via activate message received 
							  from upper layer. 
*   \post-condition			  DAB Tuner Control is in idle state.
*   \ErrorHandling    		  When the message received by instance idle state handler cannot be 
                              processed, the parent handles the message, when the message is not 
							  handled by the parent state then HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM active Listen handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in instance active busy state.
*   \details                  When DAB Tuner Control tunes to a particular frequency, component and 
                              receives audio data from SOC,then DAB Tuner Control HSM transits to
							  listen state.
*   \post-condition			  DAB Tuner Control is in listen state.
*   \ErrorHandling    		  When the message received by instance listen state handler cannot be 
                              processed, then parent handles the message, when the message is not 
							  handled by the parent state then HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ListenHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM active busy handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in idle state handler.
*   \details                  When DAB Tuner Control receives any request from upper layer, DAB Tuner 
                              Control transits to active busy handler and the requests are processed
							  in this state.
*   \post-condition			  DAB Tuner Control is in instance active busy state.
*   \ErrorHandling    		  When the message received by instance active busy state handler cannot 
                              be processed, then parent handles the message, when the message is not 
							  handled by the parent state then HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM select service and component handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in idle state handler.
*   \details                  When DAB Tuner Control receives select service request from upper layer, 
                              DAB Tuner Control transits to select service and component state and 
							  the request is processed in its sub states.
*   \post-condition			  DAB Tuner Control is in instance HSM select service and component state.
*   \ErrorHandling    		  When the message received by instance HSM select service and component 
                              state handler cannot be processed, then parent handles the message, 
							  when the message is not handled by the parent state then HSM	throws 
							  error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM tune ensemble handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in idle state handler.
*   \details                  When DAB Tuner Control receives select service request from upper layer, 
                              DAB Tuner Control transits to select service and component state and 
							  then to tune ensemble state.In this state DAB Tuner Control performs 
							  tuning operation to the requested ensemble.
*   \post-condition			  DAB Tuner Control is in instance HSM tune ensemble handler state.
*   \ErrorHandling    		  When the message received by instance HSM tune ensemble state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);



/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM select service handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in idle state handler or tune ensemble state.
*   \details                  When DAB Tuner Control receives select service request from upper layer, 
                              DAB Tuner Control transits to select service and component state, if
							  the requested service belongs to already tuned ensemble then DAB Tuner
							  Control transits to select service state else it transits to tune
							  ensemble state and then to select service.In this state DAB Tuner Control 
							  performs service selection operation.
*   \post-condition			  DAB Tuner Control is in instance HSM select service handler state.
*   \ErrorHandling    		  When the message received by instance HSM select service state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelServHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);



/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM select component handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in idle state handler or tune ensemble state
                              or select service handler.
*   \details                  When DAB Tuner Control receives select service request from upper layer, 
                              DAB Tuner Control transits to select service and component state, if
							  the requested service component belongs to already tuned ensemble and already 
							  selected service then DAB Tuner Control transits to select component state, 
							  else it transits either to select service state (When ensemble is same and 
							  service is different) or it transits tune ensemble state and then tune
							  to select service state(When ensemble as well as service are different).
							  In this state DAB Tuner Control performs component selection operation.
*   \post-condition			  DAB Tuner Control is in instance HSM select component handler state.
*   \ErrorHandling    		  When the message received by instance HSM select component state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelCompHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM sleep handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in idle state handler.
*   \details                  When DAB Tuner Control receives deactivate request from upper layer, 
                              DAB Tuner Control transits to state state.
*   \post-condition			  DAB Tuner Control is in instance HSM sleep handler state.
*   \ErrorHandling    		  When the message received by instance HSM sleep state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SleepHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM error handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in active state handler.
*   \details                  When DAB Tuner Control receives error message, then DAB Tuner Control 
                              transits to error state state.
*   \post-condition			  DAB Tuner Control is in instance HSM error handler state.
*   \ErrorHandling    		  When the message received by instance HSM error state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ErrHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM BackgroundScan handler.
*   \param[in]				  msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in active idle handler.
*   \details                  When DAB Tuner Control receives background scan message from main HSM, 
                              then DAB Tuner Control transits to BackgroundScan handler.In this state DAB Tuner
							  Control do background scan operation.
*   \post-condition			  DAB Tuner Control is in instance HSM BackgroundScan handler state.
*   \ErrorHandling    		  When the message received by instance HSM BackgroundScan state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_BackgroundScanHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM Background Ensemble Scan state handler.
*   \param[in]				  msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in BackgrndScanStateGetcomponentList handler.
*   \details                  When DAB Tuner Control receives Background Ensemble Scan state get component list from main HSM, 
                              then DAB Tuner Control transits to Background Ensemble Scan handler.In this state DAB Tuner
							  Control do background scan operation.
*   \post-condition			  DAB Tuner Control is in instance HSM Background Ensemble Scan handler state.
*   \ErrorHandling    		  When the message received by instance HSM Background Ensemble Scan state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_backgrndEnsembleScanStateHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM Background Scan State Getcomponent List Hndlr handler.
*   \param[in]				  msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in backgrndEnsembleScanStateHndlr handler.
*   \details                  When DAB Tuner Control receives Background Scan State Getcomponent List from main HSM, 
                              then DAB Tuner Control transits to Background Scan State Getcomponent List Hndlr.In this state DAB Tuner
							  Control do background scan operation.
*   \post-condition			  DAB Tuner Control is in instance HSM Background Scan State Getcomponent List handler state.
*   \ErrorHandling    		  When the message received by instance HSM Background Scan State Getcomponent List Hndlr
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_BackgrndScanStateGetcomponentListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Ts_Sys_Msg* msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM stop handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in active state handler.
*   \details                  When DAB Tuner Control receives shut down message from main HSM, 
                              then DAB Tuner Control transits to stop state.In this state DAB Tuner
							  Control shut down operation.
*   \post-condition			  DAB Tuner Control is in instance HSM stop handler state.
*   \ErrorHandling    		  When the message received by instance HSM stop state handler 
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_StopHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM scan get component list handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in start up scan state handler.
*   \details                  When DAB Tuner Control completes processing the ensemble service 
                              information during scan, then DAB Tuner Control transits to scan get
							  component list handler.In this state DAB Tuner Control retrieves
							  component information from SoC for all the services detected during
							  scan.
*   \post-condition			  DAB Tuner Control is in instance HSM scan get component handler state.
*   \ErrorHandling    		  When the message received by instance HSM scan state get component list 
                              state handler cannot be processed, then parent handles the message, 
							  when the message is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ScanStateGetcomponentListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM DAB to FM blending handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in listen state handler.
*   \details                  When DAB Tuner Control receives DAB_FM blending from main HSM, 
                              then DAB Tuner Control transits to DAB_FM_blendingHndlr Hndlr.In this state DAB Tuner
							  Control do FM linking operation.
*   \post-condition			  DAB Tuner Control is in instance HSM DAB_FM_blending handler state.
*   \ErrorHandling    		  When the message received by instance HSM Background Scan State Getcomponent List Hndlr
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_DAB_FM_blendingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM FM to DAB blending handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in DAB_FM_blendingHndlr handler.
*   \details                  When DAB Tuner Control receives FM_DAB_blending from main HSM, 
                              then DAB Tuner Control transits to FM_DAB_blendingHndlr Hndlr.In this state 
							  DAB linking operation is performed.
*   \post-condition			  DAB Tuner Control is in instance HSM FM_DAB_blending handler state.
*   \ErrorHandling    		  When the message received by instance HSM FM_DAB State Hndlr
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM initialisation.
*   \param[in]				  pst_msg
*   \param[out]				  None
*   \pre-condition			  DAB Tuner Control main HSM is initialised.
*   \details                  DAB Tuner Control initialises the instance HSM.
*   \post-condition			  DAB Tuner Control instance HSM is initialised.
*   \ErrorHandling    		  N/A 
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_INST_HSM_Init(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM message handling.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control receives request from upper layer that is to be 
                              processed by instance HSM.
*   \details                  When DAB Tuner Control receives any request from upper that has to be 
                              processed in instance HSM, this API routes the message to instance HSM.
*   \post-condition			  DAB Tuner Control instance HSM message is routed to instance HSM.
*   \ErrorHandling    		  N/A 
* 
***************************************************************************************************/
void DAB_TUNER_CTRL_INST_HSM_HandleMessage( Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

void DAB_Tuner_Ctrl_hsm_inst_SystemAbnormal(Tu16 dest_cid, Tu16 comp_msgid) ;

/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM start response.
*   \param[in]				  Cid
*   \param[in]				  Msgid
*   \pre-condition			  DAB Tuner Control instance HSM completes all start up operations.
*   \details                  When DAB Tuner Control instance HSM completes all start up operation then
                              instance HSM sends response to main HSM via this API. 
*   \post-condition			  DAB Tuner Control main HSM receives start response from instance HSM.
*   \ErrorHandling    		  N/A 
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_hsm_inst_start_response(Tu16 Cid, Tu16 Msgid);

/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM stop response.
*   \param[in]				  Cid
*   \param[in]				  Msgid
*   \pre-condition			  DAB Tuner Control instance HSM completes all shut down operations.
*   \details                  When DAB Tuner Control instance HSM completes all shut down operation then
                              instance HSM sends response to main HSM via this API. 
*   \post-condition			  DAB Tuner Control main HSM receives stop response from instance HSM.
*   \ErrorHandling    		  N/A 
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_hsm_inst_stop_response(Tu16 Cid, Tu16 Msgid);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM Get ensemble program list handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in TuneEnsembleHndlr handler.
*   \details                  When DAB Tuner Control receives GetCurrEnsembleProgmList from main HSM, 
                              then DAB Tuner Control transits to GetCurrEnsembleProgmListHndlr.In this state DAB Tuner
							  Control get current ensemble program list.
*   \post-condition			  DAB Tuner Control is in instance HSM GetCurrEnsembleProgmList handler state.
*   \ErrorHandling    		  When the message received by instance HSM GetCurrEnsembleProgmList Hndlr
                              cannot be processed, then parent handles the message, when the message 
							  is not handled by the parent state then HSM throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleCompListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SearchNextHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ErrorHandlingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_DAB_DAB_blendingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_NosignalListenHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_FM_DAB_LinkingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM Announcment handler.
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in Idle listen.
*   \details                  <to be wrritten>
*   \post-condition			  <to be wrritten>
*   \ErrorHandling    		  <to be wrritten>
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ESD_AllBandTuneHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_LinkingHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg);
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg);
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg);
void DAB_Tuner_Ctrl_Clear_Stationlist(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Make_Transition(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Check_AFtune_Scan(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Transition_to_Scanstate(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Print_Debug_Logs(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me ,Tu8 type);
void DAB_Tuner_Ctrl_Service_Following_OPerations(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);
Tbool DAB_Tuner_Ctrl_DABAlternate_Original_Freq_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Clear_FMDAB_LinkingData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Check_Request_Type(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);
#endif

/*=============================================================================
    end of file
=============================================================================*/
