/*=============================================================================
	start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_RDS_Collector.c                                                                          *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     *
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: 						                                                            *
*  Description			: FM RDS Decoder Process from Tuner.   							                    *
*                                                                                                           *
*                                                                                                           *
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
	includes
-----------------------------------------------------------------------------*/
//#include "AMFM_HAL_RDS_decoder.h"
#include "AMFM_HAL_RDS_Collector.h"
#include "AMFM_Tuner_Ctrl_App.h"

/*-----------------------------------------------------------------------------
defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
variables (extern)
-----------------------------------------------------------------------------*/
extern Tu8		u8_NUMPI;
extern Tbool	b_ForceFast_PI;
extern Tu8  	u8_RDSTimeout;
/*-----------------------------------------------------------------------------
type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
variables (static)
-----------------------------------------------------------------------------*/
Tu32            u32_RDS_Index    = 0;
Tu8             No_of_AC_Blocks  = 1;
Tu8				RDSRead_Buff[54] = {0};

Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data  RDS_Collect_Data;
Ts_Sys_Msg                           st_sndmsg;

/*-----------------------------------------------------------------------------
private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*	void RDS_Block_Control_Info   				                             */
/*===========================================================================*/
void RDS_Block_Control_Info(Tu8* RDSRead_Buffer, Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data* pRDS_Collect_Data)
{
	u32_RDS_Index++;

	/* Decoding Block control bits */
	pRDS_Collect_Data->u8_Block_ID = LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x03);
	pRDS_Collect_Data->b_Ctype = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x04), 2);
	pRDS_Collect_Data->b_BlockE = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x08), 3);
	pRDS_Collect_Data->u8_ERR_count = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x70), 4);
}

/*===========================================================================*/
/*  RDS_Collect_ErrorFree_All_Blocks									     */
/*===========================================================================*/
void RDS_Collect_ErrorFree_All_Blocks(Tu8 *RDSRead_Buffer, Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data* pRDS_Collect_Data)
{
	Tu8 Update_loop;
	pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_A_Block = FALSE;
	pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_B_Block = FALSE;
	pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_C_Block = FALSE;
	pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_D_Block = FALSE;

	while ((u32_RDS_Index) < (pRDS_Collect_Data->u32_Actual_RDS_Buffer_Size - 1))
	{   
		RDS_Block_Control_Info(RDSRead_Buffer, pRDS_Collect_Data);

		switch (pRDS_Collect_Data->u8_Block_ID)
		{
			case RDS_DECODER_BLOCK_A:
			{  
				/* Check if the Block A is having no error */
				if (pRDS_Collect_Data->u8_ERR_count == RDS_DECODER_BLOCK_NO_ERROR)
				{
					/* Collect Block A RDS Data*/
					for (Update_loop = 0; Update_loop < RDS_ACTWL_DATA_BYTE; Update_loop++)
					{  
						pRDS_Collect_Data->st_RDS_Process_All_Blocks.RDS_Block_A[Update_loop] = RDSRead_Buffer[++(u32_RDS_Index)];
					}
					pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_A_Block = TRUE;
				}
				else
				{
					(u32_RDS_Index) += 2;// if the block is having error then the error block indexes have to be skipped
				}
			}
			break;
			case RDS_DECODER_BLOCK_B:
			{   /* Check if the Block B is having no error */
				if (pRDS_Collect_Data->u8_ERR_count == RDS_DECODER_BLOCK_NO_ERROR)
				{
					/* Collect Block B RDS Data*/
					for (Update_loop = 0; Update_loop < RDS_ACTWL_DATA_BYTE; Update_loop++)
					{
						pRDS_Collect_Data->st_RDS_Process_All_Blocks.RDS_Block_B[Update_loop] = RDSRead_Buffer[++(u32_RDS_Index)];
					}
					pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_B_Block = TRUE;
				}
				else
				{
					(u32_RDS_Index) += 2;// skipping the error block indexes
				}
			}
			break;
			case RDS_DECODER_BLOCK_C:
			{   
				/* Check if the Block C is having no error */
				if (pRDS_Collect_Data->u8_ERR_count == RDS_DECODER_BLOCK_NO_ERROR)
				{
					/* Check if Block C is having PI i.e.,C' Block */
					if (pRDS_Collect_Data->b_Ctype == TRUE)
					{
						pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_Ctype_Flag = TRUE;
					}
					else
					{
						/* MISRA C */
					}
					/* Collect Block C RDS Data*/
					for (Update_loop = 0; Update_loop < RDS_ACTWL_DATA_BYTE; Update_loop++)
					{
						pRDS_Collect_Data->st_RDS_Process_All_Blocks.RDS_Block_C[Update_loop] = RDSRead_Buffer[++(u32_RDS_Index)];
					}
					pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_C_Block = TRUE;
				}
				else
				{
					(u32_RDS_Index) += 2;// skipping the error block indexes
				}
			}
			break;
			case RDS_DECODER_BLOCK_D:
			{   
				/* Check if the Block D is having no error */
				if (pRDS_Collect_Data->u8_ERR_count == RDS_DECODER_BLOCK_NO_ERROR)
				{
					/* Collect Block D RDS Data*/
					for (Update_loop = 0; Update_loop < RDS_ACTWL_DATA_BYTE; Update_loop++)
					{
						pRDS_Collect_Data->st_RDS_Process_All_Blocks.RDS_Block_D[Update_loop] = RDSRead_Buffer[++(u32_RDS_Index)];
					}
					pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_D_Block = TRUE;
				}
				else
				{
					(u32_RDS_Index) += 2;// skipping the error block indexes
				}
			}
			break;
			default:
			{
				/*Nothing*/
			}
			break;
		}
		/*check if all blocks i.e., A,B,C,D are collected */
		if ((pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_A_Block == TRUE) && (pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_B_Block == TRUE) && (pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_C_Block == TRUE) && (pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_D_Block == TRUE))
		{   
			/*process all blocks of a RDS group */
			FM_RDS_Group_Decode_Process(&pRDS_Collect_Data->st_RDS_Process_All_Blocks);
			pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_A_Block = FALSE;
			pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_B_Block = FALSE;
			pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_C_Block = FALSE;
			pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_D_Block = FALSE;
			pRDS_Collect_Data->st_RDS_Process_All_Blocks.b_Ctype_Flag = FALSE;
		}
		else
		{
			/* MISRA C */
		}
	}
}
/*===========================================================================*/
/*  RDS_Collect_ErrorFree_AC_Blocks											 */
/*===========================================================================*/
void RDS_Collect_ErrorFree_AC_Blocks(Tu8 *RDSRead_Buffer, Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data* pRDS_Collect_Data)
{
	Tu8 Block_index ;
	Tu8 Update_loop;
	pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_A_Block = FALSE;
	pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_C_Block = FALSE;
	//Tu8 Block_index = 0;

	while ((No_of_AC_Blocks <= u8_NUMPI) && (u32_RDS_Index < (pRDS_Collect_Data->u32_Actual_RDS_Buffer_Size - 1)))
	{
		RDS_Block_Control_Info(RDSRead_Buffer, pRDS_Collect_Data);
		/*Check if Block A is present and has no errors*/
		if ((pRDS_Collect_Data->u8_Block_ID == RDS_DECODER_BLOCK_A) && (pRDS_Collect_Data->u8_ERR_count == RDS_DECODER_BLOCK_NO_ERROR))
		{
			/* Collect Block A RDS Data*/
			for (Block_index = 0; Block_index < RDS_ACTWL_DATA_BYTE; Block_index++)
			{
				pRDS_Collect_Data->st_RDS_Process_AC_Blocks.RDS_Block_A[Block_index] = RDSRead_Buffer[++u32_RDS_Index];
			}
			pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_A_Block = TRUE;
			No_of_AC_Blocks++;
		}
		/*Check if Block C is present and has no errors*/
		else if ((pRDS_Collect_Data->u8_Block_ID == RDS_DECODER_BLOCK_C) && (pRDS_Collect_Data->u8_ERR_count == RDS_DECODER_BLOCK_NO_ERROR))
		{
			/* Collect Block C RDS Data*/
			for (Update_loop = 0; Update_loop < RDS_ACTWL_DATA_BYTE; Update_loop++)
			{
				pRDS_Collect_Data->st_RDS_Process_AC_Blocks.RDS_Block_C[Update_loop] = RDSRead_Buffer[++u32_RDS_Index];
			}
			pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_C_Block = TRUE;
			No_of_AC_Blocks++;
		}
		else
		{
			(u32_RDS_Index) += 2; // to skip the error block indexes
		}

		/*Check if either A or C Block is present*/
		if ((pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_A_Block == TRUE) || (pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_C_Block == TRUE))
		{
			/*Read A and C Blocks*/
			FM_RDS_PI_Decode_Process(&pRDS_Collect_Data->st_RDS_Process_AC_Blocks);
			pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_A_Block = FALSE;
			pRDS_Collect_Data->st_RDS_Process_AC_Blocks.b_C_Block = FALSE;
		}
		else
		{
			/* MISRA C */
		}
	}
	No_of_AC_Blocks = 1;
}

/*===========================================================================*/
/*  RDS_CB_Mode_Check															*/
/*===========================================================================*/
Tu8 RDS_CB_Mode_Check(Tu8 u8_numPI, Tbool b_forcefastPI)
{
	Tu8 Mode;

	/*
	* Note: DO not optimise below if block to avoid confusion
	* Reference:TDA770X_STAR_API_IF.pdf Page 102
	*/

	if ((u8_numPI == 0) && (b_forcefastPI == FALSE))
	{
		/* All blocks(A B C D) are fed out.The host is notified after
		receiving NQRST valid blocks.*/
		Mode = ETAL_RDS_MODE_0;
	}
	else if ((u8_numPI == 0) && (b_forcefastPI == TRUE))
	{
		/*Not allowed*/
		Mode = ETAL_RDS_MODE_1;
	}
	else if ((u8_numPI > 0) && (b_forcefastPI == FALSE))
	{
		/* After receiving valid NUMPI number of blocks A or C', the
		* mode returns to normal mode automatically i.e.,
		* host is notified by receiving NRQST valid blocks (A B C D).
		*/
		Mode = ETAL_RDS_MODE_2;
	}
	else if ((u8_numPI > 0) && (b_forcefastPI == TRUE))
	{
		/* Only blocks A and C' are fed out.
		* Notification is done after receiving NUMPI
		* valid blocks A or C'.
		*/
		Mode = ETAL_RDS_MODE_3;
	}
	else
	{
		/* Received invalid mode*/
		Mode = ETAL_STUB_RDS_MODE_INVALID;
	}

	return Mode;
}


/*===========================================================================*/
/*  void RDS_Process   				                                         */
/*===========================================================================*/
void RDS_Blocks_Read(Tu8* RDSRead_Buffer, Tu32 dwActualBufferSize)
{
	Tu8 u8_mode = 0;
	Tu32 u32_tmp = 0;
	Tu32 u32_tmp1 = 0;
	Tu32 u32_tmp2 = 0;
	u32_RDS_Index = 0;

	if (dwActualBufferSize > 0)
	{ 
		/*Decode the Read Notification Register bits */
		RDS_Collect_Data.b_RDS_DATARDY     = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x80), 7);
		RDS_Collect_Data.b_RDS_SYNC        = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x40), 6);
		RDS_Collect_Data.b_RDS_BOF         = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x20), 5);
		RDS_Collect_Data.b_RDS_BNE         = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x10), 4);
		u32_tmp                            = (Tu32)(LIB_SHIFTLEFT((LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x0F)), 12));
		u32_tmp1                           = (Tu32)(LIB_SHIFTLEFT((LIB_AND(RDSRead_Buffer[++u32_RDS_Index], 0XFF)), 4));
		u32_tmp2                           = (Tu32)(LIB_SHIFTRIGHT((LIB_AND(RDSRead_Buffer[++u32_RDS_Index], 0XF0)), 4));
		RDS_Collect_Data.u32_RDS_Deviation = u32_tmp | u32_tmp1 | u32_tmp2;
		RDS_Collect_Data.b_TUNCH2          = LIB_SHIFTRIGHT(LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x02), 1);
		RDS_Collect_Data.b_TUNCH1          = LIB_AND(RDSRead_Buffer[u32_RDS_Index], 0x01);
    
		RDS_Collect_Data.u32_Actual_RDS_Buffer_Size = dwActualBufferSize;
	}
	/*Check if RDS Buffer is having atleast one data word to collect */
	if ((RDS_Collect_Data.b_RDS_BNE != FALSE))
	{   
		/*Decrementing the count for NON-RDS station*/
		if (u8_RDSTimeout > 0)
		{
			u8_RDSTimeout -= 1;
		}
			
		/*Collect the modes */
		u8_mode = RDS_CB_Mode_Check(u8_NUMPI, b_ForceFast_PI);
		switch (u8_mode)
		{
			case ETAL_RDS_MODE_0:
			{
				/*decoding A B C D blocks*/
				RDS_Collect_ErrorFree_All_Blocks(RDSRead_Buffer, &RDS_Collect_Data);
			}
			break;
			case ETAL_RDS_MODE_1:
			{
				/*not allowed*/
			}
			break;
			case ETAL_RDS_MODE_2:
			{
				/*decoding valid A and C blocks*/
				RDS_Collect_ErrorFree_AC_Blocks(RDSRead_Buffer,&RDS_Collect_Data);
			
				/*when the numPi number of A, C’ RDS blocks is reached the mode changes to normal mode automatically */ 
				RDS_Collect_ErrorFree_All_Blocks(RDSRead_Buffer, &RDS_Collect_Data);
			}
			break;
			case ETAL_RDS_MODE_3:
			{
				/*decoding valid A and C blocks*/
				RDS_Collect_ErrorFree_AC_Blocks(RDSRead_Buffer, &RDS_Collect_Data);
			}
			break;
		}
	}
	else
	{
		/* MISRA C */
	}
}

void RDS_CB_Notify(Tu8* pBuffer, Tu32 dwActualBufferSize)
{
	Ts_Sys_Msg* pst_reqmsg = &st_sndmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg, RADIO_FM_RDS_DECODER, RDS_CALLBACK_DATA_RDY_MSG_ID);
	/* Function to assign RDS callback data actual size in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &dwActualBufferSize, (Tu8)(sizeof(Tu8)), &(pst_reqmsg->msg_length));
	/* Function to assign RDS callback data in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), pBuffer, (Tu8)(dwActualBufferSize), &(pst_reqmsg->msg_length));

	SYS_SEND_MSG(pst_reqmsg);
}
/*===========================================================================*/
/*  RDS_cbFunc									                             */
/*===========================================================================*/
void RDS_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* gRDSMode)
{
	UNUSED(status);
	UNUSED(gRDSMode);
	RDS_CB_Notify(pBuffer, dwActualBufferSize);
}
/*=============================================================================
	end of file
=============================================================================*/

