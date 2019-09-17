/*
 * (C) COPYRIGHT 2011 HANTRO PRODUCTS
 *
 * Please contact: hantro-support@verisilicon.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

/*------------------------------------------------------------------------------
--
--  Description : Basic type definitions.
--
------------------------------------------------------------------------------*/

/*!\file
 * \brief Basic type definitions.
 *
 * Basic numeric data type definitions used in the decoder software.
 */


#ifndef __BASETYPE_H__
#define __BASETYPE_H__

/*! \addtogroup common Common definitions
 *  @{
 */

#if defined( __linux__ ) || defined( WIN32 )
#include <stddef.h>
#endif

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

/* Macro to signal unused parameter. */
#define UNUSED(x) (void)(x)

typedef unsigned char u8; /**< unsigned 8 bits integer value */
typedef signed char i8; /**< signed 8 bits integer value */
typedef unsigned short u16; /**< unsigned 16 bits integer value */
typedef signed short i16; /**< signed 16 bits integer value */
typedef unsigned int u32; /**< unsigned 32 bits integer value */
typedef signed int i32; /**< signed 8 bits integer value */

/*!\cond SWDEC*/
/* SW decoder 16 bits types */
#if defined(VC1SWDEC_16BIT) || defined(MP4ENC_ARM11)
typedef unsigned short u16x;
typedef signed short i16x;
#else
typedef unsigned int u16x;
typedef signed int i16x;
#endif
/*!\endcond */

/*! @} - end group common */

#endif /* __BASETYPE_H__ */
