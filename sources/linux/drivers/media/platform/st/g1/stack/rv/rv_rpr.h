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
--  Abstract: api internal defines
--
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Table of contents

    1. Include headers
    2. Internal Definitions
    3. Prototypes of Decoder API internal functions

------------------------------------------------------------------------------*/

#ifndef RV_RPR_H
#define RV_RPR_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "rv_utils.h"

/*------------------------------------------------------------------------------
    2. Internal Definitions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Prototypes of functions
------------------------------------------------------------------------------*/

void rvRpr( picture_t *pSrc,
               picture_t *pDst,
               DWLLinearMem_t *rprWorkBuffer,
               u32 round,
               u32 newCodedWidth, u32 newCodedHeight,
               u32 tiledMode );

#endif /* RV_RPR_H */
