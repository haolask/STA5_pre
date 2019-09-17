
/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app_freq_band.h 																		    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains range of frequencies supported in different region.				*
*																											*
*																											*
*************************************************************************************************************/
#ifndef __DAB_APP_FREQ_BAND_H__
#define __DAB_APP_FREQ_BAND_H__


/*-----------------------------------------------------------------------------
						File Inclusion
-----------------------------------------------------------------------------*/
#include "dab_app_types.h"

#define DAB_APP_EU_MAX_FREQUENCIES      ( 41u )  /* Maximum frequencies in Band III EUROPE region */
/*-----------------------------------------------------------------------------
					macros for Band III frequencies in EUROPE region
-----------------------------------------------------------------------------*/
#define DAB_APP_5A_EU      ( 174928u )
#define DAB_APP_5B_EU      ( 176640u )
#define DAB_APP_5C_EU      ( 178352u )
#define DAB_APP_5D_EU      ( 180064u )
#define DAB_APP_6A_EU      ( 181936u )
#define DAB_APP_6B_EU      ( 183648u )
#define DAB_APP_6C_EU      ( 185360u )
#define DAB_APP_6D_EU      ( 187072u )
#define DAB_APP_7A_EU      ( 188928u )
#define DAB_APP_7B_EU      ( 190640u )
#define DAB_APP_7C_EU      ( 192352u )
#define DAB_APP_7D_EU      ( 194064u )
#define DAB_APP_8A_EU      ( 195936u )
#define DAB_APP_8B_EU      ( 197648u )
#define DAB_APP_8C_EU      ( 199360u )
#define DAB_APP_8D_EU      ( 201072u )
#define DAB_APP_9A_EU      ( 202928u )
#define DAB_APP_9B_EU      ( 204640u )

#define DAB_APP_9C_EU      ( 206352u )
#define DAB_APP_9D_EU      ( 208064u )
#define DAB_APP_10A_EU     ( 209936u )
#define DAB_APP_10B_EU     ( 211648u )
#define DAB_APP_10C_EU     ( 213360u )
#define DAB_APP_10D_EU     ( 215072u )
#define DAB_APP_10N_EU     ( 210096u )
#define DAB_APP_11A_EU     ( 216928u )
#define DAB_APP_11B_EU     ( 218640u )
#define DAB_APP_11C_EU     ( 220352u )
#define DAB_APP_11D_EU     ( 222064u )
#define DAB_APP_11N_EU     ( 217088u )
#define DAB_APP_12A_EU     ( 223936u )
#define DAB_APP_12B_EU     ( 225648u )
#define DAB_APP_12C_EU     ( 227360u )
#define DAB_APP_12D_EU     ( 229072u )
#define DAB_APP_12N_EU     ( 224096u )
#define DAB_APP_13A_EU     ( 230784u )
#define DAB_APP_13B_EU     ( 232496u )
#define DAB_APP_13C_EU     ( 234208u )
#define DAB_APP_13D_EU     ( 235776u )
#define DAB_APP_13E_EU     ( 237488u )
#define DAB_APP_13F_EU     ( 239200u )


/*-----------------------------------------------------------------------------
					Type Definitions
-----------------------------------------------------------------------------*/

/**
 *  
 * @brief Structure with parameters to store channel name and frequency
 */
typedef struct 
{
    Tu8		au8_ChannelName[DAB_APP_MAX_CHANNEL_NAME_SIZE];         /**< this stores the channel name eg:5A,5B   */
    Tu32 u32_frequency;									/**< this stores the frequency in KHz*/
}Ts_dab_app_frequency_table;


#endif  /* __DAB_APP_FREQ_BAND_H__ */


/*=============================================================================
    end of file
=============================================================================*/