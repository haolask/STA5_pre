#include "hmi_if_app_request.h"
#include "Radio_App.h"
#include "stdio.h"
#include <windows.h>
#include <process.h>
#include <winbase.h>
#include "DAB_HAL_Interface.h"

#pragma comment(lib, "Ws2_32.lib")

tRadioDABDataServiceInfoDataBox_Display slsdata;

Radio_EtalHardwareAttr R_Etal_Initparams;

int main()
{
    //UCHAR i = 0;
	/* Callback function to receive notifications from RadioLib */
	_pfnNotifyHdlr = radioNotificationHdlr;

	/* Remove the LSM & NVM files to make it a Cold start */
	remove("D:\\ST_NVM.bin");
	remove("D:\\ST_LSM.bin");

	/* Startup Test */

	printf("[HMI_TEST_APP] Initializing ETAL...\n");
	// Enable CMOST device
	R_Etal_Initparams.m_DCOPAttr.m_isDisabled = FALSE;		/* To be revisited for DAB enabled in market																	*/
	R_Etal_Initparams.m_DCOPAttr.m_doFlashDump = FALSE;		/* ETAL reads back the current DCOP firmware image to verify, if program flashed correctly						*/
	R_Etal_Initparams.m_DCOPAttr.m_doFlashProgram = FALSE;		/* ETAL writes firmware image into flash memory attached to DCOP												*/
	R_Etal_Initparams.m_DCOPAttr.m_doDownload = 0;		/* ETAL writes the DCOP image to the DCOP volatile memory. If m_doFlashProgram is TRUE, m_doDownload  is FALSE	*/
	R_Etal_Initparams.m_DCOPAttr.m_cbGetImage = NULL;			/* Pointer to a function that ETAL calls to obtain a DCOP Firmware chunk										*/
	R_Etal_Initparams.m_DCOPAttr.m_pvGetImageContext = NULL;			/* ETAL copies this parameter as - is to the pvContext parameter of the m_cbGetImage function					*/
	R_Etal_Initparams.m_DCOPAttr.m_cbPutImage = NULL;			/* Pointer to a function that ETAL calls to provide to the application a DCOP Firmware block					*/
	R_Etal_Initparams.m_DCOPAttr.m_pvPutImageContext = NULL;			/* ETAL copies this parameter as - is to the pvContext parameter of the m_cbPutImage function					*/

	/* STAR Tuner - 1 Attribute Configuration */

	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_isDisabled = FALSE;		/* This CMOST device is initialized and  used in the active ETAL configuration					*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_useXTALalignment = FALSE;		/* To be revisited Crystal Value alignment														*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_XTALalignment = 0;			/* XTAL adjustment value, considered only if the m_useXTALalignment flag is set to TRUE			*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_useCustomParam = 0xFF;			/* ETAL does not write any parameter to the CMOST device										*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_CustomParamSize = 0;			/* Number of entries in the m_CustomParam array													*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_CustomParam = NULL;			/* Pointer to an array of integers containing the custom parameters for this CMOST device		*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_useDownloadImage = 0;			/* ETAL uses the default images built in the TUNER_DRIVER										*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_DownloadImageSize = 500;			/* The size in bytes of the m_DownaloadImage array. ignored if m_useDownaloadImage is set to 0	*/
	R_Etal_Initparams.m_tunerAttr[TUNER_0].m_DownloadImage = NULL;			/* Pointer to a one-dimensional array containing the CMOST firmware image or patches			*/

	/* STAR Tuner - 2 Attribute Configuration */

	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_isDisabled = FALSE;		/* This CMOST device is initialized and  used in the active ETAL configuration					*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_useXTALalignment = FALSE;		/* To be revisited Crystal Value alignment														*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_XTALalignment = 0;			/* XTAL adjustment value, considered only if the m_useXTALalignment flag is set to TRUE			*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_useCustomParam = 0xFF;			/* ETAL does not write any parameter to the CMOST device										*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_CustomParamSize = 0;			/* Number of entries in the m_CustomParam array													*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_CustomParam = NULL;			/* Pointer to an array of integers containing the custom parameters for this CMOST device		*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_useDownloadImage = 0;			/* ETAL uses the default images built in the TUNER_DRIVER										*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_DownloadImageSize = 200;			/* The size in bytes of the m_DownaloadImage array. ignored if m_useDownaloadImage is set to 0	*/
	R_Etal_Initparams.m_tunerAttr[TUNER_1].m_DownloadImage = NULL;			/* Pointer to a one-dimensional array containing the CMOST firmware image or patches			*/

	Request_EtalHWConfig(R_Etal_Initparams);

	printf("[HMI_TEST_APP] Starting Radio...\n");

	/* Fix e_variant as RADIO_VARIANT_A2 and u8_RadioSourceInfo as 7 to enable DAB */
	Request_StartRadio(RADIO_VARIANT_A2, RADIO_MARKET_WESTERN_EUROPE, 7);
	
	Sleep(10000);

	/* Tune to a frequency in FM */

	Request_TuneByFrequency(93500);

	Sleep(10000);

	/* Request the FM quality information in DIAG mode */
	//Request_GetQuality_Diag();

	//Sleep(20000);
	
	/* Change band to DAB */

	Request_ChangeRadioMode(RADIO_MODE_DAB);

	Sleep(25000);

	/* Request the DAB quality information in DIAG mode */
	Request_GetQuality_Diag();


	/* Tune Up in DAB */

	Request_TuneUpDown(RADIO_DIRECTION_DOWN);

	Sleep(20000);

	/* Seek Up in DAB */

	Request_SeekStation(RADIO_DIRECTION_DOWN);

	Sleep(30000);

	/* Scan in DAB */

	Request_ManualUpdateSTL();

	Sleep(30000);

	/* Shutdown */

	printf("[HMI_TEST_APP] Shutting Down Radio...\n");
	Request_ShutDownTuner();
	Sleep(10000);

	printf("[HMI_TEST_APP] ETAL Deinitialize...\n");
	Request_EtalHWDeconfig();

#if 0
	slsdata.e_Header = RADIO_DAB_DATASERV_TYPE_SLS_XPAD;
	slsdata.u32_PayloadSize = 200;
	for (i = 0; i < 200; i++)
	{
		slsdata.u8_Payload[i] = i;
	}

	DABDataService_cbFunc(&slsdata, 200, NULL, NULL);
#endif

	while (1)
	{

	}
}

void radioNotificationHdlr(HMIIF_IDataObject* pData)
{
	int dataObjStatus = 0;

	dataObjStatus = pData->GetId(pData->thiss);

	switch (dataObjStatus)
	{
		case RADIO_DOID_STATUS:
			pData->Get(pData->thiss, RADIO_DOSID_STATUS, &data);
			switch (data)
			{
				case RADIO_ETALHWCONFIG_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWCONFIG_SUCCESS\n");
				break;
				case RADIO_ETALHWCONFIG_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWCONFIG_FAILURE\n");
				break;
				case RADIO_STARTUP_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_STARTUP_SUCCESS\n");
				break;
				case RADIO_STARTUP_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_STARTUP_SUCCESS\n");
				break;
				case RADIO_SELECTBAND_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SELECTBAND_SUCCESS\n");
				break;
				case RADIO_SELECTBAND_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SELECTBAND_FAILURE\n");
				break;
				case RADIO_STNLISTSELECT_REQ_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_STNLISTSELECT_REQ_SUCCESS\n");
				break;
				case RADIO_STNLISTSELECT_REQ_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_STNLISTSELECT_REQ_FAILURE\n");
				break;
				case RADIO_SEEK_REQ_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SEEK_REQ_SUCCESS\n");
				break;
				case RADIO_SEEK_NO_SIGNAL:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SEEK_NO_SIGNAL\n");
				break;
				case RADIO_SEEK_REQ_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SEEK_REQ_FAILURE\n");
				break;
				case RADIO_SCAN_STARTED:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SCAN_STARTED\n");
				break;
				case RADIO_SCAN_INPROGRESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SCAN_INPROGRESS\n");
				break;
				case RADIO_SCAN_COMPLETE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SCAN_COMPLETE\n");
				break;
				case RADIO_SHUTDOWN_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SHUTDOWN_SUCCESS\n");
				break;
				case RADIO_SHUTDOWN_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_SHUTDOWN_FAILURE\n");
				break;
				case RADIO_MANUAL_UPDATE_STL_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_MANUAL_UPDATE_STL_SUCCESS\n");
				break;
				case RADIO_MANUAL_UPDATE_STL_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_MANUAL_UPDATE_STL_FAILURE\n");
				break;
				case RADIO_TUNE_UP_DOWN_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_TUNE_UP_DOWN_SUCCESS\n");
					break;
				case RADIO_TUNE_UP_DOWN_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_TUNE_UP_DOWN_FAILURE\n");
					break;
				case RADIO_ETALHWDECONFIG_SUCCESS:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWDECONFIG_SUCCESS\n");
				break;
				case RADIO_ETALHWDECONFIG_FAILURE:
					printf("[HMI_TEST_APP] HMI APP Status: RADIO_ETALHWDECONFIG_FAILURE\n");
				break;
				default:
                    printf("[HMI_TEST_APP] HMI APP Status Received: %lu\n", data);
			}	
		break;

		case RADIO_DOID_ACTIVITY_STATE:
			pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE_BAND, &data);
			displayBandInfo(data);

			pData->Get(pData->thiss, RADIO_DOSID_ACTIVITY_STATE, &data);
			displayActivityStatus(data);
		break;
		
		case RADIO_DOID_SETTINGS:
			/* Read DAB-FM Switch Settings */
			pData->Get(pData->thiss, RADIO_DOSID_DABFM_SETTING_STATUS, &data);
			switch (data)
			{
				case RADIO_SWITCH_SETTING_ON:
					printf("[HMI_TEST_APP] DABFM Switch is Enabled\n");
				break;
				case RADIO_SWITCH_SETTING_OFF:
					printf("[HMI_TEST_APP] DABFM Switch is Disabled\n");
				break;
				case RADIO_SWITCH_SETTING_INVALID:
					printf("[HMI_TEST_APP] DABFM Switch is Disabled\n");
				break;
			}

			/* Read Announcement Switch Settings */
			pData->Get(pData->thiss, RADIO_DOSID_ANNO_SETTING_STATUS, &data);
			switch (data)
			{
				case RADIO_SWITCH_SETTING_ON:
					printf("[HMI_TEST_APP] Announcement Switch is Enabled\n");
				break;
				case RADIO_SWITCH_SETTING_OFF:
					printf("[HMI_TEST_APP] Announcement Switch is Disabled\n");
				break;
				case RADIO_SWITCH_SETTING_INVALID:
					printf("[HMI_TEST_APP] Announcement Switch is Disabled\n");
				break;
			}

			/* Read RDS Followup Switch Settings */
			pData->Get(pData->thiss, RADIO_DOSID_RDS_FOLLOWUP_SETTING_STATUS, &data);
			switch (data)
			{
				case RADIO_SWITCH_SETTING_ON:
					printf("[HMI_TEST_APP] RDS Followup Switch is Enabled\n");
				break;
				case RADIO_SWITCH_SETTING_OFF:
					printf("[HMI_TEST_APP] RDS Followup Switch is Disabled\n");
				break;
				case RADIO_SWITCH_SETTING_INVALID:
					printf("[HMI_TEST_APP] RDS Followup Switch is Disabled\n");
				break;
			}

			/* Read Info Announcement Switch Settings */
			pData->Get(pData->thiss, RADIO_DOSID_INFO_ANNO_SETTING_STATUS, &data);
			switch (data)
			{
				case RADIO_SWITCH_SETTING_ON:
					printf("[HMI_TEST_APP] Info Announcement Switch is Enabled\n");
				break;
				case RADIO_SWITCH_SETTING_OFF:
					printf("[HMI_TEST_APP] Info Announcement Switch is Disabled\n");
				break;
				case RADIO_SWITCH_SETTING_INVALID:
					printf("[HMI_TEST_APP] Info Announcement Switch is Disabled\n");
				break;
			}

			/* Read Multiplex Emsemble list Switch Settings */
			pData->Get(pData->thiss, RADIO_DOSID_MULTIPLEX_SWITCH_SETTING_STATUS, &data);
			switch (data)
			{
				case RADIO_SWITCH_SETTING_ON:
					printf("[HMI_TEST_APP] Multiplex Ensemble List Switch is Enabled\n");
				break;
				case RADIO_SWITCH_SETTING_OFF:
					printf("[HMI_TEST_APP] Multiplex Ensemble List Switch is Disabled\n");
				break;
				case RADIO_SWITCH_SETTING_INVALID:
					printf("[HMI_TEST_APP] Multiplex Ensemble List Switch is Disabled\n");
				break;
			}
		break;

		case RADIO_DOID_STATION_INFO:
			/* Read Band Information */
			pData->Get(pData->thiss, RADIO_DOSID_BAND, &data);
			displayBandInfo(data);
			activeBand = data;

			/* Read Frequency Information */
			pData->Get(pData->thiss, RADIO_DOSID_FREQUENCY, &data);
            printf("[HMI_TEST_APP] Frequency: %d\n", data);
						
			/* Read Service Name Information */
			pData->Get(pData->thiss, RADIO_DOSID_SERVICENAME, &data);
			printf("[HMI_TEST_APP] Service Name: %s\n", data);

			if (activeBand == RADIO_MODE_DAB)
			{
				/* Read Channel Name Information */
				pData->Get(pData->thiss, RADIO_DOSID_CHANNELNAME, &data);
				printf("[HMI_TEST_APP] Channel Name: %s\n", data);

				/* Read Ensemble Name Information */
				pData->Get(pData->thiss, RADIO_DOSID_ENSEMBLENAME, &data);
				printf("[HMI_TEST_APP] Ensemble Name: %s\n", data);

				/* Read Current Service Number Information */
				pData->Get(pData->thiss, RADIO_DOSID_CURRENTSERVICENUMBER, &data);
				printf("[HMI_TEST_APP] Current Service Number: %d\n", data);

				/* Read information on total no. of services */
				pData->Get(pData->thiss, RADIO_DOSID_TOTALNUMBEROFSERVICE, &data);
				printf("[HMI_TEST_APP] Total Services: %d\n", data);
			}
			else if (activeBand == RADIO_MODE_FM)
			{
				/* Read Channel Name Information */
				pData->Get(pData->thiss, RADIO_DOSID_PROGRAMME_TYPE, &data);
				printf("[HMI_TEST_APP] Programme Type: %s\n", data);

				/* Read Channel Name Information */
				pData->Get(pData->thiss, RADIO_DOSID_TA, &data);
				printf("[HMI_TEST_APP] TA: %d\n", data);

				/* Read Channel Name Information */
				pData->Get(pData->thiss, RADIO_DOSID_TP, &data);
				printf("[HMI_TEST_APP] TP: %d\n", data);

				/* Read Channel Name Information */
				pData->Get(pData->thiss, RADIO_DOSID_PI, &data);
				printf("[HMI_TEST_APP] PI: %d\n", data);
			}
			
			/* Read Radio Text Information */
			pData->Get(pData->thiss, RADIO_DOSID_RADIOTEXT, &data);
			printf("[HMI_TEST_APP] Radio Text: %s\n", data);

		break;
		
		case RADIO_DOID_STATION_LIST_DISPLAY:
			StnList = ((IObjectList*)pData->thiss);
			pData->Get(StnList->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_BAND, &data);
			displayBandInfo(data);

			StnListCount = StnList->GetCount(StnList->thiss);

			printf("[HMI_TEST_APP] Station List Information\n");
			printf("**********************************************\n");
			printf("Index\tFrequency\tPI/Service\tMatched Index\n");

			for (i = 0; i < StnListCount; i++)
			{
				StnListObj = StnList->GetAt(StnList->thiss, i);
				StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_INDEX, &data);
				printf("[%d]\t", data);

				StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_FREQUENCY, &data);
				printf("%d \t", data);

				StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_SERVICENAME, &data);
				printf("%s \t", (char*)data);

				StnListObj->Get(StnListObj->thiss, RADIO_DOSID_STATIONLIST_DISPLAY_TUNEDSTN_INDEX, &data);
				printf("\t %d \n", data);

			}
			printf("**********************************************\n");

		break;

		case RADIO_DOID_DATA_SERVICE:
			DataService = ((IObjectList*)pData->thiss);
			pData->Get(DataService->thiss, RADIO_DOSID_DATASERVICE_HEADER, &data);
			displayDataServiceInfo(data);

			payloadSize = DataService->GetCount(DataService->thiss);

			printf("**********************************************\n");

			for (i = 0; i < payloadSize; i++)
			{
				DataServiceObj = DataService->GetAt(DataService->thiss, i);
				DataServiceObj->Get(DataServiceObj->thiss, RADIO_DOSID_DATASERVICE_PAYLOAD, &data);
				printf("[%d]\t", data);

			}
			printf("\n");
			printf("**********************************************\n");

		break;

		case RADIO_DOID_QUALITY_DIAG:
			/* Read Band Information */
			pData->Get(pData->thiss, RADIO_DOSID_QUALITY_BAND, &data);
			displayBandInfo(data);
			activeBand = data;

			pData->Get(pData->thiss, RADIO_DOSID_QUALITY_RF_FS, &data);
			printf("[HMI_TEST_APP] RF Field Strength: %d\n", data);

			pData->Get(pData->thiss, RADIO_DOSID_QUALITY_BB_FS, &data);
			printf("[HMI_TEST_APP] BB Field Strength: %d\n", data);

			if ((activeBand == RADIO_MODE_AM) || (activeBand == RADIO_MODE_FM))
			{
				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_OFS, &data);
				printf("[HMI_TEST_APP] Frequency Offset: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_MODULATION_DET, &data);
				printf("[HMI_TEST_APP] Modulation Detector: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_MULTIPATH, &data);
				printf("[HMI_TEST_APP] Multipath: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_USN, &data);
				printf("[HMI_TEST_APP] USN: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_ADJCHANNEL, &data);
				printf("[HMI_TEST_APP] Adjacent Channel: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_SNR_LEVEL, &data);
				printf("[HMI_TEST_APP] SNR: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_COCHANNEL, &data);
				printf("[HMI_TEST_APP] Co Channel: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_STEREOMONO, &data);
				printf("[HMI_TEST_APP] Stereo Mono: %d\n", data);
			}
			else if (activeBand == RADIO_MODE_DAB)
			{
				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_FICBER, &data);
				printf("[HMI_TEST_APP] m_FicBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_IS_VALID_FICBER, &data);
				printf("[HMI_TEST_APP] m_isValidFicBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_MSCBER, &data);
				printf("[HMI_TEST_APP] m_MscBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_IS_VALID_MSCBER, &data);
				printf("[HMI_TEST_APP] m_isValidMscBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_DATASCHBER, &data);
				printf("[HMI_TEST_APP] m_dataSubChBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_IS_VALID_DATASCHBER, &data);
				printf("[HMI_TEST_APP] m_isValiddataSubChBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_AUDSCHBER, &data);
				printf("[HMI_TEST_APP] m_audioSubChBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_IS_VALID_AUDSCHBER, &data);
				printf("[HMI_TEST_APP] m_isValidaudioSubChBitErrorRatio: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_AUDBER_LEVEL, &data);
				printf("[HMI_TEST_APP] Audio BER Level: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_REED_SOLOMON, &data);
				printf("[HMI_TEST_APP] Reed Solomon Info: %d\n", data);

				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_SYNC_STATUS, &data);
				printf("[HMI_TEST_APP] Sync Status: %d\n", data);
				
				pData->Get(pData->thiss, RADIO_DOSID_QUALITY_MUTE_STATUS, &data);
				printf("[HMI_TEST_APP] Mute Status: %d\n", data);
			}
			else
			{
				/* Do Nothing */
			}

		break;

		default:
			printf("[HMI_TEST_APP] Invalid ID received\n");
		break;
	}

}

void displayDataServiceInfo(DWORD data)
{
	switch (data)
	{
		case RADIO_DAB_DATASERV_TYPE_SLS_XPAD:
			printf("[HMI_TEST_APP] Data Service Received: SLS XPAD\n");
		break;
		case RADIO_DAB_DATASERV_TYPE_DLPLUS:
			printf("[HMI_TEST_APP] Data Service Received: DLPLUS\n");
		break;
		default:
		break;
	}
}

void displayBandInfo(DWORD data)
{
	switch (data)
	{
		case RADIO_MODE_AM:
			printf("[HMI_TEST_APP] Current Band: AM\n");
		break;
		case RADIO_MODE_FM:
			printf("[HMI_TEST_APP] Current Band: FM\n");
		break;
		case RADIO_MODE_DAB:
			printf("[HMI_TEST_APP] Current Band: DAB\n");
		break;
		case RADIO_MODE_INVALID:
			printf("[HMI_TEST_APP] Current Band: INVALID\n");
		break;
	}
}

void displayActivityStatus(DWORD data)
{
	switch (data)
	{
		case RADIO_STATION_NOT_AVAILABLE:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_STATION_NOT_AVAILABLE\n");
        }
        break;

		case RADIO_FM_AF_PROCESSING:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_FM_AF_PROCESSING\n");    
        }
        break;

		case RADIO_FM_INTERNAL_SCAN_PROCESS:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_FM_INTERNAL_SCAN_PROCESS\n");
        }
        break;

		case RADIO_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING\n");
        }
        break;

		case RADIO_DAB_AF_PROCESSING:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DAB_AF_PROCESSING\n");
        }
        break;

		case RADIO_DAB_INTERNAL_SCAN_PROCESS:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DAB_INTERNAL_SCAN_PROCESS\n");
        }
        break;

		case RADIO_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING\n");
        }
        break;
		
		case RADIO_FM_LEARNMEM_AF_PROCESSING:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_FM_LEARNMEM_AF_PROCESSING\n");
        }
        break;
		
		case RADIO_DAB_LEARNMEM_AF_PROCESSING:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DAB_LEARNMEM_AF_PROCESSING\n");
        }
        break;
		
		case RADIO_SIGNAL_LOSS:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_SIGNAL_LOSS\n");
        }
        break;
		
		case RADIO_DAB_DAB_STARTED:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DAB_DAB_STARTED\n");
        }
        break;
		
		case RADIO_DAB_DAB_LINKING_DONE:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DAB_DAB_LINKING_DONE\n");
        }
        break;
		
		case RADIO_LISTENING:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_LISTENING\n");
        }
        break;

		case RADIO_IN_SCAN:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_IN_SCAN\n");    
        }
        break;

		case RADIO_TUNE_UPDOWN:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_TUNE_UPDOWN\n");
        }
        break;

		case RADIO_ANNOUNCEMENT:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_ANNOUNCEMENT\n");
        }
        break;

		case RADIO_DABFMLINKING_DONE:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DABFMLINKING_DONE\n");
        }
        break;

		case RADIO_IMPLICIT_LINKING_DONE:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_IMPLICIT_LINKING_DONE\n");
        }
        break;
		
		case RADIO_AF_SWITCHING_ESTABLISHED:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_AF_SWITCHING_ESTABLISHED\n");
        }
        break;
		
		case RADIO_DABTUNER_ABNORMAL:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_DABTUNER_ABNORMAL\n");
        }
        break;
		case RADIO_STATUS_INVALID:
        {
            printf("[HMI_TEST_APP] Activity Status = RADIO_STATUS_INVALID\n");
        }
        break;
	}
}
