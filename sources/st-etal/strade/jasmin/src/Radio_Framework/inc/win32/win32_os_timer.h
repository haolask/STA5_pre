/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file win32_os_timer.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of win32 timers APIs								*
*																											*
*************************************************************************************************************/
#ifndef _WIN32_OS_TIMER_H
#define _WIN32_OS_TIMER_H

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "win32_os_private.h"

/* ---------------------------------------------------------------------------- -
defines
---------------------------------------------------------------------------- - */
#define MAX_TIMERS 50 /*can be changed as per the requirement of components*/

/*-----------------------------------------------------------------------------
type definitions
-----------------------------------------------------------------------------*/
typedef void(*fp_timer_callback)(Tu32);

typedef struct Ts_Sys_Timer_msg
{
	Tu32 content;
	fp_timer_callback fp_timer_routine;
	Tu8 timerCount;
	Tu32 timerId;
}Ts_Timer_msg;

/*-----------------------------------------------------------------------------
function declarations extern
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Timer_Init is to Initialization of Timer Queue
*   \param[in]				void
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Initialization of Timer Queue
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_Timer_Init();

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Create_Timer is to Create timer
*   \param[in]				fp_timer_callback		-- pointer to timer call back function
							Tu32 u32_Timerduration	-- timer duration
							Tu32  repeat_interval	-- repeat interval
							Tu32 u32_content		-- content to be call back
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Create timer
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 WIN32_Create_Timer(fp_timer_callback, Tu32 u32_Timerduration, Tu32  repeat_interval, Tu32 u32_content);

/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Delete_Timer is to Delete timer
*   \param[in]				Tu32 u32_timerId	-- timer id
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Delete timer
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu8 WIN32_Delete_Timer(Tu32 u32_timerId);

#endif  /* WIN32_OS_TIMER_H */
/*=============================================================================
end of file
=============================================================================*/