/*==================================================================================================
    start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_Tuner_Ctrl_inst_hsm.c																   *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.											   *
*  All rights reserved. Reproduction in whole or part is prohibited								   *
*  without the written permission of the copyright owner.										   *
*																								   *
*  Project              : ST_Radio_Middleware																	   *
*  Organization			: Jasmin Infotech Pvt. Ltd.												   *
*  Module				: DAB Tuner Control														   *
*  Description			: This file contains DAB Tuner Control HSM instance HSM function           *
                          definitions.		                                                       *
*																								   *
*																								   *
***************************************************************************************************/
/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "hsm_api.h"
#include "DAB_Tuner_Ctrl_Response.h"
#include "DAB_Tuner_Ctrl_inst_hsm.h"
#include  <string.h>
#include "dab_app_freq_band.h"
#include "DAB_Tuner_Ctrl_app.h"
#include "DAB_Tuner_Ctrl_Announcement.h"
#include  "DAB_HAL_FIC_parsing.h"
#include "sys_nvm.h"
#include "dab_app_request.h"
/*--------------------------------------------------------------------------------------------------
    variables (extern)
--------------------------------------------------------------------------------------------------*/
extern 	Ts_dab_app_frequency_table dab_app_freq_band_eu[] ;
Ts_DabTunerMsg_R_AudioStatus_notify AudioStatus;
extern 	Tbool 	b_delayvalue;
extern  Tu8 	ALternateFreq;
extern	Tbool 	b_Tune_to_Orginal_freq_Check ;
extern	Tu8		u8_Alt_Ensemble_ID;
extern  Ts_DAB_DataServiceRaw st_SLS_Data;
Ts_DAB_Tuner_Ctrl_DataServiceRaw			st_DabTc_SLS_Data;
Ts_Tuner_Ctrl_EnsembleInfo		ast_EnsembleInfo[DAB_APP_MAX_ENSEMBLES];
Ts_Tuner_Ctrl_ServiceInfo		ast_ServiceInfo[DAB_APP_MAX_SERVICES];
Ts_Tuner_Ctrl_ComponentInfo     ast_ComponentInfo[DAB_APP_MAX_COMPONENTS];
Ts_Tuner_Ctrl_Tunableinfo		ast_LearnMem[DAB_APP_MAX_COMPONENTS];
Ts_DabTunerMsg_R_GetVersion_Reply	st_GetVersion_Reply;
Tu16 u16_usermask;
Ts_DabTunerMsg_GetCurrEnsembleProgListReply st_Dab_Tuner_Ctrl_CurrEnsembleProgList;
Tbool b_bgGetCompList_handler = FALSE;

/*--------------------------------------------------------------------------------------------------
    variables (static)
--------------------------------------------------------------------------------------------------*/
Tu16 	ensembleIndex;
Tu16 	Components;
Tu16 	Services;
Tu8		AltEnsembleindex;
Tu8		Hardlinkindex;
Tu8 	Altfreqindex 		= 	0;
Tbool 	b_Hardlinksused		= 	FALSE;
Tbool	b_handlerchanged	= 	FALSE;
Tu8		Same_SID_Search		=	0;
//Tu8 DAB_Tuner_Ctrl_Best_PI_Check = 0;

/* Announcement */
Tbool b_Announcement_Cancel_Flag = FALSE;
Tbool b_Announcement_Stop_Flag = FALSE;
 
Tbool	b_Anno_ongoingflag = FALSE;
// Tu8 u8_EnsembleScanDone =0;

/* FIC Data Enable Flag*/
Tbool b_FIC_Data_Enable = FALSE;

/* AutoSeek Process End Flag */
Tbool b_AutoSeekEnded = FALSE;

Ts_dab_tuner_ctrl_inst_timer st_TimerId;

/*SCAN */
Ts_DabTunerGetEnsembleProperties_reply			st_ScanEnsembleinfo;
Ts_DabTunerMsg_GetCurrEnsembleProgListReply		st_ScanServiceinfo;

/* Etal return status */
ETAL_STATUS etal_ret = ETAL_RET_ERROR;

/* HSM state hierarchy */
  
  HSM_CREATE_STATE(dab_tuner_ctrl_hsm_inst_top_state, 									NULL, 											DAB_TUNER_CTRL_INST_HSM_TopHndlr, 								"dab_tuner_ctrl_hsm_inst_top_state");
  	HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_inactive_state, 							&dab_tuner_ctrl_hsm_inst_top_state,				DAB_TUNER_CTRL_INST_HSM_InactiveHndlr, 							"dab_tuner_ctrl_inst_hsm_inactive_state");
  	HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_active_state, 								&dab_tuner_ctrl_hsm_inst_top_state,				DAB_TUNER_CTRL_INST_HSM_ActiveHndlr, 							"dab_tuner_ctrl_inst_hsm_active_state");
		HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_active_start_state, 					&dab_tuner_ctrl_inst_hsm_active_state,      	DAB_TUNER_CTRL_INST_HSM_ActiveStartHndlr, 						"dab_tuner_ctrl_inst_hsm_active_start_state");
    	HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_backgrndScan_state, 					&dab_tuner_ctrl_inst_hsm_active_state,      	DAB_TUNER_CTRL_INST_HSM_BackgroundScanHndlr, 					"dab_tuner_ctrl_inst_hsm_backgrndScan_state");
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state, 		&dab_tuner_ctrl_inst_hsm_backgrndScan_state,    DAB_TUNER_CTRL_INST_HSM_backgrndEnsembleScanStateHndlr, 		"dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state");
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_backgrndScan_GetcomponentListState,&dab_tuner_ctrl_inst_hsm_backgrndScan_state,    DAB_TUNER_CTRL_INST_HSM_BackgrndScanStateGetcomponentListHndlr, "dab_tuner_ctrl_inst_hsm_backgrndScan_GetcomponentListState");
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_FM_DAB_Linking_state, 				&dab_tuner_ctrl_inst_hsm_backgrndScan_state,    DAB_TUNER_CTRL_INST_HSM_FM_DAB_LinkingHndlr, 					"dab_tuner_ctrl_inst_hsm_FM_DAB_linking_state");
		HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_err_state, 							&dab_tuner_ctrl_inst_hsm_active_state,      	DAB_TUNER_CTRL_INST_HSM_ErrHndlr, 								"dab_tuner_ctrl_inst_hsm_err_state");
  		HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_idle_state,							&dab_tuner_ctrl_inst_hsm_active_state,     		DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr, 						"dab_tuner_ctrl_inst_hsm_idle_state");
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_Nosignallisten_state, 				&dab_tuner_ctrl_inst_hsm_idle_state,        	DAB_TUNER_CTRL_INST_HSM_NosignalListenHndlr, 					"dab_tuner_ctrl_inst_hsm_Nosignallisten_state");    	
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_listen_state, 						&dab_tuner_ctrl_inst_hsm_idle_state,        	DAB_TUNER_CTRL_INST_HSM_ListenHndlr, 							"dab_tuner_ctrl_inst_hsm_listen_state");    	
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_Autoseek_state,					&dab_tuner_ctrl_inst_hsm_idle_state,			DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr,							"dab_tuner_ctrl_inst_hsm_AutoSeek_state");
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_Linking_state, 					&dab_tuner_ctrl_inst_hsm_listen_state, 			DAB_TUNER_CTRL_INST_HSM_LinkingHndlr, 							"dab_tuner_ctrl_inst_hsm_Linking_state"); 	
		    	HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state, 		&dab_tuner_ctrl_inst_hsm_Linking_state, 		DAB_TUNER_CTRL_INST_HSM_DAB_FM_blendingHndlr, 					"dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state");
				HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state, 		&dab_tuner_ctrl_inst_hsm_Linking_state, 		DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr, 					"dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state");
				HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state, 		&dab_tuner_ctrl_inst_hsm_Linking_state,      	DAB_TUNER_CTRL_INST_HSM_DAB_DAB_blendingHndlr, 					"dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state");
			    HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_announcement_state, 			&dab_tuner_ctrl_inst_hsm_listen_state,        	DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr, 						"dab_tuner_ctrl_inst_hsm_announcement_state");  
		HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_active_busy_state, 					&dab_tuner_ctrl_inst_hsm_active_state,     		DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr, 						"dab_tuner_ctrl_inst_hsm_active_busy_state");
 	 		HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_active_busy_scan_state, 			&dab_tuner_ctrl_inst_hsm_active_busy_state, 	DAB_TUNER_CTRL_INST_HSM_ScanStateHndlr,  						"dab_tuner_ctrl_inst_hsm_active_busy_scan_state");
    		HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_ScanGetComponentList_state,		&dab_tuner_ctrl_inst_hsm_active_busy_state, 	DAB_TUNER_CTRL_INST_HSM_ScanStateGetcomponentListHndlr, 		"dab_tuner_ctrl_inst_hsm_ScanGetComponentList_state");		
		 	HSM_CREATE_STATE(dab_tuner_ctrl_hsm_selectserv_and_component,				&dab_tuner_ctrl_inst_hsm_active_busy_state, 	DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr,					"dab_tuner_ctrl_hsm_selectserv_and_component");
				HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_ensemble_tune_state, 				&dab_tuner_ctrl_hsm_selectserv_and_component,   DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr, 						"dab_tuner_ctrl_inst_hsm_ensemble_tune_state");		
			  	HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_GetCurrEnsembleProgmList_state,	&dab_tuner_ctrl_hsm_selectserv_and_component,	DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr, 			"dab_tuner_ctrl_inst_hsm_GetCurrEnsembleProgmList_state");
				HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_GetCurrEnsembleCompList_state, 	&dab_tuner_ctrl_hsm_selectserv_and_component,   DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleCompListHndlr, 			"dab_tuner_ctrl_inst_hsm_GetCurrEnsembleCompList_state");		
				HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_SelectAudio_state,					&dab_tuner_ctrl_hsm_selectserv_and_component,	DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr,						"dab_tuner_ctrl_inst_hsm_SelectAudio_state");
                HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_selectserv_cmd, 					&dab_tuner_ctrl_hsm_selectserv_and_component,   DAB_TUNER_CTRL_INST_HSM_SelServHndlr, 							"dab_tuner_ctrl_inst_hsm_selectserv_cmd");
                HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_select_component_cmd, 				&dab_tuner_ctrl_hsm_selectserv_and_component,   DAB_TUNER_CTRL_INST_HSM_SelCompHndlr, 							"dab_tuner_ctrl_inst_hsm_select_component_cmd");
			HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_AllBand_tune_state, 				&dab_tuner_ctrl_inst_hsm_active_busy_state,   	DAB_TUNER_CTRL_INST_HSM_ESD_AllBandTuneHndlr, 					"dab_tuner_ctrl_inst_hsm_AllBand_tune_state");		
		HSM_CREATE_STATE(dab_tuner_ctrl_inst_hsm_stop_state, 							&dab_tuner_ctrl_inst_hsm_active_state,      	DAB_TUNER_CTRL_INST_HSM_StopHndlr, 								"dab_tuner_ctrl_inst_hsm_stop_state");
 /*======================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_TopHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg) */
/*======================================================================================================================*/

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_TopHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{

	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);
	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_TopHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_TopHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_TopHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_TopHndlr"));
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;

		default:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC]Top Handler MSG: %d", msg->msg_id);
			/* in top state throw system error */
			ret = msg; 
		}
		break;

	}
	return ret;
}

/*===========================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_InactiveHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)  */
/*===========================================================================================================================*/
	
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_InactiveHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu8  u8_NVM_ret = 0;
	PRINT_MSG_DATA(msg);
	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_InactiveHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_TopHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_InactiveHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_InactiveHndlr"));
		
		}
		break;

		case DAB_TUNER_CTRL_INST_HSM_ACTIVATE_DEACTIVATE_REQ:
		{
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_active_start_state);
		}
		break;
		case DAB_TUNER_CTRL_INST_HSM_STARTUP:
		{
			if(DAB_Tuner_Ctrl_me->u8_StartType == 0XAA)
			{
				u8_NVM_ret = SYS_NVM_READ(NVM_ID_DAB_TC_LINKING, &(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo), sizeof(Ts_CurrentSidLinkingInfo), &DAB_Tuner_Ctrl_me->nvm_read );
				u8_NVM_ret = SYS_NVM_READ(NVM_ID_DAB_TC_LEARN_MEMORY, ast_LearnMem, (sizeof(Ts_Tuner_Ctrl_Tunableinfo)*DAB_APP_MAX_COMPONENTS), &DAB_Tuner_Ctrl_me->nvm_read );
				/* NVM read Failure */
				if(u8_NVM_ret == 1)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] NVM Read failed");
				}
				else
				{
					/* for MISRA*/
				}
			}
			else
			{
				/*Do nothing*/
			}
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_active_start_state);
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;

		default:
		{
			/* In top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;

}

/*==========================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*==========================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu8 u8_NVM_ret = 1;
	Tbool b_resultcode;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ActiveHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_ActiveHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ActiveHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ActiveHndlr"));
		}
		break;

		case DAB_TUNER_CTRL_INST_SHUTDOWN:
		{ 
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_stop_state);
		}
		break;

		/*	case DAB_TUNER_CTRL_SYNCHRONISATION_REPLY:
		{
			SetSynchronisationNotifier_reply(&(DAB_Tuner_Ctrl_me->st_synchReply),msg->data);
		}	
		break;

		case DAB_TUNER_CTRL_SYNCHRONISATION_NOTIFICATION:
		{
			DabTuner_MsgRcvSetSynchronisationNotification(&(DAB_Tuner_Ctrl_me->st_synchNotification),msg->data);
		}
		break;		 
		*/
		
		case DAB_TUNER_CTRL_DAB_FACTORY_RESET_REQID:
		{
			DAB_Tuner_Ctrl_me->e_DABTUNERRestartCmd = DAB_TUNER_CTRL_FCATORY_RESET_REQ_VALID;
			memset(&(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo),0,sizeof(Ts_CurrentSidLinkingInfo));
			memset(ast_LearnMem,0,(sizeof(Ts_Tuner_Ctrl_Tunableinfo)*DAB_APP_MAX_COMPONENTS));
			u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LINKING, &(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo), sizeof(Ts_CurrentSidLinkingInfo),&DAB_Tuner_Ctrl_me->nvm_write );
			u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LEARN_MEMORY, ast_LearnMem, (sizeof(Ts_Tuner_Ctrl_Tunableinfo)*DAB_APP_MAX_COMPONENTS) ,&DAB_Tuner_Ctrl_me->nvm_write );
			if(u8_NVM_ret ==0)
			{
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_stop_state);
			}
			else
			{
				DAB_Tuner_Ctrl_Response_Factory_Reset_Settings(REPLYSTATUS_FAILURE);
			}
		}
		break;
		
		case DAB_TUNER_CTRL_DAB_AF_SETTINGS_REQID:
		{
			DAB_Tuner_Ctrl_me->Index =0;  
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_DAB_AF_Settings),&(DAB_Tuner_Ctrl_me->Index));
			DAB_Tuner_Ctrl_Response_DAB_AF_Settings(REPLYSTATUS_SUCCESS);
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings == DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
			{
				if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
				{
					DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
					if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;	
					else
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_DISABLED;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state);
				}
				else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
				{
					DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
					if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;	
					else
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_DISABLED;
					DabTuner_StartBlending_for_implicit(0x00);
					if(st_TimerId.u32_StartBlending_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
						}
						else
						{
							st_TimerId.u32_StartBlending_Timer = 0;
						}							
					}
					else
					{
						/* MISRA C*/	
					}
					st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE && DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == TRUE)
				{
					DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}
				else
				{
					
				}						
			}
			else
			{
				DAB_Tuner_Ctrl_Service_Following_OPerations(DAB_Tuner_Ctrl_me);
			}
			
		}
		break;

		case DAB_TUNER_CTRL_DAB_FM_LINKING_ENABLE_REQID:
		{
			DAB_Tuner_Ctrl_me->Index =0;  
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_Linking_Switch_Status),(msg->data),sizeof(Te_DAB_Tuner_DABFMLinking_Switch),&(DAB_Tuner_Ctrl_me->Index));
			if(DAB_Tuner_Ctrl_me->e_Linking_Switch_Status==DAB_TUNER_CTRL_DABFMLINKING_ENABLE)
			{
				DAB_Tuner_Ctrl_me->u8_SettingStatus = (Tu8)(LIB_SETBIT(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1));
				DAB_Tuner_Ctrl_Response_EnableDABtoFMLinking(REPLYSTATUS_SUCCESS);
#if 0
				if((DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable == TRUE))
				{
					memcpy((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks),(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI),sizeof(DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks));
					if(DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)   /*If FM is NORMAL onlywe will send Hardlinks to FM.*/
					{
							DAB_Tuner_Ctrl_Notify_PICodeList((DAB_Tuner_Ctrl_me->st_PI_data_available),DAB_FM_LINKING_MIN_THRESHOULD,DAB_FM_LINKING_MAX_THRESHOULD,DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId,CHECK_HARDLINK);	
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.b_Hardlinks_sent = TRUE;
							DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
					}
					
			     }
#endif
			}
			else if(DAB_Tuner_Ctrl_me->e_Linking_Switch_Status==DAB_TUNER_CTRL_DABFMLINKING_DISABLE)
			{
				DAB_Tuner_Ctrl_me->u8_SettingStatus = (Tu8)(LIB_CLEARBIT(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1));
				DAB_Tuner_Ctrl_Response_EnableDABtoFMLinking(REPLYSTATUS_SUCCESS);
				if((Tu8)(LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)) == 0)				/* Implememted for DAB_FM Linking settings OFF */
				{
					if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
					{
						memset(&(DAB_Tuner_Ctrl_me->st_Blending_info),0,sizeof(Ts_Blending_info));
						DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
						if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK ;
						else
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_DISABLED;
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state);
					}
					else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
					{
						memset(&(DAB_Tuner_Ctrl_me->st_Blending_info),0,sizeof(Ts_Blending_info));
						DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
						if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK ;
						else
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_DISABLED;
						DabTuner_StartBlending_for_implicit(0x00);
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;
							}							
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
					}						
					else
					{
					}
					
				}																										/* Implememted for DAB_FM Linking settings OFF */
			}
			else
			{
			}
		}
		break;

	case DAB_TUNER_CTRL_ANNO_CONFIG_REQID:
		{	
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u16_AnnoConfig),(msg->data),sizeof(Tu16),&(DAB_Tuner_Ctrl_me->Index));
			
			u16_usermask = DAB_Tuner_Ctrl_me->u16_AnnoConfig;
			
			DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
			DAB_Tuner_Ctrl_Response_SetAnnoConfig(DAB_Tuner_Ctrl_me->e_ReplyStatus);
			if(DAB_Tuner_Ctrl_me->u16_AnnoConfig == 0)
			{
				/* Clear all announcement related database */	
				DAB_Tuner_Ctrl_ClearingAnnoDatabases(DAB_Tuner_Ctrl_me);
			}
			else
			{
				/* For MISRA */
			}
			
		}	
		 break;
		 
		case DAB_TUNER_CTRL_AMFMTUNER_STATUS_NOTIFYID :
		{
		 	DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus),(msg->data),sizeof(Te_RADIO_Comp_Status),&(DAB_Tuner_Ctrl_me->Index));
			if(DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_ABNORMAL)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] AMFM TUNER is in Abnormal state ");
				/* DAB-FM linking things need to be taken care of AMFM Tuner goes into ABNORMAL state	*/
				if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
				{
					//memset(&(DAB_Tuner_Ctrl_me->st_Blending_info),0,sizeof(Ts_Blending_info));
					DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
					if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;	
					else
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state);
				}
				else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
				{
					//memset(&(DAB_Tuner_Ctrl_me->st_Blending_info),0,sizeof(Ts_Blending_info));
					DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
					if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;	
					else
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					DabTuner_StartBlending_for_implicit(0x00);
					if(st_TimerId.u32_StartBlending_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
						}
						else
						{
							st_TimerId.u32_StartBlending_Timer = 0;
						}							
					}
					else
					{
						/* MISRA C*/	
					}
					st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				else
				{
				}						
				
			}
			else
			{
				/* DAB-FM linking things need to be taken care of AMFM TUNER comes back to NORMAL state */	
			}
 
		}
		break;
		case DAB_TUNER_CTRL_SELSERV_REQID:
		case DAB_TUNER_CTRL_SCAN_REQID:
		case DAB_TUNER_CTRL_DESELBAND_REQID:
		{
			if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_ESD_CHECK)
			{
				DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
				SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->Blending_Process),(msg),sizeof(Ts_Sys_Msg));
				(void)SYS_SEND_MSG(&(DAB_Tuner_Ctrl_me->Blending_Process));
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					
			}
		}
		break;
		case DAB_TUNER_CTRL_ENG_MODE_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_Eng_Mode_Request),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_Eng_Mode_Request),&(DAB_Tuner_Ctrl_me->Index));
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_Eng_Mode_Request == DAB_TUNER_CTRL_MODE_ON)
			{
				DAB_Tuner_Ctrl_me->u8_ENGMODEStatus = DAB_TUNER_CTRL_MODE_ON;
				//DAB_Tuner_Ctrl_Response_AFList(DAB_Tuner_Ctrl_me->st_AFList);
			}
			else if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_Eng_Mode_Request == DAB_TUNER_CTRL_MODE_OFF)
			{
				DAB_Tuner_Ctrl_me->u8_ENGMODEStatus = DAB_TUNER_CTRL_MODE_OFF;
			}
			else
			{
				/*MISRA*/
			}
		}
		break;
		
		case DAB_TUNER_CTRL_STRATERGY_STATUS_NOTIFYID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus),&(DAB_Tuner_Ctrl_me->Index));
		}
		break;

		case SYSTEM_MONITOR_NOTIFICATION_TIMEOUT :
		{
			if (DAB_Tuner_Ctrl_me->e_SystemMonitoringNotReceived == DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_RECEIVED)
			{
				DAB_Tuner_Ctrl_me->e_SystemMonitoringNotReceived = DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_TIMEOUT ;
				if(SYS_TIMER_START(SYSTEM_MONITOR_NOTIFICATION_TIME, SYSTEM_MONITOR_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL) <= 0)
				{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SYSTEM_MONITOR_NOTIFICATION_TIMEOUT");
				}
				else
				{
					/*MISRA c*/
				}
			}
			else if(DAB_Tuner_Ctrl_me->e_SystemMonitoringNotReceived == DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_TIMEOUT)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state");
				DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
				/* Send Notification to DAB App that DABTuner is Abnormal*/
				DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
				/* Transit to Inactive state*/
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
			}
		}
		break ;
		
		case DAB_TUNER_CTRL_AUDIO_ERROR_CONCEALMENT_REPLY:
		{
			b_resultcode = DabTuner_MsgRcvGetAudioErrorConcealment2SettingsReply(&(DAB_Tuner_Ctrl_me->st_DabTunerMsg_GetAudioErrorConcealment2SettingsReply),msg->data);
			if(b_resultcode == TRUE)
			{
				
			}
		}
		break;
		
		case DAB_TUNER_CTRL_SET_AUDIO_ERROR_CONCEALMENT_REPLY:
		{
			b_resultcode = DabTuner_MsgReplySetAudioErrorConcealment2_repl(&(DAB_Tuner_Ctrl_me->st_DabTunerMsg_R_AudioErrorConcealment2_repl),msg->data);
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_DabTunerMsg_R_AudioErrorConcealment2_repl.Reply == 0)
				{
			 		DabTuner_MsgSndGetAudioErrorConcealment2Settings_Cmd();
				}
			}
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;

		  
		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}



/*==============================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveStartHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)  */
/*==============================================================================================================================*/
	
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveStartHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Te_Frontend_type e_Frontend_type;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_FAILURE;
			DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = FALSE;
			DAB_Tuner_Ctrl_me->e_SystemMonitoringNotReceived = DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_INVALID ;
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ActiveStartHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ActiveStartHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ActiveStartHndlr"));
			e_Frontend_type = FOREGROUND_CHANNEL;
			etal_ret = DabTunerCtrl_Config_Receiver_cmd(e_Frontend_type);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				e_Frontend_type = BACKGROUND_CHANNEL;
				etal_ret = DabTunerCtrl_Config_Receiver_cmd(e_Frontend_type);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL ,DAB_TUNER_CTRL_RECEIVER_CONFIG_DONE_RESID);
				}
				else 
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Error in Receiver_Configuration:ERROR_NO - %d \n",etal_ret);
				}
			}
			else 
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] Error in Receiver_Configuration:ERROR_NO - %d \n",etal_ret);
			}
		}
		break;

		case DAB_TUNER_CTRL_RECEIVER_CONFIG_DONE_RESID:
		{
		    e_Frontend_type = FOREGROUND_CHANNEL;			
			etal_ret = DabTunerCtrl_Config_Datapath_cmd(e_Frontend_type);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				/* Temporarily disable FIC to avoid too many notifications */
				//etal_ret = DabTunerCtrl_Config_FICData_cmd(e_Frontend_type);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					e_Frontend_type = BACKGROUND_CHANNEL;
					etal_ret = DabTunerCtrl_Config_Datapath_cmd(e_Frontend_type);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_DATAPATH_CONFIG_DONE_RESID);
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] Error in Datapath_Configuration:ERROR_NO - %d \n", etal_ret);
					}
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] Error in FICData_Configuration:ERROR_NO - %d \n", etal_ret);
				}
			}
			else 
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] Error in Datapath_Configuration:ERROR_NO - %d \n",etal_ret);
			}
		
		}
		break;

		case DAB_TUNER_CTRL_DATAPATH_CONFIG_DONE_RESID:
		{
			etal_ret = DabTunerCtrl_Audio_Source_Select_cmd();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_AUDIO_SOURCE_SELECT_DONE_RESID);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] Error in Audio_Source_Selection:ERROR_NO - %d \n", etal_ret);
			}
		}
		break;

		case DAB_TUNER_CTRL_AUDIO_SOURCE_SELECT_DONE_RESID:
		{
			etal_ret = DabTunerCtrl_Enable_Data_Service_cmd();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_ENABLE_DATASERVICE_DONE_RESID);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] Error in Enable_Data_Service:ERROR_NO - %d \n", etal_ret);
			}
		}
		break;

		case DAB_TUNER_CTRL_ENABLE_DATASERVICE_DONE_RESID:
		{
			e_Frontend_type = FOREGROUND_CHANNEL;
			etal_ret = DabTuner_Config_FGQualityMonitor(e_Frontend_type);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_ENABLE_QUALITY_MONITOR_DONE_RESID);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] Error in ForeGROUND ENABLE_QUALITY_MONITOR:ERROR_NO - %d \n", etal_ret);
			}
		}
		break;

		case DAB_TUNER_CTRL_ENABLE_QUALITY_MONITOR_DONE_RESID:
		{
			if (DAB_Tuner_Ctrl_me->e_DABTUNERRestartCmd == DAB_TUNER_CTRL_DABTUNER_RESTART_REQ_VALID)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] SATRUN Reset successful, DAB Tuner is in Normal state)");
				DAB_Tuner_Ctrl_hsm_inst_start_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_INST_HSM_STARTUP_DONE);

				if (DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State == DAB_TUNER_CTRL_FG_STATE)
				{
					b_FIC_Data_Enable = FALSE;
					DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_INVALID_STATE;
					/* Transit to Idle state */
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else if (DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State == DAB_TUNER_CTRL_BG_STATE)
				{
					DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_INVALID_STATE;
					/* Transit to Background state */
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndScan_state);
				}
				else
				{
					/* FOR MISRA */
				}

			}
			else
			{
				if (DAB_Tuner_Ctrl_me->e_DABTUNERRestartCmd == DAB_TUNER_CTRL_FCATORY_RESET_REQ_VALID)
				{
					if (DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State == DAB_TUNER_CTRL_FG_STATE)
					{
						DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_INVALID_STATE;
						/* Transit to Idle state */
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					}
					else if (DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State == DAB_TUNER_CTRL_BG_STATE)
					{
						DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_INVALID_STATE;
						/* Transit to Background state */
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndScan_state);
					}
					else
					{
						/* FOR MISRA */
					}
				}
				DAB_Tuner_Ctrl_hsm_inst_start_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_INST_HSM_STARTUP_DONE);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndScan_state);
			}
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;

		default:
		{			
			/* in top state throw system error */
			ret = msg;
		}   
		break;
	}
	return ret;
}


/*===========================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)    */
/*===========================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ErrorHandlingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ErrorHandlingHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ErrorHandlingHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ErrorHandlingHndlr"));
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}




	

/*=============================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ScanStateHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*=============================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ScanStateHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu8 labelIndex =0;
	Tu16  serviceIndex;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ScanStateHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_BackgrndScanState";
			SYS_RADIO_MEMCPY((void*) DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ScanStateHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ScanStateHndlr"));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Scan started ");
			etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;
				/* Stop the timer if it's already running */
				if (st_TimerId.u32_AutoseekNotify_Timer > 0)
				{
					if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
					}
					else
					{
						st_TimerId.u32_AutoseekNotify_Timer = 0;
					}
				}
				else
				{
					/* MISRA C*/
				}
				st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if (st_TimerId.u32_AutoseekNotify_Timer == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
				}
				else
				{
					/*MISRA C*/
				}
			}
			else 
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
			}
		}
		break;

		case SCAN_NOTIFY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SCAN_NOTIFY_TIMEOUT - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_IN_PROGRESS:
				{
					// Retry the commands for 2 times 
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SCAN_NOTIFY_CMD_MAX_REPEAT_COUNT)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] StartScan2_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
						//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;
		
		case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT:
		{
			st_TimerId.u32_AutoseekNotify_Timer = 0;
			// Retry the commands for 2 times 
			if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SCAN_NOTIFY_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] StartScan2_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
				DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
				DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
				Update_Stationlist_Into_LearnMem();
				DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
				//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION:
		{
			if (st_TimerId.u32_AutoseekNotify_Timer > 0)
			{
				if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
				}
				else
				{
					st_TimerId.u32_AutoseekNotify_Timer = 0;
				}
			}
			else
			{
				/* MISRA C*/
			}
			DAB_Tuner_Ctrl_me->Index = 0;
			/*clearing the seekstatus structure before getting the event*/
			memset(&(DAB_Tuner_Ctrl_me->st_SeekStatus), 0, sizeof(EtalSeekStatus));
			// extracting the seek status data and copying into st_SeekStatus structure 
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_SeekStatus), (msg->data), sizeof(DAB_Tuner_Ctrl_me->st_SeekStatus), &(DAB_Tuner_Ctrl_me->Index));
			
			if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_status == ETAL_SEEK_RESULT)
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData), 0, sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
				/* copy every step frequency to notify*/
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency;

				/* Notifying the frequency to be tuned by the autoseek FOR EVERY STEP SIZE FREQ, to DAB application */
				DAB_Tuner_Ctrl_Notify_FrequencyChange(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);

				if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequencyFound == TRUE) /* if frequency is detected */
				{
					/* copying the dab qualiy parametrs in local structure */
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.RFFieldStrength			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_RFFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.BBFieldStrength		    =  DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_BBFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.FicBitErrorRatio			=  DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_FicBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.isValidMscBitErrorRatio	=  DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_isValidMscBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.MscBitErrorRatio			=  DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_MscBitErrorRatio;

					etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
					}
				}
			}
			else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_status == ETAL_SEEK_FINISHED)
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* if full cycle is reached but freq not found then Abort_scancmd(autoseek_stop) is called to stop
				autoseek properly then  ETAL_SEEK_FINISHED is triggered and handled here */
				if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE && DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequencyFound == FALSE)
				{
					DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData), 0, sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					/* copy every step frequency to notify*/
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency;

					/* Notifying the frequency to be tuned by the autoseek FOR EVERY STEP SIZE FREQ, to DAB application */
					DAB_Tuner_Ctrl_Notify_FrequencyChange(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Scan ended sucessfully ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
					Update_Stationlist_Into_LearnMem();
					DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
					//DabTuner_SetSynchronisationNotifier_cmd(0x01);
					//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
					/*if cancel request is SCAN CANCEL */
				else if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_SCAN_CANCEL || DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_AF_TUNE_CANCEL)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Abort Scan sucess ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
					DAB_Tuner_Ctrl_Response_Abort_Scan();
					DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else
				{
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_CANCEL_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_CancelType),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_CancelType),&(DAB_Tuner_Ctrl_me->Index));
			memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			if (DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_SCAN_CANCEL || DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_AF_TUNE_CANCEL)
			{
				/* Send Abort scan request to Soc */
				etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN_TIMEOUT);
				}
			}
			else
			{
				/* For MISRA */
			}			
		}
		break; 
		
		case ABORT_SCAN_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ABORT_SCAN_TIMEOUT - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					/*if cancel request is SCAN CANCEL */
					if (DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_SCAN_CANCEL || DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_AF_TUNE_CANCEL)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Abort Scan sucess ");
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
						DAB_Tuner_Ctrl_Response_Abort_Scan();
						DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					}
					else  /* Event timeout for Autoseek stop called when full cycle is reached   */
					{
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Scan ended sucessfully ");
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
					}
#if 0
					// Retry the commands for 2 times 
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_ABORT_SCAN_CMD_MAX_REPEAT_COUNT)
					{
						etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN2_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] AbortScan2_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						/* Added Scan response with negative status, so that Radio Manager can follow strategy */
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						/* Clearing station list related information */
						ensembleIndex = 0;
						Services = 0;
						Components = 0;
						memset(ast_EnsembleInfo, 0, sizeof(Ts_Tuner_Ctrl_EnsembleInfo)* DAB_APP_MAX_ENSEMBLES);
						memset(ast_ServiceInfo, 0, sizeof(Ts_Tuner_Ctrl_ServiceInfo)* DAB_APP_MAX_SERVICES);
						memset(ast_ComponentInfo, 0, sizeof(Ts_Tuner_Ctrl_ComponentInfo)* DAB_APP_MAX_COMPONENTS);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					}
#endif
				}
				break;

				default:
				{
					/* To avoid Warnings */
				}
				break;


			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT:
		{
			st_TimerId.u32_AutoseekNotify_Timer = 0;
			// Retry the commands for 2 times for to get the Autoseek notification purpose
			if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SCAN_NOTIFY_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}

				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/*if cancel request is SCAN CANCEL */
				if (DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_SCAN_CANCEL || DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_AF_TUNE_CANCEL)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Abort Scan sucess ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
					DAB_Tuner_Ctrl_Response_Abort_Scan();
					DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else  /* Event timeout for Autoseek stop called when full cycle is reached   */
				{
					DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Scan ended sucessfully ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
					Update_Stationlist_Into_LearnMem();
					DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Command to get the ensemble properties */
			etal_ret = DabTuner_MsgSndGetEnsembleProperties();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
			}

		}
		break;


		case GET_ENSEMBLE_PROPERTIES_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Extracting the msg and updating st_ScanEnsembleinfo */
			DabTuner_MsgReplyGetEnsembleProperties(&(st_ScanEnsembleinfo));
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			if (ensembleIndex < DAB_APP_MAX_ENSEMBLES)
			{
				/* Updating Ensemble list structure and current Ensemble list structure to display the HMI for few seconds */
				ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u32_Frequency			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency;
				ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u16_EId					= st_ScanEnsembleinfo.EnsembleIdentifier;
				ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.RSSI					= DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.RFFieldStrength;
				ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u8_ECC					= st_ScanEnsembleinfo.ECC;
				ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u16_ShortLabelFlags	= st_ScanEnsembleinfo.ShortLabelFlags;
				if (st_ScanEnsembleinfo.CharSet == 0x00)
				{
					ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet			= DAB_TUNER_CTRL_CHARSET_EBU;
				}
				else if (st_ScanEnsembleinfo.CharSet == 0x06)
				{
					ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet			= DAB_TUNER_CTRL_CHARSET_UCS2;
				}
				else if (st_ScanEnsembleinfo.CharSet == 0x0f)
				{
					ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet			= DAB_TUNER_CTRL_CHARSET_UTF8;
				}
				else
				{
					/*Doing nothing */
				}/* For MISRA C */
				for (labelIndex = 0; labelIndex < ETAL_DEF_MAX_LABEL_LEN; labelIndex++)
				{
					ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.au8_label[labelIndex] = st_ScanEnsembleinfo.LabelString[labelIndex];
				}

				/*Command to get service list*/
				etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
				}
				SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
			}
		}
		break;


		case GET_PROGRAM_SERVLIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(&(st_ScanServiceinfo));
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			DAB_Tuner_Ctrl_me->serviceindx = Services;
			DAB_Tuner_Ctrl_me->currentserviceindx = 0;
			/* Updating Service List id's in stationlist Structure */
			for (serviceIndex = 0; serviceIndex < st_ScanServiceinfo.u8_NumOfServices; serviceIndex++, Services++)
			{
				if (Services < DAB_APP_MAX_SERVICES)
				{
					SYS_RADIO_MEMCPY(&(ast_ServiceInfo[Services].st_BasicEnsInfo), &(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo), sizeof(ast_ServiceInfo[Services].st_BasicEnsInfo));
					ast_ServiceInfo[Services].u32_SId = st_ScanServiceinfo.st_serviceinfo[serviceIndex].ProgServiceId;
				}
			}
			if (st_ScanServiceinfo.u8_NumOfServices > 0)
			{
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_ScanGetComponentList_state);
			}
			else if (st_ScanServiceinfo.u8_NumOfServices == 0)/* if there is no data or signal availabe in detected freq */
			{
				SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
				if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
						|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
				{
					DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
					Update_Stationlist_Into_LearnMem();
					DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
				}
				else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
				{
					etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						/* Stop the timer if it's already running */
						if (st_TimerId.u32_AutoseekNotify_Timer > 0)
						{
							if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
							}
							else
							{
								st_TimerId.u32_AutoseekNotify_Timer = 0;
							}
						}
						else
						{
							/* MISRA C*/
						}
						st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
						if (st_TimerId.u32_AutoseekNotify_Timer == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
						}
						else
						{
							/*MISRA C*/
						}
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
					}
				}
			}
			else
			{
				/* do nothing */
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB GET CURR ENSEMBLE FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					//Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the uEID */
						etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB ENSEMBLE FAILED  ");
						if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
								|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
						{
							DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
						}
						else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
						{
							etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								/* Stop the timer if it's already running */
								if (st_TimerId.u32_AutoseekNotify_Timer > 0)
								{
									if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
									}
									else
									{
										st_TimerId.u32_AutoseekNotify_Timer = 0;
									}
								}
								else
								{
									/* MISRA C*/
								}
								st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
								if (st_TimerId.u32_AutoseekNotify_Timer == 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
								}
								else
								{
									/*MISRA C*/
								}
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
							}
						}
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

		case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ETAL DAB ENSEMBLE PROPERTIES FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					//Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the ensemble properties */
						etal_ret = DabTuner_MsgSndGetEnsembleProperties();
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetEnsembleProperties_Cmd Timeout even after retriggering");
						if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
							|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
						{
							DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
						}
						else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
						{
							etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								/* Stop the timer if it's already running */
								if (st_TimerId.u32_AutoseekNotify_Timer > 0)
								{
									if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
									}
									else
									{
										st_TimerId.u32_AutoseekNotify_Timer = 0;
									}
								}
								else
								{
									/* MISRA C*/
								}
								/* Start the timer */
								st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
								if (st_TimerId.u32_AutoseekNotify_Timer == 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
								}
								else
								{
									/*MISRA C*/
								}
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
							}
						}
					}
				}
				break;

				case ETAL_RET_NO_DATA:
				{
					if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
						|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
					{
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
					}
					else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

		case GET_PROGRAMLIST_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB ENSEMBLE SERVICE LIST PROPERTIES FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					// Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_PROGRAMLIST_CMD_MAX_REPEAT_COUNT)
					{
						/*Command to get service list*/
						etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetProgrammeServiceList_Cmd Timeout even after retriggering");
						if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
							|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
						{
							DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
						}
						else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
						{
							etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								/* Stop the timer if it's already running */
								if (st_TimerId.u32_AutoseekNotify_Timer > 0)
								{
									if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
									}
									else
									{
										st_TimerId.u32_AutoseekNotify_Timer = 0;
									}
								}
								else
								{
									/* MISRA C*/
								}
								/* Start the timer */
								st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
								if (st_TimerId.u32_AutoseekNotify_Timer == 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
								}
								else
								{
									/*MISRA C*/
								}
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
							}
						}
						
					}
				}
				break;

				case ETAL_RET_NO_DATA:
				{
					if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
											 || DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
					{
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
					}
					else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
					}
				}
				break;

				default:
				{

				}
				break;

			}
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;
		
		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}


/*=============================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ScanStateGetcomponentListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*=============================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ScanStateGetcomponentListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Ts_Sys_Msg* msg) 
{
	Ts_Sys_Msg* ret = NULL; // mark the message as handled 
	Tbool b_resultcode = FALSE;
	Tu16  serviceIndex = 0;
	Tu16 labelIndex = 0;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
		
		case HSM_MSGID_START:
		{	 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ScanStateGetcomponentListHndlr ");
			SYS_RADIO_MEMCPY((void*) DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ScanStateGetcomponentListHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ScanStateGetcomponentListHndlr"));
			etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
			if(etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
			}
			else 
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL,GET_COMPONENTLIST_TIMEOUT);
			}
		}
		break;

		case GET_COMPONENTLIST_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GET_COMPONENTLIST_TIMEOUT - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_NO_DATA:
				{
					 // Retry the commands for 2 times 
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT)
					{
						etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] GetComponentListReq_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						/* No of services of current ensemble is more than one and current service is not the last service*/
						if ((st_ScanServiceinfo.u8_NumOfServices != ((DAB_Tuner_Ctrl_me->currentserviceindx) + 1))
							&& (st_ScanServiceinfo.u8_NumOfServices > 1))
						{
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx], &ast_ServiceInfo[(DAB_Tuner_Ctrl_me->serviceindx) + 1], sizeof(Ts_Tuner_Ctrl_ServiceInfo)* (DAB_APP_MAX_SERVICES - ((DAB_Tuner_Ctrl_me->serviceindx) + 1)));
							/* Decrement total no of services count*/
							Services--;
							DAB_Tuner_Ctrl_me->currentserviceindx++;
							/* Re-trigger Get component list for remaining services */
							etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
							}
						}
						/* No of services of current ensemble is more than one and current service is the last service*/
						else if (((st_ScanServiceinfo.u8_NumOfServices == (DAB_Tuner_Ctrl_me->currentserviceindx) + 1))
							&& (st_ScanServiceinfo.u8_NumOfServices > 1))
						{
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx], &ast_ServiceInfo[(DAB_Tuner_Ctrl_me->serviceindx) + 1], sizeof(Ts_Tuner_Ctrl_ServiceInfo));
							/* Decrement total no of services count*/
							Services--;
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							/* Check to find if all ensemble service component list is fetched or not*/
							if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
								|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
							{
								ensembleIndex++;
								DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
								DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
								DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
								DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
								Update_Stationlist_Into_LearnMem();
								DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
							}
							else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								ensembleIndex++;
								HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_active_busy_scan_state);
							}	
						}
						else if (st_ScanServiceinfo.u8_NumOfServices == 1)
						{
							/* Clear particular ensemble info in ast_EnsembleInfo array */
							memset(&ast_EnsembleInfo[ensembleIndex], 0, sizeof(Ts_Tuner_Ctrl_EnsembleInfo));
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx], &ast_ServiceInfo[(DAB_Tuner_Ctrl_me->serviceindx) + 1], sizeof(Ts_Tuner_Ctrl_ServiceInfo));
							/* Decrement total no of services count*/
							Services--;
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							/* Check to find if all ensemble service component list is fetched or not*/
							
							if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit
								|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
							{
								DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
								DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
								DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
								DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
								Update_Stationlist_Into_LearnMem();
								DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
							}
							else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_active_busy_scan_state);
							}
						}
						else
						{
							/* Send scan timeout response to DAB App */
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
							//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
						}
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

#if 0
		case DAB_TUNER_CTRL_CANCEL_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_CancelType),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_CancelType),&(DAB_Tuner_Ctrl_me->Index));
			if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_SCAN_CANCEL)
			{
				if ((st_ScanServiceinfo.u8_NumOfServices != ((DAB_Tuner_Ctrl_me->serviceindx) + 1)) &&
					(st_ScanServiceinfo.u8_NumOfServices > 1))
				{
					Services = (Tu16)(Services - (st_ScanServiceinfo.u8_NumOfServices - ((DAB_Tuner_Ctrl_me->serviceindx) + 1)));
					memset(&ast_ServiceInfo[(DAB_Tuner_Ctrl_me->serviceindx) + 1], 0, sizeof(Ts_Tuner_Ctrl_ServiceInfo)* (st_ScanServiceinfo.u8_NumOfServices - ((DAB_Tuner_Ctrl_me->serviceindx) + 1)));
				}
				else if (st_ScanServiceinfo.u8_NumOfServices == 1)
				{
					memset(&ast_EnsembleInfo[ensembleIndex],0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo));
					memset(&ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx],0,sizeof(Ts_Tuner_Ctrl_ServiceInfo));
					Services--;	
				}
				else 
				{
					/* For MISRA */
				}
			}
			else
			{
				/* For MISRA */
			}
			DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
			DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
		}
		break;
#endif	

		case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY:
		{	
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Cmd for getting the service information */
			b_resultcode = DabTuner_GetServListpropertiesReply(&(st_ScanServiceinfo.st_serviceinfo[DAB_Tuner_Ctrl_me->serviceindx]));
			if (b_resultcode == TRUE)
			{
				Services = Services - st_ScanServiceinfo.u8_NumOfServices;
				/* Updating Service List Structure */
				for (serviceIndex = 0; serviceIndex< st_ScanServiceinfo.u8_NumOfServices; serviceIndex++, Services++)
				{
					if (Services < DAB_APP_MAX_SERVICES)
					{
						SYS_RADIO_MEMCPY(&(ast_ServiceInfo[Services].st_BasicEnsInfo), &(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo), sizeof(ast_ServiceInfo[Services].st_BasicEnsInfo));
						ast_ServiceInfo[Services].st_ServiceLabel.u16_ShortLabelFlags = st_ScanServiceinfo.st_serviceinfo[serviceIndex].ShortLabelFlags;
						if (st_ScanServiceinfo.st_serviceinfo[serviceIndex].CharSet == 0x00)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if (st_ScanServiceinfo.st_serviceinfo[serviceIndex].CharSet == 0x06)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if (st_ScanServiceinfo.st_serviceinfo[serviceIndex].CharSet == 0x0f)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;
						}
						else
						{
							/*Doing nothing */
						}/* For MISRA C */
						for (labelIndex = 0; labelIndex < ETAL_DEF_MAX_LABEL_LEN; labelIndex++)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.au8_label[labelIndex] = st_ScanServiceinfo.st_serviceinfo[serviceIndex].LabelString[labelIndex];
						}
						
					}
				}
			}
			b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply));
			if(b_resultcode == TRUE)
			{
				for(DAB_Tuner_Ctrl_me->CompIndex=0; DAB_Tuner_Ctrl_me->CompIndex < DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.NoOfComponents;DAB_Tuner_Ctrl_me->CompIndex++,Components++)
				{
					if(Components < DAB_APP_MAX_COMPONENTS)
					{
						SYS_RADIO_MEMCPY(&(ast_ComponentInfo[Components].st_BasicEnsInfo),&(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo),sizeof(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo));
						ast_ComponentInfo[Components].u16_SCIdI        =	  DAB_Tuner_Ctrl_me-> DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].InternalCompId;
						ast_ComponentInfo[Components].u32_SId = ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId;
						SYS_RADIO_MEMCPY((ast_ComponentInfo[Components].st_compLabel.au8_label),(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						ast_ComponentInfo[Components].st_compLabel.u16_ShortLabelFlags   =    DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].ShortLabelFlags;
						if(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x00)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x06)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x0f)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
						}
						else
						{
						/*Doing nothing */ 	
						}/* For MISRA C */
						ast_ComponentInfo[Components].u8_ComponentType = DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].ComponentType;
					}
				}
				DAB_Tuner_Ctrl_me->currentserviceindx++;
				DAB_Tuner_Ctrl_me->serviceindx++;
				if ((DAB_Tuner_Ctrl_me->currentserviceindx != st_ScanServiceinfo.u8_NumOfServices) && (DAB_Tuner_Ctrl_me->serviceindx < DAB_APP_MAX_SERVICES))
				{
					etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
					}
				}
				else
				{
					/* copying the ensemble info in current ensemble info to notify in the HMI */
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency							= ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u32_Frequency;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId								= ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u16_EId;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC								= ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u8_ECC;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u16_ShortLabelFlags	= ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u16_ShortLabelFlags;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet				= ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet;
					
					/* copying the Firstservice info of scanned ensemble in current service info to notify in the HMI */
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId								= ast_ServiceInfo[Services - st_ScanServiceinfo.u8_NumOfServices].u32_SId; // copying first serviceid of each ensemble
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u16_ShortLabelFlags		= ast_ServiceInfo[Services - st_ScanServiceinfo.u8_NumOfServices].st_ServiceLabel.u16_ShortLabelFlags;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet				= ast_ServiceInfo[Services - st_ScanServiceinfo.u8_NumOfServices].st_ServiceLabel.u8_CharSet;
					SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.au8_label), (ast_ServiceInfo[Services - st_ScanServiceinfo.u8_NumOfServices].st_ServiceLabel.au8_label), ETAL_DEF_MAX_LABEL_LEN);
					
					/* copying the Firstservicecomp info of Firstservice of scanned ensemble in current service info to notify in the HMI */
					for (DAB_Tuner_Ctrl_me->CompIndex = 0; DAB_Tuner_Ctrl_me->CompIndex < DAB_APP_MAX_COMPONENTS; DAB_Tuner_Ctrl_me->CompIndex++)
					{
						if (DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == ast_ComponentInfo[DAB_Tuner_Ctrl_me->CompIndex].u32_SId)
						{
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI										= ast_ComponentInfo[DAB_Tuner_Ctrl_me->CompIndex].u16_SCIdI;// copying first servicecomp id of first servid of every ensemble scan
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u16_ShortLabelFlags	= ast_ComponentInfo[DAB_Tuner_Ctrl_me->CompIndex].st_compLabel.u16_ShortLabelFlags;
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet				= ast_ComponentInfo[DAB_Tuner_Ctrl_me->CompIndex].st_compLabel.u8_CharSet;
							SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.au8_label), (ast_ComponentInfo[DAB_Tuner_Ctrl_me->CompIndex].st_compLabel.au8_label), DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						}
					}
					DAB_Tuner_Ctrl_me->requestedinfo.u32_SId	= DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
					DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI	= DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI;
					
					ensembleIndex++;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_SelectAudio_state);
				}
			}
		SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;
		
		default:
		{
			ret = msg;   
		}
		break;
	}
	return ret;
}

/*=============================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg) */
/*=============================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode = FALSE;
	Tu32 index=0;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr ");
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_FG_STATE ;
			if (b_FIC_Data_Enable == FALSE)
			{
				etal_ret = DabTunerCtrl_Enable_FICData_cmd();
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					b_FIC_Data_Enable = TRUE;
				}
				else
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Error in FIC Data Enable:ERROR_NO - %d \n",etal_ret);
				}
			}
			else
			{
				/* Do Nothing*/
			}

			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ActiveIdleHndlr"));
		}
		break;

		case DAB_TUNER_CTRL_CANCEL_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_CancelType),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_CancelType),&(DAB_Tuner_Ctrl_me->Index));
			
			if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_AF_TUNE_CANCEL)
			{
				//memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
			}
			else if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_TUNE_CANCEL)
			{
				DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
			}
			else if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_SCAN_CANCEL)
			{
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
			}
			else
			{
				
			}
			DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
			
			DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);	
		}
		break;

		case DAB_TUNER_CTRL_SELSERV_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Play select request ");
			index=0;
			memset(&(DAB_Tuner_Ctrl_me->requestedinfo),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency),(msg->data),sizeof(Tu32),&index);
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->requestedinfo.u16_EId),(msg->data),sizeof(Tu16),&index);
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->requestedinfo.u32_SId),(msg->data),sizeof(Tu32),&index);
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI),(msg->data),sizeof(Tu16),&index);
						
			//	u8_selectServRequest=1;
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_SEL_SER;
			DAB_Tuner_Ctrl_Disable_Fig_Notifications();
			DAB_Tuner_Ctrl_Disable_Quality_Notifications();
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_hsm_selectserv_and_component);
		}
		break;

		case DAB_TUNER_CTRL_SEEK_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] AutoSeek request ");
			index = 0;
			memset(&(DAB_Tuner_Ctrl_me->requestedinfo), 0, sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
				
			/* Extracting the frequency which is seekstart freq from DAB app used for boundary check */
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u32_AutoSeekStartFrequency), (msg->data), sizeof(Tu32), &index);
				
			/* Extracting the seek direction from DAB app */
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_SeekDirection), (msg->data), sizeof(Te_RADIO_DirectionType), &index);

			/* Extracting the seekstarted status from DAB app for for Updatestopfreq in autoseek_start cmd */
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->b_AutoSeekStart), (msg->data), sizeof(Tbool), &index);
				
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_SEEK;

			DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = TRUE;
			/*Transition to Autoseek state*/
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Autoseek_state);
		}
		break;
		
		case SYSTEM_CONTORL_NOTIFICATION:
		{
			b_resultcode = DabTunerCtrl_SystemMonitoring_not(&(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not),(Ts8*)(msg->data));
			DAB_Tuner_Ctrl_me->e_SystemMonitoringNotReceived = DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_RECEIVED ;
			/* Store the current state of DAB_Tuner_Ctrl */
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_FG_STATE ;
			//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] System monitoring notification MonitorId = %x Status = %x ",DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId, DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status );
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x05)
				{
					/* Out going messages dropped by DABTuner */
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status > 2000) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state");
						/* Store the current state of DAB_Tuner_Ctrl */
					DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				}
	#if 1
				/* Audio-out event notifications */
				else if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x20)
				{
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status == 0x10) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state due to Audio-out event change");
						DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,QUALITY_PARAMETERS);
						/* Store the current state of DAB_Tuner_Ctrl */
						DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				}
	#endif 
	#if 0
				/* system overload event notifications */
				else if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x22)
				{
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status == 0x04 ) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state");
						/* Store the current state of DAB_Tuner_Ctrl */
						DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */					
				}				
	#endif						
				else
				{
					/* Doing nothing */	
				}/* For MISRA C */
			}
			else
			{
				/* FOR MISRA */
			}
		}
		break;
		
		case  DAB_TUNER_CTRL_SCAN_REQID: 
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Scan request ");
			index=0;
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_BANDSCAN;
			ensembleIndex = 0; DAB_Tuner_Ctrl_me->serviceindx = 0;DAB_Tuner_Ctrl_me->CompIndex = 0; Services = 0; Components = 0;
			DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
			DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency;/* Storing the current freq to retune to prevcurr freq after finishing scan */
			DAB_Tuner_Ctrl_Disable_Fig_Notifications();
			memset(&(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_ScanNotification),0,sizeof(Ts_DabTunerMsg_R_ScanStatus_Not));
			memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			memset(&(st_ScanEnsembleinfo), 0, sizeof(Ts_DabTunerGetEnsembleProperties_reply));
			memset(&(st_ScanServiceinfo), 0, sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
			memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
			memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->b_Direction),msg->data,sizeof(DAB_Tuner_Ctrl_me->b_Direction),&index);
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me,&dab_tuner_ctrl_inst_hsm_active_busy_scan_state);
		}
		break;
		
		case DAB_TUNER_CTRL_DESELBAND_REQID:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] De-select Band request ");
			ensembleIndex = 0; DAB_Tuner_Ctrl_me->serviceindx = 0;DAB_Tuner_Ctrl_me->CompIndex = 0; Services = 0; Components = 0;
			DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
			DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_STATION_UNIDENTIFIED ;
			//ClearLinkingData(DAB_Tuner_Ctrl_me);
			DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level	 =  UN_RELIABLE;
			DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_CANCELLED;   /* added newly for linking cancel */
			DAB_Tuner_Ctrl_Response_DeActivate(REPLYSTATUS_SUCCESS);
			DAB_Tuner_Ctrl_Disable_Fig_Notifications();
			DAB_Tuner_Ctrl_Disable_Quality_Notifications();
			DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x20);
			SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			b_Anno_ongoingflag = FALSE;
			/* Clear all announcement related database */	
			DAB_Tuner_Ctrl_ClearingAnnoDatabases(DAB_Tuner_Ctrl_me);
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndScan_state);
		}
		break;
		
		case HSM_MSGID_EXIT:  
		{
			
		}
		break;
		
		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}	

/*=============================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_NosignalListenHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg) */
/*=============================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_NosignalListenHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_NosignalListenHndlr ");
			/* Store the current state of DAB_Tuner_Ctrl */
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_FG_STATE;
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_NosignalListenHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_NosignalListenHndlr"));
		}
		break;
		
		case DAB_TUNER_CTRL_TUNE_STATUS_NOTIFICATION:
		{
				DAB_Tuner_Ctrl_me->Index = 0;
			/*clearing the seekstatus structure before getting the event*/
				memset(&(DAB_Tuner_Ctrl_me->st_Tunestatus), 0, sizeof(EtalTuneStatus));
			// extracting the seek status data and copying into st_SeekStatus structure 
				ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_Tunestatus), (msg->data), sizeof(EtalTuneStatus), &(DAB_Tuner_Ctrl_me->Index));

			if (DAB_Tuner_Ctrl_me->st_Tunestatus.m_sync > 3)
				{
					/* Command to get the uEID */
					etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
					}
				}
			}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

			/* Command to get the ensemble properties */
			etal_ret = DabTuner_MsgSndGetEnsembleProperties();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
			}

		}
		break;
		
		case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:
		{
			st_TimerId.u32_NosignalListen_Timer = 0;
			/* Re-trigger GetEnsembleProperties_req */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTuner_MsgSndGetEnsembleProperties();
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] GetEnsembleProperties_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			}
		}
		break;
		
		case GET_ENSEMBLE_PROPERTIES_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;			
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Ensemble properties received in no signal listen handler");
			DabTuner_MsgReplyGetEnsembleProperties(&(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply));
			SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_Tunestatus.m_stopFrequency;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId 		= DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.EnsembleIdentifier;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC = DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.ECC ;
			if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x06)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2 ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x0f)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8 ;
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */
			etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
			memset(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),0 , sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
			}
		}
		break;
		
		case GET_PROGRAMLIST_TIMEOUT:
		{
			/* Re-trigger GetProgrammeServiceList_req */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_PROGRAMLIST_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
				memset(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply), 0, sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] GetProgrammeServiceList_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			}
		}	 
		break;		

		case GET_PROGRAM_SERVLIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Program list received in no signal listen handler");
			DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
			//memset(scan_buffer_temp, 0x00, SCAN_SPI_READ_BUFFER_LENGTH);
			DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx = 0;
			etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
			}
		}
		break;
		
		case GET_COMPONENTLIST_TIMEOUT:
		{
			/* Re-trigger GetComponentListReq_Cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] GetComponentListReq_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* No of services of current ensemble is more than one and current service is not the last service*/
				if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) != ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
					&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
				{
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo)* (MAX_ENSEMBLE_SERVICES - ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1)));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;

					/* Re-trigger Get component list for remaining services */
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);				
				}
				/* No of services of current ensemble is more than one and current service is the last service*/
				else if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) == ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
					&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
				{
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;
					/* Copy st_GetCurrEnsembleProgListReply to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
					SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					/* Send sort request to DAB App */
					DAB_App_Request_sort();
					
				}
				else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices == 1)
				{
					/* Clear particular ensemble info in ast_EnsembleInfo array */
					memset(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],0,sizeof(Ts_CurrEnsemble_serviceinfo));
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;
					
				}
				else /* Timeout of GetComponentListReq_Cmd */
				{
					/* Do Nothing */
				}
			}
		}
		break;		
		case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Cmd for getting the service information */
			b_resultcode = DabTuner_GetServListpropertiesReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx]));
			b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));

			DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx++;
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)
				{
					
					etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
					}
				}
				else
				{
					//DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
					/* Clearing FIG 0/2 Data base */
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Component list received in no signal listen handler");
					memset(DAB_Tuner_Ctrl_me->st_Service_Info,0, sizeof(Ts_Service_Info)*MAX_ENSEMBLE_SERVICES);
					DAB_Tuner_Ctrl_me->u8_NoOfServicesIn_ServiceInfo = 0 ;
					//DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE ;
					/* Copy st_GetCurrEnsembleProgListReply to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
					SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					/* Send sort request to DAB App */
					DAB_App_Request_sort();
					/*
					etal_ret = DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId, DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].ProgServiceId);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT);
					}
					*/
				}
			}	 		 
		}
		break;
	
			
		case DAB_TUNER_CTRL_COMP_LIST_SORT_RESID:
		{
			/* Copy st_Dab_Tuner_Ctrl_CurrEnsembleProgList back to st_GetCurrEnsembleProgListReply after sorting */
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply, &st_Dab_Tuner_Ctrl_CurrEnsembleProgList,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					   
			DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = TRUE;
			UpdateStationListInfo(DAB_Tuner_Ctrl_me);
			Update_Stationlist_Into_LearnMem();
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Learn memory updated in no signal listen handler");
		    /* Notify the DAB App that GET_COMPONENT_LIST request is success and send freq and EId info */
			DAB_Tuner_Ctrl_Notify_CompListStatus(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId, REPLYSTATUS_SUCCESS);
			
			
			//DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICECOMPONENT_SELECTED;
			/* Cmd to select a service component in a service */
			etal_ret = DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI, DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT);
			}
		}
		break;
		
		case HSM_MSGID_EXIT:  
		{
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}	

void DAB_Tuner_Ctrl_Service_Following_OPerations(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	
	if((b_handlerchanged == FALSE) && ((LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)) == 0 || DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == FALSE || DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay == 0)) 
	{
		if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >=0x02 && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD && b_Anno_ongoingflag != TRUE)
		{
			if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
			{
				DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE;
				//(void)SYS_StopTimer(4);
				st_TimerId.u32_DecodingStatus_Timer = SYS_StartTimer(480, DAB_TUNER_CTRL_DECODING_STATUS, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DecodingStatus_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_DECODING_STATUS");	
				}
				else
				{
					/*MISRA C*/	
				}
			}	
		}
	}
	else if((LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)) != 0 && DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus != RADIO_FRMWK_COMP_STATUS_ABNORMAL)
	{
		if((DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_IMPLICIT_SID ) && (DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x02) && (DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality > DAB_FM_LINKING_MAX_THRESHOULD))
		{
			if(DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS && DAB_Tuner_Ctrl_me->b_Implicit_Linking != TRUE)
			{
				DAB_Tuner_Ctrl_me->b_Implicit_Linking = TRUE;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Implicit Blending command sent from listen %d ", DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent);
				DabTuner_StartBlending_for_implicit(0x80);
				/* Starting timer for StartBlending_cmd time out */
				if(st_TimerId.u32_StartBlending_Timer > 0)
				{
					if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
					}
					else
					{
						st_TimerId.u32_StartBlending_Timer = 0;
					}									
				}
				else
				{
					/* MISRA C*/	
				}
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
			}	
		}
		else if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) || (DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
		{
			if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01))
			{
				if(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD)
				{
					if(DAB_Tuner_Ctrl_me->b_Timer_start != TRUE)
					{
						DAB_Tuner_Ctrl_me->b_Timer_start = TRUE;
						st_TimerId.u32_DecodingStatus_Timer = SYS_StartTimer(7600, DAB_TUNER_CTRL_DECODING_STATUS, RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_DecodingStatus_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_DECODING_STATUS");	
						}
						else
						{
							/*MISRA C*/	
						}
					}	
				}
			}
		}
		else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x01) && (DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE) && (b_Anno_ongoingflag != TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay != 0) &&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality < DAB_QUALITY_THRESHOULD))
		{
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
				if(DAB_Tuner_Ctrl_me->b_Timer_start != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Timer started for seamless DS 1");
					DAB_Tuner_Ctrl_me->b_Timer_start = TRUE;
					DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
					st_TimerId.u32_DecodingStatus_Timer = SYS_StartTimer(1200, DAB_TUNER_CTRL_DECODING_STATUS, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DecodingStatus_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_DECODING_STATUS");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
			}	
		}
		else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x02)&&(DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE) && (b_Anno_ongoingflag != TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay!= 0))
		{
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
			  if(DAB_Tuner_Ctrl_me->Start_Blending != TRUE)
			  {	
				DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
				if(DAB_Tuner_Ctrl_me->Start_Blending != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
				{
					DAB_Tuner_Ctrl_me->Start_Blending = TRUE;
				}
				if(DAB_Tuner_Ctrl_me->b_Linking_Handler != TRUE)
				{
					DAB_Tuner_Ctrl_me->b_Linking = TRUE;
					if(SYS_StopTimer(st_TimerId.u32_DecodingStatus_Timer) == FALSE)
					{
 						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_DECODING_STATUS failed");	
					}
					else
					{
						st_TimerId.u32_DecodingStatus_Timer=0;
					}	
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DS 2 for blending moved to DAB-FM handler");
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
				}
				else
				{
					DAB_Tuner_Ctrl_me->b_Linking = TRUE;
					if(SYS_StopTimer(st_TimerId.u32_DecodingStatus_Timer) == FALSE)
					{
 						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_DECODING_STATUS failed");	
					}
					else
					{
						st_TimerId.u32_DecodingStatus_Timer=0;
					}	
					if(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality > DAB_FM_LINKING_THRESHOULD)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Blended to FM ");
						DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
						/* Starting timer for DabTuner_StartBlending_cmd time out */
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;
							}										
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(100,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
						DAB_Tuner_Ctrl_me->b_Linking = FALSE;
					}
					else
					{
						b_handlerchanged = TRUE;
						DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
						DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
					}
				}
			  }
			}  	
		}

		else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01) && (DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD) &&(b_handlerchanged == TRUE))
		{
		
			DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
			if(DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes > 0 && DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == FALSE && DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_NORMAL_PLAYBACK)
			{
				DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.b_Hardlinks_sent = FALSE;	
				DAB_Tuner_Ctrl_me->e_BestPI_Type = DAB_TUNER_CTRL_NO_IMPLICIT;
				DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality = 0;
				memset((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks),0,sizeof(DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks));
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Notifying Hardlinks to FM from Listen Handler signal resume back ");
				Notify_Hardlinks_To_FM(DAB_Tuner_Ctrl_me);
			}
			DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_NORMAL_PLAYBACK;
			if(DAB_Tuner_Ctrl_me->b_Notifications_Disabled == TRUE)
			{
				DAB_Tuner_Ctrl_Enable_Fig_Notifications();
				DAB_Tuner_Ctrl_me->b_Notifications_Disabled = FALSE;
			}
		}		
		else if ((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >=0x02 && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD && b_handlerchanged == TRUE))
		{
			if((DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE) && (DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == FALSE))
			 {
				DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE;
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
				st_TimerId.u32_DecodingStatus_Timer = SYS_StartTimer(480, DAB_TUNER_CTRL_DECODING_STATUS, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DecodingStatus_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_DECODING_STATUS");	
				}
				else
				{
					/*MISRA C*/	
				}
				//DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
			 }		
		}	
		else
		{
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;

		}					
	}
	else
	{
		if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01) && (DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD) &&(b_handlerchanged == TRUE))
		{
			DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_NORMAL_PLAYBACK;
			DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
		}	
		else if ((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >=0x02 && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD) && (b_handlerchanged == TRUE))
		{
			if((DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE) && (DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == FALSE))
			 {
				DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE;
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
				st_TimerId.u32_DecodingStatus_Timer = SYS_StartTimer(480, DAB_TUNER_CTRL_DECODING_STATUS, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DecodingStatus_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_DECODING_STATUS");	
				}
				else
				{
					/*MISRA C*/	
				}
			 }		
		}	
		else
		{
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
		}
	}
	
}
/*===========================================================================================================================*/
/* Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_ListenHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)  */
/*===========================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_ListenHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode = FALSE;
	Tu8 DAB_Tuner_Ctrl_Best_PI_Check = 0;
	//Te_Tuner_Ctrl_Bool e_Result = FALSE ; 
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ListenHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ListenHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ListenHndlr"));
			DAB_Tuner_Ctrl_Check_Service_Present_In_Stl(DAB_Tuner_Ctrl_me);
			/* Store the current state of DAB_Tuner_Ctrl */
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_FG_STATE ;
			DAB_Tuner_Ctrl_me->b_Linking_Handler = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
			DAB_Tuner_Ctrl_me->b_Implicit_Linking = FALSE;
			DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
			DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
			DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.u32_TunedSId = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
			DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
			if(DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble == TRUE)
			{
				DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
				DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE;
				DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = FALSE;
			}
			if(SYS_StartTimer(TIME_TO_CHECK_HARDLINKS_FOR_SID, CHECK_HARDLINKS_FOR_TUNED_SID, RADIO_DAB_TUNER_CTRL)<= 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for CHECK_HARDLINKS_FOR_TUNED_SID");
			}
			else
			{
				/*MISRA*/	
			}
		}
		break;

		case DAB_QUALITY_NOTIFICATION_MSGID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB_QUALITY_NOTIFICATION_MSGID");
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_EtalBcastQualityContainer), (msg->data), sizeof(DAB_Tuner_Ctrl_me->st_EtalBcastQualityContainer), &(DAB_Tuner_Ctrl_me->Index));

			if (DAB_Tuner_Ctrl_me->u8_ENGMODEStatus == DAB_TUNER_CTRL_MODE_ON)
			{
				memset(&DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification, 0, sizeof(Ts_Tuner_Status_Notification));

				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s32_RFFieldStrength = (Ts32)DAB_Tuner_Ctrl_me->st_EtalBcastQualityContainer.EtalQualityEntries.dab.m_RFFieldStrength;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s32_BBFieldStrength = (Ts32)DAB_Tuner_Ctrl_me->st_EtalBcastQualityContainer.EtalQualityEntries.dab.m_BBFieldStrength;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.b_isValidMscBitErrorRatio = (Tbool)DAB_Tuner_Ctrl_me->st_EtalBcastQualityContainer.EtalQualityEntries.dab.m_isValidMscBitErrorRatio;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u32_MscBitErrorRatio = (Tu32)DAB_Tuner_Ctrl_me->st_EtalBcastQualityContainer.EtalQualityEntries.dab.m_MscBitErrorRatio;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u32_FicBitErrorRatio = (Tu32)DAB_Tuner_Ctrl_me->st_EtalBcastQualityContainer.EtalQualityEntries.dab.m_FicBitErrorRatio;

				DAB_Tuner_Ctrl_Notify_TunerStatus(DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification);
				
			}
			else
			{

			}

		}
		break;

		case AUDIO_STATUS_NOTIFICATION:
		{
			b_resultcode = DabTuner_MsgRcvSetAudioStatusNotifier(&(DAB_Tuner_Ctrl_me->st_Audio_Status_notification), msg->data);
			if(b_resultcode == TRUE)
			{
				
			}
		}
		break;

		case BER_STATUS_NOTIFICATION:
		{
			b_resultcode = DabTuner_MsgRcvSetPeriodicalBERQualityNotifier(&(DAB_Tuner_Ctrl_me->st_BER_notification),msg->data);
			if(DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent = DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_BER_Exponent;
			}
			else
			{
				
			}
			if(DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD)
			{
				/* Starting Linking after 10 sec once seek operation is performed */
				DAB_Tuner_Ctrl_Service_Following_OPerations(DAB_Tuner_Ctrl_me);
			}
			else
			{
				
			}
			if(b_resultcode == TRUE)
			{
				if((DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_BER_Exponent != DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_Decodingstatus != DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioQuality != DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioLevel != DAB_Tuner_Ctrl_me->st_Audiostatus.AudioLevel))
				{
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_BER_Significant = DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Significant;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_BER_Exponent = DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_Decodingstatus = DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioQuality = DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioLevel = DAB_Tuner_Ctrl_me->st_Audiostatus.AudioLevel;
					DAB_Tuner_Ctrl_Notify_TunerStatus(DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification);
					DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,QUALITY_PARAMETERS);
				}
			}
		}
		break;

		case RSSI_NOTIFICATION:
		{
			b_resultcode = DabTuner_MsgRcvRSSI_notify(&(DAB_Tuner_Ctrl_me->st_RSSI_notification),msg->data);
			if(b_resultcode == TRUE)
			{
				if((DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_RSSI != DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_Decodingstatus != DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioQuality != DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioLevel != DAB_Tuner_Ctrl_me->st_Audiostatus.AudioLevel))
				{
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_Decodingstatus = DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioQuality = DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioLevel = DAB_Tuner_Ctrl_me->st_Audiostatus.AudioLevel;
					DAB_Tuner_Ctrl_Notify_TunerStatus(DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification);
					DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,QUALITY_PARAMETERS);
				}	
			}
		}	
		break;

		case SNR_NOTIFICATION:
		{
			b_resultcode = DabTuner_MsgRcvSNRNotifier(&(DAB_Tuner_Ctrl_me->st_SNR_notification),msg->data);
			if(b_resultcode == TRUE)
			{
				if((DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_SNR_Level < (DAB_Tuner_Ctrl_me->st_SNR_notification.SNRLevel -2)) 
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_SNR_Level > (DAB_Tuner_Ctrl_me->st_SNR_notification.SNRLevel +2))
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_Decodingstatus != DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioQuality != DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality)
					|| (DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioLevel != DAB_Tuner_Ctrl_me->st_Audiostatus.AudioLevel))
				{
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_SNR_Level = DAB_Tuner_Ctrl_me->st_SNR_notification.SNRLevel;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_Decodingstatus = DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioQuality = DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality;
					DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioLevel = DAB_Tuner_Ctrl_me->st_Audiostatus.AudioLevel;
					DAB_Tuner_Ctrl_Notify_TunerStatus(DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification);
					DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,QUALITY_PARAMETERS);
				}
			}
		}
		break;
		case CHECK_HARDLINKS_FOR_TUNED_SID:
		{
			Check_Hardlinks_For_Tuned_SID(DAB_Tuner_Ctrl_me);
		
		}
		break;
		
		case FIG_DATA_NOTIFICATION:
		{
			/* Checking if no. of services in current ensemble program list and same as no. of servies in FIG 0/2 data base */
			if((DAB_Tuner_Ctrl_me->u8_NoOfServicesIn_ServiceInfo == DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) && (DAB_Tuner_Ctrl_me->b_FIG02_Disabled == FALSE))
			{
				/* Disabling FIG 0/2 */ 
				DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x20);
				DAB_Tuner_Ctrl_me->b_FIG02_Disabled = TRUE ;
			}
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
				ExtractFIB(DAB_Tuner_Ctrl_me, msg->data);
				if((DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available == TRUE )||(DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_available == TRUE) || (DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG024_available == TRUE)||(DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_Hardlink_PI_available == TRUE))
				{
					Update_CurSId_Hardlink_DataBase(DAB_Tuner_Ctrl_me);
					Notify_Hardlinks_To_FM(DAB_Tuner_Ctrl_me);
#if 0
					if((DAB_Tuner_Ctrl_me->st_Linkingstatus.FMHardlinksAvailable == TRUE) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
					{
						Update_CurSId_Hardlink_DataBase(DAB_Tuner_Ctrl_me);
						Notify_Hardlinks_To_FM(DAB_Tuner_Ctrl_me);
						if((DAB_Tuner_Ctrl_me->st_Linkingstatus.FMHardlinksAvailable == TRUE) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
						{
							if((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlinks_sent == 1) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA == 0))
							{
								memset(&(DAB_Tuner_Ctrl_me->st_PI_data_available),0,sizeof(Ts_PI_Data));
								DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlinks_sent = 0;
								DAB_Tuner_Ctrl_Notify_Hardlinks_Status(DAB_TUNER_CTRL_FM_LINKING_CANCELLED);
							}
							else
							{
								for(DAB_Tuner_Ctrl_me->Index=0;((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks[DAB_Tuner_Ctrl_me->Index] != DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[DAB_Tuner_Ctrl_me->Index]) && (DAB_Tuner_Ctrl_me->Index <DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes)) ;DAB_Tuner_Ctrl_me->Index++)
								{
									memcpy((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks),(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI),sizeof(DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks));
									if(DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == DAB_TUNER_CTRL_COMP_STATUS_NORMAL)   If FM is NORMAL onlywe will send Hardlinks to FM.
									{
										DAB_Tuner_Ctrl_Notify_PICodeList((DAB_Tuner_Ctrl_me->st_PI_data_available),DAB_FM_LINKING_MIN_THRESHOULD,DAB_FM_LINKING_MAX_THRESHOULD,DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId,CHECK_HARDLINK);	
										DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlinks_sent = 1;
										DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Received = 0;
									}
									break;
								}
							}	
					}
#endif
					DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available = FALSE;
					DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_available = FALSE;
					DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG024_available = FALSE;
					DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_Hardlink_PI_available = FALSE;
				}
			}
		}
		break;


		case DLS_DATA_NOTIFICATION:
		{	

			DabTunerMsg_XPAD_DLS_Data(&(DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data),msg->data);
			if(DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.Runningstatus == 1)
			{
				if(DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.CharacterSet == 0)
				{
					DAB_Tuner_Ctrl_me->st_Dls_data.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
				}
				else if (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.CharacterSet == 6)
				{
					DAB_Tuner_Ctrl_me->st_Dls_data.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
				}
				else if (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.CharacterSet == 15)
				{
					DAB_Tuner_Ctrl_me->st_Dls_data.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;
				}
				else
				{

				}
				SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_Dls_data.au8_DLSData), (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.DLS_LabelByte), 128);
				if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE) || (DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == FALSE))
					DAB_Tuner_Ctrl_Notify_DLSdata(DAB_Tuner_Ctrl_me->st_Dls_data);
			}
			else
			{

			}
		
		}
		break;
		case SLS_DATA_NOTIFICATION:
		{
			SYS_MUTEX_LOCK(SLS_DAB_APP_DAB_TC);
			SYS_RADIO_MEMCPY(&st_DabTc_SLS_Data, &st_SLS_Data, sizeof(st_SLS_Data));
			SYS_MUTEX_UNLOCK(SLS_DAB_APP_DAB_TC);
			DAB_Tuner_Ctrl_Notify_SLSdata();
		}
		break;

		case  BEST_PI_RECEIVED_NOTIFICATION:
		{
			DAB_Tuner_Ctrl_me->Index = 0; 
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI),(msg->data),sizeof(Tu16),&(DAB_Tuner_Ctrl_me->Index));	
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality),(msg->data),sizeof(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality),&(DAB_Tuner_Ctrl_me->Index));
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_BestPI_Type),(msg->data),sizeof(Te_Tuner_Ctrl_BestPI_Type),&(DAB_Tuner_Ctrl_me->Index));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Best Pi notification received");
			/* u8_ImplicitPIstatus == 0 : DAB-FM linking	*/
			if(DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_HARDLINK_PI)
			{
				for(DAB_Tuner_Ctrl_Best_PI_Check = 0; DAB_Tuner_Ctrl_Best_PI_Check < DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes ; DAB_Tuner_Ctrl_Best_PI_Check++ )
				{
					if(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI == DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks[DAB_Tuner_Ctrl_Best_PI_Check])
					{
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED;

						DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = TRUE;
						DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLENDING_INFO);
						break;
					}
					else
					{
						DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
					}	
				}
				//DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = TRUE;
			}
			/* u8_ImplicitPIstatus == 1 : Implicit linking	*/	
			else if(DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_IMPLICIT_SID)
			{
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,IMPLICIT_SID);
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED;
			}
			/* u8_ImplicitPIstatus == 2 : DAB-DAB linking	*/
			else if(DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_NO_IMPLICIT)
			{
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT;
			}	
			else
			{
				/*Do nothing*/
			}
			if(DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE)
			{

				if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE)
				{
					if(DAB_Tuner_Ctrl_me->Start_Blending != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
					{
						DAB_Tuner_Ctrl_me->b_Linking = FALSE;
						DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;
							}							
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
						if((DAB_Tuner_Ctrl_me->b_Linking_Handler == FALSE))
						{
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
						}
					}
					else
					{
						
					}
					
					
				}
				else	
				{
					if(DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay == 0)
						DAB_Tuner_Ctrl_me->Best_PI_notification++;
					else
						DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
						
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED;
					if((DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_HARDLINK_PI) && (b_Anno_ongoingflag != TRUE))
					{
						if((DAB_Tuner_Ctrl_me->b_Linking_Handler == FALSE))
						{
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
						}
						else
						{
							DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State();
						}
					}
				}
			}	
		}
		break;

		case DAB_TUNER_CTRL_AUDIO_STATUS_NOTIFICATION:
		{
			b_resultcode = DabTuner_Get_Audio_Status_Notification(&(DAB_Tuner_Ctrl_me->st_Audiostatus), (Ts8*)(msg->data));
			if(b_resultcode == TRUE)
			{
				/* Signal loss scenario while announcement is playing */
				if((b_Anno_ongoingflag == TRUE) && (DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent == FALSE) && ((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x01) || (DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x02)))
				{
					DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent = TRUE;
					/* Notify Dab App that announcement signal is lost */
					DAB_Tuner_Ctrl_Notify_AnnoSignalLoss_Status();
				}
				else if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings != DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
				{
					/* Starting Linking after 10 sec once seek operation is performed */
						DAB_Tuner_Ctrl_Service_Following_OPerations(DAB_Tuner_Ctrl_me);
				}
				else
				{
					/*MISRA C*/
				}
			}
		}
		break;				
/************************************************************************************************************************************************************************************************/					
					
					/*	if((DAB_Tuner_Ctrl_me->u8_SettingStatus & 0x02) == 0 || DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Received == 0 || DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay == 0 && handlerchanged == 0) 
						{
							if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >=0x01 && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD && e_Anno_ongoingflag != TRUE)
							{
								if(DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start != TRUE)
								{
									DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start = TRUE;
									//(void)SYS_StopTimer(4);
									(void)SYS_StartTimer(4, 480, DAB_TUNER_CTRL_DECODING_STATUS);
								}	
							}
						}
						else if((DAB_Tuner_Ctrl_me->u8_SettingStatus & 0x02) != 0 && DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus != DAB_TUNER_CTRL_COMP_STATUS_ABNORMAL)
						{
							if((DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_IMPLICIT_SID ) && (DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x02) && (DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality > DAB_FM_LINKING_MAX_THRESHOULD))
							{
								if(DAB_Tuner_Ctrl_me->e_LinkingStatus != DAB_TUNER_CTRL_IMPLICIT_FM_BLENDING_SUCCESS)
								{
									DabTuner_StartBlending_for_implicit(0x80);
									// Starting timer for StartBlending_cmd time out 
									(void)SYS_StartTimer(4, DAB_START_BLENDING_CMD_TIMEOUT_TIME, START_BLENDING_REPLY_TIMEOUT);
								}	
							}
							else if((DAB_Tuner_Ctrl_me->e_LinkingStatus == DAB_TUNER_CTRL_FM_BLENDING_SUCCESS) || (DAB_Tuner_Ctrl_me->e_LinkingStatus == DAB_TUNER_CTRL_IMPLICIT_FM_BLENDING_SUCCESS))
							{
								if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01))
								{
									if(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD)
									{
										if(DAB_Tuner_Ctrl_me->Timer_start != TRUE)
										{
											DAB_Tuner_Ctrl_me->Timer_start = TRUE;
											(void)SYS_StartTimer(4, 7600, DAB_TUNER_CTRL_DECODING_STATUS);
										}	
									}
								}
							}
							else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x01) && (DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Received == 1) && (e_Anno_ongoingflag != TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay != 0))
							{
								if((DAB_Tuner_Ctrl_me->e_LinkingStatus != DAB_TUNER_CTRL_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != DAB_TUNER_CTRL_IMPLICIT_FM_BLENDING_SUCCESS))
								{
									if(DAB_Tuner_Ctrl_me->Timer_start != TRUE)
									{
										DAB_Tuner_Ctrl_me->Timer_start = TRUE;
										DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
										(void)SYS_StartTimer(4, 1200, DAB_TUNER_CTRL_DECODING_STATUS);
									}
								}	
							}
							else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x02)&&(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Received == 1) && (e_Anno_ongoingflag != TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay!= 0))
							{
								if((DAB_Tuner_Ctrl_me->e_LinkingStatus != DAB_TUNER_CTRL_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != DAB_TUNER_CTRL_IMPLICIT_FM_BLENDING_SUCCESS))
								{
								  if(DAB_Tuner_Ctrl_me->Start_Blending != TRUE)
								  {	
									DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
									if(DAB_Tuner_Ctrl_me->Start_Blending != DAB_TUNER_CTRL_FM_BLENDING_SUCCESS)
									{
										DAB_Tuner_Ctrl_me->Start_Blending = TRUE;
									}
									if(DAB_Tuner_Ctrl_me->Linking_Handler != 1u)
									{
										DAB_Tuner_Ctrl_me->Linking = TRUE;
										(void)SYS_StopTimer(4);	
										HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
									}
									else
									{
										DAB_Tuner_Ctrl_me->Linking = TRUE;
										(void)SYS_StopTimer(4);
										if(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality > DAB_FM_LINKING_THRESHOULD)
										{
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Blended to FM ");
											DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
											// Starting timer for DabTuner_StartBlending_cmd time out 
											(void)SYS_StartTimer(4, 100, START_BLENDING_REPLY_TIMEOUT);
											DAB_Tuner_Ctrl_me->Linking = FALSE;
										}
										else
										{
											handlerchanged = 1;
											DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
											DAB_Tuner_Ctrl_me->Timer_start = FALSE;
											DAB_Tuner_Ctrl_me->e_LinkingStatus = DAB_TUNER_CTRL_FM_LINKING_NOT_AVAILABLE;
											HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
										}

									//DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State();
									}
								  }
								}  	
							}
					
							else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01) && (DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD) &&(handlerchanged == 1))
							{
								DAB_Tuner_Ctrl_me->e_LinkingStatus = DAB_TUNER_CTRL_DAB_NORMAL_PLAYBACK;
								DAB_Tuner_Ctrl_me->u8_DAB_Alternatecheck_Ongoing = FALSE;
								DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start = FALSE;
								DAB_Tuner_Ctrl_me->Timer_start = FALSE;
								if((DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes > 0) && (DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Received == 0))
								{
									DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlinks_sent = 0;	
									DAB_Tuner_Ctrl_me->e_BestPI_Type = DAB_TUNER_CTRL_NO_IMPLICIT;
									DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality = 0;
									memset((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks),0,sizeof(DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks));
									Notify_Hardlinks_To_FM(DAB_Tuner_Ctrl_me);
								}
								if(DAB_Tuner_Ctrl_me->e_Notifications_Disabled == TRUE)
								{
									DAB_Tuner_Ctrl_Enable_Fig_Notifications();
									DAB_Tuner_Ctrl_me->e_Notifications_Disabled = FALSE;
								}
							}		
							else if ((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >=0x01 && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD) && (handlerchanged == 1))
							{
								if((DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start != TRUE) && (DAB_Tuner_Ctrl_me->u8_DAB_Alternatecheck_Ongoing == FALSE))
								 {
									DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start = TRUE;
									DAB_Tuner_Ctrl_me->e_LinkingStatus = DAB_TUNER_CTRL_FM_LINKING_NOT_AVAILABLE;
									(void)SYS_StartTimer(4, 480, DAB_TUNER_CTRL_DECODING_STATUS);
									//DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
								 }		
							}	
							else
							{
								DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start = FALSE;
						
							}					
						}
						else
						{
							if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01) && (DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD) &&(handlerchanged == 1))
							{
								DAB_Tuner_Ctrl_me->e_LinkingStatus = DAB_TUNER_CTRL_DAB_NORMAL_PLAYBACK;
								DAB_Tuner_Ctrl_me->u8_DAB_Alternatecheck_Ongoing = FALSE;
								DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start = FALSE;
								DAB_Tuner_Ctrl_me->Timer_start = FALSE;
								DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
							}	
							else if ((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >=0x01 && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD) && (handlerchanged == 1))
							{
								if((DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start != TRUE) && (DAB_Tuner_Ctrl_me->u8_DAB_Alternatecheck_Ongoing == FALSE))
								 {
									DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start = TRUE;
									DAB_Tuner_Ctrl_me->e_LinkingStatus = DAB_TUNER_CTRL_FM_LINKING_NOT_AVAILABLE;
									(void)SYS_StartTimer(4, 480, DAB_TUNER_CTRL_DECODING_STATUS);
								 }		
							}	
							else
							{
								DAB_Tuner_Ctrl_me->DAB_DAB_Linking_Timer_start = FALSE;
							}
						}
					}
					else
					{
						
					}
				}
				else
				{
					
				}
			}	
			else
			{
				// RDS DISBALE	
				
			}	
		}
		break;*/
/******************************************************************************************************************************************************/

		case DAB_TUNER_CTRL_DECODING_STATUS:
		{
			st_TimerId.u32_DecodingStatus_Timer = 0;
			DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
			
			/*** After Blending DAB to FM has to check this condition */
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings != DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
			{
				if((DAB_Tuner_Ctrl_me->Start_Blending == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)) && DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus != RADIO_FRMWK_COMP_STATUS_ABNORMAL)
				{
					if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01) && (DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD))
					{
						if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
						{
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;
						  	DabTuner_StartBlending_for_implicit(0x00);
							/* Starting timer for StartBlending_cmd time out */
							if(st_TimerId.u32_StartBlending_Timer > 0)
							{
								if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
								}
								else
								{
									st_TimerId.u32_StartBlending_Timer = 0;
								}									
							}
							else
							{
								/* MISRA C*/	
							}
							st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
							if(st_TimerId.u32_StartBlending_Timer == 0)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
							}
							else
							{
								/*MISRA C*/	
							}
						}	
						else
						{
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state);
						  	//DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State();
						}	
					}
				}
				/* Before blending DAB to FM has to check this condition */
				else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x01) && DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality < DAB_QUALITY_THRESHOULD && (DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay!= 0) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)) && DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus != RADIO_FRMWK_COMP_STATUS_ABNORMAL)
				{	
					if(DAB_Tuner_Ctrl_me->Start_Blending != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
					{
						DAB_Tuner_Ctrl_me->Start_Blending = TRUE;
					}
					if(DAB_Tuner_Ctrl_me->b_Linking_Handler != TRUE)
					{
						DAB_Tuner_Ctrl_me->b_Linking = TRUE;
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
					}
					else
					{
						DAB_Tuner_Ctrl_me->b_Linking = TRUE;
						DAB_Tuner_Ctrl_Internal_Msg_To_DAB_FM_Blending_State();
					}	
				}
				else if(((DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == FALSE) || (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay == 0) || ((Tu8)(LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)) == 0)) && (DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x02) && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD && (b_handlerchanged == FALSE))
				{
					//b_handlerchanged = TRUE;
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
					memset(&(DAB_Tuner_Ctrl_me->st_Blending_info),0,sizeof(Ts_Blending_info));
					DAB_Tuner_Ctrl_Disable_Fig_Notifications();
					DAB_Tuner_Ctrl_me->b_Notifications_Disabled = TRUE;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
				}
				else if(((DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == FALSE) || (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay == 0) || ((Tu8)(LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)) == 0)) && (DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x02) && DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD && (b_handlerchanged == TRUE))
				{
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
					memset(&(DAB_Tuner_Ctrl_me->st_Blending_info),0,sizeof(Ts_Blending_info));
					DAB_Tuner_Ctrl_Disable_Fig_Notifications();
					DAB_Tuner_Ctrl_me->b_Notifications_Disabled = TRUE;
					if(st_TimerId.u32_DAB_DABLinking_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
						}
						else
						{
							st_TimerId.u32_DAB_DABLinking_Timer = 0;
						}						
					}
					else
					{
						/* MISRA C*/	
					}
					st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(50, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				else
				{
					DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
					DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
				}
			}
			else
			{
				DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
				DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
			}

		}
		break;

		case DAB_TUNER_CTRL_DAB_FM_LINKING_STATUS_NOTIFYID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
			DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality = 0;
			DAB_Tuner_Ctrl_me->e_BestPI_Type = DAB_TUNER_CTRL_NO_IMPLICIT;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB_TUNER_CTRL_FM_LINKING_NOT_AVAILABLE received from FM ");
			if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
			{
				ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_LinkingStatus),(msg->data),sizeof(Te_RADIO_DABFM_LinkingStatus),&(DAB_Tuner_Ctrl_me->Index));
				if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE)
				{
				  	DabTuner_StartBlending_for_implicit(0x00);
					if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK ;
					else
					{
						if(st_TimerId.u32_DAB_DABLinking_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
							}
							else
							{
								st_TimerId.u32_DAB_DABLinking_Timer = 0;
							}								
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(400, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
						}
						else
						{
							/*MISRA C*/	
						}
					}
				}
			}
			else if (DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
			{
				ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_LinkingStatus),(msg->data),sizeof(Te_RADIO_DABFM_LinkingStatus),&(DAB_Tuner_Ctrl_me->Index));
				if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE)
				{
					if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)
					{
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK ;
						if(SYS_StopTimer(st_TimerId.u32_DecodingStatus_Timer) == FALSE)
						{
			 				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_DECODING_STATUS failed");	
						}
						else
						{
							st_TimerId.u32_DecodingStatus_Timer = 0;
						}							 
					}							 
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state);      /* This is added because, while DAB_FM linking fails it has to go to DAB-DAB linking */
				}	
			}
			else
			{
				/*if((DAB_Tuner_Ctrl_me->e_LinkingStatus != DAB_TUNER_CTRL_DAB_NORMAL_PLAYBACK) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != DAB_TUNER_CTRL_FM_BLENDING_SUSPENDED))
				{
					ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_LinkingStatus),(msg->data),sizeof(Te_RADIO_DABFM_LinkingStatus),&(DAB_Tuner_Ctrl_me->Index));
				}*/
				DAB_Tuner_Ctrl_me->e_BestPI_Type = DAB_TUNER_CTRL_NO_IMPLICIT;
			}
		}
		break;

		case PROGRAMME_SERV_LIST_CHANGED_NOTIFICATION:
		{
			if(DAB_Tuner_Ctrl_me->b_ReConfigurationFlag == TRUE)
			{
				DAB_Tuner_Ctrl_Disable_Fig_Notifications();
				DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x20);
				DAB_Tuner_Ctrl_me->b_FIG02_Disabled = TRUE ;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Multiplex Re-configuration happened");
				DAB_Tuner_Ctrl_me->b_ReConfigurationFlag = FALSE ;
				
				/* clearing st_ProgrammeServListChanged_not */
				memset(&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not),0,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			
				/* Updating service list in st_ProgrammeServListChanged_not */
//				DabTunerCtrl_ProgrammeServListChanged_not(&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not),&scan_buffer_temp[8]);
			
				/* Clearing the scan_buffer_temp buffer */
//				memset(scan_buffer_temp, 0x00, SCAN_SPI_READ_BUFFER_LENGTH);
			
				/* Copy st_ProgrammeServListChanged_not to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
				SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
				SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
				SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
				/* Send sort request to DAB App */
				DAB_App_Request_sort();
			}		
		}
		break;

		case SERVICE_PROPERTY_CHANGED_NOTIFICATION:
		{
			if(DAB_Tuner_Ctrl_me->b_ReConfigurationFlag == TRUE)
			{
				DAB_Tuner_Ctrl_Disable_Fig_Notifications();
				DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x20);
				DAB_Tuner_Ctrl_me->b_FIG02_Disabled = TRUE ;
				DAB_Tuner_Ctrl_me->b_ReConfigurationFlag = FALSE ;
				DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx = 0 ;
				DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_SERVICE_RECONFIG ;
				DabTunerCtrl_ServPropsChanged_not(&(DAB_Tuner_Ctrl_me->st_ServPropsChanged_not),msg->data);
				/* Finding the index of currenty selected service present in the  st_GetCurrEnsembleProgListReply (i.e Current ensemble service and service component list)*/
			 	for(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx = 0 ; DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx < MAX_ENSEMBLE_SERVICES ; DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx++)
				{
					if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId == DAB_Tuner_Ctrl_me->st_ServPropsChanged_not.ServiceId)
					{
						break ;
					}	
					else
					{
					/* Doing nothing */
					} /* For MISRA C */
				}
				/* Getting service components of the currently selected service and updating the components in the st_GetCurrEnsembleProgListReply (i.e Current ensemble station list) */
				(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
				/* Starting timer for GetComponentListReq_Cmd time out */
				if(st_TimerId.u32_Listen_Timer > 0)
				{
					if(SYS_StopTimer(st_TimerId.u32_Listen_Timer) == FALSE)
					{
			 			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_COMPONENTLIST_TIMEOUT failed");	
					}
					else
					{
						st_TimerId.u32_Listen_Timer = 0;
					}
				
				}
				else
				{
					/*MISRA C*/	
				}
				st_TimerId.u32_Listen_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
				if(st_TimerId.u32_Listen_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}			
			}
		}
		break;
		
		case GET_COMPONENTLIST_TIMEOUT:
		{
			st_TimerId.u32_Listen_Timer = 0;
			/* Re-trigger GetComponentListReq_Cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT)
			{
				if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_LIST_RECONFIG)
				{
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId) ;
				}	 
				else if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_RECONFIG)
				{
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
				}
				else
				{
					/* Doing nothing */
				}
				/* Starting timer for GetComponentListReq_Cmd time out */
				st_TimerId.u32_Listen_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
				if(st_TimerId.u32_Listen_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}			
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] After Reconfiguration,GetComponentListReq_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_LIST_RECONFIG)
				{
					/* No of services of current ensemble is more than one and current service is not the last service*/
					if(((DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices) != ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
						&& (DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices > 1))
					{
						/* Move service list to the previous service to remove current service information from service list */
						memmove(&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo)* (MAX_ENSEMBLE_SERVICES - ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+2)));
						/* Decrement total no of services count*/
						(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices)--;

						/* Re-trigger Get component list for remaining services */
						(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
						/* Starting timer for GetComponentListReq_Cmd time out */
						st_TimerId.u32_Listen_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
						if(st_TimerId.u32_Listen_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}				
					}
					/* No of services of current ensemble is more than one and current service is the last service*/
					else if(((DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices) == ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
						&& (DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices > 1))
					{
						/* Move service list to the previous service to remove current service information from service list */
						memmove(&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
						/* Decrement total no of services count*/
						(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices)--;
						
						/* Checking if the curent ensemble service list is same as the newly received list */
						b_resultcode = CompareCurrentEnsembleServiceList(&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not),&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
						
						/* If the newly received service list is not same as current ensemble service list then update current ensemble service list */
						if(b_resultcode == FALSE)
						{
							/* Updating the current ensemble service list */		 
							UpdateCurrentEnsembleServiceList(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not));
							/* Updating ast_EnsembleInfo, ast_ServiceInfo and ast_ComponentInfo */
							SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
							UpdateEnsembleInfo(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							UpdateServiceInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							UpdateServiceCompInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							/* Notifying station list change to DAB application */						 
							 DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							 Update_Stationlist_Into_LearnMem();
							/* Notifying Re-Configuration to DAB application */
							DAB_Tuner_Ctrl_Notify_ReConfiguration(TUNER_CTRL_SERVICE_LIST_RECONFIG, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId,
							DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].ProgServiceId,
							DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId) ;
							DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
							DAB_Tuner_Ctrl_Enable_Fig_Notifications();
							DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
							DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE;
						}
						else
						{
							DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
							DAB_Tuner_Ctrl_Enable_Fig_Notifications();
							DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
							DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE;
						}
						
					}
					else if(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices == 1)
					{
						/* Clear particular ensemble info in ast_EnsembleInfo array */
						memset(&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],0,sizeof(Ts_CurrEnsemble_serviceinfo));
						/* Move service list to the previous service to remove current service information from service list */
						memmove(&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
						/* Decrement total no of services count*/
						(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices)--;
						DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices = DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices ;
					
					}
					else /* Timeout of GetComponentListReq_Cmd */
					{
					}
				}	 
				else if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_RECONFIG)
				{
					/* No Component List is received for tuned service Hence send reConfiguration notification having first sid and primary component information */
					DAB_Tuner_Ctrl_Notify_ReConfiguration(TUNER_CTRL_SERVICE_RECONFIG, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId,
					DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].ProgServiceId,
					DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId) ;
				}
				DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
				DAB_Tuner_Ctrl_Enable_Fig_Notifications();
				DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
				DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE;
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY:
		{	
			if(SYS_StopTimer(st_TimerId.u32_Listen_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_COMPONENTLIST_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_Listen_Timer = 0;
			}				
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_LIST_RECONFIG)
			{
				b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));
			}	 
			else if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_RECONFIG)
			{
				b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));
			}
			else
			{
				/* Doing nothing */
			}/* For MISRA C */
			
			
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_LIST_RECONFIG)
				{
					if(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx!=DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices)
					{
						DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx++;
						(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
					}
					else
					{
						/* Checking if the curent ensemble service list is same as the newly received list */
						b_resultcode = CompareCurrentEnsembleServiceList(&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not),&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
						
						/* If the newly received service list is not same as current ensemble service list then update current ensemble service list */
						if(b_resultcode == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Change in program list, updating current ensemble program list ");
							/* Updating the current ensemble service list */		 
							UpdateCurrentEnsembleServiceList(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not));
							/* Updating ast_EnsembleInfo, ast_ServiceInfo and ast_ComponentInfo */
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Updating station list ");
							SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
							UpdateEnsembleInfo(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							UpdateServiceInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							UpdateServiceCompInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							/* Notifying station list change to DAB application */						 
							 DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							 Update_Stationlist_Into_LearnMem();
							/* Notifying Re-Configuration to DAB application */
							DAB_Tuner_Ctrl_Notify_ReConfiguration(TUNER_CTRL_SERVICE_LIST_RECONFIG, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId,
							DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].ProgServiceId,
							DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId) ;
							DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] No Change in program list, no Multiplex Re-Configuration happened ");
							DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
						}
						DAB_Tuner_Ctrl_Enable_Fig_Notifications();
						DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
						DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE;
					}
				}
				
				else if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_RECONFIG)
				{
					/* Updating current SId components in ast_ComponentInfo */
					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
					UpdateCurrentServiceCompsToServiceCompInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData), DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx);
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					DAB_Tuner_Ctrl_Notify_ReConfiguration(TUNER_CTRL_SERVICE_RECONFIG, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId,
					DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId,
					DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo.Component[0].InternalCompId) ;
					DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
					DAB_Tuner_Ctrl_Enable_Fig_Notifications();
					DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
					DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE;
				}
				else
				{
					/* Doing nothing */
				}/* For MISRA C */
			}
		}   
		break;   
		
		/* Notification from Dab_App component when samechannel announcement is ON/OFF */
		case DAB_TUNER_CTRL_DAB_SAMECHANNELANNO_NOTIFYID:
		{
			Tu8 		u8_index			 = 0;
			Tu8      	u8_match_position	 = 0xff;
			Tbool       b_ASWinfo_found	     = FALSE;
			DAB_Tuner_Ctrl_me->Index = 0;
	 		ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_Tuner_Ctrl_AnnoIndication),(msg->data),sizeof(Te_Tuner_Ctrl_AnnoIndication),&(DAB_Tuner_Ctrl_me->Index));
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u8_SameChannelSubChId),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u8_SameChannelClusterid),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));
			
			if(DAB_Tuner_Ctrl_me->e_Tuner_Ctrl_AnnoIndication == DAB_TUNER_CTRL_ANNO_ON)
			{
				b_Anno_ongoingflag = TRUE;
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Same Channel announcement start notification");
			}
			else if(DAB_Tuner_Ctrl_me->e_Tuner_Ctrl_AnnoIndication == DAB_TUNER_CTRL_ANNO_OFF)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Same Channel announcement stop notification");
				if(DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent == TRUE)
				{
					DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent = FALSE;
				}
				else
				{
					
				}/* For MISRA*/
				
				/* Clear Anno switching database when user announcement cancel is received*/
				if((DAB_Tuner_Ctrl_me->u8_Anno_index > 0) && (DAB_Tuner_Ctrl_me->u8_Anno_index < MAX_NUM_OF_ANNO_DATA)) /* only if there is Announcement switching information sent already, then send stop*/
				{
					/* finding and clearing the announcement switching data from announcement switching structure 'st_Anno_Swtch_Notify_Info' of that particular announcement which has stopped  */
					for(u8_index=0; ((u8_index < DAB_Tuner_Ctrl_me->u8_Anno_index) && (b_ASWinfo_found == FALSE) ); u8_index++)
					{
						//Tu8 u8_cmp = 0 ;
						//u8_cmp = memcmp(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_Anno_index].st_Anno_Swtch_Info),&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info), sizeof(Ts_Anno_Swtch_Info));
						if((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_Clusterid) == (DAB_Tuner_Ctrl_me->u8_SameChannelClusterid)
							&& (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_SubChId) == (DAB_Tuner_Ctrl_me->u8_SameChannelSubChId))
						{
							u8_match_position = u8_index;
							b_ASWinfo_found = TRUE; /* condition to break the For loop*/
						}
						else
						{
							/* Do Nothing */
						}
					}
				
					if(u8_match_position != 0xff)
					{
						if((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].b_Notification_sent_flag == TRUE) && (b_Anno_ongoingflag == TRUE)) 
						{
							memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position]),0,sizeof(Ts_Anno_Swtch_Notify_Info));
							
							/* Moving the remaining announcement switching data to eliminate empty array element */
							SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position]), &(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position+1]),(12 - u8_match_position - 1)*sizeof(Ts_Anno_Swtch_Notify_Info));
						
							/* Clear last array element */
							memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index]),0,sizeof(Ts_Anno_Swtch_Notify_Info));
							if(DAB_Tuner_Ctrl_me->u8_Anno_index > 0)
							{		
								DAB_Tuner_Ctrl_me->u8_Anno_index--;
							}		
							
						}
						else
						{
							/* No need to clear announcement data as it is not stored in database*/
						}
					}
					else
					{
						/* Do Nothing */
					}
				}
				else
				{
				
				}/*For MISRA*/
				b_Anno_ongoingflag = FALSE;
				
			}
			else
			{
				
			}/*For MISRA*/
		}
		break;
		                                                          
		 case DAB_TUNER_CTRL_ANNOUNCEMENT_START_SWITCHING_REQID:
		 {	
			DAB_Tuner_Ctrl_me->Index = 0;
	 		ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_tuned_announcement_type),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&(DAB_Tuner_Ctrl_me->Index));
		
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u8_Subchannelid),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));
 		
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG,"[RADIO][DAB_TC] DAB_TUNER_CTRL: Transition to Announcement state ");
		
			if(DAB_Tuner_Ctrl_me->b_Anno_Onstate == TRUE)
			{
				/* Send internal message to announcement handler */
				DAB_Tuner_Ctrl_Internal_Msg_To_Anno_State();
			}
			else
			{
				/* Transit to announcement state*/
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_announcement_state);
			}
		}
		break;
				
		case DAB_TUNER_CTRL_CANCEL_ANNOUNCEMENT_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
	 		ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u8_DiffChannelSubChId),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u8_DiffChannelClusterid),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));
			b_Anno_ongoingflag = FALSE;
			b_Announcement_Cancel_Flag = TRUE;
			DabTuner_MsgSndAnnouncementStop_cmd();
			/* Starting timer for AnnouncementSwitching_cmd time out */
			if(st_TimerId.u32_AnnouncementSwitching_Timer > 0)
			{
				if(SYS_StopTimer(st_TimerId.u32_AnnouncementSwitching_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT failed");	
				}
				else
				{
					st_TimerId.u32_AnnouncementSwitching_Timer = 0;
				}				
			}
			else
			{
				/* MISRA C*/	
			}
			st_TimerId.u32_AnnouncementSwitching_Timer = SYS_StartTimer(DAB_ANNOUNCEMENT_SWITCHING_CMD_TIMEOUT_TIME, ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_AnnouncementSwitching_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT");	
			}
			else
			{
				/*MISRA C*/	
			}
			
		}	
		break;
		
		case DAB_TUNER_CTRL_COMP_LIST_SORT_RESID:
		{
			/* Copy st_Dab_Tuner_Ctrl_CurrEnsembleProgList back to st_ProgrammeServListChanged_not after sorting */
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not, &st_Dab_Tuner_Ctrl_CurrEnsembleProgList,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
		    /* Updating Re-ConfigType as TUNER_CTRL_SERVICE_LIST_RECONFIG */
			DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_SERVICE_LIST_RECONFIG ;
			DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx = 0 ;
		
			(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId) ;
			/* Starting timer for GetComponentListReq_Cmd time out */
			if(st_TimerId.u32_Listen_Timer > 0)
			{
				if(SYS_StopTimer(st_TimerId.u32_Listen_Timer) == FALSE)
				{
		 			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_COMPONENTLIST_TIMEOUT failed");	
				}
				else
				{
					st_TimerId.u32_Listen_Timer = 0;
				}					
			}
			else
			{
			 /*MISRA C*/
			}
			st_TimerId.u32_Listen_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
			if(st_TimerId.u32_Listen_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
			}
			else
			{
				/*MISRA C*/	
			}			
		}	
		break;
		
		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}


/*========================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);                             */
/*========================================================================================================*/

Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
 Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
 Tbool b_resultcode = FALSE;
  switch(msg->msg_id)
  {
     case HSM_MSGID_ENTRY:
		{
		}
         break;

     case HSM_MSGID_START:
	 { 
		// DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr";
		DAB_Tuner_Ctrl_me->b_Linking_Handler = FALSE;
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr ");
		SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_AnnouncementHndlr"));
		
				  
		DAB_Tuner_Ctrl_me->b_Anno_Onstate = TRUE;
		b_Anno_ongoingflag = TRUE;
		DabTuner_MsgSndAnnouncementSwitching_cmd(DAB_Tuner_Ctrl_me->u8_Subchannelid);
		/* Starting timer for AnnouncementSwitching_cmd time out */
		if(st_TimerId.u32_AnnouncementSwitching_Timer > 0)
		{
			if(SYS_StopTimer(st_TimerId.u32_AnnouncementSwitching_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_AnnouncementSwitching_Timer = 0;
			}			
		}
		else
		{
			/* MISRA C*/	
		}
		st_TimerId.u32_AnnouncementSwitching_Timer = SYS_StartTimer(DAB_ANNOUNCEMENT_SWITCHING_CMD_TIMEOUT_TIME, ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
		if(st_TimerId.u32_AnnouncementSwitching_Timer == 0)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT");	
	  	}
		else
		{
		 	/*MISRA C*/	
		}
	}
    break;
	
	case ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT:
	{
		st_TimerId.u32_AnnouncementSwitching_Timer = 0; 
		/* Re-trigger AnnouncementSwitching_cmd */
		if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_ANNOUNCEMENT_SWITCHING_CMD_MAX_REPEAT_COUNT)
		{
			if((b_Announcement_Cancel_Flag == TRUE) || (b_Announcement_Stop_Flag == TRUE))
			{
				DabTuner_MsgSndAnnouncementStop_cmd();
			}
			else
			{
				DabTuner_MsgSndAnnouncementSwitching_cmd(DAB_Tuner_Ctrl_me->u8_Subchannelid);
			}
	
			/* Starting timer for AnnouncementSwitching_cmd time out */
			st_TimerId.u32_AnnouncementSwitching_Timer = SYS_StartTimer(DAB_ANNOUNCEMENT_SWITCHING_CMD_TIMEOUT_TIME, ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_AnnouncementSwitching_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT");	
			}
			else
			{
				/*MISRA C*/	
			}
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
		}
		else
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] AnnouncementSwitching_Cmd Timeout even after retriggering");
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(b_Announcement_Cancel_Flag == TRUE)
			{
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
				b_Announcement_Cancel_Flag = FALSE;
				DAB_Tuner_Ctrl_Response_CancelAnnouncement(DAB_Tuner_Ctrl_me->e_ReplyStatus);
			}
			else if(b_Announcement_Stop_Flag == TRUE)
			{
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
				b_Announcement_Stop_Flag = FALSE;

				/* Send stop announcement response to DAB APP */
				DAB_Tuner_Ctrl_Response_StopAnnouncement(DAB_Tuner_Ctrl_me->e_ReplyStatus, DAB_Tuner_Ctrl_me->e_tuned_announcement_type,DAB_Tuner_Ctrl_me->u8_Subchannelid);
			}
			else
			{
				/* AnnouncementSwitching Done*/
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
	
				DAB_Tuner_Ctrl_Response_StartAnnouncement(DAB_Tuner_Ctrl_me->e_ReplyStatus, DAB_Tuner_Ctrl_me->e_tuned_announcement_type);
			}
		}
	}
	break;

	case DAB_TUNER_CTRL_INTERNAL_ANNO_MSG:
	{
		b_Anno_ongoingflag = TRUE;
		DabTuner_MsgSndAnnouncementSwitching_cmd(DAB_Tuner_Ctrl_me->u8_Subchannelid);	
		/* Starting timer for AnnouncementSwitching_cmd time out */
		if(st_TimerId.u32_AnnouncementSwitching_Timer > 0)
		{
			if(SYS_StopTimer(st_TimerId.u32_AnnouncementSwitching_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_AnnouncementSwitching_Timer = 0;
			}			
		}
		else
		{
			/* MISRA C*/	
		}
		st_TimerId.u32_AnnouncementSwitching_Timer = SYS_StartTimer(DAB_ANNOUNCEMENT_SWITCHING_CMD_TIMEOUT_TIME, ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
		if(st_TimerId.u32_AnnouncementSwitching_Timer == 0)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT");	
		}
		else
		{
			/*MISRA C*/	
		}	
	}
	break;
	case DAB_TUNER_CTRL_ANNOUNCEMENT_SWITCHING_REPLY:
	{
			
		
		DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
		if(SYS_StopTimer(st_TimerId.u32_AnnouncementSwitching_Timer) == FALSE)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT failed");	
		}
		else
		{
			st_TimerId.u32_AnnouncementSwitching_Timer = 0;
		}		
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG,"[RADIO][DAB_TC] DAB_TUNER_CTRL: Announcement reply received ");
		b_resultcode = DabTuner_MsgRcvAnnouncementSwitching(&(DAB_Tuner_Ctrl_me->st_AnnouncementSwitching_reply),msg->data);
		if(b_resultcode == TRUE)
		{
			if(DAB_Tuner_Ctrl_me->st_AnnouncementSwitching_reply.reply == 0x00) /* Please add the macro here*/
			{
				if(b_Announcement_Cancel_Flag == TRUE)
				{
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					b_Announcement_Cancel_Flag = FALSE;
				
					if(DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent == TRUE)
					{
						DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent = FALSE;
					}						
					
						
					DAB_Tuner_Ctrl_Response_CancelAnnouncement(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					if((DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level != RELIABLE))
					{
						DAB_Tuner_Ctrl_me->b_Prepare_Blending = FALSE ;
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
					} 
				}
				else if(b_Announcement_Stop_Flag == TRUE)
				{
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					b_Announcement_Stop_Flag = FALSE;
			
					if(DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent == TRUE)
					{
						DAB_Tuner_Ctrl_me->b_AnnoSigLow_sent = FALSE;
					}
					/* Send stop announcement response to DAB APP */
					DAB_Tuner_Ctrl_Response_StopAnnouncement(DAB_Tuner_Ctrl_me->e_ReplyStatus, DAB_Tuner_Ctrl_me->e_tuned_announcement_type,DAB_Tuner_Ctrl_me->u8_Subchannelid);
					if((DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level != RELIABLE))
					{
						DAB_Tuner_Ctrl_me->b_Prepare_Blending = FALSE ;
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
					} 
				}
				else
				{
					/* AnnouncementSwitching Done*/
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
			
			
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_TC] DAB_TUNER_CTRL: Sending Announcement response to DAB APP ");
			
					DAB_Tuner_Ctrl_Response_StartAnnouncement(DAB_Tuner_Ctrl_me->e_ReplyStatus, DAB_Tuner_Ctrl_me->e_tuned_announcement_type);
			
					/* Update tuned announcement station information */
					DAB_Tuner_Ctrl_Update_CurrentTunedAnnoData(DAB_Tuner_Ctrl_me);
			
					/* Send tuned announcement station notification to DAB App */
					DAB_Tuner_Ctrl_Notify_AnnoStationInfo(DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo);	
				}
			} 
			else
			{
			/*	if(Announcement_Cancel_Flag == 1)
				{
					DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_CancelAnnoReplyStatus = DAB_TUNER_CTRL_CANCEL_ANNO_FAILURE;
					Announcement_Cancel_Flag = 0;
					e_Anno_ongoingflag = TRUE;
					DAB_Tuner_Ctrl_Response_CancelAnnouncement(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_CancelAnnoReplyStatus);
				}
				else if(Announcement_Stop_Flag == 1)
				{
					DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StopAnnoReplyStatus = DAB_TUNER_CTRL_STOP_ANNO_FAILURE;
					Announcement_Stop_Flag = 0;
					e_Anno_ongoingflag = TRUE;
					DAB_Tuner_Ctrl_Response_StopAnnouncement(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StopAnnoReplyStatus, DAB_Tuner_Ctrl_me->e_announcement_type,DAB_Tuner_Ctrl_me->u8_Subchannelid);
				}
				else	
				{
					DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StartAnnoReplyStatus = DAB_TUNER_CTRL_START_ANNO_FAILURE;
					e_Anno_ongoingflag = FALSE;
					DAB_Tuner_Ctrl_Response_StartAnnouncement(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StartAnnoReplyStatus, DAB_Tuner_Ctrl_me->e_announcement_type);
				}*/
			}
		}
	}
	break;
		
	case DAB_TUNER_CTRL_ANNOUNCEMENT_STOP_SWITCHING_REQID:
	{
		DAB_Tuner_Ctrl_me->Index = 0;
		ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_tuned_announcement_type),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&(DAB_Tuner_Ctrl_me->Index));	
		ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u8_Subchannelid),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));	
		b_Anno_ongoingflag = FALSE;
		DabTuner_MsgSndAnnouncementStop_cmd();
		/* Starting timer for AnnouncementSwitching_cmd time out */
		if(st_TimerId.u32_AnnouncementSwitching_Timer > 0)
		{
			if(SYS_StopTimer(st_TimerId.u32_AnnouncementSwitching_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_AnnouncementSwitching_Timer = 0;
			}			
		}
		else
		{
			/* MISRA C*/	
		}
		st_TimerId.u32_AnnouncementSwitching_Timer = SYS_StartTimer(DAB_ANNOUNCEMENT_SWITCHING_CMD_TIMEOUT_TIME, ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
		if(st_TimerId.u32_AnnouncementSwitching_Timer == 0)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for ANNOUNCEMENT_SWITCHING_REPLY_TIMEOUT");	
		}
		else
		{
			/*MISRA C*/	
		}
		b_Announcement_Stop_Flag = TRUE;
	}
	break;
			
	case HSM_MSGID_EXIT:
		 {
			 DAB_Tuner_Ctrl_me->b_Anno_Onstate = FALSE;
		 }
		 break;

    default:
		{
          /* in top state throw system error */
          ret = msg;
		}
      break;
  }
  
  return ret;
}
/*=======================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_LinkingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);  */
/*=======================================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_LinkingHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_LinkingHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_LinkingHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_LinkingHndlr"));
	
		}
		break;

		case DAB_TUNER_CTRL_SELSERV_REQID:
		case DAB_TUNER_CTRL_SCAN_REQID:
		case DAB_TUNER_CTRL_DESELBAND_REQID:
		{
			if(DAB_Tuner_Ctrl_me->u8_BlendingProcess == BLENDING_IN_PROGRESS)
			{
				SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->Blending_Process),(msg),sizeof(Ts_Sys_Msg));
				DabTuner_ResetBlending_cmd();
				/* Starting timer for ResetBlending_cmd time out */
				st_TimerId.u32_Linking_Timer = SYS_StartTimer(DAB_RESET_BLENDING_CMD_TIMEOUT_TIME, RESET_BLENDING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_Linking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for RESET_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
			}
			else if(DAB_Tuner_Ctrl_me->u8_BlendingProcess == STOP_BLENDING_IN_PROGRESS)
			{
				SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->Blending_Process),(msg),sizeof(Ts_Sys_Msg));
				DAB_Tuner_Ctrl_me->u8_BlendingProcess = NO_BLENDING;
				(void)SYS_SEND_MSG(&(DAB_Tuner_Ctrl_me->Blending_Process));
				memset(&(DAB_Tuner_Ctrl_me->Blending_Process),0,sizeof(Ts_Sys_Msg));
			}
			else if(DAB_Tuner_Ctrl_me->u8_BlendingProcess == NO_BLENDING)
			{
				ret = msg;
			}
			else
			{
				/*Do Nothing*/
			}
		}
		break;
		
		case RESET_BLENDING_REPLY_TIMEOUT:
		{
			st_TimerId.u32_Linking_Timer = 0;
			/* Re-trigger ResetBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_RESET_BLENDING_CMD_MAX_REPEAT_COUNT)
			{
				DabTuner_ResetBlending_cmd();
				/* Starting timer for ResetBlending_cmd time out */
				st_TimerId.u32_Linking_Timer = SYS_StartTimer(DAB_RESET_BLENDING_CMD_TIMEOUT_TIME, RESET_BLENDING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_Linking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for RESET_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] ResetBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				switch(DAB_Tuner_Ctrl_me->Blending_Process.msg_id)
				{
					case DAB_TUNER_CTRL_SELSERV_REQID:
					{
						/* Send select service failure response */
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_BLENDING_NO_SIGNAL,DAB_Tuner_Ctrl_me->requestedinfo);
					}
					break;
					
					case DAB_TUNER_CTRL_SCAN_REQID:
					{
						/* Send scan failure response */
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_NO_SIGNAL;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					}
					break;
					
					case DAB_TUNER_CTRL_DESELBAND_REQID:
					{
						/* Send de-select failure response */
						DAB_Tuner_Ctrl_Response_DeActivate(REPLYSTATUS_REQ_ERROR);
					}
					break;
					
					default:
					{
						/* Do nothing */
					}
					break;		
				}
			}
		}
		break;
		
		case RESET_BLENDING_REPLY:
		{
			DabTuner_ResetBlending_Reply(&(DAB_Tuner_Ctrl_me->st_ResetBlending_Reply),msg->data);
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_Linking_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for RESET_BLENDING_REPLY_TIMEOUT failed");	
			}
			else
			{
			 	st_TimerId.u32_Linking_Timer = 0;
			}			
			if(DAB_Tuner_Ctrl_me->st_ResetBlending_Reply.Reply == 0x00)
			{
				DAB_Tuner_Ctrl_me->u8_BlendingProcess = NO_BLENDING;
				if((DAB_Tuner_Ctrl_me->Blending_Process.msg_id == DAB_TUNER_CTRL_SELSERV_REQID) || (DAB_Tuner_Ctrl_me->Blending_Process.msg_id == DAB_TUNER_CTRL_SCAN_REQID) || (DAB_Tuner_Ctrl_me->Blending_Process.msg_id == DAB_TUNER_CTRL_DESELBAND_REQID))
				{
					(void)SYS_SEND_MSG(&(DAB_Tuner_Ctrl_me->Blending_Process));
					memset(&(DAB_Tuner_Ctrl_me->Blending_Process),0,sizeof(Ts_Sys_Msg));
				}
			}
			else
			{
				/* FOR MISRA */
			}
			
		}
		break;
			

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}

/*=======================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_DAB_FM_blendingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);  */
/*=======================================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_DAB_FM_blendingHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_result_code = FALSE;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_DAB_FM_blendingHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_DAB_FM_blendingHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_DAB_FM_blendingHndlr"));
			b_delayvalue = FALSE;
			if (b_delayvalue == FALSE)
			{
				//Added for MISRA - Todo: Check if this variable is required or not
			}
			DAB_Tuner_Ctrl_me->b_Linking_Handler = TRUE;
			DAB_Tuner_Ctrl_me->b_Anno_Onstate = FALSE;
			/* First time transition giving PrepareForBlending_cmd() */
			if((DAB_Tuner_Ctrl_me->b_Prepare_Blending != TRUE)&&(DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE))
			{
				if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Delay caluculation started for seamless");
					DabTuner_PrepareForBlending_cmd();
					/* Starting timer for PrepareForBlending_cmd time out */
					st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}	
			}
			/*	When DAB receives best PI notification again */
			else if((DAB_Tuner_Ctrl_me->Best_PI_notification > 1) && (DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_NORMAL))/*Once best PI is received checking FM is NORMAL if it is NORMAL only calling prepare for blending command*/
			{
				if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
				{
					DabTuner_PrepareForBlending_cmd();
					DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
					/* Starting timer for PrepareForBlending_cmd time out */
					st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}				
				}	
			}
			else if(DAB_Tuner_Ctrl_me->Start_Blending == TRUE && DAB_Tuner_Ctrl_me->b_Prepare_Blending == TRUE && DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x01)
			{
				if(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality > DAB_FM_LINKING_THRESHOULD)
				{
					//DAB_Tuner_Ctrl_me->Linking = TRUE;
					if(DAB_Tuner_Ctrl_me->b_Linking == TRUE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Blending command given ");
						DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
						/* Starting timer for DabTuner_StartBlending_cmd time out */
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;
							}						
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
						DAB_Tuner_Ctrl_me->b_Linking = FALSE;
					}	
				}
				else
				{
					DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
					b_handlerchanged = TRUE;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Moving to DAB-DAB BEST PI quality not good");
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
				}
			}
			else
			{
				
			}
		}
		break;
		
		case PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT:
		{
			st_TimerId.u32_DAB_FM_blending_Timer = 0;
			/* Re-trigger PrepareForBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_PREPARE_FOR_BLENDING_CMD_MAX_REPEAT_COUNT)
			{
				if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
				{
					DabTuner_PrepareForBlending_cmd();
					/* Starting timer for PrepareForBlending_cmd time out */
					st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}	
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] PrepareForBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* NEED TO UPDATE TIMEOUT RESPONSE */
				if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
				{
					DabTuner_PrepareForBlending_cmd();
					/* Starting timer for PrepareForBlending_cmd time out */
					st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}	
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
		}
		break;
		

		case PREPARE_FOR_BLENDING_REPLY:
		{
			DabTuner_PrepareForBlending_Reply(&(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Reply),msg->data);
			if((DAB_Tuner_Ctrl_me->st_PrepareForBlending_Reply.Reply == 0x05)&&(DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level == UN_RELIABLE))
			{	
				if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
				{
					if(SYS_StopTimer(st_TimerId.u32_DAB_FM_blending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT failed");	
					}
					else
					{
			 			st_TimerId.u32_DAB_FM_blending_Timer = 0;
					}					
					DabTuner_PrepareForBlending_cmd();
					/* Starting timer for PrepareForBlending_cmd time out */
					st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
			}
		}
		break;


		case PREPARE_FOR_BLENDING_NOTIFICATION:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
			{
				if(SYS_StopTimer(st_TimerId.u32_DAB_FM_blending_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT failed");	
				}
				else
				{
			 		st_TimerId.u32_DAB_FM_blending_Timer = 0;
				}					
			}
			b_result_code = DabTuner_PrepareForBlending_Notify(&(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify), (Ts8*)(msg->data));
			if(b_result_code == TRUE)
			{
				if((DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level == UN_RELIABLE)&&(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.DelayFound == TRUE))
				{
					DAB_Tuner_Ctrl_CheckDelayValues(DAB_Tuner_Ctrl_me);
					//DAB_Tuner_Ctrl_Check_Leveldata(DAB_Tuner_Ctrl_me);
				}
			
				/* Checking whether AMFM TUNER is NORMAL, if so than calling prepare for blending */
				if((DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level == UN_RELIABLE) && (DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_NORMAL))
				{
					if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
					{
						DabTuner_PrepareForBlending_cmd();
						/* Starting timer for PrepareForBlending_cmd time out */
						st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
					}	
				}
				else	
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Time allignment command for allignment");
					DabTuner_StartTimeAlignmentForBlending_cmd(DAB_Tuner_Ctrl_me->Blending_delay_To_Chk_Allignment);
				
					/* Starting timer for StartTimeAlignmentForBlending_cmd time out */
					st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_START_TIME_ALLIGNMENT_CMD_TIMEOUT_TIME, START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
			}
		}
		break;
		
		case START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT:
		{
			st_TimerId.u32_DAB_FM_blending_Timer = 0;
			/* Re-trigger StartTimeAlignmentForBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_START_TIME_ALLIGNMENT_CMD_MAX_REPEAT_COUNT)
			{
				DabTuner_StartTimeAlignmentForBlending_cmd(DAB_Tuner_Ctrl_me->Blending_delay_To_Chk_Allignment);
				/* Starting timer for StartTimeAlignmentForBlending_cmd time out */
				st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_START_TIME_ALLIGNMENT_CMD_TIMEOUT_TIME, START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] StartTimeAlignmentForBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			}
		}
		break;

		case START_TIME_ALLIGNMENT_REPLY:
		{
			DabTuner_StartTimeAlignmentForBlending_Reply(&(DAB_Tuner_Ctrl_me->st_StartTimeAlignmentForBlending_repl),msg->data);
			if(DAB_Tuner_Ctrl_me->st_StartTimeAlignmentForBlending_repl.ReplyStatus == 0x05)
			{
				DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay = 0;
				DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level = UN_RELIABLE;
				if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
				{
					DAB_Tuner_Ctrl_me->Level_data_count = 0;
					DabTuner_PrepareForBlending_cmd();
					/* Starting timer for PrepareForBlending_cmd time out */
					st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
			}	
		}
		break;

		case START_TIME_ALLIGNMENT_NOTIFICATION:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_DAB_FM_blending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_TIME_ALLIGNMENT_NOTIFICATION_TIMEOUT failed");	
			}
			else
			{
			 	st_TimerId.u32_DAB_FM_blending_Timer = 0;
			}						
			b_result_code = DabTuner_TimeAlignmentForBlending_Notify(&(DAB_Tuner_Ctrl_me->st_Start_Timeallignment_notify), (Ts8*)(msg->data));
			if(b_result_code == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_Start_Timeallignment_notify.Done == 0x01)
				{
					DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay = DAB_Tuner_Ctrl_me->Blending_delay_To_Chk_Allignment;
					DAB_Tuner_Ctrl_me->b_Prepare_Blending = TRUE;
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Time allignment success");
					DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLENDING_INFO);
					//Set_Drifttracking_Cmd();
					/* If AMFM TUNER is NORMAL only we should blend to FM
					if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x02)&& ((DAB_Tuner_Ctrl_me->u8_SettingStatus & 0x02) != 0) && (DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == DAB_TUNER_CTRL_COMP_STATUS_NORMAL))
					{
						DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
					    Starting timer for DabTuner_StartBlending_cmd time out 
						(void)SYS_StartTimer(4, DAB_START_BLENDING_CMD_TIMEOUT_TIME, START_BLENDING_REPLY_TIMEOUT);
						DAB_Tuner_Ctrl_me->Linking = FALSE;
					}*/
				}
				else
				{
					DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay = 0;
					DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level = UN_RELIABLE;
					if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
					{
						DabTuner_PrepareForBlending_cmd();
						/* Starting timer for PrepareForBlending_cmd time out */
						st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
					}
				}
			}
		}
		break;
		
		
		case SET_DRIFTTRACKING_REPLY:
		{
			 DabTuner_Set_Drift_Tracking_Reply(&(DAB_Tuner_Ctrl_me->st_Set_DriftTracking_Reply),msg->data);
		}
		break;

		case DAB_TUNER_CTRL_DRIFT_TRACKING_NOTIFICATION:
		{
			DabTuner_Drift_Tracking_Not(&(DAB_Tuner_Ctrl_me->st_Drift_Tracking_Notification),msg->data);
			if((DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level == RELIABLE) || (DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level == PROBABLE))
			{
				if(DAB_Tuner_Ctrl_me->st_Drift_Tracking_Notification.DelayFound == 0x01)
				{	
					DAB_Tuner_Ctrl_me->BlendingDelay_differene = DAB_Tuner_Ctrl_CaluculateDelayValues (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay, DAB_Tuner_Ctrl_me->st_Drift_Tracking_Notification.Delay);
					if(DAB_Tuner_Ctrl_me->BlendingDelay_differene > PREPARE_BLENDING_DELTA_DELAY)
					{
						if(DAB_Tuner_Ctrl_me->st_Drift_Tracking_Notification.Delay > 0)
						{
							DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay = DAB_Tuner_Ctrl_me->st_Drift_Tracking_Notification.Delay;
							DabTuner_StartTimeAlignmentForBlending_cmd(DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay);
						}
					}
					else
					{
					
					}
			  	}
				if(DAB_Tuner_Ctrl_me->st_Drift_Tracking_Notification.LevelStatus == 0x01)
				{
					DAB_Tuner_Ctrl_Check_Leveldata(DAB_Tuner_Ctrl_me);
				}
				
			}

		}
		break;
		
		
		case START_BLENDING_REPLY_TIMEOUT:
		{
			st_TimerId.u32_StartBlending_Timer  = 0;
			/* Re-trigger DabTuner_StartBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_START_BLENDING_CMD_MAX_REPEAT_COUNT)
			{
				DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] StartBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* NEED TO UPDATE TIMEOUT RESPONSE */
				DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				if(st_TimerId.u32_StartBlending_Timer > 0)
				{
					if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
					}
					else
					{
						st_TimerId.u32_StartBlending_Timer = 0;
					}					
				}
				else
				{
					/* MISRA C*/	
				}
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
		}
		break;		

		case START_BLENDING_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_StartBlending_Timer = 0;
			}				
			if(DAB_Tuner_Ctrl_me->b_Linking != TRUE)
			{
				DabTuner_StartBlending_Reply(&(DAB_Tuner_Ctrl_me->st_Start_Blending_Reply),msg->data);
				DAB_Tuner_Ctrl_me->Start_Blending =RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS;
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS;
				DAB_Tuner_Ctrl_me->u8_BlendingProcess = BLENDING_IN_PROGRESS;    /*for linking*/
				DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
				DAB_Tuner_Ctrl_me->b_Linking = TRUE;      /*added newly for link ater seek*/
				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Blended to FM");
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLENDED_TO_FM);
				DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x50);
			}	
		}
		break;

		case STATUS_CHECKING_MSG:
		{
			if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_RESUME_BACK)
			{	  	
				if(DAB_Tuner_Ctrl_me->b_Linking_Handler != FALSE)
				{
					DAB_Tuner_Ctrl_me->b_Linking_Handler = FALSE;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state);
				}
			}

			/* Checking whether AMFM TUNER is NORMAL once best PI is recei*/
			else if((DAB_Tuner_Ctrl_me->Best_PI_notification > 1) && (DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_NORMAL))
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Dealy caluculation started fro seamless 1 ");
				DabTuner_PrepareForBlending_cmd();
				DAB_Tuner_Ctrl_me->Best_PI_notification = 1;
				/* Starting timer for PrepareForBlending_cmd time out */
				st_TimerId.u32_DAB_FM_blending_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DAB_FM_blending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}				
			}
			else if(DAB_Tuner_Ctrl_me->Start_Blending == TRUE && DAB_Tuner_Ctrl_me->b_Prepare_Blending == TRUE && DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus >= 0x01)
			{
				if(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality > DAB_FM_LINKING_THRESHOULD)
				{
					if(DAB_Tuner_Ctrl_me->b_Linking == TRUE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Blended to FM ");
						DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
						/* Starting timer for DabTuner_StartBlending_cmd time out */
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;	
							}
							
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(100,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
						DAB_Tuner_Ctrl_me->b_Linking = FALSE;
					}	
				}
				else
				{
					b_handlerchanged = TRUE;
					DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
					DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
				}
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Blending not happened ");
			}
		}
		break;

		case DAB_TUNER_CTRL_PI_QUALITY_NOTIFYID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));		
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLENDING_INFO);		
			if(DAB_Tuner_Ctrl_me->Start_Blending == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
			{
				if((DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality < DAB_FM_LINKING_MIN_THRESHOULD))// || (DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD) && (DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01) && (DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == DAB_TUNER_CTRL_COMP_STATUS_NORMAL))
				{			
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
					DAB_Tuner_Ctrl_me->e_BestPI_Type = DAB_TUNER_CTRL_NO_IMPLICIT;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_blending_state);
				}
			}
		}
		break;

		case HSM_MSGID_EXIT:
		{
			DAB_Tuner_Ctrl_me->b_Linking_Handler = FALSE;
			
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}


/*==========================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);     */
/*==========================================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr ");
			// DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_FM_DAB_blendingHndlr"));
			DAB_Tuner_Ctrl_me->b_Anno_Onstate = FALSE;
			DAB_Tuner_Ctrl_me->b_Linking_Handler = FALSE;
			/*check*/
			if((DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality < DAB_FM_LINKING_MAX_THRESHOULD) || ((DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality > DAB_FM_LINKING_THRESHOULD)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x00)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQualityvalidflag == 0x01)))
			{
				DabTuner_StartBlending_cmd((Tu8)0x50,DAB_Tuner_Ctrl_me->u16_LevelData);
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				if(st_TimerId.u32_StartBlending_Timer > 0)
				{
					if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
					}
					else
					{
						st_TimerId.u32_StartBlending_Timer = 0;
					}						
				}
				else
				{
					/* MISRA C*/	
				}
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}				
			}
		}
		break;
		
		case START_BLENDING_REPLY_TIMEOUT:
		{
			st_TimerId.u32_StartBlending_Timer  = 0;
			/* Re-trigger DabTuner_StartBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_START_BLENDING_CMD_MAX_REPEAT_COUNT)
			{
				DabTuner_StartBlending_cmd((Tu8)0x50,DAB_Tuner_Ctrl_me->u16_LevelData);
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] StartBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* NEED TO UPDATE TIMEOUT RESPONSE */
				DabTuner_StartBlending_cmd((Tu8)0x50,DAB_Tuner_Ctrl_me->u16_LevelData);
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				if(st_TimerId.u32_StartBlending_Timer > 0)
				{
					if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
					}
					else
					{
						st_TimerId.u32_StartBlending_Timer = 0;
					}						
				}
				else
				{
						/* MISRA C*/	
				}
		    	st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
		}
		break;		

		case START_BLENDING_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_StartBlending_Timer = 0;
			}			
			DabTuner_StartBlending_Reply(&(DAB_Tuner_Ctrl_me->st_Start_Blending_Reply),msg->data);
			DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
			DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS; /*for linking*/
			DAB_Tuner_Ctrl_me->b_Linking = TRUE;
			DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLEND_BACK_TO_DAB);	
			if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] FM singal is low blend back to DAB ");
				b_handlerchanged = TRUE;
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED;
				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData); /* when DAB signal resumes back then, sending the current ensemble data to upperlayer*/
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
			}
			else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_DISABLED) /*Once Linking setting is disabled, after linking to FM*/
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Blending to FM is suspended, due to linking settings disabled ");
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED;	/* assigning linking status as FM blending has been suspended*/
				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);/* sending select service success to upper layer, returning to DAB*/
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
				b_handlerchanged = TRUE;
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state); /*Going to DAB-DAB for tuning to its alternate frequency in DAB*/
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DAB Signal Resumes back ");
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED;
				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData); /* when DAB signal resumes back in hardlink, sending the current ensemble data to upperlayer*/
				DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x51);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
			}	
		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}



/*===========================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_DAB_DAB_blendingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);     */
/*===========================================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_DAB_DAB_blendingHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Tbool b_resultcode;
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu8 DAB_Tuner_Ctrl_Best_PI_Check_Index = (Tu8) 0;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_DAB_DAB_blendingHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_DAB_DAB_blendingHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_DAB_DAB_blendingHndlr"));
			SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			DAB_Tuner_Ctrl_me->b_Anno_Onstate = FALSE;
			DAB_Tuner_Ctrl_me->b_Linking_Handler = FALSE;
			b_handlerchanged = TRUE;
			AltEnsembleindex		= 0;
			b_Hardlinksused 		= FALSE;
			Hardlinkindex			= 0;
			Altfreqindex			= 0;
			b_Tune_to_Orginal_freq_Check	=	FALSE;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
			DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_NORMAL_PLAYBACK))
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] DAB-DAB AF Search started");
				DAB_Tuner_Ctrl_Notify_DAB_DAB_Status(DAB_TUNER_CTRL_DAB_ALT_TUNE_STARTED);
				DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(0x00);
				DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}	
		}
		break;
#if 0
		case DAB_TUNER_CTRL_CANCEL_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_CancelType),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_CancelType),&(DAB_Tuner_Ctrl_me->Index));
			if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_AF_TUNE_CANCEL)
			{
				DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID ;
				/* No need to clear currentensemble data structure, even if the AF tune(i.e., Select service Req in DAB Tuner Ctrl) is SUCCESS and
					sends "SelectService response" to DAB App, DAB APP won't handle this response in Idle state */
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
				DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
			}
		}
		break;
#endif
		case DAB_TUNER_CTRL_TUNE_SIGNAL_PRESENT:
		{
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
				if(DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted != FALSE)
				{
					/* Stopping TUNE_TIME_OUT timer */
					if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
					}
					else
					{
						st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = 0;	
					}
					b_resultcode = DabTuner_MsgRcvSetTuneStatusNot(&(DAB_Tuner_Ctrl_me->st_tuneStatus),msg->data);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] signal present came for freq is %d",DAB_Tuner_Ctrl_me->st_tuneStatus.Frequency);
					if(b_resultcode == TRUE)
					{
						/* Starting the timer for DAB_TUNER_CTRL_TUNE_MCI_PRESENT */
						st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(1800, DAB_TUNER_CTRL_TUNE_MCI_PRESENT, RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
						}
						else
						{
							/*MISRA C*/	
						}
					}
				}
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] MCI timer started");
				/* Starting the timer for DAB_TUNER_CTRL_TUNE_MCI_PRESENT */
				st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(1800, DAB_TUNER_CTRL_TUNE_MCI_PRESENT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
				}
				else
				{
					/*MISRA C*/	
				}
			}
		}
		break;
		case RSSI_NOTIFICATION:
		{
			b_resultcode = DabTuner_MsgRcvRSSI_notify(&(DAB_Tuner_Ctrl_me->st_RSSI_notification),msg->data);
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted == FALSE)
				{
					/* Stopping TUNE_TIME_OUT timer */
					if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
					}
					else
					{
					 	st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = 0;	
					}
					if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_TUNED)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[Altfreqindex-1] = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						DAB_Tuner_Ctrl_me->st_tuneStatus.DABSignalPresent = 0;
						DabTuner_MsgSndSetRSSINotifier_cmd(0x00);
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
					else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_SORTED)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[Altfreqindex-1] = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						DabTuner_MsgSndSetRSSINotifier(0x00);
						if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[Altfreqindex-1] > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
						{
							DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTFREQ_BER_SORTED;
							Altfreqindex--;
						}
						else
						{
							Altfreqindex = MAX_ALT_FREQUENCY;
						}
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);

					}
					else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_TUNED)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex -1].Freq_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						DAB_Tuner_Ctrl_me->st_tuneStatus.DABSignalPresent = 0;
						DabTuner_MsgSndSetRSSINotifier_cmd(0x00);
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
					else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_SORTED)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex -1].Freq_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						DabTuner_MsgSndSetRSSINotifier(0x00);
						if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex -1].Freq_RSSI > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
						{
							DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTEID_BER_SORTED;
							AltEnsembleindex--;
						}
						else
						{
							AltEnsembleindex = MAX_ALT_EID;
						}
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);

					}
					else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_TUNED )
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex-1].EId_Data.Freq_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						DAB_Tuner_Ctrl_me->st_tuneStatus.DABSignalPresent = 0;
						DabTuner_MsgSndSetRSSINotifier_cmd(0x00);
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
					else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_SORTED)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex-1].EId_Data.Freq_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						DabTuner_MsgSndSetRSSINotifier(0x00);
						if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex-1].EId_Data.Freq_RSSI > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
						{
							DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_HARDLINK_FREQ_BER_SORTED;
							Hardlinkindex--;
						}
						else
						{
							Hardlinkindex = MAX_HARDLINK_SID;
						}
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);

					}
					else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_BEFORE_CHECK_ORIGINAL_TUNED)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						DAB_Tuner_Ctrl_me->st_tuneStatus.DABSignalPresent = 0;
						DabTuner_MsgSndSetRSSINotifier(0x00);
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
					else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_AFTER_CHECK_ORIGINAL_TUNED)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
						if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI > ESD_THRESHOLD && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI !=0 && DAB_Tuner_Ctrl_me->e_RequestCmd != DAB_TUNER_CTRL_AFTUNE && DAB_Tuner_Ctrl_me->e_RequestCmd != DAB_TUNER_CTRL_AFTUNE_END)
						{
							DabTuner_MsgSndSetRSSINotifier(0x00);
							DabTunerCtrl_GetSynchronisationState_req();
							/* Starting the timer for TUNE_TIME_OUT */
							st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
							if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
							}
							else
							{
								/*MISRA C*/	
							}
						}
						else
						{
							DabTuner_MsgSndSetRSSINotifier_cmd(0x00);
							DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
						}
					}
					else
					{
						DabTuner_MsgSndSetRSSINotifier_cmd(0x00);
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
				}
				else
				{
					DabTuner_MsgSndSetRSSINotifier_cmd(0x00);	
				}	
			}
		}	
		break;
#if 0
		case BER_STATUS_NOTIFICATION:
		{
			resultcode=DabTuner_MsgRcvSetPeriodicalBERQualityNotifier(&(DAB_Tuner_Ctrl_me->st_BER_notification),msg->data);
			if(resultcode==1 && DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted == FALSE)
			{
				
				if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_SORTED) 
				{
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Best_BER = DAB_Tuner_Ctrl_me->st_BER_notification.FIC_BER_Exponent;
					if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Best_BER < BER_LINKING_THRESHOULD)
					{
						DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTFREQ_BER_SORTED;
						Altfreqindex--;
					}
					else
					{
						Altfreqindex = MAX_ALT_FREQUENCY;
					}
					//DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}
				else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_SORTED)
				{
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Best_BER = DAB_Tuner_Ctrl_me->st_BER_notification.FIC_BER_Exponent;
					if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Best_BER < BER_LINKING_THRESHOULD)
					{
						DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTEID_BER_SORTED;
						AltEnsembleindex--;
					}
					else
					{
						AltEnsembleindex = MAX_ALT_EID;
					}
					//DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}
				else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_SORTED)
				{
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Best_BER = DAB_Tuner_Ctrl_me->st_BER_notification.FIC_BER_Exponent;
					if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Best_BER < BER_LINKING_THRESHOULD)
					{
						DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_HARDLINK_FREQ_BER_SORTED;
						Hardlinkindex--;
					}
					else
					{
						Hardlinkindex = MAX_HARDLINK_SID;
					}
					//DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);					
				}
				else
				{
					
					
				}	
			}
		}
		break;
#endif
		case DAB_TUNER_CTRL_GET_SYNC_STATE_REPLY :
		{
			 
			(void)DabTunerCtrl_GetSynchronisationState_repl(&(DAB_Tuner_Ctrl_me->st_GetSynchronisationState_repl), (Ts8*)(msg->data));
			if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_BEFORE_CHECK_ORIGINAL_TUNED )
			{
				/* Stopping TUNE_TIME_OUT timer */
				if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
				}
				else
				{
				 	st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = 0;	
				}

				if(DAB_Tuner_Ctrl_me->st_GetSynchronisationState_repl.ChannelSync != 1 && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI > ESD_THRESHOLD && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI !=0 && DAB_Tuner_Ctrl_me->e_RequestCmd != DAB_TUNER_CTRL_AFTUNE && DAB_Tuner_Ctrl_me->e_RequestCmd != DAB_TUNER_CTRL_AFTUNE_END)
				{
					DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_ESD_CHECK;
					DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);
					DAB_Tuner_Ctrl_Transition_to_Scanstate(DAB_Tuner_Ctrl_me);
				}
				else
				{
					DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}
			}
		}
		break;			
				
		
		case DAB_TUNER_CTRL_TUNE_MCI_PRESENT:
		{
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
				if(DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted != FALSE)
				{
					/* Clearing the e_Bool_ReConfigurationFlag (This flag is set 6 seconds before ReConfiguration) as DABTuner is giving 
					PROGRAMME_SERV_LIST_CHANGED_NOTIFICATION after tuning to new frequency even ReConfiguration has not happend,
					now after getting this notification ReConfiguration is decided based on this flag which is set when ever 
					ChangeFlag in FIG 0/0 is other then 0 */
					DAB_Tuner_Ctrl_me->b_ReConfigurationFlag = FALSE;
					/* Stopping the timer for DAB_TUNER_CTRL_TUNE_MCI_PRESENT */
					if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
					}
					else
					{
					 	st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = 0;
					}			
					if(b_Hardlinksused == FALSE)
					{
						(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Select Service Cmd sent to DAB Tuner %d",DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
					}
					else
					{
						(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex - 1].SId);
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC]DAB-DAB: Select Service Cmd sent to DAB Tuner");
					}
					/* Starting the timer for SELECT_SERVICE_TIMEOUT */
					st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME, SELECT_SERVICE_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}	
			}
			else
			{
				if(b_Hardlinksused == FALSE)
				{
					(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Select Service Cmd sent to DAB Tuner %d",DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
				}
				else
				{
					(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex - 1].SId);
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC]DAB-DAB: Select Service Cmd sent to DAB Tuner");
				}			 
			}	
		}
		break;

		case TUNE_TIME_OUT:
		{
			st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = 0;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB-DAB: Tune_to_Cmd Timeout ");
			if(DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count > 2)
				{
					DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}
				else
				{
					/* Starting the timer for TUNE_TIME_OUT */
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(150, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				}
			}
			else
			{
				DabTuner_MsgSndSetPeriodicalBERQualityNotifier(0x00);
				DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}		
		}
		break;
		
/****************************************************************************************************************************************************/
#if 0	
		case DAB_TUNER_CTRL_TUNE_STATUS_NOTIFICATION :
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
				if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
				}	
				else
				{
			 		st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = 0;
				}					 
				DAB_Tuner_Ctrl_Notify_FrequencyChange(DAB_Tuner_Ctrl_me->st_tuneStatus.Frequency);
			}	
			
			/* Clearing the b_ReConfigurationFlag (This flag is set 6 seconds before ReConfiguration) as DABTuner is giving
				PROGRAMME_SERV_LIST_CHANGED_NOTIFICATION after tuning to new frequency even ReConfiguration has not happend,
				now after getting this notification ReConfiguration is decided based on this flag which is set when ever 
				ChangeFlag in FIG 0/0 is other then 0 */
			DAB_Tuner_Ctrl_me->b_ReConfigurationFlag = FALSE ;
			
			b_resultcode=DabTuner_MsgRcvSetTuneStatusNot(&(DAB_Tuner_Ctrl_me->st_tuneStatus),msg->data);
			
			if(b_resultcode == TRUE)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_tuneStatus.Frequency;
				if(b_Hardlinksused == FALSE)
				{
					(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Select Service Cmd sent to DAB Tuner");
				}
				else
				{
					(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex-1].SId);
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC]DAB-DAB: Select Service Cmd sent to DAB Tuner");
				}	
				if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
				{
					/* Starting timer for SelectService_repl time out */
					st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME, SELECT_SERVICE_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_SERVICE_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}					
				}
				else
				{
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_TUNE_FAILURE:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
				if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
				}
				else
				{
				 	st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = 0;
				}			
			}
			b_resultcode = DabTuner_MsgRcvSetTuneStatusNot(&(DAB_Tuner_Ctrl_me->st_tuneStatus),msg->data);
			if(b_resultcode == TRUE)
			{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] DAB-DAB: Tune Failure");
			//	DAB_Tuner_Ctrl_Notify_FrequencyChange(DAB_Tuner_Ctrl_me->st_tuneStatus.Frequency);
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}
		}
		break;

#endif
/****************************************************************************************************************************************************/
		case  SELECT_SERVICE_TIMEOUT :
		{
			st_TimerId.u32_DABDABBlending_Timer = 0;
			/* Re-trigger SelectService_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_SERVICE_CMD_MAX_REPEAT_COUNT)
			{
				if(b_Hardlinksused == FALSE)
				{
					(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Select Service Cmd sent to DAB Tuner");
				}
				else
				{
					(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex - 1].SId);
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC]DAB-DAB: Select Service Cmd sent to DAB Tuner");
				}	
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				/* Starting timer for SelectService_repl time out */
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME, SELECT_SERVICE_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_SERVICE_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}					
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectService_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}
		}
		break;

		case DAB_TUNER_CTRL_SELECT_SERVICE_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for SelectService_repl time out */
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{	
				if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for SELECT_SERVICE_TIMEOUT failed");	
				}
				else
				{
				 	st_TimerId.u32_DABDABBlending_Timer = 0;
				}
			}			
			b_resultcode = DabTuner_MsgRcvSelectServiceReply(&(DAB_Tuner_Ctrl_me->st_selectServiceReply),msg->data);
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus==0)
				{
					b_Anno_ongoingflag = FALSE;

					if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI != DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC]DAB-DAB:Component ID Not matched");
					}
					if(b_Hardlinksused == FALSE)
					{
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId;
						//DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI,DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
						(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC]DAB-DAB: Select Component Cmd sent to DAB Tuner");
					}
					else
					{
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId;
						(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId, DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex - 1].SId);
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC]DAB-DAB: Select Component Cmd sent to DAB Tuner");
					}
					if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
					{
						/* Starting timer for SelectComponent_repl time out */
						st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_SELECT_COMPONENT_CMD_TIMEOUT_TIME, SELECT_COMPONENT_TIMEOUT, RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_DABDABBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_COMPONENT_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}	
					}	
				}
				else
				{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB-DAB: Service is not Present in Tuned Frequency");
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}	
			}
			else
			{
				/*MISRA C*/
			}
		}
		break;		

		case SELECT_COMPONENT_TIMEOUT:
		{
			st_TimerId.u32_DABDABBlending_Timer = 0;
			/* Re-trigger SelectComponent_repl */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_COMPONENT_CMD_MAX_REPEAT_COUNT)
			{
				if(b_Hardlinksused == FALSE)
				{
					(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
				}
				else
				{
					(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId, DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex - 1].SId);
				}
				/* Starting timer for SelectComponent_repl time out */
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_SELECT_COMPONENT_CMD_TIMEOUT_TIME, SELECT_COMPONENT_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_COMPONENT_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectComponent_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}
					
		}
		break;
		
		case DAB_TUNER_CTRL_SELECT_COMPONENT_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for SelectComponent_repl time out */
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
			{
				if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for SELECT_COMPONENT_TIMEOUT failed");	
				}
				else
				{
				 	st_TimerId.u32_DABDABBlending_Timer = 0;
				}				
				DabTuner_MsgReply_SelectComponent(&(DAB_Tuner_Ctrl_me->st_selectcompreply),msg->data);
				if(DAB_Tuner_Ctrl_me->st_selectcompreply.reply == 0)
				{
					//AudioStatus.DecodingStatus = 0x02;
					//DabTuner_MsgSndSetAudioStatusNotifier_cmd(0x01);
					DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(0x03);
					if(SYS_StartTimer(400, AUDIO_TIMER_NOTIFICATION, RADIO_DAB_TUNER_CTRL) <= 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for AUDIO_TIMER_NOTIFICATION");	
					}
					else
					{
						/*MISRA C*/	
					}
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Select Component Reply received");
				}
				else
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Component is not present in Tuned Ensemble");
					DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);	
				}
			}			
		}
		break;

		case AUDIO_TIMER_NOTIFICATION:
		{

			if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus <=1 || DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent < BER_LINKING_THRESHOULD)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Audio Staus Notification Success");
				if((DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == FALSE) &&((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT) || (DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_PI_REQUEST_SENT)))
			   	{
					if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for AUDIO_TIMER_NOTIFICATION failed");	
					}
					else
					{
					 	st_TimerId.u32_DABDABBlending_Timer = 0;
					}					
					DAB_Tuner_Ctrl_me->b_Timer_start = FALSE ;
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;
			   	}
				//AltEnsembleindex		= 0;
				//Altfreqindex			= 0;   /*Comment not in new dab-dab code*/
				if(b_Hardlinksused == TRUE)
				{
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId =	DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex-1].SId ;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId;
					b_Hardlinksused = FALSE;
				}	
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Ensemble Properties Cmd sent to DAB Tuner");
				DAB_Tuner_Ctrl_ClearingAnnoDatabases(DAB_Tuner_Ctrl_me);
				(void)DabTuner_MsgSndGetEnsembleProperties();	
				/* Starting timer for GetEnsembleProperties_req time out */
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_ENSEMBLE_PROPERTIES_CMD_TIMEOUT_TIME, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				//Hardlinkindex			= 0;
			}
			else
			{
				if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < GET_AUDIO_STATUS_REQ_MAX_TIME_RETRIGGER)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Audio Staus Notification Retrigger");
					if(SYS_StartTimer(200, AUDIO_TIMER_NOTIFICATION, RADIO_DAB_TUNER_CTRL) <= 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for AUDIO_TIMER_NOTIFICATION");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				}
				else
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GET_AUDIO_STATUS_REPLY time out even after retriggering");
					DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}	
			}

		}
		break;		
#if 0
		case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT:
		{
			st_TimerId.u32_DABDABBlending_Timer = 0;
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_COMPONENT_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTunerCtrl_GetAudioStatus_req(); 	
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GET_AUDIO_STATUS_REPLY time out even after retriggering");
					DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}
		}
		break;
		case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY:
		{
			if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT failed");	
			}
			else
			{
			 	st_TimerId.u32_DABDABBlending_Timer = 0;
			}			
			b_resultcode = DabTunerCtrl_GetAudioStatus_repl(&(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl),msg->data);
			if(b_resultcode == TRUE)
			{
				if((DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.DecodingStatus != (Tu8)(0))||(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.AudioSynchronised != (Tu8)(1)))
				{
					if(DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger < (Tu8)GET_AUDIO_STATUS_REQ_MAX_TIME_RETRIGGER)
					{
						(void)DabTunerCtrl_GetAudioStatus_req(); 	
						st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT, RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_DABDABBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT");	
						}
						else
						{
							/*MISRA C*/	
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Decoding Status Not received as '0' even after retriggering");
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
				}
				else
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Audio Staus Notification Success");
					if((DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == FALSE) &&((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT) || (DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_PI_REQUEST_SENT)))
					{
						if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT failed");	
						}
						else
						{
							st_TimerId.u32_DABDABBlending_Timer = 0;
						}						
						DAB_Tuner_Ctrl_me->b_Timer_start = FALSE ;
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_RESUME_BACK;
					}
					AltEnsembleFreqindex 	= 0;
					AltEnsembleindex		= 0;
					HardlinkFreqindex 		= 0;
					HardlinkEnseindex 		= 0;
					Altfreqindex			= 0;   /*Comment not in new dab-dab code*/
					if(b_Hardlinksused == TRUE)
					{
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId =	DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex].SId ;
						b_Hardlinksused = FALSE;
					}	
					(void)DabTuner_MsgSndGetEnsembleProperties();	
					/* Starting timer for GetEnsembleProperties_req time out */
					st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_ENSEMBLE_PROPERTIES_CMD_TIMEOUT_TIME, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					Hardlinkindex = 0;					
				}
			}
		}
		break;		
#endif	
		case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:
		{
			st_TimerId.u32_DABDABBlending_Timer = 0;
			/* Re-trigger GetEnsembleProperties_req */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTuner_MsgSndGetEnsembleProperties();
				/* Starting timer for GetEnsembleProperties_req time out */
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_ENSEMBLE_PROPERTIES_CMD_TIMEOUT_TIME, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}				
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetEnsembleProperties_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}
		}
		break;

		case GET_ENSEMBLE_PROPERTIES_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for GetEnsembleProperties_req time out */
			if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT failed");	
			}
			else
			{
			 	st_TimerId.u32_DABDABBlending_Timer = 0;	
			}			 
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Ensemble properties reply received for freq is %d",DAB_Tuner_Ctrl_me->st_tuneStatus.Frequency);
			DabTuner_MsgReplyGetEnsembleProperties(&(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply));
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_tuneStatus.Frequency;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId 		= DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.EnsembleIdentifier;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC = DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.ECC ;
			SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
			DAB_Tuner_Ctrl_Update_EnsembleLabel(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
			if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x06)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2 ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x0f)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8 ;
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */			
			if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE_END)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Program List Cmd sent to DAB Tuner");
				memset(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),0 , sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
				(void)DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
				/* Starting timer for GetProgrammeServiceList_req time out */
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_PROGRAMLIST_CMD_TIMEOUT_TIME, GET_PROGRAMLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_PROGRAMLIST_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
			}
			else
			{
				if(DAB_Tuner_Ctrl_me->st_lsmdata.u32_Frequency == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency)
				{
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
					/* Clear Component label in "st_currentEnsembleData" structure */
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
					DAB_Tuner_Ctrl_Update_Label(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
					DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
				//	DAB_Tuner_Ctrl_me->e_LinkingStatus	=	DAB_TUNER_CTRL_DAB_NORMAL_PLAYBACK;
					DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
					DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
					DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE ;
					DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;/* newly added for AF Tune */		
					DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = FALSE;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.u32_TunedSId = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] Original Frequency tuned successfully");
					DAB_Tuner_Ctrl_Enable_Fig_Notifications();
					DAB_Tuner_Ctrl_Enable_Quality_Notifications();
			
				}
				else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_SORTED || DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_SORTED)
				{
					(void)DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Program List Cmd sent to DAB Tuner");
					memset(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),0 , sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
					/* Starting timer for GetProgrammeServiceList_req time out */
					st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_PROGRAMLIST_CMD_TIMEOUT_TIME, GET_PROGRAMLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_PROGRAMLIST_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				else
				{
					/* Clear service label in "st_currentEnsembleData" structure */
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
					/* Clear Component label in "st_currentEnsembleData" structure */
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
					DAB_Tuner_Ctrl_Update_Label(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
					DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
					SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
					/* Clearing FIG 0/2 Data base */
					DAB_Tuner_Ctrl_me->e_LinkingStatus	=	RADIO_FRMWK_DAB_NORMAL_PLAYBACK;
					DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
					DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
					DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE ;
					DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;/* newly added for AF Tune */
					memset(DAB_Tuner_Ctrl_me->st_Service_Info,0, sizeof(Ts_Service_Info)*MAX_ENSEMBLE_SERVICES);
					DAB_Tuner_Ctrl_me->u8_NoOfServicesIn_ServiceInfo = 0 ;
					DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE ;
					memset((DAB_Tuner_Ctrl_me->freqInfo),0,sizeof(DAB_Tuner_Ctrl_me->freqInfo));
					memset((DAB_Tuner_Ctrl_me->oeServices),0,sizeof(DAB_Tuner_Ctrl_me->oeServices));
					memset((DAB_Tuner_Ctrl_me->hardlinkinfo),0,sizeof(DAB_Tuner_Ctrl_me->hardlinkinfo));  /*Now commented for new dab-dab code */
					DAB_Tuner_Ctrl_Notify_DAB_DAB_Status(DAB_TUNER_CTRL_DAB_DAB_TUNE_SUCCESS);
					/* Updating the Station List After AF switching */
					DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = TRUE;
					UpdateStationListInfo(DAB_Tuner_Ctrl_me);
					DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = FALSE;
					Update_Stationlist_Into_LearnMem();
					ClearLinkingData(DAB_Tuner_Ctrl_me);
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.u32_TunedSId = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
					DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] DAB-DAB AF Successfull");
					DAB_Tuner_Ctrl_Enable_Fig_Notifications();
					DAB_Tuner_Ctrl_Enable_Quality_Notifications();
				}
				
			}	
		}
		break;
		
		case GET_PROGRAMLIST_TIMEOUT:
		{
			st_TimerId.u32_DABDABBlending_Timer = 0;
			/* Re-trigger GetProgrammeServiceList_req */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_PROGRAMLIST_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
				/* Starting timer for GetProgrammeServiceList_req time out */
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_PROGRAMLIST_CMD_TIMEOUT_TIME, GET_PROGRAMLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_PROGRAMLIST_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}			
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetProgrammeServiceList_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}
		}	 
		break;		

		case GET_PROGRAM_SERVLIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_PROGRAMLIST_TIMEOUT failed");	
			}
			else
			{
			 	st_TimerId.u32_DABDABBlending_Timer = 0;
			}				 
			DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
			//memset(scan_buffer_temp, 0x00, SCAN_SPI_READ_BUFFER_LENGTH);
			DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx = 0;
			/* Clear service label in "st_currentEnsembleData" structure */
			memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
			/* Clear Component label in "st_currentEnsembleData" structure */
			memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
			(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: Component List Cmd sent to DAB Tuner");
			/* Starting timer for GetComponentListReq_Cmd time out */
			st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_DABDABBlending_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
			}
			else
			{
				/*MISRA C*/	
			}
		}
		break;
		
		case GET_COMPONENTLIST_TIMEOUT:
		{
			st_TimerId.u32_DABDABBlending_Timer = 0;
			/* Re-trigger GetComponentListReq_Cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT)
			{
				if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_LIST_RECONFIG)
				{
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId) ;
				}	 
				else if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_RECONFIG)
				{
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
				}
				else
				{
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);

				}
				/* Starting timer for GetComponentListReq_Cmd time out */
				st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
				if(st_TimerId.u32_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}			
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetComponentListReq_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* No of services of current ensemble is more than one and current service is not the last service*/
				if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) != ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
					&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
				{
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo)* (MAX_ENSEMBLE_SERVICES - ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1)));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;

					/* Re-trigger Get component list for remaining services */
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
					/* Starting timer for GetComponentListReq_Cmd time out */
					st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
					if(st_TimerId.u32_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}				
				}
				/* No of services of current ensemble is more than one and current service is the last service*/
				else if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) == ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
					&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
				{
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;
				
					DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
					SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
					/* Copy st_GetCurrEnsembleProgListReply to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
					SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					/* Send sort request to DAB App */
					DAB_App_Request_sort();
				}
				else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices == 1)
				{
					/* Clear particular ensemble info in ast_EnsembleInfo array */
					memset(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],0,sizeof(Ts_CurrEnsemble_serviceinfo));
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;
					DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
					SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					DAB_Tuner_Ctrl_me->e_LinkingStatus	=	RADIO_FRMWK_DAB_NORMAL_PLAYBACK;
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
				}
				else /* Timeout of GetComponentListReq_Cmd */
				{
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_DABDABBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_COMPONENTLIST_TIMEOUT failed");	
			}
			else
			{
			 	st_TimerId.u32_DABDABBlending_Timer = 0;
			}				
			if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_LIST_RECONFIG)
			{
				b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));
			}	 
			else if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_RECONFIG)
			{
				b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));
			}
			else
			{
				b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));
			}
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_LIST_RECONFIG)
				{
					if(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx!=DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.u8_NumOfServices)
					{
						DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx++;
						(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
					}
					else
					{
						/* Checking if the curent ensemble service list is same as the newly received list */
						b_resultcode = CompareCurrentEnsembleServiceList(&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not),&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
						/* If the newly received service list is not same as current ensemble service list then update current ensemble service list */
						if(b_resultcode == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Change in program list, updating current ensemble program list ");
							/* Updating the current ensemble service list */		 
							UpdateCurrentEnsembleServiceList(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_ProgrammeServListChanged_not));
							/* Updating ast_EnsembleInfo, ast_ServiceInfo and ast_ComponentInfo */
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Updating station list ");
							SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
							UpdateEnsembleInfo(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							UpdateServiceInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							UpdateServiceCompInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData));
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							/* Notifying station list change to DAB application */						 
							 DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);//, DAB_Tuner_Ctrl_me->e_RequestCmd);
							/* Notifying Re-Configuration to DAB application */
							DAB_Tuner_Ctrl_Notify_ReConfiguration(TUNER_CTRL_SERVICE_LIST_RECONFIG, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId,
							DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].ProgServiceId,
							DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId) ;
							DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] No Change in program list, no Multiplex Re-Configuration happened ");
							DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
						}
					}
				}
				
				else if(DAB_Tuner_Ctrl_me->e_ReConfigType == TUNER_CTRL_SERVICE_RECONFIG)
				{
					/* Updating current SId components in ast_ComponentInfo */
					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
					UpdateCurrentServiceCompsToServiceCompInfo(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData), DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx);
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					DAB_Tuner_Ctrl_Notify_ReConfiguration(TUNER_CTRL_SERVICE_RECONFIG, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId,
					DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId,
					DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo.Component[0].InternalCompId) ;
					DAB_Tuner_Ctrl_me->e_ReConfigType = TUNER_CTRL_NO_RECONFIG ;
				}
				else
				{
					if(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx!=DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)
					{
						DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx++;
						(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
						/* Starting timer for GetComponentListReq_Cmd time out */
						st_TimerId.u32_DABDABBlending_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
						if(st_TimerId.u32_DABDABBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}				
					}
					else
					{
						DAB_Tuner_Ctrl_Update_Label(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
						DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
						if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE_END)
						{
							DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
						}
						else
						{
							DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
						}
						SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
						SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
						SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
						/* Send sort request to DAB App */
						DAB_App_Request_sort();
					}
				}
			}	 		 
		}
		break;
		
		case START_BLENDING_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] StartBlending_Cmd Timeout ");
			st_TimerId.u32_StartBlending_Timer  = 0;
			/* Re-trigger StartBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_START_BLENDING_CMD_MAX_REPEAT_COUNT)
			{
				if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
				{
					DabTuner_StartBlending_for_implicit(0x00);
					/* Starting timer for StartBlending_cmd time out */
					st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				}
				else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED)
				{
					DabTuner_StartBlending_for_implicit(0x80);
					/* Starting timer for StartBlending_cmd time out */
					if(st_TimerId.u32_StartBlending_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
						}
						else
						{
							st_TimerId.u32_StartBlending_Timer = 0;
						}						
					}
					else
					{
						/* MISRA C*/	
					}
					st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;					
				}
				else
				{
					/* FOR MISRA */
				}

			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] StartBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
				{
					DabTuner_StartBlending_for_implicit(0x00);
					/* Starting timer for StartBlending_cmd time out */
					if(st_TimerId.u32_StartBlending_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
						}
						else
						{
							st_TimerId.u32_StartBlending_Timer = 0;
						}						
					}
					else
					{
						/* MISRA C*/	
					}
					st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				}
				else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED)
				{
					DabTuner_StartBlending_for_implicit(0x80);
					/* Starting timer for StartBlending_cmd time out */
					if(st_TimerId.u32_StartBlending_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
						}
						else
						{
							st_TimerId.u32_StartBlending_Timer = 0;
						}						
					}
					else
					{
						/* MISRA C*/	
					}
					st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;					
				}
				else
				{
					/* FOR MISRA */
				}	
			}
		}
		break;
		
		case START_BLENDING_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->b_Implicit_Linking = FALSE;
			if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_StartBlending_Timer = 0;
			}				
			DabTuner_StartBlending_Reply(&(DAB_Tuner_Ctrl_me->st_Start_Blending_Reply),msg->data);
			DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] strat blending reply ");
			if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE)
			{
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLEND_BACK_TO_DAB);	
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] FM singal is low blend back to DAB ");
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED;
				DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData); /* when DAB signal resumes back then, sending the current ensemble data to upperlayer*/
				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
				if(st_TimerId.u32_DAB_DABLinking_Timer > 0)
				{
					if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
					}
					else
					{
						st_TimerId.u32_DAB_DABLinking_Timer	= 0;
					}
					
				}
				else
				{
					/* MISRA C*/	
				}
				st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(400, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE ;
				DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x51);
			}
			else if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_DISABLED) /*Once Linking setting is disabled, after linking to FM*/
			{
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLEND_BACK_TO_DAB);	
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Blending to FM is suspended, due to linking settings disabled ");
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED;	/* assigning linking status as FM blending has been suspended*/
				DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;
				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);/* sending select service success to upper layer, returning to DAB*/
				if(st_TimerId.u32_DAB_DABLinking_Timer > 0)
				{
					if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
					}
					else
					{
						st_TimerId.u32_DAB_DABLinking_Timer = 0;
					}					
				}
				else
				{
					/* MISRA C*/	
				}
				st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(400, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);/* tuning to its alternate frequency in DAB*/
				if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE ;
				DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x51);
			}
			else if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_RESUME_BACK))
			{
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLEND_BACK_TO_DAB);	
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DAB Signal Resumes back ");
				DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED;
				DAB_Tuner_Ctrl_me->u8_BlendingProcess = STOP_BLENDING_IN_PROGRESS;

				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData); /* when DAB signal resumes back then, sending the current ensemble data to upperlayer*/
				DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x51);
				if((DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes > 0) && (DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == FALSE))
				{
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.b_Hardlinks_sent = FALSE;	
					DAB_Tuner_Ctrl_me->e_BestPI_Type = DAB_TUNER_CTRL_NO_IMPLICIT;
					DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality = 0;
					memset((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks),0,sizeof(DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks));
					Notify_Hardlinks_To_FM(DAB_Tuner_Ctrl_me);
				}	
				if((DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay == 0))
				{
					 DAB_Tuner_Ctrl_me->b_Prepare_Blending = FALSE;
					 HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
				}
				else if((DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE) && (DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay != 0))
				{
					 DAB_Tuner_Ctrl_me->b_Prepare_Blending = TRUE;
					 HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
				}	 
				else
				{
					
				}
			}
			else
			{
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLENDED_TO_FM);	
			    DAB_Tuner_Ctrl_me->Start_Blending =	RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS;
				DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS;
				DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
				DAB_Tuner_Ctrl_me->u8_BlendingProcess = BLENDING_IN_PROGRESS;
				DabTuner_SetFIG_filter_command(0x0013,(Tu8)0x05,(Tu8)0x50);
			}	
			
		}
		break;
		
		case  BEST_PI_RECEIVED_NOTIFICATION:
		{
			DAB_Tuner_Ctrl_me->Index = 0; 
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI),(msg->data),sizeof(Tu16),&(DAB_Tuner_Ctrl_me->Index));	
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality),(msg->data),sizeof(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality),&(DAB_Tuner_Ctrl_me->Index));
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_BestPI_Type),(msg->data),sizeof(Te_Tuner_Ctrl_BestPI_Type),&(DAB_Tuner_Ctrl_me->Index));
			if(DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_IMPLICIT_SID && DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings != DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Implicit PI Received");
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,IMPLICIT_SID);	
				DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
				if((DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == FALSE) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
				{
				  if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE))
				  {
					if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
					}
					else
					{
						st_TimerId.u32_DAB_DABLinking_Timer = 0;
					}					
					DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE ;
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED;
					//DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = TRUE;
					DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Implicit Blending command sent ");
					DabTuner_StartBlending_for_implicit(0x80);
					/* Starting timer for StartBlending_cmd time out */
					if(st_TimerId.u32_StartBlending_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
						}
						else
						{
							st_TimerId.u32_StartBlending_Timer = 0;
						}						
					}
					else
					{
						/* MISRA C*/	
					}
				    st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				   	if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					
				  }
				  else if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_PI_REQUEST_SENT) || (DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT))
				  {
						if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
						}
						else
						{
							st_TimerId.u32_DAB_DABLinking_Timer = 0;	
						}						
						DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE ;
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED;
						DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Implicit Blending1 command sent ");
						DabTuner_StartBlending_for_implicit(0x80);
						/* Starting timer for StartBlending_cmd time out */
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;
							}							
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
					}
					else
					{
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED;
						DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
						
					}
				}
				else
				{
						if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
						}
						else
						{
							st_TimerId.u32_DAB_DABLinking_Timer = 0;
						}							
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED;
						DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
					DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
				}	
			}
			else if(DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_NO_IMPLICIT && DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings != DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
			{
				if(LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1))
				{
					DAB_Tuner_Ctrl_Notify_PICodeList((DAB_Tuner_Ctrl_me->st_PI_data_available),DAB_FM_LINKING_MIN_THRESHOULD,DAB_FM_LINKING_MAX_THRESHOULD,DAB_Tuner_Ctrl_me->st_lsmdata.u32_SId,CHECK_HARDLINK_AND_IMPLICIT);						
				}
				else
				{
					/*Misra*/
				}	
				if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_NORMAL_PLAYBACK) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED))
				{
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT;
					if((DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == TRUE))
					{	
						if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
						}
						else
						{
							st_TimerId.u32_DAB_DABLinking_Timer = 0;
						}							 
						DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
					else
					{
						if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start == TRUE)
						{
							if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
							}
							else
							{
								 st_TimerId.u32_DAB_DABLinking_Timer = 0;
							}

							st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(400, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);
							if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
							}
							else
							{
								/*MISRA C*/	
							}
						}
	
					}	
				}	
			}	
			else if(DAB_Tuner_Ctrl_me->e_BestPI_Type == DAB_TUNER_CTRL_HARDLINK_PI && DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings != DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
			{
				
				for(DAB_Tuner_Ctrl_Best_PI_Check_Index = 0; DAB_Tuner_Ctrl_Best_PI_Check_Index < DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes ; DAB_Tuner_Ctrl_Best_PI_Check_Index++ )
				{
					if(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI == DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks[DAB_Tuner_Ctrl_Best_PI_Check_Index])
					{
						//DAB_Tuner_Ctrl_me->e_LinkingStatus = DAB_TUNER_CTRL_FM_BEST_PI_RECEIVED;

						DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = TRUE;
						DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLENDING_INFO);
						break;
					}
					else
					{
						DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
					}	
				}
				if(DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received == TRUE)
				{
					if((DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == FALSE) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
					{
						DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE ;
					    if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE))
						{
							if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
							}
							else
							{
								st_TimerId.u32_DAB_DABLinking_Timer = 0;
							}						
						 	DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE ;
						 	DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED;
						 	DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
							if(DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay == 0)
								DabTuner_StartBlending_Without_delay(0x80);
							else
								DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
						}
					    else if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_PI_REQUEST_SENT) || (DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT))
					  	{
							if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
							}
							else
							{
								st_TimerId.u32_DAB_DABLinking_Timer = 0;
							}					
						 	DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE ;
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED;
						 	DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
						 	DabTuner_StartBlending_Without_delay(0x80);
						}
					  	else
					  	{
							if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_NORMAL_PLAYBACK) || (DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED))
							{
								if(DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay != 0) 
									DAB_Tuner_Ctrl_me->b_Prepare_Blending = TRUE;
								else
									DAB_Tuner_Ctrl_me->b_Prepare_Blending = FALSE;
							  	DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED;
								HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
							}
							DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);
						}
					}
					else
					{
						if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
						}
						else
						{
							st_TimerId.u32_DAB_DABLinking_Timer = 0;
						}						 
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED;
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					} 	
				}
				else
				{
					/*Misra*/
				} 	
			}
			else
			{
				if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings == DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
				{
					if(DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing == TRUE)
					{
						DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
					}
					else
					{
							/*Misra*/
					}
					
				}
			}	

		}
		break;
		
		case DAB_TUNER_CTRL_PI_QUALITY_NOTIFYID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality),(msg->data),sizeof(Tu8),&(DAB_Tuner_Ctrl_me->Index));		
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,BLENDING_INFO);	
			if(DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
			{
				if((DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality < DAB_FM_LINKING_MIN_THRESHOULD))
				{			
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE;
					DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
					DAB_Tuner_Ctrl_me->e_BestPI_Type = DAB_TUNER_CTRL_NO_IMPLICIT;
					DabTuner_StartBlending_for_implicit(0x00);
					/* Starting timer for StartBlending_cmd time out */
					if(st_TimerId.u32_StartBlending_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
						}
						else
						{
							st_TimerId.u32_StartBlending_Timer = 0;
						}							
					}
					else
					{
						/* MISRA C*/	
					}
				    st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);	
					if(st_TimerId.u32_StartBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}				
				}
			}
		}
		break;
		
		case START_DAB_DAB_LINKING:
		{
			st_TimerId.u32_DAB_DABLinking_Timer = 0;	
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
			DAB_Tuner_Ctrl_me->b_Timer_start = FALSE;
			AltEnsembleindex		= 0;
			b_Hardlinksused 		= FALSE;
			Hardlinkindex			= 0;
			Altfreqindex			= 0;
			b_Tune_to_Orginal_freq_Check	=	FALSE;
			if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_NORMAL_PLAYBACK))
			{
				
				DAB_Tuner_Ctrl_Notify_DAB_DAB_Status(DAB_TUNER_CTRL_DAB_ALT_TUNE_STARTED);
				DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(0x00);
				DAB_Tuner_Ctrl_CheckForDABAlternative(DAB_Tuner_Ctrl_me);
			}
				
		}
		break;
		case DAB_TUNER_CTRL_COMP_LIST_SORT_RESID:
		{
			/* Copy st_Dab_Tuner_Ctrl_CurrEnsembleProgList back to st_GetCurrEnsembleProgListReply after sorting */
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply, &st_Dab_Tuner_Ctrl_CurrEnsembleProgList,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
			
			DAB_Tuner_Ctrl_me->e_LinkingStatus	=	RADIO_FRMWK_DAB_NORMAL_PLAYBACK;
			DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = FALSE;
			DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE ;
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;/* newly added for AF Tune */
  			RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] DAB-DAB: DAB-DAB linking switching response sent to Higher layer");
			DAB_Tuner_Ctrl_Enable_Fig_Notifications();
			DAB_Tuner_Ctrl_Enable_Quality_Notifications();
			//ALternateFreq = 0;
			if(DAB_Tuner_Ctrl_me->st_lsmdata.u32_Frequency != DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency)
			{
				/* Enabling FIG 0/2 */ 
				DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
				/* Clearing FIG 0/2 Data base */
				memset(DAB_Tuner_Ctrl_me->st_Service_Info,0, sizeof(Ts_Service_Info)*MAX_ENSEMBLE_SERVICES);
				DAB_Tuner_Ctrl_me->u8_NoOfServicesIn_ServiceInfo = 0 ;
				DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE;
				memset((DAB_Tuner_Ctrl_me->freqInfo),0,sizeof(DAB_Tuner_Ctrl_me->freqInfo));
				memset((DAB_Tuner_Ctrl_me->oeServices),0,sizeof(DAB_Tuner_Ctrl_me->oeServices));
				memset((DAB_Tuner_Ctrl_me->hardlinkinfo),0,sizeof(DAB_Tuner_Ctrl_me->hardlinkinfo));  /*Now commented for new dab-dab code */
				DAB_Tuner_Ctrl_Notify_DAB_DAB_Status(DAB_TUNER_CTRL_DAB_DAB_TUNE_SUCCESS);
				/* Updating the Station List during tune */
				// DAB_Tuner_Ctrl_Notify_STLUpdated(TC_DAB_TUNER_APP);//, DAB_Tuner_Ctrl_me->e_RequestCmd);
				DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = TRUE;
				UpdateStationListInfo(DAB_Tuner_Ctrl_me);
				DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = FALSE;
				DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
				Update_Stationlist_Into_LearnMem();
				ClearLinkingData(DAB_Tuner_Ctrl_me);
				DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.u32_TunedSId = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
				//Update_Stationlist_Into_LearnMem();
				DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] DAB-DAB AF Successfull");
				//DAB_Tuner_Ctrl_me->e_TunedtoNewEnsemble = FALSE;
			}
			else
			{
				//ClearLinkingData(DAB_Tuner_Ctrl_me);
				DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.u32_TunedSId = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
				//Update_Stationlist_Into_LearnMem();
				DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);	
				
			}	
			SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
		}
		break;

		case HSM_MSGID_EXIT:
		{
			b_handlerchanged = FALSE;
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}


/*===============================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*===============================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode = FALSE ;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
	
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ActiveBusyHndlr"));
		}
		break;
		/*	case DAB_TUNER_CTRL_SYNCHRONISATION_NOTIFICATION:
		{
		DabTuner_MsgRcvSetSynchronisationNotification(&(DAB_Tuner_Ctrl_me->st_synchNotification),msg->data);
		}
		break ;*/
		
		case DAB_TUNER_CTRL_ERROR:
		{
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_err_state);
		}
		break;
		
		case SYSTEM_CONTORL_NOTIFICATION:
		 {
			b_resultcode = DabTunerCtrl_SystemMonitoring_not(&(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not),(Ts8*)(msg->data));
			DAB_Tuner_Ctrl_me->e_SystemMonitoringNotReceived = DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_RECEIVED ;
			/* Store the current state of DAB_Tuner_Ctrl */
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_FG_STATE ;
			
			//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] System monitoring notification MonitorId = %x Status = %x ",DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId, DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status );
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x05)
				{
					/* Out going messages dropped by DABTuner */
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status > 2000) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state");
						/* Store the current state of DAB_Tuner_Ctrl */
					DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				}
		#if 1
				/* Audio-out event notifications */
				else if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x20)
				{
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status == 0x10) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state due to Audio-out event change");
						DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,QUALITY_PARAMETERS);
						/* Store the current state of DAB_Tuner_Ctrl */
						DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				}
		#endif
		#if 0
				/* system overload event notifications */
				else if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x22)
				{
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status == 0x04 ) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state");
						/* Store the current state of DAB_Tuner_Ctrl */
						DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */					
				}				
		#endif						
				else
				{
					/* Doing nothing */	
				}/* For MISRA C */
			}
			else
			{
				/* FOR MISRA */
			}
		}
		break;
		
		case DAB_TUNER_CTRL_CANCEL_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_CancelType),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_CancelType),&(DAB_Tuner_Ctrl_me->Index));
			
			if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_TUNE_CANCEL)
			{
				if(DAB_Tuner_Ctrl_me->e_SelectService_Status == TUNER_CTRL_SERVICE_OTHER_ENSEMBLE)
				{
						DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices = 0;
				}
				else if(DAB_Tuner_Ctrl_me->e_SelectService_Status == TUNER_CTRL_SERVICE_SAME_ENSEMBLE)
				{
					//DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId;
					//DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI;
				}
				else
				{
				
				}
				DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
				DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
				DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
			else
			{
				/* For MISRA */ 
			}
		}
		break;

		case HSM_MSGID_EXIT: 
		{
			
		}
		break;
		
		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}	


/*==================================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Ts_Sys_Msg* msg)   */
/*==================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
    UNUSED(dab_tuner_ctrl_inst_hsm_select_component_cmd);
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode = FALSE;
	PRINT_MSG_DATA(msg);
	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr"));			
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_SelServAndCompHndlr";
			if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER) /* If the request is Select Service */
			{
				if((DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency == 0) && (DAB_Tuner_Ctrl_me->requestedinfo.u32_SId != 0))
				{
					//Radio_NVM_ReadDATA(CURRENT_SID_LINKING_DATA,&(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo),sizeof(Ts_CurrentSidLinkingInfo));
					if(DAB_Tuner_Ctrl_me->requestedinfo.u32_SId == DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.u32_TunedSId)
					{
						SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
						DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_CANCELLED;
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId ;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_AFTUNE;
						DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
					}
					else
					{
						SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
						ClearLinkingData(DAB_Tuner_Ctrl_me);
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId ;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_AFTUNE;
						DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
						//DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = 0;
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
					}
				}
				else if((DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency == DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency) && (DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == DAB_Tuner_Ctrl_me->requestedinfo.u32_SId) && (DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI == DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI) && (DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId == DAB_Tuner_Ctrl_me->requestedinfo.u16_EId))
				{
					if(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x02)
					{
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);	
					}
					else
					{
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
						/* Enable Notifications */
						DAB_Tuner_Ctrl_Enable_Fig_Notifications();
						DAB_Tuner_Ctrl_Enable_Quality_Notifications();
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_listen_state);
					}
				}		
				/* Check the threshold value for the current ensemble*/
				else if((DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency == DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency) && (DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == DAB_Tuner_Ctrl_me->requestedinfo.u32_SId))
				{
					DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICE_SAME_ENSEMBLE;
					if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices == 0)
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_ensemble_tune_state); 
					else	 
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_SelectAudio_state);
				}	
				else if((DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency == DAB_Tuner_Ctrl_me-> requestedinfo.u32_Frequency)&&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId !=DAB_Tuner_Ctrl_me->requestedinfo.u32_SId))
				{
					b_resultcode = DAB_Tuner_Ctrl_Check_Service_Available(DAB_Tuner_Ctrl_me);
					if(b_resultcode == TRUE)
					{
						DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICE_SAME_ENSEMBLE;
						if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices == 0)
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_ensemble_tune_state); 
						else	 
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_SelectAudio_state);
					}
					else
					{
						DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICE_NOT_IN_ENSEMBLE;
						DabTuner_MsgSndDeSelectService_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId);
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SERVICE_NOT_AVAILABLE,DAB_Tuner_Ctrl_me->requestedinfo);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);	
					}		
				}
				else
				{
					if(DAB_Tuner_Ctrl_me->requestedinfo.u32_SId == 0 && DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI == 0)
					{
						DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = TRUE;
					}
					DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICE_OTHER_ENSEMBLE;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_ensemble_tune_state);  
				}  
			}
			else
			{
			/* Doing nothing */
			} /* For MISRA C */
		}
		break;
#if 0	
	case DAB_TUNER_CTRL_SYNCHRONISATION_NOTIFICATION:
		{
			DabTuner_MsgRcvSetSynchronisationNotification(&(DAB_Tuner_Ctrl_me->st_synchNotification),msg->data);
	        DAB_Tuner_Ctrl_Notify_Synchronisation(DAB_Tuner_Ctrl_me->st_synchNotification.ChannelSync);
		}
		break;
#endif	
	case DAB_TUNER_CTRL_CANCEL_REQID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_CancelType),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_CancelType),&(DAB_Tuner_Ctrl_me->Index));
			
			if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_TUNE_CANCEL)
			{
				if(DAB_Tuner_Ctrl_me->e_SelectService_Status == TUNER_CTRL_SERVICE_OTHER_ENSEMBLE)
				{
						DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices = 0;
				}
				else if(DAB_Tuner_Ctrl_me->e_SelectService_Status == TUNER_CTRL_SERVICE_SAME_ENSEMBLE)
				{
				}
				else if(DAB_Tuner_Ctrl_me->e_SelectService_Status == TUNER_CTRL_SERVICE_NOT_IN_ENSEMBLE)
				{
					//DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = 0;
				}
				else if(DAB_Tuner_Ctrl_me->e_SelectService_Status == TUNER_CTRL_SERVICECOMPONENT_SELECTED)
				{
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI;
					DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
				}
				else if(DAB_Tuner_Ctrl_me->e_SelectService_Status == TUNER_CTRL_SERVICE_SELECTED)
				{
					(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI, DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId;
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI;
					DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
				}
				DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICE_INVALID;
				DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
				DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
				DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
			else if(DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_AF_TUNE_CANCEL)
			{
				memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
				DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
				DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
		}
		break;

	case HSM_MSGID_EXIT: 
		{
			
		}
		break;
	default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}



/*==============================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ESD_AllBandTuneHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)  */
/*==============================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_ESD_AllBandTuneHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */

	PRINT_MSG_DATA(msg); 

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ESD_AllBandTuneHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_ESD_AllBandTuneHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ESD_AllBandTuneHndlr"));
			SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_lsmdata),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_TUNER_CTRL_5A_EU ;
			DAB_Tuner_Ctrl_me->u32_SeekStartFrequency = DAB_TUNER_CTRL_5A_EU ;
			DAB_Tuner_Ctrl_me->e_seekDirection = RADIO_FRMWK_DIRECTION_UP;
			DAB_Tuner_Ctrl_me->u8_DABTuner_Abnormal_RSSI_Check_Count = 0;
			DAB_Tuner_Ctrl_me->u8_SeekFrequencyIndex = (Tu8)(FindSeekFrequencyIndex(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->e_seekDirection) );
			/* Updating current ensemble data frequency with the frequency to be given for tune */
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = dab_app_freq_band_eu[DAB_Tuner_Ctrl_me->u8_SeekFrequencyIndex].u32_frequency ;
			/* Updating seekInfo frequency with the frequency to be given for tune */	
			DAB_Tuner_Ctrl_me->st_seekInfo.u32_Frequency = dab_app_freq_band_eu[DAB_Tuner_Ctrl_me->u8_SeekFrequencyIndex].u32_frequency ;
			/* Sending TuneTo_cmd to DABTuner */
			(void)DabTuner_MsgSndTuneTo_Cmd(dab_app_freq_band_eu[DAB_Tuner_Ctrl_me->u8_SeekFrequencyIndex].u32_frequency);
			
			/* Starting timer for TUNE_TIME_OUT */
			st_TimerId.u32_ESD_AllBandTune_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_ESD_AllBandTune_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
			}
			else
			{
				/*MISRA C*/	
			}

			DabTuner_MsgSndSetRSSINotifier_cmd(0x01);
		}  
		break;


		case RSSI_NOTIFICATION:
		{
			if(SYS_StopTimer(st_TimerId.u32_ESD_AllBandTune_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
			}
			else
			{
			 	st_TimerId.u32_ESD_AllBandTune_Timer = 0;
			}
			DabTuner_MsgSndSetRSSINotifier_cmd(0x00);
			(void)DabTuner_MsgRcvRSSI_notify(&(DAB_Tuner_Ctrl_me->st_RSSI_notification),msg->data);
			if(DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel < ESD_THRESHOLD )
			{
				DAB_Tuner_Ctrl_me->u8_DABTuner_Abnormal_RSSI_Check_Count++;
			}
			else
			{
				
			}
			if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency != DAB_Tuner_Ctrl_me->u32_SeekStartFrequency)
			{
				/* Finding the index of the frequency to be tuned based on current ensemble data Frequency and seek direction */
				DAB_Tuner_Ctrl_me->u8_SeekFrequencyIndex 	= (Tu8)(FindSeekFrequencyIndex(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->e_seekDirection) );
		
				/* Updating current ensemble data frequency with the frequency to be given for tune */
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = dab_app_freq_band_eu[DAB_Tuner_Ctrl_me->u8_SeekFrequencyIndex].u32_frequency ;
		
				/* Sending TuneTo_cmd to DABTuner */
				(void)DabTuner_MsgSndTuneTo_Cmd(dab_app_freq_band_eu[DAB_Tuner_Ctrl_me->u8_SeekFrequencyIndex].u32_frequency);
		
				/* Starting timer for TUNE_TIME_OUT */
				st_TimerId.u32_ESD_AllBandTune_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_ESD_AllBandTune_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
				}
				else
				{
					/*MISRA C*/	
				}

				DabTuner_MsgSndSetRSSINotifier_cmd(0x01);
				
			}
			else
			{
				if(DAB_Tuner_Ctrl_me->u8_DABTuner_Abnormal_RSSI_Check_Count > 0)
				{
					SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),&(DAB_Tuner_Ctrl_me->st_lsmdata),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
					
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DABTUNER in abnormal state");
					/* Store the current state of DAB_Tuner_Ctrl */
					DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL;
					/* Send Notification to DAB App that DABTuner is Abnormal*/
					DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
					/* Transit to Inactive state*/
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
				}
				
				
			}
			
		}
		break;	

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;
				
		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;
	}
	return ret;
}


/*================================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)  */
/*================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{ 
			//b_resultcode=FALSE;
		}
		break;

		case HSM_MSGID_START :
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr ");
			// DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_TuneEnsembleHndlr"));
			
			/* Clearing the current ensemble service and component list */
			memset(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),0,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId = 0;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC = 0;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = 0;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = 0;
			/* Updating current ensemble frequency with requested frequency */
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency;
			
			/* Notifying the frequency to be tuned, to DAB application */ 
			DAB_Tuner_Ctrl_Notify_FrequencyChange(DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency);
			
			/* Sending TuneTo_cmd to HAL */
			etal_ret = DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency);
			if(etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_TUNE_REPLY);
			}
			else 
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, TUNE_TIME_OUT);
			}

          
		}
		break;

		case TUNE_TIME_OUT:
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ETAL DAB TUNING FAILED %d", etal_ret);
			switch(etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_IN_PROGRESS:
				case ETAL_RET_NO_DATA:
				{
					//Retry the command for 2 times
					if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_TUNE_CMD_MAX_REPEAT_COUNT)
					{
						/* Sending TuneTo_cmd to HAL */
						etal_ret = DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency);
						if(etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_TUNE_REPLY);
						}
						else 
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, TUNE_TIME_OUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB TUNING FAILED  ");
						/* In initial start up tune fails */
						if(DAB_Tuner_Ctrl_me->b_InitialStartUpFlag == TRUE)
						{
							DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE ;
						}
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Tune failed");
			
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
		
						/* Clear Component label in "st_currentEnsembleData" structure */
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
						/* Sending tune fail response to DAB Application */
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
			
						/* Transiting to dab_tuner_ctrl_inst_hsm_Nosignallisten_state */
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

		case DAB_TUNER_CTRL_TUNE_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			etal_ret = DabTunerCtrl_GetAudioStatus_req();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT);
			}

		}
		break;

		case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY:
		{
			memset(&(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl), 0, sizeof(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl));
			DabTunerCtrl_GetAudioStatus_repl(&(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl));
			if(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.FicBitErrorRatio < FIC_BER_THRESHOLD)
			{
				/* Command to get the uEID */
				etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
				}
			}
			else
			{
				memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

				/* Clear Component label in "st_currentEnsembleData" structure */
				memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->requestedinfo);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB FG SCAN ENSEMBLE AUDIO QUALITY FAILED - %d ", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					/* Retry the commands for 2 times */
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_AUDIO_STATUS_MAX_REPEAT_COUNT)
					{
						/* Cmd to get quality parameters from HAL (ETAL) */
						etal_ret = DabTunerCtrl_GetAudioStatus_req();
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT);
						}


						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetAudioStatus_req_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

						/* Clear Component label in "st_currentEnsembleData" structure */
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->requestedinfo);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
					}
				}
				break;

				default:
				{
					/* To avoid Warnings */
				}
				break;
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

			/* Command to get the ensemble properties */
			etal_ret = DabTuner_MsgSndGetEnsembleProperties();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
			}

		}
		break;
		
		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB GET CURR ENSEMBLE FAILED - %d", etal_ret);

			switch(etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					//Retry the commands for 2 times
					if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the uEID */
						etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
						if(etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
						}
						else 
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

						/* Clear Component label in "st_currentEnsembleData" structure */
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT FAILED  ");
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->requestedinfo);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
					}
				}
				break;

				default:
				{
						
				}
				break;

			}
		}
		break;
			
		case GET_ENSEMBLE_PROPERTIES_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

			/* Extracting the msg and updating st_GetEnsembleProperties_reply */ 
			DabTuner_MsgReplyGetEnsembleProperties(&(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply));
			
			/* Updating current ensemble label */
			SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
			
			/* Updating current ensemble EId */
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId = DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.EnsembleIdentifier;
			
			/* Updating current ensemble ECC */
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC = DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.ECC ;
			
			if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x06)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2 ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x0f)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8 ;
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */
			
		
			/* Transiting to Idle state dab_tuner_ctrl_inst_hsm_GetCurrEnsembleProgmList_state */
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_GetCurrEnsembleProgmList_state);
		}
		break;
		
		case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ETAL DAB ENSEMBLE PROPERTIES FAILED - %d", etal_ret);
			switch(etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_NO_DATA:
				{
					//Retry the commands for 2 times
					if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the ensemble properties */
						etal_ret = DabTuner_MsgSndGetEnsembleProperties();
		
						if(etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL ,GET_ENSEMBLE_PROPERTIES_REPLY);
						}
						else 
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL ,GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetEnsembleProperties_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

						/* Clear Component label in "st_currentEnsembleData" structure */
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL,DAB_Tuner_Ctrl_me->requestedinfo);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);	
					}
				}
				break;

				default:
				{
					
				}
				break;
			}
		}
		break;

		case HSM_MSGID_EXIT: 
		{
			
		} 
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;
	}
	return ret;
}

/*===========================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)  */
/*===========================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{

	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */

	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr"));
			/*Command to get service list*/
			etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
			if(etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
			}
			else 
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
			}

		}
		break;
		
		case GET_PROGRAMLIST_TIMEOUT:
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB ENSEMBLE SERVICE LIST PROPERTIES FAILED - %d", etal_ret);

			switch(etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_NO_DATA:
				{
					// Retry the commands for 2 times
					if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_PROGRAMLIST_CMD_MAX_REPEAT_COUNT)
					{
						/*Command to get service list*/
						etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
						if(etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
						}
						else 
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
						}
	
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetProgrammeServiceList_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID ;
			
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_FAILURE,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
				
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);	
						
					}
				}
				break;

				default:
				{
						
				}
				break;

			}
		}	 
		break;		

		case GET_PROGRAM_SERVLIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

			DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
			//memset(scan_buffer_temp, 0x00, SCAN_SPI_READ_BUFFER_LENGTH);
			/*	   if(u8_test == 0)
			{
			DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices = 0;
			u8_test = 1;
			}*/	
		 	if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > MAX_ENSEMBLE_SERVICES)
			{
				DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices = MAX_ENSEMBLE_SERVICES;
			}
			if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 0)
			{
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_GetCurrEnsembleCompList_state);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Getting program list failed ");
				DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID ;
					
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_FAILURE,DAB_Tuner_Ctrl_me->st_currentEnsembleData) ;
					
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);	
			}
		}
		break;
		
		case HSM_MSGID_EXIT:  
		{
			
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg; 
		}
		break;
	}
	return ret;
}


/*===========================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleCompListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)  */
/*===========================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleCompListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleCompListHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleProgmListHndlr";
			SYS_RADIO_MEMCPY((void*) DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleCompListHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_GetCurrEnsembleCompListHndlr"));
			DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx = 0;
			
			/* Command to get component list */
			etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
			if(etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL,DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
			}
			else 
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL,GET_COMPONENTLIST_TIMEOUT);
			}
		}
		break;
		
		case GET_COMPONENTLIST_TIMEOUT:
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ETAL DAB ENSEMBLE SERVICE COMPONENT LIST PROPERTIES FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_NO_DATA:
				{
					/* Retry the commands for 2 times */
					if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get component list */
						etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
						if(etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
						}
						else 
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
						}
		
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetComponentListReq_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						/* No of services of current ensemble is more than one and current service is not the last service*/
						if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) != ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
							&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
						{
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo)* (MAX_ENSEMBLE_SERVICES - ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1)));
							/* Decrement total no of services count*/
							(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;

							/* Re-trigger Get component list for remaining services */
							etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);

							if (etal_ret == ETAL_RET_SUCCESS)
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
							}
						}
						/* No of services of current ensemble is more than one and current service is the last service*/
						else if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) == ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
							&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
						{
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
							/* Decrement total no of services count*/
							(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;
							
							/* Copy st_GetCurrEnsembleProgListReply to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
							SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
							SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							/* Send sort request to DAB App */
							DAB_App_Request_sort();
					
						}
						else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices == 1)
						{
							/* Clear particular ensemble info in ast_EnsembleInfo array */
							memset(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],0,sizeof(Ts_CurrEnsemble_serviceinfo));
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
							/* Decrement total no of services count*/
							(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;
					
							DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
							SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),&(DAB_Tuner_Ctrl_me->requestedinfo),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));	 
							DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL,DAB_Tuner_Ctrl_me->requestedinfo);
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state); 	
					
						}
						else /* Timeout of GetComponentListReq_Cmd */
						{
							/* Do Nothing */
						}
					}
				}
				break;

				default:
				{
						
				}
				break;
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
		
			/* Cmd for getting the service information */
			b_resultcode = DabTuner_GetServListpropertiesReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx]));
			
			if (b_resultcode == TRUE)
			{
				/* Cmd for getting the service comp info */
				b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));

				DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx++;

				if (b_resultcode == TRUE)
				{
					if (DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)
					{

						/* Command to get component list */
						etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
						}

					}
					else
					{
						//DabTuner_SetFIG_filter_command((Tu16)2u,(Tu8)0x02,(Tu8)0x21);
						/* Clearing FIG 0/2 Data base */
						memset(DAB_Tuner_Ctrl_me->st_Service_Info, 0, sizeof(Ts_Service_Info)*MAX_ENSEMBLE_SERVICES);
						DAB_Tuner_Ctrl_me->u8_NoOfServicesIn_ServiceInfo = 0;
						//DAB_Tuner_Ctrl_me->b_FIG02_Disabled = FALSE ;

						/* Copy st_GetCurrEnsembleProgListReply to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
						SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
						SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList, &DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply, sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
						SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
						/* Send sort request to DAB App */
						DAB_App_Request_sort();
					}

				}
				else
				{
					/* Doing nothing */
				}/* For MISRA C */
			}
		}
		break;
		
		case DAB_TUNER_CTRL_COMP_LIST_SORT_RESID:
         {
			/* Copy st_Dab_Tuner_Ctrl_CurrEnsembleProgList back to st_GetCurrEnsembleProgListReply after sorting */
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply, &st_Dab_Tuner_Ctrl_CurrEnsembleProgList,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
			DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = TRUE;
			UpdateStationListInfo(DAB_Tuner_Ctrl_me);	
			Update_Stationlist_Into_LearnMem();	
			
			if(DAB_Tuner_Ctrl_me->b_InitialStartUpFlag == TRUE)
			{	
				/* Notify the DAB App that GET_COMPONENT_LIST request is success and send freq and EId info */
				DAB_Tuner_Ctrl_Notify_CompListStatus(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency, DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId, REPLYSTATUS_SUCCESS);
				DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
			else if(DAB_Tuner_Ctrl_me->b_InitialStartUpFlag == FALSE)
			{
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_SelectAudio_state);
			}
			else
			{
				/* Doing nothing */	 
			}/* For MISRA C */

		}
	    break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg; 
		}
		break;
	}
	return ret;
}

/*===========================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelServHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*===========================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelServHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
    UNUSED(dab_tuner_ctrl_inst_hsm_selectserv_cmd);
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_SelServHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_SelServHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_SelServHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_SelServHndlr"));
			if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER) /* If the Request is Select Service */
			{
				(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
				DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICE_SELECTED;
				/* Starting timer for SelectService_repl time out */
				st_TimerId.u32_SelectService_Timer = SYS_StartTimer(DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME, SELECT_SERVICE_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_SelectService_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_SERVICE_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
			}
			else
			{
				/* Do nothing */
			}/* For MISRA C */
		}  
		break;

		case  SELECT_SERVICE_TIMEOUT :
		{
			st_TimerId.u32_SelectService_Timer = 0;
			/* Re-trigger SelectService_repl */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_SERVICE_CMD_MAX_REPEAT_COUNT)
			{
				if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
				{
					(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
					/* Starting timer for SelectService_repl time out */
					st_TimerId.u32_SelectService_Timer = SYS_StartTimer(DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME, SELECT_SERVICE_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_SelectService_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_SERVICE_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;				
				}
				else
				{
					/* For Misra */
				}
			}
			else
			{
				if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectService_Cmd Timeout even after retriggering");
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
					SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_currentEnsembleData,&DAB_Tuner_Ctrl_me->requestedinfo,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SERVICE_NOT_AVAILABLE,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else
				{
					/* For Misra */
				}
			}
		}
		break;
		
		case DAB_TUNER_CTRL_SELECT_SERVICE_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for SelectService_repl time out */
			if(SYS_StopTimer(st_TimerId.u32_SelectService_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for SELECT_SERVICE_TIMEOUT failed");	
			}
			else
			{
			 	st_TimerId.u32_SelectService_Timer = 0;
			}			
			DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus = 1;
			b_resultcode = DabTuner_MsgRcvSelectServiceReply(&(DAB_Tuner_Ctrl_me->st_selectServiceReply),msg->data);
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] reply received for Seeking Sid is %d reply is %d",DAB_Tuner_Ctrl_me->st_seekInfo.u32_SId,DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus);
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus==0)
				{
					//ClearLinkingData(DAB_Tuner_Ctrl_me);
					b_Anno_ongoingflag = FALSE;
					/* Clear all announcement related database */	
					DAB_Tuner_Ctrl_ClearingAnnoDatabases(DAB_Tuner_Ctrl_me);
					ClearLinkingData(DAB_Tuner_Ctrl_me);
					DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level	 =  UN_RELIABLE;
					DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_LINKING_CANCELLED;   /* added newly for linking cancel */
					DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(DAB_Tuner_Ctrl_me->e_LinkingStatus);  /* added newly for linking cancel */
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_SelectAudio_state);
				}
				else
				{
					if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Select serivce failed");
						SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_currentEnsembleData,&DAB_Tuner_Ctrl_me->requestedinfo,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SERVICE_NOT_AVAILABLE,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					}
					else
					{
						/* Doing Nothing */
					} /* For MISRA C */
				}
			}
		}
		break;


		/*	case DAB_TUNER_CTRL_AUDIO_STATUS_NOTIFICATION:
		{

		DabTuner_Get_Audio_Status_Notification(&(DAB_Tuner_Ctrl_me->st_Audiostatus),msg->data);
		DAB_Tuner_Ctrl_Service_Search(DAB_Tuner_Ctrl_me);

		}
		break;	*/

		case DAB_TUNER_CTRL_SYNCHRONISATION_NOTIFICATION:
		{
			DabTuner_MsgRcvSetSynchronisationNotification(&(DAB_Tuner_Ctrl_me->st_synchNotification),msg->data);
			DAB_Tuner_Ctrl_Notify_Synchronisation(DAB_Tuner_Ctrl_me->st_synchNotification.ChannelSync);
		}
		break;
		
		case HSM_MSGID_EXIT: 
		{
			
		} 
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;

	}
	return ret;
}


/*==============================================================================================================================*/
/*  Ts_Sys_Msg* (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr  Ts_Sys_Msg* msg)   */
/*==============================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_SelectAudioHndlr"));
			if (DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN) /* If the Request is Select Service or select first service of each scan ensemble*/
			{
				DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICECOMPONENT_SELECTED;
				/* Cmd to select a service component in a service */
				etal_ret = DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI,DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
				if(etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY);
				}
				else 
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT);
				}

			}
			else
			{
				/* Do nothing */
			}/* For MISRA C */
		}  
		break;

		case  DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT :
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ETAL DAB ENSEMBLE SELECT SERVICE AUDIO FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_IN_PROGRESS:
				{
				    /* Retry the commands for 2 times */
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_AUDIO_CMD_MAX_REPEAT_COUNT)
					{
						if (DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN)
						{
							/* Cmd to select a service component in a service */
							etal_ret = DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI, DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY);
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT);
							}

							DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
						}
						else
						{
							/* For Misra */
						}
					}
					else
					{
						if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectService_Cmd Timeout even after retriggering");
							DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
							SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_currentEnsembleData, &DAB_Tuner_Ctrl_me->requestedinfo, sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
							DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE, DAB_Tuner_Ctrl_me->st_currentEnsembleData);
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
						}
						/* For foreground Scan */
						else if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN)
						{
							if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
								|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
							{
								DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
								DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
								DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
								DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
								Update_Stationlist_Into_LearnMem();
								DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
							}
							else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Audio Play failed for first service");
								/* if there is no audio playing then transition to scan state to get next available freq or ensemble audio*/
								HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_active_busy_scan_state);
							}
						}
						else
						{
							/* For Misra */
						}
					}
				}
				break;

				case ETAL_RET_NO_DATA:
				{
					/* Retry the commands for 2 times */
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_AUDIO_CMD_MAX_REPEAT_COUNT)
					{
						if (DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN)
						{
							/* Cmd to select a service component in a service */
							etal_ret = DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI,DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
							if(etal_ret == ETAL_RET_SUCCESS)
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY);
							}
							else 
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_SELECT_AUDIO_REPLY_TIMEOUT);
							}

							DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;				
						}
						else
						{
							/* For Misra */
						}
					}
					else
					{
						if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectService_Cmd Timeout even after retriggering");
							DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
							SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_currentEnsembleData,&DAB_Tuner_Ctrl_me->requestedinfo,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
							DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SERVICE_NOT_AVAILABLE, DAB_Tuner_Ctrl_me->st_currentEnsembleData);
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
						}
						/* For foreground Scan */
						else if (DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN)
						{
							if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
								|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
							{
								DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
								DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
								DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
								DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
								Update_Stationlist_Into_LearnMem();
								DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
							}
							else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Audio Play failed for first service");
								/* if there is no audio playing then transition to scan state to get next available freq or ensemble audio*/
								HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_active_busy_scan_state);
							}
						}
						else
						{
							/* For Misra */
						}
					}
				}
				break;

				default:
				{
					
				}
				break;

			}
		}
		break;
		
		case DAB_TUNER_CTRL_SELECT_AUDIO_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			
			if (DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN)
			{
				st_TimerId.u32_SelectService_Timer = SYS_StartTimer(DAB_SCAN_AUDIO_PLAY_TIME, DAB_TUNER_CTRL_STOP_SCAN_AUDIO, RADIO_DAB_TUNER_CTRL);
				if (st_TimerId.u32_SelectService_Timer == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_SERVICE_TIMEOUT");
				}
				else
				{
					/*MISRA C*/
				}
				/* To notify the first service of each scan ensemble to the DAB APP*/
				DAB_Tuner_Ctrl_Notify_AutoScan_PlayStation(REPLYSTATUS_SUCCESS, DAB_Tuner_Ctrl_me->st_currentEnsembleData);
			}
			else
			{
				/* updating the selected service and servcomponent in st_currentEnsembleData structure */
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId;
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI;
				DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
				DAB_Tuner_Ctrl_Enable_Fig_Notifications();
				DAB_Tuner_Ctrl_Enable_Quality_Notifications();
				DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0);
				DAB_Tuner_Ctrl_UpdateDABServiceType(&(DAB_Tuner_Ctrl_me->st_GetAudioProperties_repl), &(DAB_Tuner_Ctrl_me->st_currentEnsembleData.e_ServiceType));

				etal_ret = DabTunerCtrl_GetAudioStatus_req();
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					memset(&(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl), 0, sizeof(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl));
					DabTunerCtrl_GetAudioStatus_repl(&(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl));
				}

				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS, DAB_Tuner_Ctrl_me->st_currentEnsembleData);

				memset(&DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification, 0, sizeof(Ts_Tuner_Status_Notification));

				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s32_RFFieldStrength = (Ts32)DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.RFFieldStrength;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s32_BBFieldStrength = (Ts32)DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.BBFieldStrength;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.b_isValidMscBitErrorRatio = (Tbool)DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.isValidMscBitErrorRatio;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u32_MscBitErrorRatio = (Tu32)DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.MscBitErrorRatio;
				DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u32_FicBitErrorRatio = (Tu32)DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.FicBitErrorRatio;

				DAB_Tuner_Ctrl_Notify_TunerStatus(DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification);

				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_listen_state);
			}

		}
		break;

		/* for Foreground scan */
		case DAB_TUNER_CTRL_STOP_SCAN_AUDIO:
		{
			st_TimerId.u32_SelectService_Timer = 0;
			if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit 
				|| DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE)
			{
				DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
				DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
				DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
				Update_Stationlist_Into_LearnMem();
				DAB_Tuner_Ctrl_Check_AFtune_Scan(DAB_Tuner_Ctrl_me);
			}
			else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
			{
				/* After playing the audio for two seconds then transition to scan_state and continue the scan */
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_active_busy_scan_state);
			}
		}
		break;

		case HSM_MSGID_EXIT: 
		{
			
		} 
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;

	}
	return ret;
}

/*==============================================================================================================================*/
/*  Ts_Sys_Msg* (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, DAB_TUNER_CTRL_INST_HSM_SelCompHndlr  Ts_Sys_Msg* msg)   */
/*==============================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SelCompHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tbool b_resultcode = FALSE ;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_SelCompHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_SelCompHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_SelCompHndlr"));
			if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
			{
				(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI, DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
				DAB_Tuner_Ctrl_me->e_SelectService_Status = TUNER_CTRL_SERVICECOMPONENT_SELECTED;
				/* Starting timer for SelectComponent_repl time out */
				st_TimerId.u32_SelectComponentTimeout_SelComp_Timer = SYS_StartTimer(DAB_SELECT_COMPONENT_CMD_TIMEOUT_TIME, SELECT_COMPONENT_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_SelectComponentTimeout_SelComp_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_COMPONENT_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}

			}
			else
			{
				/* Doing Nothing */
			} /* For MISRA C */

		}
		break;
		case  SELECT_COMPONENT_TIMEOUT :
		{
			st_TimerId.u32_SelectComponentTimeout_SelComp_Timer = 0;
			/* Re-trigger SelectComponent_repl */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_COMPONENT_CMD_MAX_REPEAT_COUNT)
			{
				if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
				{
					(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI, DAB_Tuner_Ctrl_me->requestedinfo.u32_SId);
					/* Starting timer for SelectComponent_repl time out */
					st_TimerId.u32_SelectComponentTimeout_SelComp_Timer = SYS_StartTimer(DAB_SELECT_COMPONENT_CMD_TIMEOUT_TIME, SELECT_COMPONENT_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_SelectComponentTimeout_SelComp_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_COMPONENT_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				}
				else
				{
					/* For MISRA C */
				}
				
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectComponent_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),&(DAB_Tuner_Ctrl_me->requestedinfo),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));	 
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE,DAB_Tuner_Ctrl_me->requestedinfo);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);		
			}		
		}
		break;

		case DAB_TUNER_CTRL_SELECT_COMPONENT_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for SelectComponent_repl time out */
			if(SYS_StopTimer(st_TimerId.u32_SelectComponentTimeout_SelComp_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for SELECT_COMPONENT_TIMEOUT failed");	
			}
			else
			{
			 	st_TimerId.u32_SelectComponentTimeout_SelComp_Timer = 0;
			}				 
			DAB_Tuner_Ctrl_me->st_selectcompreply.reply  = 1;
			//DAB_Tuner_Ctrl_me->IsSearchNextDone = FALSE;
			DabTuner_MsgReply_SelectComponent(&(DAB_Tuner_Ctrl_me->st_selectcompreply),msg->data);
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] reply received for Seeking Scid is %d reply is %d",DAB_Tuner_Ctrl_me->st_seekInfo.u16_SCIdI,DAB_Tuner_Ctrl_me->st_selectcompreply.reply);
		  	DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ; 
			DabTunerCtrl_GetAudioProperties_req();
			st_TimerId.u32_Selcomp_Timer = SYS_StartTimer(DAB_GET_AUDIO_PROPERTIES_REPLY_TIME, DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_Selcomp_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT");	
			}
			else
			{
				/*MISRA C*/	
			}			
		}
		break;

		case DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY :
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for SelectComponent_repl time out */
			if(SYS_StopTimer(st_TimerId.u32_Selcomp_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT failed");	
			}
			else
			{
				 st_TimerId.u32_Selcomp_Timer = 0;
			}	
			 
			b_resultcode = DabTunerCtrl_GetAudioProperties_repl(&(DAB_Tuner_Ctrl_me->st_GetAudioProperties_repl), (Ts8*)msg->data);
			if(b_resultcode == TRUE)
			{ 
				/*	Triggering audio status request */
				if( SYS_StartTimer(200, TRIGGER_GET_AUDIO_STATUS_REQ, RADIO_DAB_TUNER_CTRL) <= 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TRIGGER_GET_AUDIO_STATUS_REQ");		
				}
				else
				{
					/*MISRA C*/
				}
			}
		}
		break;
	
		case DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT:
		{
			st_TimerId.u32_Selcomp_Timer = 0;
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_COMPONENT_CMD_MAX_REPEAT_COUNT)
			{
				DabTunerCtrl_GetAudioProperties_req();
				st_TimerId.u32_Selcomp_Timer = SYS_StartTimer(DAB_GET_AUDIO_PROPERTIES_REPLY_TIME, DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_Selcomp_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY_TIME_OUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0;
				if( SYS_StartTimer(200, TRIGGER_GET_AUDIO_STATUS_REQ, RADIO_DAB_TUNER_CTRL) <= 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TRIGGER_GET_AUDIO_STATUS_REQ");		
				}
				else
				{
					/*MISRA C*/
				}

			}
		}
		break;
	
	
		case TRIGGER_GET_AUDIO_STATUS_REQ :
		{
			(void)DabTunerCtrl_GetAudioStatus_req(); 	
			st_TimerId.u32_Selcomp_Timer = SYS_StartTimer(DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_Selcomp_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT");	
			}
			else
			{
				/*MISRA C*/	
			}

		}
		break;
		
		case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT:
		{
			st_TimerId.u32_Selcomp_Timer = 0;
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_COMPONENT_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTunerCtrl_GetAudioStatus_req();
				st_TimerId.u32_Selcomp_Timer = SYS_StartTimer(DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME, DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_Selcomp_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0 ;
				if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
				{
					SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),&(DAB_Tuner_Ctrl_me->requestedinfo),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));	 
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE,DAB_Tuner_Ctrl_me->requestedinfo);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);		
				}
				else
				{
						
				}/* For MISRA C */
			}	
			
		}
		break;
		
		case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY:
		{
			if(SYS_StopTimer(st_TimerId.u32_Selcomp_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY_TIME_OUT failed");	
			}
			else
			{
			 	st_TimerId.u32_Selcomp_Timer = 0;
			}			
			DabTunerCtrl_GetAudioStatus_repl(&(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl));
			
			if (DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
			{
				if((DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.ReplyStatus == (Tu8)(0x20))||(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.DecodingStatus == (Tu8)(2))||(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.AudioSynchronised != (Tu8)(1)))
				{
					if(DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger < (Tu8)GET_AUDIO_STATUS_REQ_MAX_TIME_RETRIGGER)
					{
						if( SYS_StartTimer(200, TRIGGER_GET_AUDIO_STATUS_REQ, RADIO_DAB_TUNER_CTRL) <= 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TRIGGER_GET_AUDIO_STATUS_REQ");		
						}
						else
						{
							/*MISRA C*/
						}
						DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger++ ;
					}
					else
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] In PlaySelectStation, station is unstable even after retriggering GetAudioStatus_Cmd ");
						DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
						if(DAB_Tuner_Ctrl_me->b_InitialStartUpFlag == TRUE) 
						{
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].ProgServiceId;
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId;
							if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].CharSet == 0x00)
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
							}
							else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].CharSet == 0x06)
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
							}
							else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].CharSet == 0x0f)
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
							}
							else
							{
							/*Doing nothing */ 	
							}/* For MISRA C */

							SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
							if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].CharSet == 0x00)
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
							}
							else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].CharSet == 0x06)
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
							}
							else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].CharSet == 0x0f)
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
							}
							else
							{
								/*Doing nothing */ 	
							}/* For MISRA C */
							SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency;
							//	DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId = DAB_Tuner_Ctrl_me->requestedinfo.u16_EId;
							DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;
							DAB_Tuner_Ctrl_Enable_Fig_Notifications();
							DAB_Tuner_Ctrl_Enable_Quality_Notifications();
							DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
							DAB_Tuner_Ctrl_UpdateDABServiceType(&(DAB_Tuner_Ctrl_me->st_GetAudioProperties_repl),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.e_ServiceType));
							//DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble = FALSE;
							DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_listen_state);
								
						}
						else
						{	
							SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),&(DAB_Tuner_Ctrl_me->requestedinfo),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
							DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE,DAB_Tuner_Ctrl_me->requestedinfo);
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
						}	
					}
				}
				else
				{		
					if((DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.DecodingStatus <= 1)&& (DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.AudioSynchronised == 1) && (DAB_Tuner_Ctrl_me->st_selectcompreply.reply == 0))
					{
						if(DAB_Tuner_Ctrl_me->b_InitialStartUpFlag == TRUE)
						{
							if(DAB_Tuner_Ctrl_me->requestedinfo.u32_SId == 0)
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].ProgServiceId;
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId;
								if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].CharSet == 0x00)
								{
									DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
								}
								else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].CharSet == 0x06)
								{
									DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
								}
								else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].CharSet == 0x0f)
								{
									DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
								}
								else
								{
								/*Doing nothing */ 	
								}/* For MISRA C */

								SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
								if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].CharSet == 0x00)
								{
									DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
								}
								else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].CharSet == 0x06)
								{
									DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
								}
								else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].CharSet == 0x0f)
								{
									DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
								}
								else
								{
									/*Doing nothing */ 	
								}/* For MISRA C */
								SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->requestedinfo.u32_Frequency;
								//	DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId = DAB_Tuner_Ctrl_me->requestedinfo.u16_EId;
							}
							else
							{
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId;
								DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI;
								DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
							}
							DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;
						}
						else
						{
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->requestedinfo.u32_SId;
							DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI = DAB_Tuner_Ctrl_me->requestedinfo.u16_SCIdI;
							DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
						}
			
						DAB_Tuner_Ctrl_Enable_Fig_Notifications();
						DAB_Tuner_Ctrl_Enable_Quality_Notifications();
						DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
						DAB_Tuner_Ctrl_UpdateDABServiceType(&(DAB_Tuner_Ctrl_me->st_GetAudioProperties_repl),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.e_ServiceType));
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_listen_state);

					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Select Component failed");
						DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
						SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),&(DAB_Tuner_Ctrl_me->requestedinfo),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE,DAB_Tuner_Ctrl_me->requestedinfo);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					}
				}
			}
		}
		break;	

		case DAB_TUNER_CTRL_SYNCHRONISATION_NOTIFICATION:
		{
			DabTuner_MsgRcvSetSynchronisationNotification(&(DAB_Tuner_Ctrl_me->st_synchNotification),msg->data);
			DAB_Tuner_Ctrl_Notify_Synchronisation(DAB_Tuner_Ctrl_me->st_synchNotification.ChannelSync);
		}
		break;


		case HSM_MSGID_EXIT:  
		{
			
		}
		break;
		
		default:
		{
			/* in top state throw system error */
			ret = msg; 
		}		  
		break;
	}
	return ret;
}

/*==========================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*==========================================================================================================================*/

Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);

	switch (msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str, "DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr", sizeof("DAB_TUNER_CTRL_INST_HSM_AutoSeekHndlr"));
			
			if (DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEEK )
			{
				etal_ret = DabTuner_AutoSeekStart_Cmd(DAB_Tuner_Ctrl_me->e_SeekDirection, DAB_Tuner_Ctrl_me->b_AutoSeekStart);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					/* Stop the timer if it's already running */
					if (st_TimerId.u32_AutoseekNotify_Timer > 0)
					{
						if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
						}
						else
						{
							st_TimerId.u32_AutoseekNotify_Timer = 0;
						}
					}
					else
					{
						/* MISRA C*/
					}
					/* Start the timer */
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, AUTOSEEK_CMD_REPLY_TIMEOUT);
				}
			}
			
		}
		break;

		case AUTOSEEK_CMD_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] AUTOSEEK_CMD_TIMEOUT - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_IN_PROGRESS:
				{
					// Retry the commands for 2 times 
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_AUTOSEEK_NOTIFY_CMD_MAX_REPEAT_COUNT)
					{
						etal_ret = DabTuner_AutoSeekStart_Cmd(DAB_Tuner_Ctrl_me->e_SeekDirection, DAB_Tuner_Ctrl_me->b_AutoSeekStart);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, AUTOSEEK_CMD_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] AUTOSEEK FAILED  ");

						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

						/* Clear Component label in "st_currentEnsembleData" structure */
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
					
						/* Sending tune OR AUTO SEEK fail response to DAB Application */
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->st_currentEnsembleData);

						/* Transiting to dab_tuner_ctrl_inst_hsm_Nosignallisten_state */
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
															
						//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
					}
				}
				break;

				default:
				{
					/* To avoid Warnings */
				}
				break;
			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT:
		{
			st_TimerId.u32_AutoseekNotify_Timer = 0;
			// Retry the commands for 2 times 
			if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_AUTOSEEK_NOTIFY_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTuner_AutoSeekStart_Cmd(DAB_Tuner_Ctrl_me->e_SeekDirection, DAB_Tuner_Ctrl_me->b_AutoSeekStart);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, AUTOSEEK_CMD_REPLY_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] AUTOSEEK FAILED  ");

				memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

				/* Clear Component label in "st_currentEnsembleData" structure */
				memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

				/* Sending tune OR AUTO SEEK fail response to DAB Application */
				DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->st_currentEnsembleData);

				/* Transiting to dab_tuner_ctrl_inst_hsm_Nosignallisten_state */
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);

				//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT:
		{
			st_TimerId.u32_AutoseekNotify_Timer = 0;
			// Retry the commands for 2 times for to get the Autoseek notification purpose
			if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_AUTOSEEK_NOTIFY_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTuner_AbortAutoSeek_Cmd();
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_AUTOSEEK_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{

				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/*if cancel request is SEEK CANCEL */
				if (DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_TUNE_CANCEL)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Abort Scan sucess ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
					DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
					DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] AUTOSEEK FAILED  ");

					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

					/* Clear Component label in "st_currentEnsembleData" structure */
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

					/* Sending tune OR AUTO SEEK fail response to DAB Application */
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->st_currentEnsembleData);

					/* Transiting to dab_tuner_ctrl_inst_hsm_Nosignallisten_state */
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);

					//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION:
		{
		
			if (st_TimerId.u32_AutoseekNotify_Timer > 0)
			{
				if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
				}
				else
				{
					st_TimerId.u32_AutoseekNotify_Timer = 0;
				}
			}
			else
			{
				/* MISRA C*/
			}
			DAB_Tuner_Ctrl_me->Index = 0;
			/*clearing the seekstatus structure before getting the event*/
			memset(&(DAB_Tuner_Ctrl_me->st_SeekStatus), 0, sizeof(EtalSeekStatus));
			// extracting the seek status data and copying into st_SeekStatus structure 
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_SeekStatus), (msg->data), sizeof(DAB_Tuner_Ctrl_me->st_SeekStatus), &(DAB_Tuner_Ctrl_me->Index));

			if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_status == ETAL_SEEK_RESULT)
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData), 0, sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					/* copy every step frequency to notify*/
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency;

				/* Notifying the frequency to be tuned by the autoseek FOR EVERY STEP SIZE FREQ, to DAB application */
				DAB_Tuner_Ctrl_Notify_FrequencyChange(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);

				if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequencyFound == TRUE) /* if frequency is detected */
				{
					/* copying the dab qualiy parametrs in local structure */
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.RFFieldStrength			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_RFFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.BBFieldStrength			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_BBFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.FicBitErrorRatio			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_FicBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.isValidMscBitErrorRatio	= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_isValidMscBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.MscBitErrorRatio			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_MscBitErrorRatio;

					etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
					}
				}
			}
			if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_status == ETAL_SEEK_FINISHED)
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/*if cancel request is SEEK CANCEL */
				if (DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_TUNE_CANCEL)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Abort Scan sucess ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
					DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;
					DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_fullCycleReached == TRUE && DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequencyFound == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] AUTOSEEK FINISHED Full cycle reached but freq not found");
					/* when full cycle is reached freq found is false then it means it is a result 
					of an autoseekstop cmd, Send response as NO SIGNAL to DAB APP */
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
					DAB_Tuner_Ctrl_me->b_InitialStartUpFlag = FALSE;

					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData), 0, sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
					/* copy every step frequency to notify*/
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency;
					/* Notifying the frequency to be tuned by the autoseek freq, to DAB application */
					/* Doing this in FINISHED only for this condition - as for full cycle reached and freq not found, RESULT is not sent*/
					/* For all the other cases RESULT will notify the frequency to DAB APP */
					DAB_Tuner_Ctrl_Notify_FrequencyChange(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);

					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

					/* Clear Component label in "st_currentEnsembleData" structure */
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

					/* Sending tune OR AUTO SEEK fail response to DAB Application */
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->st_currentEnsembleData);

					/* Transiting to dab_tuner_ctrl_inst_hsm_Nosignallisten_state */
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
				}
				else
				{
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

			/* Command to get the ensemble properties */
			etal_ret = DabTuner_MsgSndGetEnsembleProperties();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
			}

		}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB GET CURR ENSEMBLE FAILED - %d", etal_ret);

			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					//Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the uEID */
						etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

						/* Clear Component label in "st_currentEnsembleData" structure */
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT FAILED  ");
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->requestedinfo);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
					}
				}
				break;
				
				default:
				{

				}
				break;
			}
		}
		break;

		case GET_ENSEMBLE_PROPERTIES_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

			/* Extracting the msg and updating st_GetEnsembleProperties_reply */
			DabTuner_MsgReplyGetEnsembleProperties(&(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply));

			/* Updating current ensemble label */
			SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.au8_label), (DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.LabelString), DAB_TUNER_CTRL_MAX_LABEL_LENGTH);

			/* Updating current ensemble EId */
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId = DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.EnsembleIdentifier;

			/* Updating current ensemble ECC */
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC = DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.ECC;

			if (DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
			}
			else if (DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x06)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
			}
			else if (DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x0f)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;
			}
			else
			{
				/*Doing nothing */
			}/* For MISRA C */


			/* Transiting to Idle state dab_tuner_ctrl_inst_hsm_GetCurrEnsembleProgmList_state */
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_GetCurrEnsembleProgmList_state);
		}
		break;

		case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ETAL DAB ENSEMBLE PROPERTIES FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_NO_DATA:
				{
					//Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the ensemble properties */
						etal_ret = DabTuner_MsgSndGetEnsembleProperties();

						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetEnsembleProperties_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

						/* Clear Component label in "st_currentEnsembleData" structure */
						memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_NO_SIGNAL, DAB_Tuner_Ctrl_me->requestedinfo);
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_Nosignallisten_state);
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;


		case DAB_TUNER_CTRL_CANCEL_REQID:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_CancelType), (msg->data), sizeof(Te_DAB_Tuner_Ctrl_CancelType), &(DAB_Tuner_Ctrl_me->Index));
			if (DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_TUNE_CANCEL )
			{
				/* Send Abort AutoSeek request to Soc */
				etal_ret = DabTuner_AbortAutoSeek_Cmd();
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					/* Stop the timer if it's already running */
					if (st_TimerId.u32_AutoseekNotify_Timer > 0)
					{
						if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
						}
						else
						{
							st_TimerId.u32_AutoseekNotify_Timer = 0;
						}
					}
					else
					{
						/* MISRA C*/
					}
					/* Start the timer */
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_AUTOSEEK_TIMEOUT);
				}
			}
			else
			{
				/* For MISRA */
			}
		}
		break;

		case ABORT_AUTOSEEK_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ABORT_AUTOSEEK_TIMEOUT - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					if (DAB_Tuner_Ctrl_me->e_CancelType == DAB_TUNER_CTRL_TUNE_CANCEL)
					{
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					}
					else
					{

					}
					DAB_Tuner_Ctrl_me->e_CancelType = DAB_TUNER_CTRL_CANCEL_INVALID;
					DAB_Tuner_Ctrl_Response_Cancel(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				break;

				default:
				{
					/* To avoid Warnings */
				}
				break;
			}
		}
		break;


		case HSM_MSGID_EXIT:
		{

		}
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}






/*==========================================================================================================================*/
/*  Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SleepHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*==========================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_SleepHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg); 

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_SleepHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_SleepHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_SleepHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_SleepHndlr"));
		}
		break;

		case DAB_TUNER_CTRL_ACTIVATE_REQID :
		{
			DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;
		
		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;
	}
	return ret;
}

/*=========================================================================================================================*/
/* Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_ErrHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)    */
/*=========================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_ErrHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg); 
	
	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
		
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_ErrHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_ErrHndlr";
			SYS_RADIO_MEMCPY( (void*)DAB_Tuner_Ctrl_me->u8_curr_state_str ,"DAB_TUNER_CTRL_INST_HSM_ErrHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_ErrHndlr"));
		}
		break;
		
		case DAB_TUNER_CTRL_INST_SHUTDOWN :
		{
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;
		
		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;
	}
	return ret;
}


void DAB_Tuner_Ctrl_Clear_Stationlist(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	ensembleIndex = 0; DAB_Tuner_Ctrl_me->serviceindx = 0;DAB_Tuner_Ctrl_me->CompIndex = 0; Services = 0; Components = 0;
	DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
	memset(&(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_ScanNotification),0,sizeof(Ts_DabTunerMsg_R_ScanStatus_Not));
	memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
	memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
	memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
	memset(&(st_ScanEnsembleinfo), 0, sizeof(Ts_DabTunerGetEnsembleProperties_reply));
	memset(&(st_ScanServiceinfo), 0, sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
	
}

/*================================================================================================================================*/
/*Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_BackgroundScanHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg) */
/*================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_BackgroundScanHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL;
	Tbool b_resultcode = FALSE;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_BG_STATE ;
			DAB_Tuner_Ctrl_me->b_ActivateRequest = FALSE ;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_BackgroundScanHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_BackgroundScanHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_BackgroundScanHndlr"));
			DAB_Tuner_Ctrl_Disable_Fig_Notifications();
			DAB_Tuner_Ctrl_Disable_Quality_Notifications();
			DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);
			
			if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
			{
			 if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
			 {
			  	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
			 }
			 else
			 {
			 	st_TimerId.u32_StartBackGrndScan_Timer = 0;	
			 }			 
			}
			else
			{
				/* MISRA C*/	
			}
			st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_TWO_SECS_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
			}
			else
			{
				/*MISRA C*/	
			}
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
		}
		break;
		
		case SYSTEM_CONTORL_NOTIFICATION:
		 {
			b_resultcode = DabTunerCtrl_SystemMonitoring_not(&(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not), (Ts8*)(msg->data));
			DAB_Tuner_Ctrl_me->e_SystemMonitoringNotReceived = DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_RECEIVED ;
			/* Store the current state of DAB_Tuner_Ctrl */
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_BG_STATE ;
			
			//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] System monitoring notification MonitorId = %x Status = %x ",DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId, DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status );
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x05)
				{
					/* Out going messages dropped by DABTuner */
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status > 2000) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DAB TUNER in abnormal state");
						/* Store the current state of DAB_Tuner_Ctrl */
						DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				}
		#if 1
				/* Audio-out event notifications */
				else if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x20)
				{
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status == 0x10) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DAB TUNER in abnormal state due to Audio-out event change");
						DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,QUALITY_PARAMETERS);
						/* Store the current state of DAB_Tuner_Ctrl */
						DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				}
		#endif 
		#if 0
				/* system overload event notifications */
				else if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.MonitorId == 0x22)
				{
					if(DAB_Tuner_Ctrl_me->st_SystemMonitoring_not.Status == 0x04 ) /* Value need to be changed once verified from DABTuner API Doc*/
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][DAB_TC] DAB TUNER in abnormal state");
						/* Store the current state of DAB_Tuner_Ctrl */
						DAB_Tuner_Ctrl_me->e_ComponentStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL ;
						/* Send Notification to DAB App that DABTuner is Abnormal*/
						DAB_Tuner_Ctrl_Notify_ComponentStatus(DAB_Tuner_Ctrl_me->e_ComponentStatus) ;
						/* Transit to Inactive state*/
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */					
				}				
		#endif 						
				else
				{
					/* Doing nothing */	
				}/* For MISRA C */
			}
			else
			{
				/* FOR MISRA */
			}
		}
		break;

		case DAB_TUNER_CTRL_DAB_AF_SETTINGS_REQID:
		{
			DAB_Tuner_Ctrl_me->Index =0;  
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_DAB_AF_Settings),&(DAB_Tuner_Ctrl_me->Index));
			DAB_Tuner_Ctrl_Response_DAB_AF_Settings(REPLYSTATUS_SUCCESS);
		}
		break;

		case DAB_TUNER_CTRL_FM_DAB_PI:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u16_FM_DAB_SID),(msg->data),sizeof(Tu16),&(DAB_Tuner_Ctrl_me->Index));
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->u16_FM_DAB_SCID),(msg->data),sizeof(Tu16),&(DAB_Tuner_Ctrl_me->Index));
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] FM PI received notification SID:%x SCID:%x",DAB_Tuner_Ctrl_me->u16_FM_DAB_SID,DAB_Tuner_Ctrl_me->u16_FM_DAB_SCID);
			DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_INVALID;
			DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_RECEIVED;
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_END)
			{
				DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus = DAB_TUNER_CTRL_FM_DAB_PI_RECEIVED;
			}
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_Linking_state);
		}
		break;

		case DAB_TUNER_CTRL_STRATERGY_STATUS_NOTIFYID:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus),(msg->data),sizeof(Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus),&(DAB_Tuner_Ctrl_me->Index));
		
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_END)
			{
				if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
				{
	 				if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
					{
	 					 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
	 				}
	 				else
					 {
					 	st_TimerId.u32_StartBackGrndScan_Timer = 0;
					 }								
				}
				else
				{
					/* MISRA C*/	
				}
				st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_HALF_SEC_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
				}
				else
				{
					/*MISRA C*/	
				}
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
			}
			else
			{
				/* FOR MISRA */
			}
		}
		break;

		case DAB_TUNER_CTRL_FM_DAB_STOP_LINKING:
		{
			DAB_Tuner_Ctrl_me->Index = 0;
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->e_FmtoDAB_Reqstatus),(msg->data),sizeof(Te_Bool),&(DAB_Tuner_Ctrl_me->Index));
			
			if(DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent == TRUE)
			{
				DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = FALSE;
			}
				
			if(DAB_Tuner_Ctrl_me->e_FmtoDAB_Reqstatus == DAB_TUNER_CTRL_FMDAB_LINKING_REQ_SIG_LOST)
			{
				DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus = DAB_TUNER_CTRL_FM_DAB_PI_RECEIVED;
				DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_RECEIVED;
				
			}
			else
			{
				DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_BLENDING_CANCELLED;
				DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus = DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_END;
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Stop Monitoring tuned station and starting background scan");
			}
			if(DAB_Tuner_Ctrl_me->e_Scannstatus == TUNER_CTRL_SCAN_COMPLETED)
			{
				if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
				{
	 				if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
					{
	 					 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
	 				}
	 				else
					 {
					 	st_TimerId.u32_StartBackGrndScan_Timer = 0;
					 }								
				}
				else
				{
					/* MISRA C*/	
				}
				st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_HALF_SEC_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
				}
				else
				{
					/*MISRA C*/	
				}
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Stop FM_DAB Linking is received and already background scan is ongoing");
			}
		}
		break;
		case DAB_TUNER_CTRL_ACTIVATE_REQID:
		{
			etal_ret = DabTunerCtrl_Audio_Source_Select_cmd();
			if (etal_ret != ETAL_RET_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB Audio Source Select Failed! ");
			}
			DAB_Tuner_Ctrl_me->b_ActivateRequest = TRUE ;
			/* Clear all FM-DAB linking related structures */
			DAB_Tuner_Ctrl_Clear_FMDAB_LinkingData(DAB_Tuner_Ctrl_me);

			/* Checking if mutex lock is held by GetCompList handler */
			if (b_bgGetCompList_handler == TRUE)
			{
				SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
				b_bgGetCompList_handler = FALSE;
			}

			/* If scan is in progress then giving the Abort scan command to DAB Tuner */
			if(DAB_Tuner_Ctrl_me->b_ScanInProgress == TRUE)
			{
 				etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					DAB_Tuner_Ctrl_me->b_ScanInProgress = FALSE;

					st_TimerId.u32_AutoseekStopNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekStopNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION STOP");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN_TIMEOUT);
				}
			}
			else /* If scan is not in progress then transiting to idle state */
			{
				DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
				DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
			 	HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
		}
		break;	

		case ABORT_SCAN_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ABORT_SCAN_TIMEOUT - %d", etal_ret);
			st_TimerId.u32_AutoseekStopNotify_Timer = 0;
			
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					//st_TimerId.u32_AbortScan_BackgroundScan_Timer = 0;
					/* Re-trigger AbortScan2_Cmd */
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_ABORT_SCAN_CMD_MAX_REPEAT_COUNT)
					{
						etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							st_TimerId.u32_AutoseekStopNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekStopNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						if (DAB_Tuner_Ctrl_me->b_ActivateRequest == TRUE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] AbortScan2_Cmd Timeout even after retriggering");
							DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;

							DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
							DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
#if 0
							//DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_REQ_ERROR);
							DAB_Tuner_Ctrl_Response_Abort_Scan();
							if (st_TimerId.u32_StartBackGrndScan_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");
								}
								else
								{
									st_TimerId.u32_StartBackGrndScan_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_TWO_MIN_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_StartBackGrndScan_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");
							}
							else
							{
								/*MISRA C*/
							}
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
#endif
						}
						else
						{
							/* if autoseek stop is FAILED when full cycle is reached successfully then send
							send the response and notify then stationlist */
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC]  Scan ended sucessfully ");
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
							DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_COMPLETED;
							DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
							DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
						}
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT:
		{
			st_TimerId.u32_AutoseekStopNotify_Timer = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB AUTOSEEK_STOP_NOTIFY_TIMEOUT");
			// Retry the commands for 2 times for to get the Autoseek notification purpose
			if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SCAN_NOTIFY_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					st_TimerId.u32_AutoseekStopNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekStopNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						st_TimerId.u32_AutoseekStopNotify_Timer = 0;
					}

				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{ 
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				if (DAB_Tuner_Ctrl_me->b_ActivateRequest == TRUE)
				{
					DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
					DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
#if 0
					DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_REQ_ERROR);
					DAB_Tuner_Ctrl_Response_Abort_Scan();
					if (st_TimerId.u32_StartBackGrndScan_Timer > 0)
					{
						if (SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");
						}
						else
						{
							st_TimerId.u32_StartBackGrndScan_Timer = 0;
						}
					}
					else
					{
						/* MISRA C*/
					}
					st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_TWO_MIN_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_StartBackGrndScan_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");
					}
					else
					{
						/*MISRA C*/
					}
#endif
				}
				else
				{
					/* if autoseek stop is FAILED when full cycle is reached successfully then send 
					   send the response and notify then stationlist */
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC]  Scan ended sucessfully ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
					DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
					DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
					Update_Stationlist_Into_LearnMem();
					DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_COMPLETED;
					DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
					DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_BACKGROUND_SCAN_START_NOTIFYID:
		{		
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Non-Radio/Power OFF mode ");
			/* FM DAB Follow up things need to be taken care if radio enter into Non-radio/Power OFF state */
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus = DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_END;/*If radio enter into Non-radio/Power OFF state we are clearing the linking status of FM-DAB follow up*/
			memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			memset((DAB_Tuner_Ctrl_me->st_fmdab_linkinfo),0,sizeof(Ts_dab_tuner_ctrl_fmdab_linkinfo));
			if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
			{
			 	if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
				{
			  		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
			 	}
			 	else
			 	{
			 		st_TimerId.u32_StartBackGrndScan_Timer = 0;	
			 	}			 
			}
			else
			{
				/* MISRA C*/	
			}
			st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_HALF_SEC_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
			}
			else
			{
				/*MISRA C*/	
			}
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
		}
		break;
			
		case HSM_MSGID_EXIT: 
		{
			
		} 
		break;

		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;
	}
	return ret;
}

void DAB_Tuner_Ctrl_Transition_to_Scanstate(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	memset(&(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo),0,sizeof(Ts_CurrentSidLinkingInfo));
	ALternateFreq = 0;
	u8_Alt_Ensemble_ID = 0;
//	AltEnsembleFreqindex 	= 0;
	AltEnsembleindex		= 0;
//	HardlinkFreqindex 		= 0;
//	HardlinkEnseindex 		= 0;
	b_Hardlinksused 		= FALSE;
	Hardlinkindex			= 0;
	Altfreqindex			= 0;
	HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me,&dab_tuner_ctrl_inst_hsm_active_busy_scan_state);
}


void DAB_Tuner_Ctrl_Check_AFtune_Scan(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN)
	{
		DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
		HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
	}
	else if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE)
	{
		if(DAB_Tuner_Ctrl_me->e_ReplyStatus == REPLYSTATUS_SUCCESS)
		{
			/* Updating the Station List during AF Tune */
			DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);//, DAB_Tuner_Ctrl_me->e_RequestCmd);
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_AFTUNE_END;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_lsmdata.u32_Frequency;
			DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
			//DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = 0;
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
		}
		else
		{
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = 0;
			DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_FAIL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
		}	
	}
	else if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_ESD_CHECK)
	{
		if(ensembleIndex > 0)
		{
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_DAB_blending_state);
		}
		else
		{
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_AllBand_tune_state);
			
		}
		
	}
	else
	{
		HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
	}

}
void DAB_Tuner_Ctrl_Check_Request_Type(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_START)
	{
		if(DAB_Tuner_Ctrl_me->e_ReplyStatus == REPLYSTATUS_SUCCESS)
		{
			DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_COMPLETED;
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_Linking_state);
		}
		else
		{
			DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_FAIL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
			if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
			{
		 		if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
		 		{
		  			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
				}
		 		else
		 		{
		 			st_TimerId.u32_StartBackGrndScan_Timer = 0;
		 		}					
			}
			else
			{
				/* MISRA C*/	
			}
			st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_TWO_MIN_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
			}
			else
			{
				/*MISRA C*/
			}
		}
		
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_FM_DAB_PI_RECEIVED)
	{
		if(DAB_Tuner_Ctrl_me->e_ReplyStatus == REPLYSTATUS_SUCCESS)
		{
			//Update_Stationlist_Into_LearnMem();
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_FM_DAB_Linking_state);
		}
		else
		{
			if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
			{
		 		if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
		 		{
		  			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
				}
		 		else
		 		{
		 			st_TimerId.u32_StartBackGrndScan_Timer = 0;
		 		}					
			}
			else
			{
				/* MISRA C*/	
			}
			st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_TWO_MIN_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
			}
			else
			{
				/*MISRA C*/
			}
		}
		
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_END)
	{
		if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
		{
	 		if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
	 		{
	  			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
			}
	 		else
	 		{
	 			st_TimerId.u32_StartBackGrndScan_Timer = 0;
	 		}					
		}
		else
		{
			/* MISRA C*/	
		}
		st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_TWO_MIN_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
		if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
		}
		else
		{
			/*MISRA C*/
		}
		HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
		
	}
	else
	{
		if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
		{
	 		if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
	 		{
	  			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
			}
	 		else
	 		{
	 			st_TimerId.u32_StartBackGrndScan_Timer = 0;
	 		}					
		}
		else
		{
			/* MISRA C*/	
		}
		st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_TWO_MIN_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
		if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");	
		}
		else
		{
			/*MISRA C*/
		}
		HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
	}
	
}

/*=============================================================================================================================================*/
/*Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_backgrndEnsembleScanStateHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)   */
/*=============================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_backgrndEnsembleScanStateHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu8 labelIndex =0;
	Tu16  serviceIndex;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}	
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_backgrndEnsembleScanStateHndlr ");
			/* Clear all FM-DAB linking related structures */
			DAB_Tuner_Ctrl_Clear_FMDAB_LinkingData(DAB_Tuner_Ctrl_me);
		}
		break;

		case DAB_TUNER_CTRL_START_BACKGRND_SCAN:
		{
			st_TimerId.u32_StartBackGrndScan_Timer = 0;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Background scan started");
			DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_LEARN;
			/* Clear Currentensemble data structure */
			memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
			SYS_RADIO_MEMCPY((void*) DAB_Tuner_Ctrl_me->u8_curr_state_str ,"dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state",sizeof("dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state"));
			if(DAB_Tuner_Ctrl_me->e_Scannstatus == TUNER_CTRL_SCAN_COMPLETED)
			{
				DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);	
			}
			else
			{
				/* Do nothing*/
				/*Misra C*/
			}
			if(DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
			{
				if(DAB_Tuner_Ctrl_me->b_scanstarted == FALSE)
				{
					DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit = 239200;
					DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);
					/* Send Tune Cmd so that ETAL seek cmd's start frequency will get updated */
					etal_ret = DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit);
					while(etal_ret == ETAL_RET_ERROR)
					{
						etal_ret = DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit);
					}
				}
				else
				{
					/* Do nothing*/
					/*Misra C*/
				}
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC]  Background Scan started ");
				etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
				DAB_Tuner_Ctrl_me->b_ScanInProgress = TRUE;
				DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_STARTED;
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					/* Stop the timer if it's already running */
					if (st_TimerId.u32_AutoseekNotify_Timer > 0)
					{
						if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
						}
						else
						{
							st_TimerId.u32_AutoseekNotify_Timer = 0;
						}
					}
					else
					{
						/* MISRA C*/
					}
					/* Start the timer */
					DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
				}
				
				
			}
			else
			{
				DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
				DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}/* For MISRA C */
		}
		break;	

		case SCAN_NOTIFY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SCAN_NOTIFY_TIMEOUT - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_IN_PROGRESS:
				{
					// Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SCAN_NOTIFY_CMD_MAX_REPEAT_COUNT)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ScanStatus2_Cmd Timeout even after retriggering");
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						DAB_Tuner_Ctrl_me->b_ScanInProgress = FALSE;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
					}
				}
				break;

				case ETAL_RET_NO_DATA:
				{
					DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;
					if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
					}
					else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_COMPLETED;
						DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
					}
					else
					{

					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION:
		{
			DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_NOTIFICATION_REEIVED;

			if (st_TimerId.u32_AutoseekNotify_Timer > 0)
			{
				if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
				}
				else
				{
					st_TimerId.u32_AutoseekNotify_Timer = 0;
				}
			}
			else
			{
				/* MISRA C*/
			}

			DAB_Tuner_Ctrl_me->Index = 0;
			/*clearing the seekstatus structure before getting the event*/
			memset(&(DAB_Tuner_Ctrl_me->st_SeekStatus), 0, sizeof(EtalSeekStatus));
			// extracting the seek status data and copying into st_SeekStatus structure 
			ExtractParameterFromMessage(&(DAB_Tuner_Ctrl_me->st_SeekStatus), (msg->data), sizeof(DAB_Tuner_Ctrl_me->st_SeekStatus), &(DAB_Tuner_Ctrl_me->Index));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION Received for status %d", DAB_Tuner_Ctrl_me->st_SeekStatus.m_status);
			if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_status == ETAL_SEEK_RESULT)
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* Checking for full cycle is reached or not */
				if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
				{
					DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
					etal_ret = DabTuner_AbortScan2_Cmd(DAB_Tuner_Ctrl_me->e_RequestCmd);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
						if (st_TimerId.u32_AutoseekNotify_Timer == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
						}
						else
						{
							/*MISRA C*/
						}
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, ABORT_SCAN_TIMEOUT);
					}
				}
				else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequencyFound == TRUE) /* if frequency is detected */
				{
					/* copying the dab qualiy parametrs in local structure */
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.RFFieldStrength 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_RFFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.BBFieldStrength 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_BBFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.FicBitErrorRatio 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_FicBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.isValidMscBitErrorRatio 	= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_isValidMscBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.MscBitErrorRatio 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_MscBitErrorRatio;
					etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
					if(etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
					}
				}
			}
			else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_status == ETAL_SEEK_FINISHED)
			{
				if (st_TimerId.u32_AutoseekStopNotify_Timer > 0)
				{
					if (SYS_StopTimer(st_TimerId.u32_AutoseekStopNotify_Timer) == FALSE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
					}
					else
					{
						st_TimerId.u32_AutoseekStopNotify_Timer = 0;
					}
				}
				else
				{
					/* MISRA C*/
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_Tuner_Ctrl_me->b_ScanInProgress = FALSE;
				/* when Activate req is TRUE Abort_scancmd(autoseek_stop) is called to stop the ongoing seek
				then ETAL_SEEK_FINISHED is triggered then send activate response and transit to idle state  */
				if(DAB_Tuner_Ctrl_me->b_ActivateRequest == TRUE)
				{
					DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
					DAB_Tuner_Ctrl_Response_Abort_Scan();
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequencyFound == TRUE) /* if frequency is detected  when full cycle is reached*/
				{
					/* copying the dab qualiy parametrs in local structure */
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.RFFieldStrength 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_RFFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.BBFieldStrength 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_BBFieldStrength;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.FicBitErrorRatio 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_FicBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.isValidMscBitErrorRatio 	= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_isValidMscBitErrorRatio;
					DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.MscBitErrorRatio 			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_quality.EtalQualityEntries.dab.m_MscBitErrorRatio;
					etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
					}
				}
					/*	if full cycle is reached but freq not found then Abort_scancmd(autoseek_stop) is called to stop
						autoseek properly then  ETAL_SEEK_FINISHED is triggered and handled here */
				else if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequencyFound != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC]  Scan ended sucessfully ");
					DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
					DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
					DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
					DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
					Update_Stationlist_Into_LearnMem();
					DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_COMPLETED;
					DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
					DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
				}
				else
				{
					/* do nothing*/
				}
			}
		}
		break;

		case DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT:
		{
			st_TimerId.u32_AutoseekNotify_Timer = 0;
			// Retry the commands for 2 times 
			if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SCAN_NOTIFY_CMD_MAX_REPEAT_COUNT)
			{
				etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if (st_TimerId.u32_AutoseekNotify_Timer == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] StartScan2_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_REQ_TIMEOUT;
				DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
				DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
				DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
				Update_Stationlist_Into_LearnMem();
				DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
				DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
				//HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Command to get the ensemble properties */
			etal_ret = DabTuner_MsgSndGetEnsembleProperties();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
			}

		}
		break;

		case GET_ENSEMBLE_PROPERTIES_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Extracting the msg and updating st_ScanEnsembleinfo */
			DabTuner_MsgReplyGetEnsembleProperties(&(st_ScanEnsembleinfo));
			
			if (ensembleIndex < DAB_APP_MAX_ENSEMBLES)
			{
				DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;

				/* Updating Ensemble list structure */

					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);

					ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u32_Frequency			= DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency;
					ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u16_EId					= st_ScanEnsembleinfo.EnsembleIdentifier;
					ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.RSSI					= DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.RFFieldStrength;
					ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u8_ECC					= st_ScanEnsembleinfo.ECC;
					ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u16_ShortLabelFlags	= st_ScanEnsembleinfo.ShortLabelFlags;

					if (st_ScanEnsembleinfo.CharSet == 0x00)
					{
						ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
					}
					else if (st_ScanEnsembleinfo.CharSet == 0x06)
					{
						ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
					}
					else if (st_ScanEnsembleinfo.CharSet == 0x0f)
					{
						ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;
					}
					else
					{
						/*Doing nothing */
					}/* For MISRA C */
					for (labelIndex = 0; labelIndex < 16; labelIndex++)
					{
						ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.au8_label[labelIndex] = st_ScanEnsembleinfo.LabelString[labelIndex];
					}

					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					/*Command to get service list*/
					etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
					}

				if (DAB_Tuner_Ctrl_me->b_ActivateRequest == TRUE)
				{
					DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
				}
				else
				{
					//MISRA
				}
			}
			else
			{

			}
		}
		break;

		case GET_PROGRAM_SERVLIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(&(st_ScanServiceinfo));
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			DAB_Tuner_Ctrl_me->serviceindx = Services;
			DAB_Tuner_Ctrl_me->currentserviceindx = 0;
			/* Updating Service List id's in stationlist Structure */
			for (serviceIndex = 0; serviceIndex < st_ScanServiceinfo.u8_NumOfServices; serviceIndex++, Services++)
			{
				if (Services < DAB_APP_MAX_SERVICES)
				{
					SYS_RADIO_MEMCPY(&(ast_ServiceInfo[Services].st_BasicEnsInfo), &(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo), sizeof(ast_ServiceInfo[Services].st_BasicEnsInfo));
					//Ts_Tuner_Ctrl_BasicEnsembleInfo
					ast_ServiceInfo[Services].u32_SId = st_ScanServiceinfo.st_serviceinfo[serviceIndex].ProgServiceId;//DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_ScanNotification.t_sat_ScanStatusNot[serviceIndex].ServiceId;
				}
			}
			if (DAB_Tuner_Ctrl_me->b_ActivateRequest == TRUE)
			{
				DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_idle_state);
			}
			if (st_ScanServiceinfo.u8_NumOfServices > 0)
			{
				HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndScan_GetcomponentListState);
			}
			else if (st_ScanServiceinfo.u8_NumOfServices == 0)
			{
				SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
				b_bgGetCompList_handler = FALSE;	//mutex released
				if(DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
				{
					DAB_Tuner_Ctrl_me->b_ScanInProgress = TRUE;
					DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_STARTED;
					if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
					}
					else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
					}
				}
				else
				{
					/* do nothing */	
				}/*For MISRA C */
			}
			else
			{
				/* do nothing */
			}
		}
		break;

		case DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB GET CURR ENSEMBLE FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					//Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the uEID */
						etal_ret = DabTuner_Msg_GetCurrEnsemble_cmd();
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_CURR_ENSMB_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB ENSEMBLE FAILED  ");
						if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit && DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
						{
							etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
								if (etal_ret == ETAL_RET_SUCCESS)
								{
									/* Stop the timer if it's already running */
									if (st_TimerId.u32_AutoseekNotify_Timer > 0)
									{
										if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
										{
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
										}
										else
										{
											st_TimerId.u32_AutoseekNotify_Timer = 0;
										}
									}
									else
									{
										/* MISRA C*/
									}
									/* Start the timer */
									st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
									if (st_TimerId.u32_AutoseekNotify_Timer == 0)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
									}
									else
									{
										/*MISRA C*/
									}
								}
								else
								{
									DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
								}
						}
						else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
						{
							DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
							DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
						}
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

		case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] ETAL DAB ENSEMBLE PROPERTIES FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					//Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT)
					{
						/* Command to get the ensemble properties */
						etal_ret = DabTuner_MsgSndGetEnsembleProperties();

						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB ENSEMBLE FAILED  ");
						if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit && DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
						{
							etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								/* Stop the timer if it's already running */
								if (st_TimerId.u32_AutoseekNotify_Timer > 0)
								{
									if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
									}
									else
									{
										st_TimerId.u32_AutoseekNotify_Timer = 0;
									}
								}
								else
								{
									/* MISRA C*/
								}
								/* Start the timer */
								st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
								if (st_TimerId.u32_AutoseekNotify_Timer == 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
								}
								else
								{
									/*MISRA C*/
								}
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
							}
							
						}
						else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
						{
							DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
							DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
						}
					}
				}
				break;

				case ETAL_RET_NO_DATA:
				{
					if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit && DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
						
					}
					else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
					}
					else
					{

					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;

		case GET_PROGRAMLIST_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB ENSEMBLE SERVICE LIST PROPERTIES FAILED - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				{
					// Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_PROGRAMLIST_CMD_MAX_REPEAT_COUNT)
					{
						/*Command to get service list*/
						etal_ret = DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAM_SERVLIST_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_PROGRAMLIST_TIMEOUT);
						}

						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] DAB ENSEMBLE FAILED  ");
						DAB_Tuner_Ctrl_me->b_scanstarted = TRUE;
						if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit && DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
						{
							
							etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								/* Stop the timer if it's already running */
								if (st_TimerId.u32_AutoseekNotify_Timer > 0)
								{
									if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
									}
									else
									{
										st_TimerId.u32_AutoseekNotify_Timer = 0;
									}
								}
								else
								{
									/* MISRA C*/
								}
								/* Start the timer */
								st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
								if (st_TimerId.u32_AutoseekNotify_Timer == 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
								}
								else
								{
									/*MISRA C*/
								}
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
							}
							
						}
						else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
						{
							DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
							DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
							DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
							DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
							DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
							Update_Stationlist_Into_LearnMem();
							DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
						}

					}
				}
				break;

				case ETAL_RET_NO_DATA:
				{
					if(DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit && DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if(etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if(st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if(SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if(st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
						
					}
					else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
					}
					else
					{

					}
				}
				break;

				default:
				{
					/* To avoid Warnings */
				}
				break;
			}
		}
		break;

		case HSM_MSGID_EXIT:  
		{
		}
		break;	

		default:
		{
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}


/*===================================================================================================================================================*/
/*Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_BackgrndScanStateGetcomponentListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg) */
/*===================================================================================================================================================*/
Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_BackgrndScanStateGetcomponentListHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; // mark the message as handled 
	Tbool b_resultcode = FALSE;
	Tu16  serviceIndex = 0;
	Tu16 labelIndex = 0;
	b_bgGetCompList_handler = TRUE;			//variable to check mutex condition
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{	 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_BackgrndScanStateGetcomponentListHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_BackgrndScanStateGetcomponentListHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_BackgrndScanStateGetcomponentListHndlr"));  
			etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
			}
			else
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
			}
		}
		break;
		
		case GET_COMPONENTLIST_TIMEOUT:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GET_COMPONENTLIST_TIMEOUT - %d", etal_ret);
			switch (etal_ret)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_NO_DATA:
				{
					// Retry the commands for 2 times
					if (DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT)
					{
						etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
					}
					else
					{
						DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetComponentListReq_Cmd Timeout even after retriggering");
						/* No of services of current ensemble is more than one and current service is not the last service*/
						if ((st_ScanServiceinfo.u8_NumOfServices != ((DAB_Tuner_Ctrl_me->currentserviceindx) + 1))
							&& (st_ScanServiceinfo.u8_NumOfServices > 1))
						{
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx], &ast_ServiceInfo[(DAB_Tuner_Ctrl_me->serviceindx) + 1], sizeof(Ts_Tuner_Ctrl_ServiceInfo)* (DAB_APP_MAX_SERVICES - ((DAB_Tuner_Ctrl_me->serviceindx) + 1)));
							/* Decrement total no of services count*/
							Services--;
							DAB_Tuner_Ctrl_me->currentserviceindx++;
							/* Re-trigger Get component list for remaining services */
							etal_ret = DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
							}
						}
						/* No of services of current ensemble is more than one and current service is the last service*/
						else if ((st_ScanServiceinfo.u8_NumOfServices == ((DAB_Tuner_Ctrl_me->currentserviceindx) + 1))
							&& (st_ScanServiceinfo.u8_NumOfServices > 1))
						{
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx], &ast_ServiceInfo[(DAB_Tuner_Ctrl_me->serviceindx) + 1], sizeof(Ts_Tuner_Ctrl_ServiceInfo));
							/* Decrement total no of services count*/
							Services--;
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							b_bgGetCompList_handler = FALSE;	//mutex released
							/* Check to find if all ensemble service component list is fetched or not*/
							if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								ensembleIndex++;
								DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
								DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
								DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
								DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
								Update_Stationlist_Into_LearnMem();
								DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
							}
							else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								ensembleIndex++;
								if (DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
								{
									etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
									if (etal_ret == ETAL_RET_SUCCESS)
									{
										/* Stop the timer if it's already running */
										if (st_TimerId.u32_AutoseekNotify_Timer > 0)
										{
											if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
											{
												RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
											}
											else
											{
												st_TimerId.u32_AutoseekNotify_Timer = 0;
											}
										}
										else
										{
											/* MISRA C*/
										}
										/* Start the timer */
										st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
										if (st_TimerId.u32_AutoseekNotify_Timer == 0)
										{
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
										}
										else
										{
											/*MISRA C*/
										}
									}
									else
									{
										DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
									}
									DAB_Tuner_Ctrl_me->b_ScanInProgress = TRUE;
									DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_STARTED;
									HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
								}
								else
								{
									/* Doing Nothing */
								}/* For MISRA C */
							}
						}
						else if (st_ScanServiceinfo.u8_NumOfServices == 1)
						{
							/* Clear particular ensemble info in ast_EnsembleInfo array */
							memset(&ast_EnsembleInfo[ensembleIndex], 0, sizeof(Ts_Tuner_Ctrl_EnsembleInfo));
							/* Move service list to the previous service to remove current service information from service list */
							memmove(&ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx], &ast_ServiceInfo[(DAB_Tuner_Ctrl_me->serviceindx) + 1], sizeof(Ts_Tuner_Ctrl_ServiceInfo));
							/* Decrement total no of services count*/
							Services--;
							SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
							b_bgGetCompList_handler = FALSE;	//mutex released
							/* Check to find if all ensemble service component list is fetched or not*/
							if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
								DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
								DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
								DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
								Update_Stationlist_Into_LearnMem();
								DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
							}
							else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
							{
								if (DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
								{
									DAB_Tuner_Ctrl_me->b_ScanInProgress = TRUE;
									etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
									if (etal_ret == ETAL_RET_SUCCESS)
									{
										/* Stop the timer if it's already running */
										if (st_TimerId.u32_AutoseekNotify_Timer > 0)
										{
											if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
											{
												RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
											}
											else
											{
												st_TimerId.u32_AutoseekNotify_Timer = 0;
											}
										}
										else
										{
											/* MISRA C*/
										}
										/* Start the timer */
										st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
										if (st_TimerId.u32_AutoseekNotify_Timer == 0)
										{
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
										}
										else
										{
											/*MISRA C*/
										}
									}
									else
									{
										DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
									}
									DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_STARTED;
									HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
								}
								else
								{
									/* Doing Nothing */
								}/* For MISRA C */
							}
						}
						else
						{

						}/* For MISRA C */
					}
				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;		

		case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Cmd for getting the service information */
			b_resultcode = DabTuner_GetServListpropertiesReply(&(st_ScanServiceinfo.st_serviceinfo[DAB_Tuner_Ctrl_me->serviceindx]));
			if (b_resultcode == TRUE)
			{
				Services = Services - st_ScanServiceinfo.u8_NumOfServices;
				/* Updating Service List Structure */
				for (serviceIndex = 0; serviceIndex < st_ScanServiceinfo.u8_NumOfServices; serviceIndex++, Services++)
				{
					if (Services < DAB_APP_MAX_SERVICES)
					{
						SYS_RADIO_MEMCPY(&(ast_ServiceInfo[Services].st_BasicEnsInfo), &(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo), sizeof(ast_ServiceInfo[Services].st_BasicEnsInfo));
						ast_ServiceInfo[Services].st_ServiceLabel.u16_ShortLabelFlags = st_ScanServiceinfo.st_serviceinfo[serviceIndex].ShortLabelFlags;
						if (st_ScanServiceinfo.st_serviceinfo[serviceIndex].CharSet == 0x00)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if (st_ScanServiceinfo.st_serviceinfo[serviceIndex].CharSet == 0x06)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if (st_ScanServiceinfo.st_serviceinfo[serviceIndex].CharSet == 0x0f)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;
						}
						else
						{
							/*Doing nothing */
						}/* For MISRA C */
						for (labelIndex = 0; labelIndex<16; labelIndex++)
						{
							ast_ServiceInfo[Services].st_ServiceLabel.au8_label[labelIndex] = st_ScanServiceinfo.st_serviceinfo[serviceIndex].LabelString[labelIndex];
						}

					}
				}
			}
			b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply));
			if(b_resultcode == TRUE)
			{
				for(DAB_Tuner_Ctrl_me->CompIndex=0; DAB_Tuner_Ctrl_me->CompIndex < DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.NoOfComponents;DAB_Tuner_Ctrl_me->CompIndex++,Components++)
				{
					if(Components < DAB_APP_MAX_COMPONENTS)
					{
						SYS_RADIO_MEMCPY(&(ast_ComponentInfo[Components].st_BasicEnsInfo),&(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo),sizeof(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo));
						ast_ComponentInfo[Components].u16_SCIdI        =	  DAB_Tuner_Ctrl_me-> DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].InternalCompId;
						ast_ComponentInfo[Components].u32_SId		   =	  ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId;
						memcpy((ast_ComponentInfo[Components].st_compLabel.au8_label),(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						ast_ComponentInfo[Components].st_compLabel.u16_ShortLabelFlags   =    DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].ShortLabelFlags;

						if(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x00)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x06)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if(DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x0f)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
						}
						else
						{
							/*Doing nothing */ 	
						}/* For MISRA C */
						ast_ComponentInfo[Components].u8_ComponentType = DAB_Tuner_Ctrl_me->DAB_Tuner_Ctrl_GetComponent_Reply.Component[DAB_Tuner_Ctrl_me->CompIndex].ComponentType;
					}
				}
				DAB_Tuner_Ctrl_me->currentserviceindx++;
				DAB_Tuner_Ctrl_me->serviceindx++;
				if ((DAB_Tuner_Ctrl_me->currentserviceindx < st_ScanServiceinfo.u8_NumOfServices) && (DAB_Tuner_Ctrl_me->serviceindx < DAB_APP_MAX_SERVICES))
				{
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(ast_ServiceInfo[DAB_Tuner_Ctrl_me->serviceindx].u32_SId);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY);
					}
					else
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, GET_COMPONENTLIST_TIMEOUT);
					}
				}
				else
				{
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					b_bgGetCompList_handler = FALSE;		//mutex released
					if ((DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency == DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit) || (Components >= DAB_APP_MAX_COMPONENTS))
					{
						ensembleIndex++;
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
						DAB_Tuner_Cntrl_Response_Scan(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
						Update_Stationlist_Into_LearnMem();
						DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_COMPLETED;
						DAB_Tuner_Ctrl_me->b_scanstarted = FALSE;
						DAB_Tuner_Ctrl_Check_Request_Type(DAB_Tuner_Ctrl_me);
					}
					else if (DAB_Tuner_Ctrl_me->st_SeekStatus.m_frequency != DAB_Tuner_Ctrl_me->sScanInput.u32UpperLimit)
					{
						ensembleIndex++;
						if (DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
						{
							etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
							if (etal_ret == ETAL_RET_SUCCESS)
							{
								/* Stop the timer if it's already running */
								if (st_TimerId.u32_AutoseekNotify_Timer > 0)
								{
									if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
									}
									else
									{
										st_TimerId.u32_AutoseekNotify_Timer = 0;
									}
								}
								else
								{
									/* MISRA C*/
								}
								/* Start the timer */
								st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
								if (st_TimerId.u32_AutoseekNotify_Timer == 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
								}
								else
								{
									/*MISRA C*/
								}
							}
							else
							{
								DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
							}
							DAB_Tuner_Ctrl_me->b_ScanInProgress = TRUE;
							DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_STARTED;
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
						}
					}
					else if(DAB_Tuner_Ctrl_me->b_ActivateRequest == FALSE)
					{
						etal_ret = DabTunerCtrl_StartScan2_cmd(DAB_Tuner_Ctrl_me->b_scanstarted, DAB_Tuner_Ctrl_me->e_RequestCmd);
						if (etal_ret == ETAL_RET_SUCCESS)
						{
							/* Stop the timer if it's already running */
							if (st_TimerId.u32_AutoseekNotify_Timer > 0)
							{
								if (SYS_StopTimer(st_TimerId.u32_AutoseekNotify_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION failed");
								}
								else
								{
									st_TimerId.u32_AutoseekNotify_Timer = 0;
								}
							}
							else
							{
								/* MISRA C*/
							}
							/* Start the timer */
							st_TimerId.u32_AutoseekNotify_Timer = SYS_StartTimer(DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT_TIME, DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
							if (st_TimerId.u32_AutoseekNotify_Timer == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION");
							}
							else
							{
								/*MISRA C*/
							}
						}
						else
						{
							DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, SCAN_NOTIFY_TIMEOUT);
						}
						DAB_Tuner_Ctrl_me->b_ScanInProgress = TRUE;
						DAB_Tuner_Ctrl_me->e_Scannstatus = TUNER_CTRL_SCAN_STARTED;
						HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
					}
					else
					{
						/* Doing Nothing */
					}/* For MISRA C */
				}
			}
		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			ret = msg;  
		} 
		break;
	}
	return ret;
}


/*=========================================================================================================================================*/
/* Ts_Sys_Msg* DAB_TUNER_CTRL_INST_HSM_FM_DAB_LinkingHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* pst_msg);     */
/*=========================================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_FM_DAB_LinkingHndlr (Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Tbool b_resultcode = FALSE;
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_FM_DAB_LinkingHndlr ");
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str,"DAB_TUNER_CTRL_INST_HSM_FM_DAB_LinkingHndlr",sizeof("DAB_TUNER_CTRL_INST_HSM_FM_DAB_LinkingHndlr"));
			DAB_Tuner_Ctrl_Check_Same_PI_InLearnMem(DAB_Tuner_Ctrl_me);
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,FM_DAB_LINKING_DATA);
			Same_SID_Search = 0;
			DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
		}
		break;

		case DAB_TUNER_CTRL_TUNE_SIGNAL_PRESENT:
		{
			if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_TUNED_STATIONS_SORTED)
			{
				b_resultcode = DabTuner_MsgRcvSetTuneStatusNot(&(DAB_Tuner_Ctrl_me->st_tuneStatus),msg->data);	
				if(b_resultcode == TRUE)
				{
					/* Stopping TUNE_TIME_OUT timer */
					if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer) == FALSE)
					{
					 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
					}
					else
					{
						st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = 0;
					}			
					/* Starting the timer for DAB_TUNER_CTRL_TUNE_MCI_PRESENT */
					st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = SYS_StartTimer(1800, DAB_TUNER_CTRL_TUNE_MCI_PRESENT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_TUNE_MCI_PRESENT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
			}
		}
		break;

		case RSSI_NOTIFICATION:
		{
			b_resultcode = DabTuner_MsgRcvRSSI_notify(&(DAB_Tuner_Ctrl_me->st_RSSI_notification),msg->data);
			if(b_resultcode == TRUE)
			{
	 			DabTuner_MsgSndSetRSSINotifier_cmd(0x00);
				if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus != DAB_TUNER_CTRL_TUNED_STATIONS_SORTED)
				{
					/* Stopping TUNE_TIME_OUT timer */
					if(SYS_StopTimer(st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer) == FALSE)
					{
					 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for TUNE_TIME_OUT failed");	
					}
					else
					{
						st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = 0;
					}			
					DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search-1].s8_RSSI = DAB_Tuner_Ctrl_me->st_RSSI_notification.TunerLevel;
					DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
				}	
			}
		}
		break;	
		
		case DAB_TUNER_CTRL_TUNE_MCI_PRESENT:
		{
			st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = 0;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] In FM-DAB linking, Select service command sent");
			(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search - 1].u16_SId);
			st_TimerId.u32_SelService_FMDABLinking_Timer = SYS_StartTimer(DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME, SELECT_SERVICE_TIMEOUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_SelService_FMDABLinking_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_SERVICE_TIMEOUT");	
			}
			else
			{
				/*MISRA C*/	
			}
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId = DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search-1].u16_SId;
			/* Clearing the b_ReConfigurationFlag (This flag is set 6 seconds before ReConfiguration) as DABTuner is giving
				PROGRAMME_SERV_LIST_CHANGED_NOTIFICATION after tuning to new frequency even ReConfiguration has not happend,
				now after getting this notification ReConfiguration is decided based on this flag which is set when ever 
				ChangeFlag in FIG 0/0 is other then 0 */
			DAB_Tuner_Ctrl_me->b_ReConfigurationFlag = FALSE ;
			 
		}
		break;
		
		case SELECT_SERVICE_TIMEOUT:
		{
			st_TimerId.u32_SelService_FMDABLinking_Timer = 0;
			/* Re-trigger SelectService_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_SERVICE_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTuner_MsgSndSelectService_Cmd(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search].u16_SId);
				/* Starting timer for SelectService_cmd time out */
				st_TimerId.u32_SelService_FMDABLinking_Timer = SYS_StartTimer(DAB_SELECT_SERVICE_CMD_TIMEOUT_TIME, SELECT_SERVICE_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_SelService_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_SERVICE_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectService_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				Same_SID_Search++;
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
			}
		}
		break;		


		case DAB_TUNER_CTRL_SELECT_SERVICE_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for SelectService_cmd time out */
			if(SYS_StopTimer(st_TimerId.u32_SelService_FMDABLinking_Timer) == FALSE)
			{
			 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for SELECT_SERVICE_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_SelService_FMDABLinking_Timer = 0;
			}			
			b_resultcode=DabTuner_MsgRcvSelectServiceReply(&(DAB_Tuner_Ctrl_me->st_selectServiceReply),msg->data);
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus==0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] In FM-DAB linking, Select component command sent");
					(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search - 1].u16_SCIdI, DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search - 1].u16_SId);
					/* Starting timer for SelectComponent_repl time out */
					st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_SELECT_COMPONENT_CMD_TIMEOUT_TIME, SELECT_COMPONENT_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_FMDABLinking_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_COMPONENT_TIMEOUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI =  DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId ;
				}
				else
				{
					
					Same_SID_Search++;
					DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
				}
			}
		}
		break;		

		case SELECT_COMPONENT_TIMEOUT:
		{
			st_TimerId.u32_FMDABLinking_Timer = 0;
			/* Re-trigger SelectComponent_repl */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_SELECT_COMPONENT_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTuner_MsgCmd_SelectComponent(DAB_Tuner_Ctrl_me->st_selectServiceReply.SelectserviceComponets[0].InternalCompId, DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search].u16_SId);
				/* Starting timer for SelectComponent_repl time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(1000, SELECT_COMPONENT_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for SELECT_COMPONENT_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] SelectComponent_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				Same_SID_Search++;
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
			}
		}
		break; 
		
		case DAB_TUNER_CTRL_SELECT_COMPONENT_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for SelectComponent_repl time out */
			if(SYS_StopTimer(st_TimerId.u32_FMDABLinking_Timer) == FALSE)
			{
			 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for SELECT_COMPONENT_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_FMDABLinking_Timer = 0;
			}			
			DabTuner_MsgReply_SelectComponent(&(DAB_Tuner_Ctrl_me->st_selectcompreply),msg->data);
			DabTuner_MsgSndSetPeriodicalBERQualityNotifier(0x03);
			if(DAB_Tuner_Ctrl_me->st_selectcompreply.reply == 0)
			{
				DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ; 
				if(SYS_StartTimer(250, AUDIO_TIMER_NOTIFICATION, RADIO_DAB_TUNER_CTRL)<= 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for AUDIO_TIMER_NOTIFICATION");
				}
				else
				{
					/*MISRA*/	
				}	
				
				//DabTuner_MsgSndSetAudioStatusNotifier_cmd(0x01);
			}
			else
			{
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
			}	
		}
		break;
		
		case AUDIO_TIMER_NOTIFICATION:
		{
			if(DAB_Tuner_Ctrl_me->st_Audiostatus.Audiosyncronised == 1 && (DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus <=1 || DAB_Tuner_Ctrl_me->st_BER_notification.FIC_BER_Exponent < BER_LINKING_THRESHOULD)) 
			{
				DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
				/* As per requirement Starting 10seconds time to validate DAB signal stability */
				//(void)SYS_StartTimer(4, DAB_SIGNAL_STABLE_CHECK_TIME, DAB_SIGNAL_STABLE_CHECK);
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] In FM-DAB linking, Audio status success");
				(void)DabTuner_MsgSndGetEnsembleProperties();	
				/* Starting timer for GetEnsembleProperties_req time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_ENSEMBLE_PROPERTIES_CMD_TIMEOUT_TIME, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
				if( st_TimerId.u32_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT");		
				}
				else
				{
					/*MISRA C*/
				}	
				
			}
			else
			{
				if(DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger < (Tu8)GET_AUDIO_STATUS_REQ_MAX_TIME_RETRIGGER)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] In FM-DAB linking, Audio status re-trigger");
					if(SYS_StartTimer(200, AUDIO_TIMER_NOTIFICATION, RADIO_DAB_TUNER_CTRL)<= 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for AUDIO_TIMER_NOTIFICATION");
					}
					else
					{
						/*MISRA*/	
					}	
					DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger++ ;
				}
				else
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] In FM-DAB linking, tuned station is unstable even after retriggering GetAudioStatus_Cmd ");
					DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
					DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_TUNED_STATION_NOTSTABLE;
					DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
				}
				
			}
		}
		break;	

		case BER_STATUS_NOTIFICATION:
		{
			b_resultcode=DabTuner_MsgRcvSetPeriodicalBERQualityNotifier(&(DAB_Tuner_Ctrl_me->st_BER_notification),msg->data);
			if(b_resultcode==TRUE)
			{
				
			}
		}
		break;	
/************************************************************************************************************************************************/
#if 0
		case DAB_TUNER_CTRL_GET_AUDIO_PROPERTIES_REPLY :
		{
			b_resultcode = DabTunerCtrl_GetAudioProperties_repl(&(DAB_Tuner_Ctrl_me->st_GetAudioProperties_repl),msg->data);
			/*	Triggering audio status request */
			if( SYS_StartTimer(200, TRIGGER_GET_AUDIO_STATUS_REQ, RADIO_DAB_TUNER_CTRL) <= 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TRIGGER_GET_AUDIO_STATUS_REQ");		
			}
			else
			{
			/*MISRA C*/
			}
		}
		break;

		case TRIGGER_GET_AUDIO_STATUS_REQ :
		{
			(void)DabTunerCtrl_GetAudioStatus_req(); 	
		}
		break;
		
		case DAB_TUNER_CTRL_GET_AUDIO_STATUS_REPLY:
		{
			b_resultcode = DabTunerCtrl_GetAudioStatus_repl(&(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl));
			if(b_resultcode == TRUE)
			{
				if((DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.DecodingStatus == 0)&& (DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.AudioSynchronised == 1))
				{
					DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
					/* As per requirement Starting 10seconds time to validate DAB signal stability */
					if( SYS_StartTimer(DAB_SIGNAL_STABLE_CHECK_TIME, DAB_SIGNAL_STABLE_CHECK, RADIO_DAB_TUNER_CTRL) <= 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_SIGNAL_STABLE_CHECK");		
					}
					else
					{
						/*MISRA C*/
					}
				}
				else if((DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.ReplyStatus == (Tu8)(0x20))||(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.DecodingStatus != (Tu8)(0))||(DAB_Tuner_Ctrl_me->st_GetAudioStatus_repl.AudioSynchronised != (Tu8)(1)))
				{
					if(DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger < (Tu8)GET_AUDIO_STATUS_REQ_MAX_TIME_RETRIGGER)
					{
						if( SYS_StartTimer(200, TRIGGER_GET_AUDIO_STATUS_REQ, RADIO_DAB_TUNER_CTRL) <= 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TRIGGER_GET_AUDIO_STATUS_REQ");		
						}
						else
						{
							/*MISRA C*/
						}
						DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger++ ;
					}
					else
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] In FM-DAB linking, tuned station is unstable even after retriggering GetAudioStatus_Cmd ");
						DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
						Same_SID_Search++;
						DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
					}
				}
				else
				{
					
				}/* For MISRA*/
			}
		}
		break;
		
		case DAB_SIGNAL_STABLE_CHECK:
		{
			if((DAB_Tuner_Ctrl_me->st_Audiostatus.Audiosyncronised == 1)&&(DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0)) 
			{
				(void)DabTuner_MsgSndGetEnsembleProperties();	
				/* Starting timer for GetEnsembleProperties_req time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_ENSEMBLE_PROPERTIES_CMD_TIMEOUT_TIME, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);	
				if( st_TimerId.u32_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT");		
				}
				else
				{
					/*MISRA C*/
				}			
			}
			else if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == (Tu8)(0x02)) || (DAB_Tuner_Ctrl_me->st_Audiostatus.Audiosyncronised != (Tu8)(0x01)))
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Same PI tuned station is unstable");
				if(DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger < (Tu8)GET_AUDIO_STATUS_REQ_MAX_TIME_RETRIGGER)
				{
					if( SYS_StartTimer(200, TRIGGER_GET_AUDIO_STATUS_REQ, RADIO_DAB_TUNER_CTRL) <= 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TRIGGER_GET_AUDIO_STATUS_REQ");		
					}
					else
					{
						/*MISRA C*/
					}
					DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger++ ;
				}
				else
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] In FM-DAB linking, tuned station is unstable(after 10sec stability check) even after retriggering GetAudioStatus_Cmd ");
					DAB_Tuner_Ctrl_me->u8_Cmd_ReTrigger = (Tu8)(0) ;
					Same_SID_Search++;
					DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
				}
			}
			else
			{
				Same_SID_Search++;
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
			}
		}
		break;
#endif
/**************************************************************************************************************************************************/	
		
		case DAB_TUNER_CTRL_AUDIO_STATUS_NOTIFICATION:
		{
			(void)DabTuner_Get_Audio_Status_Notification(&(DAB_Tuner_Ctrl_me->st_Audiostatus), (Ts8*)(msg->data));
			/* Check audio status notification */ 
			if(DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent == TRUE)
			{
				if((DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x01 || DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus == 0x02) && (DAB_Tuner_Ctrl_me->st_BER_notification.Subchannel_BER_Exponent >= BER_LINKING_THRESHOULD))
				{	
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Monitoring tuned same PI station is unstable notification sent to DAB_App");
					if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_START)
					{
						DAB_Tuner_Ctrl_Notify_SignalStatus(DAB_TUNER_CTRL_SIGNAL_LOW);	
						DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] FM_DAB Same PI tuned station signal lost ");
						DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = FALSE;
						DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_STATION_UNIDENTIFIED ;
						DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality,DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
						if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus != DAB_TUNER_CTRL_FMDAB_BLENDING_SUCCESS)
						{
							DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_RECEIVED ;
							if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
							{
								if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
							 	{
							  		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
							 	}
							 	else
							 	{
									st_TimerId.u32_StartBackGrndScan_Timer = 0;		
								}			 
							}
							else
							{
								/* MISRA C*/	
							}
							st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_HALF_SEC_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
							if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");
							}
							else
							{
								/*MISRA C*/
							}
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
						}
					}					
				}
				else
				{
					/* Doing nothing */
				}/* For MISRA C */
			}
			/* Skip audio status notification */
			else
			{
				/* Skipping audio status notification */	
			}
		}
		break;	

		case GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT:
		{
			st_TimerId.u32_FMDABLinking_Timer = 0;
			/* Re-trigger GetEnsembleProperties_req */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_ENSEMBLE_PROPERTIES_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTuner_MsgSndGetEnsembleProperties();
				/* Starting timer for GetEnsembleProperties_req time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_ENSEMBLE_PROPERTIES_CMD_TIMEOUT_TIME, GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if( st_TimerId.u32_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT");		
				}
				else
				{
					/*MISRA C*/
				}					
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetEnsembleProperties_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				Same_SID_Search++;
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
			}
		}
		break;

		case GET_ENSEMBLE_PROPERTIES_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			/* Stopping timer for GetEnsembleProperties_req time out */
			if(SYS_StopTimer(st_TimerId.u32_FMDABLinking_Timer) == FALSE)
			{
			 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_ENSEMBLE_PROPERTIES_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_FMDABLinking_Timer = 0;
			}				 
			DabTuner_MsgReplyGetEnsembleProperties(&(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply));
			SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_tuneStatus.Frequency;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId 		= DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.EnsembleIdentifier;
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC = DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.ECC ;
			if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x06)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2 ;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.CharSet == 0x0f)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8 ;
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */
			(void)DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
			memset(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply),0 , sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			/* Starting timer for GetProgrammeServiceList_req time out */
			st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_PROGRAMLIST_CMD_TIMEOUT_TIME, GET_PROGRAMLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
			if( st_TimerId.u32_FMDABLinking_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_PROGRAMLIST_TIMEOUT");		
			}
			else
			{
				/*MISRA C*/
			}
		}
		break;
		
		case GET_PROGRAMLIST_TIMEOUT:
		{
			st_TimerId.u32_FMDABLinking_Timer = 0;
			/* Re-trigger GetProgrammeServiceList_req */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_PROGRAMLIST_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd();
				/* Starting timer for GetProgrammeServiceList_req time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_PROGRAMLIST_CMD_TIMEOUT_TIME, GET_PROGRAMLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if( st_TimerId.u32_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_PROGRAMLIST_TIMEOUT");		
				}
				else
				{
					/*MISRA C*/
				}			
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetProgrammeServiceList_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				Same_SID_Search++;
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
			}	 
		}	 
		break;		

		case GET_PROGRAM_SERVLIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_FMDABLinking_Timer) == FALSE)
			{
			 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_PROGRAMLIST_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_FMDABLinking_Timer = 0;
			}				 
			DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));
		//	memset(scan_buffer_temp, 0x00, SCAN_SPI_READ_BUFFER_LENGTH);
			DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx = 0;
			(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
			/* Starting timer for GetComponentListReq_Cmd time out */
			st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
			if( st_TimerId.u32_FMDABLinking_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");		
			}
			else
			{
				/*MISRA C*/
			}
		}
		break;

		case GET_COMPONENTLIST_TIMEOUT:
		{
			st_TimerId.u32_FMDABLinking_Timer = 0;
			/* Re-trigger GetComponentListReq_Cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_GET_COMPONENTLIST_CMD_MAX_REPEAT_COUNT)
			{
				(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
				/* Starting timer for GetComponentListReq_Cmd time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if( st_TimerId.u32_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");		
				}
				else
				{
					/*MISRA C*/
				}				
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] GetComponentListReq_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* No of services of current ensemble is more than one and current service is not the last service*/
				if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) != ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
					&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
				{
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo)* (MAX_ENSEMBLE_SERVICES - ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1)));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;

					/* Re-trigger Get component list for remaining services */
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
					/* Starting timer for GetComponentListReq_Cmd time out */
					st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if( st_TimerId.u32_FMDABLinking_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");		
					}
					else
					{
						/*MISRA C*/
					}					
				}
				/* No of services of current ensemble is more than one and current service is the last service*/
				else if(((DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) == ((DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1))
					&& (DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices > 1))
				{
					/* Move service list to the previous service to remove current service information from service list */
					memmove(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx],&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx)+1],sizeof(Ts_CurrEnsemble_serviceinfo));
					/* Decrement total no of services count*/
					(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)--;
					/* Decrement total no of service index count*/
					DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx--;
					DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = TRUE;
					/* Copy st_GetCurrEnsembleProgListReply to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
					SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					/* Send sort request to DAB App */
					DAB_App_Request_sort();

				}
				else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices == 1)
				{
					DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
				}
				else /* Timeout of GetComponentListReq_Cmd */
				{
					/* Do Nothing */
				}
			}
		}
		break;
		
		case DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_FMDABLinking_Timer) == FALSE)
			{
			 	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for GET_COMPONENTLIST_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_FMDABLinking_Timer = 0;
			}			
			b_resultcode = DabTuner_MsgRcvScan2GetComponentListReply(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].st_compInfo));
			if(b_resultcode == TRUE)
			{
				if(DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx !=DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices)
				{
					DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx++;
					(void)DabTuner_MsgSndScan2GetComponentListReq_Cmd(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[DAB_Tuner_Ctrl_me->u8_CurrEnsembleSerIndx].ProgServiceId);
					/* Starting timer for GetComponentListReq_Cmd time out */
					st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_GET_COMPONENTLIST_CMD_TIMEOUT_TIME, GET_COMPONENTLIST_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if( st_TimerId.u32_FMDABLinking_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_COMPONENTLIST_TIMEOUT");		
					}
					else
					{
						/*MISRA C*/
					}
				}
				else
				{
					/* Clear service label in "st_currentEnsembleData" structure */
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label), 0, sizeof(Ts_Tuner_Ctrl_Label));
					/* Clear Component label in "st_currentEnsembleData" structure */
					memset(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label), 0, sizeof(Ts_Tuner_Ctrl_Label));

					DAB_Tuner_Ctrl_Update_Label(&(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply));

					/* Copy st_GetCurrEnsembleProgListReply to st_Dab_Tuner_Ctrl_CurrEnsembleProgList to share it with DAB App for sorting */
					SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
					SYS_RADIO_MEMCPY(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList,&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
					SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
					/* Send sort request to DAB App */
					DAB_App_Request_sort();
					//DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
				}
			}	 		 
		}
		break;
		case PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT:
		{
			st_TimerId.u32_FMDABLinking_Timer = 0;

			/* Re-trigger DabTuner_PrepareForBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_PREPARE_FOR_BLENDING_CMD_MAX_REPEAT_COUNT)
			{
				DabTuner_PrepareForBlending_cmd();
				/* Starting timer for DabTuner_PrepareForBlending_cmd time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if( st_TimerId.u32_DAB_FM_blending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT");		
				}
				else
				{
					/*MISRA C*/
				}			
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] DabTuner_PrepareForBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				
				DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_TUNED_STATION_NOTSTABLE;
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
			}
		}
		break;
		

		case PREPARE_FOR_BLENDING_REPLY:
		{
			DabTuner_PrepareForBlending_Reply(&(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Reply),msg->data);
			if(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Reply.Reply == 0x05)
			{	
				/* Re-trigger DabTuner_PrepareForBlending_cmd */
				if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_PREPARE_FOR_BLENDING_CMD_MAX_REPEAT_COUNT)
				{
					DabTuner_PrepareForBlending_cmd();
					/* Starting timer for DabTuner_PrepareForBlending_cmd time out */
					st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_FMDABLinking_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_PROGRAMLIST_TIMEOUT");		
					}
					else
					{
						/*MISRA C*/
					}			
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				}
				else
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] DabTuner_PrepareForBlending_Cmd reply:'OutofResources' Timeout even after retriggering");
					DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				
					//DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_TUNED_STATION_NOTSTABLE;
					//DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);
					if(DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent == FALSE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] Notify same PI found station information to RM  ");
						/* Notify same PI found station information */
						DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_SAME_PI_STATION ;
						DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = TRUE;
						DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
						DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality,DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
					}
					else
					{
						
					}
					
					/* Testing purpose */
					/* Notify same PI found station information */
					//DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_SAME_PI_STATION ;
					//DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
					//DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality,DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
					
				}
			}
			else
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				if(DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] DabTuner_PrepareForBlending_Reply is 0x00:Notify same PI found station information to RM  ");
					/* Notify same PI found station information */
					DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_SAME_PI_STATION ;
					DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = TRUE;
					DAB_Tuner_Ctrl_Response_SelectService(REPLYSTATUS_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
					DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality,DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
				}
				else
				{
						
				}
			}
		}
		break;


		case DAB_TUNER_CTRL_INIT_FMDAB_LINKING_NOTIFYID:
		{
			/* Start Blending */
			//DabTuner_StartBlending_cmd((Tu8)0x50,DAB_Tuner_Ctrl_me->u16_LevelData);
			/* Starting timer for DabTuner_StartBlending_cmd time out */
			//(void)SYS_StartTimer(4, DAB_START_BLENDING_CMD_TIMEOUT_TIME, START_BLENDING_REPLY_TIMEOUT);
			//DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_BLENDING_SUCCESS;
			DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality,DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] FM_DAB Linking successful ");
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,FM_DAB_LINKING_DATA);
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,FM_DAB_LINKING_STATUS);
		}
		break;
		
		case START_BLENDING_REPLY_TIMEOUT:
		{
			st_TimerId.u32_StartBlending_Timer  = 0;
			/* Re-trigger DabTuner_StartBlending_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_START_BLENDING_CMD_MAX_REPEAT_COUNT)
			{
				if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_FMDAB_BLENDING_SUCCESS)
				{
					DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
				}
				else
				{
					DabTuner_StartBlending_cmd((Tu8)0x50,DAB_Tuner_Ctrl_me->u16_LevelData);
				}
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] DabTuner_StartBlending_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* NEED TO UPDATE TIMEOUT RESPONSE */
				if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_FMDAB_BLENDING_SUCCESS)
				{
					DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
				}
				else
				{
					DabTuner_StartBlending_cmd((Tu8)0x50,DAB_Tuner_Ctrl_me->u16_LevelData);
				}
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
		}
		break;		

		case START_BLENDING_REPLY:
		{
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
			}
			else
			{
				st_TimerId.u32_StartBlending_Timer = 0;
			}

			DabTuner_StartBlending_Reply(&(DAB_Tuner_Ctrl_me->st_Start_Blending_Reply),msg->data);
			
			if(DAB_Tuner_Ctrl_me->st_Start_Blending_Reply.Reply == (Tu8) 0)
			{
				if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_FMDAB_BLENDING_STOP)
				{
					DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_BLENDING_CANCELLED;
					/*I2CWRITE[0] =0x20;
					I2CWRITE[1] =0x10;
					I2c_Ret=i2c1_write(0x00, I2CWRITE, sizeof(I2CWRITE));
					dly_tsk(2);*/
					
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Blending stopped and starting background scan");
					
					if(DAB_Tuner_Ctrl_me->e_FmtoDAB_Reqstatus == DAB_TUNER_CTRL_FMDAB_LINKING_REQ_SIG_LOST)
					{
						DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_RECEIVED;
					}
					else
					{
						
					}

					if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
					 	{
					  		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
					 	}
					 	else
					 	{
							st_TimerId.u32_StartBackGrndScan_Timer = 0;		
						}			 
					}
					else
					{
						/* MISRA C*/	
					}
					st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(DAB_BG_SCAN_START_HALF_SEC_DELAY_TIME, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");
					}
					else
					{
						/*MISRA C*/
					}
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
				}
				else
				{
					DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_BLENDING_SUCCESS;
					/*I2CWRITE[0] =0x20;
					I2CWRITE[1] =0x13;
					I2c_Ret=i2c1_write(0x00, I2CWRITE, sizeof(I2CWRITE));
					dly_tsk(2);*/
					DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality,DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
				}
							
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,FM_DAB_LINKING_DATA);
				DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,FM_DAB_LINKING_STATUS);
		
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR] StartBlending_Cmd failed hence retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				/* NEED TO UPDATE TIMEOUT RESPONSE */
				if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_FMDAB_BLENDING_SUCCESS)
				{
					DabTuner_StartBlending_cmd((Tu8)0xD0,DAB_Tuner_Ctrl_me->u16_LevelData);
				}
				else
				{
					DabTuner_StartBlending_cmd((Tu8)0x50,DAB_Tuner_Ctrl_me->u16_LevelData);
				}
				/* Starting timer for StartBlending_cmd time out */
				/* Starting timer for DabTuner_StartBlending_cmd time out */
				st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME, START_BLENDING_REPLY_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_StartBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
		}
		break;
		
		case DLS_DATA_NOTIFICATION:
		{
			 if(DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent == TRUE)
			 {	
				DabTunerMsg_XPAD_DLS_Data(&(DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data),msg->data);
					if (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.Runningstatus == 1)
					{
						if (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.CharacterSet == 0)
						{
							DAB_Tuner_Ctrl_me->st_Dls_data.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.CharacterSet == 6)
						{
							DAB_Tuner_Ctrl_me->st_Dls_data.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.CharacterSet == 15)
						{
							DAB_Tuner_Ctrl_me->st_Dls_data.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;
						}
						else
						{

						}
						SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_Dls_data.au8_DLSData), (DAB_Tuner_Ctrl_me->st_Dynamic_Label_Data.DLS_LabelByte), 128);
						DAB_Tuner_Ctrl_Notify_DLSdata(DAB_Tuner_Ctrl_me->st_Dls_data);
					}
			 }	
		}
		break;

		case SLS_DATA_NOTIFICATION:
		{
			SYS_MUTEX_LOCK(SLS_DAB_APP_DAB_TC);
			SYS_RADIO_MEMCPY(&st_DabTc_SLS_Data, &st_SLS_Data, sizeof(st_SLS_Data));
			SYS_MUTEX_UNLOCK(SLS_DAB_APP_DAB_TC);
			DAB_Tuner_Ctrl_Notify_SLSdata();
		}
		break;

		case TUNE_TIME_OUT:
		{
			st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = 0;
			/* Re-trigger TuneTo_cmd */
			if(DAB_Tuner_Ctrl_me->u8_cmd_recall_count < DAB_TUNE_TIMEOUT_MAX_WAIT_COUNT)
			{
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
				st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
				if( st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");		
				}
				else
				{
					/*MISRA C*/
				}
			}
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] TuneTo_Cmd Timeout even after retriggering");
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
				DAB_TUNER_CTRL_Tune_To_Same_PI(DAB_Tuner_Ctrl_me);			
			}
		}
		break;

		case START_BACKGROUND_SCAN :
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Starting background scan after two minutes");
			if(st_TimerId.u32_StartBackGrndScan_Timer > 0)
			{
				if(SYS_StopTimer(st_TimerId.u32_StartBackGrndScan_Timer) == FALSE)
			 	{
			  		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for DAB_TUNER_CTRL_START_BACKGRND_SCAN failed");	
			 	}
			 	else
			 	{
					st_TimerId.u32_StartBackGrndScan_Timer = 0;		
				}			 
			}
			else
			{
				/* MISRA C*/	
			}
			st_TimerId.u32_StartBackGrndScan_Timer = SYS_StartTimer(300, DAB_TUNER_CTRL_START_BACKGRND_SCAN, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_StartBackGrndScan_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for DAB_TUNER_CTRL_START_BACKGRND_SCAN");
			}
			else
			{
				/*MISRA C*/
			}
			HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_backgrndEnsembleScan_state);
		}
		break;

		case DAB_TUNER_CTRL_COMP_LIST_SORT_RESID:
		{
			/* Copy st_Dab_Tuner_Ctrl_CurrEnsembleProgList back to st_GetCurrEnsembleProgListReply after sorting */
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			SYS_RADIO_MEMCPY(&DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply, &st_Dab_Tuner_Ctrl_CurrEnsembleProgList,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
			
			DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(DAB_Tuner_Ctrl_me);
			/* Check audio status */				
			 
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_FM_DAB_PI_RECEIVED)
			{
				//DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_SAME_PI_STATION ;
				//DAB_Tuner_Ctrl_Response_SelectService(TUNER_CTRL_SELECTSERVICE_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
				//DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality,DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] FM_DAB Same PI station delay check");
				 DabTuner_PrepareForBlending_cmd();
				/* Starting timer for DabTuner_PrepareForBlending_cmd time out */
				st_TimerId.u32_FMDABLinking_Timer = SYS_StartTimer(DAB_PREPARE_FOR_BLENDING_CMD_TIMEOUT_TIME, PREPARE_FOR_BLENDING_NOTIFICATION_TIMEOUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_FMDABLinking_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for GET_PROGRAMLIST_TIMEOUT");		
				}
				else
				{
					/*MISRA C*/
				}			
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count++;
			}
			else if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_START)
			{
				DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_SUCCESS,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
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
			/* in top state throw system error */
			ret = msg;
		}
		break;
	}
	return ret;
}


/*=========================================================================================================================*/
/*Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_StopHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)    */
/*=========================================================================================================================*/
Ts_Sys_Msg*  DAB_TUNER_CTRL_INST_HSM_StopHndlr(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Sys_Msg* msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu8 u8_NVM_ret = 0;
	//  	Te_Tuner_Ctrl_ShutdownReplyStatus st_status = TUNER_CTRL_SHUTDOWN_FAILURE;
	Te_Frontend_type e_Frontend_type;
	DAB_Tuner_Ctrl_me->e_ReplyStatus =  REPLYSTATUS_FAILURE;
	PRINT_MSG_DATA(msg);

	switch(msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DAB_TUNER_CTRL_INST_HSM_StopHndlr ");
			//DAB_Tuner_Ctrl_me->u8_curr_state_str ="DAB_TUNER_CTRL_INST_HSM_StopHndlr";
			SYS_RADIO_MEMCPY((void*)DAB_Tuner_Ctrl_me->u8_curr_state_str, "DAB_TUNER_CTRL_INST_HSM_StopHndlr", sizeof("DAB_TUNER_CTRL_INST_HSM_StopHndlr"));
			/* Update the current state of DAB_Tuner_Ctrl as INVALID as Abnormal notification need not be handled stop state */
			DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_State = DAB_TUNER_CTRL_INVALID_STATE;
			etal_ret = DabTuner_Destroy_FGQualityMonitor();
			if (etal_ret == ETAL_RET_SUCCESS)
			{   
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_DESTROY_QUALITYMONITOR_DONE_RESID);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]DESTROY_QUALITYMONITOR in ForeGround failed due to: %d\n", etal_ret);
			}
			
		}
		break;

		case DAB_TUNER_CTRL_DESTROY_QUALITYMONITOR_DONE_RESID:
		{
			etal_ret = DabTunerCtrl_Disable_Data_Service_cmd();
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_DISABLE_DATA_SERVICE_DONE_RESID);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Disable_Data_Service failed due to: %d\n", etal_ret);
			}
		}
		break;

		case DAB_TUNER_CTRL_DISABLE_DATA_SERVICE_DONE_RESID:
		{
			e_Frontend_type = FOREGROUND_CHANNEL;
			etal_ret = DabTunerCtrl_Destroy_Datapath_cmd(e_Frontend_type);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				/* Temporarily disabled FIC due to too many messages. So, do not destroy */
				//etal_ret = DabTunerCtrl_Destroy_FICData_cmd(e_Frontend_type);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					e_Frontend_type = BACKGROUND_CHANNEL;
					etal_ret = DabTunerCtrl_Destroy_Datapath_cmd(e_Frontend_type);
					if (etal_ret == ETAL_RET_SUCCESS)
					{
						DAB_Tuner_Ctrl_hsm_inst_ETAL_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_DESTROY_DATAPATH_DONE_RESID);
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Destroy_datapath failed due to: %d\n", etal_ret);
					}
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Destroy_FICData failed due to: %d\n", etal_ret);
				}
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Destroy_datapath failed due to: %d\n", etal_ret);
			}
		}
		break;

		case DAB_TUNER_CTRL_DESTROY_DATAPATH_DONE_RESID:
		{
			e_Frontend_type = FOREGROUND_CHANNEL;
			etal_ret = DabTunerCtrl_Destroy_Receiver_cmd(e_Frontend_type);
			if (etal_ret == ETAL_RET_SUCCESS)
			{
				e_Frontend_type = BACKGROUND_CHANNEL;
				etal_ret = DabTunerCtrl_Destroy_Receiver_cmd(e_Frontend_type);
				if (etal_ret == ETAL_RET_SUCCESS)
				{
					if(DAB_Tuner_Ctrl_me->e_DABTUNERRestartCmd == DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_VALID)
					{
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Ctrl_hsm_inst_stop_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_INST_SHUTDOWN_DONE);
					}
					else if (DAB_Tuner_Ctrl_me->e_DABTUNERRestartCmd == DAB_TUNER_CTRL_FCATORY_RESET_REQ_VALID)
					{
						//DAB_Tuner_Ctrl_me->e_DABTUNERRestartCmd = DAB_TUNER_CTRL_FCATORY_RESET_REQ_INVALID;
						memset(&(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo), 0, sizeof(Ts_CurrentSidLinkingInfo));
						memset(ast_LearnMem, 0, (sizeof(Ts_Tuner_Ctrl_Tunableinfo)*DAB_APP_MAX_COMPONENTS));
						u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LINKING, &(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo), sizeof(Ts_CurrentSidLinkingInfo), &DAB_Tuner_Ctrl_me->nvm_write);
						u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LEARN_MEMORY, ast_LearnMem, (sizeof(Ts_Tuner_Ctrl_Tunableinfo)*DAB_APP_MAX_COMPONENTS), &DAB_Tuner_Ctrl_me->nvm_write);
						if (u8_NVM_ret == 0)
						{
							DAB_Tuner_Ctrl_Response_Factory_Reset_Settings(REPLYSTATUS_SUCCESS);
							DAB_Tuner_Ctrl_hsm_inst_factory_reset_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_RESET_FACTORY_SETTINGS_RESID);
							HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
						}
						else
						{
							DAB_Tuner_Ctrl_Response_Factory_Reset_Settings(REPLYSTATUS_FAILURE);

						}

					}
					else
					{
						DAB_Tuner_Ctrl_me->e_ReplyStatus = REPLYSTATUS_SUCCESS;
						DAB_Tuner_Ctrl_Response_Shutdown(DAB_Tuner_Ctrl_me->e_ReplyStatus);
						DAB_Tuner_Ctrl_hsm_inst_stop_response(RADIO_DAB_TUNER_CTRL, DAB_TUNER_CTRL_INST_SHUTDOWN_DONE);
					}	
					HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_inactive_state);
				}
				else 
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Destroy_Receiver failed due to:%d \n", etal_ret);
				}
			}
			else 
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC][ERR]Destroy_Receiver failed due to:%d \n", etal_ret);
			}
		}
		break;

		case HSM_MSGID_EXIT:  
		{
			
		}
		break;
		
		default:
		{
			/* in top state throw system error */
			ret = msg;   
		}
		break;
	}
	return ret;
}
/*================================================================================================*/
/*  void  DAB_Tuner_Ctrl_INST_HSM_Init(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)             */
/*================================================================================================*/
void  DAB_Tuner_Ctrl_INST_HSM_Init(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	if(DAB_Tuner_Ctrl_me!= NULL)
	{
		/* clear all hsm */
		// DAB_TUNER_CTRL_APP_Ctor(pMain);
		memset(DAB_Tuner_Ctrl_me,0,sizeof(Ts_dab_tuner_ctrl_inst_hsm));
		DAB_Tuner_Ctrl_me->u8_curr_state_str = DAB_Tuner_Ctrl_me->str_state;
		/* Call the base class Ctor */
		HSM_CTOR((Ts_hsm*)DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_hsm_inst_top_state, RADIO_DAB_TUNER_CTRL);
		/* start HSM */
		HSM_OnStart((Ts_hsm*)DAB_Tuner_Ctrl_me);
	}
	else
	{
		// SYS_MSG_GET_LID(DAB_TUNER_CTRL_MAIN_CID, DAB_TUNER_CTRL_ERROR);
	}
}
Tu16 msgid = 0;

/*================================================================================================*/
/*  void DAB_TUNER_CTRL_INST_HSM_HandleMessage( Ts_dab_tuner_ctrl_inst_hsm* me, Ts_Sys_Msg* msg ) */
/*================================================================================================*/
void DAB_TUNER_CTRL_INST_HSM_HandleMessage( Ts_dab_tuner_ctrl_inst_hsm* me, Ts_Sys_Msg* msg )
{
	switch(msg->msg_id)
	{
		case DAB_TUNER_CTRL_REQID: 
		case DAB_TUNER_CTRL_RESID:
		{
			HSM_ON_MSG(me,msg);
		}
		break;
		default:
		{
			
		}
		break;		
	}
}



void DAB_Tuner_Ctrl_Make_Transition(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	HSM_STATE_TRANSITION(DAB_Tuner_Ctrl_me, &dab_tuner_ctrl_inst_hsm_DAB_FM_blending_state);
}
/*==================================================================================================*/
/*    end of file                                                                                   */
/*==================================================================================================*/


