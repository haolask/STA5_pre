/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file dab_app_hsm.h 																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains Type Definitions and functions for handling DAB APP main HSM.	*
*																											*
*																											*
*************************************************************************************************************/

#ifndef __DAB_APP_HSM_H__
#define __DAB_APP_HSM_H__

/** \file */
/** \page DAB_APP_HSM_top DAB Application Main HSM package

\subpage DAB_APP_HSM_Overview
\n
\subpage DAB_APP_HSM_Functions
\n
*/

/**\page DAB_APP_HSM_Overview Overview   
    \n
     DAB Application Main HSM package consists of function handlers of Main HSM.   
    \n\n
*/

/** \page DAB_APP_HSM_Functions Main Function Handlers 
    <ul>
        <li> #DAB_APP_HSM_TopHndlr         : This is the handler function to the top state of dab_app_hsm. </li>
        <li> #DAB_APP_HSM_InactiveHndlr    : This is the handler function to the Inactive state of dab_app_hsm.</li>
		<li> #DAB_APP_HSM_ActiveHndlr      : This is the handler function to the Active state of dab_app_hsm. </li>
		<li> #DAB_APP_HSM_ActiveStartHndlr : This is the handler function to the Active Start State of dab_app_hsm. </li>
		<li> #DAB_APP_HSM_ActiveIdleHndlr  : This is the handler function to the Active Idle State of dab_app_hsm. </li>
		<li> #DAB_APP_HSM_ActiveStopHndlr  : This is the handler function to the Active Stop State of dab_app_hsm. </li>
    </ul>
*/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_inst_hsm.h"
#include "dab_app_msg_id.h"

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/


/**
 * @brief Structure definition of main hsm object, details Holds the current message, next message, current state handler name, cid and other varaiables for main HSM
 */
typedef struct 
{
	Ts_hsm					hsm;							/* the base HSM object, have to be the first member of this struct (handles state transitions) */ 
	Ts_dab_app_inst_hsm		instance;						/* instance sub hsm's*/	
	Tu8 					str_state[100];
	const Tu8				*ptr_curr_hdlr_name;			/* Pointer to store the current state handler name */

}Ts_dab_app_hsm;


/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				This function initializes the dab_app_hsm.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				none
*   \pre					DAB Application HSM is not initialized.
*   \details 				This function initializes the dab_app_hsm and transits to Inactive state. 
*   \post					DAB application main HSM is in Inactive state\n
*   \ErrorHandling    		none.
* 
******************************************************************************************************/
void DAB_APP_Init(Ts_dab_app_hsm *pst_me);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the top state of dab_app_hsm.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				pst_msg Pointer to the msg. This message can be modified.\n
*							A result code has to be set if message is handled in this state.
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the handler function for top state of the dab_app_hsm. In this handler,on receiving \n
*							START message transition takes place from top state to the inactive state .No other transitions are allowed.
*   \post					if a message is not handled in the top state a system error occurs.\n
*   \ErrorHandling    		A unhandled message in top state causes a system error.
* 
******************************************************************************************************/
Ts_Sys_Msg*  DAB_APP_HSM_TopHndlr(Ts_dab_app_hsm* pst_me, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Inactive state of dab_app_hsm.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				pst_msg Pointer to the msg. This message can be modified.\n
*							A result code has to be set if message is handled in this state.
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the handler function for inactive state of the dab_app_hsm. In this handler, \n
*							on receiving DAB_APP_STARTUP_REQID message transition takes place from inactive state\n
*							to the active start state.
*   \post					if a message is not handled in the top state a system error occurs.\n
*   \ErrorHandling    		A unhandled message in top state causes a system error.
* 
******************************************************************************************************/

Ts_Sys_Msg*  DAB_APP_HSM_InactiveHndlr(Ts_dab_app_hsm* pst_me, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active state of dab_app_hsm.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				pst_msg Pointer to the msg. This message can be modified.\n
*							A result code has to be set if message is handled in this state.
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active state handler of the dab_app_hsm \n
*							This is the child state of top state. Under this state only\n
*							all the operations related to dab application will be handled.
*   \post					if a message is not handled in the top state a system error occurs.\n
*   \ErrorHandling    		A unhandled message in top state causes a system error.
* 
******************************************************************************************************/

Ts_Sys_Msg*  DAB_APP_HSM_ActiveHndlr(Ts_dab_app_hsm* pst_me, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active state of dab_app_hsm.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				pst_msg Pointer to the msg. This message can be modified.\n
*							A result code has to be set if message is handled in this state.
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the handler function for active start state of the dab_app_hsm. In this handler,\n
*							on receiving DAB_APP_INST_HSM_START_DONE message transition takes place from active start\n
*							state to the active idle state.
*   \post					if a message is not handled handle the message in parent state.\n
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*  DAB_APP_HSM_ActiveStartHndlr(Ts_dab_app_hsm* pst_me, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Idle State of dab_app_hsm.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				pst_msg Pointer to the msg. This message can be modified.\n
*							A result code has to be set if message is handled in this state.
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the handler function for active idle state of the dab_app_hsm. All the messages first comes in this state\n
*							and then routed to instance hsm.
*   \post					if a message is not handled handle the message in parent state.\n
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*  DAB_APP_HSM_ActiveIdleHndlr(Ts_dab_app_hsm* pst_me, Ts_Sys_Msg* pst_msg);


/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Stop State of dab_app_hsm.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				pst_msg Pointer to the msg. This message can be modified.\n
*							A result code has to be set if message is handled in this state.
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the handler function for active stop state of the dab_app_hsm. In this handler,\n
*							on receiving DAB_APP_INST_HSM_SHUTDOWN_DONE message transition takes place from active\n
*							stop state to the inactive state.
*   \post					if a message is not handled handle the message in parent state.\n
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*  DAB_APP_HSM_ActiveStopHndlr(Ts_dab_app_hsm* pst_me, Ts_Sys_Msg* pst_msg);


#endif /* __DAB_APP_HSM_H__ */

/*=============================================================================
    end of file
=============================================================================*/