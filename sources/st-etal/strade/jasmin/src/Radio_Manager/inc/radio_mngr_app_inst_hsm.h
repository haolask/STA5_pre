/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_inst_hsm.h																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager	Application														    *
*  Description			: This header file consists of declaration of all function handlers of HSM			*
*						  radio_mngr_app_inst_hsm and INST HSM structure 									*
*																											*
*************************************************************************************************************/
#ifndef RADIO_MNGR_APP_INST_HSM_H
#define RADIO_MNGR_APP_INST_HSM_H
/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "dab_app_msg_id.h"
#include "amfm_app_types.h"
/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/
/**
 * @brief Number of radio manager band available
 * This can be used as a count for number of radio manager bands available
 */
#define RADIO_MNGR_APP_INST_HSM									((Tu16)(0x3999u))

#define RADIO_MNGR_APP_NOOF_BANDS                               ((Tu8)(3u))

#define RADIO_MNGR_APP_INST_START								((Tu16)(0x3300u))

#define RADIO_MNGR_APP_INST_STOP								((Tu16)(0x3301u))

#define RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID				((Tu16)(0x3302u))

#define RADIO_MNGR_APP_SELECTBAND_REQID							((Tu16)(0x3303u))

#define RADIO_MNGR_APP_SEEKUPDOWN_REQID                         ((Tu16)(0x3304u))

#define RADIO_MNGR_APP_GET_AFLIST_DIAG_REQID					((Tu16)(0x3305u))

#define RADIO_MNGR_APP_GET_QUALITY_DIAG_REQID					((Tu16)(0x3306u))

#define RADIO_MNGR_APP_ENABLE_DABFMLINKING_REQID				((Tu16)(0x3307u))

#define RADIO_MNGR_APP_GET_CURRENTSTATIONINFO_DIAG_REQID		((Tu16)(0x3308u))

#define RADIO_MNGR_APP_OBTAIN_STATIONLIST_DIAG_REQID			((Tu16)(0x3309u))

#define RADIO_MNGR_APP_POWER_OFF_REQID							((Tu16)(0x330Au))

#define RADIO_MNGR_APP_PRESET_RECALL_REQID						((Tu16)(0x330Bu))

#define RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID				((Tu16)(0x330Cu)) 

#define RADIO_MNGR_APP_AF_SWITCH_REQID							((Tu16)(0x330Eu))

#define RADIO_MNGR_APP_TUNEUPDOWN_REQID							((Tu16)(0x330Fu))  

#define RADIO_MNGR_APP_TA_ANNO_SWITCH_REQID						((Tu16)(0x3310u))	

#define RADIO_MNGR_APP_ANNO_CANCEL_REQID						((Tu16)(0x3311u))		

#define RADIO_MNGR_APP_AMFM_RETUNE_REQID						((Tu16)(0x3312u))

#define RADIO_MNGR_APP_ENG_MODE_REQID							((Tu16)(0x3313u))

#define RADIO_MNGR_APP_POWER_ON_REQID							((Tu16)(0x3314u))

#define RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID					((Tu16)(0x3315u))

#define RADIO_MNGR_APP_SRC_ACTIVATE_DEACTIVATE_REQID			((Tu16)(0x3316u))

#define RADIO_MNGR_APP_GETCLOCKTIME_REQID						((Tu16)(0x3317u))

#define RADIO_MNGR_APP_MANUAL_STLUPDATE_CANCEL_REQID			((Tu16)(0x3318u))

#define RADIO_MNGR_APP_INFO_ANNO_SWITCH_REQID					((Tu16)(0x3319u))

#define RADIO_MNGR_APP_LOW_QUALITY_TIMEOUT						((Tu16)(0x331Au))

#define RADIO_MNGR_APP_PI_DECODE_TIMEOUT						((Tu16)(0x331Bu))

#define RADIO_MNGR_APP_FACTORY_RESET_REQID						((Tu16)(0x331Cu))

#define RADIO_MNGR_APP_INST_HSM_FACTORY_RESET_REQID				((Tu16)(0x331Du))

#define RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID			((Tu16)(0x331Eu))

#define RADIO_MNGR_APP_MULTIPLEX_SWITCH_REQID					((Tu16)(0x3320u))
#define RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID	((Tu16)(0x3321u))



#define RADIO_MNGR_APP_ANNO_CANCEL_REQ							RADIO_MNGR_APP_SELECTBAND_REQID:\
														   case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:\
														   case RADIO_MNGR_APP_PRESET_RECALL_REQID:\
														   case RADIO_MNGR_APP_SEEKUPDOWN_REQID:\
														   case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:\
														   case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:\
														   case RADIO_MNGR_APP_TUNEUPDOWN_REQID:\
														   case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:\
														   case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:\
														   case RADIO_MNGR_APP_POWER_OFF_REQID


#define RADIO_MNGR_APP_SCAN_CANCEL_REQ                          RADIO_MNGR_APP_SEEKUPDOWN_REQID:\
                                                           case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:\
                                                           case RADIO_MNGR_APP_SELECTBAND_REQID:\
                                                           case RADIO_MNGR_APP_PRESET_RECALL_REQID:\
														   case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:\
														   case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:\
														   case RADIO_MNGR_APP_POWER_OFF_REQID:\
														   case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:\
														   case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:\
														   case RADIO_MNGR_APP_TUNEUPDOWN_REQID
														   
#define RADIO_MNGR_APP_SEEK_CANCEL_REQ                       	RADIO_MNGR_APP_SEEKUPDOWN_REQID:\
                                                           case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:\
                                                           case RADIO_MNGR_APP_SELECTBAND_REQID:\
                                                           case RADIO_MNGR_APP_PRESET_RECALL_REQID:\
														   case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:\
														   case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:\
														   case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:\
														   case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:\
														   case RADIO_MNGR_APP_POWER_OFF_REQID:\
														   case RADIO_MNGR_APP_TUNEUPDOWN_REQID
														   
														   
#define RADIO_MNGR_APP_AF_TUNE_CANCEL_REQ                       RADIO_MNGR_APP_SEEKUPDOWN_REQID:\
                                                           case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:\
                                                           case RADIO_MNGR_APP_SELECTBAND_REQID:\
                                                           case RADIO_MNGR_APP_PRESET_RECALL_REQID:\
														   case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:\
														   case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:\
														   case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:\
														   case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:\
														   case RADIO_MNGR_APP_POWER_OFF_REQID:\
														   case RADIO_MNGR_APP_TUNEUPDOWN_REQID

#define RADIO_MNGR_APP_TUNE_CANCEL_REQ                          RADIO_MNGR_APP_SEEKUPDOWN_REQID:\
                                                           case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:\
                                                           case RADIO_MNGR_APP_SELECTBAND_REQID:\
                                                           case RADIO_MNGR_APP_PRESET_RECALL_REQID:\
														   case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:\
														   case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:\
														   case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:\
														   case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:\
														   case RADIO_MNGR_APP_POWER_OFF_REQID:\
														   case RADIO_MNGR_APP_TUNEUPDOWN_REQID

#define RADIO_MNGR_APP_INST_REQID_CASES						    RADIO_MNGR_APP_INST_START:\
														   case RADIO_MNGR_APP_INST_STOP:\
														   case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:\
														   case RADIO_MNGR_APP_SELECTBAND_REQID:\
														   case RADIO_MNGR_APP_SEEKUPDOWN_REQID:\
														   case RADIO_MNGR_APP_GET_AFLIST_DIAG_REQID:\
														   case RADIO_MNGR_APP_GET_QUALITY_DIAG_REQID:\
														   case RADIO_MNGR_APP_ENABLE_DABFMLINKING_REQID:\
														   case RADIO_MNGR_APP_GET_CURRENTSTATIONINFO_DIAG_REQID:\
														   case RADIO_MNGR_APP_OBTAIN_STATIONLIST_DIAG_REQID:\
														   case RADIO_MNGR_APP_PRESET_RECALL_REQID:\
														   case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:\
														   case RADIO_MNGR_APP_AF_SWITCH_REQID:\
														   case RADIO_MNGR_APP_TUNEUPDOWN_REQID:\
														   case RADIO_MNGR_APP_TA_ANNO_SWITCH_REQID:\
														   case RADIO_MNGR_APP_ANNO_CANCEL_REQID:\
														   case RADIO_MNGR_APP_AMFM_RETUNE_REQID:\
														   case RADIO_MNGR_APP_ENG_MODE_REQID:\
														   case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:\
														   case RADIO_MNGR_APP_SRC_ACTIVATE_DEACTIVATE_REQID:\
														   case RADIO_MNGR_APP_GETCLOCKTIME_REQID:\
														   case RADIO_MNGR_APP_MANUAL_STLUPDATE_CANCEL_REQID:\
														   case RADIO_MNGR_APP_INFO_ANNO_SWITCH_REQID:\
														   case RADIO_MNGR_APP_LOW_QUALITY_TIMEOUT:\
														   case RADIO_MNGR_APP_PI_DECODE_TIMEOUT:\
														   case RADIO_MNGR_APP_POWER_ON_REQID:\
														   case RADIO_MNGR_APP_POWER_OFF_REQID:\
														   case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:\
														   case RADIO_MNGR_APP_INST_HSM_FACTORY_RESET_REQID:\
														   case RADIO_MNGR_APP_MULTIPLEX_SWITCH_REQID:\
														   case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID
														   

												
														



#define RADIO_MNGR_APP_INST_RESID_DONE_CASES					 RADIO_MNGR_APP_AMFM_STARTUP_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_STARTUP_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_SHUTDOWN_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_SHUTDOWN_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_DESELECTBAND_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_DESELECTBAND_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_SELECTBAND_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_SELECTBAND_DONE_RESID:\
															case RADIO_MNGR_APP_MUTE_DONE_RESID:\
															case RADIO_MNGR_APP_DEMUTE_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_PLAY_SELECT_STATION_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_PLAY_SELECT_STATION_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_STATIONLIST_DONE_RESID:\
															case RADIO_MNGR_APP_AM_STATIONLIST_DONE_RESID:\
															case RADIO_MNGR_APP_FM_STATIONLIST_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_STATIONLIST_READ_DONE:\
															case RADIO_MNGR_APP_FM_STATIONLIST_READ_DONE:\
															case RADIO_MNGR_APP_AM_STATIONLIST_READ_DONE:\
															case RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_TUNEUPDOWN_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_TUNEUPDOWN_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_SEEKUPDOWN_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_SEEK_DONE_RESID:\
															case RADIO_MNGR_APP_DABTOFM_LINKING_ENABLE_RESID:\
															case RADIO_MNGR_APP_AMFM_APP_AF_SWITCH_RESID:\
															case RADIO_MNGR_APP_FM_DAB_SWITCH_RESID:\
															case RADIO_MNGR_APP_TA_SWITCH_RESID:\
															case RADIO_MNGR_APP_DAB_ANNO_SWITCH_RESID:\
															case RADIO_MNGR_APP_AMFM_AF_TUNE_DONE_RESID:\
															case RADIO_MNGR_APP_DAB_AF_TUNE_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_ANNO_CANCEL_RESID:\
															case RADIO_MNGR_APP_DAB_ANNO_CANCEL_RESID:\
															case RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID:\
															case RADIO_MNGR_APP_DABTUNER_RESTART_DONE_RESIID:\
															case RADIO_MNGR_APP_FM_AFLIST_UPDATE_RESID:\
															case RADIO_MNGR_APP_DAB_AFLIST_UPDATE_RESID:\
															case RADIO_MNGR_APP_DAB_ACTIVATE_DEACTIVATE_DONE_RESID:\
															case RADIO_MNGR_APP_AMFM_CT_RESID:\
															case RADIO_MNGR_APP_DAB_APP_RDS_SETTINGS_RESID
														




#define RADIO_MNGR_APP_INST_NOTIFYID_CASES						  RADIO_MNGR_APP_DAB_TUNER_STATUS_NOTIFYID:\
															case RADIO_MNGR_APP_AMFM_TUNER_STATUS_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_STATIONLIST_UPDATE_DONE_NOTIFYID:\
															case RADIO_MNGR_APP_AM_STATIONLIST_UPDATE_DONE_NOTIFYID:\
															case RADIO_MNGR_APP_FM_STATIONLIST_UPDATE_DONE_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_FREQ_CHANGE_NOTIFYID:\
															case RADIO_MNGR_APP_AMFM_FREQ_CHANGE_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_FM_LINKING_STATUS_NOTIFYID:	\
															case RADIO_MNGR_APP_DAB_DLS_DATA_NOTIFYID:		\
															case RADIO_MNGR_APP_DAB_SLS_DATA_NOTIFYID:		\
															case RADIO_MNGR_APP_DAB_FM_BLENDING_STATUS_NOTIFYID:	\
															case RADIO_MNGR_APP_LINKING_STATUS_NOTIFYID:\
															case RADIO_MNGR_APP_PI_QUALITY_NOTIFYID:\
															case RADIO_MNGR_APP_BEST_PI_NOTIFYID:\
															case RADIO_MNGR_APP_PICODE_LIST_NOTIFYID:\
															case RADIO_MNGR_APP_AMFM_FIND_SID_NOTIFYID:\
															case RADIO_MNGR_APP_FMDAB_SID_QUALITY_NOTIFYID:\
															case RADIO_MNGR_APP_START_FM_DAB_FOLLOWUP_NOTIFYID:\
															case RADIO_MNGR_APP_FM_ANNOUNCEMENT_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_ANNOUNCEMENT_NOTIFYID:\
															case RADIO_MNGR_APP_AF_STATUS_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_RECONFIGURATION_NOTIFYID:\
															case RADIO_MNGR_APP_BEST_PI_CHANGED_NOTIFYID:\
															case RADIO_MNGR_APP_AMFMTUNER_ABNORMAL_NOTIFYID:\
															case RADIO_MNGR_APP_DABTUNER_ABNORMAL_NOTIFYID:\
															case RADIO_MNGR_APP_FM_AF_LEARN_MEM_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_AF_LEARN_MEM_NOTIFYID:\
															case RADIO_MNGR_APP_FM_AF_SIGLOST_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_AF_SIGLOST_NOTIFYID:\
															case RADIO_MNGR_APP_STOP_FM_DAB_LINKING_NOTIFYID:\
															case RADIO_MNGR_APP_FMDAB_SID_STATION_NOTIFYID:\
															case RADIO_MNGR_APP_DAB_DAB_STATUS_NOTIFYID:\
															case RADIO_MNGR_APP_UPNOT_RECEIVED_NOTIFYID:\
															case RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION_NOTIFYID
														

 
/*-----------------------------------------------------------------------------
    Type definitions
-----------------------------------------------------------------------------*/

/*@brief Structure definition of hsm object*/
typedef struct
{
	Ts_hsm												st_radio_mngr_inst_hsm;						/*The base HSM object, have to be the first member of this struct (handels state transitions) */								/*Current pending message */									/* Instance HSM component ID*/
    Tu8 									 			str_state[100];								/*Array to show instant Hsm state*/
    const Tu8											*u8p_curr_state_str;						/*Pointer to the name of the current state handler */
	Tu16												u16_cid;									/*Instance HSM component ID */
	Tu8													u8_Settings;								/*0bit- Anno; 1bit-DABFM; 2bit- RDS*/
	Tu8													u8_StartType_BlockOne;						/*NVM read/write starttype address location first byte*/
	Tu8													u8_StartType_BlockTwo;						/*NVM read/write starttype address location second byte*/
	Tu8													u8_StartType_BlockThree;					/*NVM read/write starttype address location Third byte*/
	Ts_Radio_Mngr_App_RadioStationList					st_RadioStationList;						/*Radio Station List for Writing into NVM at Shutdown Time */
	Ts_Radio_Mngr_App_DAB_MultiplexStationList	 		st_DABEnsembleList;							/*Radio Ensemble List for Writing into NVM at Shutdown Time */
	Ts_Radio_Mngr_App_Preset_Mixed_List					st_PrestMixedList;							/*Preset Mixed List for Writing into NVM at Shutdown Time */
	Ts_Radio_Mngr_Tunuable_Station_Info					st_LSM_Station_Info;						/*LSM Station Info for Writing into NVM at Shutdown Time */
	Ts_Radio_Mngr_Tunuable_Station_Info					st_Tunable_Station_Info;					/*Tunable Station Information for Tune Request */
	Ts_Radio_Mngr_App_APP_TO_RM_FM_AFList				st_APPtoRM_FMAFList;						/*APP to RM FM AF List*/
	Ts_Radio_Mngr_App_AMFM_CurrentStationInfo			st_AMFM_currentstationinfo;					/*Currently tuner station information of AM/FM */
	Ts_Radio_Mngr_App_AMFM_CurrentStationInfo			st_AMFM_EONAnno_stationinfo;				/*Currently tuned announcement station information of FM */
	Ts_Radio_Mngr_App_AMFM_CurrentStationInfo			st_AMFM_FreqChangeNotinfo;					/*Currently tuned frequency information of AM/FM */
	Ts_Radio_Mngr_App_ComponentName						st_CurrentStationName;						/*Current station name with service and service component name */
	Ts_Radio_Mngr_App_DAB_AFList						st_DAB_AFList;								/*Structure for DAB AF list*/
	Ts_Radio_Mngr_App_ComponentName						st_DAB_Anno_CurrentStationName;				/*Announcement station name with service and service component name */
	Ts_Radio_Mngr_App_DAB_CurrentStationInfo			st_DAB_Anno_currentstationinfo;				/*Currently tuned announcement station information of DAB */
	Ts_Radio_Mngr_App_DAB_CurrentStationInfo			st_DAB_currentstationinfo;					/*Currently tuner station information of DAB */
	Ts_Radio_Mngr_App_DAB_CurrentStationInfo			st_DAB_FreqChangeNotinfo;					/*Currently tuned frequency information of DAB */
	Ts_Radio_Mngr_App_DAB_TunerStatusNotify				st_DAB_TunerNotify;							/*Structure to store  DAB Tuner notification*/
	Ts_Radio_Mngr_App_DAB_Tunable_StationInfo			st_DABTunableStation;
	Ts_Radio_Mngr_App_DLS_Data							st_DLS_Data;
	Ts_Radio_Mngr_App_FM_AFList							st_FM_AFList;								/*Structure for FM AF list*/
	Ts_Radio_Mngr_App_CT_Info							st_AMFMTUNER_CTInfo;						/*AMFM Tuner Clock Information*/
	Ts_Sys_Msg 						                    st_msg_cpy;
	Ts_Radio_Mngr_App_MultiplexStationListInfo 			st_MultiplexStlView;						/*Structure for DAB Stl Multiplex view*/
	Ts_Radio_Mngr_App_DAB_SL 							st_NormalStnView;							/*Structure for DAB Stl Normal view*/
	Ts_Radio_Mngr_App_Common_Station_Indexes			st_PI_Station_Indexes;						/*Structure of Common stations information array of indexes and Number of common stations */
	Ts_AMFM_App_PIList						            st_Radio_Mngr_App_PiList;
	Ts_Radio_Mngr_App_Common_Station_Indexes			st_SID_Station_Indexes;						/*Structure of Common stations information array of indexes and Number of common stations */
	Ts_Radio_Mngr_App_STL_Search 						st_StationList_Search;						/*Structure for stationlist search*/
	Te_Radio_Mngr_App_Band								e_activeBand;								/*Current tuned/active band */
	Te_RADIO_ReplyStatus								e_ActivateDeactivate_ReplyStatus;			/*Activate Deactivate ReplyStaus*/
	Te_Radio_Mngr_App_Activity_Status					e_Activity_Status;							/*Activity status*/
	Te_RADIO_ReplyStatus								e_ReplyStatus;
	Te_Radio_Mngr_App_AF_Status							e_AF_Status;
	Te_Radio_Mngr_App_SRC_ActivateDeActivate			e_AMActiveDeActiveStatus;					/*AM Band Active/DeActive Status*/
	Te_Radio_Mngr_App_FG_Signal_Status					e_AMFMSignalStatus;							/*Enum variable for Foreground AMFM band signal status*/
	Te_RADIO_ReplyStatus								e_AMFM_TA_Switch_Status;
	Te_Radio_Mngr_App_Anno_Cancel_Request_Type 			e_AnnoCancel_Request_Type;
	Te_Radio_Mngr_App_Anno_Status						e_Anno_Status;
	Te_Radio_Mngr_App_Anno_Status						e_Anno_Status_Type;
	Te_Radio_Mngr_App_Band								e_AnnoCancelRequestedBand;
	Te_Radio_Mngr_App_BestPI_Type                       e_BestPI_Type;
	Te_Radio_Mngr_App_Band								e_Curr_Audio_Band;							/*Which give the status of Current Enabled Audio Band*/
	Te_Radio_Mngr_App_SRC_ActivateDeActivate			e_DABActiveDeActiveStatus;					/*DAB Band Active/DeActive Status*/
	Te_RADIO_ReplyStatus								e_DAB_AFTuneReplyStatus;
	Te_Radio_Mngr_App_DAB_Alt_Status					e_DAB_ALT_Status;							/*Notification for DAB-DAB start, status*/
	Te_RADIO_DABFM_LinkingStatus						e_DABFM_LinkingStatus;						/*DAB-FM linking status */
	Te_Radio_Mngr_App_DABFMLinking_Switch				e_DABFMLinking_Switch;						/*DAB FM Settings Notification*/
	Te_Radio_Mngr_App_DABFMLinking_Switch				e_DABFMLinking_Switch_Request;				/*DAB FM Settings Request*/
	Te_Radio_Mngr_App_FG_Signal_Status					e_DABSignalStatus;							/*Enum variable for Foreground DAB band signal status*/
	Te_RADIO_ReplyStatus								e_DAB_TA_Switch_Status;
	Te_RADIO_ReplyStatus								e_DABtoFMFollowingStatus;
	Te_Radio_Mngr_App_DAB_UpNotification_Status			e_DAB_UpNot_Status;							/*DAB Up Notification Status*/
	Te_Radio_Mngr_App_Eng_Mode_Request					e_EngMode_Switch;							/*ENG MODE switch Requested for ON/OFF */
	Te_RADIO_ReplyStatus								e_FM_AFTuneReplyStatus;
	Te_Radio_Mngr_App_DABFM_SID_Type					e_FMDAB_SID_Type;
	Te_RADIO_ReplyStatus								e_FMtoDABFollowingStatus;
	Te_Radio_Mngr_App_FMDABLinking_Stop_Type			e_FMtoDABLinkingStopType;					/*FM-DAB Linking stop type*/
	Te_RADIO_ReplyStatus								e_GetAMstationlistreplystatus;				/*AM Station reply status from lower layer */
	Te_RADIO_ReplyStatus								e_GetFMstationlistreplystatus;				/*FM Station reply status from lower layer */
	Te_RADIO_ReplyStatus								e_GetDABstationlistreplystatus;				/*DAB Station reply status from lower layer */
	Te_RADIO_AMFMTuner_Status							e_AMFMTunerStatus;							/*AMFM TUNER Status*/
	Te_Radio_Mngr_App_EnableInfoAnno_Switch				e_Info_Anno_Switch;							/*Info Anno switch Notification */
	Te_Radio_Mngr_App_EnableInfoAnno_Switch				e_Info_Anno_Switch_Request;					/*Info Anno switch request type */
	Te_RADIO_ReplyStatus								e_Info_Switch_Status;						/*Info Anno switch reply status */
	Te_Radio_Mngr_App_Band								e_lastband;									/*Last active band */
	Te_Radio_Mngr_App_LearnMemAFStatus					e_LearnAFStatus;							/*AF Operation Status in Udpated Learn Memory*/
	Te_Radio_Mngr_App_Market						    e_Market;
	Te_Radio_Mngr_App_Multiplex_Switch					e_MultiplexSettings;						/*DAB STL view Settings Request*/
	Te_RADIO_ReplyStatus								e_MultiplexSettings_ReplyStatus;			/*DAB STL view Settings Response*/
	Te_RADIO_ReplyStatus								e_PowerOnReplyStatus;						/*Power On Reply status*/
	Te_RADIO_ReplyStatus								e_PowerOffReplyStatus;						/*Power Off Reply status*/
	Te_RADIO_ReplyStatus								e_PresetRecallReplayStatus;					/*Preset recall replay status */
	Te_Radio_Mngr_App_PSNChange							e_PSNChangeFlag_not;						/*PSN change notification flag before/after blending*/
	Te_Radio_Mngr_App_Req_Id							e_Radio_Mngr_App_Req_Id;					/*Radio Manager Req ID at which usecase radio manager is executing */
	Te_Radio_Mngr_App_RDSSettings						e_RDSSettings;								/*RDS Settings Notification*/					
	Te_Radio_Mngr_App_RDSSettings						e_RDSSettings_Request;						/*RDS FM Settings Request*/
	Te_RADIO_ReplyStatus								e_RDSSettings_AMFM_ReplyStatus;
	Te_RADIO_ReplyStatus								e_RDSSettings_DAB_ReplyStatus;
	Te_Radio_Mngr_App_Band								e_requestedBand;							/*Requested band to activate */
	Te_RADIO_ReplyStatus					            e_DABTunerRestartReplyStatus;				/*DABTUNER Restart Reply Status*/
	Te_RADIO_Comp_Status								e_DABTunerStatus;							/*DABTUNER Status*/
	Te_Radio_Mngr_App_Band								e_ScanCancelRequestedBand;					/*Requested band to activate due to scan cancel request */	
	Te_Radio_Mngr_App_Band								e_SeekCancelRequestedBand;					/*Requested band to activate due to seek cancel request */
	Te_Radio_Mngr_App_Scan_Cancel_Request_Type			e_ScanCancel_Request_Type;					/*Scan cancel Request Types*/
	Te_RADIO_DirectionType								e_SeekDirection;                            /*Seek request direction */
	Te_RADIO_DirectionType								e_Seek_NewReq_Direction;                    /*Requested band to activate due to seek cancel request */
	Te_RADIO_ReplyStatus								e_SeekReplyStatus;                          /*Seek up/down request reply status from lower layer */
	Te_RADIO_ReplyStatus								e_SelectStationReplyStatus;					/*Select status reply status from lower layer */
	Te_Radio_Mngr_App_SRC_ActivateDeActivate			e_SRCActivateDeactivate;					/*Enable/Desable Request*/
	Te_Radio_Mngr_App_Band                              e_SRC_ActivateDeactivate_Band;				/*Source Activate/Deactivate Band from the HMI*/
	Te_Radio_Mngr_App_StrategyFlow						e_StrategyFlow;								/*To know the strategy flow*/
	Te_Radio_Mngr_App_StationNotAvail_StrategyStatus	e_StrategyStatus;							/*Strategy Status flag ON/OFF*/
	Te_Radio_Mngr_App_EnableTAAnno_Switch				e_TA_Anno_Switch;							/*TA Settings Notification*/
	Te_Radio_Mngr_App_EnableTAAnno_Switch				e_TA_Anno_Switch_Request;					/*TA FM Settings Request*/
	Te_Radio_Mngr_App_EnableTAAnno_Switch				e_TA_Anno_Switchstatus;
	Te_RADIO_ReplyStatus								e_TuneByFreqReplyStatus;					/*Tune By Freq reply status from lower layer */
	Te_Radio_Mngr_App_Band								e_TuneCancelRequestedBand;					/*Requested band to activate due to Tune cancel request */
	Te_RADIO_DirectionType								e_TuneUpDownDirection;
	Te_RADIO_ReplyStatus								e_TuneUpDownReplyStatus;
	Tu32							                    u32_Implicit_Sid;
	Tu32                                            	u32_NVM_Read_Write_Bytes;       			/*Number of bytes Read or Write from NVM*/
	Tu32												u32_ReqFreq;								/*Tune by Freq requested Frequency*/
	Tu32							                    u32_slot;
	Tu16							                    u16_BestPi;
    Tu16							                    u16_FMtoDAB_PI;
	Tbool												b_AF_Tune_Cancel_Status;
	Tbool												b_AM_BandStatus;							/*Status variable to know AM Band status w.r.t Diag Input Info*/
	Tbool												b_Check_AudioSwitchNeed;
	Tu8													u8_CurrentlyPlayingServiceInEnsemble;
	Tbool												b_DAB_BandStatus;							/*Status variable to know DAB Band status w.r.t Activate/DeActivate and CompInfo*/
	Tbool												b_DAB_StartStatus;
	Tbool												b_EEPROM_Status;							/*EEPROM Validation Status*/
	Tbool												b_FM_StartStatus;
	Tu8								                    u8_FMtoDAB_SID_Quality;
	Tbool												b_FM_To_DAB_Linking_Status;
	Tu8													u8_EnsembleSelect_Req_Status;				/*Flag to determine to highlight on preset matched services in given index ensemble */
	Tu8 												u8_GetStl_Ret_Value;						/*Value to know which structure HMI should return in get stl req*/
	Tu8													u8_Index;									/*Current Playing station Index*/
	Tbool												b_InfoAnnoFlag;							/*Flag to determine Info or TA Anno response*/
	Tbool							                    b_Internal_Msg_Flag;   
	Tu8								                    u8_Linktype;
	Tu8													u8_MatchedPresetL_Stn_Index;
	Tu8													u8_MatchedStL_Stn_Index;
	Tu8													u8_NVM_LastMode_ReadWriteStatus;			/*Last mode Write/Read Reply status of NVM*/
	Tu8                                             	u8_NVM_ReplyStatus;             			/*NVM Reply Status for NVM Read write functions*/		
	Tbool												b_PIDecode_TimerFlag;		
	Tu8								                    u8_PiQuality;
	Tu8													u8_Preset_Recall_Index;						/*Preset Recall index */						
	Tu8													u8_Preset_StnlistSelectInfo_NotifyCheck;	/*Flag to check & display PSN from preset/station list instead of tuner decoding of PSN*/
	Tu8													u8_Preset_Store_Index_Listen;				/*Preset Store index which is listening */
	Tu8													u8_Preset_Clear_Index;						/*Station index which need to clear from preset list */
	Tu32												u32_Quality;
    Tu32												u32_QualityMin;
    Tu32								                u32_QualityMax;
	Tu8													u8_Original_Band;
	Tbool												b_Radio_Mute_state;
	Tu8													u8_ReqEnsembleIndex;						/*Get services for particular ensemble in Multiplex view*/
	Tu8													u8_Req_PlaySearchIndex;						/*Play selected station from stl search structure */
	Tbool												b_Scan_Cancel_Status;
	Tbool												b_ScanRequestStatus;
	Tbool												b_Search_Found;							/*Status variable to know stationlist search, found any station or not*/
	Tbool												b_Seek_Cancel_Status;
	Tu8													u8_ServiceIndex;							/*Index for Play select service from multiplex list*/
	Tu8													u8_SelectBandCheckReturnValue;				/*Return value of Select Band condition validation*/
	Tu8													u8_SettingsStateWithMarket;					/*Flag updated for Market supported settings use in Settings req before sending to below layers*/
	Tu8													u8_StartType;								/*Start type cold/warm start*/	
	Tbool												b_TimerFlag;								/*check whether timer is running or stopped*/
	Tu8													u8_TotalNoOfServiceInEnsemble;
	Tbool												b_TuneCancelReqStatus;
	Tbool												b_Tune_Cancel_Status;						/*Flag to know the Tune cancel*/
	Tu8													au8_DABChannelName[RADIO_MNGR_APP_MAX_CHANNEL_NAME_SIZE];  /*DAB Channel Name for tune by request*/
	Tu8													au8_MatchedPresetInSTL_Stn_Index[RADIO_MNGR_APP_MAX_PSML_SIZE];  /*Station list indices which is already added in favorite list to high late */
}Ts_Radio_Mngr_App_Inst_Hsm;


/*-----------------------------------------------------------------------------
    public Function Declaration
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/* 
 * Description			    This function should be called first in order to initialise the HSM radio_mngr_app_inst_hsm.
 *
 * param[in]  
 *	 pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm 
 *
 * Return_Val				None	
 *
 * pre[mandatory]			Radio_Mngr_App_Inst_Hsm_Init() function should be called 
 *
 * post [mandatory]			HSM is initialised and inactive state is reached 
 *
 */
/*===========================================================================*/
void Radio_Mngr_App_Inst_Hsm_Init(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

/*===========================================================================*/
/* 
 * Description				Message intended for inst HSM will be passed from main HSM to inst HSM via this function.
 *							This is message Handler function for inst HSM radio_mngr_app_inst_hsm.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm 
 *			     pst_msg	Pointer to the message to be handled 
 *
 * Return_Val				None
 *
 * pre[mandatory]			Inst HSM radio_mngr_app_inst_hsm is initialized 
 *
 * post [mandatory]			Message will be sent to inst HSM radio_mngr_app_inst_hsm and processed
 *
 */
/*===========================================================================*/
void Radio_Mngr_App_Inst_Hsm_HandleMsg(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);


/*-----------------------------------------------------------------------------
    private Function Declaration
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/* 
 * Description				It is the handler function for top state of the HSM radio_mngr_app_inst_hsm. In this handler,
 *							transition to the inactive state is only allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm 
 *	 pst_msg				Pointer to the message to be handled 
 *
 * Return_Val	 
 *	 Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_TopHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for inactive state of the HSM radio_mngr_app_inst_hsm. This is the child 
 *							state of top state.In this handler,upon receving RADIO_MNGR_APP_INST_START message from 
 *							HSM(main) amfm_app_hsm,transition to the active_start state is allowed.No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_InactiveHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active state of the HSM radio_mngr_app_inst_hsm.This is the child state 
 *							of top state.In this handler, Transition to the required states only allowed upon 
 *							receiving DAB StL done , Shutdown request and Tuner status notify messages.
 *
 * param[in]  
 *  pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				It is the handler function for active start state of the HSM radio_mngr_app_inst_hsm.This is the 
 *							child of active state.Once this state is reached, It will send the Inst start response to the Main HSM and state trasit to the 
 *							radio_mngr_app_inst_hsm_active_busy_stationlist_state. No other transitions are allowed. 
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveStartHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active_idle state of the HSM radio_mngr_app_inst_hsm. This is the 
 *							child of active state.In this handler,Transition to the radio_mngr_app_inst_hsm_active_busy_selectstation_mute_state  and 
 *							radio_mngr_app_inst_hsm_active_busy_selectband_state are only allowed upon receiving RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID 
 *							and RADIO_MNGR_APP_SELECTBAND_REQID message. No other transitions are allowed. 
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveIdleHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active_idle listen state of the HSM radio_mngr_app_inst_hsm. This is the 
 *							child of active idle state.In this handler we are handling the all notifications and corresponding result IDs.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveIdleListenHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active_busy state of the HSM radio_mngr_app_inst_hsm.
 *							This is the child of active state.In this handler,No transitions is allowed .  
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusyHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active stop state of the HSM radio_mngr_app_inst_hsm.This is the child 
 *							of active state. In shutdown case this state transits to inactive state. 
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveStopHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active stop state of the HSM radio_mngr_app_inst_hsm.This is the child 
 *							of active busy state. 
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg					Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectStationHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for SelectBand state of the HSM radio_mngr_app_inst_hsm. This is the 
 *							child of active state. In this handler, select the FM band in startup case and in User request select band, this handler will request to 
 *							respective application layer for band selection and transition to the radio_mngr_app_inst_hsm_active_busy_selectstation_mute_state.
 *							No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *	pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectBandHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for request to the Audio manager for Audio Mute. After completed the Mute state trasit to the 
 *							radio_mngr_app_inst_hsm_active_busy_play_selectstation_state state of the HSM radio_mngr_app_inst_hsm. 
 *							No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_MuteHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function as per the user requested station from the station list, will request to the respective 
 *							application layer and also in startup case default index 0 of FM station will be requested to the FM application layer.
 *							after successful tune state trasistion move to the radio_mngr_app_inst_hsm_active_busy_selectstation_demute_state state.
 *							No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_Play_SelectStationHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function request to the Audio manager to demute the audio after successful tune and send the response and station info to the HMI IF.
 *							Then after state transit to the radio_mngr_app_inst_hsm_active_idle_state.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_DemuteHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function request for seek up/down for AM, FM and DAB. And also notifies the seek frequency information to HMI IF.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_SeekUpDownHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function request for the Station list of AM, DAB and FM to respective application layers. Afte updation of station list,
 *							state transit to the radio_mngr_app_inst_hsm_active_busy_selectband_state state in startup condition.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_StationlistHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function request for the Preset Recall of Mixed Preset List to Tune the Preset Station. 
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_PresetRecallHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function request for the Tune up down.
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneUpDownHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function request for AF Tune request in strategy
 *
 * param[in]  
 *	pst_me_radio_mngr_inst	Pointer to the HSM object of type Ts_Radio_Mngr_App_Inst_Hsm
 *				 pst_msg	Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_AF_TuneHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/

Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneByFrequency_Hndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg);

#endif /* End of RADIO_MNGR_APP_INST_HSM_H */

/*=============================================================================
    end of file
=============================================================================*/