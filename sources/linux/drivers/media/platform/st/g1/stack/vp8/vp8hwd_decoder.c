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
-
-  Description : ...
-
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Includes
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "vp8hwd_headers.h"
#include "vp8hwd_probs.h"
#include "vp8hwd_container.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
    Functions
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
    ResetScan

        Reset decoder to initial state
------------------------------------------------------------------------------*/
void vp8hwdPrepareVp7Scan( vp8Decoder_t * dec, u32 * newOrder )
{
    u32 i;
    static const u32 Vp7DefaultScan[] = {
        0,  1,  4,  8,  5,  2,  3,  6,
        9, 12, 13, 10,  7, 11, 14, 15,
    };

    if(!newOrder)
    {
        for( i = 0 ; i < 16 ; ++i )
            dec->vp7ScanOrder[i] = Vp7DefaultScan[i];
    }
    else
    {
        for( i = 0 ; i < 16 ; ++i )
            dec->vp7ScanOrder[i] = Vp7DefaultScan[newOrder[i]];
    }
}

/*------------------------------------------------------------------------------
    vp8hwdResetDecoder

        Reset decoder to initial state
------------------------------------------------------------------------------*/
void vp8hwdResetDecoder( vp8Decoder_t * dec)
{
    vp8hwdResetProbs( dec );
    vp8hwdPrepareVp7Scan( dec, NULL );
}

/*------------------------------------------------------------------------------
    vp8hwdResetDecoder

        Reset decoder to initial state
------------------------------------------------------------------------------*/
void vp8hwdResetSegMap( vp8Decoder_t * dec, DecAsicBuffers_t *asicBuff, u32 coreID)
{
    UNUSED(dec);
    if (asicBuff->segmentMap[coreID].virtualAddress)
    {
        DWLmemset(asicBuff->segmentMap[coreID].virtualAddress,
                  0, asicBuff->segmentMapSize);
    }
}
