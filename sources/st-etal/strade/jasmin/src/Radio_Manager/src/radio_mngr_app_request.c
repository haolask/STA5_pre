/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_request.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager Application															*
*  Description			: This source file consists of function defintions of all  Request APIs				*
*						  which will be	provided to  HMI IF Application										*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
							radio_mngr_app_request.c
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/


#include "radio_mngr_app_hsm.h"
#include "radio_mngr_app_request.h"



Ts_Sys_Msg ret_msg;

/*===========================================================================*/
/* void MSG_Update                                          */
/*===========================================================================*/
Ts_Sys_Msg* MSG_Update(Tu16 u16_cid, Tu16 u16_msgid)
{
	memset(&ret_msg,0,sizeof(Ts_Sys_Msg));
	ret_msg.dest_id   = u16_cid;
	ret_msg.msg_id	  =	u16_msgid;
	return(&ret_msg);
}


/*===========================================================================*/
/* UpdateParameterIntoMessage                                        */
/*===========================================================================*/
void UpdateParameterIntoMsg(char *pu8_data,const void *vp_parameter, Tu8 u8_ParamLength, Tu16 *pu16_Datalength)
{
	/* copying parameter to the data slots in Ts_Sys_Msg structure */
	SYS_RADIO_MEMCPY((pu8_data+*pu16_Datalength),(vp_parameter),(Tu32)(u8_ParamLength));

	/*  Updating msg_length for each parameter which represents length of data in  Ts_Sys_Msg structure   */
	*pu16_Datalength = (Tu16)(*pu16_Datalength + u8_ParamLength);
}

/*===========================================================================*/
/*ExtractParameterFromMessage                                                                                  */
/*=========================================================================== */
void ExtractParameterFromMsg(void *vp_Parameter, const char *pu8_DataSlot, Tu8 u8_ParamLength, Tu32 *pu32_index)
{
	/* reading parameter from the data slot present in Ts_Sys_Msg structure  */
	SYS_RADIO_MEMCPY(vp_Parameter,pu8_DataSlot+*pu32_index, (Tu32)(u8_ParamLength));

	/*  Updating index inorder to point to next parameter present in the data slot in  Ts_Sys_Msg structure   */
	*pu32_index= (Tu32)(*pu32_index + u8_ParamLength);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_StartTuner                                          */
/*===========================================================================*/
void Radio_Mngr_App_Request_StartTuner(Te_Radio_Mngr_App_Market e_Market, Tu8 u8_RadioComponentInfo)
{
	Ts_Sys_Msg* msg = NULL;

	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_STARTUP_REQID);
	
	UpdateParameterIntoMsg(msg->data, &e_Market,       		  sizeof(Te_Radio_Mngr_App_Market), &(msg->msg_length));
	UpdateParameterIntoMsg(msg->data, &u8_RadioComponentInfo, sizeof(Tu8),      				&(msg->msg_length));

	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/*  void Radio_Mngr_App_Inst_Hsm_Start(Te_Radio_Mngr_App_Market e_Market)                                 			 */
/*===========================================================================*/
void Radio_Mngr_App_Inst_Hsm_Start(Te_Radio_Mngr_App_Market e_Market)
{
	Ts_Sys_Msg *msg;

	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_INST_START);
	
    UpdateParameterIntoMsg(msg->data, &e_Market,    sizeof(e_Market),    &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_ShutDownTuner                                          */
/*===========================================================================*/
void Radio_Mngr_App_Request_ShutDownTuner(void)
{
	Ts_Sys_Msg* msg = NULL;
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_SHUTDOWN_REQID);

	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_SelectBand                                          */
/*===========================================================================*/
 void Radio_Mngr_App_Request_SelectBand(Te_Radio_Mngr_App_Band e_Band)
{
	Ts_Sys_Msg* msg = NULL;
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_SELECTBAND_REQID);
	UpdateParameterIntoMsg(msg->data,&e_Band,sizeof(e_Band),&(msg->msg_length));

	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_PlaySelectSt                                          */
/*===========================================================================*/
void Radio_Mngr_App_Request_PlaySelectSt(Tu8 u8_Index)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID);
	
	UpdateParameterIntoMsg(msg->data,&u8_Index,sizeof(u8_Index),&(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Audio_Manager_Request_Mute                                          */
/*===========================================================================*/
void Audio_Manager_Request_Mute(Te_Radio_Mngr_App_Band eBand)
{
	Notify_UpdateTunerMute(eBand, AUDIO_MNGR_MUTE);
}

/*===========================================================================*/
/* void Audio_Manager_Request_DeMute                                          */
/*===========================================================================*/
void Audio_Manager_Request_DeMute(Te_Radio_Mngr_App_Band eBand)
{
	Notify_UpdateTunerMute(eBand, AUDIO_MNGR_DEMUTE);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_SeekUpDown                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_SeekUpDown(Te_RADIO_DirectionType e_direction)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_SEEKUPDOWN_REQID);
	
	UpdateParameterIntoMsg(msg->data, &e_direction, sizeof(e_direction), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_RDS_Switch                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_RDSSettings(Te_Radio_Mngr_App_RDSSettings e_RDSSettings_Request)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_AF_SWITCH_REQID);

	UpdateParameterIntoMsg(msg->data, &e_RDSSettings_Request, sizeof(Te_Radio_Mngr_App_RDSSettings), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_EnableDABtoFMLinking                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_EnableDABFMLinking(Te_Radio_Mngr_App_DABFMLinking_Switch e_DABtoFMLinkingSwitch)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_ENABLE_DABFMLINKING_REQID);

	UpdateParameterIntoMsg(msg->data, &e_DABtoFMLinkingSwitch, sizeof(Te_Radio_Mngr_App_DABFMLinking_Switch), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_EnableTAAnnouncement                                   */
/*===========================================================================*/
void Radio_Mngr_App_Request_EnableTAAnnouncement(Te_Radio_Mngr_App_EnableTAAnno_Switch e_TA_Anno_Switch)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_TA_ANNO_SWITCH_REQID);

	UpdateParameterIntoMsg(msg->data, &e_TA_Anno_Switch, sizeof(Te_Radio_Mngr_App_EnableTAAnno_Switch), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_GetAFList_Diag                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_GetAFList_Diag(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_GET_AFLIST_DIAG_REQID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_GetQuality_Diag                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_GetQuality_Diag(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_GET_QUALITY_DIAG_REQID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_GetCurStationInfo_Diag                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_GetCurStationInfo_Diag(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_GET_CURRENTSTATIONINFO_DIAG_REQID);
	
	SYS_SEND_MSG(msg);
}


/*===========================================================================*/
/* void Radio_Mngr_App_Request_ObtainStationList_Diag                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_ObtainStationList_Diag(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_OBTAIN_STATIONLIST_DIAG_REQID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_PresetStore                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_PresetStore(Tu8 u8_Preset_Index)
{
	Preset_Store_with_Current_Station(u8_Preset_Index);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_PresetRecall                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_PresetRecall(Tu8 u8_Preset_Index)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_PRESET_RECALL_REQID);

	UpdateParameterIntoMsg(msg->data, &u8_Preset_Index, sizeof(Tu8), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_GetPresetList                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_GetPresetList()
{
	Radio_Mngr_App_GetPresetList();
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_UpdateStationList                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_UpdateStationList(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID);
	
	SYS_SEND_MSG(msg);
}
/*===========================================================================*/
/* void Radio_Mngr_App_Request_TuneUpDown                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_TuneUpDown(Te_RADIO_DirectionType e_TuneUpDownDirection)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_TUNEUPDOWN_REQID);

	UpdateParameterIntoMsg(msg->data, &e_TuneUpDownDirection, sizeof(Te_RADIO_DirectionType), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_AnnoCancel                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_AnnoCancel(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_ANNO_CANCEL_REQID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Internal_HMI_Request_Message                                    */
/*===========================================================================*/
void Radio_Mngr_App_Internal_HMI_Request_Message(Ts_Sys_Msg *pst_msg)
{	
	SYS_SEND_MSG(pst_msg);
}
/*===========================================================================*/
/* void Radio_Mngr_App_Request_RadioStatus                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_AMFMReTune(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_AMFM_RETUNE_REQID);
	
	SYS_SEND_MSG(msg);
}
/*===========================================================================*/
/* void Radio_Mngr_App_Request_ENG_Mode                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_ENG_Mode(Te_Radio_Mngr_App_Eng_Mode_Request e_EngModeRequest)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_ENG_MODE_REQID);
	
	UpdateParameterIntoMsg(msg->data, &e_EngModeRequest, sizeof(Te_Radio_Mngr_App_Eng_Mode_Request), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_TuneByFreq                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_TuneByFreq(Tu32 u32_Freq, Tu8 *pu8_ChannelName)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID);

	UpdateParameterIntoMsg(msg->data, &u32_Freq, sizeof(Tu32), &(msg->msg_length));
	UpdateParameterIntoMsg(msg->data, pu8_ChannelName, RADIO_MNGR_APP_MAX_CHANNEL_NAME_SIZE, &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_RadioSRCActivateDeActivate                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_RadioSRCActivateDeActivate(Te_Radio_Mngr_App_Band e_Band, Te_Radio_Mngr_App_SRC_ActivateDeActivate e_ActivateDeactivate)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_SRC_ACTIVATE_DEACTIVATE_REQID);

	UpdateParameterIntoMsg(msg->data, &e_Band,			     sizeof(Te_Radio_Mngr_App_Band),				      &(msg->msg_length));
	UpdateParameterIntoMsg(msg->data, &e_ActivateDeactivate, sizeof(Te_Radio_Mngr_App_SRC_ActivateDeActivate),    &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* Radio_Mngr_App_Request_GetClockTime                                   */
/*===========================================================================*/
void Radio_Mngr_App_Request_GetClockTime(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_GETCLOCKTIME_REQID);

	SYS_SEND_MSG(msg);
}
/*===========================================================================*/
/* Radio_Mngr_App_Request_ManualSTLUpdateCancel                                   */
/*===========================================================================*/
void Radio_Mngr_App_Request_ManualSTLUpdateCancel(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_MANUAL_STLUPDATE_CANCEL_REQID);

	SYS_SEND_MSG(msg);
}
/*===========================================================================*/
/* void Radio_Mngr_App_Request_EnableAnnouncementInfo                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_EnableAnnouncementInfo(Te_Radio_Mngr_App_EnableInfoAnno_Switch e_Info_Anno_Switch)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_INFO_ANNO_SWITCH_REQID);

	UpdateParameterIntoMsg(msg->data, &e_Info_Anno_Switch, sizeof(Te_Radio_Mngr_App_EnableInfoAnno_Switch), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}
/*===========================================================================*/
/* void Radio_Mngr_App_Request_RadioPowerON                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_RadioPowerON(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_POWER_ON_REQID);
	
	SYS_SEND_MSG(msg);
}
/*===========================================================================*/
/* void Radio_Mngr_App_Request_RadioPowerOFF                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_RadioPowerOFF(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_POWER_OFF_REQID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_FactoryReset                                    */
/*===========================================================================*/
void Radio_Mngr_App_Request_FactoryReset(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_FACTORY_RESET_REQID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_InstHSM_FactoryReset                          */
/*===========================================================================*/
void Radio_Mngr_App_Request_InstHSM_FactoryReset(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_INST_HSM_FACTORY_RESET_REQID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Response_InstHSMFactoryReset                          */
/*===========================================================================*/
void Radio_Mngr_App_Response_InstHSMFactoryReset(void)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_INST_HSM_FACTORY_RESET_DONE_RESID);
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_Play_SelectStation_From_StlSearch                                   */
/*===========================================================================*/
void Radio_Mngr_App_Request_Play_SelectStation_From_StlSearch(Tu8 u8_index)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID);

	UpdateParameterIntoMsg(msg->data, &u8_index, sizeof(Tu8), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}
/*===========================================================================*/
/* void Radio_Mngr_App_Request_MultiplexList_Switch                                 */
/*===========================================================================*/
void Radio_Mngr_App_Request_MultiplexList_Switch(Te_Radio_Mngr_App_Multiplex_Switch e_MultiplexSettings)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_MULTIPLEX_SWITCH_REQID);

	UpdateParameterIntoMsg(msg->data, &e_MultiplexSettings, sizeof(Te_Radio_Mngr_App_Multiplex_Switch), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Request_StationSelect_From_EnsembleList                                 */
/*===========================================================================*/
void Radio_Mngr_App_Request_StationSelect_From_EnsembleList(Tu8 u8_ServiceIndex)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID);

	UpdateParameterIntoMsg(msg->data, &u8_ServiceIndex, sizeof(Tu8), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

