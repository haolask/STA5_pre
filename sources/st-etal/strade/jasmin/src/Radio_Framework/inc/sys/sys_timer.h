/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_timer.h																				    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of timer private									*
*																											*
*************************************************************************************************************/

#ifndef _SYS_TIMER_H
#define _SYS_TIMER_H

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "osal_api.h"
#ifdef UITRON
#include "xcpbasic.h"
#include "Sw_timer_api.h"
#elif defined OS_WIN32
#include "win32_os_timer.h"
#endif
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
#define		SYS_TIMER_CNT   	11

/** Timer id's  ***/
#define 	SYS_TIMER_0     	0		/*Timer0 for AMFM_TUNER_CTRL*/
#define 	SYS_TIMER_1     	1		/*Timer1 for AMFM_TUNER_CTRL*/
#define 	SYS_TIMER_2     	2		/*Timer2 for AMFM_TUNER_CTRL*/
#define 	SYS_TIMER_3     	3		/*Timer3 for DAB_SPI_HANDLER*/
#define 	SYS_TIMER_4     	4		/*Timer4 for DAB_TUNER_CTRL */
#define 	SYS_TIMER_5     	5		/*Timer5 for Radio_AMFM_APP */
#define 	SYS_TIMER_6     	6		/*Timer6 for RADIO_FM_RDS_DECODER*/
#define 	SYS_TIMER_7     	7		/*Timer7 for Calibration tool*/
#define 	SYS_TIMER_8     	8		/*Timer8 for Radio Manager*/
#define 	SYS_TIMER_9     	9		/*Timer9 for AMFM_APP*/
#define 	SYS_TIMER_10     	10		/*Timer10 for DAB_TUNER_CTRL */

#define 	SYS_TIMER_START		SYS_StartTimer  
#define 	SYS_TIMER_STOP   	SYS_StopTimer 
#ifdef OS_WIN32
#define TMR_TYPE_ONE_TIME		0		/* Macro for Repetition times of Timer */
#endif

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
typedef struct Ts_sys_timer
{	
	void*  		timer_funcptr; 
	ER_ID       timerid;
	Tu16        destid;
	Tu16        msgid;		
}Ts_System_Timer;

/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    function declarations extern
-----------------------------------------------------------------------------*/
/*****************************************************************************************************/
/**	 \brief                 This API function osal_sys_timer0 send message to mailbox based on dest_id.  
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                osal_sys_timer0 is a handler function to send message to mailbox 
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
void osal_sys_timer(Tu32 u32_content);

#ifdef UITRON
/*****************************************************************************************************/
/**	 \brief                 This API function is intializing the acre_alm os api. 
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API is creating timer to provide delay  
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
Ts8 SYS_StartTimer(Tu32 u32_Timerduration, Tu16 u16_msgId, Tu16 u16_destId);

/*****************************************************************************************************/
/**	 \brief                 The API Function SYS_StopTimer is stop the timer 
*   \param[in]				Tu8 TimerId -- Timer id
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function stops the timer with respect to TimerId
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
Tbool SYS_StopTimer(Tu8 u8_timerId);
#elif defined OS_WIN32
/*****************************************************************************************************/
/**	 \brief                 The API Function  WIN32_Create_Timer is to Create timer
*   \param[in]				fp_timer_callback		-- pointer to timer call back function
							Tu32 u32_Timerduration	-- timer duration
							Tu16 u16_msgId			-- Messgae Id
							Tu16 u16_destId			-- Destination Component Id
*   \param[out]				Tu32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Create timer
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tu32 SYS_StartTimer(Tu32 u32_Timerduration, Tu16 u16_msgId, Tu16 u16_destId);

/*****************************************************************************************************/
/**	 \brief                 The API Function SYS_StopTimer is stop the timer
*   \param[in]				Tu32 u32_timerId -- Timer id
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function stops the timer with respect to TimerId
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool SYS_StopTimer(Tu32 u32_timerId);
#endif

#endif /* SYS_TIMER_H */

/*=============================================================================
    end of file
=============================================================================*/
