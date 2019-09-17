//!
//!  \file 		etalreceiver.c
//!  \brief 	<i><b> ETAL Receiver management </b></i>
//!  \details   Contains device-independent functions to manage the ETAL Receiver abstraction
//!  \details	ETAL uses the Receiver abstraction to refer to a particular
//! 			Tuner device and channel (a.k.a Frontend) and optional DCOP
//! 			combination.
//!  \details	A Receiver is associated at configuration time to a Tuner device
//! 			(e.g. a STAR device) and to one or two channels of that Tuner
//! 			(provided the Tuner supports more than one channel, e.g. STAR-T).
//! 			Associating two channels to the Receiver permits to exploit
//! 			FM VPA, HD Radio FM MRC or DAB MRC.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!  \see		etalReceiverStatusTy

#include "osal.h"
#include "etalinternal.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		ETAL_RECEIVER_SEM_NAME_MAX
 * 			Max length of the semaphore name used for the receiver lock
 */
#define ETAL_RECEIVER_SEM_NAME_MAX  16

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*!
 * \var		etalReceiverSem
 * 			Array of semaphores used to protect Receivers from concurrent accesses.
 */
static OSAL_tSemHandle      etalReceiverSem[ETAL_MAX_RECEIVERS];

/*!
 * 			The Receiver status array.
 */
static etalReceiverStatusTy etalReceivers[ETAL_MAX_RECEIVERS];

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
static tBool ETAL_receiverCheckFeDataCapability(ETAL_HANDLE hReceiver, EtalBcastDataType data_type);

/***************************
 *
 * ETAL_receiverIsValidHandleInternal
 *
 **************************/
/*!
 * \brief		Checks if a Receiver handle is valid and returns a pointer to its internal status
 * \details		The function performs the following checks on *hReceiver*:
 * 				- it is not ETAL_INVALID_HANDLE
 * 				- it is a Receiver handle type
 * 				- the index part of the handle is less than #ETAL_MAX_RECEIVERS
 * 				- the internal status (field isValid) indicates an initialized entry
 *
 * \param[in]	hReceiver - the Receiver handle to be checked
 * \param[out]	recvp - pointer to a location initialized by the function with the
 * 				        pointer to the Receiver internal status. If NULL it is ignored.
 * \return		TRUE it the handle is valid (and *recvp* points to its internal status), FALSE
 * 				otherwise. In the latter case *recvp* is unchanged.
 * \callgraph
 * \callergraph
 */
static tBool ETAL_receiverIsValidHandleInternal(ETAL_HANDLE hReceiver, etalReceiverStatusTy **recvp)
{
	etalReceiverStatusTy *recvp_local;
	ETAL_HINDEX receiver_index;
	tBool ret = TRUE;

	if (!ETAL_handleIsValid(hReceiver))
	{
		ret = FALSE;
		goto exit;
	}

	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);
	if ((receiver_index == ETAL_INVALID_HINDEX) ||
		(receiver_index >= (ETAL_HINDEX)ETAL_MAX_RECEIVERS))
	{
		ASSERT_ON_DEBUGGING(0);
		ret = FALSE;
		goto exit;
	}
	recvp_local = &etalReceivers[receiver_index];
	if (!recvp_local->isValid)
	{
		ret = FALSE;
		goto exit;
	}

	if (recvp)
	{
		*recvp = recvp_local;
	}
	
exit:
	return ret;
}

/***************************
 *
 * ETAL_receiverCopy
 *
 **************************/
/*!
 * \brief		Copies one Receiver internal status to another one
 * \param[in]	dst - pointer to a Receiver internal status (the destination)
 * \param[in]	src - pointer to a Receiver internal status (the source)
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_receiverCopy(etalReceiverStatusTy *dst, etalReceiverStatusTy *src)
{
	(void)OSAL_pvMemoryCopy((tVoid *)dst, (tPCVoid)src, sizeof(etalReceiverStatusTy));
}

/***************************
 *
 * ETAL_receiverGetInProgress
 *
 **************************/
/*!
 * \brief		Checks if a special command is in progress on a Receiver
 * \details		Special commands are those requiring a state machine action.
 * 				Some special commands are incompatible with certain operations.
 * \param[in]	recvp - pointer to the internal Receiver status
 * \param[in]	cmd - the special command to be checked
 * \return		TRUE if the requested special command is in progress, FALSE otherwise (or
 * 				if the *cmd* is not recognized)
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverGetInProgress(etalReceiverStatusTy *recvp, etalCmdSpecialTy cmd)
{
	tBool res = FALSE;

	switch (cmd)
	{
		case cmdSpecialTune:
			res = recvp->isTuneInProgress;
			break;

		case cmdSpecialManualAFCheck:
			res = recvp->isManualAFCheckInProgress;
			break;

		case cmdSpecialManualSeek:
			res = recvp->isManualSeekInProgress;
			break;

		case cmdSpecialRDS:
			res = recvp->isRDSInProgress;
			break;

		case cmdSpecialExternalSeekRequestInProgress:
			res = recvp->isExternalSeekRequestInProgress;
			break;

#ifdef CONFIG_ETAL_HAVE_ETALTML
		case cmdSpecialExternalLearnRequestInProgress:
			res = recvp->isExternalLearnRequestInProgress;
			break;

		case cmdSpecialExternalScanRequestInProgress:
			res = recvp->isExternalScanRequestInProgress;
			break;

		case cmdSpecialExternalRDSRequestInProgress:
			res = recvp->isExternalRDSRequestInProgress;
			break;
#endif // CONFIG_ETAL_HAVE_ETALTML

		case cmdSpecialExternalTuneRequestInProgress:
			res = recvp->isExternalTuneRequestInProgress;
			break;

		case cmdSpecialExternalDABAnnouncementRequestInProgress:
			res = recvp->isExternalDABAnnouncementRequestInProgress;
			break;

		case cmdSpecialExternalDABReconfigurationRequestInProgress:
			res = recvp->isExternalDABReconfigurationRequestInProgress;
			break;

		case cmdSpecialExternalDABDataStatusRequestInProgress:
			res = recvp->isExternalDABDataStatusRequestInProgress;
			break;

		case cmdSpecialExternalDABStatusRequestInProgress:
			res = recvp->isExternalDABStatusRequestInProgress;
			break;

		case cmdSpecialExternalSeamlessSwitchingRequestInProgress:
			res = recvp->isExternalSeamlessSwitchingRequestInProgress;
			break;

		case cmdSpecialExternalSeamlessEstimationRequestInProgress:
			res = recvp->isExternalSeamlessEstimationRequestInProgress;
			break;

		case cmdSpecialSeek:
			res = recvp->isSeekInProgress;
			break;

		case cmdSpecialSeamlessEstimation:
			res = recvp->isSeamlessEstimationInProgress;
			break;

		case cmdSpecialSeamlessSwitching:
			res = recvp->isSeamlessSwitchingInProgress;
			break;

#ifdef CONFIG_ETAL_HAVE_ETALTML
		case cmdSpecialScan:
			res = recvp->isScanInProgress;
			break;

		case cmdSpecialLearn:
			res = recvp->isLearnInProgress;
			break;

		case cmdSpecialDecodedRDS:
			res = recvp->isDecodedRDSInProgress;
			break;

		case cmdSpecialRDSStrategy:
			res = recvp->isRDSStrategyInProgress;
			break;

		case cmdSpecialTextInfo:
			res = recvp->isTextInfoInProgress;
			break;
#endif // CONFIG_ETAL_HAVE_ETALTML

		case cmdSpecialAnyChangingFrequency:
			res = recvp->isManualSeekInProgress ||
					recvp->isTuneInProgress || 
					recvp->isManualAFCheckInProgress ||
					recvp->isSeekInProgress;

#ifdef CONFIG_ETAL_HAVE_ETALTML
			res |= (recvp->isLearnInProgress ||
					recvp->isScanInProgress);
#endif
			break;

		case cmdSpecialEventFmStero:
			res = recvp->isEventFmSteroInProgress;
			break;
			
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
		case cmdSpecialRdsSeek:
			res = recvp->isRdsSeekInProgress;
			break;
#endif

		default:
			ASSERT_ON_DEBUGGING(0);
			break;
	}
	return res;
}

/***************************
 *
 * ETAL_receiverInitLock
 *
 **************************/
/*!
 * \brief		Creates all the Receivers locks
 * \details		Semaphores are stored in a static array of size #ETAL_MAX_RECEIVERS.
 * \remark		Normally called only once at ETAL startup.
 * \return		OSAL_OK    - no errors
 * \return		OSAL_ERROR - not all semaphores created, this is a FATAL error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverInitLock(tVoid)
{
	tU32 i;
	tChar sem_name[ETAL_RECEIVER_SEM_NAME_MAX];
	tSInt ret = OSAL_OK;

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		if (OSAL_s32NPrintFormat(sem_name, ETAL_RECEIVER_SEM_NAME_MAX, "%s%.2u", ETAL_SEM_RECEIVER_BASE, i) < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
		if (OSAL_s32SemaphoreCreate(sem_name, &etalReceiverSem[i], 1) == OSAL_ERROR)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
	}

exit:	
	return ret;
}

/***************************
 *
 * ETAL_receiverDeinitLock
 *
 **************************/
/*!
 * \brief		Destroys all the Receivers locks
 * \remark		Normally called only at ETAL shutdown.
 * \return		OSAL_OK    - no errors
 * \return		OSAL_ERROR - not all semaphores destroyed, this is a non-fatal error
 * \see			ETAL_receiverInitLock
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverDeinitLock(tVoid)
{
	tU32 i;
	tChar sem_name[ETAL_RECEIVER_SEM_NAME_MAX];
	tSInt ret = OSAL_OK;

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		if (OSAL_s32NPrintFormat(sem_name, ETAL_RECEIVER_SEM_NAME_MAX, "%s%.2u", ETAL_SEM_RECEIVER_BASE, i) < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
		if (OSAL_s32SemaphoreClose(etalReceiverSem[i]) == OSAL_ERROR)
		{
			ret = OSAL_ERROR;
		}
		else if (OSAL_s32SemaphoreDelete(sem_name) == OSAL_ERROR)
		{
			ret = OSAL_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_receiverGetLock
 *
 **************************/
/*!
 * \brief		Locks a Receiver
 * \param[in]	hReceiver - Receiver handle
 * \return		ETAL_RET_SUCCESS - the Receiver is locked
 * \return		ETAL_RET_ERROR   - error accessing the OSAL Semaphore; this typically should be treated as a FATAL error
 * \return		ETAL_RET_INVALID_HANDLE - wrong Receiver handle
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverGetLock(ETAL_HANDLE hReceiver)
{
	ETAL_HINDEX receiver_index;
	ETAL_STATUS retval;

	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);

	if ((receiver_index == ETAL_INVALID_HINDEX) ||
		((tU32)receiver_index >= ETAL_MAX_RECEIVERS))
	{
		retval = ETAL_RET_INVALID_HANDLE;
	}
	else if (OSAL_s32SemaphoreWait(etalReceiverSem[receiver_index], OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
	{
		retval = ETAL_RET_ERROR;
	}
	else
	{
		retval = ETAL_RET_SUCCESS;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverReleaseLock
 *
 **************************/
/*!
 * \brief		Unlocks a Receiver
 * \param[in]	hReceiver - Receiver handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverReleaseLock(ETAL_HANDLE hReceiver)
{
	ETAL_HINDEX receiver_index;

	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);

	if ((receiver_index == ETAL_INVALID_HINDEX) ||
		((tU32)receiver_index >= ETAL_MAX_RECEIVERS))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		(void)OSAL_s32SemaphorePost(etalReceiverSem[receiver_index]);
	}
}

/***************************
 *
 * ETAL_receiverGet
 *
 **************************/
/*!
 * \brief		Returns a pointer to the Receiver's internal status
 * \details		The Receiver status is held in an array of #etalReceiverStatusTy elements.
 * 				The function returns the pointer to the element for the Receiver.
 * \param[in]	hReceiver - Receiver handle
 * \return		The pointer to the Receiver's internal status,
 * 				or NULL if the Receiver handle is invalid
 * \callgraph
 * \callergraph
 */
etalReceiverStatusTy *ETAL_receiverGet(ETAL_HANDLE hReceiver)
{
	ETAL_HINDEX receiver_index;
	etalReceiverStatusTy *retval;

	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);

	if ((receiver_index == ETAL_INVALID_HINDEX) ||
		((tU32)receiver_index >= ETAL_MAX_RECEIVERS))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = NULL;
	}
	else
	{
		retval = &etalReceivers[receiver_index];
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverAllocateIfFree
 *
 **************************/
/*!
 * \brief		Changes the Receiver status to initialized
 * \details		If the receiver is not initialized, the function sets it to initialized
 * 				and returns TRUE. The caller can update the Receiver status.
 * \details		If the receiver is already initialized the function does not modify it
 * 				and returns FALSE. The caller should avoid changing an already initialized
 * 				Receiver status.
 * \param[in]	hReceiver - Receiver handle
 * \return		TRUE  - the Receiver was correctly allocated and can be configured
 * \return		FALSE - the Receiver is already initialized; this condition should be treated as error
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverAllocateIfFree(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	tBool retval = FALSE;

	recvp = ETAL_receiverGet(hReceiver);
	if ((recvp != NULL) && !recvp->isValid)
	{
		recvp->isValid = TRUE;
		retval = TRUE;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverHaveFreeSpace
 *
 **************************/
/*!
 * \brief		Checks if it would be possible to add a new Receiver to ETAL
 * \details		Receiver descriptions are stored in a fixed size array; the function checks
 * 				if there is at least one empty location (i.e. one not initialized Receiver
 * 				description) in the array.
 * \return		TRUE  - ETAL can accept a new Receiver
 * 				FALSE - the space reserved for Receivers is exhausted
 * \see			ETAL_MAX_RECEIVERS
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverHaveFreeSpace(tVoid)
{
	ETAL_HANDLE hReceiver;
	ETAL_HINDEX receiver_index;
	etalReceiverStatusTy *recvp;
	tU32 i;
	tBool retval = FALSE;

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		receiver_index = (ETAL_HINDEX)i;
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
		recvp = ETAL_receiverGet(hReceiver);
		if (recvp == NULL)
		{
			retval = FALSE;
			goto exit;
		}
		else if (!recvp->isValid)
		{
			retval = TRUE;
			goto exit;
		}
		else
		{
			/* Nothing to do */
		}

	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_receiverGetStandard
 *
 **************************/
/*!
 * \brief		Returns the Receiver's Broadcast Standard
 * \details		The Broadcast Standard is AM, FM, etc. The function returns
 * 				the standard for which this Receiver is currently configured.
 * \remark		To change a Receiver's standard is is necessary to reconfigure it.
 * \param[in]	hReceiver - Receiver handle
 * \return		The receiver's current standard
 * \see			etal_config_receiver
 * \callgraph
 * \callergraph
 */
EtalBcastStandard ETAL_receiverGetStandard(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	EtalBcastStandard retval = ETAL_BCAST_STD_UNDEF;

	recvp = ETAL_receiverGet(hReceiver);
	if ((recvp != NULL) && recvp->isValid)
	{
		retval = recvp->currentStandard;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverGetSupportedStandard
 *
 **************************/
/*!
 * \brief		Return the list of Broadcast Standards supported by this Receiver
 * \details		The list of Broadcast Standards supported by a Receiver depends on the
 * 				Tuner currently connected to that Receiver, and possibly on the presence of
 * 				a DCOP. The list of supported Standards for a Tuner is defined statically
 * 				in ETAL in the etalconfig_*.c files.
 * \param[in]	hReceiver - Receiver handle
 * \return		bitmap containing the list of supported Broadcast Standards.
 * 				The encoding of the bitmap follows the #EtalBcastStandard enum definition.
 * \see			etalconfig_accordo2.c for an example of a Tuner configuration
 * \callgraph
 * \callergraph
 */
tU32 ETAL_receiverGetSupportedStandard(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	tU32 retval = 0;

	recvp = ETAL_receiverGet(hReceiver);
	if ((recvp != NULL) && recvp->isValid)
	{
		retval = recvp->supportedStandards;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverSupportsQuality
 *
 **************************/
/*!
 * \brief		Checks if the Receiver supports Quality measures
 * \details		Quality measure for AM/FM is supported only by STAR device,
 * 				in all other cases it is performed on the DCOP.
 * 				The function thus only checks if the Tuner connected to the
 * 				Receiver is a STAR, for AM/FM receivers.
 * \param[in]	hReceiver - Receiver handle
 * \return		TRUE - the Receiver supports quality measures, FALSE otherwise
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverSupportsQuality(ETAL_HANDLE hReceiver)
{
	EtalDeviceType device_type;
	EtalBcastStandard std;
	tBool retval = TRUE;

	std = ETAL_receiverGetStandard(hReceiver);
	if ((std == ETAL_BCAST_STD_DAB) ||
		(std == ETAL_BCAST_STD_DRM) ||
		ETAL_IS_HDRADIO_STANDARD(std))
	{
		/*
		 * quality measures and seek command are performed on the DCOP
		 * independent of the frontend type
		 */
		retval = TRUE;
	}
	else
	{
		device_type = ETAL_receiverGetTunerType(hReceiver);
		if (ETAL_DEVICE_IS_DOT(device_type))
		{
			retval = FALSE;
		}
	}
	/*
	 * this covers also the STA662 case, where EtalDeviceType is DCOP
	 */
	return retval;
}

/***************************
 *
 * ETAL_receiverSupportsRDS
 *
 **************************/
/*!
 * \brief		Checks if the Receiver supports RDS decoding
 * \details		The function checks if the Receiver is configured
 * 				for the correct Broadcast standard (FM or HD), then
 * 				checks if the Tuner connected to the Receiver is a
 * 				STAR device (DOT doesn't provide any RDS support)
 * 				and finally checks if the Tuner channel used by
 * 				the Receiver is enabled for RDS from configuration.
 * \remark		The device type (DOT vs STAR) overrides the data type
 * 				listed in the etalTuner array.
 * \param[in]	hReceiver - Receiver handle
 * \return		TRUE - the Receiver supports RDS, FALSE otherwise
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverSupportsRDS(ETAL_HANDLE hReceiver)
{
	EtalDeviceType device_type;
	EtalBcastStandard std;
	ETAL_HANDLE hTuner;
	etalChannelTy channel;
	tU32 data_types;
	tBool retval = FALSE;

	/* RDS only make sense for FM or HD */
	std = ETAL_receiverGetStandard(hReceiver);
	if ((std != ETAL_BCAST_STD_FM) &&
		(std != ETAL_BCAST_STD_HD_FM))
	{
		retval = FALSE;
		goto exit;
	}

	/* DOT cannot provide RDS, neither decoded nor raw */
	device_type = ETAL_receiverGetTunerType(hReceiver);
	if (ETAL_DEVICE_IS_DOT(device_type))
	{
		retval = FALSE;
		goto exit;
	}

	/* check if RDS decoding is enabled in config */
	if (ETAL_receiverGetTunerId(hReceiver, &hTuner) == OSAL_OK)
	{
		if (ETAL_receiverGetChannel(hReceiver, &channel) == OSAL_OK)
		{
			data_types = ETAL_tunerGetChannelSupportedDataTypes(hTuner, channel);
			if ((data_types & ETAL_DATA_TYPE_FM_RDS) == ETAL_DATA_TYPE_FM_RDS)
			{
				retval = TRUE;
				goto exit;
			}
		}
	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_receiverSupportsAudio
 *
 **************************/
/*!
 * \brief		Checks if the Receiver supports Audio decoding
 * \details		The function checks if the devices used in the
 * 				Receiver are capable of producing audio for the
 * 				Receiver's Broadcast Standard and also if they
 * 				have been so configured in #etalTuner array.
 * 				The only device checks is for AM/FM, audio may be
 * 				available only if the tuner is a STAR
 * \remark		The function does not cope with AM/FM audio decoded on
 * 				the Host
 * \param[in]	hReceiver - Receiver handle
 * \return		TRUE - the Receiver supports Audio
 * \return		FALSE - the Receiver does not support Audio
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverSupportsAudio(ETAL_HANDLE hReceiver)
{
	tBool has_audio = FALSE;
	EtalBcastStandard std;
	EtalDeviceType device_type;
	etalChannelTy channel;
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	tyHDRADIOInstanceID instanceId;
#endif

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
		case ETAL_BCAST_STD_FM:
		case ETAL_BCAST_STD_AM:
			has_audio = ETAL_receiverCheckFeDataCapability(hReceiver, ETAL_DATA_TYPE_AUDIO);
			/* 
			 * check if it is actually doable, to avoid
			 * possible problems due to etalTuner misconfigurations
			 */
			if (has_audio)
			{
				device_type = ETAL_receiverGetTunerType(hReceiver);
				if (ETAL_DEVICE_IS_STAR(device_type))
				{
					if (ETAL_receiverGetChannel(hReceiver, &channel) == OSAL_OK)
					{
						if ((channel == ETAL_CHN_FOREGROUND) || (channel == ETAL_CHN_BOTH))
						{
							/* in VPA mode both channels are used so 
							 * it is treated like ETAL_CHN_FOREGROUND
							 */
							has_audio = TRUE;
						}
					}
				}
			}
			break;

		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			/* HD Radio DCOP supports 2 channels (instances) but audio
			 * only from the INSTANCE_1.
			 * TODO how to cope with MRC, what channel is returned by ETAL_receiverGetHdInstance?
			 */
			has_audio = ETAL_receiverCheckFeDataCapability(hReceiver, ETAL_DATA_TYPE_AUDIO) ||
						ETAL_receiverCheckFeDataCapability(hReceiver, ETAL_DATA_TYPE_DCOP_AUDIO);
			if (has_audio)
			{
				/* 
				 * check if it is actually doable, to avoid
				 * possible problems due to etalTuner misconfigurations
				 */
				if (ETAL_receiverGetHdInstance(hReceiver, &instanceId) == OSAL_OK)
				{
					if (instanceId == INSTANCE_1)
					{
						has_audio = TRUE;
					}
				}
			}
#endif
			break;

		case ETAL_BCAST_STD_DAB:
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
			has_audio = ETAL_receiverCheckFeDataCapability(hReceiver, ETAL_DATA_TYPE_AUDIO) ||
						ETAL_receiverCheckFeDataCapability(hReceiver, ETAL_DATA_TYPE_DCOP_AUDIO);
#endif
			break;

		case ETAL_BCAST_STD_UNDEF:
		case ETAL_BCAST_STD_DRM:
		default:
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_receiverSupportsAudio : error handle = %d, invalid standard = %d", hReceiver, std);
			ASSERT_ON_DEBUGGING(0);
			break;
	}

	return has_audio;
}


/***************************
 *
 * ETAL_receiverAddInternal
 *
 **************************/
/*!
 * \brief		Adds a new Receiver or modifies an existing one
 * \details		If the location pointed by *pReceiverReconf* contains a valid Receiver handle the function
 * 				reconfigure it with the attributes contained in parameter *st*. Otherwise it creates a new Receiver
 * 				and initializes its attributes from parameter *st*. In the latter case the newly created
 * 				Receiver handle is returned in *pReceiverReconf*. If it is not possible to create a new Receiver
 * 				the function writes ETAL_INVALID_HANDLE in *pReceiverReconf*.
 * \remark		This function is for internal use only.
 * \param[in]	st - pointer to a Receiver attribute to be used for the Receiver creation or modification
 * \param[in,out] pReceiverReconf - location pointing to a Receiver handle to be reconfigured,
 * 				                    or ETAL_INVALID_HANDLE; in the latter case, in this same
 * 				                    location the function stores the new Receiver handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverAddInternal(ETAL_HANDLE *pReceiverReconf, etalReceiverStatusTy *st)
{
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hReceiver;

	if (*pReceiverReconf == ETAL_INVALID_HANDLE)
	{
		hReceiver = ETAL_statusAddReceiverHandle();
	}
	else
	{
		hReceiver = *pReceiverReconf;
	}

	if (hReceiver == ETAL_INVALID_HANDLE)
	{
		*pReceiverReconf = ETAL_INVALID_HANDLE;
	}
	else
	{
		recvp = ETAL_receiverGet(hReceiver);
		if (recvp)
		{
			ETAL_receiverCopy(recvp, st);
			*pReceiverReconf = hReceiver;
		}
	}
}

/***************************
 *
 * ETAL_receiverIsValidRDSHandle
 *
 **************************/
/*!
 * \brief		Checks if the Receiver supports RDS
 * \remark		The function only checks the Broadcast Standard currently used by the Receiver.
 * \param[in]	hReceiver - the Receiver handle
 * \return		TRUE  - the Receiver supports RDS
 * \return		FALSE - the Receiver does not support RDS
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsValidRDSHandle(ETAL_HANDLE hReceiver)
{
	EtalBcastStandard std;
	tBool retval = FALSE;

	std = ETAL_receiverGetStandard(hReceiver);

	if ((std == ETAL_BCAST_STD_FM) ||
		(std == ETAL_BCAST_STD_HD_FM))
	{
		retval = TRUE;
	}

	return retval;
}

#if 0
/* currently unused, commented out */
/***************************
 *
 * ETAL_receiverIsValidHDHandle
 *
 **************************/
/*!
 * \brief		Checks if the Receiver is configured for HDRadio
 * \param[in]	hReceiver - the Receiver handle
 * \return		TRUE  - the Receiver is currently configured for HDRadio
 * \return		FALSE - the Receiver is not currently configured for HDRadio
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsValidHDHandle(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	return ETAL_IS_HDRADIO_STANDARD(recvp->currentStandard);
}
#endif

/***************************
 *
 * ETAL_receiverInit
 *
 **************************/
/*!
 * \brief		Resets all the Receiver internal state
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverInit(tVoid)
{
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
	tU32 i;
#endif

	(void)OSAL_pvMemorySet((tVoid *)etalReceivers, 0x00, sizeof(etalReceivers));
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
	for(i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		etalReceivers[i].MDRConfig.subch = ETAL_INVALID_SUBCH_ID;
	}
#endif
}


/***************************
 *
 * ETAL_receiverResetStatus
 *
 **************************/
/*!
 * \brief		Resets the state for a single Receiver
 * \remark		The function does not validate its parameter.
 * \param[in]	hReceiver - the Receiver handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverResetStatus(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp)
	{
		(void)OSAL_pvMemorySet((tVoid *)recvp, 0x00, sizeof(etalReceiverStatusTy));
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
		recvp->MDRConfig.subch = ETAL_INVALID_SUBCH_ID;
#endif
	}
}

/***************************
 *
 * ETAL_receiverInitPtr
 *
 **************************/
/*!
 * \brief		Resets given etalReceiverStatusTy state
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverInitPtr(etalReceiverStatusTy *recvp)
{
	if (recvp)
	{
		(void)OSAL_pvMemorySet((tVoid *)recvp, 0x00, sizeof(etalReceiverStatusTy));
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
		recvp->MDRConfig.subch = ETAL_INVALID_SUBCH_ID;
#endif
	}
}

/***************************
 *
 * ETAL_receiverUpperBandLimit
 *
 **************************/
/*!
 * \brief		Finds the upper band limit for a Receiver
 * \details		If the receiver has already been tuned the band limits are known precisely,
 * 				otherwise the function tries to guess based on the receiver broadcast standard.
 * \param[in]	hReceiver - the Receiver handle
 * \return		The upper band limit in Hz, or 0 if it cannot be deduced
 * \callgraph
 * \callergraph
 */
tU32 ETAL_receiverUpperBandLimit(ETAL_HANDLE hReceiver)
{
	tU32 limit;
	etalReceiverStatusTy *recvp;
	tU32 retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = 0;
	}

	else if (recvp->bandInfo.band != ETAL_BAND_UNDEF)
	{
		retval = recvp->bandInfo.bandMax;
	}
	else
	{
		switch (ETAL_receiverGetStandard(hReceiver))
		{
			case ETAL_BCAST_STD_FM:
				limit = ETAL_BAND_FMEU_MAX;
				break;
			case ETAL_BCAST_STD_DAB:
				limit = ETAL_BAND_DAB3_MAX;
				break;
			default:
				limit = 0; // INVALID FREQ
				break;
		}
		retval = limit;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverLowerBandLimit
 *
 **************************/
/*!
 * \brief		Finds the lower band limit for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \return		The lower band limit in Hz, or 0 if it cannot be deduced
 * \see			ETAL_receiverUpperBandLimit
 * \callgraph
 * \callergraph
 */
tU32 ETAL_receiverLowerBandLimit(ETAL_HANDLE hReceiver)
{
	tU32 limit;
	etalReceiverStatusTy *recvp;
	tU32 retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = 0;
	}
	else if (recvp->bandInfo.band != ETAL_BAND_UNDEF)
	{
		retval = recvp->bandInfo.bandMax;
	}
	else
	{
		switch (ETAL_receiverGetStandard(hReceiver))
		{
			case ETAL_BCAST_STD_FM:
				limit = ETAL_BAND_FMEU_MIN;
				break;
			case ETAL_BCAST_STD_DAB:
				limit = ETAL_BAND_DAB3_MIN;
				break;
			default:
				limit = 0; // INVALID FREQ
				break;
		}
		retval = limit;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverIsValidHandle
 *
 **************************/
/*!
 * \brief		Checks if a Receiver handle is valid
 * \param[in]	hReceiver - the Receiver handle
 * \return		TRUE  - the Receiver handle is valid
 * \return		FALSE - the Receiver handle is not valid
 * \see			ETAL_receiverIsValidHandleInternal
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsValidHandle(ETAL_HANDLE hReceiver)
{
	return ETAL_receiverIsValidHandleInternal(hReceiver, NULL);
}

/***************************
 *
 * ETAL_receiverSetStandard
 *
 **************************/
/*!
 * \brief		Sets the current Broadcast Standard for a Receiver and resets the Receiver band limits
 * \remark		The function performs the operation only if it verifies that the Receiver supports
 * 				the requested Broadcast Standard.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	std       - the new Broadcast Standard
 * \return		ETAL_RET_SUCCESS - operation performed
 * \return		ETAL_RET_ERROR   - invalid Receiver, or unsupported Broadcast Standard
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverSetStandard(ETAL_HANDLE hReceiver, EtalBcastStandard std)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_ERROR;
	}
	else if ((recvp->supportedStandards & std) == 0)
	{
		retval = ETAL_RET_ERROR;
	}
	else
	{
		recvp->currentStandard = std;
		/*
		 * reset band info
		 */
		ETAL_receiverSetBandInfo(hReceiver, ETAL_BAND_UNDEF, 0, 0, 0);
		retval = ETAL_RET_SUCCESS;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverSetFrequency
 *
 **************************/
/*!
 * \brief		Sets the frequency to which the Receiver is currently tuned
 * \remark		The function only records the information, it does not perform any tune operation.
 * \remark		The function validates only the Receiver handle, the other parameter is used as-is.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	freq      - the new frequency
  * \param[in]	resetRDS      - reset RDS or not
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetFrequency(ETAL_HANDLE hReceiver, tU32 freq, tBool resetRDS)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
		/* on change of hReceiver frequency we reset RDS data notification context */
		if (recvp->frequency != freq)
		{
#if defined(CONFIG_ETAL_HAVE_ETALTML) && defined(CONFIG_ETALTML_HAVE_RADIOTEXT)
			// check if valid receiver for RDS
			if (true == ETAL_receiverIsValidRDSHandle(hReceiver))
			{

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
				if (resetRDS)
				{
					ETALTML_RDS_Strategy_Data_Init(hReceiver);
				}
#endif
				ETALTML_RDSresetData(hReceiver);  //  CLEAR RDS DATA, reset
			}
#endif /* CONFIG_ETAL_HAVE_ETALTML && CONFIG_ETALTML_HAVE_RADIOTEXT */
#if defined(CONFIG_ETAL_SUPPORT_CMOST_STAR)
#if defined(CONFIG_ETAL_HAVE_FMSTEREO_EVENT_CONTROL) || defined(CONFIG_ETAL_HAVE_ALL_API)
			/* init default FM audio stereo status regardless of frontend and broadcast standard */
			ETAL_receiverInitFMAudioStereo(hReceiver);
#endif /* CONFIG_ETAL_HAVE_FMSTEREO_EVENT_CONTROL || CONFIG_ETAL_HAVE_ALL_API */
#endif /* CONFIG_ETAL_SUPPORT_CMOST_STAR */
		}
#endif /* CONFIG_ETAL_SUPPORT_CMOST */
	}

	recvp->frequency = freq;

exit:
	return;
}

/***************************
 *
 * ETAL_receiverSetBandInfo
 *
 **************************/
/*!
 * \brief		Sets the Band Information for a Receiver
 * \details		The Band Information encompasses: the Frequency Band, the lower and upper Band limits
 * 				and the default step to be used for seek.
 * \remark		The function only records the information, it does not perform any change band operation.
 * \remark		The function validates only the Receiver handle, other parameters used as-is.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	band      - the new Frequency Band
 * \param[in]	bandMin   - the new lower Band limit (in Hz)
 * \param[in]	bandMax   - the new upper Band limit (in Hz)
 * \param[in]	step      - the new step for seek (in Hz)
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetBandInfo(ETAL_HANDLE hReceiver, EtalFrequencyBand band, tU32 bandMin, tU32 bandMax, tU32 step)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		recvp->bandInfo.band = band;
		recvp->bandInfo.bandMin = bandMin;
		recvp->bandInfo.bandMax = bandMax;
		recvp->bandInfo.step = step;
	}

	return;
}

/***************************
 *
 * ETAL_receiverGetFrequency
 *
 **************************/
/*!
 * \brief		Returns the currently tuned frequency for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \return		The current frequency in Hz
 * \return		ETAL_INVALID_FREQUENCY if the Receiver handle is invalid or
 * 				the current frequency has not yet been initialized
 * \callgraph
 * \callergraph
 */
tU32 ETAL_receiverGetFrequency(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	tU32 retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		retval = ETAL_INVALID_FREQUENCY;
	}

	else if (recvp->frequency == 0)
	{
		retval = ETAL_INVALID_FREQUENCY;
	}
	
	else
	{
		retval = recvp->frequency;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverGetBandInfo
 *
 **************************/
/*!
 * \brief		Returns the current Band Information for the Receiver
 * \details		The function returns the Band Information set with the #ETAL_receiverSetBandInfo function.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	band_info - pointer to the location where the function stores the current Band Information
 * \return		OSAL_OK    - *band_info* was correctly initialized
 * \return		OSAL_ERROR - the Receiver handle is invalid, or the Band Information was not yet initialized.
 * 				             In both cases the location pointed by *band_info* is reset to 0
 * \see			ETAL_receiverSetBandInfo
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetBandInfo(ETAL_HANDLE hReceiver, etalFrequencyBandInfoTy *band_info)
{
	etalReceiverStatusTy *recvp;
	tSInt retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		(void)OSAL_pvMemorySet((tVoid *)band_info, 0x00, sizeof(etalFrequencyBandInfoTy));
		retval = OSAL_ERROR;
	}
	else if (recvp->bandInfo.band == ETAL_BAND_UNDEF)
	{
		(void)OSAL_pvMemorySet((tVoid *)band_info, 0x00, sizeof(etalFrequencyBandInfoTy));
		retval = OSAL_ERROR;
	}
	else
	{
		(void)OSAL_pvMemoryCopy((tVoid *)band_info, (tPCVoid)&(recvp->bandInfo), sizeof(etalFrequencyBandInfoTy));
		retval = OSAL_OK;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverGetRDSAttrInt
 *
 **************************/
/*!
 * \brief		Returns the current RDS _internal_ attributes for a Receiver
 * \details		RDS attributes are the parameters used to configure the RDS decoder
 * 				block of the STAR Tuner. RDS Internal attributes contain also some
 * 				RDS Interrupt configuration information.
 * \remark		The function returns a pointer to an internal Receiver status that should not be changes directly.
 * \param[in]	hReceiver   - the Receiver handle
 * \param[in]	pRDSAttrInt - pointer to a location where the function stores the current RDS attributes
 * \return		ETAL_RET_SUCCESS          - the RDS attributes are available
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver handle is invalid, in this case *pRDSAttrInt* is set to NULL
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverGetRDSAttrInt(ETAL_HANDLE hReceiver, etalRDSAttrInternal **pRDSAttrInt)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
        *pRDSAttrInt = NULL;
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
    /* return rdsAttrInt */
		*pRDSAttrInt = &(recvp->CMOSTConfig.rdsAttrInt);
    retval = ETAL_RET_SUCCESS;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverGetRDSAttr
 *
 **************************/
/*!
 * \brief		Returns the current RDS attributes for a Receiver
 * \details		RDS attributes are the parameters used to configure the RDS decoder
 * 				block of the STAR Tuner.
 * \remark		The function returns a pointer to an internal Receiver status that should not be changes directly.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	pRDSAttr  - pointer to a location where the function stores the current RDS attributes
 * \return		ETAL_RET_SUCCESS          - the RDS attributes are available
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver handle is invalid, in this case pRDSAttrInt is set to NULL
 * \see			ETAL_receiverGetRDSAttrInt
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverGetRDSAttr(ETAL_HANDLE hReceiver, etalRDSAttr **pRDSAttr)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;
	
	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
        *pRDSAttr = NULL;
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
	    /* return rdsAttr */
		*pRDSAttr = &(recvp->CMOSTConfig.rdsAttrInt.rdsAttr);
	    retval = ETAL_RET_SUCCESS;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverSetRDSAttr
 *
 **************************/
/*!
 * \brief		Sets the RDS attributes for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	RDSAttr   - pointer to the struct containing the RDS attributes
 * \return		ETAL_RET_SUCCESS          - the RDS attributes were successfully set
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver handle is invalid,
 * 				                            the Receiver RDS attributes are left unchanged
 * \see			ETAL_receiverGetRDSAttr
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverSetRDSAttr(ETAL_HANDLE hReceiver, etalRDSAttr *RDSAttr)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;
	
	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		(void)OSAL_pvMemoryCopy((tPVoid)&recvp->CMOSTConfig.rdsAttrInt.rdsAttr, (tPCVoid)RDSAttr, sizeof(etalRDSAttr));
   	retval = ETAL_RET_SUCCESS;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverGetProcessingFeatures
 *
 **************************/
/*!
 * \brief		Returns the current processing features for a Receiver
 * \details		Get the current processing features for a Receiver like FM VPA, antenna diversity, 
 * 				HD radio / DRM digital BB interface and filter, HD radio on chip blending
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	*proc_features - the processing features pointer
 * \return		ETAL_STATUS - processing features available
 * \return		ETAL_RET_INVALID_RECEIVER - invalid receiver handle, the processing features is not available.
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverGetProcessingFeatures(ETAL_HANDLE hReceiver, EtalProcessingFeatures *proc_features)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;
	
	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
    /* return processing features */
    proc_features->u.m_processing_features = recvp->CMOSTConfig.processingFeatures.u.m_processing_features;
    retval = ETAL_RET_SUCCESS;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverSetProcessingFeatures
 *
 **************************/
/*!
 * \brief		Set the current processing features for a Receiver
 * \details		Set the current processing features for a Receiver like FM VPA, 
 *				HD radio / DRM digital BB interface and filter, HD radio on chip blending
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	*proc_features - the processing features pointer
 * \return		ETAL_RET_SUCCESS          - the processing features was successfully set
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver handle is invalid, the Receiver processing features is left unchanged
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverSetProcessingFeatures(ETAL_HANDLE hReceiver, EtalProcessingFeatures proc_features)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* Set processing features */
		recvp->CMOSTConfig.processingFeatures.u.m_processing_features = proc_features.u.m_processing_features;
		retval = ETAL_RET_SUCCESS;
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverSetDefaultProcessingFeatures
 *
 **************************/
/*!
 * \brief		Set the default processing features for a Receiver
 * \details		Set the default processing features for a Receiver like FM VPA, 
 *				HD radio / DRM digital BB interface and filter, HD radio on chip blending
 *				when processing features is unspecified
 * \param[in]			std - broadcasting standard
 * \param[in]			frontendsz - front end size
 * \param[in/out]		*proc_features - the processing features pointer
 * \return			tVoid
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetDefaultProcessingFeatures(EtalBcastStandard std, tU8 frontendsz, EtalProcessingFeatures *proc_features)
{
	if (proc_features->u.m_processing_features == (tU8)ETAL_PROCESSING_FEATURE_UNSPECIFIED)
	{
		/* set processing features with default values when unspecified */
		if (frontendsz == (tU8)2)
		{
			switch (std)
			{
				case ETAL_BCAST_STD_FM:
					proc_features->u.m_processing_features = (tU8)ETAL_PROCESSING_FEATURE_FM_VPA;
					break;
				case ETAL_BCAST_STD_HD_FM:
					proc_features->u.m_processing_features = 
						(tU8)ETAL_PROCESSING_FEATURE_FM_VPA | (tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | (tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING;
					break;
				case ETAL_BCAST_STD_HD_AM:
					/* this configuration should not happen */
					proc_features->u.m_processing_features = 
						(tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | (tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING;
					break;
				case ETAL_BCAST_STD_DRM:
					proc_features->u.m_processing_features = (tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF;
					break;
				default:
					proc_features->u.m_processing_features = (tU8)ETAL_PROCESSING_FEATURE_NONE;
					break;
			}
		}
		else
		{
			switch (std)
			{
				case ETAL_BCAST_STD_HD_FM:
				case ETAL_BCAST_STD_HD_AM:
					proc_features->u.m_processing_features = 
						(tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | (tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING;
					break;
				case ETAL_BCAST_STD_DRM:
					proc_features->u.m_processing_features = (tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF;
					break;
				default:
					proc_features->u.m_processing_features = (tU8)ETAL_PROCESSING_FEATURE_NONE;
					break;
			}
		}
	}
	else
	{
		if (ETAL_IS_HDRADIO_STANDARD(std))
		{
			proc_features->u.m_processing_features |= 
				(tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | (tU8)ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING;
		}
	}
}

#if defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_receiverGetConfigVPAMode
 *
 **************************/
/*!
 * \brief		Returns the current VPA Mode configuration for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	*configVpaMode - the Config VPA Mode
 * \return		ETAL_STATUS - Config VPA Mode available
 * \return		ETAL_RET_INVALID_RECEIVER - invalid receiver handle, the Config VPA Mode is not available.
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverGetConfigVPAMode(ETAL_HANDLE hReceiver, tBool *configVpaMode)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;
	
	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* return Config VPA Mode */
		*configVpaMode = (recvp->CMOSTConfig.processingFeatures.u.bf.m_fm_vpa != (tU8)0) ? TRUE : FALSE;
		retval = ETAL_RET_SUCCESS;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverGetDebugVPAMode
 *
 **************************/
/*!
 * \brief		Returns the current Debug VPA Mode for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	*debugVpaMode - the Debug VPA Mode
 * \return		ETAL_STATUS - Debugging VPA Mode available
 * \return		ETAL_RET_INVALID_RECEIVER - invalid receiver handle, the Debug VPA Mode is not available.
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverGetDebugVPAMode(ETAL_HANDLE hReceiver, EtalDebugVPAModeEnumTy *debugVpaMode)
{
  etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;
	
	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* return m_DebugVPAMode */
		*debugVpaMode =  recvp->CMOSTConfig.m_DebugVPAMode;
		retval = ETAL_RET_SUCCESS;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverSetConfigVPAMode
 *
 **************************/
/*!
 * \brief		Set the current VPA Mode Configuration for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	configVpaMode - the Config VPA Mode
 * \return		ETAL_RET_SUCCESS          - the Config VPA Mode was successfully set
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver handle is invalid, the Receiver Config VPA Mode is left unchanged
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverSetConfigVPAMode(ETAL_HANDLE hReceiver, tBool configVpaMode)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* Set Config VPA Mode */
		recvp->CMOSTConfig.processingFeatures.u.bf.m_fm_vpa = (configVpaMode) ? (tU8)1 : (tU8)0;
		retval = ETAL_RET_SUCCESS;		
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverSetDebugVPAMode
 *
 **************************/
/*!
 * \brief		Set the current Debug VPA Mode for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	debugVpaMode - the Debug VPA Mode
 * \return		ETAL_RET_SUCCESS          - the Debug VPA Mode was successfully set
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver handle is invalid, the Receiver Debug VPA Mode is left unchanged
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverSetDebugVPAMode(ETAL_HANDLE hReceiver, EtalDebugVPAModeEnumTy debugVpaMode)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* Set Debug VPA Mode */
		recvp->CMOSTConfig.m_DebugVPAMode = debugVpaMode;
		retval = ETAL_RET_SUCCESS;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverGetDISSStatus
 *
 **************************/
/*!
 * \brief		Returns the current DISS status for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	*diss_status - the DISS status pointer
 * \return		ETAL_STATUS - DISS status available
 * \return		ETAL_RET_INVALID_RECEIVER - invalid receiver handle, the DISS status is not available.
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverGetDISSStatus(ETAL_HANDLE hReceiver, EtalDISSStatusTy *diss_status)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* return DISS status */
		*diss_status = recvp->CMOSTConfig.dissStatus;
		retval = ETAL_RET_SUCCESS;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverSetDISSStatus
 *
 **************************/
/*!
 * \brief		Set the current DISS status for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	diss_status - the DISS status pointer
 * \return		ETAL_RET_SUCCESS          - the DISS status was successfully set
 * \return		ETAL_RET_INVALID_RECEIVER - the Receiver handle is invalid, the Receiver DISS status is left unchanged
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverSetDISSStatus(ETAL_HANDLE hReceiver, EtalDISSStatusTy *diss_status)
{
	etalReceiverStatusTy *recvp;
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		/* Set DISS status */
		recvp->CMOSTConfig.dissStatus = *diss_status;
		retval = ETAL_RET_SUCCESS;
	}

	return retval;
}
#endif // defined(CONFIG_ETAL_HAVE_DEBUG_COMMANDS) || defined(CONFIG_ETAL_HAVE_ALL_API)

/***************************
 *
 * ETAL_receiverSetSpecial
 *
 **************************/
/*!
 * \brief		Sets the Receiver's Special Command status
 * \details		Some commands are internally identified as special because they cannot be interrupted
 * 				or because they cannot be performed together with other operations, or for some
 * 				other reason.
 * \remark		If the Receiver handle is invalid the function silently aborts.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	cmd       - indicates the Special Command type
 * \param[in]	action    - indicates if the Special Command is started or stopped
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetSpecial(ETAL_HANDLE hReceiver, etalCmdSpecialTy cmd, etalCmdActionTy action)
{
	etalReceiverStatusTy *recvp;
	tBool local_action = (tBool)((action == cmdActionStart) ? 1 : 0);

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}

	switch (cmd)
	{
		case cmdSpecialTune:
			recvp->isTuneInProgress = local_action;
			break;			
			
		case cmdSpecialManualAFCheck:
			recvp->isManualAFCheckInProgress = local_action;
			break;

		case cmdSpecialManualSeek:
			recvp->isManualSeekInProgress = local_action;
			break;

		case cmdSpecialRDS:
			recvp->isRDSInProgress = local_action;
			break;

		case cmdSpecialExternalSeekRequestInProgress:
			recvp->isExternalSeekRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event info seek notification : %d (0:stopped, 1:started) on %d", recvp->isExternalSeekRequestInProgress, hReceiver);
			// Suspend implicitly INFO TUNE event
			recvp->isExternalTuneRequestInProgress = (local_action == cmdActionStart) ? cmdActionStop : cmdActionStart;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event info tune notification : %d (0:stopped, 1:started) on %d", recvp->isExternalTuneRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalSeamlessEstimationRequestInProgress:
			recvp->isExternalSeamlessEstimationRequestInProgress = local_action;
			// Suspend implicitly INFO TUNE event
			recvp->isExternalTuneRequestInProgress = (local_action == cmdActionStart) ? cmdActionStop : cmdActionStart;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event info tune notification : %d (0:stopped, 1:started) on %d", recvp->isExternalTuneRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalSeamlessSwitchingRequestInProgress:
			recvp->isExternalSeamlessSwitchingRequestInProgress = local_action;
			// Suspend implicitly INFO TUNE event
			recvp->isExternalTuneRequestInProgress = (local_action == cmdActionStart) ? cmdActionStop : cmdActionStart;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event info tune notification : %d (0:stopped, 1:started) on %d", recvp->isExternalTuneRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalTuneRequestInProgress:
			recvp->isExternalTuneRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event info tune notification : %d (0:stopped, 1:started) on %d", recvp->isExternalTuneRequestInProgress, hReceiver);
			break;

		case cmdSpecialSeek:
			recvp->isSeekInProgress = local_action;
			break;

		case cmdSpecialSeamlessEstimation:
			recvp->isSeamlessEstimationInProgress = action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event seamless estimation notification : %d (0:stopped, 1:started) on %d", recvp->isSeamlessEstimationInProgress, hReceiver);
			break;

		case cmdSpecialSeamlessSwitching:
			recvp->isSeamlessSwitchingInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event seamless switching notification : %d (0:stopped, 1:started) on %d", recvp->isSeamlessSwitchingInProgress, hReceiver);
			break;

		case cmdSpecialExternalDABAnnouncementRequestInProgress:
			recvp->isExternalDABAnnouncementRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event DAB announcement switching notification : %d (0:stopped, 1:started) on %d", recvp->isExternalDABAnnouncementRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalDABDataStatusRequestInProgress:
			recvp->isExternalDABDataStatusRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event DAB data status notification : %d (0:stopped, 1:started) on %d", recvp->isExternalDABDataStatusRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalDABStatusRequestInProgress:
			recvp->isExternalDABStatusRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event DAB status notification : %d (0:stopped, 1:started) on %d", recvp->isExternalDABStatusRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalDABReconfigurationRequestInProgress:
			recvp->isExternalDABReconfigurationRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event DAB reconfiguration notification : %d (0:stopped, 1:started) on %d", recvp->isExternalDABReconfigurationRequestInProgress, hReceiver);
			break;

#ifdef CONFIG_ETAL_HAVE_ETALTML
		case cmdSpecialExternalLearnRequestInProgress:
			recvp->isExternalLearnRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event Learn notification : %d (0:stopped, 1:started) on %d", recvp->isExternalLearnRequestInProgress, hReceiver);
			// Suspend implicitly INFO TUNE event
			recvp->isExternalTuneRequestInProgress = (local_action == cmdActionStart) ? cmdActionStop : cmdActionStart;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event info tune notification : %d (0:stopped, 1:started) on %d", recvp->isExternalTuneRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalScanRequestInProgress:
			recvp->isExternalScanRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event Scan notification : %d (0:stopped, 1:started) on %d", recvp->isExternalScanRequestInProgress, hReceiver);
			// Suspend implicitly INFO TUNE event
			recvp->isExternalTuneRequestInProgress = (local_action == cmdActionStart) ? cmdActionStop : cmdActionStart;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event info tune notification : %d (0:stopped, 1:started) on %d", recvp->isExternalTuneRequestInProgress, hReceiver);
			break;

		case cmdSpecialExternalRDSRequestInProgress:
			recvp->isExternalTuneRequestInProgress = local_action;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "Event RDS notification : %d (0:stopped, 1:started) on %d", recvp->isExternalTuneRequestInProgress, hReceiver);
			break;

		case cmdSpecialScan:
			recvp->isScanInProgress = local_action;
			break;

		case cmdSpecialLearn:
			recvp->isLearnInProgress = local_action;
			break;

		case cmdSpecialDecodedRDS:
			recvp->isDecodedRDSInProgress = local_action;
			break;

		case cmdSpecialRDSStrategy:
			recvp->isRDSStrategyInProgress = local_action;
			break;

		case cmdSpecialTextInfo:
			recvp->isTextInfoInProgress = local_action;
			break;
#endif // CONFIG_ETAL_HAVE_ETALTML

		case cmdSpecialEventFmStero:
			recvp->isEventFmSteroInProgress = local_action;
			break;	
			
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
		case cmdSpecialRdsSeek:
			recvp->isRdsSeekInProgress = local_action;
			break;
#endif			
		default:
			ASSERT_ON_DEBUGGING(0);
			break;
	}

exit:
	return;
}

/***************************
 *
 * ETAL_receiverIsFrontendUsed
 *
 **************************/
/*!
 * \brief		Checks if a Frontend is used by a Receiver
 * \details		A Receiver is configured with one or more Frontend (a.k.a CMOST channel).
 * 				The function checks if the Frontend is used by the Receiver.
 * \param[in]	recvp - pointer to the Receiver's internal status
 * \param[in]	hFrontend - the Frontend to search
 * \return		TRUE  - the Frontend is used in the Receiver
 * \return		FALSE - the Frontend is not used in the Receiver,
 * 				        or the Frontend is unknown
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsFrontendUsed(etalReceiverStatusTy *recvp, ETAL_HANDLE hFrontend)
{
	tU32 i;
	tBool retval = FALSE;

	if (ETAL_tunerSearchFrontend(hFrontend) == OSAL_ERROR)
	{
		/*
		 * Unknown Frontend, this is an abnormal situation
		 */
		ASSERT_ON_DEBUGGING(0);
		retval = FALSE;
		goto exit;
	}

	for (i = 0; i < (tU32)recvp->diversity.m_DiversityMode; i++)
	{
		if (recvp->diversity.m_FeConfig[i] == hFrontend)
		{
			retval = TRUE;
			goto exit;
		}
	}
	
exit:
	return retval;
}

/***************************
 *
 * ETAL_receiverFreeAllFrontend
 *
 **************************/
/*!
 * \brief		Frees all Frontend of a Receiver
 * \details		Frees all Used FrontEnd (a.k.a CMOST channel) of a Receiver, clear diversity context.
 * \param[in]	hReceiver - the Receiver handle
 * \return		ETAL_RET_SUCCESS  - receiver FrontEnd freed
 * \return		ETAL_RET_INVALID_RECEIVER - given receiver is invalid, no FrontEnd freed
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_receiverFreeAllFrontend(ETAL_HANDLE hReceiver)
{
    tU32 i;
    etalReceiverStatusTy *recvp;
		ETAL_STATUS retval = ETAL_RET_SUCCESS;

    if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
    {
        ASSERT_ON_DEBUGGING(0);
        retval = ETAL_RET_INVALID_RECEIVER;
    }
		else
		{
	    recvp->diversity.m_DiversityMode = (tU8)0;
	    for (i = 0; i < ETAL_CAPA_MAX_FRONTEND; i++)
	    {
	        recvp->diversity.m_FeConfig[i] = ETAL_INVALID_HANDLE;
	    }
	    recvp->CMOSTConfig.channel = ETAL_CHN_UNDEF;
  	}
    return retval;
}


/***************************
 *
 * ETAL_receiverCheckFeDataCapability
 *
 **************************/
/*!
 * \brief		Checks the specified Receiver data type
 * \details		Checks if the at least one Frontend associated with the Receiver
 * 				is supports the *data_type* according to the configured capabilities in
 * 				#etalTuner.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	data_type - the data type to search in the Frontend configured capabilities
 * \return		TRUE  - the data_type is supported
 * \return		FALSE - the data_type is not supported
 * \callgraph
 * \callergraph
 */
static tBool ETAL_receiverCheckFeDataCapability(ETAL_HANDLE hReceiver, EtalBcastDataType data_type)
{
	tBool is_capable = FALSE;
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hFrontend;
	tU32 i;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp)
	{
		for (i = 0; i < (tU32)recvp->diversity.m_DiversityMode; i++)
		{
			hFrontend = recvp->diversity.m_FeConfig[i];
			if ((ETAL_tunerGetFEsupportedDataTypes(hFrontend) & data_type) == data_type)
			{
				is_capable = TRUE;
			}
		}
	}
#else
		/* this else was here to support the STA662 case but it is dead code */
		#error "etalreceiver.c does not support non-CMOST configuration"
#endif
	return is_capable;
}

/***************************
 *
 * ETAL_receiverIsRDSCapable
 *
 **************************/
/*!
 * \brief		Checks if the Receiver is capable of RDS
 * \details		Simply checks if the first Frontend associated with the Receiver is RDS-capable.
 * \param[in]	hReceiver - the Receiver handle
 * \return		TRUE  - the Receiver is capable of RDS
 * \return		FALSE - the Receiver is not capable RDS
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsRDSCapable(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hFrontend;
	tBool retval = FALSE;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp)
	{
		hFrontend = recvp->diversity.m_FeConfig[0];
		retval = ((ETAL_tunerGetFEsupportedDataTypes(hFrontend) & ETAL_DATA_TYPE_FM_RDS) == ETAL_DATA_TYPE_FM_RDS);
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverGetFrontendType
 *
 **************************/
/*!
 * \brief		Returns the Frontend associated with the Receiver
 * \details		Normally the Frontend is some CMOST type, but for STA662 it is DCOP.
 * \param[in]	hReceiver - the Receiver handle
 * \return		The Frontend type, or deviceUnknown in case of unknown Receiver
 * \callgraph
 * \callergraph
 */
EtalDeviceType ETAL_receiverGetTunerType(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	EtalDeviceType retval = deviceUnknown;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp)
	{
		retval = recvp->tunerType;
	}
	return retval;
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/***************************
 *
 * ETAL_receiverResetRadioInfo_HDRADIO
 *
 **************************/
/*!
 * \brief		Resets the available program list for the Receiver
 * \details		The list of programs is initialized to non-zero values
 * 				(#ETAL_INVALID_PROG) in this function.
 * \param[in]	recvp - pointer to the Receiver internal status
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_receiverResetRadioInfo_HDRADIO(etalReceiverStatusTy *recvp)
{
	tU32 i;

	for (i = 0; i < ETAL_HD_MAX_PROGRAM_NUM; i++)
	{
		recvp->HDRADIOConfig.availablePrograms[i] = ETAL_INVALID_PROG;
	}
}

/***************************
 *
 * ETAL_receiverSetRadioInfo_HDRADIO
 *
 **************************/
/*!
 * \brief		Sets the HDRadio Radio Information for a Receiver		
 * \details		The HDRadio Radio Information is an ETAL name for the information
 * 				comprising the acquisition status, the currently tuned program and the available programs.
 * \details	 	The parameter *avail_prog* is a bitmap indicating which audio programs are available
 *	 			in the HD station (LSB is MPS, MSB is SPS7). The function expands
 * 				it into an array of services, where the first entry contains the
 * 				index of the first service (the MPS, always index 0), the second entry
 * 				contains the index of the first SPS (e.g. 2 for SPS2) and so on.
 * \details	 	Uninitialized entries contain ETAL_INVALID_PROG.
 * \details	 	These indices can be used as parameter to the etal_service_select.
 * \remark		If the Receiver is invalid the function silently aborts the operation.
 * \param[in]	hReceiver  - the Receiver handle
 * \param[in]	acq_status - bitmap of tyHDRADIODigitalAcquisitionStatus values representing the acquisition status
 * \param[in]	curr_prog  - the index of the currently tuned program
 * \param[in]	avail_prog - bitmap describing the available programs as described above
  * \param[in]	audio_information - bitmap describing the AudioIndicatorTy : Alignment status and Digitial Audio avalaibility
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetRadioInfo_HDRADIO(ETAL_HANDLE hReceiver, tU8 acq_status, tS8 curr_prog, tU8 avail_prog, tU8 audio_information)
{
	tU32 i;
	etalReceiverStatusTy *recvp;

	if (ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		recvp->HDRADIOConfig.acquisitionStatus = acq_status;
		recvp->HDRADIOConfig.audioInformation = audio_information;
		recvp->HDRADIOConfig.currentProgram = curr_prog;
		ETAL_receiverResetRadioInfo_HDRADIO(recvp);
		recvp->HDRADIOConfig.programNum = (tU8)0;
		/*
		 * the audioProgramsAvailable field of the Tune_GetStatus is valid
		 * only after HD acquired
		 */
		if ((acq_status & (tU8)HD_ACQUIRED) == (tU8)HD_ACQUIRED)
		{
			for (i = 0; i < ETAL_HD_MAX_PROGRAM_NUM; i++)
			{
				if ((avail_prog & (1 << i)) == (tU8)(1 << i))
				{
					recvp->HDRADIOConfig.availablePrograms[recvp->HDRADIOConfig.programNum] = (tS8)i;
					recvp->HDRADIOConfig.programNum = recvp->HDRADIOConfig.programNum + (tU8)1;
				}
			}
		}
	}
	else
	{
		ASSERT_ON_DEBUGGING(0);
	}
}

/***************************
 *
 * ETAL_receiverGetRadioInfo_HDRADIO
 *
 **************************/
/*!
 * \brief		Returns the HDRadio Radio Information
 * \details		Returns the information set with the ETAL_receiverSetRadioInfo_HDRADIO
 * \remark		Pointer parameters can be set to NULL, the function will ignore them.
 * \param[in]	hReceiver  - the Receiver handle
 * \param[out]	acq_status - the acquisition status
 * \param[out]	curr_prog  - the current program
 * \param[out]	avail_prog_num - the number of available programs on the current frequency
  * \param[out]	pO_audio_information - the audio information 
 * \see			ETAL_receiverSetRadioInfo_HDRADIO for a description of the parameter format
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverGetRadioInfo_HDRADIO(ETAL_HANDLE hReceiver, tU8 *acq_status, tS8 *curr_prog, tU32 *avail_prog_num, tU8 *pO_audio_information)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		if (acq_status)
		{
			*acq_status = recvp->HDRADIOConfig.acquisitionStatus;
		}
		if (curr_prog)
		{
			*curr_prog = recvp->HDRADIOConfig.currentProgram;
		}
		if (avail_prog_num)
		{
			*avail_prog_num = (tU32)recvp->HDRADIOConfig.programNum;
		}
		if (pO_audio_information)
		{
			*pO_audio_information = recvp->HDRADIOConfig.audioInformation;
		}
	}
	
	return;
}

/***************************
 *
 * ETAL_receiverSetCurrentProgram_HDRADIO
 *
 **************************/
/*!
 * \brief		Sets the current Program for an HDRadio Receiver
 * \details		HDRadio supports one Main Program Service and several Secondary Program Services
 * 				on each frequency; this function specifies to ETAL which Program is currently tuned.
 * \remark		Silently fails if the program is not in the list of available programs for the current frequency.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	prog      - the current program
 * \see			ETAL_receiverSetRadioInfo_HDRADIO for the *prog* parameter format
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetCurrentProgram_HDRADIO(ETAL_HANDLE hReceiver, tS8 prog)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}

	if (!ETAL_receiverHasService_HDRADIO(hReceiver, prog))
	{
		goto exit;
	}

	recvp->HDRADIOConfig.currentProgram = prog;

exit:
	return;
}

/***************************
 *
 * ETAL_receiverHasService_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks if the currently tuned HDRadio frequency supports the Service
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	prog      - the index of an HDRadio Service, see ETAL_receiverSetRadioInfo_HDRADIO
 * \return		TRUE  - *prog* is available in the currently tuned HDRadio frequency
 * \return		FALSE - *prog* is not available, or an error occurred:
 * 				        the Receiver is invalid or the *prog* parameter is invalid
 * \see			ETAL_receiverSetCurrentProgram_HDRADIO for a description of Services in HDRadio
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverHasService_HDRADIO(ETAL_HANDLE hReceiver, tS8 prog)
{
	tU32 i;
	etalReceiverStatusTy *recvp;
	tBool retval = FALSE;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = FALSE;
		goto exit;
	}

	if ((prog >= (tS8)ETAL_HD_MAX_PROGRAM_NUM) ||
		(prog == ETAL_INVALID_PROG))
	{
		retval = FALSE;
		goto exit;
	}

	if (prog < (tS8)0)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = FALSE;
		goto exit;
	}

	for (i = 0; i < (tU32)recvp->HDRADIOConfig.programNum; i++)
	{
		if (recvp->HDRADIOConfig.availablePrograms[i] == prog)
		{
			retval = TRUE;
			goto exit;
		}
	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_receiverGetService_HDRADIO
 *
 **************************/
/*!
 * \brief		Returns the HDRadio Service
 * \details		ETAL maintains an array of ETAL_HD_MAX_PROGRAM_NUM entries
 * 				containing the identifiers of the HDRadio Services present
 * 				on the current frequency. The function returns the identifier
 * 				stored in the *index* location.
 * \remark		The function is used to loop over the available HDRadio Services.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	index     - index into the internal array of Services
 * \return		The HDRadio Service identifier
 * \see			ETAL_receiverSetRadioInfo_HDRADIO for a description of how Service are stored
 * \callgraph
 * \callergraph
 */
tS8 ETAL_receiverGetService_HDRADIO(ETAL_HANDLE hReceiver, tU32 index)
{
	etalReceiverStatusTy *recvp;
	tS8 retval;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_PROG;
	}
	else if (index >= ETAL_HD_MAX_PROGRAM_NUM)
	{
		retval = ETAL_INVALID_PROG;
	}
	else
	{
		retval = (tS8)recvp->HDRADIOConfig.availablePrograms[index];
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverGetHdInstance
 *
 **************************/
/*!
 * \brief		Returns the HDRadio Instance used by the Receiver
 * \details		The HDRadio DCOP may support more than one channel (Instance in HDRadio terminology).
 * 				The function checks which Instance is configured for the Receiver.
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	pInstId   - pointer to a location where the function stores the Instance,
 * 				            or INSTANCE_UNDEF if there was an error.
 * \return		OSAL_OK    - the *pInstId* was correctly initialized
 * \return		OSAL_ERROR - the Receiver is not valid, or the Instance was not initialized
 * 				             In both cases *pInstId* contains INSTANCE_UNDEF
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetHdInstance(ETAL_HANDLE hReceiver, tyHDRADIOInstanceID *pInstId)
{
	etalReceiverStatusTy *recvp;
	tSInt retval = OSAL_ERROR;

	/*
	 * fake value used during system start-up when there
	 * are no receivers configured
	 */
	if (hReceiver == (ETAL_HANDLE)HDRADIO_FAKE_RECEIVER)
	{
		*pInstId = INSTANCE_1;
		retval = OSAL_OK;
		goto exit;
	}
	else if (hReceiver == (ETAL_HANDLE)HDRADIO_FAKE_RECEIVER2)
	{
		*pInstId = INSTANCE_2;
		retval = OSAL_OK;
		goto exit;
	}
	else
	{
		/* Nothing to do */
	}

	/*
	 * extract the HD instance ID
	 */
	recvp = ETAL_receiverGet(hReceiver);
	if (recvp != NULL)
	{
		*pInstId = recvp->HDRADIOConfig.instanceId;
		if ((*pInstId == INSTANCE_1) || (*pInstId == INSTANCE_2))
		{
			retval = OSAL_OK;
			goto exit;
		}
	}
	else
	{
		ASSERT_ON_DEBUGGING(0);
	}

	*pInstId = INSTANCE_UNDEF;

exit:
	return retval;
}

/***************************
 *
 * ETAL_receiverHasDigitalAudio_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks if the HDRadio Receiver acquired digital audio on the current frequency
 * \details		HDRadio signal is transmitted on a side band of the plain FM signal.
 * 				An ETAL HDRadio Receiver tunes to the FM signal and outputs immediately
 * 				the analog FM; acquiring the digital (i.e. HDRadio) audio requires some
 * 				additional time. When (if) the digital audio is available ETAL updates
 * 				an internal status. This function returns the internal status.
 * \remark		The switch from analogue to digital audio is currently controlled by the HW:
 * 				the HDRadio DCOP raises a hardware signal when the digital audio is available and
 * 				the signal is routed to a CMOST input used to select in hardware the FM or digital
 * 				input.
 * \remark		Note that digital audio acquisition depends on the FM signal quality, if the
 * 				FM signal is not good it may be possible that the DCOP does not manage to
 * 				acquire digital audio reliably.
 * \param[in]	hReceiver - the Receiver handle
 * \return		TRUE  - digital audio is acquired
 * \return		FALSE - digital audio is not acquired
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverHasDigitalAudio_HDRADIO(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	tBool retval;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		retval = FALSE;
	}
	else
	{
		retval = ((recvp->HDRADIOConfig.acquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) == (tU8)DIGITAL_AUDIO_ACQUIRED);
	}

	return  retval;
}

#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_receiverHasSystemData_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks if System Data is available for the current frequency
 * \details		HDRadio System Data comprises the Station information (e.g. station
 * 				name, current program information, ...). The System Data is
 * 				available some time after the Digital Audio acquisition.
 * \details 	The function first checks an internal status periodically updated
 * 				by ETAL (see ETAL_tuneFSM_HDRADIO). If this status does not indicate
 * 				System Data it polls the DCOP for an update. If after the direct
 *				request the System Data results acquired, the function also updates
 *				the internal status.
 * \param[in]	hReceiver - the Receiver handle
 * \return		TRUE  - the System Data is available
 * \return		FALSE - the System Data is not available
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverHasSystemData_HDRADIO(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
#ifndef HD_MONITORING	
	etalToHDRADIODigiAcqStatusTy digi_acq_status;
	tSInt retval;
	tS8 curr_prog;
#endif
	tBool retval = FALSE;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		retval = FALSE;
		goto exit;
	}
	if ((recvp->HDRADIOConfig.acquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED)
	{
		retval = TRUE;
		goto exit;
	}

// In case of HD MONITORING : the update is done automatically so the acquisition status is correct
#ifndef HD_MONITORING
	/*
	 * SIS not acquired at the time of the last receiver status update,
	 * check if things changed now
	 */
	retval = ETAL_cmdGetStatusDigital_HDRADIO(hReceiver, &digi_acq_status);
	if (retval == OSAL_OK)
	{
		if (((digi_acq_status.m_TuneOperationType == (tU8)ALL_TYPES_TUNED) ||
			(digi_acq_status.m_TuneOperationType == (tU8)ONLY_DIGITAL_TUNED)) &&
			(digi_acq_status.m_Band != (tU8)IDLE_MODE))
		{
			if ((digi_acq_status.m_DigiAcquisitionStatus & (tU8)DIGITAL_AUDIO_ACQUIRED) == (tU8)DIGITAL_AUDIO_ACQUIRED)
			{
				curr_prog = (tS8)MPS_AUDIO_HD_1;
			}
			else
			{
				curr_prog = ETAL_INVALID_PROG;
			}
			ETAL_receiverSetRadioInfo_HDRADIO(hReceiver, digi_acq_status.m_DigiAcquisitionStatus, curr_prog, digi_acq_status.m_AudioProgramsAvailable, 0x00);
			retval = (recvp->HDRADIOConfig.acquisitionStatus & (tU8)SIS_ACQUIRED) == (tU8)SIS_ACQUIRED;
			goto exit;
		}

	}
#endif

exit:
	return retval;

}
#endif // CONFIG_ETAL_HAVE_SYSTEMDATA || CONFIG_ETAL_HAVE_ALL_API
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/***************************
 *
 * ETAL_receiverSearchActiveSpecial
 *
 **************************/
/*!
 * \brief		Searches the first Receiver having a Special Command active
 * \param[in]	cmd - the Special Command to search
 * \return		Handle of the Receiver with the requested Special Command active
 * \return		ETAL_INVALID_HANDLE if no Special Command is active
 * \see			ETAL_receiverSetSpecial for a description of Special Command
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_receiverSearchActiveSpecial(etalCmdSpecialTy cmd)
{
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hReceiver;
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;
	ETAL_HINDEX receiver_index;
	tU32 i;

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		receiver_index = (ETAL_HINDEX)i;
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
		recvp = ETAL_receiverGet(hReceiver);
		if ((recvp != NULL) &&
			(!recvp->isValid ||
			!ETAL_receiverIsSpecialInProgress(hReceiver, cmd)))
		{
			continue;
		}
		else
		{
			retval = hReceiver;
			goto exit;
		}
	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_receiverIsSpecialInProgress
 *
 **************************/
/*!
 * \brief		Checks if the Receiver has a Special Command active
 * \details		If the *hReceiver* parameter is ETAL_INVALID_HANDLE this function
 * 				searches all the Receivers for the Special Command and returns
 * 				TRUE if at least one has the command in active.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	cmd - the Special Command to search
 * \return		TRUE  - the Receiver has the Special Command active or,
 * 				        if Receiver was ETAL_INVALID_HANDLE, at least one Receiver
 * 				        exists with the Special Command active
 * \return		FALSE - the Special Command is not in progress in the Receiver, or
 * 				        in any Receiver if ETAL_INVALID_HANDLE was specified
 * \see			ETAL_receiverSetSpecial for a description of Special Command
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsSpecialInProgress(ETAL_HANDLE hReceiver, etalCmdSpecialTy cmd)
{
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE l_hReceiver;
	ETAL_HINDEX receiver_index;
	tU32 i;
	tBool retval = FALSE;

	if (hReceiver != ETAL_INVALID_HANDLE)
	{
		recvp = ETAL_receiverGet(hReceiver);
		if ((recvp != NULL) &&
			recvp->isValid &&
			ETAL_receiverGetInProgress(recvp, cmd) == TRUE)
		{
			retval = TRUE;
			goto exit;
		}
		retval = FALSE;
		goto exit;
	}

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		receiver_index = (ETAL_HINDEX)i;
		l_hReceiver = ETAL_handleMakeReceiver(receiver_index);
		recvp = ETAL_receiverGet(l_hReceiver);
		if ((recvp != NULL) &&
			recvp->isValid &&
			ETAL_receiverGetInProgress(recvp, cmd) == TRUE)
		{
			retval = TRUE;
			goto exit;
		}
	}

exit:
	return retval;
}

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_receiverSearchFromApplication
 *
 **************************/
/*!
 * \brief		Searches the Receiver using a DAB Application
 * \details		DAB DCOP supports several channels (Applications in DAB DCOP
 * 				terminology). The Application identifier is normally
 * 				embedded in every response returned by the DCOP DAB to
 * 				ETAL, so ETAL can use it to infer which Receiver originated
 * 				the request.
 * \remark		Only defined for DAB DCOP.
 * \param[in]	app - the DAB DCOP Application identifier
 * \return		Handle of the Receiver using the Application
 * \return		ETAL_INVALID_HANDLE if no Receiver uses the Application or
 * 				DCOP DAB support is not available in ETAL
 * \see			DABMW_mwAppTy for the definition of the DAB DCOP Applications
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_receiverSearchFromApplication(tU8 app)
{
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hReceiver;
	ETAL_HINDEX receiver_index;
	tU32 i;

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		receiver_index = (ETAL_HINDEX)i;
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
		recvp = ETAL_receiverGet(hReceiver);
		if ((recvp != NULL) &&
			recvp->isValid &&
			(tU8)recvp->MDRConfig.application == app)
		{
			return hReceiver;
		}
	}
	return ETAL_INVALID_HANDLE;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_CMOST
/***************************
 *
 * ETAL_receiverSearchFromTunerId
 *
 **************************/
/*!
 * \brief		Searches the Receiver using the Tuner
 * \details		Receivers are stored in an array of size #ETAL_MAX_RECEIVERS.
 * 				The function loops over all the array entries, starting from
 * 				the entry defined in parameter *hReceiver_start*, until it
 * 				finds a Receiver configured to use the *hTuner*.
 * \param[in]	hTuner - the Tuner to search
 * \param[in]	hReceiver_start - if set to a valid Receiver handle, the function
 * 				                  starts searching the Receiver array from this handle;
 * 				                  if set to ETAL_INVALID_HANDLE, the function starts
 * 				                  searching from the first handle
 * \return		Handle of the Receiver using the *hTuner*, or ETAL_INVALID_HANDLE if not found
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_receiverSearchFromTunerId(ETAL_HANDLE hTuner, ETAL_HANDLE hReceiver_start)
{
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hReceiver;
	ETAL_HINDEX receiver_index;
	tU32 i, receiver_index_start;
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;

    if (hReceiver_start == ETAL_INVALID_HANDLE)
    {
        hReceiver_start = ETAL_handleMakeReceiver((ETAL_HINDEX)0);
    }

    if (ETAL_receiverIsValidHandle(hReceiver_start) == TRUE)
    {
		receiver_index_start = (tU32)ETAL_handleReceiverGetIndex(hReceiver_start);
    	for (i = receiver_index_start; i < ETAL_MAX_RECEIVERS; i++)
    	{
			receiver_index = (ETAL_HINDEX)i;
			hReceiver = ETAL_handleMakeReceiver(receiver_index);
    		recvp = ETAL_receiverGet(hReceiver);
    		if ((recvp != NULL) &&
    			recvp->isValid &&
    			recvp->CMOSTConfig.tunerId == hTuner)
    		{
    			retval = hReceiver;
    			goto exit;
    		}
    	}
    }

exit:
	return retval;
}
#endif // CONFIG_ETAL_SUPPORT_CMOST

/***************************
 *
 * ETAL_receiverStopAllSpecial
 *
 **************************/
/*!
 * \brief		Stops the Special Command for all the Receivers
 * \param[in]	cmd - the Special Command
 * \see			ETAL_receiverSetSpecial for a description of Special Command
 * \callgraph
 * \callergraph
 * \todo		The needed functionality would be to stop the Special Command only for
 * 				a specific Receiver; this has to be implemented yet but the current implementation
 * 				works since ETAL supports only one Special Command at a time
 */
tVoid ETAL_receiverStopAllSpecial(etalCmdSpecialTy cmd)
{
	ETAL_HANDLE hReceiver;
	ETAL_HINDEX receiver_index;
	tU32 i;

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		receiver_index = (ETAL_HINDEX)i;
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
		if (ETAL_receiverIsValidHandle(hReceiver))
		{
			ETAL_receiverSetSpecial(hReceiver, cmd, cmdActionStop);
		}
	}
}

/***************************
 *
 * ETAL_receiverGetRDSSlot
 *
 **************************/
/*!
 * \brief		Returns the RDS Slot for the Receiver
 * \details		The RDS Slot is a concept inherited from the RDS Landscape implementation
 * 				imported from the MDR3 project, where the RDS state is maintained
 * 				on Tuner-basis. ETAL makes sure that there is a unique
 * 				RDS slot identifier assigned to each Tuner Frontend.
 * \param[in]	hReceiver - the Receiver handle
 * \return		The unique RDS Slot, or -1 in case of errors
 * \see			ETAL_tunerInitRDSSlot
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetRDSSlot(ETAL_HANDLE hReceiver)
{
	etalDiversTy *divp;
	etalFrontendDescTy *fe_descp = NULL;
	etalReceiverStatusTy *recvp;
	tSInt retval = -1;

	recvp = ETAL_receiverGet(hReceiver);
	if (!ETAL_receiverIsValidHandle(hReceiver) ||
		!ETAL_receiverIsValidRDSHandle(hReceiver) ||
		(recvp == NULL) || (recvp->diversity.m_DiversityMode < (tU8)1))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = -1;
		goto exit;
	}

	/*
	 * Search the Frontend used by this receiver handle
	 */
	divp = &(recvp->diversity);
	if (divp->m_FeConfig[0] != ETAL_INVALID_HANDLE)
	{
		if (recvp->CMOSTConfig.processingFeatures.u.bf.m_fm_vpa == (tU8)1)
		{
			/* if VPA ON return RDS slot of foreground Front End */
			if (divp->m_DiversityMode >= (tU8)1)
			{
				if (ETAL_handleFrontendGetChannel(divp->m_FeConfig[0]) == ETAL_FE_FOREGROUND)
				{
					fe_descp = ETAL_tunerGetFrontend(divp->m_FeConfig[0]);
				}
			}
			if ((fe_descp == NULL) && (divp->m_DiversityMode >= (tU8)2))
			{
				if (ETAL_handleFrontendGetChannel(divp->m_FeConfig[1]) == ETAL_FE_FOREGROUND)
				{
					fe_descp = ETAL_tunerGetFrontend(divp->m_FeConfig[1]);
				}
			}
		}
		else
		{
			fe_descp = ETAL_tunerGetFrontend(divp->m_FeConfig[0]);
		}
		if (fe_descp)
		{
			retval = (fe_descp->RDSSlotIndex != ETAL_UNDEF_SLOT) ? (tSInt)fe_descp->RDSSlotIndex : -1;
			goto exit;
		}
	}

exit:
	return retval;
}


/***************************
 *
 * ETAL_receiverGetTunerId
 *
 **************************/
/*!
 * \brief		Returns the Tuner associated with a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	hTuner    - pointer to the location where the function stores the Tuner handle
 * \return		OSAL_OK    - The Tuner handle was found
 * \return		OSAL_ERROR - The Tuner handle was not found due to invalid Receiver, or CMOST support is not available in ETAL
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetTunerId(ETAL_HANDLE hReceiver, ETAL_HANDLE *hTuner)
{
	tSInt retval = OSAL_ERROR;

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp != NULL)
	{
		*hTuner = recvp->CMOSTConfig.tunerId;
		retval = OSAL_OK;
	}
	else
#endif
	{
		ASSERT_ON_DEBUGGING(0);
	}
	return retval;
}

/***************************
 *
 * ETAL_receiverGetChannel
 *
 **************************/
/*!
 * \brief		Returns the Tuner channel (a.k.a. Frontend) used by the Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	pchannel  - pointer to the location where the function stores the channel identifier
 * \return		OSAL_OK    - The channel was found
 * \return		OSAL_ERROR - The channel was not found due to invalid Receiver, or CMOST support is not available in ETAL
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetChannel(ETAL_HANDLE hReceiver, etalChannelTy *pchannel)
{
	tSInt retval = OSAL_ERROR;

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp != NULL)
	{
		*pchannel = recvp->CMOSTConfig.channel;
		retval = OSAL_OK;
	}
#endif

	return retval;
}

/***************************
 *
 * ETAL_receiverSetMute
 *
 **************************/
/*!
 * \brief		Sets the mute flag for the Receiver
 * \remark		The function only affects the ETAL internal status, no command is sent to the device.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	mute      - the mute status
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetMute(ETAL_HANDLE hReceiver, tBool mute)
{
	etalReceiverStatusTy *recvp;
	recvp = ETAL_receiverGet(hReceiver);
	if (recvp)
	{
		recvp->receiverMuted = mute;
	}
}


/***************************
 *
 * ETAL_receiverIsMute
 *
 **************************/
/*!
 * \brief		Returns the mute status of the Receiver
 * \remark		The function only reads the ETAL internal status, not the actual device status.
 * \param[in]	hReceiver - the Receiver handle
 * \return		The mute status (TRUE if muted)
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsMute(ETAL_HANDLE hReceiver)
{
	tBool retval;
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		retval = FALSE;
	}
	else
	{
		retval = recvp->receiverMuted;
	}
	return retval;
}

/***************************
 *
 * ETAL_receiverSetFMAudioStereo
 *
 **************************/
/*!
 * \brief		Sets the default stereo flag for the Receiver
 * \remark		The function only affects the ETAL internal status, no command is sent to the device.
 * \param[in]	hReceiver - the Receiver handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverInitFMAudioStereo(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp)
	{
		recvp->CMOSTConfig.audioChannel = ETAL_AUDIO_CHN_UNDEFINED;
	}
}

/***************************
 *
 * ETAL_receiverSetFMAudioStereo
 *
 **************************/
/*!
 * \brief		Sets the stereo flag for the Receiver
 * \remark		The function only affects the ETAL internal status, no command is sent to the device.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	stereo      - the stereo status
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverSetFMAudioStereo(ETAL_HANDLE hReceiver, tBool stereo)
{
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp)
	{
		if (stereo)
		{
			recvp->CMOSTConfig.audioChannel = ETAL_AUDIO_CHN_STEREO;
		}
		else
		{
			recvp->CMOSTConfig.audioChannel = ETAL_AUDIO_CHN_MONO;
		}
	}
}

/***************************
 *
 * ETAL_receiverIsFMAudioStereo
 *
 **************************/
/*!
 * \brief		Returns the stereo status of the Receiver
 * \remark		The function only reads the ETAL internal status, not the actual device status.
 * \param[in]	hReceiver - the Receiver handle
 * \return		The stereo status (TRUE if muted)
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsFMAudioStereo(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;
	tBool retval;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		retval = FALSE;
	}
	else
	{
		retval = (recvp->CMOSTConfig.audioChannel == ETAL_AUDIO_CHN_STEREO);
	}
	return retval;
}

/***************************
 *
 * ETAL_receiverIsFMAudioStereo
 *
 **************************/
/*!
 * \brief		Returns the stereo status of the Receiver
 * \remark		The function only reads the ETAL internal status, not the actual device status.
 * \param[in]	hReceiver - the Receiver handle
 * \return		The stereo status (TRUE if muted)
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverIsFMAudioMono(ETAL_HANDLE hReceiver)
{
	tBool retval;
	
	etalReceiverStatusTy *recvp;
	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		retval = TRUE;
	}
	else
	{
		retval = (recvp->CMOSTConfig.audioChannel == ETAL_AUDIO_CHN_MONO);
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverConvertEtalBcastDataTypeToIndex
 *
 **************************/
/*!
 * \brief		Converts a Broadcast Data Type to a numerical index
 * \details		ETAL supports one Datapath per Data Type per Receiver.
 * 				Each Datapath status is maintained in a Receiver-private
 * 				array. This function returns the index to that array location.
 * \remark		#ETAL_DATA_TYPE_AUDIO and #ETAL_DATA_TYPE_DCOP_AUDIO are not processed
 * 				through Datapath thus ETAL does not reserve any array location for it.
 * \param[in]	dtype - the Broadcast Data Type
 * \return		The #ETAL_HINDEX corresponding to the Data Type, which can be used
 * 				to access the Receiver Datapath array or to build an #ETAL_HANDLE.
 * 				#ETAL_INVALID_HINDEX in case of invalid *dtype*.
 * \callgraph
 * \callergraph
 */
static ETAL_HINDEX ETAL_receiverConvertEtalBcastDataTypeToIndex(EtalBcastDataType dtype)
{
	ETAL_HINDEX retval = ETAL_INVALID_HINDEX;
	tBool assert = TRUE;
	
	switch (dtype)
	{
		case ETAL_DATA_TYPE_UNDEF:
		case ETAL_DATA_TYPE_AUDIO:
		case ETAL_DATA_TYPE_DCOP_AUDIO:
			retval = ETAL_INVALID_HINDEX;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_DATA_SERVICE:
			retval = (ETAL_HINDEX)0;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_DAB_DATA_RAW:
			retval = (ETAL_HINDEX)1;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_DAB_AUDIO_RAW:
			retval = (ETAL_HINDEX)2;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_DAB_FIC:
			retval = (ETAL_HINDEX)3;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_FM_RDS:
			retval = (ETAL_HINDEX)4;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_FM_RDS_RAW:
			retval = (ETAL_HINDEX)5;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_TEXTINFO:
			retval = (ETAL_HINDEX)6;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_DAB_DLS:
			retval = (ETAL_HINDEX)7;
			assert = FALSE;
			break;

		case ETAL_DATA_TYPE_DAB_DLPLUS:
			retval = (ETAL_HINDEX)8;
			assert = FALSE;
			break;

		/* don't use a default label to trigger a compiler
		 * warning in case a value is added to EtalBcastDataType
		 * without updating this function */
	}
	/* memory corruption, or the EtalBcastDataType
	 * was modified and this function not updated */
	if(assert == TRUE)
	{
		ASSERT_ON_DEBUGGING(0);
	}
	return retval;
}

/***************************
 *
 * ETAL_receiverConvertIndexToEtalBcastDataType
 *
 **************************/
/*!
 * \brief		Converts an #ETAL_HINDEX to a Broadcast Data Type
 * \param[in]	index - the index to convert
 * \return		The Broadcast Data Type, or ETAL_DATA_TYPE_UNDEF if
 * 				*index* is invalid.
 * \see			ETAL_receiverConvertEtalBcastDataTypeToIndex
 * \callgraph
 * \callergraph
 */
static EtalBcastDataType ETAL_receiverConvertIndexToEtalBcastDataType(ETAL_HINDEX index)
{
	EtalBcastDataType retval;

	switch (index)
	{
		case ETAL_INVALID_HINDEX:
			retval = ETAL_DATA_TYPE_UNDEF; // might as well be ETAL_DATA_TYPE_AUDIO
			break;
		case 0:
			retval = ETAL_DATA_TYPE_DATA_SERVICE;
			break;
		case 1:
			retval = ETAL_DATA_TYPE_DAB_DATA_RAW;
			break;
		case 2:
			retval = ETAL_DATA_TYPE_DAB_AUDIO_RAW;
			break;
		case 3:
			retval = ETAL_DATA_TYPE_DAB_FIC;
			break;
		case 4:
			retval = ETAL_DATA_TYPE_FM_RDS;
			break;
		case 5:
			retval = ETAL_DATA_TYPE_FM_RDS_RAW;
			break;
		case 6:
			retval = ETAL_DATA_TYPE_TEXTINFO;
			break;
		case 7:
			retval = ETAL_DATA_TYPE_DAB_DLS;
			break;
		case 8:
			retval = ETAL_DATA_TYPE_DAB_DLPLUS;
			break;
		default:
			/* memory corruption, or the EtalBcastDataType was modified
			 * and this function not updated */
			ASSERT_ON_DEBUGGING(0);
			retval = ETAL_DATA_TYPE_UNDEF;
			break;
	}

	return retval;
}


/***************************
 *
 * ETAL_receiverAddDatapath
 *
 **************************/
/*!
 * \brief		Updates a Receiver's Datapath array with the given Datapath properties
 * \details		The function extracts from the Datapath properties the Receiver
 * 				handle and from the Datapath Data Type the Datapath array index.
 * 				If the array location is empty, the function fills it with
 * 				the properties of *pDatapathAttr* and returns the handle of
 * 				the new Datapath; if it is non-empty, it
 * 				does nothing and returns #ETAL_INVALID_HANDLE.
 * \param[in]	pDatapathAttr - pointer to the description of the Datapath to be added
 * \return		The ETAL_HANDLE of the new Datapath, or #ETAL_INVALID_HANDLE
 * 				if the Datapath cannot be added because the corresponding Receiver
 * 				description is already used or the Datapath handle is invalid.
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_receiverAddDatapath(const EtalDataPathAttr *pDatapathAttr)
{
	etalDatapathTy *dpath;
	ETAL_HINDEX datapath_index;
	ETAL_HINDEX receiver_index;
	ETAL_HANDLE hReceiver;
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;

	hReceiver = pDatapathAttr->m_receiverHandle;
	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);
	datapath_index = ETAL_receiverConvertEtalBcastDataTypeToIndex(pDatapathAttr->m_dataType);
	if (datapath_index != ETAL_INVALID_HINDEX)
	{
		dpath = &etalReceivers[receiver_index].datapaths[datapath_index];
		if (dpath->m_sink.m_CbProcessBlock == NULL)
		{
			(void)OSAL_pvMemoryCopy((tVoid *)&(dpath->m_sink), (tPCVoid)&(pDatapathAttr->m_sink), sizeof(EtalSink));
			retval = ETAL_handleMakeDatapath(receiver_index, datapath_index);
		}
		/* avoid overwriting an existing datapath */
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverGetFromDatapath
 *
 **************************/
/*!
 * \brief		Extracts the Receiver handle from a Datapath handle
 * \details		The #ETAL_HANDLE for a Datapath encodes the Receiver
 * 				handle also. This function extracts it.
 * \param[in]	hDatapath - the Datapath handle
 * \return		the Receiver handle
 * \see			ETAL_handleMake for detailed description on the ETAL_HANDLE
 * 				encoding.
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_receiverGetFromDatapath(ETAL_HANDLE hDatapath)
{
	return ETAL_handleDatapathGetReceiver(hDatapath);
}

/***************************
 *
 * ETAL_receiverGetDataTypeForDatapath
 *
 **************************/
/*!
 * \brief		Returns the Broadcast Data Type of a Datapath
 * \param[in]	hDatapath - the Datapath handle
 * \return		The Broadcast Data Type
 * \see			ETAL_receiverConvertEtalBcastDataTypeToIndex
 * \callgraph
 * \callergraph
 */
EtalBcastDataType ETAL_receiverGetDataTypeForDatapath(ETAL_HANDLE hDatapath)
{
	ETAL_HINDEX datapath_index = ETAL_handleDatapathGetIndex(hDatapath);
	return ETAL_receiverConvertIndexToEtalBcastDataType(datapath_index);
}

/***************************
 *
 * ETAL_receiverGetDatapath
 *
 **************************/
/*!
 * \brief		Returns the pointer to the internal status of a Datapath
 * \details		Datapath statuses are stored in a Receiver private
 * 				array. This function returns the pointer to the
 * 				array location corresponding to *hDatapath*.
 * \param[in]	hDatapath - Datapath handle
 * \return		a pointer to the Datapath's internal status
 * \callgraph
 * \callergraph
 */
static etalDatapathTy *ETAL_receiverGetDatapath(ETAL_HANDLE hDatapath)
{
	ETAL_HANDLE hReceiver;
	ETAL_HINDEX datapath_index;
	ETAL_HINDEX receiver_index;
	etalReceiverStatusTy *recvp;
	etalDatapathTy *retval;

	datapath_index = ETAL_handleDatapathGetIndex(hDatapath);
	receiver_index = ETAL_handleDatapathGetReceiverIndex(hDatapath);

	if ((datapath_index == ETAL_INVALID_HINDEX) ||
		((tU32)datapath_index >= ETAL_MAX_DATAPATH_PER_RECEIVER))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = NULL;
	}
	else
	{
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
		if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
		{
			ASSERT_ON_DEBUGGING(0);
			retval = NULL;
		}
		else
		{
			retval = &recvp->datapaths[datapath_index];
		}
	}
	
	return retval;
}

/***************************
 *
 * ETAL_receiverGetDatapathFromIndex
 *
 **************************/
/*!
 * \brief		Returns the pointer to the internal status of a Datapath
 * \details		This function is similar to #ETAL_receiverGetDatapath but
 * 				accepts the Receiver index and Datapath index instead of
 * 				the Datapath handle.
 * \remark		Receiver and Datapath indexes are normally not exported
 * 				outside of ETAL
 * \param[in]	receiver_index - the Receiver index
 * \param[in]	datapath_index - the Datapath index
 * \return		a pointer to the Datapath's internal status
 * \see			ETAL_handleMake for details on Receiver and Datapath index
 * \callgraph
 * \callergraph
 */
static etalDatapathTy *ETAL_receiverGetDatapathFromIndex(ETAL_HINDEX receiver_index, ETAL_HINDEX datapath_index)
{
	etalReceiverStatusTy *recvp;
	ETAL_HANDLE hReceiver;
	etalDatapathTy *retval;

	hReceiver = ETAL_handleMakeReceiver(receiver_index);
	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = NULL;
	}
	else
	{
		retval = &recvp->datapaths[datapath_index];
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverDatapathGetSink
 *
 **************************/
/*!
 * \brief		Returns the sink for a Datapath
 * \details		The sink is the user-defined function that ETAL should
 * 				invoke each time there is new data on the Datapath.
 * \param[in]	hDatapath - the Datapath handle
 * \return		the function pointer to the sink function, or NULL if
 * 				the provided Datapath is invalid.
 * \callgraph
 * \callergraph
 */
EtalSink *ETAL_receiverDatapathGetSink(ETAL_HANDLE hDatapath)
{
	etalDatapathTy *dpath;
	EtalSink *retval;

	if ((dpath = ETAL_receiverGetDatapath(hDatapath)) == NULL)
	{
		retval = NULL;
	}
	else
	{
		retval = &dpath->m_sink;
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverDatapathIsValid
 *
 **************************/
/*!
 * \brief		Checks if a Datapath handle is valid
 * \details		A Datapath handle is valid if:
 * 				- it is of type Datapath
 * 				- it has a valid (non-NULL) sink function pointer
 *
 * \param[in]	hDatapath - the Datapath handle
 * \return		TRUE if the Datapath handle is valid, FALSE otherwise
 * \see			ETAL_receiverDatapathGetSink for the definition of sink.
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverDatapathIsValid(ETAL_HANDLE hDatapath)
{
	etalDatapathTy *dpath;
	tBool retval = TRUE;

	if (!ETAL_handleIsDatapath(hDatapath))
	{
		retval = FALSE;
	}
	else
	{
		dpath = ETAL_receiverGetDatapath(hDatapath);
		if (dpath == NULL)
		{
			retval = FALSE;
		}
		else if (dpath->m_sink.m_CbProcessBlock == NULL)
		{
			retval = FALSE;
		}
		else
		{
			/* Nothing to do */
		}
	}
	return retval;
}

/***************************
 *
 * ETAL_receiverDatapathIsValidAndEmpty
 *
 **************************/
/*!
 * \brief		Checks if a Datapath handle is valid and points to a non-initialized Datapath
 * \details		This function is similar to #ETAL_receiverDatapathIsValid but
 * 				returns true if the Datapath is not initialized. A Datapath
 * 				is considered not initialized if it contains a NULL sink function pointer.
 * \param[in]	hDatapath - the Datapath handle
 * \return		TRUE if the Datapath handle is valid and refers to and empty Datapath entry,
 * 				FALSE otherwise
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverDatapathIsValidAndEmpty(ETAL_HANDLE hDatapath)
{
	etalDatapathTy *dpath;
	tBool retval = FALSE;

	if (!ETAL_handleIsDatapath(hDatapath))
	{
		retval = FALSE;
	}
	else
	{
		dpath = ETAL_receiverGetDatapath(hDatapath);
		if (dpath == NULL)
		{
			retval = FALSE;
		}
		else if (dpath->m_sink.m_CbProcessBlock == NULL)
		{
			retval = TRUE;
		}
		else
		{
			/* Nothing to do */
		}
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverDatapathTypeIs
 *
 **************************/
/*!
 * \brief		Checks if a Datapath is of the specified Broadcast Data Type
 * \param[in]	hDatapath - the Datapath handle
 * \param[in]	type - the Broadcast Data Type to be checked
 * \return		TRUE if the Datapath Data Type is *type*, FALSE otherwise
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverDatapathTypeIs(ETAL_HANDLE hDatapath, EtalBcastDataType type)
{
	EtalBcastDataType data_type;

	data_type = ETAL_receiverGetDataTypeForDatapath(hDatapath);
	return (data_type == type);
}

/***************************
 *
 * ETAL_receiverDatapathIsRawRDS
 *
 **************************/
/*!
 * \brief		Checks if a Datapath has Data Type #ETAL_DATA_TYPE_FM_RDS_RAW
 * \param[in]	hDatapath - the Datapath handle
 * \return		TRUE if the Datapath Data Type is of type #ETAL_DATA_TYPE_FM_RDS_RAW,
 * 				FALSE otherwise or in case of invalid Datapath
 * \callgraph
 * \callergraph
 */
tBool ETAL_receiverDatapathIsRawRDS(ETAL_HANDLE hDatapath)
{
	ETAL_HANDLE hReceiver;
	EtalBcastDataType data_type;
	tBool retval;

	hReceiver = ETAL_handleDatapathGetReceiver(hDatapath);
	data_type = ETAL_receiverGetDataTypeForDatapath(hDatapath);

	if (data_type == ETAL_DATA_TYPE_UNDEF)
	{
		retval = FALSE;
	}
	else
	{
		retval = (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_FM) && (data_type == ETAL_DATA_TYPE_FM_RDS_RAW);
	}

	return retval;
}

/***************************
 *
 * ETAL_receiverDatapathDestroy
 *
 **************************/
/*!
 * \brief		Destroys a Datapath and stops all the associated actions
 * \details		The function does the following:
 * 				- stops all periodic internal callbacks (#ETAL_intCbDeregisterPeriodicDatapath)
 * 				  for this Datapath
 * 				- stops all non-periodic internal callbacks (#ETAL_intCbDeregisterDatapath)
 * 				  for this Datapath
 * 				- resets to 0 the Datapath internal status
 *
 * \details		If the Datapath handle is invalid the function returns OSAL_OK
 * 				and does nothing, because it is considered a legal operation.
 *
 * \param[in]	hDatapath - the Datapath handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverDatapathDestroy(ETAL_HANDLE hDatapath)
{
	etalDatapathTy *dpath;
	ETAL_HANDLE hReceiver;
	EtalBcastDataType data_type;
	tSInt vl_ret = OSAL_OK;

	if (!ETAL_receiverDatapathIsValid(hDatapath))
	{
		/* exit without doing something*/
	}
	else
	{
		ETAL_intCbDeregisterPeriodicDatapath(hDatapath);
		ETAL_intCbDeregisterDatapath(hDatapath);

		hReceiver = ETAL_handleDatapathGetReceiver(hDatapath);
				
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ		
		ETAL_statusStopRDSIrq(hReceiver);
#endif

		// stop on going activity since there is no more datapath...
		//
	
		data_type = ETAL_receiverGetDataTypeForDatapath(hDatapath);

		switch (data_type)
			{
			case ETAL_DATA_TYPE_UNDEF:
			case ETAL_DATA_TYPE_AUDIO:
			case ETAL_DATA_TYPE_DCOP_AUDIO:
				break;
			
			case ETAL_DATA_TYPE_DATA_SERVICE:

				break;
			
			case ETAL_DATA_TYPE_DAB_DATA_RAW:

				break;
			
			case ETAL_DATA_TYPE_DAB_AUDIO_RAW:

				break;
			
			case ETAL_DATA_TYPE_DAB_FIC:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)
				// stop the Raw FIC
				//
				if (OSAL_OK != ETAL_cmdGetFIC_MDR(hReceiver, cmdActionStop))
				{
					vl_ret = ETAL_RET_ERROR;
				}
#endif
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
				break;
			
			case ETAL_DATA_TYPE_FM_RDS:
				// stop decoded RDS
				break;
			
			case ETAL_DATA_TYPE_FM_RDS_RAW:

				break;
			
			case ETAL_DATA_TYPE_TEXTINFO:
				// stop RADIO  TEXT.
				
				break;
			
			case ETAL_DATA_TYPE_DAB_DLS:

				break;
			
			case ETAL_DATA_TYPE_DAB_DLPLUS:

				break;

			default:
				break;
			}


		dpath = ETAL_receiverGetDatapath(hDatapath);
		if (dpath != NULL)
		{
			(void)OSAL_pvMemorySet((tVoid *)dpath, 0x00, sizeof(etalDatapathTy));
		}
	}

	// warning tmp removal
	(tVoid) vl_ret;
	(tVoid) hReceiver;
	
	return;
}

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_receiverConvertDABMWDataServiceTypeToInternal
 *
 **************************/
/*!
 * \brief		Converts a DABMW Data Service type identifier to an ETAL Data Service type identifier
 * \details		The DAB DCOP (MDR) provides the Data Service type
 * 				in certain command responses. This function converts
 * 				the DABMW identifier into the ETAL correspondent.
 * \param[in]	dabmw_dstype - the DABMW Service type
 * \return		The ETAL Data Service type, or #ETAL_DATASERV_TYPE_UNDEFINED for
 * 				unknown DABMW Data Service type
 * \callgraph
 * \callergraph
 */
static EtalDataServiceType ETAL_receiverConvertDABMWDataServiceTypeToInternal(tU8 dabmw_dstype)
{
	switch (dabmw_dstype)
	{
		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_RAW:
			return ETAL_DATASERV_TYPE_EPG_RAW;
	
		case DABMW_DATACHANNEL_DECODED_TYPE_SLS:
			return ETAL_DATASERV_TYPE_SLS;

		case DABMW_DATACHANNEL_DECODED_TYPE_SLS_XPAD:
			return ETAL_DATASERV_TYPE_SLS_XPAD;

		case DABMW_DATACHANNEL_DECODED_TYPE_TPEG_RAW:
			return ETAL_DATASERV_TYPE_TPEG_RAW;

		case DABMW_DATACHANNEL_DECODED_TYPE_TPEG_SNI:
			return ETAL_DATASERV_TYPE_TPEG_SNI;

		case DABMW_DATACHANNEL_DECODED_TYPE_SLI:
			return ETAL_DATASERV_TYPE_SLI;

		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_BIN:
			return ETAL_DATASERV_TYPE_EPG_BIN;

		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_SRV:
			return ETAL_DATASERV_TYPE_EPG_SRV;

		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_PRG:
			return ETAL_DATASERV_TYPE_EPG_PRG;

		case DABMW_DATACHANNEL_DECODED_TYPE_EPG_LOGO:
			return ETAL_DATASERV_TYPE_EPG_LOGO;

		case DABMW_DATACHANNEL_DECODED_TYPE_JML_OBJ:
			return ETAL_DATASERV_TYPE_JML_OBJ;

		case DABMW_DATACHANNEL_DECODED_TYPE_BWS_RAW:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_OBJ:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_TM:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_QUALITY:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_FAC:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_SDC:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_PCMAUDIO:
		case DABMW_DATACHANNEL_DECODED_TYPE_DRM_MDI:
		case DABMW_DATACHANNEL_DECODED_TYPE_DEBUGDUMP:
			/* support for these not implemented yet in ETAL */
			return ETAL_DATASERV_TYPE_UNDEFINED;

		case DABMW_DATACHANNEL_DECODED_TYPE_UNDEF:
			/* something really screwed up the DCOP */
			ASSERT_ON_DEBUGGING(0);
			return ETAL_DATASERV_TYPE_UNDEFINED;
	}
	/*
	 * should not arrive here, the switch above needs to be updated
	 * with some new DABMW values
	 */
	ASSERT_ON_DEBUGGING(0);

	return ETAL_DATASERV_TYPE_UNDEFINED;
}

/***************************
 *
 * ETAL_receiverSetAutonotification_MDR
 *
 **************************/
/*!
 * \brief		Update the Receiver status of enabled DAB auto notification
 * \details		A DAB radio station may provide a number of auto notification
 * 				carrying e.g. DAB status, announcement switching and so on.
 *              Each auto notification Service may be enabled/disabled dynamically by the ETAL
 * 				API user, so ETAL maintains an internal state of which services
 * 				are currently enabled for a Receiver.
 *
 * 				This function updates the Receiver internal status of the
 * 				enabled DAB auto notification by adding the specified service.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	eventBitMap - the auto notification bit map to set to enable event notification
 * \return		OSAL_OK
 * \return		OSAL_ERROR - the Receiver handle or the auto notification Service is invalid
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverSetAutonotification_MDR(ETAL_HANDLE hReceiver, EtalAutonotificationEventType eventBitMap)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	if ((eventBitMap & ETAL_AUTONOTIF_TYPE_UNDEFINED) != 0)
	{
		return OSAL_ERROR;
	}

	recvp->MDRConfig.enabledAutonotificationEventBitmap = eventBitMap;
	return OSAL_OK;
}

/***************************
 *
 * ETAL_receiverGetAutonotification_MDR
 *
 **************************/
/*!
 * \brief		Returns the list of DAB auto notification currently enabled for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	serviceBitmap - pointer to a location where the function stores
 * 				                the list of currently enabled services.
 * \return		The list of enabled services, that is a bitmap of #EtalDataServiceType
 * 				values.
 * \see			ETAL_receiverSetDataService_MDR
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetAutonotification_MDR(ETAL_HANDLE hReceiver, tU16 *eventBitmap)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	*eventBitmap = recvp->MDRConfig.enabledAutonotificationEventBitmap;
	return OSAL_OK;
}

/***************************
 *
 * ETAL_receiverSetDataService_MDR
 *
 **************************/
/*!
 * \brief		Update the Receiver status of enabled DAB Data Services
 * \details		A DAB radio station may provide a number of Data Services
 * 				carrying e.g. Electronic Program Guide (EPG), Slideshow (SLS),
 * 				Journaline and so on. ETAL may transfer these services
 * 				using a Datapath of type #ETAL_DATA_TYPE_DATA_SERVICE. Each
 * 				Data Service may be enabled/disabled dynamically by the ETAL
 * 				API user, so ETAL maintains an internal state of which services
 * 				are currently enabled for a Receiver.
 *
 * 				This function updates the Receiver internal status of the
 * 				enabled DAB Data Services by adding the specified service.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	service - the Data Service to set to enabled state
 * \return		OSAL_OK
 * \return		OSAL_ERROR - the Receiver handle or the Data Service is invalid
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverSetDataService_MDR(ETAL_HANDLE hReceiver, EtalDataServiceType service)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	if ((service & ETAL_DATASERV_TYPE_UNDEFINED) != 0)
	{
		return OSAL_ERROR;
	}
	recvp->MDRConfig.enabledDataServiceBitmap |= service;

	return OSAL_OK;
}

/***************************
 *
 * ETAL_receiverClearDataService_MDR
 *
 **************************/
/*!
 * \brief		Update the Receiver status of enabled DAB Data Services
 * \details		This function removes the specified services from the list
 * 				of DAB Data services enabled for the Receiver.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	service - the Data Service to set to disabled state
 * \return		OSAL_OK
 * \return		OSAL_ERROR - the Receiver handle or the Data Service is invalid
 * \see			ETAL_receiverSetDataService_MDR
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverClearDataService_MDR(ETAL_HANDLE hReceiver, EtalDataServiceType service)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	if ((service & ETAL_DATASERV_TYPE_UNDEFINED) != 0)
	{
		return OSAL_ERROR;
	}
	recvp->MDRConfig.enabledDataServiceBitmap &= ~service;

	return OSAL_OK;
}

/***************************
 *
 * ETAL_receiverGetDataServices_MDR
 *
 **************************/
/*!
 * \brief		Returns the list of DAB Data Services currently enabled for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	serviceBitmap - pointer to a location where the function stores
 * 				                the list of currently enabled services.
 * \return		The list of enabled services, that is a bitmap of #EtalDataServiceType
 * 				values.
 * \see			ETAL_receiverSetDataService_MDR
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetDataServices_MDR(ETAL_HANDLE hReceiver, tU32 *serviceBitmap)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	*serviceBitmap = recvp->MDRConfig.enabledDataServiceBitmap;

	return OSAL_OK;
}

/***************************
 *
 * ETAL_receiverGetDatapathFromDABType
 *
 **************************/
/*!
 * \brief		Returns the handle of the Datapath supporting the specified DAB Data Service
 * \details		The function checks if the Receiver is configured to process
 * 				the specified DAB Data Service, that is if the Data Service
 * 				is enabled for the Receiver, and if the Datapath of Data Type
 * 				#ETAL_DATA_TYPE_DATA_SERVICE is configured (i.e. it has a non-NULL
 * 				sink function).
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	dabmw_dstype - the DAB Data Service (in DABMW notation)
 * \param[out]	context - pointer to a location where the function stores the pointer
 * 				          to the context associated to the sink function at Datapath
 * 				          creation.
 * \return		The Datapath handle, or #ETAL_INVALID_HANDLE if the Data Service is not
 * 				enabled for the Receiver, or the Receiver is invalid, or the
 * 				Datapath of type #ETAL_DATA_TYPE_DATA_SERVICE is not configured for the Receiver.
 * 				If the function returns #ETAL_INVALID_HANDLE *context* is unchanged.
 * \see			ETAL_receiverSetDataService_MDR
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_receiverGetDatapathFromDABType(ETAL_HANDLE hReceiver, DABMWDataServiceType dabmw_dstype, tVoid **context)
{
	etalReceiverStatusTy *recvp;
	etalDatapathTy *dpath;
	ETAL_HANDLE hDatapath;
	ETAL_HINDEX receiver_index;
	EtalDataServiceType dstype;

	recvp = ETAL_receiverGet(hReceiver);
	if (recvp == NULL)
	{
		return ETAL_INVALID_HANDLE;
	}

	/*
	 * if the data service is not enabled for the receiver
	 * skip all the rest
	 */
	dstype = ETAL_receiverConvertDABMWDataServiceTypeToInternal((tU8)dabmw_dstype);
	if ((dstype == ETAL_DATASERV_TYPE_UNDEFINED) ||
		((recvp->MDRConfig.enabledDataServiceBitmap & dstype) != dstype))
	{
		return ETAL_INVALID_HANDLE;
	}

	/*
	 * We are searching the datapath for a DABMW DAB DECODED stream
	 * which in ETAL is named ETAL_DATA_TYPE_DATA_SERVICE
	 * we use this information and the hReceiver to create the datapath handle
	 */
	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);
	if (receiver_index == ETAL_INVALID_HINDEX)
	{
		return ETAL_INVALID_HANDLE;
	}
	hDatapath = ETAL_handleMakeDatapath(receiver_index, ETAL_receiverConvertEtalBcastDataTypeToIndex(ETAL_DATA_TYPE_DATA_SERVICE));
	dpath = ETAL_receiverGetDatapath(hDatapath);
	if (dpath->m_sink.m_CbProcessBlock != NULL)
	{
		*context = dpath->m_sink.m_context;
		return hDatapath;
	}
	return ETAL_INVALID_HANDLE;
}

#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
/***************************
 *
 * ETAL_receiverSetSubch_MDR
 *
 **************************/
/*!
 * \brief		Update the Receiver status of DAB subch
 * \details		Set the DAB subch.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	subch - the subch used
 * \return		OSAL_OK
 * \return		OSAL_ERROR - the Receiver handle or the subch is invalid
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverSetSubch_MDR(ETAL_HANDLE hReceiver, tU8 subch)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	if (subch >= ETAL_MAX_NUM_SUBCH_PER_ENSEMBLE)
	{
		return OSAL_ERROR;
	}
	recvp->MDRConfig.subch = subch;

	return OSAL_OK;
}

/***************************
 *
 * ETAL_receiverClearSubch_MDR
 *
 **************************/
/*!
 * \brief		Update the Receiver status of DAB subch
 * \details		This function removes the subch for the Receiver.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	service - the Data Service to set to disabled state
 * \return		OSAL_OK
 * \return		OSAL_ERROR - the Receiver handle or the Data Service is invalid
 * \see			ETAL_receiverSetSubch_MDR
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverClearSubch_MDR(ETAL_HANDLE hReceiver)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	recvp->MDRConfig.subch = ETAL_INVALID_SUBCH_ID;

	return OSAL_OK;
}

/***************************
 *
 * ETAL_receiverGetSubch_MDR
 *
 **************************/
/*!
 * \brief		Returns the DAB subch currently enabled for a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \param[out]	psubch - pointer to a location where the function stores the DAB subch
 * \return		The DAB subch.
 * \see			ETAL_receiverSetSubch_MDR
 * \callgraph
 * \callergraph
 */
tSInt ETAL_receiverGetSubch_MDR(ETAL_HANDLE hReceiver, tU8 *psubch)
{
	etalReceiverStatusTy *recvp;

	if (!ETAL_receiverIsValidHandleInternal(hReceiver, &recvp))
	{
		return OSAL_ERROR;
	}
	*psubch = recvp->MDRConfig.subch;

	return OSAL_OK;
}
#endif /* CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST */

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

/***************************
 *
 * ETAL_receiverGetDatapathFromDataType
 *
 **************************/
/*!
 * \brief		Returns the Datapath handle corresponding to the Broadcast Data Type
 * \details		Each Receiver has a number of Datapaths potentially associated, indexed
 * 				by the Broadcast Data Type.
 *
 * 				This function returns the handle of the Datapath corresponding to the
 * 				given Data Type.
 * \param[in]	hReceiver - the Receiver handle
 * \param[in]	type - the Broadcast Data Type
 * \return		The Datapath handle, or ETAL_INVALID_HANDLE if the Datapath is empty
 * 				or *type* is invalid.
 * \see			ETAL_receiverConvertEtalBcastDataTypeToIndex
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_receiverGetDatapathFromDataType(ETAL_HANDLE hReceiver, EtalBcastDataType type)
{
	etalDatapathTy *dpath;
	ETAL_HINDEX datapath_index;
	ETAL_HINDEX receiver_index;
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;

	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);
	datapath_index = ETAL_receiverConvertEtalBcastDataTypeToIndex(type);

	if (datapath_index != ETAL_INVALID_HINDEX)
	{
		dpath = ETAL_receiverGetDatapathFromIndex(receiver_index, datapath_index);
		if (dpath == NULL)
		{
			retval = ETAL_INVALID_HANDLE;
		}
		else if (dpath->m_sink.m_CbProcessBlock != NULL)
		{
			retval = ETAL_handleMakeDatapath(receiver_index, datapath_index);
		}
		else
		{
			/* Nothing to do */
		}
	}
	return retval;
}

/***************************
 *
 * ETAL_receiverDestroyAllDatapathsForReceiver
 *
 **************************/
/*!
 * \brief		Destroys all the Datapaths associated to a Receiver
 * \details		The function invokes #ETAL_receiverDatapathDestroy on all
 * 				the non-empty Datapaths registered for the *hReceiver*.
 * 				A Datapath is non-empty if it has a non-NULL sink
 * 				function (see #ETAL_receiverDatapathGetSink).
 * \param[in]	hReceiver - the Receiver handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_receiverDestroyAllDatapathsForReceiver(ETAL_HANDLE hReceiver)
{
	etalDatapathTy *dpath;
	ETAL_HINDEX datapath_index;
	ETAL_HINDEX receiver_index;
	ETAL_HANDLE hDatapath;
	tU32 i;

	receiver_index = ETAL_handleReceiverGetIndex(hReceiver);
	if (receiver_index == ETAL_INVALID_HINDEX)
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}

	for (i = 0; i < ETAL_MAX_DATAPATH_PER_RECEIVER; i++)
	{
		datapath_index = (ETAL_HINDEX)i;
		dpath = ETAL_receiverGetDatapathFromIndex(receiver_index, datapath_index);
		if (dpath == NULL)
		{
			goto exit;
		}
		if (dpath->m_sink.m_CbProcessBlock != NULL)
		{
			/*
			 * an error return implies a programming error and is under ASSERT
			 */
			hDatapath = ETAL_handleMakeDatapath(receiver_index, datapath_index);
			ETAL_receiverDatapathDestroy(hDatapath);
		}
	}

exit:
	return;
}


