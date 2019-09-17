/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file hsm_implementation.c																  				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Hierarchical State Machine																		    		*
*  Description			:  This file contains Basic HSM API's definitions																						*
*																											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    File Inclusion
-----------------------------------------------------------------------------*/
#include "hsm_api.h"


/*-----------------------------------------------------------------------------
			  Variable Definition(static) 
-----------------------------------------------------------------------------*/


/* This message is delivered while entering into a state.
 *
 * While transiting from current state to target state , ENTRY message(st_EntryMsg)
 * is received by all parent states of target state including target state. 
 */
static Ts_Sys_Msg st_EntryMsg = { { NULL }, 0, HSM_CID, HSM_MSGID_ENTRY, 0, { 0 } };


/* This message is delivered while starting a state.
 *
 * During state transition,once the target state is reached, START message(st_StartMsg) will be 
 * delivered to target state soon after ENTRY message. 
 * 
 * Note: START message is not delivered to all parent states of target state but only delivered to 
 *		 target state.
 */
static Ts_Sys_Msg st_StartMsg = { { NULL }, 0, HSM_CID, HSM_MSGID_START, 0, { 0 } };



/* This message is delivered while leaving a state */
static Ts_Sys_Msg st_ExitMsg = { { NULL }, 0, HSM_CID, HSM_MSGID_EXIT, 0, { 0 } };


/*-----------------------------------------------------------------------------
			  Macro Definition
-----------------------------------------------------------------------------*/

/*
 * Calls current state handler with the given message
 *
 * Use this macro to check whether given message
 * can be handled by current state of HSM
 *
 * param[in]  
 *			pst_state		pointer to the state whose function handler will be called 
 *			pst_me			Pointer to the HSM which shall perform the transition 
 *			pst_msg			Pointer to the message to be handled		
 *			
 *
 */
#define HSM_STATE_HANDLER_CALL(pst_state, pst_me, pst_msg) ((*(pst_state)->fp_hndlr )((pst_me), (pst_msg)))

/*-----------------------------------------------------------------------------
    Private Function Definitions
-----------------------------------------------------------------------------*/

static const Ts_hsm_state* Find_Least_Cmn_Ancestor(const Ts_hsm_state  *pst_curr,const Ts_hsm_state  *pst_next)
{
	const Ts_hsm_state *pst_lca = NULL ;
	
	while(pst_curr != NULL )
	{
		const Ts_hsm_state *pst_top = pst_next ;
		
		while(pst_top != NULL )
		{
			if( pst_curr == pst_top )
			{
				pst_lca   =  pst_curr ;		/* Assign current state as LCA state   */
				pst_top   =  NULL;			/*Condition breaks inner while loop */
				pst_curr  =  NULL;			/*Condition breaks outer while loop */
			}
			else
			{
				pst_top   = pst_top -> pst_parent ;	
			}		
		}
		
		/* Checking pst_curr is Valid State */
		if(pst_curr != NULL)
		{
			pst_curr = pst_curr -> 	pst_parent;
		}
		else
		{
			/*Do nothing*/
		}	
	}
	
	return pst_lca ;
}

/*-----------------------------------------------------------------------------
    Public Function Definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*				void HSM_OnStart											 */
/*===========================================================================*/
void HSM_OnStart(Ts_hsm *pst_me)
{
		if(pst_me != NULL)
		{    
			/* At start of the hsm the current state is the top state */
			pst_me->pst_curr = pst_me->pst_top;   

			/*Calling state handler of Top State with Entry msg*/
			(void)HSM_STATE_HANDLER_CALL(pst_me->pst_curr, pst_me, &st_EntryMsg);
	        
			/* Send first start event to the top state and handle possible state transitions */
			HSM_OnMsg(pst_me, &st_StartMsg);
		} 
}

/*===========================================================================*/
/*				void HSM_OnMsg												 */		
/*===========================================================================*/
void HSM_OnMsg(Ts_hsm *pst_me, Ts_Sys_Msg *pst_msg)
{
		if(pst_me != NULL)
		{				
			const Ts_hsm_state *pst_lstate = pst_me -> pst_curr ;
			
			while(pst_lstate != NULL )
			{
			
				/*  HSM_STATE_HANDLER_CALL() calls Current state handler to check whether given message can be handled by current state of HSM  */
				if(  (  HSM_STATE_HANDLER_CALL(pst_lstate, pst_me,pst_msg) == NULL )  ||  ( pst_lstate -> pst_parent == NULL )  )
				{
					/* Checking for Transition */	
					while( pst_me -> pst_next != NULL)
					{
						const Ts_hsm_state *pst_lca = NULL ;
						
						/* Finds LCA for current and next State */
						pst_lca = Find_Least_Cmn_Ancestor(pst_me-> pst_curr,pst_me -> pst_next );
						
						/* Checking whether LCA is found */
						if(pst_lca != NULL)
						{
							const Ts_hsm_state  *pst_Temp_State = pst_me -> pst_curr ;
							
							/* Exit all states till LCA is reached */
							while(pst_Temp_State != pst_lca)
							{
								/* Calling current state handler with Exit msg */
								(void)HSM_STATE_HANDLER_CALL(pst_Temp_State, pst_me,&st_ExitMsg);
								
								/* Assigning current state to parent state of the one which we left*/
								pst_me -> pst_curr = pst_Temp_State -> pst_parent ;
								
								pst_Temp_State = pst_Temp_State -> pst_parent ;								
							}
							
						{
							/* pst_Trace_Path stores list of states from Next state to lca */
							const Ts_hsm_state  *pst_Trace_Path[HSM_MAX_STATE_NESTING] ={NULL};
							
							Tu8 u8_Trace_Index = 0;
							
							for(pst_Temp_State = pst_me -> pst_next ;pst_Temp_State != pst_lca  ; pst_Temp_State = pst_Temp_State -> pst_parent   )
							{
								pst_Trace_Path[u8_Trace_Index] = pst_Temp_State ;	 
								
								if(u8_Trace_Index >= HSM_MAX_STATE_NESTING )
								{
									RADIO_DEBUG_LOG (RADIO_LOG_LVL_WARNING, "[RADIO][FW]MAXIMUM NESTING STATE LIMIT IS CROSSED IN HSM \n");
								}	
								u8_Trace_Index++;								
							}	 
							
							/* Entering into the states from LCA to next state */
							while(u8_Trace_Index > 0)
							{
								u8_Trace_Index--;
								
								/* Assigning current state to the one which we are going to enter  */
								pst_me -> pst_curr =  pst_Trace_Path[u8_Trace_Index];
								
								/*Calling current state handler with Entry msg*/
								(void)HSM_STATE_HANDLER_CALL(pst_me -> pst_curr,pst_me,&st_EntryMsg );
							}		
						}	
							/* Transition is done */
							
							pst_me ->  pst_next = NULL ;  /* Set next State to NULL as transition done*/
							
							/*Calling current state handler with Start msg*/
							(void)HSM_STATE_HANDLER_CALL(pst_me -> pst_curr,pst_me,&st_StartMsg);	
						}	
						else
						{						
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW]Invalid HSM LCA\n");
						}	
					}
					pst_lstate = NULL;
				}
				else
				{
					pst_lstate = pst_lstate -> pst_parent ;		/* Give message to parent state as can't handle in current state */	
				}			
			}			
		}
}

/*===========================================================================*/
/*				void HSM_StateTransition									 */
/*===========================================================================*/
void HSM_StateTransition(Ts_hsm* pst_me, const Ts_hsm_state* pst_target)
{
   
    if (pst_target == NULL)
    {	
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW]HSM TRANSITION IS NULL\n");
    }
	else
	{
		/* Do nothing*/
	}    
    pst_me->pst_next = pst_target;	
}

/*===========================================================================*/
/*				void HSM_HsmCtor											 */
/*===========================================================================*/
void HSM_HsmCtor(Ts_hsm *pst_me, const Ts_hsm_state *pst_top,const Tu16 u16_cid)
{
    
    pst_me->pst_curr  = NULL;
    pst_me->pst_next  = NULL;
	pst_me->pst_top   = pst_top;
	pst_me->u16_cid	  = u16_cid;
}
/*=============================================================================
    end of file
=============================================================================*/