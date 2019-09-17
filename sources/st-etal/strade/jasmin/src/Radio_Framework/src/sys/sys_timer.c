/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_message.c																				    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains sys timer API definitions										*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "sys_timer.h"
#include "sys_task.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/
/*===========================================================================
 void osal_sys_timer                                                
===========================================================================*/
void osal_sys_timer(Tu32 u32_content)
{
    Ts_Sys_Msg snd_timer_msg;
	Tu16 destid, msgid;
	
	destid = (Tu16)(u32_content & 0xFFFF);
	msgid = (Tu16)((u32_content & 0xFFFF0000) >> 16);
	
	snd_timer_msg.msg.msghead 	= (VP)NULL;
	snd_timer_msg.dest_id 		= destid;	
	snd_timer_msg.msg_id 		= msgid;	
	snd_timer_msg.msg_length 	= (Tu16)0;

	(void)SYS_SEND_MSG(&snd_timer_msg);
	
}
#ifdef UITRON
/*===========================================================================
  Ts8 SYS_StartTimer                                                      
===========================================================================*/
Ts8 SYS_StartTimer(Tu32 u32_Timerduration, Tu16 u16_msgId, Tu16 u16_destId)
{
    Ts8		s8_timerID;
	Tu32	u32_content;

	u32_content = (u16_msgId << 16) | (u16_destId);

	s8_timerID = SmTmrCre(osal_sys_timer, u32_Timerduration, TMR_TYPE_ONE_TIME, u32_content);

	if(s8_timerID > 0)
	{
	}	
    if(s8_timerID <= 0)
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] Timer%d creation is failed for msg_id:%x\n",s8_timerID,u16_msgId);
	}
	else
	{
		/* Do nothing*/
	}
	
	return s8_timerID;
}
/*===========================================================================
  Tbool Sys_StopTimer                                                       
===========================================================================*/
Tbool SYS_StopTimer(Tu8 u8_timerId)
{
	Tbool b_ret = TRUE;

	if(u8_timerId > 0)
	{
		if((SmTmrDel(u8_timerId)) < 0)
		{
			b_ret = FALSE;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] Timer%d stop is failed\n",u8_timerId);
			//System Restart may Handle
		}
		else
		{
			// Do nothing
		}	
	}
	else
	{
		/* Do nothing*/
	}
	return b_ret;	
}
#elif defined OS_WIN32
/*===========================================================================
Ts8 SYS_StartTimer
===========================================================================*/
Tu32 SYS_StartTimer(Tu32 u32_Timerduration, Tu16 u16_msgId, Tu16 u16_destId)
{
	Tu32	u32_timerID = 0u;
	Tu32	u32_content;

	u32_content = (u16_msgId << 16) | (u16_destId);

	//	cout << "the user content is " <<u32_content << "\n";

	u32_timerID = WIN32_Create_Timer(osal_sys_timer, u32_Timerduration, TMR_TYPE_ONE_TIME, u32_content);

	if (u32_timerID == 0)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] Timer%d creation is failed for msg_id:%x\n", u32_timerID, u16_msgId);
		return 0;
	}
	else
	{
		/* Timer Started. Return TimerID*/
		return u32_timerID;
	}

}

/*===========================================================================
Tbool Sys_StopTimer
===========================================================================*/
Tbool SYS_StopTimer(Tu32 u32_timerId)
{
	Tbool b_ret = TRUE;

	if (u32_timerId > 0)
	{
		if ((WIN32_Delete_Timer(u32_timerId)) == 0)
		{
			b_ret = FALSE;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] Timer%d stop is failed\n", u32_timerId);
			//System Restart may Handle
		}
		else
		{
			/* Do nothing*/
		}
	}
	else
	{
		/* Do nothing*/
	}
	return b_ret;
}
#endif
/*=============================================================================
    end of file
=============================================================================*/
