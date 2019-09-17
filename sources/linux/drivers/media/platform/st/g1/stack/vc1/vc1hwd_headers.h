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
--  Description : Interface for sequence level header decoding
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_HEADERS_H
#define VC1HWD_HEADERS_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "vc1hwd_util.h"
#include "vc1hwd_stream.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/* enumerated sample aspect ratios, ASPECT_RATIO_M_N means M:N */
enum
{
    ASPECT_RATIO_UNSPECIFIED = 0,
    ASPECT_RATIO_1_1,
    ASPECT_RATIO_12_11,
    ASPECT_RATIO_10_11,
    ASPECT_RATIO_16_11,
    ASPECT_RATIO_40_33,
    ASPECT_RATIO_24_11,
    ASPECT_RATIO_20_11,
    ASPECT_RATIO_32_11,
    ASPECT_RATIO_80_33,
    ASPECT_RATIO_18_11,
    ASPECT_RATIO_15_11,
    ASPECT_RATIO_64_33,
    ASPECT_RATIO_160_99,
    ASPECT_RATIO_EXTENDED = 15
};

typedef enum {
    SC_END_OF_SEQ       = 0x0000010A,
    SC_SLICE            = 0x0000010B,
    SC_FIELD            = 0x0000010C,
    SC_FRAME            = 0x0000010D, 
    SC_ENTRY_POINT      = 0x0000010E,
    SC_SEQ              = 0x0000010F,
    SC_SLICE_UD         = 0x0000011B,
    SC_FIELD_UD         = 0x0000011C,
    SC_FRAME_UD         = 0x0000011D,
    SC_ENTRY_POINT_UD   = 0x0000011E,
    SC_SEQ_UD           = 0x0000011F,
    SC_NOT_FOUND        = 0xFFFE
} startCode_e;

typedef enum {
    VC1_SIMPLE,
    VC1_MAIN,
    VC1_ADVANCED
} vc1Profile_e;

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

struct swStrmStorage;
u32 vc1hwdDecodeSequenceLayer( struct swStrmStorage *pStorage,
                               strmData_t *pStrmData );

u32 vc1hwdDecodeEntryPointLayer( struct swStrmStorage *pStorage,
                                 strmData_t *pStrmData );

u32 vc1hwdGetStartCode( strmData_t *pStrmData );

u32 vc1hwdGetUserData( struct swStrmStorage *pStorage,
                       strmData_t *pStrmData );

#endif /* #ifndef VC1HWD_HEADERS_H */

