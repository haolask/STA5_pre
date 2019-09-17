/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app.c																						  	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains function definition for handling DAB main and instance HSM's.	*
*																											*
*																											*
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_hsm.h"
#include "dab_app_request.h"
#include "dab_app_stationlist.h"
#include "dab_app_freq_band.h"


/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/
Ts_dab_app_hsm st_me_dab_app;		/* Variable for main hsm object */

extern Ts_dab_app_frequency_table dab_app_freq_band_eu[] ;

/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void DAB_APP_MSG_HandleMsg                                        */
/*===========================================================================*/
void DAB_APP_MSG_HandleMsg(Ts_Sys_Msg* pst_msg)
{
	if(pst_msg != NULL)		/* Check if the msg pointer is NULL */
	{
		HSM_ON_MSG((Ts_hsm *)&st_me_dab_app, pst_msg);
	}
	else
	{
		/* Report error */
	}
}

/*===========================================================================*/
/*  void DAB_APP_INST_HSM_HandleMessage                                */
/*===========================================================================*/
void DAB_APP_INST_HSM_HandleMessage(Ts_Sys_Msg *pst_msg )
{
	if(pst_msg != NULL)		/* Check if the msg pointer is NULL */
	{
		HSM_ON_MSG((Ts_hsm *)&st_me_dab_app.instance, pst_msg);
	}
	else
	{
		/* Report error*/
	} 
}

/*===========================================================================*/
/*  void DAB_App_Component_Init()                               */
/*===========================================================================*/
void DAB_App_Component_Init()
{
   DAB_APP_Init(&st_me_dab_app);
}

#ifdef PC_TEST
int main()
{
	DAB_App_Component_Init();
	DAB_App_Request_Startup(DAB_APP_MARKET_WESTERN_EUROPE);
	DAB_App_Request_GetStationList();
	DAB_App_Request_SelectBand();
	DAB_App_Request_DeSelectBand();
	DAB_App_Request_PlaySelectSt(4);
	DAB_App_Request_Shutdown();
	return 0;
}
#endif

void DAB_App_UpdateChannelName(Tu32 u32_Frequency, Tu8 *pu8_ChannelName)
{
	Tu8 u8_ChannelNameIndex ;
	
	for(u8_ChannelNameIndex = (Tu8)0 ; u8_ChannelNameIndex <  DAB_APP_EU_MAX_FREQUENCIES ; u8_ChannelNameIndex++)
	{
		if(dab_app_freq_band_eu[u8_ChannelNameIndex].u32_frequency == u32_Frequency)
		{
			SYS_RADIO_MEMCPY(pu8_ChannelName,dab_app_freq_band_eu[u8_ChannelNameIndex].au8_ChannelName,DAB_APP_MAX_CHANNEL_NAME_SIZE);
			break ;
		}
		else
		{
			
		}/* For MISRA C */
	}
}


void DAB_App_Updatefrequency(Tu8 *au8_ChannelName,Tu32 *u32_Frequency)
{
	Tu8 u8_ChannelNameIndex = 0;
	
	for(u8_ChannelNameIndex = (Tu8)0 ; u8_ChannelNameIndex <  DAB_APP_EU_MAX_FREQUENCIES ; u8_ChannelNameIndex++)
	{
		if(DAB_App_String_comparison((dab_app_freq_band_eu[u8_ChannelNameIndex].au8_ChannelName),
			(au8_ChannelName),DAB_APP_MAX_CHANNEL_NAME_SIZE) == (Tu8)0)
		{
			*u32_Frequency = dab_app_freq_band_eu[u8_ChannelNameIndex].u32_frequency;
			break;
		}
		else
		{
			
		}
	}
}

/*=============================================================================
    end of file
=============================================================================*/