/*==================================================================================================
    start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_Tuner_Ctrl_main_hsm.h																   *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.											   *
*  All rights reserved. Reproduction in whole or part is prohibited								   *
*  without the written permission of the copyright owner.										   *
*																								   *
*  Project              : ST_Radio_Middleware													   *
*  Organization			: Jasmin Infotech Pvt. Ltd.												   *
*  Module				: Radio DAB Tuner Control 												   *
*  Description			: This file contains API declarations related to main HSM.  			   *
*																								   *
***************************************************************************************************/

#ifndef DAB_TUNER_CTRL_MAIN_HSM_
#define DAB_TUNER_CTRL_MAIN_HSM_
/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "hsm_api.h"
#include "DAB_Tuner_Ctrl_inst_hsm.h"
#include "cfg_types.h"

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/
#define DAB_TUNER_CTRL_NO_ANNO_SET		    				0x0000 /* Macro for checking announcement stop condition  */
#define DAB_TUNER_CTRL_ANNO_SETCONFIG    					0xffff /* Macro for checking alarm announcement set condition  */
#define DAB_UP_NOTIFY_TIMEOUT_TIME							((Tu16)2000) /* Timeout time for DestroyReciver_cmd is 2000ms */

/*--------------------------------------------------------------------------------------------------
    type definitions
--------------------------------------------------------------------------------------------------*/

/**
* @brief Main HSM structure definition.
*/
typedef struct
{
  Ts_hsm                         st_hsm;                  /* the base HSM object, has to be the first member of 
                                                             this structure - handles state transitions */
  Tu8 							 		str_state[100];
  const Tu8*                     		u8_curr_state_str;       /* pointer to the name of the current state handler */
  Ts_dab_tuner_ctrl_inst_hsm     		st_dab_tuner_receiver;  /* DABTuner receiver structures */
  Ts_DabTunerMsg_N_UpNot		 		st_upnot;                /* Up Notification structure */
  Ts_SystemError_not					st_SystemError_not ;
  Te_RADIO_ReplyStatus					e_ReplyStatus;
  Te_DAB_Tuner_Market			 		e_market;		
  Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus		e_DAB_Tuner_Ctrl_ActivateDeActivateStatus;	
} Ts_Tuner_Ctrl_main;


/**
* struct for TimerId
*/

typedef struct
{
	
	Tu32 u32_Up_ActiveStart_Timer;
	Tu32 u32_Up_ActiveIdle_Timer;	

} Ts_Tuner_Ctrl_main_timer;
/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control main HSM top handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is initialised.
*   \details                  When DAB Tuner Control is initialised, it enters the top state that is 
                              top handler function and processes the valid messages received in the handler. 
*   \post-condition			  DAB Tuner Control is in top state.
*   \ErrorHandling    		  When the message received by the top handler cannot be processed, it 
                              throws an error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_TopHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control main HSM inactive handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in top state handler.
*   \details                  When DAB Tuner Control is in top state, the top handler transits to 
                              inactive handler via HSM_MSGID_START message and processes the valid 
							  messages received in the handler. 
*   \post-condition			  DAB Tuner Control is in inactive state.
*   \ErrorHandling    		  When the message received by the inactive handler cannot be processed, 
                              the parent handles the message, when the message is not handled by the 
                              parent state then the HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_InactiveHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control main HSM active handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in inactive state handler.
*   \details                  When DAB Tuner Control is in inactive state, inactive handler transits 
                              to active handler via start up message received from upper layer. 
*   \post-condition			  DAB Tuner Control is in active state.
*   \ErrorHandling    		  When the message received by the active handler cannot be processed, 
                              the parent handles the message, when the message is not handled by the 
                              parent state then the HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control main HSM active start handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in active state handler.
*   \details                  When DAB Tuner Control is in active state, active handler transits 
                              to active start handler via start up message received from upper layer. 
*   \post-condition			  DAB Tuner Control is in active start state.
*   \ErrorHandling    		  When the message received by the active start state handler cannot be 
                              processed, the parent handles the message, when the message is not 
							  handled by the parent state then the HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control main HSM active idle handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in active start state handler.
*   \details                  When DAB Tuner Control is in active start state, active start handler 
                              transits to idle state handler via internal message received from 
							  instance HSM. 
*   \post-condition			  DAB Tuner Control is in idle state.
*   \ErrorHandling    		  When the message received by the idle state handler cannot be 
                              processed, the parent handles the message, when the message is not 
							  handled by the parent state then the HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);


/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control main HSM active stop handler
*   \param[in]				  pst_msg
*   \param[out]				  DAB_Tuner_Ctrl_me
*   \pre-condition			  DAB Tuner Control HSM is in active idle handler or active state handler.
*   \details                  When DAB Tuner Control is in active idle handler or active state handler, 
                              the transition to active stop state occurs via shut down message received 
							  from upper layer. 
*   \post-condition			  DAB Tuner Control is in active stop state.
*   \ErrorHandling    		  When the message received by the active stop handler cannot be 
                              processed, the parent handles the message, when the message is not 
							  handled by the parent state then the HSM	throws error. 
* 
***************************************************************************************************/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control HSM initialisation.
*   \param[in]				  pMain
*   \param[out]				  None
*   \pre-condition			  DAB Tuner Control component is initialised.
*   \details                  Whenever DAB Tuner Control component is initialised,the HSM is 
                              initialised to default values.
*   \post-condition			  DAB Tuner Control is initialised.
*   \ErrorHandling    		  None
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_HSM_Init(Ts_Tuner_Ctrl_main* pMain);

 #endif
/*=============================================================================
    end of file
=============================================================================*/
