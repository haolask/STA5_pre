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
--  Abstract : Header file for stream decoding utilities
--
------------------------------------------------------------------------------*/

#ifndef BQUEUE_H_DEFINED
#define BQUEUE_H_DEFINED

#include "basetype.h"

typedef struct
{
    u32 *picI;
    u32 ctr;
    u32 queueSize;
    u32 prevAnchorSlot;
} bufferQueue_t;

#define BQUEUE_UNUSED (u32)(0xffffffff)

u32  BqueueInit( bufferQueue_t *bq, u32 numBuffers );
void BqueueRelease( bufferQueue_t *bq );
u32  BqueueNext( bufferQueue_t *bq, u32 ref0, u32 ref1, u32 ref2, u32 bPic );
void BqueueDiscard( bufferQueue_t *bq, u32 buffer );

#endif /* BQUEUE_H_DEFINED */
