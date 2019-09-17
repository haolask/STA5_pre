//!
//!  \file      service_following_mainloop.c
//!  \brief     <i><b> Service following implementation : main loop </b></i>
//!  \details   This file provides functionalities for service following main loop
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.10.07
//!  \bug       Unknown
//!  \warning   None
//!


#ifndef SERVICE_FOLLOWING_MAINLOOP_C
#define SERVICE_FOLLOWING_MAINLOOP_C
/* END EPR CHANGE */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "service_following_background.h"

#include "service_following_mainloop.h"

#include "service_following_meas_and_evaluate.h"

#include "service_following_audioswitch.h"

/* if seamless active */
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "service_following_seamless.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)

#ifdef __cplusplus
extern "C" {
#endif


/*
* Main entry for service following : 
* monitoring the state and step to be done.
* 
*/
tSInt DABMW_ServiceFollowing_MainLoop()
{
	DABMW_SF_EvaluationActionResultTy vl_NeededAction, vl_NeededActionAF;
	tBool vl_lastBackgroundSearchResult;
	
	switch (DABMW_serviceFollowingStatus.status)
		{
		case DABMW_SF_STATE_INIT:
			/*
			* Nothing specific to do
			* just check that if service following has been enabled meanwhile and information ready then we can start
			*/
			if ((DABMW_INVALID_SERVICE_ID != DABMW_serviceFollowingStatus.originalSid)
				&&
				(true == DABMW_serviceFollowingData.enableServiceFollowing)
				)
				{
				DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
				}
			break;
		case DABMW_SF_STATE_IDLE:
			/* Check if this is time for measurement check
			*/
			if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_Threshold))
				{
				/* we should get the measure and check if an action is needed 
				*/
				 vl_NeededAction = DABMW_ServiceFollowing_EvaluateOriginalCellSuitability();
				 DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

				if (DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY == vl_NeededAction)
					{
					/* current level is bad : start a service recovery 
					*/
							
					/* this is time : start the service recovery */
					/* data are : 
					* -> search for original service ID
					* -> priority given to prior stored info in landscape
					* -> order = DAB then FM.
					* -> all in one scan should be possible !!
					*  DABMW_ServiceFollowing_ServiceRecoveryScan(tU32 vI_SearchedServiceID)
					*/
					
					/* Check if this is time for full scan
					*/
					if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_ServiceRecovery))
						{
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_SERVICE_RECOVERY);
                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_RECOVERY;
                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_RECOVERY;

						DABMW_ServiceFollowing_ServiceRecoveryScan(DABMW_serviceFollowingStatus.originalSid);	
						}
					}								
				else if ((DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY == vl_NeededAction)
					|| (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR == vl_NeededAction))
					{
					/* An action is required to look for AF frequency 
					* let's prepare for a switch in case for AF measurements
					*
					* we could do a difference to scan DAB only..but for the moment we do not
					*/
					/* is that good time to do it ? 
					*/
					if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AFSearch))
						{
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
						/* background scan : for the moment, same than service recovery one */

						if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY == vl_NeededAction)
							{
                            if ( true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch))
                                {
                                DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
                                // CHANGES : even if FM is good, may be interesting to find an alternate FM.
                                 DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_FULL;
                                }
                            else
                                {
                                // for the moment always FULL SCAN for DAB when in IDLE
                                DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
                                // CHANGES : even if FM is good, may be interesting to find an alternate FM.
                                DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_AF;
                                }
                            
                            // the CheckTimeAction full time will indicate if time for full scan
                            // for the moment always FULL SCAN for DAB
							DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, true, true, true, DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch));
							}
						else
							{
                            if ( true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch))
                                {
                                DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
                                DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_FULL;
                                }
                            else
                                {
                                    // for the moment always FULL SCAN for DAB
                                DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
                                DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_AF;
                                }                           
                            // for the moment always FULL SCAN for DAB
							DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, true, true, true, DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch));
							}
						}
						/* all sounds good  : check if this may be time for landscape building */
						else if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LandscapeBuildingScan))
							{
							// do a landscape scan
							// act as if it was a normal scan			
							DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
							DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
							
	                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_LANDSCAPE;
	                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_LANDSCAPE;
	       
							DABMW_ServiceFollowing_LandscapeScan();
							}			
					}
				else if (DABMW_SF_EVALUATE_RES_NO_ACTION_GOOD_CELL == vl_NeededAction)
					{
                    // even if the cell is good, it may be nice to find an alternate
                    /* is that good time to do it ? 
					*/
					if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AFSearch))
						{
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);

                        if ( true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch))
                            {
                            DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
                            DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_FULL;
                            }
                        else
                            {
                            // for the moment always FULL SCAN for DAB
                            DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_AF;
                            DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_AF;
                            }                           
                            // for the moment always FULL SCAN for DAB
							DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, true, true, DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch), DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch));
						}                   
					/* all sounds good  : check if this may be time for landscape building */
					else if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LandscapeBuildingScan))
						{
						// do a landscape scan
						// act as if it was a normal scan			
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
						
                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_LANDSCAPE;
                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_LANDSCAPE;
       
						DABMW_ServiceFollowing_LandscapeScan();
						}
                    else
                        {
                            // do nothing
                        }
					}
#ifdef	CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
				else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_FM_ONLY == vl_NeededAction)
				    {
                    // the cell is good, it may be nice to find an alternate on DAB or FM 
                    // or to find a FM only for seamless....
                    /* is that good time to do it ? 
					*/
                    /* Check if it is time for background scanning of FM for delay estimation... */
                    if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AFSearch))
						{
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);

                        if ( true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch))
                            {
                            DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
                            DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_FULL;
                            }
                        else
                            {
                            // for the moment always FULL SCAN for DAB
                            DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_AF;
                            DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_AF;
                            }                           
                            // for the moment always FULL SCAN for DAB
							DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, true, true, DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch), DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch));
						}                                           
                    else if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FMLandscapeDelayEstimation))
						{
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
						/*set the timer info... */
						DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_NONE;
                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_SEAMLESS;

                        // we can do a full scan for both
						DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, false, true, true, true);
						}
                    else if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LandscapeBuildingScan))
						{
                        /* all sounds good  : check if this may be time for landscape building */
							// do a landscape scan
						// act as if it was a normal scan			
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
	

                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_LANDSCAPE;
                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_LANDSCAPE;
                        
						DABMW_ServiceFollowing_LandscapeScan();
						}                           
				    }
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
                else
                    {
                     // no action MEDIUM or POOR : 
                     // do nothing for the moment, in case it changes....
                     //
                    }
				}
			else
				{
				/* nothing to do */
				}
			break;

		case DABMW_SF_STATE_SERVICE_RECOVERY:
				/* Do we get a result of prior scan
				* if state = Service Recovery
				* and prior state = Background .. (SCAN, CHECK, AF_CHECK)
				* this means the background has just been ended....
				*/
				if ((DABMW_SF_STATE_BACKGROUND_SCAN == DABMW_serviceFollowingStatus.prevStatus)
					||
					(DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.prevStatus)
					||
					(DABMW_SF_STATE_AF_CHECK == DABMW_serviceFollowingStatus.prevStatus)
					)
					{
					/* we have ended the background search	*/
					/* did we find something ? */
					/* store the time it has been completed */

                    // set log evacuation flag 
                    DABMW_SF_LOG_INFO_UPDATE_BG_SEARCH_DONE;
            
					DABMW_serviceFollowingStatus.lastServiceRecoveryTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

                    // do not clear flags : this is nice to keep for log info...
                    //
                    // DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_NONE;
                    // DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_NONE;

					
					// Get the action needed on current 'recovery' cell
                    vl_NeededAction = DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY;
	

                    
                    if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.originalFrequency)
                        {
						vl_lastBackgroundSearchResult = DABMW_ServiceFollowing_LastBackgroundSearchResult(DABMW_serviceFollowingStatus.originalSid);				
						
						// Get the action needed on current 'recovery' cell
						vl_NeededAction = DABMW_ServiceFollowing_EvaluateOriginalCellSuitability();
  
                        }
                    else
                        {
                        // nothing found yet : look for initial searched PI
                        //
                        vl_lastBackgroundSearchResult = DABMW_ServiceFollowing_LastBackgroundSearchResult(DABMW_serviceFollowingData.initial_searchedPI);
                        }
                    
					if (true == vl_lastBackgroundSearchResult)
						{

						// Correction after FT experience 
						// it is possible that the original is back to acceptable after the 'recovery' procedure.
						// therefore if it is the case, do not immediately switch to the receovery cell
						// but act as if it was a new AF found...
						//

						/* This has been found : perfect !
						* When leaving background scan we are correctly tune as well !
						* tune the audio if needed and
						* so just change of state
						*/
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    	OSALUTIL_s32TracePrintf(0, TR_LEVEL_SYSTEM,TR_CLASS_APP_DABMW, "DABMW_Service_Following (Main Loop) :  Recovery searched SUCCESS\n");  
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                                            /* END TMP LOG */

				

						if (DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY == vl_NeededAction)
							{
							// Current cell is still not ok : switch to new one
						
							/* This has been found : perfect !
							* now enter idle mode on the 'current'
							*/
							//
							// we should set the audio port !!
							
	            			DABMW_ServiceFollowing_SetCurrentCellAsOriginal();									

	                        // back to Idle from Service Recovery
				            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
							DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
							}
						else
							{
							// current cell is acceptable : 
							// add the found one as a alternate
							
							 /* change state to IDLE with AF monitoring */
							 // here we know the service is not selected in outcome 
							
							 DABMW_ServiceFollowing_SetMonitoredAF(DABMW_serviceFollowingStatus.currentAudioUserApp,
																 DABMW_serviceFollowingStatus.currentSystemBand,
																 DABMW_serviceFollowingStatus.currentSid,
																 DABMW_serviceFollowingStatus.currentEid,
																 DABMW_serviceFollowingStatus.currentFrequency,
																 DABMW_serviceFollowingStatus.current_Quality,
																 false); 
							
							
							
							// get back to normal PI detection mode for FM which has been used
							// 
							  if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.currentAudioUserApp))
								  {
								  // set the RDS to normal mode 
								  DABMW_ServiceFollowing_EnableRds(DABMW_serviceFollowingStatus.currentAudioUserApp, false);
								  } 

							  // go to AF monitoring
							  //
							  // Note : here we do not resume the bg scan : we were in recovery, we will get back to normal operation later...
							  // important was finding a cell
							  DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
					  		  DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);			
							}
								
						}
					else
						{
						/* not successfull search ... 
						*/
						
						/* change the previous state : set it to recovery to indicate we are and remain in recovery
						*/
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_SERVICE_RECOVERY);
						}
					}
				else
					{
					/* Check if this is time for measurement check
					*/
					/* assume needed action is service recovery ...*/
					
					vl_NeededAction = DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY;
			
					/* if time for measurement : check new needed action */
					
					if (true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_Threshold))
						{
						/* if original cell exist check this is still needed
						* else assume still needed to find the requested services
						*/
						if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.originalFrequency)
							{
							vl_NeededAction = DABMW_ServiceFollowing_EvaluateOriginalCellSuitability();
							}
			
						DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
						}
			
					/* if still service recovery : either because new measurement done */
					if (DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY == vl_NeededAction)
						{
						
						/* Check if this is time for full scan
						*/
						if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_ServiceRecovery))
							{
							/* this is time : start the service recovery */
							/* data are : 
							* -> search for original service ID
							* -> priority given to prior stored info in landscape
							* -> order = DAB then FM.
							* -> all in one scan should be possible !!
							*  DABMW_ServiceFollowing_ServiceRecoveryScan(tU32 vI_SearchedServiceID)
							*/
							DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_RECOVERY;
                            DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_RECOVERY;

                            if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.originalFrequency)
                                {
							    DABMW_ServiceFollowing_ServiceRecoveryScan(DABMW_serviceFollowingStatus.originalSid);				
                                }
                            else
                                {
                                    // nothing found yet : look for initial searched PI
                                    //
                                DABMW_ServiceFollowing_ServiceRecoveryScan(DABMW_serviceFollowingData.initial_searchedPI);
                                }
                            
							}
						else
							{
							/* nothing to do but remain in service recovery*/
							}
						}
					else 
						{
						/* service recovery not needed any more */
			
						//TODO possiblity : reset service recovery info 
						
						/* change state to IDLE
						* idle will process with further correct state 
						*/
						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);				
						}
					}
				break;

		case DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF:
			/* Do we get a result of prior scan
			* if state = Service Recovery
			* and prior state = Background .. (SCAN, CHECK, AF_CHECK)
			* this means the background has just been ended....
			*/

            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
            /* END TMP LOG */

			if ((DABMW_SF_STATE_BACKGROUND_SCAN == DABMW_serviceFollowingStatus.prevStatus)
				||
				(DABMW_SF_STATE_BACKGROUND_CHECK == DABMW_serviceFollowingStatus.prevStatus)
				||
				(DABMW_SF_STATE_AF_CHECK == DABMW_serviceFollowingStatus.prevStatus)
				||
				(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF == DABMW_serviceFollowingStatus.prevStatus)
				)
				{
				/* we have ended the background search  */
				/* did we find something ? */
				/* store the time it has been completed */

                // SEARCH_FOR_AF means : nothing has been done....
                
				/* store time from last AF search  for repetition....
				*/
				DABMW_serviceFollowingStatus.lastSearchForAFTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

                // if it was a full scan, store the info (currently only on FM case)
                // either DAB or FM is FULL, or NONE for the not supported case.
                if ((DABMW_SF_SCAN_ALTERNATE_FM_FULL == DABMW_serviceFollowingStatus.ScanTypeFm) 
                    &&
                    (DABMW_SF_SCAN_ALTERNATE_DAB_FULL == DABMW_serviceFollowingStatus.ScanTypeDab)
                    )
                    {
                    DABMW_serviceFollowingStatus.lastFullScanForAFTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
                    }
                // correction stored actions result time
                // if it was for seamless : save the information
                //
                else if (DABMW_SF_SCAN_SEAMLESS == DABMW_serviceFollowingStatus.ScanTypeFm)
                    {
                        DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
                    }
                else if (DABMW_SF_SCAN_LANDSCAPE  == DABMW_serviceFollowingStatus.ScanTypeFm)
                    {              
                        DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
                    }
                
                // update log evacuation flags.
                DABMW_SF_LOG_INFO_UPDATE_BG_SEARCH_DONE;
                
	                // do not clear flags : this is nice to keep for log info...
                    //
                    // DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_NONE;
                    // DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_NONE;
				
				if (true == DABMW_ServiceFollowing_LastBackgroundSearchResult(DABMW_serviceFollowingStatus.originalSid))
					{
					/* This has been found : perfect !
					* When leaving background scan we are correctly tune as well !
					* tune the audio if needed and
					* so just change of state
					*/
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Main Loop) :  searched for AF SUCCESS\n");
   
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                                            /* END TMP LOG */


					/* change state to IDLE with AF monitoring */
                    // here we know the service is not selected in outcome 

					DABMW_ServiceFollowing_SetMonitoredAF(DABMW_serviceFollowingStatus.currentAudioUserApp,
														DABMW_serviceFollowingStatus.currentSystemBand,
														DABMW_serviceFollowingStatus.currentSid,
														DABMW_serviceFollowingStatus.currentEid,
														DABMW_serviceFollowingStatus.currentFrequency,
														DABMW_serviceFollowingStatus.current_Quality,
														false); 

                   
                   
       


                    // we may select the DabService Already if alternate is DAB, and current is fm...
                    // that save time for later...
                    // let's do that in 'SetMonitoredAF
                    //  DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;

                    // Change : continue the bg search until nothing more found, so that we finish the scan and be sure nothing better exist
                    // DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
					// DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);

                    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
				    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);

                    // if what has been found is a DAB... 
                    // then do not restart
                    // case FM - DAB => no point to look for a FM for now
                    // and may be better to do seamless estimation as well...
                    // case DAB - DAB => having 3 DAB, hm...
                    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.currentAudioUserApp))
                    {                  
                       DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
					   DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
                    }
                    else
                    {
                        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
                        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);

						DABMW_ServiceFollowing_SuspendMonitoredAF();
						
                        DABMW_ServiceFollowing_RestartBackgroundScan();
                    }
                    

					}
                else
					{
					/* not successfull search ... 
					*/
                            
					/* let's find next action */
					/* if alternate frequency exist and nothing found  : it means we should resume the alternate monitoring... 
					*   the typical (and only current case is : we were looking for DAB and did not find it
					*   resume alternate on FM
					*/
					if (OSAL_OK == DABMW_ServiceFollowing_ResumeMonitoredAF())
						{
 
                    /* TMP LOG */                                
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Main Loop) :  searched for AF -- FAILED, back to previous alternate, Freq = %d, app = %d \n",
                            DABMW_serviceFollowingStatus.alternateFrequency,
                            DABMW_serviceFollowingStatus.alternateApp);      
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                    /* END TMP LOG */  


						// get back to normal PI detection mode for FM which has been used
						// that is not required : only when resuming the AF since the 'bg' will be restarted
						// Correction : this is to be done on the AlternateApp not on the current... 
						// 
						if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
							{
							 // set the RDS to normal mode 
							 DABMW_ServiceFollowing_EnableRds(DABMW_serviceFollowingStatus.alternateApp, false);
							} 
								 

						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
					    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
						}
					else
						{
						/* change the previous state : and back to idle
						*/

                    /* TMP LOG */                                
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Main Loop) :  searched for AF -- FAILED, back to IDLE, no alternate \n");      
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                    /* END TMP LOG */

						DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
					    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
						}
					}
				}
			else
				{
				/* we should not come here : back to idle
				*/				
				/* change the previous state : and back to idle
				*/
			 /* TMP LOG */                                
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                    DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Main Loop) :  searched for AF -- FAILED -- error case , back to IDLE, no alternate \n");      
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
               /* END TMP LOG */

				DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
				DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
				}	

            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
            /* END TMP LOG */
			break;

		case DABMW_SF_STATE_IDLE_AF_MONITORING:
			/* in that state it is ~ like IDLE except, an AF exist
			* so should be monitored 
			* and evaluate a switch
			*/
			if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_Threshold))
				{
				/* we should get the measure and check if an action is needed 
				*/
				 vl_NeededAction = DABMW_ServiceFollowing_EvaluateOriginalCellSuitability();

				/* 
				* update the time for next measurement check
				*/
				DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();	

				if (DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY == vl_NeededAction)
					{
					/* current level is bad : start a service recovery 
					*/
							
					/* this is time : start the service recovery */
					/* data are : 
					* -> search for original service ID
					* -> priority given to prior stored info in landscape
					* -> order = DAB then FM.
					* -> all in one scan should be possible !!
					*  DABMW_ServiceFollowing_ServiceRecoveryScan(tU32 vI_SearchedServiceID)
					*/

                    // if the alternate is acceptable, switch to the alternate
                    //
                    if (true == DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_serviceFollowingStatus.alternateApp, 
                                                                    DABMW_serviceFollowingStatus.alternate_Quality))
                        {
                        /* do the cell switch */
						DABMW_ServiceFollowing_SwitchToAF();

                        // do not reset the alternate : just switch... 
                        // we never know : alternate may become good later on...
                        // let's the normal monitoring continue and decide
                        //DABMW_ServiceFollowing_ResetMonitoredAF();
                        
    					// now back to idle on that new setting 
    					// note : idle because we are in RECOVERY of service, so current original one is not ok.
                        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
                        // let's remain in the current state 
                        // as if a 'switch of cell' was done 
                        //DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);    				    
                        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
                        }
                    else
                        {
    					/* reset the alternate this is not valid for now in that state */
    					
    					/* reset the alternate frequency : nothing monitored anymore */
    					DABMW_ServiceFollowing_ResetMonitoredAF();

    					/* change state to recovery */
                        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
    				    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_SERVICE_RECOVERY);

                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_RECOVERY;
                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_RECOVERY;
                            
    					DABMW_ServiceFollowing_ServiceRecoveryScan(DABMW_serviceFollowingStatus.originalSid);	
                        }
					}	
                    /* EPR CHANGE */
                    /* We have an alternate, keep it instead of going back to idle and reset it
                                    * alternate removal will be done based on alternate status
                                    *
				    else if ((DABMW_SF_EVALUATE_RES_NO_ACTION_GOOD_CELL == vl_NeededAction)
                                || (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY == vl_NeededAction)
                                    )
					{			
                                
                                    // back to idle
                                    // which will take care of doing the right processing 'dab only scan'...
                                    //
					// back to idle 
					// Note : we have a valid alternate cell : shouldn't we keep it and remain in monitoring...
					//  the case  here is : original DAB and good....
					// because original FM we should have the result evaluate DAB only
					//
					// reset the alternate frequency : nothing monitored anymore 
					 DABMW_ServiceFollowing_ResetMonitoredAF();

                                    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
				        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);	                    
					}
					*/
					/* END EPR CHANGE */
                    /* EPR CHANGE */
                    /* We have an alternate, keep it instead of going back to idle and reset it
                                      * alternate removal will be done based on alternate status
                                     *

#ifdef	CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
				else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_FM_ONLY == vl_NeededAction)
					{
                                    // we have the original in DAB, GOOD level
                                    //
                                    // We may need to scan for FM to prepare the bg estimations for seamless...
                                    //
                                    // Depending on the alternate we do seamless estimation or not
                                    if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
                                        {
                                        // if we have the alternate in FM
                                        // this results means we should have a bg task for seamless estimate with orignal = GOOD
                    					// We should launch the delay estimation 
                    				       // if not already known
                    					// if known : back to idle 
                    					// 
                    					if (false == DABMW_ServiceFollowing_SSIsStoredInfoFromDatabase(DABMW_serviceFollowingStatus.originalFrequency, 
                    											DABMW_serviceFollowingStatus.originalEid,
                    											DABMW_serviceFollowingStatus.originalSid,
                    											DABMW_serviceFollowingStatus.alternateSid))
                    						{
                    						DABMW_ServiceFollowing_SeamlessEstimationStart();

                                            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
                    				        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_SEAMLESS);	
                    						}
                    					else
                    						{
                    						// no need to reestimate yet save time information 
                    						DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
                    						
                    						// now back to idle on that new setting 
                    						DABMW_ServiceFollowing_ResetMonitoredAF();

                                                             DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
                    				            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);	
                    						}
                                        }
                                    else
                                        {
                                        // act as a GOOD CELL => back to idle
                                        // reset the alternate frequency : nothing monitored anymore 
                    					DABMW_ServiceFollowing_ResetMonitoredAF();

                                        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
                    				    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);	
                                        }
					}
#endif // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
                    */
       /* END EPR CHANGE */     
				else
					{
                    // the Original is acceptable :
                    // either NO_ACTION_MEDIUM_CELL, or POOR_CELL or DAB_ONLY.
                    // or GOOD  level : DAB_ONLY, FM_ONLY, GOOD_CELL
                    // or MEDIUM (evaluate neighboor)
                    // this is the AF study which will state.
                    
					/* continue AF activity : 
					* get measurement & check suitability & action
					*/
					 vl_NeededActionAF = DABMW_ServiceFollowing_EvaluateAFCellSuitability();

					/* return 
					* = NO_ACTION means nothing to do continue the monitoring 
					* = EVALUATE NIEGHBOOR_IS_POOR = means current AF not ok, look for new one
					* = EVALUATE_NEIGHBOOR_DAB_ONLY = means it is time to look for a  DAB AF.
					* = EVALUATE_NEIGHBOOR = means we could look for a better AF
					* ... however only if correct period : so switch to state 'IDLE' which will decide
					* = CHANGE_CELL => switch is needed without seamless / direct switch
					* = CHANGE_CELL_SEAMLESS => switch needed with seamless estimation...
					*/

					if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR == vl_NeededActionAF)
						{
						/* the alternate AF is not good anymore.
						* Back to idle. idle state will do the processing
						* however the goal here is to look for an other cell : so alternate needs to be removed 
						*/

						/* reset the alternate frequency : nothing monitored anymore */
						DABMW_ServiceFollowing_ResetMonitoredAF();

						// we can force to look immediately for an AF if one in pipe
						//
						// BEFORE
						//
						// DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
						// DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
						// 
						//
						//AFTER

						 DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
    				     DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);

						// for the moment always FULL SCAN for DAB
                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_AF;
                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_AF;

    					DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, true, true, false, false);
    
						}
					else if (DABMW_SF_EVALUATE_CHANGE_CELL == vl_NeededActionAF)
						{
						/* do the cell switch */
						DABMW_ServiceFollowing_SwitchToAF();

                        // Change for Improvement : 
                        // Instead of going back to idle, just do an 'alternate switching' so continue being in IDLE_AF_MONITORING
                        // BEFORE
    					// now back to idle on that new setting 
                        //    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
    				    //    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
    				    // AFTER
    				    DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
    				    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);						
						}
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
					else if (DABMW_SF_EVALUATE_CHANGE_CELL_SEAMLESS == vl_NeededActionAF)
						{
						/* What should we do : 
						* either a full seamless estimation correlation start
						* either a direct siwtch reusing stored info 
						* 
						* so check store info
						*/
						//
                        // Check if a seamless estimation or confirmation may apply
                        // ... typically, we need to wait that the buffer is filled 
                        //
                        //  if (true == DABMW_ServiceFollowing_CheckIfSeamlessActionIsNeeded())
                            {			
                            // we should launch the delay estimation or reconf 
                            // however only if there is audio on alternate for long enough

							// we need to have the audio for a certain time before switching
						    // else we may get a switch to mute or no audio...
							// 
						  	if (true == DABMW_ServiceFollowing_CheckIfReadyToSeamlessSwitch())
						  		{
	    						/* We should launch the delay estimation */
	    						DABMW_ServiceFollowing_SeamlessEstimationStart();
	    							
	    						/* now back to idle on that new setting */
	                            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
	    				        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_SEAMLESS);
						  		}
							else
								{
								// we are not ready for estimation or switch... 
								
								}
                            }
						}
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
					else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY == vl_NeededActionAF)
						{
						/* do we need to look for other Alternate Frequency, typically on other bearer ?
						* case = DAB alternate, we may need to find a better DAB alternate... (DAB prefer, so do not care about FM...)
						*/
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
                        //
                        // Check if a seamless estimation or confirmation may apply
                        //
                        if (true == DABMW_ServiceFollowing_CheckIfSeamlessActionIsNeeded())
                        {
                           /* We should launch the delay estimation 
        					        * if not already known
        					        * if known : back to idle 
        					        */   					
        					DABMW_ServiceFollowing_SeamlessEstimationStart();

                            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
        				    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_SEAMLESS);	
                        }
						/* is that good time to do it ? 
						*/
						else if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AFSearch))
 #else
                        /* is that good time to do it ? 
						*/
						 if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AFSearch))    
 #endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
							{

                            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
				            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);

                            if ( true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch))
                                {
                                DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
                                }
                            else
                                {
                                DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_AF;
                                }   

                            DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_NONE;

							// Suspend Monitored AF before the scan
                            DABMW_ServiceFollowing_SuspendMonitoredAF();
							
							DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, true, false, DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AFSearch), false);
							}
	 				/* all sounds good  : check if this may be time for landscape building */
						else if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LandscapeBuildingScan))
							{
							// do a landscape scan
							// act as if it was a normal scan			
							DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
							DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
							
	                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_LANDSCAPE;
	                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_LANDSCAPE;
		   
							// Suspend Monitored AF before the scan
                            DABMW_ServiceFollowing_SuspendMonitoredAF();
							
							DABMW_ServiceFollowing_LandscapeScan();
							}
						}
                    else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR == vl_NeededActionAF)
                        {
                        /* do we need to look for other Alternate Frequency, typically on other bearer ?
						* case = we have a FM alternate we may need to find a better one : DAB alternate or a better FM alternate as well....
						*/
#ifdef	CONFIG_APP_SEAMLESS_SWITCHING_BLOCK						
                        // At first, check if we may do a seamless estimation for bg
                        // ie if we have the original in DAB, GOOD level, and the alternate in FM
                        // if seamless not stored, then run the seamless delay estimation
                        //
                        // We may need to scan for FM to prepare the bg estimations for seamless...
                        //
                        /*
                                            if ((DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_FM_ONLY == vl_NeededAction) 
                                                &&
                                                (false == DABMW_ServiceFollowing_SSIsStoredInfoFromDatabase(DABMW_serviceFollowingStatus.originalFrequency, 
                            											DABMW_serviceFollowingStatus.originalEid,
                            											DABMW_serviceFollowingStatus.originalSid,
                            											DABMW_serviceFollowingStatus.alternateSid))
                            					)*/
        					if (true == DABMW_ServiceFollowing_CheckIfSeamlessActionIsNeeded())
					        {
                            // we have the original in DAB, GOOD level, and the alternate in FM
                                                      
        					/* We should launch the delay estimation 
        					        * if not already known
        					        * if known : back to idle 
        					        */   					
        					DABMW_ServiceFollowing_SeamlessEstimationStart();

                            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
        				    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_SEAMLESS);	
        					
                            }
                        else
#endif //  CONFIG_APP_SEAMLESS_SWITCHING_BLOCK                           
                            {      
                            // if case we have only 2 tuners (DAB - FM) 
                            // then do not disturb the alternate to guarantee a good seamless
                            // so no bg search
                            // this will be done only when alternate will be lost.
                            // in 2 tuners cases we will not always do the search
    						// if DAB = original, looking for FM or DAB will disturb the alternate audio, so not good for seamless 
    						// if original = FM this is different, because switch to FM is done thru AF switch, and switch to DAB
    						// will remain ok since DAB is in advance...
                            

#if !defined(CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM) && defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK) 						    						

	   						if (!((true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp))
								&& (true == DABMW_serviceFollowingData.seamlessSwitchingMode)))
	   							{																				
#endif // defined (CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM) && defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)													 
								/* is that good time to do it ? 
	    						        */
	    						if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_AFSearch))
	    							{
	                                DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
	    				            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);

	                                if ( true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch))
	                                    {
	                                    DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_FULL;
	                                    DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_FULL;
	                                    }
	                                else
	                                    {
	                                        // for the moment always FULL SCAN for DAB
	                                    DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_ALTERNATE_DAB_AF;
	                                    DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_ALTERNATE_FM_AF;
	                                    }  

									// Suspend Monitored AF before the scan
	                            	DABMW_ServiceFollowing_SuspendMonitoredAF();
								
	    							DABMW_ServiceFollowing_AFScan(DABMW_serviceFollowingStatus.originalSid, true, true, DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch), DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_FullScanAFSearch));
	    							}
									 				/* all sounds good  : check if this may be time for landscape building */
									else if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LandscapeBuildingScan))
										{
										// do a landscape scan
										// act as if it was a normal scan			
										DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
										DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
										
				                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_LANDSCAPE;
				                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_LANDSCAPE;
					   
										// Suspend Monitored AF before the scan
			                            DABMW_ServiceFollowing_SuspendMonitoredAF();
										
										DABMW_ServiceFollowing_LandscapeScan();
										}
#if !defined(CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM) && defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK) 						    						
								}
#endif // defined (CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM) && defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
                            }
                        }
					else 
						{				
						/* should be no action here */
						/* nothing to do continue AF idle monitoring */
						/* ie state remains DABMW_SF_STATE_IDLE_AF_MONITORING
						*/
						// we may need to build / update the landscape
						if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LandscapeBuildingScan))
							{
	                        /* all sounds good  : check if this may be time for landscape building */
								// do a landscape scan
							// act as if it was a normal scan			
							DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
							DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF);
		

	                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_LANDSCAPE;
	                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_LANDSCAPE;
	                        					   
							// Suspend Monitored AF before the scan
		                    DABMW_ServiceFollowing_SuspendMonitoredAF();
							
							DABMW_ServiceFollowing_LandscapeScan();
							}    
						}
					}
				}
			else
				{
				/* nothing to do */
				}
			break;
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK			
		case DABMW_SF_STATE_IDLE_SEAMLESS:
			/* in this state we are ~like in IDLE_AF_MONITORING 
			* with the particularity that a cell change has been seen as needed 
			* and therefore an seamless estimation has been requested
			*
			* Anyhow : continue monitoring Original & Alternate cell
			* which should be DAB / FM or FM / DAB
			* 
			* wait estimation result
			*/

			/* Check time for measurement of Original and Alternate */
			if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_Threshold))
				{
				/* we should get the measure and check if an action is needed 
				*/
				 vl_NeededAction = DABMW_ServiceFollowing_EvaluateOriginalCellSuitability();

				/* 
				* update the time for next measurement check
				*/
				DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();	


				if (DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY == vl_NeededAction)
					{
					/* current level is bad : start a service recovery 
					*/
							
					/* this may be time to start the service recovery 
					* ie stop estimation and recover and/or switch without estimation
					*/
					
					DABMW_ServiceFollowing_SeamlessEstimationStop();

                    // if the alternate is acceptable, switch to the alternate
                    //
                    if (true == DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_serviceFollowingStatus.alternateApp, 
                                                                                        DABMW_serviceFollowingStatus.alternate_Quality))
                        {
                        /* do the cell switch */
                        DABMW_ServiceFollowing_SwitchToAF();
                    
                         // do not reset the alternate : just switch... 
                         // we never know : alternate may become good later on...
                        
                         DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
                         DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
                         }
                     else
                        {
                        // we definitively come to recover...;
                        //
    					/* reset the alternate this is not valid for now in that state */
    					
    					/* reset the alternate frequency : nothing monitored anymore */
    					DABMW_ServiceFollowing_ResetMonitoredAF();

    					/* change state to recovery */
                        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
    				    DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_SERVICE_RECOVERY);

                        DABMW_serviceFollowingStatus.ScanTypeDab = DABMW_SF_SCAN_RECOVERY;
                        DABMW_serviceFollowingStatus.ScanTypeFm = DABMW_SF_SCAN_RECOVERY;
                            
    					DABMW_ServiceFollowing_ServiceRecoveryScan(DABMW_serviceFollowingStatus.originalSid);	
                        }	
					}								
				else if ((DABMW_SF_EVALUATE_RES_NO_ACTION_GOOD_CELL == vl_NeededAction)
                    ||
                    (DABMW_SF_EVALUATE_RES_NO_ACTION_MEDIUM_CELL == vl_NeededAction)
                    ||
                    (DABMW_SF_EVALUATE_RES_NO_ACTION_POOR_CELL == vl_NeededAction)
					||
					(DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR == vl_NeededAction)
					||
					(DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY == vl_NeededAction)
					||
					(DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_FM_ONLY == vl_NeededAction)
					)
					{
        			    /* continue AF activity : 
        					* get measurement & check suitability & action
        					*/
        				vl_NeededActionAF = DABMW_ServiceFollowing_EvaluateAFCellSuitability();

        					
        				/* return 
        					* = NO_ACTION means nothing to do continue the monitoring 
        					* = EVALUATE NIEGHBOOR = means current AF not ok, look for new one
        					* = EVALUATE_NEIGHBOOR_DAB_ONLY = means it is time to look for a  DAB AF.
        					* ... however only if correct period : so switch to state 'IDLE' which will decide
        					* = CHANGE_CELL => switch is needed without seamless / direct switch
        					* = CHANGE_CELL_SEAMLESS => switch needed with seamless estimation...
        					*/

        				if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR == vl_NeededActionAF)
        				    {
        					/* the alternate is not good anymore... */
        					/* we should stop the seamless estimation and do AF search for a new one */

       						/* ie stop estimation and recover and/or switch without estimation */
        					
       						DABMW_ServiceFollowing_SeamlessEstimationStop();

       						/* reset the alternate frequency : nothing monitored anymore */
       						DABMW_ServiceFollowing_ResetMonitoredAF();

       						/* go back to Idle which will do the processing for AF search if needed.. */

                               
                            // DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
                            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
      				        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
      						}
       					else if ((DABMW_SF_EVALUATE_CHANGE_CELL == vl_NeededActionAF)
        						|| 
        						(DABMW_SF_EVALUATE_CHANGE_CELL_SEAMLESS == vl_NeededActionAF)
        						)
       						{
       						/* in those state : the need for change of cell is confirmed */
       						/* continue waiting for seamless estimation result */
       						}
       					else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY == vl_NeededActionAF)
       						{
       						/* in those state : the change of cell is not needed anymore
        						* but a look to a better DAB one... 
        						* the case should be : FM - DAB, look for a better DAB 
        						*/
       						/* let's continue waiting for seamless estimation result now that it is started */
       						}
                        else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR == vl_NeededActionAF)
       						{
       						/**/
       						/* let's continue waiting for seamless estimation result now that it is started */
       						}
       					else 
       						{				
       						/* should have cover all now */
       						}					
					}
				else 
					{
					/* should be nothing left ...*/
					}
				
				}
			break;

       case DABMW_SF_STATE_IDLE_SEAMLESS_SWITCHING:
        // state when switch is on going...
        //
        // this should be short : nothing specific to do : just wait.
        //
        
        break;
#endif // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
			
		case DABMW_SF_STATE_INITIAL_SERVICE_SELECTION:
		case DABMW_SF_STATE_BACKGROUND_CHECK:
		case DABMW_SF_STATE_BACKGROUND_WAIT_PI:
		case DABMW_SF_STATE_BACKGROUND_WAIT_PS:
		case DABMW_SF_STATE_BACKGROUND_WAIT_DAB:
		case DABMW_SF_STATE_BACKGROUND_SCAN:
		case DABMW_SF_STATE_BACKGROUND_WAIT_FIC:
		case DABMW_SF_STATE_AF_CHECK:
		case DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK:
			/* this is handle in background main loop */

			/* confirm current cell quality if applicable
			* this is valid only if we have an orignal and if we are not in initial service selection
			* ie prev state != inital service selection
			* since in that state there is no master cell selected...
			*/

            if ((DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.originalFrequency)
                &&
                (DABMW_SF_STATE_INITIAL_SERVICE_SELECTION != DABMW_serviceFollowingStatus.prevStatus)
                )
				{
				if (DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_Threshold))
					{
					/* we should get the measure and check if an action is needed 
					*/
					 vl_NeededAction = DABMW_ServiceFollowing_EvaluateOriginalCellSuitability();


					/* 
					* update the time for next measurement check
					*/
					DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();	

					/* next state may be changed depending on current serving cell : but let's handle it when back from activity
					*/

                    // 
                    // If we have an alternate, shouldn't we measure it ?
                    // 
                    if (DABMW_INVALID_FREQUENCY != DABMW_serviceFollowingStatus.alternateFrequency)
                        
                        {                
					    vl_NeededActionAF = DABMW_ServiceFollowing_EvaluateAFCellSuitability();

                        // in case a cell change is needed, 
                        // do it
                        //
                        if (DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY == vl_NeededAction)
        					{
        					/* current level is bad : start a service recovery 
        					        */
        							
        					/* this is time : start the service recovery */
        					/* data are : 
                    					 * -> search for original service ID
                    					* -> priority given to prior stored info in landscape
                    					* -> order = DAB then FM.
                    					* -> all in one scan should be possible !!
                    					*  DABMW_ServiceFollowing_ServiceRecoveryScan(tU32 vI_SearchedServiceID)
                    					*/

                            // if the alternate is acceptable, switch to the alternate
                            // !! but not in DAB to DAB case for now , since alternate DAB app is being used for bg search
                            //
                            if ((true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp))
                                && (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp))
                                )
                            {
                                // not in DAB to DAB case for now , since alternate DAB app is being used for bg search
                                //
                            }
                            else if ((true == DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_serviceFollowingStatus.alternateApp, 
                                                                            DABMW_serviceFollowingStatus.alternate_Quality))
                                      && (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR != vl_NeededActionAF)
                                      )
                                {
                                
                                	// We may need to stop the bg to get the tuner back 
                                	// 
                                	// Tuner are shared...
#ifndef	CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM
									// tuners are being shared...
									// in FM -FM switch can be done easily thru AF switch
									//
									// if alternate is FM and not tuned => stop & resume
									// 
									// cases should only be : 
									// DAB - FM
									// because FM -FM is ok thru AF switch
									// FM - DAB ==> the DAB has not been 'suspended'

									if (((true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp))
                               			 && (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
                               			 )
                               			 && (false == DABMW_serviceFollowingStatus.alternateTuned)
                               			 )
										{
										DABMW_ServiceFollowing_StopBgScan();
										DABMW_ServiceFollowing_ResumeMonitoredAF();
										}
#endif
								
                                	/* do the cell switch */
        							DABMW_ServiceFollowing_SwitchToAF();
                                }
							else 
								{
								// we should stop the bg search and resume it : 
								// priority to recovery
								// that will save time if bg check procedure is over in particular
								//
								// stop the background to do reconf and switch 
                           			DABMW_ServiceFollowing_StopBgScan();
								}
                            }
						 else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR == vl_NeededActionAF)
                            {
						 	/* reset the alternate frequency : nothing monitored anymore */
							// continue the scan
       						DABMW_ServiceFollowing_ResetMonitoredAF(); 
						 	}
                         else if ((DABMW_SF_EVALUATE_CHANGE_CELL == vl_NeededActionAF)
                                &&
                                (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
                                &&
                                (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.originalAudioUserApp)))
    						{
        						/* do the cell switch FM to FM */
        						DABMW_ServiceFollowing_SwitchToAF();
                                // nothing more
                            } 
                         else if ((DABMW_SF_EVALUATE_CHANGE_CELL == vl_NeededActionAF) 
                                &&
                                (DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.originalAudioUserApp)
                                != DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(DABMW_serviceFollowingStatus.alternateApp))
                                     )
                            {
                                /* do the cell switch FM to DAB or DAB to FM */
                                // note : this is valid only in 2 DAB tuners case
                                DABMW_ServiceFollowing_SwitchToAF();
                            }
#if (defined CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)                          
                        else if (DABMW_SF_EVALUATE_CHANGE_CELL_SEAMLESS == vl_NeededActionAF)
                            {
                            // a seamless cell change is needed....
                            // however, we are in bg processing 
                            // we cannot do the seamless switch even with right info in database
                            // because a reconf is needed 
                            // so choices are : 
                            // 1- stop the bg check/scan procedure and switch will be done at next evaluation
                            // 2- postpone the switch until the end of bg scan
                            // 3- handle in parallel switch and bg scan
                            // 
                            // decision : let's go for 1-
                            // being on the best cell is higher priority 
                            // and it is almost sure the bg scan will not bring anything because
                            // either we are in FM - DAB => finding a better DAB is almost unprobable
                            // either we are in DAB - FM => the FM is already better than the DAB, so FM good, finding a better FM is nice but won't bring much

                            
                            // let's wait or do the change only if DB not empty.... result will be ~ok


							// however only if there is audio on alternate for long enough
							
							   // we need to have the audio for a certain time before switching
							   // else we may get a switch to mute or no audio...
							   // 
							   if (true == DABMW_ServiceFollowing_CheckIfReadyToSeamlessSwitch())
								   {
								   	// stop the background to do reconf and switch 
                           			DABMW_ServiceFollowing_StopBgScan();
								   }
							   else
								   {
								   // we are not ready for estimation or switch... 							   
								   }                          

                            
                            // BEFORE
                            /*
                                                     tU32  vl_dabFrequency;
                                                     tU32  vl_dabEid;
                                                     tU32  vl_dabSid;
                                                     tU32  vl_fmPi;

                                                     if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp))
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
                                                
                                                      if (true == DABMW_ServiceFollowing_SSIsStoredInfoFromDatabase(vl_dabFrequency, 
                                                                                                    vl_dabEid,
                                                                                                    vl_dabSid,
                                                                                                    vl_fmPi,
                                                                                                    DABMW_SS_EstimationParameter.measureValidityDuration))
                                                        {

                                                             // do the cell switch FM to DAB or DAB to FM 
                                                             DABMW_ServiceFollowing_SwitchToAF();

                                                        }
                                                    */

                            }
#endif // #if (defined CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)                         
                    }                    
					}
				}
			/* Improvement : check the current cell quality, and possibly abort the procesude on going.... */
			 DABMW_ServiceFollowing_BackgroundMainLoop();
			 break;
			 
		default:
			break;
		}  


	// if SF activated 
	if (true == DABMW_serviceFollowingData.enableServiceFollowing)
	{
		DABMW_ServiceFollowing_SendChangeNotificationsToHost();
	}

	return OSAL_OK;

}


/* Procedure to check if a time has elapsed for a given event
* current event : 
*
* DABMW_SF_TimeCheck_FM_PI
* DABMW_SF_TimeCheck_DAB_TUNE
* DABMW_SF_TimeCheck_DAB_FIG
* DABMW_SF_TimeCheck_AutoSeek
* DABMW_SF_TimeCheck_ServiceRecovery
* 
*/
tBool DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheckTy eventToCheck)
{
    SF_tMSecond vl_systemTime;
	tBool vl_res = false;
    
    // Get the current system time
    vl_systemTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();

	switch (eventToCheck)
		{
		case DABMW_SF_TimeCheck_FM_PI:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.bg_timerStart) > DABMW_serviceFollowingData.maxTimeToDecodePi)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;
			
		case DABMW_SF_TimeCheck_FM_PS:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.bg_timerStart) > DABMW_serviceFollowingData.maxTimeToDecodePs)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;
			
		case DABMW_SF_TimeCheck_DAB_TUNE:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.bg_timerStart) > DABMW_SF_DAB_TUNE_SEARCH_DELTA_TIME_MS)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;
			
		case DABMW_SF_TimeCheck_DAB_FIG:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.bg_timerStart) > DABMW_SF_DAB_WAIT_FIC_ACQUISITION_TIME_MS)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;	
			
		case DABMW_SF_TimeCheck_AutoSeek:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.bg_timerStart) > DABMW_SF_FM_WAIT_AUTO_SEEK_TIME_MS)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;
			
		case DABMW_SF_TimeCheck_ServiceRecovery:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.lastServiceRecoveryTime) > DABMW_serviceFollowingData.ServiceRecoverySearchPeriodicity)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;

		case DABMW_SF_TimeCheck_Threshold:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.idle_timerStart) > DABMW_serviceFollowingStatus.measurementPeriodicityReference)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;
			
		case DABMW_SF_TimeCheck_AFSearch:
			if ((DABMW_INVALID_DATA == DABMW_serviceFollowingStatus.lastSearchForAFTime) 
				|| ((vl_systemTime - DABMW_serviceFollowingStatus.lastSearchForAFTime) > DABMW_serviceFollowingData.AFsearchPeriodicity)
				)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;

        case DABMW_SF_TimeCheck_FullScanAFSearch:
			if ((DABMW_INVALID_DATA == DABMW_serviceFollowingStatus.lastFullScanForAFTime)
				|| ((vl_systemTime - DABMW_serviceFollowingStatus.lastFullScanForAFTime) > DABMW_serviceFollowingData.FullScanAFsearchPeriodicity)
				)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;

		case DABMW_SF_TimeCheck_LastSwitch:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.lastSwitchTime) > DABMW_serviceFollowingStatus.SwitchDelayReference)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;
			
		case DABMW_SF_TimeCheck_LandscapeBuildingScan:
			if ((DABMW_INVALID_DATA == DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime)
				|| ((vl_systemTime - DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime) > DABMW_serviceFollowingData.LandscapeBuildingScanPeriodicity)
				)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
		
			break;

		case DABMW_SF_TimeCheck_FMLandscapeDelayEstimation:
			if ((vl_systemTime - DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime) > DABMW_serviceFollowingData.FMLandscapeDelayEstimationPeriodicity)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;
            
		case DABMW_SF_TimeCheck_LogInfoStatusEvacuation:
			if ((vl_systemTime - DABMW_serviceFollowing_LogInfo.lastStatusSentOut) > DABMW_serviceFollowingData.LogInfoStatusPeriodicity)
				{
				vl_res = true;
				}
			else
				{
				vl_res = false;
				}
			break;

        case DABMW_SF_TimeCheck_Hysteresis_DAB:
			if (((vl_systemTime - DABMW_serviceFollowingStatus.dab_hysteresis_start_time) < DABMW_serviceFollowingData.Quality_Threshold_Hysteresis_Duration)
                ||
                (DABMW_serviceFollowingStatus.dab_hysteresis_start_time == DABMW_INVALID_DATA))
				{
                // hysteresis still needed.    
				vl_res = true;
				}
			else
				{
                // hysteresis period is over  
				vl_res = false;
				}
			break;

        case DABMW_SF_TimeCheck_Hysteresis_FM:
            if (((vl_systemTime - DABMW_serviceFollowingStatus.fm_hysteresis_start_time) < DABMW_serviceFollowingData.Quality_Threshold_Hysteresis_Duration)
                ||
                (DABMW_serviceFollowingStatus.fm_hysteresis_start_time == DABMW_INVALID_DATA))
               {
               // hysteresis still needed.  
               vl_res = true;
               }
            else
               {
                // hysteresis period is over  
               vl_res = false;
               }
            break;

		case DABMW_SF_TimeCheck_MinToKeepAlternate:
			 if ((vl_systemTime - DABMW_serviceFollowingStatus.deltaTimeFromAlternateSwitch) > DABMW_SF_MINIMUM_TIME_TO_KEEP_ALTERNATE)             
               {
               // time is over
               vl_res = true;
               }
            else
               {
               vl_res = false;
               }
            break;
		
		default :
			vl_res = false;
			break;
		}

    return vl_res;
}

/* Handling of specific event for service following
* Current EVENT 
* DABMW_SF_EVENT_PI_RECEIVED : PI reception notification
* DABMW_SF_EVENT_DAB_TUNE : TUNE notification
*/

tSInt DABMW_ServiceFollowing_EventHandling(tU8 vI_event)
{

	/* clear the received event */
	DABMW_ServiceFollowing_ExtInt_TaskClearEvent(vI_event);

	switch (vI_event) {
		case DABMW_SF_EVENT_PI_RECEIVED:
			/* PI notification => process PI check, and continue procedure */
			/* Check state in case we should ignore
			*/
			if (DABMW_SF_STATE_BACKGROUND_WAIT_PI == DABMW_serviceFollowingStatus.status)
				{
				DABMW_ServiceFollowing_PI_processing(true);
				}
			else if (DABMW_SF_STATE_INIT == DABMW_serviceFollowingStatus.status)
				{
				/* init case, PI received : init the value */
				DABMW_ServiceFollowingOnTune (DABMW_serviceFollowingStatus.currentAudioUserApp, DABMW_serviceFollowingStatus.currentSystemBand,
      							      DABMW_serviceFollowingStatus.currentFrequency, DABMW_INVALID_SERVICE_ID, 
      							      DABMW_serviceFollowingStatus.currentSid, false, DABMW_serviceFollowingStatus.currentHandle);
				}
			else
				{
				/* just ignore */
				}

			break;

		case DABMW_SF_EVENT_PS_RECEIVED:
			/* PS notification => process PS check, and continue procedure */
			/* Check state in case we should ignore
			*/
			if (DABMW_SF_STATE_BACKGROUND_WAIT_PS == DABMW_serviceFollowingStatus.status)
				{
				DABMW_ServiceFollowing_PS_processing(true);
				}
			else
				{
				/* just ignore */
				}

			break;

		case DABMW_SF_EVENT_DAB_TUNE:
			/* DAB TUNE status (sync...) => process to check Service/quality, and continue procedure*/
			
			/* Check state in case we should ignore
			*/
			if (DABMW_SF_STATE_BACKGROUND_WAIT_DAB == DABMW_serviceFollowingStatus.status)
				{
				DABMW_ServiceFollowing_DAB_processing(true);
				}
			else
				{
				/* just ignore */
				}
			break;
		case DABMW_SF_EVENT_DAB_FIC_READ:
			/* DAB TUNE status (sync...) => process to check Service/quality, and continue procedure*/
			
			/* Check state in case we should ignore
			*/
			if (DABMW_SF_STATE_BACKGROUND_WAIT_FIC == DABMW_serviceFollowingStatus.status)
				
				{
				DABMW_ServiceFollowing_FIC_ReadyProcessing(true);
				}
			else
				{
				/* just ignore */
				}
			break;

		case DABMW_SF_EVENT_AUTO_SEEK:
			/* step 2 above.
			* check if time has elapsed
			* if so process
			* else wait
			*/	
			if (DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK == DABMW_serviceFollowingStatus.status)
				{
				DABMW_ServiceFollowing_Seek_processing(true);
				}
			else
				{
				/* nothing to do */
				}
			break;
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK			
		case DABMW_SF_EVENT_SS_ESTIMATION_RSP:
			if (DABMW_SF_STATE_IDLE_SEAMLESS == DABMW_serviceFollowingStatus.status)
				{
				DABMW_ServiceFollowing_SeamlessEstimationResponseProcessing(true);
				}
			else
				{
				/* nothing to do */
				}
			break;

        case DABMW_SF_EVENT_SS_SWITCHING_RSP:
				DABMW_ServiceFollowing_SeamlessSwitchResponseProcessing(true);
			break;
            
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
		default :
			/* unknown event */
			DABMW_SF_PRINTF (TR_LEVEL_ERRORS, "DABMW SF ERROR: unknown event %d\n", vI_event);
			break;
		}


	return OSAL_OK;
}

// Procedure which enter on a new cell 
//based on the current cell
// It does not handle the alternate : it is assumed there is none
//
tSInt DABMW_ServiceFollowing_SetCurrentCellAsOriginal()
{
//	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
    tBool vl_currentIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.currentAudioUserApp);
    DABMW_mwAppTy	vl_selectedApp = DABMW_serviceFollowingStatus.currentAudioUserApp;
	tSInt vl_tmp_res = OSAL_OK;
    tBool vl_dabServiceSelectionSucceed = false;

    if (DABMW_INVALID_FREQUENCY == DABMW_serviceFollowingStatus.currentFrequency)
     {
 //        vl_originalIsDab = false;
         // This is an error
         //
         return OSAL_ERROR;
     }

	/* Alternate quality */
	if (false == vl_currentIsDab)  
		{
		/* Tune on FM */
 
		/* let's tune as requested now 
		* MAIN FM as reference 
		* improvement could be an AF switch... however, we are in loss of service, so no interest a priori !
		*/
		vl_selectedApp = DABMW_MAIN_AMFM_APP;

        // codex ER  301882 : 
        // SF should not change the source, and request a seamless switching if SS is activated
        //
        // before
        /*		vl_tmp_res = DABMW_TuneFrequency(vl_selectedApp, 
										DABMW_serviceFollowingStatus.currentFrequency, 
										SF_BACKGROUND_TUNE_KEEP_DECODING,
										SF_BACKGROUND_TUNE_INJECTION_SIDE,
										false,
										true);
		* AFTER */
		
        vl_tmp_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency(vl_selectedApp, 
										DABMW_serviceFollowingStatus.currentFrequency);
        
		if (OSAL_ERROR == vl_tmp_res)
			{
			/* it means either that error in tune 
			*/
	
                    
            
             /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                                    DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
                                                        "DABMW_ServiceFollowing (SwitchCurrentCellAsOriginal) : FM TUNE ERROR app = %d, Freq = %d \n",
                                                        vl_selectedApp,
                                                        DABMW_serviceFollowingStatus.currentFrequency
                                            );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
             /* END TMP LOG */

            return OSAL_ERROR;
			}

            // now if seamless is activated, request a switch to correct source 
            // 
            
            
		}
	else 
	    {

		vl_selectedApp = DABMW_serviceFollowingStatus.currentAudioUserApp;

        // It is assumed we are tuned on the right frequency already...
        //
        
		/* Select the DAB service
		* not that it could already be selected... 
		* here should be improve to select is not already selected...
		*/
	
		/* reload EID in case it has change...
		* case is ECC received meanwhile 
		*/
						
		DABMW_serviceFollowingStatus.currentEid= DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(DABMW_serviceFollowingStatus.currentAudioUserApp);

		vl_tmp_res = DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_serviceFollowingStatus.currentAudioUserApp,										   // Use parameter passed
										 DABMW_SERVICE_SELECT_SUBFNCT_SET,				// Sub-function is 5
										 DABMW_serviceFollowingStatus.currentEid,		 // Ensemble  
										 DABMW_serviceFollowingStatus.currentSid);		// Service ID 		  

			
			/* check : is the service selection ok ?  */
			if (OSAL_OK == vl_tmp_res)
				{
				// Service ok  
				vl_dabServiceSelectionSucceed = true;
				
  				// consider we start here the audio 
				DABMW_serviceFollowingStatus.originalLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
				
                
				  /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                                                                 /* END TMP LOG */  

				/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SwitchCurrentCellAsOriginal) : new DAB SERVICE SELECTION success / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.currentAudioUserApp,
                            DABMW_serviceFollowingStatus.currentFrequency,
                            DABMW_serviceFollowingStatus.currentEid,
                            DABMW_serviceFollowingStatus.currentSid);
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
              /* END TMP LOG */  
                                                                  /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */  

				}
			else
				{
                // should not come here but you never know 
                vl_dabServiceSelectionSucceed = false;
                
                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                                                                 /* END TMP LOG */  

				/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)  
               DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (SwitchCurrentCellAsOriginal) : ERROR => DAB SERVICE SELECTION FAILED / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                                    DABMW_serviceFollowingStatus.currentAudioUserApp,
                                    DABMW_serviceFollowingStatus.currentFrequency,
                                    DABMW_serviceFollowingStatus.currentEid,
                                    DABMW_serviceFollowingStatus.currentSid);

#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
              /* END TMP LOG */  
                                                                  /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                                                                 DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */  

								 /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                                /* END TMP LOG */   
				// should not come here but you never know 
                // let's print a failure
                //
			}

            DABMW_serviceFollowingStatus.original_DabServiceSelected = vl_dabServiceSelectionSucceed;
                
	}
  
	/* Init the idle mode value */
	DABMW_ServiceFollowing_EnterIdleMode(vl_selectedApp,
					DABMW_serviceFollowingStatus.currentSystemBand,
					DABMW_serviceFollowingStatus.currentSid,
					DABMW_serviceFollowingStatus.currentEid,
					DABMW_serviceFollowingStatus.currentFrequency);	

    // configure the SS for this new original
#if defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
    DABMW_ServiceFollowing_ConfigureSeamlessForOriginal();
#else
    DABMW_SF_SetCurrentAudioPortUser(vl_selectedApp, true);
#endif // #if (defined CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)    
    
    DABMW_ServiceFollowing_SetOriginalFrequency_SourceInfo();

    return vl_tmp_res;
}

/* procedure to init the basic parameter for idle
* this is when entering in idle : 
* starting being tuned on a freq
*/
tVoid DABMW_ServiceFollowing_EnterIdleMode(DABMW_mwAppTy vI_App, DABMW_SF_systemBandsTy vI_SystemBand, tU32 vI_Sid, tU32 vI_Eid, tU32 vI_Frequency)
{
	tBool vl_NewOriginalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_App) ;
    // Original data source (user tuned one)
    DABMW_mwAppTy vl_current_originalAudioUserApp;
    DABMW_SF_systemBandsTy vl_current_originalSystemBand;
    tU32 vl_current_originalSid;
    tU32 vl_current_originalEid;
    tU32 vl_current_originalFrequency;
    DABMW_SF_QualityTy vl_current_quality;
    tBool vl_current_DabServiceSelected;

	
	DABMW_serviceFollowingStatus.idle_timerStart = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

	/* Set the DAB FIRST information for correlation & delay later on 
	* if a 1st set : ie if original was empty one...
	*/
	if ((true == vl_NewOriginalIsDab) && (DABMW_INVALID_FREQUENCY != vI_Frequency))
		{
		DABMW_serviceFollowingStatus.configurationIsDabFirst = true;
		}

    // Improvement : instead of just switching to new original, the former one should become now an alternate
    //
    // Store info in local pointer
    //
    // the app may be changed : 
    // in case of DAB :  a simple swap, no change of APP
    // in case of FM, original is always on MAIN_AMFM !
    // so set alternate on BG.
    //
    if (DABMW_INVALID_FREQUENCY == DABMW_serviceFollowingStatus.originalFrequency)
    {
        vl_current_originalAudioUserApp = DABMW_NONE_APP;
    }
    else if (DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp))
    {
        vl_current_originalAudioUserApp = DABMW_serviceFollowingStatus.originalAudioUserApp;
    }
    else
    {
        // old original is FM, if new is FM then set to BG
        // else : set to main
        //
        vl_current_originalAudioUserApp = DABMW_serviceFollowingStatus.originalAudioUserApp;
        
        if (false == vl_NewOriginalIsDab)
            {
            // FM  FM case
            vl_current_originalAudioUserApp = DABMW_BACKGROUND_AMFM_APP;
            }
    }

    
    //
    // store the current systemBand, Sid, Eid,..; 
    // only if different  from the new one...
    // 
     
    
    vl_current_originalSystemBand = DABMW_serviceFollowingStatus.originalSystemBand;
    vl_current_originalSid = DABMW_serviceFollowingStatus.originalSid;
    vl_current_originalEid = DABMW_serviceFollowingStatus.originalEid;
    vl_current_originalFrequency = DABMW_serviceFollowingStatus.originalFrequency;
    vl_current_quality = DABMW_serviceFollowingStatus.original_Quality;
    vl_current_DabServiceSelected = DABMW_serviceFollowingStatus.original_DabServiceSelected;

    
	/* Copy current info in original information */

	// add-on etal
	DABMW_serviceFollowingStatus.originalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_App);

	DABMW_serviceFollowingStatus.originalAudioUserApp = vI_App;	// Original data source (user tuned one)
	DABMW_serviceFollowingStatus.originalSystemBand = vI_SystemBand;
	DABMW_serviceFollowingStatus.originalFrequency = vI_Frequency;
	DABMW_serviceFollowingStatus.originalEid = vI_Eid;
	DABMW_serviceFollowingStatus.originalSid = vI_Sid; 

	/* reset the counter since we start on new idle mode & cell */
    // small check : if this is the alternate, we may keep the quality which is known
    if (DABMW_serviceFollowingStatus.originalFrequency == DABMW_serviceFollowingStatus.alternateFrequency)
    {
        DABMW_serviceFollowingStatus.original_Quality = DABMW_serviceFollowingStatus.alternate_Quality;
        DABMW_serviceFollowingStatus.original_Quality_onBackground = DABMW_serviceFollowingStatus.alternate_Quality;
    }
    else
    {
        DABMW_serviceFollowingStatus.original_Quality = DABMW_ServiceFollowing_QualityInit();
        DABMW_serviceFollowingStatus.original_Quality_onBackground = DABMW_ServiceFollowing_QualityInit();
    }
    
	DABMW_serviceFollowingStatus.original_badQualityCounter = 0;
	DABMW_serviceFollowingStatus.original_mediumQualityCounter = 0;

	/* set timer info */
	DABMW_serviceFollowingStatus.lastSwitchTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();	
	DABMW_serviceFollowingStatus.lastServiceRecoveryTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
	// No need to reset the search for AF Time
	// it has been set correctly at init time or end of search
	//DABMW_serviceFollowingStatus.lastSearchForAFTime = OSAL_ClockGetElapsedTime();

	// update the audio time status
	// for orignial should be updated before correctly


    // the original service selection state has been updated already
    //

    // update log evacuation flags.
    DABMW_SF_LOG_INFO_UPDATE_ORIGINAL_CELL;

	if (true == vl_NewOriginalIsDab)
		{
		DABMW_serviceFollowingStatus.measurementPeriodicityReference = DABMW_serviceFollowingData.measurementPeriodicityDAB;
		// update the counter reference
		DABMW_serviceFollowingStatus.counter_NbMeasureLossService = DABMW_serviceFollowingData.counter_NbMeasureLossService_DAB;
		DABMW_serviceFollowingStatus.counter_NbMeasureStartAfSearch = DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_DAB;
		DABMW_serviceFollowingStatus.counter_NbMeasureSwitch = DABMW_serviceFollowingData.counter_NbMeasureSwitch_DAB;
		}
	else
		{
		DABMW_serviceFollowingStatus.measurementPeriodicityReference = DABMW_serviceFollowingData.measurementPeriodicityFM;
		
		// update the counter reference
		DABMW_serviceFollowingStatus.counter_NbMeasureLossService = DABMW_serviceFollowingData.counter_NbMeasureLossService_FM;
		DABMW_serviceFollowingStatus.counter_NbMeasureStartAfSearch = DABMW_serviceFollowingData.counter_NbMeasureStartAfSearch_FM;
		DABMW_serviceFollowingStatus.counter_NbMeasureSwitch = DABMW_serviceFollowingData.counter_NbMeasureSwitch_FM;
		}

	if (false == vl_NewOriginalIsDab)
		{
		// set the RDS to normal mode 
		DABMW_ServiceFollowing_EnableRds(DABMW_serviceFollowingStatus.originalAudioUserApp, false);
		}
	
	/* reset the alternate frequency : nothing monitored anymore */
    // Iinstead of just switching to new original, the former one should become now an alternate
    // BEFORE
    // DABMW_ServiceFollowing_ResetMonitoredAF();
    // AFTER
    
    if ((vl_current_originalFrequency == DABMW_serviceFollowingStatus.originalFrequency )
        && (vl_current_originalSid == DABMW_serviceFollowingStatus.originalSid))
    {
        // new one and original current one are the same
        // do not set that it as alternate 
        // (note this may happen is external tune.. and service selection typically
        // in any case we should make sure the alternate is not the current one !!
        DABMW_ServiceFollowing_ResetMonitoredAF();
    }
    else if (DABMW_INVALID_FREQUENCY != vl_current_originalFrequency)
    {
    	
   
		DABMW_serviceFollowingStatus.alternateLastTimeNoAudio = DABMW_serviceFollowingStatus.originalLastTimeNoAudio;
		
        DABMW_ServiceFollowing_SetMonitoredAF(vl_current_originalAudioUserApp,
                                            vl_current_originalSystemBand,
                                            vl_current_originalSid,
                                            vl_current_originalEid,
                                            vl_current_originalFrequency,
                                            vl_current_quality,
                                            vl_current_DabServiceSelected);
		

	

        DABMW_ServiceFollowing_ResumeMonitoredAF();
    }
    else
    {
        DABMW_ServiceFollowing_ResetMonitoredAF();
    }

	/* notifiy the entering to idle and new freq & bearer to host */
	DABMW_ServiceFollowing_SendChangeNotificationsToHost();

	/* Print out for information */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)
    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
	if (true == vl_NewOriginalIsDab)
		{
		DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Enter Idle Mode): Enter new cell. Bearer : DAB, Freq %d, SID 0x%04x, EID 0x%06x, app = %d, etal_handle = %d\n", 
							vI_Frequency, vI_Sid, vI_Eid, 
							DABMW_serviceFollowingStatus.originalFrequency, DABMW_serviceFollowingStatus.originalHandle);
		}
	else
		{
		DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (Enter Idle Mode): Enter new cell. Bearer : FM, Freq %d, PI 0x%04x, , app = %d, etal_handle = %d\n", 
							vI_Frequency, vI_Sid,
							DABMW_serviceFollowingStatus.originalFrequency, DABMW_serviceFollowingStatus.originalHandle);
		}
    DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   						

    DABMW_ServiceFollowing_SetOriginalFrequency_SourceInfo();

}

/* procedure to init the basic parameter of a target AF
* this is when entering an AF found
*/
tVoid DABMW_ServiceFollowing_SetMonitoredAF(DABMW_mwAppTy vI_App, DABMW_SF_systemBandsTy vI_SystemBand, tU32 vI_Sid, tU32 vI_Eid, tU32 vI_Frequency,DABMW_SF_QualityTy vI_quality, tBool vI_dabServiceIsSelected)
{
	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_App);
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);

    
    // keep few information on prior alternate
    DABMW_mwAppTy old_alternate_app = DABMW_serviceFollowingStatus.alternateApp;
    tSInt vl_tmp_res;
    
    // should we reset the tuner of prior alternate ?  
    // 
    if ((old_alternate_app != vI_App)
        && (DABMW_NONE_APP != old_alternate_app))
    {
        DABMW_ServiceFollowing_ResetTuner(old_alternate_app);
    }
    
	/* Copy current info in original information */

	// Select the APP here : in FM-FM case, APP is main
	if ((true == vl_originalIsDab) && (false == vl_alternateIsDab))
        {
            /* Correction of #302883
                    // BEFORE : 
                    //      an unmute on alternate APP was requested, but this is not correct
                    //      if an alternate FM was existing, and a better one is found, then we should only do a 'AF Switch' on the MAIN_FM to the new frequency
                    //      alternate FM should ALWAYS remain on MAIN_APP whereas, in DAB-FM 1 alternate, better FM 2 alternate found, the app here will be set to bg
                    //
                    // BEFORE 
                        // 
                        // 
                        // Original is DAB
                        // an Alternate FM is found
                        // This should be configured as : MAIN_AMFM, unmuted.
                        //
                        // Cor
                        // 
                        // in case of FM alternate found, this has been done thru seek, with no mute on the tuner bg
                        // purpose was : not disturb on the audio
                        // now that we have one, just unmute
                        //
                        // case is : alternate = main-amfm, ie theory is : original is not FM
                        //
                        // send an unmute command
                        //
                         vl_tmp_res = DABMW_ServiceFollowing_ExtInt_FM_MuteUnmute(DABMW_serviceFollowingStatus.alternateApp, false);
                    *
                    * AFTER 
                    */
                    
            // set the alternate APP as MAIN_AMFM
            vI_App = DABMW_MAIN_AMFM_APP;
		}
	else
		{	
			// keep the one in parameter
		}
			
	// add-on etal
	DABMW_serviceFollowingStatus.alternateHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_App);
	DABMW_serviceFollowingStatus.alternateApp = vI_App; // Original data source (user tuned one)

	DABMW_serviceFollowingStatus.alternateSystemBand = vI_SystemBand;
	DABMW_serviceFollowingStatus.alternateFrequency = vI_Frequency;
	DABMW_serviceFollowingStatus.alternateEid = vI_Eid;
	DABMW_serviceFollowingStatus.alternateSid = vI_Sid; 
    DABMW_serviceFollowingStatus.deltaTimeFromAlternateSwitch = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

	

    // the alternate flags service selected
    // have been updated outside.
    //
    DABMW_serviceFollowingStatus.alternateTuned = true;

	/* reset the counter since we start on new idle mode & cell */

	DABMW_serviceFollowingStatus.alternate_badQualityCounter = 0;
	DABMW_serviceFollowingStatus.alternate_qualityCounter = 0;

    //DABMW_serviceFollowingStatus.alternate_Quality = DABMW_ServiceFollowing_QualityInit();  
    DABMW_serviceFollowingStatus.alternate_Quality = vI_quality;  

    

    // update log evacuation flags.
    DABMW_SF_LOG_INFO_UPDATE_ALTERNATE_CELL;

	/* Set the reference time for switching delay */
	
	if ((true == vl_originalIsDab) && (true == vl_alternateIsDab))
		{
		/* DAB to DAB case */
		DABMW_serviceFollowingStatus.SwitchDelayReference = DABMW_serviceFollowingData.dabToDabDeltaTime;
		}
	else if ((true == vl_originalIsDab) && (false == vl_alternateIsDab))
		{
		/* DAB to FM case */
		DABMW_serviceFollowingStatus.SwitchDelayReference = DABMW_serviceFollowingData.dabToFmDeltaTime;
		}
	else if ((false == vl_originalIsDab) && (true == vl_alternateIsDab))
		{
		/* FM to DAB case */
		DABMW_serviceFollowingStatus.SwitchDelayReference = DABMW_serviceFollowingData.fmToDabDeltaTime;
		}
	else if ((false == vl_originalIsDab) && (false == vl_alternateIsDab))
		{
		/* FM to FM case */
		DABMW_serviceFollowingStatus.SwitchDelayReference = DABMW_serviceFollowingData.fmToFmDeltaTime;
		}
	else
		{
		/* non reachable */
		}

    
    DABMW_serviceFollowingStatus.alternate_DabServiceSelected = vI_dabServiceIsSelected;
    
    // we may select the DabService Already if alternate is DAB, and current is fm...
    // that save time for later...
    // let's do that in 'SetMonitoredAF
    //

    if ((false == vl_originalIsDab) && (true == vl_alternateIsDab) && (false == vI_dabServiceIsSelected))
        {
                      
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK	
			DABMW_SS_EstimationParameter.serviceSelected = true;
#endif // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

			/* reload EID in case it has change...
			* case is ECC received meanwhile 
			*/
						
			DABMW_serviceFollowingStatus.alternateEid = DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(DABMW_serviceFollowingStatus.alternateApp);

			vl_tmp_res = DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_serviceFollowingStatus.alternateApp,										   // Use parameter passed
										 DABMW_SERVICE_SELECT_SUBFNCT_SET,				// Sub-function is 5
										 DABMW_serviceFollowingStatus.alternateEid,		 // Ensemble  
										 DABMW_serviceFollowingStatus.alternateSid);		// Service ID 		  

			
			/* check : is the service selection ok ?  */
			if (OSAL_OK == vl_tmp_res)
				{
				// Service ok  
				DABMW_serviceFollowingStatus.alternate_DabServiceSelected = true;
				
				// consider we start here the audio 
				DABMW_serviceFollowingStatus.alternateLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

				
				  /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
              DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
              DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SetMonitoredAF) : new DAB SERVICE SELECTION success / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.alternateApp,
                            DABMW_serviceFollowingStatus.alternateFrequency,
                            DABMW_serviceFollowingStatus.alternateEid,
                            DABMW_serviceFollowingStatus.alternateSid);
              DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */
			
				}
			else
				{
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS) 
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );

               DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (SetMonitoredAF) : ERROR => DAB SERVICE SELECTION FAILED / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.alternateApp,
                            DABMW_serviceFollowingStatus.alternateFrequency,
                            DABMW_serviceFollowingStatus.alternateEid,
                            DABMW_serviceFollowingStatus.alternateSid);
                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                // should not come here but you never know 
                DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;
                
                }
            }
     // we may need to unmute and switch the FM alternate to correct one
    else if ((true == vl_originalIsDab) && (false == vl_alternateIsDab))
        {
 

            // Tune the MAIN_AMFM to rigth alternate
            // if app is already tuned, that could be only an AF switch... but as we do not listen to audio, we do not care yet.
            
            vl_tmp_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency(DABMW_serviceFollowingStatus.alternateApp, 
										DABMW_serviceFollowingStatus.alternateFrequency); // internal tune

            if (OSAL_ERROR == vl_tmp_res)
            {
            // it means either that error in tune 
      
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
                                    "DABMW_ServiceFollowing (DABMW_ServiceFollowing_SetMonitoredAF) : Tune Alternate FAILING app = %d, Freq = %d, \n",
                                    DABMW_serviceFollowingStatus.alternateApp,
                                    DABMW_serviceFollowingStatus.currentFrequency
                                    );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
            
               return ;
              }
                    

            // unmute the MAIN_AMFM path for later switch....            
            vl_tmp_res = DABMW_ServiceFollowing_ExtInt_FM_MuteUnmute(DABMW_serviceFollowingStatus.alternateApp, false);
            
            
		if (OSAL_ERROR == vl_tmp_res)
			{
			//it means either that error in tune 

#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                                    DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
                                                        "DABMW_ServiceFollowing (DABMW_ServiceFollowing_SetMonitoredAF) : FM UNMUTE ERROR app = %d, Freq = %d, \n",
                                                        DABMW_serviceFollowingStatus.alternateApp,
                                                        DABMW_serviceFollowingStatus.currentFrequency
                                            );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   

            return ;
			}
            
        }
    else if ((false == vl_originalIsDab) && (false == vl_alternateIsDab))
    {
        // both original and target are FM
        // no specific processing needed : 
    }

		/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
						/* END TMP LOG */
			
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			   DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SetMonitoredAF) :  Freq = %d, APP %d, handle = %d\n", 
										DABMW_serviceFollowingStatus.alternateFrequency,
										DABMW_serviceFollowingStatus.alternateApp,
										DABMW_serviceFollowingStatus.alternateHandle
										);			
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
		  /* END TMP LOG */
		
		
				   /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
						/* END TMP LOG */
	


    DABMW_ServiceFollowing_SetAlternateFrequency_SourceInfo();

    return;
    
}

/* procedure to reset the monitored AF information
*/
tVoid DABMW_ServiceFollowing_ResetMonitoredAF()
{


	/* reset the alternate app */
	/* free the alternate tuner ... if not the current active one !!
	*/
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK		
	DABMW_SS_EstimationParameter.serviceSelected = false;
#endif // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

	DABMW_ServiceFollowing_ResetTuner(DABMW_serviceFollowingStatus.alternateApp);
	
	/* reset the alternate frequency : nothing monitored anymore */
	DABMW_serviceFollowingStatus.alternateApp = DABMW_NONE_APP;
	DABMW_serviceFollowingStatus.alternateSystemBand = DABMW_BAND_NONE;
	DABMW_serviceFollowingStatus.alternateFrequency = DABMW_INVALID_FREQUENCY;
	DABMW_serviceFollowingStatus.alternateEid = DABMW_INVALID_EID;
	DABMW_serviceFollowingStatus.alternateSid = DABMW_INVALID_SERVICE_ID; 
    DABMW_serviceFollowingStatus.alternateTuned = false;
    DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;

	DABMW_serviceFollowingStatus.alternate_badQualityCounter = 0;
	DABMW_serviceFollowingStatus.alternate_qualityCounter = 0;    
    DABMW_serviceFollowingStatus.alternate_Quality = DABMW_ServiceFollowing_QualityInit();  


	/* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
	DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
	DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (ResetMonitoredAF)\n");
	DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
					/* END TMP LOG */


    // If the AF is DAB and audio is selected... 
    // should not we remove the service ? 
    // or it is implicit on tune ?

	// update log evacuation flags.
	DABMW_SF_LOG_INFO_UPDATE_ALTERNATE_CELL;



}

/* procedure to reset the monitored AF information
*/
tSInt DABMW_ServiceFollowing_ResumeMonitoredAF()
{
	tSInt vl_res = OSAL_OK;


	/* Resume FM by tuning back to freq
	* Resume DAB : basically works only if alternateApp remained configured...
	* not yet supported if alternateAPP was used for scan...
	* And, only if alternate not empty */

	if (DABMW_NONE_APP == DABMW_serviceFollowingStatus.alternateApp)
		{
		vl_res = OSAL_ERROR;
		}
	else if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp))
		{
		/* alternate is DAB */
		
        // We may need to re-tune.... if the app of the alternate has been used for bg scan DAB...(DAB-DAB case)
        //
        if (false == DABMW_serviceFollowingStatus.alternateTuned)
            {
            vl_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency (DABMW_serviceFollowingStatus.alternateApp, 
                                          DABMW_serviceFollowingStatus.alternateFrequency);

            DABMW_serviceFollowingStatus.alternateTuned = true;
            DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;
            }

		}
	else 
		{
		/* alternate is FM */

        // Change : we do not need to tune the alternate, because we will use AF check to measure it.
        // so Tune not usefull 
        // in DAB - FM case
        // that will lead to alternate being well tuned, on MAIN_AMFM, because the scan has been done on othe FM path (in theory...)
        // but this is true only if several FM path exists...
        // in FM - FM case 
        // that will lead to alternate not tuned... because Alternate is managed on main...
        //
        // can be better to tune it ? 
        //
        
		/* DABMW_TuneFrequency(DABMW_serviceFollowingStatus.alternateApp, 
						DABMW_serviceFollowingStatus.alternateFrequency, // frequency 
						0, // keep decoding
						1, // injection side
						true, // no output switch
						true); // internal tune

		*/


		// Update alternate may have been suspended. Retune it.
		if (false == DABMW_serviceFollowingStatus.alternateTuned)
		{
			vl_res = DABMW_ServiceFollowing_ExtInt_TuneFrequency (DABMW_serviceFollowingStatus.alternateApp, 
                                          DABMW_serviceFollowingStatus.alternateFrequency);

	        DABMW_serviceFollowingStatus.alternateTuned = true;
		}


        DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;
		vl_res = OSAL_OK;				
		
		}

	return vl_res;
}

/* procedure to reset the monitored AF information
*/
tSInt DABMW_ServiceFollowing_SuspendMonitoredAF()
{
	tSInt vl_res = OSAL_OK;


	
	//Suspend the monitored AF if needed
	//for now, only in case of tuner restrcitions...
#ifdef CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM
	// all is fine, do nothing
	return vl_res;
#else

	// Tuner are shared...
	// suspend the AF in following case : 
	// original is FM, alternate is FM ==> the alternate can be measured thru AF check on original tuner, and a tuner is freed for bg activity
	// original is FM, alternate is DAB ==> do nothing for now, because if we stop the DAB it will make it complicated for a possible switch if needed.
	// original is DAB, alternate is FM ==> suspend alternate, bg scan will be suspended.
	// original is DAB, alternate is DAB  ==> do nothing for now, because if we stop the DAB it will make it complicated for a possible switch if needed.
		
	if (DABMW_NONE_APP == DABMW_serviceFollowingStatus.alternateApp)
		{
		vl_res = OSAL_ERROR;
		}
	else if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp))
		{
		/* alternate is DAB */
		
		// original is DAB, alternate is DAB  ==> do nothing for now, because if we stop the DAB it will make it complicated for a possible switch if needed.
		// original is FM, alternate is DAB ==> do nothing for now, because if we stop the DAB it will make it complicated for a possible switch if needed.

        // do nothing for now
        vl_res = OSAL_OK;	
		}
	else // means is FM case (so either FM - FM , either DAB - FM) we suspend it
		{
		/* alternate is FM */
		//reset the tuner to free it
		

		DABMW_ServiceFollowing_ResetTuner(DABMW_serviceFollowingStatus.alternateApp);


        DABMW_serviceFollowingStatus.alternateTuned = false;
        DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;
		vl_res = OSAL_OK;				
		
		}

	return vl_res;
#endif
}


/* procedure to sent the auto notification on the API to the Host
*/
tU8 DABMW_ServiceFollowing_BuildPayloadNotifications (tPU8 pO_payloadBuffer, tU16 vI_bufferLen)
{

#define DABMW_SF_SF_NOTIFICATION_PAYLOAD_SIZE 	12

	tU8 vl_index = 0;

	/* Service following status notification
	* either for NOTIFICATION either of GetStatus command */

	/* filled by API  DECODER : paylaod and size
	* 
	Header 0 = 0xF2 
	Header 1 = RFU = 0
	Header 2 = Cmd Num = 0x10

	*	PAYLOAD LENGTH	12	Fixed size
	*
	* PAYLOAD SETTING Notes
	*
	BYTE 0	APPLICATION 	The applications that can answer are:
	-  1: MAIN DAB APP
	-  2: SECONDARY DAB APP
	-  3: MAIN AMFM APP
	-  4: BACKGROUND AMFM APP
	BYTES 1 - 4 TUNED  FREQUENCY	Tuned frequency
	BYTES 5 - 7 UEId	Unique Ensemble Identifier
	BYTES 8 - 11	SERVICE IDENTIFIER	Service identifier
	*/

	/* Header */
	/* vl_notificationBuffer[vl_index] = 0xF2;
	vl_index++;
	vl_notificationBuffer[vl_index] = 0x00;
	vl_index++;
	vl_notificationBuffer[vl_index] = DABMW_NOTIFICATION_IS_SF_STATUS;
	vl_index++;

	//payload len 
	vl_notificationBuffer[vl_index] = DABMW_SF_SF_NOTIFICATION_LEN_SIZE;
	vl_index++;
	*/

    if (vI_bufferLen < DABMW_SF_SF_NOTIFICATION_PAYLOAD_SIZE)
        {
        /* buffer not correct len */
        return 0;
        }
    
	/* BYTE 0 : application */
	pO_payloadBuffer[vl_index] = DABMW_serviceFollowingStatus.originalAudioUserApp;
	vl_index++;

	/*	BYTES 1 - 4 TUNED  FREQUENCY	Tuned frequency*/
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalFrequency >> 24) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalFrequency >> 16) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalFrequency >> 8) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalFrequency >> 0) & (tU8)0xFF;
	vl_index++;

	/* BYTES 5 - 7 UEId	Unique Ensemble Identifier */
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalEid>> 16) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalEid >> 8) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalEid >> 0) & (tU8)0xFF;
	vl_index++;

	/* BYTES 8 - 11	SERVICE IDENTIFIER	Service identifier */
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalSid>> 24) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalSid >> 16) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalSid >> 8) & (tU8)0xFF;
	vl_index++;
	pO_payloadBuffer[vl_index]  = (DABMW_serviceFollowingStatus.originalSid >> 0) & (tU8)0xFF;
	vl_index++;

    return vl_index;
    
}


/*
*  Procedure to put back a tuner to Idle 
*/
tSInt DABMW_ServiceFollowing_ResetTuner(DABMW_mwAppTy vI_App)
{
	tSInt vl_tmp_res = OSAL_OK;
  //  tU32 vl_tmp_freq;

#define DABMW_SERVICE_FOLLOWING_FREQUENCY_FOR_RESETTING_TUNER_1 174928
#define DABMW_SERVICE_FOLLOWING_FREQUENCY_FOR_RESETTING_TUNER_2 239200

	/* reset the alternate app */
	/* free the alternate tuner ... if not the current active one !!
	*/

	if ((vI_App != DABMW_serviceFollowingStatus.originalAudioUserApp)
		&&
		(vI_App != DABMW_NONE_APP)
		)
		{
		if (DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(vI_App))
			{
			DABMW_ServiceFollowing_ExtInt_TuneFrequency(vI_App, DABMW_SF_RESET_FREQUENCY); 
			}
		else
			{
			// EPR TMP attempt : codec #287756
			// Frequency to 0 is raising pb
			// add a 1st tune on base freq
			//

            // add a tune on an other freq for the moment...
            //
            
            /*
		            if (DABMW_SERVICE_FOLLOWING_FREQUENCY_FOR_RESETTING_TUNER_1 == DABMW_ServiceFollowing_ExtInt_GetFrequencyFromApp(vI_App))
		            {
		                vl_tmp_freq = DABMW_SERVICE_FOLLOWING_FREQUENCY_FOR_RESETTING_TUNER_2;
		            }
		            else
		            {
		                vl_tmp_freq =   DABMW_SERVICE_FOLLOWING_FREQUENCY_FOR_RESETTING_TUNER_1;
		                    
		            }
		                
					// tune to 0 anyhow.
					DABMW_ServiceFollowing_ExtInt_TuneFrequency(vI_App, DABMW_SF_RESET_FREQUENCY);

		            // add a tune on an other freq for the moment...
		            //
		           			// tune to 0 anyhow.
					DABMW_ServiceFollowing_ExtInt_TuneFrequency(vI_App, vl_tmp_freq); 
				    }
				    */
				// tune to 0 anyhow.
				DABMW_ServiceFollowing_ExtInt_TuneFrequency(vI_App, DABMW_SF_RESET_FREQUENCY);
			}

		}
	else
		{
		vl_tmp_res = OSAL_ERROR;
		}


	return vl_tmp_res;
	
}

tPChar DABMW_ServiceFollowing_StateName(DABMW_serviceFollowingStateMachineTy vI_state)
{
#define DABMW_SF_MACRO_STATE_NAME(x) x##_NAME

   tPChar vl_stateName;
    
    switch(vI_state)
        {
        case DABMW_SF_STATE_IDLE: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_IDLE); 
            break;
        case DABMW_SF_STATE_INIT: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_INIT); 
            break;
        case DABMW_SF_STATE_INITIAL_SERVICE_SELECTION:
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_INITIAL_SERVICE_SELECTION); 
            break;
        case DABMW_SF_STATE_BACKGROUND_CHECK: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_BACKGROUND_CHECK); 
            break;
        case DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK); 
            break;
        case DABMW_SF_STATE_BACKGROUND_WAIT_PI: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_BACKGROUND_WAIT_PI); 
            break;
		case DABMW_SF_STATE_BACKGROUND_WAIT_PS: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_BACKGROUND_WAIT_PS); 
            break;
        case DABMW_SF_STATE_BACKGROUND_WAIT_DAB: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_BACKGROUND_WAIT_DAB); 
            break;
        case DABMW_SF_STATE_BACKGROUND_WAIT_FIC: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_BACKGROUND_WAIT_FIC); 
            break;
        case DABMW_SF_STATE_BACKGROUND_SCAN: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_BACKGROUND_SCAN); 
            break;
        case DABMW_SF_STATE_AF_CHECK: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_AF_CHECK); 
            break;
        case DABMW_SF_STATE_SERVICE_RECOVERY:
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_SERVICE_RECOVERY); 
            break;
        case DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF); 
            break;
        case DABMW_SF_STATE_IDLE_AF_MONITORING:
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_IDLE_AF_MONITORING); 
            break;
        case DABMW_SF_STATE_IDLE_SEAMLESS: 
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_IDLE_SEAMLESS); 
            break;
        default:
            vl_stateName = DABMW_SF_MACRO_STATE_NAME(DABMW_SF_STATE_DEFAULT);
            break;
        }


    return vl_stateName;
  
}

/* Procedure to handle the state change */
tVoid DABMW_ServiceFollowing_ChangeState(DABMW_serviceFollowingStateMachineTy vI_newState)
{

 #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)       
 #ifdef  CONFIG_ENABLE_CLASS_APP_DABMW_SF
    static DABMW_serviceFollowingStateMachineTy vl_last_loggedState = DABMW_SF_STATE_INIT;
              
    tPChar vl_inputStatusName;
    tPChar vl_outputStatusName;  
	
        /* ADD State Information log */
        if ((vI_newState != vl_last_loggedState)
            /* do not log the temporary state */
            &&
            (vI_newState != DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK)
            &&
            (vI_newState != DABMW_SF_STATE_BACKGROUND_WAIT_PI)
            &&
            (vI_newState != DABMW_SF_STATE_BACKGROUND_WAIT_PS)
            &&
            (vI_newState != DABMW_SF_STATE_BACKGROUND_WAIT_DAB)
            )
            {     
            vl_last_loggedState = vI_newState;

    
            vl_inputStatusName = DABMW_ServiceFollowing_StateName(DABMW_serviceFollowingStatus.status);
            vl_outputStatusName = DABMW_ServiceFollowing_StateName(vI_newState);

            // set log evacuation flag 
            DABMW_SF_LOG_INFO_UPDATE_STATE_CHANGE;

          DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
          DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (ChangeState): state transition from %02d (%s) to new state %02d (%s), \n", DABMW_serviceFollowingStatus.status, vl_inputStatusName, vI_newState, vl_outputStatusName );
          DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
            }
#endif // #ifdef  CONFIG_ENABLE_CLASS_APP_DABMW_SF
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   			

        DABMW_serviceFollowingStatus.status = vI_newState;
    return;
}



#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

/* Procedure which cares of the processing following the Estimation Response Reception
*/
tSInt DABMW_ServiceFollowing_SeamlessEstimationResponseProcessing(tBool vI_ResponseIsReceived)
{
	tSInt vl_res = OSAL_ERROR;
	DABMW_SF_EvaluationActionResultTy vl_NeededAction;
	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
//	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);


	/* save time information */
	DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
	
	if (DABMW_SF_STATE_IDLE_SEAMLESS != DABMW_serviceFollowingStatus.status)
		{
		/* this is not correct : an error */
		return OSAL_ERROR;
		}


	// update log evacuation flags.
	DABMW_SF_LOG_INFO_UPDATE_SEAMLESS;
		
	if (false == vI_ResponseIsReceived)
		{
		/* we have not yet received the response...
		* we have wait enough
		*/
		/* send stop command */
		DABMW_ServiceFollowing_SeamlessEstimationStop();

        // back to idle


        DABMW_ServiceFollowing_ResetMonitoredAF();

        DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
		DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);		
		}
	else
		{
		if (true == vl_alternateIsDab)
			{
			/* response received : store the result => DAB is alternate & FM is original */
			DABMW_ServiceFollowing_SSDatabaseStore(DABMW_serviceFollowingStatus.alternateFrequency, 
												DABMW_serviceFollowingStatus.alternateEid,
												DABMW_serviceFollowingStatus.alternateSid,
												DABMW_serviceFollowingStatus.originalSid);
			}
		else
			{
			/* response received : store the result => DAB is original & FM is alternate */ 
			DABMW_ServiceFollowing_SSDatabaseStore(DABMW_serviceFollowingStatus.originalFrequency, 
												DABMW_serviceFollowingStatus.originalEid,
												DABMW_serviceFollowingStatus.originalSid,
												DABMW_serviceFollowingStatus.alternateSid);
			}
		}

	/* Evaluate Original */
	vl_NeededAction = DABMW_ServiceFollowing_EvaluateOriginalCellSuitability();
	
	if (DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY == vl_NeededAction)
		{
		/* Should not happen neither be process here : this is handle already in the normal main loop in the state
		*/
		}								
	else if ((DABMW_SF_EVALUATE_RES_NO_ACTION_GOOD_CELL == vl_NeededAction)
		|| (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_FM_ONLY == vl_NeededAction))
		{	
        /* EPR CHANGE */
        /* keep the alternate that we have 
        
		// back to idle 
		// Note : we have a valid alternate cell : shouldn't we keep it and remain in monitoring...
		//  the case  here is : original DAB and good....
		// because original FM we should have the result evaluate DAB only
		//
		// reset the alternate frequency : nothing monitored anymore 
		DABMW_ServiceFollowing_ResetMonitoredAF();

              DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
              DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
               */

        /* it means it is not time to switch : 
		  * back to monitoring..
		  */	
		DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
        DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);        
        /* END EPR CHANGE */
		}
	else
		{
		/* continue AF activity : 
		* get measurement & check suitability & action
		*/
		 vl_NeededAction = DABMW_ServiceFollowing_EvaluateAFCellSuitability();
	
		/* return 
		* = NO_ACTION means nothing to do continue the monitoring 
		* = EVALUATE NIEGHBOOR = means current AF not ok, look for new one
		* = EVALUATE_NEIGHBOOR_DAB_ONLY = means it is time to look for a  DAB AF.
		* ... however only if correct period : so switch to state 'IDLE' which will decide
		* = CHANGE_CELL => switch is needed without seamless / direct switch
		* = CHANGE_CELL_SEAMLESS => switch needed with seamless estimation...
		*/
	
		if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR == vl_NeededAction)
			{
			
			/* Should not happen neither be process here : this is handle already in the normal main loop in the state
			*/
			}
		else if (DABMW_SF_EVALUATE_CHANGE_CELL == vl_NeededAction)
			{
			/* should not happen : error case
			* since we left estimation... 
			*/
			/* do the cell switch */
			DABMW_ServiceFollowing_SwitchToAF();

             // Change for Improvement : 
             // Instead of going back to idle, just do an 'alternate switching' so continue being in IDLE_AF_MONITORING
             // BEFORE
    		 // now back to idle on that new setting 
             // DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
             // DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
             // DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);
    		 // AFTER    
			
            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);   
			}
		else if (DABMW_SF_EVALUATE_CHANGE_CELL_SEAMLESS == vl_NeededAction)
			{
			/* Change is confirmed */

			if (true == vI_ResponseIsReceived)
				{
				/* all is right */
				DABMW_ServiceFollowing_SwitchToAF();
				}
			else 
				{
				/* do a normal switch  */
				DABMW_ServiceFollowing_SwitchToAF();
				}
				
			 // Change for Improvement : 
             // Instead of going back to idle, just do an 'alternate switching' so continue being in IDLE_AF_MONITORING
             // BEFORE
    		 //  DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
             // DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE);	
    		 // AFTER    

            DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);	
			}
		else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY == vl_NeededAction)
			{
			/* it means it is not time to switch : 
			* back to monitoring..
			*/
			
			DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
			}
        else if (DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR == vl_NeededAction)
            {
             /* it means it is not time to switch : 
			* back to monitoring..
			*/	
			 
			DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);   
            }
		else 
			{
			/* it means it is not time to switch : 
			* back to monitoring..
			*/
			
			DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(DABMW_serviceFollowingStatus.status);
            DABMW_SERVICE_FOLLOWING_CHANGE_STATE(DABMW_SF_STATE_IDLE_AF_MONITORING);
			}
		}

	
	/* And now : should I switch ?
	* current implementation : handle everything here 
	*
	*/


	return vl_res;

}


/* Procedure which cares of the processing following the Estimation Response Reception
*/
tSInt DABMW_ServiceFollowing_SeamlessSwitchResponseProcessing(tBool vI_ResponseIsReceived)
{
    tSInt vl_res = OSAL_OK;
//	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
    DABMW_mwAppTy   vl_selectedApp = DABMW_NONE_APP;

    // only processing for now is to configure the audio port correctly if needed.
    // 
    // if we are not in  Early FM, configure the port
    // in general it should already be ok if Dabfirst was already the case....

    if (true == vl_originalIsDab)
    {
         vl_selectedApp = DABMW_serviceFollowingStatus.originalAudioUserApp;
    }
    else
    {
        //
        // note that in theory in that case the audio port should already be correct
        //

        if (SD_SEAMLESS_SWITCHING_SWITCH_TO_EARLY_FM != DABMW_ServiceFollowing_RetrieveLastSwitchSystemToSwitch())
        {

            vl_selectedApp = DABMW_ServiceFollowing_ExtInt_GetCurrentAudioPortUser();
            /*
                        if (DABMW_NONE_APP != DABMW_serviceFollowingStatus.alternateApp)
                        {
                            vl_selectedApp = DABMW_serviceFollowingStatus.alternateApp;
                        }
                        else
                        {   
                            vl_selectedApp = DABMW_MAIN_AUDIO_DAB_APP;
                        }*/           
        }
        else
        {
            vl_selectedApp = DABMW_serviceFollowingStatus.originalAudioUserApp;
        }
    }

    if (vl_selectedApp != DABMW_ServiceFollowing_ExtInt_GetCurrentAudioPortUser())
        {
         DABMW_SF_SetCurrentAudioPortUser(vl_selectedApp, true);
        }
    
    return vl_res;
}
    

#endif // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

/* EPR CHANGE */
/* ADD A PROCEDURE TO PRINT BAND NAME */

tPChar DABMW_ServiceFollowing_GetBandName (tU32 vI_band)
{
    tPChar vl_bandName;
    
#define DABMW_SF_MACRO_BAND_NAME(x) x##_NAME

    switch (vI_band)
    {
        case DABMW_BAND_NONE:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_NONE);
            break;
        case DABMW_BAND_FM_EU:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_FM_EU);
            break;
        case DABMW_BAND_FM_US:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_FM_US);
            break;
        case DABMW_BAND_FM_JAPAN:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_FM_JAPAN);
            break;
        case DABMW_BAND_FM_EAST_EU:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_FM_EAST_EU);
            break;
        case DABMW_BAND_FM_WEATHER_US:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_FM_WEATHER_US);
            break;
        case DABMW_BAND_AM_MW_EU:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_MW_EU);
            break;
        case DABMW_BAND_AM_MW_US:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_MW_US);
            break;
        case DABMW_BAND_AM_MW_JAPAN:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_MW_JAPAN);
            break;
        case DABMW_BAND_AM_MW_EAST_EU:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_MW_EAST_EU);
            break;
        case DABMW_BAND_DAB_III:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_DAB_III);
            break;
        case DABMW_BAND_CHINA_DAB_III:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_CHINA_DAB_III);
            break;
        case DABMW_BAND_KOREA_DAB_III: 
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_KOREA_DAB_III);
            break;
        case DABMW_BAND_DAB_L:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_DAB_L);
            break;
        case DABMW_BAND_CANADA_DAB_L:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_CANADA_DAB_L);
            break;
        case DABMW_BAND_AM_LW                   :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_LW);
            break;
        case DABMW_BAND_AM_SW1                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW1);
            break;
        case DABMW_BAND_AM_SW2                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW2);
            break;
        case DABMW_BAND_AM_SW3                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW3);
            break;
        case DABMW_BAND_AM_SW4                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW4);
            break;
        case DABMW_BAND_AM_SW5                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW5);
            break;
        case DABMW_BAND_AM_SW6                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW6);
            break;
        case DABMW_BAND_AM_SW7                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW7);
            break;
        case DABMW_BAND_AM_SW8                  :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW8);
            break;
        case DABMW_BAND_AM_SW9                  : 
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW9);
            break;
        case DABMW_BAND_AM_SW10                 :  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_AM_SW10);
            break;
        case DABMW_BAND_DRM30:  
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_DRM30);
            break;
        default:
            vl_bandName = DABMW_SF_MACRO_BAND_NAME(DABMW_BAND_NONE);
            break;
    }

    return vl_bandName;
    
}

/* ADD A PROCEDURE TO KNOW IF DAB BAND */

/* procedure to sent the auto notification on the API to the Host
*/
tU16 DABMW_ServiceFollowing_BuildLogPayloadMsg (tPU8 pO_payloadBuffer, tU16 vI_bufferLen, tBool vI_isAuto, tU8 vI_version)
{
#define DABMW_SF_LOG_INFO_STATUS_MSG_PAYLOAD_SIZE 	500

    tU16 vl_index = 0;
    DABMW_serviceFollowing_LogInfoMsgTy vl_logInfo;
    SF_tMSecond vl_systemTime;
    tBool evacuation_flag = false;
    tU16 vl_len;
		
    // Get the current system time
    vl_systemTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();


    if (vI_bufferLen < DABMW_SF_LOG_INFO_STATUS_MSG_PAYLOAD_SIZE)
    {
    /* buffer not correct len */
    return 0;
    }

    // check condition for evacuation
    // either normal request
    // either change is status flag
    // either time to evacuate

    if (true == vI_isAuto)
    {
        // service following not enabled : return
        if ( false == DABMW_serviceFollowingData.enableServiceFollowing)
            {
            return 0;
            }

        // check update which requires log evacuation not conditionnal
        if ((true == DABMW_serviceFollowing_LogInfo.stateChange) ||
            (true == DABMW_serviceFollowing_LogInfo.originalCellUpdate)||
            (true == DABMW_serviceFollowing_LogInfo.alternateCellUpdate)||
            (true == DABMW_serviceFollowing_LogInfo.backgroundSearchCompleted) ||
            (true == DABMW_serviceFollowing_LogInfo.measurementUpdate)
           )
            {
                evacuation_flag = true;
            }

        // check update which requires log evacuation periodic                
        if (true == DABMW_ServiceFollowing_CheckTimeAction(DABMW_SF_TimeCheck_LogInfoStatusEvacuation))
                {
                evacuation_flag = true;
                }

        if (false == evacuation_flag)
            {
            return 0;
            }
    }
    else
    {
        // normal request : no change.
    }

    // init the structure
    DABMW_ServiceFollowing_ExtInt_MemorySet(&vl_logInfo,  0x00, sizeof(DABMW_serviceFollowing_LogInfoMsgTy));
    DABMW_ServiceFollowing_ExtInt_MemorySet(pO_payloadBuffer, 0x00,  vI_bufferLen);

    // fill the structure

    // 1st log Info struct

    //
    // log version
    //
    if (DABMW_SF_LOG_VERSION_REQUESTED_V1 == vI_version)
    {
        // 1st version of the sent out is requested : kept for compatibility
        //
        vl_logInfo.sf_log_version = DABMW_SF_LOG_VERSION_V1;
    }
    else if (DABMW_SF_LOG_VERSION_REQUESTED_V2 ==  vI_version)
    {          
        // which log do we evacuate ? 
        if (true == DABMW_serviceFollowing_LogInfo.backgroundSearchCompleted)
        { 
            // it will be BG only
            vl_logInfo.sf_log_version = DABMW_SF_MASK_LOG_VERSION_BG | DABMW_SF_LOG_VERSION_V2;
        }
        else
        {
            vl_logInfo.sf_log_version = DABMW_SF_LOG_VERSION_V2;
        }
    }
    else if (DABMW_SF_LOG_VERSION_REQUESTED_V3 ==  vI_version)
    {          
        // which log do we evacuate ? 
        if (true == DABMW_serviceFollowing_LogInfo.backgroundSearchCompleted)
        { 
            // it will be BG only
            vl_logInfo.sf_log_version = DABMW_SF_MASK_LOG_VERSION_BG | DABMW_SF_LOG_VERSION_V3;
        }
        else
        {
            vl_logInfo.sf_log_version = DABMW_SF_LOG_VERSION_V3;
        }
    }
    else 
    {
        // which log do we evacuate ? 
        if (true == DABMW_serviceFollowing_LogInfo.backgroundSearchCompleted)
        { 
            // it will be BG only
            vl_logInfo.sf_log_version = DABMW_SF_LOG_VERSION_BG | DABMW_SF_LOG_VERSION;
        }
        else
        {
            vl_logInfo.sf_log_version = DABMW_SF_LOG_VERSION;
        }
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    // evacuation flags info
    //////////////////////////////////////////////////////////////////////////////////////////////////

    vl_logInfo.changeFieldInfo.field.stateChange = DABMW_serviceFollowing_LogInfo.stateChange;
    vl_logInfo.changeFieldInfo.field.measurementUpdate = DABMW_serviceFollowing_LogInfo.measurementUpdate;
    vl_logInfo.changeFieldInfo.field.originalCellUpdate = DABMW_serviceFollowing_LogInfo.originalCellUpdate;
    vl_logInfo.changeFieldInfo.field.alternateCellUpdate = DABMW_serviceFollowing_LogInfo.alternateCellUpdate;
    vl_logInfo.changeFieldInfo.field.backgroundSearchCompleted = DABMW_serviceFollowing_LogInfo.backgroundSearchCompleted;
    vl_logInfo.changeFieldInfo.field.seamlessUpdate = DABMW_serviceFollowing_LogInfo.seamlessUpdate;
    vl_logInfo.changeFieldInfo.field.databaseUpdate = DABMW_serviceFollowing_LogInfo.databaseUpdate;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Generic INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////
    vl_logInfo.searchSid_Pi =  DABMW_serviceFollowingData.initial_searchedPI;
    vl_logInfo.state = DABMW_serviceFollowingStatus.status;

    // timing data for system switch
    //
    vl_logInfo.deltaTimeSinceLastSwitch = (vl_systemTime - DABMW_serviceFollowingStatus.lastSwitchTime);
    vl_logInfo.deltaTimeSinceLastServiceRecoverySearch = (vl_systemTime - DABMW_serviceFollowingStatus.lastServiceRecoveryTime);
    vl_logInfo.idle_timerStart = (vl_systemTime - DABMW_serviceFollowingStatus.idle_timerStart);
    vl_logInfo.deltaTimeSinceLastSearchForAF = (vl_systemTime - DABMW_serviceFollowingStatus.lastSearchForAFTime);
    vl_logInfo.deltaTimeSinceLastFullScan = (vl_systemTime - DABMW_serviceFollowingStatus.lastFullScanForAFTime);
    vl_logInfo.deltaTimeSinceLastLandscaspeBuilding = (vl_systemTime - DABMW_serviceFollowingStatus.lastLandscaspeBuildingTime);
    vl_logInfo.deltaTimeSinceLastLandscapeDelayEstimation = (vl_systemTime - DABMW_serviceFollowingStatus.lastFMLandscapeDelayEstimationTime);
    vl_logInfo.deltaTimeFromAlternateSwitch = (vl_systemTime - DABMW_serviceFollowingStatus.deltaTimeFromAlternateSwitch);


    //////////////////////////////////////////////////////////////////////////////////////////////////
    // CURRENT CELL INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////
    vl_logInfo.originalCell.app = DABMW_serviceFollowingStatus.originalAudioUserApp;
    vl_logInfo.originalCell.Frequency = DABMW_serviceFollowingStatus.originalFrequency;
    vl_logInfo.originalCell.Sid = DABMW_serviceFollowingStatus.originalSid;
    vl_logInfo.originalCell.Eid = DABMW_serviceFollowingStatus.originalEid;
    vl_logInfo.originalCell.Quality = DABMW_serviceFollowingStatus.original_Quality;
    // handle the no audio
    if ((DABMW_SF_QUALITY_IS_NO_AUDIO == DABMW_serviceFollowingStatus.original_Quality.qualityStatus)
		|| (DABMW_SF_QUALITY_IS_NO_SYNC == DABMW_serviceFollowingStatus.original_Quality.qualityStatus))
    {
        DABMW_serviceFollowingStatus.original_Quality.qualityStatus = DABMW_SF_QUALITY_IS_POOR;
    }
    vl_logInfo.originalCell.source.value = DABMW_serviceFollowingStatus.originalSource.value;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Alternate CELL INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////
    vl_logInfo.alternateCell.app = DABMW_serviceFollowingStatus.alternateApp;
    vl_logInfo.alternateCell.Frequency = DABMW_serviceFollowingStatus.alternateFrequency;
    vl_logInfo.alternateCell.Sid = DABMW_serviceFollowingStatus.alternateSid;
    vl_logInfo.alternateCell.Eid = DABMW_serviceFollowingStatus.alternateEid;
    vl_logInfo.alternateCell.Quality = DABMW_serviceFollowingStatus.alternate_Quality;
        // handle the no audio
	if ((DABMW_SF_QUALITY_IS_NO_AUDIO == DABMW_serviceFollowingStatus.alternate_Quality.qualityStatus)
		|| (DABMW_SF_QUALITY_IS_NO_SYNC == DABMW_serviceFollowingStatus.alternate_Quality.qualityStatus))		
    {
        DABMW_serviceFollowingStatus.original_Quality.qualityStatus = DABMW_SF_QUALITY_IS_POOR;
    }
    vl_logInfo.alternateCell.source.value = DABMW_serviceFollowingStatus.alternateSource.value;


    //////////////////////////////////////////////////////////////////////////////////////////////////
    // SS information for Alternate INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////  
	// set the SS status flag.
	// if SS not activated or not alternate, or  alternate & orignial are not DAB - FM : SS not applicable.
	vl_logInfo.seamlessInfo = DABMW_ServiceFollowing_SetSeamlessLogInfoStatus();
	
    //////////////////////////////////////////////////////////////////////////////////////////////////
    // BG PROCESSING INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////  
    vl_logInfo.bgProcessingInformation.dab_TypeOfScan = DABMW_serviceFollowingStatus.ScanTypeDab;
    vl_logInfo.bgProcessingInformation.fm_TypeOfScan = DABMW_serviceFollowingStatus.ScanTypeFm;

    // last bg search result
    vl_logInfo.bgProcessingInformation.last_searchSuccessfull = DABMW_ServiceFollowing_LastBackgroundSearchResult(vl_logInfo.searchSid_Pi);
    vl_logInfo.bgProcessingInformation.last_searchProcInfo = DABMW_ServiceFollowing_BackgroundLastSearchLogInfo();

    vl_logInfo.bgProcessingInformation.DABMW_SF_freqCheckList = DABMW_SF_freqCheckList;


    // 2nd : the payload

    // *************** //
    // now build the structure with values buffer 
    // **************  //

    
    //////////////////////////////////////////////////////////////////////////////////////////////////
    // log version
    //////////////////////////////////////////////////////////////////////////////////////////////////
    PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.sf_log_version, pO_payloadBuffer, vl_index);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // evacuation flags info
    //////////////////////////////////////////////////////////////////////////////////////////////////
    PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.changeFieldInfo.value, pO_payloadBuffer, vl_index);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Generic INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////
    PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.searchSid_Pi, pO_payloadBuffer, vl_index);
    PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.state, pO_payloadBuffer, vl_index);

    // now which kind of log 
    if (DABMW_SF_MASK_LOG_VERSION_BG == (DABMW_SF_MASK_LOG_VERSION_BG & vl_logInfo.sf_log_version))
    {
        // bg only.... 
        vl_len = DABMW_ServiceFollowing_BuildLogPayloadMsgBgProcessing((pO_payloadBuffer+vl_index), (vI_bufferLen-vl_index), vI_isAuto, vI_version, &vl_logInfo);        
        vl_index+=vl_len;
    }
    else
    {
        // Version for generic info, general...
        // 
        
        // timing data for system switch
        //
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.deltaTimeSinceLastSwitch, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.deltaTimeSinceLastServiceRecoverySearch, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.idle_timerStart, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.deltaTimeSinceLastSearchForAF , pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.deltaTimeSinceLastFullScan, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.deltaTimeSinceLastLandscaspeBuilding, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.deltaTimeSinceLastLandscapeDelayEstimation, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.deltaTimeFromAlternateSwitch, pO_payloadBuffer, vl_index);
             

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // CURRENT CELL INFORMATION 
        //////////////////////////////////////////////////////////////////////////////////////////////////
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.app, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Frequency, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Eid, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Sid, pO_payloadBuffer, vl_index);

        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.qualityStatus, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.fmQuality.combinedQ, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.fmQuality.multipath, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.fmQuality.adjacentChannel, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.fmQuality.deviation, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.fmQuality.fieldStrength, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.fmQuality.fieldStrength_dBuV, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.fmQuality.detuning, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.dabQuality.fic_ber, pO_payloadBuffer, vl_index);

        if (DABMW_SF_LOG_VERSION_V3 <= vl_logInfo.sf_log_version)
        {
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.source.value, pO_payloadBuffer, vl_index);
        }

        if (DABMW_SF_LOG_VERSION_V4 <= vl_logInfo.sf_log_version)
        {
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.dabQuality.service_selected, pO_payloadBuffer, vl_index);
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.dabQuality.component_type, pO_payloadBuffer, vl_index);
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.originalCell.Quality.dabQuality.audio_ber_level, pO_payloadBuffer, vl_index);    
        }

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // Alternate CELL INFORMATION 
        //////////////////////////////////////////////////////////////////////////////////////////////////

        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.app, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Frequency, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Eid, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Sid, pO_payloadBuffer, vl_index);

        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.qualityStatus, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.fmQuality.combinedQ, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.fmQuality.multipath, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.fmQuality.adjacentChannel, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.fmQuality.deviation, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.fmQuality.fieldStrength, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.fmQuality.fieldStrength_dBuV, pO_payloadBuffer, vl_index);
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.fmQuality.detuning, pO_payloadBuffer, vl_index);
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.dabQuality.fic_ber, pO_payloadBuffer, vl_index);

        if (DABMW_SF_LOG_VERSION_V3 <= vl_logInfo.sf_log_version)
        {
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.source.value, pO_payloadBuffer, vl_index);
        }

        if (DABMW_SF_LOG_VERSION_V4 <= vl_logInfo.sf_log_version)
        {
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.dabQuality.service_selected, pO_payloadBuffer, vl_index); 
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.dabQuality.component_type, pO_payloadBuffer, vl_index);
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.alternateCell.Quality.dabQuality.audio_ber_level, pO_payloadBuffer, vl_index);    
        }
        

        //////////////////////////////////////////////////////////////////////////////////////////////////
        // BG PROCESSING INFORMATION 
        //////////////////////////////////////////////////////////////////////////////////////////////////  
        if (DABMW_SF_LOG_VERSION_V1 == vl_logInfo.sf_log_version)
        {
            // 1st version of the sent out is requested : kept for compatibility
            //
            vl_index += DABMW_ServiceFollowing_BuildLogPayloadMsgBgProcessing((pO_payloadBuffer+vl_index), (vI_bufferLen-vl_index), vI_isAuto, vI_version, &vl_logInfo);        
       }
       else
       {
        	 //////////////////////////////////////////////////////////////////////////////////////////////////
       		 // Seamless Alternate CELL INFORMATION 
        	 //////////////////////////////////////////////////////////////////////////////////////////////////
		 	// database info
			 PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.seamlessInformationStatus, pO_payloadBuffer, vl_index); 
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.databaseInfo.delay_estimate, pO_payloadBuffer, vl_index);
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.databaseInfo.average_RMS2_FAS, pO_payloadBuffer, vl_index);
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.databaseInfo.average_RMS2_SAS, pO_payloadBuffer, vl_index);
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.databaseInfo.confidence_level, pO_payloadBuffer, vl_index);
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.databaseInfo.lastStoredTime, pO_payloadBuffer, vl_index);
			 PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.databaseInfo.provider_type, pO_payloadBuffer, vl_index); 

			// last switch info
			 PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.lastSwitchInfo.systemToSwitch, pO_payloadBuffer, vl_index); 
			 PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.lastSwitchInfo.status, pO_payloadBuffer, vl_index); 		
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.lastSwitchInfo.delay_estimate, pO_payloadBuffer, vl_index);
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.lastSwitchInfo.average_RMS2_FAS, pO_payloadBuffer, vl_index);
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.lastSwitchInfo.average_RMS2_SAS, pO_payloadBuffer, vl_index);
			 PUT_U32_TO_U8_ARRAY_WITH_INDEX(vl_logInfo.seamlessInfo.lastSwitchInfo.lastSwitchTime, pO_payloadBuffer, vl_index);		 
       	}
    
        // now it is evacuated : clear flags
        DABMW_SF_LOG_INFO_UPDATE_EVACAUTION_DONE;
    }

    DABMW_SF_LOG_INFO_UPDATE_EVACAUTION_DONE
        
    return vl_index;
}

tU16 DABMW_ServiceFollowing_BuildLogPayloadMsgBgProcessing(tPU8 pO_payloadBuffer, tU16 vI_bufferLen, tBool vI_isAuto, tU8 vI_version, DABMW_serviceFollowing_LogInfoMsgTy *pI_logInfo)
{
    tU16 vl_index = 0;
    tU8 vl_cnt1=0;
    tU16 vl_nbFMFreqOut = 0;
	tU16 vl_nbDABreqOut = 0;
   
    PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.last_searchProcInfo.value, pO_payloadBuffer, vl_index);
    PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.dab_TypeOfScan, pO_payloadBuffer, vl_index);
    PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.fm_TypeOfScan, pO_payloadBuffer, vl_index);
    PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.last_searchSuccessfull, pO_payloadBuffer, vl_index);

    // start by the FM freq
    // adapt to avoid > than buffer
    vl_nbFMFreqOut = pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.numFMFreqForCheck;

	// check the remaining size available versus the needed one
	// needded one = 1 for num AF FM, 1 for num AF DAB, AFFM * DABMW_SF_LOG_FM_AF_SIZE, 
   	if (((vl_nbFMFreqOut * DABMW_SF_LOG_FM_AF_SIZE) + 2) > (vI_bufferLen - vl_index))
   		{
   		// decrease the number of FM freq out
   		// Left minimum room for DAB ... let's say 2 DABs ? 
   		vl_nbFMFreqOut = ((vI_bufferLen - vl_index) - 2 - (DABWM_SG_LOG_MIN_ROOM_DAB_AF * DABMW_SF_LOG_DAB_AF_SIZE)) / DABMW_SF_LOG_FM_AF_SIZE;
   		}

    PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_nbFMFreqOut, pO_payloadBuffer, vl_index);

    for (vl_cnt1=0;vl_cnt1<vl_nbFMFreqOut ;vl_cnt1++)
        {
        // put the AF Frequency
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt1].frequency, pO_payloadBuffer, vl_index);

        // put the AF Frequency
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt1].piValue, pO_payloadBuffer, vl_index);

        // put the AF place information
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt1].statusFlagInfo.value, pO_payloadBuffer, vl_index);

        // put the evaluation result  information
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt1].bgCheckStatus, pO_payloadBuffer, vl_index);

        // put the frequency quality information
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.FM_FrequencyCheck.entry[vl_cnt1].quality.fmQuality.fieldStrength_dBuV, pO_payloadBuffer, vl_index);

        }

	// check the remaining size available versus the needed one
	// needded one = 1 for num AF DAB, NB DAB * DABMW_SF_LOG_FM_AF_SIZE, 

	vl_nbDABreqOut = pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.numDabFreqForCheck;
	
   	if (((vl_nbDABreqOut * DABMW_SF_LOG_DAB_AF_SIZE) + 1) > (vI_bufferLen - vl_index))
   		{
   		// decrease the number of FM freq out
   		//
   		vl_nbDABreqOut = ((vI_bufferLen - vl_index) - 1) / DABMW_SF_LOG_DAB_AF_SIZE;
   		}

    // start by the DAB freq
    PUT_U8_TO_U8_ARRAY_WITH_INDEX(vl_nbDABreqOut, pO_payloadBuffer, vl_index);

    // start by the DAB freq
    //
    for (vl_cnt1=0;vl_cnt1<vl_nbDABreqOut;vl_cnt1++)
        {
        // put the AF Frequency
        PUT_U32_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt1].frequency, pO_payloadBuffer, vl_index);

        // put the Service Linking information
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt1].statusFlagInfo.value, pO_payloadBuffer, vl_index);

        // put the evaluation result  information
        PUT_U8_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt1].bgCheckStatus, pO_payloadBuffer, vl_index);

        // put the frequency quality information
        PUT_U16_TO_U8_ARRAY_WITH_INDEX(pI_logInfo->bgProcessingInformation.DABMW_SF_freqCheckList.DAB_FrequencyCheck.entry[vl_cnt1].quality.dabQuality.fic_ber, pO_payloadBuffer, vl_index);

        }

    // now it is evacuated : clear flags
    DABMW_SF_LOG_INFO_UPDATE_BG_PROC_EVACAUTION_DONE;


    return vl_index;
}

tVoid DABMW_ServiceFollowing_InitLogInfo()
{
    DABMW_SF_LOG_INFO_UPDATE_EVACAUTION_DONE;
    
    return;
}



//
// procedure to log status from Seamless for logs... 
//

DABMW_serviceFollowingLogInfoSeamlessTy DABMW_ServiceFollowing_SetSeamlessLogInfoStatus()
{
	DABMW_serviceFollowingLogInfoSeamlessTy vl_res;

 #ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
    tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);
    tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
  	tU32  vl_dabFrequency;
    tU32  vl_dabEid;
    tU32  vl_dabSid;
    tU32  vl_fmPi;
  	DABMW_SS_EstimationResutDatabaseTy* pl_StoredReport = NULL;
	DABMW_SF_SS_LastSwitchRequestInfoTy vl_lastSwitchInfo;
 	SF_tMSecond vl_systemTime;
		
	// Get the current system time
    vl_systemTime = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime ();

	DABMW_ServiceFollowing_ExtInt_MemorySet(&vl_res, 0x00,  sizeof(DABMW_serviceFollowingLogInfoSeamlessTy));
	 
    if (false == DABMW_serviceFollowingData.seamlessSwitchingMode)
    	{
    	vl_res.seamlessInformationStatus = DABMW_SF_LOG_INFO_SEAMLESS_NOT_ACTIVATED;
    	}
	else
		{
		if ((DABMW_serviceFollowingStatus.originalAudioUserApp == DABMW_NONE_APP)
		|| (DABMW_serviceFollowingStatus.alternateApp == DABMW_NONE_APP)
        || (vl_originalIsDab == vl_alternateIsDab))    
    		{
	    	// seamless not applicable
	    	vl_res.seamlessInformationStatus = DABMW_SF_LOG_INFO_SEAMLESS_NOT_APPLICABLE;
    		}
		else
			{
			// we have some information to provide from Database

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
			
	        	/* Look if already present : if so just update */

				pl_StoredReport = DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(vl_dabFrequency,
																		vl_dabEid,
																		vl_dabSid,
																		vl_fmPi,
																	    DAMBW_SF_MEASURE_VALIDITY_INFINITE);

				/* if found & result == SUCCESS 
				*/
				if ((NULL != pl_StoredReport)
					&&
					(SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS == pl_StoredReport->SS_EstimationResult.status))
					{
			  
					vl_res.seamlessInformationStatus = DABMW_SF_LOG_INFO_SEAMLESS_STORED_INFO;
					vl_res.databaseInfo.average_RMS2_FAS = pl_StoredReport->SS_EstimationResult.average_RMS2_FAS;
					vl_res.databaseInfo.average_RMS2_SAS = pl_StoredReport->SS_EstimationResult.average_RMS2_SAS;
					vl_res.databaseInfo.confidence_level = pl_StoredReport->SS_EstimationResult.confidence_level;
					vl_res.databaseInfo.delay_estimate = pl_StoredReport->SS_EstimationResult.absolute_estimated_delay_in_samples;
					vl_res.databaseInfo.provider_type = (DABMW_SF_LogInfoSeamlessInformationProviderTy) pl_StoredReport->SS_EstimationResult.provider_type;	
					vl_res.databaseInfo.lastStoredTime = (vl_systemTime - pl_StoredReport->TimeLastEstimated);
				    }
				else
					{
					vl_res.seamlessInformationStatus = DABMW_SF_LOG_INFO_SEAMLESS_NO_STORED_INFO;					
					}

                // update the status if estimation is on-going
				if (DABMW_SF_STATE_IDLE_SEAMLESS == DABMW_serviceFollowingStatus.status)
				    {
					vl_res.seamlessInformationStatus = DABMW_SF_LOG_INFO_SEAMLESS_EVALUATION_ON_GOING;
					}
                    
			}
	
			// add information around last switch
			//

			vl_lastSwitchInfo = DABMW_ServiceFollowing_RetrieveLastSwitchInfo();

			if (false == vl_lastSwitchInfo.switchRequestValidity)
				{
				vl_res.lastSwitchInfo.status = SF_SEAMLESS_SWITCHING_STATUS_NO_SWITCH_STORED;
				}
			else
				{
				vl_res.lastSwitchInfo.status = (DABMW_SF_LogInfoSeamlessSwitching_SwitchingStatusTy) vl_lastSwitchInfo.SS_SwitchResponse.status;
				if (SD_SEAMLESS_ESTIMATION_DEFAULT_DELAY != vl_lastSwitchInfo.SS_SwitchRequest.absolute_estimated_delay_in_samples)
					{
					// it means a delay has been requested
					vl_res.lastSwitchInfo.delay_estimate = vl_lastSwitchInfo.SS_SwitchRequest.absolute_estimated_delay_in_samples;
					}
				else
					{
					// it means switch without delay
					vl_res.lastSwitchInfo.delay_estimate = 0;
					}
				vl_res.lastSwitchInfo.average_RMS2_FAS = vl_lastSwitchInfo.SS_SwitchRequest.average_RMS2_FAS;
				vl_res.lastSwitchInfo.average_RMS2_SAS = vl_lastSwitchInfo.SS_SwitchRequest.average_RMS2_SAS;
				vl_res.lastSwitchInfo.lastSwitchTime = (vl_systemTime - vl_lastSwitchInfo.TimeLastSwitchRequested);
				vl_res.lastSwitchInfo.systemToSwitch = (DABMW_SF_LogInfoSeamlessSwitchingSystemToSwitchTy) vl_lastSwitchInfo.SS_SwitchRequest.systemToSwitch;
				}
		
		}
       
   

#else // CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
	
	// seamless not applicable
    vl_res.seamlessInformationStatus = DABMW_SF_LOG_INFO_SEAMLESS_NOT_APPLICABLE;
#endif
    
    return vl_res;
}


#ifdef __cplusplus
}
#endif

#endif // CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // SERVICE_FOLLOWING_MAINLOOP_C
// End of file

