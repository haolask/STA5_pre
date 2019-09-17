//!
//!  \file 		etalseek_internal.c
//!  \brief 	<i><b> ETAL auto seek </b></i>
//!  \details   Auto seek based on internal CMOST auto seek
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"
#include "etalinternal.h"

/*!
 * \def     ETAL_DAB_SEEK_THREAD_NAME
 *          Thread name for the
 *          #ETAL_DABSeek_WaitingTask
 */
#define ETAL_DAB_SEEK_THREAD_NAME  "ETAL_DABSeek"

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

static OSAL_tEventHandle DABSeek_TaskEventHandler[ETAL_MAX_RECEIVERS];
static OSAL_tThreadID ETAL_DABSeek_ThreadId[ETAL_MAX_RECEIVERS];

static tSInt ETAL_DABSeekStartTask(tU32 receiver_index);
static tVoid ETAL_DABSeekThreadEntry(tPVoid thread_param);
static tVoid ETAL_DABSeekMainLoop(ETAL_HANDLE hReceiver);

#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETAL_DABSeekWaitingTask(tSInt stacd, tPVoid thread_param);
#else
static tVoid ETAL_DABSeekWaitingTask (tPVoid thread_param);
#endif //CONFIG_HOST_OS_TKERNEL

typedef struct
{
    tU32    receiverIndex;
}etalSeekThreadAttrTy;

static etalSeekThreadAttrTy seekThreadAttr[ETAL_MAX_RECEIVERS];

/***************************
 *
 * ETAL_DABSeekTaskInit
 *
 **************************/
tSInt ETAL_DABSeekTaskInit(tU32 receiver_index)
{
    tSInt ret = OSAL_OK;
    OSAL_trThreadAttribute attr0;
    char eventHandler[25];
    char threadName[25];

    if(receiver_index < ETAL_MAX_RECEIVERS)
    {
        sprintf(eventHandler, "DABSeek_EventHandler_%d", receiver_index);

        if (OSAL_s32EventCreate((tCString)eventHandler, &(DABSeek_TaskEventHandler[receiver_index])) == OSAL_OK)
        {
            /* Initialize the thread */
            sprintf(threadName, "%s_%d", ETAL_DAB_SEEK_THREAD_NAME, receiver_index);

            attr0.szName = threadName;
            attr0.u32Priority = ETAL_DAB_SEEK_THREAD_PRIORITY;
            attr0.s32StackSize = ETAL_DAB_SEEK_STACK_SIZE;
            attr0.pfEntry = ETAL_DABSeekWaitingTask;
            seekThreadAttr[receiver_index].receiverIndex = receiver_index;
            attr0.pvArg = (tPVoid)&seekThreadAttr[receiver_index];

            if ((ETAL_DABSeek_ThreadId[receiver_index] = OSAL_ThreadSpawn(&attr0)) == OSAL_ERROR)
            {
                ret = OSAL_ERROR_DEVICE_INIT;
            }
        }
        else
        {
            ret = OSAL_ERROR;
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
 * ETAL_DABSeekTaskDeinit
 *
 **************************/
tSInt ETAL_DABSeekTaskDeinit(tU32 receiver_index)
{
    tSInt ret = OSAL_OK;

    if(receiver_index < ETAL_MAX_RECEIVERS)
    {
        /* Stop the DAB seek if it was in running process */
        ETAL_DABSeekStopTask(receiver_index);

        /* Post the kill event */
        (LINT_IGNORE_RET) OSAL_s32EventPost (DABSeek_TaskEventHandler[receiver_index],
                ETAL_DAB_SEEK_EVENT_KILL_FLAG,
                OSAL_EN_EVENTMASK_OR);
        OSAL_s32ThreadWait(10);

        /* Delete the thread */
        if (OSAL_s32ThreadDelete(ETAL_DABSeek_ThreadId[receiver_index]) != OSAL_OK)
        {
            ret = OSAL_ERROR;
        }
        else
        {
            ETAL_DABSeek_ThreadId[receiver_index] = (OSAL_tThreadID)0;
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
 * ETAL_DABSeekStartTask
 *
 **************************/
static tSInt ETAL_DABSeekStartTask(tU32 receiver_index)
{
    tSInt ret = OSAL_OK;

    if(receiver_index < ETAL_MAX_RECEIVERS)
    {
#ifdef DEBUG_SEEK
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekStartTask recIdx: %d\n", receiver_index);
#endif
        /* This is done by posting the awake event */
        (LINT_IGNORE_RET) OSAL_s32EventPost (DABSeek_TaskEventHandler[receiver_index],
                ETAL_DAB_SEEK_EVENT_WAKEUP_FLAG,
                OSAL_EN_EVENTMASK_OR);
    }
    else
    {
        ret = OSAL_ERROR;
    }
    return ret;
}

/***************************
 *
 * ETAL_DABSeekStopTask
 *
 **************************/
tSInt ETAL_DABSeekStopTask(tU32 receiver_index)
{
    tSInt ret = OSAL_OK;
    if(receiver_index < ETAL_MAX_RECEIVERS)
    {
#ifdef DEBUG_SEEK
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekStopTask recIdx: %d\n", receiver_index);
#endif
        /* this is done by posting the sleep event */
        (LINT_IGNORE_RET) OSAL_s32EventPost (DABSeek_TaskEventHandler[receiver_index],
                ETAL_DAB_SEEK_EVENT_SLEEP_FLAG,
                OSAL_EN_EVENTMASK_OR);
    }
    else
    {
        ret = OSAL_ERROR;
    }
    return ret;
}

/***************************
 *
 * ETAL_DABSeek_TaskClearEventFlag
 *
 **************************/
static tSInt ETAL_DABSeek_TaskClearEventFlag(tU32 eventFlag, tU32 receiver_index)
{
    tSInt ret = OSAL_OK;

    if(receiver_index < ETAL_MAX_RECEIVERS)
    {
#ifdef DEBUG_SEEK
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_DABSeek_TaskClearEventFlag eveFla: %d, recIdx: %d\n", eventFlag, receiver_index);
#endif
        /* Clear old event if any (this can happen after a stop) */
        (LINT_IGNORE_RET) OSAL_s32EventPost(DABSeek_TaskEventHandler[receiver_index],
                ~eventFlag, OSAL_EN_EVENTMASK_AND);
    }
    else
    {
        ret = OSAL_ERROR;
    }
    return ret;
}

/***************************
 *
 * ETAL_DABSeekWaitingTask
 *
 **************************/
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETAL_DABSeekWaitingTask(tSInt stacd, tPVoid thread_param)
#else
static tVoid ETAL_DABSeekWaitingTask (tPVoid thread_param)
#endif //CONFIG_HOST_OS_TKERNEL
{
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETAL_DAB_SEEK_NO_EVENT_FLAG;
    char eventHandler[25];
    etalSeekThreadAttrTy *seekthreadAttr;

    if(thread_param != NULL)
    {
        seekthreadAttr = (etalSeekThreadAttrTy*)thread_param;

        if(seekthreadAttr->receiverIndex < ETAL_MAX_RECEIVERS)
        {
            while (TRUE)
            {
                // Wait here the auto wake up event
                ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekWaitingTask : waiting for DAB seek activation, recIdx = %d\n",
                        seekthreadAttr->receiverIndex);

                vl_res = OSAL_s32EventWait (DABSeek_TaskEventHandler[seekthreadAttr->receiverIndex],
                        ETAL_DAB_SEEK_EVENT_WAIT_WAKEUP_MASK,
                        OSAL_EN_EVENTMASK_OR,
                        OSAL_C_TIMEOUT_FOREVER,
                        &vl_resEvent);

                ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekWaitingTask : DAB seek awaked for recIdx = %d, event = 0x%x, vl_res = %d\n",
                        seekthreadAttr->receiverIndex, vl_resEvent, vl_res);

                if (OSAL_ERROR == vl_res)
				{
					// Event wait failure ==> break;
					
					ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_DABSeekWaitingTask: wait error");
					break;
				}
				else if ((vl_resEvent == ETAL_DAB_SEEK_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
                {
                    /* This is a timeout : no even received */
                }
				else if (OSAL_OK == vl_res)
                /* DAB seek event trigger DAB seek event processing
                * else normal handling ie if res = OSAL_ERROR => this is a timeout, so
                * normal processing or event return flag contains EVENT_CMD.. so normal processing */
				{
	                if (ETAL_DAB_SEEK_EVENT_KILL_FLAG == (ETAL_DAB_SEEK_EVENT_KILL_FLAG & vl_resEvent))
	                {
	                    /* Kill the DAB seek */
	                    break;
	                }
	                /* This is a DAB SEEK EVENT WAKE UP call the DAB SEEK event handler */
	                else   if (ETAL_DAB_SEEK_EVENT_WAKEUP_FLAG == (ETAL_DAB_SEEK_EVENT_WAKEUP_FLAG & vl_resEvent))
	                {
	                    /*
	                     * if the DAB seek stop was called without a DAB seek start, the SLEEP flag
	                     * is set and the ETAL_DABSeekStartTask has no effect
	                     * so ensure the SLEEP flag is clear
	                     */
	                    ETAL_DABSeek_TaskClearEventFlag(ETAL_DAB_SEEK_EVENT_ALL_FLAG, seekthreadAttr->receiverIndex);
	                    ETAL_DABSeekThreadEntry(thread_param);
	                    /* Clear the received event */
	                }
	                else
	                {
	                    /* a non expected event : just clear all */
	                    ETAL_DABSeek_TaskClearEventFlag(ETAL_DAB_SEEK_EVENT_ALL_FLAG, seekthreadAttr->receiverIndex);
	                }
				}
				else
				{
					// nothing to do
				}

            }

            /* Close the events */
#ifndef OSAL_EVENT_SKIP_NAMES
            if (OSAL_s32EventClose(DABSeek_TaskEventHandler[seekthreadAttr->receiverIndex]) != OSAL_OK)
            {
                vl_res = OSAL_ERROR;
            }

            OSAL_s32ThreadWait(100);

            sprintf(eventHandler, "DABSeek_EventHandler_%d", seekthreadAttr->receiverIndex);
            if (OSAL_s32EventDelete((tCString)eventHandler) != OSAL_OK)
            {
                vl_res = OSAL_ERROR;
            }
#else
            if (OSAL_s32EventFree(DABSeek_TaskEventHandler[seekthreadAttr->receiverIndex]) != OSAL_OK)
            {
                vl_res = OSAL_ERROR;
            }
#endif //#ifndef OSAL_EVENT_SKIP_NAMES
        }
        else
        {
            ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekWaitingTask : Invalid receiver index: %d\n", seekthreadAttr->receiverIndex);
        }
    }
    else
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekThreadEntry :  thread_param is NULL!\n");
    }

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
 * ETAL_DABSeekThreadEntry
 *
 **************************/
static tVoid ETAL_DABSeekThreadEntry(tPVoid thread_param)
{
    ETAL_HANDLE hReceiver;
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETAL_DAB_SEEK_NO_EVENT_FLAG;
    etalSeekThreadAttrTy *seekthreadAttr;

    if(thread_param != NULL)
    {
        seekthreadAttr = (etalSeekThreadAttrTy*)thread_param;

        ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekThreadEntry :  DAB seek awaked for recIdx = %d\n",
                seekthreadAttr->receiverIndex);

        while (TRUE)
        {
           vl_res = OSAL_s32EventWait(DABSeek_TaskEventHandler[seekthreadAttr->receiverIndex],
                   ETAL_DAB_SEEK_EVENT_WAIT_MASK,
                   OSAL_EN_EVENTMASK_OR,
                   ETAL_DAB_SEEK_EVENT_WAIT_TIME_MS,
                   &vl_resEvent);

           ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekThreadEntry : awaked for recIdx = %d, event = 0x%x, vl_res = %d\n",
                   seekthreadAttr->receiverIndex, vl_resEvent, vl_res);

            /* If this is for sleeping, leave this */
			if (OSAL_ERROR == vl_res)
			{
				// Event wait failure ==> break;			
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_DABSeekThreadEntry: wait error");
				break;
			}
            else if (ETAL_DAB_SEEK_EVENT_SLEEP_FLAG == (vl_resEvent & ETAL_DAB_SEEK_EVENT_SLEEP_FLAG))
            {
                ETAL_DABSeek_TaskClearEventFlag(ETAL_DAB_SEEK_EVENT_SLEEP_FLAG, seekthreadAttr->receiverIndex);
                break;
            }
			else 
			{
	            if ((vl_resEvent == ETAL_DAB_SEEK_NO_EVENT_FLAG) ||
	                     (vl_res == OSAL_ERROR_TIMEOUT_EXPIRED))
	            {
	                hReceiver = ETAL_handleMakeReceiver(seekthreadAttr->receiverIndex);

	                if (ETAL_receiverIsValidHandle(hReceiver))
	                {
	                    if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
	                    {
	                        ASSERT_ON_DEBUGGING(0);
	                        continue;
	                    }

	                    ETAL_DABSeekMainLoop(hReceiver);
	                    ETAL_receiverReleaseLock(hReceiver);
	                }
	            }
	            else
	            {
	                /* Manage  events */
	            }
			}		
        }
    }
    else
    {
        ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekThreadEntry :  thread_param is NULL!\n");
    }

    return;
}

/***************************
 *
 * ETAL_DABSeekMainLoop
 *
 **************************/
static tVoid ETAL_DABSeekMainLoop(ETAL_HANDLE hReceiver)
{
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
    etalSeekConfigTy *seekCfgp = &(recvp->seekCfg);
    EtalSeekStatus *seekStatusp = &(seekCfgp->autoSeekStatus);
    tSInt retval;
	etalFrequencyBandInfoTy band_info;
	tU32 freq;
	OSAL_tMSecond now;
	EtalEnsembleList ens_list;
	tBool have_data;

	if (ETAL_receiverIsValidHandle(hReceiver))
    {
#ifdef DEBUG_SEEK
		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_API, "state: %d for receiver: %d\n",seekCfgp->state, hReceiver);
#endif
		switch(seekCfgp->state)
		{
			 case ETAL_DABSEEK_NULL:
				break;

             case ETAL_DABSEEK_START:
            	seekCfgp->noiseFloor = NOISEFLOOR_INIT;
             	seekCfgp->seekThr = seekCfgp->noiseFloor;
             	seekCfgp->isNoiseFloorSet = FALSE;

             	/* tune to frequency 0 */
          		retval = ETAL_cmdTune_MDR(hReceiver, 0, cmdTuneNormalResponse, 0);
          		if ((retval != OSAL_OK) && (retval != OSAL_ERROR_TIMEOUT_EXPIRED))
          		{
	                /* Stop seek and send event */
	                ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR_ON_START);

                    /* Change seek state */
                    seekCfgp->state = ETAL_DABSEEK_NULL;
          		}
          		else
          		{
#ifdef DEBUG_SEEK
          			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB TUNED freq: 0 on receiver: %d!!!\n", hReceiver);
#endif
          			seekCfgp->time = OSAL_ClockGetElapsedTime();

          			/* Change seek state */
          			seekCfgp->state = ETAL_DABSEEK_FST;
          		}
            	break;

             case ETAL_DABSEEK_FST:
            	 if (OSAL_OK != ETAL_receiverGetBandInfo(hReceiver,&band_info))
            	 {
            		 /* Stop seek and send event */
            		 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

            		 /* Change seek state */
                     seekCfgp->state = ETAL_DABSEEK_NULL;
            	 }
            	 else
            	 {
            		 /* Get next frequency from current frequency in the chosen direction */
            		 freq = DABMW_GetNextFrequencyFromFreq(ETAL_receiverGetFrequency(hReceiver),DABMW_TranslateEtalBandToDabmwBand(band_info.band), (seekCfgp->direction == cmdDirectionUp) ? TRUE : FALSE);
            		 if (freq == DABMW_INVALID_FREQUENCY)
            		 {
            			 /* Stop seek and send event */
            			 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

            			 /* Change seek state */
            			 seekCfgp->state = ETAL_DABSEEK_NULL;
            		 }
            		 else
            		 {
            			 /* Build seek event */
            			 ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
                         seekStatusp->m_status = ETAL_SEEK_RESULT;

                         if(((band_info.bandMin == freq) && (seekCfgp->direction == cmdDirectionUp)) ||
                        	((band_info.bandMax == freq) && (seekCfgp->direction == cmdDirectionDown)))
                         {
                        	 /* Reset at band border crossed */
                        	 seekCfgp->noiseFloor = NOISEFLOOR_INIT;
                        	 seekCfgp->seekThr = seekCfgp->noiseFloor;
                        	 seekCfgp->isNoiseFloorSet = FALSE;
                         }

                         if (ETAL_cmdSeekStart_CMOST(hReceiver, seekCfgp->direction, freq, cmdManualModeStart, seekCfgp->exitSeekAction, TRUE, TRUE) != OSAL_OK)
                         {
                        	 /* Stop seek and send event */
                        	 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

                        	 /* Change seek state */
                        	 seekCfgp->state = ETAL_DABSEEK_NULL;
                         }
                         else
                         {
                        	 ETAL_receiverSetFrequency(hReceiver, freq, TRUE);

                        	 /* Get Channel Quality */
                        	 if (ETAL_cmdGetChannelQuality_CMOST(hReceiver, &(seekStatusp->m_quality)) != OSAL_OK)
                        	 {
                        		 /* Stop seek and send event */
                        		 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

                        		 /* Change seek state */
                        		 seekCfgp->state = ETAL_DABSEEK_NULL;
                        	 }
                        	 else
                        	 {
                        		 if(seekStatusp->m_quality.EtalQualityEntries.dab.m_RFFieldStrength < seekCfgp->noiseFloor)
                        		 {
                        			 seekCfgp->noiseFloor = seekStatusp->m_quality.EtalQualityEntries.dab.m_RFFieldStrength;
                        			 seekCfgp->seekThr = seekCfgp->noiseFloor + SEEK_TH_INC;

                        			 if(seekCfgp->startFrequency == ETAL_receiverGetFrequency(hReceiver))
                        			 {
                        				 seekStatusp->m_fullCycleReached = true;

                        				 if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
                        				 {
                        					 /* Seek is implicitly stopped */

                        					 /* Change seek state */
                        					 seekCfgp->state = ETAL_DABSEEK_STOP;
                        				 }
                        				 else
                        				 {
                        					 /* Change seek state */
                        					 seekCfgp->state = ETAL_DABSEEK_NULL;
                        				 }
                        			 }

                        			 /* Send event and notify */
                        			 ETAL_Report_Seek_Info(hReceiver, seekStatusp);
                        		 }
                        		 else
                        		 {
                        			 if(seekStatusp->m_quality.EtalQualityEntries.dab.m_RFFieldStrength > seekCfgp->seekThr)
                        			 {
                        				 if (ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == FALSE)
                        				 {
                        					 if(ETAL_intCbRegister(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver, ETAL_INTCB_CONTEXT_UNUSED) == ETAL_RET_SUCCESS)
                        					 {
                            					 if (ETAL_cmdTune_MDR(hReceiver, freq, cmdTuneImmediateResponse, 0) == OSAL_OK)
                            					 {
#ifdef DEBUG_SEEK
                            						 ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB TUNED freq: %d on receiver: %d!!!\n", freq, hReceiver);
#endif
                            						 seekCfgp->time = OSAL_ClockGetElapsedTime();

                            						 /* Change seek state */
                            						 seekCfgp->state = ETAL_DABSEEK_WAIT_FOR_SYNC;
                            					 }
                            					 else
                            					 {
                            						 /* Stop seek and send event */
                            						 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

                            						 /* Change seek state */
                            						 seekCfgp->state = ETAL_DABSEEK_NULL;
                            					 }
                        					 }
                        					 else
                        					 {
                        						 /* Stop seek and send event */
                        						 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

                        						 /* Change seek state */
                        						 seekCfgp->state = ETAL_DABSEEK_NULL;
                        					 }
                        				 }
                        				 else
                        				 {
                        					 if (ETAL_cmdTune_MDR(hReceiver, freq, cmdTuneImmediateResponse, 0) == OSAL_OK)
                        					 {
#ifdef DEBUG_SEEK
                        						 ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB TUNED freq: %d on receiver: %d!!!\n", freq, hReceiver);
#endif
                        						 seekCfgp->time = OSAL_ClockGetElapsedTime();

                        						 /* Change seek state */
                        						 seekCfgp->state = ETAL_DABSEEK_WAIT_FOR_SYNC;
                        					 }
                        					 else
                        					 {
                        						 /* Stop seek and send event */
                        						 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

                        						 /* Change seek state */
                        						 seekCfgp->state = ETAL_DABSEEK_NULL;
                        					 }
                        				 }
                        			 }
                        			 else
                        			 {
                            			 if(seekCfgp->startFrequency == ETAL_receiverGetFrequency(hReceiver))
                        				 {
                        					 seekStatusp->m_fullCycleReached = true;

                        					 if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
                        					 {
                        						 /* Seek is implicitly stopped */

                        						 /* Change seek state */
                        						 seekCfgp->state = ETAL_DABSEEK_STOP;
                        					 }
                        					 else
                        					 {
                        						 /* Change seek state */
                        						 seekCfgp->state = ETAL_DABSEEK_NULL;
                        					 }
                        				 }

                        				 /* Send event and notify */
                        				 ETAL_Report_Seek_Info(hReceiver, seekStatusp);
                        			 }
                        		 }
                        	 }
                         }
            		 }
            	 }
            	 break;

             case ETAL_DABSEEK_WAIT_FOR_SYNC:
				 now = OSAL_ClockGetElapsedTime();

            	 if(now > seekCfgp->time + TUNE_SYNC_TIMEOUT)
            	 {
#ifdef DEBUG_SEEK
            		 ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB SYNC TIMEOUT receiver: %d!!!\n", hReceiver);
#endif

            		 /* Build seek event */
        			 ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
                     seekStatusp->m_status = ETAL_SEEK_RESULT;

					 if ((ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == TRUE) &&
					     (ETAL_intCbDeregister(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) != ETAL_RET_SUCCESS))
					 {
						 /* Stop seek and send event */
						 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

						 /* Change seek state */
						 seekCfgp->state = ETAL_DABSEEK_NULL;
					 }
					 else
					 {
            			 if(seekCfgp->startFrequency == ETAL_receiverGetFrequency(hReceiver))
        				 {
        					 seekStatusp->m_fullCycleReached = true;

        					 if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
        					 {
                        		 seekCfgp->terminationMode = lastFrequency;

        						 /* Change seek state */
        						 seekCfgp->state = ETAL_DABSEEK_STOP;
        					 }
        					 else
        					 {
        						 /* Change seek state */
        						 seekCfgp->state = ETAL_DABSEEK_NULL;
        					 }
        				 }
        				 else
        				 {
        					 /* Change seek state */
    						 seekCfgp->state = ETAL_DABSEEK_SET_THRESHOLD;
        				 }

						 /* Send event and notify */
						 ETAL_Report_Seek_Info(hReceiver, seekStatusp);
					 }
            	 }
            	 break;

             case ETAL_DABSEEK_WAIT_FOR_ENSEMBLE:
				 now = OSAL_ClockGetElapsedTime();

            	 if(now > seekCfgp->time + WAIT_FOR_ENSEMBLE)
            	 {
#ifdef DEBUG_SEEK
            		 ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB GET ENSEMBLE TIMEOUT receiver: %d!!!\n", hReceiver);
#endif

            		 /* Build seek event */
        			 ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
                     seekStatusp->m_status = ETAL_SEEK_RESULT;

        			 if(seekCfgp->startFrequency == ETAL_receiverGetFrequency(hReceiver))
    				 {
    					 seekStatusp->m_fullCycleReached = true;

    					 if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
    					 {
                    		 seekCfgp->terminationMode = lastFrequency;

    						 /* Change seek state */
    						 seekCfgp->state = ETAL_DABSEEK_STOP;
    					 }
    					 else
    					 {
    						 /* Change seek state */
    						 seekCfgp->state = ETAL_DABSEEK_NULL;
    					 }
    				 }
    				 else
    				 {
    					 /* Change seek state */
						 seekCfgp->state = ETAL_DABSEEK_SET_THRESHOLD;
    				 }

					 /* Send event and notify */
					 ETAL_Report_Seek_Info(hReceiver, seekStatusp);
            	 }
            	 else
            	 {
            		 memset(&ens_list, 0x00, sizeof(ens_list));
            		 if ((ETAL_cmdGetEnsembleList_MDR(&ens_list, &have_data) == OSAL_OK) &&
            			 (have_data == TRUE))
            		 {
                		 /* Build seek event */
            			 ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
                         seekStatusp->m_status = ETAL_SEEK_RESULT;

            			 seekStatusp->m_frequencyFound = true;

            			 if(seekCfgp->startFrequency == ETAL_receiverGetFrequency(hReceiver))
        				 {
        					 seekStatusp->m_fullCycleReached = true;
        				 }

            			 if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
            			 {
            				 seekCfgp->terminationMode = lastFrequency;

            				 /* Change seek state */
            				 seekCfgp->state = ETAL_DABSEEK_STOP;
            			 }
            			 else
            			 {
            				 /* Change seek state */
            				 seekCfgp->state = ETAL_DABSEEK_NULL;
            			 }

    					 /* Send event and notify */
    					 ETAL_Report_Seek_Info(hReceiver, seekStatusp);
            		 }
            	 }
            	 break;

             case ETAL_DABSEEK_SET_THRESHOLD:
				 if(seekStatusp->m_quality.EtalQualityEntries.dab.m_RFFieldStrength < NOISEFLOOR_MAX)
				 {
					 freq = ETAL_receiverGetFrequency(hReceiver);

		         	 if (OSAL_OK != ETAL_receiverGetBandInfo(hReceiver,&band_info))
		         	 {
		         		 /* Stop seek and send event */
		         		 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

		         		 /* Change seek state */
		         		 seekCfgp->state = ETAL_DABSEEK_NULL;
		         	 }
		         	 else
		         	 {
						 if((band_info.band == ETAL_BAND_DAB3) &&
							((freq != 175008)/*6N*/ && (freq != 191008)/*8N*/ && (freq != 207008)/*10N*/ && (freq != 210096)/*10N*/ && (freq != 217088)/*11N*/ && (freq != 224096)/*12N*/))
						 {
							 if(seekCfgp->isNoiseFloorSet == TRUE)
							 {
								 seekCfgp->noiseFloor += SEEK_TH_INC_NO_SYNC;
								 seekCfgp->seekThr += SEEK_TH_INC_NO_SYNC;
							 }
							 else
							 {
								 seekCfgp->noiseFloor = seekStatusp->m_quality.EtalQualityEntries.dab.m_RFFieldStrength;
								 seekCfgp->seekThr = seekCfgp->noiseFloor + SEEK_TH_INC_NO_SYNC;
								 seekCfgp->isNoiseFloorSet = FALSE;
							 }
						 }
		         	 }
				 }

				 /* tune to frequency 0 */
				 retval = ETAL_cmdTune_MDR(hReceiver, 0, cmdTuneNormalResponse, 0);
				 if ((retval != OSAL_OK) && (retval != OSAL_ERROR_TIMEOUT_EXPIRED))
				 {
					 /* Stop seek and send event */
					 ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

					 /* Change seek state */
					 seekCfgp->state = ETAL_DABSEEK_NULL;
				 }
				 else
				 {
#ifdef DEBUG_SEEK
					 ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "DAB TUNED freq: 0 on receiver: %d!!!\n", hReceiver);
#endif
					 /* Change seek state */
					 seekCfgp->state = ETAL_DABSEEK_FST;
				 }
            	 break;

             case ETAL_DABSEEK_STOP:
            	 if (ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == TRUE)
            	 {
            		 (LINT_IGNORE_RET)ETAL_intCbDeregister(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver);
            	 }

            	 if (ETAL_cmdSeekEnd_CMOST(hReceiver, seekCfgp->exitSeekAction) == OSAL_OK)
            	 {
        			 if(seekCfgp->startFrequency == ETAL_receiverGetFrequency(hReceiver))
        			 {
        				 seekStatusp->m_fullCycleReached = true;
        			 }

            		 /* Tune initial frequency if requested */
            		 if((seekCfgp->terminationMode == initialFrequency) && (seekCfgp->startFrequency != ETAL_INVALID_FREQUENCY))
            		 {
						 if(ETAL_tuneReceiverInternal(hReceiver, seekCfgp->startFrequency, cmdTuneNormalResponse) == ETAL_RET_ERROR)
						 {
							 seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
						 }
						 else
						 {
							 seekStatusp->m_status = ETAL_SEEK_FINISHED;
						 }
            		 }
            		 else
            		 {
            			 seekStatusp->m_status = ETAL_SEEK_FINISHED;
            		 }
            	 }
            	 else
            	 {
            		 seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
            	 }

            	 /* Stop seek and send event */
            	 ETAL_stop_and_send_event(hReceiver, seekCfgp, seekStatusp->m_status);

            	 /* Change seek state */
            	 seekCfgp->state = ETAL_DABSEEK_NULL;
            	 break;

             default:
            	 ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_DABSeekMainLoop wrong state: %d on receiver: %d", seekCfgp->state, hReceiver);
                 ASSERT_ON_DEBUGGING(0);
            	 break;
         }
    }
    return;
}

tVoid ETAL_SeekIntCbFuncDabSync(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
    ETAL_HANDLE hReceiver = hGeneric;
    etalAutoNotificationStatusTy *DABnotif = (etalAutoNotificationStatusTy *) param;
    etalReceiverStatusTy *recvp = NULL;
    etalSeekConfigTy *seekCfgp = NULL;
    EtalSeekStatus *seekStatusp = NULL;
    tU32 synchronized;

    if (ETAL_handleIsReceiver(hGeneric))
    {
        recvp = ETAL_receiverGet(hReceiver);
        seekCfgp = &(recvp->seekCfg);
        seekStatusp = &(seekCfgp->autoSeekStatus);

        if (DABnotif != NULL)
        {
        	if(DABnotif->type == autoDABStatus)
        	{		
        		synchronized = ETAL_TUNESTATUS_SYNCMASK_DAB_FOUND | ETAL_TUNESTATUS_SYNCMASK_DAB_SYNC | ETAL_TUNESTATUS_SYNCMASK_DAB_MCI;

				// process only the 'Tune' Reason
				if (DABnotif->status.DABStatus.reason == DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_TUNE)
				{
				
	        		if (((DABnotif->status.DABStatus.sync & synchronized) == synchronized) ||
	        			(DABnotif->status.DABStatus.sync == 0))
	        		{
	        			if ((ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == TRUE) &&
	        			    (ETAL_intCbDeregister(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == ETAL_RET_SUCCESS))
	        			{
							if(DABnotif->status.DABStatus.sync == 0)
	        				{
	        					if(seekCfgp->startFrequency == ETAL_receiverGetFrequency(hReceiver))
	        					{
	        						seekStatusp->m_fullCycleReached = true;

	        						if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
	        						{
	        							seekCfgp->terminationMode = lastFrequency;

	        							/* Change seek state */
	        							seekCfgp->state = ETAL_DABSEEK_STOP;
	        						}
	        						else
	        						{
	        							/* Change seek state */
	        							seekCfgp->state = ETAL_DABSEEK_NULL;
	        						}
	        					}
	        					else
	        					{
	        						/* Change seek state */
	        						seekCfgp->state = ETAL_DABSEEK_SET_THRESHOLD;
	        					}

	        					/* Send event and notify */
	        					ETAL_Report_Seek_Info(hReceiver, seekStatusp);
	        				}
	        				else
	        				{
	        					/* Update time stamps*/
	        					seekCfgp->time = OSAL_ClockGetElapsedTime();

	        					/* Change seek state */
	        					seekCfgp->state = ETAL_DABSEEK_WAIT_FOR_ENSEMBLE;
	        				}
	        			}
	        			else
	        			{
	        				/* Send event and notify */
	        				ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

	        				/* Change seek state */
	        				seekCfgp->state = ETAL_DABSEEK_NULL;
	        			}
        			}
					else
					{
						// this is a notification for other reasons : 
						// in seek mode we wait the tune reason
					}
        		}
        	}
        }
        else
        {
			/* Send event and notify */
			ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);

			/* Change seek state */
			seekCfgp->state = ETAL_DABSEEK_NULL;
        }
    }
    return;
}
#endif //#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

/***************************
 *
 * ETAL_SeekStatusPeriodicFuncInternal
 *
 **************************/
tVoid ETAL_SeekStatusPeriodicFuncInternal(ETAL_HANDLE hGeneric)
{
    ETAL_HANDLE hReceiver = hGeneric;
    etalReceiverStatusTy *recvp = NULL;
    etalSeekConfigTy *seekCfgp = NULL;
    EtalSeekStatus *seekStatusp = NULL;
	ETAL_STATUS ret;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
    tU8 cmd[1];
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

    if (ETAL_handleIsReceiver(hGeneric) == true)
    {
    	// should not we lock ???
    	// what is lock is not the receiver but internal lock
    	// which allow other processing
    	// safer would be to lock
        hReceiver = hGeneric;
		ret = ETAL_receiverGetLock(hReceiver);
		
		//check receiver is still valid and has not been destroyed !
		if (false == ETAL_receiverIsValidHandle(hReceiver))
		{
			goto exit;
		}

	    recvp = ETAL_receiverGet(hReceiver);
	    seekCfgp = &(recvp->seekCfg);
	    seekStatusp = &(seekCfgp->autoSeekStatus);
		
	    /* Prepare ETAL_INFO_SEEK event*/
	    ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
	    seekStatusp->m_status = ETAL_SEEK_RESULT;

 	   	if (ret == ETAL_RET_SUCCESS)
   	   	{
	        if (ETAL_cmdSeekGetStatus_CMOST(hReceiver, &(seekStatusp->m_frequencyFound), &(seekStatusp->m_fullCycleReached),
	                NULL, &(seekStatusp->m_frequency), &(seekStatusp->m_quality)) == OSAL_OK)
	        {
	             if ((seekStatusp->m_frequency != 0) && (seekStatusp->m_frequency != ETAL_INVALID_FREQUENCY))
	            {
	                /* Update the current frequency */
	                ETAL_receiverSetFrequency(hReceiver, seekStatusp->m_frequency, TRUE);
	            }

	            if((seekStatusp->m_frequencyFound == true) || (seekStatusp->m_fullCycleReached == true))
	            {
	                /* Deregister callback function */
	                if (ETAL_intCbIsRegisteredPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver) == TRUE)
	                {
	                    /* Deregister periodic callback */
	                    ETAL_intCbDeregisterPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver);
	                }

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	                if ((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM))
	                {
	                    /* issue a seek start to the HD thread */
	                    cmd[0] = (tU8)HDRADIO_SEEK_START_SPECIAL;
	                    ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1);
	                }
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

	                if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
	                {
	                    /* Seek is implicitly stopped */

	                    /* Send event and notify */
	                    ETAL_Report_Seek_Info(hReceiver, seekStatusp);

	                    /* Send event to notify seek is stopped */
	                    ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_FINISHED);
	                }
	                else
	                {
	                    /* Send event and notify */
	                    ETAL_Report_Seek_Info(hReceiver, seekStatusp);
	                }
	            }
	            else
	            {
	                /* Seek ongoing */
	                /* Send event and notify */
	                ETAL_Report_Seek_Info(hReceiver, seekStatusp);
	            }
	        }
	        else
	        {
	            /* Stop seek and send event */
	            ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);
	        }

			// release receiver lock
			ETAL_receiverReleaseLock(hReceiver);
			
    	}
		else
		{
			// lock error
		}
	}
    else
    {
        /* Stop seek and send event */
		// not a valid receiver... 
		// so we cannot do anything
        //ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR);
    }

exit:	
    return;
}


/***************************
 *
 * ETAL_seek_start_internal
 *
 **************************/
ETAL_STATUS ETAL_seek_start_internal(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, etalSeekHdModeTy seekHDSPS, tBool updateStopFrequency)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	tU32 device_list;
	tBool need_tune = TRUE; 
    etalReceiverStatusTy *recvp = NULL;
    etalSeekConfigTy *seekCfgp = NULL;
    EtalSeekStatus *seekStatusp = NULL;

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
    ETAL_HANDLE hreceiver_main, hreceiver_secondary;
	tBool main_seek_started, secondary_seek_started;
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO 
	tU8 cmd[1];
#endif

	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
#if 0
    else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency) ||
			 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessEstimation) ||
			 ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessSwitching))
	{
		ret = ETAL_RET_IN_PROGRESS;
	}
#endif // if 0

	else if (!ETAL_receiverSupportsQuality(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
 		if (ETAL_checkSeekStartParameters(hReceiver, direction, step, exitSeekAction, seekHDSPS) != ETAL_RET_SUCCESS)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else
		{
		    recvp = ETAL_receiverGet(hReceiver);
		    seekCfgp = &(recvp->seekCfg);
		    seekStatusp = &(seekCfgp->autoSeekStatus);

	        /* Prepare ETAL_INFO_SEEK event*/
	        ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
	        seekStatusp->m_status = ETAL_SEEK_STARTED;

		    seekCfgp->exitSeekAction      = exitSeekAction;
            seekCfgp->direction           = (direction == 0) ? cmdDirectionUp : cmdDirectionDown;
            seekCfgp->step                = step;
            seekCfgp->updateStopFrequency = updateStopFrequency;

            /* this field is used only for a special case of the HD seek algo */
            seekCfgp->autoSeekStatus.m_HDProgramFound = FALSE;
            seekCfgp->seekHDSPS = dontSeekInSPS;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
            if (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB)
            {
				/* if this is a DAB receiver, we should check whether we should start task or not. */
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
					/* activate the DAB seek task */
					ETAL_DABSeekStartTask(ETAL_handleReceiverGetIndex(hReceiver));
				}
            }
#endif //CONFIG_ETAL_SUPPORT_DCOP_MDR

            ETAL_receiverSetSpecial(hReceiver, cmdSpecialSeek, cmdActionStart);

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
            // if we are in HD ie HD FM or HD AM, we may need to seek in SPS
            if (((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM)) &&
                (seekHDSPS == seekInSPS))
            {
                /* In case we are already tuned HD requires some special
                 processing before possibly jumping to a different frequency */
                need_tune = ETAL_seekHDNeedsTune_HDRADIO(hReceiver, (direction == cmdDirectionUp ? 0 : 1));
                /* HD seek only: also step into the SPS */
                seekCfgp->seekHDSPS = seekInSPS;
            }
            if (!need_tune)
            {
                /* the seek start event is normally send by the FSM */
                ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
                seekStatusp->m_status    = ETAL_SEEK_STARTED;
                ETAL_Report_Seek_Info(hReceiver, &(seekCfgp->autoSeekStatus));

                ETAL_InitAutoSeekStatus(ETAL_receiverGetStandard(hReceiver), seekStatusp);
                seekStatusp->m_HDProgramFound = TRUE;
                seekStatusp->m_frequencyFound = TRUE;
                if(ETAL_get_reception_quality_internal(hReceiver, &(seekStatusp->m_quality)) == ETAL_RET_SUCCESS)
                {
                    seekStatusp->m_status = ETAL_SEEK_RESULT;
                }
                else
                {
                    ret = ETAL_RET_ERROR;
                }

                // retrieve current program information
                ETAL_receiverGetRadioInfo_HDRADIO(hReceiver, NULL, &(seekStatusp->m_serviceId), NULL, NULL);

                if(seekCfgp->exitSeekAction == cmdAudioUnmuted)
                {
                    /* Seek is implicitly stopped */

                    /* Send event and notify */
                    ETAL_Report_Seek_Info(hReceiver, seekStatusp);

                    /* Send event to notify seek is stopped */
                    ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_FINISHED);
                }
                else
                {
                    /* Send event and notify */
                    ETAL_Report_Seek_Info(hReceiver, seekStatusp);
                }
            }
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

			if (need_tune)
			{
				if(seekCfgp->updateStopFrequency == TRUE)
				{
					seekCfgp->startFrequency = ETAL_receiverGetFrequency(hReceiver);
				}

				device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
				if (ETAL_CMDROUTE_TO_STAR(device_list))
				{

					
					
					/* Send Tuner_Seek_Start command */
				    if (ETAL_cmdSeekStart_CMOST(hReceiver, seekCfgp->direction, seekCfgp->step, cmdAutomaticModeStart, seekCfgp->exitSeekAction, seekCfgp->updateStopFrequency, FALSE) == OSAL_OK)
					{
						// In FM case, we should set the frequency to invalid during the seek
						// because the frequency will be known only on seek status polling
						// If we do not do that, there might be error on field association freq to field
						// typically RDS data : will be received valid but associated to wrong frequency !
						// Reset also the RDS since we move to new frequency
						ETAL_receiverSetFrequency(hReceiver, ETAL_INVALID_FREQUENCY, TRUE);
						
		                if (ETAL_intCbIsRegisteredPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver) == FALSE)
		                {
		                	if((ret = ETAL_intCbRegisterPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver, ETAL_SEEK_INTCB_DELAY)) != ETAL_RET_SUCCESS)
		                	{
		                		ret = ETAL_RET_ERROR;
		                	}
		                }
					}
					else
					{
                        ret = ETAL_RET_ERROR;
					}
				}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
				if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
					(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
				{
					/* start DAB seek */
					seekCfgp->state = ETAL_DABSEEK_START;
				}
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
				if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
					((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM)))
				{
					/* post a stop to avoid processing when seek on going */
					cmd[0] = (tU8)HDRADIO_SEEK_STOP_SPECIAL;
					if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
					{
					    ret = ETAL_RET_ERROR;
					}

					(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
				    /*
					 * commandBandSpecific returns deviceDCOP so the STAR part is not executed
					 * so we need to repeat it here
					 * Just to remember I put an ASSERT, if things change the STAR part
					 * below may be removed.
					 */
					ASSERT_ON_DEBUGGING(!ETAL_CMDROUTE_TO_STAR(device_list));

                    /* Send Tuner_Seek_Start command */
					if (ETAL_cmdSeekStart_CMOST(hReceiver, seekCfgp->direction, seekCfgp->step, cmdAutomaticModeStart, seekCfgp->exitSeekAction, seekCfgp->updateStopFrequency, FALSE) == OSAL_OK)
					{
		                if (ETAL_intCbIsRegisteredPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver) == FALSE)
		                {
		                if((ret = ETAL_intCbRegisterPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver, ETAL_SEEK_INTCB_DELAY)) != ETAL_RET_SUCCESS)
		                {
		                    ret = ETAL_RET_ERROR;
		                }
					}
					}
					else
					{
                        ret = ETAL_RET_ERROR;
					}
				}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

	            if(ret != ETAL_RET_SUCCESS)
	            {
	                /* Stop seek and send event */
	                ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR_ON_START);
	            }
	            else
	            {
	            	/* Send event and notify */
	            	ETAL_Report_Seek_Info(hReceiver, seekStatusp);
	            }
			}
		}
	}

	return ret;
}

/***************************
 *
 * ETAL_seek_continue_internal
 *
 **************************/
ETAL_STATUS ETAL_seek_continue_internal(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;


	tU32 device_list;
    etalReceiverStatusTy *recvp = NULL;
    etalSeekConfigTy *seekCfgp = NULL;
    EtalSeekStatus *seekStatusp = NULL;


	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else 
	{
		// set the seek pointer 
		
		recvp = ETAL_receiverGet(hReceiver);
		seekCfgp = &(recvp->seekCfg);
		seekStatusp = &(seekCfgp->autoSeekStatus);

		if (false == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeek))
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_seek_continue_internal seek not in progress");
			ret = ETAL_RET_ERROR;
		}
		// if a callback is registered it means the seek on the cmost side is ongoing
		// so continue is not valid
		else if (ETAL_intCbIsRegisteredPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver) == TRUE)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_seek_continue_internal FM seek on-going already");
			ret = ETAL_RET_ERROR;
		}
		// in DAB, the seek continue is allowed only if SEEK on-going action is ended
		// ie if state is seek_null
		else if ((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB) 
			&& (seekCfgp->state !=  ETAL_DABSEEK_NULL))
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_seek_continue_internal DAB seek on-going already");
			ret = ETAL_RET_ERROR;
		}
		else if (!ETAL_receiverSupportsQuality(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
	        if (ETAL_checkSeekContinueParameters(hReceiver) != ETAL_RET_SUCCESS)
	        {
	            ret = ETAL_RET_PARAMETER_ERR;
	        }
	        else
	        {
	            recvp = ETAL_receiverGet(hReceiver);
	            seekCfgp = &(recvp->seekCfg);
	            seekStatusp = &(seekCfgp->autoSeekStatus);

	            device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);

	            /* Prepare ETAL_INFO_SEEK event*/
	            seekStatusp->m_status = ETAL_SEEK_STARTED;

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	            if (ETAL_CMDROUTE_TO_STAR(device_list))
	            {
	                /* Send Tuner_Seek_Start command */
	                if (ETAL_cmdSeekStart_CMOST(hReceiver, /*unused*/seekCfgp->direction, /*unused*/seekCfgp->step, cmdContinue, /*unused*/seekCfgp->exitSeekAction, /*unused*/seekCfgp->updateStopFrequency, /*unused*/FALSE) == OSAL_OK)
	                {

							// In FM case, we should set the frequency to invalid during the seek
							// because the frequency will be known only on seek status polling
							// If we do not do that, there might be error on field association freq to field
							// typically RDS data : will be received valid but associated to wrong frequency !
							// Reset also the RDS since we move to new frequency
							ETAL_receiverSetFrequency(hReceiver, ETAL_INVALID_FREQUENCY, TRUE);
	
		                    if((ret = ETAL_intCbRegisterPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver, ETAL_SEEK_INTCB_DELAY)) != ETAL_RET_SUCCESS)
		                    {
		                    	ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_seek_continue_internal could not register call back");
		                        ret = ETAL_RET_ERROR;
		                    }
	                }
	                else
	                {
	                    ret = ETAL_RET_ERROR;
	                }

	            }
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	            if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
	                (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
	            {
	            	seekCfgp->state = ETAL_DABSEEK_START;
	            }
#endif // CONFIG_ETAL_SUPPORT_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	            if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
	                    ((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_FM) || (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_HD_AM)))
	            {
	                (LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);
	                /*
	                 * commandBandSpecific returns deviceDCOP so the STAR part is not executed
	                 * so we need to repeat it here
	                 * Just to remember I put an ASSERT, if things change the STAR part
	                 * below may be removed.
	                 */
	                ASSERT_ON_DEBUGGING(!ETAL_CMDROUTE_TO_STAR(device_list));

	                /* Send Tuner_Seek_Start command */
	                if (ETAL_cmdSeekStart_CMOST(hReceiver, /*unused*/seekCfgp->direction, /*unused*/seekCfgp->step, cmdContinue, /*unused*/seekCfgp->exitSeekAction, /*unused*/seekCfgp->updateStopFrequency, /*unused*/FALSE) == OSAL_OK)
	                {
		                if (ETAL_intCbIsRegisteredPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver) == FALSE)
		                {
	                    if((ret = ETAL_intCbRegisterPeriodic(ETAL_SeekStatusPeriodicFuncInternal, hReceiver, ETAL_SEEK_INTCB_DELAY)) != ETAL_RET_SUCCESS)
	                    {
	                        ret = ETAL_RET_ERROR;
	                    }
	                }
	                }
	                else
	                {
	                    ret = ETAL_RET_ERROR;
	                }
	            }
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

	            if(ret != ETAL_RET_SUCCESS)
	            {
	                /* Stop seek and send event */
	                ETAL_stop_and_send_event(hReceiver, seekCfgp, ETAL_SEEK_ERROR_ON_START);
	            }
	        }
		}
	}

	return ret;
}


/***************************
 *
 * ETAL_seek_stop_internal
 *
 **************************/
ETAL_STATUS ETAL_seek_stop_internal(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    tU32 device_list;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tU32 initial_frequency;
	etalFrequencyBandInfoTy band_info;
#endif
    etalReceiverStatusTy *recvp = NULL;
    etalSeekConfigTy *seekCfgp = NULL;
    EtalSeekStatus *seekStatusp = NULL;
    tBool bandBorderCrossed;


    if (!ETAL_receiverIsValidHandle(hReceiver))
    {
        ret = ETAL_RET_INVALID_HANDLE;
    }
    else
    {
        if (ETAL_checkSeekStopParameters(hReceiver, terminationMode) != ETAL_RET_SUCCESS)
        {
            ret = ETAL_RET_PARAMETER_ERR;
        }
        else
        {
            recvp = ETAL_receiverGet(hReceiver);
            seekCfgp = &(recvp->seekCfg);
            seekStatusp = &(seekCfgp->autoSeekStatus);

            seekCfgp->terminationMode = terminationMode;

            /* commandTune is fine as long as DAB is not supported, review later */
            device_list = ETAL_cmdRoutingCheck(hReceiver, commandTune);

            if (ETAL_CMDROUTE_TO_STAR(device_list))
            {
                if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeek))
                {
                     switch (ETAL_receiverGetStandard(hReceiver))
                     {
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
                         case ETAL_BCAST_STD_DAB:
                        	 if (ETAL_intCbIsRegistered(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver) == TRUE)
                        	 {
                        		 (LINT_IGNORE_RET)ETAL_intCbDeregister(callAtDABAutonotification, ETAL_SeekIntCbFuncDabSync, hReceiver);
                        	 }

                        	 if (ETAL_cmdSeekEnd_CMOST(hReceiver, seekCfgp->exitSeekAction) == OSAL_OK)
                        	 {
                        		 /* Tune initial frequency if requested */
                        		 if((seekCfgp->terminationMode == initialFrequency) && (seekCfgp->startFrequency != ETAL_INVALID_FREQUENCY))
                        		 {
                                	 if (OSAL_OK != ETAL_receiverGetBandInfo(hReceiver,&band_info))
                                	 {
                                		 seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
    									 ret = ETAL_RET_ERROR;
                                	 }
                                	 else
                                	 {
            							 initial_frequency = DABMW_GetNextFrequencyFromFreq(seekCfgp->startFrequency,DABMW_TranslateEtalBandToDabmwBand(band_info.band), (seekCfgp->direction == cmdDirectionUp) ? FALSE : TRUE);
            							 if (ETAL_receiverGetFrequency(hReceiver) != initial_frequency)
            							 {
            								 if(ETAL_tuneReceiverInternal(hReceiver, seekCfgp->startFrequency, cmdTuneNormalResponse) == ETAL_RET_ERROR)
            								 {
            									 seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
            									 ret = ETAL_RET_ERROR;
            								 }
            								 else
            								 {
            									 seekStatusp->m_status = ETAL_SEEK_FINISHED;
            								 }
            							 }
            							 else
            							 {
        									 seekStatusp->m_status = ETAL_SEEK_FINISHED;
            							 }
                        			 }
                        		 }
                        		 else
                        		 {
                        			 seekStatusp->m_status = ETAL_SEEK_FINISHED;
                        		 }
                        	 }
                        	 else
                        	 {
                        		 seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
                                 ret = ETAL_RET_ERROR;
                        	 }

                        	 /* Stop seek and send event */
                        	 ETAL_stop_and_send_event(hReceiver, seekCfgp, seekStatusp->m_status);

                        	 /* Change seek state */
                        	 seekCfgp->state = ETAL_DABSEEK_NULL;
                             break;
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

                         default:
                             if (ETAL_cmdSeekEnd_CMOST(hReceiver, seekCfgp->exitSeekAction) == OSAL_OK)
                             {
                                 if (ETAL_cmdSeekGetStatus_CMOST(hReceiver, &(seekStatusp->m_frequencyFound), NULL,
                                     &bandBorderCrossed, &(seekStatusp->m_frequency), &(seekStatusp->m_quality)) == OSAL_OK)
                                 {
                                     /* Tune initial frequency if requested */
                                     if(seekCfgp->terminationMode == initialFrequency)
                                     {
                                         if ((seekCfgp->startFrequency != ETAL_INVALID_FREQUENCY) &&
                                         (ETAL_receiverGetFrequency(hReceiver) != seekCfgp->startFrequency))
                                         {
                                             if(ETAL_tuneReceiverInternal(hReceiver, seekCfgp->startFrequency, cmdTuneNormalResponse) == ETAL_RET_ERROR)
                            				 {
                            					 seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
                                                 ret = ETAL_RET_ERROR;
                            				 }
                            				 else
                            				 {
                            					 seekStatusp->m_status = ETAL_SEEK_FINISHED;
                            				 }
                                         }
                                         else
                                         {
                                             seekStatusp->m_status = ETAL_SEEK_FINISHED;
                                         }
                                     }
                                     else
                                     {
                                         if ((seekStatusp->m_frequency != 0) && (seekStatusp->m_frequency != ETAL_INVALID_FREQUENCY))
                                         {
                                             /* Update the current frequency */
                                             ETAL_receiverSetFrequency(hReceiver, seekStatusp->m_frequency, TRUE);
                                         }
                                         seekStatusp->m_status = ETAL_SEEK_FINISHED;
                                     }
                                 }
                                 else
                                 {
                                     seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
                                     ret = ETAL_RET_ERROR;
                                 }
                             }
                             else
                             {
                                 seekStatusp->m_status = ETAL_SEEK_ERROR_ON_STOP;
                                 ret = ETAL_RET_ERROR;
                             }

                             /* Stop seek and send event */
                             ETAL_stop_and_send_event(hReceiver, seekCfgp, seekStatusp->m_status);
                             break;
                     }
                }
            }
        }
    }
    return ret;
}

#endif // #if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
#endif // defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)


