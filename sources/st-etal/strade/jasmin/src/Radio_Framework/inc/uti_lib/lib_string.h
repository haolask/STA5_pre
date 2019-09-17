/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file lib_string.h																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains platform related API's declarations and Macro definitions		*
*																											*
*																											*
*************************************************************************************************************/

#ifndef LIB_STRING_H_
#define LIB_STRING_H_

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "debug_log.h"
#include <ctype.h>
#include "cfg_types.h"
#include <string.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
#define SYS_RADIO_MEMCPY		Sys_Radio_Memcpy
#define SYS_RADIO_STR_CMP		Sys_Radio_String_comparison
#define SYS_RADIO_STR_LEN		Sys_Radio_Strlen
#define SYS_RADIO_STR_STR		Sys_Radio_Strstr
#define SYS_RADIO_STRNCPY		Sys_Radio_Strncpy
#define SYS_RADIO_STRN_CAT		Sys_Radio_Strncat
#define SYS_RADIO_STRN_CMP		Sys_Radio_Strncmp
#define SYS_RADIO_STR_CHR		Sys_Radio_Strchr
#define SYS_RADIO_STRR_CHR		Sys_Radio_Strrchr
#define SYS_RADIO_STR_UPR		Sys_Radio_StrToUpr
/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    function declarations intern
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    function declarations extern
-----------------------------------------------------------------------------*/
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_Memcpy is copy the content from source to destination
*   \param[in]				void* src--   
*							void* dst-- 
*                           Tu8 len -- size number of bytes to copy
*   \param[out]				void
*   \pre-condition			No preconditions
*   \details                This API function used to copy data
*   \post-condition			No preconditions
*   \ErrorHandling    		None
* 
******************************************************************************************************/
void Sys_Radio_Memcpy(void *dest, const void *src, Tu32 u32_len);
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_String_comparison is get string comparision
*   \param[in]				Tu8* src-- src First string to compare  
*							Tu8* dst-- dst Second string to compare
*                           Tu8 size -- size number of characters to compare
*   \param[out]				Ts32
*   \pre-condition			No preconditions
*   \details                This API function used to get string length
*   \post-condition			No preconditions
*   \ErrorHandling    		None
* 
******************************************************************************************************/
Ts32 Sys_Radio_String_comparison(Tu8 *pu8_src,Tu8 *pu8_dst,Tu8 u8_size);
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_Strlen is get string length
*   \param[in]				const Tchar* str-- str pointer to string  
*                           
*   \param[out]				Tu32
*   \pre-condition			No preconditions
*   \details                This API function used to get string length
*   \post-condition			No preconditions
*   \ErrorHandling    		None
* 
******************************************************************************************************/
Tu32 Sys_Radio_Strlen(const Tchar* pchar_str);
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_Strstr is Locate sub string
*   \param[in]				const Tchar* str1-- string1   
*                           const Tchar* str2 -- string2 containing the sequence of characters to match
*   \param[out]				const Tchar*
*   \pre-condition			No preconditions
*   \details                This API function used to locate sub string
*   \post-condition			No preconditions
*   \ErrorHandling    		Returns NULL in case search pattern not found or search not possible
* 
******************************************************************************************************/
const Tchar* Sys_Radio_Strstr(const Tchar* pchar_str1, const Tchar* pchar_str2);
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_Strncpy is Copy part of a string
*   \param[in]				Tchar* dest-- Destination string
*                           const Tchar* src -- Source string
*							Tu32 num --number of characters to copy
*   \param[out]				Tchar*
*   \pre-condition			No preconditions
*   \details                This API function used to Copy part of a string
*   \post-condition			No preconditions
*   \ErrorHandling    		A System Error is called in case \p dest or \p src is a null pointer.
* 
******************************************************************************************************/
Tchar* Sys_Radio_Strncpy(Tchar* pchar_dest, const Tchar* pchar_src, Tu32 u32_num);
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_Strncat is Append characters from string
*   \param[in]				Tchar* dest-- Destination string
*
*                           const Tchar* src -- Source string
*							Tu32 num --number of characters to concatenate
*   \param[out]				Tchar*
*   \pre-condition			No preconditions
*   \details                This API function used to Append characters from string
*   \post-condition			No preconditions
*   \ErrorHandling    		Returns NULL in case given pointers are NULL.
* 
******************************************************************************************************/
Tchar* Sys_Radio_Strncat(Tchar* pchar_dest, const Tchar* pchar_src, Tu32 u32_num);
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_Strncmp is Compare part of two strings
*   \param[in]				const Tchar* s1-- First string to be compared.
* 							const Tchar* s2 --Second string to be compared.
*                           Tu32 n -- Maximum number of characters to compare.
*							
*   \param[out]				Ts32
*   \pre-condition			No preconditions
*   \details                This API function used to Compare part of two strings
*   \post-condition			No preconditions
*   \ErrorHandling    		Returns NULL in case given pointers are NULL.
* 
******************************************************************************************************/
Ts32 Sys_Radio_Strncmp(const Tchar* pchar_s1, const Tchar* pchar_s2, Tu32 u32_num);
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Radio_Strchr is locate first occurrence of character in string
*   \param[in]				const Tchar* str--  string 
* 							Tu32 character--Character to be search in string
*                           
*							
*   \param[out]				const Tchar*
*   \pre-condition			No preconditions
*   \details                This API function used to locate first occurrence of character in string
*   \post-condition			No preconditions
*   \ErrorHandling    		Returns NULL in case given pointer is NULL.
* 
******************************************************************************************************/
const Tchar* Sys_Radio_Strchr(const Tchar* pchar_str, Tu32 u32_character);
/*****************************************************************************************************/
/**	 \brief                 The API Function Sys_Radio_Strrchr is Locate last occurrence of character in string
*   \param[in]				const Tchar* str--  string 
* 							Tu32 character--Character to be search in string
*                           
*							
*   \param[out]				const Tchar*
*   \pre-condition			No preconditions
*   \details                This API function used to Locate last occurrence of character in string
*   \post-condition			No preconditions
*   \ErrorHandling    		Returns NULL in case given pointer is NULL.
* 
******************************************************************************************************/
const Tchar* Sys_Radio_Strrchr(const Tchar* pchar_str, Tu32 u32_character);
/*****************************************************************************************************/
/**	 \brief                 The API Function Sys_Radio_StrToUpr is Convert a string to uppercase letters
*   \param[in]				Tchar* str--  string 
* 							Tu32 character--Character to Convert Uppercase
*                           
*							
*   \param[out]				Tbool
*   \pre-condition			No preconditions
*   \details                This API function used to Convert a string to uppercase letters
*   \post-condition			No preconditions
*   \ErrorHandling    		In case the parameter str is a null pointer a System Error is called.
* 
******************************************************************************************************/
Tbool Sys_Radio_StrToUpr(Tchar* pchar_str, Tu32 u32_num);

#endif /* LIB_STRING_H_ */
/*=====================================================================================================
    end of file
======================================================================================================*/
