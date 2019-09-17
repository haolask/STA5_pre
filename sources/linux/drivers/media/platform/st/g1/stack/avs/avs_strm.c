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
    1. Include headers
------------------------------------------------------------------------------*/

#include "avs_strm.h"
#include "avs_utils.h"
#include "avs_headers.h"

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

   5.1  Function name: AvsStrmDec_Decode

        Purpose: Decode AVS stream. Continues decoding until END_OF_STREAM
        encountered or whole frame decoded. Returns after decoding of sequence
        layer header.

        Input:
            Pointer to DecContainer structure
                -uses and updates StrmStorage
                -uses and updates StrmDesc
                -uses Hdrs

        Output:

------------------------------------------------------------------------------*/

u32 AvsStrmDec_Decode(DecContainer * pDecContainer)
{

    u32 status;
    u32 startCode;

    AVSDEC_DEBUG(("Entry StrmDec_Decode\n"));

    status = HANTRO_OK;

    /* keep decoding till something ready or something wrong */
    do
    {
        startCode = AvsStrmDec_NextStartCode(pDecContainer);

        /* parse headers */
        switch (startCode)
        {
        case SC_SEQUENCE:
            /* Sequence header */
            status = AvsStrmDec_DecodeSequenceHeader(pDecContainer);
            if(status != HANTRO_OK)
            {
                pDecContainer->StrmStorage.validSequence = 0;
                return (DEC_PIC_HDR_RDY_ERROR);
            }
            pDecContainer->StrmStorage.validSequence = status == HANTRO_OK;
            break;

        case SC_EXTENSION:
            /* Extension headers */
            status = AvsStrmDec_DecodeExtensionHeader(pDecContainer);
            if(status == END_OF_STREAM)
                return (DEC_END_OF_STREAM);
            break;

        case SC_I_PICTURE:
            /* Picture header */
            /* decoder still in "initialization" phase and sequence headers
             * successfully decoded -> set to normal state */
            if(pDecContainer->StrmStorage.strmDecReady == FALSE &&
               pDecContainer->StrmStorage.validSequence)
            {
                pDecContainer->StrmStorage.strmDecReady = TRUE;
                pDecContainer->StrmDesc.strmBuffReadBits -= 32;
                pDecContainer->StrmDesc.pStrmCurrPos -= 4;
                return (DEC_HDRS_RDY);
            }
            else if(pDecContainer->StrmStorage.strmDecReady)
            {
                status = AvsStrmDec_DecodeIPictureHeader(pDecContainer);
                if(status != HANTRO_OK)
                    return (DEC_PIC_HDR_RDY_ERROR);
                pDecContainer->StrmStorage.validPicHeader = 1;

            }
            break;

        case SC_PB_PICTURE:
            /* Picture header */
            status = AvsStrmDec_DecodePBPictureHeader(pDecContainer);
            if(status != HANTRO_OK)
                return (DEC_PIC_HDR_RDY_ERROR);
            pDecContainer->StrmStorage.validPicHeader = 1;

            if(pDecContainer->StrmStorage.sequenceLowDelay &&
               pDecContainer->Hdrs.picCodingType == BFRAME)
            {
                return (DEC_PIC_SUPRISE_B);
            }
            break;

        case SC_SLICE:
            /* start decoding picture data (HW) if decoder is in normal
             * decoding state and picture headers have been successfully
             * decoded */
            if (pDecContainer->StrmStorage.strmDecReady == TRUE &&
                pDecContainer->StrmStorage.validPicHeader)
            {
                /* handle stream positions and return */
                pDecContainer->StrmDesc.strmBuffReadBits -= 32;
                pDecContainer->StrmDesc.pStrmCurrPos -= 4;
                return (DEC_PIC_HDR_RDY);
            }
            break;

        case END_OF_STREAM:
            return (DEC_END_OF_STREAM);

        default:
            break;
        }

    }
    /*lint -e(506) */ while(1);

    /* execution never reaches this point (hope so) */
    /*lint -e(527) */ return (DEC_END_OF_STREAM);
    /*lint -restore */
}
