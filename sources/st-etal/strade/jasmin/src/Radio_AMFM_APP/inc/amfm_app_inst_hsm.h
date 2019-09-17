/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_inst_hsm.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declaration of all function handlers of HSM			*
*						  amfm_app_inst_hsm and INST HSM structure 											*
*																											*
*************************************************************************************************************/
#ifndef AMFM_APP_INST_HSM_H
#define AMFM_APP_INST_HSM_H

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/

#include "hsm_api.h"
#include "cfg_types.h"
#include "cfg_variant_market.h"
#include "amfm_app_request.h"
#include "amfm_app_response.h"
#include "amfm_app_notify.h"
#include "AMFM_Tuner_Ctrl_Types.h"
#include "AMFM_Tuner_Ctrl_Request.h"
#include "amfm_app_market.h"
#include "lib_bitmanip.h"

/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/
//#define FS_IN_DB(LEV_value) 	( (0.5f *(float)(LEV_value) ) - 8.0f )

/*-----------------------------------------------------------------------------
    Type Definitions
-----------------------------------------------------------------------------*/
/*	This structure is an HSM object contains information about instance HSM amfm_app_inst_hsm  */
	
typedef struct 																	
{																	
	Ts_hsm										st_inst_hsm;			/* Primary HSM object holds data about HSM amfm_app_inst_hsm used  for handling state transitions.This should be  have to be the first member of this structure  */	
	Tu8 										str_state[100];								
	const Tchar									*pu8_curr_state_name;		/* char Pointer points to string representing Current state name  */			
	Ts_AMFM_App_LSM_FM_Band						st_LSM_FM_Info;					/* Strucutre holds AF list of Last played FM station (LSM Memory) */	
	Ts_AMFM_App_AF_learn_mem    				ast_AF_Learn_mem[AMFM_APP_NO_STATIONS_FM_BAND];	/* Structure holds list of possible frequencies in the market and its corresponding PI value and it will be stored into NVM  */	
	
	Ts_AMFM_TunerCtrl_EON_Info 					st_AMFM_TunerCtrl_Eon_Info;												
	Ts_AFM_App_BG_AFtune_param					st_BG_AFtune_param;													
	Ts_AMFM_App_CT_Info							st_CT_Info;											
   	Ts_AMFM_App_StationInfo						st_current_station;		/* Structure to hold information about AM/FM current station */				
	Ts_AMFM_App_LinkingParam					st_DAB_FM_LinkingParam;    												
	Ts_AMFM_APP_DAB_FollowUp_StationInfo 		st_DAB_FollowUp_StationInfo;												
	Ts_AMFM_App_ENG_AFList_Info					st_ENG_AFList_Info;													
	Ts_AMFM_App_EON_List 						st_EON_List;												
	Ts_AMFM_App_StationInfo						st_EON_station_Info;		/* Structure to hold EON station information  */					
	Ts_AMFM_App_MarketInfo						st_MarketInfo;			/* */									
	Ts_AMFM_Tuner_Ctrl_Interpolation_info		st_QualParam;														
	Ts_AMFM_AppRDS_CT_Info						st_RDS_CT_Info;												
#ifdef 	AMFM_APP_ENABLE_STARTUP_SCAN																
	Ts_Sys_Msg									st_temp_msg_buffer;									
#endif																	
	Ts_Sys_Msg									st_tmp_msg_buffer;			
	Ts_AMFM_Tuner_Ctrl_CurrStationInfo  		st_TunerctrlCurrentStationInfo;												
	Ts_AMFM_App_TuneUpDown_parameters			st_TuneUpDownParam;   													
	
	Te_AMFM_AF_REGIONAL_Switch					e_AF_REGIONAL_Switch;		/* Enum  indicating AF Regional  switch ON/OFF  */					
	Te_AMFM_App_AF_Switch						e_AF_Switch;			/* Enum  indicating AF switch ON/OFF  */						
	Te_AMFM_App_Anno_Cancel_Request     		e_Anno_Cancel_Request;													
	Te_AMFM_App_mode							e_current_mode;			/* Enum holds the current tuned mode */							
	Te_RADIO_DABFM_LinkingStatus            	e_DAB2FM_Linking_status;												
	Te_AMFM_App_Eng_Mode_Switch					e_ENG_ModeSwitch;		/* Enum  indicating status of ENG mode switch ON/OFF */					
	Te_AMFM_App_FactoryResetReplyStatus 		e_FactoryResetReplyStatus;												
	Te_AMFM_App_FM_To_DAB_Switch				e_FM_DAB_Switch;		/* Enum  indicating FM-DAM FOLLOW UP switch ON/OFF  */					
	Te_RADIO_AMFMTuner_Status                 	e_AMFMTunerStatus;												
	Te_AMFM_App_Market							e_MarketType;			/* Enum holds present market type*/  							
	Te_AMFM_App_Processing_Status				e_Processing_Status;		/* Enum indicates whether AM/FM is in Foreground or Background */			
	Te_AMFM_App_mode							e_requested_mode;		/* Enum holds the requested mode */							
	Te_RADIO_Comp_Status 					    e_DABTunerStatus;													
	Te_AMFM_App_Scan_Type						e_Scan_Type;												
	Te_RADIO_DirectionType						e_SeekDirection;		/* Enum holds direction of seek operation */						
	Te_AMFM_App_SigQuality						e_SigQuality;											
	Te_AMFM_App_StationNotAvailStrategyStatus 	e_StaNotAvail_Strategy_status;	/* enum to hold AF Tune strategy start or end status */					
	Te_AMFM_App_mode							e_stationlist_mode;		/* Enum holds the mode used to indicate which stationlist need to be generated  */	
	Te_AMFM_App_TA_Switch						e_TA_Switch;			/* Enum  indicating TA switch ON/OFF  */						

	Tu32				 						u32_AF_StatusTimeout;							
	Tu32 										u32_Curr_stat_qual_check_Delay;							
	Tu32										u32_StartFrequency;		/* Frequency given by radio manager at start up */					

	Tu16 										u16_af_Tuned_PI;							
 	Tu16										u16_curr_station_pi;								
	Tu16										u16_EON_TA_Station_PI;								
	Tu32										u32_ReqFrequency;		/* Frequency to be sent to tuner ctrl */

	Tu32					 					u32_AF_Next_Freq_Update_Delay;
	Tu32 										u32_AF_qual_decr_alpha;
	Tu32 										u32_AF_qual_Existance_decr_alpha;
	Tu32		 								u32_AF_qual_Existance_incr_alpha;
	Tu32 										u32_AF_qual_incr_alpha;
	Tu32 										u32_AF_qual_Modified_decr_alpha;
	Tu32			 							u32_AF_qual_Modified_incr_alpha;
	Tu8											u8_aflistindex;			/*variable to hold the current Af list index */				
	Tu32										u32_AMFM_Normal_FS_Threshold;
	Tu32										u32_AMFM_quality;
	Tu32										u32_AMFM_Regional_Threshold;
	Tu32										u32_AMFM_Siglow_Threshold;
	Tu8											u8_charset;								
	Tu8					 						u8_Curr_qua_check_count;						
	Tu8										    u8_CurrStationPIupdateInSTL;							
	Tu8											u8_DABFM_LinkingCharset;							
	Tu32										u32_FrequencyOffset;
	Tu32										u32_quality;								
	Tu32 										u8_QualityDrop_Margin;							
	Tu32										u32_RDS_Senitivity_Threshold;
					
	Tu8											u8_SigResumeback_Threshold;
	Tu8											u8_start_up_type;							
	Tu8											u8_StrategyAFUpdate_Delay;						
	Tu8											u8_StationListIndex;		    /* Station list index value of station which is tuned */				
	Tu8 										u8_switch_setting; 								
	Tu32										u32_UltrasonicNoise;
	Tu32										u32_Multipath;
	
	Ts8											s8_CurrStation_IndexInSTL;	    /* Variable indicates the index value of current station in STL */			
	
	Tbool 										b_AFlist_copy_check;
	Tbool										b_CurrStationPSNupdateInSTL;	/*  Flag used to update PSN of current station only once in FM STL */			
	Tbool										b_CurStationPICpy;
	Tbool										b_MsgPendingFlag;							
	Tbool										b_NEG_TimerStartCheck;
	Tbool									    b_StartAF_Flag;		            /* Flag is used to indicate whether AF strategy can be started or not because after 				
																		          succesful FM seek, AF strategy to be started after 10 seconds only */
	//#ifdef AMFM_APP_ENABLE_BGSCANFLAG							
	Tbool										b_BackgroungScanFlag;							
   //#endif
   Tbool									    b_RegThresholdCheck;		

}Ts_AMFM_App_inst_hsm;

/*-----------------------------------------------------------------------------
    public Function Declaration
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/* 
 * Description			    This function should be called first in order to initialise the HSM amfm_app_inst_hsm .
 *
 * param[in]  
 *	 pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm 
 *
 * Return_Val				None	
 *
 * pre[mandatory]			AMFM_APP_HSM_Init() function should be called 
 *
 * post [mandatory]			HSM is initialised and inactive state is reached 
 *
 */
/*****************************************************************************************************/
void AMFM_APP_INST_HSM_Init(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst);


/*****************************************************************************************************/
/* 
 * Description				This function is called to send AMFM_APP_INST_HSM_STARTUP or AMFM_APP_INST_HSM_SHUTDOWN message 
 *							from main hsm to inst hsm during startup or shutdown process respectively.
 *
 * param[in]  
 *  pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm 
 *			     u16_msgid	Local ID (Message ID ) of the message.Message can be either AMFM_APP_INST_HSM_STARTUP or 
 *							AMFM_APP_INST_HSM_SHUTDOWN
 *
 * Return_Val				None
 *
 * pre[mandatory]			Both HSMs are initialized 
 *
 * post [mandatory]			inst hsm either Active start/inactive state is reached while sending AMFM_APP_INST_HSM_STARTUP/ 
 *							AMFM_APP_INST_HSM_SHUTDOWN message respectively.
 *
 */
/*****************************************************************************************************/
void AMFM_APP_INST_HSM_SendInternalMsg(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Tu16 u16_msgid);

/*****************************************************************************************************/
/* 
 * Description				Message intended for inst HSM will be passed from main HSM to inst HSM via this function.
 *							This is message Handler function for inst HSM amfm_app_inst_hsm.
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm 
 *			     pst_msg	Pointer to the message to be handled 
 *
 * Return_Val				None
 *
 * pre[mandatory]			Inst HSM amfm_app_inst_hsm is initialized 
 *
 * post [mandatory]			Message will be sent to inst HSM amfm_app_inst_hsm and processed
 *
 */
/*****************************************************************************************************/
void AMFM_APP_INST_HSM_MessageHandler(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_Sys_Msg* pst_msg);

/*-----------------------------------------------------------------------------
    private Function Declaration
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for top state of the HSM amfm_app_inst_hsm. In this handler,
 *							transition to the inactive state is only allowed. No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm 
 *			     pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_TopHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


/*****************************************************************************************************/
/* 
 * Description				It is the handler function for inactive state of the HSM amfm_app_inst_hsm. This is the child 
 *							state of top state.In this handler,upon receving AMFM_APP_INST_HSM_STARTUP message from 
 *							HSM(main) amfm_app_hsm,transition to the active_start state is allowed.No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			     pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_InactiveHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active state of the HSM amfm_app_inst_hsm.This is the child state 
 *							of top state.In this handler, Transition to the active stop state is only allowed upon 
 *							receiving AMFM_APP_SHUTDOWN_REQID message. 
 *
 * param[in]  
 *  pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			     pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active start state of the HSM amfm_app_inst_hsm.This is the 
 *							child of active state.Once this state is reached, AMFM_APP_INST_HSM_START_DONE message has to 
 *							be send asynchronously to HSM(main) amfm_app_hsm. Transition to the active_idle state is allowed 
 *							upon receving AMFM_APP_SELECT_BAND_REQID message. No other transitions are allowed. 
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			     pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveStartHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active_sleep state of the HSM amfm_app_inst_hsm.This is the 
 *							child of active state.In this handler,Transition to the active_idle state is only allowed 
 *							upon receving AMFM_APP_SELECT_BAND_REQID message.No other transitions are allowed. 
 *
 * param[in]  
 *	 pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			      pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);



/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active_sleep state of the HSM amfm_app_inst_hsm.This is the 
 *							child of active state.In this handler,Transition to the active_idle state is only allowed 
 *							upon receving AMFM_APP_SELECT_BAND_REQID message.No other transitions are allowed. 
 *
 * param[in]  
 *	 pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			      pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepBackgroundProcessSTLHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepAFTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active_idle state of the HSM amfm_app_inst_hsm.This is the 
 *							child of active state.In this handler,Transition to the active_busy_directtune state 
 *							is only allowed upon receiving AMFM_APP_SELECT_STATION_REQID message.
 *							No other transitions are allowed. 
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			     pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleListenHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleTuneFailedListenHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFStrategyTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFRegTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

#ifdef CALIBRATION_TOOL
	Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveCalibrationStartTuneState(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);
	
	Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveCalibrationStopTuneState(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);
#endif	


Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleEONStationAnnouncementHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleProgramStationAnnouncementHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active_busy state of the HSM amfm_app_inst_hsm.
 *							This is the child of active state.In this handler,No transitions is allowed .  
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			     pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active_busy_direct_tune state of the HSM amfm_app_inst_hsm.
 *							This is the child of active_busy state.In this handler,Transition to the active_idle state 
 *							is allowed upon receving response TUNER_CTRL_TUNE_RESID from Tuner_ctrl.No other transitions are allowed.  
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
 *			     pst_msg	Pointer to the message to be handled 
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyProcessSTLHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusySelectStationHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusySeekHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyTuneUpDownHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingGenerateHLFreqListHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingReadQualityHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingBgTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingFindBestPIHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringAFtuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active stop state of the HSM amfm_app_inst_hsm.This is the child 
 *							of active state.Once AMFM_APP_INST_HSM_SHUTDOWN message is processed, AMFM_APP_INST_HSM_SHUTDOWN_DONE message 
 *							has to be sent to HMS(main) amfm_app_hsm asynchronously and then transits to inactive state. 
 *
 * param[in]  
 *	pst_me_amfm_app_inst	Pointer to the HSM object of type Ts_AMFM_App_inst_hsm
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
/*****************************************************************************************************/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveStopHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg);


#endif		/* End of AMFM_APP_INST_HSM_H */