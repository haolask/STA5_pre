/*=============================================================================
    start of file
=============================================================================*/
/****************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Instance_hsm.h																		 *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														     *
*  All rights reserved. Reproduction in whole or part is prohibited											     *
*  without the written permission of the copyright owner.													     *
*																											     *
*  Project              : ST_Radio_Middleware																		             *
*  Organization			: Jasmin Infotech Pvt. Ltd.															     *
*  Module				: SC_AMFM_TUNER_CTRL																     *
*  Description			: Instance hsm structure,instance hsm hadler                                             *
                          declarations and soc tune reply function declations.                                   *																											*
*																											     *
******************************************************************************************************************/

#ifndef AMFM_TUNER_INST_H_
#define AMFM_TUNER_INST_H_


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "hsm_api.h"
#include "AMFM_Tuner_Ctrl_Types.h"
#include "etal_types.h"
#include "etal_api.h"
#include "etalversion.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
#define AMFM_TUNER_CTRL_REQID	   AMFM_TUNER_CTRL_ACTIVATE_REQID: \
						      case AMFM_TUNER_CTRL_TUNE_REQID: \
                              case AMFM_TUNER_CTRL_DEACTIVATE_REQID: \
							  case AMFM_TUNER_CTRL_GETSTATIONLIST_REQID: \
                              case AMFM_TUNER_CTRL_STOP:\
							  case AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_REQID:\
                              case AMFM_TUNER_CTRL_ERROR_REQID:\
							  case AMFM_TUNER_CTRL_SEEK_UP_DOWN_REQID:\
							  case AMFM_TUNER_CTRL_BGQUALITY_REQID:\
							  case AMFM_TUNER_CTRL_INST_START_REQID :\
							  case AMFM_TUNER_CTRL_CANCEL_REQID :\
							  case AMFM_TUNER_CTRL_ANNOUNCEMENT_CANCEL_REQID :\
							  case AMFM_TUNER_CTRL_AF_UPDATE_REQID :\
							  case AMFM_TUNER_CTRL_AF_CHECK_REQID :\
							  case AMFM_TUNER_CTRL_AF_JUMP_REQID  :\
							  case AMFM_TUNER_CTRL_LISTEN_RDS_START_REQID :\
							  case AMFM_TUNER_CTRL_SCAN_RDS_START_REQID :\
							  case AMFM_TUNER_CTRL_LOWSIGNAL_AF_CHECK_REQID:\
							  case AMFM_TUNER_CTRL_TUNE_FAIL:\
							  case AMFM_TUNER_CTRL_QUAL_READ:\
							  case AMFM_TUNER_CTRL_QUAL_READ_FAIL:\
							  case AMFM_TUNER_CTRL_SEEK_START_FAIL:\
							  case AM_FM_TUNER_CTRL_SOC_SEEK_TUNE_NEXT_FREQ_REQ:\
							  case AMFM_TUNER_CTRL_SEEK_CONTINUE_FAIL:\
							  case AMFM_TUNER_CTRL_SEEK_STOP:\
							  case AMFM_TUNER_CTRL_SEEK_STOP_FAILS:\
							  case AMFM_TUNER_CTRL_RDS_REQID:\
							  case STOP_RDS_MSG_ID
							  
							  

#define AMFM_TUNER_CTRL_RESID	  AMFM_TUNER_CTRL_SCAN_DONE_RESID: \
							 case AMFM_TUNER_CTRL_TUNE_DONE_RESID: \
							 case AMFM_TUNER_CTRL_FM_QUALITY_DONE_RESID: \
							 case AMFM_TUNER_CTRL_RDS_DONE_RESID: \
							 case AMFM_TUNER_CTRL_AMFMSCAN_QUALITY_READ_MSGID: \
							 case AMFM_TUNER_CTRL_SEEK_READ_RDS_RESID: \
							 case AMFM_TUNER_CTRL_STARTUP_TIMER: \
							 case AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID:\
							 case AM_FM_TUNER_CTRL_SOC_AMSCAN_CONTINUE_MSGID:\
							 case AMFM_TUNER_CTRL_FM_AF_UPDATE_QUALITY_DONE_RESID :\
							 case AMFM_TUNER_CTRL_FM_AF_CHECK_QUALITY_DONE_RESID :\
							 case AMFM_TUNER_CTRL_NOTIFICATION :\
							 case AMFM_TUNER_CTRL_AMSEEK_FAST_QUALITY_RESID :\
							 case AMFM_TUNER_CTRL_TUNER_STATUS_NOTIFICATION :\
							 case AMFM_TUNER_CTRL_AF_QUALITY_DONE_RESID:\
							 case AMFM_TUNER_CTRL_SEEK_DONE_RESID :\
							 case STOP_DONE_MSG_ID:\
							 case  QUALITY_NOTIFICATION_MSGID:\
							 case AMFM_TUNER_CTRL_FMAMTUNER_STATUS_NOTIFICATION  :\
							 case AMFM_TUNER_CTRL_FMCHECK_DONE_RESID:\
							 case AMFM_TUNER_CTRL_LOWSIGNAL_FMCHECK_DONE_RESID:\
							 case AMFM_TUNER_CTRL_AMFMTUNER_ABNORMAL_NOTIFICATION:\
							 case AMFM_TUNER_CTRL_BGTUNE_DONE_RESID:\
							 case AMFM_TUNER_CTRL_CONFIG_RECEIVER_DONE_RESID:\
							 case AMFM_TUNER_CTRL_CONFIG_DATAPATH_DONE_RESID:\
							 case AMFM_TUNER_CTRL_DESTROY_DATAPATH_DONE_RESID: \
							 case AMFM_TUNER_CTRL_AUDIO_SOURCE_SELECT_DONE_RESID:\
							 case AMFM_TUNER_CTRL_CHANGE_BAND_RECEIVER_DONE_RESID:\
							 case AMFM_TUNER_CTRL_STOP_RDS_DONE_RESID:\
							 case AMFM_TUNER_CTRL_SCAN_START_MSGID:\
							 case AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID:\
							 case AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID:\
							 case AMFM_TUNER_CTRL_SCAN_STOP_MSGID
						
						
			



/**
 * @brief Struct which contains quality data of a station
  * This struct contains all sensor values of a station including the calculated
 * quality.
 */

typedef struct 
{
    
    Tu32						  		u32_freq;							        /**< frequency of the tuner */
    Tu8							  		u8_status;							        /**< quality of the station, see #TUN_QUAL_ERROR, #TUN_QUAL_MIN and #TUN_QUAL_MAX for valid ranges! */
    Tu32							  	u32_Multipath;								        /**< multipath indication */
    Tu32							  	u32_FrequencyOffset;								        /**< frequency offset in kHz */
    Tu32						  		u32_AdjacentChannel;								        /**< neighbour channel disturbance indication */
    Tu32							  	u32_ModulationDetector;								        /**< modulation in case of FM */
} Ts_AMFM_Tuner_Ctrl_Qual;


/**
 * @brief Structure definition of instance hsm object
 * detailed Structure definition of instance hsm object
 */
typedef struct
{
	Ts_hsm								   hsm;								        /**< the base HSM object, have to be the first member of this struct (handles state transitions) */

	Ts_AMFM_Tuner_Ctrl_CurrStationInfo     st_Current_StationInfo;
	Ts_AMFM_Tuner_Ctrl_Getstationreq_info  st_Getstationreq;
	Ts_AMFM_Tuner_Ctrl_Tunereq_info        st_Tunereq;
	Ts_AMFM_Tuner_Ctrl_Interpolation_info  st_interpolation;
	Ts_AMFM_TunerCtrl_EON_Info			   st_EON_StationInfo;

	Te_RADIO_ReplyStatus			       e_SOC_ReplyStatus;
	Te_AMFM_Scan_Type 					   Scantype;
	Te_AMFM_Tuner_State					   e_Tuner_State;
	Te_RADIO_AMFMTuner_Status			   e_AMFMTunerErrorType;
	Te_AMFM_Tuner_Ctrl_Band                e_Band;								    /**< band of the tuner */
	Te_AMFM_Tuner_Ctrl_Market			   e_Market;

	Tu32								   u32_freq;							    /**< current active frequency */
	Tu32								   u32_Seek_Freq;							    /**< current active frequency */
	Tu32								   u32_Requested_freq;

	Tu16								   u16_pi;							        /**< expected pi which might be set by a tune command */

	Tu8 							 	   str_state[AMFM_TUNER_CTRL_MAXSTRSTATESIZE];
	Tu8*							   	   p_curr_state_str;					    /**< Pointer to the current HSM state string */
    Tu8									   u8_AMFMCmd_Recall_Count;
    Tu8								       u8_AMFMCmd_QualRecall_Count;
	
	Tbool								   b_Mutex_resource;
	Tbool								   b_CheckLimit;
	Tbool					   			   b_TATP_Flage;
	Tbool					   			   b_EON_Flage;
	Tbool								   b_PICheckFlage;							/**< Flage for Check Request given for Noraml case or LowSignal Case*/
	Tbool								   b_PI_Updated;							/**< Flage for PI_Upadted to AMFM_APP which will be useful for Annocement data to be sent after PI*/
	Tbool								   b_IsScan_FullcycleReached;



    ETAL_STATUS                            AMFM_Tuner_Ctrl_ETAL_Status;                    
}Ts_AMFM_Tuner_Ctrl_Inst_hsm;



/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/


/*===========================================================================*/
/* 
 * Description			           This function should be called first in order to initialise the HSM amfm_app_inst_hsm .
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl_inst   Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm 
 *
 * Return_Val				       None	
 *
 * pre[mandatory]			       AMFM_TUNER_CTRL_MAIN_HSM_Init function should be called 
 *
 * post [mandatory]			       Instance hsm is initialised and inactive state is reached 
 *
 */
/*===========================================================================*/
void AMFM_TUNER_CTRL_INST_HSM_Init(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me);

/*===========================================================================*/
/* 
 * Description				      It is the handler function for top state of the AMFM_Tuner_Ctrl_Inst_hsm. In this handler,
 *							      transition to the inactive state is only allowed. No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm 
 *			     pst_msg	    Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			    HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			    Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_TopHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst,Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				      It is the handler function for inactive state of the HSM AMFM_Tuner_Ctrl_Inst_hsm. This is the child 
 *							      state of top state.In this handler,upon receving AMFM_TUNER_CTRL_INST_START_REQID message from 
 *							      HSM(main) AMFM_Tuner_Ctrl_Main_hsm,transition to the active_start state is allowed.No other transitions are allowed.
 *
 * param[in]  
 *	pst_me_amfm_tuner_ctrl_inst   Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			     pst_msg	      Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			    HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			    Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_InactiveHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				    It is the handler function for active state of the AMFM_Tuner_Ctrl_Inst_hsm.This is the child state 
 *							    of top state.In this handler, Transition to the active stop state is only allowed upon 
 *							    receiving AMFM_TUNER_CTRL_STOP(instance hsm shutdown) message. 
 *
 * param[in]  
 *  pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			     pst_msg	    Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*		   Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							   message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			   HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			   Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				     It is the handler function for active_idle state of the AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							     child of active state.In this handler,Transition to the active_busy_direct tune state 
 *							     is allowed upon receiving AMFM_TUNER_CTRL_TUNE_REQID message,Transition to the active_busy_stlscan state 
 *							     is allowed upon receiving AMFM_TUNER_CTRL_GETSTATIONLIST_REQID message,Transition to the 
 *                               active_busy_sekkupdown state is allowed upon receiving AMFM_TUNER_CTRL_SEEK_UP_DOWN_REQID message.
 *							     No other transitions are allowed. 
 *
 * param[in]  
 *	pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			     pst_msg	    Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			    HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			    Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				      It is the handler function for active start state of the AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							      child of active state.Once this state is reached, AMFM_TUNER_CTRL_START_DONE_RESID message has to 
 *							      be send asynchronously to HSM(main) AMFM_Tuner_Ctrl_Main_hsm. Transition to the active_idle state is allowed 
 *							      upon receving AMFM_TUNER_CTRL_ACTIVATE_REQID message. No other transitions are allowed. 
 * 
 * param[in]  
 *	pst_me_amfm_tuner_ctrl_inst	 Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			     pst_msg	     Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			 Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							     message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			     HSM has to be initialized and msg should be valid ie. Need not be NULL.
 * 
 * post [mandatory]			     Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveStartHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				    It is the handler function for active_busy_direct_tune state of the AMFM_Tuner_Ctrl_Inst_hsm.
 *							    This is the child of active_busy state.In this handler,Transition to the active_idle state 
 *							     is allowed upon receving response AMFM_TUNER_CTRL_TUNE_DONE_RESID from Tuner_ctrl.No other transitions are allowed.  
 *
 * param[in]  
 *	pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			     pst_msg	    Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			    HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			    Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				      It is the handler function for active_sleep state of the HSM AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							      child of active state.In this handler,Transition to the active_idle state is only allowed 
 *							      upon receving AMFM_TUNER_CTRL_ACTIVATE_REQID message.No other transitions are allowed. 
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl_inst  Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			      pst_msg	      Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			   HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			   Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				   It is the handler function for active stop state of the AMFM_Tuner_Ctrl_Inst_hsm.This is the child 
 *							   of active state.Once AMFM_APP_INST_HSM_SHUTDOWN message is processed, AMFM_APP_INST_HSM_SHUTDOWN_DONE message 
 *							   has to be sent to HMS(main) AMFM_Tuner_Ctrl_Main_hsm asynchronously and then transits to inactive state. 
 *
 * param[in]  
 *	pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *				 pst_msg	    Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*	      Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							  message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			  HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			  Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveStopHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				    It is the handler function for active_busy state of the AMFM_Tuner_Ctrl_Inst_hsm.
 *							    This is the child of active state.In this handler,No transitions is allowed .  
 *
 * param[in]  
 *	pst_me_amfm_tuner_ctrl_inst  Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			     pst_msg	     Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			    HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			    Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				       It is the handler function for active error state of the AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							       child of active state.In this handler,Transition to the active stop.No other transitions are allowed. 
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			      pst_msg	        Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			   Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							       message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			      HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			      Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveErrorHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				       It is the handler function for active listen  of the AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							       child of active_Idle state.In this handler,Contiously moniter the Current Tuned frequency 
 *                                 Queality and RDS is only allowed upon receving AMFM_TUNER_CTRL_NOTIFICATION message.
 *   							   No other transitions are allowed. 
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			      pst_msg	        Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			   Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							       message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			      HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			      Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleListenHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				       It is the handler function for active_busy_SeekUpDown  of the AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							       child of active_busy state.In this handler,Transition to the active_Idle is only allowed 
 *							       upon receving AMFM_TUNER_CTRL_TUNE_DONE_RESID message.No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			      pst_msg	        Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			   Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							       message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			      HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			      Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusySeekUpDownHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				       It is the handler function for active busy stl_scan of the AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							       child of active_busy.In this handler,Transition to the active idle is only allowed 
 *							       upon receving AMFM_TUNER_CTRL_SCAN_DONE_RESID message.No other transitions are allowed. 
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			      pst_msg	        Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			   Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							       message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			      HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			      Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusySTLScanHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				       It is the handler function for active sleep background quality of the AMFM_Tuner_Ctrl_Inst_hsm.This is the 
 *							       child of active sleep state.No other transitions are allowed. 
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm
 *			      pst_msg	        Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*			   Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							       message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			      HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			      Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgQualityHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFUpdateTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFMCheckTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFmJumpTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveIdleAFUpdateTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFMCheckTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFmJumpTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgScanHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFStopHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyPISeekHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

#endif   /*AMFM_TUNER_CTRL_INST_H_*/


/*=============================================================================
                End of file
=============================================================================*/
