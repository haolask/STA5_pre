//!
//!  \file      service_following_audioswitch.c
//!  \brief     <i><b> Service following implementation : audio switch interface handling </b></i>
//!  \details   This file provides functionalities for service following audio switching functionnality
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.10.07
//!  \bug       Unknown
//!  \warning   None
//!


#ifndef SERVICE_FOLLOWING_AUDIOSWITCH_C
#define SERVICE_FOLLOWING_AUDIOSWITCH_C
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
#endif // #if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)


#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "seamless_switching_common.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)


#include "service_following.h"

#include "service_following_internal.h"

#include "service_following_mainloop.h"

#include "service_following_audioswitch.h"

#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "service_following_seamless.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)


#ifdef __cplusplus
extern "C" {
#endif





/* Procedure which cares of the switch between original and AF
* assuming everything is calculated and set previously
*/


tVoid DABMW_ServiceFollowing_SwitchToAF()
{

	tBool vl_alternateIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.alternateApp);
	tBool vl_originalIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(DABMW_serviceFollowingStatus.originalAudioUserApp);

	DABMW_SF_mwAppTy	vl_selectedApp = DABMW_serviceFollowingStatus.alternateApp;
	tSInt vl_tmp_res;
    tBool vl_dabServiceSelectionSucceed = false;
    
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
	SD_SeamlessSwitchingSystemToSwitchTy vl_systemToSwitch;

	/* action depends on configuration */
	if (true == vl_alternateIsDab)
		{
		vl_systemToSwitch = SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB;
		}
	else 
		{
		vl_systemToSwitch = SD_SEAMLESS_SWITCHING_SWITCH_TO_FM;
		}
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)

	/* action depends on configuration */

	/* Orig = FM , AF =  FM => AF switch 
	* Orig = FM , AF =  DAB => just a audio port switch
	* Orig = DAB , AF =  FM => just a audio port switch for now... later will be a seamless ?
	* Orig = DAB , AF =  DAB => just a audio port switch for now..
	*/

    if (DABMW_INVALID_FREQUENCY == DABMW_serviceFollowingStatus.originalFrequency)
     {
         vl_originalIsDab = false;
     }

	/* Alternate quality */
	if ((false == vl_originalIsDab) && (false == vl_alternateIsDab)) 
		{
		/* Orig = FM , AF =  FM => AF switch */

		/* let's tune as requested now 
		* MAIN FM as reference 
		* improvement could be an AF switch... however, we are in loss of service, so no interest a priori !
		*/
		vl_selectedApp = DABMW_MAIN_AMFM_APP;

		 DABMW_ServiceFollowing_ExtInt_AmFmAFSwitchRequest(vl_selectedApp, DABMW_serviceFollowingStatus.alternateFrequency);
		}
	else if ((false == vl_originalIsDab) && (true == vl_alternateIsDab))
		{
		/* Orig = FM , AF =  DAB => just a audio port switch */
		vl_selectedApp = DABMW_serviceFollowingStatus.alternateApp;

		/* for the moment, configure the output port has being DAB alternate..; so that seamless switch could work
		* that may already be do 
		*/
		/* Select the DAB service
		* not that it could already be selected... 
		* here should be improve to select is not already selected...
		*/
	
		if (false == DABMW_serviceFollowingStatus.alternate_DabServiceSelected)
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
				vl_dabServiceSelectionSucceed = true;

				// consider we start here the audio 
				DABMW_serviceFollowingStatus.alternateLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
								
				  /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
               DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SwitchToAF) : new DAB SERVICE SELECTION success / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
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
                // should not come here but you never know 
                vl_dabServiceSelectionSucceed = false;
                // let's print a failure
                //
                
                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
               DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (SwitchToAF) : ERROR => DAB SERVICE SELECTION FAILED / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.alternateApp,
                            DABMW_serviceFollowingStatus.alternateFrequency,
                            DABMW_serviceFollowingStatus.alternateEid,
                            DABMW_serviceFollowingStatus.alternateSid);
                DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */  

				}
			}
        else
            {
            vl_dabServiceSelectionSucceed = true;
            }

		
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK					
		if (true == DABMW_serviceFollowingData.seamlessSwitchingMode)
			{
			/* seamless switching request */
			// the audio port is configured only on the response
			// DABMW_SF_SetCurrentAudioPortUser(vl_selectedApp, true);

			// EPR correction
			// process the case of the switching not working
			// error in switch ==> return the error and stop there

			vl_tmp_res = DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(vl_systemToSwitch);
			if (OSAL_ERROR == vl_tmp_res)
				{
				// EPR correction
				// process the case of the switching not working
				// error in switch ==> return the error and stop there
			
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
			   DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (SwitchToAF) : ERROR => Seamless Switching Request Failure\n");
			   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   

				return;

				}

			}
		else
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
			{
            // codex #315761
            // EPR Change :always configure Seamless so that it is ready to play the rigth buffers...
            // 
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK                  
           /* seamless switching request */
           if ((SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB == vl_systemToSwitch)
               && (SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB != DABMW_ServiceFollowing_RetrieveLastSwitchSystemToSwitch())
               )
                {
                DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(vl_systemToSwitch);    
                }
           else
#endif
                {               
    			DABMW_SF_SetCurrentAudioPortUser(vl_selectedApp, true);
                } 
			}

        // update the alternate app flag : 
        // alternate is tuned since it was original
        // alternate service selection is the one from the original
        DABMW_serviceFollowingStatus.alternateTuned = true;
        DABMW_serviceFollowingStatus.original_DabServiceSelected = vl_dabServiceSelectionSucceed;

        // update the hysteresis flags to avoid too much transition
        // we are in switch to DAB, so hysteresis FM gets penalities
        DABMW_serviceFollowingStatus.fm_hysteresis_start_time = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
        DABMW_serviceFollowingStatus.dab_hysteresis_start_time = DABMW_INVALID_DATA;

		// consider we start here the audio 
		DABMW_serviceFollowingStatus.originalLastTimeNoAudio = DABMW_serviceFollowingStatus.alternateLastTimeNoAudio;
        
		}
	else if ((true == vl_originalIsDab) && (false == vl_alternateIsDab))
		{
		/* Orig = DAB , AF =  FM => just a audio port switch for now... later will be a seamless ?*/

        // here alternate APP should be MAIN_AMFM
		vl_selectedApp = DABMW_serviceFollowingStatus.alternateApp;        
		
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK	
		if (true == DABMW_serviceFollowingData.seamlessSwitchingMode)
			{
			/* seamless switching request */

			// EPR correction
			// process the case of the switching not working
			// error in switch ==> return the error and stop there

			vl_tmp_res = DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(vl_systemToSwitch);
			if (OSAL_ERROR == vl_tmp_res)
				{
				// EPR correction
				// process the case of the switching not working
				// error in switch ==> return the error and stop there
			
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
			   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
			   DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (SwitchToAF) : ERROR => Seamless Switching Request Failure\n");
			   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   

				return;

				}
			}
		else
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
			{
            // codex #315761
            // EPR Change :always configure Seamless so that it is ready to play the rigth buffers...
            // 
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK                  
           /* seamless switching request */
           if ((SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB == vl_systemToSwitch)
               && (SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB != DABMW_ServiceFollowing_RetrieveLastSwitchSystemToSwitch())
               )
                {
                DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(vl_systemToSwitch);    
                }
           else
#endif
                {               
    			DABMW_SF_SetCurrentAudioPortUser(vl_selectedApp, true);
                }   
			}

        // update the alternate app flag : 
        // alternate is tuned since it was original
        // alternate service selection is the one from the original
        DABMW_serviceFollowingStatus.alternateTuned = true;
        DABMW_serviceFollowingStatus.alternate_DabServiceSelected = DABMW_serviceFollowingStatus.original_DabServiceSelected;

        // update the hysteresis flags to avoid too much transition
        // we are in switch to DAB, so hysteresis FM gets penalit
        DABMW_serviceFollowingStatus.dab_hysteresis_start_time = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
        DABMW_serviceFollowingStatus.fm_hysteresis_start_time = DABMW_INVALID_DATA;
                
        // 
        // ??? Should not we deselect the DAB audio as well ?
        //
        
		}
	else if ((true == vl_originalIsDab) && (true == vl_alternateIsDab))
		{
		/*  Orig = DAB , AF =  DAB => just a audio port switch for now..  */
		vl_selectedApp = DABMW_serviceFollowingStatus.alternateApp;

		/* no seamless switch needed... */

        // Correction 
        // Just reload the Eid of original : in case it has changes ... 
        // case is ECC received meanwhile 
        
        DABMW_serviceFollowingStatus.originalEid = DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble(DABMW_serviceFollowingStatus.originalAudioUserApp);
                

		/* remove current service to avoid 2 audios
		* if we do not do that, the service selection of the 2nd service will fail
		*/
			
		 vl_tmp_res = DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_serviceFollowingStatus.originalAudioUserApp,										   // Use parameter passed
										 DABMW_SERVICE_SELECT_SUBFNCT_REMOVE,				// Sub-function is 5
										 DABMW_serviceFollowingStatus.originalEid,		 // Ensemble  
										 DABMW_serviceFollowingStatus.originalSid);		// Service ID 		  
	
		/* check : is the service removal  ok ?  */
		if (OSAL_OK == vl_tmp_res)
			{
		// Service ok  
				  /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
               DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SwitchToAF) : current DAB SERVICE removal ok / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.originalAudioUserApp,
                            DABMW_serviceFollowingStatus.originalFrequency,
                            DABMW_serviceFollowingStatus.originalEid,
                            DABMW_serviceFollowingStatus.originalSid);
                                                                 DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */  
			}
		else
			{
			
			// should not come here but you never know 
                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
               DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (SwitchToAF) : ERROR => DAB SERVICE REMOVAL FAILED / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.originalAudioUserApp,
                            DABMW_serviceFollowingStatus.originalFrequency,
                            DABMW_serviceFollowingStatus.originalEid,
                            DABMW_serviceFollowingStatus.originalSid);
              DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */  
			}		
		
		// Select the DAB service 
		//
		// Correction : error on setting the 'internal tune' to True
		//
		DABMW_SF_SetCurrentAudioPortUser(vl_selectedApp, true);
		
		
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
			vl_dabServiceSelectionSucceed = true;		

            /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
            DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
            DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SwitchToAF) : new DAB SERVICE SELECTION success / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
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
            // Service ok  
			vl_dabServiceSelectionSucceed = false;
            
			// should not come here but you never know 
			// if it fails : lets wait a bit the fic to be read and try again.
			// should not come here but you never know 
			
                /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
               DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
               DABMW_SF_PRINTF( TR_LEVEL_ERRORS, "DABMW_Service_Following (SwitchToAF) : ERROR => DAB SERVICE SET FAILED / app = %d, Freq = %d, Eid = 0x%04x, Sid= 0x%04x \n",
                            DABMW_serviceFollowingStatus.alternateApp,
                            DABMW_serviceFollowingStatus.alternateFrequency,
                            DABMW_serviceFollowingStatus.alternateEid,
                            DABMW_serviceFollowingStatus.alternateSid);
              DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                /* END TMP LOG */ 
			}			

            // update the alternate app flag : 
            // alternate is tuned since it was original
            // alternate service selection is reset since it has been removed
            DABMW_serviceFollowingStatus.alternateTuned = true;
            DABMW_serviceFollowingStatus.original_DabServiceSelected = vl_dabServiceSelectionSucceed;
            DABMW_serviceFollowingStatus.alternate_DabServiceSelected = false;
			
			// consider we start here the audio 
			DABMW_serviceFollowingStatus.alternateLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();
			DABMW_serviceFollowingStatus.originalLastTimeNoAudio = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();

		}
	else
		{
		/* unreachable statement... */
		;
		}

	/* Init the idle mode value */
	DABMW_ServiceFollowing_EnterIdleMode(vl_selectedApp,
					DABMW_serviceFollowingStatus.alternateSystemBand,
					DABMW_serviceFollowingStatus.alternateSid,
					DABMW_serviceFollowingStatus.alternateEid,
					DABMW_serviceFollowingStatus.alternateFrequency);	

//	DABMW_SF_SetCurrentAudioPortUser(DABMW_serviceFollowingStatus.originalAudioUserApp, true);
	
}




// Local function for audio port switch path tracking
tSInt DABMW_SF_SetCurrentAudioPortUser (DABMW_SF_mwAppTy app, tBool isInternalTune)
{

    tSInt vl_res;

    vl_res = DABMW_ServiceFollowing_ExtInt_SetCurrentAudioPortUser(app, isInternalTune);
    
                      /* TMP LOG */
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_SYSTEM)  
                   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
                   DABMW_SF_PRINTF( TR_LEVEL_SYSTEM, "DABMW_Service_Following (SF_SetCurrentAudioPortUser) : app = %d, isInternalTune = %d, result =  %d\n",
                                app,
                                isInternalTune,
                                vl_res);
                   DABMW_SF_PRINTF( TR_LEVEL_USER_1, "*-*-*-*-*-*-*-*-*-*-*-*\n" );
#endif // #if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)   
                    /* END TMP LOG */               

    return vl_res;
    
}


#ifdef __cplusplus
}
#endif

#endif //CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // SERVICE_FOLLOWING_MAINLOOP_C
// End of file

