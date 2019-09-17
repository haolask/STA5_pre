/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file api.h																								*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager	Application														    *
*  Description			: This header file consists of api's declaration using by other layaers				*
*						  																					*
*																											*
*************************************************************************************************************/
#ifndef RAADIO_MNGR_APP_H
#define RAADIO_MNGR_APP_H

/*-----------------------------------------------------------------------------
				File Inclusions
-----------------------------------------------------------------------------*/
#include "hmi_if_app.h"
#include "radio_mngr_app_hsm.h"
/*-----------------------------------------------------------------------------
  Macros Definations
-----------------------------------------------------------------------------*/

#define RADIO_MNGR_APP_ASCII_SPACE								((Tu8)(0X20))

#define RADIO_MNGR_APP_UPPER_TO_LOWER							(32u)

#define RADIO_APP_NONRDS_SEARCH_ASCII_HASH						((Tu8)(0X23))	

#define RADIO_MNGR_APP_ASCII_UPPERCASE_A						((Tu8)(0X41))

#define RADIO_MNGR_APP_ASCII_UPPERCASE_Z						((Tu8)(0X5A))

#define RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_144					((Tu32)144u)

#define RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_531					((Tu32)531u)

#define RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_530					((Tu32)530u)

#define RADIO_MNGR_APP_DEFAULT_AM_FREQUENCY_522					((Tu32)522u)

#define RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_87500				((Tu32)87500u)

#define RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_87900				((Tu32)87900u)

#define RADIO_MNGR_APP_DEFAULT_FM_FREQUENCY_76000				((Tu32)76000u)

#define RADIO_MNGR_APP_DEFAULT_DAB_FREQUENCY					((Tu32)174928u)




/*Response ID Macros*/

#define RADIO_MNGR_APP_AMFM_STARTUP_DONE_RESID					((Tu16)(0X3502u))

#define RADIO_MNGR_APP_DAB_STARTUP_DONE_RESID					((Tu16)(0X3503u))

#define RADIO_MNGR_APP_AMFM_SHUTDOWN_DONE_RESID					((Tu16)(0x3504u))

#define RADIO_MNGR_APP_DAB_SHUTDOWN_DONE_RESID					((Tu16)(0x3505u))

#define RADIO_MNGR_APP_DAB_DESELECTBAND_DONE_RESID				((Tu16)(0x3506u))

#define RADIO_MNGR_APP_AMFM_DESELECTBAND_DONE_RESID				((Tu16)(0x3507u))

#define RADIO_MNGR_APP_DAB_SELECTBAND_DONE_RESID				((Tu16)(0x3508u))

#define RADIO_MNGR_APP_AMFM_SELECTBAND_DONE_RESID				((Tu16)(0x3509u))

#define RADIO_MNGR_APP_MUTE_DONE_RESID							((Tu16)(0x350Au))

#define RADIO_MNGR_APP_DEMUTE_DONE_RESID						((Tu16)(0x350Bu))

#define RADIO_MNGR_APP_DAB_PLAY_SELECT_STATION_DONE_RESID		((Tu16)(0x350Cu))

#define RADIO_MNGR_APP_AMFM_PLAY_SELECT_STATION_DONE_RESID		((Tu16)(0x350Du))

#define RADIO_MNGR_APP_DAB_STATIONLIST_DONE_RESID				((Tu16)(0x350Eu))

#define RADIO_MNGR_APP_AM_STATIONLIST_DONE_RESID				((Tu16)(0x350Fu))

#define RADIO_MNGR_APP_FM_STATIONLIST_DONE_RESID				((Tu16)(0x3510u))

#define RADIO_MNGR_APP_DAB_STATIONLIST_READ_DONE				((Tu16)(0x3511u))

#define RADIO_MNGR_APP_FM_STATIONLIST_READ_DONE					((Tu16)(0x3512u))

#define RADIO_MNGR_APP_AM_STATIONLIST_READ_DONE					((Tu16)(0x3513u))

#define RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID					((Tu16)(0x3514u))

#define RADIO_MNGR_APP_AMFM_TUNEUPDOWN_DONE_RESID				((Tu16)(0x3518u))

#define RADIO_MNGR_APP_DAB_TUNEUPDOWN_DONE_RESID				((Tu16)(0x3519u))

#define RADIO_MNGR_APP_AMFM_SEEKUPDOWN_DONE_RESID        		((Tu16)(0x351Au))

#define RADIO_MNGR_APP_DAB_SEEK_DONE_RESID                		((Tu16)(0x351Bu))

#define RADIO_MNGR_APP_DABTOFM_LINKING_ENABLE_RESID	            ((Tu16)(0x351Cu))

#define RADIO_MNGR_APP_AMFM_APP_AF_SWITCH_RESID					((Tu16)(0x351Du))

#define RADIO_MNGR_APP_DAB_ANNO_CANCEL_RESID					((Tu16)(0x351Eu))

#define RADIO_MNGR_APP_FM_DAB_SWITCH_RESID						((Tu16)(0x351Fu))		

#define RADIO_MNGR_APP_TA_SWITCH_RESID							((Tu16)(0x3520u))

#define RADIO_MNGR_APP_DAB_ANNO_SWITCH_RESID					((Tu16)(0x3521u))			

#define RADIO_MNGR_APP_AMFM_AF_TUNE_DONE_RESID					((Tu16)(0x3522u))

#define RADIO_MNGR_APP_DAB_AF_TUNE_DONE_RESID					((Tu16)(0x3523u))		

#define RADIO_MNGR_APP_AMFM_ANNO_CANCEL_RESID					((Tu16)(0x3524u))	

#define RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID					((Tu16)(0x3525u))

#define RADIO_MNGR_APP_DABTUNER_RESTART_DONE_RESIID				((Tu16)(0x3526u))

#define RADIO_MNGR_APP_FM_AFLIST_UPDATE_RESID					((Tu16)(0x3527u))

#define RADIO_MNGR_APP_DAB_AFLIST_UPDATE_RESID					((Tu16)(0x3528u))

#define RADIO_MNGR_APP_AMFM_CT_RESID							((Tu16)(0x3529u)) 

#define RADIO_MNGR_APP_DAB_ACTIVATE_DEACTIVATE_DONE_RESID		((Tu16)(0x3530u)) 

#define RADIO_MNGR_APP_DAB_APP_RDS_SETTINGS_RESID				((Tu16)(0x3531u))		

#define RADIO_MNGR_APP_INST_HSM_FACTORY_RESET_DONE_RESID		((Tu16)(0x3532u))

#define RADIO_MNGR_APP_AMFM_FACTORY_RESET_RESID					((Tu16)(0x3533u))

#define RADIO_MNGR_APP_DAB_FACTORY_RESET_RESID					((Tu16)(0x3534u))



/*Notification ID Macros*/

#define RADIO_MNGR_APP_DAB_TUNER_STATUS_NOTIFYID				((Tu16)(0x3700u))

#define RADIO_MNGR_APP_AMFM_TUNER_STATUS_NOTIFYID				((Tu16)(0x3701u))

#define RADIO_MNGR_APP_DAB_STATIONLIST_UPDATE_DONE_NOTIFYID		((Tu16)(0x3702u))

#define RADIO_MNGR_APP_AM_STATIONLIST_UPDATE_DONE_NOTIFYID		((Tu16)(0x3703u))

#define RADIO_MNGR_APP_FM_STATIONLIST_UPDATE_DONE_NOTIFYID		((Tu16)(0x3704u))

#define RADIO_MNGR_APP_DAB_FREQ_CHANGE_NOTIFYID					((Tu16)(0x3705u))

#define RADIO_MNGR_APP_AMFM_FREQ_CHANGE_NOTIFYID                ((Tu16)(0x3706u))

#define RADIO_MNGR_APP_DAB_DAB_STATUS_NOTIFYID               	((Tu16)(0x3707u))

#define RADIO_MNGR_APP_DAB_FM_LINKING_STATUS_NOTIFYID           ((Tu16)(0x3708u))

#define RADIO_MNGR_APP_DAB_DLS_DATA_NOTIFYID					((Tu16)(0x3709u))

#define RADIO_MNGR_APP_DAB_FM_BLENDING_STATUS_NOTIFYID          ((Tu16)(0x370Au))

#define RADIO_MNGR_APP_LINKING_STATUS_NOTIFYID                  ((Tu16)(0x370Bu))

#define RADIO_MNGR_APP_PI_QUALITY_NOTIFYID                      ((Tu16)(0x370Cu))

#define RADIO_MNGR_APP_BEST_PI_NOTIFYID                         ((Tu16)(0x370Du))

#define RADIO_MNGR_APP_PICODE_LIST_NOTIFYID                     ((Tu16)(0x370Eu))

#define RADIO_MNGR_APP_AMFM_FIND_SID_NOTIFYID                   ((Tu16)(0x370Fu)) 

#define RADIO_MNGR_APP_FMDAB_SID_QUALITY_NOTIFYID               ((Tu16)(0x3710u))

#define RADIO_MNGR_APP_START_FM_DAB_FOLLOWUP_NOTIFYID           ((Tu16)(0x3711u))

#define RADIO_MNGR_APP_FM_ANNOUNCEMENT_NOTIFYID					((Tu16)(0x3712u))

#define RADIO_MNGR_APP_DAB_ANNOUNCEMENT_NOTIFYID			    ((Tu16)(0x3713u))

#define RADIO_MNGR_APP_DAB_RECONFIGURATION_NOTIFYID				((Tu16)(0x3714u))

#define RADIO_MNGR_APP_AF_STATUS_NOTIFYID						((Tu16)(0x3715u))  

#define RADIO_MNGR_APP_BEST_PI_CHANGED_NOTIFYID				    ((Tu16)(0x3716u))

#define RADIO_MNGR_APP_AMFMTUNER_ABNORMAL_NOTIFYID				((Tu16)(0x3717u))

#define RADIO_MNGR_APP_DABTUNER_ABNORMAL_NOTIFYID				((Tu16)(0x3718u))		

#define RADIO_MNGR_APP_FM_AF_LEARN_MEM_NOTIFYID					((Tu16)(0x3719u))

#define RADIO_MNGR_APP_DAB_AF_LEARN_MEM_NOTIFYID				((Tu16)(0x3720u))

#define RADIO_MNGR_APP_FM_AF_SIGLOST_NOTIFYID					((Tu16)(0x3721u))

#define RADIO_MNGR_APP_DAB_AF_SIGLOST_NOTIFYID					((Tu16)(0x3722u))

#define RADIO_MNGR_APP_STOP_FM_DAB_LINKING_NOTIFYID				((Tu16)(0x3723u))

#define RADIO_MNGR_APP_FMDAB_SID_STATION_NOTIFYID               ((Tu16)(0x3724u))

#define RADIO_MNGR_APP_UPNOT_RECEIVED_NOTIFYID				    ((Tu16)(0x3725u))

#define RADIO_MNGR_APP_DAB_VERSION_NOTIFYID						((Tu16)(0x3727u))

#define RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION_NOTIFYID			((Tu16)(0x3728u))

#define RADIO_MNGR_APP_DAB_SLS_DATA_NOTIFYID					((Tu16)(0x3729u))

/*-----------------------------------------------------------------------------
				Type Defintions
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
   Public Function Declaration
-----------------------------------------------------------------------------*/

void Radio_Mngr_App_Msg_HandleMsg(Ts_Sys_Msg* pst_msg);

void Radio_Mngr_App_Inst_Hsm_Stop(void);

void Radio_Mngr_App_Inst_Hsm_Start_Response(Te_RADIO_ReplyStatus e_InstHSMReplystatus);

void Radio_Mngr_App_Responsse_Inst_Hsm_Stop(Te_RADIO_ReplyStatus e_InstHSMReplystatus);

Ts_Sys_Msg* MSG_Update(Tu16 u16_cid, Tu16 u16_msgid);

void UpdateParameterIntoMsg(char *pu8_data,const void *vp_parameter,Tu8 u8_ParamLength,Tu16 *pu16_Datalength);

void ExtractParameterFromMsg(void *vp_Parameter, const char *pu8_DataSlot, Tu8 u8_ParamLength, Tu32 *pu32_index);

void Update_Radio_Mngr_StationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band);

void Update_ComponentName(Ts_Radio_Mngr_App_DAB_CurrentStationInfo* st_DAB_CurrentStationInfo, Ts_Radio_Mngr_App_ComponentName* st_ComponentName);

void Update_StationInfo_with_index(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band, Tu8 u8_Index);

void Update_LSM_Station_Info(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst);

void Update_Tunable_Station_Info_with_LSM(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst);

void Update_CurrentStationInfo_with_TunableStn(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst);

void Update_LSM_TunableStn_with_CurrentStationInfo(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst);

void Update_Tunable_Station_with_Preset_Index(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Tu8 u8_Preset_Recall_Index);

void Update_TunuableStation_with_DAB_Station(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Mngr_Update_OriginalStn(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band);

void Update_MatchedStationListIndex(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Mngr_App_ClearCheckParameters(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

Tu8 Radio_Mngr_App_SelectBandConditionCheck(Te_Radio_Mngr_App_Band e_requestedBand, Te_Radio_Mngr_App_Band e_activeBand, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

Tbool Radio_Mngr_App_PresetRecallConditionCheck(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

Tu8 Radio_Mngr_App_GetRadioStationList(void);

void Preset_Store_with_Current_Station(Tu8 u8_Preset_Store_Index);

void Radio_Mngr_App_GetPresetList(void);

void Radio_Mngr_App_Timer_ClearCheckParameters(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Manager_EEPROM_Log(Tu8 u8_EEPROM_Write_Status);

void Radio_Manager_App_Write_Flash_Data(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Mngr_App_AudioChange(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_ReqAudioChangeBand);

void Radio_Manager_App_Update_PresetMixedList_AFTune(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Update_MatchedPresetInStationListIndex(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Manager_App_StationList_Search(Tu8 u8_RequestedChar, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Mngr_App_GetRadioStationListSearch(Tu8 u8_RequestedChar);

void Update_Searched_STL_StationInfo_with_index(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst);

void Radio_Mngr_App_Update_ServiceNumber_In_EnsembleList(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst);

void Radio_Mngr_App_CreateNormalRadioStationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Mngr_App_CreateMultiplexRadioStationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Mngr_App_Request_EnsembleSelect_From_MultiplexList(Tu8 u8_EnsembleIndex);

void Update_Multiplex_Service_StationInfo_with_index(Ts_Radio_Mngr_App_Inst_Hsm * pst_me_radio_mngr_inst);

void Radio_Mngr_App_Find_Greatest_RSSI(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Tu8 u8_DAB_MatchedStn_Start_index, Tu8 u8_DAB_MatchedStn_End_index, Tu8 u8_NoOfServiceComponent);
void Radio_Mngr_IntrMsg(Ts_Sys_Msg *msg);
#endif /*  End of RAADIO_MNGR_APP_H */

/*=============================================================================
    end of file
=============================================================================*/


