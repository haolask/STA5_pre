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
--  Abstract : Utility macros and functions
--
------------------------------------------------------------------------------*/

#ifndef H264BSDDEC_UTIL_H
#define H264BSDDEC_UTIL_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "dwl.h"

#include "h264hwd_stream.h"
#include "h264hwd_debug.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

#define HANTRO_OK   0
#define HANTRO_NOK  1

#define HANTRO_FALSE   (0U)
#define HANTRO_TRUE    (1U)

#define MEMORY_ALLOCATION_ERROR     0xFFFF
#define PARAM_SET_ERROR             0xFFF0

/* value to be returned by GetBits if stream buffer is empty */
#define END_OF_STREAM               0xFFFFFFFFU

#define EMPTY_RESIDUAL_INDICATOR    0xFFFFFF

/* macro to mark a residual block empty, i.e. contain zero coefficients */
#define MARK_RESIDUAL_EMPTY(residual) ((residual)[0] = EMPTY_RESIDUAL_INDICATOR)
/* macro to check if residual block is empty */
#define IS_RESIDUAL_EMPTY(residual) ((residual)[0] == EMPTY_RESIDUAL_INDICATOR)

/* macro to get smaller of two values */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* macro to get greater of two values */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/* macro to get absolute value */
#define ABS(a) (((a) < 0) ? -(a) : (a))

/* macro to clip a value z, so that x <= z =< y */
#define CLIP3(x,y,z) (((z) < (x)) ? (x) : (((z) > (y)) ? (y) : (z)))

/* macro to clip a value z, so that 0 <= z =< 255 */
#define CLIP1(z) (((z) < 0) ? 0 : (((z) > 255) ? 255 : (z)))

/* macro to allocate memory */
#define ALLOCATE(ptr, count, type) \
{ \
    ptr = DWLmalloc((count) * sizeof(type)); \
}

/* macro to free allocated memory */
#define FREE(ptr) \
{ \
    if(ptr != NULL) {DWLfree(ptr); ptr = NULL;}\
}

extern const u32 h264bsdQpC[52];

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

typedef enum
{
    TOPFIELD = 0,
    BOTFIELD = 1,
    FRAME    = 2
} picStruct_e;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

u32 h264bsdCountLeadingZeros(u32 value, u32 length);

u32 h264bsdRbspTrailingBits(strmData_t * strmData);

u32 h264bsdMoreRbspData(strmData_t * strmData);

u32 h264bsdNextMbAddress(u32 * pSliceGroupMap, u32 picSizeInMbs,
                         u32 currMbAddr);

u32 h264CheckCabacZeroWords( strmData_t *strmData );

#endif /* #ifdef H264BSDDEC_UTIL_H */
