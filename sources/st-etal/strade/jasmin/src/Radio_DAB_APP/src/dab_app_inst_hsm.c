/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app_inst_hsm.c																	  			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains state handler functions of DAB APP instance HSM.				*
*																											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app.h"
#include "dab_app_hsm.h"
#include "dab_app_stationlist.h"
#include "dab_app_request.h"
#include "dab_app_response.h"
#include "dab_app_notify.h"
#include "DAB_Tuner_Ctrl_Request.h"
#include "msg_cmn.h"
#include "dab_app_freq_band.h"
#ifdef PC_TEST
//#include "tuner_response.h"
//#include "tuner_notify.h"
#endif


/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
extern Ts_dab_app_frequency_table dab_app_freq_band_eu[];
Tu32 u32_Index = 0 ;	/* Initialize station list index */
static Te_RADIO_ReplyStatus e_CreateSTLStatus = REPLYSTATUS_SUCCESS;

static Tu16 u16_DAB_App_SCIdI = 0 ;		/* Initialize SCIdI variable */
 Tu32 u32_DAB_App_SId = 0 ;	/* Initialize SId variable */
static Tu32 u32_Frequency = 0;	/* Initialize frequency variable */
static Tu16 u16_EId = 0;	/* Initialize EId variable */
Tu8 u8_MatchStationIndex = 0;

extern Tu16 u16_Num_Of_Stations ;
Tbool b_Anno_Ongoing_flag = FALSE;
Tbool b_Anno_Onstate = FALSE;
Tbool b_AnnoSID_TunedSIDSame = FALSE;
Tbool b_cancel_flag = FALSE;
Tbool b_Tuneup_down = FALSE;
Tbool b_AFtune_Request = FALSE;
Tu8 u8_linkingcheck = 3;
Ts_DAB_App_CurrentStationInfo st_AnnoStationInfo;
Ts_DAB_App_DataServiceRaw			st_Dabapp_SLS_Data;
extern	Ts_DabTunerMsg_GetCurrEnsembleProgListReply   st_Dab_Tuner_Ctrl_CurrEnsembleProgList;
extern Ts_Tuner_Ctrl_EnsembleInfo		ast_EnsembleInfo[DAB_APP_MAX_ENSEMBLES];
extern Ts_Tuner_Ctrl_ServiceInfo 		ast_ServiceInfo[DAB_APP_MAX_SERVICES];
extern Ts_Tuner_Ctrl_ComponentInfo		ast_ComponentInfo[DAB_APP_MAX_COMPONENTS];
extern Ts_DAB_Tuner_Ctrl_DataServiceRaw	st_DabTc_SLS_Data;
extern Tu16 ensembleIndex;
extern Tu16 Services;
extern Tu16 Components;
extern Tu16 u16_Num_Of_Stations ;
Tu8 u8_ServIndex=0, u8_ServCompIndex=0 ; 
/*	Defining states of DAB_APP_INST_HSM	*/

	HSM_CREATE_STATE(dab_app_inst_hsm_top_state,											NULL,										DAB_APP_INST_HSM_TopHndlr,						"dab_app_inst_hsm_top_state" );
		HSM_CREATE_STATE(dab_app_inst_hsm_inactive_state,									&dab_app_inst_hsm_top_state,				DAB_APP_INST_HSM_InactiveHndlr,					"dab_app_inst_hsm_inactive_state" );
		HSM_CREATE_STATE(dab_app_inst_hsm_active_state,										&dab_app_inst_hsm_top_state,				DAB_APP_INST_HSM_ActiveHndlr,					"dab_app_inst_hsm_active_state");
			HSM_CREATE_STATE(dab_app_inst_hsm_active_start_state,							&dab_app_inst_hsm_active_state,				DAB_APP_INST_HSM_ActiveStartHndlr,				"dab_app_inst_hsm_active_start_state");
			HSM_CREATE_STATE(dab_app_inst_hsm_active_backgnd_scan_state,					&dab_app_inst_hsm_active_state,				DAB_APP_INST_HSM_Active_BGScanHndlr,				"dab_app_inst_hsm_active_backgnd_scan_state");
			HSM_CREATE_STATE(dab_app_inst_hsm_active_idle_state,							&dab_app_inst_hsm_active_state,				DAB_APP_INST_HSM_ActiveIdleHndlr,				"dab_app_inst_hsm_active_idle_state");
				HSM_CREATE_STATE(dab_app_inst_hsm_active_idle_listened_state,				&dab_app_inst_hsm_active_idle_state,		DAB_APP_INST_HSM_ActiveIdleListenedHndlr,		"dab_app_inst_hsm_active_idle_listened_state");
					HSM_CREATE_STATE(dab_app_inst_hsm_active_idle_announcement_state,		&dab_app_inst_hsm_active_idle_listened_state,	DAB_APP_INST_HSM_ActiveIdleAnnouncementHndlr,	"dab_app_inst_hsm_active_idle_announcement_state");
			HSM_CREATE_STATE(dab_app_inst_hsm_active_stop_state,							&dab_app_inst_hsm_active_state,				DAB_APP_INST_HSM_ActiveStopHndlr,				"dab_app_inst_hsm_active_stop_state");
			HSM_CREATE_STATE(dab_app_inst_hsm_active_busy_state,							&dab_app_inst_hsm_active_state,				DAB_APP_INST_HSM_ActiveBusyHndlr,				"dab_app_inst_hsm_active_busy_state");
				HSM_CREATE_STATE(dab_app_inst_hsm_active_busy_scan_state,					&dab_app_inst_hsm_active_busy_state,		DAB_APP_INST_HSM_ActiveBusyScanHndlr,			"dab_app_inst_hsm_active_busy_scan_state");
				HSM_CREATE_STATE(dab_app_inst_hsm_active_busy_play_sel_stn_state,			&dab_app_inst_hsm_active_busy_state,		DAB_APP_INST_HSM_ActiveBusyPlaySelStnHndlr,		"dab_app_inst_hsm_active_busy_play_sel_stn_state");
				HSM_CREATE_STATE(dab_app_inst_hsm_active_busy_ser_comp_seek_state,			&dab_app_inst_hsm_active_busy_state,		DAB_APP_INST_HSM_ActiveBusysSerCompSeekHndlr,		"dab_app_inst_hsm_active_busy_ser_comp_seek_state");				
								

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_APP_INST_Init                                					 */
/*===========================================================================*/
void DAB_APP_INST_Init(Ts_dab_app_inst_hsm *pst_me_dab_app_inst)
{
	/* Clear instance hsm */
    memset(pst_me_dab_app_inst, (int) 0U, sizeof(Ts_dab_app_inst_hsm));
	
	pst_me_dab_app_inst->ptr_curr_hdlr_name =(const Tu8 *)(pst_me_dab_app_inst->str_state);

	/* Invoking constructor for instance Hsm */
	HSM_CTOR((Ts_hsm*)pst_me_dab_app_inst,&dab_app_inst_hsm_top_state,DAB_APP_INST_HSM_CID);	
	
	/* Start of instance Hsm */
	HSM_ON_START(pst_me_dab_app_inst);
}


/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_TopHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_TopHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
{
	Ts_Sys_Msg *ret = NULL; /* mark the message as handled */
	

  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
			
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_TopHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_TopHndlr \n";

			/* Transit to instance inactive state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_inactive_state);
		}
		break;

		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB APP] TopHndlr MSG: %d", pst_msg->msg_id);
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
 	
	}
	return ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_InactiveHndlr                           */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_InactiveHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_InactiveHndlr ");
 			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_InactiveHndlr \n";
		}
		break;

		case DAB_APP_INST_HSM_STARTUP:
		{			
			/* Transit to instance active start state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_start_state);

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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveHndlr                           */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveHndlr(Ts_dab_app_inst_hsm* pst_me_dab_app_inst, Ts_Sys_Msg* pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveHndlr \n";
		}
		break;
		case DAB_APP_CANCEL_REQID:
		{
			u32_Index = 0;

			/* Extracting Seek Cancel request status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_CancelType,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_CancelType),&u32_Index);

			if (pst_me_dab_app_inst->e_CancelType == DAB_APP_SEEK_CANCEL)
			{
				pst_me_dab_app_inst->e_CancelType = DAB_APP_TUNE_CANCEL;
				DAB_Tuner_Ctrl_Request_Cancel((Te_DAB_Tuner_Ctrl_CancelType)(pst_me_dab_app_inst->e_CancelType));
				pst_me_dab_app_inst->b_SeekStarted = FALSE;
			}
			else
			{
				/* Sending Cancel Scan request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_Cancel((Te_DAB_Tuner_Ctrl_CancelType)(pst_me_dab_app_inst->e_CancelType));
			}

		}
		break;
		case DAB_APP_ABORT_SCAN_RESID:
		{
			DAB_App_Update_TunerCtrlLayer_EnsembleInfoBuffer() ;
			DAB_App_Update_TunerCtrlLayer_ServiceInfoBuffer() ;
			DAB_App_Update_TunerCtrlLayer_ComponentInfoBuffer() ;
			DAB_App_Update_TunerCtrlLayer_MultiplexInfo();
		}
		break;
		
		case DAB_APP_CANCEL_RESID:
		{
			u32_Index = 0;

			/* Extracting Scan Cancel request status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			if(pst_me_dab_app_inst->e_CancelType == DAB_APP_AF_TUNE_CANCEL)
			{
				b_AFtune_Request = FALSE ;
			}

			pst_me_dab_app_inst->e_CancelType = DAB_APP_CANCEL_INVALID;
			/* Sending Scan Cancel response to Radio Manager */
			DAB_App_Response_Cancel(pst_me_dab_app_inst->e_Replystatus);

			/* Transit to Idle state*/
		}
		break;
		case DAB_APP_INST_HSM_SHUTDOWN:
		{
			/* Transit to Active stop state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_stop_state);

		}
		break;
		
		case DAB_APP_FACTORY_RESET_REQID:
		{
			pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type = DAB_APP_FACTORY_RESET_REQ;
			pst_me_dab_app_inst->u8_Handler_Check = 2;
			DAB_Tuner_Ctrl_FactoryReset_Request();
		}
		break;
		case DAB_APP_FACTORY_RESET_DONE_RESID:
		{
			u32_Index = 0;	/* Initialize station list index */
			/* Extracting Tuner Ctrl start up status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			if(pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{
				DAB_Inst_App_Response_Shutdown();
				//pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type = DAB_APP_ACTIVATE_REQ_INVALID;
				DAB_App_Response_FactoryReset(REPLYSTATUS_SUCCESS);
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_inactive_state);
			}
			else
			{
				DAB_App_Response_FactoryReset(REPLYSTATUS_FAILURE);
			}	
		}
		break;
		
		case DAB_APP_STATIONNOTAVAIL_STRATERGY_STATUS_NOTIFYID:
		{
			u32_Index =0 ;  
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_DAB_App_StationNotAvailStrategyStatus),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_App_StationNotAvailStrategyStatus),&u32_Index);
			DAB_Tuner_Ctrl_Notify_StationNootAvail_StrategyStatus((Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus)(pst_me_dab_app_inst->e_DAB_App_StationNotAvailStrategyStatus));
		}
		break;
		
		
		case DAB_APP_ANNO_CONFIG_REQID:
		{
			u32_Index =0 ;  
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->u16_AnnoConfig),(Tchar *)(pst_msg->data),(Tu8)sizeof(Tu16),&u32_Index);
			
			/* Send set announcement configuration request to Tuner Ctrl*/
			DAB_Tuner_Ctrl_Request_SetAnnoConfig(pst_me_dab_app_inst->u16_AnnoConfig);	
			if((b_Anno_Ongoing_flag == TRUE) && (pst_me_dab_app_inst->u16_AnnoConfig == (Tu16)0) && (b_AnnoSID_TunedSIDSame == TRUE))
			{
				b_Anno_Ongoing_flag = FALSE;
				b_AnnoSID_TunedSIDSame = FALSE;				
				pst_me_dab_app_inst->u8_SameChannelSubChId = (Tu8) 0x00 ;
				pst_me_dab_app_inst->u8_SameChannelClusterid = (Tu8) 0x00 ;
				memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
							
				/* Clear announcement station structure*/
				memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
				
				pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_ANNO_SETTINGS_OFF;
				
				/* Send stop indication to radio manager*/
				DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
				
				pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
				
				/*	Updating channel name according to frequency */
				DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
				
				/* Send tuned announcement station information to radio manager */
				DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);	
				
			}
			else if((b_Anno_Ongoing_flag == TRUE) && (pst_me_dab_app_inst->u16_AnnoConfig == (Tu16) 0))
			{
				pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_ANNO_SETTINGS_OFF;
				/* Notify Radio Manager*/
				DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 1.Request DAB_Tuner_Ctrl to stop announcement status: OFF ");
				/* Send cancel announcement request to tuner ctrl */
				DAB_Tuner_Ctrl_Request_CancelAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_Clusterid);
				b_cancel_flag = TRUE;
				b_Anno_Ongoing_flag = FALSE;
			}
			else
			{
				/* For MISRA */
			}
		}
		break;
		
		case DAB_APP_DAB_AF_LEARN_MEM_NOTIFYID:
		{
			u32_Index = 0;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] AF TUNE RESPONSE ");
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_DAB_App_LearnMemAFStatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_LearnMemAFStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
			DAB_App_Notify_UpdatedLearnMem_AFStatus(pst_me_dab_app_inst->e_DAB_App_LearnMemAFStatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
		}
		break;
		case DAB_APP_ANNO_CONFIG_RESID:
		{
			u32_Index =0 ;  
			
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_Replystatus),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_ReplyStatus),&u32_Index);	
			
			DAB_App_Response_SetAnnoConfig(pst_me_dab_app_inst->e_Replystatus);
			if(pst_me_dab_app_inst->u16_AnnoConfig == (Tu16)0)
			{
				memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
							
				/* Clear announcement station structure*/
				memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
			}
			else
			{
				
			}/*For MISRA*/

		}
		break;		
		
		case DAB_APP_DAB_AF_SETTINGS_REQID:
		{
			u32_Index =0 ;  
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_DAB_App_RDS_Settings),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_App_AF_Switch),&u32_Index);
			DAB_Tuner_Ctrl_RDS_Settings_Request((Te_DAB_Tuner_Ctrl_DAB_AF_Settings)(pst_me_dab_app_inst->e_DAB_App_RDS_Settings));
		}
		break;
		
		case DAB_APP_DAB_AF_SETTINGS_RESID:
		{
			u32_Index =0 ;  
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_Replystatus),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_Response_DAB_AF_Settings(pst_me_dab_app_inst->e_Replystatus);
		}
		break;
		
		case DAB_APP_DAB_FM_LINKING_ENABLE_REQID:
		{
			u32_Index =0 ;  
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_Linking_Switch_Status),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_App_DABFMLinking_Switch),&u32_Index);

			DAB_Tuner_Ctrl_Request_EnableDABtoFMLinking((Te_DAB_Tuner_DABFMLinking_Switch)(pst_me_dab_app_inst->e_Linking_Switch_Status));
		}
		break;
  		
		case DAB_APP_DAB_FM_LINKING_ENABLE_RESID:
		{
			u32_Index =0 ;  
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_LinkingSettings),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_Response_EnableDABtoFMLinking((Te_RADIO_ReplyStatus)pst_me_dab_app_inst->e_LinkingSettings);
		}
		break;

		case DAB_APP_FREQ_CHANGE_NOTIFYID:
		{
			u32_Index = 0 ;	/* Initialize station list index */

			/* Extracting Tuner Ctrl frequency change response status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u32_Frequency_Change,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu32),&u32_Index);
			
			
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->u32_Frequency_Change,pst_me_dab_app_inst->au8_ChannelName);
			
			/* Sending frequency change notification to Radio Manager */
			DAB_App_Notify_FrequencyChange(pst_me_dab_app_inst->u32_Frequency_Change,pst_me_dab_app_inst->au8_ChannelName);

		}
		break;
		
		case DAB_APP_AMFMTUNER_STATUS_NOTIFYID :
		{
			u32_Index = 0 ;	/* Initialize station list index */

			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_AMFMTUNERStatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_Comp_Status),&u32_Index);
			
			DAB_Tuner_Ctrl_Notify_AMFMTunerStatus((Te_RADIO_Comp_Status)(pst_me_dab_app_inst->e_AMFMTUNERStatus));
		}
		break;
		
		case DAB_APP_ENG_MODE_REQID:
		{
			u32_Index =0 ;  
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_DAB_APP_Eng_Mode_Request),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_APP_Eng_Mode_Request),&u32_Index);	
			DAB_Tuner_Ctrl_Request_ENG_Mode((Te_DAB_Tuner_Ctrl_Eng_Mode_Request)(pst_me_dab_app_inst->e_DAB_APP_Eng_Mode_Request));
		}
		break;	
		case DAB_APP_BACKGROUND_SCAN_START_NOTIFYID :
		{
			DAB_Tuner_Ctrl_Notify_StartBackgroundScan();
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveBusyHndlr                           */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveBusyHndlr(Ts_dab_app_inst_hsm* pst_me_dab_app_inst, Ts_Sys_Msg* pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveBusyHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveBusyHndlr \n";

		}
		break;
		case DAB_APP_COMP_LIST_SORT_REQID:
		{   
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
	     	Dab_App_Sort_CurrEnsembleProgList(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList);
		 	SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList),&(st_Dab_Tuner_Ctrl_CurrEnsembleProgList),sizeof(st_Dab_Tuner_Ctrl_CurrEnsembleProgList));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
			DAB_App_Response_Sort();
		}
		break;
		
	 	case DAB_APP_GET_COMP_LIST_STATUS_NOTIFYID:
		{
			u32_Index = 0;
			/* Extract frequency, EId and replystatus info sent by DAB Tuner Ctrl */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u32_Frequency,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu32),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u16_EId,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			if(pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{  	
				/* Select the first SId and SCId to tune and send select service request to DAB Tuner Ctrl */
			    u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency;
				u16_EId = pst_me_dab_app_inst->st_tunableinfo.u16_EId;	/* Initialize EId variable */
			    u32_DAB_App_SId = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[0].ProgServiceId;
				u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId;
			    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI);
				pst_me_dab_app_inst->st_tunableinfo.u32_SId = u32_DAB_App_SId;
				pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = u16_DAB_App_SCIdI;
			}
			

		}
		break;

		case DAB_APP_CANCEL_REQID:
		{
			u32_Index = 0;

			/* Extracting Seek Cancel request status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_CancelType,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_CancelType),&u32_Index);
			
			if(pst_me_dab_app_inst->e_CancelType == DAB_APP_SEEK_CANCEL)
			{
				pst_me_dab_app_inst->e_CancelType = DAB_APP_TUNE_CANCEL;
				DAB_Tuner_Ctrl_Request_Cancel((Te_DAB_Tuner_Ctrl_CancelType)(pst_me_dab_app_inst->e_CancelType));
			  	pst_me_dab_app_inst->b_SeekStarted = FALSE;
			}
			else
			{
				/* Sending Cancel Scan request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_Cancel((Te_DAB_Tuner_Ctrl_CancelType)(pst_me_dab_app_inst->e_CancelType));
			}

		}
		break;
		
		case DAB_APP_CANCEL_RESID:
		{
			u32_Index = 0;

			/* Extracting Scan Cancel request status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			if(pst_me_dab_app_inst->e_CancelType == DAB_APP_AF_TUNE_CANCEL)
			{
				b_AFtune_Request = FALSE ;
			}
			
			pst_me_dab_app_inst->e_CancelType =	DAB_APP_CANCEL_INVALID;
			/* Sending Scan Cancel response to Radio Manager */
			DAB_App_Response_Cancel(pst_me_dab_app_inst->e_Replystatus);

			/* Transit to Idle state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
		}
		break;
		
		case DAB_APP_DAB_FM_BLENDING_STATUS_NOTIFYID:
		{
			u32_Index = 0 ;	

			/* Extracting PI Code list structure from Tuner Ctrl msg */ 
			ExtractParameterFromMessage(&pst_me_dab_app_inst->e_LinkingStatus,(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_DABFM_LinkingStatus),&u32_Index);
			
			/* Sending PI Code list Notification to Radio Manager */
			DAB_App_Notify_DABtoFM_BlendingStatus(pst_me_dab_app_inst->e_LinkingStatus);

		}
		break;
		
		case DAB_APP_COMPONENT_STATUS_NOTIFYID:
		{
			u32_Index = 0;	/* Initialize index */
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_ComponentStatus),(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_Comp_Status),&u32_Index);
			if(pst_me_dab_app_inst->e_ComponentStatus == RADIO_FRMWK_COMP_STATUS_ABNORMAL)
			{
				DAB_App_Notify_ComponentStatus(pst_me_dab_app_inst->e_ComponentStatus) ;
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
			}
			else
			{
				/* No action need to be taken */	
			}
		}
		break ;
		
		case DAB_APP_SYNCHRONISATION_NOTIFYID:
		{   
			/* There is no need handle this in Parent and hence just capturing the msg*/
		}
		break ;
	
		
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveStartHndlr                           */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveStartHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveStartHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveStartHndlr \n";

			/* Initialize Announcement Enum's*/
			pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_ANNO_INVALID;
			if(pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type == DAB_APP_ACTIVATE_REQ_VALID)
			{
				DAB_Tuner_Ctrl_Request_Activate_Deactivate((Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus)(pst_me_dab_app_inst->e_DAB_App_ActivateDeactivateStatus));
			}
			else
			{
				if(pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type == DAB_APP_FACTORY_RESET_REQ)
				{
				}
				else
				{
					pst_me_dab_app_inst->u8_Handler_Check = 2;
				}	
				/* Checking if warm-startup, then updating DAB TC EnsembleInfo, ServiceInfo, ComponentInfo buffers */
				if(pst_me_dab_app_inst->u8_StartType == 0xAA)
				{
					SYS_MUTEX_LOCK(STL_RM_DAB_APP);
					
					DAB_App_Update_TunerCtrlLayer_EnsembleInfoBuffer() ;
					DAB_App_Update_TunerCtrlLayer_ServiceInfoBuffer() ;
					DAB_App_Update_TunerCtrlLayer_ComponentInfoBuffer() ;
					DAB_App_Update_TunerCtrlLayer_MultiplexInfo();
					
					SYS_MUTEX_UNLOCK(STL_RM_DAB_APP);
				}				
				/* Sending DAB startup request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_Startup((Te_DAB_Tuner_Market)(pst_me_dab_app_inst->e_Market),pst_me_dab_app_inst->u8_SettingStatus,pst_me_dab_app_inst->u8_StartType);
			}	

#ifdef PC_TEST
			DAB_Tuner_Ctrl_Response_Startup(REPLYSTATUS_SUCCESS);
#endif
		}
		break;
		
		case DAB_APP_ACTIVATE_DEACTIVATE_RESID:
		{
			
			u32_Index = 0;	/* Initialize station list index */

			/* Extracting Tuner Ctrl start up status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_Inst_App_Response_Startup();
			DAB_App_Response_Activate_Deactivate(pst_me_dab_app_inst->e_Replystatus);
			pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type = DAB_APP_ACTIVATE_REQ_INVALID;
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_backgnd_scan_state);
		}
		break;
				
		case DAB_APP_STARTUP_DONE_RESID:
		{
			u32_Index = 0;	/* Initialize station list index */

			/* Extracting Tuner Ctrl start up status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					/* Sending DAB instance hsm startup success response to Main hsm */
					DAB_Inst_App_Response_Startup();		

					/* Sending DAB startup success response to Radio Manager */
 					DAB_App_Response_Startup(pst_me_dab_app_inst->e_Replystatus);
					
				}
				break;

				case REPLYSTATUS_FAILURE:
				{
					/* Sending DAB startup failure response to Radio Manager */
 					DAB_App_Response_Startup(pst_me_dab_app_inst->e_Replystatus);
				}
				break;
   
				case REPLYSTATUS_REQ_TIMEOUT:
				{
					/* Sending DAB startup timeout response to Radio Manager */
					DAB_App_Response_Startup(pst_me_dab_app_inst->e_Replystatus);
				}
				break;

				default:
				{

				}
				break;

			}
			if(pst_me_dab_app_inst->u8_Handler_Check == 2)
			{
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_backgnd_scan_state);
			}
			else
			{
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
			}	
					
		}
		break;
/*
		case DAB_APP_SELECT_DAB_REQID:
		{
			// Sending DAB activate request to Tuner Ctrl
			DAB_Tuner_Ctrl_Request_Activate();
	
			// Transit to instance active idle state
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
					
		}
		break;
	*/		
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveBusyScanHndlr                 */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveBusyScanHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveBusyScanHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveBusyScanHndlr \n";
			if(pst_me_dab_app_inst->e_DAB_App_StationNotAvailStrategyStatus	!= DAB_APP_STATIONNOTAVAIL_STRATEGY_START)
			{
			/* Sending Scan request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_Scan((Tbool)(RADIO_FRMWK_DIRECTION_UP));
			}
#ifdef PC_TEST			
			/*  Scan response from Tuner Ctrl */
			DAB_Tuner_Ctrl_Response_Scan(REPLYSTATUS_SUCCESS);
#endif
		}
		break;

		case DAB_APP_SCAN_RESID:
		{
			 u32_Index = 0 ;	/* Initialize station list index */

			/* Extracting Tuner Ctrl scan response status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					/* Sending GETSTL success response to Radio Manager */
					DAB_App_Response_GetStationList(pst_me_dab_app_inst->e_Replystatus);
				}
				break;

				case REPLYSTATUS_INVALID_PARAM: 
				{

				}
				break;

				case REPLYSTATUS_SYSTEMISCURRENTLYSEARCHING:
				{

				}
				break;

				case REPLYSTATUS_REQ_TIMEOUT:
				{
					DAB_App_Response_GetStationList(pst_me_dab_app_inst->e_Replystatus);
				}
				break;

				case REPLYSTATUS_NO_SIGNAL:
				{
					DAB_App_Response_GetStationList(pst_me_dab_app_inst->e_Replystatus);
				}
				break;

				default:
				{

				}
				break;
			}

#ifdef PC_TEST				
			/* STLUpdated notification from Tuner Ctrl */
			DAB_Tuner_Ctrl_Notify_STLUpdated(STL_RM_DAB_APP);
#endif
		}
		break;

		case DAB_APP_AUTOSCAN_PLAY_STATION_NOTIFYID:
		{

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_APP] Play select response");
			u32_Index = 0;	/* Initialize station list index */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus, (Tchar *)(pst_msg->data), (Tu8) sizeof(Te_RADIO_ReplyStatus), &u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo, (Tchar *)(pst_msg->data), (Tu8) sizeof(Ts_DAB_App_CurrentStationInfo), &u32_Index);

			/*	Updating channel name according to frequency */
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency, pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);

			if (pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{
				/* Sending Play SCAN FIRST Station of each ensemble response to Radio Manager */
				DAB_App_Notify_AutoScan_PlayStation(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
			}
			else
			{
			
			}
		}
		break;
		

		case DAB_APP_STL_UPDATE_NOTIFYID:
		{
			 e_CreateSTLStatus = REPLYSTATUS_SUCCESS;
			 u32_Index = 0 ;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Station list update notification ");
			/* Extracting Tuner Ctrl STL updated notify parameter from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_SharedMemoryType,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_SharedMemoryType),&u32_Index);

			if(pst_me_dab_app_inst->e_SharedMemoryType == DAB_TUNER_APP)
			{
				/* Create Station list */
				e_CreateSTLStatus = DAB_APP_CreateStationList(pst_me_dab_app_inst->e_DAB_App_RequestCmd,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency);
				
				if(e_CreateSTLStatus == REPLYSTATUS_SUCCESS)
				{
					/* Station list Sort */
					DAB_APP_EnsembleSortSTL();
					DAB_APP_SortSTL();
					
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
				}
				else
				{
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
					
				}
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
			}
			else
			{
				
			}/* For misra c */
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveIdleHndlr                     */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveIdleHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu16 u16_FM_DAB_SID;

  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveIdleHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveIdleHndlr \n";
			pst_me_dab_app_inst->u8_Handler_Check = 1;

#ifdef PC_TEST
			DAB_Tuner_Ctrl_Response_Activate(REPLYSTATUS_SUCCESS);
#endif			
		}
		break;

		case DAB_APP_ACTIVATE_RESID:
		{
			u32_Index = 0 ;	/* Initialize station list index */
			
			/* Extracting Tuner Ctrl activate response status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					/* Sending DAB select success response to Radio Manager */
					DAB_App_Response_SelectBand(pst_me_dab_app_inst->e_Replystatus);

				}
				break;

				case REPLYSTATUS_REQ_CANCELLED: 
				{
					
				}
				break;

				case REPLYSTATUS_REQ_ERROR:
				{

				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;
			
		case DAB_APP_DESELECT_DAB_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] De-select band request");
			b_Tuneup_down = FALSE;
			/* Transit to instance active sleep state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_backgnd_scan_state);

		}
		break;
		
		case DAB_APP_AF_TUNE_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] AF-Tune request");
     		 u32_Index = 0 ;	/* Initialize index */
 			DAB_App_ExtractParameterFromMessage(&u16_FM_DAB_SID,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
 			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_AFTUNE;
			 pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = 0;
			pst_me_dab_app_inst->st_tunableinfo.u32_SId = u16_FM_DAB_SID;
			 b_AFtune_Request = TRUE;
  			 HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_busy_play_sel_stn_state);
	 
		}
		break;
		case DAB_APP_MANUAL_TUNEBY_CHNAME_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Manual Tune by Channel Name request");
			u32_Index = 0 ;	/* Initialize index */
 			DAB_App_ExtractParameterFromMessage(pst_me_dab_app_inst->au8_ChannelName,(const Tchar *)(pst_msg->data),(Tu8) (sizeof(Tu8) *DAB_TUNER_CTRL_MAX_LABEL_LENGTH),&u32_Index);
			DAB_App_Updatefrequency(pst_me_dab_app_inst->au8_ChannelName, &(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency));
			if(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency == pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency)
			{
				/* Already tuned to requested station */
				pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS ;
				
				/* Sending Manual Tune by Channel name response to Radio Manager */
				DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
			}
			else
			{
				pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_MANUAL_TUNEBY_CHNAME;
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_busy_play_sel_stn_state);
			}
		}
		break;
		
		case DAB_APP_GETSTL_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Manual station list update request");
			b_Tuneup_down = FALSE;
			pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_BANDSCAN;
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_busy_scan_state);

		}
		break;

		case DAB_APP_PLAY_SEL_STN_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Play select request");
			u32_Index = 0 ;	/* Initialize index */
			b_Tuneup_down = FALSE;
			/* Extracting station index from msg in the play select request */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u32_Frequency,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu32),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u16_EId,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u32_SId,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu32),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			
			if((pst_me_dab_app_inst->st_tunableinfo.u32_SId ==(Tu32) 0) && (pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI ==(Tu16) 0))
			{
				pst_me_dab_app_inst->e_TuneRequest = DAB_APP_REQUEST_MANUAL_TUNE ;
			}
		
			/* Transit to instance active busy play select station state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_busy_play_sel_stn_state);
		
		}
		break;
		
		case DAB_APP_DAB_AF_LEARN_MEM_NOTIFYID:
		{
			u32_Index = 0;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] AF TUNE RESPONSE ");
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_DAB_App_LearnMemAFStatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_LearnMemAFStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			DAB_App_Notify_UpdatedLearnMem_AFStatus(pst_me_dab_app_inst->e_DAB_App_LearnMemAFStatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
		}
		break;
		case DAB_APP_PLAY_SEL_STN_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Play select response");
			u32_Index = 0;	/* Initialize station list index */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
						
			/*	Updating channel name according to frequency */
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			
			if(pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{
				if(pst_me_dab_app_inst->e_DAB_App_StationNotAvailStrategyStatus	== DAB_APP_STATIONNOTAVAIL_STRATEGY_START)
				{
					if(b_AFtune_Request == TRUE)
					{
						DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
						b_AFtune_Request = FALSE;
					}
				}
				else
				{
					/* Sending Play Select Station response to Radio Manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
				}
				/* Transit to active Idle listen state*/
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_listened_state);
			}
			else
			{
				if(pst_me_dab_app_inst->e_DAB_App_StationNotAvailStrategyStatus	== DAB_APP_STATIONNOTAVAIL_STRATEGY_START)
				{
					if(b_AFtune_Request == TRUE)
					{
						DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
						b_AFtune_Request = FALSE;
					}
				}
				else
				{
					/* Sending Play Select Station response to Radio Manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
				}
				/* Transit to active Idle listen state*/
			}/*FOR MISRA C*/
		}
		break;
			
		case DAB_APP_TUNEUPDOWN_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Tune Up-Down request");
			u32_Index = 0 ;	/* Initialize index */
			b_Tuneup_down = TRUE;
			//Tu8 u8_ComparisonIndex = 0 ;
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_TuneUpDownDirection,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_DirectionType),&u32_Index);
			
			pst_me_dab_app_inst->e_TuneRequest = DAB_APP_REQUEST_TUNE_UP_DOWN ;
			
			for(pst_me_dab_app_inst->u8_Curr_Frequency_Index = 0 ; pst_me_dab_app_inst->u8_Curr_Frequency_Index < DAB_APP_EU_MAX_FREQUENCIES ; pst_me_dab_app_inst->u8_Curr_Frequency_Index++) /* Finding the currently tuned frequency in the band */
			{
				if(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency == dab_app_freq_band_eu[pst_me_dab_app_inst->u8_Curr_Frequency_Index].u32_frequency) /* currently tuned frequency matched in the band */
				{
					if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency == DAB_APP_5A_EU) && (pst_me_dab_app_inst->e_TuneUpDownDirection == RADIO_FRMWK_DIRECTION_DOWN)) /* If the currently tuned frequency is 5A and Tune Up/Down direction is Down then tunable frequency is 13F */ 
					{
						pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = DAB_APP_13F_EU ;
					}
					else if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency != DAB_APP_5A_EU) && (pst_me_dab_app_inst->e_TuneUpDownDirection == RADIO_FRMWK_DIRECTION_DOWN)) /* If the currently tuned frequency is other than 5A and Tune Up/Down direction is Down, then select the previous frequency in the band*/
					{
						pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = dab_app_freq_band_eu[pst_me_dab_app_inst->u8_Curr_Frequency_Index - 1].u32_frequency ;
					}
					else if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency == DAB_APP_13F_EU) && (pst_me_dab_app_inst->e_TuneUpDownDirection == RADIO_FRMWK_DIRECTION_UP)) /* If the currently tuned frequency is 13F and Tune Up/Down direction is Up then tunable frequency is 5A */ 
					{
						pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = DAB_APP_5A_EU ;
					}
					else if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency != DAB_APP_13F_EU) && (pst_me_dab_app_inst->e_TuneUpDownDirection == RADIO_FRMWK_DIRECTION_UP)) /* If the currently tuned frequency is other than 13F and Tune Up/Down direction is Up, then select the next frequency in the band */
					{
						pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = dab_app_freq_band_eu[pst_me_dab_app_inst->u8_Curr_Frequency_Index + 1].u32_frequency;
					}
					else
					{
						/* Do nothing */
					}/* For MISRA C */
					
					break ; /* break from the loop if the frequency is mathced */
									
				}
			}
			pst_me_dab_app_inst->st_tunableinfo.u16_EId = 0;
			pst_me_dab_app_inst->st_tunableinfo.u32_SId = 0;
			pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = 0;
			
			/* Transit to instance active busy play select station state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_busy_play_sel_stn_state);
		}
		break;	
				
		case DAB_APP_SER_COMP_SEEK_REQID:  
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Seek request");
			u32_Index = 0  ;
			b_Tuneup_down = FALSE;
			pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_SER_COMP_SEEK;
			/* Extracting seek start frequency from msg in the service component seek request  */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u32_SeekFrequency,(Tchar *)(pst_msg->data),(Tu8)sizeof(Tu32),&u32_Index);

			/* copying Seek start Freqency in next tunable variable to check for boundary freq check  */
			pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
			/* Extracting seek direction from msg in the service component seek request  */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_SeekDirection,(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_DirectionType),&u32_Index);

			/* Transit to instance active busy service component seek state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_busy_ser_comp_seek_state);

		}
		break;
					
		case DAB_APP_RECONFIG_NOTIFYID:
		{
			u32_Index =0 ;  
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Multiplex Re-Configuration notification ");
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_ReConfigType),(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_App_ReConfigType),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency),(Tchar *)(pst_msg->data),(Tu8)sizeof(Tu32),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->st_tunableinfo.u16_EId),(Tchar *)(pst_msg->data),(Tu8)sizeof(Tu16),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->st_tunableinfo.u32_SId),(Tchar *)(pst_msg->data),(Tu8)sizeof(Tu32),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI),(Tchar *)(pst_msg->data),(Tu8)sizeof(Tu16),&u32_Index);
			DAB_App_Notify_ReConfiguration(pst_me_dab_app_inst->st_tunableinfo);
			/* Transit to active idle state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
			
		}
		break;
		case DAB_APP_STL_UPDATE_NOTIFYID:
		{
				
				e_CreateSTLStatus = REPLYSTATUS_SUCCESS;
				u32_Index = 0 ;	/* Initialize station list index */
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Station list update notification ");
				/* Extracting Tuner Ctrl STL updated notify parameter from msg */ 

				DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_SharedMemoryType,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_SharedMemoryType),&u32_Index);
				
				if(pst_me_dab_app_inst->e_SharedMemoryType == DAB_TUNER_APP)
				{
					/* Create Station list */
					e_CreateSTLStatus = DAB_APP_CreateStationList(pst_me_dab_app_inst->e_DAB_App_RequestCmd,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency);

					if(e_CreateSTLStatus == REPLYSTATUS_SUCCESS)
					{
						/* Station list Sort */
						DAB_APP_EnsembleSortSTL();
						DAB_APP_SortSTL();
						
						/* Sending STL update notification to Radio Manager */
						DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
					}
					else
					{
						/* Sending STL update notification to Radio Manager */
						DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);

					}
					
				//	HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
				}
				else
				{

				}/* For misra c */
			}
		break;
		
		case DAB_APP_COMPONENT_STATUS_NOTIFYID:
		{
			
			u32_Index = 0;	/* Initialize index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] DABTuner status notification ");
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_ComponentStatus),(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_Comp_Status),&u32_Index);
			DAB_App_Notify_ComponentStatus(pst_me_dab_app_inst->e_ComponentStatus) ;
		}
		break ;
		
		case DAB_APP_DABTUNER_RESTART_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DABTuner Restart request ");
			DAB_Tuner_Ctrl_Request_DABTUNERRestart();
		}
		break;
		
		case DAB_APP_DABTUNER_RESTART_RESID:
		{
			u32_Index = 0;	/* Initialize index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DABTuner Restart response ");
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_Replystatus),(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_Response_DABTUNERRestart(pst_me_dab_app_inst->e_Replystatus) ;
		}
		break;		

		case DAB_APP_COMP_LIST_SORT_REQID:
		{
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
			Dab_App_Sort_CurrEnsembleProgList(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList);
			SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList), &(st_Dab_Tuner_Ctrl_CurrEnsembleProgList), sizeof(st_Dab_Tuner_Ctrl_CurrEnsembleProgList));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
			DAB_App_Response_Sort();
		}
		break;

		case DAB_APP_GET_COMP_LIST_STATUS_NOTIFYID:
		{
			u32_Index = 0;
			/* Extract frequency, EId and replystatus info sent by DAB Tuner Ctrl */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u32_Frequency, (const Tchar *)(pst_msg->data), (Tu8) sizeof(Tu32), &u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u16_EId, (const Tchar *)(pst_msg->data), (Tu8) sizeof(Tu16), &u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus, (const Tchar *)(pst_msg->data), (Tu8) sizeof(Te_RADIO_ReplyStatus), &u32_Index);

			if (pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{
				/* Select the first SId and SCId to tune and send select service request to DAB Tuner Ctrl */
				u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency;
				u16_EId = pst_me_dab_app_inst->st_tunableinfo.u16_EId;	/* Initialize EId variable */
				u32_DAB_App_SId = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[0].ProgServiceId;
				u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId;
				DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI);
				pst_me_dab_app_inst->st_tunableinfo.u32_SId = u32_DAB_App_SId;
				pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = u16_DAB_App_SCIdI;
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveIdleListenedHndlr             */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveIdleListenedHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO] DAB_APP_INST_HSM_ActiveIdleListenedHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveIdleListenedHndlr \n";
		}
		break;
		
		case DAB_APP_STL_UPDATE_NOTIFYID:
		{
			 e_CreateSTLStatus = REPLYSTATUS_SUCCESS;
			 u32_Index = 0 ;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Station list update notification ");
			pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_AFTUNE;
			/* Extracting Tuner Ctrl STL updated notify parameter from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_SharedMemoryType,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_SharedMemoryType),&u32_Index);
			//DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_DAB_App_RequestCmd,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_DAB_App_RequestCmd),&u32_Index);
	
			if(pst_me_dab_app_inst->e_SharedMemoryType == DAB_TUNER_APP)
			{
				/* Create Station list */
				e_CreateSTLStatus = DAB_APP_CreateStationList(pst_me_dab_app_inst->e_DAB_App_RequestCmd,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency);
		
				if(e_CreateSTLStatus == REPLYSTATUS_SUCCESS)
				{
					/* Station list Sort */
					DAB_APP_EnsembleSortSTL();
					DAB_APP_SortSTL();
			
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
					pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;
				}
				else
				{
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
					pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;
			
				}
				//HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
			}
			else
			{
		
			}/* For misra c */
		}
		break;

		case DAB_APP_STATUS_NOTIFYID:
		{
			
			u32_Index = 0 ;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Tuner status notification ");
			/* Extracting tuner status notify structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_TunerStatusNotify,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Ts_DAB_APP_Status_Notification),&u32_Index);
			//memcpy(&(pst_me_dab_app_inst->s_DAB_App_Lsmstationinfo),&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo),sizeof(Ts_DAB_App_CurrentStationInfo));
			if((pst_me_dab_app_inst->st_TunerStatusNotify.s8_BER_Exponent >= -2) && (pst_me_dab_app_inst->st_TunerStatusNotify.u8_Decodingstatus >= 0x02))
			{
				//memset(&(pst_me_dab_app_inst->s_DAB_App_Lsmstationinfo.st_ServiceLabel.au8_label),0,DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
				//memset(&(pst_me_dab_app_inst->s_DAB_App_Lsmstationinfo.st_ComponentLabel.au8_label),0,DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
				//memcpy(&(pst_me_dab_app_inst->s_DAB_App_Lsmstationinfo.st_ServiceLabel.au8_label),"Sig Low",8);
				//pst_me_dab_app_inst->s_DAB_App_Lsmstationinfo.st_ServiceLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
				pst_me_dab_app_inst->e_DAB_App_SignalStatus = DAB_APP_SIGNAL_LOW ;
			}
			else
			{
				pst_me_dab_app_inst->e_DAB_App_SignalStatus = DAB_APP_SIGNAL_HIGH ;
			}
			/* Sending tuner status status Notification to Radio Manager */
			DAB_App_Notify_TunerStatus(pst_me_dab_app_inst->st_TunerStatusNotify, pst_me_dab_app_inst->e_DAB_App_SignalStatus);
		}

		break;
		
		case DAB_APP_PICODE_LIST_NOTIFYID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] PI list notification ");
			/* Extracting PI Code list structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_DAB_PICodeList,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_PICodeList),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u8_QualityMin,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu8),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u8_QualityMax,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu8),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&u32_DAB_App_SId,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu32),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&u8_linkingcheck,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu8),&u32_Index);
			/* Sending PI Code list Notification to Radio Manager */
			DAB_App_Notify_PICodeList(pst_me_dab_app_inst->st_DAB_PICodeList,pst_me_dab_app_inst->u8_QualityMin,pst_me_dab_app_inst->u8_QualityMax,u32_DAB_App_SId,u8_linkingcheck);

		}
		break;
		
		case DAB_APP_DAB_DAB_STATUS_NOTIFYID:
		{
			u32_Index = 0 ;	
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_DAB_APP_DAB_DAB_Status,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_APP_DAB_DAB_Status),&u32_Index);
			DAB_App_Notify_DAB_DAB_Status(pst_me_dab_app_inst->e_DAB_APP_DAB_DAB_Status);
		}
		break;
		
		case DAB_APP_BESTPI_NOTIFYID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Best PI notification ");
			/* Extracting PI Code list structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->PICode,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Tu16),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u8_Quality,(const Tchar *)(pst_msg->data),(Tu8)sizeof(pst_me_dab_app_inst->u8_Quality),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_BestPI_Type,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_App_BestPI_Type),&u32_Index);
			/* Sending PI Code list Notification to Radio Manager */
			DAB_Tuner_Ctrl_Notify_BestPI(pst_me_dab_app_inst->PICode,pst_me_dab_app_inst->u8_Quality,(Te_Tuner_Ctrl_BestPI_Type)(pst_me_dab_app_inst->e_BestPI_Type));
		}
		break;
		
		case DAB_APP_DAB_FM_LINKING_STATUS_NOTIFYID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB-FM Linking status notification ");
			/* Extracting PI Code list structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_LinkingStatus,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_DABFM_LinkingStatus),&u32_Index);
			
			/* Sending PI Code list Notification to Radio Manager */
			DAB_Tuner_Ctrl_Notify_DABtoFM_LinkingStatus((Te_RADIO_DABFM_LinkingStatus)(pst_me_dab_app_inst->e_LinkingStatus));

		}
		break;	
		
		case DAB_APP_PI_QUALITY_NOTIFYID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] PI quality notification ");
			/* Extracting PI Code list structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u8_Quality,(Tchar *)(pst_msg->data),(Tu8)sizeof(Tu8),&u32_Index);
			
			/* Sending PI Code list Notification to Radio Manager */
			DAB_Tuner_Ctrl_Notify_PIQuality(pst_me_dab_app_inst->u8_Quality);

		}
		break;
		
		case DAB_APP_DAB_FM_BLENDING_STATUS_NOTIFYID:
		{
			u32_Index = 0 ;	
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB-FM blending status notification ");
			/* Extracting PI Code list structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_LinkingStatus,(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_DABFM_LinkingStatus),&u32_Index);
			
			/* Sending PI Code list Notification to Radio Manager */
			DAB_App_Notify_DABtoFM_BlendingStatus(pst_me_dab_app_inst->e_LinkingStatus);

		}
		break;
		
		case DAB_APP_DAB_DLS_DATA_NOTIFYID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] DLS notification ");
			/* Extracting DLS data structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_DLSData,(Tchar *)(pst_msg->data),(Tu8)sizeof(Ts_DAB_DLS_Data),&u32_Index);
			
			/* Sending DLS data Notification to Radio Manager */
			DAB_App_Notify_DLSData(pst_me_dab_app_inst->st_DLSData);
			
		}
		break;
		case DAB_APP_DAB_SLS_DATA_NOTIFYID:
		{
			u32_Index = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_APP] SLS notification ");
			/*Accessing the shared memory for reading the SLS image*/
			SYS_MUTEX_LOCK(SLS_DAB_APP_DAB_TC);
			SYS_MUTEX_LOCK(SLS_RM_DAB_APP);
			SYS_RADIO_MEMCPY(&st_Dabapp_SLS_Data, &st_DabTc_SLS_Data, sizeof(Ts_DAB_Tuner_Ctrl_DataServiceRaw));
			SYS_MUTEX_UNLOCK(SLS_DAB_APP_DAB_TC);
			SYS_MUTEX_UNLOCK(SLS_RM_DAB_APP);
			/* Sending SLS data Notification to Radio Manager */
			DAB_App_Notify_SLSData();
		}
		break;
		
		case DAB_APP_DAB_FM_HARDLINKS_STATUS_NOTIFYID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] DLS notification ");
			/* Extracting PI Code list structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_LinkingStatus,(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_DABFM_LinkingStatus),&u32_Index);
			
			DAB_App_Notify_Hardlinks_Status(pst_me_dab_app_inst->e_LinkingStatus);
			
		}
		break;
		
		case DAB_APP_ANNO_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Received Announcement Notification from Tuner Ctrl ");
			
			u32_Index = 0 ;	  			
			
			/* Extracting announcement start or stop status information from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u8_DAB_APP_announcement_status,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Tu8),&u32_Index);

			/* Extracting announcement structure information from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_CurrAnnoInfo,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Ts_DAB_APP_Curr_Anno_Info),&u32_Index);
			
			/*when the announcment occurs in the same tuned SID*/
			if((pst_me_dab_app_inst->st_CurrAnnoInfo.u32_SId == pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_SId)
			&&(pst_me_dab_app_inst->st_CurrAnnoInfo.u8_SubChId == pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u16_SCIdI))
			{
			   if(pst_me_dab_app_inst->u8_DAB_APP_announcement_status == DAB_APP_ANNO_START) 
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Same Channel Announcement start Notification from Tuner Ctrl ");
					if(b_Anno_Ongoing_flag == FALSE)
					{
				  		pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_ON;
			
						b_Anno_Ongoing_flag = TRUE;
						b_AnnoSID_TunedSIDSame = TRUE;
			         	SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_Tuned_Anno_Info), &(pst_me_dab_app_inst->st_CurrAnnoInfo),sizeof(Ts_DAB_APP_Curr_Anno_Info));
								
						pst_me_dab_app_inst->u8_SameChannelSubChId = pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId ;	
						pst_me_dab_app_inst->u8_SameChannelClusterid = pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_Clusterid ;
						
						/* Store extracted current announcement information in structure st_Tuned_Anno_Info */
					
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Send same channel announcement ON notification to Tuner Ctrl ");
					
						/* Notify DAB_Tuner_Ctrl that samechannel announcement status as ON */
						DAB_App_Notify_SameChannelAnnoStatus(DAB_APP_ANNO_ON, pst_me_dab_app_inst->u8_SameChannelSubChId, pst_me_dab_app_inst->u8_SameChannelClusterid);
						
						
						/* Send start indication to radio manager*/
						DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
											
						pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
				
						/*	Updating channel name according to frequency */
						DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
						
						/* Send tuned announcement station information to radio manager */
						DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					else
					{
						if(pst_me_dab_app_inst->st_CurrAnnoInfo.u8_SubChId != pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId)
						{
							if((pst_me_dab_app_inst->st_CurrAnnoInfo.e_announcement_type == DAB_TUNER_CTRL_ALARM_ANNO) && (pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type != DAB_TUNER_CTRL_ALARM_ANNO))
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Higher priority Announcement start Notification from Tuner Ctrl ");
								b_Anno_Ongoing_flag = FALSE;
								
								pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF;	
								/* Notify Radio Manager*/
								DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 2. Request DAB_Tuner_Ctrl to stop low priority announcement status: OFF ");
														
								DAB_Tuner_Ctrl_Request_CancelAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_Clusterid); /*This is not user cancel. So we start announcement for ALARM*/					                
							}
						}
					}

				}
				else if((pst_me_dab_app_inst->u8_DAB_APP_announcement_status == DAB_APP_ANNO_STOP) && (b_Anno_Ongoing_flag == TRUE))	
				{
					if(b_AnnoSID_TunedSIDSame == TRUE)
					{				
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Same Channel Announcement stop Notification from Tuner Ctrl ");	
						b_Anno_Ongoing_flag = FALSE;
						b_AnnoSID_TunedSIDSame = FALSE;				
						pst_me_dab_app_inst->u8_SameChannelSubChId = (Tu8) 0x00 ;
						pst_me_dab_app_inst->u8_SameChannelClusterid = (Tu8) 0x00 ;
						memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
						pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF;
						
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Send same channel announcement OFF notification to Tuner Ctrl ");						
						/* Notify DAB_Tuner_Ctrl that samechannel announcement status as OFF */
						DAB_App_Notify_SameChannelAnnoStatus(DAB_APP_ANNO_OFF, pst_me_dab_app_inst->u8_SameChannelSubChId, pst_me_dab_app_inst->u8_SameChannelClusterid);
					
						/* Send stop indication to radio manager*/
						DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
						
						pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
				
						/*	Updating channel name according to frequency */
						DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
				
						/* Send tuned announcement station information to radio manager */
						DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					else
					{
						if((pst_me_dab_app_inst->st_CurrAnnoInfo.e_announcement_type == pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type) && 
						    (pst_me_dab_app_inst->st_CurrAnnoInfo.u8_SubChId == pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId))
						{
							b_Anno_Ongoing_flag = FALSE;
							/* Clear announcement station structure*/
							memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
							pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF;
							
							/* Notify Radio Manager*/
							DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP]1. Request DAB_Tuner_Ctrl to stop announcement status: OFF ");
							/*Send announcement stop request to tuner ctrl*/
							DAB_Tuner_Ctrl_Request_StopAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);						
						}
					}

				}
				else
				{
					/* do nothing */
				}
			}
			else  /*when the announcment occurs in different subchannel other than Tuned SID*/
			{
				if((pst_me_dab_app_inst->u8_DAB_APP_announcement_status == DAB_APP_ANNO_START) && (b_Anno_Onstate == TRUE))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Different subchannel announcement start notification from Tuner Ctrl ");
					/* Send internal message to announcement handler */
					DAB_App_Internal_Msg_To_Anno_State();
				}
				else if((pst_me_dab_app_inst->u8_DAB_APP_announcement_status == DAB_APP_ANNO_STOP) && (b_Anno_Ongoing_flag == TRUE))	
				{
					if((pst_me_dab_app_inst->st_CurrAnnoInfo.e_announcement_type == pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type) && (pst_me_dab_app_inst->st_CurrAnnoInfo.u8_SubChId == pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId))
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Different subchannel announcement stop Notification from Tuner Ctrl ");
						b_Anno_Ongoing_flag = FALSE;
						
						/* Clear announcement station structure*/
						memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
						pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF;
						
						/* Notify Radio Manager*/
						DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 2. Request DAB_Tuner_Ctrl to stop announcement status: OFF ");
						/*Send announcement stop request to tuner ctrl*/
						DAB_Tuner_Ctrl_Request_StopAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);						
					}
				}
				else if((pst_me_dab_app_inst->u8_DAB_APP_announcement_status == DAB_APP_ANNO_START) && (b_Anno_Onstate == FALSE))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Different subchannel announcement start Notification from Tuner Ctrl and Dab_App current state is not announcement state ");
			
					/* Transit to announcement state*/
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_announcement_state);
				}
				else
				{
					/* do nothing */
				}
				
			}
		}
		break;
		
		case DAB_APP_CANCEL_ANNO_REQID:
		{
			if(b_Anno_Ongoing_flag == TRUE)
			{	
				if(b_AnnoSID_TunedSIDSame == TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Same Channel Cancel Announcement request from Radio manager ");
					b_Anno_Ongoing_flag = FALSE;
					b_AnnoSID_TunedSIDSame = FALSE;				
					pst_me_dab_app_inst->u8_SameChannelSubChId = (Tu8) 0x00 ;
					pst_me_dab_app_inst->u8_SameChannelClusterid = (Tu8) 0x00 ;
					memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
					
					/* Clear announcement station structure*/
					memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
						
					pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_USER_CANCEL;
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Send same channel announcement OFF notification to Tuner Ctrl ");
					/* Notify DAB_Tuner_Ctrl that samechannel announcement status as OFF */
					DAB_App_Notify_SameChannelAnnoStatus(DAB_APP_ANNO_OFF, pst_me_dab_app_inst->u8_SameChannelSubChId, pst_me_dab_app_inst->u8_SameChannelClusterid);
						
					/* Send announcement cancel response to Radio Manager*/
					DAB_App_Response_AnnoCancel(pst_me_dab_app_inst->e_Replystatus);
						
					/* Send stop indication to radio manager*/
					DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
					
					pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
					
					/*	Updating channel name according to frequency */
					DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
					
					/* Send tuned announcement station information to radio manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Cancel different channel announcement request from Radio manager ");
					b_cancel_flag = TRUE;
					b_Anno_Ongoing_flag = FALSE;
					pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_USER_CANCEL;
					
					/* Notify Radio Manager*/
					DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 3. Request DAB_Tuner_Ctrl to stop different channel announcement status: OFF ");
										
					DAB_Tuner_Ctrl_Request_CancelAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_Clusterid);
				}
			}
			else
			{
			}
	
		}
		break;
		
		case DAB_APP_ANNO_SIGNAL_LOSS_NOTIFYID:
		{
			if(b_Anno_Ongoing_flag == TRUE)
			{
				if(b_AnnoSID_TunedSIDSame == TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Same Channel Announcement station signal lost ");
					b_Anno_Ongoing_flag = FALSE;
					b_AnnoSID_TunedSIDSame = FALSE;				
					pst_me_dab_app_inst->u8_SameChannelSubChId = (Tu8) 0x00 ;
					pst_me_dab_app_inst->u8_SameChannelClusterid = (Tu8) 0x00 ;
					memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
				
					/* Clear announcement station structure*/
					memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
					
					pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_SIGNAL_LOSS;
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Send same channel announcement OFF notification to Tuner Ctrl ");
					/* Notify DAB_Tuner_Ctrl that samechannel announcement status as OFF */
					DAB_App_Notify_SameChannelAnnoStatus(DAB_APP_ANNO_OFF, pst_me_dab_app_inst->u8_SameChannelSubChId, pst_me_dab_app_inst->u8_SameChannelClusterid);
				
					/* Send stop indication to radio manager*/
					DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
				
					pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
				
					/*	Updating channel name according to frequency */
					DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
						
					/* Send tuned announcement station information to radio manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Different channel announcement station signal lost ");
					b_Anno_Ongoing_flag = FALSE;
					b_cancel_flag = TRUE;
					/* Clear announcement station structure*/
					memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
					pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_SIGNAL_LOSS;
					
					/* Notify Radio Manager*/
					DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 4. Request DAB_Tuner_Ctrl to stop different channel announcement status: OFF signal lost");
										
					/*Send announcement stop request to tuner ctrl*/
					//DAB_Tuner_Ctrl_Request_StopAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);
					DAB_Tuner_Ctrl_Request_CancelAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_Clusterid);
				}
			}
		}
		break;
		
		case DAB_APP_ANNO_STATION_INFO_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Tuned Announcement station Notification from DAB_Tuner_Ctrl ");
			u32_Index = 0 ;	  

			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_CurrentTunedAnnoInfo,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo),&u32_Index);
			
			pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
			
			st_AnnoStationInfo.st_Tunableinfo.u32_Frequency = pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.u32_Frequency;
			st_AnnoStationInfo.st_Tunableinfo.u32_SId = pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.u32_SId;
			st_AnnoStationInfo.st_Tunableinfo.u16_EId = pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.u16_EId;
			st_AnnoStationInfo.st_Tunableinfo.u8_ECC = pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.u8_ECC;
			st_AnnoStationInfo.st_Tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.u16_SCIdI;
			SYS_RADIO_MEMCPY(&(st_AnnoStationInfo.st_EnsembleLabel),&(pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.Ensemble_label),sizeof(Ts_Tuner_Ctrl_Label)); 
			SYS_RADIO_MEMCPY(&(st_AnnoStationInfo.st_ServiceLabel),&(pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.service_label),sizeof(Ts_Tuner_Ctrl_Label)); 
			SYS_RADIO_MEMCPY(&(st_AnnoStationInfo.st_ComponentLabel),&(pst_me_dab_app_inst->st_CurrentTunedAnnoInfo.servicecomponent_label),sizeof(Ts_Tuner_Ctrl_Label));
			
			/*	Updating channel name according to frequency */
			DAB_App_UpdateChannelName(st_AnnoStationInfo.st_Tunableinfo.u32_Frequency, st_AnnoStationInfo.au8_ChannelName);
			
			/* Send tuned announcement station information to radio manager */
			DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,st_AnnoStationInfo);
			 
		}
		break;
		
		case DAB_APP_PLAY_SEL_STN_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Play select response ");
			u32_Index = 0;	/* Initialize station list index */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
			/*	Updating channel name according to frequency */
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			
			if(b_Tuneup_down == TRUE)
				DAB_App_Response_TuneUpDown(REPLYSTATUS_SUCCESS,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
			else
				DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
		}
		break;
		
		case DAB_APP_AF_LIST_RESID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] AF-list response ");
			/* Extracting AF list from Tuner Ctrl msg */ 
			ExtractParameterFromMessage(&pst_me_dab_app_inst->st_DAB_App_AFList,(Tchar *)(pst_msg->data),(Tu8)sizeof(Ts_DAB_App_AFList),&u32_Index);
			
			/* Sending AF list Response to Radio Manager */
			DAB_App_Response_AF_List(pst_me_dab_app_inst->st_DAB_App_AFList);

		}
		break;
		
		case DAB_APP_CANCEL_REQID:
		{
			u32_Index = 0;

			/* Extracting Seek Cancel request status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_CancelType,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_CancelType),&u32_Index);
			
			if(pst_me_dab_app_inst->e_CancelType == DAB_APP_SEEK_CANCEL)
			{
				pst_me_dab_app_inst->e_CancelType = DAB_APP_TUNE_CANCEL;
				DAB_Tuner_Ctrl_Request_Cancel((Te_DAB_Tuner_Ctrl_CancelType)(pst_me_dab_app_inst->e_CancelType));
			  	pst_me_dab_app_inst->b_SeekStarted = FALSE;
			}
			else
			{
				/* Sending Cancel Scan request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_Cancel((Te_DAB_Tuner_Ctrl_CancelType)(pst_me_dab_app_inst->e_CancelType));
			}

		}
		break;
		
		case DAB_APP_CANCEL_RESID:
		{
			u32_Index = 0;

			/* Extracting Scan Cancel request status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			if(pst_me_dab_app_inst->e_CancelType == DAB_APP_AF_TUNE_CANCEL)
			{
				b_AFtune_Request = FALSE ;
			}

			pst_me_dab_app_inst->e_CancelType =	DAB_APP_CANCEL_INVALID;
			/* Sending Scan Cancel response to Radio Manager */
			DAB_App_Response_Cancel(pst_me_dab_app_inst->e_Replystatus);

			/* Transit to Idle state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveIdleAnnouncementHndlr             */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveIdleAnnouncementHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveIdleAnnouncementHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveIdleAnnouncementHndlr \n";
			
			
			b_Anno_Onstate = TRUE;
			
			if(b_Anno_Ongoing_flag == FALSE)
			{
				/* Store extracted current announcement information in structure st_Tuned_Anno_Info */
				SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_Tuned_Anno_Info), &(pst_me_dab_app_inst->st_CurrAnnoInfo),sizeof(Ts_DAB_APP_Curr_Anno_Info));
			
				b_Anno_Ongoing_flag = TRUE;
				pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_ON;
				/* Notify Radio Manager*/
				DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 1. Sending start announcement request to Tuner Ctrl ");
				/* Send announcement switch request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_StartAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);
			
			}
			else if((b_Anno_Ongoing_flag == TRUE) && (b_AnnoSID_TunedSIDSame == TRUE))
			{
				if((pst_me_dab_app_inst->st_CurrAnnoInfo.e_announcement_type == DAB_TUNER_CTRL_ALARM_ANNO) && (pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type != DAB_TUNER_CTRL_ALARM_ANNO))
				{
					/* Store extracted current announcement information in structure st_Tuned_Anno_Info */
					SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_Tuned_Anno_Info), &(pst_me_dab_app_inst->st_CurrAnnoInfo),sizeof(Ts_DAB_APP_Curr_Anno_Info));
					
					b_AnnoSID_TunedSIDSame = FALSE;
					
					pst_me_dab_app_inst->u8_SameChannelSubChId = (Tu8) 0x00 ;
					pst_me_dab_app_inst->u8_SameChannelClusterid = (Tu8) 0x00 ;
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Notify DAB_Tuner_Ctrl that samechannel announcement status as OFF ");
					/* Notify DAB_Tuner_Ctrl that samechannel announcement status as OFF */
					DAB_App_Notify_SameChannelAnnoStatus(DAB_APP_ANNO_OFF, pst_me_dab_app_inst->u8_SameChannelSubChId, pst_me_dab_app_inst->u8_SameChannelClusterid);
									
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 2. Notify DAB_Tuner_Ctrl to start alarm announcement status as ON ");
					/* Send announcement switch request to Tuner Ctrl */
					DAB_Tuner_Ctrl_Request_StartAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);					
				}
				else
				{
					/* Do nothing */
				}
			}
			else
			{
				/* Do nothing */
			}
			
		}
		break;
		
		case DAB_APP_INTERNAL_ANNO_MSG:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Internal msg received to start different channel announcement ");
			if(b_Anno_Ongoing_flag == TRUE)
			{
				if((pst_me_dab_app_inst->st_CurrAnnoInfo.e_announcement_type == DAB_TUNER_CTRL_ALARM_ANNO) && (pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type != DAB_TUNER_CTRL_ALARM_ANNO))
				{
					if(b_AnnoSID_TunedSIDSame == TRUE)
					{
						/* Store extracted current announcement information in structure st_Tuned_Anno_Info */
						SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_Tuned_Anno_Info), &(pst_me_dab_app_inst->st_CurrAnnoInfo),sizeof(Ts_DAB_APP_Curr_Anno_Info));
					
						b_AnnoSID_TunedSIDSame = FALSE;
						pst_me_dab_app_inst->u8_SameChannelSubChId = (Tu8) 0x00 ;
						pst_me_dab_app_inst->u8_SameChannelClusterid = (Tu8) 0x00 ;
					
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] Send same channel announcement OFF notification to Tuner Ctrl ");
						/* Notify DAB_Tuner_Ctrl that samechannel announcement status as OFF */
						DAB_App_Notify_SameChannelAnnoStatus(DAB_APP_ANNO_OFF, pst_me_dab_app_inst->u8_SameChannelSubChId, pst_me_dab_app_inst->u8_SameChannelClusterid);
					
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 3. Notify DAB_Tuner_Ctrl to start alarm announcement status as ON ");
						/* Send announcement switch request to Tuner Ctrl */
						DAB_Tuner_Ctrl_Request_StartAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);	
						
					}
					else
					{
						b_Anno_Ongoing_flag = FALSE;
						
						pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF;
						/* Notify Radio Manager*/
						DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
				
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 5. cancel different channel announcement request to Dab_tuner_Ctrl ");
						DAB_Tuner_Ctrl_Request_CancelAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_Clusterid);
					}
				}
				else
				{
					/* Ignore alarm announcement when already alarm announcement is playing */
				}

			}	
			else
			{
				/* Store extracted current announcement information in structure st_Tuned_Anno_Info */
				SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_Tuned_Anno_Info), &(pst_me_dab_app_inst->st_CurrAnnoInfo),sizeof(Ts_DAB_APP_Curr_Anno_Info));
				b_Anno_Ongoing_flag = TRUE;	
				pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_ON;
				/* Notify Radio Manager*/
				DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
								
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 4. Notify DAB_Tuner_Ctrl to start announcement status: ON ");
				/* Send announcement switch request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_StartAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);
				
			}
		}
		break;	
		
		case DAB_APP_CANCEL_ANNO_RESID:
		{
			u32_Index = 0 ;	

			/* Extracting announcement status information from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_ReplyStatus),&u32_Index);
					
			//if((Anno_Onstate == E_TRUE) && (pst_me_dab_app_inst->e_DAB_APP_announcement_status == DAB_APP_ANNO_CANCEL_SUCCESS))
			if(pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{
				//Anno_Ongoing_flag = FALSE;
				
				if((b_cancel_flag == TRUE) && (pst_me_dab_app_inst->e_DAB_App_AnnoIndication == DAB_APP_ANNO_OFF_ANNO_SETTINGS_OFF))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Announcement cancelled as Settings is turned OFF ");
					b_cancel_flag = FALSE;
					
					/* Clear structure*/
					memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
					/* Clear announcement station structure*/
					memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
				
					//pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_ANNO_SETTINGS_OFF;
				
					/* Notify Radio Manager*/
					//DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
					
					pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
							
					/*	Updating channel name according to frequency */
					DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
					
					/* Send tuned announcement station information to radio manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
				
				}
				else if((b_cancel_flag == TRUE) && (pst_me_dab_app_inst->e_DAB_App_AnnoIndication == DAB_APP_ANNO_OFF_USER_CANCEL))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] User cancelled announcement ");
					b_cancel_flag = FALSE;
					
					/* Send announcement cancel response to Radio Manager*/
					DAB_App_Response_AnnoCancel(pst_me_dab_app_inst->e_Replystatus);
					
					/* Clear structure*/
					memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
					/* Clear announcement station structure*/
					memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
				
					//pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF_ANNO_SETTINGS_OFF;
				
					/* Notify Radio Manager*/
					//DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
					
					pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
									
					/*	Updating channel name according to frequency */
					DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
						
					/* Send tuned announcement station information to radio manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
				
				}
				else if((b_cancel_flag == TRUE) && (pst_me_dab_app_inst->e_DAB_App_AnnoIndication == DAB_APP_ANNO_OFF_SIGNAL_LOSS))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Announcement cancelled as signal degraded ");
					b_cancel_flag = FALSE;
					
					/* Clear structure*/
					memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
					/* Clear announcement station structure*/
					memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
									
					/* Notify Radio Manager*/
					//DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
					
					pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;		

					/*	Updating channel name according to frequency */
					DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
		
					/* Send tuned announcement station information to radio manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Priority Announcement- Alarm start request to DAB_Tuner_Ctrl ");
					/* Store extracted current announcement information in structure st_Tuned_Anno_Info */
					SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_Tuned_Anno_Info), &(pst_me_dab_app_inst->st_CurrAnnoInfo),sizeof(Ts_DAB_APP_Curr_Anno_Info));
				    b_Anno_Ongoing_flag = TRUE;
					pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_ON;
					/* Notify Radio Manager*/
					DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE," [RADIO][DAB_APP] 5. Notify DAB_Tuner_Ctrl to start announcement status: ON ");
					/* Send announcement switch request to Tuner Ctrl */
					DAB_Tuner_Ctrl_Request_StartAnnouncement(pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type,pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId);
					
				}
				
			}
		}
		break;	
		
		case DAB_APP_START_ANNO_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG,"[RADIO][DAB_APP] Received start announcement response from Tuner Ctrl ");
			
			u32_Index = 0 ;	

			/* Extracting Announcement reply status from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_ReplyStatus),&u32_Index);	
			
			/* Extracting announcement type information from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_announcement_type,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&u32_Index);
			
			if(pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{
				/*pst_me_dab_app_inst->e_Cur_Anno_type =	pst_me_dab_app_inst->e_announcement_type;
				pst_me_dab_app_inst->e_New_Anno_type = DAB_APP_ANNO_TYPE_INVALID;*/
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Announcement started response from DAB_Tuner_Ctrl ");			
				//pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_ON;
				
				/* Notify Radio Manager*/
				//DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
			}
			else
			{
				b_Anno_Ongoing_flag = FALSE;
				
				pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF;
				/* Notify Radio Manager*/
				DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
			}

			/* Send announcement status notification to Radio Manager */
			//DAB_App_Notify_AnnouncementStatus(pst_me_dab_app_inst->u8_DAB_APP_announcement_status,pst_me_dab_app_inst->e_announcement_type);	
				
		}
		break;
		
		case DAB_APP_STOP_ANNO_RESID:
		{
			u32_Index = 0 ;	  

			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_announcement_type,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_DAB_Tuner_Ctrl_announcement_type),&u32_Index);	
			
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u8_SubChId,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Tu8),&u32_Index);
		
			//if(Anno_Onstate == E_TRUE)
			//{
			
			if((pst_me_dab_app_inst->st_Tuned_Anno_Info.e_announcement_type == pst_me_dab_app_inst->e_announcement_type) && (pst_me_dab_app_inst->st_Tuned_Anno_Info.u8_SubChId == pst_me_dab_app_inst->u8_SubChId))
			{
				pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG," [RADIO][DAB_APP] Announcement stopped response from DAB_Tuner_Ctrl ");
			//	pst_me_dab_app_inst->e_Cur_Anno_type = DAB_APP_ANNO_TYPE_INVALID;
				
				/* Clear structure*/
				memset(&(pst_me_dab_app_inst->st_Tuned_Anno_Info),0,sizeof(Ts_DAB_APP_Curr_Anno_Info));
				
				/* Clear announcement station structure*/
				memset(&(st_AnnoStationInfo),0,sizeof(Ts_DAB_App_CurrentStationInfo));
				//pst_me_dab_app_inst->e_DAB_App_AnnoIndication = DAB_APP_ANNO_OFF;
				
				/* Notify Radio Manager*/
				//DAB_App_Notify_AnnoIndication(pst_me_dab_app_inst->e_DAB_App_AnnoIndication);
				
				pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_SUCCESS;		

				/*	Updating channel name according to frequency */
				DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
				
				/* Send tuned announcement station information to radio manager */
				DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
														
				//Send stop announcement notification to Radio Manager 
//				DAB_App_Notify_AnnouncementStatus(pst_me_dab_app_inst->e_DAB_App_StopAnnoReplyStatus,pst_me_dab_app_inst->e_announcement_type);
							
				/* Send Stop announcement response to Tuner Ctrl */
				//DAB_App_Response_StopAnnouncement(pst_me_dab_app_inst->e_DAB_APP_announcement_status,pst_me_dab_app_inst->e_Cur_Anno_type);
			}
			//}
			
		}
		break;
				
		case HSM_MSGID_EXIT:
		{
			b_Anno_Onstate = FALSE;
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveBusyPlaySelStnHndlr                           */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveBusyPlaySelStnHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	 u32_Index = 0;	/* Initialize station list index */
	 u16_DAB_App_SCIdI = 0 ;		/* Initialize SCIdI variable */
	 u32_DAB_App_SId = 0 ;	/* Initialize SId variable */
	 

  	 PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
				
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveBusyPlaySelStnHndlr ");
			 u32_Frequency = 0;	/* Initialize frequency variable */
			
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveBusyPlaySelStnHndlr \n";

			u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency ;
		
			/* Checking the frequency if present in the specified Band III range */
			if((u32_Frequency >= DAB_APP_5A_EU) && (u32_Frequency <= DAB_APP_13F_EU)) 
			{
				if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_MANUAL_TUNEBY_CHNAME)
				{
					u16_EId = 0;	/* Initialize EId variable */
					u32_DAB_App_SId = 0; 	/* Initialize SId variable */
					u16_DAB_App_SCIdI = 0;	/* Initialize SCIdI variable */
				}
				else
				{
					u16_EId = 0;	/* Initialize EId variable */
					u32_DAB_App_SId = 0; 	/* Initialize SId variable */
					u16_DAB_App_SCIdI = 0;	/* Initialize SCIdI variable */
			
					/* Retrieving EId,SId and SCIdI information from station list */
					u16_EId = pst_me_dab_app_inst->st_tunableinfo.u16_EId ; 
					u32_DAB_App_SId = pst_me_dab_app_inst->st_tunableinfo.u32_SId ;
					u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI ; 
				}
				
				/* Sending play select station request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI);

			}
			else
			{
				if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_MANUAL_TUNEBY_CHNAME)
				{
					/* Invalid frequency */
					
					pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;

					pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_FAILURE;
					/* Sending Manual Tune by Channel name response to Radio Manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
					
				}
				else if(pst_me_dab_app_inst->st_tunableinfo.u32_SId != (Tu32) 0)
				{
					b_AFtune_Request = FALSE;
					u32_DAB_App_SId = pst_me_dab_app_inst->st_tunableinfo.u32_SId;
					u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI;
					DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI);
				}
				else if(pst_me_dab_app_inst->st_tunableinfo.u32_SId == (Tu32) 0)
				{
					/*	Updating channel name according to frequency */
					DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
					
					pst_me_dab_app_inst->e_Replystatus = REPLYSTATUS_NO_SIGNAL;
					/* Send AF Tune response to Radio Manager */
					DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					b_AFtune_Request = FALSE;
					/* Transit to active Idle state*/
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
				}
				else
				{/* If the freuency is not in the specified Band III range then report Radio Manager */
					DAB_App_Response_PlaySelectSt(REPLYSTATUS_INVALID_PARAM,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);

				/* Transit to active Idle state*/
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
				}	
			}
		}
		break;
		case DAB_APP_STL_UPDATE_NOTIFYID:
		{
			 e_CreateSTLStatus = REPLYSTATUS_SUCCESS;
			 u32_Index = 0 ;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Station list update notification ");
			/* Extracting Tuner Ctrl STL updated notify parameter from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_SharedMemoryType,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_SharedMemoryType),&u32_Index);
			//DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_DAB_App_RequestCmd,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_DAB_App_RequestCmd),&u32_Index);
	
			if(pst_me_dab_app_inst->e_SharedMemoryType == DAB_TUNER_APP)
			{
				/* Create Station list */
				e_CreateSTLStatus = DAB_APP_CreateStationList(pst_me_dab_app_inst->e_DAB_App_RequestCmd,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency);
		
				if(e_CreateSTLStatus == REPLYSTATUS_SUCCESS)
				{
					/* Station list Sort */
					DAB_APP_EnsembleSortSTL();
					DAB_APP_SortSTL();
			
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
				}
				else
				{
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
			
				}
				//HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
			}
			else
			{
		
			}/* For misra c */
		}
		break;

		case DAB_APP_DAB_AF_LEARN_MEM_NOTIFYID:
		{
			u32_Index = 0;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] AF TUNE RESPONSE ");
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
			b_AFtune_Request = FALSE;
			if(pst_me_dab_app_inst->e_DAB_App_StationNotAvailStrategyStatus	== DAB_APP_STATIONNOTAVAIL_STRATEGY_START && pst_me_dab_app_inst->e_Replystatus != REPLYSTATUS_SUCCESS)
			{
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_busy_scan_state);
			}
			else
			{
				HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_listened_state);
			}
		}
		break;
		
			
		case DAB_APP_PLAY_SEL_STN_RESID:
		{
			u32_Index = 0;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Play select response ");
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
						
			/*	Updating channel name according to frequency */
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			
			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_MANUAL_TUNEBY_CHNAME)
					{
						pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;
						
						/* Sending Manual Tune by Channel name response to Radio Manager */
						DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}				
					else if(pst_me_dab_app_inst->e_TuneRequest == DAB_APP_REQUEST_TUNE_UP_DOWN)
					{
						DAB_App_Response_TuneUpDown(REPLYSTATUS_SUCCESS,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					else
					{
											
						if(b_AFtune_Request == TRUE)
						{
							DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
							b_AFtune_Request = FALSE;
						}
						else	
							/* Sending Play Select Station response to Radio Manager */
							DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					pst_me_dab_app_inst->e_TuneRequest = DAB_APP_REQUEST_TUNE_INVALID;
					
					/* Transit to active Idle listen state*/
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_listened_state);
				}
				break;

				case REPLYSTATUS_NO_SIGNAL: 
				{
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					
					if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_MANUAL_TUNEBY_CHNAME)
					{
						pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;
						
						/* Sending Manual Tune by Channel name response to Radio Manager */
						DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}										
					else if(pst_me_dab_app_inst->e_TuneRequest == DAB_APP_REQUEST_TUNE_UP_DOWN)
					{
						DAB_App_Response_TuneUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					else
					{
						if(b_AFtune_Request == TRUE)
						{
							DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
							b_AFtune_Request = FALSE;
						}
						else
						{
							/* Sending Play Select Station response to Radio Manager */
							DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
						}
					}
					pst_me_dab_app_inst->e_TuneRequest = DAB_APP_REQUEST_TUNE_INVALID;
					
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);

				}
				break;

				case REPLYSTATUS_REQ_TIMEOUT:
				{

				}
				break;

				case REPLYSTATUS_FAILURE:
				{
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_MANUAL_TUNEBY_CHNAME)
					{
						//pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;
						
						/* Sending Manual Tune by Channel name response to Radio Manager */
						DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}										
					else if(pst_me_dab_app_inst->e_TuneRequest == DAB_APP_REQUEST_TUNE_UP_DOWN)
					{
						DAB_App_Response_TuneUpDown(REPLYSTATUS_FAILURE,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					else
					{
						if(b_AFtune_Request == TRUE)
						{
							DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
							b_AFtune_Request = FALSE;
						}
						else
						{
							/* Sending Play Select Station response to Radio Manager */
							DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
						}
					}
					pst_me_dab_app_inst->e_TuneRequest = DAB_APP_REQUEST_TUNE_INVALID;
					
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);

				}
				break;

				case REPLYSTATUS_SERVICE_NOT_AVAILABLE:
				{
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_MANUAL_TUNEBY_CHNAME)
					{
						//pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;
						
						/* Sending Manual Tune by Channel name response to Radio Manager */
						DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}										
					else if(pst_me_dab_app_inst->e_TuneRequest == DAB_APP_REQUEST_TUNE_UP_DOWN)
					{
						DAB_App_Response_TuneUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					else
					{
						if(b_AFtune_Request == TRUE)
						{
							DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
							b_AFtune_Request = FALSE;
						}
						else
						/* Sending Play Select Station response to Radio Manager */
						DAB_App_Response_PlaySelectSt(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					pst_me_dab_app_inst->e_TuneRequest = DAB_APP_REQUEST_TUNE_INVALID;
					
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);

				}
				break;

				case REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE:
				{
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_MANUAL_TUNEBY_CHNAME)
					{
						//pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;
						
						/* Sending Manual Tune by Channel name response to Radio Manager */
						DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}										
					else if(pst_me_dab_app_inst->e_TuneRequest == DAB_APP_REQUEST_TUNE_UP_DOWN)
					{
						DAB_App_Response_TuneUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					}
					else
					{
						if(b_AFtune_Request == TRUE)
						{
							DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
							b_AFtune_Request = FALSE;
						}
						else/* Sending Play Select Station response to Radio Manager */
						{
							DAB_App_Response_PlaySelectSt(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
						}
					}
					pst_me_dab_app_inst->e_TuneRequest = DAB_APP_REQUEST_TUNE_INVALID;
					
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);

				}
				break;

				case REPLYSTATUS_INVALID_PARAM:
				{
					
				}
				break;
				case REPLYSTATUS_BLENDING_NO_SIGNAL:
				{
					/* Sending Play Select Station response to Radio Manager */
					DAB_App_Response_PlaySelectSt(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
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
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
 	
	}
	return ret;	
}

/*================================================================================================================================*/
/* Ts_Sys_Msg* DAB_APP_INST_HSM_ActiveBusysSerCompSeekHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg) */
/*================================================================================================================================*/

Ts_Sys_Msg* DAB_APP_INST_HSM_ActiveBusysSerCompSeekHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
{
	Ts_Sys_Msg* ret = NULL; /* mark the message as handled */
	Tu8 u8_ChannelSync;
	
  	PRINT_MSG_DATA(pst_msg);
	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
				
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveBusysSerCompSeekHndlr ");
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveBusysSerCompSeekHndlr \n";
			if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_SER_COMP_SEEK)
			{
				pst_me_dab_app_inst->b_SeekStarted = TRUE;
				/* If no services are available in current ensemble, search for next frequency and send tune request to DAB Tuner Ctrl */
				if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices == 0)
				{
					u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
					u16_EId = 0;	/* Initialize EId variable */
				    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
				    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
					DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
				    
				}
				/* If services are available in current ensemble, search for next/prev SId/SCId to tune next and send Select service request to DAB Tuner Ctrl */	
				else
				{
					SearchSeekInfo(pst_me_dab_app_inst);		
				}
			}
		}
		break;

		case DAB_APP_FREQ_CHANGE_NOTIFYID:
		{
			u32_Index = 0;	/* Initialize station list index */

			/* Extracting Tuner Ctrl frequency change response status from msg */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u32_Frequency, (Tchar *)(pst_msg->data), (Tu8) sizeof(Tu32), &u32_Index);


			DAB_App_UpdateChannelName(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency, pst_me_dab_app_inst->au8_ChannelName);

			/* Sending frequency change notification to Radio Manager */
			DAB_App_Notify_FrequencyChange(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency, pst_me_dab_app_inst->au8_ChannelName);
		}
		break;

		case  DAB_APP_PLAY_SEL_STN_RESID:
		{
			u32_Index = 0;	/* Initialize station list index */
			
			/*	Updating seek reply status */	
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			
			/* Updating seek current staion info */		
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8)sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);	
						
		   pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID;			
			/*	Updating channel name according to frequency */
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			
			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					/* Sending Seek response to Radio Manager */
					DAB_App_Response_ServiceCompSeekUpDown(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					
					/* Transit to active Idle listen state*/
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_listened_state);
				}
				break;
					
				case REPLYSTATUS_NO_SIGNAL: 
				{
					pst_me_dab_app_inst->b_SeekStarted = FALSE;
					if(pst_me_dab_app_inst->u32_SeekFrequency == pst_me_dab_app_inst->st_tunableinfo.u32_Frequency)
					{
						DAB_App_Response_ServiceCompSeekUpDown(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);	
						HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
					}
					else if(pst_me_dab_app_inst->e_CancelType != DAB_APP_TUNE_CANCEL)
					{
						pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices = 0;
						/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
						u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
						u16_EId = 0;	/* Initialize EId variable */
					    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
					    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
						DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
					}
					else
					{
						
					}
				}
				break;
				
				case REPLYSTATUS_SERVICE_NOT_AVAILABLE:
				{
					/* Find the next/prev SId to tune next and send request to DAB Tuner Ctrl */
					SearchServiceSeekInfo(pst_me_dab_app_inst);
				}
				break;
				
				case REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE:
				{
					/* Find the next/prev SCId to tune next and send request to DAB Tuner Ctrl */
					SearchServiceCompSeekInfo(pst_me_dab_app_inst);
					
				}
				break;
				
			    case REPLYSTATUS_FAILURE:
				{   
					pst_me_dab_app_inst->b_SeekStarted = FALSE;
					if(pst_me_dab_app_inst->u32_SeekFrequency == pst_me_dab_app_inst->st_tunableinfo.u32_Frequency)
					{
						DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);	
						HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
					}
					else
					{
						pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices = 0;
						/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
						u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
						u16_EId = 0;	/* Initialize EId variable */
					    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
					    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
						DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
					}
				}
				break;
				case REPLYSTATUS_BLENDING_NO_SIGNAL:
				{
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel.u8_CharSet = DAB_APP_CHARSET_UTF8 ;
					/* Sending Seek response to Radio Manager */
					DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);	
					
				}
				break;
		
				default:
				{
				
				}
				break;
			}
		
		}
		break;
		
		case DAB_APP_COMP_LIST_SORT_REQID:
		{
			/* Sort the list and copy it to the st_DAB_App_CurrEnsembleProgList to use for seek */
			SYS_MUTEX_LOCK(STL_DAB_APP_DAB_TC);
	     	Dab_App_Sort_CurrEnsembleProgList(&st_Dab_Tuner_Ctrl_CurrEnsembleProgList);
		 	SYS_RADIO_MEMCPY(&(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList),&(st_Dab_Tuner_Ctrl_CurrEnsembleProgList),sizeof(st_Dab_Tuner_Ctrl_CurrEnsembleProgList));
			SYS_MUTEX_UNLOCK(STL_DAB_APP_DAB_TC);
			/* Send sort response to DAB Tuner Ctrl */
			DAB_App_Response_Sort();
		}
		break ;
		
		case DAB_APP_STL_UPDATE_NOTIFYID:
		{
			
			e_CreateSTLStatus = REPLYSTATUS_SUCCESS;
			u32_Index = 0 ;	/* Initialize station list index */
			/* Extracting Tuner Ctrl STL updated notify parameter from msg */ 
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Station list update notification ");
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_SharedMemoryType,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_SharedMemoryType),&u32_Index);
			
			if(pst_me_dab_app_inst->e_SharedMemoryType == DAB_TUNER_APP)
			{
				/* Create Station list */
				e_CreateSTLStatus = DAB_APP_CreateStationList(pst_me_dab_app_inst->e_DAB_App_RequestCmd,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency);

				if(e_CreateSTLStatus == REPLYSTATUS_SUCCESS)
				{
					/* Station list Sort */
					DAB_APP_EnsembleSortSTL();
					DAB_APP_SortSTL();
					
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
				}
				else
				{
					/* Sending STL update notification to Radio Manager */
					DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);

				}
			}
			else
			{

			}/* For misra c */
		}
		break;
		case DAB_APP_GET_COMP_LIST_STATUS_NOTIFYID:
		{
			u32_Index = 0;
			/* Extract frequency, EId and replystatus info sent by DAB Tuner Ctrl */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u32_Frequency,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu32),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_tunableinfo.u16_EId,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(const Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			if(pst_me_dab_app_inst->e_Replystatus == REPLYSTATUS_SUCCESS)
			{
				if(pst_me_dab_app_inst->e_SeekDirection == RADIO_FRMWK_DIRECTION_UP)
				{
					pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = 0;
					pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0;
				}
				else
				{
					pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices - 1 ;
					pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents - 1;
				}
				
				/* Update the seekinfo structure */
				UpdateSeekInfo(pst_me_dab_app_inst);
				pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
				pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
			
				/* Select the first SId and SCId to tune and send select service request to DAB Tuner Ctrl */
			    u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency;
				u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
			    u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
			    u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI ;	/* Initialize SCIdI variable */
			    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI);
			}
			
		}
		break;
		
		case DAB_APP_SYNCHRONISATION_NOTIFYID:
		{   
			DAB_App_ExtractParameterFromMessage(&u8_ChannelSync,(const Tchar *)(pst_msg->data), sizeof(Tu8),&u32_Index);
			if(pst_me_dab_app_inst->e_DAB_App_RequestCmd == DAB_APP_SER_COMP_SEEK)
			{
				if(u8_ChannelSync == 0)
				{
					if(pst_me_dab_app_inst->u32_SeekFrequency == pst_me_dab_app_inst->st_tunableinfo.u32_Frequency)
					{
						if(pst_me_dab_app_inst->b_SeekStarted == TRUE)
						{
							pst_me_dab_app_inst->b_SeekStarted = FALSE;
							pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices = 0;
							u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
							u16_EId = 0;	/* Initialize EId variable */
						    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
						    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
							DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
						}
						else
						{
							/* Clear service label in "s_DAB_App_CurrentStationInfo" structure */
							memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

							/* Clear Component label in "s_DAB_App_CurrentStationInfo" structure */
							memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

							/* Sending Seek response to DAB application with No Signal */
							DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);

							/* Clearing the e_DAB_App_RequestCmd */
							pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID ;	
							
							/* Clear stationlist as no stations are available*/
							ensembleIndex = 0;
							Services = 0;
							Components = 0;
							memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
							memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
							memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
							DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
							HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
					   	}
					}
					else
					{
						pst_me_dab_app_inst->b_SeekStarted = FALSE;
						pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices = 0;
						/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
						u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
						u16_EId = 0;	/* Initialize EId variable */
					    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
					    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
						DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
					}
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
			/* in active idle state throw system error */
			ret = pst_msg;
		}
		break;
 	
	}
	return ret;	
}


/*===========================================================================*/
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_Active_BGScanHndlr                           */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_Active_BGScanHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_Active_BGScanHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_Active_BGScanHndlr \n";
			pst_me_dab_app_inst->u8_Handler_Check = 2;
			/* Sending DAB Deactivate request to Tuner Ctrl */
			DAB_Tuner_Ctrl_Request_DeActivate();

#ifdef PC_TEST
			DAB_Tuner_Ctrl_Response_DeActivate(REPLYSTATUS_SUCCESS);
#endif

		}
		break;
		
		case DAB_APP_INIT_FMDAB_LINKING_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Initate FMDAB linking request ");
			/* Sending Initate FMDAB linking request to Tuner Ctrl */
			DAB_Tuner_Ctrl_Notify_Init_FMDAB_linking();
		}
		break;

		case DAB_APP_DEACTIVATE_RESID:
		{
			u32_Index = 0 ;

			/* Extracting Tuner Ctrl De-Activate response parameter from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					/* Sending DAB deselect response to Radio Manager */
					DAB_App_Response_DeSelectBand(pst_me_dab_app_inst->e_Replystatus);

				}
				break;

				case REPLYSTATUS_REQ_CANCELLED: 
				{
					
				}
				break;

				case REPLYSTATUS_REQ_ERROR:
				{

				}
				break;

				default:
				{

				}
				break;
			}
		}
		break;
			
		case DAB_APP_SELECT_DAB_REQID:
		{	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Select band request ");
			/* Sending DAB activate request to Tuner Ctrl */
			DAB_Tuner_Ctrl_Request_Activate();

			/* Transit to instance active idle state*/
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);

		}
		break;
		
		case DAB_APP_SCAN_RESID:
		{
			 u32_Index = 0 ;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Scan response ");
			/* Extracting Tuner Ctrl scan response status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					/* Sending GETSTL success response to Radio Manager */
					DAB_App_Response_GetStationList(pst_me_dab_app_inst->e_Replystatus);
				}
				break;

				case REPLYSTATUS_INVALID_PARAM: 
				{

				}
				break;

				case REPLYSTATUS_SYSTEMISCURRENTLYSEARCHING:
				{

				}
				break;

				case REPLYSTATUS_REQ_TIMEOUT:
				{

				}
				break;

				case REPLYSTATUS_NO_SIGNAL:
				{
					DAB_App_Response_GetStationList(pst_me_dab_app_inst->e_Replystatus);
				}
				break;

				default:
				{

				}
				break;
			}
		}	
		break;
		case DAB_APP_STL_UPDATE_NOTIFYID:
		{
			
				e_CreateSTLStatus = REPLYSTATUS_SUCCESS;
				u32_Index = 0 ;	/* Initialize station list index */
				/* Extracting Tuner Ctrl STL updated notify parameter from msg */ 
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Station list update notification ");
				DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_SharedMemoryType,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_SharedMemoryType),&u32_Index);
				
				if(pst_me_dab_app_inst->e_SharedMemoryType == DAB_TUNER_APP)
				{
					/* Create Station list */
					e_CreateSTLStatus = DAB_APP_CreateStationList(pst_me_dab_app_inst->e_DAB_App_RequestCmd,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency);

					if(e_CreateSTLStatus == REPLYSTATUS_SUCCESS)
					{
						/* Station list Sort */
						DAB_APP_EnsembleSortSTL();
						DAB_APP_SortSTL();
						
						/* Sending STL update notification to Radio Manager */
						DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
					}
					else
					{
						/* Sending STL update notification to Radio Manager */
						DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);

					}
					
				//	HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
				}
				else
				{

				}/* For misra c */
			}
		break;
		case DAB_APP_FM_DAB_STOP_LINKING_NOTIFYID:
		{
			 u32_Index = 0;	/* Initialize index */
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] FM-DAB linking stop  ");
			 DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_FmtoDAB_Reqstatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_Dab_App_FmtoDAB_Reqstatus),&u32_Index);
			 DAB_Tuner_Ctrl_Notify_FM_DAB_LinkingStop((Te_FmtoDAB_Reqstatus)(pst_me_dab_app_inst->e_FmtoDAB_Reqstatus));
		}
		break;
		
		case DAB_APP_FM_DAB_LINKING_PI:
		{
			 u32_Index = 0;	/* Initialize index */
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] FM-DAB linking PI ");
			 DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u16_FM_DAB_SID,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			 DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u16_FM_DAB_SCID,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			 DAB_Tuner_Ctrl_Request_GetSIDStation(pst_me_dab_app_inst->u16_FM_DAB_SID,pst_me_dab_app_inst->u16_FM_DAB_SCID);
		}
		break;
		
		case DAB_APP_AF_TUNE_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] AF-Tune request");
     		u32_Index = 0 ;	/* Initialize index */
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u16_FM_DAB_SID,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u16_FM_DAB_SCID,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu16),&u32_Index);
			DAB_Tuner_Ctrl_Request_GetSIDStation(pst_me_dab_app_inst->u16_FM_DAB_SID,pst_me_dab_app_inst->u16_FM_DAB_SCID);
		}
		break;
		case DAB_APP_PLAY_SEL_STN_RESID:
		{
			u32_Index = 0;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Play select response ");
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
						
			/*	Updating channel name according to frequency */
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			DAB_App_Notify_FMtoDAB_linked_Station(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
		}
		break;
			
	/*	case DAB_APP_FM_DAB_LINKING_STATION_NOTIFYID:
		{
			 u32_Index = 0;	 Initialize index 
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] FM-DAB linking station notification ");
			 DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_FM_DAB_linking_station,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_APP_fmdab_linkinfo),&u32_Index);
			 pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = pst_me_dab_app_inst->st_FM_DAB_linking_station.u32_Frequency;
			 pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_FM_DAB_linking_station.u32_SId;
			 pst_me_dab_app_inst->st_tunableinfo.u16_EId = pst_me_dab_app_inst->st_FM_DAB_linking_station.u16_EId;
			 pst_me_dab_app_inst->st_tunableinfo.u8_ECC = pst_me_dab_app_inst->st_FM_DAB_linking_station.u8_ECC;
			 pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_FM_DAB_linking_station.u16_SCIdI;
			 DAB_App_Notify_FMtoDAB_linked_Station(pst_me_dab_app_inst->st_tunableinfo);
		}
		break;
	*/	
		case DAB_APP_FM_DAB_LINKING_STATUS_NOTIFYID:
		{
			 u32_Index = 0;	/* Initialize index */
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] SFM-DAB linking status notification  ");
			 DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->u8_FM_DAB_Station_Quality,(Tchar *)(pst_msg->data),(Tu8) sizeof(Tu8),&u32_Index);
			 DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_FM_DAB_linking_status,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_APP_fmdab_linkstatus),&u32_Index);
			 DAB_App_Notify_FMtoDAB_linking_Status(pst_me_dab_app_inst->u8_FM_DAB_Station_Quality,pst_me_dab_app_inst->e_FM_DAB_linking_status);
		}
		break;
		
		case DAB_APP_COMPONENT_STATUS_NOTIFYID:
		{
			u32_Index = 0;	/* Initialize index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] DABTuner status notification ");
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_ComponentStatus),(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_Comp_Status),&u32_Index);
			DAB_App_Notify_ComponentStatus(pst_me_dab_app_inst->e_ComponentStatus) ;
		}
		break ;
		
		case DAB_APP_DABTUNER_RESTART_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DABTuner Restart request ");
			DAB_Tuner_Ctrl_Request_DABTUNERRestart();
		}
		break;
		
		case DAB_APP_DABTUNER_RESTART_RESID:
		{
			u32_Index = 0;	/* Initialize index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DABTuner Restart response ");
			DAB_App_ExtractParameterFromMessage(&(pst_me_dab_app_inst->e_Replystatus),(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_Response_DABTUNERRestart(pst_me_dab_app_inst->e_Replystatus) ;
		}
		break;		
		case DAB_APP_SIGNAL_STATUS_NOTIFYID:
		{
			 u32_Index = 0;	/* Initialize index */
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Signal status notification during FM-DAB linking ");
			 DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_DAB_App_SignalStatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_DAB_App_SignalStatus),&u32_Index);			 
			 DAB_App_Notify_SignalStatus(pst_me_dab_app_inst->e_DAB_App_SignalStatus);
		}
		break;	
		
		case DAB_APP_DAB_DLS_DATA_NOTIFYID:
		{
			u32_Index = 0 ;	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] DLS notification ");
			/* Extracting DLS data structure from Tuner Ctrl msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->st_DLSData,(Tchar *)(pst_msg->data),(Tu8)sizeof(Ts_DAB_DLS_Data),&u32_Index);
			/* Sending DLS data Notification to Radio Manager */
			DAB_App_Notify_DLSData(pst_me_dab_app_inst->st_DLSData);
		}
		break;

		case DAB_APP_DAB_SLS_DATA_NOTIFYID:
		{
			u32_Index = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_APP] SLS notification ");
			/*Accessing the shared memory for reading the SLS image*/
			SYS_MUTEX_LOCK(SLS_DAB_APP_DAB_TC);
			SYS_MUTEX_LOCK(SLS_RM_DAB_APP);
			SYS_RADIO_MEMCPY(&st_Dabapp_SLS_Data, &st_DabTc_SLS_Data, sizeof(Ts_DAB_Tuner_Ctrl_DataServiceRaw));
			SYS_MUTEX_UNLOCK(SLS_DAB_APP_DAB_TC);
			SYS_MUTEX_UNLOCK(SLS_RM_DAB_APP);
			/* Sending SLS data Notification to Radio Manager */
			DAB_App_Notify_SLSData();
		}
		break;
		
		case DAB_APP_DAB_AF_LEARN_MEM_NOTIFYID:
		{
			u32_Index = 0;	/* Initialize station list index */
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] AF TUNE RESPONSE ");
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,(Tchar *)(pst_msg->data),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo,(Tchar *)(pst_msg->data),(Tu8) sizeof(Ts_DAB_App_CurrentStationInfo),&u32_Index);
			DAB_App_UpdateChannelName(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.au8_ChannelName);
			DAB_App_Response_AFtune(pst_me_dab_app_inst->e_Replystatus,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
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
/*  Ts_Sys_Msg*  DAB_APP_INST_HSM_ActiveStopHndlr                           */
/*===========================================================================*/
Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveStopHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg)
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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] DAB_APP_INST_HSM_ActiveStopHndlr ");
			/* Storing name of current state handler */
			pst_me_dab_app_inst->ptr_curr_hdlr_name = (Tu8 const *) "DAB_APP_INST_HSM_ActiveStopHndlr \n";
			if(pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type == DAB_APP_ACTIVATE_REQ_VALID)
			{
				DAB_Tuner_Ctrl_Request_Activate_Deactivate((Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus)pst_me_dab_app_inst->e_DAB_App_ActivateDeactivateStatus);
			}
			else if(pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type == DAB_APP_FACTORY_RESET_REQ)
			{
				/* Sending DAB Factory reset request to Tuner Ctrl */
				DAB_Tuner_Ctrl_FactoryReset_Request();
			}
			else
			{
				/* Sending DAB shutdown request to Tuner Ctrl */
				DAB_Tuner_Ctrl_Request_Shutdown();
			}	
			
#ifdef PC_TEST		
			DAB_Tuner_Ctrl_Response_Shutdown(REPLYSTATUS_SUCCESS);
#endif
		}
		break;
		
		case DAB_APP_ACTIVATE_DEACTIVATE_RESID:
		{
			u32_Index = 0;	/* Initialize station list index */

			/* Extracting Tuner Ctrl start up status from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);
			DAB_Inst_App_Response_Shutdown();
			DAB_App_Response_Activate_Deactivate(pst_me_dab_app_inst->e_Replystatus);
			pst_me_dab_app_inst->e_DAB_App_Satrtup_Request_Type = DAB_APP_ACTIVATE_REQ_INVALID;
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_inactive_state);
		}
		break;

		case DAB_APP_SHUTDOWN_RESID:
		{
			u32_Index = 0 ;

			/* Extracting market parameter from msg */ 
			DAB_App_ExtractParameterFromMessage(&pst_me_dab_app_inst->e_Replystatus,&(pst_msg->data[u32_Index]),(Tu8) sizeof(Te_RADIO_ReplyStatus),&u32_Index);

			switch(pst_me_dab_app_inst->e_Replystatus)
			{
				case REPLYSTATUS_SUCCESS:
				{
					/* Sending DAB instance hsm shutdown success response to Main hsm */
					DAB_Inst_App_Response_Shutdown();

					/* Sending DAB shutdown response to Radio Manager */
					DAB_App_Response_Shutdown(pst_me_dab_app_inst->e_Replystatus);
										
					/* Transit to instance inactive state*/
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_inactive_state);

				}
				break;

				case REPLYSTATUS_FAILURE:
				{
										/* Sending DAB instance hsm shutdown success response to Main hsm */
					DAB_Inst_App_Response_Shutdown();

					/* Sending DAB shutdown response to Radio Manager */
					DAB_App_Response_Shutdown(pst_me_dab_app_inst->e_Replystatus);
										
					/* Transit to instance inactive state*/
					HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_inactive_state);

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
			ret = pst_msg;	/* Returning the message pointer to parent state */
		}
		break;
 	
	}
	return ret;	
}

/*================================================================================================*/
/* void Tuner_Ctrl_Sort_CurrEnsembleProgList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply* CurrEnsembleProgList)   */
/*================================================================================================*/
void Dab_App_Sort_CurrEnsembleProgList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *CurrEnsembleProgList)
{
	
	Tu16 i,j;
	Ts_CurrEnsemble_serviceinfo st_TempServiceInfo;
	
		
	for(i = 0 ; ((i < CurrEnsembleProgList->u8_NumOfServices) && (i < MAX_ENSEMBLE_SERVICES)) ; i++)
	{
		for(j = (Tu16)(i+1) ; ((j < CurrEnsembleProgList->u8_NumOfServices) && ( j < MAX_ENSEMBLE_SERVICES)) ; j++)
		{
						
			if(strcmp((char *)CurrEnsembleProgList->st_serviceinfo[i].LabelString,(char *)CurrEnsembleProgList->st_serviceinfo[j].LabelString) > 0)
			{
				st_TempServiceInfo = CurrEnsembleProgList->st_serviceinfo[i];
				CurrEnsembleProgList->st_serviceinfo[i] = CurrEnsembleProgList->st_serviceinfo[j] ;
				CurrEnsembleProgList->st_serviceinfo[j] = st_TempServiceInfo ;
			}
			else
			{
				
			}/* For misra c */
		
		}
	}
	
}

/*====================================================================================================================================

                          FUNCTIO TO FIND NEXT FREQUENCY
=====================================================================================================================================*/


void Search_Next_Frequency(Ts_dab_app_inst_hsm *pst_me_dab_app_inst ,Te_RADIO_DirectionType e_Direction)
{  
	for(pst_me_dab_app_inst->u8_Curr_Frequency_Index = 0 ; pst_me_dab_app_inst->u8_Curr_Frequency_Index < DAB_APP_EU_MAX_FREQUENCIES ; pst_me_dab_app_inst->u8_Curr_Frequency_Index++) /* Finding the currently tuned frequency in the band */
	{
		if(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency == dab_app_freq_band_eu[pst_me_dab_app_inst->u8_Curr_Frequency_Index].u32_frequency) /* currently tuned frequency matched in the band */
		{
			if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency == DAB_APP_5A_EU) && (e_Direction == RADIO_FRMWK_DIRECTION_DOWN)) /* If the currently tuned frequency is 5A and Tune Up/Down direction is Down then tunable frequency is 13F */ 
			{
				pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = DAB_APP_13F_EU ;
			}
			else if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency != DAB_APP_5A_EU) && (e_Direction == RADIO_FRMWK_DIRECTION_DOWN)) /* If the currently tuned frequency is other than 5A and Tune Up/Down direction is Down, then select the previous frequency in the band*/
			{
				pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = dab_app_freq_band_eu[pst_me_dab_app_inst->u8_Curr_Frequency_Index - 1].u32_frequency ;
			}
			else if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency == DAB_APP_13F_EU) && (e_Direction == RADIO_FRMWK_DIRECTION_UP)) /* If the currently tuned frequency is 13F and Tune Up/Down direction is Up then tunable frequency is 5A */ 
			{
				pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = DAB_APP_5A_EU ;
			}
			else if((pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_Tunableinfo.u32_Frequency != DAB_APP_13F_EU) && (e_Direction == RADIO_FRMWK_DIRECTION_UP)) /* If the currently tuned frequency is other than 13F and Tune Up/Down direction is Up, then select the next frequency in the band */
			{
				pst_me_dab_app_inst->st_tunableinfo.u32_Frequency = dab_app_freq_band_eu[pst_me_dab_app_inst->u8_Curr_Frequency_Index + 1].u32_frequency;
			}
			else
			{
				/* Do nothing */
			}/* For MISRA C */
			
			break ; /* break from the loop if the frequency is mathced */
							
		}
	}
}


/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/
/*==============================================================================================*/
/**************************************************************************************************
		void SearchSeekInfo(Ts_dab_tuner_ctrl_inst_hsm* pst_me_dab_app_inst)
***************************************************************************************************/
void SearchSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst)
{
	Tbool b_SIdFound = FALSE, b_SCIdFound = FALSE ;
	u8_ServIndex=0;
	u8_ServCompIndex=0 ; 
	if(pst_me_dab_app_inst->st_tunableinfo.u32_SId != 0)
	{
		for(u8_ServIndex = 0 ; u8_ServIndex < pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices ; u8_ServIndex++) /* Loop for finding current station SId in the current ensemble service list */
		{ 
			if(pst_me_dab_app_inst->st_tunableinfo.u32_SId == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].ProgServiceId)  /* Finding the current station SId in the current ensemble service list */
			{
				b_SIdFound = TRUE ;
				if(pst_me_dab_app_inst->e_SeekDirection == RADIO_FRMWK_DIRECTION_UP)
				{
					if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents == 0)
					{
						if(u8_ServIndex == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices - 1) /* Current service and service component is last in the list */
						{
							/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
							u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
							u16_EId = 0;	/* Initialize EId variable */
							u32_DAB_App_SId = 0; 	/* Initialize SId variable */
							u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
							DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
						}
						else
						{
							pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex+1) ; /* Saving the selected service index */ 
							pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0 ; /* Saving the selected component index */
							UpdateSeekInfo(pst_me_dab_app_inst); /* Updating Seek Info */
							pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
							pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
						
							u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
						    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
					      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
					     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
					      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
						}	
					}
					else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents != 0)
					{
						for(u8_ServCompIndex = 0; (u8_ServCompIndex < pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents); u8_ServCompIndex++) /* Loop for finding current station SCIdI in the current service */
						{
							if(pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.Component[u8_ServCompIndex].InternalCompId)/* Finding the current station SCIdI in the current service */
							{
							b_SCIdFound = TRUE ;
								if(u8_ServCompIndex == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents - 1)  /* Checking if the matched component is last in the current service*/
								{
									if(u8_ServIndex == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices - 1) /* Current service and service component is last in the list */
									{
										/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
										u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
										u16_EId = 0;	/* Initialize EId variable */
										u32_DAB_App_SId = 0; 	/* Initialize SId variable */
										u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
										DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
									}
									else
									{
										pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex+1) ; /* Saving the selected service index */ 
										pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0 ; /* Saving the selected component index */
										UpdateSeekInfo(pst_me_dab_app_inst); /* Updating Seek Info */
										pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
										pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
										
										u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
									    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
								      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
								     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
								      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
									}
								}
								else
								{
									pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = u8_ServIndex ; /* Saving the selected service index */ 
									pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(u8_ServCompIndex+1); /* Saving the selected component index */
									UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
									pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
									pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;									
									
									u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
								    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
							      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
							     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
							      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
								}
								break ; /* If component is matched updating seek info then break */
							}
							
						
						}/* End of for loop */
					}
					else
					{
					}
					
				}
				else if(pst_me_dab_app_inst->e_SeekDirection == RADIO_FRMWK_DIRECTION_DOWN) 
				{
					if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents == 0)
					{
						if(u8_ServIndex == 0) /* Current service and service component is first in the list */
						{
							/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
							u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
							u16_EId = 0;	/* Initialize EId variable */
							u32_DAB_App_SId = 0; 	/* Initialize SId variable */
							u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
							DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
						}
						else
						{
							pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex-1) ; /* Saving the selected service index */ 
							if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents != 0)
							{
								pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents-1); /* Saving the selected component index */
							}
							else
							{
								pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0;
							}
							UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
							pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
							pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
													
							u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
						    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
					      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
					     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
					      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
						}	
					}
					else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents != 0)
					{
						for(u8_ServCompIndex = 0;(u8_ServCompIndex < pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents); u8_ServCompIndex++) /* Loop for finding current station SCIdI in the current service */
						{
							if(pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.Component[u8_ServCompIndex].InternalCompId)/* Finding the current station SCIdI in the current service */
							{
							b_SCIdFound = TRUE ;
								if(u8_ServCompIndex == 0) /* Checking if the matched component is first in the current service*/
								{
									if(u8_ServIndex == 0) /* Current service and service component is first in the list */
									{
										/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
										u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
										u16_EId = 0;	/* Initialize EId variable */
										u32_DAB_App_SId = 0; 	/* Initialize SId variable */
										u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
										DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
									}
									else
									{
										pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex-1) ; /* Saving the selected service index */ 
										if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents != 0)
										{
											pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents-1); /* Saving the selected component index */
										}
										else
										{
											pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0;
										}
										UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
										pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
										pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
																
										u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
									    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
								      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
								     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
								      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
									}
								}
								else
								{
									pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = u8_ServIndex ; /* Saving the selected service index */ 
									pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(u8_ServCompIndex-1); /* Saving the selected component index */
									UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
									pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
									pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
									
									u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
								    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
							      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
							     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
							      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
								}
								break ; /* If component is matched then break */
							}
							else
							{
								b_SCIdFound = FALSE ;
							}
						
						}/* End of for loop */
					}
					else
					{
					}
					
				}
				else
				{
					/* Doing nothing */
				}/* For MISRA C */
			
				break ; /* If Service and component is matched updating seek info then break */
			}
			else
			{
				b_SIdFound = FALSE ;
			}
		
		}/* End of for loop */		
	}
	else
	{
		/* Doing nothing */
	}/* For MISRA C */
	
	if((b_SIdFound == FALSE) || (b_SCIdFound == FALSE))
	{
		pst_me_dab_app_inst->u32_SeekFrequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency ;
	   
		u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency;
		u16_EId = 0;	/* Initialize EId variable */
		u32_DAB_App_SId = 0; 	/* Initialize SId variable */
		u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
		DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
	}
	else
	{
		/* Doing nothing */
	}/* For MISRA C */
}

/**************************************************************************************************
		void UpdateSeekInfo(Ts_dab_tuner_ctrl_inst_hsm* pst_me_dab_app_inst)
***************************************************************************************************/

void UpdateSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst)
{
	pst_me_dab_app_inst->st_SeekInfo.u32_SId = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].ProgServiceId;
	pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI = pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].st_compInfo.Component[pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek].InternalCompId;
	
	if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].CharSet == 0x00)
	{
		pst_me_dab_app_inst->st_SeekInfo.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
	}
	else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].CharSet == 0x06)
	{
		pst_me_dab_app_inst->st_SeekInfo.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
	}
	else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].CharSet == 0x0f)
	{
		pst_me_dab_app_inst->st_SeekInfo.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
	}
	else
	{
		/*Doing nothing */ 	
	}/* For MISRA C */
	
	SYS_RADIO_MEMCPY((pst_me_dab_app_inst->st_SeekInfo.service_label.au8_label),(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);

	if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].st_compInfo.Component[pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek].CharSet == 0x00)
	{
		pst_me_dab_app_inst->st_SeekInfo.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
	}
	else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].st_compInfo.Component[pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek].CharSet == 0x06)
	{
		pst_me_dab_app_inst->st_SeekInfo.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
	}
	else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].st_compInfo.Component[pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek].CharSet == 0x0f)
	{
		pst_me_dab_app_inst->st_SeekInfo.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
	}
	else
	{
		/*Doing nothing */ 	
	}/* For MISRA C */
	
	SYS_RADIO_MEMCPY((	pst_me_dab_app_inst->st_SeekInfo.servicecomponent_label.au8_label),(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek].st_compInfo.Component[pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
	pst_me_dab_app_inst->st_SeekInfo.u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency;
	pst_me_dab_app_inst->st_SeekInfo.u16_EId = pst_me_dab_app_inst->st_tunableinfo.u16_EId;
}
/**************************************************************************************************
		void SearchServiceSeekInfo(Ts_dab_tuner_ctrl_inst_hsm* pst_me_dab_app_inst)
***************************************************************************************************/

void SearchServiceSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst)
{
	Tbool b_SIdFound = FALSE;
	u8_ServIndex=0; 
	if(pst_me_dab_app_inst->st_SeekInfo.u32_SId != 0)
	{
		for(u8_ServIndex = 0 ; u8_ServIndex < pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices ; u8_ServIndex++) /* Loop for finding current station SId in the current ensemble service list */
		{ 
			if(pst_me_dab_app_inst->st_SeekInfo.u32_SId == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].ProgServiceId)  /* Finding the current station SId in the current ensemble service list */
			{
				b_SIdFound = TRUE ;
				if(pst_me_dab_app_inst->e_SeekDirection == RADIO_FRMWK_DIRECTION_UP)
				{
			
					if(u8_ServIndex == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices - 1) /* Current service and service component is last in the list */
					{
						if(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency != pst_me_dab_app_inst->u32_SeekFrequency)
						{
							/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
							u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
							u16_EId = 0;	/* Initialize EId variable */
						    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
						    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
							DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
							
						}
						else
						{   
							if(pst_me_dab_app_inst->b_SeekStarted == TRUE)
							{
								/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
								pst_me_dab_app_inst->b_SeekStarted = FALSE;
								u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
								u16_EId = 0;	/* Initialize EId variable */
							    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
							    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
								DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
							}
							else
							{
						
								/* Clear service label in "s_DAB_App_CurrentStationInfo" structure */

								memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

								/* Clear Component label in "s_DAB_App_CurrentStationInfo" structure */
								memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

								/* Sending Seek response to DAB application with No Signal */
								DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);

								/* Clearing the e_DAB_App_RequestCmd */
								pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID ;	
								
								/* Clear stationlist as no stations are available*/
								ensembleIndex = 0;
								Services = 0;
								Components = 0;
								memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
								memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
								memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
   								DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
								HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
						   	}
						}
					}
					else
					{
						pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek =(Tu8)(u8_ServIndex+1) ; /* Saving the selected service index */ 
						pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0 ; /* Saving the selected component index */
						UpdateSeekInfo(pst_me_dab_app_inst); /* Updating Seek Info */
						pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
						pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
					    
						u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
					    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
				      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
				     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
				      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
						
					}
					
				}
				else if(pst_me_dab_app_inst->e_SeekDirection == RADIO_FRMWK_DIRECTION_DOWN)
				{
			
					if(u8_ServIndex == 0) /* Current service and service component is first in the list */
					{
						if(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency != pst_me_dab_app_inst->u32_SeekFrequency)
						{
							/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
							u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
							u16_EId = 0;	/* Initialize EId variable */
							u32_DAB_App_SId = 0; 	/* Initialize SId variable */
							u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
							DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
							
						}
						else
						{   
							if(pst_me_dab_app_inst->b_SeekStarted == TRUE)
							{
								/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
								pst_me_dab_app_inst->b_SeekStarted = FALSE;
								u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
								u16_EId = 0;	/* Initialize EId variable */
								u32_DAB_App_SId = 0; 	/* Initialize SId variable */
								u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
								DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
							}
							else
							{
							
								/* Clear service label in "s_DAB_App_CurrentStationInfo" structure */

								memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

								/* Clear Component label in "s_DAB_App_CurrentStationInfo" structure */
								memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

								/* Sending Seek response to DAB application with No Signal */
								DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);

								/* Clearing the e_DAB_App_RequestCmd */
								pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID ;	
							
								/* Clear stationlist as no stations are available*/
								ensembleIndex = 0;
								Services = 0;
								Components = 0;
								memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
								memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
								memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
								DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
								HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
													
							}
						}
					}
					else
					{
						pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex-1) ; /* Saving the selected service index */ 
						pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents-1); /* Saving the selected component index */
						UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
					    pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
						pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
						
						u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
					    u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
				      	u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
				     	u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;	/* Initialize SCIdI variable */
				      	DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */					
					}
		
				}
				else
				{
					/* Doing nothing */
				}/* For MISRA C */
				break ;
			}
			else
			{
				b_SIdFound = FALSE ;
			}
		
		}/* End of for loop */
	}
	else
	{
		/* Doing nothing */
	}/* For MISRA C */
	if(b_SIdFound == FALSE)
	{
		pst_me_dab_app_inst->b_SeekStarted = FALSE;
		if(pst_me_dab_app_inst->u32_SeekFrequency == pst_me_dab_app_inst->st_tunableinfo.u32_Frequency)
		{
			DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
			HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
		}
		else
		{
			/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
			u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency;
			u16_EId = 0;	/* Initialize EId variable */
		    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
		    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
			DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
		}
 
	}
	else
	{
		/* Doing nothing */
	}/* For MISRA C */
	
}


/**************************************************************************************************
		void SearchServiceCompSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst)
***************************************************************************************************/

void SearchServiceCompSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst)
{
	Tbool b_SIdFound = FALSE, b_SCIdFound = FALSE ;
	u8_ServIndex=0;
	u8_ServCompIndex=0;
	
	
	if(pst_me_dab_app_inst->st_SeekInfo.u32_SId != 0)
	{
		for(u8_ServIndex = 0 ; u8_ServIndex < pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices ; u8_ServIndex++) /* Loop for finding current station SId in the current ensemble service list */
		{ 
			if(pst_me_dab_app_inst->st_SeekInfo.u32_SId == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].ProgServiceId)  /* Finding the current station SId in the current ensemble service list */
			{
				b_SIdFound = TRUE ;
				if(pst_me_dab_app_inst->e_SeekDirection == RADIO_FRMWK_DIRECTION_UP)
				{
					if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents == 0)
					{
						if(u8_ServIndex == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices - 1) /* Current service and service component is last in the list */
						{
							if(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency != pst_me_dab_app_inst->u32_SeekFrequency)
							{
								/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
								u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
								u16_EId = 0;	/* Initialize EId variable */
							    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
							    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
								DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
							}
							else
							{
								if(pst_me_dab_app_inst->b_SeekStarted == TRUE)
								{
									pst_me_dab_app_inst->b_SeekStarted = FALSE;
									/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
									u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
									u16_EId = 0;	/* Initialize EId variable */
								    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
								    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
									DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
								}
								else
								{
									/* Clear service label in "s_DAB_App_CurrentStationInfo" structure */

									memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

									/* Clear Component label in "s_DAB_App_CurrentStationInfo" structure */
									memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

									/* Sending Seek response to DAB application with No Signal */
									DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);

									/* Clearing the e_DAB_App_RequestCmd */
									pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID ;	
											
									/* Clear stationlist as no stations are available*/
									ensembleIndex = 0;
									Services = 0;
									Components = 0;
									memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
									memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
									memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
								    DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
									HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
								}
							}
						}
						else
						{
							b_SCIdFound = FALSE ;
							pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex+1) ; /* Saving the selected service index */ 
							pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0 ; /* Saving the selected component index */
							UpdateSeekInfo(pst_me_dab_app_inst); /* Updating Seek Info */
							pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
							pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
							
							u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
							u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
						    u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
						    u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
						    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
						}
					}
					else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents != 0)
					{
						for(u8_ServCompIndex = 0;(u8_ServCompIndex < pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents); u8_ServCompIndex++) /* Loop for finding current station SCIdI in the current service */
						{
							if(pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.Component[u8_ServCompIndex].InternalCompId)/* Finding the current station SCIdI in the current service */
							{
							b_SCIdFound = TRUE ;
								if((u8_ServCompIndex == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents - 1)||(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents == 0)) /* Checking if the matched component is last in the current service*/
								{
									if(u8_ServIndex == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.u8_NumOfServices - 1) /* Current service and service component is last in the list */
									{
										if(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency != pst_me_dab_app_inst->u32_SeekFrequency)
										{
											/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
											u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
											u16_EId = 0;	/* Initialize EId variable */
										    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
										    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
											DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
										}
										else
										{
											if(pst_me_dab_app_inst->b_SeekStarted == TRUE)
											{
												pst_me_dab_app_inst->b_SeekStarted = FALSE;
												/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
												u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
												u16_EId = 0;	/* Initialize EId variable */
											    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
											    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
												DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
											}
											else
											{
												
												/* Clear service label in "s_DAB_App_CurrentStationInfo" structure */

												memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));
		
												/* Clear Component label in "s_DAB_App_CurrentStationInfo" structure */
												memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));
		
												/* Sending Seek response to DAB application with No Signal */
												DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
		
												/* Clearing the e_DAB_App_RequestCmd */
												pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID ;	
																		
												/* Clear stationlist as no stations are available*/
												ensembleIndex = 0;
												Services = 0;
												Components = 0;
												memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
												memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
												memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
												DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);											
												HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);
											}
										}
									}
									else
									{
										pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex+1) ; /* Saving the selected service index */ 
										pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0 ; /* Saving the selected component index */
										UpdateSeekInfo(pst_me_dab_app_inst); /* Updating Seek Info */
										pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
										pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
										
										u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
										u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
									    u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
									    u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
									    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
									}
								}
								else
								{
									pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = u8_ServIndex ; /* Saving the selected service index */ 
									pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(u8_ServCompIndex+1); /* Saving the selected component index */
									UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
									pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
									pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
									
									u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
									u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
								    u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
								    u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
								    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
								}
								break ; /* If component is matched updating seek info then break */
							}
							else
							{
								b_SCIdFound = FALSE ;
							}
					
						}/* End of for loop */
					}
				 
				}
				else if(pst_me_dab_app_inst->e_SeekDirection == RADIO_FRMWK_DIRECTION_DOWN)
				{
					if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents == 0)
					{
						b_SCIdFound = TRUE ;
						if(u8_ServIndex == 0) /* Current service and service component is first in the list */
						{
							if(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency != pst_me_dab_app_inst->u32_SeekFrequency)
							{
								/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
								u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
								u16_EId = 0;	/* Initialize EId variable */
							    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
							    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
								DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
							}
							else
							{
								if(pst_me_dab_app_inst->b_SeekStarted == TRUE)
								{
									pst_me_dab_app_inst->b_SeekStarted = FALSE;
									/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
									u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
									u16_EId = 0;	/* Initialize EId variable */
								    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
								    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
									DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
								}
								else
								{
									
									/* Clear service label in "s_DAB_App_CurrentStationInfo" structure */

									memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

									/* Clear Component label in "s_DAB_App_CurrentStationInfo" structure */
									memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));

									/* Sending Seek response to DAB application with No Signal */
									DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);

									/* Clearing the e_DAB_App_RequestCmd */
									pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID ;	
											
									/* Clear stationlist as no stations are available*/
									ensembleIndex = 0;
									Services = 0;
									Components = 0;
									memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
									memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
									memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
									DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);								
									HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);	
								}
							}
						}
						else
						{
							pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex-1) ; /* Saving the selected service index */ 
							if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents != 0)
								pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents-1); /* Saving the selected component index */
							else
								pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0;
							
							UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
							pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
							pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
							
							u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
							u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
						    u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
						    u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
						    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
						}						
					}
					else if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents != 0)
					{
						for(u8_ServCompIndex = 0; (u8_ServCompIndex < pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.NoOfComponents); u8_ServCompIndex++) /* Loop for finding current station SCIdI in the current service */
						{
							if(pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI == pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[u8_ServIndex].st_compInfo.Component[u8_ServCompIndex].InternalCompId)/* Finding the current station SCIdI in the current service */
							{
							b_SCIdFound = TRUE ;
								if(u8_ServCompIndex == 0) /* Checking if the matched component is first in the current service*/
								{
									if(u8_ServIndex == 0) /* Current service and service component is first in the list */
									{
										if(pst_me_dab_app_inst->st_tunableinfo.u32_Frequency != pst_me_dab_app_inst->u32_SeekFrequency)
										{
											/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
											u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
											u16_EId = 0;	/* Initialize EId variable */
										    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
										    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
											DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
										}
										else
										{
											if(pst_me_dab_app_inst->b_SeekStarted == TRUE)
											{
												pst_me_dab_app_inst->b_SeekStarted = FALSE;
												/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
												u32_Frequency = pst_me_dab_app_inst->u32_SeekFrequency;
												u16_EId = 0;	/* Initialize EId variable */
											    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
											    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
												DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
											}
											else
											{
												/* Clear service label in "s_DAB_App_CurrentStationInfo" structure */

												memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ServiceLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));
		
												/* Clear Component label in "s_DAB_App_CurrentStationInfo" structure */
												memset(&(pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo.st_ComponentLabel), 0, sizeof(Ts_Tuner_Ctrl_Label));
		
												/* Sending Seek response to DAB application with No Signal */
												DAB_App_Response_ServiceCompSeekUpDown(REPLYSTATUS_NO_SIGNAL,pst_me_dab_app_inst->s_DAB_App_CurrentStationInfo);
		
												/* Clearing the e_DAB_App_RequestCmd */
												pst_me_dab_app_inst->e_DAB_App_RequestCmd = DAB_APP_INVALID ;	
											
												/* Clear stationlist as no stations are available*/
												ensembleIndex = 0;
												Services = 0;
												Components = 0;
												memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
												memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
												memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
												DAB_App_Notify_STLUpdated(DAB_APP_RADIO_MNGR);
												HSM_STATE_TRANSITION(pst_me_dab_app_inst, &dab_app_inst_hsm_active_idle_state);		
											}
										}
									}
									else
									{
										pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = (Tu8)(u8_ServIndex-1) ; /* Saving the selected service index */ 
										if(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents != 0)
											pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(pst_me_dab_app_inst->st_DAB_App_CurrEnsembleProgList.st_serviceinfo[pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek ].st_compInfo.NoOfComponents-1); /* Saving the selected component index */
										else
										    pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = 0;
										UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
										pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
										pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;	
										
										u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
										u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
									    u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
									    u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
									    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
									}
								}
								else
								{
									pst_me_dab_app_inst->u8_CurrentServiceIndex_Seek = u8_ServIndex ; /* Saving the selected service index */ 
									pst_me_dab_app_inst->u8_CurrentServiceCompIndex_Seek = (Tu8)(u8_ServCompIndex-1); /* Saving the selected component index */
									UpdateSeekInfo(pst_me_dab_app_inst);  /* Updating Seek Info */
									pst_me_dab_app_inst->st_tunableinfo.u32_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId;
									pst_me_dab_app_inst->st_tunableinfo.u16_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI;
									
									u32_Frequency = pst_me_dab_app_inst->st_SeekInfo.u32_Frequency;
									u16_EId = pst_me_dab_app_inst->st_SeekInfo.u16_EId;	/* Initialize EId variable */
								    u32_DAB_App_SId = pst_me_dab_app_inst->st_SeekInfo.u32_SId; 	/* Initialize SId variable */
								    u16_DAB_App_SCIdI = pst_me_dab_app_inst->st_SeekInfo.u16_SCIdI  ;	/* Initialize SCIdI variable */
								    DAB_Tuner_Ctrl_Request_SelectService(u32_Frequency, u16_EId, u32_DAB_App_SId, u16_DAB_App_SCIdI); /* If the current service and service component is last in the list and direction is UP then transiting to SearchNext_state */
								}
								break ; /* If component is matched then break */
							}
							else
							{
								b_SCIdFound = FALSE ;
							}
					
						}/* End of for loop */
					}
			
				}
				else
				{
					/* Doing nothing */
				}/* For MISRA C */
			
				break ; /* If Service and component is matched updating seek info then break */
			}
			else
			{
				b_SIdFound = FALSE ;
			}
		
		}/* End of for loop */
	}
	else
	{
		/* Doing nothing */
	}/* For MISRA C */
	
	if((b_SIdFound == FALSE) || (b_SCIdFound == FALSE))
	{
		pst_me_dab_app_inst->b_SeekStarted = FALSE;
		/* Find the next/prev frequency to tune next and send request to DAB Tuner Ctrl */
		u32_Frequency = pst_me_dab_app_inst->st_tunableinfo.u32_Frequency;
		u16_EId = 0;	/* Initialize EId variable */
	    u32_DAB_App_SId = 0; 	/* Initialize SId variable */
	    u16_DAB_App_SCIdI = 0 ;	/* Initialize SCIdI variable */
		DAB_Tuner_Ctrl_Request_AutoSeekUpDown(u32_Frequency, pst_me_dab_app_inst->e_SeekDirection, pst_me_dab_app_inst->b_SeekStarted);
	}
	else
	{
		/* Doing nothing */
	}/* For MISRA C */
	
}

/*==================================================================================================*/
/*    end of file                                                                                   */
/*==================================================================================================*/
