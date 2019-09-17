/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app_request.c																	  				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains request API's for DAB Application.								*
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_hsm.h"
#include "dab_app_request.h"
#include "msg_cmn.h"


/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/
Ts_Sys_Msg msg ;


/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_App_Request_Startup                         					 */
/*===========================================================================*/
void DAB_App_Request_Startup(Te_DAB_App_Market    e_Market, Tu8 u8_SettingStatus, Tu8 u8_StartType)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_STARTUP_REQID,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Market,(Tu8)sizeof(Te_DAB_App_Market),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&u8_SettingStatus,(Tu8)sizeof(Tu8),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&u8_StartType,(Tu8)sizeof(Tu8),&msg.msg_length);

#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Request_GetStationList                         			 */
/*===========================================================================*/
void DAB_App_Request_GetStationList(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_GETSTL_REQID,RADIO_MNGR_APP);
	
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Request_SelectBand                         				 */
/*===========================================================================*/
void DAB_App_Request_SelectBand(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_SELECT_DAB_REQID,RADIO_MNGR_APP);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Request_DeSelectBand                         				 */
/*===========================================================================*/
void DAB_App_Request_DeSelectBand(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_DESELECT_DAB_REQID,RADIO_MNGR_APP);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif

}

void DAB_App_Request_Activate_Deactivate(Te_DAB_App_ActivateDeactivateStatus e_ActivateDeactivateStatus)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_ACTIVATE_DEACTIVATE_REQID,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&e_ActivateDeactivateStatus,(Tu8)sizeof(Te_DAB_App_ActivateDeactivateStatus),&msg.msg_length);
	Sys_Send_Msg(&msg);
	
}
/*===========================================================================*/
/*  void DAB_App_Request_PlaySelectSt                         				 */
/*===========================================================================*/
void DAB_App_Request_PlaySelectSt(Tu32 u32_Freq, Tu16 u16_EId, Tu32 u32_Sid, Tu16 u16_SCIdI)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_PLAY_SEL_STN_REQID,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&u32_Freq,(Tu8)sizeof(Tu32),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&u16_EId,(Tu8)sizeof(Tu16),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&u32_Sid,(Tu8)sizeof(Tu32),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&u16_SCIdI,(Tu8)sizeof(Tu16),&msg.msg_length);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif

}

/*===========================================================================*/
/*  void DAB_App_Request_Shutdown                         					 */
/*===========================================================================*/
void DAB_App_Request_Shutdown(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_SHUTDOWN_REQID,RADIO_MNGR_APP);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_Inst_App_Request_Startup                      					 */
/*===========================================================================*/
void DAB_Inst_App_Request_Startup(void)
{
	MessageHeaderWrite(&msg,DAB_APP_INST_HSM_CID,DAB_APP_INST_HSM_STARTUP,RADIO_DAB_APP);
	DAB_APP_INST_HSM_HandleMessage(&msg);
}

/*===========================================================================*/
/*  void DAB_App_Request_ServiceCompSeekUpDown                         		 */
/*===========================================================================*/

void DAB_App_Request_ServiceCompSeekUpDown(Tu32 u32_Frequency, Te_RADIO_DirectionType e_Direction)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_SER_COMP_SEEK_REQID,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&u32_Frequency,(Tu8)sizeof(Tu32),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Direction,(Tu8)sizeof(Te_RADIO_DirectionType),&msg.msg_length);

#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif

}

/*===========================================================================*/
/*  void DAB_App_Request_Cancel                         		 */
/*===========================================================================*/

void DAB_App_Request_Cancel(Te_DAB_App_CancelType e_CancelType)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_CANCEL_REQID,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&e_CancelType,(Tu8)sizeof(Te_DAB_App_CancelType),&msg.msg_length);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif

}

/*===========================================================================*/
/*  void DAB_Inst_App_Request_Shutdown                     					 */
/*===========================================================================*/
void DAB_Inst_App_Request_Shutdown(void)
{
	MessageHeaderWrite(&msg,DAB_APP_INST_HSM_CID,DAB_APP_INST_HSM_SHUTDOWN,RADIO_DAB_APP);
	DAB_APP_INST_HSM_HandleMessage(&msg);
}


/*===============================================================================*/
/*  void DAB_App_Request_TuneUpDown(Te_RADIO_DirectionType e_Direction) */
/*===============================================================================*/
void DAB_App_Request_TuneUpDown(Te_RADIO_DirectionType e_Direction)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_TUNEUPDOWN_REQID,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&e_Direction,(Tu8)sizeof(Te_RADIO_DirectionType),&msg.msg_length);

#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Request_EnableDABtoFMLinking                         		 */
/*===========================================================================*/
void DAB_App_Request_EnableDABtoFMLinking(Te_DAB_App_DABFMLinking_Switch e_LinkingStatus)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_DAB_FM_LINKING_ENABLE_REQID,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&e_LinkingStatus,(Tu8)sizeof(Te_DAB_App_DABFMLinking_Switch),&msg.msg_length);

#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}
/*================================================================================================*/
/* void DAB_App_Request_GetSIDStation                                                         */
/*================================================================================================*/
void DAB_App_Request_GetSIDStation(Tu16 u16_FM_PI)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_FM_DAB_LINKING_PI,RADIO_MNGR_APP);
	UpdateParameterInMessage((Tu8*)msg.data,&u16_FM_PI,(Tu8)sizeof(Tu16),&msg.msg_length);

#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif	
	
	
	
}

/*================================================================================================*/
/* void DAB_App_Internal_Msg_To_Anno_State                                                         */
/*================================================================================================*/
void DAB_App_Internal_Msg_To_Anno_State(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_INTERNAL_ANNO_MSG,RADIO_DAB_APP);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
} 


/*================================================================================================*/
/* void DAB_App_CancelAnnouncement                                                         */
/*================================================================================================*/
void DAB_App_Request_AnnoCancel(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_CANCEL_ANNO_REQID,RADIO_MNGR_APP);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
} 

/*===============================================================================*/
/*  void DAB_App_Request_SetAnnoConfig(Tu16 u16_AnnoConfig)						*/ 
/*===============================================================================*/
void DAB_App_Request_SetAnnoConfig(Tu16 u16_AnnoConfig) 
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_ANNO_CONFIG_REQID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,&u16_AnnoConfig,(Tu8)sizeof(Tu16),&msg.msg_length);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*===============================================================================*/
/*  void DAB_App_Request_AFTune(Tu32 u32_Sid, Tu16 u16_SCId)					 */ 
/*===============================================================================*/

void DAB_App_Request_AFTune(Tu16 u16_Sid, Tu16 u16_SCId)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_AF_TUNE_REQID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,&u16_Sid,(Tu8)sizeof(Tu16),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&u16_SCId,(Tu8)sizeof(Tu16),&msg.msg_length);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*================================================================================================*/
/* void DAB_App_Request_DABTunerRestart      														  */                                             
/*================================================================================================*/
void DAB_App_Request_DABTunerRestart(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_DABTUNER_RESTART_REQID,RADIO_MNGR_APP); 
	Sys_Send_Msg(&msg);
}

/*===============================================================================*/
/*  void DAB_App_Request_ENG_Mode						*/ 
/*===============================================================================*/
void DAB_App_Request_ENG_Mode(Te_DAB_APP_Eng_Mode_Request e_DAB_APP_Eng_Mode_Request) 
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_ENG_MODE_REQID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,&e_DAB_APP_Eng_Mode_Request,(Tu8)sizeof(Te_DAB_APP_Eng_Mode_Request),&msg.msg_length);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}
/*===============================================================================*/
/* void DAB_App_Request_sort(void) 						*/ 
/*===============================================================================*/
void DAB_App_Request_sort(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_COMP_LIST_SORT_REQID,RADIO_DAB_TUNER_CTRL);
	Sys_Send_Msg(&msg);
}

/*===============================================================================*/
/*  void DAB_App_Request_AFSwitch settings enable disable												*/ 
/*===============================================================================*/
void DAB_App_Request_DAB_AF_Settings(Te_DAB_App_AF_Switch e_DAB_App_AF_Switch)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_DAB_AF_SETTINGS_REQID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,&e_DAB_App_AF_Switch,(Tu8)sizeof(Te_DAB_App_AF_Switch),&msg.msg_length);
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

void DAB_App_Request_FactoryReset(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_FACTORY_RESET_REQID,RADIO_MNGR_APP); 

#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif

}


void DAB_App_Request_TuneByChannelName(Tu8 *au8_DABChannelName)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_MANUAL_TUNEBY_CHNAME_REQID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,au8_DABChannelName,(Tu8)(sizeof(Tu8) * DAB_APP_MAX_CHANNEL_NAME_SIZE),&msg.msg_length);

#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif
}

/*=============================================================================
    end of file
=============================================================================*/