/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/* Copyright (c) 2016, Jasmin Infotech Private Limited.
*  All rights reserved. Reproduction in whole or part is prohibited
*  without the written permission of the copyright owner.
*
*  Project              : ST_Radio_Middleware
*  Organization			: Jasmin Infotech Pvt. Ltd.
*  Module				: HMI IF
*  Description			: This file contains all the function declarations for UTF8 conversion functions
*
*
*********************************************************************************************************/
/** \file hmi_if_utf8_conversion.h 
	 <b> Brief </b>	 API's consists of UTF8 conversion functions declarations 
*********************************************************************************************************/

#ifndef __HMI_IF_UNICODE_CONVERSION_H__
#define __HMI_IF_UNICODE_CONVERSION_H__

#include "cfg_types.h"

//#define UNICODE_ENABLE 

/**
 * @brief This brief enum contains EBU charset
*/
typedef struct
{
	Tu8 u8_EBU;
	Tu32 u32_UTF8;


}HMI_IF_EBU_CHARSET;

/**
 * @brief This brief enum contains UCS2 Encoding Types
*/
typedef enum
{
	ENC_INVALID,
	ENC_UTF16BE,
	ENC_UTF16LE

}Te_UCS2_Encoding_Type;

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function used to convert the EBU data to UTF8 type \n
*						 
*   \param[in]				au8_EBU_Data		Pointer to EBU data(source)
*   \param[in]				au8_UTF8_Data		Pointer to UTF8 converted data (destination)
*   \param[in]				u16_EBU_size		Size of data to be converted
*   \param[in]				u16_UTF8_size		Size of destination data
*   \param[out]				None
*   \pre					For conversion, valid EBU data should come.
*   \post					Data in UTF8 format should shown to HMI
*   \return					None
* 
******************************************************************************************************/
void HMI_IF_EBU_to_UTF8(Tu8 *au8_EBU_Data, Tu8 *au8_UTF8_Data, Tu16 u16_EBU_size, Tu16 u16_UTF8_size);

/*****************************************************************************************************/
/**	  <b> Brief </b>	 \n This function used to convert the UCS2 data to UTF8 type \n
*						 
*   \param[in]				u8_LabelString		Pointer to string to be converted(source)
*   \param[in]				utf8				Pointer to UTF8 converted data (destination)
*   \param[in]				u16_size			Size of data to be converted
*   \param[out]				None
*   \pre					For conversion, valid label string should come in UCS2 form.
*   \post					Data in UTF8 format should shown to HMI
*   \return					None
* 
******************************************************************************************************/
void HMI_IF_UCS2_to_UTF8 (Tu8 *u8_LabelString,Tu8 *utf8,Tu16 u16_size);

#endif

