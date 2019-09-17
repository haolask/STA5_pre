//!
//!  \file 		etalapi_debug.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, debug functions
//!  \author 	David Pastor
//!

#include "osal.h"
#include "etalinternal.h"

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
#if defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)

/***************************
 *
 * Macros
 *
 **************************/

/***************************
 *
 * Local function
 *
 **************************/

/***************************
 *
 * ETAL_debugSetDISSInternal
 *
 **************************/
ETAL_STATUS ETAL_debugSetDISSInternal(ETAL_HANDLE hReceiver, etalChannelTy tuner_channel, EtalDISSMode mode, tU8 filter_index)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt retval;
	EtalBcastStandard std;
	tBool configVPAMode;
	EtalDebugVPAModeEnumTy debugVPAMode;
	etalChannelTy channel;
	EtalDISSStatusTy diss_status;
	etalReceiverStatusTy *recvp;

	/* check receiver is STAR */
	if ((recvp = ETAL_receiverGet(hReceiver)) == NULL)
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	if (false == ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(recvp->CMOSTConfig.tunerId)))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}

	/* check receiver is FM or FM HD */
	std = ETAL_receiverGetStandard(hReceiver);
	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
	{
		return ETAL_RET_INVALID_BCAST_STANDARD;
	}

	/* check tuner_channel */
	if ((tuner_channel != ETAL_CHN_FOREGROUND) && (tuner_channel != ETAL_CHN_BACKGROUND) && (tuner_channel != ETAL_CHN_BOTH))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	/* check mode parameter, VPA tracking not supported for the moment */
	if ((mode != ETAL_DISS_MODE_AUTO) && (mode != ETAL_DISS_MODE_MANUAL))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	/* check filter_index parameter */
	if (filter_index > ETAL_DISS_FILTER_INDEX_MAX)
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	/* check Config VPA Mode off and Debug VPA Mode on not allowed */
	if ((ETAL_receiverGetConfigVPAMode(hReceiver, &configVPAMode) != ETAL_RET_SUCCESS) ||
		(ETAL_receiverGetDebugVPAMode(hReceiver, &debugVPAMode) != ETAL_RET_SUCCESS))
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	if ((configVPAMode == FALSE) && (debugVPAMode == ETAL_DEBUG_VPA_MODE_ON))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	/* get channel type */
	if (ETAL_receiverGetChannel(hReceiver, &channel) != OSAL_OK)
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	/* check tuner channel foreground only for auto mode and for manual mode (only with CVM on and DVM on) */
	if (((mode == ETAL_DISS_MODE_AUTO) && (channel == ETAL_CHN_BACKGROUND)) ||
		((mode == ETAL_DISS_MODE_MANUAL) && (configVPAMode == TRUE) && (debugVPAMode == ETAL_DEBUG_VPA_MODE_ON) && (channel == ETAL_CHN_BACKGROUND)))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	if (mode == ETAL_DISS_MODE_AUTO)
	{
		/* set tuner channel in the command */
		if ((configVPAMode == TRUE) && (debugVPAMode == ETAL_DEBUG_VPA_MODE_ON))
		{
			tuner_channel = ETAL_CHN_BOTH;
		}
		else
		{
			tuner_channel = ETAL_CHN_FOREGROUND;
		}
	}
	else    /* mode == ETAL_DISS_MODE_MANUAL */
	{
		/* set tuner channel in the command */
		if ((configVPAMode == TRUE) && (debugVPAMode == ETAL_DEBUG_VPA_MODE_ON))
		{
			tuner_channel = ETAL_CHN_BOTH;
		}
	}

	/* send set DISS command */
	if ((retval = ETAL_cmdDebugSetDISS_CMOST(hReceiver, tuner_channel, mode, filter_index)) != OSAL_OK)
	{
		switch (retval)
		{
			case OSAL_ERROR_INVALID_PARAM:
				ret = ETAL_RET_PARAMETER_ERR;
				break;
			case OSAL_ERROR:
			default:
				ret = ETAL_RET_ERROR;
				break;
		}
	}

	if (ret ==  ETAL_RET_SUCCESS)
	{
		/* save DISS status */
		diss_status.m_mode = mode;
		diss_status.m_filter_index = filter_index;
		if (ETAL_receiverSetDISSStatus(hReceiver, &diss_status) != ETAL_RET_SUCCESS)
		{
			return ETAL_RET_ERROR;
		}
	}


	return ret;
}

/***************************
 *
 * ETAL_debugGetWSPStatusInternal
 *
 **************************/
ETAL_STATUS ETAL_debugGetWSPStatusInternal(ETAL_HANDLE hReceiver, EtalWSPStatus *WSPStatus)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt retval;
	EtalBcastStandard std;
	etalReceiverStatusTy *recvp;

	/* check receiver is STAR */
	if ((recvp = ETAL_receiverGet(hReceiver)) == NULL)
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	if (false == ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(recvp->CMOSTConfig.tunerId)))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}

	/* check receiver is FM or AM or FM HD or AM HD */
	std = ETAL_receiverGetStandard(hReceiver);
	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_AM)&& !ETAL_IS_HDRADIO_STANDARD(std))
	{
		return ETAL_RET_INVALID_BCAST_STANDARD;
	}

	/* send get WSP status command */
	if ((retval = ETAL_cmdDebugGetWSPStatus_CMOST(hReceiver, WSPStatus)) != OSAL_OK)
	{
		switch (retval)
		{
			case OSAL_ERROR_INVALID_PARAM:
				ret = ETAL_RET_PARAMETER_ERR;
				break;
			case OSAL_ERROR:
			default:
				ret = ETAL_RET_ERROR;
				break;
		}
	}
	return ret;
}

/***************************
 *
 * ETAL_debugVPAControlInternal
 *
 **************************/
ETAL_STATUS ETAL_debugVPAControlInternal(ETAL_HANDLE hReceiver, tBool VPAmode, ETAL_HANDLE *hReceiverNew)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU32 i;
	etalFrequencyBandInfoTy band_info;
	EtalProcessingFeatures proc_features;
	EtalDISSStatusTy diss_status;
	tU32 frequency;
	tBool configVPAMode;
	EtalDebugVPAModeEnumTy debugVPAMode;
	EtalReceiverAttr attr;
	etalReceiverStatusTy *recvp;
	EtalBcastStandard std;
	ETAL_HINDEX tunerIndex, channelIndex, channelIndexNew;
	ETAL_HANDLE hFrontend;

	/* check Config VPA Mode is on */
	if ((ret = ETAL_receiverGetConfigVPAMode(hReceiver, &configVPAMode)) != ETAL_RET_SUCCESS)
	{
		return ret;
	}
	if (configVPAMode == FALSE)
	{
		return ETAL_RET_ERROR;
	}
	/* check receiver is FM */
	std = ETAL_receiverGetStandard(hReceiver);
	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	/* get receiver processing features */
	if ((ret = ETAL_receiverGetProcessingFeatures(hReceiver, &proc_features)) != ETAL_RET_SUCCESS)
	{
		return ret;
	}

	if ((recvp = ETAL_receiverGet(hReceiver)) == NULL)
	{
		return ETAL_RET_INVALID_RECEIVER;
	}

	/* check receiver is STAR-T */
	if (false == ETAL_DEVICE_IS_START(ETAL_tunerGetType(recvp->CMOSTConfig.tunerId)))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}

	/* check channel used and VPAmode */
	if (((recvp->diversity.m_DiversityMode == (tU8)2) && (VPAmode == TRUE) && ((proc_features.u.m_processing_features | ((tU8)ETAL_PROCESSING_FEATURE_FM_VPA | (tU8)ETAL_PROCESSING_FEATURE_ANTENNA_DIVERSITY)) != (tU8)0)) ||
		((recvp->CMOSTConfig.channel == ETAL_CHN_FOREGROUND) && (recvp->diversity.m_DiversityMode == (tU8)1) && (VPAmode == FALSE)) ||
		((recvp->CMOSTConfig.channel == ETAL_CHN_BACKGROUND) && (recvp->diversity.m_DiversityMode == (tU8)1)) ||
		((VPAmode == TRUE) && (hReceiverNew != NULL)))
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	/* get receiver band info */
	if (ETAL_receiverGetBandInfo(hReceiver, &band_info) != OSAL_OK)
	{
		return ETAL_RET_ERROR;
	}
	/* get receiver DISS status */
	if ((ret = ETAL_receiverGetDISSStatus(hReceiver, &diss_status)) != ETAL_RET_SUCCESS)
	{
		return ret;
	}
	/* get debug VPA mode */
	if ((ret = ETAL_receiverGetDebugVPAMode(hReceiver, &debugVPAMode)) != ETAL_RET_SUCCESS)
	{
		return ret;
	}


	if (VPAmode == TRUE)
	{
		/* status on */

		/* set Debug VPA Mode on */
		if ((ret = ETAL_receiverSetDebugVPAMode(hReceiver, ETAL_DEBUG_VPA_MODE_ON)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* set Config VPA Mode off temporarily for etal_change_band */
		if ((ret = ETAL_receiverSetConfigVPAMode(hReceiver, FALSE)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* configure hReceiver with foreground Front End only, processing features VPA still OFF */
		if ((recvp = ETAL_receiverGet(hReceiver)) == NULL)
		{
			return ETAL_RET_INVALID_RECEIVER;
		}
		(void)OSAL_pvMemorySet((tPVoid)&attr, 0x00, sizeof(EtalReceiverAttr));
		attr.m_Standard = ETAL_receiverGetStandard(hReceiver);
		attr.processingFeatures.u.m_processing_features = (recvp->CMOSTConfig.processingFeatures.u.m_processing_features | ETAL_PROCESSING_FEATURE_FM_VPA);

		if (ETAL_tunerSearchFrontend(recvp->diversity.m_FeConfig[0]) != OSAL_OK)
		{
			return ETAL_RET_FRONTEND_NOT_AVAILABLE;
		}
		attr.m_FrontEnds[0] = recvp->diversity.m_FeConfig[0];
		if (attr.m_FrontEnds[1] == ETAL_INVALID_HANDLE)
		{
			/* search the other frontend of the same tuner and add it to attribute */
			channelIndex = ETAL_handleFrontendGetChannel(attr.m_FrontEnds[0]);
			if (channelIndex == ETAL_FE_FOREGROUND)
			{
				channelIndexNew = ETAL_FE_BACKGROUND;
			}
			else if (channelIndex == ETAL_FE_BACKGROUND)
			{
				channelIndexNew = ETAL_FE_FOREGROUND;
			}
			else
			{
				return ETAL_RET_FRONTEND_LIST_ERR;
			}
			attr.m_FrontEnds[1] = ETAL_handleMakeFrontend(ETAL_handleFrontendGetTunerIndex(attr.m_FrontEnds[0]), channelIndexNew);
			attr.m_FrontEndsSize = (tU8)2;
		}

		if (attr.m_FrontEndsSize == (tU8)0)
		{
			return ETAL_RET_ERROR;
		}
		if ((ret = ETAL_configReceiverInternal(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* set Config VPA Mode on, only Debug VPA Mode is changed */
		if ((ret = ETAL_receiverSetConfigVPAMode(hReceiver, TRUE)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* tune foreground frequency */
		if ((frequency = ETAL_receiverGetFrequency(hReceiver)) == ETAL_INVALID_FREQUENCY)
		{
			return ETAL_RET_INVALID_RECEIVER;
		}
		if ((ret = ETAL_tuneReceiverInternal(hReceiver, frequency, cmdTuneNormalResponse)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		if ((hReceiverNew != NULL) && (ETAL_receiverIsValidHandle(*hReceiverNew) == TRUE))
		{
			/* set Config VPA Mode on, only Debug VPA Mode is changed */
			if ((ret = ETAL_receiverSetConfigVPAMode(*hReceiverNew, TRUE)) != ETAL_RET_SUCCESS)
			{
				return ret;
			}
			/* set Debug VPA Mode on */
			if ((ret = ETAL_receiverSetDebugVPAMode(*hReceiverNew, ETAL_DEBUG_VPA_MODE_ON)) != ETAL_RET_SUCCESS)
			{
				return ret;
			}
		}
		/* set DISS both channel */
		if ((ret = ETAL_debugSetDISSInternal(hReceiver, ETAL_CHN_BOTH, diss_status.m_mode, diss_status.m_filter_index)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
	}
	else
	{
		/* status off */

		/* set hReceiver Debug VPA Mode OFF */
		if ((ret = ETAL_receiverSetDebugVPAMode(hReceiver, ETAL_DEBUG_VPA_MODE_OFF)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* configure hReceiver with foreground Front End only, processing features VPA still ON */
		if ((recvp = ETAL_receiverGet(hReceiver)) == NULL)
		{
			return ETAL_RET_INVALID_RECEIVER;
		}
		(void)OSAL_pvMemorySet((tPVoid)&attr, 0x00, sizeof(EtalReceiverAttr));
		attr.m_Standard = ETAL_receiverGetStandard(hReceiver);
		attr.processingFeatures.u.m_processing_features = (recvp->CMOSTConfig.processingFeatures.u.m_processing_features & (~ETAL_PROCESSING_FEATURE_FM_VPA));

		for (i = 0; i < (tU32)recvp->diversity.m_DiversityMode; i++)
		{
			if (attr.m_FrontEnds[0] == ETAL_INVALID_HANDLE)
			{
				if (ETAL_tunerSearchFrontend(recvp->diversity.m_FeConfig[i]) == OSAL_OK)
				{
					if (ETAL_handleFrontendGetChannel(recvp->diversity.m_FeConfig[i]) == ETAL_FE_FOREGROUND)
					{
						/* keep only foreground FrontEnd */
						attr.m_FrontEnds[0] = recvp->diversity.m_FeConfig[i];
						attr.m_FrontEndsSize = (tU8)1;
						break;
					}
				}
			}
		}
		if (attr.m_FrontEndsSize == (tU8)0)
		{
			return ETAL_RET_ERROR;
		}
		if ((ret = ETAL_configReceiverInternal(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* set foreground Config VPA Mode on, only Debug VPA Mode is changed */
		if ((ret = ETAL_receiverSetConfigVPAMode(hReceiver, TRUE)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* tune foreground frequency */
		if ((frequency = ETAL_receiverGetFrequency(hReceiver)) == ETAL_INVALID_FREQUENCY)
		{
			return ETAL_RET_INVALID_RECEIVER;
		}
		if ((ret = ETAL_tuneReceiverInternal(hReceiver, frequency, cmdTuneNormalResponse)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}
		/* set DISS foreground channel */
		if ((ret = ETAL_debugSetDISSInternal(hReceiver, ETAL_CHN_FOREGROUND, diss_status.m_mode, diss_status.m_filter_index)) != ETAL_RET_SUCCESS)
		{
			return ret;
		}

		/* configure hReceiver_bg with background Front End only, processing features VPA OFF */
		if (hReceiverNew != NULL)
		{
			/* check if the other FE in this Receiver is available to create a new Receiver */
			if (recvp->diversity.m_DiversityMode == (tU8)0)
			{
				return ETAL_RET_FRONTEND_NOT_AVAILABLE;
			}
			tunerIndex = ETAL_handleFrontendGetTunerIndex(recvp->diversity.m_FeConfig[0]);
			channelIndex = ETAL_handleFrontendGetChannel(recvp->diversity.m_FeConfig[0]);
			if (channelIndex != ETAL_FE_FOREGROUND)
			{
				return ETAL_RET_FRONTEND_NOT_AVAILABLE;
			}
			hFrontend = ETAL_handleMakeFrontend(tunerIndex, ETAL_FE_BACKGROUND);
			if ((ETAL_tunerSearchFrontend(hFrontend) != OSAL_OK) ||
				(ETAL_receiverIsFrontendUsed(recvp, hFrontend) == TRUE))
			{
				return ETAL_RET_FRONTEND_NOT_AVAILABLE;
			}

			/* the background exists and is free, use it */
			(void)OSAL_pvMemorySet((tPVoid)&attr, 0x00, sizeof(EtalReceiverAttr));
			attr.m_Standard = ETAL_receiverGetStandard(hReceiver);
			attr.processingFeatures.u.m_processing_features = (recvp->CMOSTConfig.processingFeatures.u.m_processing_features & (~ETAL_PROCESSING_FEATURE_FM_VPA));
			attr.m_FrontEnds[0] = hFrontend;
			attr.m_FrontEndsSize = (tU8)1;

			if ((ret = ETAL_configReceiverInternal(hReceiverNew, &attr)) != ETAL_RET_SUCCESS)
			{
				return ret;
			}
			/* set background Debug VPA Mode off */
			if ((ret = ETAL_receiverSetDebugVPAMode(*hReceiverNew, ETAL_DEBUG_VPA_MODE_OFF)) != ETAL_RET_SUCCESS)
			{
				return ret;
			}
			/* tune background frequency */
			if ((frequency = ETAL_receiverGetFrequency(hReceiver)) == ETAL_INVALID_FREQUENCY)
			{
				return ETAL_RET_INVALID_RECEIVER;
			}
			if ((ret = ETAL_tuneReceiverInternal(*hReceiverNew, frequency, cmdTuneNormalResponse)) != ETAL_RET_SUCCESS)
			{
				return ret;
			}
			/* set DISS background channel */
			if ((ret = ETAL_debugSetDISSInternal(*hReceiverNew, ETAL_CHN_BACKGROUND, ETAL_DISS_MODE_MANUAL, (tU8)5)) != ETAL_RET_SUCCESS)
			{
				return ret;
			}
		}
	}

	return ret;
}

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_debug_DISS_control
 *
 **************************/
ETAL_STATUS etal_debug_DISS_control(ETAL_HANDLE hReceiver, etalChannelTy tuner_channel, EtalDISSMode mode, tU8 filter_index)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_debug_DISS_control(rec: %d, tunCha: %d, mod: %d, filInd: %d)", hReceiver, tuner_channel, mode, filter_index);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_handleIsValid(hReceiver))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			/* send set DISS command */
			ret = ETAL_debugSetDISSInternal(hReceiver, tuner_channel, mode, filter_index);
		}
		ETAL_statusReleaseLock();
	}
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_debug_DISS_control() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_debug_get_WSP_Status
 *
 **************************/
ETAL_STATUS etal_debug_get_WSP_Status(ETAL_HANDLE hReceiver, EtalWSPStatus *WSPStatus)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_debug_get_WSP_Status(rec: %d)", hReceiver);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_handleIsValid(hReceiver))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			/* send set WSP status command */
			ret = ETAL_debugGetWSPStatusInternal(hReceiver, WSPStatus);
		}
		ETAL_statusReleaseLock();
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_debug_get_WSP_Status() = %s", ETAL_STATUS_toString(ret));
	if(WSPStatus != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "WSPStatus: (filInd[0]:%d, filInd[1]: %d, sofMut: %d, higCut: %d, lowCut: %d, steBle: %d, higBle: %d, rolOff: 0x%x)",
			WSPStatus->m_filter_index[0], WSPStatus->m_filter_index[1], WSPStatus->m_softmute, WSPStatus->m_highcut,
			WSPStatus->m_lowcut, WSPStatus->m_stereoblend, WSPStatus->m_highblend, WSPStatus->m_rolloff);
	}

	return ret;
}

/***************************
 *
 * etal_debug_VPA_control
 *
 **************************/
ETAL_STATUS etal_debug_VPA_control(ETAL_HANDLE hReceiver, tBool VPAmode, ETAL_HANDLE *hReceiverNew)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_debug_VPA_control(rec: %d, VPAMod: %d)", hReceiver, VPAmode);
	if(hReceiverNew != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "recNew: %d", *hReceiverNew);
	}

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_handleIsValid(hReceiver))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			/* send set WSP status command */
			ret = ETAL_debugVPAControlInternal(hReceiver, VPAmode, hReceiverNew);
		}

		ETAL_statusReleaseLock();
	}
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_debug_VPA_control() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

#endif // defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR


#if defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_debug_config_audio_alignment
 *
 **************************/
/*!
 * \brief		configure the AAA algorithm available on some versions of the HD Radio DCOP
 *
 * \details		Configures the audio alignement control, 
 *				a control of the AAA (de)activation for FM and AM standard case is possible
 *				In addition, the action uppon alignement not found is defined: 
 *				either blend to digital, even when alignment not found, or stay in analog.
 *
 * \remark		the HD-DCOP fw as a default configuration for AAA feature support. 
 *				this commands applies only on fw which support the AAA feature.
 *
 *				Note that it applies to all DCOP instances.
 *		
 * \param[in]	hReceiver : hinit_params - pointer to initialization parameters
 *
 * \return 	#ETAL_RET_ERROR - communication error
 * \return		#ETA_RET_SUCCESS - command executed correctly
 *
 * \see		
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_debug_config_audio_alignment(EtalAudioAlignmentAttr *alignmentParams)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tSInt vl_res;
#endif

	if (alignmentParams == NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_debug_config_audio_alignment(alignmentParams(NULL))");
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_debug_config_audio_alignment(alignmentParams(AAA_FM :%d, AAA_AM : %d))", 
			alignmentParams->m_enableAutoAlignmentForFM, 
			alignmentParams->m_enableAutoAlignmentForAM);

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		if (ETAL_statusGetLock() != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			if ( (vl_res = ETAL_cmdSetAlignParam_HDRADIO(alignmentParams->m_enableAutoAlignmentForAM, 
					alignmentParams->m_enableAutoAlignmentForFM,
					true, true)) != OSAL_OK)
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL, "etal_debug_config_audio_alignment ==> ETAL_cmdSetAlignParam_HDRADIO failed res = %d", vl_res);
				ret = ETAL_RET_ERROR;
			}

			ETAL_statusReleaseLock();
		}
#else
		ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_debug_config_audio_alignment() = %s", ETAL_STATUS_toString(ret));

	return ret;
}
#endif // CONFIG_ETAL_HAVE_DEBUG_COMMANDS || CONFIG_ETAL_HAVE_ALL_API
