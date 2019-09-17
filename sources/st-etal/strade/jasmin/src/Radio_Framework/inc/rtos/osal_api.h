/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file osal_api.h																		    			*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of osal private									*
*																											*
*************************************************************************************************************/

#ifndef _OSAL_API_H
#define _OSAL_API_H

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "osal_private.h"
#if defined OS_WIN32
#include "win32_os_api.h"
#endif

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
    function declarations extern
-----------------------------------------------------------------------------*/
#ifdef UITRON
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateMB is to create Mailbox 
*   \param[in]				None
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create mailbox,to send message from one component to other component  
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_CreateMB(void);
#elif defined OS_WIN32
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateMB is to create Mailbox
*   \param[in]				None
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create mailbox,to send message from one component to other component
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32* OSAL_CreateMB(Tu32 *mbbuf, Tu16 mbsize);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateSemaphore is to create semaphore
*   \param[in]				None
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create semaphore
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts32 OSAL_CreateSemaphore(void);
#endif
#ifdef UITRON
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateSemaphore is to create semaphore 
*   \param[in]				None
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create semaphore  
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_CreateSemaphore(void);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateMemPool is to create memorypool 
*   \param[in]				const char *task_name
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create memorypool 
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_CreateMemPool(const char *task_name);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateTimer is to create timer
*   \param[in]				void* func --function pointer
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create timer  
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_CreateTimer(void* func);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_StartTimer is to create two tasks 
*   \param[in]				ER_ID TimerId --timerid
*                           RELTIM timerduration --duration
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to start timer withrespect to timerid and give delay time withrespect to timeduration  
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_StartTimer(ER_ID TimerId, RELTIM timerduration);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_StopTimer is to stop the timer 
*   \param[in]				ER_ID TimerId--timerid
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to stop the timer withrespect to timerid  
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_StopTimer(ER_ID TimerId);
#endif
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateMutex is to create mutex 
*   \param[in]				None
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create mutex 
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_CreateMutex(void);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_Mutex_Lock is to lock the source 
*   \param[in]				ER_ID mutexId --mutexid
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to lock the source withrespect to mutexId   
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_Mutex_Lock(ER_ID mutexId);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_Mutex_Unlock unlock the source 
*   \param[in]				ER_ID mutexId --- mutexid
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to unlock the source withrespect to mutexId 
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_Mutex_Unlock(ER_ID mutexId);

#ifdef UITRON
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateTask is to create tasks 
*   \param[in]				Tu8   	priority--Priority
*                            OSAL_TASK_FUNC 	func-- function pointer
*                            void*  	stack--function pointer
*                            Tu32   	stack_size--stack size
*
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used Create tasks 
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_CreateTask(Tu8  priority, OSAL_TASK_FUNC func, void*  stack, Tu32  stack_size);

#elif defined OS_WIN32
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateTask is to create tasks
*   \param[in]				Tu8   	priority--Priority
*                            OSAL_TASK_FUNC 	func-- function pointer
*                            void*  	stack--function pointer
*                            Tu32   	stack_size--stack size
*
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used Create tasks 
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/

Tu32 OSAL_CreateTask(Tu8 priority, OSAL_TASK_FUNC func, void* stack, Tu32 stack_size);
#endif

#ifdef UITRON					
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_TaskActivate is activate the created two tasks 
*   \param[in]				ER_ID taskid --taskid
*   \param[out]				ER_ID
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to activate the tasks  
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER_ID OSAL_TaskActivate(ER_ID taskid);
#elif defined OS_WIN32
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_TaskActivate is activate the created two tasks
*   \param[in]				Tu32 taskid --taskid
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to activate the tasks
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts32 OSAL_TaskActivate(Tu32 taskid);
#endif

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_CreateEvent is to create event object
*   \param[in]				Tu32 taskId  -- taskId 
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to create Event object.
*   \post-condition			The event object is created.
*   \ErrorHandling    		Return 0 for FAILURE
*									  else for SUCCESS
*
******************************************************************************************************/
Tu32 OSAL_CreateEvent(Tu32 taskId);

#ifdef UITRON
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_DeleteMB is delete the created Number of Mailboxes 
*   \param[in]				ER_ID mbx_id --mbx_id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to delete the Mailboxes  
*   \post-condition			The system Shutdown up. Multitasking can end.
*   \ErrorHandling    		Return on 0 for SUCCESS
*									  else for FAILURE
* 
******************************************************************************************************/
Tu8 OSAL_DeleteMB(ER_ID mbx_id);
#elif defined OS_WIN32
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_DeleteMB is delete the created Number of Mailboxes
*   \param[in]				Ts32* mbx_id --mbx_id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to delete the Mailboxes
*   \post-condition			The system Shutdown up. Multitasking can end.
*   \ErrorHandling    		Return on 0 for SUCCESS
*									  else for FAILURE
*
******************************************************************************************************/
Tu8 OSAL_DeleteMB(Tu32* mbx_id);
#endif

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_DeleteSemaphore is delete the created Number of Semaphores 
*   \param[in]				ER_ID sem_id --sem_id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to delete the Semaphores  
*   \post-condition			The system Shutdown up. Multitasking can end.
*   \ErrorHandling    		Return on 0 for SUCCESS
*									  else for FAILURE
* 
******************************************************************************************************/
Tu8 OSAL_DeleteSemaphore(ER_ID sem_id);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_DeleteMemPool is delete the created Number of Memorypools 
*   \param[in]				ER_ID mpf_id --mpf_id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to delete the Memorypools  
*   \post-condition			The system Shutdown up. Multitasking can end.
*   \ErrorHandling    		Return on 0 for SUCCESS
*									  else for FAILURE
* 
******************************************************************************************************/
Tu8 OSAL_DeleteMemPool(ER_ID mpf_id);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_DeleteTask is delete the created Number of Tasks 
*   \param[in]				ER_ID tsk_id --tsk_id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to delete the Tasks  
*   \post-condition			The system Shutdown up. Multitasking can end.
*   \ErrorHandling    		Return on 0 for SUCCESS
*									  else for FAILURE
* 
******************************************************************************************************/
Tu8 OSAL_DeleteTask(ER_ID tsk_id);

/*****************************************************************************************************/
/**	 \brief                 This API function RTOS_DeleteMutex is delete the created Mutex
*   \param[in]				ER_ID MutexId --Mutex id
*   \param[out]				Tu8
*   \pre-condition			RTOS layer must be fully operational.
*   \details                This API function is used to delete the Mutex.
*   \post-condition			The system Shutdown up. Multitasking can end.
*   \ErrorHandling    		Return on 0 for SUCCESS
*									  else for FAILURE
*
******************************************************************************************************/
Tu8 OSAL_DeleteMutex(ER_ID  MutexId);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_Terminate_Task is to put task into DORMANT state 
*   \param[in]				ER_ID tsk_id --tsk_id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to put the target task is excluded from the scheduling subject. 
*   \post-condition			The system Shutdown up. Multitasking can end.
*   \ErrorHandling    		Return on 0 for SUCCESS
*									  else for FAILURE
* 
******************************************************************************************************/
Tu8 OSAL_Terminate_Task(ER_ID tsk_id);

#ifdef	OS_WIN32
/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_DeleteEvent is to delete the created event object
*   \param[in]				Tu32 eventId -- Event ID
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to delete the Event object.
*   \post-condition			The event object gets deleted and is no longer valid.
*   \ErrorHandling    		Return 0 for SUCCESS
*									  else for FAILURE
*
******************************************************************************************************/
Tu8 OSAL_DeleteEvent(Tu32 eventId);

/*****************************************************************************************************/
/**	 \brief                 This API function OSAL_IsThreadTerminated is to check if the terminate event
*							object is signalled
*   \param[in]				Tu32 eventID -- Event ID
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to check the Terminate Event object status.
*   \post-condition			Termination status is obtained
*   \ErrorHandling    		Return 0 for Signalled
*									  else for FAILURE
*
******************************************************************************************************/
Tbool OSAL_IsThreadTerminated(Tu32 eventID);
#endif

#endif /* OSAL_API_H_ */
/*=============================================================================
    end of file
=============================================================================*/
