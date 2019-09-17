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
*  Description			: This file contains all the function declarations for request functions
*
*
**********************************************************************************************************/
/** \file hmi_if_request.h 
	 <b> Brief </b>	 API's consists of HMI IF requests functions declarations 
*********************************************************************************************************/
#ifndef HMI_IF_APP_REQUEST_H_
#define HMI_IF_APP_REQUEST_H_


/** \file */

/** \page HMI_IF_APP_REQUEST_top HMI IF Request Package 

\subpage	HMI_IF_REQUEST_Overview 
\n
\subpage	HMI_IF_REQUEST_APIs
\n

*/

/**\page HMI_IF_REQUEST_Overview Overview
	\n
	HMI IF Request package consists of functions which are used to send the request to radio manager application layer.
	\n\n
*/

/** \page HMI_IF_REQUEST_APIs HMI IF Request API's
 <ul>
 	<li>  #Request_ChangeRadioMode : This function requests to change the current band to the selected band. </li>
	<li>  #Request_StartRadio : This function requests for the start up of the radio and it passes the market and variant info to radio manager layer. </li>
	<li>  #Request_ShutDownTuner : This function requests for the shut down of all components of the radio. </li>
	<li>  #Request_PlaySelectStation : This function requests for selection of station from station list based on index. </li>
	<li>  #Request_SeekStation : This function requests for tune to the next/previous station/service based on the direction. </li>
	<li>  #Request_GetQuality_Diag : This function requests for getting the quality of station in diagnostic mode. </li>
	<li>  #Request_GetAFList_Diag : This function requests for getting the AF list in diagnostic mode. </li>
	<li>  #Request_EnableDABFMLinking : This function requests for enabling/disabling DAB to FM linking switch. </li>
	<li>  #Request_GetCurStationInfo_Diag : This function requests for getting current station information in diagnostic mode. </li>
	<li>  #Request_ObtainSTL_Diag : This function request for getting the station list of the current mode in diagnostic mode. </li>
	<li>  #Request_GetRadioStationListData : This function request for station list information to display in the HMI. </li>
	<li>  #Request_GetRadioCommonData : This function request for getting the common data. </li>
	<li>  #Request_GetRadioMode : This function requests to get the current RadioMode by Mode Manager. </li>
	<li>  #Request_RDSFollowing : This function requests to enable/disable RDS following switch from HMI. </li>
	<li>  #Request_MemoryList : This function requests to get the preset list from HMI. </li>
	<li>  #Request_StoreMemoryList : This function requests to store the preset list from HMI. </li>
	<li>  #Request_PlaySelectMemoryList : This function requests to perform preset list recall from HMI. </li>
	<li>  #Request_ManualUpdateSTL : This function requests to get the manual updated station list from HMI. </li>
	<li>  #Request_TuneUpDown : This function requests to tune up/down from HMI. </li>
	<li>  #Request_EnableAnnouncement : This function requests to enable Announcement switch from HMI. </li>
	<li>  #Request_CancelAnnouncement : This function requests to cancel announcement from HMI. </li>
	<li>  #Request_RadioComponentStatus : This function requests to get the radio component status. </li>
	<li>  #Request_RadioSettings : This function requests to get the current Radio settings from the HMI. </li>
	<li>  #Request_AMFMReTune : This function requests to tune to AMFM after abnormal AMFMTuner/DABTuner. </li>
	<li>  #Request_BestPI_Info : This function requests to get the Best PI station information from HMI. </li>
	<li>  #Request_ENG_Mode : This function requests to enable/disable ENG Mode from HMI. </li>
	<li>  #Request_TuneByFrequency : This function requests to tune to frequency from HMI. </li>
	<li>  #Request_GetClockTime : This function requests to get the Clock Time. </li>
	<li>  #Request_CancelManualSTLUpdate : This function requests to cancel the Manual Updation of station List. </li>
	<li>  #Request_EnableAnnouncement_Info : This function requests to enable/disable Announcement Info switch from HMI. </li>
	<li>  #Request_RadioPowerOFF : This function requests the Radio to Power OFF. </li>
	<li>  #Request_RadioPowerON : This function requests the Radio to Power ON. </li>
	<li>  #Request_FactoryReset : This function requests the Radio to perform Factory Reset operation. </li>
	<li>  #Request_RadioFirmwareVersion : This function requests the Radio to get Radio Firmware version. </li>
	<li>  #Request_RadioActivityStatus : This function requests the Radio to get current Activity status. </li>
	<li>  #Request_RadioDABTuneByFrequency : This function requests for tuning to given channel name from HMI. </li>
	<li>  #Request_RadioSTLSearch : This function requests for searching the station list with input character. </li>
	<li>  #Request_playSelectStnFromSearchedSTL : This function requests for selection of station from searched station list based on index. </li>
	<li>  #Request_MultiplexList_Switch : This function requests for Enabling or disabling multiplex switch setting. </li>
	<li>  #Request_EnsembleSelection : This function requests for selection of Ensemble from multiplex list based on index. </li>
	<li>  #Request_playSelectStnFromMultiplexList : This function requests for selection of station from Ensemble list based on index. </li>
</ul> 

*/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "hmi_if_common.h"
#include "IRadio.h"
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
/**	  <b> Brief </b>	 \n This function requests to change the current band to the selected band \n
*						 
*   \param[in]				e_Mode	Band type
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Band change request is delivered to radio manager application layer
*   \return  				None
* 
******************************************************************************************************/
void Request_ChangeRadioMode(MODE_TYPE e_mode);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for the ETAL Intitialization and it passes the firmware information to HMIIF \n
*
*   \param[in]				attr		Radio_EtalHardwareAttr
*   \param[out]				None
*   \pre					HMI IF component is not activated
*   \post					HMI IF component is ready to process requests
*   \return					None
*
******************************************************************************************************/
void Request_EtalHWConfig(Radio_EtalHardwareAttr attr);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for the ETAL Deinitialization to HMIIF \n
*
*   \param[in]				None
*   \param[out]				None
*   \pre					ETAL component is activated
*   \post					ETAL component is not activated
*   \return					None
*
******************************************************************************************************/
void Request_EtalHWDeconfig(void);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for the start up of the radio and it passes the market and variant info to radio manager layer \n
*						 
*   \param[in]				e_market		Market type
*   \param[in]				e_variant		Variant type
*   \param[in]				u8_RadioSourceInfo	Radio Source Info
*   \param[out]				None
*   \pre					HMI IF component is not activated
*   \post					HMI IF component is ready to process requests
*   \return					None
* 
******************************************************************************************************/
void Request_StartRadio(VARIANT_TYPE e_variant, MODEL_TYPE e_market, UINT8 u8_RadioSourceInfo);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for the shut down of all components of the radio \n
*						
*   \param[in]				None
*   \param[out]				None
*   \pre					HMI IF component is ready to process requests
*   \post					HMI IF component is not activated
*   \return					None
* 
******************************************************************************************************/
void Request_ShutDownTuner(void);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for selection of station from station list based on index \n
*						
*   \param[in]				u8_Index	Index number to select the station
*   \param[out]				None
*   \pre					Availability of station list for current band at HMI
*   \post					Request of selecting the station is delivered to radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_PlaySelectStation(UINT8 u8_Index);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for tune to the next/previous station/service based on the direction \n
*						
*   \param[in]				Seek_Direction	Direction Up/Down for seek		
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for seek to selected direction is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_SeekStation(RADIO_DIRECTION Seek_Direction);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for getting the quality of station in diagnostic mode \n
*						
*   \param[in]				None		
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the quality of AMFM station is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_GetQuality_Diag();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for getting the AF list in diagnostic mode  \n
*						 
*   \param[in]				None		
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the AF list is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_GetAFList_Diag();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for enabling/disabling DAB to FM linking switch  \n
*						 
*   \param[in]				e_dabtofmlinkingenable		Enable option is open or close		
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for enabling DAB to FM linking is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_EnableDABFMLinking(Radio_Switch_Request_Settings e_DABFMlinkrequest);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for getting current station information  in diagnostic mode \n
*						
*   \param[in]				None		
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting for current station information is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_GetCurStationInfo_Diag();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for getting the station list of the current mode in diagnostic mode  \n
*						
*   \param[in]				None		
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the station list of the current mode is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_ObtainSTL_Diag();

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for station list information to display in the HMI  \n
*						
*   \param[in]				None		
*   \param[out]				HMIIF_IDataObject
*   \pre					HMI IF is ready to process any request
*   \post					Request for station list information to display at HMI is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_GetRadioStationListData();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for getting the common data  \n
*   \param[in]				None		
*   \param[out]				IDataObject
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the common data is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_GetRadioCommonData();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests to get the current RadioMode by Mode Manager  \n
*						 
*   \param[in]				None		
*   \param[out]				MODE_TYPE	Current mode
*   \pre					HMI IF is ready to process any request
*   \post					Request to get the current RadioMode is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
MODE_TYPE Request_GetRadioMode();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests to enable/disable RDS following switch from HMI  \n
*						
*   \param[in]				Radio_RDS_Following Enum consisting of RDS following parameters	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request enabling or disabling RDS following feature is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_RDSFollowing(Radio_Switch_Request_Settings e_RDS_following_request);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for showing the preset list to HMI  \n
*						
*   \param[in]				None 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for showing the preset list to HMI is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_MemoryList();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for storing the preset list at selected index  \n
*						 
*   \param[in]				u8_Index Index to store the semeted station 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for storing the preset list at selected index by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_StoreMemoryList(UINT8 u8_Index);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for tuning the station present at selected memory index  \n
*						
*   \param[in]				u8_Index Index to play the selected index 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for tuning the station present at selected memory index is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_PlaySelectMemoryList(UINT8 u8_Index);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for updated station list  \n
*						 
*   \param[in]				None 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for updated station list is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_ManualUpdateSTL();
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function request for Tune Up or Down  \n
*						 
*   \param[in]				e_TuneUpDownDirection		Direction up or down 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for Tune Up or Down is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_TuneUpDown(RADIO_DIRECTION e_TuneUpDownDirection);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for Enabling Announcement Switch Feature   \n
*						 
*   \param[in]				e_AnnoEnableTASwitch		Announcement switch status for Traffic announcement 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for Enabling announcement feature is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_EnableAnnouncement(Radio_Switch_Request_Settings e_AnnoEnableTASwitch);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for cancelling the announcement from the HMI  \n
*						 
*   \param[in]				None 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for cancelling the announcement is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_CancelAnnouncement(void);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for getting Radio component status from the HMI  \n
*						
*   \param[in]				None 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Radio component status is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_RadioComponentStatus(void);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for getting Radio settings from the HMI  \n
*						 
*   \param[in]				None 	
*   \param[out]				HMIIF_IDataObject
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Radio settings is received by HMI IF layer and value is returned
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_RadioSettings(void);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for retuning to AMFM component from HMI  \n
*						 
*   \param[in]				None 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for retuning to AMFM component is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_AMFMReTune(void);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for getting Best PI information  \n
*						
*   \param[in]				None 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Best PI information is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_BestPI_Info(void);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests to enable/disable ENG Mode from HMI  \n
*						 
*   \param[in]				Radio_Eng_Mode_Request  Whether ENG mode is ON or OFF 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for ON or OFF of ENG Mode is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_ENG_Mode(Radio_Eng_Mode_Request e_EngModeRequest);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for tuning to given frequency from HMI  \n
*						 
*   \param[in]	            Frequency      Frequency to be tune
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for tuning to given frequency is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_TuneByFrequency(UINT32 Frequency);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests to get the Clock Time  \n
*						 
*   \param[in]	            None
*   \param[out]				None
*   \pre					ENG Mode shall be ON
*   \post					Request to get the Clock Time is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_GetClockTime(void);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests to cancel the Manual Updation of station List  \n
*						 
*   \param[in]	            None
*   \param[out]				None
*   \pre					Manual updation of Station list is going on
*   \post					Request to cancel the Manual Updation of station List is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_CancelManualSTLUpdate(void);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests to enable/disable Announcement Info switch from HMI   \n
*						 
*   \param[in]				e_AnnoEnableInfoSwitch		Announcement switch status for Info announcement 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for Enabling Announcement Info feature is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_EnableAnnouncement_Info(Radio_Switch_Request_Settings e_AnnoEnableInfoSwitch);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests the Radio to Power OFF  \n
*						 
*   \param[in]	            None
*   \param[out]				None
*   \pre					Radio is in Power ON Mode
*   \post					Request to Radio to Power OFF is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_RadioPowerOFF(void);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests to Radio to Power ON  \n
*						 
*   \param[in]	            None
*   \param[out]				None
*   \pre					Radio is in Power OFF Mode
*   \post					Request to Radio to Power OFF is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_RadioPowerON(void);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests the Radio to perform Factory Reset operation  \n
*						 
*   \param[in]	            None
*   \param[out]				None
*   \pre					Radio is in Active Mode
*   \post					Request to Radio to perform Factory Reset operation is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_FactoryReset(void);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests the Radio to get Radio Firmware version  \n
*						 
*   \param[in]	            None
*   \param[out]				None
*   \pre					Radio is in Active Mode
*   \post					Request for getting the Radio Firmware version to display on HMI is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_RadioFirmwareVersion(void);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests the Radio to get current Activity status  \n
*						 
*   \param[in]	            None
*   \param[out]				None
*   \pre					Radio is in Active Mode
*   \post					Request for getting current Activity status to HMI is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_RadioActivityStatus(void);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for tuning to given channel name from HMI  \n
*						 
*   \param[in]	            Channel Name      Channel Name for Tuning to DAB band
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for tuning to given frequency for DAB is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_RadioDABTuneByFrequency(UINT8* au8_ChannelName);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for searching the station list with input character  \n
*						 
*   \param[in]				None		
*   \param[out]				HMIIF_IDataObject
*   \pre					HMI IF is ready to process any request
*   \post					Request for searching the station list with input character is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_RadioSTLSearch(UINT8 RequestedChar);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for selection of station from searched station list based on index \n
*						 
*   \param[in]				nIndex	Index number to select the station
*   \param[out]				None
*   \pre					Availability of station list for current band at HMI
*   \post					Request of selecting the station from searched list is delivered to radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_playSelectStnFromSearchedSTL(UINT8 nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for Enabling or disabling multiplex switch setting   \n
*						
*   \param[in]				e_MultiplexInfoSwitch		Announcement switch status for Info announcement 	
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for Enabling or disabling multiplex switch setting is received by radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_MultiplexList_Switch(Radio_Switch_Request_Settings e_MultiplexInfoSwitch);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for selection of Ensemble from multiplex list based on index \n
*						
*   \param[in]				nIndex	Index number of ensemble to view the corresponding services
*   \param[out]				None
*   \pre					Availability of ensembles in Multiplex list on HMI
*   \post					Request of selecting the Ensemble from multiplex list is delivered to radio manager application layer
*   \return					None
* 
******************************************************************************************************/
HMIIF_IDataObject Request_EnsembleSelection(UINT8 nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function requests for selection of station from Ensemble list based on index \n
*						 
*   \param[in]				nIndex	Index number to select the station from ensemble list
*   \param[out]				None
*   \pre					Availability of station in selected ensemble on HMI
*   \post					Request of selecting the station from Ensemble list is delivered to radio manager application layer
*   \return					None
* 
******************************************************************************************************/
void Request_playSelectStnFromMultiplexList(UINT8 nIndex);
#endif
/*=============================================================================
    end of file
=============================================================================*/