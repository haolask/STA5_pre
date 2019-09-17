//!
//!  \file 		etaldemo.c
//!  \brief 	<i><b> ETAL application demo </b></i>
//!  \details   Demo ETAL user application, simulates a radio
//!  \author 	Raffaele Belardi
//!

/*
 * Implements a demo ETAL API user function with two purposes:
 * 1. verify that lib_etal/exports/ contains all is needed to link
 *    with the ETAL library
 * 2. provide an example usage of the ETAL library.
 *
 * The demo is coded for AM/FM and DAB system based on CMOSTuner (TDA7707)
 * and STA66x for the Linux/Accordo2 platform.
 * Note that in such system the DAB audio decoded by the STA66x
 * is routed back to the CMOST for conversion to analog and
 * output on the same pins as the analog FM signal.
 */
#include "etal_api.h"
#include "etaltml_api.h"


#include <stdio.h>
#include <string.h>
//#include <unistd.h>  // usleep
//#include <windows.h>

#define HAVE_DAB
#define ETALDEMO_OK 0
#define ETALDEMO_ERROR -1



#define USLEEP_ONE_SEC 1000000
#define usleep(x) Sleep(x/1000)


/*****************************************************************
| variables;
|----------------------------------------------------------------*/
//#define DAB_TEST

#if (defined DAB_TEST)
#define TEST_STAND ETAL_BCAST_STD_DAB
#define TEST_BAND ETAL_BAND_DAB3
#else
#define TEST_STAND ETAL_BCAST_STD_FM
#define TEST_BAND ETAL_BAND_FMEU
#endif // #if (defined DAB_TEST)
 EtalLearnFrequencyTy        freqList[50];

/***************************
 *
 * userNotificationHandler
 *
 **************************/
static void userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
	int i;

	if (dwEvent == ETAL_INFO_TUNE)
	{
		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		printf("Tune info event, Frequency %d, good station found %d\n", status->m_stopFrequency, (status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND);
	}
	else if (dwEvent == ETAL_INFO_LEARN)
	{
		EtalLearnStatusTy *learn_status;

		learn_status = (EtalLearnStatusTy *)pstatus;
		printf("Learn info event, state %d, freq %d, numb %d\n", learn_status->m_status, learn_status->m_frequency, learn_status->m_nbOfFrequency);

		if(ETAL_LEARN_FINISHED == learn_status->m_status)
		{
			printf("\nLearned Frequencies:\n");

			for(i=0; i<learn_status->m_nbOfFrequency; i++)
			{
				printf("\t%d\t%d\tfst %d dB:\n",i, freqList[i].m_frequency, freqList[i].m_fieldStrength);
			}
		}
	}
	else
	{
                printf("Event not handled %d\n", dwEvent);
	}
}

void FMRadio(void)
{
	EtalProcessingFeatures processingFeatures;
	EtalReceiverAttr attr;

	ETAL_HANDLE hReceiver;
	ETAL_STATUS ret;
	volatile char tmp = ' ';


	/*
	 * Create a receiver configuration
	 */

	hReceiver = ETAL_INVALID_HANDLE;
	printf("Create receiver\n");
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));
	attr.m_Standard = TEST_STAND;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1; // use ETAL_FE_HANDLE_3 for MTD board
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_config_receiver (%d)\n", ret);
		return;
	}

    /* Select audio source */
	if ((ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_audio_select STAR (%d)\n", ret);
		return;
	}

    processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;

	if ((ret = etal_change_band_receiver(hReceiver, TEST_BAND, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_change_band (%d)\n", ret);
		return;
	}

	/*
	 * Tune to a DAB
	 */
//	printf("Tune to FM freq %d\n", ETAL_VALID_FM_FREQ);
//	if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_SUCCESS)
//	{
//		printf("ERROR: etal_tune_receiver FM (%d)\n", ret);
//		return;
//	}

	printf("running ... \n");

	while(tmp != 'q')
	{
		if ((ret = etaltml_learn_start(hReceiver, TEST_BAND, 100, 30, normalMode, freqList)) != ETAL_RET_SUCCESS)
		{
			printf("error start learn\n");
			return;
		}

		tmp = getchar();
	}




	/*
	 * Destroy all receivers
	 */
	printf("Destroy FM receiver\n");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_destroy_receiver FM (%d)\n", ret);
		return;
	}
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
	EtalHardwareAttr init_params;

	/*
	 * Initialize ETAL
	 */
	printf("Init ... ");
	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = userNotificationHandler;

	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_initialize (%d)\n", ret);
		return -1;
	}
  
	FMRadio();

	/*
	 * Final cleanup
	 */
	printf("Clean up ... ");
	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_deinitialize\n");
	}

	return 0;
}

