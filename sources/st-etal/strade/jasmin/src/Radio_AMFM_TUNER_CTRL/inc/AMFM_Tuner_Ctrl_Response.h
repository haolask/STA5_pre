
/*=============================================================================
    start of file
=============================================================================*/
/*****************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Response.h																		      *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														      *
*  All rights reserved. Reproduction in whole or part is prohibited											      *
*  without the written permission of the copyright owner.													      *
*																											      *
*  Project              : ST_Radio_Middleware																		              *
*  Organization			: Jasmin Infotech Pvt. Ltd.															      *
*  Module				: SC_AMFM_TUNER_CTRL																      *
*  Description			: The file contains response API's of AMFM Tuner Control.                                    *
*                                                                                                                 *																											*
*																											      *
*******************************************************************************************************************/

#ifndef AMFM_TUNER_CTRL_RESPONSE_H_
#define AMFM_TUNER_CTRL_RESPONSE_H_

/** \file */
/** \page AMFM_TUNER_CTRL_RESPONSE_top AMFM Tuner Control Reponse package

\subpage AMFM_TUNER_CTRL_RESPONSE_Overview
\n
\subpage AMFM_TUNER_CTRL_RESPONSE_API_Functions
\n
*/

/**\page AMFM_TUNER_CTRL_RESPONSE_Overview Overview   
    \n
     AMFM Tuner Control Reponse package consists of Reponse API's which are called by AMFM Application in response to AMFM Application Request.   
    \n\n
*/

/** \page AMFM_TUNER_CTRL_RESPONSE_API_Functions API Functions 
    <ul>
    <li> #AMFM_Tuner_Ctrl_Response_Startup         : Response for Start of AMFM Tuner Control. </li>
    <li> #AMFM_Tuner_Ctrl_Response_GetStationList  : Response for Get StationList request.</li>
	<li> #AMFM_Tuner_Ctrl_Response_Shutdown        : Response for Shut-down of AMFM Tuner Control. </li>
	<li> #AMFM_Tuner_Ctrl_Response_Activate        : Response of the Activate AMFM band request. </li>
	<li> #AMFM_Tuner_Ctrl_Response_DeActivate      : Response of the DeActivate AMFM band request. </li>
	<li> #AMFM_Tuner_Ctrl_Response_Tune            : Response for the Tune to station. </li>
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
/**	 \brief 				API for sending the reply status of start up request.
*   \param[in]				e_StartupReplystatus - Enumeration depicting the status of start up request.
*   \param[out]				None
*   \pre-condition			AM/FM tuner control request start up is processed. 
*   \details 				This API is sent after startup request is completed.The API sends either success or failure status.
*   \post-condition			Start up response is sent.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Response_Startup(Te_RADIO_ReplyStatus e_StartupReplyStatus);

/*****************************************************************************************************/
/**	 \brief 				API for sending the reply status of shut down request.
*   \param[in]				e_ShutdownReplyStatus - Enumeration depicting the status of shut down request.														
*   \param[out]				None
*   \pre-condition			AM/FM tuner control request shut-down request is completed.
*   \details 				This API is sent after shutdown is requested.The API sends either success or failure status.
*   \post-condition			Shutdown response is sent.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus);

/*****************************************************************************************************/
/**	 \brief 				Tune response in AM/FM mode.Response may be specified using TuneReplyStatus and CurrentStationInfo parameters.
*   \param[in]				e_TuneReplyStatus- Enumeration depicting the status of Tune request.
*   \param[in]				st_CurrentStationInfo - the structure contains the currently tunned station information 
*   \param[out]				None
*   \pre-condition			AM/FM tune request is completed.
*   \details 				This function is  returned after tune request for sending success or failure status and current station information.
*   \post-condition			Tune response is sent.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Response_Tune(Te_RADIO_ReplyStatus e_TuneReplyStatus,Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStationInfo);

/*****************************************************************************************************/
/**	 \brief 				API for sending reply status of AM/FM tuner control activation request.
*   \param[in]				e_ActivateReplyStatus - Enumeration depicting the status of activation request.
*   \param[out]				None
*   \pre-condition			AM/FM band select activation is completed.
*   \details 				This API acknowledges Radio Application on selecting the DAB band with the select band request status.
*   \post-condition			AM/FM tuner control activation response is sent.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Response_Activate(Te_RADIO_ReplyStatus e_ActivateReplyStatus);

/*****************************************************************************************************/
/**	 \brief 				API for sending reply status of AM/FM tuner control deactivation request.
*   \param[in]				e_DeActivateReplyStatus - Enumeration depicting the status of deactivation request.
*   \param[out]				None
*   \pre-condition			Deactivation of AM/FM tuner control is processed.
*   \details 				The API is sent after AM/FM tuner control deactivation request is processed. The API sends either success or failure status.
*   \post-condition			AM/FM tuner control deactivation response is sent.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void AMFM_Tuner_Ctrl_Response_DeActivate(Te_RADIO_ReplyStatus e_DeActivateReplyStatus);

/*****************************************************************************************************/
/**	 \brief 				API for sending reply status of the service list  request.
*   \param[in]				e_StationListReplyStatus - Enumeration depicting the status of service list request.
*   \param[out]				None
*   \pre-condition			Get service list request is processed.
*   \details 				The API is sent after service list request is processed. The API sends either success or failure status.
*   \post-condition			Get service list response is sent.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_Response_GetStationList(Te_RADIO_ReplyStatus e_StationlistReplyStatus);

/*****************************************************************************************************/
/**	 \brief 				Seek response in AM/FM mode.Response may be specified using SeekReplyStatus and CurrentStationInfo parameters.
*   \param[in]				e_SeekReplyStatus- Enumeration depicting the status of Seek request.
*   \param[in]				st_CurrentStationInfo - the structure contains the currently tunned station information 
*   \param[out]				None
*   \pre-condition			AM/FM Seek request is completed.
*   \details 				This function is  returned after seek request for sending success or failure status and current station information.
*   \post-condition			Seek response is sent.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void AMFM_Tuner_Ctrl_Response_SeekUpDown(Te_RADIO_ReplyStatus e_SeekReplyStatus, Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStationInfo);

void AMFM_Tuner_Ctrl_Response_Cancel(Te_RADIO_ReplyStatus e_CancelReplyStatus, Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStationInfo);

void AMFM_Tuner_Ctrl_Response_FM_Check(Te_RADIO_ReplyStatus e_AF_CheckReplyStatus, Tu16 u16_pi);


void AMFM_Tuner_Ctrl_Response_LowSignal_FM_Check(Te_RADIO_ReplyStatus e_AF_CheckReplyStatus, Tu16 u16_pi);

void AMFM_Tuner_Ctrl_Response_AF_Update(Te_RADIO_ReplyStatus e_AF_UpdateReplyStatus,Tu32 u32_AFupdateFreq,Ts_AMFM_Tuner_Ctrl_Interpolation_info st_AF_Update_StationInfo);

void AMFM_Tuner_Ctrl_Send_EONTA(Tu16 EON_PI);

void AMFM_Tuner_Ctrl_EON_TAdata(Ts_AMFM_TunerCtrl_EON_Info EON_StationInfo);

void AMFM_Tuner_Ctrl_Send_TAStatus(Tu8 TA,Tu8 TP);

void AMFM_Tuner_Ctrl_Response_Announcement_Cancel(Te_RADIO_ReplyStatus e_CancelReplyStatus);
void AMFM_Tuner_Ctrl_Response_FactoryReset(Te_RADIO_ReplyStatus e_FactoryresetReplyStatus);
#endif /*AMFM_TUNER_CTRL_RESPONSE_H_*/

/*=============================================================================
    end of file
=============================================================================*/