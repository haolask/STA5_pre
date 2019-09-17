//!
//!  \file 		ossmphr.c
//!  \brief 	<i><b>OSAL semaphores Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) semaphores Functions.
//!  \author 	Luca Pesenti
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

/* DeactivateLintMessage_ID0026 */
/*lint -esym(750, OS*_C) */
#ifndef OSSMPHR_C
#define OSSMPHR_C

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"   
/* osal Header */
#include "osal.h"


/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/

/**
 * @brief     OSAL_s32SemaphoreCreate
 *
 *
 * @details   This function create a OSAL Semaphore by a os21 Semaphore.
 *
 * @param     coszName name of semaphore (I)
 * @param     phSemaphore pseudo-handle for semaphore (O)
 * @param     uCount payload of semaphore (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreCreate (tCString coszName, OSAL_tSemHandle * phSemaphore, tU32 uCount)
{
 
    UBaseType_t uxMaxCount = 0;
    UBaseType_t uxInitialCount = 0;
    SemaphoreHandle_t SemHandle = NULL;
    tS32 ret = 0;
    uxMaxCount = 0xF;
    uxInitialCount = uCount;
    
    SemHandle = xSemaphoreCreateCounting(uxMaxCount, uxInitialCount);
 

   if ( SemHandle == NULL )
    {
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
      OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32SemaphoreCreate: failed! ErrorCode = 0x%x",vl_sem_id);
#endif
	ret = -1;
    }
   else
   {
	*phSemaphore = (OSAL_tSemHandle)SemHandle;
   }

   return (ret);
}
#ifndef OSAL_SEMAPHORE_SKIP_NAMES
/**
 *
 * @brief   OSAL_s32SemaphoreDelete
 *
 * @details This function removes an OSAL Semaphore.
 *
 * @param   coszName semaphore name to be removed (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreDelete (tCString coszName)
{
 	// In theory here we should delete the semaphore
	// But for now, we do not have the name, so not possible
	// So we do nothing here, it will be deleted on the close
	// in FreeRTOS : this is done nothing to do, the semaphore close is enough
  
  return (OSAL_OK);
}


/**
 *
 * @brief   OSAL_s32SemaphoreOpen
 *
 * @details This function returns a valid handle to an OSAL Semaphore
 *              already created.
 *
 * @param   coszName     semaphore name to be removed (I)
 * @param   phSemaphore  pointer to the semaphore handle (->O)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreOpen (tCString coszName, OSAL_tSemHandle * phSemaphore)
{
	// in FreeRTOS : nothing to do, the semaphore creation is enough
	return (OSAL_OK);
}



/**
 *
 * @brief   OSAL_s32SemaphoreClose
 *
 * @details This function closes an OSAL Semaphore.
 *
 * @param   hSemaphore semaphore handle (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreClose (OSAL_tSemHandle hSemaphore)
{
   tS32 s32ReturnValue = OSAL_OK;
 
   vSemaphoreDelete( (SemaphoreHandle_t) hSemaphore ) ;

   return (s32ReturnValue);
}

#else

/**
 *
 * @brief   OSAL_s32SemaphoreFree
 *
 * @details This function closes an OSAL Semaphore.
 *
 * @param   hSemaphore semaphore handle (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreFree (OSAL_tSemHandle hSemaphore)
{
   tS32 s32ReturnValue = OSAL_OK;
 
   vSemaphoreDelete( (SemaphoreHandle_t) hSemaphore ) ;

   return (s32ReturnValue);
}

#endif
/**
 *
 * @brief   OSAL_s32SemaphoreGetValue
 *
 * @details This function retrieve the actual countvalue.
 *
 * @param   hSemaphore semaphore handle (I)
 * @param   ps32Value semaphore value (->O)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreGetValue (OSAL_tSemHandle hSemaphore, tPS32 ps32Value)
{
    /*UBaseType_t FreeRTOSReturnValue = 0;
   
    FreeRTOSReturnValue = uxSemaphoreGetCount ((SemaphoreHandle_t) hSemaphore);

    *ps32Value = (tS32)FreeRTOSReturnValue;*/

   return (OSAL_ERROR);
}


/**
 *
 * @brief    OSAL_s32SemaphorePost
 *
 * @details  This function release a semaphore
 *                The operation checks the queue of tasks waiting for the semaphore,
 * 				  if the list is not empty, then the first task on the list is restarted,
 * 				  possibly preempting the current task.
 * 				  Otherwise the semaphore count is incremented, and the task continues running.
 *
 * @param   hSemaphore semaphore handle (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphorePost (OSAL_tSemHandle hSemaphore)
{
    tS32 s32ReturnValue = OSAL_OK;
    BaseType_t ercd;

    ercd = xSemaphoreGive ( (SemaphoreHandle_t) hSemaphore);

	//printf("OSAL_s32SemaphorePost (0x%x)\n", hSemaphore);



    if (ercd != pdPASS) // There has been an error!!
    {
#if defined (CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
        OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,
            "OSAL_s32SemaphorePost for ID %d: failed! ErrorCode = 0x%x", hSemaphore, ercd);
#endif // #if (defined CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)

	s32ReturnValue = OSAL_ERROR;

    }

    return (s32ReturnValue);
}

/**
 *
 * @brief   OSAL_s32SemaphoreWait
 *
 * @details This function performs a wait operation on the specified semaphore.
 *                The operation checks the semaphore counter, and if it is 0,
 *                adds the current task to the list of queued tasks, before descheduling.
 *                Otherwise the semaphore counter is decremented, and the task continues running.
 *
 * @param   hSemaphore semaphore handle (I)
 * @param   msec max delaytime before timeout (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreWait (OSAL_tSemHandle hSemaphore, OSAL_tMSecond msec)
{
    tS32 s32ReturnValue = OSAL_OK;
 
    BaseType_t ercd;

    //  printf("OSAL_s32SemaphoreWait (0x%x), time = %d\n", hSemaphore, msec);


    switch ((tU32)msec)
    {
        case OSAL_C_TIMEOUT_FOREVER:
            ercd = xSemaphoreTake((xSemaphoreHandle)hSemaphore, portMAX_DELAY);
            break;
 
        case OSAL_C_TIMEOUT_NOBLOCKING:
            ercd = xSemaphoreTake((xSemaphoreHandle)hSemaphore, pdMS_TO_TICKS(msec));
            break;
        
        default:
            ercd = xSemaphoreTake((xSemaphoreHandle)hSemaphore, pdMS_TO_TICKS(msec));
            break;
    }

    #if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    if (ercd != pdPASS)
    {
        (void)OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreWait failure %d", ercd);
        s32ReturnValue = OSAL_ERROR;
    }
    #endif
    
    return (s32ReturnValue);
}

#ifdef __cplusplus
}
#endif

#endif  //OSSMPHR_C

/** @} */

/* End of File */

