/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file win32_os.c																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains definitions of win32 os APIs									*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "win32_os_api.h"
#include "debug_log.h"

/*-----------------------------------------------------------------------------
variables (extern)
-----------------------------------------------------------------------------*/
static Ts_Win32Task win32_TaskList[WIN32_MAX_TASKS];	   /*win32 task array*/

/*-----------------------------------------------------------------------------
private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/* Ts32 WIN32_CreateMB                                                       */
/*===========================================================================*/
void WIN32_CreateMB(Ts_MailBox *mailbox, Tu16 max_size_of_msg, void* buffer)
{
	Ts_MailBox  *mbox = mailbox;
	Ts32 size;
	static unsigned char mb_index=0;

	mbox->mb_array			= (const void**)(buffer);   
	mbox->size				= max_size_of_msg;		 	
	mbox->count				= 0U;
	mbox->read				= 0U;
	mbox->write				= 0U;
	mbox->max_count_last	= 0U;
	mbox->mailboxid         = mb_index;

	// to assign index for next mailbox
	mb_index++;

	/* OSAL will only store pointers to the messages */
	size = sizeof(void*)* max_size_of_msg;
	WIN32_MemorySet(buffer, 0, size);

	WIN32_CreateSemaphore(&mbox->mb_semaphore, WIN32_SEMAPHORE_MAX_COUNT - 1, WIN32_SEMAPHORE_MAX_COUNT);
	InitializeCriticalSection(&mbox->crit);
	//InitializeCriticalSection(&mbox->crit_sem);
}

/*===========================================================================*/
/*  void WIN32_GetMessage                                                    */
/*===========================================================================*/
Tu32 WIN32_GetMessage(Ts_MailBox *mailbox, void *message)
{
	Ts_MailBox *mbox		= mailbox;
	void*		m;
	Tu32		ret			= 1U;		/*INITIALIZE WITH FAILURE*/
	Tbool		b_SemErr	= TRUE;

	/* wait for message */
	b_SemErr = WIN32_WaitSemaphore(&mbox->mb_semaphore);

	if (b_SemErr != TRUE && mbox->mb_semaphore != INVALID_HANDLE_VALUE)
	{
		if (mbox != NULL)
		{

			EnterCriticalSection(&mbox->crit);
			/*****************************************/
			m = (void*)WIN32_GetMsg(mbox);

			if (m != NULL)
			{
				WIN32_MemoryCopy(message, &m, sizeof(void*));
				ret = 0U;	/*SUCCESS*/
			}
			/*****************************************/
			LeaveCriticalSection(&mbox->crit);
		}
		else
		{

		}
	}
	else
	{
		/* Do Nothing */
	}
	return ret;			
}

/*===========================================================================*/
/*  const void* WIN32_GetMsg												 */
/*===========================================================================*/
const void* WIN32_GetMsg(Ts_MailBox *mbox)
{
	const void* msg = NULL;

	if (mbox->read != mbox->write)
	{
		do {
			/* read */
			msg = mbox->mb_array[mbox->read];
			if (msg)
			{
				mbox->mb_array[mbox->read] = NULL;
				mbox->count--;
				mbox->read++;
			}

			if (mbox->read >= mbox->size)
			{
				mbox->read = 0U;
			}

		} while ((msg == NULL) && (mbox->read != mbox->write));
	}

	return msg;
}

/*===========================================================================*/
/* Tbool WIN32_PutMessage                                                    */
/*===========================================================================*/
Tbool WIN32_PutMessage(Ts_MailBox *mailbox, const void* message)
{
	Tbool        ret = TRUE;
	Ts_MailBox* l_mb;

	if (mailbox != NULL)
	{

		EnterCriticalSection(&mailbox->crit);
		ret = WIN32_StoreMsg(mailbox, message);
		LeaveCriticalSection(&mailbox->crit);
		if (ret == TRUE)
		{
			l_mb = mailbox;
			WIN32_PostSemaphore(&l_mb->mb_semaphore);
			ret = FALSE; /* SUCCESS */
		}
		else
		{
			ret = TRUE; /* FAILURE */
		}
	}
	else
	{

	}
	return ret;
}

/*===========================================================================*/
/*  Tbool WIN32_StoreMsg				                                     */
/*===========================================================================*/
Tbool WIN32_StoreMsg(Ts_MailBox *mbox, const void* msg)
{
	Tbool			ret = FALSE;

	/* store */
	if ((((mbox->write) + 1U) == (mbox->read)) || (((mbox->write) == ((mbox->size) - 1U)) && ((mbox->read) == 0U)))
	{
		ret = FALSE; /*FAILURE*/
	}
	else if (mbox->mb_array)
	{
		mbox->mb_array[mbox->write] = (void**)msg;
		mbox->write++;
		if (mbox->write >= mbox->size)
		{
			mbox->write = 0U;
		}
		mbox->count++;
		if (mbox->count > mbox->max_count_last)
		{
			mbox->max_count_last = mbox->count;
		}
		/* ok */
		ret = TRUE; /*SUCCESS*/
	}
	else
	{
		ret = FALSE; /*FAILURE*/
	}

	return ret;
}

/*===========================================================================*/
/*  Ts32 WIN32_CreateMutex(void)                                             */
/*===========================================================================*/
Ts32 WIN32_CreateMutex(void)
{
	Ts32 RET;
	HANDLE hMutex;
	// Create a mutex with no initial owner
	hMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (hMutex != NULL)
	{
		RET = (Ts32)hMutex; /*SUCCESS*/
	}
	else
	{
		RET = -1; /*FAILURE*/
	}

	return RET;
}

/*===========================================================================*/
/*  Ts32 WIN32_Mutex_Lock(Ts32 mutexId)                                      */
/*===========================================================================*/
Ts32 WIN32_Mutex_Lock(Ts32 mutexId)
{
	Ts32 RET;
	HANDLE hMutex = (HANDLE)mutexId;
	RET = WaitForSingleObject(
		hMutex,    // handle to mutex
		INFINITE);  // no time-out interval

	return RET;
}

/*===========================================================================*/
/*  Ts32 WIN32_Mutex_Unlock(Ts32 mutexId)                                    */
/*===========================================================================*/
Ts32 WIN32_Mutex_Unlock(Ts32 mutexId)
{
	Ts32 RET;
	HANDLE hMutex = (HANDLE)mutexId;
	RET = ReleaseMutex(hMutex); // handle to mutex

	return RET;
}

/*===========================================================================*/
/*  Tu32 WIN32_CreateTask(Ts_Win32Task *task)                                */
/*===========================================================================*/
Tu32 WIN32_CreateTask(Ts_Win32Task *task)
{
	Ts_Win32Task *ltask = task;
	Tu32 result;
	Tu8 task_count;
	Tbool task_added = FALSE;

	ltask->threadhandle = CreateThread(NULL, (Tu32)task->stacksize, WIN32_ThreadFunc, task->func, CREATE_SUSPENDED, &ltask->threadid);

	task->threadid = ltask->threadid;

	if (ltask->threadhandle == INVALID_HANDLE_VALUE)
	{
		/* Create Thread failed... */
		return 0; /*FAILURE*/
	}
	else
	{

		result = (Tu32)SetThreadPriority(ltask->threadhandle, (Ts32)(ltask->priority));

		if (!result)
		{
			/* SetThreadPriority failed... */
			return 0; /*FAILURE*/
		}
		else
		{
			for (task_count = 0; task_count < WIN32_MAX_TASKS && !task_added; task_count++)
			{
				if (win32_TaskList[task_count].threadid == 0)
				{
					win32_TaskList[task_count].threadid = ltask->threadid;
					win32_TaskList[task_count].threadhandle = ltask->threadhandle;
					win32_TaskList[task_count].suspended = TRUE;
					task_added = TRUE;
				}
			}
			return (Tu32)ltask->threadid; /*SUCCESS*/
		}		
	}
}

/*===========================================================================*/
/*  Ts32 WIN32_TaskActivate(Tu32 taskid)					                 */
/*===========================================================================*/
Ts32 WIN32_TaskActivate(Tu32 taskid)
{
	Tu8 task_count;
	Tbool task_found = FALSE;

	for (task_count = 0; task_count < WIN32_MAX_TASKS && !task_found; task_count++)
	{
		if (win32_TaskList[task_count].threadid == taskid)
		{
			task_found = TRUE;
		}
	}

	if (ResumeThread(win32_TaskList[task_count - 1].threadhandle) == (DWORD)-1)
	{
		/* Resume Thread failed... */
		return -1; /*FAILURE*/
	}
	else
	{
		win32_TaskList[task_count-1].suspended = FALSE;
		return 0; /*SUCCESS*/
	}
}

/*===========================================================================*/
/*  DWORD WINAPI WIN32_ThreadFunc(const LPVOID param)                 */
/*===========================================================================*/
DWORD WINAPI WIN32_ThreadFunc(const LPVOID param)
{
	if (param)
	{
		OSAL_TASK_FUNC  func = (OSAL_TASK_FUNC)param;
		func();
	}
	return (DWORD)0; /*SUCCESS*/
}

/*===========================================================================*/
/*  Tu8 WIN32_DeleteTask(Tu32 taskid)						                 */
/*===========================================================================*/
Tu8 WIN32_DeleteTask(Tu32 taskid)
{
	Tu8 task_count;
	Tbool task_found = FALSE;
	Tu8 ret;

	for (task_count = 0; task_count < WIN32_MAX_TASKS && !task_found; task_count++)
	{
		if (win32_TaskList[task_count].threadid == taskid)
		{
			task_found = TRUE;
		}
	}

	ret = SetEvent(win32_TaskList[task_count - 1].terminateEvent);
	if (ret == 0)
	{
		return 1; /* FAILURE */
	}
	else
	{
		return 0; /* SUCCESS */
	}
}

/*===========================================================================*/
/*  Tu8 WIN32_Terminate_Task(Tu32 taskid)					                 */
/*===========================================================================*/
Tu8 WIN32_Terminate_Task(Tu32 taskid)
{
	Tu8 task_count;
	Tbool task_found = FALSE;

	for (task_count = 0; task_count < WIN32_MAX_TASKS && !task_found; task_count++)
	{
		if (win32_TaskList[task_count].threadid == taskid)
		{
			task_found = TRUE;
		}
	}

	if (SuspendThread(win32_TaskList[task_count - 1].threadhandle) == (DWORD)-1)
	{
		return 1; /* FAILURE */
	}
	else
	{
		return 0; /* SUCCESS */
	}
}


/*===========================================================================*/
/*  Tbool WIN32_Delete_MailBox(Tu32 *mbx_id)					             */
/*===========================================================================*/
Tbool WIN32_Delete_MailBox(Tu32 *mbx_id)
{
	Ts_MailBox *mbx = (Ts_MailBox *)mbx_id;
	Tbool b_ret = TRUE;

	(void)ReleaseSemaphore(mbx->mb_semaphore,  /*handle to semaphore*/
		1,            /*increase count by one*/
		NULL);       /* not interested in previous count*/
	
	b_ret = WIN32_DeleteSemaphore((Ts32)mbx->mb_semaphore);

	if (b_ret != TRUE)
	{
		DeleteCriticalSection(&(mbx->crit));

		WIN32_MemorySet(mbx_id, 0U, sizeof(Ts_MailBox));
		mbx->mb_semaphore = INVALID_HANDLE_VALUE;
		mbx_id = NULL;
	}
	else
	{
		/* Do Nothing */
	}
	return b_ret; /*SUCCESS*/
}

/*===========================================================================*/
/*  void WIN32_CreateSemaphore                                               */
/*===========================================================================*/
void WIN32_CreateSemaphore(t_SemaphoreHandle* sem_handle, Tu32 initialValue, Tu32 maxValue)
{
	*sem_handle = CreateSemaphore(NULL,                 /* default security attributes */
		(LONG)initialValue,								/* initial count */
		(LONG)maxValue,									/* maximum count */
		(LPCTSTR)NULL);									/* name  can be used for process sharing */
}

/*===========================================================================*/
/* Tbool WIN32_WaitSemaphore                                                 */
/*===========================================================================*/
Tbool WIN32_WaitSemaphore(t_SemaphoreHandle* sem_handle)
{
	DWORD dwWaitResult;
	Tbool bContinue;
	Tbool b_err = TRUE;

	if (*sem_handle != INVALID_HANDLE_VALUE)
	{
		bContinue = (Tbool)TRUE;
	}
	else
	{
		bContinue = (Tbool)FALSE;
		b_err = TRUE; /* Return Failure */
	}

	while (bContinue)
	{
		dwWaitResult = WaitForSingleObject(*sem_handle,   /* handle to semaphore*/
			INFINITE);									  /* infinite time-out interval*/

		switch (dwWaitResult)
		{
			/* The semaphore object was signaled.*/
			case WAIT_OBJECT_0:
			{
				bContinue = FALSE;
				b_err = FALSE; /* Return Success */
			}
			break;

			case WAIT_TIMEOUT:
			{
				bContinue = FALSE;
				b_err = TRUE; /* Return Failure */
			}
			break;

			default:
			{
				b_err = TRUE; /* Return Failure */
			}
			break;
		}
	}

	return b_err;
}

/*===========================================================================*/
/* void WIN32_PostSemaphore                                                  */
/*===========================================================================*/
void WIN32_PostSemaphore(t_SemaphoreHandle* sem_handle)
{
	if (*sem_handle != INVALID_HANDLE_VALUE)
	{
		if (!ReleaseSemaphore(*sem_handle,  /*handle to semaphore*/
			1,            /*increase count by one*/
			NULL))       /* not interested in previous count*/
		{
			/* ReleaseSemaphore failed... */
		}
	}
	else
	{
		/* Invalid Handle... */
	}
}

/*===========================================================================*/
/*  Tbool WIN32_DeleteSemaphore(Ts32 sem_id)					                 */
/*===========================================================================*/
Tbool WIN32_DeleteSemaphore(Ts32 sem_id)
{
	Tbool ret;

	ret = CloseHandle((HANDLE)sem_id);

	if (ret == FALSE)
	{
		return TRUE; /*FAILURE*/
	}
	else
	{
		return FALSE; /*SUCCESS*/
	}
}

/*===========================================================================*/
/*  Tbool WIN32_DeleteEvent(Tu32 eventId)					                 */
/*===========================================================================*/
Tbool WIN32_DeleteEvent(Tu32 eventId)
{
	Tbool ret;

	ret = CloseHandle((HANDLE)eventId);

	if (ret == FALSE)
	{
		return TRUE; /*FAILURE*/
	}
	else
	{
		return FALSE; /*SUCCESS*/
	}
}

/*===========================================================================*/
/*  Tbool WIN32_IsThreadTerminated(Tu32 eventID)					         */
/*===========================================================================*/
Tbool WIN32_IsThreadTerminated(Tu32 eventID)
{
	DWORD dwWaitResult;

	dwWaitResult = WaitForSingleObject((HANDLE)eventID, WIN32_EVENT_NO_WAIT);

	if (dwWaitResult == WAIT_OBJECT_0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*===========================================================================*/
/*  Tu32 WIN32_CreateEvent(void)									         */
/*===========================================================================*/
Tu32 WIN32_CreateEvent(Tu32 taskid)
{
	HANDLE hEvent;
	Tu8 task_count;
	Tbool task_found = FALSE;
	DWORD err;

	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (hEvent != NULL)
	{
		for (task_count = 0; task_count < WIN32_MAX_TASKS && !task_found; task_count++)
		{
			if (win32_TaskList[task_count].threadid == taskid)
			{
				task_found = TRUE;
			}
		}

		win32_TaskList[task_count - 1].terminateEvent = hEvent;
	}
	else
	{
		err = GetLastError();
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][WIN32] Event creation failed due to: %lu\n", err);
	}
	return (Tu32)hEvent;
}

/*===========================================================================*/
/*  Tbool WIN32_Delete_Mutex(Ts32 MutexId)					                 */
/*===========================================================================*/
Tbool WIN32_Delete_Mutex(Ts32 MutexId)
{
	Tbool ret;

	ret = CloseHandle((HANDLE)MutexId);

	if (ret == FALSE)
	{
		return TRUE; /*FAILURE*/
	}
	else
	{
		return FALSE; /*SUCCESS*/
	}
}

/*=============================================================================
end of file
=============================================================================*/