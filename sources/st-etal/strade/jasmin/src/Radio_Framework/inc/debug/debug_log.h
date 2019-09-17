/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file debug_log.h																				    	*
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
#ifndef DEBUG_LOG_H_
#define DEBUG_LOG_H_

#include "cfg_types.h"

/******************************************************************************
    Macro Definitions 
******************************************************************************/
#if 0
#define PRINT_MSG_DATA(a)	
#define RADIO_DEBUG_LOG     SmLogPrint  /* 2016_11_28 added system debuglog  */  
#else
#define PRINT_MSG_DATA(a)	
#define RADIO_DEBUG_LOG     radio_debuglog 
#endif

/******************************************************************************
    type definitions
*******************************************************************************/
typedef enum
{
	RADIO_LOG_LVL_NO_LOG,		/* no log output */
	RADIO_LOG_LVL_ASSERT,		/* the highest log level, only logs with this level output */
	RADIO_LOG_LVL_ERROR,
	RADIO_LOG_LVL_WARNING,
	RADIO_LOG_LVL_NOTICE,
	RADIO_LOG_LVL_INFO,
	RADIO_LOG_LVL_DEBUG,
	RADIO_LOG_LVL_QUALITY,	
	RADIO_LOG_LVL_ALL_LOG		/* the lowest log level, all logs output */
}Te_Log_Level ;

/******************************************************************************
    Function Declarations
******************************************************************************/
#if 0
void PrintMessageData(Ts_Sys_Msg* pst_msg);
#endif

void Sys_Debug_Initialize(void);

void radio_debuglog(Te_Log_Level e_log_level, const Tchar* char_format,...);

#endif /* #ifndef DEBUG_LOG_H_ */
/*=============================================================================
    end of file
=============================================================================*/