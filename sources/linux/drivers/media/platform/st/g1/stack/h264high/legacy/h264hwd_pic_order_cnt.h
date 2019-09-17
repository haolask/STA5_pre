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
--  Abstract : Compute Picture Order Count (POC) for a picture
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents
   
    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef H264HWD_PIC_ORDER_CNT_H
#define H264HWD_PIC_ORDER_CNT_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "h264hwd_seq_param_set.h"
#include "h264hwd_slice_header.h"
#include "h264hwd_nal_unit.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

/* structure to store information computed for previous picture, needed for
 * POC computation of a picture. Two first fields for POC type 0, last two
 * for types 1 and 2 */
typedef struct
{
    u32 prevPicOrderCntLsb;
    i32 prevPicOrderCntMsb;
    u32 prevFrameNum;
    u32 prevFrameNumOffset;
    u32 containsMmco5;
    i32 picOrderCnt[2];
} pocStorage_t;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

void h264bsdDecodePicOrderCnt(pocStorage_t *poc, const seqParamSet_t *sps,
     const sliceHeader_t *sliceHeader, const nalUnit_t *pNalUnit);

#endif /* #ifdef H264HWD_PIC_ORDER_CNT_H */
