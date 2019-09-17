/************************************************************************************************************/
/** \file hmi_if_app_request.c																				    *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: HMI IF															     			*
*  Description			: This file contains API's for request to HMI IF									*
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include <string.h>
#include "hmi_if_extern.h"
#ifndef __IRADIO_API_H__
#include "IRadio.h"
#endif
#include "hmi_if_app_request.h"
#include "radio_mngr_app_request.h"
#include "Tuner_core_sys_main.h"
#include "radio_mngr_app.h"
#include "debug_log.h"

/**
* @brief This character buffer represents the requested channel name for Tune by Frequency feature in DAB
*/
UINT8				DAB_Requested_ChannelName[RADIO_MAX_CHANNEL_LABEL_SIZE];

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
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


/*===========================================================================*/
/*  void Request_ChangeRadioMode																*/
/*===========================================================================*/
void Request_ChangeRadioMode(MODE_TYPE e_mode)
{
    Te_Radio_Mngr_App_Band e_band = RADIO_MNGR_APP_BAND_INVALID;

    switch(e_mode)
    {
        case RADIO_MODE_AM:
            e_band = RADIO_MNGR_APP_BAND_AM;
            break;

        case RADIO_MODE_FM:
            e_band = RADIO_MNGR_APP_BAND_FM;
            break;

        case RADIO_MODE_DAB:
            e_band = RADIO_MNGR_APP_BAND_DAB;
            break;

        default:
            break;
    }
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Band Change Request: RequestedBand: %d",e_mode);
    Radio_Mngr_App_Request_SelectBand(e_band);
}

/*===========================================================================*/
/* void Request_EtalHWConfig												 */
/*===========================================================================*/
void Request_EtalHWConfig(Radio_EtalHardwareAttr attr)
{
	Sys_Debug_Initialize();

	SYS_MAIN_EtalInit(attr);
}

/*===========================================================================*/
/* void Request_StartRadio																*/
/*===========================================================================*/
void Request_StartRadio(VARIANT_TYPE e_variant, MODEL_TYPE e_market, UINT8 u8_RadioSourceInfo)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Startup Request: VariantType: %d, MarketType: %d, RadioSourceInfo: %d", e_variant, e_market, u8_RadioSourceInfo);
	
	SYS_MAIN_StartOS((Te_Radio_Framework_Variant)e_variant, (Te_Radio_Framework_Market)e_market, (Tu8)u8_RadioSourceInfo);
}

/*===========================================================================*/
/* void Request_ShutDownTuner																	*/
/*===========================================================================*/
void Request_ShutDownTuner(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Shutdown Request");
	
	SYS_MAIN_StopOS();
}

/*===========================================================================*/
/* void Request_EtalHWDeconfig												*/
/*===========================================================================*/
void Request_EtalHWDeconfig(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[Radio][HMI IF]Radio EtalHWDeconfig Request");

	SYS_MAIN_EtalDeInit();
}

/*===========================================================================*/
/* void Request_PlaySelectStation																*/
/*===========================================================================*/
void Request_PlaySelectStation(UINT8 u8_Index)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Select Station from Station list Request: RequestedIndex: %d",u8_Index);
    Radio_Mngr_App_Request_PlaySelectSt(u8_Index);
}

/*===========================================================================*/
/* void Request_SeekStation																		*/
/*===========================================================================*/
void Request_SeekStation(RADIO_DIRECTION Seek_Direction)
{
	if(Seek_Direction == RADIO_DIRECTION_DOWN)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Seek Down Request: RequestedDirection: %d",Seek_Direction);	
	}
	else if(Seek_Direction == RADIO_DIRECTION_UP)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Seek Up Request: RequestedDirection: %d",Seek_Direction);	
	}
	else{/*FOR MISRA C*/}
	Radio_Mngr_App_Request_SeekUpDown((Te_RADIO_DirectionType)Seek_Direction);
}

/*===========================================================================*/
/* MODE_TYPE Request_GetRadioMode																*/
/*===========================================================================*/
MODE_TYPE Request_GetRadioMode()
{
        MODE_TYPE e_Mode;
        e_Mode = _radioCommonData.Aud_Band;
		
		/*Debug Logs for Get Radio Mode -- Current Audio band will be returned*/
		if(_radioCommonData.Aud_Band == RADIO_MODE_AM)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]GetRadioMode Request: AudioBand:AM %d", _radioCommonData.Aud_Band);
		}
		else if(_radioCommonData.Aud_Band == RADIO_MODE_FM)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]GetRadioMode Request: AudioBand:FM %d", _radioCommonData.Aud_Band);
		}
		else if(_radioCommonData.Aud_Band == RADIO_MODE_DAB)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]GetRadioMode Request: AudioBand:DAB %d", _radioCommonData.Aud_Band);
		}
		else{/*FOR MISRA C*/}
		
        return e_Mode;
}
/*===========================================================================*/
/* HMIIF_IDataObject Request_GetRadioStationListData															*/
/*===========================================================================*/
HMIIF_IDataObject Request_GetRadioStationListData()
{
	Tu8 u8_Station_or_Ensemble_List = 0;
	memset(_radioStationListData_Display,0,sizeof(_radioStationListData_Display[0]) * MAX_RADIO_STATIONS);
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Get Radio StationList Request");

	u8_Station_or_Ensemble_List = Radio_Mngr_App_GetRadioStationList();
	
	if(u8_Station_or_Ensemble_List == RADIO_NORMAL_STL_VIEW)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Normal STL object returning to HMI");
		return _radioStaionDataObjectDisplay;
	}
	else if(u8_Station_or_Ensemble_List == RADIO_MULTIPLEX_VIEW)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Multiplex view List object returning to HMI");
		return _radioEnsembleListDataObjectDisplay;
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Neither STL Nor Multiplex List notified to HMI");
		return _radioStaionDataObjectDisplay;
	}
	
}
/*===========================================================================*/
/* HMIIF_IDataObject Request_GetRadioCommonData																*/
/*===========================================================================*/
HMIIF_IDataObject Request_GetRadioCommonData()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Get Radio CommonData Request");
	return _radioDataObject;
}


/*===========================================================================*/
/* void Request_GetAFList_Diag																	*/
/*===========================================================================*/
void Request_GetAFList_Diag()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Radio AF List DiagRequest");
	Radio_Mngr_App_Request_GetAFList_Diag();
}

/*===========================================================================*/
/* void Request_EnableDABFMLinking															*/
/*===========================================================================*/
void Request_EnableDABFMLinking(Radio_Switch_Request_Settings e_DABFM_LinkRequest)
{
	if(e_DABFM_LinkRequest == RADIO_SWITCH_REQUEST_ENABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]DAB-FM Linking Switch Enable Request: EnableSwitchRequest: %d",e_DABFM_LinkRequest);
	}
	else if(e_DABFM_LinkRequest == RADIO_SWITCH_REQUEST_DISABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]DAB-FM Linking Switch Disable Request: DisableSwitchRequest: %d",e_DABFM_LinkRequest);
	}
	else{/*FOR MISRA C*/}
	Radio_Mngr_App_Request_EnableDABFMLinking((Te_Radio_Mngr_App_DABFMLinking_Switch)e_DABFM_LinkRequest);
}

/*===========================================================================*/
/* void Request_GetCurStationInfo_Diag															*/
/*===========================================================================*/
void Request_GetCurStationInfo_Diag()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Current Station Info DiagRequest");
	Radio_Mngr_App_Request_GetCurStationInfo_Diag();
}

/*===========================================================================*/
/* void Request_ObtainSTL_Diag																	*/
/*===========================================================================*/
void Request_ObtainSTL_Diag()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Station List DiagRequest");
	Radio_Mngr_App_Request_ObtainStationList_Diag();
}

/*===========================================================================*/
/* void Request_GetQuality_Diag																	*/
/*===========================================================================*/
void Request_GetQuality_Diag()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Quality parameters DiagRequest");
	Radio_Mngr_App_Request_GetQuality_Diag();
}


/*===========================================================================*/
/* void Request_RDSFollowing																	*/
/*===========================================================================*/
void Request_RDSFollowing(Radio_Switch_Request_Settings e_RDS_following_request)
{
	if(e_RDS_following_request == RADIO_SWITCH_REQUEST_ENABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]RDS Switch Enable Request: EnableSwitchRequest: %d",e_RDS_following_request);
	}
	else if(e_RDS_following_request == RADIO_SWITCH_REQUEST_DISABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]RDS Switch Disable Request: DisableSwitchRequest: %d",e_RDS_following_request);
	}
	else{/*FOR MISRA C*/}
	Radio_Mngr_App_Request_RDSSettings((Te_Radio_Mngr_App_RDSSettings)e_RDS_following_request);
}

/*===========================================================================*/
/* void Request_MemoryList																	*/
/*===========================================================================*/
void Request_MemoryList()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Memory(Preset) List Request");
	memset(_radioMemoryListData, 0, sizeof(_radioMemoryListData[0]) * MAX_RADIO_PRESET_STATIONS);
	Radio_Mngr_App_Request_GetPresetList();
}

/*===========================================================================*/
/* void Request_StoreMemoryList																	*/
/*===========================================================================*/
void Request_StoreMemoryList(UINT8 u8_Index)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Store Memory List Request: IndexRequest: %d",u8_Index);
	Radio_Mngr_App_Request_PresetStore(u8_Index);
}

/*===========================================================================*/
/* void Request_PlaySelectMemoryList																	*/
/*===========================================================================*/
void Request_PlaySelectMemoryList(UINT8 u8_Index)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Recall Station from Memory list Request: RequestedIndex: %d",u8_Index);
	Radio_Mngr_App_Request_PresetRecall(u8_Index);
}

/*===========================================================================*/
/* void Request_ManualUpdateSTL																	*/
/*===========================================================================*/
void Request_ManualUpdateSTL()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Manual Update Station list Request");
	Radio_Mngr_App_Request_UpdateStationList();
}

/*===========================================================================*/
/* void Request_TuneUpDown													*/
/*===========================================================================*/
void Request_TuneUpDown(RADIO_DIRECTION e_TuneUpDownDirection)
{
	if(e_TuneUpDownDirection == RADIO_DIRECTION_DOWN)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Tune Down DiagRequest: RequestedDirection: %d",e_TuneUpDownDirection);	
	}
	else if(e_TuneUpDownDirection == RADIO_DIRECTION_UP)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Tune Up DiagRequest: RequestedDirection: %d",e_TuneUpDownDirection);	
	}
	else{/*FOR MISRA C*/}
	Radio_Mngr_App_Request_TuneUpDown((Te_RADIO_DirectionType)e_TuneUpDownDirection);
}

/*===========================================================================*/
/* void Request_EnableAnnouncement														*/
/*===========================================================================*/
void Request_EnableAnnouncement(Radio_Switch_Request_Settings e_AnnoEnableTASwitch)
{
	if(e_AnnoEnableTASwitch == RADIO_SWITCH_REQUEST_ENABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]TA Anno Switch Enable Request: EnableSwitchRequest: %d",e_AnnoEnableTASwitch);
	}
	else if(e_AnnoEnableTASwitch == RADIO_SWITCH_REQUEST_DISABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]TA Anno Switch Disable Request: DisableSwitchRequest: %d",e_AnnoEnableTASwitch);
	}
	else{/*FOR MISRA C*/}
	Radio_Mngr_App_Request_EnableTAAnnouncement((Te_Radio_Mngr_App_EnableTAAnno_Switch)e_AnnoEnableTASwitch);
	Radio_Mngr_App_Request_EnableAnnouncementInfo((Te_Radio_Mngr_App_EnableInfoAnno_Switch)e_AnnoEnableTASwitch);
}

/*===========================================================================*/
/* void Request_CancelAnnouncement														*/
/*===========================================================================*/
void Request_CancelAnnouncement(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Announcement Cancel Request");
	Radio_Mngr_App_Request_AnnoCancel();
}

/*===========================================================================*/
/* void Request_RadioComponentStatus														*/
/*===========================================================================*/
void Request_RadioComponentStatus(void)
{
	//Radio_Mngr_App_Request_RadioStatus();
}

/*===========================================================================*/
/* HMIIF_IDataObject Request_RadioSettings														*/
/*===========================================================================*/
HMIIF_IDataObject Request_RadioSettings(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Radio Settings Request");
	return _radioSettingsStatusObject;
}

/*===========================================================================*/
/* Request_AMFMReTune														*/
/*===========================================================================*/
void Request_AMFMReTune(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]AMFM Retune Request");
	Radio_Mngr_App_Request_AMFMReTune();
}

/*===========================================================================*/
/* Request_BestPI_Info														*/
/*===========================================================================*/
HMIIF_IDataObject Request_BestPI_Info(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Best PI Info DiagRequest");
	return _radioBestPIStationObject;
}

/*===========================================================================*/
/* Request_ENG_Mode														*/
/*===========================================================================*/
void Request_ENG_Mode(Radio_Eng_Mode_Request e_EngModeRequest)
{
	if(e_EngModeRequest == RADIO_ENG_MODE_OFF)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Engg Mode OFF Request: ENG Mode OFF: %d",e_EngModeRequest);
	}
	else if(e_EngModeRequest == RADIO_ENG_MODE_ON)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Engg Mode ON Request: ENG Mode ON: %d",e_EngModeRequest);
	}
	else{/*FOR MISRA C*/}
    Radio_Mngr_App_Request_ENG_Mode((Te_Radio_Mngr_App_Eng_Mode_Request)e_EngModeRequest);
}

/*===========================================================================*/
/* Request_TuneByFrequency														*/
/*===========================================================================*/
void Request_TuneByFrequency(UINT32 Frequency)
{
	/*Clearing Channel Name as this tune by frequency is requested for AM or FM*/
	memset(DAB_Requested_ChannelName, 0, RADIO_MAX_CHANNEL_LABEL_SIZE);

	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Tune by Frequency Request: RequestedFrequency: %d",Frequency);
	Radio_Mngr_App_Request_TuneByFreq((Tu32)Frequency, DAB_Requested_ChannelName);
}

/*===========================================================================*/
/* Request_SRC_ActivateDeActivate														*/
/*===========================================================================*/
void Request_SRC_ActivateDeActivate(MODE_TYPE e_Mode, Radio_SRC_ActivateDeactivate_Request e_ActivateDeactivate)
{
	if(e_ActivateDeactivate == RADIO_SRC_ACTIVATE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Activate Request: %d for Band: %d", e_ActivateDeactivate, e_Mode);
	}
	else if(e_ActivateDeactivate == RADIO_SRC_DEACTIVATE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]De-Activate Request: %d for Band: %d", e_ActivateDeactivate, e_Mode);
	}
	else{/*FOR MISRA C*/}
	Sys_Request_SRC_Activate_DeActivate((Te_Sys_Mode_Type)e_Mode, (Te_Sys_SRC_Activate_DeActivate)e_ActivateDeactivate);
}

/*===========================================================================*/
/* Request_GetClockTime														*/
/*===========================================================================*/
void Request_GetClockTime(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Get Clock Time Request");
	Radio_Mngr_App_Request_GetClockTime();
}

/*===========================================================================*/
/* Request_CancelManualSTLUpdate														*/
/*===========================================================================*/
void Request_CancelManualSTLUpdate(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Cancel Manual Station list Request");
	Radio_Mngr_App_Request_ManualSTLUpdateCancel();
} 
/*===========================================================================*/
/* void Request_EnableAnnouncement_Info														*/
/*===========================================================================*/
void Request_EnableAnnouncement_Info(Radio_Switch_Request_Settings e_AnnoEnableInfoSwitch)
{
	if(e_AnnoEnableInfoSwitch == RADIO_SWITCH_REQUEST_ENABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Anno Info Switch Enable Request: EnableSwitchRequest: %d",e_AnnoEnableInfoSwitch);
	}
	else if(e_AnnoEnableInfoSwitch == RADIO_SWITCH_REQUEST_DISABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Anno Info Switch Disable Request: DisableSwitchRequest: %d",e_AnnoEnableInfoSwitch);
	}
	else{/*FOR MISRA C*/}
	Radio_Mngr_App_Request_EnableAnnouncementInfo((Te_Radio_Mngr_App_EnableInfoAnno_Switch)e_AnnoEnableInfoSwitch);
}
 
/*===========================================================================*/
/* void Request_RadioPowerOFF														*/
/*===========================================================================*/
void Request_RadioPowerOFF(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Power OFF Request");
	Radio_Mngr_App_Request_RadioPowerOFF();
}

/*===========================================================================*/
/* void Request_RadioPowerOFF														*/
/*===========================================================================*/
void Request_RadioPowerON(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Power ON Request");
	Radio_Mngr_App_Request_RadioPowerON();
}
 
/*===========================================================================*/
/* void Request_FactoryReset														*/
/*===========================================================================*/
void Request_FactoryReset(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Radio Factory Reset Request");
	Sys_Factory_Reset_Request();
} 

/*===========================================================================*/
/* HMIIF_IDataObject Request_RadioFirmwareVersion														*/
/*===========================================================================*/
HMIIF_IDataObject Request_RadioFirmwareVersion(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Radio Get Firmware Version Request");
	return _radioFirmwareVersionInfoObject;
} 

/*===========================================================================*/
/* HMIIF_IDataObject Request_RadioActivityStatus														*/
/*===========================================================================*/
HMIIF_IDataObject Request_RadioActivityStatus(void)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Radio Get Activity status Request");
	return _radioActivityStatusObject;
}
/*===========================================================================*/
/* void Request_RadioDABTuneByFrequency														*/
/*===========================================================================*/
void Request_RadioDABTuneByFrequency(UINT8* au8_ChannelName)
{
	/*Updating Frequency as 0 since this manual tune is for DAB and HMI is giving channel name as input*/
	Tu32 Frequency = 0;
	
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Tune by Frequency Request For DAB: RequestedChannelName: %s",au8_ChannelName);
	
	if(au8_ChannelName != NULL)
	{
		SYS_RADIO_MEMCPY(DAB_Requested_ChannelName, au8_ChannelName, RADIO_MAX_CHANNEL_LABEL_SIZE);	
	}else{/*FOR MISRA C*/}
	
	
	Radio_Mngr_App_Request_TuneByFreq((Tu32)Frequency, DAB_Requested_ChannelName);
		
}
/*===========================================================================*/
/* HMIIF_IDataObject Request_RadioSTLSearch														*/
/*===========================================================================*/
HMIIF_IDataObject Request_RadioSTLSearch(UINT8 RequestedChar)
{
	memset(_radioSTLSearchData_Display,0,sizeof(_radioSTLSearchData_Display[0]) * MAX_RADIO_STATIONS);
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Get Search Radio StationList Request with %c charcter", RequestedChar);
	
	Radio_Mngr_App_GetRadioStationListSearch(RequestedChar);
	return _radioSearchSTLDataObjectDisplay;
}

/*===========================================================================*/
/* void Request_playSelectStnFromSearchedSTL														*/
/*===========================================================================*/
void Request_playSelectStnFromSearchedSTL(UINT8 nIndex)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Select Station from Searched STL89 Request: RequestedIndex: %d",nIndex);
	
	Radio_Mngr_App_Request_Play_SelectStation_From_StlSearch(nIndex);
}

/*===========================================================================*/
/* void Request_MultiplexList_Switch														*/
/*===========================================================================*/
void Request_MultiplexList_Switch(Radio_Switch_Request_Settings e_MultiplexInfoSwitch)
{
	if(e_MultiplexInfoSwitch == RADIO_SWITCH_REQUEST_ENABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Multiplex Info Switch Enable Request: EnableSwitchRequest: %d",e_MultiplexInfoSwitch);
	}
	else if(e_MultiplexInfoSwitch == RADIO_SWITCH_REQUEST_DISABLE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Multiplex Info Switch Disable Request: DisableSwitchRequest: %d",e_MultiplexInfoSwitch);
	}
	else{/*FOR MISRA C*/}
	
	Radio_Mngr_App_Request_MultiplexList_Switch((Te_Radio_Mngr_App_Multiplex_Switch)e_MultiplexInfoSwitch);
}

/*===========================================================================*/
/* void Request_EnsembleSelection														*/
/*===========================================================================*/
HMIIF_IDataObject Request_EnsembleSelection(UINT8 nIndex)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Select Ensemble from Multiplex list Request: RequestedIndex: %d",nIndex);
	
	Radio_Mngr_App_Request_EnsembleSelect_From_MultiplexList(nIndex);

	return _radioStaionDataObjectDisplay;
}

/*===========================================================================*/
/* void Request_playSelectStnFromMultiplexList														*/
/*===========================================================================*/
void Request_playSelectStnFromMultiplexList(UINT8 nIndex)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Select Station from Ensemble list Request: RequestedIndex: %d",nIndex);
	
	Radio_Mngr_App_Request_StationSelect_From_EnsembleList(nIndex);
}


/*=============================================================================
    end of file
=============================================================================*/
