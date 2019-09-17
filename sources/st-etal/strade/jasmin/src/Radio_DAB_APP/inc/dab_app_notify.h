/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file dab_app_notify.h 																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains notify API's for DAB Application.								*
*																											*
*																											*
*************************************************************************************************************/

#ifndef __DAB_APP_NOTIFY_H__
#define __DAB_APP_NOTIFY_H__

/** \file */
/** \page DAB_APP_NOTIFY_top DAB Application Notify  package

\subpage DAB_APP_NOTIFY_Overview
\n
\subpage DAB_APP_NOTIFY_API_Functions
\n
*/

/**\page DAB_APP_NOTIFY_Overview Overview   
    \n
     DAB Application Notify package consists of Notify API's.   
    \n\n
*/

/** \page DAB_APP_NOTIFY_API_Functions API Functions 
    <ul>
        <li> #DAB_App_Notify_STLUpdated         : Notification for station list change  . </li>
        <li> #DAB_App_Notify_FrequencyChange    : Notification for frequency(in KHz) changes while scanning process.</li>
		<li> #DAB_App_Notify_TunerStatus        : Notification which provides information of current tuned ensemble and service. </li>	
    </ul>
*/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	<b>Brief   </b>		\n	Notification for station list change.\n\n
*	<b>Details   </b>  	  \n	The DAB Application retrieves ensemble, service, service component list from shared memory between\n
*							DAB Application and Tuner Ctrl. The DAB Application updates the sorted station list in the shared memory between\n
*							Radio Application and DAB Application. After updating the station list, this notification is delivered to Radio Application.\n 
*   \param[in]				e_SharedMemoryType is a enum if type #Te_RADIO_SharedMemoryType.
*   \param[out]				None
*   \pre					DAB Application component is active.\n
*   \post					Radio Application is notified about changes in the station list.\n
*	<b>ErrorHandling </b>		NA\n
* 
******************************************************************************************************/

void DAB_App_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_SharedMemoryType);

/*!
*   \brief 				Notification for frequency(in KHz) changes while scanning process.
*   \param[in]				u32_Frequency - frequency(in KHz) of each tuned ensemble.
*   \param[out]				None
*   \pre		DAB Application component is active.
*   \description 			This API notifies Radio Application about each tuned ensemble(frequency in KHz) while scanning is in process.
*   \post			The corresponding frequency(in KHz) information is delivered to Radio Application.\n
*   \ErrorHandling    		NA
* 
*/
/******************************************************************************************************/

void DAB_App_Notify_FrequencyChange(Tu32 u32_Frequency, Tu8 *pu8_ChannelName);

/*****************************************************************************************************/
/**	 \brief 				Notification which provides information of current tuned ensemble and service..
*   \param[in]				st_TunerStatusNotify - structure contains information  of current tuned ensemble and service\n
*							like(frequency in KHz, Reception quality, Audio Status, Mute status,labels for ensemble, service and component)
*   \param[out]				None
*   \pre		DAB Application component is active.
*   \details 				The DAB Application provides all relevant information (frequency in KHz, Reception quality, Audio Status, Mute status,\n
*							labels for ensemble, service and component) of the current ensemble and service to the Radio Application.
*   \post			Radio Application is notified with all information of current ensemble and service.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/

void DAB_App_Notify_TunerStatus(Ts_DAB_APP_Status_Notification st_TunerStatusNotify , Te_DAB_App_SignalStatus e_DAB_App_SignalStatus);


void DAB_App_Notify_PICodeList(Ts_DAB_PICodeList st_DAB_PICodeList, Tu8 u8_QualityMin, Tu8 u8_QualityMax , Tu32 Sid , Tu8 Linking_check);


void DAB_App_Notify_BestPI(Tu16 PICode, Tu8 u8_Quality, Te_DAB_App_BestPI_Type e_BestPI_Type);


void DAB_App_Notify_DABtoFM_LinkingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus);

void DAB_App_Notify_PIQuality(Tu8 u8_Quality);

void DAB_App_Notify_Hardlinks_Status(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus);

void DAB_App_Notify_DABtoFM_BlendingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus);

void DAB_App_Notify_FMtoDAB_linked_Station(Ts_DAB_App_CurrentStationInfo st_FM_DAB_Linked_station);

void DAB_App_Notify_FMtoDAB_linking_Status(Tu8 Quality, Te_DAB_APP_fmdab_linkstatus e_FM_DAB_Linking_status);

void DAB_App_Notify_DLSData(Ts_DAB_DLS_Data st_DLS_Data);

void DAB_App_Notify_SLSData(void);

void DAB_App_Notify_ReConfiguration(Ts_DAB_App_Tunable_StationInfo st_tunableinfo);

void DAB_App_Notify_AnnoIndication(Te_DAB_App_AnnoIndication e_DAB_App_AnnoIndication);

void DAB_App_Notify_ComponentStatus(Te_RADIO_Comp_Status e_ComponentStatus);

void DAB_App_Notify_AMFMTunerStatus(Te_RADIO_Comp_Status e_AMFMTUNERStatus) ;

void DAB_App_Notify_SameChannelAnnoStatus(Te_DAB_App_AnnoIndication e_DAB_App_AnnoIndication, Tu8 u8_SameChannelSubChId, Tu8 u8_SameChannelClusterid) ;

void DAB_APP_Notify_StartBackgroundScan(void);
void DAB_App_Notify_StationNotAvail_StrategyStatus(Te_DAB_App_StationNotAvailStrategyStatus  e_DAB_App_StationNotAvailStrategyStatus);

void DAB_App_Notify_UpdatedLearnMem_AFStatus(Te_DAB_App_LearnMemAFStatus e_DAB_App_LearnMemAFStatus, Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo);

void DAB_App_Notify_AF_SIgLow(void);

void DAB_App_Notify_DAB_DAB_Status(Te_DAB_APP_DAB_DAB_Status e_DAB_APP_DAB_DAB_Status);

void DAB_App_Notify_SignalStatus(Te_DAB_App_SignalStatus e_DAB_App_SignalStatus);

void DAB_App_Notify_FMDAB_Stop_Type(Te_Dab_App_FmtoDAB_Reqstatus e_FmtoDAB_Reqstatus);
void DAB_App_Notify_SW_HW_Version(Ts_DAB_APP_GetVersion_Reply st_Verion_Number);

void DAB_App_Notify_AutoScan_PlayStation(Ts_DAB_App_CurrentStationInfo st_CurrentStationInfo);
/******************************************************************************************************/

#endif /* __DAB_APP_NOTIFY_H__ */

/*=============================================================================
    end of file
=============================================================================*/