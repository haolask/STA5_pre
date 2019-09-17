/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_hsm.c																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager Application															*
*  Description			: This source file consists of function definitions of all function handlers of HSM	*
*						  radio_mngr_app_hsm(main) of Radio Manager Application component					*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
							radio_mngr_app_hsm.c
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include <string.h>
#include "amfm_app_request.h"
#include "dab_app_request.h"
#include "radio_mngr_app_hsm.h"
#include "radio_mngr_app_response.h"
#include "sys_nvm.h"
#include "radio_mngr_app_request.h"
#include "radio_mngr_app_notify.h"


/*---------------------------------------------------------------------------*/



/* HSM state hierarchy */
HSM_CREATE_STATE(radio_mngr_app_hsm_top_state,								NULL,                                                      Radio_Mngr_App_Hsm_TopHndlr,                                    "radio_mngr_app_hsm_top_state");
	HSM_CREATE_STATE(radio_mngr_app_hsm_inactive_state,						&radio_mngr_app_hsm_top_state,                             Radio_Mngr_App_Hsm_InactiveHndlr,                               "radio_mngr_app_hsm_inactive_state");
	HSM_CREATE_STATE(radio_mngr_app_hsm_active_state,						&radio_mngr_app_hsm_top_state,                             Radio_Mngr_App_Hsm_ActiveHndlr,                                 "radio_mngr_app_hsm_active_state");
		HSM_CREATE_STATE(radio_mngr_app_hsm_active_start_state,             &radio_mngr_app_hsm_active_state,                          Radio_Mngr_App_Hsm_ActiveStartHndlr,                            "radio_mngr_app_hsm_active_start_state");
		HSM_CREATE_STATE(radio_mngr_app_hsm_active_idle_state,              &radio_mngr_app_hsm_active_state,                          Radio_Mngr_App_Hsm_ActiveIdleHndlr,                             "radio_mngr_app_hsm_active_idle_state");
		HSM_CREATE_STATE(radio_mngr_app_hsm_active_stop_state,              &radio_mngr_app_hsm_active_state,                          Radio_Mngr_App_Hsm_ActiveStopHndlr,                             "radio_mngr_app_hsm_active_stop_state");

/*-----------------------------------------------------------------------------
    private function definitions
------------------------------------------------------------------------------*/
/*===========================================================================*/
/*  void Radio_Mngr_App_Hsm_Init                                                    */
/*===========================================================================*/
void Radio_Mngr_App_Hsm_Init(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr)
{
    if(pst_me_radio_mngr != NULL)
    {
        /* clear the hsm */
        memset(pst_me_radio_mngr, RADIO_MNGR_APP_UINT8_ZERO, sizeof(Ts_Radio_Mngr_App_Hsm));
		pst_me_radio_mngr->u8p_curr_state_str = pst_me_radio_mngr->str_state;
        /* Call the base class Ctor */
        HSM_CTOR((Ts_hsm*)pst_me_radio_mngr, &radio_mngr_app_hsm_top_state, RADIO_MNGR_APP);

        /* start HSM */
        HSM_ON_START(pst_me_radio_mngr);
	
		Radio_Mngr_App_Inst_Hsm_Init(&pst_me_radio_mngr->st_inst_hsm);				// Calling initialization function of instance hsm
	}
    else
    {
        /* do nothing*/
    }
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  Radio_Mngr_App_Hsm_TopHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_TopHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handeled */

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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Hsm_TopHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr->u8p_curr_state_str , "Radio_Mngr_App_Hsm_TopHndlr \n",
								sizeof("Radio_Mngr_App_Hsm_TopHndlr \n"));
            HSM_STATE_TRANSITION(pst_me_radio_mngr, &radio_mngr_app_hsm_inactive_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Hsm_InactiveHndlr                                */
/*===========================================================================*/
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_InactiveHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handeled */
	Tu32 u32_slot = 0;

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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Hsm_InactiveHndlr");
            SYS_RADIO_MEMCPY( (void*)pst_me_radio_mngr->u8p_curr_state_str ,"Radio_Mngr_App_Hsm_InactiveHndlr \n",
								sizeof("Radio_Mngr_App_Hsm_InactiveHndlr \n"));
        }
        break;

        case RADIO_MNGR_APP_STARTUP_REQID:
        {
            /*Extracting the Market and Radio Component Type type*/
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_Market),				    pst_msg->data, sizeof(Te_Radio_Mngr_App_Market),    &u32_slot);
			ExtractParameterFromMsg(&(pst_me_radio_mngr->u8_RadioComponentInfo),    pst_msg->data, sizeof(Tu8),    						&u32_slot);
			
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Startup Request Received");
			
			/*Startup Audio Issue Fix Start*/	
		
			/*Reading the LSM, Starttype data and Market from Last mode api's*/
			pst_me_radio_mngr->st_inst_hsm.u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_READ(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																									&(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info));

			/*Checking the start type*/													
			if(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSMMarket == (Tu8)pst_me_radio_mngr->e_Market &&				
								(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockOne == RADIO_MNGR_APP_WARM_START_BLOCKONE && 
								 pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockTwo == RADIO_MNGR_APP_WARM_START_BLOCKTWO     ) )
			{
				/*Updating to HMI IF common data object Band to use immidiately with Getmode request*/
				Update_HMI_IF_Common_Data(pst_me_radio_mngr);
			}else{/*FOR MISRA C*/}
			
			/*Startup Audio Issue Fix End*/	
			/*<AM>-RadioComponentInfo Variable Bit Positions-- 0th-> AM, 1st -> FM, 2nd-> DAB*/
			if(LIB_ISBITSET(pst_me_radio_mngr->u8_RadioComponentInfo, 0u))
			{
				pst_me_radio_mngr->st_inst_hsm.b_AM_BandStatus = RADIO_MANAGER_AM_BAND_SUPPORTED;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AM is Activated");
			}
			else
			{
				pst_me_radio_mngr->st_inst_hsm.b_AM_BandStatus = RADIO_MANAGER_AM_BAND_NOT_SUPPORTED;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]AM is Deactivated");
			}
			
			/*<AM>-RadioComponentInfo Variable Bit Positions-- 0th-> AM, 1st -> FM, 2nd-> DAB*/
			if(LIB_ISBITSET(pst_me_radio_mngr->u8_RadioComponentInfo, 2u))
			{
				pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus = RADIO_MANAGER_DAB_BAND_SUPPORTED;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB is Supporting");
			}
			else
			{
				pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus = RADIO_MANAGER_DAB_BAND_NOT_SUPPORTED;
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]DAB is Not Supporting");
			}

            /*Transiting to Active start state to give further start up requests*/
			HSM_STATE_TRANSITION(pst_me_radio_mngr, &radio_mngr_app_hsm_active_start_state);
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveHndlr                                  */
/*===========================================================================*/
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handeled */
	Tu32 u32_slot = 0;

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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Hsm_ActiveHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr->u8p_curr_state_str , "Radio_Mngr_App_Hsm_ActiveHndlr \n",
								sizeof("Radio_Mngr_App_Hsm_ActiveHndlr \n"));
        }
        break;
		
		case RADIO_MNGR_APP_SHUTDOWN_REQID:
        {
            /*To give shut down related requests, transiting to Active stop state*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Shutdown Request");
        	HSM_STATE_TRANSITION(pst_me_radio_mngr, &radio_mngr_app_hsm_active_stop_state);
        }               
		break;
		
		/*Handling DAB Up-Notification*/
		case RADIO_MNGR_APP_UPNOT_RECEIVED_NOTIFYID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Up Notification Received");
			pst_me_radio_mngr->st_inst_hsm.e_DAB_UpNot_Status = RADIO_MNGR_APP_DAB_UP_NOTIFICATION_RECEIVED;
			/*Sending Up-Notification status to HMI*/
			Radio_Mngr_App_Notify_Components_Status(pst_me_radio_mngr->st_inst_hsm.e_activeBand, pst_me_radio_mngr->st_inst_hsm.e_AMFMTunerStatus, pst_me_radio_mngr->st_inst_hsm.e_DABTunerStatus, pst_me_radio_mngr->st_inst_hsm.e_DAB_UpNot_Status);
		}
		break;
		
		case RADIO_MNGR_APP_FACTORY_RESET_REQID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Factory Reset Requested");
			AMFM_App_Request_FactoryReset();
			
			if(pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && 
						pst_me_radio_mngr->st_inst_hsm.e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE &&
						pst_me_radio_mngr->st_inst_hsm.e_DABTunerStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
			{
				DAB_App_Request_FactoryReset();
			}else{/*FOR MISRA C*/}
			if(pst_me_radio_mngr->st_inst_hsm.e_activeBand == pst_me_radio_mngr->st_inst_hsm.e_Curr_Audio_Band)
			{
				Audio_Manager_Request_Mute(pst_me_radio_mngr->st_inst_hsm.e_activeBand);
			}
			else
			{
				Audio_Manager_Request_Mute(pst_me_radio_mngr->st_inst_hsm.e_Curr_Audio_Band);
			}			
		}
		break;
		
		case RADIO_MNGR_APP_AMFM_FACTORY_RESET_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_AMFM_FactoryReset_ReplyStatus), pst_msg->data, sizeof(Te_RADIO_ReplyStatus), &u32_slot);
			
			if(pst_me_radio_mngr->e_AMFM_FactoryReset_ReplyStatus != REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Factory Reset Failure from AMFM");
				Radio_Mngr_App_Response_Factory_Reset(REPLYSTATUS_FAILURE);
				Audio_Manager_Request_DeMute(pst_me_radio_mngr->st_inst_hsm.e_activeBand);
			}
			else if((pst_me_radio_mngr->e_DAB_FactoryReset_ReplyStatus == REPLYSTATUS_SUCCESS) || 
						(pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus != RADIO_MANAGER_DAB_BAND_SUPPORTED || 
						 pst_me_radio_mngr->st_inst_hsm.e_DABActiveDeActiveStatus != RADIO_MNGR_APP_SRC_ACTIVE ||
						 pst_me_radio_mngr->st_inst_hsm.e_DABTunerStatus != RADIO_FRMWK_COMP_STATUS_NORMAL) )
			{
				if(Radio_Mngr_App_StartFactoryReset(pst_me_radio_mngr) == REPLYSTATUS_SUCCESS)
				{
					Radio_Mngr_App_Request_InstHSM_FactoryReset();
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Factory Reset Failure from RM");
					Radio_Mngr_App_Response_Factory_Reset(REPLYSTATUS_FAILURE);
					Audio_Manager_Request_DeMute(pst_me_radio_mngr->st_inst_hsm.e_activeBand);
				}
			}
			else
			{/*FOR MISRA C*/}
		}
		break;
		
		case RADIO_MNGR_APP_DAB_FACTORY_RESET_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_DAB_FactoryReset_ReplyStatus), pst_msg->data, sizeof(Te_RADIO_ReplyStatus), &u32_slot);

			if(pst_me_radio_mngr->e_DAB_FactoryReset_ReplyStatus != REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Factory Reset Failure from DAB");
				Radio_Mngr_App_Response_Factory_Reset(REPLYSTATUS_FAILURE);
				Audio_Manager_Request_DeMute(pst_me_radio_mngr->st_inst_hsm.e_activeBand);
			}
			else if(pst_me_radio_mngr->e_AMFM_FactoryReset_ReplyStatus == REPLYSTATUS_SUCCESS)
			{
				if(Radio_Mngr_App_StartFactoryReset(pst_me_radio_mngr) == REPLYSTATUS_SUCCESS)
				{
					Radio_Mngr_App_Request_InstHSM_FactoryReset();
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Factory Reset Failure from RM");
					Radio_Mngr_App_Response_Factory_Reset(REPLYSTATUS_FAILURE);
					Audio_Manager_Request_DeMute(pst_me_radio_mngr->st_inst_hsm.e_activeBand);
				}
			}
			else{/*FOR MISRA C*/}
		}
		break;
		
		case RADIO_MNGR_APP_INST_HSM_FACTORY_RESET_DONE_RESID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Factory Reset Success");
			
			pst_me_radio_mngr->e_AMFMStartReplyStatus 			= REPLYSTATUS_INVALID_PARAM;
			pst_me_radio_mngr->e_DABStartReplyStatus			= REPLYSTATUS_INVALID_PARAM;
			pst_me_radio_mngr->e_DAB_FactoryReset_ReplyStatus 	= REPLYSTATUS_INVALID_PARAM;
			pst_me_radio_mngr->e_AMFM_FactoryReset_ReplyStatus	= REPLYSTATUS_INVALID_PARAM;
			pst_me_radio_mngr->st_inst_hsm.b_FM_StartStatus    = RADIO_MNGR_APP_COLD_START;
			pst_me_radio_mngr->st_inst_hsm.b_DAB_StartStatus   = RADIO_MNGR_APP_COLD_START;
			
			Radio_Mngr_App_Response_Factory_Reset(REPLYSTATUS_SUCCESS);
			HSM_STATE_TRANSITION(pst_me_radio_mngr, &radio_mngr_app_hsm_inactive_state);			
		}
		break;
		
		case RADIO_MNGR_APP_DAB_VERSION_NOTIFYID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr->st_DAB_Version_Info), pst_msg->data, sizeof(Ts_Radio_Mngr_App_DABVersion_Reply), &u32_slot);
			Radio_Mngr_App_Notify_FirmwareVersion(pst_me_radio_mngr);
		}
		break;

		default:
        {
			Radio_Mngr_App_Inst_Hsm_HandleMsg(&(pst_me_radio_mngr->st_inst_hsm), pst_msg);
		}
        break;
    }
	return pst_ret;
}

/*===========================================================================*/
/*  Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveStartHndlr                                  */
/*===========================================================================*/
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveStartHndlr(Ts_Radio_Mngr_App_Hsm *pst_me_radio_mngr, Ts_Sys_Msg *pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handeled */
	Tu32 u32_slot = 0;
	Tu8 u8_presetindex;
	
	PRINT_MSG_DATA(pst_msg);
	
    switch ((pst_msg->msg_id))
    {
        case HSM_MSGID_ENTRY:
        {
			/* do Nothing */ 
        }
        break;

        case HSM_MSGID_START:
        {
			/*To Print the Current State for Debug*/
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Hsm_ActiveStartHndlr");
			
			pst_me_radio_mngr->e_AMFMStartReplyStatus 			= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr->e_DABStartReplyStatus 			= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr->e_InstHSMReplyStatus				= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr->st_inst_hsm.e_DAB_UpNot_Status	= RADIO_MNGR_APP_DAB_UP_NOTIFICATION_INVALID;
			pst_me_radio_mngr->e_AMFM_FactoryReset_ReplyStatus	= REPLYSTATUS_FAILURE;
			pst_me_radio_mngr->e_DAB_FactoryReset_ReplyStatus	= REPLYSTATUS_FAILURE;

			SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr->u8p_curr_state_str , "Radio_Mngr_App_Hsm_ActiveStartHndlr \n",sizeof("Radio_Mngr_App_Hsm_ActiveStartHndlr \n"));
			
			pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSMMarket = (Tu8)RCE_MARKET_INVALID_RM;
			
			/*Reading the LSM, Starttype data and Market from Last mode api's*/
			pst_me_radio_mngr->st_inst_hsm.u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_READ(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																									&(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info));	
			if(!(pst_me_radio_mngr->st_inst_hsm.u8_NVM_LastMode_ReadWriteStatus))
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]EEPROM Read Success");
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]EEPROM Read Failed");
			}			
			
			/*Checking the start type*/													
			if(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSMMarket == (Tu8)pst_me_radio_mngr->e_Market &&				
								pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockOne == RADIO_MNGR_APP_WARM_START_BLOCKONE && 
								 pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockTwo == RADIO_MNGR_APP_WARM_START_BLOCKTWO && 
								 pst_me_radio_mngr->st_inst_hsm.u8_NVM_LastMode_ReadWriteStatus == RADIO_MNGR_APP_VALUE_ZERO)
			{
				pst_me_radio_mngr->st_inst_hsm.b_EEPROM_Status = EEPROM_KNOWN_VALUES;

				/*Updating to HMI IF common data object Band to use immidiately with Getmode request*/
				Update_HMI_IF_Common_Data(pst_me_radio_mngr);
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]EEPROM: Market, Blocks and Read Status Check Success");

				/*Reading the Settings and Preset and StL from the NVM*/
				pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus = SYS_NVM_READ(NVM_ID_TUNER_RADIOMNGR_APP, &(pst_me_radio_mngr->st_inst_hsm.u8_Settings), 
																			(Tu32)RADIO_MNGR_APP_NVM_DATA_SIZE, &(pst_me_radio_mngr->st_inst_hsm.u32_NVM_Read_Write_Bytes));
																
				if(!(pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]FLASH Read Success");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FLASH Read Failed");
				}
				
				/*FLASH Validation Success consider it as Warm startup with LSM parameters*/												
				if(pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockOne                        == RADIO_MNGR_APP_WARM_START_BLOCKONE 	&& 
										pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockTwo   == RADIO_MNGR_APP_WARM_START_BLOCKTWO  	&& 
										pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockThree == RADIO_MNGR_APP_WARM_START_BLOCKTHREE	&&
										pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus  	   == RADIO_MNGR_APP_VALUE_ZERO)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]After EEPROM Success, FLASH: Blocks, and Read Status Success => Warm Startup");
					/*Updating as warm startup if the NVM written value & read value are same*/
					pst_me_radio_mngr->st_inst_hsm.u8_StartType = RADIO_MNGR_APP_WARM_START;
					/*Function to update Application layer station List structures*/
					Radio_Manager_App_Update_AppLayer_STL(pst_me_radio_mngr);
				}
				/*FLASH Validation fails consider it as Cold startup with LSM parameters*/
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]After EEPROM Success, FLASH: Check failure => ColdStartup");
					
					/*Updating as cold startup */
					pst_me_radio_mngr->st_inst_hsm.u8_StartType = RADIO_MNGR_APP_COLD_START;
					
					/*To Validate the FLASH, updating the FLASH to known values*/
					pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockOne	 = RADIO_MNGR_APP_WARM_START_BLOCKONE;
					pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockTwo	 = RADIO_MNGR_APP_WARM_START_BLOCKTWO;
					pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockThree	 = RADIO_MNGR_APP_WARM_START_BLOCKTHREE;
						
					/*To avoid using garbage value during FLASH Fails,cleared the StL Parameters*/
					memset(&(pst_me_radio_mngr->st_inst_hsm.st_RadioStationList), 0, sizeof(Ts_Radio_Mngr_App_RadioStationList));	
					/*To avoid using garbage value during FLASH Fails,cleared the Preset list Parameters*/
					memset(&(pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList), 0, sizeof(Ts_Radio_Mngr_App_Preset_Mixed_List));	
						
					pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList.u8_NumPresetList = 0;	
					
					/*Updating the all preset stations bands with Invalid*/
					for(u8_presetindex = RADIO_MNGR_APP_VALUE_ZERO; u8_presetindex < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_presetindex++)
					{
						pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_presetindex].e_Band = RADIO_MNGR_APP_BAND_INVALID;
					}
					
					/*Function to update Flash Memory parameters*/
					Radio_Manager_App_Write_Flash_Data(&(pst_me_radio_mngr->st_inst_hsm));
				}				
			}
			/*If LSM Market & Tuned market is not same then follow cold start-up strategy*/
			else if(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSMMarket != (Tu8)pst_me_radio_mngr->e_Market)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]After EEPROM checks fail, Check only Market with LSM: Not Matched => ColdStartup");
				
				pst_me_radio_mngr->st_inst_hsm.b_EEPROM_Status = EEPROM_UNKNOWN_VALUES;
				
				/*Updating as cold startup */
				pst_me_radio_mngr->st_inst_hsm.u8_StartType = RADIO_MNGR_APP_COLD_START;
				
				/*To avoid using garbage value during cold startup,cleared the Last Mode Parameters*/
				memset(&(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info), 0, RADIO_MNGR_APP_NVM_LASTMODE_SIZE);
				
				/*Updating the LSM market with the system market*/
				pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSMMarket = (Tu8)pst_me_radio_mngr->e_Market;
				
				/*To Validate the FLASH, updating the FLASH to known values*/
				pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockOne	 = RADIO_MNGR_APP_WARM_START_BLOCKONE;
				pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockTwo	 = RADIO_MNGR_APP_WARM_START_BLOCKTWO;
				pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockThree	 = RADIO_MNGR_APP_WARM_START_BLOCKTHREE;
					
				/*To avoid using garbage value during FLASH Fails,cleared the StL Parameters*/
				memset(&(pst_me_radio_mngr->st_inst_hsm.st_RadioStationList), 0, sizeof(Ts_Radio_Mngr_App_RadioStationList));	
				/*To avoid using garbage value during FLASH Fails,cleared the Preset list Parameters*/
				memset(&(pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList), 0, sizeof(Ts_Radio_Mngr_App_Preset_Mixed_List));
				
				pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList.u8_NumPresetList = 0;
				
				/*Updating the all preset stations bands with Invalid*/
				for(u8_presetindex = RADIO_MNGR_APP_VALUE_ZERO; u8_presetindex < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_presetindex++)
				{
					pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_presetindex].e_Band = RADIO_MNGR_APP_BAND_INVALID;
				}
				
				/*Function to update Flash Memory parameters*/
				Radio_Manager_App_Write_Flash_Data(&(pst_me_radio_mngr->st_inst_hsm));
				
				/*To confirm the startup type as warm by assigning the below values,and writting into lastmode api's during shutdown*/
				pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockOne	 = RADIO_MNGR_APP_WARM_START_BLOCKONE;
				pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockTwo	 = RADIO_MNGR_APP_WARM_START_BLOCKTWO;
			}
			/*EEPROM Validation/ LSMMarket Validation Fails Consider it as Cold Startup*/			
			else if(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSMMarket == (Tu8)pst_me_radio_mngr->e_Market)
			{				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]After EEPROM checks fail, Check only Market with LSM: Matched => Read FLASH and Validate");
				
				pst_me_radio_mngr->st_inst_hsm.b_EEPROM_Status = EEPROM_UNKNOWN_VALUES;
				
				/*To avoid using garbage value during cold startup,cleared the Last Mode Parameters*/
				memset(&(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info), 0, RADIO_MNGR_APP_NVM_LASTMODE_SIZE);
				
				/*Updating the LSM market with the system market*/
				pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_LSMMarket = (Tu8)pst_me_radio_mngr->e_Market;
				pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus = SYS_NVM_READ(NVM_ID_TUNER_RADIOMNGR_APP, &(pst_me_radio_mngr->st_inst_hsm.u8_Settings), 
																			(Tu32)RADIO_MNGR_APP_NVM_DATA_SIZE, &(pst_me_radio_mngr->st_inst_hsm.u32_NVM_Read_Write_Bytes));
																
				if(!(pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]FLASH Read Success");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][RM]FLASH Read Failed");
				}
				
				/*FLASH Validation Success consider it as Warm startup with LSM parameters*/												
				if(pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockOne                        == RADIO_MNGR_APP_WARM_START_BLOCKONE 	&& 
										pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockTwo   == RADIO_MNGR_APP_WARM_START_BLOCKTWO  	&& 
										pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockThree == RADIO_MNGR_APP_WARM_START_BLOCKTHREE	&&
										pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus  	   == RADIO_MNGR_APP_VALUE_ZERO)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]After EEPROM Chaeck Fails, FLASH: Blocks, and Read Status Success => Warm Startup");
					
					/*Updating as warm startup if the NVM written value & read value are same*/
					pst_me_radio_mngr->st_inst_hsm.u8_StartType = RADIO_MNGR_APP_WARM_START;
					/*Function to update Application layer station List structures*/
					Radio_Manager_App_Update_AppLayer_STL(pst_me_radio_mngr);
				}
				/*FLASH Validation fails consider it as Cold startup with LSM parameters*/
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE ,"[Radio][RM]After EEPROM Check Fails, FLASH: Check failure => ColdStartup");
					
					/*Updating as cold startup */
					pst_me_radio_mngr->st_inst_hsm.u8_StartType = RADIO_MNGR_APP_COLD_START;
					
					/*To Validate the FLASH, updating the FLASH to known values*/
					pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockOne	 = RADIO_MNGR_APP_WARM_START_BLOCKONE;
					pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockTwo	 = RADIO_MNGR_APP_WARM_START_BLOCKTWO;
					pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockThree	 = RADIO_MNGR_APP_WARM_START_BLOCKTHREE;
						
					/*To avoid using garbage value during FLASH Fails,cleared the StL Parameters*/
					memset(&(pst_me_radio_mngr->st_inst_hsm.st_RadioStationList), 0, sizeof(Ts_Radio_Mngr_App_RadioStationList));	
					/*To avoid using garbage value during FLASH Fails,cleared the Preset list Parameters*/
					memset(&(pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList), 0, sizeof(Ts_Radio_Mngr_App_Preset_Mixed_List));
				
					pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList.u8_NumPresetList = 0;
					/*Updating the all preset stations bands with Invalid*/
					for(u8_presetindex = RADIO_MNGR_APP_VALUE_ZERO; u8_presetindex < RADIO_MNGR_APP_MAX_PSML_SIZE; u8_presetindex++)
					{
						pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList.ast_presetlist[u8_presetindex].e_Band = RADIO_MNGR_APP_BAND_INVALID;
					}
				
					/*Function to update Flash Memory parameters*/
					Radio_Manager_App_Write_Flash_Data(&(pst_me_radio_mngr->st_inst_hsm));
				}
				
				/*To confirm the startup type as warm by assigning the below values,and writting into lastmode api's during shutdown*/
				pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockOne	 = RADIO_MNGR_APP_WARM_START_BLOCKONE;
				pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info.u8_StartType_BlockTwo	 = RADIO_MNGR_APP_WARM_START_BLOCKTWO;
			}else{/*FOR MISRA C*/}
 
            Update_Settings(pst_me_radio_mngr, pst_me_radio_mngr->e_Market);			

			/*Function for checking system start type*/
			StartupCheck(pst_me_radio_mngr);

			/* Requesting AMFM App layer for startup */
            AMFM_App_Request_Startup((Te_AMFM_App_Market)pst_me_radio_mngr->e_Market, pst_me_radio_mngr->st_inst_hsm.u8_Settings, pst_me_radio_mngr->st_inst_hsm.u8_StartType);						
        }
        break;
		
		case RADIO_MNGR_APP_AMFM_STARTUP_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_AMFMStartReplyStatus), pst_msg->data, sizeof(pst_me_radio_mngr->e_AMFMStartReplyStatus), &u32_slot);

			/* sending starttuner response to the HMI-IF based on DAB and AMFM startup response*/
			if(pst_me_radio_mngr->e_AMFMStartReplyStatus != REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Response_StartTuner(REPLYSTATUS_FAILURE);
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT,"[Radio][RM]Radio Startup Failure");
			}
			/*** added newely, after AMFM reciever configuration,datapath creation and audio select is done then only start up request has to be sent to DAB ***/
			/*If DAB supports then only send the request to App*/
			else if (pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED)
			{
				DAB_App_Request_Startup((Te_DAB_App_Market)pst_me_radio_mngr->e_Market, pst_me_radio_mngr->st_inst_hsm.u8_Settings, pst_me_radio_mngr->st_inst_hsm.u8_StartType);
			}
			else if (pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus != RADIO_MANAGER_DAB_BAND_SUPPORTED)
			{
				Radio_Mngr_App_Response_StartTuner(REPLYSTATUS_SUCCESS);
				Radio_Mngr_App_Inst_Hsm_Start(pst_me_radio_mngr->e_Market);
			}
			else
			{
				/*FOR MISRA C*/
			}
		}
		break;

		case RADIO_MNGR_APP_DAB_STARTUP_DONE_RESID:
		{
			
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_DABStartReplyStatus), pst_msg->data, sizeof(pst_me_radio_mngr->e_DABStartReplyStatus), &u32_slot);

			/* sending starttuner response to the HMI-IF based on DAB and AMFM startup response*/
			if (pst_me_radio_mngr->e_DABStartReplyStatus != REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Response_StartTuner(REPLYSTATUS_FAILURE);

				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[Radio][RM]Radio Startup Failure");
			}
			else if (pst_me_radio_mngr->e_DABStartReplyStatus == REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Response_StartTuner(REPLYSTATUS_SUCCESS);
				Radio_Mngr_App_Inst_Hsm_Start(pst_me_radio_mngr->e_Market);
			}
			else{/*FOR MISRA C*/ }
		}
		break;
	
		case RADIO_MNGR_INST_HSM_START_DONE:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_InstHSMReplyStatus), pst_msg->data, sizeof(pst_me_radio_mngr->e_InstHSMReplyStatus), &u32_slot);
			if(pst_me_radio_mngr->e_InstHSMReplyStatus == REPLYSTATUS_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio Startup Success");
				
                /*Notifying settings related updates to HMI after instant HSM done*/
				Radio_Mngr_App_Notify_Settings(pst_me_radio_mngr->st_inst_hsm.e_DABFMLinking_Switch, pst_me_radio_mngr->st_inst_hsm.e_TA_Anno_Switch, pst_me_radio_mngr->st_inst_hsm.e_RDSSettings, pst_me_radio_mngr->st_inst_hsm.e_Info_Anno_Switch, pst_me_radio_mngr->st_inst_hsm.e_MultiplexSettings);
				HSM_STATE_TRANSITION(pst_me_radio_mngr, &radio_mngr_app_hsm_active_idle_state);
			}
			else
			{/* FOR MISRA C*/}
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveStopHndlr                                  */
/*===========================================================================*/
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveStopHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handeled */
	Tu32 u32_slot = 0;

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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Hsm_ActiveStopHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr->u8p_curr_state_str , "Radio_Mngr_App_Hsm_ActiveStopHndlr \n",
								sizeof("Radio_Mngr_App_Hsm_ActiveStopHndlr \n"));
			pst_me_radio_mngr->e_AMFMShutdownReplyStatus = REPLYSTATUS_FAILURE;
			pst_me_radio_mngr->e_DABShutdownReplyStatus  = REPLYSTATUS_FAILURE;
            /*Requesting AMFM App for shut down*/
			AMFM_APP_Request_Shutdown();

            /*Based on the DAB supported only requesting for DAB shut down*/
			if(pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus == RADIO_MANAGER_DAB_BAND_SUPPORTED && pst_me_radio_mngr->st_inst_hsm.e_DABActiveDeActiveStatus == RADIO_MNGR_APP_SRC_ACTIVE)
			{
				DAB_App_Request_Shutdown();
			}
			else{/*For MISRA C*/}
		}
        break;

		case RADIO_MNGR_APP_AMFM_SHUTDOWN_DONE_RESID:
		{
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_AMFMShutdownReplyStatus), pst_msg->data, sizeof(pst_me_radio_mngr->e_AMFMShutdownReplyStatus), &u32_slot);

			/* sending starttuner response to the HMI-IF based on DAB and AMFM startup response*/
			if(pst_me_radio_mngr->e_AMFMShutdownReplyStatus != REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Response_Shutdown(REPLYSTATUS_FAILURE);
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT,"[Radio][RM]Radio Shutdown Failure");
			}
			else if( (pst_me_radio_mngr->e_DABShutdownReplyStatus == REPLYSTATUS_SUCCESS) ||
						(pst_me_radio_mngr->st_inst_hsm.b_DAB_BandStatus != RADIO_MANAGER_DAB_BAND_SUPPORTED || pst_me_radio_mngr->st_inst_hsm.e_DABActiveDeActiveStatus != RADIO_MNGR_APP_SRC_ACTIVE) )
			{
				Radio_Mngr_App_Inst_Hsm_Stop();
			}
			else{/*For MISRA C*/}
		}
		break;

		case RADIO_MNGR_APP_DAB_SHUTDOWN_DONE_RESID:
		{		
			ExtractParameterFromMsg(&(pst_me_radio_mngr->e_DABShutdownReplyStatus), pst_msg->data, sizeof(pst_me_radio_mngr->e_DABShutdownReplyStatus), &u32_slot);

			/* sending starttuner response to the HMI-IF based on DAB and AMFM startup response*/
			if(pst_me_radio_mngr->e_DABShutdownReplyStatus != REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Response_Shutdown(REPLYSTATUS_FAILURE);
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT,"[Radio][RM]Radio Shutdown Failure");
			}
			else if((pst_me_radio_mngr->e_AMFMShutdownReplyStatus) == REPLYSTATUS_SUCCESS)
			{
				Radio_Mngr_App_Inst_Hsm_Stop();
			}
			else{/*For MISRA C*/}
		}
		break;

		case RADIO_MNGR_INST_HSM_STOP_DONE:
        {
            ExtractParameterFromMsg(&(pst_me_radio_mngr->e_InstHSMReplyStatus), pst_msg->data, sizeof(pst_me_radio_mngr->e_InstHSMReplyStatus), &u32_slot);

			if(pst_me_radio_mngr->e_InstHSMReplyStatus == REPLYSTATUS_SUCCESS)
			{
				/*Function to update Flash Memory parameters*/
				Radio_Manager_App_Write_Flash_Data(&(pst_me_radio_mngr->st_inst_hsm));
				
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio Shutdown Success");									

				Radio_Mngr_App_Response_Shutdown(REPLYSTATUS_SUCCESS);
                /*After instant hsm stop done successfully, transiting the state to inactive state*/
				HSM_STATE_TRANSITION(pst_me_radio_mngr, &radio_mngr_app_hsm_inactive_state);
			}
			else{/*For MISRA C*/}
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveErrorHndlr                                  */
/*===========================================================================*/
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveErrorHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handeled */

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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Hsm_ActiveErrorHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr->u8p_curr_state_str , "Radio_Mngr_App_Hsm_ActiveErrorHndlr \n",
								sizeof("Radio_Mngr_App_Hsm_ActiveErrorHndlr \n"));
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
/*  Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveIdleHndlr                   */
/*===========================================================================*/
Ts_Sys_Msg*  Radio_Mngr_App_Hsm_ActiveIdleHndlr(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr, Ts_Sys_Msg* pst_msg)
{
    Ts_Sys_Msg* pst_ret = NULL; /* mark the message as handeled */

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
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]Radio_Mngr_App_Hsm_ActiveIdleHndlr");
            SYS_RADIO_MEMCPY((void*)pst_me_radio_mngr->u8p_curr_state_str ,"Radio_Mngr_App_Hsm_ActiveIdleHndlr \n",
								sizeof("Radio_Mngr_App_Hsm_ActiveIdleHndlr \n"));
        }
        break;

		case RADIO_MNGR_APP_INST_RESID_DONE_CASES:
		case RADIO_MNGR_APP_INST_REQID_CASES:
		case RADIO_MNGR_APP_INST_NOTIFYID_CASES:
		{
            /*Passing the Messages From Main HSM  to Instance HSM*/
			Radio_Mngr_App_Inst_Hsm_HandleMsg(&pst_me_radio_mngr->st_inst_hsm, pst_msg); 
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

/*====================================================================================================================*/
/*  Te_RADIO_ReplyStatus Radio_Mngr_App_StartFactoryReset(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr)                   */
/*====================================================================================================================*/
Te_RADIO_ReplyStatus Radio_Mngr_App_StartFactoryReset(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr)
{	
	memset(&(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info), 0, (size_t)RADIO_MNGR_APP_NVM_LASTMODE_SIZE);
	
	/*Clearing FLash Data*/
	pst_me_radio_mngr->st_inst_hsm.u8_Settings 				= 0;
	pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockOne 	= 0;
	pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockTwo 	= 0;
	pst_me_radio_mngr->st_inst_hsm.u8_StartType_BlockThree 	= 0;
	memset(&(pst_me_radio_mngr->st_inst_hsm.st_RadioStationList), 0, sizeof(Ts_Radio_Mngr_App_RadioStationList));
	memset(&(pst_me_radio_mngr->st_inst_hsm.st_PrestMixedList), 0, sizeof(Ts_Radio_Mngr_App_Preset_Mixed_List));
	
	pst_me_radio_mngr->st_inst_hsm.u8_NVM_LastMode_ReadWriteStatus = SYS_TUNER_LSM_WRITE(RADIO_MNGR_TUNER_LSM, RADIO_MNGR_APP_NVM_LASTMODE_SIZE, 
																										&(pst_me_radio_mngr->st_inst_hsm.st_LSM_Station_Info));																										
	Radio_Manager_EEPROM_Log(pst_me_radio_mngr->st_inst_hsm.u8_NVM_LastMode_ReadWriteStatus);
	
	pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus = SYS_NVM_WRITE(NVM_ID_TUNER_RADIOMNGR_APP, &(pst_me_radio_mngr->st_inst_hsm.u8_Settings), 
		   												 (Tu32)RADIO_MNGR_APP_NVM_DATA_SIZE, &(pst_me_radio_mngr->st_inst_hsm.u32_NVM_Read_Write_Bytes));
																	
	if(!(pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus))
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][RM]FLASH Write Success");
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR,"[Radio][RM]FLASH Write Failed");
	}
	
	if(pst_me_radio_mngr->st_inst_hsm.u8_NVM_ReplyStatus == RADIO_MNGR_APP_VALUE_ZERO && pst_me_radio_mngr->st_inst_hsm.u8_NVM_LastMode_ReadWriteStatus == RADIO_MNGR_APP_VALUE_ZERO)	
	{
		return(REPLYSTATUS_SUCCESS);
	}	
	else
	{
		return(REPLYSTATUS_FAILURE);
	}
}

