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
--  Abstract : Header file for short video (h263) decoding functionality
--
------------------------------------------------------------------------------*/
#ifndef STRMDEC_SHORTVIDEO_H_DEFINED
#define STRMDEC_SHORTVIDEO_H_DEFINED

#include "mp4dechwd_container.h"

u32 StrmDec_DecodeShortVideoHeader(DecContainer * pDecContainer);
u32 StrmDec_DecodeSorensonHeader(DecContainer * pDecContainer);
u32 StrmDec_DecodeGobLayer(DecContainer *);
u32 StrmDec_DecodeShortVideo(DecContainer *);
u32 StrmDec_CheckNextGobNumber(DecContainer *);
u32 StrmDec_DecodeSorensonSparkHeader(DecContainer * pDecContainer);

#endif
