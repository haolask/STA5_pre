/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file DAB_Tuner_Ctrl_Notify.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        * 
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: DAB Tuner Control															     	*
*  Description			: This file contains functions definitions of DAB Tuner Control notifications.		*
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
      includes
-----------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Notify.h" 
#include "dab_app_hsm.h"
#include "DAB_Tuner_Ctrl_Request.h"
/*
#include "DAB_Tuner_Ctrl_Types.h"
#include "dab_app_hsm.h"
#include "DAB_Tuner_Ctrl_messages.h"
#include "DAB_Tuner_Ctrl_Notify.h" 
#include "DAB_Tuner_Ctrl_Request.h"
#include <string.h>
*/
/*--------------------------------------------------------------------------------------------------
    variables (extern)
--------------------------------------------------------------------------------------------------*/
Ts_Sys_Msg DAB_Tuner_Ctrl_Notmsg;


/*================================================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_FrequencyChange(Tu32 u32_Frequency)                                */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Notify_FrequencyChange(Tu32 u32_Frequency)
{
Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
memset(p_msg,0,sizeof(Ts_Sys_Msg));
p_msg->dest_id = RADIO_DAB_APP;
p_msg->msg_id = DAB_APP_FREQ_CHANGE_NOTIFYID;
UpdateParameterIntoMessage(p_msg->data,&u32_Frequency,sizeof(Tu32),&(p_msg->msg_length));
SYS_SEND_MSG(p_msg);
}
/*================================================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_Tuner_SharedMemoryType)                               */
/*================================================================================================*/

void DAB_Tuner_Ctrl_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_Tuner_SharedMemoryType)
{
Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
memset(p_msg,0,sizeof(Ts_Sys_Msg));
p_msg->dest_id = RADIO_DAB_APP;
p_msg->msg_id = DAB_APP_STL_UPDATE_NOTIFYID;
UpdateParameterIntoMessage(p_msg->data,&e_Tuner_SharedMemoryType,sizeof(Te_RADIO_SharedMemoryType),&(p_msg->msg_length));
SYS_SEND_MSG(p_msg);
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Notify_TunerStatus(Ts_Tuner_Status_Notification st_tunerstatus)                             */
/*================================================================================================*/

void DAB_Tuner_Ctrl_Notify_TunerStatus(Ts_Tuner_Status_Notification st_tunerstatus)
{
	
Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
memset(p_msg,0,sizeof(Ts_Sys_Msg));
p_msg->dest_id = RADIO_DAB_APP;
p_msg->msg_id = DAB_APP_STATUS_NOTIFYID;
UpdateParameterIntoMessage(p_msg->data,&st_tunerstatus,sizeof(Ts_Tuner_Status_Notification),&(p_msg->msg_length));
SYS_SEND_MSG(p_msg);

}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_PICodeList                         					 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_PICodeList(Ts_PI_Data st_DAB_PICodeList, Tu8 u8_QualityMin, Tu8 u8_QualityMax,Tu32 Sid, Tu8 u8_linktype)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_PICODE_LIST_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&st_DAB_PICodeList,sizeof(Ts_PI_Data),&p_msg->msg_length);
	UpdateParameterIntoMessage(p_msg->data,&u8_QualityMin,sizeof(Tu8),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u8_QualityMax,sizeof(Tu8),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&Sid,sizeof(Tu32),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u8_linktype,sizeof(Tu8),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}


void DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DAB_FM_BLENDING_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_LinkingStatus,sizeof(Te_RADIO_DABFM_LinkingStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);	
	
}


void DAB_Tuner_Ctrl_Notify_FMtoDAB_linked_Station(Ts_dab_tuner_ctrl_fmdab_linkinfo st_FM_DAB_Stationinfo)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_FM_DAB_LINKING_STATION_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&st_FM_DAB_Stationinfo,sizeof(Ts_dab_tuner_ctrl_fmdab_linkinfo),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);	
	
}

void DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(Tu8 Quality, Te_dab_tuner_ctrl_fmdab_linkstatus e_LinkingStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_FM_DAB_LINKING_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&Quality,sizeof(Tu8),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&e_LinkingStatus,sizeof(Te_dab_tuner_ctrl_fmdab_linkstatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);	
}



/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_BestPI                         					 	 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_BestPI(Tu16 PICode, Tu8 u8_Quality , Te_Tuner_Ctrl_BestPI_Type e_BestPI_Type)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = BEST_PI_RECEIVED_NOTIFICATION;
	UpdateParameterIntoMessage(p_msg->data,(const void *)&PICode,sizeof(Tu16),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,(const void *)&u8_Quality,sizeof(Tu8),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,(const void *)&e_BestPI_Type,sizeof(Te_Tuner_Ctrl_BestPI_Type),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_DABtoFM_LinkingStatus                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_DABtoFM_LinkingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_DAB_FM_LINKING_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_LinkingStatus,sizeof(Te_RADIO_DABFM_LinkingStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_PIQuality                   					 	 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_PIQuality(Tu8 u8_Quality)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_PI_QUALITY_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&u8_Quality,sizeof(Tu8),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_DLSdata                  					 	 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_DLSdata(Ts_dab_DLS_data st_Dynamicdata)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DAB_DLS_DATA_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&st_Dynamicdata,sizeof(Ts_dab_DLS_data),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}
/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_SLSdata                  					 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_SLSdata(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg, 0, sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DAB_SLS_DATA_NOTIFYID;
	SYS_SEND_MSG(p_msg);
}

void DAB_Tuner_Ctrl_Notify_ReConfiguration(Te_Tuner_Ctrl_ReConfigType e_ReConfigType,Tu32 u32_Frequency, Tu16 u16_Eid, Tu32 u32_SId, Tu16 u16_SCId)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_RECONFIG_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_ReConfigType,sizeof(Te_Tuner_Ctrl_ReConfigType),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u32_Frequency,sizeof(Tu32),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u16_Eid,sizeof(Tu16),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u32_SId,sizeof(Tu32),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u16_SCId,sizeof(Tu16),&(p_msg->msg_length));	
	SYS_SEND_MSG(p_msg);
	
}

void DAB_Tuner_Ctrl_Notify_AnnoStatus(Tu8 u8_announcement_status,Ts_DAB_Tuner_Anno_Swtch_Info st_DAB_Tuner_Anno_Swtch_Info)
{
	
   Ts_Sys_Msg *announcement_update =&DAB_Tuner_Ctrl_Notmsg;
   memset(announcement_update,0,sizeof(Ts_Sys_Msg));
   announcement_update->src_id    = RADIO_DAB_TUNER_CTRL;
   announcement_update->dest_id   = RADIO_DAB_APP;
   announcement_update->msg_id     = DAB_APP_ANNO_NOTIFYID;
   UpdateParameterIntoMessage(announcement_update->data,&u8_announcement_status,sizeof(Tu8),&(announcement_update->msg_length));
   UpdateParameterIntoMessage(announcement_update->data,&st_DAB_Tuner_Anno_Swtch_Info,sizeof(Ts_DAB_Tuner_Anno_Swtch_Info),&(announcement_update->msg_length));
   SYS_SEND_MSG(announcement_update);
}

void DAB_Tuner_Ctrl_Notify_AnnoStationInfo(Ts_Tuner_Ctrl_CurrentEnsembleInfo st_CurrentTunedAnnoInfo)
{
	
   Ts_Sys_Msg *announcement_update =&DAB_Tuner_Ctrl_Notmsg;
   memset(announcement_update,0,sizeof(Ts_Sys_Msg));
   announcement_update->src_id    = RADIO_DAB_TUNER_CTRL;
   announcement_update->dest_id   = RADIO_DAB_APP;
   announcement_update->msg_id     = DAB_APP_ANNO_STATION_INFO_NOTIFYID;
   UpdateParameterIntoMessage(announcement_update->data,&st_CurrentTunedAnnoInfo,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo),&(announcement_update->msg_length));
   SYS_SEND_MSG(announcement_update);
}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_Hardlinks_Status                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_Hardlinks_Status(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DAB_FM_HARDLINKS_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_LinkingStatus,sizeof(Te_RADIO_DABFM_LinkingStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_AnnoSignalLoss_Status                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_AnnoSignalLoss_Status(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_ANNO_SIGNAL_LOSS_NOTIFYID;
	SYS_SEND_MSG(p_msg);
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_ComponentStatus	                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_ComponentStatus(Te_RADIO_Comp_Status e_ComponentStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_COMPONENT_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_ComponentStatus,sizeof(Te_RADIO_Comp_Status),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_AMFMTunerStatus	                   			 */
/*===========================================================================*/
void  DAB_Tuner_Ctrl_Notify_AMFMTunerStatus(Te_RADIO_Comp_Status e_AMFMTUNERStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_AMFMTUNER_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_AMFMTUNERStatus,sizeof(Te_RADIO_Comp_Status),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}	

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_StartBackgroundScan	                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_StartBackgroundScan(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_BACKGROUND_SCAN_START_NOTIFYID;
	SYS_SEND_MSG(p_msg);
}
/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_CompListStatus	                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_CompListStatus(Tu32 u32_Frequency, Tu16 u16_Eid, Te_RADIO_ReplyStatus e_ReplyStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_GET_COMP_LIST_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&u32_Frequency,sizeof(Tu32),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u16_Eid,sizeof(Tu16),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&e_ReplyStatus,sizeof(Te_RADIO_ReplyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_Synchronisation	                   			 */
/*===========================================================================*/

void DAB_Tuner_Ctrl_Notify_Synchronisation(Tu8 u8_ChannelSync)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_SYNCHRONISATION_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&u8_ChannelSync,sizeof(Tu8),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}	

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_StationNootAvail_StrategyStatus	                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_StationNootAvail_StrategyStatus(Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_STRATERGY_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus,sizeof(Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus	                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(Te_DAB_Tuner_Ctrl_LearnMemAFStatus e_DAB_Tuner_Ctrl_LearnMemAFStatus ,Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_DAB_AF_LEARN_MEM_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_LearnMemAFStatus,sizeof(Te_DAB_Tuner_Ctrl_LearnMemAFStatus),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&st_currentEnsembleData,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Notify_SignalStatus(Te_DAB_Tuner_Ctrl_SignalStatus e_DAB_Tuner_Ctrl_SignalStatus)                             */
/*================================================================================================*/

void DAB_Tuner_Ctrl_Notify_SignalStatus(Te_DAB_Tuner_Ctrl_SignalStatus e_DAB_Tuner_Ctrl_SignalStatus)
{
	
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_SIGNAL_STATUS_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_SignalStatus,sizeof(Te_DAB_Tuner_Ctrl_SignalStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*===============================================================================================================*/
/* void DAB_Tuner_Ctrl_Notify_DAB_DAB_Status(Te_DAB_Tuner_Ctrl_DAB_DAB_Status e_DAB_Tuner_Ctrl_DAB_DAB_Status)   */
/*===============================================================================================================*/

void DAB_Tuner_Ctrl_Notify_DAB_DAB_Status(Te_DAB_Tuner_Ctrl_DAB_DAB_Status 	e_DAB_Tuner_Ctrl_DAB_DAB_Status)
{
Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
memset(p_msg,0,sizeof(Ts_Sys_Msg));
p_msg->dest_id = RADIO_DAB_APP;
p_msg->msg_id = DAB_APP_DAB_DAB_STATUS_NOTIFYID;
UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_DAB_DAB_Status,sizeof(Te_DAB_Tuner_Ctrl_DAB_DAB_Status),&(p_msg->msg_length));
SYS_SEND_MSG(p_msg);
}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_FM_DAB_LinkingStop	                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_FM_DAB_LinkingStop(Te_FmtoDAB_Reqstatus e_FmtoDAB_Reqstatus)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_FM_DAB_STOP_LINKING;
	UpdateParameterIntoMessage(p_msg->data,&e_FmtoDAB_Reqstatus,sizeof(Te_FmtoDAB_Reqstatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Notify_SW_HW_Version	                   			 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Notify_SW_HW_Version(Ts_DabTunerMsg_R_GetVersion_Reply st_Vesrion_Number)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_APP;
	p_msg->msg_id = DAB_APP_VERSION_NOTIFYID;
	UpdateParameterIntoMessage(p_msg->data,&st_Vesrion_Number,sizeof(Ts_DabTunerMsg_R_GetVersion_Reply),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

void DAB_Tuner_Ctrl_Notify_Init_FMDAB_linking(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_Notmsg;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_INIT_FMDAB_LINKING_NOTIFYID;
	SYS_SEND_MSG(p_msg);	
}

/*=========================================================================================================================================================================*/
/* void DAB_Tuner_Ctrl_Response_SelectService(Te_RADIO_ReplyStatus e_SelectServiceReplyStatus,Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData)  */
/*=========================================================================================================================================================================*/
void DAB_Tuner_Ctrl_Notify_AutoScan_PlayStation(Te_RADIO_ReplyStatus e_SelectServiceReplyStatus, Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData)
{
	Ts_Sys_Msg *GetSelectServicereply = &DAB_Tuner_Ctrl_Notmsg;
	memset(GetSelectServicereply, 0, sizeof(Ts_Sys_Msg));

	GetSelectServicereply->src_id = RADIO_DAB_TUNER_CTRL;
	GetSelectServicereply->dest_id = RADIO_DAB_APP;
	GetSelectServicereply->msg_id = DAB_APP_AUTOSCAN_PLAY_STATION_NOTIFYID;
	GetSelectServicereply->msg_length = 0;
	UpdateParameterIntoMessage(GetSelectServicereply->data, &e_SelectServiceReplyStatus, sizeof(e_SelectServiceReplyStatus), &(GetSelectServicereply->msg_length));
	UpdateParameterIntoMessage(GetSelectServicereply->data, &st_currentEnsembleData, sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo), &(GetSelectServicereply->msg_length));

	SYS_SEND_MSG(GetSelectServicereply);

}
/*=============================================================================
    end of file
=============================================================================*/