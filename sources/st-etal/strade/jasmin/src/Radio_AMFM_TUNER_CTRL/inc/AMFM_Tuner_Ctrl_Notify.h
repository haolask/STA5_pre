

/*=============================================================================
    start of file
=============================================================================*/
/****************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Notify.h																		         *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														     *
*  All rights reserved. Reproduction in whole or part is prohibited											     *
*  without the written permission of the copyright owner.													     *
*																											     *
*  Project              : ST_Radio_Middleware																		             *
*  Organization			: Jasmin Infotech Pvt. Ltd.															     *
*  Module				: SC_AMFM_TUNER_CTRL																     *
*  Description			:  The file contains notify API's for AMFM Tuner Control.                                   *
*                                                                                                                *																											*
*																											     *
******************************************************************************************************************/

#ifndef AMFM_TUNER_CTRL_NOTIFY_H_
#define AMFM_TUNER_CTRL_NOTIFY_H_

/** \file */
/** \page AMFM_TUNER_CTRL_NOTIFY_top AMFM Tuner control Notify  package

\subpage AMFM_TUNER_CTRL_NOTIFY_Overview
\n
\subpage AMFM_TUNER_CTRL_NOTIFY_API_Functions
\n
*/

/**\page AMFM_TUNER_CTRL_NOTIFY_Overview Overview   
    \n
     AMFM_TUNER_CTRL Notify package consists of Notify API's.   
    \n\n
*/

/** \page AMFM_TUNER_CTRL_NOTIFY_API_Functions API Functions 
    <ul>
        <li> #AMFM_Tuner_Ctrl_Notify_STLUpdated         : Notification for station list change  . </li>
        <li> #AMFM_Tuner_Ctrl_Notify_TunerStatus        : Notification which provides information of current tuned ensemble and service. </li>	
    </ul>
*/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/


#include "AMFM_Tuner_Ctrl_Types.h"
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
/**	\Brief					notifying the current tuned information of station.Notification may be specified using CurrentStationInfo structure.
*   \param[in]				CurrentStationInfo structure containing frequency,program type,program station name. 
*   \param[out]				None
*   \pre-condition			AMFM Tuner Control component is active.
*	\Details	  			This API notifies about quality,field strength etc, and/or any changes in parameters.							
*   \post-condition			Tuner status is notified.\n
*	\ErrorHandling			NA\n
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_NotifyTunerStatus(Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStationInfo);

/*****************************************************************************************************/
/**	\Brief					notifying the station list is updated into shared memory.
*   \param[in]				e_SharedMemoryType is a enum 
*   \param[out]				None
*   \pre-condition			AMFM Tuner Control component is active.
*	\Details	  			This API notifies about stations list is updated.After updating the station list, this notification is delivered to AMFM Application.\n 							
*   \post-condition			Sation list status is notified.\n
*	\ErrorHandling			NA\n
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_SharedMemoryType);


/*****************************************************************************************************/
/**	\Brief					notifying the current tuned Frequency.Notification may be specified using frequency parameter.
*   \param[in]				Frequency  parameter. 
*   \param[out]				None
*   \pre-condition			AMFM Tuner Control component is active.
*	\Details	  			This API notifies about frequency changes in parameters.							
*   \post-condition			current tuned freuqncy is notified.\n
*	\ErrorHandling			NA\n
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_Notify_CurrFrequency(Tu32 u32_freq);


/*****************************************************************************************************/
/**	\Brief					notifying the current tuned Frequency Field Strength.Notification may be specified using FS parameter.
*   \param[in]				Field Strength  parameter. 
*   \param[out]				None
*   \pre-condition			AMFM Tuner Control component is active.
*	\Details	  			This API notifies about field strength changes in parameters.							
*   \post-condition			current tuned freuqncy field strength is notified.\n
*	\ErrorHandling			NA\n
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_NotifyFS(Tu8 u8_FS);

void AMFM_Tuner_Ctrl_Notify_CurrQual(Ts_AMFM_Tuner_Ctrl_Interpolation_info Tuner_Qual);

void AMFM_Tuner_Ctrl_Notify_AMFMTuner_Abnormal(Te_RADIO_AMFMTuner_Status e_AMFMTunerErrType);

#endif /*AMFM_TUNER_CTRL_NOTIFY_H_*/

/*=============================================================================
    end of file
=============================================================================*/