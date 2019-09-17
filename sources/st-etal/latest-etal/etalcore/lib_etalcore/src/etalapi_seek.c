//!
//!  \file 		etalapi_seek.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, automatic seek command
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

/***************************
 *
 * Local types
 *
 **************************/

/***************************
 *
 * Defines
 *
 **************************/

/******************************************************************************
 * Exported functions
 *****************************************************************************/
#if defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
static ETAL_STATUS ETAL_checkSeekStartManualParameters(ETAL_HANDLE hReceiver, tU32 step, tU32 *freq);
static ETAL_STATUS ETAL_checkSeekContinueManualParameters(ETAL_HANDLE hReceiver, tU32 *freq);
static ETAL_STATUS ETAL_checkSeekStopManualParameters(ETAL_HANDLE hReceiver, tU32 *freq);
static ETAL_STATUS ETAL_checkSeekGetStatusManualParameters(ETAL_HANDLE hReceiver, EtalSeekStatus *seekStatus);
#endif

/******************************************************************************
 * Variable
 *****************************************************************************/

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
static tBool v_serviceFollowingHasBeenDisabled = false;
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETAL_InitAutoSeekStatus
 *
 **************************/
tVoid ETAL_InitAutoSeekStatus(EtalBcastStandard standard, EtalSeekStatus* extAutoSeekStatus)
{
    extAutoSeekStatus->m_receiverHandle                                                   = ETAL_INVALID_HANDLE;
    extAutoSeekStatus->m_status                                                           = ETAL_SEEK_ERROR;
    extAutoSeekStatus->m_frequencyFound                                                   = FALSE;
    extAutoSeekStatus->m_fullCycleReached                                                 = FALSE;
    extAutoSeekStatus->m_frequency                                                        = ETAL_INVALID_FREQUENCY;
    extAutoSeekStatus->m_serviceId                                                        = ETAL_INVALID_PROG;

	ETAL_resetQualityContainer(standard, &(extAutoSeekStatus->m_quality));
    return;
}


/***************************
 *
 * ETAL_stop_and_send_event
 *
 **************************/
tVoid ETAL_stop_and_send_event(ETAL_HANDLE hReceiver, etalSeekConfigTy *seekCfgp, etalSeekStatusTy status)
{
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	ETAL_HANDLE hreceiver_main, hreceiver_secondary;
	tBool main_seek_started, secondary_seek_started;
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

	/* Set event status */
    seekCfgp->autoSeekStatus.m_status    = status;

    ETAL_receiverSetSpecial(hReceiver, cmdSpecialSeek, cmdActionStop);

    if (ETAL_intCbIsRegisteredPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver) == TRUE)
    {
        /* Deregister periodic callback */
        ETAL_intCbDeregisterPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver);
    }


#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	/* if this is a DAB receiver, we should check whether we should stop task or not. */
	if (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
	{
		// Stop DAB seek task only if there is no more DAB receiver active
    	hreceiver_main = ETAL_receiverSearchFromApplication(DABMW_MAIN_AUDIO_DAB_APP);
    	hreceiver_secondary = ETAL_receiverSearchFromApplication(DABMW_SECONDARY_AUDIO_DAB_APP);

    	if(hreceiver_main != ETAL_INVALID_HANDLE)
    	{
    		main_seek_started = ETAL_receiverIsSpecialInProgress(hreceiver_main, cmdSpecialSeek);
    	}
    	else
    	{
    		main_seek_started = FALSE;
    	}

    	if(hreceiver_secondary != ETAL_INVALID_HANDLE)
    	{
    		secondary_seek_started = ETAL_receiverIsSpecialInProgress(hreceiver_secondary, cmdSpecialSeek);
    	}
    	else
    	{
    		secondary_seek_started = FALSE;
    	}

		if(((main_seek_started == FALSE) && (hreceiver_secondary != ETAL_INVALID_HANDLE)) ||
		   ((hreceiver_main != ETAL_INVALID_HANDLE) && (secondary_seek_started == FALSE)) ||
		   ((main_seek_started == FALSE) && (secondary_seek_started == FALSE)))
		{
		    /* Stop the DAB seek task */
		    ETAL_DABSeekStopTask(ETAL_handleReceiverGetIndex(hReceiver));
		}
	}

    if (ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == TRUE)
    {
    	/* Deregister callback */
    	ETAL_intCbDeregister(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver);
    }
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR

    /* Send event and notify */
    ETAL_Report_Seek_Info(hReceiver, &(seekCfgp->autoSeekStatus));

    // Disable external seek event
	ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStop);
}


/***************************
 *
 * ETALTML_checkSetSeekThresholdsParameters
 *
 **************************/
ETAL_STATUS ETALTML_checkSetSeekThresholdsParameters(ETAL_HANDLE hReceiver, EtalSeekThreshold* seekThreshold)
{
    etalFrequencyBandInfoTy bandInfo;

    if (ETAL_receiverGetBandInfo(hReceiver, &bandInfo) != OSAL_OK)
    {
        return ETAL_RET_INVALID_RECEIVER;
    }
    else if (!ETAL_receiverIsValidHandle(hReceiver))
    {
        return ETAL_RET_INVALID_RECEIVER;
    }
    else if (((bandInfo.band & ETAL_BAND_AM_BIT) == 0) && ((bandInfo.band & ETAL_BAND_FM_BIT) == 0))
    {
        return ETAL_RET_INVALID_BCAST_STANDARD;
    }
    else
    {
        if(seekThreshold == NULL)
        {
            return ETAL_RET_PARAMETER_ERR;
        }
    }
    return ETAL_RET_SUCCESS;
}


/***************************
 *
 * ETAL_checkSeekStartParameters
 *
 **************************/
ETAL_STATUS ETAL_checkSeekStartParameters(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, etalSeekHdModeTy seekHDSPS)
{
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);

    /* Check pointer on Seek configuration */
    if(recvp == NULL)
    {
        return ETAL_RET_ERROR;
    }
    else
    {
         if (&(recvp->seekCfg) == NULL)
         {
             return ETAL_RET_ERROR;
         }
         else
         {
             if (&(recvp->seekCfg.autoSeekStatus) == NULL)
             {
                 return ETAL_RET_ERROR;
             }
         }
    }

    /* validate the receiver frequency */
    if (ETAL_receiverGetFrequency(hReceiver) == ETAL_INVALID_FREQUENCY)
    {
        return ETAL_RET_ERROR;
    }

    /* validate direction*/
    if((direction != cmdDirectionUp) && (direction != cmdDirectionDown))
    {
        return ETAL_RET_ERROR;
    }

    /* validate exitSeekAction*/
    if((exitSeekAction != cmdAudioUnmuted) && (exitSeekAction != cmdAudioMuted))
    {
        return ETAL_RET_ERROR;
    }

    /* validate seekHDSPS*/
    if((seekHDSPS != dontSeekInSPS) && (seekHDSPS != seekInSPS))
    {
        return ETAL_RET_ERROR;
    }

    /* validate frequency step mainly done to avoid parameters passed in Hz instead of KHz...  */
    if (step == ETAL_SEEK_STEP_UNDEFINED)
    {
        return ETAL_RET_SUCCESS;
    }
    switch (ETAL_receiverGetStandard(hReceiver))
    {
        case ETAL_BCAST_STD_DAB:
            /* ignore the parameter */
            break;

        case ETAL_BCAST_STD_FM:
        case ETAL_BCAST_STD_HD_FM:
            if (step > ETAL_SEEK_MAX_FM_STEP)
            {
                return ETAL_RET_ERROR;
            }
            break;

        case ETAL_BCAST_STD_AM:
        case ETAL_BCAST_STD_HD_AM:
            if (step > ETAL_SEEK_MAX_AM_STEP)
            {
                return ETAL_RET_ERROR;
            }
            break;

        default:
            return ETAL_RET_INVALID_BCAST_STANDARD;
    }
    return ETAL_RET_SUCCESS;
}


/***************************
 *
 * ETAL_checkSeekStopParameters
 *
 **************************/
ETAL_STATUS ETAL_checkSeekStopParameters(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
{
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);

    /* Check pointer on Seek configuration */
    if(recvp == NULL)
    {
        return ETAL_RET_ERROR;
    }
    else
    {
         if (&(recvp->seekCfg) == NULL)
         {
             return ETAL_RET_ERROR;
         }
         else
         {
             if (&(recvp->seekCfg.autoSeekStatus) == NULL)
             {
                 return ETAL_RET_ERROR;
             }
         }
    }

    /* validate terminationMode*/
    if((terminationMode != initialFrequency) && (terminationMode != lastFrequency))
    {
        return ETAL_RET_ERROR;
    }

    return ETAL_RET_SUCCESS;
}


/***************************
 *
 * ETAL_checkSeekContinueParameters
 *
 **************************/
ETAL_STATUS ETAL_checkSeekContinueParameters(ETAL_HANDLE hReceiver)
{
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);

    /* Check pointer on Seek configuration */
    if(recvp == NULL)
    {
        return ETAL_RET_ERROR;
    }
    else
    {
         if (&(recvp->seekCfg) == NULL)
         {
             return ETAL_RET_ERROR;
         }
         else
         {
             if (&(recvp->seekCfg.autoSeekStatus) == NULL)
             {
                 return ETAL_RET_ERROR;
             }
         }
    }

    return ETAL_RET_SUCCESS;
}


/***************************
 *
 * ETAL_Report_Seek_Info
 *
 **************************/
tVoid ETAL_Report_Seek_Info(ETAL_HANDLE hReceiver, EtalSeekStatus* extAutoSeekStatus)
{

	EtalTuneInfoInternal tuneStatusInternal;
	

    extAutoSeekStatus->m_receiverHandle      = hReceiver;
    extAutoSeekStatus->m_frequency           = ETAL_receiverGetFrequency(hReceiver);
    extAutoSeekStatus->m_quality.m_standard  = ETAL_receiverGetStandard(hReceiver);
    extAutoSeekStatus->m_quality.m_TimeStamp = OSAL_ClockGetElapsedTime();
    extAutoSeekStatus->m_quality.m_Context   = NULL;

    switch(extAutoSeekStatus->m_status)
    {
        case ETAL_SEEK_STARTED :
        case ETAL_SEEK_ERROR :
        case ETAL_SEEK_ERROR_ON_START :
        case ETAL_SEEK_ERROR_ON_STOP :
        	/* Send ETAL_INFO_SEEK event */
        	ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEEK, (tVoid *)extAutoSeekStatus,  sizeof(EtalSeekStatus));
        	break;

        case ETAL_SEEK_RESULT :
#ifdef DEBUG_SEEK
        	if(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
        	{
        		etalSeekConfigTy *seekCfgp = &(ETAL_receiverGet(hReceiver)->seekCfg);

        		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB SEEK INFO rec: %d, freq: %d, fst: %d, noiseFloor: %d, seekThr: %d, isNoiseFloorSet: %d, startFrequency: %d\n",
        		        hReceiver,
        		        extAutoSeekStatus->m_frequency,
						extAutoSeekStatus->m_quality.EtalQualityEntries.dab.m_RFFieldStrength,
						seekCfgp->noiseFloor,
						seekCfgp->seekThr,
						seekCfgp->isNoiseFloorSet,
						seekCfgp->startFrequency);
        	}
#endif

        	/* Send ETAL_INFO_SEEK event */
        	ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEEK, (tVoid *)extAutoSeekStatus,  sizeof(EtalSeekStatus));

            /* Internal callback : callAtSeekStatus */
            ETAL_intCbScheduleCallbacks(hReceiver, callAtSeekStatus, (tVoid *)extAutoSeekStatus, sizeof(EtalSeekStatus));
            break;

        case ETAL_SEEK_FINISHED :

#ifdef DEBUG_SEEK
        	if(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
        	{
        		etalSeekConfigTy *seekCfgp = &(ETAL_receiverGet(hReceiver)->seekCfg);

        		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB SEEK INFO rec: %d, freq: %d, fst: %d, noiseFloor: %d, seekThr: %d, isNoiseFloorSet: %d, startFrequency: %d\n",
        		        hReceiver,
        		        extAutoSeekStatus->m_frequency,
						extAutoSeekStatus->m_quality.EtalQualityEntries.dab.m_RFFieldStrength,
						seekCfgp->noiseFloor,
						seekCfgp->seekThr,
						seekCfgp->isNoiseFloorSet,
						seekCfgp->startFrequency);
        	}
#endif
        	/* Send ETAL_INFO_SEEK event */
        	ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEEK, (tVoid *)extAutoSeekStatus,  sizeof(EtalSeekStatus));

        	/* Internal callback : callAtEndOfSeek */
            ETAL_intCbScheduleCallbacks(hReceiver, callAtEndOfSeek, (tVoid *)extAutoSeekStatus, sizeof(EtalSeekStatus));

           	 // ADD-ON
           	 // We should keep sending a tune information, if it is external seek on-going,
             // so that anybody knows the new frequency
             // this avoids internal users to register on many different event : seek/tune/af/....
             /* Internal callback : callAtTuneFrequency */
             tuneStatusInternal.m_receiverHandle          = hReceiver;
             tuneStatusInternal.m_Frequency               = extAutoSeekStatus->m_frequency;
             tuneStatusInternal.m_syncInternal            = (tU32)((extAutoSeekStatus->m_frequencyFound) ? ETAL_TUNESTATUS_SYNCMASK_FOUND : 0);
             tuneStatusInternal.m_externalRequestInfo     = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
             ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));
			 

            if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeek))
            {
            	printf("Seek still in progress\n");
            	ASSERT_ON_DEBUGGING(0);
            }

            if (ETAL_intCbIsRegisteredPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver) == TRUE)
            {
            	printf("ETAL_SeekStatusPeriodicFuncInternal not deregistered\n");
            	ASSERT_ON_DEBUGGING(0);
            }

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
            if (ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == TRUE)
            {
            	printf("ETAL_SeekIntCbFuncDabSync not deregistered\n");
            	ASSERT_ON_DEBUGGING(0);
            }
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
            break;

        default:
            break;
    }
    return;
}

/***************************
 *
 * ETALTML_get_internal_seek_thresholds
 *
 **************************/
ETAL_STATUS ETALTML_get_internal_seek_thresholds(ETAL_HANDLE hReceiver, EtalSeekThreshold* threshold)
{
    etalReceiverStatusTy *recvp;
    recvp = ETAL_receiverGet(hReceiver);

    if(threshold != NULL)
    {
        OSAL_pvMemoryCopy((tVoid *)threshold, (tPVoid)&recvp->seekCfg.seekThreshold, sizeof(EtalSeekThreshold));
        return ETAL_RET_SUCCESS;
    }
    else
    {
        return ETAL_RET_ERROR;
    }
}
#endif //CONFIG_ETAL_SUPPORT_CMOST_STAR
/***************************
 *
 * etal_autoseek_start
 *
 **************************/
ETAL_STATUS etal_autoseek_start(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, etalSeekHdModeTy seekHDSPS, tBool updateStopFrequency)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_autoseek_start(rec: %d, dir: %d, ste: %d, exiSeeAct: %d, seeHDSPS: %d, updStoFre: %d)", hReceiver, direction, step, exitSeekAction, seekHDSPS, updateStopFrequency);

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
    if ((ret = ETAL_receiverGetLock(hReceiver)) == ETAL_RET_SUCCESS)
    {
    	// Disable Service Following if needed
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
    	if (true == DABWM_ServiceFollowing_ExtInt_IsEnable())
		{
			if(DABMW_ServiceFollowing_ExtInt_DisableSF() == OSAL_OK)
			{
				v_serviceFollowingHasBeenDisabled = true;
			}
			else
			{
				ret = ETAL_RET_ERROR;
			}
		}
#endif
		if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
		{
			ret = ETAL_RET_IN_PROGRESS;
		}
		else
		{
		    // Enable external seek event
			ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStart);

			ret = ETAL_seek_start_internal(hReceiver, direction, step, exitSeekAction, seekHDSPS, updateStopFrequency);
		}
		
		if ((ret != ETAL_RET_IN_PROGRESS) && (ret != ETAL_RET_SUCCESS))
		{
		    // Disable external seek event
			ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStop);
		}

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
		// in case of error : we should resume the SF
		// renable Service Following

		if (ETAL_RET_SUCCESS != ret)
		{
			if (true == v_serviceFollowingHasBeenDisabled)
			{
				if(DABMW_ServiceFollowing_ExtInt_ActivateSF() == OSAL_OK)
				{
					v_serviceFollowingHasBeenDisabled = false;
				}
				else
				{
					ret = ETAL_RET_ERROR;
				}
			}
		}
#endif

		ETAL_receiverReleaseLock(hReceiver);
    }
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_autoseek_start() = %s", ETAL_STATUS_toString(ret));
    return ret;
}

/***************************
 *
 * etal_autoseek_continue
 *
 **************************/
ETAL_STATUS etal_autoseek_continue(ETAL_HANDLE hReceiver)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_autoseek_continue(rec: %d)", hReceiver);

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
    if ((ret = ETAL_receiverGetLock(hReceiver)) == ETAL_RET_SUCCESS)
    {
        ret = ETAL_seek_continue_internal(hReceiver);
		ETAL_receiverReleaseLock(hReceiver);
    }
#endif //CONFIG_ETAL_SUPPORT_CMOST_STAR
   	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_autoseek_continue() = %s", ETAL_STATUS_toString(ret));
    return ret;
}

/***************************
 *
 * etal_autoseek_stop
 *
 **************************/
ETAL_STATUS etal_autoseek_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_autoseek_stop(rec: %d, terMod: %d)", hReceiver, terminationMode);

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
    if ((ret = ETAL_receiverGetLock(hReceiver)) == ETAL_RET_SUCCESS)
    {
    	ret = ETAL_seek_stop_internal(hReceiver, terminationMode);

		ETAL_receiverReleaseLock(hReceiver);

		// renable Service Following
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
		if (true == v_serviceFollowingHasBeenDisabled)
		{
			if(DABMW_ServiceFollowing_ExtInt_ActivateSF() == OSAL_OK)
			{
				v_serviceFollowingHasBeenDisabled = false;
			}
			else
			{
				ret = ETAL_RET_ERROR;
			}
		}
#endif
    }
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_autoseek_stop() = %s", ETAL_STATUS_toString(ret));
    return ret;
}

/***************************
 *
 * etal_set_autoseek_thresholds_value
 *
 **************************/
ETAL_STATUS etal_set_autoseek_thresholds_value(ETAL_HANDLE hReceiver, EtalSeekThreshold* seekThreshold)
{
	ETAL_STATUS ret = ETAL_RET_ERROR;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_set_autoseek_thresholds_value(rec: %d)", hReceiver);

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "SeeThr: (seeThrBBFieStr: %d, seeThrDet: %u, seeThrAdjCha: %d, seeThrMul: %u, seeThrSigNoiRat: %u, seeThrMpxNoi: %u,  seeThrCoCha: %u)",
            seekThreshold->SeekThresholdBBFieldStrength, seekThreshold->SeekThresholdDetune,
            seekThreshold->SeekThresholdAdjacentChannel, seekThreshold->SeekThresholdMultipath,
            seekThreshold->SeekThresholdSignalNoiseRatio, seekThreshold->SeekThresholdMpxNoise,
            seekThreshold->SeekThresholdCoChannel);

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
	if ((ret = ETALTML_checkSetSeekThresholdsParameters(hReceiver, seekThreshold)) == ETAL_RET_SUCCESS)
	{
		if (ETAL_cmdSetSeekThreshold_CMOST(hReceiver, seekThreshold) != OSAL_OK)
		{
		    ret = ETAL_RET_ERROR;
		}
	}
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_set_autoseek_thresholds_value() = %s", ETAL_STATUS_toString(ret));
    return ret;
}

#endif // defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

#if defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

/***************************
 *
 * ETAL_checkSeekStartManualParameters
 *
 **************************/
/*!
 * \brief       Checks the #etal_seek_start_manual parameters
 * \details     Performs the following checks:
 *              - the *hReceiver* is valid
 *              - it is tuned, to DAB/AF/FM/HD Broadcast Standard
 *              - it is not employed in a special operation (seek, scan, learn)
 *              - the *freq* parameter is non-NULL
 *              - for AM/FM/HD, the *step* parameter is within some
 *              hard-coded bounds (see #ETAL_SEEK_MAX_FM_STEP and
 *              #ETAL_SEEK_MAX_AM_STEP).
 * \param[in]   hReceiver - Receiver handle
 * \param[in]   step - seek step in kHz - ignored for DAB
 * \param[in]   freq - pointer to integer, expected to be non-NULL
 * \return      #ETAL_RET_INVALID_RECEIVER - formally invalid handle
 * \return      #ETAL_RET_IN_PROGRESS - *hReceiver* is already employed in a
 *                                      seek or similar operation
 * \return      #ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return      #ETAL_RET_PARAMETER_ERR
 * \return      #ETAL_RET_ERROR - *hReceiver* is not tuned
 * \return      #ETAL_RET_SUCCESS
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
 * etal_seek_start_manual
 *
 **************************/
/*!
 * \brief       Start a manual seek operation
 * \details     The function stores the seek parameters and performs the first
 *              seek step; subsequent seek steps must be performed
 *              through the #etal_seek_continue_manual API.
 * \param[in]   hReceiver - the Receiver handle; supports DAB, HDRadio, AM and FM
 * \param[in]   direction - the seek direction
 * \param[in]   step - the seek step in kHz; ignored for DAB
 * \param[out]  freq - pointer to location where the function stores the frequency
 *                     on which the tuner stopped at the end of the seek step
 * \return      #ETAL_RET_INVALID_HANDLE - formally invalid *hReceiver*
 * \return      #ETAL_RET_INVALID_RECEIVER - *hReceiver* not configured
 * \return      #ETAL_RET_PARAMETER_ERR - NULL *pFrequency*
 * \return      #ETAL_RET_IN_PROGRESS - *hReceiver* is already employed in a
 *                                      seek or similar operation
 * \return      #ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return      #ETAL_RET_NOT_IMPLEMENTED - ETAL built without CMOST support
 * \return      #ETAL_RET_NO_DATA - tune operation successful but no signal detected
 *                                  on the *Frequency*; may be returned only for DAB or HDRadio
 *                                  *hReceiver*
 * \return      #ETAL_RET_ERROR - communication error with the device
 * \return      #ETAL_RET_SUCCESS
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

            // Enable external seek event
            ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStart);

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
                            tuneStatusInternal.m_receiverHandle      = hReceiver;
                            tuneStatusInternal.m_Frequency           = *freq;
                            tuneStatusInternal.m_serviceId           = ETAL_INVALID_PROG;
                            tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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
                    tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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
                        tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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
 * ETAL_checkSeekContinueManualParameters
 *
 **************************/
/*!
 * \brief       Checks the #etal_seek_continue_manual parameters
 * \details     Performs the following checks:
 *              - the *hReceiver* is valid and currently employed in a manual seek
 *              operation
 *              - the *freq* parameter is non-NULL
 * \details     For HDRadio receivers the first condition is not checked
 *              because the seek algo may jump to the next SPS on the same
 *              frequency. In this case no command is issued to the CMOST and
 *              thus the #cmdSpecialManualSeek is not set.
 * \param[in]   hReceiver - Receiver handle
 * \param[in]   freq - pointer to integer, expected to be non-NULL
 * \return      #ETAL_RET_INVALID_RECEIVER - formally invalid Receiver handle, or
 *                                           Receiver not currently performing a manual seek
 * \return      #ETAL_RET_PARAMETER_ERR
 * \return      #ETAL_RET_SUCCESS
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
                                tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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
                        tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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
                            tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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
 * ETAL_checkSeekStopManualParameters
 *
 **************************/
/*!
 * \brief       Checks the #etal_seek_stop_manual parameters
 * \details     Performs the following checks:
 *              - the *hReceiver* is valid and currently employed in a manual seek
 *              operation on DAB/AM/FM/HD Broadcast Standard
 *              - the *freq* parameter is non-NULL
 * \param[in]   hReceiver - Receiver handle
 * \param[in]   freq - pointer to integer, expected to be non-NULL
 * \return      #ETAL_RET_INVALID_RECEIVER - formally invalid handle, or *hReceiver*
 *                                           not employed in manual seek operation
 * \return      #ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return      #ETAL_RET_PARAMETER_ERR
 * \return      #ETAL_RET_SUCCESS
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
                        tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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
                    tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalSeekRequestInProgress);
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

            ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStop);
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
 * ETAL_checkSeekGetStatusManualParameters
 *
 **************************/
/*!
 * \brief       Checks the #etal_seek_get_status_manual parameters
 * \details     Performs the following checks:
 *              - the *hReceiver* is valid and currently employed in a manual seek
 *              operation on DAB/AM/FM/HD Broadcast Standard
 *              - the *hReceiver* supports quality measures
 *              - the *seekStatus* parameter is non-NULL
 * \param[in]   hReceiver - Receiver handle
 * \param[in]   seekStatus - pointer to integer, expected to be non-NULL
 * \return      #ETAL_RET_INVALID_RECEIVER - formally invalid handle, or *hReceiver*
 *                                           does not support quality measure
 * \return      #ETAL_RET_INVALID_BCAST_STANDARD - *hReceiver* not DAB, AM, FM or HD
 * \return      #ETAL_RET_PARAMETER_ERR
 * \return      #ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_checkSeekGetStatusManualParameters(ETAL_HANDLE hReceiver, EtalSeekStatus *seekStatus)
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
        ret = ETAL_checkSeekGetStatusManualParameters(hReceiver, seekStatus);
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


