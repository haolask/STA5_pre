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
#undef RESET_RDS_AFTER_SEEK
#undef HAVE_WINDOWS
//#define HAVE_DAB

//#include "target_config.h"
#include "etal_api.h"
#include "etaltml_api.h"

#include <stdio.h>
#include <string.h>
#ifndef HAVE_WINDOWS
#include <unistd.h>  // usleep
#else
#include <windows.h>
#define usleep(x) Sleep(x/1000)
#endif


#define ETALDEMO_OK 0
#define ETALDEMO_ERROR -1

#define USLEEP_ONE_MSEC            1000
#define USLEEP_ONE_SEC          1000000
#define USEC_MAX                1000000

// default time for PI & PS : 2 s
// time for RT : 10s
#define ETAL_DEMO_WAITTIME_RDS_PIPS			2
#define ETAL_DEMO_WAITTIME_RDS_RT			10

#define ETAL_DEMO_WAITTIME_RDS_INFO_SEEK	ETAL_DEMO_WAITTIME_RDS_RT



/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB_FREQ     227360
#define ETAL_VALID_FM_FREQ     	87500
#define ETAL_DAB_ETI_SERV1_SID  0x232F

void RDS_enable(ETAL_HANDLE handler,tBool en);

/*****************************************************************
| variables;
|----------------------------------------------------------------*/
ETAL_HANDLE hReceiverFg, hReceiverBg;
ETAL_HANDLE hReceiverFgRDS, hReceiverBgRDS;
unsigned int v_FgFrequency, v_BgFrequency;
unsigned int v_FgCurrentPI, v_BgCurrentPI;

volatile unsigned int stationFound, fullCycle;

/***************************
 *
 * userNotificationHandler
 *
 **************************/
static void userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
	int ret;


	if (dwEvent == ETAL_INFO_TUNE)
	{
		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		printf("Tune info event, Frequency %d, good station found %d\n", status->m_stopFrequency, (status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND);
	}
	else if (dwEvent == ETAL_INFO_SEEK)
	{
		EtalSeekStatus *seek_status;

		seek_status = (EtalSeekStatus *)pstatus;

		// bg handling
		if(seek_status->m_receiverHandle == hReceiverBg)
		{

			//printf("BG FM\tFrequency found=%d, full cycle=%d, %dkHz\n", seek_status->m_frequencyFound, seek_status->m_fullCycleReached, seek_status->m_frequency);

			if(seek_status->m_status == ETAL_SEEK_RESULT)
			{
				if(seek_status->m_frequencyFound)
				{
					stationFound = 1;
                                        //printf("\t\t\t\tBG FM\tFrequency %dkHz\n", seek_status->m_frequency);
                                        printf("*** ETALDEMO BG FM\tFrequency %dkHz\n", seek_status->m_frequency);
										 v_BgFrequency = seek_status->m_frequency;
				}

				if(seek_status->m_fullCycleReached)
				{
					fullCycle = 1;
                                     printf("\n\tBG FM\tFullcycle\n");
									  v_BgFrequency = seek_status->m_frequency;
				}
			}
		}

		// fg handling
		if(seek_status->m_receiverHandle == hReceiverFg)
		{
			if(seek_status->m_status == ETAL_SEEK_FINISHED)
			{
				if(seek_status->m_frequencyFound)
				{
                                        printf("*** ETALDEMO FG FM\tFrequency %dkHz\n", seek_status->m_frequency);
                                        RDS_enable(hReceiverFg, 1);
										v_FgFrequency = seek_status->m_frequency;
                }

				if(seek_status->m_fullCycleReached)
				{
                                        printf("FG FM\tFull Cycle\n");
										v_FgFrequency = seek_status->m_frequency;
				}

				if((ret = etal_autoseek_stop(hReceiverFg, lastFrequency)) != ETAL_RET_SUCCESS)
				{
                                        printf("ERROR: etal_autoseek_stop FG FM (%d)\n", ret);
				}
			}
		}
	}
	else
	{
                printf("Event not handled %d\n", dwEvent);
	}
}


void etalRDSCallbackFG(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
    EtalRDSData *prds = (EtalRDSData *)pBuffer;

	printf("*** ETALDEMO FG FM : Freq %d kHz,", v_FgFrequency);

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
    {
        printf("\tPI %4.4x ", prds->m_PI);
		v_FgCurrentPI = prds->m_PI;
    }
	else
	{
		printf("\tcurrent PI %4.4x ", v_FgCurrentPI);
	}

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
    {
        printf("\tPS %s", prds->m_PS);
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
    {
        printf("\tRT %s", prds->m_RT);
    }

	printf("*** \n");

}

void etalRDSCallbackBG(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
    EtalRDSData *prds = (EtalRDSData *)pBuffer;

	printf("*** ETALDEMO BG FM : Freq %d kHz,", v_BgFrequency);

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
    {
        printf("\tPI %4.4x", prds->m_PI);
		v_BgCurrentPI = prds->m_PI;
    }
	else if ((prds->m_storedInfoBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
	{
		// PI stored
		if (v_BgCurrentPI != prds->m_PI)
		{
			printf("\tcurrent PI %4.4x different from Stored in ETAL %4.4x!!", v_BgCurrentPI, prds->m_PI);
			v_BgCurrentPI = prds->m_PI;
		}
		
		printf("\tcurrent PI %4.4x ", v_BgCurrentPI);
	}

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
    {
        printf("\tPS %s", prds->m_PS);
    }

    if ((prds->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
    {
        printf("\tRT %s", prds->m_RT);
    }

	printf("*** \n");
}

void RDS_enable(ETAL_HANDLE handler,tBool en)
{
	ETAL_STATUS ret;

	if(en)
	{
		if ((ret = etal_start_RDS(handler, 0, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etal_start_RDS (%d)\n", ret);
		}

		if ((ret = etaltml_start_decoded_RDS(handler, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
		{
			printf("etaltml_start_decoded_RDS (%d)\n", ret);
		}
	}
	else
	{
		if ((ret = etal_stop_RDS(handler)) != ETAL_RET_SUCCESS)
		{
			printf("etal_stop_RDS (%d)\n", ret);
	    }

		if ((ret = etaltml_stop_decoded_RDS(handler, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
		{
			printf("etaltml_stop_decoded_RDS (%d)\n", ret);
		}
	}
}


int cntr = 0;

void FMRadio(void)
{
	EtalProcessingFeatures processingFeatures;
	EtalReceiverAttr attr;
	EtalDataPathAttr rdsAttr;
	ETAL_STATUS ret;
	char tmp;

	/*
	 * Create a receiver configuration
	 */
	hReceiverFg = ETAL_INVALID_HANDLE;
	printf("Create FG FM receiver\n");
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1; // use ETAL_FE_HANDLE_3 for MTD board
	if ((ret = etal_config_receiver(&hReceiverFg, &attr)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_config_receiver FG FM (%d)\n", ret);
		return;
	}

	/*
	 * Create a bg receiver configuration
	 */
	hReceiverBg = ETAL_INVALID_HANDLE;
	printf("Create BG FM receiver\n");
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_2; // use ETAL_FE_HANDLE_3 for MTD board
	if ((ret = etal_config_receiver(&hReceiverBg, &attr)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_config_receiver BG FM (%d)\n", ret);
		return;
	}

    /*
     * Select audio source
     */
	if ((ret = etal_audio_select(hReceiverFg, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_audio_select DCOP (%d)\n", ret);
		return;
	}

	printf("FG receiver band change to FM\n");
    processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;

    if ((ret = etal_change_band_receiver(hReceiverFg, ETAL_BAND_FMEU, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_change_band FG FM (%d)\n", ret);
		return;
	}

	printf("BG receiver band change to FM\n");
    processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;

    if ((ret = etal_change_band_receiver(hReceiverBg, ETAL_BAND_FMEU, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
    {
		printf("ERROR: etal_change_band FM (%d)\n", ret);
		return;
	}

    /*
	 * Tune to
	 */
	//printf("Tune FG to FM freq %d\n", ETAL_VALID_FM_FREQ);
    printf("Tune FG to FM freq %d\n", 94600);

        if ((ret = etal_tune_receiver(hReceiverFg, 94600)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_tune_receiver FG FM (%d)\n", ret);
	}

    /*
	 * Tune to a FM
	 */
	printf("Tune BG to FM freq %d\n", ETAL_VALID_FM_FREQ);

	if ((ret = etal_tune_receiver(hReceiverBg, ETAL_VALID_FM_FREQ)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_tune_receiver BG FM (%d)\n", ret);
	}

	/*
	 * RDS monitor configuration for FG
	 */
    hReceiverFgRDS = ETAL_INVALID_HANDLE;
    memset(&rdsAttr, 0x00, sizeof(EtalDataPathAttr));
    rdsAttr.m_receiverHandle = hReceiverFg;
    rdsAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
    rdsAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
    rdsAttr.m_sink.m_CbProcessBlock = etalRDSCallbackFG;

    if ((ret = etal_config_datapath(&hReceiverFgRDS, &rdsAttr)) != ETAL_RET_SUCCESS)
    {
    	printf("ERROR: etal_config_datapath FG (%d)\n", ret);
    }

	/*
	 * RDS monitor configuration for BG
	 */
    hReceiverBgRDS = ETAL_INVALID_HANDLE;
    memset(&rdsAttr, 0x00, sizeof(EtalDataPathAttr));
    rdsAttr.m_receiverHandle = hReceiverBg;
    rdsAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
    rdsAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
    rdsAttr.m_sink.m_CbProcessBlock = etalRDSCallbackBG;

    if ((ret = etal_config_datapath(&hReceiverBgRDS, &rdsAttr)) != ETAL_RET_SUCCESS)
    {
    	printf("ERROR: etal_config_datapath BG (%d)\n", ret);
    }

#ifndef RESET_RDS_AFTER_SEEK
    RDS_enable(hReceiverBg, 1);
#endif

    stationFound = 0;
	fullCycle = 0;

	// fm auto seek  on
	// Reset the PI information 
	v_BgCurrentPI = ETAL_INVALID_SID;
	v_FgCurrentPI = ETAL_INVALID_SID;
	if ((ret = etal_autoseek_start(hReceiverFg, cmdDirectionUp, 100, cmdAudioUnmuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
		return;
	}

	// fm auto seek  on
	if ((ret = etal_autoseek_start(hReceiverBg, cmdDirectionUp, 100, cmdAudioMuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
		return;
	}

	printf("running bg scan ... \n");

	tmp = ' ';

	while(tmp != 'q')
	{
		while(!fullCycle)
		{
			if(stationFound)
			{
				// cont
				stationFound = 0;
				
#ifdef RESET_RDS_AFTER_SEEK
                RDS_enable(hReceiverBg, 1);
#endif
				usleep(ETAL_DEMO_WAITTIME_RDS_INFO_SEEK * USLEEP_ONE_SEC);

#ifdef RESET_RDS_AFTER_SEEK
				RDS_enable(hReceiverBg, 0);
#endif

				if((ret = etal_autoseek_continue(hReceiverBg)) != ETAL_RET_SUCCESS)
				{
					printf("ERROR: etal_autoseek_continue FM (%d)\n", ret);

				}
				
				// Reset the PI information 
				v_BgCurrentPI = ETAL_INVALID_SID;

			}
		}

		fullCycle = 0;

		if((ret = etal_autoseek_stop(hReceiverBg, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etal_autoseek_stop() FM (%d)\n", ret);
		}

		printf("... end of scan  ... \n");

		printf("... press u or d to start parallel seek on FG and BG  ... \n");

//		do {
//			tmp = getchar();
//		} while(!(tmp == 'u') && !(tmp == 'd') && !(tmp == 'q') && !(tmp == 'f') && !(tmp == 'b')) ;

                if(cntr++ < 100)
                    tmp = 'u';
                else
                    tmp = 'q';

                printf("... starting parallel seek on FG and BG  ... \n");

		if(tmp == 'u')
		{

                        RDS_enable(hReceiverFg, 0);

			// FM auto seek  on
			if ((ret = etal_autoseek_start(hReceiverFg, cmdDirectionUp, 100, cmdAudioUnmuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
				return;
			}
			
			// Reset the PI information 
			v_FgCurrentPI = ETAL_INVALID_SID;

			// FM auto seek  on
			if ((ret = etal_autoseek_start(hReceiverBg, cmdDirectionUp, 100, cmdAudioMuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
				return;
			}

							
			// Reset the PI information 
			v_BgCurrentPI = ETAL_INVALID_SID;
		}

		if(tmp == 'd')
		{
                       RDS_enable(hReceiverFg, 0);

			// FM auto seek  on
			if ((ret = etal_autoseek_start(hReceiverFg, cmdDirectionDown, 100, cmdAudioUnmuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
				return;
			}
			
			// Reset the PI information 
			v_FgCurrentPI = ETAL_INVALID_SID;

			// FM auto seek  on
			if ((ret = etal_autoseek_start(hReceiverBg, cmdDirectionDown, 100, cmdAudioMuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
				return;
			}

							
			// Reset the PI information 
			v_BgCurrentPI = ETAL_INVALID_SID;
		}

		if(tmp == 'b')
		{
			// FM auto seek  on
			if ((ret = etal_autoseek_start(hReceiverBg, cmdDirectionUp, 100, cmdAudioMuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
				return;
			}
							
			// Reset the PI information 
			v_BgCurrentPI = ETAL_INVALID_SID;
		}


		if(tmp == 'f')
		{
                       RDS_enable(hReceiverFg, 0);

			// FM auto seek  on
			if ((ret = etal_autoseek_start(hReceiverFg, cmdDirectionUp, 100, cmdAudioUnmuted,  dontSeekInSPS, true)) != ETAL_RET_SUCCESS)
			{
				printf("ERROR: etal_autoseek_start FM (%d)\n", ret);
				return;
			}
						
			// Reset the PI information 
			v_FgCurrentPI = ETAL_INVALID_SID;
			fullCycle = 1;
		}
	}

	/*
	 * Destroy all receivers
	 */
	printf("Destroy FG receiver\n");
	if ((ret = etal_destroy_receiver(&hReceiverFg)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_destroy_receiver FG (%d)\n", ret);
		return;
	}

	printf("Destroy BG receiver\n");
	if ((ret = etal_destroy_receiver(&hReceiverBg)) != ETAL_RET_SUCCESS)
	{
		printf("ERROR: etal_destroy_receiver BG (%d)\n", ret);
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

