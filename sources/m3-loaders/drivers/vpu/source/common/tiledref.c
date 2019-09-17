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
--  Abstract : Stream decoding utilities
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

#include "tiledref.h"
#include "regdrv.h"

/*------------------------------------------------------------------------------
    2. External identifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*#define TILED_NOT_SUPPORTED             (0)*/
#define TILED_SUPPORT_PROGRESSIVE_8x4   (1)
#define TILED_SUPPORT_INTERLACED_8x4    (2)

/*------------------------------------------------------------------------------
    4. Module indentifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    DecCheckTiledMode
        Check if suggested tile-mode / DPB combination is valid.

        Returns OK if valid, otherwise NOK.
------------------------------------------------------------------------------*/
u32 DecCheckTiledMode( u32 tiledModeSupport, DecDpbMode dpbMode,
                       u32 interlacedStream )
{

    return 0;
    
    if(interlacedStream)
    {
        if( (tiledModeSupport != TILED_SUPPORT_INTERLACED_8x4) ||
            (dpbMode != DEC_DPB_INTERLACED_FIELD ))
            return 1;
    }
    else /* progressive */
    {
        if( dpbMode != DEC_DPB_FRAME )
            return 1;
    }
    
    return 0;
}

/*------------------------------------------------------------------------------
    DecSetupTiledReference
        Enable/disable tiled reference mode on HW. See inside function for
        disable criterias.

        Returns tile mode.
------------------------------------------------------------------------------*/
u32 DecSetupTiledReference( u32 *regBase, u32 tiledModeSupport, 
                            DecDpbMode dpbMode, u32 interlacedStream )
{
    u32 tiledAllowed = 1;
    u32 mode = TILED_REF_NONE;

    if(!tiledModeSupport)
    {
        SetDecRegister(regBase, HWIF_TILED_MODE_MSB, 0 );
        SetDecRegister(regBase, HWIF_TILED_MODE_LSB, 0 );
        return TILED_REF_NONE;
    }

    /* disallow for interlaced streams if no support*/
    if(interlacedStream && 
        (dpbMode != DEC_DPB_INTERLACED_FIELD) )
        tiledAllowed = 0; 

    /* if tiled mode allowed, pick a tile mode that suits us best (pretty easy
     * as we currently only support 8x4 */
    if(tiledAllowed)
    {
        mode = TILED_REF_8x4;
    }

    /* Setup HW registers */
    SetDecRegister(regBase, HWIF_TILED_MODE_MSB, (mode >> 1) & 0x1 );
    SetDecRegister(regBase, HWIF_TILED_MODE_LSB, mode & 0x1 );
    
    return mode;
}


