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

#ifndef STRMDEC_VLC_H
#define STRMDEC_VLC_H

#include "basetype.h"
#include "mp4dechwd_container.h"

/* constant definitions */

/* function prototypes */

u32 StrmDec_DecodeMcbpc(DecContainer *, u32, u32, u32 *);
u32 StrmDec_DecodeCbpy(DecContainer *, u32, u32, u32 *);
u32 mp4dechwd_vlcDec(DecContainer * pDecContainer, u32 mbNum, u32 blockNum);
u32 StrmDec_DecodeDcCoeff(DecContainer * pDecContainer, u32 mbNum, u32 blockNum,
              i32 * dcCoeff);
u32 StrmDec_DecodeMv(DecContainer * pDecContainer, u32 mbNum);
u32 StrmDec_DecodeVlcBlock(DecContainer * pDecContainer, u32 mbNum,
    u32 blockNum);


#endif
