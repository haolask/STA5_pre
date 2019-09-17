/*=============================================================================
    start of file
=============================================================================*/
/****************************************************************************************************************/
/** \file AMFM_Tuner_RDS_decoder.h																		             *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														     *
*  All rights reserved. Reproduction in whole or part is prohibited											     *
*  without the written permission of the copyright owner.													     *
*																											     *
*  Project              : ST_Radio_Middleware																		             *
*  Organization			: Jasmin Infotech Pvt. Ltd.															     *
*  Module				: SC_AMFM_TUNER_CTRL																     *
*  Description			:                                                                                        *
                                                                                                                 *																											*
*																											     *
******************************************************************************************************************/

#ifndef AMFM_TUNER_RDS_DECODER_H_
#define AMFM_TUNER_RDS_DECODER_H_

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "sys_task.h"
#include "AMFM_Tuner_Ctrl_Types.h" 
#include "AMFM_HAL_RDS_Collector.h"
#include "lib_bitmanip.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

#define RDS_TIMEOUT 					(65u) 

#define RDS_DECODER_BLOCK_A_ERROR_CHECK_MASK_VALUE		(Tu8)(((Tu8)(1)<<(Tu8)(7))|((Tu8)(1)<<(Tu8)(6)))

#define RDS_DECODER_BLOCK_B_ERROR_CHECK_MASK_VALUE		(Tu8)(((Tu8)(1)<<(Tu8)(5))|((Tu8)(1)<<(Tu8)(4)))

#define RDS_DECODER_BLOCK_C_ERROR_CHECK_MASK_VALUE		(Tu8)(((Tu8)(1)<<(Tu8)(3))|((Tu8)(1)<<(Tu8)(2)))

#define RDS_DECODER_BLOCK_D_ERROR_CHECK_MASK_VALUE		(Tu8)(((Tu8)(1)<<(Tu8)(1))|((Tu8)(1)<<(Tu8)(0)))

#define	RDS_DECODER_0A_GROUP							((Tu8)(0x00))

#define	RDS_DECODER_0B_GROUP							((Tu8)(0x08))

#define	RDS_DECODER_2A_GROUP							((Tu8)(0x20))

#define	RDS_DECODER_2B_GROUP							((Tu8)(0x28))

#define RDS_DECODER_4A_GROUP							((Tu8)(0x40))
#define	RDS_DECODER_14A_GROUP							((Tu8)(0xE0))
				
#define	RDS_DECODER_14B_GROUP							((Tu8)(0xE8))

#define RDS_DECODER_15B_GROUP                           ((Tu8)(0xF8))
#define	RDS_DECODER_RTEND		     					((Tu8)(0x0D))

#define RDS_DECODER_TRUE 								((Tu8)(0x01))

#define RDS_DECODER_FALSE				   			 	((Tu8)(0x00))

#define RDS_DECODER_SUBADDR_RDSSTATUS      			 	((Tu8)(0x07))

#define RDS_DECODER_MAX_PS_SIZE             			((Tu8)(0x08))

#define	RDS_DECODER_AFSIZE_STARTCODE 					((Tu8)(223u))

#define	RDS_DECODER_AFSIZE_ENDCODE    					((Tu8)(250u))

#define	RDS_DECODER_AF_FILLERCODE						((Tu8)(205u))

#define	RDS_DECODER_AFSIZE_MINCODE						((Tu8)(224u))

#define	RDS_DECODER_AFFREQ_MINVAL						((Tu16)(875u))

#define	RDS_DECODER_MAXREADSBUFF						((Tu8)(9u))

#define RDS_DECODER_WRITEBUFFSIZE						((Tu8)(1u))

#define RDS_DECODER_STARTTIMEOUT      	  				((Tu8)(65u))

#define RDS_DECODER_MAX_RT_SIZE             			((Tu8)(64u))

#define RDS_DECODER_MAX_GRPB_RT_SIZE					((Tu8)32u)

#define RDS_DECODER_MAX_AF_LIST         				((Tu8)25u)

#define RDS_DECODER_FMCHECK_NONRDS_STN_DETECT_COUNT		((Tu8)(4u))

//#define RDS_DECODER_FM_NONRDS_STN_DETECT_COUNT 			((Tu8)(20u))
#define RDS_DECODER_FM_NONRDS_STN_DETECT_COUNT 			((Tu8)(1u))
#define RDS_DECODER_BGTUNEPIPSN_NONRDS_STN_DETECT_COUNT	((Tu8)(47u))
/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
 /** 
  *@brief structure 
  */
typedef struct _RDS_PSN
{
	Tu16								   u16_pi;									
    Tu8                                    u8_ps[RDS_DECODER_MAX_PS_SIZE];						
}Ts_AMFM_Tuner_Ctrl_RDS_PSN;

 /** 
  *@brief structure 
  */
typedef struct _RDS_AF
{
	Tu16								   u16_pi;									
    Tu32                                   u32_Same_AF_Freq[RDS_DECODER_MAX_AF_LIST];	
	Tu32                                   u32_Rgnl_AF_Freq[RDS_DECODER_MAX_AF_LIST];	
	Tu8 								   u8_Num_Same_Pgm;	
	Tu8 								   u8_Num_Rgnl_Pgm;
	Tu8                                    u8_NumAFeqList;	
	Tbool                                  b_AF_MethodA;
	Tbool                                  b_AF_MethodB;
}Ts_AMFM_Tuner_Ctrl_RDS_AF;

 /** 
  *@brief structure 
  */
typedef struct _RDS_RT
{
	Tu16								   u16_pi;	
	Tu8 								   u8_RT_len;		
    Tu8                                    u8_RT_Data[RDS_DECODER_MAX_RT_SIZE];	
}Ts_AMFM_Tuner_Ctrl_RDS_RT;

 /** 
  *@brief structure 
  */
typedef struct _RDS_TA
{
	Tu16								   u16_pi;
	Tu16								   u16_EONpi;	
	Tu32								   u32_AF[2];
	Tu8									   u8_TA;
	Tu8									   u8_TP;
		
}Ts_AMFM_Tuner_Ctrl_RDS_TA;


typedef struct _RDS_CT
{
	Tu16								u16_pi;	
    Tu8                                 u8_Hour ;
    Tu8                                 u8_Min;
    Tu8                                 u8_offset_sign;
    Ts8                                 u8_Decode_Localtime_offset;/*decoded from RDS */
    Tu32                                u32_Decode_Mod_Jul_day;    /* Modified Julian Date */
    Tu8                                 u8_Day;                    /*Day of current month*/
    Tu8                                 u8_Month;                  /*month of current year*/
    Tu16                                u16_Curr_Year;            /* current year */
}Ts_AMFM_Tuner_Ctrl_RDS_CT;
 /** 
  *@brief structure 
  */
typedef struct _RDS_Data
{	
    Tbool                             	b_PSN_Update;	
    Tbool                             	b_AF_Update;	
    Tbool                            	b_RT_Update;	
    Tbool                             	b_TA_Update;
	Tbool                             	b_PI_Update;
	Tbool                            	b_EONTA_Update;	
	Tbool                            	b_CT_Update;	
	Tbool								b_NonRDS_Station;
	Tbool								b_PTY_Update;
	Tbool								b_EON_AF_Update;
	Tu8									u8_PTY_Code;
	Tu16								u16_PI_Code;
    Tu32 								u32_TunedFrequency;
		
	Ts_AMFM_Tuner_Ctrl_RDS_PSN			st_FM_RDS_PSN;
	Ts_AMFM_Tuner_Ctrl_RDS_AF 			st_FM_RDS_AF;
	Ts_AMFM_Tuner_Ctrl_RDS_RT 			st_FM_RDS_RT;
	Ts_AMFM_Tuner_Ctrl_RDS_TA 			st_FM_RDS_TA;
	Ts_AMFM_Tuner_Ctrl_RDS_CT           st_FM_RDS_CT;
}Ts_AMFM_Tuner_Ctrl_RDS_Data;


/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

void FM_RDS_DECODER_HandleMsg(Ts_Sys_Msg* pst_msg);

Tu8 FM_RDS_PSN_Decode(Tu8* au8_PSN_Block_B, Tu8* au8_PSN_Block_D, Ts_AMFM_Tuner_Ctrl_RDS_Data *pst_RDS_Data);

Tu8 FM_RDS_AF_Decode(Tu8* au8_AF_Block_C, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data);

Tu8 FM_RDS_RT_Decode(Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks* pst_RDS_Process_All_Blocks, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data);

Tu8 FM_RDS_TA_Decode(Tu8* au8_TA_Block_B, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data);

Tu8 FM_RDS_EON_Anno_TA_Decode(Tu8* au8_TA_Block_B, Tu8* au8_TA_Block_D, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data);

Tu8 FM_RDS_EON_Anno_AF_Decode(Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks* pst_RDS_Process_All_Blocks, Ts_AMFM_Tuner_Ctrl_RDS_Data* pst_curr_rds_data);

Tu8 AFRepFreq_Check(Tu32 AFList[AMFM_TUNER_CTRL_MAX_AF_LIST], Tu8 Numoffreq, Tu32 RecvFreq);

Tu8 FM_RDS_PTY_Decode(Tu8* au8_PTY_Block_B, Tu8* au8_PTY_Block_D, Ts_AMFM_Tuner_Ctrl_RDS_Data *pst_RDS_Data);

void FM_RDS_PI_Decode_Process(Ts_AMFM_Tuner_Ctrl_RDS_Process_AC_Blocks* pst_RDS_Process_AC_Blocks);

void FM_RDS_Group_Decode_Process(Ts_AMFM_Tuner_Ctrl_RDS_Process_All_Blocks* pst_RDS_Process_All_Blocks);

void RDS_Notify_Process(Ts_AMFM_Tuner_Ctrl_RDS_Data *pst_RDS_Data);

void FM_RDS_Notify_Non_RDS_Station();

#endif /* AMFM_TUNER_RDS_DECODER_H_ */

/*=============================================================================
    end of file
=============================================================================*/
