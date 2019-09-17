/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_hsm.c																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This source file consists of function defintions of all function handlers of HSM	*
*						  amfm_app_hsm(main) of AMFM Applicatiopn component									*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "hsm_api.h"
#include "amfm_app_hsm.h"
//#include "debug_log.h"

/*-----------------------------------------------------------------------------
	Global variable Definitions (static)	                                        
-----------------------------------------------------------------------------*/

/* Defining states of HSM amfm_app_hsm */
HSM_CREATE_STATE(			amfm_app_hsm_top_state,				 NULL,						AMFM_APP_HSM_TopHndlr,			"amfm_app_hsm_top_state");
	HSM_CREATE_STATE(		amfm_app_hsm_inactive_state,		&amfm_app_hsm_top_state,	AMFM_APP_HSM_InactiveHndlr,		"amfm_app_hsm_inactive_state");
	HSM_CREATE_STATE(		amfm_app_hsm_active_state,			&amfm_app_hsm_top_state,	AMFM_APP_HSM_ActiveHndlr,		"amfm_app_hsm_active_state");
		HSM_CREATE_STATE(	amfm_app_hsm_active_start_state,	&amfm_app_hsm_active_state,	AMFM_APP_HSM_ActiveStartHndlr,	"amfm_app_hsm_active_start_state");
		HSM_CREATE_STATE(	amfm_app_hsm_active_idle_state,		&amfm_app_hsm_active_state,	AMFM_APP_HSM_ActiveIdleHndlr,	"amfm_app_hsm_active_idle_state");
		HSM_CREATE_STATE(	amfm_app_hsm_active_stop_state,		&amfm_app_hsm_active_state,	AMFM_APP_HSM_ActiveStopHndlr,	"amfm_app_hsm_active_stop_state");
		
/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/		

/*===========================================================================*/
/*				void AMFM_APP_HSM_Init										 */ 
/*===========================================================================*/
void AMFM_APP_HSM_Init(Ts_AMFM_App_hsm *pst_me_amfm_app)
{
	if(pst_me_amfm_app != NULL)
	{
		/* Clearing the HSM amfm_app_hsm buffer */
		memset(pst_me_amfm_app,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_hsm)); 	
		
		pst_me_amfm_app->pu8_curr_state_name = 	(const char *)(pst_me_amfm_app->str_state);
		/*Constructing the HSM amfm_app_hsm */
		HSM_CTOR(&pst_me_amfm_app->st_hsm,&amfm_app_hsm_top_state,RADIO_AM_FM_APP);
		
		/*Initialising the Inst HSM */
		AMFM_APP_INST_HSM_Init(&pst_me_amfm_app->st_inst_hsm_info);

		/* Starting the HSM amfm_app_hsm .After this call,inactive state is reached */
		HSM_ON_START(&pst_me_amfm_app ->st_hsm );			
	}
	else
	{
		/* Send  error message */
	}	
}


/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/


/*===========================================================================*/
/*				Ts_Sys_Msg*  AMFM_APP_HSM_TopHndlr					 */ 
/*===========================================================================*/	
Ts_Sys_Msg*  AMFM_APP_HSM_TopHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL;         /* Pointer to be updated to the  msg pst_msg  at default case*/
	
			
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_HSM_TopHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app->pu8_curr_state_name),(const void *)"amfm_app_hsm_top_state",sizeof("amfm_app_hsm_top_state"));

			/* Transisting to inactive state */
			HSM_STATE_TRANSITION(pst_me_amfm_app, &amfm_app_hsm_inactive_state);
		}
		break;
		
		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Main HSM TopHndlr MSG: %d", pst_msg->msg_id);
			pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}	

/*===========================================================================*/
/*				Ts_Sys_Msg*  AMFM_APP_HSM_InactiveHndlr				 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_HSM_InactiveHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg 				 *pst_ret		= NULL;                         /* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_AMFM_App_Market		 e_MarketType	= AMFM_APP_MARKET_INVALID;      /* enum to update the market type */
    Tu8 					 u8_switch_setting;
	Tu8						 u8_start_up_type;
    Tu32	                 u32_DataSlotIndex = 0;                     /* variable to update the message slot index */
    
	
	
	switch( pst_msg->msg_id )
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
	
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_HSM_InactiveHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app->pu8_curr_state_name),(const void *)"amfm_app_hsm_inactive_state",sizeof("amfm_app_hsm_inactive_state"));	
		}
		break;

		case AMFM_APP_STARTUP_REQID:
		{
			
			/* Reading Market type value from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_MarketType,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_AMFM_App_Market),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&u8_switch_setting,(const Tchar *)(pst_msg->data),(Tu8)sizeof(u8_switch_setting),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&u8_start_up_type ,(const Tchar *)(pst_msg->data),(Tu8)sizeof(u8_switch_setting),&u32_DataSlotIndex);
		
				
			pst_me_amfm_app->st_inst_hsm_info.e_MarketType = e_MarketType;
			pst_me_amfm_app->st_inst_hsm_info.u8_switch_setting=u8_switch_setting;
			pst_me_amfm_app->st_inst_hsm_info.u8_start_up_type=u8_start_up_type;

			/* Transisting to active start state */
			HSM_STATE_TRANSITION(pst_me_amfm_app, &amfm_app_hsm_active_start_state);	
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
/*				Ts_Sys_Msg*  AMFM_APP_HSM_ActiveHndlr				 */  	
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL;                 /* Pointer to be updated to the  msg pst_msg  at default case*/
    
	switch( pst_msg->msg_id )
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
	
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_HSM_ActiveHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app->pu8_curr_state_name),(const void *)"amfm_app_hsm_active_state",sizeof("amfm_app_hsm_active_state"));
		}
		break;

		case AMFM_APP_SHUTDOWN_REQID:
		{
			/* Transisting to active stop state */
			HSM_STATE_TRANSITION(pst_me_amfm_app, &amfm_app_hsm_active_stop_state);	
		}	
		break;
		
		case AMFM_APP_FACTORY_RESET_REQID:
		case AMFM_APP_FACTORY_RESET_DONE_RESID:
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			/* Sending tuner status notification message to inst hsm via this function */
			AMFM_APP_INST_HSM_MessageHandler(&pst_me_amfm_app->st_inst_hsm_info, pst_msg);	

		}
		break;
		
		case AMFM_APP_INST_HSM_RESET_DONE:
		{
			AMFM_App_Response_FactoryReset(pst_me_amfm_app->st_inst_hsm_info.e_FactoryResetReplyStatus);
			/* Transisting to inactive state */
			HSM_STATE_TRANSITION(pst_me_amfm_app, &amfm_app_hsm_inactive_state);	
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
/*				Ts_Sys_Msg*  AMFM_APP_HSM_ActiveStartHndlr			 */ 
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveStartHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL;                 /* Pointer to be updated to the  msg pst_msg  at default case*/

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
	
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_HSM_ActiveStartHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app->pu8_curr_state_name),(const void *)"amfm_app_hsm_active_start_state",sizeof("amfm_app_hsm_active_start_state")); 
	
			/* Sending AMFM_APP_INST_HSM_STARTUP message to inst hsm  */
			AMFM_APP_INST_HSM_SendInternalMsg(&pst_me_amfm_app->st_inst_hsm_info,AMFM_APP_INST_HSM_STARTUP);
		}
		break;

		case AMFM_APP_STARTUP_DONE_RESID :
		{
			/* Sending startup response message to inst hsm via this function */
			AMFM_APP_INST_HSM_MessageHandler(&pst_me_amfm_app->st_inst_hsm_info, pst_msg);		
		}
		break;

		case AMFM_APP_INST_HSM_START_DONE:
		{	
			/* Transisting to active idle state */
			HSM_STATE_TRANSITION(pst_me_amfm_app, &amfm_app_hsm_active_idle_state);	
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
/*				Ts_Sys_Msg*  AMFM_APP_HSM_ActiveIdleHndlr			 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveIdleHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL;             /* Pointer to be updated to the  msg pst_msg  at default case*/
    
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
	
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_HSM_ActiveIdleHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app->pu8_curr_state_name ),(const void *) "amfm_app_hsm_active_idle_state" ,sizeof("amfm_app_hsm_active_idle_state"));	
		}
		break;
		
		/* Add all cases which is handled in instance HSM amfm_app_inst_hsm */

		case AMFM_APP_SELECT_BAND_REQID:
		case AMFM_APP_DESELECT_BAND_REQID:
		case AMFM_APP_GET_STL_REQID:
		case AMFM_APP_SEEK_UP_DOWN_REQID:
        case AMFM_APP_SELECT_STATION_REQID:
        case AMFM_APP_FIND_BEST_PI_REQID:
		case AMFM_APP_BLENDING_STATUS_REQID:
		case AMFM_APP_CANCEL_REQID :
		case AMFM_APP_TUNE_UP_DOWN_REQID:
		case AMFM_APP_AF_SWITCH_REQID:
		case AMFM_APP_SET_AF_REGIONAL_SWITCH_REQID:
		case AMFM_APP_TA_SWITCH_REQID:
		case AMFM_APP_FM_TO_DAB_SWITCH_REQID:
		case AMFM_APP_BACKGROUND_UPDATE_STL_REQID:
		case AMFM_APP_AF_TUNE_REQID:
		case AMFM_APP_ANNOUNCEMENT_CANCEL_REQID:
		case AMFM_APP_ENG_MODE_SWITCH_REQID:
		case AMFM_APP_GET_CT_INFO_REQID:

		case AMFM_APP_AFFREQ_UPDATE_DONE_RESID:
		case AMFM_APP_AFFREQ_CHECK_DONE_RESID:
		case AMFM_APP_SELECT_BAND_DONE_RESID:
		case AMFM_APP_DESELECT_BAND_DONE_RESID:
		case AMFM_APP_GET_STL_DONE_RESID:
		case AMFM_APP_SELECT_STATION_DONE_RESID:
		case AMFM_APP_SEEK_UP_DOWN_DONE_RESID:
		case AMFM_APP_CANCEL_DONE_RESID:
		case AMFM_APP_ANNO_CANCEL_DONE_RESID:
        case AMFM_APP_AF_LOW_SIGNAL_CHECK_DONE_RESID:
		
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		case AMFM_APP_STL_UPDATED_NOTIFYID:
		case AMFM_APP_CURR_FREQUENCY_NOTIFYID:
        case AMFM_APP_QUALITY_NOTIFYID:
		case AMFM_AF_UPDATE_TIMERID:
		case AMFM_CURR_STATION_QUALITY_CHECK_TIMERID:
		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		case AMFM_APP_DAB_FOLLOWUP_NOTIFYID:
		case AMFM_APP_EON_ANNOUNCEMENT_START_NOTIFIED:
		case AMFM_APP_CURRENT_STATION_TA_TP_NOTIFIED:
		case AMFM_APP_EON_INFO_NOTIFIED:
		case AMFM_APP_STOP_DAB2FM_LINKING_NOTIFYID:
		case AMFM_APP_TUNER_AMFMTUNER_ABNORMAL_NOTIFYID:
		case AMFM_APP_DABTUNER_STATUS_NOTIFYID:	
		case AMFM_REGIONAL_QUALITY_CHECK_TIMERID:	
		case AMFM_AF_STATUS_CHECK_NOTIFYID:
		case AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID:	
		case AMFM_APP_NON_RADIO_MODE_NOTIFYID:
		case AMFM_APP_START_AF_STRATEGY_NOTIFYID:
		case AMFM_APP_STA_NOT_AVAIL_STRA_STATUS_NOTIFYID:
		case AMFM_APP_BG_AF_TUNE_UPDATE_TIMERID:
		case AMFM_STRATEGY_AF_UPDATE_TIMERID:
		{
			/* inst HSM messages processed via this function */
			AMFM_APP_INST_HSM_MessageHandler(&pst_me_amfm_app->st_inst_hsm_info, pst_msg);	
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
/*				Ts_Sys_Msg*  AMFM_APP_HSM_ActiveStopHndlr			 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_HSM_ActiveStopHndlr(Ts_AMFM_App_hsm* pst_me_amfm_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL;             /* Pointer to be updated to the  msg pst_msg  at default case*/
    
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
	
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_HSM_ActiveStopHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app->pu8_curr_state_name ),(const void *)"amfm_app_hsm_active_stop_state",sizeof("amfm_app_hsm_active_stop_state"));

			/* Sending AMFM_APP_INST_HSM_SHUTDOWN message to inst hsm  */
			AMFM_APP_INST_HSM_SendInternalMsg(&pst_me_amfm_app->st_inst_hsm_info,AMFM_APP_INST_HSM_SHUTDOWN);			
		}
		break;

		case AMFM_APP_SHUTDOWN_DONE_RESID:
		{
			/* Sending shutdown response message to inst hsm via this function */
			AMFM_APP_INST_HSM_MessageHandler(&pst_me_amfm_app->st_inst_hsm_info, pst_msg);	
		}
		break;
		case AMFM_APP_INST_HSM_SHUTDOWN_DONE :
		{
			/* Transisting to inactive state */
			HSM_STATE_TRANSITION(pst_me_amfm_app, &amfm_app_hsm_inactive_state);	
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

