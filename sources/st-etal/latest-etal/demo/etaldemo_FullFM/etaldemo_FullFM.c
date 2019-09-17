//!
//!  \file 		etaldemo_FullFM.c
//!  \brief 	<i><b> ETAL application for test </b></i>
//!  \details   Tune all receivers on FM (optionally 1 DAB)
//!  \author 	Yann Hemon
//!

#include "target_config.h"

#include "osal.h"
#ifndef CONFIG_ETAL_HAVE_ETALTML
#error "CONFIG_ETAL_HAVE_ETALTML must be defined"
#endif // CONFIG_ETAL_HAVE_ETALTML

#include "etal_api.h"
#include "etaltml_api.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef CONFIG_HOST_OS_WIN32
	#include <windows.h>
#else
	#include <unistd.h>  // usleep
#endif

#include "etaldemo_FullFM.h"


#ifdef HAVE_FM

static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq)
{
	EtalReceiverAttr attr_fm;
	ETAL_HANDLE vl_handle;
	ETAL_STATUS ret;

    EtalProcessingFeatures processingFeatures;

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM FG RADIO TUNING ***************\n");

	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_FG, &attr_fm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMFgRadio / etaltml_getFreeReceiverForPath FM ERROR\n");
		return OSAL_ERROR;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMFgRadio / etaltml_getFreeReceiverForPath fm ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n",
											attr_fm.m_Standard,
											attr_fm.m_FrontEndsSize,
											attr_fm.m_FrontEnds[0], attr_fm.m_FrontEnds[1]);
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set FM FG band\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_FM, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver FM ERROR");
			return OSAL_ERROR;
		}
		else
			{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver fm ok\n");
			}
	/*
	 * Tune to an FM station
	 */
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to fm FM freq %d\n", vI_freq);

	if ((ret = etal_tune_receiver(vl_handle, vI_freq)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver fm (%d)\n", ret);
		return 1;
	}
	else
			{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver fm ok, freq = %d\n", vI_freq);
			}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE FM FG DONE ***************\n");

	return vl_handle;
}

static ETAL_HANDLE etalDemo_TuneFMBgRadio(tU32 vI_freq)
{
	EtalReceiverAttr attr_fm;
	ETAL_HANDLE vl_handle;
	ETAL_STATUS ret;

    EtalProcessingFeatures processingFeatures;

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM BG RADIO TUNING ***************\n");

	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_BG, &attr_fm)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMBgRadio / etaltml_getFreeReceiverForPath FM ERROR\n");
		return OSAL_ERROR;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMBgRadio / etaltml_getFreeReceiverForPath fm ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n",
											attr_fm.m_Standard,
											attr_fm.m_FrontEndsSize,
											attr_fm.m_FrontEnds[0], attr_fm.m_FrontEnds[1]);

	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set FM BG band\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_FM, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver FM ERROR");
			return OSAL_ERROR;
		}
		else
			{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver fm ok\n");
			}
	/*
	 * Tune to an FM station
	 */
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to fm FM freq %d\n", vI_freq);

	if ((ret = etal_tune_receiver(vl_handle, vI_freq)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver fm (%d)\n", ret);
		return 1;
	}
	else
			{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver fm ok, freq = %d\n", vI_freq);
			}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE FM BG DONE ***************\n");

	return vl_handle;
}
#endif // HAVE FM

#ifdef HAVE_DAB

/***************************
 *
 * etalDemo_DABRadio
 *
 **************************/
static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq)
{
	EtalReceiverAttr attr_dab_1;

	ETAL_HANDLE vl_handle;

    EtalProcessingFeatures processingFeatures;

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

	char line[256];
	int Service;
	tBool vl_selectedSidPresent = false;
	tBool vl_continue = false;

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO TUNING ***************\n");

	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_DAB_1, &attr_dab_1)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB1Radio / etaltml_getFreeReceiverForPath DAB ERROR\n");
		return OSAL_ERROR;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB1Radio / etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n",
											attr_dab_1.m_Standard,
											attr_dab_1.m_FrontEndsSize,
											attr_dab_1.m_FrontEnds[0], attr_dab_1.m_FrontEnds[1]);

	}


	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB ERROR");
		return OSAL_ERROR;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_freq);
	ret = etal_tune_receiver(vl_handle, vI_freq);
	if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA ))
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
		return OSAL_ERROR;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB ok, freq = %d\n", vI_freq);
	}


	/*
	 * Unlike FM, for DAB we need to allow the tuner some time
	 * to capture and decode the ensemble information
	 * Without this information the tuner is unable to perform
	 * any operation on the ensemble except quality measure
	 */
	Sleep(USLEEP_ONE_SEC);


	/*
	 * Before selecting a service we need to set up an Audio
	 * datapath
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, first pass\n");


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
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Reading the ensemble list\n");
	memset(&ens_list, 0x00, sizeof(ens_list));
	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_list (%d)\n", ret);
		return 1;
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
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
		return 1;
	}
	etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

	/*
	 * A DAB ensemble contains one or more services: fetch the list
	 * and present it to the user for selection. Some services will be
	 * audio only, others data only. The ETAL interface permits to select
	 * which service list to fetch, here we fetch only the audio services
	 */
	if ((ret = etal_get_service_list(vl_handle, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
		return 1;
	}
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
			etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d)\n", ret);
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
	}

	if (false == vl_selectedSidPresent)
	{
		do
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

			if (fgets(line, sizeof(line), stdin))
			{
				if (1 == sscanf(line, "%d", &Service))
				{
					if (Service > serv_list.m_serviceCount)
					{
						etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
						vl_continue=true;
					}
					else
					{
						/* i can be safely used */
						Service--;
						vl_continue=false;
					}
				}
			}
		}while (true == vl_continue);

	}

	/*
	 * Now the user could select the service
	 * We do it for him
	 */

	/*
	 * This is the DAB equivalent of tuning to an FM station.
	 * Audio may be available after this step, depending on the CMOST CUT:
	 * - CUT 1.0 no audio
	 * - CUT 2.x audio
	 * See above for hints on how to find out the CUT version.
	 */
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n",
					Service+1,
					serv_list.m_service[Service], svinfo.m_serviceLabel);

	if ((ret = etal_service_select_audio(vl_handle, ETAL_SERVSEL_MODE_SERVICE, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)\n", ret);
		return 1;
	}

	if ((ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select fm error" );
		return ETAL_INVALID_HANDLE;
	}

	return vl_handle;
}
#endif // HAVE_DAB

/***************************
 *
 * etalDemo_userNotificationHandler
 *
 **************************/
static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{

	if (dwEvent == ETAL_INFO_TUNE)
	{
		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune info event, Frequency %d, good station found %d\n",
						status->m_stopFrequency,
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND);

		if (((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND)
			||
			(status->m_stopFrequency == etaldemo_seek_frequency_start)
			)
			{
			etalDemo_PostEvent(ETALDEMO_EVENT_TUNED_RECEIVED);
			}

	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Unexpected event %d\n", dwEvent);
	}
}

/***************************
 *
 * etalDemo_parseParameters
 *
 **************************/
int etalDemo_parseParameters(int argc, char **argv, tU32 *pO_freq1, tU32 *pO_freq2, tU32 *pO_freq3, tU32 *pO_freq4)
{
	int i;
#define ETAL_DEMO_BAD_PARAM -1
#define ETAL_DEMO_RETURN_OK 0


	int vl_res = ETAL_DEMO_RETURN_OK;

	etaldemo_OneDABChannel = false;

// We need the 4 frequency parameters
	if (argc < 5)
	{
		vl_res = ETAL_DEMO_BAD_PARAM;
	}
	else
	{
		for (i=1; i<argc; i++)
		{
			switch (i)
			{
				case 1:
					*pO_freq1 = atoi(argv[i]);
					break;
				case 2:
					*pO_freq2 = atoi(argv[i]);
					break;
				case 3:
					*pO_freq3 = atoi(argv[i]);
					break;
				case 4:
					*pO_freq4 = atoi(argv[i]);
					break;
				default:
					break;
			}
			if (argv[i][0] == 'd')
			{
				etaldemo_OneDABChannel = true;
			}
		}
	}

	if ( ETAL_DEMO_BAD_PARAM == vl_res)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Usage : %s <freq1> <freq2> <freq3> <freq4> [d]\n", argv[0]);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tfreqX = FM tune on frequency\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tAll fours is needed\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\td = Third frequency will be tuned on DAB\n");
	}

	return vl_res;
}

// event creation
// Init globals
tSInt etalDemo_initEvent()
{
	tSInt vl_res;


	vl_res = OSAL_s32EventCreate ((tCString)"EtalDemo", &etalDemo_EventHandler);

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_initEvent : ret %d\n", vl_res);

	return(vl_res);
}

tSInt etalDemo_PostEvent(tU8 event)
{
	tSInt vl_res;

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_PostEvent : postEvent %d\n", event);

	vl_res = OSAL_s32EventPost (etalDemo_EventHandler,
                       ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);

	return(vl_res);

}

tSInt etalDemo_ClearEvent(tU8 event)
{
	tSInt vl_res;

	// Clear old event if any (this can happen after a stop)

	vl_res = OSAL_s32EventPost (etalDemo_EventHandler,
						  (~((tU32)0x01 << event)), OSAL_EN_EVENTMASK_AND);

	return(vl_res);

}

tSInt etalDemo_ClearEventFlag(OSAL_tEventMask event)
{
	tSInt vl_res;

	
	// Clear old event if any (this can happen after a stop)


	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
						  (~event), OSAL_EN_EVENTMASK_AND);

	return(vl_res);

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
	tU32 vl_freq1, vl_freq2, vl_freq3, vl_freq4;

	if (etalDemo_parseParameters(argc, argv, &vl_freq1, &vl_freq2, &vl_freq3, &vl_freq4))
	{
		return 1;
	}

	etalDemo_Appli_entryPoint(vl_freq1, vl_freq2, vl_freq3, vl_freq4);

	return 0;
}

void etalDemo_Press_Any_Key(void)
{
	char line[256];

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** PRESS ANY KEY ***************\n");

	if (fgets(line, sizeof(line), stdin))
	{
	}
}

// vI_frequency (for tune case)
//
void etalDemo_Appli_entryPoint(tU32 vI_frequency1, tU32 vI_frequency2, tU32 vI_frequency3, tU32 vI_frequency4)
{
	int ret;
	ETAL_HANDLE vl_handle1, vl_handle2, vl_handle3, vl_handle4 = ETAL_INVALID_HANDLE;
	EtalHardwareAttr init_params;
	tBool vl_continue = false;
	char line[256];
//	EtalAudioInterfTy vl_audioIf;

	/*
	 * Initialize ETAL
	 */

	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = etalDemo_userNotificationHandler;

	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_initialize (%d)\n", ret);
		return;
	}

	// create an event to work on notification
	//
	etalDemo_initEvent();

#ifdef HAVE_FM
	vl_handle1 = etalDemo_TuneFMFgRadio(vI_frequency1);
	etalDemo_Press_Any_Key();
	vl_handle2 = etalDemo_TuneFMBgRadio(vI_frequency2);
	etalDemo_Press_Any_Key();
#endif
#ifdef HAVE_DAB
	if (etaldemo_OneDABChannel)
	{
		vl_handle3 = etalDemo_TuneDAB1Radio(vI_frequency3);
		etalDemo_Press_Any_Key();
	}
	else
#endif
	{
#ifdef HAVE_FM
		vl_handle3 = etalDemo_TuneFMFgRadio(vI_frequency3);
		etalDemo_Press_Any_Key();
#endif
	}
#ifdef HAVE_FM
	vl_handle4 = etalDemo_TuneFMBgRadio(vI_frequency4);
#endif

	//Pause before quitting
	do
	{

		vl_continue = false;

		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** PRESS ANY KEY TO QUIT ***************\n");

		if (fgets(line, sizeof(line), stdin))
		{
		}
	}
	while (true == vl_continue);

	/*
	  * Destroy all receivers
	  */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy all receivers\n");
	if ((ret = etal_destroy_receiver(&vl_handle1)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver 1 (%d)\n", ret);
	}
		if ((ret = etal_destroy_receiver(&vl_handle2)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver 2 (%d)\n", ret);
	}
			if ((ret = etal_destroy_receiver(&vl_handle3)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver 3 (%d)\n", ret);
	}
				if ((ret = etal_destroy_receiver(&vl_handle4)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver 4 (%d)\n", ret);
	}

	/*
	 * Final cleanup
	 */

	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_deinitialize\n");
	}

	return;
}

