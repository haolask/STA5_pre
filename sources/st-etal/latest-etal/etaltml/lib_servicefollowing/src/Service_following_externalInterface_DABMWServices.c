//!
//!  \file      service_following_background.c
//!  \brief     <i><b> Service following implementation : external interface definition => OSAL </b></i>
//!  \details   This file provides functionalities for service following background check, scan and AF check
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_C


#include "osal.h"
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"


#include "service_following.h"

#include "service_following_internal.h"

#include "Service_following_externalInterface_DABMWServices.h"
#include "service_following_background.h"

//
// Function to retrieve the country information
//

DABMW_SF_mwCountryTy DABMW_ServiceFollowing_ExtInt_GetCountry (tVoid)
{
    return ((DABMW_SF_mwCountryTy) DABMW_GetCountry());
   //return(DABMW_COUNTRY_EUROPE);
}


DABMW_SF_systemBandsTy DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(tU32 vI_frequency)
{
	// return((DABMW_SF_systemBandsTy) DABMW_GetApplicationTableSystemBand(DABMW_NONE_APP, vI_frequency, DABMW_COUNTRY_NONE));
	return(DABMW_GetReceiverTableSystemBand(DABMW_NONE_APP, vI_frequency, DABMW_COUNTRY_NONE));
}

tBool DABMW_ServiceFollowing_ExtInt_IsApplicationDab (DABMW_SF_mwAppTy app)
{
	tBool vl_res = false;

/*	ETAL_HANDLE vl_etalHandle;

	vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(app);

	if (ETAL_INVALID_HANDLE != vl_etalHandle)
	{
		vl_res = ((ETAL_BCAST_STD_DAB == ETAL_receiverGetStandard(vl_etalHandle)) ? true:false);
	}
	*/

	if ((DABMW_MAIN_AUDIO_DAB_APP == app) || ( DABMW_SECONDARY_AUDIO_DAB_APP == app))
		{
		vl_res = true;
		}
   
		
    return (vl_res);
}

ETAL_HANDLE DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(DABMW_SF_mwAppTy app)
{
	ETAL_HANDLE vl_etalHandle = ETAL_INVALID_HANDLE;

    vl_etalHandle = ETALTML_GetPathAllocation(DABMW_ServiceFollowing_ExtInt_GetEtalPath(app));

  /*
	if (DABMW_serviceFollowingStatus.originalAudioUserApp == app)
		{
		vl_etalHandle = DABMW_serviceFollowingStatus.originalHandle;
		}
	else if (DABMW_serviceFollowingStatus.alternateApp == app)
		{
		vl_etalHandle = DABMW_serviceFollowingStatus.alternateHandle;
		}
	else if (DABMW_serviceFollowingStatus.currentAudioUserApp == app)
		{
		vl_etalHandle = DABMW_serviceFollowingStatus.currentHandle;
		}
	else if (DABMW_SF_BackgroundScanInfo.bgScan_App == app)
		{
		vl_etalHandle = DABMW_SF_BackgroundScanInfo.bgScan_Handle;
		}

*/
	// check if the found handle is valid. If not ==> null
	if ((ETAL_INVALID_HANDLE != vl_etalHandle) && (false == ETAL_receiverIsValidHandle(vl_etalHandle)))
	{
		vl_etalHandle = ETAL_INVALID_HANDLE;
	}	

    return (vl_etalHandle);
}

tSInt DABMW_ServiceFollowing_ExtInt_StoreEtalHandleFromApp(ETAL_HANDLE vI_etalHandle, DABMW_SF_mwAppTy app)
{
	tSInt vl_res = OSAL_OK; 

	if (DABMW_serviceFollowingStatus.originalAudioUserApp == app)
		{
		DABMW_serviceFollowingStatus.originalHandle = vI_etalHandle;
		}
	
	if (DABMW_serviceFollowingStatus.alternateApp == app)
		{
		DABMW_serviceFollowingStatus.alternateHandle = vI_etalHandle;
		}
	
	if (DABMW_serviceFollowingStatus.currentAudioUserApp == app)
		{
		DABMW_serviceFollowingStatus.currentHandle = vI_etalHandle;
		}

	if (DABMW_SF_BackgroundScanInfo.bgScan_App == app)
		{
		DABMW_SF_BackgroundScanInfo.bgScan_Handle = vI_etalHandle;
		}
	
    return (vl_res);
}


tBool DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm (DABMW_SF_mwAppTy app)
{	
	tBool vl_res = false;

	/*
	ETAL_HANDLE vl_etalHandle;

	vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(app);

	if (ETAL_INVALID_HANDLE != vl_etalHandle)
	{
		vl_res = ((ETAL_BCAST_STD_FM == ETAL_receiverGetStandard(vl_etalHandle)) ? true:false);
	}
	*/

	if ((DABMW_MAIN_AMFM_APP == app) || ( DABMW_BACKGROUND_AMFM_APP == app))
		{
		vl_res = true;
		}

   
    return (vl_res);

}

tBool DABMW_ServiceFollowing_ExtInt_IsFMBand (DABMW_SF_systemBandsTy band)
{
   return (DABMW_IsFMBand(band));

}

tBool DABMW_ServiceFollowing_ExtInt_IsDABBand (DABMW_SF_systemBandsTy band)
{
   return (DABMW_IsDABBand(band));
   
}


tU32 DABMW_ServiceFollowing_ExtInt_GetFrequencyFromApp (DABMW_SF_mwAppTy vI_app)
{
	tU32 vl_frequency = DABMW_INVALID_FREQUENCY; 
	ETAL_HANDLE vl_etalHandle;

	vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			vl_frequency = ETAL_receiverGetFrequency(vl_etalHandle);
		}

		ETAL_receiverReleaseLock(vl_etalHandle);
		
	}
	else
	{
		// nothing
	}
	//	Lock addition change end

	return (vl_frequency);
}

tBool DABMW_ServiceFollowing_ExtInt_GetApplicationBusyStatus (DABMW_SF_mwAppTy vI_app)
{

    tBool vl_res = false;
    
    if (DABMW_INVALID_FREQUENCY != DABMW_ServiceFollowing_ExtInt_GetFrequencyFromApp(vI_app))
    {
    	vl_res = true;
    }
	
    return vl_res;

}

tU32 DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin (DABMW_SF_systemBandsTy vI_bandValue, DABMW_SF_mwCountryTy vI_countryId)
{

	tU32 vl_bandMin;

	vl_bandMin = DABMW_GetSystemBandMinFreq(vI_bandValue);
	
	//return(DABMW_GetBandInfoTable_freqmin(vI_bandValue, vI_countryId));
	return(vl_bandMin);
}


tU32 DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmax (DABMW_SF_systemBandsTy vI_bandValue, DABMW_SF_mwCountryTy vI_countryId)
{
	tU32 vl_bandMax;
		

	vl_bandMax = DABMW_GetSystemBandMaxFreq(vI_bandValue);

	//	return(DABMW_GetBandInfoTable_freqmax(vI_bandValue, vI_countryId));
	return(vl_bandMax);
}


tU32 DABMW_ServiceFollowing_ExtInt_GetNextFrequencyFromFreq (tU32 vI_frequency, DABMW_SF_systemBandsTy vI_systemBand, tBool vI_DirectionIsUp)
{
	return(DABMW_GetNextFrequencyFromFreq(vI_frequency, vI_systemBand, vI_DirectionIsUp));

}


tSInt DABMW_ServiceFollowing_ExtInt_RegisterExternalTune()
{
	
	tSInt vl_res = OSAL_OK;

	if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtTuneFrequency, DABMW_ServiceFollowing_ExtInt_TuneCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
		{
			vl_res = OSAL_ERROR;
		}


	// we need also to register on receiver destroy 	
	if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtReceiverDestroy, DABMW_ServiceFollowing_ExtInt_ReceiverDestroyCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
		{
			vl_res = OSAL_ERROR;
		}

	return(vl_res);
}


tSInt DABMW_ServiceFollowing_ExtInt_RegisterExternalAudioSourceSelection()
{
	tSInt vl_res = OSAL_OK;

	if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtAudioSourceSelect, DABMW_ServiceFollowing_ExtInt_AudioSourceSelectionCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
		{
			vl_res = OSAL_ERROR;
		}

	return vl_res;
}


tVoid DABMW_ServiceFollowing_ExtInt_AudioSourceSelectionCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
	
	EtalAudioSourceSelectInternal *pl_AudioSource = (EtalAudioSourceSelectInternal *) param;
	DABMW_SF_mwAppTy vl_app;
	DABMW_systemBandsTy vl_systemBand;
	tU32 vl_frequency = DABMW_INVALID_FREQUENCY;
	tU32 vl_Sid = DABMW_INVALID_SERVICE_ID;
	tU32 vl_Ueid = DABMW_INVALID_ENSEMBLE_ID, vl_Ueid_tmp;
	tBool vl_isNew;
	ETAL_HANDLE vl_etalHandle = ETAL_INVALID_HANDLE;

	DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,"DABMW_ServiceFollowing_ExtInt_AudioSourceSelectionCallback : audio source = %d, external request = %d\n",
						pl_AudioSource->m_audioSourceSelected,
						pl_AudioSource->m_externalRequestInfo
						);
		
	// Depending on the auido 
	if (ETAL_AUDIO_SOURCE_STAR_AMFM == pl_AudioSource->m_audioSourceSelected)
	{
	
		vl_systemBand = DABMW_BAND_FM_EU;
		vl_app = DABMW_MAIN_AMFM_APP;
	}
	else if ( ETAL_AUDIO_SOURCE_DCOP_STA660 == pl_AudioSource->m_audioSourceSelected)
	{
		vl_systemBand = DABMW_BAND_DAB_III;
		vl_app = DABMW_MAIN_AUDIO_DAB_APP;
	}
	else
	{
		// not applicable for SF
		// deactivate SF
		DABWM_ServiceFollowing_Disable();
		return;
	}


	
	if (true == pl_AudioSource->m_externalRequestInfo)
	{
		//
		// Retrieve the receiver associated to the standard
		//
		vl_etalHandle = ETALTML_GetPathAllocation(DABMW_ServiceFollowing_ExtInt_GetEtalPath(vl_app));

		if (ETAL_INVALID_HANDLE != vl_etalHandle)
		{
			// we have an handle available.
			// let's retrieve the frequency and Sid
			//

			// Addition of receiver Lock while sending the command
			// to avoid conflict management on interface
			// Lock addition change start
			if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
			{

				// check receiver is still valid and has not been destroyed !
				if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
				{
					vl_frequency = ETAL_receiverGetFrequency(vl_etalHandle);

					if (ETAL_AUDIO_SOURCE_DCOP_STA660 == pl_AudioSource->m_audioSourceSelected)
					{
						// for DAB : 
						// Get the current EID
						ETAL_cmdGetCurrentEnsemble_MDR(vl_etalHandle, &vl_Ueid);

						
						if (DABMW_INVALID_ENSEMBLE_ID != vl_Ueid)
						{
							// For DAB, get the SID
							ETAL_statusGetDABService(&vl_Ueid_tmp, &vl_Sid, &vl_isNew);

							if (vl_Ueid_tmp	!= vl_Ueid)
							{
								// this is strange
								
								vl_Ueid = DABMW_INVALID_ENSEMBLE_ID;
								vl_Sid = DABMW_INVALID_SERVICE_ID;
							}
							else
							{
								
							}
						}
						else
						{
							// nothing selected
						}
						
					}
				}
				else
				{
					// nothing to do
				}
				
				ETAL_receiverReleaseLock(vl_etalHandle);
				
			}
			else
			{
				// nothing
			}
			//	Lock addition change end				
		}
		
		// external tune or seek 
		// call the SF on Tune to get the tune info
		//
		DABMW_ServiceFollowingOnTune (vl_app, vl_systemBand,vl_frequency, vl_Ueid, vl_Sid, false, vl_etalHandle); 	

		DABMW_SF_PRINTF( TR_LEVEL_USER_1,"DABMW_ServiceFollowing_ExtInt_AudioSourceSelectionCallback :handle = %d,  freq = %d, app = %d, ueid = 0x%x, Sid = 0x%x, etal_path = %d\n",
								vl_etalHandle,
								vl_frequency,
								vl_app,
								vl_Ueid,
								vl_Sid,
								DABMW_ServiceFollowing_ExtInt_GetEtalPath(vl_app));
				

	}
	else 
	{
		// do nothing 
				
	
	}
	
}


tVoid DABMW_ServiceFollowing_ExtInt_TuneCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{

	EtalTuneInfoInternal *pl_TuneStatus = (EtalTuneInfoInternal *) param;
	DABMW_SF_mwAppTy vl_app;
	DABMW_systemBandsTy vl_systemBand;
	EtalBcastStandard vl_standard = ETAL_receiverGetStandard(hGeneric);


	if (ETAL_BCAST_STD_DAB == vl_standard)
	{
		vl_systemBand = DABMW_BAND_DAB_III;
		vl_app = DABMW_MAIN_AUDIO_DAB_APP;
	}
	else if (ETAL_BCAST_STD_FM == vl_standard)
	{
		vl_systemBand = DABMW_BAND_FM_EU;
		vl_app = DABMW_MAIN_AMFM_APP;
	}
	else
	{
		// not supported, return
		return;
	}


	DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,"DABMW_ServiceFollowing_ExtInt_TuneCallback : handle = %d,  freq = %d, app = %d, sync status = %d, is external = %d\n",
					hGeneric, pl_TuneStatus->m_Frequency, vl_app, pl_TuneStatus->m_syncInternal, pl_TuneStatus->m_externalRequestInfo
					);
	

	if (true == pl_TuneStatus->m_externalRequestInfo)
	{
		// specific handling is required for ETAL 
		// 
		
		// add-on ETAL :
		// We need also to reset as well the handle, and even to release it if not the current one !
		
		// only if SF is activated : else we let the interface client to decide
		// 
		if (true == DABMW_serviceFollowingData.enableServiceFollowing)
		{

			if ((ETAL_INVALID_HANDLE != DABMW_serviceFollowingStatus.originalHandle)
			&& (hGeneric != DABMW_serviceFollowingStatus.originalHandle)
			)
			{
				// destroy the handle
				ETAL_destroyReceiverInternal(DABMW_serviceFollowingStatus.originalHandle);
			}
		}

		// external tune or seek 
		// call the SF on Tune to get the tune info
		//
		if (ETAL_BCAST_STD_FM == vl_standard)
		{
			DABMW_ServiceFollowingOnTune (vl_app, vl_systemBand, pl_TuneStatus->m_Frequency, DABMW_INVALID_ENSEMBLE_ID, DABMW_INVALID_SERVICE_ID, false, hGeneric);		
		}
		else
		{
			// in DAB on Tune we consider we have no EID/SID  : this will be provided by Service Select ...
			
			DABMW_ServiceFollowingOnTune (vl_app, vl_systemBand, pl_TuneStatus->m_Frequency, DABMW_INVALID_ENSEMBLE_ID, DABMW_INVALID_SERVICE_ID, false, hGeneric);					
		}
	}
	else 
	{
		// tune internal: call the right callback
		// this is for DAB only in theory

		if (ETAL_BCAST_STD_DAB == vl_standard)
		{
		
			DABMW_serviceFollowingStatus.dab_syncStatus	 = pl_TuneStatus->m_syncInternal;
			
			DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_DAB_TUNE);

			// EPR Correction : 
			// the callback should not do any thread wait since it will block the calling thread.
			// In addition, the timing is already managed in background scan handler
			//
			/*
			if (DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_TUNED == DABMW_serviceFollowingStatus.dab_syncStatus)
			{
				// if the result was ok, and simulate the fig available
				// this trigger the service list reading...
				// so wait some sec
				// oo
				OSAL_s32ThreadWait(DABMW_SF_DAB_WAIT_FIC_ACQUISITION_TIME_MS);
				
				DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_DAB_FIC_READ);
			}
			*/
		}
		else
		{
			// do nothing 
			
		}
	}

}

tSInt DABMW_ServiceFollowing_ExtInt_RegisterExternalServiceSelect()
{
	
	tSInt vl_res = OSAL_OK;

	if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtServiceSelection, DABMW_ServiceFollowing_ExtInt_ServiceSelectCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
		{
			vl_res = OSAL_ERROR;
		}
	
	return(vl_res);
}

tVoid DABMW_ServiceFollowing_ExtInt_ServiceSelectCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{

	EtalServiceSelectionStatus *pl_ServiceSelectStatus = (EtalServiceSelectionStatus *) param;
	DABMW_SF_mwAppTy vl_app;
	DABMW_systemBandsTy vl_systemBand;
	EtalBcastStandard vl_standard = ETAL_receiverGetStandard(hGeneric);


	if (ETAL_BCAST_STD_DAB == vl_standard)
	{
		vl_systemBand = DABMW_BAND_DAB_III;
		vl_app = DABMW_MAIN_AUDIO_DAB_APP;
	}
	else if (ETAL_BCAST_STD_FM == vl_standard)
	{
		vl_systemBand = DABMW_BAND_FM_EU;
		vl_app = DABMW_MAIN_AMFM_APP;
	}
	else
	{
		// not supported, return
		return;
	}

	
	DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,"DABMW_ServiceFollowing_ExtInt_ServiceSelectCallback : handle = %d, vl_app = %d, frequency = %d, Ueid = 0x%x, Sid = 0x%x, \n",
					hGeneric, vl_app,
					pl_ServiceSelectStatus->m_Frequency,
					pl_ServiceSelectStatus->m_Ueid, 
					pl_ServiceSelectStatus->m_Sid
					);
	
	DABMW_ServiceFollowingOnTune (vl_app, vl_systemBand, pl_ServiceSelectStatus->m_Frequency, pl_ServiceSelectStatus->m_Ueid, pl_ServiceSelectStatus->m_Sid, false, hGeneric);
	
}

tVoid DABMW_ServiceFollowing_ExtInt_ReceiverDestroyCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
	ETAL_HANDLE vl_handle = hGeneric;

	DABMW_SF_PRINTF( TR_LEVEL_SYSTEM,"DABMW_ServiceFollowing_ExtInt_ReceiverDestroyCallback : handle = %d\n",
						hGeneric
						);
	

	// reset the information 
	if (vl_handle == DABMW_serviceFollowingStatus.originalHandle)
	{
	    DABMW_serviceFollowingStatus.originalSystemBand = DABMW_BAND_NONE;
	    DABMW_serviceFollowingStatus.originalFrequency = DABMW_INVALID_FREQUENCY;      
	    DABMW_serviceFollowingStatus.originalEid = DABMW_INVALID_ENSEMBLE_ID;
	    DABMW_serviceFollowingStatus.originalSid= DABMW_INVALID_SERVICE_ID;
		DABMW_serviceFollowingStatus.originalAudioUserApp = DABMW_NONE_APP;
	    DABMW_serviceFollowingStatus.original_DabServiceSelected = false;
	 	DABMW_serviceFollowingStatus.originalHandle= ETAL_INVALID_HANDLE;
 
	}
	else if (vl_handle == DABMW_serviceFollowingStatus.alternateHandle)
	{
/*	    DABMW_serviceFollowingStatus.alternateFrequency = DABMW_INVALID_FREQUENCY;
	    DABMW_serviceFollowingStatus.alternateSystemBand = DABMW_BAND_NONE;
	    DABMW_serviceFollowingStatus.alternateApp = DABMW_NONE_APP;
	    DABMW_serviceFollowingStatus.alternateEid = DABMW_INVALID_ENSEMBLE_ID;
*/
		DABMW_serviceFollowingStatus.alternateHandle= ETAL_INVALID_HANDLE;
	}
	else if (vl_handle == DABMW_serviceFollowingStatus.currentHandle)
	{
/*	   DABMW_serviceFollowingStatus.currentSystemBand = DABMW_BAND_NONE;
	    DABMW_serviceFollowingStatus.currentFrequency = DABMW_INVALID_FREQUENCY;      
	    DABMW_serviceFollowingStatus.currentEid = DABMW_INVALID_ENSEMBLE_ID;
	    DABMW_serviceFollowingStatus.currentSid= DABMW_INVALID_SERVICE_ID; 
		DABMW_serviceFollowingStatus.currentAudioUserApp = DABMW_NONE_APP;
*/
		DABMW_serviceFollowingStatus.currentHandle = ETAL_INVALID_HANDLE;
	}
		

}


	
tSInt DABMW_ServiceFollowing_ExtInt_AllocateEtalHandleFromApp(DABMW_SF_mwAppTy vI_app)
{

	ETAL_HANDLE vl_etalHandle = ETAL_INVALID_HANDLE;
	tSInt vl_res = OSAL_OK; 
	


	ETALTML_getReceiverForPathInternal(&vl_etalHandle, DABMW_ServiceFollowing_ExtInt_GetEtalPath(vI_app), NULL, true);


	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no hanlde found. return error
		// 
		DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "*DABMW_ServiceFollowing_ExtInt_AllocateEtalHandleFromApp :  ERROR, Handle not available, for app = %d\n",
						vI_app);
		
		return(OSAL_ERROR);
	}
	
	vl_res = DABMW_ServiceFollowing_ExtInt_StoreEtalHandleFromApp(vl_etalHandle, vI_app);



	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_AllocateEtalHandleFromApp :  path = %d, etalhandle %d\n",
						vI_app,
						vl_etalHandle);
	
	return(vl_res);
}




#endif // #if defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) 

#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_C
