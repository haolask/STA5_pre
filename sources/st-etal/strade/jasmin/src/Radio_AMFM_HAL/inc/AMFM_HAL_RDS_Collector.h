/*=============================================================================
	start of file
=============================================================================*/
/****************************************************************************************************************/
/** \file AMFM_Tuner_RDS_Collector.h																		         *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														     *
*  All rights reserved. Reproduction in whole or part is prohibited											     *
*  without the written permission of the copyright owner.													     *
*																											     *
*  Project              : ST_Radio_Middleware																     *
*  Organization			: Jasmin Infotech Pvt. Ltd.															     *
*  Module				: SC_AMFM_TUNER_CTRL																     *
*  Description			:                                                                                        *
*																											     *
*																											     *
******************************************************************************************************************/

#ifndef AMFM_TUNER_RDS_COLLECTOR_H_
#define AMFM_TUNER_RDS_COLLECTOR_H_

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
	 defines
-----------------------------------------------------------------------------*/
#define ETAL_RDS_MODE_0 								( 0 )
#define ETAL_RDS_MODE_1									( 1 )
#define ETAL_RDS_MODE_2									( 2 )
#define ETAL_RDS_MODE_3									( 3 )
#define ETAL_STUB_RDS_MODE_INVALID						(0xFF)

#define RDS_DECODER_BLOCK_A                               ((Tu8)0x00)
#define RDS_DECODER_BLOCK_B                               ((Tu8)0x01)
#define RDS_DECODER_BLOCK_C                               ((Tu8)0x02)
#define RDS_DECODER_BLOCK_D                               ((Tu8)0x03)
#define RDS_DECODER_BLOCK_NO_ERROR                        ((Tu8)0x000)

#define RDS_ACTWL_DATA_BYTE								  2
#define RDS_START_INDEX								      3
#define RDS_SIGNAL_THRESHOLD							  50

/*-----------------------------------------------------------------------------
	type definitions
-----------------------------------------------------------------------------*/
 /**
 *@brief structure
 */

typedef struct 
{
	Tu8                 RDS_Block_A[RDS_ACTWL_DATA_BYTE];
	Tu8                 RDS_Block_B[RDS_ACTWL_DATA_BYTE];
	Tu8                 RDS_Block_C[RDS_ACTWL_DATA_BYTE];
	Tu8                 RDS_Block_D[RDS_ACTWL_DATA_BYTE];
	Tbool				b_A_Block;
	Tbool				b_B_Block;
	Tbool				b_C_Block;
	Tbool				b_D_Block;
	Tbool               b_Ctype_Flag;
}Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks;

 /**
 *@brief structure
 */
typedef struct 
{
	Tu8					RDS_Block_A[RDS_ACTWL_DATA_BYTE];
	Tu8					RDS_Block_C[RDS_ACTWL_DATA_BYTE];
	Tbool				b_A_Block;
	Tbool				b_C_Block;
}Ts_AMFM_Tuner_Ctrl_RDS_Process_AC_Blocks;
 /**
 *@brief structure
 */
typedef struct 
{
	
	Tu32								      u32_RDS_Deviation;
	Tu32                                      u32_No_of_ValidEntries;
	Tu32                                      u32_Actual_RDS_Buffer_Size;
	Tu8									      u8_Block_ID;
	Tu8									      u8_ERR_count;
	Tbool								      b_Ctype;
	Tbool							          b_BlockE;
	Tbool                                     b_TUNCH1;
	Tbool                                     b_TUNCH2;
	Tbool								      b_RDS_DATARDY;
	Tbool								      b_RDS_SYNC;
	Tbool								      b_RDS_BOF;
	Tbool								      b_RDS_BNE;
	
	Ts_AMFM_Tuner_Ctrl_RDS_Process_AC_Blocks  st_RDS_Process_AC_Blocks;
	Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks st_RDS_Process_All_Blocks;
}Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data;

/*-----------------------------------------------------------------------------
variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
Function declarations
-----------------------------------------------------------------------------*/
void RDS_Blocks_Read(Tu8* RDSRead_Buffer, Tu32 dwActualBufferSize);
void RDS_Collect_ErrorFree_All_Blocks(Tu8* RDSRead_Buffer, Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data* RDS_Collect_Data);
void RDS_Collect_ErrorFree_AC_Blocks(Tu8* RDSRead_Buffer, Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data* RDS_Collect_Data);
void RDS_Block_Control_Info(Tu8* RDSRead_Buffer, Ts_AMFM_Tuner_Ctrl_RDS_Collect_Data* RDS_Collect_Data);
Tu8  RDS_CB_Mode_Check(Tu8 u8_numPI, Tbool b_forcefastPI);
void RDS_CB_Notify(Tu8* pBuffer, Tu32 dwActualBufferSize);
#endif
/*=============================================================================
	end of file
=============================================================================*/
