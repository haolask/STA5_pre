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
--  Description : interface for bitplane decoding module
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_BITPLANE_H
#define VC1HWD_BITPLANE_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "vc1hwd_stream.h"
#include "vc1hwd_util.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/* Bitplane Coding Modes */
typedef enum {
    BPCM_RAW,
    BPCM_NORMAL_2,
    BPCM_DIFFERENTIAL_2,
    BPCM_NORMAL_6,
    BPCM_DIFFERENTIAL_6,
    BPCM_ROW_SKIP,
    BPCM_COLUMN_SKIP
} BitPlaneCodingMode_e;

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

u16x vc1hwdDecodeBitPlane( strmData_t * const strmData, const u16x colMb,
                           const u16x rowMb, u8 *pData, const u16x bit,
                           u16x * const pRawMask ,  const u16x maskbit,
                           const u16x syncMarker );

#endif /* #ifndef VC1HWD_BITPLANE_H */

