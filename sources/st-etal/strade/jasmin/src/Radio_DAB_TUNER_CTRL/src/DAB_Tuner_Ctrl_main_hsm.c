/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_tuner_ctrl_main_hsm.c																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: DAB Tuner Control															     		*
*  Description			: This file contains function definitions of main HSM.																		*
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_main_hsm.h"
#include "DAB_Tuner_Ctrl_Response.h"
#include "DAB_Tuner_Ctrl_app.h"
#include "sys_nvm.h"
#include "lib_bitmanip.h"

/*--------------------------------------------------------------------------------------------------
    variables (extern)
--------------------------------------------------------------------------------------------------*/
extern Tu16 u16_usermask;
Ts_Tuner_Ctrl_main_timer st_main_TimerId;


/* HSM state hierarchy */
  HSM_CREATE_STATE(dab_tuner_ctrl_main_hsm_top_state, 					NULL, 									DAB_TUNER_CTRL_MAIN_HSM_TopHndlr,			"dab_tuner_ctrl_main_hsm_top_state");
  	HSM_CREATE_STATE(dab_tuner_ctrl_main_hsm_inactive_state, 			&dab_tuner_ctrl_main_hsm_top_state, 	DAB_TUNER_CTRL_MAIN_HSM_InactiveHndlr,		"dab_tuner_ctrl_main_hsm_inactive_state");
    HSM_CREATE_STATE(dab_tuner_ctrl_main_hsm_active_state, 				&dab_tuner_ctrl_main_hsm_top_state,		DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr,		"dab_tuner_ctrl_main_hsm_active_state");
    	HSM_CREATE_STATE(dab_tuner_ctrl_main_hsm_active_start_state,    &dab_tuner_ctrl_main_hsm_active_state,  DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr,   "dab_tuner_ctrl_main_hsm_active_start_state");
    	HSM_CREATE_STATE(dab_tuner_ctrl_main_hsm_active_idle_state,     &dab_tuner_ctrl_main_hsm_active_state,  DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr,    "dab_tuner_ctrl_main_hsm_active_idle_state");
    	HSM_CREATE_STATE(dab_tuner_ctrl_main_hsm_active_stop_state,     &dab_tuner_ctrl_main_hsm_active_state,  DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr,    "dab_tuner_ctrl_main_hsm_active_stop_state");
/*-----------------------------------------------------------------------------
                                        Type Definitions
-----------------------------------------------------------------------------*/

  /*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  static Ts_msg*  DAB_TUNER_CTRL_MAIN_HSM_TopHndlr                         */
/*===========================================================================*/

  Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_TopHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
  {
    
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
  	PRINT_MSG_DATA(msg);
   switch(msg->msg_id)
   {
     case HSM_MSGID_ENTRY:
	
          break;

     case HSM_MSGID_START:
	 {
		  RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_MAIN_HSM_TopHndlr ");
		  SYS_RADIO_MEMCPY((void*) DAB_Tuner_Ctrl_me->u8_curr_state_str , "DAB_TUNER_CTRL_MAIN_HSM_TopHndlr" ,sizeof("DAB_TUNER_CTRL_MAIN_HSM_TopHndlr"));
          HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_inactive_state);
          
	 }
	 	break;

     case HSM_MSGID_EXIT:  
          break;

    default:
          /* in top state throw system error */
             ret = msg;
             break;
   
   }
   
     return ret;
  
  }
  
  
  /*===========================================================================*/
/*  static Ts_msg*  DAB_TUNER_CTRL_MAIN_HSM_InactiveHndlr                                     */
/*===========================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_InactiveHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{

	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
  	Tu32 Slot = 0;
  	PRINT_MSG_DATA(msg);
   switch(msg->msg_id)
   {
     case HSM_MSGID_ENTRY:
          break;

	 case HSM_MSGID_START:
	 {
		 RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_MAIN_HSM_InactiveHndlr ");
		 SYS_RADIO_MEMCPY( (void*)DAB_Tuner_Ctrl_me->u8_curr_state_str , "DAB_TUNER_CTRL_MAIN_HSM_InactiveHndlr" ,sizeof("DAB_TUNER_CTRL_MAIN_HSM_InactiveHndlr"));
	 }	 
		 break;
	 case DAB_TUNER_CTRL_ACTIAVTE_DEACTIVATE_REQID:
	 {
	 	ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_ActivateDeActivateStatus),msg->data,sizeof(Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus),&Slot);
		if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_ActivateDeActivateStatus == DAB_TUNER_CTRL_ACTIVATE_REQUEST)
		{
			DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd = DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_VALID;
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_active_start_state);
		}
		else if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_ActivateDeActivateStatus == DAB_TUNER_CTRL_DEACTIVATE_REQUEST)
		{
			DAB_Tuner_Ctrl_Response_Activate_Deactivate(REPLYSTATUS_SUCCESS);
		}
		else
		{
		}
	 }
	 break;

     case DAB_TUNER_CTRL_STARTUP_REQID:
	 	{
		  	ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_market),msg->data,sizeof(Te_DAB_Tuner_Market),&Slot);
		  	ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.u8_SettingStatus),msg->data,sizeof(Tu8),&Slot);
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.u8_StartType),msg->data,sizeof(Tu8),&Slot);
			
		  	/* Announcement setting is checked*/		
			if(LIB_ISBITSET(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.u8_SettingStatus, 0))
			{
				/* All announcement types are ON */
				u16_usermask = DAB_TUNER_CTRL_ANNO_SETCONFIG;
			}
			else
			{
				/* All announcement types are OFF */
				u16_usermask = DAB_TUNER_CTRL_NO_ANNO_SET;
			}
			
          	HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_active_start_state);
		 } 
          break;

     case HSM_MSGID_EXIT:
          break;

    default:
      /* in top state throw system error */
      ret = msg;   
      break;
   }
   
     return ret;

}

   /*===========================================================================*/
/*  static Ts_msg*  DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr                                     */
/*===========================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{

     	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
  		Tu32 Slot = 0;
  		PRINT_MSG_DATA(msg);
		switch(msg->msg_id)
		{
			case HSM_MSGID_ENTRY:
			break;
		
			case HSM_MSGID_START: 
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr ");
				//DAB_Tuner_Ctrl_me->u8_curr_state_str = "DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr";
				SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr" ,sizeof("DAB_TUNER_CTRL_MAIN_HSM_ActiveHndlr"));
			break;

			case DAB_TUNER_CTRL_SHUTDOWN_REQID:
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_active_stop_state);
			break;

			case DAB_TUNER_CTRL_REQID:
			case DAB_TUNER_CTRL_RESID:
			{
				DAB_TUNER_CTRL_INST_HSM_HandleMessage(&(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver),msg);
			}	 
			break;

			case DAB_TUNER_CTRL_RESET_FACTORY_SETTINGS_RESID:
			{
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_inactive_state);		
			}  
			break;
		    
			case DAB_TUNER_CTRL_ACTIAVTE_DEACTIVATE_REQID:
			{ 
				ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_ActivateDeActivateStatus),msg->data,sizeof(Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus),&Slot);
				if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_ActivateDeActivateStatus == DAB_TUNER_CTRL_ACTIVATE_REQUEST)
				{
					DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd = DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_VALID;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_active_start_state);
				}
				else if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_ActivateDeActivateStatus == DAB_TUNER_CTRL_DEACTIVATE_REQUEST)
				{
					DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd = DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_VALID;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_active_stop_state);
				}
				else
				{
				}
			}
			break;
			case HSM_MSGID_EXIT:
			break;

			default:
				/* in top state throw system error */
				ret = msg;
			break;
		}
		return ret;
}
   

/*===========================================================================*/
/*  static Ts_msg*  DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr                 */
/*===========================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
  	PRINT_MSG_DATA(msg);
   switch(msg->msg_id)
   {
		case HSM_MSGID_ENTRY:

        break;

		case HSM_MSGID_START:
        {
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str = "DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str , "DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr" ,sizeof("DAB_TUNER_CTRL_MAIN_HSM_ActiveStartHndlr"));

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Sending DAB Tuner Ctrl instantce HSM start up request ");
			DAB_Tuner_Ctrl_hsm_inst_start(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_INST_HSM_STARTUP); 	
		}
		break;
			 
		case SYSTEM_ERROR_NOTIFICATION :
		{
			(void)DabTuner_SystemError_not(&(DAB_Tuner_Ctrl_me->st_SystemError_not), (Ts8*)(msg->data));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] System Error Notification ProcessorId = %x  ErrorType = %x \n", DAB_Tuner_Ctrl_me->st_SystemError_not.ProcessorId,DAB_Tuner_Ctrl_me->st_SystemError_not.ErrorType );
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] %s \n", DAB_Tuner_Ctrl_me->st_SystemError_not.DataByte );			
						
		}
		break;
	
	
	case DAB_TUNER_CTRL_INST_HSM_STARTUP_DONE:
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DAB Tuner Ctrl instantce HSM start up done successful ");
		if(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd == DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_VALID)
		{
			DAB_Tuner_Ctrl_Response_Activate_Deactivate(REPLYSTATUS_SUCCESS);
			DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd = DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_INVALID;
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_active_idle_state);
		}
		else
		{
			DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS ;
			DAB_Tuner_Ctrl_Response_Startup(DAB_Tuner_Ctrl_me->e_ReplyStatus);
	        HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_active_idle_state);
		}	
	}
          break;

	case HSM_MSGID_EXIT:
          break;
    
	default:
      /* in top state throw system error */
      ret = msg;   
      break;
   }
   
     return ret;


}

/*===========================================================================*/
/*  static Ts_msg*  DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr                                     */
/*===========================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
   	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
  	PRINT_MSG_DATA(msg);
   switch(msg->msg_id)
   {
     case HSM_MSGID_ENTRY:
		   break;

     case HSM_MSGID_START:
		 RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr ");
		 //DAB_Tuner_Ctrl_me->u8_curr_state_str = "DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr";
		 SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str , "DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr" ,sizeof("DAB_TUNER_CTRL_MAIN_HSM_ActiveIdleHndlr"));
          break; 

	case DAB_TUNER_CTRL_REQID:
	case DAB_TUNER_CTRL_RESID:
	{
		 DAB_TUNER_CTRL_INST_HSM_HandleMessage(&(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver),msg);
	}
	break;
		 
	case DAB_TUNER_CTRL_DABTUNER_RESTART_REQID:
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Sending Reset Command to DABTUNER (As DAB Tuner is in Abnormal state)");
		/* Update Enum to store Restart Request is valid */
		DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd = DAB_TUNER_CTRL_DABTUNER_RESTART_REQ_VALID ;
			DAB_Tuner_Ctrl_hsm_inst_start(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_INST_HSM_STARTUP); 
		
	}
	break;
	
	case SYSTEM_ERROR_NOTIFICATION :
	{
		(void)DabTuner_SystemError_not(&(DAB_Tuner_Ctrl_me->st_SystemError_not), (Ts8*)(msg->data)) ;
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] System Error Notification ProcessorId = %x  ErrorType = %x \n", DAB_Tuner_Ctrl_me->st_SystemError_not.ProcessorId,DAB_Tuner_Ctrl_me->st_SystemError_not.ErrorType );
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] %s \n", DAB_Tuner_Ctrl_me->st_SystemError_not.DataByte );		
	}
	break;
	
	case DAB_TUNER_CTRL_INST_HSM_STARTUP_DONE:
	{
		if(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd == DAB_TUNER_CTRL_DABTUNER_RESTART_REQ_VALID)
		{
			DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_ReplyStatus = REPLYSTATUS_SUCCESS ;
			DAB_Tuner_Ctrl_Response_DABTUNERRestart(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_ReplyStatus) ;
			DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd = DAB_TUNER_CTRL_DABTUNER_RESTART_REQ_INVALID ;
		}
		else
		{
			/* FOR MISRA */
		}
	
	}
    break;	
			 
	default:

		/* in top state throw system error */
      ret = msg;
      break;
   }

     return ret;
}
  
  
/*===========================================================================*/
/*  static Ts_msg*  DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr                                     */
/*===========================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{

   	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */ 
  	PRINT_MSG_DATA(msg);
   switch(msg->msg_id)
   {
     case HSM_MSGID_ENTRY:
          break;
	 
	 case HSM_MSGID_START:
		 {
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str = "DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str , "DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr",sizeof("DAB_TUNER_CTRL_MAIN_HSM_ActiveStopHndlr"));
			DAB_Tuner_Ctrl_hsm_inst_stop(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_INST_SHUTDOWN); 
		 }
		 break;
    
	 case DAB_TUNER_CTRL_INST_SHUTDOWN_DONE:
	 {
		if(DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd == DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_VALID)
		{
			DAB_Tuner_Ctrl_Response_Activate_Deactivate(REPLYSTATUS_SUCCESS);
			DAB_Tuner_Ctrl_me->st_dab_tuner_receiver.e_DABTUNERRestartCmd = DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_INVALID;	
		}
		HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_inactive_state);
	 }		
        break;
     
	 case HSM_MSGID_EXIT:
        break;
    
	 default:
      /* in top state throw system error */
      ret = msg;   
      break;
   }
   
     return ret;


}

   



  
  /*===========================================================================*/
/*  void DAB_Tuner_Ctrl_HSM_Init                                                    */
/*===========================================================================*/
void DAB_Tuner_Ctrl_HSM_Init(Ts_Tuner_Ctrl_main* DAB_Tuner_Ctrl_me)
{
   if(DAB_Tuner_Ctrl_me!= NULL)
   {
   
      /* clear all hsm */
	//  DAB_TUNER_CTRL_APP_Ctor(pMain);

	 memset(DAB_Tuner_Ctrl_me,0,sizeof(Ts_Tuner_Ctrl_main));

	DAB_Tuner_Ctrl_me->u8_curr_state_str = DAB_Tuner_Ctrl_me->str_state;
	  
	   /* Call the base class Ctor */
      HSM_CTOR((Ts_hsm*)DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_main_hsm_top_state, RADIO_DAB_TUNER_CTRL);

	  	  /* start HSM */
      HSM_ON_START((Ts_hsm*)DAB_Tuner_Ctrl_me);

	  DAB_Tuner_Ctrl_INST_HSM_Init(&DAB_Tuner_Ctrl_me->st_dab_tuner_receiver);
	
	
   }

   else
   {
     // SYS_MSG_GET_LID(DAB_TUNER_CTRL_MAIN_CID, DAB_TUNER_CTRL_ERROR);
    
	}
}
 
/*=============================================================================
    end of file
=============================================================================*/

	