//!
//!  \file      service_following_seamless.c
//!  \brief     <i><b> Service following seamless implementation </b></i>
//!  \details   This file provides functionalities for service following seamless
//!  \author    David Pastor
//!  \author    (original version) David Pastor
//!  \version   1.0
//!  \date      2013.11.04
//!  \bug       Unknown
//!  \warning   None
//!



#ifndef SERVICE_FOLLOWING_SEAMLESS_C
#define SERVICE_FOLLOWING_SEAMLESS_C

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <math.h>

#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"

/*
#include "system_status.h"

#include "audio_mngr.h"

#include "DabMwTask.h"

#include "fic_common.h"

#include "fic_types.h"

#include "fic_broadcasterdb.h"

#include "system_status.h"

#include "audio_mngr.h"

#if defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)
    #include "AMFMDevCmds.h"

    #include "amfmdev_comm.h"
#endif // #if defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)


#if defined (CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
    #include "rds_comm.h"
    #include "rds_mngr.h"
    #include "rds_data.h"
    #include "rds_landscape.h"
#endif // #if defined (CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)

#include "system_app.h"

#include "radio_control.h"


// SD includes 
#include "Signals.h"

#include "MMObject.h"

#include "sd_messages.h"

#include "streamdec_comm.h"

*/

#include "seamless_switching_common.h"

// END SD includes 

#include "service_following.h"

#include "service_following_internal.h"

#include "service_following_seamless.h"

#include "service_following_log_status_info.h"

#include "service_following_audioswitch.h"

tSInt DABMW_ServiceFollowing_SeamlessEstimationStartDemo(tVoid);

/*
*********************
* VARIABLE SECTION
**********************
*/

static SF_msg_SeamlessEstimationResponse dabmw_msg_sf_seamless_report;
static tBool vl_SS_callback_registered = false;  

/*
*********************
*  PROCEDURES SECTION
**********************
*/

/* Procedure to init the SeamLess Parameters */

tVoid DABMW_ServiceFollowing_SeamLessParametersInit()
{
	DABMW_SS_EstimationParameter.FullEstimationStartWindowInMs = DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_MS;
    DABMW_SS_EstimationParameter.FullEstimationStartWindowInSample = DABMW_SEAMLESS_ESTIMATION_CONVERT_MS_IN_SAMPLE(DABMW_SS_EstimationParameter.FullEstimationStartWindowInMs);
    DABMW_SS_EstimationParameter.FullEstimationStopWindowInMs = DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_STOP_IN_MS;
    DABMW_SS_EstimationParameter.FullEstimationStopWindowInSample = DABMW_SEAMLESS_ESTIMATION_CONVERT_MS_IN_SAMPLE(DABMW_SS_EstimationParameter.FullEstimationStopWindowInMs);
    
    DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInMs = DABMW_SF_DEFAULT_CONFIG_SS_RECONF_WINDOWS_HALF_SIZE_IN_MS;
    DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInSample = DABMW_SF_DEFAULT_CONFIG_SS_RECONF_WINDOWS_HALF_SIZE_IN_SAMPLES;
    
	DABMW_SS_EstimationParameter.LoudnessEstimationDurationInSec = DABMW_SF_DEFAULT_CONFIG_SS_LOUDNESS_DURATION_SECOND;
	DABMW_SS_EstimationParameter.DownSampling = DABMW_SF_DEFAULT_CONFIG_SS_DOWN_SAMPLING;
    DABMW_SS_EstimationParameter.DownSamplingReconf = DABMW_SF_DEFAULT_CONFIG_SS_DOWN_SAMPLING_RECONF;

    // 
    DABMW_SS_EstimationParameter.ConfidenceLevelThreshold = DABMW_SF_DEFAULT_CONFIG_SS_CONFIDENCE_LEVEL_THRESHOLD;
    
	/* set the response not received yet */
	DABMW_SF_SS_EstimationResult.IsResponseReceived = false;

	/* set the estimation is on going */
	DABMW_SF_SS_EstimationResult.IsEstimationOnGoing = false;
    //DABMW_SF_SS_EstimationResult.timeLastEstimationDone = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_SF_SS_EstimationResult.timeLastEstimationDone = DABMW_INVALID_DATA;
	
	/* init the response */
	dabmw_msg_sf_seamless_report.status = SD_SEAMLESS_ESTIMATION_STATUS_NONE;

    // timing info
    DABMW_SS_EstimationParameter.measureValidityDuration = DABMW_SF_DEFAULT_CONFIG_SS_MEASURE_VALIDITY;
    DABMW_SS_EstimationParameter.reconfPeriodicity = DABMW_SF_DEFAULT_CONFIG_SS_RECONF_PERIODICY;
    DABMW_SS_EstimationParameter.minimumTimeBetween2Seamless = DABMW_SF_DEFAULT_CONFIG_SS_MINIMUM_SPACING_TIME;


	// stored info
	DABMW_SF_SS_LastSwitchRequestInfo.switchRequestValidity = false;

    //set flag SS action on going.
    DABMW_serviceFollowingStatus.seamless_action_on_going = false;
    
	/* Init database */
	DABMW_ServiceFollowing_SSDatabaseInit();
}

tSInt DABMW_ServiceFollowing_SeamlessEstimationStart()
{
	tSInt vl_res = OSAL_OK;
//	tU8 al_buffer[DABMW_SEAMLESS_ESTIMATION_MSG_LEN];
	static tBool vl_registered = false;

	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
	DABMW_SS_EstimationResutDatabaseTy* pl_ExistingSeamlessReport;
    tU32  vl_dabFrequency;
    tU32  vl_dabEid;
    tU32  vl_dabSid;
    tU32  vl_fmPi;
    
    SF_msg_SeamlessEstimationRequest   vl_seamlessEstimationStart;

#if defined CONFIG_ENABLE_CLASS_APP_DABMW_SF
//#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	// codex #329753 : add a local variable to print absolute_delay_in_sample
	tS32 vl_absolute_estimated_delay_in_samples = 0;
//#endif
#endif // CONFIG_TR_CLASS_APP_DABMW_SF

	/* here we should have : 1 FM & 1 DAB */
	if (vl_originalIsDab == vl_alternateIsDab)
		{
		return OSAL_ERROR;
		}

	/* if DAB is alternate : we should select the service */
	if (true == vl_alternateIsDab)
		{
		/* Orig = FM , AF =  DAB => just a audio port switch */
	
		/* for the moment, configure the output port has being DAB alternate..; so that seamless switch could work
		* that may already be do 
		*/
		// Check service is not already selected
		if (false == DABMW_serviceFollowingStatus.alternate_DabServiceSelected)
		    {
    		/* Select the DAB service */
    		vl_res = DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_serviceFollowingStatus.alternateApp, 									   // Use parameter passed
    										 DABMW_SERVICE_SELECT_SUBFNCT_SET,				// Sub-function is 5
    										 DABMW_serviceFollowingStatus.alternateEid, 	 // Ensemble  
    										 DABMW_serviceFollowingStatus.alternateSid); 	// Service ID		  
  
    		DABMW_SS_EstimationParameter.serviceSelected = true;
		
            
    		/* check : is the service selection ok ?  */
    		if (OSAL_OK == vl_res)
    			{
    			// Service ok  
    			DABMW_serviceFollowingStatus.alternate_DabServiceSelected = true;
    			}
    		else
    			{
    			// should not come here but you never know 
    			DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;
    		    }
		    }
        else
            {
                //
                // nothing to do : the service is already selected... so ready for seamless switching...
                //
            }
		}
						
	/* fill the command in buffer way
	* B0 = MSG TYPE
	
	BYTE 1
		Bits 0 - 1
		SEAMLESS_CMD
	
		Bits 2 - 7
		RFU The SEAMLESS_CMD can be :
		0 - STOP command
		1 - START command
		2 - START delay only command
		3 - START loudness only command
	
	BYTE 2 to 5 = Start position in samples

	BYTE 6 to 10 = Stop position in samples

	BYTE 11 = Down-sampling factor

	BYTE 12 = Minimum loudness duration in seconds
	
	*/

	/* init the buffer */
	DABMW_ServiceFollowing_ExtInt_MemorySet(&vl_seamlessEstimationStart, 0, sizeof(SF_msg_SeamlessEstimationRequest));

	/* Now choose the command to do */
	
	/* have we got pre-store info ?
	*/
	if (true == vl_originalIsDab)
        {
            vl_dabFrequency = DABMW_serviceFollowingStatus.originalFrequency;
            vl_dabEid = DABMW_serviceFollowingStatus.originalEid;
            vl_dabSid = DABMW_serviceFollowingStatus.originalSid;
            vl_fmPi = DABMW_serviceFollowingStatus.alternateSid;
        }
        else
        {
            vl_dabFrequency = DABMW_serviceFollowingStatus.alternateFrequency;
            vl_dabEid = DABMW_serviceFollowingStatus.alternateEid;
            vl_dabSid = DABMW_serviceFollowingStatus.alternateSid;
            vl_fmPi = DABMW_serviceFollowingStatus.originalSid;
        }
        
		// Retrieve the existing result in database 
		// check a valid one exists (with valid date)
		// if so, get it

  	    pl_ExistingSeamlessReport = DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(vl_dabFrequency, 
    											vl_dabEid,
    											vl_dabSid,
    											vl_fmPi, 
    											DABMW_SS_EstimationParameter.measureValidityDuration);
		
	if ((NULL == pl_ExistingSeamlessReport)
		||
		(SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS != pl_ExistingSeamlessReport->SS_EstimationResult.status)
		||
        // codex 304488 : manage the SS error cases.
        // check the error counter threshold : 
        // if we are above, it means that we get a number of consecutive estimation failure... 
        // it may be good to reestimate : we do not trust what we have... 
        (pl_ExistingSeamlessReport->error_counter >= DABMW_SF_SS_THRESHOLD_ERROR_COUNTER)
        )
		{
		/* no or invalid prestore : 
		* full start command
		*/
            
		DABMW_SF_SS_EstimationResult.requestedMode = SD_SEAMLESS_ESTIMATION_START_MODE_1;
		
		/* Msg TYPE */
        vl_seamlessEstimationStart.msg_id = SD_MESSAGE_RX_SEAMLESS_ESTIMATION;
        //		al_buffer[0] = SD_MESSAGE_RX_SEAMLESS_ESTIMATION;

		/* BYTE 0	SEAMLESS_CMD */
        //		al_buffer[1] = DABMW_SEAMLESS_ESTIMATION_START_CMD_MODE_1;
        vl_seamlessEstimationStart.mode = SD_SEAMLESS_ESTIMATION_START_MODE_1;
        
		
		/* BYTE 1 to 4 = Start position in samples */
        /*
		al_buffer[2] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 24) & 0xFF);
		al_buffer[3] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 16) & 0xFF);
		al_buffer[4] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 8) & 0xFF);
		al_buffer[5] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 0) & 0xFF);
		*/
		vl_seamlessEstimationStart.start_position_in_samples = DABMW_SS_EstimationParameter.FullEstimationStartWindowInSample;

		/* BYTE 5 to 9 = Stop position in samples */
		/*al_buffer[6] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 24) & 0xFF);
		al_buffer[7] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 16) & 0xFF);
		al_buffer[8] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 8) & 0xFF);
		al_buffer[9] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 0) & 0xFF);
            */
            
        vl_seamlessEstimationStart.stop_position_in_samples = DABMW_SS_EstimationParameter.FullEstimationStopWindowInSample;

		/* BYTE 10 = Downsampling factor */
		// al_buffer[10] = DABMW_SS_EstimationParameter.DownSampling;
		vl_seamlessEstimationStart.downSampling = DABMW_SS_EstimationParameter.DownSampling;
		
		/* BYTE 11 = Minimum loudness duration in seconds */
		// al_buffer[11] = MINIMUM_LOUDNESS_DURATION;
        vl_seamlessEstimationStart.loudness_duration = DABMW_SS_EstimationParameter.LoudnessEstimationDurationInSec;

        
		}
	else
		{
		/* prestore exist
		* do a MODE 1 for reconfirmation only 
		*/

		DABMW_SF_SS_EstimationResult.requestedMode = SD_SEAMLESS_ESTIMATION_START_MODE_1;
		
		/* Msg TYPE */
		//al_buffer[0] = SD_MESSAGE_RX_SEAMLESS_ESTIMATION;
		// vl_seamlessEstimationStart.msg_id = SD_MESSAGE_RX_SEAMLESS_ESTIMATION;

		/* BYTE 0	SEAMLESS_CMD */
		//al_buffer[1] = DABMW_SEAMLESS_ESTIMATION_START_CMD_MODE_1;
		vl_seamlessEstimationStart.mode = SD_SEAMLESS_ESTIMATION_START_MODE_1;
		
		/* BYTE 1 to 4 = Start position in samples */
		/*al_buffer[2] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 24) & 0xFF);
		al_buffer[3] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 16) & 0xFF);
		al_buffer[4] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 8) & 0xFF);
		al_buffer[5] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 0) & 0xFF);
                */
        vl_seamlessEstimationStart.start_position_in_samples = pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples - DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInSample;

        if (vl_seamlessEstimationStart.start_position_in_samples < DABMW_SS_EstimationParameter.FullEstimationStartWindowInSample)
        {
            vl_seamlessEstimationStart.start_position_in_samples = DABMW_SS_EstimationParameter.FullEstimationStartWindowInSample;
        }

		/* BYTE 5 to 9 = Stop position in samples */
		/*al_buffer[6] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 24) & 0xFF);
		al_buffer[7] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 16) & 0xFF);
		al_buffer[8] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 8) & 0xFF);
		al_buffer[9] = (tU8) ((pl_ExistingSeamlessReport->absolute_estimated_delay_in_samples >> 0) & 0xFF);
                */
        vl_seamlessEstimationStart.stop_position_in_samples = pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples + DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInSample;
        if (vl_seamlessEstimationStart.stop_position_in_samples > DABMW_SS_EstimationParameter.FullEstimationStopWindowInSample)
        {
            vl_seamlessEstimationStart.stop_position_in_samples = DABMW_SS_EstimationParameter.FullEstimationStopWindowInSample;
        }
                
		/* BYTE 10 = Downsampling factor */
		// al_buffer[10] = DABMW_SS_EstimationParameter.DownSampling;
		vl_seamlessEstimationStart.downSampling = DABMW_SS_EstimationParameter.DownSamplingReconf;
	
		/* BYTE 11 = Minimum loudness duration in seconds */
		
        // al_buffer[11] = MINIMUM_LOUDNESS_DURATION;
        vl_seamlessEstimationStart.loudness_duration = DABMW_SS_EstimationParameter.LoudnessEstimationDurationInSec;

#if defined CONFIG_ENABLE_CLASS_APP_DABMW_SF
		vl_absolute_estimated_delay_in_samples = pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples;
#endif // CONFIG_TR_CLASS_APP_DABMW_SF
		}

	// register to call back for response
	if (false == vl_registered)
		{		
		DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessEstimationResponse((DABMW_SF_audio_msgsCallbackTy)DABMW_ServiceFollowing_SeamlessEstimationResponse);
		vl_registered = true;
		}
    
          /*  LOG */
//#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
           DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
           DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                                   "DABMW_Service_Following (SeamlessEstimationStart) : start %d (%d ms), stop %d (%d ms), DownSampling %d, Loudness duration %d s, delay %d (%d ms)\n",
                                    vl_seamlessEstimationStart.start_position_in_samples , DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(vl_seamlessEstimationStart.start_position_in_samples ),
                                    vl_seamlessEstimationStart.stop_position_in_samples , DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(vl_seamlessEstimationStart.stop_position_in_samples ),
                                    vl_seamlessEstimationStart.downSampling,
                                    vl_seamlessEstimationStart.loudness_duration,
                                   // codex #329753 : 
                                    // correction : pointer pl_ExistingSeamlessReport may be null if database if empty
                                    // before pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples, DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples));
                                    // after
                                    vl_absolute_estimated_delay_in_samples, DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(vl_absolute_estimated_delay_in_samples));
                                                        
           DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
//#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
            /* END TMP LOG */

    
	// Send the message to the StreamDecoder of length 6
	vl_res = DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest(vl_seamlessEstimationStart);

    // set flag SS action on going.
    DABMW_serviceFollowingStatus.seamless_action_on_going = true;

	/* set the response not received yet */
	DABMW_SF_SS_EstimationResult.IsResponseReceived = false;
	/* set the estimation is on going */
	DABMW_SF_SS_EstimationResult.IsEstimationOnGoing = true;
    DABMW_SF_SS_EstimationResult.timeLastEstimationDone = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
    
	/* init the response */
	dabmw_msg_sf_seamless_report.status = SD_SEAMLESS_ESTIMATION_STATUS_NONE;

    
     
	return vl_res;
}


/* procedure to request the start of seamless estimation */
tSInt DABMW_ServiceFollowing_SeamlessEstimationStop()
{
	tSInt vl_res = OSAL_OK;

    DABMW_SF_SS_EstimationResult.requestedMode = SD_SEAMLESS_ESTIMATION_STOP_MODE;
    

    
    /*  LOG */
//#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                                       "DABMW_Service_Following (SeamlessEstimationStop) : Stop estimation requested\n");
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
//#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */

	// Send the message to the StreamDecoder
	vl_res = DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationStop();

	/* Clear  any pending event if exist */
	DABMW_ServiceFollowing_ExtInt_TaskClearEvent(DABMW_SF_EVENT_SS_ESTIMATION_RSP);

	/* Set the flag */
	DABMW_SF_SS_EstimationResult.IsEstimationOnGoing = false;
	
	return vl_res;

}

/* Registered function for  SeamLess Estimation Response */
tVoid DABMW_ServiceFollowing_SeamlessEstimationResponse (tPVoid msgPayload)
{
	/* Processing here is to store the result locally 
	* and notify Event Seamless Estimate Received 
	*/

	// do not filter on state : this will be done on event processing
	//
//	if (DABMW_SF_STATE_IDLE_SEAMLESS == DABMW_serviceFollowingStatus.status)
		{
		/* Store the result 
		* msgPayload - 1 is passed : because we need to work on the header..
		*
		*/
		DABMW_ServiceFollowing_ExtInt_MemoryCopy(&dabmw_msg_sf_seamless_report,(SF_msg_SeamlessEstimationResponse *)(msgPayload), sizeof(SF_msg_SeamlessEstimationResponse));

		DABMW_SF_SS_EstimationResult.IsResponseReceived = true;
        DABMW_SF_SS_EstimationResult.timeLastEstimationDone = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

        /*  LOG */
//#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
         DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
         DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                                 "DABMW_Service_Following (SeamlessEstimationResponse) : status %s,  provider_type %s, delay_in_ms %d, absolute_estimated_delay_in_samples %d, delay_estimate %d, timestamp_FAS %d, timestamp_SAS %d, average_RMS2_FAS %d (%2.3f dB), average_RMS2_SAS %d (%2.3f dB), confidence_level %d\n",
                                   DABMW_SF_SEAMLESS_LOG_STATUS(dabmw_msg_sf_seamless_report.status),
                                   DABMW_SF_SEAMLESS_LOG_PROVIDER_TYPE(dabmw_msg_sf_seamless_report.provider_type),
                                   DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(dabmw_msg_sf_seamless_report.absolute_estimated_delay_in_samples),
                                   dabmw_msg_sf_seamless_report.absolute_estimated_delay_in_samples,
                                   dabmw_msg_sf_seamless_report.delay_estimate,
                                   dabmw_msg_sf_seamless_report.timestamp_FAS,
                                   dabmw_msg_sf_seamless_report.timestamp_SAS,
                                   dabmw_msg_sf_seamless_report.average_RMS2_FAS,
                                   DABMW_SEAMLESS_ESTIMATION_CONVERT_LOUDNESS_IN_DB(dabmw_msg_sf_seamless_report.average_RMS2_FAS),
                                   dabmw_msg_sf_seamless_report.average_RMS2_SAS,
                                   DABMW_SEAMLESS_ESTIMATION_CONVERT_LOUDNESS_IN_DB(dabmw_msg_sf_seamless_report.average_RMS2_SAS),
                                   dabmw_msg_sf_seamless_report.confidence_level);
         DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
//#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                    /* END TMP LOG */

        //set flag SS action on going.
        DABMW_serviceFollowingStatus.seamless_action_on_going = false;
        
        /* Set the flag */
    	DABMW_SF_SS_EstimationResult.IsEstimationOnGoing = false;
        

    	/* Notify Event */
    	DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_SS_ESTIMATION_RSP);
    	           
		}
	/*
	else
		{
		// this is an error : not correct state.. 
		}
	*/



}

/* Procedure to init the database */
tVoid DABMW_ServiceFollowing_SSDatabaseInit()
{
	tU8 vl_cnt;

	for (vl_cnt=0;vl_cnt<DABMW_SF_SS_DATABASE_SIZE;vl_cnt++)
		{
		DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Frequency = DABMW_INVALID_FREQUENCY;
		DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Eid = DABMW_INVALID_EID;
		DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Sid = DABMW_INVALID_SERVICE_ID;
		DABMW_SF_SSEstimation_Database[vl_cnt].FM_PI = DABMW_INVALID_SERVICE_ID;
         // codex 304488 : manage the ss error cases
        DABMW_SF_SSEstimation_Database[vl_cnt].error_counter = 0;
		}

	return;
}

/* Procedure to reset a given info in the database 
* return the OSAL_ERROR if not found, else the index in database
*
*/
tSInt DABMW_ServiceFollowing_SSDatabaseReset(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI)
{
	tU8 vl_cnt;
	tU8 vl_storingIndex = DABMW_INVALID_DATA_BYTE;
	tSInt	vl_res = OSAL_ERROR;

	/* Look if already present : if so just update */
	
	for (vl_cnt=0;vl_cnt<DABMW_SF_SS_DATABASE_SIZE;vl_cnt++)
		{
		if ((DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Frequency == vI_DAB_Frequency)
			&&
			(DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Eid == vI_DAB_Eid)
			&&
			(DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Sid == vI_DAB_Sid)
			&&
			(DABMW_SF_SSEstimation_Database[vl_cnt].FM_PI == vI_FM_PI)
			)
			{
			vl_storingIndex = vl_cnt;
			break;
			}
		}
	
	
	/* Ok, now we know where to store 
	*/
	if (DABMW_INVALID_DATA_BYTE != vl_storingIndex)
		{
		DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Frequency = DABMW_INVALID_FREQUENCY;
		DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Eid = DABMW_INVALID_EID;
		DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Sid = DABMW_INVALID_SERVICE_ID;
		DABMW_SF_SSEstimation_Database[vl_cnt].FM_PI = DABMW_INVALID_SERVICE_ID;
        
        // codex 304488 : manage the ss error cases
        DABMW_SF_SSEstimation_Database[vl_cnt].error_counter = 0;
		
		vl_res = vl_cnt;
		}
		
	return vl_res;
}


/* Procedure to store a new result in the database 
* the result being stored is the current latest one...
*/
tU8 DABMW_ServiceFollowing_SSDatabaseStore(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI)
{
	tU8 vl_cnt;
	tU8 vl_storingIndex = DABMW_INVALID_DATA_BYTE;
	
	/* Look if already present : if so just update */
	
	/* due to the ECC which may not yet be valid when store, 
	* proposal is for now, to compare EID without ECC
	* ie 
	*/
	for (vl_cnt=0;vl_cnt<DABMW_SF_SS_DATABASE_SIZE;vl_cnt++)
		{
		if ((DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Frequency == vI_DAB_Frequency)
			&&
			((DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Eid & 0x0000FFFF) == (vI_DAB_Eid & 0x0000FFFF))
			&&
			(DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Sid == vI_DAB_Sid)
			&&
			(DABMW_SF_SSEstimation_Database[vl_cnt].FM_PI == vI_FM_PI)
			)
			{
			vl_storingIndex = vl_cnt;
			break;
			}
		}

    // codex 304488 : manage the ss error cases
    //
    // if we have an error, and there is already something stored valid, do not write over it
    // 
    
    if ((DABMW_INVALID_DATA_BYTE != vl_storingIndex)
        && (SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS != dabmw_msg_sf_seamless_report.status)
        && (SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS == DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.status)
        )
    {
        // increment the counter and return
        if (DABMW_SF_SSEstimation_Database[vl_storingIndex].error_counter < DABMW_SF_SS_THRESHOLD_ERROR_COUNTER)
        {
            DABMW_SF_SSEstimation_Database[vl_storingIndex].error_counter++;
        }
        
        // in case of mode 1 & we may store the delay
        // but not now.

        // no update on time last estimated so that 
        return vl_storingIndex;
        
    }
       
 	
	/* if not found : and not a reset ... then find a room
	* hypothese is to find the last stored one.. ie take room of the older stored result 
	*/
	if (DABMW_INVALID_DATA_BYTE == vl_storingIndex)
		{
		vl_storingIndex = 0;
		
		for (vl_cnt=0;vl_cnt<DABMW_SF_SS_DATABASE_SIZE;vl_cnt++)
			{
			if (DABMW_SF_SSEstimation_Database[vl_cnt].TimeLastEstimated < DABMW_SF_SSEstimation_Database[vl_storingIndex].TimeLastEstimated)
				{
				vl_storingIndex = vl_cnt;
				}
			}
        
        // this is reused
        // make it clean
        DABMW_SF_SSEstimation_Database[vl_storingIndex].DAB_Frequency = DABMW_INVALID_FREQUENCY;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].DAB_Eid = DABMW_INVALID_EID;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].DAB_Sid = DABMW_INVALID_SERVICE_ID;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].FM_PI = DABMW_INVALID_SERVICE_ID;
         // codex 304488 : manage the ss error cases
        DABMW_SF_SSEstimation_Database[vl_storingIndex].error_counter = 0;
		}
	
	
	/* Ok, now we know where to store 
	*/	
	DABMW_SF_SSEstimation_Database[vl_storingIndex].DAB_Frequency = vI_DAB_Frequency;
	DABMW_SF_SSEstimation_Database[vl_storingIndex].DAB_Eid = vI_DAB_Eid;
	DABMW_SF_SSEstimation_Database[vl_storingIndex].DAB_Sid = vI_DAB_Sid;
	DABMW_SF_SSEstimation_Database[vl_storingIndex].FM_PI = vI_FM_PI;

    // error counter to be managed : 
    // codex 304488 : manage the ss error cases
    // 
    // if we are here, we decide to store in database : 
    // either because nothing better was in or nothing already stored
    //
    // handle the error counter
    //
    if (SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS != dabmw_msg_sf_seamless_report.status)
    {
        if (DABMW_SF_SSEstimation_Database[vl_storingIndex].error_counter < DABMW_SF_SS_THRESHOLD_ERROR_COUNTER)
        {
            DABMW_SF_SSEstimation_Database[vl_storingIndex].error_counter++;
        }
    }
    else
    {
        //
        // we have a good one : reset the consecutive counter error...
        //
        DABMW_SF_SSEstimation_Database[vl_storingIndex].error_counter = 0;
       
    }
    
    
	/* we should not do a memcpy but a 1 to 1 matching
	* because depending on cmd mode, some parameter have not been recalculated..
	* so store one should be kept
	*/
	if (SD_SEAMLESS_ESTIMATION_START_MODE_1 == DABMW_SF_SS_EstimationResult.requestedMode)
		{
		/* full command estimate delay & loudness */
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.msg_id = dabmw_msg_sf_seamless_report.msg_id;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.status = dabmw_msg_sf_seamless_report.status;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.provider_type = dabmw_msg_sf_seamless_report.provider_type;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.absolute_estimated_delay_in_samples = dabmw_msg_sf_seamless_report.absolute_estimated_delay_in_samples;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.delay_estimate = dabmw_msg_sf_seamless_report.delay_estimate;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.timestamp_FAS = dabmw_msg_sf_seamless_report.timestamp_FAS;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.timestamp_SAS = dabmw_msg_sf_seamless_report.timestamp_SAS;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.average_RMS2_FAS = dabmw_msg_sf_seamless_report.average_RMS2_FAS;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.average_RMS2_SAS = dabmw_msg_sf_seamless_report.average_RMS2_SAS;
        DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.confidence_level = dabmw_msg_sf_seamless_report.confidence_level;
		}
	else if (SD_SEAMLESS_ESTIMATION_START_MODE_2 == DABMW_SF_SS_EstimationResult.requestedMode)
		{
		/* MODE 2 command estimate delay only */
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.msg_id = dabmw_msg_sf_seamless_report.msg_id;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.status = dabmw_msg_sf_seamless_report.status;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.provider_type = dabmw_msg_sf_seamless_report.provider_type;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.absolute_estimated_delay_in_samples = dabmw_msg_sf_seamless_report.absolute_estimated_delay_in_samples;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.delay_estimate = dabmw_msg_sf_seamless_report.delay_estimate;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.timestamp_FAS = dabmw_msg_sf_seamless_report.timestamp_FAS;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.timestamp_SAS = dabmw_msg_sf_seamless_report.timestamp_SAS;
        DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.confidence_level = dabmw_msg_sf_seamless_report.confidence_level;
		}
	else if (SD_SEAMLESS_ESTIMATION_START_MODE_3 == DABMW_SF_SS_EstimationResult.requestedMode)
		{
		/* MODE 3  command estimate loudness only */
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.msg_id = dabmw_msg_sf_seamless_report.msg_id;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.status = dabmw_msg_sf_seamless_report.status;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.provider_type = dabmw_msg_sf_seamless_report.provider_type;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.average_RMS2_FAS = dabmw_msg_sf_seamless_report.average_RMS2_FAS;
		DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult.average_RMS2_SAS = dabmw_msg_sf_seamless_report.average_RMS2_SAS;
		}
	else
		{
		/* error case : no mode... */
		/* copy anyhow */
		DABMW_ServiceFollowing_ExtInt_MemoryCopy(&(DABMW_SF_SSEstimation_Database[vl_storingIndex].SS_EstimationResult), &dabmw_msg_sf_seamless_report, sizeof(SF_msg_SeamlessEstimationResponse));
		}
		
	DABMW_SF_SSEstimation_Database[vl_storingIndex].TimeLastEstimated = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

	return vl_storingIndex;
}

/* Procedure to retrieve an existing valid result in the database 
* output :  the pointer on the response
*/
DABMW_SS_EstimationResutDatabaseTy* DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI, SF_tMSecond vI_validityDuration)
{
	tU8 vl_cnt;
	DABMW_SS_EstimationResutDatabaseTy* pl_StoredReport = NULL;
    SF_tMSecond vl_currentTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

	/* Look if already present : if so just update */
	
    /* due to the ECC which may not yet be valid when store, 
	* proposal is for now, to compare EID without ECC
	* ie 
	*/
    // codex #315761
    // EPR Change :always configure Seamless so that it is ready to play the rigth buffers...
    // 
    // if seamless not activated : default switch
    //
	if (false == DABMW_serviceFollowingData.seamlessSwitchingMode)
	    {
        // seamless not activated : just simulate empty database.
        pl_StoredReport  = NULL;
	    }
    else 
        {
    	for (vl_cnt=0;vl_cnt<DABMW_SF_SS_DATABASE_SIZE;vl_cnt++)
    		{
    		// print the database for debug
     		if ((DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Frequency == vI_DAB_Frequency)
    			&&
    			((DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Eid & 0x0000FFFF) == (vI_DAB_Eid & 0x0000FFFF))
    			&&
    			(DABMW_SF_SSEstimation_Database[vl_cnt].DAB_Sid == vI_DAB_Sid)
    			&&
    			(DABMW_SF_SSEstimation_Database[vl_cnt].FM_PI == vI_FM_PI)
    			)
    			{
    			pl_StoredReport = &(DABMW_SF_SSEstimation_Database[vl_cnt]);

                // check the time
            
                if ((DAMBW_SF_MEASURE_VALIDITY_INFINITE != vI_validityDuration)
                && ((vl_currentTime -  pl_StoredReport->TimeLastEstimated) > vI_validityDuration))
                {
                    pl_StoredReport  = NULL;
    		    }
              
    			break;
    			}
    		}
    }



	return pl_StoredReport;
}


/* Procedure to check if an existing valid result in the database 
* output : bool true false 
*/
tBool DABMW_ServiceFollowing_SSIsStoredInfoFromDatabase(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI, SF_tMSecond vI_validityDuration)
{
	DABMW_SS_EstimationResutDatabaseTy* pl_StoredReport = NULL;
	tBool vl_res = false;
 
	/* Look if already present : if so just update */

	pl_StoredReport = DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(vI_DAB_Frequency,
																	vI_DAB_Eid,
																	vI_DAB_Sid,
																	vI_FM_PI,
																    vI_validityDuration);

	/* if found & result == SUCCESS 
	*/
	if ((NULL != pl_StoredReport)
		&&
		(SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS == pl_StoredReport->SS_EstimationResult.status))
		{
  
		vl_res 	= true;
	    }
	
	return vl_res;
}

// procedure to check if an action around seamless may be needed.
// The conditions : 
//
// we should be in DAB 1st since this is the only configuration supported.
// we should have 1 freq in FM, 1 freq in DAB
// We should either have nothing in database (full estimation) , either a reconf needed (elapse validity time...)
//
// The alternate should be active for more than a certain time... especially for reconf, 
// this is a constraint from seamless switching algo so that buffers are filled...
//

tBool DABMW_ServiceFollowing_CheckIfSeamlessActionIsNeeded()
{
    tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
    tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
    tBool vl_seamlessActionNeeded = false;
    tU32  vl_dabFrequency;
    tU32  vl_dabEid;
    tU32  vl_dabSid;
    tU32  vl_fmPi;
    DABMW_SS_EstimationResutDatabaseTy *pl_StoredReport;
    SF_tMSecond vl_currentTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
	SF_tMSecond vl_referenceTimeToCompare;



    
    if ((true == DABMW_serviceFollowingData.seamlessSwitchingMode) 
        // even in FM first, we may do estimation , which will provide loudness info and gain time for further switch
        //		&& (true == DABMW_serviceFollowingStatus.configurationIsDabFirst)
        && (vl_originalIsDab != vl_alternateIsDab))    
    {
        // we are in dab first.
        // we have one FM & one DAB

        if (true == vl_originalIsDab)
        {
            vl_dabFrequency = DABMW_serviceFollowingStatus.originalFrequency;
            vl_dabEid = DABMW_serviceFollowingStatus.originalEid;
            vl_dabSid = DABMW_serviceFollowingStatus.originalSid;
            vl_fmPi = DABMW_serviceFollowingStatus.alternateSid;
        }
        else
        {
            vl_dabFrequency = DABMW_serviceFollowingStatus.alternateFrequency;
            vl_dabEid = DABMW_serviceFollowingStatus.alternateEid;
            vl_dabSid = DABMW_serviceFollowingStatus.alternateSid;
            vl_fmPi = DABMW_serviceFollowingStatus.originalSid;
        }
        
        // if alternate is DAB : check the service is selected
        // 
        if (((true == vl_alternateIsDab) && (true == DABMW_serviceFollowingStatus.alternate_DabServiceSelected))
            ||
            (false == vl_alternateIsDab))
        {
            // we have condition for estimations...
            //
            // check the database
            //

            // 1st check if something in database
            pl_StoredReport = DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(vl_dabFrequency, 
                            										   vl_dabEid,
                            										   vl_dabSid,
                            										   vl_fmPi,
                            										   DABMW_SS_EstimationParameter.measureValidityDuration);

			// if alternate is FM : the reference time is last time alternate has been activated
			// Whereas if alternate is DAB, the reference time is last time the audio is activate : ie time since no_audio

			if (true == vl_alternateIsDab)
				{
				
				// mark we have no audio 
				vl_referenceTimeToCompare = DABMW_serviceFollowingStatus.alternateLastTimeNoAudio;
				}
			else
				{
				vl_referenceTimeToCompare = DABMW_serviceFollowingStatus.deltaTimeFromAlternateSwitch;
				}
			

            if ((NULL == pl_StoredReport)
                ||
                (SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS != pl_StoredReport->SS_EstimationResult.status)
                ||
                // codex 304488 : manage the SS error cases.
                // check the error counter threshold : 
                // if we are above, it means that we get a number of consecutive estimation failure... 
                // it may be good to reestimate : we do not trust what we have... 
                (pl_StoredReport->error_counter >= DABMW_SF_SS_THRESHOLD_ERROR_COUNTER)
                )
                {
                    // nothing valid in database : a full estimate needed
                    // the check is this  is time do it : avoid spending time requesting again and again seamless will be done later
                    // make sure this is tune for enough time already....
                    //
                    // we can launch the seamless directly..
 //               if ((vl_currentTime - vl_referenceTimeToCompare) > DABMW_SF_SS_DELAY_FOR_ESTIMATION_AFTER_ALTERNATE_TUNED)
                    {
                        vl_seamlessActionNeeded = true;
                    }
                }
              // do we trust the confidence level ? 
            else  if ((pl_StoredReport->SS_EstimationResult.confidence_level < DABMW_SS_EstimationParameter.ConfidenceLevelThreshold)
                && ((vl_currentTime - vl_referenceTimeToCompare) > DABMW_SF_SS_DELAY_FOR_ESTIMATION_AFTER_ALTERNATE_TUNED)
                )
                {
                // do we trust the confidence level ? 
                // here no : below the threshold of trust
                // the check is this  is time do it : avoid spending time requesting again and again seamless will be done later

                vl_seamlessActionNeeded = true;
                }
            else
            {
                // something valid in database...
                // but is that still valid for reconf ? 
                if ((vl_currentTime -  pl_StoredReport->TimeLastEstimated) > DABMW_SS_EstimationParameter.reconfPeriodicity)
                {
                    // it needs a reconf...
                    //
                    // Last check : the alternate should be present for more that the estimated delay
                     // the check is this  is time do it : avoid spending time requesting again and again seamless will be done later
                    if ((vl_currentTime - vl_referenceTimeToCompare) > (tU32)(DABMW_ABS(DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(pl_StoredReport->SS_EstimationResult.absolute_estimated_delay_in_samples))+DABMW_SF_SS_MARGIN_BEFORE_RECONF_LAUNCH_MS))
                    {
                        // all is fine
                        vl_seamlessActionNeeded = true;
                    }
                }

            }
        }
       

        // check is this  is time do it : avoid spending time requesting again and again seamless will be done later
        // in any case : if the last estimation has been done recently : do nothing
        // 
		// INVALID DATA for time means not yet done.
		 if ((DABMW_INVALID_DATA != DABMW_SF_SS_EstimationResult.timeLastEstimationDone)
			 && ((vl_currentTime - DABMW_SF_SS_EstimationResult.timeLastEstimationDone) < DABMW_SS_EstimationParameter.minimumTimeBetween2Seamless)
			 )
        {
            vl_seamlessActionNeeded = false;
        }
        
    }

    
    return vl_seamlessActionNeeded;
}


// procedure to check if seamless estimation or switch can be done
// i.e. if the audio is on for long enough to get a switch, or to get an estimation prior to switch
// The conditions : 
//
// we should be in DAB 1st since this is the only configuration supported.
// we should have 1 freq in FM, 1 freq in DAB
//
// The alternate should be active for more than a certain time... especially for reconf, 
// this is a constraint from seamless switching algo so that buffers are filled...
//

tBool DABMW_ServiceFollowing_CheckIfReadyToSeamlessSwitch()
{
    tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
    tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
    tBool vl_readyForSeamlessSwitch = false;
    tU32  vl_dabFrequency;
    tU32  vl_dabEid;
    tU32  vl_dabSid;
    tU32  vl_fmPi;
    DABMW_SS_EstimationResutDatabaseTy *pl_StoredReport;
    SF_tMSecond vl_currentTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
	SF_tMSecond vl_referenceTimeToCompare;

    
    if ((true == DABMW_serviceFollowingData.seamlessSwitchingMode) 
        // even in FM first, we may do estimation , which will provide loudness info and gain time for further switch
        //		&& (true == DABMW_serviceFollowingStatus.configurationIsDabFirst)
        && (vl_originalIsDab != vl_alternateIsDab))    
    {
        // we are in dab first.
        // we have one FM & one DAB

        if (true == vl_originalIsDab)
        {
            vl_dabFrequency = DABMW_serviceFollowingStatus.originalFrequency;
            vl_dabEid = DABMW_serviceFollowingStatus.originalEid;
            vl_dabSid = DABMW_serviceFollowingStatus.originalSid;
            vl_fmPi = DABMW_serviceFollowingStatus.alternateSid;
        }
        else
        {
            vl_dabFrequency = DABMW_serviceFollowingStatus.alternateFrequency;
            vl_dabEid = DABMW_serviceFollowingStatus.alternateEid;
            vl_dabSid = DABMW_serviceFollowingStatus.alternateSid;
            vl_fmPi = DABMW_serviceFollowingStatus.originalSid;
        }
        
        // if alternate is DAB : check the service is selected
        // 
        if (((true == vl_alternateIsDab) && (true == DABMW_serviceFollowingStatus.alternate_DabServiceSelected))
            ||
            (false == vl_alternateIsDab))
        {
            // we have condition for estimations...
            //
            // check the database
            //

            // 1st check if something in database
            pl_StoredReport = DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(vl_dabFrequency, 
                            										   vl_dabEid,
                            										   vl_dabSid,
                            										   vl_fmPi,
                            										   DABMW_SS_EstimationParameter.measureValidityDuration);

			// if alternate is FM : the reference time is last time alternate has been activated
			// Whereas if alternate is DAB, the reference time is last time the audio is activate : ie time since no_audio

			if (true == vl_alternateIsDab)
				{
				
				// mark we have no audio 
				vl_referenceTimeToCompare = DABMW_serviceFollowingStatus.alternateLastTimeNoAudio;
				}
			else
				{
				vl_referenceTimeToCompare = DABMW_serviceFollowingStatus.deltaTimeFromAlternateSwitch;
				}
					

            if ((NULL == pl_StoredReport)
                ||
                (SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS != pl_StoredReport->SS_EstimationResult.status)
                ||
                // codex 304488 : manage the SS error cases.
                // check the error counter threshold : 
                // if we are above, it means that we get a number of consecutive estimation failure... 
                // it may be good to reestimate : we do not trust what we have... 
                (pl_StoredReport->error_counter >= DABMW_SF_SS_THRESHOLD_ERROR_COUNTER)
                )
                {
                    // nothing valid in database : a full estimate needed
                    // the check is this  is time do it : avoid spending time requesting again and again seamless will be done later
                    // make sure this is tune for enough time already....
                    //
                    // we can run seamless immediately
 //               if ((vl_currentTime - vl_referenceTimeToCompare) > DABMW_SF_SS_DELAY_FOR_ESTIMATION_AFTER_ALTERNATE_TUNED)
                    {
                        vl_readyForSeamlessSwitch = true;
                    }
                }
              // do we trust the confidence level ? 
              // for switching we always trust
              //
	           	/* else  if ((pl_StoredReport->SS_EstimationResult.confidence_level < DABMW_SS_EstimationParameter.ConfidenceLevelThreshold)
	                && ((vl_currentTime - DABMW_serviceFollowingStatus.deltaTimeFromAlternateSwitch) > DABMW_SF_SS_DELAY_FOR_ESTIMATION_AFTER_ALTERNATE_TUNED)
	                )
	                {
	                // do we trust the confidence level ? 
	                // here no : below the threshold of trust
	                // the check is this  is time do it : avoid spending time requesting again and again seamless will be done later

	                vl_seamlessActionNeeded = true;
	                }
	                */
            else
            {
                // something valid in database...
               	// Last check : the alternate should be present for more that the estimated delay
               	// the check is this  is time do it : avoid spending time requesting again and again seamless will be done later
               if ((vl_currentTime - vl_referenceTimeToCompare) > (tU32)(DABMW_ABS(DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(pl_StoredReport->SS_EstimationResult.absolute_estimated_delay_in_samples))+DABMW_SF_SS_MARGIN_BEFORE_RECONF_LAUNCH_MS))
                    {
                    // all is fine
                    vl_readyForSeamlessSwitch = true;
               	    }

            }
        }
               
    }

    
    return vl_readyForSeamlessSwitch;
}


// Procedure to do a seamless switch
// retrieve the information from the database concerning the parameters...
//
tS32 DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(SD_SeamlessSwitchingSystemToSwitchTy vI_systemToSwitch)
{
    tU32  vl_dabFrequency;
    tU32  vl_dabEid;
    tU32  vl_dabSid;
    tU32  vl_fmPi;
    tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
//    tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
    DABMW_SS_EstimationResutDatabaseTy* pl_ExistingSeamlessReport = NULL;
    SF_msg_SeamlessSwitchingRequest dabmw_msg_sf_seamless_switch;    
	tS32 vl_res;

    if (true == vl_originalIsDab)
        {
        vl_dabFrequency = DABMW_serviceFollowingStatus.originalFrequency;
        vl_dabEid = DABMW_serviceFollowingStatus.originalEid;
        vl_dabSid = DABMW_serviceFollowingStatus.originalSid;
        vl_fmPi = DABMW_serviceFollowingStatus.alternateSid;
        }
    else
        {
        vl_dabFrequency = DABMW_serviceFollowingStatus.alternateFrequency;
        vl_dabEid = DABMW_serviceFollowingStatus.alternateEid;
        vl_dabSid = DABMW_serviceFollowingStatus.alternateSid;
        vl_fmPi = DABMW_serviceFollowingStatus.originalSid;
        }

    // Retrieve the existing result in database 
    // check a valid one exists (with valid date)
    // if so, get it
    pl_ExistingSeamlessReport = DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(vl_dabFrequency, 
                                            vl_dabEid,
                                            vl_dabSid,
                                            vl_fmPi,
                                            DABMW_SS_EstimationParameter.measureValidityDuration);
    
    if ((NULL != pl_ExistingSeamlessReport)
        && (SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS == pl_ExistingSeamlessReport->SS_EstimationResult.status)      
        // even in FM first, we may do estimation , which will provide loudness info and gain time for further switch
        //      && (true == DABMW_serviceFollowingStatus.configurationIsDabFirst) 
        )
        
    {
		//dabmw_msg_sf_seamless_switch.msg_id = SD_MESSAGE_RX_SEAMLESS_SWITCHING;
		dabmw_msg_sf_seamless_switch.systemToSwitch = vI_systemToSwitch;
		dabmw_msg_sf_seamless_switch.provider_type = pl_ExistingSeamlessReport->SS_EstimationResult.provider_type;
		dabmw_msg_sf_seamless_switch.absolute_estimated_delay_in_samples = pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples;
		dabmw_msg_sf_seamless_switch.delay_estimate = pl_ExistingSeamlessReport->SS_EstimationResult.delay_estimate;
		dabmw_msg_sf_seamless_switch.timestamp_FAS = pl_ExistingSeamlessReport->SS_EstimationResult.timestamp_FAS;
		dabmw_msg_sf_seamless_switch.timestamp_SAS = pl_ExistingSeamlessReport->SS_EstimationResult.timestamp_SAS;
		dabmw_msg_sf_seamless_switch.average_RMS2_FAS = pl_ExistingSeamlessReport->SS_EstimationResult.average_RMS2_FAS;
		dabmw_msg_sf_seamless_switch.average_RMS2_SAS = pl_ExistingSeamlessReport->SS_EstimationResult.average_RMS2_SAS;


    	dabmw_msg_sf_seamless_switch.confirmationRequested = SD_SEAMLESS_ESTIMATION_CONFIRMATION_NOT_REQUESTED;
	}
	else
	{
        // codex #319037
        // if the switch is to FM, and there is no info in database
        // then do a switch to "early FM"
        // the audio port switch will be done on response
        // even if it could be done rigth now
        //
        if (SD_SEAMLESS_SWITCHING_SWITCH_TO_FM == vI_systemToSwitch)
        {
            vI_systemToSwitch = SD_SEAMLESS_SWITCHING_SWITCH_TO_EARLY_FM;
        }
        // end codex #319037
        
		//dabmw_msg_sf_seamless_switch.msg_id = SD_MESSAGE_RX_SEAMLESS_SWITCHING;
		dabmw_msg_sf_seamless_switch.systemToSwitch = vI_systemToSwitch;
		dabmw_msg_sf_seamless_switch.provider_type = SD_SEAMLESS_PROVIDER_IS_DAB;
		dabmw_msg_sf_seamless_switch.absolute_estimated_delay_in_samples = SD_SEAMLESS_ESTIMATION_DEFAULT_DELAY;
		dabmw_msg_sf_seamless_switch.delay_estimate = SD_SEAMLESS_ESTIMATION_DEFAULT_DELAY;
		dabmw_msg_sf_seamless_switch.timestamp_FAS = SD_SEAMLESS_ESTIMATION_DEFAULT_TS;
		dabmw_msg_sf_seamless_switch.timestamp_SAS = SD_SEAMLESS_ESTIMATION_DEFAULT_TS;
		dabmw_msg_sf_seamless_switch.average_RMS2_FAS = SD_SEAMLESS_ESTIMATION_DEFAULT_RMS2;
		dabmw_msg_sf_seamless_switch.average_RMS2_SAS = SD_SEAMLESS_ESTIMATION_DEFAULT_RMS2;
		dabmw_msg_sf_seamless_switch.confirmationRequested = SD_SEAMLESS_ESTIMATION_CONFIRMATION_NOT_REQUESTED;
	}

//#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)
    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                             "DABMW_Service_Following (ServiceFollowingSeamlessSwitchWithDatabaseInfo): request seamless switching ** system to switch : %s, provider_type %s,  delay in ms %d, absolute_estimated_delay_in_samples %d, delay_estimate %d, timestamp_FAS %d, timestamp_SAS %d, average_RMS2_FAS %d (%2.3f dB), average_RMS2_SAS %d (%2.3f dB), confirmation NOT requested \n",
                               DABMW_SF_SEAMLESS_SYSTEM_TO_SWITCH(dabmw_msg_sf_seamless_switch.systemToSwitch),
                               DABMW_SF_SEAMLESS_LOG_PROVIDER_TYPE(dabmw_msg_sf_seamless_switch.provider_type),
                               DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(dabmw_msg_sf_seamless_switch.absolute_estimated_delay_in_samples),
                               dabmw_msg_sf_seamless_switch.absolute_estimated_delay_in_samples,
                               dabmw_msg_sf_seamless_switch.delay_estimate,
                               dabmw_msg_sf_seamless_switch.timestamp_FAS,
                               dabmw_msg_sf_seamless_switch.timestamp_SAS,
                               dabmw_msg_sf_seamless_switch.average_RMS2_FAS,
                               DABMW_SEAMLESS_ESTIMATION_CONVERT_LOUDNESS_IN_DB(dabmw_msg_sf_seamless_report.average_RMS2_FAS),
                               dabmw_msg_sf_seamless_switch.average_RMS2_SAS, 
                               DABMW_SEAMLESS_ESTIMATION_CONVERT_LOUDNESS_IN_DB(dabmw_msg_sf_seamless_report.average_RMS2_SAS));

  
    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
//#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   						

	// register to call back for response
	if (false == vl_SS_callback_registered)
		{		
		DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessSwitchingResponse((DABMW_SF_audio_msgsCallbackTy)DABMW_ServiceFollowing_SeamlessSwitchingResponse);
		vl_SS_callback_registered = true;
		}
    
	vl_res = DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessSwitchingRequest(dabmw_msg_sf_seamless_switch);
	
	DABMW_SF_SS_LastSwitchRequestInfo.switchRequestValidity = true;
	DABMW_SF_SS_LastSwitchRequestInfo.TimeLastSwitchRequested = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
	DABMW_SF_SS_LastSwitchRequestInfo.SS_SwitchRequest = dabmw_msg_sf_seamless_switch;

    // set flag to monitor a seamless msg is on-going
    DABMW_serviceFollowingStatus.seamless_action_on_going = true;
    
	return vl_res;
}


// Procedure to do a seamless switch
// retrieve the information from the database concerning the parameters...
//
tS32 DABMW_ServiceFollowingSeamlessSwitchDefault(SD_SeamlessSwitchingSystemToSwitchTy vI_systemToSwitch)
{

    SF_msg_SeamlessSwitchingRequest dabmw_msg_sf_seamless_switch;    
	tS32 vl_res;
    

                // codex #319037
        // if the switch is to FM, and there is no info in database
        // then do a switch to "early FM"
        // the audio port switch will be done on response
        // even if it could be done rigth now
        //
        if (SD_SEAMLESS_SWITCHING_SWITCH_TO_FM == vI_systemToSwitch)
        {
            vI_systemToSwitch = SD_SEAMLESS_SWITCHING_SWITCH_TO_EARLY_FM;
        }
        // end codex #319037
        
		//dabmw_msg_sf_seamless_switch.msg_id = SD_MESSAGE_RX_SEAMLESS_SWITCHING;
		dabmw_msg_sf_seamless_switch.systemToSwitch = vI_systemToSwitch;
		dabmw_msg_sf_seamless_switch.provider_type = SD_SEAMLESS_PROVIDER_IS_DAB;
		dabmw_msg_sf_seamless_switch.absolute_estimated_delay_in_samples = SD_SEAMLESS_ESTIMATION_DEFAULT_DELAY;
		dabmw_msg_sf_seamless_switch.delay_estimate = SD_SEAMLESS_ESTIMATION_DEFAULT_DELAY;
		dabmw_msg_sf_seamless_switch.timestamp_FAS = SD_SEAMLESS_ESTIMATION_DEFAULT_TS;
		dabmw_msg_sf_seamless_switch.timestamp_SAS = SD_SEAMLESS_ESTIMATION_DEFAULT_TS;
		dabmw_msg_sf_seamless_switch.average_RMS2_FAS = SD_SEAMLESS_ESTIMATION_DEFAULT_RMS2;
		dabmw_msg_sf_seamless_switch.average_RMS2_SAS = SD_SEAMLESS_ESTIMATION_DEFAULT_RMS2;
		dabmw_msg_sf_seamless_switch.confirmationRequested = SD_SEAMLESS_ESTIMATION_CONFIRMATION_NOT_REQUESTED;

//#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)
    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                             "DABMW_Service_Following (DABMW_ServiceFollowingSeamlessSwitchDefault): request seamless switching ** system to switch : %s, provider_type %s,  delay in ms %d, absolute_estimated_delay_in_samples %d, delay_estimate %d, timestamp_FAS %d, timestamp_SAS %d, average_RMS2_FAS %d (%2.3f dB), average_RMS2_SAS %d (%2.3f dB), confirmation NOT requested \n",
                               DABMW_SF_SEAMLESS_SYSTEM_TO_SWITCH(dabmw_msg_sf_seamless_switch.systemToSwitch),
                               DABMW_SF_SEAMLESS_LOG_PROVIDER_TYPE(dabmw_msg_sf_seamless_switch.provider_type),
                               DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(dabmw_msg_sf_seamless_switch.absolute_estimated_delay_in_samples),
                               dabmw_msg_sf_seamless_switch.absolute_estimated_delay_in_samples,
                               dabmw_msg_sf_seamless_switch.delay_estimate,
                               dabmw_msg_sf_seamless_switch.timestamp_FAS,
                               dabmw_msg_sf_seamless_switch.timestamp_SAS,
                               dabmw_msg_sf_seamless_switch.average_RMS2_FAS,
                               DABMW_SEAMLESS_ESTIMATION_CONVERT_LOUDNESS_IN_DB(dabmw_msg_sf_seamless_report.average_RMS2_FAS),
                               dabmw_msg_sf_seamless_switch.average_RMS2_SAS, 
                               DABMW_SEAMLESS_ESTIMATION_CONVERT_LOUDNESS_IN_DB(dabmw_msg_sf_seamless_report.average_RMS2_SAS));

  
    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
//#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   						

	// register to call back for response
	if (false == vl_SS_callback_registered)
		{		
		DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessSwitchingResponse((DABMW_SF_audio_msgsCallbackTy)DABMW_ServiceFollowing_SeamlessSwitchingResponse);
		vl_SS_callback_registered = true;
		}
    
	vl_res = DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessSwitchingRequest(dabmw_msg_sf_seamless_switch);
	
	DABMW_SF_SS_LastSwitchRequestInfo.switchRequestValidity = true;
	DABMW_SF_SS_LastSwitchRequestInfo.TimeLastSwitchRequested = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
	DABMW_SF_SS_LastSwitchRequestInfo.SS_SwitchRequest = dabmw_msg_sf_seamless_switch;

    // set flag to monitor a seamless msg is on-going
    DABMW_serviceFollowingStatus.seamless_action_on_going = true;
    
	return vl_res;
}


/* Registered function for  SeamLess Estimation Response */
tVoid DABMW_ServiceFollowing_SeamlessSwitchingResponse (tPVoid msgPayload)
{
	/* Processing here is to store the result locally 
	* and notify Event Seamless Estimate Received 
	*/
    SF_msg_SeamlessSwitchingResponse *dabmw_msg_sf_switching_report = (SF_msg_SeamlessSwitchingResponse *)msgPayload;

	// do not check here the state, it will be done on event processing
	//
//	if (DABMW_SF_STATE_IDLE_SEAMLESS_SWITCHING == DABMW_serviceFollowingStatus.status)
		{
 
         /*  LOG */
//#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
          DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
          DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                                         "DABMW_Service_Following (DABMW_ServiceFollowing_SeamlessSwitchingResponse) : switching done : status %d, delay %d (%d ms)\n",
                                         dabmw_msg_sf_switching_report->status,
                                         dabmw_msg_sf_switching_report->delay,
                                         DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(dabmw_msg_sf_switching_report->delay));
                 DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
//#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
        /* END TMP LOG */

        // for warnings
        (tVoid)dabmw_msg_sf_switching_report; // Make Lint happy
        
		}
	/*
		else
		{
		// this is an error : not correct state.. 
		}
		*/


	DABMW_SF_SS_LastSwitchRequestInfo.SS_SwitchResponse= *dabmw_msg_sf_switching_report;

	/* Notify Event */
	DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_SS_SWITCHING_RSP);

    //set flag SS action on going.
    DABMW_serviceFollowingStatus.seamless_action_on_going = false;


}


/* Procedure which cares of configuring correctly the SS : source to be used and switch if needed
* 
*/
tVoid DABMW_ServiceFollowing_ConfigureSeamlessForOriginal()
{

	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
    
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
	SD_SeamlessSwitchingSystemToSwitchTy vl_systemToSwitch;

	/* action depends on configuration */
             /*  LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
              DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
              DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                                             "DABMW_Service_Following (DABMW_ServiceFollowing_ConfigureSeamlessForOriginal) : configure Seamless port source\n"
                                           );
              DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
            /* END TMP LOG */

	if (true == vl_originalIsDab)
		{
		vl_systemToSwitch = SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB;
		}
	else 
		{
        if (true == DABMW_serviceFollowingStatus.configurationIsDabFirst)
            {
		    vl_systemToSwitch = SD_SEAMLESS_SWITCHING_SWITCH_TO_FM;
            }
        else
            {
            vl_systemToSwitch = SD_SEAMLESS_SWITCHING_SWITCH_TO_EARLY_FM;
            }
		}

	if (true == DABMW_serviceFollowingData.seamlessSwitchingMode)
	    {

		// 
		// if the switch request is to same do not send Switch
		//
		if (vl_systemToSwitch != DABMW_ServiceFollowing_RetrieveLastSwitchSystemToSwitch())
			{
		
			/* seamless switching request */
			DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(vl_systemToSwitch);	
			}
		}
    else
#endif 	// CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
        {
        // inform seamless of audio port source change
        // 
        
        // codex #315761
        // EPR Change :always configure Seamless so that it is ready to play the rigth buffers...
        // 

        if ((SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB == vl_systemToSwitch)
            && (SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB != DABMW_ServiceFollowing_RetrieveLastSwitchSystemToSwitch())
            )
            {
            /* seamless switching request */
		    DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(vl_systemToSwitch);	
            }
        else 
            {
            // just set audio port 
            //
            // DABMW_SF_SetCurrentAudioPortUser(DABMW_serviceFollowingStatus.originalAudioUserApp, true);
            DABMW_ServiceFollowing_ExtInt_SetCurrentAudioPortUser(DABMW_serviceFollowingStatus.originalAudioUserApp, true);
            }
        }

}

// Retrieve last Switch status info
//
DABMW_SF_SS_LastSwitchRequestInfoTy DABMW_ServiceFollowing_RetrieveLastSwitchInfo()
{

	return (DABMW_SF_SS_LastSwitchRequestInfo);
	
}

// Retrieve last Switch status System To Swich
//
SD_SeamlessSwitchingSystemToSwitchTy DABMW_ServiceFollowing_RetrieveLastSwitchSystemToSwitch()
{

	return (DABMW_SF_SS_LastSwitchRequestInfo.SS_SwitchRequest.systemToSwitch);
	
}


// clear seamless info : on service change
//
tVoid DABMW_ServiceFollowing_CleanSeamlessInfo()
{

	DABMW_SF_SS_LastSwitchRequestInfo.switchRequestValidity = false;	
}


// TMP ADD ON DEMO
// 
tSInt DABMW_ServiceFollowing_SeamlessEstimationStartDemo()
{
	tSInt vl_res = OSAL_OK;
	static tBool vl_registered = false;

	DABMW_SS_EstimationResutDatabaseTy* pl_ExistingSeamlessReport = NULL;
    SF_msg_SeamlessEstimationRequest   vl_seamlessEstimationStart;

#if defined (CONFIG_ENABLE_CLASS_APP_DABMW_SF)
	// codex #329753 : add a local variable to print absolute_delay_in_sample
	tS32 vl_absolute_estimated_delay_in_samples = 0;
#endif

	/* fill the command in buffer way
	* B0 = MSG TYPE
	
	BYTE 1
		Bits 0 - 1
		SEAMLESS_CMD
	
		Bits 2 - 7
		RFU The SEAMLESS_CMD can be :
		0 - STOP command
		1 - START command
		2 - START delay only command
		3 - START loudness only command
	
	BYTE 2 to 5 = Start position in samples

	BYTE 6 to 10 = Stop position in samples

	BYTE 11 = Down-sampling factor

	BYTE 12 = Minimum loudness duration in seconds
	
	*/


	/* init the buffer */
	DABMW_ServiceFollowing_ExtInt_MemorySet(&vl_seamlessEstimationStart, 0, sizeof(SF_msg_SeamlessEstimationRequest));

	/* Now choose the command to do */
	


		
	if (NULL == pl_ExistingSeamlessReport)
		{
		/* no or invalid prestore : 
		* full start command
		*/
  
		
		DABMW_SF_SS_EstimationResult.requestedMode = SD_SEAMLESS_ESTIMATION_START_MODE_1;
		
		/* Msg TYPE */
        vl_seamlessEstimationStart.msg_id = SD_MESSAGE_RX_SEAMLESS_ESTIMATION;
        //		al_buffer[0] = SD_MESSAGE_RX_SEAMLESS_ESTIMATION;

		/* BYTE 0	SEAMLESS_CMD */
        //		al_buffer[1] = DABMW_SEAMLESS_ESTIMATION_START_CMD_MODE_1;
        vl_seamlessEstimationStart.mode = SD_SEAMLESS_ESTIMATION_START_MODE_1;
        
		
		/* BYTE 1 to 4 = Start position in samples */
        /*
		al_buffer[2] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 24) & 0xFF);
		al_buffer[3] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 16) & 0xFF);
		al_buffer[4] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 8) & 0xFF);
		al_buffer[5] = (tU8) ((START_POSITION_FOR_SEAMLESS_ESTIMATION >> 0) & 0xFF);
		*/
		vl_seamlessEstimationStart.start_position_in_samples = DABMW_SS_EstimationParameter.FullEstimationStartWindowInSample;

		/* BYTE 5 to 9 = Stop position in samples */
		/*al_buffer[6] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 24) & 0xFF);
		al_buffer[7] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 16) & 0xFF);
		al_buffer[8] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 8) & 0xFF);
		al_buffer[9] = (tU8) ((STOP_POSITION_FOR_SEAMLESS_ESTIMATION >> 0) & 0xFF);
            */
            
        vl_seamlessEstimationStart.stop_position_in_samples = DABMW_SS_EstimationParameter.FullEstimationStopWindowInSample;

		/* BYTE 10 = Downsampling factor */
		// al_buffer[10] = DABMW_SS_EstimationParameter.DownSampling;
		vl_seamlessEstimationStart.downSampling = DABMW_SS_EstimationParameter.DownSampling;
		
		/* BYTE 11 = Minimum loudness duration in seconds */
		// al_buffer[11] = MINIMUM_LOUDNESS_DURATION;
        vl_seamlessEstimationStart.loudness_duration = DABMW_SS_EstimationParameter.LoudnessEstimationDurationInSec;

        
		}


	// register to call back for response
	if (false == vl_registered)
		{		
		DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessEstimationResponse((DABMW_SF_audio_msgsCallbackTy)DABMW_ServiceFollowing_SeamlessEstimationResponse);
		vl_registered = true;
		}


          /*  LOG */
           DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );

           DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,
                                   "DABMW_Service_Following (SeamlessEstimationStart) : start %d (%d ms), stop %d (%d ms), DownSampling %d, Loudness duration %d s, delay %d (%d ms)\n",
                                    vl_seamlessEstimationStart.start_position_in_samples , DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(vl_seamlessEstimationStart.start_position_in_samples ),
                                    vl_seamlessEstimationStart.stop_position_in_samples , DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(vl_seamlessEstimationStart.stop_position_in_samples ),
                                    vl_seamlessEstimationStart.downSampling,
                                    vl_seamlessEstimationStart.loudness_duration,
                                    // codex #329753 : 
                                    // correction : pointer pl_ExistingSeamlessReport may be null if database if empty
                                    // before pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples, DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(pl_ExistingSeamlessReport->SS_EstimationResult.absolute_estimated_delay_in_samples));
                                    // after
                                    vl_absolute_estimated_delay_in_samples, DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(vl_absolute_estimated_delay_in_samples));
                                                         
           DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );

            /* END TMP LOG */

    
	// Send the message to the StreamDecoder of length 6
	vl_res = DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest(vl_seamlessEstimationStart);

    // set flag SS action on going.
    DABMW_serviceFollowingStatus.seamless_action_on_going = true;

	/* set the response not received yet */
	DABMW_SF_SS_EstimationResult.IsResponseReceived = false;
	/* set the estimation is on going */
	DABMW_SF_SS_EstimationResult.IsEstimationOnGoing = true;
    DABMW_SF_SS_EstimationResult.timeLastEstimationDone = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
    
	/* init the response */
	dabmw_msg_sf_seamless_report.status = SD_SEAMLESS_ESTIMATION_STATUS_NONE;

    
     
	return vl_res;
}


#endif //#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // SERVICE_FOLLOWING_SEAMLESS_C

