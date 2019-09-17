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
--  Abstract : algorithm header file
--
------------------------------------------------------------------------------*/

#ifndef DECMBSETDESC_DEFINED
#define DECMBSETDESC_DEFINED

#include "basetype.h"
#include "dwl.h"
#include "mp4decapi.h"

typedef struct DecMbSetDesc_t
{
    u32 *pCtrlDataAddr; /* pointer to asic control bits */
    DWLLinearMem_t ctrlDataMem;
    u32 *pRlcDataAddr;  /* pointer to beginning of asic rlc data */
    DWLLinearMem_t rlcDataMem;
    u32 *pRlcDataCurrAddr;  /* current write address */
    u32 *pRlcDataVpAddr;    /* pointer to rlc data buffer in the
                             * beginning of current video packet */
    u32 *pMvDataAddr;   /* pointer to motion vector data */
    DWLLinearMem_t mvDataMem;

    u32 *pDcCoeffDataAddr;  /* pointer to separately coded DC coeffs */
    DWLLinearMem_t DcCoeffMem;

    u32 rlcDataBufferSize;  /* size of rlc data buffer (u32)  */
    u32 oddRlc; /* half-word left empty from last rlc */
    u32 oddRlcVp;               /* half-word left empty from last rlc */
    
    MP4DecOutput outData;   /* Return PIC info */

} DecMbSetDesc;

#endif
