
/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Msg.c                                                                             *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     * 
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: SC_AMFM_TUNER_CTRL                                                                *
*  Description			: This source file consists of function definitions of all Notification APIs		*
*						  which send messages to AMFM Application                                           *
*                                                                                                           *
*                                                                                                           *
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "AMFM_Tuner_Ctrl_Notify.h"
#include "amfm_app_msg_id.h"
#include "AMFM_HAL_Interface.h"
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
	Ts_Sys_Msg st_notifymsg;
/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_Notify_STLUpdated  					             */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_SharedMemoryType)
{
	Ts_Sys_Msg* pst_notifymsg = &st_notifymsg;
	AMFM_Tuner_Ctrl_MessageInit(pst_notifymsg ,RADIO_AM_FM_APP,AMFM_APP_STL_UPDATED_NOTIFYID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage(pst_notifymsg->data,&e_SharedMemoryType,sizeof(e_SharedMemoryType),&(pst_notifymsg->msg_length));
	 SYS_SEND_MSG(pst_notifymsg);
}

/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_NotifyTunerStatus  					             */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_NotifyTunerStatus(Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_Current_StationInfo)
{
	
	Ts_Sys_Msg* pst_notifymsg = &st_notifymsg;
	AMFM_Tuner_Ctrl_MessageInit(pst_notifymsg ,RADIO_AM_FM_APP,AMFM_APP_TUNER_STATUS_NOTIFYID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_notifymsg->data), &st_Current_StationInfo, (Tu16)(sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo)), &(pst_notifymsg->msg_length));
	 SYS_SEND_MSG(pst_notifymsg);
}
/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_Notify_CurrFrequency  					             */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Notify_CurrFrequency(Tu32 u32_freq)
{
	Ts_Sys_Msg* pst_notifymsg = &st_notifymsg;
	AMFM_Tuner_Ctrl_MessageInit(pst_notifymsg ,RADIO_AM_FM_APP,AMFM_APP_CURR_FREQUENCY_NOTIFYID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_notifymsg->data),&u32_freq, (Tu16)(sizeof(u32_freq)), &(pst_notifymsg->msg_length));
	 SYS_SEND_MSG(pst_notifymsg);
}

/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_NotifyFS           					             */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_NotifyFS(Tu8 u8_FS)
{
	Ts_Sys_Msg* pst_notifymsg = &st_notifymsg;
	AMFM_Tuner_Ctrl_MessageInit(pst_notifymsg ,RADIO_AM_FM_APP,AMFM_APP_TUNER_STATUS_NOTIFYID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_notifymsg->data),&u8_FS, (Tu16)(sizeof(u8_FS)), &(pst_notifymsg->msg_length));
	 SYS_SEND_MSG(pst_notifymsg);
}
/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_Notify_CurrQual  					             */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Notify_CurrQual(Ts_AMFM_Tuner_Ctrl_Interpolation_info Tuner_Qual)
{
	Ts_Sys_Msg* pst_notifymsg = &st_notifymsg;
	AMFM_Tuner_Ctrl_MessageInit(pst_notifymsg ,RADIO_AM_FM_APP,AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_notifymsg->data),&Tuner_Qual, (Tu16)(sizeof(Tuner_Qual)), &(pst_notifymsg->msg_length));
	 SYS_SEND_MSG(pst_notifymsg);
}

/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_Notify_SOC_Abnormal  					             */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Notify_AMFMTuner_Abnormal(Te_RADIO_AMFMTuner_Status e_AMFMTunerErrType)
{
	Ts_Sys_Msg* pst_notifymsg = &st_notifymsg;
	AMFM_Tuner_Ctrl_MessageInit(pst_notifymsg ,RADIO_AM_FM_APP,AMFM_APP_TUNER_AMFMTUNER_ABNORMAL_NOTIFYID);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_notifymsg->data),&e_AMFMTunerErrType, (Tu16)(sizeof(e_AMFMTunerErrType)), &(pst_notifymsg->msg_length));
	SYS_SEND_MSG(pst_notifymsg);
}
