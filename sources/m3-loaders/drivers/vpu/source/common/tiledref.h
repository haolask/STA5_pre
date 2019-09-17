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
--  Abstract : Header file for stream decoding utilities
--
------------------------------------------------------------------------------*/

#ifndef TILEDREF_H_DEFINED
#define TILEDREF_H_DEFINED

#include "basetype.h"
#include "decapicommon.h"

#define TILED_REF_NONE              (0)
#define TILED_REF_8x4               (1)
#define TILED_REF_8x4_INTERLACED    (2)

u32 DecSetupTiledReference( u32 *regBase, u32 tiledModeSupport, 
                            DecDpbMode dpbMode, u32 interlacedStream );
u32 DecCheckTiledMode( u32 tiledModeSupport, DecDpbMode dpbMode,
                       u32 interlacedStream );                            

#endif /* TILEDREF_H_DEFINED */
