/*=============================================================================
    start of file
=============================================================================*/
/****************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Request.h																		         *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														     *
*  All rights reserved. Reproduction in whole or part is prohibited											     *
*  without the written permission of the copyright owner.													     *
*																											     *
*  Project              : ST_Radio_Middleware																		             *
*  Organization			: Jasmin Infotech Pvt. Ltd.															     *
*  Module				: SC_AMFM_TUNER_CTRL																     *
*  Description			: The file contains requesting api's to AMFM_Tuner_Ctrl.                                 *
                                                                                                                 *																											*
*																											     *
******************************************************************************************************************/

#ifndef AMFM_TUNER_CTRL_REQUEST_H_
#define AMFM_TUNER_CTRL_REQUEST_H_

/** \file */
/** \page AMFM_TUNER_CTRL_REQUEST_top AMFM_Tuner_Ctrl Request package

\subpage AMFM_TUNER_CTRL_APP_REQUEST_Overview
\n
\subpage AMFM_TUNER_CTRL_REQUEST_API_Functions
\n
*/

/**\page AMF_TUNER_CTRL_REQUEST_Overview Overview   
    \n
     AMFM tuner control Request package cosists of Request API's which are called by AMFM Application.   
    \n\n
*/

/** \page AMFM_TUNER_CTRL_REQUEST_API_Functions API Functions 
    <ul>
        <li> #AMFM_Tuner_Ctrl_Request_Startup         : Start of AMFM tuner control. </li>
        <li> #AMFM_Tuner_Ctrl_Request_GetStationList  : Request for updating station list.</li>
		<li> #AMFM_Tuner_Ctrl_Request_Shutdown        : Shut-down of AMFM tuner control. </li>
		<li> #AMFM_Tuner_Ctrl_Request_Activate        : Request for selecting AMFM band. </li>
		<li> #AMFM_Tuner_Ctrl_Request_DeActivate      : Request for Deselecting AMFM band. </li>
		<li> #AMFM_Tuner_Ctrl_Request_Tune            : Request for tuning to station. </li>
    </ul>
*/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "AMFM_Tuner_Ctrl_Types.h"
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				Start of AMFM tuner control.
*   \param[in]				None.
*   \param[out]				None.
*   \pre        			AM/FM tuner control is in Inactive state.
*   \details 				This API is called for start up of tuner control.This API initializes the tuner. 						
*   \post       			AM/FM tuner control is in Active state\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_Request_Startup(Te_AMFM_Tuner_Ctrl_Market e_Market);


/*****************************************************************************************************/
/**	 \brief 				Shutdown of AM/FM tuner control.
*   \param[in]				None.
*   \param[out]				None.
*   \pre        			AM/FM tuner control is in active state.
*   \details 				This API is called for shut down of AM/FM tuner control.This API de-initialises the tuner. 						
*   \post       			AM/FM tuner control is in Inactive state\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Request_Shutdown(void);

/*****************************************************************************************************/
/**	 \brief 				Requesting to tune to a station.The station may be AM/FM band and frequency(KHZ) can be parameterized.
*   \param[in]				e_Band - Selecting the AM or FM band.
*   \param[out]				Frequency for tuning.
*   \pre        			The AMFM package has to be started.
*   \details 				This API is called for Tune to a frequency  of AM/FM tuner control.						
*   \post       			In case no error occurs,tuned to the station\n
*   \ErrorHandling    		In case Frequency is not in supported range and/or SOC doesn't respond within the time the System reports an error.
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Request_Tune(Te_AMFM_Tuner_Ctrl_Band e_Band,Tu32 u32_freq);

/*****************************************************************************************************/
/**	 \brief 				Requesting activation of the AM/FM tuner control.
*   \param[in]				e_Band - Selecting the AM or FM band.
*   \param[out]				None.
*   \pre        			AM/FM tuner control is in inactive state.
*   \details 				This API is called for activating the AM/FM tuner control.The API initialises all relevant parameters 
*                           for the activation of AM/FM tuner control. 						
*   \post       			AM/FM tuner control is in active state\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Request_Activate(Te_AMFM_Tuner_Ctrl_Band e_Band);

/*****************************************************************************************************/
/**	 \brief 				Requesting deactivation of the AM/FM tuner control.
*   \param[in]				e_Band - Selecting the AM or FM band.
*   \param[out]				None.
*   \pre        			AM/FM tuner control is in active state.
*   \details 				This API is called for deactivation of AM/FM tuner control.The API de-initialises all relevant
*                           parameters for the deactivation of AM/FM tuner control. 						
*   \post       			AM/FM tuner control is in inactive state\n
*   \ErrorHandling    		In case tuner doesn't respond for Select Band Request,then the system reports an error.The 
*						    error is reported via  AM/FM _Tuner_Ctrl_Response_DeActivate
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Request_DeActivate(Te_AMFM_Tuner_Ctrl_Band e_Band);

/*****************************************************************************************************/
/**	 \brief 				Requesting AM/FM station information for station list.
*   \param[in]				e_Band - Selecting the AM or FM band.
*   \param[in]              U32_Startfreq - start frequency for scan.
*   \param[in]              U32_Startfreq - upper frequency for scan.
*   \param[in]              U32_Lowerfreq - lower frequency for scan.
*   \param[in]              U32_Stepsize - step size for scan.
*   \param[out]				None.
*   \pre        			AM/FM tuner control is in active state.
*   \details 				This API is called for AM/FM station list and tuner controller triggers scan to soc.
*   \post       			Updated stations information delivered to AM/FM Application\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
//void AMFM_Tuner_Ctrl_Request_GetStationList(Tu16 U32_Startfreq,Tu16 U32_Upperfreq,Tu16 U32_Lowerfreq,Tu32 U32_Stepsize,Te_AMFM_Tuner_Ctrl_Band e_Band );
void AMFM_Tuner_Ctrl_Request_GetStationList(Tu32 U32_Lowerfreq,Tu32 U32_Startfreq,Tu32 U32_Upperfreq,Tu32 U32_Stepsize,Te_AMFM_Tuner_Ctrl_Band e_Band ,Te_AMFM_Scan_Type Scantype );


/*****************************************************************************************************/
/**	 \brief 				Requesting to seek to a station.
*   \param[in]				e_Direction - Direction  of seek.
*   \param[in]				e_Band - Selecting the AM or FM band.
*   \param[in]              U32_Startfreq - start frequency for scan.
*   \param[in]              U32_Upperfreq - upper frequency for scan.
*   \param[in]              U32_Lowerfreq - lower frequency for scan.
*   \param[in]              U32_Stepsize - step size for scan.
*   \param[out]				None.
*   \pre        			The AMFM package has to be started.
*   \details 				This API is called for Seek to next/pervioes sation of AM/FM tuner control.						
*   \post       			In case no error occurs,tuned to the station\n
*   \ErrorHandling    		In case Frequency is not in supported range and/or SOC doesn't respond within the time the System reports an error.
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_Request_SeekUpDown(Te_RADIO_DirectionType e_Direction, Tu32 U32_Lowerfreq, Tu32 U32_Startfreq, Tu32 U32_Upperfreq, Tu32 u32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band );

/*****************************************************************************************************/
/**	 \brief 				Quality of current tuned station.
*   \param[in]				None.
*   \param[out]				None.
*   \pre        			AM/FM tuner control is tuned to a station.
*   \details 				This API is called for quality of tuned station. 						
*   \post       			AM/FM tuner control is in Inactive state\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_Request_Bg_Quality(void);
void AMFM_Tuner_Ctrl_Request_AF_Update(Tu32 u32_freq);
void AMFM_Tuner_Ctrl_Request_LowSignal_FM_Check(Tu16 u16_freq);
void AMFM_Tuner_Ctrl_Request_Cancel(void);
void AMFM_Tuner_Ctrl_Request_Announcement_Cancel(void);
void AMFM_Tuner_Ctrl_Request_FM_Jump(Tu32 u32_freq);
void AMFM_Tuner_Ctrl_Request_FM_Check(Tu32 u32_freq);
void AMFM_Tuner_Ctrl_Request_PISeek(Tu32 u32_Lowerfreq, Tu32 u32_Startfreq, Tu32 u32_Upperfreq, Tu32 u32_Stepsize,Tu16 u16_pi);
void AMFM_Tuner_Ctrl_Request_Factory_Reset(void);
#endif /*AMFM_TUNER_CTRL_REQUEST_H_*/

/*=============================================================================
    end of file
=============================================================================*/