
/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Main_hsm.c                                                                        *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     * 
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: SC_AMFM_TUNER_CTRL                                                                *
*  Description			: Every request accessed by main hsm handlers and routed to instance hsm            *
*                         sending startup and shutdown respone to AMFM application                          *
*                         layer through respective handlers                                                 *
*                                                                                                           *
*                                                                                                           *  
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "AMFM_Tuner_Ctrl_Main_hsm.h"
#include "AMFM_Tuner_Ctrl_Response.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/
/* HSM state hierarchy */
  HSM_CREATE_STATE(			amfm_tuner_ctrl_hsm_top_state,                	NULL,                            	AMFM_TUNER_CTRL_HSM_TopHndlr,        	"amfm_tuner_ctrl_hsm_top_state");
	HSM_CREATE_STATE(		amfm_tuner_ctrl_hsm_inactive_state,        		&amfm_tuner_ctrl_hsm_top_state,   	AMFM_TUNER_CTRL_HSM_InactiveHndlr,  	"amfm_tuner_ctrl_hsm_inactive_state");
	HSM_CREATE_STATE(		amfm_tuner_ctrl_hsm_active_state,          		&amfm_tuner_ctrl_hsm_top_state,   	AMFM_TUNER_CTRL_HSM_ActiveHndlr,     	"amfm_tuner_ctrl_hsm_active_state");
		HSM_CREATE_STATE(	amfm_tuner_ctrl_hsm_active_start_state,			&amfm_tuner_ctrl_hsm_active_state,	AMFM_TUNER_CTRL_HSM_ActiveStartHndlr,	"amfm_tuner_ctrl_hsm_active_start_state");
		HSM_CREATE_STATE(	amfm_tuner_ctrl_hsm_active_idle_state, 			&amfm_tuner_ctrl_hsm_active_state,	AMFM_TUNER_CTRL_HSM_ActiveIdleHndlr, 	"amfm_tuner_ctrl_hsm_active_idle_state");
		HSM_CREATE_STATE(	amfm_tuner_ctrl_hsm_active_stop_state, 			&amfm_tuner_ctrl_hsm_active_state,	AMFM_TUNER_CTRL_HSM_ActiveStopHndlr, 	"amfm_tuner_ctrl_hsm_active_stop_state");
    

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/


/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_TopHndlr                                     */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_TopHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 		pst_ret = 	NULL; /* mark the message as handled */
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_HSM_TopHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl->p_curr_state_str,"AMFM_TUNER_CTRL_HSM_TopHndlr \n",sizeof("AMFM_TUNER_CTRL_HSM_TopHndlr \n"));
			
        	/* Transit to AMFM tuner control main hsm inactive handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl, &amfm_tuner_ctrl_hsm_inactive_state);
		}
		break;

		case HSM_MSGID_EXIT:
		{

		}
		break;

		default:
		{
			 /* updating unhandled message result code to AMFM application layer */
			 pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}



/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_InactiveHndlr                    */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_InactiveHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 	pst_ret 	= 	NULL; /* mark the message as handled */
	Tu32 			slot 		= 	0;
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_HSM_InactiveHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl->p_curr_state_str,"AMFM_TUNER_CTRL_HSM_InactiveHndlr \n",sizeof("AMFM_TUNER_CTRL_HSM_InactiveHndlr \n"));
		}
		break;
	  
		case AMFM_TUNER_CTRL_STARTUP_REQID:
		{
			/* Reading Market value from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl->instances.e_Market),pst_msg->data,sizeof(Te_AMFM_Tuner_Ctrl_Market),&slot);

			/* Transit to AMFM tuner control main hsm active start handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl, &amfm_tuner_ctrl_hsm_active_start_state);
		}
		break;
	  
		case HSM_MSGID_EXIT:
		{

		}
		break;

		 default:
		{
			pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}



/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveHndlr                               */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 		pst_ret 	= NULL; /* mark the message as handeled */
	Te_RADIO_ReplyStatus 	e_FactoryresetReplyStatus 	= REPLYSTATUS_INVALID_PARAM;
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_HSM_ActiveHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl->p_curr_state_str,"AMFM_TUNER_CTRL_HSM_ActiveHndlr \n",sizeof("AMFM_TUNER_CTRL_HSM_ActiveHndlr \n"));
		}
		break;
		case AMFM_TUNER_CTRL_SHUTDOWN_REQID:
		{
			/* Transit to AMFM tuner control main hsm active stop handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl, &amfm_tuner_ctrl_hsm_active_stop_state);
		}
		break;
		case AMFM_TUNER_CTRL_FACTORY_RESET_REQID:
		{
			/* Sending AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_REQID message to inst hsm  */
			AMFM_Tuner_Ctrl_stop(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_REQID);
		}
		break;
		case AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_DONE:
		{
			e_FactoryresetReplyStatus = REPLYSTATUS_SUCCESS;
            /* sending respone to AMFM application layer */
		    AMFM_Tuner_Ctrl_Response_FactoryReset(e_FactoryresetReplyStatus);
            /* Transit to AMFM tuner control main hsm inactive handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl, &amfm_tuner_ctrl_hsm_inactive_state);
		}
		break;
	  	case HSM_MSGID_EXIT:
		{
			
		}
		break;
		default:
		{      
			HSM_ON_MSG(&(pst_me_amfm_tuner_ctrl->instances), pst_msg);
		}
		break;
	}
	return pst_ret;
}



/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveStartHndlr                     */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveStartHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*  							pst_ret    = 	NULL;  /* mark the message as handeled */
    Te_RADIO_ReplyStatus 	e_StartupReplyStatus;

	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_HSM_ActiveStartHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl->p_curr_state_str,"AMFM_TUNER_CTRL_HSM_ActiveStartHndlr \n",sizeof("AMFM_TUNER_CTRL_HSM_ActiveStartHndlr \n"));
			
            /* Sending AMFM_TUNER_CTRL_INST_HSM_STARTUP message to inst hsm  */
			AMFM_Tuner_Ctrl_hsm_inst_start(&pst_me_amfm_tuner_ctrl->instances,AMFM_TUNER_CTRL_INST_START_REQID);
								
		}
		break;
        case AMFM_TUNER_CTRL_START_DONE_RESID:
		{
			AMFM_TUNER_CTRL_MSG_INST_HandleMsg(&(pst_me_amfm_tuner_ctrl->instances),pst_msg);
		} 
		break;
		case AMFM_TUNER_CTRL_INST_HSM_START_DONE:
		{
             e_StartupReplyStatus = REPLYSTATUS_SUCCESS;
			/* sending respone to AMFM application layer */
			AMFM_Tuner_Ctrl_Response_Startup(e_StartupReplyStatus);
            /* Transit to AMFM tuner control main hsm active idle handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl, &amfm_tuner_ctrl_hsm_active_idle_state);
		}
		break;
			
		case HSM_MSGID_EXIT:
		{

		}
		break;

		default:
		{
			pst_ret = pst_msg;
		}
		break;
	}
	return pst_ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveIdleHndlr                      */
/*===========================================================================*/

Ts_Sys_Msg* AMFM_TUNER_CTRL_HSM_ActiveIdleHndlr (Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 			pst_ret = NULL; /* mark the message as handeled */

	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_HSM_ActiveIdleHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl->p_curr_state_str,"AMFM_TUNER_CTRL_HSM_ActiveIdleHndlr \n",sizeof("AMFM_TUNER_CTRL_HSM_ActiveIdleHndlr \n"));
			
		}
		break;
        /* All cases which is handled in AMFM_Tuner_Ctrl_instance hsm*/
		case AMFM_TUNER_CTRL_REQID:
		case AMFM_TUNER_CTRL_RESID:
		{
			/* inst HSM messages routed via this function */
            AMFM_TUNER_CTRL_MSG_INST_HandleMsg(&pst_me_amfm_tuner_ctrl->instances, pst_msg);
		}
        break; 
		case HSM_MSGID_EXIT:
		{

		}
		break;
		default:
		{	    
		
			pst_ret = pst_msg;
		}
		break;
	
	}
	return pst_ret;

}

/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveStopHndlr                      */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_HSM_ActiveStopHndlr(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*   							pst_ret    				= NULL;                             /* mark the message as handled */
    Te_RADIO_ReplyStatus 	e_ShutdownReplyStatus 	= REPLYSTATUS_REQ_CANCELLED;
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_HSM_ActiveStopHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl->p_curr_state_str,"AMFM_TUNER_CTRL_HSM_ActiveStopHndlr \n",sizeof("AMFM_TUNER_CTRL_HSM_ActiveStopHndlr \n"));
			
			/* Sending AMFM_TUNER_CTRL_INST_HSM_SHUTDOWN message to inst hsm  */
			AMFM_Tuner_Ctrl_stop(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_STOP);
		}
		break;
		/* AMFM_TUNER_CTRL_STOP_DONE_RESID msg is not posted anywhere so commenting the case
		case AMFM_TUNER_CTRL_STOP_DONE_RESID:
		{
			//shutdown message response routed via this function to instance hsm 
            AMFM_TUNER_CTRL_MSG_INST_HandleMsg(&pst_me_amfm_tuner_ctrl->instances,pst_msg);
		} 
		break;*/
		case AMFM_TUNER_CTRL_INST_HSM_STOP_DONE:
		{
			e_ShutdownReplyStatus = REPLYSTATUS_SUCCESS;
            /* sending respone to AMFM application layer */
		    AMFM_Tuner_Ctrl_Response_Shutdown(e_ShutdownReplyStatus);
            /* Transit to AMFM tuner control main hsm inactive handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl, &amfm_tuner_ctrl_hsm_inactive_state);
		}
		break;
		case HSM_MSGID_EXIT:
		{

		}
		break;
		default:
		{
			pst_ret = pst_msg;
		}
		break;
	}
	return pst_ret;
}

/*===========================================================================*/
/*  void AMFM_TUNER_CTRL_MAIN_HSM_Init                                               */
/*===========================================================================*/

void AMFM_TUNER_CTRL_MAIN_HSM_Init(Ts_AMFM_Tuner_Ctrl_Main_hsm* pst_me_amfm_tuner_ctrl)
{
	if (pst_me_amfm_tuner_ctrl != NULL)
	{
		/* clear all hsm */
		memset(pst_me_amfm_tuner_ctrl,0x00, sizeof(Ts_AMFM_Tuner_Ctrl_Main_hsm)); 
		
		pst_me_amfm_tuner_ctrl->p_curr_state_str =  pst_me_amfm_tuner_ctrl->str_state;

		/* Call the base class Ctor */
		HSM_CTOR((Ts_hsm*)pst_me_amfm_tuner_ctrl, &amfm_tuner_ctrl_hsm_top_state, RADIO_AM_FM_TUNER_CTRL);

		/* start HSM */
		HSM_OnStart((Ts_hsm*)pst_me_amfm_tuner_ctrl);

		AMFM_TUNER_CTRL_INST_HSM_Init(&pst_me_amfm_tuner_ctrl->instances); /*comment is update the index value & cid*/ 
																		   
		
	}
}

/*=============================================================================
    end of file
=============================================================================*/

