/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file dab_app_response.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains response API's for DAB Application.								*
*																											*
*																											*
*************************************************************************************************************/

#ifndef __DAB_APP_RESPONSE_H__
#define __DAB_APP_RESPONSE_H__


/** \file */
/** \page DAB_APP_RESPONSE_top DAB Application Reponse package

\subpage DAB_APP_RESPONSE_Overview
\n
\subpage DAB_APP_RESPONSE_API_Functions
\n
*/

/**\page DAB_APP_RESPONSE_Overview Overview   
    \n
     DAB Application Reponse package consists of Reponse API's which are called by DAB Application in response to Radio Manager Request.   
    \n\n
*/

/** \page DAB_APP_RESPONSE_API_Functions API Functions 
    <ul>
        <li> #DAB_App_Response_Startup         : Acknowledgement for Start of DAB Application. </li>
        <li> #DAB_App_Response_GetStationList       : Acknowledgement for Update StationList request.</li>
		<li> #DAB_App_Response_Shutdown         : Acknowledgement for Shut-down of DAB Application. </li>
		<li> #DAB_App_Response_SelectBand         : Acknowledgement of the Select DAB band request. </li>
		<li> #DAB_App_Response_DeSelectBand         : Acknowledgement of the DeSelect DAB band request. </li>
		<li> #DAB_App_Response_PlaySelectSt         : Acknowledgement for the Tune to station in station list request. </li>
    </ul>
*/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_extern.h"
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement for Start of DAB Application.
*   \param[in]				e_Replystatus - the response status of the Start-up request #Te_RADIO_ReplyStatus
*   \param[out]				None
*   \pre		DAB Application component start-up request is completed.
*   \details 				This API acknowledges Radio Application on activating the DAB Application with the request status.
*   \post			DAB Application component is started.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Response_Startup(Te_RADIO_ReplyStatus e_Replystatus);

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement for Update StationList request.
*   \param[in]				e_Replystatus - the response status of the update station list request #Te_RADIO_ReplyStatus
*   \param[out]				None
*   \pre		DAB Get station list request is completed.
*   \details 				This API acknowledges Radio Application about response status of update station list request
*   \post			Nothing changed.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Response_GetStationList(Te_RADIO_ReplyStatus e_Replystatus);

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement for Shut-down of DAB Application.
*   \param[in]				e_Replystatus - the response status of the shut-down request #Te_RADIO_ReplyStatus
*   \param[out]				None
*   \pre		DAB Application component shut-down request is completed.
*   \details 				This API acknowledges Radio Application on deactivating the DAB Application with the shutdown request status.
*   \post			DAB Application component is not started.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Response_Shutdown(Te_RADIO_ReplyStatus e_Replystatus);

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement of the Select DAB band request.
*   \param[in]				e_Replystatus - the response status of the SelectBand request #Te_RADIO_ReplyStatus
*   \param[out]				None
*   \pre		DAB band select request is completed.
*   \details 				This API acknowledges Radio Application on selecting the DAB band with the select band request status.
*   \post			DAB is active source.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Response_SelectBand(Te_RADIO_ReplyStatus e_Replystatus);

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement of the DeSelect DAB band request.
*   \param[in]				e_Replystatus - the status response status of the DeSelectBand request #Te_RADIO_ReplyStatus 
*   \param[out]				None
*   \pre		DAB band deselect request is completed.
*   \details 				This API acknowledges Radio Application on deselecting the DAB band with the deselect band request status.
*   \post			DAB is not active source.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Response_DeSelectBand(Te_RADIO_ReplyStatus e_Replystatus);

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement for the Tune to station in station list request.
*   \param[in]				e_Replystatus - the response status for the tune request #Te_RADIO_ReplyStatus
*   \param[in]				st_CurrentStationInfo - the structure contains the currently tunned station information #Ts_DAB_App_CurrentStationInfo
*   \param[out]				None
*   \pre		DAB tune request is completed.
*   \details 				This API acknowledges the Radio Application about the tune station request and delivers the station information \n
*							a(frequency(in KHz),ensemble ID, ensemble label, service ID, service label, service component ID) to Radio \n
*							Application.
*   \post			Nothing changed.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Response_PlaySelectSt(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo);

/*****************************************************************************************************/
/*****************************************************************************************************/
/**	 \brief 				Acknowledgement for Service Seek Up/Down Request
*   \param[in]				e_Replystatus - Response status for Seek Up/Down Request
*   \param[in]				st_CurrentStationInfo - the structure contains the currently tunned station information #Ts_DAB_App_CurrentStationInfo
*   \param[out]				None
*   \pre					DAB package is active and tuned to a service in an ensemble.
*   \details 				Status for Seek Up/Down request is given to Radio application along with information about newly tuned station. 
*   \post					Radio application is updated with newly tuned station information.
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Response_ServiceCompSeekUpDown(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo);

/*-----------------------------------------------------------------------------
						DAB	Internal Response Function Declaration 
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement for Start Instance HSM request of DAB Application.
*   \param[in]				None
*   \param[out]				None
*   \pre		DAB Application Instance HSM is started.
*   \details 				This API acknowledges Start Instance HSM request of DAB Application.
*   \post			DAB Application component is started.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_Inst_App_Response_Startup(void);

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement  for Shut-down Instance HSM request of DAB Application.
*   \param[in]				None
*   \param[out]				None
*   \pre		DAB Application component is started.
*   \details 				This API acknowledges  shuts down Instance HSM of DAB Application.
*   \post			DAB Application component is stopped.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_Inst_App_Response_Shutdown(void);

void DAB_App_Response_EnableDABtoFMLinking(Te_RADIO_ReplyStatus e_Replystatus);

void DAB_App_Response_Cancel(Te_RADIO_ReplyStatus e_Replystatus);

void DAB_App_Response_TuneUpDown(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo);

/*****************************************************************************************************/
/**	 \brief 				Acknowledgement of cancel announcement request.
*   \param[in]				Te_RADIO_ReplyStatus
*   \param[out]				None
*   \pre					DAB band cancel announcement request is completed.
*   \details 				This API acknowledges cancel announcement request status.
*   \post					DAB cancel announcement has been done.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void DAB_App_Response_AnnoCancel(Te_RADIO_ReplyStatus e_Replystatus);

void DAB_App_Response_SetAnnoConfig(Te_RADIO_ReplyStatus e_Replystatus);

void DAB_App_Response_AFtune(Te_RADIO_ReplyStatus e_Replystatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo);

void DAB_App_Response_DABTUNERRestart(Te_RADIO_ReplyStatus e_Replystatus);
void DAB_App_Response_AF_List(Ts_DAB_App_AFList st_DAB_App_AFList);
void DAB_App_Response_Activate_Deactivate(Te_RADIO_ReplyStatus e_Replystatus);
void DAB_App_Response_DAB_AF_Settings(Te_RADIO_ReplyStatus e_Replystatus);
void DAB_App_Response_FactoryReset(Te_RADIO_ReplyStatus e_Replystatus);
void DAB_App_Response_Sort(void);

#endif /* __DAB_APP_RESPONSE_H__*/

/*=============================================================================
    end of file
=============================================================================*/





