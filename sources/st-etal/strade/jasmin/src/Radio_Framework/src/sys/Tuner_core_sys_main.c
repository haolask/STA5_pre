/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_main.c																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains system layer initalization API definition						*
*																											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "sys_task.h"
#include "Tuner_core_sys_main.h"
#include "sys_nvm.h"
#ifdef UITRON
#include "sw_mem_manage_api.h"
#endif
#include "hmi_if_app_notify.h"
#include "radio_mngr_app_request.h"
#include "etal.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
extern Ts_task_info	st_sys_task_info[SYS_TASK_CNT];	/* DATA BASE FOR TASKS AND MBX*/

/*-----------------------------------------------------------------------------
    variables (Global)
-----------------------------------------------------------------------------*/
Tu8 	u8_radio_component_info;		/* Diag information */
Te_Radio_Framework_Market	e_market_info; /* Market information */
Te_RADIO_ReplyStatus		e_Factory_Reset_Status = REPLYSTATUS_INVALID_PARAM;  /* Factory Reset Status */

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/
static Te_Sys_SRC_Activate_DeActivate		e_SRC_Act_Deact; /* Source Active and Deactive */
static Te_Sys_Mode_Type						e_Mode;			 /* Band type*/

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/


/*===========================================================================*/
/*  void SYS_MAIN_EtalInit                                                    */
/*===========================================================================*/
void SYS_MAIN_EtalInit(Radio_EtalHardwareAttr attr)
{
	Tu8 u8_ETAL_Init_Status;
	u8_ETAL_Init_Status = ETAL_Init(attr);
	if (u8_ETAL_Init_Status == FALSE)
	{
		Sys_ETALHwConfig_Response(REPLYSTATUS_SUCCESS);
	}
	else
	{
		Sys_ETALHwConfig_Response(REPLYSTATUS_FAILURE);
	}
}

/*===========================================================================*/
/*  void SYS_MAIN_EtalDeInit                                                    */
/*===========================================================================*/
void SYS_MAIN_EtalDeInit(void)
{
	Tu8 u8_ETAL_DeInit_Status;
	u8_ETAL_DeInit_Status = ETAL_Deinit();
	if (u8_ETAL_DeInit_Status == FALSE)
	{
		Sys_ETALHwDeConfig_Response(REPLYSTATUS_SUCCESS);
	}
	else
	{
		Sys_ETALHwDeConfig_Response(REPLYSTATUS_FAILURE);
	}
}

/*===========================================================================*/
/*  void SYS_MAIN_StartOS                                                    */
/*===========================================================================*/
void SYS_MAIN_StartOS(Te_Radio_Framework_Variant e_variant, Te_Radio_Framework_Market e_market, Tu8 u8_radio_resource)
{	 
	Tu8 u8_task_result		= OS_RESOURCE_CREATE_ERR;
	Tu8 u8_update_status	= UPDATE_STATUS_FAILURE;
	Tu8 u8_ETAL_Init_Status = OS_RESOURCE_CREATE_ERR;
	Tu8	u8_reg_res = SYS_NVM_FAILED;
	u8_radio_component_info = u8_radio_resource;
	e_market_info			= e_market;

	u8_reg_res 	= sys_nvm_file_open();
	
	if(u8_reg_res == SYS_NVM_SUCCESS)
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO,"[RADIO][FW] NVM_FILE OPEN SUCCESS");
	}
	else
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] NVM_FILE OPEN FAILED");
	}
	
	u8_reg_res = sys_lsm_file_open();

	if(u8_reg_res == SYS_LSM_SUCCESS)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] LSM_FILE OPEN SUCCESS");
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] LSM_FILE OPEN FAILED");
	}
	/* To Check DAB, RDS availability with Variant, Market and Radio resource  */
	u8_update_status = Sys_get_variant_details( e_variant, e_market, u8_radio_resource);
	if(u8_update_status == UPDATE_STATUS_SUCCESS)
	{	
		/* HSM Intialization based on variant and market*/
		Sys_HSM_intialization();
	
		/* OS RESOURCES CREATION (TASK,MAILBOX,SEMAPHORE,MUTEX)*/
		u8_task_result = SYS_TASK_CreateTasks();

		/* OS RESOURCES CREATION SUCCESS -- 0 */
		if(u8_task_result == OS_RESOURCE_CREATE_SUCCESS)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][FW] RADIO TASKS ARE CREATED SUCCESSFULLY\n");

		   /* Configure ETAL Components, Config_Audio_Path */
		   u8_ETAL_Init_Status = ETAL_Config();

		   if(u8_ETAL_Init_Status == OS_RESOURCE_CREATE_SUCCESS)
		   {
			   Radio_Mngr_App_Request_StartTuner((Te_Radio_Mngr_App_Market)e_market, u8_radio_resource);
		   }
		   else
		   {
			   RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO ETAL INTIALIZATION FAILED\n");
		   }
		}
		else
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO TASKS CREATION FAILED\n");
		}
	}
	else
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO STARTUP FAILURE\n");
	}
}
/*======================================================================================================================*/
/*  void SYS_MAIN_StopOS()			                                                                                    */
/*======================================================================================================================*/
void SYS_MAIN_StopOS()
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] RADIO SHUTDOWN REQUEST\n");
	Radio_Mngr_App_Request_ShutDownTuner();
}

/*=======================================================================================================================*/
/*  void Sys_ETALHwConfig_Response(Te_RADIO_ReplyStatus e_etal_startup_reply_status)                                     */
/*=======================================================================================================================*/
void Sys_ETALHwConfig_Response(Te_RADIO_ReplyStatus e_etal_startup_reply_status)
{
	if (e_etal_startup_reply_status == REPLYSTATUS_SUCCESS)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] ETAL Initialization Success\n");
	}
	/* ETAL INIT FAILURE -- 1 */
	else if (e_etal_startup_reply_status == REPLYSTATUS_FAILURE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] ETAL Initialization Failure\n");
	}
	else
	{
		/* Do nothing */
	}
	Radio_Response_ETALHwConfig(e_etal_startup_reply_status);
}

/*=======================================================================================================================*/
/*  void Sys_ETALHwDeConfig_Response(Te_RADIO_ReplyStatus e_etal_startup_reply_status)                                     */
/*=======================================================================================================================*/
void Sys_ETALHwDeConfig_Response(Te_RADIO_ReplyStatus e_etal_deinit_reply_status)
{
	if (e_etal_deinit_reply_status == REPLYSTATUS_SUCCESS)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] ETAL Deinitialization Success\n");
	}
	/* ETAL DEINIT FAILURE -- 1 */
	else if (e_etal_deinit_reply_status == REPLYSTATUS_FAILURE)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] ETAL Deinitialization Failure\n");
	}
	else
	{
		/* Do nothing */
	}
	Radio_Response_ETALHwDeConfig(e_etal_deinit_reply_status);
}

/*=======================================================================================================================*/
/*  void Sys_Startup_Response(Te_RADIO_ReplyStatus e_startup_reply_status)                                         */
/*=======================================================================================================================*/
void Sys_Startup_Response(Te_RADIO_ReplyStatus e_startup_reply_status)
{
	if(e_Factory_Reset_Status == REPLYSTATUS_SUCCESS)
	{
		/* STARTUP SUCCESS -- 0 */
		if (e_startup_reply_status == REPLYSTATUS_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] RADIO STARTUP SUCCESS\n");
		}
		/* STARTUP FAILURE -- 1 */
		else if (e_startup_reply_status == REPLYSTATUS_FAILURE)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO STARTUP FAILURE\n");
			e_Factory_Reset_Status = REPLYSTATUS_FAILURE;
		}
		/* STARTUP TIMEOUT -- 2 */
		else if (e_startup_reply_status == REPLYSTATUS_REQ_TIMEOUT)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO STARTUP TIMEOUT\n");
			e_Factory_Reset_Status = REPLYSTATUS_FAILURE;
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO STARTUP INVALID REPLY STATUS\n");
			e_Factory_Reset_Status = REPLYSTATUS_FAILURE;
		}
		Radio_Response_FactoryReset(e_Factory_Reset_Status);
	}
	else
	{
		/* STARTUP SUCCESS -- 0 */
		if (e_startup_reply_status == REPLYSTATUS_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] RADIO STARTUP SUCCESS\n");
		}
		/* STARTUP FAILURE -- 1 */
		else if (e_startup_reply_status == REPLYSTATUS_FAILURE)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO STARTUP FAILURE\n");
		}
		/* STARTUP TIMEOUT -- 2 */
		else if (e_startup_reply_status == REPLYSTATUS_REQ_TIMEOUT)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO STARTUP TIMEOUT\n");
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO STARTUP INVALID REPLY STATUS\n");
		}
		/* STARTUP RESPONSE NOTIFYING TO HMI_IF */
		Notify_UpdateStartRadioStatus(e_startup_reply_status);
	}	
}
/*=========================================================================================================================*/
/*  void Sys_Shutdown_Response(Te_RADIO_ReplyStatus e_shutdown_reply_status)                                        */
/*=========================================================================================================================*/
void Sys_Shutdown_Response(Te_RADIO_ReplyStatus e_shutdown_reply_status)
{
	Tu8	u8_resrc_release = OS_RESOURCE_RELEASE_ERR;
	Te_RADIO_ReplyStatus e_PF_shutdown_status = REPLYSTATUS_FAILURE;
	Tu8 u8_ret_value = SYS_LSM_FAILED;

	/* SHUTDOWN RESPONSE SUCCESS -- 0 */
	if(e_shutdown_reply_status == REPLYSTATUS_SUCCESS)
	{
		/* OS RESOURCES RELEASING */
		u8_resrc_release  = Radio_Sys_Task_Delete();
		/* OS RESOURCES RELEASING SUCCESS -- 0 */
		if(u8_resrc_release == OS_RESOURCE_RELEASE_SUCCESS)
		{
			e_PF_shutdown_status = REPLYSTATUS_SUCCESS;
			u8_ret_value = sys_lsm_file_close();
			if (u8_ret_value == SYS_LSM_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] LSM_FILE CLOSE SUCCESS\n");
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] LSM_FILE CLOSE FAILURE\n");
			}
			u8_ret_value = SYS_NVM_FAILED;
			u8_ret_value = sys_nvm_file_close();
			if (u8_ret_value == SYS_NVM_SUCCESS)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] NVM_FILE CLOSE SUCCESS\n");
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] NVM_FILE CLOSE FAILURE\n");
			}	
		}
		else
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] RADIO RESOURCES ARE NOT RELEASED SUCCESSFULLY\n");
		}
	}
	/* SHUTDOWN RESPONSE FAILURE -- 1 */
	else if(e_shutdown_reply_status == REPLYSTATUS_FAILURE)
	{
		e_PF_shutdown_status = e_shutdown_reply_status;
	}
	/* SHUTDOWN RESPONSE TIMEOUT -- 2 */
	else if(e_shutdown_reply_status == REPLYSTATUS_REQ_TIMEOUT)
	{
		e_PF_shutdown_status = e_shutdown_reply_status;
	}
	else
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO SHUTDOWN INVALID REPLY STATUS\n");
	}
	Notify_UpdateShutdownTunerStatus(e_PF_shutdown_status);
}
/*==========================================================================================================================*/
/* void Sys_Request_SRC_Activate_DeActivate (Te_Sys_Mode_Type e_Band, Te_Sys_SRC_Activate_DeActivate e_ActivateDeactivate)  */
/*==========================================================================================================================*/
void Sys_Request_SRC_Activate_DeActivate (Te_Sys_Mode_Type e_Band, Te_Sys_SRC_Activate_DeActivate e_ActivateDeactivate)
{
//	Te_RADIO_ReplyStatus  e_SRC_Act_Deact_Reply;
	Tu8 u8_act_tsk_1 = OS_RESOURCE_CREATE_ERR;
	e_SRC_Act_Deact = e_ActivateDeactivate;
	e_Mode = e_Band;

	/* ACTIVE REQUEST -- 0 */
	if (e_SRC_Act_Deact == SYS_SRC_ACTIVE)
	{
		switch (e_Mode)
		{
			/* MODE AM -- 0 */
		case SYS_MODE_AM:
		{
							/*Do nothing*/
		}
			break;
			/* MODE FM -- 1 */
		case SYS_MODE_FM:
		{
							/*Do nothing*/
		}
			break;
			/* MODE DAB -- 2 */
		case SYS_MODE_DAB:
		{
							 /* ACTIVATING DAB TAKS */
							 u8_act_tsk_1 = OSAL_TaskActivate(st_sys_task_info[1].taskid);
							 if (u8_act_tsk_1 == OS_RESOURCE_CREATE_SUCCESS)
							 {
								 /* REQUESTING TO RM FOR */
								 Radio_Mngr_App_Request_RadioSRCActivateDeActivate((Te_Radio_Mngr_App_Band)e_Mode, (Te_Radio_Mngr_App_SRC_ActivateDeActivate)e_SRC_Act_Deact);
								 SYS_RADIO_MEMCPY(st_sys_task_info[1].task_status, "ACTIVE", sizeof("ACTIVE"));
							 }
							 else
							 {
								 //e_SRC_Act_Deact_Reply = REPLYSTATUS_FAILURE;
								 /* NOTIFYING FAILURE RESPONSE TO HMI_IF */
								// Radio_Response_SRCActivateDeactivate(e_SRC_Act_Deact_Reply);
								 RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] DAB TUNER CTRL TASK ACTIVATION FAILED\n");
							 }
		}
			break;
		default:
		{
				   /*Do nothig*/
		}
			break;
		}
	}
	else if (e_SRC_Act_Deact == SYS_SRC_DEACTIVE)
	{
		Radio_Mngr_App_Request_RadioSRCActivateDeActivate((Te_Radio_Mngr_App_Band)e_Mode, (Te_Radio_Mngr_App_SRC_ActivateDeActivate)e_SRC_Act_Deact);
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] INVALID ACTIVE DEACTIVE REQUEST\n");
	}
}
/*==========================================================================================================================*/
/* void Sys_Response_SRC_Activate_DeActivate (Te_RADIO_ReplyStatus e_ActivateDeactivateReplyStatus)   */
/*==========================================================================================================================*/
void Sys_Response_SRC_Activate_DeActivate (Te_RADIO_ReplyStatus e_ActivateDeactivateReplyStatus)
{
//	Te_RADIO_ReplyStatus  e_SRC_Act_Deact_Reply;
	Tu8 u8_ter_tsk;
	if(e_SRC_Act_Deact == SYS_SRC_ACTIVE)
	{		
//		Radio_Response_SRCActivateDeactivate(e_ActivateDeactivateReplyStatus);
	}
	else if(e_SRC_Act_Deact == SYS_SRC_DEACTIVE)
	{
		if(e_ActivateDeactivateReplyStatus == REPLYSTATUS_SUCCESS)
		{
			switch (e_Mode)
			{
				case SYS_MODE_AM:
				{
					/*Do nothing*/
				}
				break;
				case SYS_MODE_FM:
				{
					/*Do nothnig*/
				}
				break;
				case SYS_MODE_DAB:
				{
					u8_ter_tsk = OSAL_Terminate_Task(st_sys_task_info[1].taskid);
					if(u8_ter_tsk == OS_RESOURCE_RELEASE_SUCCESS)
					{
						SYS_RADIO_MEMCPY(st_sys_task_info[1].task_status,"DE_ACTIVATE",sizeof("DE_ACTIVATE"));
//						Radio_Response_SRCActivateDeactivate(e_ActivateDeactivateReplyStatus);
					}
					else
					{
//						e_SRC_Act_Deact_Reply = REPLYSTATUS_FAILURE;
						//Radio_Response_SRCActivateDeactivate(e_SRC_Act_Deact_Reply);
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] DAB TUNER CTRL TASK ACTIVATION FAILED\n");
					}
				}
				break;
				default:
				{
					/*Do nothig*/
				}
				break;
			}
		}
		else if(e_ActivateDeactivateReplyStatus == REPLYSTATUS_FAILURE)
		{
//			Radio_Response_SRCActivateDeactivate(e_ActivateDeactivateReplyStatus);
		}
		else
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] INVALID RESPONSE FOR ACTIVE DEACTIVE REQUEST\n");
		}		
	}
	else
	{
		/*Do nothnig*/
	}	
}

/*==========================================================================================================================*/
/* void Sys_Factory_Reset_Request(void)																						*/
/*==========================================================================================================================*/
void Sys_Factory_Reset_Request(void)
{
	/* Updating Factory Reset status as INVALID to make differentiate in Startup Response */
	e_Factory_Reset_Status = REPLYSTATUS_INVALID_PARAM;

	/* Request to RM for Factory Reset */
	Radio_Mngr_App_Request_FactoryReset();
}

/*==========================================================================================================================*/
/* void Sys_Factory_Reset_Response(Te_Sys_Factory_Reset_ReplyStatus		e_Factory_Reset_ReplyStatus)   						*/
/*==========================================================================================================================*/
void Sys_Factory_Reset_Response(Te_RADIO_ReplyStatus	e_Factory_Reset_ReplyStatus)
{
	/* Factory Reset Success -- 1 */
	if (e_Factory_Reset_ReplyStatus == REPLYSTATUS_SUCCESS)
	{
		/* Updating Factory Reset to make differentiate in Startup Response */
		e_Factory_Reset_Status = e_Factory_Reset_ReplyStatus;

		/* Startup  Request to RM */
		Radio_Mngr_App_Request_StartTuner((Te_Radio_Mngr_App_Market)e_market_info, u8_radio_component_info);
	}
	/* Factory Reset Failure -- 2 */
	else if (e_Factory_Reset_ReplyStatus == REPLYSTATUS_FAILURE)
	{
		/* Notifying Failure response to HMI_IF*/
		Radio_Response_FactoryReset(e_Factory_Reset_ReplyStatus);
	}
	else
	{
		/* Notifying Invalid response to HMI_IF*/
		Radio_Response_FactoryReset(e_Factory_Reset_ReplyStatus);
	}
}
/*============================================================================================================================
    end of file
=============================================================================================================================*/
