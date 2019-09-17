/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file win32_os_private.h																		    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of win32 os private								*
*																											*
*************************************************************************************************************/

#ifndef _WIN32_OS_PRIVATE_H
#define _WIN32_OS_PRIVATE_H

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "cfg_types.h"
#include <Windows.h>
#include <stdio.h>

/* ---------------------------------------------------------------------------- -
defines
---------------------------------------------------------------------------- - */
/**
* @brief the max count value of the WinCe semaphore
*
*  this is maximum count value for semaphore creation+
*  at the moment set fix to infinte.
*/

#define WIN32_SEMAPHORE_MAX_COUNT (0x100)	//Max msgs 256
/**
* @brief Maximum number of window tasks
*/
#define WIN32_MAX_TASKS          ((Tu16)4)

#define WIN32_EVENT_NO_WAIT			0u


// Memory support functions
#define WIN32_MemorySet(_d_, _s_, _size_)  memset((_d_), (_s_), (_size_))
#define WIN32_MemoryCopy(_d_, _s_, _size_) memcpy((_d_), (_s_), (_size_))
#define WIN32_MemoryMove(_d_, _s_, _size_) memmove((_d_), (_s_), (_size_))

/*-----------------------------------------------------------------------------
type definitions
-----------------------------------------------------------------------------*/

typedef HANDLE t_SemaphoreHandle;

/**
* @brief Task function pointer
*
* pointer type for task function call
*/
typedef void(*OSAL_TASK_FUNC)  (void);

/**
* @brief Timer function pointer
*
* pointer type for timer function call
*/
typedef void(*OSAL_TIMER_FUNC) (void);

typedef struct Ts_MailBox
{
	const void*			*mb_array;		   /* mailbox array								*/
	t_SemaphoreHandle	mb_semaphore;      /* mailbox semaphore							*/
	CRITICAL_SECTION    crit;              /* critical section for normal use			*/
	CRITICAL_SECTION    crit_sem;          /* critical section for semaphore use		*/
	Tu16                size;              /* mailbox size								*/
	Tu8                 read;              /* mailbox read index						*/
	Tu8                 write;             /* mailbox write index						*/
	Tu8                 count;             /* message count								*/
	Tu8                 max_count_last;    /* last msg count							*/
	Tu8					mailboxid;		   /* To identify the mailbox - for debugging	*/
} Ts_MailBox;

typedef struct Ts_Win32Task
{
	Tu8                 priority;         /* system priority					*/
	HANDLE				threadhandle;	  /* thread handle                      */
	DWORD               threadid;         /* the thread's id					*/
	OSAL_TASK_FUNC      func;             /* pointer to the task fuction		*/
	void*               stack;            /* stack pointer						*/
	Tu32                stacksize;        /* the thread's stack size			*/
	Tbool               suspended;        /* thread start state					*/
	const char*			name;			  /* task name							*/
	HANDLE				terminateEvent;
}Ts_Win32Task;

/** Task ID */
typedef unsigned char T_taskid;

#endif  /* WIN32_OS_PRIVATE_H */
/*=============================================================================
end of file
=============================================================================*/