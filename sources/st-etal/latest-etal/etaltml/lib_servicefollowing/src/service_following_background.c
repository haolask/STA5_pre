//!
//!  \file      service_following_background.c
//!  \brief     <i><b> Service following implementation : background check & scan and AF check </b></i>
//!  \details   This file provides functionalities for service following background check, scan and AF check
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.10.07
//!  \bug       Unknown
//!  \warning   None
//!


/* EPR CHANGE :  add define for Service Following C
*/
#ifndef SERVICE_FOLLOWING_BACKGROUND_C
#define SERVICE_FOLLOWING_BACKGROUND_C
/* END EPR CHANGE */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"



#if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
    #include "rds_data.h"
    #include "rds_landscape.h"
#endif // #if defined (CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)

/* if seamless active */
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "seamless_switching_common.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)


#include "service_following.h"

#include "service_following_internal.h"

#include "service_following_log_status_info.h"

#include "service_following_mainloop.h"

#include "service_following_background.h"

#include "service_following_meas_and_evaluate.h"

#include "service_following_audioswitch.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
* Procedure to init the number background scan procedure : setting the band order to scan.
* returns the number of band to scan
* 
*/

tU8 DABMW_ServiceFollowing_Background_ScanInit(tU32 vI_SearchedServiceID, DABMW_SF_systemBandsTy vI_band1, DABMW_SF_systemBandsTy vI_band2,DABMW_SF_systemBandsTy vI_band3)
{
	
	DABMW_SF_systemBandsTy vl_band;
	DABMW_SF_mwCountryTy vl_country = DABMW_ServiceFollowing_ExtInt_GetCountry();


	/* Init number of band to scan and array
	*/
	DABMW_SF_BackgroundScanInfo.numBearerToScan = 0;
	DABMW_SF_BackgroundScanInfo.currentScanBandIndex = DABMW_INVALID;
	DABMW_SF_BackgroundScanInfo.bg_Freq = DABMW_INVALID_FREQUENCY;
	DABMW_SF_BackgroundScanInfo.succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;

    if (true == DABMW_SF_BackgroundScanInfo.seek_ongoing)
    {
         // end the seek procedure if it was on going.
        DABMW_SF_AutoSeekEnd();
        DABMW_SF_BackgroundScanInfo.seek_ongoing = false;
    }

	DABMW_ServiceFollowing_ExtInt_MemorySet((tPVoid)&(DABMW_SF_BackgroundScanInfo.requestedBandToScan[0]), DABMW_BAND_NONE, sizeof(DABMW_SF_systemBandsTy)*DABMW_SF_BG_MAX_BAND_TO_SCAN);


	/* build array & num of band to scan depending on settting
	*/	
	vl_band = vI_band1;

	if (DABMW_BAND_NONE != vl_band)
		{
		DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.numBearerToScan] = vl_band;
		DABMW_SF_BackgroundScanInfo.numBearerToScan += 1;
		}

	vl_band = vI_band2;

	if (DABMW_BAND_NONE != vl_band)
		{
		DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.numBearerToScan] = vl_band;
		DABMW_SF_BackgroundScanInfo.numBearerToScan += 1;
		}

	vl_band = vI_band3;

	if (DABMW_BAND_NONE != vl_band)
		{
		DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.numBearerToScan] = vl_band;
		DABMW_SF_BackgroundScanInfo.numBearerToScan += 1;
		}


	/* If something is found :)
	* then initialize with the 1st to start
	*/
	
	if (DABMW_SF_BackgroundScanInfo.numBearerToScan > 0)
		{
		/* set the 1st band to scan Index
		*/
		DABMW_SF_BackgroundScanInfo.currentScanBandIndex = 0;
		vl_band = DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.currentScanBandIndex];

		/* init the frequency range
		*/
		DABMW_SF_BackgroundScanInfo.startFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin(vl_band, vl_country);
		DABMW_SF_BackgroundScanInfo.stopFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmax(vl_band, vl_country);
		DABMW_SF_BackgroundScanInfo.bandStep = DABMW_ServiceFollowing_GetBandInfoTable_step(vl_band, vl_country);
		/* Select the best suited app */
		// this will be init in the "next scan" or "next check"
		// DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_ServiceFollowing_Background_SelectApp(vl_band);

		}
    
    /* Store the targetted Service ID or PI whatever
         */
    DABMW_SF_BackgroundScanInfo.searchedServiceID = vI_SearchedServiceID;

    // store the VPA status at start.
    DABMW_SF_BackgroundScanInfo.VPA_IsOnAtStart = DABMW_ServiceFollowing_ExtInt_IsVpaEnabled();
    DABMW_SF_BackgroundScanInfo.VPA_HasBeenBroken = false;
    
	return DABMW_SF_BackgroundScanInfo.numBearerToScan;
}

/*
* Procedure to continue  the background scan procedure to next frequency within the set bands: setting the band order to scan.
* returns the next band to scan information, if NONE = no more band requested in background scan.
* 
*/


tU32 DABMW_ServiceFollowing_Background_NextScan()
{
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_NONE;
	DABMW_SF_mwCountryTy vl_country = DABMW_ServiceFollowing_ExtInt_GetCountry();
	tSInt vl_tmp_res;
	tU32 vl_currentFreq = DABMW_INVALID_FREQUENCY;
	tBool vl_tunerAssociationNeed = false;

	/* Retrieve where we are, and which frequency to switch 
	*/
	/* Valid configuration ?
	*/

	vl_band = DABMW_ServiceFollowing_Background_GetCurrentScanBand();
	
	if (DABMW_BAND_NONE == vl_band)
		{
		/* what are we doing here ? nothing configured !!
		*/
        DABMW_SF_BackgroundScanInfo.bg_Freq = vl_currentFreq;

		return DABMW_INVALID_FREQUENCY;
		}
                    
	/* check current Freq.
	* Invalid = start
	* Other = switch to next freq or band..
	*/
	vl_currentFreq = DABMW_SF_BackgroundScanInfo.bg_Freq;
	
	if (DABMW_INVALID_FREQUENCY == vl_currentFreq)
		{
         
		/* this means we are at a start of band (and even start of processing)
		* The specific init/ configuratiion has been done in scan Init.
		*/
		vl_tunerAssociationNeed = true;

		/* set the start freq, stop freq and band step... just in case 
		* it should be already ok from bg scan init 
		*/
		DABMW_SF_BackgroundScanInfo.startFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin(vl_band, vl_country);
		DABMW_SF_BackgroundScanInfo.stopFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmax(vl_band, vl_country);
		DABMW_SF_BackgroundScanInfo.bandStep = DABMW_ServiceFollowing_GetBandInfoTable_step(vl_band, vl_country);
		
		vl_currentFreq = DABMW_SF_BackgroundScanInfo.startFreq;

		}
	else
		{
		/* move to next frequency */
		/* get next frequency from function and table
		*/

		/*
		* vl_currentFreq += DABMW_SF_BackgroundScanInfo.bandStep;

			if ((true == DABMW_SF_BackgroundScanInfo.backgroundScanSkipOriginalFreq)
				&&
				(vl_currentFreq == DABMW_serviceFollowingStatus.originalFrequency)
				)
				{
				vl_currentFreq += DABMW_SF_BackgroundScanInfo.bandStep;
				}

				if ((vl_currentFreq > DABMW_SF_BackgroundScanInfo.stopFreq) 
			*/

		/* Get frequency */
		vl_currentFreq = DABMW_ServiceFollowing_ExtInt_GetNextFrequencyFromFreq (DABMW_SF_BackgroundScanInfo.bg_Freq, vl_band, true);

        // if we need to skip the original
        //
		if ((true == DABMW_SF_BackgroundScanInfo.backgroundScanSkipOriginalFreq)
			&&
			(vl_currentFreq == DABMW_serviceFollowingStatus.originalFrequency)
			)
			{
			vl_currentFreq = DABMW_ServiceFollowing_ExtInt_GetNextFrequencyFromFreq (vl_currentFreq, vl_band, true);
			}

        // we need to skip the alternate
        if (vl_currentFreq == DABMW_serviceFollowingStatus.alternateFrequency)
			{
			vl_currentFreq = DABMW_ServiceFollowing_ExtInt_GetNextFrequencyFromFreq (vl_currentFreq, vl_band, true);
			}

		/* Are we still in the range ? 
		* if out of range => try next band or end
		* if result of NextFreq  is less that the bg_Freq, it means the table has been completed (since a loop done )
		*/
		if ((vl_currentFreq < DABMW_SF_BackgroundScanInfo.bg_Freq) || (DABMW_INVALID_FREQUENCY == vl_currentFreq))
			{
			/* Out of band so think to a next band
			*/

            // If the current band is FM, we should close it correctly
            // ie end the seek procedure 
            if (DABMW_ServiceFollowing_ExtInt_IsFMBand(vl_band))
                {
                // end the seek procedure if it was on going.
                DABMW_SF_AutoSeekEnd();
                }
            
			/* If current index is below number of band : some bands remain to scan
			* note : index is from 0 to max 2
			* num band is 0 if none (then band index should be invalid, else from 1 to 3.
			*/
			vl_band = DABMW_ServiceFollowing_Background_GetNextScanBand();

            if (DABMW_BAND_NONE != vl_band)
                {
	    		DABMW_SF_BackgroundScanInfo.startFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin(vl_band, vl_country);
				DABMW_SF_BackgroundScanInfo.stopFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmax(vl_band, vl_country);
				DABMW_SF_BackgroundScanInfo.bandStep = DABMW_ServiceFollowing_GetBandInfoTable_step(vl_band, vl_country);

				vl_tunerAssociationNeed = true;
				
				/* indicate this is a start to new band/range..
				*/
				vl_currentFreq = DABMW_SF_BackgroundScanInfo.startFreq;
							
				}
			else
				{
				vl_currentFreq = DABMW_INVALID_FREQUENCY;
				}
			}
		}

	if (true == vl_tunerAssociationNeed)
		{
		/* associate the tuner
		*/
		/* we need to release the tuner in case.. */
		/* Reset/Release the prior used tuner to free resources */
		DABMW_ServiceFollowing_ResetTuner(DABMW_SF_BackgroundScanInfo.bgScan_App);

		/* Select the new best suited app */
		DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_ServiceFollowing_Background_SelectApp(vl_band);

        // Print to indicate we move scan to a new band 
        //
        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
             DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
       /* END TMP LOG */

        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
       DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Background scan) : BAND CHANGE, app = %d, BAND = %s)\n", 
                                DABMW_SF_BackgroundScanInfo.bgScan_App,
                                DABMW_ServiceFollowing_GetBandName(vl_band));      
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
       /* END TMP LOG */

       /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
       DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
      /* END TMP LOG */


		if (DABMW_NONE_APP!= DABMW_SF_BackgroundScanInfo.bgScan_App)
			{
			vl_tmp_res = OSAL_OK;
		
			/* Tuner Association here is not needed : 
			* done by TuneFrequency... 
			*
			* vl_tmp_res  = DABMW_ConfigureTunerForSpecificApp (DABMW_SF_BackgroundScanInfo.bgScan_App, vl_band, DABMW_SF_BackgroundScanInfo.startFreq);;
			*/
			}
		else 
			{
			vl_tmp_res = OSAL_ERROR;
			}

		if (OSAL_ERROR == vl_tmp_res)
			{
			DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW Error: DABMW_ServiceFollowing_Background_NextScanInit failed, ScanApp = %d, Band = %d, startFreq = %d\n",
												DABMW_SF_BackgroundScanInfo.bgScan_App, vl_band, DABMW_SF_BackgroundScanInfo.startFreq);

//			DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.currentScanBandIndex] = DABMW_BAND_NONE;
			vl_currentFreq = DABMW_INVALID_FREQUENCY;
			}
		else 
			{
			/* If the APP / band is FM : enable RDS 
			*/
			if (DABMW_ServiceFollowing_ExtInt_IsFMBand(vl_band))
				{
	
                // We may need to stop the VPA during the bg search
                //
                DABMW_ServiceFollowing_AmFMStopVpa(DABMW_SF_BackgroundScanInfo.bgScan_App);

              
	            // RDS should already be enable but let's check in the procedure
	            // if Landscape is on going, then this is a 'normal' PI request
	            if (false == DABMW_ServiceFollowing_IsLanscapeScanOnGoing())
	            	{
	            	DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, true);
	            	}
				else
	            	{
	            	DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, false);
	            	}
			
				/*	tuned on starting freq , and explicit tune is needed.
				* ~1st init for tuner to be ready.
				*/
				vl_tmp_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency(DABMW_SF_BackgroundScanInfo.bgScan_App, 
												DABMW_SF_BackgroundScanInfo.startFreq);


				/* check tune ok */
				if (OSAL_OK == vl_tmp_res)
					{
					// Set seek data, direction up
					DABMW_ServiceFollowing_ExtInt_SetAutoSeekData (DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.startFreq, DABMW_SF_BackgroundScanInfo.stopFreq, true, 
									DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin(vl_band, vl_country), DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmax(vl_band, vl_country));
					}
				else
					{
					/* tune failed */
					vl_currentFreq = DABMW_INVALID_FREQUENCY;
					}

				// configure autoseek threshold
				
				DABWM_ServiceFollowing_ExtInt_SetAmFmAutoSeekThreshold(DABMW_SF_BackgroundScanInfo.bgScan_App, 
												DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV,
												DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.adjacentChannel,
												DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.detuning,
												DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.multipath,
												DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ,
												DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.snr,
												DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.mpxNoise);

				}
			}
						
		}

	if (DABMW_INVALID_FREQUENCY == vl_currentFreq)
		{
		/* no frequency found */
		/* we need to release the tuner in case.. */
		/* Reset/Release the prior used tuner to free resources */
		DABMW_ServiceFollowing_ResetTuner(DABMW_SF_BackgroundScanInfo.bgScan_App);
		}
		
	DABMW_SF_BackgroundScanInfo.bg_Freq = vl_currentFreq;

	return vl_currentFreq;
}

/* Procedure to retrieve current scan band
*/

DABMW_SF_systemBandsTy DABMW_ServiceFollowing_Background_GetCurrentScanBand()
{
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_NONE ;

    // some basic checks 
    // if error on band index => return
    //
    if (DABMW_INVALID == DABMW_SF_BackgroundScanInfo.currentScanBandIndex) 
        {
        return DABMW_BAND_NONE;
        }


        // add for mobility simulate
        // check the band & the lock feature
        // if band => and lock on that band, then go to next band
        //
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    
    // if feature is locked on 2 frequencies...
    // or if band is DAB and feature is lock on DAB => go to next band
    // if band is FM and feature is lock to FM => go to next band
    
    if (false == DABMW_serviceFollowingData.lockFeatureActivated_2Freq)
        {
        // we are not locked on 2 freq
        // scan may be needed : find the band depending on what is locked.
        //
        do
            {
            vl_band = DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.currentScanBandIndex];
                
            if (((DABMW_ServiceFollowing_ExtInt_IsDABBand(vl_band)) && (true == DABMW_serviceFollowingData.lockFeatureActivated_DAB))
             || ((DABMW_ServiceFollowing_ExtInt_IsFMBand(vl_band)) && (true == DABMW_serviceFollowingData.lockFeatureActivated_FM)))
                {
                // band not ok    
                vl_band = DABMW_BAND_NONE;    
                DABMW_SF_BackgroundScanInfo.currentScanBandIndex++;

                // reset the current freq to mark start new band
                DABMW_SF_BackgroundScanInfo.bg_Freq = DABMW_INVALID_FREQUENCY;
                }
                    
             } while ((DABMW_SF_BackgroundScanInfo.currentScanBandIndex < DABMW_SF_BackgroundScanInfo.numBearerToScan)
                        && (DABMW_BAND_NONE == vl_band));
        }
        else
        {
            // we are locked on 2 freq => end of bg scan :)
            vl_band = DABMW_BAND_NONE;       
        }
                                   
#else //  CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* If current index is below number of band : some bands remain to scan
	* note : index is from 0 to max 2
	* num band is 0 if none (then band index should be invalid, else from 1 to 3.
	*/
	vl_band = DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.currentScanBandIndex];

#endif

	return vl_band;
}

/* Procedure to retrieve next band to scan
*/

DABMW_SF_systemBandsTy DABMW_ServiceFollowing_Background_GetNextScanBand()
{
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_NONE ;


    // some basic checks 
    // if error on band index => return
    //
    if (DABMW_INVALID == DABMW_SF_BackgroundScanInfo.currentScanBandIndex) 
        {
        return DABMW_BAND_NONE;
        }
   
    // add for mobility simulate
    // check the band & the lock feature
    // if band => and lock on that band, then go to next band
    //
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    
    // if feature is locked on 2 frequencies...
    // or if band is DAB and feature is lock on DAB => go to next band
    // if band is FM and feature is lock to FM => go to next band
    
    if (false == DABMW_serviceFollowingData.lockFeatureActivated_2Freq)
        {
        // we are not locked on 2 freq
        // scan may be needed : find the band depending on what is locked.
        //
       while (((DABMW_SF_BackgroundScanInfo.currentScanBandIndex + 1)< DABMW_SF_BackgroundScanInfo.numBearerToScan)
             && (DABMW_BAND_NONE == vl_band))
            {
                
            DABMW_SF_BackgroundScanInfo.currentScanBandIndex++;
            
            vl_band = DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.currentScanBandIndex];
                
            if (((DABMW_ServiceFollowing_ExtInt_IsDABBand(vl_band)) && (true == DABMW_serviceFollowingData.lockFeatureActivated_DAB))
             || ((DABMW_ServiceFollowing_ExtInt_IsFMBand(vl_band)) && (true == DABMW_serviceFollowingData.lockFeatureActivated_FM)))
                {
                // band not ok    
                vl_band = DABMW_BAND_NONE;    
                }
            }
        }
    else
        {
            // we are locked on 2 freq => end of bg scan :)
            vl_band = DABMW_BAND_NONE;       
        }
                                
#else //  CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG

    if ((DABMW_SF_BackgroundScanInfo.currentScanBandIndex + 1) < DABMW_SF_BackgroundScanInfo.numBearerToScan )
		{
		DABMW_SF_BackgroundScanInfo.currentScanBandIndex++;
				
		vl_band = DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.currentScanBandIndex];	
        }
    
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG

	return vl_band;
}


/* Return Current selected freq for BG scan
*/
tU32 DABMW_ServiceFollowing_Background_GetCurrentScanFreq()
{
	tU32 vl_Frequency = DABMW_INVALID_FREQUENCY;

	/* If current index is below number of band : some bands remain to scan
	* note : index is from 0 to max 2
	* num band is 0 if none (then band index should be invalid, else from 1 to 3.
	*/
	if (DABMW_INVALID != DABMW_SF_BackgroundScanInfo.currentScanBandIndex) 
		{
		vl_Frequency = DABMW_SF_BackgroundScanInfo.bg_Freq;
		}

	return (vl_Frequency);
}


/* Procedure to select the right app & tuner to to the procedure depending on current band/techno, and free tuner
*/

DABMW_SF_mwAppTy DABMW_ServiceFollowing_Background_SelectApp(DABMW_SF_systemBandsTy vI_band)
{

	DABMW_SF_mwAppTy vl_app = DABMW_SF_BackgroundScanInfo.bgScan_App;
		
	// TODO: CONFIGURE THE BEST SUITED APP depending on 

		// DABMW_ConfigureTunerForSpecificApp ?
		// other in tuner control.c ?

	/* for Inital Service Selection : use the one in parameter
	* for other cases : get one.
	*/ 

	if (DABMW_SF_STATE_INITIAL_SERVICE_SELECTION != DABMW_serviceFollowingStatus.prevStatus)
		{
		/* select the app */
		if (DABMW_ServiceFollowing_ExtInt_IsFMBand(vI_band))
			{
			/* we should select an FM app */
			/* For FM, we could consider to always use background even if main not use
			* for the moment : Main if not use
			* else use background
			*/
			if (((DABMW_MAIN_AMFM_APP != DABMW_serviceFollowingStatus.originalAudioUserApp))
                && (DABMW_MAIN_AMFM_APP != DABMW_serviceFollowingStatus.alternateApp))
				{
				vl_app = DABMW_MAIN_AMFM_APP;
				}
			else
				{
				vl_app = DABMW_BACKGROUND_AMFM_APP;
				}
			}
		else if (DABMW_ServiceFollowing_ExtInt_IsDABBand(vI_band))
			{
			/* we should select a DAB app */
			/* service recovery : so use main dab if free */
			if (DABMW_MAIN_AUDIO_DAB_APP == DABMW_serviceFollowingStatus.originalAudioUserApp)
				{
				/* audio is on main DAB ... 
				* then select secondary DAB 
				*/
				vl_app = DABMW_SECONDARY_AUDIO_DAB_APP;
				}
			else if (DABMW_SECONDARY_AUDIO_DAB_APP == DABMW_serviceFollowingStatus.originalAudioUserApp)
				{
				/* audio is on second DAB ... 
				* then select Main DAB
				*/
				vl_app = DABMW_MAIN_AUDIO_DAB_APP;
				}
			else
				{
				/* audio is not on DAB */
				/* select the app depending on alternate */
				if (DABMW_MAIN_AUDIO_DAB_APP == DABMW_serviceFollowingStatus.alternateApp)
					{
					/* alternate set on main DAB... 
					* so search on the 2nd DAB to keep alternate tuned 
					*/
					vl_app = DABMW_SECONDARY_AUDIO_DAB_APP;
					}
				else if (DABMW_SECONDARY_AUDIO_DAB_APP == DABMW_serviceFollowingStatus.alternateApp)
					{
					/* alternate set on 2nd DAB... 
					* so search on the main DAB to keep alternate tuned 
					*/
					vl_app = DABMW_MAIN_AUDIO_DAB_APP;
					}
				else
					{
					/* alternate is not DAB :select main one*/
					vl_app = DABMW_MAIN_AUDIO_DAB_APP;
					}
				}

                // clear flags around app : to retune it if needed (resume...) after the bg procedure
                // because if the app is used by the bg, then it will need to be retuned.
                //
                if (vl_app == DABMW_serviceFollowingStatus.alternateApp)
                    {
                        DABMW_serviceFollowingStatus.alternateTuned = false;
                        DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;
                    }
			}
		else
			{
			/* error case : not DAB neither FM.. */
			vl_app = DABMW_NONE_APP;
			}
		
		}

	/* for initial selection : we suppose we use the app requested...
	*/

	// add-on ETAL 
	
	if (DABMW_NONE_APP != vl_app)
	{

		// need to set the bg scan app 
		// if current bgScan App != previous => reset the handle
		// else we might reuse
		
 		// reset the handle for now
 		if (DABMW_SF_BackgroundScanInfo.bgScan_App != vl_app)
		{
			DABMW_SF_BackgroundScanInfo.bgScan_Handle = ETAL_INVALID_HANDLE;
			DABMW_SF_BackgroundScanInfo.bgScan_HandleDatapth = ETAL_INVALID_HANDLE;
 		}
		
		DABMW_SF_BackgroundScanInfo.bgScan_App = vl_app;

		if (OSAL_OK != DABMW_ServiceFollowing_ExtInt_AllocateEtalHandleFromApp(vl_app))
		{
			// error
			vl_app = DABMW_NONE_APP;
		}
	}
	
	// end ADD ON  ETAL
	
	return(vl_app);
	
}


/* Procedure to select the right app & tuner to to the procedure depending on current band/techno, and free tuner
*/

DABMW_SF_mwAppTy DABMW_ServiceFollowing_AFCheck_SelectApp(DABMW_SF_systemBandsTy vI_band)
{

	DABMW_SF_mwAppTy vl_app = DABMW_NONE_APP;
    tBool vl_originalIsFM = DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.originalAudioUserApp);
    tBool vl_alternateIsFM = DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp);

	// for AF Check , we select the main_app always
	// the choice to use AF check or not has been done before 
	// we consider here the app is ok.
	// cases are : 
	// 
	// either original APP to be used if both original & 
	//

    if (false == DABMW_ServiceFollowing_ExtInt_IsFMBand(vI_band))
        {
        /* AF CHECK only in FM
		*/
		vl_app = DABMW_NONE_APP;
        }
    // below are the cases 
    // original = DAB no alternate             ==> we consider we have a free tuner, no AF Check
    // original = DAB and alternate = DAB ==> not ok for AF check, do background
    // original = DAB and alternate = FM   ==> either we do on alternate (which is MAIN), either we consider we have a free tuner so no AF check
    //      ==> consider no AF check.
    // orignial = FM and no alternate         ==> either we do on original (which is MAIN) , either we consider we have a free tuner so no AF check
    // original = FM and alternate = DAB   ==> either we do on original (which is MAIN) , either we consider we have a free tuner so no AF check
    // original = FM and alternate = FM      ==> either we do on original (which is MAIN), either we do it on alternate tuner if tuned, either we consider we have a free tuner so no AF check
    //    ==> consider no AF check for now.
    // VPA on  
    else if (true == DABMW_SF_BackgroundScanInfo.VPA_IsOnAtStart)
        {
        // VPA is on, so the MAIN_APP should be used for AF check    
        vl_app = DABMW_MAIN_AMFM_APP;
        }
    else if ((false == vl_alternateIsFM) && (false == vl_originalIsFM))
        {
            /* AF CHECK only in FM  
		        */
		    vl_app = DABMW_NONE_APP;
        }
    else
        {
            //  MAIN_APP should be used for AF check 
            vl_app = DABMW_MAIN_AMFM_APP;
        }
				
	return(vl_app);
	
}


/* Procedure handling the main algorithm for background check/scan
*
* basic steps are :
* 
* Step 0 : init => done by caller at start of procedure.
*	-- init the frequency list of freq to be parsed for AF check & background check
*     -- init the band and range to be scan for background scan
*     -- init the targetted SID if needed.
*
* Step 1 : Tune & Seek on next frequency
*     -- this is done by DABMW_ServiceFollowing_BackgroundScan()
*     -- when ok, normal state is 
*			DABMW_serviceFollowingStatus.status = DABMW_SF_STATE_BACKGROUND_WAIT_PI or DABMW_SF_STATE_BACKGROUND_WAIT_DAB
*			DABMW_serviceFollowingStatus.nextStatus = DABMW_SF_STATE_BACKGROUND_SCAN;
*     -- if nothing more to scan
*			DABMW_serviceFollowingStatus.status = prevStatus
*			DABMW_serviceFollowingStatus.prevStatus = DABMW_SF_STATE_BACKGROUND_SCAN;
*
* Step 2 : Wait for TUNE (DAB case) or PI (FM case)
*     -- states are : 			
*			DABMW_serviceFollowingStatus.status = DABMW_SF_STATE_BACKGROUND_WAIT_PI or DABMW_SF_STATE_BACKGROUND_WAIT_DAB
*			DABMW_serviceFollowingStatus.nextStatus = DABMW_SF_STATE_BACKGROUND_SCAN;
*     -- In that state, timinig should be checked, if > max waiting time, then process.
*
* Step 3 : 
*     -- Compare PI/Service ID with searched one.
*     -- IF OK => Stop procedure ie mark the end of the scan
*              DABMW_serviceFollowingStatus.status = prevStatus
*		  DABMW_serviceFollowingStatus.prevStatus = DABMW_SF_STATE_BACKGROUND_SCAN;
*     -- if NOK : continue the scan.
*     -- DABMW_serviceFollowingStatus.Status = DABMW_SF_STATE_BACKGROUND_SCAN;
*     -- DABMW_serviceFollowingStatus.nextStatus = DABMW_SF_STATE_BACKGROUND_SCAN;
*
* Step 4 : 
*     -- if NOK => loop to Step 1.
*
*/
tSInt DABMW_ServiceFollowing_BackgroundMainLoop()
{

	tSInt vl_tmp_res;
	
	switch (DABMW_serviceFollowingStatus.status)
		{
		case DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK:
			/* step 2 above.
			* check if time has elapsed
			* if so process
			* else wait
			*/

			if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AutoSeek))
				{
				DABMW_ServiceFollowing_Seek_processing(false);
				}
			else
				{
				/* nothing to do */
				}
			break;


		case DABMW_SF_STATE_BACKGROUND_WAIT_PI:
			/* step 2 above.
			* check if time has elapsed
			* if so process
			* else wait
			*/
			if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FM_PI))
				{
				DABMW_ServiceFollowing_PI_processing(false);
				}
			else
				{
				/* nothing to do */
				}
			break;
		// add on for PS NAME awaiting
		// a landscape building function
		//
		case DABMW_SF_STATE_BACKGROUND_WAIT_PS:
			/* step 2 above.
			* check if time has elapsed
			* if so process
			* else wait
			*/
			if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FM_PS))
				{
				DABMW_ServiceFollowing_PS_processing(false);
				}
			else
				{
				/* nothing to do */
				}
			break;
		case DABMW_SF_STATE_BACKGROUND_WAIT_DAB:
			/* step 2 above.
			* check if time has elapsed
			* if so process
			* else wait
			*/
			if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_DAB_TUNE))
				{
				DABMW_ServiceFollowing_DAB_processing(false);
				}
			else
				{
				/* nothing to do */
				}
			break;
		case DABMW_SF_STATE_BACKGROUND_WAIT_FIC:
				/* step 2 above.
				* check if time has elapsed
				* if so process
				* else wait
				*/
				if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_DAB_FIG))
					{
					DABMW_ServiceFollowing_FIC_ReadyProcessing(false);
					}
				else
					{
					/* nothing to do */
					}
				break;

		case DABMW_SF_STATE_AF_CHECK:
			/* That means we should continue the check
			*/
			DABMW_ServiceFollowing_AFCheck();
			break;

		case DABMW_SF_STATE_BACKGROUND_CHECK:
			/* That means we should continue the check
			*/
			DABMW_ServiceFollowing_BackgroundCheck();
			break;

		case DABMW_SF_STATE_BACKGROUND_SCAN:
			/* That means we should continue the scan
			*/
			DABMW_ServiceFollowing_BackgroundScan();
			break;

		case DABMW_SF_STATE_INITIAL_SERVICE_SELECTION:
			/* It means we should be at the end of intial service selection
			*/

			/* check */
			if ((DABMW_SF_STATE_BACKGROUND_SCAN == DABMW_serviceFollowingStatus.prevStatus)
				||
				(DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.prevStatus)
				||
				(DABMW_SF_STATE_AF_CHECK == DABMW_serviceFollowingStatus.prevStatus)
				)
				{
				/* End of initial service selection */
				/* post result thru call back to dab_device
				* we can check the successfull searched status
				* however, we should also know by the currentFreq setting : if invalid => not ok
				*/

                DABMW_ServiceFollowing_EndBackgroundScan();
				
				if (DABMW_SF_BG_RESULT_NOT_FOUND == DABMW_SF_BackgroundScanInfo.succesfulSearched)
					{
					/* enter in service recovery... */                    
                    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_SERVICE_RECOVERY);
                    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_SERVICE_RECOVERY);
                    
					/* init the   values
					*
					* no need to 'enter idle mode'
					* this should be in Service Recovery
					* and all should be reset already when selection started
					*
					DABMW_ServiceFollowing_EnterIdleMode(DABMW_SF_BackgroundScanInfo.bgScan_App,
										DABMW_BAND_NONE,
										DABMW_SF_BackgroundScanInfo.searchedServiceID,
										DABMW_INVALID_EID,
										DABMW_INVALID_FREQUENCY);
                                    */
                                    
					/* unset the current tuned freq on requested app... */

					if (DABMW_NONE_APP != DABMW_SF_BackgroundScanInfo.bgScan_App)
						{
                        DABMW_ServiceFollowing_ResetTuner(DABMW_SF_BackgroundScanInfo.bgScan_App);

						}
					
					DABMW_ServiceFollowing_ExtInt_TuneServiceIdCallback(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_INVALID_FREQUENCY, false);
					
					}
				else
					{
					/*We are tune on right freq.
					* Set the service and audio choice 
					*/
					/* If it was an AF check => let's tune through to new freq
					*/

					if (DABMW_SF_STATE_AF_CHECK == DABMW_serviceFollowingStatus.prevStatus)
						{
						/* setting	*/
						DABMW_serviceFollowingStatus.currentAudioUserApp = (DABMW_SF_mwAppTy) DABMW_SF_BackgroundScanInfo.bgScan_App ;
						DABMW_serviceFollowingStatus.currentFrequency = DABMW_SF_BackgroundScanInfo.bg_Freq; 
						DABMW_serviceFollowingStatus.currentSystemBand =  DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand (DABMW_SF_BackgroundScanInfo.bg_Freq);
						DABMW_serviceFollowingStatus.currentSid = DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].piValue;


						DABMW_ServiceFollowing_ExtInt_AmFmAFSwitchRequest(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq);

						
						}
                    else 
                        {
                        vl_tmp_res = DABMW_ServiceFollowing_SetCurrentCellAsOriginal();

                        // we  may need to set the audio output ???
                        // Select the DAB service 
                		//
                		// Correction : error on setting the 'internal tune' to True
                		//
                		DABMW_SF_SetCurrentAudioPortUser(DABMW_SF_BackgroundScanInfo.bgScan_App, true);
                        // or 
                        // DABMW_SF_SetCurrentAudioPortUser(DABMW_MAIN_AUDIO_DAB_APP, true);

                        if (OSAL_ERROR == vl_tmp_res)
                            {
                            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)  
                            DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (BG_MainLoop) : ERROR => entering idle cell\n");
#endif 
//  defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
              /* END TMP LOG */ 
                            }
                    }
                    
                    DABMW_ServiceFollowing_ExtInt_TuneServiceIdCallback(DABMW_SF_BackgroundScanInfo.bgScan_App, 
												DABMW_serviceFollowingStatus.currentFrequency,
												(DABMW_SF_BG_RESULT_FOUND_PI_CONFIRMED == DABMW_SF_BackgroundScanInfo.succesfulSearched));

					/* back to idle */
                    
                    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
                    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_IDLE);
                    }
	

			}
			else
				{
				/* why are we here ?
				*/
				}
	
			break;
			
		default:
			/* should not come here => this is error case !!
			*/
			break;
		}

	return OSAL_OK;
}


/* Procedure to continue the background scan, in charge of tuning to next frequency 
* 
*/

tSInt DABMW_ServiceFollowing_BackgroundScan()
{

	tSInt vl_tmp_res;
	tU32  vl_currentFreq = DABMW_INVALID_FREQUENCY;
	
	/* Retrieve where we are, and which frequency to switch 
	*/

	vl_currentFreq = DABMW_ServiceFollowing_Background_NextScan();
	
	if (DABMW_INVALID_FREQUENCY == vl_currentFreq)
		{
		/* this means we have finished the process of full band scanning
		*/
        DABMW_ServiceFollowing_EndBackgroundScan();

        DABMW_SF_BackgroundScanInfo.backgroundScanDone = true;
        
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
		DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
        
		/* this is stop of procedure reset data */

		DABMW_serviceFollowingStatus.currentAudioUserApp = (DABMW_SF_mwAppTy) DABMW_SF_BackgroundScanInfo.bgScan_App ;
		DABMW_serviceFollowingStatus.currentFrequency = DABMW_INVALID_FREQUENCY;	
		DABMW_serviceFollowingStatus.currentSystemBand = DABMW_BAND_NONE;
		DABMW_serviceFollowingStatus.currentEid = DABMW_INVALID_ENSEMBLE_ID;
		DABMW_serviceFollowingStatus.currentSid = DABMW_INVALID_SERVICE_ID;
		DABMW_serviceFollowingStatus.dab_syncStatus = DABMW_INVALID_DATA_BYTE;
        DABMW_serviceFollowingStatus.current_Quality = DABMW_ServiceFollowing_QualityInit();
        DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath = DABMW_ServiceFollowing_QualityInit();

		
		/* back to main loop for decision what's next 
		*/
		vl_tmp_res = OSAL_ERROR;
		return OSAL_ERROR;

		}
	else
		{
		/* a new freq is there => tune
		*/
		}

	/* if we come here, then the scan need to continue
	*/
	DABMW_serviceFollowingStatus.currentAudioUserApp = (DABMW_SF_mwAppTy) DABMW_SF_BackgroundScanInfo.bgScan_App ;
	DABMW_serviceFollowingStatus.currentFrequency = vl_currentFreq;	
	DABMW_serviceFollowingStatus.currentSystemBand = DABMW_SF_BackgroundScanInfo.requestedBandToScan[DABMW_SF_BackgroundScanInfo.currentScanBandIndex];
	DABMW_serviceFollowingStatus.currentEid = DABMW_INVALID_ENSEMBLE_ID;
	DABMW_serviceFollowingStatus.currentSid = DABMW_INVALID_SERVICE_ID;
	DABMW_serviceFollowingStatus.dab_syncStatus = DABMW_INVALID_DATA_BYTE;
    DABMW_serviceFollowingStatus.current_Quality = DABMW_ServiceFollowing_QualityInit();
    DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath = DABMW_ServiceFollowing_QualityInit();

	// add on etal : get the handle which may have been allocated
	DABMW_serviceFollowingStatus.currentHandle = DABMW_SF_BackgroundScanInfo.bgScan_Handle;
    

	if (DABMW_ServiceFollowing_ExtInt_IsFMBand(DABMW_serviceFollowingStatus.currentSystemBand))
		{
		/* FM is being tuned 
		*/
		DABMW_ServiceFollowing_ExtInt_TaskClearEvent(DABMW_SF_EVENT_PI_RECEIVED);

		/* in FM  auto-seek ith immediate return*/
		// suspend the RDS block during the seek
		// seek ended : resume the Rds which should have been suspended
		DABMW_ServiceFollowing_SuspendRds(DABMW_SF_BackgroundScanInfo.bgScan_App);
		
		/* Auto seek command */

		DABMW_SF_AmFmLearnFrequencyAutoSeekImmediate(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bandStep);

		/* Now let's wait for the auto seek return 
		* meanwhile go WAIT_AUTO_SEEK state 
		*/
		DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK);
		DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
	
		}
	else
		{
		/* We are in DAB
		*/

		/* clear event in case */
		DABMW_ServiceFollowing_ExtInt_TaskClearEvent(DABMW_SF_EVENT_DAB_TUNE);

		/* tune and wait */	
		/* tSInt DABMW_TuneFrequency (DABMW_SF_mwAppTy app, tU32 frequency, tBool keepDecoding,
		*							  tU8 injectionSide, tBool noOutputSwitch, tBool isInternalTune)
		*/

		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency(DABMW_SF_BackgroundScanInfo.bgScan_App, 
										vl_currentFreq);


        DABMW_SF_BackgroundScanInfo.fig00_read = false;
        DABMW_SF_BackgroundScanInfo.fig01_read = false;
        DABMW_SF_BackgroundScanInfo.fig02_read = false;
		DABMW_SF_BackgroundScanInfo.fig02_readCounter = 0;
		

		if (OSAL_ERROR == vl_tmp_res)
			{
			/* it means either that no frequency is found by seek or frequency found is bad quality.
			*/
			/*
			* if no frequency : this mark the end of this band scan
			* proposition is that the end of backround scan is decided next iteration in any case
			* so that task is rescheduled meanwhile....
			*
			*/
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
    		DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
			}
		else
			{
			DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_DAB);
    		DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
			}
		
		}
					
					
		/* Tune ok : now just wait for tune, Sid and PI
		*/

	return OSAL_OK;

}


/* Procedure to act while on PI awaiting state
* wake-up status in boolean parameter
* TRUE = PI received (event awake..)
* FALSE = Timeout !
*/
tSInt DABMW_ServiceFollowing_PI_processing(tBool vI_PIreceptionStatus)
{
	
	DABMW_SF_BG_SearchedResultTy vl_succesfulSearched;

         /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
            /* END TMP LOG */

#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : app = %d, Freq = %d, Quality %d (dBuV)\n", DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq, DABMW_serviceFollowingStatus.current_Quality.fmQuality.fieldStrength_dBuV);      
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                /* END TMP LOG */

 //   DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (PI_Processing) :");

	/* Is that a PI reception or a timeout ?
	*/
	if (vI_PIreceptionStatus)
		{
		/* the PI information as been received, check PI. 
		*/

		/* Store the PI if AF check on-going */
		if (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
			{
			/* AF on going : store PI */
			DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].piValue = DABMW_serviceFollowingStatus.currentSid;
            // We should not 'again' get the quality 
            // this is suppose to be stored already
			// DABMW_ServiceFollowing_StoredAndEvaluate_AF_FMQuality();
			}
														
//		if (DABMW_SF_BackgroundScanInfo.searchedServiceID == DABMW_serviceFollowingStatus.currentSid)
		vl_succesfulSearched = DABMW_ServiceFollowing_GetSearchResult(DABMW_serviceFollowingStatus.currentSid,
																				false, // bearer is not DAB
																				true); // ID received

        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : PI received 0x%04x, searched %s\n", 
                                                                                    DABMW_serviceFollowingStatus.currentSid,
                                                                                    ((vl_succesfulSearched!=DABMW_SF_BG_RESULT_NOT_FOUND)?"SUCCESS":"FAILURE"));
        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */

		}
	else
		{
		/* Check if the same frequency has already that PI registered. 
		* If so, return found but pi not confirmed.
		*/
		vl_succesfulSearched = DABMW_ServiceFollowing_GetSearchResult(DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPiFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq),
																							false, // bearer is not DAB
																							false); // ID received


        // for the moment do not handle the case of PI not confirmed...  
        // this is at risk : the PI is not ok, and anyhow if PI not read... we can guess quality is relatively poor !
        // better to continue searching isn't it ?
        //
        if ((DABMW_SF_BG_RESULT_FOUND_PI_NOT_CONFIRMED == vl_succesfulSearched)
            ||
            (DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_NOT_CONFIRMED == vl_succesfulSearched)
            ||
            (DABMW_SF_BG_RESULT_FOUND_INITIAL_PI_NOT_CONFIRMED == vl_succesfulSearched))
            {
            vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
            }
        
        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : PI not received \n");
        
#endif // defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */
		}

#if defined (CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
		// De-register callback
		DABMW_ServiceFollowing_ExtInt_AmFmLandscapeDeRegister (&DABMW_ServiceFollowing_OnPI_Callback, DABMW_SF_BackgroundScanInfo.bg_Freq);
#endif // CONFIG_TARGET_DABMW_RDS_COMM_ENABLE


    //  if correct then do some check 
    // - skip frequency if same than alternate
    // - or if quality not as good as alternate (if alternate is FM)
    // - skip if not suitable
    //
    if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
        {
    	/* Should we check for the quality ?	*/
    	/* check the quality : if an alternate is configured in FM already, we should look for an better that current 
    	        * state here should be : AF Scan
    	        */

    	if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.alternateFrequency)
            {
            if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
        	    {
                /* if we have an alternate, check if this one is different and better */
                /* if same freq than alternate, skip */

               // measure the current frequency on same app than alternate for far comarison
               // 
               DABMW_ServiceFollowing_Measure_AF_FMQuality_OnAlternatePath();                                         

                // measure the current on same app than alternate
            	if (DABMW_serviceFollowingStatus.alternateFrequency == DABMW_SF_BackgroundScanInfo.bg_Freq)
            		{
            		/* the alternate found is not better than current alternate : 
            		        *   continue the seach 
            		        */
            		/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : this is alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                /* END TMP LOG */
            		vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

                    /* Store the STATUS if AF check on-going */
                    if (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
                        {
                         /* AF on going : store PI */
                         DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE;
                        }

            		 }
            	else if (false == DABMW_ServiceFollowing_qualityA_better_qualityB_FM(DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath, DABMW_serviceFollowingStatus.alternate_Quality, 
                                                                            DABMW_serviceFollowingStatus.currentFrequency, DABMW_serviceFollowingStatus.alternateFrequency))
            		{
            		/* the alternate found is not better than current alternate : 
            		        * continue the seach 
            		        */
            		/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : this is not as good quality as alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                /* END TMP LOG */
            		vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

                    /* Store the STATUS if AF check on-going */
                    if (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
                        {
                         /* Store the STATUS if AF check on-going */
                         DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE;
                        }

            		}
    		    }
            else  
                {
                //alternate is DAB, current if FM

                if (true == DABMW_ServiceFollowing_qualityA_DAB_better_qualityB_FM(DABMW_serviceFollowingStatus.alternate_Quality, DABMW_serviceFollowingStatus.current_Quality, 
                                                                                                       DABMW_serviceFollowingStatus.alternateFrequency, DABMW_serviceFollowingStatus.currentFrequency))
                    {
                    /* the current found is not better than current alternate : 
                                     * continue the seach 
                                     */
                    /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : this is not as good as alternate because DAB preferred--> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    /* END TMP LOG */
                     vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;
                                                
                    /* Store the STATUS if AF check on-going */
                    if (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
                        {
                         /* Store the STATUS if AF check on-going */
                         DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE;
                        }
                    }   
                }
        	}

        // we do not need to check the quality : this is done at Seek processing... 
        // 
        }
    else
        {
       /* Store the STATUS if AF check on-going */
        if (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
            {
             /*  Store the STATUS if AF check on-going */
             if (true == vI_PIreceptionStatus)
                {
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_MATCHING_SID_PI;
                }
             else
                {
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_PI_NOT_RECEIVED;
                }
             
            }
        }

	if (DABMW_SF_BG_RESULT_NOT_FOUND == vl_succesfulSearched)
		{                  
		 // if we are in landscape scan : request the PS 
		 // 
	 	if ((true == DABMW_ServiceFollowing_IsLanscapeScanOnGoing())
			&&
			(true == vI_PIreceptionStatus)
	 		&&			
			(NULL == DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPsFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq))
			)
			{
			// there is not decoded PS yet
			// wait for the PS before moving forward
#if defined(CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
			DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPsAtFreq(DABMW_SF_BackgroundScanInfo.bg_Freq, NULL, &DABMW_ServiceFollowing_OnPS_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
				
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_PS);

			/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : Go in WAIT_PS \n"); 	   
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
			/* END TMP LOG */
	 		}
		else
			{
			if ((true == DABMW_ServiceFollowing_IsLanscapeScanOnGoing())
				&&
				(true == vI_PIreceptionStatus))
				{		
				/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
				DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Seek_Processing) : PS known %s -> continue \n", DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPsFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq)); 	   
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
				/* END TMP LOG */
				}
			else
				{
					/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
					DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : Freq skipped \n"); 	   
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
					/* END TMP LOG */
				}
			
		    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.nextStatus);

            if (DABMW_SF_STATE_AF_CHECK == DABMW_serviceFollowingStatus.status)
                {
                DABMW_ServiceFollowing_AFCheck();
                }
            else if (DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.status)
				{
				DABMW_ServiceFollowing_BackgroundCheck();
				}
			else
				{
				DABMW_ServiceFollowing_BackgroundScan();
				}
			}
		}
	else
		{
		/* PI found or confirmed
		* this marks the end of scan
		*/
		 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PI_Processing) : Freq SUCCESS \n");
        
#endif //  defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */

		DABMW_SF_BackgroundScanInfo.succesfulSearched = vl_succesfulSearched;

        /* Store the STATUS if AF check on-going */
        if (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
            {
             /*  Store the STATUS if AF check on-going */
             DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_FREQ_OK;
            }

        DABMW_ServiceFollowing_EndBackgroundScan();

        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
    	DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.nextStatus);
		}

             /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                /* END TMP LOG */


	return OSAL_OK;
}

/* Procedure to act while on PI awaiting state
* wake-up status in boolean parameter
* TRUE = PI received (event awake..)
* FALSE = Timeout !
*/
tSInt DABMW_ServiceFollowing_PS_processing(tBool vI_PIreceptionStatus)
{
	// PS processing.
	// for now we do not manage the PS, only letting time to decode.
	// continue the scan
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	tPU8 vl_Ps;
		
	vl_Ps =  DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPsFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq);
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    

	// if we are in landscape scan : request the PS 
	// 

#if defined (CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
	// De-register callback
	DABMW_ServiceFollowing_ExtInt_AmFmLandscapeDeRegister(&DABMW_ServiceFollowing_OnPS_Callback, DABMW_SF_BackgroundScanInfo.bg_Freq);
#endif // CONFIG_TARGET_DABMW_RDS_COMM_ENABLE		

	// continue the BG
	// when we are in PS processing it should be only the background 
	DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.nextStatus);

 	/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
		DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PS_processing) : Freq = %d, PS received = %d\n", DABMW_SF_BackgroundScanInfo.bg_Freq, vI_PIreceptionStatus);
		if (NULL != vl_Ps)
		{			
			DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (PS_processing) : Freq = %d, PS = %s \n", DABMW_SF_BackgroundScanInfo.bg_Freq, vl_Ps);
		}
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	/* END TMP LOG */

	if (DABMW_SF_STATE_AF_CHECK == DABMW_serviceFollowingStatus.status)
		{
		DABMW_ServiceFollowing_AFCheck();
		}
	else if (DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.status)
		{
		DABMW_ServiceFollowing_BackgroundCheck();
		}
	else
		{
		DABMW_ServiceFollowing_BackgroundScan();
		}

	
	return OSAL_OK;					 
}

/* Procedure to act while on DAB Tune  awaiting state
* wake-up status in boolean parameter
* TRUE = PI received (event awake..)
* FALSE = Timeout !
*/
tSInt DABMW_ServiceFollowing_DAB_processing(tBool vI_TuneStatus)
{
	
	/* Is that a notification or a timeout ?
	*/
	if (vI_TuneStatus)
		{
		
        //	if (DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_TUNED != DABMW_serviceFollowingStatus.dab_syncStatus)
        if (DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SIG != (DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SIG & DABMW_serviceFollowingStatus.dab_syncStatus))   
           {
    		/* continue the background scan */

            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.nextStatus);

            /* store info */
            	
    	    if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
    		    && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                {   
                
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
			DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (DAB_processing) : app = %d, Freq = %d => Sync not ok \n", DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq);
			DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif
	
                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_QUALITY_NOT_OK;
                }


            if (DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.status)
        			{
        			DABMW_ServiceFollowing_BackgroundCheck();
        			}
        		else
        			{
        			DABMW_ServiceFollowing_BackgroundScan();
        			}
    		}
        else if ((DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_TUNED == DABMW_serviceFollowingStatus.dab_syncStatus)
            ||
            (DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_MCI == (DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_MCI & DABMW_serviceFollowingStatus.dab_syncStatus))
            )
    		{
            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (DAB_processing) : app = %d, Freq = %d is tuned\n", DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq);
            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif
    		/* WAIT FIC Reading
    		*/
    		DABMW_SF_BackgroundScanInfo.fig00_read = false;
            DABMW_SF_BackgroundScanInfo.fig01_read = false;
            DABMW_SF_BackgroundScanInfo.fig02_read = false;
    		DABMW_SF_BackgroundScanInfo.fig02_readCounter = 0;
            
    		DABMW_ServiceFollowing_ExtInt_DabFigLandscapeRegisterForFigAtFreq (DABMW_SF_BackgroundScanInfo.bg_Freq, NULL, DABMW_ServiceFollowing_OnFIG00_Callback,
    												DABMW_FIG_TYPE_0, DABMW_FIG_EXT_0, true);
    		DABMW_ServiceFollowing_ExtInt_DabFigLandscapeRegisterForFigAtFreq (DABMW_SF_BackgroundScanInfo.bg_Freq, NULL, DABMW_ServiceFollowing_OnFIG00_Callback,
    												DABMW_FIG_TYPE_0, DABMW_FIG_EXT_1, false);
    		DABMW_ServiceFollowing_ExtInt_DabFigLandscapeRegisterForFigAtFreq (DABMW_SF_BackgroundScanInfo.bg_Freq, NULL, DABMW_ServiceFollowing_OnFIG00_Callback,
    												DABMW_FIG_TYPE_0, DABMW_FIG_EXT_2, false);

    		
    		DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
            
            if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
    		    && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                {  
                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].statusFlagInfo.field.isQualityAcceptable = true;
                }
            
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_FIC);
    		}
        else
            {
            /* we have DAB here, but not ready yet.. should we just wait ? 
                  */
            
            }
	}
    else
    {
        //
        // We are not tuned yet in DAB, this is time to change to next...
        //
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (DAB_Processing) :app = %d, Freq = %d timeout, Freq skipped \n", DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq);        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */

		    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.nextStatus);

            if (DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.status)
				{
				DABMW_ServiceFollowing_BackgroundCheck();
				}
			else
				{
				DABMW_ServiceFollowing_BackgroundScan();
				}
        
    }

	return OSAL_OK;
}

/* Procedure to act while on DAB Tune  awaiting state
* wake-up status in boolean parameter
* TRUE = PI received (event awake..)
* FALSE = Timeout !
*/
tSInt DABMW_ServiceFollowing_FIC_ReadyProcessing(tBool vI_FIC_Ready_Status)
{
    tU32 *pl_serviceList, *pl_serviceList_2; 
 	tSInt vl_sidCnt;
	tU32  vl_ensembleUniqueId;
    tSInt vl_serviceNum;
//	tSInt vl_tmp_res;
	DABMW_SF_BG_SearchedResultTy vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
    tBool vl_serviceFound = false;

	/* DAB is synchronized */
	/* and if we are here,  FIG ready now */

	/* Deregister callback for all registered fig */
	DABMW_ServiceFollowing_ExtInt_DabFigLandscapeDeRegister(DABMW_ServiceFollowing_OnFIG00_Callback, DABMW_SF_BackgroundScanInfo.bg_Freq, 0xFF, 0xFF);
										
	// Store the quality        
    DABMW_serviceFollowingStatus.current_Quality.dabQuality = DABMW_ServiceFollowing_DAB_GetQuality(DABMW_SF_BackgroundScanInfo.bgScan_App);
    //set the service has not selected for the current..; 
    // since for bg we do not select the service.
    DABMW_serviceFollowingStatus.current_Quality.dabQuality.service_selected = false;
        
	// should we check if service is present ? 
	// we should use the EnsembleID from the system_app to be sure this is well set with the many tuned...
	// BEFORE
	// vl_ensembleUniqueId = DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(DABMW_SF_BackgroundScanInfo.bgScan_App);
    // AFTER
	vl_ensembleUniqueId = DABMW_ServiceFollowing_ExtInt_GetApplicationEnsembleId(DABMW_SF_BackgroundScanInfo.bgScan_App);

	DABMW_serviceFollowingStatus.currentEid = vl_ensembleUniqueId;

	// check if searched service exists
	vl_serviceNum = DABMW_ServiceFollowing_ExtInt_GetServiceList(vl_ensembleUniqueId, (tVoid **)&pl_serviceList);

	/* Store the Eid if AF check on-going */
	if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
		&& (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
		{
		DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].uniqueEid = vl_ensembleUniqueId;
		DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].num_Sid = vl_serviceNum;
        DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].quality = DABMW_serviceFollowingStatus.current_Quality;
		}


    /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FicProcessing) : FIC_Ready_Status %d, num fig02 %d\n", vI_FIC_Ready_Status,DABMW_SF_BackgroundScanInfo.fig02_readCounter );
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FicProcessing) : vl_ensembleUniqueId = %08x\n",  vl_ensembleUniqueId);
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FicProcessing) : vl_serviceNum = %08x\n", vl_serviceNum);
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FicProcessing) : app = %d, Freq = %d, quality (ficber) = %d ;\n", DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq, DABMW_serviceFollowingStatus.current_Quality.dabQuality.fic_ber );                  

        pl_serviceList_2 = pl_serviceList;
        for (vl_sidCnt= 0; vl_sidCnt < vl_serviceNum; vl_sidCnt++)
		    {
		    vl_succesfulSearched = DABMW_ServiceFollowing_GetSearchResult(*(pl_serviceList_2),true, true);
		    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FicProcessing) : ServiceId = %08x, Service Found Status %d\n", *(pl_serviceList_2), vl_succesfulSearched);
            pl_serviceList_2++;
            }
        DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
    /* END TMP LOG */


	pl_serviceList_2 = pl_serviceList;
	
	for (vl_sidCnt= 0; vl_sidCnt < vl_serviceNum; vl_sidCnt++)
		{

		/* Store the Eid if AF check on-going */
		if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
			&& (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
			{
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].uniqueSidList[vl_sidCnt] = *(pl_serviceList_2);          
			}
		
		vl_succesfulSearched = DABMW_ServiceFollowing_GetSearchResult(*(pl_serviceList_2),
																true, // bearer is  DAB
																true); // ID received
																					
		if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
			{
			DABMW_serviceFollowingStatus.currentSid = *(pl_serviceList_2);
            vl_serviceFound = true;
			
			if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
				{
				DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].statusFlagInfo.field.SearchedSidFound = true;
                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_FREQ_OK;
				}
			
			break;
			}
		
		pl_serviceList_2++;
		}

	/* remove that part : 
	* serivceselect does an audio source switch 
	* so only to be done at later stage !!
	*/
	/*
    	if (DABMW_SF_BG_RESULT_NOT_FOUND != DABMW_SF_BackgroundScanInfo.succesfulSearched)
		{
		// DAB is here. Select the service 
				
		 vl_tmp_res = DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_SF_BackgroundScanInfo.bgScan_App,										   // Use parameter passed
										 DABMW_SERVICE_SELECT_SUBFNCT_SET,				// Sub-function is 5
										 DABMW_serviceFollowingStatus.currentEid,		 // Ensemble  has already been set in tune callback
										 DABMW_serviceFollowingStatus.currentSid); 		// Service ID does not change			  
			
		//  check : is the service selection ok ? 
				
		if (OSAL_OK == vl_tmp_res)
			{
			// Service ok  
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].SearchedSidFound = true;
			}
		else
			{
			// should not come here but you never know 
			DABMW_SF_BackgroundScanInfo.succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
			DABMW_serviceFollowingStatus.currentSid = DABMW_INVALID_SC_ID;
			}
		}
		*/
		
	/* Should we check for the quality ?	*/

    if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched )
        {
        // record the flow type for the quality
        DABMW_ServiceFollowing_ExtInt_GetServiceTypeFromServiceId(DABMW_serviceFollowingStatus.currentEid, DABMW_serviceFollowingStatus.currentSid, &(DABMW_serviceFollowingStatus.current_Quality.dabQuality.component_type), &(DABMW_serviceFollowingStatus.current_Quality.dabQuality.kindOfComponentType));

		 /* the cell should be suitable ie MEDIUM OR GOOD else we will not remain on it as an alternate, neither for camping...
                * not that decision is to change the alternate to DAB one even if we have an FM alternate... as soon as DAB is MEDIUM or GOOD it is preferred
                */

            // Change for DAB : codex #318038
            // if we are in scan ... we should be happy with the tune and FIC reception
            // this make the cell implicitly acceptable.
            //

            /*
                    if (false == DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_serviceFollowingStatus.current_Quality))
                        {
                           vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
                          // TMP LOG 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FIC_ReadyProcessing) : this is not acceptable quality --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                            //END TMP LOG 
                        if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                            && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                            {   
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_QUALITY_NOT_OK;
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].statusFlagInfo.field.isQualityAcceptable = false;
                            }

                        }
                    else 
                        */
            {
            // mark the quality as acceptable if freq used.
            if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
				    {
                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].statusFlagInfo.field.isQualityAcceptable = true;
                    }

            // compare towards alternate
            //
            if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.alternateFrequency)
                {
                // we have an alternate 
                
            	/* check the quality : if an alternate is configured in DAB already, we should look for an better that current 
            	                */
            	if ((true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp))
					&&
					(true == DABMW_serviceFollowingData.dabToDabIsActive))
            		{
            		/* if same freq than alternate, skip */
            		if (DABMW_serviceFollowingStatus.alternateFrequency == DABMW_SF_BackgroundScanInfo.bg_Freq)
            			{
            			/* the alternate found is not better than current alternate : 
            			        * continue the seach 
            			        */
            			vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;
                        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FIC_ReadyProcessing) : this is alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                        /* END TMP LOG */
                        if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                            && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                            {   
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE;
                            }

            			}
            		else if (false == DABMW_ServiceFollowing_qualityA_better_qualityB_DAB(DABMW_serviceFollowingStatus.current_Quality, DABMW_serviceFollowingStatus.alternate_Quality,
                                                                                DABMW_serviceFollowingStatus.currentFrequency, DABMW_serviceFollowingStatus.alternateFrequency))
            			{
            			/* the alternate found is not better than current alternate : 
            			                * continue the seach 
            			                */           
            			vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
                        
                        	/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FIC_ReadyProcessing) : this is not as good quality as alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                        /* END TMP LOG */
                        if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                            && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                            {   
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE;
                            }
            			}
                    }
                else if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
                    {
                        // alternate is FM
                        // is DAB any better ? 
                        // 
   
                        // 
                        //alternate is FM, current if DAB

                         if (false == DABMW_ServiceFollowing_qualityA_DAB_better_qualityB_FM(DABMW_serviceFollowingStatus.current_Quality, DABMW_serviceFollowingStatus.alternate_Quality,
                                                                                       DABMW_serviceFollowingStatus.currentFrequency, DABMW_serviceFollowingStatus.alternateFrequency))
                            {
                            /* the alternate found is not better than current alternate : 
                                                   * continue the seach 
                                                   */
                            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (FIC_ReadyProcessing) : this is not as good as alternate because DAB preferred --> skip \n");        
#endif 
//  defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                             /* END TMP LOG */
                             vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

                            if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                                && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                                {   
                                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE;
                                }

                                
                            }                    
                    }
                 }                
            }
    }
    else
    {
      if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                {   
                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_MATCHING_SID_PI;
                }
    }
    
	

	if (DABMW_SF_BG_RESULT_NOT_FOUND == vl_succesfulSearched)
		{	
		/* Search not successfull */
        /* if this is a timer expiry or because enough fig02 or because good service but not suitable : change state */
        if ((DABMW_SF_BackgroundScanInfo.fig02_readCounter >= SF_NB_MAX_FIG02_TO_WAIT)
            ||
            (false == vI_FIC_Ready_Status)
            ||
            (true == vl_serviceFound)
            )
           {
            /* Timer has elapse or /  consider fig02 are complete : continue scan */
    		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.nextStatus);

    		if (DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.status)
    			{
    			DABMW_ServiceFollowing_BackgroundCheck();
    			}
    		else
    			{
    			DABMW_ServiceFollowing_BackgroundScan();
    			}
            }
        else
            {
            /* service not found, assuming that it may be because vl_service not found
                    *   so  wait more fig02
                    */
                    
            /* keep fig_02 registration */
            /* only partial reception of fig02  : remain in that state to wait new fig02*/
            DABMW_SF_BackgroundScanInfo.fig02_read = false;
            
          	DABMW_ServiceFollowing_ExtInt_DabFigLandscapeRegisterForFigAtFreq (DABMW_SF_BackgroundScanInfo.bg_Freq, NULL, DABMW_ServiceFollowing_OnFIG00_Callback,
												DABMW_FIG_TYPE_0, DABMW_FIG_EXT_2, false);
            

            }
		}
	else
		{
		/* Service found or confirmed
		* this marks the end of scan
		*/
		DABMW_SF_BackgroundScanInfo.succesfulSearched = vl_succesfulSearched;
        
            if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
                && (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status))
                {   
                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_FREQ_OK;
                }

        DABMW_ServiceFollowing_EndBackgroundScan();
        
        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.nextStatus);
		}
    
	return OSAL_OK;
}


/* 
* Start Service Selection Request
*/

tSInt DABMW_ServiceFollowing_ServiceIDSelection(DABMW_SF_mwAppTy vI_app, tU32 vI_SearchedServiceID, tBool vI_keepDecoding,tU8 vI_injectionSide, tBool vI_noOutputSwitch)
{

	tSInt vl_numLandscapeFreq;



	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ServiceIDSelection : search requested : path = %d, Service = 0x%x\n",
				vI_app,
				vI_SearchedServiceID);

	/* Check if we are not already in service selection
	* else reject the selection
	*
	* else : allow the service selection...
	* that should/will interrupt the on-going processing if any
	*
	* if ((DABMW_SF_STATE_INITIAL_SERVICE_SELECTION == DABMW_serviceFollowingStatus.prevStatus)
	*	&&
	*	()
	*	{
	*	return OSAL_ERROR;
	*	}
	*/


    // Set the selected service
    DABMW_serviceFollowingData.initial_searchedPI = vI_SearchedServiceID;
    
	/* Set the selected tuner for initial sel selection 
	*/
	DABMW_SF_BackgroundScanInfo.bgScan_App = vI_app;

	/* Set the audio config choice
	*/
	DABMW_SF_BackgroundAudioInfo.keepDecoding = vI_keepDecoding;
	DABMW_SF_BackgroundAudioInfo.injectionSide = vI_injectionSide;
	DABMW_SF_BackgroundAudioInfo.noOutputSwitch = vI_noOutputSwitch;

	/* Init the requested algo */
	
	DABMW_SF_BackgroundScanInfo.AFCheckRequested = false;
	DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = false;
	DABMW_SF_BackgroundScanInfo.backgroundScanRequested = true;
    DABMW_SF_BackgroundScanInfo.backgroundScanDone = false;

    // init the 'FM first' flag
    DABMW_serviceFollowingStatus.configurationIsDabFirst = false;

	/* allow to camp on current freq if any.. */
	DABMW_SF_BackgroundScanInfo.backgroundScanSkipOriginalFreq = false;

	/* init previous state & current */
    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_INITIAL_SERVICE_SELECTION);

	/* reset current freq info */
	DABMW_serviceFollowingStatus.currentAudioUserApp = (DABMW_SF_mwAppTy) DABMW_SF_BackgroundScanInfo.bgScan_App ;
	DABMW_serviceFollowingStatus.currentFrequency = DABMW_INVALID_FREQUENCY;	
	DABMW_serviceFollowingStatus.currentSystemBand = DABMW_BAND_NONE;
	DABMW_serviceFollowingStatus.currentEid = DABMW_INVALID_ENSEMBLE_ID;
	DABMW_serviceFollowingStatus.currentSid = DABMW_INVALID_SERVICE_ID;
	DABMW_serviceFollowingStatus.dab_syncStatus = DABMW_INVALID_DATA_BYTE;
	// add on etal
	DABMW_serviceFollowingStatus.currentHandle = ETAL_INVALID_HANDLE;

	/* reset alternate */
	DABMW_ServiceFollowing_ResetMonitoredAF();

    // reset original as well
    DABMW_serviceFollowingStatus.originalAudioUserApp = DABMW_NONE_APP;
	DABMW_serviceFollowingStatus.originalFrequency= DABMW_INVALID_FREQUENCY;	
	DABMW_serviceFollowingStatus.originalSystemBand= DABMW_BAND_NONE;
	DABMW_serviceFollowingStatus.originalEid= DABMW_INVALID_ENSEMBLE_ID;
	DABMW_serviceFollowingStatus.originalSid= DABMW_INVALID_SERVICE_ID;
	// add on etal
	DABMW_serviceFollowingStatus.originalHandle = ETAL_INVALID_HANDLE;
	
	/* reset the counter since we start on new idle mode & cell */
    DABMW_serviceFollowingStatus.original_Quality = DABMW_ServiceFollowing_QualityInit();
    DABMW_serviceFollowingStatus.original_Quality_onBackground = DABMW_ServiceFollowing_QualityInit();
	DABMW_serviceFollowingStatus.original_badQualityCounter = 0;

    DABMW_serviceFollowingStatus.original_mediumQualityCounter = 0;
    
	/* this is the enabling of service following */
	DABMW_serviceFollowingData.enableServiceFollowing = true;

    // enable RDS : is done later on
    // if fm to fm supported, enable the RDS it is mandatory for SF
    // DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, true);

	
    // Init  data
    //
    // Most of the data should already be init. 
    //
    // either by init, either when 'service following on tune'...
    //
    // What we need to init is the timer part
    //
    DABMW_serviceFollowingStatus.deltaTimeFromLastSwitch = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime (); 
	DABMW_serviceFollowingStatus.lastServiceRecoveryTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	//DABMW_serviceFollowingStatus.lastSearchForAFTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_serviceFollowingStatus.lastSearchForAFTime = DABMW_INVALID_DATA;
    //DABMW_serviceFollowingStatus.lastFullScanForAFTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
    DABMW_serviceFollowingStatus.lastFullScanForAFTime = DABMW_INVALID_DATA;
	DABMW_serviceFollowingStatus.lastSwitchTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	// landscape scan to not need update/
	// it runs in bg permanent
	// DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();
	DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();

    DABMW_serviceFollowingStatus.original_badQualityCounter = 0;
	DABMW_serviceFollowingStatus.original_mediumQualityCounter = 0;   
    

	/* reset the link information */
	DABMW_ServiceFollowing_LinkInfo_Release();

	/* Init data for search 
	*/
	if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_app))
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vl_numLandscapeFreq = DABMW_ServiceFollowing_BuildAlternateFrequency_DAB(vI_SearchedServiceID, DABMW_INVALID_FREQUENCY, DABMW_INVALID_FREQUENCY);
		/* init data for future full band scan */
		DABMW_ServiceFollowing_Background_ScanInit(vI_SearchedServiceID, DABMW_ServiceFollowingGetDABBandIII(), DABMW_BAND_NONE, DABMW_BAND_NONE);
		}
	else
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vl_numLandscapeFreq = DABMW_ServiceFollowing_BuildAlternateFrequency_FM(vI_SearchedServiceID, DABMW_INVALID_FREQUENCY, DABMW_INVALID_FREQUENCY);
		/* init data for future full band scan */
		DABMW_ServiceFollowing_Background_ScanInit(vI_SearchedServiceID, DABMW_ServiceFollowingGetFMBand(), DABMW_BAND_NONE, DABMW_BAND_NONE);
		}
				
	if (vl_numLandscapeFreq > 0)
		{
		/* start by AF or background check on the stored info in landscape
		*/
		DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = true;
        DABMW_SF_BackgroundScanInfo.backgroundCheckDone = false;

		/* AF Check : only if current application is FM & is the selected one 
		* else we assume this is not the tuned port for user => we can do background check
		*/
		if ((true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(vI_app)) && (vI_app == DABMW_serviceFollowingStatus.originalAudioUserApp))
			{
			/* let's do an AF Check 1st */
			DABMW_SF_BackgroundScanInfo.AFCheckRequested = true;
            DABMW_SF_BackgroundScanInfo.AFCheckdone = false;
            
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
			}				
		else 
			{
			/* Initialize the new state that will be entered
			*/
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            
			}
		}
	else
		{
		/* this will be a background scan, no step to AF or background check
		*/
		/* Initialize the new state that will be entered
		*/

        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);            
		}
	

	/* this is all for the init, now the background will start on trigger
	*/

	
	return OSAL_OK;
}


/* Callback notification function for PI  case
*/
#if defined(CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
tVoid DABMW_ServiceFollowing_OnPI_Callback (tU32 vI_pi, tU32 vI_freq, tPVoid p_paramPtr)
{

	
	/* p_paramPtr not used for the moment : avoid Lint Warning */
	(tVoid)p_paramPtr; // Avoid Lint warning

	if (vI_freq == DABMW_serviceFollowingStatus.currentFrequency)
		{
			/* set PI */
			DABMW_serviceFollowingStatus.currentSid = vI_pi; 
	
			DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_PI_RECEIVED);
		}
	else
		{
		/* this is a called back for a different frequency 
		* => crossing case because should have been cancel
		* nothing to do
		*/
		}
	
	return;
}

tVoid DABMW_ServiceFollowing_OnPS_Callback (tU32 vI_pi, tU32 vI_freq, tPVoid p_paramPtr)
{

	
	/* p_paramPtr not used for the moment : avoid Lint Warning */
	(tVoid)p_paramPtr; // Avoid Lint warning

	if (vI_freq == DABMW_serviceFollowingStatus.currentFrequency)
		{
			// wake up the name is there :)
			
			DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_PS_RECEIVED);
		}
	else
		{
		/* this is a called back for a different frequency 
		* => crossing case because should have been cancel
		* nothing to do
		*/
		}
	
	return;
}

#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)

/* Callback notification function for FIC Ready  case
*/
tVoid DABMW_ServiceFollowing_OnFIG00_Callback (tU8 vI_fig, tU8 vI_ext, tU32 vI_freq, tU8 vI_ecc, tU16 vI_id, tPVoid pI_params)
{


	/* params not used for the moment : avoid Lint Warning */
	(tVoid)pI_params; // Avoid Lint warning

	if ((vI_freq == DABMW_serviceFollowingStatus.currentFrequency)
		&&
		(DABMW_FIG_TYPE_0 == vI_fig)
		)
		{
			/* Store the Eid if AF check on-going */
		if ((DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.nextStatus)
			&&
			(DABMW_FIG_EXT_1 == vI_ext)
			&&
			(DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status)
			)
			{
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[DABMW_SF_freqCheckList.DAB_FrequencyCheck.index].uniqueEid = DABMW_ENSEMBLE_UNIQUE_ID(vI_ecc, vI_id);
			}
		

		switch (vI_ext)
			{
            case DABMW_FIG_EXT_0:
				DABMW_SF_BackgroundScanInfo.fig00_read = true;
                /* now we can deregister */
                DABMW_ServiceFollowing_ExtInt_DabFigLandscapeDeRegister(DABMW_ServiceFollowing_OnFIG00_Callback, vI_freq, DABMW_FIG_TYPE_0, DABMW_FIG_EXT_0);
				break;
			case DABMW_FIG_EXT_1:
				DABMW_SF_BackgroundScanInfo.fig01_read = true;
                 /* now we can deregister */
                DABMW_ServiceFollowing_ExtInt_DabFigLandscapeDeRegister(DABMW_ServiceFollowing_OnFIG00_Callback, vI_freq, DABMW_FIG_TYPE_0, DABMW_FIG_EXT_1);
				break;
			case DABMW_FIG_EXT_2:
				DABMW_SF_BackgroundScanInfo.fig02_readCounter++;
                DABMW_SF_BackgroundScanInfo.fig02_read = true;
				break;	
			default:
				break;
			}

		if ((true == DABMW_SF_BackgroundScanInfo.fig00_read)
            &&
            (true == DABMW_SF_BackgroundScanInfo.fig01_read)
			&&
			(true == DABMW_SF_BackgroundScanInfo.fig02_read)
			)
			{
			DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_DAB_FIC_READ);
			}
		}
	else
		{
		/* this is a called back for a different frequency 
		* => crossing case because should have been cancel
		* nothing to do
		*/
		/* we should deregister */
        DABMW_ServiceFollowing_ExtInt_DabFigLandscapeDeRegister(DABMW_ServiceFollowing_OnFIG00_Callback, vI_freq, vI_fig, vI_ext);
		}
	
	return;
}

/* Callback notification function for DAB Tune case
*/

tVoid DABMW_SF_DabAfCheckCallbackNotification(DABMW_SF_mwAppTy vI_application, tU32 vI_frequency, tPVoid pIO_paramPtr, DABMW_dabStatusTy *pI_notificationStatus)
{

	/* pIO_paramPtr not used for the moment : avoid Lint Warning */
	(tVoid)pIO_paramPtr; // Avoid Lint warning
	
/* information on notificiation 
notify_reason
	This 4-bit field, coded as an unsigned binary number, indicates the reason for the notification.
	The reasons get_dab_status and tune indicate a direct notification on the corresponding commands.

	The reasons sync, reconf, ber_fic and mute indicate a notification that was sent because a condition set by set_dab_status_auto_notification(...) became true.
	b3...b0
	0000 get_dab_status
	0001 tune
	0010 search_for_ensemble
	0011 Rfd
	0100 sync
	0101 reconf
	0110 ber_fic
	0111 mute
	1000...1111

	==> as a result for a tune we should get notify reason = tune.

ber_fic
	This 3-bit unsigned number shall describe the current FIC channel bit error rate BER (before the Viterbi decoder) as follows:
	b2...b0 BER level
	000 Rfd
	001 Rfd
	010 BER level 1 (BER < 5e-4)
	011 BER level 2 (BER < 5e-3)
	100 BER level 3 (BER < 5e-2)
	101 BER level 4 (BER < 1e-1)
	110 BER level 5 (BER   1e-1)
	111 no BER level available
	This value shall be a mean value retrieved from the FIBs associated with the most recent
	block of 16 periods of 24 ms (= 384 ms).

sync
This 4-bit field indicates how far the receiver has synchronized on the DAB ensemble.

	Bit flag
	b0 DAB signal detected
		The receiver has detected a DAB signal because it was capable of detecting the transmission mode
		b0 = 0: no DAB signal detected
		b0 = 1: DAB signal detected
	b1 DAB receiver is fully synchronized 
		The receiver finished time- and frequency-synchronization so that it is physically synchronized on the signal.
		b1 = 0: not fully synchronized
		b1 = 1: fully synchronized
	b2 Ability of MCI decoding
		The receiver is able to decode the FIC (i.e. the FIC bit error rates are sufficiently low) to gain information about the DAB multiplex.
		b2 = 0: not able to decode the MCI
		b2 = 1: able to decode the MCI

	==> for service selection, we need to wait the synchronisation with b0= 1, b1 = 1, b2 = 1
	==> when notify fpr tune,  it is assume status is complete : so store and post event.

*/


	if ((vI_frequency == DABMW_serviceFollowingStatus.currentFrequency)
		&&
		(vI_application == DABMW_SF_BackgroundScanInfo.bgScan_App)
		)
		{

		if (DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_TUNE == pI_notificationStatus->notifyReason)
        	{
            if (DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_TUNED == pI_notificationStatus->sync)
				{
				DABMW_serviceFollowingStatus.dab_syncStatus = pI_notificationStatus->sync;

	            }
    	    else
        	    {
				DABMW_serviceFollowingStatus.dab_syncStatus = pI_notificationStatus->sync;
            	}
			}
   				
		DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_DAB_TUNE);
		}
	else
		{
		/* this is a called back for a different frequency 
		* => crossing case because should have been cancel
		* nothing to do
		*/
		}

	return;


}

/* Auto Seek Function immediate return
*/

tSInt DABMW_SF_AmFmLearnFrequencyAutoSeekImmediate (DABMW_SF_mwAppTy app, tS16 deltaFreq)
{

    tSInt res = OSAL_ERROR;

#if defined (CONFIG_TARGET_DABMW_TUNERDEV_COMM_ENABLE) && \
    defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)

	/* Some specificity of the AUTO_SEEK & SEEK COMMAND 
	* AUTO_SEEK is a start of SEEK...
	* It will tune and loop on all the band until either a frequency is found or a reach the current tuned frequency
	* In notification result, the seek status indicate if this is the end of full cycle or not : 0x80 seek status >> full cycle
	* using only auto seek means :
	* 	1) tuned on 87.500 MHz,
	* 	2) auto seek command
	*	3) let's say freq is found : 100.100 MHz
	* 	4) auto seek command
	* 	due to 4) =>  auto seek will continue until a freq is found, or it is looped back to 100.1 MHz...
	* 	
	* However, in background we want the autoseek to stop when full band done
	* this is achieved by 'continue' 
	* 	1) tuned on 87.500 MHz,
	* 	2) auto seek command
	*	3) let's say freq is found : 100.100 MHz
	* 	4) SEEK_END command with parameter 0x01: continue 
	* 	thanks  to 4) =>  auto seek will continue until a freq is found, or it is looped back original tune freq : 87.500 Mhz...
	*/



	/* do we need an autoseek (ie start of seek) or a seek continuation ?
	*/

//	if (DABMW_SF_BackgroundScanInfo.bg_Freq != DABMW_INVALID_FREQUENCY)
    if (false == DABMW_SF_BackgroundScanInfo.seek_ongoing)
		{
		/* this is a seek start */
		
        DABMW_SF_BackgroundScanInfo.seek_ongoing = true;
        DABMW_SF_BackgroundScanInfo.seek_app = app;
		DABMW_SF_BackgroundScanInfo.seek_frequency = DABMW_INVALID_FREQUENCY;


	    res = DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekStart(app, deltaFreq, DABMW_SF_AmFmLearnFrequencyAutoSeekCallback);
         
		}
	else 
		{
		/* we need a seek continuation */
		
		res = DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekContinue(app);
		}

#endif

	return res;
}

/* Auto Seek END function
*/

tSInt DABMW_SF_AutoSeekEnd ()
{

tSInt res = OSAL_ERROR;

#if defined (CONFIG_TARGET_DABMW_TUNERDEV_COMM_ENABLE) && defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)


    // if no seek on going, nothing to do
    if (false == DABMW_SF_BackgroundScanInfo.seek_ongoing)
    {
        return res;
    }
    
	res = DABMW_ServiceFollowing_ExtInt_AmFmSeekEnd(DABMW_SF_BackgroundScanInfo.seek_app);
  
    DABMW_SF_BackgroundScanInfo.seek_ongoing = false;
    DABMW_SF_BackgroundScanInfo.seek_app = DABMW_NONE_APP;
	DABMW_SF_BackgroundScanInfo.seek_frequency = DABMW_INVALID_FREQUENCY;

#endif // #if defined (CONFIG_TARGET_DABMW_TUNERDEV_COMM_ENABLE) &&   defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)

    return res;
}


/* AUTO SEEK CALL BACK 
*/
tVoid DABMW_SF_AmFmLearnFrequencyAutoSeekCallback (DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, tPVoid p_I_param)
{

	/* p_I_param not used for the moment : avoid Lint Warning */
	(tVoid)p_I_param; // Avoid Lint warning

	if (vI_app == DABMW_SF_BackgroundScanInfo.seek_app)
		{
		// set bg scan frequency
		DABMW_SF_BackgroundScanInfo.seek_frequency = vI_frequency;		
		}


	DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_AUTO_SEEK);
	
	return;

}

/* AUTO SEEK RESULT PROCESSING */
tSInt DABMW_ServiceFollowing_Seek_processing(tBool vI_SeekReceptionStatus)
{
	tU32 vl_tunedFrequency = DABMW_INVALID_FREQUENCY;
	tU32 vl_tmpTunedFreq = DABMW_INVALID_FREQUENCY;
	tU32 vl_tmpStopFreq = DABMW_INVALID_FREQUENCY;
	tU32 vl_currentFreq = DABMW_INVALID_FREQUENCY;
	tU32 vl_maxFreq, vl_minFreq;
	DABMW_SF_mwCountryTy vl_country = DABMW_ServiceFollowing_ExtInt_GetCountry();
	DABMW_SF_mwAppTy vl_app;
	tBool vl_seekIsGoodFreq = false;
//	tU32 vl_fieldStrength;
	tU32 vl_startFreq;
	tU32 vl_stopFreq;
	DABMW_SF_systemBandsTy vl_systemBand;
	DABMW_SF_BG_SearchedResultTy vl_succesfulSearched;
//	DABMW_SF_QualityStatusTy vl_qualityValue = DABMW_SF_QUALITY_IS_POOR;
    tU32 vl_PI;
	tBool vl_PiReconfirmationNeeded = false;


	vl_app = DABMW_SF_BackgroundScanInfo.bgScan_App;
	vl_startFreq = DABMW_SF_BackgroundScanInfo.startFreq;
	vl_stopFreq = DABMW_SF_BackgroundScanInfo.stopFreq;
	vl_systemBand = DABMW_serviceFollowingStatus.currentSystemBand;

	vl_currentFreq = DABMW_SF_BackgroundScanInfo.bg_Freq;
	vl_maxFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmax(vl_systemBand, vl_country);
	vl_minFreq = DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin(vl_systemBand, vl_country);

	if (true == vI_SeekReceptionStatus)
		{
		/* proceed with seek result */
		// Get the current frequency
		vl_tunedFrequency = DABMW_SF_BackgroundScanInfo.seek_frequency;

        // Get the seek status for the current application doing seek
        vl_seekIsGoodFreq = DABMW_ServiceFollowing_ExtInt_SeekEndedOnGoodFreq(vl_app);
       
 		/* build the tmp value : get the current frequency, and tuned value, back to linear 0 / (max-min) freq.
		* in that condition, the tuner should always be more than current, else it means we have done full path
		*/
		// correction because some compiler do not manage same way the modulo with negative numer
		// so to be sure add the range.
		vl_tmpStopFreq = (((vl_stopFreq-1) - vl_startFreq + (vl_maxFreq-vl_minFreq)) % (vl_maxFreq-vl_minFreq)) + 1;
		vl_tmpTunedFreq = (vl_tunedFrequency - vl_startFreq + (vl_maxFreq-vl_minFreq)) % (vl_maxFreq-vl_minFreq);	
		vl_currentFreq = (vl_currentFreq - vl_startFreq + (vl_maxFreq-vl_minFreq)) % (vl_maxFreq-vl_minFreq);

				
		if (((vl_tmpTunedFreq > vl_tmpStopFreq) || (vl_tmpTunedFreq < vl_currentFreq))
			||
			(true == DABMW_ServiceFollowing_ExtInt_SeekFullCycleDone(vl_app)))
			{
			/* we have done a full seek
			* known because seek Status should be set to 0x80 or 0x81
			* either known because range of stop / start
			*/
			vl_tunedFrequency = DABMW_INVALID_FREQUENCY;

            /* send an auto seek END */
            // Note : it could be avoided here, because it is handle in Background_NextScan
		    DABMW_SF_AutoSeekEnd();            
			}
		else 
			{
			// Get the quality retrieved from the seek command

            // Get the seek quality
            DABMW_serviceFollowingStatus.current_Quality.fmQuality = DABMW_ServiceFollowing_ExtInt_GetSeekQuality(vl_app);

            /* EPR TMP CHANGE */
            //in current implementation, we do not remain in seek, but just tuned
            // so quality retrieve has usual
            //DABMW_serviceFollowingStatus.current_Quality.fmQuality = DABMW_ServiceFollowing_ExtInt_AmFmGetQuality(vl_app);
            /* END EPR CHANGE */ 
            
			// Get the field strength indication in order to store it in the
			// RDS landscape
//			vl_qualityValue = DABMW_ServiceFollowing_GetQualityLevelFM(vl_fieldStrength);

//            vl_qualityValue = DABMW_ServiceFollowing_GetQualityStatusFM(DABMW_serviceFollowingStatus.current_Quality);        
			}
		}
	else
		{
		/* seek did not end correctly ..*/
		/* send an auto seek END and continue marking current freq as invalid */
		DABMW_SF_AutoSeekEnd();
		
		vl_tunedFrequency = DABMW_INVALID_FREQUENCY;
		}



	// seek ended : resume the Rds which should have been suspended
	// this will also purge the RDS data to restart clean
	DABMW_ServiceFollowing_ResumeRds(vl_app);
       
	DABMW_serviceFollowingStatus.currentFrequency = vl_tunedFrequency; 

      /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) : app = %d, Seek Freq = %d, Quality %d (dBuV)\n", 
									DABMW_SF_BackgroundScanInfo.bgScan_App, 
									vl_tunedFrequency,
									DABMW_serviceFollowingStatus.current_Quality.fmQuality.fieldStrength_dBuV);
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */

    DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (SeekProcessing) :");
		 
	if (DABMW_INVALID_FREQUENCY == vl_tunedFrequency)
		{
		 /* if current freq is invalid, it means that seek has ended to nothing...
		 * for continuation of scan, we need to put bg_freq at border.
		 * so that at scanNext => it switch of band.
		 */
		 DABMW_SF_BackgroundScanInfo.bg_Freq = DABMW_SF_BackgroundScanInfo.stopFreq;
		}
	else
		{
		DABMW_SF_BackgroundScanInfo.bg_Freq = vl_tunedFrequency;
		}

	/* if this is the original (camped) frequency : skip it potentially
	*/
	if ((true == DABMW_SF_BackgroundScanInfo.backgroundScanSkipOriginalFreq)
		&&
		(vl_tunedFrequency == DABMW_serviceFollowingStatus.originalFrequency)
		)
		{
		/* skip this one
		* this is decided in next iteration to continue background scan (else we may lock the task ..) 
		 * so that task is rescheduled meanwhile....
		 */
		     /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) :  Frequency skipped\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */

		 DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
         DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 
		}
//	else if ((DABMW_INVALID_FREQUENCY != vl_tunedFrequency) && (DABMW_SEEK_STATUS_IS_GOOD_QUALITY == vl_seekStatus))
	/* check the cell quality : it should be above POOR quality... else we get to loop in mainloop to recovery...
	*/
	else if ((DABMW_INVALID_FREQUENCY != vl_tunedFrequency) 
			&& 
			(true == vl_seekIsGoodFreq)
			&&
			(true == DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_serviceFollowingStatus.current_Quality))
			)
		 {
		 /* Check if PI stored and valid */
		 DABMW_serviceFollowingStatus.currentSid = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetValidPiFromFreq(vl_tunedFrequency, DABMW_serviceFollowingData.PI_validityTime);

     
		 /*
		 * Get PI Status = INVALID => not valid (unknown or stored since too long)
		 * if valid, check value versus searched one.
		 * get the PI only if the quality is enough to decode it
		 */
		 if (DABMW_INVALID_DATA == DABMW_serviceFollowingStatus.currentSid)
			 {

             if (true == DABMW_ServiceFollowing_IsFMQualityEnoughForRdsDecoding(DABMW_serviceFollowingStatus.current_Quality.fmQuality))
                {
                // PI may be decoded
    			 /* the PI at that freq is either unknown either not valid
    			        */
    			 /* PI no valid : wait for PI check */
    				 
#if defined(CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
    			 DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (vl_tunedFrequency, NULL, &DABMW_ServiceFollowing_OnPI_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
    		 
    			 DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

                 // increase the PI matching possibility : 
                 // either this is a PI confirmation 
                 // either this is a AF
                 // let's get chances... 
                 // 

                 // if this is an AF.
                 // if this is only a PI reconf (cause PI invalid) 
                 vl_PI = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPiFromFreq(vl_tunedFrequency);
                 if (DABMW_INVALID_RDS_PI_VALUE != vl_PI)
                     {
                     DABMW_ServiceFollowing_ExtInt_IncreasePiMatchingAF(DABMW_SF_BackgroundScanInfo.bgScan_App, vl_tunedFrequency, vl_PI);                
                     }
                 else if (DABMW_ServiceFollowing_IsFrequencyInAlternateFM(vl_tunedFrequency))
                     {
                     DABMW_ServiceFollowing_ExtInt_IncreasePiMatchingAF(DABMW_SF_BackgroundScanInfo.bgScan_App, vl_tunedFrequency, DABMW_SF_BackgroundScanInfo.searchedServiceID);
                     }
                     
                 
                 DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_PI);
                 DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 
                }
             else 
                {
                    // we consider quality is not good enough for decoding the PI...
                    // 
                    vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
                    /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) :  Frequency skiped, quality below threshold to decode PI\n");

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
                    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 
                }
             
			 }
		 else 
		 	{
		 	vl_succesfulSearched = DABMW_ServiceFollowing_GetSearchResult(DABMW_serviceFollowingStatus.currentSid,
																false, // bearer is not DAB
																true); // ID received emulate because valid

             /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) :  Frequency known PI 0x%04x, searched %s\n", 
                                                                            DABMW_serviceFollowingStatus.currentSid,
                                                                            ((vl_succesfulSearched!=DABMW_SF_BG_RESULT_NOT_FOUND)?"SUCCESS":"FAILURE"));

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
             /* END TMP LOG */

            if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
				{
                // we have a frequency with good PI    
                /* Should we check for the quality ?    */
                /* check the quality : if an alternate is configured in FM already, we should look for an better that current                         
                              * state here should be : AF Scan
                            */
                /* if we have an alternate, check if this one is different and better */
                if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.alternateFrequency)    
                    {             

                    // Get quality on orignial app 
                    // 

	                    // measure the current frequency on same app than alternate for far comarison
	                     // 
	                     DABMW_ServiceFollowing_Measure_AF_FMQuality_OnAlternatePath(); 
	                    
	                    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
	                        {

	                            
	                        /* if same freq than alternate, skip */
	                        if (DABMW_serviceFollowingStatus.alternateFrequency == DABMW_SF_BackgroundScanInfo.bg_Freq)
	                            {
	                            /* the alternate found is not better than current alternate : 
	                                                 * continue the seach 
	                                                */
								 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) : this is alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	   
	                            vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;
								// do not reconfirm the alternate : it should be done during the bg check.
								vl_PiReconfirmationNeeded = false;

	                            }
	                        else if (false == DABMW_ServiceFollowing_qualityA_better_qualityB_FM(DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath, DABMW_serviceFollowingStatus.alternate_Quality,
	                                                                                    DABMW_serviceFollowingStatus.currentFrequency, DABMW_serviceFollowingStatus.alternateFrequency))
	                            {
	                            /* the alternate found is not better than current alternate : 
	                                                   * continue the seach 
	                                                   */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) : this is not as good as alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	   
	                            vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;
	                            } 
							else
								{
								// reconfirm the PI before switch
								vl_PiReconfirmationNeeded = true;
								}
	                        }
	                    else if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp))
	                        {
	                        // alternate is DAB : but maybe this one is better :)
	                        // 
	                        //alternate is DAB, current if FM

	                         if (true == DABMW_ServiceFollowing_qualityA_DAB_better_qualityB_FM(DABMW_serviceFollowingStatus.alternate_Quality, DABMW_serviceFollowingStatus.current_Quality, 
	                                                                                       DABMW_serviceFollowingStatus.alternateFrequency, DABMW_serviceFollowingStatus.currentFrequency))
	                            {
	                            /* the alternate found is not better than current alternate : 
	                                                   * continue the seach 
	                                                   */
	                            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) : this is not as good as alternate because DAB preferred --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	                             /* END TMP LOG */
	                             vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;
	                                
	                            }
							 else
							 	{
							 	// reconfirm the PI before switch
							 	vl_PiReconfirmationNeeded = true;
							 	}
	                        }
	                    }                             
            	}
 
			// reconfirm the PI
			// either on the alternate either before switch
			//
			if ((true == vl_PiReconfirmationNeeded)
				&&
				(true == DABMW_ServiceFollowing_IsFMQualityEnoughForRdsDecoding(DABMW_serviceFollowingStatus.current_Quality.fmQuality))
				)	
				{
					// In case PI is perturbated 
					// and to be sure before switching PI is confirmed.
					// force PI reading...
					//

						// 
						// Increase PI matching chance
						DABMW_ServiceFollowing_ExtInt_IncreasePiMatchingAF(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq, DABMW_SF_BackgroundScanInfo.searchedServiceID);
										
#if defined(CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
						DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (vl_currentFreq, NULL, &DABMW_ServiceFollowing_OnPI_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
					
						DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
					
						/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
					   DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,	   
												 "DABMW_Service_Following (Seek Processing) : Freq = %d, Force to read PI again\n",
												  DABMW_serviceFollowingStatus.currentFrequency);  
										
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
						/* END TMP LOG */

						vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_PI);
						DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 

				}
			else
				{
				if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
					{
					 /* PI at that freq is valid, and the right one
					 */
					 DABMW_SF_BackgroundScanInfo.succesfulSearched = vl_succesfulSearched;

	                 /* PI found or confirmed
					 * this marks the end of scan
					 */
					 // end the seek procedure if it was on going.
	                 DABMW_ServiceFollowing_EndBackgroundScan();
	                 
					 DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
	                 DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 
	                 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                         DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) : Freq SUCCESS \n");                         
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	                 /* END TMP LOG */


					 }
				 else
					 {
					 /* this is not the right PI 
					 * move next scan			 
					 * this is decided in next iteration to continue background scan (else we may lock the task ..) 
					 * so that task is rescheduled meanwhile....
					 */
					 
					 // if we are in landscape scan : request the PS 
					 // 
					if (true == DABMW_ServiceFollowing_IsLanscapeScanOnGoing())
						{
						// we are in landscape scan : should we wait for the PS ? 
						//
						if (NULL == DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPsFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq))
						 	{
						 	// there is not decoded PS yet
						 	// wait for the PS before moving forward
#if defined(CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
							DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPsAtFreq(vl_currentFreq, NULL, &DABMW_ServiceFollowing_OnPS_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)

						 	DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_PS);
		                 	DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 

							/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
							DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Seek_Processing) : Go in WAIT_PS \n"); 	   
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
				/* END TMP LOG */

						 	}
						 else
						 	{
						 	/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
							DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Seek_Processing) : PS known %s -> continue \n", DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPsFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq)); 	   
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							/* END TMP LOG */

						 	DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
		                 	DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 
						 	}
						}
					else
						{
							// we are not in landscape scan : just continue
							//
							/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
							DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) : Freq skipped \n");							 
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							 /* END TMP LOG */

						 	DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
		                 	DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 
						}
					 }
				}
		 	}		 
		 }
	 else
		 {
		 /* it means either that no frequency is found by seek or frequency found is bad quality.
		 */
		 /* if bad quality : seek/tune should conitnue on next background scan iteration
		 * if no frequency : this mark the end of this band scan
		 * proposition is that the end of backround scan is decided next iteration in any case
		 * so that task is rescheduled meanwhile....
		 *
		 */
		 
		 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SeekProcessing) :  Frequency not ok\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
         /* END TMP LOG */
		 DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
         DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN); 

		 }
			
          /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    

	return OSAL_OK;
}

/* Procedure which prepare the Alternate Freq in FM for a given PI
* returns
* --> OSAL_ERROR if no frequency found
* -->  
* else return number of frequency added to AF list
*/

tSInt DABMW_ServiceFollowing_BuildAlternateFrequency_FM(tU32 vI_PiAF, tU32 vI_freqToRemove1, tU32 vI_freqToRemove2)
{
	tSInt vl_numFreq = 0;
	tPU32 pl_listFreq;
	tPU8 pl_listAfFreq;
	tU8 vl_cnt, vl_cnt2, vl_cnt3;
	tU8 vl_numFreqStored = 0, vl_tmp_numFreqFM = 0;
	tU32 vl_tmp_freq;
	tBool vl_freq_in_landscape;
    tU32 vl_frequencyForAf;

	/* Init structure for FM 
	*/
	DABMW_ServiceFollowing_InitAlternateFrequency(true, false);

// Add feature to lock on a frequency
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    if (true == DABMW_serviceFollowingData.lockFeatureActivated_FM)
    {
        // check if lock Freq 1 is FM
        if (true == DABMW_ServiceFollowing_ExtInt_IsFMBand(DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(DABMW_serviceFollowingData.lockFrequency_1)))
            {
            // check it is not to be removed
            if ((DABMW_serviceFollowingData.lockFrequency_1 != vI_freqToRemove1)
                && (DABMW_serviceFollowingData.lockFrequency_1 != vI_freqToRemove2))
                {
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = DABMW_serviceFollowingData.lockFrequency_1;
                vl_numFreq++;
                }
            }

        // check if lock Freq 2 is FM
        if (true == DABMW_ServiceFollowing_ExtInt_IsFMBand(DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(DABMW_serviceFollowingData.lockFrequency_2)))
            {
            // check it is not to be removed    
            if ((DABMW_serviceFollowingData.lockFrequency_2 != vI_freqToRemove1)
                && (DABMW_serviceFollowingData.lockFrequency_2 != vI_freqToRemove2))
                {
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = DABMW_serviceFollowingData.lockFrequency_2;
                vl_numFreq++;
                }
            }
        
        // we are lock in FM
        // stop here
        if (vl_numFreq > 0)
		    {
		    DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_STORED;
		    }
	
	    DABMW_SF_freqCheckList.numFMFreqForCheck = vl_numFreq;
        
        return vl_numFreq;
    }
#endif


    // we have a valid PI, let's search for it in database
  	vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(vI_PiAF);
 
	if (vl_numFreqStored > 0)
		{

		pl_listFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * vl_numFreqStored);

		if (NULL == pl_listFreq )
			{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)  
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
			}
					
		vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList(pl_listFreq, vI_PiAF, vl_numFreqStored);
		vl_numFreq = 0;

		/* loop to store retrieve frequency... 
		* make sure not to go further than the freq check array as well 
		*/
		for (vl_cnt=0;(vl_cnt < vl_numFreqStored) && (vl_numFreq < DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF); vl_cnt++)
			{

				DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = pl_listFreq[vl_cnt];
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inLandscape = true;

                if (pl_listFreq[vl_cnt] == vI_freqToRemove1)
    				{
                        // mark it as original
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                    }
                else if (pl_listFreq[vl_cnt] == vI_freqToRemove2)
                    {
                        // mark it as alternate
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE; 
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                    }
                
				vl_numFreq++;


                        
			}
		
		DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listFreq);
		
		}

	/*  add all implicit hard link  */
    /* EPR change : check that we are still in the range allowed for the array */
	for (vl_cnt=0;((vl_cnt < DABMW_SF_BackgroundScanInfo.nb_LinkId_FM) && (vl_numFreq < DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF)); vl_cnt++)
		{
		/* get the frequency */
		//		vl_tmp_freq = DABMW_ServiceFollowing_LinkInfo_GetFrequencyFromId(DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id, true);
		vl_tmp_freq = DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].frequency;
        
		if (DABMW_INVALID_FREQUENCY != vl_tmp_freq) 
			{
            // avoid several time the frequency : check not stored already
            /* loop to check if frequency not already stored */
            vl_freq_in_landscape = false;
            
    		for (vl_cnt2=0;vl_cnt2 < vl_numFreq; vl_cnt2++)
			    {	
    			if (DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].frequency == vl_tmp_freq)
    				{
    				vl_freq_in_landscape = true;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].statusFlagInfo.field.is_inServiceLinking = true;

                    /* set info to store if PI in landscape */
                    DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].IdIsInLandscape = true;
    				break;
    				}
			    }

            /* if freq not in landscape : add it */
		    if (false == vl_freq_in_landscape)
			    {
    			DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = vl_tmp_freq;	
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inServiceLinking = true;
                
                if (vl_tmp_freq == vI_freqToRemove1)
    				{
                          // mark it as original
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;      
                    }
                else if (vl_tmp_freq == vI_freqToRemove2)
                    {
                          // mark it as alternate
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE; 
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;      
                    }
                
    			vl_numFreq++;
    			}

			/* set info to store if PI in landscape */
 
			}
        // EPR ADDITION : if no frequency in link id list : may be this is in landscape....
        //
        else if (DABMW_INVALID_FREQUENCY == vl_tmp_freq)
            {
                // check if information for the given PI exist in landscape
                //
                // we have a valid PI, let's search for it in database
              	vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id);
             
            	if (vl_numFreqStored > 0)
            		{

            		pl_listFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * vl_numFreqStored);

            		if (NULL == pl_listFreq )
            			{
            											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
            			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif 
// defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
            							
            			return OSAL_ERROR;
            			}
            					
            		vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList(pl_listFreq, DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id, vl_numFreqStored);
            		vl_numFreq = 0;

            		/* loop to store retrieve frequency... 
            		                * make sure not to go further than the freq check array as well 
            		                */
            		for (vl_cnt=0;(vl_cnt < vl_numFreqStored) && (vl_numFreq < DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF); vl_cnt++)
            			{
            			DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = pl_listFreq[vl_cnt];
                        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inLandscape = true;

                        if (pl_listFreq[vl_cnt] == vI_freqToRemove1)
                            {
                                  // mark it as original
                                // and already checked : nothing need to recheck
                            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;      
                            }
                        else if (pl_listFreq[vl_cnt] == vI_freqToRemove2)
                            {
                                  // mark it as alternate
                                // and already checked : nothing need to recheck
                            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE;
                            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;      
                            }
                
            			vl_numFreq++;
		
            			}

            		
            		DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listFreq);
            		
            		}
            }
		}

	/* add FM alternate frequency if known */


	/* allocate list to receive the data */
	pl_listAfFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);

  	if (NULL == pl_listAfFreq )
		{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif 
//defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
		}
	
	/* retrieve the data in database 
        * the answer is formatted with : 
        * then we get the AF : each one being an U8 which is a code as a number of 0.1 MHz steps toward 87500 
        *
	 */

    /* CHANGE :     
        * We should provide the original frequency if existing else we get the 1st AF list stored from the PI...
        *    may not be the one of current freq...
        * optimisation could be : compiled a set of AF for all history...
        */
        
	/* vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 
										DABMW_SF_BackgroundScanInfo.searchedServiceID, 
										DABMW_INVALID_FREQUENCY, 
										DABMW_AF_LIST_BFR_LEN);

        */
    // Improvement : 
    // if orignal is FM : take AF from original
    // if original is DAB : then try AF of the alternate if Alternate is FM
    //else (DAB - DAB or no alternate) : try 1st value in database 
    //
    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.originalAudioUserApp))
    { 
        vl_frequencyForAf = DABMW_serviceFollowingStatus.originalFrequency;
    }
    else if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
    {
        vl_frequencyForAf = DABMW_serviceFollowingStatus.alternateFrequency;
    }
    else
    {
        vl_frequencyForAf = DABMW_INVALID_FREQUENCY;
    }

    
    if (DABMW_INVALID_FREQUENCY != vl_frequencyForAf)
    {

  		 vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 
			 // EPR CHANGE : should be the requested PI for building instead of
			 // the 'searched service ID' which may not yet been init ....
			 // 
			 // BEFORE
			 //						DABMW_SF_BackgroundScanInfo.searchedServiceID,
		     // AFTER
		     						vI_PiAF,
		     // END CHANGE 
									vl_frequencyForAf, 
									DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);	

        /* END CHANGE */
        

    /* loop to store retrieve frequency... 
    	* make sure not to go further than the freq check array as well 
    	* make sure not to store twice same freq as well... 
    	*/
    	for (vl_cnt=0;(vl_cnt < vl_numFreqStored) && (vl_numFreq < DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF); vl_cnt++)
    		{
    		vl_freq_in_landscape = false;

    		/* convert the value in frequency
        		* from section 3.2.1.6.1 AF code tables of IEC62106 5RDS standard 
        		* it corresponds to the indice of freq from 87500 + step by 100 
        		*/
    		vl_tmp_freq = 87500 + (100 * pl_listAfFreq[vl_cnt]);

    		/* loop to check if frequency not already stored */
    		for (vl_cnt2=0;vl_cnt2 < vl_numFreq; vl_cnt2++)
    			{	
    			if (DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].frequency == vl_tmp_freq)
    				{
    				vl_freq_in_landscape = true;
                    if (vl_frequencyForAf == DABMW_serviceFollowingStatus.originalFrequency)
                    {
                        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].statusFlagInfo.field.is_inAFList = true;
                    }
                    else
                    {
                        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].statusFlagInfo.field.is_inAFListFromAlternate = true;
                    }
                      
    				break;
    				}
    			}
    			
    		/* if freq not in landscape and not the freq to remove : add it */
    		if (false == vl_freq_in_landscape) 
    			{
    			DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = vl_tmp_freq;	
                
                if (vl_frequencyForAf == DABMW_serviceFollowingStatus.originalFrequency)
                    {
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inAFList = true;
                    }
                else
                    {
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inAFListFromAlternate = true;
                    }

                if (vl_tmp_freq == vI_freqToRemove1)
    				{
                          // mark it as original
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;      
                    }
                else if (vl_tmp_freq == vI_freqToRemove2)
                    {
                          // mark it as alternate
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;      
                    }
                

    			vl_numFreq++;
    			}	

    		}
    }
		
	DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listAfFreq);


    // EPR Change : complete the list with other known frequency in the history, 
    // in the future : order by 'storing time ?'
    //
    
    vl_tmp_numFreqFM = vl_numFreq; 

    
    //
    // 1st, start by the alternate if it is fm, ie "freq to remove"
    //

    if (DABMW_INVALID_FREQUENCY != vI_freqToRemove2)
    {
         /* allocate list to receive the data */
    
        pl_listAfFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);

  	    if (NULL == pl_listAfFreq )
		{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
		}

	   	// EPR CHANGE : should be the requested PI for building instead of
    	// the 'searched service ID' which may not yet been init ....
    	// 
    	// BEFORE
    	//
        vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 
		// EPR CHANGE : should be the requested PI for building instead of
		// the 'searched service ID' which may not yet been init ....
		// 
		// BEFORE
		//					   DABMW_SF_BackgroundScanInfo.searchedServiceID,
		// AFTER
									   vI_PiAF,
		// END CHANGE 

										vI_freqToRemove2, 
										DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);	



        /* loop to store retrieve frequency... 
        	* make sure not to go further than the freq check array as well 
        	* make sure not to store twice same freq as well... 
        	*/
        for (vl_cnt=0;(vl_cnt < vl_numFreqStored) && (vl_numFreq < DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF); vl_cnt++)
            {
        	vl_freq_in_landscape = false;

        	/* convert the value in frequency
        		* from section 3.2.1.6.1 AF code tables of IEC62106 5RDS standard 
        		* it corresponds to the indice of freq from 87500 + step by 100 
        		*/
        	vl_tmp_freq = 87500 + (100 * pl_listAfFreq[vl_cnt]);

        	/* loop to check if frequency not already stored */
        	for (vl_cnt2=0;vl_cnt2 < vl_numFreq; vl_cnt2++)
        	    {	
        		if (DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].frequency == vl_tmp_freq)
        		    {
        			vl_freq_in_landscape = true;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].statusFlagInfo.field.is_inAFListFromAlternate = true;
        			break;
        			}
        		}
        			
        	/* if freq not in landscape and not the freq to remove : add it */
        	if (false == vl_freq_in_landscape) 
        	    {
        		DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = vl_tmp_freq;	
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inAFListFromAlternate = true;

                if (vl_tmp_freq == vI_freqToRemove1)
    				{
                          // mark it as original
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;      
                    }
                else if (vl_tmp_freq == vI_freqToRemove2)
                    {
                          // mark it as alternate
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE; 
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                    }
                
        		vl_numFreq++;
        		}	
                        
        	}
        		
        	DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listAfFreq);
    //
    }
    
    // Now, loop on all existing AF in landscape...
    // ...
    // 
    
    for (vl_cnt=0;vl_cnt<vl_tmp_numFreqFM;vl_cnt++)
        {
         /* allocate list to receive the data */

  	    pl_listAfFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);

  	    if (NULL == pl_listAfFreq )
		{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)  
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
		}
    
        vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 	
			// EPR CHANGE : should be the requested PI for building instead of
			// the 'searched service ID' which may not yet been init ....
			// 
			// BEFORE
			//					   DABMW_SF_BackgroundScanInfo.searchedServiceID,
			// AFTER
									   vI_PiAF,
			// END CHANGE 

										DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].frequency, 
										DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);


        /* loop to store retrieve frequency... 
        	* make sure not to go further than the freq check array as well 
        	* make sure not to store twice same freq as well... 
        	*/
        for (vl_cnt2=0;(vl_cnt2 < vl_numFreqStored) && (vl_numFreq < DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF); vl_cnt2++)
            {
        	vl_freq_in_landscape = false;

        	/* convert the value in frequency
        		* from section 3.2.1.6.1 AF code tables of IEC62106 5RDS standard 
        		* it corresponds to the indice of freq from 87500 + step by 100 
        		*/
        	vl_tmp_freq = 87500 + (100 * pl_listAfFreq[vl_cnt2]);

        	/* loop to check if frequency not already stored */
        	for (vl_cnt3=0;vl_cnt3 < vl_numFreq; vl_cnt3++)
        	    {	
        		if (DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt3].frequency == vl_tmp_freq)
        		    {
                    // freq is in the landscape
        			vl_freq_in_landscape = true;
                    // set the flag 
                    // if not the requested one
                    // but we should not update the status : this is the AF list of the same freq...
                    if (DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].frequency != vl_tmp_freq)
                    {
                        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt3].statusFlagInfo.field.is_inAFListFromOtherFrequency = true;
                    }
        			break;
        			}
        		}
        			
        	/* if freq not in landscape and not the freq to remove : add it */
        	if (false == vl_freq_in_landscape) 
        	    {
        		DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].frequency = vl_tmp_freq;	
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inAFListFromOtherFrequency = true;

                 if (vl_tmp_freq == vI_freqToRemove1)
    				{
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;

                    }
                else if (vl_tmp_freq == vI_freqToRemove2)
                    {
                          // mark it as alternate
                        // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE;   
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                    }
                
        		vl_numFreq++;
        		}	
            
        	}
        		
        	DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listAfFreq);
            
        }
        
	if (vl_numFreq > 0)
		{
		DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_STORED;
		}
	
	DABMW_SF_freqCheckList.numFMFreqForCheck = vl_numFreq;

    //update the alternate source information 
    //where does it come from ?
    //
    // ?? do we do it periodically ?? or only when alternate is set ??
    //
    
    DABMW_ServiceFollowing_SetAlternateFrequency_SourceInfo();

	return vl_numFreq;
}

/* Procedure which set the Alternate Freq in FM source infos
* returns
* --> OSAL_ERROR if no frequency found
* -->  
* else return number of frequency added to AF list
*/

tSInt DABMW_ServiceFollowing_SetAlternateFrequency_SourceInfo()
{
	tPU32 pl_listFreq;
	tPU8 pl_listAfFreq;
	tU8 vl_cnt, vl_cnt2;
	tU8 vl_numFreqStored = 0;
	tU32 vl_tmp_freq;
    tSInt vl_res = OSAL_OK;

    DABMW_serviceFollowingStatus.alternateSource.value = 0x00;


    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp))
    {
       DABMW_serviceFollowingStatus.alternateSource.field.is_inLandscape = true;
       return OSAL_OK;
    }
    else if (false == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
    {
        return OSAL_ERROR;
    }
    

    // we have a valid PI, let's search for it in database
  	vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(DABMW_serviceFollowingStatus.alternateSid);
 
	if (vl_numFreqStored > 0)
		{
		pl_listFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * vl_numFreqStored);

		if (NULL == pl_listFreq )
			{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
			}
					
		vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList(pl_listFreq, DABMW_serviceFollowingStatus.alternateSid, vl_numFreqStored);

		/* loop to store retrieve frequency... 
		* make sure not to go further than the freq check array as well 
		*/
		for (vl_cnt=0;(vl_cnt < vl_numFreqStored) ; vl_cnt++)
			{
            // set the Alternate sourceinfo
             if (pl_listFreq[vl_cnt] == DABMW_serviceFollowingStatus.alternateFrequency)
                {
                     DABMW_serviceFollowingStatus.alternateSource.field.is_inLandscape = true;
                }                     
			}
		
		DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listFreq);
		
		}

	/*  add all implicit hard link  */
    /* EPR change : check that we are still in the range allowed for the array */
	for (vl_cnt=0;(vl_cnt < DABMW_SF_BackgroundScanInfo.nb_LinkId_FM); vl_cnt++)
		{
		/* get the frequency */
		//		vl_tmp_freq = DABMW_ServiceFollowing_LinkInfo_GetFrequencyFromId(DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id, true);
		vl_tmp_freq = DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].frequency;
        
		if (DABMW_INVALID_FREQUENCY != vl_tmp_freq) 
			{
            // avoid several time the frequency : check not stored already
            /* loop to check if frequency not already stored */
   

            // set the Alternate sourceinfo
             if (vl_tmp_freq == DABMW_serviceFollowingStatus.alternateFrequency)
                {
                     DABMW_serviceFollowingStatus.alternateSource.field.is_inServiceLinking = true;
                }

			}
        // EPR ADDITION : if no frequency in link id list : may be this is in landscape....
        //
        else if (DABMW_INVALID_FREQUENCY == vl_tmp_freq)
            {
                // check if information for the given PI exist in landscape
                //
                // we have a valid PI, let's search for it in database
              	vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id);
             
            	if (vl_numFreqStored > 0)
            		{

            		pl_listFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * vl_numFreqStored);

            		if (NULL == pl_listFreq )
            			{
            											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
            			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
            							
            			return OSAL_ERROR;
            			}
            					
            		vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList(pl_listFreq, DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id, vl_numFreqStored);

            		/* loop to store retrieve frequency... 
            		                * make sure not to go further than the freq check array as well 
            		                */
            		for (vl_cnt=0;(vl_cnt < vl_numFreqStored) ; vl_cnt++)
            			{
 
                         // set the Alternate sourceinfo
                         if (vl_tmp_freq == DABMW_serviceFollowingStatus.alternateFrequency)
                            {
                                 DABMW_serviceFollowingStatus.alternateSource.field.is_inLandscape = true;
                            }		
            			}
            		
            		DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listFreq);
            		
            		}
            }
		}

	/* add FM alternate frequency if known */


	/* allocate list to receive the data */
	pl_listAfFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);

  	if (NULL == pl_listAfFreq )
		{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
		}
	
	/* retrieve the data in database 
        * the answer is formatted with : 
        * then we get the AF : each one being an U8 which is a code as a number of 0.1 MHz steps toward 87500 
        *
	 */

    /* CHANGE :     
        * We should provide the original frequency if existing else we get the 1st AF list stored from the PI...
        *    may not be the one of current freq...
        * optimisation could be : compiled a set of AF for all history...
        */
        
	/* vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 
										DABMW_SF_BackgroundScanInfo.searchedServiceID, 
										DABMW_INVALID_FREQUENCY, 
										DABMW_AF_LIST_BFR_LEN);

        */
    // check if part of original AF list
    //
    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.originalAudioUserApp))
    { 

         vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 
    										DABMW_SF_BackgroundScanInfo.searchedServiceID,
    										DABMW_serviceFollowingStatus.originalFrequency, 
    										DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);
        /* END CHANGE */
        

    	/* loop to store retrieve frequency... 
        	* make sure not to go further than the freq check array as well 
    	        * make sure not to store twice same freq as well...             
    	        */
    	for (vl_cnt=0;(vl_cnt < vl_numFreqStored); vl_cnt++)
    		{

    		/* convert the value in frequency
    		        * from section 3.2.1.6.1 AF code tables of IEC62106 5RDS standard 
    		        * it corresponds to the indice of freq from 87500 + step by 100 
    		        */
    		vl_tmp_freq = 87500 + (100 * pl_listAfFreq[vl_cnt]);

            // set the Alternate sourceinfo
            if (vl_tmp_freq == DABMW_serviceFollowingStatus.alternateFrequency)
                {                  
                    DABMW_serviceFollowingStatus.alternateSource.field.is_inAFList = true;
                }
    		}
        
    }
    
	DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listAfFreq);


    // EPR Change : complete the list with other known frequency in the history, 
    // in the future : order by 'storing time ?'
    //
    
    
    // Now, loop on all existing AF in landscape...
    // ...
    // 
    
    for (vl_cnt2=0;vl_cnt2<DABMW_SF_freqCheckList.numFMFreqForCheck;vl_cnt2++)
        {
         /* allocate list to receive the data */

  	    pl_listAfFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);

  	    if (NULL == pl_listAfFreq )
		{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
		}
    
        vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 
										DABMW_SF_BackgroundScanInfo.searchedServiceID,
										DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].frequency, 
										DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);


        /* loop to store retrieve frequency... 
        	* make sure not to go further than the freq check array as well 
        	* make sure not to store twice same freq as well... 
        	*/
        for (vl_cnt=0;(vl_cnt < vl_numFreqStored); vl_cnt++)
            {
    
        	/* convert the value in frequency
        		* from section 3.2.1.6.1 AF code tables of IEC62106 5RDS standard 
        		* it corresponds to the indice of freq from 87500 + step by 100 
        		*/
        	vl_tmp_freq = 87500 + (100 * pl_listAfFreq[vl_cnt]);

            
            // set the Alternate sourceinfo
            if ((vl_tmp_freq == DABMW_serviceFollowingStatus.alternateFrequency) 
                && (DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].frequency != vl_tmp_freq))
                {
                DABMW_serviceFollowingStatus.alternateSource.field.is_inAFListFromOtherFrequency = true;
                }

            
        	}
        		
        	DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listAfFreq);
            
        }
        

	return vl_res;
}

/* Procedure which set the original Freq in FM source infos
* returns
* --> OSAL_ERROR if no frequency found
* -->  
* else return number of frequency added to AF list
*/

tSInt DABMW_ServiceFollowing_SetOriginalFrequency_SourceInfo()
{
	tPU32 pl_listFreq;
	tPU8 pl_listAfFreq;
	tU8 vl_cnt, vl_cnt2;
	tU8 vl_numFreqStored = 0;
	tU32 vl_tmp_freq;
    tSInt vl_res = OSAL_OK;

    DABMW_serviceFollowingStatus.originalSource.value = 0x00;


    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp))
    {
       DABMW_serviceFollowingStatus.originalSource.field.is_inLandscape = true;
       return OSAL_OK;
    }
    else if (false == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.originalAudioUserApp))
    {
        return OSAL_ERROR;
    }
    

    // we have a valid PI, let's search for it in database
  	vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(DABMW_serviceFollowingStatus.originalSid);
 
	if (vl_numFreqStored > 0)
		{
		pl_listFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * vl_numFreqStored);

		if (NULL == pl_listFreq )
			{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
			}
					
		vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList(pl_listFreq, DABMW_serviceFollowingStatus.originalSid, vl_numFreqStored);

		/* loop to store retrieve frequency... 
		* make sure not to go further than the freq check array as well 
		*/
		for (vl_cnt=0;(vl_cnt < vl_numFreqStored) ; vl_cnt++)
			{
            // set the Alternate sourceinfo
             if (pl_listFreq[vl_cnt] == DABMW_serviceFollowingStatus.originalFrequency)
                {
                     DABMW_serviceFollowingStatus.originalSource.field.is_inLandscape = true;
                }                     
			}
		
		DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listFreq);
		
		}

	/*  add all implicit hard link  */
    /* EPR change : check that we are still in the range allowed for the array */
	for (vl_cnt=0;(vl_cnt < DABMW_SF_BackgroundScanInfo.nb_LinkId_FM); vl_cnt++)
		{
		/* get the frequency */
		//		vl_tmp_freq = DABMW_ServiceFollowing_LinkInfo_GetFrequencyFromId(DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id, true);
		vl_tmp_freq = DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].frequency;
        
		if (DABMW_INVALID_FREQUENCY != vl_tmp_freq) 
			{
            // avoid several time the frequency : check not stored already
            /* loop to check if frequency not already stored */
   

            // set the Alternate sourceinfo
             if (vl_tmp_freq == DABMW_serviceFollowingStatus.originalFrequency)
                {
                     DABMW_serviceFollowingStatus.originalSource.field.is_inServiceLinking = true;
                }

			}
        // EPR ADDITION : if no frequency in link id list : may be this is in landscape....
        //
        else if (DABMW_INVALID_FREQUENCY == vl_tmp_freq)
            {
                // check if information for the given PI exist in landscape
                //
                // we have a valid PI, let's search for it in database
              	vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id);
             
            	if (vl_numFreqStored > 0)
            		{

            		pl_listFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * vl_numFreqStored);

            		if (NULL == pl_listFreq )
            			{
            											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
            			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
            							
            			return OSAL_ERROR;
            			}
            					
            		vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList(pl_listFreq, DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_cnt].id, vl_numFreqStored);

            		/* loop to store retrieve frequency... 
            		                * make sure not to go further than the freq check array as well 
            		                */
            		for (vl_cnt=0;(vl_cnt < vl_numFreqStored) ; vl_cnt++)
            			{
 
                         // set the Alternate sourceinfo
                         if (vl_tmp_freq == DABMW_serviceFollowingStatus.originalFrequency)
                            {
                                 DABMW_serviceFollowingStatus.originalSource.field.is_inLandscape = true;
                            }		
            			}
            		
            		DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listFreq);
            		
            		}
            }
		}

	/* add FM alternate frequency if known */


	/* allocate list to receive the data */
	pl_listAfFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);

  	if (NULL == pl_listAfFreq )
		{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
		}
	
	/* retrieve the data in database 
        * the answer is formatted with : 
        * then we get the AF : each one being an U8 which is a code as a number of 0.1 MHz steps toward 87500 
        *
	 */

    /* CHANGE :     
        * We should provide the original frequency if existing else we get the 1st AF list stored from the PI...
        *    may not be the one of current freq...
        * optimisation could be : compiled a set of AF for all history...
        */
        
	/* vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 
										DABMW_SF_BackgroundScanInfo.searchedServiceID, 
										DABMW_INVALID_FREQUENCY, 
										DABMW_AF_LIST_BFR_LEN);

        */
    // check if part of alternate AF list
    //
    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
    { 

         vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 	
    										DABMW_SF_BackgroundScanInfo.searchedServiceID,
    										DABMW_serviceFollowingStatus.alternateFrequency, 
    										DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);
        /* END CHANGE */
        

    	/* loop to store retrieve frequency... 
        	* make sure not to go further than the freq check array as well 
    	        * make sure not to store twice same freq as well...             
    	        */
    	for (vl_cnt=0;(vl_cnt < vl_numFreqStored); vl_cnt++)
    		{

    		/* convert the value in frequency
    		        * from section 3.2.1.6.1 AF code tables of IEC62106 5RDS standard 
    		        * it corresponds to the indice of freq from 87500 + step by 100 
    		        */
    		vl_tmp_freq = 87500 + (100 * pl_listAfFreq[vl_cnt]);

            // set the Alternate sourceinfo
            if (vl_tmp_freq == DABMW_serviceFollowingStatus.originalFrequency)
                {                  
                    DABMW_serviceFollowingStatus.originalSource.field.is_inAFListFromAlternate= true;
                }
    		}
        
    }
    
	DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listAfFreq);


    // EPR Change : complete the list with other known frequency in the history, 
    // in the future : order by 'storing time ?'
    //
    
    
    // Now, loop on all existing AF in landscape...
    // ...
    // 
    
    for (vl_cnt2=0;vl_cnt2<DABMW_SF_freqCheckList.numFMFreqForCheck;vl_cnt2++)
        {
         /* allocate list to receive the data */

  	    pl_listAfFreq = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(tU32) * DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);

  	    if (NULL == pl_listAfFreq )
		{
											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
							
			return OSAL_ERROR;
		}
    
        vl_numFreqStored = DABMW_ServiceFollowing_ExtInt_RdsGetAfList(pl_listAfFreq, 	
										DABMW_SF_BackgroundScanInfo.searchedServiceID,
										DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].frequency, 
										DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF);


        /* loop to store retrieve frequency... 
        	* make sure not to go further than the freq check array as well 
        	* make sure not to store twice same freq as well... 
        	*/
        for (vl_cnt=0;(vl_cnt < vl_numFreqStored); vl_cnt++)
            {
    
        	/* convert the value in frequency
        		* from section 3.2.1.6.1 AF code tables of IEC62106 5RDS standard 
        		* it corresponds to the indice of freq from 87500 + step by 100 
        		*/
        	vl_tmp_freq = 87500 + (100 * pl_listAfFreq[vl_cnt]);

            
            // set the Alternate sourceinfo
            if ((vl_tmp_freq == DABMW_serviceFollowingStatus.originalFrequency) 
                && (DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt2].frequency != vl_tmp_freq))
                {
                DABMW_serviceFollowingStatus.originalSource.field.is_inAFListFromOtherFrequency = true;
                }

            
        	}
        		
        	DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_listAfFreq);
            
        }
        

	return vl_res;
}

/* Procedure which prepare the Alternate Freq in DAB for a given Sid
* returns
* --> OSAL_ERROR if no frequency found
* -->  
* else return number of frequency added to AF list
*/

tSInt DABMW_ServiceFollowing_BuildAlternateFrequency_DAB(tU32 vI_Sid, tU32 vI_freqToRemove1, tU32 vI_freqToRemove2 )
{
    tSInt vl_ensembleNum;
    tSInt vl_serviceNum;
    DABMW_ensembleUniqueIdTy *pl_ensembleIdsPtr, *pl_ensembleIdsPtr_2 ;
    tU32 *pl_serviceList, *pl_serviceList_2; 
 
 	tSInt vl_eidCnt, vl_sidCnt, vl_alternateFreq, vl_cnt2;
	tU32 vl_ensembleUniqueId;
	tSInt vl_numFreq = 0;
	tU32 vl_tmp_freq;
    tBool vl_freq_in_landscape = false;


	/* Init the DAB structure
	*/
	DABMW_ServiceFollowing_InitAlternateFrequency(false, true);

    // Add feature to lock on a frequency
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
        if (true == DABMW_serviceFollowingData.lockFeatureActivated_DAB)
        {
            // check if lock Freq 1 is FM
            if (true == DABMW_ServiceFollowing_ExtInt_IsDABBand(DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(DABMW_serviceFollowingData.lockFrequency_1)))
                {
                // check it is not to be removed
                if ((DABMW_serviceFollowingData.lockFrequency_1 != vI_freqToRemove1)
                    && (DABMW_serviceFollowingData.lockFrequency_1 != vI_freqToRemove2))
                    {
                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].frequency = DABMW_serviceFollowingData.lockFrequency_1;
                    vl_numFreq++;
                    }
                }
    
            // check if lock Freq 2 is FM
            if (true == DABMW_ServiceFollowing_ExtInt_IsDABBand(DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(DABMW_serviceFollowingData.lockFrequency_2)))
                {
                // check it is not to be removed    
                if ((DABMW_serviceFollowingData.lockFrequency_2 != vI_freqToRemove1)
                    && (DABMW_serviceFollowingData.lockFrequency_2 != vI_freqToRemove2))
                    {
                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].frequency = DABMW_serviceFollowingData.lockFrequency_2;
                    vl_numFreq++;
                    }
                }
            
            // we are lock in DAB
            // stop here
  
          if (vl_numFreq > 0) 
		    {
		    DABMW_SF_freqCheckList.DAB_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_STORED;
		    }
	
    	    DABMW_SF_freqCheckList.numDabFreqForCheck= vl_numFreq;
 
            return vl_numFreq;
        }
#endif


	/* First start with information in Landscape */
	
	// Retrieve the ensemble number and all the SIDs in order to check if a SID 
	// equal to the requested PI is available and can be used to check quality
	vl_ensembleNum = DABMW_ServiceFollowing_ExtInt_GetEnsembleList (&pl_ensembleIdsPtr);

	pl_ensembleIdsPtr_2 = pl_ensembleIdsPtr;
		
	// if ensemble exists, parse each of those to get the service list on each ensemble and check if one match
			
	for (vl_eidCnt = 0; vl_eidCnt < vl_ensembleNum; vl_eidCnt++)
		{
			vl_ensembleUniqueId = DABMW_ENSEMBLE_UNIQUE_ID (pl_ensembleIdsPtr_2->ecc, pl_ensembleIdsPtr_2->id);
			vl_serviceNum = DABMW_ServiceFollowing_ExtInt_GetServiceList (vl_ensembleUniqueId, (tVoid **)&pl_serviceList);
			
			pl_serviceList_2 = pl_serviceList; 

			/* Store the Eid if AF check on-going */

			for (vl_sidCnt= 0; ((vl_sidCnt < vl_serviceNum) && (vl_numFreq < DABMW_FIC_MAX_SERVICES_NUMBER)); vl_sidCnt++)
				{
				if (DABMW_SF_BG_SID_NOT_EQUAL != DABMW_ServiceFollowing_CompareSidPI(vI_Sid, *pl_serviceList_2, true))
					{
					// An implicit link has been found
					// save the information if this is not a frequency to be removed.
					// check also the alternate freq

    					// An implicit link has been found, save the info
    					//
    					// note : here we should add a check to avoid duplication...
    					//
					    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].frequency = pl_ensembleIdsPtr_2->frequency;
					    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].uniqueEid = vl_ensembleUniqueId;
                        DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inLandscape = true;

                        // CHANGES : This will be done other way later
					    /*
					        //indicate the Id is present in the landscape 
					         DABMW_ServiceFollowing_LinkInfo_SetIdPresentInLandscape(vI_Sid, DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].frequency,true);
	                                    */		

                        if (pl_ensembleIdsPtr_2->frequency == vI_freqToRemove1)
        				    {
                            // mark it as original
                            // and already checked : nothing need to recheck
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                            }
                        else if (pl_ensembleIdsPtr_2->frequency == vI_freqToRemove2)
                            {
                            // mark it as alternate
                            // and already checked : nothing need to recheck
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE; 
                            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                            }
                
					    vl_numFreq++;


                    /* Check if present on alternate frequency as well */
                    for (vl_alternateFreq=0;((vl_alternateFreq<DABMW_LANDSCAPE_DB_ALT_FREQ_NUM) && (vl_numFreq < DABMW_FIC_MAX_SERVICES_NUMBER)); vl_alternateFreq++)
                        {
                        if (DABMW_INVALID_FREQUENCY != pl_ensembleIdsPtr_2->alternativeFrequencies[vl_alternateFreq])           
                                {
                                // An other implicit link has been found, save the info
                             	//
            					// note : here we should add a check to avoid duplication...
    	        				//
    					        DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].frequency = pl_ensembleIdsPtr_2->alternativeFrequencies[vl_alternateFreq];
            					DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].uniqueEid = vl_ensembleUniqueId;

                                    // CHANGES : This will be done other way later
                                    /*
        		        			              // indicate the Id is present in the landscape 
        				        	            DABMW_ServiceFollowing_LinkInfo_SetIdPresentInLandscape(vI_Sid, DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].frequency,true);
                                                                */

                                if (pl_ensembleIdsPtr_2->alternativeFrequencies[vl_alternateFreq] == vI_freqToRemove1)
                				    {
                                    // mark it as original
                                    // and already checked : nothing need to recheck
                                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                                    }
                                else if (pl_ensembleIdsPtr_2->alternativeFrequencies[vl_alternateFreq] == vI_freqToRemove2)
                                    {
                                    // mark it as alternate
                                    // and already checked : nothing need to recheck
                                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE; 
                                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                                    }
                             
            					vl_numFreq++;
                                    
                                }
                        }
                    }
                pl_serviceList_2++;
                }
		/* move to next ensemble */
		pl_ensembleIdsPtr_2++;
		}


	/* then add information not in landscape but known from service linking information */

	for (vl_sidCnt=0;((vl_sidCnt < DABMW_SF_BackgroundScanInfo.nb_LinkId_DAB) && (vl_numFreq < DABMW_FIC_MAX_SERVICES_NUMBER)); vl_sidCnt++)
		{
        // avoid several time the frequency : check not stored already
        /* loop to check if frequency not already stored */
        vl_freq_in_landscape = false;
        vl_tmp_freq = DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_sidCnt].frequency;
        
    	for (vl_cnt2=0;vl_cnt2 < vl_numFreq; vl_cnt2++)
		    {	
    		if (DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt2].frequency == vl_tmp_freq)
    			{
    			vl_freq_in_landscape = true;
                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt2].statusFlagInfo.field.is_inServiceLinking = true;

                /* set info to store if PI in landscape */
                DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_sidCnt].IdIsInLandscape = true;
    		    break;
    			}
			 }
        
		if (false == vl_freq_in_landscape)
			{
			if (DABMW_INVALID_FREQUENCY != vl_tmp_freq) 
				{
				DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].frequency = vl_tmp_freq;
				DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].uniqueEid = DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_sidCnt].eid;
                DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].statusFlagInfo.field.is_inServiceLinking = true;

                if (vl_tmp_freq == vI_freqToRemove1)
                    {
                    // mark it as original
                    // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ORIGINAL;
                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                    }
                else if (vl_tmp_freq == vI_freqToRemove2)
                    {
                    // mark it as alternate
                    // and already checked : nothing need to recheck
                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE; 
                    DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_numFreq].checkedForQuality = true;
                    }
                                
				vl_numFreq++;
				}
			}
		}
	
	if (vl_numFreq > 0)
		{
		// Set global status tom stored
	   DABMW_SF_freqCheckList.DAB_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_STORED; 
		}
	
	DABMW_SF_freqCheckList.numDabFreqForCheck = vl_numFreq;

	return vl_numFreq;
}


/* procedure to init the Global list of alternate Frequency
* specify if init is required for FM and/or DAB
*/
tVoid DABMW_ServiceFollowing_InitAlternateFrequency(tBool vI_initFM, tBool vI_initDAB)
{
	tU8 vl_cnt;

	if (vI_initFM)
		{
		DABMW_SF_freqCheckList.numFMFreqForCheck = 0;
		DABMW_SF_freqCheckList.freqCheck_status = DABMW_SF_FreqCheck_FM_AF_CHECK_ERROR;
		DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_EMPTY;
		DABMW_SF_freqCheckList.FM_FrequencyCheck.index = DABMW_INVALID_DATA_BYTE;

		for (vl_cnt = 0; vl_cnt < DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF; vl_cnt++)
			{
			DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].frequency = DABMW_INVALID_FREQUENCY;
			DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].piValue= DABMW_INVALID_RDS_PI_VALUE;
			DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].checkedForQuality = false;
            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].quality = DABMW_ServiceFollowing_QualityInit();
            // set all flags to false
            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].statusFlagInfo.value = 0x00;

            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_CHECKED;
			}
		}

	if (vI_initDAB)
		{
		DABMW_SF_freqCheckList.numDabFreqForCheck = 0;
		DABMW_SF_freqCheckList.freqCheck_status = DABMW_SF_FreqCheck_FM_AF_CHECK_ERROR;
		DABMW_SF_freqCheckList.DAB_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_EMPTY;
		DABMW_SF_freqCheckList.DAB_FrequencyCheck.index = DABMW_INVALID_DATA_BYTE;

		for (vl_cnt = 0; vl_cnt < DABMW_FIC_MAX_ENSEMBLE_NUMBER; vl_cnt++)
			{
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].frequency = DABMW_INVALID_FREQUENCY;
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].uniqueEid= DABMW_INVALID_ENSEMBLE_ID;
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].num_Sid = DABMW_INVALID_DATA_BYTE;
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].statusFlagInfo.field.SearchedSidFound = false;
            
			DABMW_ServiceFollowing_ExtInt_MemorySet(&(DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].uniqueSidList[0]),
							 DABMW_INVALID_SERVICE_ID,
							sizeof(tU32)*DABMW_FIC_MAX_SERVICES_NUMBER);
								
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].checkedForQuality = false;
			DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].statusFlagInfo.field.isQualityAcceptable = false;
            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].quality = DABMW_ServiceFollowing_QualityInit();
            // set all flags to false
            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].statusFlagInfo.value = 0x00;
              
            DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_CHECKED;
			}
		}

	return;

}

/* procedure to check if a frequency is an alternate Frequency FM
*/
tBool DABMW_ServiceFollowing_IsFrequencyInAlternateFM(tU32 vI_frequency)
{
	tU8 vl_cnt;
    tBool vl_res = false;

    if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
        {
        for (vl_cnt = 0; vl_cnt < DABMW_SF_freqCheckList.numFMFreqForCheck; vl_cnt++)
    	    {
            if (vI_frequency == DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt].frequency)
                {
                    vl_res = true;
                    break;
                }
        	}
        }


	return vl_res;

}

/* procedure to check if a frequency is an alternate Frequency DAB
*/
tBool DABMW_ServiceFollowing_IsFrequencyInAlternateDAB(tU32 vI_frequency)
{
	tU8 vl_cnt;
    tBool vl_res = false;

    if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_SF_freqCheckList.DAB_FrequencyCheck.status)
        {
        for (vl_cnt = 0; vl_cnt < DABMW_SF_freqCheckList.numDabFreqForCheck; vl_cnt++)
    	    {
            if (vI_frequency == DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt].frequency)
                {
                    vl_res = true;
                    break;
                }
        	}
        }


	return vl_res;

}

/* Procedure which provides the next frequency to check
* based on the Freq list
* Order = DAB First then FM.
*/
tU32 DABMW_ServiceFollowing_Background_NextCheck()
{
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_NONE;
	tSInt vl_tmp_res;
	tU32 vl_currentFreq = DABMW_INVALID_FREQUENCY;
	tBool vl_tunerAssociationNeed = false;
    tU8 vl_index;

	/* Retrieve where we are, and which frequency to switch 
	*/

	/* check current Freq.
	* Invalid = start
	* Other = switch to next freq or band..
	*/
	
	/* Let's check if DAB or FM */
	/* Reminder on FrequencyCheck Status :
	* STORED = exist and not used yet
	* EMPTY = nothing stored / to check
	* USED = currently under parsing
	* VERIFIED = verification complete...
	*/
	/* DAB = STORED => start of the Check, by DAB
	* freq still invalid & DAB_USED => check next DAB
	* freq still invalid ? means now it is time for FM
	* freq invalid & FM stored => start FM
	* freq invalid & FM used => next FM
	*/

	/* DAB = STORED => start of the Check, by DAB */ 
	if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status)
		{
		/* DAB is store, so start by DAB background check */
		if (DABMW_INVALID_DATA_BYTE == DABMW_SF_freqCheckList.DAB_FrequencyCheck.index)
			{
			/* this means we are at a start of check  (and even start of processing)
			* for DAB
			* The specific init/ configuratiion has been done in scan Init.
			*/

            // loop until a not already checked frequency is found
            //
            for (vl_index = 0; vl_index < DABMW_SF_freqCheckList.numDabFreqForCheck; vl_index++)
            {
                if (false == DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_index].checkedForQuality)
                    {
                    vl_currentFreq = DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_index].frequency;  
                    break;
                    }
            }

            DABMW_SF_freqCheckList.DAB_FrequencyCheck.index = vl_index;

    
            if (DABMW_INVALID_FREQUENCY != vl_currentFreq)
                {
                // a frequency is found , different from original and alternate
                // 
    			vl_tunerAssociationNeed = true;
			    			
    			/* change the DAB status to indicate it is being used */
    			DABMW_SF_freqCheckList.DAB_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_USED;
                }
            else
                {
                // nothing found => put it has verified
                /* we are at the end of DAB check */
			    DABMW_SF_freqCheckList.DAB_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_VERIFIED;
                }
			}
		else
			{
			/* should not happen : when DAB is STORED, the index should be INVALID
			* after the DAB move to 'USED' when being in use, or VERIFIED when done
			*/
			}
		}



	/* freq still invalid ? check if DAB used
	* freq still invalid & DAB_USED => check next DAB
	*/	
	if ((DABMW_INVALID_FREQUENCY == vl_currentFreq ) // freq not yet found check if DAB selected
		&& (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.DAB_FrequencyCheck.status)
		)
		{

            // loop until a not already checked frequency is found
            //
            DABMW_SF_freqCheckList.DAB_FrequencyCheck.index++;
            
            for (vl_index = DABMW_SF_freqCheckList.DAB_FrequencyCheck.index; vl_index < DABMW_SF_freqCheckList.numDabFreqForCheck; vl_index++)
                {
                if (false == DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_index].checkedForQuality)
                    {
                    vl_currentFreq = DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_index].frequency;  
                    break;
                    }
                }

            DABMW_SF_freqCheckList.DAB_FrequencyCheck.index = vl_index;

           /* check if index > number of freq to check or if found
                    */

    
            if (DABMW_INVALID_FREQUENCY == vl_currentFreq)
                {
                /* we are at the end of DAB check */
			    DABMW_SF_freqCheckList.DAB_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_VERIFIED;
                }
                    
		}

	
	/* freq still invalid ? means now it is time for FM 
	* freq invalid & FM stored => start FM
	*/
	// This may be time for AF Check in fact
	// 
	if ((true == DABMW_SF_BackgroundScanInfo.AFCheckRequested)
         && (false== DABMW_SF_BackgroundScanInfo.AFCheckdone))
        {
            // it is time for AF check 1st
	    }
    else
        {
    	if ((DABMW_INVALID_FREQUENCY == vl_currentFreq ) // freq not yet found check if FM 
    		&& (DABMW_STORAGE_STATUS_IS_STORED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
    		)
    		{
            if (DABMW_INVALID_DATA_BYTE == DABMW_SF_freqCheckList.FM_FrequencyCheck.index)
        	    {
        		/* this means we are at a start of check  (and even start of processing)
        			* for FM
        			* The specific init/ configuratiion has been done in scan Init.
        			*/
        		vl_tunerAssociationNeed = true;
                
                // loop until a not already checked frequency is found
                //
                for (vl_index = 0; vl_index < DABMW_SF_freqCheckList.numFMFreqForCheck; vl_index++)
                    {
                    if (false == DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_index].checkedForQuality)
                        {
                        // this is not checked yet so valid one.
            		    DABMW_SF_freqCheckList.FM_FrequencyCheck.index = vl_index;
            		    vl_currentFreq = DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_index].frequency;	
            		    DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_USED;
                        // stop the loop
                        break;
                        }
                    }
                
                DABMW_SF_freqCheckList.FM_FrequencyCheck.index = vl_index;
                
                 /* check if index > number of freq to check
                                */
                if (vl_index == DABMW_SF_freqCheckList.numFMFreqForCheck)
                    {
                     /* all the FM data have been parsed 
                                       * mark the end 
                                       */
                     /* we are at the end of FM check */
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_VERIFIED;
                    vl_currentFreq = DABMW_INVALID_FREQUENCY;
                    }
                
        		}
        		else
        		{
        		/* should not happen : when FM is STORED, the index should be INVALID
        			* after the FM  move to 'USED' when being in use, or VERIFIED when done
        			*/
        		}
    		}

    	/* freq still invalid ? is FM in used ?
    	        * freq invalid & FM used => next FM
    	        */
    	if ((DABMW_INVALID_FREQUENCY == vl_currentFreq ) // freq not yet found check if FM 
    		&& (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
    		)
    		{
    		/* it is now time for FM */

            // loop until a free one not checked for quality
            //
            /* increment the index */
    		DABMW_SF_freqCheckList.FM_FrequencyCheck.index++;	
            
            for (vl_index = DABMW_SF_freqCheckList.FM_FrequencyCheck.index; vl_index < DABMW_SF_freqCheckList.numFMFreqForCheck; vl_index++)
                {
                if (false == DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_index].checkedForQuality)
                    {
                    vl_currentFreq = DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_index].frequency;
                    break;
                    }
                }
            DABMW_SF_freqCheckList.FM_FrequencyCheck.index = vl_index;

    		/* check if index > number of freq to check
    		        */
    		if (vl_index == DABMW_SF_freqCheckList.numFMFreqForCheck)
    			{
    			/* all the FM data have been parsed 
    			        * mark the end 
    			        */
    			/* we are at the end of FM check */
    			DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_VERIFIED;
    			vl_currentFreq = DABMW_INVALID_FREQUENCY;
    			}
    		}
	}
	

	/* a frequency have been found for check */
	if (DABMW_INVALID_FREQUENCY != vl_currentFreq)
		{
		/* is that a continuation on same tuner & app ? 
		* if tunerAssociation Need is set : it means change of app ... bearer 
		*/

		if (vl_tunerAssociationNeed)
			{
			/* associate the tuner
			*/		
			vl_band = DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand (vl_currentFreq); 

			/* Reset/Release the prior used tuner to free resources */
			DABMW_ServiceFollowing_ResetTuner(DABMW_SF_BackgroundScanInfo.bgScan_App);
			
			/* Select the new best suited app */

			DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_ServiceFollowing_Background_SelectApp(vl_band);

			if (DABMW_NONE_APP != DABMW_SF_BackgroundScanInfo.bgScan_App)
				{
				vl_tmp_res = OSAL_OK;

				/* Tuner Association here is not needed : 
				* done by TuneFrequency... 
				*
				* vl_tmp_res = DABMW_ConfigureTunerForSpecificApp (DABMW_SF_BackgroundScanInfo.bgScan_App, vl_band, vl_currentFreq);
				*/
				}
			else 
				{
				vl_tmp_res = OSAL_ERROR;
				}
					
			if (OSAL_ERROR == vl_tmp_res)
				{
				DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW Error: DABMW_ServiceFollowing_Background_NextCheckInit failed, ScanApp = %d, Band = %d, startFreq = %d\n",
												DABMW_SF_BackgroundScanInfo.bgScan_App, vl_band, vl_currentFreq);

				vl_currentFreq = DABMW_INVALID_FREQUENCY;
				}
			else 
				{
				/* If the APP / band is FM : enable RDS 
				*/
				if (DABMW_ServiceFollowing_ExtInt_IsFMBand(vl_band))
					{

                     // We may need to stop the VPA during the bg search
                     //
                    DABMW_ServiceFollowing_AmFMStopVpa(DABMW_SF_BackgroundScanInfo.bgScan_App);

                     // RDS should already be enable but let's check in the procedure
                    DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, true);
					
					}
				}
							
			}
		}
	else
		{
		/* no frequency found */
		/* we need to release the tuner in case.. */
		/* Reset/Release the prior used tuner to free resources */
		DABMW_ServiceFollowing_ResetTuner(DABMW_SF_BackgroundScanInfo.bgScan_App);
		}
	
	DABMW_SF_BackgroundScanInfo.bg_Freq = vl_currentFreq;

	return vl_currentFreq;
}

/* Procedure to continue the background scan, in charge of tuning to next frequency 
* 
*/

tSInt DABMW_ServiceFollowing_BackgroundCheck()
{

	tSInt vl_tmp_res;
	tU32  vl_currentFreq = DABMW_INVALID_FREQUENCY;
	DABMW_SF_BG_SearchedResultTy vl_succesfulSearched;		
	tBool vl_PiReconfirmationNeeded = false;

	/* Retrieve where we are, and which frequency to switch 
	*/
	vl_currentFreq = DABMW_ServiceFollowing_Background_NextCheck();

	
	if (DABMW_INVALID_FREQUENCY == vl_currentFreq)
		{
        // perhaps     
        
        // This may be time for AF Check in fact
    	if ((true == DABMW_SF_BackgroundScanInfo.AFCheckRequested)
             && (false== DABMW_SF_BackgroundScanInfo.AFCheckdone))
            {

            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
			}
        // do we need to do background scan instead ?
        else if (true == DABMW_SF_BackgroundScanInfo.backgroundScanRequested)
            {
            // this marks the end of bg check
            //
            DABMW_SF_BackgroundScanInfo.backgroundCheckDone = true;
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);   
            }
        else 
            {
             // this marks the end of bg check
            //
            DABMW_SF_BackgroundScanInfo.backgroundCheckDone = true;
          
			/* No scan request => end of processing */
            DABMW_ServiceFollowing_EndBackgroundScan();

            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            
			}
    	/* this stops of procedure reset data */
    	DABMW_serviceFollowingStatus.currentAudioUserApp = (DABMW_SF_mwAppTy) DABMW_SF_BackgroundScanInfo.bgScan_App ;
    	DABMW_serviceFollowingStatus.currentFrequency = DABMW_INVALID_FREQUENCY;	
    	DABMW_serviceFollowingStatus.currentSystemBand = DABMW_BAND_NONE;
    	DABMW_serviceFollowingStatus.currentEid = DABMW_INVALID_ENSEMBLE_ID;
    	DABMW_serviceFollowingStatus.currentSid = DABMW_INVALID_SERVICE_ID;
    	DABMW_serviceFollowingStatus.dab_syncStatus = DABMW_INVALID_DATA_BYTE;
        DABMW_serviceFollowingStatus.current_Quality = DABMW_ServiceFollowing_QualityInit();
        DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath = DABMW_ServiceFollowing_QualityInit();
            
		
		// add on etal : get the handle which may have been allocated
		DABMW_serviceFollowingStatus.currentHandle = DABMW_SF_BackgroundScanInfo.bgScan_Handle;
	   
		/* back to main loop for decision what's next 
		*/
		vl_tmp_res = OSAL_ERROR;
		return OSAL_ERROR;
		}	
	else
		{
		/* a new freq is there => tune
		*/
		}

	/* if we come here, then the scan need to continue
	*/
	DABMW_serviceFollowingStatus.currentAudioUserApp = (DABMW_SF_mwAppTy) DABMW_SF_BackgroundScanInfo.bgScan_App ;
	DABMW_serviceFollowingStatus.currentFrequency = vl_currentFreq;	
	DABMW_serviceFollowingStatus.currentSystemBand = DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand (vl_currentFreq);
	DABMW_serviceFollowingStatus.currentEid = DABMW_INVALID_ENSEMBLE_ID;
	DABMW_serviceFollowingStatus.currentSid = DABMW_INVALID_SERVICE_ID;
	DABMW_serviceFollowingStatus.dab_syncStatus = DABMW_INVALID_DATA_BYTE;
    DABMW_serviceFollowingStatus.current_Quality = DABMW_ServiceFollowing_QualityInit();
    DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath = DABMW_ServiceFollowing_QualityInit();

	if (DABMW_ServiceFollowing_ExtInt_IsFMBand(DABMW_serviceFollowingStatus.currentSystemBand))
		{
		/* FM is being tuned 
		*/
		DABMW_ServiceFollowing_ExtInt_TaskClearEvent(DABMW_SF_EVENT_PI_RECEIVED);

		/* Tune on frequency */

		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency(DABMW_SF_BackgroundScanInfo.bgScan_App, 
										vl_currentFreq);	
		

                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                        DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    /* END TMP LOG */

		if (OSAL_ERROR == vl_tmp_res)
			{
			/* it means either that error in tune 
			*/
			/* continue
			*/
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK); 
            
			}
		else if (false == DABMW_ServiceFollowing_StoredAndEvaluate_AF_FMQuality())
			{
			/* quality is not good , no need to go further
			*/
			/* store attempted freq, but continue */			
			DABMW_serviceFollowingStatus.currentFrequency = vl_currentFreq; 
			DABMW_SF_BackgroundScanInfo.bg_Freq = vl_currentFreq;

            /* Store the reason */
            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_QUALITY_NOT_OK;
            

			/* back to check / check */
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK); 
                   /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,      
                                     "DABMW_Service_Following (Background check) : Freq = %d, POOR Quality\n",
                                      DABMW_serviceFollowingStatus.currentFrequency);  

             DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (Background check) :");

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
       /* END TMP LOG */

			}
		else
			{
			/* Quality is acceptable => go on */
			DABMW_serviceFollowingStatus.currentFrequency = vl_currentFreq; 
			DABMW_SF_BackgroundScanInfo.bg_Freq = vl_currentFreq;
			/* Check if PI stored and valid */
			DABMW_serviceFollowingStatus.currentSid = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetValidPiFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq, DABMW_serviceFollowingData.PI_validityTime);
                    
			/*
			* Get PI Status = INVALID => not valid (unknown or stored since too long)
			* if valid, check value versus searched one.
			*/
			if (DABMW_INVALID_DATA == DABMW_serviceFollowingStatus.currentSid)
				{
				/* the PI at that freq is either unknown either not valid
				*/
				/* PI no valid : wait for PI check */
                // decode PI only if quality is above the RDS PI quality decoding threshold

               if (true == DABMW_ServiceFollowing_IsFMQualityEnoughForRdsDecoding(DABMW_serviceFollowingStatus.current_Quality.fmQuality))
                    {
                    // 
                    // Increase PI matching chance
                    DABMW_ServiceFollowing_ExtInt_IncreasePiMatchingAF(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq, DABMW_SF_BackgroundScanInfo.searchedServiceID);
    				
#if defined(CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
    				DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (vl_currentFreq, NULL, &DABMW_ServiceFollowing_OnPI_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)

    				DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

                    /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                   DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,      
                                                         "DABMW_Service_Following (Background check) : Freq = %d, Quality OK, PI not in database\n",
                                                          DABMW_serviceFollowingStatus.currentFrequency);  
                    
                    DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (Background check) :");
                    
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                                /* END TMP LOG */

                    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_PI);
                    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK); 
                   }
               else
                    {
                    // we consider quality is not good enough for decoding the PI...
                    // 
                    vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_PI_NOT_RECEIVED;
                    
                    /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Background check) :  Frequency skiped, quality below threshold to decode PI\n");

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
                    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);

                    }               
				}
			else 
				{
				/* PI exist : check next action depending if PI ok or not
				*/
				DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].piValue = DABMW_serviceFollowingStatus.currentSid;

				
				vl_succesfulSearched = DABMW_ServiceFollowing_GetSearchResult(DABMW_serviceFollowingStatus.currentSid,
																false, // bearer is not DAB
																true); // ID received emulate because valid


                
                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,      
                                                     "DABMW_Service_Following (Background check) : Freq = %d, Quality OK, PI in database 0x%04x, searched %s\n",
                                                      DABMW_serviceFollowingStatus.currentFrequency,
                                                      DABMW_serviceFollowingStatus.currentSid,
                                                      ((vl_succesfulSearched!=DABMW_SF_BG_RESULT_NOT_FOUND)?"SUCCESS":"FAILURE"));  
                
                DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (Background check) :");
                
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                                /* END TMP LOG */

                if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
                    {
                    // if we have found a candidate with good SID    
                    /* if we have an alternate, check if this one is different and better */

	                if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.alternateFrequency)    
	                	{
	                        
	                    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp))
	                    	{
	                        //alternate is DAB, current if FM
	                        // for now,  if the alternate is DAB : we consider we do not need a new FM alternate...;
	                        // 

	                        if (true == DABMW_ServiceFollowing_qualityA_DAB_better_qualityB_FM(DABMW_serviceFollowingStatus.alternate_Quality, DABMW_serviceFollowingStatus.current_Quality, 
	                                                                                       DABMW_serviceFollowingStatus.alternateFrequency, DABMW_serviceFollowingStatus.currentFrequency))
	                        	{
	                            /* the current found is not better than  alternate : 
	                                                   * continue the seach 
	                                                   */
	                            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Background check) : this is not as good as alternate because DAB preferred--> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	                             /* END TMP LOG */
	                             vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;
	                                
	                               /* Store the reason */
	                              DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE;                            
	                             }
							else
								{
								// we need to reconfirm the PI before a potential switch
								vl_PiReconfirmationNeeded = true;
								}
	                        }
	                  else
	                      {
	                      // measure the current frequency on same app than alternate for far comarison
	                      // 
	                      DABMW_ServiceFollowing_Measure_AF_FMQuality_OnAlternatePath();
	                            

						// add-on quality display
					  
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)
					  if (DABMW_serviceFollowingStatus.alternateFrequency != DABMW_SF_BackgroundScanInfo.bg_Freq)
					  { 			   
						  DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath.fmQuality, "DABMW_Service_Following (Background check) : current quality on alternate path");
						  DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.alternate_Quality.fmQuality, "DABMW_Service_Following (Background check) : alternate quality on alternate path");
										 
					  }
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    	                            

	                      /* Check that current found is different and better than alternate */
	                      if (DABMW_serviceFollowingStatus.alternateFrequency == DABMW_SF_BackgroundScanInfo.bg_Freq)
	                  	  	{
	                        /* if same freq than alternate, skip */
	                        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Background check) : this is alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	                         /* END TMP LOG */
	                         vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

	                         /* Store the reason */
	                         DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE;

							// reconfirm the alternate PI
							vl_PiReconfirmationNeeded = true;
                            }
                          else if (false == DABMW_ServiceFollowing_qualityA_better_qualityB_FM(DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath, DABMW_serviceFollowingStatus.alternate_Quality,
	                                                                                            DABMW_serviceFollowingStatus.currentFrequency, DABMW_serviceFollowingStatus.alternateFrequency))
                            {
                            /* the alternate found is not better than current alternate : 
                                                   * continue the seach 
                                                   */
                            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Background check) : this is not as good as alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	                /* END TMP LOG */
                            vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

                            /* Store the reason */
                            DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE;                            

                            }
							else
								{
								// we need to reconfirm the PI
								vl_PiReconfirmationNeeded = true;
								}
	                   		}
						}
                	}
                else
                    {
                    // Not the right PI : update the error reason
                    //
                    /* Store the reason */
                    // Reason should be updated before....
                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_MATCHING_SID_PI;                       
                    }

			// if a PI reconfirmation is needed : move to PI...
			//
			if ((true == vl_PiReconfirmationNeeded)
				&&
				(true == DABMW_ServiceFollowing_IsFMQualityEnoughForRdsDecoding(DABMW_serviceFollowingStatus.current_Quality.fmQuality))
				)
				{
					// In case PI is perturbated 
					// and to be sure before switching PI is confirmed.
					// force PI reading...
					//
						// 
						// Increase PI matching chance
						DABMW_ServiceFollowing_ExtInt_IncreasePiMatchingAF(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bg_Freq, DABMW_SF_BackgroundScanInfo.searchedServiceID);
										
#if defined(CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
						DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (vl_currentFreq, NULL, &DABMW_ServiceFollowing_OnPI_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
					
						DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
					
						/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
					   DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,	   
												 "DABMW_Service_Following (Background check) : Freq = %d, Force to read PI again\n",
												  DABMW_serviceFollowingStatus.currentFrequency);  
										
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
						/* END TMP LOG */

						vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_PI);
						DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK); 
						
				}
			else
				{

				if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
					{
					/* PI at that freq is valid, and the right one
					*/
						
						DABMW_SF_BackgroundScanInfo.succesfulSearched = vl_succesfulSearched;
						/* PI found or confirmed
						* this marks the end of scan
						*/
						 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Background check) : Freq SUCCESS \n");       
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
	                   /* END TMP LOG */

	                    /* Store the reason */
	                    DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_FREQ_OK;
	                    


	                    DABMW_ServiceFollowing_EndBackgroundScan();

						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
	                    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_BACKGROUND_CHECK); 				
					}
				 else
				 	{
					/* this is not the right PI 
					* move next scan			
					* this is decided in next iteration to continue background scan (else we may lock the task ..) 
					* so that task is rescheduled meanwhile....
					*/
					/* Let's store the PI in AF list
					* quality already stored
					*/
					 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Background check) : Freq skipped \n");    
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    /* END TMP LOG */

                    /* Store the reason */
                    // Reason should be updated before....
                    // DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_MATCHING_SID_PI;
                    

                    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
                    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK); 
				 	}	 
				}
				}
			}
		}
	else
		{
		/* We are in DAB
		*/

		//TODO : check the Eid validity storage. If valid, check SIDs and do not tune if not necessary
		
		/* clear event in case */
		DABMW_ServiceFollowing_ExtInt_TaskClearEvent(DABMW_SF_EVENT_DAB_TUNE);

		/* tune and wait */	
		/* tSInt DABMW_TuneFrequency (DABMW_SF_mwAppTy app, tU32 frequency, tBool keepDecoding,
		*							  tU8 injectionSide, tBool noOutputSwitch, tBool isInternalTune)
		*/

		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency(DABMW_SF_BackgroundScanInfo.bgScan_App, 
										vl_currentFreq);
		
        DABMW_SF_BackgroundScanInfo.fig00_read = false;
		DABMW_SF_BackgroundScanInfo.fig01_read = false;
        DABMW_SF_BackgroundScanInfo.fig02_read = false;
		DABMW_SF_BackgroundScanInfo.fig02_readCounter = 0;
		
		if (OSAL_ERROR == vl_tmp_res)
			{
			/*error in tuning.
			*/
			/*
			* if no frequency : this mark the end of this band scan
			* proposition is that the end of backround scan is decided next iteration in any case
			* so that task is rescheduled meanwhile....
			*
			*/
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
			
			}
		else
			{
			DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

							   /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, 	 
									 "DABMW_Service_Following (Background check) : Tune DAB Freq = %d, app = %d\n",
									DABMW_serviceFollowingStatus.currentFrequency, DABMW_SF_BackgroundScanInfo.bgScan_App);  
#endif
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_DAB);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            
			}
		
		}
					
        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
            /* END TMP LOG */

		/* Tune ok : now just wait for tune, Sid and PI
		*/

	return OSAL_OK;

}

/* Procedure to check the AF quality and return if quality is acceptable
* return false if bad quality. true if good quality.
*/
tBool DABMW_ServiceFollowing_StoredAndEvaluate_AF_FMQuality()
{
 
    tBool vl_qualityIsGood = false;
 

	/* Do we have an AF list (ie AF check or background check) available
	*/
	if (DABMW_STORAGE_STATUS_IS_USED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
		{
		/* AF on going : store quality */
		 
        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].quality.fmQuality = DABMW_ServiceFollowing_ExtInt_AmFmGetQuality(DABMW_SF_BackgroundScanInfo.bgScan_App);

        DABMW_serviceFollowingStatus.current_Quality.fmQuality =  DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].quality.fmQuality;
        
		DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].checkedForQuality = true;
		
		vl_qualityIsGood = DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].quality);

        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].statusFlagInfo.field.isQualityAcceptable = vl_qualityIsGood;

		}

        
    return vl_qualityIsGood;
}

// procedure to retrieve current quality on a different path than Bg_Path : Alternate one's
// 
tSInt DABMW_ServiceFollowing_Measure_AF_FMQuality_OnAlternatePath()
{
    // we do it only if Alternate path is different from Bg path 
    // and if an alternate exist :)
    tSInt vl_res = OSAL_OK;
	DABMW_SF_amfmQualityTy vl_measQuality;
	
    // by default : set same value
    DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath = DABMW_serviceFollowingStatus.current_Quality;

	// this is applicable only if the alternate if FM of course, but also if number of tuner is sufficiant 
	// else we consider alternateApp is periodically measured on the bg app : so no need to re-check
	// the 'alternate is tuned' is managing that information

    if ((true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp)
        &&
        (DABMW_serviceFollowingStatus.alternateApp != DABMW_serviceFollowingStatus.currentAudioUserApp)
        &&
        (true == DABMW_serviceFollowingStatus.alternateTuned))
        )
    {
        // alternate is FM , and has been measured on a different path
        //
        
        // measure current on this path
        // thru an AF check
        
        // check quality using AF check which is better : alternate will be measured on same tuner than original
        vl_res = DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(DABMW_serviceFollowingStatus.alternateApp, // app on which to do the AF check
                                          DABMW_serviceFollowingStatus.currentFrequency, // Alternate Frequency check
										  &vl_measQuality); // quality storage      

	   if (OSAL_OK == vl_res)
	   {
	   		DABMW_serviceFollowingStatus.current_Quality_OnAlternatePath.fmQuality = vl_measQuality;
	   }
	   else
	   {
	   		// AF check could not be done, keep comparison with current quality
	   }
    }


    return vl_res;
}



/* Procedure to check the AF on AF check quality and return if quality is acceptable
*/
tBool DABMW_ServiceFollowing_StoredAndEvaluate_AFCheck_AF_FMQuality()
{
 
    tBool vl_qualityIsGood = false;
 
	// Quality has already been measured by the AF Start procedure
	//
	
	/* Do we have an AF list (ie AF check or background check) available
	*/
	if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
		{
		/* AF Check on going : store quality */

        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].quality.fmQuality = DABMW_serviceFollowingStatus.current_Quality.fmQuality;     

  		DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].checkedForQuality = true;
		
        vl_qualityIsGood = DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].quality);
        
        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].statusFlagInfo.field.isQualityAcceptable = vl_qualityIsGood;
		}
        
    return vl_qualityIsGood;
}


/* FM   AF Check request
*/

tSInt DABMW_SF_AmFmAFCheckRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality)
{
	return (DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(vI_app, vI_frequency, pO_quality));
}


/* FM   AF Start request
*/

tSInt DABMW_SF_AmFmAFStartRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality)
{

	tSInt res; 

    res = DABMW_ServiceFollowing_ExtInt_AmFmAFStartRequest(vI_app, vI_frequency, pO_quality);
	
    // If the seek failed then set to invalid the frequency, if it is not
    // ask for the tuned frequency
    if (OSAL_ERROR == res)
    {
        res = OSAL_ERROR;
    }
    else
    {
        DABMW_SF_BackgroundScanInfo.AF_proc_ongoing = true;
        DABMW_SF_BackgroundScanInfo.afproc_app = vI_app;
    }

    return res;
}


/* FM   AF END request
*/

tSInt DABMW_SF_AmFmAFEndRequest()
{

	tSInt res = OSAL_ERROR; 

	
    // check if something on going
    if ( false == DABMW_SF_BackgroundScanInfo.AF_proc_ongoing )
    {
        return res;
    }
      

    res = DABMW_ServiceFollowing_ExtInt_AmFmAFEndRequest(DABMW_SF_BackgroundScanInfo.afproc_app);
	
    // If the seek failed then set to invalid the frequency, if it is not
    // ask for the tuned frequency
    if (OSAL_ERROR == res)
    {
        res = OSAL_ERROR;
    }
    else
    {
		res = OSAL_OK; 
    }

    // reset flags
    DABMW_SF_BackgroundScanInfo.AF_proc_ongoing = false;
    DABMW_SF_BackgroundScanInfo.afproc_app = DABMW_NONE_APP;


    return res;
}

// Procedure to check if AF bg procedure is on going.
tBool DABMW_ServiceFollowing_IsBackgroundAFCheckOnGoing()
{
    return (DABMW_SF_BackgroundScanInfo.AF_proc_ongoing);
}


// Procedure to check if AF bg procedure is on going.
tBool DABMW_ServiceFollowing_IsLanscapeScanOnGoing()
{
    return ((DABMW_SF_SCAN_LANDSCAPE == DABMW_serviceFollowingStatus.ScanTypeDab) 
		|| (DABMW_SF_SCAN_LANDSCAPE == DABMW_serviceFollowingStatus.ScanTypeFm));
}

/* Procedure which provides the next frequency to check
* based on the Freq list
* Order = DAB First then FM.
*/
tU32 DABMW_ServiceFollowing_AFcheck_NextCheck()
{
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_NONE;
	tU32 vl_currentFreq = DABMW_INVALID_FREQUENCY;
	DABMW_SF_mwAppTy vl_AFCheckApp;
	

	/* Retrieve where we are, and which frequency to switch 
	*/

	/* AF check is valid for FM only, and only when FM tuned, and on a given AF list
	* so check if FM is tuned and check if stored frequency exists
	*/

	if (DABMW_STORAGE_STATUS_IS_STORED != DABMW_SF_freqCheckList.FM_FrequencyCheck.status)
		{
		return (DABMW_INVALID_FREQUENCY);
		}
	
	if (DABMW_INVALID_DATA_BYTE == DABMW_SF_freqCheckList.FM_FrequencyCheck.index)
		{
		DABMW_SF_freqCheckList.FM_FrequencyCheck.index = 0;
		vl_currentFreq = DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].frequency;	
		}
	else
		{
		/* Check should go on 
		*/

		/* increment the index */
		DABMW_SF_freqCheckList.FM_FrequencyCheck.index++;		

		/* check if index > number of freq to check
		*/
		if (DABMW_SF_freqCheckList.numFMFreqForCheck < DABMW_SF_freqCheckList.FM_FrequencyCheck.index)
			{
			/* all the FM data have been parsed 
			* mark the end 
			*/
			/* we are at the end of FM check */
			DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_VERIFIED;
			vl_currentFreq = DABMW_INVALID_FREQUENCY;
			}
		else
			{
			vl_currentFreq = DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].frequency;	
			}
		}
	

	/* a frequency have been found for check */
	if (DABMW_INVALID_FREQUENCY != vl_currentFreq)
		{
		vl_band = DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand ( vl_currentFreq);	

		/* Select the new best suited app */
		vl_AFCheckApp = DABMW_ServiceFollowing_AFCheck_SelectApp(vl_band);

      
        // RDS should already be enable but let's check in the procedure
       DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, true);

		if (DABMW_NONE_APP == vl_AFCheckApp)
			{
			/* no APP found : we are cannot to AF check...should be background 
			*/
			vl_currentFreq = DABMW_INVALID_FREQUENCY;
			}
		else
			{
			DABMW_SF_BackgroundScanInfo.bgScan_App = vl_AFCheckApp;
			}

		}
	
	DABMW_SF_BackgroundScanInfo.bg_Freq = vl_currentFreq;

	return vl_currentFreq;
}

/* Procedure to continue the background scan, in charge of tuning to next frequency 
* 
*/

tSInt DABMW_ServiceFollowing_AFCheck()
{

	tSInt vl_tmp_res;
	tU32  vl_currentFreq = DABMW_INVALID_FREQUENCY;
	tU32  vl_Pi;
	DABMW_SF_BG_SearchedResultTy vl_succesfulSearched;


	/* Retrieve where we are, and which frequency to switch 
	*/
	vl_currentFreq = DABMW_ServiceFollowing_AFcheck_NextCheck();

	
	if (DABMW_INVALID_FREQUENCY == vl_currentFreq)
		{
		/* this means we have finished the process of Freq Check 
		*/
        DABMW_SF_BackgroundScanInfo.AFCheckdone = true;


        // We should end the AF CHECK procedure
        //
        DABMW_SF_AmFmAFEndRequest();
        
		/* do we need to continue for Scan ?
		*/
		if (DABMW_SF_BackgroundScanInfo.backgroundCheckRequested)
			{
			/* we should re-init some information in the 'list' 
			* the index is enough
			* the stored info / freq should already be ok
			*/		
			DABMW_SF_freqCheckList.FM_FrequencyCheck.index = DABMW_INVALID_DATA_BYTE;

            // we should also put back the list to 'STORED'
            // so that FM which have not been 'excluded' are parse (ie those which are quality ok, but for which we do not have the PI...)
            //
            DABMW_SF_freqCheckList.FM_FrequencyCheck.status = DABMW_STORAGE_STATUS_IS_STORED;
            
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
           			
			}
		else if (DABMW_SF_BackgroundScanInfo.backgroundScanRequested)
			{
			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
            					
			}
		else
			{
			/* No scan request => end of processing */
            DABMW_ServiceFollowing_EndBackgroundScan();

            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_AF_CHECK);
            
			}

		/* this is stop of proceudre reset data */
		
		/* back to main loop for decision what's next 
		*/
		vl_tmp_res = OSAL_ERROR;
		return OSAL_ERROR;
		}	
	else
		{
		/* a new freq is there => tune
		*/
		}

	/* if we come here, then the AF check on FM need to continue
	*/

    /*
	vl_tmp_res = DABMW_SF_AmFmAFCheckRequest(DABMW_SF_BackgroundScanInfo.bgScan_App, 
											vl_currentFreq, 
											&DABMW_serviceFollowingStatus.current_Quality.fmQuality);	
        */

    vl_tmp_res = DABMW_SF_AmFmAFStartRequest(DABMW_SF_BackgroundScanInfo.bgScan_App, 
                                                    vl_currentFreq, 
                                                    &DABMW_serviceFollowingStatus.current_Quality.fmQuality);   


         /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
            /* END TMP LOG */

    /* store attempted freq, but continue */            
    DABMW_serviceFollowingStatus.currentFrequency = vl_currentFreq; 
    DABMW_SF_BackgroundScanInfo.bg_Freq = vl_currentFreq;

	if (OSAL_ERROR == vl_tmp_res)
		{
		/* it means either that error in tune 
		*/
		/* continue
		*/

        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
            
		}
	else if (false == DABMW_ServiceFollowing_StoredAndEvaluate_AFCheck_AF_FMQuality())
		{
		/* quality is not good , no need to go further
		*/
        /* Store the reason */
        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_QUALITY_NOT_OK;
        
		/* back to check / check */
        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);

       /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,      
                                     "DABMW_Service_Following (AF check) : Freq = %d, POOR Quality\n",
                                      DABMW_serviceFollowingStatus.currentFrequency);  

             DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (AF check) :");

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
       /* END TMP LOG */
        
		}
	else
		{
		/* Quality is acceptable => go on 
		* Check if valid PI
		*/

		/* Check if PI stored and valid */
		vl_Pi = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetValidPiFromFreq(DABMW_SF_BackgroundScanInfo.bg_Freq, DABMW_serviceFollowingData.PI_validityTime);

		/*
		* Get PI Status = INVALID => not valid (unknown or stored since too long)
		* if valid, check value versus searched one.
		*/
		if (DABMW_INVALID_DATA == vl_Pi)
			{
			/* the PI at that freq is either unknown either not valid
			*/
            /* PI no valid : wait for PI check */
            // In current version : PI check is done in BG CHECK (ie VPA break if needed, TUNE...)
            // In future version (to be checked) we could stay in that state..
            //
            /*

                    if (true == DABMW_SF_BackgroundScanInfo.AF_proc_ongoing)
                        {
#if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
                       DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (vl_currentFreq, NULL, &DABMW_ServiceFollowing_OnPI_Callback, true);
#endif // #if (defined CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)
                            
                       DABMW_serviceFollowingStatus.bg_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
                       
            			// In FM, thanks to AF start we can get the PI
            			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_WAIT_PI);
                        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
                        }
                    else
                        */
                {
                    
                // reset the quality check info to indicate it may be reuse in BG check
                //
                /* not in AF start way ==> skipp that freq, will be ahndle in bg check
    			        */
    			DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
                DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].checkedForQuality = false;
                }

            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,      
                                     "DABMW_Service_Following (AF check) : Freq = %d, Quality OK, PI not in database\n",
                                      DABMW_serviceFollowingStatus.currentFrequency);  

             DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (AF check) :");

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
            /* END TMP LOG */
            
			}
		else 
			{
			/* PI exist : check next action depending if PI ok or not
			*/

			DABMW_serviceFollowingStatus.currentSid = vl_Pi;
			
			vl_succesfulSearched = DABMW_ServiceFollowing_GetSearchResult(vl_Pi,
																false, // bearer is not DAB
																true); // ID received emulate because valid

                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
             DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,      
                                     "DABMW_Service_Following (AF check) : Freq = %d, Quality OK, PI in database 0x%04x, searched %s\n",
                                      DABMW_serviceFollowingStatus.currentFrequency,
                                      DABMW_serviceFollowingStatus.currentSid,
                                      ((vl_succesfulSearched!=DABMW_SF_BG_RESULT_NOT_FOUND)?"SUCCESS":"FAILURE"));  

             DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.current_Quality.fmQuality, "DABMW_Service_Following (AF check) :");

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                /* END TMP LOG */


             if ( DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
                {
                // we have found an alternate with good PI.    
                /* Should we check for the quality ?    */
                /* check the quality : if an alternate is configured in FM already, we should look for an better that current 
                              * state here should be : AF Scan
                              */
                /* if we have an alternate, check if this one is different and better */
                if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.alternateFrequency)    
                    {           
                 /* if same freq than alternate, skip */
                 if (DABMW_serviceFollowingStatus.alternateFrequency == DABMW_SF_BackgroundScanInfo.bg_Freq)
                        {
                        /* the alternate found is not better than current alternate : 
                                        * continue the seach 
                                        */
                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                 DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (AF check) : this is alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */
                        vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

                        /* Store the reason */
                        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_IS_ALTERNATE;

                        }
                 else if (false == DABMW_ServiceFollowing_qualityA_better_qualityB_FM(DABMW_serviceFollowingStatus.current_Quality, DABMW_serviceFollowingStatus.alternate_Quality,
                                                                                      DABMW_serviceFollowingStatus.currentFrequency, DABMW_serviceFollowingStatus.alternateFrequency))
                        {
                        /* the alternate found is not better than current alternate : 
                                        * continue the seach 
                                        */
                    /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                 DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (AF check) : this is not as good as alternate --> skip \n");        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    /* END TMP LOG */
                        vl_succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND ;

                        /* Store the reason */
                        DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE;

                        }
                    }
                 }
             else
                {
                
                /* Store the reason */
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_MATCHING_SID_PI;
                
                }

			if (DABMW_SF_BG_RESULT_NOT_FOUND != vl_succesfulSearched)
				{
				/* PI at that freq is valid, and the right one
				*/
				DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].piValue = vl_Pi;
				DABMW_SF_BackgroundScanInfo.succesfulSearched = vl_succesfulSearched;
				/* PI found or confirmed
				* this marks the end of scan
				*/
           
                /* Store the reason */
                DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_FREQ_OK;

                DABMW_ServiceFollowing_EndBackgroundScan();
                                
				DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
                DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_AF_CHECK);
            				
				}
			 else
			 	{
				/* this is not the right PI 
				* move next scan			
				* this is decided in next iteration to continue background scan (else we may lock the task ..) 
				* so that task is rescheduled meanwhile....
				*/
				/* Let's store the PI in AF list
				* quality already stored
				*/
				DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].piValue = vl_Pi;
                
                /* Store the reason */
                // EPR Correction : reason should already be ok                
                // DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[DABMW_SF_freqCheckList.FM_FrequencyCheck.index].bgCheckStatus = DABMW_SF_CHECK_STATUS_NOT_MATCHING_SID_PI;

                DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
                DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
			    }
    		}
    	}

             /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                /* END TMP LOG */

	return OSAL_OK;
}

/* Procedure to request a background service recovery scan
* input parameter = 
* target searched service + choice of bearer. (DAB / FM)
*/
tSInt DABMW_ServiceFollowing_ServiceRecoveryScan(tU32 vI_SearchedServiceID)
{
				
	tSInt vl_numLandscapeFreq = 0, vl_numLandscapeFreqDab = 0, vl_numLandscapeFreqFm = 0;
	DABMW_SF_systemBandsTy vl_band[3] = {DABMW_BAND_NONE, DABMW_BAND_NONE, DABMW_BAND_NONE};
	tU8 vl_numBandToScan = 0;
	tBool vl_initDAB, vl_initFM;
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);

	/* Check if we are in idle...
	* else reject the selection
	*/
	if (DABMW_SF_STATE_SERVICE_RECOVERY != DABMW_serviceFollowingStatus.status)
		{
		return OSAL_ERROR;
		}

	/* Init structure for FM & DAB
	*/
	DABMW_ServiceFollowing_InitAlternateFrequency(true, true);

	if (true == vl_originalIsDab)
		{
		vl_initDAB = DABMW_serviceFollowingData.dabToDabIsActive;

		vl_initFM = DABMW_serviceFollowingData.dabToFmIsActive;
		}
	else
		{
		vl_initDAB = DABMW_serviceFollowingData.fmToDabIsActive;
		vl_initFM = DABMW_serviceFollowingData.fmToFmIsActive;
		}

	/* check at least 1 bearer is requested */
	if ((false == vl_initDAB) && (false == vl_initFM))
		{
		/* we got a major error here.. bad configuration */
		/* back to previous state */
        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
        	
		return OSAL_ERROR;
		}


	/* init the service linking information if needed 
	* only if original is DAB
	* else if original is FM we keep the stored one... assuming this was valid when switching from DAB to FM...
	*/
	if (true == vl_originalIsDab)
		{
		DABMW_ServiceFollowing_BuildLinkIdList(DABMW_serviceFollowingStatus.originalAudioUserApp, vI_SearchedServiceID, vl_initDAB, vl_initFM);	
		}

	
	/* init previous state & current (is already correctly set) */
    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_SERVICE_RECOVERY);

	/* Set the selected tuner for initial sel selection 
	*/
	DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_NONE_APP;

	/* Set the audio config choice
	*/

	DABMW_SF_BackgroundAudioInfo.keepDecoding = SF_BACKGROUND_TUNE_KEEP_DECODING;
	DABMW_SF_BackgroundAudioInfo.injectionSide = SF_BACKGROUND_TUNE_INJECTION_SIDE;
	DABMW_SF_BackgroundAudioInfo.noOutputSwitch = SF_BACKGROUND_TUNE_NO_OUTPUT_SWITCH;

	/* Init the requested algo */
	
	DABMW_SF_BackgroundScanInfo.AFCheckRequested = false;
	DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = false;
	DABMW_SF_BackgroundScanInfo.backgroundScanRequested = true;
    DABMW_SF_BackgroundScanInfo.backgroundScanDone = false;
	DABMW_SF_BackgroundScanInfo.backgroundScanSkipOriginalFreq = true;

	/* Init data for search 
	*/

	if (true == vl_initDAB)
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vl_numLandscapeFreqDab = DABMW_ServiceFollowing_BuildAlternateFrequency_DAB(vI_SearchedServiceID, DABMW_serviceFollowingStatus.originalFrequency, DABMW_INVALID_FREQUENCY);
		vl_band[vl_numBandToScan] = DABMW_ServiceFollowingGetDABBandIII();
		vl_numBandToScan++;
        vl_numLandscapeFreq = vl_numLandscapeFreqDab;
		}
	
	if (true == vl_initFM)
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vl_numLandscapeFreqFm = DABMW_ServiceFollowing_BuildAlternateFrequency_FM(vI_SearchedServiceID, DABMW_serviceFollowingStatus.originalFrequency, DABMW_INVALID_FREQUENCY);
		vl_band[vl_numBandToScan] = DABMW_ServiceFollowingGetFMBand();
		vl_numBandToScan++;
        vl_numLandscapeFreq += vl_numLandscapeFreqFm;
		}

	/* init data for future full band scan */
	DABMW_ServiceFollowing_Background_ScanInit(vI_SearchedServiceID, vl_band[0],  vl_band[1], vl_band[2]);



				
	if (vl_numLandscapeFreq > 0)
		{
		/* start by AF or background check on the stored info in landscape
		*/
		DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = true;

		/* no AF check in service recovery : no service so can be interupted 
		*/
		/* Initialize the new state that will be entered
		*/
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
					
		}
	else
		{
		/* this will be a background scan, no step to AF or background check
		*/
		/* Initialize the new state that will be entered
		*/
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
        
		}


                                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                                       DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                                   /* END TMP LOG */

                   /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, 
                                    "DABMW_Service_Following (Service Recovery) : start scanning for recovery, searched PI 0x%04x. DAB %s, num LandscapeFreqDAB %d, FM %s, num LandscapeFreqFM %d\n",
                                    vI_SearchedServiceID,
                                    DABMW_SERVICE_FOLLOWING_LOG_BOOL(vl_initDAB),
                                    vl_numLandscapeFreqDab,
                                    DABMW_SERVICE_FOLLOWING_LOG_BOOL(vl_initFM),
                                    vl_numLandscapeFreqFm);                                       
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                   /* END TMP LOG */

                 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                        DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    /* END TMP LOG */


	/* this is all for the init, now the background will start on trigger
	*/
	
	return OSAL_OK;

}

/* Procedure to request a background service AF scan
* input parameter = 
* target searched service + choice of bearer. (DAB / FM)
*/
tSInt DABMW_ServiceFollowing_AFScan(tU32 vI_SearchedServiceID, tBool vI_initDAB, tBool vI_initFM, tBool vI_fullScanDAB, tBool vI_fullScanFM)
{
				
	tSInt vl_numLandscapeFreq = 0, vl_numLandscapeFreqDab = 0, vl_numLandscapeFreqFm = 0;
	DABMW_SF_systemBandsTy vl_band[3] = {DABMW_BAND_NONE, DABMW_BAND_NONE, DABMW_BAND_NONE};
	tU8 vl_numBandToScan = 0;
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
    tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
	tBool vl_initDAB, vl_initFM;
    

	/* Check if we are in idle...
	* else reject the selection
	*/
	if (DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF != DABMW_serviceFollowingStatus.status)
		{
		return OSAL_ERROR;
		}


	/* Init structure for FM & DAB
	*/
	DABMW_ServiceFollowing_InitAlternateFrequency(true, true);
	

	/* update the variable compare to what is configured/supported 
	*/
	if (true == vl_originalIsDab)
		{
		/* original is DAB : check if DAB to DAB */
		vl_initDAB = ((true == vI_initDAB) && (true == DABMW_serviceFollowingData.dabToDabIsActive));
		/* original is DAB : check if DAB to FM */
		vl_initFM = ((true == vI_initFM) && (true == DABMW_serviceFollowingData.dabToFmIsActive));
		}
	else
		{
		/* original is FM : check if FM to DAB */          
		vl_initDAB = ((true == vI_initDAB)  && (true == DABMW_serviceFollowingData.fmToDabIsActive));

        // Change : 
        // if alternate is DAB and acceptable, 
        // if DAB to DAB is not active
        // do not look for an other alternate => there is no point, since it is similar to DAB - DAB
        // (and for the moment it creates error on the platform
        //
        if (true == vl_alternateIsDab)
            {
            if (false == DABMW_serviceFollowingData.dabToDabIsActive)
                {
                    vl_initDAB = false;
                }
            }
            
		/* original is FM : check if FM to FM */
		vl_initFM = ((true == vI_initFM) && (true == DABMW_serviceFollowingData.fmToFmIsActive));
		}
  
  
      // update the scan type
      // NO UPDATE OF SCAN TYPE REQUESTED
          /*
          if (false ==  vl_initDAB)
          {
              DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_NONE;
          }
          
          if (false ==  vl_initFM)
          {
              DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_NONE;
          }
      */

	/* check at least 1 bearer is requested */
	if ((false == vl_initDAB) && (false == vl_initFM))
		{
		/* It means we evaluate nothing to do
		*/ 
		// 
		// indicate prev state = AF search
		// set the result of background search to not success
		//
		
        /* Init the info  */
        
        DABMW_SF_BackgroundScanInfo.AFCheckRequested = false;
        DABMW_SF_BackgroundScanInfo.AFCheckdone = false;
        DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = false;
        DABMW_SF_BackgroundScanInfo.backgroundCheckDone = false;
        DABMW_SF_BackgroundScanInfo.backgroundScanRequested = false;
        DABMW_SF_BackgroundScanInfo.backgroundScanDone = false;

        DABMW_ServiceFollowing_Background_ScanInit(vI_SearchedServiceID, DABMW_BAND_NONE,  DABMW_BAND_NONE, DABMW_BAND_NONE);

        
        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
		return OSAL_ERROR;
		}

    
	/* init the service linking information if needed 
	* only if original is DAB
	* else if original is FM we keep the stored one... assuming this was valid when switching from DAB to FM...
	*/
	if (true == vl_originalIsDab)
		{
		DABMW_ServiceFollowing_BuildLinkIdList(DABMW_serviceFollowingStatus.originalAudioUserApp, vI_SearchedServiceID, vl_initDAB, vl_initFM);	
		}

	/* init previous state & current should already be ok*/
    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);

	/* Set the selected tuner for initial sel selection 
	*/
	DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_NONE_APP;

	/* Set the audio config choice
	*/

	DABMW_SF_BackgroundAudioInfo.keepDecoding = SF_BACKGROUND_TUNE_KEEP_DECODING;
	DABMW_SF_BackgroundAudioInfo.injectionSide = SF_BACKGROUND_TUNE_INJECTION_SIDE;
	DABMW_SF_BackgroundAudioInfo.noOutputSwitch = SF_BACKGROUND_TUNE_NO_OUTPUT_SWITCH;

	/* Init the requested algo */
	
	DABMW_SF_BackgroundScanInfo.AFCheckRequested = false;
    DABMW_SF_BackgroundScanInfo.AFCheckdone = false;
	DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = false;
    DABMW_SF_BackgroundScanInfo.backgroundCheckDone = false;
	DABMW_SF_BackgroundScanInfo.backgroundScanRequested = true;
    DABMW_SF_BackgroundScanInfo.backgroundScanDone = false;

	DABMW_SF_BackgroundScanInfo.backgroundScanSkipOriginalFreq = true;

	/* Init data for search 
	*/

	if (true == vl_initDAB)
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vl_numLandscapeFreqDab= DABMW_ServiceFollowing_BuildAlternateFrequency_DAB(vI_SearchedServiceID, DABMW_serviceFollowingStatus.originalFrequency, DABMW_serviceFollowingStatus.alternateFrequency);
        vl_numLandscapeFreq = vl_numLandscapeFreqDab;

        // check if a full scan of band (ie bg scan) is needed for DAB
        if (true == vI_fullScanDAB)
            {
    		vl_band[vl_numBandToScan] = DABMW_ServiceFollowingGetDABBandIII();
    		vl_numBandToScan++;
            }
		}
	
	if (true == vl_initFM)
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vl_numLandscapeFreqFm = DABMW_ServiceFollowing_BuildAlternateFrequency_FM(vI_SearchedServiceID, DABMW_serviceFollowingStatus.originalFrequency, DABMW_serviceFollowingStatus.alternateFrequency);
        vl_numLandscapeFreq += vl_numLandscapeFreqFm;
            
         // check if a full scan of band (ie bg scan) is needed for FM
        if (true == vI_fullScanFM)
            {
    		vl_band[vl_numBandToScan] =  DABMW_ServiceFollowingGetFMBand();
    		vl_numBandToScan++;
            }
	}

	/* init data for future full band scan */
	DABMW_ServiceFollowing_Background_ScanInit(vI_SearchedServiceID, vl_band[0],  vl_band[1], vl_band[2]);
				
	if (vl_numLandscapeFreq > 0)
		{
        
		/* set the background check on the stored info in landscape
		*/
		DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = true;

        /* Let's Request some AF check when this is FM... 
                * conditions are : if bg tuner not free
                * - Original is FM, and  VPA is on.
                * - single tuner FM original is FM.
                           
               // below are the cases 
              // original = DAB no alternate             ==> we consider we have a free tuner, no AF Check
              // original = DAB and alternate = DAB ==> not ok for AF check, do background
              // original = DAB and alternate = FM   ==> either we do on alternate (which is MAIN), either we consider we have a free tuner so no AF check
              //      ==> consider no AF check.
              // orignial = FM and no alternate         ==> either we do on original (which is MAIN) , either we consider we have a free tuner so no AF check
              // original = FM and alternate = DAB   ==> either we do on original (which is MAIN) , either we consider we have a free tuner so no AF check
              // original = FM and alternate = FM      ==> either we do on original (which is MAIN), either we do it on alternate tuner if tuned, either we consider we have a free tuner so no AF check
              //    ==> consider no AF check for now.
              // VPA on                                         ==> we consider we have no free tuner, 1st do AF check.
              
              // for the moment consider always MAIN_AMFM
                * Else we can use the 2nd tuner and normal tune
               */
        if ((vl_numLandscapeFreqFm > 0 )
            &&
            (false == vl_originalIsDab)
            && 
            (true == DABMW_SF_BackgroundScanInfo.VPA_IsOnAtStart))
            {
            DABMW_SF_BackgroundScanInfo.AFCheckRequested = true;
            }

		/* check next step : 
		* DAB 1st ie background check if DAB present, 
		* else AF check for FM
		*/

        if (vl_numLandscapeFreqDab > 0)
            {
    		/* Initialize the new state that will be entered
    		        */
    		 DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
             DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            }
        else if (true == DABMW_SF_BackgroundScanInfo.AFCheckRequested)
            {
            // AF check
            /* Initialize the new state that will be entered
    		        */
    	    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
            }
        else
            {
            /* Initialize the new state that will be entered
    		        */
    		 DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
             DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
            }
        }
	else
		{
		/* this will be a background scan, no step to AF or background check
		*/
		/* Initialize the new state that will be entered
		*/
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
         
		}

                           /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                                  DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                              /* END TMP LOG */


              /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
        DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, 
                                "DABMW_Service_Following (AF Scan) : start scanning for AF for PI 0x%04x => DAB %s, num LandscapeFreqDAB %d, full scan DAB %s, FM %s, num LandscapeFreqFM %d, full scan FM %s\n",
                                vI_SearchedServiceID,
                                DABMW_SERVICE_FOLLOWING_LOG_BOOL(vl_initDAB),
                                vl_numLandscapeFreqDab,
                                DABMW_SERVICE_FOLLOWING_LOG_BOOL(vI_fullScanDAB),
                                DABMW_SERVICE_FOLLOWING_LOG_BOOL(vl_initFM),
                                vl_numLandscapeFreqFm,
                                DABMW_SERVICE_FOLLOWING_LOG_BOOL(vI_fullScanFM));                                       
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
               /* END TMP LOG */

                 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                        DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    /* END TMP LOG */



	/* this is all for the init, now the background will start on trigger
	*/
	
	return OSAL_OK;

}

/* Procedure to request a background search for full landscape scanning
* Bearer to scan is computed autonomously
* the procedure trigger is : searched service = FFFF
*/
tSInt DABMW_ServiceFollowing_LandscapeScan()
{
				
	DABMW_SF_systemBandsTy vI_band[3] = {DABMW_BAND_NONE, DABMW_BAND_NONE, DABMW_BAND_NONE};
	tU8 vl_numBandToScan = 0;
	tBool vl_initDAB = false, vl_initFM = false;

	/* Check if we are in idle...
	* else reject the selection
	*/
	if (DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF != DABMW_serviceFollowingStatus.status)
		{
		return OSAL_ERROR;
		}

	/* update the variable compare to what is configured/supported 
	*/
	//
	// DAB not scanned for landscape : 
	// all information are already available in normal
	//
	vl_initDAB = false;
	   
	if ((true == DABMW_serviceFollowingData.fmToFmIsActive)
		 ||
	   (true == DABMW_serviceFollowingData.fmToDabIsActive))
	   {
		   vl_initFM = true;
	   }
	

	/* check at least 1 bearer is requested */
	if ((false == vl_initDAB) && (false == vl_initFM))
		{
		/* not very nice : back to previous state */
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
          
		return OSAL_ERROR;
		}

	/* init previous state & current */
    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);

	/* Set the selected tuner for initial sel selection 
	*/
	DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_NONE_APP;

	/* Set the audio config choice
	*/

	DABMW_SF_BackgroundAudioInfo.keepDecoding = SF_BACKGROUND_TUNE_KEEP_DECODING;
	DABMW_SF_BackgroundAudioInfo.injectionSide = SF_BACKGROUND_TUNE_INJECTION_SIDE;
	DABMW_SF_BackgroundAudioInfo.noOutputSwitch = SF_BACKGROUND_TUNE_NO_OUTPUT_SWITCH;

	/* Init the requested algo */
	
	DABMW_SF_BackgroundScanInfo.AFCheckRequested = false;
	DABMW_SF_BackgroundScanInfo.backgroundCheckRequested = false;
	DABMW_SF_BackgroundScanInfo.backgroundScanRequested = true;
    DABMW_SF_BackgroundScanInfo.backgroundScanDone = false;

	DABMW_SF_BackgroundScanInfo.backgroundScanSkipOriginalFreq = true;

	/* Init data for search 
	*/
	if (true == vl_initDAB)
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vI_band[vl_numBandToScan] = DABMW_ServiceFollowingGetDABBandIII();
		vl_numBandToScan++;
		}
	
	if (true == vl_initFM)
		{
		/* Init the Frequency stored in landscape to parse as a start */
		vI_band[vl_numBandToScan] =  DABMW_ServiceFollowingGetFMBand();
		vl_numBandToScan++;
		}

	/* init data for future full band scan 
	* PUT the search to INVALID_SERVICE_ID so that we do a full scan
	*/
	DABMW_ServiceFollowing_Background_ScanInit(DABMW_INVALID_SERVICE_ID, vI_band[0],  vI_band[1], vI_band[2]);


	/* this will be a background scan, no step to AF or background check
	*/
	/* Initialize the new state that will be entered
	*/
	// the timing are set at the end
	// no update
	//
	//DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
    DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);

                     /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                        /* END TMP LOG */


        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, 
                                    "DABMW_Service_Following (Landscape Scan) : start scanning Lanscape  => DAB %s, FM %s\n",
                                    DABMW_SERVICE_FOLLOWING_LOG_BOOL(vl_initDAB),
                                    DABMW_SERVICE_FOLLOWING_LOG_BOOL(vl_initFM));                                       
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
        /* END TMP LOG */

                 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                        DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
                    /* END TMP LOG */

        
	/* this is all for the init, now the background will start on trigger
	*/
	
	return OSAL_OK;

}


/* Procedure which returns if last background search result was successful for requested PI.
* 
*/
tBool DABMW_ServiceFollowing_LastBackgroundSearchResult(tU32 vI_SearchedServiceID)
{

	tBool vl_res = false;

	/* compare if request = last searched SID
	* compare if searched result = success + PI confirmed
	*/
	if (DABMW_SF_BackgroundScanInfo.searchedServiceID == vI_SearchedServiceID)
		{
		if ((DABMW_SF_BG_RESULT_FOUND_PI_CONFIRMED == DABMW_SF_BackgroundScanInfo.succesfulSearched)
			||
			(DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_CONFIRMED == DABMW_SF_BackgroundScanInfo.succesfulSearched)
			||
			(DABMW_SF_BG_RESULT_FOUND_INITIAL_PI_CONFIRMED == DABMW_SF_BackgroundScanInfo.succesfulSearched)
			)
			{
			vl_res = true;
			}
		}

	return vl_res;

}

/* procedure to compare Sid versus requested one
* either (implicit link) the Sid match directly 
* either look for equivalent SID : ie matching the Hard link or Soft Link
*/

DABMW_SF_BG_SidPi_MatchingResultTy DABMW_ServiceFollowing_CompareSidPI(tU32 vI_refSid, tU32 vI_SidToCompare, tBool vI_bearerIsDab)
{
        
#define REGIONAL_VARIANT_MASK	0x0000F0FF
#define PI_MASK 0x0000FFFF

	DABMW_SF_BG_SidPi_MatchingResultTy vl_SidCompareResult = DABMW_SF_BG_SID_NOT_EQUAL;	
	tU8 vl_cnt;
	DABMW_SF_BackgroundLinkInfo *pl_linkList;
	tU8 vl_numId = 0;
    tU32 vl_refSid;
    tU32 vl_SidToCompare;

	// Specific handling in Lock Feature case : 
	// for FM avoid / bypass the PI check
	// 
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
		
	// if feature is locked on 2 frequencies...
	// and the SID to compare is on FM
	// consider the SID is ok.	   
	if ((true == DABMW_serviceFollowingData.lockFeatureActivated_2Freq)
		&&
		(false == vI_bearerIsDab)
		)
	{
		vl_SidCompareResult = DABMW_SF_BG_SID_EQUAL;
	}
	else
#endif
	{

	    // Change to be check if ok or not
	    // the PI FM is on 4 digits, whereas it may happen that the DAB PI is on 8 (with Eid in input)
	    // I suppose then we should add a mask to check only the corresponding digits...
	    //

	    vl_refSid = vI_refSid & PI_MASK;
	    vl_SidToCompare = vI_SidToCompare & PI_MASK;


		/* addition of the regional service variant */
		/* extract for regional service management 
		*
		*specific attention for regional provision of service : [IEC 62106:1999] section 3.2.1.6.4
		*  Specific attention for regional provision of service,
		* PI with different program coverage code may be considered identical (for service following) depending on a user setting for regional mode selection. 	
		* a receiver to use a regional on/off mode which, when a receiver is in the mode "regional off",
		* will lead to the acceptance of the PI with the differing second element, 
		*and thus permit switching to a different regional network. This option can be deactivated by choosing the mode "regional on". Then only AFs having
		*the same second element of the PI (i.e. the same programme) will be used. This should also be the case for receivers without regional on/off mode. 
		*/

		
		/* compare to reference Id */	
		if (vl_refSid == vl_SidToCompare)
			{
			vl_SidCompareResult = DABMW_SF_BG_SID_EQUAL;
			}
		/* check if regional mask to apply : only valid for FM comparison */
		else if ((false == vI_bearerIsDab)
				&&
				(false == DABMW_serviceFollowingData.regionalModeIsOn) 
				&&
				((vl_refSid & REGIONAL_VARIANT_MASK) == (vl_SidToCompare & REGIONAL_VARIANT_MASK))
				)
			{
			vl_SidCompareResult = DABMW_SF_BG_SID_REGIONAL_VARIANT;
			}
	    /* compare to the intial searched one which is kind of automatically put in service linking to be found ...*/
	    else if (DABMW_serviceFollowingData.initial_searchedPI == vl_SidToCompare)
			{
			vl_SidCompareResult = DABMW_SF_BG_SID_EQUAL_INITIAL_SEARCHED;
			}
	    else  if ((false == vI_bearerIsDab)
				&&
				(false == DABMW_serviceFollowingData.regionalModeIsOn) 
				&&
				((DABMW_serviceFollowingData.initial_searchedPI & REGIONAL_VARIANT_MASK) == (vl_SidToCompare & REGIONAL_VARIANT_MASK))
				)
	        {
			vl_SidCompareResult = DABMW_SF_BG_SID_EQUAL_INITIAL_SEARCHED_REGIONAL;
			}
		else 
			{
			/* init the list to be used */
			if (true == vI_bearerIsDab)
				{
				pl_linkList = DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB;
				vl_numId = DABMW_SF_BackgroundScanInfo.nb_LinkId_DAB;
				}
			else
				{
				pl_linkList = DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM;		
				vl_numId = DABMW_SF_BackgroundScanInfo.nb_LinkId_FM;
				}
		
			/* look if this is a link...*/	
			for (vl_cnt=0; vl_cnt < vl_numId; vl_cnt++)
				{
				if ((pl_linkList[vl_cnt].id & PI_MASK) == vl_SidToCompare)
					{
					if (true == pl_linkList[vl_cnt].IsHardLink)
						{
						vl_SidCompareResult = DABMW_SF_BG_SID_EQUIVALENT_HARD_LINK;
						}
					else
						{
						vl_SidCompareResult = DABMW_SF_BG_SID_EQUIVALENT_SOFT_LINK;
						}
					break;
					}
				/* check if regional mask to apply 
				* only valid for target bearrer to compare being FM */
				else if ((false == vI_bearerIsDab)
						&&
						(pl_linkList[vl_cnt].id & REGIONAL_VARIANT_MASK) == (vl_SidToCompare & REGIONAL_VARIANT_MASK))
					{
					if (true == pl_linkList[vl_cnt].IsHardLink)
						{
						vl_SidCompareResult = DABMW_SF_BG_SID_EQUIVALENT_HARD_LINK_REGIONAL;
						}
					else
						{
						vl_SidCompareResult = DABMW_SF_BG_SID_EQUIVALENT_SOFT_LINK_REGIONAL;
						}
					break;
					}
				}
	   
			}
	} 	

	return vl_SidCompareResult;
}

/* procedure to set search success depending on SID / PI matching
*/
DABMW_SF_BG_SearchedResultTy DABMW_ServiceFollowing_GetSearchResult(tU32 vI_Sid, tBool bearerIsDab, tBool ID_received)
{
	DABMW_SF_BG_SidPi_MatchingResultTy vl_SidCompareResult = DABMW_SF_BG_SID_NOT_EQUAL;
	DABMW_SF_BG_SearchedResultTy vl_computedSearchResult = DABMW_SF_BG_RESULT_NOT_FOUND;
	
	vl_SidCompareResult = DABMW_ServiceFollowing_CompareSidPI(DABMW_SF_BackgroundScanInfo.searchedServiceID, vI_Sid, bearerIsDab);
				
	if ((DABMW_SF_BG_SID_EQUAL == vl_SidCompareResult)
		||
		(DABMW_SF_BG_SID_REGIONAL_VARIANT == vl_SidCompareResult))
		{
		
		vl_computedSearchResult = (true == ID_received) ? DABMW_SF_BG_RESULT_FOUND_PI_CONFIRMED : DABMW_SF_BG_RESULT_FOUND_PI_NOT_CONFIRMED ;
		/* this is the end of the scan 
		*/
		}
	else if ((DABMW_SF_BG_SID_EQUIVALENT_HARD_LINK == vl_SidCompareResult)
		||
		(DABMW_SF_BG_SID_EQUIVALENT_HARD_LINK_REGIONAL == vl_SidCompareResult))
		{
		vl_computedSearchResult = (true == ID_received) ? DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_CONFIRMED : DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_NOT_CONFIRMED;
		/* this is the end of the scan 
		*/
		}
	else if ((DABMW_SF_BG_SID_EQUIVALENT_SOFT_LINK == vl_SidCompareResult)
		||
		(DABMW_SF_BG_SID_EQUIVALENT_SOFT_LINK_REGIONAL == vl_SidCompareResult))
		{
		vl_computedSearchResult = (true == ID_received) ? DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_CONFIRMED : DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_NOT_CONFIRMED;
		/* this is the end of the scan 
		*/
		}
    else if ((DABMW_SF_BG_SID_EQUAL_INITIAL_SEARCHED == vl_SidCompareResult)
            ||
            (DABMW_SF_BG_SID_EQUAL_INITIAL_SEARCHED_REGIONAL == vl_SidCompareResult))
            {
            vl_computedSearchResult = (true == ID_received) ? DABMW_SF_BG_RESULT_FOUND_INITIAL_PI_CONFIRMED : DABMW_SF_BG_RESULT_FOUND_INITIAL_PI_NOT_CONFIRMED;
            /* this is the end of the scan 
                    */
            }

	return vl_computedSearchResult;
}

/* Retrieve the DAB_III band to use depending on country set
*/

DABMW_SF_systemBandsTy DABMW_ServiceFollowingGetDABBandIII()
{
	DABMW_SF_mwCountryTy vl_country = DABMW_ServiceFollowing_ExtInt_GetCountry();
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_DAB_III;

	if (DABMW_COUNTRY_CHINA == vl_country)
		{
		vl_band = DABMW_BAND_CHINA_DAB_III;
		}
	else if (DABMW_COUNTRY_KOREA == vl_country)
		{
		vl_band = DABMW_BAND_KOREA_DAB_III;
		}

	return vl_band;
}

/* Retrieve the DAB_L band to use depending on country set
*/

DABMW_SF_systemBandsTy DABMW_ServiceFollowingGetDABBandL()
{
	DABMW_SF_mwCountryTy vl_country = DABMW_ServiceFollowing_ExtInt_GetCountry();
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_DAB_L;

	if (DABMW_COUNTRY_CANADA == vl_country)
		{
		vl_band = DABMW_BAND_CANADA_DAB_L;
		}

	return vl_band;
}

/* Retrieve the FM band to use depending on country set
*/
DABMW_SF_systemBandsTy DABMW_ServiceFollowingGetFMBand()
{
	DABMW_SF_mwCountryTy vl_country = DABMW_ServiceFollowing_ExtInt_GetCountry();
	DABMW_SF_systemBandsTy vl_band = DABMW_BAND_FM_EU;

	if (DABMW_COUNTRY_EUROPE == vl_country)
		{
		vl_band = DABMW_BAND_FM_EU;
		}
	else if (DABMW_COUNTRY_USA == vl_country)
		{
		vl_band = DABMW_BAND_FM_US;
		}
	else if (DABMW_COUNTRY_JAPAN == vl_country)
		{
		vl_band = DABMW_BAND_FM_JAPAN;
		}
	else if (DABMW_COUNTRY_EAST_EU == vl_country)
		{
		vl_band = DABMW_BAND_FM_EAST_EU;
		}

	return vl_band;
}

// Procedure to retrieve the band step and band infor table
tU32 DABMW_ServiceFollowing_GetBandInfoTable_step (DABMW_SF_systemBandsTy vI_bandValue, DABMW_SF_mwCountryTy vI_countryId)
{
    tU32 vl_res;

    vl_res = DABMW_GetBandInfoTable_step(vI_bandValue, vI_countryId);

    // codex #307374
    // specific handling for FM
    //
    // the minimum band step should be 100 kHz because the RDS is at +57 kHz...  this leads to false detection 
    // the lower step (50KHz..) has sense only if the manufacturers wants to provide a fine search in an area with no rds stations 
    // and it is a manual seek where you stop looking only to FM quality parameters (SMeter, MP, Adj, Detune) not checking rds 
    //
    // 
    if ((vl_res != DABMW_INVALID_DATA) && (true == DABMW_ServiceFollowing_ExtInt_IsFMBand(vI_bandValue)))
        {
        if (vl_res < DABMW_SF_MIN_FREQ_BAND_STEP)
            {
            vl_res = DABMW_SF_MIN_FREQ_BAND_STEP;           
            }
        }

   return vl_res;
}



/* Built and store the hard linking information on a given Sid
* should be the searched one
* this is based on original stored one.
* DAB only...
* FM : reuse the stored one...
*/ 
tSInt DABMW_ServiceFollowing_BuildLinkIdList(DABMW_SF_mwAppTy vI_app, tU32 vI_ServiceId, tBool vI_dabRequested, tBool vI_fmRequested)
{
    tSInt vl_res = OSAL_OK;
	tSInt vl_resDb;
	DABMW_BrDbLinkageSetTy *pl_linkageSetPtr = (DABMW_BrDbLinkageSetTy *)NULL;
    tU32 vl_linkageSetSize = 0; // lint
	tU32 vl_filter = DABMW_BRDB_LS_GET_HARDLINK | DABMW_BRDB_LS_GET_ACTIVE;
	tU8	 vl_numId = 0, vl_indexNumId, vl_indexNumFreq, vl_indexNumLink;

	/* add Soft Link if supported */
	if (true == DABMW_serviceFollowingData.followSoftLinkage)
		{
		vl_filter |= DABMW_BRDB_LS_GET_SOFTLINK;
		}

    /* release all existing one's if needed */
	DABMW_ServiceFollowing_LinkInfo_Release();
    

	/* if app is not DAB then no point to continue */
	if (false == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_app))
		{
   
		return OSAL_ERROR;
	    }

	if ((false == DABMW_serviceFollowingData.followHardLinkage) 
		&& (false == DABMW_serviceFollowingData.followSoftLinkage)
		)
		{
		/* nothing to build */
		return OSAL_ERROR;
		}

	/*
 	* Call the procedure to retrieve the linkage set
 	*
 	*	
 	* 
	* When called with *lsarray==NULL the function scans the BrDb and LandscapeDb for
 	* Linkage Sets containing the <id> of <kindOfId>, mallocs an array of size <*size> and fills it.
	*   
	* <filter> is a combination of:
	*  DABMW_BRDB_LS_GET_HARDLINK - return the hard links only
	*  DABMW_BRDB_LS_GET_SOFTLINK - return the soft links only
	*  DABMW_BRDB_LS_GET_ACTIVE   - return the active links only
	*  DABMW_BRDB_LS_GET_INACTIVE - return the inactive links only
	* default (<filter>==0) is to return all LS, hard and soft, active and not active.
	*
	*/

    // 1st part : retrieve the SID service linking
    // applicable only if DAB requested (ie DAB to DAB supported)
    //
    if (true == vI_dabRequested)
    {
    	vl_resDb = DABMW_ServiceFollowing_ExtInt_BrDbLinkageSetBuildCheckArray (vI_app, vI_ServiceId, DABMW_ID_IS_SID_TYPE, 
    													&pl_linkageSetPtr, (tPU32)&vl_linkageSetSize, vl_filter );
    	

    	/* check the result */	
    	if ((OSAL_ERROR == vl_resDb)
    		||
    		((DABMW_BrDbLinkageSetTy *)NULL == pl_linkageSetPtr)
    		)
    		{
                if (NULL != pl_linkageSetPtr)
                    {
                        DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_linkageSetPtr);
                    }
                    
    		/* no link Sid on DAB side */
    		DABMW_SF_BackgroundScanInfo.DAB_linkageSet_size = 0;
    		DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr = NULL;
    		
    		/* stored at list the current one !!*/
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(DABMW_SF_BackgroundLinkInfo) * 1);

    		if (NULL == DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB )
    			{											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
    			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    						

    			return OSAL_ERROR;
    			}
    		
    		/* ADD the current one if not in */
    		DABMW_SF_BackgroundScanInfo.nb_LinkId_DAB = 1;
    		vl_numId = 0;
    		
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].id = DABMW_serviceFollowingStatus.originalSid;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].eid = DABMW_serviceFollowingStatus.originalEid;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IsHardLink = true;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].kindOfId = DABMW_ID_IS_SID_TYPE;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IdIsInLandscape = false;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].frequency = DABMW_serviceFollowingStatus.originalFrequency;
    		vl_numId++;	
    		}
    	else
    		{
    		/* we get something */
    		
    		DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr = pl_linkageSetPtr;
    		/* add 1 : to make sure at list the current camped freq is stored */
    		DABMW_SF_BackgroundScanInfo.DAB_linkageSet_size = vl_linkageSetSize;

    		/* while waiting for chain list : count number 1st for accurate allocation 
    		* organisation : 
    		*
    		* number of linkageSet = vl_linkageSetSize
    		* For each Linkage Set : 
    		* number of Sid = size in linkage set ...
    		* for each ID
    		* number of frequency = frequencyArray.size
    		* 
    		*/
    		
    		vl_numId = 0;
    		
    		for (vl_indexNumLink=0;vl_indexNumLink < vl_linkageSetSize; vl_indexNumLink++)
    			{
    			if (true == pl_linkageSetPtr[vl_indexNumLink].la)
    				{
    				for (vl_indexNumId=0;vl_indexNumId < pl_linkageSetPtr[vl_indexNumLink].size;vl_indexNumId++)
    					{
    					if (pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId] == DABMW_ID_IS_SID_TYPE)
    						{
    						if (pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size > 0)
    							{
                                // ER codex : 300846 
                                // we should increment the vl_numId with the num of freq
                                // before
                                // vl_numId = pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size;
                                // after
    							vl_numId += pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size;
    							}
    						else
    							{
    							vl_numId++;
    							}
    						}
    					}
    				}
    			}

    		/* fill the number of linkID_DAB */
    		/* add 1 because addition of current ID */
    		DABMW_SF_BackgroundScanInfo.nb_LinkId_DAB = vl_numId+1;

    		/* end count : now fill */
    		
    		/* fill the DAB Linked ID List */
    		/* TODO : clarifiy how it works... 
    		* possibly we need an other loop on linkageSet size if several linkage in the set 
    		* Create a chain list else it will be difficult...
    		*/
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(DABMW_SF_BackgroundLinkInfo) * (vl_numId + 1));

    		if (NULL == DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB )
    			{											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
    			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    						
    		
    			return OSAL_ERROR;
    			}

    		vl_numId = 0;

    		for (vl_indexNumLink=0;vl_indexNumLink < vl_linkageSetSize; vl_indexNumLink++)
    			{
    			if (true == pl_linkageSetPtr[vl_indexNumLink].la)
    				{
    				for (vl_indexNumId=0;vl_indexNumId < pl_linkageSetPtr[vl_indexNumLink].size;vl_indexNumId++)
    					{
    					/* check the Kind of ID : only SID here */
    					if (pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId] == DABMW_ID_IS_SID_TYPE)
    						{
    						if (pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size > 0)
    							{
    							for (vl_indexNumFreq=0;vl_indexNumFreq < pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size; vl_indexNumFreq++)
    								{
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].id = pl_linkageSetPtr[vl_indexNumLink].id[vl_indexNumId];
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].eid = pl_linkageSetPtr[vl_indexNumLink].eid[vl_indexNumId];
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IsHardLink = (true == pl_linkageSetPtr[vl_indexNumLink].sh)?true:false;
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].kindOfId = pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId];
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IdIsInLandscape = false;
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].frequency = pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].freqList[vl_indexNumFreq].freq;					
    								vl_numId++;
    								}
    							}
    						else
    							{
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].id = pl_linkageSetPtr[vl_indexNumLink].id[vl_indexNumId];
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].eid = pl_linkageSetPtr[vl_indexNumLink].eid[vl_indexNumId];
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IsHardLink = (true == pl_linkageSetPtr[vl_indexNumLink].sh)?true:false;
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].kindOfId = pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId];
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IdIsInLandscape = false;
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].frequency = DABMW_INVALID_FREQUENCY;	
    							vl_numId++;
    							}
    						}
    					}
    				}
    			}

    		/* ADD the current one if not in */
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].id = DABMW_serviceFollowingStatus.originalSid;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].eid = DABMW_serviceFollowingStatus.originalEid;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IsHardLink = (true == pl_linkageSetPtr->sh)?true:false;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].kindOfId = DABMW_ID_IS_SID_TYPE;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].IdIsInLandscape = false;
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB[vl_numId].frequency = DABMW_serviceFollowingStatus.originalFrequency;
            vl_numId++;

    		}	
        
    }
    
	/* reset the Linkage point to retrieve the FM info now */
	pl_linkageSetPtr = NULL;
	vl_linkageSetSize = 0;

    // check the service linking for FM
    // note : only if DAB to FM is supported...
    //

    if (true == vI_fmRequested)
    {
    	/* do the same for FM */
    	vl_resDb = DABMW_ServiceFollowing_ExtInt_BrDbLinkageSetBuildCheckArray (vI_app, vI_ServiceId, DABMW_ID_IS_RDS_PI_CODE_TYPE, 
    													&pl_linkageSetPtr, (tPU32)&vl_linkageSetSize, vl_filter);
    	
    	if ((OSAL_ERROR == vl_resDb)
    		||
    		((DABMW_BrDbLinkageSetTy *)NULL == pl_linkageSetPtr)
    		)
    		{	
                if (NULL != pl_linkageSetPtr)
                    {
                        DABMW_ServiceFollowing_ExtInt_MemoryFree(pl_linkageSetPtr);
                    }
                
    		/* no link Sid on DAB side */
    		DABMW_SF_BackgroundScanInfo.FM_linkageSet_size = 0;
    		DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr = NULL;
    		DABMW_SF_BackgroundScanInfo.nb_LinkId_FM = 0;
    		
    		}
    	else
    		{
    		/* we get something */
    		DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr = pl_linkageSetPtr;
    		DABMW_SF_BackgroundScanInfo.FM_linkageSet_size = vl_linkageSetSize;
    		
    		/* fill the FM Linked ID List */

    		/* while waiting for chain list : count number 1st for accurate allocation 
    		* organisation : 
    		*
    		* number of linkageSet = vl_linkageSetSize
    		* For each Linkage Set : 
    		* number of Sid = size in linkage set ...
    		* for each ID
    		* number of frequency = frequencyArray.size
    		* 
    		*/
     		vl_numId = 0;
            
    		for (vl_indexNumLink=0;vl_indexNumLink < vl_linkageSetSize; vl_indexNumLink++)
    			{
    			if (true == pl_linkageSetPtr[vl_indexNumLink].la)
    				{
    				for (vl_indexNumId=0;vl_indexNumId < pl_linkageSetPtr[vl_indexNumLink].size;vl_indexNumId++)
    					{
    					/* check the Kind of ID : only RDS here */
    					if (pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId] == DABMW_ID_IS_RDS_PI_CODE_TYPE)
    						{
    						if (pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size > 0)
    							{
                                // ER codex : 300846 
                                // we should increment the vl_numId with the num of freq
                                // before 
                                //
                                // vl_numId = pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size;
                                // after
                                vl_numId += pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size;
    							}
    						else
    							{
    							vl_numId++;
    							}
    						}
    					}
    				}
    			}

    		/* fill the DAB Linked ID List */
    		/* TODO : clarifiy how it works... 
    		* possibly we need an other loop on linkageSet size if several linkage in the set 
    		* Create a chain list else it will be difficult...
    		*/

    		/* allocate structure */
    		DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM = DABMW_ServiceFollowing_ExtInt_MemoryAllocate(sizeof(DABMW_SF_BackgroundLinkInfo) 
    																				* (vl_numId) );
    		
    		if (NULL == DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM )
    			{											 
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
    			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: allocation failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    						
    			return OSAL_ERROR;
    			}
    		
    		/* reset the number now to count again */
    		vl_numId = 0;

    		/* fill it */
    		for (vl_indexNumLink=0;vl_indexNumLink < vl_linkageSetSize; vl_indexNumLink++)
    			{
    			if (true == pl_linkageSetPtr[vl_indexNumLink].la)
    				{
    				for (vl_indexNumId=0;vl_indexNumId < pl_linkageSetPtr[vl_indexNumLink].size;vl_indexNumId++)
    					{
    					/* check the Kind of ID : only RDS here */
    					if (pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId] == DABMW_ID_IS_RDS_PI_CODE_TYPE)
    						{
    						if (pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size > 0)
    							{
    							for (vl_indexNumFreq=0;vl_indexNumFreq < pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].size ;vl_indexNumFreq++)
    								{
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].id = pl_linkageSetPtr[vl_indexNumLink].id[vl_indexNumId];
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].eid = pl_linkageSetPtr[vl_indexNumLink].eid[vl_indexNumId];
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].IsHardLink = (true == pl_linkageSetPtr[vl_indexNumLink].sh)?true:false;
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].kindOfId = pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId];
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].IdIsInLandscape = false;
    								DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].frequency = pl_linkageSetPtr[vl_indexNumLink].frequencyArray[vl_indexNumId].freqList[vl_indexNumFreq].freq;					
    								vl_numId++;
    								}
    							}
    						else
    							{
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].id = pl_linkageSetPtr[vl_indexNumLink].id[vl_indexNumId];
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].eid = pl_linkageSetPtr[vl_indexNumLink].eid[vl_indexNumId];
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].IsHardLink = (true == pl_linkageSetPtr[vl_indexNumLink].sh)?true:false;
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].kindOfId = pl_linkageSetPtr[vl_indexNumLink].kindOfId[vl_indexNumId];
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].IdIsInLandscape = false;
    							DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM[vl_numId].frequency = DABMW_INVALID_FREQUENCY;	
    							vl_numId++;
    							}
    						}
    					}
    				}
			}
		
		    DABMW_SF_BackgroundScanInfo.nb_LinkId_FM =  vl_numId;

		}
    }
   

	return vl_res;
}

/* Update the service linking information 
* set the field for Id Linking info
*/
tVoid DABMW_ServiceFollowing_LinkInfo_SetIdPresentInLandscape(tU32 vI_Sid, tU32 vI_freq, tBool vI_bearerIsDab)
{
	tU8 vl_cnt;
	DABMW_SF_BackgroundLinkInfo *pl_linkList;
	tU8 vl_numId= 0;


	/* init the list to be used */
	if (true == vI_bearerIsDab)
		{
		pl_linkList = DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB;
		vl_numId = DABMW_SF_BackgroundScanInfo.nb_LinkId_DAB;
		}
	else
		{
		pl_linkList = DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM;		
		vl_numId = DABMW_SF_BackgroundScanInfo.nb_LinkId_FM;
		}

	
	/* fill it */
	for (vl_cnt=0; vl_cnt < vl_numId; vl_cnt++)
		{
		if ((pl_linkList[vl_cnt].id == vI_Sid) && (pl_linkList[vl_cnt].frequency == vI_freq))
			{
			pl_linkList[vl_cnt].IdIsInLandscape = true;
			}
		}
	
	return;
}

/* Get the frequency associated to a service ID
*/
tU32 DABMW_ServiceFollowing_LinkInfo_GetFrequencyFromId(tU32 vI_Sid, tBool serviceLinking_isDab)
{
	tU8 vl_cnt;
	DABMW_BrDbLinkageSetTy *pl_linkList;
	tU8 vl_numId= 0;
	tU32 vl_freq = DABMW_INVALID_FREQUENCY;

	
	if (true == serviceLinking_isDab)
		{
		pl_linkList = DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr;
		if (NULL != pl_linkList)
			{
			vl_numId = DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr->size;
			}
		}
	else
		{
		pl_linkList = DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr;		
		if (NULL != pl_linkList)
			{
			vl_numId = DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr->size;
			}
		}


	/* fill it */
	if (NULL != pl_linkList)
		{
		for (vl_cnt=0; vl_cnt < vl_numId; vl_cnt++)
			{
			if (pl_linkList->id[vl_cnt] == vI_Sid)
				{
				/* Matching ID */
				/* get Freq... */
				/* note : this is to improve to get more than 1 freq */
				vl_freq = pl_linkList->frequencyArray[vl_cnt].freqList[0].freq;
				}
			}
		}
	
	return vl_freq;
}


/* Init Service Linking infos
*/
tVoid DABMW_ServiceFollowing_LinkInfo_Init()
{

	/* init the Link ones.. */
	DABMW_SF_BackgroundScanInfo.DAB_linkageSet_size = 0;
	DABMW_SF_BackgroundScanInfo.FM_linkageSet_size = 0;
	DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr = NULL;
	DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr = NULL;

	/* release all existing one's if needed */
	DABMW_SF_BackgroundScanInfo.nb_LinkId_DAB = 0;
	DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB = NULL;
	DABMW_SF_BackgroundScanInfo.nb_LinkId_FM = 0;
	DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM = NULL;

return;
}

/* procedure to release any Link Info
*/

tVoid DABMW_ServiceFollowing_LinkInfo_Release()
{
 	
	/* release all existing one's if needed */
	DABMW_SF_BackgroundScanInfo.DAB_linkageSet_size = 0;
	
	if (NULL != DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr)
		{
		DABMW_ServiceFollowing_ExtInt_MemoryFree(DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr);
		DABMW_SF_BackgroundScanInfo.p_DAB_linkageSetPtr = NULL;
		}

	DABMW_SF_BackgroundScanInfo.FM_linkageSet_size = 0;
	
	if (NULL != DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr)
		{
		DABMW_ServiceFollowing_ExtInt_MemoryFree(DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr);
		DABMW_SF_BackgroundScanInfo.p_FM_linkageSetPtr = NULL;
		}

	/* release all existing one's if needed */
	DABMW_SF_BackgroundScanInfo.nb_LinkId_DAB = 0;
		
	if (NULL != DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB)
		{
		DABMW_ServiceFollowing_ExtInt_MemoryFree(DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB);
		DABMW_SF_BackgroundScanInfo.p_LinkIdList_DAB = NULL;
		}

	/* release all existing one's if needed */
	DABMW_SF_BackgroundScanInfo.nb_LinkId_FM = 0;
		
	if (NULL != DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM)
		{
		DABMW_ServiceFollowing_ExtInt_MemoryFree(DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM);
		DABMW_SF_BackgroundScanInfo.p_LinkIdList_FM = NULL;
		}
}


// Procedure to stop/break the VPA for freeing the bg tuner.
//
tVoid DABMW_ServiceFollowing_AmFMStopVpa(DABMW_SF_mwAppTy vI_app)
{
	// update the status 
	
    if ((true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(vI_app))
        &&(true == DABMW_SF_BackgroundScanInfo.VPA_IsOnAtStart)
        && (false == DABMW_SF_BackgroundScanInfo.VPA_HasBeenBroken)
        )
        {
        // we need to break the VPA.
		DABMW_ServiceFollowing_ExtInt_DisableVPA();

        DABMW_SF_BackgroundScanInfo.VPA_HasBeenBroken = true;

        }
    else
        {
        // nothing to do
        }

    return;
    
}


// Procedure to stop/break the VPA for freeing the bg tuner.
//
tVoid DABMW_ServiceFollowing_AmFMRestartVpa()
{
 
    if ((true == DABMW_SF_BackgroundScanInfo.VPA_IsOnAtStart)
        && (true == DABMW_SF_BackgroundScanInfo.VPA_HasBeenBroken)
        )
        {
		DABMW_ServiceFollowing_ExtInt_EnableVPA();   

		
		DABMW_SF_BackgroundScanInfo.VPA_HasBeenBroken = true;
        }
    else
        {
        // nothing to do
        }

    return;
    
}


/* procedure to re-init the background scan if interrupted
*/
tSInt DABMW_ServiceFollowing_RestartBackgroundScan()
{

   	tSInt vl_tmp_res;
    DABMW_SF_systemBandsTy vl_band;   					
	DABMW_SF_mwCountryTy vl_country = DABMW_ServiceFollowing_ExtInt_GetCountry();

	/* Check if we are in idle...
	* else reject the selection
	*/
	if (DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF != DABMW_serviceFollowingStatus.status)
		{
		return OSAL_ERROR;
		}

		 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
		DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
								  /* END TMP LOG */
	
	
				  /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, 
									"DABMW_Service_Following (DABMW_ServiceFollowing_RestartBackgroundScan) : resume scanning for AF for PI 0x%04x => app = %d, freq from start = %d \n",
									DABMW_SF_BackgroundScanInfo.searchedServiceID,
									DABMW_SF_BackgroundScanInfo.lastBgScanApp,
									DABMW_SF_BackgroundScanInfo.lastBgFreq
									);										 
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
				   /* END TMP LOG */
	
		 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
		 DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
		/* END TMP LOG */

	/* Init structure for FM & DAB => Is already done in prior scan : nothing more
	*/

	/* init previous state & current should already be ok*/
    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);

    // Init search result 
    DABMW_SF_BackgroundScanInfo.succesfulSearched = DABMW_SF_BG_RESULT_NOT_FOUND;
    
	/* Set the selected tuner for initial sel selection 
        * this should already be set correctly
        */
        
	
	/* Init the requested algo : was already done in prior scan */
	

	/* Init data for search : was already up to date
	*/

	/* init data for future full band scan : 
	* here only one thing to be done : change the start frequency if background scan was on-going, to resume it 
	* in order to resume it to prior found one.
	* this is mandatory in FM only 
	*/

    // manage the "tuner association" if needed 
    // should not be needed
    DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_SF_BackgroundScanInfo.lastBgScanApp;
    DABMW_SF_BackgroundScanInfo.bg_Freq = DABMW_SF_BackgroundScanInfo.lastBgFreq;
    //
    // ho ho, we may need to change the tuner to be used
    //
    // use case : we have found an alternate on previous scan, so we should use an other app


 	/* find and associate the tuner
	*/		
	vl_band = DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand (DABMW_SF_BackgroundScanInfo.bg_Freq); 

	/* Select the new best suited app */
	DABMW_SF_BackgroundScanInfo.bgScan_App = DABMW_ServiceFollowing_Background_SelectApp(vl_band);

	if (DABMW_NONE_APP != DABMW_SF_BackgroundScanInfo.bgScan_App)
	{
    	vl_tmp_res = OSAL_OK;

		/* Tuner Association here is not needed : 
		  * done by TuneFrequency... 
		  *
		  * vl_tmp_res = DABMW_ConfigureTunerForSpecificApp (DABMW_SF_BackgroundScanInfo.bgScan_App, vl_band, vl_currentFreq);
		*/
	}
	else 
	{
		vl_tmp_res = OSAL_ERROR;
	}
					
	if (OSAL_ERROR == vl_tmp_res)
	{
	    DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW Error (DABMW_ServiceFollowing_RestartBackgroundScan) failed to get a tuner, ScanApp = %d, Band = %d, startFreq = %d\n",
											DABMW_SF_BackgroundScanInfo.bgScan_App, vl_band, DABMW_SF_BackgroundScanInfo.bg_Freq);

		DABMW_SF_BackgroundScanInfo.bg_Freq = DABMW_INVALID_FREQUENCY;
        return OSAL_ERROR;
	}
	else 
	{
	    /* If the APP / band is FM : enable RDS 
		*/
        if (DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_SF_BackgroundScanInfo.bgScan_App))
        {
            // We may need to stop the VPA during the bg search
            //
            DABMW_ServiceFollowing_AmFMStopVpa(DABMW_SF_BackgroundScanInfo.bgScan_App);
                  
            // RDS should already be enable but let's check in the procedure
            // if Landscape is on going, then this is a 'normal' PI request
            if (false == DABMW_ServiceFollowing_IsLanscapeScanOnGoing())
            	{
            	DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, true);
            	}
			else
            	{
            	DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, false);
            	}
			
        }
	}
     
               
	// what is the next step
    if ((true == DABMW_SF_BackgroundScanInfo.AFCheckRequested)
        && (false == DABMW_SF_BackgroundScanInfo.AFCheckdone))
        {
            // AF check
            /* Initialize the new state that will be entered
    		        */
    	    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_AF_CHECK);
            DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_AF_CHECK);
        }
    else if ((true == DABMW_SF_BackgroundScanInfo.backgroundCheckRequested)
        && (false == DABMW_SF_BackgroundScanInfo.backgroundCheckDone))
        {
            /* Initialize the new state that will be entered
    		        */
    		 DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
             DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_CHECK);
        }
	else
		{
		/* this will be a background scan, no step to AF or background check
		*/
		// init the data for FM case : 
		/* If the APP / band is FM : enable RDS 
			*/
			if (DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_SF_BackgroundScanInfo.lastBgScanApp))
				{
                DABMW_SF_BackgroundScanInfo.startFreq = DABMW_SF_BackgroundScanInfo.lastBgFreq;
                
				/*	tuned on starting freq , and explicit tune is needed.
				* ~1st init for tuner to be ready.
				*/


				/* tSInt DABMW_TuneFrequency (DABMW_SF_mwAppTy app, tU32 frequency, tBool keepDecoding,
                        	*							  tU8 injectionSide, tBool noOutputSwitch, tBool isInternalTune)
                        	* During 'scan' no output switch : only when ended.
                            */
				vl_tmp_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency (DABMW_SF_BackgroundScanInfo.bgScan_App, 
												DABMW_SF_BackgroundScanInfo.startFreq);


				/* check tune ok */
				if (OSAL_OK == vl_tmp_res)
					{
					// Set seek data, direction up
					DABMW_ServiceFollowing_ExtInt_SetAutoSeekData (DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.startFreq, DABMW_SF_BackgroundScanInfo.stopFreq, true, 
										DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin(vl_band, vl_country), DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin(vl_band, vl_country));
					}
				else
					{
					/* tune failed */
					}
				}
            
		/* Initialize the new state that will be entered
		*/
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
        DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(DABMW_SF_STATE_BACKGROUND_SCAN);
	    }



	/* this is all for the init, now the background will start on trigger
	*/
	
	return OSAL_OK;

}

/* procedure to End and set back field before leaving baackground scan()
*/
tVoid DABMW_ServiceFollowing_EndBackgroundScan()
{

    //End the auto-seek if needed
    DABMW_SF_AutoSeekEnd();

    // end the AF start if needed
    DABMW_SF_AmFmAFEndRequest();
    
    // re-init the VPA if interupted
    DABMW_ServiceFollowing_AmFMRestartVpa();

    // save information

    //last frequency attempted
    if (true == DABMW_ServiceFollowing_LastBackgroundSearchResult(DABMW_SF_BackgroundScanInfo.searchedServiceID))
    {
        DABMW_SF_BackgroundScanInfo.lastBgFreq = DABMW_SF_BackgroundScanInfo.bg_Freq;
        DABMW_SF_BackgroundScanInfo.lastBgScanApp = DABMW_SF_BackgroundScanInfo.bgScan_App;
    }
    else
    {
        DABMW_SF_BackgroundScanInfo.lastBgFreq = DABMW_INVALID_FREQUENCY;
        DABMW_SF_BackgroundScanInfo.lastBgScanApp = DABMW_NONE_APP;
    }


	// Reset the fast_PI which was set if we were on FM
	if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_SF_BackgroundScanInfo.bgScan_App))
	{
		DABMW_ServiceFollowing_ExtInt_RdsDataSetupPiDetectionMode(DABMW_SF_BackgroundScanInfo.bgScan_App, false);
	}
	
	
	// when we have few app, we need to the one used
	// if not a main one.
	// even with numerous tuner, we should free unused tuner for the other users
	//
	
//#ifndef	CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM

#if 1
	// basically : if  not found
	// or if found, but FM case, found on bg.. the main-amfm will be used instead
	// if main amfm : that will be used for alternate, no need to reset
	// if dab = that will be used for alternate

	if ((DABMW_INVALID_FREQUENCY == DABMW_SF_BackgroundScanInfo.lastBgFreq)
		||
		(DABMW_SF_BackgroundScanInfo.bgScan_App == DABMW_BACKGROUND_AMFM_APP)
		)
	{
		// reset the tuner to free it
		DABMW_ServiceFollowing_ResetTuner(DABMW_SF_BackgroundScanInfo.bgScan_App);	
	}
	else
	{
		// set back the tuner to correct RDS mode
		//
		if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_SF_BackgroundScanInfo.bgScan_App))
		{
			DABMW_ServiceFollowing_EnableRds(DABMW_SF_BackgroundScanInfo.bgScan_App, false);
		}
	}
	
#endif

	
    return;
}

// Procedures to retrieve some bg scan info for logging
//
DABMW_serviceFollowingLogInfoBgSearchProcTy DABMW_ServiceFollowing_BackgroundLastSearchLogInfo() 
{

    DABMW_serviceFollowingLogInfoBgSearchProcTy vl_logInfo;
    
    vl_logInfo.field.AFCheckdone = (DABMW_SF_BackgroundScanInfo.AFCheckdone == true)?1:0;
    vl_logInfo.field.AFCheckRequested = (DABMW_SF_BackgroundScanInfo.AFCheckRequested == true)?1:0;
    vl_logInfo.field.backgroundCheckDone = (DABMW_SF_BackgroundScanInfo.backgroundCheckDone == true)?1:0;
    vl_logInfo.field.backgroundCheckRequested = (DABMW_SF_BackgroundScanInfo.backgroundCheckRequested == true)?1:0;
    vl_logInfo.field.backgroundScanDone = (DABMW_SF_BackgroundScanInfo.backgroundScanDone == true)?1:0;
    vl_logInfo.field.backgroundScanRequested = (DABMW_SF_BackgroundScanInfo.backgroundScanRequested == true)?1:0;

    return (vl_logInfo);
}

//
// Procdure which return the path to be used for Alternate measurements...
// if background scan thru auto-seek in FM is on-going... this cannot be interupted...
// it required 
//
tBool DABMW_ServiceFollowing_SuspendBgForAlternateFMMeasurement(DABMW_SF_mwAppTy vI_app)
{
    tBool vl_bgActivityOnGoing = false;
    tBool vl_res = false;
    
    // if background scan on-going for FM => we need to use an other app
    //

    switch (DABMW_serviceFollowingStatus.status)
            {
                case DABMW_SF_STATE_BACKGROUND_CHECK:
        		case DABMW_SF_STATE_BACKGROUND_WAIT_PI:
				case DABMW_SF_STATE_BACKGROUND_WAIT_PS:
        		case DABMW_SF_STATE_BACKGROUND_WAIT_DAB:
        		case DABMW_SF_STATE_BACKGROUND_SCAN:
        		case DABMW_SF_STATE_BACKGROUND_WAIT_FIC:
        		case DABMW_SF_STATE_AF_CHECK:
        		case DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK:
                    vl_bgActivityOnGoing = true;
                    break;
                default:
                    vl_bgActivityOnGoing = false;
                    break;
            }

    if (false == vl_bgActivityOnGoing)
    {
        // no bg activity on going : so no problem
        vl_res = true;
    }
    else if (DABMW_SF_BackgroundScanInfo.bgScan_App != vI_app)
    {
    	// Here what we should check is not the APP but the Tuner associated 
    	// in 2 tuner case, a tuner may be used for APP FM or DAB
    	//
        // bg scan on going on different tuner : no problem
        // check the tuner on each APP
 #ifdef CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM       
        vl_res = true;
 #else

		 // it means suspension for FM activity is requested, 
		 // this is accepted only if we are in FM for the moment, and we will recover in FM !!
		 // other cases will lead to bad situations !!
 		if ((true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_SF_BackgroundScanInfo.bgScan_App))
			&&
			(true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(vI_app))
			)
		{
			// we are in FM-FM case 
			// tuner is shared...
			// we can allow the procedure 
			// stop seek if needed
   		 	vl_res = true;

			if (true == DABMW_SF_BackgroundScanInfo.seek_ongoing)
			{
				// background scan procedure not compatible with AF check on the path
		        // interrupt the seek
		        DABMW_SF_AutoSeekEnd();
			}
 		}
 		else
 		{	
			// it means a DAB activity is on-going on the bg tuner 
			// of a suspension is request for a DAB activity 
			// Tuner being shared, this is not ok for now
 			vl_res = false;
 		}
 #endif
    }
    else if (false == DABMW_SF_BackgroundScanInfo.seek_ongoing)
    { 
    	// no seek on going : AF check ok.
        vl_res = true;
    }
	else if (true == DABMW_SF_BackgroundScanInfo.seek_ongoing)
	{
		// background scan procedure not compatible with AF check on the path
        // interrupt the seek
        DABMW_SF_AutoSeekEnd();
		
        vl_res = true;
	}
    else
    {
        // here we cannot....
        vl_res = false;
    }


			
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
										DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );							  
										DABMW_SF_PRINTF( TR_LEVEL_USER_1, "DABMW_Service_Following (DABMW_ServiceFollowing_SuspendBgAfterAlternateFMMeasurement) :	result %d \n", vl_res);
										DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif //    defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)                   

    return vl_res;
    
}

tVoid DABMW_ServiceFollowing_ResumeBgAfterAlternateFMMeasurement()
{

	// resume the seek if needed....
	  switch (DABMW_serviceFollowingStatus.status)
            {
        		case DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK:    
                                        
                    /* Set the autoseek callback
                                     */
					DABMW_SF_AmFmLearnFrequencyAutoSeekImmediate(DABMW_SF_BackgroundScanInfo.bgScan_App, DABMW_SF_BackgroundScanInfo.bandStep);                   

#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
                       
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "DABMW_Service_Following (DABMW_ServiceFollowing_ResumeBgAfterAlternateFMMeasurement) :  Resume AutoSeek for AF check\n");
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif //    defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)                   
                    break;
                default:
                    break;
            }
}

//
// Procdure to stop/interupt the bg scan, 
// an other procedure higher priority is needed to be done
//
tSInt DABMW_ServiceFollowing_StopBgScan()
{

    tSInt vl_res = OSAL_OK;
	
    DABMW_ServiceFollowing_EndBackgroundScan();
    
	// free the tuner
	// 
	DABMW_ServiceFollowing_ResetTuner(DABMW_SF_BackgroundScanInfo.bgScan_App);


    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_serviceFollowingStatus.prevStatus);
    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.nextStatus);

        
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );                              
                            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (DABMW_ServiceFollowing_StopBgScan) : Background Scan procedure stopped\n");
                            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif //    defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)                   
           
    return vl_res;
    
}



#if defined (CONFIG_TARGET_DABMW_RDS_COMM_ENABLE)   

tSInt DABMW_ServiceFollowing_EnableRds(DABMW_SF_mwAppTy app, tBool vI_fastPI)
{
    tSInt vl_tmp_res = OSAL_OK;

	if (DABMW_MAIN_AMFM_APP == app)
		{
		// main AMFM
		// check if already activated with right settings
		//
		// For 2 tuners configuration, and reuse of tuner the prior 'settings' may have been erased ..
		// so ask again
		//
		/*
		if ((true == DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_1) && (vI_fastPI == DABMW_SF_BackgroundScanInfo.IsFastPiEnabled_app_1))
			{
			// all is good already
			}
		else
		*/
			{
			// we need to change the configuration at least
			//
			if (true == vI_fastPI)
				{
				vl_tmp_res = DABMW_ServiceFollowing_ExtInt_RdsEnable(app, vI_fastPI);


				// we should also change the DISS mode  & FILTER
				// 
				DABMW_ServiceFollowing_ExtInt_SetDISSModeDSP(app, DABMW_SF_DISS_MODE_FAST_PI);
				DABMW_ServiceFollowing_ExtInt_SetDISSFilterDSP(app, DABMW_SF_DISS_FILTER_FAST_PI);
				}
			else
				{
				vl_tmp_res = DABMW_ServiceFollowing_ExtInt_RdsEnable(app, vI_fastPI);

				DABMW_ServiceFollowing_ExtInt_SetDISSModeDSP(app, DABMW_SF_DISS_MODE_NORMAL_PI);
				DABMW_ServiceFollowing_ExtInt_SetDISSFilterDSP(app, DABMW_SF_DISS_FILTER_NORMAL_PI);	
				}
			
			if (OSAL_OK == vl_tmp_res )
				{
				DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_1 = true;
				DABMW_SF_BackgroundScanInfo.IsFastPiEnabled_app_1 = vI_fastPI;
				DABMW_ServiceFollowing_ExtInt_RdsDataSetupPiDetectionMode(DABMW_MAIN_AMFM_APP, vI_fastPI);
				}
			else
				{
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
										DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: enabling of rds device failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
			  
				}
			}
			
		}
	else if (DABMW_BACKGROUND_AMFM_APP== app)
		{
		// main AMFM
		// check if already activated with right settings
		// For 2 tuners configuration, and reuse of tuner the prior 'settings' may have been erased ..
		// so ask again
		//
		/*
		if ((true == DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_2) && (vI_fastPI == DABMW_SF_BackgroundScanInfo.IsFastPiEnabled_app_2))
			{
			// all is good already
			}
		else 
		*/
			{
			// we need to change the configuration at least
			//
			if (true == vI_fastPI)
				{
				vl_tmp_res = DABMW_ServiceFollowing_ExtInt_RdsEnable(app, vI_fastPI);
				
				DABMW_ServiceFollowing_ExtInt_SetDISSModeDSP(app, DABMW_SF_DISS_MODE_FAST_PI);
				DABMW_ServiceFollowing_ExtInt_SetDISSFilterDSP(app, DABMW_SF_DISS_FILTER_FAST_PI);
				}
			else
				{
				vl_tmp_res = DABMW_ServiceFollowing_ExtInt_RdsEnable(app, vI_fastPI);

				DABMW_ServiceFollowing_ExtInt_SetDISSModeDSP(app, DABMW_SF_DISS_MODE_NORMAL_PI);
				DABMW_ServiceFollowing_ExtInt_SetDISSFilterDSP(app, DABMW_SF_DISS_FILTER_NORMAL_PI);
		
				}
			
			if (OSAL_OK == vl_tmp_res )
				{
				DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_2 = true;
				DABMW_SF_BackgroundScanInfo.IsFastPiEnabled_app_2 = vI_fastPI;
				DABMW_ServiceFollowing_ExtInt_RdsDataSetupPiDetectionMode(DABMW_BACKGROUND_AMFM_APP, vI_fastPI);
				}
			else
				{
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
										DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW Error: enabling of rds device failed\n");
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)    
			  
				}
			}
			
		}
	

    return vl_tmp_res;
    
}   

// RDS disabling (ie suspend / resume)
//
tSInt DABMW_ServiceFollowing_DisableRds(DABMW_SF_mwAppTy app, tBool vI_fastPI)
{
	// during the 
	tSInt vl_tmp_res = OSAL_OK;

	if (DABMW_MAIN_AMFM_APP == app)
		{
		// main AMFM
		DABMW_ServiceFollowing_ExtInt_RdsDisable (app);
		DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_1 = false;
		}
	else
		{
		// main AMFM
		DABMW_ServiceFollowing_ExtInt_RdsDisable(app);
		DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_2 = false;
		}

	return vl_tmp_res;
}

// RDS disabling (ie suspend / resume)
//
tSInt DABMW_ServiceFollowing_SuspendRds(DABMW_SF_mwAppTy app)
{
	// during the 
	tSInt vl_tmp_res = OSAL_OK;

	if ((DABMW_MAIN_AMFM_APP == app) && (true == DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_1))
		{
		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOff(app);
		DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_1 = false;
		}
	else if ((DABMW_BACKGROUND_AMFM_APP == app) && (true == DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_2))
		{
		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOff(app);
		DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_2 = false;
		}

	return vl_tmp_res;
}

// RDS enabling (ie suspend / resume)
//
tSInt DABMW_ServiceFollowing_ResumeRds(DABMW_SF_mwAppTy app)
{
	// during the 
	tSInt vl_tmp_res = OSAL_OK;

	if ((DABMW_MAIN_AMFM_APP == app) && (false == DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_1))
		{
		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOn(app);
		DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_1 = true;

		}
	else if ((DABMW_BACKGROUND_AMFM_APP == app) && (false == DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_2))
		{
		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOn(app);
		DABMW_SF_BackgroundScanInfo.IsRdsEnabled_app_2 = true;

		}

	return vl_tmp_res;
}

#endif // RDS CONFIG_TARGET_DABMW_RDS_COMM_ENABLE

#ifdef __cplusplus
}
#endif

#endif //CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // SERVICE_FOLLOWING_BACKGROUND_C
// End of file
