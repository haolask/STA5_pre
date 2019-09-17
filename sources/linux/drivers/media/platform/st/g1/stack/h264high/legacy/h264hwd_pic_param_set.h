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
--  Abstract : Decode picture parameter set information from the stream
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents
   
    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef H264HWD_PIC_PARAM_SET_H
#define H264HWD_PIC_PARAM_SET_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"

#include "h264hwd_stream.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

/* data structure to store PPS information decoded from the stream */
typedef struct
{
    u32 picParameterSetId;
    u32 seqParameterSetId;
    u32 picOrderPresentFlag;
    u32 numSliceGroups;
    u32 sliceGroupMapType;
    u32 *runLength;
    u32 *topLeft;
    u32 *bottomRight;
    u32 sliceGroupChangeDirectionFlag;
    u32 sliceGroupChangeRate;
    u32 picSizeInMapUnits;
    u32 *sliceGroupId;
    u32 numRefIdxL0Active;
    u32 numRefIdxL1Active;
    u32 picInitQp;
    i32 chromaQpIndexOffset;
    i32 chromaQpIndexOffset2;
    u32 deblockingFilterControlPresentFlag;
    u32 constrainedIntraPredFlag;
    u32 redundantPicCntPresentFlag;
    u32 entropyCodingModeFlag;
    u32 weightedPredFlag;
    u32 weightedBiPredIdc;
    u32 transform8x8Flag;
    u32 scalingMatrixPresentFlag;
    u32 scalingListPresent[8];
    u32 useDefaultScaling[8];
    u8 scalingList[8][64];
} picParamSet_t;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

u32 h264bsdDecodePicParamSet(strmData_t *pStrmData,
    picParamSet_t *pPicParamSet);

#endif /* #ifdef H264HWD_PIC_PARAM_SET_H */
