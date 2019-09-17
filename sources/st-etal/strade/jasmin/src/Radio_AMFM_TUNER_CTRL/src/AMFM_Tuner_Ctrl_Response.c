/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Response.c                                                                        *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     * 
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: SC_AMFM_TUNER_CTRL                                                                *
*  Description			: AMFM tuner control Response api's definitions     .                               *
*                                                                                                           *
*                                                                                                           *
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "AMFM_Tuner_Ctrl_Response.h"
#include "amfm_app_msg_id.h"
#include "AMFM_Tuner_Ctrl_App.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables 
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/
	Ts_Sys_Msg st_resmsg;
/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_Startup									 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Response_Startup(Te_RADIO_ReplyStatus e_StartupReplyStatus)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_STARTUP_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_StartupReplyStatus,(Tu16)(sizeof(e_StartupReplyStatus)),&(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_FactoryReset									 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Response_FactoryReset(Te_RADIO_ReplyStatus e_FactoryresetReplyStatus)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_FACTORY_RESET_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_FactoryresetReplyStatus,(Tu16)(sizeof(e_FactoryresetReplyStatus)),&(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}
/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_GetStationList									 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Response_GetStationList(Te_RADIO_ReplyStatus e_GetStationListReplyStatus)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_GET_STL_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_GetStationListReplyStatus,(Tu16)(sizeof(e_GetStationListReplyStatus)),&(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_Activate									 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Response_Activate(Te_RADIO_ReplyStatus e_ActivateReplyStatus)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_SELECT_BAND_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_ActivateReplyStatus,(Tu16)(sizeof(e_ActivateReplyStatus)),&(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_Tune									     */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Response_Tune(Te_RADIO_ReplyStatus e_TuneReplyStatus, Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStation_Info)
{
	
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_SELECT_STATION_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_TuneReplyStatus, (Tu16)(sizeof(e_TuneReplyStatus)), &(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&st_CurrentStation_Info, (Tu16)(sizeof(st_CurrentStation_Info)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_SeekUpDown								 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Response_SeekUpDown(Te_RADIO_ReplyStatus e_SeekReplyStatus, Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStation_Info)
{
	
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_SEEK_UP_DOWN_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_SeekReplyStatus, (Tu16)(sizeof(e_SeekReplyStatus)), &(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&st_CurrentStation_Info, (Tu16)(sizeof(st_CurrentStation_Info)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_Activate									 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Response_DeActivate(Te_RADIO_ReplyStatus e_DeActivateReplyStatus)
{
	
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_DESELECT_BAND_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_DeActivateReplyStatus,(Tu16)(sizeof(e_DeActivateReplyStatus)),&(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}


/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Startup									 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_SHUTDOWN_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_ShutdownReplyStatus,(Tu16)(sizeof(e_ShutdownReplyStatus)),&(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

void AMFM_Tuner_Ctrl_Response_Cancel(Te_RADIO_ReplyStatus e_CancelReplyStatus, Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrStationInfo)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_CANCEL_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_CancelReplyStatus,(Tu16)(sizeof(e_CancelReplyStatus)),&(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data), &st_CurrStationInfo, (Tu16)(sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

void AMFM_Tuner_Ctrl_Response_Announcement_Cancel(Te_RADIO_ReplyStatus e_CancelReplyStatus)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_ANNO_CANCEL_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_CancelReplyStatus,(Tu16)(sizeof(e_CancelReplyStatus)),&(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}
/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_Tune									     */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Response_AF_Update(Te_RADIO_ReplyStatus e_AF_UpdateReplyStatus,Tu32 u32_AFupdateFreq,Ts_AMFM_Tuner_Ctrl_Interpolation_info st_AF_Update_StationInfo)
{
	
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_AFFREQ_UPDATE_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_AF_UpdateReplyStatus, (Tu16)(sizeof(e_AF_UpdateReplyStatus)), &(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&u32_AFupdateFreq, (Tu16)(sizeof(Tu32)), &(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&st_AF_Update_StationInfo, (Tu16)(sizeof(st_AF_Update_StationInfo)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}
/*===========================================================================*/

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_FM_Check									     */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Response_FM_Check(Te_RADIO_ReplyStatus e_AF_CheckReplyStatus, Tu16 u16_pi)
{
	
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_AFFREQ_CHECK_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_AF_CheckReplyStatus, (Tu16)(sizeof(e_AF_CheckReplyStatus)), &(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&u16_pi, (Tu16)(sizeof(u16_pi)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Response_LowSignal_FM_Check									     */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Response_LowSignal_FM_Check(Te_RADIO_ReplyStatus e_AF_CheckReplyStatus, Tu16 u16_pi)
{
	
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_AF_LOW_SIGNAL_CHECK_DONE_RESID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&e_AF_CheckReplyStatus, (Tu16)(sizeof(e_AF_CheckReplyStatus)), &(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&u16_pi, (Tu16)(sizeof(u16_pi)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}
void AMFM_Tuner_Ctrl_Send_EONTA(Tu16 u16_EON_PI)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_EON_ANNOUNCEMENT_START_NOTIFIED);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&u16_EON_PI, (Tu16)(sizeof(u16_EON_PI)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

void AMFM_Tuner_Ctrl_EON_TAdata(Ts_AMFM_TunerCtrl_EON_Info st_EON_StationInfo)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_EON_INFO_NOTIFIED);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&st_EON_StationInfo, (Tu16)(sizeof(st_EON_StationInfo)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
}

void AMFM_Tuner_Ctrl_Send_TAStatus(Tu8 u8_TA,Tu8 u8_TP)
{
	Ts_Sys_Msg* pst_resmsg = &st_resmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_resmsg ,RADIO_AM_FM_APP,AMFM_APP_CURRENT_STATION_TA_TP_NOTIFIED);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&u8_TA, (Tu16)(sizeof(u8_TA)), &(pst_resmsg->msg_length));
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_resmsg->data),&u8_TP, (Tu16)(sizeof(u8_TP)), &(pst_resmsg->msg_length));
	SYS_SEND_MSG(pst_resmsg);
	
}
