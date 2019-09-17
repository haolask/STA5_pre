//!
//!  \file      service_following_external_FM.c
//!  \brief     <i><b> Service following implementation : external interface definition => FM part </b></i>
//!  \details   This file provides functionalities for external interface : FM part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_C


#include "osal.h"
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"

#include "rds_data.h"

#include "Service_following_externalInterface_OSAL.h"
#include "Service_following_externalInterface_DABMWServices.h"
#include "Service_following_externalInterface_radio.h"


#include "Service_following_externalInterface_FM.h"


// AF Switch API handling
// This commands is used for smooth switch to a new frequency.
//This command performs a tuning operation to an AF-frequency and AM/FM processing path. 

tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFSwitchRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency)
{
	// not available for now
	tSInt vl_res = OSAL_OK; 
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
		
	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_AmFmAFSwitchRequest :  path = %d, etalhandle %d, frequency = %d\n",
							vI_app,
							vl_etalHandle,
							vI_frequency);
	
			

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			vl_res =  ETAL_AF_switch(vl_etalHandle, vI_frequency);
		}
		else
		{
			vl_res = OSAL_ERROR;
		}
		
		ETAL_receiverReleaseLock(vl_etalHandle);
				
	}
	else
	{
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end
					
	

	// not available for now

    return (vl_res);
}


// autoseek callback
tVoid DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{

	EtalSeekStatus *pl_autoSeekStatus = (EtalSeekStatus *) param;
	tU32 vl_frequency = DABMW_INVALID_FREQUENCY;


	if (NULL != pl_autoSeekStatus)
	{
		// change in AUTO SEEK STATUS
		// for case used by SF (unmute) the seek result is received
		// we should filter only those corresponding to the real seek ended
		// freq found or full cycle
		//
        if ((pl_autoSeekStatus->m_frequencyFound != true) && (pl_autoSeekStatus->m_fullCycleReached != true))
        {
        	// intermediate result we do not want to proceed
        	//
			return;
        }
		
		vl_frequency = pl_autoSeekStatus->m_frequency;
		// store the autoseek result information
		v_DABMW_SF_FM_AutoSeek_Result = *pl_autoSeekStatus;	
	}

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekCallback :   etalhandle %d, freq = %d, autoseek callback = %p\n",
							hGeneric, pl_autoSeekStatus->m_frequency, v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_DABMW_SF_FM_AutoSeek_Callback);

		
	if (hGeneric == v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_handle)
		{
		// DABMW_SetApplication (v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_app, DABMW_INVALID_EID, vI_frequency, DABMW_IDENTIFIER_UNKNOWN, (tPU8)NULL);     
		
		if (NULL != v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_DABMW_SF_FM_AutoSeek_Callback)
			{		
			v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_DABMW_SF_FM_AutoSeek_Callback(v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_app, vl_frequency, (tPVoid)(tULong) context);
			}
		}
	
	return;
}

tSInt DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekStart(DABMW_SF_mwAppTy vI_app, tS16 deltaFreq, DABMW_SF_SeekCallbackTy pI_callBack )
{
	
	tSInt vl_res = OSAL_OK;

	ETAL_HANDLE vl_etalHandleCurrent = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	ETAL_HANDLE vl_etalHandle;

	ETALTML_getReceiverForPathInternal(&vl_etalHandle, DABMW_ServiceFollowing_ExtInt_GetEtalPath(vI_app), NULL, true);

	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no hanlde found. return error
		// 
		return(OSAL_ERROR);
	}

	
	if ((vl_etalHandleCurrent != ETAL_INVALID_HANDLE) && (vl_etalHandleCurrent != vl_etalHandle))
	{
		// this should not happen : there is a different value for currenlty stored and handle
		//		
	}

	// update (may be not needed) the etalhandle.

	DABMW_ServiceFollowing_ExtInt_StoreEtalHandleFromApp(vl_etalHandle, vI_app);


#define DABMW_SF_AUTOSEEK_STEP				100
#define DABMW_SF_AUTOSEEK_BUFFER_LEN		4
	

	/* this is a seek start */


	/*Parameter 1:	Name	Seek channel
		Bit [1:0]	0x01: Seek on foreground channel
			0x02: Seek on background channel
	Parameter 2:	Name	Configuration
		Bit[0]	0: Manual mode (default setting)
			1: Automatic mode
		Bit [1] 0: Seek up (default)
			1: Seek down 
		Bit [2] 0: Unmute and exit auto seek  (default) - valid only for automatic mode
			1: Stay muted in auto seek and wait either for Seek_End or next Seek_Start (valid only for automatic mode)
		Bit [3] 0: Seek step [kHz] is applied 
			1: Seek is performed on discrete frequency specified in parameter 3
		Bit [7:4]	Number of additional measurements for averaging (maximally 15)
		Bit [8] 0: Seek Stop frequency is set to current frequency
			1: Seek Stop frequency is not updated (Continue Scan Flag)
	*/


	v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_DABMW_SF_FM_AutoSeek_Callback = pI_callBack;
	v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_app = vI_app;
	v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_handle = vl_etalHandle;

	// reset the autoseek result
	v_DABMW_SF_FM_AutoSeek_Result.m_frequency = DABMW_INVALID_FREQUENCY;
	
	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{

		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			// check the receiver is not busy
			if (false == ETAL_receiverIsSpecialInProgress(vl_etalHandle, cmdSpecialAnyChangingFrequency))
			{
				if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtSeekStatus, DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
				{
					vl_res = OSAL_ERROR;
				}
				
				if (ETAL_RET_SUCCESS != ETAL_intCbRegister(callAtSeekStatus, DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekCallback, ETAL_INVALID_HANDLE, ETAL_INTCB_CONTEXT_UNUSED))
				{
					vl_res = OSAL_ERROR;
				}
					
				vl_res = ETALTML_seek_start_internal(vl_etalHandle, cmdDirectionUp, deltaFreq, cmdAudioMuted, dontSeekInSPS, TRUE);



				DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekStart :  path = %d, etalhandle %d, res = %d\n",
									vI_app,
									vl_etalHandle,
									vl_res);
			}
			else
			{
				vl_res = OSAL_ERROR;
			}
		}
		else
		{
			vl_res = OSAL_ERROR;
		}
		
		ETAL_receiverReleaseLock(vl_etalHandle);
				
	}
	else
	{
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end

    return vl_res;

}

tSInt DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekContinue(DABMW_SF_mwAppTy vI_app)
{
		tSInt vl_res = OSAL_ERROR; 
		ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);		

		DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekContinue :  path = %d, etalhandle %d\n",
							vI_app,
							vl_etalHandle);

	 	if (ETAL_INVALID_HANDLE == vl_etalHandle)
		{
			// no hanlde found. return error
			// 
			return(OSAL_ERROR);
		}
		
		// reset the autoseek result
		v_DABMW_SF_FM_AutoSeek_Result.m_frequency = DABMW_INVALID_FREQUENCY;

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{	

		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			// this is a seek internal with specific unused params
			vl_res = ETALTML_seek_continue_internal(vl_etalHandle);
		}
		else
		{
			vl_res = OSAL_ERROR;
		}
		
			ETAL_receiverReleaseLock(vl_etalHandle);
					
	}
	else
	{
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end

	return vl_res;	
}

tSInt DABMW_ServiceFollowing_ExtInt_AmFmSeekEnd(DABMW_SF_mwAppTy vI_app)
{

	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	tSInt vl_res = OSAL_ERROR; 
	
	// reset the callback
	
	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_AmFmSeekEnd :  path = %d, etalhandle %d\n",
						vI_app,
						vl_etalHandle);


	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no hanlde found. return error
		// 
		return(OSAL_ERROR);
	}

	v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_DABMW_SF_FM_AutoSeek_Callback = NULL;
	v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_app = DABMW_NONE_APP;
	v_DABMW_SF_FM_AutoSeek_CallbackInfo.v_handle = ETAL_INVALID_HANDLE;


	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{

		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
   			vl_res = ETALTML_seek_stop_internal(vl_etalHandle, lastFrequency);
		}
		else
		{
			vl_res = OSAL_ERROR;
		}

		ETAL_receiverReleaseLock(vl_etalHandle);
					
	}
	else
	{
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end

	return(vl_res);
}


tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality)
{
#define DABMW_SF_ETAL_AF_CHECK_ANTENNA_SELECTION_AUTO 0

	// not available for now
	tSInt vl_res = OSAL_OK; 
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	EtalBcastQualityContainer vl_etalFMQuality;
	DABMW_SF_amfmQualityTy vl_SF_amFmQuality;

	//tU32 vl_combinedQualityIndex;
	//tU32 vl_resp; // in return of cmdReadCMOST, this will point on the CMOS response , buffer being a global vairable in ETAL...
	//tU16 vl_respLen;
//	ETAL_HANDLE vl_hTuner;

	vl_SF_amFmQuality = DABMW_ServiceFollowing_ExtInt_AmFmQualityInit(); 

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest :  path = %d, etalhandle %d, frequency = %d\n",
						vI_app,
						vl_etalHandle,
						vI_frequency);


	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no handle found : return
		return OSAL_ERROR;
	}
	
//	vl_res =  ETAL_cmdAFCheck_CMOST(vl_etalHandle, vI_frequency, DABMW_SF_ETAL_AF_CHECK_ANTENNA_SELECTION_AUTO, &vl_etalFMQuality);

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			vl_res =  ETAL_AF_check_and_get_AF_quality(vl_etalHandle, vI_frequency, DABMW_SF_ETAL_AF_CHECK_ANTENNA_SELECTION_AUTO, &vl_etalFMQuality);

			if (ETAL_RET_SUCCESS != vl_res)
			{
				DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest : returns erros %d\n",
						vl_res);
				vl_res = OSAL_ERROR;

			}
		}
		else
		{
			vl_res = OSAL_ERROR;
		}
	

		if (OSAL_ERROR != vl_res)
		{
			// Retrieved quality parameters	
			vl_SF_amFmQuality.multipath = vl_etalFMQuality.EtalQualityEntries.amfm.m_Multipath ;
			vl_SF_amFmQuality.adjacentChannel = vl_etalFMQuality.EtalQualityEntries.amfm.m_UltrasonicNoise ;
			vl_SF_amFmQuality.deviation = vl_etalFMQuality.EtalQualityEntries.amfm.m_ModulationDetector;
			//vl_SF_amFmQuality.fieldStrength = vl_etalFMQuality.EtalQualityEntries.amfm ;
			vl_SF_amFmQuality.detuning = vl_etalFMQuality.EtalQualityEntries.amfm.m_FrequencyOffset ;
			vl_SF_amFmQuality.fieldStrength_dBuV = vl_etalFMQuality.EtalQualityEntries.amfm.m_BBFieldStrength;
		}
		
#if 0 				
					// for now combinedQ invalid
					// put the code as invalid
		//
		// get the combined Q 

		if (DABMW_MAIN_AMFM_APP == vI_app)
		{
			vl_combinedQualityIndex = IDX_CMT_tunApp0_fm_qdAf_quality;
		}
		else
		{
			vl_combinedQualityIndex = IDX_CMT_tunApp1_fm_qdAf_quality;
		}


		ETAL_receiverGetTunerId(vl_etalHandle, &vl_hTuner);
			
		if (ETAL_RET_SUCCESS == ETAL_read_parameter_internal(vl_hTuner, fromIndex, &vl_combinedQualityIndex, 1, &vl_resp, &vl_respLen))
		{
			vl_SF_amFmQuality.combinedQ = (tU16) (vl_resp & 0x0000FFFF);
		}
#endif // 0 for combined Q


		*pO_quality = vl_SF_amFmQuality;

		
		ETAL_receiverReleaseLock(vl_etalHandle);		

	}
	else
	{
		//vl_combinedQualityIndex = IDX_CMT_tunApp1_fm_qdAf_quality;
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end

    return (vl_res);


}

tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFStartRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality)
{
	// not available for now

    return (OSAL_OK);


}

tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFEndRequest(DABMW_SF_mwAppTy vI_app)
{
	// not available for now

    return (OSAL_OK);


}


/* Mute/unmute Function
*/
tSInt DABMW_ServiceFollowing_ExtInt_FM_MuteUnmute(DABMW_SF_mwAppTy vI_app, tBool vI_muteRequested)
{

	tSInt vl_res = OSAL_OK;
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	tBool vl_muteAction_Cmost;


    
    vl_muteAction_Cmost = (true == vI_muteRequested)? SF_EXT_CMOST_MUTE : SF_EXT_CMOST_UNMUTE;


	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no hanlde found. return error
		// 
		return(OSAL_ERROR);
	}

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{

		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			if (ETAL_RET_SUCCESS != ETAL_cmdAudioMute_CMOST(vl_etalHandle, vl_muteAction_Cmost))
				{
				vl_res = OSAL_ERROR;
				}
		}
		else
		{
			vl_res = OSAL_ERROR;
		}
		
		ETAL_receiverReleaseLock(vl_etalHandle);		

	}
	else
	{
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end
	
    return (vl_res);
}


DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_ExtInt_AmFmGetQuality(DABMW_SF_mwAppTy vI_app) 
{

	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	DABMW_SF_amfmQualityTy vl_SF_amFmQuality = DABMW_ServiceFollowing_ExtInt_AmFmQualityInit(); 
	EtalBcastQualityContainer vl_etalFMQuality;
		// for now combinedQ invalid
		// put the code as invalid
#if 0 

	tU32 vl_resp; // in return of cmdReadCMOST, this will point on the CMOS response , buffer being a global vairable in ETAL...
	tU16 vl_respLen;

	tU32 vl_combinedQualityIndex;
#endif // combined Q not used.


	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no hanlde found. return error
		// 
		return(vl_SF_amFmQuality);
	}


	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{	
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{		
			if (OSAL_OK == ETAL_cmdGetReceptionQuality_CMOST(vl_etalHandle, &vl_etalFMQuality))
			{
			    // Retrieved quality parameters
			   
			    vl_SF_amFmQuality.multipath = vl_etalFMQuality.EtalQualityEntries.amfm.m_Multipath ;
			    vl_SF_amFmQuality.adjacentChannel = vl_etalFMQuality.EtalQualityEntries.amfm.m_UltrasonicNoise ;
			    vl_SF_amFmQuality.deviation = vl_etalFMQuality.EtalQualityEntries.amfm.m_ModulationDetector;
			    //vl_SF_amFmQuality.fieldStrength = vl_etalFMQuality.EtalQualityEntries.amfm ;
			    vl_SF_amFmQuality.detuning = vl_etalFMQuality.EtalQualityEntries.amfm.m_FrequencyOffset ;
			    vl_SF_amFmQuality.fieldStrength_dBuV = vl_etalFMQuality.EtalQualityEntries.amfm.m_BBFieldStrength;

					// for now combinedQ invalid
					// put the code as invalid
#if 0 	

				if (DABMW_MAIN_AMFM_APP == vI_app)
				{
					vl_combinedQualityIndex = IDX_CMT_tunApp0_fm_qd_quality;
				}
				else
				{
					vl_combinedQualityIndex = IDX_CMT_tunApp1_fm_qd_quality;
				}


				ETAL_receiverGetTunerId(vl_etalHandle, &vl_hTuner);
							
				if (ETAL_RET_SUCCESS == ETAL_read_parameter_internal(vl_hTuner, fromIndex, &vl_combinedQualityIndex, 1, &vl_resp, &vl_respLen))
				{
					vl_SF_amFmQuality.combinedQ =  (vl_resp & 0x00FFFFFF);
				}
				else
				{
					DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
		                            "DABMW_ServiceFollowing_ExtInt_AmFmGetQuality : ERROR in Get Combined Quality\n");


				}
#endif		// if 0 combined Q not yet valid

			}
			else
			{		
			  	DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
		                            "DABMW_ServiceFollowing_ExtInt_AmFmGetQuality : ERROR in Get Quality\n");
			}
		}
		else
		{
			DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
		                            "DABMW_ServiceFollowing_ExtInt_AmFmGetQuality : ERROR in receiver = %d\n",vl_etalHandle);
		}

		ETAL_receiverReleaseLock(vl_etalHandle);		

	}
	else
	{
		//nothing
	}
	//	Lock addition change end
	
	return(vl_SF_amFmQuality);

}

DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_ExtInt_AmFmQualityInit()
{
    DABMW_SF_amfmQualityTy vl_fmQuality;  

    vl_fmQuality.detuning = DABMW_INVALID_DATA_U16;
    vl_fmQuality.adjacentChannel = DABMW_INVALID_DATA_U16;
    vl_fmQuality.multipath = DABMW_INVALID_DATA_U16;
    vl_fmQuality.combinedQ = DABMW_INVALID_DATA;
    vl_fmQuality.deviation = DABMW_INVALID_DATA_U16; // deviation not application for seek
    vl_fmQuality.fieldStrength_dBuV = DABMW_INVALID_DATA_S16;

    return vl_fmQuality;

}


tBool DABMW_ServiceFollowing_ExtInt_IsVpaEnabled()
{
	// not available for now
	

	return(false);
}

tSInt DABMW_ServiceFollowing_ExtInt_EnableVPA()
{
		// not available for now
	
	return (OSAL_ERROR);
}

tSInt DABMW_ServiceFollowing_ExtInt_DisableVPA()
{
		// not available for now
	
	return (OSAL_ERROR);

}


tSInt DABWM_ServiceFollowing_ExtInt_SetAmFmAutoSeekThreshold(DABMW_SF_mwAppTy vI_app, 
                                            tU16 vI_fieldStrenghThreshold_dBuV,
                                            tU16 vI_adjacentChannel_Threshold,
                                            tU16 vI_detuningChannel_Threshold,
                                            tU16 vI_multipath_Threshold,
                                            tU16 vI_combinedQuality_Threshold, 
                                            tU16 vI_snr_Threshold,
                                            tU16 vI_MpxNoiseThreshold)

{

	// TMP 
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	tSInt vl_res = OSAL_OK;
	EtalSeekThreshold vl_etalSeekThreshold;


	DABMW_SF_PRINTF( TR_LEVEL_COMPONENT, "DABWM_ServiceFollowing_ExtInt_SetAmFmAutoSeekThreshold : app %d, handle %d, BB FS %d (dBuV), Adjacent %d (percent), Detune %d  (percent), mpth %d (percent) \n",
			vI_app, vl_etalHandle, vI_fieldStrenghThreshold_dBuV, vI_adjacentChannel_Threshold,vI_detuningChannel_Threshold, vI_multipath_Threshold);


	vl_etalSeekThreshold.SeekThresholdBBFieldStrength = vI_fieldStrenghThreshold_dBuV;
	vl_etalSeekThreshold.SeekThresholdAdjacentChannel = vI_adjacentChannel_Threshold;
	vl_etalSeekThreshold.SeekThresholdDetune = vI_detuningChannel_Threshold;
	vl_etalSeekThreshold.SeekThresholdMultipath = vI_multipath_Threshold;

	// update the threshold....
	// specific to CMOST
	vl_etalSeekThreshold.SeekThresholdSignalNoiseRatio = vI_snr_Threshold;
	vl_etalSeekThreshold.SeekThresholdMpxNoise = vI_MpxNoiseThreshold;

	// on ETAL, the ALL application is not valid, the threshold will be set at procedure init app by app, so remove the 'all_application' is not an error
	if (DABMW_ALL_APPLICATIONS == vI_app)
	{
		return(OSAL_ERROR);
	}
	else if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		// no hanlde found. return error
		// 
		DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
                            "DABWM_ServiceFollowing_ExtInt_SetAmFmAutoSeekThreshold : ERROR invalid receiver \n");
		return(OSAL_ERROR);
	}

	// for the now, use default threshold in CMOST
	// 
#if 0
	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{	
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{

			if (ETAL_RET_SUCCESS != ETAL_cmdSetSeekThreshold_CMOST(vl_etalHandle, &vl_etalSeekThreshold))
				{
					vl_res = OSAL_ERROR;

					DABMW_SF_PRINTF( TR_LEVEL_ERRORS, 
		                            "DABWM_ServiceFollowing_ExtInt_SetAmFmAutoSeekThreshold : ERROR in set seek threshold \n");
				
				
				}
		}
		else
		{
			vl_res = OSAL_ERROR;
		}


		ETAL_receiverReleaseLock(vl_etalHandle);		


	}
	else
	{
		//nothing
		vl_res = OSAL_ERROR;
	}
	//	Lock addition change end
#else
	(tVoid) vl_etalSeekThreshold;
#endif

	return(vl_res);


}

tSInt DABMW_ServiceFollowing_ExtInt_SetAutoSeekData (DABMW_SF_mwAppTy app, tU32 startFrequency, tU32 stopFrequency, tBool directionIsUp, tU32 min_freq_band, tU32 max_freq_band)
{
	// nothing specific to do for now
	return(OSAL_OK);
	
}

tU32 DABMW_ServiceFollowing_ExtInt_GetSeekFrequency(DABMW_SF_mwAppTy vI_app)
{
	// we may get it from ETAL 
	// or in stored info
	tU32 vl_frequency = DABMW_INVALID_FREQUENCY;
	
	if (DABMW_INVALID_FREQUENCY != v_DABMW_SF_FM_AutoSeek_Result.m_frequency)
	{
		
		vl_frequency = v_DABMW_SF_FM_AutoSeek_Result.m_frequency;
	}
	
	return(vl_frequency);
}


tBool DABMW_ServiceFollowing_ExtInt_SeekEndedOnGoodFreq(DABMW_SF_mwAppTy vI_app)
{
	tBool vl_res = false;

	if (DABMW_INVALID_FREQUENCY != v_DABMW_SF_FM_AutoSeek_Result.m_frequency)
	{
		
		vl_res = v_DABMW_SF_FM_AutoSeek_Result.m_frequencyFound;
	}
	

	return(vl_res);
}

tBool DABMW_ServiceFollowing_ExtInt_SeekFullCycleDone(DABMW_SF_mwAppTy vI_app)
{
	tBool vl_res = false;

	if (DABMW_INVALID_FREQUENCY != v_DABMW_SF_FM_AutoSeek_Result.m_frequency)
	{
		
		vl_res = v_DABMW_SF_FM_AutoSeek_Result.m_fullCycleReached;
	}
	

	return(vl_res);

}

DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_ExtInt_GetSeekQuality(DABMW_SF_mwAppTy vI_app)
{
	DABMW_SF_amfmQualityTy vl_SF_amFmQuality = DABMW_ServiceFollowing_ExtInt_AmFmQualityInit(); 
	// for now combinedQ invalid
	// put the code as invalid
#if 0 
	tU32 vl_resp; // in return of cmdReadCMOST, this will point on the CMOS response , buffer being a global vairable in ETAL...
	tU16 vl_respLen;
	tU32 vl_combinedQualityIndex;
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
#endif // combined Q not yet valid // if 0

	if (DABMW_INVALID_FREQUENCY != v_DABMW_SF_FM_AutoSeek_Result.m_frequency)
	{ 
			vl_SF_amFmQuality.multipath = v_DABMW_SF_FM_AutoSeek_Result.m_quality.EtalQualityEntries.amfm.m_Multipath ;
			vl_SF_amFmQuality.adjacentChannel = v_DABMW_SF_FM_AutoSeek_Result.m_quality.EtalQualityEntries.amfm.m_UltrasonicNoise ;
			vl_SF_amFmQuality.deviation = v_DABMW_SF_FM_AutoSeek_Result.m_quality.EtalQualityEntries.amfm.m_ModulationDetector;
			//vl_SF_amFmQuality.fieldStrength = v_DABMW_SF_FM_AutoSeek_Result.m_quality.EtalQualityEntries.amfm ;
			vl_SF_amFmQuality.detuning = v_DABMW_SF_FM_AutoSeek_Result.m_quality.EtalQualityEntries.amfm.m_FrequencyOffset ;
			vl_SF_amFmQuality.fieldStrength_dBuV = v_DABMW_SF_FM_AutoSeek_Result.m_quality.EtalQualityEntries.amfm.m_BBFieldStrength;
	}

	// for now combinedQ invalid
	// put the code as invalid
#if 0 	
	// get the combined Q 	

	if (DABMW_MAIN_AMFM_APP == vI_app)
	{
		vl_combinedQualityIndex = IDX_CMT_tunApp0_fm_qd_quality;
	}
	else
	{
		vl_combinedQualityIndex = IDX_CMT_tunApp1_fm_qd_quality;
	}

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{	
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{

			ETAL_receiverGetTunerId(vl_etalHandle, &vl_hTuner);

			if (ETAL_RET_SUCCESS == ETAL_read_parameter_internal(vl_hTuner, fromIndex, &vl_combinedQualityIndex, 1, &vl_resp, &vl_respLen))
			{
				vl_SF_amFmQuality.combinedQ = (tU16) (vl_resp & 0x0000FFFF);
			}
		}
		else
		{
			// nothing, it is an error in cross case
		}

		ETAL_receiverReleaseLock(vl_etalHandle);		

	}
	else
	{
		
	}
	//	Lock addition change end
#endif 	// combined Q not yet valid // if 0

	return( vl_SF_amFmQuality);


}


tSInt DABMW_ServiceFollowing_ExtInt_SetDISSModeDSP(DABMW_mwAppTy vI_app, tU8 vI_DISSMode)
{
	// for now, not needed on STAR a priori
	// return(DABMW_SetDISSModeDSP(vI_app, vI_DISSMode));
	return(OSAL_OK);

	

}

tSInt DABMW_ServiceFollowing_ExtInt_SetDISSFilterDSP(DABMW_mwAppTy vI_app, tU32 vI_DISSFilter)
{

	// for now, not needed on STAR a priori
	// return(DABMW_SetDISSFilterDSP(vI_app, vI_DISSFilter));
	return(OSAL_OK);
}

#endif // CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_C

