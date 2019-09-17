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

#ifndef _MPEG2DECAPI_INTERNAL_H_
#define _MPEG2DECAPI_INTERNAL_H_

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "mpeg2hwd_cfg.h"
#include "mpeg2hwd_utils.h"
#include "mpeg2decapi.h"
#include "regdrv.h"
/*------------------------------------------------------------------------------
    2. Internal Definitions
------------------------------------------------------------------------------*/
#define MPEG2_DEC_X170_MODE_MPEG2      5
#define MPEG2_DEC_X170_MODE_MPEG1      6

#define MPEG2_DEC_X170_IRQ_DEC_RDY          DEC_8190_IRQ_RDY
#define MPEG2_DEC_X170_IRQ_BUS_ERROR        DEC_8190_IRQ_BUS
#define MPEG2_DEC_X170_IRQ_BUFFER_EMPTY     DEC_8190_IRQ_BUFFER
#define MPEG2_DEC_X170_IRQ_ASO              DEC_8190_IRQ_ASO
#define MPEG2_DEC_X170_IRQ_STREAM_ERROR     DEC_8190_IRQ_ERROR
#define MPEG2_DEC_X170_IRQ_TIMEOUT          DEC_8190_IRQ_TIMEOUT
#define MPEG2_DEC_X170_IRQ_CLEAR_ALL        0xFF

/*
 *  Size of internal frame buffers (in 32bit-words) per macro block
 */
#define MPEG2API_DEC_FRAME_BUFF_SIZE  96

/*
 *  Size of CTRL buffer (macroblks * 4 * 32bit-words/Mb), same for MV and DC
 */
#define MPEG2API_DEC_CTRL_BUFF_SIZE   NBR_OF_WORDS_MB * MPEG2API_DEC_MBS

#define MPEG2API_DEC_MV_BUFF_SIZE     NBR_MV_WORDS_MB * MPEG2API_DEC_MBS

#define MPEG2API_DEC_DC_BUFF_SIZE     NBR_DC_WORDS_MB * MPEG2API_DEC_MBS

#define MPEG2API_DEC_NBOFRLC_BUFF_SIZE MPEG2API_DEC_MBS * 6

#ifndef NULL
#define NULL 0
#endif

#define SWAP_POINTERS(A, B, T) T = A; A = B; B = T;

#define INVALID_ANCHOR_PICTURE ((u32)-1)

/*------------------------------------------------------------------------------
    3. Prototypes of Decoder API internal functions
------------------------------------------------------------------------------*/

/*void regDump(Mpeg2DecInst decInst);*/
void mpeg2API_InitDataStructures(DecContainer * pDecCont);
void mpeg2DecTimeCode(DecContainer * pDecCont, Mpeg2DecTime * timeCode);
Mpeg2DecRet mpeg2AllocateBuffers(DecContainer * pDecCont);
void mpeg2HandleQTables(DecContainer * pDecCont);
void mpeg2HandleMpeg1Parameters(DecContainer * pDecCont);
Mpeg2DecRet mpeg2DecCheckSupport(DecContainer * pDecCont);
void mpeg2DecPreparePicReturn(DecContainer * pDecCont);
void mpeg2DecAspectRatio(DecContainer * pDecCont, Mpeg2DecInfo * pDecInfo);
void mpeg2DecBufferPicture(DecContainer * pDecCont, u32 picId, u32 bufferB,
                           u32 isInter, Mpeg2DecRet returnValue, u32 nbrErrMbs);
Mpeg2DecRet mpeg2DecAllocExtraBPic(DecContainer * pDecCont);
void mpeg2FreeBuffers(DecContainer * pDecCont);

#endif /* _MPEG2DECAPI_INTERNAL_H_ */
