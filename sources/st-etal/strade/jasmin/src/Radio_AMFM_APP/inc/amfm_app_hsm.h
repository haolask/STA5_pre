/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_hsm.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declaration of all function handlers of HSM			*
*						  amfm_app_hsm(main) and HSM structure												*
*						  Structures belonging to this component											*
*																											*
*************************************************************************************************************/

#ifndef AMFM_APP_HSM_H
#define AMFM_APP_HSM_H


/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "amfm_app_inst_hsm.h"


/*-----------------------------------------------------------------------------
    Type Definitions 
-----------------------------------------------------------------------------*/

/*	This structure is an HSM object contains information about HSM amfm_app_hsm (main hsm) */

typedef struct 
{
	Ts_hsm					st_hsm;					/* Primary HSM object holds data about HSM amfm_app_hsm used  for handling state transitions.This should be  have to be the first member of this structure  */
	Ts_AMFM_App_inst_hsm	st_inst_hsm_info;		/* Holds data about instance HSM amfm_app_inst_hsm */
	Tu8 					str_state[100];
	const char*				pu8_curr_state_name;	/* char Pointer points to string representing Current state name  */
}Ts_AMFM_App_hsm;


/*-----------------------------------------------------------------------------
    public Function Declaration
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/* 
 * Description			    This function should be called first in order to initialise the HSM amfm_app_hsm .
 *
 * param[in]  
 *		pst_me_amfm_app		Pointer to the HSM object of type Ts_AMFM_App_hsm 
 *
 * Return_Val				None	
 *
 * pre[mandatory]			N/A
 *
 * post [mandatory]			HSM is initialised and inactive state is reached 
 *
 */
/*****************************************************************************************************/
void AMFM_APP_HSM_Init(Ts_AMFM_App_hsm * pst_me_amfm_app);



/*-----------------------------------------------------------------------------
    private Function Declaration
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for top state of the HSM amfm_app_hsm. In this handler,
 *							Transition to the inactive state is only allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_app		Pointer to the HSM object of type Ts_AMFM_App_hsm 
 *			 pst_msg		Pointer to the message to be handled 
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
Ts_Sys_Msg*  AMFM_APP_HSM_TopHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for inactive state of the HSM amfm_app_hsm. In this handler, 
 *							Transition to the active start state is only allowed upon receiving AMFM_APP_STARTUP_REQID message. 
 *							No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_app		Pointer to the HSM object of type Ts_AMFM_App_hsm 
 *			 pst_msg		Pointer to the message to be handled 
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
Ts_Sys_Msg*  AMFM_APP_HSM_InactiveHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active state of the HSM amfm_app_hsm. In this handler,
 *							Transition to the active stop state is only allowed upon receiving AMFM_APP_SHUTDOWN_REQID message. 
 *							No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_app		Pointer to the HSM object of type Ts_AMFM_App_hsm 
 *			 pst_msg		Pointer to the message to be handled 
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
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active start state of the HSM amfm_app_hsm. In this handler,
 *							during startup process ,instance HSM amfm_app_inst_hsm is set to active start state by sending 
 *							request AMFM_APP_INST_HSM_STARTUP internally .Once AMFM_APP_INST_HSM_START_DONE message is received 
 *							from inst HSM , transtion to active idle state is allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_app		Pointer to the HSM object of type Ts_AMFM_App_hsm 
 *			 pst_msg		Pointer to the message to be handled 
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
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveStartHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				It is the handler function for active idle state of the HSM amfm_app_hsm. In this handler, 
 *							messages intented to instance HSM are routed to its corresponding message handler function. 
 *
 * param[in]  
 *	 pst_me_amfm_app		Pointer to the HSM object of type Ts_AMFM_App_hsm 
 *			 pst_msg		Pointer to the message to be handled 
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
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveIdleHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/* 
 * Description				 It is the handler function for active stop state of the HSM amfm_app_hsm. In this handler,
 *							 during shutdown process,instance hsm is set to inactive state by sending request 
 *							 AMFM_APP_INST_HSM_SHUTDOWN internally.Once AMFM_APP_INST_HSM_SHUTDOWN_DONE message is received 
 *							 from inst HSM , transtion to inactive state is allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_amfm_app		Pointer to the HSM object of type Ts_AMFM_App_hsm 
 *			 pst_msg		Pointer to the message to be handled 
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
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveStopHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg);



#endif /* End of AMFM_APP_HSM_H */