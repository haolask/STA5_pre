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

/******************************************************************************
 * Variable
 *****************************************************************************/

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
static tBool v_serviceFollowingHasBeenDisabled = false;
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETALTML_InitAutoSeekStatus
 *
 **************************/
tVoid ETALTML_InitAutoSeekStatus(EtalBcastStandard standard, EtalSeekStatus* extAutoSeekStatus)
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
 * ETALTML_stop_and_send_event
 *
 **************************/
tVoid ETALTML_stop_and_send_event(ETAL_HANDLE hReceiver, etalSeekConfigTy *seekCfgp, etalSeekStatusTy status)
{
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	ETAL_HANDLE hreceiver_main, hreceiver_secondary;
	tBool main_seek_started, secondary_seek_started;
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

	/* Set event status */
    seekCfgp->autoSeekStatus.m_status    = status;

    ETAL_receiverSetSpecial(hReceiver, cmdSpecialSeek, cmdActionStop);

    if (ETAL_intCbIsRegisteredPeriodic(ETALTML_SeekStatusPeriodicFuncInternal, hReceiver) == TRUE)
    {
        /* Deregister periodic callback */
        ETAL_intCbDeregisterPeriodic(ETALTML_SeekStatusPeriodicFuncInternal, hReceiver);
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
		    ETAL_DABSeekStopTask();
		}
	}

    if (ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == TRUE)
    {
    	/* Deregister callback */
    	ETAL_intCbDeregister(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver);
    }
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR

#if defined (CONFIG_ETALTML_HAVE_RDS) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
    if (((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM))
        && (ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_PISeek)))
    {
      ETALTML_RDS_Strategy_SetFunctionStatus(hReceiver, RDS_FUNC_PISeek, FALSE);
    }
#endif

    /* Send event and notify */
    ETALTML_Report_Seek_Info(hReceiver, &(seekCfgp->autoSeekStatus));

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
 * ETALTML_checkSeekStartParameters
 *
 **************************/
ETAL_STATUS ETALTML_checkSeekStartParameters(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, etalSeekHdModeTy seekHDSPS)
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
 * ETALTML_checkSeekStopParameters
 *
 **************************/
ETAL_STATUS ETALTML_checkSeekStopParameters(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
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
 * ETALTML_checkSeekContinueParameters
 *
 **************************/
ETAL_STATUS ETALTML_checkSeekContinueParameters(ETAL_HANDLE hReceiver)
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
 * ETALTML_Report_Seek_Info
 *
 **************************/
tVoid ETALTML_Report_Seek_Info(ETAL_HANDLE hReceiver, EtalSeekStatus* extAutoSeekStatus)
{
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

        		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB SEEK INFO freq: %d, fst: %d, noiseFloor: %d, seekThr: %d, isNoiseFloorSet: %d, startFrequency: %d\n",
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

        		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB SEEK INFO freq: %d, fst: %d, noiseFloor: %d, seekThr: %d, isNoiseFloorSet: %d, startFrequency: %d\n",
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

            if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeek))
            {
            	printf("Seek still in progress\n");
            	ASSERT_ON_DEBUGGING(0);
            }

            if (ETAL_intCbIsRegisteredPeriodic(ETALTML_SeekStatusPeriodicFuncInternal, hReceiver) == TRUE)
            {
            	printf("ETALTML_SeekStatusPeriodicFuncInternal not deregistered\n");
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

			ret = ETALTML_seek_start_internal(hReceiver, direction, step, exitSeekAction, seekHDSPS, updateStopFrequency);
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
        ret = ETALTML_seek_continue_internal(hReceiver);
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
    	ret = ETALTML_seek_stop_internal(hReceiver, terminationMode);

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

