//!
//!  \file      service_following.c
//!  \brief     <i><b> Service following implementation </b></i>
//!  \details   This file provides functionalities for service following
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      2012.07.20
//!  \bug       Unknown
//!  \warning   None
//!

/* EPR CHANGE :  add define for Service Following C
*/
#ifndef SERVICE_FOLLOWING_C
#define SERVICE_FOLLOWING_C
/* END EPR CHANGE */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"


/*
#include "dabmw.h"

#include "fic_common.h"

#include "fic_types.h"

#include "fic_broadcasterdb.h"

#include "system_status.h"

#include "audio_mngr.h"

#include "dabmw.h"
*/

#if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
    #include "rds_landscape.h"
    
    #include "rds_data.h"

    #include "rds_landscape.h"
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)

/*
#include "tuner_control.h"

#include "radio_control.h"

#include "system_app.h"

#if defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)
    #include "AMFMDevCmds.h"
    
    #include "amfmdev_comm.h"
#endif // #if defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)

#include "cis_decoding.h"

#include "fic_landscape.h"

#include "signal_quality.h"

#include "af_check.h"

#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "seamless_switching_common.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)

*/

#include "service_following.h"

#include "service_following_internal.h"

#include "service_following_log_status_info.h"

#include "service_following_mainloop.h"

#include "service_following_background.h"

#include "service_following_meas_and_evaluate.h"

#include "service_following_audioswitch.h"




#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "service_following_seamless.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)


tVoid DABWM_ServiceFollowing_Suspend(tVoid);
tVoid DABWM_ServiceFollowing_Resume(tVoid);
tVoid DABMW_ServiceFollowing_StopOnGoingActivities(tVoid);

#ifdef __cplusplus
extern "C" {
#endif

/// 
// Service following algorithm description
// -----------------------------------------------------------------------------
//                          FM SRC                  FM DST
//
//  MEASURE                           Always check
//  ALTERNATE QUALITY                 alternate
//
//  TUNE                    Below good thr          Above good thr
//
//  AUDIO SWITCH                      Immediate            
// -----------------------------------------------------------------------------
//                          FM SRC                  DAB DST
//
//  MEASURE                           Always check
//  ALTERNATE QUALITY                 alternate               
//
//  TUNE                     
//
//  AUDIO SWITCH                            
// -----------------------------------------------------------------------------
//                          DAB SRC                 DAB DST
//
//  MEASURE                          
//  ALTERNATE QUALITY               
//
//  TUNE                    
//
//  AUDIO SWITCH                            
// -----------------------------------------------------------------------------
//                          DAB SRC                 DAB DST
//
//  MEASURE                      
//  ALTERNATE QUALITY          
//
//  TUNE                   
//
//  AUDIO SWITCH                          
// -----------------------------------------------------------------------------
///
tSInt DABMW_ServiceFollowing (tVoid)
{
    tSInt res = OSAL_OK;  

	/* EPR : add selection loop handling */
	if ((DABMW_SF_STATE_INITIAL_SERVICE_SELECTION == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_BACKGROUND_WAIT_PI == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_BACKGROUND_WAIT_DAB == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_BACKGROUND_SCAN == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_BACKGROUND_WAIT_FIC == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_BACKGROUND_WAIT_PS == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_AF_CHECK == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK == DABMW_serviceFollowingStatus.status)
		|| 
		(DABMW_SF_STATE_SERVICE_RECOVERY == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF == DABMW_serviceFollowingStatus.status)
		|| 
		(DABMW_SF_STATE_IDLE_AF_MONITORING == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_IDLE == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_INIT == DABMW_serviceFollowingStatus.status)
		||
		(DABMW_SF_STATE_IDLE_SEAMLESS == DABMW_serviceFollowingStatus.status)
		)
		{
		res = DABMW_ServiceFollowing_MainLoop();
		}
	/* END EPR CHANGE */
    else
    { 
    ;
    }
    return res;
}

tSInt DABMW_ServiceFollowingInit (tVoid)
{
    tSInt res = OSAL_OK;


    DABMW_ServiceFollowing_ExtInt_MemorySet ((tPVoid)&DABMW_serviceFollowingData, 0x00, sizeof (DABMW_serviceFollowingData));    
    DABMW_serviceFollowingData.enableServiceFollowing = false;

    DABMW_ServiceFollowing_ExtInt_MemorySet ((tPVoid)&DABMW_serviceFollowingStatus, 0x00, sizeof (DABMW_serviceFollowingStatus));   
    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_INIT);
    
    DABMW_serviceFollowingStatus.originalSystemBand = DABMW_BAND_NONE;
    DABMW_serviceFollowingStatus.originalFrequency = DABMW_INVALID_FREQUENCY;      
    DABMW_serviceFollowingStatus.originalEid = DABMW_INVALID_ENSEMBLE_ID;
    DABMW_serviceFollowingStatus.originalSid= DABMW_INVALID_SERVICE_ID;
    DABMW_serviceFollowingStatus.original_DabServiceSelected = false;
	// add on etal
	DABMW_serviceFollowingStatus.originalHandle = ETAL_INVALID_HANDLE;
 
    DABMW_serviceFollowingStatus.currentSystemBand = DABMW_BAND_NONE;
    DABMW_serviceFollowingStatus.currentFrequency = DABMW_INVALID_FREQUENCY;      
    DABMW_serviceFollowingStatus.currentEid = DABMW_INVALID_ENSEMBLE_ID;
    DABMW_serviceFollowingStatus.currentSid= DABMW_INVALID_SERVICE_ID; 
	// add on etal
	DABMW_serviceFollowingStatus.currentHandle = ETAL_INVALID_HANDLE;

 

    DABMW_serviceFollowingStatus.alternateFrequency = DABMW_INVALID_FREQUENCY;
    DABMW_serviceFollowingStatus.alternateSystemBand = DABMW_BAND_NONE;
    DABMW_serviceFollowingStatus.alternateApp = DABMW_NONE_APP;
    DABMW_serviceFollowingStatus.alternateEid = DABMW_INVALID_ENSEMBLE_ID;
	// add on etal
	DABMW_serviceFollowingStatus.alternateHandle = ETAL_INVALID_HANDLE;

 

    DABMW_serviceFollowingData.initial_searchedPI = DABMW_INVALID_SERVICE_ID;

	// by default, enable SS                
	DABMW_ServiceFollowingSetup(false, DABMW_SF_DEFAULT_CONFIG_SS_ACTIVATED,
							DABMW_SF_DEFAULT_CONFIG_FOLLOW_SOFT_LINKAGE,
							DABMW_SF_DEFAULT_KIND_OF_SWITCH,
							DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_THR,
							DABMW_SF_DEFAULT_CONFIG_DAB_POOR_QUALITY_THR,
							DABMW_SF_DEFAULT_CONFIG_FM_FIELD_STRENGH_GOOD_QUALITY_THR,
							DABMW_SF_DEFAULT_CONFIG_FM_FIELD_STRENGH_POOR_QUALITY_THR,
							DABMW_SF_DEFAULT_CONFIG_DAB_TO_FM_DELTA_TIME,
							DABMW_SF_DEFAULT_CONFIG_FM_TO_DAB_DELTA_TIME,
							DABMW_SF_DEFAULT_CONFIG_DAB_TO_DAB_DELTA_TIME,
							DABMW_SF_DEFAULT_CONFIG_FM_TO_FM_DELTA_TIME);



	/* add internal threshold */
		// input internal set for now : 
	// threshold and counter for measure and evaluation criteria
	DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_DAB = DABMW_SF_NB_MEASURE_START_AF_SEARCH_DAB;
	DABMW_serviceFollowingData.counter_NbMeasureLossService_DAB = DABMW_SF_NB_MEASURE_LOSS_SERVICE_DAB;
	DABMW_serviceFollowingData.counter_NbMeasureSwitch_DAB = DABMW_SF_NB_MEASURE_CRITERIA_SWITCH_DAB;
	
	DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_FM = DABMW_SF_NB_MEASURE_START_AF_SEARCH_FM;
	DABMW_serviceFollowingData.counter_NbMeasureLossService_FM = DABMW_SF_NB_MEASURE_LOSS_SERVICE_FM;
	DABMW_serviceFollowingData.counter_NbMeasureSwitch_FM = DABMW_SF_NB_MEASURE_CRITERIA_SWITCH_FM;

	
	DABMW_serviceFollowingData.ponderation_CurrentQuality = DABMW_SF_CURRENT_QUALITY_PONDERATION;
	DABMW_serviceFollowingData.ponderation_NewQuality = DABMW_SF_NEW_QUALITY_PONDERATION;

	DABMW_serviceFollowingStatus.original_badQualityCounter = 0;
	DABMW_serviceFollowingStatus.original_mediumQualityCounter = 0;
	/* add configuration DAB First storage */
	DABMW_serviceFollowingStatus.configurationIsDabFirst = false;

	/* init of landscape building time 
	* emulate courante time + periodicity - 1st time for 1st time ...
	*/
	DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime = DABMW_INVALID_DATA;
	DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

	DABMW_serviceFollowingStatus.originalLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
	DABMW_serviceFollowingStatus.alternateLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();


	DABMW_ServiceFollowing_LinkInfo_Init();

    // 1 resource may be use for alternate : free it
    DABMW_ServiceFollowing_ResetMonitoredAF();

    // init the log info
    DABMW_ServiceFollowing_InitLogInfo();

	/* END EPR CHANGE */

#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    DABMW_ServiceFollowingSimulate_DAB_FM_LevelChangeReset ();
#endif


	// register functions
	DABMW_ServiceFollowing_ExtInt_RegisterExternalTune();
	DABMW_ServiceFollowing_ExtInt_RegisterExternalServiceSelect();

	// Addition of audio source selection
	// in case there is only a audio source selected, which do not need retune
	DABMW_ServiceFollowing_ExtInt_RegisterExternalAudioSourceSelection();

    return res;
}

// Diff ETAL
/* tSInt DABMW_ServiceFollowingOnTune (DABMW_mwAppTy application, DABMW_systemBandsTy systemBand, 
                                    tU32 frequency, tU32 ensemble, tU32 service, tBool isInternalTune) */
                                    
tSInt DABMW_ServiceFollowingOnTune (DABMW_mwAppTy application, DABMW_systemBandsTy systemBand, 
                                    tU32 frequency, tU32 ensemble, tU32 service, tBool isInternalTune, DABMW_SF_mwEtalHandleTy handle)                                   
{
    tSInt res = OSAL_OK;

	if (DABMW_BAND_NONE == systemBand)
	{
		 systemBand = DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(frequency);
	}
	
	/* EPR change 
	* specific processing if service following not enabled for the moment
	*/
	if (false == isInternalTune)
		{
		// Reset information depending on on-going activity



		// Init  data
		//
		DABMW_ServiceFollowing_StopOnGoingActivities();		

		// move to init state 
		// this will suspend any SF activity and any further crossing cases
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_INIT);
		
        // Reset current original    
		DABMW_serviceFollowingStatus.originalSystemBand = DABMW_BAND_NONE;
        DABMW_serviceFollowingStatus.originalFrequency = DABMW_INVALID_FREQUENCY;      
        DABMW_serviceFollowingStatus.originalEid = DABMW_INVALID_ENSEMBLE_ID;
        DABMW_serviceFollowingStatus.originalSid= DABMW_INVALID_SERVICE_ID;      


        DABMW_serviceFollowingStatus.currentFrequency = frequency;
        DABMW_serviceFollowingStatus.currentAudioUserApp = application;
        DABMW_serviceFollowingStatus.currentSid = service;
        DABMW_serviceFollowingStatus.currentEid = ensemble;
		// ETAL
		DABMW_serviceFollowingStatus.currentHandle = handle;

		/* EPR CHANGE */
		/* UPDATE some parameters */

		DABMW_serviceFollowingStatus.original_badQualityCounter = 0;
		DABMW_serviceFollowingStatus.original_mediumQualityCounter = 0;
        DABMW_serviceFollowingStatus.original_Quality = DABMW_ServiceFollowing_QualityInit();
        DABMW_serviceFollowingStatus.original_Quality_onBackground = DABMW_ServiceFollowing_QualityInit();


		if (DABMW_INVALID_SERVICE_ID != service) 
			{
            DABMW_serviceFollowingData.initial_searchedPI = service;
            
			if (true == DABMW_serviceFollowingData.enableServiceFollowing)
				{

                DABMW_ServiceFollowing_EnterIdleMode(application, systemBand, service, ensemble, frequency);    
                
                // configure the SS for this new original
#if (defined CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
                DABMW_ServiceFollowing_ConfigureSeamlessForOriginal();
#endif // #if (defined CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)                  
                
				/* enable service following */
                DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);

				// Set the initial switching time now because we would
		        // like to avoid to switch immediately
        		DABMW_serviceFollowingStatus.deltaTimeFromLastSwitch = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime (); 
				}
			else
				{
				/* remain in init state, until service following is enabled */
                DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_INIT);
				}
			

            //
            // TO BE CHECKED : I suppose the DabFist should be set only on audio port selection...
            //
            
          	/* Set the DAB FIRST information for correlation & delay later on */
			if (DABMW_ServiceFollowing_ExtInt_IsApplicationDab(application))
				{
				DABMW_serviceFollowingStatus.configurationIsDabFirst = true;
                // we should set that service is selected
                DABMW_serviceFollowingStatus.original_DabServiceSelected = true;
				}
			else
				{
				DABMW_serviceFollowingStatus.configurationIsDabFirst = false;
                DABMW_serviceFollowingStatus.original_DabServiceSelected = false;
				}
            
			}
		else
			{
			/* for service following, RDS is required */
			if (DABMW_ServiceFollowing_ExtInt_IsFMBand(systemBand))
				{
                if (true == DABMW_serviceFollowingData.enableServiceFollowing)  
                    {
#if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)

                    DABMW_ServiceFollowing_EnableRds(application, false);

    		 	   	DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (frequency, NULL, &DABMW_ServiceFollowing_OnPI_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
                    }

                // set configuration DAB first to false
                DABMW_serviceFollowingStatus.configurationIsDabFirst = false;
				}
            else
                {
                    
                    DABMW_serviceFollowingStatus.configurationIsDabFirst = true;
                    DABMW_serviceFollowingStatus.original_DabServiceSelected = false;
                }
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_INIT);
			}
        
            
			return OSAL_ERROR;
	}
	      
    return res;
}

tSInt DABMW_ServiceFollowingOnAudioPortUserChange (DABMW_mwAppTy application, tBool isInternalTune)
{

	/* EPR TMP CHANGE 
	* we should not do a change of SF status ... 
	* and to be aligned with ServiceFollowingOnTune, I suppose it should here be the isInternalTune == false
	*
	*/
    if (true == isInternalTune)
    {
        //   DABMW_serviceFollowingStatus.status = DABMW_SF_STATE_INIT; 

        // on internal tune, we should not change anything ... 
        // DABMW_serviceFollowingStatus.currentAudioUserApp = application;      
    }
    else
    {
        DABMW_serviceFollowingStatus.originalAudioUserApp = application;  

        DABMW_serviceFollowingStatus.currentAudioUserApp = application;   
        
    }        

	/* set the audio configuration :
	* if Port != DAB port =>  reset the DAB 1st info
	*
	*/
	/*
	* here tbd : I think that if it is an internal tune we may not need to set to DAB FIRST = false
	* typically : when seamless... ?
	*/
	if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(application))
		{
		DABMW_serviceFollowingStatus.configurationIsDabFirst = false;
		}
    else
        {
        DABMW_serviceFollowingStatus.configurationIsDabFirst = true;
        }
	
    return OSAL_OK;
}

tSInt DABMW_ServiceFollowingSetup (tBool enableServiceFollowing, tBool seamlessSwitchingMode,
                                   tBool followSoftLinkage, tU8 kindOfSwitch,                                
                                   tU32 dabGoodQualityThr, tU32 dabPoorQualityThr,
                                   tU8 fmGoodQualityThr, tU8 fmPoorQualityThr,
                                   tU8 dabToFmDeltaTime, tU8 fmToDabDeltaTime,
                                   tU8 dabToDabDeltaTime, tU8 fmToFmDeltaTime)
{   
    tSInt res = OSAL_OK;

	
    // Init thresholds (service following algorithm input data)
    if (true == enableServiceFollowing)
    {
        DABWM_ServiceFollowing_Enable();
    }
    else
    {
        DABWM_ServiceFollowing_Disable();
    }


#ifdef  CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
		/* if seamless not active : keep to  0 */
    DABMW_serviceFollowingData.seamlessSwitchingMode  = seamlessSwitchingMode;
#endif // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

    DABMW_serviceFollowingData.followSoftLinkage      = followSoftLinkage;
    DABMW_serviceFollowingData.kindOfSwitch           = kindOfSwitch;
    if (kindOfSwitch & DABMW_SF_MASK_DAB_DAB)
    {        
        DABMW_serviceFollowingData.dabToDabIsActive = true;
    }
    else
    {
        DABMW_serviceFollowingData.dabToDabIsActive = false;
    }
    if (kindOfSwitch & DABMW_SF_MASK_DAB_FM)
    {        
        DABMW_serviceFollowingData.dabToFmIsActive = true;
        DABMW_serviceFollowingData.fmToDabIsActive = true;
    }
    else
    {
        DABMW_serviceFollowingData.dabToFmIsActive = false;
        DABMW_serviceFollowingData.fmToDabIsActive = false;
    }
    if (kindOfSwitch & DABMW_SF_MASK_FM_FM)
    {        
        DABMW_serviceFollowingData.fmToFmIsActive = true;
    }
    else
    {
        DABMW_serviceFollowingData.fmToFmIsActive = false;
    }
    

    // set the quality structure thresholds
    // for DAB
    // direct pure value

    DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber = dabGoodQualityThr * 1;
    DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.audio_ber_level = DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_AUDIO_BER_LEVEL;
    
    DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber = dabPoorQualityThr * 1;
    DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.audio_ber_level = DABMW_SF_DEFAULT_CONFIG_DAB_POOR_QUALITY_AUDIO_BER_LEVEL;
    

    DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.fic_ber = DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_DABPLUS_DMB_THR;
    DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.audio_ber_level = DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_AUDIO_BER_LEVEL;

    DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.fic_ber = DABMW_SF_DEFAULT_CONFIG_DAB_POOR_QUALITY_DABPLUS_DMB_THR;
    DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.audio_ber_level = DABMW_SF_DEFAULT_CONFIG_DAB_POOR_QUALITY_AUDIO_BER_LEVEL;
     

    // for FM
    DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.fieldStrength_dBuV = fmGoodQualityThr;
    DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.combinedQ = DABMW_SF_DEFAULT_CONFIG_FM_COMBINEDQ_GOOD_QUALITY_THR;
    DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.multipath = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MULTIPATH;
    DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.adjacentChannel = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_ADJACENT_CHANNEL;
    DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.detuning = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DETUNING;
    DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.deviation = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DEVIATION;

    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV = fmPoorQualityThr;
    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ = DABMW_SF_DEFAULT_CONFIG_FM_COMBINEDQ_POOR_QUALITY_THR;
    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.multipath = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MULTIPATH;
    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.adjacentChannel = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_ADJACENT_CHANNEL;
    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.detuning = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DETUNING;
    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.deviation = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DEVIATION;
	
	DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.mpxNoise= DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MPXNOISE;
	DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.snr = DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_SNR;

    // Set the Strategy choice
    // for FM
    DABMW_serviceFollowingData.strategy_FM = DABMW_SF_DEFAULT_CONFIG_STRATEGY_FM;

    // for DAB
    DABMW_serviceFollowingData.strategy_DAB = DABMW_SF_DEFAULT_CONFIG_STRATEGY_DAB;

    //Hysteresis
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.fic_ber = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_FIC_BER_GOOD_QUALITY * 1;
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.audio_ber_level = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_AUDIO_BER_LEVEL_GOOD_QUALITY;
    
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_FIELD_STRENGH_GOOD_QUALITY;
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.combinedQ = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_COMBINEDQ_GOOD_QUALITY;
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.multipath = 0;
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.adjacentChannel = 0;
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.detuning = 0;
    DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.deviation = 0;

    
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.fic_ber = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_FIC_BER_POOR_QUALITY * 1;
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.audio_ber_level = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_AUDIO_BER_LEVEL_POOR_QUALITY;
   
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_FIELD_STRENGH_POOR_QUALITY;
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.combinedQ = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_COMBINEDQ_POOR_QUALITY;
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.multipath = 0;
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.adjacentChannel = 0;
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.detuning = 0;
    DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.deviation = 0;
    DABMW_serviceFollowingStatus.fm_hysteresis_start_time = DABMW_INVALID_DATA;
    DABMW_serviceFollowingStatus.dab_hysteresis_start_time = DABMW_INVALID_DATA;
        
    DABMW_serviceFollowingData.Quality_Threshold_Hysteresis_Duration = DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DURATION * 1000;

  
    DABMW_serviceFollowingData.dabToFmDeltaTime       = dabToFmDeltaTime * 1000;  // User passes ms
    DABMW_serviceFollowingData.fmToDabDeltaTime       = fmToDabDeltaTime * 1000;  // User passes ms
    DABMW_serviceFollowingData.dabToDabDeltaTime      = dabToDabDeltaTime * 1000; // User passes ms
    DABMW_serviceFollowingData.fmToFmDeltaTime        = fmToFmDeltaTime * 1000;   // User passes ms


    // init for PI timing & decode configuration
    // stored the time to wait for a PI as configurable
    // and the threshold to decode a PI
    //
    DABMW_serviceFollowingData.maxTimeToDecodePi = DABMW_SF_PI_SEARCH_DELTA_TIME_MS;
    DABMW_serviceFollowingData.PI_decoding_QualityThreshold = DABMW_serviceFollowingData.poorQuality_Threshold;
    DABMW_serviceFollowingData.PI_validityTime = DABMW_SF_PI_VALIDITY_TIME_MS;

	// PS information when in landscape building
	DABMW_serviceFollowingData.maxTimeToDecodePs = DABMW_SF_PS_SEARCH_DELTA_TIME_MS;
	
	/* Init some more delay value 
	* which could be made configurable in the future 
	*/
	DABMW_serviceFollowingData.measurementPeriodicityFM = DABMW_SF_TIME_FOR_MEASUREMENT;
	DABMW_serviceFollowingData.measurementPeriodicityDAB = DABMW_SF_TIME_FOR_MEASUREMENT;
	DABMW_serviceFollowingData.AFsearchPeriodicity = DABMW_SF_SEARCH_AF_TIME;
    DABMW_serviceFollowingData.FullScanAFsearchPeriodicity = DABMW_SF_FULL_SEARCH_AF_TIME;
	DABMW_serviceFollowingData.ServiceRecoverySearchPeriodicity = DABMW_SF_SERVICE_RECOVERY_TIME;
	DABMW_serviceFollowingData.LandscapeBuildingScanPeriodicity = DABMW_SF_LANDSCAPE_BUILDING_PERIODICITY_TIME;
	DABMW_serviceFollowingData.followHardLinkage      = DABMW_SF_DEFAULT_CONFIG_FOLLOW_HARD_LINKAGE;
	DABMW_serviceFollowingData.FMLandscapeDelayEstimationPeriodicity = DABMW_SF_FM_LANDSCAPE_BUILDING_PERIODICITY_TIME;
    DABMW_serviceFollowingData.LogInfoStatusPeriodicity = DABMW_SF_LOG_INFO_STATUS_PERIODICITY_TIME;

	/* default is regional mode is on : when not supported */
	DABMW_serviceFollowingData.regionalModeIsOn = false;


	
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
	/* Init / setup the Seamless parameters */
	DABMW_ServiceFollowing_SeamLessParametersInit();
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)

	/* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* set simulated level DAB & FM to auto */
	DABMW_serviceFollowingData.FM_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
	DABMW_serviceFollowingData.DAB_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* END EPR CHANGE */


    // Configure the SEEK threshold according to the setup 
    //
    DABMW_ServiceFollowing_ConfigureAutoSeek();



    return res;
}


/* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
/* add procedure for emulating the DABMW Level */
// command is 
// len = 1 byte
// F1 = 4 bytes
// F1_level = 1 byte
// F2 = 4 bytes
// F2_level = 1 byte
// 
// if F1 or F2 is set to a frequency => this is used to restrict the search on that frequency
// the level is simulated / set depending on Fx_level = poor, medium, good or auto
//
// Specific value : ANY_DAB_FREQ (0x00000001) & ANY_FM_FREQ (0x00000002)
// if F1 is set to ANY_DAB_FREQ  => it means all the DAB mobility applyes
// if F1 is set to ANY_FM_FREQ => it means all the FM mobitlity applyes
//
//
// this enables the following : 
// 
// F1 & F2 set : restrict mobility to F1 & F2 only, apply the F(x) level simulated for each frequency
// example : FM to FM moiblity, or DAB to DAB , or FM to DAB
// 
// F1 set to FM, F2 set to ANY_DAB_FREQ : retrcit the mobility to F1 in FM, but normal processing on DAB.
// F1 set to DAB, F2 set to ANY_FM_FREQ: retrcit the mobility to F1 in DAB, but normal processing on FM.
// F1 set to FM, F2 set to ANY_FM_FREQ : has no effect
// F1 set to DAB, F2 set to ANY_DAB_FREQ : has no effect
//
// if F2 is set to invalid : apply the changes to F1 only.
// if F1 & F2 are set to invalid : disable every thing
//


tSInt DABMW_ServiceFollowingSimulate_DAB_FM_LevelChange (tU32 vI_frequency1, tU8 vI_F1_levelChangeType, tU32 vI_frequency2, tU8 vI_F2_levelChangeType)
{
	tSInt vl_res = OSAL_OK;
    DABMW_SF_systemBandsTy vl_band_f1, vl_band_f2;
    tBool vl_f1_is_dab = false, vl_f2_is_dab = false, vl_f1_is_fm = false, vl_f2_is_fm = false;
    tU8 vl_numLockedFreq = 0;
    
    // start by resetting all previous
    DABMW_ServiceFollowingSimulate_DAB_FM_LevelChangeReset();
    
    // check on F1 input parameters
    // possible values : 
    // INVALID FREQ
    // DABMW_SF_SIMULATE_FREQUENCY_ALL_DAB
    // DABMW_SF_SIMULATE_FREQUENCY_ALL_FM
    //
    
    if (DABMW_SF_SIMULATE_FREQUENCY_ALL_DAB == vI_frequency1)
    {      
        switch (vI_F1_levelChangeType)
        {
            case DABMW_SERVICE_FOLLOWING_SIMULATE_LOW:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD:
                DABMW_serviceFollowingData.DAB_LevelSimulate = vI_F1_levelChangeType;
                break;
            default:
                DABMW_serviceFollowingData.DAB_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
        }
    }
    else if (DABMW_SF_SIMULATE_FREQUENCY_ALL_FM == vI_frequency1)
    {
        switch (vI_F1_levelChangeType)
        {
            case DABMW_SERVICE_FOLLOWING_SIMULATE_LOW:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD:
                DABMW_serviceFollowingData.FM_LevelSimulate = vI_F1_levelChangeType;
                break;
            default:
                DABMW_serviceFollowingData.FM_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
        }
    }
    else
    {
        // check if frequency in range
        vl_band_f1 = DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(vI_frequency1);

        switch (vI_F1_levelChangeType)
            {
            case DABMW_SERVICE_FOLLOWING_SIMULATE_LOW:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD:
                DABMW_serviceFollowingData.lockFrequency_1_level = vI_F1_levelChangeType;
                break;
             default:
                DABMW_serviceFollowingData.lockFrequency_1_level = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
             }
        

        if (DABMW_ServiceFollowing_ExtInt_IsFMBand(vl_band_f1))
            {
            vl_f1_is_fm = true;
            DABMW_serviceFollowingData.lockFeatureActivated_FM = true;
            DABMW_serviceFollowingData.lockFrequency_1 = vI_frequency1;
            vl_numLockedFreq++;
            }
        else if (DABMW_ServiceFollowing_ExtInt_IsDABBand(vl_band_f1))
            {
            vl_f1_is_dab = true;
            DABMW_serviceFollowingData.lockFeatureActivated_DAB = true;
            DABMW_serviceFollowingData.lockFrequency_1 = vI_frequency1;
            vl_numLockedFreq++;
            }            
        else
            {
            // incorrect freq => 
            DABMW_serviceFollowingData.lockFrequency_1_level = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
            }

        
    }

     // check on F2 input parameters
     // possible values : 
     // INVALID FREQ
     // DABMW_SF_SIMULATE_FREQUENCY_ALL_DAB
     // DABMW_SF_SIMULATE_FREQUENCY_ALL_FM
     //

    if (DABMW_SF_SIMULATE_FREQUENCY_ALL_DAB == vI_frequency2)
    {
        
        switch (vI_F2_levelChangeType)
        {
            case DABMW_SERVICE_FOLLOWING_SIMULATE_LOW:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD:
                DABMW_serviceFollowingData.DAB_LevelSimulate = vI_F2_levelChangeType;
                break;
            default:
                DABMW_serviceFollowingData.DAB_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
        }
    }
    else if (DABMW_SF_SIMULATE_FREQUENCY_ALL_FM == vI_frequency2)
    {
        switch (vI_F2_levelChangeType)
        {
            case DABMW_SERVICE_FOLLOWING_SIMULATE_LOW:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD:
                DABMW_serviceFollowingData.FM_LevelSimulate = vI_F2_levelChangeType;
                break;
            default:
                DABMW_serviceFollowingData.FM_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
        }
    }
    else
    {
        // check if frequency in range
        vl_band_f2 = DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(vI_frequency2);

        switch (vI_F2_levelChangeType)
            {
            case DABMW_SERVICE_FOLLOWING_SIMULATE_LOW:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM:
            case DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD:
                DABMW_serviceFollowingData.lockFrequency_2_level = vI_F2_levelChangeType;
                break;
             default:
                DABMW_serviceFollowingData.lockFrequency_2_level = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
             }
        

        if (DABMW_ServiceFollowing_ExtInt_IsFMBand(vl_band_f2))
            {
            vl_f2_is_fm = true;  
            DABMW_serviceFollowingData.lockFeatureActivated_FM = true;
            DABMW_serviceFollowingData.lockFrequency_2 = vI_frequency2;
            vl_numLockedFreq++;
            }
        else if (DABMW_ServiceFollowing_ExtInt_IsDABBand(vl_band_f2))
            {
            vl_f2_is_dab = true;
            DABMW_serviceFollowingData.lockFeatureActivated_DAB = true;
            DABMW_serviceFollowingData.lockFrequency_2 = vI_frequency2;
            vl_numLockedFreq++;
            }            
        else
            {
            // incorrect freq => 
            DABMW_serviceFollowingData.lockFrequency_2_level = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
            }        
    }


    // now some combined check
    // F1 set to FM, F2 set to ANY_FM_FREQ : has no effect
    // F1 set to DAB, F2 set to ANY_DAB_FREQ : has no effect

    if (((DABMW_SF_SIMULATE_FREQUENCY_ALL_DAB == vI_frequency2) && (true == vl_f1_is_dab))
        ||((DABMW_SF_SIMULATE_FREQUENCY_ALL_DAB == vI_frequency1) && (true == vl_f2_is_dab))
        )
        {
        // this is an error  request for lock on a Freq DAB + ANY DAB
        vl_res = OSAL_ERROR;
        }
    else if (((DABMW_SF_SIMULATE_FREQUENCY_ALL_FM == vI_frequency2) && (true == vl_f1_is_fm))
        ||((DABMW_SF_SIMULATE_FREQUENCY_ALL_FM == vI_frequency1) && (true == vl_f2_is_fm))
        )
        {
        // this is an error  request for lock on a Freq FM + ANY FM
        vl_res = OSAL_ERROR; 
        }
        

    if (OSAL_ERROR == vl_res)
        {
            DABMW_ServiceFollowingSimulate_DAB_FM_LevelChangeReset();
        }
    else
        {
            // check if full locked
            if (vl_numLockedFreq == 2) 
                {
                DABMW_serviceFollowingData.lockFeatureActivated_2Freq = true;
                }
        }
    
	return vl_res;
}

tVoid DABMW_ServiceFollowingSimulate_DAB_FM_LevelChangeReset ()
{
    DABMW_serviceFollowingData.lockFeatureActivated_FM = false;
    DABMW_serviceFollowingData.lockFeatureActivated_DAB = false;
    DABMW_serviceFollowingData.lockFeatureActivated_2Freq = false;
    DABMW_serviceFollowingData.FM_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
    DABMW_serviceFollowingData.DAB_LevelSimulate = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
    DABMW_serviceFollowingData.lockFrequency_1 = DABMW_INVALID_FREQUENCY;
    DABMW_serviceFollowingData.lockFrequency_2 = DABMW_INVALID_FREQUENCY;
    DABMW_serviceFollowingData.lockFrequency_1_level= DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;
    DABMW_serviceFollowingData.lockFrequency_2_level = DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO;

}

#endif //CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
/* END EPR CHANGE */


tSInt DABWM_SetServiceFollowingParameters(DABMW_SF_mwAppTy vI_app, tPU16 p_cellParamIndex, tPS32 p_cellParamValue,  tS16 vI_cellParamSize)
{
        


    tSInt vl_res = OSAL_OK; 
    tSInt vl_len = 0;
    
#if defined (CONFIG_DABMW_SERVICEFOLLOWING_SUPPORTED)

    tSInt vl_srcIndex = 0;
    tU8   vl_translatedIndex;
  
    /* basic check : on nb params */
    if ((vI_cellParamSize < 1) || (vI_cellParamSize > DABMW_API_SF_NB_PARAM_MAX))
        {
         DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (DABWM_SetServiceFollowingParameters) : param size error %d \n",
                                            vI_cellParamSize);
        
            return OSAL_ERROR;
        }

    /* basic check : valid indexes */
     for (vl_srcIndex = 0; vl_srcIndex < vI_cellParamSize; vl_srcIndex++)
        {
        if ((p_cellParamIndex[vl_srcIndex] < DABMW_API_SERVICE_FOLLOWING_PARAM_S) 
            || (p_cellParamIndex[vl_srcIndex] > (DABMW_API_SERVICE_FOLLOWING_PARAM_S+DABMW_API_SERVICE_FOLLOWING_MAXSIZE))
            )
            {
             DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (DABWM_SetServiceFollowingParameters) : Set Parameter Request ERROR. Param_ID = 0x%04x, Param_Value = %d \n",
                                            p_cellParamIndex[vl_srcIndex],
                                            p_cellParamValue[vl_srcIndex]);
        
                return OSAL_ERROR;
            }
        }
 

    /* init the  index */
    vl_srcIndex = 0;
        
    /* loop on all index values */
    for (vl_srcIndex = 0; vl_srcIndex < vI_cellParamSize; vl_srcIndex++)
        {
        vl_translatedIndex = p_cellParamIndex[vl_srcIndex]-DABMW_API_SERVICE_FOLLOWING_PARAM_S;
       
        /* store in local variable */
        vl_res = OSAL_OK;
        
        switch (vl_translatedIndex)
            {            
            case DABMW_API_SF_PARAM_ENABLE_SF:
                if (DABMW_API_SF_FEATURE_DISABLE == p_cellParamValue[vl_srcIndex])
                {                    
                    DABWM_ServiceFollowing_Disable();
                }
                else if (DABMW_API_SF_FEATURE_ENABLE == p_cellParamValue[vl_srcIndex])
                {
                    DABWM_ServiceFollowing_Enable();
                }
                else
                {
                    // this is an error 
                    vl_res = OSAL_ERROR;
                }
                                
                break;
                
            case DABMW_API_SF_PARAM_ENABLE_SS_MODE:	
                if (DABMW_API_SF_FEATURE_DISABLE == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.seamlessSwitchingMode = false;
                    // nothing else to do
                    // If either a seamless was on going... 
                    // just wait the answer, the switch will be without it because handle with variable
                }
                else if (DABMW_API_SF_FEATURE_ENABLE == p_cellParamValue[vl_srcIndex])
                {
                    DABMW_serviceFollowingData.seamlessSwitchingMode = true;

                    // configure SS current source if one is configured
                    if (DABMW_NONE_APP != DABMW_serviceFollowingStatus.originalAudioUserApp)
                    {                        
                        // configure the SS for this new original
#if (defined CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
                        DABMW_ServiceFollowing_ConfigureSeamlessForOriginal();
#endif // #if (defined CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)                           
                    }
                }
                else
                {
                    // this is an error 
                    
                }
                break;
              
            case DABMW_API_SF_PARAM_FOLLOW_SOFT_LINKAGE:	
                if (DABMW_API_SF_FEATURE_DISABLE == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.followSoftLinkage= false;
                }
                else if (DABMW_API_SF_FEATURE_ENABLE == p_cellParamValue[vl_srcIndex])
                {
                    DABMW_serviceFollowingData.followSoftLinkage = true;
                }
                else
                {
                    // this is an error 
                    vl_res = OSAL_ERROR;
                }
                break;
                
            case DABMW_API_SF_PARAM_FOLLOW_HARD_LINKAGE:	
                if (DABMW_API_SF_FEATURE_DISABLE == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.followHardLinkage= false;
                }
                else if (DABMW_API_SF_FEATURE_ENABLE == p_cellParamValue[vl_srcIndex])
                {
                    DABMW_serviceFollowingData.followHardLinkage = true;
                }
                else
                {
                    // this is an error 
                    vl_res = OSAL_ERROR;
                }
                break;
                
            case DABMW_API_SF_PARAM_KIND_OF_SWITCH:	
                
                DABMW_serviceFollowingData.kindOfSwitch           = p_cellParamValue[vl_srcIndex] & DABMW_SF_MASK_KIND_OF_SWITCH;

                // DAB TO DAB
                if (DABMW_serviceFollowingData.kindOfSwitch & DABMW_SF_MASK_DAB_DAB)
                {        
                    DABMW_serviceFollowingData.dabToDabIsActive = true;
                }
                else
                {
                    DABMW_serviceFollowingData.dabToDabIsActive = false;
                }

                // DAB TO FM 
                if (DABMW_serviceFollowingData.kindOfSwitch & DABMW_SF_MASK_DAB_FM)
                {        
                    DABMW_serviceFollowingData.dabToFmIsActive = true;
                    DABMW_serviceFollowingData.fmToDabIsActive = true;
                }
                else
                {
                    DABMW_serviceFollowingData.dabToFmIsActive = false;
                    DABMW_serviceFollowingData.fmToDabIsActive = false;
                }

                // FM TO FM
                if (DABMW_serviceFollowingData.kindOfSwitch & DABMW_SF_MASK_FM_FM)
                {        
                    DABMW_serviceFollowingData.fmToFmIsActive = true;
                }
                else
                {
                    DABMW_serviceFollowingData.fmToFmIsActive = false;
                }
                
                break;
                
            case DABMW_API_SF_PARAM_REGIONAL_MODE:	
                if (DABMW_API_SF_FEATURE_DISABLE == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.regionalModeIsOn = false;
                }
                else if (DABMW_API_SF_FEATURE_ENABLE == p_cellParamValue[vl_srcIndex])
                {
                    DABMW_serviceFollowingData.regionalModeIsOn = true;
                }
                else
                {
                    // this is an error 
                    vl_res = OSAL_ERROR;
                }
                break;
                
            case DABMW_API_SF_PARAM_FM_STRATEGY:	
                if (DABMW_API_SF_FM_STRATEGY_FIELD_STRENGH == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.strategy_FM = DABMW_SF_STRATEGY_FM_FIELD_STRENGH;
                }
                else if (DABMW_API_SF_FM_STRATEGY_COMBINED_Q == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.strategy_FM = DABMW_SF_STRATEGY_FM_COMBINED_QUALITY;
                }
                else
                {
                    // this is an error 
                    vl_res = OSAL_ERROR;
                }
                break;

            case DABMW_API_SF_PARAM_DAB_STRATEGY:	                    
                if (DABMW_API_SF_DAB_STRATEGY_FIC_BER == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.strategy_DAB = DABMW_SF_STRATEGY_DAB_FIC_BER;
                }
                else if (DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL == p_cellParamValue[vl_srcIndex])
                {                    
                    DABMW_serviceFollowingData.strategy_DAB = DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL;
                }               
                else
                {
                    // this is an error 
                    vl_res = OSAL_ERROR;
                }
                break;

            case DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_FIC_BER:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_DAB_FIC_BER_UNIT;
                
                break;
                
            case DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_FIC_BER:	
                 // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_DAB_FIC_BER_UNIT;
                break;

            case DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_FIC_BER_DABPLUS_DMB:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.fic_ber = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_DAB_FIC_BER_UNIT;
                
                break;
                
            case DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_FIC_BER_DABPLUS_DMB:	
                 // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.fic_ber = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_DAB_FIC_BER_UNIT;
                break;

            case DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_AUDIO_BER_LEVEL:  
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.audio_ber_level = p_cellParamValue[vl_srcIndex];               
                DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.audio_ber_level = p_cellParamValue[vl_srcIndex];
                break;

            case DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_AUDIO_BER_LEVEL:	
                 // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.audio_ber_level = p_cellParamValue[vl_srcIndex];
                DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.audio_ber_level = p_cellParamValue[vl_srcIndex];
            
                break;

                
            case DABMW_API_SF_PARAM_FM_GOOD_QUAL_THR_FS_DBUV:	
                 // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.fieldStrength_dBuV = p_cellParamValue[vl_srcIndex];
               
                break;
                
            case DABMW_API_SF_PARAM_FM_POOR_QUAL_THR_FS_DBUV:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV = p_cellParamValue[vl_srcIndex];
 
                // It requires an update of the SEEK DATA setting as well
                //
                
                break;
                
            case DABMW_API_SF_PARAM_FM_GOOD_QUAL_THR_COMBINEDQ:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.combinedQ = p_cellParamValue[vl_srcIndex];    
                break;
                
            case DABMW_API_SF_PARAM_FM_POOR_QUAL_THR_COMBINEDQ:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ = p_cellParamValue[vl_srcIndex];
     
                break; 

           case DABMW_API_SF_PARAM_FM_RDS_QUAL_THR_FS_DBUV:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.PI_decoding_QualityThreshold.fmQuality.fieldStrength_dBuV = p_cellParamValue[vl_srcIndex];
         
                break;

           case DABMW_API_SF_PARAM_FM_RDS_QUAL_THR_FS_COMBINEDQ: 
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.PI_decoding_QualityThreshold.fmQuality.combinedQ = p_cellParamValue[vl_srcIndex];              
                break;


            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_GOOD_QUAL_THR_FIC_BER: 
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.fic_ber = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_DAB_FIC_BER_UNIT;
                              
                break;


            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_POOR_QUAL_THR_FIC_BER: 
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.fic_ber = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_DAB_FIC_BER_UNIT;
                              
                break;  

            case DABMW_API_SF_PARAM_HYSTERESIS_FM_GOOD_QUAL_THR_FS_DBUV: 
                // We consider what is set is good, 
                // directly use the value without check
               DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV = p_cellParamValue[vl_srcIndex];
                
                              
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_FM_POOR_QUAL_THR_FS_DBUV: 
                // We consider what is set is good, 
                // directly use the value without check
               DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV = p_cellParamValue[vl_srcIndex];
                             
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_FM_GOOD_QUAL_THR_COMBINEDQ: 
                // We consider what is set is good, 
                // directly use the value without check
               DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.combinedQ = p_cellParamValue[vl_srcIndex];
                              
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_FM_POOR_QUAL_THR_COMBINEDQ: 
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.combinedQ = p_cellParamValue[vl_srcIndex];
                              
                break;
                
            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_GOOD_QUAL_THR_AUDIO_BER_LEVEL: 
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.audio_ber_level = p_cellParamValue[vl_srcIndex];
                              
                break;                

            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_POOR_QUAL_THR_AUDIO_BER_LEVEL: 
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.audio_ber_level = p_cellParamValue[vl_srcIndex];
                              
                break;

           case DABMW_API_SF_PARAM_HYSTERESIS_PENALITY_DURATION:
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.Quality_Threshold_Hysteresis_Duration = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                              
                break;

            case DABMW_API_SF_PARAM_DAB_TO_FM_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.dabToFmDeltaTime = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;  
                break;

            case DABMW_API_SF_PARAM_DAB_TO_DAB_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.dabToDabDeltaTime = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
     
                break;
                
            case DABMW_API_SF_PARAM_FM_TO_FM_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.fmToFmDeltaTime = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;     
                break;
                
            case DABMW_API_SF_PARAM_FM_TO_DAB_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.fmToDabDeltaTime = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;   
                break;

            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_AF_SEARCH_DAB:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_DAB= p_cellParamValue[vl_srcIndex];   
                break;
  
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_LOSS_SERVICE_DAB:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.counter_NbMeasureLossService_DAB= p_cellParamValue[vl_srcIndex];   
                break;
                
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_CELL_CHANGE_DAB:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.counter_NbMeasureSwitch_DAB = p_cellParamValue[vl_srcIndex];   
                break;

			case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_AF_SEARCH_FM:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_FM = p_cellParamValue[vl_srcIndex];   
                break;
  
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_LOSS_SERVICE_FM:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.counter_NbMeasureLossService_FM = p_cellParamValue[vl_srcIndex];   
                break;
                
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_CELL_CHANGE_FM:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.counter_NbMeasureSwitch_FM = p_cellParamValue[vl_srcIndex];   
                break;	

            case DABMW_API_SF_AVERAGING_PONDERATION_CURRENT_QUALITY:
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.ponderation_CurrentQuality = p_cellParamValue[vl_srcIndex];   
                break;
                
            case DABMW_API_SF_AVERAGING_PONDERATION_NEW_QUALITY:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.ponderation_NewQuality = p_cellParamValue[vl_srcIndex];   
                break;
                
            case DABMW_API_SF_PERIODICITY_AF_SEARCH:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.AFsearchPeriodicity = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;   
                break;
                
            case DABMW_API_SF_PERIODICITY_FULL_SCAN_AF_SEARCH:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.FullScanAFsearchPeriodicity = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_SERVICE_RECOVERY_SEARCH:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.ServiceRecoverySearchPeriodicity = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_LANDSCAPE_BUILDING:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.LandscapeBuildingScanPeriodicity = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_LANDSCAPE_FOR_SEAMLESS:
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.FMLandscapeDelayEstimationPeriodicity = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_MEASUREMENTS_DAB_SELECTED:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.measurementPeriodicityDAB = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_MEASUREMENTS_FM_SELECTED:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.measurementPeriodicityFM = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;

            case DABMW_API_SF_LOG_INFO_STATUS_PERIODICITY_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.LogInfoStatusPeriodicity = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;

           case DABMW_API_SF_RDS_PI_MAX_WAIT_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.maxTimeToDecodePi = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;
                
                
          case DABMW_API_SF_PARAM_PI_VALIDITY_TIME:
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.PI_validityTime = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                                   
                 break;

          case DABMW_API_SF_RDS_PS_MAX_WAIT_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                DABMW_serviceFollowingData.maxTimeToDecodePs = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT; 
                break;               
                    
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK            
            case DABMW_API_SF_SS_CONFIDENCE_LEVEL_THRESHOLD:	
                DABMW_SS_EstimationParameter.ConfidenceLevelThreshold = p_cellParamValue[vl_srcIndex];
                break;
            case DABMW_API_SF_SS_MEAS_VALIDITY_DURATION:	
                DABMW_SS_EstimationParameter.measureValidityDuration = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                break;
            case DABMW_API_SF_SS_RECONF_PERIODICITY:
                DABMW_SS_EstimationParameter.reconfPeriodicity = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                break; 
            case DABMW_API_SF_SS_MINIMUM_TIME_BETWEEN_SS:
                DABMW_SS_EstimationParameter.minimumTimeBetween2Seamless= p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                break;

            case DABMW_API_SF_SS_FULL_ESTIMATION_START_WINDOW_IN_MS:
                DABMW_SS_EstimationParameter.FullEstimationStartWindowInMs = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                DABMW_SS_EstimationParameter.FullEstimationStartWindowInSample = DABMW_SEAMLESS_ESTIMATION_CONVERT_MS_IN_SAMPLE(DABMW_SS_EstimationParameter.FullEstimationStartWindowInMs);
                break;
                
            case DABMW_API_SF_SS_FULL_ESTIMATION_STOP_WINDOW_IN_MS:
                DABMW_SS_EstimationParameter.FullEstimationStopWindowInMs = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                DABMW_SS_EstimationParameter.FullEstimationStopWindowInSample = DABMW_SEAMLESS_ESTIMATION_CONVERT_MS_IN_SAMPLE(DABMW_SS_EstimationParameter.FullEstimationStopWindowInMs);
                break;
                
            case DABMW_API_SF_SS_RECONF_ESTIMATION_HALF_WINDOW_IN_MS:
                DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInMs = p_cellParamValue[vl_srcIndex] * DABMW_API_SF_TIME_UNIT;
                DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInSample = DABMW_SEAMLESS_ESTIMATION_CONVERT_MS_IN_SAMPLE(DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInMs);
                break;
            case DABMW_API_SF_SS_FULL_ESTIMATION_DOWN_SAMPLING:
                DABMW_SS_EstimationParameter.DownSampling = p_cellParamValue[vl_srcIndex];
                break;   
            case DABMW_API_SF_SS_RECONF_ESTIMATION_DOWN_SAMPLING:
                DABMW_SS_EstimationParameter.DownSamplingReconf = p_cellParamValue[vl_srcIndex];
                break;   
#endif // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK   

            default:
                    vl_res = OSAL_ERROR;
                    break;  
            }

            if (OSAL_OK == vl_res)
                {
                // increment the len : this is the number of param written
                vl_len++;
                }

#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   

                        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)  
                       DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (DABWM_SetServiceFollowingParameters) : Set Parameter Request. Param_ID = 0x%04x, Param_Value = %d, vl_res = %d \n",
                                            p_cellParamIndex[vl_srcIndex],
                                            p_cellParamValue[vl_srcIndex],
                                            vl_res);
        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   

#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                       DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   


                        
        }
#else
    vl_res = OSAL_ERROR;
	vl_len = 0;
#endif // #if defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)

    return vl_len;  
}

//-------------------------------------------------------------------------------------------------------------------
// DABWM_GetServiceFollowingParameters()
// Procedure to Get the Service Following Paramters
// parameters index are contained in p_cellParamIndex
// for each parameter , the value is returned in p_cellParamValue
//

//	Input Parameters:
//	1) vI_app (application target)
//	2) p_cellParamIndex (parameters index)
//	2) p_cellParamValue (parameters value to write)
//	3) vI_cellParamSize  (number of parameters)
//
// Output Parameters:
// 	1) function return number of parameters written (vl_len)
//-------------------------------------------------------------------------------------------------------------------


tSInt DABWM_GetServiceFollowingParameters  (DABMW_SF_mwAppTy vI_app, tPU16 p_cellParamIndex, tPS32 p_cellParamValue,  tS16 vI_cellParamSize)
{
   tSInt vl_len = 0;

    tSInt vl_res = OSAL_ERROR;  
#if defined (CONFIG_DABMW_SERVICEFOLLOWING_SUPPORTED)

    tSInt vl_srcIndex = 0;
    tU8   vl_translatedIndex;

     /* basic check : on nb params */
    if ((vI_cellParamSize < 1) || (vI_cellParamSize > DABMW_API_SF_NB_PARAM_MAX))
        {
            return OSAL_ERROR;
        }

    /* basic check : valid indexes */
     for (vl_srcIndex = 0; vl_srcIndex < vI_cellParamSize; vl_srcIndex++)
        {
        if ((p_cellParamIndex[vl_srcIndex] < DABMW_API_SERVICE_FOLLOWING_PARAM_S) 
            || (p_cellParamIndex[vl_srcIndex] > (DABMW_API_SERVICE_FOLLOWING_PARAM_S+DABMW_API_SERVICE_FOLLOWING_MAXSIZE))
            )
            {
                return OSAL_ERROR;
            }
        }
 

    /* init the  index */
    vl_srcIndex = 0;    
    
   	/* Now write parameter 
	*  seek Mute Time
	*/
	for (vl_srcIndex = 0; vl_srcIndex < vI_cellParamSize; vl_srcIndex++)
        {
        /* write values 
                */
        vl_translatedIndex = p_cellParamIndex[vl_srcIndex]-DABMW_API_SERVICE_FOLLOWING_PARAM_S;
       
        /* store in local variable */
        vl_res = OSAL_OK;
        
        switch (vl_translatedIndex)
            {            
            case DABMW_API_SF_PARAM_ENABLE_SF:
                if (false == DABMW_serviceFollowingData.enableServiceFollowing)
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_DISABLE ;                
                }
                else
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_ENABLE ;
                }
                break;
                
            case DABMW_API_SF_PARAM_ENABLE_SS_MODE:	
                if (false == DABMW_serviceFollowingData.seamlessSwitchingMode)
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_DISABLE ;                
                }
                else
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_ENABLE ;
                }

                break;
              
            case DABMW_API_SF_PARAM_FOLLOW_SOFT_LINKAGE:
                if (false == DABMW_serviceFollowingData.followSoftLinkage)
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_DISABLE ;                
                }
                else
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_ENABLE ;
                }
                
                break;
                
            case DABMW_API_SF_PARAM_FOLLOW_HARD_LINKAGE:	
                if (false == DABMW_serviceFollowingData.followHardLinkage)
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_DISABLE ;                
                }
                else
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_ENABLE ;
                }
                
                break;
                
            case DABMW_API_SF_PARAM_KIND_OF_SWITCH:	
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.kindOfSwitch;
               
                break;
                
            case DABMW_API_SF_PARAM_REGIONAL_MODE:
                if (false == DABMW_serviceFollowingData.regionalModeIsOn)
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_DISABLE ;                
                }
                else
                {
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FEATURE_ENABLE ;
                }
                
                break;
                
            case DABMW_API_SF_PARAM_FM_STRATEGY:	
                if (DABMW_SF_STRATEGY_FM_FIELD_STRENGH == DABMW_serviceFollowingData.strategy_FM)
                {                    
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FM_STRATEGY_FIELD_STRENGH;
                }
                else if (DABMW_SF_STRATEGY_FM_COMBINED_QUALITY == DABMW_serviceFollowingData.strategy_FM)
                {                    
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_FM_STRATEGY_COMBINED_Q;
                }
                else
                {
                    // this is an error 
                    // should not happen
                    vl_res = OSAL_ERROR;
                }
                break;

            case DABMW_API_SF_PARAM_DAB_STRATEGY:	                    
                if (DABMW_SF_STRATEGY_DAB_FIC_BER == DABMW_serviceFollowingData.strategy_DAB) 
                {                    
                    p_cellParamValue[vl_srcIndex] = DABMW_API_SF_DAB_STRATEGY_FIC_BER;
                }
                else if (DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL == DABMW_serviceFollowingData.strategy_DAB) 
                {                    
                    p_cellParamValue[vl_srcIndex] = DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL;
                }
                else
                {
                    // this is an error 
                    // should not happen
                    vl_res = OSAL_ERROR;
                }
                break;

            case DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_FIC_BER:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber / DABMW_API_SF_DAB_FIC_BER_UNIT;
               
                break;
                
            case DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_FIC_BER:	
                 // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber / DABMW_API_SF_DAB_FIC_BER_UNIT;
                break;

           case DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_FIC_BER_DABPLUS_DMB:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.fic_ber / DABMW_API_SF_DAB_FIC_BER_UNIT;
               
                break;
                
            case DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_FIC_BER_DABPLUS_DMB:	
                 // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.fic_ber / DABMW_API_SF_DAB_FIC_BER_UNIT;
                break;

            case DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_AUDIO_BER_LEVEL:  
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.audio_ber_level;               
                break;

            case DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_AUDIO_BER_LEVEL:	
                 // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.audio_ber_level;
                
                break;
                
            case DABMW_API_SF_PARAM_FM_GOOD_QUAL_THR_FS_DBUV:	
                 // We consider what is set is good, 
                // directly use the value without check            
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.fieldStrength_dBuV;
                          
                break;
                
            case DABMW_API_SF_PARAM_FM_POOR_QUAL_THR_FS_DBUV:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV;

                // It requires an update of the SEEK DATA setting as well
                //
                
                break;
                
            case DABMW_API_SF_PARAM_FM_GOOD_QUAL_THR_COMBINEDQ:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.combinedQ;    
                break;
                
            case DABMW_API_SF_PARAM_FM_POOR_QUAL_THR_COMBINEDQ:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ ;
     
                break;
                
            case DABMW_API_SF_PARAM_FM_RDS_QUAL_THR_FS_DBUV:    
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.PI_decoding_QualityThreshold.fmQuality.fieldStrength_dBuV ;
                     
                break;
                
            case DABMW_API_SF_PARAM_FM_RDS_QUAL_THR_FS_COMBINEDQ: 
                // We consider what is set is good, 
                // directly use the value without check
               p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.PI_decoding_QualityThreshold.fmQuality.combinedQ;
                              
                break; 

            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_GOOD_QUAL_THR_FIC_BER: 
                // We consider what is set is good, 
                // directly use the value without check
               p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.fic_ber / DABMW_API_SF_DAB_FIC_BER_UNIT;
                              
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_POOR_QUAL_THR_FIC_BER: 
                // We consider what is set is good, 
                // directly use the value without check
               p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.fic_ber / DABMW_API_SF_DAB_FIC_BER_UNIT;
                              
                break;  


            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_GOOD_QUAL_THR_AUDIO_BER_LEVEL: 
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.audio_ber_level;
                              
                break;                

            case DABMW_API_SF_PARAM_HYSTERESIS_DAB_POOR_QUAL_THR_AUDIO_BER_LEVEL: 
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.audio_ber_level;
                              
                break;


            case DABMW_API_SF_PARAM_HYSTERESIS_FM_GOOD_QUAL_THR_FS_DBUV: 
                // We consider what is set is good, 
                // directly use the value without check
               p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV;
                              
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_FM_POOR_QUAL_THR_FS_DBUV: 
                // We consider what is set is good, 
                // directly use the value without check
               p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV;
                              
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_FM_GOOD_QUAL_THR_COMBINEDQ: 
                // We consider what is set is good, 
                // directly use the value without check
               p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.combinedQ;
                              
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_FM_POOR_QUAL_THR_COMBINEDQ: 
                // We consider what is set is good, 
                // directly use the value without check
               p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.combinedQ;
                              
                break;

            case DABMW_API_SF_PARAM_HYSTERESIS_PENALITY_DURATION:
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.Quality_Threshold_Hysteresis_Duration / DABMW_API_SF_TIME_UNIT;
                              
                break; 

                 
            case DABMW_API_SF_PARAM_DAB_TO_FM_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.dabToFmDeltaTime / DABMW_API_SF_TIME_UNIT;  
                break;

            case DABMW_API_SF_PARAM_DAB_TO_DAB_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.dabToDabDeltaTime / DABMW_API_SF_TIME_UNIT;
     
                break;
                
            case DABMW_API_SF_PARAM_FM_TO_FM_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.fmToFmDeltaTime / DABMW_API_SF_TIME_UNIT;     
                break;
                
            case DABMW_API_SF_PARAM_FM_TO_DAB_DELTA_SWITCH_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.fmToDabDeltaTime / DABMW_API_SF_TIME_UNIT;   
                break;

            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_AF_SEARCH_DAB:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_DAB;   
                break;
  
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_LOSS_SERVICE_DAB:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.counter_NbMeasureLossService_DAB;   
                break;
                
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_CELL_CHANGE_DAB:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.counter_NbMeasureSwitch_DAB;   
                break;

            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_AF_SEARCH_FM:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_FM;   
                break;
  
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_LOSS_SERVICE_FM:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.counter_NbMeasureLossService_FM;   
                break;
                
            case DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_CELL_CHANGE_FM:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.counter_NbMeasureSwitch_FM;   
                break;
				

            case DABMW_API_SF_AVERAGING_PONDERATION_CURRENT_QUALITY:
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.ponderation_CurrentQuality;   
                break;
                
            case DABMW_API_SF_AVERAGING_PONDERATION_NEW_QUALITY:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.ponderation_NewQuality;   
                break;
                
            case DABMW_API_SF_PERIODICITY_AF_SEARCH:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.AFsearchPeriodicity / DABMW_API_SF_TIME_UNIT;   
                break;
                
            case DABMW_API_SF_PERIODICITY_FULL_SCAN_AF_SEARCH:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.FullScanAFsearchPeriodicity / DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_SERVICE_RECOVERY_SEARCH:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.ServiceRecoverySearchPeriodicity / DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_LANDSCAPE_BUILDING:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.LandscapeBuildingScanPeriodicity / DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_LANDSCAPE_FOR_SEAMLESS:
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.FMLandscapeDelayEstimationPeriodicity / DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_MEASUREMENTS_DAB_SELECTED:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.measurementPeriodicityDAB / DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PERIODICITY_MEASUREMENTS_FM_SELECTED:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.measurementPeriodicityFM / DABMW_API_SF_TIME_UNIT; 
                break;

            case DABMW_API_SF_LOG_INFO_STATUS_PERIODICITY_TIME:	
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.LogInfoStatusPeriodicity / DABMW_API_SF_TIME_UNIT; 
                break;  

            case DABMW_API_SF_RDS_PI_MAX_WAIT_TIME:  
                // We consider what is set is good, 
                // directly use the value without check
                p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.maxTimeToDecodePi *  DABMW_API_SF_TIME_UNIT; 
                break;
                
            case DABMW_API_SF_PARAM_PI_VALIDITY_TIME:
                     // We consider what is set is good, 
                     // directly use the value without check
                      p_cellParamValue[vl_srcIndex] = DABMW_serviceFollowingData.PI_validityTime / DABMW_API_SF_TIME_UNIT;
                                   
                     break;

#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
           case DABMW_API_SF_SS_CONFIDENCE_LEVEL_THRESHOLD:	
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.ConfidenceLevelThreshold;
                break;
            case DABMW_API_SF_SS_MEAS_VALIDITY_DURATION:	
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.measureValidityDuration / DABMW_API_SF_TIME_UNIT;
                break;
            case DABMW_API_SF_SS_RECONF_PERIODICITY:
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.reconfPeriodicity / DABMW_API_SF_TIME_UNIT;
                break; 
             case DABMW_API_SF_SS_MINIMUM_TIME_BETWEEN_SS:
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.minimumTimeBetween2Seamless / DABMW_API_SF_TIME_UNIT;
                break;
             case DABMW_API_SF_SS_FULL_ESTIMATION_START_WINDOW_IN_MS:
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.FullEstimationStartWindowInMs / DABMW_API_SF_TIME_UNIT;
                break;
                
            case DABMW_API_SF_SS_FULL_ESTIMATION_STOP_WINDOW_IN_MS:
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.FullEstimationStopWindowInMs  / DABMW_API_SF_TIME_UNIT;
                break;
                
            case DABMW_API_SF_SS_RECONF_ESTIMATION_HALF_WINDOW_IN_MS:
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.ReconfEstimationWindowSizeInMs / DABMW_API_SF_TIME_UNIT;
               break;
               
            case DABMW_API_SF_SS_FULL_ESTIMATION_DOWN_SAMPLING:
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.DownSampling;
                break;   
            case DABMW_API_SF_SS_RECONF_ESTIMATION_DOWN_SAMPLING:
                p_cellParamValue[vl_srcIndex] = DABMW_SS_EstimationParameter.DownSamplingReconf;
                break;  
#endif //  CONFIG_APP_SEAMLESS_SWITCHING_BLOCK                
            default:
                    vl_res = OSAL_ERROR;
                    break;  
            }

        if (OSAL_OK == vl_res)
                {
                // increment the len : this is the number of param written
                vl_len++;
                }
                       
       }
#else
    vl_res = OSAL_ERROR;
    vl_len = 0;
#endif // #if defined (SERVICE_FOLLOWING)

    return vl_len;  
}


// Procedure to enable Service Following
tVoid DABWM_ServiceFollowing_Enable()
{

    if ( true == DABMW_serviceFollowingData.enableServiceFollowing )
    {
        // This is already enabled : nothing to do
        //
        return;
    }
    
    DABMW_serviceFollowingData.enableServiceFollowing = true;

    // if fm to fm supported, enable the RDS it is mandatory for SF
   /* this is done later 
   	DABMW_ServiceFollowing_EnableRds(DABMW_MAIN_AMFM_APP, false);
	DABMW_ServiceFollowing_EnableRds(DABMW_BACKGROUND_AMFM_APP, false);
	*/
	
    // Init  data
    //
    // Most of the data should already be init. 
    //
    // either by init, either when 'service following on tune'...
    //
    // What we need to init is the timer part
    //
    // the Service following may be activated/deactivated in a raw for sub function
    // like new tuning... 
    // so do not reset all datas , else it does not run correctly
    //
    DABMW_serviceFollowingStatus.deltaTimeFromLastSwitch = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime (); 
	DABMW_serviceFollowingStatus.lastServiceRecoveryTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	//DABMW_serviceFollowingStatus.lastSearchForAFTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_serviceFollowingStatus.lastSearchForAFTime = DABMW_INVALID_DATA;
    //DABMW_serviceFollowingStatus.lastFullScanForAFTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_serviceFollowingStatus.lastFullScanForAFTime = DABMW_INVALID_DATA;
	DABMW_serviceFollowingStatus.lastSwitchTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	// do not update the landscape building : 
	// it has be updated at init and last time
	// DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();

    DABMW_serviceFollowingStatus.original_badQualityCounter = 0;
	DABMW_serviceFollowingStatus.original_mediumQualityCounter = 0;   

	// Correction EPR : get the Sid & Eid of current freq... 
	// in case it has been received meanwhile...
	// correction of use case : Tune, and then RDS enabled... In that use case, the currentEid & Sid are not uptodate
	//

	if (DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.currentAudioUserApp))
		{
		DABMW_serviceFollowingStatus.currentSid = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPiFromFreq(DABMW_serviceFollowingStatus.currentFrequency);
		}
	else
		{
		// for DAB nothing to do, it is already up to date when service has been selected...
		}
	DABMW_SF_PRINTF(TR_LEVEL_USER_1, "*DABWM_ServiceFollowing_Enable : current path = %d, handle = %d, freq = %d, Service = 0x%x, band = %d\n",
				DABMW_serviceFollowingStatus.currentAudioUserApp,
				DABMW_serviceFollowingStatus.currentHandle,
				DABMW_serviceFollowingStatus.currentFrequency,
				DABMW_serviceFollowingStatus.currentSid,
				DABMW_serviceFollowingStatus.currentSystemBand);

    // Call the procedure to manage the service following restart
    DABMW_ServiceFollowingOnTune (DABMW_serviceFollowingStatus.currentAudioUserApp, 
                                  DABMW_serviceFollowingStatus.currentSystemBand, 
                                  DABMW_serviceFollowingStatus.currentFrequency,
                                  DABMW_serviceFollowingStatus.currentEid,
                                  DABMW_serviceFollowingStatus.currentSid,
                                  false,
                                  // change ETAL
                                  DABMW_serviceFollowingStatus.currentHandle);

#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
	DABMW_ServiceFollowing_CleanSeamlessInfo();
#endif

    return;
    
}

// Procedure to disable Service Following
tVoid DABWM_ServiceFollowing_Disable()
{
    if ( false == DABMW_serviceFollowingData.enableServiceFollowing )
    {
        // This is already disabled : nothing to do
        //
        return;
    }
    
    DABMW_serviceFollowingData.enableServiceFollowing = false;

    // Init  data
    //
	DABMW_ServiceFollowing_StopOnGoingActivities();
                
    // set current to original
    DABMW_serviceFollowingStatus.currentSystemBand =  DABMW_serviceFollowingStatus.originalSystemBand;
    DABMW_serviceFollowingStatus.currentFrequency = DABMW_serviceFollowingStatus.originalFrequency;      
    DABMW_serviceFollowingStatus.currentEid = DABMW_serviceFollowingStatus.originalEid;
    DABMW_serviceFollowingStatus.currentSid= DABMW_serviceFollowingStatus.originalSid; 
    DABMW_serviceFollowingStatus.currentAudioUserApp = DABMW_serviceFollowingStatus.originalAudioUserApp;
    
    // the reset of monitored AF is done above. 
    // DABMW_ServiceFollowing_ResetMonitoredAF(); 
    
    // state part
    
	/* enable service following */
    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_INIT);
    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_INIT);

    return;
    
}

// Procedure to disable Service Following
tVoid DABWM_ServiceFollowing_Suspend()
{
    return;
}

// Procedure to disable Service Following
tVoid DABWM_ServiceFollowing_Resume()
{
    return;
}

// Procedure to enable Service Following
tBool DABWM_ServiceFollowing_IsEnable()
{

    return (DABMW_serviceFollowingData.enableServiceFollowing);
        
}

// Procedure to abort any on-going activities
// 
tVoid DABMW_ServiceFollowing_StopOnGoingActivities()
{

	// Init  data
	//
		
	//
	// If alternate was on going : reset the alternate app to release the ressource
	// 
	switch (DABMW_serviceFollowingStatus.status)
		{
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK					
		case DABMW_SF_STATE_IDLE_SEAMLESS:
			// Stop estimation if on going
			DABMW_ServiceFollowing_SeamlessEstimationStop();
			// We were in idle.
			// 1 resource may be use for alternate : free it
			DABMW_ServiceFollowing_ResetMonitoredAF();
					
					break;
#endif /* CONFIG_APP_SEAMLESS_SWITCHING_BLOCK */
		
		case DABMW_SF_STATE_INITIAL_SERVICE_SELECTION:
		case DABMW_SF_STATE_BACKGROUND_CHECK:
		case DABMW_SF_STATE_BACKGROUND_WAIT_PI:
		case DABMW_SF_STATE_BACKGROUND_WAIT_PS:
		case DABMW_SF_STATE_BACKGROUND_WAIT_DAB:
		case DABMW_SF_STATE_BACKGROUND_SCAN:
		case DABMW_SF_STATE_BACKGROUND_WAIT_FIC:
		case DABMW_SF_STATE_AF_CHECK:
		case DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK:	  
			// We are in background search case : 
			// Terminate the background search
			//
			DABMW_ServiceFollowing_EndBackgroundScan();
		
			// 1 resource may be use for alternate : free it
			DABMW_ServiceFollowing_ResetMonitoredAF();
								
			break;
		default : 
			// We were in idle.
			// 1 resource may be use for alternate : free it
			DABMW_ServiceFollowing_ResetMonitoredAF();
			break;									
		}


}

#ifdef __cplusplus
}
#endif

#endif // #ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // SERVICE_FOLLOWING_C
// End of file

