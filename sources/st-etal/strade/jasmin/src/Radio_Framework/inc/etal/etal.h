/*============================================================================================================
start of file
============================================================================================================*/

/************************************************************************************************************/
/** \file etal.h																							*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework																    *
*  Description			: This file contains declarations of ETAL Startup APIs								*
*										                                                                    *
*************************************************************************************************************/

#ifndef ETAL_H
#define ETAL_H

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/

#include "etal_types.h"
#include "etal_api.h"
#include "etalversion.h"
#include "debug_log.h"
#include "lib_string.h"
#include "sys_task.h"
#include <stddef.h>
#include "IRadio.h"
/*-----------------------------------------------------------------------------
defines
-----------------------------------------------------------------------------*/
#define TUNER_0							0
#define TUNER_1							1
/*-----------------------------------------------------------------------------
function declarations extern
-----------------------------------------------------------------------------*/
Ts_Sys_Msg* ETAL_SYS_MSG_HANDLE(Tu16 cid, Tu16 msgid);
void ETAL_UpdateParameterIntoMessage(Tchar *pu8_data, const void *vp_parameter, Tu8 u8_ParamLength, Tu16 *pu16_Datalength);
Tbool ETAL_Init(Radio_EtalHardwareAttr attr);
Tbool ETAL_Config(void);
ETAL_STATUS ETAL_Initialize(EtalHardwareAttr *pInitParams);
ETAL_STATUS ETAL_Get_Capabilities(EtalHwCapabilities  **pCapabilities);
ETAL_STATUS ETAL_Get_Version(EtalVersion   *vers);
ETAL_STATUS ETAL_Config_AudioPath(void);
Tbool ETAL_Deinit(void);
void ETAL_Event_Callback(void *pvContext, ETAL_EVENTS dwEvent, void *pvParams);
void ETAL_print_version(EtalVersion   *vers);
void AMFM_Tuner_ctrl_Quality_Monitor_CB_Notify(EtalBcastQualityContainer* pQuality, void* vpContext);
#endif /* ETAL_H */

/*=============================================================================
end of file
=============================================================================*/