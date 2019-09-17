//!
//!  \file 		etalapi_tune.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, tune and seek functions
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

/***************************
 * Defines
 **************************/

/***************************
 * Local functions
 **************************/
#if defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
static ETAL_STATUS ETAL_checkSeekStartManualParameters(ETAL_HANDLE hReceiver, tU32 step, tU32 *freq);
static ETAL_STATUS ETAL_checkSeekContinueManualParameters(ETAL_HANDLE hReceiver, tU32 *freq);
static ETAL_STATUS ETAL_checkSeekStopManualParameters(ETAL_HANDLE hReceiver, tU32 *freq);
static ETAL_STATUS ETAL_checkSeek_get_status_manualParameters(ETAL_HANDLE hReceiver, EtalSeekStatus *seekStatus);
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
static ETAL_STATUS ETAL_tuneReceiverInternal_HDRADIO(ETAL_HANDLE hReceiver, tU32 Frequency);
#endif

/***************************
 *
 * ETAL_changeBandInternal
 *
 **************************/
/*!
 * \brief		Sends a change band command to the CMOST
 * \details		Sets the band limits and auto seek step based on the *band*
 * 				parameter. The defaults are defined in the function
 * 				#ETAL_utilityGetDefaultBandLimits.
 * 				For *band* set to #ETAL_BAND_USERFM or #ETAL_BAND_USERAM,
 * 				uses the *fmin* and *fmax* parameter to set the
 * 				band limits; in this case the auto seek step is set to 0.
 * 				Also defines sets the processing features to a default
 * 				if not specified (see #ETAL_receiverSetDefaultProcessingFeatures)
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	band - the frequency band
 * \param[in]	fmin - the minimum frequency, ignored if band is not
 * 				       #ETAL_BAND_USERFM or #ETAL_BAND_USERAM
 * \param[in]	fmax - the maximum frequency, ignored if band is not
 * 				       #ETAL_BAND_USERFM or #ETAL_BAND_USERAM
 * \param[in]	processingFeatures - processing features passed to the CMOST
 * \param[in]	isInternalOnly - Indicate if the change band should be internal only or sent to the CMOST
 *				values TRUE / FALSE
 *			A change band is audible, in early audio case, if tuner is well configured, avoid sending a change band
 *
 * \return		ETAL_RET_ERROR - communication error
 * \return		ETAL_RET_PARAMETER_ERR - undefined *band* 
 * \return		ETAL_RET_INVALID_HANDLE - invalid *hReceiver*
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_changeBandInternal(ETAL_HANDLE hGeneric, EtalFrequencyBand band, tU32 fmin, tU32 fmax, EtalProcessingFeatures processingFeatures, tBool isInternalOnly)
{
	tU32 bandMin, bandMax, step;
	etalReceiverStatusTy *recvp;

	if ((band == ETAL_BAND_USERFM) ||
		(band == ETAL_BAND_USERAM))
	{
		bandMin = fmin;
		bandMax = fmax;
		step = 0;
	}
	else if (ETAL_utilityGetDefaultBandLimits(band, &bandMin, &bandMax, &step) != ETAL_RET_SUCCESS)
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		/* Nothing to do */
	}

	if (ETAL_handleIsReceiver(hGeneric))
	{
		/* set processing features with default values when unspecified */
		recvp = ETAL_receiverGet(hGeneric);
		if (recvp == NULL)
		{
			return ETAL_RET_INVALID_HANDLE;
		}
		ETAL_receiverSetDefaultProcessingFeatures(recvp->currentStandard, recvp->diversity.m_DiversityMode, &(processingFeatures));
	}
	else if (!ETAL_handleIsFrontend(hGeneric))
	{
		return ETAL_RET_INVALID_HANDLE;
	}
	else
	{
		/* Nothing to do */
	}

	if (FALSE == isInternalOnly)
	{
		if (ETAL_cmdChangeBand_CMOST(hGeneric, band, bandMin, bandMax, step, processingFeatures) != OSAL_OK)
		{
			/* the function could return also OSAL_ERROR_INVALID_PARAM
			 * but since the parameters are constructed here or already checked
			 * it can be assumed it will never be returned and any error due to communication */
			return ETAL_RET_ERROR;
		}
	}
	else
	{
		if (ETAL_handleIsReceiver(hGeneric))
		{
			ETAL_receiverSetBandInfo(hGeneric, band, bandMin, bandMax, step);
		}
	}
	
	return ETAL_RET_SUCCESS;
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/***************************
 *
 * ETAL_tuneReceiverInternal_HDRADIO
 *
 **************************/
/*!
 * \brief		Starts the tune FSM for HDRadio
 * \details		Invokes the ETAL_tuneFSM_HDRADIO for a blocking operation;
 * 				if the *hReceiver* supports audio, as decided by
 *				#ETAL_receiverSupportsAudio, requests also the additional
 *				HD audio sync operation to the Tune FSM.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	Frequency - the frequency to tune, in Hz
 * \return		The same as #ETAL_tuneFSM_HDRADIO
 * \see			ETAL_tuneFSM_HDRADIO
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_tuneReceiverInternal_HDRADIO(ETAL_HANDLE hReceiver, tU32 Frequency)
{
	tBool waitAudio;

	waitAudio = ETAL_receiverSupportsAudio(hReceiver);
	return ETAL_tuneFSM_HDRADIO(hReceiver, Frequency, tuneFSMHDNormalResponse, waitAudio, ETAL_HDRADIO_TUNEFSM_API_USER);
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/***************************
 *
 * ETAL_setTunerIdleInternal
 *
 **************************/
/*!
 * \brief		Sets a Receiver to idle mode
 * \details		When a Receiver is destroyed this function is called
 * 				to put its associated devices into idle mode, to reduce
 * 				power consumption. The actual command used depends on the
 * 				device:
 * 				- for DAB DCOP, tune to 0 command
 * 				- for HDRadio not command is available
 * 				- for CMOST, change band to 0
 * \param[in]	hReceiver - the Receiver handle
 * \return		ETAL_RET_ERROR - communication error with the CMOST
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_setTunerIdleInternal(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;
	tU32 device_list;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tSInt retval;
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	EtalProcessingFeatures proc_features;
#endif

	ret = ETAL_RET_SUCCESS;
	device_list = ETAL_cmdRoutingCheck(hReceiver, commandTune);

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
		(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
		{
		/* tune to frequency 0
		 * OSAL_ERROR_TIMEOUT_EXPIRED means no signal detected
		 * on the frequency, which is normal */
		retval = ETAL_cmdTune_MDR(hReceiver, 0, cmdTuneNormalResponse, 0);
		if ((retval != OSAL_OK) && (retval != OSAL_ERROR_TIMEOUT_EXPIRED))
		{
			ret = ETAL_RET_ERROR;
		}
	}
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
		ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
	{
		/* tune to 0 in HDRadio is more similar to a regular tune than a put to idle
		 * better not do anything */
	}
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	/* Finish by putting the CMOST in idle
	 * For a regular Tune, the order is CMOST then DCOP
	 * For a 'set to idle' the order should be 1st DCOP, then CMOST ie other way round */
	if (ETAL_CMDROUTE_TO_CMOST(device_list))
	{
		proc_features.u.m_processing_features = (tU8)ETAL_PROCESSING_FEATURE_NONE;
		if (ETAL_cmdChangeBand_CMOST(hReceiver, ETAL_BAND_UNDEF, 0, 0, 0, proc_features) != OSAL_OK)
		{
			return ETAL_RET_ERROR;
		}
	}
#endif

		if (ret != ETAL_RET_ERROR)
		{
			ETAL_receiverSetFrequency(hReceiver, 0, TRUE);
		}
	
	return ETAL_RET_SUCCESS;
}


/***************************
 *
 * ETAL_isValidFrequency
 *
 **************************/
/*!
 * \brief		Checks if a frequency is valid for a Receiver
 * \details		The function compares the *f* with the band limits
 * 				defined for the *hReceiver*.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	f - frequency
 * \return		TRUE - the *f* is within the *hReceiver*'s band limits
 * \return		FALSE - the *f* is not within the *hReceiver*'s band limits,
 * 				      or the band is not defined for *hReceiver*
 * \callgraph
 * \callergraph
 */
tBool ETAL_isValidFrequency(ETAL_HANDLE hReceiver, tU32 f)
{
	etalFrequencyBandInfoTy band_info;

	if (ETAL_receiverGetBandInfo(hReceiver, &band_info) != OSAL_OK)
	{
		return FALSE;
	}
	if ((band_info.band != ETAL_BAND_UNDEF) &&
		(f >= band_info.bandMin) &&
		(f <= band_info.bandMax))
	{

		return TRUE;
	}
				
	return FALSE;
}

/***************************
 *
 * ETAL_tuneReceiverInternal
 *
 **************************/
/*!
 * \brief		Sends a tune command to a Receiver
 * \details		For DAB the parameter *dcop_action* defines the type of tuner:
 * 				when called with this parameter set to cmdTuneImmediateResponse
 * 				the function uses an alternate DABMW Tune command which
 * 				sends the Response immediately after the Notification (instead
 * 				of waiting until the tune operation is actually complete).
 * 				This mode of operation is used by learn, which uses a different
 * 				mechanism to detect the end of the tune (i.e. the autonotification
 * 				coming from the MDR).
 *
 * 				For all other standards the *dcop_action* parameter is ignored.
 *
 * 				For DAB issues a tune to 0Hz before the real tune to stop
 * 				DAB decoding and avoid database corruption in the DCOP.
 * 				Thus the sequence for DAB is:
 * 				- Tune 0 DCOP
 * 				- Tune *Frequency* CMOST
 * 				- Tune *Frequency* DCOP
 * \param[in]	hReceiver - Receiver handle
 * \param[in]	Frequency - frequency in Hz, 0 is allowed
 * \param[in]	dcop_action - type of operation, see above; only used for DAB Receivers
 * 				              ignored in all other cases
 * \return		#ETAL_RET_NO_DATA - tune operation successful but no signal detected
 * 				                    on the *Frequency*; may be returned only for DAB or HDRadio
 * 				                    *hReceiver*
 * \return		#ETAL_RET_ERROR - communication error or internal error (see #ETAL_queueCommand_HDRADIO)
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_tuneReceiverInternal(ETAL_HANDLE hReceiver, tU32 Frequency, etalCmdTuneActionTy dcop_action)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt retval;
	tU32 device_list;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tU8 cmd[1];
#endif
	tU32 scsr0, freqFG, freqBG;
	ETAL_HANDLE hTuner;
	tBool vl_tuneCmost = TRUE;
	etalReceiverStatusTy *recvp;

	device_list = ETAL_cmdRoutingCheck(hReceiver, commandTune);
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
		(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB) &&
		((dcop_action == cmdTuneNormalResponse) || (dcop_action ==  cmdTuneDoNotWaitResponseDcopDirectResponseOnStatus))
		)
	{
		/*
		 * before actually tuning to the new frequency we need to 
		 * issue a 'tune 0Hz' command to avoid DAB database corruption
		 * This is only required if the DCOP is currently tuned to a DAB frequency
		 */
		if (ETAL_receiverGetFrequency(hReceiver) != ETAL_INVALID_FREQUENCY)
		{
			retval = ETAL_cmdTune_MDR(hReceiver, 0, cmdTuneNormalResponse, 0);
			if ((retval != OSAL_ERROR_TIMEOUT_EXPIRED) &&
				(retval != OSAL_OK))
			{
				ret = ETAL_RET_ERROR;
			}
		}
	}
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	if (ETAL_CMDROUTE_TO_CMOST(device_list))
	{	
		// for early audio FM : 
		// if the receiver is FM & FG, and freq already configured ==> do not tune the CMOST
		
		if ((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_FM) || 
			(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM))
		{
			if (ETAL_receiverGetTunerId(hReceiver, &hTuner) == OSAL_OK)
			{
				if (ETAL_readStatusRegister_CMOST(hTuner, &scsr0, &freqFG, &freqBG) == OSAL_OK)
				{
					recvp = ETAL_receiverGet(hReceiver);
					if ((recvp->CMOSTConfig.channel == ETAL_CHN_FOREGROUND) 
						&&
						(freqFG == Frequency)
						&& 
						(FALSE == recvp->isTunedRequiredAfterChangeBand))
					{
						vl_tuneCmost = FALSE;
					}
				}
				
			}
		}

		if (TRUE == vl_tuneCmost)
		{
			retval = ETAL_cmdTune_CMOST(hReceiver, Frequency);
			if (retval != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
	}
#endif // CONFIG_ETAL_SUPPORT_CMOST
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
		(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
	{
		if (ret == ETAL_RET_SUCCESS)
		{
			// Reset DAB autonotification
			ETAL_ResetDabAutonotification_MDR(ETAL_receiverGet(hReceiver));

			retval = ETAL_cmdTune_MDR(hReceiver, Frequency, dcop_action, 0);

			if (retval == OSAL_ERROR_TIMEOUT_EXPIRED)
			{
				ret = ETAL_RET_NO_DATA;
			}
			else if (retval != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}

			if (dcop_action ==  cmdTuneDoNotWaitResponseDcopDirectResponseOnStatus)
			{
				ETAL_receiverSetSpecial(hReceiver, cmdSpecialTune, cmdActionStart);
			}
		}
	}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
		ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
	{

		// no need for STOP in the present case : the FSM is reset thru 'restart' and the START will get the FSM to work.
		//

		// reset the FSM
		(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);

		if (ret == ETAL_RET_SUCCESS)
		{
			ret = ETAL_tuneReceiverInternal_HDRADIO(hReceiver, Frequency);
		}

		// post a start special to indicate that HD monitoring should now be done
		cmd[0] = (tU8)HDRADIO_SEEK_START_SPECIAL;
		if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
	}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	if (ret != ETAL_RET_ERROR)
	{
		ETAL_receiverSetFrequency(hReceiver, Frequency, TRUE);
	}
	return ret;
}

/***************************
 *
 * ETAL_isSupportedBand
 *
 **************************/
/*!
 * \brief		Checks if a frequency band is compatible with the Receiver
 * \details		Frequency bands may fall in one or more Broadcast Standards.
 * 				The function checks if the *band* is compatible with the
 * 				*hReceiver*'s current Broadcast Standard.
 * \remark		The function assumes the #EtalFrequencyBand is encoded
 * 				in some particular way.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	band - the frequency band to check
 * \return		TRUE if the *band* is compatible, FALSE if it is ETAL_BCAST_STD_UNDEF
 * 				or is not compatible
 * \callgraph
 * \callergraph
 */
static tBool ETAL_isSupportedBand(ETAL_HANDLE hReceiver, EtalFrequencyBand band)
{
	EtalBcastStandard std;

	std = ETAL_receiverGetStandard(hReceiver);
	if (std == ETAL_BCAST_STD_UNDEF)
	{
		return FALSE;
	}

	switch (std)
	{
		case ETAL_BCAST_STD_HD_FM:
			return (((band & ETAL_BAND_HD_BIT) == ETAL_BAND_HD_BIT) && ((band & ETAL_BAND_FM_BIT) == ETAL_BAND_FM_BIT));
		case ETAL_BCAST_STD_HD_AM:
			return (((band & ETAL_BAND_HD_BIT) == ETAL_BAND_HD_BIT) && ((band & ETAL_BAND_AM_BIT) == ETAL_BAND_AM_BIT));
		case ETAL_BCAST_STD_FM:
			return ((band & ETAL_BAND_FM_BIT) == ETAL_BAND_FM_BIT);
		case ETAL_BCAST_STD_AM:
			return ((band & ETAL_BAND_AM_BIT) == ETAL_BAND_AM_BIT);
		case ETAL_BCAST_STD_DRM:
			return ((band & ETAL_BAND_DRM_BIT) == ETAL_BAND_DRM_BIT);
		case ETAL_BCAST_STD_DAB:
			return ((band & ETAL_BAND_DAB_BIT) == ETAL_BAND_DAB_BIT);
		default:
			return FALSE;
	}
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/***************************
 *
 * ETAL_receiverSelectProgram_HDRADIO
 *
 **************************/
/*!
 * \brief		Selects an HDRadio program (MPS or SPS)
 * \details		Sends the HDRadio command to select a service on the
 * 				currently tuned frequency, and if successful updates
 * 				the ETAL internal status with the currently selected
 * 				program.
 * \remark		Does not validate its parameters. In particular
 * 				parameter *prog* should only take one of the
 * 				values of the #tyHDRADIOAudioPrgID enum.
 * \param[in]	hReceiver - the HDRadio Receiver handle
 * \param[in]	prog - the program (or service) to select
 * \return		OSAL_ERROR - hReceiver is not tuned
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - communication error with the device
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverSelectProgram_HDRADIO(ETAL_HANDLE hReceiver, tS8 prog)
{
	tU32 freq;

	freq = ETAL_receiverGetFrequency(hReceiver);
	if (freq == ETAL_INVALID_FREQUENCY)
	{
		return OSAL_ERROR;
	}
	if (ETAL_cmdTuneSelect_HDRADIO(hReceiver, freq, (tyHDRADIOAudioPrgID)prog) != OSAL_OK)
	{
		return OSAL_ERROR_DEVICE_NOT_OPEN;
	}
	ETAL_receiverSetCurrentProgram_HDRADIO(hReceiver, prog);

    /* reset the HD Radio tune state machine */
    (LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDNewService, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
	
	return OSAL_OK;
}

/***************************
 *
 * ETAL_seekHDNeedsTune_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks if during an HDRadio seek operation it is necessary to send a tune command
 * \details		iBiquity specifies that a seek operation on an
 * 				HDRadio service should seek on the SPS if available.
 * 				This function checks id the Receiver has reached Digital Audio
 * 				sync on the current frequency and if SPS services are available
 * 				on that frequency:
 * 				- when seeking down, if the current program is the MPS, force
 * 				a new tune; otherwise (the current program is an SPS), select the
 * 				previous SPS or the MPS
 * 				- when seeking up, if the current program is the last SPS
 * 				in the list of available services for the current frequency,
 * 				force a tune; otherwise select the next service.
 *
 * 				The SPS are ordered based on iBiquity specification 
 * 				(see #tyHDRADIOAudioPrgID).
 *
 * 				The function performs the service selection invoking the
 * 				#ETAL_receiverSelectProgram_HDRADIO. The tune command
 * 				instead must be performed by the caller.
 * \param[in]	hReceiver - handle of the HDRadio Receiver
 * \param[in]	dir - the seek direction
 * \return		TRUE - the caller should tune to the next frequency
 * \return		FALSE - there current frequency contains a service compatible
 * 				        with the seek request, the function changed to that
 * 				        service.
 * \callgraph
 * \callergraph
 */
tBool ETAL_seekHDNeedsTune_HDRADIO(ETAL_HANDLE hReceiver, etalSeekDirectionTy dir)
{
	tS8 curr_prog, prog;
	tU32 prog_num;
	tU32 i;

	if (!ETAL_receiverHasDigitalAudio_HDRADIO(hReceiver))
	{
		return TRUE;
	}
	/*
	 * If digital audio is acquired the current program may be the MPS or
	 * one of the SPS (if present)
	 */
	ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &curr_prog, &prog_num, NULL);
	if (curr_prog == ETAL_INVALID_PROG)
	{
		/*
		 * should not happen
		 */
		return TRUE;
	}

	if (dir == cmdDirectionDown)
	{
		tS8 prev_prog = ETAL_INVALID_PROG;

		/*
		 * HD seek spec requires to skip SPS if going down from MPS
		 */
		if (curr_prog == (tS8)MPS_AUDIO_HD_1)
		{
			return TRUE;
		}
		/*
		 * SPS is selected, search the previous one, it may be
		 * another SPS or the MPS
		 */
		for (i = 0; i < prog_num; i++)
		{
			prog = ETAL_receiverGetService_HDRADIO(hReceiver, i);
			if (prog == curr_prog)
			{
				break;
			}
			prev_prog = prog;
		}
		if (prev_prog == ETAL_INVALID_PROG)
		{
			/*
			 * MPS must be present, this is an abnormal situation
			 */
			ASSERT_ON_DEBUGGING(0);
			return TRUE;
		}
		if (ETAL_receiverSelectProgram_HDRADIO(hReceiver, prev_prog) != OSAL_OK)
		{
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		tS8 next_prog = ETAL_INVALID_PROG;
		tU32 start = 0;

		/*
		 * regardless if the current program is MPS o SPS, search
		 * the next one
		 */
		for (i = 0; i < prog_num; i++)
		{
			prog = ETAL_receiverGetService_HDRADIO(hReceiver, i);
			if (prog == curr_prog)
			{
				start = i + 1;
				break;
			}
		}
		for (i = start; i < prog_num; i++)
		{
			prog = ETAL_receiverGetService_HDRADIO(hReceiver, i);
			if (prog != ETAL_INVALID_PROG)
			{
				next_prog = prog;
				break;
			}
		}
		if (next_prog == ETAL_INVALID_PROG)
		{
			/*
			 * we have completed the list of available programs,
			 * search next station
			 */
			return TRUE;
		}
		if (ETAL_receiverSelectProgram_HDRADIO(hReceiver, next_prog) != OSAL_OK)
		{
			return TRUE;
		}
		return FALSE;
	}
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

#if defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_checkSeekStartManualParameters
 *
 **************************/
/*!
 * \brief		Checks the #etal_seek_start_manual parameters
 * \details		Performs the following checks:
 * 				- the *hReceiver* is valid
 * 				- it is tuned, to DAB/AF/FM/HD Broadcast Standard
 * 				- it is not employed in a special operation (seek, scan, learn)
 * 				- the *freq* parameter is non-NULL
 * 				- for AM/FM/HD, the *step* parameter is within some
 * 				hard-coded bounds (see #ETAL_SEEK_MAX_FM_STEP and
 * 				#ETAL_SEEK_MAX_AM_STEP).
 * \param[in]	hReceiver - Receiver handle
 * \param[in]	step - seek step in kHz - ignored for DAB
 * \param[in]	freq - pointer to integer, expected to be non-NULL
 * \return		#ETAL_RET_INVALID_RECEIVER - formally invalid handle
 * \return		#ETAL_RET_IN_PROGRESS - *hReceiver* is already employed in a
 * 				                        seek or similar operation
 * \return		#ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return		#ETAL_RET_PARAMETER_ERR
 * \return		#ETAL_RET_ERROR - *hReceiver* is not tuned
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_checkSeekStartManualParameters(ETAL_HANDLE hReceiver, tU32 step, tU32 *freq)
{
	etalFrequencyBandInfoTy band_info;

	/* Check receiver status */
	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency) ||
			 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessEstimation) ||
			 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessSwitching))
	{
		return ETAL_RET_IN_PROGRESS;
	}
	else if(ETAL_receiverGetFrequency(hReceiver) == ETAL_INVALID_FREQUENCY)
	{
		return ETAL_RET_ERROR;
	}
	else if (freq == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		/* Nothing to do */
	}

	/* Check step: mainly done to avoid parameters passed in Hz instead of KHz.
	 * Also check Broadcast Standard */
	switch (ETAL_receiverGetStandard(hReceiver))
	{
		case ETAL_BCAST_STD_DAB:
			/* ignore the parameter */
			break;

		case ETAL_BCAST_STD_FM:
			if (step > ETAL_SEEK_MAX_FM_STEP)
			{
				return ETAL_RET_PARAMETER_ERR;
			}
			break;

		case ETAL_BCAST_STD_AM:
			if (step > ETAL_SEEK_MAX_AM_STEP)
			{
				return ETAL_RET_PARAMETER_ERR;
			}
			break;

		case ETAL_BCAST_STD_HD_FM:
			if (ETAL_receiverGetBandInfo(hReceiver, &band_info) == OSAL_OK)
			{
				if ((band_info.band == ETAL_BAND_FMUS) ||
					(band_info.band == ETAL_BAND_HD))
				{
					if (step > ETAL_SEEK_MAX_FM_STEP)
					{
						return ETAL_RET_PARAMETER_ERR;
					}
				}
				else
				{
					return ETAL_RET_PARAMETER_ERR;
				}
			}
			else
			{
				return ETAL_RET_PARAMETER_ERR;
			}
			break;

		case ETAL_BCAST_STD_HD_AM:
			if (ETAL_receiverGetBandInfo(hReceiver, &band_info) == OSAL_OK)
			{
				if (band_info.band == ETAL_BAND_MWUS)
				{
					if (step > ETAL_SEEK_MAX_AM_STEP)
					{
						return ETAL_RET_PARAMETER_ERR;
					}
				}
				else
				{
					return ETAL_RET_PARAMETER_ERR;
				}
			}
			else
			{
				return ETAL_RET_PARAMETER_ERR;
			}
			break;
		default:
			return ETAL_RET_INVALID_BCAST_STANDARD;
	}
	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_checkSeekContinueManualParameters
 *
 **************************/
/*!
 * \brief		Checks the #etal_seek_continue_manual parameters
 * \details		Performs the following checks:
 * 				- the *hReceiver* is valid and currently employed in a manual seek
 * 				operation
 * 				- the *freq* parameter is non-NULL
 * \details		For HDRadio receivers the first condition is not checked
 * 				because the seek algo may jump to the next SPS on the same
 * 				frequency. In this case no command is issued to the CMOST and
 * 				thus the #cmdSpecialManualSeek is not set.
 * \param[in]	hReceiver - Receiver handle
 * \param[in]	freq - pointer to integer, expected to be non-NULL
 * \return		#ETAL_RET_INVALID_RECEIVER - formally invalid Receiver handle, or
 * 				                             Receiver not currently performing a manual seek
 * \return		#ETAL_RET_PARAMETER_ERR
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_checkSeekContinueManualParameters(ETAL_HANDLE hReceiver, tU32 *freq)
{
	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	/* Normally we check here if there is a special operation in progress,
	 * but for HDRadio receiver it may happen that the flag is not set
	 * but the command is already in progress. That is because the HD seek algo
	 * may jump to the next SPS on the same frequency and in this case the flag
	 * is not set (because no seek command is sent to the CMOST).
	 * Thus we need to bypass the check for HDRadio receivers */
	else if (!ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualSeek) && !ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
	{
		/* continue command must be issued on a Receiver having
		 * seek in progress, otherwise it's an invalid receiver */
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if (freq == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		/* Nothing to do */
	}

	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_checkSeekStopManualParameters
 *
 **************************/
/*!
 * \brief		Checks the #etal_seek_stop_manual parameters
 * \details		Performs the following checks:
 * 				- the *hReceiver* is valid and currently employed in a manual seek
 * 				operation on DAB/AM/FM/HD Broadcast Standard
 * 				- the *freq* parameter is non-NULL
 * \param[in]	hReceiver - Receiver handle
 * \param[in]	freq - pointer to integer, expected to be non-NULL
 * \return		#ETAL_RET_INVALID_RECEIVER - formally invalid handle, or *hReceiver*
 * 				                             not employed in manual seek operation
 * \return		#ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return		#ETAL_RET_PARAMETER_ERR
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_checkSeekStopManualParameters(ETAL_HANDLE hReceiver, tU32 *freq)
{
	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if ((ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM) &&
			 (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_AM) &&
			 !ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)) &&
			 (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_DAB))
	{
		return ETAL_RET_INVALID_BCAST_STANDARD;
	}
	else if (freq == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}
 	else if (!ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualSeek))
 	{
		/* stop command must be issued on a Receiver having
		 * seek in progress, otherwise it's an invalid receiver */
 		return ETAL_RET_INVALID_RECEIVER;
 	}
	else
	{
		/* Nothing to do */
	}

	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_checkSeek_get_status_manualParameters
 *
 **************************/
/*!
 * \brief		Checks the #etal_seek_get_status_manual parameters
 * \details		Performs the following checks:
 * 				- the *hReceiver* is valid and currently employed in a manual seek
 * 				operation on DAB/AM/FM/HD Broadcast Standard
 * 				- the *hReceiver* supports quality measures
 * 				- the *seekStatus* parameter is non-NULL
 * \param[in]	hReceiver - Receiver handle
 * \param[in]	seekStatus - pointer to integer, expected to be non-NULL
 * \return		#ETAL_RET_INVALID_RECEIVER - formally invalid handle, or *hReceiver*
 * 				                             does not support quality measure
 * \return		#ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return		#ETAL_RET_PARAMETER_ERR
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_checkSeek_get_status_manualParameters(ETAL_HANDLE hReceiver, EtalSeekStatus *seekStatus)
{
	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if (!ETAL_receiverSupportsQuality(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if ((ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM) &&
			 (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_AM) &&
			 !ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)) &&
			 (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_DAB))
	{
		return ETAL_RET_INVALID_BCAST_STANDARD;
	}
	else if(seekStatus == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		/* Nothing to do */
	}
	return ETAL_RET_SUCCESS;
}
#endif // CONFIG_ETAL_HAVE_MANUAL_SEEK || CONFIG_ETAL_HAVE_ALL_API

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_change_band_receiver
 *
 **************************/
/*!
 * \brief		Sets the frequency band on which the Receiver operates
 * \details		The frequency band is used by ETAL for parameter checks and
 * 				for commands based on automatic seek.
 * 				The function can also be used to set the processing features
 * 				(see #EtalProcessingFeatures) although using this feature
 * 				is **deprecated** since it may disrupt ETAL's internal state.
 *
 * 				For *band* set to #ETAL_BAND_USERFM or #ETAL_BAND_USERAM the
 * 				parameters *fmin* and *fmax* define the min and max band limits,
 * 				in KHz. For all other values of *band* the parameters are
 * 				ignored.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	band - the requested frequency band; must be compatible with the
 * 				       *hReceiver*'s Broadcast Standard
 * \param[in]	fmin - lower band frequency, in KHz, for #ETAL_BAND_USERFM or #ETAL_BAND_USERAM
 * \param[in]	fmax - higher band frequency, in KHz, for #ETAL_BAND_USERFM or #ETAL_BAND_USERAM
 * \param[in]	processingFeatures - it is recommended to set to #ETAL_PROCESSING_FEATURE_UNSPECIFIED
 * \return		#ETAL_RET_INVALID_RECEIVER
 * \return		#ETAL_RET_PARAMETER_ERR - invalid *band*
 * \return		#ETAL_RET_INVALID_HANDLE - invalid *hReceiver*
 * \return		#ETAL_RET_ERROR - communication error
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_change_band_receiver(ETAL_HANDLE hReceiver, EtalFrequencyBand band, tU32 fmin, tU32 fmax, EtalProcessingFeatures processingFeatures)
{
	ETAL_STATUS ret;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_change_band_receiver(rec: %d, ban: 0x%08x, fmin: %d, fmax: %d, proFea: 0x%x)", hReceiver, band, fmin, fmax, processingFeatures.u.m_processing_features);

	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (!ETAL_isSupportedBand(hReceiver, band))
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else
		{
			ret = ETAL_changeBandInternal(hReceiver, band, fmin, fmax, processingFeatures, FALSE);
		}

		ETAL_receiverReleaseLock(hReceiver);
	}
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_change_band_receiver() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

#if defined (CONFIG_ETAL_HAVE_TUNE_BLOCKING) || defined (CONFIG_ETAL_HAVE_TUNE_BOTH) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_tune_receiver
 *
 **************************/
/*!
 * \brief		Tunes the Receiver to a frequency
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	Frequency - the requested frequency in Hz
 * \return		#ETAL_RET_INVALID_HANDLE - formally invalid *hReceiver*
 * \return		#ETAL_RET_INVALID_RECEIVER - *hReceiver* not configured
 * \return		#ETAL_RET_PARAMETER_ERR - invalid *Frequency*
 * \return		#ETAL_RET_ERROR - communication error
 * \return		#ETAL_RET_IN_PROGRESS - the *hReceiver* is currently used for
 * 				                        an operation involving a tune operation
 * \return		#ETAL_RET_NO_DATA - tune operation successful but no signal detected
 * 				                    on the *Frequency*; may be returned only for DAB or HDRadio
 * 				                    *hReceiver*
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_tune_receiver(ETAL_HANDLE hReceiver, tU32 Frequency)
{
	ETAL_STATUS ret;
	EtalTuneInfoInternal vl_TuneStatusInternal;
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	tBool vl_serviceFollowingHasBeenDisabled = false;
#endif

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_tune_receiver(rec: %d, fre: %d)", hReceiver, Frequency);

	// Disable Service Following if needed
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	if (true == DABWM_ServiceFollowing_ExtInt_IsEnable())
	{
		if (DABMW_ServiceFollowing_ExtInt_DisableSF() == OSAL_OK)
		{
			vl_serviceFollowingHasBeenDisabled = true;
		}
		else
		{
			/* never gets here */
			ret = ETAL_RET_ERROR;
		}
	}
#endif

	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (!ETAL_isValidFrequency(hReceiver, Frequency))
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency) ||
				 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessEstimation) ||
				 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessSwitching))
		{
			ret = ETAL_RET_IN_PROGRESS;
		}
		else
		{
			ret = ETAL_tuneReceiverInternal(hReceiver, Frequency, cmdTuneNormalResponse);
		}

		// external request for Tune is currently only enabled in DAB.

		// Internal callback
		vl_TuneStatusInternal.m_receiverHandle = hReceiver;
		vl_TuneStatusInternal.m_Frequency = Frequency;
		vl_TuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
		vl_TuneStatusInternal.m_syncInternal = ETAL_TUNESTATUS_SYNCMASK_FOUND;
		ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&vl_TuneStatusInternal, sizeof(vl_TuneStatusInternal));
		
		ETAL_receiverReleaseLock(hReceiver);

		// re-enable Service Following
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
		if (true == vl_serviceFollowingHasBeenDisabled)
		{
			if (DABMW_ServiceFollowing_ExtInt_ActivateSF() == OSAL_OK)
			{
				vl_serviceFollowingHasBeenDisabled = false;
			}
			else
			{
				/* never gets here */
				ret = ETAL_RET_ERROR;
			}
		}
#endif
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_tune_receiver() = %s (rec: %d)", ETAL_STATUS_toString(ret), hReceiver);
	return ret;
}
#endif // CONFIG_ETAL_HAVE_TUNE_BLOCKING || CONFIG_ETAL_HAVE_TUNE_BOTH || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_TUNE_ASYNC) || defined (CONFIG_ETAL_HAVE_TUNE_BOTH) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_tune_receiver_async
 *
 **************************/
/*!
 * \brief		Tunes the Receiver to a frequency end returns immediately
 * \details		Instead of waiting for the tune operation to complete
 * 				and provide the status of the operation to the caller
 * 				as the #etal_tune_receiver, this function starts the
 * 				tune operation and returns immediately. The result of the
 * 				operation is notified to the caller later, asynchronously,
 * 				through an #ETAL_INFO_TUNE event.
 *
 * 				This function is only provided for HDRadio Receivers,
 * 				for which the tune operation may require a long time to 
 * 				acquire the audio (~7 seconds).
 * \param[in]	hReceiver - the HDRadio Receiver handle
 * \param[in]	Frequency - the requested frequency in Hz
 * \return		#ETAL_RET_INVALID_HANDLE - formally invalid *hReceiver*
 * \return		#ETAL_RET_INVALID_RECEIVER - *hReceiver* not configured
 * \return		#ETAL_RET_PARAMETER_ERR - invalid *band*
 * \return		#ETAL_RET_ERROR - communication error
 * \return		#ETAL_RET_IN_PROGRESS - the *hReceiver* is currently used for
 * 				                        an operation involving a tune operation
 * \return		#ETAL_RET_SUCCESS
 * \see			ETAL_tuneFSM_HDRADIO
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_tune_receiver_async(ETAL_HANDLE hReceiver, tU32 Frequency)
{
	ETAL_STATUS ret;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tU8 cmd[1];

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_tune_receiver_async(rec: %d, fre: %d)", hReceiver, Frequency);
	
	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (!ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (!ETAL_isValidFrequency(hReceiver, Frequency))
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency) ||
				 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessEstimation) ||
				 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessSwitching))
		{
			ret = ETAL_RET_IN_PROGRESS;
		}
		else
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
			if (ETAL_cmdTune_CMOST(hReceiver, Frequency) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
			else
			{
				ETAL_receiverSetFrequency(hReceiver, Frequency, TRUE);
			}
#endif // CONFIG_ETAL_SUPPORT_CMOST
			if (ret == ETAL_RET_SUCCESS)
			{
				(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
				
				/* start the FSM that sends the tune command to the DCOP, waits the response
				 * and sends event to the API caller */
				cmd[0] = (tU8)HDRADIO_SEEK_START_SPECIAL;
				if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
				{
					ret = ETAL_RET_ERROR;
				}
			}
		}

		ETAL_receiverReleaseLock(hReceiver);
	}
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_tune_receiver_async() = %s", ETAL_STATUS_toString(ret));
	return ret;
}
#endif // CONFIG_ETAL_HAVE_TUNE_ASYNC || CONFIG_ETAL_HAVE_TUNE_ASYNC || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * etal_get_receiver_frequency
 *
 **************************/
/*!
 * \brief		Returns the currently tuned frequency
 * \details		ETAL assumes a Receiver is composed of a Tuner (CMOST device)
 * 				and optionally a DCOP. This command reads the frequency
 * 				to which the Tuner is currently tuned to, directly from
 * 				the Tuner registers.
 *
 * 				In case of dual Frontend CMOST devices, the API reads the
 * 				frequency only from the Frontend attached to the *hReceiver*.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	pFrequency - poiner to location where the function stores the frequency
 * \return		#ETAL_RET_INVALID_HANDLE - formally invalid *hReceiver*
 * \return		#ETAL_RET_INVALID_RECEIVER - *hReceiver* not configured
 * \return		#ETAL_RET_PARAMETER_ERR - NULL *pFrequency*
 * \return		#ETAL_RET_NOT_IMPLEMENTED - ETAL built without CMOST support
 * \return		#ETAL_RET_ERROR - communication error with the device
 * \return		#ETAL_RET_SUCCESS
 * \see			ETAL_readStatusRegister_CMOST
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_get_receiver_frequency(ETAL_HANDLE hReceiver, tU32* pFrequency)
{
	ETAL_STATUS ret;
	tU32 *f0 = NULL, *f1 = NULL;
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hTuner;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_receiver_frequency(rec: %d)", hReceiver);

	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (pFrequency == NULL)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else
		{
			recvp = ETAL_receiverGet(hReceiver);
			if (recvp == NULL)
			{
				/* will never happen, already checked by ETAL_receiverGetLock
				 * done only to avoid SCC warning */
				ret = ETAL_RET_INVALID_HANDLE;
			}
			else
			{
				switch (recvp->CMOSTConfig.channel)
				{
					case ETAL_CHN_BOTH:
						f0 = pFrequency;
						break;

					case ETAL_CHN_FOREGROUND:
						f0 = pFrequency;
						break;
	
					case ETAL_CHN_BACKGROUND:
						f1 = pFrequency;
						break;
	
					default:
						ret = ETAL_RET_INVALID_RECEIVER;
						break;
				}
				if (ret == ETAL_RET_SUCCESS)
				{
					if (ETAL_receiverGetTunerId(hReceiver, &hTuner) != OSAL_OK)
					{
						/* since the hReceiver was already validated above,
						 * we may get here only if CMOST support is not built in ETAL */
						ret = ETAL_RET_NOT_IMPLEMENTED;
					}
					else
					{
						if (ETAL_readStatusRegister_CMOST(hTuner, NULL, f0, f1) != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
					}
				}
			}
		}

		ETAL_receiverReleaseLock(hReceiver);
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_receiver_frequency() = %s", ETAL_STATUS_toString(ret));
	if(pFrequency != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Freq: %d", *pFrequency);
	}
	return ret;
}

#if defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_seek_start_manual
 *
 **************************/
/*!
 * \brief		Start a manual seek operation
 * \details		The function stores the seek parameters and performs the first
 * 				seek step; subsequent seek steps must be performed
 * 				through the #etal_seek_continue_manual API.
 * \param[in]	hReceiver - the Receiver handle; supports DAB, HDRadio, AM and FM
 * \param[in]	direction - the seek direction 
 * \param[in]	step - the seek step in kHz; ignored for DAB
 * \param[out]	freq - pointer to location where the function stores the frequency
 * 				       on which the tuner stopped at the end of the seek step
 * \return		#ETAL_RET_INVALID_HANDLE - formally invalid *hReceiver*
 * \return		#ETAL_RET_INVALID_RECEIVER - *hReceiver* not configured
 * \return		#ETAL_RET_PARAMETER_ERR - NULL *pFrequency*
 * \return		#ETAL_RET_IN_PROGRESS - *hReceiver* is already employed in a
 * 				                        seek or similar operation
 * \return		#ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return		#ETAL_RET_NOT_IMPLEMENTED - ETAL built without CMOST support
 * \return		#ETAL_RET_NO_DATA - tune operation successful but no signal detected
 * 				                    on the *Frequency*; may be returned only for DAB or HDRadio
 * 				                    *hReceiver*
 * \return		#ETAL_RET_ERROR - communication error with the device
 * \return		#ETAL_RET_SUCCESS
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_seek_start_manual(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, tU32 *freq)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	EtalTuneInfoInternal tuneStatusInternal;
    tBool need_tune = TRUE;
    tU32 device_list;
	etalReceiverStatusTy *recvp = NULL;
#endif //CONFIG_ETAL_SUPPORT_CMOST

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
    tU8 cmd[1];
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_seek_start_manual(rec: %d, dir: %d, ste: %d)", hReceiver, direction, step);

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_checkSeekStartManualParameters(hReceiver, step, freq);
		if(ret == ETAL_RET_SUCCESS)
		{
			*freq = ETAL_INVALID_FREQUENCY;
			device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);

			recvp = ETAL_receiverGet(hReceiver);
			if (recvp == NULL)
			{
				/* will never happen, hReceiver already checked in ETAL_receiverGetLock
				 * done only to avoid SCC warning */
				return ETAL_RET_INVALID_HANDLE;
			}
			recvp->seekCfg.direction = direction;
			recvp->seekCfg.step      = step;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			if (ETAL_CMDROUTE_TO_DCOP(device_list) && (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
			{
				etalFrequencyBandInfoTy band_info;
				if (OSAL_OK != ETAL_receiverGetBandInfo(hReceiver,&band_info))
				{
					ret = ETAL_RET_ERROR;
				}
				else
				{
					/* Get next frequency from current frequency in the chosen direction */
					*freq = DABMW_GetNextFrequencyFromFreq(ETAL_receiverGetFrequency(hReceiver),DABMW_TranslateEtalBandToDabmwBand(band_info.band), (direction == cmdDirectionUp) ? TRUE : FALSE);
					if (*freq == DABMW_INVALID_FREQUENCY)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
					    ret = ETAL_tuneReceiverInternal(hReceiver, *freq, cmdTuneNormalResponse);
						if (ETAL_RET_ERROR != ret)
						{
							/* Set manual seek flag to active */
							ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualSeek, cmdActionStart);

							/* Update the current frequency */
							ETAL_receiverSetFrequency(hReceiver, *freq, TRUE);

							/* Internal callback : callAtTuneFrequency */
							tuneStatusInternal.m_receiverHandle		 = hReceiver;
							tuneStatusInternal.m_Frequency           = *freq;
							tuneStatusInternal.m_serviceId           = ETAL_INVALID_PROG;
							tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
							tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
							ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
						}
					}
				}
			}
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			if (ETAL_CMDROUTE_TO_DCOP(device_list) && ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
			{
				/*
				 * In case we are already tuned HD requires some special
				 * processing before possibly jumping to a different frequency
				 */
				need_tune = ETAL_seekHDNeedsTune_HDRADIO(hReceiver, recvp->seekCfg.direction);

				if (!need_tune)
				{
					*freq = ETAL_receiverGetFrequency(hReceiver);

					/* Update the current frequency */
					ETAL_receiverSetFrequency(hReceiver, *freq, TRUE);

					/* Internal callback : callAtTuneFrequency */
					tuneStatusInternal.m_receiverHandle      = hReceiver;
					tuneStatusInternal.m_Frequency           = *freq;
					ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &(tuneStatusInternal.m_serviceId), NULL, NULL);
					tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
					tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
					ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
				}
				else
				{
					/* Reset HD FSM */
					cmd[0] = (tU8)HDRADIO_SEEK_STOP_SPECIAL;
					if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
						(LINT_IGNORE_RET)ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
					}
				}
			}
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

			if(need_tune)
			{
				if (ETAL_CMDROUTE_TO_CMOST(device_list) ||
					(ETAL_CMDROUTE_TO_DCOP(device_list) && ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver))))
				{
					if (ETAL_cmdSeekStart_CMOST(hReceiver, recvp->seekCfg.direction, recvp->seekCfg.step, cmdManualModeStart, cmdAudioMuted, TRUE, FALSE) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
						/* Set manual seek flag to active */
						ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualSeek, cmdActionStart);

						/* memorize direction parameter it may be useful */
						recvp->seekCfg.direction = direction;

						/* Get CMOST current frequency */
						if (ETAL_cmdSeekGetStatus_CMOST(hReceiver, NULL, NULL, NULL, freq, NULL) != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
						else if(*freq != 0)
						{
							/* Update the current frequency */
							ETAL_receiverSetFrequency(hReceiver, *freq, TRUE);
						}
						else
						{
							*freq = ETAL_INVALID_FREQUENCY;
						}

						/* Internal callback : callAtTuneFrequency */
						tuneStatusInternal.m_receiverHandle      = hReceiver;
						tuneStatusInternal.m_Frequency           = *freq;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
						ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &(tuneStatusInternal.m_serviceId), NULL, NULL);
#else
						tuneStatusInternal.m_serviceId           = ETAL_INVALID_PROG;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
						tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
						tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
						ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
					}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					if (ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
					{
						// post a start special to indicate that HD monitoring should now be done
						cmd[0] = (tU8)HDRADIO_SEEK_START_SPECIAL;
						if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
					}
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				}
			}
		}
		ETAL_receiverReleaseLock(hReceiver);
	}
#else //CONFIG_ETAL_SUPPORT_CMOST
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif //CONFIG_ETAL_SUPPORT_CMOST

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_seek_start_manual() = %s", ETAL_STATUS_toString(ret));
	if(freq != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Freq: %d", *freq);
	}
	return ret;
}

/***************************
 *
 * etal_seek_continue_manual
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_seek_continue_manual(ETAL_HANDLE hReceiver, tU32 *freq)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	tU32 device_list;
	etalSeekModeTy seekMode;
	etalReceiverStatusTy *recvp = NULL;
    tBool need_tune = TRUE;
    EtalTuneInfoInternal tuneStatusInternal;
#endif //CONFIG_ETAL_SUPPORT_CMOST

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
    tU8 cmd[1];
#endif //CONFIG_ETAL_SUPPORT_CMOST

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_seek_continue_manual(rec: %d)", hReceiver);

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
	ret = ETAL_receiverGetLock(hReceiver);
	if(ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_checkSeekContinueManualParameters(hReceiver, freq);
		if(ret == ETAL_RET_SUCCESS)
		{
			*freq = ETAL_INVALID_FREQUENCY;
			recvp = ETAL_receiverGet(hReceiver);

			if(recvp != NULL)
			{
				device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
				if (ETAL_CMDROUTE_TO_DCOP(device_list) && (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
				{
					etalFrequencyBandInfoTy band_info;
					if (OSAL_OK != ETAL_receiverGetBandInfo(hReceiver,&band_info))
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
						/* Get next frequency from current frequency in the chosen direction */
						*freq = DABMW_GetNextFrequencyFromFreq(ETAL_receiverGetFrequency(hReceiver),DABMW_TranslateEtalBandToDabmwBand(band_info.band), (recvp->seekCfg.direction == cmdDirectionUp) ? TRUE : FALSE);
						if (*freq == DABMW_INVALID_FREQUENCY)
						{
							ret = ETAL_RET_ERROR;
						}
						else
						{
							ret = ETAL_tuneReceiverInternal(hReceiver, *freq, cmdTuneNormalResponse);
							if (ETAL_RET_ERROR != ret)
							{
								/* Update the current frequency */
								ETAL_receiverSetFrequency(hReceiver, *freq, TRUE);

								/* Internal callback : callAtTuneFrequency */
								tuneStatusInternal.m_receiverHandle      = hReceiver;
								tuneStatusInternal.m_Frequency           = *freq;
								tuneStatusInternal.m_serviceId           = ETAL_INVALID_PROG;
								tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
								tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
								ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
							}
						}
					}
				}
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				if (ETAL_CMDROUTE_TO_DCOP(device_list) && ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
				{
					need_tune = ETAL_seekHDNeedsTune_HDRADIO(hReceiver, recvp->seekCfg.direction);

					if (!need_tune)
					{
						*freq = ETAL_receiverGetFrequency(hReceiver);

						/* Update the current frequency */
						ETAL_receiverSetFrequency(hReceiver, *freq, TRUE);

						/* Internal callback : callAtTuneFrequency */
						tuneStatusInternal.m_receiverHandle      = hReceiver;
						tuneStatusInternal.m_Frequency           = *freq;
						ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &(tuneStatusInternal.m_serviceId), NULL, NULL);
						tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
						tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
						ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
					}
					else
					{
						/* Reset HD FSM */
						cmd[0] = (tU8)HDRADIO_SEEK_STOP_SPECIAL;
						if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
						else
						{
							(LINT_IGNORE_RET)ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
						}
					}
				}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

				if(need_tune)
				{
					if (ETAL_CMDROUTE_TO_CMOST(device_list) ||
						(ETAL_CMDROUTE_TO_DCOP(device_list) && ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver))))
					{
						if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualSeek) == false)
						{
							seekMode = cmdManualModeStart;
						}
						else
						{
							seekMode = cmdContinue;
						}

						/* Send TUNER_seek_start */
						if (ETAL_cmdSeekStart_CMOST(hReceiver, recvp->seekCfg.direction, recvp->seekCfg.step, seekMode, cmdAudioMuted, FALSE, FALSE) != OSAL_OK)
						{
							ret = ETAL_RET_ERROR;
						}
						else
						{
							if (seekMode == cmdManualModeStart)
							{
								/* Set manual seek flag to active */
								ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualSeek, cmdActionStart);
							}

							/* Get CMOST current frequency */
							if (ETAL_cmdSeekGetStatus_CMOST(hReceiver, NULL, NULL, NULL, freq, NULL) != OSAL_OK)
							{
								ret = ETAL_RET_ERROR;
							}
							else if(*freq != 0)
							{
								/* Update the current frequency */
								ETAL_receiverSetFrequency(hReceiver, *freq, TRUE);
							}
							else
							{
								*freq = ETAL_INVALID_FREQUENCY;
							}

							/* Internal callback : callAtTuneFrequency */
							tuneStatusInternal.m_receiverHandle      = hReceiver;
							tuneStatusInternal.m_Frequency           = *freq;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
							ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &(tuneStatusInternal.m_serviceId), NULL, NULL);
#else
							tuneStatusInternal.m_serviceId           = ETAL_INVALID_PROG;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
							tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
							tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
							ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
						}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
						if (ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
						{
							// post a start special to indicate that HD monitoring should now be done
							cmd[0] = (tU8)HDRADIO_SEEK_START_SPECIAL;
							if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
							{
								ret = ETAL_RET_ERROR;
							}

						}
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					}
				}
			}
			else
			{
				ret = ETAL_RET_ERROR;
			}
		}
		ETAL_receiverReleaseLock(hReceiver);
	}
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif //CONFIG_ETAL_SUPPORT_CMOST

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_seek_continue_manual() = %s", ETAL_STATUS_toString(ret));
	if(freq != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Freq: %d", *freq);
	}

	return ret;
}

/***************************
 *
 * etal_seek_stop_manual
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_seek_stop_manual(ETAL_HANDLE hReceiver, etalSeekAudioTy exitSeekAction, tU32 *freq)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt vl_res = OSAL_OK;
	EtalTuneInfoInternal tuneStatusInternal;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_seek_stop_manual(rec: %d, exiSeeAct: %d)", hReceiver, exitSeekAction);

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
	ret = ETAL_receiverGetLock(hReceiver);
	if(ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_checkSeekStopManualParameters(hReceiver, freq);
		if(ret == ETAL_RET_SUCCESS)
		{
			tU32 device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			if (ETAL_CMDROUTE_TO_DCOP(device_list) && (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
			{
				/* Unset Manual seek flag */
				ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualSeek, cmdActionStop);
			}
#endif

			if (ETAL_CMDROUTE_TO_CMOST(device_list) ||
				(ETAL_CMDROUTE_TO_DCOP(device_list) && ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver))))
			{
				*freq = ETAL_INVALID_FREQUENCY;

				if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualSeek) == true)
				{
					/* Send TUNER_seek_end */
					if (ETAL_cmdSeekEnd_CMOST(hReceiver, cmdAudioUnmuted /* Unused in manual seek */ ) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else if (ETAL_cmdSeekGetStatus_CMOST(hReceiver, NULL, NULL, NULL, freq, NULL) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
						if(*freq != 0)
						{
							/* Update the current frequency */
							ETAL_receiverSetFrequency(hReceiver, *freq, TRUE);
						}
						else
						{
							*freq = ETAL_INVALID_FREQUENCY;
						}

						/* Internal callback : callAtTuneFrequency */
						tuneStatusInternal.m_receiverHandle      = hReceiver;
						tuneStatusInternal.m_Frequency           = *freq;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
						ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &(tuneStatusInternal.m_serviceId), NULL, NULL);
#else
						tuneStatusInternal.m_serviceId           = ETAL_INVALID_PROG;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
						tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
						tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
						ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
					}

					/* Unset Manual seek flag */
					ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualSeek, cmdActionStop);
				}
				else
				{
					tuneStatusInternal.m_receiverHandle      = hReceiver;
					tuneStatusInternal.m_Frequency           = ETAL_receiverGetFrequency(hReceiver);
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &(tuneStatusInternal.m_serviceId), NULL, NULL);
#else
					tuneStatusInternal.m_serviceId           = ETAL_INVALID_PROG;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalTuneRequestInProgress);
					tuneStatusInternal.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
					ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
				}

				/* Mute the audio */
				if (ETAL_receiverSupportsAudio(hReceiver))
				{
					vl_res = ETAL_cmdAudioMute_CMOST(hReceiver, (tU8)((exitSeekAction == cmdAudioMuted) ? 0 : 1));
					if (OSAL_OK != vl_res)
					{
						ret = ETAL_RET_ERROR;
					}
				}
			}
		}
		ETAL_receiverReleaseLock(hReceiver);
	}
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // CONFIG_ETAL_SUPPORT_CMOST

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_seek_stop_manual() = %s", ETAL_STATUS_toString(ret));
	if(freq != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Freq: %d", *freq);
	}
	return ret;
}

/***************************
 *
 * etal_seek_get_status_manual
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
ETAL_STATUS etal_seek_get_status_manual(ETAL_HANDLE hReceiver, EtalSeekStatus *seekStatus)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
	tU32 freq;
	tBool seekStoppedOnGoodFrequency, fullCycleReached, bandBorderCrossed;
	EtalBcastQualityContainer qualityInfo;
	tU32 device_list;
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tU8 vl_acq_status;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_seek_get_status_manual(rec: %d)", hReceiver);

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	ret = ETAL_receiverGetLock(hReceiver);
	if(ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_checkSeek_get_status_manualParameters(hReceiver, seekStatus);
		if(ret == ETAL_RET_SUCCESS)
		{
			(void)OSAL_pvMemorySet((tVoid *)seekStatus, 0x00, sizeof(EtalSeekStatus));
			(void)OSAL_pvMemorySet((tVoid *)&qualityInfo, 0x00, sizeof(EtalBcastQualityContainer));

			device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			if (ETAL_CMDROUTE_TO_DCOP(device_list) && (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
			{
				if ((ret = ETAL_get_reception_quality_internal(hReceiver, &qualityInfo)) != ETAL_RET_SUCCESS)
				{
					seekStatus->m_status         = ETAL_SEEK_ERROR;
					seekStatus->m_frequency      = ETAL_INVALID_FREQUENCY;
					seekStatus->m_frequencyFound = false;
				}
				else
				{
					seekStatus->m_status         = ETAL_SEEK_RESULT;
					seekStatus->m_frequency      = ETAL_receiverGetFrequency(hReceiver);
					seekStatus->m_frequencyFound = true;
				}
				seekStatus->m_receiverHandle     = hReceiver;
				seekStatus->m_quality            = qualityInfo;
				seekStatus->m_HDProgramFound     = false;
				seekStatus->m_serviceId          = ETAL_INVALID_PROG;
			}
#endif

			if (ETAL_CMDROUTE_TO_STAR(device_list) ||
		        (ETAL_CMDROUTE_TO_DCOP(device_list) && ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver))))
			{
				if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualSeek))
				{
					/* Send TUNER_seek_get_status */
					if (ETAL_cmdSeekGetStatus_CMOST(hReceiver, &seekStoppedOnGoodFrequency, &fullCycleReached, &bandBorderCrossed, &freq, &qualityInfo) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
						seekStatus->m_status           = ETAL_SEEK_ERROR;
						seekStatus->m_frequency        = ETAL_INVALID_FREQUENCY;
						seekStatus->m_frequencyFound   = false;
						seekStatus->m_fullCycleReached = false;
					}
					else
					{
						/*Send ETAL_INFO_SEEK*/
						seekStatus->m_status           = ETAL_SEEK_RESULT;
						seekStatus->m_frequency        = freq;
						seekStatus->m_frequencyFound   = seekStoppedOnGoodFrequency;
						seekStatus->m_fullCycleReached = fullCycleReached;
					}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					/* Check ETAL internal status */
					if (ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
					{
			        	ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, &vl_acq_status, &(seekStatus->m_serviceId), NULL, NULL);
						seekStatus->m_HDProgramFound   = (seekStatus->m_serviceId == ETAL_INVALID_PROG) ? false : true;

						if(vl_acq_status != (tU8)0)
						{
#if 0
							if (ETAL_cmdGetQuality_HDRADIO(hReceiver, &qualityInfo) != OSAL_OK)
							{
								ret = ETAL_RET_ERROR;
							}
#endif
						}
						else
						{
							/* HD has been reset but DCOP need time to be properly reset
							 * so we don't call ETAL_cmdGetQuality_HDRADIO() but keep HD quality
							 * container with reset value */
						}
					}
					else
					{
			        	seekStatus->m_HDProgramFound  = false;
			        	seekStatus->m_serviceId       = ETAL_INVALID_PROG;
					}
#else
		        	seekStatus->m_HDProgramFound   = false;
		        	seekStatus->m_serviceId        = ETAL_INVALID_PROG;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

					seekStatus->m_receiverHandle   = hReceiver;
					seekStatus->m_quality          = qualityInfo;
				}
				else
				{
					if ((ret  = ETAL_get_channel_quality_internal(hReceiver, &qualityInfo)) != ETAL_RET_SUCCESS)
					{
						seekStatus->m_status           = ETAL_SEEK_ERROR;
						seekStatus->m_frequency        = ETAL_INVALID_FREQUENCY;
						seekStatus->m_frequencyFound   = false;
						seekStatus->m_fullCycleReached = false;
					}
					else
					{
						/*Send ETAL_INFO_SEEK*/
						seekStatus->m_status           = ETAL_SEEK_RESULT;
						seekStatus->m_frequency        = ETAL_receiverGetFrequency(hReceiver);
						seekStatus->m_frequencyFound   = true;
						seekStatus->m_fullCycleReached = false;
					}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
					/* Check ETAL internal status */
					if (ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
					{
			        	ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, &vl_acq_status, &(seekStatus->m_serviceId), NULL, NULL);
						seekStatus->m_HDProgramFound   = (seekStatus->m_serviceId == ETAL_INVALID_PROG) ? false : true;

						if(vl_acq_status != (tU8)0)
						{
							if (ETAL_cmdGetQuality_HDRADIO(hReceiver, &qualityInfo) != OSAL_OK)
							{
								ret = ETAL_RET_ERROR;
							}
						}
						else
						{
							/* HD has been reset but DCOP need time to be properly reset
							 * so we don't call ETAL_cmdGetQuality_HDRADIO() but keep HD quality
							 * container with reset value */
						}
					}
					else
					{
			        	seekStatus->m_HDProgramFound  = false;
			        	seekStatus->m_serviceId       = ETAL_INVALID_PROG;
					}
#else
		        	seekStatus->m_HDProgramFound   = false;
		        	seekStatus->m_serviceId        = ETAL_INVALID_PROG;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

					seekStatus->m_receiverHandle   = hReceiver;
					seekStatus->m_quality          = qualityInfo;
				}
			}
		}
		ETAL_receiverReleaseLock(hReceiver);
	}
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_seek_get_status_manual() = %s", ETAL_STATUS_toString(ret));
	if(seekStatus != NULL)
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Seek status (rec: %d, sta: %d, fre: %d, freFou: %d, fulCyc: %d, HDProFou: %d, SerID: %d)",
				seekStatus->m_receiverHandle, seekStatus->m_status, seekStatus->m_frequency,
				seekStatus->m_frequencyFound, seekStatus->m_fullCycleReached,
				seekStatus->m_HDProgramFound, seekStatus->m_serviceId);
		ETAL_tracePrintQuality(TR_LEVEL_COMPONENT, &(seekStatus->m_quality));
	}

	return ret;
}
#endif // CONFIG_ETAL_HAVE_MANUAL_SEEK || CONFIG_ETAL_HAVE_ALL_API

