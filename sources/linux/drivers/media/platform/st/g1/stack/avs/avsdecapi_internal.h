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

#ifndef _AVSDECAPI_INTERNAL_H_
#define _AVSDECAPI_INTERNAL_H_

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "avs_cfg.h"
#include "avs_utils.h"
#include "avsdecapi.h"
#include "regdrv.h"

/*------------------------------------------------------------------------------
    2. Internal Definitions
------------------------------------------------------------------------------*/
#define AVS_DEC_X170_MODE      9

#define AVS_DEC_X170_IRQ_DEC_RDY        DEC_8170_IRQ_RDY
#define AVS_DEC_X170_IRQ_BUS_ERROR      DEC_8170_IRQ_BUS
#define AVS_DEC_X170_IRQ_BUFFER_EMPTY   DEC_8170_IRQ_BUFFER

#define AVS_DEC_X170_IRQ_STREAM_ERROR   DEC_8170_IRQ_ERROR
#define AVS_DEC_X170_IRQ_TIMEOUT        DEC_8170_IRQ_TIMEOUT
#define AVS_DEC_X170_IRQ_CLEAR_ALL      0xFF

/*
 *  Size of internal frame buffers (in 32bit-words) per macro block
 */
#define AVSAPI_DEC_FRAME_BUFF_SIZE  96

#ifndef NULL
#define NULL 0
#endif

#define SWAP_POINTERS(A, B, T) T = A; A = B; B = T;

#define INVALID_ANCHOR_PICTURE ((u32)-1)

/*------------------------------------------------------------------------------
    3. Prototypes of Decoder API internal functions
------------------------------------------------------------------------------*/

void AvsAPI_InitDataStructures(DecContainer * pDecCont);
void AvsDecTimeCode(DecContainer * pDecCont, AvsDecTime * timeCode);
AvsDecRet AvsAllocateBuffers(DecContainer * pDecCont);
AvsDecRet AvsDecCheckSupport(DecContainer * pDecCont);
void AvsDecPreparePicReturn(DecContainer * pDecCont);
void AvsDecAspectRatio(DecContainer * pDecCont, AvsDecInfo * pDecInfo);
void AvsDecBufferPicture(DecContainer * pDecCont, u32 picId, u32 bufferB,
                           u32 isInter, AvsDecRet returnValue, u32 nbrErrMbs);
void AvsFreeBuffers(DecContainer * pDecCont);

#endif /* _AVSDECAPI_INTERNAL_H_ */
