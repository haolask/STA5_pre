/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_response.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager Application															*
*  Description			: This source file consists of function defintions of all Response APIs				*
*						  which send response messages to HMI IF Application 								*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
							radio_mngr_app_response.c
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
							File Inclusion
-----------------------------------------------------------------------------*/
#include "radio_mngr_app_hsm.h"
#include "radio_mngr_app_response.h"
/*---------------------------------------------------------------------------*/

/*===========================================================================*/
/*  Radio_Mngr_App_Response_StartTuner                            */
/*===========================================================================*/
void Radio_Mngr_App_Response_StartTuner(Te_RADIO_ReplyStatus e_startTunerReplyStatus)
{
	Sys_Startup_Response(e_startTunerReplyStatus);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_Shutdown                              */
/*===========================================================================*/
void Radio_Mngr_App_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus)
{
	Sys_Shutdown_Response(e_ShutdownReplyStatus);
}

/*===========================================================================*/
/*  Function for response of the select band request                               */
/*===========================================================================*/
void Radio_Mngr_App_Response_SelectBand(Te_RADIO_ReplyStatus e_selectbandreplystatus, Te_Radio_Mngr_App_Band e_activeBand)
{
	Notify_UpdateCurRadioMode(e_selectbandreplystatus, e_activeBand);
}

/*===========================================================================*/
/*  Function for response of the play select station request                       */
/*===========================================================================*/
void Radio_Mngr_App_Response_PlaySelectSt(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{	
	Radio_Response_PlaySelectStation(pst_me_radio_mngr_inst->e_SelectStationReplyStatus);

	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq, 
																		(Tu8*)NULL, (Tu8)3, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			if(pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_FM)
			{
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																		(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}
			else if(pst_me_radio_mngr_inst->e_Anno_Status == RADIO_MNGR_APP_ANNO_START)
			{
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.au8_RadioText,
																		(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);	
			}
			else if(pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			if(pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																		pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																		pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																		pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
			}
			else if(pst_me_radio_mngr_inst->e_Anno_Status == RADIO_MNGR_APP_ANNO_START)
			{
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																		pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.st_Tunableinfo.u32_Frequency,
																		pst_me_radio_mngr_inst->st_DAB_Anno_CurrentStationName.au8_CompLabel,
																		pst_me_radio_mngr_inst->st_DAB_Anno_CurrentStationName.u8_CharSet,
																		pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																		pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.au8_ChannelName,
																		pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);	
			}
			else if(pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_FM)
			{
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}
		}
		break;

		default:
		{
		}
		break;
	}
}

/*===========================================================================*/
/*  Radio_Mngr_App_Response_Mute                               */
/*===========================================================================*/
void Radio_Mngr_App_Response_Mute(Te_RADIO_ReplyStatus e_muteReplayStatus)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_MUTE_DONE_RESID);
	
	UpdateParameterIntoMsg(msg->data ,&e_muteReplayStatus, sizeof(e_muteReplayStatus), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}


/*===========================================================================*/
/*  Radio_Mngr_App_Response_DeMute                             */
/*===========================================================================*/
void Radio_Mngr_App_Response_DeMute(Te_RADIO_ReplyStatus e_demutereplaystatus)
{
	Ts_Sys_Msg* msg = NULL;
	
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_DEMUTE_DONE_RESID);
	
	UpdateParameterIntoMsg(msg->data ,&e_demutereplaystatus, sizeof(e_demutereplaystatus), &(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}


/*===========================================================================*/
/* Radio_Mngr_App_Response_SeekUpDown                                        */
/*===========================================================================*/
void Radio_Mngr_App_Response_SeekUpDown(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
    Radio_Response_SeekStation(pst_me_radio_mngr_inst->e_SeekReplyStatus);

	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq,
																		(Tu8*)NULL, (Tu8)3, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																		(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																		pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																		pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																		pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
		}
		break;

		default:
		{
		}
		break;
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_TuneUpDown                                        */
/*===========================================================================*/
void Radio_Mngr_App_Response_TuneUpDown(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Radio_Response_TuneUpDown(pst_me_radio_mngr_inst->e_TuneUpDownReplyStatus);

	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq, 
																		(Tu8*)NULL, (Tu8)RADIO_MNGR_APP_VALUE_ZERO, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																		(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																		pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																		pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																		pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
		}
		break;

		default:
		{
		}
		break;
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_UpdateCurStationInfo_Display                                       */
/*===========================================================================*/
void Radio_Mngr_App_Response_UpdateCurStationInfo_Display(Te_Radio_Mngr_App_Band e_ActBand, Tu32 Freq, Tu8 *pu8_Name, Tu8 u8_CharSet, Tu8 *pu8_RadioText, Tu8 *pu8_ChannelName, Tu8 *pu8Ensemble_Label, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	if(e_ActBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_SELECT_STATION_END)
	{
		Radio_Mngr_App_Update_ServiceNumber_In_EnsembleList(pst_me_radio_mngr_inst);
	}else{/*FOR MISRA C*/}

	if (e_ActBand == RADIO_MNGR_APP_BAND_FM)
	{
		Notify_UpdateCurStationInfo_Display(e_ActBand, Freq, pu8_Name, pu8_RadioText, u8_CharSet, pu8_ChannelName, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_Prgm_Name, pu8Ensemble_Label, pst_me_radio_mngr_inst->u8_CurrentlyPlayingServiceInEnsemble, pst_me_radio_mngr_inst->u8_TotalNoOfServiceInEnsemble, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_TA, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_TP, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI);
	}
	else
	{
		Notify_UpdateCurStationInfo_Display(e_ActBand, Freq, pu8_Name, pu8_RadioText, u8_CharSet, pu8_ChannelName, (Tu8*)NULL, pu8Ensemble_Label, pst_me_radio_mngr_inst->u8_CurrentlyPlayingServiceInEnsemble, pst_me_radio_mngr_inst->u8_TotalNoOfServiceInEnsemble, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_TA, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_TP, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI);
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_PresetStore                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_PresetStore(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_RADIO_ReplyStatus e_PresetStoreReplyStatus)
{
	Radio_Response_StoreMemoryList(e_PresetStoreReplyStatus);
	Notify_UpdateMemoryList(&(pst_me_radio_mngr_inst->st_PrestMixedList));
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_PresetRecall                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_PresetRecall(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Radio_Response_PlaySelectMemoryList(pst_me_radio_mngr_inst->e_PresetRecallReplayStatus);
	
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq, 
																		(Tu8*)NULL, (Tu8)RADIO_MNGR_APP_VALUE_ZERO, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																		(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																		pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																		pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																		pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																		pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
		}
		break;
		
		default:
		{
			/* To avoid Warnings */
		}
		break;
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_PresetList                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_PresetList(Ts_Radio_Mngr_App_Preset_Mixed_List *st_PrestMixedList)
{
	Notify_UpdateMemoryList(st_PrestMixedList);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_Update_StationList                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_Update_StationList(Te_RADIO_ReplyStatus e_StationListReplyStatus)
{
	Radio_Response_ManualUpdateSTL(e_StationListReplyStatus);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_EnableDABtoFMLinking                                       */
/*===========================================================================*/
void Radio_Mngr_App_Response_FMDABSwitch(Te_RADIO_ReplyStatus  e_EnableDABtoFMLinkingStatus)
{
	Radio_Response_EnableDABFMLinking(e_EnableDABtoFMLinkingStatus);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_RDSFollowing                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_RDS_Switch_Status(Te_RADIO_ReplyStatus e_RDSSwitchStatus)
{
	Radio_Response_RDSFollowing(e_RDSSwitchStatus);
}

/*===========================================================================*/
/* Radio_Mngr_Response_TA_Anno_Switch                                      */
/*===========================================================================*/
void Radio_Mngr_Response_TA_Anno_Switch(Te_RADIO_ReplyStatus e_TA_Switch_Status)
{
	Radio_Response_EnableTAAnnouncement(e_TA_Switch_Status);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_AnnoCancel                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_AnnoCancel(Te_RADIO_ReplyStatus  e_AnnoCancelStatus)
{
	Radio_Response_CancelAnnouncement(e_AnnoCancelStatus);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_ClockTime                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_ClockTime(Te_RADIO_ReplyStatus e_CTInfoReplyStatus)
{
	Radio_Response_GetClockTime(e_CTInfoReplyStatus);
}
/*===========================================================================*/
/* Radio_Mngr_App_Response_CancelManualSTlUpdate                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_CancelManualSTlUpdate(Te_RADIO_ReplyStatus e_CancelManualSTLReplyStatus)
{
	Radio_Response_CancelManualSTLUpdate(e_CancelManualSTLReplyStatus);
}
/*===========================================================================*/
/* Radio_Mngr_Response_Info_Anno_Switch                                      */
/*===========================================================================*/
void Radio_Mngr_Response_Info_Anno_Switch(Te_RADIO_ReplyStatus e_Info_Switch_Status)
{
	Radio_Response_EnableInfoAnnouncement(e_Info_Switch_Status);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_FM_DAB_AFStationInfo                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_BGStationInfo(Te_Radio_Mngr_App_Band e_ActSRC, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	if(e_ActSRC == RADIO_MNGR_APP_BAND_FM)
	{
		Radio_Response_BG_AFStationInfo(e_ActSRC, RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency,
														pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
														pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
														pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
														(Tu8*)NULL, pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, RADIO_MNGR_APP_VALUE_ZERO, RADIO_MNGR_APP_VALUE_ZERO);
	}
	else if(e_ActSRC == RADIO_MNGR_APP_BAND_DAB)
	{
		/*Function to update service number and total number of service present in the ensemble*/
		Radio_Mngr_App_Update_ServiceNumber_In_EnsembleList(pst_me_radio_mngr_inst);
		Radio_Response_BG_AFStationInfo(e_ActSRC, RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency, 
														pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
														pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
														pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName, (Tu8*)NULL, pst_me_radio_mngr_inst->u8_CurrentlyPlayingServiceInEnsemble, pst_me_radio_mngr_inst->u8_TotalNoOfServiceInEnsemble);
	}else{/*FOR MISRA C*/}
}
/*===========================================================================*/
/* Radio_Mngr_App_Response_Power_ON                                      	 */
/*===========================================================================*/
void Radio_Mngr_App_Response_Power_ON(Te_RADIO_ReplyStatus e_PowerOnReplyStatus)
{
	Radio_Response_PowerOnStatus(e_PowerOnReplyStatus);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_Power_ON                                      	 */
/*===========================================================================*/
void Radio_Mngr_App_Response_Power_OFF(Te_RADIO_ReplyStatus e_PowerOffReplyStatus)
{
	Radio_Response_PowerOffStatus(e_PowerOffReplyStatus);
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_Factory_Reset                                      	 */
/*===========================================================================*/
void Radio_Mngr_App_Response_Factory_Reset(Te_RADIO_ReplyStatus e_FactoryReset_ReplyStatus)
{
	Sys_Factory_Reset_Response(e_FactoryReset_ReplyStatus);
}
/*===========================================================================*/
/* Radio_Mngr_App_Response_Multiplex_Switch_Status                                      */
/*===========================================================================*/
void Radio_Mngr_App_Response_Multiplex_Switch_Status(Te_RADIO_ReplyStatus e_MultiplexSettings_ReplyStatus)
{
	Radio_Response_EnableMultiplexInfoSwitch(e_MultiplexSettings_ReplyStatus);
}
