/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file win32_os_api.h																			    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of win32 os APIs									*
*																											*
*************************************************************************************************************/

#ifndef _WIN32_OS_API_H
#define _WIN32_OS_API_H

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "win32_os_private.h"

/*-----------------------------------------------------------------------------
function declarations extern
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_CreateSemaphore is to create semaphore
*   \param[in]				t_SemaphoreHandle* sem_handle -- Semaphore handle
*                           Tu32 initialValue			  -- Initial semaphore count
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to create semaphore
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void WIN32_CreateSemaphore(t_SemaphoreHandle* sem_handle, Tu32 initialValue, Tu32 maxValue);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_WaitSemaphore is to wait semaphore
*   \param[in]				t_SemaphoreHandle* sem_handle -- Semaphore handle
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to wait semaphore
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_WaitSemaphore(t_SemaphoreHandle* sem_handle);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_PostSemaphore is to post semaphore
*   \param[in]				t_SemaphoreHandle* sem_handle -- Semaphore handle
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to create semaphore
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void WIN32_PostSemaphore(t_SemaphoreHandle* sem_handle);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_DeleteSemaphore is to delete semaphore
*   \param[in]				Ts32 sem_id -- semaphore id
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to delete semaphore
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_DeleteSemaphore(Ts32 sem_id);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_CreateMB is to create mailbox
*   \param[in]				Ts_MailBox *mailbox -- pointer to mailbox
*                           Tu8 max_size_of_msg -- max size of msg
							void* buffer		-- buffer holding msgs
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to create mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void WIN32_CreateMB(Ts_MailBox *mailbox, Tu16 max_size_of_msg, void* buffer);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_CreateTimer is to Create timer
*   \param[in]				void(*cb_func)(Tu32 content) -- call back function with Tu32 content
							Tu32 time_ms				 -- Time duration
							Tu8 type					 -- Type of timer
							Tu32 content				 -- Content to be return through call back	
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Create timer
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_CreateTimer(void(*cb_func)(Tu32 content), Tu32 time_ms, Tu8 type, Tu32 content);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_StartTimer is to Start timer
*   \param[in]				Tu32 TimerId -- Timer Id
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Start timer
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_StartTimer(Tu32 TimerId, Tuint timerduration);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_StopTimer is to Stop timer
*   \param[in]				Tu32 TimerId -- Timer Id
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Stop timer
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_StopTimer(Tu32 TimerId);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_CreateMutex is to create Mutex
*   \param[in]				void
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to create Mutex
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts32 WIN32_CreateMutex(void);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Mutex_Lock is to lock Mutex
*   \param[in]				Ts32 mutexId -- Mutex Id
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to lock Mutex
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts32 WIN32_Mutex_Lock(Ts32 mutexId);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Mutex_Unlock is to unlock Mutex
*   \param[in]				Ts32 mutexId -- Mutex Id
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to unlock Mutex
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts32 WIN32_Mutex_Unlock(Ts32 mutexId);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_CreateTask is to create thread
*   \param[in]				Ts_Win32Task *task -- Task creation structure
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to create thread
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_CreateTask(Ts_Win32Task *task);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_ThreadFunc is call back function
*   \param[in]				const LPVOID param -- return param
*   \param[out]				DWORD
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used as call back function
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
DWORD WINAPI WIN32_ThreadFunc(const LPVOID param);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_DeleteTask is to Resume the thread
*   \param[in]				t_Tu32 taskid -- Thread id
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Resume the thread
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts32 WIN32_TaskActivate(Tu32 taskid);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_DeleteTask is to Terminate(Delete) the thread
*   \param[in]				t_Tu32 taskid -- Thread id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Terminate(Delete) the thread
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu8 WIN32_DeleteTask(Tu32 taskid);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_PutMessage is to Get message from Mailbox
*   \param[in]				Ts_MailBox *mbox -- Mailbox pointer
*                           void* msg		 -- Message pointer to be stored
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Get message from Mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_GetMessage(Ts_MailBox *mailbox, void *message);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_GetMsg is to Get message from Mailbox
*   \param[in]				Ts_MailBox *mbox -- Mailbox pointer
*   \param[out]				const void*
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Get message from Mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
const void* WIN32_GetMsg(Ts_MailBox *mbox);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_PutMessage is to Store message in the mailbox
*   \param[in]				Ts_MailBox *mbox -- Mailbox pointer
*                           const void* msg	 -- Message pointer to be stored
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Store message in the mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_PutMessage(Ts_MailBox *mailbox, const void* message);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_StoreMsg is to Store message in the mailbox
*   \param[in]				Ts_MailBox *mbox -- Mailbox pointer
*                           const void* msg	 -- Message pointer to be stored
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Store message in the mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_StoreMsg(Ts_MailBox *mbox, const void* msg);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_CreateEvent is to Create event object
*   \param[in]				Tu32 taskId -- ThreadId
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to create event object
*   \post-condition			Returns the handle for created event object
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_CreateEvent(Tu32 taskId);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Terminate_Task is to suspend the thread
*   \param[in]				t_Tu32 taskid -- Thread id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to suspend the thread
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu8 WIN32_Terminate_Task(Tu32 taskid);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Delete_MailBox is to delete mailbox
*   \param[in]				Tu32 *mbx_id -- mailbox id
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to delete mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_Delete_MailBox(Tu32 *mbx_id);

/*****************************************************************************************************/
/**	 \brief                 The API Function WIN32_DeleteEvent is to delete event object
*   \param[in]				Tu32 eventId -- Event id
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to delete event object
*   \post-condition			Event gets deleted
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_DeleteEvent(Tu32 eventId);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Delete_Mutex is to delete Mutex
*   \param[in]				Ts32 MutexId -- MutexId
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to delete Mutex
*   \post-condition			Mutex gets deleted
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_Delete_Mutex(Ts32 MutexId);

/*****************************************************************************************************/
/**	 \brief                 The API Function WIN32_IsThreadTerminated is to check if the terminate event
*							object is signalled
*   \param[in]				Tu32 eventID -- Event ID
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function is used to check the Terminate Event object status.
*   \post-condition			Termination status is obtained
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool WIN32_IsThreadTerminated(Tu32 eventID);

#endif /* WIN32_OS_API_H */
/*=============================================================================
end of file
=============================================================================*/