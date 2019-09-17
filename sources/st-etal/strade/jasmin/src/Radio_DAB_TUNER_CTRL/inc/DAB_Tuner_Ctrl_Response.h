/*==================================================================================================
    start of file
==================================================================================================*/
/***************************************************************************************************/
/** \file DAB_Tuner_Ctrl_Response.h																	*
* Copyright (c) 2016, Jasmin Infotech Private Limited.                                              *
*  All rights reserved. Reproduction in whole or part is prohibited                                 *
*  without the written permission of the copyright owner.                                           *
*                                                                                                   *
*  Project              :  ST_Radio_Middleware                                                      *
*  Organization			:  Jasmin Infotech Pvt. Ltd.                                                *
*  Module				:  Radio DAB Tuner Control                                                  *
*  Description			:  This file contains DAB Tuner Control response API function declarations. *
*                                                                                                   *
*                                                                                                   *
*                                                                                                   *
***************************************************************************************************/

#ifndef DAB_TUNER_CTRL_RESPONSE_
#define DAB_TUNER_CTRL_RESPONSE_

/** \file */
/** \page DAB_TUNER_CTRL_RESPONSE_top DAB Tuner Control Response package

\subpage DAB_TUNER_CTRL_RESPONSE_Overview
\n
\subpage DAB_TUNER_CTRL_RESPONSE_API_Functions
\n
*/

/**\page DAB_TUNER_CTRL_RESPONSE_Overview Overview   
    \n
     DAB Tuner Control Response package consists of Response API's which are called by DAB application.   
    \n\n
*/

/** \page DAB_TUNER_CTRL_RESPONSE_API_Functions API Functions 
    <ul>
        <li> #DAB_Tuner_Ctrl_Response_Startup         : DAB Tuner Control start up response API. </li>
        <li> #DAB_Tuner_Ctrl_Response_Shutdown       : DAB Tuner Control shut down response API.</li>
		<li> #DAB_Tuner_Ctrl_Response_Activate         : DAB Tuner Control activate response API. </li>
		<li> #DAB_Tuner_Ctrl_Response_DeActivate         : DAB Tuner Control deactivate API. </li>
		<li> #DAB_Tuner_Ctrl_Response_SelectService         : DAB Tuner Control select service response API. </li>
		<li> #DAB_Tuner_Cntrl_Response_Scan         : DAB Tuner Control scan response. </li>
    </ul>
*/
/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Types.h"
#include "DAB_HAL_Interface.h"
#include "cfg_types.h"

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/
#define DAB_TUNER_CTRL_DATAPATH_CONFIG_DONE_RESID           (0x6494u)
#define DAB_TUNER_CTRL_RECEIVER_CONFIG_DONE_RESID           (0x6495u)
#define DAB_TUNER_CTRL_AUDIO_SOURCE_SELECT_DONE_RESID       (0x6496u)
#define DAB_TUNER_CTRL_ENABLE_DATASERVICE_DONE_RESID        (0x6497u)
#define DAB_TUNER_CTRL_DISABLE_DATA_SERVICE_DONE_RESID      (0x6498u)
#define DAB_TUNER_CTRL_DESTROY_DATAPATH_DONE_RESID          (0x6499u)
#define DAB_TUNER_CTRL_INST_HSM_STARTUP_DONE            	(0x6500u)
#define DAB_TUNER_CTRL_INST_SHUTDOWN_DONE                   (0x6501u)
#define GetProgrammeServiceList_repl                        (0x6502u) 
#define GetDataServiceList_repl                             (0x6503u) 
#define GetComponentList_repl                               (0x6504u) 
#define DAB_TUNER_CTRL_SELECT_SERVICE_REPLY                 (0x6505u) 
#define DAB_TUNER_CTRL_SELECT_COMPONENT_REPLY               (0x6506u) 
#define DESTROY_RECEIVER_REPLY                              (0x6507u) 
#define DAB_TUNER_CTRL_CREATE_RECEIVER_REPLY                (0x6508u) 
#define CREATE_RECEIVER_REPLY_FAILURE                       (0x6509u) 
#define DAB_TUNER_CTRL_SCAN_REPLY                           (0x650Au) 
#define DAB_TUNER_CTRL_TUNE_REPLY	                        (0x650Bu) 
#define DAB_TUNER_CTRL_CREATE_CONTEXT_REPLY  			    (0x650Cu)
#define DAB_TUNER_CTRL_SET_TUNE_STATUS_NOTIFICATION_REPLY  	(0x650Du)
#define DAB_TUNER_GET_VERSION_NUMBER_REPLY                  (0x650Eu)
#define DAB_TUNER_CTRL_SCAN_FAILURE							(0x650Fu)
#define DAB_TUNER_CTRL_TUNE_FAILURE							(0x6510u)
#define GET_PROGRAM_SERVLIST_REPLY                          (0x6511u) 
#define DAB_TUNER_CTRL_SEARCHNEXT_REPLY                     (0x6512u)
#define FIG_REPLY											(0x6513u)
#define DAB_TUNER_CTRL_AUDIO_STATUS_REPLY					(0x6514u)
#define DAB_TUNER_CTRL_SYNCHRONISATION_REPLY                (0x6515u)
#define DAB_TUNER_CTRL_SYNCHRONISATION_NOTIFICATION         (0x6516u)
#define STATUS_TIME_OUT										(0x6517u)
#define DAB_TUNER_CTRL_AUDIO_STATUS2_REPLY					(0x6518u)
#define DAB_TUNER_CTRL_DECODING_STATUS						(0x6519u)
#define PREPARE_FOR_BLENDING_REPLY							(0x651Au)
#define	PREPARE_FOR_BLENDING_NOTIFICATION					(0x651Bu)
#define	START_TIME_ALLIGNMENT_REPLY							(0x651Cu)
#define	START_TIME_ALLIGNMENT_NOTIFICATION					(0x651Du)
#define	START_BLENDING_REPLY								(0x651Eu)
#define GET_ENSEMBLE_PROPERTIES_REPLY                       (0x651Fu)
#define	GET_PROGRAMLIST_TIMEOUT								(0x6520u)
#define	GET_SEARCHNEXT_TIMEOUT								(0x6521u)
#define	SET_DRIFTTRACKING_REPLY								(0x6522u)
#define	REGISTER_SINK_TO_SERVCOMP_REPLY						(0x6523u)
#define	TUNE_TIME_OUT										(0x6524u)
#define	SELECT_COMPONENT_TIMEOUT							(0x6525u)
#define	SELECT_SERVICE_TIMEOUT								(0x6526u)
#define TUNE_ENSEMBLE_TIMEOUT                               (0x6527u)
#define UP_NOTIFY_TIMEOUT                                   (0x6528u)
#define SCAN_NOTIFY_TIMEOUT                                 (0x6529u)
#define SCAN2SEEK_NOTIFY_TIMEOUT                            (0x652Au)
#define	SERV_LIST_CHANGED_NOTIFIER_REPLY					(0x652Bu)
#define	SERV_PROPERTY_CHANGED_NOTIFIER_REPLY				(0x652Cu)
#define DAB_TUNER_CTRL_ANNOUNCEMENT_SWITCHING_REPLY			(0x652Du)
#define	DAB_TUNER_CTRL_TUNE_SIGNAL_PRESENT					(0x652Eu)
#define DAB_TUNER_CTRL_TUNE_MCI_PRESENT						(0x652Fu)
#define	GET_TUNESTATUS_TIMEOUT								(0x6530u)
#define START_DAB_DAB_LINKING								(0x6531u)
#define RESET_BLENDING_REPLY								(0x6532u)	
#define ABORT_SCAN2_REPLY									(0x6533u)

#define DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY				(0x6534u)
#define SYSTEM_CONTORL_REPLY								(0x6535u)
#define DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY			(0x6536u)
#define GET_VERSION_NUMBER_REPLY_TIMEOUT					(0x6537u)
#define CREATE_RECEIVER_REPLY_TIMEOUT						(0x6538u)
#define SET_TUNE_STATUS_NOTIFICATION_REPLY_TIMEOUT			(0x6539u)
#define SET_AUDIO_STATUS_REPLY_TIMEOUT						(0x653Au)
#define SET_SYNCHRONISATION_REPLY_TIMEOUT					(0x653Bu)
#define SERV_LIST_CHANGED_NOTIFIER_REPLY_TIMEOUT			(0x653Cu)
#define SERV_PROPERTY_CHANGED_NOTIFIER_REPLY_TIMEOUT		(0x653Du)
#define REGISTER_SINK_TO_SERVCOMP_REPLY_TIMEOUT				(0x653Eu)
#define ABORT_SCAN_TIMEOUT									(0x653Fu)
#define GET_COMPONENTLIST_TIMEOUT							(0x6540u)
#define GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT				(0x6541u)
#define ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT				(0x6542u)
#define RESET_BLENDING_REPLY_TIMEOUT						(0x6543u)
#define PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT			(0x6544u)
#define START_BLENDING_REPLY_TIMEOUT						(0x6545u)
#define START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT			(0x6546u)
#define DESTROY_RECEIVER_REPLY_TIMEOUT						(0x6547u)
#define SYSTEM_CONTORL_REPLY_TIMEOUT						(0x6548u)
#define DAB_TUNER_CTRL_CANCEL_ANNO_TIMEOUT					(0x6549u)
#define DAB_TUNER_CTRL_STOP_ANNO_TIMEOUT					(0x654Au)
#define DAB_TUNER_CTRL_START_ANNO_TIMEOUT					(0x654Bu)
#define DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT	(0x654Cu)
#define DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT		(0x654Du)
#define DAB_TUNER_CTRL_GET_SYNC_STATE_REPLY					(0x654Eu)
#define DAB_TUNER_CTRL_GET_SYNC_STATE_REPLY_TIME_OUT		(0x654Fu)
#define	DAB_TUNER_CTRL_TUNE_TO_SERVICE						(0x6550u)
#define DAB_TUNER_CTRL_AUDIO_ERROR_CONCEALMENT_REPLY		(0x6551u)
#define DAB_TUNER_CTRL_SET_AUDIO_ERROR_CONCEALMENT_REPLY	(0x6552u)
#define	DAB_TUNER_CTRL_RESET_FACTORY_SETTINGS_RESID			(0x6553u)
#define	DAB_TUNER_CTRL_START_TIMESTRETCH_REPLY				(0x6554u)
#define	START_TIME_STRETCH_REPLY_TIMEOUT					(0x6555u)
#define DAB_TUNER_CTRL_COMP_LIST_SORT_RESID					(0x6556u)
#define DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY					(0x6557u)
#define	DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT			(0x6558u)
#define DAB_TUNER_CTRL_SELECT_AUDIO_REPLY					(0x6559u)
#define DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT			(0x655Au)
#define DAB_TUNER_CTRL_STOP_SCAN_AUDIO						(0x655Bu)
#define AUTOSEEK_CMD_REPLY_TIMEOUT							(0x655Cu)
#define ABORT_AUTOSEEK_TIMEOUT								(0x655Du)
#define DAB_QUALITY_NOTIFICATION_MSGID						(0x655Eu)
#define DAB_TUNER_CTRL_ENABLE_QUALITY_MONITOR_DONE_RESID	(0x655Fu)
#define DAB_TUNER_CTRL_DESTROY_QUALITYMONITOR_DONE_RESID	(0x6560u)

/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/

/**************************************************************************************************/
/**	 \brief                 API for sending the reply status of start up request.
*   \param[in]				None
*   \param[out]				e_StartupReplyStatus
*   \pre-condition			DAB tuner control request start up is processed. 
*   \details                This API is sent after start up request is completed.The API sends either 
*                           success or failure status.
*   \post-condition			Start up response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/

void DAB_Tuner_Ctrl_Response_Startup(Te_RADIO_ReplyStatus e_StartupReplyStatus);

/**************************************************************************************************/
/**	 \brief                 API for sending the reply status of shut down request.
*   \param[in]				None
*   \param[out]				e_ShutdownReplyStatus
*   \pre-condition			DAB tuner control is in active state.
*   \details                This API is sent after shut down is requested.The API sends either 
*                           success or failure status.
*   \post-condition			Shut down response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/

void DAB_Tuner_Ctrl_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus);

/**************************************************************************************************/
/**	 \brief                 API for sending reply status of DAB tuner control activation request.
*   \param[in]				None
*   \param[out]				e_ActivateReplyStatus
*   \pre-condition			DAB tuner control activation request is processed.
*   \details                The API is sent after DAB tuner control activation request is processed.
*                           The API sends either success or failure status.
*   \post-condition			DAB tuner control activation response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_Response_Activate(Te_RADIO_ReplyStatus e_ActivateReplyStatus);

/**************************************************************************************************/
/**	 \brief                 API for sending reply status of DAB tuner control deactivation request.
*   \param[in]				None
*   \param[out]				e_DeActivateReplyStatus
*   \pre-condition			Deactivation of DAB tuner control is processed.
*   \details                The API is sent after DAB tuner control deactivation request is processed. 
*                           The API sends either success or failure status.
*   \post-condition			DAB tuner control deactivation response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_Response_DeActivate(Te_RADIO_ReplyStatus e_DeActivateReplyStatus);

/**************************************************************************************************/
/**	 \brief                 API for sending service selection request response.
*   \param[in]				None
*   \param[out]				e_SelectServiceReplyStatus
*   \param[out]				st_currentEnsembleData
*   \pre-condition			Service selection request is processed.
*   \details                The API is sent after service selection request is processed.The API sends 
*                           either success or failure status along with service ID and service component
*                           id information. 
*   \post-condition			Service select response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_Response_SelectService(Te_RADIO_ReplyStatus e_SelectServiceReplyStatus,Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData);

/**************************************************************************************************/
/**	 \brief                 API for sending scan request response.
*   \param[in]				None
*   \param[out]				e_ScanReplyStatus
*   \pre-condition			Scan request is processed.
*   \details                The API is sent after scan request is processed.The API sends 
*                           either success or failure status.
*   \post-condition			Scan response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/
void DAB_Tuner_Cntrl_Response_Scan (Te_RADIO_ReplyStatus e_ScanReplyStatus);


/**************************************************************************************************/
/**	 \brief                 API for sending service component seek response.
*   \param[in]				None
*   \param[out]				e_ServiceSeekUpDownReplyStatus
*   \param[out]				st_CurrentStationInfo
*   \pre-condition			Scan request is processed.
*   \details                The API is sent after scan request is processed.The API sends 
*                           either success or failure status.
*   \post-condition			Scan response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/

void DAB_Tuner_Ctrl_Response_Cancel(Te_RADIO_ReplyStatus e_CancelReplyStatus);
/**************************************************************************************************/
/**************************************************************************************************/
/**	 \brief                 API for sending enable DAB to FM linking request response.
*   \param[in]				None
*   \param[out]				Te_RADIO_ReplyStatus
*   \pre-condition			Enable DAB to FM linking request is processed.
*   \details                The API is sent after enable DAB to FM linking request is processed.The API sends 
*                           either success or failure status.
*   \post-condition			Enable DAB to FM linking response is sent.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_Response_EnableDABtoFMLinking(Te_RADIO_ReplyStatus e_LinkingStatus);

void DAB_Tuner_Ctrl_Response_StartAnnouncement(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_StartAnnoReplyStatus, Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type);

void DAB_Tuner_Ctrl_Response_StopAnnouncement(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_StopAnnoReplyStatus,Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type,Tu8 SubChId);
void DAB_Tuner_Ctrl_Response_CancelAnnouncement(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_CancelAnnoReplyStatus);

void DAB_Tuner_Ctrl_Response_SetAnnoConfig(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_SetAnnoConfigReplyStatus);

void DAB_Tuner_Ctrl_Response_DABTUNERRestart(Te_RADIO_ReplyStatus e_DABTUNERRestartReplyStatus);
void DAB_Tuner_Ctrl_Response_AFList(Ts_AFList st_AFList);
void DAB_Tuner_Ctrl_Response_DAB_AF_Settings(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_DAB_AF_Settings_Replystatus);
void DAB_Tuner_Ctrl_Response_Activate_Deactivate(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_ActivateDeactivateReplyStatus);
void DAB_Tuner_Ctrl_Response_Factory_Reset_Settings(Te_RADIO_ReplyStatus e_Factory_reset_settings_reply_status);
void DAB_Tuner_Ctrl_hsm_inst_factory_reset_response(Tu16 Cid, Tu16 Msgid);
void DAB_Tuner_Ctrl_hsm_inst_ETAL_response(Tu16 Cid, Tu16 Msgid);
void DAB_Tuner_Ctrl_Response_Abort_Scan(void);

#endif
/*==================================================================================================
    end of file
==================================================================================================*/


