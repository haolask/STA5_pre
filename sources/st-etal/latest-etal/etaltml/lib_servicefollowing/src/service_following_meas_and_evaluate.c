//!
//!  \file      service_following_meas_and_evaluate.c
//!  \brief     <i><b> Service following implementation : measurements procedure and criteria evaluation </b></i>
//!  \details   This file provides functionalities for service following measurements procedure and criteria evaluation
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.11.03
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_MEAS_AND_EVALUATE_C
#define SERVICE_FOLLOWING_MEAS_AND_EVALUATE_C
/* END EPR CHANGE */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"


#if defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)
    #include "AMFMDevCmds.h"    
    #include "amfmdev_comm.h"
#endif // #if defined (CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE)

#if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
    #include "rds_data.h"
    #include "rds_landscape.h"
#endif // #if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)



#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "seamless_switching_common.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)


#include "service_following.h"

#include "service_following_internal.h"

#include "service_following_log_status_info.h"

#include "service_following_mainloop.h"

#include "service_following_meas_and_evaluate.h"

#include "service_following_background.h"



#ifdef __cplusplus
extern "C" {
#endif

/*
*  Procedure to evaluate the original cell 
* output : action to be done 
* 	DABMW_SF_EVALUATE_RES_NO_ACTION
* 	DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY
* 	DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR
*
* Principle : 
* get measurement on original cell
*
* Check the measurement versus the threshold.
*
* basic principle : 
* the cell should be below the defined threshold during at least n consecutives measurements.
*
* coniguration parameters 
* quality threshold for DAB & FM. 
* 	2 threshold : 
*	- poor quality which will lead to service recovery
*	- good quality which will mean no action
* 	- between poor & good : will lead to evaluate neighboor... 
* 	
* N consecutives meas
* 	- defines the number of time the threshold should be reach to lead to decision
* 
*/

DABMW_SF_EvaluationActionResultTy DABMW_ServiceFollowing_EvaluateOriginalCellSuitability()
{
	DABMW_SF_EvaluationActionResultTy vl_NeededAction = DABMW_SF_EVALUATE_RES_NO_ACTION_GOOD_CELL;
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
	DABMW_SF_QualityStatusTy vl_originalQuality;
    static DABMW_SF_QualityStatusTy vl_originalLoggedQuality = DABMW_SF_QUALITY_IS_POOR;
    static tU8 vl_logQualityCounter = 0;

	tSInt vl_tmp_res = OSAL_OK;
    tBool vl_dabServiceSelectionSucceed = false;
	tU32 vl_currentOriginalPI;

#define DABMW_SF_COUNTER_MAX_VALUE (0xFF - 1)


	DABMW_ServiceFollowing_MeasureOrignalCell();

    // specific workaround to make sure the service is selected 
    // because sometimes it fails, resulting in no audio
    //
    	/* orignial quality */
	if ((true == vl_originalIsDab)
        &&
        (false == DABMW_serviceFollowingStatus.original_DabServiceSelected)
        )
		{
            // the DAB service is not selected : try to select it again !!
            //

        // It is assumed we are tuned on the right frequency already...
        //
        
		/* Select the DAB service
		* not that it could already be selected... 
		* here should be improve to select is not already selected...
		*/
	
		/* reload EID in case it has change...
		* case is ECC received meanwhile 
		*/

#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   

                        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)  
                       DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (DABMW_ServiceFollowing_EvaluateOriginalCellSuitability) : ERROR => DAB SERVICE NOT SELECTED, try to select it again / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                                            DABMW_serviceFollowingStatus.originalAudioUserApp,
                                            DABMW_serviceFollowingStatus.originalFrequency,
                                            DABMW_serviceFollowingStatus.originalEid,
                                            DABMW_serviceFollowingStatus.originalSid);
        
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                      /* END TMP LOG */  

		DABMW_serviceFollowingStatus.originalEid= DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(DABMW_serviceFollowingStatus.originalAudioUserApp);

		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_serviceFollowingStatus.originalAudioUserApp,										   // Use parameter passed
										 DABMW_SERVICE_SELECT_SUBFNCT_SET,				// Sub-function is 5
										 DABMW_serviceFollowingStatus.originalEid,		 // Ensemble  
										 DABMW_serviceFollowingStatus.originalSid);		// Service ID 		  
				
			/* check : is the service selection ok ?  */
			if (OSAL_OK == vl_tmp_res)
				{
				// Service ok  
				vl_dabServiceSelectionSucceed = true;
                

				// consider we start here the audio 
				DABMW_serviceFollowingStatus.originalLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();


				/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (DABMW_ServiceFollowing_EvaluateOriginalCellSuitability) : new DAB SERVICE SELECTION success / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.originalAudioUserApp,
                            DABMW_serviceFollowingStatus.originalFrequency,
                            DABMW_serviceFollowingStatus.originalEid,
                            DABMW_serviceFollowingStatus.originalSid);
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
              /* END TMP LOG */  


				}
			else
				{
                // should not come here but you never know 
                vl_dabServiceSelectionSucceed = false;
                
				/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)  
               DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (DABMW_ServiceFollowing_EvaluateOriginalCellSuitability) : ERROR => DAB SERVICE SELECTION FAILED / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                                    DABMW_serviceFollowingStatus.originalAudioUserApp,
                                    DABMW_serviceFollowingStatus.originalFrequency,
                                    DABMW_serviceFollowingStatus.originalEid,
                                    DABMW_serviceFollowingStatus.originalSid);

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
              /* END TMP LOG */  
                                                                  
  
				// should not come here but you never know 
                // let's print a failure
                //
			}

            DABMW_serviceFollowingStatus.original_DabServiceSelected = vl_dabServiceSelectionSucceed;
          
								 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                                /* END TMP LOG */     
            
    	}

	/* orignial quality */
	if (true == vl_originalIsDab)
		{
        vl_originalQuality = DABMW_ServiceFollowing_GetQualityStatusDAB(DABMW_serviceFollowingStatus.original_Quality);
		}
	else
		{
        vl_originalQuality = DABMW_ServiceFollowing_GetQualityStatusFM(DABMW_serviceFollowingStatus.original_Quality);
		}

#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    // check if it is one lock freq...
    //
    if (DABMW_serviceFollowingStatus.originalFrequency == DABMW_serviceFollowingData.lockFrequency_1)
        {
        if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_GOOD;
            }   
        }
    else if (DABMW_serviceFollowingStatus.originalFrequency == DABMW_serviceFollowingData.lockFrequency_2)
        {
        if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_GOOD;
            }               
        }
#endif

  DABMW_serviceFollowingStatus.original_Quality.qualityStatus = vl_originalQuality;

  /* let's log quality from time to time : 
    * only when it changes of category
    */

  if ((vl_originalLoggedQuality != vl_originalQuality)
      || (vl_logQualityCounter > DABMW_SERVICE_FOLLOWING_LOG_COUNTER_QUALITY))
  {
       /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
       DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */
    
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
       DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Original Cell Suitability) :  app = %d, Freq = %d, Quality range %s \n", 
                                DABMW_serviceFollowingStatus.originalAudioUserApp,
                                DABMW_serviceFollowingStatus.originalFrequency, 
                                DABMW_SERVICE_FOLLOWING_LOG_QUALITY_RANGE(vl_originalQuality)
                                );      

        DABMW_SF_LOG_INFO_UPDATE_MEASUREMENT;

        if (true == vl_originalIsDab)
        {
            DABMW_ServiceFollowing_DisplayQualityDAB(DABMW_serviceFollowingStatus.original_Quality.dabQuality, "DABMW_Service_Following (Original Cell Suitability) : ");
        }
        else
        {
            DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.original_Quality.fmQuality, "DABMW_Service_Following (Original Cell Suitability) : ");
        }

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                    /* END TMP LOG */


           /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
       DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */
    
      vl_originalLoggedQuality = vl_originalQuality;
      vl_logQualityCounter = 0;
  }
  else
  {
    vl_logQualityCounter++;
  }
  
	/* evaluate the action
	* evaluate the quality 
	*
	* basic principle : 
	* the cell should be below the defined threshold during at least n consecutives measurements.
	*
	* coniguration parameters 
	* quality threshold
	* N consecutives meas
	*/

    if ((DABMW_SF_QUALITY_IS_NO_AUDIO == vl_originalQuality)
		|| 
		(DABMW_SF_QUALITY_IS_NO_SYNC == vl_originalQuality))
        {
        DABMW_serviceFollowingStatus.original_badQualityCounter = DABMW_SF_COUNTER_MAX_VALUE;
        DABMW_serviceFollowingStatus.original_mediumQualityCounter = DABMW_SF_COUNTER_MAX_VALUE;

		// mark we have no audio 
		DABMW_serviceFollowingStatus.originalLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
						
        vl_NeededAction = DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY;
        }
    else if (DABMW_SF_QUALITY_IS_POOR == vl_originalQuality)
		{
		/* Quality is poor : 
		* increment the counter 
		* if counter above threshold : it means this is time !
		*/
		if (DABMW_serviceFollowingStatus.original_badQualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
			{
			DABMW_serviceFollowingStatus.original_badQualityCounter++;
			}
	
		/* increment the medium quality as well... 
		* if not at the max
		*/
		if (DABMW_serviceFollowingStatus.original_mediumQualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
			{
			DABMW_serviceFollowingStatus.original_mediumQualityCounter++;
			}

		if (DABMW_serviceFollowingStatus.original_badQualityCounter > DABMW_serviceFollowingStatus.counter_NbMeasureLossService)
			{
			vl_NeededAction = DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY;
			}
        // add the processing of medium counter :
        // if was already medium, we should handle it
        else if (DABMW_serviceFollowingStatus.original_mediumQualityCounter > DABMW_serviceFollowingStatus.counter_NbMeasureStartAfSearch)
		    {
			vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR;
            }   
        else
            {
            vl_NeededAction = DABMW_SF_EVALUATE_RES_NO_ACTION_POOR_CELL;
            }
       	}
	else 
		{
		/* quality is GOOD or MEDIUM  : reset the counter */
		DABMW_serviceFollowingStatus.original_badQualityCounter = 0;

		if (DABMW_SF_QUALITY_IS_MEDIUM == vl_originalQuality)
			{
			/* Quality is medium : 
			* increment the counter 
			* if counter above threshold : it means this is time !
			*/
			if (DABMW_serviceFollowingStatus.original_mediumQualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
				{
				DABMW_serviceFollowingStatus.original_mediumQualityCounter++;
				}

			if (DABMW_serviceFollowingStatus.original_mediumQualityCounter > DABMW_serviceFollowingStatus.counter_NbMeasureStartAfSearch)
				{
				vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR;
				}
            else
                {
                vl_NeededAction = DABMW_SF_EVALUATE_RES_NO_ACTION_MEDIUM_CELL;
                }
			}
		else
			{
			/* quality is above the medium quality : reset the counter */
			DABMW_serviceFollowingStatus.original_mediumQualityCounter = 0;

			/* quality is GOOD */
			/* if in FM then DAB measurement only may be trigger */

			if ((false == vl_originalIsDab) && (true == DABMW_serviceFollowingData.fmToDabIsActive))
				{			
				/* we may look for DAB anyhow because want to resume to DAB */
				vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY;
				}
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
			else if ((true == vl_originalIsDab) && (true == DABMW_serviceFollowingData.fmToDabIsActive)
				&& (true == DABMW_serviceFollowingData.seamlessSwitchingMode))
				{
				// we may look for FM to prepare the measurement for seamless estimation for next switch 
				vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_FM_ONLY;
				}
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
			else 
				{
				vl_NeededAction = DABMW_SF_EVALUATE_RES_NO_ACTION_GOOD_CELL;
				}
			}
		}


	// specific changes to handle the PI confirmation as needed 
	// in FM , if PI on original is wrong, the consider original as poor
	//
	if (false == vl_originalIsDab)
		{		
		vl_currentOriginalPI = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPiFromFreq(DABMW_serviceFollowingStatus.originalFrequency);
		
		// correction : error in PI comparison 
		//
		// BEFORE : 
		//	if (vl_currentOriginalPI != DABMW_serviceFollowingStatus.originalSid)
		// after
		// 
		if (DABMW_SF_BG_SID_NOT_EQUAL == DABMW_ServiceFollowing_CompareSidPI(DABMW_serviceFollowingStatus.originalSid, vl_currentOriginalPI, false))
			{
			// due to some interferences or perturbation,
			// original PI is not ok...
			//

			 DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Original Cell Suitability) :  CHANGE IN PI !!!! app = %d, freq = %d, Quality range %s PI ori = %d, PI current = %d \n", 
                                DABMW_serviceFollowingStatus.originalAudioUserApp,
                                DABMW_serviceFollowingStatus.originalFrequency, 
                                DABMW_SERVICE_FOLLOWING_LOG_QUALITY_RANGE(vl_originalQuality),
                                DABMW_serviceFollowingStatus.originalSid,
                                vl_currentOriginalPI
                                );      
			
			vl_NeededAction = DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY;
			}
		else
			{
			// update Sid 
			DABMW_serviceFollowingStatus.originalSid = vl_currentOriginalPI;
			}
		}
		
    return vl_NeededAction;    
	
}


/*
*  Procedure to measure the original cell suitability
* output : update the variable storing the original cell quality 
*
* basic principle 
* alogirhtm is based on 2G one... 
* - take the increase directly. 
* - but ponderate the decrease
*
* parameter : 
* ponderation parameters : 
* DABMW_SF_CURRENT_QUALITY_PONDERATION
* DABMW_SF_NEW_QUALITY_PONDERATION
*/
tVoid DABMW_ServiceFollowing_MeasureOrignalCell()
{

//	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);

    // extension with new structure for quality parameters
    // init the structures 
    
    // set the service selected flag for the quality

    DABMW_serviceFollowingStatus.original_Quality = DABMW_ServiceFollowing_MeasureAndPonderateOriginalCell(DABMW_serviceFollowingStatus.originalAudioUserApp);  
    
}


/*
*  Procedure to evaluate the AF  cell 
* output : action to be done 
* 	DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR
*	- means the current AF is not good anymore, we should look for an other one
* 	DABMW_SF_EVALUATE_CHANGE_CELL
*	- means a change of cell, ie a switch is needed
* 	DABMW_SF_EVALUATE_NO_ACTION
*	- means nothing specific continue as is
*
* Principle : 
* get measurement on af cell
*
* Check the measurement versus the threshold.
* check the measurement versus orignial
*
* basic principle : 
* the cell should be below the defined threshold during at least n consecutives measurements.
*
* coniguration parameters 
* quality threshold for DAB & FM. 
* 	2 threshold : 
*	- poor quality which will lead to service recovery
*	- good quality which will mean no action
* 	- between poor & good : will lead to evaluate neighboor... 
* 	
* N consecutives meas
* 	- defines the number of time the threshold should be reach to lead to decision
* 
*/


DABMW_SF_EvaluationActionResultTy DABMW_ServiceFollowing_EvaluateAFCellSuitability()
{
	DABMW_SF_EvaluationActionResultTy vl_NeededAction = DABMW_SF_EVALUATE_RES_NO_ACTION;
	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
	DABMW_SF_QualityStatusTy vl_alternateQuality, vl_alternateQualityWithHysteresis, vl_originalQuality;
	tBool vl_AFisbetter = false;
    static DABMW_SF_QualityStatusTy vl_alternateLoggedQuality = DABMW_SF_QUALITY_IS_POOR;
    static tU8 vl_logQualityCounter = 0;
	tU32 vl_currentalternatePI;

    // be carreful : if alternate is FM, we need to do an AF check procedure, which may be in conflict with AF check case
    // which is doing AF start procedures .... 
    // so no action in that case : wait for the on-going AF check/start to be ended.
    //

    if ((false == vl_alternateIsDab)
        && (true == DABMW_ServiceFollowing_IsBackgroundAFCheckOnGoing())
        )
    {
        return DABMW_SF_EVALUATE_RES_NO_ACTION;
    }
    else if ((true == vl_alternateIsDab) && (false == DABMW_serviceFollowingStatus.alternateTuned))
    {
        // alternate is DAB but not tuned, because used for bg app typically
        // so no measure valid.
        return DABMW_SF_EVALUATE_RES_NO_ACTION;
    }
    
	/* Start by measuring the alternate cell */
	DABMW_ServiceFollowing_MeasureAFCell();

#define DABMW_SF_COUNTER_MAX_VALUE (0xFF - 1)


	/* get quality */

	/* Alternate quality */
	if (true == vl_alternateIsDab)
		{
        vl_alternateQuality = DABMW_ServiceFollowing_GetQualityStatusDAB(DABMW_serviceFollowingStatus.alternate_Quality);
        vl_alternateQualityWithHysteresis = DABMW_ServiceFollowing_GetQualityStatusDABwithHysteresis(DABMW_serviceFollowingStatus.alternate_Quality);

		}
	else
		{
        vl_alternateQuality = DABMW_ServiceFollowing_GetQualityStatusFM(DABMW_serviceFollowingStatus.alternate_Quality);
        vl_alternateQualityWithHysteresis = DABMW_ServiceFollowing_GetQualityStatusFMwithHysteresis(DABMW_serviceFollowingStatus.alternate_Quality);

		}

    // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
        // check if it is one lock freq...
        //
        if (DABMW_serviceFollowingStatus.alternateFrequency == DABMW_serviceFollowingData.lockFrequency_1)
            {
            if (DABMW_serviceFollowingData.lockFrequency_1_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                vl_alternateQuality = DABMW_SF_QUALITY_IS_MEDIUM;			
				vl_alternateQualityWithHysteresis = vl_alternateQuality;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_alternateQuality = DABMW_SF_QUALITY_IS_POOR;
				vl_alternateQualityWithHysteresis = vl_alternateQuality;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_alternateQuality = DABMW_SF_QUALITY_IS_GOOD;
				vl_alternateQualityWithHysteresis = vl_alternateQuality;
                }   

            }
        else if (DABMW_serviceFollowingStatus.alternateFrequency == DABMW_serviceFollowingData.lockFrequency_2)
            {
            if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                vl_alternateQuality = DABMW_SF_QUALITY_IS_MEDIUM;
				vl_alternateQualityWithHysteresis = vl_alternateQuality;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_alternateQuality = DABMW_SF_QUALITY_IS_POOR;
			    vl_alternateQualityWithHysteresis = vl_alternateQuality;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_alternateQuality = DABMW_SF_QUALITY_IS_GOOD;
				vl_alternateQualityWithHysteresis = vl_alternateQuality;
                }   
            }
        
#endif


	/* orignial quality */
	if (true == vl_originalIsDab)
		{
        vl_originalQuality = DABMW_ServiceFollowing_GetQualityStatusDAB(DABMW_serviceFollowingStatus.original_Quality);
		}
	else
		{
        // original is FM
        // if both original and alternate are FM, the focus on the background quality
        // 
            if (true == vl_alternateIsDab)
            {
                // case Original = FM & Alternate DAB
                vl_originalQuality = DABMW_ServiceFollowing_GetQualityStatusFM(DABMW_serviceFollowingStatus.original_Quality);
            }
            else
            {
                // case Original = FM & Alternate FM
                // get quality on bg...
                // so that we compare same type
                vl_originalQuality = DABMW_ServiceFollowing_GetQualityStatusFM(DABMW_serviceFollowingStatus.original_Quality_onBackground);
            }
            
		}


    // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    // check if it is one lock freq...
    //
    if (DABMW_serviceFollowingStatus.originalFrequency == DABMW_serviceFollowingData.lockFrequency_1)
        {
        if (DABMW_serviceFollowingData.lockFrequency_1_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_GOOD;
            }   
        }
    else if (DABMW_serviceFollowingStatus.originalFrequency == DABMW_serviceFollowingData.lockFrequency_2)
        {
        if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
             vl_originalQuality = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_originalQuality = DABMW_SF_QUALITY_IS_GOOD;
            }               
         }

#endif


      DABMW_serviceFollowingStatus.alternate_Quality.qualityStatus = vl_alternateQuality;

      /* let's log quality from time to time : 
            * only when it changes of category
            */

      if ((vl_alternateLoggedQuality != vl_alternateQuality)
        || (vl_logQualityCounter > DABMW_SERVICE_FOLLOWING_LOG_COUNTER_QUALITY))
          {

           /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
           DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                    /* END TMP LOG */
        
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  

           
           // ADD an information if applicable of the original on background
           // 
           if ((false == vl_alternateIsDab) && (false == vl_originalIsDab))
           {
            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Original Cell Suitability on Background) : app = %d, Freq = %d, Quality range %s\n", 
                                    DABMW_serviceFollowingStatus.alternateApp,
                                    DABMW_serviceFollowingStatus.originalFrequency, 
                                    DABMW_SERVICE_FOLLOWING_LOG_QUALITY_RANGE(vl_originalQuality)
                                    );

            DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.original_Quality_onBackground.fmQuality, "DABMW_Service_Following (Original Cell Suitability on Background) : ");
           } 
           

          // alternate quality print.
           DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Alternate Cell Suitability) :  app = %d, Freq = %d, Quality range %s, Quality rang with hysteresis %s\n", 
                                    DABMW_serviceFollowingStatus.alternateApp,
                                    DABMW_serviceFollowingStatus.alternateFrequency, 
                                    DABMW_SERVICE_FOLLOWING_LOG_QUALITY_RANGE(vl_alternateQuality),
                                    DABMW_SERVICE_FOLLOWING_LOG_QUALITY_RANGE(vl_alternateQualityWithHysteresis)
                                    );

            DABMW_SF_LOG_INFO_UPDATE_MEASUREMENT;
     
            if (true == vl_alternateIsDab)
            {
                DABMW_ServiceFollowing_DisplayQualityDAB(DABMW_serviceFollowingStatus.alternate_Quality.dabQuality, "DABMW_Service_Following (Alternate Cell Suitability) : ");
            }
            else
            {
                DABMW_ServiceFollowing_DisplayQualityFM(DABMW_serviceFollowingStatus.alternate_Quality.fmQuality, "DABMW_Service_Following (Alternate Cell Suitability) : ");
            }
    
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                        /* END TMP LOG */
    
    
               /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
           DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                    /* END TMP LOG */

            vl_alternateLoggedQuality = vl_alternateQuality;
            vl_logQualityCounter = 0;

          }
      else
      {
        vl_logQualityCounter++;
      }
          
	/* now decision part */

	// move with hysteresis

     if ((DABMW_SF_QUALITY_IS_NO_AUDIO == vl_alternateQuality)
		|| (DABMW_SF_QUALITY_IS_NO_SYNC == vl_alternateQuality))
     {
        vl_alternateQuality = DABMW_SF_QUALITY_IS_POOR;
			
		// mark we have no audio 
		DABMW_serviceFollowingStatus.alternateLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
    }

    
    if ((DABMW_SF_QUALITY_IS_NO_AUDIO == vl_alternateQualityWithHysteresis)
		|| (DABMW_SF_QUALITY_IS_NO_SYNC == vl_alternateQualityWithHysteresis))
     {
        vl_alternateQualityWithHysteresis = DABMW_SF_QUALITY_IS_POOR;
    }
    
    
	if (DABMW_SF_QUALITY_IS_POOR == vl_alternateQualityWithHysteresis)
		{
		/* Quality is poor : 
		* increment the counter 
		* if counter above threshold : it means this is time to look to an other neighboor.... !
		*/
		if (DABMW_serviceFollowingStatus.alternate_badQualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
			{
			DABMW_serviceFollowingStatus.alternate_badQualityCounter++;
			}

        // needed action is POOR only if not in hysteresis...
        // because may remain medium in reality 
		if (DABMW_serviceFollowingStatus.alternate_badQualityCounter > DABMW_serviceFollowingStatus.counter_NbMeasureLossService)
			{
            if (DABMW_SF_QUALITY_IS_POOR == vl_alternateQuality)
                {
			    vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR;
                }            
			}
		}
	else 
		{
		/* Alternate quality is good or medium */

		DABMW_serviceFollowingStatus.alternate_badQualityCounter = 0;
		
		/* evaluate the switch depending on the algorithm and original & af bearer*/
		if (true == vl_alternateIsDab)
    		{
    		/* alternate is DAB and ~ok (medium or good)
			* algorithm is : DAB preferred > FM 
			* so if original = FM, and DAB ok, then switch to DAB
			*/		
			if (false == vl_originalIsDab)
				{
				/* original is not DAB ... so this is fm
				* Orig = FM, Alternate = DAB
				* switch to DAB as priority
				* switch to DAB as soon as DAB is same or better than FM
				* ie DAB GOOD and FM Medium or GOOD
				* or DAB Medium & FM Medium
				*/

				if (vl_alternateQualityWithHysteresis >= vl_originalQuality)
					{
					/* alternate is good */
					if (DABMW_serviceFollowingStatus.alternate_qualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
						{
						DABMW_serviceFollowingStatus.alternate_qualityCounter++;
						}
					}
				else
					{
					DABMW_serviceFollowingStatus.alternate_qualityCounter = 0;
					}
				}
			else
				{
				/* Orig = DAB Alternate = DAB
				* switch criteria is on the threshold...
				* siwthc to AF if quality of AF is better : AF GOOD & Original MEDIUM, OR if AF > MEDIUM+HYSTERESIS
				* as a start : switch to AF only is DAB source = medium & DAB target = good 
				*/			
				if (vl_alternateQualityWithHysteresis> vl_originalQuality)
					{
					/* quality is better take new one */
					vl_AFisbetter = true;
					}
				else if (vl_alternateQualityWithHysteresis == vl_originalQuality)
					{
					/* same quality : switch if alternate better than original with hysteresis */
//					if 	((DABMW_serviceFollowingStatus.alternateQuality + DABMW_SF_HYSTERESIS_DAB)< DABMW_serviceFollowingStatus.originalQuality)
                        //TODO :  call a function for quality (and hysteresys) comparison
                    if (true == DABMW_ServiceFollowing_qualityA_better_qualityB_DAB(DABMW_serviceFollowingStatus.alternate_Quality, DABMW_serviceFollowingStatus.original_Quality,
                                                                    DABMW_serviceFollowingStatus.alternateFrequency, DABMW_serviceFollowingStatus.originalFrequency))
						{
						/* the alternate is same quality but ok to switch because better than current alternate */
						vl_AFisbetter = true;
						}
					}

				if (true == vl_AFisbetter)
					{
					/* increment counter */
					if (DABMW_serviceFollowingStatus.alternate_qualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
						{
						DABMW_serviceFollowingStatus.alternate_qualityCounter++;
						}
					}
				else 
					{
					/* reset counter : condition not filled */
					DABMW_serviceFollowingStatus.alternate_qualityCounter = 0;
					}				
				}
			}
		else
			{
			/* Alternate is FM */
			/* evaluate the quality in FM
			*
			* basic principle : 
			* the cell should be below the defined threshold during at least n consecutives measurements.
			*
			* coniguration parameters 
			* quality threshold
			* N consecutives meas
			*/		
			if (true == vl_originalIsDab)
				 {
				 /* original is  DAB ... 
				 *  Orig = DAB Alternate = FM
				 * so switch from DAB to FM only, possibly no switch until service recovery,
				 * but current setting : only if DAB medium & FM good (ie FM > DAB)
				 * ~same way than DAB to DAB
				 * as a test ~
				 */
				if ((DABMW_SF_QUALITY_IS_GOOD == vl_alternateQualityWithHysteresis)
					&& (DABMW_SF_QUALITY_IS_MEDIUM == vl_originalQuality)
					)
					{
					/* increment counter */
					if (DABMW_serviceFollowingStatus.alternate_qualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
						{
						DABMW_serviceFollowingStatus.alternate_qualityCounter++;
						}
					}
				else 
					{
					/* reset counter : condition not filled */
					DABMW_serviceFollowingStatus.alternate_qualityCounter = 0;
					}	
				}
			else
				{
				/* original is  FM ... 
				 *  Orig = FM Alternate = FM
				 * so switch from FM to FM only, only on specific conditions... 
				 * switch to AF if quality of AF is better : AF GOOD & Original MEDIUM, OR if AF > MEDIUM+HYSTERESIS
				 * as a start : switch to AF only is DAB source = medium & DAB target = good 
                            */
                // handled with hysteresis

                if (vl_alternateQualityWithHysteresis> vl_originalQuality)
					{
					/* quality is better take new one */
					vl_AFisbetter = true;
					}
				else if (vl_alternateQualityWithHysteresis == vl_originalQuality)
					{
                    //  codex #309933 : compare reference quality on background 
                    // 
                    
                    if (true == DABMW_ServiceFollowing_qualityA_better_qualityB_FM(DABMW_serviceFollowingStatus.alternate_Quality, DABMW_serviceFollowingStatus.original_Quality_onBackground,
                                                                                DABMW_serviceFollowingStatus.alternateFrequency, DABMW_serviceFollowingStatus.originalFrequency))
    						{
    						/* the alternate is same quality but ok to switch because better than current alternate */
    						vl_AFisbetter = true;
    						}
				    }
			
				if (true == vl_AFisbetter)
					{
					/* increment counter */
					if (DABMW_serviceFollowingStatus.alternate_qualityCounter < DABMW_SF_COUNTER_MAX_VALUE)
						{
						DABMW_serviceFollowingStatus.alternate_qualityCounter++;
						}
					}
				else 
					{
					/* reset counter : condition not filled */
					DABMW_serviceFollowingStatus.alternate_qualityCounter = 0;
					}
				}
			}

		/* now let's check if we need a switch */
		
		if (DABMW_serviceFollowingStatus.alternate_qualityCounter > DABMW_serviceFollowingStatus.counter_NbMeasureSwitch)
			{
			/* Which kind of CELL Change ? 
			* do we need correlation estimation ? 
			*
			* if seamless activated, and change if FM <->, and DAB First is set
			* then SS neeed 
			* Even in FM first case, loudness is good to have .... 
			*
			*/
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
			if ((true == DABMW_serviceFollowingData.seamlessSwitchingMode)
				//&&
				//(true == DABMW_serviceFollowingStatus.configurationIsDabFirst)
				&&
				(vl_originalIsDab != vl_alternateIsDab)
				)
				{
				vl_NeededAction = DABMW_SF_EVALUATE_CHANGE_CELL_SEAMLESS;
				}
			else
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
				{
                //codex #309933 :    
                // Alternate is better than Original for a certain time.
                // in FM - FM case, this is decided on bg tuner evaluation for both... 
                //is that still true on main tuner ? 
                //
				vl_NeededAction = DABMW_SF_EVALUATE_CHANGE_CELL;
				}
			}
		else
			{
			if (0 == DABMW_serviceFollowingStatus.alternate_qualityCounter)
				{
				/* the current alternate is not better that original 
				* should we look for an other one ?
				*/
                if (true == vl_alternateIsDab)
                    {
                    // alternate is DAB
                    // may be we need to search a better alternate on DAB bearer only
                    //
                    
                    // temporary attempt
                    //
                    // vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY;
                    //
                    vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR;
                    }
				else 
					{
                    // may be we need to search for a better alternate
                    // whatever the bearer 
                    // ie either a better FM, either a better DAB
					vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR;
					}
				}
			}
		
		}

	/* in any case, if switch has been done recently ... do not switch  
	* so if not time for switch : do not swtitch
	*/
	  if ((false == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LastSwitch))
	  	&& 
	  	((DABMW_SF_EVALUATE_CHANGE_CELL == vl_NeededAction)
	  	|| (DABMW_SF_EVALUATE_CHANGE_CELL_SEAMLESS == vl_NeededAction))
	  	)
	  	{
	  	vl_NeededAction = DABMW_SF_EVALUATE_RES_NO_ACTION;
	  	}


	   // Specific handling in POOR case : 
	  if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR == vl_NeededAction)
		  {
		  
		  // if the cell has just been selected as alternate and is poor
		  // let's keep it for a while before removing it
		  // this is to avoid that a just selected cell by background is remove directly...
		  // and entering infinite loop of cell being selected in background and remove on evaluation
		  // especially in dab...
		  
		  if (false == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_MinToKeepAlternate))
		  	{
		  	// we should not remove the alternate ... 
		  	// do as if it was a no action
		  	vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR;
		  	}

		  // Correction after FT experience
		  // In DAB, keep the alternate despite it is poor, until it is sync
		  // this is to cope the case of DAB changing between audio level poor and medium, but kept sync
		  // if DAB is poor and removed, it will be reselected during bc scan... so no point to remove it.
		  //
		  if ((true == vl_alternateIsDab)
		  	&& (false == DABMW_ServiceFollowing_IsOutOfSync(DABMW_serviceFollowingStatus.alternate_Quality.dabQuality)))
		  	{
		  	// we should not remove the alternate ... 
			// do as if it was a no action
			vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR;
			}
	  }


	  // specific changes to handle the PI confirmation as needed 
	  // in FM , if PI on alternate is wrong, the consider alternate as poor
	  //
	  if (false == vl_alternateIsDab)
		  { 	  
		  vl_currentalternatePI = DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPiFromFreq(DABMW_serviceFollowingStatus.alternateFrequency);

		  // correction : error in PI comparison 
		  //
		  // BEFORE : 
		  //  if (vl_currentalternatePI != DABMW_serviceFollowingStatus.alternateSid)
		  // after
		  // 
		  if (DABMW_SF_BG_SID_NOT_EQUAL == DABMW_ServiceFollowing_CompareSidPI(DABMW_serviceFollowingStatus.alternateSid, vl_currentalternatePI, false))
			  {
			  // due to some interferences or perturbation,
			  // original PI is not ok...
			  //
			  
			  DABMW_SF_PRINTF( TR_LEVEL_COMPONENT, "Change neighboor status : vl_currentalternatePI = 0x%x, DABMW_serviceFollowingStatus.alternateSid = 0x%x, freq = %d \n",
					  vl_currentalternatePI, DABMW_serviceFollowingStatus.alternateSid,
					  DABMW_serviceFollowingStatus.alternateFrequency);
			  
			  vl_NeededAction = DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR;
			  }
		else
			{
				// update Sid 
				DABMW_serviceFollowingStatus.alternateSid = vl_currentalternatePI;
			}
		  }
	  

	  return vl_NeededAction;

}

/*
*  Procedure to measure the AF cell suitability
* output : update the variable storing the original cell quality 
*
* basic principle 
* alogirhtm is based on 2G one... 
* - take the increase directly. 
* - but ponderate the decrease
*
* parameter : 
* ponderation parameters : 
* DABMW_SF_CURRENT_QUALITY_PONDERATION
* DABMW_SF_NEW_QUALITY_PONDERATION
*/
tVoid DABMW_ServiceFollowing_MeasureAFCell()
{
	
//	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);



    // extension with new structure for quality parameters
    // init the structures 
    DABMW_serviceFollowingStatus.alternate_Quality = DABMW_ServiceFollowing_MeasureAndPonderateAlternateCell(DABMW_serviceFollowingStatus.alternateApp);
    
}

/*
*  Procedure to retrieve quality level of a given measure depending on techno
*
* quality level = poor, medium, good
*/
DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityLevelDAB(tU32 vI_ficBerValue)
{
	DABMW_SF_QualityStatusTy vl_quality;

	/* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* If quality is emulated, return the wished one */
	if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
		{
		vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
		}
	else if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
		{
		vl_quality = DABMW_SF_QUALITY_IS_POOR;
		}
	else if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
		{
		vl_quality = DABMW_SF_QUALITY_IS_GOOD;
		}
	else
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
		{
	/* END EPR CHANGE */
		if (vI_ficBerValue > DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber)
			{
			vl_quality = DABMW_SF_QUALITY_IS_POOR;
			}
		else if (vI_ficBerValue > DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber)
			{
			vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
			}
		else
			{	
			vl_quality = DABMW_SF_QUALITY_IS_GOOD;
			}
		}
			
	return vl_quality;

}

/*
*  Procedure to retrieve quality level of a given measure depending on techno

*  Input = quality value structure
 *
 * Output  
* quality range = poor, medium, good
*/
DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusDAB(DABMW_SF_QualityTy vI_qualityValue)
{
	DABMW_SF_QualityStatusTy vl_quality;
    tU32 vl_ficBerValue;
    tU32 vl_ficBerPoorThreshold, vl_ficBerGoodThreshold;

    vl_ficBerValue = vI_qualityValue.dabQuality.fic_ber;

    // currently based on ficber only
    //
    
	/* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* If quality is emulated, return the wished one */
	if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
		{
		vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
		}
	else if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
		{
		vl_quality = DABMW_SF_QUALITY_IS_POOR;
		}
	else if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
		{
		vl_quality = DABMW_SF_QUALITY_IS_GOOD;
		}
	else
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
		{
    	/* END EPR CHANGE */

       
        // currently based on ficber only
        // 

        // codex #319038 : add a stretegy to fine tune mapping of audio and DAB quality
        //

        if (DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL == DABMW_serviceFollowingData.strategy_DAB)
            {
            // for now if service not selected keep the dab threshold ... 
            // 
            if (true == vI_qualityValue.dabQuality.service_selected)
                {
                // in DAB+ or DMB, if reed solomon is 15 => need to switch  
                // this is hidden / integrated in audio_ber_level
    

                if (0 == vI_qualityValue.dabQuality.audio_ber_level)                   
                    {
                    vl_quality = DABMW_SF_QUALITY_IS_NO_AUDIO;
                    }
                else if (vI_qualityValue.dabQuality.audio_ber_level < DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.audio_ber_level)
                    {
                    vl_quality = DABMW_SF_QUALITY_IS_POOR;
                    }
                else if (vI_qualityValue.dabQuality.audio_ber_level < DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.audio_ber_level)
                    {
                    vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
                    }
                else
                    {
                    vl_quality = DABMW_SF_QUALITY_IS_GOOD;
                    }
                }
            else
                {
                // depends on strategy
                // get the current service information : different management for dab & dab_plus
                //
                if ((DABMW_COMPONENT_TYPE_DAB_PLUS == vI_qualityValue.dabQuality.component_type)
                    || (DABMW_COMPONENT_TYPE_MPEG2TS == vI_qualityValue.dabQuality.component_type )
                    )
                    {
                    vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                    vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                    }
                else
                    {          
                    vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber;
                    vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber;
                    }
                
                    
           		if (vl_ficBerValue > vl_ficBerPoorThreshold)
            		{
            		vl_quality = DABMW_SF_QUALITY_IS_POOR;
            		}
            	else if (vl_ficBerValue > vl_ficBerGoodThreshold)
            		{
            		vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
            		}
            	else
            		{	
            		vl_quality = DABMW_SF_QUALITY_IS_GOOD;
            		}
                }

     
            }
        else 
            {
            
            // depends on strategy
            // get the current service information : different management for dab & dab_plus
            //
            if ((DABMW_COMPONENT_TYPE_DAB_PLUS == vI_qualityValue.dabQuality.component_type)
                || (DABMW_COMPONENT_TYPE_MPEG2TS == vI_qualityValue.dabQuality.component_type )
                )
                {
                vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                }
            else
                {          
                vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber;
                vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber;
                }
            
                
       		if (vl_ficBerValue > vl_ficBerPoorThreshold)
        		{
        		vl_quality = DABMW_SF_QUALITY_IS_POOR;
        		}
        	else if (vl_ficBerValue > vl_ficBerGoodThreshold)
        		{
        		vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
        		}
        	else
        		{	
        		vl_quality = DABMW_SF_QUALITY_IS_GOOD;
        		}
            }
        }


	// whatever the strategy, if out of sync...	
	if (true == DABMW_ServiceFollowing_IsOutOfSync(vI_qualityValue.dabQuality))
	{
		vl_quality = DABMW_SF_QUALITY_IS_NO_SYNC;
	}
					
	return vl_quality;

}

/*
*  Procedure to retrieve quality level of a given measure depending on techno

*  Input = quality value structure
 *
 * Output  
* quality range = poor, medium, good
*/
DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusDABwithHysteresis(DABMW_SF_QualityTy vI_qualityValue)
{
	DABMW_SF_QualityStatusTy vl_quality;
    tU32 vl_ficBerValue;
    tU32 vl_ficBerPoorThreshold, vl_ficBerGoodThreshold;
    
    vl_ficBerValue = vI_qualityValue.dabQuality.fic_ber;

    // currently based on ficber only
    //
    
	/* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* If quality is emulated, return the wished one */
	if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
		{
		vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
		}
	else if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
		{
		vl_quality = DABMW_SF_QUALITY_IS_POOR;
		}
	else if (DABMW_serviceFollowingData.DAB_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
		{
		vl_quality = DABMW_SF_QUALITY_IS_GOOD;
		}
	else
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
		{
    	/* END EPR CHANGE */

        
        if (true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_Hysteresis_DAB))
            {   

            if (DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL == DABMW_serviceFollowingData.strategy_DAB)
                 {
                 // for now if service not selected keep the dab threshold ... 
                 // 
                 if (true == vI_qualityValue.dabQuality.service_selected)
                     {
                     // audio-ber-level to be considered independantly of the hysteresis
                    if (0 == vI_qualityValue.dabQuality.audio_ber_level)                   
                    	{
                   		vl_quality = DABMW_SF_QUALITY_IS_NO_AUDIO;
                    	}			 
                    else if (vI_qualityValue.dabQuality.audio_ber_level < (DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.audio_ber_level + DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.audio_ber_level))
                         {
                         vl_quality = DABMW_SF_QUALITY_IS_POOR;
                         }
                     else if (vI_qualityValue.dabQuality.audio_ber_level < (DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.audio_ber_level + DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.audio_ber_level))
                         {
                         vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
                         }
                     else
                         {
                         vl_quality = DABMW_SF_QUALITY_IS_GOOD;
                         }
                     }
                 else
                     {
                     // depends on strategy
                     // get the current service information : different management for dab & dab_plus
                     //
                     if ((DABMW_COMPONENT_TYPE_DAB_PLUS == vI_qualityValue.dabQuality.component_type)
                         || (DABMW_COMPONENT_TYPE_MPEG2TS == vI_qualityValue.dabQuality.component_type )
                         )
                         {
                         vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                         vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                         }
                     else
                         {          
                         vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber;
                         vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber;
                         }
                     
                         
                     if (vl_ficBerValue > (vl_ficBerPoorThreshold - DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.fic_ber))
                         {
                         vl_quality = DABMW_SF_QUALITY_IS_POOR;
                         }
                     else if (vl_ficBerValue > (vl_ficBerGoodThreshold - DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.fic_ber))
                         {
                         vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
                         }
                     else
                         {   
                         vl_quality = DABMW_SF_QUALITY_IS_GOOD;
                         }
                     }
            
            
                 }
             else 
                 {
                 
                 // depends on strategy
                 // get the current service information : different management for dab & dab_plus
                 //
                 if ((DABMW_COMPONENT_TYPE_DAB_PLUS == vI_qualityValue.dabQuality.component_type)
                     || (DABMW_COMPONENT_TYPE_MPEG2TS == vI_qualityValue.dabQuality.component_type )
                     )
                     {
                     vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                     vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold_dabplus_dmb.dabQuality.fic_ber;
                     }
                 else
                     {          
                     vl_ficBerPoorThreshold = DABMW_serviceFollowingData.poorQuality_Threshold.dabQuality.fic_ber;
                     vl_ficBerGoodThreshold = DABMW_serviceFollowingData.goodQuality_Threshold.dabQuality.fic_ber;
                     }
                 
                     
                 if (vl_ficBerValue > (vl_ficBerPoorThreshold - DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.dabQuality.fic_ber))
                     {
                     vl_quality = DABMW_SF_QUALITY_IS_POOR;
                     }
                 else if (vl_ficBerValue > (vl_ficBerGoodThreshold - DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.dabQuality.fic_ber))
                     {
                     vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
                     }
                 else
                     {   
                     vl_quality = DABMW_SF_QUALITY_IS_GOOD;
                     }
                 }

			// whatever the strategy, if out of sync...
			if (true == DABMW_ServiceFollowing_IsOutOfSync(vI_qualityValue.dabQuality))
				{
				vl_quality = DABMW_SF_QUALITY_IS_NO_SYNC;
				}	

        }
        else
        {    		
            vl_quality = DABMW_ServiceFollowing_GetQualityStatusDAB(vI_qualityValue);                
        }
    }


	return vl_quality;

}


// no more used
#if 0 
/*
 *	Procedure to retrieve quality level of a given measure depending on techno
 *
 * quality level = poor, medium, good
 */
DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityLevelFM(tU32 vI_FMqualityValue)
{
	 DABMW_SF_QualityStatusTy vl_quality;

 	/* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* If quality is emulated, return the wished one */
	if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
		{
		vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
		}
	else if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
		{
		vl_quality = DABMW_SF_QUALITY_IS_POOR;
		}
	else if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
		{
		vl_quality = DABMW_SF_QUALITY_IS_GOOD;
		}
	else
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
        {
    	 if (vI_FMqualityValue < DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV)
    		 {
    		 vl_quality = DABMW_SF_QUALITY_IS_POOR;
    		 }
    	 else if (vI_FMqualityValue < DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.fieldStrength_dBuV)
    		 {
    		 vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
    		 }
    	 else
    		 {	 
    		 vl_quality = DABMW_SF_QUALITY_IS_GOOD;
    		 }
        }
			 
	 return vl_quality; 
}
#endif // If 0

/*
 *	Procedure to retrieve quality range  of a given measure for FM
 *  Input = quality value structure
 *
 * Output 
 * quality level = poor, medium, good
 */
DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusFM(DABMW_SF_QualityTy vI_qualityValue)
{
	 DABMW_SF_QualityStatusTy vl_quality;
     tS16 vl_FieldStrenghValue_dBuV;
     tU16 vl_CombinedQ;


     /* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    /* If quality is emulated, return the wished one */
    if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
        {
        vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
        }
    else if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
        {
        vl_quality = DABMW_SF_QUALITY_IS_POOR;
        }
    else if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
        {
        vl_quality = DABMW_SF_QUALITY_IS_GOOD;
        }
    else
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
        {
          // currently based on Field Strengh and combinedQ
          //
          
            if ( DABMW_SF_STRATEGY_FM_COMBINED_QUALITY == DABMW_serviceFollowingData.strategy_FM )
            {
                // we are on the field strenght based strategy
                
                vl_CombinedQ = vI_qualityValue.fmQuality.combinedQ;
                
            	if (vl_CombinedQ < DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_POOR;
            		 }
            	else if (vl_CombinedQ < DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.combinedQ)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
            		 }
            	else
            		 {	 
            		 vl_quality = DABMW_SF_QUALITY_IS_GOOD;
            		 }
            }
            else
            {
                //           Default : DABMW_SF_STRATEGY_FM_FIELD_STRENGH
           
                 // we are on the field strenght based strategy
                
                vl_FieldStrenghValue_dBuV = vI_qualityValue.fmQuality.fieldStrength_dBuV;
                
                // we are on the field strenght based strategy
            	if (vl_FieldStrenghValue_dBuV < DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_POOR;
            		 }
            	else if (vl_FieldStrenghValue_dBuV < DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.fieldStrength_dBuV)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
            		 }
            	else
            		 {	 
            		 vl_quality = DABMW_SF_QUALITY_IS_GOOD;
            		 }
            }

        }
			 
	 return vl_quality; 
}

/*
 *	Procedure to retrieve quality range  of a given measure for FM
 *  Input = quality value structure
 *
 * Output 
 * quality level = poor, medium, good
 */
DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusFMwithHysteresis(DABMW_SF_QualityTy vI_qualityValue)
{
	 DABMW_SF_QualityStatusTy vl_quality;
     tS16 vl_FieldStrenghValue_dBuV;
     tU16 vl_CombinedQ;


     /* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    /* If quality is emulated, return the wished one */
    if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
        {
        vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
        }
    else if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
        {
        vl_quality = DABMW_SF_QUALITY_IS_POOR;
        }
    else if (DABMW_serviceFollowingData.FM_LevelSimulate == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
        {
        vl_quality = DABMW_SF_QUALITY_IS_GOOD;
        }
    else
#endif // CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
        {
          // currently based on Field Strengh and combinedQ
          //

        if (true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_Hysteresis_FM))
            {      
            if ( DABMW_SF_STRATEGY_FM_COMBINED_QUALITY == DABMW_serviceFollowingData.strategy_FM )
                {
                // we are on the field strenght based strategy
                
                vl_CombinedQ = vI_qualityValue.fmQuality.combinedQ;
                
            	if (vl_CombinedQ < (DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ + DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.combinedQ))
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_POOR;
            		 }
            	else if (vl_CombinedQ < (DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.combinedQ + DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.combinedQ))
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
            		 }
            	else
            		 {	 
            		 vl_quality = DABMW_SF_QUALITY_IS_GOOD;
            		 }
                }
            else
                {
                //           Default : DABMW_SF_STRATEGY_FM_FIELD_STRENGH
           
                 // we are on the field strenght based strategy
                
                vl_FieldStrenghValue_dBuV = vI_qualityValue.fmQuality.fieldStrength_dBuV;
                
                // we are on the field strenght based strategy
            	if (vl_FieldStrenghValue_dBuV < (DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV + DABMW_serviceFollowingData.poorQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV))
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_POOR;
            		 }
            	else if (vl_FieldStrenghValue_dBuV < (DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.fieldStrength_dBuV + DABMW_serviceFollowingData.goodQuality_Threshold_Hysteresis.fmQuality.fieldStrength_dBuV))
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
            		 }
            	else
            		 {	 
            		 vl_quality = DABMW_SF_QUALITY_IS_GOOD;
            		 }
                }
            }
        else
            {
            if ( DABMW_SF_STRATEGY_FM_COMBINED_QUALITY == DABMW_serviceFollowingData.strategy_FM )
                {
                // we are on the field strenght based strategy
                
                vl_CombinedQ = vI_qualityValue.fmQuality.combinedQ;
                
            	if (vl_CombinedQ < DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_POOR;
            		 }
            	else if (vl_CombinedQ < DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.combinedQ)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
            		 }
            	else
            		 {	 
            		 vl_quality = DABMW_SF_QUALITY_IS_GOOD;
            		 }
                }
            else
                {
                //           Default : DABMW_SF_STRATEGY_FM_FIELD_STRENGH
           
                 // we are on the field strenght based strategy
                
                vl_FieldStrenghValue_dBuV = vI_qualityValue.fmQuality.fieldStrength_dBuV;
                
                // we are on the field strenght based strategy
            	if (vl_FieldStrenghValue_dBuV < DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_POOR;
            		 }
            	else if (vl_FieldStrenghValue_dBuV < DABMW_serviceFollowingData.goodQuality_Threshold.fmQuality.fieldStrength_dBuV)
            		 {
            		 vl_quality = DABMW_SF_QUALITY_IS_MEDIUM;
            		 }
            	else
            		 {	 
            		 vl_quality = DABMW_SF_QUALITY_IS_GOOD;
            		 }
                }
                
            }

        }
			 
	 return vl_quality; 
}



/*
*  Procedure to check if an new found AF is better current Alternate
* output : true if new AF better
*
*/
tBool DABMW_ServiceFollowing_IsNewFoundAFBetter(DABMW_mwAppTy vI_newAFApp, DABMW_SF_QualityTy vI_newFoundQuality)
{
	tBool vl_res = false;
	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
	tBool vl_newAFIsDAB = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_newAFApp);
    
    tU32 vl_new_alternateCandidate_frequency = DABMW_INVALID_FREQUENCY;
    
    vl_new_alternateCandidate_frequency = DABMW_ServiceFollowing_ExtInt_GetFrequencyFromApp(vI_newAFApp);

	/* 1st check */
	/* we compare only 2 AF with same bearer and the app should be different 
	* else : it is assume that we take the new found...
	*/
	if (DABMW_INVALID_FREQUENCY == DABMW_serviceFollowingStatus.alternateFrequency)
    {
        /* we have no alternate so this new one is better :)... */
        return true;
    }
    
	if (vl_alternateIsDab != vl_newAFIsDAB)
		{
		/* not the same 2 bearer : return
		*/
		return true;
		}

	if (vI_newAFApp == DABMW_serviceFollowingStatus.alternateApp)
		{
		/* same app for the 2 alternate... 
		* this not ok
		*/
		return true;
		}


    // extension with new quality format
    //

    // calculate the current value
    if (true == vl_alternateIsDab)
    	{
        if (true == DABMW_ServiceFollowing_qualityA_better_qualityB_DAB(vI_newFoundQuality, DABMW_serviceFollowingStatus.alternate_Quality, 
                                                                    vl_new_alternateCandidate_frequency, DABMW_serviceFollowingStatus.alternateFrequency))
				{
				/* the alternate is same quality but ok to switch because better than current alternate */
				vl_res = true;
				}
    	}
    else 
    	{
        // Deal with audio application FM
    		/* switch to new AF only if better  */
      
            if (true == DABMW_ServiceFollowing_qualityA_better_qualityB_FM(vI_newFoundQuality, DABMW_serviceFollowingStatus.alternate_Quality,
                                                                        vl_new_alternateCandidate_frequency, DABMW_serviceFollowingStatus.alternateFrequency))
    	    {
    		    /* the alternate is same quality but ok to switch because better than current alternate */
    			vl_res = true;
    		}		
     	} 
	
	return vl_res;
}

/*
*  Procedure to check if an new found AF is acceptable
* ie medium or good
*
*/
tBool DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_mwAppTy vI_newAFApp, DABMW_SF_QualityTy vI_newFoundQuality)
{
	tBool vl_res = false;
	DABMW_SF_QualityStatusTy vl_alternateQualityNew ;
	tBool vl_newAFIsDAB = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_newAFApp);

    // new quality not measured again : this is already done by caller : in vI_newFoundQuality
    //
    // DABMW_SF_QualityTy vl_new_measQualityValueAlternateNew;
    //    vl_new_measQualityValueAlternateNew = DABMW_ServiceFollowing_QualityInit();
        


    // extension new quality handling
    //
    if (true == vl_newAFIsDAB)
    	{
        // Deal with audio application DAB
		vl_alternateQualityNew = DABMW_ServiceFollowing_GetQualityStatusDAB(vI_newFoundQuality);
    	}
    else 
    	{
        // Deal with audio application FM
		vl_alternateQualityNew = DABMW_ServiceFollowing_GetQualityStatusFM(vI_newFoundQuality);		
     	}      

            // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    
            tU32 vl_new_alternateCandidate_frequency = DABMW_INVALID_FREQUENCY;
    
            vl_new_alternateCandidate_frequency = DABMW_ServiceFollowing_ExtInt_GetFrequencyFromApp(vI_newAFApp);
            
            // check if it is one lock freq...
            //
            if (vl_new_alternateCandidate_frequency == DABMW_serviceFollowingData.lockFrequency_1)
                {
                if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                    {
                    vl_alternateQualityNew = DABMW_SF_QUALITY_IS_MEDIUM;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                    {
                    vl_alternateQualityNew = DABMW_SF_QUALITY_IS_POOR;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                    {
                    vl_alternateQualityNew = DABMW_SF_QUALITY_IS_GOOD;
                    }   
                }
            else if (vl_new_alternateCandidate_frequency == DABMW_serviceFollowingData.lockFrequency_2)
                {
                if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                    {
                    vl_alternateQualityNew = DABMW_SF_QUALITY_IS_MEDIUM;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                    {
                    vl_alternateQualityNew = DABMW_SF_QUALITY_IS_POOR;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                    {
                    vl_alternateQualityNew = DABMW_SF_QUALITY_IS_GOOD;
                    }               
                }
#endif


    if ((DABMW_SF_QUALITY_IS_POOR != vl_alternateQualityNew)
        && (DABMW_SF_QUALITY_IS_NO_AUDIO != vl_alternateQualityNew)
        && (DABMW_SF_QUALITY_IS_NO_SYNC != vl_alternateQualityNew)
        )
        {
		/* quality is better take new one */
		vl_res = true;
        }
	
	return vl_res;


}

/*
*  Procedure to check if a quality is above the threshold for RDS quality decoding capability 
*
*/
tBool DABMW_ServiceFollowing_IsFMQualityEnoughForRdsDecoding(DABMW_SF_amfmQualityTy vI_newFoundQuality)
{
	tBool vl_res = false;

    // compare the quality to the RDS thresholds
    // for the moment, focus only on the dbUV

    if (vI_newFoundQuality.fieldStrength_dBuV >= DABMW_serviceFollowingData.PI_decoding_QualityThreshold.fmQuality.fieldStrength_dBuV)
    {
        vl_res = true;
    }
       
	return vl_res;
}

/*
*  Procedure to check if a quality is above the threshold for RDS quality decoding capability 
*
*/
tBool DABMW_ServiceFollowing_IsOutOfSync(DABMW_SF_dabQualityTy vI_DabQualityValue)
{
	tBool vl_res = false;

	// the sync status value is a flag with 3 information
	// 
	// Signal reception information : DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SIG
	// Synchronization information : DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SYNC
	// MCI data decoding informantion : DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_MCI
	//
	// In theory for proper tune, all 3 signals should be available
	// but some tests demonstrate that audio-ber may still be ok despite MCI not decoded
	// so keep the sync flag only
	//

#if 1
 	if (DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SYNC != (vI_DabQualityValue.sync_status & DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SYNC))
    {
       	vl_res = true;
    }
	else
	{
		vl_res = false;
	}
#endif 

	// for now : always consider sync 
	// 

	
	return vl_res;
}


// Procedure to init the quality information 

DABMW_SF_QualityTy DABMW_ServiceFollowing_QualityInit()
{
    DABMW_SF_QualityTy vl_quality;

     vl_quality.fmQuality = DABMW_ServiceFollowing_ExtInt_AmFmQualityInit();
     vl_quality.dabQuality = DABMW_ServiceFollowing_DabQualityInit();
     vl_quality.qualityStatus = DABMW_SF_QUALITY_IS_POOR;

     return vl_quality;
}


// procedure to init the DAB quality information
DABMW_SF_dabQualityTy DABMW_ServiceFollowing_DabQualityInit()
{
    DABMW_SF_dabQualityTy vl_quality;

     vl_quality.fic_ber = DABMW_INVALID_DATA;
     vl_quality.audio_ber = DABMW_INVALID_DATA;
     vl_quality.audio_ber_level = DABMW_INVALID_DATA_BYTE;
     vl_quality.kindOfComponentType = DABMW_COMPONENTTYPE_IS_NO_TYPE;
     vl_quality.mute_flag = false;
     vl_quality.service_selected = false;
     vl_quality.reed_solomon_information = DABMW_INVALID_DATA_BYTE;
	 vl_quality.sync_status = DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_NOTTUNED;

     return vl_quality;
}

// Procedure which compare quality A and quality B for 2 DAB 
//
// returns true is quality A is better than B
//
// 
tBool DABMW_ServiceFollowing_qualityA_better_qualityB_DAB(DABMW_SF_QualityTy vI_qualityA, DABMW_SF_QualityTy vI_qualityB, tU32 vI_freqA, tU32 vI_freqB)
{
    tBool vl_res;
    DABMW_SF_QualityStatusTy vl_QualityStatusA,vl_QualityStatusB;

    if (DABMW_INVALID_DATA == vI_qualityB.dabQuality.fic_ber)
    {
        /* the quality B is not set : so it is better */
        return true;
    }

        vl_QualityStatusA = DABMW_ServiceFollowing_GetQualityStatusDAB(vI_qualityA);
               
        vl_QualityStatusB = DABMW_ServiceFollowing_GetQualityStatusDAB(vI_qualityB);

        // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    
        // check if it is one lock freq...
        //
        if (vI_freqA == DABMW_serviceFollowingData.lockFrequency_1)
            {
            if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                vl_QualityStatusA = DABMW_SF_QUALITY_IS_MEDIUM;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_QualityStatusA = DABMW_SF_QUALITY_IS_POOR;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_QualityStatusA = DABMW_SF_QUALITY_IS_GOOD;
                }   
            }
        else if (vI_freqA == DABMW_serviceFollowingData.lockFrequency_2)
            {
            if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                 vl_QualityStatusA = DABMW_SF_QUALITY_IS_MEDIUM;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_QualityStatusA = DABMW_SF_QUALITY_IS_POOR;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_QualityStatusA = DABMW_SF_QUALITY_IS_GOOD;
                }               
            }
#endif
        
            // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
        
            // check if it is one lock freq...
            //
            if (vI_freqB == DABMW_serviceFollowingData.lockFrequency_1)
                {
                if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                    {
                    vl_QualityStatusB = DABMW_SF_QUALITY_IS_MEDIUM;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                    {
                    vl_QualityStatusB = DABMW_SF_QUALITY_IS_POOR;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                    {
                    vl_QualityStatusB = DABMW_SF_QUALITY_IS_GOOD;
                    }   
                }
            else if (vI_freqB == DABMW_serviceFollowingData.lockFrequency_2)
                {
                if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                    {
                     vl_QualityStatusB = DABMW_SF_QUALITY_IS_MEDIUM;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                    {
                    vl_QualityStatusB = DABMW_SF_QUALITY_IS_POOR;
                    }
                else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                    {
                    vl_QualityStatusB = DABMW_SF_QUALITY_IS_GOOD;
                    }               
                }
#endif


    /*Quality A better  only if better with hysteris */
    
    if (vl_QualityStatusA > vl_QualityStatusB)
        {
        /* quality is better take new one */
        vl_res = true;
        }
    else if (vl_QualityStatusA == vl_QualityStatusB)
    {
        // currently based on ficBer only
        if ((vI_qualityA.dabQuality.fic_ber + DABMW_SF_HYSTERESIS_DAB_FICBER) < vI_qualityB.dabQuality.fic_ber)
        {
            vl_res = true;
        }
        else
        {
            vl_res = false;
        }
        }
    else
    {
        vl_res = false;
    }

    return vl_res;
}


// Procedure which compare quality A and quality B for 2 FM 
//
// returns true is quality A is better than B
//
// 
tBool DABMW_ServiceFollowing_qualityA_better_qualityB_FM(DABMW_SF_QualityTy vI_qualityA, DABMW_SF_QualityTy vI_qualityB, tU32 vI_freqA, tU32 vI_freqB)
{
    tBool vl_res = false;
	DABMW_SF_QualityStatusTy vl_QualityStatusA,vl_QualityStatusB;

    // depending on the strategy, choose the right comparison
    //

	

    if (DABMW_INVALID_DATA_S16 == vI_qualityB.fmQuality.fieldStrength_dBuV)
    {
        /* the quality B is not set : so it is better */
        return true;
    }

    vl_QualityStatusA = DABMW_ServiceFollowing_GetQualityStatusFM(vI_qualityA);
           
    vl_QualityStatusB = DABMW_ServiceFollowing_GetQualityStatusFM(vI_qualityB);

    // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG

    // check if it is one lock freq...
    //
    if (vI_freqA == DABMW_serviceFollowingData.lockFrequency_1)
        {
        if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_GOOD;
            }   
        }
    else if (vI_freqA == DABMW_serviceFollowingData.lockFrequency_2)
        {
        if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
             vl_QualityStatusA = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_GOOD;
            }               
        }
#endif
    
        // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    
        // check if it is one lock freq...
        //
        if (vI_freqB == DABMW_serviceFollowingData.lockFrequency_1)
            {
            if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_MEDIUM;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_POOR;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_GOOD;
                }   
            }
        else if (vI_freqB == DABMW_serviceFollowingData.lockFrequency_2)
            {
            if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                 vl_QualityStatusB = DABMW_SF_QUALITY_IS_MEDIUM;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_POOR;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_GOOD;
                }               
            }
#endif

    
    /*Quality A better  only if better with hysteris */
     if (vl_QualityStatusA > vl_QualityStatusB)
        {
        // add an hysteresis here : 
        
        /* quality is better take new one */
        vl_res = true;
     }
    else if (vl_QualityStatusA == vl_QualityStatusB)
    {
        if ( DABMW_SF_STRATEGY_FM_COMBINED_QUALITY == DABMW_serviceFollowingData.strategy_FM )
        {
           // based on combined Q
           if (vI_qualityA.fmQuality.combinedQ > (vI_qualityB.fmQuality.combinedQ + DABMW_SF_HYSTERESIS_FM_COMBINED_Q))
            {
                vl_res = true;
            }
            else
            {
                vl_res = false;
            }
        }   
        else       
        {
            // default : ( DABMW_SF_STRATEGY_FM_FIELD_STRENGH == DABMW_serviceFollowingData.strategy_FM )
            //  based on field strengh 
            if (vI_qualityA.fmQuality.fieldStrength_dBuV > (vI_qualityB.fmQuality.fieldStrength_dBuV + DABMW_SF_HYSTERESIS_FM_dBuV))
            {
                vl_res = true;
            }
            else
            {
                vl_res = false;
            }
        }
    }
    else // vl_QualityStatusA < vl_QualityStatusB
    {
        vl_res = false;        
    }

    return vl_res;
}

// Procedure which compare quality A and quality B for 1 DAB & 1 FM 
//
// returns true is quality A is better than B
//
// 
tBool DABMW_ServiceFollowing_qualityA_DAB_better_qualityB_FM(DABMW_SF_QualityTy vI_qualityA, DABMW_SF_QualityTy vI_qualityB, tU32 vI_freqA, tU32 vI_freqB)
{
    tBool vl_res = false;
	DABMW_SF_QualityStatusTy vl_QualityStatusA,vl_QualityStatusB;

    // depending on the strategy, choose the right comparison
    //

	 DABMW_SF_PRINTF( TR_LEVEL_USER_1, "DABMW_ServiceFollowing_qualityA_DAB_better_qualityB_FM :  Freq_A = %d, Quality_A %d (dBuV) ; freq_B = %d, Quality_B %d (dBuV)\n", 
	 				vI_freqA, vI_qualityA.fmQuality.fieldStrength_dBuV,vI_freqB, vI_qualityB.fmQuality.fieldStrength_dBuV);


    if (DABMW_INVALID_DATA_S16 == vI_qualityB.fmQuality.fieldStrength_dBuV)
    {
        /* the quality B is not set : so it is better */
        return true;
    }

    vl_QualityStatusA = DABMW_ServiceFollowing_GetQualityStatusDAB(vI_qualityA);
           
    vl_QualityStatusB = DABMW_ServiceFollowing_GetQualityStatusFM(vI_qualityB);

    // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG

    // check if it is one lock freq...
    //
    if (vI_freqA == DABMW_serviceFollowingData.lockFrequency_1)
        {
        if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_GOOD;
            }   
        }
    else if (vI_freqA == DABMW_serviceFollowingData.lockFrequency_2)
        {
        if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
            {
             vl_QualityStatusA = DABMW_SF_QUALITY_IS_MEDIUM;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_POOR;
            }
        else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
            {
            vl_QualityStatusA = DABMW_SF_QUALITY_IS_GOOD;
            }               
        }
#endif
    
        // Add-on for the lock freq feature
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    
        // check if it is one lock freq...
        //
        if (vI_freqB == DABMW_serviceFollowingData.lockFrequency_1)
            {
            if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_MEDIUM;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_POOR;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_1_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_GOOD;
                }   
            }
        else if (vI_freqB == DABMW_serviceFollowingData.lockFrequency_2)
            {
            if (DABMW_serviceFollowingData.lockFrequency_2_level== DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM)
                {
                 vl_QualityStatusB = DABMW_SF_QUALITY_IS_MEDIUM;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_LOW)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_POOR;
                }
            else if (DABMW_serviceFollowingData.lockFrequency_2_level == DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD)
                {
                vl_QualityStatusB = DABMW_SF_QUALITY_IS_GOOD;
                }               
            }
#endif

    // compare the quality status 
    // DAB is preferred if Quality >= FM quality
    //

	// specific handling of no audio
    if ((DABMW_SF_QUALITY_IS_NO_AUDIO == vl_QualityStatusA)
		|| (DABMW_SF_QUALITY_IS_NO_SYNC == vl_QualityStatusA))
    	{
        vl_QualityStatusA = DABMW_SF_QUALITY_IS_POOR;			
    	}

    if (vl_QualityStatusA >= vl_QualityStatusB)
    {
        vl_res = true;
    }
    else 
    {

       vl_res = false;
    }

    // specific cases handling : 
    // if we look for an alternate for specific procedure : FM only..; then we should consider it is better :)
    // 
    if (DABMW_SF_SCAN_SEAMLESS == DABMW_serviceFollowingStatus.ScanTypeDab)
    {
        // the DAB should be considered better than FM , so that we reselect it !!
        vl_res = true;
    }
    else if (DABMW_SF_SCAN_SEAMLESS == DABMW_serviceFollowingStatus.ScanTypeFm)
    {
        // good to choose an FM alternate for seamless :)
        
        vl_res = false;
    }

    // for now, if DAB is available, not poor, always keep the DAB ... 
    if ((DABMW_SF_QUALITY_IS_POOR != vl_QualityStatusA)
		&& (DABMW_SF_QUALITY_IS_NO_AUDIO != vl_QualityStatusA)
		&& (DABMW_SF_QUALITY_IS_NO_SYNC != vl_QualityStatusA))
		{
        vl_res = true;
        }
        
    return vl_res;


}

DABMW_SF_dabQualityTy DABMW_ServiceFollowing_DAB_GetQuality(DABMW_mwAppTy vI_app)
{
    DABMW_SF_dabQualityTy vl_dabQuality = DABMW_ServiceFollowing_DabQualityInit();
    tU32   vl_ensembleID;
    tU32   vl_serviceID;
    tSInt  vl_res = OSAL_OK;
    


	vl_dabQuality = DABMW_ServiceFollowing_ExtInt_GetDabQuality(vI_app);
	/*
    // fic ber only for the moment
    vl_dabQuality.fic_ber = DABMW_ServiceFollowing_ExtInt_GetDabQualityFicBer(vI_app);

    // get the audio ber
     vl_dabQuality.audio_ber= DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBer(vI_app);

    // get reed solomon
     vl_dabQuality.reed_solomon_information = DABMW_ServiceFollowing_ExtInt_GetDabQualityReedSolomon(vI_app);  

    // get audio_ber_level
    vl_dabQuality.audio_ber_level = DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBerLevel(vI_app); 


    // get mute flag
    vl_dabQuality.mute_flag = DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioMuteFlag(vI_app);
    */
    // Get sync Status
    // this is up to date
	//	vl_dabQuality.sync_status = DABMW_ServiceFollowing_ExtInt_GetDabSyncStatus(vI_app);
   
    // get the service type
    if (vI_app == DABMW_serviceFollowingStatus.originalAudioUserApp)
    {
        vl_ensembleID = DABMW_serviceFollowingStatus.originalEid;
        vl_serviceID = DABMW_serviceFollowingStatus.originalSid;
    }
    else if (vI_app == DABMW_serviceFollowingStatus.alternateApp)
    {
        vl_ensembleID = DABMW_serviceFollowingStatus.alternateEid;
        vl_serviceID = DABMW_serviceFollowingStatus.alternateSid;
    }
    else if (vI_app == DABMW_serviceFollowingStatus.currentAudioUserApp)
    {
        vl_ensembleID = DABMW_serviceFollowingStatus.currentEid;
        vl_serviceID = DABMW_serviceFollowingStatus.currentSid;
    }
    else
    {
        // this is an error
        vl_res = OSAL_ERROR;
    }
        
    if (OSAL_OK == vl_res)
    {
          
        DABMW_ServiceFollowing_ExtInt_GetServiceTypeFromServiceId(vl_ensembleID, vl_serviceID, &(vl_dabQuality.component_type), &(vl_dabQuality.kindOfComponentType));
    }
     
    
    return vl_dabQuality;
}


// Procedure to handle the measurements ponderations for original freq
// ie : find the balance between the new meas, and the store one to get new value.
// This procedure does the new measurement of quality on vI_app 
// and return the ponderated value for this app
// based on ponderation choice.
// 
DABMW_SF_QualityTy DABMW_ServiceFollowing_MeasureAndPonderateOriginalCell(DABMW_mwAppTy vI_app)
{
	tBool vl_cellIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_app);

    DABMW_SF_QualityTy  vl_meas_QualtiyValue;
    DABMW_SF_QualityTy vl_new_QualtiyValue;
    DABMW_SF_QualityTy vl_current_QualtiyValue;

     // init the structures 
    vl_current_QualtiyValue = DABMW_serviceFollowingStatus.original_Quality;
    vl_new_QualtiyValue = DABMW_serviceFollowingStatus.original_Quality;
    vl_meas_QualtiyValue = DABMW_serviceFollowingStatus.original_Quality;
    
    if (true == vl_cellIsDab)
    {
        // Deal with audio application DAB
     
        // update Eid in case... 
        // this is to manage the ecc which may now be updated
        //so if ECC still not ok try to get it
        if ((0x00FF0000 & DABMW_serviceFollowingStatus.originalEid) == 0x00FF0000)
        {
            DABMW_serviceFollowingStatus.originalEid = DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(DABMW_serviceFollowingStatus.originalAudioUserApp);
        }

        
        // get the Audio DAB Quality
        // DABMW_ServiceFollowing_GetDabAudioQuality(vI_app);
        
        vl_meas_QualtiyValue.dabQuality = DABMW_ServiceFollowing_DAB_GetQuality(vI_app);

        vl_new_QualtiyValue.dabQuality = DABMW_ServiceFollowing_MeasureAndPonderateCellDAB(vl_current_QualtiyValue.dabQuality, vl_meas_QualtiyValue.dabQuality);
      
        vl_new_QualtiyValue.dabQuality.service_selected = DABMW_serviceFollowingStatus.original_DabServiceSelected;
        
    }
    else // (false == vl_originalIsDab)
    {
        // for original, we should have vI_app = audio user... 
        // in FM do a get quality and then ponderate
        vl_meas_QualtiyValue.fmQuality = DABMW_ServiceFollowing_ExtInt_AmFmGetQuality(vI_app);   

        vl_new_QualtiyValue.fmQuality = DABMW_ServiceFollowing_MeasureAndPonderateCellFM(vl_current_QualtiyValue.fmQuality, vl_meas_QualtiyValue.fmQuality);        
    }
    
    return vl_new_QualtiyValue;
}

// Procedure to handle the measurements ponderations for alternate freq
// ie : find the balance between the new meas, and the store one to get new value.
// This procedure does the new measurement of quality on vI_app 
// and return the ponderated value for this app
// based on ponderation choice.
// 
DABMW_SF_QualityTy DABMW_ServiceFollowing_MeasureAndPonderateAlternateCell(DABMW_mwAppTy vI_app)
{
	tBool vl_alternateCellIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_app);
    tBool vl_originalCellIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);

    DABMW_SF_QualityTy vl_meas_QualtiyValue;
    DABMW_SF_QualityTy vl_new_QualtiyValue;
    DABMW_SF_QualityTy vl_current_QualtiyValue;

    DABMW_SF_QualityTy vl_meas_OriginalBackgroundQualtiyValue;
    DABMW_SF_QualityTy vl_new_OriginalBackgroundQualtiyValue;
    DABMW_SF_QualityTy vl_current_OriginalBackgroundQualtiyValue;

    

    tSInt vl_res = OSAL_OK;
    tSInt vl_res_oriQuality = OSAL_OK;

     // init the structures 
    vl_current_QualtiyValue = DABMW_serviceFollowingStatus.alternate_Quality;
    vl_new_QualtiyValue = DABMW_serviceFollowingStatus.alternate_Quality;
    vl_meas_QualtiyValue = DABMW_serviceFollowingStatus.alternate_Quality;
    
    if (true == vl_alternateCellIsDab)
    {
        // update Eid in case... 
        // this is to manage the ecc which may now be updated
        //so if ECC still not ok try to get it
        if ((0x00FF0000 & DABMW_serviceFollowingStatus.alternateEid) == 0x00FF0000)
        {
            DABMW_serviceFollowingStatus.alternateEid = DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(DABMW_serviceFollowingStatus.alternateApp);
        }
        
        // Deal with audio application DAB
        // if service selected
        
        if (true == DABMW_serviceFollowingStatus.alternate_DabServiceSelected)
        {
            
        //     DABMW_ServiceFollowing_GetDabAudioQuality(vI_app);
        }

		// if the alternate is not tune do nothing
		if (true == DABMW_serviceFollowingStatus.alternateTuned)
		{
            vl_meas_QualtiyValue.dabQuality = DABMW_ServiceFollowing_DAB_GetQuality(vI_app);

	        vl_new_QualtiyValue.dabQuality = DABMW_ServiceFollowing_MeasureAndPonderateCellDAB(vl_current_QualtiyValue.dabQuality, vl_meas_QualtiyValue.dabQuality);

	        vl_new_QualtiyValue.dabQuality.service_selected = DABMW_serviceFollowingStatus.alternate_DabServiceSelected; 
		}
        
    }
    else // (false == vl_alternateCellIsDab)
    {
        // The way to measure may be different depending on the situation 
        // if Original currently tuned on DAB : then we do a get Quality
        // if Original currently tuned on FM : we may measure the alternate by a simple AF check.
        // 
        // in FM do a get quality and then ponderate

        if (true == vl_originalCellIsDab)
        {
        	// TO BE UPDATED
        	// In multi-tuner case, this is nice : we assume the alternate has its own tuner, not use for anything
        	// so just get quality
        	// But, if we have less tuner... this does not work. The alternate tuner will be also used for bg scan
        	// so we should manage the sharing
        	//
            // mutli-tuner case Get quality directly

			if (true == DABMW_serviceFollowingStatus.alternateTuned)
			{
            	vl_meas_QualtiyValue.fmQuality = DABMW_ServiceFollowing_ExtInt_AmFmGetQuality(vI_app);		
			}
			else 
			{
				// we are in Ori = DAB, Alternate = FM  and not tuned
				// so Alternate has been 'suspended' for the bg for tuner sharing
				// Alternate APP should be MAIN_AMFM, but we measure it on bg
				// how to measure ?
				// Suspend the bg activity
				
				if (false == DABMW_ServiceFollowing_SuspendBgForAlternateFMMeasurement(DABMW_serviceFollowingStatus.alternateApp))
				{
					//  the bg tuner cannot be used ==> no measurement
					
				}
				else
				{
					// we can measure the alternate by AF check
					// on the bg scan app
	
	                // check quality using AF check which is better : alternate will be measured on same tuner than original
	               vl_res = DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(DABMW_BACKGROUND_AMFM_APP, // app on which to do the AF check
	                                                    DABMW_serviceFollowingStatus.alternateFrequency, // Alternate Frequency check
	                                                    &vl_meas_QualtiyValue.fmQuality ); // quality storage

       

					// we may need to resume the AF Seek ?
					if (OSAL_ERROR != vl_res)
	                {
	                        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
	                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "DABMW_Service_Following (DABMW_ServiceFollowing_MeasureAndPonderateAlternateCell) :  AF Check on app = %d (alternate app = %d), Freq = %d\n", 
	                                    DABMW_BACKGROUND_AMFM_APP,
	                                    DABMW_serviceFollowingStatus.alternateApp,
	                                    DABMW_serviceFollowingStatus.alternateFrequency);
	                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
	                   
#endif //   defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)                  
					}
                
					DABMW_ServiceFollowing_ResumeBgAfterAlternateFMMeasurement();
				}

			}

		
			 // check if valid measure
			if (DABMW_INVALID_DATA_U16 == vl_meas_QualtiyValue.fmQuality.fieldStrength)
			{
				vl_res = OSAL_ERROR;
			}

			vl_res_oriQuality = OSAL_ERROR; // mark that original is not measured on bg 

        }
        else 
        {
            // Changes : depending on the status, we might do AF check or just a 'get quality'
            //
            // we are in the FM-FM case;
            //
            // AF check : if VPA is on, or if a background search/check on-going which is using the 2nd tuner
           vl_meas_OriginalBackgroundQualtiyValue = DABMW_serviceFollowingStatus.original_Quality_onBackground;
           vl_new_OriginalBackgroundQualtiyValue = DABMW_serviceFollowingStatus.original_Quality_onBackground;
           vl_current_OriginalBackgroundQualtiyValue = DABMW_serviceFollowingStatus.original_Quality_onBackground;
    
            if (true == DABMW_ServiceFollowing_ExtInt_IsVpaEnabled())
            {

				//  the VPA has been broken : since we break the VPA, let's measure at the same time the Original on Background.
	            
	            // check quality using AF check which is better : alternate will be measured on same tuner than original
	            // codex #309933
	            vl_res = DABMW_ServiceFollowing_ExtInt_AmFmAFStartRequest(DABMW_serviceFollowingStatus.originalAudioUserApp, // app on which to do the AF check
	                                    DABMW_serviceFollowingStatus.alternateFrequency,
	                                    &vl_meas_QualtiyValue.fmQuality ); // quality storage

	            vl_res_oriQuality = DABMW_ServiceFollowing_ExtInt_AmFmAFStartRequest(DABMW_serviceFollowingStatus.originalAudioUserApp, // app on which to do the AF check
	                                    DABMW_serviceFollowingStatus.originalFrequency, // Alternate Frequency check
	                                    &vl_meas_OriginalBackgroundQualtiyValue.fmQuality ); // quality storage

	            // end the AF procedure 
	            DABMW_ServiceFollowing_ExtInt_AmFmAFEndRequest(DABMW_serviceFollowingStatus.originalAudioUserApp);
	            
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	                                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );              

				                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "DABMW_Service_Following (DABMW_ServiceFollowing_MeasureAndPonderateAlternateCell) :  AF START on app = %d, Freq = %d\n", 
	                                    DABMW_serviceFollowingStatus.originalAudioUserApp,
	                                    DABMW_serviceFollowingStatus.alternateFrequency
	                                    ); 
#endif
            }
			else  if (false == DABMW_ServiceFollowing_SuspendBgForAlternateFMMeasurement(DABMW_serviceFollowingStatus.alternateApp))
            {
				// Either the bg tuner cannot be used ==> measure the Alternate thru AF check interuption on original
	            
	            // check quality using AF check which is better : alternate will be measured on same tuner than original
				// do not measure again the original : this is on same tuner.
				
	            // codex #309933
	            vl_res = DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(DABMW_serviceFollowingStatus.originalAudioUserApp, // app on which to do the AF check
	                                    DABMW_serviceFollowingStatus.alternateFrequency,
	                                    &vl_meas_QualtiyValue.fmQuality ); // quality storage

				// consider the Original Background = Orignial 
				vl_res_oriQuality = OSAL_ERROR;
				DABMW_serviceFollowingStatus.original_Quality_onBackground  = DABMW_serviceFollowingStatus.original_Quality;
	            
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );              

			    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "DABMW_Service_Following (DABMW_ServiceFollowing_MeasureAndPonderateAlternateCell) :  AF CHECK on app = %d, Freq = %d\n", 
                                    DABMW_serviceFollowingStatus.originalAudioUserApp,
                                    DABMW_serviceFollowingStatus.alternateFrequency
                                    ); 
#endif
            }
            else
            {
            	// the background activity has been interupted
            	// 
            	
                // quality measurement on alternate app
                // either AF check, either tune ? 
                //
                // check if it is correctly tuned else do it.
                if (false == DABMW_ServiceFollowing_ExtInt_GetApplicationBusyStatus(DABMW_serviceFollowingStatus.alternateApp))
                {
                    //the tuner is not tuned, tuned it....
                    DABMW_ServiceFollowing_ExtInt_TuneFrequency(DABMW_serviceFollowingStatus.alternateApp, 
                    									DABMW_serviceFollowingStatus.alternateFrequency);  
                }
                
                // check quality using AF check which is better : alternate will be measured on same tuner than original
               vl_res = DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(DABMW_serviceFollowingStatus.alternateApp, // app on which to do the AF check
                                                    DABMW_serviceFollowingStatus.alternateFrequency, // Alternate Frequency check
                                                    &vl_meas_QualtiyValue.fmQuality ); // quality storage


                //  codex #309933
                // get original quality background
                // 
                vl_res_oriQuality = DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(DABMW_serviceFollowingStatus.alternateApp, // app on which to do the AF check
                                          DABMW_serviceFollowingStatus.originalFrequency, // Alternate Frequency check
                                            &vl_meas_OriginalBackgroundQualtiyValue.fmQuality ); // quality storage

				// we may need to resume the AF Seek ?
				if (OSAL_ERROR != vl_res)
                {
                        /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "DABMW_Service_Following (DABMW_ServiceFollowing_MeasureAndPonderateAlternateCell) :  AF Check on app = %d, Freq = %d\n", 
                                    DABMW_serviceFollowingStatus.alternateApp,
                                    DABMW_serviceFollowingStatus.alternateFrequency);
                    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
                   
#endif //   defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)                  
				}
                
				DABMW_ServiceFollowing_ResumeBgAfterAlternateFMMeasurement();

				
                // we assume this is correctly tuned...

            }

        }                         

        if (OSAL_ERROR != vl_res)               
            {
                // successull 
                vl_new_QualtiyValue.fmQuality = DABMW_ServiceFollowing_MeasureAndPonderateCellFM(vl_current_QualtiyValue.fmQuality, vl_meas_QualtiyValue.fmQuality);
            }
            else
            {
                // we do nothing this is not good !
            }

         if (OSAL_ERROR != vl_res_oriQuality)               
            {
                // successull 
                vl_new_OriginalBackgroundQualtiyValue.fmQuality = DABMW_ServiceFollowing_MeasureAndPonderateCellFM(vl_current_OriginalBackgroundQualtiyValue.fmQuality, vl_meas_OriginalBackgroundQualtiyValue.fmQuality);
                DABMW_serviceFollowingStatus.original_Quality_onBackground = vl_new_OriginalBackgroundQualtiyValue;
            }
            else
            {
                // we do nothing this is not good !
            }    
     }
        
    
    return vl_new_QualtiyValue;
}


// Procedure to handle the measurements ponderations for DAB quality
// ie : find the balance between the new meas, and the store one to get new value.
DABMW_SF_dabQualityTy DABMW_ServiceFollowing_MeasureAndPonderateCellDAB(DABMW_SF_dabQualityTy vI_current_DAB_Quality, DABMW_SF_dabQualityTy vI_measured_DAB_Quality )
{

    DABMW_SF_dabQualityTy vl_ponderatedQuality;

    // init structure
    vl_ponderatedQuality = vI_measured_DAB_Quality;
    
    // now ponderate the DAB : based on fic_ber only for the moment

    if (DABMW_INVALID_DATA == vI_current_DAB_Quality.fic_ber)
        {
            vl_ponderatedQuality.fic_ber= vI_measured_DAB_Quality.fic_ber;
        }
    else
        {
        /* evaluate new quality : 
            * alogirhtm is based on 2G one... 
            * take the increase directly. 
            * but ponderate the decrease
            *
            * if new measure > old , then take new measure
            * if new measure < old , ponderate.
            */
        if (vI_measured_DAB_Quality.fic_ber < vI_current_DAB_Quality.fic_ber)
            {
            /* in DAB : better = smaller value 
                    *  if we are here we take the best */
            vl_ponderatedQuality.fic_ber = vI_measured_DAB_Quality.fic_ber;
            }
        else
            {
            /* evaluate the new quality : ponderation of CURRENT + NEW */
            vl_ponderatedQuality.fic_ber = ((DABMW_serviceFollowingData.ponderation_CurrentQuality * vI_current_DAB_Quality.fic_ber) + 
                    (DABMW_serviceFollowingData.ponderation_NewQuality * vI_measured_DAB_Quality.fic_ber))
                    / (DABMW_serviceFollowingData.ponderation_CurrentQuality + DABMW_serviceFollowingData.ponderation_NewQuality);

            }

        }


    return vl_ponderatedQuality;
}

// Procedure to handle the measurements ponderations for FM quality
// ie : find the balance between the new meas, and the store one to get new value.

DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_MeasureAndPonderateCellFM(DABMW_SF_amfmQualityTy vI_current_FM_Quality, DABMW_SF_amfmQualityTy vI_measured_FM_Quality )
{
    DABMW_SF_amfmQualityTy vl_ponderatedQuality;

    // init structure
    vl_ponderatedQuality = vI_measured_FM_Quality;
              
    if (DABMW_INVALID_DATA_S16 == vI_current_FM_Quality.fieldStrength_dBuV)
        {
    	/* this is a start on new cell... ie 1st measurement */
    	vl_ponderatedQuality = vI_measured_FM_Quality;
    	}
    else
        {
        /* evaluate new quality : 
        	* alogirhtm is based on 2G one... 
        	* take the increase directly. 
        	* but ponderate the decrease
        	*
        	* if new measure > old , then take new measure
        	* if new measure < old , ponderate.
        	*/

		// just in case : if the measured was not correct, do not ponderate but keep old one
		// 
		if (DABMW_INVALID_DATA_S16 == vI_measured_FM_Quality.fieldStrength_dBuV)
		{
			vl_ponderatedQuality.fieldStrength_dBuV = vI_current_FM_Quality.fieldStrength_dBuV;
			vl_ponderatedQuality.fieldStrength = vI_current_FM_Quality.fieldStrength;
		}
    	else if (vI_measured_FM_Quality.fieldStrength_dBuV > vI_current_FM_Quality.fieldStrength_dBuV)
    	    {
    		/*  in fm = greater value
    			  *  if we are here we take the best */
    		vl_ponderatedQuality.fieldStrength_dBuV = vI_measured_FM_Quality.fieldStrength_dBuV;
            vl_ponderatedQuality.fieldStrength = vI_measured_FM_Quality.fieldStrength;
    		}
    	else
    	    {
            /* evaluate the new quality : ponderation of CURRENT + NEW */

            vl_ponderatedQuality.fieldStrength_dBuV = ((DABMW_serviceFollowingData.ponderation_CurrentQuality * vI_current_FM_Quality.fieldStrength_dBuV) + 
    					(DABMW_serviceFollowingData.ponderation_NewQuality * vI_measured_FM_Quality.fieldStrength_dBuV))
    					/ (DABMW_serviceFollowingData.ponderation_CurrentQuality + DABMW_serviceFollowingData.ponderation_NewQuality);

                
            vl_ponderatedQuality.fieldStrength = ((DABMW_serviceFollowingData.ponderation_CurrentQuality * vI_current_FM_Quality.fieldStrength) + 
                            (DABMW_serviceFollowingData.ponderation_NewQuality * vI_measured_FM_Quality.fieldStrength))
                            / (DABMW_serviceFollowingData.ponderation_CurrentQuality + DABMW_serviceFollowingData.ponderation_NewQuality);
                  	
    		}

        // same for combined Quality
        // just in case : if the measured was not correct, do not ponderate but keep old one
		// 
		if (DABMW_INVALID_DATA == vI_measured_FM_Quality.combinedQ)
		{
			vl_ponderatedQuality.combinedQ = vI_current_FM_Quality.combinedQ;

		}
        else if (vI_measured_FM_Quality.combinedQ > vI_current_FM_Quality.combinedQ)
                {
                /*  in fm = greater value
                              *  if we are here we take the best */
                vl_ponderatedQuality.combinedQ = vI_measured_FM_Quality.combinedQ;
                }
        else
                {
                /* evaluate the new quality : ponderation of CURRENT + NEW */
        
                vl_ponderatedQuality.combinedQ = ((DABMW_serviceFollowingData.ponderation_CurrentQuality * vI_current_FM_Quality.combinedQ) + 
                            (DABMW_serviceFollowingData.ponderation_NewQuality * vI_measured_FM_Quality.combinedQ))
                            / (DABMW_serviceFollowingData.ponderation_CurrentQuality + DABMW_serviceFollowingData.ponderation_NewQuality);
        
                    
                vl_ponderatedQuality.combinedQ = ((DABMW_serviceFollowingData.ponderation_CurrentQuality * vI_current_FM_Quality.combinedQ) + 
                                (DABMW_serviceFollowingData.ponderation_NewQuality * vI_measured_FM_Quality.combinedQ))
                                / (DABMW_serviceFollowingData.ponderation_CurrentQuality + DABMW_serviceFollowingData.ponderation_NewQuality);
                        
                }

        // for other just take latest
        // focus on detuning & adjacent
        //
        // vl_ponderatedQuality.detuning = (vI_measured_FM_Quality.detuning + vI_current_FM_Quality.detuning) / 2;
        // vl_ponderatedQuality.adjacentChannel= (vI_measured_FM_Quality.adjacentChannel + vI_current_FM_Quality.adjacentChannel) / 2;
        
        
        }

    return vl_ponderatedQuality;       
}

// procedure to configure the right parameter on seek interface
//
tVoid DABMW_ServiceFollowing_ConfigureAutoSeek()
{
   
    // when Service Following is enabled : only 
    // reconfigure the seek threshold which will apply
    //

    //
    // for that purpose, use existing procedure to CIS 
    //
    
    // call Procedure to control the AutoSeek (and seek) Threshold
    // both APP together
   
    DABWM_ServiceFollowing_ExtInt_SetAmFmAutoSeekThreshold(DABMW_ALL_APPLICATIONS, 
                                    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.fieldStrength_dBuV,
                                    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.adjacentChannel,
                                    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.detuning,
                                    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.multipath,
                                    DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.combinedQ, 
									DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.snr,
									DABMW_serviceFollowingData.poorQuality_Threshold.fmQuality.mpxNoise);
    
 
    return;
    
}


// procedure to configure the right parameter on seek interface
//
tVoid DABMW_ServiceFollowing_DisplayQualityFM(DABMW_SF_amfmQualityTy vI_quality, tPChar printHeader)
{
   
    /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, 
                            "%s quality => fieldStrength %d (dBuV), combinedQ %d, detuning %d, adjChannel %d, multipath %d\n", printHeader,
                            vI_quality.fieldStrength_dBuV,
                            vI_quality.combinedQ,
                            vI_quality.detuning,
                            vI_quality.adjacentChannel,
                            vI_quality.multipath);
                                  
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
    /* END TMP LOG */
     
    return;   
}

// procedure to configure the right parameter on seek interface
//
tVoid DABMW_ServiceFollowing_DisplayQualityDAB(DABMW_SF_dabQualityTy vI_quality,  tPChar printHeader)
{
#define DABMW_SF_COMPONENT_TYPE_STRING(x) ((x==DABMW_COMPONENT_TYPE_UNSPECIFIED_DATA__FOREGROUND_SOUND)?"DAB":((x==DABMW_COMPONENT_TYPE_DAB_PLUS)?"DAB+":((x==DABMW_COMPONENT_TYPE_MPEG2TS)?"DMB":"OTHERS")))

       /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
       DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, 
                               "%s : fic_ber %d, service_selected %s, audio_ber %d,  component_type %s, mute_flag %s, audio_ber_level %d, reed_solomon_info %d , sync_status %d\n", printHeader,
                               vI_quality.fic_ber,
                               DABMW_SERVICE_FOLLOWING_LOG_BOOL(vI_quality.service_selected),
                               vI_quality.audio_ber,
                               DABMW_SF_COMPONENT_TYPE_STRING(vI_quality.component_type),
                               DABMW_SERVICE_FOLLOWING_LOG_BOOL(vI_quality.mute_flag),
                               vI_quality.audio_ber_level,
                               vI_quality.reed_solomon_information,
                               vI_quality.sync_status);
                                     
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
       /* END TMP LOG */

    return;
}

#if defined(RADIO_CONTROL_C) || defined (SERVICE_FOLLOWING_C) || defined(SERVICE_FOLLOWING_BACKGROUND_C) || defined(SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_C)

//codex #319038 : audio status dab notification
//

tVoid DABMW_ServiceFollowing_AudioStatusCallbackNotification(DABMW_mwAppTy vI_application)
{
    // the structure to be filled


    // get the service type
    if (vI_application == DABMW_serviceFollowingStatus.originalAudioUserApp)
    {
        // this is the original
        DABMW_serviceFollowingStatus.original_Quality.dabQuality.audio_ber_level = DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBerLevel(vI_application);

    }
    else if (vI_application == DABMW_serviceFollowingStatus.alternateApp)
    {
        // this is the app
        DABMW_serviceFollowingStatus.alternate_Quality.dabQuality.audio_ber_level = DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBerLevel(vI_application);
     }
    else
    {
        // this is an error
    }
    
}

#endif

#ifdef __cplusplus
}
#endif

#endif // CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // SERVICE_FOLLOWING_MEAS_AND_EVALUATE
// End of file

