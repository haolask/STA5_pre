//!
//!  \file 		etaldemoData.c
//!  \brief 	<i><b> ETAL application demo </b></i>
//!  \details   Demo ETAL user application, simulates a radio with HDRadio or DAB and RAW data service
//!  \author 	Raffaele Belardi
//!

/*
 * Implements a demo ETAL API user function with two purposes:
 * 1. verify that etalcore/exports/ contains all is needed to link
 *    with the ETAL library
 * 2. provide an example usage of the ETAL library.
 *
 * The demo is coded for DAB and HD system based on CMOSTuner (TDA7707)
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
#define HAVE_DAB // not compatible with HD
#define HAVE_FM
//#define HAVE_HD  // not compatible with DAB
//#define HAVE_AM

#define HAVE_RADIOTEXT

typedef enum
{
	DAB_MODE,
	FM_MODE,
	HD_FM_MODE,
	AM_MODE
} radioModeTy;

#define NUMBER_OF_CYCLES           10
#define SECONDS_PER_CYCLE          10
#define MAX_PRINTED_DATA_BYTES     32

/*
 * Adjust ETAL_VALID_*_FREQ to some known good FM/HD station possibly with RDS
 */
#define ETAL_VALID_FM_FREQ     104800
#define ETAL_VALID_HD_FREQ      88700
#define ETAL_VALID_AM_FREQ        900
#define ETAL_VALID_HD_AM_FREQ     711

/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB_FREQ    225648
#define ETAL_DAB_ETI_UEID      0xE0106E
#define ETAL_DAB_ETI_SERV1_SID 0x1D12      // Fantasy Aktuell
#define ETAL_DAB_ETI_SERVD_SID 0xE0D7106E  // EPG Augsburg

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
ETAL_HANDLE hDABDatapathAudio;
ETAL_HANDLE hDABDatapathData;

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
		case HD_FM_MODE:
			return "hdfm";
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
			return FRONTEND_FOR_AM;
		default:
			return -1;
	}
}


/***************************
 *
 * printRadioText
 *
 **************************/
static void printRadioText(char * prefix, EtalTextInfo *radio_text)
{
	if ((radio_text->m_serviceNameIsNew) || (radio_text->m_currentInfoIsNew))
	{
		printf("%s Radio text Broadcast Standard: %s\n", prefix, standard2Ascii(radio_text->m_broadcastStandard));
	}
	if (radio_text->m_serviceNameIsNew)
	{
		printf("%s Radio text Service name: %s\n", prefix, radio_text->m_serviceName);
	}
	if (radio_text->m_currentInfoIsNew)
	{
		printf("%s Radio text Current Info: %s\n", prefix, radio_text->m_currentInfo);
	}
}

/***************************
 *
 * etalTestRadiotextCallback
 *
 **************************/
void etalTestRadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	printRadioText("  (radiotext callback)", (EtalTextInfo *)pBuffer);
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
	else
	{
		printf("Unexpected event %d\n", dwEvent);
	}
}

/***************************
 *
 * userDataCallback
 *
 **************************/
static void userDataCallback(tU8 *pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void *pvContext)
{
	int i;

	printf("Received new data, size %d\n", dwActualBufferSize);
#if 1
	for (i = 0; (i < dwActualBufferSize) && (i < MAX_PRINTED_DATA_BYTES); i++)
	{
		printf("%.2x ", pBuffer[i]);
		if ((i % 8) == 7)
		{
			printf("\n");
		}
	}
	if (dwActualBufferSize > MAX_PRINTED_DATA_BYTES)
	{
		printf("...\n");
	}
#endif
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
			if ((ret = etal_start_RDS(hReceiver, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_start_RDS for %s (%d)\n", mode2string(mode), ret);
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
	EtalDataPathAttr dataPathAttr;
	ETAL_STATUS ret;

	/*
	 * for DAB we need to allow the tuner some time
	 * to capture and decode the ensemble information
	 * Without this information the tuner is unable to perform
	 * any operation on the ensemble except quality measure
	 */
	Sleep(USLEEP_ONE_SEC);

	/*
	 * Before selecting a service we need to set up an Audio
	 * datapath
	 */

#if 0
	printf("Set up audio datapath for AUDIO service selection\n");
	hDABDatapathAudio = ETAL_INVALID_HANDLE;
	dataPathAttr.m_receiverHandle = hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_AUDIO;
	memset(&dataPathAttr.m_sink, 0x00, sizeof(EtalSink));
	if ((ret = etal_config_datapath(&hDABDatapathAudio, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_config_datapath for Service Select (%d)", ret);
		return 1;
	}

	/*
	 * A DAB ensemble contains one or more services, select one.
	 * This is the DAB equivalent of tuning to an FM station.
	 */
	printf("Selecting an AUDIO service from the ensemble\n");
	if ((ret = etal_service_select(hDABDatapathAudio, ETAL_SERVSEL_MODE_DAB_SID, ETAL_SERVSEL_SUBF_SET, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_service_select (%d)", ret);
		return 1;
	}
#endif

	/*
	 * Set up a RAW data datapath
	 * ETAL invokes the Radiotext callback every time there is new data
	 *
	 * datapaths associated with a receiver are destroyed by ETAL when the
	 * receiver is destroyed or reconfigured, so they must be recreated every
	 * time the Config is called
	 */

	printf("Creating DAB RAW data datapath\n");
	hDABDatapathData = ETAL_INVALID_HANDLE;
	memset(&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
	dataPathAttr.m_receiverHandle = hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_DATA_RAW;
	dataPathAttr.m_sink.m_context = (void *)0;
	dataPathAttr.m_sink.m_BufferSize = 0; // unused
	dataPathAttr.m_sink.m_CbProcessBlock = userDataCallback;
	if ((ret = etal_config_datapath(&hDABDatapathData, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_config_datapath for DAB (%d)\n", ret);
		return 1;
	}

	/*
	 * Select a data service
	 */
	printf("Selecting a DATA service from the ensemble\n");
	if ((ret = etal_service_select_data(hDABDatapathData, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERVD_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
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
#ifdef HAVE_DAB
	ETAL_STATUS ret;

#if 0
	if ((ret = etal_destroy_datapath(&hDABDatapathAudio)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_destroy_datapath for Service Select AUDIO (%d)", ret);
		return 1;
	}
#endif
	if ((ret = etal_destroy_datapath(&hDABDatapathData)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_destroy_datapath for Service Select DATA (%d)", ret);
		return 1;
	}
#endif
	return 0;
}

/***************************
 *
 * Radio
 *
 **************************/
/*
 * A simple radio receiver, wich only supports tuning and fetching
 * the radiotext information
 *
 * Radiotext information is not supported for AM mode
 */
static int Radio(radioModeTy mode, int freq, int do_seek)
{
	EtalBcastQualityContainer qual;
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	EtalTextInfo radio_text;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	ETAL_STATUS ret;
	int delay;

	/*
	 * Tune 
	 */

	printf("Tune to %s freq %d\n", mode2string(mode), freq); // ETAL_VALID_HD_FREQ);
	if (((ret = etal_tune_receiver(hReceiver, freq)) != ETAL_RET_SUCCESS) && ((mode != DAB_MODE) || ((mode == DAB_MODE) && (ret != ETAL_RET_NO_DATA))))
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
		else
		{
			printf("  (quality oneshot) %s RF signal strength = %d\n", mode2string(mode), qual.EtalQualityEntries.amfm.m_RFFieldStrength);
		}

#ifdef HAVE_RADIOTEXT
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
		if (mode != AM_MODE)
		{
			/*
			 * Get Radio text, one shot
			 * This is not required if the datapath approach is use, it's just to demonstrate
			 * another API.
			 * The receiver needs some time to acquire all the radiotext so
			 * some info may be missing from the printout: this is perfectly normal
			 */
			memset(&radio_text, 0x00, sizeof(radio_text));
			if ((ret = etaltml_get_textinfo(hReceiver, &radio_text) != ETAL_RET_SUCCESS))
			{
				printf("ERROR: etaltml_get_textinfo %s (%d)\n", mode2string(mode), ret);
				return 1;
			}
			printRadioText("  (radiotext oneshot)", &radio_text);
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
		 */
		Sleep(USLEEP_ONE_SEC);

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
#ifdef HAVE_HD
		printf("\n");
		if (!Config(HD_FM_MODE))
		{
			ret += Radio(HD_FM_MODE, ETAL_VALID_HD_FREQ, 1);
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

