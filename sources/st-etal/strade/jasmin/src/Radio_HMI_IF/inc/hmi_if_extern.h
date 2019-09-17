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
*  Description			: This file contains enum and structure definitions for HMI IF component
*
*
**********************************************************************************************************/
/** \file hmi_if_extern.h 
	 <b> Brief </b>	 This file contains declaration for structures as extern type \n
*********************************************************************************************************/
#ifndef HMI_IF_EXTERN_H_
#define HMI_IF_EXTERN_H_

#include "hmi_if_common.h"
#include "IRadio.h"
#include "hmi_if_app.h"

extern PFN_NOTIFY  _pfnNotifyHdlr;
extern tRadioCommonDataBox _radioCommonData;
extern tRadioDataBoxStatus _radioStatus;
extern tRadioStationListDataBox_AM _radioStationListData_AM[MAX_AMRADIO_STATIONS];
extern tRadioStationListDataBox_FM _radioStationListData_FM[MAX_FMRADIO_STATIONS];
extern tRadioStationListDataBox_DAB _radioStationListData_DAB[MAX_DABRADIO_STATIONS];
extern tRadioStationListDataBox_Display _radioStationListData_Display[MAX_RADIO_STATIONS];
extern tRadioEnsembleListDataBox_Display _radioEnsembleListData_Display[RADIO_MAX_ENSEMBLE];
extern tRadioSTLSearchDataBox_Display _radioSTLSearchData_Display[MAX_RADIO_STATIONS];
extern tRadioMemoryListInfoDataBox_Display _radioMemoryListData[MAX_RADIO_PRESET_STATIONS];
extern tRadioAMFMStnInfoDataBox _radioAMFMStnInfoData;
extern tRadioDABStnInfoDataBox _radioDABStnInfoData;
extern tRadioAMFMQualityDataBox _radioAMFMQualityData;
extern tRadioDABQualityDataBox _radioDABQualityData;
extern tRadioAnnouncementInfoDataBox _radioAnnouncementInfoData;
extern tRadioComponentStatusInfoDataBox _radioComponentStatusData;
extern tRadioSettingsStatusInfoDataBox _radioSettingsStatusData;
extern tRadioActivityStatusDataBox _radioActivityStatusData;
extern tRadioBestPIStationDataBox _radioBestPIStationData;
extern tRadioGetClockTimeInfoDataBox	_radioClockTimeInfoData;
//extern tRadioMuteStatusDataBox _radioMuteStatusData;
extern tRadioAFSwitchStatusDataBox _radioAFSwitchStatusData;
extern tRadioAMFMAFListDataBox _radioAMFMAFListData;
extern tRadioDABAFListDataBox _radioDABAFListData;
extern tRadioDABFMLinkStatusDataBox _radioDABFMLinkStatusData;
extern tRadioGetFirmareVersionInfoDataBox _radioFirmwareVersionData;
extern tRadioDABDataServiceInfoDataBox_Display _radioDABDataServiceRawData;
extern Tu8 noAMStations;
extern Tu8 noFMStations;
extern Tu8 noDABStations;
extern Tu8 noStationsDisplay;
extern Tu8 noSearchSTLStationsDisplay;
extern Tu8 noEnsembleListDisplay;
extern Tu8 ListenerIdxHMI;
extern Tu8 ListenerIdxModeMngr;
extern Tu8 preset_index;
extern HMIIF_IDataObject _radioStaionDataObjectFM;
extern HMIIF_IDataObject _radioStaionDataObjectAM;
extern HMIIF_IDataObject _radioAMFMQualityObject;
extern HMIIF_IDataObject _radioDataObject;
extern HMIIF_IDataObject _radioStatusObject;
extern HMIIF_IDataObject _radioMuteStatusObject;
extern HMIIF_IDataObject _radioStaionDataObjectDAB;
extern HMIIF_IDataObject _radioDABQualityObject;
extern HMIIF_IDataObject _radioAMFMStnInfoObject;
extern HMIIF_IDataObject _radioDABStnInfoObject;
extern HMIIF_IDataObject _radioAFSwitchStatusObject;
extern HMIIF_IDataObject _radioAMFMAFListObject;
extern HMIIF_IDataObject _radioDABAFListObject;
extern HMIIF_IDataObject _radioDABFMLinkStatusObject;
extern HMIIF_IDataObject _radioStaionDataObjectDisplay;
extern HMIIF_IDataObject _radioSearchSTLDataObjectDisplay;
extern HMIIF_IDataObject _radioEnsembleListDataObjectDisplay;
extern HMIIF_IDataObject _radioMemoryDataObjectDisplay;
extern HMIIF_IDataObject _radioAnnouncementObject;
extern HMIIF_IDataObject _radioComponentStatusObject;
extern HMIIF_IDataObject _radioSettingsStatusObject;
extern HMIIF_IDataObject _radioActivityStatusObject;
extern HMIIF_IDataObject _radioBestPIStationObject;
extern HMIIF_IDataObject _radioClockTimeInfoObject;
extern HMIIF_IDataObject _radioFirmwareVersionInfoObject;
extern HMIIF_IDataObject _radioDABDataServiceRawDataObject;
#endif
/*=============================================================================
    end of file
=============================================================================*/