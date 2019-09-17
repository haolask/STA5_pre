/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app_response.c																  				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains response API's for DAB Application.								*
*																											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_hsm.h"
#include "dab_app_response.h"
#include "msg_cmn.h"
#include "radio_mngr_app.h"
#include "DAB_Tuner_Ctrl_Response.h"


/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_App_Response_Startup                         					 */
/*===========================================================================*/
void DAB_App_Response_Startup(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_STARTUP_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Replystatus,(Tu8)sizeof(e_Replystatus),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Response_GetStationList                   					 */
/*===========================================================================*/
void DAB_App_Response_GetStationList(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_STATIONLIST_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Response_SelectBand	                   					 */
/*===========================================================================*/
void DAB_App_Response_SelectBand(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_SELECTBAND_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Response_DeSelectBand	                   					 */
/*===========================================================================*/
void DAB_App_Response_DeSelectBand(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_DESELECTBAND_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Response_PlaySelectSt	                   					 */
/*===========================================================================*/
void DAB_App_Response_PlaySelectSt(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_PLAY_SELECT_STATION_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&st_CurrentStationInfo,(Tu8)sizeof(st_CurrentStationInfo),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Response_Shutdown		                   					 */
/*===========================================================================*/
void DAB_App_Response_Shutdown(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_SHUTDOWN_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}


/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_Inst_App_Response_Startup                    					 */
/*===========================================================================*/
void DAB_Inst_App_Response_Startup(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_INST_HSM_START_DONE,RADIO_DAB_APP);
	DAB_APP_MSG_HandleMsg(&msg);
}

/*===========================================================================*/
/*  void DAB_App_Response_ServiceCompSeekUpDown	                   			 */
/*===========================================================================*/
void DAB_App_Response_ServiceCompSeekUpDown(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_SEEK_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&st_CurrentStationInfo,(Tu8)sizeof(Ts_DAB_App_CurrentStationInfo),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif

}

/*===========================================================================*/
/*  void DAB_App_Response_Cancel	                   			 */
/*===========================================================================*/
void DAB_App_Response_Cancel(Te_RADIO_ReplyStatus e_Replystatus) 
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Replystatus,(Tu8)sizeof(Te_RADIO_ReplyStatus),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif

}

/*===========================================================================*/
/*  void DAB_Inst_App_Response_Shutdown                    					 */
/*===========================================================================*/
void DAB_Inst_App_Response_Shutdown(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_INST_HSM_SHUTDOWN_DONE,RADIO_DAB_APP);
	DAB_APP_MSG_HandleMsg(&msg);
}


/*=====================================================================================================================================================*/
/*  void DAB_App_Response_TuneUpDown(Te_RADIO_ReplyStatus e_TuneUpDownReplyStatus ,Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo)    */
/*=====================================================================================================================================================*/
void DAB_App_Response_TuneUpDown(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_TUNEUPDOWN_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&st_CurrentStationInfo,(Tu8)sizeof(Ts_DAB_App_CurrentStationInfo),&msg.msg_length);
	
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}


/*===========================================================================*/
/*  void DAB_App_Response_EnableDABtoFMLinking                   			 */
/*===========================================================================*/
void DAB_App_Response_EnableDABtoFMLinking(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DABTOFM_LINKING_ENABLE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}
/*===========================================================================================*/
/* void DAB_App_Response_CancelAnnouncement													 */
/*===========================================================================================*/
void DAB_App_Response_AnnoCancel(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_ANNO_CANCEL_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Response_SetAnnoConfig          					 */
/*===========================================================================*/
void DAB_App_Response_SetAnnoConfig (Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_ANNO_SWITCH_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}


void DAB_App_Response_AFtune(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_AF_TUNE_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, (Tu8)sizeof(e_Replystatus), &msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&st_CurrentStationInfo,(Tu8)sizeof(st_CurrentStationInfo),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

void DAB_App_Response_Activate_Deactivate(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_ACTIVATE_DEACTIVATE_DONE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Replystatus,(Tu8)sizeof(e_Replystatus),&msg.msg_length);
	Sys_Send_Msg(&msg);
}
/*=======================================================================================*/
/* void DAB_App_Response_DABTUNERRestart									                 */                      
/*=======================================================================================*/
void DAB_App_Response_DABTUNERRestart(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DABTUNER_RESTART_DONE_RESIID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &e_Replystatus, sizeof(e_Replystatus), &msg.msg_length);
	SYS_SEND_MSG(&msg);
}
/*===========================================================================*/
/*  void DAB_App_Response_AF_List		                   			 */
/*===========================================================================*/
void DAB_App_Response_AF_List(Ts_DAB_App_AFList st_DAB_App_AFList)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_AFLIST_UPDATE_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&st_DAB_App_AFList,sizeof(Ts_DAB_App_AFList),&msg.msg_length);
	Sys_Send_Msg(&msg);
}
/*===============================================================================*/
/* void DAB_App_Response_sort(void) 						*/ 
/*===============================================================================*/
void DAB_App_Response_Sort(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_COMP_LIST_SORT_RESID,RADIO_DAB_APP);
	Sys_Send_Msg(&msg);
}

/*===========================================================================*/
/*  void DAB_App_Response_DAB_AF_Settings		                   			 */
/*===========================================================================*/
void DAB_App_Response_DAB_AF_Settings(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_APP_RDS_SETTINGS_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Replystatus,sizeof(Te_RADIO_ReplyStatus),&msg.msg_length);
	Sys_Send_Msg(&msg);
}

/*===========================================================================*/
/*  void DAB_App_Response_FactoryReset		                   			 */
/*===========================================================================*/
void DAB_App_Response_FactoryReset(Te_RADIO_ReplyStatus e_Replystatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_FACTORY_RESET_RESID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Replystatus,sizeof(Te_RADIO_ReplyStatus),&msg.msg_length);
	Sys_Send_Msg(&msg);
}

/*=============================================================================
    end of file
=============================================================================*/