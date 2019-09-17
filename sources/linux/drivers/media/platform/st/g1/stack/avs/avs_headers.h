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
--  Abstract :
--
------------------------------------------------------------------------------*/

#ifndef AVSSTRMDEC_DECODEHEADERS_H
#define AVSSTRMDEC_DECODEHEADERS_H

#include "basetype.h"
#include "avs_container.h"

u32 AvsStrmDec_DecodeExtensionHeader(DecContainer * pDecContainer);
u32 AvsStrmDec_DecodeSequenceHeader(DecContainer * pDecContainer);
u32 AvsStrmDec_DecodeIPictureHeader(DecContainer * pDecContainer);
u32 AvsStrmDec_DecodePBPictureHeader(DecContainer * pDecContainer);

u32 AvsStrmDec_DecodeSeqDisplayExtHeader(DecContainer * pDecContainer);

#endif /* #ifndef AVSSTRMDEC_DECODEHEADERS_H */
