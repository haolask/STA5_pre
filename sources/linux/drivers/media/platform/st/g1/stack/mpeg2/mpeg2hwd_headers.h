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
/*------------------------------------------------------------------------------

    Table of context 

     1. xxx...
   
 
------------------------------------------------------------------------------*/

#ifndef MPEG2STRMDEC_DECODEHEADERS_H
#define MPEG2STRMDEC_DECODEHEADERS_H

#include "basetype.h"
#include "mpeg2hwd_container.h"

u32 mpeg2StrmDec_DecodeExtensionHeader(DecContainer * pDecContainer);
u32 mpeg2StrmDec_DecodeSequenceHeader(DecContainer * pDecContainer);
u32 mpeg2StrmDec_DecodeGOPHeader(DecContainer * pDecContainer);
u32 mpeg2StrmDec_DecodePictureHeader(DecContainer * pDecContainer);

u32 mpeg2StrmDec_DecodeSeqExtHeader(DecContainer * pDecContainer);
u32 mpeg2StrmDec_DecodeSeqDisplayExtHeader(DecContainer * pDecContainer);
u32 mpeg2StrmDec_DecodeQMatrixExtHeader(DecContainer * pDecContainer);
u32 mpeg2StrmDec_DecodePicCodingExtHeader(DecContainer * pDecContainer);
u32 mpeg2StrmDec_DecodePicDisplayExtHeader(DecContainer * pDecContainer);

#endif /* #ifndef MPEG2STRMDEC_DECODEHEADERS_H */
