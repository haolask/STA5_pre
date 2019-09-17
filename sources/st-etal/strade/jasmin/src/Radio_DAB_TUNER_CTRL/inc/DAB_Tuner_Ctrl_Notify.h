/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/* Copyright (c) 2016, Jasmin Infotech Private Limited.
*  All rights reserved. Reproduction in whole or part is prohibited
*  without the written permission of the copyright owner.
*
*  Project              :  ST_Radio_Middleware
*  Organization			:  Jasmin Infotech Pvt. Ltd.
*  Module				:  Radio DAB Tuner Control
*  Description			:  This file contains API declarations related to DAB Tuner Control notifications.
*
*	
**********************************************************************************************************/
/** \file   DAB_Tuner_Ctrl_messages.h 
	\brief  DAB_Tuner_Ctrl_messages.h This file contains API declarations related to 
	                                  DAB Tuner Control notifications.
*********************************************************************************************************/

#ifndef DAB_TUNER_CTRL_NOTIFY_
#define DAB_TUNER_CTRL_NOTIFY_
/** \file */
/** \page DAB_TUNER_CTRL_Notify_top DAB Tuner Control Notification package

\subpage DAB_TUNER_CTRL_Notify_Overview
\n
\subpage DAB_TUNER_CTRL_Notify_Functions
\n
*/

/**\page DAB_TUNER_CTRL_Notify_Overview Overview   
    \n
     DAB Tuner Control Notification package consists of notification API's which are called DAB application.   
    \n\n
*/

/** \page DAB_TUNER_CTRL_Notify_Functions API Functions 
    <ul>
        <li> #DAB_Tuner_Ctrl_Notify_FrequencyChange         : Frequency change notification. </li>
        <li> #DAB_App_Notify_STLUpdated       : Station list updated notification.</li>

    </ul>
*/
/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_HAL_Interface.h" 
#include "cfg_types.h"

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/

#define DAB_TUNER_CTRL_TUNE_STATUS_NOTIFICATION         (0x6A00u) //(0x1A)
#define DAB_TUNER_CTRL_MAIN_MSGID_UP_NOT                (0x6A01u) //(0x25)
#define DAB_TUNER_CTRL_SCAN_NOTIFICATION                (0x6A02u) //(0x2E)
#define DAB_TUNER_CTRL_GET_COMPONENT_LIST_REPLY		    (0x6A03u)
#define AUDIO_STATUS_REPLY								(0x6A04u)
#define AUDIO_STATUS_NOTIFICATION						(0x6A05u)
#define BER_STATUS_REPLY								(0x6A06u)
#define BER_STATUS_NOTIFICATION							(0x6A07u)
#define	RSSI_REPLY										(0x6A08u)
#define RSSI_NOTIFICATION								(0x6A09u)
#define SNR_REPLY										(0x6A0Au)
#define SNR_NOTIFICATION								(0x6A0Bu)
#define FIG_DATA_NOTIFICATION							(0x6A0Cu) 
#define DAB_TUNER_CTRL_AUDIO_STATUS_NOTIFICATION		(0x6A0Du)										    
#define BEST_PI_RECEIVED_NOTIFICATION					(0x6A0Eu)
#define DAB_TUNER_CTRL_DAB_FM_LINKING_STATUS_NOTIFYID	(0x6A0Fu)
#define DAB_TUNER_CTRL_PI_QUALITY_NOTIFYID				(0x6A10u)	
#define DAB_TUNER_CTRL_FREQUENCY_CHANGE_NOTIFICATION	(0x6A11u)
#define AUDIO_TIMER_NOTIFICATION						(0x6A12u)
#define	DLS_DATA_NOTIFICATION							(0x6A13u)
#define	PROGRAMME_SERV_LIST_CHANGED_NOTIFICATION		(0x6A14u)
#define	SERVICE_PROPERTY_CHANGED_NOTIFICATION			(0x6A15u)
#define SYSTEM_CONTORL_NOTIFICATION						(0x6A16u)
#define DAB_TUNER_CTRL_AMFMTUNER_STATUS_NOTIFYID		(0x6A17u)
#define DAB_TUNER_CTRL_DAB_SAMECHANNELANNO_NOTIFYID		(0x6A18u)
#define SYSTEM_ERROR_NOTIFICATION						(0x6A19u)
#define SYSTEM_MONITOR_NOTIFICATION_TIMEOUT				(0x6A1Au)	
#define	DAB_TUNER_CTRL_DRIFT_TRACKING_NOTIFICATION		(0x6A1Bu)							    
#define DAB_TUNER_CTRL_BACKGROUND_SCAN_START_NOTIFYID	(0x6A1Cu)
#define DAB_TUNER_CTRL_STRATERGY_STATUS_NOTIFYID		(0x6A1Du)
#define DAB_TUNER_CTRL_START_TIMESTRETCH_NOTIFICATION	(0x6A1Eu)
#define DAB_TUNER_CTRL_INIT_FMDAB_LINKING_NOTIFYID		(0x6A1Fu)
#define DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION			(0x6A20u)
#define DAB_TUNER_CTRL_AUTOSEEK_NOTIFICATION_TIMEOUT	(0x6A21u)
#define DAB_TUNER_CTRL_AUTOSEEK_STOP_NOTIFY_TIMEOUT		(0x6A22u)
#define SLS_DATA_NOTIFICATION                           (0x6A23u)
/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/

void DAB_Tuner_Ctrl_Notify_Synchronisation(Tu8 u8_ChannelSync);
/*****************************************************************************************************/
/**	 \brief                 API for notifying when DAB tuner control tunes to new frequency.
*   \param[in]				None
*   \param[out]				u32_Frequency
*   \pre-condition			Tuned to an ensemble.
*   \details                This API notifies about change  in frequency  information  only the tuning 
                            to the given Frequency was done.DAB signal detection is still ongoing.i.e 
							searching for new Ensemble/Searching new ensemble.
*   \post-condition			Frequency change is notified.
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Notify_FrequencyChange(Tu32 u32_Frequency);


/*****************************************************************************************************/
/**	 \brief                 API for notifying DAB Application when the current ensemble service and 
                            service-component information is updated.
*   \param[in]				None
*   \param[out]				st_currentEnsembleInfo
*   \pre-condition			Tuned to an ensemble.
*   \details                This API notifies about the update of current ensemble service and service-component 
                            information.
*   \post-condition			Current ensemble related information is updated.
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/
//void DAB_Tuner_Cntrl_Notify_CurrentEnsInfoUpdated(Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleInfo);

/*****************************************************************************************************/
/**	 \brief                 API for notifying DAB Application when station list is updated.
*   \param[in]				None
*   \param[out]				component
*   \pre-condition			Station list is updated.
*   \details                This API notifies about the update of station list updated information.
*   \post-condition			Station list update is notified.
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/

void DAB_Tuner_Ctrl_Notify_TunerStatus(Ts_Tuner_Status_Notification st_tunerstatus);

void DAB_Tuner_Ctrl_Notify_PICodeList(Ts_PI_Data st_DAB_PICodeList, Tu8 u8_QualityMin, Tu8 u8_QualityMax,Tu32 Tuned_Sid , Tu8 u8_linktype);
/*****************************************************************************************************/
/**	 \brief                 API for notifying when station list is updated.
*   \param[in]				None
*   \param[out]				e_Tuner_SharedMemoryType
*   \pre-condition			Station list is updated.
*   \details                This API notifies about the update of station list updated information.
*   \post-condition			Station list update is notified.
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_Tuner_SharedMemoryType);
void DAB_Tuner_Ctrl_Notify_DABtoFM_BlendingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus);
void DAB_Tuner_Ctrl_Notify_DLSdata(Ts_dab_DLS_data st_Dynamicdata);
void DAB_Tuner_Ctrl_Notify_SLSdata(void);
void DAB_Tuner_Ctrl_Notify_ReConfiguration(Te_Tuner_Ctrl_ReConfigType e_ReConfigType,Tu32 u32_Frequency, Tu16 u16_Eid, Tu32 u32_SId, Tu16 u16_SCId);

void DAB_Tuner_Ctrl_Notify_BestPI(Tu16 PICode, Tu8 u8_Quality , Te_Tuner_Ctrl_BestPI_Type e_BestPI_Type);

void DAB_Tuner_Ctrl_Notify_StationNootAvail_StrategyStatus(Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus);

/*****************************************************************************************************/
/**	 \brief                 API for notifying DAB Application when there is announcement.
*   \param[in]				Announcement status - start / stop 
                            Ts_DAB_Tuner_Anno_Swtch_Info - switching information
*   \param[out]				none
*   \pre-condition			Announcment switching info is sent.
*   \details                This API notifies about the announcment start and stop
*   \post-condition			Announcment notification is updated
*   \ErrorHandling    		N/A.
* 
******************************************************************************************************/
void DAB_Tuner_Ctrl_Notify_AnnoStatus(Tu8 u8_announcement_status,Ts_DAB_Tuner_Anno_Swtch_Info st_DAB_Tuner_Anno_Swtch_Info);

void DAB_Tuner_Ctrl_Notify_AnnoStationInfo(Ts_Tuner_Ctrl_CurrentEnsembleInfo st_CurrentTunedAnnoInfo);

void DAB_Tuner_Ctrl_Notify_DABtoFM_LinkingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus);

void DAB_Tuner_Ctrl_Notify_PIQuality(Tu8 u8_Quality);

void DAB_Tuner_Ctrl_Notify_FMtoDAB_linked_Station(Ts_dab_tuner_ctrl_fmdab_linkinfo st_FM_DAB_Stationinfo);

void DAB_Tuner_Ctrl_Notify_FMtoDAB_linking_Status(Tu8 Quality, Te_dab_tuner_ctrl_fmdab_linkstatus e_LinkingStatus);

void DAB_Tuner_Ctrl_Notify_Hardlinks_Status(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus);
void DAB_Tuner_Ctrl_Notify_AnnoSignalLoss_Status(void);
void DAB_Tuner_Ctrl_Notify_ComponentStatus(Te_RADIO_Comp_Status e_ComponentStatus);
void DAB_Tuner_Ctrl_Notify_AMFMTunerStatus(Te_RADIO_Comp_Status e_AMFMTUNERStatus);
void DAB_Tuner_Ctrl_Notify_StartBackgroundScan(void);
void DAB_Tuner_Ctrl_Notify_CompListStatus(Tu32 u32_Frequency, Tu16 u16_Eid, Te_RADIO_ReplyStatus e_ReplyStatus);

void DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(Te_DAB_Tuner_Ctrl_LearnMemAFStatus e_DAB_Tuner_Ctrl_LearnMemAFStatus ,Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData);
void DAB_Tuner_Ctrl_Notify_SignalStatus(Te_DAB_Tuner_Ctrl_SignalStatus e_DAB_Tuner_Ctrl_SignalStatus);
void DAB_Tuner_Ctrl_Notify_DAB_DAB_Status(Te_DAB_Tuner_Ctrl_DAB_DAB_Status 	e_DAB_Tuner_Ctrl_DAB_DAB_Status);
void DAB_Tuner_Ctrl_Notify_FM_DAB_LinkingStop(Te_FmtoDAB_Reqstatus e_FmtoDAB_Reqstatus);
//void DAB_Tuner_Ctrl_Notify_SW_HW_Version(Ts_DabTunerMsg_R_GetVersion_Reply st_Vesrion_Number);
void DAB_Tuner_Ctrl_Notify_StationNootAvail_StrategyStatus(Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus);
void DAB_Tuner_Ctrl_Notify_Init_FMDAB_linking(void);
void DAB_Tuner_Ctrl_Notify_AutoScan_PlayStation(Te_RADIO_ReplyStatus e_SelectServiceReplyStatus, Ts_Tuner_Ctrl_CurrentEnsembleInfo st_currentEnsembleData);
#endif
/*=============================================================================
    end of file
=============================================================================*/

