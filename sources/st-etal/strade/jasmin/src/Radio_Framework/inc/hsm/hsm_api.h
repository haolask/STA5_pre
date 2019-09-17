/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file hsm_api.h																							*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Hierarchical State Machine																	     		*
*  Description			: This file contains Basic HSM API's declarations																					*	
*																											*
*																											*
*************************************************************************************************************/


#ifndef HSM_API_H
#define HSM_API_H


/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "sys_task.h"


/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/

#define PRIVATE static
#define PUBLIC


/* Maximum number of parameters in one slot message */
#define MSG_MAX_PARAM_SLOT					((Tu8)8)

/* Maximum number of allowable nesting of states in an HSM*/
#define HSM_MAX_STATE_NESTING				((Tu8)10)



/************* Message IDs for HSM default messages	*****************/


/*Message ID for ENTRY message */
#define HSM_MSGID_ENTRY						((Tu16)1)

/*Message ID for START message */
#define HSM_MSGID_START						((Tu16)2)

/*Message ID for EXIT message */
#define HSM_MSGID_EXIT						((Tu16)3)

/* Logical component ID */
#define HSM_CID								((Tu16)0xff)


/*-----------------------------------------------------------------------------
    Type Definitions
-----------------------------------------------------------------------------*/

/* Type definition of the basic struct for a HSM
 *
 * To create a HSM, define a variable of this type in your data structure as first 
 * item. This struct includes controlling values( pointer to current,next and top states ,PID of HSM)
 * needed for HSM handling.
 */
typedef struct Hsm Ts_hsm; 

 
/* Type definition for a HSM-state
 *
 * This includes pointer to parent state and pointer to function handler of that state. 
 * Use the macro HSM_CREATE_STATE() for defining a state in HSM. 
 */
typedef struct Hsm_state Ts_hsm_state;


/* Type definition for function pointer to state handler function
 *
 * Use this prototype to define a pointer to any state handler function in HSM 
 */
typedef Ts_Sys_Msg* (*Tfp_hsm_evt_hndlr)(Ts_hsm* pst_me, Ts_Sys_Msg* pst_msg);



/* Structure for HSM-state 
 *
 * This includes pointer to parent state and pointer to function handler of that state.
 * Use the macro HSM_CREATE_STATE() for defining a state in HSM.
 */
struct 	Hsm_state			
{
    const Ts_hsm_state      *pst_parent;		/* pointer to parent state, or NULL if this is the Top state */
    Tfp_hsm_evt_hndlr       fp_hndlr;			/* HSM state handling function */
    const Tu8				*pu8_state_name;	/* char pointer to string represents state name */
};



/*
 * Structure holds data for HSM handling .
 *
 * This structure includes controlling values( pointer to current,next and top states ,PID of HSM)
 * needed for HSM handling.
 */
struct Hsm
{
    const Ts_hsm_state  *pst_curr;    /* pointer to the current active state */
    const Ts_hsm_state  *pst_next;    /* pointer to next state, if transition was taken, otherwise NULL */
    const Ts_hsm_state  *pst_top;     /* pointer to top state */
    Tu16           	 u16_cid;     /* CID of the HSM owner */
};



/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/

/*
 * Performs a state transition
 *
 * Use this macro for transiting from current state to target state in a HSM  
 * This macro will invoke HSM_StateTransition() function. 
 *	
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling 
 *			 pst_target		Pointer to the target state 	
 * 
 */
#define HSM_STATE_TRANSITION(pst_me,pst_target)   (HSM_StateTransition((Ts_hsm*)(pst_me), (pst_target)))


/*
 * Starts the HSM
 *
 * This macro will allow the current state handler to handle the START message and will perform  
 * the possible state transitions until they are completed. 
 * This macro will invoke HSM_OnStart() function.
 *
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling  			
 *	
 */
#define HSM_ON_START(pst_me)             (HSM_OnStart((Ts_hsm*)(pst_me)))


/*
 * Handles a message in the given HSM
 *
 * Use this macro when there is any message to be handled. 
 * This macro will invoke HSM_OnMsg() function.
 *
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling  			
 *			 pst_msg		Pointer to the message to be handled
 */
#define HSM_ON_MSG(pst_me,pst_msg)        (HSM_OnMsg((Ts_hsm*)(pst_me),(pst_msg)))


/*
 * Defines a state in an HSM 
 *
 * Use this macro when new state has to be created in an HSM 
 * This macro creates a state of type Ts_hsm_state.
 */
#define HSM_CREATE_STATE(state_name,pst_parent_state,function_handler,Desp)  \
	  static const Ts_hsm_state state_name = { (const Ts_hsm_state  *)(pst_parent_state),(Tfp_hsm_evt_hndlr)(function_handler),(const Tu8*)(Desp) } 





/*
 * Creates HSM base structure
 *
 * Use this macro to construct an HSM
 *
 * param[in]	
 *			 pst_me			Pointer to the HSM structure of type Ts_hsm which includes control values needed for HSM handling  			
 *			 pst_top		Pointer to the top state 
 *			 u16_pid		Pid of the HSM 
 */
#define HSM_CTOR(pst_me,pst_top,u16_cid) (HSM_HsmCtor( (pst_me),(pst_top),(u16_cid) ))


/*-----------------------------------------------------------------------------
    Public Function Declaration
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/* 
 * Description :  This function is called to perform a state transition from the 
 * actual state to the target state. The state transition is not performed immediatley 
 * after the call, but after handling the actual message.
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 pst_target		Pointer to the target state 
 *
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_StateTransition(Ts_hsm* pst_me, const Ts_hsm_state* pst_target) ;



/*===========================================================================*/
/* 
 * Description:  This function will start the handling of an message by a HSM by 
 * calling State handler of the current state with the given message. Depending on 
 * the return value of the handler, the HSM performs one of the following 
 * a) Performs a state transition if next state is updated in state handler   
 * b) Give the message to superior state if return value of the handling function 
 *	  is not NULL
 * c) Do nothing more
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 pst_msg		Pointer to the message to be handled  
 *
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_OnMsg(Ts_hsm *pst_me, Ts_Sys_Msg *pst_msg) ;


/*===========================================================================*/
/* 
 * Description:  This function will start a constructed HSM. This means it will
 * let the current state handler handle the start event and will react on the 
 * possible following state transitions, until they are finished.
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_OnStart(Ts_hsm *pst_me);


/*===========================================================================*/
/* 
 * Description:  This function creates the HSM based on the values given as 
 * parameters. All data within the HSM are initialized. 
 *
 * param[in]  
 *			 pst_me			Pointer to the HSM which shall perform the transition
 *			 pst_top        Pointer to the  top state  
 *			 u16_pid		Pid of the HSM 
 *
 * Return_Val	  : None 		
 */
/*===========================================================================*/
void HSM_HsmCtor(Ts_hsm* pst_me, const Ts_hsm_state* pst_top,const Tu16 u16_cid);


#endif /* End of HSM_API_H */

/*=============================================================================
    end of file
=============================================================================*/