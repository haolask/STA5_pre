/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_inst_hsm.c																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This source file consists of function defintions of all function handlers of HSM	*
*						  amfm_app_inst_hsm	of AMFM Applicatiopn component									*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "amfm_app_inst_hsm.h"
#include "amfm_app_application.h"
#include "interpolation.h"
#include "sys_nvm.h"

/*-----------------------------------------------------------------------------
    extern variable Declaration
-----------------------------------------------------------------------------*/
extern Te_AMFM_App_mode		e_CurrentMode;				// Defined in amfm_app_response.c 
extern Ts_Sys_Msg			st_amfm_app_req_msg;		// Defined in amfm_app_request.c
extern Ts_Sys_Msg			st_amfm_app_res_msg;		// Defined in amfm_app_response.c


/*-----------------------------------------------------------------------------
    Global variable Defintions (static)
-----------------------------------------------------------------------------*/
Ts_AMFM_App_AM_STL			st_am_station_list;		/* structure holds AM station list as per master specified format */ 
Ts_AMFM_App_FM_STL			st_fm_station_list;		/* structure holds FM station list as per master specified format */ 


Tbool	b_NVM_ActiveUpdateSwitch = TRUE;
Tu32	u32_start_freq           = 0;
Tu32 	u32_StepSize             = 0;
Tu32	u32_End_Freq             = 0;
Tu8		u8_af_update_cycle       = 0;
Tu8 	u8_af_updatecount        = AMFM_APP_AF_UPDATE_LOOP_COUNT;
Tu8 	u8_StrategyAFupdatecount = AMFM_APP_AF_STRATEGY_UPDATE_LOOP_COUNT;
  
#ifdef CALIBRATION_TOOL

Ts_AMFM_App_LinkingParam				st_Calibration_DAB2FM_linkParam;

Tbool 	b_start_stop_switch = FALSE;	/*initially zero considering switch is off when it is set to one it is considered as tuened on*/
Tu32 u32_calib_freq=0;
Tu8 u8_notify_count=0;
Tu32 u32_cal_curr=0;
Tu32 u32_cal_avg=0;
Tu32 u32_cal_old=0;
Tu32 u32_cal_ofs=0;
Ts32 s32_cal_BBFieldStrength;
Tu32 u32_cal_UltrasonicNoise;
Tu32 u32_cal_Multipath;

#endif


/*New strategy*/
Ts_AMFM_App_AFList_Info		st_AMFM_App_AFList_Info;


#ifdef CALIBRATION_TOOL
	Ts_AMFM_App_AFList_Info st_Calib_AMFM_App_AFList_Info;
#endif

Ts_AMFM_App_TimerId		st_AMFM_TimerId; /* structure holds timerIds*/

/* Defining states of HSM amfm_app_inst_hsm */
HSM_CREATE_STATE(	    		    	amfm_app_inst_hsm_top_state,    					        		    NULL,									                    AMFM_APP_INST_HSM_TopHndlr,	        				    		"amfm_app_inst_hsm_top_state");
	HSM_CREATE_STATE(   			    amfm_app_inst_hsm_inactive_state,   		        				    &amfm_app_inst_hsm_top_state,			                    AMFM_APP_INST_HSM_InactiveHndlr,	        		    		"amfm_app_inst_hsm_inactive_state");
	HSM_CREATE_STATE(	    		    amfm_app_inst_hsm_active_state,		            				    	&amfm_app_inst_hsm_top_state,			                    AMFM_APP_INST_HSM_ActiveHndlr,				        	    	"amfm_app_inst_hsm_active_state");
		HSM_CREATE_STATE(       		amfm_app_inst_hsm_active_start_state,   				              	&amfm_app_inst_hsm_active_state,		                    AMFM_APP_INST_HSM_ActiveStartHndlr,         					"amfm_app_inst_hsm_active_start_state");
		HSM_CREATE_STATE(	        	amfm_app_inst_hsm_active_sleep_state,	    		        	    	&amfm_app_inst_hsm_active_state,                    		AMFM_APP_INST_HSM_ActiveSleepHndlr,			            		"amfm_app_inst_hsm_active_sleep_state");
			HSM_CREATE_STATE(  		    amfm_app_inst_hsm_active_sleep_background_processSTL_state,	   	        &amfm_app_inst_hsm_active_sleep_state,	             	    AMFM_APP_INST_HSM_ActiveSleepBackgroundProcessSTLHndlr,	        "amfm_app_inst_hsm_active_sleep_background_process_stl_state");
			HSM_CREATE_STATE(  		    amfm_app_inst_hsm_active_sleep_aftune_state,	   	        			&amfm_app_inst_hsm_active_sleep_state,	             	    AMFM_APP_INST_HSM_ActiveSleepAFTuneHndlr,	        "amfm_app_inst_hsm_active_sleep_aftune_state");
		HSM_CREATE_STATE(		        amfm_app_inst_hsm_active_idle_state,		            	    		&amfm_app_inst_hsm_active_state,                    		AMFM_APP_INST_HSM_ActiveIdleHndlr,	            				"amfm_app_inst_hsm_active_idle_state");
			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_idle_listen_state,     		    		    &amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveIdleListenHndlr,		            	"amfm_app_inst_hsm_active_idle_listen_state");
	
			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_idle_af_strategy_tune_state,     	        	&amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveIdleAFStrategyTuneHndlr,            			"amfm_app_inst_hsm_active_idle_af_strategy_tune_state");
			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_idle_af_tune_state,        		        	&amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveIdleAFTuneHndlr,            			"amfm_app_inst_hsm_active_idle_af_tune_state");
			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_idle_af_reg_tune_state,        		       	&amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveIdleAFRegTuneHndlr,            			"amfm_app_inst_hsm_active_idle_af_reg_tune_state");
		
			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_idle_tune_failed_listen_state,        		    &amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveIdleTuneFailedListenHndlr,		       	"amfm_app_inst_hsm_active_idle_tune_failed_listen_state");
//			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_idle_program_station_announcement_state,       &amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveIdleProgramStationAnnouncementHndlr,	"amfm_app_inst_hsm_active_idle_program_station_announcement_state");
//			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_idle_eon_station_announcement_state,        	&amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveIdleEONStationAnnouncementHndlr,		"amfm_app_inst_hsm_active_idle_eon_station_announcement_state");
	
		#ifdef CALIBRATION_TOOL
			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_calibration_start_tune_state,        		        	&amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveCalibrationStartTuneState,            			"amfm_app_inst_hsm_active_calibration_start_tune_state");
			HSM_CREATE_STATE(       	amfm_app_inst_hsm_active_calibration_stop_tune_state,        		        	&amfm_app_inst_hsm_active_idle_state,                   	AMFM_APP_INST_HSM_ActiveCalibrationStopTuneState,            			"amfm_app_inst_hsm_active_calibration_stop_tune_state");
		#endif	
		
		HSM_CREATE_STATE(	    	    amfm_app_inst_hsm_active_busy_state,				                	&amfm_app_inst_hsm_active_state,		                    AMFM_APP_INST_HSM_ActiveBusyHndlr,          					"amfm_app_inst_hsm_active_busy_state");
			HSM_CREATE_STATE(	        amfm_app_inst_hsm_active_busy_select_station_state,          	    	&amfm_app_inst_hsm_active_busy_state,                   	AMFM_APP_INST_HSM_ActiveBusySelectStationHndlr,         		"amfm_app_inst_hsm_active_busy_select_station_state");
			HSM_CREATE_STATE(   	    amfm_app_inst_hsm_active_busy_seek_state,			        	        &amfm_app_inst_hsm_active_busy_state,	                    AMFM_APP_INST_HSM_ActiveBusySeekHndlr,			            	"amfm_app_inst_hsm_active_busy_seek_state");
			HSM_CREATE_STATE(   	    amfm_app_inst_hsm_active_busy_tune_up_down_state,			        	&amfm_app_inst_hsm_active_busy_state,	                    AMFM_APP_INST_HSM_ActiveBusyTuneUpDownHndlr,			        "amfm_app_inst_hsm_active_busy_tune_up_down_state");			
			HSM_CREATE_STATE(        	amfm_app_inst_hsm_active_busy_processSTL_state,     		        	&amfm_app_inst_hsm_active_busy_state,	                    AMFM_APP_INST_HSM_ActiveBusyProcessSTLHndlr,	            	"amfm_app_inst_hsm_active_busy_process_stl_state");
			HSM_CREATE_STATE(           amfm_app_inst_hsm_active_busy_linking_state ,							&amfm_app_inst_hsm_active_busy_state,						AMFM_APP_INST_HSM_ActiveBusyLinkingHndlr,						"amfm_app_inst_hsm_active_busy_linking_state");
				HSM_CREATE_STATE(       amfm_app_inst_hsm_active_busy_linking_searching_state ,					&amfm_app_inst_hsm_active_busy_linking_state,				AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingHndlr,				"amfm_app_inst_hsm_active_busy_linking_searching_state");
					HSM_CREATE_STATE(	amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state ,&amfm_app_inst_hsm_active_busy_linking_searching_state,	AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingGenerateHLFreqListHndlr, "amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state");	
						HSM_CREATE_STATE(   amfm_app_inst_hsm_active_busy_linking_searching_read_quality_state ,		 &amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state,	AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingReadQualityHndlr,		"amfm_app_inst_hsm_active_busy_linking_searching_read_quality_state");					
					HSM_CREATE_STATE(   amfm_app_inst_hsm_active_busy_linking_searching_bg_tune_state ,		 		 &amfm_app_inst_hsm_active_busy_linking_searching_state,	AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingBgTuneHndlr,			"amfm_app_inst_hsm_active_busy_linking_searching_bg_tune_state");
					HSM_CREATE_STATE(	amfm_app_inst_hsm_active_busy_linking_searching_find_best_pi_state ,	   	 &amfm_app_inst_hsm_active_busy_linking_searching_state,	AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingFindBestPIHndlr,		"amfm_app_inst_hsm_active_busy_linking_searching_find_best_pi_state"); 		
				HSM_CREATE_STATE(       amfm_app_inst_hsm_active_busy_linking_monitoring_state ,					 &amfm_app_inst_hsm_active_busy_linking_state,				AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringHndlr,					"amfm_app_inst_hsm_active_busy_linking_monitoring_state");					
					HSM_CREATE_STATE(   amfm_app_inst_hsm_active_busy_linking_monitoring_af_tune_state,			 	&amfm_app_inst_hsm_active_busy_linking_monitoring_state,	AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringAFtuneHndlr,			"amfm_app_inst_hsm_active_busy_linking_monitoring_af_tune_state");					
		HSM_CREATE_STATE(		        amfm_app_inst_hsm_active_stop_state,					                &amfm_app_inst_hsm_active_state,		                    AMFM_APP_INST_HSM_ActiveStopHndlr,					            "amfm_app_inst_hsm_active_stop_state");






/*-----------------------------------------------------------------------------
    Private Function Definitions
-----------------------------------------------------------------------------*/	

/*===========================================================================*/
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_TopHndlr				 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_TopHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL;             /* Pointer to be updated to the  msg pst_msg  at default case*/


	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_TopHndlr");

			/* Updating the current state name  used for debugging purpose */ 
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name),(const void *)"amfm_app_inst_hsm_top_state",sizeof("amfm_app_inst_hsm_top_state"));

			/* Transisting to inactive state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_inactive_state);
		}
		break;
		
		case HSM_MSGID_EXIT:
		{
			
		}
		break;

		default:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Inst HSM TopHndlr MSG: %d", pst_msg->msg_id);
			pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}	

/*===========================================================================*/
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_InactiveHndlr			 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_InactiveHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL;              /* Pointer to be updated to the  msg pst_msg  at default case*/

	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_InactiveHndlr");
	
			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_inactive_state" ,sizeof("amfm_app_inst_hsm_inactive_state" ));
		}
		break;
		
		case AMFM_APP_INST_HSM_STARTUP:
		{
			
			/* Transisting to active start state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_start_state);
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
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveHndlr			 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg						        *pst_ret = NULL;	/* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_AMFM_App_AF_Switch					e_AF_Switch;		/*Enum variable to read the AF switch value.*/				
	Te_AMFM_App_TA_Switch					e_TA_Switch;					
	Te_AMFM_App_FM_To_DAB_Switch			e_FM_to_DAB_Switch; 
	Te_RADIO_ReplyStatus 		            e_AMFM_APP_AFSwitchReplyStatus  ;
	Te_RADIO_ReplyStatus		           	e_TASwitchReplyStatus;
	Te_RADIO_ReplyStatus	                e_FM_to_DAB_SwitchReplyStatus;
	Ts_Sys_Msg								*pst_ResetDoneMsg = &st_amfm_app_res_msg;			/* pointer to message structure defined globally */
	Tu8										u8_NVM_ret=1;
	Tu32									u32_NVM_Update_Param =0;
	Tu32	                                u32_DataSlotIndex    =0;
	
    switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_state",sizeof("amfm_app_inst_hsm_active_state"));
		}
		break;
		
		case AMFM_APP_INST_HSM_SHUTDOWN:
		{
			/* Transisting to active stop state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_stop_state);	
		}
		break;

		case AMFM_APP_FACTORY_RESET_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Factory reset request received");

			/* Resetting the LSM data*/
			memset(&(pst_me_amfm_app_inst->st_LSM_FM_Info),0,sizeof(Ts_AMFM_App_LSM_FM_Band));				
			memset((pst_me_amfm_app_inst->ast_AF_Learn_mem),0,(sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) );				
			
			/*Writting the cleared LSM data into NVM */
			u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
			if(u8_NVM_ret == 0)
			{
				/*NVM data cleared*/
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM reset success  ");
				AMFM_Tuner_Ctrl_Request_Factory_Reset();
			}
			else
			{
				/*Factory reset failed*/
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during factory reset  ");
				AMFM_App_Response_FactoryReset(AMFM_APP_FACTORY_RESET_FAILURE);
			}
			
		}break;
		
		
		case AMFM_APP_FACTORY_RESET_DONE_RESID:
		{
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Factory reset done response id received from tuner control");

			/* Sending AMFM_APP_INST_HSM_RESET_DONE message from  amfm_app_hsm asynchronously. */

			/* Reading e_ResetDoneReplyStatus reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->e_FactoryResetReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_AMFM_App_FactoryResetReplyStatus), &u32_DataSlotIndex);
			
            /* sending factory reset done message from instance hsm to main hsm */
            AMFM_APP_SendMsgtoMainhsm(pst_ResetDoneMsg,AMFM_APP_INST_HSM_RESET_DONE);
            
			/* Transisting to inactive  state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_inactive_state);	
			
		}break;
		
		case AMFM_APP_TUNER_AMFMTUNER_ABNORMAL_NOTIFYID:
		{
			/*Debug Log for AMFM APP receiving POR*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][AMFM_APP] Power on reset occured ");

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->e_AMFMTunerStatus,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_RADIO_Comp_Status),&u32_DataSlotIndex); 

			/* Notifying FMTUNER tuner Abnormality to Radio Manager component */
			AMFM_App_Notify_AMFMTuner_Abnormal(pst_me_amfm_app_inst->e_AMFMTunerStatus);

			if(pst_me_amfm_app_inst->e_Processing_Status == AMFM_APP_FOREGROUND)
			{
				/* Transisting to active idle state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_state);						
			}
			else
			{
				/* Transisting to active sleep state. After transtion to active sleep state,st_DAB_FM_LinkingParam & st_AMFM_App_AFList_Info will be cleared  */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);					
			}
		}
		break;

		case AMFM_APP_DABTUNER_STATUS_NOTIFYID:
		{
			/* Reading DABTuner status value from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->e_DABTunerStatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_Comp_Status),&u32_DataSlotIndex); 

			if(pst_me_amfm_app_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_ABNORMAL)
			{
				/*Debug log for staturn abnormal notification*/
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] DABTuner abnormal occured ");

				if(pst_me_amfm_app_inst->e_Processing_Status == 	AMFM_APP_FOREGROUND)
				{
					/* Initialising parameters related to FM<=>DAB Follow up */					
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality 		= AMFM_APP_CONSTANT_ZERO;
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE 	= AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status	= AMFM_APP_FM_STATION_TUNED;
				}
				else
				{
					/* Transisting to active sleep state. After transtion to active sleep state,st_DAB_FM_LinkingParam & st_AMFM_App_AFList_Info will be cleared  */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);					
				}
			}
			else
			{
				/*Debug log for staturn normal notification*/
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] DABTuner normaled back ");

				if(pst_me_amfm_app_inst->e_Processing_Status == 	AMFM_APP_FOREGROUND)
				{
					if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
					{
						if((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI  != 0) && (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON) &&
						   (pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)&& (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
						{
							/*Debug log for  notification to find SID*/
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] DABTuner back to normal so sending %x PI to DAB  to proceed for FM-DAB",pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);

							/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
							AMFM_App_Notify_Find_SID(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						}
						else
						{
							/* No need to send notification if Current station PI is zero */
						}
					}
					else
					{
						/* No need to send notification if Active Band is AM */
					}
				}
				else
				{
					/* Nothing to do. */
				}					
			}
		}
		break;		

		case AMFM_APP_AF_SWITCH_REQID:
		{
			/* Reading mode value from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_AF_Switch,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_AF_Switch),&u32_DataSlotIndex);

			if(e_AF_Switch==AMFM_APP_AF_SWITCH_OFF || e_AF_Switch==AMFM_APP_AF_SWITCH_ON )
			{
					pst_me_amfm_app_inst->e_AF_Switch = e_AF_Switch;
					
				e_AMFM_APP_AFSwitchReplyStatus = REPLYSTATUS_SUCCESS ;			
			}
			else
			{
				e_AMFM_APP_AFSwitchReplyStatus = REPLYSTATUS_FAILURE;
			}
			
			AMFM_App_Response_AFSwitch(e_AMFM_APP_AFSwitchReplyStatus);
			
			if( e_AF_Switch == AMFM_APP_AF_SWITCH_ON)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] RDS switch turned ON");

				if(pst_me_amfm_app_inst->e_Processing_Status == 	AMFM_APP_FOREGROUND)
				{
					if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
					{
						if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u16_curr_PI!=0) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
						{
							/*Debug log for  notification to find SID*/
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] RDS switch turned on sending %x PI to DAB  to proceed for FM-DAB",pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
							/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
							AMFM_App_Notify_Find_SID(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						}
						else
						{
							/* No need to send notification if Current station PI is zero or FM to dab switch is OFF */
						}
					}
					else
					{
						/* No need to send notification if Active Band is AM */
					}
				}
				else
				{
					/* Nothing to do. */
				}
			}
			else
			{
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] RDS switch turned OFF");
				/* AF switch turned OFF */
				if(pst_me_amfm_app_inst->e_Processing_Status == AMFM_APP_FOREGROUND)
				{
					if((pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM) &&
					   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_NEW_REQUEST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_STATION_TUNED;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					else
					{
						/* no need to reinitialise if active mode is AM or Fm to Dab request itself not sent*/
					}
				}
				else
				{
					/* nothing to do */
				}
			}
		}
		break;
	
		case AMFM_APP_TA_SWITCH_REQID:
		{
			/* Reading mode value from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_TA_Switch,(const Tchar *)(pst_msg->data),(Tu16)sizeof(e_TA_Switch),&u32_DataSlotIndex);
			
			if(e_TA_Switch==AMFM_APP_TA_SWITCH_OFF || e_TA_Switch==AMFM_APP_TA_SWITCH_ON )
			{
				pst_me_amfm_app_inst->e_TA_Switch = e_TA_Switch;		/*updating instance hsms AF Switch parameter*/
				
				e_TASwitchReplyStatus    = REPLYSTATUS_SUCCESS;	
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] TA switch turned %d ",e_TA_Switch);
			}
			else
			{
				e_TASwitchReplyStatus    = REPLYSTATUS_FAILURE;
			}
			AMFM_App_Response_TA_Switch(e_TASwitchReplyStatus);
		}
		break;
		
		case AMFM_APP_FM_TO_DAB_SWITCH_REQID:
		{
			/* Reading FM2DAB Switch status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_FM_to_DAB_Switch,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_FM_To_DAB_Switch),&u32_DataSlotIndex);
			
			if(e_FM_to_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON  || e_FM_to_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_OFF )
			{
				pst_me_amfm_app_inst->e_FM_DAB_Switch = e_FM_to_DAB_Switch;		/*updating instance hsms FM-DAB Switch parameter*/
				
				e_FM_to_DAB_SwitchReplyStatus = REPLYSTATUS_SUCCESS;
			}
			else
			{
				e_FM_to_DAB_SwitchReplyStatus = REPLYSTATUS_FAILURE;
			}

			AMFM_App_Response_FM_to_DAB_Switch(e_FM_to_DAB_SwitchReplyStatus);
			
			if( e_FM_to_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] FM-DAB switch turned ON ");
			
				if(pst_me_amfm_app_inst->e_Processing_Status == AMFM_APP_FOREGROUND)
				{
					if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
					{
						if((pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u16_curr_PI!=0) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
						{
							/*Debug log for  notification to find SID*/
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] RDS switch turned on sending %x PI to DAB  to proceed for FM-DAB",pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
							/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
							AMFM_App_Notify_Find_SID(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						}
						else
						{
							/* No need to send notification if Current station PI is zero or AF switch is OFF */
						}
					}
					else
					{
						/* No need to send notification if Active Band is AM */
					}
				}
				else
				{
					/* Nothing to do. */
				}
			}
			else
			{
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] FM-DAB switch turned OFF ");
				/* FM-DAB switch turned OFF */
				if(pst_me_amfm_app_inst->e_Processing_Status == AMFM_APP_FOREGROUND)
				{
					if((pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM) &&
					   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
					{
						
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_NEW_REQUEST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_STATION_TUNED;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					else
					{
						/* no need to reinitialise if active mode is AM or Fm to Dab request itself not sent*/
					}
				}
				else
				{
					/* nothing to do */
				}
			}
		}
		break;
		
		case AMFM_APP_ENG_MODE_SWITCH_REQID:
		{
			/* Reading ENG mode Switch status from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->e_ENG_ModeSwitch,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_Eng_Mode_Switch),&u32_DataSlotIndex);
		}
		break;

		case AMFM_APP_STA_NOT_AVAIL_STRA_STATUS_NOTIFYID:
		{
			/* Reading StationNotAvailable strategy status from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_StationNotAvailStrategyStatus),&u32_DataSlotIndex);

			if(pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status == AMFM_APP_STATIONNOTAVAIL_STRATEGY_START)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  StaNotAvail Str Started ");
			}
			else if(pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status ==  AMFM_APP_STATIONNOTAVAIL_STRATEGY_END)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  StaNotAvail Str Stopped ");
			}
				
			if(pst_me_amfm_app_inst->e_Processing_Status == AMFM_APP_FOREGROUND)
			{
				
			}
			else if(pst_me_amfm_app_inst->e_Processing_Status == AMFM_APP_BACKGROUND)
			{
				if(pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status == AMFM_APP_STATIONNOTAVAIL_STRATEGY_START)
				{				
					/* Clearing DAB-FM Linking parameters */
					memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LinkingParam));

					/* Clearing AF structure parameters */
					memset(&st_AMFM_App_AFList_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AFList_Info));

					#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info = st_AMFM_App_AFList_Info;
					#endif
				}
				else if(pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status ==  AMFM_APP_STATIONNOTAVAIL_STRATEGY_END)
				{
					memset(&pst_me_amfm_app_inst->st_BG_AFtune_param,AMFM_APP_CONSTANT_ZERO,(Tu8)sizeof(Ts_AFM_App_BG_AFtune_param));

					if(pst_me_amfm_app_inst->b_BackgroungScanFlag == FALSE)
					{
						/* Transisting to active sleep state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);
					}
					else
					{
						/*MISRA C*/
					}
				}				
			}
			else
			{
					/* Nothing to do */
			}

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
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveStartHndlr		 */	
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveStartHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*							pst_ret = NULL;                                     /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu8									u8_switch_setting;
	Te_RADIO_ReplyStatus				e_StartupReplyStatus;                               /* enum variable to hold the  startup reply status*/
	Tu32								u32_DataSlotIndex = 0;			/* variable to update the message slot index */
	Tu32 								u32_NVM_Update_Param=0;
    Ts_Sys_Msg							*pst_StartDoneMsg = &st_amfm_app_res_msg;			/* pointer to message structure defined globally */
	Tu8 								u8_NVM_ret=0;
	Tu8									u8_NVM_Write=0;
	Te_AMFM_App_mode					e_mode		=AMFM_APP_MODE_FM;	
	Tbool								b_VerStartFreq;
	//Tbool								b_VerEndFreq;	
	Tu8									u8_LastFreqindex = 0;

	switch(pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveStartHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *) "amfm_app_inst_hsm_active_start_state" ,sizeof("amfm_app_inst_hsm_active_start_state"));
						
			/*setting default values at start up */
		
			pst_me_amfm_app_inst->u8_charset		=	AMFM_APP_CHARSET_EBU;
			st_fm_station_list.u8_charset			=	AMFM_APP_CHARSET_EBU;
					
			u8_switch_setting = pst_me_amfm_app_inst->u8_switch_setting;

			pst_me_amfm_app_inst->e_Anno_Cancel_Request = AMFM_APP_NO_ANNO_CANCEL;			
			
			if ((LIB_ISBITSET(u8_switch_setting, 0u)))
			{
				/*setting initial TA switch as ON*/
				pst_me_amfm_app_inst->e_TA_Switch		=AMFM_APP_TA_SWITCH_ON;
			}
			else
			{
				/*setting initial TA switch as OFF*/
				pst_me_amfm_app_inst->e_TA_Switch		=AMFM_APP_TA_SWITCH_OFF;
			}
			
			if ((LIB_ISBITSET(u8_switch_setting, 1u)))
			{
				/*setting initial FM-DAB switch as ON*/
				pst_me_amfm_app_inst->e_FM_DAB_Switch	=AMFM_APP_FM_TO_DAB_SWITCH_ON;
			}
			else
			{
				/*setting initial FM-DAB switch as OFF*/
				pst_me_amfm_app_inst->e_FM_DAB_Switch	=AMFM_APP_FM_TO_DAB_SWITCH_OFF;
			}
			
			if ((LIB_ISBITSET(u8_switch_setting, 2u)))
			{
				/*setting initial AF switch as ON*/
				pst_me_amfm_app_inst->e_AF_Switch		=AMFM_APP_AF_SWITCH_ON;
			}
			else
			{
				/*setting initial AF switch as OFF*/
				pst_me_amfm_app_inst->e_AF_Switch		=AMFM_APP_AF_SWITCH_OFF;
			}
#if 0
			if ((LIB_ISBITSET(u8_switch_setting, 3u)))
			{
				/*setting initial AF Regional switch as ON*/
				pst_me_amfm_app_inst->e_AF_REGIONAL_Switch		=AMFM_APP_AF_REGIONAL_SWITCH_ON;
			}
			else
			{
				/*setting initial AF Regional switch as OFF*/
				pst_me_amfm_app_inst->e_AF_REGIONAL_Switch		=AMFM_APP_AF_REGIONAL_SWITCH_OFF;
			}	
#endif
			pst_me_amfm_app_inst->e_AF_REGIONAL_Switch		=AMFM_APP_AF_REGIONAL_SWITCH_ON;
			
			#ifdef 	AMFM_APP_ENABLE_STARTUP_SCAN
			pst_me_amfm_app_inst->e_Scan_Type 	= 	AMFM_APP_STARTUP_SCAN;
			#else 
			pst_me_amfm_app_inst->e_Scan_Type 	= 	AMFM_APP_NON_STARTUP_SCAN;
			#endif
			
			/* assigning  basic frequency(start freq ,end freq and step size) info of AM and FM band as per market  */
			AMFM_APP_SetMarketFrequency(pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo );

			u32_start_freq = pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq;
			u32_StepSize   = pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StepSize;
			u32_End_Freq   =  pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_EndFreq;
			AMFM_App_StartUp_Initialisation(pst_me_amfm_app_inst);	
		
			if(pst_me_amfm_app_inst->u8_start_up_type == AMFM_APP_WARM_START_UP)
			{
				/* Reading Details of Last played FM station and learn memory  from NVM and storing into st_LSM_FM_Info(LSM memory) */
				u8_NVM_ret = Sys_NVM_Read(NVM_ID_TUNER_AMFM_APP,&(pst_me_amfm_app_inst->st_LSM_FM_Info),sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
				if(u8_NVM_ret == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM warm start up read success  ");

					b_VerStartFreq 	 = AMFM_APP_VerifyFrequency((pst_me_amfm_app_inst->ast_AF_Learn_mem[0].u32_frequency*10),&e_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
		
					u8_LastFreqindex   =  (Tu8)((u32_End_Freq - u32_start_freq)/u32_StepSize);
				
					b_VerStartFreq = AMFM_APP_VerifyFrequency((pst_me_amfm_app_inst->ast_AF_Learn_mem[u8_LastFreqindex].u32_frequency*10),&e_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
				
					if((b_VerStartFreq != TRUE) || (b_VerStartFreq != TRUE))
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Learn memory data corrupted  ");

						AMFM_App_Update_LM_Freq(&(pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo),&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
					}
					else
					{
						/*values are not corrupted */
					}
				}
				else
				{
					u8_NVM_Write=1;

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM warm start up read fails  ");

					/*In case of NVM read failure. Initialising learn memory Frequencies */
					AMFM_App_Update_LM_Freq(&(pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo),&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
				}
			}
			else
			{
				u8_NVM_Write=1;
				/*for cold start up. Initialising learn memory Frequencies */
				AMFM_App_Update_LM_Freq(&(pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo),&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
			}
			/* Request to Tuner ctrl in order to start up */
			AMFM_Tuner_Ctrl_Request_Startup((Te_AMFM_Tuner_Ctrl_Market)(pst_me_amfm_app_inst->e_MarketType));
		
			if((u8_NVM_Write == 1) && (b_NVM_ActiveUpdateSwitch == TRUE))	
			{
				/*code to store LSM data into NVM */
				u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
				if(u8_NVM_ret == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  ");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  ");
				}
			}
	
		}
		break;
		
		case AMFM_APP_STARTUP_DONE_RESID:
		{
			/* Reading Reply status value from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_StartupReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
		
            /* sending start done message from instance hsm to main hsm */
            AMFM_APP_SendMsgtoMainhsm(pst_StartDoneMsg,AMFM_APP_INST_HSM_START_DONE);
            
			/* Sending response to Radio Manager Application inorder to indicate startup is done */
			AMFM_App_Response_Startup(e_StartupReplyStatus);
			
			/* Transisting to active sleep state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);
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
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepHndlr		 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*							pst_ret = NULL;                              /* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_AMFM_App_mode					e_ReqMode;	                                 /* enum variable to hold the requested mode */
	Te_RADIO_ReplyStatus				e_SelectBandReplyStatus;                     /* enum variable to hold the  selectband reply status*/
	Tu32								u32_DataSlotIndex = 0;  /* variable to update the message slot index */

	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{ 
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveSleepHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ), (const void *)"amfm_app_inst_hsm_active_sleep_state",sizeof("amfm_app_inst_hsm_active_sleep_state"));
			
			pst_me_amfm_app_inst ->u16_af_Tuned_PI=0;

			/* Updating Processing status of AM/FM Band as Background */
			pst_me_amfm_app_inst ->e_Processing_Status = AMFM_APP_BACKGROUND;
			
			pst_me_amfm_app_inst->e_stationlist_mode = AMFM_APP_MODE_FM;

			/* Clearing DAB-FM Linking parameters */
			memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LinkingParam));
			
			/* Clearing AF structure parameters */
			memset(&st_AMFM_App_AFList_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AFList_Info));

			/* Clearing the timer flag */
			pst_me_amfm_app_inst->b_NEG_TimerStartCheck = FALSE;

			/* Initialising the charset value.This u8_DABFM_LinkingCharset variable is used only for DAB<=>FM linking */
			pst_me_amfm_app_inst->u8_DABFM_LinkingCharset = AMFM_APP_CHARSET_EBU;
			
			#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info = st_AMFM_App_AFList_Info;
			#endif
	
			/* Transisting to active sleep state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
		}
		break;
		
		case AMFM_APP_SELECT_BAND_REQID:
		{
			
			/* Reading mode from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_ReqMode,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_mode),&u32_DataSlotIndex);

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  Select band req for %d band received ",e_ReqMode);
	
			pst_me_amfm_app_inst->e_requested_mode = e_ReqMode ; 
			/* Request to Tuner ctrl for activating the mode  */
			AMFM_Tuner_Ctrl_Request_Activate((Te_AMFM_Tuner_Ctrl_Band)e_ReqMode);
		}
		break;

		case AMFM_APP_SELECT_BAND_DONE_RESID:
		{
			/* Reading Reply status value from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_SelectBandReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			pst_me_amfm_app_inst->e_current_mode  = pst_me_amfm_app_inst->e_requested_mode;

			/* Clearing NEG flag timer */
			pst_me_amfm_app_inst->b_NEG_TimerStartCheck			= FALSE;

			/* Sending response to Radio Manager Application inorder to indicate select band is done */
			AMFM_App_Response_SelectBand(e_SelectBandReplyStatus);
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  Select band response for %d band received ",pst_me_amfm_app_inst->e_current_mode);
	
			/* Transisting to active idle state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_state);
		}
		break;

		case AMFM_APP_FIND_BEST_PI_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  BG : Best PI received ");

			/* Clearing Hardlink buffer */
			memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_PIList));
			
			/* Reading PI list from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList,pst_msg->data,sizeof(Ts_AMFM_App_PIList),&u32_DataSlotIndex);

			/* Reading QualityMin from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin,pst_msg->data,sizeof(Tu8),&u32_DataSlotIndex);

			/* Reading QualityMax from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMax,pst_msg->data,sizeof(Tu8),&u32_DataSlotIndex);
				
			/* Reading Implicit SID from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid,pst_msg->data,sizeof(Tu32),&u32_DataSlotIndex);

			/* Reading Linking Type from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType,pst_msg->data,sizeof(Tu8),&u32_DataSlotIndex);

			#ifdef CALIBRATION_TOOL
				SYS_RADIO_MEMCPY(&st_Calibration_DAB2FM_linkParam,(const void *)(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam),sizeof(Ts_AMFM_App_LinkingParam));
			#endif

			if( 
				#ifdef AMFM_APP_ENABLE_BGSCANFLAG
				(pst_me_amfm_app_inst->b_BackgroungScanFlag == TRUE) && 
			    #endif  
			   (pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) )
			{	
				/* FM STL is completed.So transisting to linking state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_busy_linking_state);			
			}
			else
			{
				#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
				/* FM Scan is under progress.So transition to active_busy_linking_state will be done after completion of FM Scan at BackgroundProcessSTLHndlr */
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_Request_Received = TRUE;
				#else
				/* Nothing to do . Just for MISRA C*/
				#endif
			}			
		}
		break;
		case AMFM_APP_NON_RADIO_MODE_NOTIFYID:
		case AMFM_APP_STOP_DAB2FM_LINKING_NOTIFYID:
		{
			/* DAB service station is changed.So clearing DAB-FM Linking parameters */
			memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LinkingParam));
			/* Clearing AF structure parameters */
			memset(&st_AMFM_App_AFList_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AFList_Info));
			
			pst_me_amfm_app_inst->e_DAB2FM_Linking_status =RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED;
			/* Clearing the timer flag */
			pst_me_amfm_app_inst->b_NEG_TimerStartCheck = FALSE;
	
		}
		break;

		case AMFM_APP_AF_TUNE_REQID:
		{
			/* Reading PI  value for AF tune from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_BG_AFtune_param.u16_AFtune_PI,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Tu16),&u32_DataSlotIndex);

			
			if((pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON) && (pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON))
			{
				pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status = AMFM_APP_SEARCHING_OLD_LM;
					
				/* Transisting to active_sleep_aftune_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_sleep_aftune_state);			
			}
			else
			{
				memset(&pst_me_amfm_app_inst->st_BG_AFtune_param,AMFM_APP_CONSTANT_ZERO,(Tu8)sizeof(Ts_AFM_App_BG_AFtune_param));
				
				/* Sending failure response to RM as Setting switch is not turned ON  */
				AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG AF tune failure response sent to RM ");

				if(pst_me_amfm_app_inst->b_BackgroungScanFlag == FALSE)
				{	
				/* Transisting to active sleep state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);
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
			pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}

/*====================================================================================*/
/*		  Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepBackgroundProcessSTLHndlr  */
/*====================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepBackgroundProcessSTLHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg								   *pst_ret							=NULL;										/* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_RADIO_ReplyStatus						e_GetStationListReplyStatus		= REPLYSTATUS_REQ_NOT_HANDLED;					/* enum variable to hold the  getstationlist reply status*/
	Te_RADIO_SharedMemoryType					e_TunerNotifiedSharedMemoryType = SHAREDMEMORY_INVALID;							/* enum variable to hold the  sharedmemory type given from the tunercontrol  */
    Tu32										u32_DataSlotIndex               =0;					                        /* variable to update the message slot index */
	Te_RADIO_SharedMemoryType					e_SharedMemoryType				= SHAREDMEMORY_INVALID;							/* enum variable to hold the  sharedmemory type depending on the band */
//	Tu8 										u8_RetValue=0;
	Tbool										b_return_value					= FALSE;	
	Tu8										    u8_NVM_ret                      = 0;
	Tu32 									    u32_NVM_Update_Param            = 0;
	Te_RADIO_ReplyStatus		                e_CancelReplyStatus             = REPLYSTATUS_FAILURE;

	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveSleepBackgroundProcessSTLHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_sleep_background_process_stl_state",sizeof("amfm_app_inst_hsm_active_sleep_background_process_stl_state")) ;
				
			if(pst_me_amfm_app_inst -> e_stationlist_mode  == AMFM_APP_MODE_FM)
			{
				e_CurrentMode = AMFM_APP_MODE_FM;

			//	#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
				/* Initialising Background Scan Flag to Zero */
				pst_me_amfm_app_inst->b_BackgroungScanFlag = FALSE;
			//	#endif
				
			#ifdef 	AMFM_APP_ENABLE_STARTUP_SCAN

				/* Sending request to Tuner ctrl for updating FM stationlist */	
				AMFM_Tuner_Ctrl_Request_GetStationList((pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_EndFreq) , (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StepSize),(Te_AMFM_Tuner_Ctrl_Band)pst_me_amfm_app_inst->e_stationlist_mode,(Te_AMFM_Scan_Type)(pst_me_amfm_app_inst->e_Scan_Type));		
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG FM scan request to TC");

			#else
			
				/* Start timer to  new background stationlist  request */
				st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(AMFM_APP_BG_STL_UPDATE_INITIAL_DELAY,AMFM_APP_BACKGROUND_UPDATE_STL_REQID,RADIO_AM_FM_APP);
				if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP] BG Time 5 Failed to start %d message will not be posted  ",AMFM_APP_BACKGROUND_UPDATE_STL_REQID);
				}
				else
				{
				//	RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] BG Time 5 started successfully %d message will be posted  ",AMFM_APP_BACKGROUND_UPDATE_STL_REQID);
				}
				
			#endif						
				
			}	
		}
		break;

		case AMFM_APP_GET_STL_DONE_RESID:
		{
			/* Reading Reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_GetStationListReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			
			/* Sending response to Radio Manager Application provides reply status */
			AMFM_App_Response_GetStationList(e_GetStationListReplyStatus);

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  BG : FM STL uupdated response ");

			if(e_GetStationListReplyStatus != REPLYSTATUS_SUCCESS)		
			{
				if(pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status ==  AMFM_APP_STATIONNOTAVAIL_STRATEGY_START)
				{
					/* Sending failure response to RM as scan is failed  */
					AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG AF tune failure response sent to RM ");
					pst_me_amfm_app_inst->b_BackgroungScanFlag = FALSE;					
				}
			}
			
			#ifdef 	AMFM_APP_ENABLE_STARTUP_SCAN
				if(pst_me_amfm_app_inst->e_Scan_Type == AMFM_APP_STARTUP_SCAN )
				{
					/* Updating Startup scan is done */
					pst_me_amfm_app_inst->e_Scan_Type = AMFM_APP_NON_STARTUP_SCAN ;

					/* Reposting  asynchronous message  */
					SYS_SEND_MSG(&pst_me_amfm_app_inst->st_temp_msg_buffer);
				}
			#endif	
				
		}
		break;
	
		case AMFM_APP_STL_UPDATED_NOTIFYID:
		{
			/* Reading Reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_TunerNotifiedSharedMemoryType, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_SharedMemoryType), &u32_DataSlotIndex);
			
			if(pst_me_amfm_app_inst -> e_stationlist_mode  == AMFM_APP_MODE_FM)
			{
				b_return_value = AMFM_App_application_GenerateFMStationList(&st_fm_station_list,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0],pst_me_amfm_app_inst->e_MarketType);
				if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
				{
					/*code to store LSM data into NVM */
					u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
					if(u8_NVM_ret == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP] BG NVM write success during stationlist updation ");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP] BG NVM write fails during stationlist updation  ");
					}
				}
			
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  BG : FM STL is updated");
					
				/* Updating charset for stations present in the list */
				st_fm_station_list.u8_charset	=  pst_me_amfm_app_inst->u8_charset;

				#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
				pst_me_amfm_app_inst->b_BackgroungScanFlag = TRUE;
				#endif
				pst_me_amfm_app_inst->b_BackgroungScanFlag = FALSE;
				
				e_SharedMemoryType = FM_APP_RADIO_MNGR;
						
				/*Sending notification to radio manager to inform STL is generated */
				AMFM_App_Notify_STLUpdated(e_SharedMemoryType);	

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP] BG FM scan done & notified to RM ");


				if(pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status ==  AMFM_APP_STATIONNOTAVAIL_STRATEGY_START)
				{
					if(pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status == AMFM_APP_STRATEGY_SCAN_ONGOING)
					{
						pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status = AMFM_APP_SEARCHING_NEW_LM;

						/* Transisting to active_sleep_aftune_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_sleep_aftune_state);
					}
				}
				else
				{
				
					if( ( 
						#ifdef	AMFM_APP_ENABLE_BGSCANFLAG 
					    (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_Request_Received == TRUE) || 
						#endif
					    (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag ==TRUE)) && (pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) )
					{
						#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
					    pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_Request_Received = FALSE;
						#endif
					    pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag  = FALSE;
							
						/* FIND_BEST_PI Request also came and FM STL is completed.So transisting to linking state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_busy_linking_state);
					}
					else
					{	
						/* Start timer to  new background stationlist  request */
					    st_AMFM_TimerId.u32_AMFM_APP_TimerId =SYS_TIMER_START(AMFM_APP_BG_STL_UPDATE_DELAY,AMFM_APP_BACKGROUND_UPDATE_STL_REQID,RADIO_AM_FM_APP);
					    if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_APP_BACKGROUND_UPDATE_STL_REQID);
						}
						else
						{
						//	RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_APP_BACKGROUND_UPDATE_STL_REQID);
						}
					}	
				}	
			}			
		}
		break;

		case AMFM_APP_BACKGROUND_UPDATE_STL_REQID:
		{
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_AMFM_APP_TimerId = 0;
			
			if(pst_me_amfm_app_inst -> e_stationlist_mode  == AMFM_APP_MODE_FM)
			{
				e_CurrentMode = AMFM_APP_MODE_FM;

				#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
				/* Initialising Background Scan Flag to Zero so that FM Scan will not cancelled
				   if FIND_BEST_PI request comes meanwhile */
				pst_me_amfm_app_inst->b_BackgroungScanFlag = FALSE;
				#endif	
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG FM scan request to TC");

				pst_me_amfm_app_inst->b_BackgroungScanFlag = TRUE;
				/* Sending request to Tuner ctrl for updating FM STL */
				AMFM_Tuner_Ctrl_Request_GetStationList((pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq), (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_EndFreq) , (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StepSize),(Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_stationlist_mode),(Te_AMFM_Scan_Type)(pst_me_amfm_app_inst->e_Scan_Type));
			}
		}
		break;

#ifdef 	AMFM_APP_ENABLE_STARTUP_SCAN

		case AMFM_APP_SELECT_BAND_REQID:
		{
			if ((pst_me_amfm_app_inst->b_MsgPendingFlag == FALSE) && (pst_me_amfm_app_inst->e_Scan_Type == AMFM_APP_STARTUP_SCAN))
			{
				pst_me_amfm_app_inst->b_MsgPendingFlag = TRUE;

				/* Clearing the temp msg buffer */
				memset((void *)(&pst_me_amfm_app_inst->st_temp_msg_buffer),AMFM_APP_NO_STATIONS_FM_BAND,sizeof(Ts_Sys_Msg));
				
				/* Copying select band msg to temp buffer in order re post it once after completion of Start up scan */
				SYS_RADIO_MEMCPY((&pst_me_amfm_app_inst->st_temp_msg_buffer),(const void *)(pst_msg),sizeof(Ts_Sys_Msg));
				//memcpy((void *)(&pst_me_amfm_app_inst->st_temp_msg_buffer),(const void *)(pst_msg),sizeof(Ts_Sys_Msg));
			}
			else if (pst_me_amfm_app_inst->b_MsgPendingFlag == TRUE)
			{	
				pst_me_amfm_app_inst->b_MsgPendingFlag = FALSE;

				/* Returning msg pointer so that msg will be handled in Parent state */
				pst_ret = pst_msg;
			}
			else
			{	
				/* Returning msg pointer so that msg will be handled in Parent state */
				pst_ret = pst_msg;
			}
			
		}
		break;
#endif		
		case AMFM_APP_AF_TUNE_REQID:
		case AMFM_APP_FIND_BEST_PI_REQID:
		case AMFM_APP_SELECT_BAND_REQID:
		{
			if(pst_me_amfm_app_inst->b_BackgroungScanFlag == TRUE)
			{
				/* Clearing the temp msg buffer */
				memset((void *)(&pst_me_amfm_app_inst->st_tmp_msg_buffer),0,sizeof(Ts_Sys_Msg));
				
				/* Copying af tune  msg to temp buffer in order re post it once after completion of scan cancel */
				SYS_RADIO_MEMCPY((&pst_me_amfm_app_inst->st_tmp_msg_buffer),(const void *)(pst_msg),sizeof(Ts_Sys_Msg));
				
				AMFM_Tuner_Ctrl_Request_Cancel();
			}	
			else
			{
				/* AF tune will be handled in parent state as Scan is not progress */
				pst_ret = pst_msg;
			}
		}
		break;
		
		case AMFM_APP_CANCEL_DONE_RESID:
		{
			/* Reading einfo  from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_CancelReplyStatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo), &u32_DataSlotIndex);
			
			if(e_CancelReplyStatus == REPLYSTATUS_SUCCESS)
			{
				pst_me_amfm_app_inst->b_BackgroungScanFlag = FALSE;
				
				SYS_RADIO_MEMCPY((pst_msg),(const void *)(&pst_me_amfm_app_inst->st_tmp_msg_buffer),sizeof(Ts_Sys_Msg));
				
				pst_ret = pst_msg;
			}
			else
			{
				
			}
		}
		break; 
		case HSM_MSGID_EXIT:
		{	
			/*  free all timers */
			if(st_AMFM_TimerId.u32_AMFM_APP_TimerId > 0)
			{
				b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_AMFM_APP_TimerId);
				if(b_return_value == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 5 failed to stop message will be posted");
				}
				else
				{
					/* After stop Timer is success,reset the timer id */
					st_AMFM_TimerId.u32_AMFM_APP_TimerId = 0;
			//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 5 successfully stopped  coming out  of background scan  ");
				}
			}
			else
			{
				/*MISRA C*/	
			}
			
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
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepAFTuneHndlr		 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveSleepAFTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret 							= NULL;            						 /* Pointer to be updated to the  msg pst_msg  at default case*/			      	
	Te_RADIO_ReplyStatus		            e_AMFM_APP_AF_UpdateReplystatus	    = REPLYSTATUS_REQ_NOT_HANDLED; 	     /* enum variable to hold the  AF update reply status*/
	Te_RADIO_ReplyStatus		            e_FM_CheckReplystatus 				= REPLYSTATUS_FAILURE;
	Te_RADIO_ReplyStatus	                e_PlaySelectStationReplyStatus  	= REPLYSTATUS_REQ_NOT_HANDLED;  /* enum variable to hold the  Play select station reply status*/
	Te_RADIO_ReplyStatus 		            e_AFTune_ReplyStatus 				= REPLYSTATUS_FAILURE;
	Tu32							    	u32_NVM_Update_Param                = 0;
	Tu32								    u32_DataSlotIndex					= 0;                                      /* variable to update the message slot index */
	Tu32								    u32_AFupdateFreq                    = 0;
	Tu16							 	    u16_afpi 							= 0;
	Ts8									    s8_RetVal                           = 0;
	Tu8									    u8_RetValue;
	Tu8									    u8_AF_index 						= 0;
	Tu8 									u8_Best_AF_station_Index			= 0;
	Tu8									    u8_NVM_ret							= 0;

	switch(pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveSleepAFTuneHndlr");

			/* Updating the current state name  used for debugging purpose */ 
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name),(const void *)"amfm_app_inst_hsm_active_sleep_aftune_state",sizeof("amfm_app_inst_hsm_active_sleep_aftune_state"));

			/* Clearing AF structure parameters */
			memset(&st_AMFM_App_AFList_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AFList_Info));

			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info = st_AMFM_App_AFList_Info;
			#endif

			memset(&pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_StationInfo));

			/* Generating the AF list for DAB strategy */
			AMFM_APP_BG_Generate_AF_list_from_LM(pst_me_amfm_app_inst->st_BG_AFtune_param.u16_AFtune_PI,
												      &pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo,
												      &st_AMFM_App_AFList_Info,
     													 pst_me_amfm_app_inst->ast_AF_Learn_mem);

			if(st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList > 0)
			{
				u8_AF_index = st_AMFM_App_AFList_Info.u8_AF_index;
				
				AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_index].u32_AF_Freq);		
			}
			else
			{
				if(pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status == AMFM_APP_SEARCHING_OLD_LM)
				{
					pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status = AMFM_APP_STRATEGY_SCAN_ONGOING;

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG : AF not found in existing LM so scan started");

					/* As alternate is not available in existing LM,needs to do BG FM scan.So transisting to active_sleep_background_process_stl_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
				}
				else if(pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status == AMFM_APP_SEARCHING_NEW_LM)
				{
					pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status  = AMFM_APP_BG_FM_AF_STRA_FAIL;
					
					/* Sending failure response to RM as alternate is not found even after updating LM */
					AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG AF tune failure response sent to RM ");
				}
				else
				{
					/* Nothing to do */
				}
			}
		}
		break;

		case AMFM_APP_AFFREQ_UPDATE_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_UpdateReplystatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u32_AFupdateFreq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);

			u8_AF_index = st_AMFM_App_AFList_Info.u8_AF_index;

			if(st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_index].u32_AF_Freq == u32_AFupdateFreq)
			{
				if(e_AMFM_APP_AF_UpdateReplystatus == REPLYSTATUS_SUCCESS)
				{ 	
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  BG : AF update Response success %d",st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_index].u32_AF_Freq);

					pst_me_amfm_app_inst->st_QualParam.u32_interpolation = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset);
					
					st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_index].u32_avg_qual = pst_me_amfm_app_inst->st_QualParam.u32_interpolation;

					#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_index].u32_avg_qual = pst_me_amfm_app_inst->st_QualParam.u32_interpolation;
					#endif	
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  BG : AF update Response Fail %d",st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_index].u32_AF_Freq);
				}
				
				/*start timer to  new update request */
				st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(10, AMFM_APP_BG_AF_TUNE_UPDATE_TIMERID,RADIO_AM_FM_APP);			

				if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_APP_BG_AF_TUNE_UPDATE_TIMERID);
				}
				else
				{
					//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_APP_BG_AF_TUNE_UPDATE_TIMERID);
				}	
			}	
			else
			{
				/* AF update response is for other frequency.So dropping this message and waiting for correct response */
			}
		}
		break;

		case AMFM_APP_BG_AF_TUNE_UPDATE_TIMERID:
		{
			st_AMFM_TimerId.u32_AMFM_APP_TimerId =0;
			/*Timer started after delay 10ms */
			u8_RetValue = AMFM_APP_AF_Update(&st_AMFM_App_AFList_Info);

			if(u8_RetValue == 1)
			{
				/* AF update is given for all frequencies in the list.So sorting the list in decsencing order as per quality */
				AMFM_App_Sort_SAME_PI_AF_List(&st_AMFM_App_AFList_Info);

				AMFM_Tuner_Ctrl_Request_FM_Check(st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FM Check request given for  freq %d ",st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
			}
			else
			{
				/* AF update request is given to AMFM TC */
			}
		}
		break;

		case AMFM_APP_AFFREQ_CHECK_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_FM_CheckReplystatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&u16_afpi,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);

			u8_Best_AF_station_Index = st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;
			
			if(e_FM_CheckReplystatus == REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FM Check response succes for  freq %d, PI =%d",st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,u16_afpi);

				if(u16_afpi == pst_me_amfm_app_inst->st_BG_AFtune_param.u16_AFtune_PI)
				{
					AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FM Jump  request given for  freq %d",st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
				}
				else if(u16_afpi != pst_me_amfm_app_inst->st_BG_AFtune_param.u16_AFtune_PI)
				{
					if(u16_afpi != 0)
					{
						if(u8_Best_AF_station_Index < st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList - 1)
						{
							/* FM check request should be given to next frequency in the list.so incrementing index value */
							u8_Best_AF_station_Index = ++(st_AMFM_App_AFList_Info.u8_Best_AF_station_Index);

						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index = AMFM_App_Calib_get_AF_index(&st_AMFM_App_AFList_Info,st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList,u8_Best_AF_station_Index);
						#endif	
							
	
							AMFM_Tuner_Ctrl_Request_FM_Check(st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);

							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FM Check request given for  freq %d ",st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
						}
						else
						{
							if(pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status == AMFM_APP_SEARCHING_OLD_LM)
							{
								pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status = AMFM_APP_STRATEGY_SCAN_ONGOING;

								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG : AF not found in existing LM so scan started");

								/* As alternate is not available in existing LM,needs to do BG FM scan.So transisting to active_sleep_background_process_stl_state */
								HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
							}
							else if(pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status == AMFM_APP_SEARCHING_NEW_LM)
							{
								pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status  = AMFM_APP_BG_FM_AF_STRA_FAIL;

								/* Sending failure response to RM as alternate is not found even after updating LM */
								AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info);

								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG AF tune failure response sent to RM ");
							}
							else
							{
								/* Nothing to do */
							}		
						}
					}
					else
					{
						if(pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status == AMFM_APP_SEARCHING_OLD_LM)
						{
							pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status = AMFM_APP_STRATEGY_SCAN_ONGOING;
												
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG : AF not found in existing LM so scan started");

							/* As alternate is not available in existing LM,needs to do BG FM scan.So transisting to active_sleep_background_process_stl_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
						}
						else if(pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status == AMFM_APP_SEARCHING_NEW_LM)
						{
							pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status  = AMFM_APP_BG_FM_AF_STRA_FAIL;
												
							/* Sending failure response to RM as alternate is not found even after updating LM */
							AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info);

							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG AF tune failure response sent to RM ");
						}
						else
						{
							/* Nothing to do */
						}						
					}
				}
				else
				{
					
				}
			}

			if(u16_afpi != 0)
			{
				u8_RetValue 	= AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);

				if((u8_RetValue == 1) && (b_NVM_ActiveUpdateSwitch == TRUE) )
				{
					/*code to store LSM data into NVM */
					u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);	

					if(u8_NVM_ret == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success  during AF check in Sleep AF tune state");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during AF check in Sleep AF tune state");
					}
				}
			}
		}
		break;

		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{
			/* Reading reply status */
			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);

			/* Reading details of currently tuned station info*/
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);	

			u8_Best_AF_station_Index = st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;

			if(e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FM jump response succes for  freq %d, PI =%d",st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,u16_afpi);

				e_AFTune_ReplyStatus = REPLYSTATUS_SUCCESS;
	
				pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info.e_mode = AMFM_APP_MODE_FM;

				pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info.un_station.st_FMstation.u32_frequency = st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq;

				pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info.un_station.st_FMstation.u16_PI = st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_Best_AF_station_Index].u16_PI;

				/* Function to read quality of currently tuned station */
				AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);

				pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info.u8_charset	= pst_me_amfm_app_inst->u8_charset;

				if((pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON) && (pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON))
				{
					pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status  = AMFM_APP_BG_FM_AF_STRA_SUCCESS;
					
					AMFM_App_Response_AFTune(e_AFTune_ReplyStatus,pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info);
				}
				else
				{
					pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status  = AMFM_APP_BG_FM_AF_STRA_FAIL;
												
					/* Sending failure response to RM as alternate is not found even after updating LM */
					AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG AF tune failure response sent to RM due to Setting switch is OFF ");
				}
			}
			else
			{
				/* Nothing to do. Because AMFM TC is sending Succes response only for FM jump*/
			}
		}
		break;

		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
	       	AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_QualParam),(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);
 
			st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
	
			AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

			if(st_AMFM_App_AFList_Info.u32_Qua_avg < AMFM_APP_BG_AF_TUNE_THRESHOLD_VALUE)
			{
				pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status  = AMFM_APP_BG_FM_AF_STRA_FAIL;
									
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG : StaNotAvail Strategy ended due to low signal ");

				/* Notifying signal lost to RM.Hence StationNotAvailable Strategy ends  */
				AMFM_App_Notify_AF_SigLost();	
			}		
		}
		break;

		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			memset(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo));
			
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);


			st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength , pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);
				
			AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

			s8_RetVal =  strncmp((char *)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,(char *)pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info.un_station.st_FMstation.au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME) ;

			if((st_AMFM_App_AFList_Info.u32_Qua_avg < AMFM_APP_BG_AF_TUNE_THRESHOLD_VALUE) || 
				(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info.un_station.st_FMstation.u16_PI))
			{
				pst_me_amfm_app_inst->st_BG_AFtune_param.e_BG_AF_Tune_Status  = AMFM_APP_BG_FM_AF_STRA_FAIL;

				/* Notifying signal lost to RM.Hence StationNotAvailable Strategy ends  */
				AMFM_App_Notify_AF_SigLost();	

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] BG : StaNotAvail Strategy ended due to low signal ");

			}
			else if(s8_RetVal != 0)
			{
				SYS_RADIO_MEMCPY((void *)		pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info.un_station.st_FMstation.au8_psn,
									(const void *)	pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,
												MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
				/* PSN is changed.Notifying to RM */
				AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_BG_AFtune_param.st_BG_AF_TunedStation_Info,AMFM_APP_GOOD_QUALITY_SIGNAL);
			}				
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
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleHndlr		 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret						 = NULL;									 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu8 									u8_currentTA;
	Tu8 									u8_currentTP;
	Tu16 									u16_EON_PI;
	Tbool 									b_PI_availability;
    Tu32									u32_DataSlotIndex			 = 0;										 /* variable to update the message slot index */
	Tu32									u32_freq					 = 0;
	Te_RADIO_ReplyStatus		 			e_AFTune_ReplyStatus;
	Te_RADIO_ReplyStatus					e_DeSelectBandReplyStatus	 = REPLYSTATUS_REQ_NOT_HANDLED;				 /* enum variable to hold the  Deselect band reply status*/
	Te_AMFM_App_mode						e_ReqMode					 = AMFM_APP_MODE_INVALID;                    /* enum variable to hold the requested mode */
	Te_RADIO_ReplyStatus					e_GetCT_InfoReplystatus		 = REPLYSTATUS_FAILURE;						 /* enum variable to hold the get CT info reply status */
	Tbool									b_return_value= FALSE;
	Tu8										u8_NVM_ret=0;
	Tu32									u32_NVM_Update_Param=0;
	//Tu8									u8_RetValue;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveIdleHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_state",sizeof("amfm_app_inst_hsm_active_idle_state"));
		
			/* Updating Processing status of AM/FM Band as Foreground */
			pst_me_amfm_app_inst ->e_Processing_Status = AMFM_APP_FOREGROUND;
		}
		break;

		case AMFM_APP_SELECT_STATION_REQID:
		{
			/* Reading Station list index  from the message */
			AMFM_APP_ExtractParameterFromMessage(&u32_freq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu32),&u32_DataSlotIndex);
			
            pst_me_amfm_app_inst->u32_StartFrequency= u32_freq;

			/* Reading Reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_ReqMode,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_mode),&u32_DataSlotIndex);
			
			pst_me_amfm_app_inst->e_requested_mode = e_ReqMode;

			/* Transisting to active busy direct tune state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_select_station_state);
		}
		break;
		
		case AMFM_APP_SEEK_UP_DOWN_REQID:
		{
            
            /* Reading e_direction value from the message */
            AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->u32_StartFrequency),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu32),&u32_DataSlotIndex);
			
			/* Reading e_direction value from the message */
			AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->e_SeekDirection), (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_DirectionType), &u32_DataSlotIndex);
			
			/* Transisting to active busy seek state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_seek_state);
		}
		break;
		
		case AMFM_APP_TUNE_UP_DOWN_REQID:
		{
			/* Reading e_direction value from the message */
			AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_TuneUpDownParam.e_TuneUpDown_Direction), (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_DirectionType), &u32_DataSlotIndex);
			
			/* Reading Step value from the message */
			AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_TuneUpDownParam.u32_No_of_Steps),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu32),&u32_DataSlotIndex);
			
			/* Transisting to active busy TuneUpDown state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_tune_up_down_state);
		}
		break;		
			
		case AMFM_APP_DESELECT_BAND_REQID:
		{
			/* Reading Reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_ReqMode,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_mode),&u32_DataSlotIndex);
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  Deselect band req for %d band received from RM ",e_ReqMode);
			/* Sending request to Tuner ctrl for deactivating the mode */
			AMFM_Tuner_Ctrl_Request_DeActivate((Te_AMFM_Tuner_Ctrl_Band)e_ReqMode);
		
		}
		break;

		case AMFM_APP_DESELECT_BAND_DONE_RESID:
		{
			/* Reading Reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_DeSelectBandReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			pst_me_amfm_app_inst->e_current_mode  = AMFM_APP_MODE_INVALID;		// Neither  AM nor FM is active 

			#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
			/* Initialising Background Scan Flag to Zero */
			pst_me_amfm_app_inst->b_BackgroungScanFlag = FALSE;
			#endif			
			
			/* Sending response to Radio Manager Application to indicate band is deactivated */
			AMFM_App_Response_DeSelectBand(e_DeSelectBandReplyStatus);

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  DeSelect band res for %d band received from TC",e_ReqMode);
			/* Transisting to active sleep state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);
		}
		break;

		case AMFM_APP_GET_STL_REQID: 
		{
			/* Reading mode value from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_ReqMode,(const Tchar *)(pst_msg->data),(Tu8)sizeof(Te_AMFM_App_mode),&u32_DataSlotIndex);
			
			pst_me_amfm_app_inst->e_stationlist_mode = e_ReqMode;

			e_CurrentMode = e_ReqMode;
			
			
			/* Transisting to active process stl state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_processSTL_state);
		}
		break;
		
		case AMFM_APP_DAB_FOLLOWUP_NOTIFYID:
		{
			/* Reading mode value from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality,(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE,(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE),&u32_DataSlotIndex);

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] DAB station info received");

		}
		break;
		
		case AMFM_APP_AF_TUNE_REQID:
		{
			/* Reading mode value from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->u16_curr_station_pi,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
		
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] AF Tune request Processing");

			if(pst_me_amfm_app_inst->u16_curr_station_pi != pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI)
			{
				/* LSM differs with the requested PI*/
				/* AF tune should be proceeded for the requested PI. Form AF_List for the new diff PI from Learn Memory*/
				pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI = pst_me_amfm_app_inst->u16_curr_station_pi;
				/*Function to clear old AF frequencies alone*/
				AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
			}
		
			pst_me_amfm_app_inst->u8_aflistindex 									= 0;
			pst_me_amfm_app_inst->u32_ReqFrequency									= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
			pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI = pst_me_amfm_app_inst->u16_curr_station_pi;
			pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality 			= AMFM_APP_CONSTANT_ZERO;
			pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE 		= AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
			pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status		= AMFM_APP_FM_STATION_TUNED;
			st_AMFM_App_AFList_Info.u16_curr_PI										= pst_me_amfm_app_inst->u16_curr_station_pi	 ;
			st_AMFM_App_AFList_Info.u32_curr_freq 									= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
			st_AMFM_App_AFList_Info.u8_AF_index  = 0;
			st_AMFM_App_AFList_Info.u8_Best_AF_station_Index= 0 ;
			st_AMFM_App_AFList_Info.b_af_check_flag=FALSE;
			u8_af_update_cycle=0;
			#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info = st_AMFM_App_AFList_Info;
			#endif
				
		

			if(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList.u8_NumAFList!=0)
			{
				b_return_value = AMFM_App_AF_List_frequency_Append(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList.au32_AFList,pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList.u8_NumAFList,0,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst);
				/*frequencies are appended from LSM only so no need to write to NVM*/
				if(b_return_value==TRUE)
				{
					b_return_value=FALSE;
				}
			}

			b_return_value = AMFM_App_AF_List_Append_From_learn_Memory(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info = st_AMFM_App_AFList_Info;
			#endif
			
			if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch == TRUE))
			{
				/*code to store LSM data into NVM */
				u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) +(sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
				if(u8_NVM_ret == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during AFTune request,Af list appended from learn memory ");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during AFTune request ");
				}
			}

			if(st_AMFM_App_AFList_Info.u8_NumAFList!=0)  
			{
				st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_AF_TUNE_REQ_RECEIVED;
					
				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
				#endif
				/* Transiting to amfm_app_inst_hsm_active_idle_af_strategy_tune_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_strategy_tune_state);
			}
			else
			{
				e_AFTune_ReplyStatus = REPLYSTATUS_FAILURE;
				/* Sending Response to Radio Mngr */
				AMFM_App_Response_AFTune(e_AFTune_ReplyStatus,pst_me_amfm_app_inst->st_current_station);
			
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] AF Tune request failed due to 0 AF Frequencies starting scan ");
				
				st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_AF_TUNE_SCAN_STARTED;
				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
				#endif
				
				pst_me_amfm_app_inst->e_stationlist_mode = AMFM_APP_MODE_FM;
				e_CurrentMode = AMFM_APP_MODE_FM;
				/* Transisting to active process stl state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_processSTL_state);
			}
			
		}
		break;
		
		case AMFM_APP_CURRENT_STATION_TA_TP_NOTIFIED:
		{
			AMFM_APP_ExtractParameterFromMessage(&u8_currentTA,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu8),&u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u8_currentTP,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu8),&u32_DataSlotIndex);	

			pst_me_amfm_app_inst->st_current_station.u8_TA	= u8_currentTA;

			pst_me_amfm_app_inst->st_current_station.u8_TP	= u8_currentTP;
			
			if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
			{
				/* Notifying st_current_station Info as there is change in TA/TP flag */
				AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);	
			}
			if((u8_currentTA== 1) && (u8_currentTP==1) && (pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_ON) && (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON)
			   &&(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status != AMFM_APP_FM_DAB_LINKED))
			{
				AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_START);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcement started  ");
				st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_ANNOUNCEMENT_HANDLING;
				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
				#endif

				/*Temporarily disable announcement*/
#if 0				
				/* Transiting  to active idle program station announcement state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_program_station_announcement_state);
#endif

			}
			else
			{
				/* do nothing */
			}
		}
		break;

		case AMFM_APP_EON_INFO_NOTIFIED:
		{
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info,(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info),&u32_DataSlotIndex);
			if(pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info.u16_EON_PI != 0)  
			{
				
				if ((pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[0] == 0) && (pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[1] == 0))
				{
					b_PI_availability = AMFM_APP_Check_PI_Availability(&(pst_me_amfm_app_inst->st_EON_List),pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info.u16_EON_PI);

					if(b_PI_availability == TRUE)
					{
						/*do nothing*/
						/*Eon is already updated with the PI received*/	
					}
					else
					{
						/*code to add frequencies from Learnmemory*/
						AMFM_APP_Adding_PIFrequency_From_LearnMemory(pst_me_amfm_app_inst,pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info.u16_EON_PI,&st_AMFM_App_AFList_Info);
					}
				}
				else
				{
					AMFM_APP_Update_EON_List(&(pst_me_amfm_app_inst->st_EON_List),pst_me_amfm_app_inst->st_AMFM_TunerCtrl_Eon_Info);
				} 
			}
			else
			{
				/*do nothing*/
			}
	
		}
		break;
		
		case AMFM_APP_EON_ANNOUNCEMENT_START_NOTIFIED:
		{
			AMFM_APP_ExtractParameterFromMessage(&u16_EON_PI,(const Tchar *)(pst_msg->data),(Tu16)sizeof(u16_EON_PI),&u32_DataSlotIndex);
		
			if((pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_ON) && (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON) &&  
			   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status != AMFM_APP_FM_DAB_LINKED) &&
			   (((pst_me_amfm_app_inst->st_current_station.u8_TA == 0) && (pst_me_amfm_app_inst->st_current_station.u8_TP == 1) ) ||
			   	((pst_me_amfm_app_inst->st_current_station.u8_TA == 1) && (pst_me_amfm_app_inst->st_current_station.u8_TP == 0) ) ) ) 
			{
				pst_me_amfm_app_inst->u16_EON_TA_Station_PI = u16_EON_PI;

				/*Temporarily disable announcement*/
#if 0				
				AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_START);
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] EON Announcement start notification received for %x PI station  ",u16_EON_PI);
#endif
				
				
				/* Clearing st_EON_station_Info structure before updating into it */
				memset((void *)&pst_me_amfm_app_inst->st_EON_station_Info,AMFM_APP_CONSTANT_ZERO,sizeof(pst_me_amfm_app_inst->st_EON_station_Info));
			
				AMFM_App_clear_PreviousEON_Qualities(&pst_me_amfm_app_inst->st_EON_List);
				st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_ANNOUNCEMENT_HANDLING;
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
				#endif
					/*Temporarily disable announcement*/
#if 0
				/* Transiting  to ative idle eon station announcement state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_eon_station_announcement_state);
#endif
			}
			else
			{
				/*Do nothing */
			}
		
		}
		break;

		case AMFM_APP_GET_CT_INFO_REQID:
		{
			e_GetCT_InfoReplystatus = AMFM_App_Compute_CT_Info(&pst_me_amfm_app_inst->st_RDS_CT_Info,&pst_me_amfm_app_inst->st_CT_Info);
			/* Sending Response to Radio Mngr */
			AMFM_App_Response_CT_Info(e_GetCT_InfoReplystatus,pst_me_amfm_app_inst->st_CT_Info);
				
			
		}
		break;
		
		case AMFM_AF_STATUS_CHECK_NOTIFYID:
		{
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_Status_Check_TimerId = 0;
			/*increment the counts of NEG flag*/
			AMFM_App_AF_StatusCountIncrementation(&st_AMFM_App_AFList_Info);
			
			/* Reset the AF's whose count reached the NEGtimeout*/
			AMFM_App_AF_NEG_StatusReset(&st_AMFM_App_AFList_Info);
			
			/* check for NEG availability */
			b_return_value = AMFM_App_NEG_flag_status(&st_AMFM_App_AFList_Info);	
			if(b_return_value == TRUE)
			{
				/*NEG flag set start the timer*/
				pst_me_amfm_app_inst->b_NEG_TimerStartCheck = TRUE;
				st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
				if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
				}
				else
				{
				//	RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
				}
			}
			else
			{
				pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
				/*Timer wont be started again,can close it*/
				if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
				{
					b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
					if(b_return_value == FALSE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
					}
					else
					{
						/* After stop Timer is success,reset the timer id */
					    st_AMFM_TimerId.u32_Status_Check_TimerId =0;
			//			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
					}	
				}
				else
				{
					/*MISRAC*/
				}
			}
			
		}
		break;

		case AMFM_APP_START_AF_STRATEGY_NOTIFYID:
		{
			pst_me_amfm_app_inst->b_StartAF_Flag = TRUE; 
		}
		break;
		
		case HSM_MSGID_EXIT:
		{
			if( (pst_me_amfm_app_inst->st_current_station.e_mode == AMFM_APP_MODE_FM) && (pst_me_amfm_app_inst->b_StartAF_Flag == FALSE) )
			{
				/*  free all timers */
				if(st_AMFM_TimerId.u32_AMFM_APP_TimerId >0)
				{
					b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_AMFM_APP_TimerId);
				
					if(b_return_value == FALSE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Timer 5 failed to stop message will be posted");
					}
					else
					{
						st_AMFM_TimerId.u32_AMFM_APP_TimerId =0;
						//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 5 successfully stopped  coming out  of active idle  ");
					}
				}
				else
				{
					/*MISRAC*/
				}
			}
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
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleListenHndlr	 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleListenHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg								*pst_ret				= NULL;             /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex       = 0;                /* variable to update the message slot index */
	Tbool 									b_return_value			= FALSE;
	Tbool									b_PSNorRT_Changeflag    = FALSE;
	Tu8										u8_NVM_ret = 0;
	Tu8										u8_NVM_Write=0;
	Tu8										u8_af_appendCheck = 0;
	Tu32 									u32_NVM_Update_Param=0;
	Tbool									b_retBool = FALSE;
	Tbool								    b_NotifySTL_Flag = FALSE ;	

	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveIdleListenHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_listen_state",sizeof("amfm_app_inst_hsm_active_idle_listen_state"));
			
			pst_me_amfm_app_inst->b_CurStationPICpy = TRUE;
			/* Enabling the flag in order to update PSN of curr station in FM STL only once */
			pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL = TRUE;
				
			pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
		
			u8_af_update_cycle=0;
		
			/*update with freq,PI and AFlist of curr station info */
			if(pst_me_amfm_app_inst->st_current_station.e_mode == AMFM_APP_MODE_FM)
			{
				/*New strategy*/
				st_AMFM_App_AFList_Info.u32_curr_freq	= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ;
				st_AMFM_App_AFList_Info.u16_curr_PI		= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;
				st_AMFM_App_AFList_Info.e_curr_status	= AMFM_APP_LISTEN_FM_STATION;
				
				/* This variable is used to send AM/FM stationlist RESID/NOTIFIED to RM */
				e_CurrentMode = AMFM_APP_MODE_FM;

				#ifdef CALIBRATION_TOOL
					b_start_stop_switch                         = FALSE ;
					u32_calib_freq                              = 0;
					st_Calib_AMFM_App_AFList_Info.u32_curr_freq = st_AMFM_App_AFList_Info.u32_curr_freq;
					st_Calib_AMFM_App_AFList_Info.u16_curr_PI   = st_AMFM_App_AFList_Info.u16_curr_PI;
					st_Calib_AMFM_App_AFList_Info.e_curr_status	= AMFM_APP_LISTEN_FM_STATION;

				#endif
				
				/* Checking whether newly tuned FM Station present in the FM STL */
				pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);

				SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/

				if(-1 == pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL)
				{
					if(st_fm_station_list.u8_NumberOfStationsInList < AMFM_APP_MAX_FM_STL_SIZE)
					{
						/* Newly tuned FM station is not present in FM STL.So appending it into list */
						st_fm_station_list.ast_Stations[st_fm_station_list.u8_NumberOfStationsInList].u32_frequency = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
						
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is updated due to new FM station %d found.Sorted and Notified to RM",st_fm_station_list.ast_Stations[st_fm_station_list.u8_NumberOfStationsInList].u32_frequency );
						
						pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = st_fm_station_list.u8_NumberOfStationsInList++;

					}
					else
					{
						/* There is no space to add newlt tuned FM staion in FM STL.So Clearing last station in FM STL */
						memset(&st_fm_station_list.ast_Stations[AMFM_APP_MAX_FM_STL_SIZE-1],AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_FMStationInfo));

						pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_APP_MAX_FM_STL_SIZE-1;
						
						/* 	Appending currently tuned station info at last index in FM STL */
						st_fm_station_list.ast_Stations[AMFM_APP_MAX_FM_STL_SIZE-1].u32_frequency = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;			

						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is full. New FM station %d is added at last index.Sorted and Notified to RM,st_fm_station_list.ast_Stations[st_fm_station_list.u8_NumberOfStationsInList].u32_frequency ");
					}

					if(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI != 0)
					{
						b_retBool = AMFM_App_Remove_SamePIfromFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI,&st_fm_station_list);

						if(b_retBool == TRUE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is updated as same PI station is removed.");
	
							/* As FM STL is sorted, index value of current station in FM STL might have changed. */
							pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
						}
			
						/* This blocks enters after  succesful AF switching/StaNotAvail Strategy */
						st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;

						SYS_RADIO_MEMCPY(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,(const void *)(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

						/* Sorting FM STL as newly FM Station appended into it */
						AMFM_App_application_Remove_SpacefromStationList((st_fm_station_list.ast_Stations),st_fm_station_list.u8_NumberOfStationsInList);
					}

					b_NotifySTL_Flag = TRUE;
				}
				else
				{
					/* Currently tuned FM station is already present in FM STL . No need to add */
					if( (pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI != 0 ) && 
					     (st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI) )
					{
						b_retBool = AMFM_App_Remove_SamePIfromFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI,&st_fm_station_list);
						if(b_retBool == TRUE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is updated as same PI station is removed.");

							/* As FM STL is sorted, index value of current station in FM STL might have changed. */
							pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
						}
			
						/* This blocks enters after  succesful AF switching/StaNotAvail Strategy */
						st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;

						SYS_RADIO_MEMCPY(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,(const void *)(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

						/* Sorting FM STL as newly FM Station appended into it */
						AMFM_App_application_Remove_SpacefromStationList((st_fm_station_list.ast_Stations),st_fm_station_list.u8_NumberOfStationsInList);

						b_NotifySTL_Flag = TRUE;
					}				
				}

				if(b_NotifySTL_Flag == TRUE)
				{
				//	u8_NotifySTL_Flag =0;

					/* Sorting FM STL as newly FM Station appended into it */
					(void)AMFM_App_application_SortFMStationList(st_fm_station_list.ast_Stations,st_fm_station_list.u8_NumberOfStationsInList);

					/* As FM STL is sorted, index value of current station in FM STL might have changed. */
					pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);	

					/*Sending notification to radio manager to inform STL is generated in shared memory */
					AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);	
				}	

				/* UnLocking the mutex between RM and APP*/
				SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);	
									
			
				/*Function to clear AF frequencied quality parameters alone*/
				AMFM_APP_Clear_AF_Qual_Parameters(&st_AMFM_App_AFList_Info);
			}
			else if( (pst_me_amfm_app_inst->st_current_station.e_mode == AMFM_APP_MODE_AM_MW )|| (pst_me_amfm_app_inst->st_current_station.e_mode == AMFM_APP_MODE_AM_LW) )
			{
				/* This variable is used to send AM/FM stationlist RESID/NOTIFIED to RM */
				e_CurrentMode = AMFM_APP_MODE_AM;
				
				/* Checking whether newly tuned  AM Station present in the AM STL */
				pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL = AMFM_App_CheckFreqPresentAMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq,&st_am_station_list);

				if( -1 == pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL)
				{
					SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/

					if(st_am_station_list.u8_NumberOfStationsInList < AMFM_APP_MAX_AM_STL_SIZE)
					{
						/* Newly seeked AM station is not present in AM STL.So appending it into list */
						st_am_station_list.ast_Stations[st_am_station_list.u8_NumberOfStationsInList].u32_Freq = pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq;

						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : AM STL is updated due to new AM station %d found.Sorted and Notified to RM",st_am_station_list.ast_Stations[st_am_station_list.u8_NumberOfStationsInList].u32_Freq);	
						st_am_station_list.u8_NumberOfStationsInList++;	
						
					}
					else
					{
						/* There is no space to add newly tuned AM staion in AM STL.So Clearing last station in AM STL */
						memset(&st_am_station_list.ast_Stations[AMFM_APP_MAX_AM_STL_SIZE-1],AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AMStationInfo));
						
						/* 	Appending currently tuned station info at last index in FM STL */
						st_am_station_list.ast_Stations[AMFM_APP_MAX_AM_STL_SIZE-1].u32_Freq = pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq;
						
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : AM STL is full. New AM station %d is added at last index.Sorted and Notified to RM",st_am_station_list.ast_Stations[st_am_station_list.u8_NumberOfStationsInList-1].u32_Freq);
					}
					
					/* Sorting AM STL as newly AM Station appended into it */
					AMFM_App_application_SortAMStationList((Ts_AMFM_App_AMStationInfo *)(st_am_station_list.ast_Stations),st_am_station_list.u8_NumberOfStationsInList);
					
					/* UnLocking the mutex between RM and APP*/
					SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);		
	
					/* As AM STL is sorted, index value of current station in AM STL might have changed. */
					pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentAMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq,&st_am_station_list);
						
					/*Sending notification to radio manager to inform STL is generated in shared memory */
					AMFM_App_Notify_STLUpdated(AM_APP_RADIO_MNGR);	
				}	
			}
		}
		break;
		
		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
            /* Reading current station info  from the message */
            AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_QualParam),(const Tchar *)(pst_msg->data),(Tu8)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);

			pst_me_amfm_app_inst->st_current_station.s32_BBFieldStrength 		= 	pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
    		pst_me_amfm_app_inst->st_current_station.u32_UltrasonicNoise	    =	pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
    		pst_me_amfm_app_inst->st_current_station.u32_Multipath				=	pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
    		pst_me_amfm_app_inst->st_current_station.u32_FrequencyOffset		= 	pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;

			if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
			{
				st_AMFM_App_AFList_Info.u32_Qua_curr   = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
		
				#ifdef CALIBRATION_TOOL
					 st_Calib_AMFM_App_AFList_Info.u32_Qua_curr = st_AMFM_App_AFList_Info.u32_Qua_curr ;
				#endif
				
			    AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
					
				if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
				}
					
				if((st_AMFM_App_AFList_Info.u16_curr_PI!=0)&&(pst_me_amfm_app_inst->b_CurStationPICpy ==TRUE)&&(st_AMFM_App_AFList_Info.u8_NumAFList == 0))
				{
					b_return_value = AMFM_App_AF_List_Append_From_learn_Memory(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
					if(b_return_value == TRUE)
					{
						u8_NVM_Write=1;
					}
					pst_me_amfm_app_inst->b_CurStationPICpy =FALSE;
				}
				if((u8_NVM_Write==1)&&(b_NVM_ActiveUpdateSwitch==TRUE))
				{
					/*code to store LSM data into NVM */
					u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
					if(u8_NVM_ret == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success during Tuner status notification ");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tuner station notification ");
					}
				}	
				if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)   
				{
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u16_curr_PI!=0) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
					{
						/*Temporarily disable FM-DAB Linking*/
#if 0
						/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
						AMFM_App_Notify_Find_SID(st_AMFM_App_AFList_Info.u16_curr_PI);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG,"[RADIO][AMFM_APP] PI  %x notified to DAB ",st_AMFM_App_AFList_Info.u16_curr_PI);
#endif
					}
					
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
					{
						/*Temporarily disable FM-DAB Linking*/
#if  0
						/* dab stationinfo that has to be tuned should be given to radio manager */

						AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();

						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;

						AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);

						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
#endif
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
					{
						/*Temporarily disable FM-DAB Linking*/
#if 0
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED due to sig lost");
#endif
					}
					if((st_AMFM_App_AFList_Info.u8_NumAFList!=0)  && (st_AMFM_App_AFList_Info.u16_curr_PI!= 0) && (pst_me_amfm_app_inst->b_StartAF_Flag == TRUE))
					{
						pst_me_amfm_app_inst->u8_aflistindex = 0;
						pst_me_amfm_app_inst->u16_curr_station_pi = (pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
						pst_me_amfm_app_inst->u32_ReqFrequency = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
						u8_af_update_cycle = 0;

						st_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
						st_AMFM_App_AFList_Info.u32_curr_freq = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
						st_AMFM_App_AFList_Info.u8_AF_index  = 0;
						st_AMFM_App_AFList_Info.u8_Best_AF_station_Index= 0 ;
						st_AMFM_App_AFList_Info.e_curr_status		=AMFM_APP_AF_SWITCHING;
						
						/*Temporarily disable af*/
#if 0
						AMFM_App_Notify_AF_Status(AMFM_APP_AF_LINK_INITIATED);
#endif						
						
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
							st_Calib_AMFM_App_AFList_Info.u32_curr_freq =(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency );
							st_Calib_AMFM_App_AFList_Info.u8_AF_index  = 0;
							st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index= 0 ;
							st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
						#endif

						/*Temporarily disable af*/
#if 0				
						/* Transiting to amfm_app_inst_hsm_active_idle_af_tune_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_tune_state);
#endif
				
					}
					else
					{
						/*nothing to do */
					}			
				}
			}
			else
			{
				/*For AM band*/
				if(pst_me_amfm_app_inst->st_current_station.s32_BBFieldStrength < (Ts32)pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
				{
					//pst_me_amfm_app_inst->st_current_station.u8_charset	= AMFM_APP_CHARSET_INVALID;
					pst_me_amfm_app_inst->e_SigQuality 						= AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					//pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
					pst_me_amfm_app_inst->e_SigQuality 						= AMFM_APP_GOOD_QUALITY_SIGNAL;
				}
			}
			if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
			{
				AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
			}
			else
			{
				if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
				{
					if(((st_AMFM_App_AFList_Info.u32_Qua_old_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold))
					|| ((st_AMFM_App_AFList_Info.u32_Qua_old_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)))
					{
						//pst_me_amfm_app_inst->u8_QualDegradedNotify_Flag=1;	
						AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
					}
					else
					{
						/*nothing to do*/
					}
				}
				else
				{
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
				}
			 }	
        }
		break;			
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
			
			/* Function to read quality of currently tuned station */
			AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
			
			if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
			{
				pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq;
				pst_me_amfm_app_inst->st_current_station.e_mode = (Te_AMFM_App_mode)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;
				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);

				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.u32_Qua_curr = st_AMFM_App_AFList_Info.u32_Qua_curr ;
				#endif
				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;

				if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
				}	

				if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi!=0)
				{
					if(st_AMFM_App_AFList_Info.u16_curr_PI==0)
					{
						u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] curretn fm station %d  PI %x updated ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);
						u8_af_appendCheck = 1;
					}
					else if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != st_AMFM_App_AFList_Info.u16_curr_PI)
					{
						/* PI changed */
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] curretn fm station %d  PI %x changed to %x ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);

						/* PI change detected notifying to RM*/
						AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_DETECTED);
	
						if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u8_NumAFList !=0))
						{
							st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_PI_CHANGE;
							#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
							#endif
							pst_me_amfm_app_inst->u16_curr_station_pi   = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;
							u8_af_update_cycle=0;
	
							/*Temporarily disable af*/
#if  0	
							/* Transiting to amfm_app_inst_hsm_active_idle_af_strategy_tune_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_strategy_tune_state);
#endif
						}
						else 
						{
							/* Updating new PI and AF lists */
	
							if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)&&(pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
							   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
									(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
							{
								/*DAB linked then stop DAB FM linking*/
								/*stop dab linking to do audio switch*/
								AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_PI_CHANGE);
								pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_STATION_TUNED;
								pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE   = AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
								/*send new PI to DAB*/
							}
	
							AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
							if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
							{
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
								/*Timer wont be started again,can close it*/
								if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
								{
									b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
									if(b_return_value == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
									}
									else
									{
										/* After stop Timer is success,reset the timer id */
										st_AMFM_TimerId.u32_Status_Check_TimerId =0;
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
									}
								}	
								else
								{
									/*MISRAC*/
								}
							}
							u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);
							pst_me_amfm_app_inst->b_CurStationPICpy =TRUE;

							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] curretn fm station %d AF Not available, New PI  %x Updated ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);

							AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_LINK_ESTABLISHED);
							memset(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
							memset(pst_me_amfm_app_inst->st_current_station.au8_RadioText,0,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
								/* As no AF is available, new station info (Pi & PSN) should be updated into STL.So enabling the flag 
								   in order to update PSN of curr station in FM STL only once */
								pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL = TRUE;

							/* Sending Response to Radio manager ,No best AF available for the PI changed station. Changed PI and PSN are updated in curr station */
							AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);
						}
					}
					else
					{
						/*PI not changed*/	
						u8_af_appendCheck = 1;
					}
				}
				else
				{
					/* PI not received nothing to do */
				}

				if(u8_af_appendCheck==1)
				{
					if((st_AMFM_App_AFList_Info.u8_NumAFList == 0) &&(pst_me_amfm_app_inst->b_CurStationPICpy ==TRUE))
					{
						b_return_value = AMFM_App_AF_List_Append_From_learn_Memory(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
						if(b_return_value==TRUE)
						{
							u8_NVM_Write=1;
						}
						pst_me_amfm_app_inst->b_CurStationPICpy =FALSE;
					}
					if((pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList !=0) && (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.b_AF_Checkbit == TRUE))
					{
						/*updating AF structure in both tuner status notify structure and current station info */
						/*Function to append AF lsit*/
						pst_me_amfm_app_inst->u32_ReqFrequency = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;

						b_return_value = AMFM_App_AF_List_frequency_Append(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_AFeqList,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumofAF_SamePgm,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumofAF_RgnlPgm,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst);

						if(b_return_value==TRUE)
						{
							u8_NVM_Write=1;
						}
						if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)/*to check NEG timer not started*/
						{
							/*timer not started*/
							b_return_value = AMFM_App_NEG_flag_status(&st_AMFM_App_AFList_Info);	
							if(b_return_value ==TRUE)
							{
								/*NEG flag set start the timer*/
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;
								st_AMFM_TimerId.u32_Status_Check_TimerId = SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID,RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
								else
								{
									//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
							}
						}
					}

					/* Copying PSN and RT into */
					b_PSNorRT_Changeflag = AMFM_App_PSN_RT_Copy(&(pst_me_amfm_app_inst->st_current_station),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);	
				    
					/* Copying Program Name */
				    AMFM_App_PTY_Copy(&(pst_me_amfm_app_inst->st_current_station),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);	

					/* Copying CT Info */
					AMFM_App_ReadCTinfo(&pst_me_amfm_app_inst->st_RDS_CT_Info,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);

					/* Copying TA TP Info */
					AMFM_App_Read_TA_TP_info(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				}
				if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)   &&(st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_PI_CHANGE))
				{
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u16_curr_PI!=0) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
					{
						/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
						AMFM_App_Notify_Find_SID(st_AMFM_App_AFList_Info.u16_curr_PI);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] PI  %x notified to DAB ",st_AMFM_App_AFList_Info.u16_curr_PI);
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
					{
						/* dab stationinfo that has to be tuned should be given to radio manager */

						AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();

						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;

						AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);

						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED due to low signal of DAB");
					}
					else
					{
						/*nothing to do */
					}

					if((st_AMFM_App_AFList_Info.u8_NumAFList!=0)  && (st_AMFM_App_AFList_Info.u16_curr_PI!= 0)&& (pst_me_amfm_app_inst->b_StartAF_Flag == TRUE))
					{
						pst_me_amfm_app_inst->u8_aflistindex = 0;
						pst_me_amfm_app_inst->u16_curr_station_pi = (pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
						pst_me_amfm_app_inst->u32_ReqFrequency = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;

						/* New strategy */
						st_AMFM_App_AFList_Info.u32_curr_freq            =(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency );
						st_AMFM_App_AFList_Info.u8_AF_index              = 0;
						st_AMFM_App_AFList_Info.u8_Best_AF_station_Index = 0 ;
						st_AMFM_App_AFList_Info.b_af_check_flag          = FALSE;
						st_AMFM_App_AFList_Info.e_curr_status		     = AMFM_APP_AF_SWITCHING;
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.b_af_check_flag          = FALSE;
							st_Calib_AMFM_App_AFList_Info.u32_curr_freq            =(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency );
							st_Calib_AMFM_App_AFList_Info.u8_AF_index              = 0;
							st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index = 0 ;
							st_Calib_AMFM_App_AFList_Info.e_curr_status            = st_AMFM_App_AFList_Info.e_curr_status;
						#endif
						u8_af_update_cycle = 0;

						/*Temporarily disable af*/
#if 0
						AMFM_App_Notify_AF_Status(AMFM_APP_AF_LINK_INITIATED);
						/* Transiting to amfm_app_inst_hsm_active_idle_af_tune_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_tune_state);
#endif

					}
					else
					{
						/*nothing to do */
					}
				}

				SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/


				/* Updating PI of current station in FM STL */
				if(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI)
				{	
					b_retBool = AMFM_App_Remove_SamePIfromFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI,&st_fm_station_list);
					if(b_retBool == TRUE	)
					{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is updated as same PI station is removed.");

					/* As FM STL is sorted, index value of current station in FM STL might have changed. */
					pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
					}
					st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;

					if(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != 0)
					{
					/* Clearing old PSN as PI of tuned Station is changed.*/
					memset(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					}	
					
					/*Sending notification to radio manager to inform STL is generated in shared memory */
					AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);
				}

				if( (b_PSNorRT_Changeflag & 0x01)  && (pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL == TRUE) )
				{
					/* Clearingthe flag so that PSN of curr station in FM STL will not be updated further */
					pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL = FALSE;

					SYS_RADIO_MEMCPY(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,(const void *)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

					/* Sorting FM STL as newly seeked FM Station appended into it */
					AMFM_App_application_Remove_SpacefromStationList((st_fm_station_list.ast_Stations),st_fm_station_list.u8_NumberOfStationsInList);

					(void)AMFM_App_application_SortFMStationList(st_fm_station_list.ast_Stations,st_fm_station_list.u8_NumberOfStationsInList);

					/* As FM STL is sorted, index value of current station in FM STL might have changed. */
					pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);

					/*Sending notification to radio manager to inform STL is generated in shared memory */
					AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);	

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : PSN is added in FM STL. So FM STL is sorted and notified to RM ");										
				}

				/* UnLocking the mutex between RM and APP*/
				SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);		

				if((u8_NVM_Write==1)&&(b_NVM_ActiveUpdateSwitch==TRUE))
				{
					/*code to store LSM data into NVM */
					u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
					if(u8_NVM_ret == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success during Tuner status notification ");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tuner station notification ");
					}
				}	
			}
			else
			{
				/*For AM band*/
				if(pst_me_amfm_app_inst->st_current_station.s32_BBFieldStrength < (Ts32)pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
				{
					//pst_me_amfm_app_inst->st_current_station.u8_charset	= AMFM_APP_CHARSET_INVALID;
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					//pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
				}
			}
			if(st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_PI_CHANGE)
			{
				AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
			}
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
/*	Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleTuneFailedListenHndlr	 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleTuneFailedListenHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg								*pst_ret				= NULL;             /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex       = 0;   				/* variable to update the message slot index */
	Tbool 									b_return_value = FALSE;
	Tu8 									u8_NVM_ret=0;
	Tu32 									u32_NVM_Update_Param=0;
	Tbool									b_retBool  = FALSE;
	Tbool									b_PSNorRT_Changeflag;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveIdleTuneFailedListenHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_tune_failed_listen_state",sizeof("amfm_app_inst_hsm_active_idle_tune_failed_listen_state"));
			pst_me_amfm_app_inst->b_CurStationPICpy = TRUE;
			/* Enabling the flag in order to update PSN of curr station in FM STL only once */
			pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL = TRUE;

			u8_af_update_cycle=0;
			
			/*update with freq,PI and AFlist of curr station info */
			if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
			{
				/*New strategy*/
				st_AMFM_App_AFList_Info.u32_curr_freq =(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency );
				st_AMFM_App_AFList_Info.u16_curr_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI	 ;
				#ifdef CALIBRATION_TOOL
					b_start_stop_switch = FALSE;
					u32_calib_freq =0;
					st_Calib_AMFM_App_AFList_Info.u32_curr_freq = st_AMFM_App_AFList_Info.u32_curr_freq;
					st_Calib_AMFM_App_AFList_Info.u16_curr_PI   = st_AMFM_App_AFList_Info.u16_curr_PI;
				#endif
			}
			
		}
		break;
		
		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
            /* Reading current station info  from the message */
            AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);
            
			
			pst_me_amfm_app_inst->st_current_station.s32_BBFieldStrength 		= 	pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
            pst_me_amfm_app_inst->st_current_station.u32_UltrasonicNoise	    =	pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
            pst_me_amfm_app_inst->st_current_station.u32_Multipath				=	pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
            pst_me_amfm_app_inst->st_current_station.u32_FrequencyOffset		= 	pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;
			
			
			if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
			{
				
				pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
			
				#ifdef CALIBRATION_TOOL
					 st_Calib_AMFM_App_AFList_Info.u32_Qua_curr = st_AMFM_App_AFList_Info.u32_Qua_curr ;
				#endif

			    AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
		
				if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)   
				{
				
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u16_curr_PI!=0) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
					{
						/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
						AMFM_App_Notify_Find_SID(st_AMFM_App_AFList_Info.u16_curr_PI);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] PI  %x notified to DAB ",st_AMFM_App_AFList_Info.u16_curr_PI);
	
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
					{
						/* dab stationinfo that has to be tuned should be given to radio manager */
				
						AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();

						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;

						AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
						
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					
				}
				
				if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u8_SigResumeback_Threshold)
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
				
					/*Transit to idle_listen state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);	
				}
				
			}
			if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
			{
				AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
			}
			else
			{
				if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
				{
					if(((st_AMFM_App_AFList_Info.u32_Qua_old_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold))
					|| ((st_AMFM_App_AFList_Info.u32_Qua_old_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)))
					{
						//pst_me_amfm_app_inst->u8_QualDegradedNotify_Flag=1;	
						AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
					}
					else
					{
						/*nothing to do*/
					}
				}
				else
				{
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
				}
			}

        }
		break;
			
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
		
			/* Function to read quality of currently tuned station */
			AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
			if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
			{				
				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength , pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);
			
				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.u32_Qua_curr = st_AMFM_App_AFList_Info.u32_Qua_curr ;
				#endif
				
				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
				pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
							
				if((pst_me_amfm_app_inst->b_CurStationPICpy==TRUE) && (st_AMFM_App_AFList_Info.u16_curr_PI==0))
				{	
					if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != 0)
					{
						pst_me_amfm_app_inst->b_CurStationPICpy=FALSE;
						pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI		=  pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi ;
						st_AMFM_App_AFList_Info.u16_curr_PI											=  pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;
					
						pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;

						SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/

						/* Updating PI of current station in FM STL */
						if(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI)
						{		
							b_retBool = AMFM_App_Remove_SamePIfromFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI,&st_fm_station_list);

							if(b_retBool == TRUE)
							{
								/* As FM STL is sorted, index value of current station in FM STL might have changed. */
								pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);

								RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is updated as same PI station is removed.");
							}	
							st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;							

							/*Sending notification to radio manager to inform STL is generated in shared memory */
							AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);
						}
						
						/* UnLocking the mutex between RM and APP*/
						SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);	
						
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.u16_curr_PI  = st_AMFM_App_AFList_Info.u16_curr_PI ;
						#endif
						b_return_value=AMFM_App_Learn_Memory_updation((st_AMFM_App_AFList_Info.u32_curr_freq), st_AMFM_App_AFList_Info.u16_curr_PI,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
						if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
						{
							/*code to store LSM data into NVM */
							u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
							if(u8_NVM_ret == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success  ");
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  ");
							}
						}
					}
					
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u16_curr_PI!=0) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
					{
						/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
						AMFM_App_Notify_Find_SID(st_AMFM_App_AFList_Info.u16_curr_PI);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] PI notified to DAB ");
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
					{
						/* dab stationinfo that has to be tuned should be given to radio manager */
				
						AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();

						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;

						AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
						
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					b_PSNorRT_Changeflag =  AMFM_App_PSN_RT_Copy(&(pst_me_amfm_app_inst->st_current_station),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);	
					/*Copying the program name */
					AMFM_App_PTY_Copy(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					AMFM_App_ReadCTinfo(&pst_me_amfm_app_inst->st_RDS_CT_Info,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					/* Copying TA TP Info */
					AMFM_App_Read_TA_TP_info(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				
					SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/

					if( (b_PSNorRT_Changeflag & 0x01)  && (pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL == TRUE) )
					{
						/* Clearingthe flag so that PSN of curr station in FM STL will not be updated further */
						pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL = FALSE;

						SYS_RADIO_MEMCPY(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,(const void *)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

						/* Sorting FM STL as newly seeked FM Station appended into it */
						AMFM_App_application_Remove_SpacefromStationList((st_fm_station_list.ast_Stations),st_fm_station_list.u8_NumberOfStationsInList);

						(void)AMFM_App_application_SortFMStationList(st_fm_station_list.ast_Stations,st_fm_station_list.u8_NumberOfStationsInList);
			
						/* As FM STL is sorted, index value of current station in FM STL might have changed. */
						pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
					
						/*Sending notification to radio manager to inform STL is generated in shared memory */
						AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);	
					
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : PSN is added in FM STL. So FM STL is sorted and notified to RM ");				
					}
					/* UnLocking the mutex between RM and APP*/
					SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);	
				}
				else
				{
					/* nothing to do */
				}

				if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
					
					/*Transit to idle_listen state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
				}
			}
			AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
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
	

/*================================================================================*/
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFStrategyTuneHndlr	  */
/*================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFStrategyTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret							=	NULL;										 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu8										u8_AFListIndex				    = 0;
	Tu8										u8_Best_AF_station_Index	    = 0;
    Tbool 									b_return_value					= FALSE;
	Tbool									b_check_next_best_AF            = FALSE;
	Tu8 									u8_NVM_ret=0;
	Tu8										u8_NVM_Write=0;
	Tu8										u8_LM_Updation=0;
	Tu8 									au8_psn[MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME];
	Tu8 									au8_RT[AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT];
//	Tu8										u8_PSNorRT_Changeflag=0;
	Tu8										u8_strategy_fail=0;
	Tu16                        	        u16_afpi;
	Tu32									u32_DataSlotIndex				= 0;						 					 /* variable to update the message slot index */
    Tu32 									u32_NVM_Update_Param=0;
	
	#ifdef CALIBRATION_TOOL
		Tu8 								u8_Calib_AF_index;
		Tu8									u8_Calib_best_AF_index;
	#endif
	Te_RADIO_ReplyStatus	                e_PlaySelectStationReplyStatus  = REPLYSTATUS_REQ_NOT_HANDLED;  /* enum variable to hold the  Play select station reply status*/
	Te_RADIO_ReplyStatus			        e_AMFM_APP_AF_CheckReplystatus ;
	Te_RADIO_ReplyStatus		            e_AMFM_APP_AF_UpdateReplystatus;
	Te_AMFM_App_CurrStatus 					e_curr_status;
	Ts_AMFM_APP_DAB_FollowUp_StationInfo	st_DAB_FollowUp_StationInfo;
	Tu16	                                u32_AFupdateFreq = 0;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:		
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: amfm_app_inst_hsm_active_idle_af_strategy_tune_state");
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] current tuned station freq= %d PI= %x ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);

			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_af_strategy_tune_state",sizeof("amfm_app_inst_hsm_active_idle_af_strategy_tune_state")) ;
						
			#ifdef CALIBRATION_TOOL
				b_start_stop_switch		= FALSE ;
				u32_calib_freq			= 0;
				u8_notify_count			= 0;
				u32_cal_curr			= 0;
				u32_cal_avg				= 0;
				u32_cal_old				= 0;
				s32_cal_BBFieldStrength = 0;
				u32_cal_UltrasonicNoise = 0;
				u32_cal_Multipath		= 0;
				
			#endif
			
			st_AMFM_App_AFList_Info.u8_AF_index =0;
			st_AMFM_App_AFList_Info.u8_AF_Update_Index=0;
			
			if(st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList !=0)
			{
				/*same PI AFList exists*/
				st_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index=0;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				#endif

				u8_AFListIndex =st_AMFM_App_AFList_Info.u8_AF_Update_Index;
				st_AMFM_App_AFList_Info.u32_af_update_newfreq=st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq=st_AMFM_App_AFList_Info.pst_current_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index].u32_AF_Freq;
				#endif
				/* Requesting AF Update to get quality of the AF Frequency*/
				AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq);
				
			}
			else
			{
				/* No same PI AFList*/
				if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_REQ_RECEIVED)
				{
					/* Sending Response to Radio Mngr */
					AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_current_station);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] AF Tune request failed due to 0 AF Frequencies");
				
					st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_AF_TUNE_SCAN_STARTED;
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
					#endif

					pst_me_amfm_app_inst->e_stationlist_mode = AMFM_APP_MODE_FM;
					e_CurrentMode = AMFM_APP_MODE_FM;
					/* Transisting to active process stl state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_processSTL_state);
			
				}
				else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_SCAN_COMPLETED)
				{
				
					/* Sending Response to Radio Mngr */
					AMFM_App_Notify_UpdatedLearnMem_AFStatus(AMFM_APP_LEARN_MEM_AF_FAIL,pst_me_amfm_app_inst->st_current_station);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] AF Tune request failed again after scan due to 0 AF Frequencies");
					/*strategy ends wait for tune to original frequency command*/
				}
				else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_PI_CHANGE)
				{
					/* PI changed and cant switch to the same Pi station*/
					
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
					    ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
						 (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_PI_CHANGE);
						/* cleat fm-dab infos*/
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality 		= AMFM_APP_CONSTANT_ZERO;
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE 	= AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status	= AMFM_APP_FM_STATION_TUNED;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					else
					{
						/*nothing to do */
					}
				
					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
					{
						pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
						/*Timer wont be started again,can close it*/
						if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
						{
							b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
							if(b_return_value == 1)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
							}
							else
							{
								st_AMFM_TimerId.u32_Status_Check_TimerId=0;
					//			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
							}
						}	
						else
						{
							/*MISRAC*/
						}
					}
					AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
					u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->u16_curr_station_pi);
					
					memset(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					memset(pst_me_amfm_app_inst->st_current_station.au8_RadioText,0,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
					if(((u8_NVM_Write==1)/*||(u8_PSNorRT_Changeflag==1)*/)&&(b_NVM_ActiveUpdateSwitch == TRUE))
					{
						/*code to store LSM data into NVM */
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success during PI change ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tuner station notification ");
						}
					}	
					AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_LINK_ESTABLISHED);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] current fm station %d AF Not available, New PI  %x Updated ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);
		
					/* Sending Response to Radio manager ,No best AF available for the PI changed station. Changed PI updated in curr station */
					AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);
						
					/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
					
								
				}
				else
				{
					/*no strategy*/
				}
			}
		}
		break;
		
		case AMFM_APP_STA_NOT_AVAIL_STRA_STATUS_NOTIFYID:
		{
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_StationNotAvailStrategyStatus),&u32_DataSlotIndex);
			/* Strategy end notification will be received here*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP] AF strategy stop notification received  ");
		
			if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_STRATEGY_SUCCESS)
			{
				/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
			}
			else
			{
				st_AMFM_App_AFList_Info.e_curr_status = AMFM_APP_WAIT_TUNE_REQ;
				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
				#endif


			}
		}break;
		
		
		case AMFM_APP_CANCEL_REQID:
		{
			/* Sending cancelDone Response to Radio Mngr */
			AMFM_App_Response_CancelDone(REPLYSTATUS_SUCCESS);
		}
		break;
		
		case AMFM_STRATEGY_AF_UPDATE_TIMERID:
		{
			if(	st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_WAIT_TUNE_REQ)
			{
				/*Timer expired after delay 0f 10 milli SEC*/
				b_return_value = AMFM_APP_AFStrategy_AFUpdate(&st_AMFM_App_AFList_Info);
				if(b_return_value == TRUE)
				{
					/*All stations from AF list are processed */
					u8_af_update_cycle++;
					AMFM_App_Sort_SAME_PI_AF_List(&st_AMFM_App_AFList_Info);
					/*Debug Log print*/
					AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AF_STRATEGY_AFLIST_SORTED_QUALITIES)	;
					if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
					{
						/*ENG Mode ON check for changes in quality of AF frequenices*/
						b_return_value=AMFM_APP_ENG_AF_Updation_Check(&(pst_me_amfm_app_inst->st_ENG_AFList_Info),&st_AMFM_App_AFList_Info);
						if(b_return_value==TRUE)
						{
							/*sending ENG notification to radio manager*/
							AMFM_App_Notify_ENG_AFList(&(pst_me_amfm_app_inst->st_ENG_AFList_Info));
						}
					}
					else
					{
						/* ENG MODE switch OFF so nothing to do */
					}
					pst_me_amfm_app_inst->u32_AF_qual_decr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Existance_decr_alpha;
					pst_me_amfm_app_inst->u32_AF_qual_incr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Existance_incr_alpha;

					if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
					{
						u8_strategy_fail=1;
					}
					else
					{	
						if(u8_af_update_cycle >= u8_StrategyAFupdatecount)
						{
							/*cycle completed can go for best AF availability check*/
							b_return_value=AMFM_App_BestAFAvailabilityCheck(&st_AMFM_App_AFList_Info);
							if(b_return_value==FALSE)
							{
								/*Best AF available check should be given*/	
								AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
								AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AF_STRATEGYCHECK_REQUEST);

							}
							else
							{
								u8_strategy_fail=1;
							}
						}
						else
						{
							/*Restart AFUpdate*/
							AMFM_App_AFupdate_Restart(&st_AMFM_App_AFList_Info);
						}
					}	
				}
				else
				{
					/*AF update request is successfully processed*/
				}
				if(u8_strategy_fail==1)
				{
					/*strategy fails,get appropriate statuses*/
					if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_REQ_RECEIVED)
					{
						/* Sending Response to Radio Mngr */
						AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_current_station);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] AF Tune request failed due to no best  AF Frequencies");
				
						st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_AF_TUNE_SCAN_STARTED;
						pst_me_amfm_app_inst->e_stationlist_mode 	= AMFM_APP_MODE_FM;
						e_CurrentMode = AMFM_APP_MODE_FM;
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
						#endif
						/* Transisting to active process stl state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_processSTL_state);
			
					}
					else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_SCAN_COMPLETED)
					{
				
						/* Sending Response to Radio Mngr */
						AMFM_App_Notify_UpdatedLearnMem_AFStatus(AMFM_APP_LEARN_MEM_AF_FAIL,pst_me_amfm_app_inst->st_current_station);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] AF Tune request failed again after scan  due to no best AF Frequencies");
						/*strategy ends wait for tune to original frequency command*/
					}
					else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_PI_CHANGE)
					{
						
						if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
					    	((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
						 	 (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
						{
							/*stop dab linking do audio switch*/
							AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_PI_CHANGE);
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality 		= AMFM_APP_CONSTANT_ZERO;
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE 	= AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status	= AMFM_APP_FM_STATION_TUNED;
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED due to PI change");
						}
						else
						{
							/*nothing to do */
						}
						if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
						{
							pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
							/*Timer wont be started again,can close it*/
							if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
							{
								b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
								if(b_return_value == FALSE)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
								}
								else
								{
									st_AMFM_TimerId.u32_Status_Check_TimerId=0;
							//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
								}	
							}
							else
							{
								/*MISRAC*/
							}
						}
						AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
						u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->u16_curr_station_pi);
						
						memset(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
						memset(pst_me_amfm_app_inst->st_current_station.au8_RadioText,0,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
					
						if(((u8_NVM_Write==1)/*||(u8_PSNorRT_Changeflag==1)*/)&&(b_NVM_ActiveUpdateSwitch == TRUE))
						{
							/*code to store LSM data into NVM */
							u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
							if(u8_NVM_ret == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success during Tuner status notification ");
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tuner station notification ");
							}
						}	
						AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_LINK_ESTABLISHED);
					
						/* Sending Response to Radio manager ,No best AF available for the PI changed station. Changed PI  updated in curr station */
						AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] curretn fm station %d  PI changed to %x ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);
	
						/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
					}
					else
					{
						/*nothing to do*/
					}
				}
			}
		}
		break;
		
	
		case AMFM_APP_AFFREQ_UPDATE_DONE_RESID:
		{
			/*Update response for the AF frequency*/
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_UpdateReplystatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u32_AFupdateFreq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);

			st_AMFM_App_AFList_Info.b_af_check_flag    = FALSE;
			
			u8_AFListIndex						       = st_AMFM_App_AFList_Info.u8_AF_Update_Index;
			
			#ifdef CALIBRATION_TOOL
				
				st_Calib_AMFM_App_AFList_Info.b_af_check_flag    = FALSE;
				u8_Calib_AF_index								 = st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index;
			#endif

			if(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq == u32_AFupdateFreq)
			{
				if(e_AMFM_APP_AF_UpdateReplystatus == REPLYSTATUS_SUCCESS)
				{
		
					pst_me_amfm_app_inst->u32_AMFM_quality =  QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset);
					/* update response for AF list frequencies*/
					/* QUALITY CHECK SUCCEEDED*/
					st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].s32_BBFieldStrength =  pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
					st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_UltrasonicNoise = pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
					st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_Multipath = pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
					st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_FrequencyOffset = pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;
					st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_curr_qual =pst_me_amfm_app_inst->u32_AMFM_quality;

					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].s32_BBFieldStrength =  pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_UltrasonicNoise = pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_Multipath = pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_FrequencyOffset = pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_curr_qual =pst_me_amfm_app_inst->u32_AMFM_quality;
					#endif

					AMFM_App_AF_Quality_Avg_computation(&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->u32_AF_qual_incr_alpha,pst_me_amfm_app_inst->u32_AF_qual_decr_alpha);
					
					/*update request to next AF freq. start timer to  new update request */
					st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(pst_me_amfm_app_inst->u8_StrategyAFUpdate_Delay, AMFM_STRATEGY_AF_UPDATE_TIMERID,RADIO_AM_FM_APP);
					if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_STRATEGY_AF_UPDATE_TIMERID);
					}
					else
					{
				//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_STRATEGY_AF_UPDATE_TIMERID);
					}

				}
				else 
				{
					/*UPDATE REQUEST FAILED */
					/*Tuner ctrl will always give success*/
					/*START TIMER TO GIVE NEXT UPDATE */
					st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(pst_me_amfm_app_inst->u8_StrategyAFUpdate_Delay, AMFM_STRATEGY_AF_UPDATE_TIMERID,RADIO_AM_FM_APP);
					if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_STRATEGY_AF_UPDATE_TIMERID);
					}
					else
					{
				//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_STRATEGY_AF_UPDATE_TIMERID);
					}
				}
			}	
			else
			{
				/* AF update response is for other frequency.So dropping this message and waiting for correct response */
			}
		}
		break;
		
		case AMFM_APP_AFFREQ_CHECK_DONE_RESID :
		{
			/* Check response for the best AF frequency*/
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_CheckReplystatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&(u16_afpi),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			u8_Best_AF_station_Index 				= st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
		
			#ifdef CALIBRATION_TOOL
				u8_Calib_best_AF_index 			    = st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
			#endif

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] During strategy,check given for Best AF  %d and PI %x received ",st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,u16_afpi);		
			
			if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
			{
				u8_strategy_fail=1;
			}
			else
			{
				if((e_AMFM_APP_AF_CheckReplystatus == REPLYSTATUS_SUCCESS)&& (st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_WAIT_TUNE_REQ))
				{
					if( u16_afpi == st_AMFM_App_AFList_Info.u16_curr_PI)
					{
						/*pi matches */
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI	= u16_afpi;
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
		
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].u16_PI			= u16_afpi;
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;	
						#endif
						AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
						u8_LM_Updation=1;
					}
					else if(u16_afpi != 0)
					{
						/* pi is  not 0 or interference signal so mark as NEG   */
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI	= u16_afpi;
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].u16_PI	= u16_afpi;
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
						#endif
						
						u8_LM_Updation=1;
						b_check_next_best_AF = TRUE;
					}
					else
					{
						/* pi not decodable strategy ends*/
						u8_strategy_fail=1;
					}
					if(u8_LM_Updation==1)
					{
						/*PI detected has to be updated in LM */
						b_return_value=AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq, u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
						if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch == TRUE))
						{
							/*code to store LSM data into NVM */
							u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
							if(u8_NVM_ret == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  during AF check");
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during AF check ");
							}
						}
					}								
				}
				else
				{
					/*check failure usualy control wont come here*/
					b_check_next_best_AF = TRUE;
				}
			}
			if((b_check_next_best_AF == TRUE)&& (st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_WAIT_TUNE_REQ))
			{
				/*To check for next best AF*/
				b_return_value=AMFM_App_BestAFAvailabilityCheck(&st_AMFM_App_AFList_Info);
				if(b_return_value==FALSE)
				{
					/*Best AF available check should be given*/
					AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
					AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AF_STRATEGYCHECK_REQUEST);
				}
				else
				{
					/*no best AF strategy fails*/
					u8_strategy_fail=1;
				}
			}
			if(u8_strategy_fail==1)
			{
					/* No same PI AFList*/
				if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_REQ_RECEIVED)
				{
					/* Sending Response to Radio Mngr */
					AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_current_station);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] AF Tune request failed due to 0 AF Frequencies");
				
					st_AMFM_App_AFList_Info.e_curr_status	 = AMFM_APP_AF_TUNE_SCAN_STARTED;
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
					#endif
					pst_me_amfm_app_inst->e_stationlist_mode = AMFM_APP_MODE_FM;
					e_CurrentMode = AMFM_APP_MODE_FM;
					/* Transisting to active process stl state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_processSTL_state);
			
				}
				else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_SCAN_COMPLETED)
				{
				
					/* Sending Response to Radio Mngr */
					AMFM_App_Notify_UpdatedLearnMem_AFStatus(AMFM_APP_LEARN_MEM_AF_FAIL,pst_me_amfm_app_inst->st_current_station);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] AF Tune request failed again due to best  AF check Fails");
					/*strategy ends wait for tune to original frequency command*/
				}
				else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_PI_CHANGE)
				{
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
					   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
					    (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_PI_CHANGE);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality 		= AMFM_APP_CONSTANT_ZERO;
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE 	= AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status	= AMFM_APP_FM_STATION_TUNED;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					else
					{
						/*nothing to do */
					}
					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
					{
						pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
						/*Timer wont be started again,can close it*/
						if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
						{
							b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
							if(b_return_value == FALSE)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
							}
							else
							{
								st_AMFM_TimerId.u32_Status_Check_TimerId =0;
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
							}	
						}
						else
						{
							/*MISRAC*/
						}
					}
					memset(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					memset(pst_me_amfm_app_inst->st_current_station.au8_RadioText,0,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
					
					/*tune to original frequency to make soc tune to original station*/
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),(st_AMFM_App_AFList_Info.u32_curr_freq));					
				}
				else
				{
					/*nothing to do*/
				}
			}
			
		}
		break;

		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);

			e_curr_status = st_AMFM_App_AFList_Info.e_curr_status;
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] During strategy,Tune given for freq  %d ",pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);		
			
			if(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	!= pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq)	
			{
				if((e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)&& (st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_WAIT_TUNE_REQ))
				{
					pst_me_amfm_app_inst->u16_af_Tuned_PI = st_AMFM_App_AFList_Info.u16_curr_PI;
					/* storing current station PSN and RT to a temporary variable to copy back */
					SYS_RADIO_MEMCPY(au8_psn,(const void *)(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					SYS_RADIO_MEMCPY(au8_RT,(const void *)(pst_me_amfm_app_inst->st_current_station.au8_RadioText),AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
					SYS_RADIO_MEMCPY(&st_DAB_FollowUp_StationInfo,(const void *)&(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo),sizeof(st_DAB_FollowUp_StationInfo));
				
					u8_NVM_Write = AMFM_App_Learn_Memory_updation((st_AMFM_App_AFList_Info.u32_curr_freq), pst_me_amfm_app_inst->u16_curr_station_pi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
			
					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)/*to check NEG timer  started*/
					{
						/*timer  started*/
						/*stop the timer*/
						if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
						{
							b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
							if(b_return_value == FALSE)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
							}
							else
							{
								st_AMFM_TimerId.u32_Status_Check_TimerId=0;
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
							}
						}
						else
						{
							/*MISRAC*/
						}
					}
					AMFM_APP_Curr_Station_Related_Clearing(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
				
					pst_me_amfm_app_inst->st_current_station.e_mode 								 = (Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band);
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 = (Tu32)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq*(Tu16)10) ;
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI		 	 =	pst_me_amfm_app_inst->u16_af_Tuned_PI;
					pst_me_amfm_app_inst->st_current_station.u8_charset								 =  pst_me_amfm_app_inst->u8_charset;

					/* copying the psn and RT back to current station */
					SYS_RADIO_MEMCPY((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn),(const void *)au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					SYS_RADIO_MEMCPY((pst_me_amfm_app_inst->st_current_station.au8_RadioText),(const void *)au8_RT,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
					
					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					
					/* Clearing LSM memory for AF list */
					memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
						
					/* Updating Frequency of newly tuned FM station into LSM memory */
					pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = (Tu32)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq*(Tu16)10);
					pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI      = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;

					AMFM_App_Notify_AF_Status(AMFM_APP_AF_LINK_ESTABLISHED);
						
					if(e_curr_status == AMFM_APP_AF_TUNE_REQ_RECEIVED)
					{
						
						/*IF it is a  Af tune strategy success response should be sent*/
						/* Sending Response for AFTune to Radio Mngr */
						AMFM_App_Response_AFTune(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);
						st_AMFM_App_AFList_Info.e_curr_status	= AMFM_APP_AF_TUNE_STRATEGY_SUCCESS;
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
						#endif
					}
					else if(e_curr_status == AMFM_APP_AF_TUNE_SCAN_COMPLETED)
					{
						/*IF it is a  Af tune strategy success response should be sent*/
						/* Sending Response to Radio Mngr */
						AMFM_App_Notify_UpdatedLearnMem_AFStatus(AMFM_APP_LEARN_MEM_AF_SUCCESS,pst_me_amfm_app_inst->st_current_station);
						st_AMFM_App_AFList_Info.e_curr_status	= AMFM_APP_AF_TUNE_STRATEGY_SUCCESS;
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
						#endif
					}
					else if(e_curr_status == AMFM_APP_PI_CHANGE)
					{
						SYS_RADIO_MEMCPY(&(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo),(const void *)&(st_DAB_FollowUp_StationInfo),sizeof(st_DAB_FollowUp_StationInfo));
						
						/* After PI change alternate is found . But needs to update New PI info into FM STL */
						st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI = pst_me_amfm_app_inst->u16_curr_station_pi ;
							
						/* Clearing PSN as it contains old PSN only.Before decoding New PSN AF strategy is started */
						memset(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
							
						/* Sending Response to Radio manager  best AF available for the PI changed station*/
						AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_current_station);
						
						/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
					}
					else
					{
						/*no strategy*/
					}
					if(b_NVM_ActiveUpdateSwitch == TRUE)
					{
						/* writing the datas into NVM*/
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0) 
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  during  AF tune");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during  AF tune ");
						}
					}

				}
				else if(e_PlaySelectStationReplyStatus == REPLYSTATUS_NO_SIGNAL)
				{
					/*JUMP REQUEST FAILED*/
					u8_strategy_fail=1;
				}
			}
			else
			{
				u8_strategy_fail=1;
				/*original frequencies tune response*/
				st_AMFM_App_AFList_Info.b_af_check_flag            = FALSE;	
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.b_af_check_flag  = FALSE;
				#endif
				/* Function to read quality of currently tuned station */
				AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);

				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength , pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);

				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
			}
			if(u8_strategy_fail==1)
			{
				if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_REQ_RECEIVED)
				{
					/* Sending Response to Radio Mngr */
					AMFM_App_Response_AFTune(REPLYSTATUS_FAILURE,pst_me_amfm_app_inst->st_current_station);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] AF Tune request failed  starting scan");
				
					st_AMFM_App_AFList_Info.e_curr_status	 = AMFM_APP_AF_TUNE_SCAN_STARTED;
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
					#endif
					pst_me_amfm_app_inst->e_stationlist_mode = AMFM_APP_MODE_FM;
					e_CurrentMode = AMFM_APP_MODE_FM;
					/* Transisting to active process stl state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_processSTL_state);
			
				}
				else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_AF_TUNE_SCAN_COMPLETED)
				{
				
					/* Sending Response to Radio Mngr */
					AMFM_App_Notify_UpdatedLearnMem_AFStatus(AMFM_APP_LEARN_MEM_AF_FAIL,pst_me_amfm_app_inst->st_current_station);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] AF Tune request failed again due to best  AF check Fails");
					/*strategy ends wait for tune to original frequency command*/
					st_AMFM_App_AFList_Info.e_curr_status	 = AMFM_APP_AF_TUNE_STRATEGY_FAILED;
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
					#endif
				}
				else if(st_AMFM_App_AFList_Info.e_curr_status == AMFM_APP_PI_CHANGE)
				{
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
					   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
					    (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_PI_CHANGE);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality 		= AMFM_APP_CONSTANT_ZERO;
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE 	= AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status	= AMFM_APP_FM_STATION_TUNED;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED due to PI change");
					}
					else
					{
						/*nothing to do */
					}
					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
					{
						pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
						/*Timer wont be started again,can close it*/
						if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
						{
							b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
							if(b_return_value == FALSE)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
							}
							else
							{
								st_AMFM_TimerId.u32_Status_Check_TimerId=0;
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
							}	
						}
						else
						{
							/*MISRAC*/
						}
					}
					AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
					u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->u16_curr_station_pi);

					if((u8_NVM_Write==1)&&(b_NVM_ActiveUpdateSwitch == TRUE))
					{
						/*code to store LSM data into NVM */
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success during Tuner status notification ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tuner station notification ");
						}
					}	
					
					AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_LINK_ESTABLISHED);
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] curretn fm station %d  PI changed to %x ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);
		
					/* Sending Response to Radio manager ,No best AF available for the PI changed station. Changed PI and PSN are updated in curr station */
					AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);
						
					/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
				}
				else
				{
					/*no strategy*/
				}
			}
		}
		break;

		case HSM_MSGID_EXIT:
		{
			/*  free all timers */
			if(st_AMFM_TimerId.u32_AMFM_APP_TimerId >0)
			{
				b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_AMFM_APP_TimerId);
				if(b_return_value == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 5 failed to stop message will be posted");
				}
				else
				{
					st_AMFM_TimerId.u32_AMFM_APP_TimerId =0;
			//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 5 successfully stopped  message will not be posted   ");
				}	
			}
			else
			{
				/*MISRAC*/
			}
			
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
/*================================================================================*/
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFTuneHndlr	  */
/*================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret							=	NULL;										 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex				= 0;						 					 /* variable to update the message slot index */
    Tu8										u8_AFListIndex				    = 0;
	Tu8										u8_Best_AF_station_Index	    = 0;
    Tu16                        	        u16_afpi;
	Tbool									b_return_value					= FALSE;
	#ifdef CALIBRATION_TOOL
		Tu8 									u8_Calib_AF_index;
		Tu8										u8_Calib_best_AF_index;
	#endif
	Tbool									b_verfreq						= FALSE;  
	Tu32 									u32_avg 						= 0;
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus  = REPLYSTATUS_REQ_NOT_HANDLED;					/* enum variable to hold the  Play select station reply status*/
	Te_RADIO_ReplyStatus					e_AMFM_APP_AF_CheckReplystatus;
	Te_RADIO_ReplyStatus					e_AMFM_APP_AF_UpdateReplystatus;
	Te_AMFM_App_Regional_PI_Check			e_Regional_PI_Check 			= REGIONAL_PI_NON_COMPITABLE;
	Tbool									b_check_next_best_AF			= FALSE;
	Tu8										u8_NVM_Write			        = 0;
	Tu32 									u32_NVM_Update_Param            = 0;
	Tu8 									u8_NVM_ret                      = 0;
	Tu8										u8_LM_Updation                  = 0;
	Tbool									b_PSNorRT_Changeflag            = FALSE ;
	Tbool									b_retBool                       = FALSE;
	Tu8 									au8_psn[MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME];
	Tu8 									au8_RT[AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT];
	Ts_AMFM_APP_DAB_FollowUp_StationInfo	st_DAB_FollowUp_StationInfo;
	Tu32								    u32_AFupdateFreq                = 0;
	
	pst_me_amfm_app_inst->u32_AMFM_quality=AMFM_APP_CONSTANT_ZERO;

	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:		
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveIdleAFTuneHndlr");
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] current tuned station freq= %d PI= %x ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);

			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_af_tune_state",sizeof("amfm_app_inst_hsm_active_idle_af_tune_state")) ;
						
			#ifdef CALIBRATION_TOOL
				b_start_stop_switch = FALSE;
				u32_calib_freq =0;
				u8_notify_count =0;
				u32_cal_curr=0;
				u32_cal_avg=0;
				u32_cal_old=0;
				
				s32_cal_BBFieldStrength =0 ;
				u32_cal_UltrasonicNoise =0;
				u32_cal_Multipath =0;
				
			#endif
			
			st_AMFM_App_AFList_Info.u8_AF_index =0;
			st_AMFM_App_AFList_Info.u8_AF_Update_Index=0;
			
			if(st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList !=0)
			{
				st_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index=0;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				#endif
			}
			else
			{
				st_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info;
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index=0;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info;
				#endif
			}

			u8_AFListIndex =st_AMFM_App_AFList_Info.u8_AF_Update_Index;
			st_AMFM_App_AFList_Info.u32_af_update_newfreq=st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
		
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq=st_AMFM_App_AFList_Info.pst_current_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index].u32_AF_Freq;
			#endif
			
			AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq);
			/* line to be added*/
			pst_me_amfm_app_inst->u8_Curr_qua_check_count =0 ;
		
		}
		break;

		case AMFM_AF_UPDATE_TIMERID:
		{
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_AMFM_APP_TimerId=0;
			/*Timer started after delay milli SEC*/
			b_return_value = AMFM_APP_AF_Update(&st_AMFM_App_AFList_Info);
		
			if(b_return_value == TRUE )
			{
				/*All stations from AF list are processed */
				u8_af_update_cycle++;
				AMFM_App_Sort_SAME_PI_AF_List(&st_AMFM_App_AFList_Info);
				AMFM_App_Sort_REG_PI_AF_List(&st_AMFM_App_AFList_Info);

				AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AFLIST_SORTED_QUALITIES);

				if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
				{
					/*ENG Mode ON check for changes in quality of AF frequenices*/
					b_return_value = AMFM_APP_ENG_AF_Updation_Check(&(pst_me_amfm_app_inst->st_ENG_AFList_Info),&st_AMFM_App_AFList_Info);
					if (b_return_value == TRUE)
					{
						/*sending ENG notification to radio manager*/
						AMFM_App_Notify_ENG_AFList(&(pst_me_amfm_app_inst->st_ENG_AFList_Info));
					}
				}
				else
				{
					/* ENG MODE switch OFF so nothing to do */
				}
				
				
				pst_me_amfm_app_inst->u32_AF_qual_decr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Existance_decr_alpha;
				pst_me_amfm_app_inst->u32_AF_qual_incr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Existance_incr_alpha;
					
				if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
				{
					/* Transisting to active idle listen state since switch setting turned OFF */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);
				}
				else
				{	
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
					{
						/* DAB Station with same SID is found so notified radio manager for connecting and switch audio to DAB*/
						AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;
						AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
					 		(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
					{
						/* Dab staion quality degraded notify RM to stop dab linking and do audio switch back to FM*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					else
					{
						/*nothing to do */
					}
					/*checking for best AF*/	
				    b_return_value = AMFM_App_SAME_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
					if((b_return_value== FALSE) &&(u8_af_update_cycle >= 3))
					{
						/*minimum number of update cycles are completed*/
						u8_af_update_cycle=u8_af_updatecount;
						/*Best AF available check should be given*/
						AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
					}
					else
					{
						pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
						/*best AF Not availble wait for two seconds and repeat the operation*/
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START( pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
						else
						{
				//			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}							
					}
			
					
				}	
			}
			else
			{
				/*AF update request is successfully processed*/
			}
		}
		break;
		
		case AMFM_CURR_STATION_QUALITY_CHECK_TIMERID:
		{	
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_AMFM_APP_TimerId = 0;
			if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
			{
				/* Transisting to active idle listen state since setting is turned OFF and thus AF process shouldnt be continued*/
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);
			}
			else
			{
				if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
				   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
				   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
				{
					/*give request to radio manager for connecting to DAB*/
					AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;
					AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
				}
				else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
				 		(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
				{
					/*stop dab linking do audio switch*/
					AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
				}
				else
				{
					/*nothing to do */
				}
				if(pst_me_amfm_app_inst->u8_Curr_qua_check_count !=0 )
				{
					if(pst_me_amfm_app_inst->u8_Curr_qua_check_count >= AMFM_APP_CURR_QUAL_CHECK_COUNT) //(u8_Curr_qua_check_count == 4) is fine but for safety measure >= is used
					{
						/* Time duration for AF update arrived */
						pst_me_amfm_app_inst->u8_Curr_qua_check_count=0;
						AMFM_App_AFupdate_Restart(&st_AMFM_App_AFList_Info);
					}
					else
					{
						b_return_value = AMFM_App_SAME_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
						if((b_return_value== FALSE) &&(u8_af_update_cycle >= 3))
						{
							/*minimum number of update cycles are completed*/
							u8_af_update_cycle=u8_af_updatecount;
							/*Best AF available check should be given*/
							AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
							AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AFCHECK_REQUEST);

						}
						else
						{
							/*best AF Not availble wait for two seconds and repeat the operation*/
							pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
							st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
							if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
							else
							{
							//	RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}	
						}	
					}
				}
				else
				{
					/*Quality degraded below threshold*/
					AMFM_App_AFupdate_Restart(&st_AMFM_App_AFList_Info);		
				}
			}				
		}
		break;
		
		case AMFM_REGIONAL_QUALITY_CHECK_TIMERID:
		{
			/* 1 sec quality monitoring for Regional threshold check */
			pst_me_amfm_app_inst->b_RegThresholdCheck=FALSE;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] Regional threshold check timeout reached ");
		
			if((pst_me_amfm_app_inst->e_AF_REGIONAL_Switch == AMFM_APP_AF_REGIONAL_SWITCH_ON) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Regional_Threshold))
			{
				/*Quality is still lesser than regional threshold */
				AMFM_App_Flag_Reset(&st_AMFM_App_AFList_Info);
				/* Transisting to active idle AF reg tune  state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_af_reg_tune_state);
			}
			else
			{
				/*Quality is greater than regional threshold or regional switch is turned OFF*/
				pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
				st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID,RADIO_AM_FM_APP);
				if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
				}
				else
				{
			//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
				}
			}
						
		}
		break;
		
		case AMFM_APP_AFFREQ_UPDATE_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_UpdateReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u32_AFupdateFreq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);

			st_AMFM_App_AFList_Info.b_af_check_flag		= FALSE;
			
			u8_AFListIndex						       = st_AMFM_App_AFList_Info.u8_AF_Update_Index;
			
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.b_af_check_flag   = FALSE;
				u8_Calib_AF_index								 = st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index;
			#endif
	
			if(st_AMFM_App_AFList_Info.u32_af_update_newfreq == u32_AFupdateFreq)
			{
			
				if (e_AMFM_APP_AF_UpdateReplystatus == REPLYSTATUS_SUCCESS)
				{
					pst_me_amfm_app_inst->u32_AMFM_quality =  QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset);

					if(st_AMFM_App_AFList_Info.u32_af_update_newfreq == st_AMFM_App_AFList_Info.u32_curr_freq)
					{
						/* update response for the same Af station since check failed and update should be given to original frequency to demute*/
						st_AMFM_App_AFList_Info.u32_Qua_curr = pst_me_amfm_app_inst->u32_AMFM_quality;
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.u32_Qua_curr   =pst_me_amfm_app_inst->u32_AMFM_quality;
						#endif

						AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
						
						if( (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold) || 
							( (st_AMFM_App_AFList_Info.u32_Qua_old_avg >  st_AMFM_App_AFList_Info.u32_Qua_avg) &&
		 					 ( (st_AMFM_App_AFList_Info.u32_Qua_old_avg - st_AMFM_App_AFList_Info.u32_Qua_avg)>=pst_me_amfm_app_inst->u8_QualityDrop_Margin ) ) ) 
						{
							pst_me_amfm_app_inst->u8_Curr_qua_check_count=0; /*AF update should start in 2 seconds*/
							st_AMFM_App_AFList_Info.b_quality_degraded = TRUE;
							
							#ifdef CALIBRATION_TOOL
								st_Calib_AMFM_App_AFList_Info.b_quality_degraded = TRUE;
							#endif
							pst_me_amfm_app_inst->u32_AF_qual_decr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Modified_decr_alpha;
							pst_me_amfm_app_inst->u32_AF_qual_incr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Modified_incr_alpha;
		
							if((pst_me_amfm_app_inst->e_AF_REGIONAL_Switch == AMFM_APP_AF_REGIONAL_SWITCH_ON) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Regional_Threshold))
							{
								if(pst_me_amfm_app_inst->b_RegThresholdCheck==FALSE)
								{
									/*Regional threshold check timer starting for the first time*/
									pst_me_amfm_app_inst->b_RegThresholdCheck=TRUE;
									st_AMFM_TimerId.u32_AMFM_APP_TimerId= SYS_TIMER_START(AMFM_APP_ONE_SECOND_DELAY,AMFM_REGIONAL_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
									if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_REGIONAL_QUALITY_CHECK_TIMERID);
									}
									else
									{
										//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
									}
									
								}
							}
							else
							{
								/*best AF Not availble wait for two seconds and repeat the operation*/
								st_AMFM_TimerId.u32_AMFM_APP_TimerId= SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
								}
								else
								{
							//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
								}
								
							}
						}
						else
						{
							pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
							
							/*best AF Not availble wait for two seconds and repeat the operation*/
							st_AMFM_TimerId.u32_AMFM_APP_TimerId= SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
							if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
							else
							{
						//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
						}
					}
					else
					{
						/* update response for AF list frequencies*/
						/* QUALITY CHECK SUCCEEDED*/
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].s32_BBFieldStrength  =  pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength ;
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_UltrasonicNoise = pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_Multipath = pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_FrequencyOffset = pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;

						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AFListIndex].u32_curr_qual =pst_me_amfm_app_inst->u32_AMFM_quality;

						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].s32_BBFieldStrength  =  pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_UltrasonicNoise = pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_Multipath = pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_FrequencyOffset = pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_AF_index].u32_curr_qual =pst_me_amfm_app_inst->u32_AMFM_quality;
						#endif
						AMFM_App_AF_Quality_Avg_computation(&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->u32_AF_qual_incr_alpha,pst_me_amfm_app_inst->u32_AF_qual_decr_alpha);
					
						/*update request to next AF freq. start timer to  new update request */
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_Next_Freq_Update_Delay, AMFM_AF_UPDATE_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
						else
						{
					//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
					}
				}
				else 
				{
					/*UPDATE REQUEST FAILED */
					/*Tune ctrl will always give success*/
					if(st_AMFM_App_AFList_Info.u32_af_update_newfreq == (st_AMFM_App_AFList_Info.u32_curr_freq)) 
					{	
						/*best AF Not availble wait for two seconds and repeat the operation*/
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
						else
						{
					//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
						
					}
					else
					{
						/*START TIMER TO GIVE NEXT UPDATE */
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_Next_Freq_Update_Delay, AMFM_AF_UPDATE_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
						else
						{
					//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
					}
				}
			}
			else
			{
				/* AF update response is for other frequency.So dropping this message and waiting for correct response */
			}
		}
		break;
		
		case AMFM_APP_AFFREQ_CHECK_DONE_RESID :
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_CheckReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&(u16_afpi),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			u8_Best_AF_station_Index 				= st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
			u32_avg   								= st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_avg_qual;
	
	
			#ifdef CALIBRATION_TOOL
				u8_Calib_best_AF_index 			    = st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
			#endif
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] During AF,check given for Best AF  freq=%d qual=%d and PI %x received ",st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_avg_qual,u16_afpi);		
			
			if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
			{
				/*tune to original frequency to make soc tune to original station*/
				AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),(st_AMFM_App_AFList_Info.u32_curr_freq));
			}
			else
			{
				if (e_AMFM_APP_AF_CheckReplystatus == REPLYSTATUS_SUCCESS)
				{
				
					st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI	= u16_afpi;
					#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].u16_PI	= u16_afpi;
					#endif
					if( u16_afpi == st_AMFM_App_AFList_Info.u16_curr_PI)
					{
						/*pi matches */
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;	
						#endif
						u8_LM_Updation=1;
						
						AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
					}
					else if ((LIB_AND(u16_afpi, (Tu16)0xf0ff)) == (LIB_AND(st_AMFM_App_AFList_Info.u16_curr_PI, (Tu16)0xf0ff)))
					{
						/*If the PI is regional then after moving to regional list the array content will vary so updating in LM here itself*/
						b_return_value = AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq, u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
						if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
						{
							/*code to store LSM data into NVM */
							u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
							if(u8_NVM_ret == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  during AF check");
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during AF check ");
							}
						}
						
						/* PI NOT SAME  MIGHT BE REGIONAL*/
						e_Regional_PI_Check = AMFM_App_Regional_Pi_Validation(st_AMFM_App_AFList_Info.u16_curr_PI , u16_afpi);
						if(e_Regional_PI_Check == REGIONAL_PI_COMPITABLE)
						{
							/* remove from same program AF list*/
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_REG;
							st_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList] 			= st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index];
							st_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList].b_af_check  = FALSE;
							st_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList++;
							#ifdef CALIBRATION_TOOL
								st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_REG;
								st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList] 			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index];
								st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList].b_af_check  = FALSE;
								st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList++;
								AMFM_App_Calib_Remove_AF_From_List(&st_Calib_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList);
							#endif

							AMFM_App_Sort_REG_PI_AF_List(&st_AMFM_App_AFList_Info);
							AMFM_App_Remove_AF_From_List(&st_AMFM_App_AFList_Info,&st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList);
						}
						else
						{
							/* pi is  not 0 so mark as NEG */
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#ifdef CALIBRATION_TOOL
								st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#endif
							if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)/*to check NEG timer not started*/
							{
								/*timer not started*/
								/*NEG flag set start the timer*/
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck = TRUE;
								st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
								else
								{
							//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
							}
						}
						b_check_next_best_AF = TRUE;
					}
					else
					{
						if(u16_afpi !=0)
						{
							/*different PI so updating in learn memory*/
							u8_LM_Updation=1;
						}
						if((u16_afpi !=0) || ((u16_afpi == 0) && (u32_avg > pst_me_amfm_app_inst->u32_RDS_Senitivity_Threshold)))
						{
							/*totally different  pi or 0 PI with high quality interference signal */
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#ifdef CALIBRATION_TOOL
								st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#endif
							/* pi is  0 or interference signal so mark as NEG   */
							if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)/*to check NEG timer not started*/
							{
								/*timer not started*/
								/*NEG flag set start the timer*/
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;
								st_AMFM_TimerId.u32_Status_Check_TimerId = SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID,RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
								else
								{
			//						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
							}
							b_check_next_best_AF= TRUE;
						}
						else
						{
							/* PI is not decoded and quality is less dont check for best AF and NEG is not marked */
							/* update to original freq to demute  and wait for two seconds and repeat the operation*/
						//	st_AMFM_App_AFList_Info.u32_af_update_newfreq=st_AMFM_App_AFList_Info.u32_curr_freq;
							#ifdef CALIBRATION_TOOL
						//		st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq = st_AMFM_App_AFList_Info.u32_af_update_newfreq;
							#endif
							/*request update for next AF */
						//	AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.u32_af_update_newfreq);
							AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.u32_curr_freq);
						}	
						
					}
				}
				else
				{
					/*check failure usualy control wont come here*/
					b_check_next_best_AF = TRUE;
				}
				if(u8_LM_Updation==1)
				{
					b_return_value=AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq, u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
					if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
					{
						/*code to store LSM data into NVM */
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  during AF check");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during AF check ");
						}
					}
				}
				if(b_check_next_best_AF == TRUE)
				{
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION)&&
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					  (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
					{
						/*give request to radio manager for connecting to DAB*/
						AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;
						AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
					}
					else
					{
						/*nothing to do */
					}
					/*To check for next best AF*/
				
					b_return_value = AMFM_App_SAME_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
					if (b_return_value == FALSE)
					{
						/*Best AF available check should be given*/
						AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
						AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AFCHECK_REQUEST);

					}
					else
					{
						/*best AF not available*/
						/* update to original freq to demute  and wait for two seconds and repeat the operation*/
					//	st_AMFM_App_AFList_Info.u32_af_update_newfreq=st_AMFM_App_AFList_Info.u32_curr_freq;
						#ifdef CALIBRATION_TOOL
					//		st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq = st_AMFM_App_AFList_Info.u32_af_update_newfreq;
						#endif
					
						/*request update for next AF */
					//	AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.u32_af_update_newfreq);
						AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.u32_curr_freq);
					}
				}
			}
		}
		break;

		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{

			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] During AF,Tune given for freq  %d ",pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
	
			if(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	!= (Tu32)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq*(Tu16)10))	
			{
				if (e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
				{
					pst_me_amfm_app_inst->u16_af_Tuned_PI = st_AMFM_App_AFList_Info.u16_curr_PI;
					/* storing current station PSN and RT to a temporary variable to copy back */
					SYS_RADIO_MEMCPY(au8_psn,(const void *)(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					SYS_RADIO_MEMCPY(au8_RT,(const void *)(pst_me_amfm_app_inst->st_current_station.au8_RadioText),AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
					SYS_RADIO_MEMCPY(&st_DAB_FollowUp_StationInfo,(const void *)&(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo),sizeof(st_DAB_FollowUp_StationInfo));
					
					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)/*to check NEG timer  started*/
					{
						/*timer  started*/
						/*stop the timer*/
						if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
						{
						b_return_value =  SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
						if(b_return_value == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
						}
						else
						{
							/* After stop Timer is success,reset the timer id */
							st_AMFM_TimerId.u32_Status_Check_TimerId = 0;
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
							}
						}
						else
						{
							/*MISRAC*/
						}
					}
					
					AMFM_APP_Curr_Station_Related_Clearing(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
				
					pst_me_amfm_app_inst->st_current_station.e_mode = (Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band);
				
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;

					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI		 	 =	pst_me_amfm_app_inst->u16_af_Tuned_PI;
					
					/* copying the psn and RT back to current station */
					SYS_RADIO_MEMCPY((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.au8_psn),(const void *)au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					SYS_RADIO_MEMCPY((pst_me_amfm_app_inst->st_current_station.au8_RadioText),(const void *)au8_RT,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);
					SYS_RADIO_MEMCPY(&(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo),(const void *)&(st_DAB_FollowUp_StationInfo),sizeof(st_DAB_FollowUp_StationInfo));

					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					
					/* Clearing LSM memory for AF list */
					memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
						
					/* Updating Frequency of newly tuned FM station into LSM memory */
					pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
					pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI      = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;
						
					pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
					
					
					AMFM_App_Notify_AF_Status(AMFM_APP_AF_LINK_ESTABLISHED);

					/* Sending Response to Radio manager  only if it is AF switch due to quallity reduction of playing station*/
					AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_current_station);
				
					if(b_NVM_ActiveUpdateSwitch == TRUE)
					{
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0) 
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  during  AF tune");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during  AF tune ");
						}
					}
					/* Transisting to active idle listen state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);	
				}
				else if (e_PlaySelectStationReplyStatus == REPLYSTATUS_NO_SIGNAL)
				{
					/*JUMP REQUEST FAILED*/
					/*tune to original frequency to make soc tune to original station*/
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),st_AMFM_App_AFList_Info.u32_curr_freq);
				}
			}
			else
			{
				/*original frequencies tune response*/
				st_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
				
				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
				#endif
				
				/* Function to read quality of currently tuned station */
				AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
	
				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);

				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
				{
					/*AF setting turned of during normal AF flow so nothing to do  */
					/* Transisting to active idle listen state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);
				}
				else
				{
					if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
				 	   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					  (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
					{
						/*give request to radio manager for connecting to DAB*/
						AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;
						AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
					}
					else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
					 		(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
					{
						/*stop dab linking do audio switch*/
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
					}
					else
					{
						/*nothing to do */
					}
					b_return_value=AMFM_App_SAME_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
					if(b_return_value==FALSE)
					{
						AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
						AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AFCHECK_REQUEST);

					}
					else
					{
						/*best AF Not availble wait for two seconds and repeat the operation*/
						pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
						else
						{
							//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
					}
				}
			}
		}
		break;

		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
			if(	st_AMFM_App_AFList_Info.b_af_check_flag != TRUE)
			{
				AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);
		
				pst_me_amfm_app_inst->st_current_station.s32_BBFieldStrength 		= 	pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
    			pst_me_amfm_app_inst->st_current_station.u32_UltrasonicNoise	   	=	pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
    			pst_me_amfm_app_inst->st_current_station.u32_Multipath		=	pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
    			pst_me_amfm_app_inst->st_current_station.u32_FrequencyOffset		= 	pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset	;

				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
				
				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold)
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
				}
				if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
				{
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
				}
				else
				{
					if(((st_AMFM_App_AFList_Info.u32_Qua_old_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold))
					|| ((st_AMFM_App_AFList_Info.u32_Qua_old_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)))
					{
						AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
					}
					else
					{
						/*nothing to do*/
					}
				}
				if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
				   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
				   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
				{
					/* dab stationinfo that has to be tuned should be given to radio manager */
			
					AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();

					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;

					AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
				}
				else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
				   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
				{
					/*stop dab linking do audio switch*/
					AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
				}
				
				if( ( (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold)&&(st_AMFM_App_AFList_Info.b_quality_degraded!=1) ) ||
				    ( (st_AMFM_App_AFList_Info.u32_Qua_old_avg > st_AMFM_App_AFList_Info.u32_Qua_avg) &&
					  ( (st_AMFM_App_AFList_Info.u32_Qua_old_avg - st_AMFM_App_AFList_Info.u32_Qua_avg) >= pst_me_amfm_app_inst->u8_QualityDrop_Margin ) ) )
				{
					pst_me_amfm_app_inst->u8_Curr_qua_check_count=0;	/*AF update should start in 2 seconds*/
					st_AMFM_App_AFList_Info.b_quality_degraded = TRUE;
					#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.b_quality_degraded = TRUE;
					#endif
					
					pst_me_amfm_app_inst->u32_AF_qual_decr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Modified_decr_alpha;
					pst_me_amfm_app_inst->u32_AF_qual_incr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Modified_incr_alpha;
					
					if((pst_me_amfm_app_inst->e_AF_REGIONAL_Switch == AMFM_APP_AF_REGIONAL_SWITCH_ON) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Regional_Threshold) && 
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status != AMFM_APP_FM_DAB_LINKED)&&
					   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE   == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED))
					{
						if(pst_me_amfm_app_inst->b_RegThresholdCheck==FALSE)
						{
							/*Regional threshold check timer starting for the first time*/
							pst_me_amfm_app_inst->b_RegThresholdCheck=TRUE;
							st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(AMFM_APP_ONE_SECOND_DELAY, AMFM_REGIONAL_QUALITY_CHECK_TIMERID,RADIO_AM_FM_APP);
							if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
							else
							{
						//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
							
						}
					}
					else
					{
						/*best AF Not availble wait for two seconds and repeat the operation*/
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
						else
						{
					//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
					}
				}
				else
				{
					//st_AMFM_App_AFList_Info.b_quality_degraded = FALSE;
				}
				#ifdef CALIBRATION_TOOL
				if(b_start_stop_switch == TRUE)
				{

					b_verfreq=AMFM_APP_VerifyFrequency((u32_calib_freq ),&pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
					if(b_verfreq== TRUE)
					{
						/* Requesting tuner control for tuning to station */
						AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),u32_calib_freq);/*For ETAL*/

						/* Transiting to amfm_app_inst_hsm_active_calibration_start_tune_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_calibration_start_tune_state);
					}
					else
					{
						u32_calib_freq =0;
						b_start_stop_switch = FALSE;
					}
				}
				#endif
			}
		}
		break;
		
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
			if(	st_AMFM_App_AFList_Info.b_af_check_flag != TRUE)
			{
				/* Reading current station info  from the message */
				AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
				/* Function to read quality of currently tuned station */
				AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				
				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength , pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);
		
				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
				{
					pst_me_amfm_app_inst->e_SigQuality	= AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					pst_me_amfm_app_inst->e_SigQuality	= AMFM_APP_GOOD_QUALITY_SIGNAL;
				}
				
				if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi!=0)
				{
					if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != st_AMFM_App_AFList_Info.u16_curr_PI)
					{
						/* PI changed */
						/* PI change detected notifying to RM*/
						AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_DETECTED);
						
						if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)
						{
							st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_PI_CHANGE;
							#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
							#endif
							pst_me_amfm_app_inst->u16_curr_station_pi   = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;
							u8_af_update_cycle=0;
							
							/* can clear current station quality info*/
							/* Transiting to amfm_app_inst_hsm_active_idle_af_strategy_tune_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_strategy_tune_state);
						}
						else 
						{
							/*If AF switch turned OFF Linking will be stopped at that instance itself*/
							/* Updating new PI and AF lists */
						
							AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
							if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
							{
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
								/*Timer wont be started again,can close it*/
								if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
								{
								    b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
									if(b_return_value == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
									}
									else
									{
										st_AMFM_TimerId.u32_Status_Check_TimerId =0;
						//				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
									}
								}	
								else
								{
									/*MISRAC*/
								}
							}
							u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);
							if((u8_NVM_Write==1)&&(b_NVM_ActiveUpdateSwitch == TRUE))
							{
								/*code to store LSM data into NVM */
								u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
								if(u8_NVM_ret == 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success during Tuner status notification ");
								}
								else
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tuner station notification ");
								}
							}
		
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] curretn fm station %d  PI changed to %x ",st_AMFM_App_AFList_Info.u32_curr_freq,st_AMFM_App_AFList_Info.u16_curr_PI);
				
							AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_LINK_ESTABLISHED);	
							/* Sending Response to Radio manager ,No best AF available for the PI changed station. Changed PI and PSN are updated in curr station */
							AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);
							/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
						}
					}
					else
					{
						/*PI not changed*/	
						if((pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList !=0) && (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.b_AF_Checkbit == TRUE))
						{
							if(pst_me_amfm_app_inst->b_AFlist_copy_check == TRUE)
							{
								/*updating AF structure in both tuner status notify structure and current station info */
								/*Function to append AF lsit*/
								pst_me_amfm_app_inst->u32_ReqFrequency=((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ));
								b_return_value = AMFM_App_AF_List_frequency_Append(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_AFeqList,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumofAF_SamePgm,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumofAF_RgnlPgm,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst);
								if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
								{
									/*code to store LSM data into NVM */
									u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
									if(u8_NVM_ret == 0)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  ");
									}
									else
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  ");
									}
								}
								if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)/*to check NEG timer not started*/
								{
									/*timer not started*/
									b_return_value = AMFM_App_NEG_flag_status(&st_AMFM_App_AFList_Info);	
									if(b_return_value ==TRUE)
									{
										/*NEG flag set start the timer*/
										st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
										if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
										{
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
										}
										else
										{
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
										}
									}
								}
							}
						}
						if( ( (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold)&&(st_AMFM_App_AFList_Info.b_quality_degraded != TRUE) ) ||
							( (st_AMFM_App_AFList_Info.u32_Qua_old_avg > st_AMFM_App_AFList_Info.u32_Qua_avg) &&
							  ( (st_AMFM_App_AFList_Info.u32_Qua_old_avg - st_AMFM_App_AFList_Info.u32_Qua_avg) >= pst_me_amfm_app_inst->u8_QualityDrop_Margin ) ) )
						{
							pst_me_amfm_app_inst->u8_Curr_qua_check_count=0;
							st_AMFM_App_AFList_Info.b_quality_degraded = TRUE;
							#ifdef CALIBRATION_TOOL
								st_Calib_AMFM_App_AFList_Info.b_quality_degraded = TRUE;
							#endif
							pst_me_amfm_app_inst->u32_AF_qual_decr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Modified_decr_alpha;
							pst_me_amfm_app_inst->u32_AF_qual_incr_alpha=pst_me_amfm_app_inst->u32_AF_qual_Modified_incr_alpha;
			
							if((pst_me_amfm_app_inst->e_AF_REGIONAL_Switch == AMFM_APP_AF_REGIONAL_SWITCH_ON) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Regional_Threshold) && 
									(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status != AMFM_APP_FM_DAB_LINKED))
							{
								if(pst_me_amfm_app_inst->b_RegThresholdCheck==FALSE)
								{
									/*Regional threshold check timer starting for the first time*/
									pst_me_amfm_app_inst->b_RegThresholdCheck=TRUE;
									st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(AMFM_APP_ONE_SECOND_DELAY, AMFM_REGIONAL_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
									if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Time 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
									}
									else
									{
										//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Time 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
									}
								}
							}
							else
							{
								/*best AF Not availble wait for two seconds and repeat the operation*/
								st_AMFM_TimerId.u32_AMFM_APP_TimerId =SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
								}
								else
								{
									//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
								}
							}
						}
						b_PSNorRT_Changeflag = AMFM_App_PSN_RT_Copy(&(pst_me_amfm_app_inst->st_current_station),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);	
						/*Copying the Program Name */
						AMFM_App_PTY_Copy(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
						AMFM_App_ReadCTinfo(&pst_me_amfm_app_inst->st_RDS_CT_Info,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
						/* Copying TA TP Info */
						AMFM_App_Read_TA_TP_info(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					}
				}
				else
				{
					/* PI not received nothing to do */
				}
				if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_SAME_PI_STATION) && 
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)&&
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.u32_Quality >= pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold))
				{
					/* dab stationinfo that has to be tuned should be given to radio manager */
			
					AMFM_App_Notify_Initiate_FM_DAB_Follow_Up();

					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_DAB_LINKED;

					AMFM_App_Notify_AF_Status(AMFM_APP_DAB_LINK_ESTABLISHED);
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB initialised");
				}
				else if((pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE == AMFM_APP_DAB_PI_STATION_UNIDENTIFIED)&& 
				(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED))
				{
					/*stop dab linking do audio switch*/
					AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_SIG_LOST);
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
				}

				SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/

				/* Updating PI of current station in FM STL */
				if(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI)
				{	
					b_retBool = AMFM_App_Remove_SamePIfromFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI,&st_fm_station_list);
					if(b_retBool == TRUE	)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is updated as same PI station is removed.");

						/* As FM STL is sorted, index value of current station in FM STL might have changed. */
						pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
					}
					st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;

					if(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != 0)
					{
						/* Clearing old PSN as PI of tuned Station is changed */
						memset(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					}

					/*Sending notification to radio manager to inform STL is generated in shared memory */
					AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);
				}

				if( (b_PSNorRT_Changeflag & 0x01)  && (pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL == TRUE) )
				{
					/* Clearingthe flag so that PSN of curr station in FM STL will not be updated further */
					pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL = FALSE;

					SYS_RADIO_MEMCPY(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,(const void *)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

					/* Sorting FM STL as newly seeked FM Station appended into it */
					AMFM_App_application_Remove_SpacefromStationList((st_fm_station_list.ast_Stations),st_fm_station_list.u8_NumberOfStationsInList);
					(void)AMFM_App_application_SortFMStationList(st_fm_station_list.ast_Stations,st_fm_station_list.u8_NumberOfStationsInList);

					/* As FM STL is sorted, index value of current station in FM STL might have changed. */
					pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
					
					/*Sending notification to radio manager to inform STL is generated in shared memory */
					AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);	
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : PSN is added in FM STL. So FM STL is sorted and notified to RM ");				
				}
				
				/* UnLocking the mutex between RM and APP*/
				SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);	
				
				#ifdef CALIBRATION_TOOL
				if(b_start_stop_switch == TRUE)
				{
					b_verfreq=AMFM_APP_VerifyFrequency((u32_calib_freq ),&pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
					if(b_verfreq== TRUE)
					{
						/* Requesting tuner control for tuning to station */
						AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),u32_calib_freq);

						/* Transiting to amfm_app_inst_hsm_active_calibration_start_tune_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_calibration_start_tune_state);
					}
					else
					{
						u32_calib_freq=0;
						b_start_stop_switch = FALSE;
					}
				}

				#endif
				if(st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_PI_CHANGE)
				{
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
				}
			}
		}
		break;
		

		case HSM_MSGID_EXIT:
		{
			/*  free all timers */
			if(st_AMFM_TimerId.u32_AMFM_APP_TimerId > 0)
			{
				b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_AMFM_APP_TimerId);
				if(b_return_value == FALSE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 5 failed to stop message will be posted");
				}
				else
				{
					/* After stop Timer is success,reset the timer id */
					st_AMFM_TimerId.u32_AMFM_APP_TimerId = 0;
			//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 5 successfully stopped  message will not be posted   ");
				}
			}
			else
			{
				/*MISRA C*/	
			}
			
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


/*================================================================================*/
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFRegTuneHndlr  	  */
/*================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleAFRegTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret							=	NULL;										 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex				= AMFM_APP_CONSTANT_ZERO;						 /* variable to update the message slot index */
	Tu8										u8_Best_AF_station_Index	    = AMFM_APP_CONSTANT_ZERO;
    Tu16                        	        u16_afpi;
	Tbool									b_return_value					= FALSE;
	#ifdef CALIBRATION_TOOL
		Tu8									u8_calib_Best_AF_station_Index=0;
	#endif
	Tu8 u32_avg = 0;
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus  = REPLYSTATUS_REQ_NOT_HANDLED;					/* enum variable to hold the  Play select station reply status*/
	Te_RADIO_ReplyStatus					e_AMFM_APP_AF_CheckReplystatus;
	Te_AMFM_App_Regional_PI_Check			e_Regional_PI_Check 			= REGIONAL_PI_NON_COMPITABLE;
	Tbool									b_check_next_best_AF			= FALSE;
	Tu32 									u32_NVM_Update_Param=0;
	Tu8 									u8_NVM_ret=0;
	Tu8 									u8_NEG_Timer_Update=0;
	Tu8										u8_LM_Update=0;
	

	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:		
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveIdleAFRegTuneHndlr");


			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_af_reg_tune_state",sizeof("amfm_app_inst_hsm_active_idle_af_reg_tune_state")) ; 
			
			#ifdef CALIBRATION_TOOL
				b_start_stop_switch = FALSE;
				u32_calib_freq =0;
				u8_notify_count =0;
				u32_cal_curr=0;
				u32_cal_avg=0;
				u32_cal_old=0;

				s32_cal_BBFieldStrength =0 ;
				u32_cal_UltrasonicNoise =0;
				u32_cal_Multipath =0;
				
			#endif
			
	
			st_AMFM_App_AFList_Info.u8_AF_index =0;
			st_AMFM_App_AFList_Info.u8_AF_Update_Index=0;
			
			b_return_value = AMFM_App_SAME_PI_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
			
			if((st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList !=0) && (b_return_value==FALSE))
			{
				st_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				#endif
				u8_Best_AF_station_Index = st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;
				st_AMFM_App_AFList_Info.b_af_check_flag   = TRUE; 
				st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].b_af_check = TRUE;
	
				#ifdef CALIBRATION_TOOL
					u8_calib_Best_AF_station_Index =st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;
					st_Calib_AMFM_App_AFList_Info.b_af_check_flag = TRUE;
					st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].b_af_check = TRUE;
				#endif
				/*Best AF available check should be given*/
				AMFM_Tuner_Ctrl_Request_LowSignal_FM_Check(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
			}
			else
			{
				b_return_value=AMFM_App_REG_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
				if (b_return_value == FALSE)
				{
					AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
				}
				else
				{
					st_AMFM_App_AFList_Info.e_curr_status		=AMFM_APP_AF_SWITCHING;
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status	=st_AMFM_App_AFList_Info.e_curr_status;
					#endif
					/* Transisting to active idle af tune state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_af_tune_state);
				}
			}
			AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AFCHECK_REQUEST);

		}
		break;

		case AMFM_APP_AF_LOW_SIGNAL_CHECK_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_CheckReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&(u16_afpi),(const Tchar *)(pst_msg->data),(Tu8)sizeof(Tu16),&u32_DataSlotIndex);

			st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI	= u16_afpi;
			u8_Best_AF_station_Index 															= st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
			u32_avg   																			= st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_avg_qual;

			#ifdef CALIBRATION_TOOL
				u8_calib_Best_AF_station_Index											 				= st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
				st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].u16_PI	= u16_afpi;
			#endif
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] check given for Best AF  freq=%d qual=%d and PI %x received ",st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_avg_qual,u16_afpi);		
	
			if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
			{
				/*tune to original frequency to make soc tune to original station*/
				AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),(st_AMFM_App_AFList_Info.u32_curr_freq));
			}
			else
			{
				if (e_AMFM_APP_AF_CheckReplystatus == REPLYSTATUS_SUCCESS)
				{
					if( u16_afpi == st_AMFM_App_AFList_Info.u16_curr_PI)
					{
						/*pi matches */
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;

						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;	
						#endif
						AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);

						u8_LM_Update=1;
					}
					else if ((LIB_AND(u16_afpi, (Tu16)0xf0ff)) == (LIB_AND(st_AMFM_App_AFList_Info.u16_curr_PI, (Tu16)0xf0ff)))
					{

						/*Updating the PI in Learnmemory*/
						/* If it is regional PI then on moving and soring the list the station position will vary so writing in LM immediately*/
						b_return_value = AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq, u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
						if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
						{
							/*code to store LSM data into NVM */
							u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
							if(u8_NVM_ret == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during AF check during AF check in Regtune handler ");
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during AF check during AF check in Regtune handler");
							}
						}
						/* PI NOT SAME  MIGHT BE REGIONAL*/
						e_Regional_PI_Check = AMFM_App_Regional_Pi_Validation(st_AMFM_App_AFList_Info.u16_curr_PI , u16_afpi);
						if(e_Regional_PI_Check == REGIONAL_PI_COMPITABLE)
						{
							/* remove from same program AF list*/
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS 						= AMFM_APP_PI_STATUS_REG;
							st_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList] 			= st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index];
							st_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList].b_af_check = FALSE;
							st_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList++;

							#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS									= AMFM_APP_PI_STATUS_REG;
							st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList] 			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index];
							st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList].b_af_check = FALSE;
							st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList++;
							AMFM_App_Calib_Remove_AF_From_List(&st_Calib_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList);
							#endif

							AMFM_App_Sort_REG_PI_AF_List(&st_AMFM_App_AFList_Info);
							AMFM_App_Remove_AF_From_List(&st_AMFM_App_AFList_Info,&st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList);
						}
						else
						{
							/* pi is  not 0 so mark as NEG */
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#endif
							u8_NEG_Timer_Update=1;
						}
						b_check_next_best_AF= TRUE;
					}
					else
					{
						/*pi doesnt matches area code differs ,totally different  pi or 0 pi*/
						if((u16_afpi !=0) || ((u16_afpi == 0) && (u32_avg > pst_me_amfm_app_inst->u32_RDS_Senitivity_Threshold)))
						{
							/*totally different  pi or 0 PI with high quality interference signal */
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#endif

							if(u16_afpi!=0)
							{
								u8_LM_Update=1;
							}	
							u8_NEG_Timer_Update=1;	
						}					
						b_check_next_best_AF = TRUE;
					}
				}
				else
				{
					/*check failure usualy control wont come here*/
					b_check_next_best_AF = TRUE;
				}
				if(u8_LM_Update==1)
				{
					/* Learm memory updation with the received PI*/
					b_return_value = AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq, u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
					if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch == TRUE))
					{
						/*code to store LSM data into NVM */
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during AF check during AF check in Regtune handler ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during AF check during AF check in Regtune handler");
						}
					}
				}
				if((u8_NEG_Timer_Update==1)&&(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE))/*to check NEG timer not started*/
				{
					/*timer not started*/
					/*NEG flag set start the timer*/
					pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;
					st_AMFM_TimerId.u32_Status_Check_TimerId = SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
					if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
					}
					else
					{
						//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
					}
				}
				if(b_check_next_best_AF == TRUE)
				{
					/*one second completed */
					/*check for regional*/
					b_return_value=AMFM_App_REG_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
					if(b_return_value==FALSE)
					{
						AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
						AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AFCHECK_REQUEST);
					}
					else
					{
						/*tune to original frequency to make soc tune to original station*/
						AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),st_AMFM_App_AFList_Info.u32_curr_freq);
					}
				}
			}
		}
		break;
		
		
		case AMFM_APP_AFFREQ_CHECK_DONE_RESID :
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_CheckReplystatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&(u16_afpi),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI	= u16_afpi;
			u8_Best_AF_station_Index 				= st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
   			u32_avg   								= st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_avg_qual;

			#ifdef CALIBRATION_TOOL
				u8_calib_Best_AF_station_Index 																	= st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
				st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].u16_PI	= u16_afpi;
			#endif
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]check given for Best AF  freq=%d qual=%d and PI %x received ",st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_avg_qual,u16_afpi);		
	
			
			if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
			{
				
				/*tune to original frequency to make soc tune to original station*/
				AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),st_AMFM_App_AFList_Info.u32_curr_freq);
			}
			else
			{
				/*check response for regional tune*/
				if (e_AMFM_APP_AF_CheckReplystatus == REPLYSTATUS_SUCCESS)
				{
					if( u16_afpi == st_AMFM_App_AFList_Info.u16_curr_PI)
					{
						
						b_return_value =	AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
						if((b_return_value== TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
						{
							/*code to store LSM data into NVM */
							u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
							if(u8_NVM_ret == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during AF check during AF check in Regtune handler ");
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during AF check during AF check in Regtune handler");
							}
						}
						/*pi matches */
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
						st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList] 			= st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index];
						st_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList].b_af_check = FALSE;
						st_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList++;
					
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;	
						st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList] 			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index];
						st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList].b_af_check = FALSE;
						st_Calib_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList++;
						AMFM_App_Calib_Remove_AF_From_List(&st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList);
						#endif
						AMFM_App_Sort_SAME_PI_AF_List(&st_AMFM_App_AFList_Info);
						AMFM_App_Remove_AF_From_List(&st_AMFM_App_AFList_Info,&st_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList);
						AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
					}
					else if ((LIB_AND(u16_afpi, (Tu16)0xf0ff)) == (LIB_AND(st_AMFM_App_AFList_Info.u16_curr_PI, (Tu16)0xf0ff)))
					{
					
						/* PI NOT SAME  MIGHT BE REGIONAL*/
						e_Regional_PI_Check = AMFM_App_Regional_Pi_Validation(st_AMFM_App_AFList_Info.u16_curr_PI , u16_afpi);
						if(e_Regional_PI_Check == REGIONAL_PI_COMPITABLE)
						{
							/* Regional PI*/
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_REG;
							#ifdef CALIBRATION_TOOL
								st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_REG;
							#endif
							AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
				
						}
						else
						{
							/* pi is  not regional so mark as NEG */
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#endif
							u8_NEG_Timer_Update=1;
							b_check_next_best_AF=TRUE;
						}
						u8_LM_Update=1;
						
					}
					else
					{
						/*pi doesnt matches area code differs ,totally different  pi or 0 pi*/
						if((u16_afpi !=0) || ((u16_afpi == 0) && (u32_avg > pst_me_amfm_app_inst->u32_RDS_Senitivity_Threshold)))
						{
							/*totally different  pi or 0 PI with high quality interference signal */
							st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#ifdef CALIBRATION_TOOL
								st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							#endif
							
							if(u16_afpi!=0)
							{
								u8_LM_Update=1;
								b_check_next_best_AF=TRUE;
							}	
							else
							{
								/*If best AF is decoded 0 PI with quality High .No need to check for next best AFs*/
								/*tune to original frequency to make soc tune to original station*/
								AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),st_AMFM_App_AFList_Info.u32_curr_freq);
							}
							u8_NEG_Timer_Update=1;						
						}
						else
						{
							/*tune to original frequency to make soc tune to original station*/
							AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),st_AMFM_App_AFList_Info.u32_curr_freq);
						}					
					}
				}
				else
				{
					/*check failure usualy control wont come here*/
					b_check_next_best_AF = TRUE;
				}
				if(u8_LM_Update==1)
				{
					/* Learm memory updation with the received PI*/
					b_return_value = AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq, u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
					if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
					{
						/*code to store LSM data into NVM */
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during AF check during AF check in Regtune handler ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during AF check during AF check in Regtune handler");
						}
					}
				}
			
				if((u8_NEG_Timer_Update==1)&&(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE))/*check NEG timer not started*/
				{
					/*timer not started*/
					/*NEG flag set start the timer*/
					pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;
					st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
					if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
					}
				}
				if(b_check_next_best_AF == TRUE)
				{
					
					b_return_value = AMFM_App_REG_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
					if (b_return_value == FALSE)
					{
						st_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info;
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info;
						#endif
						/*Best AF available check should be given*/
						AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
						AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,AMFM_APP_AFCHECK_REQUEST);

					}
					else
					{
						/*tune to original frequency to make soc tune to original station*/
						AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),st_AMFM_App_AFList_Info.u32_curr_freq);
					}
				}
			}
		}
		break;

		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{

			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
			u8_Best_AF_station_Index 				= st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] During AF,Tune given for freq  %d ",pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
	
			if(((st_AMFM_App_AFList_Info.u32_curr_freq))	!= (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq))	
			{
				if (e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
				{
			
					if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)&&(pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
					   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
						(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
					{
							/*DAB linked then stop DAB FM linking*/
							/*stop dab linking to do audio switch*/
							AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_PI_CHANGE);
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
							/*send new PI to DAB*/
					}
							
					pst_me_amfm_app_inst->u16_af_Tuned_PI   = st_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI;
			
					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)/*to check NEG timer  started*/
					{
						/*timer  started*/
						/*stop the timer*/
						if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
						{
							b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
							if(b_return_value == FALSE)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
							}
							else
							{
								/* After stop Timer is success,reset the timer id */
								st_AMFM_TimerId.u32_Status_Check_TimerId = 0;
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
							}
						}
						else
						{
							/*MISRAC*/
						}
					}
					AMFM_APP_Curr_Station_Related_Clearing(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_STATION_TUNED;
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE   = AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
					
					pst_me_amfm_app_inst->st_current_station.e_mode = (Te_AMFM_App_mode)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;
				
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;

					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI		 	 =			pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;

					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&(pst_me_amfm_app_inst->st_current_station),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					
					/* Clearing LSM memory for AF list */
					memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
						
					/* Updating Frequency of newly tuned FM station into LSM memory */
					pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
					pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI      = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;
						
					pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
					
					AMFM_App_Notify_AF_Status(AMFM_APP_AF_LINK_ESTABLISHED);
		
					/* Sending Response to Radio manager  only if it is AF switch due to quallity reduction of playing station*/
					AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_current_station);
					
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  tuned to Af frequency  ");
					/*code to store LSM data into NVM */
					if(b_NVM_ActiveUpdateSwitch == TRUE)
					{
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0) 
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success reg AF station tuned ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails reg AF tune AF station tuned ");
						}
					}
					/* Transisting to active idle listen state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);	
				}
				else if (e_PlaySelectStationReplyStatus == REPLYSTATUS_NO_SIGNAL)
				{
					/*JUMP REQUEST FAILED*/
					/*tune to original frequency to make soc tune to original station*/
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),st_AMFM_App_AFList_Info.u32_curr_freq);
				
				}
			}
			else
			{
				/*original frequencies tune response*/
				st_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
				
				#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
				#endif
				
				/* Function to read quality of currently tuned station */
				AMFM_App_ReadQuality(&(pst_me_amfm_app_inst->st_current_station),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
	
				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);

				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				if(pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_OFF) 
				{
					
					/*AF setting turned of during normal AF flow so nothing to do  */
					/* Transisting to active idle listen state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);
				}
				else
				{
					b_return_value = AMFM_App_REG_PI_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
					if (b_return_value == FALSE)
					{
						AMFM_APP_Curr_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
					}
					else
					{
						st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_AF_SWITCHING;
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
						#endif
						/* Transisting to active idle af tune state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_af_tune_state);
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
			pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}	

#if 0
/*====================================================================================================*/
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleEONStationAnnouncementHndlr	  		      */
/*====================================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleEONStationAnnouncementHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret							=	NULL;								  /* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus	= REPLYSTATUS_REQ_NOT_HANDLED;				/* enum variable to hold the  Play select station reply status*/
	Te_RADIO_ReplyStatus					e_AMFM_APP_AF_CheckReplystatus;
	Te_RADIO_ReplyStatus					e_AMFM_APP_AF_UpdateReplystatus;
	Te_RADIO_ReplyStatus					e_SeekReplyStatus				= REPLYSTATUS_REQ_NOT_HANDLED;				/* enum variable to hold the  seek reply status*/
	Te_RADIO_ReplyStatus					e_AnnoCancelReplyStatus         = REPLYSTATUS_REQ_NOT_HANDLED;
	Tu32									u32_DataSlotIndex				= 0;			      					 /* variable to update the message slot index */
	Ts_AMFM_App_StationInfo				*pst_StationInfo					= NULL;	  								/* Pointer to either Current station Info /EON station Info structure present inside  Ts_AMFM_App_inst_hsm*/
	Te_AMFM_App_Select_Station_Response_Flag	e_Select_Station_Response_flag = AMFM_APP_INVALID;					/* Enum tells whether select station response is for EON/Original Station */
	Tbool 	b_PI_availability;
	Tbool	b_AF_availability;
	Tu8 u8_EONlist_PI_index;
	Tu8 u8_EON_AF_List_Index;
	Tu8 u8_EON_TA;
	Tu8 u8_EON_TP;
	Tu8 u8_EON_stopped=0;
	Tu16 u16_pi;
	Tu16	u32_AFupdateFreq = 0;

	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:		
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveIdleEONStationAnnouncementHndlr");

			
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_eon_station_announcement_state",sizeof("amfm_app_inst_hsm_active_idle_eon_station_announcement_state")) ;
			
			/* code to tune to  TA frequency*/
			b_PI_availability = AMFM_APP_Check_PI_Availability(&(pst_me_amfm_app_inst->st_EON_List),pst_me_amfm_app_inst->u16_EON_TA_Station_PI);
			if(b_PI_availability == TRUE)
			{
				/* PI is available in the lsitcode to give update request */
				u8_EONlist_PI_index=AMFM_APP_AvailablePI_Index(&(pst_me_amfm_app_inst->st_EON_List),pst_me_amfm_app_inst->u16_EON_TA_Station_PI);
				pst_me_amfm_app_inst->st_EON_List.u8_EONlist_PIindex=u8_EONlist_PI_index;
				pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex=0;
				u8_EON_AF_List_Index=pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex;
				AMFM_Tuner_Ctrl_Request_AF_Update(pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index]);
			}
			else
			{
				/* function to start PI seek*/
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] EON PI not available in EON PI list so doing PI seek");

				AMFM_APP_Start_Seek(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&(pst_me_amfm_app_inst->st_MarketInfo),pst_me_amfm_app_inst->u16_EON_TA_Station_PI);
			}
		}
		break;
		
		case AMFM_APP_ANNOUNCEMENT_CANCEL_REQID:
		{
			AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->e_Anno_Cancel_Request),(const Tchar *)(pst_msg->data), sizeof(Te_AMFM_App_Anno_Cancel_Request), &u32_DataSlotIndex);

			AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_USER_CANCEL);
			
			if(pst_me_amfm_app_inst->e_Anno_Cancel_Request == AMFM_APP_ANNO_CANCEL_BY_NEW_REQUEST)
			{
				/* Reuesting Tuner Ctrl for Anno cancel */
				AMFM_Tuner_Ctrl_Request_Announcement_Cancel();				
			}
			else if(pst_me_amfm_app_inst->e_Anno_Cancel_Request == AMFM_APP_ANNO_CANCEL_BY_USER)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcment cancelled by user ");
				
				/* Requesting tuner control for tuning to original Station */
				AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency);	
			}
			else{/*FOR MISRA C*/}			
		}
		break;
		
		case AMFM_APP_ANNO_CANCEL_DONE_RESID:
		{	
			AMFM_APP_ExtractParameterFromMessage(&e_AnnoCancelReplyStatus,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_RADIO_ReplyStatus),&u32_DataSlotIndex);
			
			pst_me_amfm_app_inst->e_Anno_Cancel_Request = AMFM_APP_NO_ANNO_CANCEL;		

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcment cancelled by new req ");
				
			AMFM_App_Response_Anno_Cancel_Done(e_AnnoCancelReplyStatus);
				
			/*Transisting to active idle state*/					
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_state);		
		}
		break;
		
		case AMFM_APP_CURRENT_STATION_TA_TP_NOTIFIED:
		{
			AMFM_APP_ExtractParameterFromMessage(&u8_EON_TA,(const Tchar *)(pst_msg->data),(Tu16)sizeof(u8_EON_TA),&u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u8_EON_TP,(const Tchar *)(pst_msg->data),(Tu16)sizeof(u8_EON_TP),&u32_DataSlotIndex);	
			
			if(pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u32_frequency != 0	)
			{
				/* TA info should be updated for the switched EON station only*/
				/* EON station info frequency parameter will be updated only after switching*/
				pst_me_amfm_app_inst->st_EON_station_Info.u8_TA = u8_EON_TA;

				pst_me_amfm_app_inst->st_EON_station_Info.u8_TP = u8_EON_TP;
			
				if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
				{
					/* Notifying st_current_station Info as there is change in TA/TP flag */
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_EON_station_Info,pst_me_amfm_app_inst->e_SigQuality);	
				}
			
				if((u8_EON_TA== 1 && u8_EON_TP==1) != 1 )
				{
					AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_COMPLETED);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcement ended TA =%d TP=%d  ",u8_EON_TA,u8_EON_TP);
			
					/* code to switch back to original frequency */
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency )));	
				}
				else
				{
					/*do nothing*/
				}
			}
		}
		break;

		case AMFM_APP_AFFREQ_UPDATE_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_UpdateReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u32_AFupdateFreq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);

			u8_EONlist_PI_index=pst_me_amfm_app_inst->st_EON_List.u8_EONlist_PIindex;


			if (e_AMFM_APP_AF_UpdateReplystatus == REPLYSTATUS_SUCCESS)
			{
				
				u8_EON_AF_List_Index =pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex;

				if(pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index]==u32_AFupdateFreq )
				{
					pst_me_amfm_app_inst->u32_AMFM_quality =QUAL_CalcFmQual(	pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, 	pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, 	pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset);
				
					if(pst_me_amfm_app_inst->u32_AMFM_quality > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold ) 
					{
						/* EONAF quality is greater than current tuned freq*/
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] EON AFcheck req for freq=%d with qual=%d  ",pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index],pst_me_amfm_app_inst->u32_AMFM_quality);

					AMFM_Tuner_Ctrl_Request_FM_Check(pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index]);
					}
					else
					{
						/* QUALITY CHECK FAILED*/
						AMFM_APP_New_EONList_AF_update(&(pst_me_amfm_app_inst->st_EON_List),(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ),&(pst_me_amfm_app_inst->st_MarketInfo));
					}
				}
			}
			else 
			{
				/*UPDATE REQUEST FAILED */
				AMFM_APP_New_EONList_AF_update(&(pst_me_amfm_app_inst->st_EON_List),(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ),&(pst_me_amfm_app_inst->st_MarketInfo));	
			}
		}
		break;
		
		case AMFM_APP_AFFREQ_CHECK_DONE_RESID :
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_CheckReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&(u16_pi),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] EON AFcheck done %x pi received ",u16_pi);

			u8_EONlist_PI_index=pst_me_amfm_app_inst->st_EON_List.u8_EONlist_PIindex;
			if (e_AMFM_APP_AF_CheckReplystatus == REPLYSTATUS_SUCCESS)
			{
				if( u16_pi == (pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u16_EON_PI))
				{
					/*pi matches */
					u8_EON_AF_List_Index=pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex;
					AMFM_Tuner_Ctrl_Request_FM_Jump(pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index]);
				}
				else 
				{
					/* PI NOT SAME*/
					AMFM_APP_New_EONList_AF_update(&(pst_me_amfm_app_inst->st_EON_List),(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ),&(pst_me_amfm_app_inst->st_MarketInfo));
				}
			}
			else
			{
				/*if check request failed */
				AMFM_APP_New_EONList_AF_update(&(pst_me_amfm_app_inst->st_EON_List),(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ),&(pst_me_amfm_app_inst->st_MarketInfo));
			}
		}
		break;

		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);	

			if( ((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency )) == pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq )	     
			{
				/* As frequency received from Tuner Control App is same as original tuned frequency,making Pointer pst_StationInfo points to Current station Info Structure */
				pst_StationInfo				= &pst_me_amfm_app_inst->st_current_station;

				/* Updating e_Select_Station_Response_flag as Original station */
				e_Select_Station_Response_flag	= AMFM_APP_ORIGINAL_STATION;	
			}
			else
			{
				/* Making Pointer pst_StationInfo points to EON station Info Structure */
				pst_StationInfo				= &pst_me_amfm_app_inst->st_EON_station_Info;

				/* Updating e_Select_Station_Response_flag as EON station */
				e_Select_Station_Response_flag	= AMFM_APP_EON_STATION;
				/* Clearing structure pointed by pst_StationInfo */
//				memset((void *)pst_StationInfo,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_StationInfo));
				
			}
			
			if (e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
			{	
				pst_StationInfo->e_mode									 = (Te_AMFM_App_mode)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;
				pst_StationInfo->un_station.st_FMstation.u32_frequency	 = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq ;
				if(	e_Select_Station_Response_flag	!= AMFM_APP_ORIGINAL_STATION)
				{
					pst_StationInfo->un_station.st_FMstation.u16_PI			 =		 pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;
				}
				/* Function to read quality of either EON station/original tuned station */
				AMFM_App_ReadQuality(pst_StationInfo,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				
				/* Announcement cancel response will be sent only for cancel request */
				if(pst_me_amfm_app_inst->e_Anno_Cancel_Request != AMFM_APP_NO_ANNO_CANCEL)
				{
					AMFM_App_Response_Anno_Cancel_Done(REPLYSTATUS_SUCCESS);
					
					pst_me_amfm_app_inst->e_Anno_Cancel_Request = AMFM_APP_NO_ANNO_CANCEL;					
				}	

				if(e_Select_Station_Response_flag == AMFM_APP_EON_STATION)
				{
					/* Announcement is started in EON station. Updating its TA and TP flag as set */
					pst_StationInfo->u8_TA = 1;
					pst_StationInfo->u8_TP = 1;

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcement station freq %d tuned  ",pst_StationInfo->un_station.st_FMstation.u32_frequency);

				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcement ended tuned back to original station freq %d  ",pst_StationInfo->un_station.st_FMstation.u32_frequency);

					/* Tuned to original frequency and transisting to active idle listen state */					
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);	
				}
				
				/* Sending Response to Radio manager */
				AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,*pst_StationInfo);
				
			}
			else 
			{
				/* Jump request Failed */
				if(e_Select_Station_Response_flag == AMFM_APP_EON_STATION)
				{
					/* Looking next available frequency */
					AMFM_APP_New_EONList_AF_update(&(pst_me_amfm_app_inst->st_EON_List),(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ),&(pst_me_amfm_app_inst->st_MarketInfo));
				}
				else
				{
					/* Tuned to original frequency and so clearing st_EON_station_Info structure  */
	//				memset((void *)&pst_me_amfm_app_inst->st_EON_station_Info,AMFM_APP_CONSTANT_ZERO,sizeof(pst_me_amfm_app_inst->st_EON_station_Info));

					/* Function to read quality of original tuned station (TN) */
					AMFM_App_ReadQuality(pst_StationInfo,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcement ended tuned back to original station freq %d  ",pst_StationInfo->un_station.st_FMstation.u32_frequency);
					
					/* Sending response of current station Info(TN)  to Radio Manager Component */
					AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,*pst_StationInfo);
					
					/* Transisting to active idle listen state */				
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);	
				}
			}
		
		}
		break;
		
		case AMFM_APP_SEEK_UP_DOWN_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_SeekReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);	
			
			/* clearing current station info */
		//	memset(&pst_me_amfm_app_inst->st_EON_station_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_StationInfo));
		
            pst_me_amfm_app_inst->st_EON_station_Info.e_mode = (Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band);	

			if (e_SeekReplyStatus == REPLYSTATUS_SUCCESS)
			{
		    
			  	pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u32_frequency	 =  (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;
				pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u16_PI		 	 =			pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;


				/* Function to read quality of currently tuned station */
				AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_EON_station_Info,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				
				pst_me_amfm_app_inst->st_EON_station_Info.u8_charset	= pst_me_amfm_app_inst->u8_charset;
				e_PlaySelectStationReplyStatus = REPLYSTATUS_SUCCESS;
				
				b_PI_availability=AMFM_APP_Check_PI_Availability(&(pst_me_amfm_app_inst->st_EON_List),pst_me_amfm_app_inst->u16_EON_TA_Station_PI);
				if(b_PI_availability == TRUE)
				{
					/* PI is available in the list*/
					u8_EONlist_PI_index=AMFM_APP_AvailablePI_Index(&(pst_me_amfm_app_inst->st_EON_List),pst_me_amfm_app_inst->u16_EON_TA_Station_PI);
					pst_me_amfm_app_inst->st_EON_List.u8_EONlist_PIindex=u8_EONlist_PI_index;
					b_AF_availability=AMFM_APP_Check_EON_AF_Availability(&(pst_me_amfm_app_inst->st_EON_List),(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq));
					if(b_AF_availability == TRUE)
					{
						pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex = AMFM_APP_EON_AF_Index(&(pst_me_amfm_app_inst->st_EON_List),(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq));
					}
					else
					{
						u8_EON_AF_List_Index = pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_AF_Count;
						pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex = u8_EON_AF_List_Index;
						pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index] = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq;
						pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_AF_Count++;
					}
				}
				else
				{
					u8_EON_AF_List_Index=0;
					u8_EONlist_PI_index=pst_me_amfm_app_inst->st_EON_List.u8_PI_Count++;
					pst_me_amfm_app_inst->st_EON_List.u8_EONlist_PIindex = u8_EONlist_PI_index;
					pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_AF_Count=0;
					pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u16_EON_PI  = pst_me_amfm_app_inst->u16_EON_TA_Station_PI;
					pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex = u8_EON_AF_List_Index;
					pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index] = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq;
					pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u8_AF_Count++;
				}

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : EON Announcement seeked to  freq %d  ",pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u32_frequency);
		
				/* Sending Response to Radio manager */
				AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_EON_station_Info);
				
			}
			else if (e_SeekReplyStatus == REPLYSTATUS_NO_SIGNAL)
			{
				pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u32_frequency	 = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;
				pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u16_PI		 =			pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;

				AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_NOT_AVAILABLE);	
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : EON Announcement seek failed");
				
				/* code to switch back to original frequency */
				AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency )));	
			}
		}
		break;



		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
            /* Reading current station info  from the message */
            AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);

			if(pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u32_frequency != 0	)
			{
				pst_me_amfm_app_inst->st_EON_station_Info.s32_BBFieldStrength 		= 	pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
	    		pst_me_amfm_app_inst->st_EON_station_Info.u32_UltrasonicNoise	    =	pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
	    		pst_me_amfm_app_inst->st_EON_station_Info.u32_Multipath		=	pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
	    		pst_me_amfm_app_inst->st_EON_station_Info.u32_FrequencyOffset		= 	pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;

				u8_EONlist_PI_index=pst_me_amfm_app_inst->st_EON_List.u8_EONlist_PIindex;
				pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);

			    AMFM_App_Eon_Station_Qual_Avg_Computation(&(pst_me_amfm_app_inst->st_EON_List));
				
				if(pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
				}
				else
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
				}
				if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
				{
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_EON_station_Info,pst_me_amfm_app_inst->e_SigQuality);
				}
				else
				{
					if(((pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_old_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold))
					|| ((pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_old_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)))
					{
						//pst_me_amfm_app_inst->u8_QualDegradedNotify_Flag=1;	
						AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_EON_station_Info,pst_me_amfm_app_inst->e_SigQuality);
					}
					else
					{
						/*nothing to do*/
					}
				}

	
				if((pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) ||(pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF) ||  (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
				{
					if((pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF) ||(pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
					{
						/* Api to command audio manager to change program audio level*/
						AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_TA_SWITCH_OFF);

						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcment cancelled by RDS/TA switch OFF");

					}	
					else
					{
						/* Api to command audio manager to change program audio level*/
						AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_SIGNAL_LOSS);

						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcment cancelled due to signal loss");

					}
						
					/* code to switch back to original frequency */
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency )));
			
				}
				else
				{
					/*nothing to do */
				}
			}
        }
		break;

		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			
			/* Reading current station info  from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
			
			if(pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u32_frequency != 0	)
			{
				u8_EONlist_PI_index=pst_me_amfm_app_inst->st_EON_List.u8_EONlist_PIindex;
				pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength , pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);

			    AMFM_App_Eon_Station_Qual_Avg_Computation(&(pst_me_amfm_app_inst->st_EON_List));
					
				if(pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u16_PI!=0)
				{
					if( pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != 0)
					{
						if(pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u16_PI	!=  pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi)
						{
							u8_EON_stopped=1;
							/* Api to command audio manager to change program audio level*/
							AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_SIGNAL_LOSS);
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcment cancelled due to PI change of EON station from %x to %x",pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u16_PI,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);
							/* code to switch back to original frequency */
							AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency);	
				
						}
					}
					
				}
				else
				{
					if( pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != 0)
					{	
						pst_me_amfm_app_inst->st_EON_station_Info.un_station.st_FMstation.u16_PI		=  pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi ;
					}
				}

				if(u8_EON_stopped!=1)
				{
					if((pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)||(pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF) || (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
					{
					
						if((pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF)|| (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
						{
							/* Api to command audio manager to change program audio level*/
							AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_TA_SWITCH_OFF);
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcment cancelled due to RDS/TA switch OFF");
						}	
						else
						{
							/* Api to command audio manager to change program audio level*/
							AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_SIGNAL_LOSS);
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  FG : EON Announcment cancelled due to signal loss");
						}
				
						/* code to switch back to original frequency */
						AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency )));	
						pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
					}
					else
					{
						pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
					}
		
					/* Copying the RadioText and PSN to st_EON_station_Info structure present in Ts_AMFM_App_inst_hsm structure */	
					(void)AMFM_App_PSN_RT_Copy(&(pst_me_amfm_app_inst->st_EON_station_Info),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					/*Copying the PTY name*/
					AMFM_App_PTY_Copy(&(pst_me_amfm_app_inst->st_EON_station_Info),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);	
					/* Copying TA TP Info */
					AMFM_App_Read_TA_TP_info(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					/* Sending notification to Radio Manager Application */	
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_EON_station_Info,pst_me_amfm_app_inst->e_SigQuality);
				}
			}
		}
		break;

		case AMFM_APP_TUNER_AMFMTUNER_ABNORMAL_NOTIFYID:
		{	
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][AMFM_APP] Power on reset occured "); 
			
			/* Notifying Announcement is ended */
			AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_COMPLETED);

			/* Sending Response to Radio manager */
			AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_current_station);
			
			/* Notifying AMFM TUNER Abnormality to Radio Manager component */
			AMFM_App_Notify_AMFMTuner_Abnormal(pst_me_amfm_app_inst->e_AMFMTunerStatus);

			/* Transisting to active idle state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_state);	
		}
		break;
		
		case AMFM_APP_EON_ANNOUNCEMENT_START_NOTIFIED:
		case AMFM_APP_EON_INFO_NOTIFIED:
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
	
/*====================================================================================================*/
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleProgramStationAnnouncementHndlr	  		  */
/*====================================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveIdleProgramStationAnnouncementHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret							=	NULL;										 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex				= 0;											 /* variable to update the message slot index */
	Tu8 u8_currentTA;
	Tu8 u8_currentTP;
	Tu32 									u32_NVM_Update_Param=0;
	Tu8 									u8_NVM_ret=0;
	Tbool									b_return_value= FALSE;
	Tu8										u8_NVM_Write=0;
	Tbool									b_PSNorRT_Changeflag   = FALSE;
	Tu8										u8_Announcement_stopped=0;
	Tu8										u8_af_appendCheck = 0;
//	Tu8										u8_UpdatePIinSTL  = 0;
	Tbool									b_retBool  = FALSE;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:		
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveIdleProgramStationAnnouncementHndlr");

			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_idle_program_station_announcement_state",sizeof("amfm_app_inst_hsm_active_idle_program_station_announcement_state")) ;
		}
		break;


		case AMFM_APP_CURRENT_STATION_TA_TP_NOTIFIED:
		{
			AMFM_APP_ExtractParameterFromMessage(&u8_currentTA,(const Tchar *)(pst_msg->data),(Tu16)sizeof(u8_currentTA),&u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u8_currentTP,(const Tchar *)(pst_msg->data),(Tu16)sizeof(u8_currentTP),&u32_DataSlotIndex);	

			
			pst_me_amfm_app_inst->st_current_station.u8_TA	= u8_currentTA;

			pst_me_amfm_app_inst->st_current_station.u8_TP	= u8_currentTP;
			
			if((u8_currentTA== 1 && u8_currentTP==1) != 1)
			{
				/* Api to command audio manager to increase program audio level*/
				AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_COMPLETED);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcement ended  ");
				
				/* Sending Response to Radio manager */
				AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS, pst_me_amfm_app_inst->st_current_station);

				/* Transiting back to ative idle listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
			}
		}
		break;
	
		case AMFM_APP_ANNOUNCEMENT_CANCEL_REQID:
		{
			
				/* Api to command audio manager to increase program audio level*/
				AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_USER_CANCEL);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcment cancelled by User ");

				
				AMFM_App_Response_Anno_Cancel_Done(REPLYSTATUS_SUCCESS);
					
				/* Sending Response to Radio manager */
				AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS, pst_me_amfm_app_inst->st_current_station);

				/* Transiting back to active idle listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
		}
		break;
		
		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
            /* Reading current station info  from the message */
       		AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);

			pst_me_amfm_app_inst->st_current_station.s32_BBFieldStrength 		= 	pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
    		pst_me_amfm_app_inst->st_current_station.u32_UltrasonicNoise	    =	pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
    		pst_me_amfm_app_inst->st_current_station.u32_Multipath		=	pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
    		pst_me_amfm_app_inst->st_current_station.u32_FrequencyOffset		= 	pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;

			st_AMFM_App_AFList_Info.u32_Qua_curr =  QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
			#ifdef CALIBRATION_TOOL
				 st_Calib_AMFM_App_AFList_Info.u32_Qua_curr = st_AMFM_App_AFList_Info.u32_Qua_curr ;
			#endif

		    AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
			
			if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)
			{
				pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
	
			}
			else
			{
				pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
//				pst_me_amfm_app_inst->u8_QualDegradedNotify_Flag=0;	
			}
			if(pst_me_amfm_app_inst->e_ENG_ModeSwitch == AMFM_APP_ENG_MODE_SWITCH_ON)
			{
				AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
			}
			else
			{
				if(((st_AMFM_App_AFList_Info.u32_Qua_old_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold))
				|| ((st_AMFM_App_AFList_Info.u32_Qua_old_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold) && (st_AMFM_App_AFList_Info.u32_Qua_avg > pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)))
				{
					//pst_me_amfm_app_inst->u8_QualDegradedNotify_Flag=1;	
					AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
				}
				else
				{
					/*nothing to do*/
				}
			}	
			if((st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)||(pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF)|| (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
			{

				if((pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF)|| (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
				{
					/* Api to command audio manager to change program audio level*/
					AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_TA_SWITCH_OFF);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcment cancelled by TA switch OFF");

				}	
				else
				{
					
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
					/* Api to command audio manager to change program audio level*/
					AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_SIGNAL_LOSS);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcment cancelled due to signal loss");

				}		
				
				/* Sending Response to Radio manager */
				AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS, pst_me_amfm_app_inst->st_current_station);
				
				/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
				
			}
			else
			{
				pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
			}
				
		}
		break;
			
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
		
			/* Function to read quality of currently tuned station */
			AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);	
			st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.u32_Qua_curr = st_AMFM_App_AFList_Info.u32_Qua_curr ;
			#endif
			
            AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
			
			if((st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold)||(pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF)|| (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
			{
				if((pst_me_amfm_app_inst->e_TA_Switch == AMFM_APP_TA_SWITCH_OFF)|| (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_OFF))
				{
					/* Api to command audio manager to change program audio level*/
					AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_TA_SWITCH_OFF);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcment cancelled by TA switch OFF");
				}	
				else
				{
					pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
					/* Api to command audio manager to change program audio level*/
					AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_SIGNAL_LOSS);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcment cancelled due to signal loss");
				}
				u8_Announcement_stopped=1;
				/* Sending Response to Radio manager */
				AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS, pst_me_amfm_app_inst->st_current_station);
				/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
			}
			else
			{
				pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_GOOD_QUALITY_SIGNAL;
			}
			if(st_AMFM_App_AFList_Info.u16_curr_PI==0)
			{	
				if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != 0) 
				{
					u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);
					u8_af_appendCheck = 1;
//					u8_UpdatePIinSTL  = 1;
				}
				else
				{
					/*PI not received nothing to do*/
				}
			}
			else
			{
				/*PI already updated*/
				if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi!=0)
				{
					if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != st_AMFM_App_AFList_Info.u16_curr_PI)
					{
						/* PI changed */
						
						/* PI change detected notifying to RM*/
						AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_DETECTED);
						
						if(u8_Announcement_stopped!=1)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : TN Announcment cancelled due to PI change");
							AMFM_App_Notify_Announcement_Status(AMFM_APP_ANNOUNCEMENT_END_SIGNAL_LOSS);
							/* Sending Response to Radio manager */
							AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);		
						}		
						if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)&&(st_AMFM_App_AFList_Info.u8_NumAFList !=0))
						{
							st_AMFM_App_AFList_Info.e_curr_status		= AMFM_APP_PI_CHANGE;
							#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
							#endif
							pst_me_amfm_app_inst->u16_curr_station_pi   = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;
							/*Function to clear AF frequencied quality parameters alone*/
							AMFM_APP_Clear_AF_Qual_Parameters(&st_AMFM_App_AFList_Info);
							u8_af_update_cycle=0;
							/* Transiting to amfm_app_inst_hsm_active_idle_af_strategy_tune_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_strategy_tune_state);
						}
						else 
						{
							if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)&&(pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
							   (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT))
							{
									/*DAB linked then stop DAB FM linking*/
									/*stop dab linking to do audio switch*/
									AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_PI_CHANGE);
									pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_STATION_TUNED;
									pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE   = AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
									/*send new PI to DAB*/
							}
							/* Updating new PI and AF lists */
							AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
							if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
							{
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =FALSE;
								/*Timer wont be started again,can close it*/
								if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
								{
									b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
									if(b_return_value == FALSE)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
									}
									else
									{
										st_AMFM_TimerId.u32_Status_Check_TimerId =0;
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
									}	
								}
								else
								{
									/*MISRAC*/
								}
							}
							u8_NVM_Write = AMFM_App_PI_Updation(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);
							//u8_af_appendCheck = 1;
//							u8_UpdatePIinSTL  = 1;
							pst_me_amfm_app_inst->b_CurStationPICpy =TRUE;	/*variable used to indicate first time copying  AFList from learn memory*/
							
							AMFM_App_Notify_AF_Status(AMFM_APP_PI_CHANGE_LINK_ESTABLISHED);
							/* Sending Response to Radio manager ,No best AF available for the PI changed station. Changed PI and PSN are updated in curr station */
							AMFM_App_Response_PlaySelectSt(REPLYSTATUS_SUCCESS,pst_me_amfm_app_inst->st_current_station);
							/* Transiting to amfm_app_inst_hsm_active_idle_listen_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
						}
					}
					else
					{
						/*PI not changed*/	
						u8_af_appendCheck = 1;
					}
				}
				else
				{
					/* PI not received nothing to do */
				}
			}
			if(u8_af_appendCheck==1)
			{
				if( (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList==0) && (st_AMFM_App_AFList_Info.u8_NumAFList == 0) &&(pst_me_amfm_app_inst->b_CurStationPICpy ==TRUE))
				{
					b_return_value = AMFM_App_AF_List_Append_From_learn_Memory(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
					if(b_return_value==TRUE)
					{
						u8_NVM_Write=1;
					}
					pst_me_amfm_app_inst->b_CurStationPICpy =FALSE;	/*variable reseted to indicate  AFList appended from learn memory*/
				}
				else
				{
				if((pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList !=0) && (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.b_AF_Checkbit == TRUE))
				{
					/*updating AF structure in both tuner status notify structure and current station info */
					/*Function to append AF lsit*/
					pst_me_amfm_app_inst->u32_ReqFrequency=((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ));
		
					b_return_value = AMFM_App_AF_List_frequency_Append(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_AFeqList,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumofAF_SamePgm,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumofAF_RgnlPgm,&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst);
					if(b_return_value == TRUE)
					{
						u8_NVM_Write=1;
					}
					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)/*to check NEG timer not started*/
					{
						/*timer not started*/
						b_return_value = AMFM_App_NEG_flag_status(&st_AMFM_App_AFList_Info);	
						if(b_return_value == TRUE)
						{
							/*NEG flag set start the timer*/
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;
								st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_AF_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
								else
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_AF_STATUS_CHECK_NOTIFYID);
								}
							}
						}
					}
				}
			}
			if(st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_PI_CHANGE)
			{
				/* Copying PSN and RT into */
				b_PSNorRT_Changeflag = AMFM_App_PSN_RT_Copy(&(pst_me_amfm_app_inst->st_current_station),&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);	

				/*Copying the Program Name */
				AMFM_App_PTY_Copy(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
			 	
				/* Copying TA TP Info */
				AMFM_App_Read_TA_TP_info(&(pst_me_amfm_app_inst->st_current_station), &pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				/* Copying CT Info */
				AMFM_App_ReadCTinfo(&pst_me_amfm_app_inst->st_RDS_CT_Info,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				if((pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI  != 0) && (pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON) &&
				(pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON)&& (pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status ==AMFM_APP_FM_STATION_TUNED))
				{
					/*Debug log for  notification to find SID*/
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP] DABTuner back to normal so sending %x PI to DAB  to proceed for FM-DAB",pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);

					/* Notifying PI value of currently tuned Station for FM-DAB LInking purpose */
					AMFM_App_Notify_Find_SID(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI);
					pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_PI_NOTIFICATION_SENT;
				}
				else
				{
					/*Nothing to do */
				}
				b_return_value = AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.u32_curr_freq, st_AMFM_App_AFList_Info.u16_curr_PI,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
				if(b_return_value==TRUE)
				{
					u8_NVM_Write=1;

				}
			}
			
			SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP */
								
			/* Updating PI of current station in FM STL */
			if(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI)
			{	
				b_retBool = AMFM_App_Remove_SamePIfromFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI,&st_fm_station_list);
				if(b_retBool == TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : FM STL is updated as same PI station is removed.");
					/* As FM STL is sorted, index value of current station in FM STL might have changed. */
					pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
				}
				st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI = pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI;

				if(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].u16_PI != 0)
				{
					/* Clearing old PSN as PI of tuned Station is changed.*/
					memset(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,0,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
				}
				/*Sending notification to radio manager to inform STL is generated in shared memory */
				AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);
			}

			if((b_PSNorRT_Changeflag & 0x01)  && (pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL == TRUE)) 
			{
				/* Clearingthe flag so that PSN of curr station in FM STL will not be updated further */
				pst_me_amfm_app_inst->b_CurrStationPSNupdateInSTL = FALSE;

				SYS_RADIO_MEMCPY(st_fm_station_list.ast_Stations[pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL].au8_psn,(const void *)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

				/* Sorting FM STL as newly seeked FM Station appended into it */
				AMFM_App_application_Remove_SpacefromStationList((st_fm_station_list.ast_Stations),st_fm_station_list.u8_NumberOfStationsInList);

				(void)AMFM_App_application_SortFMStationList(st_fm_station_list.ast_Stations,st_fm_station_list.u8_NumberOfStationsInList);
	
				/* As FM STL is sorted, index value of current station in FM STL might have changed. */
			 	pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  = AMFM_App_CheckFreqPresentFMSTL(pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency,&st_fm_station_list);
				
				/*Sending notification to radio manager to inform STL is generated in shared memory */
				AMFM_App_Notify_STLUpdated(FM_APP_RADIO_MNGR);	
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  FG : PSN is added in FM STL. So FM STL is sorted and notified to RM ");										
			}	
		
			/* UnLocking the mutex between RM and APP*/
			SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);		
			if((u8_NVM_Write==1)&&(b_NVM_ActiveUpdateSwitch==TRUE))
			{
				/*code to store LSM data into NVM */
				u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
				if(u8_NVM_ret == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  NVM write success during Tuner status notification ");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tuner station notification ");
				}
			}
			if(st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_PI_CHANGE)
			{
				AMFM_App_Notify_TunerStatus(pst_me_amfm_app_inst->st_current_station,pst_me_amfm_app_inst->e_SigQuality);
			}
		}		
		break;
			
			
		case AMFM_APP_EON_ANNOUNCEMENT_START_NOTIFIED:
		{
			/* while playing announcement in current station Announcement from EON is not supported*/	
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
#endif
#ifdef CALIBRATION_TOOL
/*===========================================================================*/
/*	Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveCalibrationStartTuneState	 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveCalibrationStartTuneState(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg								* pst_ret				= NULL;           		 	 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex       = 0;   						 /* variable to update the message slot index */
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus	= REPLYSTATUS_REQ_NOT_HANDLED;	/* enum variable to hold the  Play select station reply status*/
	
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveCalibrationStartTuneState");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_calibration_start_tune_state",sizeof("amfm_app_inst_hsm_active_calibration_start_tune_state"));		
		}
		break;
				
		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{
		
			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);

		
			s32_cal_BBFieldStrength   = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength;
			u32_cal_UltrasonicNoise  = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise;
			u32_cal_Multipath  = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath;
			u32_cal_curr = QUAL_CalcFmQual(s32_cal_BBFieldStrength , u32_cal_UltrasonicNoise, u32_cal_Multipath, NULL, NULL, 0);
			
			AMFM_App_Calib_Tune_Qual_Avg_Computation();		
		}
		break;
		
		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
            AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_QualParam),(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);
		
			s32_cal_BBFieldStrength	= pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
            u32_cal_UltrasonicNoise  = pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
            u32_cal_Multipath	= pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
		
			u32_cal_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
	
			AMFM_App_Calib_Tune_Qual_Avg_Computation();
			
			/*New strategy*/
			
			if(b_start_stop_switch == FALSE)
			{
				u32_cal_curr=0;
				u32_cal_avg=0;
				u32_cal_old=0;

				/* Requesting tuner control for tuning to station */
				AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),((st_AMFM_App_AFList_Info.u32_curr_freq)));

				/* Transiting to amfm_app_inst_hsm_active_calibration_stop_tune_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_calibration_stop_tune_state);
			}
		
				
		}
		break;
		
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{ 
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo),&u32_DataSlotIndex);

			s32_cal_BBFieldStrength   = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength;
			u32_cal_UltrasonicNoise  = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise;
			u32_cal_Multipath  = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath;
			u32_cal_curr = QUAL_CalcFmQual(s32_cal_BBFieldStrength , u32_cal_UltrasonicNoise, u32_cal_Multipath, NULL, NULL, 0);
			AMFM_App_Calib_Tune_Qual_Avg_Computation();
			
			if(b_start_stop_switch == FALSE)
			{
				u32_cal_curr=0;
				u32_cal_avg=0;
				u32_cal_old=0;
				/* Requesting tuner control for tuning to station */
				AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),((st_AMFM_App_AFList_Info.u32_curr_freq)));
				/* Transiting to amfm_app_inst_hsm_active_calibration_stop_tune_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_calibration_stop_tune_state);
			}
			
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
/*	Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveCalibrationStopTuneState	 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveCalibrationStopTuneState(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg								* pst_ret				= NULL;           		 	 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex       = 0;    				     /* variable to update the message slot index */
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus  = REPLYSTATUS_REQ_NOT_HANDLED;  /* enum variable to hold the  Play select station reply status*/
	Tu8 u8_AFListIndex;
	
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveCalibrationStopTuneState");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_calibration_stop_tune_state",sizeof("amfm_app_inst_hsm_active_calibration_stop_tune_state"));			
		}
		break;
		
		
		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
			/* Function to read quality of currently tuned station */
			AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);

			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.u32_Qua_curr =st_AMFM_App_AFList_Info.u32_Qua_curr ;
			#endif


			st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength , pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, AMFM_APP_CONSTANT_ZERO);

			AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);	
		}
		break;
		
		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
            AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_QualParam),(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);

			st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
			
			AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
		
			if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
			{
				/*New strategy*/
					
				if(u8_notify_count >= AMFM_APP_NOTIFY_COUNT)
				{
					u8_notify_count =0;
				
					AMFM_App_Flag_Reset(&st_AMFM_App_AFList_Info);
					
					u8_AFListIndex=st_AMFM_App_AFList_Info.u8_AF_index;

					for(;u8_AFListIndex < st_AMFM_App_AFList_Info.u8_NumAFList; u8_AFListIndex++)
					{
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].b_af_check = FALSE;
						
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].b_af_check = FALSE;
						#endif
					}
					
					st_AMFM_App_AFList_Info.e_curr_status		=AMFM_APP_AF_SWITCHING;
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
					#endif
					/* Transiting to amfm_app_inst_hsm_active_idle_af_tune_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_tune_state);
				}
				else
				{
					u8_notify_count++;
				}
			}		
		}
		break;
		
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{ 
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo),&u32_DataSlotIndex);

			/* Function to read quality of currently tuned station */
			AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);

			st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);	

			AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
		
			if(pst_me_amfm_app_inst->st_current_station.e_mode== AMFM_APP_MODE_FM)
			{
					
				if(u8_notify_count >= AMFM_APP_NOTIFY_COUNT)
				{
					u8_notify_count =0;
				
					AMFM_App_Flag_Reset(&st_AMFM_App_AFList_Info);
					u8_AFListIndex=st_AMFM_App_AFList_Info.u8_AF_index;
					for(;u8_AFListIndex < st_AMFM_App_AFList_Info.u8_NumAFList; u8_AFListIndex++)
					{
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].b_af_check = FALSE;
						#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].b_af_check = FALSE;
						#endif
					}
			
					st_AMFM_App_AFList_Info.e_curr_status		=AMFM_APP_AF_SWITCHING;
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
					#endif
					/* Transiting to amfm_app_inst_hsm_active_idle_af_tune_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_tune_state);
				}
				else
				{
					u8_notify_count++;
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
			pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}	
#endif

/*===========================================================================*/
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyHndlr		 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 						pst_ret 			= NULL;             /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32								u32_Temp_Freq		= 0;
	Tu32								u32_DataSlotIndex	= 0;			    /* variable to update the message slot index */
	//Te_AMFM_App_mode					e_Currmode			= pst_me_amfm_app_inst->e_requested_mode;
	Te_RADIO_ReplyStatus				e_CancelReplyStatus = REPLYSTATUS_FAILURE;
	
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			if(pst_me_amfm_app_inst->e_Processing_Status == 	AMFM_APP_FOREGROUND)
			{
				if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
				{
					if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)&&(pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
					   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
						(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
					{
							/*DAB linked then stop DAB FM linking*/
							/*stop dab linking to do audio switch*/
							AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_NEW_REQUEST);
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_STATION_TUNED;
							pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE   = AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
							/*send new PI to DAB*/
					}
					
				}
				else
				{
					/* No need to send notification if Active Band is AM */
				}
			}
			else
			{
				/* Nothing to do. */
			}	
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ), (const void *)"amfm_app_inst_hsm_active_busy_state",sizeof("amfm_app_inst_hsm_active_busy_state"));
		}
		break;
		
		case AMFM_APP_CURR_FREQUENCY_NOTIFYID:
		{

           AMFM_APP_ExtractParameterFromMessage(&u32_Temp_Freq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu32),&u32_DataSlotIndex);
					
			/* Notifying Radio Manager Application */
			AMFM_App_Notify_CurrFrequency(u32_Temp_Freq);
		}
		break;
		
		case AMFM_APP_CANCEL_REQID:
		{
			/* Requesting tuner ctrl to cancel ongoing operation either seek or scan */
			AMFM_Tuner_Ctrl_Request_Cancel();
		}
		break;
		
		case AMFM_APP_CANCEL_DONE_RESID:
		{
			/* Reading einfo  from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_CancelReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo), &u32_DataSlotIndex);

			
			if ((Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band) == AMFM_APP_MODE_FM)
			{
				pst_me_amfm_app_inst->st_current_station.e_mode = (Te_AMFM_App_mode) pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;
				pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq;
			}
			else if ((Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band) == AMFM_APP_MODE_AM_MW || (Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band) == AMFM_APP_MODE_AM_LW)
			{
				pst_me_amfm_app_inst->st_current_station.e_mode = (Te_AMFM_App_mode)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;
				pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq;
			}
			else
			{
				/* Nothing to do. */
			}
			if (e_CancelReplyStatus == REPLYSTATUS_SUCCESS)
			{				
				/* Transiting back to ative idle state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_state);	
			}
			else
			{
				/* Nothing to do. */
			}
			/* Sending cancelDone Response to Radio Mngr */
			AMFM_App_Response_CancelDone(e_CancelReplyStatus);
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

/*================================================================================*/
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusySelectStationHndlr	  */
/*================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusySelectStationHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret							=	NULL;										 /* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_Freq						= 0;	                     			  /* variable to hold the current station frequency */
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus	= REPLYSTATUS_REQ_NOT_HANDLED;			  /* enum variable to hold the  Play select station reply status*/
    Tu32									u32_DataSlotIndex				= 0;						              /* variable to update the message slot index */
    Tbool									b_verfreq						= FALSE;   
	Tu8 									u8_NVM_ret						= 0;
	Tu32									u32_NVM_Update_Param			=0;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:		
		{		
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusySelectStationHndlr");

			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_busy_select_station_state",sizeof("amfm_app_inst_hsm_active_busy_select_station_state"));
			
			/* update content of frequency given from radio manager*/
			u32_Freq = pst_me_amfm_app_inst->u32_StartFrequency; 
			pst_me_amfm_app_inst->u16_af_Tuned_PI=0;
			
			/*New strategy */
			memset(&st_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(st_AMFM_App_AFList_Info));
			#ifdef CALIBRATION_TOOL
				memset(&st_Calib_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(st_Calib_AMFM_App_AFList_Info));
			#endif
			if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM)
			{
					b_verfreq=AMFM_APP_VerifyFrequency(u32_Freq,&pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
					if(b_verfreq== TRUE)
					{
						pst_me_amfm_app_inst->u32_ReqFrequency = u32_Freq;
						/* Requesting tuner control for tuning to station */
						AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),pst_me_amfm_app_inst->u32_ReqFrequency);
					}
					else
					{
						/* Sending error report saying invalid frequency  */
						e_PlaySelectStationReplyStatus = REPLYSTATUS_FAILURE;
						AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_current_station);	

						/* Transiting back to ative idle listen state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_listen_state);
					}
				if((pst_me_amfm_app_inst->e_AF_Switch==AMFM_APP_AF_SWITCH_ON)&&(pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON) && 
				   ((pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_PI_NOTIFICATION_SENT)||
					(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status == AMFM_APP_FM_DAB_LINKED)))
				{
						/*during band change to AM from FM , FM-DAB should be stopped*/
						/*after band updation to AM only tune request will be received so handled here */				
						AMFM_App_Notify_Stop_FM_DAB_Follow_Up(AMFM_APP_DAB_LINKING_NEW_REQUEST);
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_FM_DAB_status = AMFM_APP_FM_STATION_TUNED;
						pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE   = AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_APP] FM-DAB STOPPED");
				}
			
					
			}

			else if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
			{

					b_verfreq = AMFM_APP_VerifyFrequency(u32_Freq,&pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
					if(b_verfreq == TRUE)
					{
						pst_me_amfm_app_inst->u32_ReqFrequency = u32_Freq;
						/* Requesting tuner control for tuning to station */
						AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_requested_mode),pst_me_amfm_app_inst->u32_ReqFrequency);
					}
					else
					{
						/* Sending error report saying invalid index  */
						e_PlaySelectStationReplyStatus = REPLYSTATUS_FAILURE;
						AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_current_station);	

						/* Transiting back to ative idle state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_state);
					}
			}	

		}
		break;

		case AMFM_APP_SELECT_STATION_DONE_RESID:
        {

			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);	
			
			AMFM_APP_Curr_Station_Related_Clearing(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
				
			pst_me_amfm_app_inst->st_current_station.e_mode = (Te_AMFM_App_mode)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;	
			
			pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE=AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;

		
			if (e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
			{				
				if( (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_MW )|| (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_LW)  )
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq ;  

					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				}
				else if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;
					
					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					
					/* Clearing LSM memory for AF list */
					memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
					
					/* Updating Frequency of newly tuned FM station into LSM memory */
					pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
					
					
					pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
					
					/*Af list is updated so current AFlist index should be made 0*/
					pst_me_amfm_app_inst->u8_aflistindex=0;
					if(b_NVM_ActiveUpdateSwitch == TRUE)
					{
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND),&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0) 
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success for play select station req ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  for play select station req ");
						}
					}
				}
				else
				{
						/* Send failure response as invalid parameters */	
				}	
				/* Transisting to active idle listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);	
				
			}
			else if(e_PlaySelectStationReplyStatus == REPLYSTATUS_NO_SIGNAL)
			{
				if( (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_MW )|| (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_LW)  )
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq ;  
				}
				else 
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;
					/* Comparing requested freqency for tuning and frequency present in LSM */
					if(pst_me_amfm_app_inst->u32_StartFrequency != pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq)
					{
						/* Clearing LSM memory for AF list */
						memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
					
						/* Updating Frequency of newly tuned FM station into LSM memory */
						pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = pst_me_amfm_app_inst->u32_StartFrequency;
					
					}
					else
					{
						/* No need to clear LSM becasue now Radio Mngr give AF tune request */
					}
				}
				/*Transit to idle_tune_failed_listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_tune_failed_listen_state);	
					
			}
			
			/* Sending Response to Radio manager */
			AMFM_App_Response_PlaySelectSt(e_PlaySelectStationReplyStatus,pst_me_amfm_app_inst->st_current_station);	
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
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusySeekHndlr		 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusySeekHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*							pst_ret			    = NULL;										/* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_RADIO_ReplyStatus				e_SeekReplyStatus	= REPLYSTATUS_REQ_NOT_HANDLED;				/* enum variable to hold the  seek reply status*/
	Tu32								u32_DataSlotIndex	= 0;					                    /* variable to update the message slot index */
	Te_RADIO_DirectionType				e_SeekDirection		= pst_me_amfm_app_inst->e_SeekDirection;
    Te_AMFM_App_mode					e_Currmode			= AMFM_APP_MODE_INVALID; 
	Tu32								u32_LowerFreq	    = 0;
	Tu32								u32_StartFreq	    = 0;
	Tu32								u32_UpperFreq   	= 0;
	Tu32								u32_Step_Size = 0;
	Tbool								b_verfreq			= FALSE;  
//	Ts8									s8_index			= -1;
	Tu8									u8_NVM_ret 			= 0;
	Tu32								u32_NVM_Update_Param= 0;
//	Tu8									u8_RetValue			= 0;

	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusySeekHndlr");

			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_busy_seek_state" ,sizeof("amfm_app_inst_hsm_active_busy_seek_state" )); 
		
			pst_me_amfm_app_inst ->u16_af_Tuned_PI=0;
			memset(&st_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(st_AMFM_App_AFList_Info));
			
			b_verfreq = AMFM_APP_VerifyFrequency(pst_me_amfm_app_inst->u32_StartFrequency,&pst_me_amfm_app_inst->e_requested_mode,
											 	 pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo);
											 
			if(b_verfreq == TRUE)
			{
				e_Currmode = pst_me_amfm_app_inst->e_requested_mode;			
            	
				if((e_Currmode == AMFM_APP_MODE_AM_MW) || ( e_Currmode == AMFM_APP_MODE_AM_LW))
				{
					u32_StartFreq	 = (pst_me_amfm_app_inst->u32_StartFrequency);
					if( pst_me_amfm_app_inst->e_MarketType == AMFM_APP_MARKET_WESTERN_EUROPE )
					{
						u32_LowerFreq = pst_me_amfm_app_inst->st_MarketInfo.st_AMbandLW_FreqInfo.u32_StartFreq;
						u32_UpperFreq = pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_EndFreq;

						if(e_Currmode == AMFM_APP_MODE_AM_MW)
						{
							u32_Step_Size  = pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StepSize;
						}
						else if(e_Currmode == AMFM_APP_MODE_AM_LW)
						{
							u32_Step_Size  = pst_me_amfm_app_inst->st_MarketInfo.st_AMbandLW_FreqInfo.u32_StepSize;
						}
					}
					else
					{
						u32_LowerFreq = pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StartFreq;
						u32_UpperFreq= pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_EndFreq;
						u32_Step_Size = pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StepSize;
					}
				}
				else if( e_Currmode == AMFM_APP_MODE_FM )
				{
					u32_StartFreq = (pst_me_amfm_app_inst->u32_StartFrequency) ;
					u32_LowerFreq = (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq );
					u32_UpperFreq = (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_EndFreq  );
					u32_Step_Size = (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StepSize);
				}
				else
				{	
					/* Mode is updated with invalid parameter in AMFM App */
					e_SeekReplyStatus = REPLYSTATUS_INVALID_PARAM;
				}
			}
			else
			{
				/* Invalid Start frequency is given by Radio Mngr  */
				e_SeekReplyStatus = REPLYSTATUS_INVALID_PARAM;
			}	
			
			if (e_SeekReplyStatus != REPLYSTATUS_INVALID_PARAM)
			{
				/* Requesting tuner Ctrl to perform seek operation */
				AMFM_Tuner_Ctrl_Request_SeekUpDown((Te_RADIO_DirectionType)e_SeekDirection, u32_LowerFreq, u32_StartFreq, u32_UpperFreq, u32_Step_Size, (Te_AMFM_Tuner_Ctrl_Band)e_Currmode);
			}
			else
			{
				/* Sending Response as invalid param to Radio manager */
				AMFM_App_Response_SeekUpDown(e_SeekReplyStatus,pst_me_amfm_app_inst->st_current_station);	
				
				/* Transisting to active idle listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);					
			}	
		}
		break;

		case AMFM_APP_SEEK_UP_DOWN_DONE_RESID:
		{

			AMFM_APP_ExtractParameterFromMessage(&e_SeekReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);	
				
			AMFM_APP_Curr_Station_Related_Clearing(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
			pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE=AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;
		
			if (e_SeekReplyStatus == REPLYSTATUS_SUCCESS)
			{
				pst_me_amfm_app_inst->st_current_station.e_mode = (Te_AMFM_App_mode)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;	
				
				/* Updating requested mode with band sent from tuner ctrl becase during seek band may change between LW & MW */
				pst_me_amfm_app_inst->e_requested_mode = (Te_AMFM_App_mode)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band;
					
		    	if( (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_MW )|| (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_LW)  )
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq =    pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq ;  

					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);					
				}
				else if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
				{
				 	pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 =  (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
					
					
					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					
					/* Clearing LSM memory */
					memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
					
					/* Updating Frequency of newly tuned FM station into LSM memory */
					pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
					/*code to store LSM data into NVM */
					if(b_NVM_ActiveUpdateSwitch == TRUE)
					{
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0) 
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during seek req  ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during seek req ");
						}
					}
					pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
		
			        pst_me_amfm_app_inst->u8_aflistindex=0;

					/* Disabling the flag. So AF strategy will be started after 10 seconds */
					pst_me_amfm_app_inst->b_StartAF_Flag = FALSE;

					/* Start timer to  initiate AF strategy after 10 seconds  */
					st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(AMFM_APP_TEN_SECONDS_DELAY, AMFM_APP_START_AF_STRATEGY_NOTIFYID, RADIO_AM_FM_APP);											
					
					if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_APP_START_AF_STRATEGY_NOTIFYID);
					}
					else
					{
				//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
					}	
				}
				else
				{
					// need to send error reply status to radio manager 
				}
				/* Transisting to active idle listen state .*/
				
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);	
			}
			else if (e_SeekReplyStatus == REPLYSTATUS_NO_SIGNAL)
			{
				pst_me_amfm_app_inst->st_current_station.e_mode = pst_me_amfm_app_inst->e_requested_mode;  
				
				if( (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_MW )|| (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_LW)  )
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq =   pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq ;  
				}
				else 
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 =  (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;	
		
				}	
				/*Transit to idle_tune_failed_listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_tune_failed_listen_state);
			}
            /* Sending Response to Radio manager */
			AMFM_App_Response_SeekUpDown(e_SeekReplyStatus,pst_me_amfm_app_inst->st_current_station);	
		
			
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
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyTuneUpDownHndlr	 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyTuneUpDownHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret			    			= NULL;		/* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_RADIO_DirectionType 					e_TuneUpDown_Direction			= pst_me_amfm_app_inst->st_TuneUpDownParam.e_TuneUpDown_Direction;
	Tu8										u32_No_of_Steps					= pst_me_amfm_app_inst->st_TuneUpDownParam.u32_No_of_Steps;							
	Tu32 									u32_CurrentFrequency			= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
//	Te_AMFM_App_mode 						e_CurrentMode					= pst_me_amfm_app_inst->st_current_station.e_mode;
	Te_AMFM_App_Market 						e_MarketType					= pst_me_amfm_app_inst->e_MarketType;
	Tu32									u32_tunefrequnecy				= 0;
	Tu32									u32_DataSlotIndex				= 0;					                    /* variable to update the message slot index */
	Te_RADIO_ReplyStatus					e_TuneUpDownReplyStatus			= REPLYSTATUS_REQ_NOT_HANDLED;  			/* enum variable to hold the  Play select station reply status*/
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus	= REPLYSTATUS_REQ_NOT_HANDLED;  			/* enum variable to hold the  Play select station reply status*/
	Tu8										u8_NVM_ret						=0;
	Tu32									u32_NVM_Update_Param			=0;

	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyTuneUpDownHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name) ,(const void *)"amfm_app_inst_hsm_active_busy_tune_up_down_state" ,sizeof("amfm_app_inst_hsm_active_busy_tune_up_down_state")); 
			
			pst_me_amfm_app_inst->e_requested_mode = pst_me_amfm_app_inst->st_current_station.e_mode;
			
			pst_me_amfm_app_inst ->u16_af_Tuned_PI=0;
			memset(&st_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(st_AMFM_App_AFList_Info));

			if (e_TuneUpDown_Direction == RADIO_FRMWK_DIRECTION_UP)
			{
				/* Updating next tune up frequency from currently tuned frequency in order to give tune request*/
				u32_tunefrequnecy = AMFM_APP_GetNextTuneUpFrequency(u32_CurrentFrequency,&pst_me_amfm_app_inst->e_requested_mode,u32_No_of_Steps,e_MarketType,&pst_me_amfm_app_inst->st_MarketInfo);
			}
			else if (e_TuneUpDown_Direction == RADIO_FRMWK_DIRECTION_DOWN)
			{
				/* Updating next tune down frequency from currently tuned frequency in order to give tune request*/
				u32_tunefrequnecy = AMFM_APP_GetNextTuneDownFrequency(u32_CurrentFrequency,&pst_me_amfm_app_inst->e_requested_mode,u32_No_of_Steps,e_MarketType,&pst_me_amfm_app_inst->st_MarketInfo);
			}
			else
			{
				/* Updating reply status  as invalid parameter*/
				e_TuneUpDownReplyStatus = REPLYSTATUS_INVALID_PARAM;
			}
			
					
			if ((e_TuneUpDown_Direction != RADIO_FRMWK_DIRECTION_INVALID) && (u32_tunefrequnecy != AMFM_APP_CONSTANT_ZERO))
			{
				if( (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_MW) || (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_LW) )
				{
					  pst_me_amfm_app_inst->u32_ReqFrequency = u32_tunefrequnecy;
				
					/* Requesting tuner ctrl to tune to a AM frequency */			
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->u32_ReqFrequency);	
				}
				else
				{
					pst_me_amfm_app_inst->u32_ReqFrequency = u32_tunefrequnecy ; 
					/* Requesting tuner ctrl to tune to a FM frequency */
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->u32_ReqFrequency);
				}		
			}
			else
			{
				/* Sending Negative Response to Radio manager */
				AMFM_App_Response_TuneUpDown(e_TuneUpDownReplyStatus,pst_me_amfm_app_inst->st_current_station);	
				
				/* Transisting to active idle listen state .continue with previous tuned station */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);	
			}
		}	
		break;
		
		case AMFM_APP_SELECT_STATION_DONE_RESID:
        {

			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
			
			AMFM_APP_Curr_Station_Related_Clearing(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
			
			pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo.e_DAB_PI_TYPE=AMFM_APP_DAB_PI_STATION_UNIDENTIFIED;

			pst_me_amfm_app_inst->st_current_station.e_mode = pst_me_amfm_app_inst->e_requested_mode;
			
			if (e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
			{			
				e_TuneUpDownReplyStatus = REPLYSTATUS_SUCCESS;
				
				if (((Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band) == AMFM_APP_MODE_AM_MW) || ((Te_AMFM_App_mode)(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.e_Band) == AMFM_APP_MODE_AM_LW))
				{
						pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq ;  					
									
						/* Function to read quality of currently tuned station */
						AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
				}
				else if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;
					
					/* Clearing LSM memory  */
					memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
					
					/* Updating Frequency of newly tuned FM station into LSM memory */
					  pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);
					if(b_NVM_ActiveUpdateSwitch == TRUE)
					{
						/*code to store LSM data into NVM */
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
						if(u8_NVM_ret == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during tune up/down req ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during tune up/down req ");
						}
					}	

								
					/* Function to read quality of currently tuned station */
					AMFM_App_ReadQuality(&pst_me_amfm_app_inst->st_current_station,&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo);
					
					pst_me_amfm_app_inst->st_current_station.u8_charset	= pst_me_amfm_app_inst->u8_charset;
					
					/*Af list is updated so current AFlist index should be made 0*/
					pst_me_amfm_app_inst->u8_aflistindex=0;	
				}
				else
				{
						/* Send failure response as invalid parameters */	
				}			
				/* Transisting to active idle listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_listen_state);						
			}
			else if (e_PlaySelectStationReplyStatus == REPLYSTATUS_NO_SIGNAL)
			{
				e_TuneUpDownReplyStatus = REPLYSTATUS_NO_SIGNAL;
								
				if( (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_MW )|| (pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_AM_LW)  )
				{
					  pst_me_amfm_app_inst->st_current_station.un_station.st_AMstation.u32_Freq = pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq ; 
				}
				else 
				{
					pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency	 = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq) ;
					
					/* Clearing LSM memory for AF list */
					memset((void *)&pst_me_amfm_app_inst->st_LSM_FM_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LSM_FM_Band));
					
					
					/* Updating Frequency of newly tuned FM station into LSM memory */
					pst_me_amfm_app_inst->st_LSM_FM_Info.u32_LSM_FM_Freq = (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq);				
				}
				/*Transit to idle_tune_failed_listen state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_tune_failed_listen_state);
			}
				
			/* Sending Response to Radio manager */
			AMFM_App_Response_TuneUpDown(e_TuneUpDownReplyStatus,pst_me_amfm_app_inst->st_current_station);
			
			if(pst_me_amfm_app_inst->e_requested_mode == AMFM_APP_MODE_FM)
			{
				if(pst_me_amfm_app_inst->st_current_station.st_Aflist.u8_NumAFList	!= 0)
				{
					/*sending AF list available notification*/
					AMFM_App_Notify_AF_Status(AMFM_APP_AF_LIST_AVAILABLE);
				}
				else
				{
					/*sending AF list not available notification*/
					AMFM_App_Notify_AF_Status(AMFM_APP_AF_LIST_EMPTY);
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
			pst_ret = pst_msg;
		}
		break;
	}

	return pst_ret;
}

/*==============================================================================*/
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyProcessSTLHndlr */
/*==============================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyProcessSTLHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg								   *pst_ret							=NULL;										/* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_RADIO_ReplyStatus						e_GetStationListReplyStatus		= REPLYSTATUS_REQ_NOT_HANDLED;				/* enum variable to hold the  getstationlist reply status*/
	Te_RADIO_SharedMemoryType					e_TunerNotifiedSharedMemoryType = SHAREDMEMORY_INVALID;						/* enum variable to hold the  sharedmemory type given from the tunercontrol  */
    Tu32										u32_DataSlotIndex               =0;					                        /* variable to update the message slot index */
	Te_RADIO_SharedMemoryType					e_SharedMemoryType				= SHAREDMEMORY_INVALID;						/* enum variable to hold the  sharedmemory type depending on the band */
	Tbool										b_return_value					= FALSE;
	Tu8											u8_NVM_ret = 0;
	Tu32 										u32_NVM_Update_Param=0;

	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyProcessSTLHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_busy_process_stl_state",sizeof("amfm_app_inst_hsm_active_busy_process_stl_state")) ;
			
			pst_me_amfm_app_inst ->u16_af_Tuned_PI=0;
			
			if(st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_AF_TUNE_SCAN_STARTED)
			{
				AMFM_APP_Curr_Station_Related_Clearing(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
			}
			/* Sending request to Tuner ctrl for updating stationlist */
			if(pst_me_amfm_app_inst -> e_stationlist_mode  == AMFM_APP_MODE_AM)
			{	
				/* For Western europe, Long wave need to be considered */
				if(pst_me_amfm_app_inst->e_MarketType == AMFM_APP_MARKET_WESTERN_EUROPE )
				{
					//AMFM_Tuner_Ctrl_Request_GetStationList((pst_me_amfm_app_inst->st_MarketInfo.st_AMbandLW_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_AMbandLW_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_EndFreq),pst_me_amfm_app_inst->st_MarketInfo.st_AMbandLW_FreqInfo.u32_StepSize,(Te_AMFM_Tuner_Ctrl_Band)AMFM_APP_MODE_AM_LW,(Te_AMFM_Scan_Type)(pst_me_amfm_app_inst->e_Scan_Type));
					/* should be Request for LW band after confirming with ST regarding AM LW and MW*/
					AMFM_Tuner_Ctrl_Request_GetStationList((pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StartFreq), (pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StartFreq), (pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_EndFreq), pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StepSize, (Te_AMFM_Tuner_Ctrl_Band)AMFM_APP_MODE_AM_MW, (Te_AMFM_Scan_Type)(pst_me_amfm_app_inst->e_Scan_Type));
				}
				else
				{
					AMFM_Tuner_Ctrl_Request_GetStationList((pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_EndFreq),pst_me_amfm_app_inst->st_MarketInfo.st_AMbandMW_FreqInfo.u32_StepSize,(Te_AMFM_Tuner_Ctrl_Band)AMFM_APP_MODE_AM_MW,(Te_AMFM_Scan_Type)(pst_me_amfm_app_inst->e_Scan_Type));
				}
			}
			else if(pst_me_amfm_app_inst -> e_stationlist_mode  == AMFM_APP_MODE_FM)
			{
					AMFM_Tuner_Ctrl_Request_GetStationList((pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq), (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq),(pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_EndFreq) , (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StepSize),(Te_AMFM_Tuner_Ctrl_Band)(pst_me_amfm_app_inst->e_stationlist_mode),(Te_AMFM_Scan_Type)(pst_me_amfm_app_inst->e_Scan_Type));

			}
		}
		break;

		case AMFM_APP_STA_NOT_AVAIL_STRA_STATUS_NOTIFYID:
		{
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_StationNotAvailStrategyStatus),&u32_DataSlotIndex);
			/* Strategy end notification will be received here*/
			st_AMFM_App_AFList_Info.e_curr_status = AMFM_APP_WAIT_TUNE_REQ;
			#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.e_curr_status= 	st_AMFM_App_AFList_Info.e_curr_status;
			#endif
				
		}break;
		
		case AMFM_APP_GET_STL_DONE_RESID:
		{
			/* Reading Reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_GetStationListReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			
			/* Sending response to Radio Manager Application provides reply status */
			AMFM_App_Response_GetStationList(e_GetStationListReplyStatus);			
		}
		break;
	
		case AMFM_APP_STL_UPDATED_NOTIFYID:
		{
			/* Reading Reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_TunerNotifiedSharedMemoryType, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_SharedMemoryType), &u32_DataSlotIndex);
			
			if(pst_me_amfm_app_inst -> e_stationlist_mode  == AMFM_APP_MODE_AM)
			{
				AMFM_App_application_GenerateAMStationList(&st_am_station_list);	
				
				e_SharedMemoryType = AM_APP_RADIO_MNGR;
								
				/*Sending notification to radio manager to inform STL is generated in shared memory */
				AMFM_App_Notify_STLUpdated(e_SharedMemoryType);
	
			}
			else if(pst_me_amfm_app_inst -> e_stationlist_mode  == AMFM_APP_MODE_FM)
			{
		
				b_return_value = AMFM_App_application_GenerateFMStationList(&st_fm_station_list,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0],pst_me_amfm_app_inst->e_MarketType);
				if((b_return_value == TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
				{
					/*code to store LSM data into NVM */
					u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) +(sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
					if(u8_NVM_ret == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]FG  NVM write success  during station list update");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]FG  NVM write fails during station list update ");
					}
				}
				/* Updating charset for stations present in the list */
				st_fm_station_list.u8_charset	=  pst_me_amfm_app_inst->u8_charset;
				
				e_SharedMemoryType = FM_APP_RADIO_MNGR;
					
				/*Sending notification to radio manager to inform STL is generated */
				AMFM_App_Notify_STLUpdated(e_SharedMemoryType);	
	
			}
			if(st_AMFM_App_AFList_Info.e_curr_status != AMFM_APP_AF_TUNE_SCAN_STARTED)			
			{
				/* Transisting to active_idle_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_state);
			}
			else
			{
				pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI = pst_me_amfm_app_inst->u16_curr_station_pi;
				/*Function to clear old AF frequencies alone*/
				AMFM_App_AFListClear(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info);
				pst_me_amfm_app_inst->u8_aflistindex 									= 0;
				pst_me_amfm_app_inst->u32_ReqFrequency									= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
				pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI = pst_me_amfm_app_inst->u16_curr_station_pi;
				st_AMFM_App_AFList_Info.u16_curr_PI										= pst_me_amfm_app_inst->u16_curr_station_pi	 ;
				st_AMFM_App_AFList_Info.u32_curr_freq 									= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency;
				u8_af_update_cycle														= 0;
				st_AMFM_App_AFList_Info.e_curr_status									= AMFM_APP_AF_TUNE_SCAN_COMPLETED;
				b_return_value = AMFM_App_AF_List_Append_From_learn_Memory(pst_me_amfm_app_inst,&st_AMFM_App_AFList_Info,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info = st_AMFM_App_AFList_Info;
				#endif
				if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch == TRUE))
				{
					/*code to store LSM data into NVM */
					u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) +(sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
					if(u8_NVM_ret == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during AFTune request,Af list appended from learn memory ");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during AFTune request ");
					}
				}
			
				/* can clear current station quality info*/
				/* Transiting to amfm_app_inst_hsm_active_idle_af_strategy_tune_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_idle_af_strategy_tune_state);
			}
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
/*			Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingHndlr	 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret	= NULL;								/* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_RADIO_ReplyStatus					e_SelectBandReplyStatus;					/* enum variable to hold the  startup reply status*/
	Tu32									u32_DataSlotIndex            =0;	        /* variable to update the message slot index */
	Te_AMFM_App_mode						e_ReqMode					 = AMFM_APP_MODE_INVALID;                    /* enum variable to hold the requested mode */
	Te_AMFM_App_FM_To_DAB_Switch			e_FM_to_DAB_Switch; 
	Te_RADIO_ReplyStatus					e_FM_to_DAB_SwitchReplyStatus;

	
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name ),(const void *)"amfm_app_inst_hsm_active_busy_linking_state" ,sizeof( "amfm_app_inst_hsm_active_busy_linking_state" ));
			
			/* Transisting to linking_searching_state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_state);
		}
		break;

		case AMFM_APP_NON_RADIO_MODE_NOTIFYID:
		case AMFM_APP_STOP_DAB2FM_LINKING_NOTIFYID:
		{
			/* DAB service station is changed.So No use in performing DAB to FM linking process*/
			
			/* Transisting to active sleep state to generate STL for FM and AM in background*/
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);
		}
		break;

		case AMFM_APP_FM_TO_DAB_SWITCH_REQID:
		{
			/* Reading FM2DAB Switch status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_FM_to_DAB_Switch,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_FM_To_DAB_Switch),&u32_DataSlotIndex);
			
			if(e_FM_to_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_OFF )
			{
				pst_me_amfm_app_inst->e_FM_DAB_Switch = e_FM_to_DAB_Switch;		/*updating instance hsms FM-DAB Switch parameter*/
				
				e_FM_to_DAB_SwitchReplyStatus = REPLYSTATUS_SUCCESS;

				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE; 
									
				/* Transisting to active_sleep_background_process_stl_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);	
			}
			else
			{
				e_FM_to_DAB_SwitchReplyStatus = REPLYSTATUS_FAILURE;
			}
			AMFM_App_Response_FM_to_DAB_Switch(e_FM_to_DAB_SwitchReplyStatus);
		}
		break;

		case AMFM_APP_FIND_BEST_PI_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  BG : Best PI received ");

			/* Clearing Hardlink buffer */
			memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_PIList));
			
			/* Reading PI list from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList,pst_msg->data,sizeof(Ts_AMFM_App_PIList),&u32_DataSlotIndex);

			/* Reading QualityMin from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin,pst_msg->data,sizeof(Tu8),&u32_DataSlotIndex);

			/* Reading QualityMax from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMax,pst_msg->data,sizeof(Tu8),&u32_DataSlotIndex);
		
			/* Reading Implicit SID from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid,pst_msg->data,sizeof(Tu32),&u32_DataSlotIndex);
			
			/* Reading Linking Type from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType,pst_msg->data,sizeof(Tu8),&u32_DataSlotIndex);
					
			/* Transisting to linking_searching_state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_state);					
		}
		break;
		
		case AMFM_APP_SELECT_BAND_REQID:
		{

			/* Reading mode from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_ReqMode,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Te_AMFM_App_mode),&u32_DataSlotIndex);
				
			pst_me_amfm_app_inst-> e_requested_mode = e_ReqMode ; 
			
			/* Request to Tuner ctrl for activating the mode  */
			AMFM_Tuner_Ctrl_Request_Activate((Te_AMFM_Tuner_Ctrl_Band)e_ReqMode);
		}
		break;

		case AMFM_APP_SELECT_BAND_DONE_RESID:
		{
			/* Reading Replystatus value from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_SelectBandReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			pst_me_amfm_app_inst->e_current_mode  = pst_me_amfm_app_inst-> e_requested_mode;

			pst_me_amfm_app_inst->b_NEG_TimerStartCheck			= FALSE;
				
			/* Sending response to Radio Manager Application in order to indicate select band is done */
			AMFM_App_Response_SelectBand(e_SelectBandReplyStatus);

			/* Transisting to active idle state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_idle_state);
		}
		break;

		case AMFM_APP_AF_TUNE_REQID:
		{
			/* Reading PI  value for AF tune from the message */
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_BG_AFtune_param.u16_AFtune_PI,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);

			/* Clearing DAB-FM Linking parameters */
			memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_LinkingParam));

			/* Clearing AF structure parameters */
			memset(&st_AMFM_App_AFList_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AFList_Info));
			
			if((pst_me_amfm_app_inst->e_AF_Switch == AMFM_APP_AF_SWITCH_ON) && (pst_me_amfm_app_inst->e_FM_DAB_Switch == AMFM_APP_FM_TO_DAB_SWITCH_ON))
			{
				/* Transisting to active_sleep_aftune_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_sleep_aftune_state);			
			}
			else
			{
				/* Transisting to active sleep state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_state);
			}
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
/*===================================================================================*/
/*		Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingHndlr		 */
/*===================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*		pst_ret			    = NULL;			/* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu8				u8_PIcount			= 0;	
	
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name) ,(const void *)"amfm_app_inst_hsm_active_busy_linking_searching_state" ,sizeof("amfm_app_inst_hsm_active_busy_linking_searching_state"));
			
			/* Clearing DAB-FM Linking Parameters other than PI list , Qmax & Qmin */
			AMFM_APP_Clear_LinkingParam(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam);

			pst_me_amfm_app_inst->b_NEG_TimerStartCheck			= FALSE;

			pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_IsBeginning = TRUE;

			if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_HARDLINK)
			{
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_HARDLINK;

				/* Transisting to searching_generate_HL_freq_list_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state);
			}
			else if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_IMPLICIT_LINKING)
			{
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_IMPLICIT_LINK;

				if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid != AMFM_APP_CONSTANT_ZERO)
				{
					/* Clearing DAB-FM Linking parameters */
					memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_PIList));

					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.au16_PIList[AMFM_APP_CONSTANT_ZERO] = (Tu16)pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid;
	
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = 1;

					/* Transisting to searching_generate_HL_freq_list_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state);
				}
				else
				{
					/* Notifying no implicit linking  to DAB Tuner Ctrl via Radio Manager Application  */
					AMFM_App_Notify_BestPI(0,0,0,AMFM_APP_NO_LINK_AVAILABLE,
											 pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
											 AMFM_APP_CHARSET_INVALID);
					
					/* Transisting to sleep_background_process_stl_state as implicit sid is zero */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
				}
			}
			else
			{
				/* Copying temporarily PI count */
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount;

				if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid != AMFM_APP_CONSTANT_ZERO)
				{
					u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount;
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.au16_PIList[u8_PIcount] = (Tu16)pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid;
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount++;
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_NO_LINK_AVAILABLE;
				} 
				else
				{
					/* Only Hardlink.No Implicit Linking */
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_HARDLINK;
				}
				/* Transisting to searching_generate_HL_freq_list_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state);
			}
						
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


/*===============================================================================================*/
/*		Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingGenerateHLFreqListHndlr	 */
/*===============================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingGenerateHLFreqListHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*							pst_ret			= NULL;		/* Pointer to be updated to the  msg pst_msg  at default case*/

	Tu8*	   			    			pu8_CountforQuality	 = &pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_CountforQuality;
	Tu16							u16_STL_count	= 0;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
		
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingGenerateHLFreqListHndlr");

			/* Updating the current state name used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name) ,(const void *)"amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state" ,sizeof("amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state"));
			
			if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_IsBeginning == TRUE)
			{	
				/* Resetting flag to zero so list wont be generated for next iteration */
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_IsBeginning = FALSE;
				
				*pu8_CountforQuality = 1;

				u16_STL_count = (Tu8)( (pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_EndFreq -pst_me_amfm_app_inst->st_MarketInfo.st_FMband_FreqInfo.u32_StartFreq )/(Tu32)100) ;
					
				/* Generating Hardlink Frequency List */
				AMFM_App_GenerateHardLinkFreq(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,u16_STL_count,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
			}
			else
			{
				(*pu8_CountforQuality)++;
			}

			if(*pu8_CountforQuality <= 10 )
			{
				if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_IsHLfreqAvailable == TRUE)
																 
				{
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_FreqIndex = AMFM_APP_CONSTANT_ZERO;

					/* Transisting to searching_read_quality_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_read_quality_state);			
				}
				else
				{
					if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_IMPLICIT_LINKING)
					{
						/* Notifying no implicit linking  to DAB Tuner Ctrl via Radio Manager Application  */
						AMFM_App_Notify_BestPI(0,0,0,AMFM_APP_NO_LINK_AVAILABLE,
												 pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
												 AMFM_APP_CHARSET_INVALID);
					}
					else 
					{
						if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
						}
						
						/* This flag is enabled because after Updating FM STL needs to do transition to Linking state */
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE;
					}
					
					/* Transisting to active_sleep_background_process_stl_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
				}			
			}
			else
			{
				/* Sorting HL Freq List in descending order based on Quality(FS)*/
				AMFM_App_Sort_HardLink_Freq_List(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam);
				
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.b_IsBeginning = TRUE;
					
				/* Transisting to searching_bg_tune_state*/
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_bg_tune_state);
			}				
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

/*==========================================================================================*/
/*		Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingReadQualityHndlr	*/
/*==========================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingReadQualityHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret							= NULL;								/* Pointer to be updated to the  msg pst_msg  at default case*/
    Tu32									u32_DataSlotIndex				= 0;		                   	    /* variable to update the message slot index */
	Te_RADIO_ReplyStatus					e_AMFM_APP_AF_UpdateReplystatus = REPLYSTATUS_REQ_NOT_HANDLED; 		/* enum variable to hold the  AF update reply status*/

	Tbool									b_RetValue;
	Tu8*   				    				pu8_FreqIndex	 = &pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_FreqIndex;
	Tu8*	   			    				pu8_CountforQuality	 = &pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_CountforQuality;
	Tu32*								    pu32_TempQuality ;	
	Tu32									u32_AFupdateFreq = 0;

	pu32_TempQuality = (Tu32*)(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[*pu8_FreqIndex].s32_BBFieldStrength);
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingReadQualityHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name) ,(const void *)"amfm_app_inst_hsm_active_busy_linking_searching_read_quality_state",sizeof("amfm_app_inst_hsm_active_busy_linking_searching_read_quality_state")) ;
						
			/* Finding next valid frequency and then give update request */
			b_RetValue = AMFM_App_FindNextValidFreq(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List,pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_HL_Freq_Count,pu8_FreqIndex);

			if(b_RetValue == TRUE)
			{
				/* AF update request in order to get Quality */
				AMFM_Tuner_Ctrl_Request_AF_Update(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[*pu8_FreqIndex].u32_Freq);	

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  BG : AF update req given %d",pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[*pu8_FreqIndex].u32_Freq);
			}
			else
			{
				/* Transisting to searching_generate_HL_freq_list_state in order to get quality for next iteration */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state);
			}		
		}
		break;	

		case  AMFM_AF_UPDATE_TIMERID:
		{
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_AMFM_APP_TimerId = 0;
			(*pu8_FreqIndex)++;
			
			if(*pu8_FreqIndex < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_HL_Freq_Count)
			{
				b_RetValue = AMFM_App_FindNextValidFreq(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List,pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_HL_Freq_Count,pu8_FreqIndex);

				if(b_RetValue == TRUE)
				{
					/* AF update request in order to get Quality */
					AMFM_Tuner_Ctrl_Request_AF_Update(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[*pu8_FreqIndex].u32_Freq);	
				}
				else
				{
					/* Transisting to searching_generate_HL_freq_list_state in order to get quality for next iteration */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state);
				}
			}
			else
			{
				/* Transisting to searching_generate_HL_freq_list_state in order to get quality for next iteration */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_generate_HL_freqlist_state);
			}
		}
		break;
	
		case AMFM_APP_AFFREQ_UPDATE_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_UpdateReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u32_AFupdateFreq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_QualParam,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);

			if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[*pu8_FreqIndex].u32_Freq == u32_AFupdateFreq)
			{
			if (e_AMFM_APP_AF_UpdateReplystatus == REPLYSTATUS_SUCCESS)
				{ 	
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_APP]  BG : AF update Response success %d",pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[*pu8_FreqIndex].u32_Freq);
					
				pst_me_amfm_app_inst->st_QualParam.u32_interpolation = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset);

				if(*pu8_CountforQuality != 1)
      				{
					*pu32_TempQuality = ((*pu32_TempQuality + pst_me_amfm_app_inst->st_QualParam.u32_interpolation) / 2) ;
      				}
				else
				{
					*pu32_TempQuality = pst_me_amfm_app_inst->st_QualParam.u32_interpolation;
				}
			}
			else
			{
				/* Nothing to do. Just for MISRA C */		
			}
							
			/*start timer to  new update request */
			st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(10, AMFM_AF_UPDATE_TIMERID, RADIO_AM_FM_APP);			
			if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_AF_UPDATE_TIMERID);
			}
			else
			{
		//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_AF_UPDATE_TIMERID);
				}
			}	
			else
			{
				/* AF update response is for other frequency.So dropping this message and waiting for correct response */
			}
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


/*=======================================================================================*/
/*		Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingBgTuneHndlr	 	 */
/*=======================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingBgTuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*		pst_ret			    = NULL;						/* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu8				u8_TuneIndex	;	
	

	
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingBgTuneHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name),(const void *)"amfm_app_inst_hsm_active_busy_linking_searching_bg_tune_state",sizeof("amfm_app_inst_hsm_active_busy_linking_searching_bg_tune_state"));
					
			if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.b_IsBeginning == TRUE)
			{
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.b_IsBeginning = FALSE;

				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_TuneIndex = 0;				
			}
			
			u8_TuneIndex = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_TuneIndex;
			
			
			if(u8_TuneIndex < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_HL_Freq_Count)
			{
				if((pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[u8_TuneIndex].u32_Freq != AMFM_APP_CONSTANT_ZERO) 
					&& (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[u8_TuneIndex].s32_BBFieldStrength > (Ts32)pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMax ) )
				{
					AMFM_Tuner_Ctrl_Request_Tune((Te_AMFM_Tuner_Ctrl_Band)AMFM_APP_MODE_FM,pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.ast_HL_Freq_Quality_List[u8_TuneIndex].u32_Freq);

					/* Transisting to searching_find_best_pi_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_find_best_pi_state);	
				}
				else
				{
					if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_IMPLICIT_LINKING)
					{
						/* Notifying no implicit linking  to DAB Tuner Ctrl via Radio Manager Application  */
						AMFM_App_Notify_BestPI(0,0,0,AMFM_APP_NO_LINK_AVAILABLE,
												 pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
												 AMFM_APP_CHARSET_INVALID);
					}
					else
					{
						if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
						}
						
						/* This flag is enabled because after Updating FM STL needs to do transition to Linking state */
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE;
					}				

					/* Transisting to active_sleep_background_process_stl_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
				}
				
			}
			else
			{
				if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_IMPLICIT_LINKING)
				{
					/* Notifying no implicit linking  to DAB Tuner Ctrl via Radio Manager Application  */
					AMFM_App_Notify_BestPI(0,0,0,AMFM_APP_NO_LINK_AVAILABLE,
											 pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
											 AMFM_APP_CHARSET_INVALID);
				}
				else
				{
					if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
					{
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
					}
					
					/* This flag is enabled because after updating FM STL needs to do transiton to Linking state */
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE;
				}	
				
				/* Transisting to active_sleep_background_process_stl_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
			}	
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

/*=======================================================================================*/
/*		Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingFindBestPIHndlr	 */
/*=======================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingFindBestPIHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*								pst_ret			    			= NULL;			/* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32									u32_DataSlotIndex				= 0;	        /* variable to update the message slot index */
	Te_RADIO_ReplyStatus					e_PlaySelectStationReplyStatus  = REPLYSTATUS_REQ_NOT_HANDLED;  /* enum variable to hold the  Play select station reply status*/
//	Tu8										u8_TuneIndex					= pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_TuneIndex;
	Tbool									b_DoBGScanFlag					= FALSE ;	
	Tbool									b_return_value					= FALSE;
	Tbool									b_RetValue	= FALSE;
	Tu8 									u8_NVM_ret=0;
	Tu32 									u32_NVM_Update_Param=0;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingSearchingFindBestPIHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name),(const void *)"amfm_app_inst_hsm_active_busy_linking_searching_find_best_pi_state",sizeof("amfm_app_inst_hsm_active_busy_linking_searching_find_best_pi_state"));
		}
		break;
		
		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{
			/* Reading reply status */
			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			/* Reading details of currently tuned station info*/
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);	
			
			if (e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
			{
				pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_quality_interpolation = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);
				
				b_RetValue	=	AMFM_App_ValidatePIcode(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi);

				if(b_RetValue == TRUE)
				{
					/* Updating details of Best PI into st_Best_PI_Info structure */
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency	= ( (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq));
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode		=  pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi;
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality 		=  pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_quality_interpolation;

					if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType == AMFM_APP_NO_LINK_AVAILABLE)
					{
						if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi  == (Tu16)pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid) 
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_IMPLICIT_LINK;
						}
						else
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_HARDLINK;
						}
					}
					else
					{
						/* Noting to do. Already e_BestPIType is updated */
					}
					
					/* Notifying Best PI and its Quality to DAB Tuner Ctrl via Radio Manager Application */
					AMFM_App_Notify_BestPI(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi,
											 pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_quality_interpolation, 
										   	 pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency, 
										   	 pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType,
										   	 pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
										   	 pst_me_amfm_app_inst->u8_DABFM_LinkingCharset);

			
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  BG : Best PI notified ");


					/* Requesting tuner ctrl to keep on sending Quality of Best PI station */
					AMFM_Tuner_Ctrl_Request_Bg_Quality();
					
					/* Transisting to monitoring state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_monitoring_state);
				}
				else
				{
					if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi != 0)
					{	
						/* Tune request to next HL freq */
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Linking_trace.u8_TuneIndex++;
				
						/* Transisting to searching_bg_tune_state*/
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_searching_bg_tune_state);	
					}
					else
					{
						b_DoBGScanFlag = TRUE; 
					}			
				}	
				b_return_value =  AMFM_App_Learn_Memory_updation(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);

				if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch == TRUE))
				{
					/*code to store LSM data into NVM */
					u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) +(sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
					
					if(u8_NVM_ret == 0)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during DAB-FM Linking tune Res  ");
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during DAB-FM Linking tune Res  ");
					}
				}			
			}
			else
			{
				b_DoBGScanFlag = TRUE; 
			}
			
			if(b_DoBGScanFlag == TRUE)
			{
				if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
				{
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
				}

				/* This flag is enabled because after updating FM STL needs to do transiton to Linking state */
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE; 
										
				/* Transisting to active_sleep_background_process_stl_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
			}	
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

/*===================================================================================*/
/*		Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringHndlr		 */
/*===================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*		pst_ret			    = NULL;			/* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32			u32_DataSlotIndex	= 0;	        /* variable to update the message slot index */
	Ts8				s8_RetVal;
	Tbool			b_return_value;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name),"amfm_app_inst_hsm_active_busy_linking_monitoring_state",sizeof("amfm_app_inst_hsm_active_busy_linking_monitoring_state")) ;
					
			/*New strategy */
			memset(&st_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(st_AMFM_App_AFList_Info));	

			#ifdef CALIBRATION_TOOL	
				/*New strategy */
				memset(&st_Calib_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AFList_Info));
			#endif
			
			/* Adding HL frequency into AF list structure st_AMFM_App_AFList_Info */
			AMFM_App_Append_HL_Freq_into_List(&(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam),&st_AMFM_App_AFList_Info);
		}
		break;
		
		case AMFM_APP_BLENDING_STATUS_REQID:
		{
			AMFM_APP_ExtractParameterFromMessage((void *)&pst_me_amfm_app_inst->e_DAB2FM_Linking_status, pst_msg->data, sizeof(Te_RADIO_DABFM_LinkingStatus), &u32_DataSlotIndex);
		}
		break;

		case AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID:
		{
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_Status_Check_TimerId = 0;
			/*	Incrementing the counts for NEG flag*/
			AMFM_App_AF_DAB2FM_Increase_NEGStatusCount(st_AMFM_App_AFList_Info.u8_NumAFList,st_AMFM_App_AFList_Info.ast_current_AFStation_Info);
			
			/* Reset the AF's whose count reached the NEG timeout*/
			AMFM_App_AF_DAB2FM_Reset_NEG_Status(st_AMFM_App_AFList_Info.u8_NumAFList,st_AMFM_App_AFList_Info.ast_current_AFStation_Info);
			
			/*Checking any AF is having status as  AMFM_APP_PI_STATUS_NEG*/
			b_return_value = AMFM_App_DAB2FM_Check_NEG_flag_AFList(st_AMFM_App_AFList_Info.u8_NumAFList,st_AMFM_App_AFList_Info.ast_current_AFStation_Info);
							
			if(b_return_value == TRUE)
			{
				/* u8_NEG_TimerStartCheck flag is set as timer is stated */
				pst_me_amfm_app_inst->b_NEG_TimerStartCheck =	TRUE;

				/*Starting the timer as NEG flag is present */
				st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
				if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
				}
				else
				{
			//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
				}
			}
			else
			{
				pst_me_amfm_app_inst->b_NEG_TimerStartCheck =	FALSE;

				/*Stopping the timer as there is no AF with AMFM_APP_PI_STATUS_NEG status */
				if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
				{
					b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
					if(b_return_value == FALSE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
					}
					else
					{
						st_AMFM_TimerId.u32_Status_Check_TimerId =0;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] BG Time 9 successfully stopped  message will not be posted   ");
					}
				}
				else
				{
					/*MISRAC*/
				}
			}
		}
		break;
	
		case AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
	       AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_QualParam),(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);
 
			st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
	
			AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

			/*New strategy*/
			st_AMFM_App_AFList_Info.u32_curr_freq = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency;	

			st_AMFM_App_AFList_Info.u16_curr_PI 	= pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode ;
			
			#ifdef CALIBRATION_TOOL
				
				st_Calib_AMFM_App_AFList_Info.u32_curr_freq = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency;	

				st_Calib_AMFM_App_AFList_Info.u16_curr_PI 	= pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode ;
			#endif 
			
			
			if( (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin)  && (st_AMFM_App_AFList_Info.u8_NumAFList  == 0 ))
			{
				AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);
				
				if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
				{
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
				}
					
				/* This flag is enabled because after updating FM STL needs to do transiton to Linking state */
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE;
							
				/* Transisting to active_sleep_background_process_stl_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
			}
			else if((st_AMFM_App_AFList_Info.u32_Qua_avg  != (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality)) )
			{
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality = st_AMFM_App_AFList_Info.u32_Qua_avg;
				
				AMFM_App_Notify_PIQuality(	pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality,
										  	pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
										  	AMFM_APP_SAME_PSN,
										  	pst_me_amfm_app_inst->u8_DABFM_LinkingCharset);
			}
			
			if(( st_AMFM_App_AFList_Info.u8_NumAFList  != 0)  && (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode != 0))
			{
				pst_me_amfm_app_inst->u8_aflistindex = 0;
        	    
				pst_me_amfm_app_inst->u16_curr_station_pi = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode;
             	
				pst_me_amfm_app_inst->u32_ReqFrequency=((pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency));
					
				st_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
		            
				//pst_me_amfm_app_inst->u32_AF_Next_Freq_Update_Delay = AMFM_APP_AF_NEXT_FREQ_UPDATE_DELAY;
					
				/*New strategy*/
				st_AMFM_App_AFList_Info.u32_curr_freq =(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency);
				st_AMFM_App_AFList_Info.u8_AF_index  = 0;
				st_AMFM_App_AFList_Info.u8_Best_AF_station_Index= 0 ;
				
				#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.b_af_check_flag			= FALSE;	
					st_Calib_AMFM_App_AFList_Info.u32_curr_freq 				=(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency);
					st_Calib_AMFM_App_AFList_Info.u8_AF_index  				= 0;
					st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index	= 0 ;
				#endif 
			
			   	 /* Transiting to linking_monitoring_af_tune_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_busy_linking_monitoring_af_tune_state);
		
			}
			else
			{
				/*nothing to do */
			}
		}
		break;

		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			memset(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo));
			
			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
			
			if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi == pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode)
			{
				st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);
				
				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				
				/*New strategy*/
				st_AMFM_App_AFList_Info.u32_curr_freq =(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency);
				
				#ifdef CALIBRATION_TOOL
					st_AMFM_App_AFList_Info.u32_curr_freq =(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency);
				#endif 

				if((pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList !=0) && (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.b_AF_Checkbit == TRUE))
				{				
						pst_me_amfm_app_inst->u32_ReqFrequency=((pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency));
	            
						/* Updating AF list as well HL freq into st_AMFM_App_AFList_Info structure */	
						AMFM_App_Append_AF_Into_List(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_AFeqList,&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,&st_AMFM_App_AFList_Info,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);

						/*Checking NEG timer is started or not */
						if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)
						{
							/*Checking any AF is having status as  AMFM_APP_PI_STATUS_NEG*/
							b_return_value = AMFM_App_DAB2FM_Check_NEG_flag_AFList(st_AMFM_App_AFList_Info.u8_NumAFList,st_AMFM_App_AFList_Info.ast_current_AFStation_Info);
							
							if(b_return_value == TRUE)
							{
								/* u8_NEG_TimerStartCheck flag is set as timer is stated */
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;

								/*Starting the timer as NEG flag is present */
								
								st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
								}
								else
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
								}
							}
						}			
				}

				
				s8_RetVal=  strncmp((char *)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,(char *)pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME) ;

				if( (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin)  && (st_AMFM_App_AFList_Info.u8_NumAFList  == 0 ))
				{
					AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);

					if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
					{
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
					}

					/* This flag is enabled because after updating FM STL needs to do transiton to Linking state */
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE; 
								
					/* Transisting to active_sleep_background_process_stl_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
				}
				else if((st_AMFM_App_AFList_Info.u32_Qua_avg  != (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality)) || (s8_RetVal != 0) )
				{
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality = st_AMFM_App_AFList_Info.u32_Qua_avg;
					
					SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn),
										(const void *)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,
										MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

				
					AMFM_App_Notify_PIQuality(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,
												(s8_RetVal == 0) ? AMFM_APP_SAME_PSN : AMFM_APP_NEW_PSN,
												pst_me_amfm_app_inst->u8_DABFM_LinkingCharset);
				}


			
				if(( st_AMFM_App_AFList_Info.u8_NumAFList  != 0)  && (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode != 0) /* && (u32_AMFM_quality < u8_AMFM_Normal_Threshold) */)
				{
					pst_me_amfm_app_inst->u8_aflistindex = 0;
	                   		pst_me_amfm_app_inst->u16_curr_station_pi = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode;
	             			pst_me_amfm_app_inst->u32_ReqFrequency=((pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency));
	  		            
			
					/* New strategy */
					st_AMFM_App_AFList_Info.u32_curr_freq =pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency;
					st_AMFM_App_AFList_Info.u8_AF_index  = 0;
					st_AMFM_App_AFList_Info.u8_Best_AF_station_Index= 0 ;
					st_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
					
					#ifdef CALIBRATION_TOOL
					st_Calib_AMFM_App_AFList_Info.b_af_check_flag				= FALSE;
						st_Calib_AMFM_App_AFList_Info.u32_curr_freq 			=(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency);
						st_Calib_AMFM_App_AFList_Info.u8_AF_index  				= 0;
						st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index	= 0 ;
					#endif 
					
					/* Transiting to linking_monitoring_af_tune_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst,&amfm_app_inst_hsm_active_busy_linking_monitoring_af_tune_state);
					
				}
				else
				{
					/*nothing to do */
				}
			}	
			else
			{
				AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);
		
				if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
				{
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
				}
				
				/* This flag is enabled because after updating FM STL needs to do transiton to Linking state */
				pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE;
				
				/* Transisting to active_sleep_background_process_stl_state */
				HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
				
			}
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

/*=======================================================================================*/
/*		Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringAFtuneHndlr	 */
/*=======================================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringAFtuneHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*							pst_ret			                    = NULL;						/* Pointer to be updated to the  msg pst_msg  at default case*/
	Tu32								u32_DataSlotIndex	                = 0;	                    /* variable to update the message slot index */
	Tu8									u8_AFListIndex				        = 0;
	Tu8									u8_Calib_AF_index					= 0;
	Tu8									u8_Best_AF_station_Index		    = 0;
	Tu8									u8_Calib_best_AF_index				= 0;
   	Tu16                        	       u16_afpi;
	Tbool 								b_return_value						= FALSE;
	Te_RADIO_ReplyStatus				e_PlaySelectStationReplyStatus		= REPLYSTATUS_REQ_NOT_HANDLED;  /* enum variable to hold the  Play select station reply status*/
	Te_RADIO_ReplyStatus				e_AMFM_APP_AF_CheckReplystatus;
	Te_RADIO_ReplyStatus				e_AMFM_APP_AF_UpdateReplystatus;
//	Tu16								u16_PIcompare;	
	Ts8									s8_RetVal;
	Tbool								b_RetValue	                        = FALSE;
	Tu32 								u32_NVM_Update_Param=0;
	Tu8 								u8_NVM_ret=0;
	Te_AMFM_APP_BestPI_Type				e_temp_BestPIType;
	Tu32								u32_AFupdateFreq = 0;
	
	pst_me_amfm_app_inst->u32_AMFM_quality	=	AMFM_APP_CONSTANT_ZERO;
	
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveBusyLinkingMonitoringAFtuneHndlr");

			/* Updating the current state name used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name),(const void *)"amfm_app_inst_hsm_active_busy_linking_monitoring_af_tune_state",sizeof("amfm_app_inst_hsm_active_busy_linking_monitoring_af_tune_state"));
			
			u8_AFListIndex=st_AMFM_App_AFList_Info.u8_AF_index;
			
			AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq);

			/* line to be added*/
			pst_me_amfm_app_inst->u8_Curr_qua_check_count =0 ;
		}
		break;

		case AMFM_AF_UPDATE_TIMERID:
		{
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_AMFM_APP_TimerId = 0;
			/*Timer started after delay milli SEC*/
			b_return_value = AMFM_APP_New_AF_Update(&st_AMFM_App_AFList_Info);
			
			if(b_return_value == TRUE )
			{
				/*All stations from AF list are processed */
				AMFM_App_Sort_AF_List(&st_AMFM_App_AFList_Info);

				if( (st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin)  && 
					(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[0].u32_avg_qual < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin) )
				{
					AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);

					if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
					{
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
					}
						
					/* This flag is enabled because after updating FM STL needs to do transition to Linking state */
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE; 
				
					/* Transisting to active_sleep_background_process_stl_state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
				}
				else
				{
				b_return_value	= AMFM_App_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
				
				if(b_return_value == FALSE)
					{
						/*Best AF available check should be given*/
						AMFM_APP_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
					}
					else
					{
					if( pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.b_PIchangedFlag  == TRUE)
						{
							AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);
	
							if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
							{
								pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
							}
						
							/* This flag is enabled because after updating FM STL needs to do transition to Linking state */
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE;
				
							/* Transisting to active_sleep_background_process_stl_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
						}
						else
						{
							pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
					
							/*best AF Not availble wait for two seconds and repeat the operation*/
							st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
							if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
							else
							{
					//			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
						}
					}
				}			
			}
			else
			{
				/*AF update request is successfully processed*/
			}
			 			
		}
		break;

		case AMFM_CURR_STATION_QUALITY_CHECK_TIMERID:
		{	
			/*Reset the TimerId, as the message is posted after the timer expiry*/
			st_AMFM_TimerId.u32_AMFM_APP_TimerId = 0;
			if(pst_me_amfm_app_inst->u8_Curr_qua_check_count !=0 )
			{
				b_return_value = AMFM_App_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
				
				if (b_return_value == FALSE)
				{
					/*Best AF available check should be given*/
					AMFM_APP_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
				}
				else
				{
					if( pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.b_PIchangedFlag  == TRUE)
					{
						AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);

						if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
						}
						
						/* This flag is enabled because after updating FM STL needs to do transiton to Linking state */
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE;
				
						/* Transisting to active_sleep_background_process_stl_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
					}
					else
					{
						if(pst_me_amfm_app_inst->u8_Curr_qua_check_count >= 4)
						{
							pst_me_amfm_app_inst->u8_Curr_qua_check_count=0;
						
							AMFM_App_AFTune_Restart(&st_AMFM_App_AFList_Info);						
						}
						else
						{
							/*best AF Not availble wait for two seconds and repeat the operation*/
							pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
							st_AMFM_TimerId.u32_AMFM_APP_TimerId = SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
							if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
							else
							{
						//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
							}
						}
					}	
				}
			}
			else
			{
				/*Quality degraded below threshold*/
				AMFM_App_AFTune_Restart(&st_AMFM_App_AFList_Info);		
			}		
		}
		break;


		case AMFM_APP_AFFREQ_UPDATE_DONE_RESID:
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_UpdateReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&u32_AFupdateFreq,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
				
			AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_QualParam),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_Interpolation_info),&u32_DataSlotIndex);
		
			st_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
			
			u8_AFListIndex			=	st_AMFM_App_AFList_Info.u8_AF_index;

			#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
			u8_Calib_AF_index									 = 	st_Calib_AMFM_App_AFList_Info.u8_AF_index;
			#endif

			if(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq == u32_AFupdateFreq)
			{
			    if (e_AMFM_APP_AF_UpdateReplystatus == REPLYSTATUS_SUCCESS)
				{
					pst_me_amfm_app_inst->u32_AMFM_quality = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset);
				
					if(st_AMFM_App_AFList_Info.u32_af_update_newfreq == st_AMFM_App_AFList_Info.u32_curr_freq)
					{
						/* update response for the same Af station since check failed and update should be given to original frequency to demute*/
					
						st_AMFM_App_AFList_Info.u32_Qua_curr = pst_me_amfm_app_inst->u32_AMFM_quality;
					
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.u32_Qua_curr = pst_me_amfm_app_inst->u32_AMFM_quality;
						#endif
					
						AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);
			
						if( st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold )
						{
							pst_me_amfm_app_inst->u8_Curr_qua_check_count=0;
						
						}
						else
						{
							pst_me_amfm_app_inst->u8_Curr_qua_check_count++;
					
						}	
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay, AMFM_CURR_STATION_QUALITY_CHECK_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}
						else
						{
					//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_CURR_STATION_QUALITY_CHECK_TIMERID);
						}	/* update response for AF list frequencies. QUALITY CHECK SUCCEEDED*/
					}
					else
					{
						/* update response for AF list frequencies. QUALITY CHECK SUCCEEDED*/
			
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].s32_BBFieldStrength  = pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].u32_UltrasonicNoise = pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].u32_Multipath = pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].u32_FrequencyOffset = pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;

						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].u32_curr_qual =pst_me_amfm_app_inst->u32_AMFM_quality;

						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_AF_index].s32_BBFieldStrength  = pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength;
							st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_AF_index].u32_UltrasonicNoise = pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise;
							st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_AF_index].u32_Multipath = pst_me_amfm_app_inst->st_QualParam.u32_Multipath;
							st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_AF_index].u32_FrequencyOffset = pst_me_amfm_app_inst->st_QualParam.u32_FrequencyOffset;
							st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_AF_index].u32_curr_qual =pst_me_amfm_app_inst->u32_AMFM_quality;
						#endif
						AMFM_App_AF_Qual_Avg_computation(&st_AMFM_App_AFList_Info,pst_me_amfm_app_inst->u32_AF_qual_incr_alpha,pst_me_amfm_app_inst->u32_AF_qual_decr_alpha);
					
						/*update request to next AF freq*/
						/*start timer to  new update request */
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_Next_Freq_Update_Delay, AMFM_AF_UPDATE_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
						else
						{
					//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
					}
				}
				else 
				{
					/*UPDATE REQUEST FAILED */
					/*Tune ctrl will always give success*/
					if(st_AMFM_App_AFList_Info.u32_af_update_newfreq == (st_AMFM_App_AFList_Info.u32_curr_freq)) 
					{
						/*request update for next AF */
						AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.u32_af_update_newfreq);		
					}
					else
					{
						/*START TIMER TO GIVE NEXT UPDATE */
						st_AMFM_TimerId.u32_AMFM_APP_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_Next_Freq_Update_Delay, AMFM_AF_UPDATE_TIMERID, RADIO_AM_FM_APP);
						if(st_AMFM_TimerId.u32_AMFM_APP_TimerId <= 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 5 Failed to start %d message will not be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
						else
						{
					//		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 5 started successfully %d message will be posted  ",AMFM_AF_UPDATE_TIMERID);
						}
					
					}
				}
			}	
			else
			{
				/* AF update response is for other frequency.So dropping this message and waiting for correct response */
			}
		}
		break;

		case AMFM_APP_AFFREQ_CHECK_DONE_RESID :
		{
			AMFM_APP_ExtractParameterFromMessage(&e_AMFM_APP_AF_CheckReplystatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			AMFM_APP_ExtractParameterFromMessage(&(u16_afpi),(const Tchar *)(pst_msg->data),(Tu16)sizeof(Tu16),&u32_DataSlotIndex);
			
			u8_Best_AF_station_Index 				= st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
			#ifdef CALIBRATION_TOOL
				u8_Calib_best_AF_index 			= st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	
			#endif
			
			if (e_AMFM_APP_AF_CheckReplystatus == REPLYSTATUS_SUCCESS)
			{
				b_RetValue	=	AMFM_App_ValidatePIcode(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,u16_afpi);

				if(b_RetValue	== TRUE)
				{
					st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;

					st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI	= u16_afpi;
					
					#ifdef CALIBRATION_TOOL
						st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
						st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_best_AF_index].u16_PI	= u16_afpi;
					#endif
					
					AMFM_Tuner_Ctrl_Request_FM_Jump(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);		

					b_return_value = AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq,u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
					if((b_return_value== TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
					{
						/*code to store LSM data into NVM */
						u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) +(sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
						
						if(u8_NVM_ret == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during DAB-FM Linking AF check  ");
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails  during DAB-FM Linking AF check  ");
						}
					}
				}
				else
				{
					/*pi doesnt matches area code differs ,totally different  pi*/
					
					if(u16_afpi !=0)
					{
						/* pi is  not 0 so mark as NEG */
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI	= u16_afpi;
						st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
						
						#ifdef CALIBRATION_TOOL
							st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_best_AF_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_NEG;
							st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Calib_best_AF_index].u16_PI	= u16_afpi;
						#endif
					
						b_return_value = AMFM_App_Learn_Memory_updation(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq, u16_afpi,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
						if((b_return_value==TRUE)&&(b_NVM_ActiveUpdateSwitch==TRUE))
						{
							/*code to store LSM data into NVM */
							u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
							if(u8_NVM_ret == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success  during DAB-FM Linking AF check  ");
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during DAB-FM Linking AF check   ");
							}
						}

						/*Checking NEG timer is started or not */
						if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)
						{
							/*Checking any AF is having status as  AMFM_APP_PI_STATUS_NEG*/
							b_return_value = AMFM_App_DAB2FM_Check_NEG_flag_AFList(st_AMFM_App_AFList_Info.u8_NumAFList,st_AMFM_App_AFList_Info.ast_current_AFStation_Info);
						
							if(b_return_value ==TRUE)
							{
								/* u8_NEG_TimerStartCheck flag is set as timer is stated */
								pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;

								/*Starting the timer as NEG flag is present */
								st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
								if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
								}
								else
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
								}
							}
						}
					}	
					
					b_return_value = AMFM_App_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
					
					if (b_return_value == FALSE)
					{
						/*Best AF available check should be given*/
						AMFM_APP_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
					}
					else
					{
						/*best AF Not available wait de-mute and wait for two seconds and repeat the operation*/
						st_AMFM_App_AFList_Info.u32_af_update_newfreq=((st_AMFM_App_AFList_Info.u32_curr_freq));

						#ifdef CALIBRATION_TOOL				
							st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq=((st_AMFM_App_AFList_Info.u32_curr_freq));
						#endif
						
						/*request update for next AF */
						AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.u32_af_update_newfreq);
		
					}
				}
			}
			else
			{
			
				b_return_value = AMFM_App_Best_AF_Avaliability_Check(&st_AMFM_App_AFList_Info);
				if (b_return_value == FALSE)
				{
					AMFM_APP_Best_Freq_AF_check(&st_AMFM_App_AFList_Info);
				}
				else
				{
					/*best AF Not availble wait demute and wait for two seconds and repeat the operation*/
					st_AMFM_App_AFList_Info.u32_af_update_newfreq=((st_AMFM_App_AFList_Info.u32_curr_freq));
					
					#ifdef CALIBRATION_TOOL				
							st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq=((st_AMFM_App_AFList_Info.u32_curr_freq));
					#endif
					
					/*request update for next AF */
					AMFM_Tuner_Ctrl_Request_AF_Update(st_AMFM_App_AFList_Info.u32_af_update_newfreq);
				}
			}			
		}
		break;

		case AMFM_APP_SELECT_STATION_DONE_RESID:
		{
			u8_Best_AF_station_Index	= st_AMFM_App_AFList_Info.u8_Best_AF_station_Index;	

			AMFM_APP_ExtractParameterFromMessage(&e_PlaySelectStationReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);

			AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);

			st_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
			
			#ifdef CALIBRATION_TOOL				
				st_Calib_AMFM_App_AFList_Info.b_af_check_flag = FALSE;
			#endif

			
			if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency !=  (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq))
			{
				if(e_PlaySelectStationReplyStatus == REPLYSTATUS_SUCCESS)
				{	

					e_temp_BestPIType = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType;
					
					/* Clearing st_current_station structure before updating into it */
					memset(&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_Best_PI_Info));

					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = e_temp_BestPIType;

					/* Updating details of Best PI into st_Best_PI_Info structure */
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency	=  pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_freq;
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode		=  st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].u16_PI;

					if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == TRUE)
					{
						pst_me_amfm_app_inst->b_NEG_TimerStartCheck = FALSE;
							
						/*Stopping the timer as there is no AF with AMFM_APP_PI_STATUS_NEG status */
						if(st_AMFM_TimerId.u32_Status_Check_TimerId >0)
						{
						b_return_value = SYS_TIMER_STOP(st_AMFM_TimerId.u32_Status_Check_TimerId);
						if(b_return_value == FALSE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]   Time 9 failed to stop message will be posted");
						}
						else
						{
							/* After stop Timer is success,reset the timer id */
							st_AMFM_TimerId.u32_Status_Check_TimerId = 0;
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Time 9 successfully stopped  message will not be posted   ");
							}
						}
						else
						{
							/*MISRAC*/
						}
					}
				
					if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType == AMFM_APP_NO_LINK_AVAILABLE)
					{
						if(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode == (Tu16)pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_Implicit_sid)
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_IMPLICIT_LINK;
						}	
						else
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType = AMFM_APP_HARDLINK;
						}		
					}
					
					/* Before notifying next alternating Best FM station, this needs to be notified  */				
					AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  BG : Best PI is changed. AF switching  ");
				
					/* Notifying next alternating Best FM station and its Quality to DAB Tuner Ctrl via Radio Manager Application */
					AMFM_App_Notify_BestPI(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode,
											st_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Best_AF_station_Index].u32_avg_qual, 
										   	pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency, 
										   	pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.e_BestPIType,
										   	pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
										   	pst_me_amfm_app_inst->u8_DABFM_LinkingCharset);

					if (pst_me_amfm_app_inst->e_DAB2FM_Linking_status == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
					{
						AMFM_App_Notify_BestPI_Changed();
					}
				
					/* Requesting tuner ctrl to keep on sending Quality of Best PI station */
					AMFM_Tuner_Ctrl_Request_Bg_Quality();

					pst_me_amfm_app_inst->u8_aflistindex=0;
					
					/* Transisting to monitoring state */
					HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_busy_linking_monitoring_state);		
					}
					else
					{
							/* Nothing to do. */		
					}
			}
			else
			{
					
			}	
		}
		break;

		case   AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID:
		{	
			/* Reading current station info  from the message */
			if(	st_AMFM_App_AFList_Info.b_af_check_flag != TRUE)
			{
				AMFM_APP_ExtractParameterFromMessage(&(pst_me_amfm_app_inst->st_QualParam),(const Tchar *)(pst_msg->data),(Tu16)sizeof(pst_me_amfm_app_inst->st_QualParam),&u32_DataSlotIndex);

				st_AMFM_App_AFList_Info.u32_Qua_curr =  QUAL_CalcFmQual(pst_me_amfm_app_inst->st_QualParam.s32_BBFieldStrength, pst_me_amfm_app_inst->st_QualParam.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_QualParam.u32_Multipath, NULL, NULL, 0);
				
				AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

				/* Checking whether quality of Currently tuned Best FM station is less than Qmin */
				if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin)  
				{
					/* Notifying link not available to DAB if quality of 1st AF from sorted AF list is less than Qmin */
					if(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[0].u32_avg_qual < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin) 
					{	
						/* If this block executes, then Best FM station is also nosisy as well as other AF too*/
						AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);

						if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
						{
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
						}
						
						/* This flag is enabled because after updating FM STL needs to do transition to Linking state */
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE; 
				
						/* Transisting to active_sleep_background_process_stl_state */
						HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
					}
					else
					{
						/*Quality degraded below threshold*/
						AMFM_App_AFTune_Restart(&st_AMFM_App_AFList_Info);
					}
				}
				else if((st_AMFM_App_AFList_Info.u32_Qua_avg  != (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality)) )
				{
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality = st_AMFM_App_AFList_Info.u32_Qua_avg;
				
					AMFM_App_Notify_PIQuality(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality,
											      pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,
											      AMFM_APP_SAME_PSN,
											      pst_me_amfm_app_inst->u8_DABFM_LinkingCharset);
				}
				else
				{
					/* Nothing to do . Added just for MISRA C */
				}
			}
		}
		break;
		
		case AMFM_APP_TUNER_STATUS_NOTIFYID:
		{
			/* Reading current station info  from the message */
			if(	st_AMFM_App_AFList_Info.b_af_check_flag != TRUE)
			{
				memset(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo));
				
				/* Reading current station info  from the message */
				AMFM_APP_ExtractParameterFromMessage(&pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo,(const Tchar *)(pst_msg->data),(Tu16)sizeof(Ts_AMFM_Tuner_Ctrl_CurrStationInfo),&u32_DataSlotIndex);
				if(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u16_pi == pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u16_PIcode)
				{

					st_AMFM_App_AFList_Info.u32_Qua_curr = QUAL_CalcFmQual(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.s32_BBFieldStrength, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_UltrasonicNoise, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_Multipath, NULL, NULL, 0);
									
					AMFM_App_Current_Qual_Avg_Computation(&st_AMFM_App_AFList_Info);

					s8_RetVal = strncmp((char *)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,( char *)pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME) ;

					/* Checking whether quality of Currently tuned Best FM station is less than Qmin */
					if(st_AMFM_App_AFList_Info.u32_Qua_avg < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin)  
					{
						/* Notifying link not available to DAB if quality of 1st AF from sorted AF list is less than Qmin */
						if(st_AMFM_App_AFList_Info.ast_current_AFStation_Info[0].u32_avg_qual < pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u32_QualityMin) 
						{	
							/* If this block executes, then Best FM station is also nosisy as well as other AF too*/
							AMFM_App_Notify_LinkingStatus(RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE);

							if (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_LinkingType == AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK)
							{
								pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_PIList.u8_PIcount = pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.u8_ActualPICount;
							}
						
							/* This flag is enabled because after updating FM STL needs to do transition to Linking state */
							pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.b_DAB2FM_LinkingFlag = TRUE; 
				
							/* Transisting to active_sleep_background_process_stl_state */
							HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_active_sleep_background_processSTL_state);
						}
						else
						{
							/*Quality degraded below threshold*/
							AMFM_App_AFTune_Restart(&st_AMFM_App_AFList_Info);
						}
					}
					else if((st_AMFM_App_AFList_Info.u32_Qua_avg  != (pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality)) || (s8_RetVal != 0) )
					{
						pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality = st_AMFM_App_AFList_Info.u32_Qua_avg;
						
						SYS_RADIO_MEMCPY(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,(const void *)pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
						//memcpy(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.au8_psn,st_TunerctrlCurrentStationInfo.au8_ps,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
					
						AMFM_App_Notify_PIQuality(pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Quality, pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.au8_ps,
												    (s8_RetVal == 0) ? AMFM_APP_SAME_PSN : AMFM_APP_NEW_PSN,
												    pst_me_amfm_app_inst->u8_DABFM_LinkingCharset);
					}


					if((pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList !=0) && (pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.b_AF_Checkbit == TRUE))
					{				
							pst_me_amfm_app_inst->u32_ReqFrequency=((pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.u32_Frequency) );
		            
							/* Updating AF list as well HL freq into st_AMFM_App_AFList_Info structure */	
							AMFM_App_Append_AF_Into_List(pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u8_NumAFeqList,pst_me_amfm_app_inst->st_TunerctrlCurrentStationInfo.u32_AFeqList,&pst_me_amfm_app_inst->st_DAB_FM_LinkingParam,&st_AMFM_App_AFList_Info,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);

							/*Checking NEG timer is started or not */
							if(pst_me_amfm_app_inst->b_NEG_TimerStartCheck == FALSE)
							{
								/*Checking any AF is having status as  AMFM_APP_PI_STATUS_NEG*/
								b_return_value = AMFM_App_DAB2FM_Check_NEG_flag_AFList(st_AMFM_App_AFList_Info.u8_NumAFList,st_AMFM_App_AFList_Info.ast_current_AFStation_Info);
							
								if(b_return_value == TRUE)
								{
									/* u8_NEG_TimerStartCheck flag is set as timer is stated */
									pst_me_amfm_app_inst->b_NEG_TimerStartCheck =TRUE;

									/*Starting the timer as NEG flag is present */
						
									st_AMFM_TimerId.u32_Status_Check_TimerId=SYS_TIMER_START(pst_me_amfm_app_inst->u32_AF_StatusTimeout , AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID, RADIO_AM_FM_APP);
									if(st_AMFM_TimerId.u32_Status_Check_TimerId <= 0)
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  Timer 9 Failed to start %d message will not be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
									}
									else
									{
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP]  Timer 9 started successfully %d message will be posted  ",AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID);
									}
								}
							}
					}
				}	
				else
				{
					pst_me_amfm_app_inst->st_DAB_FM_LinkingParam.st_Best_PI_Info.b_PIchangedFlag  = TRUE;
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
			pst_ret = pst_msg;
		}
		break;
	}
	return pst_ret;
}
/*===========================================================================*/
/*				Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveStopHndlr		 */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_APP_INST_HSM_ActiveStopHndlr(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*							pst_ret				  = NULL;                              /* Pointer to be updated to the  msg pst_msg  at default case*/
	Te_RADIO_ReplyStatus				e_ShutdownReplyStatus = REPLYSTATUS_REQ_NOT_HANDLED;		/* enum variable to hold the  shut down reply status*/
    Tu32								u32_DataSlotIndex	  = 0;                                 /* variable to update the message slot index */
	Ts_Sys_Msg							*pst_ShutDownDoneMsg	  =	&st_amfm_app_res_msg;			/* pointer to message structure defined globally */
	Tu32 								u32_NVM_Update_Param=0;
	Tu8									u8_NVM_ret=0;
	
	switch (pst_msg->msg_id)			
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;

		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] Handler: AMFM_APP_INST_HSM_ActiveStopHndlr");

			/* Updating the current state name  used for debugging purpose */
			SYS_RADIO_MEMCPY((void *)(pst_me_amfm_app_inst->pu8_curr_state_name) ,(const void *) "amfm_app_inst_hsm_active_stop_state",sizeof("amfm_app_inst_hsm_active_stop_state"));
			if(b_NVM_ActiveUpdateSwitch==FALSE)
			{
				/*code to store LSM data into NVM */
				u8_NVM_ret = Sys_NVM_Write(NVM_ID_TUNER_AMFM_APP,&pst_me_amfm_app_inst->st_LSM_FM_Info,sizeof(Ts_AMFM_App_LSM_FM_Band) + (sizeof(Ts_AMFM_App_AF_learn_mem)*AMFM_APP_NO_STATIONS_FM_BAND) ,&u32_NVM_Update_Param);
				if(u8_NVM_ret == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  NVM write success during shutdown ");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_APP]  NVM write fails during shutdown ");
				}
			}
			/* Sending Shutdown Request to Tuner ctrl */
			AMFM_Tuner_Ctrl_Request_Shutdown();		
		}
		break;
		
		case AMFM_APP_SHUTDOWN_DONE_RESID:
		{
				
			/* Sending AMFM_APP_INST_HSM_SHUTDOWN_DONE message from  amfm_app_hsm asynchronously. */

			/* Reading e_ShutdownReplyStatus reply status from the message */
			AMFM_APP_ExtractParameterFromMessage(&e_ShutdownReplyStatus, (const Tchar *)(pst_msg->data), (Tu16)sizeof(Te_RADIO_ReplyStatus), &u32_DataSlotIndex);
			
            /* sending shut down done message from instance hsm to main hsm */
            AMFM_APP_SendMsgtoMainhsm(pst_ShutDownDoneMsg,AMFM_APP_INST_HSM_SHUTDOWN_DONE);
            
			/* Sending response to Radio Manager Application inorder to indicate shutdown is done */
			AMFM_APP_Response_Shutdown(e_ShutdownReplyStatus);

			/* Transisting to inactive  state */
			HSM_STATE_TRANSITION(pst_me_amfm_app_inst, &amfm_app_inst_hsm_inactive_state);	
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

/*-----------------------------------------------------------------------------
    Public function definitions
-----------------------------------------------------------------------------*/		

/*===========================================================================*/
/*				void AMFM_APP_INST_HSM_Init									 */ 		
/*===========================================================================*/
void AMFM_APP_INST_HSM_Init(Ts_AMFM_App_inst_hsm* pst_me_amfm_app_inst)
{
	if(pst_me_amfm_app_inst != NULL)
	{
		/* Clearing the HSM amfm_app_inst_hsm buffer */
		memset(pst_me_amfm_app_inst,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_inst_hsm)); 	
		
		pst_me_amfm_app_inst->pu8_curr_state_name = (const char *) (pst_me_amfm_app_inst->str_state);	
		/*Constructing the HSM amfm_app_inst_hsm */
		HSM_CTOR(&pst_me_amfm_app_inst->st_inst_hsm,&amfm_app_inst_hsm_top_state,AMFM_APP_INST_HSM_CID);
		
		/* Starting the HSM amfm_app_inst_hsm. After this call,current state of inst hsm will be inactive state */
		HSM_ON_START(&pst_me_amfm_app_inst -> st_inst_hsm );
			
	}
	else
	{
		/* Send  error message */
	}	
}

/*===========================================================================*/
/*				void AMFM_APP_INST_HSM_SendInternalMsg						 */ 		
/*===========================================================================*/
void AMFM_APP_INST_HSM_SendInternalMsg(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Tu16 u16_msgid)
{
	if(pst_me_amfm_app_inst != NULL)
	{	
		Ts_Sys_Msg   *pst_Internal_msg = &st_amfm_app_req_msg;			/* pointer to message structure defined globally */
		
		/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure defined in amfm_app_request.c */	
		AMFM_APP_MessageInit(pst_Internal_msg,AMFM_APP_INST_HSM_CID,u16_msgid,RADIO_AM_FM_APP);	
		
		HSM_ON_MSG(&pst_me_amfm_app_inst->st_inst_hsm,pst_Internal_msg);	
	}
	else
	{
		/* Send error message */
	}			
}
/**/
/*===========================================================================*/
/*				void AMFM_APP_INST_HSM_MessageHandler						 */
/*===========================================================================*/
void AMFM_APP_INST_HSM_MessageHandler(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_Sys_Msg* pst_msg)
{
	if(pst_me_amfm_app_inst != NULL && pst_msg != NULL)
	{
		HSM_ON_MSG(&pst_me_amfm_app_inst->st_inst_hsm,pst_msg);
	}	
	else
	{
		/* Send error message */
	}	
}
