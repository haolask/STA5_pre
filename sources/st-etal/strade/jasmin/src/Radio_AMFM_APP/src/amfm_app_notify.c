/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_notify.c																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This source file consists of function definitions of all Notification APIs		*
*						  which send messages to Radio Manager Application									*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/
#include "amfm_app_notify.h"

/*-----------------------------------------------------------------------------
    extern variable Declaration
-----------------------------------------------------------------------------*/
extern Te_AMFM_App_mode	e_CurrentMode;				// Defined in amfm_app_response.c 

/*-----------------------------------------------------------------------------
     Global variable Defintion 
-----------------------------------------------------------------------------*/
Ts_Sys_Msg			st_amfm_app_notify_msg;	/* used to send any notification message to Radio Manager Application component */

/*-----------------------------------------------------------------------------
    Public function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*			void  AMFM_App_Notify_TunerStatus							     */
/*===========================================================================*/
void AMFM_App_Notify_TunerStatus(Ts_AMFM_App_StationInfo st_current_station,Te_AMFM_App_SigQuality	e_SigQuality)
{

	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_TUNER_STATUS_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),(const void *)&st_current_station,(Tu8)sizeof(Ts_AMFM_App_StationInfo),&pst_amfm_app_notify_msg->msg_length);
	
	/*updating flag bit in the corresponding message data slot*/
    AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_SigQuality,(Tu8)sizeof(Te_AMFM_App_SigQuality),&pst_amfm_app_notify_msg->msg_length);
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_STLUpdated							     */
/*===========================================================================*/
void AMFM_App_Notify_STLUpdated(Te_RADIO_SharedMemoryType e_SharedMemoryType)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	if(e_CurrentMode==AMFM_APP_MODE_FM)	  
	{
		/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
		AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_FM_STATIONLIST_UPDATE_DONE_NOTIFYID,RADIO_AM_FM_APP);
	}
	else if(e_CurrentMode==AMFM_APP_MODE_AM)
	{
		/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
		AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AM_STATIONLIST_UPDATE_DONE_NOTIFYID,RADIO_AM_FM_APP);

	}
	else
	{
		/* do nothing */
	}
	/* Updating e_SharedMemoryType value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_SharedMemoryType,(Tu8)sizeof(Te_RADIO_SharedMemoryType),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_CurrFrequency							     */
/*===========================================================================*/
void AMFM_App_Notify_CurrFrequency(Tu32 u32_Frequency)
{

	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_FREQ_CHANGE_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&u32_Frequency,(Tu8)sizeof(Tu32),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Notify_BestPI									 */
/*===========================================================================*/
void AMFM_App_Notify_BestPI(Tu16 u16_BestPI,Tu32 u32_Quality,Tu32 u32_BestPI_Freq,Te_AMFM_APP_BestPI_Type e_BestPI_Type,Tu8* pu8_PSN,Tu8 u8_DABFM_LinkingCharset)
{

	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_BEST_PI_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating u16_BestPI value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&u16_BestPI,(Tu8)sizeof(Tu16),&pst_amfm_app_notify_msg->msg_length);

	/* Updating u32_Quality value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&u32_Quality,(Tu8)sizeof(Tu32),&pst_amfm_app_notify_msg->msg_length);
	
	/* Updating Best PI Frequency value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&u32_BestPI_Freq,(Tu8)sizeof(Tu32),&pst_amfm_app_notify_msg->msg_length);
	
	/* Updating e_BestPI_Type value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&e_BestPI_Type,(Tu8)sizeof(Te_AMFM_APP_BestPI_Type),&pst_amfm_app_notify_msg->msg_length);

	/* Updating PSN of Best FM station in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,pu8_PSN,(Tu8)MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME,&pst_amfm_app_notify_msg->msg_length);

	/* Updating Charset for PSN of Best FM station in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&u8_DABFM_LinkingCharset,(Tu8)sizeof(Tu8),&pst_amfm_app_notify_msg->msg_length);
	
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Notify_BestPI									 */
/*===========================================================================*/
void AMFM_App_Notify_BestPI_Changed(void)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_BEST_PI_CHANGED_NOTIFYID,RADIO_AM_FM_APP);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_LinkingStatus								 */
/*===========================================================================*/
void AMFM_App_Notify_LinkingStatus(Te_RADIO_DABFM_LinkingStatus	e_LinkingStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_LINKING_STATUS_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating e_LinkingStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&e_LinkingStatus,(Tu8)sizeof(Te_RADIO_DABFM_LinkingStatus),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_PIQuality									 */
/*===========================================================================*/
void AMFM_App_Notify_PIQuality(Tu32  u32_Quality, Tu8* pu8_BestPI_PSN,Te_AMFM_App_PSNChangeFlag e_PSNChangeFlag,Tu8 u8_DABFM_LinkingCharset)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_PI_QUALITY_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating u32_Quality (Interpolated) value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&u32_Quality,(Tu32)sizeof(Tu32),&pst_amfm_app_notify_msg->msg_length);
	
	/* Updating Best PI PSN in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,pu8_BestPI_PSN,(Tu8)MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME,&pst_amfm_app_notify_msg->msg_length);

	/* Updating e_PSNChangeFlag in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&e_PSNChangeFlag,(Tu8)sizeof(Te_AMFM_App_PSNChangeFlag),&pst_amfm_app_notify_msg->msg_length);

	/* Updating Charset for PSN of Best FM station in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage(pst_amfm_app_notify_msg->data,&u8_DABFM_LinkingCharset,(Tu8)sizeof(Tu8),&pst_amfm_app_notify_msg->msg_length);
	
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_Find_SID								     */
/*===========================================================================*/
void AMFM_App_Notify_Find_SID(Tu16 u16_pi)
{

	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFM_FIND_SID_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating st_CurrentStationInfo value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&u16_pi,(Tu8)sizeof(u16_pi),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}


/*===========================================================================*/
/*			void  AMFM_App_Notify_DAB_Quality							     */
/*===========================================================================*/
void AMFM_App_Notify_FMtoDAB_SIDStaion_Quality(Tu32 u32_Quality,Te_AMFM_DAB_PI_TYPE	e_DAB_PI_TYPE)
{

	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */

	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_AM_FM_APP,AMFM_APP_DAB_FOLLOWUP_NOTIFYID,RADIO_MNGR_APP);

	/* Updating u32_Quality value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&u32_Quality,(Tu8)sizeof(u32_Quality),&pst_amfm_app_notify_msg->msg_length);
	/* Updating e_DAB_PI_TYPE value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_DAB_PI_TYPE,(Tu8)sizeof(e_DAB_PI_TYPE),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_Initiate_FM_DAB_Follow_Up				     */
/*===========================================================================*/
void AMFM_App_Notify_Initiate_FM_DAB_Follow_Up(void)
{

	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_START_FM_DAB_FOLLOWUP_NOTIFYID,RADIO_AM_FM_APP);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}


/*===========================================================================*/
/*			void  AMFM_App_Notify_Announcement_Status					     */
/*===========================================================================*/

void AMFM_App_Notify_Announcement_Status(Te_AMFM_APP_Announcement_Status e_Announcement_Status)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_FM_ANNOUNCEMENT_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating e_Announcement_Status value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_Announcement_Status,(Tu8)sizeof(e_Announcement_Status),&pst_amfm_app_notify_msg->msg_length);

	
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}


/*===========================================================================*/
/*			void  AMFM_App_Notify_AF_Status								     */
/*===========================================================================*/

void AMFM_App_Notify_AF_Status(Te_AMFM_App_AF_Status e_AF_Status)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AF_STATUS_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating e_Announcement_Status value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_AF_Status,(Tu8)sizeof(e_AF_Status),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);

}
/*===========================================================================*/
/*			void  AMFM_App_Notify_Stop_DAB_to_FM_Linking				     */
/*===========================================================================*/
void AMFM_App_Notify_Stop_DAB_to_FM_Linking(void)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_AM_FM_APP,AMFM_APP_STOP_DAB2FM_LINKING_NOTIFYID ,RADIO_MNGR_APP);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Notify_AMFMTuner_Abnormal								     */
/*===========================================================================*/
void AMFM_App_Notify_AMFMTuner_Abnormal(Te_RADIO_AMFMTuner_Status e_AMFMTunerStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_AMFMTUNER_ABNORMAL_NOTIFYID,RADIO_AM_FM_APP);

	/* Updating e_DABTunerStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_AMFMTunerStatus,(Tu8)sizeof(Te_RADIO_AMFMTuner_Status),&pst_amfm_app_notify_msg->msg_length);
 
	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Notify_DABStatus				  					 */
/*===========================================================================*/
void AMFM_App_Notify_DABStatus(Te_RADIO_Comp_Status e_DABTunerStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_AM_FM_APP,AMFM_APP_DABTUNER_STATUS_NOTIFYID ,RADIO_MNGR_APP);

	/* Updating e_DABTunerStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_DABTunerStatus,(Tu8)sizeof(Te_RADIO_Comp_Status),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_ENG_AFList				  				 */
/*===========================================================================*/
void AMFM_App_Notify_ENG_AFList(Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP, RADIO_MNGR_APP_FM_AFLIST_UPDATE_RESID ,RADIO_AM_FM_APP);

	/* Updating e_DABTunerStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),pst_AMFM_App_ENG_AFList_Info,(Tu8)sizeof(Ts_AMFM_App_ENG_AFList_Info),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}


/*===========================================================================*/
/*			void  AMFM_App_Notify_StartBackgroundScan		  				 */
/*===========================================================================*/
void AMFM_App_Notify_StartBackgroundScan(void)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_AM_FM_APP,AMFM_APP_NON_RADIO_MODE_NOTIFYID ,RADIO_MNGR_APP);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);

}

/*===========================================================================*/
/*			void  AMFM_App_Notify_UpdatedLearnMem_AFStatus	  				 */
/*===========================================================================*/
void AMFM_App_Notify_UpdatedLearnMem_AFStatus(Te_AMFM_App_LearnMemAFStatus e_AFTune_ReplyStatus,Ts_AMFM_App_StationInfo st_current_station)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP, RADIO_MNGR_APP_FM_AF_LEARN_MEM_NOTIFYID ,RADIO_AM_FM_APP);

	/* Updating e_DABTunerStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_AFTune_ReplyStatus,(Tu8)sizeof(Te_AMFM_App_LearnMemAFStatus),&pst_amfm_app_notify_msg->msg_length);

	/* Updating e_DABTunerStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&st_current_station,(Tu8)sizeof(Ts_AMFM_App_StationInfo),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}


/*===========================================================================*/
/*			void  AMFM_App_Notify_StationNotAvail_StrategyStatus			 */
/*===========================================================================*/
void AMFM_App_Notify_StationNotAvail_StrategyStatus(Te_AMFM_App_StationNotAvailStrategyStatus e_StationNotAvailStrategyStatus)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_AM_FM_APP, AMFM_APP_STA_NOT_AVAIL_STRA_STATUS_NOTIFYID ,RADIO_MNGR_APP);

	/* Updating e_StationNotAvailStrategyStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_StationNotAvailStrategyStatus,(Tu8)sizeof(Te_AMFM_App_StationNotAvailStrategyStatus),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}
/*===========================================================================*/
/*			void  AMFM_App_Notify_AF_SigLost		  				 */
/*===========================================================================*/
void AMFM_App_Notify_AF_SigLost(void)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP,RADIO_MNGR_APP_FM_AF_SIGLOST_NOTIFYID,RADIO_AM_FM_APP);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

/*===========================================================================*/
/*			void  AMFM_App_Notify_Stop_FM_DAB_Follow_Up						 */
/*===========================================================================*/
void AMFM_App_Notify_Stop_FM_DAB_Follow_Up(Te_AMFM_FMDAB_STOP_status e_AMFM_FMDAB_STOP_status)
{
	Ts_Sys_Msg		*pst_amfm_app_notify_msg = &st_amfm_app_notify_msg;				/* pointer to message structure defined globally */
	
	/* Updating header information like dest id,src id ,msg id  in st_amfm_app_notify_msg structure */	
	AMFM_APP_MessageInit(pst_amfm_app_notify_msg,RADIO_MNGR_APP, RADIO_MNGR_APP_STOP_FM_DAB_LINKING_NOTIFYID ,RADIO_AM_FM_APP);

	/* Updating e_DABTunerStatus value in the corresponding message data slot */
	AMFM_APP_UpdateParameterIntoMessage((Tchar *)(pst_amfm_app_notify_msg->data),&e_AMFM_FMDAB_STOP_status,(Tu8)sizeof(Te_AMFM_FMDAB_STOP_status),&pst_amfm_app_notify_msg->msg_length);

	/* Sending asynchronous message  */
	SYS_SEND_MSG(pst_amfm_app_notify_msg);
}

