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
--  Abstract : Decode NAL unit header
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents
   
    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef H264HWD_NAL_UNIT_H
#define H264HWD_NAL_UNIT_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "h264hwd_stream.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

/* macro to determine if NAL unit pointed by pNalUnit contains an IDR slice */
#define IS_IDR_NAL_UNIT(pNalUnit) \
    ((pNalUnit)->nalUnitType == NAL_CODED_SLICE_IDR || \
     ((pNalUnit)->nalUnitType == NAL_CODED_SLICE_EXT && \
      (pNalUnit)->nonIdrFlag == 0))

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

typedef enum
{
    NAL_UNSPECIFIED = 0,
    NAL_CODED_SLICE = 1,
    NAL_CODED_SLICE_DP_A = 2,
    NAL_CODED_SLICE_DP_B = 3,
    NAL_CODED_SLICE_DP_C = 4,
    NAL_CODED_SLICE_IDR = 5,
    NAL_SEI = 6,
    NAL_SEQ_PARAM_SET = 7,
    NAL_PIC_PARAM_SET = 8,
    NAL_ACCESS_UNIT_DELIMITER = 9,
    NAL_END_OF_SEQUENCE = 10,
    NAL_END_OF_STREAM = 11,
    NAL_FILLER_DATA = 12,
    NAL_SPS_EXT = 13,
    NAL_PREFIX = 14,
    NAL_SUBSET_SEQ_PARAM_SET = 15,
    NAL_CODED_SLICE_AUX = 19,
    NAL_CODED_SLICE_EXT = 20,
    NAL_MAX_TYPE_VALUE = 31
} nalUnitType_e;

typedef struct
{
    nalUnitType_e nalUnitType;
    u32 nalRefIdc;
    u32 svcExtensionFlag;
    u32 nonIdrFlag;
    u32 priorityId;
    u32 viewId;
    u32 temporalId;
    u32 anchorPicFlag;
    u32 interViewFlag;
} nalUnit_t;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

u32 h264bsdDecodeNalUnit(strmData_t * pStrmData, nalUnit_t * pNalUnit);

#endif /* #ifdef H264HWD_NAL_UNIT_H */
