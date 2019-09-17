/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file msg_cmn.c																						  	*
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

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "hsm_api.h"


/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void MessageHeaderWrite                                        			 */
/*===========================================================================*/
void MessageHeaderWrite(Ts_Sys_Msg *pst_msg ,Tu16 u16_DestCID,Tu16 u16_MsgID,Tu16 u16_SrcCID)
{
    if(pst_msg != NULL)
    {
        /* Clearing the message buffer */
        memset( (void *)pst_msg,(int) 0,sizeof(Ts_Sys_Msg) );
                        
        pst_msg -> src_id                = u16_SrcCID;
        pst_msg -> dest_id               = u16_DestCID;
        pst_msg -> msg_id                = u16_MsgID;
                                        
    }
    else
    {
        // Send error message
    }
		
}

/*===========================================================================*/
/*  void UpdateParameterInMessage                                        	 */
/*===========================================================================*/
void UpdateParameterInMessage(Tu8 *pu8_data,const void *vp_parameter,Tu8 u8_ParamLength,Tu16 *pu16_Datalength)
{
    /* Copying parameter to the data slots in Ts_Sys_Msg structure */
     SYS_RADIO_MEMCPY(pu8_data+ *pu16_Datalength,vp_parameter,(size_t)(u8_ParamLength));

    /* Updating msg_length for each parameter which represents length of data in Ts_Sys_Msg structure */
      *pu16_Datalength = (Tu16)(*pu16_Datalength + u8_ParamLength);
                
}

/*===========================================================================*/
/*  void DAB_App_ExtractParameterFromMessage                                       	 */
/*===========================================================================*/
void DAB_App_ExtractParameterFromMessage(void *vp_Parameter,const Tchar *pu8_DataSlot,Tu8 u8_ParamLength,Tu32 *pu8_index)
{
    /* Reading parameter from the data slot present in Ts_Sys_Msg structure  */
    SYS_RADIO_MEMCPY(vp_Parameter,pu8_DataSlot+*pu8_index,(size_t)(u8_ParamLength));

    /* Updating index inorder to point to next parameter present in the data slot in  Ts_Sys_Msg structure */
    *pu8_index = *pu8_index + u8_ParamLength;
                
}

/*=============================================================================
    end of file
=============================================================================*/