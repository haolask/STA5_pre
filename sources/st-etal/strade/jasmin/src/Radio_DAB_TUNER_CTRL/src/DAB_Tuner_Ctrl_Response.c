/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file DAB_Tuner_Ctrl_Response.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: DAB Tuner Control															     	*
*  Description			: This file contains the response APIs of DAB Tuner Control.						*
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Response.h"
#include "DAB_Tuner_Ctrl_Request.h"
#include "dab_app_hsm.h"
/*
#include "DAB_Tuner_Ctrl_Types.h"
#include "DAB_Tuner_Ctrl_Response.h"
#include "hsm_api.h"
#include "cfg_types.h"
#include "sys_task.h"
#include "DAB_Tuner_Ctrl_messages.h"
#include "DAB_Tuner_Ctrl_Request.h"
#include "dab_app_hsm.h"
#include <string.h>
*/
/*--------------------------------------------------------------------------------------------------
    variables (extern)
--------------------------------------------------------------------------------------------------*/
 Ts_Sys_Msg DAB_Tuner_Ctrl_resmsg;

/*================================================================================================*/
/*  void DAB_Tuner_Ctrl_Response_Startup(Te_RADIO_ReplyStatus e_StartupReplyStatus)   */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Response_Startup(Te_RADIO_ReplyStatus e_StartupReplyStatus)
{
	Ts_Sys_Msg *startUpreply =&DAB_Tuner_Ctrl_resmsg;
   //Tu temp =0;
	memset(startUpreply,0,sizeof(Ts_Sys_Msg));

	startUpreply->src_id     = RADIO_DAB_TUNER_CTRL;
	startUpreply->dest_id    = RADIO_DAB_APP;
	startUpreply->msg_id     = DAB_APP_STARTUP_DONE_RESID;
	
	UpdateParameterIntoMessage(startUpreply->data,&e_StartupReplyStatus,sizeof(e_StartupReplyStatus), &(startUpreply->msg_length));

	SYS_SEND_MSG(startUpreply);
 
}

/*=========================================================================================================================================================================*/
/* void DAB_Tuner_Ctrl_Response_SelectService(Te_RADIO_ReplyStatus e_SelectServiceReplyStatus,Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData)  */                           
/*=========================================================================================================================================================================*/
void DAB_Tuner_Ctrl_Response_SelectService(Te_RADIO_ReplyStatus e_SelectServiceReplyStatus,Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData)
{
	Ts_Sys_Msg *GetSelectServicereply =&DAB_Tuner_Ctrl_resmsg;
	memset(GetSelectServicereply,0,sizeof(Ts_Sys_Msg));
    
	GetSelectServicereply->src_id     = RADIO_DAB_TUNER_CTRL;
	GetSelectServicereply->dest_id    = RADIO_DAB_APP;
	GetSelectServicereply->msg_id     = DAB_APP_PLAY_SEL_STN_RESID;
	GetSelectServicereply->msg_length = 0;
	UpdateParameterIntoMessage(GetSelectServicereply->data,&e_SelectServiceReplyStatus,sizeof(e_SelectServiceReplyStatus), &(GetSelectServicereply->msg_length));
	UpdateParameterIntoMessage(GetSelectServicereply->data,&st_currentEnsembleData,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo), &(GetSelectServicereply->msg_length));
	
	SYS_SEND_MSG(GetSelectServicereply);
   
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus) */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus)
{
	Ts_Sys_Msg *Shutdownreply =&DAB_Tuner_Ctrl_resmsg;
	memset(Shutdownreply,0,sizeof(Ts_Sys_Msg));
    
	Shutdownreply->src_id     = RADIO_DAB_TUNER_CTRL;
	Shutdownreply->dest_id    = RADIO_DAB_APP;
	Shutdownreply->msg_id     = DAB_APP_SHUTDOWN_RESID;
	
	
	UpdateParameterIntoMessage(Shutdownreply->data,&e_ShutdownReplyStatus,sizeof(e_ShutdownReplyStatus), &(Shutdownreply->msg_length));

	SYS_SEND_MSG(Shutdownreply);
   
}

/*================================================================================================*/
/* void DAB_Tuner_Cntrl_Response_Scan (Te_RADIO_ReplyStatus e_ScanReplyStatus)           */
/*================================================================================================*/

void DAB_Tuner_Cntrl_Response_Scan (Te_RADIO_ReplyStatus e_ScanReplyStatus)
{

	Ts_Sys_Msg *ScanStationListreply =&DAB_Tuner_Ctrl_resmsg;
   
   memset(ScanStationListreply,0,sizeof(Ts_Sys_Msg));
   ScanStationListreply->src_id    = RADIO_DAB_TUNER_CTRL;
   ScanStationListreply->dest_id   = RADIO_DAB_APP;
   ScanStationListreply->msg_id     = DAB_APP_SCAN_RESID;
   
   UpdateParameterIntoMessage(ScanStationListreply->data,&e_ScanReplyStatus,sizeof(e_ScanReplyStatus),&(ScanStationListreply->msg_length));
   SYS_SEND_MSG(ScanStationListreply);

}


void DAB_Tuner_Ctrl_Response_Activate_Deactivate(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_ActivateDeactivateReplyStatus)
{
	
	Ts_Sys_Msg *ActivateReplyStatus =&DAB_Tuner_Ctrl_resmsg;
   
   memset(ActivateReplyStatus,0,sizeof(Ts_Sys_Msg));
   ActivateReplyStatus->src_id    = RADIO_DAB_TUNER_CTRL;
   ActivateReplyStatus->dest_id   = RADIO_DAB_APP;
   ActivateReplyStatus->msg_id     = DAB_APP_ACTIVATE_DEACTIVATE_RESID;
   
   UpdateParameterIntoMessage(ActivateReplyStatus->data,&e_DAB_Tuner_Ctrl_ActivateDeactivateReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(ActivateReplyStatus->msg_length));
   SYS_SEND_MSG(ActivateReplyStatus);

}
void DAB_Tuner_Ctrl_Response_Activate(Te_RADIO_ReplyStatus e_ActivateReplyStatus)
{
	
	Ts_Sys_Msg *ActivateReplyStatus =&DAB_Tuner_Ctrl_resmsg;
   
   memset(ActivateReplyStatus,0,sizeof(Ts_Sys_Msg));
   ActivateReplyStatus->src_id    = RADIO_DAB_TUNER_CTRL;
   ActivateReplyStatus->dest_id   = RADIO_DAB_APP;
   ActivateReplyStatus->msg_id     = DAB_APP_ACTIVATE_RESID;
   
   UpdateParameterIntoMessage(ActivateReplyStatus->data,&e_ActivateReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(ActivateReplyStatus->msg_length));
   SYS_SEND_MSG(ActivateReplyStatus);

}

void DAB_Tuner_Ctrl_Response_DeActivate(Te_RADIO_ReplyStatus e_DeActivateReplyStatus)
{
	
	Ts_Sys_Msg *DeActivateReplyStatus =&DAB_Tuner_Ctrl_resmsg;
   
   memset(DeActivateReplyStatus,0,sizeof(Ts_Sys_Msg));
   DeActivateReplyStatus->src_id    = RADIO_DAB_TUNER_CTRL;
   DeActivateReplyStatus->dest_id   = RADIO_DAB_APP;
   DeActivateReplyStatus->msg_id     = DAB_APP_DEACTIVATE_RESID;
   UpdateParameterIntoMessage(DeActivateReplyStatus->data,&e_DeActivateReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(DeActivateReplyStatus->msg_length));
   SYS_SEND_MSG(DeActivateReplyStatus);

}


void DAB_Tuner_Ctrl_Response_Cancel(Te_RADIO_ReplyStatus e_CancelReplyStatus)
{
	Ts_Sys_Msg *CancelReplyStatus =&DAB_Tuner_Ctrl_resmsg;
   
   memset(CancelReplyStatus,0,sizeof(Ts_Sys_Msg));
   CancelReplyStatus->src_id    = RADIO_DAB_TUNER_CTRL;
   CancelReplyStatus->dest_id   = RADIO_DAB_APP;
   CancelReplyStatus->msg_id     = DAB_APP_CANCEL_RESID;

UpdateParameterIntoMessage(CancelReplyStatus->data,&e_CancelReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(CancelReplyStatus->msg_length));
   
 SYS_SEND_MSG(CancelReplyStatus);

}


void DAB_Tuner_Ctrl_hsm_inst_start_response(Tu16 Cid, Tu16 Msgid)
{
	Ts_Sys_Msg *msg =&DAB_Tuner_Ctrl_resmsg;

	msg->dest_id = Cid;
	msg->msg_id = Msgid;

   SYS_SEND_MSG(msg);

}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_hsm_inst_stop_response(Tu16 Cid, Tu16 Msgid)                               */
/*================================================================================================*/
void DAB_Tuner_Ctrl_hsm_inst_stop_response(Tu16 Cid, Tu16 Msgid)
{
	Ts_Sys_Msg *msg =&DAB_Tuner_Ctrl_resmsg;

	msg->dest_id = Cid;
	msg->msg_id = Msgid;

   SYS_SEND_MSG(msg);

}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_hsm_inst_factory_reset_response(Tu16 Cid, Tu16 Msgid)                               */
/*================================================================================================*/
void DAB_Tuner_Ctrl_hsm_inst_factory_reset_response(Tu16 Cid, Tu16 Msgid)
{
	Ts_Sys_Msg *msg =&DAB_Tuner_Ctrl_resmsg;

	msg->dest_id = Cid;
	msg->msg_id = Msgid;

   SYS_SEND_MSG(msg);

}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Response_EnableDABtoFMLinking                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Response_EnableDABtoFMLinking(Te_RADIO_ReplyStatus e_LinkingStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	
	p_msg->src_id = RADIO_DAB_TUNER_CTRL;
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DAB_FM_LINKING_ENABLE_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_LinkingStatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Response_Factory_Reset_Settings                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Response_Factory_Reset_Settings(Te_RADIO_ReplyStatus e_Factory_reset_settings_reply_status)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->src_id = RADIO_DAB_TUNER_CTRL;
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_FACTORY_RESET_DONE_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_Factory_reset_settings_reply_status,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}
/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Response_StartAnnouncement                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Response_StartAnnouncement(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_StartAnnoReplyStatus, Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->src_id = RADIO_DAB_TUNER_CTRL;
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_START_ANNO_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_StartAnnoReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&e_announcement_type,sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&(p_msg->msg_length));	
	SYS_SEND_MSG(p_msg);
	
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Response_StopAnnouncement                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Response_StopAnnouncement(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_StopAnnoReplyStatus,Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type,Tu8 SubChId)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->src_id = RADIO_DAB_TUNER_CTRL;
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_STOP_ANNO_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_StopAnnoReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&e_announcement_type,sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&SubChId,sizeof(Tu8),&(p_msg->msg_length));		
	SYS_SEND_MSG(p_msg);
}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Response_CancelAnnouncement                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Response_CancelAnnouncement(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_CancelAnnoReplyStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->src_id = RADIO_DAB_TUNER_CTRL;
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_CANCEL_ANNO_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_CancelAnnoReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
	
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Response_SetAnnoConfig(Tu8 u8_Result)                 */                      
/*================================================================================================*/
void DAB_Tuner_Ctrl_Response_SetAnnoConfig(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_SetAnnoConfigReplyStatus)
{
	
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->src_id = RADIO_DAB_TUNER_CTRL;
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_ANNO_CONFIG_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_SetAnnoConfigReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);

}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Response_DABTUNERRestart									                 */                      
/*================================================================================================*/
void DAB_Tuner_Ctrl_Response_DABTUNERRestart(Te_RADIO_ReplyStatus e_DABTUNERRestartReplyStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DABTUNER_RESTART_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_DABTUNERRestartReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}
/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Response_AFList	                   			 */
/*===========================================================================*/	
void DAB_Tuner_Ctrl_Response_AFList(Ts_AFList st_AFList)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_AF_LIST_RESID;
	UpdateParameterIntoMessage(p_msg->data,&st_AFList,sizeof(Ts_AFList),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);	
	
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Response_DAB_AF_Settings             				 */
/*===========================================================================*/	
void DAB_Tuner_Ctrl_Response_DAB_AF_Settings(Te_RADIO_ReplyStatus e_DAB_Tuner_Ctrl_DAB_AF_Settings_Replystatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DAB_AF_SETTINGS_RESID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_DAB_AF_Settings_Replystatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);	
	
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_hsm_inst_ETAL_response(Tu16 Cid, Tu16 Msgid)                               */
/*================================================================================================*/
void DAB_Tuner_Ctrl_hsm_inst_ETAL_response(Tu16 Cid, Tu16 Msgid)
{   
    Ts_Sys_Msg *msg =&DAB_Tuner_Ctrl_resmsg;
    msg->dest_id = Cid;
    msg->msg_id = Msgid;

   SYS_SEND_MSG(msg);

}

void DAB_Tuner_Ctrl_Response_Abort_Scan(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_resmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_ABORT_SCAN_RESID;
	SYS_SEND_MSG(p_msg);
}

/*=============================================================================
    end of file
=============================================================================*/

