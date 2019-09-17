//!
//!  \file 		etalapi_audio.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, initialization functions
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"
#include "boot_cmost.h"

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
    #include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_T
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
    #include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_S
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
    #include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_T
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
    #include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_S
#endif

/***************************
 *
 * Local function
 *
 **************************/

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

/***************************
 *
 * ETAL_checkAudioSelectParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkAudioSelectParameters(ETAL_HANDLE hReceiver, EtalAudioSourceTy src)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		switch (src)
		{
			case ETAL_AUDIO_SOURCE_AUTO_HD:
			case ETAL_AUDIO_SOURCE_STAR_AMFM:
#ifndef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
			case ETAL_AUDIO_SOURCE_DCOP_STA680:
			case ETAL_AUDIO_SOURCE_HD_ALIGN:
#endif
			case ETAL_AUDIO_SOURCE_DCOP_STA660:
				break;

			default:
				ret = ETAL_RET_PARAMETER_ERR;
				break;
		}
	}

	return ret;
}
#endif

#if defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETAL_checkForceMonoParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkForceMonoParameters(ETAL_HANDLE hReceiver)
{
	ETAL_HANDLE validated_hReceiver = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (hReceiver == ETAL_INVALID_HANDLE)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		validated_hReceiver = hReceiver;
		if (!ETAL_receiverIsValidHandle(validated_hReceiver))
		{
			ret = ETAL_RET_ERROR;
		}
		else if (!ETAL_receiverSupportsAudio(validated_hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			/* Nothing to do */
		}
	}
	
	return ret;
}

/***************************
 *
 * ETAL_checkMuteParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkMuteParameters(ETAL_HANDLE hReceiver, tBool muteFlag)
{
	ETAL_HANDLE validated_hReceiver = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if((muteFlag != TRUE) && (muteFlag != FALSE))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		if (hReceiver == ETAL_INVALID_HANDLE)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else
		{
			validated_hReceiver = hReceiver;
			if (!ETAL_receiverIsValidHandle(validated_hReceiver))
			{
				ret = ETAL_RET_ERROR;
			}
			else if (!ETAL_receiverSupportsAudio(validated_hReceiver))
			{
				ret = ETAL_RET_INVALID_RECEIVER;
			}
			else
			{
				/* Nothing to do */
			}
	
		}
	}
	
	return ret;
}
#endif // #if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
#endif // CONFIG_ETAL_HAVE_AUDIO_CONTROL || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

/***************************
 *
 * ETAL_configAudioPathInternal
 *
 **************************/
static ETAL_STATUS ETAL_configAudioPathInternal(tU32 tunerIndex, etalAudioIntfStatusTy intf)
{

	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	ETAL_HANDLE hTuner, hReceiver;
	EtalReceiverAttr RecvCfg;
	EtalDeviceType device;

	// check parameter validity
	if (intf.bitfield.reserved2 != (tU8)0)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		/*
		 * Since this function is used for an exported API
		 * but it is a 'low-level' API, we only check if the
		 * Tuner is capable of Audio, that is if it is a STAR
		 * and not a DOT, and not if it is configured for audio
		 * data type in etalTuner config.
		 */
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tunerIndex);
		device = ETAL_tunerGetType(hTuner);
		if (device == deviceUnknown)
		{
			ret = ETAL_RET_INVALID_HANDLE; // TODO update ETAL spec
		}
		else if (ETAL_DEVICE_IS_STAR(device))
		{
			if (ETAL_cmdSelectAudioInterface_CMOST(hTuner, intf) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
			else
			{
				/* CMOST take into account a m_dac change with a Tuner_change_band */
				/* config receiver that will send a change band */
				hReceiver = ETAL_INVALID_HANDLE;
				memset(&RecvCfg, 0, sizeof(RecvCfg));
				RecvCfg.m_Standard = ETAL_BCAST_STD_FM;
				RecvCfg.m_FrontEndsSize = 1;
				RecvCfg.m_FrontEnds[0] = ETAL_FE_HANDLE_1;
				RecvCfg.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
				/* check if the requested FE is already selected in some other ETAL receiver */
				if (ETAL_isAllFrontendFree(RecvCfg.m_FrontEnds, (tU32)(RecvCfg.m_FrontEndsSize)))
				{
					if ((ret = ETAL_configReceiverInternal(&hReceiver, &RecvCfg)) == ETAL_RET_SUCCESS)
					{
						ret = ETAL_destroyReceiverInternal(hReceiver);
					}
				}
				else
				{
					ret = ETAL_RET_FRONTEND_LIST_ERR;
				}
				if ((ret == ETAL_RET_FRONTEND_LIST_ERR) || (ret != ETAL_RET_SUCCESS))
				{
					/* FG frontend seems already used, try with BG frontend to send a Tuner_change_band */
					hReceiver = ETAL_INVALID_HANDLE;
					memset(&RecvCfg, 0, sizeof(RecvCfg));
					RecvCfg.m_Standard = ETAL_BCAST_STD_FM;
					RecvCfg.m_FrontEndsSize = 1;
					RecvCfg.m_FrontEnds[0] = ETAL_FE_HANDLE_2;
					RecvCfg.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
					/* check if the requested FE is already selected in some other ETAL receiver */
					if (ETAL_isAllFrontendFree(RecvCfg.m_FrontEnds, (tU32)(RecvCfg.m_FrontEndsSize)))
					{
						if ((ret = ETAL_configReceiverInternal(&hReceiver, &RecvCfg)) == ETAL_RET_SUCCESS)
						{
							ret = ETAL_destroyReceiverInternal(hReceiver);
						}
					}
					else
					{
						ret = ETAL_RET_FRONTEND_LIST_ERR;
					}
				}
			}
		}
		else
		{
			/* Nothing to do */
		}
	}

	return ret;
}
#endif // #if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

/***************************
 *
 * ETAL_mute
 *
 **************************/
/*!
 * \brief		Mutes the audio for a Receiver
 * \details		
 * \remark		
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	muteFlag
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS ETAL_mute(ETAL_HANDLE hReceiver, tBool muteFlag)
{
	ETAL_STATUS ret;
	ETAL_HANDLE hTuner;
	tSInt retosal;
	EtalBcastStandard std;

	ret = ETAL_RET_ERROR;

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
			retosal = ETAL_getTunerIdForAudioCommands(hReceiver, &hTuner);
			if (retosal == OSAL_ERROR_NOT_SUPPORTED)
			{
				/* audio managed on Host, no command sent to CMOST or DCOP.
				 * Assume the Host mutes the Receiver an only update the internal
				 * status */
				ret = ETAL_RET_SUCCESS;
			}
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			/* audio managed by the STAR connected to the DCOP
			 * We cannot use the ETAL_cmdAudioMute_CMOST because in general
			 * the audio may be routed to a STAR different than the one
			 * recorded in the Receiver status */
			else if ((retosal == OSAL_OK) &&
				(ETAL_directCmdAudioMute_CMOST(hTuner, (tU8)(muteFlag ? 0 : 1)) == OSAL_OK))
			{
				ret = ETAL_RET_SUCCESS;
			}
#endif
			else
			{
				/* Nothing to do */
			}
			break;

		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			if (ETAL_cmdAudioMute_CMOST(hReceiver, (tU8)(muteFlag ? 0 : 1)) == OSAL_OK)
			{
				ret = ETAL_RET_SUCCESS;
			}
#endif
			break;

		case ETAL_BCAST_STD_DRM:
		case ETAL_BCAST_STD_DAB:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		// tmp correction : only mute the CMOST !
/*			if (ETAL_cmdMute_MDR(hReceiver, muteFlag) == OSAL_OK)
			{
				ret = ETAL_RET_SUCCESS;
			}
*/
			// in case of audio routing thru the CMOST, we need also to make sure the CMOST is unmute... 
			// 
			retosal = ETAL_getTunerIdForAudioCommands(hReceiver, &hTuner);
			if (retosal == OSAL_ERROR_NOT_SUPPORTED)
			{
				/* audio managed on Host, no command sent to CMOST or DCOP.
				 * Assume the Host mutes the Receiver an only update the internal
				 * status */
//				 ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ERROR in ETAL_mute  / ETAL_getTunerIdForAudioCommands return OSAL_ERROR_NOT_SUPPORTED\n");
				ret = ETAL_RET_SUCCESS;
			}
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			/* audio managed by the STAR connected to the DCOP
			 * We cannot use the ETAL_cmdAudioMute_CMOST because in general
			 * the audio may be routed to a STAR different than the one
			 * recorded in the Receiver status */
			else if ((retosal == OSAL_OK) &&
			(ETAL_directCmdAudioMute_CMOST(hTuner, (tU8)(muteFlag ? 0 : 1)) == OSAL_OK))
			{
				ret = ETAL_RET_SUCCESS;
			}
			else
			{
//			 	ETAL_tracePrintError(TR_CLASS_APP_ETAL_CMD, "ERROR in ETAL_mute  / ETAL_getTunerIdForAudioCommands return resosal %d, ret = %d\n", retosal, ret);
			}
#endif

#endif
			break;

		default:
			break;
	}
	if (ret == ETAL_RET_SUCCESS)
	{
		ETAL_receiverSetMute(hReceiver, muteFlag);
	}

	return ret;
}

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
#if defined(CONFIG_ETAL_HAVE_FMSTEREO_EVENT_CONTROL) || defined(CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_receiver_check_state_periodic_callback
 *
 **************************/
/*!
 * \brief		Checks if the FM reception changed from mono to stereo or viceversa
 * \details		Periodically invoked to check the quality of the FM reception:
 * 				if it detects that the receiption mono/stereo changed since last
 * 				invocation it sends an #ETAL_INFO_FM_STEREO event to the Host
 * \param[in]	hReceiver - the Receiver handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiver_check_state_periodic_callback(ETAL_HANDLE hReceiver)
{
	EtalBcastQualityContainer quality;
	EtalBcastStandard std;
	EtalStereoStatus stereo_status;
	tU32 stereo_mono_reception;
	tBool vl_releaseLock = FALSE;
	
	if (!ETAL_handleIsReceiver(hReceiver))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}
	
	/* get receiver lock */
	if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}

	// we got the lock.
	// mark it for release
	//
	vl_releaseLock = TRUE;
	
	//check receiver is still valid and has not been destroyed !
	if (false == ETAL_receiverIsValidHandle(hReceiver))
	{
		goto exit;
	}

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_FM:
			/* Check if event FM stereo is already started */
			if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialEventFmStero) == TRUE)
			{
				/* check state mono/stereo */
				if (ETAL_cmdGetReceptionQuality_CMOST(hReceiver, &quality) == ETAL_RET_SUCCESS)
				{
					if (std == ETAL_BCAST_STD_HD_FM)
					{
						stereo_mono_reception = quality.EtalQualityEntries.hd.m_analogQualityEntries.m_StereoMonoReception;
					}
					else
					{
						stereo_mono_reception = quality.EtalQualityEntries.amfm.m_StereoMonoReception;
					}
					if (((stereo_mono_reception == 0 /* mono */) && (ETAL_receiverIsFMAudioMono(hReceiver) == FALSE)) ||
						((stereo_mono_reception == 1 /* stereo */) && (ETAL_receiverIsFMAudioStereo(hReceiver) == FALSE)))
					{
						/* stereo/mono changed, send event */
						stereo_status.m_hReceiver = hReceiver;
						stereo_status.m_isStereo = (stereo_mono_reception != 0) ? TRUE : FALSE;
						ETAL_receiverSetFMAudioStereo(hReceiver, stereo_status.m_isStereo);
						ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_FM_STEREO, (tVoid *)&stereo_status, sizeof(EtalStereoStatus));
					}
				}
			}
			break;
		default:
			break;
	}

exit:

	if (TRUE == vl_releaseLock)
	{
		ETAL_receiverReleaseLock(hReceiver);
	}
		
	return;
}

/***************************
 *
 * ETAL_event_FM_stereo_start
 *
 **************************/
/*!
 * \brief		Starts sending the #ETAL_INFO_FM_STEREO
 * \param[in]	hReceiver - the Receiver handle
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver is not FM,
 * 				or not tuned to a frequency in the FM band, or uses the
 * 				background channel, or is not a STAR type
 * \return		ETAL_RET_IN_PROGRESS - the FM stereo is already started
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_event_FM_stereo_start(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;
	etalReceiverStatusTy *recvp;
	EtalBcastStandard std;
	etalFrequencyBandInfoTy band_info;
	tU32 frequency;

	ret = ETAL_RET_SUCCESS;

	if (!ETAL_handleIsValid(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (ETAL_receiverIsValidHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* Check if hReceiver tuned on FM frequency */
		std = ETAL_receiverGetStandard(hReceiver);
		if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if ((ETAL_receiverGetBandInfo(hReceiver, &band_info) != OSAL_OK) || ((band_info.band & ETAL_BAND_FM_BIT) != ETAL_BAND_FM_BIT))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			frequency = ETAL_receiverGetFrequency(hReceiver);
			if ((frequency == ETAL_INVALID_FREQUENCY) || (frequency < band_info.bandMin) || (frequency > band_info.bandMax))
			{
				ret = ETAL_RET_INVALID_RECEIVER;
			}
			/* Check if event FM stereo is already started */
			else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialEventFmStero) == TRUE)
			{
				ret = ETAL_RET_IN_PROGRESS;
			}
			else
			{
				/* get receiver status */
				recvp = ETAL_receiverGet(hReceiver);
				if (recvp == NULL)
				{
					ret = ETAL_RET_INVALID_RECEIVER;
				}
				else
				{
					/* init default FM audio stereo status regardless of frontend and broadcast standard */
					ETAL_receiverInitFMAudioStereo(hReceiver);

					/* register internal check state periodic callback of foreground channel */
					if (TRUE == ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(recvp->CMOSTConfig.tunerId)))
					{
						if ((recvp->CMOSTConfig.channel == ETAL_CHN_FOREGROUND) || (recvp->CMOSTConfig.channel == ETAL_CHN_BOTH))
						{
							/* receiver quality stereo/mono only for foreground channel */
							if (ETAL_intCbIsRegisteredPeriodic(&ETAL_receiver_check_state_periodic_callback, hReceiver) == FALSE)
							{
								if ((ret = ETAL_intCbRegisterPeriodic(&ETAL_receiver_check_state_periodic_callback, hReceiver, CONFIG_ETAL_RECEIVER_CHECK_STATE_PERIOD)) == ETAL_RET_SUCCESS)
								{
									/* Set special event FM stereo in progress */
									ETAL_receiverSetSpecial(hReceiver, cmdSpecialEventFmStero, cmdActionStart);
								}
							}
						}
						else
						{
							ret = ETAL_RET_INVALID_RECEIVER;
						}
					}
					else
					{
						ret = ETAL_RET_INVALID_RECEIVER;
					}
				}
			}
		}
	}

	return ret;
}

/***************************
 *
 * ETAL_event_FM_stereo_stop
 *
 **************************/
/*!
 * \brief		Stops sending the #ETAL_INFO_FM_STEREO event
 * \param[in]	hReceiver - the Receiver handle
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_event_FM_stereo_stop(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;

	ret = ETAL_RET_SUCCESS;

	if (!ETAL_handleIsValid(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (ETAL_receiverIsValidHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* Check if event FM stereo is already started */
		if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialEventFmStero) == TRUE)
		{
			/* Set special event FM stereo stop */
			ETAL_receiverSetSpecial(hReceiver, cmdSpecialEventFmStero, cmdActionStop);

			/* deregister internal check state periodic callback */
			if (ETAL_intCbIsRegisteredPeriodic(&ETAL_receiver_check_state_periodic_callback, hReceiver) == TRUE)
			{
				ret = ETAL_intCbDeregisterPeriodic(&ETAL_receiver_check_state_periodic_callback, hReceiver);
			}
		}
	}

	return ret;
}

#endif // defined(CONFIG_ETAL_HAVE_FMSTEREO_EVENT_CONTROL) || defined(CONFIG_ETAL_HAVE_ALL_API)
#endif /* CONFIG_ETAL_SUPPORT_CMOST_STAR */

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_config_audio_path
 *
 **************************/
ETAL_STATUS etal_config_audio_path(tU32 tunerIndex, EtalAudioInterfTy intf)
{
	ETAL_STATUS ret;

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

	etalAudioIntfStatusTy vl_intf;


	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_config_audio_path(tunInd: %d, dac: %d, saiOut: %d, saiIn: %d, res: %d, saiSlaMod: %d)",
			tunerIndex, intf.m_dac, intf.m_sai_out, intf.m_sai_in, intf.reserved, intf.m_sai_slave_mode);



	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		vl_intf.bitfield = intf;
		ret = ETAL_configAudioPathInternal(tunerIndex, vl_intf);

		ETAL_statusReleaseLock();
	}
#else
	ret=ETAL_RET_NOT_IMPLEMENTED;
#endif

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_config_audio_path() = %s", ETAL_STATUS_toString(ret));
    return ret;
}

/***************************
 *
 * etal_audio_select
 *
 **************************/
ETAL_STATUS etal_audio_select(ETAL_HANDLE hReceiver, EtalAudioSourceTy src)
{
	ETAL_STATUS ret;

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

	EtalAudioSourceSelectInternal vl_AudioSource;
	
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_audio_select(rec: %d, src: %d)", hReceiver, src);


	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_checkAudioSelectParameters(hReceiver, src);
		if (ret == ETAL_RET_SUCCESS)
		{
			ret = ETAL_audioSelectInternal(hReceiver, src);

			// Internal callback to notify external audio select done
			vl_AudioSource.m_audioSourceSelected = src;
			vl_AudioSource.m_externalRequestInfo = TRUE;
			ETAL_intCbScheduleCallbacks(ETAL_INVALID_HANDLE, callAtAudioSourceSelect, (tVoid *)&vl_AudioSource, sizeof(vl_AudioSource));
		}

		ETAL_statusReleaseLock();
	}
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_audio_select() = %s", ETAL_STATUS_toString(ret));
	return ret;
}


#if defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_force_mono
 *
 **************************/
ETAL_STATUS etal_force_mono(ETAL_HANDLE hReceiver, tBool forceMonoFlag)
{
	ETAL_STATUS ret;

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

	tSInt retosal;
	ETAL_HANDLE hTuner;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_force_mono(rec: %d, forMonFla: %d)", hReceiver, forceMonoFlag);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_checkForceMonoParameters(hReceiver);
		if (ret == ETAL_RET_SUCCESS)
		{
			tU8 fmStereoMode = (tU8)(forceMonoFlag ? 1 : 0);
			EtalBcastStandard std = ETAL_receiverGetStandard(hReceiver);

			switch(std)
			{
				case ETAL_BCAST_STD_HD_FM:
					retosal = ETAL_getTunerIdForAudioCommands(hReceiver, &hTuner);
					if (retosal == OSAL_ERROR_NOT_SUPPORTED)
					{
						/* audio managed on Host, no command sent to CMOST or DCOP.
						 * Assume the Host mutes the Receiver an only update the internal
						 * status */
						ret = ETAL_RET_SUCCESS;
					}
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
					/* audio managed by the STAR connected to the DCOP
					 * We cannot use the ETAL_cmdAudioMute_CMOST because in general
					 * the audio may be routed to a STAR different than the one
					 * recorded in the Receiver status */
					else if ((retosal == OSAL_OK) &&
						(ETAL_directCmdFMStereoMode_CMOST(hTuner, fmStereoMode) == OSAL_OK))
					{
						ret = ETAL_RET_SUCCESS;
					}
#endif
					else
					{
						/* Nothing to do */
					}
					break;
					
				case ETAL_BCAST_STD_FM:
#ifdef CONFIG_ETAL_SUPPORT_CMOST
					if (ETAL_cmdFMStereoMode_CMOST(hReceiver, fmStereoMode) == OSAL_OK)
					{
						ret = ETAL_RET_SUCCESS;
					}
#endif
					break;

				case ETAL_BCAST_STD_DAB:
					/* Not needed in DAB */
					ret = ETAL_RET_NOT_IMPLEMENTED;
					break;

				default:
					ret = ETAL_RET_INVALID_BCAST_STANDARD;
					break;
			}
		}

		ETAL_statusReleaseLock();
	}
#else
		ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_force_mono() = %s", ETAL_STATUS_toString(ret));
	return ret;
}


/***************************
 *
 * etal_mute
 *
 **************************/
ETAL_STATUS etal_mute(ETAL_HANDLE hReceiver, tBool muteFlag)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_mute(rec: %d, mutFla: %d)", hReceiver, muteFlag);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}

	else
	{
		if ((ret = ETAL_checkMuteParameters(hReceiver, muteFlag)) == ETAL_RET_SUCCESS)
		{
			ret = ETAL_mute(hReceiver, muteFlag);
		}

		ETAL_statusReleaseLock();
	}
#else
		ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_mute() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_audio_output_scaling
 *
 **************************/
ETAL_STATUS etal_audio_output_scaling(tU32 volume)
{

	 ETAL_STATUS ret = ETAL_RET_SUCCESS;
 
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
		/*
		 * Memory mappings for CMOST devices
		 */

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		tU32 param[2] = {TDA7707_tunAppCtrlY1_audioMute_maxGain, 0x400000};
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
		tU32 param[2] = {TDA7708_tunAppCtrlY1_audioMute_maxGain, 0x400000};
#endif

   
    tU32 volumeTableList[25] = { 0x000000, 0x08FACD, 0x0A1451, 0x0B504F,
                                 0x0CB2FF, 0x0E411F, 0x100000, 0x11F59A,
                                 0x1428A2, 0x16A09E, 0x1965FE, 0x1C823E,
                                 0x200000, 0x23EB35, 0x285145, 0x2D413C,
                                 0x32CBFD, 0x39047C, 0x400000, 0x47D66B,
                                 0x50A28B, 0x5A8279, 0x6597FA, 0x7208F8,
                                 0x7FFFFF};

    if(volume <= 24)
    {
        param[1] = volumeTableList[volume];
        ret = ETAL_write_parameter_internal(ETAL_handleMakeTuner((ETAL_HINDEX)0), fromAddress, param ,1);
    }
    else
    {
        ret = ETAL_RET_ERROR;
    }
	
#else
			ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

    return ret;
}
#endif // CONFIG_ETAL_HAVE_AUDIO_CONTROL || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_FMSTEREO_EVENT_CONTROL) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_event_FM_stereo_start
 *
 **************************/
ETAL_STATUS etal_event_FM_stereo_start(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_event_FM_stereo_start(rec: %d)", hReceiver);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_event_FM_stereo_start(hReceiver);

		ETAL_statusReleaseLock();
	}
#else
			ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_event_FM_stereo_start() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_event_FM_stereo_stop
 *
 **************************/
ETAL_STATUS etal_event_FM_stereo_stop(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_event_FM_stereo_stop(rec: %d)", hReceiver);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_event_FM_stereo_stop(hReceiver);

		ETAL_statusReleaseLock();
	}
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_event_FM_stereo_stop() = %s", ETAL_STATUS_toString(ret));
#else
				ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

	return ret;
}
#endif // CONFIG_ETAL_HAVE_FMSTEREO_EVENT_CONTROL || CONFIG_ETAL_HAVE_ALL_API
