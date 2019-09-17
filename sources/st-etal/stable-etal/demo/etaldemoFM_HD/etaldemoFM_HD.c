//!
//!  \file 		etaldemoFM_HD.c
//!  \brief 	<i><b> ETAL application demo </b></i>
//!  \details   Demo ETAL user application, simulates a radio with AM/FM and HDRadio or DAB
//!  \author 	Raffaele Belardi
//!

/*
 * Implements a demo ETAL API user function with two purposes:
 * 1. verify that lib_etal/exports/ contains all is needed to link
 *    with the ETAL library
 * 2. provide an example usage of the ETAL library.
 *
 * The demo is coded for DAB, AM, FM and HD system based on CMOSTuner (TDA7707)
 * and STA680 or STA660 for the Linux/Accordo2 platform.
 * Note that in such system the HDRadio audio decoded by the STA680
 * is routed back to the CMOST for conversion to analog and
 * output on the same pins as the analog FM signal.
 */

#include "target_config.h"

#include "etal_api.h"
#include "etaltml_api.h"


#include <stdio.h>
#include <string.h>
#ifdef CONFIG_HOST_OS_WIN32
	#include <windows.h>
#else
	#include <unistd.h>  // usleep
#endif

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
//#define HAVE_FM
//#define HAVE_DAB // not compatible with HD
#define HAVE_HD_AM  // not compatible with DAB
//#define HAVE_HD_FM  // not compatible with DAB
//#define HAVE_AM

#define HAVE_RADIOTEXT
#define HAVE_QUALITYMONITOR

typedef enum
{
	DAB_MODE,
	FM_MODE,
	HD_FM_MODE,
	HD_AM_MODE,
	AM_MODE
} radioModeTy;

#define NUMBER_OF_CYCLES           10
#define SECONDS_PER_CYCLE          10

/*
 * Adjust ETAL_VALID_*_FREQ to some known good FM/HD station possibly with RDS
 */
#define ETAL_VALID_FM_FREQ     104800
#define ETAL_VALID_HD_FREQ      88700
#define ETAL_VALID_AM_FREQ        720
#define ETAL_VALID_HD_AM_FREQ     711

/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB_FREQ    225648
#define ETAL_DAB_ETI_UEID      0xE0106E
#define ETAL_DAB_ETI_SERV1_SID 0x1D12

#ifdef CONFIG_HOST_OS_WIN32
	#define USLEEP_ONE_SEC        1000
#elif CONFIG_HOST_OS_TKERNEL
	#define USLEEP_ONE_SEC        1000
	#define Sleep				  tk_dly_tsk
#else
	#define Sleep                 usleep
	#define USLEEP_ONE_SEC        1000000
#endif

#define FRONTEND_FOR_FM  ETAL_FE_HANDLE_1
#define FRONTEND_FOR_AM  ETAL_FE_HANDLE_1
#define FRONTEND_FOR_DAB ETAL_FE_HANDLE_1 // use ETAL_FE_HANDLE_3 for MTD board

/*****************************************************************
| variables;
|----------------------------------------------------------------*/
ETAL_HANDLE hReceiver;
ETAL_HANDLE hDatapath;
ETAL_HANDLE hMonitor;

/***************************
 *
 * standard2Ascii
 *
 **************************/
static char *standard2Ascii(EtalBcastStandard std)
{
	switch (std)
	{
		case ETAL_BCAST_STD_UNDEF:
			return "Undefined";
		case ETAL_BCAST_STD_DRM:
			return "DRM";
		case ETAL_BCAST_STD_DAB:
			return "DAB";
		case ETAL_BCAST_STD_FM:
			return "FM";
		case ETAL_BCAST_STD_HD_FM:
			return "HDFM";
		case ETAL_BCAST_STD_HD_AM:
			return "HDAM";
		case ETAL_BCAST_STD_AM:
			return "AM";
		default:
			return "Illegal";
	}
}

/***************************
 *
 * mode2string
 *
 **************************/
static char *mode2string(radioModeTy mode)
{
	switch (mode)
	{
		case DAB_MODE:
			return "dab";
		case FM_MODE:
			return "fm";
		case HD_AM_MODE:
			return "hd_am";
		case HD_FM_MODE:
			return "hd_fm";
		case AM_MODE:
			return "am";
		default:
			return "undefined";
	}
}

/***************************
 *
 * mode2standard
 *
 **************************/
static EtalBcastStandard mode2standard(radioModeTy mode)
{
	switch (mode)
	{
		case DAB_MODE:
			return ETAL_BCAST_STD_DAB;
		case FM_MODE:
			return ETAL_BCAST_STD_FM;
		case HD_AM_MODE:
			return ETAL_BCAST_STD_HD_AM;
		case HD_FM_MODE:
			return ETAL_BCAST_STD_HD_FM;
		case AM_MODE:
			return ETAL_BCAST_STD_AM;
		default:
			return ETAL_BCAST_STD_UNDEF;
	}
}

/***************************
 *
 * mode2frontend
 *
 **************************/
static int mode2frontend(radioModeTy mode)
{
	switch (mode)
	{
		case FM_MODE:
		case HD_FM_MODE:
			return FRONTEND_FOR_FM;
		case DAB_MODE:
			return FRONTEND_FOR_DAB;
		case AM_MODE:
		case HD_AM_MODE:
			return FRONTEND_FOR_AM;
		default:
			return -1;
	}
}


/***************************
 *
 * printTextInfo
 *
 **************************/
static void printTextInfo(char * prefix, EtalTextInfo *text_info)
{
	if ((text_info->m_serviceNameIsNew) || (text_info->m_currentInfoIsNew))
	{
		printf("%s Text info Broadcast Standard: %s\n", prefix, standard2Ascii(text_info->m_broadcastStandard));
	}
	if (text_info->m_serviceNameIsNew)
	{
		printf("%s Text info Service name: %s\n", prefix, text_info->m_serviceName);
	}
	if (text_info->m_currentInfoIsNew)
	{
		printf("%s Text info Current Info: %s\n", prefix, text_info->m_currentInfo);
	}
}

/***************************
 *
 * etalTestRadiotextCallback
 *
 **************************/
void etalTestRadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	printTextInfo("  (textinfo callback)", (EtalTextInfo *)pBuffer);
}

#ifdef HAVE_QUALITYMONITOR
/***************************
 *
 * etalTestDABMonitorcb
 *
 **************************/
static void etalTestDABMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	printf("  (quality callback) dab signal strength = %d\n", pQuality->EtalQualityEntries.dab.m_BBFieldStrength);
}

/***************************
 *
 * etalTestFMAMMonitorcb
 *
 **************************/
static void etalTestFMAMMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	printf("  (quality callback) fm/am RF signal strength = %d\n", pQuality->EtalQualityEntries.amfm.m_RFFieldStrength);
}
#endif

/***************************
 *
 * seekStatus2String
 *
 **************************/
static tChar *seekStatus2String(etalSeekStatusTy status)
{
	switch (status)
	{
		case ETAL_SEEK_STARTED:
			return "Seek started";
		case ETAL_SEEK_RESULT:
			return "Seek result";
		case ETAL_SEEK_FINISHED:
			return "Seek finished";
		case ETAL_SEEK_ERROR:
			return "Seek ERROR";
		case ETAL_SEEK_ERROR_ON_START:
			return "Seek ERROR on start";
		case ETAL_SEEK_ERROR_ON_STOP:
			return "Seek ERROR on stop";
		default:
			return "Undefined seek status";
	}
}

/***************************
 *
 * userNotificationHandler
 *
 **************************/
static void userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
	if (dwEvent == ETAL_INFO_TUNE)
	{
		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		printf("Tune info event, Frequency %d, good station found %d\n", status->m_stopFrequency, (status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND);
#ifdef HAVE_HD
		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_SYNC)
		{
			printf("HD signal found\n");
		}
		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC)
		{
			printf("HD digital audio aquired\n");
		}
#endif
	}
	else if (dwEvent == ETAL_INFO_SEEK)
	{
		EtalSeekStatus *seek_status;

		seek_status = (EtalSeekStatus *)pstatus;
		printf("Seek info event\n");
		printf("\t%s\n", seekStatus2String(seek_status->m_status));
		printf("\tFrequency found=%d, full cycle=%d, %dkHz\n", seek_status->m_frequencyFound, seek_status->m_fullCycleReached, seek_status->m_frequency);
	}
	else
	{
		printf("Unexpected event %d\n", dwEvent);
	}
}

/***************************
 *
 * Config
 *
 **************************/
static int Config(radioModeTy mode)
{
	EtalReceiverAttr attr;
#ifdef HAVE_RADIOTEXT
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	EtalDataPathAttr dataPathAttr;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
#endif
#ifdef HAVE_QUALITYMONITOR
	EtalBcastQualityMonitorAttr mon;
#endif
	ETAL_STATUS ret;
	static int first_time = 1;

	/*
	 * Create (or reconfigure) a receiver
	 */

	if (first_time)
	{
		hReceiver = ETAL_INVALID_HANDLE;
		printf("Create %s receiver\n", mode2string(mode));
		first_time = 0;
	}
	else
	{
		printf("Reconfiguring %s receiver\n", mode2string(mode));
	}

	memset(&attr, 0x00, sizeof(EtalReceiverAttr));

	attr.m_Standard = mode2standard(mode);
	if (attr.m_Standard == ETAL_BCAST_STD_UNDEF)
	{
		return 1;
	}
	attr.m_FrontEnds[0] = mode2frontend(mode);
	if (attr.m_FrontEnds[0] < 0)
	{
		return 1;
	}

	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_config_receiver %s (%d)\n", mode2string(mode), ret);
		return 1;
	}

#ifdef HAVE_RADIOTEXT
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	if (mode != AM_MODE)
	{
		/*
		 * Set up a Radiotext datapath
		 * ETAL invokes the Radiotext callback every time there is new data
		 * The datapath creation is not necessary if only the etaltml_get_textinfo is used
		 *
		 * datapaths associated with a receiver are destroyed by ETAL when the
		 * receiver is destroyed or reconfigured, so they must be recreated every
		 * time the Config is called
		 */

		if (mode == FM_MODE)
		{
			if ((ret = etal_start_RDS(hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
			{
				printf("etal_start_RDS for fm (%d)", ret);
				return 1;
			}		
		}
		printf("Creating %s Radiotext datapath\n", mode2string(mode));
		hDatapath = ETAL_INVALID_HANDLE;
		memset(&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
		dataPathAttr.m_receiverHandle = hReceiver;
		dataPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
		dataPathAttr.m_sink.m_context = (void *)0;
		dataPathAttr.m_sink.m_BufferSize = sizeof(EtalTextInfo);
		dataPathAttr.m_sink.m_CbProcessBlock = etalTestRadiotextCallback;
		if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etal_config_datapath for %s (%d)\n", mode2string(mode), ret);
			return 1;
		}
		if ((ret = etaltml_start_textinfo(hReceiver)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etaltml_start_textinfo for %s (%d)\n", mode2string(mode), ret);
			return 1;
		}
	}
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
#endif
#ifdef HAVE_QUALITYMONITOR
	/*
	 * Set up a quality monitor
	 * We want to be notified only when the Signal strength is higher than 10.0,
	 * and only every 1000ms
	 */

	printf("Creating %s monitor\n", mode2string(mode));
	hMonitor = ETAL_INVALID_HANDLE;
	memset(&mon, 0x00, sizeof(EtalBcastQualityMonitorAttr));
	mon.m_receiverHandle = hReceiver;
	switch (mode)
	{
		case DAB_MODE:
			mon.m_CbBcastQualityProcess = etalTestDABMonitorcb;
			mon.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_DabFieldStrength;
			break;
		case FM_MODE:
		case AM_MODE:
		case HD_AM_MODE:
		case HD_FM_MODE:
			mon.m_CbBcastQualityProcess = etalTestFMAMMonitorcb;
			mon.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_FmFieldStrength;
			break;
		default:
			printf("ERROR: undefined case in switch\n");
			return 1;
	}
	mon.m_monitoredIndicators[0].m_InferiorValue = 10;
	mon.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	mon.m_monitoredIndicators[0].m_UpdateFrequency = 1000;  // millisec
	if ((ret = etal_config_reception_quality_monitor(&hMonitor, &mon)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_config_reception_quality_monitor for %s (%d)", mode2string(mode), ret);
		return 1;
	}
#endif

	/* audio output selection */
	switch (mode)
	{
		case DAB_MODE:
			if ((ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_audio_select dab (%d)\n", ret);
				return 1;
			}
			break;
		case FM_MODE:
		case AM_MODE:
			if ((ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_audio_select dab (%d)\n", ret);
				return 1;
			}
			break;
		case HD_AM_MODE:
		case HD_FM_MODE:
			if ((ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_audio_select dab (%d)\n", ret);
				return 1;
			}
			break;
		default:
			printf("ERROR: undefined case in switch\n");
			return 1;
	}
	return 0;
}

/***************************
 *
 * Destroy
 *
 **************************/
static int Destroy(void)
{
	ETAL_STATUS ret;

	printf("Destroy receiver\n");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_destroy_receiver (%d)\n", ret);
		return 1;
	}
	return 0;
}

/***************************
 *
 * RadioDABStart
 *
 **************************/
static int RadioDABStart(void)
{
#ifdef HAVE_DAB
	ETAL_STATUS ret;

	/*
	 * for DAB we need to allow the tuner some time
	 * to capture and decode the ensemble information
	 * Without this information the tuner is unable to perform
	 * any operation on the ensemble except quality measure
	 */
	Sleep(USLEEP_ONE_SEC);

	/*
	 * A DAB ensemble contains one or more services, select one.
	 * This is the DAB equivalent of tuning to an FM station.
	 */
	printf("Selecting a service from the ensemble\n");
	if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_service_select (%d)", ret);
		return 1;
	}
#endif
	return 0;
}

/***************************
 *
 * RadioDABStop
 *
 **************************/
static int RadioDABStop(void)
{
	return 0;
}

/***************************
 *
 * Radio
 *
 **************************/
/*
 * A simple radio receiver, wich only supports tuning and fetching
 * the Text information
 *
 * Text information is not supported for AM mode
 */
static int Radio(radioModeTy mode, int freq, int do_seek)
{
	EtalBcastQualityContainer qual;
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	EtalTextInfo text_info;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

	ETAL_STATUS ret;
	int delay;
    EtalProcessingFeatures processingFeatures;

	/*
	 * Tune 
	 */

	if(mode == HD_FM_MODE)
	{
		/* Default band is set to FM so no need to change it */
	}

	if(mode == HD_AM_MODE)
	{
		processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_change_band_receiver(hReceiver, ETAL_BAND_MWUS, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
		{
			printf("etal_change_band_receiver AM ERROR");
		}
		else
		{
			printf("etal_change_band_receiver ETAL_BAND_MWUS OK\n");
		}
	}

	printf("Tune to %s freq %d\n", mode2string(mode), freq); // ETAL_VALID_HD_FREQ);
	if (((mode != DAB_MODE) && ((ret = etal_tune_receiver_async(hReceiver, freq)) != ETAL_RET_SUCCESS)) ||
		((mode == DAB_MODE) && ((ret = etal_tune_receiver(hReceiver, freq)) != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA)))
	{
		printf("ERROR: etal_tune_receiver %s (%d)\n", mode2string(mode), ret);
		return 1;
	}

	if (mode == DAB_MODE)
	{
		if (RadioDABStart())
		{
			return 1;
		}
	}

	/*
	 * Listen to some radio while performing some simple operations 
	 */

	delay = 0;
	while (delay < SECONDS_PER_CYCLE)
	{

		/*
		 * Allow some time to capture RDS data and evaluate the quality
		 * (actually only needed for the first <delay> loop)
		 * This time might be long in case of low signal condition
		 */
		Sleep(1 * USLEEP_ONE_SEC);

		/*
		 * Read the signal strength, one shot
		 *
		 */
		if ((ret = etal_get_reception_quality(hReceiver, &qual)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etal_get_reception_quality for %s (%d)\n", mode2string(mode), ret);
			return 1;
		}
		if (mode == DAB_MODE)
		{
			printf("  (quality oneshot) %s BB signal strength = %d\n", mode2string(mode), qual.EtalQualityEntries.dab.m_BBFieldStrength);
		}
		else if (mode == FM_MODE)
		{
			printf("  (quality oneshot) %s RF signal strength = %d\n", mode2string(mode), qual.EtalQualityEntries.amfm.m_RFFieldStrength);
		}
		else if ((mode == HD_AM_MODE) || (mode == HD_FM_MODE))
		{
			printf("  (quality oneshot) %s QI = %d\n", mode2string(mode), qual.EtalQualityEntries.hd.m_QI);
		}

#ifdef HAVE_RADIOTEXT
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
		if ((mode != AM_MODE) && (mode != HD_AM_MODE))
		{
			/*
			 * Get Text info, one shot
			 * This is not required if the datapath approach is use, it's just to demonstrate
			 * another API.
			 * The receiver needs some time to acquire all the textinfo so
			 * some info may be missing from the printout: this is perfectly normal
			 */
			memset(&text_info, 0x00, sizeof(text_info));
			if ((ret = etaltml_get_textinfo(hReceiver, &text_info) != ETAL_RET_SUCCESS))
			{
				printf("ERROR: etaltml_get_textinfo %s (%d)\n", mode2string(mode), ret);
				return 1;
			}
			printTextInfo("  (textinfo oneshot)", &text_info);
		}
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
#endif
		delay++;
	}

	/*
	 * Seek to the next station
	 */

	if (do_seek)
	{
		/*
		 * Seek to the next good station, up, default step, start from current frequency
		 *
		 * NOTE: seek command support depends on the silicon version:
		 *       - CMOST CUT1   (unofficial firmware release), supported
		 *       - CMOST CUT2.0 not supported
		 *       - CMOST CUT2.1 supported
		 *       If seek is not supported by the silicon the command below may silently fail,
		 *       or it may return an error (depending on how the ETAL was built). In case of
		 *       silent fail tune will remain on the current frequency and no seek station
		 *       will be found
		 *
		 *       You can discover which CUT you have looking at the silicon stamp:
		 *       STA710 AA  = CUT 2.0
		 *       STA710 AB  = CUT 2.1
		 *       TDA7707 BC = CUT 2.1
		 */
		printf("Seek up %s\n", mode2string(mode));
		if ((ret = etal_autoseek_start(hReceiver, cmdDirectionUp, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etal_autoseek_start for %s (%d)\n", mode2string(mode), ret);
			return 1;
		}
		/*
		 * Allow some time for the seek to find a good station
		 * (an event will also be sent to the userNotificationHandler())
		 * If ETAL finds no good station try increasing the delay up to
		 * few seconds, in bad reception conditions the seek takes time
	 	 * NOTE: also using the 'external auto seek' requires longer delay,
	 	 * this is now the default for ETAL library. So the delay is
	 	 * set to 2 seconds.
		 */
		Sleep(2 * USLEEP_ONE_SEC);

		/*
		 * Stop seek and listen to the new radio station (if it was found,
		 * otherwise ETAL remains tuned to the last frequency found by the
		 * seek algorithm even if no station was found there)
		 */
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etal_autoseek_stop for %s (%d)\n", mode2string(mode), ret);
			return 1;
		}
		Sleep(5 * USLEEP_ONE_SEC);
	}

	if (mode == DAB_MODE)
	{
		if (RadioDABStop())
		{
			return 1;
		}
	}

	return 0;
}

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
int main(int argc, char **argv)
{
	int ret;
	int count = 0;
	EtalHardwareAttr init_params;
    EtalAudioInterfTy audioIf;

	/*
	 * Initialize ETAL
	 */

	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = userNotificationHandler;

	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_initialize (%d)\n", ret);
		return 1;
	}

    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
    if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
    {
        printf("etal_config_audio_path (%d)", ret);
        return 0;
    }
	
	ret = 0;
	while ((count < NUMBER_OF_CYCLES) && !ret)
	{
		printf("\nLoop %d\n", count + 1);

#ifdef HAVE_DAB
		printf("\n");
		if (!Config(DAB_MODE))
		{
			ret += Radio(DAB_MODE, ETAL_VALID_DAB_FREQ, 1);
		}
#endif
#ifdef HAVE_FM
		printf("\n");
		if (!Config(FM_MODE))
		{
			ret += Radio(FM_MODE, ETAL_VALID_FM_FREQ, 1);
		}
#endif
#ifdef HAVE_HD_FM
		printf("\n");
		if (!Config(HD_FM_MODE))
		{
			ret += Radio(HD_FM_MODE, ETAL_VALID_HD_FREQ, 1);
		}
#endif
#ifdef HAVE_HD_AM
		printf("\n");
		if (!Config(HD_AM_MODE))
		{
			ret += Radio(HD_AM_MODE, ETAL_VALID_AM_FREQ, 1);
		}
#endif
#ifdef HAVE_AM
		printf("\n");
		if (!Config(AM_MODE))
		{
			ret += Radio(AM_MODE, ETAL_VALID_AM_FREQ, 1);
		}
#endif
		count++;
	}

	Destroy();

	/*
	 * Final cleanup
	 */

	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_deinitialize\n");
	}

	return 0;
}

