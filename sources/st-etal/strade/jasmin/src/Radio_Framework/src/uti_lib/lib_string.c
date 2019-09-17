/*===============================================================================
    start of file
================================================================================*/
/*******************************************************************************/
/** \file lib_string.c																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains rtos related API definitions									*
*																											*
*																											*
*******************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "lib_string.h"
//#include "iodefine.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    public function definitions
-----------------------------------------------------------------------------*/
/*==============================================================================
void Sys_Radio_Memcpy(void *dest, const void *src, Tu32 u32_len)
===============================================================================*/
void Sys_Radio_Memcpy(void *dest, const void *src, Tu32 u32_len)
{
	if(src!= NULL)
	{
        /* Typecast src and dest addresses to (char *)*/
        char *csrc		= (char *)src;
        char *cdest		= (char *)dest;
        Tu32 u32_index	= 0;
		
        /* Copy contents of src[] to dest[] */
		for (u32_index = 0; u32_index<u32_len; u32_index++)
		{
			cdest[u32_index] = csrc[u32_index];
		}
	}
	else
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_MEMCPY\n");
	}
}

/*===================================================================================
Ts32 Sys_Radio_String_comparison(Tu8 *pu8_src,Tu8 *pu8_dst,Tu8 u8_size)
====================================================================================*/
Ts32 Sys_Radio_String_comparison(Tu8 *pu8_src,Tu8 *pu8_dst,Tu8 u8_size)
{
    Tu8 u8_count = 0;
    while( (u8_count < u8_size -1) && ( pu8_src[u8_count]!=0 && pu8_dst[u8_count]!=0) )
    {
        if(pu8_src[u8_count]!=pu8_dst[u8_count])
        {
            return pu8_src[u8_count]-pu8_dst[u8_count];
        }
		else
		{
			/* Do nothing*/
		}
        u8_count++;
    }
    return pu8_src[u8_count]-pu8_dst[u8_count];
}
/*==================================================================================
 Tu32 Sys_Radio_Strlen(const Tchar *pchar_str)                              
===================================================================================*/
Tu32 Sys_Radio_Strlen(const Tchar *pchar_str)
{
    Tu32 u32_ret = 0U;
	
	if(pchar_str!= NULL)
	{
		for(u32_ret = 0; pchar_str[u32_ret]!= '\0'; ++u32_ret)
		{
			/* Do nothing */
		}
	}
	else
	{
		/* Do nothing */
	}
    
    return u32_ret;
}
/*==================================================================================
const Tchar* Sys_Radio_Strstr(const Tchar* pchar_str1, const Tchar* pchar_str2)       
===================================================================================*/
const Tchar* Sys_Radio_Strstr(const Tchar* pchar_str1, const Tchar* pchar_str2)
{
    const Tchar* pchar_ret = NULL;

    if ((pchar_str1 != NULL) && (pchar_str2 != NULL))
    {
        pchar_ret = strstr((const void*)pchar_str1, (const void*)pchar_str2);
    }
    else
    {
        RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_STR_STR");
    }
    return pchar_ret;
}
/*=============================================================================
void Sys_Radio_Strncpy(Tchar* pchar_dest, const Tchar* pchar_src, Tu32 u32_num)           
==============================================================================*/
Tchar*  Sys_Radio_Strncpy(Tchar* pchar_dest, const Tchar* pchar_src, Tu32 u32_num)
{
    Tchar* pchar_ret = NULL;

    if ((pchar_dest != NULL) && (pchar_src != NULL))
    {
        pchar_ret = strncpy(pchar_dest, pchar_src, u32_num);

        /*
         * Ensure that destination is always null-terminated.
         * This may cause truncation of the copied string.
         */
        if (u32_num > 0U)
        {
            pchar_dest[u32_num - 1U] = '\0';
        }
		else
		{
			/* Do nothing*/
		}
    }
    else
    {
        RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_STRNCPY");
    }
    return pchar_ret;
}
/*==============================================================================
void Sys_Radio_Strncat(Tchar* pchar_dest, const Tchar* pchar_src, Tu32 u32_num)			 
================================================================================*/
Tchar*  Sys_Radio_Strncat(Tchar* pchar_dest, const Tchar* pchar_src, Tu32 u32_num)
{
    Tchar* pchar_ret = NULL;

    if ((pchar_dest != NULL) && (pchar_src != NULL))
    {
         pchar_ret = (Tchar*)strncat((void*)pchar_dest, (const void*)pchar_src, u32_num); /*lint !e960 Violates MISRA 2004 Required Rule 10.1 */
    }
    else
    {
        RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_STRN_CAT");
    }
    return pchar_ret;
}
/*=============================================================================
Ts32 Sys_Radio_Strncmp(const Tchar* pchar_s1, const Tchar* pchar_s2, Tu32 u32_n)
===============================================================================*/
Ts32 Sys_Radio_Strncmp(const Tchar* pchar_s1, const Tchar* pchar_s2, Tu32 u32_n)
{
    Ts32 s32_ret = 1;

    if ((pchar_s1 != NULL) && (pchar_s2 != NULL))
    {
        s32_ret = (Ts32)strncmp(pchar_s1, pchar_s2, u32_n);
    }
    else
    {
        RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_STRN_CMP");
    }
    return s32_ret;
}
/*===========================================================================
const Tchar* Sys_Radio_Strchr(const Tchar* pchar_str, Tu32 u32_character)                                                          
===========================================================================*/
const Tchar* Sys_Radio_Strchr(const Tchar* pchar_str, Tu32 u32_character)
{
    const Tchar* pchar_ret = NULL;

    if (pchar_str != NULL)
    {
        /*lint -save -e960 Violates MISRA 2004 Required Rule 10.1, Prohibited Implicit Conversion: Non-constant argument to function */
        pchar_ret = strchr(pchar_str, (Ts32)u32_character);
        /*lint -restore */
    }
    else
    {
        RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_STR_CHR");
    }
    return pchar_ret;
}
/*===========================================================================
const Tchar* Sys_Radio_Strrchr(const Tchar* pchar_str, Tu32 u32_character)                                                          
===========================================================================*/
const Tchar* Sys_Radio_Strrchr(const Tchar* pchar_str, Tu32 u32_character)
{
    const Tchar* pchar_ret = NULL;

    if (pchar_str != NULL)
    {
        /*lint -save -e960 Violates MISRA 2004 Required Rule 10.1, Prohibited Implicit Conversion: Non-constant argument to function */
        pchar_ret = strrchr(pchar_str, (Ts32)u32_character);
        /*lint -restore  */
    }
    else
    {
        RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_STRR_CHR");
    }
    return pchar_ret;
}
/*===================================================================================
Tbool Sys_Radio_StrToUpr(Tchar* pchar_str, Tu32 u32_num)                                                     
====================================================================================*/
Tbool Sys_Radio_StrToUpr(Tchar* pchar_str, Tu32 u32_num)
{
    Tbool e_ret = FALSE;

    if (pchar_str != NULL)
    {
        Tu32 u32_i = (Tu32)0;
        while ((*pchar_str != (Tchar)0U) && (u32_i< u32_num) )
        {
            if (((Tu8)*pchar_str >= (Tu8)'a') && ((Tu8)*pchar_str <= (Tu8)'z'))
            {                
                *pchar_str = *pchar_str-(Tchar)((Tu8)'a' - (Tu8)'A');               
            }
			else
			{
				/* Do nothing*/
			}
            pchar_str++;
            u32_i++;
        }

        if (*pchar_str == (Tchar)'\0')
        {
            e_ret = TRUE;
        }
		else
		{
			/* Do nothing*/
		}
    }
    else
    {
        RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] Accessing null pointer in SYS_RADIO_STR_UPR");
    }
    return e_ret;
}

/*===============================================================================
    end of file
=================================================================================*/
