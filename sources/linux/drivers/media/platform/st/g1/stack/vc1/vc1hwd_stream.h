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
--  Description : Stream buffer handling
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_STREAM_H
#define VC1HWD_STREAM_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

typedef struct
{
    u8  *pStrmBuffStart;    /* pointer to start of stream buffer */
    u8  *pStrmCurrPos;      /* current read address in stream buffer */
    u32  bitPosInWord;      /* bit position in stream buffer byte */
    u32  strmBuffSize;      /* size of stream buffer (bytes) */
    u32  strmBuffReadBits;  /* number of bits read from stream buffer */
    u32  strmExhausted;     /* attempted to read more bits from the stream
                             * than available. */
    u32  removeEmulPrevBytes;
    u32  slice_piclayer_emulation_bits;   /*fix slice picture layer length error when emulation  */
} strmData_t;

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

u32 vc1hwdGetBits(strmData_t *pStrmData, u32 numBits);

u32 vc1hwdShowBits(strmData_t *pStrmData, u32 numBits );

u32 vc1hwdFlushBits(strmData_t *pStrmData, u32 numBits);

u32 vc1hwdIsExhausted(const strmData_t * const);

#endif /* #ifndef VC1HWD_STREAM_H */

