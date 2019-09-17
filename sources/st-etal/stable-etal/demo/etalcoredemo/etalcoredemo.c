//!
//!  \file 		etalcoredemo.c
//!  \brief 	<i><b> ETAL application demo </b></i>
//!  \details   Demo ETAL user application, simulates a radio using only the functions
//!             available from ETALCORE (i.e. no radiotext nor RDS)
//!  \author 	Raffaele Belardi
//!

/*
 * Implements a demo ETAL API user function with two purposes:
 * 1. verify that etalcore/exports/ contains all is needed to link
 *    with the ETAL library
 * 2. provide an example usage of the ETAL library.
 *
 * The demo is coded for AM/FM and DAB system based on CMOSTuner (TDA7707)
 * and STA66x for the Linux/Accordo2 platform.
 * Note that in such system the DAB audio decoded by the STA66x
 * is routed back to the CMOST for conversion to analog and
 * output on the same pins as the analog FM signal.
 */
#include "target_config.h" // for CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER

#if CONFIG_HOST_OS_TKERNEL
#include <tk/tkernel.h> //m//
#endif

#include "etal_api.h"

/*
 * On MinGW the usleep() function does not work well.
 * If compiling for MinGW, it usesthe Windows Sleep() function instead
 */

#include <stdio.h>
#include <string.h>
#ifdef CONFIG_HOST_OS_WIN32
	#include <windows.h>
#else
	#include <unistd.h>  // usleep
#endif
#include <stdlib.h>
#include <errno.h>

#ifdef CONFIG_COMM_CMOST_FIRMWARE_IMAGE
#include "cmost1_firmware.boot.h"     /* Local file, not under scm. User need to copy one of the tuner_driver\exports\firmware file xxx.boot.h  */
#include "cmost2_firmware.boot.h"     /* Local file, not under scm. User need to copy one of the tuner_driver\exports\firmware file xxx.boot.h or empty file */
#endif

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
//#define HAVE_FM
//#define HAVE_DAB
//#define HAVE_HD

/*
 * Adjust ETAL_VALID_*_FREQ to some known good FM/HD station
 */
#define ETAL_VALID_FM_FREQ     104800
#define ETAL_EMPTY_FM_FREQ      98800
#define ETAL_VALID_AM_FREQ     720
#define ETAL_VALID_HD_FREQ      88700
#define ETAL_VALID_HD_AM_FREQ     711

/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB_FREQ    225648
#define ETAL_EMPTY_DAB_FREQ    224096
#define ETAL_DAB_SERVICE_INDEX_DEFAULT 0

#define PREFIX0 "    "
#define PREFIX1 "        "

#ifdef CONFIG_HOST_OS_WIN32
	#define USLEEP_ONE_SEC        1000
#elif CONFIG_HOST_OS_TKERNEL
	#define USLEEP_ONE_SEC        1000
	#define Sleep				  tk_dly_tsk
#else
	#define Sleep                 usleep
	#define USLEEP_ONE_SEC        1000000
#endif

// Macros
#define etalDemoPrintf	printf

#define ETAL_DEMO_BAD_PARAM -1
#define ETAL_DEMO_RETURN_OK 0
#define ETAL_DEMO_RETURN_ERROR 1


#define ETAL_DEMO_QUALITY_PERIODICITY 1000 
//#define ETAL_DEMO_QUALITY_PERIODICITY 50



/*****************************************************************
| variables;
|----------------------------------------------------------------*/
#ifdef CONFIG_HOST_OS_TKERNEL
#define HAVE_FM
#ifdef TUNER_SUPPORT_DAB
#define HAVE_DAB
#endif
#ifdef TUNER_SUPPORT_HD
#define HAVE_HD
#endif

#endif

/*
 * these may be overridden by etalDemo_parseParameters */
#ifdef HAVE_FM
tBool useFM = 1;
tBool useAM = 0;
ETAL_HANDLE v_handle_amfm;
ETAL_HANDLE v_monitor_amfm_handle;
#else
tBool useFM = 0;
tBool useAM = 0;
#endif
#ifdef HAVE_DAB
tBool useDAB = 1;
ETAL_HANDLE v_monitor_dab_handle;
ETAL_HANDLE v_handle_dab;
#else
tBool useDAB = 0;
#endif
#ifdef HAVE_HD
tBool useHD_FM = 1;
tBool useHD_AM = 0;
ETAL_HANDLE v_monitor_hd_handle;
ETAL_HANDLE v_handle_hd;
#else
tBool useHD_FM = 0;
tBool useHD_AM = 0;
#endif
tBool digitalOut;
tBool disable_AAA = false;;
tBool earlyAudio = false;
tBool tuner1AlreadyStarted = false;
tBool tuner2AlreadyStarted = false;
tBool DCOPAlreadyStarted = false;

tBool DCOP_is_NVMless = false;

#define ETAL_DCOP_LOADER_BOOTSTRAP_FILENAME  "spiloader.bin"
#define ETAL_DCOP_SECTIONS_DESCR_FILENAME    "dcop_fw_sections.cfg"

#define ETAL_DCOP_BOOT_FILENAME_MAX_SIZE      256

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
static tChar ETALDcop_bootstrap_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_LOADER_BOOTSTRAP_FILENAME;
static tChar ETALDcop_sections_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_SECTIONS_DESCR_FILENAME;
#endif 

tU32 freq_amfm;
tU32 freq_dab;
tU32 v_ServiceNumber;
tU32 freq_HD;
ETAL_HANDLE v_HandleFE;

#ifdef CONFIG_COMM_CMOST_FIRMWARE_IMAGE
	/* The Tuner 1 firmware image */
	tU32               downloadImage1Size = sizeof(CMOST_Firmware_STAR_T);
	tU8               *downloadImage1 = (tU8 *)CMOST_Firmware_STAR_T;
	/* The Tuner 2 firmware image */
#ifndef CONFIG_MODULE_INTEGRATED
	tU32            downloadImage2Size = 0;
	tU8             *downloadImage2 = NULL;
#else
	tU32            downloadImage2Size = sizeof(CMOST_Firmware_STAR_S);
	tU8             *downloadImage2 = (tU8 *)CMOST_Firmware_STAR_S;
#endif
#else
	/* The size in bytes of the *downloadImage* array */
	tU32               downloadImage1Size = 0;
	tU32               downloadImage2Size = 0;
	/* The Tuner firmware image */
	tU8               *downloadImage1 = NULL;
	tU8               *downloadImage2 = NULL;
#endif

// Functions 

static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus);
#ifdef CONFIG_HOST_OS_TKERNEL
static int etalDemo_parseParameters(int vI_choice, tU32 freq, tU32 Sid, tU32 vI_channel);

#else
static int etalDemo_parseParameters(int argc, char **argv);
#endif

#ifdef HAVE_FM
static void etalTestFMMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext);
static int AMFMRadio(tU32 vI_AmFmFreq, tU32 vI_duration);
#endif


#ifdef HAVE_DAB
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
static int etalDemo_GetDCOPImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap);
#endif
static void etalTestDABMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext);
static int DABRadio(tU32 vI_freq, tU32 vI_Sid, tU32 vI_duration);
#endif

#ifdef HAVE_HD
static void etalTestHDMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext);
static int HDRadio(tU32 vI_freq, tU32 vI_duration);
#endif

static void etalDemo_ConfigureAudioForTuner();


#ifdef HAVE_FM
/***************************
 *
 * etalTestFMMonitorcb
 *
 **************************/
static void etalTestFMMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	etalDemoPrintf("Quality callback: FM BB signal strength is %d (dBuV)\n", pQuality->EtalQualityEntries.amfm.m_BBFieldStrength);
	etalDemoPrintf("Quality callback: FM RF signal strength is %d (dBuV)\n", pQuality->EtalQualityEntries.amfm.m_RFFieldStrength);
}

/***************************
 *
 * AMFMRadio
 *
 **************************/
static int AMFMRadio(tU32 vI_AmFmFreq, tU32 vI_duration)
{
	EtalReceiverAttr attr;
	EtalBcastQualityContainer qualfm;
	EtalBcastQualityMonitorAttr monfm;

	ETAL_HANDLE handle_amfm;
	ETAL_HANDLE monitor_amfm_handle;
	ETAL_STATUS ret;
	char line[256];
//	ETAL_HANDLE vl_FEHandle;

	/*
	 * Create a receiver configuration
	 */
	etalDemoPrintf("*************** FM FG RADIO TUNING ***************\n");

	v_monitor_amfm_handle = ETAL_INVALID_HANDLE;
	v_handle_amfm = ETAL_INVALID_HANDLE;
	handle_amfm = ETAL_INVALID_HANDLE;
	etalDemoPrintf("Create fm receiver\n");
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));

	if (useAM)
	{
		attr.m_Standard = ETAL_BCAST_STD_AM;
	}
	else
	{
		attr.m_Standard = ETAL_BCAST_STD_FM;
	}
	
	attr.m_FrontEnds[0] = v_HandleFE;
	attr.m_FrontEndsSize = 1;

	if ((ret = etal_config_receiver(&handle_amfm, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_config_receiver fm (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

	// in early audio FM 
	// assume the tuner is already stated
	//
	if ((earlyAudio == false) || (tuner1AlreadyStarted == false))
	{
    	/* Select audio source */
		if ((ret = etal_audio_select(handle_amfm, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_audio_select fm (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}
	}
	
	/*
	 * Tune to an FM station
	 */

	etalDemoPrintf("Tune to fm freq %d\n", vI_AmFmFreq);
	if ((ret = etal_tune_receiver(handle_amfm, vI_AmFmFreq)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_tune_receiver fm (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

	/*
	 * Read the signal strength, one shot
	 */

	etalDemoPrintf("GetQuality for AMFM\n");
	if ((ret = etal_get_reception_quality(handle_amfm, &qualfm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_get_reception_quality for fm (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}
	etalDemoPrintf("FM RF signal strength = %d\n", qualfm.EtalQualityEntries.amfm.m_RFFieldStrength);

	/*
	 * Set up a quality monitor
	 * We want to be notified only when the FM Signal strength is higher than 10.0,
	 * and only every 1000ms
	 */

	etalDemoPrintf("Creating FM monitor\n");
	monitor_amfm_handle = ETAL_INVALID_HANDLE;
	memset(&monfm, 0x00, sizeof(EtalBcastQualityMonitorAttr));
	monfm.m_receiverHandle = handle_amfm;
	monfm.m_CbBcastQualityProcess = etalTestFMMonitorcb;
	monfm.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_FmFieldStrength;
	monfm.m_monitoredIndicators[0].m_InferiorValue = ETAL_INVALID_MONITOR;
	monfm.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	monfm.m_monitoredIndicators[0].m_UpdateFrequency = ETAL_DEMO_QUALITY_PERIODICITY;  // millisec
	if ((ret = etal_config_reception_quality_monitor(&monitor_amfm_handle, &monfm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_config_reception_quality_monitor for fm (%d)", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}


	etalDemoPrintf("*************** TUNE FM FG DONE ***************\n");

	/*
	 * Demo end
	 */
	// duration = 0 ==> not in auto - mode : wait user to end test
	// 

	if (0 == vI_duration)
	{
			
		etalDemoPrintf("*************** ENTER A KEY TO END TEST ***************\n");
		/*
		 * Listen to some radio
		 */
		
		Sleep(5 * USLEEP_ONE_SEC);
		

		fgets(line, sizeof(line), stdin);
	}
	else if (0xFFFFFFFF != vI_duration)
	{
		/*
		 * Listen to some radio
		 */

		Sleep(vI_duration * USLEEP_ONE_SEC);
	}
	

	if (0xFFFFFFFF != vI_duration)
	{
		/*
		 * Tune to invalid station, just to signal demo end
		 */

		etalDemoPrintf("\n\n***************  END OF TEST ***************\n\n");
		
		/*
		 * Destroy quality monitor
		 */

		if ((ret = etal_destroy_reception_quality_monitor(&monitor_amfm_handle)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_reception_quality_monitor for fm (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}

		/*
		 * Destroy all receivers
		 */
		 
		etalDemoPrintf("Destroy fm receiver\n");
		if ((ret = etal_destroy_receiver(&handle_amfm)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_receiver fm (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}
	}
	else
	{
		v_monitor_amfm_handle = monitor_amfm_handle;
		v_handle_amfm = handle_amfm;
	}

	return ETAL_DEMO_RETURN_OK;
}
#endif // HAVE FM

#ifdef HAVE_DAB
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
/***************************
 *
 * etalDemo_GetDCOPImageCb
 *
 **************************/
static int etalDemo_GetDCOPImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap)
{
	static FILE *fhbs = NULL, *fhpg = NULL;
	static long file_bs_size = 0, file_pg_size = 0;
	FILE **fhgi;
	long file_position = 0, *file_size = NULL;
	size_t retfread;
	int do_get_file_size = 0;

	/* open file bootstrap or program firmware if not already open */
	if (isBootstrap != 0)
	{
		fhgi = &fhbs;
		file_size = &file_bs_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcop_bootstrap_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcop_bootstrap_filename);
			return errno;
		}
	}
	else
	{
		fhgi = &fhpg;
		file_size = &file_pg_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcop_sections_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcop_sections_filename);
			return errno;
		}
	}

	if (do_get_file_size == 1)
	{
		/* read size of file */
		if (fseek(*fhgi, 0, SEEK_END) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_END) %d\n", errno);
			return errno;
		}
		if ((*file_size = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell end of file %d\n", errno);
			return errno;
		}
		if (fseek(*fhgi, 0, SEEK_SET) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_SET) %d\n", errno);
			return errno;
		}
	}

	/* set remaining bytes number */
	if (remainingByteNum != NULL)
	{
		if ((file_position = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell %d\n", errno);
			return errno;
		}
		*remainingByteNum = (*file_size - file_position);
	}

	/* read requestedByteNum bytes in file */
	retfread = fread(block, 1, requestedByteNum, *fhgi);
	*returnedByteNum = (tU32) retfread;
	if (*returnedByteNum != requestedByteNum)
	{
		if (ferror(*fhgi) != 0)
		{
			/* error reading file */
			if (isBootstrap != 0)
			{
				printf("Error %d reading file %s\n", errno, ETALDcop_bootstrap_filename);
			}
			else
			{
				printf("Error %d reading file %s\n", errno, ETALDcop_sections_filename);
			}
			clearerr(*fhgi);
			fclose(*fhgi);
			*fhgi = NULL;
			return -1;
		}
	}

	/* Close file if EOF */
	if (feof(*fhgi) != 0)
	{
		/* DCOP bootstrap or flash program successful */
		clearerr(*fhgi);
		fclose(*fhgi);
		*fhgi = NULL;
	}
	return 0;
}
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE

/***************************
 *
 * etalTestDABMonitorcb
 *
 **************************/
static void etalTestDABMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	etalDemoPrintf("Quality callback: DAB signal strength is %d\n", pQuality->EtalQualityEntries.dab.m_BBFieldStrength);
}

/***************************
 *
 * DABRadio
 *
 **************************/
static int DABRadio(tU32 vI_freq, tU32 vI_ServiceIndex,  tU32 vI_duration)
{
	EtalReceiverAttr attr;
	EtalBcastQualityContainer qualdab;
	EtalBcastQualityMonitorAttr mondab;

	ETAL_HANDLE hReceiver;
	ETAL_HANDLE monitor_dab_handle;
	ETAL_STATUS ret;

	EtalEnsembleList ens_list;
	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tU8 charset;
	char label[17];
	tU16 bitmap;
	unsigned int ueid;
	int i;
	tBool vl_selectedSidPresent = false;
	char line[256];
	tU32 vl_Sid = 1;
	EtalProcessingFeatures processingFeatures;

	/*
	 * Create a receiver configuration
	 */
	etalDemoPrintf("*************** DAB RADIO TUNING ***************\n");
		
	v_monitor_dab_handle = ETAL_INVALID_HANDLE;
	v_handle_dab = ETAL_INVALID_HANDLE;
	

	hReceiver = ETAL_INVALID_HANDLE;
	etalDemoPrintf("Create DAB receiver\n");
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));
	attr.m_Standard = ETAL_BCAST_STD_DAB;
#ifndef CONFIG_MODULE_INTEGRATED
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1;
#else
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_3;
#endif // !CONFIG_MODULE_INTEGRATED
	attr.m_FrontEndsSize = 1;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_config_receiver dab (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

    /* Select audio source */
	if ((ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_audio_select dab (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

	/*
	 * Tune to a DAB ensemble
	 */
	// set band
	etalDemoPrintf("set DABband\n");
	
	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(hReceiver, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("etal_change_band_receiver DAB ERROR\n");
		return ETAL_DEMO_RETURN_ERROR;
	}
	else
	{
		etalDemoPrintf("etal_change_band_receiver DAB  ok\n");
	}

	etalDemoPrintf("Tune to DAB freq %d\n", vI_freq);
	if (((ret = etal_tune_receiver(hReceiver, vI_freq)) != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
	{
		etalDemoPrintf("ERROR: etal_tune_receiver DAB (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}
	else if (ret == ETAL_RET_NO_DATA)
	{
		etalDemoPrintf("WARNING: etal_tune_receiver DAB (%d)\n", ret);
	}


	/*
	 * Unlike FM, for DAB we need to allow the tuner some time
	 * to capture and decode the ensemble information
	 * Without this information the tuner is unable to perform
	 * any operation on the ensemble except quality measure
	 */
	 
	Sleep(USLEEP_ONE_SEC);


	/*
	 * Get the Ensemble list; this is necessary to learn the unique Ensemble ID which
	 * is required by many DAB commands. The etal_get_ensemble_list actually
	 * returns an array containing all the currently known emsembles (also
	 * on frequencies different from the current one) but for simplicity
	 * we assume there is only one entry
	 * 
	 * For a more realistic approach we should parse the list
	 * and take the entry with m_frequency equal to the currently tuned
	 * frequency
	 */
	etalDemoPrintf("Reading the ensemble list\n");
	memset(&ens_list, 0x00, sizeof(ens_list));
	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_get_ensemble_list (%d)\n", ret);
		if (ret == ETAL_RET_NO_DATA)
		{
			etalDemoPrintf("Could be because no DAB broadcasting on frequency %d kHz\n", vI_freq);
		}
		return ETAL_DEMO_RETURN_ERROR;
	}

	/*
	 * Ensembles are uniquely identified by the 'Unique EnsembleId (UEId)' which is a combination
	 * of the m_ensembleId and the m_ECC, as shown here
	 */
	ueid = ens_list.m_ensemble[0].m_ECC << 16 | ens_list.m_ensemble[0].m_ensembleId;

	/*
	 * Now that we know the UEId let's get the ensemble label; this is not required
	 * but nicer to show rather than the ensemble frequency
	 */
	if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_get_ensemble_data (%d)", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}
	etalDemoPrintf("The ensemble UEID is: %x\n", ueid);
	etalDemoPrintf("The ensemble label is: %s\n", label);

	/*
	 * A DAB ensemble contains one or more services: fetch the list
	 * and present it to the user for selection. Some services will be
	 * audio only, others data only. The ETAL interface permits to select
	 * which service list to fetch, here we fetch only the audio services
	 */
	if ((ret = etal_get_service_list(hReceiver, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_get_service_list (%d)", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

	// in early audio avoid getting all information
	//

	if (false == earlyAudio)
	{
		/*
		 * etal_get_service_list returns a list of numerical ids, not very useful
		 * to show; so now for every service we read the service label
		 * and present it
		 */
		for (i = 0; i < serv_list.m_serviceCount; i++)
		{
			memset(&svinfo, 0, sizeof(EtalServiceInfo));
			memset(&scinfo, 0, sizeof(EtalServiceComponentList));
			if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf("WARNING: etal_get_specific_service_data_DAB (%d)\n", ret);
			}
			etalDemoPrintf("\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);

			if (vI_ServiceIndex == (i+1))
			{
				vl_selectedSidPresent = true;
				vl_Sid = serv_list.m_service[i];
			}
		}
	}
	else
	{
		vl_selectedSidPresent = true;
		vl_Sid = serv_list.m_service[vI_ServiceIndex-1];
	}

	/*
	 * Now the user could select the service
	 * We do it for him
	 */
#ifndef CONFIG_HOST_OS_TKERNEL

	if (0 == vI_duration)
	{
		if (false == vl_selectedSidPresent)
		{
			etalDemoPrintf("*** choose the service to select in the list : --> ");
		
			if (fgets(line, sizeof(line), stdin))
			{
				if (1 == sscanf(line, "%d", &i))
				{
					if ((i>0) && (i<=serv_list.m_serviceCount))
					/* i can be safely used */
					vl_Sid = serv_list.m_service[i-1];
				}
			}
		}

	}
	else
	{
		if (false == vl_selectedSidPresent)
		{
			// set serivce 0
			vl_Sid = serv_list.m_service[0];
		}
	}
#endif // #ifdef CONFIG_HOST_OS_TKERNEL
	/*
	 * This is the DAB equivalent of tuning to an FM station.
	 * Audio may be available after this step, depending on the CMOST CUT:
	 * - CUT 1.0 no audio
	 * - CUT 2.x audio
	 * See above for hints on how to find out the CUT version.
	 */
	etalDemoPrintf("Selecting a service from the ensemble, second pass, service 0x%x\n", vl_Sid);
	if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ueid, vl_Sid, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_service_select (%d)", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}
	
	/*
	 * Read the signal strength, one shot
	 */

	etalDemoPrintf("GetQuality for DAB\n");
	if ((ret = etal_get_reception_quality(hReceiver, &qualdab)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_get_reception_quality for DAB (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}
	etalDemoPrintf("DAB signal strength = %d\n", qualdab.EtalQualityEntries.dab.m_BBFieldStrength);

	/*
	 * Set up a quality monitor
	 * We want to be notified only when the DAB Signal strength is higher than 10.0,
	 * and only every 1000ms
	 */

	etalDemoPrintf("Creating DAB monitor\n");
	monitor_dab_handle = ETAL_INVALID_HANDLE;
	memset(&mondab, 0x00, sizeof(EtalBcastQualityMonitorAttr));
	mondab.m_receiverHandle = hReceiver;
	mondab.m_CbBcastQualityProcess = etalTestDABMonitorcb;
	mondab.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_DabFieldStrength;
	mondab.m_monitoredIndicators[0].m_InferiorValue = ETAL_INVALID_MONITOR;
	mondab.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	mondab.m_monitoredIndicators[0].m_UpdateFrequency = ETAL_DEMO_QUALITY_PERIODICITY;  // millisec
	if ((ret = etal_config_reception_quality_monitor(&monitor_dab_handle, &mondab)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_config_reception_quality_monitor for DAB (%d)", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}


	/*
	 * Listen to some radio
	 */


	etalDemoPrintf("*************** DAB RADIO TUNING DONE ***************\n");
	
	/*
	 * Demo end
	 */
	// not in auto - mode : wait user to end test
	
	if (0 ==  vI_duration)
	{
				
		etalDemoPrintf("*************** ENTER A KEY TO END TEST ***************\n");
		
		fgets(line, sizeof(line), stdin);
	}
	else if (0xFFFFFFFF != vI_duration)
		{
		/*
		 * Listen to some radio
		 */
		
		Sleep(vI_duration * USLEEP_ONE_SEC); // 6 seconds
		}
		


	if (0xFFFFFFFF != vI_duration)
	{
		etalDemoPrintf("\n\n***************  END OF TEST ***************\n\n");


		/*
		 * Destroy quality monitor
		 */

		if ((ret = etal_destroy_reception_quality_monitor(&monitor_dab_handle)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_reception_quality_monitor for DAB (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}

		/*
		 * Destroy all receivers
		 */

		etalDemoPrintf("Destroy DAB receiver\n");
		if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_receiver DAB (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}
	}
	else
	{
		v_monitor_dab_handle = monitor_dab_handle;
		v_handle_dab = hReceiver;
	}
	
	return ETAL_DEMO_RETURN_OK;
}
#endif // HAVE_DAB

#ifdef HAVE_HD
/***************************
 *
 * HDRadio
 *
 **************************/
 static void etalTestHDMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	etalDemoPrintf("Quality callback: HD analog BB signal strength is %d (dBuV)\n", pQuality->EtalQualityEntries.hd.m_analogQualityEntries.m_BBFieldStrength);
	etalDemoPrintf("Quality callback: HD analog RF signal strength is %d (dBuV)\n", pQuality->EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength);
}

static int HDRadio(tU32 vI_freq,  tU32 vI_duration)
{
	EtalReceiverAttr attr;

	ETAL_HANDLE handlehd;
	ETAL_STATUS ret;
	char line[256];
	EtalBcastQualityMonitorAttr monHD;
	ETAL_HANDLE monitor_hd_handle;

	/*
	 * Create a receiver configuration
	 */
	etalDemoPrintf("*************** HD RADIO TUNING ***************\n");
	

	/*
	 * Create a receiver configuration
	 */
	v_monitor_hd_handle = ETAL_INVALID_HANDLE;
	v_handle_hd = ETAL_INVALID_HANDLE;

	handlehd = ETAL_INVALID_HANDLE;
	etalDemoPrintf("Create hd receiver\n");
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));
	if (useHD_AM)
	{
		attr.m_Standard = ETAL_BCAST_STD_HD_AM;
	}
	else
	{
		attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	}
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1;
	attr.m_FrontEndsSize = 1;
	attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;

	if ((ret = etal_config_receiver(&handlehd, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_config_receiver hd (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

    /* Select audio source */
	if ((ret = etal_audio_select(handlehd, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_audio_select hd (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

	if (1 == 	disable_AAA)
	{
		EtalAudioAlignmentAttr alignmentParams;
		alignmentParams.m_enableAutoAlignmentForAM = false;
		alignmentParams.m_enableAutoAlignmentForFM = false;
		
		etal_debug_config_audio_alignment( &alignmentParams);
	}


	/*
	 * Tune to the FM frequency for HD
	 */

	etalDemoPrintf("Tune to HD freq %d\n", vI_freq);
	if ((ret = etal_tune_receiver_async(handlehd, vI_freq)) != ETAL_RET_SUCCESS)
//	if ((ret = etal_tune_receiver(handlehd, vI_freq)) != ETAL_RET_SUCCESS)

	{
		etalDemoPrintf("ERROR: etal_tune_receiver HD (%d)\n", ret);
//		return ETAL_DEMO_RETURN_ERROR;
	}

	/*
	 * Listen to some radio
	 */


	etalDemoPrintf("*************** HD RADIO TUNING DONE ***************\n");

	etalDemoPrintf("Creating HD monitor\n");
	monitor_hd_handle = ETAL_INVALID_HANDLE;
	memset(&monHD, 0x00, sizeof(EtalBcastQualityMonitorAttr));
	monHD.m_receiverHandle = handlehd;
	monHD.m_CbBcastQualityProcess = etalTestHDMonitorcb;
	monHD.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_HdCdToNo;
	monHD.m_monitoredIndicators[0].m_InferiorValue = ETAL_INVALID_MONITOR;
	monHD.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	monHD.m_monitoredIndicators[0].m_UpdateFrequency = ETAL_DEMO_QUALITY_PERIODICITY;  // millisec
	if ((ret = etal_config_reception_quality_monitor(&monitor_hd_handle, &monHD)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_config_reception_quality_monitor for HD (%d)", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}


			
	if (0 ==  vI_duration)
	{
				
		etalDemoPrintf("*************** ENTER A KEY TO END TEST ***************\n");
		
		fgets(line, sizeof(line), stdin);
	}
	else if (0xFFFFFFFF != vI_duration)
	{
		/*
		 * Listen to some radio
		 */
		
		//Sleep(60 * USLEEP_ONE_SEC); // 60 seconds - for some chance to capture also PAD data
		Sleep(vI_duration * USLEEP_ONE_SEC);
	}
		

	/*
	 * Destroy all receivers
	 */
	if (0xFFFFFFFF != vI_duration)
	{
		etalDemoPrintf("Destroy hd receiver\n");
		if ((ret = etal_destroy_receiver(&handlehd)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_receiver HD (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}
	}
	else
	{
		v_monitor_hd_handle = monitor_hd_handle;
		v_handle_hd = handlehd;
	}

	return ETAL_DEMO_RETURN_OK;
}
#endif // HAVE_HD





/***************************
 *
 * userNotificationHandler
 *
 **************************/
static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
	if (dwEvent == ETAL_INFO_TUNE)
	{
		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		etalDemoPrintf("Tune info event, Frequency %d, good station found %d\n", status->m_stopFrequency, (status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND);
#ifdef HAVE_HD
		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_SYNC)
		{
			etalDemoPrintf("HD signal found\n");
		}
		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC)
		{
			etalDemoPrintf("HD digital audio aquired\n");
		}
#endif
	}
	else
	{
		etalDemoPrintf("Unexpected event %d\n", dwEvent);
	}
}

#ifndef CONFIG_HOST_OS_TKERNEL

/***************************
 *
 * etalDemo_PrintHelpAndExit
 *
 **************************/
static int etalDemo_PrintHelpAndExit(char **argv)
{
	etalDemoPrintf("Usage: %s [f|a|d|hf|ha|hfd|had][F|A|D] [e|E|n|d] <freq> [n|v] <service_index>\n", argv[0]);
	etalDemoPrintf("-------------- [f|a|d|hf|ha|hfd|had][F|A|D] ----------\n");
#ifdef HAVE_FM
	etalDemoPrintf("\tf = select FM radio (default)\n");
	etalDemoPrintf("\ta = select AM radio (default)\n");
#else
	etalDemoPrintf("\tf = select FM radio - not supported\n");
	etalDemoPrintf("\ta = select AM radio - not supported\n");
#endif

#ifdef HAVE_DAB
	etalDemoPrintf("\td = select DAB radio\n");
#else
	etalDemoPrintf("\td = select DAB radio - not supported\n");
#endif

#ifdef HAVE_HD
	etalDemoPrintf("\thf = select HD radio FM\n");
	etalDemoPrintf("\tha = select HD radio AM\n");
	etalDemoPrintf("\thfd = select HD radio FM force disabling AAA\n");
	etalDemoPrintf("\thad = select HD radio AM force disabling AAA\n");
#else
	etalDemoPrintf("\thf = select HD radio - not supported\n");
	etalDemoPrintf("\tha = select HD radio - not supported\n");
#endif
	etalDemoPrintf("\tupper case letter = use digital audio output\n");
	etalDemoPrintf("\tlower case letter = use analog  audio output\n");

	etalDemoPrintf("------------- [n|e|E|d] ----------\n");

	etalDemoPrintf("\tn = indicate to start in normal mode booting CMOST & DCOP\n");
	etalDemoPrintf("\te = indicate to start in earlyAudio mode, with tuner reset & loading\n");
	etalDemoPrintf("\tE = indicate to start in earlyAudio mode, tuners already started\n");
	etalDemoPrintf("\td = indicate to start in earlyAudio mode, DCOP & tuner already started\n");
	
	etalDemoPrintf("------------- <freq> <service_index> ----------\n");

	etalDemoPrintf("\t<freq> = frequency to be tuned, if not set default used\n");

#ifdef HAVE_DAB
	etalDemoPrintf("\tn = for DAB : DCOP FW already \n");
	etalDemoPrintf("\tv = for DAB : DCOP has no NVM ; FW must be loaded\n");
	etalDemoPrintf("\t<service_index> = for DAB : Service Index in the list to be tuned, if not set default used\n");
#endif
	etalDemoPrintf("------------- ----- ----------\n");


	return ETAL_DEMO_RETURN_ERROR;
}

#endif // #ifdef CONFIG_HOST_OS_TKERNEL
/***************************
 *
 * parseParameters
 *
 **************************/
 
#ifdef CONFIG_HOST_OS_TKERNEL
static int etalDemo_parseParameters(int vI_choice, tU32 freq, tU32 Sid, tU32 vI_channel)
{
#define	ETALCOREDEMO_STD_CHOICE_FM		1
#define	ETALCOREDEMO_STD_CHOICE_AM		2
#define	ETALCOREDEMO_STD_CHOICE_DAB		3
#define	ETALCOREDEMO_STD_CHOICE_HD_FM	4
#define	ETALCOREDEMO_STD_CHOICE_HD_AM	5
#define	ETALCOREDEMO_STD_CHOICE_HD_FM_AAA_DISABLED	6
#define	ETALCOREDEMO_STD_CHOICE_HD_AM_AAA_DISABLED	7

		//argument 1 : choose the techno
		switch (vI_choice)
		{
			case ETALCOREDEMO_STD_CHOICE_FM:
				useFM = 1;
				useAM = 0;
				useDAB = 0;
				useHD_FM = 0;
				useHD_AM = 0;
	
				freq_amfm = freq;
				break;
			case ETALCOREDEMO_STD_CHOICE_AM:
				useFM = 0;
				useAM = 1;
				useDAB = 0;
				useHD_FM = 0;
				useHD_AM = 0;
	
				freq_amfm = freq;
				break;
			case ETALCOREDEMO_STD_CHOICE_DAB:
				useFM = 0;
				useAM = 0;
				useDAB = 1;
				useHD_FM = 0;
				useHD_AM = 0;
				freq_dab = freq;
				break;	
			case ETALCOREDEMO_STD_CHOICE_HD_FM:
				useFM = 0;
				useAM = 0;
				useDAB = 0;
				useHD_FM = 1;
				useHD_AM = 0;
				
				freq_HD = freq;
				break;
			case ETALCOREDEMO_STD_CHOICE_HD_AM:
				useFM = 0;
				useAM = 0;
				useDAB = 0;
				useHD_FM = 0;
				useHD_AM = 1;
				freq_HD = freq;
				break;
			case ETALCOREDEMO_STD_CHOICE_HD_FM_AAA_DISABLED:
				useFM = 0;
				useAM = 0;
				useDAB = 0;
				useHD_FM = 1;
				useHD_AM = 0;
				disable_AAA = true;
				freq_HD = freq;
				break;
			case ETALCOREDEMO_STD_CHOICE_HD_AM_AAA_DISABLED:
				useFM = 0;
				useAM = 0;
				useDAB = 0;
				useHD_FM = 0;
				useHD_AM = 1;
				disable_AAA = true;
				
				freq_HD = freq;
				break;
			default:
				break;	
		}


	// Select the FE
	switch (vI_channel)
	{
		case 1:
			v_HandleFE = ETAL_FE_HANDLE_1;
			break;
		case 2:
			v_HandleFE = ETAL_FE_HANDLE_2;
			break;
		case 3:
			v_HandleFE = ETAL_FE_HANDLE_3;
			break;
		case 4: 
			v_HandleFE = ETAL_FE_HANDLE_4;
			break;
		default:
			v_HandleFE = ETAL_FE_HANDLE_1;
			break;
	}

	return ETAL_DEMO_RETURN_ERROR;

}
#else
static int etalDemo_parseParameters(int argc, char **argv)
{
#define ARGV_NUM_STD		1
#define ARGV_EARLY_AUDIO	2
#define ARGV_NUM_FREQ		3
#define ARGV_FW_LOAD        4
#define ARGV_NUM_SID		5
#ifndef HAVE_DAB
#define MIN_ARGC 			(ARGV_NUM_FREQ+1)
#else
#define MIN_ARGC 			(ARGV_FW_LOAD+1)
#endif
#define NB_ARGC_WITH_SID	(ARGV_NUM_SID+1)

	// no default but the help
#if 0
	if (argc == 1)
	{
#ifdef HAVE_FM
		useFM = 1;
		useAM = 0;
		useDAB = 0;
		useHD_FM = 0;
		useHD_AM = 0;
		freq_amfm = ETAL_VALID_FM_FREQ;
		etalDemoPrintf("\t-->  Default selection : FM radio , freq = %d\n", freq_amfm);
#elif defined (HAVE_DAB)
		useFM = 0;
		useAM = 0;
		useDAB = 1;
		useHD_FM = 0;
		useHD_AM = 0;
		freq_dab = ETAL_VALID_DAB_FREQ;
		Sid = ETAL_DAB_ETI_SERV1_SID;
		etalDemoPrintf("\t-->  Default selection: DAB radio, freq = %d, Sid = 0x%x\n", freq_dab, Sid);
#elif defined (HAVE_HD)
		useFM = 0;
		useAM = 0;
		useDAB = 0;
		useHD_FM = 1;
		useHD_AM = 0;
		freq_HD = ETAL_VALID_HD_FREQ;
		etalDemoPrintf("\t-->  Default selection: HD radio, freq = %d\n", freq_HD);
#else
		etalDemoPrintf("No radio compiled in\n"); 
		return ETAL_DEMO_RETURN_ERROR;
#endif
		digitalOut = 0;

		return ETAL_DEMO_RETURN_OK;
	}
	else
#endif // if 0
	if (argc >= MIN_ARGC)
	{
#ifdef HAVE_DAB
		if (argv[ARGV_FW_LOAD][0] == 'v')
		{
		    DCOP_is_NVMless = true;
		}
#endif
		if (argv[ARGV_EARLY_AUDIO][0] == 'e')
		{
			// we are in early audio, tuner not started
			earlyAudio = true;
			tuner1AlreadyStarted = false;
		}
		else if (argv[ARGV_EARLY_AUDIO][0] == 'E')
		{
			// we are in early audio, tuner started
			earlyAudio = true;
			tuner1AlreadyStarted = true;
			tuner2AlreadyStarted = true;
			DCOPAlreadyStarted = false;
		}
		else if (argv[ARGV_EARLY_AUDIO][0] == 'd')
		{
			// we are in early audio, tuner started
			earlyAudio = true;
			tuner1AlreadyStarted = true;
			DCOPAlreadyStarted = true;
			tuner2AlreadyStarted = true;
		}
		else
		{
			earlyAudio = false;
			tuner1AlreadyStarted = false;
		}
		etalDemoPrintf("\t-->  Early audio config = %d, Tuner 1 started = %d, Tuner 2 started = %d, DCOP %d\n", 
			earlyAudio, tuner1AlreadyStarted, tuner2AlreadyStarted, DCOPAlreadyStarted );
		
		if ((argv[ARGV_NUM_STD][0] == 'f') || (argv[ARGV_NUM_STD][0] == 'F'))
		{
#ifndef HAVE_FM
			etalDemoPrintf("FM radio not compiled in\n");
			return etalDemo_PrintHelpAndExit(argv);
#else
			useFM = 1;
			useAM = 0;
			useDAB = 0;
			useHD_FM = 0;
			digitalOut = 0;
			if (argv[ARGV_NUM_STD][0] == 'F')
			{
				digitalOut = 1;
			}

			freq_amfm = atoi(argv[ARGV_NUM_FREQ]);

			v_HandleFE = ETAL_FE_HANDLE_1;

			etalDemoPrintf("\t-->  FM radio selection, freq = %d\n", freq_amfm);

			goto success;
#endif
		}
		else if ((argv[ARGV_NUM_STD][0] == 'a') || (argv[ARGV_NUM_STD][0] == 'A'))
		{
#ifndef HAVE_FM
			etalDemoPrintf("AM radio not compiled in\n");
			return etalDemo_PrintHelpAndExit(argv);
#else
			v_HandleFE = ETAL_FE_HANDLE_1;

			useFM = 0;
			useAM = 1;
			useDAB = 0;
			useHD_FM = 0;
			digitalOut = 0;
			if (argv[ARGV_NUM_STD][0] == 'A')
			{
				digitalOut = 1;
			}

			freq_amfm = atoi(argv[ARGV_NUM_FREQ]);

			etalDemoPrintf("\t-->  AM radio selection, freq = %d\n", freq_amfm);
			
            goto success;
#endif
		}
		else if ((argv[ARGV_NUM_STD][0] == 'd') || (argv[ARGV_NUM_STD][0] == 'D'))
		{
#ifndef HAVE_DAB
			etalDemoPrintf("DAB radio not compiled in\n");
			return etalDemo_PrintHelpAndExit(argv);
#else
			useFM = 0;
			useDAB = 1;
			useHD_FM = 0;
			digitalOut = 0;
			if (argv[ARGV_NUM_STD][0] == 'D')
			{
				digitalOut = 1;
			}
			
			freq_dab = atoi(argv[ARGV_NUM_FREQ]);

			if (argc >= NB_ARGC_WITH_SID)
			{
				sscanf(argv[ARGV_NUM_SID], "%d", &v_ServiceNumber);
			}
			else
			{
				v_ServiceNumber= ETAL_DAB_SERVICE_INDEX_DEFAULT;
			}

			etalDemoPrintf("\t-->  DAB radio selection, freq = %d, Service Index = 0x%x\n", freq_dab, v_ServiceNumber);
	
			goto success;
#endif
		}
		else if ((0 == strcmp(argv[ARGV_NUM_STD],"hf")) || (0 == strcmp(argv[ARGV_NUM_STD],"hfd")))
		{
#ifndef HAVE_HD
			etalDemoPrintf("HD radio not compiled in\n");
			return etalDemo_PrintHelpAndExit(argv);
#else
			useFM = 0;
			useAM = 0;
			useDAB = 0;
			useHD_FM = 1;
			useHD_AM = 0;
			digitalOut = 0;

			freq_HD= atoi(argv[ARGV_NUM_FREQ]);

			
			if (0 == strcmp(argv[ARGV_NUM_STD],"hfd"))
			{
				disable_AAA = true;
			}


			etalDemoPrintf("\t-->  HD FM radio selection, freq = %d\n", freq_HD);

			goto success;
#endif
		}
		else if ((0 == strcmp(argv[ARGV_NUM_STD],"ha")) || (0 == strcmp(argv[ARGV_NUM_STD],"had")))
		{
#ifndef HAVE_HD
			etalDemoPrintf("HD radio not compiled in\n");
			return etalDemo_PrintHelpAndExit(argv);
#else
			useFM = 0;
			useDAB = 0;
			useHD_FM = 0;
			useHD_AM = 1;
			digitalOut = 0;

			if (0 == strcmp(argv[ARGV_NUM_STD],"had"))
			{
				disable_AAA = true;
			}

			freq_HD= atoi(argv[ARGV_NUM_FREQ]);


			etalDemoPrintf("\t-->  HD AM radio selection, freq = %d\n", freq_HD);

			goto success;
#endif
		}
	}
	return etalDemo_PrintHelpAndExit(argv);

success:
	return ETAL_DEMO_RETURN_OK;
}
#endif

/***************************
 *
 * main
 *
 **************************/
/*
 * Returns:
 * 1 error(s)
 * 0 success
 */
#ifdef CONFIG_HOST_OS_TKERNEL
	int etalcoreDemo_EntryPoint(int vI_choice, tU32 freq, tU32 vI_ServiceIndex, tU32 vI_duration, tBool vl_digitalOutput, tU32 vI_channel)
#else
	int main(int argc, char **argv)
#endif

{
	int ret;
	EtalHardwareAttr init_params;
    EtalAudioInterfTy audioIf;
	tU32 vl_duration = 0xFFFFFFFF;
#ifdef CONFIG_COMM_CMOST_FIRMWARE_IMAGE
	tU32 i;
#endif
#if defined(HAVE_DAB) || defined(HAVE_HD)
	EtalDcopInitTypeEnum vl_DcopInitType;
#endif

#ifdef CONFIG_HOST_OS_TKERNEL
	etalDemo_parseParameters(vI_choice, freq, vI_ServiceIndex, vI_channel);

	digitalOut = vl_digitalOutput;
	
	vl_duration =	vI_duration;
	v_ServiceNumber = vI_ServiceIndex;

#else
	if (etalDemo_parseParameters(argc, argv))
	{
		return ETAL_DEMO_RETURN_ERROR;
	}
#endif

	// start by configuring the audios

//	if ((false == earlyAudio) || (useDAB))
	if (false == earlyAudio)
	{
		// we assume the audio is already configured
		etalDemo_ConfigureAudioForTuner();
	}
	
	/*
	 * Initialize ETAL
	 */
	etalDemoPrintf("Initializing ETAL\n");
	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = etalDemo_userNotificationHandler;
#if !defined(HAVE_DAB) && !defined(HAVE_HD) && !defined(HAVE_HD_AM)
	init_params.m_DCOPAttr.m_isDisabled = TRUE;
#endif // !HAVE_DAB && !HAVE_HD && !HAVE_HD_AM
#ifndef CONFIG_MODULE_INTEGRATED
	init_params.m_tunerAttr[1].m_isDisabled = TRUE;
#else
	init_params.m_tunerAttr[1].m_DownloadImageSize = downloadImage2Size;
	init_params.m_tunerAttr[1].m_DownloadImage = downloadImage2;
#endif
	init_params.m_tunerAttr[0].m_DownloadImageSize = downloadImage1Size;
	init_params.m_tunerAttr[0].m_DownloadImage = downloadImage1;
#ifdef CONFIG_COMM_CMOST_FIRMWARE_IMAGE
	for(i = 0; i < 2; i++)
	{
		 if ((init_params.m_tunerAttr[i].m_DownloadImageSize != 0) && 
		 	 (init_params.m_tunerAttr[i].m_DownloadImage != 0))
		{
			 init_params.m_tunerAttr[i].m_useDownloadImage = TRUE;
		}
	}
#endif

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
    if (true == DCOP_is_NVMless)
   	{
    	init_params.m_DCOPAttr.m_doDownload = TRUE;
		init_params.m_DCOPAttr.m_cbGetImage = etalDemo_GetDCOPImageCb;
		init_params.m_DCOPAttr.m_sectDescrFilename = ETALDcop_sections_filename;
   	}
#endif

	// Step init : 
	// 1st step : init ETAL
	// Then tuner 1
	// Then Tuner 2
	// then DCOP
	if (true == earlyAudio)
	{
		init_params.m_tunerAttr[0].m_isDisabled = TRUE;
		init_params.m_tunerAttr[1].m_isDisabled = TRUE;
		init_params.m_DCOPAttr.m_isDisabled = TRUE;
	}

	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_initialize (%d)\n", ret);
		return ETAL_DEMO_RETURN_ERROR;
	}

	// boot 1st tuner
	if (true == earlyAudio)
	{
		init_params.m_tunerAttr[0].m_isDisabled = FALSE;

		if ((ret = etal_tuner_initialize(0, &init_params.m_tunerAttr[0], tuner1AlreadyStarted)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_tuner_initialize (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}

		tuner1AlreadyStarted = true;
	}
	
    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));

	if (true == digitalOut)
	{
		audioIf.m_dac = false;
	}
	else
	{
		audioIf.m_dac = true;
	}

	audioIf.m_sai_in  = true;
#ifdef HAVE_HD
	audioIf.m_sai_out = true;
#else
	audioIf.m_sai_out  = true;
#endif

#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = 1;
#else
	audioIf.m_sai_slave_mode = 0;
#endif

	// Audio path is supposed to be correctly set by ETAL at init
#if 0
   if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
    {
        etalDemoPrintf("etal_config_audio_path (%d)", ret);
        return ETAL_DEMO_RETURN_ERROR;
    }
#endif 


	
	if ((useFM) || (useAM))
	{
#ifdef HAVE_FM
		ret = AMFMRadio(freq_amfm, vl_duration);
#endif
	}

#ifdef CONFIG_MODULE_INTEGRATED
	// boot 2nd Tuner & DCOP only in case required:
	// if in DCOP or if starting with tuner 1 & tuner 2
	//
	if ((true == tuner1AlreadyStarted) 
		 ||
		 ((useDAB) || ((useHD_FM) || (useHD_AM))))
	{
		if (true == earlyAudio)
		{
			init_params.m_tunerAttr[1].m_isDisabled = FALSE;
			// tuner1AlreadyStarted applies only for Tuner 1 
			
			if ((ret = etal_tuner_initialize(1, &init_params.m_tunerAttr[1], tuner2AlreadyStarted)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf("ERROR: etal_tuner_initialize (%d)\n", ret);
				return ETAL_DEMO_RETURN_ERROR;
			}
			tuner2AlreadyStarted = true;
		}
	}

#endif

	
#if defined(HAVE_DAB) || defined(HAVE_HD)
	// In Early audio, save time and boot DCOP
	// Boot type depends on configuration.
	//
	if ((true == earlyAudio)
#ifdef CONFIG_MODULE_INTEGRATED		
		&& (true == tuner2AlreadyStarted)
#endif
		)
	{
		if (true == DCOPAlreadyStarted)
		{
			vl_DcopInitType = ETAL_DCOP_INIT_ALREADY_STARTED;
		}
		else
		{
			if ((useDAB) || ((useHD_FM) || (useHD_AM)))
			{
				vl_DcopInitType = ETAL_DCOP_INIT_FULL;
			}
			else
			{
#ifdef HAVE_DAB
				vl_DcopInitType = ETAL_DCOP_INIT_RESET_ONLY;
#else
				vl_DcopInitType = ETAL_DCOP_INIT_FULL;
#endif
			}
		}
		
		init_params.m_DCOPAttr.m_isDisabled = FALSE;
		 
		if ((ret = etal_dcop_initialize(&init_params.m_DCOPAttr, vl_DcopInitType)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_tuner_initialize (%d)\n", ret);
			return ETAL_DEMO_RETURN_ERROR;
		}		
	}
#endif

	if (useDAB)
	{


#ifdef HAVE_DAB
		ret = DABRadio(freq_dab, v_ServiceNumber, vl_duration);
#endif
	}
	else if ((useHD_FM) || (useHD_AM))
	{
#ifdef HAVE_HD
		ret = HDRadio(freq_HD, vl_duration);
#endif
	}

	if (0xFFFFFFFF != vl_duration)
	{
		/*
		 * Final cleanup
		 */

		if (etal_deinitialize() != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_deinitialize\n");
		}

		
		etalDemoPrintf("Demo ended\n");
	}
	
	return 0;
}

// Configure the audio path for Tuner
//
static void etalDemo_ConfigureAudioForTuner()
{
	// this is based on alsa mixer in Linux
	// This requires the appropriate audio fwk

	// ETAL audio select has been modified to support digital audio configuration,
	// so no need to care about digital audio or band choice anymore
	

#ifdef CONFIG_BOARD_ACCORDO5
#ifndef CONFIG_DIGITAL_AUDIO
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif

		system("amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");
		system("echo amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");


#endif

#ifdef CONFIG_BOARD_ACCORDO2
	// select the audio source
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
#ifndef CONFIG_HOST_OS_WIN32
	// Set the output volume 

		system("amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");
		system("echo amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");

#endif // CONFIG_HOST_OS_WIN32
}


#ifdef CONFIG_HOST_OS_TKERNEL
void etalcoreDemo_EndPoint()
{
	ETAL_STATUS ret;

	/*
	 * Final cleanup
	 */
	if ((useFM) || (useAM))
	{
#ifdef HAVE_FM
		/*
		 * Destroy quality monitor
		 */

		if ((ret = etal_destroy_reception_quality_monitor(&v_monitor_amfm_handle)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_reception_quality_monitor for fm (%d)\n", ret);
			return ;
		}

		/*
		 * Destroy all receivers
		 */
		 
		etalDemoPrintf("Destroy fm receiver\n");
		if ((ret = etal_destroy_receiver(&v_handle_amfm)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_receiver fm (%d)\n", ret);
			return ;
		}
#endif
	}
	else if (useDAB)
	{
#ifdef HAVE_DAB

		/*
		 * Destroy quality monitor
		 */

		if ((ret = etal_destroy_reception_quality_monitor(&v_monitor_dab_handle)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_reception_quality_monitor for DAB (%d)\n", ret);
			return ;
		}

		/*
		 * Destroy all receivers
		 */

		etalDemoPrintf("Destroy DAB receiver\n");
		if ((ret = etal_destroy_receiver(&v_handle_dab)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_receiver DAB (%d)\n", ret);
			return ;
		}
#endif
	}
	else if ((useHD_FM) || (useHD_AM))
	{
#ifdef HAVE_HD
		etalDemoPrintf("Destroy hd receiver\n");
		/*
		 * Destroy quality monitor
		 */

		if ((ret = etal_destroy_reception_quality_monitor(&v_monitor_hd_handle)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_reception_quality_monitor for HD (%d)\n", ret);
			return ;
		}

		if ((ret = etal_destroy_receiver(&v_handle_hd)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf("ERROR: etal_destroy_receiver HD (%d)\n", ret);
			return ;
		};
#endif
	}

	
	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_deinitialize\n");
	}

	
	etalDemoPrintf("Demo ended\n");
	return ;
}

#endif

