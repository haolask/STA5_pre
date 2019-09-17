/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_inst_hsm.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager Application															*
*  Description			: This source file consists of function defintions of all function handlers of HSM	*
*						  radio_mngr_app_inst_hsm of Radio Manager Application component					*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
							radio_mngr_app_inst_hsm.c
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include <string.h>
#include "amfm_app_request.h"
#include "amfm_app_notify.h"
#include "amfm_app_response.h"
#include "dab_app_request.h"
#include "dab_app_notify.h"		
#include "radio_mngr_app_notify.h"  
#include "radio_mngr_app_response.h"
#include "radio_mngr_app_request.h"
#include "sys_nvm.h"

/*---------------------------------------------------------------------------- -
variables(extern)
---------------------------------------------------------------------------- - */
extern Ts_DAB_App_DataServiceRaw			st_Dabapp_SLS_Data;
/*-----------------------------------------------------------------------------
variables
-----------------------------------------------------------------------------*/
Ts_Radio_Mngr_App_DataServiceRaw			st_Radio_Mngr_App_SLS_Data;
/*---------------------------------------------------------------------------*/

/* HSM state hierarchy */
HSM_CREATE_STATE(radio_mngr_app_inst_hsm_top_state,                                           NULL,                                                Radio_Mngr_App_Inst_Hsm_TopHndlr,                                "radio_mngr_app_inst_hsm_top_state");
	HSM_CREATE_STATE(radio_mngr_app_inst_hsm_inactive_state,                                  &radio_mngr_app_inst_hsm_top_state,                  Radio_Mngr_App_Inst_Hsm_InactiveHndlr,                           "radio_mngr_app_inst_hsm_inactive_state");
	HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_state,                                    &radio_mngr_app_inst_hsm_top_state,                  Radio_Mngr_App_Inst_Hsm_ActiveHndlr,                             "radio_mngr_app_inst_hsm_active_state");
		HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_start_state,                          &radio_mngr_app_inst_hsm_active_state,               Radio_Mngr_App_Inst_Hsm_ActiveStartHndlr,                        "radio_mngr_app_inst_hsm_active_start_state");
		HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_idle_state,                           &radio_mngr_app_inst_hsm_active_state,               Radio_Mngr_App_Inst_Hsm_ActiveIdleHndlr,                         "radio_mngr_app_inst_hsm_active_idle_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_idle_listen_state,                &radio_mngr_app_inst_hsm_active_idle_state,          Radio_Mngr_App_Inst_Hsm_ActiveIdleListenHndlr,                   "radio_mngr_app_inst_hsm_active_idle_listen_state");
		HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_stop_state,                           &radio_mngr_app_inst_hsm_active_state,               Radio_Mngr_App_Inst_Hsm_ActiveStopHndlr,                         "radio_mngr_app_inst_hsm_active_stop_state");
		HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_state,                           &radio_mngr_app_inst_hsm_active_state,               Radio_Mngr_App_Inst_Hsm_ActiveBusyHndlr,                         "radio_mngr_app_inst_hsm_active_busy_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_stationlist_state,			  &radio_mngr_app_inst_hsm_active_busy_state,		   Radio_Mngr_App_Inst_Hsm_ActiveBusy_StationlistHndlr,			    "radio_mngr_app_inst_hsm_active_busy_stationlist_state");  
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_mute_state,				  &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_MuteHndlr,				    "radio_mngr_app_inst_hsm_active_busy_mute_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_demute_state,				  &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_DemuteHndlr,				    "radio_mngr_app_inst_hsm_active_busy_demute_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_seekupdown_state,			  &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_SeekUpDownHndlr,              "radio_mngr_app_inst_hsm_active_busy_seekupdown_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_selectband_state,            &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectBandHndlr,              "radio_mngr_app_inst_hsm_active_busy_selectband_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_play_selectstation_state,    &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_Play_SelectStationHndlr,      "radio_mngr_app_inst_hsm_active_busy_Play_SelectStation_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_preset_recall_state,		  &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_PresetRecallHndlr,			"radio_mngr_app_inst_hsm_active_busy_Preset_Recall_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_tuneupdown,				  &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneUpDownHndlr,				"radio_mngr_app_inst_hsm_active_busy_Tune_UpDown_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_af_tune_state,				  &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_AF_TuneHndlr,					"radio_mngr_app_inst_hsm_active_busy_AF_Tune_state");
			HSM_CREATE_STATE(radio_mngr_app_inst_hsm_active_busy_tune_by_frequency_state,	  &radio_mngr_app_inst_hsm_active_busy_state,          Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneByFrequency_Hndlr,		"radio_mngr_app_inst_hsm_active_busy_tune_by_frequency_state");
		
/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

Ts_Radio_Mngr_App_TimerIds st_Radio_Mngr_App_TimerID;		  
/*===========================================================================*/
/*  void Radio_Mngr_App_Hsm_Init                                                    */
/*===========================================================================*/
void Radio_Mngr_App_Inst_Hsm_Init(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst)
{
    if(pst_me_radio_mngr_inst != NULL)
    {
        /* clear the hsm */
        memset(pst_me_radio_mngr_inst, RADIO_MNGR_APP_UINT8_ZERO, sizeof(Ts_Radio_Mngr_App_Inst_Hsm));
		 pst_me_radio_mngr_inst->u8p_curr_state_str = pst_me_radio_mngr_inst->str_state;

        /* Call the base class Ctor */
        HSM_CTOR((Ts_hsm*)pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_top_state, RADIO_MNGR_APP_INST_HSM);

        /* Register Services */
		pst_me_radio_mngr_inst->u16_cid = RADIO_MNGR_APP_INST_HSM;

        /* start HSM */
        HSM_ON_START(pst_me_radio_mngr_inst);

    }
    else
    {
        /* do nothing*/
    }
}


/*===========================================================================*/
/*  Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_TopHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_TopHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */

    PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_TopHndlr");
            SYS_RADIO_MEMCPY( (void*)pst_me_radio_mngr_inst->u8p_curr_state_str , "Radio_Mngr_App_Inst_Hsm_TopHndlr \n",
								sizeof( "Radio_Mngr_App_Inst_Hsm_TopHndlr \n"));
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_inactive_state);
        }
        break;

        case HSM_MSGID_EXIT:
        {
        }
        break;

        default:
        {
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]TopHandler MSG: %d", pst_msg->msg_id);
            pst_ret = pst_msg;
        }
        break;
    }

    return pst_ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_InactiveHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_InactiveHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
    pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);
    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_InactiveHndlr");
            SYS_RADIO_MEMCPY( (void*)pst_me_radio_mngr_inst->u8p_curr_state_str , "Radio_Mngr_App_Inst_Hsm_InactiveHndlr \n" ,sizeof("Radio_Mngr_App_Inst_Hsm_InactiveHndlr \n"));
        }
        break;

        /* Instance HSM is starting */
        case RADIO_MNGR_APP_INST_START:
        {
            /*Copying the Market,as it will be used for making switch request based on market*/
            ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_Market),   pst_msg->data, sizeof(pst_me_radio_mngr_inst->e_Market),   &(pst_me_radio_mngr_inst->u32_slot));
            HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_start_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveHndlr");
            SYS_RADIO_MEMCPY((void*) pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveHndlr \n",
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveHndlr \n"));
		}
		break;
		
		/*Stop the instance HSM in shutdown case*/
		case RADIO_MNGR_APP_INST_STOP:
		{
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_stop_state);
		}
		break;

		/*Request for Send Qyality information to the HMI in Diag/ENG Mode*/
		case RADIO_MNGR_APP_GET_QUALITY_DIAG_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Quality Diag Requested");
			Radio_Mngr_App_Notify_Quality_Diag(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
		}
		break;

		/*Request for Send Active Band Current Station information to the HMI in Diag/ENG Mode*/
		case RADIO_MNGR_APP_GET_CURRENTSTATIONINFO_DIAG_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Station info Diag Requested");
			Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst);
		}
		break;

		/*Request for Send Active Band Station List to the HMI in Diag/ENG Mode*/
		case RADIO_MNGR_APP_OBTAIN_STATIONLIST_DIAG_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]StL Diag Requested");
			Radio_Mngr_App_Notify_UpdateSTL_Diag(pst_me_radio_mngr_inst);
		}
		break;

		/*Request for Send Current FM Station Alternat Frequency list to the HMI in Diag/ENG Mode*/
		case RADIO_MNGR_APP_GET_AFLIST_DIAG_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AFList Diag Requested");
			Radio_Mngr_App_Notify_AFList_Diag(pst_me_radio_mngr_inst);
		}
		break;		

        /*Request for AF Switch from HMI*/
		case RADIO_MNGR_APP_AF_SWITCH_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_RDSSettings_Request), (pst_msg->data), sizeof(Te_Radio_Mngr_App_RDSSettings), &(pst_me_radio_mngr_inst->u32_slot));

			pst_me_radio_mngr_inst->e_RDSSettings_AMFM_ReplyStatus	= REPLYSTATUS_INVALID_PARAM;
			pst_me_radio_mngr_inst->e_RDSSettings_DAB_ReplyStatus	= REPLYSTATUS_INVALID_PARAM;
 
			AMFM_App_Request_AFSwitch((Te_AMFM_App_AF_Switch)pst_me_radio_mngr_inst->e_RDSSettings_Request);

			if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && 
					pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
			{
				DAB_App_Request_DAB_AF_Settings((Te_DAB_App_AF_Switch)pst_me_radio_mngr_inst->e_RDSSettings_Request);
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_AMFM_APP_AF_SWITCH_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_RDSSettings_AMFM_ReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_RDSSettings_AMFM_ReplyStatus == REPLYSTATUS_SUCCESS && 
									(pst_me_radio_mngr_inst->e_RDSSettings_DAB_ReplyStatus == REPLYSTATUS_SUCCESS || 
										(pst_me_radio_mngr_inst->b_DAB_BandStatus != RADIO_MANAGER_DAB_BAND_SUPPORTED || 
										 pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus != RADIO_MNGR_APP_SRC_ACTIVE)))
			{
				/*Update the u8_Settings variable*/
				if(pst_me_radio_mngr_inst->e_RDSSettings_Request == RADIO_MNGR_APP_RDS_SETTINGS_ENABLE)
				{
					pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_SETBIT(pst_me_radio_mngr_inst->u8_Settings, 2u));
					pst_me_radio_mngr_inst->e_RDSSettings = RADIO_MNGR_APP_RDS_SETTINGS_ENABLE;
				}
				else
				{
					pst_me_radio_mngr_inst->u8_Settings  = (Tu8)(LIB_CLEARBIT(pst_me_radio_mngr_inst->u8_Settings, 2u));
					pst_me_radio_mngr_inst->e_RDSSettings = RADIO_MNGR_APP_RDS_SETTINGS_DISABLE;
				}
				/*Send AF switch response to HMI*/
				Radio_Mngr_App_Response_RDS_Switch_Status(pst_me_radio_mngr_inst->e_RDSSettings_AMFM_ReplyStatus);
				
				/*Notify DAB<=>FM switch,Announcement switch,AF switch status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
				
				/*Function to update Flash Memory parameters*/
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				/*If Response is failure then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}

			if(pst_me_radio_mngr_inst->e_RDSSettings_Request == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE && pst_me_radio_mngr_inst->e_RDSSettings == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE &&
						pst_me_radio_mngr_inst->e_RDSSettings_AMFM_ReplyStatus == REPLYSTATUS_SUCCESS && 
						(pst_me_radio_mngr_inst->e_RDSSettings_DAB_ReplyStatus == REPLYSTATUS_SUCCESS ||
							pst_me_radio_mngr_inst->b_DAB_BandStatus != RADIO_MANAGER_DAB_BAND_SUPPORTED || 
							 pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus != RADIO_MNGR_APP_SRC_ACTIVE ||
							 pst_me_radio_mngr_inst->e_DABTunerStatus != RADIO_FRMWK_COMP_STATUS_NORMAL) &&
							 pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
			{
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
				{
					AMFM_App_Request_Cancel();
				}
				else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
				{
					/*Cancel Request to the DAB App layer for the Current Execution */
					DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
				}
				else{/*FOR MISRA C*/}
			}else{/*FOR MISRA C*/}
		}
		break;

		/*DAB AF/RDS setting Response*/
		case RADIO_MNGR_APP_DAB_APP_RDS_SETTINGS_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_RDSSettings_DAB_ReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			
			/*Wait for AMFM and DAB success response*/
			if(pst_me_radio_mngr_inst->e_RDSSettings_DAB_ReplyStatus == REPLYSTATUS_SUCCESS && 
									pst_me_radio_mngr_inst->e_RDSSettings_AMFM_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				/*Update the u8_Settings variable*/
				if(pst_me_radio_mngr_inst->e_RDSSettings_Request == RADIO_MNGR_APP_RDS_SETTINGS_ENABLE)
				{
					pst_me_radio_mngr_inst->u8_Settings   = (Tu8)(pst_me_radio_mngr_inst->u8_Settings | (0X04));
					pst_me_radio_mngr_inst->e_RDSSettings = RADIO_MNGR_APP_RDS_SETTINGS_ENABLE;
				}
				else
				{
					pst_me_radio_mngr_inst->u8_Settings   = (Tu8)(pst_me_radio_mngr_inst->u8_Settings & (~(0X04)));					
					pst_me_radio_mngr_inst->e_RDSSettings = RADIO_MNGR_APP_RDS_SETTINGS_DISABLE;
				}
				/*Send AF switch response to HMI*/
				Radio_Mngr_App_Response_RDS_Switch_Status(pst_me_radio_mngr_inst->e_RDSSettings_DAB_ReplyStatus);
				
				/*Notify DAB<=>FM switch,Announcement switch,AF switch status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
				
				/*Function to update Flash Memory parameters*/
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				/*If Response Failure then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Settings DAB<=>FM: %d, TA_Anno: %d, RDS: %d, Info_Anno: %d", 
								pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch,
								pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch);
								
			if(pst_me_radio_mngr_inst->e_RDSSettings_Request == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE && pst_me_radio_mngr_inst->e_RDSSettings == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE &&
						pst_me_radio_mngr_inst->e_RDSSettings_AMFM_ReplyStatus == REPLYSTATUS_SUCCESS && 
						pst_me_radio_mngr_inst->e_RDSSettings_DAB_ReplyStatus == REPLYSTATUS_SUCCESS &&
						pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
			{
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
				{
					AMFM_App_Request_Cancel();
				}
				else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
				{
					/*Cancel Request to the DAB App layer for the Current Execution */
					DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
				}
				else{/*FOR MISRA C*/}
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_ENABLE_DABFMLINKING_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request), (pst_msg->data), sizeof(Te_Radio_Mngr_App_DABFMLinking_Switch), &(pst_me_radio_mngr_inst->u32_slot));

			pst_me_radio_mngr_inst->e_FMtoDABFollowingStatus 		= REPLYSTATUS_INVALID_PARAM;
			pst_me_radio_mngr_inst->e_DABtoFMFollowingStatus 		= REPLYSTATUS_INVALID_PARAM;
			
			/*If DAB supported, Activated, RDS and DAB FM support w.r.t market*/
			if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && 
					pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE &&
					pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				AMFM_APP_Request_FM_to_DAB_Switch((Te_AMFM_App_FM_To_DAB_Switch)pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request);
				DAB_App_Request_EnableDABtoFMLinking((Te_DAB_App_DABFMLinking_Switch)pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request);
			}
			else
			{
				/*If market not supported then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}			
		}
		break;

		case RADIO_MNGR_APP_FM_DAB_SWITCH_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_FMtoDABFollowingStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			
			/*Wait for AMFM and DAB success response*/
			if(pst_me_radio_mngr_inst->e_FMtoDABFollowingStatus == REPLYSTATUS_SUCCESS && pst_me_radio_mngr_inst->e_DABtoFMFollowingStatus == REPLYSTATUS_SUCCESS)
			{
				/*Update the u8_Settings variable*/
				if(pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request == RADIO_MNGR_APP_DABFMLINKING_ENABLE)
				{
					pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_SETBIT(pst_me_radio_mngr_inst->u8_Settings, 1u));
					pst_me_radio_mngr_inst->e_DABFMLinking_Switch = RADIO_MNGR_APP_DABFMLINKING_ENABLE;
				}
				else
				{
					pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_CLEARBIT(pst_me_radio_mngr_inst->u8_Settings, 1u));					
					pst_me_radio_mngr_inst->e_DABFMLinking_Switch = RADIO_MNGR_APP_DABFMLINKING_DISABLE;
				}
				/*Send DAB<=>FM switch response to HMI*/
				Radio_Mngr_App_Response_FMDABSwitch(pst_me_radio_mngr_inst->e_FMtoDABFollowingStatus);
				
				/*Notify DAB<=>FM switch,Announcement switch,AF switch status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
				
				/*Function to update Flash Memory parameters*/
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				/*If Response Failure then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}
		}
		break;

		case RADIO_MNGR_APP_DABTOFM_LINKING_ENABLE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABtoFMFollowingStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			
			/*Wait for AMFM and DAB success response*/
			if(pst_me_radio_mngr_inst->e_FMtoDABFollowingStatus == REPLYSTATUS_SUCCESS && pst_me_radio_mngr_inst->e_DABtoFMFollowingStatus == REPLYSTATUS_SUCCESS)
			{
				/*Update the u8_Settings variable*/
				if(pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request == RADIO_MNGR_APP_DABFMLINKING_ENABLE)
				{
					pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_SETBIT(pst_me_radio_mngr_inst->u8_Settings, 1u));
					pst_me_radio_mngr_inst->e_DABFMLinking_Switch = RADIO_MNGR_APP_DABFMLINKING_ENABLE;
				}
				else
				{
					pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_CLEARBIT(pst_me_radio_mngr_inst->u8_Settings, 1u));
					pst_me_radio_mngr_inst->e_DABFMLinking_Switch = RADIO_MNGR_APP_DABFMLINKING_DISABLE;
				}
				/*Send DAB<=>FM switch response to HMI*/
				Radio_Mngr_App_Response_FMDABSwitch(pst_me_radio_mngr_inst->e_DABtoFMFollowingStatus);
				
				/*Notify DAB<=>FM switch,Announcement switch,AF switch status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
				
				/*Function to update Flash Memory parameters*/
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				/*If response failure then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Settings DAB<=>FM: %d, TA_Anno: %d, RDS: %d, Info_Anno: %d", 
								pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch,
								pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch);
			
			if(pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request == RADIO_MNGR_APP_DABFMLINKING_DISABLE && pst_me_radio_mngr_inst->e_DABFMLinking_Switch == RADIO_MNGR_APP_DABFMLINKING_DISABLE &&
						pst_me_radio_mngr_inst->e_FMtoDABFollowingStatus == REPLYSTATUS_SUCCESS &&
						pst_me_radio_mngr_inst->e_DABtoFMFollowingStatus == REPLYSTATUS_SUCCESS &&
						pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
			{
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
				{
					AMFM_App_Request_Cancel();
				}
				else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
				{
					/*Cancel Request to the DAB App layer for the Current Execution */
					DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
				}
				else{/*FOR MISRA C*/}
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_TA_ANNO_SWITCH_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request), (pst_msg->data), sizeof(Te_Radio_Mngr_App_EnableTAAnno_Switch), &(pst_me_radio_mngr_inst->u32_slot));
			
			pst_me_radio_mngr_inst->e_TA_Anno_Switch = pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request;
			pst_me_radio_mngr_inst->e_AMFM_TA_Switch_Status = REPLYSTATUS_INVALID_PARAM;
			pst_me_radio_mngr_inst->e_DAB_TA_Switch_Status 	= REPLYSTATUS_INVALID_PARAM;

			 AMFM_APP_Request_TASwitch((Te_AMFM_App_TA_Switch)pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request);
				
			/*Announcement switch request for DAB application if the system support DAB and Market also need to support*/
			if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && 
				 pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
			{
				if(pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request == RADIO_MNGR_APP_TA_ANNO_ENABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch == RADIO_MNGR_APP_INFO_ANNO_DISABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_TA_ON_INFO_OFF);
				}
				else if(pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request == RADIO_MNGR_APP_TA_ANNO_DISABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch == RADIO_MNGR_APP_INFO_ANNO_ENABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_TA_OFF_INFO_ON);
				}
				else if(pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request == RADIO_MNGR_APP_TA_ANNO_ENABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch == RADIO_MNGR_APP_INFO_ANNO_ENABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_TA_ON_INFO_ON);
				}
				else if(pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request == RADIO_MNGR_APP_TA_ANNO_DISABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch == RADIO_MNGR_APP_INFO_ANNO_DISABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_VALUE_ZERO);
				}
				else{/*FOR MISRA C*/}
			}
			else
			{
				/*If market and Band not supported then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}
		}
		break;
		
		case RADIO_MNGR_APP_INFO_ANNO_SWITCH_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_Info_Anno_Switch_Request), (pst_msg->data), sizeof(Te_Radio_Mngr_App_EnableInfoAnno_Switch), &(pst_me_radio_mngr_inst->u32_slot));	
			
			pst_me_radio_mngr_inst->e_Info_Switch_Status = REPLYSTATUS_INVALID_PARAM;
			
			/*Setting the Info Anno flag which can be used to distinguish response of TA or Info Announcement*/
			pst_me_radio_mngr_inst->b_InfoAnnoFlag = RADIO_MNGR_APP_INFO_ANNO_REQ;
			
			/*Announcement switch request for DAB application if the system support DAB and Market also need to support*/
			if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && 
					pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
			{
				if(pst_me_radio_mngr_inst->e_TA_Anno_Switch == RADIO_MNGR_APP_TA_ANNO_ENABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch_Request == RADIO_MNGR_APP_INFO_ANNO_DISABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_TA_ON_INFO_OFF);
				}
				else if(pst_me_radio_mngr_inst->e_TA_Anno_Switch == RADIO_MNGR_APP_TA_ANNO_DISABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch_Request == RADIO_MNGR_APP_INFO_ANNO_ENABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_TA_OFF_INFO_ON);
				}
				else if(pst_me_radio_mngr_inst->e_TA_Anno_Switch == RADIO_MNGR_APP_TA_ANNO_ENABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch_Request == RADIO_MNGR_APP_INFO_ANNO_ENABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_TA_ON_INFO_ON);
				}
				else if(pst_me_radio_mngr_inst->e_TA_Anno_Switch == RADIO_MNGR_APP_TA_ANNO_DISABLE && pst_me_radio_mngr_inst->e_Info_Anno_Switch_Request == RADIO_MNGR_APP_INFO_ANNO_DISABLE)
				{
					DAB_App_Request_SetAnnoConfig(RADIO_MNGR_APP_VALUE_ZERO);
				}
				else{/*FOR MISRA C*/}
			}
			else
			{
				/*If market and Band not supported then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}
		}
		break;

		case RADIO_MNGR_APP_TA_SWITCH_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_AMFM_TA_Switch_Status), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			
			/*If the system is not supported DAB varient,wait for only AMFM success response then send to HMI (or) Wait for AMFM and DAB success response then send to HMI*/
			if( (pst_me_radio_mngr_inst->e_AMFM_TA_Switch_Status == REPLYSTATUS_SUCCESS && (pst_me_radio_mngr_inst->b_DAB_BandStatus != RADIO_MANAGER_DAB_BAND_SUPPORTED || pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus != RADIO_MNGR_APP_SRC_ACTIVE))|| 
				(pst_me_radio_mngr_inst->e_AMFM_TA_Switch_Status == REPLYSTATUS_SUCCESS && pst_me_radio_mngr_inst->e_DAB_TA_Switch_Status == REPLYSTATUS_SUCCESS) )
			{
				/*Update the u8_Settings variable*/
				if(pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request == RADIO_MNGR_APP_TA_ANNO_ENABLE)
				{
					pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_SETBIT(pst_me_radio_mngr_inst->u8_Settings, 0u));
					pst_me_radio_mngr_inst->e_TA_Anno_Switch = RADIO_MNGR_APP_TA_ANNO_ENABLE;
				}
				else
				{
					pst_me_radio_mngr_inst->u8_Settings 		= (Tu8)(LIB_CLEARBIT(pst_me_radio_mngr_inst->u8_Settings, 0u));
					pst_me_radio_mngr_inst->e_TA_Anno_Switch 	= RADIO_MNGR_APP_TA_ANNO_DISABLE;
				}
				/*Send Announcement switch response to HMI*/
				Radio_Mngr_Response_TA_Anno_Switch(pst_me_radio_mngr_inst->e_AMFM_TA_Switch_Status);
				
				/*Notify DAB<=>FM switch,Announcement switch,AF switch status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
				
				/*Function to update Flash Memory parameters*/
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				/*If response fail then we are notifying older Setting status to HMI*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
			}
		}
		break;

		case RADIO_MNGR_APP_DAB_ANNO_SWITCH_RESID:
		{
			if (pst_me_radio_mngr_inst->b_InfoAnnoFlag == RADIO_MNGR_APP_INFO_ANNO_REQ)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_Info_Switch_Status), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));	
				
				pst_me_radio_mngr_inst->b_InfoAnnoFlag = RADIO_MNGR_APP_CLEAR_INFO_ANNO_FLAG;
				
				if(pst_me_radio_mngr_inst->e_Info_Switch_Status == REPLYSTATUS_SUCCESS)
				{
					/*Update the u8_Settings variable*/
					if(pst_me_radio_mngr_inst->e_Info_Anno_Switch_Request == RADIO_MNGR_APP_INFO_ANNO_ENABLE)
					{
						pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_SETBIT(pst_me_radio_mngr_inst->u8_Settings, 4u));
						pst_me_radio_mngr_inst->e_Info_Anno_Switch = RADIO_MNGR_APP_INFO_ANNO_ENABLE;
					}
					else
					{
						pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_CLEARBIT(pst_me_radio_mngr_inst->u8_Settings, 4u));
						pst_me_radio_mngr_inst->e_Info_Anno_Switch = RADIO_MNGR_APP_INFO_ANNO_DISABLE;
					}
					/*Send Info Announcement Success switch response to HMI*/
					Radio_Mngr_Response_Info_Anno_Switch(pst_me_radio_mngr_inst->e_Info_Switch_Status);
				
					/*Notify DAB<=>FM switch,Announcement switch(Info & TA),AF switch status to HMI*/
					Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
					
					/*Function to update Flash Memory parameters*/
					Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
				}
				else
				{
					/*If response fial then we are notifying older Setting status to HMI*/
					Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
				}
			}
			else
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DAB_TA_Switch_Status), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));

				/*Wait for AMFM and DAB success response*/
				if((pst_me_radio_mngr_inst->e_AMFM_TA_Switch_Status == REPLYSTATUS_SUCCESS) && (pst_me_radio_mngr_inst->e_DAB_TA_Switch_Status == REPLYSTATUS_SUCCESS))
				{
					/*Update the u8_Settings variable*/
					if(pst_me_radio_mngr_inst->e_TA_Anno_Switch_Request == RADIO_MNGR_APP_TA_ANNO_ENABLE)
					{
						pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_SETBIT(pst_me_radio_mngr_inst->u8_Settings, 0u));
						pst_me_radio_mngr_inst->e_TA_Anno_Switch = RADIO_MNGR_APP_TA_ANNO_ENABLE;
					}
					else
					{
						pst_me_radio_mngr_inst->u8_Settings = (Tu8)(LIB_CLEARBIT(pst_me_radio_mngr_inst->u8_Settings, 0u));
						pst_me_radio_mngr_inst->e_TA_Anno_Switch = RADIO_MNGR_APP_TA_ANNO_DISABLE;
					}
					/*Send Announcement switch response to HMI*/
					Radio_Mngr_Response_TA_Anno_Switch(pst_me_radio_mngr_inst->e_DAB_TA_Switch_Status);
				
					/*Notify DAB<=>FM switch,Announcement switch,AF switch status to HMI*/
					Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
					
					/*Function to update Flash Memory parameters*/
					Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
				}
				else
				{
					/*If response fail then we are notifying older Setting status to HMI*/
					Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
				}
			}
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Settings DAB<=>FM: %d, TA_Anno: %d, RDS: %d, Info_Anno: %d", 
								pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch,
								pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch);
		}
		break;
		
		case RADIO_MNGR_APP_MULTIPLEX_SWITCH_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_MultiplexSettings),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus),&(pst_me_radio_mngr_inst->u32_slot));
			
			if(pst_me_radio_mngr_inst->e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_ENABLE)
			{
				pst_me_radio_mngr_inst->u8_Settings   = (Tu8)(pst_me_radio_mngr_inst->u8_Settings | (0X20));	
			}
			else if(pst_me_radio_mngr_inst->e_MultiplexSettings == RADIO_MNGR_APP_MULTIPLEX_DISABLE)
			{
				pst_me_radio_mngr_inst->u8_Settings   = (Tu8)(pst_me_radio_mngr_inst->u8_Settings & (~(0X20)));
			}else{/*FOR MISRA C*/}
			
			/*Send Multiplex switch response to HMI*/
			pst_me_radio_mngr_inst->e_MultiplexSettings_ReplyStatus = REPLYSTATUS_SUCCESS;
			Radio_Mngr_App_Response_Multiplex_Switch_Status(pst_me_radio_mngr_inst->e_MultiplexSettings_ReplyStatus);
		
			/*Notify DAB<=>FM switch,Announcement switch,AF switch status to HMI*/
			Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr_inst->e_DABFMLinking_Switch, pst_me_radio_mngr_inst->e_TA_Anno_Switch, pst_me_radio_mngr_inst->e_RDSSettings, pst_me_radio_mngr_inst->e_Info_Anno_Switch, pst_me_radio_mngr_inst->e_MultiplexSettings);
		
			/*Function to update Flash Memory parameters*/
			Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
		}
		break;
		
        /*Announcement cancel request from HMI*/
		case RADIO_MNGR_APP_ANNO_CANCEL_REQID:
		{
			switch(pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_FM:
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM Announcement Cancel Requested from HMI");
					
                    /*Updating Announcement cancel type by HMI*/
					pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_HMI;
					AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request) pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{	
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB Announcement Cancel Requested from HMI");
					
					/*Updating Announcement cancel type by HMI*/
					pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_HMI;
					
                    /*Calling announcement cancel function for DAB*/
					DAB_App_Request_AnnoCancel();
				}
				break;

				default:
				break;
			}
		}
		break;

		case RADIO_MNGR_APP_AMFM_ANNO_CANCEL_RESID:
		case RADIO_MNGR_APP_DAB_ANNO_CANCEL_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Announcement Ended");
			
            /*Extracting the Announcement cancel response*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			pst_me_radio_mngr_inst->e_Anno_Status = RADIO_MNGR_APP_ANNO_END;
			Radio_Mngr_App_Response_AnnoCancel(pst_me_radio_mngr_inst->e_ReplyStatus);
		}
		break;

		case RADIO_MNGR_APP_DAB_RECONFIGURATION_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DAB Reconfiguration Received");
			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DABTunableStation), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_DAB_Tunable_StationInfo), &(pst_me_radio_mngr_inst->u32_slot));
			pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SERVICE_RECONFIG;
			Update_TunuableStation_with_DAB_Station(pst_me_radio_mngr_inst);
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
		}
		break;

		
		
/******************************FM to DAB Following***************************************************/

		/*FM=>DAB finding the SID station for Same PI*/
		case RADIO_MNGR_APP_AMFM_FIND_SID_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u16_FMtoDAB_PI),(pst_msg->data), sizeof(Tu16),&(pst_me_radio_mngr_inst->u32_slot));

            if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE &&
				  pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END &&
				  pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]PI Notified to DAB");
			    DAB_App_Request_GetSIDStation(pst_me_radio_mngr_inst->u16_FMtoDAB_PI);
            }else{/*FOR MISRA C*/}
		}
		break;

		/*DAB=>RM After Finding Same PI station, current station of requested PI*/
		case RADIO_MNGR_APP_FMDAB_SID_STATION_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
			Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
		}
		break;

		/*DAB=>FM For Same PI, Quality and SID Type*/
		case RADIO_MNGR_APP_FMDAB_SID_QUALITY_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_FMtoDAB_SID_Quality),(pst_msg->data), sizeof(Tu8),&(pst_me_radio_mngr_inst->u32_slot));

			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_FMDAB_SID_Type),(pst_msg->data), sizeof(Te_Radio_Mngr_App_DABFM_SID_Type),&(pst_me_radio_mngr_inst->u32_slot));

			if(pst_me_radio_mngr_inst->e_FMDAB_SID_Type == RADIO_MNGR_APP_FMDAB_BLENDING_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM] FM to DAB Done Notification Rxed by DAB");
				
				pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STARTED;
				
				Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
				
				/*Updating DAB Linked station Info on HMI with active band as FM*/
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
			
				/*clearing timer flag & Stopping timer*/
				Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
			
			}
			else
			{
				AMFM_App_Notify_FMtoDAB_SIDStaion_Quality(pst_me_radio_mngr_inst->u8_FMtoDAB_SID_Quality, (Te_AMFM_DAB_PI_TYPE)pst_me_radio_mngr_inst->e_FMDAB_SID_Type);
			}
		}
		break;

		case RADIO_MNGR_APP_START_FM_DAB_FOLLOWUP_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM] FM to DAB followed Requested to DAB");
		
			DAB_App_Notify_Init_FMDAB_linking();
			
		}
		break;
		
		case RADIO_MNGR_APP_STOP_FM_DAB_LINKING_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM] FM to DAB Linking Stopped");		
			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_FMtoDABLinkingStopType),(pst_msg->data), sizeof(Te_Radio_Mngr_App_FMDABLinking_Stop_Type), &(pst_me_radio_mngr_inst->u32_slot));
			
			pst_me_radio_mngr_inst->e_FMDAB_SID_Type = RADIO_MNGR_APP_FMDAB_BLENDING_CANCELLED;
			
			if(pst_me_radio_mngr_inst->e_Curr_Audio_Band != pst_me_radio_mngr_inst->e_activeBand)
			{
				Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
			
			
			
				/*Displaying FM information to HMI if FM-DAB is stopped*/
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																		(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}
			
			/*Notifying FM-DAB stop linking type to DAB App*/
			DAB_App_Notify_FMDAB_Stop_Type((Te_Dab_App_FmtoDAB_Reqstatus)pst_me_radio_mngr_inst->e_FMtoDABLinkingStopType);
			
		}
		break;

		case RADIO_MNGR_APP_AF_STATUS_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_AF_Status),(pst_msg->data), sizeof(Te_Radio_Mngr_App_AF_Status), &(pst_me_radio_mngr_inst->u32_slot));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM AF Status: %d", pst_me_radio_mngr_inst->e_AF_Status);
			
			if(pst_me_radio_mngr_inst->e_AF_Status == RADIO_MNGR_APP_AF_LINK_ESTABLISHED)
			{
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_AF_SWITCHING_ESTABLISHED;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
			}
			Radio_Mngr_App_Notify_AFStatus(pst_me_radio_mngr_inst->e_AF_Status);
		}
		break;
		
/******************************FM to DAB Following END***************************************************/

		case RADIO_MNGR_APP_DAB_DAB_STATUS_NOTIFYID:
		{			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DAB_ALT_Status),(pst_msg->data), sizeof(Te_Radio_Mngr_App_DAB_Alt_Status), &(pst_me_radio_mngr_inst->u32_slot));

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB AF Status: %d", pst_me_radio_mngr_inst->e_DAB_ALT_Status);
			
			if(pst_me_radio_mngr_inst->e_DAB_ALT_Status == RADIO_MNGR_APP_DAB_ALT_TUNE_STARTED)
			{
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DAB_DAB_STARTED;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
			}
			else if(pst_me_radio_mngr_inst->e_DAB_ALT_Status == RADIO_MNGR_APP_DAB_ALT_TUNE_SUCCESS)
			{
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DAB_DAB_LINKING_DONE;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
			}else{/*FOR MISRA C*/}
			
			if(pst_me_radio_mngr_inst->e_DAB_ALT_Status == RADIO_MNGR_APP_DAB_ALT_TUNE_SUCCESS)
			{
				Radio_Manager_App_Update_PresetMixedList_AFTune(pst_me_radio_mngr_inst);
			}
			
		}
		break;

/*********************************************Componenet Status Start****************************************************************************/
		/*AMFMTuner Abnormal Notification*/
		case RADIO_MNGR_APP_AMFMTUNER_ABNORMAL_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_AMFMTunerStatus), (pst_msg->data), sizeof(Te_RADIO_AMFMTuner_Status), &(pst_me_radio_mngr_inst->u32_slot));

			/*Debug Log for Radio Manager receiving POR*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT,"[Radio][RM]RADIO RM POWER ON RESET OCCURED");

			/*Notifying to DAB Layers about AMFM TUNER Abnormal Status*/
			DAB_App_Notify_AMFMTunerStatus((Te_RADIO_Comp_Status)RADIO_FRMWK_COMP_STATUS_ABNORMAL);

			/*Notifying to system- AMFMTuner abnormal status*/
			Radio_Mngr_App_Notify_Components_Status(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_AMFMTunerStatus, 
															pst_me_radio_mngr_inst->e_DABTunerStatus, pst_me_radio_mngr_inst->e_DAB_UpNot_Status);
		}
		break;

		/*After reset the AMFMTuner Audio Configurations, req got from sys for Tune to AM/FM lsm station*/
		case RADIO_MNGR_APP_AMFM_RETUNE_REQID:
		{
			pst_me_radio_mngr_inst->e_AMFMTunerStatus = RADIO_FRMWK_AMFMTUNER_NORMAL;

			/*Debug Log for Radio Manager receiving AMFMTuner Normal Status*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]RADIO RADIO MANAGER AMFM TUNER NORMAL");


			/*Notifying to DAB Layers about AMFM TUNER Normal Status after AMFM TUNER Reset*/
			DAB_App_Notify_AMFMTunerStatus((Te_RADIO_Comp_Status)RADIO_FRMWK_COMP_STATUS_NORMAL);

			/*Notifying to system- AMFMTuner normal status from Abnormal*/
			Radio_Mngr_App_Notify_Components_Status(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_AMFMTunerStatus, 
															pst_me_radio_mngr_inst->e_DABTunerStatus, pst_me_radio_mngr_inst->e_DAB_UpNot_Status);

			/*Tune to LSM for AM or FM only when active band is AM/FM*/
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				/*clear the check parametres*/
				Radio_Mngr_App_ClearCheckParameters(pst_me_radio_mngr_inst);
			
				Update_Tunable_Station_Info_with_LSM(pst_me_radio_mngr_inst);
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_SELECT_STATION;
			
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}
			else
			{
				/*if AMFM Tuner is in background then no need to tune for LSM.*/
			}
		}
		break;

		/*DABTuner Abnormal Notification*/
		case RADIO_MNGR_APP_DABTUNER_ABNORMAL_NOTIFYID:
		{
			pst_me_radio_mngr_inst->e_DABTunerStatus = RADIO_FRMWK_COMP_STATUS_ABNORMAL;
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT,"[Radio][RM]DABTUNER ABNORMAL");

			/*Notifying to AMFM Layers about DABTUNER Abnormal Status*/
			AMFM_App_Notify_DABStatus((Te_RADIO_Comp_Status) pst_me_radio_mngr_inst->e_DABTunerStatus);
			memset(&(pst_me_radio_mngr_inst->st_DLS_Data), 0, sizeof(Ts_Radio_Mngr_App_DLS_Data));
			memset(pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel, 0, RADIO_MNGR_APP_COMPONENT_LABEL); 	

			/*Updating activity status as DAB Tuner abnormal and sending to system*/
			pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DABTUNER_ABNORMAL;
			Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);

			/*Notifying to system- DABTuner abnormal status*/
			Radio_Mngr_App_Notify_Components_Status(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_AMFMTunerStatus, pst_me_radio_mngr_inst->e_DABTunerStatus, 
														pst_me_radio_mngr_inst->e_DAB_UpNot_Status);

			/*Requesting to DAB layers to restart the DABTuner*/
			DAB_App_Request_DABTunerRestart();
		}
		break;

		/*DABTuner Abnormal Notification*/
		case RADIO_MNGR_APP_DABTUNER_RESTART_DONE_RESIID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABTunerRestartReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));

			/*check for DAB Tuner restart done successfully or not*/
			if(pst_me_radio_mngr_inst->e_DABTunerRestartReplyStatus == REPLYSTATUS_SUCCESS)
			{
				pst_me_radio_mngr_inst->e_DABTunerStatus = RADIO_FRMWK_COMP_STATUS_NORMAL;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DABTUNER NORMAL");

				/*Notifying to AMFM Layers about DABTUNER Normal Status*/
				AMFM_App_Notify_DABStatus((Te_RADIO_Comp_Status) pst_me_radio_mngr_inst->e_DABTunerStatus);

				/*Notifying to system- DABTuner normal status from Abnormal*/
				Radio_Mngr_App_Notify_Components_Status(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_AMFMTunerStatus, 
																pst_me_radio_mngr_inst->e_DABTunerStatus, pst_me_radio_mngr_inst->e_DAB_UpNot_Status);

				/*Tune to LSM for DAB only when active band is DAB*/
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
				{
					/*clear the check parametres*/
					Radio_Mngr_App_ClearCheckParameters(pst_me_radio_mngr_inst);
			
					Update_Tunable_Station_Info_with_LSM(pst_me_radio_mngr_inst);
					pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_SELECT_STATION;
			
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
				}
				else
				{
					/*if DABTuner is in background then no need to tune for LSM.*/
				}
			}
			else
			{
				/*Error Handling need to take care if DAB Tuner restart fail*/
			}
		}
		break;
/*********************************************Componenet Status End****************************************************************************/

		case RADIO_MNGR_APP_ENG_MODE_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_EngMode_Switch), (pst_msg->data), sizeof(Te_Radio_Mngr_App_Eng_Mode_Request), &(pst_me_radio_mngr_inst->u32_slot));

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]ENG MODE:%d", pst_me_radio_mngr_inst->e_EngMode_Switch);
			
			/* Requesting AMFM App layer for ENG mode */
			AMFM_App_Request_ENG_Mode((Te_AMFM_App_Eng_Mode_Switch)pst_me_radio_mngr_inst->e_EngMode_Switch);
			
			/*For DAB Supported only, requesting for ENG mode*/
			if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
			{
				DAB_App_Request_ENG_Mode((Te_DAB_APP_Eng_Mode_Request)pst_me_radio_mngr_inst->e_EngMode_Switch);
			}else{/*FOR MISRA C*/}		
		}
		break;

		case RADIO_MNGR_APP_GETCLOCKTIME_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]CT Requested");
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				AMFM_App_Request_GetCT_Info();
			}
			else
			{
				/*Only In ENG mode ON and Active Band is FM case we have to provide CT Info*/
				Radio_Mngr_App_Response_ClockTime(REPLYSTATUS_FAILURE);
			}
		}
		break;

		case RADIO_MNGR_APP_AMFM_CT_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));

			/*if CT is available in APP layer then only have to copy with referece of reply status*/
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFMTUNER_CTInfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_CT_Info), &(pst_me_radio_mngr_inst->u32_slot));

				Radio_Mngr_App_Response_ClockTime(REPLYSTATUS_SUCCESS);
				Radio_Mngr_App_Notify_ClockTime(&pst_me_radio_mngr_inst->st_AMFMTUNER_CTInfo);
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]CT Notified");
			}
			else
			{
				Radio_Mngr_App_Response_ClockTime(REPLYSTATUS_FAILURE);
			}	
		}
		break;

/*********************************************SRC Activate De-Activate Start****************************************************************************/
		case RADIO_MNGR_APP_SRC_ACTIVATE_DEACTIVATE_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SRC_ActivateDeactivate_Band), (pst_msg->data), sizeof(Te_Radio_Mngr_App_Band),              &(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SRCActivateDeactivate),            (pst_msg->data), sizeof(Te_Radio_Mngr_App_SRC_ActivateDeActivate), &(pst_me_radio_mngr_inst->u32_slot));

			/*AM Activate/Deactive shall handle only if AM in Background(InAcvtive Band)*/
			if(pst_me_radio_mngr_inst->e_SRC_ActivateDeactivate_Band == RADIO_MNGR_APP_BAND_AM)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]SRC AM Activate/De-Activate Request Received");
				
				if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_AM)
				{
					if(pst_me_radio_mngr_inst->e_SRCActivateDeactivate == RADIO_MNGR_APP_SRC_ACTIVE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]SRC AM Activated");
						
						/*Updating the AM Active/Deactive Status Parameter, which can be used in Select AM Band*/
						pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus = RADIO_MNGR_APP_SRC_ACTIVE;
					}
					else if(pst_me_radio_mngr_inst->e_SRCActivateDeactivate == RADIO_MNGR_APP_SRC_DEACTIVE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]SRC AM De-Activated");
						
						/*Updating the AM Active/Deactive Status Parameter, which can be used in Select AM Band*/
						pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus = RADIO_MNGR_APP_SRC_DEACTIVE;
					}else{/*FOR MISRA C*/}

					/*Response for DAB Activate/Deactivate Status to the system*/
					Sys_Response_SRC_Activate_DeActivate(REPLYSTATUS_SUCCESS);
				}	
				else
				{
					/*Response for DAB Activate/Deactivate Status to the system*/
				}				
			}
			
			/*DAB Activate/Deactive shall handle only if DAB is in Background(InAcvtive Band)*/
			else if(pst_me_radio_mngr_inst->e_SRC_ActivateDeactivate_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]SRC DAB Activate/De-Activate Request Received");
				
				if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED)
				{
					if(pst_me_radio_mngr_inst->e_SRCActivateDeactivate == RADIO_MNGR_APP_SRC_ACTIVE)
					{
						/*Updating the DAB Active/Deactive Status Parameter, which can be used in Select DAB Band*/
						pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus = RADIO_MNGR_APP_SRC_ACTIVE;
						
						/*Requesting DAB Application for Activate*/
						DAB_App_Request_Activate_Deactivate((Te_DAB_App_ActivateDeactivateStatus)pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus);
					}
					else if(pst_me_radio_mngr_inst->e_SRCActivateDeactivate == RADIO_MNGR_APP_SRC_DEACTIVE)
					{
						/*Updating the DAB Active/Deactive Status Parameter, which can be used in Select DAB Band*/
						pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus = RADIO_MNGR_APP_SRC_DEACTIVE;
						
						/*Requesting DAB Application for deactivate*/
						DAB_App_Request_Activate_Deactivate((Te_DAB_App_ActivateDeactivateStatus)pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus);
						
					}else{/*FOR MISRA C*/}
				}	
				else
				{
					/*Response for DAB Activate/Deactivate Status to the system*/
					Sys_Response_SRC_Activate_DeActivate(REPLYSTATUS_FAILURE);
				}				
			}
			else{/*FOR MISRA C*/}
		}
		break;
		
		case RADIO_MNGR_APP_DAB_ACTIVATE_DEACTIVATE_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				if(pst_me_radio_mngr_inst->e_SRCActivateDeactivate == RADIO_MNGR_APP_SRC_ACTIVE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]SRC DAB Activated");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]SRC DAB DeActivated");
				}
				/*Notifying DAB Activate Status to the AMFM Application for FM-DAB Follow up support*/
				AMFM_App_Notify_DABStatus((Te_RADIO_Comp_Status)pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus);
				
				/*Response for DAB Activate/Deactivate Status to the system*/
				Sys_Response_SRC_Activate_DeActivate(REPLYSTATUS_SUCCESS);
			}
			else
			{
				/*Response for DAB Activate/Deactivate Status to the system*/
				Sys_Response_SRC_Activate_DeActivate(REPLYSTATUS_FAILURE);
			}	
		}
		break;
/*********************************************SRC Activate De-Activate End****************************************************************************/

		/*FM Active Band Updated learn memory AF Status*/
		case RADIO_MNGR_APP_FM_AF_LEARN_MEM_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_LearnAFStatus), (pst_msg->data), sizeof(Te_Radio_Mngr_App_LearnMemAFStatus), &(pst_me_radio_mngr_inst->u32_slot));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM Learn Memory AFStatus: %d", pst_me_radio_mngr_inst->e_LearnAFStatus);

			if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_SUCCESS)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_AMFM_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));

				Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
				/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
				pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																										&(pst_me_radio_mngr_inst->st_LSM_Station_Info));																										
				Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);

				Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
				
				Radio_Manager_App_Update_PresetMixedList_AFTune(pst_me_radio_mngr_inst);

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
			}
			else
			{
				/*FM FM_AF fail, FM_DAB_AF Fail then FM FM_Learn_AF also fail or FM FM AF Fail and D<=>F off then FM Learn AF Fail*/
				if(pst_me_radio_mngr_inst->e_Curr_Audio_Band != RADIO_MNGR_APP_BAND_DAB && 
								(pst_me_radio_mngr_inst->e_DAB_AFTuneReplyStatus != REPLYSTATUS_INVALID_PARAM || 
								pst_me_radio_mngr_inst->e_DABFMLinking_Switch == RADIO_MNGR_APP_DABFMLINKING_DISABLE))
				{
					Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
					/*Tune to Original Station*/
					Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
				}else{/*Do Nothing wait for DAB AF response because FM 2ndAF also fail and DBA AF not yer received*/}
			}
		}
		break;

		/*DAB Active Band Updated learn memory AF Status*/
		case RADIO_MNGR_APP_DAB_AF_LEARN_MEM_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_LearnAFStatus), (pst_msg->data), sizeof(Te_Radio_Mngr_App_LearnMemAFStatus), &(pst_me_radio_mngr_inst->u32_slot));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB Learn Memory AFStatus: %d", pst_me_radio_mngr_inst->e_LearnAFStatus);

			if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_SUCCESS)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_DAB_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
				Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));

				Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
				/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
				pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																										&(pst_me_radio_mngr_inst->st_LSM_Station_Info));																										
				Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);

				Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
			}
			else
			{
				/*DAB DAB_AF fail, DAB_FM_AF Fail then DAB DAB_Learn_AF also fail or DAB DAB AF Fail and D<=>F off then DAB Learn AF Fail*/
				if(pst_me_radio_mngr_inst->e_Curr_Audio_Band != RADIO_MNGR_APP_BAND_FM && 
								(pst_me_radio_mngr_inst->e_FM_AFTuneReplyStatus != REPLYSTATUS_INVALID_PARAM || 
								pst_me_radio_mngr_inst->e_DABFMLinking_Switch == RADIO_MNGR_APP_DABFMLINKING_DISABLE))
				{
					Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
					/*Tune to Original Station*/
					Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
				}else{/*Do Nothing wait for FM AF response because DAB 2ndAF also fail and FM AF not yet received*/}
			}
		}
		break;

		/*FM BG AF Signal Low Notification*/
		case RADIO_MNGR_APP_FM_AF_SIGLOST_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM AF Signal Low Received");
			
			/*DAB DAB AF Learn memory already fail*/
			if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_FAIL)
			{
				Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
			}
			/*DAB DAB AF Fail, DAB FM AF Success, then got SigLost but DAB AF Learn memory status not yet received*/
			else if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_INVALID)
			{

				Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);

				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_DAB,
																				pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																				pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																				pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																				pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																				pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																				pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);

			    pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_IN_STRATEGY;
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state); 
			}else{/*FOR MISRA C*/}
		}
		break;

		/*DAB BG AF Signal Low Notification*/
		case RADIO_MNGR_APP_DAB_AF_SIGLOST_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB AF Signal Low Received");
			
			/*FM FM AF Learn memory already fail*/
			if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_FAIL)
			{
				Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);

			}
			/*FM AF Learn memory status not yet received*/
			else if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_INVALID)
			{
				Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);

				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																		(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);

			    pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_IN_STRATEGY;
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state); 

			}else{/*FOR MISRA C*/}
		}
		break;
		
		case RADIO_MNGR_APP_DAB_STATIONLIST_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus),&(pst_me_radio_mngr_inst->u32_slot));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB StationListReplayStatus: %d", pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus);	
		}
		break;

		case RADIO_MNGR_APP_FM_STATIONLIST_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus),&(pst_me_radio_mngr_inst->u32_slot));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM StationListReplayStatus: %d", pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus);
		}
		break;
		
		case RADIO_MNGR_APP_INST_HSM_FACTORY_RESET_REQID:
		{
			Radio_Mngr_App_Response_InstHSMFactoryReset();
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_inactive_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveStartHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveStartHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	
	PRINT_MSG_DATA(pst_msg);

	switch (pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveStartHndlr");
			/*Reset the all Reply station variables and active band variable with invalid or negative values for better debugging purposes*/
		    pst_me_radio_mngr_inst->e_activeBand					= RADIO_MNGR_APP_BAND_INVALID;


			pst_me_radio_mngr_inst->e_SelectStationReplyStatus 		= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus  = REPLYSTATUS_FAILURE;
			pst_me_radio_mngr_inst->e_GetAMstationlistreplystatus 	= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus 	= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr_inst->e_SeekReplyStatus 				= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr_inst->e_Anno_Status					= RADIO_MNGR_APP_ANNO_INVALID;
			pst_me_radio_mngr_inst->e_Anno_Status_Type				= RADIO_MNGR_APP_ANNO_INVALID;
			pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type		= RADIO_MNGR_APP_ANNOCANCEL_INVALID;
			pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id 		= RADIO_MNGR_APP_PLAY_SELECT_STATION;
			pst_me_radio_mngr_inst->e_ScanCancel_Request_Type		= RADIO_MNGR_APP_SCAN_CANCEL_INVALID;
			pst_me_radio_mngr_inst->e_ReplyStatus 					= REPLYSTATUS_FAILURE;

            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveStartHndlr \n",
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveStartHndlr \n"));
			
			/*sending response to the main HSM as instance HSM started */
			Radio_Mngr_App_Inst_Hsm_Start_Response(REPLYSTATUS_SUCCESS);

			/*After Inst HSM Starts, Do the Select Band FM by trasit to Select Band State*/
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveIdleHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveIdleHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveIdleHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveIdleHndlr \n" ,sizeof("Radio_Mngr_App_Inst_Hsm_ActiveIdleHndlr \n"));

			Radio_Mngr_App_ClearCheckParameters(pst_me_radio_mngr_inst);
			
			/*Checking if any one of the cancel status is true i.e either Scan or seek or AF tune, if set, do internal msg post and clear the flags*/
			if(pst_me_radio_mngr_inst->b_Scan_Cancel_Status == RADIO_MNGR_APP_SCAN_CANCELLED || pst_me_radio_mngr_inst->b_Seek_Cancel_Status == RADIO_MNGR_APP_SEEK_CANCELLED ||
				 pst_me_radio_mngr_inst->b_AF_Tune_Cancel_Status == RADIO_MNGR_APP_AF_TUNE_CANCELLED || pst_me_radio_mngr_inst->b_Tune_Cancel_Status == RADIO_MNGR_APP_TUNE_CANCELLED)
			{
				pst_me_radio_mngr_inst->b_Scan_Cancel_Status		= RADIO_MNGR_APP_UINT8_ZERO;
				pst_me_radio_mngr_inst->b_Seek_Cancel_Status		= RADIO_MNGR_APP_UINT8_ZERO;
				pst_me_radio_mngr_inst->b_AF_Tune_Cancel_Status		= RADIO_MNGR_APP_UINT8_ZERO;
				pst_me_radio_mngr_inst->b_Tune_Cancel_Status		= RADIO_MNGR_APP_UINT8_ZERO;

				/*Calling internal message posting function*/
				Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
			}
			else{/*FOR MISRA C*/}
		}
		break;
		
		/*Request From HMI - Select the Station from Station List of Active Band by Index*/
		case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Station List Selection Requested");
			
			if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB || pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				/* Updating the requested station index into the inst hsm structure*/
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Index),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->u8_Index),&(pst_me_radio_mngr_inst->u32_slot));
				
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && 
						pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].u32_frequency == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency &&
						pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].u16_PI == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI)
				{
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																	(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
				}
				else
				{
					/*clearing timer flag & Stopping timer*/
					Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);

					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList) ||
						 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList) ||
						 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Index < pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList) )
					{
						pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
		
						/*Update the Active Band Tunable Station Info Struvcture with the selected Index station from the Station list*/
						Update_StationInfo_with_index(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->u8_Index);
				
						/*Updating the Req_ID enum with the RADIO_MNGR_APP_STATIONLIST_SELECT which can help to decide in Mute state and Station List State */
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_STATIONLIST_SELECT;
						
						Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
				
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
					}else{/*FOR MISRA C*/}
				}				
			}else{/*FOR MISRA C*/}
		}
		break;

		/*Request From HMI - Select the Band/Change the Band */
		case RADIO_MNGR_APP_SELECTBAND_REQID:
		{		
			/*Updating the requested Band into the inst hsm structure*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_requestedBand),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_requestedBand),&(pst_me_radio_mngr_inst->u32_slot));

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Select Band Requested for %d", pst_me_radio_mngr_inst->e_requestedBand);	
			
			if(pst_me_radio_mngr_inst->e_requestedBand != pst_me_radio_mngr_inst->e_activeBand)
			{	
				if((pst_me_radio_mngr_inst->e_requestedBand != RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB) ||
					    	(pst_me_radio_mngr_inst->e_requestedBand != RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB) ||
							(pst_me_radio_mngr_inst->e_requestedBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB 
							      && pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL))
				{
					switch(pst_me_radio_mngr_inst->e_requestedBand)
					{
						case RADIO_MNGR_APP_BAND_AM:
						{
							if(pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE && pst_me_radio_mngr_inst->b_AM_BandStatus == RADIO_MANAGER_AM_BAND_SUPPORTED)
							{
								pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_SELECT_STATION;
								Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_requestedBand, pst_me_radio_mngr_inst);
								pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
								Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
								HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
							}else{/*FOR MISRA C*/}
						}
						break;
				
						case RADIO_MNGR_APP_BAND_FM:
						{
							pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_SELECT_STATION;
							Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_requestedBand, pst_me_radio_mngr_inst);
							pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
							Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
						}
						break;
				
						case RADIO_MNGR_APP_BAND_DAB:
						{
							if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
							{
									pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_SELECT_STATION;
									Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_requestedBand, pst_me_radio_mngr_inst);
									pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
									Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
									HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
							}else{/*FOR MISRA C*/}
						}
						break;
					
						case RADIO_MNGR_APP_RADIO_MODE:
						{
							if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_NON_RADIO_MODE)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Select Band <Radio Mode> Requested");
								pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
								pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_RADIOMODE;
								HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
							}
						}
						break;

						case RADIO_MNGR_APP_NON_RADIO_MODE:
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Select Band <Non Radio Mode> Requested");
							Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
							pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
							pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_NONRADIOMODE;
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
						}
						break;

						default:
						{
							/* Notify the Frequency and Station information if Select Band request is received for same Band  and move state to Idle*/
							Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);

							/*Send the response to the HMI about Selected Band*/
							Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_REQ_NOT_HANDLED, pst_me_radio_mngr_inst->e_activeBand);
						}
					}
				}else{/*FOR MISRA C*/}
			}
		}
		break;

		/*Request From HMI - Active Band Seek Up/Down */
		case RADIO_MNGR_APP_SEEKUPDOWN_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Seek Up/Down Requested");
			
			if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB || pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				/*clearing timer flag & Stopping timer*/
				Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);

				/*Updating the requested seek direction into the inst hsm structure*/
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SeekDirection),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_SeekDirection),&(pst_me_radio_mngr_inst->u32_slot));

				/*Updating the Req_ID enum with the RADIO_MNGR_APP_SEEK_UPDOWN which can help to decide in Mute state */
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SEEK_UPDOWN;
				
				Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
				 
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
			}else{/*FOR MISRA C*/}
		}
		break;
		

		/*Request from the HMI -  Tune the Station from the Mixed Preset List with Index*/
		case RADIO_MNGR_APP_PRESET_RECALL_REQID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Preset_Recall_Index), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM &&
							pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency &&
							pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.u16_PI == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI)
			{
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																	(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}
			else
			{
				/*clearing timer flag & Stopping timer*/
				Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
			
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall Requested for index: %d", pst_me_radio_mngr_inst->u8_Preset_Recall_Index);
			
				if((pst_me_radio_mngr_inst->st_PrestMixedList.u8_NumPresetList != RADIO_MNGR_APP_VALUE_ZERO) && (pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band != RADIO_MNGR_APP_BAND_INVALID))
				{
					/*Update the Tunable statio with selected preset station info*/
					Update_Tunable_Station_with_Preset_Index(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->u8_Preset_Recall_Index);
				
					/*Updating the Req ID with RADIO_MNGR_APP_PRESET_RECALL can be useful in Mute State*/
					pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PRESET_RECALL;
			
					Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band, pst_me_radio_mngr_inst);
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_preset_recall_state);
				}else{/*FOR MISRA C*/}
			}
		}
		break;

		/*Request from the HMI - Update the Active Band StationList*/
		case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Manual Updated Station List Requested");
			
			if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB || pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				/*clearing timer flag & Stopping timer*/
				Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);

				pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
				
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_UPDATE_STLIST;

				Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
				
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_TUNEUPDOWN_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Tune Up/Down Requested");
			
			if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB || pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{			
				/*clearing timer flag & Stopping timer*/
				Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
			
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TuneUpDownDirection), (pst_msg->data), sizeof(Te_RADIO_DirectionType), &(pst_me_radio_mngr_inst->u32_slot));
			
				/*Updating the Req ID with RADIO_MNGR_APP_TUNEUPDOWN can be useful in Mute State*/
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_TUNEUPDOWN;
			
				Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
					
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_TUNE_UPDOWN;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
				
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Tune By Frequency Requested");
			
			if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB || pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				/*clearing timer flag & Stopping timer*/
				Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
				
				/*Updating the Req ID with RADIO_MNGR_APP_TUNEUPDOWN can be useful in Mute State*/
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_TUNEBYFREQ;
				
				Radio_Mngr_App_Notify_Clear_HMI_Data(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
				
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u32_ReqFreq), (pst_msg->data), sizeof(Tu32), &(pst_me_radio_mngr_inst->u32_slot));
				ExtractParameterFromMsg((pst_me_radio_mngr_inst->au8_DABChannelName), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->au8_DABChannelName), &(pst_me_radio_mngr_inst->u32_slot));
				
				
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);		
			}else{/*FOR MISRA C*/}			
		}
		break;
		
		case RADIO_MNGR_APP_POWER_ON_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio Power ON Requested");
			
			/*clearing timer flag & Stopping timer*/
			Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_NON_RADIO_MODE)
			{
				/*If Active Band is Non-Radio Mode then Do Nothing*/	
			}
			else
			{
				/*Updating the Request Id with Radio Power ON to use in select band handler*/
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_RADIO_POWER_ON;
				//pst_me_radio_mngr_inst->u8_Original_Band = (Te_Radio_Mngr_App_Band)(pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
			}
		}
		break;
		
		case RADIO_MNGR_APP_POWER_OFF_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]Radio Power OFF Requested");
			
			if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB || pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				/*clearing timer flag & Stopping timer*/
				Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
			
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_NON_RADIO_MODE)
				{
					/*If Active Band is Non-Radio Mode then Do Nothing*/	
				}
				else
				{				
					/*Update the LSM Band with Active Band*/
					pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
					/*Updating the Request Id with Radio Power OFF to use in select band handler*/
					pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_RADIO_POWER_OFF;
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
				}	
			}else{/*FOR MISRA C*/}					
		}
		break;
		case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio Play select from stationlist search");
			
			/*clearing timer flag & Stopping timer*/
			Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
			
			if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_DAB || pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				switch(pst_me_radio_mngr_inst->e_activeBand)
				{
					case RADIO_MNGR_APP_BAND_FM:
					{
						Update_Searched_STL_StationInfo_with_index(pst_me_radio_mngr_inst);
						
						pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
			
	
						/*Updating the Req_ID enum with the RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL which can help to decide in Mute state */
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL;
			
						Radio_Mngr_App_Notify_Clear_HMI_Data(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
	
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);	
					}
					break;
		
					case RADIO_MNGR_APP_BAND_DAB:
					{
						Update_Searched_STL_StationInfo_with_index(pst_me_radio_mngr_inst);
						
						pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
					
						/*Updating the Req_ID enum with the RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL which can help to decide in Mute state */
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL;
					
						Radio_Mngr_App_Notify_Clear_HMI_Data(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);
		
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
					}
					break;

					case RADIO_MNGR_APP_BAND_AM:
					case RADIO_MNGR_APP_RADIO_MODE:
					case RADIO_MNGR_APP_NON_RADIO_MODE:
					case RADIO_MNGR_APP_BAND_INVALID:
					{
						/* To avoid Warnings */
					}
					break;
				}
				
			}else{/*FOR MISRA C*/}		
		}
		break;
		
		case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio Play select service from multiplex list");
			
			/*clearing timer flag & Stopping timer*/
			Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_ServiceIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				if(pst_me_radio_mngr_inst->u8_ServiceIndex < 
						pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble)
				{
					Update_Multiplex_Service_StationInfo_with_index(pst_me_radio_mngr_inst);
				
					pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
			
					/*Updating the Req_ID enum with the RADIO_MNGR_APP_STATIONLIST_SELECT which can help to decide in Mute state and Station List State */
					pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_STATIONLIST_SELECT;
			
					Radio_Mngr_App_Notify_Clear_HMI_Data(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);

					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
				}else{/*FOR MISRA C*/}	
			}else{/*FOR MISRA C*/}
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveIdleListenHndlr        */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveIdleListenHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
	Tu8 u8_AFIndex;
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveIdleListenHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveIdleListenHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveIdleListenHndlr \n"));
		}
		break;

/******************************DAB to FM Linking***************************************************/
		/*From DAB to FM: For Hardlinks PI for DAB FM Linking or SID for implicit*/
		case RADIO_MNGR_APP_PICODE_LIST_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]PI List Received");
            ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_Radio_Mngr_App_PiList),	(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_Radio_Mngr_App_PiList),	&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u32_QualityMin),		(pst_msg->data), sizeof(Tu32),				&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u32_QualityMax),		(pst_msg->data), sizeof(Tu32),				&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u32_Implicit_Sid),	(pst_msg->data), sizeof(Tu32),				&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Linktype),		(pst_msg->data), sizeof(Tu8),				&(pst_me_radio_mngr_inst->u32_slot));

			AMFM_App_Request_FindBestPI((pst_me_radio_mngr_inst->st_Radio_Mngr_App_PiList), pst_me_radio_mngr_inst->u32_QualityMin, 
						pst_me_radio_mngr_inst->u32_QualityMax, pst_me_radio_mngr_inst->u32_Implicit_Sid, pst_me_radio_mngr_inst->u8_Linktype);
		}
		break;

		/*From FM to DAB: After find the Best PI Station from Hardlink PI's*/
		case RADIO_MNGR_APP_BEST_PI_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Best PI Received");
            ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u16_BestPi),  (pst_msg->data), sizeof(Tu16), &(pst_me_radio_mngr_inst->u32_slot));
            ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u32_Quality),  (pst_msg->data), sizeof(Tu32),  &(pst_me_radio_mngr_inst->u32_slot));

			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency),(pst_msg->data), sizeof(Tu32),&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_BestPI_Type), (pst_msg->data), sizeof(Te_Radio_Mngr_App_BestPI_Type),  &(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN), (pst_msg->data), RADIO_MNGR_APP_CHAN_NAME ,  &(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet),  (pst_msg->data), sizeof(Tu8),  &(pst_me_radio_mngr_inst->u32_slot));

			/*Notification to the HMI about DABFM_Linking_BestPI Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_DABFM_Linking_BestPI_Diag(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}

            DAB_App_Notify_BestPI(pst_me_radio_mngr_inst->u16_BestPi, pst_me_radio_mngr_inst->u32_Quality, (Te_DAB_App_BestPI_Type)pst_me_radio_mngr_inst->e_BestPI_Type);		
		}
		break;

		/* Form FM to RM: After blending successfully,If FM got another AF station quality higher than Hardlink PI (Beset PI) Station */
		case RADIO_MNGR_APP_BEST_PI_CHANGED_NOTIFYID:
		{
			Radio_Mngr_App_Notify_DABFMLinkingStatus(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus);

			/*Sending the FM current station info to the HMI */
			Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);	
		}
		break;

		/*From FM to DAB: After find the Best PI Station by tuning to Best PI or Same SID station, sending its Quality continously*/
		case RADIO_MNGR_APP_PI_QUALITY_NOTIFYID:
		{
			/*Reading the Quality and PSN of Best PI into the Current Station Info frequently*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u32_Quality), (pst_msg->data), sizeof(Tu32), &(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN),(pst_msg->data), 
											sizeof(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN),&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_PSNChangeFlag_not),  (pst_msg->data), sizeof(Te_Radio_Mngr_App_PSNChange),  &(pst_me_radio_mngr_inst->u32_slot));								
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
            DAB_App_Notify_PIQuality(pst_me_radio_mngr_inst->u32_Quality);
			
			/*if the PSN content is changed after DAB<=>FM/implicit linking then sending current station info to HMI*/
			if(pst_me_radio_mngr_inst->e_PSNChangeFlag_not == RADIO_MNGR_APP_NEW_PSN && (pst_me_radio_mngr_inst->e_DABFM_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS || 
				pst_me_radio_mngr_inst->e_DABFM_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS ))
			{
				/*Sending the FM current station info to the HMI */
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);				
			}
			else{/*FOR MISRA C*/}
		}
		break;

		/*From FM to DAB: While sendig the Quality before linking/implicit done if quality reduced below the threshold of Best PI/Same SID*/
		case RADIO_MNGR_APP_LINKING_STATUS_NOTIFYID:
		{
			/* To get the DabFMLining request information */
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus), (pst_msg->data), sizeof(Te_RADIO_DABFM_LinkingStatus), &(pst_me_radio_mngr_inst->u32_slot));
			DAB_App_Notify_DABtoFM_LinkingStatus((Te_RADIO_DABFM_LinkingStatus)pst_me_radio_mngr_inst->e_DABFM_LinkingStatus);
		}
		break;	

		/*Form DAB to FM: After DAB FM linking success/Resumes Back/Implicit linking succes/resumes back Done*/
		case RADIO_MNGR_APP_DAB_FM_BLENDING_STATUS_NOTIFYID:
		{
			/* To get the DabFMLining request information */
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus), (pst_msg->data), sizeof(Te_RADIO_DABFM_LinkingStatus), &(pst_me_radio_mngr_inst->u32_slot));

			Radio_Mngr_App_Notify_DABFMLinkingStatus(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus);

			switch(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus)
			{	
				/*Bleded from DAB to FM*/
				case RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS:
				case RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS:
				{
					/*clearing timer flag & Stopping timer*/
					Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);

					AMFM_App_Request_BlendingStatus((Te_RADIO_DABFM_LinkingStatus)pst_me_radio_mngr_inst->e_DABFM_LinkingStatus);

					if(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus == RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB to FM Linking Done");
						
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DABFMLINKING_DONE;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);	
					}
					else if(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB to FM Implicit Linking Done and Curr AudBand is:%d", pst_me_radio_mngr_inst->e_Curr_Audio_Band);						
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_IMPLICIT_LINKING_DONE;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);							
					}
					else{/*FOR MISRA C*/}

					/*Sending the FM current station info to the HMI */
					Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);
				}
				break;

				/*Bleded from FM to DAB*/
				case RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED:
				{
					AMFM_App_Request_BlendingStatus((Te_RADIO_DABFM_LinkingStatus)pst_me_radio_mngr_inst->e_DABFM_LinkingStatus);
					/*Sending the DAB current station info to the HMI */
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_DAB,
																			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																			pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																			pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																			pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);

				}
				break;

				case RADIO_FRMWK_DAB_FM_LINKING_CANCELLED:
				{
					AMFM_App_Notify_Stop_DAB_to_FM_Linking();
				}
				break;
				
				default:
				break;
			}
		}
		break;

/******************************DAB to FM Following END***************************************************/
		
		/* Notification from the lower layers for any updates in Tuner in both FM and DAB */
		case RADIO_MNGR_APP_AMFM_TUNER_STATUS_NOTIFYID:
		{
			/*Announcement is not going */
			if(pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_AMFM_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_AMFMSignalStatus), (pst_msg->data), sizeof(Te_Radio_Mngr_App_FG_Signal_Status), &(pst_me_radio_mngr_inst->u32_slot));
					
				/*if Current Station freq/PI and LSM Freq/PI both are different then only need to update LSM and Call LastMode NVM api*/
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && 
							pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_SELECT_STATION_END &&
							(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency != pst_me_radio_mngr_inst->st_LSM_Station_Info.u32_FM_Freq || 
								pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI != pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_PI))
				{
					Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
					
					/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
					pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																									&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
					Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
				}

				/*RDS Settings ON only then PI Check if Quality good at before strategy started*/
				if((pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_STATIONLIST_SELECT     	 ||
				   			pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PLAY_SELECT_STATION ||
							pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL ||
							pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PRESET_RECALL) && 
					      pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_AMFMSignalStatus == RADIO_MNGR_APP_SIGNAL_HIGH)
				{
					/*PI Matched*/
					if(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI == pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI)
					{
						/*while waiting for PI with 3sec in middle if we receive same PI then stop the timer*/
						if(pst_me_radio_mngr_inst->b_TimerFlag  == RADIO_MNGR_APP_SET_TIMER_FLAG)
						{
							/*clearing timer flag & Stopping timer*/
							Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
						}else{/*FOR MISRA C*/}
						
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;

						Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
					
						/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
						pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																										&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
						Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
						
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]PI Matched");				
					}

					/*PI zero and not started timer yet then start*/
					else if(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI == RADIO_MNGR_APP_VALUE_ZERO &&
						       pst_me_radio_mngr_inst->b_TimerFlag  != RADIO_MNGR_APP_SET_TIMER_FLAG)
					{
						pst_me_radio_mngr_inst->b_TimerFlag  = RADIO_MNGR_APP_SET_TIMER_FLAG;
						pst_me_radio_mngr_inst->b_PIDecode_TimerFlag = PI_DECODE_TIMER_STARTED;
						st_Radio_Mngr_App_TimerID.u32_PI_Decode_Timerid =  SYS_StartTimer(RADIO_MNGR_APP_FM_MAX_TIME_PI_DECODE, RADIO_MNGR_APP_PI_DECODE_TIMEOUT, RADIO_MNGR_APP);
						if(st_Radio_Mngr_App_TimerID.u32_PI_Decode_Timerid == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][RM]  :PI Decode timer failed ");	
						}
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]PI Zero");
					}

					/*PI is not same*/
					else if(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI != pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]PI not Matched Started Startegy");
						
						Update_LSM_Station_Info(pst_me_radio_mngr_inst);
						
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM] Freq: %d, Last PI:%d, Received PI: %d", 
								pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency,
								pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI,
								pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI);
						
						/*Sending Signal Loss Notification to HMI*/
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_STATION_NOT_AVAILABLE;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
#if 0												
						Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START);
#endif
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
					}
				}
			}
			/*Announcement is going*/
			else
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo),&(pst_me_radio_mngr_inst->u32_slot));
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_AMFMSignalStatus), (pst_msg->data), sizeof(Te_Radio_Mngr_App_FG_Signal_Status), &(pst_me_radio_mngr_inst->u32_slot));
					
				/*Non EON Station Announcement*/
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && 
							pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency == pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.u32_frequency)
					{
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo), &(pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo), sizeof(Ts_Radio_Mngr_App_AMFM_CurrentStationInfo));	
						
						Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																	(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
													
						if(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI != pst_me_radio_mngr_inst->st_LSM_Station_Info.u16_PI)
						{
							Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
					
							/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
							pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																										&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
							Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
						}							
					}
					/*EON Station Announcement*/
					else
					{
						Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																	pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																	pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.au8_RadioText,
																	(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);	
					}	
				}			

			/*ENG mode is ON - Notification to the HMI about current station info*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst);
				Radio_Mngr_App_Notify_Quality_Diag(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
			}
			else{/*FOR MISRA C*/}

			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_FM && 
						pst_me_radio_mngr_inst->b_PIDecode_TimerFlag != PI_DECODE_TIMER_STARTED)
			{
				/*Quality flag check for Label clear - Good*/
				if(pst_me_radio_mngr_inst->e_AMFMSignalStatus != RADIO_MNGR_APP_SIGNAL_LOW)
				{
					Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, RADIO_MNGR_APP_LISTENING);
					
					/*clearing timer flag & Stopping timer*/
					Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);
				
					pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet = 3;
				}
				/*Low Quality Start the timer if its not started*/
				else
				{
					if(pst_me_radio_mngr_inst->b_TimerFlag  != RADIO_MNGR_APP_SET_TIMER_FLAG)
					{
						/*setting timer flag as one,to know while stopping timer*/
						pst_me_radio_mngr_inst->b_TimerFlag  = RADIO_MNGR_APP_SET_TIMER_FLAG;
				
						/*If quality reduces wait for 60 sec before clearing label*/
						 st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid =  SYS_StartTimer(RADIO_MNGR_APP_FM_MAX_TIME_QUALITY_RESUME, RADIO_MNGR_APP_LOW_QUALITY_TIMEOUT, RADIO_MNGR_APP);
						 if(st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid == 0)
						 {
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[RADIO][RM]  : AMFM PSN and RT clear timer failed ");	
						 }
						 else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
			}else{/*FOR MISRA C*/}
						
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && 
					pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck == RADIO_MNGR_APP_STATIONLIST_FM_SUCCESS)
			{
				pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck = RADIO_MNGR_APP_VALUE_ZERO;
				
				if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH)
				{					
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
															pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].au8_PSN,
															pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_CharSet, 
															(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
				}
				else
				{
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].au8_PSN,
																	pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_CharSet, 
																	(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
				}
			}
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM &&
							pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck == RADIO_MNGR_APP_PRESET_FM_SUCCESS)
			{
				pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck = RADIO_MNGR_APP_VALUE_ZERO;
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	 pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.au8_PSN,
																	 pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u8_CharSet, 
																	 pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}		
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START && pst_me_radio_mngr_inst->e_AMFMSignalStatus != RADIO_MNGR_APP_SIGNAL_LOW &&
						pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_FMDAB_SID_Type != RADIO_MNGR_APP_FMDAB_BLENDING_SUCCESS)
			{
				if(SYS_RADIO_STR_LEN((const Tchar *)pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN) != 0)
				{
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																	(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
				}else{/*FOR MISRA C*/}	
			}

			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_FM)
			{
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}

		}	
		break;

		case RADIO_MNGR_APP_PI_DECODE_TIMEOUT:
		{
			/* Posted Timer message. Reset the timerid */
			st_Radio_Mngr_App_TimerID.u32_PI_Decode_Timerid = 0;
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]PI Decode Time 3sec Completed");
			
			Radio_Mngr_App_Notify_Clear_HMI_Data(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
			pst_me_radio_mngr_inst->b_PIDecode_TimerFlag = RADIO_MNGR_APP_VALUE_ZERO;

			/*After Timer Elapsed, clearing timer flag as zero*/
			pst_me_radio_mngr_inst->b_TimerFlag  = RADIO_MNGR_APP_CLEAR_TIMER_FLAG;

			pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PI_NOT_FOUND_AF_TUNE;
			
			pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_STATION_NOT_AVAILABLE;
			Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
#if 0
			Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START);
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
#endif
		}
		break;

		case RADIO_MNGR_APP_DAB_TUNER_STATUS_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_TunerNotify),(pst_msg->data),sizeof(Ts_Radio_Mngr_App_DAB_TunerStatusNotify),&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABSignalStatus), (pst_msg->data), sizeof(Te_Radio_Mngr_App_FG_Signal_Status), &(pst_me_radio_mngr_inst->u32_slot));

			/*Notification to the HMI about quality Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_Quality_Diag(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
			}
			else{/*FOR MISRA C*/}
			
			if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				if (pst_me_radio_mngr_inst->e_DABSignalStatus != RADIO_MNGR_APP_SIGNAL_LOW)
				{
					Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, RADIO_MNGR_APP_LISTENING);

					/*clearing timer flag & Stopping timer*/
					Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);

					pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet = 3;
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst->st_CurrentStationName));
				}
				else
				{
					/*If timer not started and DAB TO FM linking and implicit linking not happened then only start the timer 10sec*/
					if (pst_me_radio_mngr_inst->b_TimerFlag != RADIO_MNGR_APP_SET_TIMER_FLAG && pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS &&
						pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
					{
						/*setting timer flag as one,to know while stopping timer*/
						pst_me_radio_mngr_inst->b_TimerFlag = RADIO_MNGR_APP_SET_TIMER_FLAG;

						/*If quality reduces wait for 10 sec before clearing label*/
						st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid = SYS_StartTimer(RADIO_MNGR_APP_DAB_MAX_TIME_QUALITY_RESUME, RADIO_MNGR_APP_LOW_QUALITY_TIMEOUT, RADIO_MNGR_APP);
						if (st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][RM]  : DAB label clear timer failed ");
						}
						else{/*FOR MISRA C*/ }
					}
					else{/*FOR MISRA C*/ }
				}
			}else{/*FOR MISRA C*/}
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS && 
				        pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS &&
						pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_DAB,
																pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
			}
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}
			
		}	
		break;

		case RADIO_MNGR_APP_LOW_QUALITY_TIMEOUT:
		{
			st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid = 0;
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->b_TimerFlag == RADIO_MNGR_APP_SET_TIMER_FLAG)
			{
				memset((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN), 0, RADIO_MNGR_APP_CHAN_NAME);
				memset((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText), 0, RADIO_MNGR_APP_CHAN_RADIOTEXT);
				
				/*clearing timer flag as zero*/
				pst_me_radio_mngr_inst->b_TimerFlag  = RADIO_MNGR_APP_CLEAR_TIMER_FLAG;
				
				Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM Signal Loss Clear Label after 60sec");

				/*Sending Signal Loss Notification to HMI*/
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_SIGNAL_LOSS;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
			}
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->b_TimerFlag == RADIO_MNGR_APP_SET_TIMER_FLAG)
			{
				memset(&(pst_me_radio_mngr_inst->st_DLS_Data), 0, sizeof(Ts_Radio_Mngr_App_DLS_Data));
				memset(pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel, 0, RADIO_MNGR_APP_COMPONENT_LABEL);	
				memset(pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, 0, RADIO_MNGR_APP_NUMCHAR_LABEL);
				
				/*clearing timer flag as zero*/
				pst_me_radio_mngr_inst->b_TimerFlag  = RADIO_MNGR_APP_CLEAR_TIMER_FLAG;
				
				Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB Signal Loss Clear Label after 10sec");

				/*Sending Signal Loss Notification to HMI*/
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_SIGNAL_LOSS;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
			}
			else{/*FOR MISRA C*/}	
		}
		break;
		case RADIO_MNGR_APP_FM_AFLIST_UPDATE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_APPtoRM_FMAFList), (pst_msg->data), (Tu8)sizeof(Ts_Radio_Mngr_App_APP_TO_RM_FM_AFList), &(pst_me_radio_mngr_inst->u32_slot));
			
			pst_me_radio_mngr_inst->st_FM_AFList.u8_NumAFList = pst_me_radio_mngr_inst->st_APPtoRM_FMAFList.u8_NumAFList;
			
			for(u8_AFIndex = RADIO_MNGR_APP_VALUE_ZERO; u8_AFIndex < pst_me_radio_mngr_inst->st_APPtoRM_FMAFList.u8_NumAFList; u8_AFIndex++)
			{
				pst_me_radio_mngr_inst->st_FM_AFList.au32_AFList[u8_AFIndex] = pst_me_radio_mngr_inst->st_APPtoRM_FMAFList.ast_ENG_FM_AFList[u8_AFIndex].u32_AFFreq;
				pst_me_radio_mngr_inst->st_FM_AFList.au16_PIList[u8_AFIndex] = pst_me_radio_mngr_inst->st_APPtoRM_FMAFList.ast_ENG_FM_AFList[u8_AFIndex].u16_PI;
				pst_me_radio_mngr_inst->st_FM_AFList.au32_Quality[u8_AFIndex] = pst_me_radio_mngr_inst->st_APPtoRM_FMAFList.ast_ENG_FM_AFList[u8_AFIndex].u32_AvgQual;
			}
			
			/*Notification to the HMI about FM AF list Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_AFList_Diag(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}

		}
		break;

		case RADIO_MNGR_APP_DAB_AFLIST_UPDATE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_AFList), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_DAB_AFList), &(pst_me_radio_mngr_inst->u32_slot));
			
			/*Notification to the HMI about DAB AF list Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_AFList_Diag(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}

		}
		break;
		/* Handle the Play select station Response in AF frequency tuning*/
		case RADIO_MNGR_APP_AMFM_PLAY_SELECT_STATION_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SelectStationReplyStatus),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_SelectStationReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));
		
				if(pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START)
				{		
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));	
							
					/* passing Freq and Station Name to HMI-IF along with response */
					Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);
				}	
				else
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo),&(pst_me_radio_mngr_inst->u32_slot));				
					
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																	pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																	pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_EONAnno_stationinfo.au8_RadioText,
																	(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);					
				}

				/*Updating Preset Mixed List with FM AF Tuned Frequency*/
				if(pst_me_radio_mngr_inst->e_AF_Status == RADIO_MNGR_APP_AF_LINK_ESTABLISHED)
				{
					Radio_Manager_App_Update_PresetMixedList_AFTune(pst_me_radio_mngr_inst);
				}else{/*FOR MISRA C*/}	
			}
		break;

		/* Handle the Play select station Response after some time sig available*/
		case RADIO_MNGR_APP_DAB_PLAY_SELECT_STATION_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SelectStationReplyStatus),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_SelectStationReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));

				if(pst_me_radio_mngr_inst->e_Anno_Status != RADIO_MNGR_APP_ANNO_START)
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
					
					/* passing Freq and Station Name to HMI-IF along with response */
					Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);

					/*Notification to the HMI about DAB currentstation Diag info, when ENG mode is ON*/
					if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
					{
						Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst);
					}
					else{/*FOR MISRA C*/}
								
					Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
					/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
					pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																											
					Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);				
				}
				else
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo), &(pst_me_radio_mngr_inst-> st_DAB_Anno_CurrentStationName));

					/*Notification to the HMI about DAB currentstation Diag info, when ENG mode is ON*/
					if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
					{
						Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst);
					}
					else{/*FOR MISRA C*/}
				
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display((Te_Radio_Mngr_App_Band)pst_me_radio_mngr_inst->e_activeBand, 
									pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.st_Tunableinfo.u32_Frequency, 
									pst_me_radio_mngr_inst->st_DAB_Anno_CurrentStationName.au8_CompLabel, pst_me_radio_mngr_inst->st_DAB_Anno_CurrentStationName.u8_CharSet, (Tu8*)NULL,
									pst_me_radio_mngr_inst->st_DAB_Anno_currentstationinfo.au8_ChannelName,
									pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);				
				}
		}
		break;

		case RADIO_MNGR_APP_AMFM_TUNEUPDOWN_DONE_RESID:
		case RADIO_MNGR_APP_DAB_TUNEUPDOWN_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TuneUpDownReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));				

			switch(pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_AM:
				case RADIO_MNGR_APP_BAND_FM:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_AMFM_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_DAB_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
				}
				break;

				default:
					break;
			}

			Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
			/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
			pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE,
				&(pst_me_radio_mngr_inst->st_LSM_Station_Info));

			Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
			/* passing Freq and Station Name to HMI-IF along with response */
			Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);
		}
		break;

		case RADIO_MNGR_APP_DAB_DLS_DATA_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DLS_Data),(pst_msg->data), sizeof(Ts_Radio_Mngr_App_DLS_Data),&(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS && pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS &&
				pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				/*TBD: Need to send DLS to HMI with decided scenario*/
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																			pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																			pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																			pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																			pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
			}
			else if(pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_DAB_SLS_DATA_NOTIFYID:
		{   
			/*Accessing shared memory to read the SLS data*/
			SYS_MUTEX_LOCK(SLS_RM_DAB_APP);
			SYS_RADIO_MEMCPY(&st_Radio_Mngr_App_SLS_Data, &st_Dabapp_SLS_Data, sizeof(Ts_DAB_App_DataServiceRaw));
			SYS_MUTEX_UNLOCK(SLS_RM_DAB_APP);
			Notify_DAB_Dataservice_To_Display(&st_Radio_Mngr_App_SLS_Data);
			

		}
		break;
		
		/*AM Background scan StL update done Notification*/
		case RADIO_MNGR_APP_AM_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]AM Station List Updated in Foreground");
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]AM Station List Updated in Background");
			}
				
			SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

			/* Read the AM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_AM);

			SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);
		}
		break;

		/*FM Background scan StL update done Notification*/
		case RADIO_MNGR_APP_FM_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FM Station List Updated in Foreground");
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FM Station List Updated in Background");
			}
			
			SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

			/* Read the FM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);

			Update_MatchedStationListIndex(pst_me_radio_mngr_inst);

			Radio_Mngr_App_Notify_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
			SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);
		}
		break;

		/*DAB Background scan StL update done Notification*/
		case RADIO_MNGR_APP_DAB_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			SYS_MUTEX_LOCK(STL_RM_DAB_APP);

			/* Read the FM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
			
			SYS_MUTEX_UNLOCK(STL_RM_DAB_APP);
			
			/*Creating Multiplex & Normal Stationlist*/
			Radio_Mngr_App_CreateNormalRadioStationList(pst_me_radio_mngr_inst);
			Radio_Mngr_App_CreateMultiplexRadioStationList(pst_me_radio_mngr_inst);

			Update_MatchedStationListIndex(pst_me_radio_mngr_inst);
			
			Radio_Mngr_App_Notify_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DAB Station List Updated in Foreground");
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DAB Station List Updated in Background");
			}
		}
		break;
		
		/***********Announcement Cancel************/
		case RADIO_MNGR_APP_ANNO_CANCEL_REQ:
		{
			/* Route message to parent state to handle the new request given by HMI */
			if(pst_me_radio_mngr_inst->b_Internal_Msg_Flag == RADIO_MNGR_APP_UINT8_ONE)
			{
				pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ZERO;
				pst_ret = &(pst_me_radio_mngr_inst->st_msg_cpy);
			}

			/*Checking whether announcement started or not,if yes then copy the message and give cancel announcement request to lower layer */
			else if(pst_me_radio_mngr_inst->e_Anno_Status == RADIO_MNGR_APP_ANNO_START || pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START ||
							pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
			{
				switch(pst_msg->msg_id)
				{
					case RADIO_MNGR_APP_TUNEUPDOWN_REQID:
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							if (pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								AMFM_App_Request_Cancel();
							}
							else if (pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
							{
								pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
								Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
								pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
								Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
							}
							else
							{
								/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
								pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;

								/*Request for Anno cancel based on request type to AMFM App*/
								AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
							}
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if (pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
								{
									DAB_App_Request_AnnoCancel();
								}
								else
								{
									DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
								}
							}
						}
						else{/*FOR MISRA C*/ }
					}
					break;
					
					/*When seek request come from HMI, we need cancel the ongoing announcement and switch to seek*/
					case RADIO_MNGR_APP_SEEKUPDOWN_REQID:
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								AMFM_App_Request_Cancel();								
							}
							else if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
							{
								pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
								Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
								pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
								Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
							}
							else
							{
								/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
								pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;
						
								/*Request for Anno cancel based on request type to AMFM App*/
								AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
							}
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
								{
									DAB_App_Request_AnnoCancel();
								}
								else
								{
									DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
								}
							}
						}else{/*FOR MISRA C*/}
					}
					break;
					
					/*When update stationlist come from HMI, we need cancel the ongoing announcement and switch to update stationlist*/
					case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								AMFM_App_Request_Cancel();								
							}
							else if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
							{
								pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
								Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
								pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
								Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
							}
							else
							{
								/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
								pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;

								/*Request for Anno cancel based on request type to AMFM App*/
								AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
							}
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
								{
									DAB_App_Request_AnnoCancel();
								}
								else
								{
									DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
								}
							}
						}else{/*FOR MISRA C*/}					
					}
					break;
					
					/*When tune by frequency come from HMI, we need cancel the ongoing announcement and switch to tune by frequency*/
					case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								AMFM_App_Request_Cancel();								
							}
							else if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
							{
								pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
								Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
								pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
								Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
							}
							else
							{
								/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
								pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;

								/*Request for Anno cancel based on request type to AMFM App*/
								AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
							}
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
								{
									DAB_App_Request_AnnoCancel();
								}
								else
								{
									DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
								}
							}
						}else{/*FOR MISRA C*/}					
					}
					break;
					
					/*When select band come from HMI, we need cancel the ongoing announcement and switch to selected band*/
					case RADIO_MNGR_APP_SELECTBAND_REQID:
					{
						ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_AnnoCancelRequestedBand),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_AnnoCancelRequestedBand),&(pst_me_radio_mngr_inst->u32_slot));
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue = Radio_Mngr_App_SelectBandConditionCheck(pst_me_radio_mngr_inst->e_AnnoCancelRequestedBand, pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
						
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
						{
							if(pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_AMFM_CANCEL)
							{
								if(pst_me_radio_mngr_inst->e_AnnoCancelRequestedBand != pst_me_radio_mngr_inst->e_activeBand)
								{
									pst_me_radio_mngr_inst->b_Check_AudioSwitchNeed = RADIO_MNGR_APP_AUDIO_SWITCH_NOT_NEEDED;
								}else{/*FOR MISRA C*/}
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								AMFM_App_Request_Cancel();								
							}
							else if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_DAB_CANCEL)
							{
								if(pst_me_radio_mngr_inst->e_AnnoCancelRequestedBand != pst_me_radio_mngr_inst->e_activeBand)
								{
									pst_me_radio_mngr_inst->b_Check_AudioSwitchNeed = RADIO_MNGR_APP_AUDIO_SWITCH_NOT_NEEDED;
								}else{/*FOR MISRA C*/}
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
						}
						else if(pst_me_radio_mngr_inst->e_AnnoCancelRequestedBand != pst_me_radio_mngr_inst->e_activeBand)
						{
							if(pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_AMFM_CANCEL)
							{
								if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
								{
									pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
									pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
									Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
								}
								else
								{
									/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
									pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;

									/*Request for Anno cancel based on request type to AMFM App*/
									AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
								}
							}
							else if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_DAB_CANCEL)
							{
								if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
								{
									DAB_App_Request_AnnoCancel();
								}
								else
								{
									DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
								}
							}else{/*FOR MISRA C*/}	
						}else{/*FOR MISRA C*/}	
					}
					break;

					/*When preset index is valid then give cancel announcement request*/
					case RADIO_MNGR_APP_PRESET_RECALL_REQID:
					{
						ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Preset_Recall_Index),(pst_msg->data),sizeof(Tu8),&(pst_me_radio_mngr_inst->u32_slot));
					
						if(Radio_Mngr_App_PresetRecallConditionCheck(pst_me_radio_mngr_inst))
						{
							/*Copying msg to local buffer, it is used when we pass request to parent handler*/
							SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
							if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
									AMFM_App_Request_Cancel();								
								}
								else if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
								{
									if(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.u32_frequency == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency &&
											pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.u16_PI == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI)
									{
										Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
									}
									else
									{
										pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
										Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
										pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
										Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
									}
								}
								else
								{
									/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
									pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;

									/*Request for Anno cancel based on request type to AMFM App*/
									AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
								}
							}
							else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
									DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
								}
								else
								{
									if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
									{
										DAB_App_Request_AnnoCancel();
									}
									else
									{
										DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
									}
								}
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}
					}
					break;
					
					/*When playselect station come from HMI, we need cancel the ongoing announcement */
					case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:
					{
						ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Index),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->u8_Index),&(pst_me_radio_mngr_inst->u32_slot));
			
						/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
						if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList) ||
						 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList) ||
						 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList) )
						{
							/*Copying msg to local buffer, it is used when we pass request to parent handler*/
							SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
							if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
									AMFM_App_Request_Cancel();								
								}
								else if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
								{
									if(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].u32_frequency == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency &&
											pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].u16_PI == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI)
									{
										Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
									}
									else
									{
										pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
										Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
										pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
										Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
									}
								}
								else
								{
									/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
									pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;

									/*Request for Anno cancel based on request type to AMFM App*/
									AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
								}
							}
							else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
									DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
								}
								else
								{
									if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
									{
										DAB_App_Request_AnnoCancel();
									}
									else
									{
										DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
									}
								}
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}
					}
					break;
					
					/*When playselect for searched stl station come from HMI, we need cancel the ongoing announcement */
					case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:
					{
						ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
						/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
						if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList) ||
							(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList) ||				 
							(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList))
						{
								/*Copying msg to local buffer, it is used when we pass request to parent handler*/
								SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
								if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
								{
									if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
									{
										Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
										AMFM_App_Request_Cancel();								
									}
									else if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
									{
										if(pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].u32_frequency == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency &&
												pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].u16_PI == pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u16_PI)
										{
											Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
										}
										else
										{
											pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
											Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
											pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
											Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
										}
									}
									else
									{
										/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
										pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;

										/*Request for Anno cancel based on request type to AMFM App*/
										AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
									}
								}
								else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
								{
									if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
									{
										Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
										DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
									}
									else
									{
										if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
										{
											DAB_App_Request_AnnoCancel();
										}
										else
										{
											DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
										}
									}
								}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}
					}
					break;
					
					/*When playselect for multiplex stl station come from HMI, we need cancel the ongoing announcement */
					case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:
					{
						ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_ServiceIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
						/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_ServiceIndex < 
								pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble)
						{
							/*Copying msg to local buffer, it is used when we pass request to parent handler*/
							SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
							
							if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
									DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
								}
								else
								{
									if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
									{
										DAB_App_Request_AnnoCancel();
									}
									else
									{
										DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
									}
								}
							}else{/*FOR MISRA C*/}
						}else{/*FOR MISRA C*/}
					}
					break;
					
					/*When Power OFF requested from HMI, we need cancel the ongoing announcement and de-select the active band*/
					case RADIO_MNGR_APP_POWER_OFF_REQID:
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								AMFM_App_Request_Cancel();								
							}
							else if(pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status == RADIO_MNGR_APP_FM_TO_DAB_STARTED)
							{
								pst_me_radio_mngr_inst->b_FM_To_DAB_Linking_Status = RADIO_MNGR_APP_FM_TO_DAB_STOPPED;
								Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
								pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
								Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
							}
							else
							{
								/*Updating Anno Cancel Request as New request, so as to pass this as argument to AMFM App layer*/
								pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type = RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST;
						
								/*Request for Anno cancel based on request type to AMFM App*/
								AMFM_App_Request_AnnoCancel((Te_AMFM_App_Anno_Cancel_Request)pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type);
							}
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
								{
									DAB_App_Request_AnnoCancel();
								}
								else
								{
									DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_ANNO_CANCEL);
								}
							}
						}else{/*FOR MISRA C*/}
					}
					break;

					default:
					{
						/*Do nothing*/
					}
					break;
				}
			}
			/*Route Msgs to parent handler if Anno is not going*/
			else
			{
				/*Copying msg to local buffer, it is used when we pass request to parent handler*/
				SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
				pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
				/*Posting of msg so it can be handled by respective handler*/
				Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
			}
		}
		break;

		/*After Anno cancel, response should be handled*/
		case RADIO_MNGR_APP_AMFM_ANNO_CANCEL_RESID:
		case RADIO_MNGR_APP_DAB_ANNO_CANCEL_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Announcement Cancelled due to New Request");
			
			/*Extracting the Anno cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_ReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type != RADIO_MNGR_APP_ANNO_CANCEL_BY_HMI)
			{
				/*If Anno cancel is success, set HMI Req flag as one & call the self post msg function to handle the request*/
				if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
				{
					/*Setting the HMI Req flag as One so it should go with respective request to parent handler*/
					pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
					
					/*Posting of msg so it can be handled by respective handler*/
					Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
				}else{/*FOR MISRA C*/}
			}
			else
			{
				pst_me_radio_mngr_inst->e_Anno_Status = RADIO_MNGR_APP_ANNO_END;
				Radio_Mngr_App_Response_AnnoCancel(pst_me_radio_mngr_inst->e_ReplyStatus); 
			}
		}
		break;
		/*Nofication of any Frequency change in AMFM*/
		case RADIO_MNGR_APP_AMFM_FREQ_CHANGE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_AMCurrentStationInfo.u32_Freq),(pst_msg->data), sizeof(Tu32),&(pst_me_radio_mngr_inst->u32_slot));
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display((Te_Radio_Mngr_App_Band)pst_me_radio_mngr_inst->e_activeBand, 
								pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_AMCurrentStationInfo.u32_Freq, 
								(Tu8*)NULL, (Tu8)3, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_FM)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_FMCurrentStationInfo.u32_frequency),(pst_msg->data), sizeof(Tu32),&(pst_me_radio_mngr_inst->u32_slot));
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display((Te_Radio_Mngr_App_Band)pst_me_radio_mngr_inst->e_activeBand, 
								pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
								(Tu8*)NULL, (Tu8)3, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}			
		}
		break;

		/*Noficatio of any Frequency change in DAB*/
		case RADIO_MNGR_APP_DAB_FREQ_CHANGE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				memset(&(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo), 0, sizeof(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo));
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_Tunableinfo.u32_Frequency),(pst_msg->data), sizeof(Tu32),&(pst_me_radio_mngr_inst->u32_slot));
				ExtractParameterFromMsg((pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName),&(pst_me_radio_mngr_inst->u32_slot));
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_Tunableinfo.u32_Frequency,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_ServiceLabel.au8_Label,
																			(Tu8)RADIO_MNGR_APP_CHARSET_UTF8, (Tu8*)NULL,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_EnsembleLabel.au8_Label,
																			pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}
		}	
		break;
		
		case RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID:
		case RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID:
		{
			/*Extracting the AF Tune cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));

			if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS) ||
				(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS))
			{
				if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START && 
						((pst_me_radio_mngr_inst->e_RDSSettings_Request == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE && 
							pst_me_radio_mngr_inst->e_RDSSettings == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE) ||
						 (pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request == RADIO_MNGR_APP_DABFMLINKING_DISABLE && 
							pst_me_radio_mngr_inst->e_DABFMLinking_Switch == RADIO_MNGR_APP_DABFMLINKING_DISABLE)))
				{
					Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
					/*Tune to Original Station*/
					Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);					
				}
				else
				{
					/*Setting the AF Tune cancel Flag, so as we can use the same in Active idle handler*/
					pst_me_radio_mngr_inst->b_AF_Tune_Cancel_Status = RADIO_MNGR_APP_AF_TUNE_CANCELLED;
					
					/*Posting of msg so it can be handled by respective handler*/
					Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
				}				
			}else{/*FOR MISRA C*/}	
		}
		break;
		
		case RADIO_MNGR_APP_FM_ANNOUNCEMENT_NOTIFYID:
		case RADIO_MNGR_APP_DAB_ANNOUNCEMENT_NOTIFYID:
		{			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_Anno_Status_Type),(pst_msg->data), sizeof(Te_Radio_Mngr_App_Anno_Status),&(pst_me_radio_mngr_inst->u32_slot));

			if(pst_me_radio_mngr_inst->e_Anno_Status_Type == RADIO_MNGR_APP_ANNO_START)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM] Announcement Started");
				
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_ANNOUNCEMENT;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);			
			}else{/*FOR MISRA C*/}
			
			if(pst_me_radio_mngr_inst->e_Anno_Status_Type == RADIO_MNGR_APP_ANNO_END || 
					pst_me_radio_mngr_inst->e_Anno_Status_Type == RADIO_MNGR_APP_ANNOUNCEMENT_END_SIGNAL_LOSS || 
					pst_me_radio_mngr_inst->e_Anno_Status_Type == RADIO_MNGR_APP_ANNOUNCEMENT_END_USER_CANCEL || 
					pst_me_radio_mngr_inst->e_Anno_Status_Type == RADIO_MNGR_APP_ANNOUNCEMENT_END_TA_SWITCH_OFF || 
					pst_me_radio_mngr_inst->e_Anno_Status_Type == RADIO_MNGR_APP_ANNOUNCEMENT_NOT_AVAILABLE)
			{
				pst_me_radio_mngr_inst->e_Anno_Status = RADIO_MNGR_APP_ANNO_END;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM] Announcement Ended");
				
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
				{
					Radio_Mngr_App_Notify_Announcement(pst_me_radio_mngr_inst->e_Anno_Status);
				}
				else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
				{
					Radio_Mngr_App_Notify_Announcement(pst_me_radio_mngr_inst->e_Anno_Status);
				}else{/*FOR MISRA C*/}
			}
			else
			{

				/* Temporarily disable announcment status update*/

				pst_me_radio_mngr_inst->e_Anno_Status					= RADIO_MNGR_APP_ANNO_INVALID;
#if 0
				pst_me_radio_mngr_inst->e_Anno_Status = pst_me_radio_mngr_inst->e_Anno_Status_Type;
#endif
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM] Announcement Started");
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
				{
					Radio_Mngr_App_Notify_Announcement(pst_me_radio_mngr_inst->e_Anno_Status);
					
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																	pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																	(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
				}
				else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
				{
					Radio_Mngr_App_Notify_Announcement(pst_me_radio_mngr_inst->e_Anno_Status);
					
					Radio_Mngr_App_Response_UpdateCurStationInfo_Display((Te_Radio_Mngr_App_Band)pst_me_radio_mngr_inst->e_activeBand, 
									pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq, 
									pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel, pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet, (Tu8*)NULL,
									pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
									pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
				}else{/*FOR MISRA C*/}
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveStopHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveStopHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */

	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveStopHndlr");
            SYS_RADIO_MEMCPY((void*) pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveStopHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveStopHndlr \n"));
            /*Self posting function used for handling instant Hsm done response*/
			Radio_Mngr_App_Responsse_Inst_Hsm_Stop(REPLYSTATUS_SUCCESS);
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_inactive_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusyHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusyHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;
	
	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusyHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusyHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusyHndlr \n"));
		}
		break;

		/*Noficatio of any Frequency change in AMFM*/
		case RADIO_MNGR_APP_AMFM_FREQ_CHANGE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_AMCurrentStationInfo.u32_Freq),(pst_msg->data), sizeof(Tu32),&(pst_me_radio_mngr_inst->u32_slot));
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display((Te_Radio_Mngr_App_Band)pst_me_radio_mngr_inst->e_activeBand, 
								pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_AMCurrentStationInfo.u32_Freq, 
								(Tu8*)NULL, (Tu8)3, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_FM)
			{
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_FMCurrentStationInfo.u32_frequency),(pst_msg->data), sizeof(Tu32),&(pst_me_radio_mngr_inst->u32_slot));
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display((Te_Radio_Mngr_App_Band)pst_me_radio_mngr_inst->e_activeBand, 
								pst_me_radio_mngr_inst->st_AMFM_FreqChangeNotinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
								(Tu8*)NULL, (Tu8)3, (Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}			
		}
		break;

		/*Noficatio of any Frequency change in DAB*/
		case RADIO_MNGR_APP_DAB_FREQ_CHANGE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				memset(&(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo), 0, sizeof(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo));
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_Tunableinfo.u32_Frequency),(pst_msg->data), sizeof(Tu32),&(pst_me_radio_mngr_inst->u32_slot));
				ExtractParameterFromMsg((pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName),&(pst_me_radio_mngr_inst->u32_slot));
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand, 
				     														pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_Tunableinfo.u32_Frequency, 
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_ServiceLabel.au8_Label, 
																			(Tu8)RADIO_MNGR_APP_CHARSET_UTF8, (Tu8*)NULL,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_EnsembleLabel.au8_Label,
																			pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}
		}	
		break;
		
		/*AM Background scan StL update done Notification*/
		case RADIO_MNGR_APP_AM_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]AM Station List Updated in Foreground");
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]AM Station List Updated in Background");
			}
			
			SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

			/* Read the AM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_AM);

			SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);
		}
		break;

		/*FM Background scan StL update done Notification*/
		case RADIO_MNGR_APP_FM_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FM Station List Updated in Foreground");
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FM Station List Updated in Background");
			}
			
			SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

			/* Read the FM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);

			Update_MatchedStationListIndex(pst_me_radio_mngr_inst);

			Radio_Mngr_App_Notify_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
			SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);
		}
		break;

		/*DAB Background scan StL update done Notification*/
		case RADIO_MNGR_APP_DAB_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			SYS_MUTEX_LOCK(STL_RM_DAB_APP);

			/* Read the FM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
			
			SYS_MUTEX_UNLOCK(STL_RM_DAB_APP);
			
			/*Creating Multiplex & Normal Stationlist*/
			Radio_Mngr_App_CreateNormalRadioStationList(pst_me_radio_mngr_inst);
			Radio_Mngr_App_CreateMultiplexRadioStationList(pst_me_radio_mngr_inst);

			Update_MatchedStationListIndex(pst_me_radio_mngr_inst);
			
			Radio_Mngr_App_Notify_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DAB Station List Updated in Foreground");
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DAB Station List Updated in Background");
			}
		}
		break;

		/*While Satern Blend to FM, if user given seek request, then DAB has to cancel the blending*/
		case RADIO_MNGR_APP_DAB_FM_BLENDING_STATUS_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus), (pst_msg->data), sizeof(Te_RADIO_DABFM_LinkingStatus), &(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_DABFM_LinkingStatus == RADIO_FRMWK_DAB_FM_LINKING_CANCELLED)
			{
				AMFM_App_Notify_Stop_DAB_to_FM_Linking();
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_DAB_TUNER_STATUS_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_TunerNotify),(pst_msg->data),sizeof(Ts_Radio_Mngr_App_DAB_TunerStatusNotify),&(pst_me_radio_mngr_inst->u32_slot));
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DABSignalStatus), (pst_msg->data), sizeof(Te_Radio_Mngr_App_FG_Signal_Status), &(pst_me_radio_mngr_inst->u32_slot));

			/*Notification to the HMI about quality Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_Quality_Diag(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
			}
			else{/*FOR MISRA C*/}
			
			if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				if (pst_me_radio_mngr_inst->e_DABSignalStatus != RADIO_MNGR_APP_SIGNAL_LOW)
				{
					Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, RADIO_MNGR_APP_LISTENING);

					/*clearing timer flag & Stopping timer*/
					Radio_Mngr_App_Timer_ClearCheckParameters(pst_me_radio_mngr_inst);

					pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet = 3;
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst->st_CurrentStationName));
				}
				else
				{
					/*If timer not started and DAB TO FM linking and implicit linking not happened then only start the timer 10sec*/
					if (pst_me_radio_mngr_inst->b_TimerFlag != RADIO_MNGR_APP_SET_TIMER_FLAG && pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS &&
						pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS)
					{
						/*setting timer flag as one,to know while stopping timer*/
						pst_me_radio_mngr_inst->b_TimerFlag = RADIO_MNGR_APP_SET_TIMER_FLAG;

						/*If quality reduces wait for 10 sec before clearing label*/
						st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid = SYS_StartTimer(RADIO_MNGR_APP_DAB_MAX_TIME_QUALITY_RESUME, RADIO_MNGR_APP_LOW_QUALITY_TIMEOUT, RADIO_MNGR_APP);
						if (st_Radio_Mngr_App_TimerID.u32_LowSig_ClearLabel_Timerid == 0)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][RM]  : DAB label clear timer failed ");
						}
						else{/*FOR MISRA C*/ }
					}
					else{/*FOR MISRA C*/ }
				}
			}else{/*FOR MISRA C*/}
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS && 
				        pst_me_radio_mngr_inst->e_DABFM_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS &&
						pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_DAB,
																pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
			}
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}
			
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



/*=====================================================================================================================================================================================================================*/
/*														Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_StationlistHndlr																																			   */
/*=====================================================================================================================================================================================================================*/

Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_StationlistHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;
	
	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
       case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_StationlistHndlr Msg");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_StationlistHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_StationlistHndlr \n"));
			
			if (pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id != RADIO_MNGR_APP_IN_STRATEGY && 
					pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id != RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION)
			{
				/*Requesting to the Active Band Application layer for the Station list */
				switch(pst_me_radio_mngr_inst->e_activeBand)
				{
					case RADIO_MNGR_APP_BAND_AM:
					{					
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AM In Scan..");

						AMFM_App_Request_GetStationList((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);
					}
					break;

					case RADIO_MNGR_APP_BAND_FM:
					{				
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM In Scan..");

						AMFM_App_Request_GetStationList((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);
					}
					break;

					case RADIO_MNGR_APP_BAND_DAB:
					{
						if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB In Scan..");
						
							DAB_App_Request_GetStationList();
						}
						else
						{
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
						}
					}
					break;
		
					default:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
					break;
				}

				/*Notification to the HMI about Scan Status as Started*/
				Radio_Mngr_App_Notify_UpdateScanStatus(RADIO_MNGR_APP_SCAN_STARTED);

				/*Notification to the HMI about Scan Status as Inprogress*/
				Radio_Mngr_App_Notify_UpdateScanStatus(RADIO_MNGR_APP_SCAN_INPROGRESS);
			
				pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_IN_SCAN;
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
				
			}
			else
			{
				if (pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION)
				{
					pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_UPDATE_STLIST;
				}
				else
				{
					/*FOR MISRA C*/
				}	
			}
		}
        break;
				
		

/**************************************************************************************************************************************************************************************************************
*******************************************************************************AM STATION LIST**************************************************************************************************/

		/*Response for AM station list Scan completed from App Layer*/
		case RADIO_MNGR_APP_AM_STATIONLIST_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_GetAMstationlistreplystatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_GetAMstationlistreplystatus),&(pst_me_radio_mngr_inst->u32_slot));
			memset((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN), 0, RADIO_MNGR_APP_CHAN_NAME);
		}
		break;

		/*Notification of the AM Stationlist Updation completed in Shared Memory by the App Layer*/
		case RADIO_MNGR_APP_AM_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
				
			SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

			/* Read the AM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_AM);

			SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);
			
			/*Notification to the HMI about Scan Status as Completed*/
			Radio_Mngr_App_Notify_UpdateScanStatus(RADIO_MNGR_APP_SCAN_COMPLETE);
			
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AM Station List Updated");

			/*Notification to the HMI about UpdateSTL_Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_UpdateSTL_Diag(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}

			if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_UPDATE_STLIST && pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
			{
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
				Radio_Mngr_App_Response_Update_StationList(pst_me_radio_mngr_inst->e_GetAMstationlistreplystatus);
				Update_MatchedStationListIndex(pst_me_radio_mngr_inst);
				Radio_Mngr_App_Notify_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_AM);
				Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_AM);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}

			else if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PLAY_SELECT_STATION || pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_STATIONLIST_SELECT || 
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PRESET_RECALL)
			{
				/*Req ID updated with INIT Scan which can use in play select and demute state*/
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_INIT_SCAN;

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}
			else{/*FOR MISRA C*/}
		}
		break;

/**************************************************************************************************************************************************************************************************************
*******************************************************************************FM STATION LIST*****************************************************************************************************/
		
		/*Response for FM station list Scan completed from App Layer*/
		case RADIO_MNGR_APP_FM_STATIONLIST_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus),&(pst_me_radio_mngr_inst->u32_slot));
		}
		break;
		
		/*Notification of the FM Stationlist Updation completed in Shared Memory by the App Layer*/
		case RADIO_MNGR_APP_FM_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			SYS_MUTEX_LOCK(STL_RM_AMFM_APP);

			/* Read the FM Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);

			SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);

			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				/*Notification to the HMI about Scan Status as Completed*/
				Radio_Mngr_App_Notify_UpdateScanStatus(RADIO_MNGR_APP_SCAN_COMPLETE);
			}else{/*FOR MISRA C*/}

			/*Notification to the HMI about UpdateSTL_Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_UpdateSTL_Diag(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FM Station List Updated in Foreground");
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FM Station List Updated in Background");
			}

			if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_UPDATE_STLIST && pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
				Radio_Mngr_App_Response_Update_StationList(pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus);
				Update_MatchedStationListIndex(pst_me_radio_mngr_inst);
				Radio_Mngr_App_Notify_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
				Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}

			/*Only for the Req id playselect station*/
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->b_EEPROM_Status == EEPROM_UNKNOWN_VALUES && pst_me_radio_mngr_inst->b_FM_StartStatus != COLD_START_DONE_ALREADY)
			{
				/*Req ID updated with INIT Scan which can use in play select and demute state*/
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_INIT_SCAN;

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}
		}
		break;

/**************************************************************************************************************************************************************************************************************
*********************************************************************************DAB STATION LIST*****************************************************************************************************/

		/*Response for DAB station list Scan completed from App Layer*/
		case RADIO_MNGR_APP_DAB_STATIONLIST_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus),&(pst_me_radio_mngr_inst->u32_slot));
		}
		break;

		/* Handle the Play first station Response after scanning each ensemble */
		case RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst->u32_slot));
			Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst->st_CurrentStationName));
			pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION;
			pst_me_radio_mngr_inst->e_Curr_Audio_Band = RADIO_MNGR_APP_BAND_DAB;
			Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_DAB,
																	pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																	pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																	pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																	pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																	pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																	pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
			/*If select station response is success then transiting to demute state*/
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
		}

		/*Notification of the DAB Stationlist Updation completed in Shared Memory by the App Layer*/
		case RADIO_MNGR_APP_DAB_STATIONLIST_UPDATE_DONE_NOTIFYID:
		{
			SYS_MUTEX_LOCK(STL_RM_DAB_APP);

			/* Read the DAB Station List from the Shared memory */
			Update_Radio_Mngr_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);	

			SYS_MUTEX_UNLOCK(STL_RM_DAB_APP);
			
			/*Creating Multiplex & Normal Stationlist*/
			Radio_Mngr_App_CreateNormalRadioStationList(pst_me_radio_mngr_inst);
			Radio_Mngr_App_CreateMultiplexRadioStationList(pst_me_radio_mngr_inst);

			Update_MatchedStationListIndex(pst_me_radio_mngr_inst);

			Radio_Mngr_App_Notify_StationList(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
			{
				/*Notification to the HMI about Scan Status as Completed*/
				Radio_Mngr_App_Notify_UpdateScanStatus(RADIO_MNGR_APP_SCAN_COMPLETE);
			}else{/*FOR MISRA C*/}

			/*Notification to the HMI about UpdateSTL_Diag info, when ENG mode is ON*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON)
			{
				Radio_Mngr_App_Notify_UpdateSTL_Diag(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DAB Station List Updated in Foreground");
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]DAB Station List Updated in Background");
			}

			if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_UPDATE_STLIST && pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Manager_App_Write_Flash_Data(pst_me_radio_mngr_inst);
				Radio_Mngr_App_Response_Update_StationList(pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus);
				Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}

			/*Only for the Req id playselect station*/
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->b_EEPROM_Status == EEPROM_UNKNOWN_VALUES && pst_me_radio_mngr_inst->b_DAB_StartStatus != COLD_START_DONE_ALREADY)
			{
				/*Req ID updated with INIT Scan which can use in play select and demute state*/
				pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_INIT_SCAN;

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}
		}
		break;

		/*Noficatio of each Frequency change in DAB Scan */
		case RADIO_MNGR_APP_DAB_FREQ_CHANGE_NOTIFYID:
		{
			if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_Curr_Audio_Band == RADIO_MNGR_APP_BAND_DAB)
			{
				memset(&(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo), 0, sizeof(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo));
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_Tunableinfo.u32_Frequency), (pst_msg->data), sizeof(Tu32), &(pst_me_radio_mngr_inst->u32_slot));
				ExtractParameterFromMsg((pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName), &(pst_me_radio_mngr_inst->u32_slot));
				Radio_Mngr_App_Response_UpdateCurStationInfo_Display(pst_me_radio_mngr_inst->e_activeBand,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_Tunableinfo.u32_Frequency,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_ServiceLabel.au8_Label,
																			(Tu8)RADIO_MNGR_APP_CHARSET_UTF8, (Tu8*)NULL,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.au8_ChannelName,
																			pst_me_radio_mngr_inst->st_DAB_FreqChangeNotinfo.st_EnsembleLabel.au8_Label,
																			pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/ }
		}
		break;
			
/**************************************************************************************************************************************************************************************************************
***********************************************************Scan cancel by New request from user***************************************************************************************************************/
        case RADIO_MNGR_APP_SCAN_CANCEL_REQ:
		{
			if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in StL Handler");
				Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}
			
			switch(pst_msg->msg_id)
			{		
				case RADIO_MNGR_APP_TUNEUPDOWN_REQID:
				{
					/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Request function to cancel both Seek and Scan, passing arguement as cancel for seek or scan*/
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
						}
					}
					else{/*FOR MISRA C*/}

				}
				break;
				case RADIO_MNGR_APP_SEEKUPDOWN_REQID:
				{
					/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Request function to cancel both Seek and Scan, passing arguement as cancel for seek or scan*/
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
							}
					}
					else{/*FOR MISRA C*/}
				}
				break;
					
				case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:
				{
					/* Updating the requested station index into the inst hsm structure*/
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Index),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->u8_Index),&(pst_me_radio_mngr_inst->u32_slot));
					/*Comparing the Index selected from HMI & Number of stations present in stationlist*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList) ||
							(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList) ||
							(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList))
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
								if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
								{
									/*Request function to cancel both Seek and Scan, passing arguement as cancel for seek or scan*/
									DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
								}
								else
								{
									DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
								}
						}
						else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				break;
				
				/*Request to process & cancel Seek*/
				case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_ServiceIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_ServiceIndex < 
								pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble)
					{
						/*Copying msg to local buffer, it is used when we pass request to parent handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
						}
					}
					else{/*FOR MISRA C*/}
				}
				break;
				
				/*Request to process & cancel Seek*/
				case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
	
					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList) ||				 
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList))
					{
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							/*Cancel Request to the AMFM App layer for seek cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				break;

				/*When select band come from HMI, we need cancel the ongoing Scan and switch to selected band*/
				case RADIO_MNGR_APP_SELECTBAND_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ScanCancelRequestedBand),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_ScanCancelRequestedBand),&(pst_me_radio_mngr_inst->u32_slot));
					if(pst_me_radio_mngr_inst->e_ScanCancelRequestedBand != pst_me_radio_mngr_inst->e_activeBand)
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
	                    /*Checking the availability of DAB band along with active band as AM/FM*/
						pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue = Radio_Mngr_App_SelectBandConditionCheck(pst_me_radio_mngr_inst->e_ScanCancelRequestedBand, pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
				
						/*Based on the function return value requesting FM/DAB for scan cancel*/
						if(pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_AMFM_CANCEL)
						{
							/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_DAB_CANCEL)
						{
							if(pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
							{
								/*Request function to cancel both Seek and Scan, passing argument as cancel for seek or scan*/
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
							}
						}else{/*FOR MISRA C*/}
					}else{/*If active and requested band both are same then, req have to ignore*/}
				}
				break;

				/*When preset index is valid then give cancel Scan request*/
				case RADIO_MNGR_APP_PRESET_RECALL_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Preset_Recall_Index),(pst_msg->data),sizeof(Tu8),&(pst_me_radio_mngr_inst->u32_slot));
			
					if(Radio_Mngr_App_PresetRecallConditionCheck(pst_me_radio_mngr_inst))
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
							AMFM_App_Request_Cancel();
						}
						else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Request function to cancel both Seek and Scan, passing arguement as cancel for seek or scan*/
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
							}
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				break;

				/*Request to process Refresh station list request & cancel internal scan*/
				case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:
				{			
					/*Whenever Internal Scan is ongoing, if user gives UpdateStl, then no need to cancel the scan, just reuse the ongoing scan for the User Request by updating reqid*/
					pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_UPDATE_STLIST;					
				}
				break;
									
				/*When tune by frequency come from HMI, we need cancel the ongoing Scan and Process tune by frequency request*/
				case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:
				{
					/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Request function to cancel both Seek and Scan, passing arguement as cancel for seek or scan*/
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
						}
					}
					else{/*FOR MISRA C*/}
				}
				break;
				/*When Power OFF request comes from HMI, we need cancel the ongoing Scan and Process tune by frequency request*/
				case RADIO_MNGR_APP_POWER_OFF_REQID:
				{
					/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Request function to cancel both Seek and Scan, passing arguement as cancel for seek or scan*/
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
						}
					}
					else{/*FOR MISRA C*/}
				}
				break;

				default:
				{
			
				}
				break;
			}
		}
		break;
	
		case RADIO_MNGR_APP_MANUAL_STLUPDATE_CANCEL_REQID:
		{
			/*Updating the Scan Cancel Request Type as Cancel by HMI, which will be helpful after transiting to active idle state */
			pst_me_radio_mngr_inst->e_ScanCancel_Request_Type = RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI;
			
			switch(pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_AM:
				case RADIO_MNGR_APP_BAND_FM:
				{
					/*Requesting AMFM Application to cancel Manual updation of station List*/
					AMFM_App_Request_Cancel();
				}	
				break;
		
				case RADIO_MNGR_APP_BAND_DAB:
				{	
					if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
					{
						/*Request function to cancel both Seek and Scan, passing arguement as cancel for seek or scan*/
						DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SCAN_CANCEL);
					}
					else
					{
						DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SCAN_CANCEL);
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

		/*Cancel Done Response id received from AMFM App layer*/	
		case RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID:
		{
			/*Extracting the Scan cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Notify_UpdateScanStatus(RADIO_MNGR_APP_SCAN_COMPLETE);
			}else{/*FOR MISRA C*/}

			/*Scan Cancel done successfully and Scan cancel Request type is Not From HMI button then transiting to idle state to handle new request*/
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS && pst_me_radio_mngr_inst->e_ScanCancel_Request_Type != RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AM/FM Scan Cancelled due to New Request");
				
				/*Setting the Scan cancel Flag, so as we can use the same in Active idle handler*/
				pst_me_radio_mngr_inst->b_Scan_Cancel_Status = RADIO_MNGR_APP_SCAN_CANCELLED;
				
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
			}else{/*FOR MISRA C*/}
		
			/*Scan Cancel done successfully and Scan cancel Request type is From HMI button then requesting active band to cancel the scan and transitiong to play select stn to play last station*/
			if(pst_me_radio_mngr_inst->e_ScanCancel_Request_Type == RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				pst_me_radio_mngr_inst->e_ScanCancel_Request_Type = RADIO_MNGR_APP_SCAN_CANCEL_INVALID;
			
				/*Sending the Scan cancel success response to HMI*/
				Radio_Mngr_App_Response_CancelManualSTlUpdate(REPLYSTATUS_SUCCESS);
		
				/*Tune to original Station if external scan is cancelled from HMI based on band*/
				switch(pst_me_radio_mngr_inst->e_activeBand)
				{
					case RADIO_MNGR_APP_BAND_AM:
					{
						Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_AM);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);	
					}
					break;
			
					case RADIO_MNGR_APP_BAND_FM:
					{
						Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
					}
					break;
			
					default:
					break;
				}	
			}
			else if(pst_me_radio_mngr_inst->e_ScanCancel_Request_Type == RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI && pst_me_radio_mngr_inst->e_ReplyStatus != REPLYSTATUS_SUCCESS)
			{
				/*Sending the Scan cancel Failure response to HMI*/
				Radio_Mngr_App_Response_CancelManualSTlUpdate(REPLYSTATUS_FAILURE);	
			}
			else{/*FOR MISRA C*/}
		}
		break;

		/*Cancel Done Response id received from DAB App layer*/	
		case RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID:
		{
			/*Extracting the Scan cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
		
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Notify_UpdateScanStatus(RADIO_MNGR_APP_SCAN_COMPLETE);
			}else{/*FOR MISRA C*/}
			/*Scan Cancel done successfully and Scan cancel Request type is Not From HMI button then transiting to idle state to handle new request*/
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS && pst_me_radio_mngr_inst->e_ScanCancel_Request_Type != RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB Scan Cancelled due to New Request");
				
				/*Setting the Scan cancel Flag, so as we can use the same in Active idle handler*/
				pst_me_radio_mngr_inst->b_Scan_Cancel_Status = RADIO_MNGR_APP_SCAN_CANCELLED;
		
				/*Transiting to Active Idle so as to handle HMI Requested request after scan cancelled*/
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
			}else{/*FOR MISRA C*/}
	
			/*Scan Cancel done successfully and Scan cancel Request type is From HMI button then requesting active band to cancel the scan and transitiong to play select stn to play last station*/
			if(pst_me_radio_mngr_inst->e_ScanCancel_Request_Type == RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				pst_me_radio_mngr_inst->e_ScanCancel_Request_Type = RADIO_MNGR_APP_SCAN_CANCEL_INVALID;
			
				/*If Manual update is cancelled through HMI Button then after cancelling tune to original station*/
				Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}else{/*FOR MISRA C*/}
		}
		break;

/**************************************************************************************************************************************************************************************************************/

		case HSM_MSGID_EXIT:
        {
			/*do nothing */
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectBandHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectBandHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);

	switch (pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
        {
			/*do nothing */
        }
        break;

		case HSM_MSGID_START:
        { 
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectBandHndlr");
			SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectBandHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_SelectBandHndlr \n"));

			Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, RADIO_MNGR_APP_LISTENING);			

			/* In Cold startup by default FM band has to be select*/
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_INVALID && 
					pst_me_radio_mngr_inst->b_EEPROM_Status == EEPROM_UNKNOWN_VALUES)
			{
				pst_me_radio_mngr_inst->e_requestedBand  = RADIO_MNGR_APP_BAND_FM;
				pst_me_radio_mngr_inst->u8_Original_Band = (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
				AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
			}
			
			/* In warm startup Select the LSM Band*/
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_INVALID && 
						pst_me_radio_mngr_inst->b_EEPROM_Status == EEPROM_KNOWN_VALUES)
			{
				/*Update the Request Band with the LSM Band*/
				pst_me_radio_mngr_inst->e_requestedBand	   = (Te_Radio_Mngr_App_Band)(pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band);
				pst_me_radio_mngr_inst->u8_Original_Band   = (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);

				Radio_Mngr_App_ClearCheckParameters(pst_me_radio_mngr_inst);
				switch(pst_me_radio_mngr_inst->e_requestedBand)
				{
					case RADIO_MNGR_APP_BAND_AM:  																
					case RADIO_MNGR_APP_BAND_FM:
					{
						/*DAB in background Started*/
						AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
					}
					break;

					case RADIO_MNGR_APP_BAND_DAB:
					{
						if(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED)
						{
							DAB_App_Request_SelectBand();
						}
						else
						{
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);							
						}
					}
					break;

					default:
					break;
				}
			}
			
			/*Checking active band as Non-radio maode then only move for requesting select band*/
			else if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_RADIOMODE && pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_NON_RADIO_MODE)
			{
				
				pst_me_radio_mngr_inst->e_requestedBand			= (Te_Radio_Mngr_App_Band)(pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band);
				pst_me_radio_mngr_inst->u8_Original_Band		= (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
			    pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_SELECT_STATION;

			    switch(pst_me_radio_mngr_inst->e_requestedBand)
			    {
				    case RADIO_MNGR_APP_BAND_AM:  																
				    case RADIO_MNGR_APP_BAND_FM:
				    {
					    /*DAB in backround Started*/
					    AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
				    }
				    break;

				    case RADIO_MNGR_APP_BAND_DAB:
				    {
					    DAB_App_Request_SelectBand();
				    }
				    break;

				    default:
				    break;
				}
			}
			
			/*Handling Radio Power ON Functionalities*/
			else if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_RADIO_POWER_ON)
			{
				pst_me_radio_mngr_inst->e_requestedBand			= (Te_Radio_Mngr_App_Band)(pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band);
				pst_me_radio_mngr_inst->u8_Original_Band		= (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
			    pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_PLAY_SELECT_STATION;
				
				/*Sending Power On success response to HMI*/
				Radio_Mngr_App_Response_Power_ON(REPLYSTATUS_SUCCESS);

			    switch(pst_me_radio_mngr_inst->e_requestedBand)
			    {
				    case RADIO_MNGR_APP_BAND_AM:  																
				    case RADIO_MNGR_APP_BAND_FM:
				    {
					    /*DAB in background Started*/
					    AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
				    }
				    break;

				    case RADIO_MNGR_APP_BAND_DAB:
				    {
					    DAB_App_Request_SelectBand();
				    }
				    break;

				    default:
				    break;
				}	
			}
			
			/*Handling Radio Power OFF Functionalities*/
			else if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_RADIO_POWER_OFF)
			{
				/* updating the last band variable while deselect the active band*/
				pst_me_radio_mngr_inst->e_lastband = pst_me_radio_mngr_inst->e_activeBand;
				
				/*Sending Power Off success response to HMI*/
				Radio_Mngr_App_Response_Power_OFF(REPLYSTATUS_SUCCESS);
				
				switch(pst_me_radio_mngr_inst->e_activeBand)
				{
					case RADIO_MNGR_APP_BAND_AM:
					case RADIO_MNGR_APP_BAND_FM:
					{
						/*Inform to bg band(DAB) to stop the linking activity and continue bg scan process*/
						DAB_APP_Notify_StartBackgroundScan();
						
						AMFM_App_Request_DeSelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);	
					}
					break;
					
					case RADIO_MNGR_APP_BAND_DAB:
					{
						/*Inform to bg band(AMFM) to stop the linking activity and continue bg scan process*/
						AMFM_App_Notify_StartBackgroundScan();
						
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							DAB_App_Request_DeSelectBand();
						}
						else
						{
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
						}
					}
					break; 
					
					default:
					break;
 				}			
			}
			
			/*Handling the Non-Radio mode functionalities, i.e deselect the active band request sending to application layer*/
			else if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_NONRADIOMODE)
			{
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
				{	
					/*Inform to bg band(DAB) to stop the linking activity and continue bg scan process*/
					DAB_APP_Notify_StartBackgroundScan();
						
					/*Request for deselect AM/FM band to app layer*/
					AMFM_App_Request_DeSelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);
				}
				else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
				{
					/*Inform to bg band(AMFM) to stop the linking activity and continue bg scan process*/
					AMFM_App_Notify_StartBackgroundScan();
						
					if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
					{	
						/*Request for deselect DAB band to app layer*/
						DAB_App_Request_DeSelectBand();
					}
					else
					{
						pst_me_radio_mngr_inst->e_activeBand = RADIO_MNGR_APP_NON_RADIO_MODE;
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
				}
				else{/*FOR MISRA C*/}
			}

			/*If the Active Band and Requested Bands are not equal*/
			else if(pst_me_radio_mngr_inst->e_activeBand != pst_me_radio_mngr_inst->e_requestedBand)
			{
				/* updating the last band variable while deselect the active band*/
				pst_me_radio_mngr_inst->e_lastband = pst_me_radio_mngr_inst->e_activeBand;

				/*In select band first deselect the Active band before select the requested band*/
				switch(pst_me_radio_mngr_inst->e_activeBand)
				{
					case RADIO_MNGR_APP_BAND_AM:
					case RADIO_MNGR_APP_BAND_FM:
					{
						/*If the Requested Band DAB and Active Band AM/FM, then only do Deselect AM/FM*/
						if(pst_me_radio_mngr_inst->e_requestedBand == RADIO_MNGR_APP_BAND_DAB)
						{
							AMFM_App_Request_DeSelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);	
						}
						else
						{
							/*Update the Active band with Requested Band*/
							pst_me_radio_mngr_inst->e_activeBand = pst_me_radio_mngr_inst->e_requestedBand;

							/*Update the LSM Band with Active Band*/
							pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);

							if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
							{
								pst_me_radio_mngr_inst->e_Curr_Audio_Band = RADIO_MNGR_APP_BAND_FM;
							}
							else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
							{
								pst_me_radio_mngr_inst->e_Curr_Audio_Band = RADIO_MNGR_APP_BAND_AM;
							}else{/*FOR MISRA C*/}
							
							/*Send the Response to the HMI for Select Band Request*/
							Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_SUCCESS, pst_me_radio_mngr_inst->e_activeBand);
							
							/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
							pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																								&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																							
							Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);

							/*State Transit to the Mute before Play the Last Station*/
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
						}
					}
					break;

					case RADIO_MNGR_APP_BAND_DAB:
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							DAB_App_Request_DeSelectBand();	
						}
						else
						{
							AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
						}
					}
					break;
					
					case RADIO_MNGR_APP_NON_RADIO_MODE:
					{
						switch(pst_me_radio_mngr_inst->e_requestedBand)
						{
							case RADIO_MNGR_APP_BAND_AM:
							{
								AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
							}
							break;
							
							case RADIO_MNGR_APP_BAND_FM:
							{
								AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
							}
							break;
							
							case RADIO_MNGR_APP_BAND_DAB:
							{
								DAB_App_Request_SelectBand();
							}
							break;
							
							default:
							{
								HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
							}
							break;
						}
					}
					break;				
						
					default:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
					break;
				}
			}
			else
			{
				/* Notify the Frequency and Station information if Select Band request is received for same Band  and move state to Idle*/
				Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst); 
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
			}	
		}
		break;
		
		/*Response from the APP layers for the Deselct the Active Band*/
		case RADIO_MNGR_APP_DAB_DESELECTBAND_DONE_RESID:
		case RADIO_MNGR_APP_AMFM_DESELECTBAND_DONE_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Deselected the Band:%d", pst_me_radio_mngr_inst->e_activeBand);
			
			if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_NONRADIOMODE)
			{
				pst_me_radio_mngr_inst->e_activeBand = RADIO_MNGR_APP_NON_RADIO_MODE;
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
			}
			else if (pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_RADIO_POWER_OFF)
			{
				//pst_me_radio_mngr_inst->e_activeBand = RADIO_MNGR_APP_NON_RADIO_MODE;
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);				
			}
			else
			{
				/* After Deactivate the active band then select the Requested band*/
				switch(pst_me_radio_mngr_inst->e_requestedBand)
				{
				
					case RADIO_MNGR_APP_BAND_AM:
					case RADIO_MNGR_APP_BAND_FM:
					{
						ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
						if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
						{
							AMFM_App_Request_SelectBand((Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_requestedBand);
						}
						else
						{
							/*If the Deselct of active band failure then send Fail response to HMI and state move to Idle.*/
							Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_FAILURE, pst_me_radio_mngr_inst->e_activeBand);
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
						}
					}
					break;	
			
					case RADIO_MNGR_APP_BAND_DAB:
					{	
						ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_ReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));
						if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
						{
							DAB_App_Request_SelectBand();	
						}
						else
						{
							/*If the Deselct of active band failure then send Fail response to HMI and state move to Idle.*/
							Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_FAILURE, pst_me_radio_mngr_inst->e_activeBand);
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
						}
					}
					break;
				
					default:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
					break;
				}
			}		
		}
		break;
		
		/*Response from the AMFM App layer for the Select Band request*/
		case RADIO_MNGR_APP_AMFM_SELECTBAND_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				/*Update the Active band with Requested Band*/
				pst_me_radio_mngr_inst->e_activeBand = pst_me_radio_mngr_inst->e_requestedBand;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Selected the Band:%d", pst_me_radio_mngr_inst->e_activeBand);
				
				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
				{
					pst_me_radio_mngr_inst->e_Curr_Audio_Band = RADIO_MNGR_APP_BAND_FM;
				}
				else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
				{
					pst_me_radio_mngr_inst->e_Curr_Audio_Band = RADIO_MNGR_APP_BAND_AM;
				}else{/*FOR MISRA C*/}
				
				/*Send the response to the HMI about Selected Band*/
				Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_SUCCESS, pst_me_radio_mngr_inst->e_activeBand);

				/*Update the LSM Band with Active Band*/
				pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
			
				/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
				pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																									&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																								
				Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
				

				/*Check, if already muted then directly go to play select state*/
				if(pst_me_radio_mngr_inst->b_Radio_Mute_state == RADIO_MNGR_APP_VALUE_ZERO)
				{
					/*State Transit to the Mute before Play the Station*/
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
				}
				else
				{
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
				}
			}
			else
			{
				/*If the Deselct of active band failure then send Fail response to HMI and state move to Idle.*/
				Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_FAILURE, pst_me_radio_mngr_inst->e_activeBand);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
			}
		}
		break;
		
		/*Response from the DAB App layer for the Select Band request*/
		case RADIO_MNGR_APP_DAB_SELECTBAND_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				/*Update the Active band with Requested Band*/
				pst_me_radio_mngr_inst->e_activeBand = pst_me_radio_mngr_inst->e_requestedBand;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Selected the Band:%d", pst_me_radio_mngr_inst->e_activeBand);

				pst_me_radio_mngr_inst->e_Curr_Audio_Band = RADIO_MNGR_APP_BAND_DAB;
				/*Send the response to the HMI about Selected Band*/
				Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_SUCCESS, pst_me_radio_mngr_inst->e_activeBand);

				/*Update the LSM Band with Active Band*/
				pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
			
				/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
				pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																									&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																								
				Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
				

				/*Check, if already muted then directly go to play select state*/
				if(pst_me_radio_mngr_inst->b_Radio_Mute_state == RADIO_MNGR_APP_VALUE_ZERO)
				{
					/*State Transit to the Mute before Play the Station*/
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
				}
				else
				{
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
				}
			}
			else
			{
				/*If the Deselct of active band failure then send Fail response to HMI and state move to Idle.*/
				Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_FAILURE, pst_me_radio_mngr_inst->e_activeBand);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
			}
		}
		break;
		
		case HSM_MSGID_EXIT:
        {
			/* do nothing*/
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_MuteHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_MuteHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_MuteHndlr");
            SYS_RADIO_MEMCPY((void*) pst_me_radio_mngr_inst->u8p_curr_state_str,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_MuteHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_MuteHndlr \n"));
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Mute Requested");
			/*Request to Audio Manager for Mute*/
			Audio_Manager_Request_Mute(pst_me_radio_mngr_inst->e_activeBand);
		}
		break;

		/*Response from the Audio Manager for the Mute Request*/
		case RADIO_MNGR_APP_MUTE_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_ReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Mute Done ResID Received");
				pst_me_radio_mngr_inst->b_Radio_Mute_state = MUTE_DONE;
				switch(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id)
				{
					case RADIO_MNGR_APP_PLAY_SELECT_STATION:
					case RADIO_MNGR_APP_STATIONLIST_SELECT:
					case RADIO_MNGR_APP_PRESET_RECALL:
					case RADIO_MNGR_APP_SERVICE_RECONFIG:
					case RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL:
					{	
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
						{
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_af_tune_state);
						}
						else
						{
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
						}
					}
					break;

					case RADIO_MNGR_APP_IN_STRATEGY:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_af_tune_state);					
					}
					break;

					case RADIO_MNGR_APP_SEEK_UPDOWN:
					{	
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_seekupdown_state);
					}
					break;

					case RADIO_MNGR_APP_TUNEUPDOWN:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_tuneupdown);
					}
					break;

					case RADIO_MNGR_APP_UPDATE_STLIST:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_stationlist_state);
					}
					break;

					case RADIO_MNGR_APP_TUNEBYFREQ:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_tune_by_frequency_state);
					}
					break;
					
					case RADIO_MNGR_APP_SELECT_STATION_END:
					{
						Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
					}
					break;

					case RADIO_MNGR_APP_PI_NOT_FOUND_AF_TUNE:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_af_tune_state);
					}
					break;

					default:
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
					break;
				}
			}
			else
			{
				/*If the Mute fails, state transit to the Idle*/
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_Play_SelectStationHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_Play_SelectStationHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);

	switch(pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
		{
        
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_Play_SelectStationHndlr Msg");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_Play_SelectStationHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_Play_SelectStationHndlr \n"));

			/* Request to the application layer to select the station based on Active Band */
			switch (pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_AM:
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AM Tune Request with Freq: %d", pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq);
					AMFM_App_Request_PlaySelectSt(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq, (Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);
				}
				break;

				case RADIO_MNGR_APP_BAND_FM:
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM Tune Request with Freq: %d, PI: %d", pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq, pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI);
					AMFM_App_Request_PlaySelectSt(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq, (Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);
				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB Tune Request with Freq: %d, SID: %d", pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq, pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid);
					DAB_App_Request_PlaySelectSt(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq,
															pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_EId,
															pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid,
															pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI);
				}
				break;

				default:
				{
					 HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}
				break;
			 }
		 }
		 break;
		
		/*Response from the APP layers for the Tune Request*/
		case RADIO_MNGR_APP_DAB_PLAY_SELECT_STATION_DONE_RESID:
		case RADIO_MNGR_APP_AMFM_PLAY_SELECT_STATION_DONE_RESID:
		{			
			if(pst_me_radio_mngr_inst->b_TuneCancelReqStatus != RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED)
			{		
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SelectStationReplyStatus),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_SelectStationReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));

			/*If the Req Id is preset recall then copying preset recall reply status*/
			if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PRESET_RECALL)
			SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->e_PresetRecallReplayStatus), &(pst_me_radio_mngr_inst->e_SelectStationReplyStatus), sizeof(Te_RADIO_ReplyStatus));
			else{/*FOR MISRA C*/}

			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				memset((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText), 0, RADIO_MNGR_APP_CHAN_RADIOTEXT);
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
			}
			else
			{	
				memset(&(pst_me_radio_mngr_inst->st_DLS_Data), 0, sizeof(Ts_Radio_Mngr_App_DLS_Data));
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
				Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
			}

			switch(pst_me_radio_mngr_inst->e_SelectStationReplyStatus)
			{
					case REPLYSTATUS_SUCCESS:
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Tune Success for Band:%d", pst_me_radio_mngr_inst->e_activeBand);
						
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PRESET_RECALL)
						{
							pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck = RADIO_MNGR_APP_PRESET_FM_SUCCESS;
						
							Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.au8_PSN,
																		pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u8_CharSet, 
																		(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
						}
						
						else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_STATIONLIST_SELECT)
						{
							pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck = RADIO_MNGR_APP_STATIONLIST_FM_SUCCESS;
						
							Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].au8_PSN,
																		pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_CharSet, 
																			(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
						}
						
						else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL)
						{
							pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck = RADIO_MNGR_APP_STATIONLIST_FM_SUCCESS;
							
							if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH)
							{
								Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].au8_PSN,
																		pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_CharSet, 
																			(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
							}
							else{/*FOR MISRA C*/}
						}
						
						else{/*FOR MISRA C*/}
						
						
						if(pst_me_radio_mngr_inst->e_activeBand != RADIO_MNGR_APP_BAND_FM)
						{
							/*update the LSM structure if the signal is available*/
							Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
						
							/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
							pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																												&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																											
							Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
						}else{/*FOR MISRA C*/}
						
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && 
								((pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_STATIONLIST_SELECT &&
									pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].u16_PI == RADIO_MNGR_APP_VALUE_ZERO)||
								(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PRESET_RECALL &&
									pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.u16_PI == RADIO_MNGR_APP_VALUE_ZERO)))
								
						{
							/*update the LSM structure if the signal is available*/
							Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
						
							/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
							pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																												&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																											
							Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
						}else{/*FOR MISRA C*/}

						/*send UpdateCurStationInfo to HMI if active band is DAB,no need to send if active band is FM ,because in AMFM tuner status notification we will get currentstation info*/
						if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON && pst_me_radio_mngr_inst->e_activeBand ==RADIO_MNGR_APP_BAND_DAB)
						{
							Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst);
						}
						else{/*FOR MISRA C*/}

						/*If select station response is success then transiting to demute state*/
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
					}
					break;

	                /*All failure cases for select station request are handled within no signal case here*/
					case REPLYSTATUS_NO_SIGNAL:
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]Tune Fail for Band:%d", pst_me_radio_mngr_inst->e_activeBand);
				
	                    /*Based on the request Id performing respective operation*/
						switch(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id)
						{
							case RADIO_MNGR_APP_PLAY_SELECT_STATION:
							case RADIO_MNGR_APP_STATIONLIST_SELECT:
							case RADIO_MNGR_APP_PRESET_RECALL:
							case RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL:
							{
							
								pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_STATION_NOT_AVAILABLE;
								Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
							
								/*For Normal Select Band Case if no signal, then go to demute state*/
								switch(pst_me_radio_mngr_inst->e_activeBand)
								{
									case RADIO_MNGR_APP_BAND_AM:
									{
										Update_LSM_Station_Info(pst_me_radio_mngr_inst);

										/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
										pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
										Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
#if 0
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Strategy for AM, Do Internal Scan");
									
										HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_stationlist_state);
#endif
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[Radio][RM]Temporarily disable stl scan, so Going to Demute and Listen State");
										pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
										HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
									}
									break;

									case RADIO_MNGR_APP_BAND_FM:
									{
										if((pst_me_radio_mngr_inst->b_FM_StartStatus != COLD_START_DONE_ALREADY) && 
															pst_me_radio_mngr_inst->b_EEPROM_Status == EEPROM_UNKNOWN_VALUES)
										{
											Update_LSM_Station_Info(pst_me_radio_mngr_inst);

											/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
											pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
											Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);

#if 0
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Strategy for FM Do Internal Scan");
										
											HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_stationlist_state);
#endif
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[Radio][RM]Temporarily disable stl scan at coldstartup, so Going to Demute and Listen State");
											pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
											HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);

										}
										else if(pst_me_radio_mngr_inst->e_RDSSettings == RADIO_MNGR_APP_RDS_SETTINGS_ENABLE)
										{
											Update_LSM_Station_Info(pst_me_radio_mngr_inst);

											/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
											pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																									
											Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
#if 0																									
											if(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI != RADIO_MNGR_APP_VALUE_ZERO)
											{																										
												RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Strategy for Band FM, Do AF");

												Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START);										
									
												HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_af_tune_state);
											}
											else
											{
												RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Since PI is not available, so Going to Demute and Listen State");
												pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
												HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
											}
#endif
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[Radio][RM]Since PI is not available, so Going to Demute and Listen State");
											pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
											HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
										}
										else
										{
											/*update the LSM structure if the signal is available*/
											Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
											/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
											pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																																&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																									
											Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
											pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
											HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
										}
									}
									break;
										
									case RADIO_MNGR_APP_BAND_DAB:
									{
										if((pst_me_radio_mngr_inst->b_DAB_StartStatus != COLD_START_DONE_ALREADY) && 
													pst_me_radio_mngr_inst->b_EEPROM_Status == EEPROM_UNKNOWN_VALUES)
										{
											Update_LSM_Station_Info(pst_me_radio_mngr_inst);

											/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
											pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																									
											Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
									
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Strategy for Band DAB, Do Internal Scan");
									
											HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_stationlist_state);
										}
										else if(pst_me_radio_mngr_inst->e_RDSSettings == RADIO_MNGR_APP_RDS_SETTINGS_ENABLE)
										{
											Update_LSM_Station_Info(pst_me_radio_mngr_inst);

											/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
											pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																									
											Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
#if 0
											if(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid != RADIO_MNGR_APP_VALUE_ZERO)
											{
												RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Strategy for Band DAB, Do AF");		

												Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START);										
									
												HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_af_tune_state);
											}
											else
											{
												RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Since SID is not available, so Going to Demute and Listen State");
												pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
												HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
											}
#endif
											RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[Radio][RM]Since SID is not available, so Going to Demute and Listen State");
											pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
											HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
										}
										else
										{
												/*update the LSM structure if the signal is available*/
												Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
												/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
												pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																																	&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																									
												Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
												pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
												HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
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
						
							case RADIO_MNGR_APP_INIT_SCAN:
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Last Tune after Strategy");
							
								/*Update the Current Station with tunable station*/
								Update_CurrentStationInfo_with_TunableStn(pst_me_radio_mngr_inst);

								HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
							}
							break;

							case RADIO_MNGR_APP_SELECT_STATION_END:
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Last Tune after Strategy");
								HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
							}
							break;

							default:
							{
								HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
							}
							break;
						}
					}
					break;

					default:
					{
						Update_CurrentStationInfo_with_TunableStn(pst_me_radio_mngr_inst);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
					}
					break;
				}
			}else{/*FOR MISRA C*/}
		}
		break;

		/*Tune Cancelwith new Request*/
		case RADIO_MNGR_APP_TUNE_CANCEL_REQ:
		{
			if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_IN_STRATEGY)
			{
				Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
			}else{/*FOR MISRA C*/}

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Tune Cancel Requested with MSG: %d", pst_msg->msg_id);
			switch (pst_msg->msg_id)
			{
				case RADIO_MNGR_APP_TUNEUPDOWN_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TuneUpDownDirection), (pst_msg->data), sizeof(Te_RADIO_DirectionType), &(pst_me_radio_mngr_inst->u32_slot));
					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Cancel Request to the AMFM App layer for Tune cancel */
						AMFM_App_Request_Cancel();
					}
					else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
					else{/*FOR MISRA C*/}

				}
				break;
				case RADIO_MNGR_APP_SEEKUPDOWN_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_Seek_NewReq_Direction), (pst_msg->data), sizeof(Te_RADIO_DirectionType), &(pst_me_radio_mngr_inst->u32_slot));

					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Cancel Request to the AMFM App layer for Tune cancel */
						AMFM_App_Request_Cancel();
					}
					else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
					else{/*FOR MISRA C*/}
				}
				break;

				/*Process Update StL after cancel the Ongoing Tune*/
				case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel Tune, App layer will take care based on their current state*/
						//AMFM_App_Request_Cancel();
					}
					else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}else{/*FOR MISRA C*/}
				}
				break;

				/*Request to process & cancel Tune*/
				case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Index),(pst_msg->data),sizeof(Tu8), &((pst_me_radio_mngr_inst->u32_slot)));

					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
			
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList))
					{
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}
				}
				break;
				
				/*Request to process & cancel Tune*/
				case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
	
					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList) ||				 
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList))
					{
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM )
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}
				}
				break;
				
				/*Request to process & cancel Tune*/
				case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_ServiceIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
	
					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_ServiceIndex < 
							pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
				}
				break;

				/*When select band come from HMI, we need cancel the ongoing Tune and switch to selected band*/
				case RADIO_MNGR_APP_SELECTBAND_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TuneCancelRequestedBand), (pst_msg->data), sizeof(Te_Radio_Mngr_App_Band), &(pst_me_radio_mngr_inst->u32_slot));
			
					if(pst_me_radio_mngr_inst->e_TuneCancelRequestedBand != pst_me_radio_mngr_inst->e_activeBand)
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
				
						/*when select band request come for DAB, check the system supports DAB or not.if yes cancel the Tune, otherwise ignore the request*/
						pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue = Radio_Mngr_App_SelectBandConditionCheck(pst_me_radio_mngr_inst->e_TuneCancelRequestedBand, pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
				
						/*Based on the function return value requesting for cancellation of Tune to AMFM/DAB*/
						if(pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_AMFM_CANCEL)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();					
						}
						else if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_DAB_CANCEL)
						{
							/*Cancel Request to the DAB App layer for Tune cancel */
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}else{/*FOR MISRA C*/}
					}else{/*If active and requested band both are same then, req have to ignore*/}
				}
				break;
		
				/*When preset index is valid then give cancel Tune request*/
				case RADIO_MNGR_APP_PRESET_RECALL_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Preset_Recall_Index), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
			
					if(Radio_Mngr_App_PresetRecallConditionCheck(pst_me_radio_mngr_inst))
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				break;

								
				/*When tune by frequency come from HMI, we need cancel the Tune and Process the tune by frequency Request*/
				case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel Tune, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}else{/*FOR MISRA C*/}
				}
				break;
				/*When Power off request come from HMI, we need cancel the Tune and Process the power off Request*/
				case RADIO_MNGR_APP_POWER_OFF_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel Tune, App layer will take care based on their current state*/
						//AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}else{/*FOR MISRA C*/}
				}
				break;
		
				default:
				break;
			}
		}
		break;

		case RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID:
		case RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID:
		{
			/*Extracting the Tune cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RADIO_MNGR_APP_VALUE_ZERO;
			if(((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM) && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS))
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Tune Cancel Done due to New Request");
				
				/*Setting Tune cancel status as one,to post msg again in idle handler*/
				pst_me_radio_mngr_inst->b_Tune_Cancel_Status = RADIO_MNGR_APP_TUNE_CANCELLED;
				/*Transiting to Active Idle Listen, to process Tune cancel with new requests*/
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
		
			}else{/*FOR MISRA C*/}
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_DemuteHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_DemuteHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */

	(pst_me_radio_mngr_inst->u32_slot) = RADIO_MNGR_APP_VALUE_ZERO;
	PRINT_MSG_DATA(pst_msg);

    switch (pst_msg->msg_id)
    {
        case HSM_MSGID_ENTRY:
        {
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_DemuteHndlr");
            SYS_RADIO_MEMCPY((void*) pst_me_radio_mngr_inst->u8p_curr_state_str , "Radio_Mngr_App_Inst_Hsm_ActiveBusy_DemuteHndlr \n" ,sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_DemuteHndlr \n"));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Demute Requested");
			Audio_Manager_Request_DeMute(pst_me_radio_mngr_inst->e_activeBand);
		}
		break;

		case RADIO_MNGR_APP_DEMUTE_DONE_RESID:
		{
			 
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_ReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Demute Done ResID Received");
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, RADIO_MNGR_APP_LISTENING);
				pst_me_radio_mngr_inst->b_Radio_Mute_state = RADIO_MNGR_APP_VALUE_ZERO;
				switch(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id)
				{
					case RADIO_MNGR_APP_PLAY_SELECT_STATION:
					case RADIO_MNGR_APP_STATIONLIST_SELECT:
					case RADIO_MNGR_APP_SELECT_STATION_END:
					case RADIO_MNGR_APP_SERVICE_RECONFIG:
					case RADIO_MNGR_APP_PRESET_RECALL:
					case RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL:
					{	
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END)
						{
							if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
							{
								if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_PRESET_RECALL)
								{
									pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck = RADIO_MNGR_APP_PRESET_FM_SUCCESS;
							
									Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																				pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u_PresetStInfo.st_FMStnInfo.au8_PSN,
																				pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].u8_CharSet, 
																				(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
								}
							
								else if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_STATIONLIST_SELECT)
								{
									pst_me_radio_mngr_inst->u8_Preset_StnlistSelectInfo_NotifyCheck = RADIO_MNGR_APP_STATIONLIST_FM_SUCCESS;
							
									Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																				pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.ast_Stations[pst_me_radio_mngr_inst->u8_Index].au8_PSN,
																				pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_CharSet, 
																				(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
								}
							
								else if(pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH)
								{			
								Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																		pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.ast_Stations[pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex].au8_PSN,
																		pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_CharSet, 
																			(Tu8*)NULL, (Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);
								}else{}
							}
							else
							{
								pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
								/* passing Freq and Station Name to HMI-IF along with response */
								Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);
							}
						}else{/*FOR MISRA C*/}
					}
					break;

					case RADIO_MNGR_APP_INIT_SCAN:
					case RADIO_MNGR_APP_TUNEBYFREQ:
					{
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END)
						{
							/* passing Freq and Station Name to HMI-IF along with response */
							Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);
						}else{/*FOR MISRA C*/}
					}
					break;

					case RADIO_MNGR_APP_SEEK_UPDOWN:
					{	
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
						/* passing updated structure to HMI-IF with response */
						Radio_Mngr_App_Response_SeekUpDown(pst_me_radio_mngr_inst);
					}
					break;

					case RADIO_MNGR_APP_TUNEUPDOWN:
					{
						pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
						/* passing updated structure to HMI-IF with response */
						Radio_Mngr_App_Response_TuneUpDown(pst_me_radio_mngr_inst);
					}
					break;
					
					/* This case is added to play the audio for 2 secs for each scanned ensemble*/
					case RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION:
					{
						/* passing Freq and Station Name to HMI-IF along with response */
						Radio_Mngr_App_Response_PlaySelectSt(pst_me_radio_mngr_inst);
					}
					break;

					default:
					{
						/* do nothing*/
					}
					break;
				}
			}
			else{/*FOR MISRA C*/}
						
			
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->b_FM_StartStatus != COLD_START_DONE_ALREADY)
			{
				pst_me_radio_mngr_inst->b_FM_StartStatus = COLD_START_DONE_ALREADY;
			}
			else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->b_DAB_StartStatus != COLD_START_DONE_ALREADY)
			{
				pst_me_radio_mngr_inst->b_DAB_StartStatus = COLD_START_DONE_ALREADY;
			}
			/*Notifying Radio Listening status Enum to HMI*/
			//pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_LISTENING;
			//Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
			
			
			if (pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION)
			{
				/* transition to busy_stationlist_state hndlr to handle the update station list notification */
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_stationlist_state);
			}
			else
			{	
				/* IF NO NEED TOPLAY THE EACH SCANNED FREQ THEN TRANSIT TO idle_listen_state*/
				/* Startup process completed. Make state transition to idle listen*/
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
			}
		}
		break;
        case HSM_MSGID_EXIT:
        {
			/* do nothing */
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_SeekUpDownHndlr   */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_SeekUpDownHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);
	switch (pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
		{
        
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_SeekUpDownHndlr");
           	SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_SeekUpDownHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_SeekUpDownHndlr \n"));
			
			/* Request to the application layer to seek the station based on Active Band with given direcction*/
			switch (pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_AM:
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AM Band InSeek..");
					
					AMFM_App_Request_SeekUpDown(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_AM_Freq, (Te_RADIO_DirectionType)pst_me_radio_mngr_inst->e_SeekDirection);
				}
				break;

				case RADIO_MNGR_APP_BAND_FM:
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM Band InSeek..");
				
					AMFM_App_Request_SeekUpDown(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_FM_Freq, (Te_RADIO_DirectionType)pst_me_radio_mngr_inst->e_SeekDirection);
				}
				break;
				case RADIO_MNGR_APP_BAND_DAB:
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB Band InSeek..");
					
					DAB_App_Request_ServiceCompSeekUpDown(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_DAB_Freq, (Te_RADIO_DirectionType)pst_me_radio_mngr_inst->e_SeekDirection);
				}
				break;

				default:
				{
					 HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}
				break;
			}
		 }
		 break;
			
		case RADIO_MNGR_APP_AMFM_SEEKUPDOWN_DONE_RESID:
		case RADIO_MNGR_APP_DAB_SEEK_DONE_RESID:
		{
			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SeekReplyStatus),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_SeekReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
			{
				memset((pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel), 0, RADIO_MNGR_APP_COMPONENT_LABEL);
				memset(&(pst_me_radio_mngr_inst->st_DLS_Data), 0, sizeof(Ts_Radio_Mngr_App_DLS_Data));			

				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst->u32_slot));
                /*Calling update component name function to merge service and component name*/
				Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
			}
			else
			{	
				memset((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN), 0, RADIO_MNGR_APP_CHAN_NAME);
				memset((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText), 0, RADIO_MNGR_APP_CHAN_RADIOTEXT);
				
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
			}

			if(pst_me_radio_mngr_inst->e_SeekReplyStatus == REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Seek Success");
				
				Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
				/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
				pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																										&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
				Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);

				if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
				{
					pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet = 3;
				}
				else
				{
					/*FOR MISRA C */
				}
			}
			else
			{
				Update_CurrentStationInfo_with_TunableStn(pst_me_radio_mngr_inst);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]Seek Failure");
			}
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);		
		}
		break;

		/*Seek Cancel with new Requests*/
		case RADIO_MNGR_APP_SEEK_CANCEL_REQ:
		{	
			switch(pst_msg->msg_id)
			{	
				case RADIO_MNGR_APP_TUNEUPDOWN_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
						}
					}else{/*FOR MISRA C*/}
				}
				break;
				case RADIO_MNGR_APP_SEEKUPDOWN_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_Seek_NewReq_Direction), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_Seek_NewReq_Direction), &(pst_me_radio_mngr_inst->u32_slot));

					/*check ongoing seek and requested seek,if both differ then process otherwise ignore the request*/
					if(pst_me_radio_mngr_inst->e_Seek_NewReq_Direction != pst_me_radio_mngr_inst->e_SeekDirection)
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for seek cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}else{/*If active and requested seek direction are same then, req have to ignore*/}
				}
				break;

				/*Process Update StL after cancel the Ongoing Seek*/
				case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
						}
					}else{/*FOR MISRA C*/}
				}
				break;

				/*Request to process & cancel Seek*/
				case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Index),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->u8_Index),&((pst_me_radio_mngr_inst->u32_slot)));

					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList) ||
					 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList) ||
					 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList) )
					{
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for seek cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}
				}
				break;
				
				/*Request to process & cancel Seek*/
				case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
	
					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList) ||				 
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList))
					{
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							/*Cancel Request to the AMFM App layer for seek cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				break;
				
				/*Request to process & cancel Seek*/
				case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_ServiceIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
	
					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_ServiceIndex < 
							pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
						}	
					}else{/*FOR MISRA C*/}
				}
				break;

				/*When select band come from HMI, we need cancel the ongoing Seek and switch to selected band*/
				case RADIO_MNGR_APP_SELECTBAND_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_SeekCancelRequestedBand),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_SeekCancelRequestedBand),&(pst_me_radio_mngr_inst->u32_slot));
					
					if(pst_me_radio_mngr_inst->e_SeekCancelRequestedBand != pst_me_radio_mngr_inst->e_activeBand)
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
						/*when select band request come for DAB, check the system supports DAB or not.if yes cancel the seek, otherwise ignore the request*/
						pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue = Radio_Mngr_App_SelectBandConditionCheck(pst_me_radio_mngr_inst->e_SeekCancelRequestedBand, pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
						
						/*Based on the function return value requesting for cancellation of seek to AMFM/DAB*/
						if(pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_AMFM_CANCEL)
						{
							/*Cancel Request to the AMFM App layer for seek cancel */
							AMFM_App_Request_Cancel();					
						}
						else if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_DAB_CANCEL)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}else{/*If active and requested band both are same then, req have to ignore*/}
				}
				break;
				
				/*When preset index is valid then give cancel Seek request*/
				case RADIO_MNGR_APP_PRESET_RECALL_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Preset_Recall_Index),(pst_msg->data),sizeof(Tu8),&(pst_me_radio_mngr_inst->u32_slot));
					
					if(Radio_Mngr_App_PresetRecallConditionCheck(pst_me_radio_mngr_inst))
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for seek cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
							}
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}
				}
				break;

				/*Process Tune by Frequency request after cancelling the Ongoing Seek*/
				case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
						}
					}else{/*FOR MISRA C*/}
				}
				break;
				
				/*Process Power OFF request after cancelling the Ongoing Seek*/
				case RADIO_MNGR_APP_POWER_OFF_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel both Seek and Scan, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_SEEK_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_SEEK_CANCEL);
						}
					}else{/*FOR MISRA C*/}
				}
				break;
				default:
				break;
			}
		}
		break;

		case RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID:
		case RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID:
		{
			/*Extracting the Seek cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));

			if(((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM) && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS))
			{
				/*Setting seek cancel status as one,to post msg again in idle handler*/
				pst_me_radio_mngr_inst->b_Seek_Cancel_Status = RADIO_MNGR_APP_SEEK_CANCELLED;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Seek Cancelled due to New Request");
				
			}else{/*FOR MISRA C*/}

			/*Transiting to Active Idle Listen, to process seek cancel with new requests*/
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_PresetRecallHndlr   */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_PresetRecallHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handled */
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);
	switch (pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
		{
			
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_PresetRecallHndlr");
           	SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_PresetRecallHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_PresetRecallHndlr \n"));
								
			switch(pst_me_radio_mngr_inst->st_PrestMixedList.ast_presetlist[pst_me_radio_mngr_inst->u8_Preset_Recall_Index].e_Band)
			{
				case RADIO_MNGR_APP_BAND_AM:
				{
					/*If Recall Band and Active band same then directly tune*/
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for AM with Active Band AM");
						pst_me_radio_mngr_inst->u8_Original_Band			    = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
					}
					/*If Recall Band is AM and Active Band is FM, no need to change the band just update internal active band as AM then Tune*/
					else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM &&  
								pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE && 
								pst_me_radio_mngr_inst->b_AM_BandStatus == RADIO_MANAGER_AM_BAND_SUPPORTED)
					{	
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for AM with Active Band FM");
						pst_me_radio_mngr_inst->e_activeBand				    = RADIO_MNGR_APP_BAND_AM;
						pst_me_radio_mngr_inst->u8_Original_Band			    = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
						pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
						Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_SUCCESS, pst_me_radio_mngr_inst->e_activeBand);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
					}
					/*If Recall is AM and Active band is DAB then DeSelect DAB band then Tune*/
					else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB &&
								pst_me_radio_mngr_inst->e_AMActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE && 
								pst_me_radio_mngr_inst->b_AM_BandStatus == RADIO_MANAGER_AM_BAND_SUPPORTED)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for AM with Active Band DAB");
						pst_me_radio_mngr_inst->e_requestedBand		= RADIO_MNGR_APP_BAND_AM;
						pst_me_radio_mngr_inst->u8_Original_Band	= (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
					}
					else
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
				}
				break;
				
				case RADIO_MNGR_APP_BAND_FM:
				{
					/*If Recall Band is FM and Active Band is AM need not to change the band just update internal active band as FM*/
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for FM with Active Band AM");
						pst_me_radio_mngr_inst->e_activeBand				    = RADIO_MNGR_APP_BAND_FM;
						pst_me_radio_mngr_inst->u8_Original_Band			    = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
						pst_me_radio_mngr_inst->st_LSM_Station_Info.u8_LSM_Band = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
						Radio_Mngr_App_Response_SelectBand(REPLYSTATUS_SUCCESS, pst_me_radio_mngr_inst->e_activeBand);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
					}
					/*If Recall Band and Active band same then directly tune*/
					else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for FM with Active Band FM");
						pst_me_radio_mngr_inst->u8_Original_Band			    = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
					}
					/*If Recall is FM and Active band is DAB then Select FM band then Tune*/
					else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for FM with Active Band DAB");
						pst_me_radio_mngr_inst->e_requestedBand		= RADIO_MNGR_APP_BAND_FM;
						pst_me_radio_mngr_inst->u8_Original_Band	= (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
					}
					else
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
				}
				break;
				
				/*Preset Recall for DAB*/
				case RADIO_MNGR_APP_BAND_DAB:
				{
					if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
					{
						/*Recall band and Active band different then select Recall Band*/
						if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM) && 
									(pst_me_radio_mngr_inst->b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED  
									&& pst_me_radio_mngr_inst->e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE))
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for DAB with Active Band AM/FM");
							pst_me_radio_mngr_inst->e_requestedBand		= RADIO_MNGR_APP_BAND_DAB;
							pst_me_radio_mngr_inst->u8_Original_Band	= (Tu8)(pst_me_radio_mngr_inst->e_requestedBand);
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_selectband_state);
						}
						/*If Recall Band and Active band same then directly tune*/
						else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB &&
						           pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Preset Recall for DAB with Active Band DAB");
							pst_me_radio_mngr_inst->u8_Original_Band			    = (Tu8)(pst_me_radio_mngr_inst->e_activeBand);
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_mute_state);
						}
						else
						{
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
						}
					}
					else
					{
						HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
					}
				}
				break;
				
				default:
				{
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
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
            pst_ret = pst_msg;
        }
        break;
	}
	return pst_ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneUpDownHndlr   */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneUpDownHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; 
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);
	switch (pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
		{
        
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneUpDownHndlr");
           	SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneUpDownHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneUpDownHndlr \n"));

			/*TuneUpDown Request has to send to respective app layer base on active band*/
			switch(pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_AM:
				case RADIO_MNGR_APP_BAND_FM:
				{
					AMFM_App_Request_TuneUpDown((Te_RADIO_DirectionType)pst_me_radio_mngr_inst->e_TuneUpDownDirection, (Tu8)RADIO_APP_VALUE_NONZERO);
				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{
					DAB_App_Request_TuneUpDown((Te_RADIO_DirectionType)pst_me_radio_mngr_inst->e_TuneUpDownDirection);
				}
				break;

				default :
				{
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}
				break;
			}
		}
		break;

		case RADIO_MNGR_APP_AMFM_TUNEUPDOWN_DONE_RESID:
		case RADIO_MNGR_APP_DAB_TUNEUPDOWN_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TuneUpDownReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));				
			
			switch(pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_AM:
				case RADIO_MNGR_APP_BAND_FM:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_AMFM_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_DAB_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
				}
				break;

				default :
				{
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}
				break;
			}

			Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
			/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
			pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE,
				&(pst_me_radio_mngr_inst->st_LSM_Station_Info));

			Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
			
		}
		break;
		/*Tune Cancelwith new Request*/
		case RADIO_MNGR_APP_TUNE_CANCEL_REQ:
		{			
			if (pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id == RADIO_MNGR_APP_IN_STRATEGY)
			{
				Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/ }

			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[Radio][RM]Tune Cancel Requested with MSG: %d", pst_msg->msg_id);
			switch (pst_msg->msg_id)
			{
				case RADIO_MNGR_APP_TUNEUPDOWN_REQID:
				{
					Te_RADIO_DirectionType e_TuneUpDownNewDirection;
					ExtractParameterFromMsg(&(e_TuneUpDownNewDirection), (pst_msg->data), sizeof(Te_RADIO_DirectionType), &(pst_me_radio_mngr_inst->u32_slot));
					/*check ongoing tune up down and requested tune up down,if both differ then process otherwise ignore the request*/
					if (e_TuneUpDownNewDirection != pst_me_radio_mngr_inst->e_SeekDirection)
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/ }
					}
					else
					{
						/*If active and requested direction are same then, req have to ignore*/					
					}

				}
				break;
				case RADIO_MNGR_APP_SEEKUPDOWN_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_Seek_NewReq_Direction), (pst_msg->data), sizeof(Te_RADIO_DirectionType), &(pst_me_radio_mngr_inst->u32_slot));

					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Cancel Request to the AMFM App layer for Tune cancel */
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
					else{/*FOR MISRA C*/ }
				}
				break;
		
				/*Process Update StL after cancel the Ongoing Tune*/
				case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel Tune, App layer will take care based on their current state*/
						//AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
					else{/*FOR MISRA C*/ }
				}
				break;

				/*Request to process & cancel Tune*/
				case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Index), (pst_msg->data), sizeof(Tu8), &((pst_me_radio_mngr_inst->u32_slot)));

					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if ((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList))
					{
						if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/ }
					}
				}
				break;
					/*Request to process & cancel Tune*/
				case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));

					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if ((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList))
					{
						if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/ }
					}
				}
				break;

					/*Request to process & cancel Tune*/
				case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_ServiceIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));

					/*Copying msg to local buffer, it is used when we transit to idle handler*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_ServiceIndex <
						pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble)
					{
						if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
				}
				break;

					/*When select band come from HMI, we need cancel the ongoing Tune and switch to selected band*/
				case RADIO_MNGR_APP_SELECTBAND_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TuneCancelRequestedBand), (pst_msg->data), sizeof(Te_Radio_Mngr_App_Band), &(pst_me_radio_mngr_inst->u32_slot));

					if (pst_me_radio_mngr_inst->e_TuneCancelRequestedBand != pst_me_radio_mngr_inst->e_activeBand)
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

						/*when select band request come for DAB, check the system supports DAB or not.if yes cancel the Tune, otherwise ignore the request*/
						pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue = Radio_Mngr_App_SelectBandConditionCheck(pst_me_radio_mngr_inst->e_TuneCancelRequestedBand, pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);

						/*Based on the function return value requesting for cancellation of Tune to AMFM/DAB*/
						if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_AMFM_CANCEL)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_DAB_CANCEL)
						{
							/*Cancel Request to the DAB App layer for Tune cancel */
							if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/ }
					}
					else{/*If active and requested band both are same then, req have to ignore*/ }
				}
				break;

					/*When preset index is valid then give cancel Tune request*/
				case RADIO_MNGR_APP_PRESET_RECALL_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Preset_Recall_Index), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));

					if (Radio_Mngr_App_PresetRecallConditionCheck(pst_me_radio_mngr_inst))
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

						if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							/*Cancel Request to the AMFM App layer for Tune cancel */
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
							}
							else
							{
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/ }
					}
					else{/*FOR MISRA C*/ }
				}
				break;


					/*When tune by frequency come from HMI, we need cancel the Tune and Process the tune by frequency Request*/
				case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel Tune, App layer will take care based on their current state*/
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
					else{/*FOR MISRA C*/ }
				}
				break;
					/*When Power off request come from HMI, we need cancel the Tune and Process the power off Request*/
				case RADIO_MNGR_APP_POWER_OFF_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						/*Request function to cancel Tune, App layer will take care based on their current state*/
						//AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if (pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED;
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType)RADIO_MNGR_APP_TUNE_CANCEL);
						}
						else
						{
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_TUNE_CANCEL);
						}
					}
					else{/*FOR MISRA C*/ }
				}
				break;

				default:
				break;
			}
		}
		break;

		case RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID:
		case RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID:
		{
			/*Extracting the Tune cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			pst_me_radio_mngr_inst->b_TuneCancelReqStatus = RADIO_MNGR_APP_VALUE_ZERO;
			if (((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM) && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS) ||
				(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS))
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[Radio][RM]Tune Cancel Done due to New Request");

				/*Setting Tune cancel status as one,to post msg again in idle handler*/
				pst_me_radio_mngr_inst->b_Tune_Cancel_Status = RADIO_MNGR_APP_TUNE_CANCELLED;
				/*Transiting to Active Idle Listen, to process Tune cancel with new requests*/
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);

			}
			else{/*FOR MISRA C*/ }
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_AF_TuneHndlr   */
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_AF_TuneHndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL; 
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);
	switch (pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
		{
        
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_AF_TuneHndlr");
           	SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_AF_TuneHndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_AF_TuneHndlr \n"));
								
			if(pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id != RADIO_MNGR_APP_IN_STRATEGY)
			{					
	            /*Requesting for AF Tune based on the active band*/
				switch(pst_me_radio_mngr_inst->e_activeBand)
				{
					case RADIO_MNGR_APP_BAND_FM:
					{	
						if(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI != RADIO_MNGR_APP_VALUE_ZERO && pst_me_radio_mngr_inst->e_RDSSettings == RADIO_MNGR_APP_RDS_SETTINGS_ENABLE)
						{
							/*Strategy flow status as Active Band FM,  FM_AF Tune Requested*/
							pst_me_radio_mngr_inst->e_StrategyFlow = RM_STRATEGY_FLOW_FM_FM_AF;
							AMFM_App_Request_AFTune(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI);
							
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM, FM AF Tune Requested with PI: %d", pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI);
							pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_FM_AF_PROCESSING;
							Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
						}
						else
						{
							Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END);
							Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
						}
					}
					break;

					case RADIO_MNGR_APP_BAND_DAB:
					{
						if(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid != RADIO_MNGR_APP_VALUE_ZERO)
						{
							/*Strategy flow status as Active Band DAB,  DAB_AF Tune Requested*/
							pst_me_radio_mngr_inst->e_StrategyFlow = RM_STRATEGY_FLOW_DAB_DAB_AF;
							DAB_App_Request_AFTune((Tu16)pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid, pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_SCIdI);
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB, DAB AF Tune Requested with SID: %d", pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid);
							pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DAB_AF_PROCESSING;
							Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
						}
						else
						{
							Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END);
							Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
							HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
						}
					}
					break;

					default :
					{
					}
					break;
				}
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_AMFM_AF_TUNE_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_FM_AFTuneReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM AF Tune Response Status: %d", pst_me_radio_mngr_inst->e_FM_AFTuneReplyStatus);

			/*FM, FM AF Success*/
			if(pst_me_radio_mngr_inst->e_FM_AFTuneReplyStatus == REPLYSTATUS_SUCCESS)
			{
				/*FM, FM AF Success*/
				if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_FM_FM_AF)
				{
					/*Updating Strategy end in RM, FM and DAB*/
					Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END);

					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_AMFM_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
					Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
					/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
					pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
					Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
									
					Radio_Manager_App_Update_PresetMixedList_AFTune(pst_me_radio_mngr_inst);

					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}
				/*DAB, FM AF Success*/
				else if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_DAB_FM_AF)
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_AMFM_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
					
					Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);

					Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_DAB, pst_me_radio_mngr_inst);

					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}else{/*FOR MISRA C*/}
			}
			/*FM, FM AF Fail*/
			else if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_FM_FM_AF)
			{
				Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
				
				if(pst_me_radio_mngr_inst->e_DABFMLinking_Switch == RADIO_MNGR_APP_DABFMLINKING_ENABLE)
				{
					pst_me_radio_mngr_inst->e_StrategyFlow = RM_STRATEGY_FLOW_FM_DAB_AF;
					DAB_App_Request_AFTune(pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI, (Tu16)RADIO_MNGR_APP_VALUE_ZERO);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FM, FM AF Tune Fail so FM, DAB AF Tune Requested with PI: %d", pst_me_radio_mngr_inst->st_Tunable_Station_Info.u16_PI);
					
					if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_INVALID && 
									pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus  != REPLYSTATUS_INVALID_PARAM)
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
					else
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_FM_INTERNAL_SCAN_PROCESS;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
				}
				else
				{
					if(pst_me_radio_mngr_inst->e_GetFMstationlistreplystatus  != REPLYSTATUS_INVALID_PARAM)
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_FM_INTERNAL_SCAN_PROCESS;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
					else
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGE_APP_FM_LEARNMEM_AF_PROCESSING;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
					/*IF D<=>F settings are off simply we have to wait for FM AF Learn response*/
				}
			}
			/*DAB, FM AF Fail*/
			else if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_DAB_FM_AF)
			{
				/*DAB AF Learn fail*/
				if(pst_me_radio_mngr_inst->e_LearnAFStatus != RADIO_MNGR_APP_LEARN_MEM_AF_INVALID)
				{
					Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
					/*Tune to Original Station*/
					Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
				}else{/*If DAB FM AF fail, then wait for DAB DAB AF learn status*/}
			}else{/*FOR MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_DAB_AF_TUNE_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_DAB_AFTuneReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB AF Tune Response Status: %d", pst_me_radio_mngr_inst->e_DAB_AFTuneReplyStatus);
			/*DAB, DAB AF Success*/
			if(pst_me_radio_mngr_inst->e_DAB_AFTuneReplyStatus == REPLYSTATUS_SUCCESS)
			{
				/*DAB, DAB AF Success*/
				if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_DAB_DAB_AF)
				{
					/*Updating Strategy end in RM, FM and DAB*/
					Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END);

					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_DAB_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));

					Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
				
					/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
					pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																											&(pst_me_radio_mngr_inst->st_LSM_Station_Info));
																										
					Radio_Manager_EEPROM_Log(pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus);
									
					Radio_Manager_App_Update_PresetMixedList_AFTune(pst_me_radio_mngr_inst);
					
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}
				/*FM, DAB AF Success*/
				else if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_FM_DAB_AF)
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), (pst_msg->data), sizeof(Ts_Radio_Mngr_App_DAB_CurrentStationInfo), &(pst_me_radio_mngr_inst->u32_slot));
					Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
					
					Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);

					Radio_Mngr_App_Response_BGStationInfo(RADIO_MNGR_APP_BAND_FM, pst_me_radio_mngr_inst);

					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
				}
			}
			/*DAB, DAB AF Fail*/
			else if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_DAB_DAB_AF)
			{
				if(pst_me_radio_mngr_inst->e_DABFMLinking_Switch == RADIO_MNGR_APP_DABFMLINKING_ENABLE)
				{
					pst_me_radio_mngr_inst->e_StrategyFlow = RM_STRATEGY_FLOW_DAB_FM_AF;
					AMFM_App_Request_AFTune((Tu16)pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB, DAB AF Tune Fail so DAB, FM AF Tune Requested with SID: %d", pst_me_radio_mngr_inst->st_Tunable_Station_Info.u32_Sid);
				
					if(pst_me_radio_mngr_inst->e_LearnAFStatus == RADIO_MNGR_APP_LEARN_MEM_AF_INVALID && 
										pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus  != REPLYSTATUS_INVALID_PARAM)
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
					else
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DAB_INTERNAL_SCAN_PROCESS;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
				}
				else
				{
					if(pst_me_radio_mngr_inst->e_GetDABstationlistreplystatus  != REPLYSTATUS_INVALID_PARAM)
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DAB_INTERNAL_SCAN_PROCESS;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
					else
					{
						pst_me_radio_mngr_inst->e_Activity_Status = RADIO_MNGR_APP_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING;
						Radio_Mngr_App_Notify_Activity_State(pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst->e_Activity_Status);
					}
					/*IF D<=>F settings are off simply we have to wait for DAB AF Learn response*/
				}
			}
			/*FM, DAB AF Fail*/
			else if(pst_me_radio_mngr_inst->e_StrategyFlow == RM_STRATEGY_FLOW_FM_DAB_AF)
			{
				/*FM AF Learn fail*/
				if(pst_me_radio_mngr_inst->e_LearnAFStatus != RADIO_MNGR_APP_LEARN_MEM_AF_INVALID)
				{
					Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
					/*Tune to Original Station*/
					Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_FM);
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
				}
				else{/*If FM DAB AF fail, then wait for FM FM AF learn status*/}
			}else{/*FOR MISRA C*/}
			
		}
		break;

		case RADIO_MNGR_APP_AF_TUNE_CANCEL_REQ:
		{	
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AF Tune Cancel Requested with MSG: %d", pst_msg->msg_id);
			switch(pst_msg->msg_id)
			{
				case RADIO_MNGR_APP_TUNEUPDOWN_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
							Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
						}else{/*FOR MISRA C*/}
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
						else
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
					}else{/*FOR MISRA C*/}					
				}
				break;			
				case RADIO_MNGR_APP_SEEKUPDOWN_REQID:
				{
					/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
							Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
						}else{/*FOR MISRA C*/}
						
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
						else
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
					}
					else{/*FOR MISRA C*/}
				}
				break;

				case RADIO_MNGR_APP_PLAY_SELECT_STATION_REQID:
				{
					/* Updating the requested station index into the inst hsm structure*/
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Index),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->u8_Index),&(pst_me_radio_mngr_inst->u32_slot));
					/*Comparing the Index selected from HMI & Number of stations present in stationlist*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_AM_StationList.u8_numberStationsInList) ||
						 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_RadioStationList.st_FM_StationList.u8_numberStationsInList) ||
						 (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Index <= pst_me_radio_mngr_inst->st_NormalStnView.u8_numberStationsInList))
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}
					else{/*FOR MISRA C*/}
				}
				break;	
				
				case RADIO_MNGR_APP_PLAY_SELECT_SEARCH_STATION_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_NON_RDS_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMNonRDS_StnListSearch.u8_numberStationsInList) ||
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM && pst_me_radio_mngr_inst->st_StationList_Search.e_STL_Search_Type == RADIO_MNGR_APP_FM_STL_SEARCH && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_FMRDS_StnListSearch.u8_numberStationsInList) ||				 
						(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_Req_PlaySearchIndex <= pst_me_radio_mngr_inst->st_StationList_Search.u_StationList_Search.st_DAB_StnListSearch.u8_numberStationsInList))
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}
					else{/*FOR MISRA C*/}
				}
				break;	
				
				case RADIO_MNGR_APP_PLAYSELECT_SERVICEIN_MULTIPLEXLIST_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_ServiceIndex), (pst_msg->data), sizeof(Tu8), &(pst_me_radio_mngr_inst->u32_slot));
					
					/*Checking the playselect index is valid or not,if valid then process,otherwise ignore the request*/
					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->u8_ServiceIndex < 
							pst_me_radio_mngr_inst->st_MultiplexStlView.ast_EnsembleInfo[pst_me_radio_mngr_inst->u8_ReqEnsembleIndex].u8_NoOfserviceInEnsemble)
					{
						/*Copying msg to local buffer, it will used while after transiting to Active idle state*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
						else
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}	
					}
					else{/*FOR MISRA C*/}
				}
				break;	

				case RADIO_MNGR_APP_SELECTBAND_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_requestedBand),(pst_msg->data),sizeof(pst_me_radio_mngr_inst->e_requestedBand),&(pst_me_radio_mngr_inst->u32_slot));
			
					if(pst_me_radio_mngr_inst->e_requestedBand != pst_me_radio_mngr_inst->e_activeBand && pst_me_radio_mngr_inst->e_requestedBand != RADIO_MNGR_APP_RADIO_MODE)
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						
						/*when select band request come for DAB, check the system supports DAB or not.if yes cancel the seek, otherwise ignore the request*/
						pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue = Radio_Mngr_App_SelectBandConditionCheck(pst_me_radio_mngr_inst->e_requestedBand, pst_me_radio_mngr_inst->e_activeBand, pst_me_radio_mngr_inst);
						
						/*Checking the function return type if One then transiting to idle state*/
						if(pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_AMFM_CANCEL)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							AMFM_App_Request_Cancel();				
						}
						else if (pst_me_radio_mngr_inst->u8_SelectBandCheckReturnValue == RADIO_MNGR_APP_DAB_CANCEL)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
						}
						else{/*FOR MISRA C*/}
					}else{/*If active and requested band both are same then, req have to ignore*/}
				
				}
				break;

				case RADIO_MNGR_APP_PRESET_RECALL_REQID:
				{
					ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->u8_Preset_Recall_Index),(pst_msg->data),sizeof(Tu8),&(pst_me_radio_mngr_inst->u32_slot));
					
					if(Radio_Mngr_App_PresetRecallConditionCheck(pst_me_radio_mngr_inst))
					{
						/*Copying msg to local buffer, it is used when we transit to idle handler*/
						SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));
						if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							AMFM_App_Request_Cancel();
						}
						else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
						{
							if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								/*Cancel Request to the DAB App layer for the Current Execution */
								DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
							else
							{
								if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
								{
									RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
									Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
								}else{/*FOR MISRA C*/}
								DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
							}
						}else{/*FOR MISRA C*/}
					}else{/*FOR MISRA C*/}				
				}
				break;
			
				case RADIO_MNGR_APP_UPDATE_STATION_LIST_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
							Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
						}else{/*FOR MISRA C*/}
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
						else
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
					}else{/*FOR MISRA C*/}					
				}
				break;

				/*When tune by frequency come from HMI, we need cancel the AF Tune and Process the tune by frequency request*/
				case RADIO_MNGR_APP_TUNE_BY_FREQUENCY_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
							Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
						}else{/*FOR MISRA C*/}
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
						else
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
					}else{/*FOR MISRA C*/}					
				}
				break;	
				
				/*When Power OFF come from HMI, we need cancel the AF Tune and Process the Power OFF request*/
				case RADIO_MNGR_APP_POWER_OFF_REQID:
				{
					/*Copying msg to local buffer, it will used while after transisting to Active idle state*/
					SYS_RADIO_MEMCPY(&(pst_me_radio_mngr_inst->st_msg_cpy), pst_msg, sizeof(Ts_Sys_Msg));

					if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM)
					{
						if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
							Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
						}else{/*FOR MISRA C*/}
						AMFM_App_Request_Cancel();
					}
					else if (pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB)
					{
						if(pst_me_radio_mngr_inst->e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							/*Cancel Request to the DAB App layer for the Current Execution */
							DAB_App_Request_Cancel((Te_DAB_App_CancelType) RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
						else
						{
							if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START)
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Stop StationNotAvail Strategy in AF Handler");
								Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
							}else{/*FOR MISRA C*/}
							DAB_Request_Internal_Cancel_DABTuner_Abnormal(pst_me_radio_mngr_inst, RADIO_MNGR_APP_AF_TUNE_CANCEL);
						}
					}else{/*FOR MISRA C*/}					
				}
				break;		
		
				default:
				break;
			}
		}
		break;

		case RADIO_MNGR_APP_AMFM_CANCEL_DONE_RESID:
		case RADIO_MNGR_APP_DAB_CANCEL_DONE_RESID:
		{
			/*Extracting the AF Tune cancel Reply status variable*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_ReplyStatus), (pst_msg->data), sizeof(pst_me_radio_mngr_inst->e_ReplyStatus), &(pst_me_radio_mngr_inst->u32_slot));

			if(((pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM) && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS) ||
				(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB && pst_me_radio_mngr_inst->e_ReplyStatus == REPLYSTATUS_SUCCESS))
			{
				if(pst_me_radio_mngr_inst->e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START && 
						((pst_me_radio_mngr_inst->e_RDSSettings_Request == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE && 
							pst_me_radio_mngr_inst->e_RDSSettings == RADIO_MNGR_APP_RDS_SETTINGS_DISABLE) ||
						 (pst_me_radio_mngr_inst->e_DABFMLinking_Switch_Request == RADIO_MNGR_APP_DABFMLINKING_DISABLE && 
							pst_me_radio_mngr_inst->e_DABFMLinking_Switch == RADIO_MNGR_APP_DABFMLINKING_DISABLE)))
				{
					Radio_Mngr_App_Stop_StationNotAvail_Strategy(pst_me_radio_mngr_inst);
					/*Tune to Original Station*/
					Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);					
				}
				else
				{
					/*Setting the AF Tune cancel Flag, so as we can use the same in Active idle handler*/
					pst_me_radio_mngr_inst->b_AF_Tune_Cancel_Status = RADIO_MNGR_APP_AF_TUNE_CANCELLED;
					
					/*Transiting to Active Idle so as to handle HMI Requested request after AF Tune cancelled*/
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
				}
			}else{/*FOR MISRA C*/}	
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneByFrequency_Hndlr*/
/*===========================================================================*/
Ts_Sys_Msg* Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneByFrequency_Hndlr(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* pst_ret = NULL; 
	pst_me_radio_mngr_inst->u32_slot = RADIO_MNGR_APP_VALUE_ZERO;

	PRINT_MSG_DATA(pst_msg);

	switch (pst_msg->msg_id)
	{
        case HSM_MSGID_ENTRY:
		{
        
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneByFrequency_Hndlr");
			SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr_inst->u8p_curr_state_str ,"Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneByFrequency_Hndlr \n" ,
								sizeof("Radio_Mngr_App_Inst_Hsm_ActiveBusy_TuneByFrequency_Hndlr \n"));

			switch (pst_me_radio_mngr_inst->e_activeBand)
			{
				case RADIO_MNGR_APP_BAND_AM:
				case RADIO_MNGR_APP_BAND_FM:
				{
					AMFM_App_Request_PlaySelectSt(pst_me_radio_mngr_inst->u32_ReqFreq, (Te_AMFM_App_mode)pst_me_radio_mngr_inst->e_activeBand);
				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{
					DAB_App_Request_TuneByChannelName(pst_me_radio_mngr_inst->au8_DABChannelName);
				}
				break;
				 
				default:
				{
					HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_listen_state);
				}
				break;
			}
		}
		break;

		/*Response from the APP layers for the Tune Request*/
		case RADIO_MNGR_APP_DAB_PLAY_SELECT_STATION_DONE_RESID:
		case RADIO_MNGR_APP_AMFM_PLAY_SELECT_STATION_DONE_RESID:
		{			
			ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->e_TuneByFreqReplyStatus), (pst_msg->data), sizeof(Te_RADIO_ReplyStatus),&(pst_me_radio_mngr_inst->u32_slot));

			if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_AM || pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM)
			{
				memset((pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText), 0, RADIO_MNGR_APP_CHAN_RADIOTEXT);
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_AMFM_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
			}
			else
			{	
				memset(&(pst_me_radio_mngr_inst->st_DLS_Data), 0, sizeof(Ts_Radio_Mngr_App_DLS_Data));
				ExtractParameterFromMsg(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),(pst_msg->data), sizeof(pst_me_radio_mngr_inst->st_DAB_currentstationinfo),&(pst_me_radio_mngr_inst->u32_slot));
				Update_ComponentName(&(pst_me_radio_mngr_inst->st_DAB_currentstationinfo), &(pst_me_radio_mngr_inst-> st_CurrentStationName));
			}

			/*update the LSM structure if the signal is available*/
			Update_LSM_TunableStn_with_CurrentStationInfo(pst_me_radio_mngr_inst);
					
			/*For Every LSM Structure Update, Need to update to System by this below API, so that system can Write in Shutdown case*/
			pst_me_radio_mngr_inst->u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																								&(pst_me_radio_mngr_inst->st_LSM_Station_Info));

			/*send UpdateCurStationInfo to HMI if active band is DAB,no need to send if active band is FM ,because in AMFM tuner status notification we will get currentstation info*/
			if(pst_me_radio_mngr_inst->e_EngMode_Switch == RADIO_MNGR_APP_ENG_MODE_ON && pst_me_radio_mngr_inst->e_activeBand ==RADIO_MNGR_APP_BAND_DAB)
			{
				Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(pst_me_radio_mngr_inst);
			}
			else{/*FOR MISRA C*/}

            /*If select station response is success then transiting to demute state*/
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_demute_state);
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
/*  void Radio_Mngr_App_Inst_Hsm_HandleMsg                                     */
/*===========================================================================*/
void Radio_Mngr_App_Inst_Hsm_HandleMsg(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst, Ts_Sys_Msg* pst_msg)
{  
	switch (pst_msg->msg_id)
	{
			case RADIO_MNGR_APP_INST_RESID_DONE_CASES:
       		case RADIO_MNGR_APP_INST_REQID_CASES:
			case RADIO_MNGR_APP_INST_NOTIFYID_CASES:
			{
				HSM_ON_MSG(pst_me_radio_mngr_inst,pst_msg);
			}
			break;
        
			default:
			{
				/* do nothing */	
			}
			break;
	}
}

/*===========================================================================*/
/*  void Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus                                     */
/*===========================================================================*/
void Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst, Te_Radio_Mngr_App_StationNotAvail_StrategyStatus e_StrategyStatus)
{
	pst_me_radio_mngr_inst->e_StrategyStatus = e_StrategyStatus;
	
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Strategy Status is now: %d", pst_me_radio_mngr_inst->e_StrategyStatus);
	AMFM_App_Notify_StationNotAvail_StrategyStatus((Te_AMFM_App_StationNotAvailStrategyStatus)e_StrategyStatus);
	DAB_App_Notify_StationNotAvail_StrategyStatus((Te_DAB_App_StationNotAvailStrategyStatus)e_StrategyStatus);
	
	if(e_StrategyStatus == RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END)
	{
		pst_me_radio_mngr_inst->e_Radio_Mngr_App_Req_Id = RADIO_MNGR_APP_SELECT_STATION_END;
		pst_me_radio_mngr_inst->e_StrategyFlow          = RM_STRATEGY_FLOW_INVALID;

		/*clear the check parametres*/
		Radio_Mngr_App_ClearCheckParameters(pst_me_radio_mngr_inst);
	}
}

/*===========================================================================*/
/*  void Radio_Mngr_App_Stop_StationNotAvail_Strategy                                     */
/*===========================================================================*/
void Radio_Mngr_App_Stop_StationNotAvail_Strategy(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst)
{
	if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_FM &&
			pst_me_radio_mngr_inst->e_activeBand != pst_me_radio_mngr_inst->e_Curr_Audio_Band)
	{
		Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_FM, 
																pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.u32_frequency, 
																pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.un_station.st_FMCurrentStationInfo.au8_PSN,
																pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.u8_CharSet, pst_me_radio_mngr_inst->st_AMFM_currentstationinfo.au8_RadioText,
																(Tu8*)NULL, (Tu8*)NULL, pst_me_radio_mngr_inst);

		
	}
	else if(pst_me_radio_mngr_inst->e_activeBand == RADIO_MNGR_APP_BAND_DAB &&
				pst_me_radio_mngr_inst->e_activeBand != pst_me_radio_mngr_inst->e_Curr_Audio_Band)
	{
		Radio_Mngr_App_Response_UpdateCurStationInfo_Display(RADIO_MNGR_APP_BAND_DAB,
																				pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_Tunableinfo.u32_Frequency,
																				pst_me_radio_mngr_inst->st_CurrentStationName.au8_CompLabel,
																				pst_me_radio_mngr_inst->st_CurrentStationName.u8_CharSet,
																				pst_me_radio_mngr_inst->st_DLS_Data.au8_DLSData,
																				pst_me_radio_mngr_inst->st_DAB_currentstationinfo.au8_ChannelName,
																				pst_me_radio_mngr_inst->st_DAB_currentstationinfo.st_EnsembleLabel.au8_Label, pst_me_radio_mngr_inst);
	}else{/*FOR MISRA C*/}
	
	Radio_Mngr_App_Notify_StationNotAvail_StrategyStatus(pst_me_radio_mngr_inst, RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END);
	
	if(pst_me_radio_mngr_inst->b_Check_AudioSwitchNeed != RADIO_MNGR_APP_AUDIO_SWITCH_NOT_NEEDED)
	{
		Radio_Mngr_App_AudioChange(pst_me_radio_mngr_inst, pst_me_radio_mngr_inst->e_activeBand);
	}
	else
	{
		pst_me_radio_mngr_inst->b_Check_AudioSwitchNeed = RADIO_MNGR_APP_AUDIO_SWITCH_NEEDED;
	}
}

/*===========================================================================*/
/*  void DAB_Request_Internal_Cancel_DABTuner_Abnormal                                 */
/*===========================================================================*/
void DAB_Request_Internal_Cancel_DABTuner_Abnormal(Ts_Radio_Mngr_App_Inst_Hsm *pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Cancel_Req_Type e_Cancel_Type)
{
	switch(e_Cancel_Type)
	{
		case RADIO_MNGR_APP_SEEK_CANCEL:
		{
			/*Setting seek cancel status as one,to post msg again in idle handler*/
			pst_me_radio_mngr_inst->b_Seek_Cancel_Status = RADIO_MNGR_APP_SEEK_CANCELLED;
			
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
		}
		break;
		
		case RADIO_MNGR_APP_TUNE_CANCEL:
		{
			/*Setting Tune cancel status as one,to post msg again in idle handler*/
			pst_me_radio_mngr_inst->b_Tune_Cancel_Status = RADIO_MNGR_APP_TUNE_CANCELLED;
			
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
		}
		break;
		
		case RADIO_MNGR_APP_SCAN_CANCEL:
		{
			/*Scan Cancel done successfully and Scan cancel Request type is Not From HMI button then transiting to idle state to handle new request*/
			if(pst_me_radio_mngr_inst->e_ScanCancel_Request_Type != RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI)
			{	
				/*Setting the Scan cancel Flag, so as we can use the same in Active idle handler*/
				pst_me_radio_mngr_inst->b_Scan_Cancel_Status = RADIO_MNGR_APP_SCAN_CANCELLED;

				/*clear the check parametres*/
				Radio_Mngr_App_ClearCheckParameters(pst_me_radio_mngr_inst);

				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);	
			}
			else if(pst_me_radio_mngr_inst->e_ScanCancel_Request_Type == RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI)
			{
				pst_me_radio_mngr_inst->e_ScanCancel_Request_Type = RADIO_MNGR_APP_SCAN_CANCEL_INVALID;

				/*If Manual update is cancelled through HMI Button then after cancelling tune to original station*/
				Radio_Mngr_Update_OriginalStn(pst_me_radio_mngr_inst, RADIO_MNGR_APP_BAND_DAB);
				HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_busy_play_selectstation_state);
			}else{/*FOR MISRA C*/}
		}
		break;
		
		case RADIO_MNGR_APP_AF_TUNE_CANCEL:
		{
			pst_me_radio_mngr_inst->b_AF_Tune_Cancel_Status = RADIO_MNGR_APP_AF_TUNE_CANCELLED;
			HSM_STATE_TRANSITION(pst_me_radio_mngr_inst, &radio_mngr_app_inst_hsm_active_idle_state);
		}
		break;
		
		case RADIO_MNGR_APP_ANNO_CANCEL:
		{
			if(pst_me_radio_mngr_inst->e_AnnoCancel_Request_Type != RADIO_MNGR_APP_ANNO_CANCEL_BY_HMI)
			{
				/*Setting the HMI Req flag as One so it should go with respective request to parent handler*/
				pst_me_radio_mngr_inst->b_Internal_Msg_Flag = RADIO_MNGR_APP_UINT8_ONE;
				/*Posting of msg so it can be handled by respective handler*/
				Radio_Mngr_App_Internal_HMI_Request_Message(&(pst_me_radio_mngr_inst->st_msg_cpy));
			}
			else
			{
				pst_me_radio_mngr_inst->e_Anno_Status = RADIO_MNGR_APP_ANNO_END;
			}
		}
		break;
		
		default:
		{
		
		}
		break;
	}
}

