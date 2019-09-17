

/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Instance_hsm.c                                                                    *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     * 
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: SC_AMFM_TUNER_CTRL                                                                *
*  Description			: Processing the all feature requests in instance hsm handler functions.            *
*                                                                                                           *
*                                                                                                           * 
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "AMFM_Tuner_Ctrl_Instance_hsm.h"
#include "AMFM_Tuner_Ctrl_App.h"
#include "AMFM_Tuner_Ctrl_Response.h"
#include "AMFM_Tuner_Ctrl_Notify.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
#define BG_AMSCAN 0
/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables 
-----------------------------------------------------------------------------*/
Tu8 							scan_index_FM;
Tu8 							u8_count = 0;
Tu8 							flag = 0; //for RDS timer purpose added
Tu32 							u32_interqual = 0;
Tu32    						u32_SOC_Frequency;
Tu32 							u32_UltrasonicNoise;
Tu32 							u32_Multipath;
Tu32							u32_FrequencyOffset;
//Tu32                          u32_Receiver_Tuned_freq = 0;
Tu32 							u32_Quality_Read_freq;
Ts32 							s32_BBFieldStrength;
Tbool							gb_AMFMTunerI2CErrorFlag;
Tbool							gb_AMFMTunerI2CErrorhandler;
Tbool                           e_audio_source_select;
Ts_AMFM_Tuner_Ctrl_Timer_Ids    st_AMFM_Tuner_Ctrl_Timer_Ids;
Ts_AMFM_Tuner_Ctrl_RDS_Data 	st_recv_data;
Ts_AMFM_Tuner_Ctrl_CurrStationInfo  ast_Scaninfo[AMFM_TUNER_CTRL_MAX_STATIONS];
EtalBcastQualityContainer pBcastQualityContainer;
EtalSeekStatus st_amfm_tuner_ctrl_seek_status;
EtalSeekStatus st_amfm_tuner_ctrl_scan_status;
EtalBcastQualityContainer st_amfm_tuner_ctrl_Quality;
extern Tbool e_audio_source_select;
EtalBcastQualityContainer St_Tuner_FMQual_parametrs;

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/


/* HSM state hierarchy */
	
	HSM_CREATE_STATE(				amfm_tuner_ctrl_inst_hsm_top_state,			               				NULL,												AMFM_TUNER_CTRL_INST_HSM_TopHndlr,			        		"amfm_tuner_ctrl_inst_hsm_top_state");
		HSM_CREATE_STATE(			amfm_tuner_ctrl_inst_hsm_inactive_state,	               				&amfm_tuner_ctrl_inst_hsm_top_state,		  		AMFM_TUNER_CTRL_INST_HSM_InactiveHndlr,             		"amfm_tuner_ctrl_inst_hsm_inactive_state");
		HSM_CREATE_STATE(			amfm_tuner_ctrl_inst_hsm_active_state,                    				&amfm_tuner_ctrl_inst_hsm_top_state,          		AMFM_TUNER_CTRL_INST_HSM_ActiveHndlr,               		"amfm_tuner_ctrl_inst_hsm_active_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_start_state,          					&amfm_tuner_ctrl_inst_hsm_active_state,         	AMFM_TUNER_CTRL_INST_HSM_ActiveStartHndlr,          		"amfm_tuner_ctrl_inst_hsm_active_start_state");
			HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_idle_state,           					&amfm_tuner_ctrl_inst_hsm_active_state,         	AMFM_TUNER_CTRL_INST_HSM_ActiveIdleHndlr,           		"amfm_tuner_ctrl_inst_hsm_active_idle_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_idle_listen_state,           			&amfm_tuner_ctrl_inst_hsm_active_idle_state,    	AMFM_TUNER_CTRL_INST_HSM_ActiveIdleListenHndlr,        		"amfm_tuner_ctrl_inst_hsm_active_idle_listen_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_idle_af_update_tune_state,  			&amfm_tuner_ctrl_inst_hsm_active_idle_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveIdleAFUpdateTuneHndlr,  	 	"amfm_tuner_ctrl_inst_hsm_active_idle_af_update_tune_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_idle_fm_check_tune_state,  				&amfm_tuner_ctrl_inst_hsm_active_idle_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFMCheckTuneHndlr,    	"amfm_tuner_ctrl_inst_hsm_active_idle_fm_check_tune_state");
        		HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_idle_fm_jump_tune_state,  				&amfm_tuner_ctrl_inst_hsm_active_idle_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFmJumpTuneHndlr,     	"amfm_tuner_ctrl_inst_hsm_active_idle_fm_jump_tune_state");
			
			HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_busy_state,           					&amfm_tuner_ctrl_inst_hsm_active_state,       		AMFM_TUNER_CTRL_INST_HSM_ActiveBusyHndlr,           		"amfm_tuner_ctrl_inst_hsm_active_busy_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_busy_tune_state,  						&amfm_tuner_ctrl_inst_hsm_active_busy_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveBusyTuneHndlr,       		"amfm_tuner_ctrl_inst_hsm_active_busy_tune_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_busy_stlscan_state,  					&amfm_tuner_ctrl_inst_hsm_active_busy_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveBusySTLScanHndlr,       		"amfm_tuner_ctrl_inst_hsm_active_busy_stlscan_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_busy_seekupdown_state, 					&amfm_tuner_ctrl_inst_hsm_active_busy_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveBusySeekUpDownHndlr, 		"amfm_tuner_ctrl_inst_hsm_active_busy_seekupdown_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_busy_piseek_state, 						&amfm_tuner_ctrl_inst_hsm_active_busy_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveBusyPISeekHndlr, 			"amfm_tuner_ctrl_inst_hsm_active_busy_piseek_state");
			HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_state,          					&amfm_tuner_ctrl_inst_hsm_active_state,       		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepHndlr,          		"amfm_tuner_ctrl_inst_hsm_active_sleep_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_bg_quality_state,        			&amfm_tuner_ctrl_inst_hsm_active_sleep_state,		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgQualityHndlr,    		"amfm_tuner_ctrl_inst_hsm_active_sleep_bg_quality_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_bg_afstop_state,        			&amfm_tuner_ctrl_inst_hsm_active_sleep_state,		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFStopHndlr,    		"amfm_tuner_ctrl_inst_hsm_active_sleep_bg_afstop_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_bg_scan_state, 					&amfm_tuner_ctrl_inst_hsm_active_sleep_state,		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgScanHndlr, 			"amfm_tuner_ctrl_inst_hsm_active_sleep_bg_scan_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_bg_tune_state, 					&amfm_tuner_ctrl_inst_hsm_active_sleep_state,		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgTuneHndlr, 			"amfm_tuner_ctrl_inst_hsm_active_sleep_bg_tune_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_bg_af_update_state,   			&amfm_tuner_ctrl_inst_hsm_active_sleep_state, 		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFUpdateTuneHndlr,   	"amfm_tuner_ctrl_inst_hsm_active_sleep_bg_af_update_state");
				HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_bg_fm_check_tune_state,  			&amfm_tuner_ctrl_inst_hsm_active_sleep_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFMCheckTuneHndlr,     "amfm_tuner_ctrl_inst_hsm_active_sleep_bg_fm_check_tune_state");
        		HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_sleep_bg_fm_jump_tune_state,   			&amfm_tuner_ctrl_inst_hsm_active_sleep_state,  		AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFmJumpTuneHndlr,      "amfm_tuner_ctrl_inst_hsm_active_sleep_bg_fm_jump_tune_state");
			HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_error_state,                 					&amfm_tuner_ctrl_inst_hsm_active_state,       		AMFM_TUNER_CTRL_INST_HSM_ActiveErrorHndlr,          		"amfm_tuner_ctrl_inst_hsm_active_error_state");
			HSM_CREATE_STATE(	amfm_tuner_ctrl_inst_hsm_active_stop_state,           					&amfm_tuner_ctrl_inst_hsm_active_state,       		AMFM_TUNER_CTRL_INST_HSM_ActiveStopHndlr,           		"amfm_tuner_ctrl_inst_hsm_active_stop_state");
				

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_TopHndlr                    */
/*===========================================================================*/

Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_TopHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 				pst_ret 	= NULL;
	

	switch((pst_msg->msg_id))
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;

		case HSM_MSGID_START:
		{
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_TopHndlr");
			
			/* Updating the current state name  used for debuging purpose */
		    SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str,"AMFM_TUNER_CTRL_INST_HSM_TopHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_TopHndlr"));
			
			/* Transit to AMFM tuner control instance hsm inactive handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_inactive_state);
		}
		break;

		case HSM_MSGID_EXIT:
		{

		}
		break;

		default:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] TOP_Handler_Message %d\n",pst_msg->msg_id);
			pst_ret = pst_msg;
		}
		break;
	}
	return pst_ret;
}


/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_InactiveHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_InactiveHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 			pst_ret 	= NULL;
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_InactiveHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str,"AMFM_TUNER_CTRL_INST_HSM_InactiveHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_InactiveHndlr"));
		}
		break;
		case AMFM_TUNER_CTRL_INST_START_REQID:
		{
				/* Transit to AMFM tuner control instance hsm active start handler */
            HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_start_state);
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveHndlr                 */
/*===========================================================================*/
Ts_Sys_Msg*   AMFM_TUNER_CTRL_INST_HSM_ActiveHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg*  			pst_ret    	= NULL;                             
	Te_RADIO_ReplyStatus    e_CancelReplyStatus     = REPLYSTATUS_FAILURE;                
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			 RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveHndlr");
		
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str,"AMFM_TUNER_CTRL_INST_HSM_ActiveHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveHndlr"));
		}
		break;
		case  AMFM_TUNER_CTRL_STOP:
		{
			/* Transit to AMFM tuner control instance hsm active stop handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst,&amfm_tuner_ctrl_inst_hsm_active_stop_state);
		}
		break;
		case AMFM_TUNER_CTRL_ERROR_REQID:
		{
			/* Transit to AMFM tuner control instance hsm error handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst,&amfm_tuner_ctrl_inst_hsm_error_state);
		}      
		break;
		case AMFM_TUNER_CTRL_AMFMTUNER_ABNORMAL_NOTIFICATION:
		{
			/*Debug Log for AMFM Tuner Ctrl POR occured status*/
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT,"[RADIO][AMFM_TC] : RADIO AMFM TUNER CTRL POWER ON RESET OCCURED");

			if(gb_AMFMTunerI2CErrorFlag == TRUE)
			{
				pst_me_amfm_tuner_ctrl_inst->e_AMFMTunerErrorType = RADIO_FRMWK_AMFMTUNER_I2CERROR;
			}
			else
			{
				pst_me_amfm_tuner_ctrl_inst->e_AMFMTunerErrorType = RADIO_FRMWK_AMFMTUNER_INTERNAL_RESET;
			}
			if(gb_AMFMTunerI2CErrorhandler != TRUE)
			{
				/* Stopping the RDS Timer if the AMFMTuner_Rest or I2c Error occurred other than AF_Update Handler */
				AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);	
				
				
			}
			else
			{
				/* clearing the check condition for next iteration */
				gb_AMFMTunerI2CErrorhandler = FALSE;
			}
			/* Sending AMFM Tuner Abnormal Status to Higher layers */
			AMFM_Tuner_Ctrl_Notify_AMFMTuner_Abnormal(pst_me_amfm_tuner_ctrl_inst->e_AMFMTunerErrorType);
			
			if(pst_me_amfm_tuner_ctrl_inst->e_Tuner_State == AMFM_TUNER_CTRL_FOREGROUND)
			{
				HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_state);
			}
			else
			{
				HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_state);
			}
			
		}
		break;
		case AMFM_TUNER_CTRL_ANNOUNCEMENT_CANCEL_REQID:
		{
			/* EON Cancel Request for currently on going EON Annoucement */
		    
			if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id > 0)
			{
				if(SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : For EON cancel timer stop  is failed ");	
				}
				else
				{
					st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
				}
			}	
			else
			{
			   /* MISRA C */
			}

			e_CancelReplyStatus = REPLYSTATUS_SUCCESS;
			AMFM_Tuner_Ctrl_Response_Announcement_Cancel(e_CancelReplyStatus);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_state);
		}
		break;
		case AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_REQID:
		{
			/*sending Factory Reset done message to main hsm*/
            AMFM_Tuner_Ctrl_stopreply(RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_DONE);
			/* Transit to AMFM tuner control instance hsm inactive handler*/
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst,&amfm_tuner_ctrl_inst_hsm_inactive_state);
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveStartHndlr                     */
/*===========================================================================*/

Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveStartHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 								pst_ret 				= NULL; /* mark the message as handeled */
	

	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveStartHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str ,"AMFM_TUNER_CTRL_INST_HSM_ActiveStartHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveStartHndlr"));
			
			pst_me_amfm_tuner_ctrl_inst->e_Tuner_State = AMFM_TUNER_CTRL_BACKGROUND;
            /* Request to AMFM tuner control soc to startup */
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNER_CONFIGURATION(pst_me_amfm_tuner_ctrl_inst->e_Market, TUN_BAND_FM, pst_me_amfm_tuner_ctrl_inst->e_Tuner_State);

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{

				AMFM_Tuner_Ctrl_startreply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_INST_HSM_START_DONE);

				/* Transit to AMFM tuner control instance hsm active idle handler */
				HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_state);
			}
			else
			{
				/* MISRA C */
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleHndlr                              */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 								pst_ret 					= NULL; /* mark the message as handeled */
	Te_RADIO_ReplyStatus 	                    e_DeActivateReplyStatus 	= REPLYSTATUS_REQ_CANCELLED;
	Tu32 										slot 						= 0;
	
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveIdleHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveIdleHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveIdleHndlr"));
			
		}
		break;
		case AMFM_TUNER_CTRL_TUNE_REQID:
		{
			/* Reading tune requset parameters from the message */
            AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq),(Tchar *)(pst_msg->data),(Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq)),&slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band),(Tchar *)(pst_msg->data),(Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band)),&slot);
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNER_CONFIGURATION(pst_me_amfm_tuner_ctrl_inst->e_Market, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band, pst_me_amfm_tuner_ctrl_inst->e_Tuner_State);
				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					/* Transit to AMFM tuner control instance hsm active busy tune handler */
					HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_busy_tune_state);
				}
				else
				{
					/* MISRA C */
				}
		}
		break;
		case  AMFM_TUNER_CTRL_DEACTIVATE_REQID:
		{
			/*Updating the Enum to Background */
			pst_me_amfm_tuner_ctrl_inst->e_Tuner_State = AMFM_TUNER_CTRL_BACKGROUND;
			e_DeActivateReplyStatus = REPLYSTATUS_SUCCESS;
            /* sending respone to AMFM application layer */
		    AMFM_Tuner_Ctrl_Response_DeActivate(e_DeActivateReplyStatus);
            /* Transit to AMFM tuner control instance hsm active sleep handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_state);
		}
		break; 
		
		case AMFM_TUNER_CTRL_SEEK_UP_DOWN_REQID:
		{			
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection),(Tchar *)(pst_msg->data),(Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection)),&slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Te_AMFM_Tuner_Ctrl_Band)), &slot);
			
			
			/* Transit to AMFM tuner control instance hsm active sleep handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_busy_seekupdown_state);
			
		}
		break;
		case AMFM_TUNER_CTRL_PISEEK_REQID:
		{			
			
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->u16_pi), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->u16_pi)), &slot);
			
			/* Transit to AMFM tuner control instance hsm active sleep handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_busy_piseek_state);
			
		}
		break;
		case AMFM_TUNER_CTRL_GETSTATIONLIST_REQID:
		{
        	/* Reading scan request parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band)), &slot);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_busy_stlscan_state);
		}
		break;
		case AMFM_TUNER_CTRL_AF_UPDATE_REQID:
		{
			/* Reading tune requset parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->u32_freq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Tu32)), &slot);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_af_update_tune_state);
		}
		break;
		case AMFM_TUNER_CTRL_LOWSIGNAL_AF_CHECK_REQID:
		{
			/* Reading tune requset parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->u32_freq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Tu32)), &slot);
			
			/*FM_Check for Low Signal Condition */
			pst_me_amfm_tuner_ctrl_inst->b_PICheckFlage =  TRUE;
			
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_fm_check_tune_state);

		}
		break;
		case AMFM_TUNER_CTRL_AF_CHECK_REQID:
		{
			/* Reading tune requset parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->u32_freq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Tu32)), &slot);

			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_fm_check_tune_state);
		}
		break;
		case AMFM_TUNER_CTRL_AF_JUMP_REQID:
		{
			/* Reading tune requset parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Tu32)), &slot);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_fm_jump_tune_state);
		}
		break;
		case HSM_MSGID_EXIT:
		{
			/*  free all timers */
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleListenHndlr                              */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleListenHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 				pst_ret					= NULL; /* mark the message as handeled */
	Tu8  						loop_index 				= (Tu8)0;
	Tu32 						slot 					= 0;
	Ts32 						PS_CMP 					= 0;
	Tbool						b_PI_CMP;
	Tbool 						b_AF_Flag 				= FALSE;
	Ts32 						RT_Flag 				= 0;
	Tbool						b_CT_CMP 			    = FALSE;
	Tbool                       b_PTY_CMP               = FALSE;
	Tbool                       b_TATP_CMP              = FALSE;

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00,sizeof(st_recv_data));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveIdleListenHndlr");
		
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveIdleListenHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveIdleListenHndlr"));
			
			if((pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band == TUN_BAND_FM) && ((pst_me_amfm_tuner_ctrl_inst->e_Market != AMFM_TUNER_CTRL_ASIA_CHINA) && (pst_me_amfm_tuner_ctrl_inst->e_Market != AMFM_TUNER_CTRL_JAPAN)))
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_LISTEN_RDS_START_REQID);
			}
			else 
			{
				//AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_NOTIFICATION);
			}
		}
		break;
	   case AMFM_TUNER_CTRL_NOTIFICATION:
	   {
		    /*Clear the timer id after timer expires */
		   	st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDSFMAM_TunerStatus_Timerid = 0;
			SOC_CMD_GET_QUALITY(&st_amfm_tuner_ctrl_Quality, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
			AMFM_Tuner_Ctrl_GetListenQuality(&st_amfm_tuner_ctrl_Quality, pst_me_amfm_tuner_ctrl_inst);
	   }
	   break;
	   case AMFM_TUNER_CTRL_LISTEN_RDS_START_REQID:
	   {
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER, AMFM_TUNER_CTRL_TUNER_STATUS_NOTIFICATION, u32_SOC_Frequency);
	   }
	   break;
	   
	   case QUALITY_NOTIFICATION_MSGID:
	   {
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(st_amfm_tuner_ctrl_Quality), (Tchar *)(pst_msg->data), (Tu16)(sizeof(st_amfm_tuner_ctrl_Quality)), &slot);

		    if (pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_FM)
			{
				AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(pst_me_amfm_tuner_ctrl_inst, &st_amfm_tuner_ctrl_Quality);

				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = TUN_BAND_FM;
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
				AMFM_Tuner_Ctrl_Current_Station_Qual_Update(&st_amfm_tuner_ctrl_Quality, pst_me_amfm_tuner_ctrl_inst);
				AMFM_Tuner_Ctrl_Notify_CurrQual(pst_me_amfm_tuner_ctrl_inst->st_interpolation);
			}
			else if (pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_AM_MW)
			{
				AMFM_Tuner_Ctrl_AM_QualParameters_Conversion(pst_me_amfm_tuner_ctrl_inst, &st_amfm_tuner_ctrl_Quality);

				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = TUN_BAND_AM;
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
				AMFM_Tuner_Ctrl_Current_Station_Qual_Update(&st_amfm_tuner_ctrl_Quality, pst_me_amfm_tuner_ctrl_inst);
				AMFM_Tuner_Ctrl_NotifyTunerStatus((pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));
			}
			else
			{
				/*for MISRA */
			}
       }
       break;
	   
	   case AMFM_TUNER_CTRL_TUNER_STATUS_NOTIFICATION:
	   { 
			/*After expires the timer clear the timerid */
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_RDS_Timer_id = 0;
			/* Reading RDS parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&st_recv_data, (Tchar *)(pst_msg->data), (Tu16)(sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data)), &slot);

			AMFM_Tuner_Ctrl_Update_Rcvd_RDS_Data(pst_me_amfm_tuner_ctrl_inst, st_recv_data);
			
		   if (st_recv_data.u32_TunedFrequency == pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq )
		   {	   
				PS_CMP = strncmp((const char*)pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_ps,(const char*)st_CurrentStationInfo.au8_ps,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_ps));

				RT_Flag = strncmp((const char*)pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_RadioText,(const char*)st_CurrentStationInfo.au8_RadioText,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_RadioText));
				if(st_CurrentStationInfo.u16_pi != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi)
				{
					b_PI_CMP = TRUE;
					pst_me_amfm_tuner_ctrl_inst->b_PI_Updated = TRUE;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG,"[RADIO][AMFM_TC] :FG Current Station PI=%x",pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi);
				}
				else
				{
					b_PI_CMP = FALSE;
				}
				if (st_CurrentStationInfo.u8_PTYCode != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_PTYCode)
				{
					b_PTY_CMP = TRUE;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_TC] :FG Current Station PI=%x", pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi);
				}
				else
				{
					b_PTY_CMP = FALSE;
				}
				if ((st_CurrentStationInfo.u8_TA != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_TA) || (st_CurrentStationInfo.u8_TP != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_TP))
				{
					b_TATP_CMP = TRUE;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_TC] :FG Current Station PI=%x", pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi);
				}
				else
				{
					b_TATP_CMP = FALSE;
				}
				
				for (loop_index = 0 ;loop_index < pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumAFeqList ; loop_index++)
				{
				    if(	st_CurrentStationInfo.u32_AFeqList[loop_index] !=  pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList[loop_index])
					{
						b_AF_Flag =  TRUE;
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.b_AF_Checkbit = TRUE;
					}
				}
				
				if((st_CurrentStationInfo.st_CT_Info.u8_Hour != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Hour) ||
					(st_CurrentStationInfo.st_CT_Info.u8_Min != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Min)	||
					(st_CurrentStationInfo.st_CT_Info.u8_Localtime_offset != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Localtime_offset)  ||
					(st_CurrentStationInfo.st_CT_Info.u8_offset_sign != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_offset_sign)	||
					(st_CurrentStationInfo.st_CT_Info.u8_Day != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Day)	||
					(st_CurrentStationInfo.st_CT_Info.u8_Month != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Month) ||
					(st_CurrentStationInfo.st_CT_Info.u16_Year != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u16_Year))
					
					{
						b_CT_CMP = TRUE;
					}
				
				
				/* Compare function */
				if ((PS_CMP != 0) || (b_AF_Flag != FALSE) || (RT_Flag != 0) || (b_PI_CMP != FALSE) || (b_CT_CMP != FALSE) || (b_PTY_CMP != FALSE) || (b_TATP_CMP != FALSE))
				{
					SYS_RADIO_MEMCPY(&(st_CurrentStationInfo),&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),sizeof(st_CurrentStationInfo));
								
					AMFM_Tuner_Ctrl_NotifyTunerStatus((pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));

					memset(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList,0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList));
	
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.b_AF_Checkbit = FALSE;
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumAFeqList = 0;
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumofAF_SamePgm = 0;
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumofAF_RgnlPgm = 0;
					/* Clearing  PI/CT/AF/PTY comparison flage */
					b_PI_CMP = FALSE;
					b_CT_CMP = FALSE;
					b_AF_Flag = FALSE; 
					b_PTY_CMP = FALSE;
					b_TATP_CMP  = FALSE;
				}
				else
				{
					/*Misra C rule */
				}
		   }
		   else
		   {
			   b_PI_CMP = FALSE;
		   }
		}
		break;
		case AMFM_TUNER_CTRL_FMAMTUNER_STATUS_NOTIFICATION:
		{
			/* Send Notofication to APP*/
			AMFM_Tuner_Ctrl_NotifyTunerStatus((pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));
				
		    st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDSFMAM_TunerStatus_Timerid = SYS_TIMER_START(AMFM_TUNER_CTRL_FMAM_TUNER_STATUS_NOTIFY_TIME,AMFM_TUNER_CTRL_NOTIFICATION, RADIO_AM_FM_TUNER_CTRL);
			if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDSFMAM_TunerStatus_Timerid == 0)
			{
				 RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : Listen Quality Read start timer is failed ");	
			}
			else
			{
				 	/* MISRA C */
			}
		
		}			
		break;
		case HSM_MSGID_EXIT:
		{
			/*  free all timers */
		    AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);	
			
			if((pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band == TUN_BAND_FM) && ((pst_me_amfm_tuner_ctrl_inst->e_Market != AMFM_TUNER_CTRL_ASIA_CHINA) && (pst_me_amfm_tuner_ctrl_inst->e_Market != AMFM_TUNER_CTRL_JAPAN)))
			{
			
			}
			else 
			{
				if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDSFMAM_TunerStatus_Timerid > 0)
				{
					if(SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDSFMAM_TunerStatus_Timerid) != TRUE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : FG NonRDSFM_AM tuner station notification timer is failed ");
					}
					else
					{
						st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDSFMAM_TunerStatus_Timerid = 0;
					}
				}	
				else
				{
				/*Nothing*/
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyHndlr                                     */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret 				= NULL; 
	Te_RADIO_ReplyStatus    				e_CancelReplyStatus     = REPLYSTATUS_FAILURE;
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{
			
		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveBusyHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveBusyHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveBusyHndlr"));

		}
		break;
		case AMFM_TUNER_CTRL_CANCEL_REQID:
		{
			if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id > 0)
			{
				if (SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : FG Seek/Scan Qual detect timer stop  is failed for cancel request ");
				}
				else
				{
					st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
				}
			}
			else
			{
				/*Nothing*/
			}
			/*************Stop the seek requested Api' for canceling the seek **********/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{				
				e_CancelReplyStatus = REPLYSTATUS_SUCCESS;				
			}
			else
			{
				pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					e_CancelReplyStatus = REPLYSTATUS_SUCCESS;					
				}
				else
				{
					e_CancelReplyStatus = REPLYSTATUS_FAILURE;					
				}				
			}

			/* Update the Band To Send exact TunerStatus Notification */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band;
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
			AMFM_Tuner_Ctrl_Response_Cancel(e_CancelReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_state);
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusySTLScanHndlr      */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusySTLScanHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Tu32 slot											= 0;
	Ts_Sys_Msg* pst_ret 								= NULL; 
    Te_RADIO_ReplyStatus e_GetStationListReplyStatus	= REPLYSTATUS_REQ_CANCELLED;
	Te_RADIO_SharedMemoryType e_SharedMemoryType 		= SHAREDMEMORY_INVALID;
	Te_RADIO_ScanType e_ScanType						= RADIO_SCAN_FG;
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00,sizeof(st_recv_data));
			
			/* Assign  PI_Updated to  E_FALSE ,Scan is Requested In the Response the new station will have New PI */
			pst_me_amfm_tuner_ctrl_inst->b_PI_Updated = FALSE;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveBusySTLScanHndlr");
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveBusySTLScanHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveBusySTLScanHndlr"));	
	 		memset(ast_Scaninfo,0x00,sizeof(ast_Scaninfo));
			scan_index_AM = 0;
			scan_index_FM = 0;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_TC] AMFM TC Mutex Lock  ActiveBusySTLScanHndlr Start Msg");
			Sys_Mutex_Lock(STL_AMFM_APP_AMFM_TC);
			pst_me_amfm_tuner_ctrl_inst->b_Mutex_resource = FALSE;
			
			/* Request to AMFM tuner control soc to scan */
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_START(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM scan start success");
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID);
			}
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM Scan start failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);

			switch (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_IN_PROGRESS:
				case ETAL_RET_ERROR:
				{
					if(pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						/*Requesting Again for Scan Start*/
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_START(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

						if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM scan start success");
						}
						else
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID);	
						}
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL AMFM scan start failed,status is  %d ", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
					}
				}
				break;
				default:
				{
					/*Nothing to do*/
				}
				break;
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_INFO_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL AMFM scan callback,status is  %d ", st_amfm_tuner_ctrl_scan_status.m_status);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(st_amfm_tuner_ctrl_scan_status), (Tchar *)pst_msg->data, (Tu8)(sizeof(st_amfm_tuner_ctrl_scan_status)), &slot);

			if (st_amfm_tuner_ctrl_scan_status.m_status == ETAL_SEEK_STARTED)
			{
				//do nothing
			}
			else if (st_amfm_tuner_ctrl_scan_status.m_status == ETAL_SEEK_RESULT)
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_RESULT_RESID);

				/* Notify Current frequency to HMI */
				AMFM_Tuner_Ctrl_Notify_CurrFrequency(st_amfm_tuner_ctrl_scan_status.m_frequency);
			}
			else if (st_amfm_tuner_ctrl_scan_status.m_status == ETAL_SEEK_FINISHED)
			{
				//do nothing
			}
			else
			{
				//do nothing
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_RESULT_RESID:
		{
			if (st_amfm_tuner_ctrl_scan_status.m_fullCycleReached == TRUE)
			{
				if (st_amfm_tuner_ctrl_scan_status.m_frequencyFound == TRUE)
				{
					pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_scan_status.m_frequency;
					//FM Band
					if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
					{
						AMFM_Tuner_Ctrl_QualUpdate_FM_Scan(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
					}
					//AM Band
					else if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW || pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW)
					{
						AMFM_Tuner_Ctrl_QualUpdate_AM_Scan(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
					}
					else
					{
						//do nothing
					}
				}
				else
				{
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
				}
			}
			else if (st_amfm_tuner_ctrl_scan_status.m_frequencyFound == TRUE)
			{
				pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_scan_status.m_frequency;
				//FM Band
				if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
				{
					AMFM_Tuner_Ctrl_QualUpdate_FM_Scan(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
				}
				//AM Band
				else if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW || pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW)
				{
					AMFM_Tuner_Ctrl_QualUpdate_AM_Scan(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
				}
				else
				{
					//do nothing
				}
			}
			else
			{
				/*For MISRA C*/
			}
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_RDS_START_REQID:
		{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER, AMFM_TUNER_CTRL_UPDATE_RDS_IN_FM_STL, pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq);
		}
		break;
		
		case AMFM_TUNER_CTRL_UPDATE_RDS_IN_FM_STL:
		{
			/* Reading RDS parameters from the message */
            AMFM_Tuner_ctrl_ExtractParameterFromMessage(&st_recv_data,(Tchar *)(pst_msg->data),(Tu16)(sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data)),&slot);	
			
			if (st_recv_data.u32_TunedFrequency == pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] : Foreground FM scan is in progress, Frequency is %d , Scan index is %d",st_recv_data.u32_TunedFrequency,scan_index_FM);	
				if(st_recv_data.b_PI_Update != FALSE)
				{
					ast_Scaninfo[scan_index_FM].u16_pi  = st_recv_data.u16_PI_Code;
				}
				else
				{
					//do nothing
				}

				if((st_recv_data.b_PSN_Update == TRUE) || (st_recv_data.b_NonRDS_Station == TRUE))
				{
					/* Copying PSN to FM STL */
					SYS_RADIO_MEMCPY((void *)ast_Scaninfo[scan_index_FM].au8_ps,(const void *)st_recv_data.st_FM_RDS_PSN.u8_ps,sizeof(st_recv_data.st_FM_RDS_PSN.u8_ps));
					
					/* Incrementing STL index */
					scan_index_FM = (Tu8)(scan_index_FM + (Tu8)1);

					if (scan_index_FM >= AMFM_TUNER_CTRL_MAX_STATIONS)
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
					}
					else
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID);
					}
				}
				else
				{
					/*Added for MISRA C*/
				}
			}
			else
			{
				/* Nothing to do.We got RDS data of some other frequency */
			}
		}
		break;
		
		case AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID:
		{
			/* reset the timer variable */
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
			/* has to change the condition after confirmation of full cycle reached from etal*/
			if (pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq != pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq)
			{
				/* request to continue scan*/
				pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_CONTINUE(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);


				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM Scan continue success");
				}
				else
				{
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID);
				}
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
			}		
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM Scan continue failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
			switch (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_IN_PROGRESS:
				case ETAL_RET_ERROR:
				{
					if (pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						/*Requesting to continue the scan again */
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_CONTINUE(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

						if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM Scan continue success");
						}
						else
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID);
						}
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM Scan continue failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
					}
				}
				break;
				default:
				{
						   /*Nothing to do*/
				}
				break;
			}
		}
		break;

		case AM_FM_TUNER_CTRL_SOC_AMSCAN_CONTINUE_MSGID:
		{	
			/* reset the timer variable */
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
			if (pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq != pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq)
			{
				/*As of now considered only AM_MW band, if LW aslo has to be considered then this has to be revisited

				if (pst_me_amfm_tuner_ctrl_inst->u32_Scan_start_freq == AMFM_TUNER_CTRL_AM_LW_ENDFREQ)
				{
					SOC_CMD_SCAN_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq = AMFM_TUNER_CTRL_AM_MW_STARTFREQ;
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize = MW_WEU_STEP;
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band = TUN_BAND_AM_MW;
					SOC_CMD_SCAN_CONTINUE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
				}
				else
				{
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq = (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq + pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize);
					SOC_CMD_SCAN_CONTINUE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
				}
			}
			else 
			{
				pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq = (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq - pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize);
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_SCAN_DONE_RESID);
			}*/

				/* request to continue scan*/
				pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_CONTINUE(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM scan continue success");
				}
				else
				{
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID);
				}
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
			}
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_DONE_RESID:
		{
			Tu8 u8_temp = 0;
			pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;

			/* reset the timer variable */
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;

		   /* sending respone to AMFM application layer */
		   Sys_Mutex_Unlock(STL_AMFM_APP_AMFM_TC);
		   pst_me_amfm_tuner_ctrl_inst->b_Mutex_resource = TRUE;
		   RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_TC] AMFM TC Mutex UnLock  ActiveBusySTLScanHndlr AMFM_TUNER_CTRL_SCAN_DONE_RESID");
		   if(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
		   {
			   u8_temp = scan_index_FM;  
		   }	   
		   else if((pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW) || (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW))
		   {
			   u8_temp = scan_index_AM;   
		   }
		   else
		   {
			   //do nothing
		   }
		   if(u8_temp != (Tu8)0)
			{
			   e_GetStationListReplyStatus = REPLYSTATUS_SUCCESS;

			   if(u8_temp == scan_index_FM)
			   {
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] : FM Scan End and Station list is notified to Application layer"); 
					e_SharedMemoryType = FM_TUNER_APP;
			   }
			   else if(u8_temp == scan_index_AM)
			   {
				   RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] : AM Scan End and Station list is notified to Application layer"); 
				   e_SharedMemoryType = AM_TUNER_APP;
			   }
			   else
			   {
				   //do nothing
			   }
			   
			}
			else 
			{
				e_GetStationListReplyStatus = REPLYSTATUS_EMPTY;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] : Scan End and Empty Station list is notified to Application layer"); 
			}
		   /*sending scan response to AMFM Application layer*/
			AMFM_Tuner_Ctrl_Response_GetStationList(e_GetStationListReplyStatus);
			/*notifying the updated station list to AMFM Application layer*/
			AMFM_Tuner_Ctrl_Notify_STLUpdated(e_SharedMemoryType);

			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_state);
		}
		break;

		case HSM_MSGID_EXIT:
		{
			/* Releasing the Mutex  if any cancel of Ongoing SCAN*/
			if( pst_me_amfm_tuner_ctrl_inst->b_Mutex_resource != TRUE)
			{
				Sys_Mutex_Unlock(STL_AMFM_APP_AMFM_TC);
				pst_me_amfm_tuner_ctrl_inst->b_Mutex_resource = FALSE;
			}
			else
			{
				/* MISRA */
			}
		   if(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
		   {
			   AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER, STOP_RDS_MSG_ID, pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq);
			}
		   else
		   {
			   //do nothing
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyTuneHndlr         */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveBusyTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 						pst_ret    			= NULL;  /* mark the message as handled */
	Te_RADIO_ReplyStatus			 	e_TuneReplyStatus 	= REPLYSTATUS_FAILURE;

	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00,sizeof(st_recv_data));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveBusyTuneHndlr");
		
			/* Updating the current state name  used for debuging purpose */ 
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveBusyTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveBusyTuneHndlr"));

			memset(&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));
			/* Assign  PI_Updated to  E_FALSE ,Tune is Requested In the Response the new station will have New PI */
			pst_me_amfm_tuner_ctrl_inst->b_PI_Updated = FALSE;
			pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage =FALSE;
			pst_me_amfm_tuner_ctrl_inst->b_EON_Flage  =FALSE;
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;

			/* Notifying requested frequency */
		    AMFM_Tuner_Ctrl_Notify_CurrFrequency(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq);

			/* Request to AMFM tuner control soc to tune */
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
             if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
             {
				 st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = SYS_TIMER_START(AMFM_TUNER_CTRL_MAX_TUNE_QUAL_READ_TIME, AMFM_TUNER_CTRL_QUAL_READ, RADIO_AM_FM_TUNER_CTRL);
		        if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id == 0)
		        {
			         RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : FM tune qual read timer is failed ");	
		        }
		        else
		        {
			         /* MISRA C */
		        }
             }
             else
             {
                 AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_FAIL);
             }
        }
		break;
		     
		case AMFM_TUNER_CTRL_TUNE_FAIL :
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : ETAL FM tune failed,status is  %d ",pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);	
			/*clearing the timer id*/
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
         
			switch(pst_me_amfm_tuner_ctrl_inst -> AMFM_Tuner_Ctrl_ETAL_Status)
			{
			case ETAL_RET_ERROR       :
			//case ETAL_RET_NO_DATA     :
			case ETAL_RET_IN_PROGRESS :
			{
				if(pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
				{
					pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;  
					pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
					if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
					{
						st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = SYS_TIMER_START(AMFM_TUNER_CTRL_MAX_TUNE_QUAL_READ_TIME, AMFM_TUNER_CTRL_QUAL_READ, RADIO_AM_FM_TUNER_CTRL);
						if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id == 0)
						{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : FM tune qual read timer is failed ");	
						}
						else
						{
								/* MISRA C */
						}
					}
					else
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_FAIL);
					}
				}
				else
				{
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_FAILURE;
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_DONE_RESID);
				}
			}
			break;
			default :
			{
				/*Nothing */
			}
			break;

			}
		}
		break;
		case AMFM_TUNER_CTRL_QUAL_READ :
        {
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
        
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_GET_QUALITY(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);

			if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				if(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_FM)
				{
					AMFM_Tuner_Ctrl_GetFMQualityCheck_Tune(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst);
				}
				else
				{
					AMFM_Tuner_Ctrl_GetAMQualityCheck_Tune(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst);
				}
			}
			else
			{
			AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_QUAL_READ_FAIL);
			}

		}
		break;
		
		case AMFM_TUNER_CTRL_QUAL_READ_FAIL :
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : ETAL FM quality failed,status is  %d ",pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);	
         
			switch(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_ERROR       :
				case ETAL_RET_NO_DATA     :
				case ETAL_RET_IN_PROGRESS :
				{
					if(pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_QualRecall_Count < AMFM_TUNER_CTRL_MAX_QUALREAD_REPEAT_COUNT)
					{
						pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_QualRecall_Count++;  
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_GET_QUALITY(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
						if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							if(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_FM)
							{
								AMFM_Tuner_Ctrl_GetFMQualityCheck_Tune(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst);
							}
							else
							{
								AMFM_Tuner_Ctrl_GetAMQualityCheck_Tune(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst);
							}
						}
						else
						{
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_QUAL_READ_FAIL);
						}
					}
					else
					{
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band;
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_FAILURE;
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_DONE_RESID);
					}
				}
				break;
				default :
				{
				/*Nothing */
				}
				break;

			}
		}
		break;
        case AMFM_TUNER_CTRL_TUNE_DONE_RESID: 
		{
			pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;
            pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_QualRecall_Count = 0;
			if(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status == REPLYSTATUS_SUCCESS)
			{
				e_TuneReplyStatus = REPLYSTATUS_SUCCESS;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : FG Tune Success ");
            	/* sending Succesful respone to AMFM application layer */
				SYS_RADIO_MEMCPY(&(st_CurrentStationInfo),&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),sizeof(st_CurrentStationInfo));				
				
				AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
			}
			else
			{
				e_TuneReplyStatus = REPLYSTATUS_NO_SIGNAL;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : FG Tune failed");
				/* sending Failure respone to AMFM application layer */
				AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
				
			}
			
			/* Transiting to idle Listen */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_listen_state);
		}
		break;
		case HSM_MSGID_EXIT:
		{
 			if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id > 0)
			{
				if(SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : FG_Tune stop timer is failed ");
				}
				else
				{
					st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleAFUpdateTuneHndlr         */

/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveIdleAFUpdateTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 								pst_ret    				= NULL;  /* mark the message as handled */
	Te_RADIO_ReplyStatus					 	e_AF_UpdateReplyStatus 	= REPLYSTATUS_FAILURE;
	
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveIdleAFUpdateTuneHndlr");

			/* Updating the current state name  used for debuging purpose */ 
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveIdleAFUpdateTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveIdleAFUpdateTuneHndlr"));
		
		} 
		break;
		
		case STOP_DONE_MSG_ID:
		{
			memset(&(AMFMTuner_Tune_AFQuality),0x00,sizeof(AMFMTuner_Tune_AFQuality));
			memset(&(pst_me_amfm_tuner_ctrl_inst->st_interpolation),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_interpolation));
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->u32_freq;
			/* Request to AMFM tuner control soc to tune */
			SOC_CMD_AF_UPDATE(pst_me_amfm_tuner_ctrl_inst->u32_freq);				
		}
		break;
		
        case AMFM_TUNER_CTRL_AF_QUALITY_DONE_RESID: 
		{			
			/*Clear the timer id after timer expires */
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;		
			SOC_CMD_READ_AF_UPDATE(&AMFMTuner_Tune_AFQuality);
		}
		break;
		
        case AMFM_TUNER_CTRL_FM_AF_UPDATE_QUALITY_DONE_RESID: 
		{
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.s32_BBFieldStrength  	= 	AMFM_FS_CONVERSION(AMFMTuner_Tune_AFQuality.s32_BBFieldStrength);
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_UltrasonicNoise 	= 	AMFM_USN_CONVERSION(AMFMTuner_Tune_AFQuality.u32_UltrasonicNoise);
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_Multipath 	= 	AMFM_WAM_CONVERSION(AMFMTuner_Tune_AFQuality.u32_Multipath);
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_FrequencyOffset 	= 	AMFM_OFS_CONVERSION(AMFMTuner_Tune_AFQuality.u32_FrequencyOffset);
			
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation = QUAL_CalcFmQual(pst_me_amfm_tuner_ctrl_inst->st_interpolation.s32_BBFieldStrength, pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_UltrasonicNoise, pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_Multipath,NULL ,NULL, pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_FrequencyOffset );
			
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  TUN_BAND_FM;
			e_AF_UpdateReplyStatus = REPLYSTATUS_SUCCESS;
			u32_SOC_Frequency  = pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq;
			AMFM_Tuner_Ctrl_Response_AF_Update(e_AF_UpdateReplyStatus,pst_me_amfm_tuner_ctrl_inst->u32_freq,pst_me_amfm_tuner_ctrl_inst->st_interpolation);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_listen_state);

		}
		break;
		case HSM_MSGID_EXIT:
		{

		}
		break;
		default:
		{
			if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id > 0)
			{
				if(SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : FG_AF_Update qual detect started timer stop is failed ");
				}
				else
				{
					st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
				}
			}	
			pst_ret = pst_msg;
		}
		break;
	}
	return pst_ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFMCheckTuneHndlr         */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFMCheckTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret    				= NULL;  /* mark the message as handled */
	Te_RADIO_ReplyStatus				 	e_AF_CheckReplyStatus 	= REPLYSTATUS_FAILURE;
	
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00, sizeof(st_recv_data));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFMCheckTuneHndlr");
	
			/* Updating the current state name  used for debuging purpose */ 
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFMCheckTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFMCheckTuneHndlr"));

			/* Request to AMFM tuner control soc to tune */
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->u32_freq;

			SOC_CMD_FM_CHECK(pst_me_amfm_tuner_ctrl_inst->u32_freq);			
		} 
		break;
	
		case AMFM_TUNER_CTRL_RDS_DONE_RESID:
		{
			if(pst_me_amfm_tuner_ctrl_inst->b_PICheckFlage ==  TRUE)
			{
				AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,AMFM_TUNER_CTRL_LOWSIGNAL_FMCHECK_DONE_RESID ,pst_me_amfm_tuner_ctrl_inst->u32_freq);	
			}
			else
			{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,AMFM_TUNER_CTRL_FMCHECK_DONE_RESID ,pst_me_amfm_tuner_ctrl_inst->u32_freq);
			}
		}
		break;
		case AMFM_TUNER_CTRL_LOWSIGNAL_FMCHECK_DONE_RESID:
	    case AMFM_TUNER_CTRL_FMCHECK_DONE_RESID: 
		{
			/*clear the RDS timer id after expires*/
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_RDS_Timer_id = 0;
			/* Reading RDS parameters from the message */
			SYS_RADIO_MEMCPY(&st_recv_data,pst_msg->data,sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data) );
			/* Update the Band To Send exact TunerStatus Notification */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  TUN_BAND_FM;
			
			if(pst_me_amfm_tuner_ctrl_inst->u32_freq == st_recv_data.u32_TunedFrequency)
			{
				if((st_recv_data.u16_PI_Code != (Tu16)0) || (st_recv_data.b_NonRDS_Station == TRUE))
				{	
					e_AF_CheckReplyStatus = REPLYSTATUS_SUCCESS;
			
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] :FG Check_Succes and PI = %x\n",st_recv_data.u16_PI_Code);
					if(pst_me_amfm_tuner_ctrl_inst->b_PICheckFlage ==  TRUE)
					{
        				/* sending respone to AMFM application layer */
						AMFM_Tuner_Ctrl_Response_LowSignal_FM_Check(e_AF_CheckReplyStatus,st_recv_data.u16_PI_Code);
						/*Clearing the flage FM_Check for Low Signal Condition  for Next Operation*/
						pst_me_amfm_tuner_ctrl_inst->b_PICheckFlage =  FALSE;
					}
					else
					{
						/* sending respone to AMFM application layer */
						AMFM_Tuner_Ctrl_Response_FM_Check(e_AF_CheckReplyStatus,st_recv_data.u16_PI_Code);
					}
			
					/* Transit to AMFM tuner control instance hsm active idle listen handler */
					HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_listen_state);
				}
			
			}
			else
			{
				/* Nothing to do. We got RDs data of some other frequency*/
			}	
		}
		break;
		
		case HSM_MSGID_EXIT:
		{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);	
			
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFmJumpTuneHndlr   */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFmJumpTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 						pst_ret    			= NULL;  /* mark the message as handled */
	Te_RADIO_ReplyStatus				e_TuneReplyStatus 	= REPLYSTATUS_FAILURE;
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFmJumpTuneHndlr");
			
			/* Updating the current state name  used for debuging purpose */ 
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFmJumpTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveIdleFmJumpTuneHndlr"));

			memset(&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));
			/* Assign  PI_Updated to  FALSE ,JUMP is Requested In the Response the new station will have New PI */
			pst_me_amfm_tuner_ctrl_inst->b_PI_Updated = FALSE;
			pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage =FALSE;
			pst_me_amfm_tuner_ctrl_inst->b_EON_Flage  =FALSE;
			 /* Request to AMFM tuner control soc to tune */
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
            
			SOC_CMD_FM_JUMP(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq);					
		} 
		break;
			 
		case AMFM_TUNER_CTRL_FM_QUALITY_DONE_RESID:
		{
			AMFM_Tuner_Ctrl_Notify_CurrFrequency(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq);
						
			if(u8_count < AMFM_TUNER_CTRL_JUMPQUAL_AVGCOUNT)
			{							
				SOC_CMD_GET_QUALITY(&St_Tuner_FMQual_parametrs, pst_me_amfm_tuner_ctrl_inst->e_Band);
				
				AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(pst_me_amfm_tuner_ctrl_inst, &St_Tuner_FMQual_parametrs);
			
				u32_interqual = u32_interqual + pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation;
				
				u8_count = (Tu8)(u8_count + (Tu8)1);
				
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_FM_QUALITY_DONE_RESID);
			}
			else
			{
				pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation = (u32_interqual/(Tu32)5);
				u8_count = 0;
				u32_interqual =0;
				AMFM_Tuner_Ctrl_GetjumpQualityCheck(&St_Tuner_FMQual_parametrs, pst_me_amfm_tuner_ctrl_inst);
			}
		
		}
		break;
	
	    case AMFM_TUNER_CTRL_TUNE_DONE_RESID: 
		{
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = TUN_BAND_FM;
			
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq 	= 	pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
			
			e_TuneReplyStatus = REPLYSTATUS_SUCCESS;
            /* sending respone to AMFM application layer */
			SYS_RADIO_MEMCPY(&(st_CurrentStationInfo),&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),sizeof(st_CurrentStationInfo));				
			
			/* sending Succesful respone to AMFM application layer */
			AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);	
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] FG FM_Jump is success");
		
			/* Transiting to Active idle listen state */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_listen_state);

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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusySeekUpDownHndlr   */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusySeekUpDownHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret						= NULL;
	Te_RADIO_ReplyStatus			 		e_SeekReplyStatus			= REPLYSTATUS_FAILURE;
	Te_RADIO_SeekType                       e_SeekType                  = RADIO_SEEK;
	Tu32 									slot                        = 0;

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveBusySeekUpDownHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveBusySeekUpDownHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveBusySeekUpDownHndlr"));	
			
	    	memset(&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));

			/* Assign  PI_Updated to  E_FALSE ,Seek is Requested In the Response the new station will have New PI */
			pst_me_amfm_tuner_ctrl_inst->b_PI_Updated = FALSE;
			pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage = FALSE;
			pst_me_amfm_tuner_ctrl_inst->b_EON_Flage  = FALSE;

			/*Requesting to soc to seek start*/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_START(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize,
				pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band, e_SeekType);
				
			if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL AMFM auto seek start success");
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_START_FAIL);
			}
        }
		break;
		case AMFM_TUNER_CTRL_SEEK_START_FAIL:
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : ETAL AMFM seek start is failed,status is  %d ",pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);	
				switch(pst_me_amfm_tuner_ctrl_inst -> AMFM_Tuner_Ctrl_ETAL_Status)
				{
					case ETAL_RET_ERROR       :
					case ETAL_RET_NO_DATA     :
					case ETAL_RET_IN_PROGRESS :
					{
						if(pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;  

							/*Calling SOC command for requesting seek start*/
							pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_START(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize,
								pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band, e_SeekType);
							if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
							{   
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL AMFM seek start success");
							}
							else
							{
								AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_START_FAIL);
							}
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL AMFM seek start is failed,status is  %d ", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
							pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus = REPLYSTATUS_FAILURE;
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_DONE_RESID);
						}
					}
					break;
					default:
					{
						/* For MISRA */
					}
					break;
				}
			}
			break;

		case AMFM_TUNER_CTRL_SEEK_INFO_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL AMFM seek callback,status is  %d ", st_amfm_tuner_ctrl_seek_status.m_status);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(st_amfm_tuner_ctrl_seek_status), (Tchar *)pst_msg->data, (Tu8)(sizeof(st_amfm_tuner_ctrl_seek_status)), &slot);
			
			if (st_amfm_tuner_ctrl_seek_status.m_status == ETAL_SEEK_STARTED)
			{
				//do nothing
			}
			else if (st_amfm_tuner_ctrl_seek_status.m_status == ETAL_SEEK_RESULT)
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_RESULT_RESID);

				/* Notify Current frequency to HMI */
				AMFM_Tuner_Ctrl_Notify_CurrFrequency(st_amfm_tuner_ctrl_seek_status.m_frequency);
			}
			else if (st_amfm_tuner_ctrl_seek_status.m_status == ETAL_SEEK_FINISHED)
			{
				//do nothing
			}
			else
			{
				//do nothing
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_RESULT_RESID:
		{
			if (st_amfm_tuner_ctrl_seek_status.m_fullCycleReached == TRUE)
			{
				pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_seek_status.m_frequency;
				pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus = REPLYSTATUS_FAILURE;
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_DONE_RESID);
			}
			else if (st_amfm_tuner_ctrl_seek_status.m_frequencyFound == TRUE)
			{
				pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_seek_status.m_frequency;

				/* Notifying AMFM Application component about frequency change */
				AMFM_Tuner_Ctrl_Notify_CurrFrequency(pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq);

				pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus = REPLYSTATUS_SUCCESS;

				if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
				{
					AMFM_Tuner_Ctrl_QualUpdate_FG_FM_Seek(&st_amfm_tuner_ctrl_seek_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
				}
				else
				{
					AMFM_Tuner_Ctrl_QualUpdate_FG_AM_Seek(&st_amfm_tuner_ctrl_seek_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
				}
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_DONE_RESID);
			}
			else
			{
				//do nothing
			}
		}
		break;
			
		case AMFM_TUNER_CTRL_SEEK_DONE_RESID:
		{
			pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;
          
            if(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
			{
				/* Update the Band To Send exact TunerStatus Notification */
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band;
			}
			else if((pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW) || (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW))
			{
				/* Update the Band To Send exact TunerStatus Notification */
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band;
			}
			else
			{
				/* Do nothing */
			}
			
			/* updating the seek freq as current tuned freq */			
			
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
			
			if( pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus 	==  REPLYSTATUS_SUCCESS)
			{
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_SUCCESS;
				e_SeekReplyStatus = REPLYSTATUS_SUCCESS;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : FG Seek operation is success Frequency is = %d\n",pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq);
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
				pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
			}
			else
			{
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_FAILURE;
				e_SeekReplyStatus = REPLYSTATUS_NO_SIGNAL;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_TC] : Seek operation is failed");
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
			}
			/* Sending response to AMFM Application Component */
			AMFM_Tuner_Ctrl_Response_SeekUpDown(e_SeekReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);

			/* Transit to AMFM tuner control instance hsm active idle listen handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_listen_state);	
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyPISeekHndlr       */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveBusyPISeekHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret						= NULL;
	Tu32 									slot 						= 0;
	Te_RADIO_ReplyStatus			 		e_SeekReplyStatus			= REPLYSTATUS_FAILURE;
	Te_RADIO_SeekType                       e_SeekType                  = RADIO_PI_SEEK;
	pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus                      = REPLYSTATUS_FAILURE;
	pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection 		= RADIO_FRMWK_DIRECTION_UP;
	pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band 				= TUN_BAND_FM;
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;

		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00,sizeof(st_recv_data));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveBusyPISeekHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveBusyPISeekHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveBusyPISeekHndlr"));	
			
			memset(&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));

			/* Assign  PI_Updated to  E_FALSE ,PI_Seek is Requested In the Response the new station will have New PI */
			pst_me_amfm_tuner_ctrl_inst->b_PI_Updated = FALSE;

			/*Requesting to soc for seek start*/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_START(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize,pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band, e_SeekType);
			
			if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL FM PIseek start success");
			}	       
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_START_FAIL);
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_START_FAIL:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : ETAL FM PIseek start is failed,status is  %d ",pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);	

			switch(pst_me_amfm_tuner_ctrl_inst -> AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_ERROR       :
				case ETAL_RET_NO_DATA     :
				case ETAL_RET_IN_PROGRESS :
				{
					if(pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;  
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_START(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection,pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize,pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band, e_SeekType);
						if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{   
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL FM PIseek start success");
						}
						else
						{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_START_FAIL);
						}
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL FM PIseek start is failed,status is  %d ", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
						pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus = REPLYSTATUS_FAILURE;
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_DONE_RESID);
					}
				}
				break;
				default:
				{
				/*For MISRA*/
				}
				break;
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_INFO_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL FM PIseek callback,status is  %d ", st_amfm_tuner_ctrl_seek_status.m_status);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(st_amfm_tuner_ctrl_seek_status), (Tchar *)pst_msg->data, (Tu8)(sizeof(st_amfm_tuner_ctrl_seek_status)), &slot);
			if (st_amfm_tuner_ctrl_seek_status.m_status == ETAL_SEEK_STARTED)
			{
				//do nothing
			}
			else if (st_amfm_tuner_ctrl_seek_status.m_status == ETAL_SEEK_RESULT)
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_RESULT_RESID);
			}
			else if (st_amfm_tuner_ctrl_seek_status.m_status == ETAL_SEEK_FINISHED)
			{
				//do nothing
			}
			else
			{
				//do nothing
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_RESULT_RESID:
		{
			if (st_amfm_tuner_ctrl_seek_status.m_fullCycleReached == TRUE)
			{
				pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus = REPLYSTATUS_FAILURE;
				pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_seek_status.m_frequency;
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_STOP_MSGID);
			}
			else if (st_amfm_tuner_ctrl_seek_status.m_frequencyFound == TRUE)
			{
				if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
				{
					pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_seek_status.m_frequency;
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_RDS_REQID);
				}
				else
				{
					//do nothing
				}
			}
			else
			{
				//do nothing
			}
		}
		break;

    	case AM_FM_TUNER_CTRL_SOC_SEEK_TUNE_NEXT_FREQ_REQ:
		{
			/*Requesting to seek continue to soc*/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_CONTINUE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize,
																			pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
			
			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL FM PIseek continue success");
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_CONTINUE_FAIL);
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_CONTINUE_FAIL:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL FM PIseek continue is failed,status is  %d ", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
			switch (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_ERROR:
				case ETAL_RET_NO_DATA:
				case ETAL_RET_IN_PROGRESS:
				{
					if (pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_CONTINUE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_SeekDirection, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize,
																					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
						
						if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{   
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : FM PIseek continue success");
						}
						else
						{
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_CONTINUE_FAIL);
						}
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL FM PIseek continue is failed,status is  %d ", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
						pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus = REPLYSTATUS_FAILURE;
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_STOP_MSGID);
					}
				}
				break;
				default:
				{
					/*For MISRA*/
				}
				break;
			}
		}
		break;

		case AMFM_TUNER_CTRL_RDS_REQID:
		{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER, AMFM_TUNER_CTRL_RDS_DONE_RESID, pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq);
		}
		break;

		case AMFM_TUNER_CTRL_RDS_DONE_RESID:
		{
			/* Reading RDS parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&st_recv_data, (Tchar *)(pst_msg->data), (Tu16)(sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data)), &slot);
			
			if (st_recv_data.u32_TunedFrequency == pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq)
			{
				if ((st_recv_data.u16_PI_Code != (Tu16)0) || (st_recv_data.b_NonRDS_Station == TRUE))
				{
					/* Updating PI value from RDS data to Current station info structure */
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi = st_recv_data.u16_PI_Code;

					if (pst_me_amfm_tuner_ctrl_inst->u16_pi == pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi)
					{
						SYS_RADIO_MEMCPY(&(st_CurrentStationInfo), &(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo), sizeof(st_CurrentStationInfo));

						/* Notifying AMFM Application component about frequency change */
						AMFM_Tuner_Ctrl_Notify_CurrFrequency(pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq);

						/* updating the etal seek freq into tuner control structure */
						pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;

						if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
						{
							AMFM_Tuner_Ctrl_QualUpdate_FG_FM_PISeek(&st_amfm_tuner_ctrl_seek_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
						}
						else
						{
							AMFM_Tuner_Ctrl_QualUpdate_FG_AM_Seek(&st_amfm_tuner_ctrl_seek_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
						}

						pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus = REPLYSTATUS_SUCCESS;

						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_STOP_MSGID);
					}
					else
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AM_FM_TUNER_CTRL_SOC_SEEK_TUNE_NEXT_FREQ_REQ);
					}
				}
				else
				{
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AM_FM_TUNER_CTRL_SOC_SEEK_TUNE_NEXT_FREQ_REQ);
				}
			}
			else
			{
				/* Nothing to do. We got RDS data of some other frequency*/
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_STOP_MSGID:
		{
			/*Requesting for Seek stop*/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				//do nothing
			}
			else
			{
				/*Requesting again for seek stop*/
				pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					//do nothing
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : learn stop failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
				}
			}
			AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_DONE_RESID);
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_DONE_RESID:
		{
			pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;
		
			/* updating the seek freq as current tuned freq for Listen State Check */
			pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
	
			/* Update the Band To Send exact TunerStatus Notification */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  TUN_BAND_FM;
			
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
			
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
			
			if (pst_me_amfm_tuner_ctrl_inst->e_SOC_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				e_SeekReplyStatus = REPLYSTATUS_SUCCESS;
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_SUCCESS;
			}
			else
			{
				e_SeekReplyStatus = REPLYSTATUS_NO_SIGNAL;
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_FAILURE;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_TC] : Foreground PI Seek operation is failed");				
			}

			/* Sending response to AMFM Application Component */
			AMFM_Tuner_Ctrl_Response_SeekUpDown(e_SeekReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);

			/* Transit to AMFM tuner control instance hsm active idle listen handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_listen_state);
			
		}	
		break;
		
		case HSM_MSGID_EXIT:
		{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);		
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepHndlr            */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 								pst_ret    				= NULL; /* mark the message as handled */
	Te_RADIO_ReplyStatus				 		e_ActivateReplyStatus 	= REPLYSTATUS_FAILURE;
	Te_RADIO_ReplyStatus    			    	e_CancelReplyStatus     = REPLYSTATUS_FAILURE;
	Tu32 										slot 					= 0;
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepHndlr"));
 
 
		}
		break;
		case AMFM_TUNER_CTRL_ACTIVATE_REQID:
		{
			/*Updating the Enum to Foreground */
			pst_me_amfm_tuner_ctrl_inst->e_Tuner_State = AMFM_TUNER_CTRL_FOREGROUND;
			e_audio_source_select = FALSE;
			e_ActivateReplyStatus = REPLYSTATUS_SUCCESS;
            /*sending response AMFM application layer*/
			AMFM_Tuner_Ctrl_Response_Activate(e_ActivateReplyStatus);
            /* Transit to AMFM tuner control instance hsm active idle handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_idle_state);
		}
		break;
		case AMFM_TUNER_CTRL_GETSTATIONLIST_REQID:
		{
            /* Reading scan request parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->Scantype), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->Scantype)), &slot);

			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNER_CONFIGURATION(pst_me_amfm_tuner_ctrl_inst->e_Market, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band, pst_me_amfm_tuner_ctrl_inst->e_Tuner_State);

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				if (pst_me_amfm_tuner_ctrl_inst->Scantype == AMFM_STARTUP_SCAN)
				{
					/* commented */
				}
				else
				{
					HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_scan_state);
				}
			}
			else
			{

			}
		}
		break;
		case AMFM_TUNER_CTRL_TUNE_REQID:
		{
			/* Reading tune requset parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band), (Tchar *)(pst_msg->data), (Tu16)(sizeof(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band)), &slot);
			/* Transit to AMFM tuner control instance hsm active busy tune handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_tune_state);
		}
		break;
		case AMFM_TUNER_CTRL_BGQUALITY_REQID:
		{
			/* Transit to AMFM tuner control instance hsm active sleep background quality handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_quality_state);
		}
		break;
		case AMFM_TUNER_CTRL_AF_UPDATE_REQID:
		{
			/* Reading tune requset parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->u32_freq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Tu32)), &slot);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_af_update_state);
		}
		break;
		case AMFM_TUNER_CTRL_AF_CHECK_REQID:
		{
			/* Reading tune requset parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->u32_freq), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Tu32)), &slot);

			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_fm_check_tune_state);
		}
		break;
		case AMFM_TUNER_CTRL_AF_JUMP_REQID:
		{
			/* Reading tune requset parameters from the message */
            AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq),(Tchar *)(pst_msg->data),(Tu16)(sizeof(Tu32)),&slot);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_fm_jump_tune_state);
		}
		break;
		case AMFM_TUNER_CTRL_CANCEL_REQID:
		{
			/* Releasing the Mutex  if any cancel of Ongoing SCAN*/
			Sys_Mutex_Unlock(STL_AMFM_APP_AMFM_TC);			
			if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id > 0)
			{
				if (SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : FG Seek/Scan Qual detect timer stop  is failed for cancel request ");
				}
				else
				{
					st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
				}
			}
			else
			{
				/*Nothing*/
			}

			/*************Stop the Scan requested Api' for canceling the scan **********/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				e_CancelReplyStatus = REPLYSTATUS_SUCCESS;
			}
			else
			{
				pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					e_CancelReplyStatus = REPLYSTATUS_SUCCESS;					
				}
				else
				{
					e_CancelReplyStatus = REPLYSTATUS_FAILURE;					
				}
			}
			AMFM_Tuner_Ctrl_Response_Cancel(e_CancelReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_afstop_state);
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFStopHndlr            */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFStopHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 					pst_ret		= NULL; /* mark the message as handeled */
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFStopHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFStopHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFStopHndlr"));			
		}
		break;
		case HSM_MSGID_EXIT:
		{
			/*  free all timers */
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);

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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgQualityHndlr            */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgQualityHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 					pst_ret		= NULL; /* mark the message as handeled */
	Tu32 							slot 		= 0;
	Tu8  							loop_index	= (Tu8)0;
	Ts32 							PS_CMP 		= 0;
	Tbool 						    b_AF_Flag 	= FALSE;
	Tbool							b_PI_CMP;


	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00,sizeof(st_recv_data));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgQualityHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgQualityHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgQualityHndlr"));
			AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_LISTEN_RDS_START_REQID);
		}
		break;
		case AMFM_TUNER_CTRL_LISTEN_RDS_START_REQID:
		{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,AMFM_TUNER_CTRL_TUNER_STATUS_NOTIFICATION ,u32_SOC_Frequency);
		}
		break;

		case AMFM_TUNER_CTRL_TUNER_STATUS_NOTIFICATION:
		{
			/*clear the RDS timer id after expires*/
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_RDS_Timer_id = 0;
			/* Reading RDS parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&st_recv_data,(Tchar *)(pst_msg->data),(Tu16)(sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data)),&slot);	

			AMFM_Tuner_Ctrl_Update_Rcvd_RDS_Data(pst_me_amfm_tuner_ctrl_inst, st_recv_data);

			if (st_recv_data.u32_TunedFrequency == pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq )
			{	   
				if(st_CurrentStationInfo.u16_pi != pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi)
				{
					b_PI_CMP = TRUE;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG,"[RADIO][AMFM_TC] :BG Current Station PI=%x",pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi);
				}
				else
				{
					b_PI_CMP = FALSE;
				}
				if(st_recv_data.b_PSN_Update == TRUE)
				{				   
					PS_CMP = strcmp((const char*)pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_ps,(const char*)st_CurrentStationInfo.au8_ps);
				}

				for (loop_index = 0 ;loop_index < pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumAFeqList ; loop_index++)
				{
					if(	st_CurrentStationInfo.u32_AFeqList[loop_index] !=  pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList[loop_index])
					{
						b_AF_Flag = TRUE;
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.b_AF_Checkbit =  TRUE;
					}
				}
				/* Compare function */
				if((b_PI_CMP != FALSE) ||(PS_CMP!= 0) || (b_AF_Flag!= FALSE))
				{
					SYS_RADIO_MEMCPY(&(st_CurrentStationInfo),&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),sizeof(st_CurrentStationInfo));
					AMFM_Tuner_Ctrl_NotifyTunerStatus((pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));
				    memset(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList,0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList));
	
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.b_AF_Checkbit = FALSE;
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumAFeqList = 0;
					/*Clearing the PI comparison flage */
					b_PI_CMP = FALSE;
					b_AF_Flag = FALSE;
				}
				else
				{
					/*for misra */
				}
			}
			else
			{
				b_PI_CMP = FALSE;
			}	
		}
		break;
		case QUALITY_NOTIFICATION_MSGID:
		{
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(st_amfm_tuner_ctrl_Quality), (Tchar *)(pst_msg->data), (Tu16)(sizeof(st_amfm_tuner_ctrl_Quality)), &slot);
			
				//AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(pst_me_amfm_tuner_ctrl_inst, &Hero_Tuner_TunerStatusNot);
				AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(pst_me_amfm_tuner_ctrl_inst, &st_amfm_tuner_ctrl_Quality);
				if (pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_FM)
				{
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = TUN_BAND_FM;
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
					AMFM_Tuner_Ctrl_Current_Station_Qual_Update(&st_amfm_tuner_ctrl_Quality, pst_me_amfm_tuner_ctrl_inst);
					AMFM_Tuner_Ctrl_Notify_CurrQual(pst_me_amfm_tuner_ctrl_inst->st_interpolation);
				}

				else if (pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_AM)
				{
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = TUN_BAND_AM;
					pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
					AMFM_Tuner_Ctrl_Current_Station_Qual_Update(&st_amfm_tuner_ctrl_Quality, pst_me_amfm_tuner_ctrl_inst);
					AMFM_Tuner_Ctrl_Notify_CurrQual(pst_me_amfm_tuner_ctrl_inst->st_interpolation);
				}
				else
				{
					/*Misra C*/
				}
			
		}
		break;			
		case HSM_MSGID_EXIT:
		{
			/*  free all timers */
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);	

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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgTuneHndlr         */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg								*pst_ret    		= NULL;  /* mark the message as handled */
	Tu32 									slot				= 0;
	Te_RADIO_ReplyStatus			 		e_TuneReplyStatus 	= REPLYSTATUS_FAILURE;

	
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00,sizeof(st_recv_data));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgTuneHndlr");
		
			/* Updating the current state name  used for debuging purpose */ 
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgTuneHndlr"));
		
			memset(&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));
			
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
			
			/* Request to AMFM tuner control soc to tune */
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
			if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = SYS_TIMER_START(AMFM_TUNER_CTRL_MAX_TUNE_QUAL_READ_TIME, AMFM_TUNER_CTRL_QUAL_READ, RADIO_AM_FM_TUNER_CTRL);
				if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : FM tune qual read timer is failed ");	
				}
				else
				{
					/* MISRA C */
				}
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_FAIL);
			}
		}
		break;
		case AMFM_TUNER_CTRL_TUNE_FAIL :
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : ETAL FM tune failed,status is  %d ",pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);	
			/*Clearing the timer id after timer expires*/         
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
			switch(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_ERROR       :
				case ETAL_RET_NO_DATA     :
				case ETAL_RET_IN_PROGRESS :
				{
					if(pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;  
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
						if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = SYS_TIMER_START(AMFM_TUNER_CTRL_MAX_TUNE_QUAL_READ_TIME, AMFM_TUNER_CTRL_QUAL_READ, RADIO_AM_FM_TUNER_CTRL);
							if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id == 0)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : FM tune qual read timer is failed ");	
							}
							else
							{
								/* MISRA C */
							}
						}
						else
						{
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_FAIL);
						}
					}
					else
					{
                  
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_FAILURE;
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_DONE_RESID);
					}
				}
				break;
				default :
				{
					/*Nothing */
				}
				break;

			}
		}
		break;
		case AMFM_TUNER_CTRL_QUAL_READ :
		{
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
        
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_GET_QUALITY(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);

			if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				if(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_FM)
				{
					AMFM_Tuner_Ctrl_GetBG_FMQualityCheck_Tune(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst);
				}
				else
				{
					/* Check the AM Quality check function, if back ground Am supports */
				}
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_QUAL_READ_FAIL);
			}

		}
		break;
		case AMFM_TUNER_CTRL_QUAL_READ_FAIL :
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : ETAL FM tune failed,status is  %d ",pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);	
         
			switch(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_ERROR       :
				case ETAL_RET_NO_DATA     :
				case ETAL_RET_IN_PROGRESS :
				{
					if(pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_QualRecall_Count < AMFM_TUNER_CTRL_MAX_QUALREAD_REPEAT_COUNT)
					{	
						pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_QualRecall_Count++;  
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_GET_QUALITY(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);
						if(pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							if(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band == TUN_BAND_FM)
							{
								AMFM_Tuner_Ctrl_GetBG_FMQualityCheck_Tune(&pBcastQualityContainer, pst_me_amfm_tuner_ctrl_inst);
							}
							else
							{
								/* Check the AM Quality check function, if back ground Am supports */
							}
						}
						else
						{
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_QUAL_READ_FAIL);
						}
					}
					else
					{
                
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_FAILURE;
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_BGTUNE_DONE_RESID);
					}
				}
				break;
				default :
				{
					/*Nothing */
				}
				break;
			}
		}
		break;
		case AMFM_TUNER_CTRL_RDS_DONE_RESID:
		{
				
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,AMFM_TUNER_CTRL_BGTUNE_DONE_RESID ,pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq);
		}
		break;
        case AMFM_TUNER_CTRL_BGTUNE_DONE_RESID: 
		{
  			pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;
            pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_QualRecall_Count = 0;
  			/*Clear the RDS timer id after expires */
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_RDS_Timer_id = 0;
			/* Updating the Band To Send exact TunerStatus Notification */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  TUN_BAND_FM;
			
			if(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status == REPLYSTATUS_SUCCESS)
			{
				/* Reading RDS parameters from the message */
				AMFM_Tuner_ctrl_ExtractParameterFromMessage(&st_recv_data,(Tchar *)(pst_msg->data),(Tu16)(sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data)),&slot);	
			
				if(st_recv_data.u32_TunedFrequency == pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq)
				{ 
					if( st_recv_data.u16_PI_Code != (Tu16)0)
					{
						pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi = st_recv_data.u16_PI_Code;
					}
					if( (st_recv_data.b_PSN_Update == TRUE) || (st_recv_data.b_NonRDS_Station == TRUE) )
					{	
						/* Updating PSN from RDS data to Current station info structure */
						SYS_RADIO_MEMCPY( (void *)pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_ps,(const void *)st_recv_data.st_FM_RDS_PSN.u8_ps,sizeof(st_recv_data.st_FM_RDS_PSN.u8_ps));
					
						e_TuneReplyStatus = REPLYSTATUS_SUCCESS;
    	        	
						/* Copying to GStruct for comparing the values */
						SYS_RADIO_MEMCPY(&(st_CurrentStationInfo),&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),sizeof(st_CurrentStationInfo));				
				
						/* sending respone to AMFM application layer */
						AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
			
					}	
					else
					{
						/* Do Nothing.Wait till RDS is read */
					}
				}
				else
				{
					/* Nothing to do. We got RDs data of some other frequency*/
				}	
			}	
			else
			{
				
				e_TuneReplyStatus = REPLYSTATUS_NO_SIGNAL;
				
				 RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : Background tune is failed");
				
				/* sending respone to AMFM application layer */
				AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);				
			}	
			pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq	;
		}
		break;
		
		case HSM_MSGID_EXIT:
		{
			if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id > 0)
			{
				if(SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : BG_Tune stop timer is failed ");
				}
				else
				{
					st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
				}
			}
			else
			{
				/*nothing*/
			}	
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);	
		    
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFUpdateTuneHndlr         */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFUpdateTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret    				= NULL;  /* mark the message as handled */
	Te_RADIO_ReplyStatus				    e_AF_UpdateReplyStatus 	= REPLYSTATUS_FAILURE;
	

	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFUpdateTuneHndlr");
	
			/* Updating the current state name  used for debuging purpose */ 
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFUpdateTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgAFUpdateTuneHndlr"));
		
		} 
		break;
		
		case STOP_DONE_MSG_ID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_TC]  BG : AF update req Stop Msg %d ",pst_me_amfm_tuner_ctrl_inst->u32_freq);
				
			memset(&(AMFMTuner_Tune_AFQuality), 0x00, sizeof(AMFMTuner_Tune_AFQuality));
			memset(&(pst_me_amfm_tuner_ctrl_inst->st_interpolation),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_interpolation));
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->u32_freq;
			 /* Request to AMFM tuner control soc to tune */
            SOC_CMD_AF_UPDATE(pst_me_amfm_tuner_ctrl_inst->u32_freq);				
		}
		break;
        case AMFM_TUNER_CTRL_AF_QUALITY_DONE_RESID: 
		{			
			/*Clear the timer id after timer expires */
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;		
			SOC_CMD_READ_AF_UPDATE(&AMFMTuner_Tune_AFQuality);
		}
		break;
        case AMFM_TUNER_CTRL_FM_AF_UPDATE_QUALITY_DONE_RESID: 
		{
			
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.s32_BBFieldStrength  	= 	AMFM_FS_CONVERSION(AMFMTuner_Tune_AFQuality.s32_BBFieldStrength );
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_UltrasonicNoise 	= 	AMFM_USN_CONVERSION(AMFMTuner_Tune_AFQuality.u32_UltrasonicNoise);
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_Multipath 	= 	AMFM_WAM_CONVERSION(AMFMTuner_Tune_AFQuality.u32_Multipath);
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_FrequencyOffset 	= 	AMFM_OFS_CONVERSION(AMFMTuner_Tune_AFQuality.u32_FrequencyOffset);
			
			pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation = QUAL_CalcFmQual(pst_me_amfm_tuner_ctrl_inst->st_interpolation.s32_BBFieldStrength, pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_UltrasonicNoise, pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_Multipath,NULL ,NULL, pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_FrequencyOffset );
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq;
			
			/* Update the Cuurent Band as FM to Read  the RDS in Listen */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = TUN_BAND_FM;
			
			e_AF_UpdateReplyStatus = REPLYSTATUS_SUCCESS;
			AMFM_Tuner_Ctrl_Response_AF_Update(e_AF_UpdateReplyStatus,pst_me_amfm_tuner_ctrl_inst->u32_freq,pst_me_amfm_tuner_ctrl_inst->st_interpolation);
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_TC]  BG : AF update Response send to APP %d ",pst_me_amfm_tuner_ctrl_inst->u32_freq);
			/* Transit to AMFM tuner control instance hsm active sleep background quality handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_quality_state);
		}
		break;
		case HSM_MSGID_EXIT:
		{

		}
		break;
		default:
		{
			if(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id > 0)
			{
				if(SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][AMFM_TC]  : BG_AF_Update stop timer is failed ");
				}
				else
				{
						st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = 0;
				}
			}
			else
			{
				/*nothing*/
			}	
			pst_ret = pst_msg;
		}
		break;
	}
	return pst_ret;
}
/*===========================================================================*/
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFMCheckTuneHndlr         */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFMCheckTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret    				= NULL;  /* mark the message as handled */
	Te_RADIO_ReplyStatus 					e_AF_CheckReplyStatus 	= REPLYSTATUS_FAILURE;
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			memset(&(st_recv_data),0x00,sizeof(st_recv_data));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFMCheckTuneHndlr");
			
			/* Updating the current state name  used for debuging purpose */ 
		
  			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFMCheckTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFMCheckTuneHndlr"));
			
			memset(&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi));
		   /* Request to AMFM tuner control soc to tune */
		
            u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->u32_freq;
              
			SOC_CMD_FM_CHECK(pst_me_amfm_tuner_ctrl_inst->u32_freq);			
		} 
		break;
	
		case AMFM_TUNER_CTRL_RDS_DONE_RESID:
		{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,AMFM_TUNER_CTRL_FMCHECK_DONE_RESID ,pst_me_amfm_tuner_ctrl_inst->u32_freq);
		}
		break;
	    case AMFM_TUNER_CTRL_FMCHECK_DONE_RESID: 
		{
  			/*clear the RDS timer id after expires*/
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_RDS_Timer_id = 0;
  			SYS_RADIO_MEMCPY(&st_recv_data,pst_msg->data,sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data) );
			/* Update the Band To Send exact TunerStatus Notification */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band =  TUN_BAND_FM;
			
			if(pst_me_amfm_tuner_ctrl_inst->u32_freq == st_recv_data.u32_TunedFrequency)
			{
				if((st_recv_data.u16_PI_Code != (Tu16) 0) || (st_recv_data.b_NonRDS_Station == TRUE))
				{	
					e_AF_CheckReplyStatus = REPLYSTATUS_SUCCESS;

					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] : BG Check_Succes and PI = %x\n",st_recv_data.u16_PI_Code);
					/* sending respone to AMFM application layer */
					AMFM_Tuner_Ctrl_Response_FM_Check(e_AF_CheckReplyStatus,st_recv_data.u16_PI_Code);

					/* Transit to AMFM tuner control instance hsm active back groune quality  handler */
					HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_quality_state);
				}
			}
			else
			{
				/* Misra C Rule */
			}	
		}
		break;
		case HSM_MSGID_EXIT:
		{
				AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER,STOP_RDS_MSG_ID ,u32_SOC_Frequency);	

			

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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFmJumpTuneHndlr   */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFmJumpTuneHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 							pst_ret    			= NULL;  /* mark the message as handled */
	Te_RADIO_ReplyStatus 					e_TuneReplyStatus 	= REPLYSTATUS_FAILURE;
	
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFmJumpTuneHndlr");
			
			/* Updating the current state name  used for debuging purpose */ 
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFmJumpTuneHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgFmJumpTuneHndlr"));

			memset(&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),0x00,sizeof(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo));
			 /* Request to AMFM tuner control soc to tune */
            
			u32_SOC_Frequency = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
			
			SOC_CMD_FM_JUMP(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq);					
		} 
		break;
		case AMFM_TUNER_CTRL_FM_QUALITY_DONE_RESID:
		{
			AMFM_Tuner_Ctrl_Notify_CurrFrequency(pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq);
			
			if(u8_count < AMFM_TUNER_CTRL_JUMPQUAL_AVGCOUNT)
			{							
//				SOC_CMD_Quality_Read(&Hero_Tuner_FMQual_parametrs);
				SOC_CMD_GET_QUALITY(&St_Tuner_FMQual_parametrs, pst_me_amfm_tuner_ctrl_inst->e_Band);
				
				AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(pst_me_amfm_tuner_ctrl_inst, &St_Tuner_FMQual_parametrs);
			
				u32_interqual = u32_interqual + pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation;
				
				u8_count =  (Tu8)(u8_count + (Tu8)1);
				
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_FM_QUALITY_DONE_RESID);
			}
			else
			{
				pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation = (u32_interqual/(Tu32)5);
				u8_count = 0;
				u32_interqual = 0;
				AMFM_Tuner_Ctrl_GetjumpQualityCheck(&St_Tuner_FMQual_parametrs, pst_me_amfm_tuner_ctrl_inst);
			}
		}
		break;
	    case AMFM_TUNER_CTRL_TUNE_DONE_RESID: 
		{
			/* Update the Cuurent Band as FM to Read  the RDS in Listen */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = TUN_BAND_FM;
			
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_freq 	= 	pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;	
			
			e_TuneReplyStatus = REPLYSTATUS_SUCCESS;
	        /* sending respone to AMFM application layer */
			SYS_RADIO_MEMCPY(&(st_CurrentStationInfo),&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),sizeof(st_CurrentStationInfo));
							
			AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
			
			
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] BG FM_Jump is success");
			
			#if 0
				if(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.SOC_Status == SOC_TUNE_SUCCESS)
				{
					e_TuneReplyStatus = AMFM_TUNER_CTRL_TUNE_SUCCESS;
	            	/* sending respone to AMFM application layer */
					SYS_RADIO_MEMCPY(&(st_CurrentStationInfo),&(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo),sizeof(st_CurrentStationInfo));				
					AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
				
				}
				else
				{
					e_TuneReplyStatus = AMFM_TUNER_CTRL_NO_AMFM_SIGNAL;
					 RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : Background FMjump tune is failed");
				
	            	/* sending respone to AMFM application layer */
					AMFM_Tuner_Ctrl_Response_Tune(e_TuneReplyStatus, pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo);
							
				}
			#endif
			pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Startfreq = pst_me_amfm_tuner_ctrl_inst->st_Tunereq.u32_freq;
			
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_active_sleep_bg_quality_state);

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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgScanHndlr      */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgScanHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret								 = NULL;
	Tu32 slot										 = 0;
	Te_RADIO_ReplyStatus e_GetStationListReplyStatus = REPLYSTATUS_REQ_CANCELLED;
	Te_RADIO_SharedMemoryType e_SharedMemoryType     = SHAREDMEMORY_INVALID;
	Te_RADIO_ScanType e_ScanType                     = RADIO_SCAN_BG;

	switch (pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;

		case HSM_MSGID_START:
		{
			memset(&(st_recv_data), 0x00, sizeof(st_recv_data));

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgScanHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str, "AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgScanHndlr", sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveSleepBgScanHndlr"));

			memset(ast_Scaninfo, 0x00, sizeof(ast_Scaninfo));
			scan_index_FM = 0;
			scan_index_AM = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_TC] AMFM TC Mutex Lock  ActiveSleepBgScanHndlr StartMsg");
			Sys_Mutex_Lock(STL_AMFM_APP_AMFM_TC);

			/*Tune to the lower frequency of the band*/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				//u32_Receiver_Tuned_freq = pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq;
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_START_MSGID);
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_FAIL);
			}
		}
		break;

		case AMFM_TUNER_CTRL_TUNE_FAIL:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : Tune to lower freq in BG failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
			switch (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_IN_PROGRESS:
				case ETAL_RET_ERROR:
				{
					if (pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

						if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							//u32_Receiver_Tuned_freq = pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq;
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_START_MSGID);
						}
						else
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_FAIL);
						}
					}
					else
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_STOP_MSGID);
					}
				}
				break;
				default:
				{
							/*Nothing to do*/
				}
				break;
			}
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_START_MSGID:
		{
			/* Request to AMFM tuner control soc to scan, As of now done for FM later has to be revisited for AM */
			#if BG_AMSCAN
			if((pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW) || (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW))
			{
				SOC_CMD_BGSCAN_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq,pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
			}

			else if(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
			{
				SOC_CMD_BGSCAN_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq,TUN_BAND_FM);
			}
			else
			{
				/*Added For MISRA */
			} 
			#else
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_START(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM learn start success");
			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID);
			}

			#endif
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM learn start failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);

			switch (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_IN_PROGRESS:
				case ETAL_RET_ERROR:
				{
					if (pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						/*requesting again for scan*/
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_START(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

						if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM learn start success");
						}
						else
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID);
						}
					}
					else
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
					}
				}
				break;
				default:
				{
					/*Nothing to do*/
				}
				break;
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_INFO_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : ETAL AMFM learn callback,status is  %d ", st_amfm_tuner_ctrl_scan_status.m_status);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(st_amfm_tuner_ctrl_scan_status), (Tchar *)pst_msg->data, (Tu8)(sizeof(st_amfm_tuner_ctrl_scan_status)), &slot);

			if (st_amfm_tuner_ctrl_scan_status.m_status == ETAL_SEEK_STARTED)
			{
				//do nothing
			}
			else if (st_amfm_tuner_ctrl_scan_status.m_status == ETAL_SEEK_RESULT)
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SEEK_RESULT_RESID);
			}
			else if (st_amfm_tuner_ctrl_scan_status.m_status == ETAL_SEEK_FINISHED)
			{
				//do nothing
			}
			else
			{
				//do nothing
			}
		}
		break;

		case AMFM_TUNER_CTRL_SEEK_RESULT_RESID:
		{
			if (st_amfm_tuner_ctrl_scan_status.m_fullCycleReached == TRUE)
			{
				pst_me_amfm_tuner_ctrl_inst->b_IsScan_FullcycleReached = TRUE;
				if (st_amfm_tuner_ctrl_scan_status.m_frequencyFound == TRUE)
				{
					pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_scan_status.m_frequency;
					//FM Band
					if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
					{
						AMFM_Tuner_Ctrl_QualUpdate_FM_Learn(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
					}
					//AM Band
					else if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW || pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW)
					{
						AMFM_Tuner_Ctrl_QualUpdate_AM_Learn(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
					}
					else
					{
						//do nothing
					}
				}
				else
				{
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_STOP_MSGID);
				}
			}
			else if (st_amfm_tuner_ctrl_scan_status.m_frequencyFound == TRUE)
			{
				pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = st_amfm_tuner_ctrl_scan_status.m_frequency;
				if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
				{
					AMFM_Tuner_Ctrl_QualUpdate_FM_Learn(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
				}
				else if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW || pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW)
				{
					AMFM_Tuner_Ctrl_QualUpdate_AM_Learn(&st_amfm_tuner_ctrl_scan_status.m_quality, pst_me_amfm_tuner_ctrl_inst);
				}
				else
				{
					/*MISRAC*/
				}

			}
			else
			{
				//do nothing
			}
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_RDS_START_REQID:
		{
			AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER, AMFM_TUNER_CTRL_UPDATE_RDS_IN_FM_STL, pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq);
		}
		break;

		case AMFM_TUNER_CTRL_UPDATE_RDS_IN_FM_STL:
		{
			/* Reading RDS parameters from the message */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&st_recv_data, (Tchar *)(pst_msg->data), (Tu8)(sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data)), &slot);

			if (st_recv_data.u32_TunedFrequency == pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_TC] : Background FM scan is in progress, Frequency is %d , Scan index is %d", st_recv_data.u32_TunedFrequency, scan_index_FM);

				if (st_recv_data.b_PI_Update != FALSE)
				{
					ast_Scaninfo[scan_index_FM].u16_pi = st_recv_data.u16_PI_Code;
				}
				if ((st_recv_data.b_PSN_Update == TRUE) || (st_recv_data.b_NonRDS_Station == TRUE))
				{
					/* Copying PSN to FM STL */
					SYS_RADIO_MEMCPY((void *)ast_Scaninfo[scan_index_FM].au8_ps, (const void *)st_recv_data.st_FM_RDS_PSN.u8_ps, sizeof(st_recv_data.st_FM_RDS_PSN.u8_ps));

					/* Incrementing STL index */
					scan_index_FM = (Tu8)(scan_index_FM + (Tu8)1);
					if (scan_index_FM >= AMFM_TUNER_CTRL_MAX_STATIONS)
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_STOP_MSGID);
					}
					else
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID);
					}
				}
				else
				{
					/*For Misra C*/
				}
			}
			else
			{
				/*Nothing to do.We got RDS data for other frequency*/
			}
		}
		break;

		case AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID:
		{
			if (pst_me_amfm_tuner_ctrl_inst->b_IsScan_FullcycleReached != TRUE)
			{
				/*request to continue the scan*/
				pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_CONTINUE(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM learn continue success");
				}
				else
				{
					AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID);
				}

			}
			else
			{
				/* To avoid incrementing of scan_index when RDS data received after scan stop */
				pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq = 0; 
				pst_me_amfm_tuner_ctrl_inst->b_IsScan_FullcycleReached = FALSE;
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_STOP_MSGID);
			}
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : Scan continue failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);

			switch (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status)
			{
				case ETAL_RET_ERROR:
				{
					if (pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count < AMFM_TUNER_CTRL_MAX_REPEAT_COUNT)
					{
						/*request to continue the scan again*/
						pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SCAN_CONTINUE(e_ScanType, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

						if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AMFM learn continue success");
						}
						else
						{
							pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count++;
							AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID);
						}
					}
					else
					{
						AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_STOP_MSGID);
					}
				}
				break;
				default:
				{
						/*Nothing to do*/
				}
				break;
			}
		}
		break;

		#if BG_AMSCAN
		/* Has to be revisited if AM background scan is also to be considered*/
		case AM_FM_TUNER_CTRL_SOC_AMSCAN_CONTINUE_MSGID:
		{
			if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq <= pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Upperfreq)
			{
				if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq == AMFM_TUNER_CTRL_AM_LW_ENDFREQ)
				{
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq = AMFM_TUNER_CTRL_AM_MW_STARTFREQ;
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize = MW_WEU_STEP;
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band = TUN_BAND_AM_MW;
					SOC_CMD_BGSCAN_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq, pst_me_amfm_tuner_ctrl_inst->st_Tunereq.e_Band);

				}
				else
				{
					pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq = pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq + pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Stepsize;
					SOC_CMD_BGSCAN_TUNE(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.U32_Lowerfreq, pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);
				}


			}
			else
			{
				AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
			}
		}
		break;
		#endif

		case AMFM_TUNER_CTRL_SCAN_STOP_MSGID:
		{
			/*Requesting for Scan stop*/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				//do nothing
			}
			else
			{
				/*Requesting again for scan stop*/
				pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SEEK_STOP(pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band);

				if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
				{
					//do nothing
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : learn stop failed due to:%d", pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status);
				}
			}
			AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
		}
		break;

		case AMFM_TUNER_CTRL_SCAN_DONE_RESID:
		{
			Tu8 u8_temp = 0;
			pst_me_amfm_tuner_ctrl_inst->u8_AMFMCmd_Recall_Count = 0;

			/* sending respone to AMFM application layer */
			Sys_Mutex_Unlock(STL_AMFM_APP_AMFM_TC);
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_TC] AMFM TC Mutex UnLock ActiveSleepBgScanHndlr AMFM_TUNER_CTRL_SCAN_DONE_RESID");
			if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
			{
				u8_temp = scan_index_FM;
				/* Updating the Band To Send exact TunerStatus Notification */
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band;
			}
			else if ((pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_MW) || (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_AM_LW))
			{
				u8_temp = scan_index_AM;
				/* Updating the Band To Send exact TunerStatus Notification */
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_Band = pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band;
			}
			else
			{
				/*Nothing to do*/
			}

			if (u8_temp != (Tu8)0)
			{
				e_GetStationListReplyStatus = REPLYSTATUS_SUCCESS;

				if (u8_temp == scan_index_FM)
				{
					e_SharedMemoryType = FM_TUNER_APP;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_TC] : BGFM Scan End and Station list is notified to Application layer");
				}
				else if (u8_temp == scan_index_AM)
				{
					e_SharedMemoryType = AM_TUNER_APP;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_TC] : BGAM Scan End and Station list is notified to Application layer");
				}
				else
				{
					/*Nothing to do*/
				}
			}
			else
			{
				e_GetStationListReplyStatus = REPLYSTATUS_EMPTY;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_TC] : BGScan End and  Background Empty station list is notified");
			}

			/* sending Scan respone to AMFM application layer */
			AMFM_Tuner_Ctrl_Response_GetStationList(e_GetStationListReplyStatus);
			/*notifying the updated station list to AMFM Application layer*/
			AMFM_Tuner_Ctrl_Notify_STLUpdated(e_SharedMemoryType);
		}
		break;

		case HSM_MSGID_EXIT:
		{
			if (pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band == TUN_BAND_FM)
			{
				AMFM_Tuner_Ctrl_startRDSDecode(RADIO_FM_RDS_DECODER, STOP_RDS_MSG_ID, pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq);

			}
			else
			{
				//do nothing
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveErrorHndlr                    */
/*===========================================================================*/
Ts_Sys_Msg* AMFM_TUNER_CTRL_INST_HSM_ActiveErrorHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 			pst_ret    = NULL; /* mark the message as handled */
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveErrorHndlr");
			
			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str , "AMFM_TUNER_CTRL_INST_HSM_ActiveErrorHndlr",sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveErrorHndlr"));
			/* Transit to AMFM tuner control instance hsm active stop handler */
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst,&amfm_tuner_ctrl_inst_hsm_active_stop_state);    
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
/*  Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveStopHndlr             */
/*===========================================================================*/
Ts_Sys_Msg*  AMFM_TUNER_CTRL_INST_HSM_ActiveStopHndlr(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* 			pst_ret    = NULL;                             /* mark the message as handled*/
	
	
	switch(pst_msg->msg_id)
	{
		case HSM_MSGID_ENTRY:
		{

		}
		break;
		case HSM_MSGID_START:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_TC] Handler: AMFM_TUNER_CTRL_INST_HSM_ActiveStopHndlr");

			/* Updating the current state name  used for debuging purpose */
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->p_curr_state_str, "AMFM_TUNER_CTRL_INST_HSM_ActiveStopHndlr", sizeof("AMFM_TUNER_CTRL_INST_HSM_ActiveStopHndlr"));

			/* Destroy datapath,stop RDS,destroy receiver*/
			pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_TUNER_DEINIT();

			if (pst_me_amfm_tuner_ctrl_inst->AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				/*sending stop done message to main hsm*/
				AMFM_Tuner_Ctrl_stopreply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_INST_HSM_STOP_DONE);
			}
			else
			{
				/*MISRAC*/
			}

			/* Transit to AMFM tuner control instance hsm inactive handler*/
			HSM_STATE_TRANSITION(pst_me_amfm_tuner_ctrl_inst, &amfm_tuner_ctrl_inst_hsm_inactive_state);
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
/*  void AMFM_TUNER_CTRL_INST_HSM_Init                     */
/*===========================================================================*/
void AMFM_TUNER_CTRL_INST_HSM_Init(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst)
{
	if(pst_me_amfm_tuner_ctrl_inst != NULL)
	{
		/*Clearing the HSM amfm_Tuner_Ctrl_hsm buffer */
		memset(pst_me_amfm_tuner_ctrl_inst,0x00,sizeof(Ts_AMFM_Tuner_Ctrl_Inst_hsm)); 
		
		pst_me_amfm_tuner_ctrl_inst->p_curr_state_str = pst_me_amfm_tuner_ctrl_inst->str_state;	
		/*Constructing the HSM amfm_Tuner_Ctrl_hsm */
		HSM_CTOR((Ts_hsm*)pst_me_amfm_tuner_ctrl_inst,&amfm_tuner_ctrl_inst_hsm_top_state,AMFM_TUNER_CTRL_INST_HSM_CID);
		/*Starting the HSM amfm_Tuner_Ctrl_hsm .After this call,inactive state is reached */
		HSM_ON_START((Ts_hsm*)pst_me_amfm_tuner_ctrl_inst );
				
	}
}
