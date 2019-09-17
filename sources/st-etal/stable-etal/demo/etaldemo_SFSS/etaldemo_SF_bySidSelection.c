//!
//!  \file 		etaldemo_SFSS.c
//!  \brief 	<i><b> ETAL application demo </b></i>
//!  \details   Demo ETAL user application, simulates a radio with Seamless Switching
//!  \author 	Erwan Preteseille
//!

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

#include <unistd.h>  // usleep

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/

#define	ETAL_DEMO_SF_SF_WAIT_TIME 	120000

/*
 * Adjust ETAL_VALID_*_FREQ to some known good FM/HD station possibly with RDS
 */
#define ETAL_VALID_FM_FREQ     87600
#define ETAL_EMPTY_FM_FREQ     0

#define etalDemoPrintf	printf

#define ETAL_DEMO_SEEK_DAB_WAIT_TIME	30000
#define ETAL_DEMO_SEEK_FM_WAIT_TIME		30000

#define ETAL_DEMO_SID_TABLE_SIZE		15
/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB_FREQ    225648
#define ETAL_EMPTY_DAB_FREQ    0
#define ETAL_DAB_DEFAULT_SID 0x1D12

#define PREFIX0 "    "
#define PREFIX1 "        "

#define USLEEP_ONE_SEC        1000000
#define NB_SEC_TO_SLEEP_BEFORE_END	60
#define USLEEP_NB_SEC_END	NB_SEC_TO_SLEEP_BEFORE_END * USLEEP_ONE_SEC



/* EPR CHANGE 
* ADD EVENT for  WAKE-UP
*/
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



ETAL_HANDLE handlefm_fg, handledab_1, handle;
ETAL_HANDLE hDatapathAudio_dab_1;

tBool etaldemo_seek_on_going; 
tU32 etaldemo_seek_frequency_start;


OSAL_tEventHandle etalDemo_EventHandler;
OSAL_tEventMask etalDemo_EventMask;


static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus);
static int etalDemo_parseParameters(int argc, char **argv, tBool *pO_FM_First, tU32 *pO_Sid);

static tVoid etalDemoEndTest();

static int etalDemo_SF(tBool vI_fmFirst, tU32 vI_Sid);
static tSInt etalDemoSF_TuneOnSid_CmdWaitResp(OSAL_tMSecond response_timeout);

static tVoid etalDemo_SFEventHandling(EtalTuneServiceIdStatus *pI_SFInformation);

static void etalDemo_printRadioText(char * prefix, EtalTextInfo *radio_text);
static void etalDemo_RadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

static void etalDemo_SF_entryPoint(tBool vI_FMfirst, tU32 vI_Sid);

static void etalDemo_ConfigureAudioForTuner();


// event
// event creation
// Init globals
tSInt etalDemo_initEvent();
tSInt etalDemo_PostEvent(tU8 event);
tSInt etalDemo_ClearEvent(tU8 event);

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

static int etalDemo_SF(tBool vI_fm, tU32 vI_Sid)
{
	EtalPathName vl_path;
	char line[256];
	
	if (true == vI_fm)
	{
		vl_path = ETAL_PATH_NAME_FM_FG;
	}
	else
	{
		vl_path = ETAL_PATH_NAME_DAB_1;
	}

	etalDemoPrintf("etalDemo_SF() : etaltml_TuneOnServiceId request path = %d \n", vl_path);


	if(OSAL_OK != etaltml_TuneOnServiceId(vl_path,  vI_Sid))
	{
		etalDemoPrintf("etalDemo_SF() : etaltml_TuneOnServiceId returned error\n");
		return OSAL_ERROR;
	}

	etalDemoPrintf("etalDemo_SF() : etaltml_TuneOnServiceId requested \n");

	// now wait for the response
	//
	etalDemoSF_TuneOnSid_CmdWaitResp(ETAL_DEMO_SF_SF_WAIT_TIME);

	// wait key
	
	etalDemoPrintf("*** enter a key to end the tune \n");
		

	fgets(line, sizeof(line), stdin);


	/*
	 * Demo end
	 */
	etalDemoPrintf("*************** END OF TEST ***************\n");

	return OSAL_OK;
}

static tSInt etalDemoSF_TuneOnSid_CmdWaitResp(OSAL_tMSecond response_timeout)
{
 	
	tSInt vl_res = OSAL_OK;
    OSAL_tEventMask vl_resEvent = 0;

    etalDemoPrintf("*etalDemoSF_TuneOnSid_CmdWaitResp \n"); 

 
	// Wait here 

	etalDemo_ClearEvent(ETALDEMO_EVENT_SF_NOTIF_RECEIVED);
	
	 vl_res = OSAL_s32EventWait (etalDemo_EventHandler,
							  ETALDEMO_EVENT_SF_NOTIF_RECEIVED_FLAG, 
							  OSAL_EN_EVENTMASK_OR, 
							  response_timeout,
							  &vl_resEvent);
	

	etalDemo_ClearEvent(ETALDEMO_EVENT_SF_NOTIF_RECEIVED);

	if (OSAL_ERROR == vl_res) 
		{
		etalDemoPrintf("etalDemoSF_TuneOnSid_CmdWaitResp wait event error\n");
		}
	else if ((OSAL_ERROR_TIMEOUT_EXPIRED == vl_res) || (vl_resEvent == 0))
		{
			etalDemoPrintf("etalDemoSF_TuneOnSid_CmdWaitResp response timeout\n");
		
		}	
	else if (vl_resEvent == ETALDEMO_EVENT_SF_NOTIF_RECEIVED_FLAG)
		{
			etalDemoPrintf("etalDemoSF_TuneOnSid_CmdWaitResp response event received \n");
		}	
		
	
	return vl_res;
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
		etalDemoPrintf("Tune info event, Frequency %d, good station found %d\n", status->m_stopFrequency, (status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND);
		if (((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND)
			||
			(status->m_stopFrequency == etaldemo_seek_frequency_start))
			{
			// found or end of  the seek
			
			etaldemo_seek_on_going = false;
			etalDemo_PostEvent(ETALDEMO_EVENT_TUNED_RECEIVED);
			}

	}
	else if (dwEvent == ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO)
	{
		EtalTuneServiceIdStatus *pl_TuneServiceIdStatus = (EtalTuneServiceIdStatus *) pstatus;
		
		etalDemo_SFEventHandling(pl_TuneServiceIdStatus);
		
		etalDemo_PostEvent(ETALDEMO_EVENT_SF_NOTIF_RECEIVED);
		handle = pl_TuneServiceIdStatus->m_receiverHandle;
	}
	else
	{
		etalDemoPrintf("Unexpected user notification event %d\n", dwEvent);
	}
}

/***************************
 *
 * etalDemo_parseParameters
 *
 **************************/
int etalDemo_parseParameters(int argc, char **argv, tBool *pO_FM_First, tU32 *pO_Sid)
{
#define ETAL_DEMO_BAD_PARAM -1
#define ETAL_DEMO_RETURN_OK 0


	int vl_res = ETAL_DEMO_BAD_PARAM;
	
	if (argc >= 2)
	{
		if (argv[1][0] == 'f')
		{
	
			if (argc > 2)
				{
				sscanf(argv[2], "%x", &*pO_Sid);
				}
			else
				{
				*pO_Sid = ETAL_DAB_DEFAULT_SID;
				}

			*pO_FM_First = true;

			etalDemoPrintf("\t\t-->  FM path PI = 0x%x\n", *pO_Sid);

			vl_res = ETAL_DEMO_RETURN_OK;
		}
		else if (argv[1][0] == 'd')
		{

	

			if (argc > 2)
				{
				sscanf(argv[2], "%x", &*pO_Sid);
				}
			else
				{
				*pO_Sid = ETAL_DAB_DEFAULT_SID;
				}

			*pO_FM_First = false;

			etalDemoPrintf("\t\t-->  DAB path Sid = 0x%x\n", *pO_Sid);

			vl_res = ETAL_DEMO_RETURN_OK;
		}
	}
	else
	{
		vl_res = ETAL_DEMO_BAD_PARAM;
	}


	
	if ( ETAL_DEMO_BAD_PARAM == vl_res)
		{
		etalDemoPrintf("Usage 1 : %s [f|d][ <sid / PI> \n", argv[0]);
		etalDemoPrintf("\tf = select FM path  (default)\n");
		etalDemoPrintf("\td = select DAB path\n");
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

	etalDemoPrintf("etalDemo_initEvent : ret %d\n", vl_res);
		
	return(vl_res);

 

}

tSInt etalDemo_PostEvent(tU8 event)
{
	tSInt vl_res;
		
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


static tVoid etalDemoEndTest()
{

	static tBool vl_destroyed = false;

    ETAL_STATUS ret;


	if (false == vl_destroyed)
	{
	
		
		/*
		 * Destroy all receivers
		 */

				
			etalDemoPrintf("Destroy receiver\n");
			if ((ret = etal_destroy_receiver(&handle)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf("ERROR: etal_destroy_receiver (%d)\n", ret);
			}

		vl_destroyed = true;	
	}
	

	return;
}

static tVoid etalDemo_SFEventHandling(EtalTuneServiceIdStatus *pI_SFInformation)
{

	// Print information
	static ETAL_HANDLE hDatapathData = ETAL_INVALID_HANDLE;
	EtalDataPathAttr dataPathAttr;
	ETAL_STATUS ret;
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	EtalTextInfo vl_Radiotext;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)


	if (false == pI_SFInformation->m_IsFoundstatus)
	{
		etalDemoPrintf("etalDemo_SFEventHandling : Service Not Found \n"); 
		return;
	}
	else
	{
		if ((tU32)0x00FFFFFF != pI_SFInformation->m_Ueid)
		{
			etalDemoPrintf("etalDemo_SFEventHandling : DAB SELECTED\n");
			etalDemoPrintf("\t\t Frequency %d, Ueid = 0x%x, Sid = 0x%x\n", 
					pI_SFInformation->m_freq,
					pI_SFInformation->m_Ueid,
					pI_SFInformation->m_SidPi);
		}
		else
		{
			etalDemoPrintf("etalDemo_SFEventHandling : FM SELECTED\n");
			etalDemoPrintf("\t\t Frequency %d, Sid = 0x%x\n", 
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
	tU32 vl_Sid;




	if (etalDemo_parseParameters(argc, argv, &vl_FMfirst, &vl_Sid))
	{
		return 1;
	}

	etalDemo_ConfigureAudioForTuner();
	

	/*
	 * Initialize ETAL
	 */
	 etalDemo_SF_entryPoint(vl_FMfirst, vl_Sid);

	return 0;
}

void etalDemo_SF_entryPoint(tBool vI_FMfirst, tU32 vI_Sid)
{
	int ret;
	EtalHardwareAttr init_params;
	
	/*
	 * Initialize ETAL
	 */
	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = etalDemo_userNotificationHandler;
	
	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_initialize (%d)\n", ret);
		return ;
	}


	// create an event to work on notification
	//
	etalDemo_initEvent();
	

	etalDemo_SF(vI_FMfirst, vI_Sid);

	// destroy handles
	etalDemoEndTest();

	/*
	 * Final cleanup
	 */

	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf("ERROR: etal_deinitialize\n");
	}

	return ;
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



