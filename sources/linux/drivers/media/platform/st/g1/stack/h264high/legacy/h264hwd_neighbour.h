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
--  Abstract : Get neighbour blocks
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents
   
    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef H264HWD_NEIGHBOUR_H
#define H264HWD_NEIGHBOUR_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"

#include "h264hwd_macroblock_layer.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

typedef enum {
    MB_A = 0,
    MB_B,
    MB_C,
    MB_D,
    MB_CURR,
    MB_NA = 0xFF
} neighbourMb_e;

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

typedef struct
{
    neighbourMb_e   mb;
    u8             index;
} neighbour_t;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

void h264bsdInitMbNeighbours(mbStorage_t *pMbStorage, u32 picWidth,
    u32 picSizeInMbs);

/*@null@*/ mbStorage_t* h264bsdGetNeighbourMb(mbStorage_t *pMb,
                                              neighbourMb_e neighbour);

u32 h264bsdIsNeighbourAvailable(mbStorage_t *pMb,
                                 /*@null@*/ mbStorage_t *pNeighbour);

const neighbour_t* h264bsdNeighbour4x4BlockA(u32 blockIndex);
const neighbour_t* h264bsdNeighbour4x4BlockB(u32 blockIndex);
const neighbour_t* h264bsdNeighbour4x4BlockC(u32 blockIndex);
const neighbour_t* h264bsdNeighbour4x4BlockD(u32 blockIndex);

#endif /* #ifdef H264HWD_NEIGHBOUR_H */
