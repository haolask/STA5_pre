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
        5.1     StrmDec_DecoderInit
        5.2     StrmDec_Decode
        5.3     StrmDec_PrepareConcealment

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "mp4dechwd_strmdec.h"
#include "mp4dechwd_utils.h"
#include "mp4dechwd_headers.h"
#include "mp4dechwd_generic.h"
#include "mp4debug.h"
#include "mp4decdrv.h"

/*------------------------------------------------------------------------------
    2. External identifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Module indentifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

   5.1  Function name: StrmDec_DecodeSusi3Header

        Purpose: initialize stream decoding related parts of DecContainer

        Input:
            Pointer to DecContainer structure
                -initializes StrmStorage

        Output:
            HANTRO_OK/NOK

------------------------------------------------------------------------------*/
u32 StrmDec_DecodeCustomHeaders(DecContainer * pDecContainer)
{
    UNUSED(pDecContainer);
    return DEC_VOP_HDR_RDY_ERROR;
}

/*------------------------------------------------------------------------------

   Function name:
                ProcessUserData

        Purpose:

        Input:
                pointer to DecContainer

        Output:
                status (HANTRO_OK/NOK/END_OF_STREAM)

------------------------------------------------------------------------------*/

void ProcessUserData(DecContainer * pDecContainer)
{

    u32 tmp;
    u32 SusI = 1147762264;
    tmp = StrmDec_ShowBits(pDecContainer, 32);
    if(tmp == SusI)
    {
        pDecContainer->StrmStorage.unsupportedFeaturesPresent = HANTRO_TRUE;
    }

}


/*------------------------------------------------------------------------------

    Function: SetConformanceFlags

        Functional description:
            Set some flags to get best conformance to different Susi versions.

        Inputs:
            none

        Outputs:
            none

        Returns:
            API version

------------------------------------------------------------------------------*/

void SetConformanceFlags( DecContainer * pDecCont )
{

/* Variables */

/* Code */

    if( pDecCont->StrmStorage.customStrmVer )
        pDecCont->StrmStorage.unsupportedFeaturesPresent =
            HANTRO_TRUE;

}



/*------------------------------------------------------------------------------

    Function: ProcessHwOutput

        Functional description:
            Read flip-flop rounding type for Susi3

        Inputs:
            none

        Outputs:
            none

        Returns:
            API version

------------------------------------------------------------------------------*/

void ProcessHwOutput( DecContainer * pDecCont )
{
    /* Nothing */
    UNUSED(pDecCont);
}

/*------------------------------------------------------------------------------

    Function: Susi3Init

        Functional description:

        Inputs:
            none

        Outputs:
            none

        Returns:
            API version

------------------------------------------------------------------------------*/

void SetCustomInfo(DecContainer * pDecCont, u32 width, u32 height ) 
{

    u32 mbWidth, mbHeight;

    mbWidth = (width+15)>>4;
    mbHeight = (height+15)>>4;

    pDecCont->VopDesc.totalMbInVop = mbWidth * mbHeight;
    pDecCont->VopDesc.vopWidth = mbWidth;
    pDecCont->VopDesc.vopHeight = mbHeight;

    pDecCont->StrmStorage.customStrmVer = CUSTOM_STRM_0;

    SetConformanceFlags( pDecCont );

}


/*------------------------------------------------------------------------------

    Function: SetConformanceRegs

        Functional description:

        Inputs:
            none

        Outputs:
            none

        Returns:
            API version

------------------------------------------------------------------------------*/

void SetConformanceRegs( DecContainer * pDecContainer )
{

    /* Set correct transform */
    SetDecRegister(pDecContainer->mp4Regs, HWIF_DIVX_IDCT_E, 0 );

}


/*------------------------------------------------------------------------------

    Function: SetStrmFmt

        Functional description:

        Inputs:
            none

        Outputs:
            none

        Returns:
            API version

------------------------------------------------------------------------------*/
void SetStrmFmt( DecContainer * pDecCont, u32 strmFmt )
{
    switch(strmFmt)
    {
        case MP4DEC_MPEG4:
            /* nothing special here */
            break;
        case MP4DEC_SORENSON:
            pDecCont->StrmStorage.sorensonSpark = 1;
            break;
        default:
            pDecCont->StrmStorage.unsupportedFeaturesPresent = HANTRO_TRUE;
            break;
    }
}

 
