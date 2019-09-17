/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_response.c																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This source file consists of function defintions of all Response APIs				*
*						  which will be	provided to Radio Manager Application 								*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "amfm_app_response.h"

/*-----------------------------------------------------------------------------
     Global variable Defintion 
-----------------------------------------------------------------------------*/
Te_AMFM_App_mode	e_CurrentMode;			/* Used to check whether AM/FM station list to be generated  */
Ts_Sys_Msg			st_amfm_app_res_msg;	/* used to send any response message to Radio Manager Application component */

/*-----------------------------------------------------------------------------
    Public function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*			void  AMFM_App_Response_Startup 							     */
/*===========================================================================*/
void AMFM_App_Response_Startup(Te_RADIO_ReplyStatus e_StartupReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_STARTUP_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_StartupReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_StartupReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);
	
	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);


}

/*===========================================================================*/
/*			void  AMFM_APP_Response_Shutdown							     */
/*===========================================================================*/
void AMFM_APP_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_SHUTDOWN_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_ShutdownReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_ShutdownReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);
	
	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);

}


/*===========================================================================*/
/*			void  AMFM_App_Response_GetStationList						     */
/*===========================================================================*/
void AMFM_App_Response_GetStationList(Te_RADIO_ReplyStatus e_GetStationListReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */

	if(e_CurrentMode == AMFM_APP_MODE_AM)
	{
		/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
		AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AM_STATIONLIST_DONE_RESID,RADIO_AM_FM_APP);
	}
	else if(e_CurrentMode == AMFM_APP_MODE_FM) 
	{
		/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
		AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_FM_STATIONLIST_DONE_RESID,RADIO_AM_FM_APP);
	}
	else
	{
			// Need to send error if Mode is other than AM or FM 
	}

	/* Updating e_GetStationListReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_GetStationListReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);
	
	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);

}


/*===========================================================================*/
/*			void  AMFM_App_Response_SelectBand							     */
/*===========================================================================*/
void AMFM_App_Response_SelectBand(Te_RADIO_ReplyStatus e_SelectBandReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_SELECTBAND_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_SelectBandReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_SelectBandReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);
	
	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}


/*===========================================================================*/
/*			void  AMFM_App_Response_DeSelectBand						     */
/*===========================================================================*/
void AMFM_App_Response_DeSelectBand(Te_RADIO_ReplyStatus e_DeSelectBandReplyStatus)
{
	
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_DESELECTBAND_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_DeSelectBandReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_DeSelectBandReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);

}

/*===========================================================================*/
/*			void  AMFM_App_Response_PlaySelectSt						     */
/*===========================================================================*/
void AMFM_App_Response_PlaySelectSt(Te_RADIO_ReplyStatus e_PlaySelectStationReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_PLAY_SELECT_STATION_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_PlaySelectStationReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_PlaySelectStationReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&st_CurrentStationInfo,(Tu8)sizeof(Ts_AMFM_App_StationInfo),&pst_amfm_app_res_msg->msg_length);
	
	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Response_SeekUpDown							     */
/*===========================================================================*/
void AMFM_App_Response_SeekUpDown(Te_RADIO_ReplyStatus e_SeekReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_SEEKUPDOWN_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_SeekReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_SeekReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&st_CurrentStationInfo,(Tu8)sizeof(Ts_AMFM_App_StationInfo),&pst_amfm_app_res_msg->msg_length);

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Response_CancelDone							     */
/*===========================================================================*/
void AMFM_App_Response_CancelDone(Te_RADIO_ReplyStatus e_CancelReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_CancelReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_CancelReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Response_TuneUpDown							     */
/*===========================================================================*/
void AMFM_App_Response_TuneUpDown(Te_RADIO_ReplyStatus e_TuneUpDownReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_TUNEUPDOWN_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_TuneUpDownReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_TuneUpDownReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&pst_amfm_app_res_msg->msg_length);

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&st_CurrentStationInfo,(Tu8)sizeof(Ts_AMFM_App_StationInfo),&pst_amfm_app_res_msg->msg_length);

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Response_AFSwitch							     */
/*===========================================================================*/
void AMFM_App_Response_AFSwitch(Te_RADIO_ReplyStatus e_AMFM_APP_AFSwitchReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_APP_AF_SWITCH_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_AMFM_APP_AFSwitchReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_AMFM_APP_AFSwitchReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&(pst_amfm_app_res_msg->msg_length));

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);


}
/*===========================================================================*/
/*			void  AMFM_App_Response_TA_Switch	   							 */
/*===========================================================================*/
void AMFM_App_Response_TA_Switch(Te_RADIO_ReplyStatus e_TASwitchReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_TA_SWITCH_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_TASwitchReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_TASwitchReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&(pst_amfm_app_res_msg->msg_length));

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Response_Anno_Cancel_Done						 */
/*===========================================================================*/
void AMFM_App_Response_Anno_Cancel_Done(Te_RADIO_ReplyStatus e_AMFM_APP_Anno_CancelReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_ANNO_CANCEL_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_FM_to_DAB_SwitchReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_AMFM_APP_Anno_CancelReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&(pst_amfm_app_res_msg->msg_length));

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);	

	
}
/*===========================================================================*/
/*			void  AMFM_App_Response_FM_to_DAB_Switch					     */
/*===========================================================================*/
void AMFM_App_Response_FM_to_DAB_Switch(Te_RADIO_ReplyStatus e_FM_to_DAB_SwitchReplyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_FM_DAB_SWITCH_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_FM_to_DAB_SwitchReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_FM_to_DAB_SwitchReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&(pst_amfm_app_res_msg->msg_length));

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Response_AFTune								     */
/*===========================================================================*/
void AMFM_App_Response_AFTune(Te_RADIO_ReplyStatus e_AFTune_ReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo)
{	
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_AF_TUNE_DONE_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_AFTune_ReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_AFTune_ReplyStatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&(pst_amfm_app_res_msg->msg_length));

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&st_CurrentStationInfo,(Tu8)sizeof(Ts_AMFM_App_StationInfo),&pst_amfm_app_res_msg->msg_length);

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Response_CT_Info								     */
/*===========================================================================*/
void AMFM_App_Response_CT_Info(Te_RADIO_ReplyStatus e_GetCT_InfoReplystatus,Ts_AMFM_App_CT_Info st_CT_Info)
{	
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_CT_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_AFTune_ReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_GetCT_InfoReplystatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&(pst_amfm_app_res_msg->msg_length));

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&st_CT_Info,(Tu8)sizeof(Ts_AMFM_App_CT_Info),&(pst_amfm_app_res_msg->msg_length));

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Response_FactoryReset						     */
/*===========================================================================*/
void AMFM_App_Response_FactoryReset(Te_AMFM_App_FactoryResetReplyStatus e_FactoryReset_ReplyStatus)
{	
	Ts_Sys_Msg		*pst_amfm_app_res_msg = &st_amfm_app_res_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_res_msg structure */
	AMFM_APP_MessageInit(pst_amfm_app_res_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_FACTORY_RESET_RESID,RADIO_AM_FM_APP);
	
	/* Updating e_AFTune_ReplyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_res_msg->data),&e_FactoryReset_ReplyStatus,(Tu8)sizeof(Te_AMFM_App_FactoryResetReplyStatus),&(pst_amfm_app_res_msg->msg_length));

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_res_msg);
}

