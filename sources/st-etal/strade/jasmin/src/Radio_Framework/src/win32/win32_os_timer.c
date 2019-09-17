/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file win32_os_timer.c																				    *
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains definitions of win32 timers									*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "win32_os_timer.h"
#include "sys_task.h"

/*-----------------------------------------------------------------------------
variables (Static)
-----------------------------------------------------------------------------*/
static HANDLE hTimerQueue = NULL;
static Ts_Timer_msg t_msg[MAX_TIMERS];

static Tu8 no_of_active_timers = 0;

CRITICAL_SECTION critic_sect;

/*-----------------------------------------------------------------------------
private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/* Ts32 WIN32_Timer_Init                                                     */
/*===========================================================================*/
Tu32 WIN32_Timer_Init()
{
	Tu8 loopCount = 0;

	// Reset TimerIds
	for (loopCount = 0; loopCount<MAX_TIMERS; ++loopCount)
	{
		t_msg[loopCount].timerCount = 0;
		t_msg[loopCount].timerId = 0;
	}

	// Create the timer queue.
	hTimerQueue = CreateTimerQueue();
	if (NULL == hTimerQueue)
	{
		/* CreateTimerQueue failed... */
		return 0;
	}
	else
	{
		InitializeCriticalSection(&critic_sect);
		return 1;
	}
}

/*===========================================================================*/
/* VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)       */
/*===========================================================================*/
VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	UNUSED(TimerOrWaitFired);
	// EnterCriticalSection for accessing a global variable
	EnterCriticalSection(&critic_sect);

	if (lpParam == NULL)
	{
		/* TimerRoutine lpParam is NULL... */
	}
	else
	{
		// lpParam points to the argument; in this case it is a typecasted address of an array element of structure array

		Ts_Timer_msg * pt_msg = (Ts_Timer_msg *)lpParam;
		
		// Mark the timer as inactive
		pt_msg->timerCount = 0;

		// Invoke User callback
		pt_msg->fp_timer_routine(pt_msg->content);

		// Reset timerId at timer expiry
		pt_msg->timerId = 0;

		// A Timer expired. Decrement the count of active timers
		if (no_of_active_timers>0)
		{
			--no_of_active_timers;
		}
		else
		{
			/* Only if timer active, the callback will be invoked; Hence, Control will not reach here*/
		}
	}

	// LeaveCriticalSection after usage completion
	LeaveCriticalSection(&critic_sect);
}

/*===========================================================================*/
/* Tu32 WIN32_Create_Timer(fp_timer_callback win32_timer_callback, \		 */
/*			Tu32 u32_Timerduration, Tu32  repeat_interval, Tu32 u32_content) */
/*===========================================================================*/
Tu32 WIN32_Create_Timer(fp_timer_callback win32_timer_callback, \
				Tu32 u32_Timerduration, Tu32  repeat_interval, Tu32 u32_content)
{

	HANDLE hTimer;
	static Tu8 curnt_timer = 0U;
	PVOID pt_msg = NULL;
	BOOL timerAvailable = TRUE;
	Tu8 loopCount = 0;
	BOOL STOP_LOOP = FALSE;
	Tu8 allocated_index = 0;

	if (curnt_timer < MAX_TIMERS)
	{
		// EnterCriticalSection for accessing a global variable
		EnterCriticalSection(&critic_sect);

		t_msg[curnt_timer].content = u32_content;
		t_msg[curnt_timer].fp_timer_routine = win32_timer_callback;
		t_msg[curnt_timer].timerCount = (curnt_timer+1);

		// pointer to the message to be read from callback
		pt_msg = (PVOID)&t_msg[curnt_timer];

		// For storing timerId to allocated timer struct
		allocated_index = curnt_timer;

		// assigning message memory for timers
		++curnt_timer;

		// keep track of total running timers
		++no_of_active_timers;

		// LeaveCriticalSection after usage completion
		LeaveCriticalSection(&critic_sect);

	}
	else if (no_of_active_timers < MAX_TIMERS)
	{
		for (loopCount = 0; (loopCount < MAX_TIMERS) && (!STOP_LOOP); ++loopCount)
		{
			if (t_msg[loopCount].timerCount == 0)
			{
				t_msg[loopCount].content = u32_content;
				t_msg[loopCount].fp_timer_routine = win32_timer_callback;
				t_msg[loopCount].timerCount = (loopCount+1);

				// pointer to the message to be read from callback
				pt_msg = (PVOID)&t_msg[loopCount];

				// For storing timerId to allocated timer struct
				allocated_index = loopCount;

				// EnterCriticalSection for accessing a global variable
				EnterCriticalSection(&critic_sect);

				// keep track of total running timers
				++no_of_active_timers;

				// LeaveCriticalSection after usage completion
				LeaveCriticalSection(&critic_sect);

				// Exit loop
				STOP_LOOP = TRUE;
			}
		}

		if (STOP_LOOP == FALSE)
		{
			//Timer Unavailable;  Don't create timer
			timerAvailable = FALSE;
			/* Timer Resource Unavailable */
		}
		else
		{
			/* Timer Memory allocated in struct. Do nothing */
		}

	}
	else
	{
		//Timer Unavailable;  Don't create timer
		timerAvailable = FALSE;
		/* Timer Struct Resource Unavailable. Timer creation failed in OS */
	}


	if (timerAvailable != FALSE)
	{
		// Set a timer to call the timer routine in given milliseconds
		if (!CreateTimerQueueTimer(&hTimer, hTimerQueue,
			(WAITORTIMERCALLBACK)TimerRoutine, (PVOID)pt_msg, (DWORD)u32_Timerduration, (DWORD)repeat_interval, 0))
		{
			/*CreateTimerQueueTimer failed. */
			return 0;
		}
		else
		{
			// Store timerId to allocated timer struct
			t_msg[allocated_index].timerId = (Tu32)hTimer;
			return (Tu32)hTimer;
		}
	}
	else
	{
		/*CreateTimerQueueTimer failed. Timer memory unavailable*/
		return 0;
	}
}

/*===========================================================================*/
/* Tu8 WIN32_Delete_Timer(Tu32 u32_timerId)								     */
/*===========================================================================*/
Tu8 WIN32_Delete_Timer(Tu32 u32_timerId)
{
	Tu8 delCount;
	BOOL LOOP_END = FALSE;

	if (!DeleteTimerQueueTimer(hTimerQueue, (HANDLE)u32_timerId, NULL))
	{
		/*DeleteTimerQueueTimer failed. */
		return 0;
	}
	else
	{
		for (delCount = 0; (delCount < MAX_TIMERS) && (!LOOP_END); delCount++)
		{
			if (t_msg[delCount].timerId == u32_timerId)
			{

				// EnterCriticalSection for accessing a global variable
				EnterCriticalSection(&critic_sect);

				// Mark timer as Inactive
				t_msg[delCount].timerCount = 0;

				// Reduce the active timer count by one
				--no_of_active_timers;

				// Reset the timerId
				t_msg[delCount].timerId = 0;

				// LeaveCriticalSection after usage completion
				LeaveCriticalSection(&critic_sect);

				//Terminate Loop
				LOOP_END = TRUE;
			}
		}
		return 1;
	}
}