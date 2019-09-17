/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_app.c																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager Application															*
*  Description			: This source file contains function definitions for all internal funtions of Radio *
*						  Manager Application component														*	
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
				File Inclusions
-----------------------------------------------------------------------------*/

#include "hmi_if_utf8_conversion.h"
#include "hmi_if_app.h"
#include "hmi_if_extern.h"
#include "radio_mngr_app_hsm.h"
#include "DAB_Tuner_Ctrl_Types.h"
#include "dab_app_types.h"
#include "radio_mngr_app_response.h"
#include "radio_mngr_app_notify.h"
#include "sys_nvm.h"

extern Ts_DAB_App_StationList		st_DAB_App_StationList; /* structure holds DAB station list*/
extern Ts_AMFM_App_AM_STL			st_am_station_list;		/* structure holds AM station list as per master specified format */ 
extern Ts_AMFM_App_FM_STL			st_fm_station_list;		/* structure holds FM station list as per master specified format */
extern Ts_DAB_App_MultiplexStationList	st_DAB_App_MultiplexStationList;/* structure holds DAB Ensemble info of station list*/
extern Ts_Radio_Mngr_App_TimerIds   st_Radio_Mngr_App_TimerID;/* structure holds Timerids */
/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/

Ts_Radio_Mngr_App_Hsm	st_radio_mngr_app_hsm; 

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/
/*===========================================================================*/
/*  Radio_Mngr_App_Component_Init(void)                                  	 */
/*===========================================================================*/
void Radio_Mngr_App_Component_Init(void)
{
	   /* create, initialise and start HSM */
    Radio_Mngr_App_Hsm_Init(&st_radio_mngr_app_hsm);
}

/*===========================================================================*/
/*  static void Radio_Mngr_App_Msg_HandleMsg                                        */
/*===========================================================================*/
void Radio_Mngr_App_Msg_HandleMsg(Ts_Sys_Msg* pst_msg)
{
  HSM_ON_MSG(&st_radio_mngr_app_hsm, pst_msg);
}



/*===========================================================================*/
/*  Radio_Mngr_App_Inst_Hsm_Stop(void)                                  			 */
/*===========================================================================*/
void Radio_Mngr_App_Inst_Hsm_Stop(void)
{
	Ts_Sys_Msg *msg;
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_APP_INST_STOP);
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/*  adio_Mngr_App_Inst_Hsm_Start_Response(void)                                  			 */
/*===========================================================================*/
void Radio_Mngr_App_Inst_Hsm_Start_Response(Te_RADIO_ReplyStatus e_InstHSMReplystatus)
{
	Ts_Sys_Msg *msg;
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_INST_HSM_START_DONE);
	UpdateParameterIntoMsg(msg->data, &e_InstHSMReplystatus, sizeof(e_InstHSMReplystatus),&(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/*  Radio_Mngr_App_Responsse_Inst_Hsm_Stop(void)                                  			 */
/*===========================================================================*/
void Radio_Mngr_App_Responsse_Inst_Hsm_Stop(Te_RADIO_ReplyStatus e_InstHSMReplystatus)
{
	Ts_Sys_Msg *msg;
	msg = MSG_Update(RADIO_MNGR_APP, RADIO_MNGR_INST_HSM_STOP_DONE);
	UpdateParameterIntoMsg(msg->data, &e_InstHSMReplystatus, sizeof(e_InstHSMReplystatus),&(msg->msg_length));
	
	SYS_SEND_MSG(msg);
}

/*=============================================================================================================================*/
/*StartupCheck(void)                               															   */
/*=============================================================================================================================*/
void StartupCheck(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr)
{
	/*Checking for Cold/Warm Startup*/
	if(pst_me_radio_mngr->st_inst_hsm.b_EEPROM_Status == EEPROM_UNKNOWN_VALUES)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Updated Default Values for Cold Start");
		
		Update_Default_Stations_Info(pst_me_radio_mngr);
		Update_Tunable_Station_Info(pst_me_radio_mngr);
	}
	else	
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Updated LSM info info Tunable Structure in Warm Start");
		
		Update_Tunable_Station_Info(pst_me_radio_mngr);
	}
}




/*=============================================================================================================================*/
/*Update_HMI_IF_Common_Data                                															   */
/*=============================================================================================================================*/

void Update_HMI_IF_Common_Data(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr)
{

        _radioCommonData.nBand = (MODE_TYPE)pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSM_Band;
		_radioCommonData.Aud_Band   = (MODE_TYPE)pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSM_Band;

		if(_radioCommonData.nBand == RADIO_MODE_AM)
		{
			_radioCommonData.Frequency = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq;
		}
		else if(_radioCommonData.nBand == RADIO_MODE_FM)
		{
			_radioCommonData.Frequency = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq;
		}
		else if(_radioCommonData.nBand == RADIO_MODE_DAB)
		{
			_radioCommonData.Frequency = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_DAB_Freq;
		}
		else{/*FOR MISRA C*/}

}


/*=============================================================================================================================*/
/*Update_Default_Stations_Info                                 															   */
/*=============================================================================================================================*/
void Update_Default_Stations_Info(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr)
{
	pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSM_Band	  = (Tu8)RADIO_MNGR_APP_BAND_FM;
	switch(pst_me_radio_mngr->e_Market)
	{
		case RCE_MARKET_WESTERN_EUROPE_RM:
		{
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq    = RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_87500;
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq    = RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_531;
		}
		break;

		case RCE_MARKET_ASIA_CHINA_RM:
		case RCE_MARKET_ARABIA_RM:
		case RCE_MARKET_KOREA_RM:
		{
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq    = RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_87500;
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq	  = RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_531;
		}
		break;

		case RCE_MARKET_LATIN_AMERICA_RM:
		{
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq    = RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_87500;
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq	  = RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_530;
		}
		break;

		case RCE_MARKET_USA_NORTHAMERICA_RM:
		{
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq    = RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_87900;
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq	  = RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_530;
		}
		break;

		case RCE_MARKET_JAPAN_RM:
		{
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq    = RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_76000;
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq	  = RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_522;
		}
		break;

		case RCE_MARKET_BRAZIL_RM:
		case RCE_MARKET_SOUTHAMERICA_RM:
		{
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq    = RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_76000;
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq	  = RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_530;
		}
		break;

		case RCE_MARKET_INVALID_RM:
		{
			/*FOR MISRA C*/	
		}
		break;
		
		default:
		{
		}
		break;
	}
	pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u16_PI         = RADIO_MNGR_APP_VALUE_ZERO;
	pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_DAB_Freq   = RADIO_MNGR_APP_DEFAULT_DAB_FREQUENCY;
	pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u16_EId        = RADIO_MNGR_APP_VALUE_ZERO;
	pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_Sid        = RADIO_MNGR_APP_VALUE_ZERO;
	pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u16_SCIdI      = RADIO_MNGR_APP_VALUE_ZERO;	
}


/*=============================================================================================================================*/
/*Update_Tunable_Station_Info                             															   */
/*=============================================================================================================================*/
void Update_Tunable_Station_Info(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr)
{
	pst_me_radio_mngr->st_inst_hsm.st_Tunable_Station_Info.u32_AM_Freq  = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_AM_Freq;
	pst_me_radio_mngr->st_inst_hsm.st_Tunable_Station_Info.u32_FM_Freq  = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_FM_Freq;
	pst_me_radio_mngr->st_inst_hsm.st_Tunable_Station_Info.u16_PI		= pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u16_PI;
	pst_me_radio_mngr->st_inst_hsm.st_Tunable_Station_Info.u32_DAB_Freq = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_DAB_Freq;
	pst_me_radio_mngr->st_inst_hsm.st_Tunable_Station_Info.u16_EId      = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u16_EId;
	pst_me_radio_mngr->st_inst_hsm.st_Tunable_Station_Info.u32_Sid      = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u32_Sid;
	pst_me_radio_mngr->st_inst_hsm.st_Tunable_Station_Info.u16_SCIdI    = pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u16_SCIdI;
}

/*=============================================================================================================================*/
/*Update_LSM_with_CurrentStationInfo                             															   */
/*=============================================================================================================================*/
void Update_LSM_TunableStn_with_CurrentStationInfo(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst)
{
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_AM_Freq		= pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq = pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq;
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq		= pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_PI			= pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI;

			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq = pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI      = pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI;
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_DAB_Freq	 = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_EId			 = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_EId;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_Sid			 = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_SId;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_SCIdI		 = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_SCIdI;

			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId		 = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_EId;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid      = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_SId;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI	 = pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_SCIdI;
		}
		break;
		
		default:
		{
		
		}
		break;
	}
}

/*=============================================================================================================================*/
/* Update_LSM_Index(void)                                  															   */
/*=============================================================================================================================*/
void Update_StationInfo_with_index(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band, Tu8 u8_Index)
{
	/*updating Tunable station info structure with the requested index station info*/
	switch(e_Band)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq = pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.ast_Stations[u8_Index].u32_Freq;
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq = pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_Index].u32_frequency;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI      = pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_Index].u16_PI;
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_Index].u32_Frequency;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId		 = pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_Index].u16_EId;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid      = pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_Index].u32_Sid;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI	 = pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_Index].u16_SCIdI;
		}
		break;
		
		default:
		{
		
		}
		break;
	}
}


/*=============================================================================================================================*/
/* Update_Tuned_Station_Info                                															   */
/*=============================================================================================================================*/
void Update_LSM_Station_Info(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst)
{
	/*updating Tuned station info structure with the current station info*/
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_AM_Freq = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq;
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_PI      = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI;
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_DAB_Freq = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_EId		 = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_Sid      = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid;
			pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_SCIdI    = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI;
		}
		break;
		
		default:
		{
		
		}
		break;
	}
}

/*=============================================================================================================================*/
/*Update_Tunable_Station_Info                             															   */
/*=============================================================================================================================*/
void Update_Tunable_Station_Info_with_LSM(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst)
{
	if(pst_me_radio_mngr_inst != NULL)
	{
		switch(pst_me_radio_mngr_inst->e_activeBand)
		{
			case RADIO_MNGR_APP_BAND_AM:
			{
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq  = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_AM_Freq;
			}
			break;

			case RADIO_MNGR_APP_BAND_FM:
			{
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq  = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq;
			}
			break;

			case RADIO_MNGR_APP_BAND_DAB:
			{	
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_DAB_Freq;
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId      = pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_EId;
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid      = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_Sid;
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI    = pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_SCIdI;
			}
			break;

			default:
			{
			}			
			break;
		}
	}
	else{/*FOR MISRA C*/}
}
/*=============================================================================================================================*/
/* Update_CurrentStationInfo_with_TunableStn                                															   */
/*=============================================================================================================================*/
void Update_CurrentStationInfo_with_TunableStn(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst)
{
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq;
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq;
			pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI 		= pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI; 
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			/*pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq;
			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_EId = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId;
			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_SId = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid;
			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_SCIdI = pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI;*/
		}
		break;

		default:
		{
		
		}
		break;
	}
}

/*======================================================================================================================*/
/*  Update_Radio_Mngr_StationList                                             													    */
/*======================================================================================================================*/
void Update_Radio_Mngr_StationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band)
{
	
	switch(e_Band)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			/*Clear all the Stations in Station List*/
			memset(&(pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList), RADIO_MNGR_APP_VALUE_ZERO, sizeof(Ts_Radio_Mngr_App_AM_SL));

			SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList), &(st_am_station_list), sizeof(Ts_Radio_Mngr_App_AM_SL));
		}
		break;
		
		case RADIO_MNGR_APP_BAND_FM:
		{
			/*Clear all the Stations in Station List*/
			memset(&(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList), RADIO_MNGR_APP_VALUE_ZERO, sizeof(Ts_Radio_Mngr_App_FM_SL));
		
			/* For Diag Request*/
			SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList), &(st_fm_station_list), (sizeof(Ts_Radio_Mngr_App_FM_SL)));
		}
		break;
		
		case RADIO_MNGR_APP_BAND_DAB:
		{
			/*Clear all the Stations in Station List*/
			memset(&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList), RADIO_MNGR_APP_VALUE_ZERO, sizeof(Ts_Radio_Mngr_App_DAB_SL));
			memset(&(pst_me_radio_mngr_inst->st_DABEnsembleList), 					  RADIO_MNGR_APP_VALUE_ZERO, sizeof(Ts_Radio_Mngr_App_DAB_MultiplexStationList));
			
			/*Copying from DAB APP stl to RM STL after scan completed*/
			SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList), &(st_DAB_App_StationList), (sizeof(Ts_Radio_Mngr_App_DAB_SL)));
			SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_DABEnsembleList), &(st_DAB_App_MultiplexStationList), sizeof(Ts_Radio_Mngr_App_DAB_MultiplexStationList));
		}
		break;
		
		default:
		{
			
		}
		break;
	}
}


/*======================================================================================================================*/
/*  Update_ComponentName		                                 													    */
/*======================================================================================================================*/
void Update_ComponentName(Ts_Radio_Mngr_App_DAB_CurrentStationInfo* st_DAB_CurrentStationInfo, Ts_Radio_Mngr_App_ComponentName* st_ComponentName)
{
	Tu8 	u8_char_loc 				=	RADIO_MNGR_APP_VALUE_ZERO;
	Tu8 	u8_dest_loc 				=	RADIO_MNGR_APP_VALUE_ZERO;
	Ts32	s32_StringCompare_RetValue 	=	RADIO_MNGR_APP_VALUE_ZERO;	
	
	/*Clearing the Component Label buffer before clearing in order to avoid copying of Garbage data*/
	memset(st_ComponentName->au8_CompLabel, 0, RADIO_MNGR_APP_COMPONENT_LABEL);
	
	/*String comparison function to check if both Service and service component Labels are same then no need to merge*/
	s32_StringCompare_RetValue = SYS_RADIO_STR_CMP((Tu8*)(st_DAB_CurrentStationInfo->st_ServiceLabel.au8_Label), 
														(st_DAB_CurrentStationInfo->st_ComponentLabel.au8_Label),
														RADIO_MNGR_APP_NUMCHAR_LABEL);
	
	/*Comparing the return value of SYS_RADIO_STR_CMP function*/														
	if(s32_StringCompare_RetValue == RADIO_MNGR_APP_VALUE_ZERO)
	{
		/*Apending the radio manager component name into service component label*/
		SYS_RADIO_MEMCPY((st_ComponentName->au8_CompLabel),(st_DAB_CurrentStationInfo->st_ServiceLabel.au8_Label),
								RADIO_MNGR_APP_NUMCHAR_LABEL);
	}
	
	else if(s32_StringCompare_RetValue != RADIO_MNGR_APP_VALUE_ZERO)
	{
		/*Loop for Copying the first three characters from the service label without any condition check*/
		for(u8_char_loc = 0; u8_char_loc <= 2; u8_char_loc++)
		{
				st_ComponentName->au8_CompLabel[u8_dest_loc] = st_DAB_CurrentStationInfo->st_ServiceLabel.au8_Label[u8_char_loc];
				u8_dest_loc++;
		}
	
		for(u8_char_loc = 3; u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL; u8_char_loc++)
		{
			if(st_DAB_CurrentStationInfo->st_ServiceLabel.au8_Label[u8_char_loc] != RADIO_MNGR_APP_ASCII_SPACE)
			{
	            /*copying the characters from the service label if the char is not space*/
				st_ComponentName->au8_CompLabel[u8_dest_loc] = st_DAB_CurrentStationInfo->st_ServiceLabel.au8_Label[u8_char_loc];
				u8_dest_loc++;
			}
			else
			{
				if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL)
				{
					u8_char_loc++;
				}
				else
				{
					/*FOR MISRA C*/
				}
				if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL && st_DAB_CurrentStationInfo->st_ServiceLabel.au8_Label[u8_char_loc] == RADIO_MNGR_APP_ASCII_SPACE)
				{
					u8_char_loc--;
					break;
				}else{/*FOR MISRA C*/}

				u8_char_loc--;
	            /*copying the characters from the service label if the second char is not space*/
				st_ComponentName->au8_CompLabel[u8_dest_loc] = st_DAB_CurrentStationInfo->st_ServiceLabel.au8_Label[u8_char_loc];
				u8_dest_loc++;
			}
		}
    	/*Apending the service component label into radio manager component name*/
		SYS_RADIO_MEMCPY(&(st_ComponentName->au8_CompLabel[u8_dest_loc]),(st_DAB_CurrentStationInfo->st_ComponentLabel.au8_Label),
							RADIO_MNGR_APP_NUMCHAR_LABEL);	
	}
	else{/*FOR MISRA C*/}
	st_ComponentName->u8_CharSet = st_DAB_CurrentStationInfo->st_ServiceLabel.u8_CharSet ;
}

/*======================================================================================================================*/
/*  Preset_Store_with_Current_Station		                                 													    */
/*======================================================================================================================*/
void Preset_Store_with_Current_Station(Tu8 u8_Preset_Store_Index)
{
    /*Checking the condition if the preset store index band is invalid only then increment number of preset list station*/
	if(st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].e_Band == RADIO_MNGR_APP_BAND_INVALID)
	{
		st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.u8_NumPresetList++;
	}else{/*FOR MISRA C*/}

	st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].e_Band	= st_radio_mngr_app_hsm.st_inst_hsm.e_activeBand;

	switch(st_radio_mngr_app_hsm.st_inst_hsm.e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_AMStnInfo.u32_Freq = 
					st_radio_mngr_app_hsm.st_inst_hsm.st_AMFM_currentstationinfo.un_station.st_AMCurrentStationInfo.u32_Freq;
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency   = 
							st_radio_mngr_app_hsm.st_inst_hsm.st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency;

			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_FMStnInfo.u16_PI		  = 
							st_radio_mngr_app_hsm.st_inst_hsm.st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI;

			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u8_CharSet								  = st_radio_mngr_app_hsm.st_inst_hsm.st_AMFM_currentstationinfo.u8_CharSet;
			
			SYS_RADIO_MEMCPY((st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_FMStnInfo.au8_PSN), 
								(st_radio_mngr_app_hsm.st_inst_hsm.st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN), 
								sizeof(st_radio_mngr_app_hsm.st_inst_hsm.st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN));
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			/*Copying Channel Name for DAB Station preset Store to show on HMI while signal/Service Label is not present*/
			SYS_RADIO_MEMCPY((st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_DABStnInfo.au8_ChannelName), 
								(st_radio_mngr_app_hsm.st_inst_hsm.st_DAB_currentstationinfo.au8_ChannelName), sizeof(st_radio_mngr_app_hsm.st_inst_hsm.st_DAB_currentstationinfo.au8_ChannelName));
			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_DABStnInfo.u32_Frequency  = st_radio_mngr_app_hsm.st_inst_hsm.st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency;
			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_DABStnInfo.u16_EId	      = st_radio_mngr_app_hsm.st_inst_hsm.st_DAB_currentstationinfo.st_Tunableinfo.u16_EId;
			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_DABStnInfo.u32_Sid	      = st_radio_mngr_app_hsm.st_inst_hsm.st_DAB_currentstationinfo.st_Tunableinfo.u32_SId;
			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_DABStnInfo.u16_SCIdI	  = st_radio_mngr_app_hsm.st_inst_hsm.st_DAB_currentstationinfo.st_Tunableinfo.u16_SCIdI;

			st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u8_CharSet								   = st_radio_mngr_app_hsm.st_inst_hsm.st_CurrentStationName.u8_CharSet;

			SYS_RADIO_MEMCPY(&(st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_Preset_Store_Index].u_PresetStInfo.st_DABStnInfo.st_ComponentName), 
								&(st_radio_mngr_app_hsm.st_inst_hsm.st_CurrentStationName), sizeof(st_radio_mngr_app_hsm.st_inst_hsm.st_CurrentStationName));
		}	
		break;

		default:
		{
		}
		break;
	}
	
	/*Function to update Flash Memory parameters*/
	Radio_Manager_App_Write_Flash_Data(&(st_radio_mngr_app_hsm.st_inst_hsm));

    /*Notifying the Preset Store Response to HMI-IF*/
	Radio_Mngr_App_Response_PresetStore(&(st_radio_mngr_app_hsm.st_inst_hsm), REPLYSTATUS_SUCCESS);
}

/*======================================================================================================================*/
/*  Update_Tunable_Station_with_Preset_Index		                                 													    */
/*======================================================================================================================*/
void Update_Tunable_Station_with_Preset_Index(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Tu8 u8_Preset_Recall_Index)
{
	if(pst_me_radio_mngr_inst != NULL)
	{
			switch(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].e_Band)
			{
				case RADIO_MNGR_APP_BAND_AM:
				{
					pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq = pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].u_PresetStInfo.st_AMStnInfo.u32_Freq;
				}
				break;

				case RADIO_MNGR_APP_BAND_FM:
				{
					pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq = pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency;
					pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI		= pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.u16_PI;
				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{
					pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].u_PresetStInfo.st_DABStnInfo.u32_Frequency;
					pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId		 = pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].u_PresetStInfo.st_DABStnInfo.u16_EId;
					pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid		 = pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].u_PresetStInfo.st_DABStnInfo.u32_Sid;
					pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI	 = pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Recall_Index].u_PresetStInfo.st_DABStnInfo.u16_SCIdI;
				}
				break;

				default:
				{

				}
				break;
			}
	}
	else
	{
		/*FOR MISRA C*/
	}
}

/*======================================================================================================================*/
/*  Update_TunuableStation_with_DAB_Station		                                 													    */
/*======================================================================================================================*/
void Update_TunuableStation_with_DAB_Station(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = pst_me_radio_mngr_inst->st_DABTunableStation.u32_Frequency;
	pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId		 = pst_me_radio_mngr_inst->st_DABTunableStation.u16_EId;
	pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid		 = pst_me_radio_mngr_inst->st_DABTunableStation.u32_SId;
	pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI	 = pst_me_radio_mngr_inst->st_DABTunableStation.u16_SCIdI;
}

/*======================================================================================================================*/
/*  Radio_Mngr_Update_OriginalStn		                                 													    */
/*======================================================================================================================*/
void Radio_Mngr_Update_OriginalStn(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Tunning to Original Station for Band: %d", e_Band);
	pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
	switch(e_Band)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_AM_Freq;
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq;
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_DAB_Freq;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId      = pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_EId;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid      = pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_Sid;
			pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI    = pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_SCIdI;
		}
		break;

		default:
		{
		}
		break;
	}
}		

/*======================================================================================================================*/
/*  Update_Settings		                                 																	    */
/*======================================================================================================================*/
void Update_Settings(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Te_Radio_Mngr_App_Market e_Market)
{
	/*1.1.1.1.1.1 -> Multiplex(on).Info_Anno(on).Regional settings(on).RDS(on).DABFM(on).TA_Anno(on)*/
	switch(e_Market)
	{
		case RCE_MARKET_WESTERN_EUROPE_RM:
		{
			if(pst_me_radio_mngr->st_inst_hsm.u8_StartType != RADIO_MNGR_APP_WARM_START)
			{
				/*0X1F -> 1.1.1.1.1 -> Info_Anno(on).Regional settings(on).RDS(on).DABFM(on).TA_Anno(on)*/
				pst_me_radio_mngr->st_inst_hsm.u8_Settings 				 = 0X1F;
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X1F;
			}
			else
			{
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X1F;
			}
		}
		break;

		case RCE_MARKET_LATIN_AMERICA_RM:
		case RCE_MARKET_ASIA_CHINA_RM:
		case RCE_MARKET_SOUTHAMERICA_RM:
		{
			
			if(pst_me_radio_mngr->st_inst_hsm.u8_StartType != RADIO_MNGR_APP_WARM_START)
			{
				/*0X19 -> 1.1.0.0.1 -> Info_Anno(on).Regional settings(on).RDS(off).DABFM(off).TA_Anno(on)*/
				pst_me_radio_mngr->st_inst_hsm.u8_Settings 				 = 0X19;
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X19;
			}
			else
			{
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X19;
			}
		}
		break;

		case RCE_MARKET_JAPAN_RM:
		{				
			if(pst_me_radio_mngr->st_inst_hsm.u8_StartType != RADIO_MNGR_APP_WARM_START)
			{
				/*0X08 -> 0.1.0.0.0 -> Info_Anno(off).Regional settings(on).RDS(off).DABFM(off).TA_Anno(off)*/
				pst_me_radio_mngr->st_inst_hsm.u8_Settings 				 = 0X08;
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X08;
			}
			else
			{
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X08;
			}
		}
		break;

		case RCE_MARKET_ARABIA_RM:
		case RCE_MARKET_USA_NORTHAMERICA_RM:
		case RCE_MARKET_KOREA_RM:
		case RCE_MARKET_BRAZIL_RM:
		{
			
			if(pst_me_radio_mngr->st_inst_hsm.u8_StartType != RADIO_MNGR_APP_WARM_START)
			{
				/*0X1D -> 1.1.1.0.1 -> Info_Anno(on).Regional settings(on).RDS(on).DABFM(off).TA_Anno(on)*/
				pst_me_radio_mngr->st_inst_hsm.u8_Settings 				 = 0X1D;
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X1D;
			}
			else
			{
				pst_me_radio_mngr->st_inst_hsm.u8_SettingsStateWithMarket = 0X1D;
			}
		}
		break;

		default:
		break;
	}

	/*TA Announcement Update*/
	if(LIB_ISBITSET(pst_me_radio_mngr->st_inst_hsm.u8_Settings, 0u))
	{
		pst_me_radio_mngr->st_inst_hsm.e_TA_Anno_Switch = RADIO_MNGR_APP_TA_ANNO_ENABLE;
	}
	else
	{
		pst_me_radio_mngr->st_inst_hsm.e_TA_Anno_Switch = RADIO_MNGR_APP_TA_ANNO_DISABLE;
	}

	/*DABFM Following Update*/
	if(LIB_ISBITSET(pst_me_radio_mngr->st_inst_hsm.u8_Settings, 1u))
	{
		pst_me_radio_mngr->st_inst_hsm.e_DABFMLinking_Switch = RADIO_MNGR_APP_DABFMLINKING_ENABLE;
	}
	else
	{
		pst_me_radio_mngr->st_inst_hsm.e_DABFMLinking_Switch = RADIO_MNGR_APP_DABFMLINKING_DISABLE;
	}

	/* RDS/AF Following Update*/
	if(LIB_ISBITSET(pst_me_radio_mngr->st_inst_hsm.u8_Settings, 2u))
	{
		pst_me_radio_mngr->st_inst_hsm.e_RDSSettings = RADIO_MNGR_APP_RDS_SETTINGS_ENABLE;
	}
	else
	{
		pst_me_radio_mngr->st_inst_hsm.e_RDSSettings = RADIO_MNGR_APP_RDS_SETTINGS_DISABLE;
	}
	
	/*Info Announcement Update*/
	if(LIB_ISBITSET(pst_me_radio_mngr->st_inst_hsm.u8_Settings, 4u))
	{
		pst_me_radio_mngr->st_inst_hsm.e_Info_Anno_Switch = RADIO_MNGR_APP_INFO_ANNO_ENABLE;
	}
	else
	{
		pst_me_radio_mngr->st_inst_hsm.e_Info_Anno_Switch = RADIO_MNGR_APP_INFO_ANNO_DISABLE;
	}
	
	
	/*Multiplex Update*/
	if(pst_me_radio_mngr->st_inst_hsm.u8_StartType != RADIO_MNGR_APP_WARM_START)
	{
		pst_me_radio_mngr->st_inst_hsm.u8_Settings = LIB_CLEARBIT(pst_me_radio_mngr->st_inst_hsm.u8_Settings, 5u);
	}
	else
	{
		/*Do Nothing*/
	}
		
	if(LIB_ISBITSET(pst_me_radio_mngr->st_inst_hsm.u8_Settings, 5u))
	{
		pst_me_radio_mngr->st_inst_hsm.e_MultiplexSettings = RADIO_MNGR_APP_MULTIPLEX_ENABLE;
	}
	else
	{
		pst_me_radio_mngr->st_inst_hsm.e_MultiplexSettings = RADIO_MNGR_APP_MULTIPLEX_DISABLE;
	}	
}		

/*======================================================================================================================*/
/*  Update_MatchedStationListIndex		                                 													    */
/*======================================================================================================================*/
void Update_MatchedStationListIndex(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Tu8 u8_StL_Index;
	pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index       = RADIO_MNGR_APP_TU8_MAX_VALUE;

	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			/*In Station List Comparison*/
			for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList; u8_StL_Index++)
			{
				if(pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_AM_Freq == pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.ast_Stations[u8_StL_Index].u32_Freq)
				{
					pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index = u8_StL_Index;
					break;
				}
				else{/*FOR MISRA C*/}
			}
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH)
			{
				/*In search Station List Comparison*/
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList; u8_StL_Index++)
				{
					if(pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq == pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.au32_frequency[u8_StL_Index])
					{
						pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index = u8_StL_Index;
						break;
					}
					else{/*FOR MISRA C*/}
				}
			}
			else if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH)
			{
				/*In search Station List Comparison*/
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList; u8_StL_Index++)
				{
					if(pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq == pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[u8_StL_Index].u32_frequency)
					{
						pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index = u8_StL_Index;
						break;
					}
					else{/*FOR MISRA C*/}
				}	
			}
			else
			{
				/*In Station List Comparison*/
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList; u8_StL_Index++)
				{
					if(pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq == pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].u32_frequency)
					{
						pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index = u8_StL_Index;
						break;
					}
					else{/*FOR MISRA C*/}
				}
			}
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_DAB_STL_SEARCH)
			{
				/*In search Station List Comparison*/
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList; u8_StL_Index++)
				{
					if((pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_Sid == pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[u8_StL_Index].u32_Sid) && 
							(pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_SCIdI == pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[u8_StL_Index].u16_SCIdI))
					{
						pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index = u8_StL_Index;
						break;
					}
					else{/*FOR MISRA C*/}
				}	
			}
			
			else if(pst_me_radio_mngr_inst->e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_DISABLE)
			{
				/*In Normal Station List Comparison*/
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList; u8_StL_Index++)
				{
					if((pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_Sid == pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].u32_Sid) && 
							(pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_SCIdI == pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].u16_SCIdI))
					{
						pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index = u8_StL_Index;
						break;
					}
					else{/*FOR MISRA C*/}
				}
			}
			
			else if(pst_me_radio_mngr_inst->e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_ENABLE)
			{
				/*In Multiplex Station List Comparison*/
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble; u8_StL_Index++)
				{
					if((pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_Sid == 
							pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].ast_MultiplexStationInfo[u8_StL_Index].u32_Sid) && 
							(pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_SCIdI == 
							pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].ast_MultiplexStationInfo[u8_StL_Index].u16_SCIdI))
					{
						pst_me_radio_mngr_inst->u8_MatchedStL_Stn_Index = u8_StL_Index;
						break;
					}
					else{/*FOR MISRA C*/}
				}
			}else{/*FOR MISRA C*/}
		}
		break;

		default:
		break;
	}
}


/*======================================================================================================================*/
/*  Radio_Mngr_App_GetPresetList		                                 													    */
/*======================================================================================================================*/
void Radio_Mngr_App_GetPresetList(void)
{
    /*Sending response for Preset Recall to HMI-IF*/
	Radio_Mngr_App_Response_PresetList(&(st_radio_mngr_app_hsm.st_inst_hsm.st_PrestMixedList));
}

/*======================================================================================================================*/
void Radio_Mngr_App_ClearCheckParameters(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
	{
		/*Updating Strategy end in RM, FM and DAB*/
		Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END);
	}else{/*FOR MISRA C*/}

	pst_me_radio_mngr_inst->e_LearnAFStatus				   = RADIO_MNGR_APP_LEARN_MEM_AF_INVALID;
	pst_me_radio_mngr_inst->e_FM_AFTuneReplyStatus		   = REPLYSTATUS_INVALID_PARAM;
	pst_me_radio_mngr_inst->e_DAB_AFTuneReplyStatus 	   = REPLYSTATUS_INVALID_PARAM;
	pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus  = REPLYSTATUS_INVALID_PARAM;
	pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus = REPLYSTATUS_INVALID_PARAM;
	pst_me_radio_mngr_inst->e_StrategyFlow 				   = RM_STRATEGY_FLOW_INVALID;
}

/*======================================================================================================================*/
/*  Radio_Mngr_App_SelectBandConditionCheck		                                 													    */
/*======================================================================================================================*/
Tu8 Radio_Mngr_App_SelectBandConditionCheck(Te_Radio_Mngr_App_Band e_requestedBand, Te_Radio_Mngr_App_Band e_activeBand, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Tu8 u8_ret_value = RADIO_MNGR_APP_VALUE_ZERO;
	
	if(e_activeBand == RADIO_MNGR_APP_BAND_DAB)
	{
		switch(e_requestedBand)
		{
			case RADIO_MNGR_APP_BAND_FM:
			case RADIO_MNGR_APP_NON_RADIO_MODE:
			{
				u8_ret_value = RADIO_MNGR_APP_DAB_CANCEL;
			}
			break;
			
			case RADIO_MNGR_APP_BAND_AM:
			{
				if(pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
				{
					u8_ret_value = RADIO_MNGR_APP_DAB_CANCEL;
				}
			}
			break;
			
			default:
			break;
		}
	}
	else if(e_activeBand == RADIO_MNGR_APP_BAND_FM)
	{
		switch(e_requestedBand)
		{
			case RADIO_MNGR_APP_NON_RADIO_MODE:
			{
				u8_ret_value = RADIO_MNGR_APP_AMFM_CANCEL;
			}
			break;
			
			case RADIO_MNGR_APP_BAND_AM:
			{
				if(pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
				{
					u8_ret_value = RADIO_MNGR_APP_AMFM_CANCEL;
				}
			}
			break;
			
			case RADIO_MNGR_APP_BAND_DAB:
			{
				if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL && pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED  
					&& pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
				{
					u8_ret_value = RADIO_MNGR_APP_AMFM_CANCEL;
				}
			}
			break;
			
			default:
			break;
		}
	}
	else if(e_activeBand == RADIO_MNGR_APP_BAND_AM)
	{
		switch(e_requestedBand)
		{
			case RADIO_MNGR_APP_BAND_FM:
			case RADIO_MNGR_APP_NON_RADIO_MODE:
			{
				u8_ret_value = RADIO_MNGR_APP_AMFM_CANCEL;
			}
			break;
			
			case RADIO_MNGR_APP_BAND_DAB:
			{
				if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL && pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED  
					&& pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
				{
					u8_ret_value = RADIO_MNGR_APP_AMFM_CANCEL;
				}
			}
			break;
			
			default:
			break;
		}
	}
	
	return 	u8_ret_value;
}

/*======================================================================================================================*/
/*  Radio_Mngr_App_PresetRecallConditionCheck		                                 													    */
/*======================================================================================================================*/
Tbool Radio_Mngr_App_PresetRecallConditionCheck(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{	
	Tbool b_ret_variable = RADIO_MNGR_APP_VALUE_ZERO;
	
	if(pst_me_radio_mngr_inst->st_PrestMixedList.u8_NumPresetList != RADIO_MNGR_APP_VALUE_ZERO &&
		(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band == RADIO_MNGR_APP_BAND_DAB || 
		 pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band == RADIO_MNGR_APP_BAND_FM ||
		 (pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)))
	{
		if((pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band != RADIO_MNGR_APP_BAND_DAB && 
		                 pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB) ||
			    	(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band != RADIO_MNGR_APP_BAND_DAB && 
					     pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB) ||
					(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band == RADIO_MNGR_APP_BAND_DAB && 
					     pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL) ||
					(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band == RADIO_MNGR_APP_BAND_DAB && 
					     pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL))
		{
			b_ret_variable = RADIO_APP_VALUE_NONZERO;
		}else{/*FOR MISRA C*/}

	}
	else{/*FOR MISRA C*/}
	
	return b_ret_variable;
}

/*======================================================================================================================*/
/*  Radio_Manager_EEPROM_Log		                                 									*/
/*======================================================================================================================*/
void Radio_Manager_EEPROM_Log(Tu8 u8_EEPROM_Write_Status)
{
	if(!(u8_EEPROM_Write_Status))
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]EEPROM Write Success");
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[Radio][RM]EEPROM Write Failed");
	}
}

/*======================================================================================================================*/
/*  Radio_Mngr_App_GetRadioStationList		                                 													    */
/*======================================================================================================================*/
Tu8 Radio_Mngr_App_GetRadioStationList(void)
{
	Tu8 index = 0;
	Tu8 Preset_In_STLMatchindex = 0;
	Ts32 s32_StringCompare_RetValue 	= RADIO_MNGR_APP_VALUE_ZERO;	
	Tu8  u8_char_loc;
	Tu8  u8_dest_loc ;
	st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.e_STL_Search_Type = RADIO_MNGR_APP_STL_SEARCH_INVALID;
	st_radio_mngr_app_hsm.st_inst_hsm.u8_EnsembleSelect_Req_Status   = RADIO_MNGR_APP_VALUE_ZERO;
	if((st_radio_mngr_app_hsm.st_inst_hsm.e_activeBand == RADIO_MNGR_APP_BAND_FM || st_radio_mngr_app_hsm.st_inst_hsm.e_activeBand == RADIO_MNGR_APP_BAND_AM) || 
			(st_radio_mngr_app_hsm.st_inst_hsm.e_activeBand == RADIO_MNGR_APP_BAND_DAB && st_radio_mngr_app_hsm.st_inst_hsm.e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_DISABLE))
	{
		Update_MatchedStationListIndex(&(st_radio_mngr_app_hsm.st_inst_hsm));
		memset(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index, RADIO_MNGR_APP_TU8_MAX_VALUE, RADIO_MNGR_APP_MAX_PSML_SIZE);
		Update_MatchedPresetInStationListIndex(&(st_radio_mngr_app_hsm.st_inst_hsm));
	}
	else
	{
		/*FOR MISRA C*/
	}
	switch(st_radio_mngr_app_hsm.st_inst_hsm.e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
	    {
			noStationsDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_RadioStationList.st_AM_StationList.u8_numberStationsInList;
			for(index = 0; index < noStationsDisplay; index++)
			{
				_radioStationListData_Display[index].nBand = (MODE_TYPE)RADIO_MNGR_APP_BAND_AM;		    
				_radioStationListData_Display[index].Index = index;
				_radioStationListData_Display[index].Frequency = st_radio_mngr_app_hsm.st_inst_hsm.st_RadioStationList.st_AM_StationList.ast_Stations[index].u32_Freq;
				_radioStationListData_Display[index].Char_set = 0;
				memset((void *)_radioStationListData_Display[index].ServiceName, (Tu16)0 , sizeof(_radioStationListData_Display[index].ServiceName));
				
				if(st_radio_mngr_app_hsm.st_inst_hsm.u8_MatchedStL_Stn_Index == index)
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL;
				}
				else
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL;
				}
				
				if(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index[Preset_In_STLMatchindex] == index)
				{
					_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_MATCHED_PRESET_IN_STL;
					Preset_In_STLMatchindex++;	
				}
				else
				{
					_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL;
				}
			}
			st_radio_mngr_app_hsm.st_inst_hsm.u8_GetStl_Ret_Value = RADIO_MNGR_NORMAL_STATIONLIST;
		}
		break;
		
		case RADIO_MNGR_APP_BAND_FM:
		{
			noStationsDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_RadioStationList.st_FM_StationList.u8_numberStationsInList;
			for(index = 0; index < noStationsDisplay; index++)
			{
				_radioStationListData_Display[index].nBand = (MODE_TYPE)RADIO_MNGR_APP_BAND_FM;		    
				_radioStationListData_Display[index].Index = index;
				_radioStationListData_Display[index].Frequency = st_radio_mngr_app_hsm.st_inst_hsm.st_RadioStationList.st_FM_StationList.ast_Stations[index].u32_frequency;
				_radioStationListData_Display[index].Char_set  = st_radio_mngr_app_hsm.st_inst_hsm.st_RadioStationList.st_FM_StationList.u8_CharSet;
				memset((void *)(_radioStationListData_Display[index].ServiceName),(Tu16)0,sizeof(_radioStationListData_Display[index].ServiceName));
				SYS_RADIO_MEMCPY((void *)(_radioStationListData_Display[index].ServiceName),
									(const void*)(st_radio_mngr_app_hsm.st_inst_hsm.st_RadioStationList.st_FM_StationList.ast_Stations[index].au8_PSN),RADIO_MNGR_APP_CHAN_NAME);
		    
				if(st_radio_mngr_app_hsm.st_inst_hsm.u8_MatchedStL_Stn_Index == index)
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL;
				}
				else
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL;
				}
				
				if(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index[Preset_In_STLMatchindex] == index)
				{
					_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_MATCHED_PRESET_IN_STL;
					Preset_In_STLMatchindex++;	
				}
				else
				{
					_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL;
				}			
			}
			st_radio_mngr_app_hsm.st_inst_hsm.u8_GetStl_Ret_Value = RADIO_MNGR_NORMAL_STATIONLIST;
		}
		break;
		
	    case RADIO_MNGR_APP_BAND_DAB:
	    {
			if(st_radio_mngr_app_hsm.st_inst_hsm.e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_DISABLE)
			{	
				noStationsDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.u8_numberStationsInList;			
				for(index = 0; index < noStationsDisplay; index++)
				{
		    		_radioStationListData_Display[index].nBand = (MODE_TYPE)RADIO_MNGR_APP_BAND_DAB;
					_radioStationListData_Display[index].Index = index;
					_radioStationListData_Display[index].Frequency = st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].u32_Frequency;
					_radioStationListData_Display[index].Char_set = st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].u8_CharSet;
					memset((void *)(_radioStationListData_Display[index].ServiceName),(Tu16)0, sizeof(_radioStationListData_Display[index].ServiceName));
				
					/*String comparison function to check if both Service and service component Labels are same then no need to merge*/
					s32_StringCompare_RetValue = SYS_RADIO_STR_CMP((Tu8*)(st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_SrvLabel), 
																		(st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_CompLabel),
																		RADIO_MNGR_APP_NUMCHAR_LABEL);
					
					
					/*Comparing the return value of SYS_RADIO_STR_CMP function*/														
					if(s32_StringCompare_RetValue == RADIO_MNGR_APP_VALUE_ZERO)
		
					{
						/*Apending the radio manager component name into service component label*/
						SYS_RADIO_MEMCPY((_radioStationListData_Display[index].ServiceName),
																		(st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_SrvLabel),
																		RADIO_MNGR_APP_NUMCHAR_LABEL);
					}
					else if(s32_StringCompare_RetValue != RADIO_MNGR_APP_VALUE_ZERO)
					{
						/*Loop for copying the first three characters from the service label without any condition*/
						for(u8_char_loc = 0, u8_dest_loc=0; u8_char_loc <= 2; u8_char_loc++)
						{
							_radioStationListData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_SrvLabel[u8_char_loc];
							u8_dest_loc++;
						}
				
						/* copying the service label & component label HMI IF*/			
						for(u8_char_loc =3,u8_dest_loc =3; u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL ; u8_char_loc++)
						{
							if( st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_SrvLabel[u8_char_loc] != RADIO_MNGR_APP_ASCII_SPACE)
							{
								_radioStationListData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_SrvLabel[u8_char_loc];
								u8_dest_loc++;
							}
							else
							{
								if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL)
								{
									u8_char_loc++;
								}
								else
								{
									/*FOR MISRA C*/
								}
								if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL && st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_SrvLabel[u8_char_loc] == RADIO_MNGR_APP_ASCII_SPACE)
								{
									u8_char_loc--;
									break;
								}else{/*FOR MISRA C*/}

								u8_char_loc--;
								/*copying the characters from the service label if the second char is not space*/
								_radioStationListData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_SrvLabel[u8_char_loc];
								u8_dest_loc++;
							}
						}
		
						/*Apending the service component label into radio manager component name*/
						SYS_RADIO_MEMCPY(&(_radioStationListData_Display[index].ServiceName[u8_dest_loc]),
																		(st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_CompLabel),
																		sizeof((st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.ast_Stations[index].au8_CompLabel)));	
					}
		
					if(st_radio_mngr_app_hsm.st_inst_hsm.u8_MatchedStL_Stn_Index == index)
					{
						_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL;
					}
					else
					{
						_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL;
					}
				
					if(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index[Preset_In_STLMatchindex] == index)
					{
						_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_MATCHED_PRESET_IN_STL;
						Preset_In_STLMatchindex++;	
					}
					else
					{
						_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL;
					}
				}
				st_radio_mngr_app_hsm.st_inst_hsm.u8_GetStl_Ret_Value = RADIO_MNGR_NORMAL_STATIONLIST;
			}
		    
			else if(st_radio_mngr_app_hsm.st_inst_hsm.e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_ENABLE)
			{
				memset((void *)(_radioEnsembleListData_Display),(Tu16)0,sizeof(_radioEnsembleListData_Display));
				noEnsembleListDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_DABEnsembleList.u8_NoOfEnsembleList;
				for(index = 0; index < noEnsembleListDisplay; index++)
				{
					SYS_RADIO_MEMCPY((void *)(_radioEnsembleListData_Display[index].EnsembleName),
										(const void*)(st_radio_mngr_app_hsm.st_inst_hsm.st_DABEnsembleList.ast_EnsembleInfo[index].au8_EnsembleLabel),
										sizeof(st_radio_mngr_app_hsm.st_inst_hsm.st_DABEnsembleList.ast_EnsembleInfo[index].au8_EnsembleLabel));
					_radioEnsembleListData_Display[index].Index		   = index;
					_radioEnsembleListData_Display[index].Char_set     = st_radio_mngr_app_hsm.st_inst_hsm.st_DABEnsembleList.ast_EnsembleInfo[index].u8_CharSet;	
				}
			st_radio_mngr_app_hsm.st_inst_hsm.u8_GetStl_Ret_Value = RADIO_MNGR_MULTIPLEX_STATIONLIST;
			}   
	    }
		break;
		
	    default:
	    {
		   
	    }
		break;		
	}
	 return st_radio_mngr_app_hsm.st_inst_hsm.u8_GetStl_Ret_Value;
}

/*======================================================================================================================*/
/*  Radio_Mngr_App_Timer_ClearCheckParameters		                                 													    */
/*======================================================================================================================*/
void Radio_Mngr_App_Timer_ClearCheckParameters(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	/*Checking whether timer is running or not*/
	if(pst_me_radio_mngr_inst->b_TimerFlag == RADIO_MNGR_APP_SET_TIMER_FLAG)
	{
		/*clearing timer flag as zero*/
		pst_me_radio_mngr_inst->b_TimerFlag  = RADIO_MNGR_APP_CLEAR_TIMER_FLAG;
		pst_me_radio_mngr_inst->b_PIDecode_TimerFlag = RADIO_MNGR_APP_VALUE_ZERO;
	
		if(st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid > 0)
		{
		 	if(SYS_StopTimer(st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid) != TRUE)
			{
				/*Nothing */
			}
			else
			{
				st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid = 0;
			}
		}else{/*FOR MISRA C*/}
		
		if(st_Radio_Mngr_App_TimerID.u32_PI_Decode_Timerid >0)
		{
			if(SYS_StopTimer(st_Radio_Mngr_App_TimerID.u32_PI_Decode_Timerid) != TRUE)
			{
				/*Nothing */
			}
			else
			{
				st_Radio_Mngr_App_TimerID.u32_PI_Decode_Timerid = 0;
			}	
		}else{/*FOR MISRA C*/}
		
	}else{/*FOR MISRA C*/}
}

/*======================================================================================================================*/
/*  Radio_Manager_App_Write_Flash_Data		                                 													    */
/*======================================================================================================================*/
void Radio_Manager_App_Write_Flash_Data(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{	
	/*Writing Flash parameters into Flash Memory for any update*/
	pst_me_radio_mngr_inst->u8_NVM_ReplyStatus = SYS_NVM_WRITE(NVM_ID_TUNER_RADIOMNGR_APP, &(pst_me_radio_mngr_inst->u8_Settings), 
		   												 (Tu32)RADIO_MNGR_APP_NVM_DATA_SIZE, &(pst_me_radio_mngr_inst->u32_NVM_Read_Write_Bytes));
	
	/*Printing Debug Logs for Flash Writing*/																
	if(!(pst_me_radio_mngr_inst->u8_NVM_ReplyStatus))
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FLASH Write Success");
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[Radio][RM]FLASH Write Failed");
	}	
}

/*======================================================================================================================*/
/*  Radio_Manager_App_Update_AppLayer_STL		                                 										*/
/*======================================================================================================================*/
void Radio_Manager_App_Update_AppLayer_STL(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr)
{
	SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

	/*Copying RM AM STL to AMFM application structure during warm startup*/
	SYS_RADIO_MEMCPY(&(st_am_station_list), &(pst_me_radio_mngr->st_inst_hsm.st_RadioStationList.st_AM_StationList), sizeof(Ts_Radio_Mngr_App_AM_SL));

	SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);
	
	SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

	/*Copying RM FM STL to AMFM application structure during warm startup*/
	SYS_RADIO_MEMCPY(&(st_fm_station_list), &(pst_me_radio_mngr->st_inst_hsm.st_RadioStationList.st_FM_StationList), (sizeof(Ts_Radio_Mngr_App_FM_SL)));
	
	SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);
	
	
	SYS_MUTEX_LOCK(STL_RM_DAB_APP);
	
	/*Copying RM DAB STL to DAB APP stl during warm startup*/
	SYS_RADIO_MEMCPY(&(st_DAB_App_StationList), &(pst_me_radio_mngr->st_inst_hsm.st_RadioStationList.st_DAB_StationList), (sizeof(Ts_Radio_Mngr_App_DAB_SL)));
	SYS_RADIO_MEMCPY(&(st_DAB_App_MultiplexStationList), &(pst_me_radio_mngr->st_inst_hsm.st_DABEnsembleList), (sizeof(Ts_Radio_Mngr_App_DAB_MultiplexStationList)));
	
	SYS_MUTEX_UNLOCK(STL_RM_DAB_APP);
	
	/*Creating Multiplex & Normal Stationlist*/
	Radio_Mngr_App_CreateNormalRadioStationList(&(pst_me_radio_mngr->st_inst_hsm));
	Radio_Mngr_App_CreateMultiplexRadioStationList(&(pst_me_radio_mngr->st_inst_hsm));

}

/*======================================================================================================================*/
/*  Radio_Mngr_App_AudioChange				                                 											*/
/*======================================================================================================================*/
void Radio_Mngr_App_AudioChange(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_ReqAudioChangeBand)
{
	if(pst_me_radio_mngr_inst->e_Curr_Audio_Band != e_ReqAudioChangeBand)
	{
		pst_me_radio_mngr_inst->e_Curr_Audio_Band = e_ReqAudioChangeBand;
		
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Audio Changed to: %d", e_ReqAudioChangeBand);
		
		Radio_Mngr_App_Notify_AudioSwitch(e_ReqAudioChangeBand);
	}else{/*FOR MISRA C*/}
}


/*======================================================================================================================*/
/*  Radio_Manager_App_Update_PresetMixedList_AFTune		                                 													    */
/*======================================================================================================================*/
void Radio_Manager_App_Update_PresetMixedList_AFTune(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Tu8 u8_Stn_Index  = 0;
	
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_FM:
		{
			/*Checking the current station info after FM AF Tune and if band of preset station is also FM*/
			for(u8_Stn_Index=0; u8_Stn_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Stn_Index++)
			{
				if(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].u_PresetStInfo.st_FMStnInfo.u16_PI
					&& pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].e_Band == RADIO_MNGR_APP_BAND_FM)
				{
					/*Updating the Frequency in Preset Mixed List with New AF Tuned Frequency since it is of higher quality*/
					pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency = pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency;
				}else{/*FOR MISRA C*/}
			}
		}
		break;
		
		case RADIO_MNGR_APP_BAND_DAB:
		{
			/*Checking the current station info after DAB-DAB Tune and if band of preset station is also DAB*/
			for(u8_Stn_Index=0; u8_Stn_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Stn_Index++)
			{
				if(pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_SId == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].u_PresetStInfo.st_DABStnInfo.u32_Sid
					&& pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].e_Band == RADIO_MNGR_APP_BAND_DAB)
				{
					/*Updating the Frequency,Sid,Eid,SCId in Preset Mixed List with New DAB-DAB Tuned Station Info*/
					pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].u_PresetStInfo.st_DABStnInfo.u32_Frequency 	= pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency;
					pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].u_PresetStInfo.st_DABStnInfo.u32_Sid 		= pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_SId;
					pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].u_PresetStInfo.st_DABStnInfo.u16_EId 		= pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_EId;
					pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Stn_Index].u_PresetStInfo.st_DABStnInfo.u16_SCIdI 		= pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_SCIdI;
				}else{/*FOR MISRA C*/}
			}
		}
		break;
		
		default:
	    {
		   
	    }
		break;
	}
}

/*======================================================================================================================*/
/*  Update_MatchedPresetInStationListIndex		                                 													    */
/*======================================================================================================================*/
void Update_MatchedPresetInStationListIndex(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Tu8 u8_StL_Index;
	Tu8 u8_Preset_Index;
	Tu8 u8_MatchedPresetInSTL_Index = 0;


	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_AM:
		{
			for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList; u8_StL_Index++)
			{
				for(u8_Preset_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_Preset_Index < RADIO_MNGR_APP_MAX_PSML_SIZE ; u8_Preset_Index++)
				{
					/*Finding active band station is present in preset list or not*/
					if(pst_me_radio_mngr_inst->e_activeBand == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].e_Band)
					{	
						if(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_AMStnInfo.u32_Freq == pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.ast_Stations[u8_StL_Index].u32_Freq)
						{
							pst_me_radio_mngr_inst->au8_MatchedPresetInSTL_Stn_Index[u8_MatchedPresetInSTL_Index] = u8_StL_Index;
							u8_MatchedPresetInSTL_Index++;
							break;
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}	
				}	
			}
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH)
			{
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList; u8_StL_Index++)
				{	
					for(u8_Preset_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_Preset_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Preset_Index++)
					{
						/*Finding active band searched station is present in preset list or not*/
						if(pst_me_radio_mngr_inst->e_activeBand == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].e_Band)
						{
							if(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency == 
									st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.au32_frequency[u8_StL_Index])
							{
								pst_me_radio_mngr_inst->au8_MatchedPresetInSTL_Stn_Index[u8_MatchedPresetInSTL_Index] = u8_StL_Index;
								u8_MatchedPresetInSTL_Index++;
								break;
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}	
					}		
				}	
			}
			else if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH)
			{
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList; u8_StL_Index++)
				{	
					for(u8_Preset_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_Preset_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Preset_Index++)
					{
						/*Finding active band searched station is present in preset list or not*/
						if(pst_me_radio_mngr_inst->e_activeBand == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].e_Band)
						{
							if(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency == 
									st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[u8_StL_Index].u32_frequency)
							{
								pst_me_radio_mngr_inst->au8_MatchedPresetInSTL_Stn_Index[u8_MatchedPresetInSTL_Index] = u8_StL_Index;
								u8_MatchedPresetInSTL_Index++;
								break;
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}	
					}		
				}	
			}
			else
			{
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList; u8_StL_Index++)
				{	
					for(u8_Preset_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_Preset_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Preset_Index++)
					{
						/*Finding active band station is present in preset list or not*/
						if(pst_me_radio_mngr_inst->e_activeBand == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].e_Band)
						{
							if(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency == pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].u32_frequency)
							{
								pst_me_radio_mngr_inst->au8_MatchedPresetInSTL_Stn_Index[u8_MatchedPresetInSTL_Index] = u8_StL_Index;
								u8_MatchedPresetInSTL_Index++;
								break;
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}	
					}		
				}
			}
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_DAB_STL_SEARCH)
			{
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList; u8_StL_Index++)
				{
					for(u8_Preset_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_Preset_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Preset_Index++)
					{
						/*Finding active band searched station is present in preset list or not*/
						if(pst_me_radio_mngr_inst->e_activeBand == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].e_Band)
						{
							if((pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_DABStnInfo.u32_Sid == 
								pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[u8_StL_Index].u32_Sid)
									&& (pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_DABStnInfo.u16_SCIdI == 
									pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[u8_StL_Index].u16_SCIdI))
							{
								pst_me_radio_mngr_inst->au8_MatchedPresetInSTL_Stn_Index[u8_MatchedPresetInSTL_Index] = u8_StL_Index;
								u8_MatchedPresetInSTL_Index++;
								break;
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}	
					}	
				}
			}
			else if(pst_me_radio_mngr_inst->e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_DISABLE)
			{
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList; u8_StL_Index++)
				{
					for(u8_Preset_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_Preset_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Preset_Index++)
					{
						/*Finding active band station is present in preset list or not*/
						if(pst_me_radio_mngr_inst->e_activeBand == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].e_Band)
						{
							if((pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_DABStnInfo.u32_Sid == pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].u32_Sid)
									&& (pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_DABStnInfo.u16_SCIdI == pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].u16_SCIdI))
							{
								pst_me_radio_mngr_inst->au8_MatchedPresetInSTL_Stn_Index[u8_MatchedPresetInSTL_Index] = u8_StL_Index;
								u8_MatchedPresetInSTL_Index++;
								break;
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}	
					}	
				}
			}
			else if(st_radio_mngr_app_hsm.st_inst_hsm.u8_EnsembleSelect_Req_Status == RADIO_MNGR_ENSEMBLE_SELECT_REQUEST_RECEIVED)
			{
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < st_radio_mngr_app_hsm.st_inst_hsm.st_NormalStnView.u8_numberStationsInList; u8_StL_Index++)
				{
					for(u8_Preset_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_Preset_Index < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_Preset_Index++)
					{
						/*Finding active band station is present in preset list or not*/
						if(pst_me_radio_mngr_inst->e_activeBand == pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].e_Band)
						{
							if((pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_DABStnInfo.u32_Sid == 
									st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[st_radio_mngr_app_hsm.st_inst_hsm.u8_ReqEnsembleIndex].ast_MultiplexStationInfo[u8_StL_Index].u32_Sid)
										&& (pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[u8_Preset_Index].u_PresetStInfo.st_DABStnInfo.u16_SCIdI == 
									st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[st_radio_mngr_app_hsm.st_inst_hsm.u8_ReqEnsembleIndex].ast_MultiplexStationInfo[u8_StL_Index].u16_SCIdI))
							{
								pst_me_radio_mngr_inst->au8_MatchedPresetInSTL_Stn_Index[u8_MatchedPresetInSTL_Index] = u8_StL_Index;
								u8_MatchedPresetInSTL_Index++;
								break;
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}	
					}	
				}
			}else{/*FOR MISRA C*/}
		}
		break;

		default:
		break;
	}

}


/*======================================================================================================================*/
/*  Radio_Manager_App_StationList_Search		                                 													    */
/*======================================================================================================================*/
void Radio_Manager_App_StationList_Search(Tu8 u8_RequestedChar, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Tu8 u8_StL_Index;
	Tu8 u8_StL_Search_Index = RADIO_MNGR_APP_VALUE_ZERO;
	pst_me_radio_mngr_inst->b_Search_Found = RADIO_MNGR_APP_VALUE_ZERO;
	
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_FM:
		{
			/*Stl search requested for non RDS*/
			if(u8_RequestedChar == RADIO_APP_NONRDS_SEARCH_ASCII_HASH)
			{
				pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type = RADIO_MNGR_APP_NON_RDS_STL_SEARCH;
				
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList; u8_StL_Index++)
				{
					
					/*Checking FM PSN is zero or not,if zero then copying into FM non RDS stllist search structure*/	
					if(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].au8_PSN[RADIO_MNGR_APP_VALUE_ZERO] == RADIO_MNGR_APP_VALUE_ZERO)
					{
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.au32_frequency[u8_StL_Search_Index]), 
							&(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].u32_frequency), sizeof(Tu32));	
						u8_StL_Search_Index++;
						if(pst_me_radio_mngr_inst->b_Search_Found == RADIO_MNGR_APP_VALUE_ZERO)
						{
							pst_me_radio_mngr_inst->b_Search_Found = RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE;
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				
				if(pst_me_radio_mngr_inst->b_Search_Found == RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE)
				{
					/*For Loop for copying remaining station in the searched station list*/
					for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList; u8_StL_Index++)
					{
						/*Checking FM PSN is zero or not,if zero then copying into FM non RDS stllist search structure*/	
						if(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].au8_PSN[RADIO_MNGR_APP_VALUE_ZERO] != RADIO_MNGR_APP_VALUE_ZERO)
						{
							SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.au32_frequency[u8_StL_Search_Index]), 
								&(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].u32_frequency), sizeof(Tu32));	
							u8_StL_Search_Index++;
						}else{/*FOR MISRA C*/}
					}
				}else{}
				pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList = u8_StL_Search_Index;	
			}
			
			/*Stl search requested for RDS*/
			else if(u8_RequestedChar >= RADIO_MNGR_APP_ASCII_UPPERCASE_A && u8_RequestedChar <= RADIO_MNGR_APP_ASCII_UPPERCASE_Z)
			{
				pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type = RADIO_MNGR_APP_FM_STL_SEARCH;
				
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList; u8_StL_Index++)
				{	
					/*if received character match with stationlist PSN first character then copying to FM RDS stationlist search structure*/
					if((u8_RequestedChar == pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].au8_PSN[RADIO_MNGR_APP_VALUE_ZERO])
						||((u8_RequestedChar + RADIO_MNGR_APP_UPPER_TO_LOWER) == pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].au8_PSN[RADIO_MNGR_APP_VALUE_ZERO]))
					{
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[u8_StL_Search_Index]), 
							&(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index]), sizeof(Ts_Radio_Mngr_App_FMStationInfo));	
						u8_StL_Search_Index++;
						if(pst_me_radio_mngr_inst->b_Search_Found == RADIO_MNGR_APP_VALUE_ZERO)
						{
							pst_me_radio_mngr_inst->b_Search_Found = RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE;
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				
				if(pst_me_radio_mngr_inst->b_Search_Found == RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE)
				{
					/*Loop for copying remaining stations from FM station list*/
					for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList; u8_StL_Index++)
					{	
						/*if received character match with stationlist PSN first character then copying to FM RDS stationlist search structure*/
						if((u8_RequestedChar != pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].au8_PSN[RADIO_MNGR_APP_VALUE_ZERO])
							&&((u8_RequestedChar + RADIO_MNGR_APP_UPPER_TO_LOWER) != pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index].au8_PSN[RADIO_MNGR_APP_VALUE_ZERO]))
						{
							SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[u8_StL_Search_Index]), 
								&(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[u8_StL_Index]), sizeof(Ts_Radio_Mngr_App_FMStationInfo));	
							u8_StL_Search_Index++;
						}else{/*FOR MISRA C*/}
					}
				}else{}
				
				st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_CharSet = pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_CharSet;
				
				pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList = u8_StL_Search_Index;
			}else{/*FOR MISRA C*/}
			
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			if(u8_RequestedChar >= RADIO_MNGR_APP_ASCII_UPPERCASE_A && u8_RequestedChar <= RADIO_MNGR_APP_ASCII_UPPERCASE_Z)
			{
				pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type = RADIO_MNGR_APP_DAB_STL_SEARCH;
				
				for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList; u8_StL_Index++)
				{
					/*if received character match with stationlist service label first character then copying to DAB stationlist search structure*/
					if((u8_RequestedChar == pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].au8_SrvLabel[RADIO_MNGR_APP_VALUE_ZERO])
							||((u8_RequestedChar + RADIO_MNGR_APP_UPPER_TO_LOWER) == pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].au8_SrvLabel[RADIO_MNGR_APP_VALUE_ZERO]))
					{
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[u8_StL_Search_Index]), 
								&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));	
						u8_StL_Search_Index++;
						if(pst_me_radio_mngr_inst->b_Search_Found == RADIO_MNGR_APP_VALUE_ZERO)
						{
							pst_me_radio_mngr_inst->b_Search_Found = RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE;
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				
				if(pst_me_radio_mngr_inst->b_Search_Found == RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE)
				{
					/*Loop for copying remaining stations in DAB station list*/
					for(u8_StL_Index = RADIO_MNGR_APP_VALUE_ZERO; u8_StL_Index < pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList; u8_StL_Index++)
					{
						/*if received character didnt match(remaining) with stationlist service label first character then copying to DAB stationlist search structure*/
						if((u8_RequestedChar != pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].au8_SrvLabel[RADIO_MNGR_APP_VALUE_ZERO])
								&&((u8_RequestedChar + RADIO_MNGR_APP_UPPER_TO_LOWER) != pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index].au8_SrvLabel[RADIO_MNGR_APP_VALUE_ZERO]))
						{
							SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[u8_StL_Search_Index]), 
									&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[u8_StL_Index]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));	
							u8_StL_Search_Index++;
						}else{/*FOR MISRA C*/}
					}
				}else{}
			}
			pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList = u8_StL_Search_Index;
		}
		break;

		default:
		break;
	}
}


/*======================================================================================================================*/
/*  Radio_Mngr_App_GetRadioStationListSearch		                                 													    */
/*======================================================================================================================*/
void Radio_Mngr_App_GetRadioStationListSearch(Tu8 u8_RequestedChar)
{
	Tu8 index = 0;
	Tu8 Preset_In_STLMatchindex = 0;
	Ts32 s32_StringCompare_RetValue 	= RADIO_MNGR_APP_VALUE_ZERO;	
	Tu8  u8_char_loc;
	Tu8  u8_dest_loc ;
	
	memset(&(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search), 0, sizeof(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search));
	Radio_Manager_App_StationList_Search(u8_RequestedChar ,&st_radio_mngr_app_hsm.st_inst_hsm);
	if(st_radio_mngr_app_hsm.st_inst_hsm.b_Search_Found == RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE)
	{
		Update_MatchedStationListIndex(&(st_radio_mngr_app_hsm.st_inst_hsm));
		memset(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index, RADIO_MNGR_APP_TU8_MAX_VALUE, RADIO_MNGR_APP_MAX_PSML_SIZE);
			Update_MatchedPresetInStationListIndex(&(st_radio_mngr_app_hsm.st_inst_hsm));
	}else{/*FOR MISRA C*/}
	
    switch(st_radio_mngr_app_hsm.st_inst_hsm.e_activeBand)
    {
		case RADIO_MNGR_APP_BAND_FM:
	    {
			if(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH)
			{
				noSearchSTLStationsDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList;
				for(index = 0; index < noSearchSTLStationsDisplay; index++)
				{
					_radioSTLSearchData_Display[index].nBand = (MODE_TYPE)RADIO_MNGR_APP_BAND_FM;		    
					_radioSTLSearchData_Display[index].Index = index;
					_radioSTLSearchData_Display[index].Frequency = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.au32_frequency[index];
					_radioSTLSearchData_Display[index].Char_set = 0;
					memset((void *)_radioSTLSearchData_Display[index].ServiceName, (Tu16)0 , sizeof(_radioSTLSearchData_Display[index].ServiceName));

					if(st_radio_mngr_app_hsm.st_inst_hsm.u8_MatchedStL_Stn_Index == index)
					{
						_radioSTLSearchData_Display[index].Matched_Search_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL;
					}
					else
					{
						_radioSTLSearchData_Display[index].Matched_Search_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL;
					}
				
					if(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index[Preset_In_STLMatchindex] == index)
					{
						_radioSTLSearchData_Display[index].MatchedPresetInSearchSTL_index_Flag = RADIO_MNGR_APP_MATCHED_PRESET_IN_STL;
						Preset_In_STLMatchindex++;	
					}
					else
					{
						_radioSTLSearchData_Display[index].MatchedPresetInSearchSTL_index_Flag = RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL;
					}
				}	
			}
			else if(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH)
			{
				noSearchSTLStationsDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList;
				for(index = 0; index < noSearchSTLStationsDisplay; index++)
				{
					_radioSTLSearchData_Display[index].nBand = (MODE_TYPE)RADIO_MNGR_APP_BAND_FM;		    
					_radioSTLSearchData_Display[index].Index = index;
					_radioSTLSearchData_Display[index].Frequency = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[index].u32_frequency;
					_radioSTLSearchData_Display[index].Char_set = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_CharSet;
					memset((void *)(_radioSTLSearchData_Display[index].ServiceName),(Tu16)0,sizeof(_radioSTLSearchData_Display[index].ServiceName));
					SYS_RADIO_MEMCPY((void *)(_radioSTLSearchData_Display[index].ServiceName),
										(const void*)(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[index].au8_PSN),RADIO_MNGR_APP_CHAN_NAME);
	    
					if(st_radio_mngr_app_hsm.st_inst_hsm.u8_MatchedStL_Stn_Index == index)
					{
						_radioSTLSearchData_Display[index].Matched_Search_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL;
					}
					else
					{
						_radioSTLSearchData_Display[index].Matched_Search_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL;
					}
			
					if(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index[Preset_In_STLMatchindex] == index)
					{
						_radioSTLSearchData_Display[index].MatchedPresetInSearchSTL_index_Flag = RADIO_MNGR_APP_MATCHED_PRESET_IN_STL;
						Preset_In_STLMatchindex++;	
					}
					else
					{
						_radioSTLSearchData_Display[index].MatchedPresetInSearchSTL_index_Flag = RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL;
					}
				}
			}else{/*FOR MISRA C*/}
		}
		break;
		
	    case RADIO_MNGR_APP_BAND_DAB:
	    {
			noSearchSTLStationsDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList;			
		    for(index = 0; index < noSearchSTLStationsDisplay; index++)
		    {
		    	_radioSTLSearchData_Display[index].nBand = (MODE_TYPE)RADIO_MNGR_APP_BAND_DAB;
			    _radioSTLSearchData_Display[index].Index = index;
				_radioSTLSearchData_Display[index].Frequency = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].u32_Frequency;
				_radioSTLSearchData_Display[index].Char_set = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].u8_CharSet;
			    memset((void *)(_radioSTLSearchData_Display[index].ServiceName),(Tu16)0, sizeof(_radioSTLSearchData_Display[index].ServiceName));
				
				/*String comparison function to check if both Service and service component Labels are same then no need to merge*/
				s32_StringCompare_RetValue = SYS_RADIO_STR_CMP((Tu8*)(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_SrvLabel), 
																		(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_CompLabel),
																		RADIO_MNGR_APP_NUMCHAR_LABEL);
				
				
				/*Comparing the return value of SYS_RADIO_STR_CMP function*/														
				if(s32_StringCompare_RetValue == RADIO_MNGR_APP_VALUE_ZERO)
				{
					SYS_RADIO_MEMCPY((_radioSTLSearchData_Display[index].ServiceName),
										(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_SrvLabel),
										RADIO_MNGR_APP_NUMCHAR_LABEL);	
				}
				
				else if(s32_StringCompare_RetValue != RADIO_MNGR_APP_VALUE_ZERO)
				{
					/*Loop for copying the first three characters from the service label without any condition*/
					for(u8_char_loc = 0, u8_dest_loc=0; u8_char_loc <= 2; u8_char_loc++)
					{
						_radioSTLSearchData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_SrvLabel[u8_char_loc];
						u8_dest_loc++;
					}
				
					/* copying the service label & component label HMI IF*/			
					for(u8_char_loc =3,u8_dest_loc =3; u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL ; u8_char_loc++)
					{
						if( st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_SrvLabel[u8_char_loc] != RADIO_MNGR_APP_ASCII_SPACE)
						{
							_radioSTLSearchData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_SrvLabel[u8_char_loc];
							u8_dest_loc++;
						}
						else
						{
							if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL)
							{
								u8_char_loc++;
							}
							else
							{
								/*FOR MISRA C*/
							}
							if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL && st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_SrvLabel[u8_char_loc] == RADIO_MNGR_APP_ASCII_SPACE)
							{
								u8_char_loc--;
								break;
							}else{/*FOR MISRA C*/}

							u8_char_loc--;
							/*copying the characters from the service label if the second char is not space*/
							_radioSTLSearchData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_SrvLabel[u8_char_loc];
							u8_dest_loc++;
						}
					}
		
					/*Apending the service component label into radio manager component name*/
					SYS_RADIO_MEMCPY(&(_radioSTLSearchData_Display[index].ServiceName[u8_dest_loc]),
																	(st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_CompLabel),
																	sizeof((st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[index].au8_CompLabel)));
				}
		
				if(st_radio_mngr_app_hsm.st_inst_hsm.u8_MatchedStL_Stn_Index == index)
				{
					_radioSTLSearchData_Display[index].Matched_Search_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL;
				}
				else
				{
					_radioSTLSearchData_Display[index].Matched_Search_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL;
				}
				
				if(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index[Preset_In_STLMatchindex] == index)
				{
					_radioSTLSearchData_Display[index].MatchedPresetInSearchSTL_index_Flag = RADIO_MNGR_APP_MATCHED_PRESET_IN_STL;
					Preset_In_STLMatchindex++;	
				}
				else
				{
					_radioSTLSearchData_Display[index].MatchedPresetInSearchSTL_index_Flag = RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL;
				}
		    }
		    
	    }
		break;
		
	    default:
	    {
		   
	    }
		break;		
	}
}

/*=============================================================================================================================*/
/* Update_LSM_Index(void)                                  															   */
/*=============================================================================================================================*/
void Update_Searched_STL_StationInfo_with_index(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst)
{
	/*updating Tunable station info structure with the requested index station info*/
	switch(pst_me_radio_mngr_inst->e_activeBand)
	{
		case RADIO_MNGR_APP_BAND_FM:
		{
			if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH && 
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList != RADIO_MNGR_APP_VALUE_ZERO)
			{
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq = 
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.au32_frequency[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex];
			}
			else if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH &&
						pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList != RADIO_MNGR_APP_VALUE_ZERO)
			{
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq = 
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].u32_frequency;
								
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI      = 
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].u16_PI;		
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			if(pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList != RADIO_MNGR_APP_VALUE_ZERO)
			{
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = 
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].u32_Frequency;
									
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId		 = 
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].u16_EId;
						
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid      = 
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].u32_Sid;
						
				pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI	 =
					pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].u16_SCIdI;
			}else{/*FOR MISRA C*/}
		}
		break;
		
		default:
		{
		
		}
		break;
	}
}

/*=============================================================================================================================*/
/* Radio_Mngr_App_CreateNormalRadioStationList                                  															   */
/*=============================================================================================================================*/
void Radio_Mngr_App_CreateNormalRadioStationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Tu8 u8_DAB_Stn_index;/*Without same service removal*/
	Tu8 u8_DAB_MatchedStn_Start_index = 0;
	Tu8 u8_DAB_MatchedStn_End_index = 0;
	Tu8 u8_CheckSameSID_index;
	Tu8 u8_NoOfServiceComponent = 0;
	memset(&(pst_me_radio_mngr_inst->st_NormalStnView), 0, sizeof(Ts_Radio_Mngr_App_DAB_SL));

	for(u8_DAB_Stn_index = 0; u8_DAB_Stn_index < pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.u8_numberStationsInList; u8_DAB_Stn_index++)
	{
		/*Copying the different Sid Stations into normal station list*/
		if(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index].u32_Sid !=
				pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index + 1].u32_Sid)
		{
			SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList]), 
											&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));
			(pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList)++;
		}

		else
		{
			u8_DAB_MatchedStn_Start_index = u8_DAB_Stn_index;
			/*If same sid found more than once need to find the end index of repeated sid*/
			for(u8_CheckSameSID_index = u8_DAB_MatchedStn_Start_index + 1; u8_CheckSameSID_index < pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.u8_numberStationsInList; u8_CheckSameSID_index++)
			{
				if(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index].u32_Sid ==
						pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_CheckSameSID_index].u32_Sid)
				{
					u8_DAB_MatchedStn_End_index = u8_CheckSameSID_index;
				}
				else 
				break;
			}
			/*Finding number of components in a service by comparing scid*/
			if(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index].u16_SCIdI ==
					pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index + 1].u16_SCIdI)
			{
				u8_NoOfServiceComponent = 1;
				Radio_Mngr_App_Find_Greatest_RSSI(pst_me_radio_mngr_inst, u8_DAB_MatchedStn_Start_index, u8_DAB_MatchedStn_End_index, u8_NoOfServiceComponent);
			}
			else if((u8_DAB_MatchedStn_End_index - u8_DAB_MatchedStn_Start_index) > 2)
			{
				u8_NoOfServiceComponent = 2;
				Radio_Mngr_App_Find_Greatest_RSSI(pst_me_radio_mngr_inst, u8_DAB_MatchedStn_Start_index, u8_DAB_MatchedStn_End_index, u8_NoOfServiceComponent);
			}
			else if((u8_DAB_MatchedStn_End_index - u8_DAB_MatchedStn_Start_index) < 2)
			{
				/*If a service has two components then comparing EID's and if both EID are same then copying into normal stl*/
				if(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index].u16_EId == 
						pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index + 1].u16_EId)
				{
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList]), 
											&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));
					(pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList)++;
		
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList]), 
													&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index + 1]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));
					(pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList)++;
				}
				/*Service has two components then comparing EID's if both EID is different means finding high RSSI station and copying into normal stl list*/
				else if(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index].u16_EId != 
							pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DAB_Stn_index + 1].u16_EId)
				{
					u8_NoOfServiceComponent = 1;
					Radio_Mngr_App_Find_Greatest_RSSI(pst_me_radio_mngr_inst, u8_DAB_MatchedStn_Start_index, u8_DAB_MatchedStn_End_index, u8_NoOfServiceComponent);
				}				
			}
			else{/*FOR MISRA C*/}
			u8_DAB_Stn_index = u8_DAB_MatchedStn_End_index;
		}		
	}
}

/*=============================================================================================================================*/
/* Radio_Mngr_App_Find_Greatest_RSSI                                  															   */
/*=============================================================================================================================*/
void Radio_Mngr_App_Find_Greatest_RSSI(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Tu8 u8_DAB_MatchedStn_Start_index, Tu8 u8_DAB_MatchedStn_End_index, Tu8 u8_NoOfServiceComponent)
{
	Tu8 u8_Index;
	Tu8 u8_HighRSSI_Index = 0;
	if(u8_NoOfServiceComponent == 1)
	{
		u8_HighRSSI_Index = u8_DAB_MatchedStn_Start_index;
		for(u8_Index = u8_DAB_MatchedStn_Start_index + 1; u8_Index <= u8_DAB_MatchedStn_End_index; u8_Index++)
		{
			if(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_HighRSSI_Index].s8_RSSI <  
					pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_Index].s8_RSSI)
			{
				u8_HighRSSI_Index = u8_Index;
			}
		}
		SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList]), 
										&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_HighRSSI_Index]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));
		(pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList)++;
	}
	else if(u8_NoOfServiceComponent == 2)
	{
		u8_HighRSSI_Index = u8_DAB_MatchedStn_Start_index;
		for(u8_Index = u8_DAB_MatchedStn_Start_index + 2; u8_Index <= u8_DAB_MatchedStn_End_index; u8_Index = u8_Index + 2)
		{
			if(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_HighRSSI_Index].s8_RSSI <  
					pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_Index].s8_RSSI)
			{
				u8_HighRSSI_Index = u8_Index;
			}
		}
		SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList]), 
										&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_HighRSSI_Index]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));
		(pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList)++;
		
		SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_NormalStnView.ast_Stations[pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList]), 
										&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_HighRSSI_Index + 1]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));
		(pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList)++;
	}
}

/*=============================================================================================================================*/
/* Radio_Mngr_App_CreateMultiplexRadioStationList                                  															   */
/*=============================================================================================================================*/
void Radio_Mngr_App_CreateMultiplexRadioStationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
	Tu8  u8_Ensemble_index;
	Tu8  u8_DABStn_Index;
	Tu8  u8_NoOfServiceInMultiplex = 0;
	
	memset(&(pst_me_radio_mngr_inst->st_MultiplexStlView), 0, sizeof(Ts_Radio_Mngr_App_MultiplexStationListInfo));
	
	pst_me_radio_mngr_inst->st_MultiplexStlView.u8_NoOfEnsembles = pst_me_radio_mngr_inst->st_DABEnsembleList.u8_NoOfEnsembleList;
	for(u8_Ensemble_index = 0; u8_Ensemble_index < pst_me_radio_mngr_inst->st_MultiplexStlView.u8_NoOfEnsembles; u8_Ensemble_index++)
	{
		SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[u8_Ensemble_index].u16_EId), 
								&(pst_me_radio_mngr_inst->st_DABEnsembleList.ast_EnsembleInfo[u8_Ensemble_index].u16_EId), sizeof(Tu16)); 
		for(u8_DABStn_Index = 0; u8_DABStn_Index < pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.u8_numberStationsInList; u8_DABStn_Index++)
		{
			if(pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[u8_Ensemble_index].u16_EId == 
					pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DABStn_Index].u16_EId)
			{
				SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[u8_Ensemble_index].ast_MultiplexStationInfo[u8_NoOfServiceInMultiplex]), 
									&(pst_me_radio_mngr_inst->st_RadioStationList.st_DAB_StationList.ast_Stations[u8_DABStn_Index]), sizeof(Ts_Radio_Mngr_App_DAB_StnInfo));
				u8_NoOfServiceInMultiplex++;
			}else{/*FOR MISRA C*/}
		}
		pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[u8_Ensemble_index].u8_NoOfserviceInEnsemble = u8_NoOfServiceInMultiplex;
		/*Clearing no of service variable to calculate for next ensemble */
		u8_NoOfServiceInMultiplex = 0;
		
	}
}

/*=============================================================================================================================*/
/* Radio_Mngr_App_Request_EnsembleSelect_From_MultiplexList                                  															   */
/*=============================================================================================================================*/
void Radio_Mngr_App_Request_EnsembleSelect_From_MultiplexList(Tu8 u8_EnsembleIndex)
{
	Tu8 index = 0;
	Tu8 Preset_In_STLMatchindex = 0;
	Ts32 s32_StringCompare_RetValue 	= RADIO_MNGR_APP_VALUE_ZERO;
	Tu8  u8_char_loc;
	Tu8  u8_dest_loc ;
	st_radio_mngr_app_hsm.st_inst_hsm.st_StationList_Search.e_STL_Search_Type = RADIO_MNGR_APP_STL_SEARCH_INVALID;
	st_radio_mngr_app_hsm.st_inst_hsm.u8_ReqEnsembleIndex = u8_EnsembleIndex;
	Update_MatchedStationListIndex(&(st_radio_mngr_app_hsm.st_inst_hsm));
	memset(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index, RADIO_MNGR_APP_TU8_MAX_VALUE, RADIO_MNGR_APP_MAX_PSML_SIZE);
	st_radio_mngr_app_hsm.st_inst_hsm.u8_EnsembleSelect_Req_Status   = RADIO_MNGR_ENSEMBLE_SELECT_REQUEST_RECEIVED;
	Update_MatchedPresetInStationListIndex(&(st_radio_mngr_app_hsm.st_inst_hsm));
	noStationsDisplay = st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].u8_NoOfserviceInEnsemble;			
	for(index = 0; index < noStationsDisplay; index++)
	{
		_radioStationListData_Display[index].nBand = (MODE_TYPE)RADIO_MNGR_APP_BAND_DAB;
		_radioStationListData_Display[index].Index = index;
		_radioStationListData_Display[index].Frequency = st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].u32_Frequency;
		_radioStationListData_Display[index].Char_set = st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].u8_CharSet;
		memset((void *)(_radioStationListData_Display[index].ServiceName),(Tu16)0, sizeof(_radioStationListData_Display[index].ServiceName));
		
		/*String comparison function to check if both Service and service component Labels are same then no need to merge*/
		s32_StringCompare_RetValue = SYS_RADIO_STR_CMP((st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_SrvLabel), 
															(st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_CompLabel),
															RADIO_MNGR_APP_NUMCHAR_LABEL);
		
		/*Comparing the return value of SYS_RADIO_STR_CMP function*/														
		if(s32_StringCompare_RetValue == RADIO_MNGR_APP_VALUE_ZERO)
		{
			SYS_RADIO_MEMCPY((_radioStationListData_Display[index].ServiceName),
									(st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_SrvLabel),
									RADIO_MNGR_APP_NUMCHAR_LABEL);	
		}
		
		else if(s32_StringCompare_RetValue != RADIO_MNGR_APP_VALUE_ZERO)
		{
			/*Loop for copying the first three characters from the service label without any condition*/
			for(u8_char_loc = 0, u8_dest_loc=0; u8_char_loc <= 2; u8_char_loc++)
			{
				_radioStationListData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_SrvLabel[u8_char_loc];
				u8_dest_loc++;
			}

			/* copying the service label & component label HMI IF*/			
			for(u8_char_loc =3,u8_dest_loc =3; u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL ; u8_char_loc++)
			{
				if( st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_SrvLabel[u8_char_loc] != RADIO_MNGR_APP_ASCII_SPACE)
				{
					_radioStationListData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_SrvLabel[u8_char_loc];
					u8_dest_loc++;
				}
				else
				{
					if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL)
					{
						u8_char_loc++;
					}
					else
					{
						/*FOR MISRA C*/
					}
					if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL && st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_SrvLabel[u8_char_loc] == RADIO_MNGR_APP_ASCII_SPACE)
					{
						u8_char_loc--;
						break;
					}else{/*FOR MISRA C*/}

					u8_char_loc--;
					/*copying the characters from the service label if the second char is not space*/
					_radioStationListData_Display[index].ServiceName[u8_dest_loc] = st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_SrvLabel[u8_char_loc];
					u8_dest_loc++;
				}
			}
			
			/*Apending the service component label into radio manager component name*/
			SYS_RADIO_MEMCPY(&(_radioStationListData_Display[index].ServiceName[u8_dest_loc]),
															(st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_CompLabel),
															sizeof((st_radio_mngr_app_hsm.st_inst_hsm.st_MultiplexStlView.ast_EnsembleInfo[u8_EnsembleIndex].ast_MultiplexStationInfo[index].au8_CompLabel)));
				
		}
		
		if(st_radio_mngr_app_hsm.st_inst_hsm.u8_MatchedStL_Stn_Index == index)
		{
			_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL;
		}
		else
		{
			_radioStationListData_Display[index].Matched_Stn_Index_Flag = RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL;
		}

		if(st_radio_mngr_app_hsm.st_inst_hsm.au8_MatchedPresetInSTL_Stn_Index[Preset_In_STLMatchindex] == index)
		{
			_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_MATCHED_PRESET_IN_STL;
			Preset_In_STLMatchindex++;	
		}
		else
		{
			_radioStationListData_Display[index].MatchedPresetInSTL_index_Flag = RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL;
		}
	}	
}

/*=============================================================================================================================*/
/* Update_Multiplex_Service_StationInfo_with_index                                															   */
/*=============================================================================================================================*/
void Update_Multiplex_Service_StationInfo_with_index(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst)
{
	if(pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble != RADIO_MNGR_APP_VALUE_ZERO)
	{
		pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq = 
			pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].ast_MultiplexStationInfo[pst_me_radio_mngr_inst->u8_ServiceIndex].u32_Frequency;
							
		pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId		 = 
			pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].ast_MultiplexStationInfo[pst_me_radio_mngr_inst->u8_ServiceIndex].u16_EId;
				
		pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid      = 
			pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].ast_MultiplexStationInfo[pst_me_radio_mngr_inst->u8_ServiceIndex].u32_Sid;
				
		pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI	 =
			pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].ast_MultiplexStationInfo[pst_me_radio_mngr_inst->u8_ServiceIndex].u16_SCIdI;
	}else{/*FOR MISRA C*/}

}
/*=============================================================================================================================*/
/* Radio_Mngr_App_Update_ServiceNumber_In_EnsembleList(pst_me_radio_mngr_inst)                                  															   */
/*=============================================================================================================================*/
void Radio_Mngr_App_Update_ServiceNumber_In_EnsembleList(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst)
{
	Ts8 s8_EIdFound_Index = -1;
	Ts8 s8_SIdFound_Index = -1;
	Tu8 u8_EId_Index 	  =  0;
	Tu8 u8_SId_Index      =  0;
	
	for(u8_EId_Index = 0; u8_EId_Index < pst_me_radio_mngr_inst->st_MultiplexStlView.u8_NoOfEnsembles; u8_EId_Index++)
	{
		if(pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_EId == pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[u8_EId_Index].u16_EId)
		{
			s8_EIdFound_Index 	= u8_EId_Index;
			break;
		}else{/*FOR MISRA C*/}
	}
	
	if(s8_EIdFound_Index != -1)
	{
		for(u8_SId_Index = 0; u8_SId_Index < pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[s8_EIdFound_Index].u8_NoOfserviceInEnsemble; u8_SId_Index++)
		{	
			if((pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_SId == pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[s8_EIdFound_Index].ast_MultiplexStationInfo[u8_SId_Index].u32_Sid)&&
				(pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u16_SCIdI == pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[s8_EIdFound_Index].ast_MultiplexStationInfo[u8_SId_Index].u16_SCIdI))
			{
				s8_SIdFound_Index = u8_SId_Index;
				break;
			}else{/*FOR MISRA C*/}
		}
	}else{/*FOR MISRA C*/}
	
	if(s8_EIdFound_Index != -1 && s8_SIdFound_Index != -1)
	{
		pst_me_radio_mngr_inst->u8_CurrentlyPlayingServiceInEnsemble 	=  s8_SIdFound_Index+1;
		pst_me_radio_mngr_inst->u8_TotalNoOfServiceInEnsemble			=  pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[s8_EIdFound_Index].u8_NoOfserviceInEnsemble;
	}
	else
	{
		pst_me_radio_mngr_inst->u8_CurrentlyPlayingServiceInEnsemble 	= RADIO_MNGR_APP_VALUE_ZERO;
		pst_me_radio_mngr_inst->u8_TotalNoOfServiceInEnsemble 			= RADIO_MNGR_APP_VALUE_ZERO;
	}
}

/*=============================================================================================================================*/
/* Radio_Mngr_IntrMsg                                  																		   */
/*=============================================================================================================================*/
void Radio_Mngr_IntrMsg(Ts_Sys_Msg *msg)
{
	HSM_ON_MSG(&(st_radio_mngr_app_hsm.st_inst_hsm), msg);
}
