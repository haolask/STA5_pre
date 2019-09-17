//!
//!  \file 		etaldemo_SFSS.c
//!  \brief 	<i><b> ETAL application demo </b></i>
//!  \details   Demo ETAL user application, simulates a radio with Seamless Switching
//!  \author 	Erwan Preteseille
//!

#include "target_config.h"

#include "osal.h"
#ifndef CONFIG_ETAL_HAVE_ETALTML
#error "CONFIG_ETAL_HAVE_ETALTML must be defined"
#else
// manage the compile switches for Servicefollowing : mapped to ETAL
#ifndef CONFIG_ETAL_HAVE_SEAMLESS
#error "CONFIG_ETAL_HAVE_SEAMLESS must be defined"
#endif

#ifndef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#error "CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING must be defined"
#endif

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

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/

/*
 * Adjust ETAL_VALID_*_FREQ to some known good FM/HD station possibly with RDS
 */
#define ETAL_VALID_FM_FREQ     87600
#define ETAL_EMPTY_FM_FREQ     0
#define ETAL_FIRST_FM_FREQ     87500

//#define etalDemoPrintf(level, ...)	OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_DABMW_SF,  __VA_ARGS__)

#define etalDemoPrintf(level, ...) \
	do { printf(__VA_ARGS__); } while (0)
	
//do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __FUNCTION__ "(): "__VA_ARGS__); } while (0)
//	do { printf(__VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)


#define ETAL_DEMO_SEEK_DAB_WAIT_TIME	(200*1000)
#define ETAL_DEMO_SEEK_FM_WAIT_TIME		30000


#define ETAL_DEMO_SID_TABLE_SIZE		15
/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB_FREQ    225648
#define ETAL_EMPTY_DAB_FREQ    0
#define ETAL_START_DAB_FREQ    174928

#define ETAL_DAB_DEFAULT_SID 0x1D12

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
#define NB_SEC_TO_SLEEP_BEFORE_END	60
#define USLEEP_NB_SEC_END	NB_SEC_TO_SLEEP_BEFORE_END * USLEEP_ONE_SEC

// nominal step default
#define ETAL_DEMO_SF_SEEK_STEP 100

/* EPR CHANGE 
* ADD EVENT for  WAKE-UP
*/
/* EVENT for PI reception and triggering SF */
/* EVENT for PI reception and triggering SF */
#define  ETALDEMO_EVENT_TUNED_RECEIVED    		    0
#define  ETALDEMO_EVENT_SF_NOTIF_RECEIVED			1


/* Add a number of event */
#define  ETALDEMO_LAST_EVENT	ETALDEMO_EVENT_SF_NOTIF_RECEIVED
/* END EPR CHANGE */

// EVENT FLAGS 
#define  ETALDEMO_EVENT_TUNED_RECEIVED_FLAG           ((tU32)0x01 << ETALDEMO_EVENT_TUNED_RECEIVED)
#define  ETALDEMO_EVENT_SF_NOTIF_RECEIVED_FLAG        ((tU32)0x01 << ETALDEMO_EVENT_SF_NOTIF_RECEIVED)


#define ETALDEMO_EVENT_WAIT_MASK	(ETALDEMO_EVENT_TUNED_RECEIVED_FLAG | ETALDEMO_EVENT_SF_NOTIF_RECEIVED_FLAG )



/*****************************************************************
| variables;
|----------------------------------------------------------------*/



tU32 Sid;


//ETAL_HANDLE hDatapathAudio_dab_1;
ETAL_HANDLE hCurrentReceiver;

static ETAL_HANDLE hDatapathData = ETAL_INVALID_HANDLE;


tBool etaldemo_seek_on_going; 
tU32 etaldemo_seek_frequency_start;


OSAL_tEventHandle etalDemo_EventHandler;
OSAL_tEventMask etalDemo_EventMask;

EtalDataPathAttr dataPathAttr;

// function 

static ETAL_HANDLE etalDemo_FMSeekRadio(tU32 vI_StartFreq);

static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq);


static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq);

static ETAL_HANDLE etalDemo_DABRadioSeek(tU32 vI_StartFreq);


static tSInt etalDemoSeek_CmdWaitResp(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, OSAL_tMSecond response_timeout);


static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus);

static int etalDemo_parseParameters(int argc, char **argv, tBool *pO_FM_first, tBool *pO_isSeek, tU32 *pO_freq);
static tVoid etalDemoEndTest();

static void etalDemo_SF_entryPoint(tBool vI_FMfirst, tBool vI_modeIsSeek, tU32 vI_frequency);

static void etalDemo_printRadioText(char * prefix, EtalTextInfo *radio_text);
static void etalDemo_RadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

static tVoid etalDemo_SFEventHandling(EtalTuneServiceIdStatus *pI_SFInformation);

static void etalDemo_ConfigureAudioForTuner();
	


// event
// event creation
// Init globals
tSInt etalDemo_initEvent();
tSInt etalDemo_PostEvent(tU8 event);
tSInt etalDemo_ClearEvent(tU8 event);
tSInt etalDemo_ClearEventFlag(OSAL_tEventMask event);



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
			return "HD_FM";
		case ETAL_BCAST_STD_HD_AM:
			return "HD_AM";
		case ETAL_BCAST_STD_AM:
			return "AM";
		default:
			return "Illegal";
	}
}

#ifdef HAVE_FM

/***************************
 *
 * etalDemo_FMRadio
 *
 **************************/
static ETAL_HANDLE etalDemo_FMSeekRadio(tU32 vI_StartFreq)
{
	EtalReceiverAttr attr;
	char line[256];
	ETAL_HANDLE vl_handlefm = ETAL_INVALID_HANDLE;
	etalSeekDirectionTy vl_direction;
    EtalProcessingFeatures proc_features;



	ETAL_STATUS ret;
	tU32 seekFreq;
	tBool vl_continue = false;

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO TUNING ***************\n");

	if ((ret = etaltml_getFreeReceiverForPath(&vl_handlefm, ETAL_PATH_NAME_FM_FG, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath FM ERROR\n");
		return ETAL_INVALID_HANDLE;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath fm ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	
	}


	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set FM band\n");
        proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_change_band_receiver(vl_handlefm, ETAL_BAND_FMEU, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver FM ERROR");
			return ETAL_INVALID_HANDLE;
		}
		else
			{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver fm ok\n");
			}
	/*
	 * Tune to an FM station
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to fm freq %d\n", vI_StartFreq);
	if ((ret = etal_tune_receiver(vl_handlefm, vI_StartFreq)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver fm (%d)\n", ret);
		return ETAL_INVALID_HANDLE;
	}
	else
			{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver fm ok, freq = %d\n", vI_StartFreq);
			}

	if ((ret = etal_audio_select(vl_handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select fm error" );
		return ETAL_INVALID_HANDLE;
	}


	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE DONE ***************\n");

	
	do
		{

		vl_continue = false;


		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONITNUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'u' -> to seek up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'd' -> to seek down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t anyother -> to continue test with SF **** \n");


		if (fgets(line, sizeof(line), stdin))
		{
			if ((line[0] == 'u') || (line[0] == 'd'))
			{
				vl_continue = true;

				if (line[0] == 'u')
				{
					vl_direction = cmdDirectionUp;
				}
				else
				{
					vl_direction = cmdDirectionDown;
				}
				
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));


				etalDemoSeek_CmdWaitResp(vl_handlefm, vl_direction, ETAL_DEMO_SEEK_FM_WAIT_TIME);

				if ((ret = etal_autoseek_stop(vl_handlefm, lastFrequency)) != ETAL_RET_SUCCESS)
					{
						etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
						return 1;
					}


				etal_get_receiver_frequency(vl_handlefm, &seekFreq);


				// get the new ensemble id
				etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);


				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SEEKING END ***************\n");
				
				}
			}
	
		} while((true == vl_continue));
		


	return vl_handlefm;
}


static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq)
{
	EtalReceiverAttr attr_fm;
	ETAL_HANDLE vl_handle;
	ETAL_STATUS ret;
    EtalProcessingFeatures proc_features;


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
    proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_FMEU, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
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


	if ((ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select fm error" );
		return ETAL_INVALID_HANDLE;
	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE FM FG DONE ***************\n");



	return vl_handle;
}
#endif // HAVE FM


#ifdef HAVE_DAB



/***************************
 *
 * etalDemo_DABRadio
 *
 **************************/
static ETAL_HANDLE etalDemo_DABRadioSeek(tU32 vI_StartFreq)
{
	EtalReceiverAttr attr;
//	EtalDataPathAttr dataPathAttr;

//	ETAL_HANDLE hDatapathAudio;
	ETAL_HANDLE handleDab;
	ETAL_STATUS ret;
	etalSeekDirectionTy vl_direction;



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
	tU32 seekFreq;
	tBool vl_continue = false;
    EtalProcessingFeatures proc_features;

	

	/*
	 * Create a receiver configuration
	 */

		
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO TUNING ***************\n");
		
		if ((ret = etaltml_getFreeReceiverForPath(&handleDab, ETAL_PATH_NAME_DAB_1, &attr)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath DAB ERROR\n");
			return OSAL_ERROR;
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
												attr.m_Standard, 
												attr.m_FrontEndsSize,
												attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
		
		}


		/*
		 * create a  datapath
		 */
/*
		etalDemoPrintf(TR_LEVEL_COMPONENT, "creating a datapath\n");
		hDatapathAudio = ETAL_INVALID_HANDLE;
		dataPathAttr.m_receiverHandle = handleDab;
		dataPathAttr.m_dataType = ETAL_DATA_TYPE_AUDIO;
		memset(&dataPathAttr.m_sink, 0x00, sizeof(EtalSink));
		if ((ret = etal_config_datapath(&hDatapathAudio, &dataPathAttr)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_config_datapath (%d)", ret);
			return 1;
		}
*/

/*
	hReceiver = ETAL_INVALID_HANDLE;
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Create DAB receiver\n");
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1;
	if ((ret = etal_config_receiver(&handleDab, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_config_receiver dab (%d)\n", ret);
		return 1;
	}

*/

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");
        proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_change_band_receiver(handleDab, ETAL_BAND_DAB3, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
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

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_StartFreq);
	if (((ret = etal_tune_receiver(handleDab, vI_StartFreq)) != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);

	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB ok, freq = %d\n", vI_StartFreq);

	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE DONE ***************\n");

	// now the seek part


	Sleep(USLEEP_ONE_SEC);

	do
		{

		vl_continue = false;


		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONITNUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'u' -> to seek up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'd' -> to seek down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t anyother -> to continue test with SF **** \n");

		if (fgets(line, sizeof(line), stdin))
		{
			if ((line[0] == 'u') || (line[0] == 'd'))
			{
				vl_continue = true;

				if (line[0] == 'u')
				{
					vl_direction = cmdDirectionUp;
				}
				else
				{
					vl_direction = cmdDirectionDown;
				}
				
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));


				etalDemoSeek_CmdWaitResp(handleDab, vl_direction, ETAL_DEMO_SEEK_DAB_WAIT_TIME);

				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** GET ENSEMBLE DATA ***************\n");


				etal_get_receiver_frequency(handleDab, &seekFreq);

				if ((ret = etal_autoseek_stop(handleDab, lastFrequency)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
				}


				// get the new ensemble id
				etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);

				// get the new ensemble id
				etalDemoPrintf(TR_LEVEL_COMPONENT, "Reading the current ensemble\n");

				// wait the dab to be ready before tune
				//
				Sleep(USLEEP_ONE_SEC *2);

				if ((ret = etal_get_current_ensemble(handleDab, &ueid)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)", ret);
				}
				else
				{
					// get the new ensemble id
					etalDemoPrintf(TR_LEVEL_COMPONENT, "current ensemble 0x%x\n", ueid );

					if (ueid != ETAL_INVALID_UEID)
					{
						if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
							{
								etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
								return 1;
							}


						
						etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s, frequency is %d\n", label, seekFreq);

						/*
						 * A DAB ensemble contains one or more services: fetch the list
						 * and present it to the user for selection. Some services will be
						 * audio only, others data only. The ETAL interface permits to select
						 * which service list to fetch, here we fetch only the audio services
						 */
						if ((ret = etal_get_service_list(handleDab, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
							return 1;
						}

						etalDemoPrintf(TR_LEVEL_COMPONENT, "found: %d services \n", serv_list.m_serviceCount);
						
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
								etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, ueid, serv_list.m_service[i] );
							}
							etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);

						}

						etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");
							

						if (fgets(line, sizeof(line), stdin))
						{
							if (1 == sscanf(line, "%d", &Service))
							{
								/* i can be safely used */
								Service--;
							}
						}

						etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n", 
											Service+1, 
											serv_list.m_service[Service], svinfo.m_serviceLabel);
						if ((ret = etal_service_select_audio(handleDab, ETAL_SERVSEL_MODE_SERVICE, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
							return 1;
						}
					}


				}
				
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO SEEKING END ***************\n");
				
				}
	
			}
		} while((true == vl_continue));
		
	if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select fm error" );
		return ETAL_INVALID_HANDLE;
	}

	return handleDab;
}


/***************************
 *
 * etalDemo_DABRadio
 *
 **************************/
static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq)
{
	EtalReceiverAttr attr_dab_1;
//	EtalDataPathAttr dataPathAttr;

	ETAL_HANDLE vl_handle;


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
	EtalProcessingFeatures proc_features;

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
        proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_DAB3, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
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
	if (((ret = etal_tune_receiver(vl_handle, vI_freq)) != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
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
	hDatapathAudio_dab_1 = ETAL_INVALID_HANDLE;
	dataPathAttr.m_receiverHandle = vl_handle;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_AUDIO;
	memset(&dataPathAttr.m_sink, 0x00, sizeof(EtalSink));
	if ((ret = etal_config_datapath(&hDatapathAudio_dab_1, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_config_datapath for Service Select (%d)", ret);
		return 1;
	}
*/

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
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_list (%d)", ret);
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
			etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d)", ret);
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);

	
	}

	if (false == vl_selectedSidPresent)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");
		

		if (fgets(line, sizeof(line), stdin))
		{
			if (1 == sscanf(line, "%d", &Service))
			{
				/* i can be safely used */
				Service--;
			}
		}
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
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
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


static tVoid etalDemoEndTest()
{

	static tBool vl_destroyed = false;

    ETAL_STATUS ret;


	if (false == vl_destroyed)
	{
	
		
		/*
		 * Destroy all receivers
		 */

				
			etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy receiver\n");
			if ((ret = etal_destroy_receiver(&hCurrentReceiver)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver DAB (%d)\n", ret);
			}


		vl_destroyed = true;	
	}
	

	return;
}




/***************************
 *
 * etalDemoSeamlessEstimation_CmdWaitResp sends seamless estimation command and wait response
 *
 **************************/
tSInt etalDemoSeek_CmdWaitResp(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, OSAL_tMSecond response_timeout)
{
    ETAL_STATUS ret;
	tSInt vl_res = OSAL_OK;
    OSAL_tEventMask vl_resEvent;
	
	  etalDemoPrintf(TR_LEVEL_COMPONENT, "***************  RADIO SEEKING ***************\n");
	  
	  	
	  if ((ret = etal_autoseek_start(receiver, vI_direction, ETAL_DEMO_SF_SEEK_STEP, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
	  {
		  etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR:etalDemoSeek_CmdWaitResp  etal_service_seek (%d)", ret);
		  return 1;
	  }
  
#if 0
    /* wait response_timeout ms for the command response */
    etaldemo_seek_on_going = true;
    while ((response_timeout > 0) && (etaldemo_seek_on_going != false)) {
        if (response_timeout >= 1000) {
            /* wait 1 s */
            OSAL_s32ThreadWait(1000);
            response_timeout -= 1000;
        }
        else {
            OSAL_s32ThreadWait(response_timeout);
            response_timeout = 0;
        }
    }

    /* check command response timeout */
    if (etaldemo_seek_on_going != false)
    {
        etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response timeout");
        return OSAL_ERROR_TIMEOUT_EXPIRED;
    }
#endif

	etalDemo_ClearEvent(ETALDEMO_EVENT_TUNED_RECEIVED);

	// Wait here 
	 vl_res = OSAL_s32EventWaitForDemo (etalDemo_EventHandler,
							  ETALDEMO_EVENT_WAIT_MASK, 
							  OSAL_EN_EVENTMASK_OR, 
							  response_timeout,
							  &vl_resEvent);
	

	etalDemo_ClearEventFlag(vl_resEvent);

	if (OSAL_ERROR == vl_res) 
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response error");
		}	
	
	if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res) 
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response timeout");
		}	
	else if (vl_resEvent == ETALDEMO_EVENT_TUNED_RECEIVED_FLAG)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response event received \n");
		}
	else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response event received 0x%x, vl_res = %d\n", vl_resEvent, vl_res);
		}
	
    return vl_res;
}


// etalDemo_SFEventHandling
static tVoid etalDemo_SFEventHandling(EtalTuneServiceIdStatus *pI_SFInformation)
{

	// Print information
	static ETAL_HANDLE hDatapathData = ETAL_INVALID_HANDLE;
	EtalDataPathAttr dataPathAttr;
	ETAL_STATUS ret;
	EtalTextInfo vl_Radiotext;


	if (false == pI_SFInformation->m_IsFoundstatus)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : Service Not Found \n"); 
		return;
	}
	else
	{
		if ((tU32)0x00FFFFFF != pI_SFInformation->m_Ueid)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : DAB SELECTED\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t Frequency %d, Ueid = 0x%x, Sid = 0x%x\n", 
					pI_SFInformation->m_freq,
					pI_SFInformation->m_Ueid,
					pI_SFInformation->m_SidPi);
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : FM SELECTED\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t Frequency %d, Sid = 0x%x\n", 
					pI_SFInformation->m_freq,
					pI_SFInformation->m_SidPi);
		}

		// manage the RadioText
		
		/*
		 * [Optional] set up an event to get signalled when new Radio Text Data are available.
		 * since the (dynamic info
		 */
		if (ETAL_INVALID_HANDLE != pI_SFInformation->m_receiverHandle)
		{
			// stop prior radioText
			etaltml_stop_textinfo(pI_SFInformation->m_receiverHandle);
		}

        if (ETAL_INVALID_HANDLE != hDatapathData)
        {
			// destroy the datapath
			etal_destroy_datapath(&hDatapathData);
		}
		
		hDatapathData = ETAL_INVALID_HANDLE;
		memset(&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
		dataPathAttr.m_receiverHandle = pI_SFInformation->m_receiverHandle;
		dataPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
		dataPathAttr.m_sink.m_context = (void *)0;
		dataPathAttr.m_sink.m_BufferSize = sizeof(EtalTextInfo);
		dataPathAttr.m_sink.m_CbProcessBlock = etalDemo_RadiotextCallback;
		
		if ((ret = etal_config_datapath(&hDatapathData, &dataPathAttr)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etal_config_datapath for Radiotext (%d)", ret);
			return;
		}

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
		if ((ret = etaltml_start_textinfo(pI_SFInformation->m_receiverHandle)) != ETAL_RET_SUCCESS)
		{
			printf("ERROR: etaltml_start_textinfo (%d)", ret);
			return;
		}


		// radio Text notifcation will happen only if somehting new
		// so get current
		etaltml_get_textinfo(pI_SFInformation->m_receiverHandle, &vl_Radiotext);
		etalDemo_printRadioText("  etalDemo_currentRadioTextInfo : ", &vl_Radiotext);
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	
	}

	

}
	


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
			// found or end of  the seek
			
			etaldemo_seek_on_going = false;
			etalDemo_PostEvent(ETALDEMO_EVENT_TUNED_RECEIVED);
			}

	}
	else if (dwEvent == ETAL_INFO_SEEK)
	{

		EtalSeekStatus *status = (EtalSeekStatus *)pstatus;
		etalDemoPrintf(TR_LEVEL_COMPONENT, "AutoSeek info event, Frequency %d, good station found %d, status = %d\n", 
						status->m_frequency, 
						status->m_frequencyFound,
						status->m_status);
		
		if (((true == status->m_frequencyFound)
			||
			(status->m_frequency == etaldemo_seek_frequency_start)
			||
			(true == status->m_fullCycleReached))
			||
			(ETAL_SEEK_FINISHED == status->m_status)
			)
			{
			// found or end of  the seek
			
			etaldemo_seek_on_going = false;
			etalDemo_PostEvent(ETALDEMO_EVENT_TUNED_RECEIVED);
			}

	}
	else if (dwEvent == ETAL_INFO_SEAMLESS_ESTIMATION_END)
	{
	
		EtalSeamlessEstimationStatus *seamless_estimation_status = (EtalSeamlessEstimationStatus *)pstatus;
	
		if (NULL != pstatus)
			{
	
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_userNotificationHandler : Seamless estimation complete, Status=%d\n",
					seamless_estimation_status->m_status);
			}

    
	}
	else if (dwEvent == ETAL_INFO_SEAMLESS_SWITCHING_END)
	{
		EtalSeamlessSwitchingStatus *seamless_switching_status = (EtalSeamlessSwitchingStatus *)pstatus;

		if (NULL != pstatus)
			{
	
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_userNotificationHandler : Seamless switching complete, status=%d\n",
					seamless_switching_status->m_status);
			}

   
	}
	else if (dwEvent == ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO)
	{

		EtalTuneServiceIdStatus *pl_TuneServiceIdStatus = (EtalTuneServiceIdStatus *) pstatus;
		
		etalDemo_SFEventHandling(pl_TuneServiceIdStatus);

		hCurrentReceiver = pl_TuneServiceIdStatus->m_receiverHandle;
		
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
int etalDemo_parseParameters(int argc, char **argv, tBool *pO_FM_first, tBool *pO_isSeek, tU32 *pO_freq)
{
#define ETAL_DEMO_BAD_PARAM -1
#define ETAL_DEMO_RETURN_OK 0


	int vl_res = ETAL_DEMO_RETURN_OK;

	if (argc >= 3)
	{
		if (argv[1][0] == 'f')
		{
			*pO_FM_first = true;
		}
		else if (argv[1][0] == 'd')
		{
			*pO_FM_first = false;
		}
		else
		{
			vl_res = ETAL_DEMO_BAD_PARAM;
		}

		if (argv[2][0] == 's')
		{
			*pO_isSeek = true;
		}
		else if (argv[2][0] == 't')
		{
			if (argc >= 3)
			{				
			*pO_isSeek = false;
			*pO_freq = atoi(argv[3]);
			}
			else
			{
			vl_res = ETAL_DEMO_BAD_PARAM;	
			}
		}
		else
		{
			vl_res = ETAL_DEMO_BAD_PARAM;
		}

	
	}
	else
	{
		vl_res = ETAL_DEMO_BAD_PARAM;
	}


	if ( ETAL_DEMO_BAD_PARAM == vl_res)
		{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Usage 1 : %s [f|d] [s|t <freq>]\n", argv[0]);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tf = FM radio as start\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\td = DAB radio as start\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\ts = seek selection \n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tt = selection by tuning on frequency <freq>\n");
		}
	else
		{
			if (true == *pO_isSeek)
				{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer %s, seek method \n",
						((true == *pO_FM_first)?"FM":"DAB"));
				}
			else
				{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t-->  bearer %s, tune method on freq %d\n",
						((true == *pO_FM_first)?"FM":"DAB"), *pO_freq);
				}
		}
	
	return vl_res;
}

// event creation
// Init globals
tSInt etalDemo_initEvent()
{
	tSInt vl_res;


	vl_res = OSAL_s32EventCreate ((tCString)"EtalDemo", &etalDemo_EventHandler);

	etalDemo_EventMask = ETALDEMO_EVENT_WAIT_MASK;

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
 * printRadioText
 *
 **************************/
static void etalDemo_printRadioText(char * prefix, EtalTextInfo *radio_text)
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

void etalDemo_RadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemo_printRadioText("  etalDemo_RadioTextInfo : ", (EtalTextInfo *) pBuffer);
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
	tBool vl_FMfirst;	
	tBool vl_isSeek;
	tU32 vl_freq;

	
	if (etalDemo_parseParameters(argc, argv, &vl_FMfirst, &vl_isSeek, &vl_freq))
	{
		return 1;
	}


	etalDemo_ConfigureAudioForTuner();
	


	etalDemo_SF_entryPoint(vl_FMfirst, vl_isSeek, vl_freq);

	return 0;
}

// vI_FMfirst : indicate if we start in FM or DAB
// vI_modeIsSeek : indicates if we start by a tune or a seek
// vI_frequency (for tune case)
//
void etalDemo_SF_entryPoint(tBool vI_FMfirst, tBool vI_modeIsSeek, tU32 vI_frequency)
{
	int ret;
	ETAL_HANDLE vl_handle;
	EtalHardwareAttr init_params;
	tBool vl_continue = false;
	char line[256];	
	EtalTextInfo vl_Radiotext;
	
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
		
	
	if (true == vI_FMfirst)
	{
#ifdef HAVE_FM
		if (true == vI_modeIsSeek)
		{
			vl_handle = etalDemo_FMSeekRadio(ETAL_FIRST_FM_FREQ);
		}
		else
		{
			vl_handle = etalDemo_TuneFMFgRadio(vI_frequency);
		}

		// start the RDS
		//
		if ((ret = etal_start_RDS(vl_handle, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_start_RDS (%d)", ret);
			return;
		}
			
#endif
	}
	else 
	{
#ifdef HAVE_DAB
		if (true == vI_modeIsSeek)
		{			
			vl_handle = etalDemo_DABRadioSeek(ETAL_START_DAB_FREQ);
		}
		else
		{
			vl_handle = etalDemo_TuneDAB1Radio(vI_frequency);
		}
#endif
	}
		

	hCurrentReceiver = vl_handle;




	// manage the RadioText
			
		/*
			 * [Optional] set up an event to get signalled when new Radio Text Data are available.
			 * since the (dynamic info
			 */
			if (ETAL_INVALID_HANDLE != vl_handle)
			{
				// stop prior radioText
				etaltml_stop_textinfo(vl_handle);
			}
	
            if (ETAL_INVALID_HANDLE != hDatapathData)
            {
				// destroy the datapath
				etal_destroy_datapath(&hDatapathData);
        	}
			
			hDatapathData = ETAL_INVALID_HANDLE;
			memset(&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
			dataPathAttr.m_receiverHandle = vl_handle;
			dataPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
			dataPathAttr.m_sink.m_context = (void *)0;
			dataPathAttr.m_sink.m_BufferSize = sizeof(EtalTextInfo);
			dataPathAttr.m_sink.m_CbProcessBlock = etalDemo_RadiotextCallback;
			
			if ((ret = etal_config_datapath(&hDatapathData, &dataPathAttr)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_config_datapath for Radiotext (%d)", ret);
				return;
			}
	
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
			if ((ret = etaltml_start_textinfo(vl_handle)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etaltml_start_textinfo (%d)", ret);
				return;
			}
	
	
			// radio Text notifcation will happen only if somehting new
			// so get current
			etaltml_get_textinfo(vl_handle, &vl_Radiotext);
			etalDemo_printRadioText("  etalDemo_currentRadioTextInfo : ", &vl_Radiotext);
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

	//now activate the SF
	do
	{

		vl_continue = false;

		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONITNUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'f' -> to activate SF\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t anyother -> end test **** \n");


		if (fgets(line, sizeof(line), stdin))
		{
			if (line[0] == 'f')
			{
			vl_continue = true; 	
			
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF activation ***************\n");
	
			
			if ((ret = etaltml_ActivateServiceFollowing()) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etaltml_ActivateServiceFollowing (%d)", ret);
					return;
				}
	
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF activated ***************\n");
			
			}
		}
	}
	while (true == vl_continue);

		

	// destroy handles
	etalDemoEndTest();
	
		/*
		 * Final cleanup
		 */
	
		if (etal_deinitialize() != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_deinitialize\n");
		}
	


	return;
}


// Configure the audio path for Tuner
//
static void etalDemo_ConfigureAudioForTuner()
{

	// this is based on alsa mixer in Linux A2.
	// This requires the appropriate audio fwk
	
	// select the audio source
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");	

	// Set the output volume 
	system("amixer -c 3 sset \"Volume Master\" 1100 > /dev/null");
}



