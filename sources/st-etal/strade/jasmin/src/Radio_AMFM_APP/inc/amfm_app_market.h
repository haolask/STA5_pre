/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_market.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file contains MACRO definitions for basic frquency range of all       *
*						  market.																			*
*************************************************************************************************************/

#ifndef AMFM_APP_MARKET_H
#define  AMFM_APP_MARKET_H
/*-----------------------------------------------------------------------------
  File Inclusions
-----------------------------------------------------------------------------*/

#include "cfg_types.h"


/*-----------------------------------------------------------------------------
    Macro Defintions
-----------------------------------------------------------------------------*/


/******************************************************************************
                    Western Europe Market
******************************************************************************/ 

#define AMFM_APP_EU_MARKET_AM_MW_STARTFREQ		((Tu32) 531 )			/*	AM band(MW) Start freq in kHz for Europe Market */
#define AMFM_APP_EU_MARKET_AM_MW_ENDFREQ		((Tu32) 1629 )			/*	AM band(MW) End freq in kHz for Europe Market  */				
#define AMFM_APP_EU_MARKET_AM_MW_STEPSIZE		((Tu32)  9)				/*	AM band(MW) Step size in kHz for Europe Market */

#define AMFM_APP_EU_MARKET_AM_LW_STARTFREQ		((Tu32) 144 )			/*	AM band(LW) Start freq in kHz for Europe Market		*/
#define AMFM_APP_EU_MARKET_AM_LW_ENDFREQ		((Tu32) 288 )			/*	AM band(LW) End freq in kHz for Europe Market	    */				
#define AMFM_APP_EU_MARKET_AM_LW_STEPSIZE		((Tu32)  3)				/*	AM band(LW) Step size in kHz for Europe Market		*/


#define AMFM_APP_EU_MARKET_FM_STARTFREQ			((Tu32) 87500 )			/* FM band Start freq in kHz for Europe Market  */
#define AMFM_APP_EU_MARKET_FM_ENDFREQ			((Tu32) 108000 )		/* FM band End freq in kHz for Europe Market    */				
#define AMFM_APP_EU_MARKET_FM_STEPSIZE			((Tu32) 100 )			/* FM band Step size in kHz for Europe Market	*/

/******************************************************************************
                    Latin America Market
******************************************************************************/ 

#define AMFM_APP_LAR_MARKET_AM_MW_STARTFREQ			((Tu32) 530 )			/*	AM band(MW) Start freq in kHz for Latin America  Market */
#define AMFM_APP_LAR_MARKET_AM_MW_ENDFREQ			((Tu32) 1710 )			/*	AM band(MW) End freq in kHz for Latin America  Market  */				
#define AMFM_APP_LAR_MARKET_AM_MW_STEPSIZE			((Tu32)  5)				/*	AM band(MW) Step size in kHz for Latin America  Market */

#define AMFM_APP_LAR_MARKET_FM_STARTFREQ			((Tu32) 87500 )			/* FM band Start freq in kHz for Latin America  Market  */
#define AMFM_APP_LAR_MARKET_FM_ENDFREQ				((Tu32) 107900 )		/* FM band End freq in kHz for Latin America  Market    */				
#define AMFM_APP_LAR_MARKET_FM_STEPSIZE				((Tu32) 100 )			/* FM band Step size in kHz for Latin America  Market	*/
/******************************************************************************
                    Asia Market
******************************************************************************/ 

#define AMFM_APP_ASIA_MARKET_AM_MW_STARTFREQ		((Tu32) 531 )				/*	AM band(MW) Start freq in kHz for Asia Market */
#define AMFM_APP_ASIA_MARKET_AM_MW_ENDFREQ			((Tu32) 1629 )				/*	AM band(MW) End freq in kHz for Asia Market  */				
#define AMFM_APP_ASIA_MARKET_AM_MW_STEPSIZE			((Tu32)  9)					/*	AM band(MW) Step size in kHz for Asia Market */

#define AMFM_APP_ASIA_MARKET_FM_STARTFREQ			((Tu32) 87500 )				/* FM band Start freq in kHz for Asia Market  */
#define AMFM_APP_ASIA_MARKET_FM_ENDFREQ				((Tu32) 108000 )			/* FM band End freq in kHz for Asia Market    */				
#define AMFM_APP_ASIA_MARKET_FM_STEPSIZE			((Tu32) 100 )				/* FM band Step size in kHz for Asia Market	*/

/******************************************************************************
                    Arabic Market
******************************************************************************/ 

#define AMFM_APP_ARAB_MARKET_AM_MW_STARTFREQ		((Tu32) 531 )				/*	AM band(MW) Start freq in kHz for Arabic Market */
#define AMFM_APP_ARAB_MARKET_AM_MW_ENDFREQ			((Tu32) 1602 )				/*	AM band(MW) End freq in kHz for Arabic Market  */				
#define AMFM_APP_ARAB_MARKET_AM_MW_STEPSIZE			((Tu32)  9)					/*	AM band(MW) Step size in kHz for Arabic Market */

#define AMFM_APP_ARAB_MARKET_FM_STARTFREQ			((Tu32) 87500 )				/* FM band Start freq in kHz for Arabic Market  */
#define AMFM_APP_ARAB_MARKET_FM_ENDFREQ				((Tu32) 108000 )			/* FM band End freq in kHz for Arabic Market    */				
#define AMFM_APP_ARAB_MARKET_FM_STEPSIZE			((Tu32) 100 )				/* FM band Step size in kHz for Arabic Market	*/

/******************************************************************************
                    North America Market
******************************************************************************/ 

#define AMFM_APP_NAR_MARKET_AM_MW_STARTFREQ			((Tu32) 530 )				/*	AM band(MW) Start freq in kHz for North America Market */
#define AMFM_APP_NAR_MARKET_AM_MW_ENDFREQ			((Tu32) 1710 )				/*	AM band(MW) End freq in kHz for North America Market  */				
#define AMFM_APP_NAR_MARKET_AM_MW_STEPSIZE			((Tu32)  10)					/*	AM band(MW) Step size in kHz for North America Market */

#define AMFM_APP_NAR_MARKET_FM_STARTFREQ			((Tu32) 87900 )				/* FM band Start freq in kHz for North America Market  */
#define AMFM_APP_NAR_MARKET_FM_ENDFREQ				((Tu32) 107900 )			/* FM band End freq in kHz for North America Market    */				
#define AMFM_APP_NAR_MARKET_FM_STEPSIZE				((Tu32) 200 )				/* FM band Step size in kHz for North America Market	*/


/******************************************************************************
                    Japan Market
******************************************************************************/ 

#define AMFM_APP_JPN_MARKET_AM_MW_STARTFREQ			((Tu32) 522 )				/*	AM band(MW) Start freq in kHz for Japan Market */
#define AMFM_APP_JPN_MARKET_AM_MW_ENDFREQ			((Tu32) 1629 )				/*	AM band(MW) End freq in kHz for Japan Market  */				
#define AMFM_APP_JPN_MARKET_AM_MW_STEPSIZE			((Tu32)  9)					/*	AM band(MW) Step size in kHz for Japan Market */

#define AMFM_APP_JPN_MARKET_FM_STARTFREQ			((Tu32) 76000 )				/* FM band Start freq in kHz for Japan Market  */
#define AMFM_APP_JPN_MARKET_FM_ENDFREQ				((Tu32) 90000 )				/* FM band End freq in kHz for Japan Market    */				
#define AMFM_APP_JPN_MARKET_FM_STEPSIZE				((Tu32) 100 )				/* FM band Step size in kHz for Japan Market	*/


/******************************************************************************
                    Korea Market
******************************************************************************/ 

#define AMFM_APP_KOREA_MARKET_AM_MW_STARTFREQ		((Tu32) 531 )				/*	AM band(MW) Start freq in kHz for Korea Market */
#define AMFM_APP_KOREA_MARKET_AM_MW_ENDFREQ			((Tu32) 1629 )				/*	AM band(MW) End freq in kHz for Korea Market  */				
#define AMFM_APP_KOREA_MARKET_AM_MW_STEPSIZE		((Tu32)  9)					/*	AM band(MW) Step size in kHz for Korea Market */

#define AMFM_APP_KOREA_MARKET_FM_STARTFREQ			((Tu32) 87500 )				/* FM band Start freq in kHz for Korea Market  */
#define AMFM_APP_KOREA_MARKET_FM_ENDFREQ			((Tu32) 108000 )			/* FM band End freq in kHz for Korea Market    */				
#define AMFM_APP_KOREA_MARKET_FM_STEPSIZE			((Tu32) 100 )				/* FM band Step size in kHz for Korea Market	*/

/******************************************************************************
                    Brazil Market
******************************************************************************/ 

#define AMFM_APP_BRAZIL_MARKET_AM_MW_STARTFREQ			((Tu32) 530)				/*	AM band(MW) Start freq in kHz for Brazil Market */
#define AMFM_APP_BRAZIL_MARKET_AM_MW_ENDFREQ			((Tu32) 1710)				/*	AM band(MW) End freq in kHz for Brazil Market  */				
#define AMFM_APP_BRAZIL_MARKET_AM_MW_STEPSIZE			((Tu32)  5)					/*	AM band(MW) Step size in kHz for Brazil Market */

#define AMFM_APP_BRAZIL_MARKET_FM_STARTFREQ				((Tu32) 76000 )				/* FM band Start freq in kHz for Brazil Market  */
#define AMFM_APP_BRAZIL_MARKET_FM_ENDFREQ				((Tu32) 108000 )			/* FM band End freq in kHz for Brazil Market    */				
#define AMFM_APP_BRAZIL_MARKET_FM_STEPSIZE				((Tu32) 100 )				/* FM band Step size in kHz for Brazil Market	*/


/******************************************************************************
                    South America Market
******************************************************************************/ 

#define AMFM_APP_SOUTH_AMERICA_MARKET_AM_MW_STARTFREQ			((Tu32) 530)				/*	AM band(MW) Start freq in kHz for Brazil Market */
#define AMFM_APP_SOUTH_AMERICA_MARKET_AM_MW_ENDFREQ				((Tu32) 1710)				/*	AM band(MW) End freq in kHz for Brazil Market  */				
#define AMFM_APP_SOUTH_AMERICA_MARKET_AM_MW_STEPSIZE			((Tu32)  5)					/*	AM band(MW) Step size in kHz for Brazil Market */

#define AMFM_APP_SOUTH_AMERICA_MARKET_FM_STARTFREQ				((Tu32) 76000 )				/* FM band Start freq in kHz for Brazil Market  */
#define AMFM_APP_SOUTH_AMERICA_MARKET_FM_ENDFREQ				((Tu32) 107900 )			/* FM band End freq in kHz for Brazil Market    */				
#define AMFM_APP_SOUTH_AMERICA_MARKET_FM_STEPSIZE				((Tu32) 100 )				/* FM band Step size in kHz for Brazil Market	*/


#endif /* End of AMFM_APP_MARKET_H  */
