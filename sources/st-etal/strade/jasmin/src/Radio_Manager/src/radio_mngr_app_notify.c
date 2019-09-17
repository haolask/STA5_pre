/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_notify.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager Application													     	*
*  Description			: This source file consists of function definitions of all Notification APIs		*
*						  which send messages to HMI IF Application											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
							radio_mngr_app_notify.c
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
							File Inclusion
-----------------------------------------------------------------------------*/
#include "radio_mngr_app_hsm.h"
#include "radio_mngr_app_notify.h"
#include "radio_mngr_app_response.h"

/*---------------------------------------------------------------------------*/
/*===========================================================================*/
/* Radio_Mngr_App_Notify_StationList								*/
/*===========================================================================*/
void Radio_Mngr_App_Notify_StationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band)
{
	switch(e_Band)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			Notify_StationList(RADIO_MNGR_APP_BAND_AM, &(pst_me_radio_mngr_inst->st_RadioStationList), pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList,
										pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			Notify_StationList(RADIO_MNGR_APP_BAND_FM, &(pst_me_radio_mngr_inst->st_RadioStationList), pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList,
										pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index);
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			Notify_StationList(RADIO_MNGR_APP_BAND_DAB, &(pst_me_radio_mngr_inst->st_NormalStnView), pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList,
										pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index);
		}
		break;

		default:
		break;
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_UpdateScanStatus								*/
/*===========================================================================*/
void Radio_Mngr_App_Notify_UpdateScanStatus(Te_Radio_Mngr_App_ScanStatus e_ScanStatus)
{
	Notify_UpdateScanStatus(e_ScanStatus);
	
}

/*===========================================================================*/
/* Radio_Mngr_App_Response_DABFMLinkingStatus                                     */
/*===========================================================================*/
void Radio_Mngr_App_Notify_DABFMLinkingStatus(Te_RADIO_DABFM_LinkingStatus e_EnableDABFM_Status)
{
	DABFM_Notify_DABFMLinkingStatus(e_EnableDABFM_Status);	
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_Announcement                                     */
/*===========================================================================*/
void Radio_Mngr_App_Notify_Announcement(Te_Radio_Mngr_App_Anno_Status e_Anno_Status)
{
	Radio_Notify_Announcement(e_Anno_Status);	
}		

/*===========================================================================*/
/* Radio_Mngr_App_Notify_AFStatus                                     */
/*===========================================================================*/
void Radio_Mngr_App_Notify_AFStatus(Te_Radio_Mngr_App_AF_Status e_AFStatus)
{
	AMFM_Notify_AFSwitchStatus(e_AFStatus);	
}		

/*===========================================================================*/
/* Radio_Mngr_App_Notify_Settings                                     */
/*===========================================================================*/
void Radio_Mngr_App_Notify_Settings(Te_Radio_Mngr_App_DABFMLinking_Switch e_DABFMLinking_Switch, Te_Radio_Mngr_App_EnableTAAnno_Switch e_TA_Anno_Switch, Te_Radio_Mngr_App_RDSSettings e_RDSSettings, Te_Radio_Mngr_App_EnableInfoAnno_Switch e_Info_Anno_Switch, Te_Radio_Mngr_App_Multiplex_Switch e_MultiplexSettings)
{
	Radio_Notify_Settings(e_DABFMLinking_Switch, e_TA_Anno_Switch, e_RDSSettings, e_Info_Anno_Switch, e_MultiplexSettings);
	
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Settings Updated DAB<=>FM:%d, TA:%d, RDS:%d, Anni_Info:%d, Multiplex_Setting:%d", e_DABFMLinking_Switch, e_TA_Anno_Switch, e_RDSSettings, e_Info_Anno_Switch, e_MultiplexSettings);	
}


/*===========================================================================*/
/* Radio_Mngr_App_Notify_Components_Status sos's status                      */
/*===========================================================================*/
void Radio_Mngr_App_Notify_Components_Status(Te_Radio_Mngr_App_Band e_ActiveBand, Te_RADIO_AMFMTuner_Status e_AMFMTunerStatus, Te_RADIO_Comp_Status e_DABTunerStatus, Te_Radio_Mngr_App_DAB_UpNotification_Status	e_DAB_UpNot_Status)
{	
	Radio_Notify_Components_Status(e_ActiveBand, e_AMFMTunerStatus, e_DABTunerStatus, e_DAB_UpNot_Status);
	
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT,"[Radio][RM]Component Status AMFM TUNER:%d, DAB TUNER:%d, DAB_UP_NOT:%d", e_AMFMTunerStatus, e_DABTunerStatus, e_DAB_UpNot_Status);
}
/*===========================================================================*/
/* Radio_Mngr_App_Notify_Activity_State                     */
/*===========================================================================*/
void Radio_Mngr_App_Notify_Activity_State(Te_Radio_Mngr_App_Band e_ActiveBand,Te_Radio_Mngr_App_Activity_Status e_Activity_State)
{	
	Notify_Activity_State(e_ActiveBand, e_Activity_State);
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag                                       */
/*===========================================================================*/
void Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
		
			AMFM_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq, 
														pst_me_radio_mngr_inst->u32_Quality, NULL, RADIO_MNGR_APP_VALUE_ZERO, RADIO_MNGR_APP_VALUE_ZERO, RADIO_MNGR_APP_VALUE_ZERO,
														RADIO_MNGR_APP_VALUE_ZERO);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			if(pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START)
			{
				AMFM_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
														pst_me_radio_mngr_inst->u32_Quality, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
														(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI),
														pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_TA, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_TP, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet);
			}
			else
			{
				AMFM_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
														pst_me_radio_mngr_inst->u32_Quality, pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
														(pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.u16_PI),
														pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.u8_TA, pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.u8_TP, pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.u8_CharSet);
			}											
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			if(pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START)
			{
				DAB_Notify_UpdateStationInfo_Diag(pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
													pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_EId,
													pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_SId,
													pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_SCIdI,
													pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel, pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet);
			}
			else
			{
				DAB_Notify_UpdateStationInfo_Diag(pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.st_Tunableinfo.u32_Frequency,
													pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.st_Tunableinfo.u16_EId,
													pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.st_Tunableinfo.u32_SId,
													pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.st_Tunableinfo.u16_SCIdI,
													pst_me_radio_mngr_inst->st_DAB_Anno_CurrentStationName.au8_CompLabel, pst_me_radio_mngr_inst->st_DAB_Anno_CurrentStationName.u8_CharSet);
			}															
		}
		break;

		default:
		{
		}
		break;
		
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_AFSwitchStatus                                      */
/*===========================================================================*/
void Radio_Mngr_App_Notify_AFList_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_DAB:
		{
			DAB_Notify_AFList_Diag(pst_me_radio_mngr_inst->st_DAB_AFList.au32_AltFrequency, pst_me_radio_mngr_inst->st_DAB_AFList.au16_AltEnsemble,
										pst_me_radio_mngr_inst->st_DAB_AFList.au32_HardlinkSid, pst_me_radio_mngr_inst->st_DAB_AFList.u8_NumAltFreq, 
										pst_me_radio_mngr_inst->st_DAB_AFList.u8_NumAltEnsemble, pst_me_radio_mngr_inst->st_DAB_AFList.u8_NumAltHardLinkSId);
		}
		break;
		
		case RADIO_MNGR_APP_BAND_FM:
		{
			AMFM_Notify_AFList_Diag(pst_me_radio_mngr_inst->st_FM_AFList.u8_NumAFList, pst_me_radio_mngr_inst->st_FM_AFList.au32_AFList,
										pst_me_radio_mngr_inst->st_FM_AFList.au32_Quality, pst_me_radio_mngr_inst->st_FM_AFList.au16_PIList);
		}
		break;
		
		default:
		{
			/* do nothing */
		}
		break;
	}
	
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_Quality_Diag								*/
/*===========================================================================*/
void Radio_Mngr_App_Notify_Quality_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_activeBand)
{
	switch(e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_DAB:
		{
			DAB_Notify_Quality_Diag(pst_me_radio_mngr_inst->st_DAB_TunerNotify.s32_RFFieldStrength,
										pst_me_radio_mngr_inst->st_DAB_TunerNotify.s32_BBFieldStrength,
							  			pst_me_radio_mngr_inst->st_DAB_TunerNotify.u32_FicBitErrorRatio, 
							  			pst_me_radio_mngr_inst->st_DAB_TunerNotify.b_isValidMscBitErrorRatio,
										pst_me_radio_mngr_inst->st_DAB_TunerNotify.u32_MscBitErrorRatio);
		}
		break;
		
		case RADIO_MNGR_APP_BAND_FM:
		{
			AMFM_Notify_Quality_Diag(pst_me_radio_mngr_inst->e_activeBand,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.s32_RFFieldStrength,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.s32_BBFieldStrength, 
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_UltrasonicNoise,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_SNR,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_AdjacentChannel,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_coChannel,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_StereoMonoReception,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_Multipath,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_FrequencyOffset,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_ModulationDetector);
		}
		break;
		
		case RADIO_MNGR_APP_BAND_AM:
		{
			AMFM_Notify_Quality_Diag(pst_me_radio_mngr_inst->e_activeBand,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.s32_RFFieldStrength,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.s32_BBFieldStrength, 
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_UltrasonicNoise,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_SNR,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_AdjacentChannel,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_coChannel,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_StereoMonoReception,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_Multipath,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_FrequencyOffset,
										pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u32_ModulationDetector);
		}
		break;
		
		default:
		{
			/* do nothing */
		}
		break;
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_UpdateSTL_Diag                                       */
/*===========================================================================*/
void Radio_Mngr_App_Notify_UpdateSTL_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{	
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			AMFM_Notify_UpdateAMSTL_Diag(&(pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList), pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			AMFM_Notify_UpdateFMSTL_Diag(&(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList), pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList);
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			if(pst_me_radio_mngr_inst->e_MultiplexSettings==RADIO_MNGR_APP_MULTIPLEX_ENABLE)
			{
				DAB_Notify_UpdateMultiplexStl_Diag(&(pst_me_radio_mngr_inst->st_DABEnsembleList),pst_me_radio_mngr_inst->st_DABEnsembleList.u8_NoOfEnsembleList);
			}
			else
			{
				DAB_Notify_UpdateNormalStl_Diag(&(pst_me_radio_mngr_inst->st_NormalStnView), pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList);
			}
		}
		break;

		default:
		{
		}
		break;
		
	}

}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_DABFM_Linking_BestPI_Diag                                      */
/*===========================================================================*/
void Radio_Mngr_App_Notify_DABFM_Linking_BestPI_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Notify_BestPIStation(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, pst_me_radio_mngr_inst->u16_BestPi, pst_me_radio_mngr_inst->u32_Quality);
}

/*===========================================================================*/
/* void Radio_Mngr_App_Notify_ClockTime                            */
/*===========================================================================*/
void Radio_Mngr_App_Notify_ClockTime(Ts_Radio_Mngr_App_CT_Info* st_AMFMTUNER_CTInfo)
{
	Radio_Notify_ClockTime(st_AMFMTUNER_CTInfo->u8_Hour, st_AMFMTUNER_CTInfo->u8_Min, st_AMFMTUNER_CTInfo->u8_Day, st_AMFMTUNER_CTInfo->u8_Month, st_AMFMTUNER_CTInfo->u16_Year);
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_Clear_HMI_Data                                      */
/*===========================================================================*/
void Radio_Mngr_App_Notify_Clear_HMI_Data(Te_Radio_Mngr_App_Band e_Band, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	switch(e_Band)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_AM,
																		pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq, 
																		(Tu8*)NULL, (Tu8)3, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																 pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq, 
																 (Tu8*)NULL, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			pst_me_radio_mngr_inst->u8_CurrentlyPlayingServiceInEnsemble 	= RADIO_MNGR_APP_VALUE_ZERO;
			pst_me_radio_mngr_inst->u8_TotalNoOfServiceInEnsemble 			= RADIO_MNGR_APP_VALUE_ZERO;
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_DAB,
																 pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq,
																 (Tu8*)NULL,pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																 (Tu8*)NULL,pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																 (Tu8*)NULL, pst_me_radio_mngr_inst);
		}
		break;

		default:
		{
		}
		break;
	}
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_Clear_HMI_Data                                      */
/*===========================================================================*/
void Radio_Mngr_App_Notify_AudioSwitch(Te_Radio_Mngr_App_Band e_ReqAudioChangeBand)
{
	Radio_Notify_AudioSwitch(e_ReqAudioChangeBand);
}

/*===========================================================================*/
/* Radio_Mngr_App_Notify_FirmwareVersion                                      */
/*===========================================================================*/
void Radio_Mngr_App_Notify_FirmwareVersion(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr)
{
	UNUSED(pst_me_radio_mngr);
	//Radio_Notify_FirmwareVersion(pst_me_radio_mngr->st_DAB_Version_Info.au8_DABTuner_SWVersion, pst_me_radio_mngr->st_DAB_Version_Info.au8_DABTuner_HWVersion, pst_me_radio_mngr->au8_AMFM_Version_Info);
	/* Commented since DAB TUNER and AMFM TUNER Version number is not needed for ST*/
}


