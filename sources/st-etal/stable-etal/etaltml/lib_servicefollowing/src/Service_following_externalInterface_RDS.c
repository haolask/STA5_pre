//!
//!  \file      service_following_external_RDS.c
//!  \brief     <i><b> Service following implementation : external interface definition => RDS</b></i>
//!  \details   This file provides functionalities for external interface : RDS part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!


#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RDS_C
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RDS_C

#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#include "etalinternal.h"

#include "dabmw_import.h"

#include "service_following_internal.h"


#if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
    #include "rds_data.h"
    #include "rds_landscape.h"
#endif // #if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)


#include "Service_following_externalInterface_DABMWServices.h"
#include "Service_following_externalInterface_OSAL.h"

#include "Service_following_externalInterface_RDS.h"



tBool DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_SF_AmFmRdsCallbackTy callback, tBool reuse)
{
	return(DABMW_AmFmLandscapeRegisterForPiAtFreq(frequency, paramPtr, callback, reuse));
}

tBool DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPsAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_SF_AmFmRdsCallbackTy callback, tBool reuse)
{
	return(DABMW_AmFmLandscapeRegisterForPsAtFreq(frequency, paramPtr, callback, reuse));
}

tBool DABMW_ServiceFollowing_ExtInt_AmFmLandscapeDeRegister (DABMW_SF_AmFmRdsCallbackTy callback, tU32 frequency)
{
	return(DABMW_AmFmLandscapeDeRegister(callback, frequency));
}


tU32 DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPiFromFreq (tU32 vI_freq)
{

	// in case of Lock Feature
	// the request is to bypass the PI check
	// therefore, if no PI available in landscape, simulate one
	// 
	tU32 vl_PiValue = DABMW_INVALID_RDS_PI_VALUE;

	vl_PiValue = DABMW_AmFmLandscapeGetPiFromFreq(vI_freq);

	
	// Specific handling in Lock Feature case : 
	// for FM avoid / bypass the PI check
	// 
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
			
	// if feature is locked on 2 frequencies...
	// and the SID to compare is on FM
	// consider the SID is ok.	   
	if (true == DABMW_serviceFollowingData.lockFeatureActivated_2Freq)
	{
		if (DABMW_INVALID_RDS_PI_VALUE == vl_PiValue)
		{
		vl_PiValue = DABMW_SF_SIMULATE_VALID_PI;
		}
	}	
#endif
	
	
	return(vl_PiValue);

}

tU32 DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetValidPiFromFreq(tU32 vI_freq, SF_tMSecond vI_piValidityDuration)
{
	// in case of Lock Feature
	// the request is to bypass the PI check
	// therefore, if no PI available in landscape, simulate one
	// 

   	tU32 vl_PiValue = DABMW_INVALID_RDS_PI_VALUE;	

	vl_PiValue = DABMW_AmFmLandscapeGetValidPiFromFreq(vI_freq, vI_piValidityDuration);

	// Specific handling in Lock Feature case : 
	// for FM avoid / bypass the PI check
	// 
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
    
    // if feature is locked on 2 frequencies...
    // and the SID to compare is on FM
    // consider the SID is ok.     
    if (true == DABMW_serviceFollowingData.lockFeatureActivated_2Freq)
    {
    	if (DABMW_INVALID_RDS_PI_VALUE == vl_PiValue)
		{
		vl_PiValue = DABMW_SF_SIMULATE_VALID_PI;
		}
    }

#endif
	
	return(vl_PiValue);
}

tSInt DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(tU32 piValue)
{
	return(DABMW_AmFmLandscapeSearchForPI_GetNumber(piValue));
}

tSInt DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList (tPU32 dstPtr, tU32 piValue, tU8 vI_ptrLen)
{
	return(DABMW_AmFmLandscapeSearchForPI_FreqList(dstPtr, piValue, vI_ptrLen));
}

tPU8 DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPsFromFreq (tU32 vI_freq)
{
	// in case of Lock Feature
	// the request is to bypass the PI check
	// therefore, if no PI available in landscape, simulate one
	// 

	tPU8 pl_PsVlaue = NULL;	
	
	pl_PsVlaue = DABMW_AmFmLandscapeGetPsFromFreq(vI_freq);
	
		
	return(pl_PsVlaue);

}

tSInt DABMW_ServiceFollowing_ExtInt_RdsGetAfList (tPU8 pO_AFList_dataPtr, tU32 vI_piVal, tU32 vI_referenceFreqForAf, tU32 vI_maxNumberRetrieved)
{

	return(DABMW_RdsGetAfList(ETAL_INVALID_HANDLE /*not used with that mode */, pO_AFList_dataPtr, true, vI_piVal, vI_referenceFreqForAf, DABMW_GET_AF_CURRENTSTATUSMODE, vI_maxNumberRetrieved));
}


tSInt DABMW_ServiceFollowing_ExtInt_RdsDataSetupPiDetectionMode (DABMW_SF_mwAppTy vI_app, tBool vI_Isfast_PI_detection)
{
	ETAL_HANDLE vl_etalHandle;

	// Find the handle, which is the entry parameter to RdsDataSetup
	// 
	// for now, based on matching app to handle .
	// could be replaced by : GetHandleFromApp
	//

	vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

	DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_RdsDataSetupPiDetectionMode :  path = %d, fast detection %d, etalhandle %d\n",
						vI_app,
						vI_Isfast_PI_detection,
						vl_etalHandle);

	// handle not valid : nothing to do
	if (ETAL_INVALID_HANDLE == vl_etalHandle)
	{
		return OSAL_OK;
	}
	
	return(DABMW_RdsDataSetupPiDetectionMode(vl_etalHandle, vI_Isfast_PI_detection));
}


tSInt DABMW_ServiceFollowing_ExtInt_RdsEnable (DABMW_SF_mwAppTy vI_app, tBool vI_Isfast_PI_detection)
{
#define SF_NB_BLOCKS_FOR_WAKEUP_FAST_PI	1
//#define SF_NB_BLOCKS_FOR_WAKEUP_NORMAL	ETAL_RDS_MAX_BLOCK_IN_BUFFER
#define SF_NB_BLOCKS_FOR_WAKEUP_NORMAL	16

		ETAL_HANDLE vl_etalHandle;
		tSInt vl_res = OSAL_OK;

		tU8 	vl_criticalDataThr;
		
		etalRDSAttr vl_etalRdsAttr;	

		vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

		if (ETAL_INVALID_HANDLE == vl_etalHandle)
			{
				// add-on ETAL 
			if (OSAL_OK != DABMW_ServiceFollowing_ExtInt_AllocateEtalHandleFromApp(vI_app))
				{
					// error
					return(OSAL_ERROR);
				}
			
			vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
			
			}

	
		DABMW_SF_PRINTF(TR_LEVEL_COMPONENT, "*DABMW_ServiceFollowing_ExtInt_RdsEnable :  path = %d, fast detection %d, etalhandle %d\n",
						vI_app,
						vI_Isfast_PI_detection,
						vl_etalHandle);

	
		if (true == vI_Isfast_PI_detection)
			{
			vl_criticalDataThr = SF_RDS_ENABLE_CRITICAL_DATA_THR_FAST_PI;
			}
		else
			{
			vl_criticalDataThr = SF_RDS_ENABLE_CRITICAL_DATA_THR;
			}

		DABMW_RdsDataSetup(vl_etalHandle, vl_criticalDataThr);

		if (true == vI_Isfast_PI_detection)
		{
	    	vl_etalRdsAttr.rdsMode = ETAL_RDS_MODE_PERMANENT_FAST_PI;
			vl_etalRdsAttr.nbRdsBlockforInteruptFastPI = SF_NB_BLOCKS_FOR_WAKEUP_FAST_PI; 
		}
		else
		{
			vl_etalRdsAttr.rdsMode = ETAL_RDS_MODE_NORMAL;
			vl_etalRdsAttr.nbRdsBlockforInteruptFastPI = 0; 
		}

		// set to RDS normal mode.
		// 
		vl_etalRdsAttr.rdsRbdsMode = ETAL_RDS_MODE;

	    vl_etalRdsAttr.nbRdsBlockforInteruptNormalMode = SF_NB_BLOCKS_FOR_WAKEUP_NORMAL;    /* for normal mode, indicate the number of decoded blocks which will generate an interrupt for buffer read */


		// Addition of receiver Lock while sending the command
		// to avoid conflict management on interface
		// Lock addition change start
		if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
		{
			
			// check receiver is still valid and has not been destroyed !
			if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
			{
				if (ETAL_RET_SUCCESS != ETAL_start_RDS(vl_etalHandle, &vl_etalRdsAttr))
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
			//nothing
			vl_res = OSAL_ERROR;
		}
		//	Lock addition change end	
	
	return(vl_res);
	
}

tSInt DABMW_ServiceFollowing_ExtInt_RdsDisable (DABMW_SF_mwAppTy vI_app)
{


	ETAL_HANDLE vl_etalHandle;
	tSInt vl_res;

	vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{
		
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
	 		vl_res = ETAL_stop_RDS(vl_etalHandle);
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
	
	return(vl_res);

}

tSInt DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOn(DABMW_SF_mwAppTy vI_app)
{
	ETAL_HANDLE vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);
	tSInt vl_res = OSAL_OK;

	// Despite it is not needed in STAR because RDS buffer are reset on seek tune step
	// it is needed due to ETAL implementation : 
	// the RDS fast PI is decoded prior the 'auto-seek' info is available.
	// as a consequence, the frequency associated to the PI is not correct
	//

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{

		
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{
			if (ETAL_RET_SUCCESS != ETAL_resume_RDS(vl_etalHandle))
				{
				vl_res = OSAL_ERROR;
				}
		
			// Purge the RDS so that we start correctly on new tuned freq
			
			DABMW_RdsDataEvent (vl_etalHandle, DABMW_EVENT_FREQUENCY_TUNED);
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
	

	return(vl_res);
}


tSInt DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOff(DABMW_SF_mwAppTy vI_app)
{
	// Despite it is not needed in STAR because RDS buffer are reset on seek tune step
	// it is needed due to ETAL implementation : 
	// the RDS fast PI is decoded prior the 'auto-seek' info is available.
	// as a consequence, the frequency associated to the PI is not correct
	//
	
	ETAL_HANDLE vl_etalHandle;
	tSInt vl_res;
	
	vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

	// Addition of receiver Lock while sending the command
	// to avoid conflict management on interface
	// Lock addition change start
	if ( ETAL_receiverGetLock(vl_etalHandle) == ETAL_RET_SUCCESS)
	{
		// check receiver is still valid and has not been destroyed !
		if (true == ETAL_receiverIsValidHandle(vl_etalHandle))
		{

			vl_res = ETAL_suspend_RDS(vl_etalHandle);
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
	

	return(vl_res);
}

tSInt DABMW_ServiceFollowing_ExtInt_IncreasePiMatchingAF (DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, tU32 vI_expected_PI)
{
	ETAL_HANDLE vl_etalHandle;

	// Find the handle, which is the entry parameter to RdsDataSetup
	// 
	// for now, based on matching app to handle .
	// could be replaced by : GetHandleFromApp
	//
	
	vl_etalHandle = DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(vI_app);

    (tVoid) vI_frequency; // for lint

	return(DABMW_RdsDataSetupIncreasePiMatchingAF(vl_etalHandle, vI_expected_PI));
}


#endif //  CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING	

#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RDS_C

