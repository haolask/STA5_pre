/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_tuner_ctrl_request.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: DAB Tuner Control															   		*
*  Description			: This file contains function definitions of request API to DAB Tuner Control		*
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Request.h"

/*
#include "DAB_Tuner_Ctrl_Types.h"
#include "hsm_api.h"
#include "cfg_types.h"
#include "sys_task.h"
#include "DAB_Tuner_Ctrl_messages.h"
#include "DAB_Tuner_Ctrl_Request.h"
#include  <string.h>
*/
/*--------------------------------------------------------------------------------------------------
    variables (extern)
--------------------------------------------------------------------------------------------------*/
//Ts_Sys_Msg DAB_Tuner_Ctrl_retmsg;

Ts_Sys_Msg DAB_Tuner_Ctrl_retmsg1;

Ts_Sys_Msg* SYS_MSG_HANDLE_Call(Tu16 cid,Tu16 msgid)
{
	DAB_Tuner_Ctrl_retmsg1.dest_id =   cid;
	DAB_Tuner_Ctrl_retmsg1.msg_id =  msgid;
	return (&DAB_Tuner_Ctrl_retmsg1);
}


/*================================================================================================*/
/* void UpdateParameterIntoMessage(char *pu8_data,const void *vp_parameter, Tu8 u8_ParamLength, 
                                   Tu16 *pu16_Datalength)                                         */
/*================================================================================================*/
void UpdateParameterIntoMessage(char *pu8_data,const void *vp_parameter, Tu8 u8_ParamLength, Tu16 *pu16_Datalength)
{
	/* copying parameter to the data slots in Ts_Sys_Msg structure */
	SYS_RADIO_MEMCPY((pu8_data+*pu16_Datalength), vp_parameter,(size_t)( u8_ParamLength));

	/*  Updating msg_length for each parameter which represents length of data in  Ts_Sys_Msg structure   */
	*pu16_Datalength = (Tu16)(*pu16_Datalength + u8_ParamLength);
}

/*================================================================================================*/
/*void ExtractParameterFromMessage(void *vp_Parameter,const char *pu8_DataSlot, Tu8 u8_ParamLength, 
                                   Tu32 *pu32_index)                                              */
/*================================================================================================*/
void ExtractParameterFromMessage(void *vp_Parameter,const char *pu8_DataSlot, Tu8 u8_ParamLength, Tu32 *pu32_index)
{
	/* reading parameter from the data slot present in Ts_Sys_Msg structure  */
	SYS_RADIO_MEMCPY(vp_Parameter,pu8_DataSlot+*pu32_index,(size_t)(u8_ParamLength));

	/*  Updating index inorder to point to next parameter present in the data slot in  Ts_Sys_Msg structure   */
	*pu32_index= *pu32_index + u8_ParamLength;
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_Startup()                                                          */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_Startup(Te_DAB_Tuner_Market    e_Market, Tu8 u8_SettingStatus, Tu8 u8_StartType)
{
  Ts_Sys_Msg* p_msg = NULL;
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_STARTUP_REQID);
  UpdateParameterIntoMessage(p_msg->data,&e_Market,sizeof(Te_DAB_Tuner_Market),&(p_msg->msg_length));
  UpdateParameterIntoMessage(p_msg->data,&u8_SettingStatus,sizeof(Tu8),&(p_msg->msg_length));
  UpdateParameterIntoMessage(p_msg->data,&u8_StartType,sizeof(Tu8),&(p_msg->msg_length));
  SYS_SEND_MSG(p_msg);
}

void DAB_Tuner_Ctrl_Request_Activate_Deactivate(Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus e_DAB_Tuner_Ctrl_ActivateDeActivateStauts)
{
	Ts_Sys_Msg* p_msg = NULL;
	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_ACTIAVTE_DEACTIVATE_REQID);
	p_msg->msg_length = 0;
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_ActivateDeActivateStauts,sizeof(Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);	
	
}
/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_Scan(Te_Tuner_Ctrl_Bool Direction)                                 */
/*================================================================================================*/


void DAB_Tuner_Ctrl_Request_Scan(Tbool b_Direction)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));

	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_SCAN_REQID);

	UpdateParameterIntoMessage(p_msg->data,&b_Direction,sizeof(b_Direction),&(p_msg->msg_length));

	SYS_SEND_MSG(p_msg);

}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_Shutdown()                                                         */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_Shutdown(void)
{

	Ts_Sys_Msg* p_msg = NULL;
	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_SHUTDOWN_REQID);
    SYS_SEND_MSG(p_msg);


}


/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_SelectService(Tu32 u32_Frequency,Tu16 u16_Eid,Tu32 u32_Sid,Tu16 u16_SCid) */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_SelectService(Tu32 u32_Frequency,Tu16 u16_Eid,Tu32 u32_Sid,Tu16 u16_SCid)
{

  Ts_Sys_Msg* p_msg = NULL;
 
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_SELSERV_REQID);
  p_msg->msg_length = 0;
 
  UpdateParameterIntoMessage(p_msg->data,&u32_Frequency,sizeof(u32_Frequency),&(p_msg->msg_length));
  UpdateParameterIntoMessage(p_msg->data,&u16_Eid,sizeof(u16_Eid),&(p_msg->msg_length));
  UpdateParameterIntoMessage(p_msg->data,&u32_Sid,sizeof(u32_Sid),&(p_msg->msg_length));
  UpdateParameterIntoMessage(p_msg->data,&u16_SCid,sizeof(u16_SCid),&(p_msg->msg_length));
  SYS_SEND_MSG(p_msg);


}

/*================================================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_AutoSeekUpDown(Tu32 u32_Frequency, Te_RADIO_DirectionType e_SeekDirection, Tbool b_SeekStarted)    */
/*================================================================================================================================*/
void DAB_Tuner_Ctrl_Request_AutoSeekUpDown(Tu32 u32_Frequency, Te_RADIO_DirectionType e_SeekDirection, Tbool b_SeekStarted)
{
	Ts_Sys_Msg* p_msg = NULL;

	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SEEK_REQID);
	p_msg->msg_length = 0;

	UpdateParameterIntoMessage(p_msg->data, &u32_Frequency, sizeof(Tu32), &(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data, &e_SeekDirection, sizeof(Te_RADIO_DirectionType), &(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data, &b_SeekStarted, sizeof(Tbool), &(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

void DAB_Tuner_Ctrl_Request_Cancel(Te_DAB_Tuner_Ctrl_CancelType e_CancelType)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
 	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_CANCEL_REQID);
	UpdateParameterIntoMessage(p_msg->data,&e_CancelType,sizeof(Te_DAB_Tuner_Ctrl_CancelType),&(p_msg->msg_length));
    SYS_SEND_MSG(p_msg);
}


/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_Activate()                                                         */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_Activate(void)
{

  Ts_Sys_Msg* p_msg = NULL;
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_ACTIVATE_REQID);
  SYS_SEND_MSG(p_msg);

}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_DeActivate()                                                       */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_DeActivate(void)
{

 	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
 
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_DESELBAND_REQID);
	SYS_SEND_MSG(p_msg);


}

/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Request_EnableDABtoFMLinking                         		 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Request_EnableDABtoFMLinking(Te_DAB_Tuner_DABFMLinking_Switch e_status)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
 
	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_DAB_FM_LINKING_ENABLE_REQID);
	UpdateParameterIntoMessage(p_msg->data,&e_status,sizeof(Te_DAB_Tuner_DABFMLinking_Switch),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State                                                         */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State()
{

  Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  memset(p_msg,0,sizeof(Ts_Sys_Msg));
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,STATUS_CHECKING_MSG);
  SYS_SEND_MSG(p_msg);
}

void DAB_Tuner_Ctrl_Request_GetSIDStation(Tu16 u16_FM_DAB_PI, Tu16 u16_FM_DAB_SCID)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_FM_DAB_PI);
	UpdateParameterIntoMessage(p_msg->data,&u16_FM_DAB_PI,sizeof(Tu16),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u16_FM_DAB_SCID,sizeof(Tu16),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);

}




/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State                                                         */
/*================================================================================================*/
void DAB_TUNER_CTRL_Internal_Msg(void)
{

  Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  memset(p_msg,0,sizeof(Ts_Sys_Msg));
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,START_BACKGROUND_SCAN);
  SYS_SEND_MSG(p_msg);
}


/*===========================================================================*/
/*  void DAB_Tuner_Ctrl_Request_StartAnnouncement                         		 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Request_StartAnnouncement(Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type,Tu8 u8_Subchannelid)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
 	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_ANNOUNCEMENT_START_SWITCHING_REQID);
	UpdateParameterIntoMessage(p_msg->data,&e_announcement_type,sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&u8_Subchannelid,sizeof(Tu8),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
	
}

/*===========================================================================*/
/*  void DAB_App_Request_StopAnnouncement                         		 */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Request_StopAnnouncement(Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type,Tu8 SubChId)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
 	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_ANNOUNCEMENT_STOP_SWITCHING_REQID);
	UpdateParameterIntoMessage(p_msg->data,&e_announcement_type,sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&(p_msg->msg_length));
	UpdateParameterIntoMessage(p_msg->data,&SubChId,sizeof(Tu8),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}


/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Internal_Msg_To_Anno_State                                                         */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Internal_Msg_To_Anno_State(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  	memset(p_msg,0,sizeof(Ts_Sys_Msg));
 	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_INTERNAL_ANNO_MSG);
	SYS_SEND_MSG(p_msg);
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_CancelAnnouncement                                                      */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_CancelAnnouncement(Tu8 u8_DiffChannelSubChId, Tu8 u8_DiffChannelClusterid)
{
  Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  memset(p_msg,0,sizeof(Ts_Sys_Msg));
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_CANCEL_ANNOUNCEMENT_REQID);
  UpdateParameterIntoMessage(p_msg->data,&u8_DiffChannelSubChId,sizeof(Tu8),&(p_msg->msg_length));
  UpdateParameterIntoMessage(p_msg->data,&u8_DiffChannelClusterid,sizeof(Tu8),&(p_msg->msg_length));
  SYS_SEND_MSG(p_msg);
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_SetAnnoConfig        */                                             
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_SetAnnoConfig(Tu16 u16_AnnoConfig)
{

  Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
  memset(p_msg,0,sizeof(Ts_Sys_Msg));
  p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_ANNO_CONFIG_REQID); 
  UpdateParameterIntoMessage(p_msg->data,&u16_AnnoConfig,sizeof(Tu16),&(p_msg->msg_length));
  SYS_SEND_MSG(p_msg);
}

void DAB_Tuner_Ctrl_Enable_Fig_Notifications(void)
{
	DabTuner_SetFIG_filter_command((Tu16)0u,(Tu8)0x01,(Tu8)0x11);
	//SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
	DabTuner_SetFIG_filter_command((Tu16)6u,(Tu8)0x03,(Tu8)0x31);
	DabTuner_SetFIG_filter_command(0x0012,(Tu8)0x04,(Tu8)0x41);
	DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x51);
	DabTuner_SetFIG0_21_filter_command(0x0015,(Tu8)0x06,(Tu8)0x61);
	DabTuner_SetFIG_filter_command(0x0018,(Tu8)0x07,(Tu8)0x71);
}

void DAB_Tuner_Ctrl_Enable_Quality_Notifications(void)
{
	DabTuner_SetSynchronisationNotifier_cmd(0x01);
	DabTuner_MsgSndSetSNRNotifierSettings_cmd(0x01);
	DabTuner_MsgSndSetRSSINotifierSettings_cmd(0x01);
	DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(0x03);
	//DabTunerCtrl_SetServListChangedNotifier_cmd((Tu8) 0x01);
  	DabTuner_SetAudioErrorConcealment2_cmd();
}

void DAB_Tuner_Ctrl_Disable_Fig_Notifications(void)
{
	DabTuner_SetFIG_filter_command((Tu16)0u,(Tu8)0x01,(Tu8)0x10);
	//SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x20);
	DabTuner_SetFIG_filter_command((Tu16)6u,(Tu8)0x03,(Tu8)0x30);
	DabTuner_SetFIG_filter_command(0x0012,(Tu8)0x04,(Tu8)0x40);
	DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x50);
	DabTuner_SetFIG0_21_filter_command(0x0015,(Tu8)0x06,(Tu8)0x60);
	DabTuner_SetFIG_filter_command(0x0018,(Tu8)0x07,(Tu8)0x70);
}

void DAB_Tuner_Ctrl_Disable_Quality_Notifications(void)
{
	DabTuner_SetSynchronisationNotifier_cmd(0x00);
	DabTuner_MsgSndSetSNRNotifierSettings_cmd(0x00);
	DabTuner_MsgSndSetRSSINotifierSettings_cmd(0x00);
	DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(0x00);
	//DabTunerCtrl_SetServListChangedNotifier_cmd((Tu8) 0x00);
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Request_DABTUNERRestart        */                                             
/*================================================================================================*/
void DAB_Tuner_Ctrl_Request_DABTUNERRestart(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = DAB_TUNER_CTRL_DABTUNER_RESTART_REQID;
	SYS_SEND_MSG(p_msg);
}
/*===============================================================================*/
/*  void DAB_Tuner_Ctrl_Request_ENG_Mode						*/ 
/*===============================================================================*/
void DAB_Tuner_Ctrl_Request_ENG_Mode(Te_DAB_Tuner_Ctrl_Eng_Mode_Request e_DAB_Tuner_Ctrl_Eng_Mode_Request) 
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_ENG_MODE_REQID);
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_Eng_Mode_Request,sizeof(Te_DAB_Tuner_Ctrl_Eng_Mode_Request),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);
}


void DAB_Tuner_Ctrl_RDS_Settings_Request(Te_DAB_Tuner_Ctrl_DAB_AF_Settings	e_DAB_Tuner_Ctrl_DAB_AF_Settings)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_DAB_AF_SETTINGS_REQID);
	UpdateParameterIntoMessage(p_msg->data,&e_DAB_Tuner_Ctrl_DAB_AF_Settings,sizeof(Te_DAB_Tuner_Ctrl_DAB_AF_Settings),&(p_msg->msg_length));
	SYS_SEND_MSG(p_msg);	
}


void DAB_Tuner_Ctrl_FactoryReset_Request(void)
{
	Ts_Sys_Msg* p_msg = &DAB_Tuner_Ctrl_retmsg1;
	memset(p_msg,0,sizeof(Ts_Sys_Msg));
	p_msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_DAB_FACTORY_RESET_REQID);
	SYS_SEND_MSG(p_msg);	
	
}
/*=============================================================================
    end of file
=============================================================================*/
