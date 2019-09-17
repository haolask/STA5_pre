/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app_notify.c																	  				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains notify API's for DAB Application.								*
*																											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_hsm.h"
#include "dab_app_notify.h"
#include "msg_cmn.h"
#include "radio_mngr_app.h"
#include "dab_app_extern.h"


/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_App_Notify_STLUpdated                         					 */
/*===========================================================================*/
void DAB_App_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_SharedMemoryType)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_STATIONLIST_UPDATE_DONE_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_SharedMemoryType,(Tu8)sizeof(e_SharedMemoryType),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Notify_FrequencyChange                         			 */
/*===========================================================================*/
void DAB_App_Notify_FrequencyChange(Tu32 u32_Frequency, Tu8 *pu8_ChannelName)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_FREQ_CHANGE_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&u32_Frequency,(Tu8)sizeof(u32_Frequency),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)pu8_ChannelName,(Tu8)(DAB_APP_MAX_CHANNEL_NAME_SIZE),&msg.msg_length);
	
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Notify_TunerStatus		                         			 */
/*===========================================================================*/
void DAB_App_Notify_TunerStatus(Ts_DAB_APP_Status_Notification st_TunerStatusNotify , Te_DAB_App_SignalStatus e_DAB_App_SignalStatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_TUNER_STATUS_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&st_TunerStatusNotify,(Tu8)sizeof(st_TunerStatusNotify),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_DAB_App_SignalStatus,(Tu8)sizeof(Te_DAB_App_SignalStatus),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}


/*===========================================================================*/
/*  void DAB_App_Notify_PICodeList                         					 */
/*===========================================================================*/
void DAB_App_Notify_PICodeList(Ts_DAB_PICodeList st_DAB_PICodeList, Tu8 u8_QualityMin, Tu8 u8_QualityMax , Tu32 Sid , Tu8 Linking_check)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_PICODE_LIST_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&st_DAB_PICodeList,(Tu8)sizeof(st_DAB_PICodeList),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&u8_QualityMin,(Tu8)sizeof(u8_QualityMin),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&u8_QualityMax,(Tu8)sizeof(u8_QualityMax),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&Sid,(Tu8)sizeof(Tu32),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&Linking_check,(Tu8)sizeof(Tu8),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Notify_BestPI                         					 	 */
/*===========================================================================*/
void DAB_App_Notify_BestPI(Tu16 PICode, Tu8 u8_Quality, Te_DAB_App_BestPI_Type e_BestPI_Type)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_BESTPI_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&PICode,(Tu8)sizeof(PICode),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&u8_Quality,(Tu8)sizeof(u8_Quality),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_BestPI_Type,(Tu8)sizeof(Te_DAB_App_BestPI_Type),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Notify_DABtoFM_LinkingStatus                   			 */
/*===========================================================================*/
void DAB_App_Notify_DABtoFM_LinkingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_DAB_FM_LINKING_STATUS_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_LinkingStatus,(Tu8)sizeof(e_LinkingStatus),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}


/*===========================================================================*/
/*  void DAB_App_Notify_Hardlinks_Status                   			 */
/*===========================================================================*/
void DAB_App_Notify_Hardlinks_Status(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_FM_BLENDING_STATUS_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_LinkingStatus,(Tu8)sizeof(e_LinkingStatus),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}



void DAB_App_Notify_DABtoFM_BlendingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_FM_BLENDING_STATUS_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_LinkingStatus,(Tu8)sizeof(Te_RADIO_DABFM_LinkingStatus),(&msg.msg_length));
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif	
	
}


/*===========================================================================*/
/*  void DAB_App_Notify_PIQuality                   					 	 */
/*===========================================================================*/
void DAB_App_Notify_PIQuality(Tu8 u8_Quality)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_PI_QUALITY_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&u8_Quality,(Tu8)sizeof(u8_Quality),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Notify_DLSData                   					 	 */
/*===========================================================================*/
void DAB_App_Notify_DLSData(Ts_DAB_DLS_Data st_DLS_Data)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_DLS_DATA_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&st_DLS_Data,(Tu8)sizeof(Ts_DAB_DLS_Data),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}
/*===========================================================================*/
/*  void DAB_App_Notify_SLSData                   					 	 */
/*===========================================================================*/
void DAB_App_Notify_SLSData(void)
{
	MessageHeaderWrite(&msg, RADIO_MNGR_APP, RADIO_MNGR_APP_DAB_SLS_DATA_NOTIFYID, DAB_APP_INST_HSM_CID);
	Sys_Send_Msg(&msg);
}

/*===========================================================================*/
/*  void DAB_App_Notify_ReConfiguration                   					 */
/*===========================================================================*/
void DAB_App_Notify_ReConfiguration(Ts_DAB_App_Tunable_StationInfo st_tunableinfo)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_RECONFIGURATION_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&st_tunableinfo,(Tu8)sizeof(Ts_DAB_App_Tunable_StationInfo),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}
/*===========================================================================*/
/*  void DAB_App_Notify_FMtoDAB_linked_Station                   					 	 */
/*===========================================================================*/
void DAB_App_Notify_FMtoDAB_linked_Station(Ts_DAB_App_CurrentStationInfo st_FM_DAB_Linked_station)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_FMDAB_SID_STATION_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&st_FM_DAB_Linked_station,(Tu8)sizeof(Ts_DAB_App_CurrentStationInfo),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}


/*===========================================================================*/
/*  void DAB_App_Notify_FMtoDAB_linking_Status                   					 	 */
/*===========================================================================*/
void DAB_App_Notify_FMtoDAB_linking_Status(Tu8 Quality, Te_DAB_APP_fmdab_linkstatus e_FM_DAB_Linking_status)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_FMDAB_SID_QUALITY_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&Quality,(Tu8)sizeof(Tu8),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_FM_DAB_Linking_status,(Tu8)sizeof(Te_DAB_APP_fmdab_linkstatus),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Notify_AnnoIndication                   					 	 */
/*===========================================================================*/
void DAB_App_Notify_AnnoIndication(Te_DAB_App_AnnoIndication e_DAB_App_AnnoIndication)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_ANNOUNCEMENT_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_DAB_App_AnnoIndication,(Tu8)sizeof(Te_DAB_App_AnnoIndication),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void DAB_App_Notify_SameChannelAnnoStatus                   					 	 */
/*===========================================================================*/
void DAB_App_Notify_SameChannelAnnoStatus(Te_DAB_App_AnnoIndication e_DAB_App_AnnoIndication, Tu8 u8_SameChannelSubChId, Tu8 u8_SameChannelClusterid)
{
	MessageHeaderWrite(&msg,RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_DAB_SAMECHANNELANNO_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&e_DAB_App_AnnoIndication,(Tu8)sizeof(Te_DAB_App_AnnoIndication),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&u8_SameChannelSubChId,(Tu8)sizeof(Tu8),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,(const void *)&u8_SameChannelClusterid,(Tu8)sizeof(Tu8),&msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}

/*===========================================================================*/
/*  void ComponentStatus		                   			 */
/*===========================================================================*/
void DAB_App_Notify_ComponentStatus(Te_RADIO_Comp_Status e_ComponentStatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DABTUNER_ABNORMAL_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_ComponentStatus,sizeof(Te_RADIO_Comp_Status),&msg.msg_length);
	Sys_Send_Msg(&msg);
}

/*===========================================================================*/
/*  void DAB_App_Notify_AMFMTUNERStatus		                   			 */
/*===========================================================================*/
void DAB_App_Notify_AMFMTunerStatus(Te_RADIO_Comp_Status e_AMFMTUNERStatus)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_AMFMTUNER_STATUS_NOTIFYID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,&e_AMFMTUNERStatus,sizeof(Te_RADIO_Comp_Status),&msg.msg_length);
	Sys_Send_Msg(&msg);
}	

/*===========================================================================*/
/*  void DAB_APP_Notify_StartBackgroundScan		                   			 */
/*===========================================================================*/
void DAB_APP_Notify_StartBackgroundScan(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_BACKGROUND_SCAN_START_NOTIFYID,RADIO_MNGR_APP); 
	Sys_Send_Msg(&msg);
}
/*===========================================================================*/
/*  void DAB_App_Notify_StragtegyStatus		                   			 */
/*===========================================================================*/
void DAB_App_Notify_StationNotAvail_StrategyStatus(Te_DAB_App_StationNotAvailStrategyStatus  e_DAB_App_StationNotAvailStrategyStatus)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_STATIONNOTAVAIL_STRATERGY_STATUS_NOTIFYID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,&e_DAB_App_StationNotAvailStrategyStatus,sizeof(Te_DAB_App_StationNotAvailStrategyStatus),&msg.msg_length);
	Sys_Send_Msg(&msg);
}	

/*===========================================================================*/
/*  void DAB_App_Notify_UpdatedLearnMem_AFStatus		                   			 */
/*===========================================================================*/
void DAB_App_Notify_UpdatedLearnMem_AFStatus(Te_DAB_App_LearnMemAFStatus e_DAB_App_LearnMemAFStatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_AF_LEARN_MEM_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_DAB_App_LearnMemAFStatus,sizeof(Te_DAB_App_LearnMemAFStatus),&msg.msg_length);
	UpdateParameterInMessage((Tu8*)msg.data,&st_CurrentStationInfo,sizeof(Ts_DAB_App_CurrentStationInfo),&msg.msg_length);
	Sys_Send_Msg(&msg);
}
/*===========================================================================*/
/*  void DAB_App_Notify_SignalStatus		                   			 */
/*===========================================================================*/
void DAB_App_Notify_SignalStatus(Te_DAB_App_SignalStatus e_DAB_App_SignalStatus)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_AF_SIGLOST_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_DAB_App_SignalStatus,sizeof(Te_DAB_App_SignalStatus),&msg.msg_length);
	Sys_Send_Msg(&msg);
}

/*===========================================================================*/
/*  void DAB_App_Notify_AF_SIgLow		                   			 */
/*===========================================================================*/
void DAB_App_Notify_DAB_DAB_Status(Te_DAB_APP_DAB_DAB_Status e_DAB_APP_DAB_DAB_Status)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_DAB_STATUS_NOTIFYID,DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data,&e_DAB_APP_DAB_DAB_Status,sizeof(Te_DAB_APP_DAB_DAB_Status),&msg.msg_length);
	Sys_Send_Msg(&msg);
}
void DAB_App_Notify_FMDAB_Stop_Type(Te_Dab_App_FmtoDAB_Reqstatus e_FmtoDAB_Reqstatus)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_FM_DAB_STOP_LINKING_NOTIFYID,RADIO_MNGR_APP); 
	UpdateParameterInMessage((Tu8*)msg.data,&e_FmtoDAB_Reqstatus,sizeof(Te_Dab_App_FmtoDAB_Reqstatus),&msg.msg_length);
	Sys_Send_Msg(&msg);
}
void DAB_App_Notify_SW_HW_Version(Ts_DAB_APP_GetVersion_Reply st_Verion_Number)
{
	MessageHeaderWrite(&msg,RADIO_MNGR_APP,RADIO_MNGR_APP_DAB_VERSION_NOTIFYID,DAB_APP_INST_HSM_CID); 
	UpdateParameterInMessage((Tu8*)msg.data,&st_Verion_Number,sizeof(Ts_DAB_APP_GetVersion_Reply),&msg.msg_length);
	Sys_Send_Msg(&msg);
}

void DAB_App_Notify_Init_FMDAB_linking(void)
{
	MessageHeaderWrite(&msg,RADIO_DAB_APP,DAB_APP_INIT_FMDAB_LINKING_NOTIFYID,RADIO_MNGR_APP); 
#ifdef PC_TEST
	DAB_APP_MSG_HandleMsg(&msg);
#else
	Sys_Send_Msg(&msg);
#endif

}

/*===========================================================================*/
/*  void DAB_App_Response_PlaySelectSt	                   					 */
/*===========================================================================*/
void DAB_App_Notify_AutoScan_PlayStation(Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo)
{
	MessageHeaderWrite(&msg, RADIO_MNGR_APP, RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION_NOTIFYID, DAB_APP_INST_HSM_CID);
	UpdateParameterInMessage((Tu8*)msg.data, &st_CurrentStationInfo, (Tu8)sizeof(st_CurrentStationInfo), &msg.msg_length);
#ifndef PC_TEST
	Sys_Send_Msg(&msg);
#endif
}
/*=============================================================================
    end of file
=============================================================================*/