/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_variant.h																				    	*
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

#ifndef CFG_VARIANT_H
#define CFG_VARIANT_H

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "cfg_types.h"
#include "lib_string.h"
#include "sys_task.h"
#include "osal_private.h"
/*-----------------------------------------------------------------------------
    defines
-------------------------------------------------------------------------------*/
#define BIT_2               	2
#define RDS_AVAILABLE       	(Tu8)0x00
#define RDS_NOT_AVAILABLE   	(Tu8)0x01
#define UPDATE_STATUS_SUCCESS	(Tu8)0x00
#define UPDATE_STATUS_FAILURE	(Tu8)0x01
/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
/* @brief this enum discribes about variant types */
typedef enum
{
	VARIANT_A1,											/*Variant A1*/										                         														
    VARIANT_A2,											/*Variant A2*/                													
    VARIANT_B1,                           				/*Variant B1*/									
    VARIANT_B2,											/*Variant B2*/
	VARIANT_C1,											/*Variant C1*/
	VARIANT_C2,											/*Variant C2*/
	VARIANT_INVALID,									/*Variant INVALID*/	                           													
}Te_Radio_Framework_Variant;

/* @brief this enum describes different markets */
typedef enum
{
    WESTERN_EUROPE_MARKET,  						/* Market Wester Europe*/													
    LATIN_AMERICA_MARKET,							/* Market Latin America*/     														
    ASIA_CHINA_MARKET,								/* Market Asia China*/  																
    ARABIA_AFRICA_MARKET,							/* Market Arabia*/ 																
    USA_NORTHAMERICA_MARKET,						/* Market USA NorthAmerica*/         													
	JAPAN_MARKET,									/* Market Japan*/ 														
	KOREA_MARKET,									/* Market Korea*/
	BRAZIL_MARKET,									/* Market Brazil*/
	SOUTHAMERICA_EXTENDED_MARKET,					/* Market South america extended*/
	USA_NORTHAMERICA_EXTENDED_MARKET,				/* Market USA north america extended*/
	INVALID_MARKET,									/* Market Invalid*/	            														
}Te_Radio_Framework_Market;

/* DAB availability status*/
typedef enum
{    
    DAB_AVAILABLE,
    DAB_SLEEP_STATE,
	DAB_NOT_AVAILABLE,
    INVALID_OPTION
}Te_DAB_Status;
/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
   Function declarations (extern)
-----------------------------------------------------------------------------*/
extern void Radio_Mngr_App_Component_Init(void);
extern void AMFM_App_Component_Init(void);
extern void DAB_App_Component_Init(void);
extern void DAB_Tuner_Ctrl_Component_Init(void);
extern void AMFM_Tuner_Ctrl_Component_Init(void);

/*-----------------------------------------------------------------------------
    function declarations intern
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief                 The API Function gives DAB, RDS availablility status
*   \param[in]				Te_Radio_Framework_Variant e_variant, Te_Radio_Framework_Market e_market, Tu8 u8_radio_resource
*   \param[out]				void
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
Tu8 Sys_get_variant_details(Te_Radio_Framework_Variant e_variant, Te_Radio_Framework_Market e_market, Tu8 u8_radio_resource);

/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_HSM_intialization function for HSM Init for application components.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                The main function of SYS invokes all functions necessary to
*                           initialize and configure the System Layer.
*                           Additionally all defined tasks and packages with their services and observers
*                           are created and started. Finally, the package initialization functions are
*                           called.
*   \post-condition			HSM intialized for application components.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
void Sys_HSM_intialization(void);

#endif /* CFG_VARIANT_H */
/*=============================================================================
    end of file
=============================================================================*/

