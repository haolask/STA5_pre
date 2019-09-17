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

#ifndef _MP4DECAPI_INTERNAL_H_
#define _MP4DECAPI_INTERNAL_H_

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "mp4deccfg.h"
#include "mp4dechwd_utils.h"
#include "mp4decapi.h"

/*------------------------------------------------------------------------------
    2. Internal Definitions
------------------------------------------------------------------------------*/
/*
 *  Size of internal frame buffers (in 32bit-words) per macro block
 */
#define MP4API_DEC_FRAME_BUFF_SIZE  96
#define BUFFER_UNDEFINED        16

/*
 *  Size of CTRL buffer (macroblks * 4 * 32bit-words/Mb), same for MV and DC
 */
#define MP4API_DEC_CTRL_BUFF_SIZE   NBR_OF_WORDS_MB * MP4API_DEC_MBS

#define MPAPI_DEC_MV_BUFF_SIZE      NBR_MV_WORDS_MB * MP4API_DEC_MBS

#define MPAPI_DEC_DC_BUFF_SIZE      NBR_DC_WORDS_MB * MP4API_DEC_MBS

#define MP4API_DEC_NBOFRLC_BUFF_SIZE MP4API_DEC_MBS * 6

#ifndef NULL
#define NULL 0
#endif

#define SWAP_POINTERS(A, B, T) T = A; A = B; B = T;

#define INVALID_ANCHOR_PICTURE ((u32)-1)

#define MP4DEC_QUANT_TABLE_SIZE (2*64)

/*------------------------------------------------------------------------------
    3. Prototypes of Decoder API internal functions
------------------------------------------------------------------------------*/

/*void regDump(MP4DecInst decInst);*/
void MP4NotCodedVop(DecContainer * pDecContainer);
void MP4API_InitDataStructures(DecContainer * pDecCont);
void MP4DecTimeCode(DecContainer * pDecCont, MP4DecTime * timeCode);
MP4DecRet MP4AllocateBuffers(DecContainer * pDecCont);
void MP4FreeBuffers(DecContainer * pDecCont);
MP4DecRet MP4AllocateRlcBuffers(DecContainer * pDecCont);
MP4DecRet MP4DecCheckSupport(DecContainer * pDecCont);
void MP4DecPixelAspectRatio(DecContainer * pDecCont, MP4DecInfo * pDecInfo);
void MP4DecBufferPicture(DecContainer *pDecCont, u32 picId,
    u32 vopType, u32 nbrErrMbs);
MP4DecRet MP4DecAllocExtraBPic(DecContainer * pDecCont);
u32 * MP4DecResolveVirtual(DecContainer * pDecCont, u32 index );
u32 MP4DecResolveBus(DecContainer * pDecCont, u32 index );
void MP4DecChangeDataIndex( DecContainer * pDecCont, u32 to, u32 from);
u32 MP4DecBFrameSupport(DecContainer * pDecCont);
void MP4SetQuantMatrix(DecContainer * pDecCont);

#endif /* _MP4DECAPI_INTERNAL_H_ */
