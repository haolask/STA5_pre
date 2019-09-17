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
--  Abstract : Stream decoding top level functions (interface functions)
--
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Table of context

     1. Include headers
     2. External identifiers
     3. Module defines
     4. Module identifiers
     5. Fuctions

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "rv_strm.h"
#include "rv_utils.h"
#include "rv_headers.h"
#include "rv_debug.h"

/*------------------------------------------------------------------------------
    2. External identifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

enum
{
    CONTINUE
};

/*------------------------------------------------------------------------------
    4. Module indentifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

   5.1  Function name: rvStrmDec_Decode

        Purpose: Decode RV stream
        
        Input:

        Output:

------------------------------------------------------------------------------*/

u32 rv_StrmDecode(DecContainer * pDecContainer)
{

    u32 type;
    u32 status = HANTRO_OK;

    RVDEC_DEBUG(("Entry rv_StrmDecode\n"));

    /* keep decoding till something ready or something wrong */
    do
    {
        /* TODO: Where do we get type, what's going to be decoded, etc. */
        type = RV_SLICE;

        /* parse headers */
        switch (type)
        {
            case RV_SLICE:
                status = rv_DecodeSliceHeader(pDecContainer);
                if (!pDecContainer->StrmStorage.strmDecReady)
                {
                    if (status == HANTRO_OK &&
                        pDecContainer->Hdrs.horizontalSize &&
                        pDecContainer->Hdrs.verticalSize)
                    {
                        pDecContainer->StrmStorage.strmDecReady = HANTRO_TRUE;
                        return DEC_HDRS_RDY;
                    }
                    /* else try next slice or what */
                }
                else if( pDecContainer->StrmStorage.rprDetected)
                {
                    /*return DEC_HDRS_RDY;*/
                    return DEC_PIC_HDR_RDY_RPR;
                }
                else if (status == HANTRO_OK)
                    return DEC_PIC_HDR_RDY;
                else
                    return DEC_PIC_HDR_RDY_ERROR;

                break;

            default:
                break;
        }

    }
    while(0);

    return (DEC_RDY);

}
