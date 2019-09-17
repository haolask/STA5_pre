/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/* Copyright (c) 2016, Jasmin Infotech Private Limited.
*  All rights reserved. Reproduction in whole or part is prohibited
*  without the written permission of the copyright owner.
*
*  Project              : hmi_if_app.h
*  Organization			: Jasmin Infotech Pvt. Ltd.
*  Module				: HMI IF
*  Description			: This file contains enumeration and structure definitions for HMI IF App component
*
*
**********************************************************************************************************/
/** \file hmi_if_app.h 
	 <b> Brief </b>	 This file contains structure definitions for HMI IF component, along with the function prototypes 
*********************************************************************************************************/
#ifndef HMI_IF_APP_H_
#define HMI_IF_APP_H_

#include "hmi_if_utf8_conversion.h"


/** \file */

/** \page HMI_IF_APP_top HMI IF Application Package

\subpage	HMI_IF_APP_Overview 
\n
\subpage	HMI_IF_APP_APIs
\n
*/

/**\page HMI_IF_APP_Overview Overview
	\n
	HMI IF Application package provides the API for the listeners for retrieving the data.
	\n\n
*/

/** \page HMI_IF_APP_APIs HMI IF Application API's
<b> All Set APIs are currently not used. </b>
 <ul>
 	<li>  #_Radio_GetModuleInfo : This function provides the Radio module information to the listeners. </li>
	<li>  #_Radio_AddListener : This function registers the callback function provided by the listener. </li>
	<li>  #_Radio_ModuleInit : This function is used to initialize the Radio module. </li>
	<li>  #_Radio_ModuleRun : This function is used to run the Radio default base module. </li>
	<li>  #_Radio_ModuleStop : This function is used to stop the default base module. </li>	
	<li>  #_Radio_Notify : This is the callback function of radio module which is provided to listeners </li>
	<li>  #_Radio_GetInfo : This function is used by listeners to get the Radio module informations. </li>
	<li>  #_Radio_SetInfo : This function is used to set the Radio module information. </li>
	<li>  #_RadioCommonData_GetId : This function is used by listener to get Object ID of radio common data. </li>
	<li>  #_RadioCommonData_Get : This function is used by listeners to get the values of subId in Common data Object ID. </li>
	<li>  #_RadioStationListData_GetId : This function is used by listeners to get the Object ID of station list Data. </li>
	<li>  #_RadioStationListData_GetAMInfo : This function is used to get the station list information of AM in station list Object ID. </li>
	<li>  #_RadioStationListData_GetFMInfo : This function is used to get the station list information of FM in station list Object ID. </li>
	<li>  #_RadioStationListData_GetDABInfo : This function is used to get the station list information of DAB in station list Object ID. </li>
	<li>  #_RadioStationList_GetAMAt : This function is used to get the AM station list information from the list for the particular index. </li>
	<li>  #_RadioStationList_GetFMAt : This function is used to get the FM station list information from the list for the particular index. </li>
	<li>  #_RadioStationList_GetDABAt :This function is used to get the DAB station list information from the list for the particular index. </li>
	<li>  #_RadioStationList_GetAMCount : This function is used to get the AM station list count. </li>
	<li>  #_RadioStationList_GetFMCount : This function is used to get the FM station list count. </li>
	<li>  #_RadioStationList_GetDABCount : This function is used to get the DAB station list count. </li>
	<li>  #_RadioAMFMStnInfoData_GetId : This function is used by listener to get Object ID of AMFM station information data. </li>
	<li>  #_RadioAMFMStnInfoData_Get : This function is used by listeners to get the values of subId in FM StationInfo data Object ID. </li>
	<li>  #_RadioAMFMStnInfoData_Set : This function is used to set the values of subId in FM StationInfo data Object ID. </li>
	<li>  #_RadioDABStnInfoData_GetId : This function is used by listener to get Object ID of DAB station information data. </li>
	<li>  #_RadioDABStnInfoData_Get : This function is used by listeners to get the values of subId in FM StationInfo data Object ID. </li>
	<li>  #_RadioAMFMQualityData_GetId : This function is used by listener to get Object ID of AMFM quality data. </li>
	<li>  #_RadioAMFMQualityData_Get : This function is used by listeners to get the values of subId in AMFM Quality data Object ID. </li>
	<li>  #_RadioDABQualityData_GetId : This function is used by listener to get Object ID of DAB quality data. </li>
	<li>  #_RadioDABQualityData_Get : This function is used by listeners to get the values of subId in DAB Quality data Object ID. </li>
	<li>  #_RadioMuteStatus_GetId : This function is used by listener to get Object ID of radio mute status. </li>
	<li>  #_RadioMuteStatus_Get : This function is used by listeners to get the values of subId in radio mute status. </li>
	<li>  # Radio_Default_IDataObject_GetId : This function is used to get the ID of the Radio default Data object. </li>
	<li>  # Radio_Default_IDataObject_Get : This function is used to get the values of Radio default Data object ID. </li>
	<li>  # Radio_Default_IObjectList_GetAt : This function is used to get the Radio default Object List ID. </li>
	<li>  # Radio_Default_IObjectList_GetCount : This function is used to get the count in Radio default Object List ID. </li>
	<li>  # Radio_Default_IObjectList_Get : This function is used to get the Radio default Object List ID. </li>
	<li>  # Radio_Default_IBaseModule_GetModuleInfo : his function registers the callback function for Radio default base modules. </li>
	<li>  # Radio_Default_IBaseModule_AddListener : This function is used to delete the Radio default Data object ID. </li>
	<li>  # Radio_Default_IBaseModule_Init : This function is used to initialize the Radio default base module. </li>
	<li>  # Radio_Default_IBaseModule_DeInit : This function is used to de-initialize the Radio default base module. </li>
	<li>  #_RadioStationListData_Display_GetAt : This function is used to get the Radio default base module. </li>
	<li>  #_RadioStationListData_Display_GetCount : This function is used to get the count in Radio station list data display. </li>
	<li>  #_RadioStationListDataDisplay_GetInfo : This function is used by listeners to get the Radio station list display. </li>
	<li>  #_RadioStationListDataDisplay_GetId : This function is used by listener to get Object ID radio station list display ID. </li>
	<li>  #_RadioAFSwitchStatus_GetId : This function is used by listener to get Object ID of radio AF switch status ID. </li>
	<li>  #_RadioAFSwitchStatus_Get : This function is used by listeners to get the values of AF switch status. </li>
	<li>  # Radio_Response_AddListener : This function is used to notify the response of notify add listener function to the HMI. </li>
	<li>  #_RadioDABFMlinkstatus_GetId : This function is used by listener to get Object ID of DAB FM link status ID. </li>
.	<li>  #_RadioDABFMlinkstatus_Get : This function is used by listeners to get the values of DAB FM link status . </li>
	<li>  #_RadioAMFMAFlistData_GetId : This function is used by listener to get Object ID of radio AMFM AF list status ID. </li>
	<li>  #_RadioAMFMAFlistData_Get : This function is used by listeners to get the values of AMFM AF list status. </li>
	<li>  #_RadioStatusData_GetId : This function is used to get Object ID of radio status object. </li>
	<li>  #_RadioStatusData_Get : This function is used to get the values of subId of radio status object. </li>
	<li>  #_RadioMemoryListData_Display_GetAt : This function is used to get the Radio memory list display. </li>
	<li>  #_RadioMemoryList_Get : This function is used to get the values of subId in radio memory list. </li>
	<li>  #_RadioMemoryListData_Display_GetCount : This function is used to get the values of subId in radio memory list Sid. </li>
	<li>  #_RadioMemoryListDataDisplay_GetId : This function is used to get the object id of subId of radio memory list. </li>
	<li>  #_RadioMemoryListDataDisplay_GetInfo : This function is used to get the values of subId in radio memory list object. </li>
	<li>  #_RadioAnnouncementData_GetId : This function is used to get the id of announcement object. </li>
	<li>  #_RadioAnnouncementData_Get : This function is used to get the values of subId in announcement object. </li>
	<li>  #_RadioComponentStatusData_GetId : This function is used to get the id of component status object. </li>
	<li>  #_RadioComponentStatusData_Get : This function is used to get the values of subId in component status object. </li>
	<li>  #_RadioSettingsStatusData_GetId : This function is used to get the id of radio setting object. </li>
	<li>  #_RadioSettingsStatusData_Get : This function is used to get the values of subId in radio setting object. </li>
	<li>  #_RadioActivityStatusData_GetId : This function is used to get the id of radio activity status object. </li>
	<li>  #_RadioActivityStatusData_Get : This function is used to get the values of subId in activity status object. </li>
	<li>  #_RadioDABAFlistData_GetId: This function is used to get the id of DAB AF list object. </li>
	<li>  #_RadioDABAFlistData_Get: This function is used to get the values of subId in DAB AF list object. </li>
	<li>  #_RadioBestPIStationData_GetId: This function is used to get the id of Best PI station data object. </li>
	<li>  #_RadioBestPIStationData_Get: This function is used to get the values of subId in Best PI station data object. </li>
	<li>  #_RadioClockTimeInfoData_GetId: This function is used to get the id of Clock Time Info data object. </li>
	<li>  #_RadioClockTimeInfoData_Get: This function is used to get the values of subId in Clock Time Info data object. </li>
		
</ul> 

*/



/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#ifndef	_ICOMMON_H_
#include "hmi_if_common.h"
#endif
#ifndef __IRADIO_API_H__
#include "IRadio.h"
#endif
#ifndef RADIO_MNGR_APP_TYPES_H
#include "radio_mngr_app_types.h"
#endif

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
/**
 * @brief enum represents the HMI IF to set the demute.
 */
#define RADIO_HMI_IF_DEMUTE          1
/**
 * @brief enum represents the HMI IF to set the mute.
 */
#define RADIO_HMI_IF_MUTE            0

/**
 * @brief enum represents the Maximum number of Alternate frequencies.
 */
#define RADIO_MAX_AFLST_SIZE		 25
/**
 * @brief Macro represents the Maximum number of AF frequency for DAB.
 */
#define RADIO_MAX_DAB_ALT_FREQ_SIZE   5

/**
 * @brief Macro represents the Maximum number of AF Ensemble for DAB.
 */
#define RADIO_MAX_DAB_ALT_ENSEMBLE_SIZE   5

/**
 * @brief Macro represents the Maximum number of AF Hardlink SID's.
 */
#define RADIO_MAX_DAB_HARDLINK_SID_SIZE   12
/**
 * @brief represents the Maximum Component label size .
 */
#define RADIO_MAX_COMPONENT_LABEL_SIZE	32

/**
 * @brief represents the Maximum Component label size .
 */
#define RADIO_MAX_CHANNEL_LABEL_SIZE	4

/**
 * @brief represents the Maximum DAB DLS data size .
 */
#define RADIO_MAX_DLS_DATA_SIZE			128

/**
 * @brief represents the Maximum radio text size .
 */
#define RADIO_MAX_CHAN_RADIOTEXT_SIZE	64

/**
 * @brief represents the Maximum PSN name size .
 */
#define RADIO_MAX_CHAN_NAME_SIZE		8

/**
 * @brief represents the Maximum service name size with UTF.
 */
#define RADIO_MAX_SERVICE_NAME_UTF		128

/**
 * @brief represents the Maximum Radio text & DLS size with UTF.
 */
#define RADIO_MAX_DLS_RADIO_TEXT_UTF    512

/**
 * @brief represents value is set to one.
 */
#define SET_FLAG						1

/**
 * @brief represents value is set to zero.
 */
#define CLEAR_FLAG						0

/**
 * @brief represents ascii value for space .
 */
#define RADIO_ASCII_SPACE								((Tu8)(0X20))

/**
* @brief Macro represents the Maximum Length of DABTuner Firmware version software length.
*/
#define RADIO_MAX_DABTUNER_SW_VERSION_LENGTH		(Tu8)48

/**
* @brief Macro represents the Maximum Length of DABTuner firmware version hardware length.
*/
#define RADIO_MAX_DABTUNER_HW_VERSION_LENGTH		(Tu8)10

/**
* @brief Macro represents the Maximum Length of AMFMTuner firmware version length.
*/
#define RADIO_MAX_AMFMTUNER_VERSION_LENGTH			(Tu8)6

/**
 * @brief Macro represents the Maximum Number of characters for Ensemble label.
 */
#define RADIO_MAX_ENSEMBLELABEL_LENGTH			16


/**
 * @brief Macro represents the Maximum Number of Ensemble List.
 */
#define RADIO_MAX_ENSEMBLE						20

/**
 * @brief Macro represents the Normal view for station list.
 */
#define RADIO_NORMAL_STL_VIEW					1

/**
 * @brief Macro represents the Multiplex view for DAB ensemble list.
 */
#define RADIO_MULTIPLEX_VIEW					2


/* As per DAB Standard SLS image maximum size is 50 KB */
#define	RADIO_MAX_DATA_SERVICE_PAYLOAD_SIZE		51200

/**
* @brief Maximum size of program name for FM
*/
#define RADIO_MAX_SIZE_CHAN_PTYNAME				(9u)

/*-----------------------------------------------------------------------------
    variable declarations 
-----------------------------------------------------------------------------*/

/*------------------------------------------RadioInfo structure-----------------------------------------------*/
/**
 * @brief This structure represents the radioInfo which shall be accessed by the listeners. This is for future use.
 */
typedef struct  
{
	CHAR        name[20];
    CHAR        version[10];
    CHAR        date[20];
	PFN_NOTIFY  pfnNotify;
}tRadioInfo;

/*------------------------------------------RadioCommonData structure------------------------------------------*/
/**
 * @brief This structure represents the Radio common Data for the RADIO_DOID_STATION_INFO Object ID.
 */
typedef struct  
{
    MODE_TYPE                nBand;
	MODE_TYPE				 Aud_Band;
    UINT32                   Frequency;
	CHAR                     ServiceCompName[RADIO_MAX_COMPONENT_LABEL_SIZE];
	CHAR                     DLS_Radio_Text[RADIO_MAX_DLS_DATA_SIZE];
	UINT8					 Char_set;
	UINT8					 Channel_Name[RADIO_MAX_CHANNEL_LABEL_SIZE];
	UINT8					 Ensemble_Label[RADIO_MAX_ENSEMBLELABEL_LENGTH];
	UINT8					 DAB_Current_Service;
	UINT8					 DAB_Total_Num_Services_In_Ensemble;
	UINT8					 Programme_Type[RADIO_MAX_SIZE_CHAN_PTYNAME];
	UINT8					 TA;
	UINT8					 TP;
	UINT16					 PI;
}tRadioCommonDataBox;

/**
 * @brief This structure represents the status for the RADIO_DOID_STATUS Object ID.
 */

typedef struct
{
	Radio_ResultCode	nStatus;
}tRadioDataBoxStatus;


/*------------------------------------------Radio station list structure for AM-----------------------------------*/
/**
 * @brief This structure represents the AM station list information for Object ID RADIO_DOID_STATION_LIST_DIAG Object ID.
 */
typedef struct
{ 
  MODE_TYPE          nBand;
  UINT32             Frequency;
}tRadioStationListDataBox_AM;

/*------------------------------------------Radio station list structure for FM-----------------------------------*/
/**
 * @brief This structure represents the FM station list information for Object ID RADIO_DOID_STATION_LIST_DIAG Object ID.
 */
typedef struct
{ 
  MODE_TYPE     nBand;
  UINT32        Frequency;
  UINT16        PI;
  UINT8			Char_set;
  CHAR          PServiceName[RADIO_MAX_CHAN_NAME_SIZE];
}tRadioStationListDataBox_FM;

/*------------------------------------------Radio station list structure for DAB----------------------------------*/
/**
 * @brief This structure represents the DAB station list information for Object ID RADIO_DOID_STATION_LIST_DIAG Object ID.
 */
typedef struct
{
  MODE_TYPE     nBand;
  UINT32        Frequency; 
  UINT32        ServiceID;
  UINT16        uEnsembleID;
  UINT16        ServiceCompID;
  CHAR          Service_ComponentName[RADIO_MAX_COMPONENT_LABEL_SIZE];
  UINT8			Char_set;
}tRadioStationListDataBox_DAB;

/*------------------------------------------RadioStnInfo structure for FM----------------------------------*/
/**
 * @brief This structure represents the FM Station information for Object ID RADIO_DOID_STATION_INFO_DIAG Object ID.
 */
typedef struct
{
  MODE_TYPE     nBand;
  UINT32        Frequency;
  UINT8         u32_Quality;
  CHAR          PServiceName[RADIO_MAX_CHAN_NAME_SIZE];
  UINT16        u16_PI;
  UINT8         u8_TA;
  UINT8         u8_TP;
  UINT8         u8_Char_Set;
}tRadioAMFMStnInfoDataBox;

/*------------------------------------------RadioStnInfo structure for DAB----------------------------------*/
/**
 * @brief This structure represents the DAB Station information for Object ID RADIO_DOID_STATION_INFO_DIAG Object ID.
 */
typedef struct
{
  UINT32        Frequency;
  UINT32        ServiceID;
  UINT16        uEnsembleID;
  UINT16        CompID;
  CHAR          ServiceName[RADIO_MAX_COMPONENT_LABEL_SIZE];
  UINT8			Char_set;
  MODE_TYPE     eBand;
}tRadioDABStnInfoDataBox;

/*------------------------------------------Radio Quality structure for AMFM----------------------------------*/
/**
 * @brief This structure represents the AMFM Quality information for Object ID RADIO_DOID_AMFM_QUALITY Object ID.
 */
typedef struct
{
    MODE_TYPE       								nBand;
	Ts32     										s32_BBFieldStrength; 
	Ts32											s32_RFFieldStrength;
	Tu32       										u32_UltrasonicNoise;
	Tu32        									u32_Multipath;
	Tu32											u32_SNR;
	Tu32											u32_AdjacentChannel;
	Tu32											u32_coChannel;
	Tu32											u32_StereoMonoReception;
	Tu8												u32_Quality;								/*Quality of the station */
	Tu32											u32_FrequencyOffset;						/*Frequency offset in kHz */
	Tu8												u8_IF_BW;									/*IF bandwidth */
	Tu32											u32_ModulationDetector;						/*Modulation Detector*/
}tRadioAMFMQualityDataBox;

/*------------------------------------------Radio Quality structure for DAB----------------------------------*/
/**
 * @brief This structure represents the DAB Quality information for Object ID RADIO_DOID_QUALITY_DIAG Object ID.
 */
typedef struct
{
	Ts32 			s32_RFFieldStrength;
	Ts32 			s32_BBFieldStrength;
	Tu32 			u32_FicBitErrorRatio;
	Tbool 			b_isValidFicBitErrorRatio;
	Tu32 			u32_MscBitErrorRatio;
	Tbool 			b_isValidMscBitErrorRatio;
	Tu32 			u32_DataSChBitErrorRatio;
	Tbool 			b_isValidDataSChBitErrorRatio;
	Tu32 			u32_AudioSChBitErrorRatio;
	Tbool 			b_isValidAudioSChBitErrorRatio;
	Tu8 			u8_AudBitErrorRatio;
	Tu8 			u8_ReedSolomonInfo;
	Tu8 			u8_syncStatus;
	Tbool 			b_muteStatus;
	Tu8       		u8_BER_Significant;
	Ts8       		s8_BER_Exponent;
	Ts8	   			s8_SNR_Level;
	Ts8          	s8_RSSI;
	Tu8 			u8_DAB_Audio_Decoding_Status;
	Tu8 			u8_DAB_Audio_Quality;
	Tu8 			u8_DAB_Audio_Level;
  MODE_TYPE     eBand;
}tRadioDABQualityDataBox;

/*------------------------------------------Radio AF Switch Status structure-------------------------------------*/
/**
 * @brief This structure represents the FM AF switch status for Object ID RADIO_DOID_AF_STATUS Object ID.
 */
typedef struct
{
  Radio_AF_Switch_Status   Status;
}
tRadioAFSwitchStatusDataBox;

/*------------------------------------------DAB FM Link Status structure-------------------------------------*/
/**
 * @brief This structure represents the DAB FM Link status for Object ID RADIO_DOID_DABFM_LINK_STATUS Object ID.
 */
typedef struct
{
  Radio_DABFM_Linking   Status;
}
tRadioDABFMLinkStatusDataBox;

/*----------------------------------------------AF List structure-------------------------------------*/
/**
 * @brief This structure represents the FM AF switch status for Object ID RADIO_DOID_AF_LIST Object ID.
 */
typedef struct
{
  UINT8     	NumAFList;
  UINT16 		AFList[RADIO_MAX_AFLST_SIZE];
  UINT16    	PIList[RADIO_MAX_AFLST_SIZE];
  UINT8	    	Quality[RADIO_MAX_AFLST_SIZE];
  MODE_TYPE     nBand;
}tRadioAMFMAFListDataBox;

/**
 * @brief This structure represents the DAB AF List structure.
 */
typedef struct
{
  UINT8       NumofAltFrequecy;
  UINT8       NumofAltEnsemble;
  UINT8       NumofHardlinkSid;
  UINT32      DAB_AltFreqList[RADIO_MAX_DAB_ALT_FREQ_SIZE];
  UINT16      DAB_AltEnsembleList[RADIO_MAX_DAB_ALT_ENSEMBLE_SIZE];
  UINT32      DAB_HardlinkSidList[RADIO_MAX_DAB_HARDLINK_SID_SIZE];
  MODE_TYPE   nBand;
}tRadioDABAFListDataBox;

/*------------------------------------------Radio station list display structure -----------------------------------*/
/**
 * @brief This structure represents the AM station list information for Object ID RADIO_DOID_STATION_LIST_DISPLAY.
 */
typedef struct
{ 
  MODE_TYPE     nBand;
  UINT32        Frequency;
  CHAR          ServiceName[RADIO_MAX_COMPONENT_LABEL_SIZE];
  UINT8         Index;
  BOOL			Matched_Stn_Index_Flag;
  BOOL			MatchedPresetInSTL_index_Flag;
  UINT8			Char_set;
}tRadioStationListDataBox_Display;

/*============================================================================================================================
------------------------------------------Radio Ensemble list structure -----------------------------------
==============================================================================================================================*/

/**
 * @brief This structure represents the Ensemble list for object ID RADIO_DOID_MULTIPLEX_ENSEMBLE_DISPLAY.
 */
typedef struct
{
  CHAR          EnsembleName[RADIO_MAX_ENSEMBLELABEL_LENGTH];
  UINT8         Index;
  UINT8			Char_set;
}tRadioEnsembleListDataBox_Display;

/*============================================================================================================================
------------------------------------------Radio Searched station list display structure -----------------------------------
==============================================================================================================================*/

/**
* @brief This structure represents the preset mixed list for object ID RADIO_DOID_MEMORY_LIST.
*/
typedef struct
{ 
  MODE_TYPE     nBand;
  UINT32        Frequency;
  CHAR          ServiceName[RADIO_MAX_COMPONENT_LABEL_SIZE];
  UINT8         Index;
  BOOL			Matched_Search_Stn_Index_Flag;
  BOOL			MatchedPresetInSearchSTL_index_Flag;
  UINT8			Char_set;
}tRadioSTLSearchDataBox_Display;

/*------------------------------------------Preset list display structure -----------------------------------*/
/**
 * @brief This structure represents the preset mixed list for object ID RADIO_DOID_MEMORY_LIST.
 */
typedef struct
{
  MODE_TYPE     nBand;
  UINT32        Frequency;
  CHAR			MemoryStName[RADIO_MAX_COMPONENT_LABEL_SIZE];
  UINT8         index;
  UINT8			DAB_Channel_Name[RADIO_MAX_CHANNEL_LABEL_SIZE];
  UINT8			Char_set;
}tRadioMemoryListInfoDataBox_Display;


/*------------------------------------------Announcement Related structure -----------------------------------*/
/**
 * @brief This structure represents the Announcement related information for object ID RADIO_DOID_ANNOUNCEMENT.
 */
typedef struct
{
	Radio_Announcement_Status		Anno_Status;
}tRadioAnnouncementInfoDataBox;

/**
 * @brief This structure represents the Component status related information for object ID RADIO_DOID_COMPONENT_STATUS.
 */
typedef struct
{
	Radio_AMFMTuner_Status				e_AMFMTuner_Status;
    Radio_Component_Status      		e_DABTuner_Status;
	Radio_DAB_UpNotification_Status		e_DAB_UpNot_Status;
	MODE_TYPE                   		nBand;
}tRadioComponentStatusInfoDataBox;

/**
 * @brief This structure represents the Announcement related information for object ID RADIO_DOID_ANNOUNCEMENT.
 */
typedef struct
{
	Radio_Switch_Setting_Status		e_DABFM_Status;
	Radio_Switch_Setting_Status		e_TA_Anno_Status;
	Radio_Switch_Setting_Status		e_RDS_Status;
	Radio_Switch_Setting_Status		e_Info_Anno_Status;
	Radio_Switch_Setting_Status		e_Multiplex_Switch_Status;
}tRadioSettingsStatusInfoDataBox;
/**
 * @brief This structure represents the activity state information.
 */
typedef struct
{
    MODE_TYPE                   nBand;
    Radio_Activity_Status       e_Activity_Status;  
}tRadioActivityStatusDataBox;

/**
 * @brief This structure represents the best PI station data
 */
typedef struct
{
    UINT32          Frequency;
    UINT16          Best_PI;
    UINT8           Quality;
}tRadioBestPIStationDataBox;
/**
 * @brief This structure represents the Clock Time Info
 */
typedef struct
{
    UINT8          	Hour;
    UINT8          	Minutes;
    UINT8          	Day;
	UINT8          	Month;
	UINT16          Year;
}tRadioGetClockTimeInfoDataBox;
/**
 * @brief This structure represents the AMFMTuner & DABTuner Firmware versions
 */
typedef struct
{
	UINT8          	DABTuner_FW_SWVersion[RADIO_MAX_DABTUNER_SW_VERSION_LENGTH];
	UINT8          	DABTuner_FW_HWVersion[RADIO_MAX_DABTUNER_HW_VERSION_LENGTH];
    UINT8          	AMFMTuner_FW_Version[RADIO_MAX_AMFMTUNER_VERSION_LENGTH];
}tRadioGetFirmareVersionInfoDataBox;

/**
* @brief This structure represents the DAB Data service information
*/
typedef struct
{
	Radio_EtalDataServiceType	e_Header;
	UINT8						u8_Payload[RADIO_MAX_DATA_SERVICE_PAYLOAD_SIZE];
	UINT32						u32_PayloadSize;
}tRadioDABDataServiceInfoDataBox_Display;

/*-----------------------------------------------------------------------------------------------------------*/
/*                                           Function Prototypes                                             */
/*-----------------------------------------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 <b> Brief </b>			\n This function provides the Radio module information like name,version and date to the listeners \n \n
*   \param[in]				ppInfo		Pointer to pointer variable for HMIIF_IDataObject type structure
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio module information is received by HMI IF component
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL _Radio_GetModuleInfo(HMIIF_IDataObject** ppInfo);
/*****************************************************************************************************/
/**	 <b> Brief </b>			\n This function registers the callback function provided by the listener, Listeners will call this API \n \n
*   \param[in]				pListener	 Pointer to IBaseModule type structure
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for add listener is received by HMI IF component
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _Radio_AddListener(IBaseModule* pListener);
/*****************************************************************************************************/
/**	<b> Brief </b>			\n This function is used to initialize the RadioModule,It will be used from mode manager,All actions that has to be performed during initialization is performed here \n \n
*   \param[in]				None
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio module initialization is received by HMI IF component
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _Radio_ModuleInit();
/*****************************************************************************************************/
/**	 <b> Brief </b>			\n This is the callback function of radio module which is provided to listeners \n \n
*   \param[in]				pData		Pointer to HMIIF_IDataObject type structure
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Call back function of radio module is processed by HMI IF component
*   \return					none
* 
******************************************************************************************************/
void _Radio_Notify(HMIIF_IDataObject* pData);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the Radio module informations \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		SubId
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio get info is received by HMI IF component
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _Radio_GetInfo(void*thiss, INT nSubId, DWORD* pdwData);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function shall be used to set the Radio module information \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID
*   \param[in]				dwData		Variable of long type
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio set info is received by HMI IF component
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _Radio_SetInfo(void*thiss, INT nSubId, DWORD dwData);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of radio common data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting radio common data get Id is received by the HMI IF
*   \return					This API returns the Object ID of radio common data.
*	\retval					#RADIO_DOID_STATION_INFO
* 
******************************************************************************************************/
INT _RadioCommonData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of subId in Common data Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_STATION_INFO)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio common data get is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioCommonData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the Object ID of station list Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list data get Id is received by HMI IF
*   \return					This API returns the Object ID of station list Data
*	\retval					#RADIO_DOID_STATION_LIST_DIAG
* 
******************************************************************************************************/
INT _RadioStationListData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the station list information of AM in station list Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_STATION_LIST_DIAG)
*   \param[out]				pdwData		Pointer to some long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station data get AM info is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL _RadioStationListData_GetAMInfo(void* thiss, INT nSubId, DWORD* pdwData);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the station list information of FM in station list Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_STATION_LIST_DIAG)
*   \param[out]				pdwData		Pointer to some long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station data get FM info is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioStationListData_GetFMInfo(void* thiss, INT nSubId, DWORD* pdwData);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function shall be used to get the station list information of DAB in station list Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_STATION_LIST_DIAG)
*   \param[out]				pdwData		Pointer to some long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station data get DAB info is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioStationListData_GetDABInfo(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the AM station list information from the list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nIndex		Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list get AM is received by HMI IF
*   \return					This API returns the address of AM station list array. 
* 
******************************************************************************************************/
HMIIF_IDataObject* _RadioStationList_GetAMAt(void *thiss, INT nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the FM station list information from the list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nIndex		Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list get FM is received by HMI IF
*   \return					This API returns the address of FM station list array.
* 
******************************************************************************************************/
HMIIF_IDataObject* _RadioStationList_GetFMAt(void *thiss, INT nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the DAB station list information from the list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nIndex		Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list get DAB is received by HMI IF
*   \return					This API returns the address of DAB station list array.
* 
******************************************************************************************************/
HMIIF_IDataObject* _RadioStationList_GetDABAt(void *thiss, INT nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the AM station list count \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list get AM count is received by HMI IF
*   \return					This API returns the number of AM stations in the AM station list. 
* 
******************************************************************************************************/
INT _RadioStationList_GetAMCount(void *thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the FM station list count \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list get FM count is received by HMI IF
*   \return					This API returns the number of FM stations in the FM station list. 
* 
******************************************************************************************************/
INT _RadioStationList_GetFMCount(void *thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the DAB station list count \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None	
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list get DAB count is received by HMI IF
*   \return					This API returns the number of DAB stations in the DAB station list. 
* 
******************************************************************************************************/
INT _RadioStationList_GetDABCount(void *thiss);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of FM station information data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio FM station info data get Id is received by the HMI IF
*   \return					This API returns Object ID of FM station information data
*	\retval					#RADIO_DOID_AMFM_STATION_INFO
* 
******************************************************************************************************/
INT _RadioAMFMStnInfoData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of subId in FM StationInfo data Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_STATION_INFO_DIAG)
*   \param[out]				pdwData		pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio FM station info data get is received by the HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL _RadioAMFMStnInfoData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of DAB station information data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio DAB station info data get Id is received by the HMI IF
*   \return					This API returns the Object ID of DAB station information data
*	\retval					#RADIO_DOID_DAB_STATION_INFO
* 
******************************************************************************************************/
INT _RadioDABStnInfoData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of subId in FM StationInfo data Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DATA_OBJECT_STNINFO_SUBID)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio DAB station info data get is received by the HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL _RadioDABStnInfoData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of AMFM quality data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio AMFM quality data get Id is received by HMI IF
*   \return					This API returns the Object ID of AMFM quality data
*	\retval					#RADIO_DOID_AMFM_QUALITY
* 
******************************************************************************************************/
INT _RadioAMFMQualityData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of subId in AMFM Quality data Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_QUALITY_DIAG)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio AMFM quality data get is received by the HMI IF
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL _RadioAMFMQualityData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of DAB quality data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio DAB quality data get Id is received by the HMI IF
*   \return					This API returns the Object ID of DAB quality data
*	\retval					#RADIO_DOID_DAB_QUALITY
* 
******************************************************************************************************/
INT _RadioDABQualityData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of subId in DAB Quality data Object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_QUALITY_DIAG)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio DAB quality data get is received by the HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL _RadioDABQualityData_Get(void*  thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of radio mute status \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio mute status get Id is received by HMI IF
*   \return					This API returns the Object ID of radio mute status
*	\retval					#RADIO_DOID_MUTE_STATUS
* 
******************************************************************************************************/
INT _RadioMuteStatus_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of subId in radio mute status \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DATA_OBJECT_MUTE_DEMUTE_SUBID)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio mute status get is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL _RadioMuteStatus_Get(void* thiss, INT nSubId, DWORD* pdwData);

//For HMIIF_IDataObject

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the ID of the Radio default Data object \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns the ID of the Data object
* 
******************************************************************************************************/
INT Radio_Default_IDataObject_GetId(void*thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the values of Radio default Data object ID \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub ID
*   \param[out]				pdwData		Pointer to long data type
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL Radio_Default_IDataObject_Get(void*thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the Radio default Object List \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nIndex		Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns the address of the Object List
* 
******************************************************************************************************/
HMIIF_IDataObject* Radio_Default_IObjectList_GetAt(void*thiss, INT nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the count in Radio default Object List \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns the count in Object List
* 
******************************************************************************************************/
INT Radio_Default_IObjectList_GetCount(void*thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the Radio default Object List \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nId			ID
*   \param[out]				pdwData		Pointer to long data type
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
* 
******************************************************************************************************/
BOOL Radio_Default_IObjectList_Get(void*thiss, INT nId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the module information for Radio default base module \n \n
*   \param[in]				ppInfo		Pointer to pointer which points to HMIIF_IDataObject type structure
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure 
* 
******************************************************************************************************/
BOOL Radio_Default_IBaseModule_GetModuleInfo(HMIIF_IDataObject** ppInfo);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function registers the callback function for Radio default base modules \n \n
*   \param[in]				pListener	Pointer to IBaseModule type structure 
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure  
* 
******************************************************************************************************/
BOOL Radio_Default_IBaseModule_AddListener(IBaseModule* pListener);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to initialize the Radio default base module \n \n
*   \param[in]				None
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure  
* 
******************************************************************************************************/
BOOL Radio_Default_IBaseModule_Init();

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to run the Radio default base module \n \n
*   \param[in]				pParameter		Pointer to HMIIF_IDataObject type structure
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure  
* 
******************************************************************************************************/
BOOL _Radio_ModuleRun(HMIIF_IDataObject* pParameter);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to stop the default base module \n \n
*   \param[in]				pParameter		Pointer to HMIIF_IDataObject type structure
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure  
* 
******************************************************************************************************/
BOOL _Radio_ModuleStop(HMIIF_IDataObject* pParameter);

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to set the Radio default base module \n \n
*   \param[in]				thiss			Pointer to the Data Source
*   \param[in]				nIndex			Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure  
* 
******************************************************************************************************/
HMIIF_IDataObject* _RadioStationListData_Display_GetAt(void *thiss, INT nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the count in Radio station list data display \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns the count of stations to be displayed
* 
******************************************************************************************************/
INT _RadioStationListData_Display_GetCount(void *thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the Radio station list display \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		SubId
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list display is received by HMI IF component
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioStationListDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID radio station list display \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting radio station list display get Id is received by the HMI IF
*   \return					This API returns the Object ID of radio station list display.
*	\retval					#RADIO_DOID_STATION_LIST_DISPLAY
* 
******************************************************************************************************/
INT _RadioStationListDataDisplay_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of radio AF switch status \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting radio AF switch status get Id is received by the HMI IF
*   \return					This API returns the Object ID of radio AF switch status.
*	\retval					#RADIO_DOID_AF_STATUS
* 
******************************************************************************************************/
INT _RadioAFSwitchStatus_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of AF switch status \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_AF_STATUS)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio common data get is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioAFSwitchStatus_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to notify the response of notify add listener function to the HMI \n \n
*   \param[in]				Status		 
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio mute status set is received by HMI IF
*   \return					This API returns #RADIO_ADDLISTENER_SUCCESS when add listener response is success otherwise it sends #RADIO_ADDLISTENER_FAILURE
*
******************************************************************************************************/
void Radio_Response_AddListener(Tu8 ListenerIdx, Radio_ResultCode Status);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of DAB FM linking status \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting DAB FM linking status get Id is received by the HMI IF
*   \return					This API returns the Object ID of DAB FM linking status.
*	\retval					#RADIO_DOID_DABFM_LINK_STATUS
* 
******************************************************************************************************/
INT _RadioDABFMlinkstatus_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of DAB FM linking status \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_DABFM_LINK_STATUS)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for DAB FM linking get is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioDABFMlinkstatus_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of radio AF list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting radio AF list get Id is received by the HMI IF
*   \return					This API returns the Object ID of radio AF list
*	\retval					#RADIO_DOID_AF_LIST
* 
******************************************************************************************************/
INT _RadioAMFMAFlistData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of AF list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_AF_LIST)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for AF list get is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioAMFMAFlistData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of Radio status Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Radio status Data get Id is received by the HMI IF
*   \return					This API returns the Object ID of Radio status Data
*	\retval					#RADIO_DOID_STATUS
* 
******************************************************************************************************/
INT _RadioStatusData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Radio status \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_STATUS)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of Radio status is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioStatusData_Get(void*  thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to set the Radio Memory list functions \n \n
*   \param[in]				thiss			Pointer to the Data Source
*   \param[in]				nIndex			Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure  
* 
******************************************************************************************************/
HMIIF_IDataObject* _RadioMemoryListData_Display_GetAt(void *thiss, INT nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Radio Memory list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_STATUS)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of Radio Memory list is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioMemoryList_Get(void *thiss, INT nId, DWORD* pdwData);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the count in Radio Memory list data display \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns the count of stations to be displayed
******************************************************************************************************/
INT _RadioMemoryListData_Display_GetCount(void *thiss);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of Radio Memory list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Radio Memory list Data get Id is received by the HMI IF
*   \return					This API returns the Object ID of Radio Memory list
*	\retval					#RADIO_DOID_MEMORY_LIST
* 
******************************************************************************************************/
INT _RadioMemoryListDataDisplay_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the Radio Memory list display \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		SubId
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for Radio Memory list get info display is received by HMI IF component
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioMemoryListDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of Announcement \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Announcement get Id is received by the HMI IF
*   \return					This API returns the Object ID of Announcement Feature
*	\retval					#RADIO_DOID_ANNOUNCEMENT
* 
******************************************************************************************************/
INT _RadioAnnouncementData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Announcement feature \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_ANNOUNCEMENT)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of Announcement feature is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioAnnouncementData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of Radio Component Status Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Radio Component Status Data get Id is received by the HMI IF
*   \return					This API returns the Object ID of Radio Component Status Data
*	\retval					#RADIO_DOID_COMPONENT_STATUS
* 
******************************************************************************************************/
INT _RadioComponentStatusData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Radio Component Status Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_COMPONENT_STATUS)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of Radio Component Status Data is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioComponentStatusData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of Radio Setting Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Radio Setting Data get Id is received by the HMI IF
*   \return					This API returns the Object ID of Radio Setting Data
*	\retval					#RADIO_DOID_SETTINGS
* 
******************************************************************************************************/
INT _RadioSettingsStatusData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Radio Setting Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_SETTINGS)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of Radio Setting Data is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioSettingsStatusData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of Radio Activity status Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Radio Setting Data get Id is received by the HMI IF
*   \return					This API returns the Object ID of Radio Activity status Data
*	\retval					#RADIO_DOID_ACTIVITY_STATE
* 
******************************************************************************************************/
INT _RadioActivityStatusData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Radio Activity status Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_ACTIVITY_STATE)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of Radio Setting Data is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioActivityStatusData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of DAB AF List Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting DAB AF list Data get Id is received by the HMI IF
*   \return					This API returns the Object ID of Radio Activity status Data
*	\retval					#RADIO_DOID_AF_LIST
* 
******************************************************************************************************/
INT _RadioDABAFlistData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of DAB AF List Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_AF_LIST)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of DAB AF list Data is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioDABAFlistData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of Best PI station information \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting Best PI info Data get Id is received by the HMI IF
*   \return					This API returns the Object ID of Best PI station data
*	\retval					#RADIO_DOID_BESTPI_INFO
* 
******************************************************************************************************/
INT _RadioBestPIStationData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Best PI station Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_BESTPI_INFO)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting the values of Best PI station Data is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioBestPIStationData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of get clock time request \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					ENG Mode shall be ON
*   \post					Request for getting clock time data get Id is received by the HMI IF
*   \return					This API returns the Object ID of get clock time data
*	\retval					#RADIO_DOID_CLOCKTIME_INFO
* 
******************************************************************************************************/
INT _RadioClockTimeInfoData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of Clock time Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_CLOCKTIME_INFO)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					ENG Mode shall be ON
*   \post					Request for getting the values of Clock Time Data is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioClockTimeInfoData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of get firmware version request \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					ENG Mode shall be ON
*   \post					Request for getting firmware version data get Id is received by the HMI IF
*   \return					This API returns the Object ID of get clock time data
*	\retval					#RADIO_DOID_FIRMWARE_VERSION_INFO
* 
******************************************************************************************************/
INT _RadioFirmwareVersionInfoData_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the values of firmware version Data \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		Sub object ID(RADIO_DOID_FIRMWARE_VERSION_INFO)
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					Radio should be in active Mode
*   \post					Request for getting the values of firmware version Data is received by HMI IF
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioFirmwareVersionInfoData_Get(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the count if system requested for searching the station list \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns the count in Object List
* 
******************************************************************************************************/
INT _RadioSTLSearchData_Display_GetCount(void *thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to set the Radio default base module \n \n
*   \param[in]				thiss			Pointer to the Data Source
*   \param[in]				nIndex			Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure  
* 
******************************************************************************************************/
HMIIF_IDataObject* _RadioSTLSearchData_Display_GetAt(void *thiss, INT nIndex);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of searched radio station list display \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting searched radio station list display get Id is received by the HMI IF
*   \return					This API returns the Object ID of radio station list display.
*	\retval					#RADIO_DOID_STL_SEARCH_DISPLAY
* 
******************************************************************************************************/
INT _RadioSTLSearchDataDisplay_GetId(void* thiss);
/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the searched Radio station list display info \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		SubId
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list display is received by HMI IF component
*   \return					This API returns #BOOL 
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioSTLSearchDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData);

INT _RadioEnsembleListData_Display_GetCount(void *thiss);
HMIIF_IDataObject* _RadioEnsembleListData_Display_GetAt(void *thiss, INT nIndex);
INT _RadioEnsembleListDataDisplay_GetId(void* thiss);
BOOL _RadioEnsembleListDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listener to get Object ID of DAB Data Service display \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for getting searched radio station list display get Id is received by the HMI IF
*   \return					This API returns the Object ID of radio station list display.
*	\retval					#RADIO_DOID_DATA_SERVICE
*
******************************************************************************************************/
INT _RadioDABDataServiceDisplay_GetId(void* thiss);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used by listeners to get the DAB Data Service display info \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nSubId		SubId
*   \param[out]				pdwData		Pointer to DWORD long data type
*   \pre					HMI IF is ready to process any request
*   \post					Request for radio station list display is received by HMI IF component
*   \return					This API returns #BOOL
*	\retval					#TRUE on Success
*	\retval					#FALSE on Failure
*
******************************************************************************************************/
BOOL _RadioDABDataServiceDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData);

/*****************************************************************************************************/
/**	  <b> Brief </b>		\n This function is used to get the DAB Data Service information from the list
								at a particular index \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[in]				nIndex		Index
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \post					Request for DAB Data Service data at the index is received by HMI IF
*   \return					This API returns the address of DAB Data Service at the index.
*
******************************************************************************************************/
HMIIF_IDataObject* _RadioDABDataService_Display_GetAt(void *thiss, INT nIndex);

/*****************************************************************************************************/
/**	  <b> Brief </b>		This function is used to get the size of the DAB Data Service Payload \n \n
*   \param[in]				thiss		Pointer to the Data Source
*   \param[out]				None
*   \pre					HMI IF is ready to process any request
*   \return					This API returns the size of the DAB Data Service Payload
*
******************************************************************************************************/
INT _RadioDABDataService_Display_GetSize(void *thiss);

#endif
/*=============================================================================
    end of file
=============================================================================*/