/***********************************************************************************/
/*!
*
*  \file      sta10xx_type.h
*
*  \brief     <i><b> Standard basic platform types </b></i>
*
*  \details
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2013/04/22
*
*  \warning
*
*  This file is part of STAudioLib and is dual licensed,
*  ST Proprietary License or GNU General Public License version 2.
*
********************************************************************************
*
* Copyright (c) 2014 STMicroelectronics - All Rights Reserved
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics.
* FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT LOCATED
* IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
*
********************************************************************************
*
* ALTERNATIVELY, this software may be distributed under the terms of the
* GNU General Public License ("GPL") version 2, in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* STAudioLib is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* STAudioLib is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __STM32F10x_TYPE_H
#define __STM32F10x_TYPE_H

//Don't include directly if not FREE_RTOS
#if !defined FREE_RTOS && !defined STA_PLATFORM_H_
#error "Please, don't include sta10xx_type.h directly, instead include sta_platforms.h!"
#endif


/* Includes -----------------------------------------------------------------*/

#ifdef LINUX_OS
#include <linux/types.h>
#include <linux/kernel.h>
#endif

#ifdef TKERNEL
#include <typedef.h>
#endif


/* Exported types -----------------------------------------------------------*/

#ifndef LINUX_OS
typedef signed long long s64;
typedef signed long  s32;
typedef signed short s16;
typedef signed char  s8;

typedef volatile signed long  vs32;
typedef volatile signed short vs16;
typedef volatile signed char  vs8;

typedef unsigned long long u64;
typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef unsigned long  const uc32;  /* Read Only */
typedef unsigned short const uc16;  /* Read Only */
typedef unsigned char  const uc8;   /* Read Only */

typedef volatile unsigned long  vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char  vu8;

typedef volatile unsigned long  const vuc32;  /* Read Only */
typedef volatile unsigned short const vuc16;  /* Read Only */
typedef volatile unsigned char  const vuc8;   /* Read Only */

#else

typedef volatile s32 vs32;
typedef volatile s16 vs16;
typedef volatile s8 vs8;

typedef const u32 uc32;
typedef const u16 uc16;
typedef const u8 uc8;

typedef volatile u32 vu32;
typedef volatile u16 vu16;
typedef volatile u8 vu8;

typedef volatile const u32 vuc32;
typedef volatile const u16 vuc16;
typedef volatile const u8 vuc8;
#endif

#ifdef FREE_RTOS
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif

#if defined TKERNEL && !defined __cplusplus
typedef BOOL bool;
#endif

#ifdef LINUX_OS
#define TRUE 1
#define FALSE 0
#endif

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;


/* Exported constants --------------------------------------------------------*/

#ifndef LINUX_OS
#define U8_MAX     ((u8)255)
#define S8_MAX     ((s8)127)
#define S8_MIN     ((s8)-128)
#define U16_MAX    ((u16)65535u)
#define S16_MAX    ((s16)32767)
#define S16_MIN    ((s16)-32768)
#define U32_MAX    ((u32)4294967295uL)
#define S32_MAX    ((s32)2147483647)
#define S32_MIN    ((s32)2147483648uL)
#endif


/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __STM32F10x_TYPE_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
