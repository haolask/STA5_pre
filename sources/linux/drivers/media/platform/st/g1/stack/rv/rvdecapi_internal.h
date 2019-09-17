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

#ifndef RV_DECAPI_INTERNAL_H
#define RV_DECAPI_INTERNAL_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "rv_cfg.h"
#include "rv_utils.h"
#include "rvdecapi.h"
#include "regdrv.h"

/*------------------------------------------------------------------------------
    2. Internal Definitions
------------------------------------------------------------------------------*/

#define RV_DEC_X170_IRQ_DEC_RDY         DEC_8190_IRQ_RDY
#define RV_DEC_X170_IRQ_BUS_ERROR       DEC_8190_IRQ_BUS
#define RV_DEC_X170_IRQ_BUFFER_EMPTY    DEC_8190_IRQ_BUFFER
#define RV_DEC_X170_IRQ_STREAM_ERROR    DEC_8190_IRQ_ERROR
#define RV_DEC_X170_IRQ_TIMEOUT         DEC_8190_IRQ_TIMEOUT
#define RV_DEC_X170_IRQ_ABORT           DEC_8190_IRQ_ABORT
#define RV_DEC_X170_IRQ_CLEAR_ALL       0xFF

#define RV_DEC_X170_MAX_NUM_SLICES  (128)

/*
 *  Size of internal frame buffers (in 32bit-words) per macro block
 */
#define RVAPI_DEC_FRAME_BUFF_SIZE  96

#ifndef NULL
#define NULL 0
#endif

#define SWAP_POINTERS(A, B, T) T = A; A = B; B = T;

#define INVALID_ANCHOR_PICTURE ((u32)-1)

/*------------------------------------------------------------------------------
    3. Prototypes of Decoder API internal functions
------------------------------------------------------------------------------*/

void rvAPI_InitDataStructures(DecContainer * pDecCont);
RvDecRet rvAllocateBuffers(DecContainer * pDecCont);
RvDecRet rvDecCheckSupport(DecContainer * pDecCont);
void rvDecPreparePicReturn(DecContainer * pDecCont);
void rvDecAspectRatio(DecContainer * pDecCont, RvDecInfo * pDecInfo);
void rvDecBufferPicture(DecContainer * pDecCont, u32 picId, u32 bufferB,
                           u32 isInter, RvDecRet returnValue, u32 nbrErrMbs);
void rvFreeBuffers(DecContainer * pDecCont);
void rvInitVlcTables(DecContainer * pDecCont);
RvDecRet rvAllocateRprBuffer(DecContainer * pDecCont);

#endif /* RV_DECAPI_INTERNAL_H */
