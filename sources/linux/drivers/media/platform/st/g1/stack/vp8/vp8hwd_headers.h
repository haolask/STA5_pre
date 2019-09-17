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

#ifndef __VP8_HEADERS_H__
#define __VP8_HEADERS_H__

#include "basetype.h"

#include "vp8hwd_decoder.h"
#include "vp8hwd_bool.h"

void vp8hwdDecodeFrameTag( const u8 *pStrm, vp8Decoder_t* dec );
u32 vp8hwdSetPartitionOffsets( const u8 *stream, u32 len, vp8Decoder_t *dec );
u32 vp8hwdDecodeFrameHeader( const u8 *pStrm, u32 strmLen, vpBoolCoder_t*bc, 
                             vp8Decoder_t* dec );

#endif /* __VP8_BOOL_H__ */
