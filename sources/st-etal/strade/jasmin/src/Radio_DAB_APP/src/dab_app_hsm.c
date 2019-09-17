/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app_hsm.c																			  			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains state handler functions of DAB APP main HSM.					*
*																											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app.h"
#include "dab_app_hsm.h"
#include "msg_cmn.h"
#include "dab_app_request.h"
#include "dab_app_response.h"
#include "dab_app_notify.h"
//#include "debug_log.h"

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

extern Tu32 u32_Index ;	/* Initialize station list index */

/*	Defining states of DAB_APP_HSM	*/

	HSM_CREATE_STATE(dab_app_hsm_top_state,							NULL,							DAB_APP_HSM_TopHndlr,			"dab_app_hsm_top_state" );
		HSM_CREATE_STATE(dab_app_hsm_inactive_state,				&dab_app_hsm_top_state,			DAB_APP_HSM_InactiveHndlr,		"dab_app_hsm_inactive_state" );
		HSM_CREATE_STATE(dab_app_hsm_active_state,					&dab_app_hsm_top_state,			DAB_APP_HSM_ActiveHndlr,		"dab_app_hsm_active_state");
			HSM_CREATE_STATE(dab_app_hsm_active_start_state,		&dab_app_hsm_active_state,		DAB_APP_HSM_ActiveStartHndlr,	"dab_app_hsm_active_start_state");
			HSM_CREATE_STATE(dab_app_hsm_active_idle_state,			&dab_app_hsm_active_state,		DAB_APP_HSM_ActiveIdleHndlr,	"dab_app_hsm_active_idle_state");
			HSM_CREATE_STATE(dab_app_hsm_active_stop_state,			&dab_app_hsm_active_state,		DAB_APP_HSM_ActiveStopHndlr,	"dab_app_hsm_active_stop_state");
	
	
/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_APP_Init                               						 */
/*===========================================================================*/
void DAB_APP_Init(Ts_dab_app_hsm *pst_me_dab_app)
{
	/* Clear main hsm */
    memset(pst_me_dab_app, (int) 0U, sizeof(Ts_hsm));
	
	pst_me_dab_app->ptr_curr_hdlr_name = (const Tu8 *)(pst_me_dab_app->str_state);

	/* Invoking constructor for main Hsm */
	HSM_CTOR((Ts_hsm*)pst_me_dab_app,&dab_app_hsm_top_state,RADIO_DAB_APP);	
	
	/* Start of Main Hsm */
	HSM_ON_START(pst_me_dab_app);

	/* Initializing Instance Hsm */
	DAB_APP_INST_Init(&pst_me_dab_app->instance);
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_HSM_TopHndlr                             	 */
/*===========================================================================*/
Ts_Sys_Msg*  DAB_APP_HSM_TopHndlr(Ts_dab_app_hsm* pst_me_dab_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* Mark the message as handled */
	

  	PRINT_MSG_DATA(pst_msg);  
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
		
		}
		break;
	
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_HSM_TopHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_HSM_TopHndlr \n";
			/* Transit to inactive state*/
			HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_inactive_state);
		}
		break;
		
		case HSM_MSGID_EXIT:
		{
		
		}
		break;

		default:
		{
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
	}

	return ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_HSM_InactiveHndlr                            */
/*===========================================================================*/
Ts_Sys_Msg*  DAB_APP_HSM_InactiveHndlr(Ts_dab_app_hsm* pst_me_dab_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* Mark the message as handled */
	

  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_HSM_InactiveHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_HSM_InactiveHndlr \n";
        }
		break;
     
	 	case DAB_APP_ACTIVATE_DEACTIVATE_REQID:
		{
			u32_Index = 0;	/* Initializing variable to store station list index */
			/* Extracting market from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app->instance.e_DAB_App_ActivateDeactivateStatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_ActivateDeactivateStatus),&u32_Index);
			if(pst_me_dab_app->instance.e_DAB_App_ActivateDeactivateStatus == DAB_APP_ACTIVATE_REQUEST)
			{
				pst_me_dab_app->instance.e_DAB_App_Satrtup_Request_Type = DAB_APP_ACTIVATE_REQ_VALID;
				HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_active_start_state);
			}
			else if(pst_me_dab_app->instance.e_DAB_App_ActivateDeactivateStatus == DAB_APP_DEACTIVE_REQUEST)
			{
				DAB_App_Response_Activate_Deactivate(REPLYSTATUS_SUCCESS);
			}
			else
			{
				
			}
			
		}
		break;
		case DAB_APP_STARTUP_REQID:
		{		
			u32_Index = 0;	/* Initializing variable to store station list index */
			
			/* Extracting market from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app->instance.e_Market,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_Market),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app->instance.u8_SettingStatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu8),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app->instance.u8_StartType,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu8),&u32_Index);

			/* Transit to active start state*/
			HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_active_start_state);	
		}	
		break;	
		
		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
	}
	return ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_HSM_ActiveHndlr                              */
/*===========================================================================*/
Ts_Sys_Msg*  DAB_APP_HSM_ActiveHndlr(Ts_dab_app_hsm* pst_me_dab_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	

  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_HSM_ActiveHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_HSM_ActiveHndlr \n";					
		}
		break;
		case DAB_APP_STARTUP_DONE_RESID:
		case DAB_APP_GETSTL_REQID:
		{	
			/* Passing Startup and GetSTL Tuner Ctrl response to Instance HSM */
			DAB_APP_INST_HSM_HandleMessage(pst_msg);	
		}
		break;
		case DAB_APP_SHUTDOWN_REQID:
		{
			/* Transit to active stop state*/
			HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_active_stop_state);	
		}
		break;
		case DAB_APP_INST_HSM_SHUTDOWN_DONE:
		{
			/* Transit to inactive state*/
			HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_inactive_state);	

		}
		break;

	 	case DAB_APP_ACTIVATE_DEACTIVATE_REQID:
		{
			u32_Index = 0;	/* Initializing variable to store station list index */
			
			/* Extracting market from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app->instance.e_DAB_App_ActivateDeactivateStatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_ActivateDeactivateStatus),&u32_Index);
			if(pst_me_dab_app->instance.e_DAB_App_ActivateDeactivateStatus == DAB_APP_ACTIVATE_REQUEST)
			{
				pst_me_dab_app->instance.e_DAB_App_Satrtup_Request_Type = DAB_APP_ACTIVATE_REQ_VALID;
				HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_active_start_state);
			}
			else if(pst_me_dab_app->instance.e_DAB_App_ActivateDeactivateStatus == DAB_APP_DEACTIVE_REQUEST)
			{
				pst_me_dab_app->instance.e_DAB_App_Satrtup_Request_Type = DAB_APP_ACTIVATE_REQ_VALID;
				HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_active_stop_state);
			}
			else
			{
				
			}
		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
	}
	return ret;
    
}


/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_HSM_ActiveStartHndlr                         */
/*===========================================================================*/
Ts_Sys_Msg*  DAB_APP_HSM_ActiveStartHndlr(Ts_dab_app_hsm* pst_me_dab_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	

  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_HSM_ActiveStartHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_HSM_ActiveStartHndlr \n";

			/* Sending Startup message to instance hsm */
			DAB_Inst_App_Request_Startup();
	
		}
		break;

		case DAB_APP_INST_HSM_START_DONE:
		{
			/* Transit to active idle state*/
			HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_active_idle_state);	

		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
	}
	return ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_HSM_ActiveIdleHndlr                          */
/*===========================================================================*/
Ts_Sys_Msg*  DAB_APP_HSM_ActiveIdleHndlr(Ts_dab_app_hsm* pst_me_dab_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	

  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_HSM_ActiveIdleHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_HSM_ActiveIdleHndlr \n";				
		}
		break;

		case DAB_APP_SELECT_DAB_REQID:
		case DAB_APP_DESELECT_DAB_REQID:
		case DAB_APP_PLAY_SEL_STN_REQID:
		case DAB_APP_SER_COMP_SEEK_REQID:
		case DAB_APP_GETSTL_REQID:
		case DAB_APP_CANCEL_REQID:
		case DAB_APP_TUNEUPDOWN_REQID:
		case DAB_APP_INTERNAL_ANNO_MSG:
		case DAB_APP_CANCEL_ANNO_REQID:
		case DAB_APP_ANNO_CONFIG_REQID:
		case DAB_APP_AF_TUNE_REQID:
		case DAB_APP_DABTUNER_RESTART_REQID:
		case DAB_APP_ENG_MODE_REQID:
		case DAB_APP_FM_DAB_LINKING_PI:
		case DAB_APP_SCAN_RESID:
		case DAB_APP_ACTIVATE_RESID:
		case DAB_APP_DEACTIVATE_RESID:
		case DAB_APP_PLAY_SEL_STN_RESID:
		case DAB_APP_DAB_AF_SETTINGS_REQID:
		case DAB_APP_FACTORY_RESET_REQID:
		case DAB_APP_MANUAL_TUNEBY_CHNAME_REQID:
		
		case DAB_APP_CANCEL_RESID:
		case DAB_APP_CANCEL_ANNO_RESID:
		case DAB_APP_ANNO_CONFIG_RESID:
		case DAB_APP_START_ANNO_RESID:
		case DAB_APP_STOP_ANNO_RESID:
		case DAB_APP_DABTUNER_RESTART_RESID:
		case DAB_APP_AF_LIST_RESID:
		case DAB_APP_ACTIVATE_DEACTIVATE_RESID:
		case DAB_APP_DAB_AF_SETTINGS_RESID:
		case DAB_APP_DAB_AF_LEARN_MEM_NOTIFYID:
		case DAB_APP_STATIONNOTAVAIL_STRATERGY_STATUS_NOTIFYID:
		case DAB_APP_FACTORY_RESET_DONE_RESID:
		case DAB_APP_ABORT_SCAN_RESID:
		
		case DAB_APP_ANNO_NOTIFYID:
		case DAB_APP_STL_UPDATE_NOTIFYID:
		case DAB_APP_FREQ_CHANGE_NOTIFYID:
		case DAB_APP_STATUS_NOTIFYID:
		case DAB_APP_DAB_FM_LINKING_ENABLE_REQID:
		case DAB_APP_DAB_FM_LINKING_ENABLE_RESID:
		case DAB_APP_PICODE_LIST_NOTIFYID:
		case DAB_APP_BESTPI_NOTIFYID:
		case DAB_APP_DAB_FM_LINKING_STATUS_NOTIFYID:
		case DAB_APP_DAB_FM_BLENDING_STATUS_NOTIFYID:
		case DAB_APP_PI_QUALITY_NOTIFYID:
		case DAB_APP_DAB_DLS_DATA_NOTIFYID:
		case DAB_APP_DAB_SLS_DATA_NOTIFYID:
		case DAB_APP_RECONFIG_NOTIFYID:
		case DAB_APP_FM_DAB_LINKING_STATION_NOTIFYID:
		case DAB_APP_FM_DAB_LINKING_STATUS_NOTIFYID:
		case DAB_APP_ANNO_STATION_INFO_NOTIFYID:
		case DAB_APP_DAB_FM_HARDLINKS_STATUS_NOTIFYID:
		case DAB_APP_ANNO_SIGNAL_LOSS_NOTIFYID:
		case DAB_APP_COMPONENT_STATUS_NOTIFYID:
		case DAB_APP_AMFMTUNER_STATUS_NOTIFYID :
		case DAB_APP_DAB_DAB_STATUS_NOTIFYID:
		case DAB_APP_SIGNAL_STATUS_NOTIFYID:
		case DAB_APP_BACKGROUND_SCAN_START_NOTIFYID:
		case DAB_APP_FM_DAB_STOP_LINKING_NOTIFYID:
		case DAB_APP_INIT_FMDAB_LINKING_NOTIFYID:
		case DAB_APP_COMP_LIST_SORT_REQID:
		case DAB_APP_GET_COMP_LIST_STATUS_NOTIFYID:
		case DAB_APP_AUTOSCAN_PLAY_STATION_NOTIFYID:
		case DAB_APP_SYNCHRONISATION_NOTIFYID:
		{
			/* Passing Radio Mngr request and Tuner Ctrl response from Main HSM to Instance HSM */
			DAB_APP_INST_HSM_HandleMessage(pst_msg);
		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
	}
	return ret;
}


/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_HSM_ActiveStopHndlr                          */
/*===========================================================================*/
Ts_Sys_Msg*  DAB_APP_HSM_ActiveStopHndlr(Ts_dab_app_hsm* pst_me_dab_app, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_HSM_ActiveStopHndlr ");

  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			/* Storing name of current state handler */
			pst_me_dab_app->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_HSM_ActiveStopHndlr \n";	

			/* Sending Shutdown request to instance hsm */
			DAB_Inst_App_Request_Shutdown();

		}
		break;

		case DAB_APP_SHUTDOWN_RESID:
		{
			DAB_APP_INST_HSM_HandleMessage(pst_msg);
		}
		break;
		case DAB_APP_INST_HSM_SHUTDOWN_DONE:
		{
			/* Transit to inactive state*/
			HSM_STATE_TRANSITION(pst_me_dab_app, &dab_app_hsm_inactive_state);	

		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
	}
	return ret;
}

/*=============================================================================
    end of file
=============================================================================*/
