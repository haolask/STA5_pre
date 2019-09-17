//!
//!  \file 		etalapi_seamless.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_seamless_estimation_start
 *
 **************************/
ETAL_STATUS etal_seamless_estimation_start(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig_ptr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	ETAL_HANDLE hRecevier_SE;
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_seamless_estimation_start(recFAS: %d, recSAS: %d)",
            hReceiverFAS, hReceiverSAS);
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "seaEstCon: (mod: %d, staPos: %d, stoPos: %d)",
            seamlessEstimationConfig_ptr->mode, seamlessEstimationConfig_ptr->startPosition, seamlessEstimationConfig_ptr->stopPosition);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		
		ret = ETAL_seamless_estimation_start_internal(hReceiverFAS, hReceiverSAS, seamlessEstimationConfig_ptr);
		if (ETAL_RET_SUCCESS == ret)
		{
			// this is temporary : 
			// the goal is to indicate that a external seamless request is on. 
			// we need to set the flag on only 1 receiver : the first cmdSpecialSeamlessEstimation receiver found
			// reason is that flag clearing will be on MDR ie DAB receiver response
			// if we set on both, we cannot retrieve which receiver is set to 'external', and we will have to clear any in progress
			// which is not good for parallel actions 
			//
			hRecevier_SE = ETAL_receiverSearchActiveSpecial(cmdSpecialSeamlessEstimation);
			// we should put  the special in Progress on the first found cmdSpecialSeamlessEstimation receiver
			if (hRecevier_SE == hReceiverFAS)
			{
			    // Enable external seamless estimation event
			    ETAL_receiverSetSpecial(hReceiverFAS, cmdSpecialExternalSeamlessEstimationRequestInProgress, cmdActionStart);
			}
			else if (hRecevier_SE == hReceiverSAS)
			{
			    // Enable external seamless estimation event
			    ETAL_receiverSetSpecial(hReceiverSAS, cmdSpecialExternalSeamlessEstimationRequestInProgress, cmdActionStart);
			}
			else
			{
				ret = ETAL_RET_INVALID_RECEIVER;
			}
		}
#endif
		ETAL_statusReleaseLock();
	}


    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_seamless_estimation_start() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_seamless_estimation_stop
 *
 **************************/
ETAL_STATUS etal_seamless_estimation_stop(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_seamless_estimation_stop(recFAS: %d, recSAS: %d)",
            hReceiverFAS, hReceiverSAS);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		ret = ETAL_seamless_estimation_stop_internal(hReceiverFAS, hReceiverSAS);
#endif
		ETAL_statusReleaseLock();
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_seamless_estimation_stop() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_seamless_switching
 *
 **************************/
ETAL_STATUS etal_seamless_switching(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig_ptr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	ETAL_HANDLE hRecevier_SS;
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_seamless_switching(recFAS: %d, recSAS: %d)", hReceiverFAS, hReceiverSAS);
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "SeaSwiCon: (sysToSwi: %d, proTyp: %d, absDelEst: %d, delEst: %d, timFAS: %d, timSAS: %d, aveRMS2FAS: %d, aveRMS2SAS: %d)",
            seamlessSwitchingConfig_ptr->systemToSwitch,
            seamlessSwitchingConfig_ptr->providerType, seamlessSwitchingConfig_ptr->absoluteDelayEstimate, seamlessSwitchingConfig_ptr->delayEstimate,
            seamlessSwitchingConfig_ptr->timestampFAS, seamlessSwitchingConfig_ptr->timestampSAS, seamlessSwitchingConfig_ptr->averageRMS2FAS,
            seamlessSwitchingConfig_ptr->averageRMS2SAS);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		ret = ETAL_seamless_switching_internal(hReceiverFAS, hReceiverSAS, seamlessSwitchingConfig_ptr);
		if (ETAL_RET_SUCCESS == ret)
		{		
			// we should put the special in Progress on the first cmdSpecialSeamlessEstimation receiver found
			hRecevier_SS = ETAL_receiverSearchActiveSpecial(cmdSpecialSeamlessSwitching);
			if (hRecevier_SS == hReceiverFAS)
			{
			    // Enable external seamless switching event
				ETAL_receiverSetSpecial(hReceiverFAS, cmdSpecialExternalSeamlessSwitchingRequestInProgress, cmdActionStart);
			}
			else if (hRecevier_SS == hReceiverSAS)
			{
			    // Enable external seamless switching event
				ETAL_receiverSetSpecial(hReceiverSAS, cmdSpecialExternalSeamlessSwitchingRequestInProgress, cmdActionStart);
			}
			else
			{
				ret = ETAL_RET_INVALID_RECEIVER;
			}
		}
#endif
		ETAL_statusReleaseLock();
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_seamless_switching() = %s", ETAL_STATUS_toString(ret));
	return ret;
}
#endif //defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
