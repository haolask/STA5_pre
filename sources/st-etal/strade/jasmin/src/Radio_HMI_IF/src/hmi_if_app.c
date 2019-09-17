/*===========================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file hmi_if_app.c																				    *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: HMI IF															     			*
*  Description			: This file contains Api's for Get_id,data get and data set for various parameters	*
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include <string.h>
#include "hmi_if_common.h"
#include "hmi_if_app.h"
#ifndef __IRADIO_API_H__
#include "IRadio.h"
#endif
#include "hmi_if_app_request.h"
#include "hmi_if_app_notify.h"
#include "radio_mngr_app_request.h"
#include "debug_log.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------*/
/*                                           Global variables                                                */
/*-----------------------------------------------------------------------------------------------------------*/

IBaseModule *_listenerArray[MAX_RADIO_LISTENER] = {NULL};
PFN_NOTIFY  _pfnNotifyHdlr = {NULL};
tRadioCommonDataBox _radioCommonData = {RADIO_MODE_FM, RADIO_MODE_FM, 87500, "", "", 3, "", "", 0, 0, "", 0, 0, 0};
tRadioDataBoxStatus _radioStatus;
CHAR Service_Name[RADIO_MAX_SERVICE_NAME_UTF];
CHAR Ensemble_Name[RADIO_MAX_SERVICE_NAME_UTF];
tRadioStationListDataBox_AM _radioStationListData_AM[MAX_AMRADIO_STATIONS];
tRadioStationListDataBox_FM _radioStationListData_FM[MAX_FMRADIO_STATIONS];
tRadioStationListDataBox_DAB _radioStationListData_DAB[MAX_DABRADIO_STATIONS];
tRadioStationListDataBox_Display _radioStationListData_Display[MAX_RADIO_STATIONS];
tRadioEnsembleListDataBox_Display _radioEnsembleListData_Display[RADIO_MAX_ENSEMBLE];
tRadioSTLSearchDataBox_Display _radioSTLSearchData_Display[MAX_RADIO_STATIONS];
tRadioMemoryListInfoDataBox_Display _radioMemoryListData[MAX_RADIO_PRESET_STATIONS];
tRadioAMFMStnInfoDataBox _radioAMFMStnInfoData;
tRadioDABStnInfoDataBox _radioDABStnInfoData;
tRadioAMFMQualityDataBox _radioAMFMQualityData;
tRadioDABQualityDataBox _radioDABQualityData;
tRadioAFSwitchStatusDataBox _radioAFSwitchStatusData;
tRadioDABFMLinkStatusDataBox _radioDABFMLinkStatusData;
tRadioAMFMAFListDataBox _radioAMFMAFListData;
tRadioDABAFListDataBox _radioDABAFListData;
tRadioAnnouncementInfoDataBox _radioAnnouncementInfoData;
tRadioComponentStatusInfoDataBox _radioComponentStatusData;
tRadioSettingsStatusInfoDataBox _radioSettingsStatusData;
tRadioActivityStatusDataBox _radioActivityStatusData;
tRadioBestPIStationDataBox _radioBestPIStationData;
tRadioGetClockTimeInfoDataBox	_radioClockTimeInfoData;
tRadioGetFirmareVersionInfoDataBox _radioFirmwareVersionData;
tRadioDABDataServiceInfoDataBox_Display _radioDABDataServiceRawData;
Tu8 noAMStations;
Tu8 noFMStations;
Tu8 noDABStations;
Tu8 noStationsDisplay;
Tu8 noSearchSTLStationsDisplay;
Tu8 noEnsembleListDisplay;
HMIIF_IDataObject pDataObject;
Tu8 ListenerIdxHMI;
Tu8 ListenerIdxModeMngr;
Tu8 preset_index;

CHAR DLS_RadioText[RADIO_MAX_DLS_RADIO_TEXT_UTF];

/*-----------------------------------------------------------------------------------------------------------*/
/*                                           Structure Instances                                             */
/*-----------------------------------------------------------------------------------------------------------*/

/**
 * @brief Radioinfo Structure instance which provides the Callback function and version informations.
 * Currently this notification function will not be used, since HMI will not notify using callbacks
 */

tRadioInfo _aRadioInfo =
{
    {'R','a','d','i','o','\0'},
	{'V','3','.','8','E','4','\0'},
	{'2','0','1','7','-','0','9','-','1','3','\0'},
    _Radio_Notify
};


/**
 * @brief Structure instance of HMIIF_IDataObject which provides the module informations to the listener
 */
HMIIF_IDataObject _aRadioModuleInfo =
{
    &_aRadioInfo,
    Radio_Default_IDataObject_GetId,
    _Radio_GetInfo
};

/**
 * @brief Structure instance of HMIIF_IDataObject for commonData(RADIO_DOID_STATION_INFO) Object ID
 */
HMIIF_IDataObject _radioDataObject =
{
    &_radioCommonData,
    _RadioCommonData_GetId,					/* This function returns the object Id of current station data object (RADIO_DOID_STATION_INFO) */
    _RadioCommonData_Get					/* This function gives common radio data */
};


/*..........................Structure instance for radio status.................*/

/**
 * @brief Structure instance of HMIIF_IDataObject for statusData(RADIO_DOID_STATUS) Object ID
 */
HMIIF_IDataObject _radioStatusObject =
{
    &_radioStatus,
    _RadioStatusData_GetId,					/* This function returns the object Id of status data object (RADIO_DOID_STATUS) */
    _RadioStatusData_Get					/* This function gives status data */
};



/**
 * @brief Structure instance of IObjectList for AM, FM & DAB Station list information (RADIO_DOID_STATION_LIST) Object ID
 */
IObjectList _radioStationList[MAX_RADIO_MODE] =
{
    {
        &_radioStationListData_AM[0],
        _RadioStationList_GetAMAt,			/* This will return the Corresponding Object of the Station Based on Index provided by the HMI */
        _RadioStationList_GetAMCount		/* This Count Should return the number of stations */
    },
    {
        &_radioStationListData_FM[0],
        _RadioStationList_GetFMAt,			/* This will return the Corresponding Object of the Station Based on Index provided by the HMI */
        _RadioStationList_GetFMCount		/* This Count Should return the number of stations */
    },
    {
        &_radioStationListData_DAB[0],
        _RadioStationList_GetDABAt,			/* This will return the Corresponding Object of the Station Based on Index provided by the HMI */
        _RadioStationList_GetDABCount		/* This Count Should return the number of stations */
    },
};

/**
 * @brief Structure instance of HMIIF_IDataObject for AM Station list information
 */
HMIIF_IDataObject _radioStaionDataObjectAM =
{
    &_radioStationList[0],
    _RadioStationListData_GetId,			/* This function returns the object Id of radio station list (RADIO_DOID_STATION_LIST_DIAG) */
    _RadioStationListData_GetAMInfo			/* This function gives the AM band information */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for FM Station list information
 */
HMIIF_IDataObject _radioStaionDataObjectFM =
{
    &_radioStationList[1],
    _RadioStationListData_GetId,			/* This function returns the object Id of radio station list (RADIO_DOID_STATION_LIST_DIAG) */
    _RadioStationListData_GetFMInfo			/* This function gives the FM band information */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for DAB Station list information
 */
HMIIF_IDataObject _radioStaionDataObjectDAB =
{
    &_radioStationList[2],
    _RadioStationListData_GetId,			/* This function returns the object Id of radio station list (RADIO_DOID_STATION_LIST_DIAG) */
    _RadioStationListData_GetDABInfo		/* This function gives the DAB band information */
};

/**
 * @brief Structure instance of IObjectList for AM, FM & DAB Station list information (RADIO_DOID_STATION_LIST) Object ID
 */
IObjectList _radioStationListDisplay =
{
    &_radioStationListData_Display[0],
    _RadioStationListData_Display_GetAt,    /* This will return the Corresponding Object of the Station Based on Index provided by the HMI */
    _RadioStationListData_Display_GetCount /* This Count Should return the number of stations */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for DAB Station list information
 */
HMIIF_IDataObject _radioStaionDataObjectDisplay =
{
    &_radioStationListDisplay,
    _RadioStationListDataDisplay_GetId,		/* This function returns the object Id of radio station list (RADIO_DOID_STATION_LIST_DISPLAY) */
    _RadioStationListDataDisplay_GetInfo	/* This function gives the band information to be display */
};

/*=======================================================================================================================================
										Instances for station List search feature
========================================================================================================================================*/
/**
 * @brief Structure instance of IObjectList for searching of stl with requested character with (RADIO_DOID_STL_SEARCH_DISPLAY) Object ID
 */
IObjectList _radioSearchSTLDisplay =
{
    &_radioSTLSearchData_Display[0],
    _RadioSTLSearchData_Display_GetAt,		/* This will return the Corresponding Object of the Station Based on Index provided by the HMI */
    _RadioSTLSearchData_Display_GetCount	/* This Count Should return the number of searched/found stations */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for DAB Station list information
 */
HMIIF_IDataObject _radioSearchSTLDataObjectDisplay =
{
    &_radioSearchSTLDisplay,
    _RadioSTLSearchDataDisplay_GetId,		/* This function returns the object Id of radio station list (RADIO_DOID_STL_SEARCH_DISPLAY) */
    _RadioSTLSearchDataDisplay_GetInfo		/* This function gives the band information to be display */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for FM Station info Object ID
 */
HMIIF_IDataObject _radioAMFMStnInfoObject =
{
    &_radioAMFMStnInfoData,					
    _RadioAMFMStnInfoData_GetId,		/* This function returns the object Id of AMFM station information (RADIO_DOID_STATION_INFO_DIAG) */
    _RadioAMFMStnInfoData_Get			/* This function gives AMFM station information */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for DAB Station Info Object ID
 */
HMIIF_IDataObject _radioDABStnInfoObject =
{
    &_radioDABStnInfoData,
    _RadioDABStnInfoData_GetId,			/* This function returns the object Id of AMFM station information data object (RADIO_DOID_STATION_INFO_DIAG) */
    _RadioDABStnInfoData_Get			/* This function gives AMFM station information */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for AMFM Quality info Object ID
 */
HMIIF_IDataObject _radioAMFMQualityObject =
{
    &_radioAMFMQualityData,
    _RadioAMFMQualityData_GetId,		/* This function returns the object Id of AMFM quality data object (RADIO_DOID_QUALITY_DIAG) */
    _RadioAMFMQualityData_Get			/* This function gives AMFM station quality */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for DAB Quality Object ID
 */
HMIIF_IDataObject _radioDABQualityObject =
{
    &_radioDABQualityData,
    _RadioDABQualityData_GetId,			/* This function returns the object Id of DAB quality data object (RADIO_DOID_QUALITY_DIAG) */
    _RadioDABQualityData_Get			/* This function gives DAB station quality */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for AF switch status Object ID
 */
HMIIF_IDataObject _radioAFSwitchStatusObject =
{
	&_radioAFSwitchStatusData,			
	_RadioAFSwitchStatus_GetId,			/* This function returns the object Id of AF switch status data object (RADIO_DOID_AF_STATUS) */
	_RadioAFSwitchStatus_Get			/* This function gives AF switch status */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for DAB-FM linking status Object ID
 */
HMIIF_IDataObject _radioDABFMLinkStatusObject = 
{
	&_radioDABFMLinkStatusData,		
	_RadioDABFMlinkstatus_GetId,		/* This function returns the object Id of DAB-FM linking data object (RADIO_DOID_DABFM_LINK_STATUS) */
	_RadioDABFMlinkstatus_Get			/* This function gives DAB-FM linking status */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for AF List Object ID
 */
HMIIF_IDataObject _radioAMFMAFListObject = 
{
	&_radioAMFMAFListData,
	_RadioAMFMAFlistData_GetId,				/* This function returns the object Id of AF list data object (RADIO_DOID_AF_LIST) */
	_RadioAMFMAFlistData_Get				/* This function gives AF list */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for AF List Object ID
 */
HMIIF_IDataObject _radioDABAFListObject =
{
	&_radioDABAFListData,
	_RadioDABAFlistData_GetId,				/* This function returns the object Id of AF list data object (RADIO_DOID_AF_LIST) */
	_RadioDABAFlistData_Get					/* This function gives AF list */
};
/*....................Memory Station List related object instances..................................*/

/**
 * @brief Structure instance of IObjectList for AM, FM & DAB Station list information (RADIO_DOID_STATION_LIST) Object ID
 */

IObjectList _radioMemoryListDisplay =
{
    &_radioMemoryListData[0],
    _RadioMemoryListData_Display_GetAt,		/* This will return the Corresponding Object of the Station Based on Index provided by the HMI */
    _RadioMemoryListData_Display_GetCount	/* This Count Should return the number of stations */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for preset Station list information
 */
HMIIF_IDataObject _radioMemoryDataObjectDisplay =
{
    &_radioMemoryListDisplay,
    _RadioMemoryListDataDisplay_GetId,		/* This function returns the object Id of radio station list (RADIO_DOID_STATION_LIST_DISPLAY) */
    _RadioMemoryListDataDisplay_GetInfo		/* This function gives the band information to be display */
};

/*....................DAB Data Service related object instances..................................*/
/**
* @brief Structure instance of IObjectList for DAB Data Service information (RADIO_DOID_DATA_SERVICE) Object ID
*/


IObjectList _radioDABDataServiceDisplay =
{
	&_radioDABDataServiceRawData,
	_RadioDABDataService_Display_GetAt,		/* This will return the Corresponding Object of the DAB Data Service Raw data on Index provided by the HMI */
	_RadioDABDataService_Display_GetSize	/* This Count Should return the size of DAB Data Service payload */
};

/**
* @brief Structure instance of HMIIF_IDataObject for DAB Data Service information
*/
HMIIF_IDataObject _radioDABDataServiceRawDataObject =
{
	&_radioDABDataServiceDisplay,
	_RadioDABDataServiceDisplay_GetId,			/* This function returns the object Id of DAB Data Service (RADIO_DOID_DATA_SERVICE) */
	_RadioDABDataServiceDisplay_GetInfo			/* This function gives the DAB Data Service Header Information */
};


/*....................Announcement related object instances..................................*/
/**
 * @brief Structure instance of HMIIF_IDataObject for announcement related notification.
 */
HMIIF_IDataObject	_radioAnnouncementObject =
{
	&_radioAnnouncementInfoData,
	_RadioAnnouncementData_GetId,
	_RadioAnnouncementData_Get
};


/**
 * @brief Structure instance of HMIIF_IDataObject for announcement related notification.
 */
HMIIF_IDataObject _radioComponentStatusObject =
{
	&_radioComponentStatusData,
	_RadioComponentStatusData_GetId,
	_RadioComponentStatusData_Get
};

/**
 * @brief Structure instance of HMIIF_IDataObject for Radio settings status related notification.
 */
HMIIF_IDataObject _radioSettingsStatusObject =
{
	&_radioSettingsStatusData,
	_RadioSettingsStatusData_GetId,
	_RadioSettingsStatusData_Get
};

/**
 * @brief Structure instance of HMIIF_IDataObject for Notify activity status .
 */
HMIIF_IDataObject _radioActivityStatusObject =
{
	&_radioActivityStatusData,
	_RadioActivityStatusData_GetId,
	_RadioActivityStatusData_Get
};

/**
 * @brief Structure instance of HMIIF_IDataObject for Notifying Best PI station.
 */
HMIIF_IDataObject _radioBestPIStationObject =
{
	&_radioBestPIStationData,
	_RadioBestPIStationData_GetId,
	_RadioBestPIStationData_Get
};

/**
 * @brief Structure instance of HMIIF_IDataObject for Notifying Clock Time Info.
 */
HMIIF_IDataObject _radioClockTimeInfoObject =
{
	&_radioClockTimeInfoData,
	_RadioClockTimeInfoData_GetId,
	_RadioClockTimeInfoData_Get
};

/**
* @brief Structure instance of HMIIF_IDataObject for Notifying Firmware version for AMFM Tuner & DAB Tuner.
*/
HMIIF_IDataObject _radioFirmwareVersionInfoObject =
{
	&_radioFirmwareVersionData,
	_RadioFirmwareVersionInfoData_GetId,
	_RadioFirmwareVersionInfoData_Get
};

/*=======================================================================================================================================
										Instances for Ensemble List structure instances
========================================================================================================================================*/
/**
 * @brief Structure instance of IObjectList for Ensemble list if multiple switch setting is ON (RADIO_DOID_MULTIPLEX_ENSEMBLE_DISPLAY) Object ID
 */
IObjectList _radioEnsembleListDisplay =
{
    &_radioEnsembleListData_Display[0],
    _RadioEnsembleListData_Display_GetAt,		/* This will return the Corresponding Object to fetch the ensemble list data */
    _RadioEnsembleListData_Display_GetCount		/* This Count Should return the number of numbles found */
};

/**
 * @brief Structure instance of HMIIF_IDataObject for DAB Station list information
 */
HMIIF_IDataObject _radioEnsembleListDataObjectDisplay =
{
    &_radioEnsembleListDisplay,
    _RadioEnsembleListDataDisplay_GetId,		/* This function returns the object Id of radio station list (RADIO_DOID_MULTIPLEX_ENSEMBLE_DISPLAY) */
    _RadioEnsembleListDataDisplay_GetInfo		/* This function gives the index & Ensemble name information to be display */
};


/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/



/*===========================================================================*/
/* BOOL _Radio_GetModuleInfo															*/
/*===========================================================================*/
BOOL _Radio_GetModuleInfo(HMIIF_IDataObject** ppInfo)
{
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Radio Get Module Info Request");
    *ppInfo = &_aRadioModuleInfo;
    return TRUE;
}

#if 0
/*===========================================================================*/
/* BOOL _Radio_AddListener																*/
/*===========================================================================*/
BOOL _Radio_AddListener(IBaseModule* pListener)
{
    HMIIF_IDataObject *pDataObject1 = NULL;
    DWORD dwData = 0;
    Tu8 listenerCount = 0;
	Tu8 ret = FALSE;

	if(pListener != NULL)
	{
		for(listenerCount = 0; listenerCount < MAX_RADIO_LISTENER; listenerCount++)
		{
			if(_listenerArray[listenerCount] == NULL)
			{
				_listenerArray[listenerCount] = pListener;
				break;
			}
		}
		if((listenerCount < MAX_RADIO_LISTENER) && (pListener->GetModuleInfo(&pDataObject1)))
		{
			if(pDataObject1)
			{
				if(pDataObject1->Get(pDataObject1->thiss, MODULE_INFO_DOSID_NAME, &dwData))
				{
					if(strncmp((CHAR*)dwData, "HMI", 3) == 0)
					{
						ListenerIdxHMI = listenerCount;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]HMI added as Listener to Radio");
					}
					else if(strncmp((CHAR*)dwData, "System",6 ) == 0)
					{
						ListenerIdxModeMngr = listenerCount;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]System added as Listener to Radio");
					}
				}
				if(pDataObject1->Get(pDataObject1->thiss, MODULE_INFO_DOSID_NOTIFY, &dwData))
				{
					_pfnNotifyHdlr[listenerCount] = (PFN_NOTIFY)dwData;
					Radio_Response_AddListener(listenerCount, RADIO_ADDLISTENER_SUCCESS);
					ret = TRUE;
				}
			}
		}
	}
	return (BOOL)(ret);
}
#endif
/*===========================================================================*/
/* BOOL _Radio_ModuleInit																		*/
/*===========================================================================*/
BOOL _Radio_ModuleInit()
{
    return TRUE;
}

/*===========================================================================*/
/* BOOL _Radio_ModuleRun																		*/
/*===========================================================================*/
BOOL _Radio_ModuleRun(HMIIF_IDataObject* pParameter)
{
	UNUSED(pParameter);
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Mode Requested");
	Radio_Mngr_App_Request_SelectBand((Te_Radio_Mngr_App_Band)RADIO_MNGR_APP_RADIO_MODE);
	return TRUE;
}

/*===========================================================================*/
/* BOOL _Radio_ModuleStop																		*/
/*===========================================================================*/
BOOL _Radio_ModuleStop(HMIIF_IDataObject* pParameter)
{
	UNUSED(pParameter);
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Non Radio Mode Requested");
	Radio_Mngr_App_Request_SelectBand((Te_Radio_Mngr_App_Band)RADIO_MNGR_APP_NON_RADIO_MODE);
	return TRUE;
}

/*===========================================================================*/
/* void _Radio_Notify																	*/
/*===========================================================================*/
void _Radio_Notify(HMIIF_IDataObject* pData)
{
	UNUSED(pData);
}

/*===========================================================================*/
/* BOOL _Radio_GetInfo																	*/
/*===========================================================================*/
BOOL _Radio_GetInfo(void*thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;

    switch(nSubId)
    {
        case MODULE_INFO_DOSID_NAME:
        {
            *pdwData = (DWORD)_aRadioInfo.name;
            break;
        }

        case MODULE_INFO_DOSID_VERSION:
        {
            *pdwData = (DWORD)((tRadioInfo*)thiss)->version; //_aRadioInfo.version;
            break;
        }

        case MODULE_INFO_DOSID_DATE:
        {
            *pdwData = (DWORD)((tRadioInfo*)thiss)->date;	//_aRadioInfo.date;
            break;
        }

        case MODULE_INFO_DOSID_NOTIFY:
        {
            *pdwData = (DWORD)((tRadioInfo*)thiss)->pfnNotify;	//_aRadioInfo.pfnNotify;
            break;
        }

        default:
        {
            bRes = FALSE;
            break;
        }
    }

    return bRes;
}

/*===========================================================================*/
/* BOOL _Radio_SetInfo																	*/
/*===========================================================================*/
BOOL _Radio_SetInfo(void*thiss, INT nSubId, DWORD dwData)
{
	UNUSED(thiss);
	UNUSED(nSubId);
	UNUSED(dwData);
    return FALSE;
}

/*===========================================================================*/
/* int _RadioCommonData_GetId															*/
/*===========================================================================*/
INT _RadioCommonData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_STATION_INFO;
}

/*===========================================================================*/
/* BOOL _RadioCommonData_Get															*/
/*===========================================================================*/
BOOL _RadioCommonData_Get(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_BAND:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->nBand;
            break;
        }

		
        case RADIO_DOSID_FREQUENCY:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->Frequency;
            break;
        }

        case RADIO_DOSID_SERVICENAME:
        {
			memset(Service_Name, 0, sizeof(Service_Name));
			if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioCommonDataBox*)thiss)->ServiceCompName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioCommonDataBox*)thiss)->ServiceCompName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, (const void*)((tRadioCommonDataBox*)thiss)->ServiceCompName, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else
			{
				/*Do Nothing*/
			}

            *pdwData =  (DWORD)Service_Name;
            break;
        }

		case RADIO_DOSID_RADIOTEXT:
		{
			memset(DLS_RadioText, 0, sizeof(DLS_RadioText));
			if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioCommonDataBox*)thiss)->DLS_Radio_Text), (Tu8*)DLS_RadioText, RADIO_MAX_DLS_DATA_SIZE, sizeof(DLS_RadioText));
			}
			else if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioCommonDataBox*)thiss)->DLS_Radio_Text), (Tu8*)DLS_RadioText, RADIO_MAX_DLS_DATA_SIZE);
			}
			else if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(DLS_RadioText, ((tRadioCommonDataBox*)thiss)->DLS_Radio_Text, RADIO_MAX_DLS_DATA_SIZE);
			}
			else
			{
				/* Do nothing For MISRA C */
			}

			*pdwData =  (DWORD)DLS_RadioText;
            break;
		}
		
		case RADIO_DOSID_CHANNELNAME:
		{
			*pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->Channel_Name;
            break;
		}
		
		case RADIO_DOSID_ENSEMBLENAME:
		{
			memset(Ensemble_Name, 0, RADIO_MAX_SERVICE_NAME_UTF);
			if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioCommonDataBox*)thiss)->Ensemble_Label), (Tu8*)Ensemble_Name, RADIO_MAX_ENSEMBLELABEL_LENGTH, sizeof(Ensemble_Name));
			}
			else if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioCommonDataBox*)thiss)->Ensemble_Label), (Tu8*)Ensemble_Name, RADIO_MAX_ENSEMBLELABEL_LENGTH);
			}
			else if(((tRadioCommonDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Ensemble_Name, ((tRadioCommonDataBox*)thiss)->Ensemble_Label, RADIO_MAX_ENSEMBLELABEL_LENGTH);
			}
			else
			{
				/* Do nothing For MISRA C */
			}
			
			*pdwData = (DWORD) Ensemble_Name;
            break;
        }
		
		case RADIO_DOSID_CURRENT_AUDIO_BAND:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->Aud_Band;
            break;
        }
		
		case RADIO_DOSID_CURRENTSERVICENUMBER:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->DAB_Current_Service;
            break;
        }
		
		case RADIO_DOSID_TOTALNUMBEROFSERVICE:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->DAB_Total_Num_Services_In_Ensemble;
            break;
		}

		case RADIO_DOSID_PROGRAMME_TYPE:
		{
			*pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->Programme_Type;
            break;
		}

		case RADIO_DOSID_TA:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->TA;
            break;
		}

		case RADIO_DOSID_TP:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->TP;
            break;
		}

		case RADIO_DOSID_PI:
        {
            *pdwData =  (DWORD) ((tRadioCommonDataBox*)thiss)->PI;
            break;
		}

		default:
            bRes = FALSE;
            break;
    }
    return bRes;
}


/*===========================================================================*/
/* int _RadioStatusData_GetId															*/
/*===========================================================================*/
INT _RadioStatusData_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_STATUS;
}

/*===========================================================================*/
/* BOOL _RadioCommonData_Get															*/
/*===========================================================================*/
BOOL _RadioStatusData_Get(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_STATUS:
        {
            *pdwData =  (DWORD) ((tRadioDataBoxStatus*)thiss)->nStatus;
            break;
        }
       
		default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

/*===========================================================================*/
/* BOOL _RadioStatusData_Set															*/
/*===========================================================================*/
BOOL _RadioStatusData_Set(void*  thiss, INT nSubId, DWORD dwData)
{
	UNUSED(thiss);
	UNUSED(nSubId);
	UNUSED(dwData);
    return FALSE;
}


/*===========================================================================*/
/* INT _RadioStationListData_GetId														*/
/*===========================================================================*/
INT _RadioStationListData_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_STATION_LIST_DIAG;
}

/*===========================================================================*/
/* BOOL _RadioStationListData_GetAMInfo													*/
/*===========================================================================*/
BOOL _RadioStationListData_GetAMInfo(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_STATIONLIST_BAND:
        {
            *pdwData =  (DWORD)RADIO_MODE_AM;
            break;
        }

        case RADIO_DOSID_STATIONLIST_FREQUENCY:
        {
            *pdwData = (DWORD)((tRadioStationListDataBox_AM*)thiss)->Frequency;
            break;
        }

        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*===========================================================================*/
/* BOOL _RadioStationListData_GetFMInfo													*/
/*===========================================================================*/
BOOL _RadioStationListData_GetFMInfo(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_STATIONLIST_BAND:
        {
            *pdwData =  (DWORD)RADIO_MODE_FM;
            break;
        }

        case RADIO_DOSID_STATIONLIST_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_FM*)thiss)->Frequency;
            break;
        }

        case RADIO_DOSID_STATIONLIST_PI_SID:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_FM*)thiss)->PI;
            break;
        }

        case RADIO_DOSID_STATIONLIST_SERVICENAME:
        {
			memset(Service_Name, 0, sizeof(Service_Name));
			if(((tRadioStationListDataBox_FM*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioStationListDataBox_FM*)thiss)->PServiceName), (Tu8*)Service_Name, RADIO_MAX_CHAN_NAME_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioStationListDataBox_FM*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioStationListDataBox_FM*)thiss)->PServiceName), (Tu8*)Service_Name, RADIO_MAX_CHAN_NAME_SIZE);
			}
			else if(((tRadioStationListDataBox_FM*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, ((tRadioStationListDataBox_FM*)thiss)->PServiceName, RADIO_MAX_CHAN_NAME_SIZE);
			}
			else
			{
				/* Do nothing For MISRA C */
			}
			
           *pdwData = (DWORD)Service_Name;
            break;
        }


        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*===========================================================================*/
/* BOOL _RadioStationListData_GetDABInfo												*/
/*===========================================================================*/
BOOL _RadioStationListData_GetDABInfo(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_STATIONLIST_BAND:
        {
            *pdwData =  (DWORD)RADIO_MODE_DAB;
            break;
        }

        case RADIO_DOSID_STATIONLIST_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_DAB*)thiss)->Frequency;
            break;
        }

        case RADIO_DOSID_STATIONLIST_ENSEMBLE_ID:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_DAB*)thiss)->uEnsembleID;
            break;
        }

        case RADIO_DOSID_STATIONLIST_PI_SID:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_DAB*)thiss)->ServiceID;
            break;
        }

        case RADIO_DOSID_STATIONLIST_SERVICECOMP_ID:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_DAB*)thiss)->ServiceCompID;
            break;
        }

        case RADIO_DOSID_STATIONLIST_SERVICECOMP_NAME:
        {
			memset(Service_Name, 0, sizeof(Service_Name));
			if(((tRadioStationListDataBox_DAB*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioStationListDataBox_DAB*)thiss)->Service_ComponentName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioStationListDataBox_DAB*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioStationListDataBox_DAB*)thiss)->Service_ComponentName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else if(((tRadioStationListDataBox_DAB*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, ((tRadioStationListDataBox_DAB*)thiss)->Service_ComponentName, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else
			{
				/* Do nothing For MISRA C */
			}

            *pdwData = (DWORD)Service_Name;
            break;
        }

        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*===========================================================================*/
/* int _RadioStationList_GetAMCount														*/
/*===========================================================================*/
INT _RadioStationList_GetAMCount(void *thiss)
{
	UNUSED(thiss);
    return (INT)(noAMStations);
}

/*===========================================================================*/
/* int _RadioStationList_GetFMCount														*/
/*===========================================================================*/
INT _RadioStationList_GetFMCount(void *thiss)
{
	UNUSED(thiss);
    return (INT)(noFMStations);
}

/*===========================================================================*/
/* int _RadioStationList_GetDABCount													*/
/*===========================================================================*/
INT _RadioStationList_GetDABCount(void *thiss)
{
	UNUSED(thiss);
    return (INT)(noDABStations);
}

/*===========================================================================*/
/* HMIIF_IDataObject _RadioStationList_GetAMAt												*/
/*===========================================================================*/
HMIIF_IDataObject* _RadioStationList_GetAMAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
    if((nIndex >= 0) && (nIndex < MAX_AMRADIO_STATIONS))
    {
        pDataObject.thiss = &_radioStationListData_AM[nIndex];
        pDataObject.GetId = _RadioStationListData_GetId;
        pDataObject.Get = _RadioStationListData_GetAMInfo;
    }
    return &pDataObject;
}

/*===========================================================================*/
/* HMIIF_IDataObject _RadioStationList_GetFMAt												*/
/*===========================================================================*/
HMIIF_IDataObject* _RadioStationList_GetFMAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
    if((nIndex >= 0) && (nIndex < MAX_FMRADIO_STATIONS))
    {
        pDataObject.thiss = &_radioStationListData_FM[nIndex];
        pDataObject.GetId = _RadioStationListData_GetId;
        pDataObject.Get = _RadioStationListData_GetFMInfo;
    }
    return &pDataObject;
}

/*===========================================================================*/
/* HMIIF_IDataObject _RadioStationList_GetDABAt												*/
/*===========================================================================*/

HMIIF_IDataObject* _RadioStationList_GetDABAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
    if((nIndex >= 0) && (nIndex < MAX_DABRADIO_STATIONS))
    {
        pDataObject.thiss = &_radioStationListData_DAB[nIndex];
        pDataObject.GetId = _RadioStationListData_GetId;
        pDataObject.Get = _RadioStationListData_GetDABInfo;
    }
    return &pDataObject;
}

/*===========================================================================*/
/* int _RadioStationListData_Display_GetCount													*/
/*===========================================================================*/
INT _RadioStationListData_Display_GetCount(void *thiss)
{
	UNUSED(thiss);
    return (INT)(noStationsDisplay);
}

/*===========================================================================*/
/* HMIIF_IDataObject _RadioStationListData_Display_GetAt												*/
/*===========================================================================*/

HMIIF_IDataObject* _RadioStationListData_Display_GetAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
    if((nIndex >= 0) && (nIndex < MAX_RADIO_STATIONS))
    {
        pDataObject.thiss = &_radioStationListData_Display[nIndex];
        pDataObject.GetId = _RadioStationListDataDisplay_GetId;
        pDataObject.Get = _RadioStationListDataDisplay_GetInfo;
    }
    return &pDataObject;
}

/*===========================================================================*/
/* int _RadioStationListDataDisplay_GetId														*/
/*===========================================================================*/
INT _RadioStationListDataDisplay_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_STATION_LIST_DISPLAY;
}

/*===========================================================================*/
/* int _RadioStationListDataDisplay_GetInfo														*/
/*===========================================================================*/
BOOL _RadioStationListDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData)
{
	BOOL bRes = TRUE;
	
    switch(nSubId)
    {
        case RADIO_DOSID_STATIONLIST_DISPLAY_BAND:
        {
            *pdwData =  (DWORD) ((tRadioStationListDataBox_Display*)thiss)->nBand;
            break;
        }

			case RADIO_DOSID_STATIONLIST_DISPLAY_INDEX:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_Display*)thiss)->Index;
            break;
        }

		case RADIO_DOSID_STATIONLIST_DISPLAY_SERVICENAME:
        {
			memset(Service_Name, 0, RADIO_MAX_SERVICE_NAME_UTF);
			if(((tRadioStationListDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioStationListDataBox_Display*)thiss)->ServiceName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioStationListDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioStationListDataBox_Display*)thiss)->ServiceName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else if(((tRadioStationListDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, ((tRadioStationListDataBox_Display*)thiss)->ServiceName, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else
			{
				/* Do nothing For MISRA C */
			}
			
			*pdwData = (DWORD) Service_Name;
            break;
        }

        case RADIO_DOSID_STATIONLIST_DISPLAY_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_Display*)thiss)->Frequency;
            break;
        }
		
		case RADIO_DOSID_STATIONLIST_DISPLAY_TUNEDSTN_INDEX:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_Display*)thiss)->Matched_Stn_Index_Flag;
            break;
        }
		case RADIO_DOSID_STATIONLIST_MATCHED_PRESET_INDEX:
        {
            *pdwData = (DWORD) ((tRadioStationListDataBox_Display*)thiss)->MatchedPresetInSTL_index_Flag;
            break;
        }

        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*====================================================================================================
		Functions for Search station list with requested char feature
=====================================================================================================*/

/*===========================================================================*/
/* int _RadioSTLSearchData_Display_GetCount													*/
/*===========================================================================*/
INT _RadioSTLSearchData_Display_GetCount(void *thiss)
{
	UNUSED(thiss);
    return (INT)(noSearchSTLStationsDisplay);
}

/*===========================================================================*/
/* HMIIF_IDataObject _RadioSTLSearchData_Display_GetAt												*/
/*===========================================================================*/

HMIIF_IDataObject* _RadioSTLSearchData_Display_GetAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
    if((nIndex >= 0) && (nIndex < MAX_RADIO_STATIONS))
    {
        pDataObject.thiss = &_radioSTLSearchData_Display[nIndex];
        pDataObject.GetId = _RadioSTLSearchDataDisplay_GetId;
        pDataObject.Get = _RadioSTLSearchDataDisplay_GetInfo;
    }
    return &pDataObject;
}

/*===========================================================================*/
/* int _RadioSTLSearchDataDisplay_GetId														*/
/*===========================================================================*/
INT _RadioSTLSearchDataDisplay_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_STL_SEARCH_DISPLAY;
}

/*===========================================================================*/
/* int _RadioSTLSearchDataDisplay_GetInfo														*/
/*===========================================================================*/
BOOL _RadioSTLSearchDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData)
{
	BOOL bRes = TRUE;
	UNUSED(thiss);

    switch(nSubId)
    {
        case RADIO_DOSID_STLSEARCH_DISPLAY_BAND:
        {
            *pdwData =  (DWORD) ((tRadioSTLSearchDataBox_Display*)thiss)->nBand;
            break;
        }

		case RADIO_DOSID_STLSEARCH_DISPLAY_INDEX:
        {
            *pdwData = (DWORD) ((tRadioSTLSearchDataBox_Display*)thiss)->Index;
            break;
        }

		case RADIO_DOSID_STLSEARCH_DISPLAY_SERVICENAME:
        {
			memset(Service_Name, 0, RADIO_MAX_SERVICE_NAME_UTF);
			if(((tRadioSTLSearchDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioSTLSearchDataBox_Display*)thiss)->ServiceName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioSTLSearchDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioSTLSearchDataBox_Display*)thiss)->ServiceName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else if(((tRadioSTLSearchDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, ((tRadioSTLSearchDataBox_Display*)thiss)->ServiceName, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else
			{
				/* Do nothing For MISRA C */
			}
			
			*pdwData = (DWORD) Service_Name;
            break;
        }

        case RADIO_DOSID_STLSEARCH_DISPLAY_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioSTLSearchDataBox_Display*)thiss)->Frequency;
            break;
        }
		
		case RADIO_DOSID_STLSEARCH_DISPLAY_TUNEDSTN_INDEX:
        {
            *pdwData = (DWORD) ((tRadioSTLSearchDataBox_Display*)thiss)->Matched_Search_Stn_Index_Flag;
            break;
        }
		case RADIO_DOSID_STLSEARCH_MATCHED_PRESET_INDEX:
        {
            *pdwData = (DWORD) ((tRadioSTLSearchDataBox_Display*)thiss)->MatchedPresetInSearchSTL_index_Flag;
            break;
        }

        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*====================================================================================================
		Functions for Searche station list with requested char feature Ended
=====================================================================================================*/


/*====================================================================================================
		Functions for Displaying Ensemble List (When multiple switch is ON) started
=====================================================================================================*/

/*===========================================================================*/
/* int _RadioEnsembleListData_Display_GetCount													*/
/*===========================================================================*/
INT _RadioEnsembleListData_Display_GetCount(void *thiss)
{
	UNUSED(thiss);
    return (INT)(noEnsembleListDisplay);
}

/*===========================================================================*/
/* HMIIF_IDataObject _RadioEnsembleListData_Display_GetAt												*/
/*===========================================================================*/

HMIIF_IDataObject* _RadioEnsembleListData_Display_GetAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
    if((nIndex >= 0) && (nIndex < RADIO_MAX_ENSEMBLE))
    {
        pDataObject.thiss = &_radioEnsembleListData_Display[nIndex];
        pDataObject.GetId = _RadioEnsembleListDataDisplay_GetId;
        pDataObject.Get = _RadioEnsembleListDataDisplay_GetInfo;
    }
    return &pDataObject;
}

/*===========================================================================*/
/* int _RadioEnsembleListDataDisplay_GetId														*/
/*===========================================================================*/
INT _RadioEnsembleListDataDisplay_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_MULTIPLEX_ENSEMBLE_DISPLAY;
}

/*===========================================================================*/
/* int _RadioEnsembleListDataDisplay_GetInfo														*/
/*===========================================================================*/
BOOL _RadioEnsembleListDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData)
{
	BOOL bRes = TRUE;
	
    switch(nSubId)
    {
		case RADIO_DOSID_STATIONLIST_DISPLAY_ENSEMBLEINDEX:
        {
            *pdwData = (DWORD) ((tRadioEnsembleListDataBox_Display*)thiss)->Index;
            break;
        }

		case RADIO_DOSID_STATIONLIST_DISPLAY_ENSEMBLENAME:
        {
			memset(Ensemble_Name, 0, RADIO_MAX_SERVICE_NAME_UTF);
			if(((tRadioEnsembleListDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioEnsembleListDataBox_Display*)thiss)->EnsembleName), (Tu8*)Ensemble_Name, RADIO_MAX_ENSEMBLELABEL_LENGTH, sizeof(Ensemble_Name));
			}
			else if(((tRadioEnsembleListDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioEnsembleListDataBox_Display*)thiss)->EnsembleName), (Tu8*)Ensemble_Name, RADIO_MAX_ENSEMBLELABEL_LENGTH);
			}
			else if(((tRadioEnsembleListDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Ensemble_Name, ((tRadioEnsembleListDataBox_Display*)thiss)->EnsembleName, RADIO_MAX_ENSEMBLELABEL_LENGTH);
			}
			else
			{
				/* Do nothing For MISRA C */
			}
			
			*pdwData = (DWORD) Ensemble_Name;
            break;
        }

        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*====================================================================================================
		Functions for Displaying Ensemble List (When multiple switch is ON) Ended
=====================================================================================================*/

/*===========================================================================*/
/* int _RadioFMStnInfoData_GetId														*/
/*===========================================================================*/
INT _RadioAMFMStnInfoData_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_STATION_INFO_DIAG;
}

/*===========================================================================*/
/* BOOL _RadioFMStnInfoData_Get															*/
/*===========================================================================*/
BOOL _RadioAMFMStnInfoData_Get(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {

        case RADIO_DOSID_STNINFO_BAND:
        {
           *pdwData =  (DWORD) ((tRadioAMFMStnInfoDataBox*)thiss)->nBand;
            break;
        }


        case RADIO_DOSID_STNINFO_FREQ:
        {
            *pdwData = (DWORD) ((tRadioAMFMStnInfoDataBox*)thiss)->Frequency;
            break;
        }

        case RADIO_DOSID_STNINFO_QUALITY:
        {
           *pdwData =  (DWORD) ((tRadioAMFMStnInfoDataBox*)thiss)->u32_Quality;
            break;
        }

        case RADIO_DOSID_STNINFO_AMFM_PSN:
        {
			memset(Service_Name, 0, sizeof(Service_Name));
			if(((tRadioAMFMStnInfoDataBox*)thiss)->u8_Char_Set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioAMFMStnInfoDataBox*)thiss)->PServiceName), (Tu8*)Service_Name, RADIO_MAX_CHAN_NAME_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioAMFMStnInfoDataBox*)thiss)->u8_Char_Set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioAMFMStnInfoDataBox*)thiss)->PServiceName), (Tu8*)Service_Name, RADIO_MAX_CHAN_NAME_SIZE);
			}
			else if(((tRadioAMFMStnInfoDataBox*)thiss)->u8_Char_Set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, ((tRadioAMFMStnInfoDataBox*)thiss)->PServiceName, RADIO_MAX_CHAN_NAME_SIZE);
			}
			else{/* For MISRA C */}

            *pdwData = (DWORD)Service_Name;
            break;
        }

        case RADIO_DOSID_STNINFO_PI:
        {
            *pdwData = (DWORD) ((tRadioAMFMStnInfoDataBox*)thiss)->u16_PI;
            break;
        }

        case RADIO_DOSID_STNINFO_TA:
        {
           *pdwData =  (DWORD) ((tRadioAMFMStnInfoDataBox*)thiss)->u8_TA;
            break;
        }

        case RADIO_DOSID_STNINFO_TP:
        {
           *pdwData =  (DWORD) ((tRadioAMFMStnInfoDataBox*)thiss)->u8_TP;
            break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

/*===========================================================================*/
/* int _RadioDABStnInfoData_GetId														*/
/*===========================================================================*/
INT _RadioDABStnInfoData_GetId(void * thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_STATION_INFO_DIAG;
}

/*===========================================================================*/
/* BOOL _RadioDABStnInfoData_Get														*/
/*===========================================================================*/
BOOL _RadioDABStnInfoData_Get(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
		case RADIO_DOSID_STNINFO_BAND:
        {
            *pdwData = (DWORD)RADIO_MODE_DAB;
            break;
        }
        case RADIO_DOSID_STNINFO_FREQ:
        {
            *pdwData = (DWORD) ((tRadioDABStnInfoDataBox*)thiss)->Frequency;
            break;
        }

        case RADIO_DOSID_STNINFO_EID:
        {
            *pdwData = (DWORD) ((tRadioDABStnInfoDataBox*)thiss)->uEnsembleID;
            break;
        }

        case RADIO_DOSID_STNINFO_SID:
        {
            *pdwData = (DWORD) ((tRadioDABStnInfoDataBox*)thiss)->ServiceID;
            break;
        }

        case RADIO_DOSID_STNINFO_SCID:
        {
            *pdwData = (DWORD) ((tRadioDABStnInfoDataBox*)thiss)->CompID;
            break;
        }

        case RADIO_DOSID_STNINFO_DAB_SERVICENAME:
        {
			memset(Service_Name, 0, sizeof(Service_Name));
			if(((tRadioDABStnInfoDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioDABStnInfoDataBox*)thiss)->ServiceName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioDABStnInfoDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioDABStnInfoDataBox*)thiss)->ServiceName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else if(((tRadioDABStnInfoDataBox*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, ((tRadioDABStnInfoDataBox*)thiss)->ServiceName, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else{/* For MISRA C */}

            *pdwData = (DWORD)Service_Name;
            break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

/*===========================================================================*/
/* int _RadioAMFMQualityData_GetId														*/
/*===========================================================================*/
INT _RadioAMFMQualityData_GetId(void * thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_QUALITY_DIAG;
}

/*===========================================================================*/
/* BOOL _RadioAMFMQualityData_Get														*/
/*===========================================================================*/
BOOL _RadioAMFMQualityData_Get(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_QUALITY_BAND:
        {
            *pdwData =  (DWORD) ((tRadioAMFMQualityDataBox*)thiss)->nBand;
            break;
        }

		case RADIO_DOSID_QUALITY_RF_FS:
        {
            *pdwData = (Ts8) ((tRadioAMFMQualityDataBox*)thiss)->s32_RFFieldStrength;
            break;
        }

		case RADIO_DOSID_QUALITY_BB_FS:
        {
            *pdwData = (Ts8) ((tRadioAMFMQualityDataBox*)thiss)->s32_BBFieldStrength;
            break;
        }

		case RADIO_DOSID_QUALITY_OFS:
		{
			*pdwData = (DWORD)((tRadioAMFMQualityDataBox*)thiss)->u32_FrequencyOffset;
			break;
		}

		case RADIO_DOSID_QUALITY_MODULATION_DET:
        {
            *pdwData = (DWORD) ((tRadioAMFMQualityDataBox*)thiss)->u32_ModulationDetector;
            break;
        }

		case RADIO_DOSID_QUALITY_MULTIPATH:
		{
			*pdwData = (Ts8)((tRadioAMFMQualityDataBox*)thiss)->u32_Multipath;
			break;
		}

        case RADIO_DOSID_QUALITY_USN:
        {
            *pdwData = (Ts8) ((tRadioAMFMQualityDataBox*)thiss)->u32_UltrasonicNoise;
            break;
        }

		case RADIO_DOSID_QUALITY_ADJCHANNEL:
        {
            *pdwData = (DWORD) ((tRadioAMFMQualityDataBox*)thiss)->u32_AdjacentChannel;
            break;
        }

		case RADIO_DOSID_QUALITY_SNR_LEVEL:
		{
			*pdwData = (DWORD)((tRadioAMFMQualityDataBox*)thiss)->u32_SNR;
			break;
		}
	
		case RADIO_DOSID_QUALITY_COCHANNEL:
		{
			*pdwData = (DWORD)((tRadioAMFMQualityDataBox*)thiss)->u32_coChannel;
			break;
		}

		case RADIO_DOSID_QUALITY_STEREOMONO:
		{
			*pdwData = (DWORD)((tRadioAMFMQualityDataBox*)thiss)->u32_StereoMonoReception;
			break;
		}

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}


/*===========================================================================*/
/* int _RadioDABQualityData_GetId														*/
/*===========================================================================*/
INT _RadioDABQualityData_GetId(void * thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_QUALITY_DIAG;
}

/*===========================================================================*/
/* BOOL _RadioDABQualityData_Get														*/
/*===========================================================================*/
BOOL _RadioDABQualityData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
		case RADIO_DOSID_QUALITY_BAND:
        {
            *pdwData = (Ts8)RADIO_MODE_DAB;
            break;
        }

		case RADIO_DOSID_QUALITY_RF_FS:
		{
			*pdwData = (Ts8)((tRadioDABQualityDataBox*)thiss)->s32_RFFieldStrength;
			break;
		}

		case RADIO_DOSID_QUALITY_BB_FS:
		{
			*pdwData = (Ts8)((tRadioDABQualityDataBox*)thiss)->s32_BBFieldStrength;
			break;
		}
		
		case RADIO_DOSID_QUALITY_FICBER:
        {
            *pdwData = (Ts8) ((tRadioDABQualityDataBox*)thiss)->u32_FicBitErrorRatio;
            break;
        }

		case RADIO_DOSID_QUALITY_IS_VALID_FICBER:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->b_isValidFicBitErrorRatio;
			break;
		}

		case RADIO_DOSID_QUALITY_MSCBER:
        {
            *pdwData = (Ts8) ((tRadioDABQualityDataBox*)thiss)->u32_MscBitErrorRatio;
            break;
        }

		case RADIO_DOSID_QUALITY_IS_VALID_MSCBER:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->b_isValidMscBitErrorRatio;
			break;
		}

		case RADIO_DOSID_QUALITY_DATASCHBER:
        {
            *pdwData = (Ts8) ((tRadioDABQualityDataBox*)thiss)->u32_DataSChBitErrorRatio;
            break;
        }

		case RADIO_DOSID_QUALITY_IS_VALID_DATASCHBER:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->b_isValidDataSChBitErrorRatio;
			break;
		}

		case RADIO_DOSID_QUALITY_AUDSCHBER:
        {
            *pdwData = (Ts8) ((tRadioDABQualityDataBox*)thiss)->u32_AudioSChBitErrorRatio;
            break;
        }

		case RADIO_DOSID_QUALITY_IS_VALID_AUDSCHBER:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->b_isValidAudioSChBitErrorRatio;
			break;
		}

		case RADIO_DOSID_QUALITY_AUDBER_LEVEL:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->u8_AudBitErrorRatio;
			break;
		}

		case RADIO_DOSID_QUALITY_REED_SOLOMON:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->u8_ReedSolomonInfo;
			break;
		}
		
		case RADIO_DOSID_QUALITY_SYNC_STATUS:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->u8_syncStatus;
			break;
		}

		case RADIO_DOSID_QUALITY_MUTE_STATUS:
		{
			*pdwData = (Tu8)((tRadioDABQualityDataBox*)thiss)->b_muteStatus;
			break;
		}

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

/*===========================================================================*/
/* int _RadioAFSwitchStatus_GetId														*/
/*===========================================================================*/
INT _RadioAFSwitchStatus_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_AF_STATUS;
}
/*===========================================================================*/
/* BOOL _RadioAFSwitchStatus_Get														*/
/*===========================================================================*/
BOOL _RadioAFSwitchStatus_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_AF_SWITCH_STATUS:
        {
            *pdwData = (DWORD) ((tRadioAFSwitchStatusDataBox*)thiss)->Status;
            break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

/*===========================================================================*/
/* int _RadioDABFMlinkstatus_GetId														*/
/*===========================================================================*/
INT _RadioDABFMlinkstatus_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_DABFM_LINK_STATUS;
}
/*===========================================================================*/
/* BOOL _RadioDABFMlinkstatus_Get														*/
/*===========================================================================*/
BOOL _RadioDABFMlinkstatus_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_DABFM_LINK_STATUS:
        {
            *pdwData = (DWORD) ((tRadioDABFMLinkStatusDataBox*)thiss)->Status;
            break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

/*===========================================================================*/
/* int _RadioDABFMlinkstatus_GetId														*/
/*===========================================================================*/
INT _RadioAMFMAFlistData_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_AF_LIST;
}
/*===========================================================================*/
/* BOOL _RadioAMFMAFlistData_Get														*/
/*===========================================================================*/
BOOL _RadioAMFMAFlistData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
		case RADIO_DOSID_AFLIST_BAND:
		{
			*pdwData = (DWORD)RADIO_MODE_FM;	
			break;
		}
        case RADIO_DOSID_AFLIST_NUM_AF:
        {
            *pdwData = (DWORD) ((tRadioAMFMAFListDataBox*)thiss)->NumAFList;
            break;
        }

		case RADIO_DOSID_AFLIST:
		{
			*pdwData = (DWORD) ((tRadioAMFMAFListDataBox*)thiss)->AFList;	
			break;
		}

        case RADIO_DOSID_AF_QUALITY:
        {
             *pdwData = (DWORD) ((tRadioAMFMAFListDataBox*)thiss)->Quality;
			  break;
        }

        case RADIO_DOSID_AFLIST_PI_LIST:
        {
            *pdwData = (DWORD) ((tRadioAMFMAFListDataBox*)thiss)->PIList;	
			  break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

/*===========================================================================*/
/* int _RadioDABAFlistData_GetId														*/
/*===========================================================================*/
INT _RadioDABAFlistData_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_AF_LIST;
}
/*===========================================================================*/
/* BOOL _RadioDABAFlistData_Get														*/
/*===========================================================================*/
BOOL _RadioDABAFlistData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
		case RADIO_DOSID_AFLIST_BAND:
        {
            *pdwData = (DWORD)RADIO_MODE_DAB;
            break;
        }
		
		case RADIO_DOSID_DAB_NUM_ALT_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioDABAFListDataBox*)thiss)->NumofAltFrequecy;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]DAB Number_Alternate Frequency- HMI APP: %d", *pdwData);
            break;
        }
		
		case RADIO_DOSID_DAB_NUM_ALT_ENSEMBLE:
        {
            *pdwData = (DWORD) ((tRadioDABAFListDataBox*)thiss)->NumofAltEnsemble;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]DAB Number_Alternate Ensemble- HMI APP: %d", *pdwData);
            break;
        }
		
		case RADIO_DOSID_DAB_NUM_HARDLINK_SID:
        {
            *pdwData = (DWORD) ((tRadioDABAFListDataBox*)thiss)->NumofHardlinkSid;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]DAB Number_HardLink Sid- HMI APP: %d", *pdwData);
            break;
        }
        case RADIO_DOSID_DAB_ALT_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioDABAFListDataBox*)thiss)->DAB_AltFreqList;
            break;
        }

		case RADIO_DOSID_DAB_ALT_EID:
		{
			*pdwData = (DWORD) ((tRadioDABAFListDataBox*)thiss)->DAB_AltEnsembleList;
			break;
		}
		
		case RADIO_DOSID_DAB_HARDLINK_SID:
		{
			*pdwData = (DWORD) ((tRadioDABAFListDataBox*)thiss)->DAB_HardlinkSidList;
			break;
		}

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}

INT Radio_Default_IDataObject_GetId(void*thiss)
{
	UNUSED(thiss);
	return FALSE;
}


/*---------------------------------Preset related functions---------------------------------------*/

/*===========================================================================*/
/* HMIIF_IDataObject _RadioPresetListData_Display_GetAt							   */
/*===========================================================================*/
HMIIF_IDataObject* _RadioMemoryListData_Display_GetAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
    if((nIndex >= 0) && (nIndex < MAX_RADIO_PRESET_STATIONS))
    {
        pDataObject.thiss =   &_radioMemoryListData[nIndex];
        pDataObject.GetId =  _RadioMemoryListDataDisplay_GetId;
        pDataObject.Get =    _RadioMemoryListDataDisplay_GetInfo;
    }
    return &pDataObject;
}

/*===========================================================================*/
/* int _RadioPresetListDataDisplay_GetId														*/
/*===========================================================================*/
INT _RadioMemoryListDataDisplay_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_MEMORY_LIST;
}
/*===========================================================================*/
/* int _RadioPresetListDataDisplay_GetInfo														*/
/*===========================================================================*/
BOOL _RadioMemoryListDataDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData)
{
	BOOL bRes = TRUE;
    switch(nSubId)
    {

        case RADIO_DOSID_MEMORYLIST_BAND:
        {
            *pdwData = (DWORD) ((tRadioMemoryListInfoDataBox_Display*)thiss)->nBand;
            break;
        }

		case RADIO_DOSID_MEMORYLIST_INDEX:
		{
			*pdwData = (DWORD) ((tRadioMemoryListInfoDataBox_Display*)thiss)->index;
            break;
		}

        case RADIO_DOSID_MEMORYLIST_SERVICENAME:
        {
			memset(Service_Name, 0, sizeof(Service_Name));
			if(((tRadioMemoryListInfoDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_EBU)
			{
				HMI_IF_EBU_to_UTF8((Tu8*)(((tRadioMemoryListInfoDataBox_Display*)thiss)->MemoryStName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE, sizeof(Service_Name));
			}
			else if(((tRadioMemoryListInfoDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UCS2)
			{
				HMI_IF_UCS2_to_UTF8((Tu8*)(((tRadioMemoryListInfoDataBox_Display*)thiss)->MemoryStName), (Tu8*)Service_Name, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else if(((tRadioMemoryListInfoDataBox_Display*)thiss)->Char_set == RADIO_MNGR_APP_CHARSET_UTF8)
			{
				SYS_RADIO_MEMCPY(Service_Name, ((tRadioMemoryListInfoDataBox_Display*)thiss)->MemoryStName, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else
			{
				/* Do nothing For MISRA C */
			}

            *pdwData = (DWORD)Service_Name; 
            break;
        }

		case RADIO_DOSID_MEMORYLIST_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioMemoryListInfoDataBox_Display*)thiss)->Frequency;
            break;
        }
		case RADIO_DOSID_MEMORYLIST_CHANNELNAME:
        {
            *pdwData = (DWORD) ((tRadioMemoryListInfoDataBox_Display*)thiss)->DAB_Channel_Name;
            break;
        }

        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*===========================================================================*/
/* BOOL _RadioPresetListData_Display_GetCount														*/
/*===========================================================================*/
INT _RadioMemoryListData_Display_GetCount(void *thiss)
{
	UNUSED(thiss);
	//return noStationsDisplay;
	return (INT)(preset_index);
}

/*===========================================================================*/
/* int _RadioAnnouncementData_GetId															*/
/*===========================================================================*/
INT _RadioAnnouncementData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_ANNOUNCEMENT;
}

/*===========================================================================*/
/* BOOL _RadioAnnouncementData_Get														*/
/*===========================================================================*/
BOOL _RadioAnnouncementData_Get(void*  thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
		case RADIO_DOSID_ANNO_STATUS:
		{
			*pdwData = (DWORD) ((tRadioAnnouncementInfoDataBox*)thiss)->Anno_Status;
			break;
		}

		default:
        bRes = FALSE;
        break;
    }
    return bRes;
}


/*===========================================================================*/
/* int _RadioComponentStatusData_GetId															*/
/*===========================================================================*/
INT _RadioComponentStatusData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_COMPONENT_STATUS;
}


/*===========================================================================*/
/* BOOL _RadioComponentStatusData_Get														*/
/*===========================================================================*/
BOOL _RadioComponentStatusData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
		case RADIO_DOSID_COMPONENT_STATUS_BAND:
        {
             *pdwData = (DWORD) ((tRadioComponentStatusInfoDataBox*)thiss)->nBand;
        }
        break;
		
    	case RADIO_DOSID_AMFMTUNER_COMPONENT_STATUS:
        {
            *pdwData = (DWORD) ((tRadioComponentStatusInfoDataBox*)thiss)->e_AMFMTuner_Status;
		}
        break;

        case RADIO_DOSID_DABTUNER_COMPONENT_STATUS:
        {
            *pdwData = (DWORD) ((tRadioComponentStatusInfoDataBox*)thiss)->e_DABTuner_Status;
		}
        break;
		case RADIO_DOSID_DAB_UP_NOTIFICATION_STATUS:
        {
            *pdwData = (DWORD) ((tRadioComponentStatusInfoDataBox*)thiss)->e_DAB_UpNot_Status;
		}
        break;

        default:
		{
        	bRes = FALSE;
		}
		break;
    }
    return bRes;
}


/*----------------------For Radio Settings Status Functions--------------------------------------*/

/*===========================================================================*/
/* int _RadioComponentStatusData_GetId															*/
/*===========================================================================*/
INT _RadioSettingsStatusData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_SETTINGS;
}


/*===========================================================================*/
/* BOOL _RadioComponentStatusData_Get														*/
/*===========================================================================*/
BOOL _RadioSettingsStatusData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_DABFM_SETTING_STATUS:
        {
            *pdwData = (DWORD) ((tRadioSettingsStatusInfoDataBox*)thiss)->e_DABFM_Status;
            break;
        }

		case RADIO_DOSID_ANNO_SETTING_STATUS:
        {
            *pdwData = (DWORD) ((tRadioSettingsStatusInfoDataBox*)thiss)->e_TA_Anno_Status;
            break;
        }

		case RADIO_DOSID_RDS_FOLLOWUP_SETTING_STATUS:
        {
            *pdwData = (DWORD) ((tRadioSettingsStatusInfoDataBox*)thiss)->e_RDS_Status;
            break;
        }
		
		case RADIO_DOSID_INFO_ANNO_SETTING_STATUS:
		{
			*pdwData = (DWORD) ((tRadioSettingsStatusInfoDataBox*)thiss)->e_Info_Anno_Status;
            break;
		}
		
		case RADIO_DOSID_MULTIPLEX_SWITCH_SETTING_STATUS:
		{
			*pdwData = (DWORD) ((tRadioSettingsStatusInfoDataBox*)thiss)->e_Multiplex_Switch_Status;
            break;
		}

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}


/*---------------------For Radio Activity State--------------------------------------------------------*/

/*===========================================================================*/
/* int _RadioActivityStatusData_GetId															*/
/*===========================================================================*/
INT _RadioActivityStatusData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_ACTIVITY_STATE;
}

/*===========================================================================*/
/* BOOL _RadioActivityStatusData_Get														*/
/*===========================================================================*/
BOOL _RadioActivityStatusData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_ACTIVITY_STATE_BAND:
        {
            *pdwData = (DWORD) ((tRadioActivityStatusDataBox*)thiss)->nBand;
            break;
        }

		case RADIO_DOSID_ACTIVITY_STATE:
        {
            *pdwData = (DWORD) ((tRadioActivityStatusDataBox*)thiss)->e_Activity_Status;
            break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}


/*------------------------------------For Notifying Best PI station Information------------------------------------------------------*/

/*===========================================================================*/
/* int _RadioBestPIStationData_GetId															*/
/*===========================================================================*/
INT _RadioBestPIStationData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_BESTPI_INFO;
}

/*===========================================================================*/
/* BOOL _RadioBestPIStationData_Get														*/
/*===========================================================================*/
BOOL _RadioBestPIStationData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_BESTPI_FREQUENCY:
        {
            *pdwData = (DWORD) ((tRadioBestPIStationDataBox*)thiss)->Frequency;
            break;
        }

		case RADIO_DOSID_BESTPI_PI:
        {
            *pdwData = (DWORD) ((tRadioBestPIStationDataBox*)thiss)->Best_PI;
            break;
        }

        case RADIO_DOSID_BESTPI_QUALITY:
        {
            *pdwData = (DWORD) ((tRadioBestPIStationDataBox*)thiss)->Quality;
            break;   
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}



/*------------------------------------For Notifying Clock Time Information------------------------------------------------------*/

/*===========================================================================*/
/* int _RadioClockTimeInfoData_GetId															*/
/*===========================================================================*/
INT _RadioClockTimeInfoData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_CLOCKTIME_INFO;
}

/*===========================================================================*/
/* BOOL _RadioClockTimeInfoData_Get														*/
/*===========================================================================*/
BOOL _RadioClockTimeInfoData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_CLOCKTIME_HOUR:
        {
            *pdwData = (DWORD) ((tRadioGetClockTimeInfoDataBox*)thiss)->Hour;
            break;
        }
		
		case RADIO_DOSID_CLOCKTIME_MINUTES:
        {
            *pdwData = (DWORD) ((tRadioGetClockTimeInfoDataBox*)thiss)->Minutes;
            break;
        }
		
		case RADIO_DOSID_CLOCKTIME_DAY:
        {
            *pdwData = (DWORD) ((tRadioGetClockTimeInfoDataBox*)thiss)->Day;
            break;
        }
		
		case RADIO_DOSID_CLOCKTIME_MONTH:
        {
            *pdwData = (DWORD) ((tRadioGetClockTimeInfoDataBox*)thiss)->Month;
            break;
        }
		
		case RADIO_DOSID_CLOCKTIME_YEAR:
        {
            *pdwData = (DWORD) ((tRadioGetClockTimeInfoDataBox*)thiss)->Year;
            break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}


/*------------------------------------For Notifying Radio Firmware Version Information------------------------------------------------------*/

/*===========================================================================*/
/* int _RadioFirmwareVersionInfoData_GetId															*/
/*===========================================================================*/
INT _RadioFirmwareVersionInfoData_GetId(void* thiss)
{
	UNUSED(thiss);
	return RADIO_DOID_FIRMWARE_VERSION_INFO;
}

/*===========================================================================*/
/* BOOL _RadioFirmwareVersionInfoData_Get														*/
/*===========================================================================*/
BOOL _RadioFirmwareVersionInfoData_Get(void* thiss, INT nSubId, DWORD* pdwData)
{
    BOOL bRes = TRUE;
    switch(nSubId)
    {
        case RADIO_DOSID_AMFMTUNER_FIRMWARE_VERSION:
        {
            *pdwData = (DWORD) ((tRadioGetFirmareVersionInfoDataBox*)thiss)->AMFMTuner_FW_Version;
            break;
        }
		
		case RADIO_DOSID_DABTUNER_HARDWARE_VERSION:
        {
            *pdwData = (DWORD) ((tRadioGetFirmareVersionInfoDataBox*)thiss)->DABTuner_FW_HWVersion;
            break;
        }
		
		case RADIO_DOSID_DABTUNER_FIRMWARE_VERSION:
        {
            *pdwData = (DWORD) ((tRadioGetFirmareVersionInfoDataBox*)thiss)->DABTuner_FW_SWVersion;
            break;
        }

        default:
            bRes = FALSE;
            break;
    }
    return bRes;
}


/*===========================================================================*/
/* int _RadioDABDataServiceDisplay_GetId									 */
/*===========================================================================*/
INT _RadioDABDataServiceDisplay_GetId(void* thiss)
{
	UNUSED(thiss);
    return RADIO_DOID_DATA_SERVICE;
}
/*===========================================================================*/
/* BOOL _RadioDABDataServiceDisplay_GetInfo									 */
/*===========================================================================*/
BOOL _RadioDABDataServiceDisplay_GetInfo(void* thiss, INT nSubId, DWORD* pdwData)
{
	BOOL bRes = TRUE;
    switch(nSubId)
    {

        case RADIO_DOSID_DATASERVICE_HEADER:
        {
            *pdwData = (DWORD) ((tRadioDABDataServiceInfoDataBox_Display*)thiss)->e_Header;
            break;
        }

		case RADIO_DOSID_DATASERVICE_PAYLOAD:
		{
			*pdwData = (DWORD) (*((UINT8*)thiss));
		}
		break;

        default:
        {
            bRes = FALSE;
            break;
        }
    }
    return bRes;
}

/*===========================================================================*/
/* HMIIF_IDataObject* _RadioDABDataService_Display_GetAt					 */
/*===========================================================================*/
HMIIF_IDataObject* _RadioDABDataService_Display_GetAt(void *thiss, INT nIndex)
{
	UNUSED(thiss);
	if ((nIndex >= 0) && (nIndex < RADIO_MAX_DATA_SERVICE_PAYLOAD_SIZE))
	{
		pDataObject.thiss = &_radioDABDataServiceRawData.u8_Payload[nIndex];
		pDataObject.GetId = _RadioDABDataServiceDisplay_GetId;
		pDataObject.Get = _RadioDABDataServiceDisplay_GetInfo;
	}
	return &pDataObject;
}

/*===========================================================================*/
/* INT _RadioDABDataService_Display_GetSize								 */
/*===========================================================================*/
INT _RadioDABDataService_Display_GetSize(void *thiss)
{
	return (INT)(((tRadioDABDataServiceInfoDataBox_Display*)thiss)->u32_PayloadSize);
}

/*=============================================================================
    end of file
=============================================================================*/