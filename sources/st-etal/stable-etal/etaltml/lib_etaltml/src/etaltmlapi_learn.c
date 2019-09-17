//!
//!  \file      etaltmlapi_learn.c
//!  \brief     <i><b> ETALTML API layer </b></i>
//!  \details   Interface functions for the ETAL TML user application
//!  \author    Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

/***************************
 *
 * Prototypes
 *
 **************************/
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_LEARN)
static ETAL_STATUS ETALTML_learn_start(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, tU32 step, tU32 nbOfFreq, etalLearnReportingModeStatusTy mode, EtalLearnFrequencyTy* freqList);
static ETAL_STATUS ETALTML_LearnStop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode);
static tVoid ETALTML_LearnStartTask(tVoid);
static tVoid ETALTML_LearnStopTask(tVoid);
static tVoid ETALTML_LearnThreadEntry(tPVoid dummy);
static tVoid ETALTML_LearnMainLoop(ETAL_HANDLE hReceiver);
static tVoid ETALTML_BuildLearnStatus(ETAL_HANDLE hReceiver, etalLearnConfigTy *learnCfgp);
static tVoid ETALTML_SortFrequency(etalLearnConfigTy *learnCfgp, EtalLearnFrequencyTy *pl_newFrequency);
static tVoid ETALTML_ReportLearnInfo(ETAL_HANDLE hReceiver, EtalLearnStatusTy* learnStatusp);
static tVoid ETALTML_StopLearnAndSendEvent(ETAL_HANDLE hReceiver, etalLearnConfigTy *learnCfgp, etalLearnEventStatusTy status);
static tVoid ETALTML_LearnDeregisterNotification(ETAL_HANDLE hReceiver);
static tVoid ETALTML_LearnIntCbFunc(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);
static tSInt ETALTML_CheckTuneReturn(ETAL_HANDLE hReceiver, ETAL_STATUS retval);
static ETAL_STATUS ETALTML_CheckLearnStartParameters(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, tU32 step, tU32 nbOfFreq, etalLearnReportingModeStatusTy mode, EtalLearnFrequencyTy* freqList);

#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_LearnWaitingTask(tSInt stacd, tPVoid thread_param);
#else
static tVoid ETALTML_LearnWaitingTask (tPVoid thread_param);
#endif //CONFIG_HOST_OS_TKERNEL

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
static tVoid ETALTML_LearnIntCbFuncHD(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO


/***************************
 *
 * Local variables
 *
 **************************/
static OSAL_tEventHandle learn_TaskEventHandler;
static OSAL_tThreadID ETALTML_learn_ThreadId;

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
static tBool v_serviceFollowingHasBeenDisabled = false;
#endif //CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

/***************************
 *
 * ETALTML_CheckTuneReturn
 *
 **************************/
static tSInt ETALTML_CheckTuneReturn(ETAL_HANDLE hReceiver, ETAL_STATUS retval)
{
    tSInt ret;

	if ((retval == ETAL_RET_SUCCESS) ||
		(retval == ETAL_RET_NO_DATA))
	{
		// Retune requested, success or no sync 
		// consider ok
	    ret = OSAL_OK;
	}
	else
	{
        ret = OSAL_ERROR;
	}
	return ret;
}

/***************************
 *
 * ETALTML_CheckLearnStartParameters
 *
 **************************/
static ETAL_STATUS ETALTML_CheckLearnStartParameters(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, tU32 step, tU32 nbOfFreq, etalLearnReportingModeStatusTy mode, EtalLearnFrequencyTy* freqList)
{
    etalFrequencyBandInfoTy bandInfo;
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    if((nbOfFreq > ETAL_LEARN_MAX_NB_FREQ) || (nbOfFreq == 0))
    {
        ret = ETAL_RET_PARAMETER_ERR;
    }
    else
    {
        /*
         * validate AMFMStep
         * mainly done to avoid parameters passed in Hz instead of KHz...
         */
        switch (ETAL_receiverGetStandard(hReceiver))
        {
            case ETAL_BCAST_STD_DAB:
                if(ETAL_receiverGetBandInfo(hReceiver, &bandInfo) == OSAL_OK)
                {
                    if(!(((bandInfo.band == ETAL_BAND_DAB3) || (bandInfo.band == ETAL_BAND_DABL)) &&
                       (bandInfo.band == bandIndex)))
                    {
                        ret = ETAL_RET_PARAMETER_ERR;
                    }
                }
                else
                {
                    ret = ETAL_RET_PARAMETER_ERR;
                }
                break;

            case ETAL_BCAST_STD_FM:
            case ETAL_BCAST_STD_HD_FM:
                if (step > ETAL_SEEK_MAX_FM_STEP)
                {
                    ret = ETAL_RET_PARAMETER_ERR;
                }
                break;

            case ETAL_BCAST_STD_AM:
            case ETAL_BCAST_STD_HD_AM:
                if (step > ETAL_SEEK_MAX_AM_STEP)
                {
                    ret = ETAL_RET_PARAMETER_ERR;
                }
                break;

            default:
                ret = ETAL_RET_PARAMETER_ERR;
                break;
        }

        if(freqList == NULL)
        {
            ret = ETAL_RET_PARAMETER_ERR;
        }
        else if((mode != normalMode) && (mode != sortMode))
        {
            ret = ETAL_RET_PARAMETER_ERR;
        }
    }
    return ret;
}


/***************************
 *
 * ETALTML_LearnIntCbFunc
 *
 **************************/
static tVoid ETALTML_LearnIntCbFunc(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
    ETAL_HANDLE hReceiver;
    EtalSeekStatus *pl_autoSeekStatus = (EtalSeekStatus *) param;
    etalReceiverStatusTy *recvp = NULL;
    etalLearnConfigTy *learnCfgp = NULL;
    EtalLearnStatusTy *learnStatusp = NULL;
    EtalLearnFrequencyTy newFrequency;

	if (ETAL_handleIsReceiver(hGeneric))
	{
	    hReceiver = hGeneric;

		// we should not lock the receiver
		// because receiver is already assumed lock by caller

	
		    recvp = ETAL_receiverGet(hReceiver);
		    learnCfgp = &(recvp->learnCfg);
		    learnStatusp = &(learnCfgp->learnStatus);

		    if (pl_autoSeekStatus != NULL)
		    {
		        OSAL_pvMemorySet((tVoid *)learnStatusp, 0x00, sizeof(EtalScanStatusTy));

		        switch(pl_autoSeekStatus->m_status)
		        {
		            case ETAL_SEEK_RESULT:
		                if(pl_autoSeekStatus->m_frequencyFound == FALSE)
		                {
		                    if(pl_autoSeekStatus->m_fullCycleReached == TRUE)
		                    {
		                        /* Deregister notification */
		                        ETALTML_LearnDeregisterNotification(hReceiver);

		                        /* Change learn state */
		                        learnCfgp->state = ETALTML_LEARN_FINISHED;
		                    }
		                    else
		                    {
		                        /* Seek is ongoing */
		                    }
		                }
		                else
		                {
		                    newFrequency.m_frequency = pl_autoSeekStatus->m_frequency;

		                    if((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM))
		                    {
		                        newFrequency.m_fieldStrength  = pl_autoSeekStatus->m_quality.EtalQualityEntries.hd.m_analogQualityEntries.m_BBFieldStrength;
		                    }
		                    else
		                    {
		                        newFrequency.m_fieldStrength  = pl_autoSeekStatus->m_quality.EtalQualityEntries.amfm.m_BBFieldStrength;
		                    }

		                    newFrequency.m_HDServiceFound = FALSE;
		                    newFrequency.m_ChannelID      = 0; // init the channel ID to 0, should not be used since no service found

		                    if(learnCfgp->mode == sortMode)
		                    {
		                        /* Sort frequencies */
		                        ETALTML_SortFrequency(learnCfgp, &newFrequency);
		                    }
		                    else
		                    {
		                        if(learnCfgp->nbOfFreq < learnCfgp->maxNbOfFrequency)
		                        {
		                            /* add a new element */
		                            learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq] = newFrequency;
		                            learnCfgp->nbOfFreq++;
		                        }
		                    }

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		                    /* Register HD tune notification */
		                    if((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM))
		                    {
		                        if (ETAL_intCbRegister(callAtHDTuneFrequency, ETALTML_LearnIntCbFuncHD, hReceiver, ETAL_INTCB_CONTEXT_UNUSED) != ETAL_RET_SUCCESS)
		                        {
		                            /* Deregister notification */
		                            ETALTML_LearnDeregisterNotification(hReceiver);

		                            /* Change learn state */
		                            learnCfgp->state = ETALTML_LEARN_ERROR;
		                        }
		                    }
		                    else
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		                    {
		                        if((pl_autoSeekStatus->m_fullCycleReached == TRUE) ||
		                           ((learnCfgp->mode == normalMode) && (learnCfgp->nbOfFreq == learnCfgp->maxNbOfFrequency)))
		                        {
		                            /* Deregister notification */
		                            ETALTML_LearnDeregisterNotification(hReceiver);

		                            /* Change learn state */
		                            learnCfgp->state = ETALTML_LEARN_FINISHED;
		                        }
		                        else
		                        {
		                            /* Change learn state */
		                            learnCfgp->state = ETALTML_LEARN_CONTINUE;
		                        }
		                    }
		                }
	                break;

	            case ETAL_SEEK_FINISHED:
	                break;

	            default:
	                /* Deregister notification */
	                ETALTML_LearnDeregisterNotification(hReceiver);

	                /* Change learn state */
	                learnCfgp->state = ETALTML_LEARN_ERROR;
	                break;
		        }
 	   	}
		else
		{
			// pl_autoSeekStatus is null
			ASSERT_ON_DEBUGGING(0);
		}
	}
	else
	{
		// is receiver error
	    ASSERT_ON_DEBUGGING(0);
	}
	return;
}

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
static tVoid ETALTML_LearnIntCbFuncHD(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
    ETAL_HANDLE hReceiver;
    EtalTuneStatus *pl_TuneStatus = (EtalTuneStatus *) param;
    etalReceiverStatusTy *recvp = NULL;
    etalLearnConfigTy *learnCfgp = NULL;
    etalFrequencyBandInfoTy bandInfo;

	if (ETAL_handleIsReceiver(hGeneric))
	{
        hReceiver = hGeneric;
        recvp = ETAL_receiverGet(hReceiver);
        learnCfgp = &(recvp->learnCfg);

        if (pl_TuneStatus != NULL)
        {
            if ((pl_TuneStatus->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC)
            {
                /* Deregister seek notification */
                if (ETAL_intCbDeregister(callAtHDTuneFrequency, ETALTML_LearnIntCbFuncHD, hReceiver) == ETAL_RET_SUCCESS)
                {
                    /* Change learn state */
                    learnCfgp->state = ETALTML_LEARN_GET_HD_SERVICE;
                }
                else
                {
                    /* Deregister notification */
                    ETALTML_LearnDeregisterNotification(hReceiver);

                    /* Change learn state */
                    learnCfgp->state = ETALTML_LEARN_ERROR;
                }
            }
            else
            {
                if (((pl_TuneStatus->m_sync & ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED) == ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED) ||
                    ((pl_TuneStatus->m_sync & ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE) == ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE) ||
                    ((pl_TuneStatus->m_sync & ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND) == ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND))
                {
                    if (ETAL_receiverGetBandInfo(hReceiver, &bandInfo) == OSAL_OK)
                    {
                        if((pl_TuneStatus->m_stopFrequency >= bandInfo.bandMax) ||
                           ((learnCfgp->mode == normalMode) && (learnCfgp->nbOfFreq == learnCfgp->maxNbOfFrequency)))
                        {
                            /* Deregister notification */
                            ETALTML_LearnDeregisterNotification(hReceiver);

                            /* Change learn state */
                            learnCfgp->state = ETALTML_LEARN_FINISHED;
                        }
                        else
                        {
                            /* Change learn state */
                            learnCfgp->state = ETALTML_LEARN_CONTINUE;
                        }
                    }
                    else
                    {
                        /* Deregister notification */
                        ETALTML_LearnDeregisterNotification(hReceiver);

                        /* Change learn state */
                        learnCfgp->state = ETALTML_LEARN_ERROR;
                    }
                }
            }
        }
	}
	else
	{
	    ASSERT_ON_DEBUGGING(0);
	}
	return;
}
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/***************************
 *
 * ETALTML_StopLearnAndSendEvent
 *
 **************************/
static tVoid ETALTML_StopLearnAndSendEvent(ETAL_HANDLE hReceiver, etalLearnConfigTy *learnCfgp, etalLearnEventStatusTy status)
{
    EtalLearnStatusTy *learnStatusp = &(learnCfgp->learnStatus);
    ETAL_STATUS ret;
    tU8 i;
    tBool learnTask_started = FALSE;
    ETAL_HANDLE hReceiver_tmp;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
    tU8 cmd[1];
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

    /* Set event status */
    learnStatusp->m_status = status;

    /* Build learn event */
    ETALTML_BuildLearnStatus(hReceiver, learnCfgp);

    if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialLearn))
    {
    	/* Indicate to ETAL that learn is active */
    	ETAL_receiverSetSpecial(hReceiver, cmdSpecialLearn, cmdActionStart);
    }

    if(ETALTML_seek_stop_internal(hReceiver, learnCfgp->terminationMode) != ETAL_RET_SUCCESS)
    {
    	learnStatusp->m_status = ETAL_LEARN_ERROR;
    }

    if(learnStatusp->m_status == ETAL_LEARN_FINISHED)
    {
        /* if the frequency list is sort by strength : tune on the strongest */
        /* if the frequency list is sort by value : tune on the first */
        /* if the no valid station is found : tune on the last tuned frequency */
		
        if(learnStatusp->m_nbOfFrequency != 0)
        {
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
            if((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM))
            {
                /* Reset HD FSM */
                cmd[0] = (tU8)HDRADIO_SEEK_STOP_SPECIAL;
                if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
                {
                    learnStatusp->m_status = ETAL_LEARN_ERROR;
                }
                else
                {
                    (LINT_IGNORE_RET)ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
                }
            }
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
            ret = ETAL_tuneReceiverInternal(hReceiver, learnStatusp->m_frequencyList[0].m_frequency, cmdTuneNormalResponse);
            if (ETALTML_CheckTuneReturn(hReceiver, ret) != OSAL_OK)
            {
                learnStatusp->m_status = ETAL_LEARN_ERROR;
            }
        }
    }
    else
    {
        /* Tune initial frequency if requested */
        if(learnCfgp->terminationMode == initialFrequency)
        {
            if ((learnCfgp->initialFrequency != ETAL_INVALID_FREQUENCY) &&
                    (ETAL_receiverGetFrequency(hReceiver) != learnCfgp->initialFrequency))
            {
                ret = ETAL_tuneReceiverInternal(hReceiver, learnCfgp->initialFrequency, cmdTuneNormalResponse);
                if (ETALTML_CheckTuneReturn(hReceiver, ret) != OSAL_OK)
                {
                    /* Stop learn procedure */
                    learnCfgp->learnStatus.m_status = ETAL_LEARN_ERROR;
                }
            }
            else
            {
                learnCfgp->learnStatus.m_status = ETAL_LEARN_ERROR;
            }
        }
    }

    /* Unmute the audio */
    if (ETAL_mute(hReceiver, FALSE) != ETAL_RET_SUCCESS)
    {
    	learnCfgp->learnStatus.m_status = ETAL_LEARN_ERROR;
    }

    for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
    {
        hReceiver_tmp = ETAL_handleMakeReceiver((ETAL_HINDEX)i);

        if (ETAL_receiverIsValidHandle(hReceiver_tmp))
        {
        	learnTask_started = ETAL_receiverGetInProgress(ETAL_receiverGet(hReceiver_tmp), cmdSpecialLearn);
        }
    }

	if(learnTask_started == FALSE)
	{
	    /* Stop the learn task */
	    ETALTML_LearnStopTask();
	}

    /* Indicate to ETAL that learn is stopped */
    ETAL_receiverSetSpecial(hReceiver, cmdSpecialLearn, cmdActionStop);

    /* Send event and notify */
    ETALTML_ReportLearnInfo(hReceiver, &(learnCfgp->learnStatus));

    // Disable external learn event
	ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalLearnRequestInProgress, cmdActionStop);
    return;
}

/***************************
 *
 * ETALTML_ReportLearnInfo
 *
 **************************/
static tVoid ETALTML_ReportLearnInfo(ETAL_HANDLE hReceiver, EtalLearnStatusTy* learnStatusp)
{
	learnStatusp->m_receiverHandle = hReceiver;
	learnStatusp->m_frequency = ETAL_receiverGetFrequency(hReceiver);

    /* Send ETAL_INFO_LEARN event */
    ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_LEARN, (tVoid *)learnStatusp,  sizeof(EtalLearnStatusTy));

    /* Internal callback : callAtEndOfLearn */
	// end of learn internal call back only if finished
	//
	if (learnStatusp->m_status == ETAL_LEARN_FINISHED)
	{
    	ETAL_intCbScheduleCallbacks(hReceiver, callAtEndOfLearn, (tVoid *)learnStatusp, sizeof(EtalLearnStatusTy));
	}

#ifdef DEBUG_LEARN
    if(learnStatusp->m_status == ETAL_LEARN_FINISHED)
    {
        if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialLearn))
        {
        	ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "Learn still in progress\n");
            ASSERT_ON_DEBUGGING(0);
        }

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
        if (ETAL_intCbIsRegistered(callAtHDTuneFrequency, ETALTML_LearnIntCbFuncHD, hReceiver) == TRUE)
        {
        	ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_LearnIntCbFuncHD not deregistered\n");
            ASSERT_ON_DEBUGGING(0);
        }
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

        if (ETAL_intCbIsRegistered(callAtEndOfSeek, ETALTML_LearnIntCbFunc, hReceiver) == TRUE)
        {
        	ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_LearnIntCbFunc not deregistered\n");
            ASSERT_ON_DEBUGGING(0);
        }

        if (ETAL_intCbIsRegistered(callAtSeekStatus, ETALTML_LearnIntCbFunc, hReceiver) == TRUE)
        {
        	ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_LearnIntCbFunc not deregistered\n");
            ASSERT_ON_DEBUGGING(0);
        }
    }
#endif // DEBUG_LEARN
    return;
}

/***************************
 *
 * ETALTML_BuildLearnStatus
 *
 **************************/
static tVoid ETALTML_BuildLearnStatus(ETAL_HANDLE hReceiver, etalLearnConfigTy *learnCfgp)
{
    EtalLearnStatusTy *learnStatusp = &(learnCfgp->learnStatus);
    tU32 i;

    if(learnCfgp->frequencyList != NULL)
    {
        learnStatusp->m_frequencyList  = learnCfgp->frequencyList;

        for(i = 0; i < learnCfgp->nbOfFreq; i++)
        {
            learnStatusp->m_frequencyList[i] = learnCfgp->frequencyListTmp[i];
        }
    }

    learnStatusp->m_nbOfFrequency = learnCfgp->nbOfFreq;
    learnStatusp->m_receiverHandle = hReceiver;

    return;
}

/***************************
 *
 * ETALTML_SortFrequency
 *
 **************************/
static tVoid ETALTML_SortFrequency(etalLearnConfigTy *learnCfgp, EtalLearnFrequencyTy *pl_newFrequency)
{
    tBool arraySorted = FALSE;
    tU32 i, size;

    if(learnCfgp->nbOfFreq < learnCfgp->maxNbOfFrequency)
    {
        /* add a new element */
        learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq] = *pl_newFrequency;
        learnCfgp->nbOfFreq++;
    }
    else
    {
    	if(pl_newFrequency->m_fieldStrength > learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq-1].m_fieldStrength)
    	{
    		/* Replace the last element */
    		learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq-1] = *pl_newFrequency;
    	}
    }

    /* Sort the table */
    size = learnCfgp->nbOfFreq;

    while(arraySorted == FALSE)
    {
    	arraySorted = TRUE;
    	for(i = 0; i < (size - 1); i++)
    	{
    		if(learnCfgp->frequencyListTmp[i].m_fieldStrength < learnCfgp->frequencyListTmp[i+1].m_fieldStrength)
    		{
    			EtalLearnFrequencyTy temp = learnCfgp->frequencyListTmp[i];
    			learnCfgp->frequencyListTmp[i] = learnCfgp->frequencyListTmp[i+1];
    			learnCfgp->frequencyListTmp[i+1] = temp;
    			arraySorted = FALSE;
    		}
    	}
    	size--;
    }

    return;
}

/***************************
 *
 * ETALTML_learnTaskInit
 *
 **************************/
tSInt ETALTML_learnTaskInit (tVoid)
{
    OSAL_trThreadAttribute attr0;
    tSInt ret = OSAL_OK;

    if (OSAL_s32EventCreate((tCString)"learn_EventHandler", &learn_TaskEventHandler) == OSAL_OK)
    {
        /* Initialize the thread */
        attr0.szName = (tChar *)ETALTML_LEARN_THREAD_NAME;
        attr0.u32Priority = ETALTML_LEARN_THREAD_PRIORITY;
        attr0.s32StackSize = ETALTML_LEARN_STACK_SIZE;
        attr0.pfEntry = ETALTML_LearnWaitingTask;
        attr0.pvArg = NULL;

        if ((ETALTML_learn_ThreadId = OSAL_ThreadSpawn(&attr0)) == OSAL_ERROR)
        {
            ret = OSAL_ERROR_DEVICE_INIT;
        }
    }
    else
    {
        ret = OSAL_ERROR;
    }

    return ret;
}

/***************************
 *
 * ETALTML_learnTaskDeinit
 *
 **************************/
tSInt ETALTML_learnTaskDeinit (tVoid)
{
    tSInt ret = OSAL_OK;

	/* Stop the learn if it was in running process */
	ETALTML_LearnStopTask();

	/* Post the kill event */
	(LINT_IGNORE_RET) OSAL_s32EventPost (learn_TaskEventHandler,
                    ETALTML_LEARN_EVENT_KILL_FLAG,
                    OSAL_EN_EVENTMASK_OR);
	OSAL_s32ThreadWait(10);

	/* Delete the thread */
    if (OSAL_s32ThreadDelete(ETALTML_learn_ThreadId) != OSAL_OK)
    {
        ret = OSAL_ERROR;
    }
	else
	{
		ETALTML_learn_ThreadId = (OSAL_tThreadID)0;
	}

    return ret;
}

/***************************
 *
 * ETALTML_LearnStartTask
 *
 **************************/
static tVoid ETALTML_LearnStartTask(tVoid)
{
    /* This is done by posting the awake event */
    (LINT_IGNORE_RET) OSAL_s32EventPost (learn_TaskEventHandler,
                    ETALTML_LEARN_EVENT_WAKEUP_FLAG,
                    OSAL_EN_EVENTMASK_OR);
    return;
}

/***************************
 *
 * ETALTML_LearnStopTask
 *
 **************************/
static tVoid ETALTML_LearnStopTask(tVoid)
{
    /* this is done by posting the sleep event */
    (LINT_IGNORE_RET) OSAL_s32EventPost (learn_TaskEventHandler,
                    ETALTML_LEARN_EVENT_SLEEP_FLAG,
                    OSAL_EN_EVENTMASK_OR);
    return;
}

/***************************
 *
 * ETAL_learn_TaskClearEventFlag
 *
 **************************/
static tVoid ETAL_learn_TaskClearEventFlag(tU32 eventFlag)
{
	/* Clear old event if any (this can happen after a stop) */
    (LINT_IGNORE_RET) OSAL_s32EventPost(learn_TaskEventHandler,
                           ~eventFlag, OSAL_EN_EVENTMASK_AND);
    return;
}

/***************************
 *
 * ETALTML_LearnWaitingTask
 *
 **************************/
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_LearnWaitingTask(tSInt stacd, tPVoid thread_param)
#else
static tVoid ETALTML_LearnWaitingTask (tPVoid thread_param)
#endif //CONFIG_HOST_OS_TKERNEL
{
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETALTML_LEARN_NO_EVENT_FLAG;

    while (TRUE)
    {
        // Wait here the auto wake up event
    	ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_LearnWaitingTask : waiting for learn activation\n");

        vl_res = OSAL_s32EventWait (learn_TaskEventHandler,
                                     ETALTML_LEARN_EVENT_WAIT_WAKEUP_MASK,
                                     OSAL_EN_EVENTMASK_OR,
                                     OSAL_C_TIMEOUT_FOREVER,
                                     &vl_resEvent);

        ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_LearnWaitingTask : learn awaked, event = 0x%x, vl_res = %d\n",
                vl_resEvent, vl_res);

        if ((vl_resEvent == ETALTML_LEARN_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
        {
        	/* This is a timeout : no even received */
        }
        /* learn event trigger learn event processing
        * else normal handling ie if res = OSAL_ERROR => this is a timeout, so
        * normal processing or event return flag contains EVENT_CMD.. so normal processing */
        else if (ETALTML_LEARN_EVENT_KILL_FLAG == (ETALTML_LEARN_EVENT_KILL_FLAG & vl_resEvent))
        {
        	/* Kill the learn */
        	break;
        }
        /* This is a LEARN EVENT WAKE UP call the LEARN event handler */
        else   if (ETALTML_LEARN_EVENT_WAKEUP_FLAG == (ETALTML_LEARN_EVENT_WAKEUP_FLAG & vl_resEvent))
        {
        	/*
        	 * if the learn stop was called without a learn start, the SLEEP flag
        	 * is set and the ETALTML_LearnStartTask has no effect
        	 * so ensure the SLEEP flag is clear
        	 */
        	ETAL_learn_TaskClearEventFlag(ETALTML_LEARN_EVENT_ALL_FLAG);
        	ETALTML_LearnThreadEntry(thread_param);
        	/* Clear the received event */
        }
        else
        {
        	/* a non expected event : just clear all */
        	ETAL_learn_TaskClearEventFlag(ETALTML_LEARN_EVENT_ALL_FLAG);
        }
    }

    /* Close the events */
#ifndef OSAL_EVENT_SKIP_NAMES
    if (OSAL_s32EventClose(learn_TaskEventHandler) != OSAL_OK)
    {
        vl_res = OSAL_ERROR;
    }

	OSAL_s32ThreadWait(100);

    if (OSAL_s32EventDelete((tCString)"learn_EventHandler") != OSAL_OK)
    {
        vl_res = OSAL_ERROR;
    }
#else
    if (OSAL_s32EventFree(learn_TaskEventHandler) != OSAL_OK)
    {
        vl_res = OSAL_ERROR;
    }
#endif //#ifndef OSAL_EVENT_SKIP_NAMES

#ifdef CONFIG_HOST_OS_TKERNEL
	while(1) 
	{ 
		OSAL_s32ThreadWait(100);
	}
#else
	return;
#endif


}

/***************************
 *
 * ETALTML_LearnThreadEntry
 *
 **************************/
static tVoid ETALTML_LearnThreadEntry(tPVoid dummy)
{
    ETAL_HANDLE hReceiver;
    ETAL_HINDEX receiver_index;
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETALTML_LEARN_NO_EVENT_FLAG;
    tU32 i;

    ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_LearnThreadEntry :  learn awaked\n");

    while (TRUE)
    {
       vl_res = OSAL_s32EventWait(learn_TaskEventHandler,
               ETALTML_LEARN_EVENT_WAIT_MASK,
               OSAL_EN_EVENTMASK_OR,
               ETALTML_LEARN_EVENT_WAIT_TIME_MS,
               &vl_resEvent);

       ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_LearnThreadEntry :  awaked, event = 0x%x, vl_res = %d\n",
            vl_resEvent, vl_res);

        /* If this is for sleeping, leave this */
        if (ETALTML_LEARN_EVENT_SLEEP_FLAG == (vl_resEvent & ETALTML_LEARN_EVENT_SLEEP_FLAG))
        {
            ETAL_learn_TaskClearEventFlag(ETALTML_LEARN_EVENT_SLEEP_FLAG);
            break;
        }
        else if ((vl_resEvent == ETALTML_LEARN_NO_EVENT_FLAG) ||
            (vl_res == OSAL_ERROR_TIMEOUT_EXPIRED))
        {
            /* Receiver specific actions, lock one receiver at a time */
            for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
            {
                receiver_index = (ETAL_HINDEX)i;
                hReceiver = ETAL_handleMakeReceiver(receiver_index);

                if (ETAL_receiverIsValidHandle(hReceiver))
                {
                    if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
                    {
                        ASSERT_ON_DEBUGGING(0);
                        continue;
                    }

                    OSAL_s32SemaphoreWait(etalLearnSem, OSAL_C_TIMEOUT_FOREVER);
                    ETALTML_LearnMainLoop(hReceiver);
                    OSAL_s32SemaphorePost(etalLearnSem);

                    ETAL_receiverReleaseLock(hReceiver);
                }
            }
        }
        else
        {
            /* Manage LEARN events */
        }
    }

    return;
}

/***************************
 *
 * ETALTML_LearnMainLoop
 *
 **************************/
static tVoid ETALTML_LearnMainLoop(ETAL_HANDLE hReceiver)
{
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
    etalLearnConfigTy *learnCfgp = &(recvp->learnCfg);
	EtalLearnStatusTy *learnStatusp = &(learnCfgp->learnStatus);
    etalFrequencyBandInfoTy bandInfo;
    ETAL_STATUS ret;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	EtalServiceList serv_list;
    EtalLearnFrequencyTy newFrequency;
    tU32 i, j;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

    if (ETAL_receiverIsValidHandle(hReceiver))
    {
        switch(learnCfgp->state)
        {
            case ETALTML_LEARN_NULL:
                break;

            case ETALTML_LEARN_START:
                OSAL_pvMemorySet((tVoid *)&(learnCfgp->learnStatus), 0x00, sizeof(EtalLearnStatusTy));

				// Ticket #488788] [ETAL] Learn is muting the audio, leading to audio mute during bg learn
				// Learn should not mute the audio, 
				// this should be controlled by application
				// Because mute should be done only if learn is done on current active audio source
				// 
                /* Mute the audio */
                //if (ETAL_mute(hReceiver, TRUE) == ETAL_RET_SUCCESS)
                {
                    if (ETAL_receiverGetBandInfo(hReceiver, &bandInfo) == OSAL_OK)
                    {
                        /* Tune to the last frequency of the band to start seek on the first of the band */
                        ret = ETAL_tuneReceiverInternal(hReceiver, bandInfo.bandMin, cmdTuneNormalResponse);
                        if (ETALTML_CheckTuneReturn(hReceiver, ret) == OSAL_OK)
                        {
                            if(ETALTML_seek_start_internal(hReceiver, cmdDirectionUp, learnCfgp->step, cmdAudioMuted, dontSeekInSPS, TRUE) == ETAL_RET_SUCCESS)
                            {
                                /* Register seek notification */
                                if ((ETAL_intCbRegister(callAtEndOfSeek, ETALTML_LearnIntCbFunc, hReceiver, ETAL_INTCB_CONTEXT_UNUSED) == ETAL_RET_SUCCESS) &&
                                    (ETAL_intCbRegister(callAtSeekStatus, ETALTML_LearnIntCbFunc, hReceiver, ETAL_INTCB_CONTEXT_UNUSED) == ETAL_RET_SUCCESS))
                                {
                                    /* Build learn event */
                                    learnStatusp->m_status = ETAL_LEARN_STARTED;
                                    ETALTML_BuildLearnStatus(hReceiver, learnCfgp);

                                    /* Send event */
                                    ETALTML_ReportLearnInfo(hReceiver, learnStatusp);

                                    /* Change learn state */
                                    learnCfgp->state = ETALTML_LEARN_STARTED;
                                }
                                else
                                {
                                    /* Stop learn and send event */
                                    ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                                    /* Change learn state */
                                    learnCfgp->state = ETALTML_LEARN_NULL;
                                }
                            }
                            else
                            {
                                /* Stop learn and send event */
                                ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                                /* Change learn state */
                                learnCfgp->state = ETALTML_LEARN_NULL;
                            }
                        }
                        else
                        {
                            /* Stop learn and send event */
                            ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                            /* Change learn state */
                            learnCfgp->state = ETALTML_LEARN_NULL;
                        }
                    }
                    else
                    {
                        /* Stop learn and send event */
                        ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                        /* Change learn state */
                        learnCfgp->state = ETALTML_LEARN_NULL;
                    }
                }
                break;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
            case ETALTML_LEARN_GET_HD_SERVICE:
                /* get HD service */
                if(ETAL_get_service_list(hReceiver, ETAL_INVALID_UEID, 0, 0, &serv_list) == ETAL_RET_SUCCESS)
                {
                    if(serv_list.m_serviceCount != 0)
                    {
                        for(i = 0; i < learnCfgp->nbOfFreq; i++)
                        {
                             if(learnCfgp->frequencyListTmp[i].m_frequency == ETAL_receiverGetFrequency(hReceiver))
                             {
                                 learnCfgp->frequencyListTmp[i].m_HDServiceFound = TRUE;
                                 learnCfgp->frequencyListTmp[i].m_ChannelID = serv_list.m_service[0];
                                 break;
                             }
                        }

                        for(j = 1; j < serv_list.m_serviceCount; j++)
                        {
                            newFrequency = learnCfgp->frequencyListTmp[i];
                            newFrequency.m_HDServiceFound = TRUE;
                            newFrequency.m_ChannelID      = serv_list.m_service[j];

                            if(learnCfgp->mode == sortMode)
                            {
                                /* Sort frequencies */
                                ETALTML_SortFrequency(learnCfgp, &newFrequency);
                            }
                            else
                            {
                                if(learnCfgp->nbOfFreq < learnCfgp->maxNbOfFrequency)
                                {
                                    /* add a new element */
                                    learnCfgp->frequencyListTmp[learnCfgp->nbOfFreq] = newFrequency;
                                    learnCfgp->nbOfFreq++;
                                }
                            }
                        }
                    }

                    /* Change learn state */
                    learnCfgp->state = ETALTML_LEARN_CONTINUE;
                }
                else
                {
                    /* Stop seek and send event */
                    ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                    /* Change learn state */
                    learnCfgp->state = ETALTML_LEARN_NULL;
                }
                break;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

            case ETALTML_LEARN_STARTED:
                break;

            case ETALTML_LEARN_STOP:
            case ETALTML_LEARN_FINISHED:
                /* Stop learn and send event */
                ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_FINISHED);

                /* Change learn state */
                learnCfgp->state = ETALTML_LEARN_NULL;
                break;

            case ETALTML_LEARN_CONTINUE:
                /* Build learn event */
                learnStatusp->m_status = ETAL_LEARN_RESULT;
                ETALTML_BuildLearnStatus(hReceiver, learnCfgp);

                /* Send event and notify */
                ETALTML_ReportLearnInfo(hReceiver, &(learnCfgp->learnStatus));

                if(ETALTML_seek_continue_internal(hReceiver) == ETAL_RET_SUCCESS)
                {
                    /* Change learn state */
                    learnCfgp->state = ETALTML_LEARN_STARTED;
                }
                else
                {
                    /* Stop learn and send event */
                    ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                    /* Change learn state */
                    learnCfgp->state = ETALTML_LEARN_NULL;
                }
                break;

            case ETALTML_LEARN_ERROR:
                /* Stop seek and send event */
                ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                /* Change learn state */
                learnCfgp->state = ETALTML_LEARN_NULL;
                break;

            default:
                /* Stop seek and send event */
                ETALTML_StopLearnAndSendEvent(hReceiver, learnCfgp, ETAL_LEARN_ERROR);

                /* Change learn state */
                learnCfgp->state = ETALTML_LEARN_NULL;
                break;
        }
    }

    return;
}


/***************************
 *
 * ETALTML_LearnDeregisterNotification
 *
 **************************/
static tVoid ETALTML_LearnDeregisterNotification(ETAL_HANDLE hReceiver)
{
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	/* Deregister HD tune notification */
	if((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM))
	{
		(LINT_IGNORE_RET)ETAL_intCbDeregister(callAtHDTuneFrequency, &ETALTML_LearnIntCbFuncHD, hReceiver);
	}
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

    (LINT_IGNORE_RET)ETAL_intCbDeregister(callAtEndOfSeek, ETALTML_LearnIntCbFunc, hReceiver);
	(LINT_IGNORE_RET)ETAL_intCbDeregister(callAtSeekStatus, ETALTML_LearnIntCbFunc, hReceiver);
	return;
}

/***************************
 *
 * ETALTML_learn_start
 *
 **************************/
static ETAL_STATUS ETALTML_learn_start(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, tU32 step, tU32 nbOfFreq, etalLearnReportingModeStatusTy mode, EtalLearnFrequencyTy* freqList)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;
    tU8 i;
    tBool learnTask_started = FALSE;
    ETAL_HANDLE hReceiver_tmp;

    if (!ETAL_receiverIsValidHandle(hReceiver))
    {
        ret = ETAL_RET_INVALID_HANDLE;
    }
    else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency) ||
			 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessEstimation) ||
			 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessSwitching))
    {
        ret = ETAL_RET_IN_PROGRESS;
    }
    else if (!ETAL_receiverSupportsQuality(hReceiver))
    {
        ret = ETAL_RET_INVALID_RECEIVER;
    }
	else if (ETALTML_CheckLearnStartParameters(hReceiver, bandIndex, step, nbOfFreq, mode, freqList) != ETAL_RET_SUCCESS)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
    else
    {
		etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
		etalLearnConfigTy *learnCfgp = &(recvp->learnCfg);

		learnCfgp->initialFrequency  = ETAL_receiverGetFrequency(hReceiver);
		learnCfgp->step              = step;
		learnCfgp->maxNbOfFrequency  = nbOfFreq;
		learnCfgp->mode              = mode;
		learnCfgp->frequencyList     = freqList;
		learnCfgp->terminationMode   = lastFrequency;
		learnCfgp->nbOfFreq          = 0;
		learnCfgp->bandIndex         = bandIndex;

		learnCfgp->state = ETALTML_LEARN_START;

        for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
        {
            hReceiver_tmp = ETAL_handleMakeReceiver((ETAL_HINDEX)i);

            if (ETAL_receiverIsValidHandle(hReceiver_tmp))
            {
            	learnTask_started = ETAL_receiverGetInProgress(ETAL_receiverGet(hReceiver_tmp), cmdSpecialLearn);
            }
        }

		if(learnTask_started == FALSE)
		{
			/* activate the learn task */
			ETALTML_LearnStartTask();
		}

		/* Start learn procedure */
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialLearn, cmdActionStart);
    }
#else
    ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // #if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)

    return ret;
}

/***************************
 *
 * ETALTML_LearnStop
 *
 **************************/
static ETAL_STATUS ETALTML_LearnStop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    if (!ETAL_receiverIsValidHandle(hReceiver))
    {
        ret = ETAL_RET_INVALID_HANDLE;
    }
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
    else
    {
        etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
        etalLearnConfigTy *learnCfgp = &(recvp->learnCfg);

        learnCfgp->terminationMode = terminationMode;

        if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialLearn))
        {
        	/* Deregister notification */
        	ETALTML_LearnDeregisterNotification(hReceiver);

        	/* Change learn state */
        	learnCfgp->state = ETALTML_LEARN_STOP;
        }
    }
#else
    ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // #if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)

    return ret;
}

/***************************
 *
 * etaltml_learn_start
 *
 **************************/
ETAL_STATUS etaltml_learn_start(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, tU32 step, tU32 nbOfFreq, etalLearnReportingModeStatusTy mode, EtalLearnFrequencyTy* freqList)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_learn_start(rec: %d, banInd: %x, ste: %d, nbOfFre: %d, mod: %d)",
            hReceiver, bandIndex, step, nbOfFreq, mode);

	ret = ETAL_receiverGetLock(hReceiver);
	
    if (ret == ETAL_RET_SUCCESS)
    {
#ifdef DEBUG_LEARN
	    // Enable external seek event
        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStart);
#endif //DEBUG_LEARN

	    // Enable external learn event
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalLearnRequestInProgress, cmdActionStart);

        ret = ETALTML_learn_start(hReceiver, bandIndex, step, nbOfFreq, mode, freqList);

        if ((ret != ETAL_RET_IN_PROGRESS) && (ret != ETAL_RET_SUCCESS))
        {
        	ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalLearnRequestInProgress, cmdActionStop);
        }

        ETAL_receiverReleaseLock(hReceiver);

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
#endif //CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

    }
    else
    {
		// Error
    }

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_learn_start() = %s", ETAL_STATUS_toString(ret));
	
    return ret;
}

/***************************
 *
 * etaltml_learn_stop
 *
 **************************/
ETAL_STATUS etaltml_learn_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_learn_stop(rec: %d, terMod: %d)", hReceiver, terminationMode);

	// we need to lock receiver
	// to avoid any cross-activity
	ret = ETAL_receiverGetLock(hReceiver);
	
    if (ret == ETAL_RET_SUCCESS)
    {
	    ret = ETALTML_LearnStop(hReceiver, terminationMode);

#ifdef DEBUG_LEARN
	    // Disable external seek event
	    ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStop);
#endif //DEBUG_LEARN

        ETAL_receiverReleaseLock(hReceiver);

	    // Resume Service Following
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
#endif //CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
    }
	else
	{
		// lock error : nothing to do
	}
	
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_learn_stop() = %s", ETAL_STATUS_toString(ret));
    return ret;
}
#endif // CONFIG_ETAL_HAVE_ETALTML && CONFIG_ETALTML_HAVE_LEARN
