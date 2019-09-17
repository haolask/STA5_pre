/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_notify.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declaration of all Notification APIs which send		*
*						  messages to Radio Manager Application												*
*																											*
*************************************************************************************************************/
#ifndef AMFM_APP_NOTIFY_H
#define AMFM_APP_NOTIFY_H

/** \file */
/** \page AMFM_APP_NOTIFY_top AMFM Application Notify package

\subpage AMFM_APP_NOTIFY_Overview
\n
\subpage AMFM_APP_NOTIFY_API_Functions
\n
*/

/** \page AMFM_APP_NOTIFY_Overview Overview   
    \n
     AMFM Application Notify package consists of Notify API's which are called by Radio Manager Application.   
    \n\n
*/

/** \page AMFM_APP_NOTIFY_API_Functions API Functions 
    <ul>
        <li> #AMFM_App_Notify_TunerStatus       : Tuner status notification from the AMFM app to radio manager . </li>
        <li> #AMFM_App_Notify_STLUpdated        : Station list updated notification from the AMFM app to radio manager .  </li>
    </ul>
*/


/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/

#include "amfm_app_msg.h"
#include "amfm_app_msg_id.h"
#include "radio_mngr_app.h"
#include "cfg_types.h"


/*-----------------------------------------------------------------------------
    Public Function Declarations
-----------------------------------------------------------------------------*/
/*****************************************************************************************************/
/**
 * \brief										Tuner status notification from the AMFM app to radio manager . 
 *
 * \param[in]		st_CurrentStationInfo        current selectd station information  from tuner control is parameterized. 
 *					e_SigQuality				 current selected stations quality.
 *
 * \pre-condition											The AMFM App is in active state and a station is requested to play.
 *
 * \details								This Service is to notify about the availability of the station from the AMFM app to radio manager.
 *
 * \post-condition										 NA. 
 *
 * \error										 Invalid output shall be returned when invalid input was received.
 */

/*****************************************************************************************************/
void AMFM_App_Notify_TunerStatus(Ts_AMFM_App_StationInfo st_current_station,Te_AMFM_App_SigQuality	e_SigQuality);


/*****************************************************************************************************/
/**
 * @brief										Station list updated notification from the AMFM app to radio manager . 
 *
 * @param[in]		e_SharedMemoryType          Shared memeory information  between AMFM app and radio manager is parameterized. 
 *
 * @retval			void
 *
 * @pre											 The station list should be updated in the shared memeory between AMFM app and tuner control .
 *
 * @description									 This notification is to indicate the radio manager that station list has been sorted and updated in the radio manager required format in the shared memory.
 *
 * @post									     Radio manager can read from shared memory and display it in the HMI. 
 *
 * @errhdl										 Invalid output shall be returned when invalid input was received.
 */
/*****************************************************************************************************/
void AMFM_App_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_SharedMemoryType);

/*****************************************************************************************************/
/**
 * \brief									Notifies about change of the frequency	during seek opearation 
 *
 * \param[in]	
 * 				u32_Frequency				Frequency in KHz
 * \param[out]			
 *				st_amfm_app_notify_msg     	structure holds CurrFrequency notification message.
 * 
 * \pre-condition							Seek opearation is started
 *
 * \details									This API notifies Radio Manager Application about change of frequency.
 *
 * \post-condition						 	Frequency change is notified 
 *
 * \error									NA
 *
 */
/*****************************************************************************************************/
void AMFM_App_Notify_CurrFrequency(Tu32 u32_Frequency);

void AMFM_App_Notify_BestPI(Tu16 u16_BestPI,Tu32 u32_Quality,Tu32 u32_BestPI_Freq,Te_AMFM_APP_BestPI_Type e_BestPI_Type,Tu8* pu8_PSN,Tu8 u8_DABFM_LinkingCharset);

void AMFM_App_Notify_BestPI_Changed(void);

void AMFM_App_Notify_PIQuality(Tu32  u32_Quality, Tu8* pu8_BestPI_PSN,Te_AMFM_App_PSNChangeFlag e_PSNChangeFlag,Tu8 u8_DABFM_LinkingCharset);

void AMFM_App_Notify_LinkingStatus(Te_RADIO_DABFM_LinkingStatus	e_LinkingStatus);

void AMFM_App_Notify_Find_SID(Tu16 u16_pi);
void AMFM_App_Notify_AF_Status(Te_AMFM_App_AF_Status e_AF_Status);

void AMFM_App_Notify_FMtoDAB_SIDStaion_Quality(Tu32 u32_Quality,Te_AMFM_DAB_PI_TYPE	e_DAB_PI_TYPE);
void AMFM_App_Notify_Initiate_FM_DAB_Follow_Up(void);
void AMFM_App_Notify_Announcement_Status(Te_AMFM_APP_Announcement_Status e_Announcement_Status);

void AMFM_App_Notify_Stop_DAB_to_FM_Linking(void);

void AMFM_App_Notify_AMFMTuner_Abnormal(Te_RADIO_AMFMTuner_Status e_AMFMTunerStatus);

void AMFM_App_Notify_DABStatus(Te_RADIO_Comp_Status e_DABTunerStatus);

void AMFM_App_Notify_ENG_AFList(Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info);


void AMFM_App_Notify_StartBackgroundScan(void);
void AMFM_App_Notify_UpdatedLearnMem_AFStatus(Te_AMFM_App_LearnMemAFStatus e_AFTune_ReplyStatus,Ts_AMFM_App_StationInfo st_current_station);

void AMFM_App_Notify_StationNotAvail_StrategyStatus(Te_AMFM_App_StationNotAvailStrategyStatus e_StationNotAvailStrategyStatus);
void AMFM_App_Notify_Stop_FM_DAB_Follow_Up(Te_AMFM_FMDAB_STOP_status e_AMFM_FMDAB_STOP_status);
void AMFM_App_Notify_AF_SigLost(void);

#endif /* End of AMFM_APP_NOTIFY_H */