/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_request.c																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This source file consists of function defintions of all  Request APIs				*
*						  which will be	provided to Radio Manager Application 								*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
   File Inclusions
-----------------------------------------------------------------------------*/
#include "amfm_app_request.h"

/*-----------------------------------------------------------------------------
   File Inclusions
-----------------------------------------------------------------------------*/
Ts_Sys_Msg	st_amfm_app_req_msg;	/* used to send any request message to tuner ctrl component */



/*-----------------------------------------------------------------------------
    Public function definitions
-----------------------------------------------------------------------------*/

/*=========================================================================== */
/*			void AMFM_APP_MessageInit										  */
/*=========================================================================== */
void AMFM_APP_MessageInit(Ts_Sys_Msg *pst_msg ,Tu16 u16_DestCID,Tu16 u16_MsgID,Tu16 u16_SrcCID)
{
	if(pst_msg != NULL)
	{
			/* Clearing the message buffer */
			memset( (void *)pst_msg,AMFM_APP_CONSTANT_ZERO,sizeof(Ts_Sys_Msg) );
			
			pst_msg -> src_id		= u16_SrcCID;
			pst_msg -> dest_id		= u16_DestCID;
			pst_msg -> msg_id 		= u16_MsgID;
		
	}
	else
	{
			// Send error message
	}
}

/*=========================================================================== */
/*			void AMFM_APP_UpdateParameterIntoMessage						  */
/*=========================================================================== */
void AMFM_APP_UpdateParameterIntoMessage(Tchar *pu8_data,const void *vp_parameter,Tu8 u8_ParamLength,Tu16 *pu16_Datalength)
{
	/* copying parameter to the data slots in Ts_Sys_Msg structure */
	SYS_RADIO_MEMCPY((pu8_data+ *pu16_Datalength),vp_parameter,(Tu32)u8_ParamLength);
	//memcpy((pu8_data+ *pu16_Datalength),vp_parameter,(size_t)u8_ParamLength);

	/*  Updating msg_length for each parameter which represents length of data in  Ts_Sys_Msg structure   */
	*pu16_Datalength = (Tu16)(*pu16_Datalength + u8_ParamLength);

		
}

/*=========================================================================== */
/*			void ExtractParameterFromMessage								  */
/*=========================================================================== */
void AMFM_APP_ExtractParameterFromMessage(void *vp_Parameter,const Tchar *pu8_DataSlot,Tu16 u8_ParamLength,Tu32 *pu32_index)
{
	/* reading parameter from the data slot present in Ts_Sys_Msg structure  */
	SYS_RADIO_MEMCPY(vp_Parameter,(const void *)(pu8_DataSlot+*pu32_index),(Tu32)u8_ParamLength);
	//memcpy(vp_Parameter,(const void *)(pu8_DataSlot+*pu32_index),(size_t)u8_ParamLength);

	/*  Updating index inorder to point to next parameter present in the data slot in  Ts_Sys_Msg structure   */
	
	*pu32_index= *pu32_index + u8_ParamLength;
	
}

/*===========================================================================*/
/*			void AMFM_App_Request_Startup									 */
/*===========================================================================*/
void AMFM_App_Request_Startup(Te_AMFM_App_Market e_market,Tu8 u8_switch_setting,Tu8 u8_start_up_type)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_STARTUP_REQID,RADIO_MNGR_APP);

	/* Updating e_market value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),(const void *)&e_market,(Tu8)sizeof(Te_AMFM_App_Market),&pst_amfm_app_req_msg->msg_length);

	/* Updating e_market value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),(const void *)&u8_switch_setting,(Tu8)sizeof(u8_switch_setting),&pst_amfm_app_req_msg->msg_length);

	/* Updating e_market value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),(const void *)&u8_start_up_type,(Tu8)sizeof(u8_switch_setting),&pst_amfm_app_req_msg->msg_length);

	/* Sending asynchronous message */
	SYS_SEND_MSG(pst_amfm_app_req_msg);

}

/*===========================================================================*/
/*			void AMFM_APP_Request_Shutdown									 */
/*===========================================================================*/
void AMFM_APP_Request_Shutdown(void)
{

	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally  */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_SHUTDOWN_REQID,RADIO_MNGR_APP);
	
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);

}

/*===========================================================================*/
/*			void  AMFM_App_Request_GetStationList							 */
/*===========================================================================*/
void AMFM_App_Request_GetStationList(Te_AMFM_App_mode e_Mode)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally  */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_GET_STL_REQID,RADIO_MNGR_APP);

	/* Updating e_Mode value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),(const void *)&e_Mode,(Tu8)sizeof(e_Mode),&pst_amfm_app_req_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);


}

/*===========================================================================*/
/*			void  AMFM_App_Request_SelectBand								 */
/*===========================================================================*/
void AMFM_App_Request_SelectBand(Te_AMFM_App_mode e_mode)
{

	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_SELECT_BAND_REQID,RADIO_MNGR_APP);

	/* Updating e_mode value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_mode,(Tu8)sizeof(e_mode),&pst_amfm_app_req_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);

}

/*===========================================================================*/
/*			void  AMFM_App_Request_DeSelectBand							     */
/*===========================================================================*/
void AMFM_App_Request_DeSelectBand(Te_AMFM_App_mode e_mode)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	

	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_DESELECT_BAND_REQID,RADIO_MNGR_APP);

	/* Updating e_mode value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_mode,(Tu8)sizeof(e_mode),&pst_amfm_app_req_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}


/*===========================================================================*/
/*			void  AMFM_App_Request_PlaySelectSt								 */
/*===========================================================================*/
void AMFM_App_Request_PlaySelectSt(Tu32 u32_freq, Te_AMFM_App_mode e_mode)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_SELECT_STATION_REQID,RADIO_MNGR_APP);

	
	/* Updating u8_Index value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&u32_freq,(Tu8)sizeof(u32_freq),&pst_amfm_app_req_msg->msg_length);

	/* Updating e_mode value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_mode,(Tu8)sizeof(e_mode),&pst_amfm_app_req_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Response_SeekUpDown								 */
/*===========================================================================*/
void AMFM_App_Request_SeekUpDown(Tu32 u32_startfreq,Te_RADIO_DirectionType  e_Direction )
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_SEEK_UP_DOWN_REQID,RADIO_MNGR_APP);

    /* Updating startfrequency value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&u32_startfreq,(Tu8)sizeof(Tu32),&pst_amfm_app_req_msg->msg_length);

	/* Updating e_Direction value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_Direction,(Tu8)sizeof(Te_RADIO_DirectionType),&pst_amfm_app_req_msg->msg_length);

    
	
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Request_Cancel									 */
/*===========================================================================*/
void AMFM_App_Request_Cancel(void)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_CANCEL_REQID,RADIO_MNGR_APP);

  	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}


/*===========================================================================*/
/*		void AMFM_App_Request_FindBestPI              						 */
/*===========================================================================*/
void AMFM_App_Request_FindBestPI(Ts_AMFM_App_PIList st_FM_PIList,Tu32 u32_QualityMin,Tu32 u32_QualityMax,Tu32 u32_Implicit_sid,Tu8 u8_LinkType)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_FIND_BEST_PI_REQID,RADIO_MNGR_APP);

	/* Updating st_FM_PIList value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_req_msg->data,&st_FM_PIList,(Tu8)sizeof(Ts_AMFM_App_PIList),&pst_amfm_app_req_msg->msg_length);

	/* Updating u32_QualityMin value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_req_msg->data,&u32_QualityMin,(Tu8)sizeof(Tu32),&pst_amfm_app_req_msg->msg_length);

	/* Updating u32_QualityMax value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_req_msg->data,&u32_QualityMax,(Tu8)sizeof(Tu32),&pst_amfm_app_req_msg->msg_length);

	/* Updating u32_Implicit_sid value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_req_msg->data,&u32_Implicit_sid,(Tu8)sizeof(Tu32),&pst_amfm_app_req_msg->msg_length);
	
	/* Updating u8_LinkType value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_req_msg->data,&u8_LinkType,(Tu8)sizeof(Tu8),&pst_amfm_app_req_msg->msg_length);
	
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}

/*===========================================================================*/
/*		void AMFM_App_Request_BlendingStatus              					 */
/*===========================================================================*/
void AMFM_App_Request_BlendingStatus(Te_RADIO_DABFM_LinkingStatus	e_LinkingStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_BLENDING_STATUS_REQID,RADIO_MNGR_APP);

	/* Updating st_FM_PIList value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_LinkingStatus,(Tu8)sizeof(Te_RADIO_DABFM_LinkingStatus),&pst_amfm_app_req_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}
/*===========================================================================*/
/*		void AMFM_App_Request_AFSwitch              						 */
/*===========================================================================*/
void AMFM_App_Request_AFSwitch(Te_AMFM_App_AF_Switch  e_AF_Switch)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_AF_SWITCH_REQID,RADIO_MNGR_APP);

	/* Updating e_AF_Switch value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_AF_Switch,(Tu8)sizeof(Te_AMFM_App_AF_Switch),&(pst_amfm_app_req_msg->msg_length));

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}


/*===========================================================================*/
/*		void 	AMFM_APP_Request_FM_to_DAB_Switch         					 */
/*===========================================================================*/
void AMFM_APP_Request_FM_to_DAB_Switch(Te_AMFM_App_FM_To_DAB_Switch  e_AMFM_FM_DAB_Switch)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_FM_TO_DAB_SWITCH_REQID,RADIO_MNGR_APP);

	/* Updating e_AMFM_FM_DAB_Switch value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_AMFM_FM_DAB_Switch,(Tu8)sizeof(Te_AMFM_App_FM_To_DAB_Switch),&(pst_amfm_app_req_msg->msg_length));

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}

/*===========================================================================*/
/*		void AMFM_APP_Request_TASwitch              						 */
/*===========================================================================*/
void AMFM_APP_Request_TASwitch(Te_AMFM_App_TA_Switch  e_TA_Switch)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_TA_SWITCH_REQID,RADIO_MNGR_APP);

	/* Updating e_TA_Switch value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_TA_Switch,(Tu8)sizeof(Te_AMFM_App_TA_Switch),&(pst_amfm_app_req_msg->msg_length));

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Request_TuneUpDown								 */
/*===========================================================================*/
void AMFM_App_Request_TuneUpDown(Te_RADIO_DirectionType  e_Direction,Tu32	u32_No_of_Steps)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_TUNE_UP_DOWN_REQID,RADIO_MNGR_APP);

	/* Updating e_Direction value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_Direction,(Tu8)sizeof(Te_RADIO_DirectionType),&pst_amfm_app_req_msg->msg_length);

	/* Updating u32_No_of_Steps value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&u32_No_of_Steps,(Tu8)sizeof(Tu32),&pst_amfm_app_req_msg->msg_length);	 	
		
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Request_AnnoCancel								 */
/*===========================================================================*/
void AMFM_App_Request_AnnoCancel(Te_AMFM_App_Anno_Cancel_Request e_Anno_Cancel_Request)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_ANNOUNCEMENT_CANCEL_REQID,RADIO_MNGR_APP);
	
	/* Updating e_Anno_Cancel_Request value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_Anno_Cancel_Request, (Tu8)sizeof(Te_AMFM_App_Anno_Cancel_Request),&pst_amfm_app_req_msg->msg_length);	 
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Request_AFTune								 	 */
/*===========================================================================*/
void AMFM_App_Request_AFTune(Tu16 u16_PI)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_AF_TUNE_REQID,RADIO_MNGR_APP);

	/* Updating u16_PI value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&u16_PI,(Tu8)sizeof(Tu16),&pst_amfm_app_req_msg->msg_length);
		
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);	
}
/*===========================================================================*/
/*			void  AMFM_App_Request_ENG_Mode								 */
/*===========================================================================*/
void AMFM_App_Request_ENG_Mode(Te_AMFM_App_Eng_Mode_Switch	e_ENG_ModeSwitch)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_ENG_MODE_SWITCH_REQID,RADIO_MNGR_APP);
	
	/* Updating e_ENG_ModeSwitch value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_req_msg->data),&e_ENG_ModeSwitch, (Tu8)sizeof(Te_AMFM_App_Eng_Mode_Switch),&pst_amfm_app_req_msg->msg_length);	 

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Request_GetCT_Info								 */
/*===========================================================================*/
void AMFM_App_Request_GetCT_Info(void)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_GET_CT_INFO_REQID,RADIO_MNGR_APP);
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Request_FactoryReset								 */
/*===========================================================================*/
void AMFM_App_Request_FactoryReset(void)
{
	Ts_Sys_Msg		*pst_amfm_app_req_msg = &st_amfm_app_req_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_req_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_req_msg,RADIO_AM_FM_APP,AMFM_APP_FACTORY_RESET_REQID,RADIO_MNGR_APP);
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_req_msg);
}

/*===========================================================================*/
/*		void AMFM_APP_SendMsgtoMainhsm              						 */
/*===========================================================================*/
void AMFM_APP_SendMsgtoMainhsm(Ts_Sys_Msg *pst_Msg,Tu16 u16_msgid)
{
	AMFM_APP_MessageInit(pst_Msg,RADIO_AM_FM_APP,u16_msgid,AMFM_APP_INST_HSM_CID);
	
	/* Sending service message to amfm_app_hsm asynchronously. */
	SYS_SEND_MSG(pst_Msg);
}