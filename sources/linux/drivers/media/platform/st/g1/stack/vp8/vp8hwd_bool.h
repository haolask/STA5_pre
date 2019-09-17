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
-
-  Description : ...
-
------------------------------------------------------------------------------*/

#ifndef __VP8_BOOL_H__
#define __VP8_BOOL_H__

#include "basetype.h"

#define END_OF_STREAM   (0xFFFFFFFF)
#define CHECK_END_OF_STREAM(s) if((s)==END_OF_STREAM) return (s)

typedef struct
{
    u32 lowvalue;
    u32 range;
    u32 value;
    i32 count;
    u32 pos;
    const u8 *buffer;
    u32 BitCounter;
    u32 streamEndPos;
    u32 strmError;
} vpBoolCoder_t;

extern void vp8hwdBoolStart(vpBoolCoder_t * bc, const u8 *buffer, u32 len);
extern u32 vp8hwdDecodeBool(vpBoolCoder_t * bc, i32 probability);
extern u32 vp8hwdDecodeBool128(vpBoolCoder_t * bc);
extern void vp8hwdBoolStop(vpBoolCoder_t * bc);

u32 vp8hwdReadBits ( vpBoolCoder_t *br, i32 bits );

#endif /* __VP8_BOOL_H__ */
