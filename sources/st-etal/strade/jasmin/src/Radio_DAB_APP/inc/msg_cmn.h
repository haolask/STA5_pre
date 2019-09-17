/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file msg_cmn.h																							*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The File contains common functions to update and extract the message				*	
*																											*
*																											*
*************************************************************************************************************/

#ifndef __MSG_CMN_H__
#define __MSG_CMN_H__

#include "hsm_api.h"



void MessageHeaderWrite(Ts_Sys_Msg *pst_msg ,Tu16 u16_DestCID,Tu16 u16_MsgID,Tu16 u16_SrcCID);

void UpdateParameterInMessage(Tu8 *pu8_data,const void *vp_parameter,Tu8 u8_ParamLength,Tu16 *pu16_Datalength);

void DAB_App_ExtractParameterFromMessage(void *vp_Parameter,const Tchar *pu8_DataSlot,Tu8 u8_ParamLength,Tu32 *pu8_index);

#endif	/* __MSG_CMN_H__*/

/*=============================================================================
    end of file
=============================================================================*/