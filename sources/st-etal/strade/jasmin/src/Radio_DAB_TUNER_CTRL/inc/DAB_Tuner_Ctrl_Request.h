/*==================================================================================================
    start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_Tuner_Ctrl_Request.h																   *
* Copyright (c) 2016, Jasmin Infotech Private Limited.                                             *
*  All rights reserved. Reproduction in whole or part is prohibited                                *
*  without the written permission of the copyright owner.                                          *
*                                                                                                  *
*  Project              :  ST_Radio_Middleware                                                     *
*  Organization			:  Jasmin Infotech Pvt. Ltd.                                               *
*  Module				:  Radio DAB Tuner Control                                                 *
*  Description			:  This file contains API declarations related to DAB Tuner Control        *
                           requests.                                                               *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
#ifndef DAB_TUNER_CTRL_REQUEST_
#define DAB_TUNER_CTRL_REQUEST_

/** \file */
/** \page DAB_TUNER_CTRL_Request_top DAB Tuner Control Request package

\subpage DAB_TUNER_CTRL_REQUEST_Overview
\n
\subpage DAB_TUNER_CTRL_REQUEST_API_Functions
\n
*/

/**\page DAB_TUNER_CTRL_REQUEST_Overview Overview   
    \n
     DAB Application Request package consists of Request API's which are called by DAB Application.   
    \n\n
*/

/** \page DAB_TUNER_CTRL_REQUEST_API_Functions API Functions 
    <ul>
        <li> #DAB_Tuner_Ctrl_Request_Startup         : Start of DAB Tuner Control. </li>
        <li> #DAB_Tuner_Ctrl_Request_Shutdown       : DAB Tuner Control shut down request.</li>
		<li> #DAB_Tuner_Ctrl_Request_SelectService         : Select service. </li>
		<li> #DAB_Tuner_Ctrl_Request_Activate         : DAB Tuner Control activation request </li>
		<li> #DAB_Tuner_Ctrl_Request_Scan         : Scan request. </li>
		<li> #DAB_Tuner_Ctrl_Request_DeActivate         : DAB Tuner Control deactivation request </li>
		<li> #DAB_Tuner_Ctrl_MsgHandlr         : Message handler API </li>
		<li> #UpdateParameterIntoMessage         : Data packing API. </li>
		<li> #ExtractParameterFromMessage         : Data unpacking API. </li>
		<li> #SYS_MSG_HANDLE_Call         :  API to update component is and message id</li>
    </ul>
*/
/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Types.h"
#include "DAB_HAL_Interface.h"
//#include "cfg_types.h"
/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/
#define DAB_TUNER_CTRL_STARTUP_REQID                    (0x6000u)
#define DAB_TUNER_CTRL_SHUTDOWN_REQID                   (0x6001u)
#define DAB_TUNER_CTRL_ACTIVATE_REQID                   (0x6002u)
#define DAB_TUNER_CTRL_TUNEBYFREQ_REQID                 (0x6003u)
#define DAB_TUNER_CTRL_SELSERV_REQID                    (0x6004u)
#define DAB_TUNER_CTRL_DESELBAND_REQID                  (0x6005u)
#define DAB_TUNER_CTRL_CREATE_RCVER_REQID               (0x6006u) 
#define DAB_TUNER_CTRL_SCAN_REQID			            (0x6007u)
#define DAB_TUNER_CTRL_SERVCOMPSEEK_REQID               (0x6008u)
#define DAB_TUNER_CTRL_DAB_FM_LINKING_ENABLE_REQID		(0x6009u)
#define	STATUS_CHECKING_MSG								(0x600Au)
#define	DAB_TUNER_CTRL_CANCEL_REQID						(0x600Bu)
#define DAB_TUNER_CTRL_DLS_ENABLE_REQID                 (0x600Cu)
#define	DAB_TUNER_CTRL_START_BACKGRND_SCAN				(0x600Du)
#define	DAB_TUNER_CTRL_FM_DAB_PI						(0x600Eu)
#define	START_BACKGROUND_SCAN							(0x600Fu)
#define DAB_TUNER_CTRL_ANNOUNCEMENT_CANCEL_REQID		(0x6010u)	
#define DAB_TUNER_CTRL_ANNO_ENABLE_REQID				(0x6011u)
#define DAB_TUNER_CTRL_ANNOUNCEMENT_START_SWITCHING_REQID	(0x6012u)
#define DAB_TUNER_CTRL_INTERNAL_ANNO_MSG				(0x6013u)
#define DAB_TUNER_CTRL_ANNOUNCEMENT_STOP_SWITCHING_REQID	(0x6014u)
#define DAB_TUNER_CTRL_CANCEL_ANNOUNCEMENT_REQID		(0x6015u)	
#define DAB_TUNER_CTRL_ANNO_CONFIG_REQID				(0x6016u)
#define TRIGGER_GET_AUDIO_STATUS_REQ					(0x6017u)
#define DAB_TUNER_CTRL_DABTUNER_RESTART_REQID			(0x6018u)
#define DAB_TUNER_CTRL_ENG_MODE_REQID					(0x6019u)
#define DAB_SIGNAL_STABLE_CHECK							(0x601Au)
#define	CHECK_HARDLINKS_FOR_TUNED_SID					(0x601Bu)
#define	DAB_TUNER_CTRL_ACTIAVTE_DEACTIVATE_REQID		(0x601Cu)
#define	DAB_TUNER_CTRL_INST_HSM_ACTIVATE_DEACTIVATE_REQ	 (0x601Du)
#define DAB_TUNER_CTRL_FM_DAB_STOP_LINKING				 (0x601Eu)
#define	DAB_TUNER_CTRL_DAB_AF_SETTINGS_REQID				 (0x601Fu)
#define DAB_TUNER_CTRL_INST_HSM_ABNORMAL_STATE			(0x602Au)	
#define	DAB_TUNER_CTRL_DAB_FACTORY_RESET_REQID			(0x602Bu)
#define DAB_INIT_ALL_LINKING_PROCESS					(0x602Cu)
#define DAB_TUNER_CTRL_SEEK_REQID						(0x602Du)

/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/

/**************************************************************************************************/
/**	 \brief                 API for requesting the start-up of DAB tuner control.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control is in inactive state.
*   \details                This API is called for start up of tuner control.This API initializes the 
*                           tuner and creates receiver handler for the DAB tuner control.
*   \post-condition			DAB tuner control is in active state.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_Request_Startup(Te_DAB_Tuner_Market  e_Market, Tu8 u8_SettingStatus, Tu8 u8_StartType);
/**************************************************************************************************/
/**	 \brief                 API for requesting shut down of DAB tuner control.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control is in active state.
*   \details                This API is called for shut down of DAB tuner control. This API de-initialises 
                            the tuner and destroys the receiver handler for the DAB tuner control.
*   \post-condition			DAB tuner control is in inactive state.
*   \ErrorHandling    		N/A.
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_Request_Shutdown(void);


/*****************************************************************************************************/
/**	 \brief                 API for requesting service selection.
*   \param[in]				u32_Frequency
*   \param[in]				u16_Eid
*   \param[in]				u32_Sid
*   \param[in]				u16_SCid
*   \param[out]				None
*   \pre-condition			Tuned to an ensemble.
*   \details                This API is called for selecting a service from service list.The API selects 
                            a service of the ensemble the receiver instance is tuned to.In one context 
							only one service is selected.When a new service is selected in a context, 
							a previously selected service in that context is implicitly de-selected. 							
*   \post-condition			A service is selected from an ensemble.
*   \ErrorHandling    		In case selected Service is invalid and/or SOC doesn't respond within 
                            the time, then DAB tuner control reports an error.The error is reported 
							via DAB_Tuner_Ctrl_Response_SelectService API.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Request_SelectService(Tu32 u32_Frequency,Tu16 u16_Eid,Tu32 u32_Sid,Tu16 u16_SCid);


/*****************************************************************************************************/
/**	 \brief                 API for requesting activation of the DAB tuner control.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control is in inactive state.
*   \details                This API is called for activating the DAB tuner control.The API initialises 
                            all relevant parameters for the activation of DAB tuner control.						
*   \post-condition			DAB tuner control is in active state.
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Request_Activate(void);


/*****************************************************************************************************/
/**	 \brief                 API for scanning stations in a band. The scan request may be specified
*   \param[in]				Direction
*   \param[out]				None
*   \pre-condition			DAB is Active source.
*   \details                This function can be called when scan is requested for preparing/updating 
                            the station list.						
*   \post-condition			can list is prepared.
*   \ErrorHandling    		If no service is found during scan,then the system reports an error.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Request_Scan(Tbool b_Direction);


/*****************************************************************************************************/
/**	 \brief                 API for requesting deactivation of the DAB tuner control.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB tuner control is in active state.
*   \details                This API is called for deactivation of DAB tuner control.The API de-initialises 
                            all relevant parameters for the deactivation of DAB tuner control.						
*   \post-condition			DAB tuner control is in inactive state.
*   \ErrorHandling    		In case tuner doesn't respond for Select Band Request,then the system 
                            reports an error.The error is reported via DAB_Tuner_Ctrl_Response_DeActivate
* 
******************************************************************************************************/

void DAB_Tuner_Ctrl_Request_DeActivate(void);

/*****************************************************************************************************/
/**	 \brief                 API for handling the messages received by DAB tuner control.
*   \param[in]				msg
*   \param[out]				None
*   \pre-condition			DAB tuner control receives request or reponse from other layers.
*   \details                When any message is received by DAB Tuner Control, this API routes the
                            message to the particular handler.						
*   \post-condition			DAB tuner control message is routed.
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_MsgHandlr(Ts_Sys_Msg* msg);


/*****************************************************************************************************/
/**	 \brief                 API for packing the message to bytes.
*   \param[in]				vp_parameter
*   \param[in]				u8_ParamLength
*   \param[in]				pu16_Datalength
*   \param[out]				pu8_data
*   \pre-condition			DAB tuner control receives message.
*   \details                When any message is received by DAB Tuner Control, this API packs the data
                            into byte format as per mailbox structure.					
*   \post-condition			DAB tuner control message is packed.
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void UpdateParameterIntoMessage(char *pu8_data,const void *vp_parameter, Tu8 u8_ParamLength, Tu16 *pu16_Datalength);

/*****************************************************************************************************/
/**	 \brief                 API for unpacking the message to DAB Tuner Control structure type.
*   \param[in]				pu8_DataSlot
*   \param[in]				u8_ParamLength
*   \param[in]				pu32_index
*   \param[out]				vp_Parameter
*   \pre-condition			DAB tuner control receives message.
*   \details                When any message is received by DAB Tuner Control, this API unpacks and
                            updates the vp_Parameter that was passed by DAB Tuner control.					
*   \post-condition			DAB tuner control message is unpacked.
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void ExtractParameterFromMessage(void *vp_Parameter,const char *pu8_DataSlot, Tu8 u8_ParamLength, Tu32 *pu32_index);

/*****************************************************************************************************/
/**	 \brief                 API for updating component id and message id.
*   \param[in]				cid
*   \param[in]				msgid
*   \pre-condition			DAB tuner control needs to send a message.
*   \details                When any message is to be sent by DAB Tuner Control, this API updates the
                            component id and message id.					
*   \post-condition			DAB tuner control component and message id updated.
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
Ts_Sys_Msg* SYS_MSG_HANDLE_Call(Tu16 cid,Tu16 msgid);

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Request_EnableDABtoFMLinking                         		 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Request_EnableDABtoFMLinking(Te_DAB_Tuner_DABFMLinking_Switch e_status);

void DAB_Tuner_Ctrl_Request_Cancel(Te_DAB_Tuner_Ctrl_CancelType e_CancelType);

void DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State(void);
/*****************************************************************************************************/
/**	 \brief                 API for starting the Announcement
*   \param[in]				Announcment type and subchannel id
*   \param[out]				None
*   \pre-condition			DAB is Active source.
*   \details                This function can be called Announcment has to be started
*   \post-condition			Announcment is started.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Request_StartAnnouncement(Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type,Tu8 u8_Subchannelid);

/*****************************************************************************************************/
/**	 \brief                 API for stoping the Announcement
*   \param[in]				Announcment type and subchannel id
*   \param[out]				None
*   \pre-condition			DAB is Active source.
*   \details                This function can be called Announcment has to be stopped						
*   \post-condition			Announcment is stopped
*   \ErrorHandling    		None
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Request_StopAnnouncement(Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type,Tu8 SubChId);


/*****************************************************************************************************/
/** DAB_Tuner_Ctrl_Internal_Msg_To_Anno_State
******************************************************************************************************/
void DAB_Tuner_Ctrl_Internal_Msg_To_Anno_State(void);


/*****************************************************************************************************/
/**	 \brief                 API for cancel announcement.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			DAB is in announcement or it might tuned to service in an ensemble.
*   \details                This API is called to cancel the announcement.						
*   \post-condition			DAB restore to original service. \n
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Request_CancelAnnouncement(Tu8 u8_DiffChannelSubChId, Tu8 u8_DiffChannelClusterid) ;
/*****************************************************************************************************/
/**	 \brief                 API for requesting announcement start or stop.
*   \param[in]				Tu16 u16_AnnoConfig
*							(0xFFFF( Announcement is ON and all type are supported)
*							0x0000( Announcement is off)
*							0x0001( Alarm  Type )
*							0x0002( Road traffic)
*							0x0004(Transport)
*							0x0008( Warning)
*							0x0010( News)
*							0x0020( Weather)
*							0x0040( Event)
*							0x0080(special event)
*							0x0100( Programme)
*							0x0200( sport)
*							0x0400( Financial))
*   \param[out]				None
*   \pre-condition			DAB tuner control tuned to service in an ensemble.
*   \details                This API is called to start or stop an announcement.						
*   \post-condition			DAB announcement is tuned or tunned announcement is stop in same ensemble. \n
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Request_SetAnnoConfig(Tu16 u16_AnnoConfig);

void DAB_Tuner_Ctrl_Request_GetSIDStation(Tu16 u16_FM_DAB_SID, Tu16 u16_FM_DAB_SCID);
void DAB_TUNER_CTRL_Internal_Msg(void);
void DAB_Tuner_Ctrl_Enable_Fig_Notifications(void);
void DAB_Tuner_Ctrl_Enable_Quality_Notifications(void);
void DAB_Tuner_Ctrl_Disable_Fig_Notifications(void);
void DAB_Tuner_Ctrl_Request_DABTUNERRestart(void);
void DAB_Tuner_Ctrl_Disable_Quality_Notifications(void);
void DAB_Tuner_Ctrl_Request_ENG_Mode(Te_DAB_Tuner_Ctrl_Eng_Mode_Request e_DAB_Tuner_Ctrl_Eng_Mode_Request);
void DAB_Tuner_Ctrl_Request_StopFmDabLinking(Te_FmtoDAB_Reqstatus e_FmtoDAB_Reqstatus);
void DAB_Tuner_Ctrl_RDS_Settings_Request(Te_DAB_Tuner_Ctrl_DAB_AF_Settings	e_DAB_Tuner_Ctrl_DAB_AF_Settings);
void DAB_Tuner_Ctrl_Request_Activate_Deactivate(Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus e_DAB_Tuner_Ctrl_ActivateDeActivateStauts);
void DAB_Tuner_Ctrl_FactoryReset_Request(void);
void DAB_Tuner_Ctrl_Request_AutoSeekUpDown(Tu32 u32_Frequency, Te_RADIO_DirectionType e_SeekDirection, Tbool b_SeekStarted);
#endif
/*==================================================================================================
    end of file
==================================================================================================*/

