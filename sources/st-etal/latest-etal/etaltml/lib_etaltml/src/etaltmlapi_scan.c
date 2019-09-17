//!
//!  \file 		etaltmlapi_scan.c
//!  \brief 	<i><b> ETALTML scan functions  </b></i>
//!  \details   ETALTML scan
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_SCAN) && defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)

/***************************
 *
 * Prototypes
 *
 **************************/
static ETAL_STATUS ETALTML_scan_start(ETAL_HANDLE hReceiver, tU32 audioPlayTime, etalSeekDirectionTy direction, tU32 step);
static ETAL_STATUS ETALTML_scan_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode);
static tVoid ETALTML_ScanIntCbFunc(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);
static tVoid ETALTML_ScanStatusPeriodicFunc(ETAL_HANDLE hGeneric);
static tVoid ETALTML_ScanDeregisterNotification(ETAL_HANDLE hReceiver);
static tVoid ETALTML_ReportScanInfo(ETAL_HANDLE hReceiver, EtalScanStatusTy* scanStatusp);
static tVoid ETALTML_ScanStartTask(tVoid);
static tVoid ETALTML_ScanStopTask(tVoid);
static tVoid ETALTML_ScanThreadEntry(tPVoid dummy);
static tVoid ETALTML_scanMainLoop(ETAL_HANDLE hReceiver);
static tVoid ETALTML_StopScanAndSendEvent(ETAL_HANDLE hReceiver, etalScanConfigTy *scanCfgp, etalScanEventStatusTy status);

#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_ScanWaitingTask(tSInt stacd, tPVoid thread_param);
#else
static tVoid ETALTML_ScanWaitingTask(tPVoid thread_param);
#endif //CONFIG_HOST_OS_TKERNEL

/***************************
 *
 * Local variables
 *
 **************************/
static OSAL_tEventHandle scan_TaskEventHandler;
static OSAL_tThreadID ETALTML_scan_ThreadId;

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
static tBool v_serviceFollowingHasBeenDisabled = false;
#endif

/***************************
 *
 * ETALTML_scanTaskInit
 *
 **************************/
tSInt ETALTML_scanTaskInit (tVoid)
{
    OSAL_trThreadAttribute attr0;
    tSInt ret = OSAL_OK;

    if (OSAL_s32EventCreate((tCString)"scan_EventHandler", &scan_TaskEventHandler) == OSAL_OK)
    {
        /* Initialize the thread */
        attr0.szName = (tChar *)ETALTML_SCAN_THREAD_NAME;
        attr0.u32Priority = ETALTML_SCAN_THREAD_PRIORITY;
        attr0.s32StackSize = ETALTML_SCAN_STACK_SIZE;
        attr0.pfEntry = ETALTML_ScanWaitingTask;
        attr0.pvArg = NULL;
        if ((ETALTML_scan_ThreadId = OSAL_ThreadSpawn(&attr0)) == OSAL_ERROR)
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
* ETALTML_scanTaskDeinit
*
**************************/
tSInt ETALTML_scanTaskDeinit (tVoid)
{
    tSInt ret = OSAL_OK;

    /* Stop the scan if it was in running process */
    ETALTML_ScanStopTask();

    /* Post the kill event */
    (LINT_IGNORE_RET) OSAL_s32EventPost (scan_TaskEventHandler,
                    ETALTML_SCAN_EVENT_KILL_FLAG,
                    OSAL_EN_EVENTMASK_OR);
    OSAL_s32ThreadWait(10);

    /* Delete the thread */
    if (OSAL_s32ThreadDelete(ETALTML_scan_ThreadId) != OSAL_OK)
    {
        ret = OSAL_ERROR;
    }
    else
    {
        ETALTML_scan_ThreadId = (OSAL_tThreadID)0;
    }

    return ret;
}

/***************************
*
* ETALTML_ScanStartTask
*
**************************/
static tVoid ETALTML_ScanStartTask(tVoid)
{
    /* This is done by posting the awake event */
    (LINT_IGNORE_RET) OSAL_s32EventPost (scan_TaskEventHandler,
                    ETALTML_SCAN_EVENT_WAKEUP_FLAG,
                    OSAL_EN_EVENTMASK_OR);
    return;
}

/***************************
*
* ETALTML_ScanStopTask
*
**************************/
static tVoid ETALTML_ScanStopTask(tVoid)
{
    /* this is done by posting the sleep event */
    (LINT_IGNORE_RET) OSAL_s32EventPost (scan_TaskEventHandler,
                    ETALTML_SCAN_EVENT_SLEEP_FLAG,
                    OSAL_EN_EVENTMASK_OR);
    return;
}

/***************************
*
* ETAL_scan_TaskClearEventFlag
*
**************************/
static tVoid ETAL_scan_TaskClearEventFlag(tU32 eventFlag)
{
    /* Clear old event if any (this can happen after a stop) */
    (LINT_IGNORE_RET) OSAL_s32EventPost(scan_TaskEventHandler,
                           ~eventFlag, OSAL_EN_EVENTMASK_AND);
    return;
}

/***************************
*
* ETALTML_ScanWaitingTask
*
**************************/
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_ScanWaitingTask(tSInt stacd, tPVoid thread_param)
#else
static tVoid ETALTML_ScanWaitingTask (tPVoid thread_param)
#endif //CONFIG_HOST_OS_TKERNEL
{
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETALTML_SCAN_NO_EVENT_FLAG;

    while (TRUE)
    {
        // Wait here the auto wake up event
    	ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_ScanWaitingTask : waiting for scan activation\n");

        vl_res = OSAL_s32EventWait (scan_TaskEventHandler,
                                     ETALTML_SCAN_EVENT_WAIT_WAKEUP_MASK,
                                     OSAL_EN_EVENTMASK_OR,
                                     OSAL_C_TIMEOUT_FOREVER,
                                     &vl_resEvent);

        ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_ScanWaitingTask : scac awaked, event = 0x%x, vl_res = %d\n",
                vl_resEvent, vl_res);

        if (OSAL_ERROR == vl_res)
		{
			// Event wait failure ==> break;			
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETALTML_ScanWaitingTask: wait error");
			break;
		}
		else if ((vl_resEvent == ETALTML_SCAN_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
        {
            /* This is a timeout : no even received */
        }
        /* scan event trigger scan event processing
        * else normal handling ie if res = OSAL_ERROR => this is a timeout, so
        * normal processing or event return flag contains EVENT_CMD.. so normal processing */
        else if (OSAL_OK == vl_res)
        {
	        if (ETALTML_SCAN_EVENT_KILL_FLAG == (ETALTML_SCAN_EVENT_KILL_FLAG & vl_resEvent))
	        {
	            /* Kill the scan */
	            break;
	        }
	        /* This is a SCAN EVENT WAKE UP call the SCAN event handler */
	        else   if (ETALTML_SCAN_EVENT_WAKEUP_FLAG == (ETALTML_SCAN_EVENT_WAKEUP_FLAG & vl_resEvent))
	        {
	            /*
	             * if the scan stop was called without a scan start, the SLEEP flag
	             * is set and the ETALTML_ScanStartTask has no effect
	             * so ensure the SLEEP flag is clear
	             */
	            ETAL_scan_TaskClearEventFlag(ETALTML_SCAN_EVENT_ALL_FLAG);
	            ETALTML_ScanThreadEntry(thread_param);
	            /* Clear the received event */
	        }
	        else
	        {
	            /* a non expected event : just clear all */
	            ETAL_scan_TaskClearEventFlag(ETALTML_SCAN_EVENT_ALL_FLAG);
	        }
    	}
		else
		{
			// nothing to do
		}
    }

    /* Close the events */
#ifndef OSAL_EVENT_SKIP_NAMES
    if (OSAL_s32EventClose(scan_TaskEventHandler) != OSAL_OK)
    {
        vl_res = OSAL_ERROR;
    }

	OSAL_s32ThreadWait(100);

    if (OSAL_s32EventDelete((tCString)"scan_EventHandler") != OSAL_OK)
    {
        vl_res = OSAL_ERROR;
    }
#else
    if (OSAL_s32EventFree(scan_TaskEventHandler) != OSAL_OK)
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
* ETALTML_ScanThreadEntry
*
**************************/
static tVoid ETALTML_ScanThreadEntry(tPVoid dummy)
{
    ETAL_HANDLE hReceiver;
    ETAL_HINDEX receiver_index;
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETALTML_SCAN_NO_EVENT_FLAG;
    tU32 i;

    ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_ScanThreadEntry :  scan awaked\n");

    while (TRUE)
    {
       vl_res = OSAL_s32EventWait(scan_TaskEventHandler,
               ETALTML_SCAN_EVENT_WAIT_MASK,
               OSAL_EN_EVENTMASK_OR,
               ETALTML_SCAN_EVENT_WAIT_TIME_MS,
               &vl_resEvent);

       ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_ScanThreadEntry :  awaked, event = 0x%x, vl_res = %d\n",
               vl_resEvent, vl_res);

        /* If this is for sleeping, leave this */
        if (OSAL_ERROR == vl_res)
		{
			// Event wait failure ==> break;			
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETALTML_ScanThreadEntry: wait error");
			break;
		}
		else if (ETALTML_SCAN_EVENT_SLEEP_FLAG == (vl_resEvent & ETALTML_SCAN_EVENT_SLEEP_FLAG))
        {
            ETAL_scan_TaskClearEventFlag(ETALTML_SCAN_EVENT_SLEEP_FLAG);
            break;
        }
		else 
		{
	        if ((vl_resEvent == ETALTML_SCAN_NO_EVENT_FLAG) ||
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

	                    /* Call scan main loop if it is registered */
	                    if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialScan))
	                    {
	                        OSAL_s32SemaphoreWait(etalScanSem, OSAL_C_TIMEOUT_FOREVER);
	                        ETALTML_scanMainLoop(hReceiver);
	                        OSAL_s32SemaphorePost(etalScanSem);
	                    }

	                    ETAL_receiverReleaseLock(hReceiver);
	                }
	            }
	        }
	        else
	        {
	            /* Manage SCAN events */
	        }
    	}
    }
	
    return;
}

/***************************
*
* ETALTML_scanMainLoop
*
**************************/
static tVoid ETALTML_scanMainLoop(ETAL_HANDLE hReceiver)
{
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
    etalScanConfigTy *scanCfgp = &(recvp->scanCfg);
    EtalScanStatusTy *scanStatusp = &(scanCfgp->scanStatus);
    etalSeekHdModeTy seekMode;

    if (ETAL_receiverIsValidHandle(hReceiver))
    {
        switch(scanCfgp->state)
        {
            case ETALTML_SCAN_NULL:
                break;

            case ETALTML_SCAN_START:
                OSAL_pvMemorySet((tVoid *)scanStatusp, 0x00, sizeof(EtalScanStatusTy));

                if((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM))
                {
                    seekMode = seekInSPS;
                }
                else
                {
                    seekMode = dontSeekInSPS;
                }

                if(ETAL_seek_start_internal(hReceiver, scanCfgp->direction, scanCfgp->step, cmdAudioUnmuted, seekMode, TRUE) == ETAL_RET_SUCCESS)
                {
                    if (ETAL_intCbRegister(callAtEndOfSeek, ETALTML_ScanIntCbFunc, hReceiver, ETAL_INTCB_CONTEXT_UNUSED) == ETAL_RET_SUCCESS)
                    {
                        /* Build and send ETAL_INFO_SCAN */
                        scanStatusp->m_status = ETAL_SCAN_STARTED;
                        ETALTML_ReportScanInfo(hReceiver, scanStatusp);

                        /* Change scan state */
                        scanCfgp->state = ETALTML_SCAN_WAIT;
                    }
                    else
                    {
                    
						ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_scanMainLoop : error in ETAL_intCbRegister");

						/* Stop scan and send event */
                        ETALTML_StopScanAndSendEvent(hReceiver, scanCfgp, ETAL_SCAN_ERROR);

                        /* Change scan state */
                        scanCfgp->state = ETALTML_SCAN_NULL;
                    }
                }
                else
                {
	                ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_scanMainLoop : error in ETAL_seek_start_internal direction %d, step %d, seekMode %d", 
						scanCfgp->direction, scanCfgp->step, seekMode);
					
                    /* Stop scan and send event */
                    ETALTML_StopScanAndSendEvent(hReceiver, scanCfgp, ETAL_SCAN_ERROR);

                    /* Change scan state */
                    scanCfgp->state = ETALTML_SCAN_NULL;
                }
                break;

            case ETALTML_SCAN_FINISHED:
            case ETALTML_SCAN_STOP:
                /* Stop scan and send event */
                ETALTML_StopScanAndSendEvent(hReceiver, scanCfgp, ETAL_SCAN_FINISHED);

                /* Change scan state */
                scanCfgp->state = ETALTML_SCAN_NULL;
                break;

            case ETALTML_SCAN_WAIT:
                break;

            case ETALTML_SCAN_ERROR:
                /* Stop scan and send event */
                ETALTML_StopScanAndSendEvent(hReceiver, scanCfgp, ETAL_SCAN_ERROR);

                /* Change scan state */
                scanCfgp->state = ETALTML_SCAN_NULL;
                break;

            default:
                /* Stop scan and send event */
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_scanMainLoop: invalid state %d\n", scanCfgp->state);
                ETALTML_StopScanAndSendEvent(hReceiver, scanCfgp, ETAL_SCAN_ERROR);

                /* Change scan state */
                scanCfgp->state = ETALTML_SCAN_NULL;
                break;
        }
    }
    return;
}


/***************************
 *
 * ETALTML_StopScanAndSendEvent
 *
 **************************/
static tVoid ETALTML_StopScanAndSendEvent(ETAL_HANDLE hReceiver, etalScanConfigTy *scanCfgp, etalScanEventStatusTy status)
{
    EtalScanStatusTy *scanStatusp = &(scanCfgp->scanStatus);
    ETAL_STATUS ret;

    /* Set event status */
    scanStatusp->m_status = status;

    if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialScan))
    {
        /* Indicate to ETAL that scan is active */
        ETAL_receiverSetSpecial(hReceiver, cmdSpecialScan, cmdActionStart);
    }

	ret = ETAL_seek_stop_internal(hReceiver, scanCfgp->terminationMode);

    if(ret != ETAL_RET_SUCCESS)
    {
    	ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_StopScanAndSendEvent: stop return error %d\n", ret);
        scanStatusp->m_status = ETAL_SCAN_ERROR;
    }

    /* Tune initial frequency if requested */
    if(scanCfgp->terminationMode == initialFrequency)
    {
        if ((scanCfgp->initialFrequency != ETAL_INVALID_FREQUENCY) &&
                (ETAL_receiverGetFrequency(hReceiver) != scanCfgp->initialFrequency))
        {
            ret = ETAL_tuneReceiverInternal(hReceiver, scanCfgp->initialFrequency, cmdTuneNormalResponse);

            if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
            {
                /* Stop scan procedure */
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_StopScanAndSendEvent: ETAL_tuneReceiverInternal return error %d\n", ret);
                scanStatusp->m_status = ETAL_SCAN_ERROR;
            }
        }
    }

    /* Unmute the audio */
	ret = ETAL_mute(hReceiver, FALSE);
    if (ret != ETAL_RET_SUCCESS)
    {
    	ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_StopScanAndSendEvent: ETAL_mute return error %d\n", ret);
        scanStatusp->m_status = ETAL_SCAN_ERROR;
    }

    /* Stop the scan task */
    ETALTML_ScanStopTask();

    /* Indicate to ETAL that scan is stopped */
    ETAL_receiverSetSpecial(hReceiver, cmdSpecialScan, cmdActionStop);

    /* Send event and notify */
    ETALTML_ReportScanInfo(hReceiver, scanStatusp);

    // Disable external scan event
	ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalScanRequestInProgress, cmdActionStop);

#ifdef DEBUG_SCAN
    if(scanStatusp->m_status == ETAL_SCAN_FINISHED)
    {
        if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialScan))
        {
            ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "Scan still in progress\n");
            ASSERT_ON_DEBUGGING(0);
        }

        if (ETAL_intCbIsRegisteredPeriodic(ETALTML_ScanStatusPeriodicFunc, hReceiver) == TRUE)
        {
            ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_ScanStatusPeriodicFunc not deregistered\n");
            ASSERT_ON_DEBUGGING(0);
        }

        if (ETAL_intCbIsRegistered(callAtEndOfSeek, ETALTML_ScanIntCbFunc, hReceiver) == TRUE)
        {
            ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_ScanIntCbFunc not deregistered\n");
            ASSERT_ON_DEBUGGING(0);
        }
    }
#endif // DEBUG_SCAN
    return;
}

/***************************
 *
 * ETALTML_ReportScanInfo
 *
 **************************/
static tVoid ETALTML_ReportScanInfo(ETAL_HANDLE hReceiver, EtalScanStatusTy* scanStatusp)
{
	EtalTuneInfoInternal tuneStatusInternal;

    scanStatusp->m_receiverHandle = hReceiver;
    scanStatusp->m_frequency      = ETAL_receiverGetFrequency(hReceiver);

    /* Send ETAL_INFO_SCAN event */
    ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SCAN, (tVoid *)scanStatusp,  sizeof(EtalScanStatusTy));

	
	// ADD-ON
	// We should keep sending a tune information, if it is external learn on-going,
	// so that anybody knows the new frequency
	// this avoids internal users to register on many different event : seek/tune/af/....
	/* Internal callback : callAtTuneFrequency */
	tuneStatusInternal.m_receiverHandle 		 = hReceiver;
	tuneStatusInternal.m_Frequency				 = scanStatusp->m_frequency;
	tuneStatusInternal.m_syncInternal			 = ETAL_TUNESTATUS_SYNCMASK_FOUND;
	tuneStatusInternal.m_externalRequestInfo	 = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalScanRequestInProgress);
	ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&tuneStatusInternal, sizeof(tuneStatusInternal));

    /* Internal callback : callAtEndOfScan */
    ETAL_intCbScheduleCallbacks(hReceiver, callAtEndOfScan, (tVoid *)scanStatusp, sizeof(EtalScanStatusTy));
    return;
}

/***************************
 *
 * ETALTML_ScanDeregisterNotification
 *
 **************************/
static tVoid ETALTML_ScanDeregisterNotification(ETAL_HANDLE hReceiver)
{
    (LINT_IGNORE_RET)ETAL_intCbDeregister(callAtEndOfSeek, ETALTML_ScanIntCbFunc, hReceiver);

    if (ETAL_intCbIsRegisteredPeriodic(ETALTML_ScanStatusPeriodicFunc, hReceiver) == TRUE)
    {
        /* Deregister periodic callback */
        ETAL_intCbDeregisterPeriodic(ETALTML_ScanStatusPeriodicFunc, hReceiver);
    }
    return;
}

tVoid ETALTML_ScanIntCbFunc(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
    ETAL_HANDLE hReceiver = hGeneric;
    EtalSeekStatus *pl_autoSeekStatus = (EtalSeekStatus *) param;
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
    etalScanConfigTy *scanCfgp = &(recvp->scanCfg);
    EtalScanStatusTy *scanStatusp = &(scanCfgp->scanStatus);

    if (ETAL_handleIsReceiver(hGeneric))
    {
        if (pl_autoSeekStatus != NULL)
        {
            scanStatusp->m_receiverHandle = hReceiver;
            scanStatusp->m_frequency      = pl_autoSeekStatus->m_frequency;

			ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_ScanIntCbFunc : hGeneric %d, status %d, freq = %d",  hGeneric, pl_autoSeekStatus->m_status, pl_autoSeekStatus->m_frequency);

            switch(pl_autoSeekStatus->m_status)
            {
                case ETAL_SEEK_FINISHED:
                    scanStatusp->m_frequencyFound = pl_autoSeekStatus->m_frequencyFound;
                    scanStatusp->m_fullCycleReached = pl_autoSeekStatus->m_fullCycleReached;

                    if(scanStatusp->m_frequencyFound == TRUE)
                    {
                        /* Register periodic callback during this time listen to the audio */
                        if(ETAL_intCbRegisterPeriodic(ETALTML_ScanStatusPeriodicFunc, hReceiver, scanCfgp->audioPlayTime) == ETAL_RET_SUCCESS)
                        {
                            /* Send ETAL_INFO_SCAN */
                            scanStatusp->m_status = ETAL_SCAN_RESULT;
                            ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SCAN, (tVoid *)scanStatusp,  sizeof(EtalScanStatusTy));

                            /* Change scan state */
                            scanCfgp->state = ETALTML_SCAN_WAIT;
                        }
                        else
                        {
                        
							ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_ScanIntCbFunc : error in ETAL_intCbRegisterPeriodic");
							
                            /* Deregister notification */
                            ETALTML_ScanDeregisterNotification(hReceiver);
	
                            /* Change scan state */
                            scanCfgp->state = ETALTML_SCAN_ERROR;
                        }
                    }
                    else
                    {
                        if(scanStatusp->m_fullCycleReached == TRUE)
                        {
                            /* Deregister notification */
                            ETALTML_ScanDeregisterNotification(hReceiver);

                            /* Change scan state */
                            scanCfgp->state = ETALTML_SCAN_STOP;
                        }
                    }
                    break;

                case ETAL_SEEK_RESULT:
                case ETAL_SEEK_STARTED:
                    break;

                default:
					ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETALTML_ScanIntCbFunc : unkown seek info %d ", pl_autoSeekStatus->m_status);
					
                    /* Deregister notification */
                    ETALTML_ScanDeregisterNotification(hReceiver);

                    /* Change scan state */
                    scanCfgp->state = ETALTML_SCAN_ERROR;
                    break;
            }
        }
    }
    else
    {
        ASSERT_ON_DEBUGGING(0);
    }
    return;
}

static tVoid ETALTML_ScanStatusPeriodicFunc(ETAL_HANDLE hGeneric)
{
    ETAL_HANDLE hReceiver = hGeneric;
    etalReceiverStatusTy *recvp;
    etalScanConfigTy *scanCfgp;
    EtalScanStatusTy *scanStatusp;
    etalSeekHdModeTy seekMode;
	tBool vl_releaseLock = FALSE;

	ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETALTML_ScanStatusPeriodicFunc : hGeneric %d,", hGeneric);

    if (ETAL_handleIsReceiver(hGeneric))
    {
    	/* get receiver lock */
		if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
		{
			ASSERT_ON_DEBUGGING(0);
			goto exit;
		}
		// we got the lock, mark it
		//
		vl_releaseLock = TRUE;

		//check receiver is still valid and has not been destroyed !
		if (false == ETAL_receiverIsValidHandle(hReceiver))
		{
			goto exit;
		}

		// Update the pointers now
		recvp = ETAL_receiverGet(hReceiver);
		scanCfgp = &(recvp->scanCfg);
		scanStatusp = &(scanCfgp->scanStatus);

        if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialScan))
        {
            if(scanStatusp->m_fullCycleReached == TRUE)
            {
                /* Deregister notification */
                ETALTML_ScanDeregisterNotification(hReceiver);

                /* Change scan state */
                scanCfgp->state = ETALTML_SCAN_STOP;
            }
            else
            {
                if((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM))
                {
                    seekMode = seekInSPS;
                }
                else
                {
                    seekMode = dontSeekInSPS;
                }

                if(ETAL_seek_start_internal(hReceiver, scanCfgp->direction, scanCfgp->step, cmdAudioUnmuted, seekMode, FALSE) != ETAL_RET_SUCCESS)
                {
                    /* Deregister notification */
                    ETALTML_ScanDeregisterNotification(hReceiver);

                    /* Change scan state */
                    scanCfgp->state = ETALTML_SCAN_ERROR;
                }
            }
        }

        if (ETAL_intCbIsRegisteredPeriodic(ETALTML_ScanStatusPeriodicFunc, hReceiver) == TRUE)
        {
            /* Deregister periodic callback */
            ETAL_intCbDeregisterPeriodic(ETALTML_ScanStatusPeriodicFunc, hReceiver);
        }
    }
    else
    {
        ASSERT_ON_DEBUGGING(0);
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
 * ETALTML_checkScanStartParameters
 *
 **************************/
static ETAL_STATUS ETALTML_checkScanStartParameters(ETAL_HANDLE hReceiver, tU32 audioPlayTime, etalSeekDirectionTy direction, tU32 step)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

	/* validate AMFMStep mainly done to avoid parameters passed in Hz instead of KHz... */
    switch (ETAL_receiverGetStandard(hReceiver))
    {
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
    }

    if(audioPlayTime > 60000)
    {
        ret = ETAL_RET_PARAMETER_ERR;
    }
    else if((direction != cmdDirectionUp) && (direction != cmdDirectionDown))
    {
        ret = ETAL_RET_PARAMETER_ERR;
    }

    return ret;
}

/***************************
 *
 * ETALTML_scan_start
 *
 **************************/
static ETAL_STATUS ETALTML_scan_start(ETAL_HANDLE hReceiver, tU32 audioPlayTime, etalSeekDirectionTy direction, tU32 step)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    if (!ETAL_receiverIsValidHandle(hReceiver))
    {
        ret = ETAL_RET_INVALID_HANDLE;
    }
    else if (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
    {
        ret = ETAL_RET_NOT_IMPLEMENTED;
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
    else if (ETALTML_checkScanStartParameters(hReceiver, audioPlayTime, direction, step) != ETAL_RET_SUCCESS)
    {
        ret = ETAL_RET_PARAMETER_ERR;
    }
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	else
	{
        tU32 device_list;
		etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
		etalScanConfigTy *scanCfgp = &(recvp->scanCfg);

		/* commandTune routes to both CMOST and DCOP, which is fine
		 * for HD because we treat it as FM */
		device_list = ETAL_cmdRoutingCheck(hReceiver, commandTune);

		if (ETAL_CMDROUTE_TO_STAR(device_list))
		{
			scanCfgp->initialFrequency    = ETAL_receiverGetFrequency(hReceiver);
			scanCfgp->step                = step;
			scanCfgp->terminationMode     = lastFrequency;
			scanCfgp->audioPlayTime       = audioPlayTime;
			scanCfgp->direction           = direction;
            scanCfgp->terminationMode     = initialFrequency;

            /* Start scan procedure */
            ETAL_receiverSetSpecial(hReceiver, cmdSpecialScan, cmdActionStart);

            /* Activate the scan task */
            ETALTML_ScanStartTask();

            /* Change the scan state */
            scanCfgp->state               = ETALTML_SCAN_START;
		}
		else
		{
	        ret = ETAL_RET_INVALID_RECEIVER;
		}
    }
#else
    ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // #if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
    return ret;
}

/***************************
 *
 * ETALTML_scan_stop
 *
 **************************/
static ETAL_STATUS ETALTML_scan_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    if (!ETAL_receiverIsValidHandle(hReceiver))
    {
        ret = ETAL_RET_INVALID_HANDLE;
    }
	else if (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
	{
		ret = ETAL_RET_NOT_IMPLEMENTED;
	}
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
    else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialScan))
    {
        tU32 device_list;
        etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
        etalScanConfigTy *scanCfgp = &(recvp->scanCfg);

        device_list = ETAL_cmdRoutingCheck(hReceiver, commandTune);

        if (ETAL_CMDROUTE_TO_STAR(device_list))
        {
            scanCfgp->terminationMode = terminationMode;

            /* Deregister notification */
            ETALTML_ScanDeregisterNotification(hReceiver);

            /* Change scan state */
            scanCfgp->state = ETALTML_SCAN_STOP;
        }
        else
        {
            ret = ETAL_RET_INVALID_RECEIVER;
        }
    }
#else
    ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR || CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
    return ret;
}


/***************************
 *
 * etaltml_scan_start
 *
 **************************/
ETAL_STATUS etaltml_scan_start(ETAL_HANDLE hReceiver, tU32 audioPlayTime, etalSeekDirectionTy direction, tU32 step)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_scan_start(rec: %d, audPlaTim: %d, dir: %d, ste: %d)", hReceiver, audioPlayTime, direction, step);

    if ((ret = ETAL_receiverGetLock(hReceiver)) == ETAL_RET_SUCCESS)
    {
#ifdef DEBUG_SCAN
	    // Enable external seek event
        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStart);
#endif //DEBUG_SCAN

	    // Enable external scan event
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalScanRequestInProgress, cmdActionStart);

        ret = ETALTML_scan_start(hReceiver, audioPlayTime, direction, step);

        if ((ret != ETAL_RET_IN_PROGRESS) && (ret != ETAL_RET_SUCCESS))
        {
        	ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalScanRequestInProgress, cmdActionStop);
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
#endif
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_scan_start() = %s", ETAL_STATUS_toString(ret));
    }
    else
    {
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_scan_start() = %s", ETAL_STATUS_toString(ret));
    }
    return ret;
}

/***************************
 *
 * etaltml_scan_stop
 *
 **************************/
ETAL_STATUS etaltml_scan_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_scan_stop(rec: %d, terMod: %d)", hReceiver, terminationMode);

    ret = ETALTML_scan_stop(hReceiver, terminationMode);

#ifdef DEBUG_SCAN
    // Disable external seek event
    ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalSeekRequestInProgress, cmdActionStop);
#endif //DEBUG_SCAN

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
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML->APP: etaltml_scan_stop() = %s", ETAL_STATUS_toString(ret));
    return ret;
}
#endif // CONFIG_ETAL_HAVE_ETALTML && CONFIG_ETALTML_HAVE_SCAN && CONFIG_ETAL_SUPPORT_CMOST_STAR
