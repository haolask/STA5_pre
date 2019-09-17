//!
//!  \file 		etalseamless.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Seamless estimation functions
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"
#include "etalinternal.h"
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
#include "streamdecadapt4etal.h"
#endif

#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
/******************************************************************************
 * Local functions
 *****************************************************************************/
static ETAL_STATUS ETAL_checkSeamlessSwitchingParameter(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig_ptr);
static ETAL_STATUS ETAL_checkSeamlessEstimationStartParameter(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig_ptr);
static ETAL_STATUS ETAL_checkSeamlessEstimationStopParameter(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS);

/***************************
 *
 * ETAL_checkSeamlessEstimationParameter
 *
 **************************/
static ETAL_STATUS ETAL_checkSeamlessEstimationStartParameter(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig_ptr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (!ETAL_receiverIsValidHandle(hReceiverFAS) &&
		!ETAL_receiverIsValidHandle(hReceiverSAS))
	{
		/* At least one receiver has to be valid (EARLY FM) */
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if (ETAL_receiverIsSpecialInProgress(hReceiverFAS, cmdSpecialAnyChangingFrequency) ||
		ETAL_receiverIsSpecialInProgress(hReceiverFAS, cmdSpecialSeamlessEstimation) ||
		ETAL_receiverIsSpecialInProgress(hReceiverFAS, cmdSpecialSeamlessSwitching) ||
		ETAL_receiverIsSpecialInProgress(hReceiverSAS, cmdSpecialAnyChangingFrequency) ||
		ETAL_receiverIsSpecialInProgress(hReceiverSAS, cmdSpecialSeamlessEstimation) ||
		ETAL_receiverIsSpecialInProgress(hReceiverSAS, cmdSpecialSeamlessSwitching))
	{
		ret = ETAL_RET_IN_PROGRESS;
	}
	else if(seamlessEstimationConfig_ptr == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else if (seamlessEstimationConfig_ptr->mode > (tU8)1)
	{
	    ret = ETAL_RET_PARAMETER_ERR;
	}

	/* Other sanity checks are performed in the DCOP */
	return ret;
}


/***************************
 *
 * ETAL_seamless_estimation_start_internal
 *
 **************************/
ETAL_STATUS ETAL_seamless_estimation_start_internal(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig_ptr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ret = ETAL_checkSeamlessEstimationStartParameter(hReceiverFAS, hReceiverSAS, seamlessEstimationConfig_ptr);

	if (ret == ETAL_RET_SUCCESS)
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		/* Signal the receiver is busy with seamless estimation
		 * and start seamless estimation Request FSM */
		/* It is possible that only 1 receiver is active, because typically seamless to FM without DAB or viceversa
		 * therefore setspecial is to be done 'only' if receiver is valid */

		if (ETAL_receiverIsValidHandle(hReceiverFAS))
		{
			ETAL_receiverSetSpecial(hReceiverFAS, cmdSpecialSeamlessEstimation, cmdActionStart);
		}

		if (ETAL_receiverIsValidHandle(hReceiverSAS))
		{
			ETAL_receiverSetSpecial(hReceiverSAS, cmdSpecialSeamlessEstimation, cmdActionStart);
		}

#ifndef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
		if (ETAL_cmdSeamlessEstimation_MDR(seamlessEstimationConfig_ptr) != OSAL_OK)
#else
		if (ETAL_cmdSeamlessEstimation_DABMW(hReceiverFAS, hReceiverSAS, seamlessEstimationConfig_ptr) != OSAL_OK)
#endif
		{
			if (ETAL_receiverIsValidHandle(hReceiverFAS))
			{
				ETAL_receiverSetSpecial(hReceiverFAS, cmdSpecialSeamlessEstimation, cmdActionStop);
			}

			if (ETAL_receiverIsValidHandle(hReceiverSAS))
			{
				ETAL_receiverSetSpecial(hReceiverSAS, cmdSpecialSeamlessEstimation, cmdActionStop);
			}

			ret = ETAL_RET_ERROR;
		}
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR
	}
	return ret;
}

/***************************
 *
 * ETAL_checkSeamlessEstimationParameterStart
 *
 **************************/
static ETAL_STATUS ETAL_checkSeamlessEstimationStopParameter(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (!ETAL_receiverIsValidHandle(hReceiverFAS) &&
		!ETAL_receiverIsValidHandle(hReceiverSAS))
	{
		/* At least one receiver has to be valid (EARLY FM) */
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if (!ETAL_receiverIsSpecialInProgress(hReceiverFAS, cmdSpecialSeamlessEstimation) ||
		!ETAL_receiverIsSpecialInProgress(hReceiverSAS, cmdSpecialSeamlessEstimation))
	{
		ret = ETAL_RET_ERROR;
	}
	return ret;
}

/***************************
 *
 * ETAL_seamless_estimation_stop_internal
 *
 **************************/
ETAL_STATUS ETAL_seamless_estimation_stop_internal(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	etalSeamlessEstimationConfigTy seamlessEstimationConfig;
#endif

	ret = ETAL_checkSeamlessEstimationStopParameter(hReceiverFAS, hReceiverSAS);

	if(ret == ETAL_RET_SUCCESS)
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		seamlessEstimationConfig.mode = (tU8)0;
		seamlessEstimationConfig.startPosition = 0;
		seamlessEstimationConfig.stopPosition = -480000;

#ifndef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
		if (ETAL_cmdSeamlessEstimation_MDR(&seamlessEstimationConfig) != OSAL_OK)
#else
		if (ETAL_cmdSeamlessEstimation_DABMW(hReceiverFAS, hReceiverSAS, &seamlessEstimationConfig) != OSAL_OK)
#endif
		{
			ret = ETAL_RET_ERROR;
		}
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR
	}
	else
	{
		/* The MDR Seamless estimation FSM was stopped autonomously by the MDR
		 * due to Seamless estimation Event.
		 * Avoid sending the command because MDR would respond with an error */
	}
	return ret;
}

/***************************
 *
 * ETAL_checkSeamlessSwitchingParameter
 *
 **************************/
static ETAL_STATUS ETAL_checkSeamlessSwitchingParameter(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig_ptr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (!ETAL_receiverIsValidHandle(hReceiverFAS) &&
		!ETAL_receiverIsValidHandle(hReceiverSAS))
	{
		/* At least one receiver has to be valid (EARLY FM) */
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if (ETAL_receiverIsSpecialInProgress(hReceiverFAS, cmdSpecialAnyChangingFrequency) ||
		ETAL_receiverIsSpecialInProgress(hReceiverFAS, cmdSpecialSeamlessSwitching) ||
		ETAL_receiverIsSpecialInProgress(hReceiverSAS, cmdSpecialAnyChangingFrequency) ||
		ETAL_receiverIsSpecialInProgress(hReceiverSAS, cmdSpecialSeamlessSwitching))
	{
		/* Seamless switching is possible during seamless estimation */
		ret = ETAL_RET_IN_PROGRESS;
	}
	else if(seamlessSwitchingConfig_ptr == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
		/* Sanity checks for the seamlessSwitchingConfig is performed in the DCOP */
	}
	return ret;
}

/***************************
 *
 * ETAL_seamless_switching_internal
 *
 **************************/
ETAL_STATUS ETAL_seamless_switching_internal(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig_ptr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ret = ETAL_checkSeamlessSwitchingParameter(hReceiverFAS, hReceiverSAS, seamlessSwitchingConfig_ptr);

	if (ret == ETAL_RET_SUCCESS)
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		/* Signal the receiver is busy with seamless switching */
		/* at least 1 receiver is needed, but 1 only receiver not both */
		if (ETAL_receiverIsValidHandle(hReceiverFAS))
		{
			ETAL_receiverSetSpecial(hReceiverFAS, cmdSpecialSeamlessSwitching, cmdActionStart);
		}

		if (ETAL_receiverIsValidHandle(hReceiverSAS))
		{
			ETAL_receiverSetSpecial(hReceiverSAS, cmdSpecialSeamlessSwitching, cmdActionStart);
		}

#ifndef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
		if (ETAL_cmdSeamlessSwitching_MDR(hReceiverFAS, hReceiverSAS, seamlessSwitchingConfig_ptr) != OSAL_OK)
#else
		if (ETAL_cmdSeamlessSwitching_DABMW(hReceiverFAS, hReceiverSAS, seamlessSwitchingConfig_ptr) != OSAL_OK)
#endif
		{
			if (ETAL_receiverIsValidHandle(hReceiverFAS))
			{
				ETAL_receiverSetSpecial(hReceiverFAS, cmdSpecialSeamlessSwitching, cmdActionStop);
			}

			if (ETAL_receiverIsValidHandle(hReceiverSAS))
			{
				ETAL_receiverSetSpecial(hReceiverSAS, cmdSpecialSeamlessSwitching, cmdActionStop);
			}

			ret = ETAL_RET_ERROR;
		}
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR
	}
	return ret;
}
#endif //defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
