/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_msg.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declarations of all functions used for sending and   *
*						  receiving data into/from messages													*
*																											*
*************************************************************************************************************/

#ifndef AMFM_APP_MSG_H
#define AMFM_APP_MSG_H

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/

#include "amfm_app_types.h"
#include "sys_task.h"



/*-----------------------------------------------------------------------------
    Function Declarations
-----------------------------------------------------------------------------*/
void AMFM_APP_MessageInit(Ts_Sys_Msg *pst_msg ,Tu16 u16_DestCID,Tu16 u16_MsgID,Tu16 u16_SrcCID);
void AMFM_APP_UpdateParameterIntoMessage(Tchar *pu8_data,const void *vp_parameter,Tu8 u8_ParamLength,Tu16 *pu16_Datalength);
void AMFM_APP_ExtractParameterFromMessage(void *vp_Parameter,const Tchar *pu8_DataSlot,Tu16 u8_ParamLength,Tu32 *pu32_index);

#endif /* End of AMFM_APP_MSG_H */