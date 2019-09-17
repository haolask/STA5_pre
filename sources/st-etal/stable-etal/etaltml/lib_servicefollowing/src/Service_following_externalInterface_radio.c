//!
//!  \file      service_following_external_radio.c
//!  \brief     <i><b> Service following implementation : external interface definition => radio part </b></i>
//!  \details   This file provides functionalities for external interface : radio  part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!


#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RADIO_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RADIO_C

#include "osal.h"
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"


#include "Service_following_externalInterface_OSAL.h"
#include "Service_following_externalInterface_DABMWServices.h"


#include "Service_following_externalInterface_radio.h"
#include "service_following_internal.h"


tSInt DABMW_ServiceFollowing_ExtInt_TuneServiceIdCallback(DABMW_SF_mwAppTy vI_app, tU32 vI_tunedFreq, tBool vp_PIconfirmedStatus)
{
	EtalTuneServiceIdStatus vl_tuneServiceIdStatus;
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

	vl_tuneServiceIdStatus.m_receiverHandle = vl_etalHandle;
	vl_tuneServiceIdStatus.m_IsFoundstatus = vp_PIconfirmedStatus;
	vl_tuneServiceIdStatus.m_freq = vI_tunedFreq;
	vl_tuneServiceIdStatus.m_SidPi = DABMW_serviceFollowingStatus.originalSid;
	vl_tuneServiceIdStatus.m_Ueid = DABMW_serviceFollowingStatus.originalEid;
	
	
	ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_TUNE_SERVICE_ID, &vl_tuneServiceIdStatus,  sizeof(EtalTuneServiceIdStatus));

	return (OSAL_OK);

}


tSInt DABMW_ServiceFollowing_ExtInt_TuneFrequency (DABMW_SF_mwAppTy vI_app, tU32 vI_frequency)
{
	ETAL_HANDLE vl_etalHandleCurrent = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	ETAL_HANDLE vl_etalHandle;
	DABMW_SF_systemBandsTy vl_SF_systemBand;
	EtalFrequencyBand vl_etalFreqBand;
	tBool vl_appIsDab = DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_app);
    EtalProcessingFeatures proc_features;

	etalCmdTuneActionTy vl_tuneAction;
	tSInt vl_res = OSAL_OK;
	ETAL_STATUS vl_etalStatus;

	// Specific case !!
	// If this is a tune to reset frequency, it means in fact that it is a destroy we want to do 
	//


	if (DABMW_SF_RESET_FREQUENCY == vI_frequency)
	{
		// this is a clean-up of tuner
		// for now, do nothing
		
		DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_TuneFrequency : app = %d, frequency %d, current handle = %d\n",
					vI_app, 
					vI_frequency, 
					vl_etalHandleCurrent );



		// Destroy the tuner to free it
		if (ETAL_INVALID_HANDLE != vl_etalHandleCurrent)
		{
			// Addition of receiver Lock while sending the command
			// to avoid conflict management on interface
			// Lock addition change start
			if ( ETAL_receiverGetLock(vl_etalHandleCurrent) == ETAL_RET_SUCCESS)
			{	

				// check receiver is still valid and has not been destroyed !
				if (true == ETAL_receiverIsValidHandle(vl_etalHandleCurrent))
				{

					// for FM, reset the fast PI request for proper restart
					// Reset the fast_PI which was set if we were on FM
					if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm(vI_app))
					{
						DABMW_ServiceFollowing_ExtInt_RdsDataSetupPiDetectionMode(vI_app, false);
					}
					
					ETAL_destroyReceiverInternal(vl_etalHandleCurrent);
					// destroy/reset the handle
					DABMW_ServiceFollowing_ExtInt_StoreEtalHandleFromApp(ETAL_INVALID_HANDLE, vI_app);

					vl_res = OSAL_OK;
				}
				else
				{
					vl_res = OSAL_ERROR;
				}
				ETAL_receiverReleaseLock(vl_etalHandleCurrent);		

			}
			else
			{
				//nothing
				vl_res = OSAL_ERROR;
			}
			//	Lock addition change end
			
		}
		
		return(vl_res);
	}

	vl_etalStatus = ETALTML_getReceiverForPathInternal(&vl_etalHandle, DABMW_ServiceFollowing_ExtInt_GetEtalPath(vI_app), NULL, true);

	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no hanlde found. return error
		// 
				
		DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW_ServiceFollowing_ExtInt_TuneFrequency : ERROR, no handle found for app = %d, frequency %d\n",
							vI_app, 
							vI_frequency);
		return(OSAL_ERROR);
	}

	if ((ETAL_INVALID_HANDLE != vl_etalHandleCurrent) && (vl_etalHandleCurrent != vl_etalHandle))
	{
		// this should not happen : there is a different value for currenlty stored and handle
		//		
	}

	// update (may be not needed) the etalhandle.
	
	DABMW_ServiceFollowing_ExtInt_StoreEtalHandleFromApp(vl_etalHandle, vI_app);

	
	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_TuneFrequency : path = %d, frequency %d, etal handle = %d\n",
					vI_app, 
					vI_frequency,
					vl_etalHandle);

	// for FM : normalResponse
	// for DAB we need a response on 'when done'


	//If the receiver is a new one : set the band
	// 

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{

		// check if it is a reuse or a new receiver
		// ie if current = invalid, this is a new one.
		
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
    		proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;

			// set the receiver in correct band
			// get band of frequency
			//
		
			vl_SF_systemBand = DABMW_GetReceiverTableSystemBand(vl_etalHandle, vI_frequency, DABMW_COUNTRY_NONE);

			// get current receiver 
			//
			vl_etalFreqBand = DABMW_TranslateDabmwBandToEtalBand(vl_SF_systemBand);


			// if this is a new receiver : change the band.
			// Else, we assume it is the good one
			//
			if (ETAL_INVALID_HANDLE == vl_etalHandleCurrent)
			{
				// this is a new receiver
				//		
				if (ETAL_RET_SUCCESS != ETAL_changeBandInternal(vl_etalHandle, vl_etalFreqBand, 0, 0, proc_features, FALSE))
					{
						DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW_ServiceFollowing_ExtInt_TuneFrequency : etal_change_band_receiver FM ERROR, sf_band %d, etal_band = %d\n",
								vl_SF_systemBand, vl_etalFreqBand);
						
						ETAL_receiverReleaseLock(vl_etalHandle);
						
						return OSAL_ERROR;
					}
				else
					{
						DABMW_SF_PRINTF(TR_LEVEL_USER_4, "DABMW_ServiceFollowing_ExtInt_TuneFrequency : change band ok path = %d, frequency %d, etal handle = %d, sf_freq_band = %d, etal_freq_band = %d\n",
								vI_app, 
								vI_frequency,
								vl_etalHandle, 
								vl_SF_systemBand,
								vl_etalFreqBand);
					}
			}
		}
		
		if (true == vl_appIsDab)
		{
			// for now : normal response
			// but in the future must be different : do not wait xs for a tuen ;)
			//		vl_tuneAction = cmdTuneImmediateResponse;
			vl_tuneAction = cmdTuneDoNotWaitResponseDcopDirectResponseOnStatus;
		}
		else
		{
			vl_tuneAction = cmdTuneNormalResponse;
		}


		vl_etalStatus = ETAL_tuneReceiverInternal(vl_etalHandle, vI_frequency, vl_tuneAction);

		if (ETAL_RET_SUCCESS != vl_etalStatus)
		{
			vl_res = OSAL_ERROR;

			// in DAB, no data means timeout, ie tune error => no print

			
			if ((true == vl_appIsDab) && (ETAL_RET_NO_DATA == vl_etalStatus))
			{
				// in DAB it is not a Tune error, it is that there is a no sync
				vl_res = OSAL_OK;
			}
			else
			{
				DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW_ServiceFollowing_ExtInt_TuneFrequency : ETAL_tuneReceiverInternal ERROR, path = %d, frequency %d, etal handle = %d\n",
						vI_app, 
						vI_frequency,
						vl_etalHandle);
			}
			
			// update dab status
			/*if (true == vl_appIsDab)
			{
				DABMW_serviceFollowingStatus.dab_syncStatus	 = DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_NOTTUNED;
			}
			*/
		}
		else
		{
			DABMW_SF_PRINTF(TR_LEVEL_USER_4, "DABMW_ServiceFollowing_ExtInt_TuneFrequency : ETAL_tuneReceiverInternal ok, path = %d, frequency %d, etal handle = %d\n",
						vI_app, 
						vI_frequency,
						vl_etalHandle);
			// update dab status
			/*if (true == DABMW_ServiceFollowing_ExtInt_IsApplicationDab(vI_app))
			{
				DABMW_serviceFollowingStatus.dab_syncStatus	 = DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_TUNED;
			}
			*/
		}


		// if we are in DAB simulate the tune callback
		/*
		if (true == vl_appIsDab)
		{
			DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_DAB_TUNE);

			if (DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_TUNED == DABMW_serviceFollowingStatus.dab_syncStatus)
			{
				// if the result was ok, and simulate the fig available
				// this trigger the service list reading...
				// so wait some sec
				// oo
				OSAL_s32ThreadWait(DABMW_SF_DAB_WAIT_FIC_ACQUISITION_TIME_MS);
				
				DABMW_ServiceFollowing_ExtInt_TaskWakeUpOnEvent(DABMW_SF_EVENT_DAB_FIC_READ);
			}
		}
		*/

		ETAL_receiverReleaseLock(vl_etalHandle);		

	}
	else
	{
		//nothing
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end	
	
	return(vl_res);

}

EtalPathName DABMW_ServiceFollowing_ExtInt_GetEtalPath(DABMW_SF_mwAppTy vI_app)
{
	EtalPathName vl_etalPathName = ETAL_PATH_NAME_UNDEF;

	switch (vI_app)
	{
		case DABMW_MAIN_AUDIO_DAB_APP:
			vl_etalPathName = ETAL_PATH_NAME_DAB_1;
			break;
		case DABMW_SECONDARY_AUDIO_DAB_APP:
			vl_etalPathName = ETAL_PATH_NAME_DAB_2;
			break;
		case DABMW_MAIN_AMFM_APP:
			vl_etalPathName = ETAL_PATH_NAME_FM_FG;
			break;	
		case DABMW_BACKGROUND_AMFM_APP:
			vl_etalPathName = ETAL_PATH_NAME_FM_BG;
			break;
		default:
			vl_etalPathName = ETAL_PATH_NAME_UNDEF;
			break;
	}

	return (vl_etalPathName);
}

DABMW_SF_mwAppTy DABMW_ServiceFollowing_ExtInt_GetAppFromEtalPath(EtalPathName vI_etalPath)
{
	DABMW_SF_mwAppTy vl_app = DABMW_NONE_APP;

	switch (vI_etalPath)
	{
		case ETAL_PATH_NAME_DAB_1:
			vl_app = DABMW_MAIN_AUDIO_DAB_APP;
			break;
		case ETAL_PATH_NAME_DAB_2:
			vl_app = DABMW_SECONDARY_AUDIO_DAB_APP;
			break;
		case ETAL_PATH_NAME_FM_FG:
			vl_app = DABMW_MAIN_AMFM_APP;
			break;	
		case ETAL_PATH_NAME_FM_BG:
			vl_app = DABMW_BACKGROUND_AMFM_APP;
			break;
		default:
			vl_app = DABMW_NONE_APP;
			break;
	}

	return (vl_app);
}



#endif //  CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RADIO_C

