//!
//!  \file      service_following_external_audio.c
//!  \brief     <i><b> Service following implementation : external interface definition => audio part </b></i>
//!  \details   This file provides functionalities for external interface : audio part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!


#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_C

#include "osal.h"

#if defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) 

#include "etalinternal.h"

#include "dabmw_import.h"

// #include "audio_mngr.h"
//#include "sd_messages.h"
//#include "streamdec_comm.h"

#include "seamless_switching_common.h"

#include "Service_following_externalInterface_OSAL.h"
#include "Service_following_externalInterface_DABMWServices.h"

#if 0
/* inclusion here will not 'see' CONFIG_APP_SEAMLESS_SWITCHING_BLOCK thus causing build warnings
 * removed since it is already included by service_following_internal.h */
#include "Service_following_externalInterface_audio.h"
#endif

#include "service_following_internal.h"


#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessEstimationResponse(DABMW_SF_audio_msgsCallbackTy callback)
{

	tSInt vl_res = OSAL_OK;
	v_SF_Callback_SeamlessEstimationResponse = callback;

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessEstimationResponse : v_SF_Callback_SeamlessEstimationResponse = 0x%x\n", v_SF_Callback_SeamlessEstimationResponse);

	if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtSeamlessEstimationResponse, DABMW_ServiceFollowing_ExtInt_SeamlessEstimationResponseCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
		{
			
			DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "*DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessEstimationResponse : registration error\n");
			vl_res = OSAL_ERROR;
		}
	
	return(vl_res);
}

tVoid DABMW_ServiceFollowing_ExtInt_SeamlessEstimationResponseCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
	SD_msg_SeamlessEstimationResponse vl_SD_msg_SeamlessEstimationResponse; // for the SF callback
	EtalSeamlessEstimationStatus *vl_EtalMsg_SeamlessEstimationResponse = (EtalSeamlessEstimationStatus *) param;// entry point

	vl_SD_msg_SeamlessEstimationResponse.msg_id = SD_MESSAGE_TX_SEAMLESS_ESTIMATION;	
	vl_SD_msg_SeamlessEstimationResponse.status = (SD_SeamlessEstimationStatusTy) vl_EtalMsg_SeamlessEstimationResponse->m_status;					// Report status
	vl_SD_msg_SeamlessEstimationResponse.provider_type = (SD_SeamlessEstimationDataProviderTy) vl_EtalMsg_SeamlessEstimationResponse->m_providerType;  	// Audio reference for delay estimation
	vl_SD_msg_SeamlessEstimationResponse.absolute_estimated_delay_in_samples = vl_EtalMsg_SeamlessEstimationResponse->m_absoluteDelayEstimate;				// Absolute delay estimate in samples
	vl_SD_msg_SeamlessEstimationResponse.delay_estimate = vl_EtalMsg_SeamlessEstimationResponse->m_delayEstimate;									// Delay estimate in samples
	vl_SD_msg_SeamlessEstimationResponse.timestamp_FAS = vl_EtalMsg_SeamlessEstimationResponse->m_timestamp_FAS;										// Time stamp on FAS for the delay estimate
	vl_SD_msg_SeamlessEstimationResponse.timestamp_SAS = vl_EtalMsg_SeamlessEstimationResponse->m_timestamp_SAS;										// Time stamp on SAS for the delay estimate
	vl_SD_msg_SeamlessEstimationResponse.average_RMS2_FAS = vl_EtalMsg_SeamlessEstimationResponse->m_RMS2_FAS;									// Average of squared RMS on FAS
	vl_SD_msg_SeamlessEstimationResponse.average_RMS2_SAS = vl_EtalMsg_SeamlessEstimationResponse->m_RMS2_SAS;									// Average of squared RMS on SAS
	vl_SD_msg_SeamlessEstimationResponse.confidence_level = vl_EtalMsg_SeamlessEstimationResponse->m_confidenceLevel;									// Confidence level of delay estimate

	if (NULL != v_SF_Callback_SeamlessEstimationResponse)
		{
		v_SF_Callback_SeamlessEstimationResponse((void*) &vl_SD_msg_SeamlessEstimationResponse);
		}
}

tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessSwitchingResponse(DABMW_SF_audio_msgsCallbackTy callback)
{
	
	tSInt vl_res = OSAL_OK;
	v_SF_Callback_SeamlessSwitchingResponse = callback;

	if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtSeamlessSwitchingResponse, DABMW_ServiceFollowing_ExtInt_SeamlessSwitchingResponseCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
		{
			DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "*DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessSwitchingResponse : registration error\n");
			vl_res = OSAL_ERROR;
		}
	
	return(vl_res);
}

tVoid DABMW_ServiceFollowing_ExtInt_SeamlessSwitchingResponseCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{

	SD_msg_SeamlessSwitchingResponse vl_SD_msg_SeamlessSwitchingResponse; // for the SF callback
	EtalSeamlessSwitchingStatus *vl_EtalMsg_SeamlessSwitchingResponse = (EtalSeamlessSwitchingStatus *) param;// entry point					

	vl_SD_msg_SeamlessSwitchingResponse.msg_id = SD_MESSAGE_TX_SEAMLESS_SWITCHING;	
	vl_SD_msg_SeamlessSwitchingResponse.status = (SD_SeamlessSwitchingStatusTy) vl_EtalMsg_SeamlessSwitchingResponse->m_status;					// Report status
	vl_SD_msg_SeamlessSwitchingResponse.delay = vl_EtalMsg_SeamlessSwitchingResponse->m_absoluteDelayEstimate;									// Delay estimate in samples								// Average of squared RMS on SAS


	if (NULL != v_SF_Callback_SeamlessSwitchingResponse)
		{
		v_SF_Callback_SeamlessSwitchingResponse((void *) &vl_SD_msg_SeamlessSwitchingResponse);
		}
}

tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest(SF_msg_SeamlessEstimationRequest vI_seamlessEstimationReq)
{
	ETAL_STATUS ret;
	
	tSInt vl_res = OSAL_OK;
	etalSeamlessEstimationConfigTy vl_seamlessEstimationReq;
	
	vl_seamlessEstimationReq.mode = vI_seamlessEstimationReq.mode;
	vl_seamlessEstimationReq.startPosition = vI_seamlessEstimationReq.start_position_in_samples;
	vl_seamlessEstimationReq.stopPosition= vI_seamlessEstimationReq.stop_position_in_samples;
	
	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest :  Start seamless estimation: Mode=%d, Start=%d, Stop=%d\n",
				vl_seamlessEstimationReq.mode,
				vl_seamlessEstimationReq.startPosition,
				vl_seamlessEstimationReq.stopPosition);

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	// 
	// in cross case : we should check the validity of receiver after the lock in case it has been destroyed :)
	//
	
	if (ETAL_receiverGetLock(DABMW_serviceFollowingStatus.originalHandle) == ETAL_RET_SUCCESS)
	{

		if (true == ETAL_receiverIsValidHandle(DABMW_serviceFollowingStatus.originalHandle))
		{
			if ((ret = ETAL_seamless_estimation_start_internal(DABMW_serviceFollowingStatus.originalHandle, DABMW_serviceFollowingStatus.alternateHandle, &vl_seamlessEstimationReq)) != ETAL_RET_SUCCESS)
			{
				DABMW_SF_PRINTF(TR_LEVEL_ERRORS,"DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest :  ETAL_seamless_estimation_start_internal ERROR ret = %d\n", ret);
				vl_res = OSAL_ERROR;
			}
		}
		else
		{
			DABMW_SF_PRINTF(TR_LEVEL_ERRORS,"DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest :  ERROR invalid handle\n");
			vl_res = OSAL_ERROR;
		}
		
		ETAL_receiverReleaseLock(DABMW_serviceFollowingStatus.originalHandle);
	}
	else
	{
			DABMW_SF_PRINTF(TR_LEVEL_ERRORS,"DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest :  get lock error\n");
			vl_res = OSAL_ERROR;
	}
	//  Lock addition change end
	
	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest : sent\n");


	return(vl_res);
}

tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationStop()
{

	ETAL_STATUS ret;
	
	tSInt vl_res = OSAL_OK;


	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest :  stop seamless estimation \n");

	
	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if (ETAL_receiverGetLock(DABMW_serviceFollowingStatus.originalHandle) == ETAL_RET_SUCCESS)
	{
		if (true == ETAL_receiverIsValidHandle(DABMW_serviceFollowingStatus.originalHandle))
		{
			if ((ret = ETAL_seamless_estimation_stop_internal(DABMW_serviceFollowingStatus.originalHandle, DABMW_serviceFollowingStatus.alternateHandle)) != ETAL_RET_SUCCESS)
			{
				DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationStop :  ERROR ret = %d\n", ret);
				vl_res = OSAL_ERROR;
			}
		}
		else
		{
			vl_res = OSAL_ERROR;
		}
		
		ETAL_receiverReleaseLock(DABMW_serviceFollowingStatus.originalHandle);
			
	}
	else
	{
			vl_res = OSAL_ERROR;
	}
	//  Lock addition change end
	


	return(vl_res);
	

}

tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessSwitchingRequest(SF_msg_SeamlessSwitchingRequest vI_seamlessSwitchingReq)
{	

	ETAL_STATUS ret;
		
	tSInt vl_res = OSAL_OK;
	etalSeamlessSwitchingConfigTy vl_seamlessSwitchingReq;
	
	vl_seamlessSwitchingReq.systemToSwitch = vI_seamlessSwitchingReq.systemToSwitch;
	vl_seamlessSwitchingReq.providerType = vI_seamlessSwitchingReq.provider_type;
	vl_seamlessSwitchingReq.absoluteDelayEstimate = vI_seamlessSwitchingReq.absolute_estimated_delay_in_samples;
	vl_seamlessSwitchingReq.delayEstimate = vI_seamlessSwitchingReq.delay_estimate;
	vl_seamlessSwitchingReq.timestampFAS = vI_seamlessSwitchingReq.timestamp_FAS;
	vl_seamlessSwitchingReq.timestampSAS = vI_seamlessSwitchingReq.timestamp_SAS;
	vl_seamlessSwitchingReq.averageRMS2FAS = vI_seamlessSwitchingReq.average_RMS2_FAS;
	vl_seamlessSwitchingReq.averageRMS2SAS = vI_seamlessSwitchingReq.average_RMS2_SAS;

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "* DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessSwitchingRequest: Seamless switching: System to switch=%d, Provider type=%d, Absolute delay estimate=%d, Delay estimate=%d, TS FAS=%d, TS SAS=%d, Avg RMS2 FAS=%u, Avg RMS2 SAS=%u\n",
				vl_seamlessSwitchingReq.systemToSwitch,
				vl_seamlessSwitchingReq.providerType,
				vl_seamlessSwitchingReq.absoluteDelayEstimate,
				vl_seamlessSwitchingReq.delayEstimate,
				vl_seamlessSwitchingReq.timestampFAS,
				vl_seamlessSwitchingReq.timestampSAS,
				vl_seamlessSwitchingReq.averageRMS2FAS,
				vl_seamlessSwitchingReq.averageRMS2SAS);

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(DABMW_serviceFollowingStatus.originalHandle) == ETAL_RET_SUCCESS)
	{
		if (true == ETAL_receiverIsValidHandle(DABMW_serviceFollowingStatus.originalHandle))
		{	
			if ((ret = ETAL_seamless_switching_internal(DABMW_serviceFollowingStatus.originalHandle, DABMW_serviceFollowingStatus.alternateHandle,&vl_seamlessSwitchingReq)) != ETAL_RET_SUCCESS)
			{
				DABMW_SF_PRINTF(TR_LEVEL_ERRORS,"DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessSwitchingRequest : ETAL_cmdSeamlessSwitching_MDR ERROR %d\n", ret);
				vl_res = OSAL_ERROR;
			}
		}
		else
		{
			vl_res = OSAL_ERROR;
		}

		// unlock receiver
		ETAL_receiverReleaseLock(DABMW_serviceFollowingStatus.originalHandle);
	}
	else
	{
			vl_res = OSAL_ERROR;
	}
	//	Lock addition change end

	
	return(vl_res);
		

}

#endif // #ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

// Local function for audio port switch path tracking
tSInt DABMW_ServiceFollowing_ExtInt_SetCurrentAudioPortUser (DABMW_SF_mwAppTy app, tBool isInternalTune)
{
    tSInt ret = OSAL_OK;

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(DABMW_serviceFollowingStatus.originalHandle) == ETAL_RET_SUCCESS)
	{
		if (true == ETAL_receiverIsValidHandle(DABMW_serviceFollowingStatus.originalHandle))
		{
			// for now, set the DAB as source for the ss demo
		  	if ((app == DABMW_MAIN_AUDIO_DAB_APP) || (app == DABMW_SECONDARY_AUDIO_DAB_APP))
		  	{
				ETAL_audioSelectInternal(DABMW_serviceFollowingStatus.originalHandle, ETAL_AUDIO_SOURCE_DCOP_STA660);
			  	v_ServiceFollowing_ExtInt_currentAudioApp = app;
			}
			else if (app == DABMW_MAIN_AMFM_APP)
			{
				ETAL_audioSelectInternal(DABMW_serviceFollowingStatus.originalHandle, ETAL_AUDIO_SOURCE_STAR_AMFM);
			  	v_ServiceFollowing_ExtInt_currentAudioApp = app;
			}
			else
			{
				ret = OSAL_ERROR;
			}
		}
		else
		{
			ret = OSAL_ERROR;
		}
		
		// unlock receiver
		ETAL_receiverReleaseLock(DABMW_serviceFollowingStatus.originalHandle);
	}
	else
	{
		ret = OSAL_ERROR;
	}
	//	Lock addition change end



  // return (DABMW_SetCurrentAudioPortUser(app, isInternalTune));
    return ret;
}

DABMW_SF_mwAppTy DABMW_ServiceFollowing_ExtInt_GetCurrentAudioPortUser (tVoid)
{
	return(v_ServiceFollowing_ExtInt_currentAudioApp);
	
    // return (DABMW_GetCurrentAudioPortUser());
}

#endif // defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) 


#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_C



