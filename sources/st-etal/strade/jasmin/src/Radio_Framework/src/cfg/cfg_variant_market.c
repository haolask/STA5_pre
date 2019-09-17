/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file cfg_variant.c																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains rtos related API definitions									*
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "cfg_variant_market.h"
#include "hmi_if_app_notify.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (Global)
-----------------------------------------------------------------------------*/
Te_DAB_Status e_dab_status  = INVALID_OPTION; 
Tu8           u8_rds_status = RDS_NOT_AVAILABLE; 
/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/


/*===========================================================================*/
/* 	Ts_variant_info Sys_get_variant_details(Tu8 variant)                     */
/*===========================================================================*/
Tu8 Sys_get_variant_details(Te_Radio_Framework_Variant e_variant, Te_Radio_Framework_Market e_market, Tu8 u8_radio_resource)
{
	Te_RADIO_ReplyStatus e_startup_reply;
	Tu8	u8_update_status = UPDATE_STATUS_SUCCESS;
   if((e_variant == VARIANT_A1) ||(e_variant == VARIANT_B1)||(e_variant == VARIANT_C1)||(e_variant == VARIANT_C2))
   {
       if(((u8_radio_resource>>BIT_2) & 1) == 1)
       {
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] DAB IS UNSUPPORTED FOR VARIANT\n");
			e_startup_reply = REPLYSTATUS_FAILURE;
			u8_update_status = UPDATE_STATUS_FAILURE;
           
			Notify_UpdateStartRadioStatus(e_startup_reply);
       }
       else
       {
           e_dab_status = DAB_NOT_AVAILABLE;
       }
   }
   else if((e_variant == VARIANT_A2) ||(e_variant == VARIANT_B2))
   {
       if(((u8_radio_resource>>BIT_2) & 1) == 1)
       {
           e_dab_status = DAB_AVAILABLE;         
       }
       else
       {
           e_dab_status = DAB_SLEEP_STATE;
       }
   }
   else
   {
       RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] INVALID VARIANT\n");
   }

   switch(e_market)
   {
        case WESTERN_EUROPE_MARKET:  																	
        case LATIN_AMERICA_MARKET:																					
        case ASIA_CHINA_MARKET:																								
        case ARABIA_AFRICA_MARKET:																				
        case USA_NORTHAMERICA_MARKET:							  																																		
        case KOREA_MARKET:								
        case BRAZIL_MARKET:									
        case SOUTHAMERICA_EXTENDED_MARKET:					
        case USA_NORTHAMERICA_EXTENDED_MARKET:
        {
			u8_rds_status = RDS_AVAILABLE;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][FW] RDS IS SUPPORTED FOR GIVEN MARKET\n");
        }
        break;
        case JAPAN_MARKET:
        {
            u8_rds_status = RDS_NOT_AVAILABLE;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][FW] RDS IS NOT SUPPORTED FOR GIVEN MARKET\n");
        }
        break;
        default:
        {
            RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] INVALID MARKET\n");
        }
        break;
   }
   return u8_update_status;
}
/*===========================================================================*/
/*  void Sys_HSM_intialization()                                             */
/*===========================================================================*/
void Sys_HSM_intialization(void)
{	
	Radio_Mngr_App_Component_Init();

	AMFM_App_Component_Init();			

	AMFM_Tuner_Ctrl_Component_Init();
	
	if((e_dab_status == DAB_AVAILABLE)||(e_dab_status == DAB_SLEEP_STATE))
	{
		DAB_App_Component_Init();

		DAB_Tuner_Ctrl_Component_Init();
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][FW] DAB APP AND DAB TUNER CTRL HSM INTIALIZED\n");
	}
	else
	{
		/*Do Nothing*/
	}
	if(u8_rds_status == RDS_AVAILABLE)
	{
		/*Do nothing*/
	}
	else
	{
		/*Do nothing*/
	}		
}
/*=============================================================================
    end of file
=============================================================================*/


