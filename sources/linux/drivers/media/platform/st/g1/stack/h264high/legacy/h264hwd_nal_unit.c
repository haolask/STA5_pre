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
     2. External compiler flags
     3. Module defines
     4. Local function prototypes
     5. Functions
          h264bsdDecodeNalUnit

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "h264hwd_nal_unit.h"
#include "h264hwd_util.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: h264bsdDecodeNalUnit

        Functional description:
            Decode NAL unit header information

        Inputs:
            pStrmData       pointer to stream data structure

        Outputs:
            pNalUnit        NAL unit header information is stored here

        Returns:
            HANTRO_OK       success
            HANTRO_NOK      invalid NAL unit header information

------------------------------------------------------------------------------*/

u32 h264bsdDecodeNalUnit(strmData_t *pStrmData, nalUnit_t *pNalUnit)
{

/* Variables */

    u32 tmp;

/* Code */

    ASSERT(pStrmData);
    ASSERT(pNalUnit);
    ASSERT(pStrmData->bitPosInWord == 0);

    (void)DWLmemset(pNalUnit, 0, sizeof(nalUnit_t));

    /* forbidden_zero_bit (not checked to be zero, errors ignored) */
    tmp = h264bsdGetBits(pStrmData, 1);
    /* Assuming that NAL unit starts from byte boundary => don't have to check
     * following 7 bits for END_OF_STREAM */
    if (tmp == END_OF_STREAM)
        return(HANTRO_NOK);

    tmp = h264bsdGetBits(pStrmData, 2);
    pNalUnit->nalRefIdc = tmp;

    tmp = h264bsdGetBits(pStrmData, 5);
    pNalUnit->nalUnitType = (nalUnitType_e)tmp;

    DEBUG_PRINT(("NAL TYPE %d\n", tmp));

    /* data partitioning NAL units not supported */
    if ( (tmp == NAL_CODED_SLICE_DP_A) ||
         (tmp == NAL_CODED_SLICE_DP_B) ||
         (tmp == NAL_CODED_SLICE_DP_C) )
    {
        ERROR_PRINT("DP slices not allowed!!!");
        return(HANTRO_NOK);
    }

    /* nal_ref_idc shall not be zero for these nal_unit_types */
    if ( ( (tmp == NAL_SEQ_PARAM_SET) || (tmp == NAL_PIC_PARAM_SET) ||
           (tmp == NAL_CODED_SLICE_IDR) ) && (pNalUnit->nalRefIdc == 0) )
    {
        ERROR_PRINT("nal_ref_idc shall not be zero!!!");
        return(HANTRO_NOK);
    }
    /* nal_ref_idc shall be zero for these nal_unit_types */
    else if ( ( (tmp == NAL_SEI) || (tmp == NAL_ACCESS_UNIT_DELIMITER) ||
                (tmp == NAL_END_OF_SEQUENCE) || (tmp == NAL_END_OF_STREAM) ||
                (tmp == NAL_FILLER_DATA) ) && (pNalUnit->nalRefIdc != 0) )
    {
        ERROR_PRINT("nal_ref_idc shall be zero!!!");
        return(HANTRO_NOK);
    }

    if (pNalUnit->nalUnitType == NAL_PREFIX ||
        pNalUnit->nalUnitType == NAL_CODED_SLICE_EXT)
    {
        tmp = h264bsdGetBits(pStrmData, 1);

        if (tmp == END_OF_STREAM)
            return(HANTRO_NOK);

        pNalUnit->svcExtensionFlag = tmp;

        if(tmp == 0)
        {
            /* MVC Annex H*/
            tmp = h264bsdGetBits(pStrmData, 1);
            pNalUnit->nonIdrFlag = tmp;
            tmp = h264bsdGetBits(pStrmData, 6);
            pNalUnit->priorityId = tmp;
            tmp = h264bsdGetBits(pStrmData, 10);
            pNalUnit->viewId = tmp;
            tmp = h264bsdGetBits(pStrmData, 3);
            pNalUnit->temporalId = tmp;
            tmp = h264bsdGetBits(pStrmData, 1);
            pNalUnit->anchorPicFlag = tmp;
            tmp = h264bsdGetBits(pStrmData, 1);
            pNalUnit->interViewFlag = tmp;
            /* reserved_one_bit */
            tmp = h264bsdGetBits(pStrmData, 1);
        }
        else
        {
            /* SVC Annex G*/
            tmp = h264bsdGetBits(pStrmData, 1); /* idr_flag */
            tmp = h264bsdGetBits(pStrmData, 6); /* priority_id */
            tmp = h264bsdGetBits(pStrmData, 1); /* no_inter_layer_pred_flag */
            tmp = h264bsdGetBits(pStrmData, 3); /* dependency_id */
            tmp = h264bsdGetBits(pStrmData, 4); /* quality_id */
            tmp = h264bsdGetBits(pStrmData, 3); /* temporal_id */
            tmp = h264bsdGetBits(pStrmData, 1); /* use_ref_base_pic_flag */
            tmp = h264bsdGetBits(pStrmData, 1); /* discardable_flag */
            tmp = h264bsdGetBits(pStrmData, 1); /* output_flag */
            tmp = h264bsdGetBits(pStrmData, 2); /* reserved_three_2_bits */
        }

        if (tmp == END_OF_STREAM)
            return(HANTRO_NOK);
    }

    return(HANTRO_OK);

}
