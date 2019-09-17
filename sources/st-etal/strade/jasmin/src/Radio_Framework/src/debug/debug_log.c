/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file debug_log.c																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains typedefinitions of datatypes.									*
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
------------------------------------------------------------------------------*/
#include "debug_log.h"
#include <stdarg.h>

#include <Windows.h>
#include <stdio.h>

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (Global)
-----------------------------------------------------------------------------*/
Te_Log_Level e_global_var_switch = RADIO_LOG_LVL_DEBUG; /* Global variable switch for filter logs*/

Tu8 u8_dummy_cnt = 0;

CRITICAL_SECTION debug_handle;

char buffer[256];  /* To collect variable arguments information */
/*----------------------------------------------------------------------------
    Function Definitions
------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
   void PrintMessageData(Ts_Sys_Msg* pst_msg)
------------------------------------------------------------------------------*/
#if 0
void PrintMessageData(Ts_Sys_Msg* pst_msg)
{
	Tu8		u8_DebugIndex;	
	for(u8_DebugIndex=0; u8_DebugIndex < pst_msg->msg_length ;u8_DebugIndex++)
	{
		//RADIO_DEBUG_LOG("%x  ",pst_msg->data[u8_DebugIndex]);  /* 29092016 Comment */
	}
	
}
#endif

void Sys_Debug_Initialize(void)
{
	InitializeCriticalSection(&debug_handle);
}
/******************************************************************************
   void radio_debuglog(Tu8 u8_level, const Tchar* char_format,...)
******************************************************************************/
void radio_debuglog(Te_Log_Level e_log_level, const Tchar* char_format,...)
{
	EnterCriticalSection(&debug_handle);
	(void)memset(buffer, 0, sizeof(buffer));
	if(e_global_var_switch >= e_log_level)
	{
		va_list args;
		va_start(args, char_format);
        (void)vsprintf(buffer,char_format, args);
		printf("%s\n",buffer);
		va_end(args);
	}
	else
	{
			/*Do nothing*/
	}
	LeaveCriticalSection(&debug_handle);
}

/*=============================================================================
    end of file
=============================================================================*/
