/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_main.h																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains System Layer initialization API declaration.					*
*																											*
*																											*
*************************************************************************************************************/

#ifndef SYS_MAIN_H
#define SYS_MAIN_H

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "IRadio.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/* SRC Active and DeActive */
typedef enum
{        
	SYS_SRC_ACTIVE,                             /*0- Enum, for the SRC Active*/                            
	SYS_SRC_DEACTIVE,                           /*1- Enum, for the SRC DeActive*/                             
	SYS_SRC_INVALID,                            /*2- Enum, for the SRC Invalid*/                                        
}Te_Sys_SRC_Activate_DeActivate;

/* Mode Identification */
typedef enum
{
    SYS_MODE_AM,						/* 0- Specifies the AM Band   */	
    SYS_MODE_FM,						/* 1- Specifies the FM Band   */
    SYS_MODE_DAB,						/* 2- Specifies the DAB Band   */
    SYS_MODE_INVALID					/* 3- Specifies the Invalid Band   */
}Te_Sys_Mode_Type;
/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
   Function declarations (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    function declarations intern
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MAIN_EtalInit function for ETAL Layer initialization.
*   \param[in]				Radio_EtalHardwareAttr attr
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                The main function of SYS invokes functions necessary to initialize ETAL.
*   \post-condition			The system is ready for start up.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void SYS_MAIN_EtalInit(Radio_EtalHardwareAttr attr);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MAIN_EtalDeInit function for ETAL Layer Deinitialization.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                The main function of SYS invokes functions necessary to Deinitialize ETAL.
*   \post-condition			The system is ready for start up.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void SYS_MAIN_EtalDeInit(void);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MAIN_StartOS function for System Layer initialization.
*   \param[in]				Te_Radio_Framework_Variant e_variant, 
*									Te_Radio_Framework_Market e_market, Tu8 u8_radio_resource
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
void SYS_MAIN_StartOS(Te_Radio_Framework_Variant e_variant, Te_Radio_Framework_Market e_market, Tu8 u8_radio_resource);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MAIN_StopOS function for System Layer Deallocation.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           Release OS resources.
*   \post-condition			The system shutdown up. Multitasking can end.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
void SYS_MAIN_StopOS(void);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_ETALHwConfig_Response function will notify the ETAL 
*							initialization status to HMI_IF.
*   \param[in]				Te_RADIO_ReplyStatus e_etal_startup_reply_status
*   \param[out]				None
*   \pre-condition			RTOS layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize ETAL.
*   \post-condition			The System will be ready to start if ETAL Initialization is successful.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_ETALHwConfig_Response(Te_RADIO_ReplyStatus e_etal_startup_reply_status);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_ETALHwDeConfig_Response function will notify the ETAL
*							Deinitialization status to HMI_IF.
*   \param[in]				Te_RADIO_ReplyStatus e_etal_deinit_reply_status
*   \param[out]				None
*   \pre-condition			RTOS layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           deinitialize ETAL.
*   \post-condition			The System will be shutdown
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_ETALHwDeConfig_Response(Te_RADIO_ReplyStatus e_etal_deinit_reply_status);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Startup_Response function will notify the startup status to HMI_IF.
*   \param[in]				Te_RADIO_ReplyStatus e_startup_reply_status
*   \param[out]				None
*   \pre-condition			RTOS layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			The System will be start if startup success.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Startup_Response(Te_RADIO_ReplyStatus e_startup_reply_status);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Shutdown_Response function will notify the shutdown status to HMI_IF.
*   \param[in]				Te_RADIO_ReplyStatus e_shutdown_reply_status
*   \param[out]				None
*   \pre-condition			The system is on condition.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			The system will be shutdown.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Shutdown_Response(Te_RADIO_ReplyStatus e_shutdown_reply_status);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Request_SRC_Activate_DeActivate function provide Active/DeActive request to RM.
*   \param[in]				Te_Sys_Mode_Type e_Band, Te_Sys_SRC_Activate_DeActivate e_ActivateDeactivate
*   \param[out]				None
*   \pre-condition			RTOS layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			SRC will be Active/DeActive.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Request_SRC_Activate_DeActivate (Te_Sys_Mode_Type e_Band, Te_Sys_SRC_Activate_DeActivate e_ActivateDeactivate);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Response_SRC_Activate_DeActivate function Notify Active/DeActive reply status to HMI_IF.
*   \param[in]				Te_RADIO_ReplyStatus e_ActivateDeactivateReplyStatus
*   \param[out]				None
*   \pre-condition			RTOS layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			SRC will be Active/DeActive.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Response_SRC_Activate_DeActivate (Te_RADIO_ReplyStatus e_ActivateDeactivateReplyStatus);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Factory_Reset_Request function give request to RM to do Factory Resetting.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			RTOS layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			Factory Resetting will be done.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Factory_Reset_Request(void);

/*****************************************************************************************************/
/**	 \brief                 The API Function Sys_Factory_Reset_Response function notify reply status of Factory Reset to the HMI_IF.
*   \param[in]				Te_RADIO_ReplyStatus e_Factory_Reset_ReplyStatus
*   \param[out]				None
*   \pre-condition			RTOS layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			Factory Resetting will be done.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Factory_Reset_Response(Te_RADIO_ReplyStatus e_Factory_Reset_ReplyStatus);

#endif /* SYS_MAIN_H */
/*=============================================================================
    end of file
=============================================================================*/
