//!
//!  \file 		etaltmlapi_radiotext.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, Radio text access
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)

#include "dabmw_import.h"
#include "rds_data.h"
#include "rds_strategy.h"

static tVoid ETALTML_RDSStrategyPeriodicFunc(ETAL_HANDLE hReceiver);
static tVoid ETALTML_RdsSeekStartTask(tVoid);
static tVoid ETALTML_RdsSeekStopTask(tVoid);
static tVoid ETALTML_RdsSeekThreadEntry(tPVoid dummy);
static tBool ETALTML_RdsSeekMainLoop(ETAL_HANDLE hReceiver);
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_RdsSeekWaitingTask(tSInt stacd, tPVoid thread_param);
#else
static tVoid ETALTML_RdsSeekWaitingTask (tPVoid thread_param);
#endif //CONFIG_HOST_OS_TKERNEL

static tVoid ETALTML_Report_RdsSeek_Info(ETAL_HANDLE hReceiver, EtalSeekStatus* extAutoSeekStatus);

/***************************
 *
 * Local variables
 *
 **************************/
static OSAL_tEventHandle v_ETALTML_RdsSeek_TaskEventHandler;
static OSAL_tThreadID v_ETALTML_RdsSeek_ThreadId;

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
static tBool v_serviceFollowingHasBeenDisabled = false;
#endif //CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

tVoid ETALTML_Report_RdsSeek_Info(ETAL_HANDLE hReceiver, EtalSeekStatus* extAutoSeekStatus)
{
	etalReceiverStatusTy *recvp = NULL;
	etalRdsSeekConfigTy *RdsSeekCfgp = NULL;
	etalSeekStatusTy vl_status;
	// 
	
	recvp = ETAL_receiverGet(hReceiver);
	RdsSeekCfgp = &(recvp->RdsSeekCfg);

	
	vl_status = extAutoSeekStatus->m_status;
	*extAutoSeekStatus = RdsSeekCfgp->seekStatus;
	extAutoSeekStatus->m_status = vl_status;
	 
	printf("ETALTML_Report_RdsSeek_Info, status %d\n", extAutoSeekStatus->m_status);
	
    switch(extAutoSeekStatus->m_status)
    {
        case ETAL_SEEK_STARTED :
        case ETAL_SEEK_ERROR :
        case ETAL_SEEK_ERROR_ON_START :
        case ETAL_SEEK_ERROR_ON_STOP :
  //          if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalRequestInProgress))
            {
                /* Send ETAL_INFO_SEEK event */
                ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEEK, (tVoid *)extAutoSeekStatus,  sizeof(EtalSeekStatus));
            }

            break;

        case ETAL_SEEK_RESULT :
			// for now consider this is not found when result
			extAutoSeekStatus->m_frequencyFound = FALSE;
	
  //      	if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalRequestInProgress))
            {
                /* Send ETAL_INFO_SEEK event */
                ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEEK, (tVoid *)extAutoSeekStatus,  sizeof(EtalSeekStatus));
            }

            break;

        case ETAL_SEEK_FINISHED :

  //          if (true == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalRequestInProgress))
            {
                /* Send ETAL_INFO_SEEK event */
                ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEEK, (tVoid *)extAutoSeekStatus,  sizeof(EtalSeekStatus));
            }

             /* Internal callback : callAtRdsEndOfSeek */
            ETAL_intCbScheduleCallbacks(hReceiver, callAtEndOfRdsSeek, (tVoid *)extAutoSeekStatus, sizeof(EtalSeekStatus));


			// stop the Rds Seek
			ETAL_receiverSetSpecial(hReceiver, cmdSpecialRdsSeek, cmdActionStop);
   
             if (ETAL_intCbIsRegisteredPeriodic(ETALTML_SeekStatusPeriodicFuncInternal, hReceiver) == TRUE)
             {
                 printf("ETALTML_SeekStatusPeriodicFuncInternal not deregistered\n");
                 ASSERT_ON_DEBUGGING(0);
             }

             break;

        default:
            break;
    }
    return;
}


/***************************
 *
 * ETALTML_get_TATPStatus
 * paramters:   	hReceiver
				DABMW_storageStatusEnumTy *pPIStatus
				 tU32 *pCurrentPI;
				 tU32 * pLastPI;
				 tU32 * pBackupPI
 * return value:  ETAL_STATUS, error code
 **************************/
ETAL_STATUS ETALTML_get_PIStatus(ETAL_HANDLE hReceiver, DABMW_storageStatusEnumTy *pPIStatus, tU32 *pCurrentPI, tU32 * pLastPI, tU32 * pBackupPI)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (pPIStatus == NULL || pCurrentPI == NULL || pLastPI == NULL || pBackupPI == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	if (!ETAL_receiverIsValidHandle(hReceiver) ||
		!ETAL_receiverIsValidRDSHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
		tSInt slot_s;

		slot_s = ETAL_receiverGetRDSSlot(hReceiver);
		if (slot_s == -1)
		{
			ret = ETAL_RET_NO_DATA;
		}
		else
		{
			// rds related Data
 			(LINT_IGNORE_RET)DABMW_RdsForceGetPiData (slot_s, pPIStatus, pCurrentPI,  pLastPI,  pBackupPI);
		}
#else
		ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
	}

	return ret;	
}



/***************************
 *
 * ETALTML_get_TATPStatus
 * paramters:   	hReceiver
				**ppTATPData to get RDS TATP data
 * return value:  ETAL_STATUS, error code
 **************************/
ETAL_STATUS ETALTML_get_TATPStatus(ETAL_HANDLE hReceiver, tVoid **ppTATPData)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (ppTATPData == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}


	if (!ETAL_receiverIsValidHandle(hReceiver) ||
		!ETAL_receiverIsValidRDSHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
		tSInt slot_s;

		slot_s = ETAL_receiverGetRDSSlot(hReceiver);
		if (slot_s == -1)
		{
			ret = ETAL_RET_NO_DATA;
		}
		else
		{
			// rds related Data
			(LINT_IGNORE_RET)DABMW_RdsForceGetTPTAData(slot_s, ppTATPData);
		}

#else
		ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
	}

	return ret;	
}


/***************************
 *
 * ETALTML_get_RDSStationFlag
 * paramters:   	hReceiver
				*bRDSStation to output RDS station status
 * return value:  ETAL_STATUS, error code
 **************************/
ETAL_STATUS ETALTML_get_RDSStationFlag(ETAL_HANDLE hReceiver,  tBool * bRDSStation)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (bRDSStation == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	//do not lock the receiver, this should be done prior to that 
//	if ((ret = ETAL_receiverGetLock(hReceiver)) != ETAL_RET_SUCCESS)
//	{
//		return ETAL_RET_ERROR;
//	}
//
	if (!ETAL_receiverIsValidHandle(hReceiver) ||
		!ETAL_receiverIsValidRDSHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
		tSInt slot_s;

		slot_s = ETAL_receiverGetRDSSlot(hReceiver);
		if (slot_s == -1)
		{
			//ETAL_receiverReleaseLock(hReceiver);
			ret =  ETAL_RET_NO_DATA;
		}
		else
		{
			// rds related Data
			(LINT_IGNORE_RET)DABMW_RdsForceGetF_RDSStation(slot_s, bRDSStation);
		}

#else
		ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
	}
	//ETAL_receiverReleaseLock(hReceiver);
	return ret;	
}


/***************************
 *
 * ETAL_RDSRawPeriodicFunc
 * paramters:   	hReceiver
 * return value:  None
 **************************/
tVoid ETALTML_RDSStrategyPeriodicFunc(ETAL_HANDLE hReceiver)
{

	if (ETAL_handleIsReceiver(hReceiver) == true)
	 {

		/* is hReceiver valid RDS handle */
		if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
		{
			return;
		}

	   	// safer would be to lock

	   	if (ETAL_receiverGetLock(hReceiver)== ETAL_RET_SUCCESS)
   	   	{
			ETALTML_RDS_Strategy_Main(hReceiver);

			// release receiver lock
			ETAL_receiverReleaseLock(hReceiver);
				
		}
		else
		{
			// lock error
		}
	}
		
}


/***************************
etaltml_RDS_AF
 
At this moment,  hReceiverB has been ignored, 
	if (recA = recB) and VPA mode is off
		(1)	normal AF management
	elseif (recA != recB)
		(2)	main + background AF management
	else if (recA = recB) and VAP mod is on
		(3)	splite VPA AF management.
		
paramters:   	hReceiverA, hReceiverB
			AFOn: TRUE - switch rds strategy AF on
				   FALSE - switch rds strategy AF off
return value: ETAL_STATUS

*******************************/
ETAL_STATUS etaltml_RDS_AF(ETAL_HANDLE hReceiverA, ETAL_HANDLE hReceiverB, tBool AFOn)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;
	RDS_FunctionInfoTy RdsFuncPara;
	
	if (ETAL_statusGetLock())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	//Did not check hReceiverB at this moment
	if (ETAL_receiverIsValidRDSHandle(hReceiverA) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
       	std = ETAL_receiverGetStandard(hReceiverA);
		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)

				/* deregister periodic function if already registered */
				if (ETAL_intCbIsRegisteredPeriodic(ETALTML_RDSStrategyPeriodicFunc, hReceiverA) == TRUE)
				{
					ret = ETAL_intCbDeregisterPeriodic(ETALTML_RDSStrategyPeriodicFunc, hReceiverA);
					if (ret != ETAL_RET_SUCCESS)
					{
						return ret;
					}
				}
				
				ret = ETAL_intCbRegisterPeriodic(ETALTML_RDSStrategyPeriodicFunc, hReceiverA, 10);
				if (ret == ETAL_RET_SUCCESS)
				{
					OSAL_pvMemorySet((tPVoid)&RdsFuncPara, 0x00, sizeof(RDS_FunctionInfoTy));

					RdsFuncPara.F_AFEnable   = AFOn;
					RdsFuncPara.F_TAEnable   = FALSE;
					RdsFuncPara.F_EONEnable = FALSE;
					RdsFuncPara.F_REGEnable = FALSE;

					ret = ETALTML_RDS_Strategy_Init(hReceiverA, RdsFuncPara);
					
					if(AFOn)
					{
	                			ETAL_receiverSetSpecial(hReceiverA, cmdSpecialRDSStrategy, cmdActionStart);
					}
					else
					{
				                ETAL_receiverSetSpecial(hReceiverA, cmdSpecialRDSStrategy, cmdActionStop);
					}
				}
#endif

				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();
	return ret;
}


/***************************
etaltml_RDS_TA
 
paramters:   	hReceiver,
			TAOn: TRUE - switch rds strategy TA on
				   FALSE - switch rds strategy TA off
return value: ETAL_STATUS
*******************************/
ETAL_STATUS etaltml_RDS_TA(ETAL_HANDLE hReceiver, tBool TAOn)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;
	
	if (ETAL_statusGetLock())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
       	std = ETAL_receiverGetStandard(hReceiver);
		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
				if(ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_AF))
				{
					ETALTML_RDS_Strategy_SetFunctionStatus(hReceiver, RDS_FUNC_TA, TAOn);
				}
				else
				{
					ret = ETAL_RET_ERROR;
				}
#endif
				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();
	
	return ret;
}


#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
/***************************
etaltml_RDS_EON
 
paramters:   	hReceiver,
			EONOn: TRUE - switch rds strategy EON on
				   FALSE - switch rds strategy EON off
return value: ETAL_STATUS
*******************************/
ETAL_STATUS etaltml_RDS_EON(ETAL_HANDLE hReceiver, tBool EONOn)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;
	
	if (ETAL_statusGetLock())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
       	std = ETAL_receiverGetStandard(hReceiver);
		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
				if(ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_AF))
				{
					ETALTML_RDS_Strategy_SetFunctionStatus(hReceiver, RDS_FUNC_EON, EONOn);
				}
				else
				{
					ret = ETAL_RET_ERROR;
				}
#endif
				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();
	
	return ret;
}	
#endif


/***************************
etaltml_RDS_REG
 
paramters:   	hReceiver,
			REGOn: TRUE - switch rds strategy REG on
				   FALSE - switch rds strategy REG off
return value: ETAL_STATUS
*******************************/
ETAL_STATUS etaltml_RDS_REG(ETAL_HANDLE hReceiver, tBool REGOn)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;
	
	if (ETAL_statusGetLock())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
       	std = ETAL_receiverGetStandard(hReceiver);
		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
				if(ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_AF))
				{
					ETALTML_RDS_Strategy_SetFunctionStatus(hReceiver, RDS_FUNC_REG, REGOn);
				}
				else
				{
					ret = ETAL_RET_ERROR;
				}
#endif
				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();
	
	return ret;
}


/***************************
etaltml_RDS_AFSearch_start
descrption:  start rds AF search 
paramters:   	hReceiver,
			afSearchData: (PI, AFNum, AF list)
return value: ETAL_STATUS
*******************************/
ETAL_STATUS etaltml_RDS_AFSearch_start(ETAL_HANDLE hReceiver, EtalRDSAFSearchData afSearchData)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;
	
	if (ETAL_statusGetLock())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
       	std = ETAL_receiverGetStandard(hReceiver);
		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
				if (afSearchData.m_AFListLen <= 0) 
				{
					ret = ETAL_RET_PARAMETER_ERR;
				}
				else
				{
					ret = ETALTML_RDS_Strategy_AFSearch_Start(hReceiver, afSearchData);
				}
#endif
				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();
	
	return ret;
}



/***************************
etaltml_RDS_AFSearch_stop
descrption:  stop rds AF search 
paramters:   	hReceiver,
return value: ETAL_STATUS
*******************************/
ETAL_STATUS etaltml_RDS_AFSearch_stop(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;
	
	if (ETAL_statusGetLock())
	{
		return ETAL_RET_NOT_INITIALIZED;
	}

	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
       	std = ETAL_receiverGetStandard(hReceiver);
		switch (std)
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			case ETAL_BCAST_STD_FM:
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			case ETAL_BCAST_STD_HD_FM:
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
				ret = ETALTML_RDS_Strategy_AFSearch_Stop(hReceiver);
#endif
				break;

			default:
				ret = ETAL_RET_NOT_IMPLEMENTED;
				break;
		}
	}

	ETAL_statusReleaseLock();
	
	return ret;
}

////////// RDS SEEK INTERFACES

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)


/***************************
 *
 * ETALTML_RdsSeekIntCbFunc
 *
 **************************/
static tVoid ETALTML_RdsSeekIntCbFunc(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context)
{
     ETAL_HANDLE hReceiver;
    EtalSeekStatus *pl_autoSeekStatus = (EtalSeekStatus *) param;
    etalReceiverStatusTy *recvp = NULL;
    etalRdsSeekConfigTy *RdsSeekCfgp = NULL;
 
 	printf("ETALTML_RdsSeekIntCbFunc, receiver = %d\n", hGeneric);

	if (ETAL_handleIsReceiver(hGeneric))
	{
	    hReceiver = hGeneric;

		// we should not lock the receiver
		// because receiver is already assumed lock by caller
		

		    recvp = ETAL_receiverGet(hReceiver);
		    RdsSeekCfgp = &(recvp->RdsSeekCfg);
			
			printf("ETALTML_RdsSeekIntCbFunc, receiver = %d, status = %d\n", hGeneric, pl_autoSeekStatus->m_status);

		    if (pl_autoSeekStatus != NULL)
		    {
		    	RdsSeekCfgp->seekStatus = *pl_autoSeekStatus;
								
		        switch(pl_autoSeekStatus->m_status)
		        {
		            case ETAL_SEEK_RESULT:					
		                if(pl_autoSeekStatus->m_frequencyFound == FALSE)
		                {
		                    if(pl_autoSeekStatus->m_fullCycleReached == TRUE)
		                    {
		                        /* Change RDS Seek state */
								// RDS SEEK should be stopped now
								// 
		                        RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_FINISHED;
								RdsSeekCfgp->autoSeek_complete = TRUE;
		                    }
		                    else
		                    {
		                        /* Seek is ongoing */
								
								// report it is done
								ETALTML_Report_RdsSeek_Info(hReceiver, &(RdsSeekCfgp->seekStatus));
		                    }
		                }
		                else
		                {
							/* Change RDS Seek state */
							// frequency found
							// 
							RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAITRDS;
							ETALTML_RDS_Strategy_RDSTimer_Set(hReceiver, RDS_TIMER_10MS, 150);

							if(pl_autoSeekStatus->m_fullCycleReached == TRUE)
							{
		                       	RdsSeekCfgp->autoSeek_complete = TRUE;
		                    }
		                }
	                break;

	            case ETAL_SEEK_FINISHED:
					// seek has been stopped
					/* Change RDS Seek state */
					// RDS SEEK should be stopped now
					// 
		            RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_FINISHED;
					RdsSeekCfgp->autoSeek_complete = TRUE;
	                break;

	            default:

	                break;
		        }
 	   	}
		else
		{
			// receiver lock error
			ASSERT_ON_DEBUGGING(0);
		}
	}
	else
	{
	    ASSERT_ON_DEBUGGING(0);
	}
	return;
}

/***************************
 *
 * ETALTML_RdsSeekTaskInit
 *
 **************************/
tSInt ETALTML_RdsSeekTaskInit (tVoid)
{
    OSAL_trThreadAttribute attr0;
    tSInt ret = OSAL_OK;

    if (OSAL_s32EventCreate((tCString)"RdsSeek_EventHandler", &v_ETALTML_RdsSeek_TaskEventHandler) == OSAL_OK)
    {
        /* Initialize the thread */
        attr0.szName = (tChar *)ETALTML_RDS_SEEK_THREAD_NAME;
        attr0.u32Priority = ETALTML_RDS_SEEK_THREAD_PRIORITY;
        attr0.s32StackSize = ETALTML_RDS_SEEK_STACK_SIZE;
        attr0.pfEntry = ETALTML_RdsSeekWaitingTask;
        attr0.pvArg = NULL;

        if ((v_ETALTML_RdsSeek_ThreadId = OSAL_ThreadSpawn(&attr0)) == OSAL_ERROR)
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
 * ETALTML_RdsSeekTTaskDeinit
 *
 **************************/
tSInt ETALTML_RdsSeekTaskDeinit (tVoid)
{
    tSInt ret = OSAL_OK;

	// Stop the Seek task
	ETALTML_RdsSeekStopTask();
	
	/* Post the kill event */
	(LINT_IGNORE_RET) OSAL_s32EventPost (v_ETALTML_RdsSeek_TaskEventHandler,
                    ETALTML_LEARN_EVENT_KILL_FLAG,
                    OSAL_EN_EVENTMASK_OR);
	OSAL_s32ThreadWait(10);

	/* Delete the thread */
    if (OSAL_s32ThreadDelete(v_ETALTML_RdsSeek_ThreadId) != OSAL_OK)
    {
        ret = OSAL_ERROR;
    }
	else
	{
		v_ETALTML_RdsSeek_ThreadId = (OSAL_tThreadID)0;
	}

    return ret;
}

/***************************
 *
 * ETALTML_LearnStartTask
 *
 **************************/
static tVoid ETALTML_RdsSeekStartTask(tVoid)
{
    /* This is done by posting the awake event */
    (LINT_IGNORE_RET) OSAL_s32EventPost (v_ETALTML_RdsSeek_TaskEventHandler,
                    ETALTML_LEARN_EVENT_WAKEUP_FLAG,
                    OSAL_EN_EVENTMASK_OR);
    return;
}

/***************************
 *
 * ETALTML_LearnStopTask
 *
 **************************/
static tVoid ETALTML_RdsSeekStopTask(tVoid)
{
    /* this is done by posting the sleep event */
    (LINT_IGNORE_RET) OSAL_s32EventPost (v_ETALTML_RdsSeek_TaskEventHandler,
                    ETALTML_LEARN_EVENT_SLEEP_FLAG,
                    OSAL_EN_EVENTMASK_OR);
    return;
}

/***************************
 *
 * ETAL_learn_TaskClearEventFlag
 *
 **************************/
static tVoid ETAL_RdsSeek_TaskClearEventFlag(tU32 eventFlag)
{
	/* Clear old event if any (this can happen after a stop) */
    (LINT_IGNORE_RET) OSAL_s32EventPost(v_ETALTML_RdsSeek_TaskEventHandler,
                           ~eventFlag, OSAL_EN_EVENTMASK_AND);
    return;
}

/***************************
 *
 * ETALTML_RdsSeekWaitingTask
 *
 **************************/
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_RdsSeekWaitingTask(tSInt stacd, tPVoid thread_param)
#else
static tVoid ETALTML_RdsSeekWaitingTask (tPVoid thread_param)
#endif //CONFIG_HOST_OS_TKERNEL
{
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETALTML_LEARN_NO_EVENT_FLAG;

    while (TRUE)
    {

        // Wait here the auto wake up event
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)
        ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_USER_4, "ETALTML_RdsSeekWaitingTask : waiting for learn activation\n");
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)


        vl_res = OSAL_s32EventWait (v_ETALTML_RdsSeek_TaskEventHandler,
                                     ETALTML_RDS_SEEK_EVENT_WAIT_WAKEUP_MASK,
                                     OSAL_EN_EVENTMASK_OR,
                                     OSAL_C_TIMEOUT_FOREVER,
                                     &vl_resEvent);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)
        ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_USER_4, "ETALTML_RdsSeekWaitingTask : learn awaked, event = 0x%x, vl_res = %d\n",
                vl_resEvent, vl_res);
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)

        if ((vl_resEvent == ETALTML_RDS_SEEK_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
        {
        	/* This is a timeout : no even received */
        }
        /* learn event trigger learn event processing
        * else normal handling ie if res = OSAL_ERROR => this is a timeout, so
        * normal processing or event return flag contains EVENT_CMD.. so normal processing */
        else if (ETALTML_RDS_SEEK_EVENT_KILL_FLAG == (ETALTML_RDS_SEEK_EVENT_KILL_FLAG & vl_resEvent))
        {
        	/* Kill the learn */
        	break;
        }
        /* This is a LEARN EVENT WAKE UP call the LEARN event handler */
        else   if (ETALTML_RDS_SEEK_EVENT_WAKEUP_FLAG == (ETALTML_RDS_SEEK_EVENT_WAKEUP_FLAG & vl_resEvent))
        {
        	/*
        	 * if the learn stop was called without a learn start, the SLEEP flag
        	 * is set and the ETALTML_LearnStartTask has no effect
        	 * so ensure the SLEEP flag is clear
        	 */
        	ETAL_RdsSeek_TaskClearEventFlag(ETALTML_RDS_SEEK_EVENT_ALL_FLAG);
        	ETALTML_RdsSeekThreadEntry(thread_param);
        	/* Clear the received event */
        }
        else
        {
        	/* a non expected event : just clear all */
        	ETAL_RdsSeek_TaskClearEventFlag(ETALTML_RDS_SEEK_EVENT_ALL_FLAG);
        }
    }

    /* Close the events */
#ifndef OSAL_EVENT_SKIP_NAMES
    if (OSAL_s32EventClose(v_ETALTML_RdsSeek_TaskEventHandler) != OSAL_OK)
    {
        vl_res = OSAL_ERROR;
    }

	OSAL_s32ThreadWait(100);

    if (OSAL_s32EventDelete((tCString)"RdsSeek_EventHandler") != OSAL_OK)
    {
        vl_res = OSAL_ERROR;
    }
#else
    if (OSAL_s32EventFree(v_ETALTML_RdsSeek_TaskEventHandler) != OSAL_OK)
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
static tVoid ETALTML_RdsSeekThreadEntry(tPVoid dummy)
{
    ETAL_HANDLE hReceiver;
    ETAL_HINDEX receiver_index;
    tSInt vl_res;
    OSAL_tEventMask vl_resEvent = ETALTML_LEARN_NO_EVENT_FLAG;
    tU32 i;
	tBool vl_completed = FALSE;

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)
    ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_USER_4, "ETALTML_RdsSeekThreadEntry :  learn awaked\n");
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)

    while (FALSE == vl_completed)
    {
       vl_res = OSAL_s32EventWait(v_ETALTML_RdsSeek_TaskEventHandler,
               ETALTML_RDS_SEEK_EVENT_WAIT_MASK,
               OSAL_EN_EVENTMASK_OR,
               ETALTML_RDS_SEEK_EVENT_WAIT_TIME_MS,
               &vl_resEvent);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)
        ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_USER_4, "ETALTML_RdsSeekThreadEntry :  awaked, event = 0x%x, vl_res = %d\n",
            vl_resEvent, vl_res);
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_USER_4)

        /* If this is for sleeping, leave this */
        if (ETALTML_RDS_SEEK_EVENT_SLEEP_FLAG == (vl_resEvent & ETALTML_RDS_SEEK_EVENT_SLEEP_FLAG))
        {
            ETAL_RdsSeek_TaskClearEventFlag(ETALTML_RDS_SEEK_EVENT_SLEEP_FLAG);
            break;
        }
        else if ((vl_resEvent == ETALTML_RDS_SEEK_NO_EVENT_FLAG) ||
            (vl_res == OSAL_ERROR_TIMEOUT_EXPIRED))
        {
            /* Receiver specific actions, lock one receiver at a time */
            for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
            {
                receiver_index = (ETAL_HINDEX)i;
                hReceiver = ETAL_handleMakeReceiver(receiver_index);

                if (ETAL_receiverIsValidHandle(hReceiver))
                {
	                if (TRUE == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialRdsSeek))
	                {
	                    if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
	                    {
	                        ASSERT_ON_DEBUGGING(0);
	                        continue;
	                    }

						// for now, assume only 1 receiver in progress at a time
	                    vl_completed =  ETALTML_RdsSeekMainLoop(hReceiver);

	                    ETAL_receiverReleaseLock(hReceiver);
	                }
                }
            }
        }
        else
        {
            /* Manage RDS SEEK events */
        }
    }

    return;
}

/***************************
 *
 * ETALTML_LearnMainLoop
 *
 **************************/
static tBool ETALTML_RdsSeekMainLoop(ETAL_HANDLE hReceiver)
{
    etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
    etalRdsSeekConfigTy *RdsSeekCfgp = &(recvp->RdsSeekCfg);
    ETAL_STATUS ret;

	DABMW_storageStatusEnumTy PIStatus;
	tU32 currentPI;
	tU32 lastPI;
	tU32 backupPI;
	EtalRDSData rdsData;
	tBool bRDSStation;
	tSInt slot;
	tBool vl_complete = FALSE;
	EtalSeekStatus vl_RdsSeekStatus;

    if (ETAL_receiverIsValidHandle(hReceiver))
    {
        if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialRdsSeek))
        {

#if defined(CONFIG_TRACE_CLASS_RDS_STRATEGY) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	      ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_USER_2, "ETALTML_RdsSeekMainLoop, receiver = %d, state = %d \n", hReceiver, RdsSeekCfgp->state);
#endif
			
	        switch(RdsSeekCfgp->state)
	        {
		        case ETALTML_RDS_AUTO_SEEK_IDLE:
					vl_complete = TRUE;
					break;
					
	            case ETALTML_RDS_AUTO_SEEK_FINISHED:
					// This means this is completed
					// 
					// we should leave the thread 
					// processing is complete
					// 
//					if (FALSE == RdsSeekCfgp->autoSeek_complete)
					{
						// the autoseek is done : deregister and stop it
						// Stop the AutoSeek this is complete

						// deregister notifications
						//
						(LINT_IGNORE_RET)ETAL_intCbDeregister(callAtEndOfSeek, ETALTML_RdsSeekIntCbFunc, hReceiver);
						(LINT_IGNORE_RET)ETAL_intCbDeregister(callAtSeekStatus, ETALTML_RdsSeekIntCbFunc, hReceiver);

						ETALTML_seek_stop_internal(hReceiver, lastFrequency);
	
					}

					// report it is done
					vl_RdsSeekStatus.m_status = ETAL_SEEK_FINISHED;
                   	ETALTML_Report_RdsSeek_Info(hReceiver, &vl_RdsSeekStatus);

					// clear RDS info
						
                    ETALTML_RDS_Strategy_Data_Init(hReceiver);
                    ETALTML_RDSresetData(hReceiver);  //  CLEAR RDS DATA, reset
                    
					vl_complete = TRUE;

					ETAL_receiverSetSpecial(hReceiver, cmdSpecialRdsSeek, cmdActionStop);

					// move to IDLE
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	                    ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_FINISHED, switch to ETALTML_RDS_AUTO_SEEK_IDLE.\r\n");
#endif

					
	                break;
				case ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK:
					// Seek algo is working just wait
					//
					break;

	             case ETALTML_RDS_AUTO_SEEK_WAITRDS:
	  
		            bRDSStation = FALSE;
	                (LINT_IGNORE_RET) ETALTML_get_RDSStationFlag(hReceiver, &bRDSStation);
					
	                if (bRDSStation)
	                { 
	                	RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAITPI;

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	                    ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_EXT_AUTO_SEEK_WAITRDS, switch to ETALTML_EXT_AUTO_SEEK_WAITPI.\r\n");
#endif

						ETALTML_RDS_Strategy_RDSTimer_Set(hReceiver, RDS_TIMER_10MS, 100);

		                /*	reset the PI seek Timer, */
#ifdef ETAL_RDS_IMPORT
		                slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
		                slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
		                ETALTML_RDS_Strategy_ResetPISeekTimer(slot, TRUE);

	                }
	                else if(ETALTML_RDS_Strategy_RDSTimer_Get(hReceiver, RDS_TIMER_10MS) == 0)
	                {
	                    //no RDS Info received... 
	                    // Seek continue to the next freq
	                    //
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	                    ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_EXT_AUTO_SEEK_WAITRDS, time out.\r\n");
#endif

	                    /* Send event and notify */
						// We should notify something else may be : dedicated to SEEK ??
						// 
						vl_RdsSeekStatus.m_status = ETAL_SEEK_RESULT;
                   		ETALTML_Report_RdsSeek_Info(hReceiver, &vl_RdsSeekStatus);
						RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK;

						// Request seek to continue 
	                    ETALTML_seek_continue_internal(hReceiver);
	                }

	                break;

	            case ETALTML_RDS_AUTO_SEEK_WAITPI:
	                PIStatus  = DABMW_STORAGE_STATUS_IS_EMPTY;
	                currentPI = 0;
	                lastPI 	= 0;
	                backupPI = 0;

	                ret = ETALTML_get_PIStatus(hReceiver, &PIStatus, &currentPI, & lastPI, &backupPI);
	                if( ( ETAL_RET_SUCCESS == ret) &&
	                    ( PIStatus != DABMW_STORAGE_STATUS_IS_EMPTY)  &&
	                    ( currentPI != 0))
	                {
	                    if(ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_PTY))
	                    {
	                    	RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAITPTY;
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	                        ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITPI, switch to ETALTML_EXT_AUTO_SEEK_WAITPTY.\r\n");
#endif

	                        ETALTML_RDS_Strategy_RDSTimer_Set(hReceiver, RDS_TIMER_FUNC_PTY_SELECT, 100);
	                        ETALTML_RDS_Strategy_RDSTimer_Set(hReceiver, RDS_TIMER_10MS, 40);
		                }
		                else if(ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_TA))
		                {
		                	RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAITTP;
								
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
		                    ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITPI, switch to ETALTML_RDS_AUTO_SEEK_WAITTP.\r\n");
#endif

							ETALTML_RDS_Strategy_RDSTimer_Set(hReceiver, RDS_TIMER_10MS, 40);
		                }
		                else if (ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_PISeek))
		                {
		                	if(ETALTML_RDS_Strategy_IsPISeekOk(hReceiver))
		                    {

								// Seek ended 
								// 
								RdsSeekCfgp->state  = ETALTML_RDS_AUTO_SEEK_FINISHED;
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
								ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITPI, PI Station Found, switch to ETALTML_RDS_AUTO_SEEK_FINISHED.\r\n");
#endif

				            }
		                	else
		                    {
#if 0
								// PI is not yet the good one 
								// should not we wait ??
								
		                    	/* Send event and notify */
								vl_RdsSeekStatus.m_status = ETAL_SEEK_RESULT;
                   				ETALTML_Report_RdsSeek_Info(hReceiver, &vl_RdsSeekStatus);
						
								// We should notify something else may be : dedicated to SEEK ??
								// 
								RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK;
								
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
								ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITPI, switch to ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK.\r\n");
#endif

								// Request seek to continue 
			                    ETALTML_seek_continue_internal(hReceiver);
#endif
		                	}
		               	}
		              	else
		               	{

					   		// Why are we here ??
		                    // nothing set for RDS seek
		                    // handle it has a normal seek
		                    // 
		                    // 
		                    /* Send event and notify */
							vl_RdsSeekStatus.m_status = ETAL_SEEK_FINISHED;
   						
							// We should notify something else may be : dedicated to SEEK ??
							// 
							RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_FINISHED;
							
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
							ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITPI, Rds Station Found, switch to ETAL_SEEK_FINISHED.\r\n");
#endif

	                    }
	                }

					if((RdsSeekCfgp->state == ETALTML_RDS_AUTO_SEEK_WAITPI)
						&& (ETALTML_RDS_Strategy_RDSTimer_Get(hReceiver, RDS_TIMER_10MS) == 0))
	                {

	 	                /* Send event and notify */
						vl_RdsSeekStatus.m_status = ETAL_SEEK_RESULT;
                   		ETALTML_Report_RdsSeek_Info(hReceiver, &vl_RdsSeekStatus);
						
						// We should notify something else may be : dedicated to SEEK ??
						// 
						RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK;
						
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
						ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: Timeout, ETALTML_RDS_AUTO_SEEK_WAITPI, switch to ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK.\r\n");
#endif
						// Request seek to continue 
			            ETALTML_seek_continue_internal(hReceiver);
	                }
	                break;

	            case ETALTML_RDS_AUTO_SEEK_WAITTP:
	               ETALTML_RDSgetDecodedBlock(hReceiver, &rdsData, TRUE);
					

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	               ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITTP, TP: %d, TA: %d  m_validityBitmap:%x\r\n", rdsData.m_TP, rdsData.m_TA, rdsData.m_validityBitmap);
#endif
	               if( (((rdsData.m_validityBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)  && rdsData.m_TP) ||
	                         (((rdsData.m_validityBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)  && rdsData.m_TA))
	               {
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	               	ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITTP, TP Found.\r\n");
#endif

						// Seek ended 
						// 
						RdsSeekCfgp->state	= ETALTML_RDS_AUTO_SEEK_FINISHED;
							
	               	}
					else
					{
						// bitmap not valid : wait
					}
	                
					if ((RdsSeekCfgp->state	== ETALTML_RDS_AUTO_SEEK_WAITTP)
					&& (ETALTML_RDS_Strategy_RDSTimer_Get(hReceiver, RDS_TIMER_10MS) == 0))
	                {
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	                    ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_RDS_AUTO_SEEK_WAITTP, timeout.\r\n");
#endif
	 					// no TP within timer
						vl_RdsSeekStatus.m_status = ETAL_SEEK_RESULT;
                   		ETALTML_Report_RdsSeek_Info(hReceiver, &vl_RdsSeekStatus);
						
						/* Send event and notify */
						// We should notify something else may be : dedicated to SEEK ??
						// 
						RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK;
					
						// Request seek to continue 
						ETALTML_seek_continue_internal(hReceiver);

	                }
	                break;

	            case ETALTML_RDS_AUTO_SEEK_WAITPTY:
	                ETALTML_RDSgetDecodedBlock(hReceiver, &rdsData, TRUE);

				 	if ((rdsData.m_validityBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
	                {
	                	if(rdsData.m_PTY == ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_PTYSELECTTYPE))
	                    {
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	                    	ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_EXT_AUTO_SEEK_WAITPTY, TPY Found.\r\n");
#endif
	   						// 
							RdsSeekCfgp->state	= ETALTML_RDS_AUTO_SEEK_FINISHED;

                            ETALTML_RDS_Strategy_RDSTimer_Set(hReceiver, RDS_TIMER_FUNC_PTY_SELECT, 1);
							
	                        break;
	                    }
	                }

	                if ((RdsSeekCfgp->state	== ETALTML_RDS_AUTO_SEEK_WAITPTY)
						&& (ETALTML_RDS_Strategy_RDSTimer_Get(hReceiver, RDS_TIMER_10MS) == 0))
	                {
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	                    ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDS Seek status: ETALTML_EXT_AUTO_SEEK_WAITPTY, timeout.\r\n");
#endif
	                   	/* Send event and notify */
						vl_RdsSeekStatus.m_status = ETAL_SEEK_RESULT;
						ETALTML_Report_RdsSeek_Info(hReceiver, &vl_RdsSeekStatus);

						// We should notify something else may be : dedicated to SEEK ??
						// 
						RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK;
					
						// Request seek to continue 
						ETALTML_seek_continue_internal(hReceiver);
	                }
	                break;

	            default:
	                // this is an invalid state
	                
	                break;
	        	}
        	}
		else
			{
				// no seek in progress
				vl_complete = TRUE;
					
			}
    }
    	
 	if (ETALTML_RDS_AUTO_SEEK_IDLE == RdsSeekCfgp->state)
 	{
		vl_complete = TRUE;
 	}
	
    return (vl_complete);
}

/***************************
 *
 * ETALTML_rds_seek_start_external
 *
 **************************/
ETAL_STATUS ETALTML_rds_seek_start(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, EtalRDSSeekTy rdsSeekOption)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalSeekStatus vl_RdsSeekStatus;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
	etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
	  etalRdsSeekConfigTy *RdsSeekCfgp = &(recvp->RdsSeekCfg);



	if (rdsSeekOption.tpSeek || rdsSeekOption.taSeek  ||  rdsSeekOption.ptySeek || rdsSeekOption.piSeek)
	{
		// break AF Check etc.
		(LINT_IGNORE_RET)ETALTML_RDS_Strategy_BreakAFCheck(hReceiver);
	}

	if (rdsSeekOption.taSeek || rdsSeekOption.tpSeek)
	{
		if (!ETALTML_RDS_Strategy_GetFunctionStatus(hReceiver, RDS_FUNC_TA))
		{
			//TA did not switched on. then return ERROR
			ret = ETAL_RET_ERROR;
			goto exit;
		}
	}

	if (rdsSeekOption.ptySeek)
	{
		if (rdsSeekOption.ptyCode > 0 && rdsSeekOption.ptyCode < 16)
		{
			ETALTML_RDS_Strategy_SetFunctionStatus(hReceiver, RDS_FUNC_PTY, TRUE);
			ETALTML_RDS_Strategy_SetFunctionStatus(hReceiver, RDS_FUNC_PTYSELECTTYPE, rdsSeekOption.ptyCode);
		}
		else
		{
			ret = ETAL_RET_ERROR;
			goto exit;
		}
	}

	if (rdsSeekOption.piSeek)
	{
		ETALTML_RDS_Strategy_SetFunctionStatus(hReceiver, RDS_FUNC_PISeek, TRUE);
		if(rdsSeekOption.pi) ETALTML_RDS_Strategy_SetBackupPI(hReceiver, rdsSeekOption.pi);
	}
#endif

	// start normal seek
	/* Send event and notify */
	vl_RdsSeekStatus.m_status = ETAL_SEEK_STARTED;
	ETALTML_Report_RdsSeek_Info(hReceiver, &vl_RdsSeekStatus);
	
	// Register a callback to process the continue...
	//
	// so that seek does not auto end and we can grab the rds and continue...
	// 
	if(ETALTML_seek_start_internal(hReceiver, direction, step, cmdAudioMuted, dontSeekInSPS, TRUE) == ETAL_RET_SUCCESS)
	{
		/* Register seek notification */
		if ((ETAL_intCbRegister(callAtEndOfSeek, ETALTML_RdsSeekIntCbFunc, hReceiver, ETAL_INTCB_CONTEXT_UNUSED) == ETAL_RET_SUCCESS) &&
			(ETAL_intCbRegister(callAtSeekStatus, ETALTML_RdsSeekIntCbFunc, hReceiver, ETAL_INTCB_CONTEXT_UNUSED) == ETAL_RET_SUCCESS))
		{
				/* Build RDS SEEK event */
				//
	
				/* Change learn state */
				RdsSeekCfgp->state = ETALTML_RDS_AUTO_SEEK_WAIT_AUTOSEEK;
				RdsSeekCfgp->autoSeek_complete = FALSE;

				// start task
				ETALTML_RdsSeekStartTask();

				ETAL_receiverSetSpecial(hReceiver, cmdSpecialRdsSeek, cmdActionStart);
		}
		else
		{
			//registration error
			ret = ETAL_RET_ERROR;
			goto exit;

		}
	}
	else
	{
		//seek start error
		ret = ETAL_RET_ERROR;
		goto exit;
	}

 
exit:
	return ret;
}



/***************************
 *
 * etaltml_RDS_seek_start
 *
 **************************/
ETAL_STATUS etaltml_RDS_seek_start(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, EtalRDSSeekTy *pRDSSeekOption)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_RDS_seek_start(rec: %d, dir: %d, ste: %d, exiSeeAct: %d)", hReceiver, direction, step, exitSeekAction);
 	if (NULL == pRDSSeekOption)
 	{	
 		// no seek option, this is incorrect	
		ret = ETAL_RET_PARAMETER_ERR;
 	}
	else
	{
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETALTML: etaltml_RDS_seek_start(tpSeek %d, taSeek %d, ptySeek %d, ptyCode %d, piSeek %d, pi %d,)",
		pRDSSeekOption->tpSeek, pRDSSeekOption->taSeek, pRDSSeekOption->ptySeek, pRDSSeekOption->ptyCode, pRDSSeekOption->piSeek, pRDSSeekOption->pi);
	

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
	    if ((ret = ETAL_receiverGetLock(hReceiver)) == ETAL_RET_SUCCESS)
	    {
	    	if (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM)
 		   	{
    			// invalid standard
		    	ret = ETAL_RET_ERROR;
			}
			else
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

					// set that an external request is on going
					// Note : for now this is set but should not else we get the seek report...
					//
					// ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalRequestInProgress, cmdActionStart);

					ret = ETALTML_rds_seek_start(hReceiver, direction, step, exitSeekAction, *pRDSSeekOption);
				}
				
				if ((ret != ETAL_RET_IN_PROGRESS) && (ret != ETAL_RET_SUCCESS))
				{
					ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalRequestInProgress, cmdActionStop);
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
			}
			ETAL_receiverReleaseLock(hReceiver);
	    }
    }
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etaltml_RDS_seek_start() = %s", ETAL_STATUS_toString(ret));
    return ret;

}

#endif // #if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

#endif // CONFIG_ETAL_HAVE_ETALTML && CONFIG_ETALTML_HAVE_RDS_STRATEGY


