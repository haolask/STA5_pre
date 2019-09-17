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

#ifndef __VP8_ERROR_H__
#define __VP8_ERROR_H__

#include "basetype.h"
#include "vp8hwd_decoder.h"

typedef struct
{
    i32 hor;
    i32 ver;
} mv_t;

typedef struct
{
    u32 totWeight[3]; /* last, golden, alt */
    mv_t totMv[3];
} ecMv_t;

typedef struct vp8ec_t
{
    ecMv_t *mvs;
    u32 width;
    u32 height;
    u32 numMvsPerMb;
} vp8ec_t;

u32 vp8hwdInitEc(vp8ec_t *ec, u32 w, u32 h, u32 mvsPerMb);
void vp8hwdReleaseEc(vp8ec_t *ec);
void vp8hwdEc(vp8ec_t *ec, u32 *pRef, u32 *pOut, u32 startMb, u32 all);

#endif /* __VP8_ERROR_H__ */
