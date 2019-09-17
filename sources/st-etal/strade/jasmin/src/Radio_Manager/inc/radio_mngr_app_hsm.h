/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_hsm.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager	Application														    *
*  Description			: This header file consists of declaration of all function handlers of HSM			*
*						  radio_mngr_app_hsm(main) and HSM structure										*
*						  Structures belonging to this component											*
*																											*
*************************************************************************************************************/
#ifndef RADIO_MNGR_APP_HSM_H
#define RADIO_MNGR_APP_HSM_H

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include <string.h>
#include "radio_mngr_app_types.h"
#include "radio_mngr_app_inst_hsm.h"
#include "radio_mngr_app.h"
#include "hmi_if_app_notify.h"
#include "lib_string.h"
#include "sys_timer.h"
#include "lib_bitmanip.h"


/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/

#define RADIO_MNGR_APP_UINT8_ZERO                               ((Tu8)(0u))

#define RADIO_MNGR_APP_UINT8_ONE                                ((Tu8)(1u))

#define AUDIO_MNGR_DEMUTE                                       ((Tu8)(1u))

#define AUDIO_MNGR_MUTE                                         ((Tu8)(0u))

#define RADIO_MNGR_APP_STARTUP_REQID							((Tu16)(0X3000u))

#define RADIO_MNGR_APP_SHUTDOWN_REQID							((Tu16)(0X3001u))

#define RADIO_MNGR_INST_HSM_STOP_DONE							((Tu16)(0X3500u))

#define RADIO_MNGR_INST_HSM_START_DONE							((Tu16)(0X3501u))


/*-----------------------------------------------------------------------------
    Type definitions
-----------------------------------------------------------------------------*/

/*@brief Structure definition of hsm object*/
typedef struct
{
    Ts_hsm											st_radio_mngr_hsm;				/*The base HSM object, have to be the first member of this struct (handels state transitions) */
    Tu8 									 		str_state[100];
    const Tu8*										u8p_curr_state_str;				/*pointer to the name of the current state handler */
	Tu8												u8_padding[3];					/*padding bytes */
	Ts_Radio_Mngr_App_DABVersion_Reply				st_DAB_Version_Info;			/*DABTuner Hardware,software version details of DAB*/
    Ts_Radio_Mngr_App_Inst_Hsm						st_inst_hsm ;  					/*Instance HSM*/
	Te_RADIO_ReplyStatus							e_AMFM_FactoryReset_ReplyStatus;
	Te_RADIO_ReplyStatus							e_AMFMShutdownReplyStatus;		/*AMFM Shutdown reply status*/
	Te_RADIO_ReplyStatus							e_AMFMStartReplyStatus;			/*AMFM startup reply status*/
	Te_RADIO_ReplyStatus							e_DAB_FactoryReset_ReplyStatus;	
	Te_RADIO_ReplyStatus							e_DABShutdownReplyStatus;		/*DAB Shutdown reply status*/
	Te_RADIO_ReplyStatus							e_DABStartReplyStatus;			/*DAB startup reply status*/
	Te_RADIO_ReplyStatus							e_InstHSMReplyStatus;			/*Reply status from the instance HSM for startup/shutdown */
	Te_Radio_Mngr_App_Market						e_Market;						/*Market Type*/
	Tu8												u8_RadioComponentInfo;			/*Radio Component Info which band is ON/OFF*/
} Ts_Radio_Mngr_App_Hsm;


/*-----------------------------------------------------------------------------
    public Function Declaration
-----------------------------------------------------------------------------*/
/*===========================================================================*/
/* 
 * Description			    This function should be called first in order to initialise the HSM radio_mngr_app_hsm.
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
 *
 * Return_Val				None	
 *
 * pre[mandatory]			N/A
 *
 * post [mandatory]			HSM is initialised and inactive state is reached 
 *
 */
/*===========================================================================*/
void Radio_Mngr_App_Hsm_Init(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr);

/*===========================================================================*/
/* 
 * Description				It is the handler function for top state of the HSM radio_mngr_app_hsm. In this handler,
 *							Transition to the inactive state is only allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
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
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_TopHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for inactive state of the HSM radio_mngr_app_hsm. In this handler, 
 *							Transition to the active start state is only allowed upon receiving RADIO_MNGR_APP_STARTUP_REQID message. 
 *							No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
 *	 pst_msg				Pointer to the message to be handled 
 *
 * Return_Val	 
 *	 pst_msg*				Returns NULL if handler can handle the message.Otherwise returns pointer to the 
 *							message(pst_msg) in this case message will be given to parent state. 
 *
 * pre[mandatory]			HSM has to be initialized and msg should be valid ie. Need not be NULL.
 *
 * post [mandatory]			Either message is handled in the current state or given to the parent state.
 *
 */
/*===========================================================================*/
Ts_Sys_Msg*   Radio_Mngr_App_Hsm_InactiveHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active state of the HSM radio_mngr_app_hsm. In this handler,
 *							Transition to the active stop state is only allowed upon receiving RADIO_MNGR_APP_SHUTDOWN_REQID message. 
 *							No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
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
Ts_Sys_Msg*   Radio_Mngr_App_Hsm_ActiveHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active start state of the HSM radio_mngr_app_hsm. In this handler,
 *							during startup process start the AMFM and DAB applications then after instance HSM radio_mngr_app_inst_hsm is set to active start state.
 *							Once AMFM_APP_INST_HSM_START_DONE message is received 
 *							from inst HSM , transtion to active idle state is allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
 *			 pst_msg		Pointer to the message to be handled 
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
Ts_Sys_Msg*   Radio_Mngr_App_Hsm_ActiveStartHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active idle state of the HSM radio_mngr_app_hsm. In this handler, 
 *							messages intented to instance HSM are routed to its corresponding message handler function. 
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
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
Ts_Sys_Msg*   Radio_Mngr_App_Hsm_ActiveIdleHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description				It is the handler function for active stop state of the HSM radio_mngr_app_hsm. In this handler,
 *							during shutdown process, Sending shutdown request to AMFM and DAB applications then after instance hsm is set to inactive state by sending stop request 
 *						    internally.Once RADIO_MNGR_INST_HSM_STOP_DONE message is received 
 *							from inst HSM , transtion to inactive state is allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
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
Ts_Sys_Msg*   Radio_Mngr_App_Hsm_ActiveStopHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/

/* 
 * Description				 It is the handler function for active stop state of the HSM radio_mngr_app_hsm. In this handler,
 *							 during error process, transtion to stop state is allowed. No other transitions are allowed.
 *
 * param[in]  
 *	 pst_me_radio_mngr		Pointer to the HSM object of type Ts_Radio_Mngr_App_Hsm 
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
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveErrorHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/

void StartupCheck(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr);

void Update_Default_Stations_Info(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr);

void Update_Tunable_Station_Info(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr);

void Update_HMI_IF_Common_Data(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr);

void Update_Settings(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Te_Radio_Mngr_App_Market e_Market);

void Radio_Manager_App_Update_AppLayer_STL(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr);

void Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst, Te_Radio_Mngr_App_StationNotAvail_StrategyStatus e_StrategyStatus);

void Radio_Mngr_App_Stop_StationNotAvail_Strategy(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst);

void DAB_Request_Internal_Cancel_DABTuner_Abnormal(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Cancel_Req_Type e_Cancel_Type);

Te_RADIO_ReplyStatus Radio_Mngr_App_StartFactoryReset(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr);


#endif /* End of RADIO_MNGR_APP_HSM_H */

/*=============================================================================
    end of file
=============================================================================*/