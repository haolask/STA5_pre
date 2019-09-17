/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_application.c																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This source file contains function definitions for all internal funtions of AMFM  *
*						  Application component																*	
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
   File Inclusions
-----------------------------------------------------------------------------*/
#include "amfm_app_hsm.h"
#include "amfm_app_application.h"


/*-----------------------------------------------------------------------------
	Global variable Definitions (static)	                                        
-----------------------------------------------------------------------------*/
/* Holds information about HSM amfm_app_hsm */
Ts_AMFM_App_hsm st_hsm_info;

/*-----------------------------------------------------------------------------
    extern variable Declaration
-----------------------------------------------------------------------------*/
/* */

extern Tu32     u32_start_freq;
extern Tu32		u32_End_Freq;
extern Tu32     u32_StepSize;


Tu8 u8_alpha_div_factor	= AMFM_APP_ALPHA_DIV_FACTOR; 
Tu8 u8_qual_incr_alpha  	= AMFM_APP_CURR_STATION_CURR_QUAL_INCREASED_ALPHA; 
Tu8 u8_qual_decr_alpha 		= AMFM_APP_CURR_STATION_CURR_QUAL_DECREASED_ALPHA; 
Tu8 u8_Delta_Difference 	= AMFM_APP_DELTA_DIFFERENCE;
Tu8 u8_Low_Delta_Difference  = AMFM_APP_LOW_QUALITY_DELTA_DIFFERENCE;
Tu8 u8_MID_Delta_Difference  = AMFM_APP_MID_QUALITY_DELTA_DIFFERENCE;
Tu8 u8_HIGH_Delta_Difference = AMFM_APP_HIGH_QUALITY_DELTA_DIFFERENCE;
Tu8 u8_Low_Delta_Threshold   = AMFM_APP_LOW_QUALITY_DELTA_THRESHOLD;
Tu8 u8_MID_Delta_Threshold   = AMFM_APP_MID_QUALITY_DELTA_THRESHOLD;
Tu8 u8_HIGH_Delta_Threshold  = AMFM_APP_HIGH_QUALITY_DELTA_THRESHOLD;

#ifdef CALIBRATION_TOOL
extern Tu32 u32_cal_curr;
extern Tu32 u32_cal_avg;
extern Tu32 u32_cal_old;
extern Ts_AMFM_App_AFList_Info st_Calib_AMFM_App_AFList_Info;
#endif

/*-----------------------------------------------------------------------------
   Public Function Definition
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*			void AMFM_APP_HSM_MessageHandler								 */
/*===========================================================================*/
void AMFM_APP_HSM_MessageHandler(Ts_Sys_Msg *pst_msg)
{	
	if(pst_msg != NULL)
	{
		HSM_ON_MSG(&st_hsm_info,pst_msg);
	}
}

/*===========================================================================*/
/*			void AMFM_App_Component_Init									 */
/*===========================================================================*/
void AMFM_App_Component_Init(void )
{
	AMFM_APP_HSM_Init(&st_hsm_info);

}

/*===========================================================================*/
/*			void AMFM_APP_SetMarketFrequency								 */
/*===========================================================================*/
void AMFM_APP_SetMarketFrequency(Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo)
{
		switch(e_MarketType)
		{
			case AMFM_APP_MARKET_WESTERN_EUROPE :	
			{
				/* Updating Frequency info for AM Band Medium wave  */
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_EU_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_EU_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_EU_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for AM Band Long wave  */
				pst_MarketInfo->st_AMbandLW_FreqInfo.u32_StartFreq		=	 AMFM_APP_EU_MARKET_AM_LW_STARTFREQ;
				pst_MarketInfo->st_AMbandLW_FreqInfo.u32_EndFreq		=	 AMFM_APP_EU_MARKET_AM_LW_ENDFREQ;
				pst_MarketInfo->st_AMbandLW_FreqInfo.u32_StepSize		=	 AMFM_APP_EU_MARKET_AM_LW_STEPSIZE;


				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_EU_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_EU_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_EU_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_LATIN_AMERICA:	
			{
				/* Updating Frequency info for AM Band Medium wave  */
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_LAR_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_LAR_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_LAR_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_LAR_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_LAR_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_LAR_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_ASIA_CHINA:	
			{
				/* Updating Frequency info for AM Band Medium wave  */		 
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_ASIA_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_ASIA_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_ASIA_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_ASIA_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_ASIA_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_ASIA_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_ARABIA:	
			{
				/* Updating Frequency info for AM Band Medium wave  */		 
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_ARAB_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_ARAB_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_ARAB_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_ARAB_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_ARAB_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_ARAB_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_USA_NORTHAMERICA:	
			{
				/* Updating Frequency info for AM Band Medium wave  */		 
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_NAR_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_NAR_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_NAR_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_NAR_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_NAR_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_NAR_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_JAPAN:	
			{
				/* Updating Frequency info for AM Band Medium wave  */		 
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_JPN_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_JPN_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_JPN_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_JPN_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_JPN_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_JPN_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_KOREA:	
			{
				/* Updating Frequency info for AM Band Medium wave  */		 
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_KOREA_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_KOREA_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_KOREA_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_KOREA_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_KOREA_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_KOREA_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_BRAZIL:	
			{
				/* Updating Frequency info for AM Band Medium wave  */		 
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_BRAZIL_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_BRAZIL_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_BRAZIL_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_BRAZIL_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_BRAZIL_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_BRAZIL_MARKET_FM_STEPSIZE;
			}
			break;
			
			case AMFM_APP_MARKET_SOUTH_AMERICA:	
			{
				/* Updating Frequency info for AM Band Medium wave  */		 
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq		=	 AMFM_APP_SOUTH_AMERICA_MARKET_AM_MW_STARTFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq		=	 AMFM_APP_SOUTH_AMERICA_MARKET_AM_MW_ENDFREQ;
				pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize		=	 AMFM_APP_SOUTH_AMERICA_MARKET_AM_MW_STEPSIZE;

				/* Updating Frequency info for FM Band */
				pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq		=	 AMFM_APP_SOUTH_AMERICA_MARKET_FM_STARTFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq			=	 AMFM_APP_SOUTH_AMERICA_MARKET_FM_ENDFREQ;
				pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize			=	 AMFM_APP_SOUTH_AMERICA_MARKET_FM_STEPSIZE;
			}
			break;
			
			default :
			{
			}
			break;
		}

}


/*===========================================================================*/
/*			Tbool  AMFM_APP_VerifyFrequency								 */
/*===========================================================================*/
Tbool AMFM_APP_VerifyFrequency(Tu32 u32_Frequency,Te_AMFM_App_mode *pe_mode,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo)
{
	Tbool  b_ret_bool	= FALSE;        /* variable to hold the result */

	if(pst_MarketInfo != NULL)
	{
		if( (*pe_mode == AMFM_APP_MODE_AM) || (*pe_mode == AMFM_APP_MODE_AM_MW) || (*pe_mode == AMFM_APP_MODE_AM_LW))
		{  
			if((u32_Frequency >= pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq) && (u32_Frequency <= pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq) )
			{ 
				b_ret_bool	= TRUE ;
				*pe_mode	= AMFM_APP_MODE_AM_MW;			
			}
			else if(e_MarketType == AMFM_APP_MARKET_WESTERN_EUROPE) 
			{
				if((u32_Frequency >= pst_MarketInfo->st_AMbandLW_FreqInfo.u32_StartFreq) && (u32_Frequency <= pst_MarketInfo->st_AMbandLW_FreqInfo.u32_EndFreq))
				{
					 b_ret_bool = TRUE ;
					*pe_mode	= AMFM_APP_MODE_AM_LW;
				}
			}
			else 
			{
				b_ret_bool	= FALSE; 	
			}
		}
		else if(*pe_mode == AMFM_APP_MODE_FM )
		{
			if((u32_Frequency >= pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq) && (u32_Frequency <= pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq) )
			{
				b_ret_bool = TRUE ;
			}
			else
			{
				b_ret_bool = FALSE ;
			}
		}
		else 
		{
			
		}
	}
	else
	{
		// sending error message 	
		b_ret_bool	= FALSE; 
	}
	return b_ret_bool;	
}

/*===========================================================================*/
/*		Ts_AMFM_App_FreqInfo	AMFM_APP_GetCurrentModeFrequencyInfo	 	 */
/*===========================================================================*/
Ts_AMFM_App_FreqInfo	AMFM_APP_GetCurrentModeFrequencyInfo(Te_AMFM_App_mode e_CurrentMode,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo)
{
	Ts_AMFM_App_FreqInfo	st_Ret_FreqInfo = {0,0,0};		/* Structure holds Frequency info of Current mode as per the Market */
	
	if(e_CurrentMode == AMFM_APP_MODE_FM)
	{
		st_Ret_FreqInfo = pst_MarketInfo->st_FMband_FreqInfo;
	}
	else if(e_CurrentMode == AMFM_APP_MODE_AM_MW || e_CurrentMode == AMFM_APP_MODE_AM_LW)
	{
		if(e_MarketType != AMFM_APP_MARKET_WESTERN_EUROPE)
		{
			st_Ret_FreqInfo = pst_MarketInfo->st_AMbandMW_FreqInfo;
		}
		else
		{
			/* This block is for EUROPE market and AM Band */

			/* for ST we are considering only AM MW band, so making the start freq as MW start freq*/
			//st_Ret_FreqInfo.u32_StartFreq = pst_MarketInfo->st_AMbandLW_FreqInfo.u32_StartFreq;
			st_Ret_FreqInfo.u32_StartFreq = pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StartFreq;
			st_Ret_FreqInfo.u32_EndFreq   = pst_MarketInfo->st_AMbandMW_FreqInfo.u32_EndFreq;
			if(e_CurrentMode == AMFM_APP_MODE_AM_MW )
			{
				st_Ret_FreqInfo.u32_StepSize  = pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize;
			}
			else
			{
				st_Ret_FreqInfo.u32_StepSize  = pst_MarketInfo->st_AMbandLW_FreqInfo.u32_StepSize;
			}			
		}		
	}
	else
	{
		/* Returning Zero as return value if Current mode is invalid */
	}	
	return st_Ret_FreqInfo;
}

/*===========================================================================*/
/*			Tu32  AMFM_APP_GetNextTuneUpFrequency							 */
/*===========================================================================*/
Tu32 AMFM_APP_GetNextTuneUpFrequency(Tu32 u32_CurrentFrequency,Te_AMFM_App_mode *pe_CurrentMode,Tu32 u32_No_of_Steps,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo)
{
	Tu32					u32_index = 0;
	Ts_AMFM_App_FreqInfo	st_FreqInfo;			/* Structure holds Frequency info of Current mode as per the Market */
	
	/* Updating Frequency info of current mode into st_FreqInfo structure */
	st_FreqInfo = AMFM_APP_GetCurrentModeFrequencyInfo(*pe_CurrentMode,e_MarketType,pst_MarketInfo);
	
	for(u32_index = 0;u32_index < u32_No_of_Steps;u32_index++)
	{
		/* Adding step size value to currently tuned frequency in order to get next tune up frequency */	
		u32_CurrentFrequency = u32_CurrentFrequency + st_FreqInfo.u32_StepSize;
	
		if((*pe_CurrentMode == AMFM_APP_MODE_FM) || (e_MarketType != AMFM_APP_MARKET_WESTERN_EUROPE ))
		{
			if(u32_CurrentFrequency > st_FreqInfo.u32_EndFreq)
			{
				u32_CurrentFrequency = st_FreqInfo.u32_StartFreq;
			}
			else
			{
				/* Nothing to do.Just for MISRA C*/
			}
				
		}

		else if (*pe_CurrentMode == AMFM_APP_MODE_AM_MW)
		{
			if (u32_CurrentFrequency > st_FreqInfo.u32_EndFreq)
			{
				u32_CurrentFrequency = AMFM_APP_EU_MARKET_AM_MW_STARTFREQ;
			}
			else
			{
				/* Nothing to do.Just for MISRA C*/
			}
		}
		/* commented the below else if case and newly added the above else if for AM MW for ST*/
			//else if(*pe_CurrentMode == AMFM_APP_MODE_AM_MW || *pe_CurrentMode == AMFM_APP_MODE_AM_LW ) /* This block is for Europe market only and Current mode should be is AM */
			//{		
			//	if(u32_CurrentFrequency > st_FreqInfo.u32_EndFreq)
			//	{
			//		u32_CurrentFrequency		= st_FreqInfo.u32_StartFreq;
			//		st_FreqInfo.u32_StepSize	= pst_MarketInfo->st_AMbandLW_FreqInfo.u32_StepSize;
			//		*pe_CurrentMode				= AMFM_APP_MODE_AM_LW;					  	
			//	
			//	}
			//	else if( (u32_CurrentFrequency > AMFM_APP_EU_MARKET_AM_LW_ENDFREQ) && (u32_CurrentFrequency  < AMFM_APP_EU_MARKET_AM_MW_STARTFREQ))
			//	{
			//		u32_CurrentFrequency		=	AMFM_APP_EU_MARKET_AM_MW_STARTFREQ;
			//		st_FreqInfo.u32_StepSize	=	pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize;
			//		*pe_CurrentMode				= 	AMFM_APP_MODE_AM_MW;
			//	}
			//	else
			//	{
			//		/* Nothing to do.Just for MISRA C*/
			//	}				
			//}	
		else
		{
			/* Returning Zero as return value if Current mode is invalid */
			u32_CurrentFrequency = 0;
		}
	}	
	return u32_CurrentFrequency;	
}	

/*===========================================================================*/
/*			Tu32  AMFM_APP_GetNextTuneDownFrequency							 */
/*===========================================================================*/
Tu32 AMFM_APP_GetNextTuneDownFrequency(Tu32 u32_CurrentFrequency,Te_AMFM_App_mode *pe_CurrentMode,Tu32 u32_No_of_Steps,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo)
{
	Tu32					u32_index = 0;
	Ts_AMFM_App_FreqInfo	st_FreqInfo;			/* Structure holds Frequency info of Current mode as per the Market */
	
	
	st_FreqInfo = AMFM_APP_GetCurrentModeFrequencyInfo(*pe_CurrentMode,e_MarketType,pst_MarketInfo);
		
	for(u32_index = 0;u32_index < u32_No_of_Steps;u32_index++)
	{
		/* Subtracting step size value from currently tuned frequency in order to get next tune down frequency*/	
		u32_CurrentFrequency	= u32_CurrentFrequency - st_FreqInfo.u32_StepSize;
		
		if((*pe_CurrentMode == AMFM_APP_MODE_FM) || (e_MarketType != AMFM_APP_MARKET_WESTERN_EUROPE ))
		{
				if(u32_CurrentFrequency < st_FreqInfo.u32_StartFreq)
				{
					u32_CurrentFrequency = st_FreqInfo.u32_EndFreq;
				}
				else
				{
					/* Nothing to do.Just for MISRA C*/
				}		
		}
		else if (*pe_CurrentMode == AMFM_APP_MODE_AM_MW)
		{
			if (u32_CurrentFrequency < st_FreqInfo.u32_StartFreq)
			{
				u32_CurrentFrequency = AMFM_APP_EU_MARKET_AM_MW_ENDFREQ;
			}
			else
			{
				/* Nothing to do.Just for MISRA C*/
			}
		}
		/* commented the below else if case and newly added the above else if for AM MW for ST*/
			//else if(*pe_CurrentMode == AMFM_APP_MODE_AM_MW || *pe_CurrentMode == AMFM_APP_MODE_AM_LW ) /* This block is for Europe market only and Current mode should be is AM */
			//{			
			//	if(u32_CurrentFrequency < st_FreqInfo.u32_StartFreq)
			//	{
			//		u32_CurrentFrequency 		= st_FreqInfo.u32_EndFreq;
			//		st_FreqInfo.u32_StepSize	= pst_MarketInfo->st_AMbandMW_FreqInfo.u32_StepSize;
			//		*pe_CurrentMode				= AMFM_APP_MODE_AM_MW;
			//	}
			//	else if( (u32_CurrentFrequency < AMFM_APP_EU_MARKET_AM_MW_STARTFREQ) && (u32_CurrentFrequency > AMFM_APP_EU_MARKET_AM_LW_ENDFREQ) )
			//	{
			//		u32_CurrentFrequency		= AMFM_APP_EU_MARKET_AM_LW_ENDFREQ;
			//		st_FreqInfo.u32_StepSize	= pst_MarketInfo->st_AMbandLW_FreqInfo.u32_StepSize;
			//		*pe_CurrentMode				= AMFM_APP_MODE_AM_LW;
			//	}
			//	else
			//	{
			//		/* Nothing to do.Just for MISRA C */
			//	}	
			//}	
		else
		{
			/* Returning Zero as return value if Current mode is invalid */
			u32_CurrentFrequency = 0;
		}
	}	
	return u32_CurrentFrequency;	
}

/*===========================================================================*/
/*			void AMFM_App_application_SortAMStationList						 */
/*===========================================================================*/
void AMFM_App_application_SortAMStationList(Ts_AMFM_App_AMStationInfo *pst_am_station_list,Tu8 u8_STL_Size)
{
	Tu32	u32_OuterIndex;             /* variable to control the outer loop*/
	Tu32	u32_InnerIndex;             /* variable to control the inner loop*/
	Tu32  u32_tempfreq;                 /* variable to hold the frequency Temporarily during sorting */
	
	
	for(u32_OuterIndex=0;u32_OuterIndex < (Tu32)(u8_STL_Size-(Tu8)1) ;u32_OuterIndex++)
	{
		for(u32_InnerIndex=0;u32_InnerIndex < (Tu32)(u8_STL_Size-(Tu8)u32_OuterIndex-(Tu8)1);u32_InnerIndex++)
		{
			if(((pst_am_station_list+u32_InnerIndex)->u32_Freq)>((pst_am_station_list+u32_InnerIndex+1)->u32_Freq))
			{
				u32_tempfreq=(pst_am_station_list+u32_InnerIndex)->u32_Freq;
				(pst_am_station_list+u32_InnerIndex)->u32_Freq=(pst_am_station_list+u32_InnerIndex+1)->u32_Freq;
				(pst_am_station_list+u32_InnerIndex+1)->u32_Freq=u32_tempfreq;

			}
		}
	}
}

/*===========================================================================*/
/*			void AMFM_App_application_GenerateAMStationList					 */
/*===========================================================================*/
void AMFM_App_application_GenerateAMStationList(Ts_AMFM_App_AM_STL *pst_am_station_list)
{
	Tu8										u8_index;                           /*variable to indicate the station list index*/

	/*Locking the MUTEX */
	RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AMFM APP Mutex Lock of both RM_APP and APP_TC at AMFM_App_application_GenerateAMStationList ");
	
	SYS_MUTEX_LOCK(STL_AMFM_APP_AMFM_TC);	/* Locking the mutex between TC and APP*/
	SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/
	
	/*clearing AM station List */
	memset(pst_am_station_list,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AM_STL));
	
	/* copies station list information from shared memory to  Ts_AMFM_App_AM_STL structure*/
	for(u8_index=0; (u8_index < scan_index_AM)  && (u8_index < AMFM_APP_MAX_AM_STL_SIZE);u8_index++)
	{
		pst_am_station_list->ast_Stations[u8_index].u32_Freq = (Tu32)(ast_Scaninfo[u8_index].u32_freq);	
		pst_am_station_list -> u8_NumberOfStationsInList++;
	}
	/*UnLocking the MUTEX between TC and APP*/
	SYS_MUTEX_UNLOCK(STL_AMFM_APP_AMFM_TC);

	RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AMFM APP Mutex UnLock of APP_TC at AMFM_App_application_GenerateAMStationList ");
	
	if(pst_am_station_list -> u8_NumberOfStationsInList != (Tu8) 0)
	{
		AMFM_App_application_SortAMStationList((Ts_AMFM_App_AMStationInfo *)(pst_am_station_list->ast_Stations),pst_am_station_list->u8_NumberOfStationsInList);
	}
	/* UNLocking the mutex between RM and APP*/
	SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);	
	RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AMFM APP Mutex UnLock of RM_APP at AMFM_App_application_GenerateAMStationList ");
}
/*************************************************************************************
Ts32 AMFM_App_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size)
**************************************************************************************/

Ts32 AMFM_App_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size)
{
    Tu8 count=0;
    while(count < size -1)
    {
        if(src[count]!=dst[count])
        {
            return (Ts32)(src[count]-dst[count]);
        }

        count++;
    }
    return (Ts32)(src[count]-dst[count]);
}
/*************************************************************************************
Tu8	AMFM_APP_String_length(Tu8 *pu8_PtrTostring,Tu8	u8_StringLen)
**************************************************************************************/
Tu8	AMFM_APP_String_length(Tu8 *pu8_PtrTostring,Tu8	u8_StringLen)
{
	Tu8	u8_count = 0;
	
	for(u8_count = 0; u8_count < u8_StringLen;u8_count++)
	{
		if((pu8_PtrTostring[u8_count] == '\0'))
		{
			break;
		}
	}
	return u8_count;
	
}
/*===========================================================================*/
/*			Tu8		AMFM_APP_Total_Non_RDS_Stations							 */
/*===========================================================================*/
Tu8	AMFM_APP_Total_Non_RDS_Stations(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_STL_Size)
{
	Tu8		u8_index				=0;
	Tu8		u8_Ret_Non_RDS_Count	=0;
	Tu8		u8_string_len			=0;	
	
	for(u8_index = 0;u8_index < u8_STL_Size ;u8_index++)
	{
		u8_string_len = AMFM_APP_String_length(pst_fm_station_list[u8_index].au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME); 
		if(u8_string_len == 0)
		{
			u8_Ret_Non_RDS_Count++;
		}
		else
		{
			/* Nothing to do .Just for MISRA C*/
		}
	}
	
	return u8_Ret_Non_RDS_Count;
}
/*===========================================================================*/
/*			void AMFM_App_SortingAscending									 */
/*===========================================================================*/
void AMFM_App_SortingAscending(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_LastRDSindex)
{
	//Tu8								u8_Lastindex  = AMFM_APP_MAX_FM_STL_SIZE - u8_No_of_Station -1 ;    /*variable to hold the last index of the station list array*/
	Tu8								u8_Firstindex = 0;                             /*variable to hold the first index of the station list array*/
	Ts_AMFM_App_FMStationInfo		st_tmpStationInfo;                                                  /*structure variable to hold the station info structure temporarily during sorting */

	while(u8_Firstindex  < u8_LastRDSindex )
	{
		st_tmpStationInfo						= pst_fm_station_list[u8_Firstindex];
		pst_fm_station_list[u8_Firstindex]		= pst_fm_station_list[u8_LastRDSindex];
		pst_fm_station_list[u8_LastRDSindex]	= st_tmpStationInfo;
		u8_Firstindex++;
		u8_LastRDSindex--;
	}
}


/*===========================================================================*/
/*			Tu8  AMFM_App_application_SortFMStationList						 */
/*===========================================================================*/
Tu8 	AMFM_App_application_SortFMStationList(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_STL_Size)
{
	Tu32								u32_OuterIndex;                                       /* variable to control the outer loop*/
	Tu32								u32_InnerIndex;                                       /* variable to control the inner loop*/
	Ts32							    s32_ReturnValue;                                       /*variable to hold the result returned by strcmp function*/
	Tbool								b_SortRequired;                                       /*variable used as a flag to check list is sorted or not */
	Ts_AMFM_App_FMStationInfo			st_tmpStationInfo;                                    /*structure variable to hold the station info structure temporarily during sorting */
	Tu8								    u8_Total_NonRDS_Station = 0;     /*variable to calculate total number of non rds stations */

	for(u32_OuterIndex=0;u32_OuterIndex < (Tu32)(u8_STL_Size-(Tu8)1);u32_OuterIndex++)
	{
		b_SortRequired = FALSE;
		for(u32_InnerIndex=0;u32_InnerIndex < ((Tu32)(u8_STL_Size- (Tu8)1)-u32_OuterIndex);u32_InnerIndex++)
		{

			s32_ReturnValue = AMFM_App_String_comparison(pst_fm_station_list[u32_InnerIndex].au8_psn,pst_fm_station_list[u32_InnerIndex+1].au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME); 
			/*To sort in Descending Order */
			if(s32_ReturnValue <(Ts8) AMFM_APP_CONSTANT_ZERO)
			{	
				b_SortRequired								= TRUE ;
				st_tmpStationInfo							= pst_fm_station_list[u32_InnerIndex];
				pst_fm_station_list[u32_InnerIndex]			= pst_fm_station_list[u32_InnerIndex+1];
				pst_fm_station_list[u32_InnerIndex+1]		= st_tmpStationInfo;	
			}
			/*For NON RDS stations or two stations with same PSN, strcmp will return 0 */
			else if (s32_ReturnValue == (Ts8)AMFM_APP_CONSTANT_ZERO)
			{
				/*For NON RDS stations  or two stations with same PSN,Sorting should be based on frequency */
				if(pst_fm_station_list[u32_InnerIndex].u32_frequency > pst_fm_station_list[u32_InnerIndex+1].u32_frequency)
				{
					b_SortRequired = TRUE ;
					st_tmpStationInfo							= pst_fm_station_list[u32_InnerIndex];
					pst_fm_station_list[u32_InnerIndex]			= pst_fm_station_list[u32_InnerIndex+1];
					pst_fm_station_list[u32_InnerIndex+1]		= st_tmpStationInfo;	
				}
			}
			else
			{
				/* No need to swap */
			}
		}
		/* Checking whether sort is required */
		if(b_SortRequired == FALSE)
		{
				u32_OuterIndex = u8_STL_Size ; // List is sorted. Condtion to break outer loop 
		}
	}
	
	/* Function to find number of Non-RDS stations present in the list */
	u8_Total_NonRDS_Station = AMFM_APP_Total_Non_RDS_Stations(pst_fm_station_list,u8_STL_Size);
	
	/*	list is sorted but all rds stations is sorted in descending order ,it needs to be  sorted in ascending by  
		AMFM_App_SortingAscending function */
	if(u8_Total_NonRDS_Station < u8_STL_Size )
	{
		AMFM_App_SortingAscending(pst_fm_station_list,(Tu8)(u8_STL_Size-u8_Total_NonRDS_Station-1));
	}

	return u8_Total_NonRDS_Station;
}

/*===========================================================================*/
/*			void AMFM_App_Remove_Non_RDS_Stations						 */
/*===========================================================================*/
void AMFM_App_Remove_Non_RDS_Stations(Ts_AMFM_App_FM_STL *pst_fm_station_list,Tu8 u8_Total_NonRDS_Station)
{
	
	Tu8						u8_loopindex					= 	0;	
	Tu8						u8_innerloop_index				= 	0;	
	Tu8						u8_STL_Size						=  	pst_fm_station_list->u8_NumberOfStationsInList;
	Tu8						u8_Non_RDS_Starting_Index  	= 	u8_STL_Size -u8_Total_NonRDS_Station;
	Tu8						u8_temp_buffer_index			=	0;
	Ts_AMFM_App_temp_buffer	st_temp_buffer[AMFM_APP_MAX_NON_RDS_STATIONS_IN_FM_STL];
	Tu32					u32_temp_quality		= 0;
	Tu32					u32_temp_minindex = 0;
	Tu8						u8_temp_RemoveStationIndex = 0;

	memset((void *)st_temp_buffer, 0u, (sizeof(Ts_AMFM_App_temp_buffer)* AMFM_APP_MAX_NON_RDS_STATIONS_IN_FM_STL));

	for(u8_loopindex = u8_Non_RDS_Starting_Index ;u8_loopindex < u8_STL_Size ;u8_loopindex++)
	{
		if(u8_temp_buffer_index < AMFM_APP_MAX_NON_RDS_STATIONS_IN_FM_STL)
		{
			st_temp_buffer[u8_temp_buffer_index].u8_index	 =	u8_loopindex;
			st_temp_buffer[u8_temp_buffer_index].u32_quality = 	AMFM_App_GetFreqQualityfrom_TunerCtrl_STL(pst_fm_station_list->ast_Stations[u8_loopindex].u32_frequency);
			
			u8_temp_buffer_index++;		
		}
		else
		{
			u32_temp_quality 	= AMFM_App_GetFreqQualityfrom_TunerCtrl_STL(pst_fm_station_list->ast_Stations[u8_loopindex].u32_frequency);

			u32_temp_minindex	= AMFM_App_findMinQuality(&st_temp_buffer[0]);

			if(u32_temp_quality	 > st_temp_buffer[u32_temp_minindex].u32_quality)
			{
				u8_temp_RemoveStationIndex = st_temp_buffer[u32_temp_minindex].u8_index;

				AMFM_App_Remove_FM_stationfromSTL(&pst_fm_station_list->ast_Stations[0],u8_STL_Size,u8_temp_RemoveStationIndex);

				st_temp_buffer[u32_temp_minindex].u8_index			= u8_loopindex;
				st_temp_buffer[u32_temp_minindex].u32_quality		= u32_temp_quality;
						
				u8_innerloop_index = 0;
				
				while(u8_innerloop_index < AMFM_APP_MAX_NON_RDS_STATIONS_IN_FM_STL)
				{
					if(st_temp_buffer[u8_innerloop_index].u8_index  > u8_temp_RemoveStationIndex)
					{
						st_temp_buffer[u8_innerloop_index].u8_index--;
					}
						
					u8_innerloop_index++;
				}
			}
			else
			{
				AMFM_App_Remove_FM_stationfromSTL(&pst_fm_station_list->ast_Stations[0],u8_STL_Size,u8_loopindex);
			}	
			u8_loopindex = u8_loopindex - 1;
			
			u8_STL_Size = --(pst_fm_station_list->u8_NumberOfStationsInList);
		}
	}	
}
/*===========================================================================*/
/*			Ts8 AMFM_App_Remove_FM_stationfromSTL						 */
/*===========================================================================*/
void AMFM_App_Remove_FM_stationfromSTL(Ts_AMFM_App_FMStationInfo * pst_fm_station_list,Tu8 u8_STL_Size,Tu8 u8_remove_Station_Index)
{
	Tu8							u8_loopindex		= 0;	

	for(u8_loopindex = u8_remove_Station_Index;u8_loopindex < u8_STL_Size;u8_loopindex++)
	{
			pst_fm_station_list[u8_loopindex] = pst_fm_station_list[u8_loopindex + 1] ;
	}
	
	if(u8_STL_Size == AMFM_APP_MAX_FM_STL_SIZE)
	{
		/* clearing the last buffer  */
		memset((&pst_fm_station_list[u8_STL_Size-1]),0,sizeof(Ts_AMFM_App_FMStationInfo));
	}	
}
/*===========================================================================*/
/*			Tu32 AMFM_App_findMinQuality									 */
/*===========================================================================*/
Tu32 AMFM_App_findMinQuality(Ts_AMFM_App_temp_buffer *pst_temp_buffer)
{
	Tu32	u32_ret_min_quality_index 	= 	0;
	Tu32	u32_loopindex				=   0;	

	for(u32_loopindex = u32_ret_min_quality_index + 1;u32_loopindex < AMFM_APP_MAX_NON_RDS_STATIONS_IN_FM_STL; u32_loopindex++)
	{
			if(pst_temp_buffer[u32_loopindex].u32_quality <  pst_temp_buffer[u32_ret_min_quality_index].u32_quality)
			{
				u32_ret_min_quality_index = u32_loopindex;
			}
	}
	return	u32_ret_min_quality_index;
}

	
/*===========================================================================*/
/*			void AMFM_App_application_Remove_SpacefromStationList						 */
/*===========================================================================*/
void AMFM_App_application_Remove_SpacefromStationList(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_STL_Size)
{
	Tu8										u8_index;                               /*variable to indicate the station list index*/
	Tu8										u8_characterindex;
	Tu8										u8_repeatindex;
 	for	(u8_index = 0; u8_index < u8_STL_Size ; u8_index++)
	{
	  for(u8_repeatindex = 0; u8_repeatindex < MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME ;  u8_repeatindex ++)
	  {		
		if(pst_fm_station_list[u8_index].au8_psn[0] == ' ')
		{
			for(u8_characterindex = 0; u8_characterindex < MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME - 1 ; u8_characterindex ++ )
			{
				pst_fm_station_list[u8_index].au8_psn[u8_characterindex] = pst_fm_station_list[u8_index].au8_psn[u8_characterindex + 1];
			}
			pst_fm_station_list[u8_index].au8_psn[7] = (Tu8)'\0';	
		}
		else
		{
			break;
		}
	   }	
		
	}
	
	
}


/*===========================================================================*/
/*			Ts8  AMFM_App_CheckPIpresentInFMSTL 					 */
/*===========================================================================*/
Ts8 AMFM_App_CheckPIpresentInFMSTL(Tu8 u8_StationIndexLimit,Tu16 u16_NewStationPI,Tu8	*pu8_PSN ,Ts_AMFM_App_FM_STL *pst_fm_station_list)
{
	Tu8			u8_index;                               /*variable to indicate the station list index*/
	Ts8			s8_RetValue_index	= -1;
	
	for(u8_index = 0; u8_index < u8_StationIndexLimit; u8_index++)
	{
		if(u16_NewStationPI == pst_fm_station_list->ast_Stations[u8_index].u16_PI)
		{	
			if (0 == AMFM_APP_String_length(pst_fm_station_list->ast_Stations[u8_index].au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME) )  
			{		
				/* Copying PSN */
				SYS_RADIO_MEMCPY(pst_fm_station_list->ast_Stations[u8_index].au8_psn,(const void *)pu8_PSN,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);
			}
			
			/* New station is already existing in STL with other frequency */
			s8_RetValue_index = u8_index;

			u8_index 	= (Tu8)(u8_StationIndexLimit + 1) ;			/* condition to stop the loop */
		}
	}
	return s8_RetValue_index;
}


Tu32	AMFM_App_GetFreqQualityfrom_TunerCtrl_STL(Tu32 u32_Freq)
{
	Tu8		u8_index = 0;                               /* Variable to indicate the station list index of Tuner Ctrl */
	Tu32	u32_ret_quality = 0;

	while((u32_Freq != ast_Scaninfo[u8_index].u32_freq) && (u8_index < scan_index_FM))
	{
		u8_index++;
	}
	
	if(u8_index < scan_index_FM)
	{
		u32_ret_quality = ast_Scaninfo[u8_index].u32_quality_interpolation;	
	}
	else
	{
		u32_ret_quality = 0;
	}
	return u32_ret_quality;
}
	
/*===========================================================================*/
/*			Tbool AMFM_App_application_GenerateFMStationList 				 */
/*===========================================================================*/
Tu8 AMFM_App_application_GenerateFMStationList(Ts_AMFM_App_FM_STL *pst_fm_station_list,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem,Te_AMFM_App_Market	e_MarketType )
{
	Tu8						u8_index;                               /*variable to indicate the station list index*/
	Tbool					b_ret_value = FALSE;
	Tbool				    b_LM_ret = FALSE;
	Tu8						u8_Append_index = 0;						/* Variable to indicate the station list index where FM station to be added in AMFM APP */
	Ts8		u8_existingStation_index = -1;
	Tu32	u32_existingStation_quality = 0;		
	Tu8		u8_Total_NonRDS_Station = 0;     /*variable to calculate total number of non rds stations */
	//Tu8		u8_Non_RDS_Starting_Index  = 0;

	RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AMFM APP Mutex Lock fo both RM_APP and APP_TC AMFM_App_application_GenerateFMStationList ");
	/*Locking the MUTEX */
	SYS_MUTEX_LOCK(STL_AMFM_APP_AMFM_TC);	/* Locking the mutex between TC and APP*/
	SYS_MUTEX_LOCK(STL_RM_AMFM_APP);		/* Locking the mutex between RM and APP*/
	
	/*clearing FM station List */
	memset(pst_fm_station_list,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_FM_STL));
	
	/* copies station list information from shared memory to  Ts_AMFM_App_FM_STL structure*/
	for(u8_index=0; (u8_index <  scan_index_FM)  && (u8_index < AMFM_APP_MAX_FM_STL_SIZE); u8_index++)
	{
		if(ast_Scaninfo[u8_index].u16_pi  != 0)
		{
			u8_existingStation_index = AMFM_App_CheckPIpresentInFMSTL(u8_index,ast_Scaninfo[u8_index].u16_pi,ast_Scaninfo[u8_index].au8_ps	,pst_fm_station_list);
		}
		else
		{
			/* It can be Non-RDS FM station. Needs to be added */
			u8_existingStation_index = -1;
		}

		if(u8_existingStation_index == -1)
		{
			pst_fm_station_list->ast_Stations[u8_Append_index].u32_frequency		=  (ast_Scaninfo[u8_index].u32_freq);	
			pst_fm_station_list->ast_Stations[u8_Append_index].u16_PI			=  ast_Scaninfo[u8_index].u16_pi;	
				
			/* Copying PSN */
			SYS_RADIO_MEMCPY((pst_fm_station_list->ast_Stations[u8_Append_index].au8_psn),(const void *)(ast_Scaninfo[u8_index].au8_ps),sizeof(ast_Scaninfo[u8_index].au8_ps) );
		
			pst_fm_station_list ->u8_NumberOfStationsInList++;
			
			u8_Append_index++;
		}
		else
		{
			u32_existingStation_quality = AMFM_App_GetFreqQualityfrom_TunerCtrl_STL(pst_fm_station_list->ast_Stations[u8_existingStation_index].u32_frequency);

			/* Comparing already existing station Quality with new station qualtiy as their PIs are same.
			   Station(frequency) is having greater quality has to be appended in FM STL */
			   
			if(u32_existingStation_quality < ast_Scaninfo[u8_index].u32_quality_interpolation)
			{
				pst_fm_station_list->ast_Stations[u8_existingStation_index].u32_frequency =  (ast_Scaninfo[u8_index].u32_freq) ;	

				/* No need to copy PI and PSN as for FM stations having same PI will have same PSN  */
				/* No need to increment u8_Append_index too as just frequency is replaced at existing index */
			}
			else
			{
				/* Existing station quality is greater than new station quality so no need to anything*/
			}
		}

		/* Updating learn memory */
		b_LM_ret = AMFM_App_Learn_Memory_updation((ast_Scaninfo[u8_index].u32_freq),ast_Scaninfo[u8_index].u16_pi,pst_AF_Learn_mem);
		
		if(b_LM_ret == TRUE)
		{	
			/* If u8_LM_ret  flag is set,it means Learn memory has been changed. So NVM has to be updated.Hence making u8_ret_value flag set */
			b_ret_value = TRUE;
		}
	}
	/*UnLocking the MUTEX between TC and APP*/
	SYS_MUTEX_UNLOCK(STL_AMFM_APP_AMFM_TC);
	RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AMFM APP Mutex UnLock of APP TC at AMFM_App_application_GenerateFMStationList ");

	if(pst_fm_station_list -> u8_NumberOfStationsInList != 0)
	{
		AMFM_App_application_Remove_SpacefromStationList((Ts_AMFM_App_FMStationInfo *)(pst_fm_station_list->ast_Stations),pst_fm_station_list->u8_NumberOfStationsInList);
		u8_Total_NonRDS_Station = AMFM_App_application_SortFMStationList((Ts_AMFM_App_FMStationInfo *)(pst_fm_station_list->ast_Stations),pst_fm_station_list->u8_NumberOfStationsInList);

		
		/* Only 5 Non-RDS station should be present in station list for specific market  */
		if(u8_Total_NonRDS_Station > AMFM_APP_MAX_NON_RDS_STATIONS_IN_FM_STL)
		{
			if(e_MarketType == AMFM_APP_MARKET_WESTERN_EUROPE)
			{	
				AMFM_App_Remove_Non_RDS_Stations(pst_fm_station_list,u8_Total_NonRDS_Station);
			}			
		}
		
	}
	/* UnLocking the mutex between RM and APP*/
	SYS_MUTEX_UNLOCK(STL_RM_AMFM_APP);		
	RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AMFM APP Mutex UnLock of RM_APP at AMFM_App_application_GenerateFMStationList ");

	
	
	return b_ret_value;
}

/*===========================================================================*/
/*			void AMFM_App_CheckFreqPresentAMSTL			 						 */
/*===========================================================================*/
Ts8 AMFM_App_CheckFreqPresentAMSTL(Tu32 u32_Freq,Ts_AMFM_App_AM_STL *pst_am_station_list)
{
	Ts8		s8_Retindex					= -1;
	Tu8		u8_NumberOfStationsInList 	= 0;
	Tu8		u8_index					= 0;	

	if(pst_am_station_list != NULL)
	{
		u8_NumberOfStationsInList = pst_am_station_list->u8_NumberOfStationsInList;
		
		for(u8_index= 0; u8_index < u8_NumberOfStationsInList;u8_index++)
		{
			if(u32_Freq == pst_am_station_list->ast_Stations[u8_index].u32_Freq)
			{
				s8_Retindex	= (Ts8)u8_index;
				u8_index 	= (Tu8)(u8_NumberOfStationsInList + 1) ;			/* Condition to stop the loop */
			}
		}
	}
	else
	{
		/* needs to send error msg or add failure DEBUG LOG */
	}

	return s8_Retindex;
}

/*===========================================================================*/
/*			void AMFM_App_Remove_SamePIfromFMSTL			 						 */
/*===========================================================================*/
Tbool AMFM_App_Remove_SamePIfromFMSTL(Tu16 u16_NewStation_PI,Ts_AMFM_App_FM_STL *pst_fm_station_list)
{
	Tu8		u8_NumberOfStationsInList 	= 0;
	Tu8		u8_index					= 0;	
	Tbool	b_retval					= FALSE;
	Tu8		u8_shiftindex;

	if(pst_fm_station_list != NULL)
	{
		u8_NumberOfStationsInList = pst_fm_station_list->u8_NumberOfStationsInList;
	
		for(u8_index= 0; u8_index < u8_NumberOfStationsInList;u8_index++)
		{
			if(u16_NewStation_PI == pst_fm_station_list->ast_Stations[u8_index].u16_PI)
			{
				for(u8_shiftindex = u8_index ; u8_shiftindex < u8_NumberOfStationsInList;u8_shiftindex++)
				{
					pst_fm_station_list->ast_Stations[u8_shiftindex] = pst_fm_station_list->ast_Stations[u8_shiftindex + 1];
				}

				pst_fm_station_list->u8_NumberOfStationsInList--;
				b_retval = TRUE;
			}
		}
	}	
	return b_retval;
}


/*===========================================================================*/
/*			void AMFM_App_CheckFreqPresentFMSTL			 						 */
/*===========================================================================*/
Ts8 AMFM_App_CheckFreqPresentFMSTL(Tu32 u32_Freq,Ts_AMFM_App_FM_STL *pst_fm_station_list)
{
	Ts8		s8_Retindex					= -1;
	Tu8		u8_NumberOfStationsInList 	= 0;
	Tu8		u8_index					= 0;	

	if(pst_fm_station_list != NULL)
	{
		u8_NumberOfStationsInList = pst_fm_station_list->u8_NumberOfStationsInList;
		
		for(u8_index= 0; u8_index < u8_NumberOfStationsInList;u8_index++)
		{
			if(u32_Freq == pst_fm_station_list->ast_Stations[u8_index].u32_frequency)
			{
				s8_Retindex 	= (Ts8)u8_index;
				u8_index 	= (Tu8)(u8_NumberOfStationsInList + 1) ;			/* Condition to stop the loop */
			}
		}
	}
	else
	{
		/* needs to send error msg or add failure DEBUG LOG */
	}

	return s8_Retindex;
}




/*===========================================================================*/
/*			void AMFM_App_ValidatePIcode			 						 */
/*===========================================================================*/
Tbool AMFM_App_ValidatePIcode(Ts_AMFM_App_LinkingParam  *pst_DAB_FM_LinkingParam,Tu16 u16_PI)
{
	Tbool		b_RetValue	= FALSE;
	Tu8			u8_index 	= 0;
	Tu8			u8_PIcount	= pst_DAB_FM_LinkingParam->st_PIList.u8_PIcount ;
	
	/* Checking given PI code with  PI list received from DAB */
	for(u8_index = 0;u8_index < u8_PIcount;u8_index++ )
	{
		if(u16_PI == pst_DAB_FM_LinkingParam->st_PIList.au16_PIList[u8_index])
		{
			b_RetValue	= TRUE;
			u8_index 	= (Tu8)(u8_PIcount + 1) ;			// condition to stop the loop 
		}
	}
	return b_RetValue;
}
/*===========================================================================*/
/*			Tu8 	AMFM_App_GenerateHardLinkFreq			 				 */
/*===========================================================================*/
void	AMFM_App_GenerateHardLinkFreq( Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam,Tu16 u16_STL_Size,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
	Tbool	b_flag		  = FALSE;				/* Flag used to check whether any Hardlink PI is available in FM Band (has any frequency) */
	Tu8		u8_OuterLoop;
	Tu8		u8_InnerLoop;

	Tu8*	pu8_FreqIndex	= 	&pst_DAB_FM_LinkingParam->u8_HL_Freq_Count;
	Tu8		u8_PI_Count		= 	pst_DAB_FM_LinkingParam->st_PIList.u8_PIcount;


	pst_DAB_FM_LinkingParam->b_IsHLfreqAvailable = FALSE;
	
	/* Outer loop is used for comparing all PIs present PI list with STL */
	for(u8_OuterLoop = 0; u8_OuterLoop < u8_PI_Count;u8_OuterLoop++)
	{
		/**/
		b_flag = FALSE;	
		
		/* Inner loop is used to compare a PI with all PIs of all station in STL*/
		for(u8_InnerLoop = 0; (u8_InnerLoop <= u16_STL_Size)  &&  (*pu8_FreqIndex < AMFM_APP_MAX_NO_OF_FREQ_IN_LIST ) ;u8_InnerLoop++ )
		{
			/* Comparing PI from PI list with PI of a station present in the STL */
			if(pst_DAB_FM_LinkingParam->st_PIList.au16_PIList[u8_OuterLoop] == 	pst_AF_Learn_mem[u8_InnerLoop].u16_PI)
			{
				if((pst_AF_Learn_mem[u8_InnerLoop].u32_frequency >= u32_start_freq) && (pst_AF_Learn_mem[u8_InnerLoop].u32_frequency <= u32_End_Freq))
				{	
					/* Updating HardLink Freq list */
					pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[*pu8_FreqIndex].u16_PI 	 = pst_AF_Learn_mem[u8_InnerLoop].u16_PI;			// (pst_AF_Learn_mem + u8_InnerLoop)->u16_PI
				
					pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[*pu8_FreqIndex].u32_Freq =  pst_AF_Learn_mem[u8_InnerLoop].u32_frequency;		// pst_AF_Learn_mem[u8_InnerLoop]->u32_frequency  
				
					(*pu8_FreqIndex)++;
	
				pst_DAB_FM_LinkingParam->b_IsHLfreqAvailable = TRUE;

				b_flag = TRUE;
				}
				else
				{
					/* Invalid freq.No need append into List */
				}
				
			}
		}
		if(*pu8_FreqIndex >= AMFM_APP_MAX_NO_OF_FREQ_IN_LIST )
		{
			break; 			/*  Hardlink frequency buffer(ast_HL_Freq_Quality_List) is full */
		}
		
		if(b_flag == FALSE)
		{	
			/* Present instance PI doesn't have any frequency in the STL.But adding it into HL Freq List with Frequency Zero  */
			pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[*pu8_FreqIndex].u16_PI  = pst_DAB_FM_LinkingParam->st_PIList.au16_PIList[u8_OuterLoop];	

			pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[*pu8_FreqIndex].u32_Freq = AMFM_APP_CONSTANT_ZERO;      
				
			(*pu8_FreqIndex)++;
		}
	}


}
/*===========================================================================*/
/*			void 	AMFM_App_FindNextValidFreq		    					 */
/*===========================================================================*/
Tbool AMFM_App_FindNextValidFreq(Ts_AMFM_App_HardLink_Freq_Quality_List *pst_HL_Freq_Quality_List, Tu8 u8_TotalCount, Tu8 *pu8_index)
 {
 	Tbool	b_RetValue = FALSE;
	
 	while(*pu8_index < u8_TotalCount)
 	{
 		if(pst_HL_Freq_Quality_List[*pu8_index].u32_Freq != 0)
 		{
 			b_RetValue = TRUE;
 			break;
 		}
		else
		{
			(*pu8_index)++; 
		}
 	}

	return b_RetValue;
 }
/*===========================================================================*/
/*			void 	AMFM_APP_Clear_LinkingParam		    					 */
/*===========================================================================*/
void AMFM_APP_Clear_LinkingParam(Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam)
{
	if(pst_DAB_FM_LinkingParam != NULL)
	{
		pst_DAB_FM_LinkingParam->u8_HL_Freq_Count = 0;

		pst_DAB_FM_LinkingParam->u8_AF_Count = 0;

		pst_DAB_FM_LinkingParam->u8_TotalFreq = 0;
			
		memset(pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List,0,AMFM_APP_MAX_NO_OF_FREQ_IN_LIST * sizeof(Ts_AMFM_App_HardLink_Freq_Quality_List));
			
		/* Clearing Best PI Info */
 		memset(&pst_DAB_FM_LinkingParam->st_Best_PI_Info,0,sizeof(Ts_AMFM_App_Best_PI_Info));

		
		pst_DAB_FM_LinkingParam->b_IsBeginning = FALSE;

#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
		pst_DAB_FM_LinkingParam->b_Request_Received = FALSE;
#endif
		pst_DAB_FM_LinkingParam->b_DAB2FM_LinkingFlag = FALSE;
		
		pst_DAB_FM_LinkingParam->u8_ActualPICount = 0;

		pst_DAB_FM_LinkingParam->st_Best_PI_Info.b_PIchangedFlag = FALSE;
		
	}	
}
/*===========================================================================*/
/*			void 	AMFM_App_Sort_HardLink_Freq_List    					 */
/*===========================================================================*/
void	AMFM_App_Sort_HardLink_Freq_List(Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam)
{
	Tu8											u8_OuterIndex;                      /* variable to control the outer loop*/
	Tu8											u8_InnerIndex;                                       /* variable to control the inner loop*/
	Ts_AMFM_App_HardLink_Freq_Quality_List		st_Temp_HL_FreqList;
	
	 
	if(pst_DAB_FM_LinkingParam != NULL)
	{
		Tu8				u8_HL_FreqCount = pst_DAB_FM_LinkingParam->u8_HL_Freq_Count;
		
		for(u8_OuterIndex = 0;u8_OuterIndex < u8_HL_FreqCount - (Tu8)1u;u8_OuterIndex++)
		{
			for(u8_InnerIndex = 0; u8_InnerIndex < (u8_HL_FreqCount - (Tu8)1u - u8_OuterIndex); u8_InnerIndex++)
			{
					if(pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_InnerIndex].s32_BBFieldStrength < pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_InnerIndex+1].s32_BBFieldStrength)
					{
						//Do swap 
						st_Temp_HL_FreqList 												= pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_InnerIndex];
						pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_InnerIndex] 	= pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_InnerIndex+1];
						pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_InnerIndex+1]	= st_Temp_HL_FreqList;						
					}
			}
		}
	}	
}
/*=====================================================================================*/
/*			void 	AMFM_App_Append_AF_Into_List									 	    				 		      */
/*=====================================================================================*/
void	AMFM_App_Append_AF_Into_List(Tu8 u8_AFCount,Tu32 *pu32_AFList,Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
	Tu8							u8_loop_index ;
	Tu16						u16_PI;
	Tbool						b_RetValue	= FALSE;
	Te_AMFM_App_AF_PI_STATUS	e_AF_PI_STATUS;
	Tu8							u8_Append_Index	= pst_AMFM_App_AFList_Info->u8_NumAFList;	// pst_DAB_FM_LinkingParam->u8_HL_Freq_in_AFlist;
	Tbool							b_verfreq						= FALSE;   
	Te_AMFM_App_mode				e_mode	=AMFM_APP_MODE_FM;
	Te_AF_Freq_Availabilty_Check	e_AF_Freq_Availabilty_Check = AF_FREQUENCY_INVALID;
	
	for(u8_loop_index =  0; (u8_loop_index < u8_AFCount) && (u8_Append_Index < AMFM_APP_MAX_NO_AF_STATIONS -1 );u8_loop_index++)
	{ 
		b_verfreq = AMFM_APP_VerifyFrequency((pu32_AFList[u8_loop_index]),&e_mode,st_hsm_info.st_inst_hsm_info.e_MarketType ,&st_hsm_info.st_inst_hsm_info.st_MarketInfo ) ;
		if(b_verfreq	== TRUE)
		{
			e_AF_Freq_Availabilty_Check = AMFM_App_Freq_Existance_Check_New(pst_AMFM_App_AFList_Info,pu32_AFList[u8_loop_index]);

			if(e_AF_Freq_Availabilty_Check == AF_FREQUENCY_TO_BE_ADDED)
			{
				if(pu32_AFList[u8_loop_index] != pst_AMFM_App_AFList_Info->u32_curr_freq)
				{
					/* Adding freq */
					pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[ u8_Append_Index ].u32_AF_Freq	= pu32_AFList[u8_loop_index];

					u16_PI = AMFM_App_Learn_memory_PI_Read(pu32_AFList[u8_loop_index],pst_AF_Learn_mem);
					
					/* Adding PI */
					pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[ u8_Append_Index].u16_PI = u16_PI;
					
					if(u16_PI == 0 )
					{
						e_AF_PI_STATUS=AMFM_APP_PI_STATUS_ZERO;
					}
					else
					{
						b_RetValue = AMFM_App_ValidatePIcode(pst_DAB_FM_LinkingParam,u16_PI);

						if(b_RetValue == TRUE)
						{
							e_AF_PI_STATUS=AMFM_APP_PI_STATUS_SAME;
						}
						else
						{
							e_AF_PI_STATUS=AMFM_APP_PI_STATUS_NEG;
						}
					}
					
					/* Updating status of PI */
					pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_Append_Index].e_AF_PI_STATUS = e_AF_PI_STATUS;

					pst_AMFM_App_AFList_Info->u8_NumAFList++;
					
					u8_Append_Index++;
					
					#ifdef CALIBRATION_TOOL
						 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_Append_Index].u32_AF_Freq		= pu32_AFList[u8_loop_index];
						 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_Append_Index].u16_PI 			= u16_PI;
						 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_Append_Index].e_AF_PI_STATUS 	= e_AF_PI_STATUS;
						 st_Calib_AMFM_App_AFList_Info.u8_NumAFList++;
					#endif
					
				}
				
			}
			else
			{
				/* Nothing to do */
			}
		}
		else
		{
			/* Go for next next freq */
		}
	}	
	pst_DAB_FM_LinkingParam->u8_TotalFreq	=	pst_AMFM_App_AFList_Info->u8_NumAFList ;
	pst_DAB_FM_LinkingParam->u8_AF_Count	= 	(Tu8)(pst_DAB_FM_LinkingParam->u8_TotalFreq - pst_DAB_FM_LinkingParam->u8_HL_Freq_in_AFlist);
		
}
/*=====================================================================================*/
/*			void 	AMFM_App_Append_HL_Freq_into_List								 	    				 		      */
/*=====================================================================================*/
void AMFM_App_Append_HL_Freq_into_List(Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8		u8_Index 			=	0;
	Tu8		u8_HL_Freq_Count 	=	pst_DAB_FM_LinkingParam->u8_HL_Freq_Count;
	Tu8		u8_Append_Index		=	0;	
	Tu32	u32_BestPI_Freq		=	pst_DAB_FM_LinkingParam->st_Best_PI_Info.u32_Frequency;		
	Tu32	u32_HL_list_freq;	
	Tbool							b_verfreq						= FALSE;   
	Te_AMFM_App_mode				e_mode	=AMFM_APP_MODE_FM;
	for(u8_Index = 0; (u8_Index < u8_HL_Freq_Count) &&  (u8_Index < (AMFM_APP_MAX_NO_AF_STATIONS -1) ); u8_Index++)
	{ 
		u32_HL_list_freq = pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_Index].u32_Freq;
		b_verfreq 		= AMFM_APP_VerifyFrequency((u32_HL_list_freq),&e_mode ,st_hsm_info.st_inst_hsm_info.e_MarketType ,&st_hsm_info.st_inst_hsm_info.st_MarketInfo ) ;
		
		if( (u32_HL_list_freq != u32_BestPI_Freq) 	&& (b_verfreq == TRUE))
		{
			pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[ u8_Append_Index ].u32_AF_Freq			= 	u32_HL_list_freq;
			pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[ u8_Append_Index].u16_PI				=	pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_Index].u16_PI;
			pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[ u8_Append_Index].e_AF_PI_STATUS 		=	AMFM_APP_PI_STATUS_SAME;		
			u8_Append_Index++;
			
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_Append_Index ].u32_AF_Freq		= 	u32_HL_list_freq;
				st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_Append_Index].u16_PI			=	pst_DAB_FM_LinkingParam->ast_HL_Freq_Quality_List[u8_Index].u16_PI;
				st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_Append_Index].e_AF_PI_STATUS 	=	AMFM_APP_PI_STATUS_SAME;		
			#endif
		}
	}	
	pst_DAB_FM_LinkingParam->u8_HL_Freq_in_AFlist 	= u8_Append_Index;
	pst_AMFM_App_AFList_Info->u8_NumAFList			= u8_Append_Index;
	
	#ifdef CALIBRATION_TOOL
		st_Calib_AMFM_App_AFList_Info.u8_NumAFList	= u8_Append_Index;
	#endif 
}	

void AMFM_APP_Start_Seek(Tu32 u32_CurrentFrequency,Ts_AMFM_App_MarketInfo *pst_MarketInfo,Tu16 u16_PI)
{
	Tu32								u32_LowerFreq	    = 0;
	Tu32								u32_StartFreq	    = 0;
	Tu32								u32_UpperFreq   	= 0;
	Tu32								u32_Step_Size	    = 0;
	u32_StartFreq = u32_CurrentFrequency;
	u32_LowerFreq = pst_MarketInfo->st_FMband_FreqInfo.u32_StartFreq;
	u32_UpperFreq = pst_MarketInfo->st_FMband_FreqInfo.u32_EndFreq;
	u32_Step_Size = pst_MarketInfo->st_FMband_FreqInfo.u32_StepSize;			
	/* Requesting tuner Ctrl to perform seek operation */
	AMFM_Tuner_Ctrl_Request_PISeek(u32_LowerFreq,u32_StartFreq,u32_UpperFreq,u32_Step_Size,u16_PI); 	

}

void AMFM_APP_New_EONList_AF_update(Ts_AMFM_App_EON_List *pst_EON_List,Tu32 u32_CurrentFrequency,Ts_AMFM_App_MarketInfo *pst_MarketInfo)
{
	Tu8 	u8_EON_AF_List_Index = 0;
	Tu8 	u8_EONlist_PI_index=pst_EON_List->u8_EONlist_PIindex;
	
	(pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex)++;
	
	u8_EON_AF_List_Index 	= 	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex;
	
	
	if(u8_EON_AF_List_Index <  pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u8_AF_Count)
	{
		/* update request to next eon af freq*/
		AMFM_Tuner_Ctrl_Request_AF_Update(pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index]);
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_APP] EON PI not available in EON PI list so doing PI seek");

		/*none of the frequencies in the list has better quality perform seek operation */
		AMFM_APP_Start_Seek(u32_CurrentFrequency,pst_MarketInfo,pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u16_EON_PI);
	}
}

Tu8 AMFM_APP_AvailablePI_Index(Ts_AMFM_App_EON_List *pst_EON_List,Tu16 u16_pi)
{
	Tu8 u8_EON_PI_Index;
	Tu8 u8_piindex = 0xff;
	for(u8_EON_PI_Index=0;u8_EON_PI_Index<(pst_EON_List->u8_PI_Count);u8_EON_PI_Index++)
	{
		if((pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index]. u16_EON_PI ) == u16_pi)
		{
			u8_piindex=u8_EON_PI_Index;
			break;
		}
		else
		{
			/*do nothing*/
		}
	}
	return u8_piindex;
}


Tbool AMFM_APP_Check_PI_Availability(Ts_AMFM_App_EON_List *pst_EON_List, Tu16 u16_pi)
{
	Tu8 u8_EON_PI_Index;
	Tbool 	b_ret_EON_PI_Flag = FALSE;
	for(u8_EON_PI_Index=0;u8_EON_PI_Index<(pst_EON_List->u8_PI_Count);u8_EON_PI_Index++)
	{
		if((pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index]. u16_EON_PI ) == u16_pi)
		{
			b_ret_EON_PI_Flag = TRUE;
			break;
		}
		else
		{
			/*do nothing*/
		}
	}
	return b_ret_EON_PI_Flag;
}


void AMFM_APP_Adding_PIFrequency_From_LearnMemory(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Tu16 u16_pi,Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info)
{
	Tu32 	u32_LearnMemIndex;
	Ts8 	s8_freq_index = -1;
	Tbool 		b_PI_Added_Flag  = FALSE;
	Tu8 	u8_EON_PI_Index =  pst_me_amfm_app_inst->st_EON_List.u8_PI_Count;
	
	for(u32_LearnMemIndex=0;u32_LearnMemIndex < AMFM_APP_NO_STATIONS_FM_BAND ;u32_LearnMemIndex++)
	{
		if(u16_pi == pst_me_amfm_app_inst->ast_AF_Learn_mem[u32_LearnMemIndex].u16_PI)
		{
			s8_freq_index= pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count;
			if(b_PI_Added_Flag == FALSE)
			{
				b_PI_Added_Flag = TRUE;
				pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EON_PI_Index]. u16_EON_PI  = u16_pi;
				pst_me_amfm_app_inst->st_EON_List.u8_PI_Count++;
			}
			pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EON_PI_Index].u32_EON_AF_List[s8_freq_index]=pst_me_amfm_app_inst->ast_AF_Learn_mem[u32_LearnMemIndex].u32_frequency;
			pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count++;	
			
		}
	}
	if(s8_freq_index != -1)
	{
		AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,pst_AMFM_App_AFList_Info,AMFM_APP_EON_FREQ_APPENDED_FROM_LEARN_MEMORY);
	}
}

/*
void AMFM_APP_Adding_PIFrequency_From_FM_STL(Ts_AMFM_App_EON_List *pst_EON_List,Tu16 u16_pi, Ts_AMFM_App_FM_STL *pst_fm_station_list)
{
	Tu8 	u8_STLindex;
	Tu8		u8_STL_Size		=	pst_fm_station_list->u8_NumberOfStationsInList;
	Tu8 	u8_PI_Added_Flag=0;
	Tu8 	u8_EON_PI_Index =   pst_EON_List->u8_PI_Count;
	
	for(u8_STLindex=0;u8_STLindex < u8_STL_Size;u8_STLindex++)
	{
		u8_EON_PI_Index =   pst_EON_List->u8_PI_Count;
		if(u16_pi == pst_fm_station_list->ast_Stations[u8_STLindex].u16_PI)
		{
			if(u8_PI_Added_Flag == 0)
			{
				u8_PI_Added_Flag++;
				pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index]. u16_EON_PI  = u16_pi;
				pst_EON_List->u8_PI_Count++;
			}
			pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u32_EON_AF_List[pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count]=((pst_fm_station_list->ast_Stations[u8_STLindex].u32_frequency));
			pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count++;	
		}
	}
}
*/
void AMFM_APP_Update_EON_List(Ts_AMFM_App_EON_List *pst_EON_List,Ts_AMFM_TunerCtrl_EON_Info st_AMFM_TunerCtrl_Eon_Info)
{

	Tu8 u8_EON_PI_Index;
	Tu8 u8_EON_AF_ListIndex;
	Tu8 u8_TunerCtrl_EON_AF_listIndex;
	Tbool 	b_EON_PI_Flag = TRUE;
	Tbool 	b_EON_AF_Flag = TRUE;
	
	for(u8_EON_PI_Index=0;u8_EON_PI_Index < (pst_EON_List->u8_PI_Count);u8_EON_PI_Index++)
	{
		if((pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u16_EON_PI ) == st_AMFM_TunerCtrl_Eon_Info.u16_EON_PI)
		{
			b_EON_PI_Flag = FALSE;
			for(u8_TunerCtrl_EON_AF_listIndex=0; u8_TunerCtrl_EON_AF_listIndex <AMFM_APP_MAX_NO_EON_AF_STATIONS; u8_TunerCtrl_EON_AF_listIndex ++)
			{
				b_EON_AF_Flag = TRUE;
				for(u8_EON_AF_ListIndex=0;u8_EON_AF_ListIndex < pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count ;u8_EON_AF_ListIndex++ )
				{
					if (pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u32_EON_AF_List[u8_EON_AF_ListIndex] == st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[u8_TunerCtrl_EON_AF_listIndex])
					{
						b_EON_AF_Flag = FALSE;
						break;
					}
					else
					{
						/*do nothing */
					}
				}
			
				if(b_EON_AF_Flag == TRUE)
				{
					if (st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[u8_TunerCtrl_EON_AF_listIndex] != 0)
					{
						pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u32_EON_AF_List[pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count] = st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[u8_TunerCtrl_EON_AF_listIndex];
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] EON Announcement station info received  %x EON PI and %d   freq of EON station  ",
													pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u16_EON_PI ,
													st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[u8_TunerCtrl_EON_AF_listIndex]);

						pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count++;
					}
					else
					{
						/*do nothing */
					}
				}
				else
				{
					/*do nothing */
				}

			}
			break;
		}
		else
		{
			/*do nothing */
		}
	}
	
	if(b_EON_PI_Flag == TRUE)
	{
		if(st_AMFM_TunerCtrl_Eon_Info.u16_EON_PI!=0)
		{
			pst_EON_List->st_EON_PI_Info[pst_EON_List->u8_PI_Count]. u16_EON_PI  = st_AMFM_TunerCtrl_Eon_Info.u16_EON_PI;
			
			
			for(u8_TunerCtrl_EON_AF_listIndex=0; u8_TunerCtrl_EON_AF_listIndex <AMFM_APP_MAX_NO_EON_AF_STATIONS; u8_TunerCtrl_EON_AF_listIndex ++)
			{
				if (st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[u8_TunerCtrl_EON_AF_listIndex] != 0)
				{
					pst_EON_List->st_EON_PI_Info[pst_EON_List->u8_PI_Count].u32_EON_AF_List[pst_EON_List->st_EON_PI_Info[u8_EON_PI_Index].u8_AF_Count] = st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[u8_TunerCtrl_EON_AF_listIndex];
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] EON Announcement station info received  %x EON PI and %d   freq of EON station  ",
													pst_EON_List->st_EON_PI_Info[pst_EON_List->u8_PI_Count]. u16_EON_PI ,
													st_AMFM_TunerCtrl_Eon_Info.u32_EON_aflist[u8_TunerCtrl_EON_AF_listIndex]);

					pst_EON_List->st_EON_PI_Info[pst_EON_List->u8_PI_Count].u8_AF_Count++;
				}
				else
				{
					/*do nothing */
				}
			}
			pst_EON_List->u8_PI_Count++;
		}
		else
		{
			/*do nothing */
		}		
	}
	else
	{
		/*do nothing*/
	}


}

/*=====================================================================================*/
/*			void 	AMFM_App_Freq_Existance_Check								 	    				 		      */
/*=====================================================================================*/

Te_AF_Freq_Availabilty_Check AMFM_App_Freq_Existance_Check(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Tu32 u32_AFfreq)
{
	Tu8 u8_AF_List_index = 0;
	Te_AF_Freq_Availabilty_Check e_AF_Freq_Availabilty_Check = AF_FREQUENCY_TO_BE_ADDED;
	
	for(u8_AF_List_index=0; u8_AF_List_index < pst_me_amfm_app_inst->st_current_station.st_Aflist.u8_NumAFList; u8_AF_List_index++ )
	{
		if(pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[ u8_AF_List_index] == u32_AFfreq )
		{
			e_AF_Freq_Availabilty_Check = AF_FREQUENCY_EXIST;
			break;
		}
	}
	return e_AF_Freq_Availabilty_Check;
}


void AMFM_App_frequency_Append(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerStatusInfo)
{
	Tu8 u8_loop_index=0;
	Te_AF_Freq_Availabilty_Check e_AF_Freq_Availabilty_Check = AF_FREQUENCY_INVALID;
	Tu8 u8_AF_List_index = pst_me_amfm_app_inst->st_current_station.st_Aflist.u8_NumAFList;
	
	for(u8_loop_index=0 ; (u8_loop_index < pst_TunerStatusInfo->u8_NumAFeqList ) && (pst_TunerStatusInfo->u32_AFeqList[u8_loop_index] != 0) ; u8_loop_index++)
	{
		e_AF_Freq_Availabilty_Check = AMFM_App_Freq_Existance_Check(pst_me_amfm_app_inst,pst_TunerStatusInfo->u32_AFeqList[u8_loop_index]);
		if(e_AF_Freq_Availabilty_Check == AF_FREQUENCY_TO_BE_ADDED)
		{
			if(( u8_AF_List_index < AMFM_APP_MAX_NO_AF_STATIONS - 1) && (pst_TunerStatusInfo->u32_AFeqList[u8_loop_index] != pst_me_amfm_app_inst->u32_ReqFrequency ))
			{
				pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[ u8_AF_List_index++ ] = pst_TunerStatusInfo->u32_AFeqList[u8_loop_index];
			}
			else
			{ 
				/*AF list full */
				break;
			}
		}
		pst_me_amfm_app_inst->st_current_station.st_Aflist.u8_NumAFList = u8_AF_List_index; 
	}

	SYS_RADIO_MEMCPY(&(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList),(const void *)(&(pst_me_amfm_app_inst->st_current_station.st_Aflist)),sizeof(pst_me_amfm_app_inst->st_current_station.st_Aflist));
}


/*New strategy*/

/*=====================================================================================*/
/*			void 	AMFM_App_Freq_Existance_Check_New      				 		       */
/*=====================================================================================*/
Te_AF_Freq_Availabilty_Check AMFM_App_Freq_Existance_Check_New(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AFfreq)
{
	Tu8 u8_AF_List_index = 0;
	Te_AF_Freq_Availabilty_Check e_AF_Freq_Availabilty_Check = AF_FREQUENCY_TO_BE_ADDED;
	
	for(u8_AF_List_index=0; u8_AF_List_index < pst_AMFM_App_AFList_Info->u8_NumAFList; u8_AF_List_index++ )
	{
		if(pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[ u8_AF_List_index].u32_AF_Freq == u32_AFfreq )
		{
			e_AF_Freq_Availabilty_Check = AF_FREQUENCY_EXIST;
			break;
		}
	}
	return e_AF_Freq_Availabilty_Check;
}

void AMFM_App_frequency_Append_New(Tu32 au32_AFeqList[],Tu8 u8_NumAFeqList,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
	Tu8 u8_loop_index=0;
	Te_AF_Freq_Availabilty_Check e_AF_Freq_Availabilty_Check = AF_FREQUENCY_INVALID;
	Tu16 u16_PI;
	Tu8 u8_AF_List_index = pst_AMFM_App_AFList_Info->u8_NumAFList;
	Te_AMFM_App_AF_PI_STATUS e_AF_PI_STATUS ;
	
	for(u8_loop_index=0 ; (u8_loop_index < u8_NumAFeqList ) && (au32_AFeqList[u8_loop_index] != 0) ; u8_loop_index++)
	{
		e_AF_Freq_Availabilty_Check = AMFM_App_Freq_Existance_Check_New(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index]);
		if(e_AF_Freq_Availabilty_Check == AF_FREQUENCY_TO_BE_ADDED)
		{
			if( u8_AF_List_index < AMFM_APP_MAX_NO_AF_STATIONS - 1)
			{
				if(au32_AFeqList[u8_loop_index] != (pst_AMFM_App_AFList_Info->u32_curr_freq ))
				{
					u16_PI = AMFM_App_Learn_memory_PI_Read(au32_AFeqList[u8_loop_index],pst_AF_Learn_mem);
					if(u16_PI == 0 )
					{
						e_AF_PI_STATUS=AMFM_APP_PI_STATUS_ZERO;
					}
					else if( u16_PI == pst_AMFM_App_AFList_Info->u16_curr_PI  )
					{
						e_AF_PI_STATUS=AMFM_APP_PI_STATUS_SAME;
					}
					else
					{
						e_AF_PI_STATUS=AMFM_APP_PI_STATUS_NEG;
					}
		
					pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_List_index].u32_AF_Freq	  = au32_AFeqList[u8_loop_index];
					pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_List_index].e_AF_PI_STATUS = e_AF_PI_STATUS;
					pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_List_index].u16_PI		  = u16_PI;

					#ifdef CALIBRATION_TOOL
						 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_AF_List_index].u32_AF_Freq	= au32_AFeqList[u8_loop_index];
					 	 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_AF_List_index].u16_PI		    = u16_PI;
					 	 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_AF_List_index].e_AF_PI_STATUS = e_AF_PI_STATUS;
					#endif
					u8_AF_List_index++;	
				}
			}
			else
			{
				/*AF list full */
				break;
			}
		}
		pst_AMFM_App_AFList_Info->u8_NumAFList = u8_AF_List_index;
	}
		
	#ifdef CALIBRATION_TOOL
	st_Calib_AMFM_App_AFList_Info.u8_NumAFList = 	pst_AMFM_App_AFList_Info->u8_NumAFList;
	#endif
}


/*===========================================================================*/
/*void AMFM_App_Sort_AF_List(Ts_AMFM_App_AFList_Info st_AMFM_App_AFList_Info)*/
/*===========================================================================*/
void AMFM_App_Sort_AF_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_AFListIndex;
	Tu8 u8_outer_loop_index;
	Tu8 u8_AF_boundary_index = (Tu8)(pst_AMFM_App_AFList_Info->u8_NumAFList-1);
	Ts_AMFM_App_AFStation_Info st_temp_AF_station;
	for(u8_outer_loop_index=0;u8_outer_loop_index < u8_AF_boundary_index ;u8_outer_loop_index++)
	{
		for(u8_AFListIndex = 0; u8_AFListIndex < (u8_AF_boundary_index - u8_outer_loop_index ) ; u8_AFListIndex++)
		{
			if(pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].u32_avg_qual <= pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex+1].u32_avg_qual)
			{
				st_temp_AF_station	 												  = pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex];
				pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex]  =	pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex+1];
				pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex+1]=	st_temp_AF_station;
			}
		}
	}
	pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index=0;
}
							



/*===========================================================================*/
/*			Tu8 AMFM_APP_New_AF_Update				 						 */
/*===========================================================================*/

Tu8 AMFM_APP_New_AF_Update(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{			
	Tu8 u8_AFListIndex;
	Tu8 u8_ret_value=0;

	pst_AMFM_App_AFList_Info->u8_AF_index++;	
	u8_AFListIndex = pst_AMFM_App_AFList_Info->u8_AF_index;
	
	if(u8_AFListIndex  <  pst_AMFM_App_AFList_Info->u8_NumAFList)
	{
		/* updating the value of u32_af_update_newfreq */
		pst_AMFM_App_AFList_Info->u32_af_update_newfreq	= pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
		
		#ifdef CALIBRATION_TOOL
		 	st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq = pst_AMFM_App_AFList_Info->u32_af_update_newfreq;
			st_Calib_AMFM_App_AFList_Info.u8_AF_index = AMFM_App_Calib_get_index(pst_AMFM_App_AFList_Info,u8_AFListIndex );
		#endif
		/*request update for next AF */
		AMFM_Tuner_Ctrl_Request_AF_Update(pst_AMFM_App_AFList_Info->u32_af_update_newfreq);
		
	}
	else
	{
		u8_ret_value++;
	}
	return u8_ret_value;
}

/*===========================================================================*/
/*			void AMFM_APP_Best_Freq_AF_check				 				 */
/*===========================================================================*/

void AMFM_APP_Best_Freq_AF_check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{			
	Tu8 u8_Best_AF_station_Index;
	Tu8 u8_calib_Best_AF_station_Index;
	
	u8_Best_AF_station_Index = pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index;
	pst_AMFM_App_AFList_Info->b_af_check_flag = TRUE;
	pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_Best_AF_station_Index].b_af_check = TRUE;
	
	#ifdef CALIBRATION_TOOL
		u8_calib_Best_AF_station_Index =st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;
	 	st_Calib_AMFM_App_AFList_Info.b_af_check_flag = TRUE;
		st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_calib_Best_AF_station_Index].b_af_check=TRUE;
	#endif
	
	AMFM_Tuner_Ctrl_Request_FM_Check(pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
}


Tbool AMFM_App_Best_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tbool	b_return_val = TRUE;
	Tu8 u8_AF_list_index;
	Tu8 u8_Same_PI_index=0;
	Tu8 u8_zero_PI_index=0;
	Tbool 	b_Same_PI_Found = FALSE;
	Tbool 	b_Zero_PI_Found = FALSE;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_NumAFList; u8_AF_list_index++ )
	{
		if((pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_SAME) &&
			(pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_list_index].b_af_check == FALSE))
		{
			u8_Same_PI_index = u8_AF_list_index;
			b_Same_PI_Found = TRUE;
			break;
		}
	}
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_NumAFList; u8_AF_list_index++ )
	{
		if((pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_ZERO)&&
			(pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_list_index].b_af_check == FALSE))
		{
			u8_zero_PI_index = u8_AF_list_index;
			b_Zero_PI_Found = TRUE;
			break;
		}
	}
	
	if ((b_Same_PI_Found != FALSE) || (b_Zero_PI_Found != FALSE))
	{
		if ((b_Same_PI_Found != FALSE) && (b_Zero_PI_Found != FALSE))
		{
			u8_AF_list_index = (u8_Same_PI_index < u8_zero_PI_index)? u8_Same_PI_index : u8_zero_PI_index;
		}
		else if (b_Same_PI_Found != FALSE)
		{
			u8_AF_list_index = u8_Same_PI_index;
		}
		else 									/*(u8_Zero_PI_Found != 0)*/
		{
			u8_AF_list_index = u8_zero_PI_index;
		}
	
		if((pst_AMFM_App_AFList_Info->u32_Qua_avg < pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_list_index].u32_avg_qual)
		   && ((pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_list_index].u32_avg_qual - pst_AMFM_App_AFList_Info->u32_Qua_avg ) > u8_Delta_Difference))
		{
			b_return_val = FALSE;
			pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index=u8_AF_list_index;
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index = AMFM_App_Calib_get_index(pst_AMFM_App_AFList_Info,u8_AF_list_index );
			#endif
		}
		else
		{
			b_return_val = TRUE;
		}
		
	}
	
	return b_return_val;
}

void AMFM_App_AFTune_Restart(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_AFListIndex;
	AMFM_App_Flag_Reset(pst_AMFM_App_AFList_Info);
	u8_AFListIndex = pst_AMFM_App_AFList_Info->u8_AF_index;
	for(;u8_AFListIndex < pst_AMFM_App_AFList_Info->u8_NumAFList; u8_AFListIndex++)
	{
		pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].b_af_check=FALSE;
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_AFListIndex].b_af_check=FALSE;
		#endif
		
	}
	u8_AFListIndex=pst_AMFM_App_AFList_Info->u8_AF_index;
	pst_AMFM_App_AFList_Info->u32_af_update_newfreq=pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
	
	#ifdef CALIBRATION_TOOL
		 	st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq = pst_AMFM_App_AFList_Info->u32_af_update_newfreq;
			st_Calib_AMFM_App_AFList_Info.u8_AF_index = AMFM_App_Calib_get_index(pst_AMFM_App_AFList_Info,u8_AFListIndex );
	#endif
	
	AMFM_Tuner_Ctrl_Request_AF_Update(pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq);
	
		
}	

void AMFM_App_Flag_Reset(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_AFListIndex=0;
	pst_AMFM_App_AFList_Info->u8_AF_index				=0;	
	pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index  =0;	
	pst_AMFM_App_AFList_Info->u32_af_update_newfreq		=0;			
	pst_AMFM_App_AFList_Info->b_af_check_flag			= FALSE;
	pst_AMFM_App_AFList_Info->b_quality_degraded		= FALSE;
	pst_AMFM_App_AFList_Info->u8_AF_Update_Index=0;

	#ifdef CALIBRATION_TOOL
		st_Calib_AMFM_App_AFList_Info.u8_AF_index				=0;	
		st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index  =0;	
		st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq		=0;			
		st_Calib_AMFM_App_AFList_Info.b_af_check_flag  			= FALSE;
		st_Calib_AMFM_App_AFList_Info.b_quality_degraded		= FALSE;
		st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index		=0;	
	
	#endif
	for(;u8_AFListIndex < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AFListIndex++)
	{
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AFListIndex].b_af_check=FALSE;
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AFListIndex].b_af_check=FALSE;
		#endif
	}
	for(u8_AFListIndex=0;u8_AFListIndex < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_AFListIndex++)
	{
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AFListIndex].b_af_check=FALSE;
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_AFListIndex].b_af_check=FALSE;
		#endif
	}
}
void  AMFM_App_AF_Qual_Avg_computation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_qual_incr_alpha,	Tu32 u32_AF_qual_decr_alpha)
{
	Tu8 u8_AF_list_index = pst_AMFM_App_AFList_Info -> u8_AF_index;
	#ifdef CALIBRATION_TOOL
		Tu8 u8_calib_AF_list_index = st_Calib_AMFM_App_AFList_Info.u8_AF_index ;
	#endif
	
	Tu32 u32_avg = pst_AMFM_App_AFList_Info -> ast_current_AFStation_Info[u8_AF_list_index].u32_avg_qual;
	Tu32 u32_old = pst_AMFM_App_AFList_Info -> ast_current_AFStation_Info[u8_AF_list_index].u32_old_qual;
	Tu32 u32_curr= pst_AMFM_App_AFList_Info -> ast_current_AFStation_Info[u8_AF_list_index].u32_curr_qual;
	
	if(u32_old == 0)
	{
		u32_avg = u32_curr;
	} 
	else
	{
		if( u32_curr >= u32_old )
		{
			u32_avg =(((u32_avg * u32_AF_qual_incr_alpha) + ((u8_alpha_div_factor - u32_AF_qual_incr_alpha)*u32_curr))/u8_alpha_div_factor);
		}
		else
		{
			u32_avg =(((u32_avg * u32_AF_qual_decr_alpha) + ((u8_alpha_div_factor - u32_AF_qual_decr_alpha)*u32_curr))/u8_alpha_div_factor);
		}
	}
	u32_old = u32_curr;
	pst_AMFM_App_AFList_Info -> ast_current_AFStation_Info[u8_AF_list_index].u32_avg_qual =  u32_avg;
	pst_AMFM_App_AFList_Info -> ast_current_AFStation_Info[u8_AF_list_index].u32_old_qual =	u32_old;
	pst_AMFM_App_AFList_Info -> ast_current_AFStation_Info[u8_AF_list_index].u32_curr_qual= 	u32_curr;
	
	#ifdef CALIBRATION_TOOL
		
		st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_calib_AF_list_index].u32_avg_qual =  u32_avg;
		st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_calib_AF_list_index].u32_old_qual =	u32_old;
		st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_calib_AF_list_index].u32_curr_qual= 	u32_curr;
	
	#endif
	
}

void AMFM_App_Current_Qual_Avg_Computation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu32 u32_Qua_avg = pst_AMFM_App_AFList_Info -> u32_Qua_avg;
	Tu32 u32_Qua_old = pst_AMFM_App_AFList_Info -> u32_Qua_old;
	Tu32 u32_Qua_curr= pst_AMFM_App_AFList_Info -> u32_Qua_curr;

	pst_AMFM_App_AFList_Info ->u32_Qua_old_avg =u32_Qua_avg;
	
	if(u32_Qua_old == 0)
	{
		u32_Qua_avg=u32_Qua_curr;
	}
	else
	{
		if(u32_Qua_curr >= u32_Qua_old)
		{
			u32_Qua_avg = (((u32_Qua_avg * u8_qual_incr_alpha) + ((u8_alpha_div_factor - u8_qual_incr_alpha)*u32_Qua_curr))/u8_alpha_div_factor);
		}
		else
		{
			u32_Qua_avg =(((u32_Qua_avg * u8_qual_decr_alpha) + ((u8_alpha_div_factor - u8_qual_decr_alpha)*u32_Qua_curr))/u8_alpha_div_factor);
		}
	}
	u32_Qua_old=u32_Qua_curr;

	pst_AMFM_App_AFList_Info -> u32_Qua_avg  = u32_Qua_avg;
	pst_AMFM_App_AFList_Info -> u32_Qua_old  = u32_Qua_old ;
	pst_AMFM_App_AFList_Info -> u32_Qua_curr = u32_Qua_curr;
	
	#ifdef CALIBRATION_TOOL
		st_Calib_AMFM_App_AFList_Info.u32_Qua_old_avg  	= st_Calib_AMFM_App_AFList_Info.u32_Qua_avg;
		st_Calib_AMFM_App_AFList_Info.u32_Qua_avg  		= u32_Qua_avg;
		st_Calib_AMFM_App_AFList_Info.u32_Qua_old  		= u32_Qua_old;
		st_Calib_AMFM_App_AFList_Info.u32_Qua_curr 		= u32_Qua_curr;
	#endif
}


void AMFM_App_Eon_Station_Qual_Avg_Computation(Ts_AMFM_App_EON_List *pst_EON_List)
{
	Tu8 u8_EONlist_PI_index 		= pst_EON_List->u8_EONlist_PIindex;
	//Tu8 u8_EON_AF_List_Index	= pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u8_TA_AF_ListIndex;
	
	Tu32 u32_Qua_avg = pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg;
	Tu32 u32_Qua_old = pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_old;
	Tu32 u32_Qua_curr= pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_curr;

	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_old_avg =u32_Qua_avg;
	
	if(u32_Qua_old == 0)
	{
		u32_Qua_avg=u32_Qua_curr;
		
	}
	else
	{
		if(u32_Qua_curr >= u32_Qua_old)
		{
			u32_Qua_avg =(((u32_Qua_avg * u8_qual_incr_alpha) + ((u8_alpha_div_factor - u8_qual_incr_alpha)*u32_Qua_curr))/u8_alpha_div_factor);
		}
		else
		{
			u32_Qua_avg =(((u32_Qua_avg * u8_qual_decr_alpha) + ((u8_alpha_div_factor - u8_qual_decr_alpha)*u32_Qua_curr))/u8_alpha_div_factor);
		}
	}
	u32_Qua_old=u32_Qua_curr;

	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg  = u32_Qua_avg;
	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_old  = u32_Qua_old ;
	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_curr = u32_Qua_curr;

}


#ifdef CALIBRATION_TOOL
void AMFM_App_Calib_Tune_Qual_Avg_Computation(void)
{
	if(u32_cal_old == 0)
	{
		u32_cal_avg=u32_cal_curr;
	}
	else
	{
		if(u32_cal_curr >= u32_cal_old)
		{
			u32_cal_avg =(((u32_cal_avg * u8_qual_incr_alpha) + ((u8_alpha_div_factor - u8_qual_incr_alpha)*u32_cal_curr))/u8_alpha_div_factor);
		}
		else
		{
			u32_cal_avg =(((u32_cal_avg * u8_qual_decr_alpha) + ((u8_alpha_div_factor - u8_qual_decr_alpha)*u32_cal_curr))/u8_alpha_div_factor);
		}
	}
	u32_cal_old=u32_cal_curr;
	
}


Tu8 AMFM_App_Calib_get_index(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info,Tu8 u8_AFListIndex)
{
	//Tu16 u32_AF_Freq = pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
	Tu8 u8_calib_index=0;
	if(u8_AFListIndex < 0x19)
	{
		//u32_AF_Freq = pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
		for(;u8_calib_index < st_Calib_AMFM_App_AFList_Info.u8_NumAFList;u8_calib_index++)
		{
			if(pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq == st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[u8_calib_index].u32_AF_Freq)
			{
				break;
			}
		}
	}
	else
	{
		
	}		
	return u8_calib_index;
	
}

#endif



void AMFM_App_PI_Status_Update(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Te_AMFM_App_AF_PI_STATUS e_AF_PI_STATUS ;
	Tu8 u8_AF_List_index;
	//Tu8 u8_loop_index;
	Tu16 u16_PI;	
	for(u8_AF_List_index=0 ; u8_AF_List_index < pst_AMFM_App_AFList_Info->u8_NumAFList ; u8_AF_List_index++)
	{	
		u16_PI = pst_AMFM_App_AFList_Info -> ast_current_AFStation_Info[u8_AF_List_index].u16_PI;
		
		if(u16_PI == 0 )
		{
			e_AF_PI_STATUS=AMFM_APP_PI_STATUS_ZERO;
			
		}
		else if( u16_PI == pst_AMFM_App_AFList_Info->u16_curr_PI  )
		{
			e_AF_PI_STATUS=AMFM_APP_PI_STATUS_SAME;
		}
	
		else
		{
			e_AF_PI_STATUS=AMFM_APP_PI_STATUS_NEG;
		}
		pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_List_index].e_AF_PI_STATUS = e_AF_PI_STATUS;
	}
}	

void AMFM_App_AF_Append_From_learn_Memory(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
	Tu8 u8_AF_List_index = pst_AMFM_App_AFList_Info->u8_NumAFList;
	Tu32 u32_loop_index=0;
	Te_AF_Freq_Availabilty_Check e_AF_Freq_Availabilty_Check = AF_FREQUENCY_INVALID;
	
	for(u32_loop_index=0 ; (u32_loop_index < AMFM_APP_NO_STATIONS_FM_BAND)&& (u8_AF_List_index < AMFM_APP_MAX_NO_AF_STATIONS ) ; u32_loop_index++)
	{
		if((pst_AF_Learn_mem[u32_loop_index].u16_PI == pst_AMFM_App_AFList_Info->u16_curr_PI)&&
		   (pst_AF_Learn_mem[u32_loop_index].u32_frequency !=((pst_AMFM_App_AFList_Info->u32_curr_freq)))&&
		   (pst_AF_Learn_mem[u32_loop_index].u32_frequency !=0))
		{
			e_AF_Freq_Availabilty_Check = AMFM_App_Freq_Existance_Check_New(pst_AMFM_App_AFList_Info,pst_AF_Learn_mem[u32_loop_index].u32_frequency);
			if(e_AF_Freq_Availabilty_Check == AF_FREQUENCY_TO_BE_ADDED)
			{
			
				pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_List_index].u32_AF_Freq	  = pst_AF_Learn_mem[u32_loop_index].u32_frequency;
				pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_List_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
				pst_AMFM_App_AFList_Info->ast_current_AFStation_Info[u8_AF_List_index].u16_PI		  = pst_AF_Learn_mem[u32_loop_index].u16_PI;

				
				pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[u8_AF_List_index] = pst_AF_Learn_mem[u32_loop_index].u32_frequency;
				
				#ifdef CALIBRATION_TOOL
					 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_AF_List_index].u32_AF_Freq	= pst_AF_Learn_mem[u32_loop_index].u32_frequency;
					 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_AF_List_index].u16_PI			= pst_AF_Learn_mem[u32_loop_index].u16_PI;
					 st_Calib_AMFM_App_AFList_Info.ast_current_AFStation_Info[ u8_AF_List_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
				#endif
				u8_AF_List_index++;	
			}
			
		}
		pst_AMFM_App_AFList_Info->u8_NumAFList = u8_AF_List_index;
		pst_me_amfm_app_inst->st_current_station.st_Aflist.u8_NumAFList=u8_AF_List_index;
	}
	#ifdef CALIBRATION_TOOL
	st_Calib_AMFM_App_AFList_Info.u8_NumAFList = 	pst_AMFM_App_AFList_Info->u8_NumAFList;
	#endif

	SYS_RADIO_MEMCPY(&(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList),(const void *)(&(pst_me_amfm_app_inst->st_current_station.st_Aflist)),sizeof(pst_me_amfm_app_inst->st_current_station.st_Aflist));
	//memcpy(&(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList),&(pst_me_amfm_app_inst->st_current_station.st_Aflist),sizeof(pst_me_amfm_app_inst->st_current_station.st_Aflist));

}

/*
void AMFM_App_Learn_Memory_updation(Tu32 u32_freq,Tu16 u16_PI,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
	Tu8 u8_index = (Tu8)((u32_freq - u32_start_freq)/u32_StepSize);
	
	pst_AF_Learn_mem[u8_index].u16_PI = u16_PI;
}
*/


Tbool AMFM_App_Learn_Memory_updation(Tu32 u32_freq,Tu16 u16_PI,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
	Tu8 u8_index = (Tu8)((u32_freq - u32_start_freq)/u32_StepSize);
	if(pst_AF_Learn_mem[u8_index].u16_PI != u16_PI)
	{
		pst_AF_Learn_mem[u8_index].u16_PI = u16_PI;
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_APP]  LM Updated freq=%d PI = %x",u32_freq,u16_PI);
		return TRUE;
	}
	else
	{
		return FALSE;
	}	
}
Tu16 AMFM_App_Learn_memory_PI_Read( Tu32 u32_freq,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
	
	Tu8 u8_index = (Tu8)((u32_freq - u32_start_freq)/u32_StepSize);
	
	return pst_AF_Learn_mem[u8_index].u16_PI;

}

void AMFM_App_Update_LM_Freq(Ts_AMFM_App_FreqInfo *pst_FMband_FreqInfo,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem)
{
    Tu32    u32_loop_index = 0;
    Tu32 u32_StartFreq;
//  Tu32 u32_End_Freq;
//  Tu32 u32_StepSize;
    
    if(pst_FMband_FreqInfo != NULL)
    {
         u32_StartFreq	   =    (pst_FMband_FreqInfo->u32_StartFreq );
//       u32_End_Freq     =    (pst_FMband_FreqInfo->u32_EndFreq   );
//       u32_StepSize     =   (pst_FMband_FreqInfo->u32_StepSize  );
        
        while(( u32_StartFreq <= u32_End_Freq) && (u32_loop_index < AMFM_APP_NO_STATIONS_FM_BAND))
        {
            pst_AF_Learn_mem[u32_loop_index].u32_frequency =  u32_StartFreq;
            u32_loop_index++;
             u32_StartFreq = ( u32_StartFreq + u32_StepSize);
        }        
    }
    else
    {
        /* Error */
    }            
}
void AMFM_App_ReadQuality(Ts_AMFM_App_StationInfo	*pst_StationInfo,Ts_AMFM_Tuner_Ctrl_CurrStationInfo  *pst_TunerctrlCurrentStationInfo)
{
	/* Updating parameters related to Quality */

	pst_StationInfo->s32_RFFieldStrength	= pst_TunerctrlCurrentStationInfo->s32_RFFieldStrength;

	pst_StationInfo->s32_BBFieldStrength	= pst_TunerctrlCurrentStationInfo->s32_BBFieldStrength;

	pst_StationInfo->u32_UltrasonicNoise	= pst_TunerctrlCurrentStationInfo->u32_UltrasonicNoise;

	pst_StationInfo->u32_Multipath		    = pst_TunerctrlCurrentStationInfo->u32_Multipath;

	pst_StationInfo->u8_Status	= pst_TunerctrlCurrentStationInfo->u8_status;

	pst_StationInfo->u32_FrequencyOffset	= pst_TunerctrlCurrentStationInfo->u32_FrequencyOffset;


	pst_StationInfo->u32_ModulationDetector = pst_TunerctrlCurrentStationInfo->u32_ModulationDetector;

	pst_StationInfo->u32_AdjacentChannel	= pst_TunerctrlCurrentStationInfo->u32_AdjacentChannel;

	pst_StationInfo->u32_coChannel			= pst_TunerctrlCurrentStationInfo->u32_coChannel;

	pst_StationInfo->u32_SNR				= pst_TunerctrlCurrentStationInfo->u32_SNR;

	pst_StationInfo->u32_StereoMonoReception = pst_TunerctrlCurrentStationInfo->u32_StereoMonoReception;

}					

/*=====================================================================================*/
/*			Tu8 	AMFM_App_PSN_RT_Copy								 	           */
/*=====================================================================================*/	
Tu8 AMFM_App_PSN_RT_Copy(Ts_AMFM_App_StationInfo *pst_current_station,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo)
{	
	Tu8		u8_string_len			= 	0;	
	Ts32	s32_Strcmp_retValue;                                       /*variable to hold the return value by strcmp function*/
	Tu8		u8_retValue = 0;							    /*   In this variable , b0 bit is set if PSN is different
																		b1 bit is set if RT   is different  */	
	u8_string_len = AMFM_APP_String_length(pst_TunerctrlCurrentStationInfo->au8_ps,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME); 
	if(u8_string_len !=AMFM_APP_CONSTANT_ZERO )
	{
		s32_Strcmp_retValue = AMFM_App_String_comparison(pst_TunerctrlCurrentStationInfo->au8_ps,pst_current_station->un_station.st_FMstation.au8_psn,MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

		if(s32_Strcmp_retValue != 0) 
		{
			/* Copying the PSN to st_current_station structure present in Ts_AMFM_App_inst_hsm structure */	
			SYS_RADIO_MEMCPY((pst_current_station->un_station.st_FMstation.au8_psn),(const void *)(pst_TunerctrlCurrentStationInfo->au8_ps),MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME);

			u8_retValue	= u8_retValue | 0x01;
		}	
	}
	else
	{	
		/*MISRA C*/
	}
	u8_string_len = AMFM_APP_String_length(pst_TunerctrlCurrentStationInfo->au8_RadioText,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT); 
	if(u8_string_len !=AMFM_APP_CONSTANT_ZERO)
	{
		s32_Strcmp_retValue = AMFM_App_String_comparison(pst_TunerctrlCurrentStationInfo->au8_RadioText,pst_current_station->au8_RadioText,AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT);

		if(s32_Strcmp_retValue != 0) 
		{
			/* Copying the RadioText to st_current_station structure present in Ts_AMFM_App_inst_hsm structure */
			SYS_RADIO_MEMCPY((pst_current_station->au8_RadioText), (const void *)(pst_TunerctrlCurrentStationInfo->au8_RadioText), pst_TunerctrlCurrentStationInfo->u8_RT_size);

			u8_retValue	= u8_retValue | 0x02;
		}	
	}
	else
	{	
		/*MISRA C*/
	}	
	return u8_retValue;
}
/*===========================================================================*/
/*			AMFM_App_Read_TA_TP_info										 */	
/*===========================================================================*/
void AMFM_App_Read_TA_TP_info(Ts_AMFM_App_StationInfo *pst_current_station, Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo)
{
	pst_current_station->u8_TA = pst_TunerctrlCurrentStationInfo->u8_TA;
	pst_current_station->u8_TP = pst_TunerctrlCurrentStationInfo->u8_TP;
}


/*===========================================================================*/
/*			void AMFM_App_PTY_Copy				 				 */
/*===========================================================================*/
void AMFM_App_PTY_Copy(Ts_AMFM_App_StationInfo *pst_current_station,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo)
{

	Tu16 prgname_len = 0;
						 
	#if EIGHTCHARDISPLAY

     Tchar Prgmtype_Name[AMFM_APP_MAX_NO_OF_PGRMS][AMFM_APP_MAX_SIZE_PRGRM_NAME] = {"None","News","Affairs","Info","Sport","Educate","Drama","Culture","Science","Varied",
																					"Pop M","Rock M","Easy M","Light M","Classics","Other M","Weather","Finance","Children",
																					"Social","Religion","Phone In","Travel","Leisure","Jazz","Country","Nation M","Oldies",
																					"Folk M","Document","TEST","Alarm !"};
													 
	

   #else
   
    /*if HMI display is 16 characters then AMFM_APP_MAX_SIZE_PRGRM_NAME is 17*/
    Tchar Prgmtype_Name[AMFM_APP_MAX_NO_OF_PGRMS][AMFM_APP_MAX_SIZE_PRGRM_NAME] = {"None","News","Current Affairs","Information","Sport","Education","Drama","Cultures","Science",
																					"Varied Speech","Pop Music","Rock Music","Easy listening","Light Classics M","Serious Classics",
																					"Other Music","Weather & Metr","Finance","Childrens & Prog","Social Affairs","Religion","Phone In",
																					"Travel & Touring","Leisure & Hobby","Jazz Music","Country Music","National Music","Oldies Music",
																					"Folk Music","Documententary","Alarm TEST","Alarm - Alarm!"};


   #endif 
													 
	pst_current_station -> u8_PTY_Code  = pst_TunerctrlCurrentStationInfo -> u8_PTYCode;												 												 
	prgname_len = strlen(Prgmtype_Name[pst_current_station -> u8_PTY_Code]);
	memset((pst_current_station->au8_PTY_Name),0x00,sizeof(pst_current_station->au8_PTY_Name));		
	SYS_RADIO_MEMCPY((pst_current_station->au8_PTY_Name),(Prgmtype_Name[pst_current_station -> u8_PTY_Code]),prgname_len);		
			
}
/* Regional Additions*/

Te_AMFM_App_Regional_PI_Check AMFM_App_Regional_Pi_Validation(Tu16 u16_curr_station_PI, Tu16 u16_reff_station_PI)
{
	Tu8 u8_Curr_Sta_Masked_PI = LIB_MASK_AND_SHIFT(u16_curr_station_PI, (Tu16)0x0f00, 8u);
	Tu8 u8_Reff_Sta_Masked_PI = LIB_MASK_AND_SHIFT(u16_reff_station_PI, (Tu16)0x0f00, 8u);
	Te_AMFM_App_Regional_PI_Check e_Regional_PI_Check = REGIONAL_PI_NON_COMPITABLE;
	
	if(( u8_Curr_Sta_Masked_PI != 0) && (u8_Reff_Sta_Masked_PI != 0))
	{
		/* assuring both PI area code is not Local(0) */
		if(( u8_Curr_Sta_Masked_PI > 3) || (u8_Reff_Sta_Masked_PI > 3))
		{
			e_Regional_PI_Check = REGIONAL_PI_COMPITABLE;
		}
	}
	return e_Regional_PI_Check;
}

Te_AF_Freq_Availabilty_Check  AMFM_App_Freq_Duplication_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AFfreq)
{
	Tu8 u8_AF_List_index = 0;
	Te_AF_Freq_Availabilty_Check e_AF_Freq_Availabilty_Check = AF_FREQUENCY_TO_BE_ADDED;
	
	for(u8_AF_List_index=0; u8_AF_List_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_List_index++ )
	{
		if(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[ u8_AF_List_index].u32_AF_Freq == u32_AFfreq )
		{
			e_AF_Freq_Availabilty_Check = AF_FREQUENCY_EXIST;
			break;
		}
	}
	if(e_AF_Freq_Availabilty_Check != AF_FREQUENCY_EXIST )
	{
		for(u8_AF_List_index=0; u8_AF_List_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_AF_List_index++ )
		{
			if(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[ u8_AF_List_index].u32_AF_Freq == u32_AFfreq )
			{
				e_AF_Freq_Availabilty_Check = AF_FREQUENCY_EXIST;
				break;
			}
		}
	}	
	return e_AF_Freq_Availabilty_Check;
}
void AMFM_App_Same_PI_List_Freq_Append(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_Freq,Tu16 u16_PI,Te_AMFM_App_AF_PI_STATUS e_AF_PI_STATUS)
{
	Tu8 u8_Same_PI_AF_List_index = pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList;
	
	pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_Same_PI_AF_List_index].u32_AF_Freq	  = u32_AF_Freq;
	pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_Same_PI_AF_List_index].e_AF_PI_STATUS = e_AF_PI_STATUS;
	pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_Same_PI_AF_List_index].u16_PI		  = u16_PI;
	(pst_AMFM_App_AFList_Info->u8_NumAFList)++;
	(pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList)++ ;

	#ifdef CALIBRATION_TOOL
		st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[ u8_Same_PI_AF_List_index].u32_AF_Freq	   = u32_AF_Freq;
		st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[ u8_Same_PI_AF_List_index].e_AF_PI_STATUS = e_AF_PI_STATUS;
		st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[ u8_Same_PI_AF_List_index].u16_PI		   = u16_PI;
		st_Calib_AMFM_App_AFList_Info.u8_NumAFList++;
		st_Calib_AMFM_App_AFList_Info.u8_Num_SAME_PI_AFList++ ;

	#endif
	
}

void AMFM_App_REG_PI_List_Freq_Append(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_Freq,Tu16 u16_PI,Te_AMFM_App_AF_PI_STATUS e_AF_PI_STATUS)
{
	Tu8 u8_REG_PI_AF_List_index  = pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList;
	
	pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_REG_PI_AF_List_index].u32_AF_Freq	  = u32_AF_Freq;
	pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_REG_PI_AF_List_index].e_AF_PI_STATUS  = e_AF_PI_STATUS;
	pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_REG_PI_AF_List_index].u16_PI		  = u16_PI;
	
	(pst_AMFM_App_AFList_Info->u8_NumAFList)++;
	(pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList)++ ;
	
	#ifdef CALIBRATION_TOOL
		st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[ u8_REG_PI_AF_List_index].u32_AF_Freq	   = u32_AF_Freq;
		st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[ u8_REG_PI_AF_List_index].e_AF_PI_STATUS   = e_AF_PI_STATUS;
		st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[ u8_REG_PI_AF_List_index].u16_PI		   = u16_PI;
		st_Calib_AMFM_App_AFList_Info.u8_NumAFList++;
		st_Calib_AMFM_App_AFList_Info.u8_Num_REG_PI_AFList++ ;
	#endif
}

/*===========================================================================*/
/*			Tu32  AMFM_APP_BG_Generate_AF_list_from_LM							 */
/*===========================================================================*/
void AMFM_APP_BG_Generate_AF_list_from_LM(Tu16 u16_PI,Ts_AMFM_App_FreqInfo	*pst_FMband_FreqInfo,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem *pst_AF_Learn_mem)
{
	Tu32	u32_loop_index		= 	0;
	Tu16	u16_Start_Freq		= 	(Tu16)(pst_FMband_FreqInfo->u32_StartFreq / (Tu32)10);
//	Tu16	u32_End_Freq		=	(Tu16)(pst_FMband_FreqInfo->u32_EndFreq  / (Tu32)10);
	Tu16	u16_Step_Size		=	(Tu16)(pst_FMband_FreqInfo->u32_StepSize / (Tu32)10);
	Tu16 	u16_LM_size			=   (u32_End_Freq- u16_Start_Freq)/u16_Step_Size ;

	for(u32_loop_index=0;(u32_loop_index <u16_LM_size) && (pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList < AMFM_APP_MAX_NO_AF_STATIONS );u32_loop_index++)
	{
		if(u16_PI == pst_AF_Learn_mem[u32_loop_index].u16_PI)
		{
			if((pst_AF_Learn_mem[u32_loop_index].u32_frequency >=u16_Start_Freq) && (pst_AF_Learn_mem[u32_loop_index].u32_frequency <= u32_End_Freq))
			{
				AMFM_App_Same_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,pst_AF_Learn_mem[u32_loop_index].u32_frequency,u16_PI,AMFM_APP_PI_STATUS_SAME);
			}
			else
			{
				/* Invalid freq.Do check for next freq in the LM */
			}
		}
	}
}

Tbool AMFM_App_AF_List_Append_From_learn_Memory(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem *pst_AF_Learn_mem)
{
	Tu8 						  u8_Tot_AF_count   			= pst_AMFM_App_AFList_Info->u8_NumAFList;
	Tu32   						  u32_loop_index				= 0;
	Te_AF_Freq_Availabilty_Check  e_AF_Freq_Availabilty_Check 	= AF_FREQUENCY_INVALID;
	Te_AMFM_App_Regional_PI_Check e_Regional_PI_Check 			= REGIONAL_PI_NON_COMPITABLE;
	Tbool						  b_verfreq						= FALSE;   
	Tbool      					  b_freq_added	 				= FALSE;
	
	for(u32_loop_index=0 ; (u32_loop_index < AMFM_APP_NO_STATIONS_FM_BAND)&& (u8_Tot_AF_count < AMFM_APP_MAX_NO_AF_STATIONS ) ; u32_loop_index++)
	{
		b_verfreq = AMFM_APP_VerifyFrequency(((pst_AF_Learn_mem[u32_loop_index].u32_frequency)),&pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
		if(b_verfreq == TRUE)
		{
			if((pst_AF_Learn_mem[u32_loop_index].u32_frequency !=((pst_AMFM_App_AFList_Info->u32_curr_freq)))&&
			(pst_AF_Learn_mem[u32_loop_index].u32_frequency !=0))
			{
				if(pst_AF_Learn_mem[u32_loop_index].u16_PI == pst_AMFM_App_AFList_Info->u16_curr_PI)
				{
					e_AF_Freq_Availabilty_Check = AMFM_App_Freq_Duplication_Check(pst_AMFM_App_AFList_Info,pst_AF_Learn_mem[u32_loop_index].u32_frequency);
					if(e_AF_Freq_Availabilty_Check == AF_FREQUENCY_TO_BE_ADDED)
					{
						AMFM_App_Same_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,pst_AF_Learn_mem[u32_loop_index].u32_frequency,pst_AMFM_App_AFList_Info->u16_curr_PI,AMFM_APP_PI_STATUS_SAME);
						pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[u8_Tot_AF_count] = pst_AF_Learn_mem[u32_loop_index].u32_frequency;
						/*For eng mode*/
						pst_me_amfm_app_inst->st_ENG_AFList_Info.ast_ENG_AF_List[u8_Tot_AF_count].u32_AF_Freq	=pst_AF_Learn_mem[u32_loop_index].u32_frequency;
						pst_me_amfm_app_inst->st_ENG_AFList_Info.ast_ENG_AF_List[u8_Tot_AF_count].u16_PI		=pst_AF_Learn_mem[u32_loop_index].u16_PI;
						b_freq_added=TRUE;
						u8_Tot_AF_count++;	
					}
				}
				else if ((LIB_AND(pst_AF_Learn_mem[u32_loop_index].u16_PI, (Tu16)0xf0ff)) == (LIB_AND(pst_AMFM_App_AFList_Info->u16_curr_PI, (Tu16)0xf0ff)))
				{
					/* PI NOT SAME*/
					e_Regional_PI_Check = AMFM_App_Regional_Pi_Validation(pst_AMFM_App_AFList_Info->u16_curr_PI , pst_AF_Learn_mem[u32_loop_index].u16_PI);
					if(e_Regional_PI_Check == REGIONAL_PI_COMPITABLE)
					{
						e_AF_Freq_Availabilty_Check = AMFM_App_Freq_Duplication_Check(pst_AMFM_App_AFList_Info,pst_AF_Learn_mem[u32_loop_index].u32_frequency);
						if(e_AF_Freq_Availabilty_Check == AF_FREQUENCY_TO_BE_ADDED)
						{
						
							AMFM_App_REG_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,pst_AF_Learn_mem[u32_loop_index].u32_frequency,pst_AF_Learn_mem[u32_loop_index].u16_PI,AMFM_APP_PI_STATUS_REG);
							pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[u8_Tot_AF_count] = pst_AF_Learn_mem[u32_loop_index].u32_frequency;
							/*For eng mode*/
							pst_me_amfm_app_inst->st_ENG_AFList_Info.ast_ENG_AF_List[u8_Tot_AF_count].u32_AF_Freq	=pst_AF_Learn_mem[u32_loop_index].u32_frequency;
							pst_me_amfm_app_inst->st_ENG_AFList_Info.ast_ENG_AF_List[u8_Tot_AF_count].u16_PI		=pst_AF_Learn_mem[u32_loop_index].u16_PI;
							b_freq_added = TRUE;
							u8_Tot_AF_count++;	
						}
					}
				}
				else
				{
					/*nothing to do */
				}
			}
			pst_me_amfm_app_inst->st_current_station.st_Aflist.u8_NumAFList=u8_Tot_AF_count;
			/*For eng mode*/
			pst_me_amfm_app_inst->st_ENG_AFList_Info.u8_NumAFList=u8_Tot_AF_count;
					
		}
	}
	if(b_freq_added == TRUE)
	{
		SYS_RADIO_MEMCPY(&(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList),(const void *)(&(pst_me_amfm_app_inst->st_current_station.st_Aflist)),sizeof(pst_me_amfm_app_inst->st_current_station.st_Aflist));
		AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,pst_AMFM_App_AFList_Info,AMFM_APP_AF_FREQ_APPENDED_FROM_LEARN_MEMORY);
	}
	return b_freq_added;
}

Tbool AMFM_App_AF_List_frequency_Append(Tu32 au32_AFeqList[],Tu8 u8_Num_Same_PI_AFeqList,Tu8 u8_Num_reg_PI_AFeqList,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst)
{
	Tu16						 	u16_PI						 = 0;
	Tu8 							u8_loop_index				 = 0;
	Tu8							 	u8_NumAFeqList				 = u8_Num_Same_PI_AFeqList  + u8_Num_reg_PI_AFeqList;
	Tu8							 	u8_Tot_AF_count 	         = pst_AMFM_App_AFList_Info->u8_NumAFList;
	Tbool							b_freq_added				 = FALSE;
	Te_AMFM_App_Regional_PI_Check 	e_Regional_PI_Check		  	 = REGIONAL_PI_NON_COMPITABLE;
	Te_AF_Freq_Availabilty_Check 	e_AF_Freq_Availabilty_Check  = AF_FREQUENCY_INVALID;
	Tbool							b_verfreq					 = FALSE;   
	
	for(u8_loop_index=0 ; (u8_loop_index < u8_NumAFeqList )  ; u8_loop_index++)
	{
		if( (u8_Tot_AF_count < AMFM_APP_MAX_NO_AF_STATIONS) && (au32_AFeqList[u8_loop_index] != 0) )
		{
			b_verfreq = AMFM_APP_VerifyFrequency((au32_AFeqList[u8_loop_index]*10),&pst_me_amfm_app_inst->e_requested_mode,pst_me_amfm_app_inst->e_MarketType ,&pst_me_amfm_app_inst->st_MarketInfo ) ;
			if(b_verfreq == TRUE)
			{
				if(au32_AFeqList[u8_loop_index] != (pst_AMFM_App_AFList_Info->u32_curr_freq))
				{
					e_AF_Freq_Availabilty_Check = AMFM_App_Freq_Duplication_Check(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index]);
					if(e_AF_Freq_Availabilty_Check == AF_FREQUENCY_TO_BE_ADDED)
					{
						b_freq_added = TRUE;
					
						pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[u8_Tot_AF_count] = au32_AFeqList[u8_loop_index];
						/*For eng mode*/
						pst_me_amfm_app_inst->st_ENG_AFList_Info.ast_ENG_AF_List[u8_Tot_AF_count].u32_AF_Freq	= au32_AFeqList[u8_loop_index];
						
						u16_PI = AMFM_App_Learn_memory_PI_Read(au32_AFeqList[u8_loop_index],pst_me_amfm_app_inst->ast_AF_Learn_mem);
					
						/*For eng mode*/
						pst_me_amfm_app_inst->st_ENG_AFList_Info.ast_ENG_AF_List[u8_Tot_AF_count].u16_PI		= u16_PI;
					
						u8_Tot_AF_count++;	
						if(u16_PI == 0 )
						{	
							/*PI UNDETERMINED YET*/
							if(u8_loop_index < u8_Num_Same_PI_AFeqList)
							{
								AMFM_App_Same_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_SAME);
							}
							else
							{
								AMFM_App_REG_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_REG);
							}
						}
						else if( u16_PI == pst_AMFM_App_AFList_Info->u16_curr_PI  )
						{
							/*SAME PI*/
							AMFM_App_Same_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_SAME);
						}
						else if ((LIB_AND(u16_PI, (Tu16)0xf0ff)) == (LIB_AND(pst_AMFM_App_AFList_Info->u16_curr_PI, (Tu16)0xf0ff)))
						{
							/* PI MIGHT BE REGIONAL*/
							e_Regional_PI_Check = AMFM_App_Regional_Pi_Validation(pst_AMFM_App_AFList_Info->u16_curr_PI , u16_PI);
							if(e_Regional_PI_Check == REGIONAL_PI_COMPITABLE)
							{
								/*REGIONAL PI*/
								AMFM_App_REG_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_REG);
							}
							else
							{
								/*NOT REGIONAL COMPITABLE*/
								if(u8_loop_index < u8_Num_Same_PI_AFeqList)
								{
									AMFM_App_Same_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_NEG);
								}
								else
								{
									AMFM_App_REG_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_NEG);
								}
							}
						}
						else
						{
							/*Totally diff PI*/
							if(u8_loop_index < u8_Num_Same_PI_AFeqList)
							{
								AMFM_App_Same_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_NEG);
							}
							else
							{
								AMFM_App_REG_PI_List_Freq_Append(pst_AMFM_App_AFList_Info,au32_AFeqList[u8_loop_index],u16_PI,AMFM_APP_PI_STATUS_NEG);
							}
						}
					}
				}
			}
		}
		else
		{
			/*AF list full */
			break;
		}
		pst_me_amfm_app_inst->st_current_station.st_Aflist.u8_NumAFList	= u8_Tot_AF_count;
		/*For eng mode*/
		pst_me_amfm_app_inst->st_ENG_AFList_Info.u8_NumAFList			= u8_Tot_AF_count;
					
	}
	if(b_freq_added == TRUE)
	{
		SYS_RADIO_MEMCPY(&(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList),(const void *)(&(pst_me_amfm_app_inst->st_current_station.st_Aflist)),sizeof(pst_me_amfm_app_inst->st_current_station.st_Aflist));
		AMFM_App_RadioDebugLogPrint(pst_me_amfm_app_inst,pst_AMFM_App_AFList_Info,AMFM_APP_AF_FREQ_APPENDED);
	}
	
	return b_freq_added;
}



Tbool AMFM_APP_AF_Update(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{			
	Tu8 u8_AFListIndex;
	Tbool 	b_ret_value = FALSE;

	pst_AMFM_App_AFList_Info->u8_AF_index++;	
	u8_AFListIndex = pst_AMFM_App_AFList_Info->u8_AF_index;
	
	
	if(u8_AFListIndex  <  pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList)
	{	
		
		pst_AMFM_App_AFList_Info->pst_current_AFStation_Info = pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info;
		pst_AMFM_App_AFList_Info->u8_AF_Update_Index = pst_AMFM_App_AFList_Info->u8_AF_index;
		/* updating the value of u32_af_update_newfreq */
		pst_AMFM_App_AFList_Info->u32_af_update_newfreq	= pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
		
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info	= st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
			st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index			= AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,pst_AMFM_App_AFList_Info->u8_AF_Update_Index);
			st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index].u32_AF_Freq;
		#endif
		/*request update for next AF */
		AMFM_Tuner_Ctrl_Request_AF_Update(pst_AMFM_App_AFList_Info->u32_af_update_newfreq);
	}
	else if(u8_AFListIndex  <  (pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList + pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList))
	{
		pst_AMFM_App_AFList_Info->u8_AF_Update_Index = pst_AMFM_App_AFList_Info->u8_AF_index - pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList ;
		pst_AMFM_App_AFList_Info->pst_current_AFStation_Info = &(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[0]);
		
		u8_AFListIndex=pst_AMFM_App_AFList_Info->u8_AF_Update_Index;
		/* updating the value of u32_af_update_newfreq */
		pst_AMFM_App_AFList_Info->u32_af_update_newfreq	= pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
		
		
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info	 = &(st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[0]);
			st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index			 = AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList,u8_AFListIndex);
			st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index].u32_AF_Freq;
		#endif
		/*request update for next AF */
		AMFM_Tuner_Ctrl_Request_AF_Update(pst_AMFM_App_AFList_Info->u32_af_update_newfreq);
	
	}
	else
	{
		b_ret_value = TRUE;
	}

	return b_ret_value;
}


void AMFM_App_Sort_SAME_PI_AF_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Ts8 s8_AFListIndex;
	Ts8 s8_outer_loop_index;
	Ts8 s8_AF_boundary_index=0;
	Ts_AMFM_App_AFStation_Info st_temp_AF_station;
	
	s8_AF_boundary_index = (Ts8)(pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList)-(Ts8)1;
	for(s8_outer_loop_index=0;s8_outer_loop_index < s8_AF_boundary_index ;s8_outer_loop_index++)
	{
		for(s8_AFListIndex = 0; s8_AFListIndex < (s8_AF_boundary_index - s8_outer_loop_index ) ; s8_AFListIndex++)
		{
			if(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[s8_AFListIndex].u32_avg_qual < pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[s8_AFListIndex+1].u32_avg_qual)
			{
				st_temp_AF_station	 														  = pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[s8_AFListIndex];
				pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[s8_AFListIndex]  =	pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[s8_AFListIndex+1];
				pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[s8_AFListIndex+1]=	st_temp_AF_station;
			}
		}
	}
	pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index=0;
	
	#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index	 = 0;
	#endif
}
							
void AMFM_App_Sort_REG_PI_AF_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Ts8 s8_AFListIndex;
	Ts8 s8_outer_loop_index;
	Ts8 s8_AF_boundary_index =0;
	Ts_AMFM_App_AFStation_Info st_temp_AF_station;
	s8_AF_boundary_index = (Ts8)(pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList)-(Ts8)1;
	for(s8_outer_loop_index=0;s8_outer_loop_index < s8_AF_boundary_index ;s8_outer_loop_index++)
	{
		for(s8_AFListIndex = 0; s8_AFListIndex < (s8_AF_boundary_index - s8_outer_loop_index ) ; s8_AFListIndex++)
		{
			if(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[s8_AFListIndex].u32_avg_qual < pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[s8_AFListIndex+1].u32_avg_qual)
			{
				st_temp_AF_station	 														  	= 	pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[s8_AFListIndex];
				pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[s8_AFListIndex] 	=	pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[s8_AFListIndex+1];
				pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[s8_AFListIndex+1]	=	st_temp_AF_station;
			}
		}
	}
	pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index=0;
	
	#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index	 = 0;
	#endif
}
							


Tbool AMFM_App_SAME_PI_Best_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tbool		b_return_val = TRUE;
	Tu8 u8_AF_list_index;
	Tu8 u8_Same_PI_index=0;
	Tu8 u8_zero_PI_index=0;
	Tbool		b_Same_PI_Found = FALSE;
	Tbool		b_Zero_PI_Found = FALSE;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_list_index++ )
	{
		if((pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info [u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_SAME) &&
			(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].b_af_check==FALSE))
		{
			u8_Same_PI_index = u8_AF_list_index;
			b_Same_PI_Found = TRUE;
			break;
		}
	}
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_list_index++ )
	{
		if((pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_ZERO)&&
			(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].b_af_check==FALSE))
		{
			u8_zero_PI_index = u8_AF_list_index;
			b_Zero_PI_Found = TRUE;
			break;
		}
	}
	
	if ((b_Same_PI_Found != FALSE) || (b_Zero_PI_Found != FALSE))
	{
		if ((b_Same_PI_Found != FALSE) && (b_Zero_PI_Found != FALSE))
		{
			u8_AF_list_index = (u8_Same_PI_index < u8_zero_PI_index)? u8_Same_PI_index : u8_zero_PI_index;
		}
		else if (b_Same_PI_Found != FALSE)
		{
			u8_AF_list_index = u8_Same_PI_index;
		}
		else 									/*(u8_Zero_PI_Found != 0)*/
		{
			u8_AF_list_index = u8_zero_PI_index;
		}
	
		if((pst_AMFM_App_AFList_Info->u32_Qua_avg < pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u32_avg_qual)
		   && ((pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u32_avg_qual - pst_AMFM_App_AFList_Info->u32_Qua_avg ) > u8_Delta_Difference))
		{
			b_return_val = FALSE;
			pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index		= u8_AF_list_index;
			pst_AMFM_App_AFList_Info->pst_current_AFStation_Info	= pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info;
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index = AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,u8_AF_list_index );
				
			#endif
		}
		else
		{
			b_return_val = TRUE;
		}
		
	}
	
	return b_return_val;
}
/*===========================================================================*/
/*			void AMFM_APP_Curr_Best_Freq_AF_check			 				 */
/*===========================================================================*/

void AMFM_APP_Curr_Best_Freq_AF_check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{			
	Tu8 u8_Best_AF_station_Index;
	Tu8 u8_calib_Best_AF_station_Index;
	
	u8_Best_AF_station_Index = pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index;
	pst_AMFM_App_AFList_Info->b_af_check_flag   = TRUE;
	pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_Best_AF_station_Index].b_af_check= TRUE;
	
	#ifdef CALIBRATION_TOOL
		u8_calib_Best_AF_station_Index =st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index;
	 	st_Calib_AMFM_App_AFList_Info.b_af_check_flag = TRUE;
		st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_Best_AF_station_Index].b_af_check=TRUE;
	#endif
	
	AMFM_Tuner_Ctrl_Request_FM_Check(pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_Best_AF_station_Index].u32_AF_Freq);
}

/*===========================================================================*/
/*			void AMFM_App_Remove_AF_From_List				 				 */
/*===========================================================================*/

void AMFM_App_Remove_AF_From_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu8 *pu8_Tot_No_AF)
{
	Tu8 u8_AF_index = pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index + 1;
	for(; u8_AF_index < (*pu8_Tot_No_AF) ; u8_AF_index++)
	{
		pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AF_index -1]=pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AF_index];
	}
	memset(&(pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AF_index -1]),0,sizeof(pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AF_index -1]));
	(*pu8_Tot_No_AF)--;
}


#ifdef CALIBRATION_TOOL

/*===========================================================================*/
/*			void AMFM_App_Calib_Remove_AF_From_List				 			 */
/*===========================================================================*/

void AMFM_App_Calib_Remove_AF_From_List(Tu8 *pu8_Tot_No_AF)
{
	Tu8 u8_AF_index = st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index + 1;
	for(; u8_AF_index < (*pu8_Tot_No_AF) ; u8_AF_index++)
	{
		st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AF_index -1]=st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AF_index];
	}
	memset(&(st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AF_index -1]),0x00,sizeof(st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_AF_index -1]));
	(*pu8_Tot_No_AF)--;
}

/*===========================================================================*/
/*			Tu8 AMFM_App_Calib_get_AF_index						 			 */
/*===========================================================================*/

Tu8 AMFM_App_Calib_get_AF_index(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info,Tu8 u8_NumAFList,Tu8 u8_index)
{

	Tu8 u8_calib_index=0;
	Tu16 u32_af_freq = pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_index].u32_AF_Freq;
	
	for(;u8_calib_index <u8_NumAFList;u8_calib_index++)
	{
		if( u32_af_freq == st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_index].u32_AF_Freq)
		{
			break;
		}
	}
	return u8_calib_index;
}
#endif

/*===========================================================================*/
/*			void AMFM_App_AFupdate_Restart  				 				 */
/*===========================================================================*/

void AMFM_App_AFupdate_Restart(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_AFListIndex=0;
	AMFM_App_Flag_Reset(pst_AMFM_App_AFList_Info);
	if(pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList > 0 )
	{
		pst_AMFM_App_AFList_Info->pst_current_AFStation_Info = pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info;
		u8_AFListIndex=pst_AMFM_App_AFList_Info->u8_AF_Update_Index;
		pst_AMFM_App_AFList_Info->u32_af_update_newfreq=pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
		
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info	= st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
			st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index			= AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,u8_AFListIndex);
			st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index].u32_AF_Freq;
		#endif
		
	}
	else
	{
		pst_AMFM_App_AFList_Info->pst_current_AFStation_Info	= pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info;
		u8_AFListIndex											= pst_AMFM_App_AFList_Info->u8_AF_Update_Index;
		pst_AMFM_App_AFList_Info->u32_af_update_newfreq			= pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
	
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info	= &(st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[0]);
			st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index			= AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList,u8_AFListIndex);
			st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index].u32_AF_Freq;
		#endif
		
	}

	AMFM_Tuner_Ctrl_Request_AF_Update(pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq);
}

/*===========================================================================*/
/*			void AMFM_App_SAME_PI_AF_Avaliability_Check		 				 */
/*===========================================================================*/

Tbool AMFM_App_SAME_PI_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tbool   b_return_val = TRUE;
	Tu8 u8_AF_list_index;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_list_index++ )
	{
		if(((pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_SAME) ||
		   (pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_ZERO))
		&&((pst_AMFM_App_AFList_Info->u32_Qua_avg < pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u32_avg_qual)&&
		   ((pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u32_avg_qual - pst_AMFM_App_AFList_Info->u32_Qua_avg ) > u8_Delta_Difference)))
		{
			b_return_val = FALSE;
			pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index		= u8_AF_list_index;
			pst_AMFM_App_AFList_Info->pst_current_AFStation_Info	= pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info;
		
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info 	= st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index 		= AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,u8_AF_list_index );
				
			#endif
		
			break;
		}
	}
	return b_return_val;
}

/*===========================================================================*/
/*			void AMFM_App_REG_PI_Best_AF_Avaliability_Check	 				 */
/*===========================================================================*/
Tbool AMFM_App_REG_PI_Best_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tbool		b_return_val = TRUE;
	Tu8 u8_AF_list_index;
	Tu8 u8_Reg_PI_index=0;
	Tu8 u8_zero_PI_index=0;
	Tbool		b_REG_PI_Found = FALSE;
	Tbool		b_Zero_PI_Found = FALSE;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_AF_list_index++ )
	{
		if((pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info [u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_REG) &&
			(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].b_af_check==FALSE))
		{
			u8_Reg_PI_index = u8_AF_list_index;
			b_REG_PI_Found = TRUE;
			break;
		}
	}
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_AF_list_index++ )
	{
		if((pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_ZERO)&&
			(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].b_af_check==FALSE))
		{
			u8_zero_PI_index = u8_AF_list_index;
			b_Zero_PI_Found = TRUE;
			break;
		}
	}
	
	if ((b_REG_PI_Found != FALSE) || (b_Zero_PI_Found != FALSE))
	{
		if ((b_REG_PI_Found != FALSE) && (b_Zero_PI_Found != FALSE))
		{
			u8_AF_list_index = (u8_Reg_PI_index < u8_zero_PI_index)? u8_Reg_PI_index : u8_zero_PI_index;
		}
		else if (b_REG_PI_Found != FALSE)
		{
			u8_AF_list_index = u8_Reg_PI_index;
		}
		else 									/*(u8_Zero_PI_Found != 0)*/
		{
			u8_AF_list_index = u8_zero_PI_index;
		}
	
		if((pst_AMFM_App_AFList_Info->u32_Qua_avg < pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].u32_avg_qual)
		   && ((pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].u32_avg_qual - pst_AMFM_App_AFList_Info->u32_Qua_avg ) > u8_Delta_Difference))
		{
			b_return_val = FALSE;
			pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index		= u8_AF_list_index;
			pst_AMFM_App_AFList_Info->pst_current_AFStation_Info	= pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info;
			
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info = st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info;
				st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index	 = AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList,u8_AF_list_index);
			
			#endif
		}
		else
		{
			b_return_val = TRUE;
		}
		
	}
	
	return b_return_val;
}



/*===========================================================================*/
/*			void AMFM_APP_Clear_AF_Qual_Parameters			 				 */
/*===========================================================================*/
void AMFM_APP_Clear_AF_Qual_Parameters(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_index=0;
	
	for(u8_index=0; u8_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_index++)
	{
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_avg_qual  =0;
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_old_qual  =0;
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_curr_qual =0;
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].b_af_check   =FALSE;
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].s32_BBFieldStrength    =0;
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_UltrasonicNoise    =0;
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_Multipath          =0;
		pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_FrequencyOffset    =0;
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].u32_avg_qual  =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].u32_old_qual  =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].u32_curr_qual =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].b_af_check   =FALSE;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].s32_BBFieldStrength =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].u32_UltrasonicNoise =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].u32_Multipath       =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_index].u32_FrequencyOffset =0;
			
		#endif
	}
	
	for(u8_index=0; u8_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_index++)
	{
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_avg_qual  =0;
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_old_qual  =0;
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_curr_qual =0;
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].b_af_check   =FALSE;
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].s32_BBFieldStrength  =0;
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_UltrasonicNoise  =0;
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_Multipath        =0;
		pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_FrequencyOffset  =0;

		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].u32_avg_qual  =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].u32_old_qual  =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].u32_curr_qual =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].b_af_check   =FALSE;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].s32_BBFieldStrength  =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].u32_UltrasonicNoise  =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].u32_Multipath        =0;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_index].u32_FrequencyOffset  =0;
		#endif
	}
	
}


/*===========================================================================*/
/*			void AMFM_App_AF_Quality_Avg_computation		 				 */
/*===========================================================================*/		
void  AMFM_App_AF_Quality_Avg_computation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_qual_incr_alpha, Tu32 u32_AF_qual_decr_alpha)
{
	Tu8 u8_AF_list_index = pst_AMFM_App_AFList_Info -> u8_AF_Update_Index;
	#ifdef CALIBRATION_TOOL
		Tu8 u8_calib_AF_list_index = st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index ;
	#endif
	
	Tu32 u32_avg = pst_AMFM_App_AFList_Info -> pst_current_AFStation_Info[u8_AF_list_index].u32_avg_qual;
	Tu32 u32_old = pst_AMFM_App_AFList_Info -> pst_current_AFStation_Info[u8_AF_list_index].u32_old_qual;
	Tu32 u32_curr= pst_AMFM_App_AFList_Info -> pst_current_AFStation_Info[u8_AF_list_index].u32_curr_qual;
	
	if(u32_old == 0)
	{
		u32_avg = u32_curr;
	} 
	else
	{
		if( u32_curr >= u32_old )
		{
			u32_avg =(((u32_avg * u32_AF_qual_incr_alpha) + ((u8_alpha_div_factor - u32_AF_qual_incr_alpha)*u32_curr))/u8_alpha_div_factor);
		}
		else
		{
			u32_avg =(((u32_avg * u32_AF_qual_decr_alpha) + ((u8_alpha_div_factor - u32_AF_qual_decr_alpha)*u32_curr))/u8_alpha_div_factor);
		}
	}
	u32_old = u32_curr;
	pst_AMFM_App_AFList_Info -> pst_current_AFStation_Info[u8_AF_list_index].u32_avg_qual =  u32_avg;
	pst_AMFM_App_AFList_Info -> pst_current_AFStation_Info[u8_AF_list_index].u32_old_qual =	u32_old;
	pst_AMFM_App_AFList_Info -> pst_current_AFStation_Info[u8_AF_list_index].u32_curr_qual= 	u32_curr;
	
	#ifdef CALIBRATION_TOOL
		
		st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_AF_list_index].u32_avg_qual =  u32_avg;
		st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_AF_list_index].u32_old_qual =	u32_old;
		st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[u8_calib_AF_list_index].u32_curr_qual= 	u32_curr;
	
	#endif
	
}


/*===========================================================================*/
/*			Tu8 AMFM_APP_ENG_AF_Existance_Check				 				 */
/*===========================================================================*/	
Tbool AMFM_APP_ENG_AF_Existance_check(Tu32 u32_AF_Freq,Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info)
{
	Tu8 u8_index;
	Tbool 	b_return_value = FALSE;
	for(u8_index=0; u8_index < pst_AMFM_App_ENG_AFList_Info->u8_NumAFList ; u8_index++)
	{
		if(u32_AF_Freq == pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_index].u32_AF_Freq )
		{
			b_return_value = TRUE;
		}
	}
	return b_return_value;
}


/*===========================================================================*/
/*			Tu8 AMFM_APP_Get_ENG_AF_Index					 				 */
/*===========================================================================*/	
Tu8 AMFM_APP_Get_ENG_AF_Index(Tu32 u32_AF_Freq,Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info)
{
	Tu8 u8_index;
	
	for(u8_index=0; u8_index < pst_AMFM_App_ENG_AFList_Info->u8_NumAFList ; u8_index++)
	{
		if(u32_AF_Freq == pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_index].u32_AF_Freq )
		{
			break;
		}
	}
	return u8_index;
}


/*===========================================================================*/
/*					Tu8 AMFM_APP_ENG_AF_Updation_Check		 				 */
/*===========================================================================*/	
Tbool AMFM_APP_ENG_AF_Updation_Check(Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info,Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info)
{
	Tbool 	b_return_value = FALSE;
	Tbool 	b_check		   = FALSE;
	Tu8 u8_index;
	Tu8 u8_AF_index;
	for(u8_index=0 ; u8_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_index++ )
	{
		b_check = AMFM_APP_ENG_AF_Existance_check(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_AF_Freq,pst_AMFM_App_ENG_AFList_Info);
		if(b_check == TRUE)
		{
			/*frequency present*/
			 u8_AF_index=AMFM_APP_Get_ENG_AF_Index(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_AF_Freq,pst_AMFM_App_ENG_AFList_Info);
			 if((pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_AF_index].u32_avg_qual != pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_avg_qual)||
			    (pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_AF_index].u16_PI      != pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u16_PI))
			 {	
				b_return_value = TRUE;
				pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_AF_index].u32_avg_qual = pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_avg_qual;
				pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_AF_index].u16_PI      = pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u16_PI;
			 }
		}
	}
	for(u8_index=0 ; u8_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_index++)
	{
		b_check = AMFM_APP_ENG_AF_Existance_check(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_AF_Freq,pst_AMFM_App_ENG_AFList_Info);
		if(b_check == TRUE)
		{
			/*frequency present*/
			 u8_AF_index=AMFM_APP_Get_ENG_AF_Index(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_AF_Freq,pst_AMFM_App_ENG_AFList_Info);
			 if((pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_AF_index].u32_avg_qual != pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_avg_qual) ||
				(pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_AF_index].u16_PI      != pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u16_PI)  )
			 {	
				b_return_value = TRUE;
				pst_AMFM_App_ENG_AFList_Info->ast_ENG_AF_List[u8_AF_index].u32_avg_qual = pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_avg_qual;
			 }
		}
	}
	return b_return_value;
}	


/*===========================================================================*/
/*				void  AMFM_APP_Curr_Station_Related_Clearing 				 */
/*===========================================================================*/	
void  AMFM_APP_Curr_Station_Related_Clearing(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info)
{
	
	/* Clearing st_current_station structure before updating into it */
	memset(&(pst_me_amfm_app_inst->st_current_station),AMFM_APP_CONSTANT_ZERO,sizeof(pst_me_amfm_app_inst->st_current_station));
	memset(&(pst_me_amfm_app_inst->st_EON_List) ,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_EON_List));
	memset(&(pst_me_amfm_app_inst->st_EON_station_Info) ,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_StationInfo));
	memset(&(pst_me_amfm_app_inst->st_ENG_AFList_Info) ,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_ENG_AFList_Info));
	memset(&(pst_me_amfm_app_inst->st_RDS_CT_Info) ,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_AppRDS_CT_Info));
	memset(pst_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_AMFM_App_AFList_Info));
	memset(&(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo),AMFM_APP_CONSTANT_ZERO,sizeof(pst_me_amfm_app_inst->st_DAB_FollowUp_StationInfo));

	
	#ifdef CALIBRATION_TOOL
		memset(&st_Calib_AMFM_App_AFList_Info ,AMFM_APP_CONSTANT_ZERO,sizeof(st_Calib_AMFM_App_AFList_Info));
	#endif
	pst_me_amfm_app_inst->b_NEG_TimerStartCheck			= FALSE;
	/*can stop timer happen here */
	pst_me_amfm_app_inst->e_SigQuality=AMFM_APP_LOW_QUALITY_SIGNAL;
	pst_me_amfm_app_inst->b_RegThresholdCheck				=FALSE;
	/* Enabling AF strategy flag  */
	pst_me_amfm_app_inst->b_StartAF_Flag = TRUE;
		
} 	
/*===========================================================================*/
/*				void  AMFM_App_ReadCTinfo					 				 */
/*===========================================================================*/	
void  AMFM_App_ReadCTinfo(Ts_AMFM_AppRDS_CT_Info *pst_RDS_CTinfo,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo)
{
	pst_RDS_CTinfo->u8_Hour 			= pst_TunerctrlCurrentStationInfo->st_CT_Info.u8_Hour;
    pst_RDS_CTinfo->u8_Min 				= pst_TunerctrlCurrentStationInfo->st_CT_Info.u8_Min;
	pst_RDS_CTinfo->u8_OffsetSign		= pst_TunerctrlCurrentStationInfo->st_CT_Info.u8_offset_sign;
    pst_RDS_CTinfo->u8_Localtime_offset	= pst_TunerctrlCurrentStationInfo->st_CT_Info.u8_Localtime_offset;       
    pst_RDS_CTinfo->u8_Day 				= pst_TunerctrlCurrentStationInfo->st_CT_Info.u8_Day;                    
	pst_RDS_CTinfo->u8_Month 			= pst_TunerctrlCurrentStationInfo->st_CT_Info.u8_Month;
	pst_RDS_CTinfo->u16_Year 			= pst_TunerctrlCurrentStationInfo->st_CT_Info.u16_Year;		
}
 	
/*===========================================================================*/
/*	Te_RADIO_ReplyStatus  AMFM_App_Compute_CT_Info					 */
/*===========================================================================*/	
Te_RADIO_ReplyStatus  AMFM_App_Compute_CT_Info(Ts_AMFM_AppRDS_CT_Info *pst_RDS_CTinfo,Ts_AMFM_App_CT_Info *pst_CT_Info)
{
	Te_RADIO_ReplyStatus	e_GetCT_InfoReplystatus = REPLYSTATUS_FAILURE;
	
 	/*...........Local time conversion from RDS time.................*/    
   
	/*Converted into local time from the RDS offset*/
	if(pst_RDS_CTinfo->u8_OffsetSign == 0)
	{
		pst_CT_Info->u8_Hour =((((pst_RDS_CTinfo->u8_Hour)*60  + pst_RDS_CTinfo->u8_Min )+ (pst_RDS_CTinfo->u8_Localtime_offset*30))/60);
		pst_CT_Info->u8_Min  =((((pst_RDS_CTinfo->u8_Hour)*60 + pst_RDS_CTinfo->u8_Min )+ ((pst_RDS_CTinfo->u8_Localtime_offset*30)))%60);
	}
	else
	{
		pst_CT_Info->u8_Hour =((((pst_RDS_CTinfo->u8_Hour)*60 + pst_RDS_CTinfo->u8_Min )- (pst_RDS_CTinfo->u8_Localtime_offset*30))/60);
		pst_CT_Info->u8_Min  =((((pst_RDS_CTinfo->u8_Hour)*60 + pst_RDS_CTinfo->u8_Min )- (pst_RDS_CTinfo->u8_Localtime_offset*30))%60);
	}
	pst_CT_Info->u8_day 	= pst_RDS_CTinfo->u8_Day;
	pst_CT_Info->u8_Month 	= pst_RDS_CTinfo->u8_Month;
	pst_CT_Info->u16_Year 	= pst_RDS_CTinfo->u16_Year;

	
	if((pst_CT_Info->u8_Hour!=0)||(pst_CT_Info->u8_Min!=0)||(pst_CT_Info->u8_day!=0)||(pst_CT_Info->u8_Month!=0)||(pst_CT_Info->u16_Year!=0))
	{
		e_GetCT_InfoReplystatus = REPLYSTATUS_SUCCESS;
	}
	return e_GetCT_InfoReplystatus;
	
}

/*===========================================================================*/
/*	void  AMFM_App_StartUp_Initialisation									 */
/*===========================================================================*/	
void AMFM_App_StartUp_Initialisation(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst)
{
	pst_me_amfm_app_inst->u32_AMFM_quality        			= 0;		
	pst_me_amfm_app_inst->u32_AMFM_Normal_FS_Threshold 		= AMFM_APP_NORMAL_THRESHOLD;	
	pst_me_amfm_app_inst->u32_AMFM_Siglow_Threshold 			= AMFM_APP_SIGLOW_THRESHOLD;		
	pst_me_amfm_app_inst->u32_AMFM_Regional_Threshold		= AMFM_APP_REGIONAL_THRESHOLD;
	pst_me_amfm_app_inst->u8_QualityDrop_Margin 			= AMFM_APP_QUALITY_DROP_MARGIN;			
	pst_me_amfm_app_inst->b_AFlist_copy_check				= TRUE;
	pst_me_amfm_app_inst->u32_AF_qual_incr_alpha 			= AMFM_APP_AF_STATION_CURR_QUAL_INCREASED_ALPHA; 	
	pst_me_amfm_app_inst->u32_AF_qual_decr_alpha 			= AMFM_APP_AF_STATION_CURR_QUAL_DECREASED_ALPHA; 	
	pst_me_amfm_app_inst->u32_AF_qual_Existance_incr_alpha 	= AMFM_APP_AF_STATION_CURR_QUAL_INCREASED_ALPHA; 	
	pst_me_amfm_app_inst->u32_AF_qual_Existance_decr_alpha	= AMFM_APP_AF_STATION_CURR_QUAL_DECREASED_ALPHA;
	pst_me_amfm_app_inst->u32_AF_Next_Freq_Update_Delay 	= AMFM_APP_AF_NEXT_FREQ_UPDATE_DELAY;		
	pst_me_amfm_app_inst->u8_StrategyAFUpdate_Delay 		= AMFM_APP_STRATEGY_AF_NEXT_FREQ_UPDATE_DELAY;		
	pst_me_amfm_app_inst->u32_RDS_Senitivity_Threshold 		= AMFM_APP_RDS_SENSITIVITY_THRESHOLD;	
	pst_me_amfm_app_inst->b_CurStationPICpy					= TRUE;
	pst_me_amfm_app_inst->u8_Curr_qua_check_count			= 0;				
	pst_me_amfm_app_inst->u32_Curr_stat_qual_check_Delay 	= AMFM_APP_CURRENT_STATION_QUALITY_CHECK_DELAY;	
	pst_me_amfm_app_inst->u16_af_Tuned_PI=0;
	pst_me_amfm_app_inst->	u32_AF_qual_Modified_incr_alpha 	= AMFM_APP_AF_STATION_MODIFIED_INCREASED_ALPHA; 
	pst_me_amfm_app_inst->	u32_AF_qual_Modified_decr_alpha 	= AMFM_APP_AF_STATION_MODIFIED_DECREASED_ALPHA; 
	pst_me_amfm_app_inst->b_NEG_TimerStartCheck				= FALSE;
	pst_me_amfm_app_inst->u32_AF_StatusTimeout				= AMFM_APP_AF_STATUS_CHECK_TIMEOUT;
	pst_me_amfm_app_inst->b_RegThresholdCheck				=FALSE;
	pst_me_amfm_app_inst->e_SigQuality						= AMFM_APP_LOW_QUALITY_SIGNAL;
	pst_me_amfm_app_inst->u8_SigResumeback_Threshold		= AMFM_APP_SIG_RESUME_BACK_THRESHOLD;	
	pst_me_amfm_app_inst->s8_CurrStation_IndexInSTL  		= -1;

	/* Enabling AF strategy flag  */
	pst_me_amfm_app_inst->b_StartAF_Flag = TRUE;
		
	pst_me_amfm_app_inst->e_StaNotAvail_Strategy_status 		= AMFM_APP_STATIONNOTAVAIL_STRATEGY_INVALID;	
}

/*===========================================================================*/
/*	Tu8  AMFM_App_NEG_flag_status											 */
/*===========================================================================*/	
Tbool AMFM_App_NEG_flag_status(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info)
{
	Tbool b_return_val = FALSE;
	Tu8 u8_AF_list_index;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_list_index++ )
	{
		if(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_NEG) 
		{
			b_return_val= TRUE; 
			
			break;
		}
	}
	if(b_return_val == FALSE)
	{
		for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_AF_list_index++ )
		{
			if(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_NEG) 
			{
				b_return_val = TRUE; 
			
				break;
			}
		}
	}
	return b_return_val;
	
}


/*===========================================================================*/
/*	Tu8  AMFM_App_DAB2FM_Check_NEG_flag_AFList											 */
/*===========================================================================*/	
Tbool AMFM_App_DAB2FM_Check_NEG_flag_AFList(	Tu8 	u8_NumAFList,Ts_AMFM_App_AFStation_Info	*pst_AFStation_Info)
{
	Tbool b_return_val = FALSE;
	Tu8 u8_AF_list_index = 0;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < u8_NumAFList; u8_AF_list_index++ )
	{
		if(pst_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_NEG)
		{
			b_return_val	= TRUE; 	
			break;
		}
	}
	
	return b_return_val;
}

/*===========================================================================*/
/*	void  AMFM_App_AF_StatusCountIncrementation								 */
/*===========================================================================*/	

void AMFM_App_AF_StatusCountIncrementation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_AF_list_index;
	Tu8 u8_AF_Calibindex;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_list_index++ )
	{
		#ifdef CALIBRATION_TOOL
			u8_AF_Calibindex = AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,u8_AF_list_index);
		#endif
		if(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_NEG) 
		{
			pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u8_TimerCount++;
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_Calibindex].u8_TimerCount++;
			#endif
		}
	}
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_AF_list_index++ )
	{
		#ifdef CALIBRATION_TOOL
			u8_AF_Calibindex = AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList,u8_AF_list_index);
		#endif
		if(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_NEG) 
		{
			pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].u8_TimerCount++;
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_AF_Calibindex].u8_TimerCount++;
			#endif
		}
	}

}

/*===========================================================================*/
/*	void  AMFM_App_AF_DAB2FM_Increase_NEGStatusCount								 */
/*===========================================================================*/	
void AMFM_App_AF_DAB2FM_Increase_NEGStatusCount(Tu8 	u8_NumAFList,Ts_AMFM_App_AFStation_Info	*pst_AFStation_Info)
{
	Tu8 u8_AF_list_index;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < u8_NumAFList; u8_AF_list_index++)
	{
		if(pst_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_NEG) 
		{
			pst_AFStation_Info[u8_AF_list_index].u8_TimerCount++;
		}
	}
}

/*===========================================================================*/
/*	void  AMFM_App_AF_NEG_StatusReset										 */
/*===========================================================================*/	

void AMFM_App_AF_NEG_StatusReset(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_AF_list_index;
	Tu8 u8_AF_Calibindex;
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_list_index++ )
	{
		#ifdef CALIBRATION_TOOL
			u8_AF_Calibindex = AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,u8_AF_list_index);
		#endif
		if(pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u8_TimerCount == AMFM_APP_AF_STATUS_RESET_TIMEOUT) 
		{
			pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u16_PI 		  = 0;
			pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
			pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].u8_TimerCount  = 0;
			#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_Calibindex].u16_PI 		  = 0;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_Calibindex].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_SAME;
			st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info[u8_AF_Calibindex].u8_TimerCount  = 0;
			#endif
		}
	}
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList; u8_AF_list_index++ )
	{
		#ifdef CALIBRATION_TOOL
			u8_AF_Calibindex = AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList,u8_AF_list_index);
		#endif
		if(pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].u8_TimerCount == AMFM_APP_AF_STATUS_RESET_TIMEOUT) 
		{
			pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].u16_PI 		  = 0;
			pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS  = AMFM_APP_PI_STATUS_REG;
			pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_AF_list_index].u8_TimerCount   = 0;
			#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_AF_Calibindex].u16_PI 		  = 0;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_AF_Calibindex].e_AF_PI_STATUS = AMFM_APP_PI_STATUS_REG;
			st_Calib_AMFM_App_AFList_Info.ast_current_REG_PI_AFStation_Info[u8_AF_Calibindex].u8_TimerCount  = 0;
			#endif
		}
	}
}

/*===========================================================================*/
/*	void  AMFM_App_AF_DAB2FM_Reset_NEG_Status										 */
/*===========================================================================*/	
void AMFM_App_AF_DAB2FM_Reset_NEG_Status(Tu8 u8_NumAFList,Ts_AMFM_App_AFStation_Info*pst_AFStation_Info)
{
	Tu8 u8_AF_list_index;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < u8_NumAFList; u8_AF_list_index++ )
	{
		if(pst_AFStation_Info[u8_AF_list_index].u8_TimerCount == AMFM_APP_AF_STATUS_RESET_TIMEOUT) 
		{
			pst_AFStation_Info[u8_AF_list_index].u16_PI 		  	= 0;
			pst_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS 	= AMFM_APP_PI_STATUS_SAME;
			pst_AFStation_Info[u8_AF_list_index].u8_TimerCount  	= 0;
		}
	}
}
/*===========================================================================*/
/*	void  AMFM_App_clear_PreviousEON_Qualities								 */
/*===========================================================================*/	
void AMFM_App_clear_PreviousEON_Qualities(Ts_AMFM_App_EON_List *pst_EON_List)
{
	Tu8 u8_EONlist_PI_index 		= pst_EON_List->u8_EONlist_PIindex;
	
	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_avg  = 0;
	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_old  = 0 ;
	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_curr = 0;
	pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_Qua_old_avg = 0 ;
}
/*===========================================================================*/
/*	Tu8 AMFM_APP_EON_AF_Index												 */
/*===========================================================================*/
Tu8 AMFM_APP_EON_AF_Index(Ts_AMFM_App_EON_List *pst_EON_List,Tu32 u32_CurrentFrequency)
{
	Tu8 	u8_EON_AF_List_Index;
	Tu8 	u8_EONlist_PI_index	= pst_EON_List->u8_EONlist_PIindex;
	
	for(u8_EON_AF_List_Index=0;u8_EON_AF_List_Index < pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u8_AF_Count ;u8_EON_AF_List_Index++ )
	{
		if( pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index] == u32_CurrentFrequency)
		{
			break;
		}
	}
	return u8_EON_AF_List_Index;
}
/*===========================================================================*/
/*	Tu8 AMFM_APP_Check_EON_AF_Availability									 */
/*===========================================================================*/

Tu8 AMFM_APP_Check_EON_AF_Availability(Ts_AMFM_App_EON_List *pst_EON_List,Tu32 u32_CurrentFrequency)
{
	Tu8 	u8_EON_AF_List_Index;
	Tu8 	u8_EONlist_PI_index	= pst_EON_List->u8_EONlist_PIindex;
	Tu8		u8_RetValue			= 0;
	for(u8_EON_AF_List_Index=0; u8_EON_AF_List_Index < pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u8_AF_Count ;u8_EON_AF_List_Index++ )
	{
		if( pst_EON_List->st_EON_PI_Info[u8_EONlist_PI_index].u32_EON_AF_List[u8_EON_AF_List_Index] == u32_CurrentFrequency)
		{
			u8_RetValue=1;
			break;
		}
	}
	return u8_RetValue;
}

/*===========================================================================*/
/*	void AMFM_App_DeltaComputation											 */
/*===========================================================================*/

void AMFM_App_DeltaComputation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	if(pst_AMFM_App_AFList_Info->u32_Qua_avg > u8_HIGH_Delta_Threshold/*60*/) 
	{
		u8_Delta_Difference = u8_HIGH_Delta_Difference;			/*20*/
	}
	else if(pst_AMFM_App_AFList_Info->u32_Qua_avg > u8_MID_Delta_Threshold/*40*/) 
	{
		u8_Delta_Difference = u8_MID_Delta_Difference;	/*10*/
	}
	else		
	{
		u8_Delta_Difference = u8_Low_Delta_Difference;	/*5*/
	}
}

/*===========================================================================*/
/*	void AMFM_App_DeltaComputation											 */
/*===========================================================================*/
Tu8 AMFM_App_PI_Updation(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu16 u16_curr_station_PI)
{
	Tu8 u8_ret_val=0;
	pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u16_PI	= u16_curr_station_PI ;
	pst_me_amfm_app_inst->st_LSM_FM_Info.u16_LSM_PI 						= u16_curr_station_PI;
	pst_AMFM_App_AFList_Info->u16_curr_PI									= u16_curr_station_PI;
	
					
	#ifdef CALIBRATION_TOOL
	st_Calib_AMFM_App_AFList_Info.u16_curr_PI  = u16_curr_station_PI ;
	#endif
	
	u8_ret_val = AMFM_App_Learn_Memory_updation((pst_AMFM_App_AFList_Info->u32_curr_freq), u16_curr_station_PI,&pst_me_amfm_app_inst->ast_AF_Learn_mem[0]);
	
	return u8_ret_val;

}

/*===========================================================================*/
/*	void AMFM_App_AFListClear												 */
/*===========================================================================*/
void AMFM_App_AFListClear(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	memset(&(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList),0,sizeof(pst_me_amfm_app_inst->st_LSM_FM_Info.st_LSM_AFList));
	memset(&(pst_me_amfm_app_inst->st_current_station.st_Aflist),0,sizeof(pst_me_amfm_app_inst->st_current_station.st_Aflist));
	memset(&(pst_me_amfm_app_inst->st_EON_List),0,sizeof(pst_me_amfm_app_inst->st_EON_List));
	memset(&(pst_me_amfm_app_inst->st_EON_station_Info),0,sizeof(pst_me_amfm_app_inst->st_EON_station_Info));
	memset((pst_AMFM_App_AFList_Info),0,sizeof(Ts_AMFM_App_AFList_Info));
	pst_AMFM_App_AFList_Info->u32_curr_freq	= pst_me_amfm_app_inst->st_current_station.un_station.st_FMstation.u32_frequency ;
	pst_AMFM_App_AFList_Info->e_curr_status	= AMFM_APP_LISTEN_FM_STATION;
	#ifdef CALIBRATION_TOOL
		st_Calib_AMFM_App_AFList_Info 	= *pst_AMFM_App_AFList_Info;
	#endif
}


/*===========================================================================*/
/*	Tu8 AMFM_APP_AFStrategy_AFUpdate										 */
/*===========================================================================*/
Tu8 AMFM_APP_AFStrategy_AFUpdate(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{			
	Tu8 u8_AFListIndex;
	Tu8 u8_ret_value=0;

	pst_AMFM_App_AFList_Info->u8_AF_index++;	
	u8_AFListIndex = pst_AMFM_App_AFList_Info->u8_AF_index;
	
	if(u8_AFListIndex  <  pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList)
	{	
		
		pst_AMFM_App_AFList_Info->pst_current_AFStation_Info = pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info;
		pst_AMFM_App_AFList_Info->u8_AF_Update_Index = pst_AMFM_App_AFList_Info->u8_AF_index;
		/* updating the value of u32_af_update_newfreq */
		pst_AMFM_App_AFList_Info->u32_af_update_newfreq	= pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_AFListIndex].u32_AF_Freq;
		
		#ifdef CALIBRATION_TOOL
			st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info	= st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
			st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index			= AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,pst_AMFM_App_AFList_Info->u8_AF_Update_Index);
			st_Calib_AMFM_App_AFList_Info.u32_af_update_newfreq			= st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info[st_Calib_AMFM_App_AFList_Info.u8_AF_Update_Index].u32_AF_Freq;
		#endif
		/*request update for next AF */
		AMFM_Tuner_Ctrl_Request_AF_Update(pst_AMFM_App_AFList_Info->u32_af_update_newfreq);
	}
	else
	{
		u8_ret_value++;
	}

	return u8_ret_value;
}

/*===========================================================================*/
/*	Tu8 AMFM_App_BestAFAvailabilityCheck									 */
/*===========================================================================*/
Tu8 AMFM_App_BestAFAvailabilityCheck(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info)
{
	Tu8 u8_return_val=1;
	Tu8 u8_AF_list_index;
	
	for(u8_AF_list_index =0 ;u8_AF_list_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList; u8_AF_list_index++ )
	{
		if(((pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_SAME) ||
		    (pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].e_AF_PI_STATUS == AMFM_APP_PI_STATUS_ZERO)) &&
		   (pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_AF_list_index].b_af_check == FALSE) )
		{
			u8_return_val=0; 
			pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index		= u8_AF_list_index;
			pst_AMFM_App_AFList_Info->pst_current_AFStation_Info	= pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info;
		
			#ifdef CALIBRATION_TOOL
				st_Calib_AMFM_App_AFList_Info.pst_current_AFStation_Info 	= st_Calib_AMFM_App_AFList_Info.ast_current_SAME_PI_AFStation_Info;
				st_Calib_AMFM_App_AFList_Info.u8_Best_AF_station_Index 		= AMFM_App_Calib_get_AF_index(pst_AMFM_App_AFList_Info,pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList,u8_AF_list_index );
				
			#endif
		
			break;
		}
	}
	return u8_return_val;
}

/*===========================================================================*/
/*	void AMFM_App_RadioDebugLogPrint										 */
/*===========================================================================*/
void AMFM_App_RadioDebugLogPrint(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info,Te_AMFM_APP_DebuggingStatus e_DebuggingStatus)
{
	Tu8 u8_index=0;
	
	
	switch(e_DebuggingStatus)
	{
		case AMFM_APP_AF_STRATEGY_AFLIST_SORTED_QUALITIES:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AFTune strategy AFList update completed   ");

			for(u8_index=0; (u8_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList) && (u8_index<5); u8_index++)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] same PI list AF freq=%d index=%d PI=%x PI_status=%d avgqual=%d currqual=%d ",
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_AF_Freq,u8_index,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u16_PI ,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].e_AF_PI_STATUS,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_avg_qual,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_curr_qual);
			}
		
		}break;

		case AMFM_APP_AFLIST_SORTED_QUALITIES:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AFList update completed   ");

			for(u8_index=0; (u8_index < pst_AMFM_App_AFList_Info->u8_Num_SAME_PI_AFList) && (u8_index<5); u8_index++)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] same PI list AF freq=%d index=%d PI=%x PI_status=%d avgqual=%d currqual=%d timercount=%d ",
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_AF_Freq,u8_index,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u16_PI ,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].e_AF_PI_STATUS,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_avg_qual,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u32_curr_qual,
								pst_AMFM_App_AFList_Info->ast_current_SAME_PI_AFStation_Info[u8_index].u8_TimerCount);
			}
			for(u8_index=0; (u8_index < pst_AMFM_App_AFList_Info->u8_Num_REG_PI_AFList) && (u8_index<5); u8_index++)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] Regional PI list AF freq=%d index=%d PI=%x PI_status=%d avgqual=%d currqual=%d timercount=%d ",
								pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_AF_Freq,u8_index,
								pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u16_PI ,
								pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].e_AF_PI_STATUS,
								pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_avg_qual,
								pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u32_curr_qual,
								pst_AMFM_App_AFList_Info->ast_current_REG_PI_AFStation_Info[u8_index].u8_TimerCount);
			}
		}break;
		
		case AMFM_APP_AF_STRATEGYCHECK_REQUEST:
		{
			u8_index=pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AFTune strategy BestAFFreq Check request AFFeq=%d  index=%d BestAFQual=%d ",
							pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_index].u32_AF_Freq,u8_index,
							pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_index].u32_avg_qual);

		}break;
		
		case AMFM_APP_AFCHECK_REQUEST:
		{
			u8_index=pst_AMFM_App_AFList_Info->u8_Best_AF_station_Index;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] BestAFFreq  Check request AFFeq=%d  index=%d curr_qual=%d BestAFQual=%d ",
							pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_index].u32_AF_Freq,u8_index,
							pst_AMFM_App_AFList_Info->u32_Qua_avg,
							pst_AMFM_App_AFList_Info->pst_current_AFStation_Info[u8_index].u32_avg_qual);

		}break;

		case AMFM_APP_AF_FREQ_APPENDED_FROM_LEARN_MEMORY:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AF freq appended from learn memory are %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu",
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[0],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[1],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[2],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[3],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[4],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[5],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[6],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[7],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[8],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[9],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[10],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[11],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[12],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[13],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[14],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[15],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[16],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[17],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[18],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[19],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[20],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[21],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[22],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[23],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[24]);
		}break;
		
		
		case AMFM_APP_AF_FREQ_APPENDED:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] AF freq appended from Tuner are %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu",
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[0],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[1],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[2],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[3],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[4],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[5],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[6],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[7],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[8],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[9],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[10],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[11],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[12],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[13],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[14],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[15],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[16],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[17],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[18],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[19],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[20],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[21],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[22],pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[23],
								pst_me_amfm_app_inst->st_current_station.st_Aflist.au32_AFList[24]);
		}break;
		
		case AMFM_APP_EON_FREQ_APPENDED_FROM_LEARN_MEMORY:
		{
			u8_index=pst_me_amfm_app_inst->st_EON_List.u8_PI_Count-1;
		
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_APP] PI=%x EON freq appended from LM are %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu",
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u16_EON_PI, 
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[0],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[1],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[2],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[3],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[4],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[5],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[6],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[7],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[8],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[9],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[10],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[11],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[12],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[13],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[14],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[15],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[16],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[17],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[18],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[19],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[20],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[21],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[22],pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[23],
								pst_me_amfm_app_inst->st_EON_List.st_EON_PI_Info[u8_index].u32_EON_AF_List[24]);
		}break;
		
		case AMFM_APP_EON_STATION_INFO:
		{
			/* Not handled */
		}
		break;
	
	}

}
