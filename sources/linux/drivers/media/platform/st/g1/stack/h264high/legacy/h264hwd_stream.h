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
--  Abstract : Stream buffer handling
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents

    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef H264HWD_STREAM_H
#define H264HWD_STREAM_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

typedef struct
{
    const u8 *pStrmBuffStart;   /* pointer to start of stream buffer */
    const u8 *pStrmCurrPos;  /* current read address in stream buffer */
    u32 bitPosInWord;        /* bit position in stream buffer byte */
    u32 strmBuffSize;        /* size of stream buffer (bytes) */
    u32 strmBuffReadBits;    /* number of bits read from stream buffer */
    u32 removeEmul3Byte;     /* signal the pre-removal of emulation prevention 3 bytes */
    u32 emulByteCount;       /* counter incremented for each removed byte */
} strmData_t;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

u32 h264bsdGetBits(strmData_t * pStrmData, u32 numBits);

u32 h264bsdShowBits(strmData_t * pStrmData, u32 numBits);

u32 h264bsdFlushBits(strmData_t * pStrmData, u32 numBits);

u32 h264bsdIsByteAligned(strmData_t *);

#endif /* #ifdef H264HWD_STREAM_H */
