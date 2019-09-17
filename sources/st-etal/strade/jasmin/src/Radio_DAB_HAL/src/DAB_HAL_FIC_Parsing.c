/*==================================================================================================
start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_HAL_FIC_Parsing.c																       *
*  Copyright (c) 2017, Jasmin Infotech Private Limited.                                            *
*  All rights reserved. Reproduction in whole or part is prohibited                                *
*  without the written permission of the copyright owner.                                          *
*                                                                                                  *
*  Project              :  ST_Radio_Middleware                                                     *
*  Organization			:  Jasmin Infotech Pvt. Ltd.                                               *
*  Module				:  RADIO_DAB_HAL                                                           *
*  Description			:  This file contains FIC and FIB related function definitions			   *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
*                                                                                                  *
***************************************************************************************************/

/*--------------------------------------------------------------------------------------------------
includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_HAL_FIC_Parsing.h"

/*--------------------------------------------------------------------------------------------------
variables (private)
--------------------------------------------------------------------------------------------------*/
Tu8  u8_NoFIB			= 0u;

const Tu16  au16_CrcTable[1][256] =
{
	{ 0x0000u, 0x1021u, 0x2042u, 0x3063u, 0x4084u, 0x50a5u, 0x60c6u, 0x70e7u,
	0x8108u, 0x9129u, 0xa14au, 0xb16bu, 0xc18cu, 0xd1adu, 0xe1ceu, 0xf1efu,
	0x1231u, 0x0210u, 0x3273u, 0x2252u, 0x52b5u, 0x4294u, 0x72f7u, 0x62d6u,
	0x9339u, 0x8318u, 0xb37bu, 0xa35au, 0xd3bdu, 0xc39cu, 0xf3ffu, 0xe3deu,
	0x2462u, 0x3443u, 0x0420u, 0x1401u, 0x64e6u, 0x74c7u, 0x44a4u, 0x5485u,
	0xa56au, 0xb54bu, 0x8528u, 0x9509u, 0xe5eeu, 0xf5cfu, 0xc5acu, 0xd58du,
	0x3653u, 0x2672u, 0x1611u, 0x0630u, 0x76d7u, 0x66f6u, 0x5695u, 0x46b4u,
	0xb75bu, 0xa77au, 0x9719u, 0x8738u, 0xf7dfu, 0xe7feu, 0xd79du, 0xc7bcu,
	0x48c4u, 0x58e5u, 0x6886u, 0x78a7u, 0x0840u, 0x1861u, 0x2802u, 0x3823u,
	0xc9ccu, 0xd9edu, 0xe98eu, 0xf9afu, 0x8948u, 0x9969u, 0xa90au, 0xb92bu,
	0x5af5u, 0x4ad4u, 0x7ab7u, 0x6a96u, 0x1a71u, 0x0a50u, 0x3a33u, 0x2a12u,
	0xdbfdu, 0xcbdcu, 0xfbbfu, 0xeb9eu, 0x9b79u, 0x8b58u, 0xbb3bu, 0xab1au,
	0x6ca6u, 0x7c87u, 0x4ce4u, 0x5cc5u, 0x2c22u, 0x3c03u, 0x0c60u, 0x1c41u,
	0xedaeu, 0xfd8fu, 0xcdecu, 0xddcdu, 0xad2au, 0xbd0bu, 0x8d68u, 0x9d49u,
	0x7e97u, 0x6eb6u, 0x5ed5u, 0x4ef4u, 0x3e13u, 0x2e32u, 0x1e51u, 0x0e70u,
	0xff9fu, 0xefbeu, 0xdfddu, 0xcffcu, 0xbf1bu, 0xaf3au, 0x9f59u, 0x8f78u,
	0x9188u, 0x81a9u, 0xb1cau, 0xa1ebu, 0xd10cu, 0xc12du, 0xf14eu, 0xe16fu,
	0x1080u, 0x00a1u, 0x30c2u, 0x20e3u, 0x5004u, 0x4025u, 0x7046u, 0x6067u,
	0x83b9u, 0x9398u, 0xa3fbu, 0xb3dau, 0xc33du, 0xd31cu, 0xe37fu, 0xf35eu,
	0x02b1u, 0x1290u, 0x22f3u, 0x32d2u, 0x4235u, 0x5214u, 0x6277u, 0x7256u,
	0xb5eau, 0xa5cbu, 0x95a8u, 0x8589u, 0xf56eu, 0xe54fu, 0xd52cu, 0xc50du,
	0x34e2u, 0x24c3u, 0x14a0u, 0x0481u, 0x7466u, 0x6447u, 0x5424u, 0x4405u,
	0xa7dbu, 0xb7fau, 0x8799u, 0x97b8u, 0xe75fu, 0xf77eu, 0xc71du, 0xd73cu,
	0x26d3u, 0x36f2u, 0x0691u, 0x16b0u, 0x6657u, 0x7676u, 0x4615u, 0x5634u,
	0xd94cu, 0xc96du, 0xf90eu, 0xe92fu, 0x99c8u, 0x89e9u, 0xb98au, 0xa9abu,
	0x5844u, 0x4865u, 0x7806u, 0x6827u, 0x18c0u, 0x08e1u, 0x3882u, 0x28a3u,
	0xcb7du, 0xdb5cu, 0xeb3fu, 0xfb1eu, 0x8bf9u, 0x9bd8u, 0xabbbu, 0xbb9au,
	0x4a75u, 0x5a54u, 0x6a37u, 0x7a16u, 0x0af1u, 0x1ad0u, 0x2ab3u, 0x3a92u,
	0xfd2eu, 0xed0fu, 0xdd6cu, 0xcd4du, 0xbdaau, 0xad8bu, 0x9de8u, 0x8dc9u,
	0x7c26u, 0x6c07u, 0x5c64u, 0x4c45u, 0x3ca2u, 0x2c83u, 0x1ce0u, 0x0cc1u,
	0xef1fu, 0xff3eu, 0xcf5du, 0xdf7cu, 0xaf9bu, 0xbfbau, 0x8fd9u, 0x9ff8u,
	0x6e17u, 0x7e36u, 0x4e55u, 0x5e74u, 0x2e93u, 0x3eb2u, 0x0ed1u, 0x1ef0u }

};

/*--------------------------------------------------------------------------------------------------
private function definitions
--------------------------------------------------------------------------------------------------*/
/*================================================================================================*/
/* void ExtractFIB(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tchar *msg)					  */
/*================================================================================================*/
void ExtractFIB(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tchar *msg)
{
	Tu32 u32_Index		= 0u;
	Tu32 u32_Index1		= 0u;
	Tu8  u8_CRC1		= 0u;
	Tu8  u8_CRC2		= 0u;
	Tu8  u8_TempIndex	= 0u;
	Tu8  u8_TempIndex1	= 0u;
	Tu8  u8_FICLength	= 0u;
	Tu8	 u8_FIGLength	= 0u;
	Tu8  au8_FIB[FIB_DATA_LENGTH];
	Tu8  au8_FIG[FIB_DATA_LENGTH];
	Te_CRC_Status e_CRCstatus = CRC_CORRECT;

	u8_FICLength = sizeof(msg);
	if (u8_FICLength != 0)
	{
		u8_NoFIB = (u8_FICLength / MAX_FIB_LENGTH);
		while (u8_NoFIB != 0)
		{
			for (u8_TempIndex = 0; u8_TempIndex < FIB_DATA_LENGTH; u8_TempIndex++)
			{
				au8_FIB[u8_TempIndex] = msg[u32_Index];
				u32_Index++;
			}
			u8_CRC1 = msg[u32_Index++];
			u8_CRC2 = msg[u32_Index++];
			e_CRCstatus = App_CheckCRC(au8_FIB, FIB_DATA_LENGTH, u8_CRC1, u8_CRC2);
			if (e_CRCstatus == CRC_CORRECT)
			{
				while (u32_Index1 < FIB_DATA_LENGTH)
				{
					u8_FIGLength = (Tu8)(LIB_AND(au8_FIB[u32_Index1], 0x1F));
					for (u8_TempIndex1 = 0; u8_TempIndex1 < u8_FIGLength; u8_TempIndex1++)
					{
						au8_FIG[u8_TempIndex1] = msg[u32_Index1];
						u32_Index1++;
					}
					/* Extract FIG details */
					Extract_Figdata(DAB_Tuner_Ctrl_me, (char*)au8_FIG);
				}
			}
			else
			{
				/* Exit from while loop */
				break;
			}
			u8_NoFIB--;
		}
	}
	else
	{
		/* Do nothing */
	}
}

/***************CRC Calculation****************/
/*================================================================================================*/
/* Tu16 Common_CalcCRC(Tu8 au8_message[], Tu32 u32_NumBytes, Tu8 u8_Polynomial,					  */
/*														Tu16 u16_IniValue, Tu16 u16_FinalXORVal)  */
/*================================================================================================*/
Tu16 Common_CalcCRC(Tu8 au8_message[], Tu32 u32_NumBytes, Tu8 u8_Polynomial, Tu16 u16_IniValue, Tu16 u16_FinalXORVal)
{
	Tu32 u32_Count		= 0u;
	Tu16 u16_Remainder	= u16_IniValue;
	Tu8  u8_Data		= 0u;

	if (au8_message == (void*)0u)
	{
		u16_Remainder = 0u;
	}
	else if (u8_Polynomial > (Tu8)CRC_CCITT)
	{
		u16_Remainder = 0u;
	}
	else
	{
		/* Divide the message by the polynomial, a byte at a time */
		for (u32_Count = 0u; u32_Count < u32_NumBytes; ++u32_Count)
		{
			u8_Data = (Tu8)((au8_message[u32_Count]) ^ (Tu16)(u16_Remainder >> (CRC_WIDTH - 8)));
			u16_Remainder = (Tu16)(au16_CrcTable[u8_Polynomial][u8_Data] ^ (Tu16)(u16_Remainder << 8));
		}
	}
	/* The final remainder is the CRC */
	return (Tu16)(u16_Remainder ^ u16_FinalXORVal);
}

/*================================================================================================*/
/* void App_CalculateCRC(Tu8 au8_Input[], Tu32 u32_Length, Tu8* u8_CRC1, Tu8* u8_CRC2)			  */
/*================================================================================================*/
void App_CalculateCRC(Tu8 au8_Input[], Tu32 u32_Length, Tu8* u8_CRC1, Tu8* u8_CRC2)
{
	Ts_CRC_Parameters st_CRC = {CRC_CCITT, 0xFFFF, 0xFFFF };
	Tu16 u16_CalcChecksum	 = 0u;

	u16_CalcChecksum = Common_CalcCRC(au8_Input, u32_Length, (Tu8)st_CRC.e_CRCPolynomial, st_CRC.u16_InitValue, st_CRC.u16_FinalXORValue);
	*u8_CRC1 = (u16_CalcChecksum & 0xFF00) >> 8;
	*u8_CRC2 = u16_CalcChecksum & 0xFF;
}

/*================================================================================================*/
/* Te_CRC_Status App_CheckCRC(Tu8 au8_Input[], Tu32 u32_Length, Tu8 u8_CRC1, Tu8 u8_CRC2)         */
/*================================================================================================*/
Te_CRC_Status App_CheckCRC(Tu8 au8_Input[], Tu32 u32_Length, Tu8 u8_CRC1, Tu8 u8_CRC2)
{
	Tu8 u8_CalculatedCRC1  = 0u;
	Tu8 u8_CalculatedCRC2  = 0u;
	Te_CRC_Status e_Status = CRC_FALSE;

	App_CalculateCRC(au8_Input, u32_Length, &u8_CalculatedCRC1, &u8_CalculatedCRC2);
	if ((u8_CalculatedCRC1 == u8_CRC1) && (u8_CalculatedCRC2 == u8_CRC2))
		e_Status = CRC_CORRECT;
	else
		e_Status = CRC_FALSE;
	return e_Status;
}

/*==================================================================================================
end of file
==================================================================================================*/