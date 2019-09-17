/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_response.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declaration of all Response APIs which will be		*
*						  provided to Radio Manager Application 											*
*																											*
*************************************************************************************************************/
#ifndef AMFM_APP_RESPONSE_H
#define AMFM_APP_RESPONSE_H

/** \file */
/** \page AMFM_APP_RESPONSE_top AMFM Application Response

\subpage AMFM_APP_RESPONSE_Overview
\n
\subpage AMFM_APP_RESPONSE_API_Functions
\n
*/
/** \page AMFM_APP_RESPONSE_Overview Overview   
    \n
     AMFM Application Response consists of Response API's which are given to Radio Manager Application.   
    \n\n
*/

/** \page AMFM_APP_RESPONSE_API_Functions API Functions 
    <ul>
        <li> #AMFM_App_Response_Startup                 : startup response from the AMFM app to radio manager . </li>
        <li> #AMFM_APP_Response_Shutdown       	        : shutdown response from the AMFM app to radio manager . </li>
		<li> #AMFM_App_Response_GetStationList          : Response to Prepare a station list  from the AMFM app to radio manager. </li>
		<li> #AMFM_App_Response_SelectBand              : Response for requesting To select a AMFM Band/Mode. </li>
		<li> #AMFM_App_Response_DeSelectBand            : Response for requesting To Deselect a AMFM Band/Mode. </li>
		<li> #AMFM_App_Response_PlaySelectSt            : Response for requesting To Play a selected station. </li>
		<li> #AMFM_App_Response_SeekUpDown				: Response  for Seek Up/Down Request . </li>
		<li> #AMFM_App_Response_CancelDone				: Response  for Seek cancel Request. </li>
    </ul>
*/
/*-----------------------------------------------------------------------------
      File Inclusions
-----------------------------------------------------------------------------*/

#include "radio_mngr_app.h"
#include "amfm_app_msg.h"
#include "amfm_app_msg_id.h"
#include "cfg_types.h"



/*-----------------------------------------------------------------------------
    Public Function Declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**
 * \brief										startup response from the AMFM app to radio manager . 
 *
 * \param[in]		
 *                  e_StartupReplyStatus        Startup reply status from tuner control is parameterized. 
 *  
 * \param[out]      
 *                  None
 *
 * \pre-condition   							The AMFM App is in inactive state.
 *
 * \details									    This Service is for checking whether startup process has been successfully processed by AMFM app.
 *
 * \post-condition								The message sent should be processed by radio manager. 
 *
 * \error										NA.
 *
 */
/*****************************************************************************************************/
void AMFM_App_Response_Startup(Te_RADIO_ReplyStatus e_StartupReplyStatus);


/*****************************************************************************************************/
/**
 * \brief                                   Shutdown response from the AMFM app to radio manager . 
 *
 * \param[in]		
 *                  e_ShutdownReplyStatus   Shutdown reply status from tuner control is parameterized. 
 *
 * \retval			void
 *
 * \pre-condition						    The AMFM App is in active state.
 *
 * \details									This Service is for checking whether shutdown process has been successfully processed by AMFM app.
 *
 * \post-condition							The message sent should be processed by radio manager and system should be shut down. 
 *
 * \error									NA.
 */
/*****************************************************************************************************/
void AMFM_APP_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus);

/*****************************************************************************************************/
/** 
 * \brief											 Response to Prepare a station list  from the AMFM app to radio manager
 *
 * \param[in]		e_GetStationListReplyStatus      enum to indicate the get station list reply status is parametrized
 *
 *
 * \pre-condition 								     The AMFM mode is in active state.      
 *
 * \details							                This Response is for checking whether the station list is been updated and written to a buffer.
 *
 * \post-condition 						            In case of no errors, selected station list will be updated.
 *
 * \error 							                NA
 *
 */
/*****************************************************************************************************/
void AMFM_App_Response_GetStationList(Te_RADIO_ReplyStatus e_GetStationListReplyStatus);

/*****************************************************************************************************/
/**
 * \brief										Response for requesting To select a AMFM Band/Mode. 
 *
 * \param[in]		e_SelectBandReplyStatus		Select band  reply status from tuner control is parameterized. 
 * 
 *
 * \pre-condition								The AMFM Mode should be in inactive state.
 *
 * \details									    This  Response is for checking whether the  AM or FM mode is selcted. 
 *
 * \post-condition								In case no error occurs, the selectedmode is activated
 *
 * \error									    NA
 *
 */
/*****************************************************************************************************/
void AMFM_App_Response_SelectBand(Te_RADIO_ReplyStatus e_SelectBandReplyStatus);


/*****************************************************************************************************/
/**
 * \brief										Response for requesting To Deselect a AMFM Band/Mode. 
 *
 * \param[in]		De_SelectBandReplyStatus	DeSelect band  reply status from tuner control is parameterized. 
 * 
 *
 * \pre-condition								The AMFM Mode should be in active state.
 *
 *\details									    This  Response is for checking whether the  AM or FM mode is Deselcted. 
 *
 * \post-condition								In case no error occurs, the selectedmode is Deactivated
 *
 * \error										NA
 *
 */

/*****************************************************************************************************/
void AMFM_App_Response_DeSelectBand(Te_RADIO_ReplyStatus e_DeSelectBandReplyStatus);

/*****************************************************************************************************/
/**
 * \brief												Response for requesting To Play a selected station.
 *
 * \param[in]		e_PlaySelectStationReplyStatus		Play select station reply status from tuner control is parameterized.
 * \param[in]		Ts_AMFM_App_StationInfo				station that has been selected to play  is parameterized.
 * 
 *
 *  \pre-condition										The	AMFM Mode should be activated.
 *
 * \details											    This  Response is for checking whether selecting an AM or FM station to play is successful or not.
 *
 * \post-condition										In case no error occurs, the selected station is tuned 
 *
 * \error												NA
 *
 */
/*****************************************************************************************************/
void AMFM_App_Response_PlaySelectSt(Te_RADIO_ReplyStatus e_PlaySelectStationReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo);


/*****************************************************************************************************/
/**
 * \brief									Response  for Seek Up/Down Request 
 *
 * \param[in]	
 * 				e_SeekReplyStatus			Reply status for Seek Up/Down Request 
 * \param[in]	
 * 				st_CurrentStationInfo		Structure holds seek AM/FM Station Information 
 * \param[out]			
 *				st_amfm_app_res_msg     	structure holds Seek Up/Down response message  
 * 
 * \pre-condition							Seek Up/Down Request is processed completely.
 *
 * \details									Acknowledgement for checking whether seek operation is successfully done or not.
 * 											Reply Status of Seek Up/Down request is given to Radio Manager Application along 
 *											with seek station Info
 *
 * \post-condition						 	Tuned to next/previous station if any station is available
 *
 * \error									NA
 *
 */
/*****************************************************************************************************/
void AMFM_App_Response_SeekUpDown(Te_RADIO_ReplyStatus e_SeekReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo);

/*****************************************************************************************************/
/**
 * \brief									Response  for Seek cancel Request 
 *
 * \param[in]	
 * 				e_SeekReplyStatus			Reply status for Seek cancel Request 
 * \param[out]			
 *				st_amfm_app_res_msg     	structure holds Seek cancel done response message  
 * 
 * \pre-condition							The AMFM Mode should be in active state.
 *
 * \details									This API is used in order to acknowledge AM/FM seek operation 
 *											is cancel whenever band is changed.
 *
 * \post-condition							On going seek is cancelled and requested bband is activated.
 *
 * \error									NA
 *
 */
/*****************************************************************************************************/
void AMFM_App_Response_CancelDone(Te_RADIO_ReplyStatus e_CancelReplyStatus);


void AMFM_App_Response_TuneUpDown(Te_RADIO_ReplyStatus e_TuneUpDownReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo);

void AMFM_App_Response_AFSwitch(Te_RADIO_ReplyStatus e_AMFM_APP_AFSwitchReplyStatus);

void AMFM_App_Response_SetAFRegionalSwitch(Te_RADIO_ReplyStatus e_AMFM_APP_AFSwitchReplyStatus);

void AMFM_App_Response_TA_Switch(Te_RADIO_ReplyStatus e_TASwitchReplyStatus);

void AMFM_App_Response_FM_to_DAB_Switch(Te_RADIO_ReplyStatus e_AMFM_APP__FM_DABSwitchReplyStatus);

void AMFM_App_Response_AFTune(Te_RADIO_ReplyStatus e_AF_SwitchReplyStatus,Ts_AMFM_App_StationInfo st_CurrentStationInfo);
void AMFM_App_Response_Anno_Cancel_Done(Te_RADIO_ReplyStatus e_AMFM_APP_Anno_CancelReplyStatus);
void AMFM_App_Response_CT_Info(Te_RADIO_ReplyStatus e_GetCT_InfoReplystatus,Ts_AMFM_App_CT_Info st_CT_Info);
void AMFM_App_Response_FactoryReset(Te_AMFM_App_FactoryResetReplyStatus e_FactoryReset_ReplyStatus);
#endif /* End of AMFM_APP_RESPONSE_H */
