/*==================================================================================================
start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_HAL_FIC_Parsing.h																       *
* Copyright (c) 2017, Jasmin Infotech Private Limited.                                             *
*  All rights reserved. Reproduction in whole or part is prohibited                                *
*  without the written permission of the copyright owner.                                          *
*                                                                                                  *
*  Project              :  ST_Radio_Middleware                                                     *
*  Organization			:  Jasmin Infotech Pvt. Ltd.                                               *
*  Module				:  RADIO_DAB_HAL                                                           *
*  Description			:  This file contains declarations of functions that decode FIB from FIC   *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/
#ifndef __DAB_HAL_FIC_PARSING_H__
#define __DAB_HAL_FIC_PARSING_H__

/*--------------------------------------------------------------------------------------------------
includes
--------------------------------------------------------------------------------------------------*/
#include "cfg_types.h"
#include "DAB_Tuner_Ctrl_inst_hsm.h"
#include "DAB_HAL_Fig_Decoder.h"
#include "lib_bitmanip.h"

/*--------------------------------------------------------------------------------------------------
defines
--------------------------------------------------------------------------------------------------*/
#define FIB_DATA_LENGTH				30u
#define MAX_FIB_LENGTH				32u

#define CRC_WIDTH					16u	/* CRC width */

/*--------------------------------------------------------------------------------------------------
type definitions
--------------------------------------------------------------------------------------------------*/
/***************CRC status****************/
typedef enum
{
	CRC_FALSE,
	CRC_CORRECT

}Te_CRC_Status;

/**  CRC polynomial list */
typedef enum
{
	CRC_CCITT   /* CCITT polynomial  0x1021 */

}Te_CMN_CRC_POLY;

/** Structure containing CRC parameters */
typedef struct
{
	Te_CMN_CRC_POLY e_CRCPolynomial;			/* CRC Polynomial ID */
	Tu16			u16_InitValue;              /* Initial value of the remainder */
	Tu16			u16_FinalXORValue;          /* Final XOR value */

}Ts_CRC_Parameters;

/*--------------------------------------------------------------------------------------------------
function declarations extern
--------------------------------------------------------------------------------------------------*/
/**************************************************************************************************/
/**	 \brief                 API for Extarcting FIB blocks.
*   \param[in]				Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tchar *msg
*   \param[out]				void
*   \pre-condition			This API recieves the array of FIC Info for FIC parsing.
*   \details                This API extracts FIB blocks from Array of FIC using FIC length.
*   \post-condition			Extract FIB are sent for FIG parsing
*   \ErrorHandling    		None
*
***************************************************************************************************/
void ExtractFIB(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tchar *msg);

/***************CRC Calculation****************/
/**************************************************************************************************/
/**	 \brief                 API for Common CRC calculation.
*   \param[in]				u8 au8_message[], Tu32 u32_NumBytes, Tu8 u8_Polynomial,
							Tu16 u16_IniValue, Tu16 u16_FinalXORVal
*   \param[out]				Tu16
*   \pre-condition			
*   \details                
*   \post-condition			
*   \ErrorHandling    		None
*
***************************************************************************************************/
Tu16 Common_CalcCRC(Tu8 au8_message[], Tu32 u32_NumBytes, Tu8 u8_Polynomial, Tu16 u16_IniValue, Tu16 u16_FinalXORVal);
/**************************************************************************************************/
/**	 \brief                 API App_CalculateCRC.
*   \param[in]				Tu8 au8_Input[], Tu32 u32_Length, Tu8* u8_CRC1, Tu8* u8_CRC2
*   \param[out]				void
*   \pre-condition			
*   \details                
*   \post-condition			
*   \ErrorHandling    		None
*
***************************************************************************************************/
void App_CalculateCRC(Tu8 au8_Input[], Tu32 u32_Length, Tu8* u8_CRC1, Tu8* u8_CRC2);
/**************************************************************************************************/
/**	 \brief                 API App_CheckCRC.
*   \param[in]				Tu8 au8_Input[], Tu32 u32_Length, Tu8 u8_CRC1, Tu8 u8_CRC2
*   \param[out]				Te_CRC_Status
*   \pre-condition			
*   \details                
*   \post-condition			
*   \ErrorHandling    		None
*
***************************************************************************************************/
Te_CRC_Status App_CheckCRC(Tu8 au8_Input[], Tu32 u32_Length, Tu8 u8_CRC1, Tu8 u8_CRC2);

#endif /* DAB_HAL_FIC_PARSING_H */
/*==================================================================================================
end of file
==================================================================================================*/