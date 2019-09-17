//!
//!  \file 		osproc.c
//!  \brief 	<i><b>OSAL Thread Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) Thread Functions.
//!  \author 	Luca Pesenti
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

/* DeactivateLintMessage_ID0026 */
#ifndef OSPROC_C
#define OSPROC_C

#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/
#include <tk/tkernel.h> //m//

/* osal Header */
#include "osal.h"



/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/
#if 0
#define OS21_MAX_PROCESS 1  // KS
#define OS21_C_THREAD_MAX_NAMELENGTH (OSAL_C_U32_MAX_NAMELENGTH - 1)  // KS

#define OS21_C_MINIMUM_INTERVAL_US  	  (100UL)

/* Max Number of entries in Tables */
#define OS21_C_THREAD_TABLE_INIT_ENTRIES        12
#define OS21_C_THREAD_TABLE_ADDED_ENTRIES       3
#endif

#define OS21_C_THREAD_MAX_NAMELENGTH (OSAL_C_U32_MAX_NAMELENGTH - 1)  // KS

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/
// typedef tVoid(* vOsalEntryFunc)(tPVoid);


/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/


/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/
/* ==> NOT NEEDED / SUPPORTED
 */
#if 0

    //OSAL_tpfThreadEntry pfOsalEntry=OSAL_NULL;
    tThreadTable        ThreadTable; /* Table control block for OSAL Thread.*/
    semaphore_t*        os21_ghThreadTableLock;

    message_queue_t*    TerminatorQueue; // KS

extern unsigned int bsp_timeslice_frequency_hz;

#endif

/************************************************************************
|function prototype (scope: module-local)
-----------------------------------------------------------------------*/
	/* ==> NOT NEEDED / SUPPORTED
	 */

static tS32 OSAL_s32ThreadActivate( OSAL_tThreadID tid,   tSInt Arg);

#if 0
 
    static tVoid pfWrapperEntry(tPVoid pvArg); //KS changed the type of argc from UNSIGNED to tU32

           tS32 os21_s32ThreadTableAddEntry(tThreadTable *aThreadTable);

           tS32 os21_s32ThreadTableCreate(tThreadTable *aThreadTable);

    static trThreadElement* os21_tThreadTableGetFreeEntry(void);

#ifndef OSAL_THREAD_LIGHT
           trThreadElement* os21_tThreadTableSearchEntryByID(OSAL_tThreadID tid);
#endif

    static trThreadElement *os21_tThreadTableSearchEntryByName(tCString coszName);

 //   static OSAL_tThreadID os21_tGetThreadID( task_t * pCurrentTaskPtr);

    static tS32 os21_s32ConvertThreadStatus( trThreadElement* ThreadElement,
                                         task_status_state_t s32Status,
                                         OSAL_tenThreadState *penThreadState);

           tVoid os21_vSetErrorCode( OSAL_tThreadID tid, tU32 u32ErrorCode );

           tVoid os21_tThreadChangePendedFlag(void);

extern tVoid OSAL_vShutdown(OSAL_tMSecond msec, tS32 s32Options);
#endif

/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/

/**
* @brief      pfWrapperEntry
*
* @details   This function is used by os21 to call OSAl Thread function,
*				            to adapt OSAL arguments to os21 arguments.
*
* @param
*            - pvArg  pointer to arguments equal to os21 and OSAL (I)
*
* @return NULL
*
*/
/* ==> NOT NEEDED / SUPPORTED
 */
#if 0

static tVoid pfWrapperEntry(tPVoid pvArg)
#endif
/**
 *
 * @brief    os21_s32ThreadTableAddEntry
 *
 * @details This function adds a new element in front of Thread List.
 *              If there isn't space it returns a error code.
 *
 * @param  aThreadTable pointer to the Thread table List (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
*/
/* ==> NOT NEEDED / SUPPORTED
 */
#if 0

tS32 os21_s32ThreadTableAddEntry(tThreadTable *aThreadTable)

#endif

/**
*
* @brief    os21_s32ThreadTableCreate
*
* @details This function creates the Thread Table List. If
*              there isn't space it returns a error code.
*
* @param   aThreadTable pointer to the Thread table List (I)
*
* @return
*         - OSAL_OK if everything goes right
*         - OSAL_ERROR otherwise
*/
/* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 os21_s32ThreadTableCreate(tThreadTable *aThreadTable)
#endif

/**
*
* @brief    os21_tThreadTableGetFreeEntry
*
* @details this function goes throught the Thread List and returns the
*              first ThreadElement.In case no entries are available a new
*              defined number of entries is attempted to be added.
*              In case of succes the first new element is returned, otherwise
*              a NULL pointer is returned.
*
*
* @return
 *          - free entry pointer to ThreadElement
 *          - OSAL_NULL otherwise
*/

/* ==> NOT NEEDED / SUPPORTED
 */
#if 0

static trThreadElement* os21_tThreadTableGetFreeEntry(void)
#endif

/**
 *
 * @brief    os21_tThreadTableSearchEntryByID
 *
 * @details This function goes throught the Thread List and returns the
 *          ThreadElement with the given ID or NULL if all the List has
 *          been checked without success.
 *
 * @param   tid thread ID wanted (I)
 *
 * @return
 *          - free entry pointer to ThreadElement
 *          - OSAL_NULL otherwise
 */
 
 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
trThreadElement *os21_tThreadTableSearchEntryByID(OSAL_tThreadID tid)
#endif

/**
 *
 * @brief    os21_tThreadTableSearchEntryByName
 *
 * @details This function goes throught the Thread List and returns the
 *          ThreadElement with the given name or NULL if all the List has
 *          been checked without success.
 *
 * @param   coszName thread name wanted (I)
 *
 * @return
 *          - free entry pointer to ThreadElement
 *          - OSAL_NULL otherwise
 */
 
 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
static trThreadElement *os21_tThreadTableSearchEntryByName(tCString coszName)
#endif

///**
// * @brief      os21_GetThreadID
// *
// * @details   This function gets the Osal_ThredID ID related to the
// *				            pointer of task control block.
// *
// *
// * @param
// *  	    pCurrentTaskPointer          pointer to a field Tast_t that
// *                                       is contained  in specific field
// *                                       (pointed by id of thread) of the
// *                                       record System Table: This pointer
// *                                       is used by os21 for manipulation
// *                                       of threads. (I)
// *
// *
// * @return
// *          - Thread ID
// *          - OSAL_NULL otherwise
// */
//OSAL_tThreadID os21_tGetThreadID( task_t *pCurrentTaskPtr)
//{
//       OSAL_tThreadID RetVal = OSAL_ERROR;
//
//       trThreadElement *pCurrent = ThreadTable.ptrHeader;
//
//       while ( (pCurrent!= OSAL_NULL) &&
//               ( (pCurrent->bIsUsed == FALSE )                     ||
//                 ((pCurrent->TaskControlBlock) != pCurrentTaskPtr))
//             )
//       {
//          pCurrent = pCurrent->pNext;
//       }
//
//       if(pCurrent!= OSAL_NULL)
//          RetVal = pCurrent->TaskControlBlock;
//
//       return RetVal;
//}

/**
 * @brief      os21_s32ConvertThreadStatus
 *
 * @details   This function converts the os21-task-s32Status into
 *                the osal-thread-s32Status.
 *
 * @param
 *        ThreadID        OSAL identifier of thread (I)
 *	      s32Status       os21 s32Status (I)
 *        penThreadState  pointer to ThreadState filled by this function (O)
 *
* @return
*         - OSAL_OK if everything goes right
*         - OSAL_ERROR otherwise
 */
/* ==> NOT NEEDED / SUPPORTED
 */
#if 0
static tS32 os21_s32ConvertThreadStatus(trThreadElement *pCurrentEntry,
                                        task_status_state_t s32Status,
                                        OSAL_tenThreadState *penThreadState)
#endif

/**
 * @brief     os21_vSetErrorCode
 *
 * @details   This function set the error codes into the sistem table
 *                  of threads.
 *
 * @param    tid            OSAL identifier of thread (I)
 * @param    u32ErrorCode   error code to store in the specific field (I)
 *
 * @return   NULL
 */

// NOT USED IN SIMPLIFY OS VERSION
//
#if 0
tVoid os21_vSetErrorCode( OSAL_tThreadID tid, tU32 u32ErrorCode )
#endif 

/**
*
* @brief    os21_tThreadChangePendedFlag
*
* @details  This function changes the value of the variable bIsSetPendedStatus
*              of the current Thread.
*
*
* @return   NULL
*/

 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tVoid os21_tThreadChangePendedFlag(void)
#endif


/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/

/*****************************************************************************/
/*****************************************************************************/
/* Process functions: Function used by OSAL process for fake (Actually!)    */
/*                     implementation of processes .                         */
/*****************************************************************************/
/*****************************************************************************/

/**
 * @brief    OSAL_ProcessSpawn
 *
 * @details  This Function creates a new process and execute the
 *                OSAL_s32Boot function of the specified application.
 *                The first thread in the process gets automatically the
 *                highest thread priority OSAL_C_U32_THREAD_PRIORITY_HIGHEST.
 *                The name of the first thread correspond to the process name.
 *
 * @param    pcorAttr pointer to a record that contains the attributes of new process (I)
 *
 * @return
 *           - ID of the Process
 *           - OSAL_ERROR otherwise
 */

 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
OSAL_tProcessID OSAL_ProcessSpawn(const OSAL_trProcessAttribute *pcorAttr)
#endif

/**
 * @brief    OSAL_s32ProcessDelete
 *
 * @details  This Function terminate a process.All threads assigned to
 *                the process are terminated likewise.
 *
 * @param    pid Id of the process (I)
 *
 * @return
 *          - OSAL_OK if everything goes right
 *          - OSAL_ERROR otherwise.
 */

 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32ProcessDelete(OSAL_tProcessID pid)
#endif
/**
 * @brief    OSAL_ProcessWhoAmI
 *
 * @details  This Function returns the process ID of the currently
 *                running process.
 *
 *
 * @return   Process ID
 */

 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
OSAL_tProcessID OSAL_ProcessWhoAmI(void)
#endif

/**
 * @brief    OSAL_vProcessExit
 *
 * @details  This Function ends on goning process.All Threads are
 *                terminated likewise.
 *
 *
 * @return   NULL
 */

 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tVoid OSAL_vProcessExit(void)
#endif

/**
 * @brief    OSAL_s32ProcessList
 *
 * @details  This Function returns all processes registered in the sistem.
 *
 * @param    pas32List[] list of process ID (I)
 * @param    s32Length   maximal size of the list (I)
 *
 * @return   Number of IDs in the list
 */
 
 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32ProcessList(tS32 pas32List[],tS32 s32Length)
#endif

/**
 * @brief     OSAL_s32ProcessControlBlock
 *
 * @details   This Function returns a process control block.
 *
 * @param    pid    Id of the process (I)
 * @param    prPcb  Pointer to control block structure (O)
 *
 * @return
 *           - OSAL_OK if everything goes right
 *           - OSAL_ERROR otherwise
 *
 */

/* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32ProcessControlBlock(OSAL_tProcessID pid,OSAL_trProcessControlBlock* prPcb)
#endif


/**
 * @brief    OSAL_ThreadCreate
 *
 * @details  This function creates a new Thread in the system.
 *           The thread is however not activated i.e. there is no
 *           possiblle allocation to the scheduler. The statre of the
 *           thread is initialized.
 *
 * @param    pcorAttr   pointer to a record that contains
 *                      the attributes of new thread (I)
 *
 * @return
 *           - ID of the Thread
 *           - OSAL_ERROR otherwise
 */
OSAL_tThreadID OSAL_ThreadCreate( const OSAL_trThreadAttribute* pcorAttr )
{
	
	/*Define Local Variables */
	
	T_CTSK 		pl_k_ctsk;
	OSAL_tThreadID vl_tskid = NULL;


	// parameter which will be provided back to the task entry point
	pl_k_ctsk.exinf = pcorAttr->pvArg; //  Extended information

	// Task Attributes : is an set of 
	/*
	TA ASM Indicates that the task is written in assembly language
	TA HLNG Indicates that the task is written in high-level language
	TA SSTKSZ Specifies the system stack size
	TA USERSTACK Points to the user stack
	TA TASKSPACE Points to the task space
	TA RESID Specifies the resource group to which the task belongs
	TA DSNAME Specifies DS object name
	TA RNGn Indicates that the task runs at protection level n
	TA COPn Specifies use of the nth co-processor (including floating point co-processor or
	DSP)
	TA FPU Specifies use of a floating point co-processor (when a co-processor specified in
	TA COPn is a general-purpose FPU
	*/
	// We choose
	// TA HLNG Indicates that the task is written in high-level language
	// TA RNGn Indicates that the task runs at protection level n
	// TA DSNAME Specifies DS object name
	// TA SSTKSZ Specifies the system stack size
		
	pl_k_ctsk.tskatr = TA_HLNG | TA_RNG0 | TA_DSNAME | TA_SSTKSZ; //Task attributes

	// Task Start Address = task entry point
	pl_k_ctsk.task = pcorAttr->pfEntry; 
	
	//Initial task priority
	pl_k_ctsk.itskpri = pcorAttr->u32Priority; //Initial task priority

	// stak size : not used, we use system stack size
	pl_k_ctsk.stksz = 0; // Stack size (bytes)

	// System stack size (bytes)
	pl_k_ctsk.sstksz = pcorAttr->s32StackSize; // System stack size (bytes)

	/* not used
	pl_k_ctsk.stkptr = ; //User stack pointer
	pl_k_ctsk.uatb = ; // Task space page table
	pl_k_ctsk.lsid = ; // Logical space ID
	pl_k_ctsk.resid = ; //Resource ID
	*/

	// Put the task Name : Max 8 characters
	strncpy((char *) pl_k_ctsk.dsname, pcorAttr->szName, 8);

	// Now create the task

	vl_tskid = tk_cre_tsk(&pl_k_ctsk);


	if (vl_tskid < E_OK) 
	{
		(void)OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "cre tsk fails = 0x%x", vl_tskid);
		vl_tskid = (OSAL_tThreadID)OSAL_ERROR;
	}

   return( (OSAL_tThreadID) vl_tskid);
}

/**
 * @brief     OSAL_s32ThreadActivate
 *
 * @details   This function activates a Thread generated earlier through
 *            OSAL_ThreadCreate i.e. the state of the thread change in
 *            ready.
 *
 * @param    tid Thread ID of the generated thread (I)
 *
 * @return
 *          - OSAL_OK if everything goes right
 *          - OSAL_ERROR otherwise
 */
static tS32 OSAL_s32ThreadActivate( OSAL_tThreadID tid,   tSInt Arg)
{
	ER ercd;
	
	tS32 s32ReturnValue = OSAL_ERROR;


	// call the T-Kernel OS function
	ercd = tk_sta_tsk ( (ID) tid, (INT) 0 ) ;

	
	if (ercd == E_OK)	
	{
		s32ReturnValue = OSAL_OK;
	}
	
	if ( s32ReturnValue != OSAL_OK )
	   {
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
		  OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32ThreadActivate / tk_sta_tsk Error code return =  %d", ercd);
#endif
	   }
	
	return(s32ReturnValue);

}

/**
 * @brief     OSAL_ThreadSpawn
 *
 * @details   This function creates a new thread with the options
 *            transferred as parameter. The thread is started
 *            immediately and readied the CPU-Queue (ready s32Status).
 *            Each new thread receives an own stack with the specified states.
 *
 * @param    pcorAttr pointer to a record that contains the attributes of new thread (I)
 *
 * @return
 *           - ID of the Thread
 *           - OSAL_ERROR otherwise
 */
OSAL_tThreadID OSAL_ThreadSpawn( const OSAL_trThreadAttribute* pcorAttr )
{
   /*Define Local Variables */
   tS32 s32ReturnValue = OSAL_ERROR;

   OSAL_tThreadID tid;

   /* Create Thread */
   tid = OSAL_ThreadCreate(pcorAttr);
   if (tid != OSAL_ERROR)
   {

   	/* Activate Thread */
	   if (OSAL_s32ThreadActivate(tid, 0) != OSAL_ERROR)
	   	{
	   	s32ReturnValue = tid;
	   	}
   	}
   
   return (s32ReturnValue);
}

/**
 * @brief     OSAL_s32Delete
 *
 * @details   Delete a thread and release the stack again.
 *		
 *
 * @param     tid  Thread ID of the thread to be delete
 *
 * @return
 *          - OSAL_OK if everything goes right
 *          - OSAL_ERROR otherwise
 */
tS32 OSAL_s32ThreadDelete( OSAL_tThreadID tid )
{

	ER ercd;
  	tS32 s32ReturnValue = OSAL_ERROR;	

	// The request is to delete the thread
	// if the thread is active, in T-Kernel it will not work

	// so first, terminate the task.

	ercd = tk_ter_tsk((ID) tid);
	
	if (ercd != E_OK)	
	{
		
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
		  OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32ThreadDelete / task terminate Error code return =  %d", ercd);
#endif
	}
	else
	{
	
		// call the T-Kernel OS function
		ercd = tk_del_tsk((ID) tid); 

		if (ercd != E_OK)	
			{
		
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
			  OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"OSAL_s32ThreadDelete / take exit Error code return =  %d", ercd);
#endif
			}
		else
			{
			s32ReturnValue = OSAL_OK;
			}
	}

	return(s32ReturnValue);

}

/**
 * @brief    OSAL_vThreadExit
 *
 * @details  This function closes the current thread (Return).
 *           The stack is released.
 *
 *
 * @return   NULL
 */
 // This is not really used for now. but can be usefull as an exemple for a
tVoid OSAL_vThreadExit( void )
{


	// call the T-Kernel OS function
	tk_ext_tsk(); 
	
}

/**
 * @brief    OSAL_s32ThreadSuspend
 *
 * @details  This function suspend the Thread specified as parameter.
 *                The suspending is additive i.e. a Thread in "delayed" or
 *                "pended" s32Status gets an additional suspended-Attribute.
 *                If the thread has filled certain resuorces e.g for inter
 *                process communication, then they are not released through
 *                the function. Here under circumstances a Deadlock-Situation
 *                may be formed.
 *
 * @param    tid  Thread ID (I)
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
 
/* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32ThreadSuspend( OSAL_tThreadID tid )
{

	ER ercd = tk_sus_tsk ( ID tskid ) ;
}
#endif 

/**
 * @brief    OSAL_s32ThreadResume
 *
 * @details  In This function the Thread is reset from the "suspended"
 *           status. A possible "delayed" or "pended" status is retained.
 *
 * @param    tid Thread ID (I)
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */

/* ==> NOT NEEDED / SUPPORTED
 */
#if 0

tS32 OSAL_s32ThreadResume( OSAL_tThreadID tid )
{
	// Would be mapped to TkErnel
	ER ercd = tk_rsm_tsk ( ID tskid ) ;
 
}
#endif
/**
 * @brief    OSAL_s32ThreadWait
 *
 * @details  This function suspended the calling thread for a period
 *           delayed status.
 *
 * @param    msec        Time interval in ms (I)
 *
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
tS32 OSAL_s32ThreadWait( OSAL_tMSecond msec )
{
	 ER ercd;
	
	tS32 s32ReturnValue = OSAL_ERROR;

   // call the T-Kernel OS function
   // 
	ercd = tk_dly_tsk (msec); 
 
	   
   if (ercd == E_OK)   
   {
	   s32ReturnValue = OSAL_OK;
   }
   else
   {
	   s32ReturnValue = OSAL_ERROR;
   }
   
  if ( s32ReturnValue != OSAL_OK )
  {
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
	 OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE,"Thread Wait Error code return =  %d", ercd);
#endif
  }
  return(s32ReturnValue);

}

/**
 * @brief    OSAL_s32ThreadWait_us
 *
 * @details  This function suspended the calling thread for a period
 *           delayed status.
 *
 * @param    msec        Time interval in ms (I)
 *
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
tS32 OSAL_s32ThreadWait_us( OSAL_tuSecond usec )
{

   tS32 s32ReturnValue = OSAL_OK;
 
	// call the T-Kernel OS function
	// 
	//	 ercd = tk_dly_tsk (usec / 1000); 
	// in TKernel, the thread suspension is based on ms time
	//
	// we assume here the call is a request for very short suspension << 1 ms
	
	WaitUsec(usec);
   

   return(s32ReturnValue);
}
/**
 * @brief    OSAL_ThreadWhoAmI
 *
 * @details  This function returns the threadID for the current Thread
 *
 *
 * @return
 *           - ID of the Thread
 *           - OSAL_ERROR otherwise
 */
OSAL_tThreadID OSAL_ThreadWhoAmI( void )
{
   return((OSAL_tThreadID)tk_get_tid());
}

/**
 * @brief    OSAL_s32ThreadPriority
 *
 * @details  This function changes the priority of a thread. The process
 *           priority is retained.
 *
 * @param   tid              Thread ID of the thread (I)
 * @param   u32Priority      new priority (I)
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
 
 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32ThreadPriority( OSAL_tThreadID tid, tU32 u32Priority )
{
	// Would be mapped to TK 
	ER ercd = tk_chg_pri ( ID tskid, PRI tskpri ) ;

}
#endif


/**
 * @brief     OSAL_s32GetPriority
 *
 * @details   Get the Priority of the Thread
 *
 * @param     tid                  ID of the thread (I)
 * @param     pu32CurrentPriority Pointer to the priority value to return (O)
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
 
/* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32GetPriority(OSAL_tThreadID tid, tU32 *pu32CurrentPriority)
{
	// Would be mapped to TK 
	//

	ER ercd = tk_ref_tsk ( ID tskid, T_RTSK *pk_rtsk ) ;

	with T_RTSK : 
		VP exinf Extended information
		PRI tskpri Current task priority
		PRI tskbpri Base priority
		UINT tskstat Task state
		UINT tskwait Wait factor
		ID wid Waiting object ID
		INT wupcnt Queued wakeup requests
		INT suscnt Nested suspend requests
		RELTIM slicetime Maximum continuous run time allowed (ms)
		UINT waitmask Disabled wait factors
		UINT texmask Allowed task exceptions
		UINT tskevent Task events
		(Other implementation-dependent parameters

}
#endif


/**
 * @brief     OSAL_s32ThreadControlBlock
 *
 * @details   In this function the Thread control block information is
 *            written in to the transferred structure: Thread control
 *            information can not be changed.
 *
 * @param     tid                  ID of the thread (I)
 * @param     prThreadControlBlock Pointer of thread control block structure (I)
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
 
 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32ThreadControlBlock(
							     OSAL_tThreadID   tid,
                                OSAL_trThreadControlBlock* prThreadControlBlock
							     )

#endif

/*
 * @brief     OSAL_s32ThreadStackInfo
 *
 * @details   In this function the Thread stack info.
 *
 * @param     tid           ID of the thread (I)
 * @param     stackSize     Stack size
 *            stackBasePtr  Stack base
 *            stackPtr      Stack current position
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
 
 /* ==> NOT NEEDED / SUPPORTED
 */
#if 0
tS32 OSAL_s32ThreadStackInfo (OSAL_tThreadID   tid, tPS32 stackSizePtr, tPU32 stackBasePtr, tPU32 stackPtr)
#endif


/**
 * @brief     OSAL_s32Threadlist
 *
 * @details   This Function gives a list of the current Thread IDs of the
 *            current process.
 *
 * @param    pas32List[] list of thread IDs (I)
 * @param    s32Lenght   maximal number of elements of the list (I)
 *
 * @return   Number of IDs in the list
 */

/* ==> NOT NEEDED / SUPPORTED
*/
#if 0
tS32 OSAL_s32ThreadList(tS32 pas32List[], tS32 s32Lenght)
#endif


#ifdef __cplusplus
}
#endif

#endif  /* OSPROC_C */

/** @} */

/* End of File */
