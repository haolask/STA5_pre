/*=============================================================================
    start of file
=============================================================================*/
/****************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Main_hsm.h																		     *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														     *
*  All rights reserved. Reproduction in whole or part is prohibited											     *
*  without the written permission of the copyright owner.													     *
*																											     *
*  Project              : ST_Radio_Middleware																		             *
*  Organization			: Jasmin Infotech Pvt. Ltd.															     *
*  Module				: SC_AMFM_TUNER_CTRL																     *
*  Description			: Main hsm  structure,main hsm hadler functions declarations                             *
                                                                                                                 *																											*
*																											     *
******************************************************************************************************************/


#ifndef AMFM_TUNER_CTRL_MAIN_H_
#define AMFM_TUNER_CTRL_MAIN_H_


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "AMFM_Tuner_Ctrl_App.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/


typedef enum 
{
    C_FALSE,
	C_TRUE,
} Main_Bool;


typedef struct
{
    Ts_hsm							 st_hsm;					    /**< the base HSM object (handles state transitions) */
    Ts_AMFM_Tuner_Ctrl_Inst_hsm      instances;				        /**< commment update the value*/
	Ts_Sys_Msg*						 curr_cmd;				        /**< current running command */
    Ts_Sys_Msg*						 stop_cmd;				        /**< pended stop command */
	const Ts_hsm_state				 *common_next;			        /**< next state for a common state */
	Tu8 							 str_state[AMFM_TUNER_CTRL_MAXSTRSTATESIZE];
	Tu8*						     p_curr_state_str;              /**< current state */
	Tu32							 u32_count;				        /**< general purpose counter, can be used in any atomic state */
	Tu16                             u16_cid;				        /**< package ID used for this instance of the HSM */
	Tu32							 u32_freq;				        /**< current active frequency */
    Tu8                  			 u8_timer;				        /**< general timer can be used in any atomic state */
}Ts_AMFM_Tuner_Ctrl_Main_hsm;


/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/


/*===========================================================================*/
/* 
 * Description				   It is the handler function for top state of the AMFM_Tuner_Ctrl_Main_hsm. In this handler,
 *							   Transition to the inactive state is only allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl		Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Main_hsm 
 *			 pst_msg		    Pointer to the message to be handled 
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
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_TopHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				    It is the handler function for inactive state of the AMFM_Tuner_Ctrl_Main_hsm. In this handler, 
 *							    Transition to the active start state is only allowed upon receiving AMFM_TUNER_CTRL_STARTUP_REQID message. 
 *							    No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl		Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Main_hsm 
 *	 pst_msg		            Pointer to the message to be handled 
 *
 * Return_Val	 
 *   Ts_Sys_Msg*			    Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *  
 * pre[mandatory]			    HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			    Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_InactiveHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg);
/*===========================================================================*/
/* 
 * Description				    It is the handler function for active state of the AMFM_Tuner_Ctrl_Main_hsm. In this handler,
 *							    Transition to the active stop state is only allowed upon receiving AMFM_TUNER_CTRL_SHUTDOWN_REQID message. 
 *							    No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl		Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Main_hsm 
 *	 pst_msg	                Pointer to the message to be handled 
 *
 * Return_Val	 
 *	 Ts_Sys_Msg*			    Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							    message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			    HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			    Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active start state of the AMFM_Tuner_Ctrl_Main_hsm. In this handler,
 *							during startup process ,instance HSM Ts_AMFM_Tuner_Ctrl_Inst_hsm is set to active start state by sending 
 *							request AMFM_APP_INST_HSM_STARTUP internally .Once AMFM_TUNER_CTRL_INST_HSM_START_DONE message is received 
 *							from inst HSM , transtion to active idle state is allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl		Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Main_hsm 
 *	 pst_msg		        Pointer to the message to be handled 
 *
 * Return_Val	 
 *   Ts_Sys_Msg*			Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveStartHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active idle state of the AMFM_Tuner_Ctrl_Main_hsm. In this handler, 
 *							messages intented to instance HSM are routed to its corresponding message handler function. 
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Main_hsm 
 *	 pst_msg		        Pointer to the message to be handled 
 *
 * Return_Val	 
 *	Ts_Sys_Msg*		        Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveIdleHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg);


/*===========================================================================*/
/* 
 * Description				 It is the handler function for active stop state of the HSM amfm_app_hsm. In this handler,
 *							 during shutdown process,instance hsm is set to inactive state by sending request 
 *							 AMFM_TUNER_CTRL_STOP internally.Once AMFM_TUNER_CTRL_INST_HSM_STOP_DONE message is received 
 *							 from inst HSM , transtion to inactive state is allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_tuner_ctrl	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Main_hsm 
 *			 pst_msg		Pointer to the message to be handled 
 *
 * Return_Val	 
 *			Ts_Sys_Msg*		Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveStopHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				This function is called to send AMFM_TUNER_CTRL_INST_START_REQID message 
 *							from main hsm to inst hsm during startup  process.
 *
 * param[in]  
 *  pst_me_amfm_tuner_ctrl_inst	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm 
 *			     u16_msgid	   Local ID (Message ID ) of the message.Message can be either AMFM_APP_INST_HSM_STARTUP  
 *							    
 *
 * Return_Val				None
 *
 * pre[mandatory]			Both HSMs are initialized 
 *
 * post [mandatory]			inst hsm either Active start is reached while sending AMFM_TUNER_CTRL_INST_START_REQID message .
 *
 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_hsm_inst_start(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl, Tu16 u16_msgid);

/*===========================================================================*/
/* 
 * Description			        This function should be called first in order to initialise the HSM amfm_app_hsm .
 *
 * param[in]  
 *		pst_me_amfm_tuner_ctrl	Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Main_hsm 
 *
 * Return_Val				    None	
 *
 * pre[mandatory]			    N/A
 *
 * post [mandatory]			    Main hsm is initialised and inactive state is reached 
 *
 */
/*===========================================================================*/
void   AMFM_TUNER_CTRL_MAIN_HSM_Init(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me);


#endif /*AMFM_TUNER_CTRL_MAIN_H_*/



/*=============================================================================
                End of file
=============================================================================*/

