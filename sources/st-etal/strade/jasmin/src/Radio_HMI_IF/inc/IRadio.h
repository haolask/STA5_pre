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
*  Description			: This file consists of detailed list of SubIds for every Object ID.
*
***********************************************************************************************************/
/** \file IRadio.h 
This file consists of detailed list of SubIds for every Object ID 
*********************************************************************************************************/
#ifndef __IRADIO_API_H__
#define __IRADIO_API_H__

/** \file */

/** \page HMI_IF_Notify_top HMI IF Notify Package 

\subpage	HMI_IF_Notify_Overview 
\n
\subpage	HMI_IF_Notify_API
\n

*/

/**\page HMI_IF_Notify_Overview Overview
	\n
	HMI IF Notify package consists of enums which are used in sending the notifications to the HMI.
	\n\n
*/

/**\page HMI_IF_Notify_API HMI IF Notify APIs
	\n
	
<b><i> _pfnNotifyHdlr </i></b> is a function pointer which is invoked by RadioLib to send the notifications.
	
Data is notified to HMI using Object IDs and Sub IDs.\n 
<b><i>Object ID</i></b> - denotes the type of data that is notified.\n
<b><i>Sub ID</i></b> - to retrieve the data corresponding to an Object ID. 

GetId Function:
--------------

In the callback handler function of type <b><i>void (\*PFN_NOTIFY)(#HMIIF_IDataObject*)</i></b>, HMI shall retrieve the object ID using <b><i>GetId</i></b> function.

<i>For example:</i>

	pData->GetId(pData->thiss);

This call returns the type of data that is being notified. <i>Eg., Status, Station list, Preset list, etc.,</i>

Get Function:
------------

For every Object ID, there is a list of Sub IDs which can be used for retrieving the actual data. HMI shall retrieve the data through Sub IDs using <b><i>Get</i></b> function.

<i>For example:</i>

For the object ID #RADIO_DOID_ACTIVITY_STATE, there are two sub IDs - #RADIO_DOSID_ACTIVITY_STATE_BAND & #RADIO_DOSID_ACTIVITY_STATE

So, in the handler, if the Object ID is #RADIO_DOID_ACTIVITY_STATE, the following code snippet retrieves band and state information in <i>'data'</i>.

	pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE_BAND, &data);
	pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE, &data);


GetCount & GetAt:
-----------------

This procedure is applicable only to the Object IDs that involve an array of data to be notified.

For the object IDs <i>station list, preset list and Data Service</i>, there are two additional functions that has to be invoked.

<b><i>GetCount</i></b> function provides the size of the station list/preset list/Data Service payload.\n\n
<b><i>GetAt</i></b> function notifies HMI IF to point to a particular index of the station list/preset list/Data Service payload, so that a <b><i>Get</i></b> function that follows this call, can retrieve data at that index of the array.

For example, in the handler, if the object ID is #RADIO_DOID_STATION_LIST_DISPLAY, the following code snippet can retrieve the entire station list information.


	case RADIO_DOID_STATION_LIST_DISPLAY:
	{
		IObjectList* StnList = ((IObjectList*)pData->thiss);
		pData->Get(StnList->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_BAND, &data);
		
		StnListCount = StnList->GetCount(StnList->thiss);
		
		for (i = 0; i < StnListCount; i++)
		{
			StnListObj = StnList->GetAt(StnList->thiss, i);
			StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_INDEX, &data);
			StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_FREQUENCY, &data);
			StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_SERVICENAME, &data);
			StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_TUNEDSTN_INDEX, &data);
		}
	}
	break;

The Object IDs are in the enum #RADIO_DATA_OBJECT_ID. The mapping of the Object IDs and the enums containing the SubIDs is as follows:

	
|			OBJECT ID 					| SUB ID 								   					|
|---------------------------------------|-----------------------------------------------------------|
|#RADIO_DOID_STATION_INFO 				|	#RADIO_DATA_OBJECT_COMMONDATA_SUBID       				|		    
|#RADIO_DOID_STATION_LIST_DISPLAY 		|	#RADIO_DATA_OBJECT_STATIONLIST_DISPLAY_SUBID      		|				
|#RADIO_DOID_STATION_LIST_DIAG 			|	#RADIO_DATA_OBJECT_STATIONLIST_DIAG_SUBID 				|
|#RADIO_DOID_STATION_INFO_DIAG 			|	#RADIO_DATA_OBJECT_STNINFO_SUBID 						|
|#RADIO_DOID_QUALITY_DIAG 				|	#RADIO_DATA_OBJECT_QUALITY_SUBID 						|
|#RADIO_DOID_MEMORY_LIST 				|	#RADIO_DATA_OBJECT_MEMORYLIST_SUBID 					|
|#RADIO_DOID_AF_LIST 					|	#RADIO_DATA_OBJECT_AFLIST_SUBID 						|
|#RADIO_DOID_AF_STATUS 					|	#RADIO_DATA_OBJECT_AF_SWITCH_STATUS_SUBID 				|
|#RADIO_DOID_DABFM_LINK_STATUS 			|	#RADIO_DATA_OBJECT_DABFM_LINK_STATUS_SUBID 				|	
|#RADIO_DOID_STATUS 					|	#RADIO_DATA_OBJECT_RESULTCODE_STATUS_SUBID 				|
|#RADIO_DOID_ANNOUNCEMENT 				|	#RADIO_ANNO_DATAOBJECT_SUBID 							|
|#RADIO_DOID_COMPONENT_STATUS 			|	#RADIO_DATA_OBJECT_RADIO_COMPONENT_STATUS_SUBID 		|	
|#RADIO_DOID_SETTINGS					|	#RADIO_DATA_OBJECT_RADIO_SWITCH_SETTING_STATUS 			|
|#RADIO_DOID_ACTIVITY_STATE 			|	#RADIO_DATA_OBJECT_ACTIVITY_STATE_SUBID 				|
|#RADIO_DOID_BESTPI_INFO 				|	#RADIO_DATA_OBJECT_BESTPI_INFO_SUBID 					|	
|#RADIO_DOID_CLOCKTIME_INFO 			|	#RADIO_DATA_OBJECT_CLOCKTIME_INFO_SUBID 				|
|#RADIO_DOID_FIRMWARE_VERSION_INFO 		|	#RADIO_DATA_OBJECT_RADIO_FIRMWARE_VERSION_SUBID 		|
|#RADIO_DOID_STL_SEARCH_DISPLAY 		|	#RADIO_DATA_OBJECT_STL_SEARCH_DISPLAY_SUBID 			|
|#RADIO_DOID_MULTIPLEX_ENSEMBLE_DISPLAY |	#RADIO_DATA_OBJECT_MULTIPLEXENSEMBLELIST_DISPLAY_SUBID 	|
|#RADIO_DOID_DATA_SERVICE 				|	#RADIO_DATA_OBJECT_RADIO_DAB_DATASERVICE_SUBID 			|

*/	




	
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "hmi_if_common.h"
/*-----------------------------------------------------------------------------
                                Macro Definitions
-----------------------------------------------------------------------------*/


/**
 * @brief Maximum Number of Listeners for Radio.
 */
#define MAX_RADIO_LISTENER                  2
/**
 * @brief Maximum Number of Modes for Radio.
 */
#define MAX_RADIO_MODE                      3    /* 0- AM 1-FM 2- DAB */
/**
 * @brief Maximum Number of Stations for AM.
 */
#define MAX_AMRADIO_STATIONS                60
/**
 * @brief Maximum Number of Stations for FM.
 */
#define MAX_FMRADIO_STATIONS                60
/**
 * @brief Maximum Number of Stations for DAB.
 */
#define MAX_DABRADIO_STATIONS               200

/**
 * @brief Maximum Number of Stations.
 */
#define MAX_RADIO_STATIONS                  200 

/**
 * @brief Maximum Number of Preset Stations.
 */
#define MAX_RADIO_PRESET_STATIONS			15

/**
* @brief Maximum Number of Tuners.
*/
#define ETAL_MAX_TUNER						2

typedef int(*Radio_EtalCbGetImage) (void *pvContext, UINT32 requestedByteNum, UINT8* block, UINT32* returnedByteNum, UINT32 *remainingByteNum, BOOL isBootstrap);
typedef void(*Radio_EtalCbPutImage) (void *pvContext, UINT8* block, UINT32 providedByteNum, UINT32 remainingByteNum);

/*-----------------------------------------------------------------------------
                               Type Definitions
-----------------------------------------------------------------------------*/	

/**
 * @brief This enum describes different variant types
 */

typedef enum
{
    RADIO_VARIANT_A1,				/**< Specifies the variant A1   */
    RADIO_VARIANT_A2,				/**< Specifies the variant A2   */
    RADIO_VARIANT_B1,				/**< Specifies the variant B1   */
    RADIO_VARIANT_B2,				/**< Specifies the variant B2   */
	RADIO_VARIANT_C1,				/**< Specifies the variant C1   */
	RADIO_VARIANT_C2,				/**< Specifies the variant C2   */
	RADIO_VARIANT_INVALID			/**< Specifies the invalid variant   */
}VARIANT_TYPE;

/**
 * @brief This enum identifies market types
*/

typedef enum
{
    RADIO_MARKET_WESTERN_EUROPE,		/**< Specifies the Western Europe market   */
    RADIO_MARKET_LATIN_AMERICA,			/**< Specifies the Latin America market   */
    RADIO_MARKET_ASIA_CHINA,			/**< Specifies the Asia and china   */
    RADIO_MARKET_ARABIA,				/**< Specifies the Arabia market   */
    RADIO_MARKET_USA_NORTHAMERICA,		/**< Specifies the North America market   */
    RADIO_MARKET_JAPAN,					/**< Specifies the Japan market   */
    RADIO_MARKET_KOREA,					/**< Specifies the Korea market   */
	RADIO_MARKET_BRAZIL,				/**< Specifies the Brazil market   */
	RADIO_MARKET_SOUTH_AMERICA,			/**< Specifies the South America market   */
    RADIO_MARKET_INVALID				/**< Specifies the Invalid market   */
}MODEL_TYPE;

/**
 * @brief This enum contains supported bands
 */

typedef enum
{
    RADIO_MODE_AM,						/**< Specifies the AM Band   */	
    RADIO_MODE_FM,						/**< Specifies the FM Band   */
    RADIO_MODE_DAB,						/**< Specifies the DAB Band   */
    RADIO_MODE_INVALID					/**< Specifies the Invalid Band   */
}MODE_TYPE;

/**
 * @brief This enum describes directions. Ex: Seek up/down
 */

typedef enum
{
	RADIO_DIRECTION_INVALID = -1,		/**< Specifies the Seek Invalid Direction   */
	RADIO_DIRECTION_DOWN,				/**< Specifies the Seek Down Direction   */
	RADIO_DIRECTION_UP					/**< Specifies the Seek Up Direction   */
}RADIO_DIRECTION;


/**
 * @brief This enum contains Main object IDs. Each Object will have separate HMIIF_IDataObject structure
*/

enum RADIO_DATA_OBJECT_ID
{
    RADIO_DOID_STATION_INFO,				/**< Specifies the Station Information Id*/	
    RADIO_DOID_STATION_LIST_DISPLAY,		/**< Specifies the Station list display object Id   */
    RADIO_DOID_STATION_LIST_DIAG,			/**< Specifies the Station list diag object Id   */
    RADIO_DOID_STATION_INFO_DIAG,			/**< Specifies the Station Info diag object Id   */
    RADIO_DOID_QUALITY_DIAG,				/**< Specifies the Quality diag object Id   */
    RADIO_DOID_MEMORY_LIST,					/**< Specifies the Memory List object Id   */
    RADIO_DOID_AF_LIST,						/**< Specifies the AF List object Id   */
    RADIO_DOID_AF_STATUS,					/**< Specifies the AF status object Id   */
    RADIO_DOID_DABFM_LINK_STATUS,			/**< Specifies the DAB FM Linking status object Id   */
	RADIO_DOID_STATUS,						/**< Specifies the Status object Id   */
	RADIO_DOID_ANNOUNCEMENT,				/**< Specifies the Announcement object Id   */
	RADIO_DOID_COMPONENT_STATUS,			/**< Specifies the Component status object Id   */
	RADIO_DOID_SETTINGS,					/**< Specifies the Settings object Id   */
	RADIO_DOID_ACTIVITY_STATE,				/**< Specifies the activity state object Id   */
	RADIO_DOID_BESTPI_INFO,					/**< Specifies the best PI Info object Id   */
	RADIO_DOID_CLOCKTIME_INFO,				/**< Specifies the Clock Time Info object Id   */
	RADIO_DOID_FIRMWARE_VERSION_INFO,		/**< Specifies the Firmware version for AMFM Tuner & DAB Tuner object Id   */
	RADIO_DOID_STL_SEARCH_DISPLAY,			/**< Specifies the Search Station list display object Id   */
	RADIO_DOID_MULTIPLEX_ENSEMBLE_DISPLAY,	/**< Specifies the Multiplex Ensemble list display object Id   */
	RADIO_DOID_DATA_SERVICE					/**< Specifies the DAB Data Service display object Id   */
};


/**
* @brief This enum contains Sub object IDs for Best PI Info ObjectId 
*/

enum RADIO_DATA_OBJECT_BESTPI_INFO_SUBID 
{
    RADIO_DOSID_BESTPI_FREQUENCY,			/**< Specifies the best PI Station Frequency(UINT32)   */
	RADIO_DOSID_BESTPI_PI,					/**< Specifies the best PI(UINT16) */
	RADIO_DOSID_BESTPI_QUALITY				/**< Specifies the best PI Station Quality(UINT8)   */
};


/**
* @brief This enum contains Sub object IDs for Clock Time Info ObjectId 
*/

enum RADIO_DATA_OBJECT_CLOCKTIME_INFO_SUBID 
{
    RADIO_DOSID_CLOCKTIME_HOUR,					/**< Specifies the Clock Time Hour Information(UINT8)   */
	RADIO_DOSID_CLOCKTIME_MINUTES,				/**< Specifies the Clock Time Minutes Information(UINT8)  */
	RADIO_DOSID_CLOCKTIME_DAY,					/**< Specifies the Clock Time Day Information(UINT8)   */
	RADIO_DOSID_CLOCKTIME_MONTH,				/**< Specifies the Clock Time Month Information(UINT8)   */
	RADIO_DOSID_CLOCKTIME_YEAR					/**< Specifies the Clock Time Year Information(UINT16)   */
};


/**
* @brief This enum contains Sub object IDs for Notifying activity state ObjectId 
*/

enum RADIO_DATA_OBJECT_ACTIVITY_STATE_SUBID 
{
    RADIO_DOSID_ACTIVITY_STATE_BAND,		/**< Specifies the SID for Activity state band(MODE_TYPE)   */
    RADIO_DOSID_ACTIVITY_STATE				/**< Specifies the SID for Activity state(Radio_Activity_Status(Enum))   */
};

/**
* @brief This enum contains Radio Activity Status
*/
typedef enum
{
	RADIO_STATION_NOT_AVAILABLE = 1,						/**<  Specifies the Selected Station Not Available */
	RADIO_FM_AF_PROCESSING,									/**<  Specifies, Active Band FM, FM AF Processing*/
	RADIO_FM_INTERNAL_SCAN_PROCESS,							/**<  Specifies, Active Band FM, Internal Scan*/
	RADIO_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING,				/**<  Specifies, Active Band FM, LearnMem FM AF and DAB AF Processing*/
	RADIO_DAB_AF_PROCESSING,								/**<  Specifies, Active Band DAB, DAB AF Processing*/
	RADIO_DAB_INTERNAL_SCAN_PROCESS,						/**<  Specifies, Active Band DAB, DAB Internal Scan Processing*/
	RADIO_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING,				/**<  Specifies, Active Band DAB, LearnMem DAB AF and FM AF Processing*/
	RADIO_FM_LEARNMEM_AF_PROCESSING,						/**<  Specifies, Active Band FM, LearnMem FM AF Proecessing when D<=>F OFF*/
	RADIO_DAB_LEARNMEM_AF_PROCESSING,						/**<  Specifies, Active Band DAB, LearnMem DAB AF Proecessing when D<=>F OFF*/
	RADIO_SIGNAL_LOSS,										/**<  Specifies, Update Signal Loss*/
	RADIO_DAB_DAB_STARTED,									/**<  Specifies, DAB to DAB Started*/
	RADIO_DAB_DAB_LINKING_DONE,								/**<  Specifies, DAB to DAB Happended*/
	RADIO_LISTENING,										/**<  Specifies for Listening Activity Status*/
	RADIO_IN_SCAN,											/**<  Specifies for Scan Activity Status*/
	RADIO_TUNE_UPDOWN,										/**<  Specifies for Tune up/down Activity Status*/
	RADIO_ANNOUNCEMENT,										/**<  Specifies for Announcement Activity Status*/
	RADIO_DABFMLINKING_DONE,								/**<  Specifies for DAB<=>FM linking Activity Status*/
	RADIO_IMPLICIT_LINKING_DONE,							/**<  Specifies for Implicit linking Activity Status*/
	RADIO_AF_SWITCHING_ESTABLISHED,							/**<  Specifies AF Switching Established*/
	RADIO_DABTUNER_ABNORMAL,								/**<  Specifies DABTuner Abnormal status*/
	RADIO_STATUS_INVALID									/**<  Specifies, for HMI Status Update Invalid*/
}Radio_Activity_Status;

/**
* @brief This enum contains Sub object IDs for Station Info ObjectId 
*/

enum RADIO_DATA_OBJECT_COMMONDATA_SUBID 
{
    RADIO_DOSID_BAND,						/**< Specifies the SID for getting Band information(MODE_TYPE)  */		
    RADIO_DOSID_FREQUENCY,					/**< Specifies the SID for getting Frequency information(UINT32)  */
    RADIO_DOSID_SERVICENAME,				/**< Specifies the SID for getting service Name information(CHAR*)  */
	RADIO_DOSID_RADIOTEXT,					/**< Specifies the SID for getting Radio Text information(CHAR*)  */
	RADIO_DOSID_CHANNELNAME,				/**< Specifies the SID for getting Channel Name information(CHAR*)  */
	RADIO_DOSID_CURRENT_AUDIO_BAND,			/**< Specifies the SID for getting Current Playing Audio Band Information(MODE_TYPE)  */
	RADIO_DOSID_ENSEMBLENAME,				/**< Specifies the SID for getting Ensemble Name information(CHAR*)  */
	RADIO_DOSID_CURRENTSERVICENUMBER,		/**< Specifies the SID for getting currently playing service number in respective ensemble(UINT8)  */
	RADIO_DOSID_TOTALNUMBEROFSERVICE,		/**< Specifies the SID for getting total number of services in ensemble(UINT8) */
	RADIO_DOSID_PROGRAMME_TYPE,				/**< Specifies the SID for getting the Programme Type(CHAR*) */
	RADIO_DOSID_TA,							/**< Specifies the SID for getting the TA information(UINT8) */
	RADIO_DOSID_TP,							/**< Specifies the SID for getting the TP information(UINT8) */
	RADIO_DOSID_PI							/**< Specifies the SID for getting the PI information(UINT8) */
};

/**
* @brief This enum contains Sub object IDs for Announcement Feature Data ObjectId 
*/

enum RADIO_ANNO_DATAOBJECT_SUBID
{
	RADIO_DOSID_ANNO_BAND,					/**< Specifies the SID for getting Announcement Band information(MODE_TYPE)  */
	RADIO_DOSID_ANNO_STATION_NAME,			/**< Specifies the SID for getting Announcement Station name information(CHAR*)  */
	RADIO_DOSID_ANNO_FREQUENCY,				/**< Specifies the SID for getting Announcement Frequency information(UINT32)  */
	RADIO_DOSID_ANNO_STATUS					/**< Specifies the SID for getting Announcement status information(Radio_Announcement_Status(Enum))  */
};

/**
* @brief This enum contains Sub object IDs for Station List Display Data ObjectId 
*/

enum RADIO_DATA_OBJECT_STATIONLIST_DISPLAY_SUBID 
{
	RADIO_DOSID_STATIONLIST_DISPLAY_BAND,					/**< Specifies the SID for getting Station list Band information(MODE_TYPE)  */
    RADIO_DOSID_STATIONLIST_DISPLAY_INDEX,					/**< Specifies the SID for getting Station list index information(UINT8)  */
    RADIO_DOSID_STATIONLIST_DISPLAY_SERVICENAME,			/**< Specifies the SID for getting Station list service name information(CHAR*)  */
	RADIO_DOSID_STATIONLIST_DISPLAY_FREQUENCY,				/**< Specifies the SID for getting Station list Frequency information(UINT32)  */
	RADIO_DOSID_STATIONLIST_DISPLAY_TUNEDSTN_INDEX,			/**< Specifies the SID for getting Station list Tuned Station index information(BOOL)  */
	RADIO_DOSID_STATIONLIST_MATCHED_PRESET_INDEX			/**< Specifies the SID for getting index information if the same station is present in Preset List(BOOL)*/
};

/**
* @brief This enum contains Sub object IDs for Multiplex Ensemble List Display Data ObjectId 
*/

enum RADIO_DATA_OBJECT_MULTIPLEXENSEMBLELIST_DISPLAY_SUBID 
{
    RADIO_DOSID_STATIONLIST_DISPLAY_ENSEMBLEINDEX,			/**< Specifies the SID for getting Multiplex list Ensemble index information(UINT8)  */
    RADIO_DOSID_STATIONLIST_DISPLAY_ENSEMBLENAME,			/**< Specifies the SID for getting Multiplex Ensemble list information(CHAR*)  */
};


/**
* @brief This enum contains Sub object IDs for Station List Display Data ObjectId 
*/

enum RADIO_DATA_OBJECT_STL_SEARCH_DISPLAY_SUBID 
{
	RADIO_DOSID_STLSEARCH_DISPLAY_BAND,						/**< Specifies the SID for getting Station list Band information(MODE_TYPE)  */
    RADIO_DOSID_STLSEARCH_DISPLAY_INDEX,					/**< Specifies the SID for getting Station list index information(UINT8)  */
    RADIO_DOSID_STLSEARCH_DISPLAY_SERVICENAME,				/**< Specifies the SID for getting Station list service name information(CHAR*)  */
	RADIO_DOSID_STLSEARCH_DISPLAY_FREQUENCY,				/**< Specifies the SID for getting Station list Frequency information(UINT32)  */
	RADIO_DOSID_STLSEARCH_DISPLAY_TUNEDSTN_INDEX,			/**< Specifies the SID for getting Station list Tuned Station index information(BOOL)  */
	RADIO_DOSID_STLSEARCH_MATCHED_PRESET_INDEX				/**< Specifies the SID for getting index information if the same station is present in Preset List(BOOL)*/
};

/**
* @brief This enum contains Sub object IDs for Station List Display Data ObjectId 
*/

enum RADIO_DATA_OBJECT_STATIONLIST_SEARCH_DISPLAY_SUBID 
{
	RADIO_DOSID_STATIONLIST_SEARCH_BAND,						/**< Specifies the SID for getting Searching Station list Band information(MODE_TYPE)  */
    RADIO_DOSID_STATIONLIST_SEARCH_INDEX,						/**< Specifies the SID for getting Searching Station list index information(UINT8)  */
    RADIO_DOSID_STATIONLIST_SEARCH_SERVICENAME,					/**< Specifies the SID for getting Searching Station list service name information(CHAR*)  */
	RADIO_DOSID_STATIONLIST_SEARCH_FREQUENCY,					/**< Specifies the SID for getting Searching Station list Frequency information(UINT32)  */
	RADIO_DOSID_STATIONLIST_SEARCH_TUNEDSTN_INDEX,				/**< Specifies the SID for getting Searching Station list Tuned Station index information(BOOL)  */
	RADIO_DOSID_STATIONLIST_SEARCH_MATCHED_MEMORY_INDEX			/**< Specifies the SID for getting Searching index information if the same station is present in Memory List(BOOL)*/
};


/**
* @brief This enum contains Sub object IDs for Station List in Diag mode Data ObjectId 
*/

enum RADIO_DATA_OBJECT_STATIONLIST_DIAG_SUBID
{
    RADIO_DOSID_STATIONLIST_BAND,					/**< Specifies the SID for getting Station list Band diag information(MODE_TYPE)  */
    RADIO_DOSID_STATIONLIST_FREQUENCY,				/**< Specifies the SID for getting Station list Frequency diag information(UINT32)  */
    RADIO_DOSID_STATIONLIST_PI_SID,					/**< Specifies the SID for getting Station list PI SID diag information(UINT32)  */
    RADIO_DOSID_STATIONLIST_SERVICENAME,			/**< Specifies the SID for getting Station list Service name diag information(CHAR*)  */
    RADIO_DOSID_STATIONLIST_ENSEMBLE_ID,			/**< Specifies the SID for getting Station list Ensemble Id diag information(UINT16)  */
    RADIO_DOSID_STATIONLIST_SERVICECOMP_ID,			/**< Specifies the SID for getting Station list Service component Id diag information(UINT16)  */
    RADIO_DOSID_STATIONLIST_SERVICECOMP_NAME		/**< Specifies the SID for getting Station list Service component name diag information(CHAR*)  */
};

/**
* @brief This enum contains Sub object IDs for Memory List Display Data ObjectId 
*/

enum RADIO_DATA_OBJECT_MEMORYLIST_SUBID
{
	RADIO_DOSID_MEMORYLIST_BAND,					/**< Specifies the SID for getting memory list Band information(MODE_TYPE)  */
    RADIO_DOSID_MEMORYLIST_INDEX,					/**< Specifies the SID for getting Station list Index  nformation(UINT8)  */
    RADIO_DOSID_MEMORYLIST_SERVICENAME,				/**< Specifies the SID for getting Station list Service name information(CHAR*)  */
	RADIO_DOSID_MEMORYLIST_FREQUENCY,				/**< Specifies the SID for getting Station list Frequency information(UINT32)  */
	RADIO_DOSID_MEMORYLIST_TUNEDSTN_INDEX,			/**< Specifies the SID for getting Station list Tuned station index information(BOOL)  */
	RADIO_DOSID_MEMORYLIST_CHANNELNAME				/**< Specifies the SID for getting DAB Channel Name information(CHAR*)  */
};


/**
* @brief This enum contains Sub object IDs for Station information Data ObjectId 
*/

enum RADIO_DATA_OBJECT_STNINFO_SUBID
{
	RADIO_DOSID_STNINFO_BAND,						/**< Specifies the SID for getting Station info Band information(MODE_TYPE)  */
    RADIO_DOSID_STNINFO_FREQ,						/**< Specifies the SID for getting Station Info frequency information(UINT32)  */
	RADIO_DOSID_STNINFO_QUALITY,					/**< Specifies the SID for getting Station Info Quality information(UINT8)  */
	RADIO_DOSID_STNINFO_AMFM_PSN,					/**< Specifies the SID for getting Station Info PSN information(CHAR*)  */
    RADIO_DOSID_STNINFO_EID,						/**< Specifies the SID for getting Station Info EID information(UINT16)  */
	RADIO_DOSID_STNINFO_PI,							/**< Specifies the SID for getting Station Info PI information(UINT16)  */
    RADIO_DOSID_STNINFO_SID,						/**< Specifies the SID for getting Station Info SID information(UINT32)  */
    RADIO_DOSID_STNINFO_SCID,						/**< Specifies the SID for getting Station Info Scid information(UINT16)  */
    RADIO_DOSID_STNINFO_DAB_SERVICENAME,			/**< Specifies the SID for getting Service name information(CHAR*)  */
	RADIO_DOSID_STNINFO_TA,							/**< Specifies the SID for getting Traffic Announcement information(UINT8)  */
	RADIO_DOSID_STNINFO_TP							/**< Specifies the SID for getting Traffic Programme information(UINT8)  */
};


/**
* @brief This enum contains Sub object IDs for AM, FM & DAB Quality parameters Data ObjectId 
*/

enum RADIO_DATA_OBJECT_QUALITY_SUBID
{
	RADIO_DOSID_QUALITY_BAND,				/**< Specifies the SID for getting Band information(MODE TYPE)  */
	RADIO_DOSID_QUALITY_RF_FS,				/**< Specifies the SID for getting RF Field strength information(INT)  */
	RADIO_DOSID_QUALITY_BB_FS,				/**< Specifies the SID for getting BB Field strength information(INT)  */
	RADIO_DOSID_QUALITY_OFS,				/**< Specifies the SID for getting Frequency Offset information(UINT8)  */
	RADIO_DOSID_QUALITY_MODULATION_DET,		/**< Specifies the SID for getting Modulation Detector information(UINT8)  */
	RADIO_DOSID_QUALITY_MULTIPATH,			/**< Specifies the SID for getting Multipath information(UINT8)  */
    RADIO_DOSID_QUALITY_USN,				/**< Specifies the SID for getting Noise disturbance information(UINT8)  */
	RADIO_DOSID_QUALITY_ADJCHANNEL,			/**< Specifies the SID for getting Adjacent Channel information(UINT8)  */
	RADIO_DOSID_QUALITY_SNR_LEVEL,			/**< Specifies the SID for getting Signal to Noise Ratio information(INT8)  */
	RADIO_DOSID_QUALITY_COCHANNEL,			/**< Specifies the SID for getting Co-Channel information(UINT8)  */
	RADIO_DOSID_QUALITY_STEREOMONO,			/**< Specifies the SID for getting Stereo Mono information(INT8)  */
	RADIO_DOSID_QUALITY_FICBER,				/**< Specifies the SID for getting FIC Bit error Ratio information(UINT32)  */
	RADIO_DOSID_QUALITY_IS_VALID_FICBER,	/**< Specifies the SID for getting if FIC Bit error Ratio information is valid or not(BOOL)  */
	RADIO_DOSID_QUALITY_MSCBER,				/**< Specifies the SID for getting MSC Bit error Ratio information(UINT32)  */
	RADIO_DOSID_QUALITY_IS_VALID_MSCBER,	/**< Specifies the SID for getting if MSC Bit error Ratio information is valid or not(BOOL)  */
	RADIO_DOSID_QUALITY_DATASCHBER,			/**< Specifies the SID for getting Data Sub Channel Bit error Ratio information(UINT32)  */
	RADIO_DOSID_QUALITY_IS_VALID_DATASCHBER,/**< Specifies the SID for getting if Data Sub Channel Bit error Ratio information is valid or not(BOOL)  */
	RADIO_DOSID_QUALITY_AUDSCHBER,			/**< Specifies the SID for getting Audio Sub Channel Bit error Ratio information(UINT32)  */
	RADIO_DOSID_QUALITY_IS_VALID_AUDSCHBER,	/**< Specifies the SID for getting if Audio Sub Channel Bit error Ratio information is valid or not(BOOL)  */
	RADIO_DOSID_QUALITY_AUDBER_LEVEL,		/**< Specifies the SID for getting Audio Bit error Ratio Level information(UINT8)  */
	RADIO_DOSID_QUALITY_REED_SOLOMON,		/**< Specifies the SID for getting Reed Solomon information(UINT8)  */
	RADIO_DOSID_QUALITY_SYNC_STATUS,		/**< Specifies the SID for getting DAB Station Sync Status(UINT8)  */
	RADIO_DOSID_QUALITY_MUTE_STATUS			/**< Specifies the SID for getting DAB Station Mute Status(BOOL)  */
};

/**
* @brief This enum contains Sub object IDs for AF List Data ObjectId 
*/

enum RADIO_DATA_OBJECT_AFLIST_SUBID
{
	RADIO_DOSID_AFLIST_BAND,				/**< Specifies the SID for getting Band for AF List information(UINT8)  */
    RADIO_DOSID_AFLIST_NUM_AF,				/**< Specifies the SID for getting Number of AF List information(UINT8)  */
    RADIO_DOSID_AFLIST,						/**< Specifies the SID for getting AF List information(Tu16*)  */
	RADIO_DOSID_AF_QUALITY,					/**< Specifies the SID for getting List of AF quality information(Tu8*)  */
    RADIO_DOSID_AFLIST_PI_LIST,				/**< Specifies the SID for getting List of PI's information(Tu16*)  */
	RADIO_DOSID_DAB_NUM_ALT_FREQUENCY,		/**< Specifies the SID for getting List of DAB Alternate Frequency information(Tu8)  */
	RADIO_DOSID_DAB_NUM_ALT_ENSEMBLE,		/**< Specifies the SID for getting List of DAB Alternate Ensemble information(Tu8)  */
	RADIO_DOSID_DAB_NUM_HARDLINK_SID,		/**< Specifies the SID for getting List of DAB Hardlink Sid's information(Tu8)  */
	RADIO_DOSID_DAB_ALT_FREQUENCY,			/**< Specifies the SID for getting List of DAB alternate frequency information(Tu32*)  */
	RADIO_DOSID_DAB_ALT_EID,				/**< Specifies the SID for getting List of DAB alternate EID's information(Tu16*)  */
	RADIO_DOSID_DAB_HARDLINK_SID			/**< Specifies the SID for getting List of DAB Hardlink Sid's information(Tu32*)  */
};

/**
* @brief This enum contains Sub object IDs for AF Switch status Data ObjectId 
*/

enum RADIO_DATA_OBJECT_AF_SWITCH_STATUS_SUBID
{
    RADIO_DOSID_AF_SWITCH_STATUS			/**< Specifies the SID for getting AF Switch status information(Radio_AF_Switch_Status(Enum))  */
};

/**
* @brief This enum contains Sub object IDs for DAB-FM Link status Data ObjectId 
*/

enum RADIO_DATA_OBJECT_DABFM_LINK_STATUS_SUBID
{
    RADIO_DOSID_DABFM_LINK_STATUS			/**< Specifies the SID for getting DAB-FM Link status information(Radio_DABFM_Linking(Enum))  */
};

/**
* @brief This enum contains Sub object IDs for Status for requests made from HMI Data ObjectId 
*/

enum RADIO_DATA_OBJECT_RESULTCODE_STATUS_SUBID
{
    RADIO_DOSID_STATUS					/**< Specifies the SID for getting Status information(Radio_ResultCode(Enum))  */
};

/**
* @brief This enum contains Sub object IDs for Radio Firmware Version ObjectId 
*/

enum RADIO_DATA_OBJECT_RADIO_FIRMWARE_VERSION_SUBID
{
    RADIO_DOSID_AMFMTUNER_FIRMWARE_VERSION,			/**< Specifies the SID for FMTuner Firmware version(UINT8*)  */
	RADIO_DOSID_DABTUNER_HARDWARE_VERSION,			/**< Specifies the SID for DABTuner Hardware version(UINT8*)  */
    RADIO_DOSID_DABTUNER_FIRMWARE_VERSION			/**< Specifies the SID for DABTuner Firmware version(UINT8*)  */
};


/**
* @brief This enum contains Sub object IDs for Radio DAB Data Service ObjectId
*/

enum RADIO_DATA_OBJECT_RADIO_DAB_DATASERVICE_SUBID
{
	RADIO_DOSID_DATASERVICE_HEADER,					/**< Specifies the SID for DAB Data Service Header(Radio_EtalDataServiceType(enum))  */
	RADIO_DOSID_DATASERVICE_PAYLOAD					/**< Specifies the SID for DAB Data Service Payload(UINT32)  */
};


/**
 * @brief This enum contains Radio Component Status
*/
typedef enum
{
	RADIO_COMP_STATUS_NORMAL,					/**< Specifies the Component status as Normal  */
    RADIO_COMP_STATUS_ABNORMAL,					/**< Specifies the Component status as Abnormal  */
    RADIO_COMP_STATUS_INVALID					/**< Specifies the Component status as Invalid  */
}Radio_Component_Status;

/**
 * @brief This enum contains various AMFM Tuner Status
*/
typedef enum
{
	RADIO_AMFMTUNER_NORMAL,							/**< Specifies the AMFM Tuner status as Normal  */
	RADIO_AMFMTUNER_I2CERROR,						/**< Specifies the AMFM Tuner status as I2C Error  */
	RADIO_AMFMTUNER_INTERNAL_RESET,					/**< Specifies the AMFM Tuner status as Internal reset  */
	RADIO_AMFMTUNER_STATUS_INVALID					/**< Specifies the AMFM Tuner status as Invalid  */
}Radio_AMFMTuner_Status;

/**
 * @brief This enum contains DAB Up Notification Status
*/
typedef enum
{
	RADIO_DAB_UP_NOTIFICATION_NOT_RECEIVED,				/**<0- Enum, DAB Up-Notification not received*/ 
    RADIO_DAB_UP_NOTIFICATION_RECEIVED,					/**<1- Enum, DAB Up-Notification Received*/ 
	RADIO_DAB_UP_NOTIFICATION_INVALID					/**<2- Enum, DAB Up-Notification Invalid*/ 
}Radio_DAB_UpNotification_Status;
/**
* @brief This enum contains Sub object IDs for Radio Component Status ObjectId 
*/

enum RADIO_DATA_OBJECT_RADIO_COMPONENT_STATUS_SUBID
{
    RADIO_DOSID_COMPONENT_STATUS_BAND,		/**< Specifies the SID for getting Component status band(MODE_TYPE)  */
	RADIO_DOSID_AMFMTUNER_COMPONENT_STATUS,	/**< Specifies the SID for getting FM Tuner status(Radio_Component_Status(Enum))  */
    RADIO_DOSID_DABTUNER_COMPONENT_STATUS,	/**< Specifies the SID for getting DAB Tuner status(Radio_Component_Status(Enum))  */
	RADIO_DOSID_DAB_UP_NOTIFICATION_STATUS	/**< Specifies the SID for getting DAB Up Notification status(Radio_DAB_UpNotification_Status(Enum))  */
};


/**
* @brief This enum contains Sub object IDs for Radio Switch Setting Status ObjectId 
*/
enum RADIO_DATA_OBJECT_RADIO_SWITCH_SETTING_STATUS
{
	RADIO_DOSID_DABFM_SETTING_STATUS,				/**< Specifies the SID for getting DAB-FM setting status(Radio_Switch_Setting_Status(Enum))  */
	RADIO_DOSID_ANNO_SETTING_STATUS,				/**< Specifies the SID for getting TA Announcement setting status(Radio_Switch_Setting_Status(Enum))  */
	RADIO_DOSID_RDS_FOLLOWUP_SETTING_STATUS,		/**< Specifies the SID for getting RDS setting status(Radio_Switch_Setting_Status(Enum))  */
	RADIO_DOSID_INFO_ANNO_SETTING_STATUS,			/**< Specifies the SID for getting Info Announcement setting status(Radio_Switch_Setting_Status(Enum))  */
	RADIO_DOSID_MULTIPLEX_SWITCH_SETTING_STATUS		/**< Specifies the SID for getting Multiplex switch setting status(Radio_Switch_Setting_Status(Enum))  */
};


/**
* @brief This enum contains Radio module response result codes
*/

typedef enum
{
	RADIO_ADDLISTENER_SUCCESS,						/**< 0 -  Specifies the Radio add listener success  */
	RADIO_ADDLISTENER_FAILURE,						/**< 1 -  Specifies the Radio add listener failure  */
	RADIO_ETALHWCONFIG_SUCCESS,						/**< 2 -  Specifies the Radio ETAL Initialization success  */
	RADIO_ETALHWCONFIG_FAILURE,						/**< 3 -  Specifies the Radio ETAL Initialization failure  */
	RADIO_STARTUP_SUCCESS,							/**< 4 -  Specifies the Radio start up success  */
	RADIO_STARTUP_FAILURE,							/**< 5 -  Specifies the Radio start up failure  */
	RADIO_SHUTDOWN_SUCCESS,							/**< 6 -  Specifies the Radio shut down success  */
	RADIO_SHUTDOWN_FAILURE,							/**< 7 -  Specifies the Radio shut down failure  */
	RADIO_SELECTBAND_SUCCESS,						/**< 8 -  Specifies the Radio select band success  */
	RADIO_SELECTBAND_FAILURE,						/**< 9 -  Specifies the Radio select band failure  */
	RADIO_STNLISTSELECT_REQ_SUCCESS,				/**< 10 - Specifies the Radio select station as success  */
	RADIO_STNLISTSELECT_REQ_FAILURE,				/**< 11 - Specifies the Radio select station as failure  */
	RADIO_SEEK_REQ_SUCCESS,							/**< 12 - Specifies the Radio status as seek success  */
	RADIO_SEEK_NO_SIGNAL,							/**< 13 - Specifies the Radio status as seek no signal  */
	RADIO_SEEK_REQ_FAILURE,							/**< 14 - Specifies the Radio status as seek failure  */
	RADIO_SCAN_STARTED,								/**< 15 - Specifies the Radio status as scan started  */
	RADIO_SCAN_INPROGRESS,							/**< 16 - Specifies the Radio status as scan in progress  */
	RADIO_SCAN_COMPLETE,							/**< 17 - Specifies the Radio status as scan complete  */
	RADIO_DABFM_BLENDING_REQ_SUCCESS,				/**< 18 - Specifies the Radio status as DAB-FM blending success  */
	RADIO_DABFM_BLENDING_REQ_FAILURE,				/**< 19 - Specifies the Radio status as DAB_FM blending failure  */
	RADIO_PRESET_RECALL_SUCCESS,					/**< 20 - Specifies the Radio status as Preset Recall success  */
	RADIO_PRESET_RECALL_FAILURE,					/**< 21 - Specifies the Radio status as Preset Recall failure  */
	RADIO_PRESET_STORE_SUCCESS,						/**< 22 - Specifies the Radio status as Preset Store success  */
	RADIO_PRESET_STORE_FAILURE,						/**< 23 - Specifies the Radio status as Preset Store failure  */
	RADIO_RDS_FOLLOWING_REQ_SUCCESS,				/**< 24 - Specifies the Radio status as RDS Following request success  */
	RADIO_RDS_FOLLOWING_REQ_FAILURE,				/**< 25 - Specifies the Radio status as RDS Following request failure  */
	RADIO_MANUAL_UPDATE_STL_SUCCESS,				/**< 26 - Specifies the Radio status as Manual Update success  */
	RADIO_MANUAL_UPDATE_STL_FAILURE,				/**< 27 - Specifies the Radio status as Manual Update failure  */
	RADIO_TUNE_UP_DOWN_SUCCESS,						/**< 28 - Specifies the Radio status as Tune up down success  */
	RADIO_TUNE_UP_DOWN_FAILURE,						/**< 29 - Specifies the Radio status as Tune up down failure  */
	RADIO_TA_ANNOUNCEMENT_SUCCESS,					/**< 30 - Specifies the Radio status as TA Announcement success  */
	RADIO_TA_ANNOUNCEMENT_FAILURE,					/**< 31 - Specifies the Radio status as TA Announcement failure  */
	RADIO_CANCEL_ANNOUNCEMENT_SUCCESS,				/**< 32 - Specifies the Radio status as Announcement cancel success  */
	RADIO_CANCEL_ANNOUNCEMENT_FAILURE,				/**< 33 - Specifies the Radio status as Announcement cancel failure  */
	RADIO_SRC_ACTIVATE_DEACTIVATE_SUCCESS,			/**< 34 - Specifies the Radio status as SRC Activate/Deactivate Success  */
	RADIO_SRC_ACTIVATE_DEACTIVATE_FAILURE,			/**< 35 - Specifies the Radio status as SRC Activate/Deactivate failure  */
	RADIO_GET_CT_INFO_SUCCESS,						/**< 36 - Specifies the Radio status as Get Clock Time success  */
	RADIO_GET_CT_INFO_FAILURE,						/**< 37 - Specifies the Radio status as Get Clock Time failure  */
	RADIO_CANCEL_MANUAL_UPDATE_STL_SUCCESS,			/**< 38 - Specifies the Radio status as cancel Manual update of STL success  */
	RADIO_CANCEL_MANUAL_UPDATE_STL_FAILURE,			/**< 39 - Specifies the Radio status as cancel Manual update of STL failure  */
	RADIO_INFO_ANNOUNCEMENT_SUCCESS,				/**< 40 - Specifies the Radio status as Info Announcement success  */
	RADIO_INFO_ANNOUNCEMENT_FAILURE,				/**< 41 - Specifies the Radio status as Info Announcement failure  */
	RADIO_POWERON_SUCCESS,							/**< 42 - Specifies the Radio status as Power ON Success  */
	RADIO_POWERON_FAILURE,							/**< 43 - Specifies the Radio status as Power ON Failure  */
	RADIO_POWEROFF_SUCCESS,							/**< 44 - Specifies the Radio status as Power OFF Success  */
	RADIO_POWEROFF_FAILURE,							/**< 45 - Specifies the Radio status as Power OFF Failure  */
	RADIO_FACTORY_RESET_SUCCESS,					/**< 46 - Specifies the Radio Factory Reset success response  */
	RADIO_FACTORY_RESET_FAILURE,					/**< 47 - Specifies the Radio Factory Reset failure response  */
	RADIO_MULTIPLEX_SWITCH_SUCCESS,					/**< 48 - Specifies the Radio status as Multiplex settings success  */
	RADIO_MULTIPLEX_SWITCH_FAILURE,					/**< 49 - Specifies the Radio status as Multiplex settings failure  */
	RADIO_ETALHWDECONFIG_SUCCESS,					/**< 50 - Specifies the Radio ETAL Deinitialization Success  */
	RADIO_ETALHWDECONFIG_FAILURE,					/**< 51 - Specifies the Radio ETAL Deinitialization failure  */
    RADIO_PARAMETER_INVALID,						/**< 52 - Specifies the Radio status as Parameter Invalid  */
	RADIO_FAILURE									/**< 53 - Specifies the Radio Failure  */
}Radio_ResultCode;


/**
 * @brief This enum contains possible values for AF Switching status
*/
typedef enum
{
	RADIO_AF_LIST_AVAILABLE,					/**< Specifies the AF List Available  */
	RADIO_AF_LIST_BECOMES_ZERO,					/**< Specifies the AF List Zero */
	RADIO_AF_LIST_EMPTY,						/**< Specifies the AF List Empty */
	RADIO_AF_LINK_INITIATED,					/**< Specifies the AF Link Initiated */
	RADIO_AF_LINK_ESTABLISHED,					/**< Specifies the AF Link Established */
	RADIO_DAB_LINK_ESTABLISHED,					/**< Specifies the DAB Link Established  */
	RADIO_NO_LINK								/**< Specifies the No Link  */
}Radio_AF_Switch_Status;

/**
 * @brief This enum contains Linking Status values
*/
typedef enum
{
    RADIO_DAB_FM_HARDLINKS_RECEIVED,    		/**< Specifies the DAB-FM Hardlinks received  */
    RADIO_DAB_FM_BEST_PI_RECEIVED,				/**< Specifies the DAB-FM Best PI received  */
    RADIO_FM_IMPLICIT_PI_RECEIVED,    			/**< Specifies the DAB-FM Implicit PI received  */
    RADIO_DAB_FM_LINKING_NOT_AVAILABLE,			/**< Specifies the DAB-FM Link Not available  */
	RADIO_FM_LINKING_CANCELLED,					/**< Specifies the DAB-FM Linking cancelled  */
    RADIO_DAB_FM_BLENDING_SUCCESS,    			/**< Specifies the DAB-FM Blending success  */
    RADIO_DAB_FM_BLENDING_FAILURE,				/**< Specifies the DAB-FM Blending failure  */
    RADIO_IMPLICIT_FM_BLENDING_SUCCESS,    		/**< Specifies the Implicit FM blending success  */
    RADIO_FM_IMPLICIT_BLENDING_FAILURE,			/**< Specifies the Implicit FM blending failure  */
    RADIO_FM_BLENDING_SUSPENDED,				/**< Specifies the FM blending suspended  */
    RADIO_DAB_RESUME_BACK						/**< Specifies the DAB resume back  */
}Radio_DABFM_Linking;

/**
 * @brief This enum contains Eng mode request either ON/OFF values enum
*/
typedef enum
{
	RADIO_ENG_MODE_OFF,							/**< Specifies the ENG Mode OFF  */
	RADIO_ENG_MODE_ON 							/**< Specifies the ENG Mode ON  */
}Radio_Eng_Mode_Request;

/**
 * @brief This enum contains Settings(Switch) related Features Enable/Disable request enum
*/
typedef enum
{
	RADIO_SWITCH_REQUEST_ENABLE,			/**< Specifies the Switch Request enable setting  */
	RADIO_SWITCH_REQUEST_DISABLE,			/**< Specifies the Switch Request disable setting  */
	RADIO_SWITCH_REQUEST_INVALID			/**< Specifies the Switch Request settings invalid  */
}Radio_Switch_Request_Settings;

/**
 * @brief This enum contains Announcement Feature Status
*/
typedef enum
{
	RADIO_ANNOUNCEMENT_START,			/**< Specifies the Announcement start  */
	RADIO_ANNOUNCEMENT_END,				/**< Specifies the Announcement End  */
	RADIO_ANNOUNCEMENT_INVALID			/**< Specifies the Announcement Invalid */
}Radio_Announcement_Status;

/**
 * @brief This enum contains Switch Features Setting Status
*/
typedef enum
{
	RADIO_SWITCH_SETTING_ON,			/**< Specifies the Notify switch swtting status ON  */
	RADIO_SWITCH_SETTING_OFF,			/**< Specifies the Notify switch swtting status OFF  */
	RADIO_SWITCH_SETTING_INVALID		/**< Specifies the Notify switch swtting status Invalid  */
}Radio_Switch_Setting_Status;

/**
 * @brief This enum contains SRC Enabling/disabling requests option
*/
typedef enum
{
	RADIO_SRC_ACTIVATE,							/**< Specifies the Request to SRC Enable  */
	RADIO_SRC_DEACTIVATE, 						/**< Specifies the Request to SRC disable  */
	RADIO_SRC_INVALID							/**< Specifies the Request to SRC Invalid  */
}Radio_SRC_ActivateDeactivate_Request;

/**
 * @brief This enum contains the types of data services that can to be notified to HMI. 
*/
typedef enum
{
	RADIO_DAB_DATASERV_TYPE_NONE,				/**< Specifies data services are not available */
	RADIO_DAB_DATASERV_TYPE_ALL,				/**< Specifies all data services are available */
	RADIO_DAB_DATASERV_TYPE_EPG_RAW,			/**< Specifies the SID for getting EPG_RAW data */
	RADIO_DAB_DATASERV_TYPE_SLS,				/**< Specifies the SID for getting SLS(Slideshow) dataservice */
	RADIO_DAB_DATASERV_TYPE_SLS_XPAD,			/**< Specifies the SID for getting SLS_XPAD data */
	RADIO_DAB_DATASERV_TYPE_TPEG_RAW,			/**< Specifies the SID for getting TPEG_RAW data */
	RADIO_DAB_DATASERV_TYPE_TPEG_SNI,			/**< Specifies the SID for getting TPEG SNI Table data */
	RADIO_DAB_DATASERV_TYPE_SLI,				/**< Specifies the SID for getting Service Linking information data */
	RADIO_DAB_DATASERV_TYPE_EPG_BIN,			/**< Specifies the SID for getting EPG Binary data */
	RADIO_DAB_DATASERV_TYPE_EPG_SRV,			/**< Specifies the SID for getting EPG Service Information data */
	RADIO_DAB_DATASERV_TYPE_EPG_PRG,			/**< Specifies the SID for getting EPG Programme Information data */
	RADIO_DAB_DATASERV_TYPE_EPG_LOGO,			/**< Specifies the SID for getting EPG_LOGO data */
	RADIO_DAB_DATASERV_TYPE_JML,				/**< Specifies the SID for getting Journaline data service */
	RADIO_DAB_DATASERV_TYPE_FIDC,				/**< Specifies the SID for getting FIDC data */
	RADIO_DAB_DATASERV_TYPE_TMC,				/**< Specifies the SID for getting TMC Service data */
	RADIO_DAB_DATASERV_TYPE_DLPLUS,				/**< Specifies the SID for getting DLPlus dataservice */
	RADIO_DAB_DATASERV_TYPE_PSD					/**< Specifies the SID for getting PSD data */
}Radio_EtalDataServiceType;

/*-----------------------------------------------------------------------------
                            Structure  Definitions
-----------------------------------------------------------------------------*/	

typedef struct
{
	Tbool					m_isDisabled;
	Tbool					m_doFlashDump;
	Tbool					m_doFlashProgram;
	Tbool					m_doDownload;
	Radio_EtalCbGetImage	m_cbGetImage;
	void					*m_pvGetImageContext;
	Radio_EtalCbPutImage	m_cbPutImage;
	void					*m_pvPutImageContext;
} Radio_EtalDCOPAttr;

typedef struct
{
	Tbool					m_isDisabled;
	Tbool					m_useXTALalignment;
	UINT32					m_XTALalignment;
	UINT8					m_useCustomParam;
	UINT32					m_CustomParamSize;
	UINT32					*m_CustomParam;
	UINT8					m_useDownloadImage;
	UINT32					m_DownloadImageSize;
	UINT8					*m_DownloadImage;
} Radio_EtalTunerAttr;


typedef struct
{
	Radio_EtalDCOPAttr		m_DCOPAttr;
	Radio_EtalTunerAttr		m_tunerAttr[ETAL_MAX_TUNER];
} Radio_EtalHardwareAttr;


#endif

/*=============================================================================
    end of file
=============================================================================*/
