/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/* Copyright (c) 2016, Jasmin Infotech Private Limited.
*  All rights reserved. Reproduction in whole or part is prohibited
*  without the written permission of the copyright owner.
*
*  Project              : ST_Radio_Middleware
*  Organization			: Jasmin Infotech Pvt. Ltd.
*  Module				: HMI IF
*  Description			: This file contains all the function declarations for notification functions
*
*
**********************************************************************************************************/
/** \file hmi_if_notify.h 
	 <b> Brief </b>	 API's consists of notifications functions declarations 
*********************************************************************************************************/
#ifndef HMI_IF_APP_NOTIFY_H_
#define HMI_IF_APP_NOTIFY_H_


/** \file */

/** \page HMI_IF_APP_NOTIFY_top HMI IF Notify Package 

\subpage	HMI_IF_NOTIFY_Overview 
\n
\subpage	HMI_IF_NOTIFY_APIs
\n

*/

/**\page HMI_IF_NOTIFY_Overview Overview
	\n
	HMI IF Notify package consists of functions which are used to send the notifications to the HMI.
	\n\n
*/

/** \page HMI_IF_NOTIFY_APIs HMI IF Notify API's
 <ul>
	<li>  #Notify_UpdateCurStationInfo_Display : This function notifies the current station information of the station. </li>
	<li>  #AMFM_Notify_UpdateCurStationInfo_Diag : This function notifies the current station information of AMFM band in diagnostic mode. </li>
	<li>  #AMFM_Notify_UpdateFMSTL_Diag : This function notifies the updated FM station list to the HMI in diagnostic mode. </li>
	<li>  #AMFM_Notify_UpdateAMSTL_Diag : This function notifies the updated AM station list to the HMI in diagnostic mode. </li>
	<li>  #AMFM_Notify_Quality_Diag : This function notifies the quality parameters for AMFM band to the HMI in diagnostic mode. </li>
	<li>  #Notify_UpdateCurRadioMode : This function notifies the current band to the HMI. </li>
	<li>  #Notify_UpdateStartRadioStatus : This function notifies the start up status to the HMI. </li>
	<li>  #Notify_UpdateShutdownTunerStatus : This function notifies the shut down status to the HMI. </li>
	<li>  #Notify_UpdateTunerMute : This function notifies the mute status to the HMI. </li>
	<li>  #DAB_Notify_UpdateStationInfo_Diag : This function notifies the updated DAB station information to the HMI in diagnostic mode. </li>
	<li>  #DAB_Notify_UpdateNormalStl_Diag : This function notifies the updated DAB Normal station list to the HMI in diagnostic mode. </li>
	<li>  #DAB_Notify_UpdateMultiplexStl_Diag : This function notifies the updated DAB Multiplex station list to the HMI in diagnostic mode. </li>
	<li>  #Radio_Response_PlaySelectStation : This function gives the reply status of select station request. </li>
	<li>  #DAB_Notify_Quality_Diag :This function notifies the quality parameters of DAB band to the HMI in diagnostic mode. </li>
	<li>  #Radio_Response_SeekStation : This function gives the reply status and band name for the radio seek request. </li>
	<li>  #AMFM_Notify_AFSwitchStatus : This function notifies the alternate frequency switch status to the HMI. </li>
	<li>  #Notify_StationList : This function notifies the current band and station list to the HMI. </li>
	<li>  #Notify_UpdateScanStatus : This function gives the reply status of the AF switch. </li>
	<li>  #Radio_Response_EnableDABFMLinking : This function gives the reply status of DAB-FM linking switch settings. </li>
	<li>  #AMFM_Notify_AFList_Diag : This function gives the PI list and number of PI in diagnostic mode. </li>
	<li>  #DABFM_Notify_DABFMLinkingStatus : This function gives the DAB FM linking status. </li>
	<li>  #Radio_Response_StoreMemoryList : This function gives the preset list store response to HMI. </li>
	<li>  #Radio_Response_PlaySelectMemoryList : This function gives the play preset list response to HMI. </li>
	<li>  #Radio_Response_RDSFollowing : This function gives the reply status of RDS following switch settings. </li>
	<li>  #Radio_Response_ManualUpdateSTL : This function gives the manual update station list response to HMI. </li>
	<li>  #Notify_UpdateMemoryList : This function gives the updated preset list to HMI. </li>
	<li>  #Radio_Response_TuneUpDown : This function gives the Tune up down response to HMI. </li>
	<li>  #Radio_Response_EnableTAAnnouncement : This function gives the announcement switch response to HMI. </li>
	<li>  #Radio_Response_CancelAnnouncement : This function gives the cancelling the announcement response to HMI. </li>
	<li>  #Radio_Notify_Announcement : This function gives the announcement notification to HMI. </li>
	<li>  #Radio_Notify_Components_Status : This function gives the component status to HMI. </li>
	<li>  #Radio_Notify_Settings : This function gives the settings notifications to HMI. </li>
	<li>  #Notify_Activity_State : This function gives the activity status to HMI. </li>
	<li>  #Notify_BestPIStation : This function gives the best PI station information to HMI. </li>
	<li>  #DAB_Notify_AFList_Diag : This function gives the updated DAB AF list notification to HMI in diagnostic mode. </li>
	<li>  #Radio_Response_GetClockTime : This function gives the clock time reply status response to HMI. </li>
	<li>  #Radio_Notify_ClockTime : This function Notifies the clock time information to HMI. </li>
	<li>  #Radio_Response_CancelManualSTLUpdate : This function gives the status of cancel request for Manual station list update. </li>
	<li>  #Radio_Response_EnableInfoAnnouncement : This function gives the Info announcement switch response to HMI. </li>
	<li>  #Radio_Response_BG_AFStationInfo : This function notifies the background station information of the station with SRC as Active band. </li>
	<li>  #Radio_Notify_AudioSwitch :This function intimates the Internal Audio change to HMI, so that Mode Manager can call Get Mode function. </li>
	<li>  #Radio_Response_PowerOnStatus : This function gives the status of Radio power on request. </li>
	<li>  #Radio_Response_PowerOffStatus : This function gives the status of Radio power off request. </li>
	<li>  #Radio_Response_FactoryReset : This function gives the status of Factory reset request. </li>
	<li>  #Radio_Notify_FirmwareVersion :This function notifies the Firmware Version information of the Tuner. </li>
	<li>  #Notify_DAB_Dataservice_To_Display : This function notifies the DAB Dataservice to display on HMI. </li>
	<li>  #Radio_Response_EnableMultiplexInfoSwitch : This function gives the Multiplex station list switch status to the HMI. </li>
</ul> 

*/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "radio_mngr_app_types.h"
//#include "itron.h"
#include "Tuner_core_sys_main.h"
#include "hmi_if_common.h"
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
/**	 <b> Brief </b>	 	\n This function notifies the current station information of the station . \n
*						 
*   \param[in]				eBand			Band type
*   \param[in]				e_AudBand		Audio Band
*   \param[in]				nFreq			Updated frequency
*   \param[in]				*stName 		Pointer to Station name
*	\param[in]				*radioText		Radio text/ DLS
*	\param[in]				CharSet			Char-set value
*	\param[in]				ChannelName		Channel name
*	\param[in]				Ensemble_Name	Ensemble_Name
*	\param[in]				DAB_Current_Playing_Service	Currently playing service in 
*	\param[in]				DAB_TotalNoofServiceInEnsemble	Total no. of services present in DAB ensemble
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					Updated current station information is notified to the HMI
*   \return					None
* 
******************************************************************************************************/
void Notify_UpdateCurStationInfo_Display(Te_Radio_Mngr_App_Band eBand, Tu32 nFreq, Tu8 *stName, Tu8* radioText, Tu8 CharSet, Tu8* ChannelName,Tu8* Prgm_Name, Tu8* Ensemble_Name, Tu8 DAB_Current_Playing_Service, Tu8 DAB_TotalNoofServiceInEnsemble,Tu8 TA,Tu8 TP,Tu16 PI);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the current station information of AMFM band in diagnostic mode. \n
*							
*   \param[in]				eBand			Band
*   \param[in]				u32_Freq		Frequency
*   \param[in]				u32_Quality		Quality of the station
*   \param[in]              *StName 		Station name
*	\param[in]				u16_PI			PI value for station
*	\param[in]				u8_TA			Traffic Announcement
*	\param[in]				u8_TP			Traffic Programme
*	\param[in]				u8_Char_Set		Char-set value
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					Updated station information related to AMFM current station is notified to the HMI 
*   \return					None
* 
******************************************************************************************************/
void AMFM_Notify_UpdateCurStationInfo_Diag(Te_Radio_Mngr_App_Band eBand, Tu32 u32_Freq, Tu32 u32_Quality, Tu8 *StName, Tu16 u16_PI, Tu8 u8_TA, Tu8 u8_TP, Tu8 u8_Char_Set);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the updated FM station list to the HMI in diagnostic mode. \n
*							
*   \param[in]				*st_FM_StL	Pointer to FM station list
*   \param[in]				u8_numStl	Number of stations present in the FM station list
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					FM station list is passed to the HMI
*   \return					None
* 
******************************************************************************************************/
void AMFM_Notify_UpdateFMSTL_Diag(Ts_Radio_Mngr_App_FM_SL *st_FM_StL, Tu8 u8_numStl);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the updated AM station list to the HMI in diagnostic mode. \n
*							
*   \param[in]				*st_AM_StL	Pointer to AM station list
*   \param[in]				u8_numStl	Number of stations present in the AM station list
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					AM station list is passed to the HMI
*   \return					None 
* 
******************************************************************************************************/
void AMFM_Notify_UpdateAMSTL_Diag(Ts_Radio_Mngr_App_AM_SL *st_AM_StL, Tu8 u8_numStl);
/*****************************************************************************************************/
/**	<b> Brief </b>	 	\n This function notifies the quality parameters for AMFM band to the HMI in diagnostic mode. \n
*						 
*   \param[in]				eBand				Band
*   \param[in]				s32_BBFieldStrength	Baseband Field strength
*   \param[in]				u32_UltrasonicNoise	Ultra Sonic Noise value		
*   \param[in]				u32_Multipath		Wideband Amplitude modulation value
*   \param[in]				u32_FrequencyOffset	FrequencyOffset value 
*   \param[in]				u8_IF_BW			IF bandwidth value
*   \param[in]				u32_ModulationDetector	Mod value
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					Quality parameters for AMFM band is notified to the HMI
*   \return					None
* 
******************************************************************************************************/
void AMFM_Notify_Quality_Diag(Te_Radio_Mngr_App_Band eBand, Ts32 s32_RFFieldStrength, Ts32 s32_BBFieldStrength, Tu32 u32_UltrasonicNoise, Tu32 u32_SNR, Tu32 u32_AdjacentChannel, Tu32 u32_CoChannel, Tu32 u32_StereoMonoReception, Tu32 u32_Multipath, Tu32 u32_FrequencyOffset, Tu32 u32_ModulationDetector);
/*****************************************************************************************************/
/**	 <b> Brief </b>	 	\n This function notifies the current band to the HMI. \n
*						  
*   \param[in]				replyStatus		Reply status for change band request
*							on success HMI IF sends #RADIO_SELECTBAND_SUCCESS, and on failure #RADIO_SELECTBAND_FAILURE to HMI
*	\param[in]				e_activeBand	Active band
*   \param[in]				e_Band		
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					Updated current band notified to the HMI
*   \return					None
* 
******************************************************************************************************/
void Notify_UpdateCurRadioMode(Te_RADIO_ReplyStatus replyStatus, Te_Radio_Mngr_App_Band e_activeBand);
/*****************************************************************************************************/
/**	 <b> Brief </b>	 		\n This function notifies the start up status to the HMI. \n
*							
*   \param[in]				replyStatus		Reply status for start up tuner request, 
*							on success HMI IF sends #RADIO_STARTUP_SUCCESS, and on failure #RADIO_STARTUP_FAILURE to HMI 
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					Reply status of the start up request is notified to HMI
*   \return					None
* 
******************************************************************************************************/
void Notify_UpdateStartRadioStatus(Te_RADIO_ReplyStatus replyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the shut down status to the HMI. \n
*							
*   \param[in]				replyStatus		Reply status for shut down tuner request,
*							on success HMI IF sends #RADIO_SHUTDOWN_SUCCESS, and on failure #RADIO_SHUTDOWN_FAILURE to HMI	
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Reply status of the shut down request is notified to HMI
*   \return					None
* 
******************************************************************************************************/
void Notify_UpdateShutdownTunerStatus(Te_RADIO_ReplyStatus replyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>		 	\n This function notifies the mute status to the HMI. \n
*							
*   \param[in]				eBand			Band
*   \param[in]				muteStatus		Mute status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Updated mute status is notified to the HMI
*   \return					None
* 
******************************************************************************************************/
void Notify_UpdateTunerMute(Te_Radio_Mngr_App_Band eBand, BOOL muteStatus);
/*****************************************************************************************************/
/**	 <b> Brief </b>	 		\n This function notifies the updated DAB station information to the HMI in diagnostic mode. \n
*							
*   \param[in]				nFreq			Frequency
*   \param[in]				EId				Ensemble id	
*   \param[in]				SId				Service Id
*   \param[in]				SCId			Service component Id
*   \param[in]				ServiceName		Pointer to service name
*	\param[in]				CharSet			Charset value
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Updated station information is passed to HMI
*   \return					None
* 
******************************************************************************************************/
void DAB_Notify_UpdateStationInfo_Diag(Tu32 nFreq,Tu16 EId,Tu32 SId,Tu16 SCId,Tu8 *ServiceName, Tu8 CharSet);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the updated DAB Normal station list to the HMI in diagnostic mode. \n
*							
*   \param[in]				*st_DAB_StL		Pointer to DAB Normal station list
*   \param[in]				u8_numStl		Number of stations present in the station list
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					DAB Normal station list is passed to the HMI
*   \return					None
* 
******************************************************************************************************/
void DAB_Notify_UpdateNormalStl_Diag(Ts_Radio_Mngr_App_DAB_SL *st_DAB_StL, Tu8 u8_numStl);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the updated DAB Multiplex station list to the HMI in diagnostic mode. \n
*							
*   \param[in]				*st_DAB_MultiplexStl		Pointer to DAB Multiplex station list
*   \param[in]				u8_NoOfEnsembleList		    Number of ensembles present in the Multiplex station list
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					DAB Multiplex station list is passed to the HMI
*   \return					None
* 
******************************************************************************************************/
void DAB_Notify_UpdateMultiplexStl_Diag(Ts_Radio_Mngr_App_DAB_MultiplexStationList *st_DAB_MultiplexStl, Tu8 u8_NoOfEnsembleList);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the reply status of select station request. \n
*							
*   \param[in]				eBand			Band
*   \param[in]				replyStatus		Reply status for play select station request
*							on success HMI IF sends #RADIO_STATIONLISTSELECT_REQ_SUCCESS, and on failure #RADIO_STATIONLISTSELECT_REQ_FAILURE to HMI
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Select station reply status is sent to HMI
*   \return					None
* 
******************************************************************************************************/
void Radio_Response_PlaySelectStation(Te_RADIO_ReplyStatus replyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the quality parameters of DAB band to the HMI in diagnostic mode. \n
*                          
*   \param[in]				ber_significant		Bit error rate Significant value
*   \param[in]				ber_exponent		Bit error rate Exponent value
*   \param[in]				snr_level			Signal to noise ratio
*   \param[in]				rssi				Received signal strength indication
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Quality parameters for DAB band is notified to the HMI
*   \return					None
* 
******************************************************************************************************/
void DAB_Notify_Quality_Diag(Ts32 s32_RFFieldStrength, Ts32 s32_BBFieldStrength, Tu32 u32_FicBitErrorRatio, Tbool bool_isValidMscBitErrorRatio, Tu32 u32_MscBitErrorRatio);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the reply status and band name for the radio seek request. \n
*							
*   \param[in]				e_SeekReplyStatus		Reply status for seek up down
*							on success HMI IF sends #RADIO_SEEK_REQ_SUCCESS, and on failure #RADIO_SEEK_REQ_FAILURE to HMI	
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Seek up down reply status is sent to HMI for DAB band
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_SeekStation(Te_RADIO_ReplyStatus e_SeekReplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the alternate frequency switch status to the HMI. \n
*							
*   \param[in]				e_AF_Notify_Status		Notification of AF switch
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Alternate frequency switch status is sent to HMI.
*   \return					None
*	
******************************************************************************************************/
void AMFM_Notify_AFSwitchStatus(Te_Radio_Mngr_App_AF_Status e_AF_Notify_Status);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the current band and station list to the HMI \n
*							
*   \param[in]				e_Band					Band
*   \param[in]				*StList					Pointer to station list
*	\param[in]				numStl					Number of stations in station list
*	\param[in]				Match_Stn_Index		Index of matched station
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Notification of station list is sent to HMI along with band
*   \return					None
*	
******************************************************************************************************/
void Notify_StationList(Te_Radio_Mngr_App_Band e_Band, void *StList, Tu8 numStl, Tu8 Match_Stn_Index);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the reply status of the AF switch. \n
*							             
*   \param[in]				replyStatus		Reply status of scan 
*							on starting scan HMI IF sends #RADIO_SCAN_STARTED, on completion HMI IF sends #RADIO_SCAN_COMPLETE, and if it is not completed then #RADIO_SCAN_INPROGRESS to HMI	
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Scan reply status is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Notify_UpdateScanStatus(Te_Radio_Mngr_App_ScanStatus replyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the reply status of DAB-FM linking switch settings. \n
*							             
*   \param[in]				replyStatus		Reply status of DABFM linking 
*							On succes it retuens #RADIO_DAB_FM_LINKING_ENABLE_SUCCESS and on failure it returns #RADIO_DAB_FM_LINKING_DISABLE_SUCCESS
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					DAB-FM linking status reply status is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_EnableDABFMLinking(Te_RADIO_ReplyStatus replyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the PI list and number of PI in diagnostic mode. \n
*							             
*   \param[in]				Num_AF		Number of AF 
*   \param[in]				AFList		AF List
*   \param[in]				Quality		Quality for AF
*   \param[in]				PIList		PI List
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					AF, number of AF stations,PI list and quality is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void AMFM_Notify_AFList_Diag(Tu8 Num_AF, Tu32 *AFList, Tu32 *Quality, Tu16 *PIList);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the DAB FM linking status. \n
*							             
*   \param[in]				e_linking_Status		Linking status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					DAB FM linking status is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void DABFM_Notify_DABFMLinkingStatus(Te_RADIO_DABFM_LinkingStatus e_linking_Status);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of storing the preset list request \n
*							             
*   \param[in]				e_presetStoreReplyStatus		Preset store reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of storing the preset list request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_StoreMemoryList(Te_RADIO_ReplyStatus e_presetStoreReplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Play select preset list \n
*							             
*   \param[in]				e_presetRecallReplyStatus	Preset recall reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Play select preset list is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_PlaySelectMemoryList(Te_RADIO_ReplyStatus e_presetRecallReplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the reply status of RDS following switch settings \n
*							             
*   \param[in]				e_AFSwitchreplyStatus	AF switch reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of RDS following request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_RDSFollowing(Te_RADIO_ReplyStatus e_RDSSettingsReplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of update station list request \n
*							             
*   \param[in]				e_StationListreplyStatus	Station list reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of update station list request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_ManualUpdateSTL(Te_RADIO_ReplyStatus e_StationListreplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the preset list to the HMI \n
*							             
*   \param[in]				MemoryStList		pointer to preset list
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Memory list is notified to the HMI
*   \return					None
*	
******************************************************************************************************/
void Notify_UpdateMemoryList(Ts_Radio_Mngr_App_Preset_Mixed_List *MemoryStList);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Tune Up Down request \n
*							             
*   \param[in]				e_TuneUpDownreplyStatus	Tune up/down reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Tune Up Down is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_TuneUpDown(Te_RADIO_ReplyStatus e_TuneUpDownreplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Enable Announcement request \n
*							             
*   \param[in]				e_Anno_EnableReplystatus	Announcement switch reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Enable Announcement request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_EnableTAAnnouncement(Te_RADIO_ReplyStatus e_Anno_EnableReplystatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Announcement cancel request \n
*							             
*   \param[in]				e_Anno_CancelReplystatus	Announcement cancel reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Announcement cancel request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_CancelAnnouncement(Te_RADIO_ReplyStatus e_Anno_CancelReplystatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Announcement cancel request \n
*							             
*   \param[in]				e_Anno_Status		Announcement status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Announcement cancel request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Notify_Announcement(Te_Radio_Mngr_App_Anno_Status e_Anno_Status);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of both tuners \n
*							             
*   \param[in]				eBand			Band
*   \param[in]				e_AMFMTunerStatus	AMFMTuner status normal/Abnormal
*   \param[in]				e_DABTunerStatus	DABTuner status normal/Abnormal
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of AMFM and DAB tuners is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Notify_Components_Status(Te_Radio_Mngr_App_Band eBand, Te_RADIO_AMFMTuner_Status e_AMFMTunerStatus, Te_RADIO_Comp_Status e_DABTunerStatus, Te_Radio_Mngr_App_DAB_UpNotification_Status	e_DAB_UpNot_Status);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of DABFM, Announcement status and RDS Follow up \n
*							             
*   \param[in]				e_DABFM_Status				DAB FM switch status
*   \param[in]				e_TA_Anno_Status			TA Announcement switch status
*   \param[in]				e_AF_Status					AF switch status
*   \param[in]				e_Info_Anno_Status			Info Announcement switch status
*   \param[in]				e_Multiplex_Switch_Status	Multiplex switch status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of settings(DAB-FM, Announcement(TA & Info), AF Switch, Ensemble Multiplex list) is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Notify_Settings(Te_Radio_Mngr_App_DABFMLinking_Switch e_DABFM_Status, Te_Radio_Mngr_App_EnableTAAnno_Switch e_TA_Anno_Status, Te_Radio_Mngr_App_RDSSettings e_AF_Status, Te_Radio_Mngr_App_EnableInfoAnno_Switch e_Info_Anno_Status, Te_Radio_Mngr_App_Multiplex_Switch e_Multiplex_Switch_Status);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Radio softeare \n
*							             
*   \param[in]				eBand				Band
*   \param[in]				e_Activity_State	Activity status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Radio software is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Notify_Activity_State(Te_Radio_Mngr_App_Band eBand, Te_Radio_Mngr_App_Activity_Status e_Activity_State);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the best PI station information \n
*							             
*   \param[in]				u32_Freq	Frequency
*   \param[in]				u16_PI		PI of station
*   \param[in]              u32_Quality	Quality
*   \param[out]				None	
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Best PI station information is sent to HMI
*   \return					None
*
******************************************************************************************************/
void Notify_BestPIStation(Tu32 u32_Freq, Tu16 u16_PI, Tu32 u32_Quality);

/*****************************************************************************************************/
/**	<b> Brief </b>	 	\n This function notifies the DAB AF list to HMI in diagnostic mode. \n
*						               
*   \param[in]				DAB_AltFreq			DAB alternate frequencies
*   \param[in]				DAB_AltEnsemble		DAB alternate Ensemble
*   \param[in]				DAB_HardlinkSid		DAB Hard links
*   \param[out]				None
*   \pre					HMI IF is ready to process any notification to HMI
*   \post					DAB AF list parameters for DAB band is notified to the HMI
*   \return					None
* 
******************************************************************************************************/
void DAB_Notify_AFList_Diag(Tu32 *DAB_AltFreq, Tu16* DAB_AltEnsemble, Tu32* DAB_HardlinkSid, Tu8 NumAltFreq, Tu8 NumAltEnsemble, Tu8 NumAltHardlinkSid);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Getting Clock Time request \n
*							             
*   \param[in]				e_GetCT_InfoReplyStatus		Get Clock Time Reply Status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Getting Clock Time request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_GetClockTime(Te_RADIO_ReplyStatus e_CTInfoReplyStatus);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function Notifies the clock time information to HMI \n
*							             
*   \param[in]				u8_hour				Hour
*   \param[in]				u8_Min				Minutes
*   \param[in]				u8_day				Day
*   \param[in]				u8_Month			Month
*   \param[in]				u16_Year			Year
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Notification of Clock Time information is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Notify_ClockTime(Tu8 u8_hour, Tu8 u8_Min, Tu8 u8_day, Tu8 u8_Month, Tu16 u16_Year);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of cancel request for Manual station list update \n
*							             
*   \param[in]				e_CancelManualSTLReplyStatus		Manual Update Cancel Reply Status
*   \param[out]				None
*   \pre					HMI IF is ready to process any notification/response to HMI
*   \post					Status of cancelling of updation of Manual station list request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_CancelManualSTLUpdate(Te_RADIO_ReplyStatus e_CancelManualSTLReplyStatus);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Enable Info Announcement request \n
*							             
*   \param[in]				e_Info_Switch_Status	Info Announcement switch reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Enable Info Announcement request is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_EnableInfoAnnouncement(Te_RADIO_ReplyStatus e_Info_Switch_Status);
/*****************************************************************************************************/
/**	 <b> Brief </b>	 	\n This function notifies the background station information of the station with SRC as Active band. \n
*						              
*   \param[in]				eBand			Band type
*   \param[in]				e_AudBand		Audio Band
*   \param[in]				nFreq			Updated frequency
*   \param[in]				*stName 		Pointer to Station name
*	\param[in]				*radioText		Radio text/ DLS
*	\param[in]				CharSet			Char-set value
*	\param[in]				ChannelName		Channel name
*	\param[in]				Ensemble_Name	Ensemble_Name
*	\param[in]				DAB_Current_Playing_Service	Currently playing service in 
*	\param[in]				DAB_TotalNoofServiceInEnsemble	Total no. of services present in DAB ensemble
*   \param[out]				None
*   \pre					HMI IF is ready to process notification to HMI
*   \post					Updated current station information is notified to the HMI
*   \return					None
*
******************************************************************************************************/
void Radio_Response_BG_AFStationInfo(Te_Radio_Mngr_App_Band e_SRC, Te_Radio_Mngr_App_Band e_AudBand, Tu32 nFreq, Tu8 *stName, Tu8 CharSet, Tu8* radioText, Tu8* ChannelName, Tu8* Ensemble_Name, Tu8 DAB_Current_Playing_Service, Tu8 DAB_TotalNoofServiceInEnsemble);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function intimates the Internal Audio change to HMI, so that Mode Manager can call Get Mode function\n
*							             
*   \param[in]				e_ReqAudioChangeBand	Requested band for changing the audio
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Request for Audio switching has been sent to Mode Manager
*   \return					None
*
******************************************************************************************************/
void Radio_Notify_AudioSwitch(Te_Radio_Mngr_App_Band e_ReqAudioChangeBand);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Radio power on request \n
*							             
*   \param[in]				e_PowerOnReplyStatus	Radio power on reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of power on is sent to HMI
*   \return					None
*
******************************************************************************************************/
void Radio_Response_PowerOnStatus(Te_RADIO_ReplyStatus e_PowerOnReplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Radio power off request \n
*							             
*   \param[in]				e_PowerOnReplyStatus	Radio power off reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of power off is sent to HMI
*   \return					None
*
******************************************************************************************************/
void Radio_Response_PowerOffStatus(Te_RADIO_ReplyStatus e_PowerOffReplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the status of Factory reset request \n
*							             
*   \param[in]				e_FactoryResetReplyStatus	Factory reset reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Factory reset is sent to HMI
*   \return					None
*
******************************************************************************************************/
void Radio_Response_FactoryReset(Te_RADIO_ReplyStatus e_FactoryResetReplyStatus);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the Firmware Version information of the Tuner \n
*							             
*   \param[in]				e_FactoryResetReplyStatus	Factory reset reply status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Status of Factory reset is sent to HMI
*   \return					None
*
******************************************************************************************************/
void Radio_Notify_FirmwareVersion(Tu8* DABTunerSWVersion, Tu8* DABTunerHWVersion, Tu8* AMFMTunerVersion);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function notifies the DAB Dataservice to display on HMI \n
*							             
*   \param[in]				Dataserviceraw	  It contains Dataservice information
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Dataservice information has to be displayed on HMI
*   \return					None
*
******************************************************************************************************/
void Notify_DAB_Dataservice_To_Display(Ts_Radio_Mngr_App_DataServiceRaw* Dataserviceraw);
/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the Multiplex station list switch status to the HMI. \n
*							
*   \param[in]				e_Multiplex_Switch_Status		Notification of Multiplex station list switch
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					Multiplex station list switch status is sent to HMI
*   \return					None
*	
******************************************************************************************************/
void Radio_Response_EnableMultiplexInfoSwitch(Te_RADIO_ReplyStatus e_Multiplex_Switch_Status);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the ETAL Initialization status to the HMI. \n
*
*   \param[in]				e_etal_startup_reply_status		ETAL Initialization Status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					ETAL Initialization Status is sent to HMI
*   \return					None
*
******************************************************************************************************/
void Radio_Response_ETALHwConfig(Te_RADIO_ReplyStatus e_etal_startup_reply_status);

/*****************************************************************************************************/
/**	<b> Brief </b>	 		\n This function gives the ETAL Deinitialization status to the HMI. \n
*
*   \param[in]				e_etal_deinit_reply_status		ETAL Deinitialization Status
*   \param[out]				None
*   \pre					HMI IF is ready to process the notification to HMI
*   \post					ETAL DeInitialization Status is sent to HMI
*   \return					None
*
******************************************************************************************************/
void Radio_Response_ETALHwDeConfig(Te_RADIO_ReplyStatus e_etal_deinit_reply_status);

#ifdef HMI_TCP_PROTOCOL
void Info_Extraction(unsigned int source_index, unsigned int dest_index, char *source, char *destination, Tu8 length);
#endif

#endif

/*=============================================================================
    end of file
=============================================================================*/