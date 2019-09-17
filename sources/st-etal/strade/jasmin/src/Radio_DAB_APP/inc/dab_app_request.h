/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file dab_app_request.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains request API's for DAB Application								*
*																											*
*																											*
*************************************************************************************************************/

#ifndef __DAB_APP_REQUEST_H__
#define __DAB_APP_REQUEST_H__


/** \file */
/** \page DAB_APP_REQUEST_top DAB Application Request package

\subpage DAB_APP_REQUEST_Overview
\n
\subpage DAB_APP_REQUEST_API_Functions
\n
*/

/**\page DAB_APP_REQUEST_Overview Overview   
    \n
     DAB Application Request package consists of Request API's which are called by Radio Manager Application.   
    \n\n
*/

/** \page DAB_APP_REQUEST_API_Functions API Functions 
    <ul>
        <li> #DAB_App_Request_Startup         : Start of DAB Application. </li>
        <li> #DAB_App_Request_GetStationList       : Request for updating station list.</li>
		<li> #DAB_App_Request_Shutdown         : Shut-down of DAB Application. </li>
		<li> #DAB_App_Request_SelectBand         : Request for selecting DAB band. </li>
		<li> #DAB_App_Request_DeSelectBand         : Request for Deselecting DAB band. </li>
		<li> #DAB_App_Request_PlaySelectSt         : Request for tuning to station in station list. </li>
    </ul>
*/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_types.h"
#include "cfg_variant_market.h"
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				Start of DAB Application.
*   \param[in]				e_Market is a enum if type #Te_DAB_App_Market.
*   \param[out]				None
*   \pre					DAB Application component is not started
*   \details 				This API starts the software component SC_DAB_APP, which DAB Application resources.\n
*							DAB Application triggers start up request to Tuner Ctrl.
*   \post					DAB Application component is started\n
*   \errhdl			   		NA
* 
******************************************************************************************************/

void DAB_App_Request_Startup(Te_DAB_App_Market    e_Market, Tu8 u8_SettingStatus, Tu8 u8_StartType);

/*****************************************************************************************************/
/**	 \brief 				Request for updating station list.
*   \param[in]				None
*   \param[out]				None
*   \pre					DAB Application component is active.
*   \details 				This API is called for updating the station list. DAB Application triggers scan request to Tuner Ctrl,\n
*							updates station list and sorts the station list according to the alphabetical order.
*   \post					Updated station list sorted in alphabetical order is delivered to Radio Application.\n
*   \errhdl		    		DAB Application reports error when Tuner Ctrl doesn't respond within X sec.
* 
******************************************************************************************************/

void DAB_App_Request_GetStationList(void);

/*****************************************************************************************************/
/**	 \brief 				Shut-down of DAB Application.
*   \param[in]				None
*   \param[out]				None
*   \pre					DAB Application component is started.
*   \details 				This API shuts down the software component SC_DAB_APP. This de-initializes DAB Application resources.\n
*							DAB Application triggers Shut-down request to Tuner Ctrl.
*   \post					DAB Application component is stopped.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Request_Shutdown(void);

/*****************************************************************************************************/
/**	 \brief 				Request for selecting DAB band.
*   \param[in]				None
*   \param[out]				None
*   \pre		DAB Application component is inactive.
*   \details 				This API is called for selecting DAB band, it activates DAB Application.\n
*							DAB Application triggers activate request to Tuner Ctrl.
*   \post			DAB Application component is active.\n
*   \ErrorHandling    		DAB Application reports error when Tuner Ctrl doesn't respond within X sec
* 
******************************************************************************************************/

void DAB_App_Request_SelectBand(void);

/*****************************************************************************************************/
/**	 \brief 				Request for Deselecting DAB band.
*   \param[in]				None
*   \param[out]				None
*   \pre		DAB Application component is active.
*   \details 				This API is called for deselecting DAB band, it deactivates DAB Application.\n
*							DAB Application triggers deactivate request to Tuner Ctrl.
*   \post			DAB Application component is inactive.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Request_DeSelectBand(void);

/*****************************************************************************************************/
/**	 \brief 				Request for tuning to station in station list.
*   \param[in]				u8_index -  Index of station in station list to be tuned
*   \param[out]				None
*   \pre		DAB Application component is active.
*   \details 				This API is called for tuning to the station in station list. DAB Application\n
*							triggers Play Select request to Tuner Ctrl.
*   \post			DAB station corresponding to selected station in station list is tuned.\n
*   \ErrorHandling    		DAB Application reports error when Tuner Ctrl doesn't respond within 3 sec.
* 
******************************************************************************************************/

void DAB_App_Request_PlaySelectSt(Tu32 u32_Freq, Tu16 u16_EId, Tu32 u32_Sid, Tu16 u16_SCIdI);


/*****************************************************************************************************/
/**	 \brief 				Request for tuning to next/previous available  (audio) service component within the same or next/previous ensemble.
* 	\param[in]				e_Direction   
*   \param[out]				None
*   \pre					DAB package is active and a service component is selected in an ensemble.
*   \details 				This API is called for tuning to the next/previous available program (audio) service
*                           within the same or next/previous ensemble according to the specified direction.
*                           When the last/first service in particular ensemble is reached, then system continues 
*						    to seek first/last service in next/previous available ensemble(frequency) respectively.		
*   \post					New DAB service station is tuned either in same or different ensemble.
*   \ErrorHandling    		DAB Application layer reports error when Tuner Control layer doesn't respond within 10sec(actual time to be decided w.r.t SOC).
* 
******************************************************************************************************/

void DAB_App_Request_ServiceCompSeekUpDown(Tu32 u32_Frequency, Te_RADIO_DirectionType e_Direction) ;

/*****************************************************************************************************/

/*****************************************************************************************************/

/*-----------------------------------------------------------------------------
						DAB	Internal Request Function Declaration 
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				Starts Instance HSM of DAB Application.
*   \param[in]				None
*   \param[out]				None
*   \pre		DAB Application main HSM is started.
*   \details 				This API starts the Instance HSM of DAB Application.
*   \post			DAB Application component is started. \n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_Inst_App_Request_Startup(void);

void DAB_App_Request_Cancel(Te_DAB_App_CancelType e_CancelType);

void DAB_Inst_App_Request_Shutdown(void);

void DAB_App_Request_EnableDABtoFMLinking(Te_DAB_App_DABFMLinking_Switch e_LinkingStatus);

void DAB_App_Request_TuneUpDown(Te_RADIO_DirectionType e_Direction);

void DAB_App_Internal_Msg_To_Anno_State(void);

void DAB_App_Request_GetSIDStation(Tu16 u16_FM_PI);

/*****************************************************************************************************/
/**	 \brief 				Request for cancel announcement.
*   \param[in]				None
*   \param[out]				None
*   \pre					DAB is in announcement or DAB package is active and a service component is selected in an ensemble.
*   \details 				This API is called for triggering an cancel announcement.
*   \post					DAB restore to original service. \n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void DAB_App_Request_AnnoCancel(void);

/*****************************************************************************************************/
/**	 \brief 				Request for announcement start or stop.
*   \param[in]				Tu16 u16_AnnoConfig
*							(0xFFFF( Announcement is ON and all type are supported)
*							0x0000( Announcement is off)
*							0x0001( Alarm  Type )
*							0x0002( Road traffic)
*							0x0004(Transport)
*							0x0008( Warning)
*							0x0010( News)
*							0x0020( Weather)
*							0x0040( Event)
*							0x0080(special event)
*							0x0100( Programme)
*							0x0200( sport)
*							0x0400( Financial))
*   \param[out]				None
*   \pre					DAB package is active and a service component is selected in an ensemble.
*   \details 				This API is called for triggering an announcement start or stop.
*   \post					DAB announcement is tuned or tunned announcement is stop in same ensemble. \n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void DAB_App_Request_SetAnnoConfig(Tu16 u16_AnnoConfig);

void DAB_App_Request_AFTune(Tu16 u16_Sid, Tu16 u16_SCId);

void DAB_App_Request_DABTunerRestart(void);
void DAB_App_Request_ENG_Mode(Te_DAB_APP_Eng_Mode_Request e_DAB_APP_Eng_Mode_Request);

void DAB_App_Request_Activate_Deactivate(Te_DAB_App_ActivateDeactivateStatus e_ActivateDeactivateStatus);

void DAB_App_Request_DAB_AF_Settings(Te_DAB_App_AF_Switch e_DAB_App_AF_Switch);

void DAB_App_Request_FactoryReset(void);

void DAB_App_Request_TuneByChannelName(Tu8 *au8_DABChannelName);

void DAB_App_Notify_Init_FMDAB_linking(void);

void DAB_App_Request_sort(void);

#endif /* __DAB_APP_REQUEST_H__ */

/*=============================================================================
    end of file
=============================================================================*/