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
#ifndef OSSMPHR_C
#define OSSMPHR_C

#ifdef __cplusplus
extern "C" {
#endif

#include <tk/tkernel.h> //m//

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
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/

/************************************************************************
|function prototype (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|function implementation (scope: module-local)
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
   tS32 s32ReturnValue = OSAL_ERROR;
 
   T_CSEM pk_csem; //m//
   ID vl_sem_id = 0 ;

 

   // Semaphore creation parameters
   //
   /*
	   [Parameters]
	   T CSEM* pk csem Information about the semaphore to be created
	   pk csem detail:
	   VP exinf Extended information
	   ATR sematr Semaphore attributes
	   INT isemcnt Initial semaphore count
	   INT maxsem Maximum semaphore count
	   */


	// VP exinf Extended information : none
	//
	pk_csem.exinf = (VP) 0x00000000;

	// sematr Semaphore attributes
	pk_csem.sematr = TA_TFIFO | TA_FIRST; // phSemaphore;	

	//  INT isemcnt Initial semaphore count
	pk_csem.isemcnt = uCount;

	// Maximum semaphore count
	pk_csem.maxsem = 0xffff; //  MAX_SEMID;  // // 65535; // MAX_VALUE!!!!
   
		
	vl_sem_id = tk_cre_sem(&pk_csem);
	
	if((vl_sem_id==E_NOMEM) || (vl_sem_id==E_LIMIT) || (vl_sem_id==E_RSATR) || (vl_sem_id==E_PAR))
	{
		s32ReturnValue = OSAL_ERROR;
		*phSemaphore = NULL;
	 }
	else
	{	
		*phSemaphore = vl_sem_id;
		s32ReturnValue = OSAL_OK;
	}

   if ( s32ReturnValue != OSAL_OK )
    {
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
      OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32SemaphoreCreate: failed! ErrorCode = 0x%x",vl_sem_id);
#endif
   	}
   else
   {
	   // printf("OSAL_s32SemaphoreCreate %s, (0x%x)\n", coszName, vl_sem_id);

   }
   
   return (s32ReturnValue);
}

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
	// in T-Kernel : this is done nothing to do, the semaphore close is enough
  
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
	// in T-Kernel : nothing to do, the semaphore creation is enough
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
   ER ercd;

 
   ercd = tk_del_sem ( (ID) hSemaphore ) ;

   if ( ercd != E_OK ) // if there has been an error
    {
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
     OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32SemaphoreClose: failed! ErrorCode = 0x%x",ercd);
#endif
	s32ReturnValue = OSAL_ERROR;

    }
   else
   {
	// printf("OSAL_s32SemaphoreClose (0x%x)\n", hSemaphore);
	s32ReturnValue = OSAL_OK;

   }

   return (s32ReturnValue);
}

/**
 *
 * @brief   OSAL_s32SemaphoreGetValue
 *
 * @details This function retrieve the actual countvalue.
 *
 * @param   hSemaphore semaphore handle (I)
 * @param   ps32Value semaphore value (->O)
 *
 * Semaphore value is expected also to reflect the number of waiting task :
 * negative value represents the number of waiting tasks. 0 means no waiting tasks. 
 * 
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreGetValue (OSAL_tSemHandle hSemaphore, tPS32 ps32Value)
{
   tS32 s32ReturnValue = OSAL_ERROR;
    ER ercd;
   T_RSEM pk_rsem;

   
   ercd = tk_ref_sem ((ID) hSemaphore, &pk_rsem ) ;

   /* parameters of T_RSEM
   
	   pk rsem detail:
	   VP exinf Extended information
	   ID wtsk Waiting task information
	   INT semcnt Semaphore count
	*/


  if ( ercd != E_OK )  // if there is an error
  {
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
          OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32SemaphoreGetValue: failed! ErrorCode = 0x%x",ercd);
#endif
   	
   	s32ReturnValue = OSAL_ERROR;

   }
	else if (ps32Value != NULL)
	{
		// reflect if any task is waiting
		// 
		if (0 == pk_rsem.wtsk)
		{	
			// nobody waits, just provide the semaphore counter
			*ps32Value = pk_rsem.semcnt;
		}
		else
		{
			// some task are waiting : 
			// we do not know how much
			// put -1
			*ps32Value = -1;
		}
			
	s32ReturnValue = OSAL_OK;
	}
	else
	{
		/* Nothing to do */
	}
	
   return (s32ReturnValue);
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
  	ER ercd;

	/*	[Parameters]
	ID semid Semaphore ID
	INT cnt Resource return count
	*/
	
	ercd = tk_sig_sem ( (ID) hSemaphore, 1) ;

	//printf("OSAL_s32SemaphorePost (0x%x)\n", hSemaphore);



    if (ercd != E_OK) // There has been an error!!
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
 
	ER ercd;
	
  //  printf("OSAL_s32SemaphoreWait (0x%x), time = %d\n", hSemaphore, msec);


 	switch ((tU32)msec)
    {
    	case OSAL_C_TIMEOUT_FOREVER:
            ercd = tk_wai_sem((ID)hSemaphore, 1, TMO_FEVR);
            break;
        case OSAL_C_TIMEOUT_NOBLOCKING:
            ercd = tk_wai_sem((ID)hSemaphore, 1, TMO_POL);
                		
            break;
         default:
            {

                ercd= tk_wai_sem((ID)hSemaphore, 1, msec);

            }
            break;
        }

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
	if ((ercd != E_OK) && (ercd != E_TMOUT))
	{
    		(void)OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreWait failure %d", ercd);
			s32ReturnValue = OSAL_ERROR;
	}
#endif


   return s32ReturnValue;
}

#ifdef __cplusplus
}
#endif

#endif  //OSSMPHR_C

/** @} */

/* End of File */

