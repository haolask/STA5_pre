
/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_RDS_decoder.c                                                                          *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     * 
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: 						                                                            *
*  Description			: FM RDS Decoder Process from Tuner.   							                    *
*                                                                                                           *
*                                                                                                           * 
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "AMFM_HAL_RDS_decoder.h"
#include "AMFM_Tuner_Ctrl_App.h"

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
Tu8		 u8_Current_PTY_Code = 0xFF;
Tu32	 u32_currTunFreq   = 0;
Tu16	 u16_Current_PI	   = 0;
Tu32	 u32_prevTunFreq   = 0;
Tu32 	 u32_playingFreq   = 0;
Tbool	 b_freq_update	   = FALSE;
Tu8  	 u8_RDSTimeout     = 0;
/* Radio Text Start */
Tu16 	u16_dcodeset       = 0x0000;
Tu16 	u16_shftvalue      = 0;
Tu8 	u8_RadioTxtDone    = 0x00; 
Tu8 	u8_RTdata_size     = 0;
Tu16 	u16_rtdcodeset     = 0;
Tu16 	u16_rtdyndcodeset  = 0;
Tu8     u8_RT_end_index    = 0xFF;
Tu8     u8_seg_index_check = 0;
Tu8 	u8_Txtdata[RDS_DECODER_MAX_RT_SIZE];
Tu8     au8_Prev_seg_index[16] = { 0xFF };
/* Radio Text End */
Tu8 	timer_ret          = 0;
Tu8  	u8_AF_Lock         = 0;
Tu8 	u8_NumofAF_SamePgm = 0;
Tu8 	u8_NumofAF_RgnlPgm = 0;
Tu8  	u8_NumOfAF_List    = 0;
Tu8  	u8_TA;
Tu8  	u8_TP;
Tbool   b_AF_MethodB     = FALSE;
Tu8  	u8_NoAF_Lock      = 0;
Tu8  	u8_AF_done        = 0;
Tu8 	u8_PSNdata[RDS_DECODER_MAX_PS_SIZE];
Tu32 	u32_AFList_SamePgm[RDS_DECODER_MAX_AF_LIST];
Tu32 	u32_AFList_RgnlPgm[RDS_DECODER_MAX_AF_LIST];
/** RDS PSN Decode variables Start **/
Tu8		u8_PSNPrevIndex      = 0xFF;
Tu8		u8_PSNCurIndex 	     = 0xFF;
Tu8		u8_PSNDataStart      = 0xFF;
Tu8		u8_PSNDataEnd	     = 0xFF;  
/** RDS PSN Decode variables End **/
extern Ts_AMFM_Tuner_Ctrl_Timer_Ids st_AMFM_Tuner_Ctrl_Timer_Ids;
Ts_Sys_Msg  prev_msg,curr_msg;
Tu8         u8_RDS_Timeout_count=RDS_DECODER_FM_NONRDS_STN_DETECT_COUNT;
Tu16 u16_RespMsgId;
Ts_AMFM_Tuner_Ctrl_RDS_Data st_Curr_RDS_Data;
Tu32                        u32_RDS_Buffer_Actual_Size;
Tu8                         au8_Callback_RDS_Buffer[54];
Tbool       b_RDS_Decode_State = FALSE;
/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void FM_RDS_DECODER_HandleMsg				                             */
/*===========================================================================*/
void FM_RDS_DECODER_HandleMsg(Ts_Sys_Msg* pst_msg)
{
	Ts_Sys_Msg* msg = NULL;
  	
	Tu32        slot = 0;

	/*Check Tuner_Ctrl request for Start RDS decode*/
	if (pst_msg->msg_id != STOP_RDS_MSG_ID && pst_msg->msg_id != RDS_CALLBACK_DATA_RDY_MSG_ID && pst_msg->msg_id != NON_RDS_NOTIFYID)
	{
		st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id = SYS_TIMER_START(AMFM_TUNER_CTRL_FM_RDS_NOTIFY_TIME, NON_RDS_NOTIFYID, RADIO_FM_RDS_DECODER);
		if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id == 0)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_RDS_Decoder]  : NONRDS notify start timer is failed ");
		}
		else
		{
			/* MISRA C */
		}
		u16_RespMsgId = pst_msg->msg_id;
		SYS_RADIO_MEMCPY(&u32_currTunFreq, pst_msg->data, sizeof(Tu32));
		u16_Current_PI	    = 0;
		u8_Current_PTY_Code = 0xFF;
		b_RDS_Decode_State  = TRUE;
		
		if ((u32_currTunFreq != u32_playingFreq))
		{
			memset(&st_Curr_RDS_Data, 0x00, sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data));

			u8_PSNPrevIndex = 0xFF;
			u8_PSNCurIndex  = 0xFF;
			u8_PSNDataStart = 0xFF;
			u8_PSNDataEnd   = 0xFF;
			memset(u8_PSNdata, 0x00, RDS_DECODER_MAX_PS_SIZE);

			u8_NumofAF_SamePgm = 0;
			u8_NumofAF_RgnlPgm = 0;
			u8_AF_Lock = 0;
			u8_NumOfAF_List = 0xff;
			memset(u32_AFList_SamePgm, 0x00, RDS_DECODER_MAX_AF_LIST);
			memset(u32_AFList_RgnlPgm, 0x00, RDS_DECODER_MAX_AF_LIST);

			u8_RadioTxtDone = 0x00;
			u8_RT_end_index = 0xFF;
			u8_seg_index_check = 0;
			u8_RTdata_size = 0;
			memset(u8_Txtdata, 0x00, RDS_DECODER_MAX_RT_SIZE);
			memset(au8_Prev_seg_index, 0xFF, 16);
			u8_RDSTimeout = 0;
			/* Clearing TA/TP flag */
			u8_TA = 0;
			u8_TP = 0;
		}
		switch(pst_msg->msg_id)
		{
			case AMFM_TUNER_CTRL_FMCHECK_DONE_RESID: 
			{
				u8_RDS_Timeout_count = RDS_DECODER_FMCHECK_NONRDS_STN_DETECT_COUNT;
			}break;
			case AMFM_TUNER_CTRL_BGTUNE_DONE_RESID:
			{
				u8_RDS_Timeout_count = RDS_DECODER_BGTUNEPIPSN_NONRDS_STN_DETECT_COUNT ;
			}
			break;
			default:
			{
				u8_RDS_Timeout_count = RDS_DECODER_FM_NONRDS_STN_DETECT_COUNT;
			}break;
		}
	}
	else if (b_RDS_Decode_State ==  TRUE)
	{
		if (pst_msg->msg_id == RDS_CALLBACK_DATA_RDY_MSG_ID)
		{
			if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id > 0)
			{
				if (SYS_TIMER_STOP(st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id) != TRUE)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_RDS_Decoder]  : FM NonRDS notification timer is failed ");
				}
				else
				{
					st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id = 0;
					u8_RDSTimeout = 0;
				}
			}
			else
			{
				/*Nothing*/
			}
			/* RDS Decode Process */
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(u32_RDS_Buffer_Actual_Size), (Tchar *)(pst_msg->data), (Tu16)(sizeof(Tu8)), &slot);
			AMFM_Tuner_ctrl_ExtractParameterFromMessage(&(au8_Callback_RDS_Buffer[0]), (Tchar *)(pst_msg->data), (Tu16)(u32_RDS_Buffer_Actual_Size), &slot);

			RDS_Blocks_Read(au8_Callback_RDS_Buffer, u32_RDS_Buffer_Actual_Size);

			if((u32_currTunFreq != (Tu32)0x00000000) )
			{	
				u32_playingFreq = u32_currTunFreq;
			}
			else
			{
				/* MISRA C */
			}
		}
		else if (pst_msg->msg_id == STOP_RDS_MSG_ID)
		{
			b_RDS_Decode_State = FALSE;
			msg = SYS_MSG_HANDLE(RADIO_AM_FM_TUNER_CTRL, STOP_DONE_MSG_ID);
			memcpy(msg->data, &u32_currTunFreq, sizeof(u32_currTunFreq));
			SYS_SEND_MSG(msg);	
		}
		else if (pst_msg->msg_id == NON_RDS_NOTIFYID)
		{
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id = 0;
			FM_RDS_Notify_Non_RDS_Station();
		}
		else
		{
			/* MISRA C */
		}
	}
}

/*===========================================================================*/
/*  void PI_Update   				                                         */
/*===========================================================================*/
void  Check_PI_Update(Ts_AMFM_Tuner_Ctrl_RDS_Data *pst_curr_rds_data)
{   
	/*Comparing the old PI to present decoded PI */
	if ((pst_curr_rds_data->u16_PI_Code != (Tu16)(0)) && (pst_curr_rds_data->u16_PI_Code != u16_Current_PI) && (pst_curr_rds_data->u16_PI_Code != 0xFFFF))
	{
		u16_Current_PI = pst_curr_rds_data->u16_PI_Code;
		pst_curr_rds_data->b_PI_Update = TRUE;
	}
	else
	{
		/* MISRA */
	}
}

/*===========================================================================*/
/*  void RDS_pi_blocks							                             */ 
/*===========================================================================*/
void FM_RDS_PI_Decode_Process(Ts_AMFM_Tuner_Ctrl_RDS_Process_AC_Blocks* pst_RDS_Process_AC_Blocks)
{
	Tu8 u8_Index = 0;

	if (pst_RDS_Process_AC_Blocks->b_A_Block!= FALSE)
	{
		/* Get PI from RDS block A*/
		st_Curr_RDS_Data.u16_PI_Code = (Tu16)(LIB_CONCATENATE((Tu16)pst_RDS_Process_AC_Blocks->RDS_Block_A[u8_Index], (Tu8)(8), pst_RDS_Process_AC_Blocks->RDS_Block_A[u8_Index + 1]));
	}
	/*check c type*/
	else if (pst_RDS_Process_AC_Blocks->b_C_Block != FALSE)
	{
		/* Get PI from RDS block A*/
		st_Curr_RDS_Data.u16_PI_Code = (Tu16)(LIB_CONCATENATE((Tu16)pst_RDS_Process_AC_Blocks->RDS_Block_C[u8_Index], (Tu8)(8), pst_RDS_Process_AC_Blocks->RDS_Block_C[u8_Index + 1]));
	}
	else
	{
		/* MISRA */
	}

	Check_PI_Update(&st_Curr_RDS_Data);
	/*send the message to tuner control*/
	RDS_Notify_Process(&st_Curr_RDS_Data);
}

/*===========================================================================*/
/*  void FM_RDS_Group_Decode_Process				                            */
/*===========================================================================*/
void FM_RDS_Group_Decode_Process(Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks* pst_RDS_Process_All_Blocks)
{
	Tu8 u8_Index = 0;
	
	Tu8 u8_Grpcode = (LIB_AND(pst_RDS_Process_All_Blocks->RDS_Block_B[0], 0XF8));

	if (pst_RDS_Process_All_Blocks->b_A_Block != FALSE)
	{
		/* Get PI ::: Get PI value from any RDS Group */
		st_Curr_RDS_Data.u16_PI_Code = (Tu16)(LIB_CONCATENATE((Tu16)pst_RDS_Process_All_Blocks->RDS_Block_A[u8_Index], (Tu8)(8), pst_RDS_Process_All_Blocks->RDS_Block_A[u8_Index + 1]));
		Check_PI_Update(&st_Curr_RDS_Data);
	}
	else
	{
		/* MISRA */
	}
	/*check c type*/
	if ((pst_RDS_Process_All_Blocks->b_C_Block != FALSE) && (pst_RDS_Process_All_Blocks->b_Ctype_Flag != FALSE))
	{
		st_Curr_RDS_Data.u16_PI_Code = (Tu16)(LIB_CONCATENATE((Tu16)pst_RDS_Process_All_Blocks->RDS_Block_C[u8_Index], (Tu8)(8), pst_RDS_Process_All_Blocks->RDS_Block_C[u8_Index + 1]));
		Check_PI_Update(&st_Curr_RDS_Data);
	}
	else
	{
		/* MISRA */
	}
	if (pst_RDS_Process_All_Blocks->b_B_Block != FALSE)
	{
		/*Get PTY from RDS 0A-15B groups*/
		(void)FM_RDS_PTY_Decode(pst_RDS_Process_All_Blocks->RDS_Block_B, pst_RDS_Process_All_Blocks->RDS_Block_D, &st_Curr_RDS_Data);
	}
	else
	{
		/* MISRA */
	}
	switch(u8_Grpcode)
	{
		case RDS_DECODER_0A_GROUP:
		{
			/* Get PSN from RDS 0A/0B-Group */
			(void)FM_RDS_PSN_Decode(pst_RDS_Process_All_Blocks->RDS_Block_B, pst_RDS_Process_All_Blocks->RDS_Block_D, &st_Curr_RDS_Data);

			/*Get AF list from RDS 0A-Group*/
			(void)FM_RDS_AF_Decode(pst_RDS_Process_All_Blocks->RDS_Block_C, &st_Curr_RDS_Data);
			
			/*Get Announcement info(TA/TP) for current tuned station*/
			(void)FM_RDS_TA_Decode(pst_RDS_Process_All_Blocks->RDS_Block_B, &st_Curr_RDS_Data);
			
		}
		break;
		case RDS_DECODER_0B_GROUP:
		{
			/* Get PSN from RDS 0A/0B-Group */
			(void)FM_RDS_PSN_Decode(pst_RDS_Process_All_Blocks->RDS_Block_B, pst_RDS_Process_All_Blocks->RDS_Block_D, &st_Curr_RDS_Data);
		
			/*Get Announcement info(TA/TP) for current tuned station*/
			(void)FM_RDS_TA_Decode(pst_RDS_Process_All_Blocks->RDS_Block_B, &st_Curr_RDS_Data);
		}
		break;
		case RDS_DECODER_2A_GROUP:
		{
			/*Get RT from RDS 2A group*/
			(void)FM_RDS_RT_Decode(pst_RDS_Process_All_Blocks, &st_Curr_RDS_Data);
		}
		break;
		case RDS_DECODER_2B_GROUP:
		{
			/*Get RT from RDS 2B group*/
			(void)FM_RDS_RT_Decode(pst_RDS_Process_All_Blocks, &st_Curr_RDS_Data);
		}
		break;
		case RDS_DECODER_14A_GROUP:
		{
			(void)FM_RDS_EON_Anno_AF_Decode(pst_RDS_Process_All_Blocks, &st_Curr_RDS_Data);
		}
			break;

		case RDS_DECODER_14B_GROUP:
		{
			(void)FM_RDS_EON_Anno_TA_Decode(pst_RDS_Process_All_Blocks->RDS_Block_B, pst_RDS_Process_All_Blocks->RDS_Block_D, &st_Curr_RDS_Data);
		}
			break;
		default:
		{
			/*Nothing*/
		}
		break;
	}
	/*send the message to tuner control*/
	RDS_Notify_Process(&st_Curr_RDS_Data);
}

/*===========================================================================*/
/*  void RDS_Process						                             */
/*===========================================================================*/

void RDS_Notify_Process(Ts_AMFM_Tuner_Ctrl_RDS_Data *pst_RDS_Data)
{
	Ts_Sys_Msg* msg = NULL;

	/* Notify the Updated RDS Data to FM Tuner Control layer */
	if ((pst_RDS_Data->b_PSN_Update == TRUE)   || (pst_RDS_Data->b_RT_Update == TRUE) || (pst_RDS_Data->b_PI_Update == TRUE) || 
		(pst_RDS_Data->b_PTY_Update == TRUE)    || (pst_RDS_Data->b_AF_Update == TRUE) || (pst_RDS_Data->b_TA_Update == TRUE) ||
		(pst_RDS_Data->b_EONTA_Update == TRUE) || (pst_RDS_Data->b_EON_AF_Update == TRUE))
	{
		/* Updating Frequency whose RDS data is being received*/
		pst_RDS_Data->u32_TunedFrequency = u32_currTunFreq;

		msg = SYS_MSG_HANDLE(RADIO_AM_FM_TUNER_CTRL, u16_RespMsgId);
		SYS_RADIO_MEMCPY(msg->data, pst_RDS_Data, sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data));
		SYS_SEND_MSG(msg);

		if (pst_RDS_Data->b_AF_Update == TRUE)
		{
			u8_NumofAF_SamePgm = 0;
			u8_NumofAF_RgnlPgm = 0;
			u8_AF_Lock = 0;
			u8_NumOfAF_List = 0xff;
			memset(u32_AFList_SamePgm, 0x00, RDS_DECODER_MAX_AF_LIST);
			memset(u32_AFList_RgnlPgm, 0x00, RDS_DECODER_MAX_AF_LIST);
		}

		pst_RDS_Data->b_PSN_Update    = FALSE;
		pst_RDS_Data->b_RT_Update     = FALSE;
		pst_RDS_Data->b_PI_Update     = FALSE;
		pst_RDS_Data->b_PTY_Update    = FALSE;
		pst_RDS_Data->b_AF_Update     = FALSE;
		pst_RDS_Data->b_TA_Update     = FALSE;
		pst_RDS_Data->b_EONTA_Update  = FALSE;
		pst_RDS_Data->b_EON_AF_Update = FALSE;
		pst_RDS_Data->u16_PI_Code     = 0;
		
		/* Reset the count for NON-RDS */
		u8_RDSTimeout = 0;
	}
	else
	{
		/* Do Nothing */
	}
}
void FM_RDS_Notify_Non_RDS_Station()
{
	Ts_Sys_Msg* msg = NULL;
	/*Incrementing the count to check for NON-RDS station*/
	u8_RDSTimeout +=1;

	if (u8_RDSTimeout > u8_RDS_Timeout_count)
	{
		memset(&st_Curr_RDS_Data, 0x00, sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data));

		/* Updating Frequency whose RDS data is being received*/
		st_Curr_RDS_Data.u32_TunedFrequency = u32_currTunFreq;

		/* Update current tuned station does not have the RDS data */
		st_Curr_RDS_Data.b_NonRDS_Station = TRUE;

		msg = SYS_MSG_HANDLE(RADIO_AM_FM_TUNER_CTRL, u16_RespMsgId);
		SYS_RADIO_MEMCPY(msg->data, &st_Curr_RDS_Data, sizeof(Ts_AMFM_Tuner_Ctrl_RDS_Data));
		SYS_SEND_MSG(msg);

		st_Curr_RDS_Data.b_NonRDS_Station = FALSE;
		/* Reset the RDS count */
		u8_RDSTimeout = 0;
	}
	else
	{
		st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id = SYS_TIMER_START(AMFM_TUNER_CTRL_FM_RDS_NOTIFY_TIME, NON_RDS_NOTIFYID, RADIO_FM_RDS_DECODER);
		if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_NonRDS_Timer_id == 0)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_RDS_Decoder]  : NONRDS notify start timer is failed ");
		}
		else
		{
			/* MISRA C */
		}
	}
}
/*===========================================================================*/
/*  Tu8 AFRepFreq_Check		    				                             */
/*===========================================================================*/
Tu8 AFRepFreq_Check(Tu32 AFList[RDS_DECODER_MAX_AF_LIST], Tu8 Numoffreq, Tu32 RecvFreq)
{
	Tu8 Relval = 0;
	Tu8 i = 0;
	
	if(Numoffreq != (Tu8)0)	
	{
		for(i=0; i<Numoffreq; i++)
		{
			if(AFList[i] == RecvFreq)
			{
				/* Received Frequency already there no need to copy */
				Relval = 1;
				break;
			}
			else
			{
				/* MISRA */
			}
		}
	}
	else
	{
		/* MISRA */
	}

	return Relval;
}

/*===========================================================================*/
/*  Tu8 FM_RDS_PSN_Decode       				                             */
/*===========================================================================*/
Tu8 FM_RDS_PSN_Decode(Tu8* au8_PSN_Block_B, Tu8* au8_PSN_Block_D, Ts_AMFM_Tuner_Ctrl_RDS_Data *pst_RDS_Data)
{     
	Tu8 u8_Dblock_index = 0;
	/* Index value of PSN Name */
	u8_PSNCurIndex = (Tu8)((LIB_AND(au8_PSN_Block_B[1], (Tu8)(0x03))* (Tu8)(2))); //macro addition
	
	/* Identifying the starting of PS Name. If the Start is detected clear the PSN Data and copy the characters*/
	if(u8_PSNCurIndex == (Tu8)0)  
	{
		u8_PSNDataStart = RDS_DECODER_TRUE;
		u8_PSNDataEnd = RDS_DECODER_FALSE;
		memset(u8_PSNdata, 0x00, RDS_DECODER_MAX_PS_SIZE);
		u8_PSNdata[u8_PSNCurIndex] = au8_PSN_Block_D[u8_Dblock_index];
		u8_PSNdata[u8_PSNCurIndex + 1] = au8_PSN_Block_D[++u8_Dblock_index];
		u8_PSNPrevIndex = u8_PSNCurIndex;
	}
	/* Copying the Subsequent characters of PSN. The PSN data will be ignored if there is no continuity in the index because of Signal conditions */
	else if(((u8_PSNCurIndex - u8_PSNPrevIndex) ==2)&&(u8_PSNDataStart== RDS_DECODER_TRUE) &&(u8_PSNDataEnd == RDS_DECODER_FALSE))
	{
		u8_PSNdata[u8_PSNCurIndex] = au8_PSN_Block_D[u8_Dblock_index];
		u8_PSNdata[u8_PSNCurIndex + 1] = au8_PSN_Block_D[++u8_Dblock_index];
		u8_PSNPrevIndex = u8_PSNCurIndex;
		if(u8_PSNCurIndex == (Tu8)6)
		{
			u8_PSNDataEnd = RDS_DECODER_TRUE;
			u8_PSNDataStart = RDS_DECODER_FALSE;
		}
		else
		{
			/* MISRA */
		}
	}
	/* If there is any discontinuity because of signal variations wait for the start of PSN again */
	else
	{
		u8_PSNDataStart = RDS_DECODER_FALSE;
		u8_PSNDataEnd = RDS_DECODER_FALSE;
	}
	
	/* Check all characters received */
	if(u8_PSNDataEnd == RDS_DECODER_TRUE)
	{
		Sys_Radio_Memcpy(pst_RDS_Data->st_FM_RDS_PSN.u8_ps, u8_PSNdata, RDS_DECODER_MAX_PS_SIZE);
		pst_RDS_Data->b_PSN_Update = TRUE;
		u8_PSNDataStart = RDS_DECODER_FALSE;
		u8_PSNDataEnd = RDS_DECODER_FALSE;
		u8_PSNPrevIndex = 0xFF;
		memset(u8_PSNdata,0x00,RDS_DECODER_MAX_PS_SIZE);
	}	
	else
	{
		/* MISRA */
	}
	return 0;
}
/*===========================================================================*/
/*  Tu8 FM_RDS_PTY_Decode       				                             */
/*===========================================================================*/
Tu8 FM_RDS_PTY_Decode(Tu8* au8_PTY_Block_B, Tu8* au8_PTY_Block_D, Ts_AMFM_Tuner_Ctrl_RDS_Data *pst_RDS_Data)
{
	Tu8 u8_index      = 0;
	Tu8 u8_PTY_Code_B = 0;
	Tu8 u8_PTY_Code_D = 0;

	if (au8_PTY_Block_B != NULL)
	{/*Read PTY other than 15B group of RDS i.e., All groups */

		if ((LIB_AND(au8_PTY_Block_B[0], (Tu8)(0xF8))) != RDS_DECODER_15B_GROUP)//checking group code
		{
			/*Decode PTY from RDS*/
			pst_RDS_Data->u8_PTY_Code = (((au8_PTY_Block_B[u8_index] & 0x03) << 3) | ((au8_PTY_Block_B[u8_index + 1] & 0xE0) >> 5));
		}
		else
		{
			/*Decode PTY from RDS 15B B block */
			u8_PTY_Code_B = (((au8_PTY_Block_B[u8_index] & 0x03) << 3) | ((au8_PTY_Block_B[u8_index + 1] & 0xE0) >> 5));

			if (au8_PTY_Block_D != NULL)
			{
				/* now check in block 15B group of 'D' Block to get PTY code */
				u8_PTY_Code_D = (((au8_PTY_Block_D[u8_index] & 0x03) << 3) | ((au8_PTY_Block_D[u8_index + 1] & 0xE0) >> 5));
			}
			else
			{
				/* MISRAC */
			}
			if (u8_PTY_Code_B == u8_PTY_Code_D)
			{
				pst_RDS_Data->u8_PTY_Code = u8_PTY_Code_B; // copying PTY to RDS struct nly if B and D blocks have same PTY.
			}
		}
		if (pst_RDS_Data->u8_PTY_Code != u8_Current_PTY_Code)
		{
			u8_Current_PTY_Code = pst_RDS_Data->u8_PTY_Code;
			pst_RDS_Data->b_PTY_Update = TRUE;
		}
		else
		{
			/* MISRAC */
		}
	}
	else
	{
		/* MISRAC */
	}
	return 0;
}
/*===========================================================================*/
/*  Tu8 FM_RDS_AF_Decode         				                             */
/*===========================================================================*/
Tu8 FM_RDS_AF_Decode(Tu8* au8_AF_Block_C,Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data)
{
	Tu32 u32_F1Val = 0x00000000;
	Tu32 u32_F2Val = 0x00000000;

	if (((au8_AF_Block_C[0] > (RDS_DECODER_AFSIZE_STARTCODE)) && (au8_AF_Block_C[0] < (Tu8)(RDS_DECODER_AFSIZE_ENDCODE))) && (u8_AF_Lock == (Tu8)0))
	{
		u8_NumOfAF_List = (Tu8)(au8_AF_Block_C[0] - (Tu8)(RDS_DECODER_AFSIZE_MINCODE));
		u8_AF_Lock      = (Tu8)1;
		u8_NoAF_Lock    = (Tu8)1;
	}
	else
	{
		/* MISRA */
	}

	if((u8_AF_Lock == (Tu8)0x01)&&(u8_NumOfAF_List != (Tu8)0x00))
	{			
		if ((au8_AF_Block_C[0] > (Tu8)0) && (au8_AF_Block_C[0] < (Tu8)205) && (u8_NumOfAF_List >= 0x01))
		{
			u32_F1Val = (Tu32)((au8_AF_Block_C[0] + RDS_DECODER_AFFREQ_MINVAL)*(Tu8)(100));

			if((u32_F1Val != (Tu32)(0)) && (u32_F1Val != u32_currTunFreq) && 
			((AFRepFreq_Check(u32_AFList_SamePgm, u8_NumofAF_SamePgm, u32_F1Val)) != (Tu8)(0x01)) && 
			((AFRepFreq_Check(u32_AFList_RgnlPgm, u8_NumofAF_RgnlPgm, u32_F1Val)) != (Tu8)(0x01)))
			{
				u8_NumOfAF_List = (Tu8)(u8_NumOfAF_List -(Tu8)(1));
			}
			else
			{
				/* MISRA */
			}
		}
		else
		{
			/* MISRA */
		}
		if ((au8_AF_Block_C[1] > (Tu8)(0)) && (au8_AF_Block_C[1] < (Tu8)(RDS_DECODER_AF_FILLERCODE)) && (u8_NumOfAF_List >= (Tu8)1))
		{
			u32_F2Val = (Tu32)((au8_AF_Block_C[1] + (RDS_DECODER_AFFREQ_MINVAL))*(Tu8)(100));

			if((u32_F2Val != (Tu8)(0)) && (u32_F2Val != u32_currTunFreq) && 
			(AFRepFreq_Check(u32_AFList_SamePgm, u8_NumofAF_SamePgm, u32_F2Val) != (Tu8)(0x01))&& 
			(AFRepFreq_Check(u32_AFList_RgnlPgm, u8_NumofAF_RgnlPgm, u32_F2Val) != (Tu8)(0x01)))
			{
				u8_NumOfAF_List = (Tu8)(u8_NumOfAF_List -(Tu8)(1));
			}
			else
			{
				/* MISRA */
			}
		}
		else
		{
			/* MISRA */
		}

		/* For Method B - To decode the Correct No of AF List */
		if((u8_NoAF_Lock == (Tu8)(1)) && (u32_F1Val == (Tu8)(0)) && (u32_F2Val == u32_currTunFreq))
		{
			u8_NumOfAF_List = (Tu8)(u8_NumOfAF_List -(Tu8)(1));
			u8_NoAF_Lock = (Tu8)(0);
		}
		else
		{
			/* MISRA */
		}

		if((u32_F1Val != u32_currTunFreq) &&(u32_F2Val != u32_currTunFreq))
		{
			/*set the AF method A flag, when method A alternative frequencies are transmitting*/
			pst_curr_rds_data->st_FM_RDS_AF.b_AF_MethodA = TRUE;
			
			/*Clearing AF method B flag,when AF method A is transmitting*/
			if(pst_curr_rds_data->st_FM_RDS_AF.b_AF_MethodB != FALSE) 
			{
				pst_curr_rds_data->st_FM_RDS_AF.b_AF_MethodB = FALSE;
			}
			else
			{
				/* MISRA */
			}
			if((u32_F1Val != (Tu32)(0)) && (u32_F1Val != u32_currTunFreq) && 
			(AFRepFreq_Check(u32_AFList_SamePgm, u8_NumofAF_SamePgm, u32_F1Val) != (Tu8)(1)))
			{
				u32_AFList_SamePgm[u8_NumofAF_SamePgm] = u32_F1Val;
				u8_NumofAF_SamePgm = (Tu8)(u8_NumofAF_SamePgm + (Tu8)(1));
			}
			else
			{
				/* MISRA */
			}

			if((u32_F2Val != (Tu32)(0)) && (u32_F2Val != u32_currTunFreq)&& 
			(AFRepFreq_Check(u32_AFList_SamePgm, u8_NumofAF_SamePgm, u32_F2Val) != (Tu8)(0x01)))
			{
				u32_AFList_SamePgm[u8_NumofAF_SamePgm] = u32_F2Val;
				u8_NumofAF_SamePgm = (Tu8)(u8_NumofAF_SamePgm + (Tu8)(1));
			}
			else
			{
				/* MISRA */
			}
		}
		else 
		{
			b_AF_MethodB = TRUE ;
			
			/*set the AF method B flag, when method B alternative frequencies are transmitting*/
			pst_curr_rds_data->st_FM_RDS_AF.b_AF_MethodB = TRUE; 
			
			/*Clearing AF method A flag,when AF method B is transmitting*/
			if(pst_curr_rds_data->st_FM_RDS_AF.b_AF_MethodA != FALSE) 
			{
				pst_curr_rds_data->st_FM_RDS_AF.b_AF_MethodA = FALSE;
			}
			else
			{
				/* MISRA */
			}
			if(u32_F1Val < u32_F2Val)
			{
				if((u32_F1Val != (Tu32)(0)) && (u32_F1Val != u32_currTunFreq) && 
				(AFRepFreq_Check(u32_AFList_SamePgm, u8_NumofAF_SamePgm, u32_F1Val) != (Tu8)(0x01)))
				{
					u32_AFList_SamePgm[u8_NumofAF_SamePgm] = u32_F1Val;
					u8_NumofAF_SamePgm = (Tu8)(u8_NumofAF_SamePgm + (Tu8)(1));
				}
				else
				{
					/* MISRA */
				}

				if((u32_F2Val != (Tu32)(0)) && (u32_F2Val != u32_currTunFreq)&& 
				(AFRepFreq_Check(u32_AFList_SamePgm, u8_NumofAF_SamePgm, u32_F2Val) != (Tu8)(0x01)))
				{
					u32_AFList_SamePgm[u8_NumofAF_SamePgm] = u32_F2Val;
					u8_NumofAF_SamePgm = (Tu8)(u8_NumofAF_SamePgm + (Tu8)(1));
				}
				else
				{
					/* MISRA */
				}
			}
			else
			{
				if((u32_F1Val != (Tu32)(0)) && (u32_F1Val != u32_currTunFreq)&& 
				(AFRepFreq_Check(u32_AFList_RgnlPgm, u8_NumofAF_RgnlPgm, u32_F1Val) != (Tu8)(0x01)))
				{
					u32_AFList_RgnlPgm[u8_NumofAF_RgnlPgm] = u32_F1Val;
					u8_NumofAF_RgnlPgm = (Tu8)(u8_NumofAF_RgnlPgm + (Tu8)(1));
				}
				else
				{
					/* MISRA */
				}

				if((u32_F2Val != (Tu32)(0)) && (u32_F2Val != u32_currTunFreq)&& 
				(AFRepFreq_Check(u32_AFList_RgnlPgm, u8_NumofAF_RgnlPgm, u32_F2Val) != (Tu8)(0x01)))
				{
					u32_AFList_RgnlPgm[u8_NumofAF_RgnlPgm] = u32_F2Val;
					u8_NumofAF_RgnlPgm = (Tu8)(u8_NumofAF_RgnlPgm + (Tu8)(1));
				}
				else
				{
					/* MISRA */
				}
			}
		}

		if((u8_NumOfAF_List == (Tu8)0x00) || ((b_AF_MethodB == TRUE) && (u8_NumofAF_SamePgm + u8_NumofAF_RgnlPgm) == u8_NumOfAF_List))
		{
			b_AF_MethodB = FALSE;
			if(u8_NumofAF_SamePgm != (Tu8)(0))
			{
				SYS_RADIO_MEMCPY(&pst_curr_rds_data->st_FM_RDS_AF.u32_Same_AF_Freq[0], &u32_AFList_SamePgm[0], u8_NumofAF_SamePgm*(sizeof(Tu32)));
			}
			else
			{
				/* MISRA */
			}

			if(u8_NumofAF_RgnlPgm != (Tu8)(0))
			{
				SYS_RADIO_MEMCPY(&pst_curr_rds_data->st_FM_RDS_AF.u32_Rgnl_AF_Freq[u8_NumofAF_SamePgm], &u32_AFList_RgnlPgm[0], u8_NumofAF_RgnlPgm*(sizeof(Tu32)));
			}
			else
			{
				/* MISRA */
			}
			pst_curr_rds_data->st_FM_RDS_AF.u8_Num_Same_Pgm = (Tu8)u8_NumofAF_SamePgm;
			pst_curr_rds_data->st_FM_RDS_AF.u8_Num_Rgnl_Pgm = (Tu8)u8_NumofAF_RgnlPgm;
			pst_curr_rds_data->st_FM_RDS_AF.u8_NumAFeqList = (Tu8)(u8_NumofAF_SamePgm + u8_NumofAF_RgnlPgm);
			pst_curr_rds_data->b_AF_Update = TRUE;
		}
		else
		{
			/* MISRA */
		}

	}	
	else
	{
		/* MISRA */
	}
return 0;
		
}

/*===========================================================================*/
/*  Tu8 FM_RDS_RT_Decode        				                             */
/*===========================================================================*/
Tu8 FM_RDS_RT_Decode(Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks* pst_RDS_Process_All_Blocks, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data)
{
	Tu8 u8_seg_index       = 0xFF;
	Tu8 u8_char_index      = 0xFF;
	Tu8 u8_index		   = 0;
	/* Get RT ::: Get Radio Text from RDS 2A/2B Group 64/32 character Radio Text */
	if (LIB_AND(pst_RDS_Process_All_Blocks->RDS_Block_B[u8_index], (Tu8)(0xF8)) == RDS_DECODER_2A_GROUP)
	{
		/* Receive the Radio Text from 2A Group 64 Characters */
		u8_seg_index = (Tu8)((LIB_AND(pst_RDS_Process_All_Blocks->RDS_Block_B[u8_index+1], (Tu8)(0x0F)))*(Tu8)(4));  //macro addition
		u8_RTdata_size = RDS_DECODER_MAX_RT_SIZE;
		for (u8_char_index = 0; u8_char_index < (Tu8)(16); u8_char_index++)
		{
			if (au8_Prev_seg_index[u8_char_index] == (u8_seg_index/4))
			{
				u8_char_index = 16;
			}
			else if(au8_Prev_seg_index[u8_char_index] == 0xff)
			{
				au8_Prev_seg_index[u8_char_index] = (u8_seg_index / 4);
				u8_seg_index_check++;
				u8_char_index = 16;
			}
			else
			{
				/*Added for MISRA C*/
			}
		}

		for (u8_char_index = 0; u8_char_index < (Tu8)(4); u8_char_index++)
		{
			if (u8_char_index < 2)
			{
				u8_Txtdata[u8_seg_index + u8_char_index] = pst_RDS_Process_All_Blocks->RDS_Block_C[u8_index + u8_char_index];
			}
			else
			{
				u8_Txtdata[u8_seg_index + u8_char_index] = pst_RDS_Process_All_Blocks->RDS_Block_D[u8_index + (u8_char_index - 2)];
			}

			if (u8_Txtdata[u8_seg_index + u8_char_index] == RDS_DECODER_RTEND)
			{
				u8_RT_end_index = u8_seg_index/4;
				u8_char_index = 4;
			}
			else
			{
				/* MISRA */
			}
		}

	}
	else if (LIB_AND(pst_RDS_Process_All_Blocks->RDS_Block_B[u8_index], (Tu8)(0xF8)) == RDS_DECODER_2B_GROUP)
	{
		/* Receive the Radio Text from 2A Group 32 Characters */
		u8_seg_index = (Tu8)((LIB_AND(pst_RDS_Process_All_Blocks->RDS_Block_B[u8_index+1], (Tu8)(0x0F))*(Tu8)(2)));   //macro addition
		u8_RTdata_size  = RDS_DECODER_MAX_GRPB_RT_SIZE;
		for (u8_char_index = 0; u8_char_index < (Tu8)(16); u8_char_index++)
		{
			if (au8_Prev_seg_index[u8_char_index] == (u8_seg_index / 4))
			{
				u8_char_index = 16;
			}
			else if (au8_Prev_seg_index[u8_char_index] == 0xff)
			{
				au8_Prev_seg_index[u8_char_index] = (u8_seg_index / 4);
				u8_seg_index_check++;
				u8_char_index = 16;
			}
			else
			{
				/*Added for MISRA C*/
			}
		}

		for (u8_char_index = 0; u8_char_index <(Tu8)(2); u8_char_index++)
		{
			u8_Txtdata[u8_seg_index + u8_char_index] = pst_RDS_Process_All_Blocks->RDS_Block_D[u8_char_index];
			if (u8_Txtdata[u8_seg_index + u8_char_index] == RDS_DECODER_RTEND)
			{
				u8_RT_end_index = u8_seg_index/2;
				u8_char_index = 2;
			}
			else
			{
				/* MISRA */
			}
		}
	}	
	else
	{
		/* Invalid Radio Text Version */
	}
	if (u8_RT_end_index == (u8_seg_index_check - 1))
	{
		u8_RadioTxtDone = 0x01;
		for (u8_char_index = 0; u8_char_index < (Tu8)(RDS_DECODER_MAX_RT_SIZE); u8_char_index++)
		{
			if (u8_Txtdata[u8_char_index] == RDS_DECODER_RTEND)
			{
				u8_RTdata_size = u8_char_index;
				u8_char_index = 64;
			}
			else
			{
				/* MISRA */
			}
		}
	}
	else if (u8_seg_index_check == 16)
	{
		u8_RadioTxtDone = 0x01;
	}
	else
	{
		/* Do Nothing */
	}

	/* Update the Radio text data */
	if(u8_RadioTxtDone == (Tu8)0x01)	
	{
		memset(pst_curr_rds_data->st_FM_RDS_RT.u8_RT_Data, 0x00, RDS_DECODER_MAX_RT_SIZE);
		SYS_RADIO_MEMCPY(pst_curr_rds_data->st_FM_RDS_RT.u8_RT_Data, u8_Txtdata, (size_t)(u8_RTdata_size));
		pst_curr_rds_data->b_RT_Update = TRUE;
		pst_curr_rds_data->st_FM_RDS_RT.u8_RT_len = u8_RTdata_size;
		
		u8_RadioTxtDone    = 0x00;
		u8_RTdata_size     = 0;
		u8_seg_index_check = 0;
		u8_RT_end_index    = 0xFF;
		memset(u8_Txtdata, 0x00, RDS_DECODER_MAX_RT_SIZE);	
		memset(au8_Prev_seg_index, 0xFF, 16u);
	}
	else
	{
		/* MISRA */
	}
	return 0;
}

/*===========================================================================*/
/*  Tu8 FM_RDS_TA_Decode         				                             */
/*===========================================================================*/
Tu8 FM_RDS_TA_Decode(Tu8* au8_TA_Block_B, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data)
{
	Tu8 u8_B_block_Index = 0;
	/* Get 0A/0B RDS Group Data */
	if ((LIB_AND(au8_TA_Block_B[u8_B_block_Index], (Tu8)(0xF8)) == (Tu8)RDS_DECODER_0A_GROUP) || (LIB_AND(au8_TA_Block_B[u8_B_block_Index], (Tu8)(0xF8)) == (Tu8)RDS_DECODER_0B_GROUP))
	{
		pst_curr_rds_data->st_FM_RDS_TA.u8_TP = LIB_ISBITSET(au8_TA_Block_B[u8_B_block_Index], (Tu8)2);       //macro addition
		pst_curr_rds_data->st_FM_RDS_TA.u8_TA = LIB_ISBITSET(au8_TA_Block_B[++u8_B_block_Index], (Tu8)4);       //macro addition
			
		if( (u8_TA == pst_curr_rds_data->st_FM_RDS_TA.u8_TA) && (u8_TP == pst_curr_rds_data->st_FM_RDS_TA.u8_TP) )
		{
			
		}
		else
		{
			u8_TA = pst_curr_rds_data->st_FM_RDS_TA.u8_TA;
			u8_TP = pst_curr_rds_data->st_FM_RDS_TA.u8_TP;
			pst_curr_rds_data->b_TA_Update = TRUE;
		}
	}
	else
	{
		/* Invalid Group Id - Do Nothing */
	}
	return 0;
}

/*===========================================================================*/
/*  Tu8 FM_RDS_EON_Anno_AF_Decode         				                             */
/*===========================================================================*/
Tu8 FM_RDS_EON_Anno_AF_Decode(Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks* pst_RDS_Process_All_Blocks, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data)
{

	Tu8 u8_varient = 0xFF;
	Tu8 u8_index = 0;
	
	if (LIB_AND(pst_RDS_Process_All_Blocks->RDS_Block_B[u8_index], (Tu8)(0xF8)) == (Tu8)RDS_DECODER_14A_GROUP) /* Get 14A RDS Group Data */
	{
		pst_curr_rds_data->st_FM_RDS_TA.u16_EONpi = (Tu16)(LIB_CONCATENATE((Tu16)pst_RDS_Process_All_Blocks->RDS_Block_D[u8_index], (Tu8)(8), pst_RDS_Process_All_Blocks->RDS_Block_D[u8_index+1]));
		u8_varient = (Tu8)(LIB_AND(pst_RDS_Process_All_Blocks->RDS_Block_B[u8_index+1], (Tu8)(0x0F)));
		if(u8_varient == (Tu8)0x04)
		{
			if ((pst_RDS_Process_All_Blocks->RDS_Block_C[u8_index] > (Tu8)(0)) && (pst_RDS_Process_All_Blocks->RDS_Block_C[u8_index] < (Tu8)(RDS_DECODER_AF_FILLERCODE)))
			{
				pst_curr_rds_data->st_FM_RDS_TA.u32_AF[u8_index] = (Tu32)((pst_RDS_Process_All_Blocks->RDS_Block_C[u8_index] + RDS_DECODER_AFFREQ_MINVAL)*(Tu8)(100));
			}
			else
			{
				/* MISRA */
			}
			
			if ((pst_RDS_Process_All_Blocks->RDS_Block_C[u8_index+1] > (Tu8)(0)) && (pst_RDS_Process_All_Blocks->RDS_Block_C[u8_index+1]  < (Tu8)(RDS_DECODER_AF_FILLERCODE)))
			{
				pst_curr_rds_data->st_FM_RDS_TA.u32_AF[u8_index+1] = (Tu32)((pst_RDS_Process_All_Blocks->RDS_Block_C[u8_index+1] + RDS_DECODER_AFFREQ_MINVAL)*(Tu8)(100));
			}
			else
			{
				/* MISRA */
			}
			pst_curr_rds_data->b_EON_AF_Update = TRUE;
		}
		else
		{
			/* MISRA */
		}
	}
	else
	{
		/* Invalid Group Id - Do Nothing */
	}
	return 0;
}



/*===========================================================================*/
/*  Tu8 FM_RDS_EON_Anno_TA_Decode         				                             */
/*===========================================================================*/
Tu8 FM_RDS_EON_Anno_TA_Decode(Tu8* au8_TA_Block_B, Tu8* au8_TA_Block_D, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data)
{

	Tu8 u8_EONTA = 0xFF;
	Tu8 u8_EONTP = 0xFF;
	Tu8 u8_index = 0;
	pst_curr_rds_data->st_FM_RDS_TA.u16_EONpi = 0xFF;
		
	if ((LIB_AND(au8_TA_Block_B[0], (Tu8)(0xF8)) == (Tu8)RDS_DECODER_14B_GROUP)) /* Get 14B RDS Group Data */
	{
		u8_EONTA = LIB_ISBITSET(au8_TA_Block_B[u8_index+1], (Tu8)3);  //macro addition
		u8_EONTP = LIB_ISBITSET(au8_TA_Block_B[u8_index+1], (Tu8)4);  // macro addition
		if((u8_EONTA == (Tu8)(1)) && (u8_EONTP == (Tu8)(1)))
		{
			pst_curr_rds_data->st_FM_RDS_TA.u16_EONpi = (Tu16)(LIB_CONCATENATE((Tu16)au8_TA_Block_D[u8_index], (Tu8)(8), au8_TA_Block_D[u8_index+1]));  //macro addition
			pst_curr_rds_data->b_EONTA_Update = TRUE;
		}
		else
		{
			/* MISRA */
		}
	}
	else
	{
		/* Invalid Group Id - Do Nothing */
	}

	return 0;
}

 
/*=============================================================================
    end of file
=============================================================================*/

