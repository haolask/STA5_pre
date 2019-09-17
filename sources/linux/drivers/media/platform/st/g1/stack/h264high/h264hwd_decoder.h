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
--  Abstract : Top level control of the decoder
--
------------------------------------------------------------------------------*/
#ifndef H264HWD_DECODER_H
#define H264HWD_DECODER_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "h264decapi.h"

#include "h264hwd_storage.h"
#include "h264hwd_container.h"
#include "h264hwd_dpb.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

/* enumerated return values of the functions */
enum
{
    H264BSD_RDY,
    H264BSD_PIC_RDY,
    H264BSD_HDRS_RDY,
    H264BSD_ERROR,
    H264BSD_PARAM_SET_ERROR,
    H264BSD_NEW_ACCESS_UNIT,
    H264BSD_FMO,
    H264BSD_UNPAIRED_FIELD,
    H264BSD_NONREF_PIC_SKIPPED
};

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

void h264bsdInit(storage_t * pStorage, u32 noOutputReordering,
    u32 useSmoothingBuffer);
u32 h264bsdDecode(decContainer_t * pDecCont, const u8 * byteStrm, u32 len,
                  u32 picId, u32 * readBytes);
void h264bsdShutdown(storage_t * pStorage);

const dpbOutPicture_t *h264bsdNextOutputPicture(storage_t * pStorage);

u32 h264bsdPicWidth(storage_t * pStorage);
u32 h264bsdPicHeight(storage_t * pStorage);
u32 h264bsdVideoRange(storage_t * pStorage);
u32 h264bsdMatrixCoefficients(storage_t * pStorage);
u32 h264bsdIsMonoChrome(storage_t * pStorage);
void h264bsdCroppingParams(storage_t *pStorage, H264CropParams *pCrop);

u32 h264bsdCheckValidParamSets(storage_t * pStorage);

void h264bsdFlushBuffer(storage_t * pStorage);

u32 h264bsdAspectRatioIdc(const storage_t * pStorage);
void h264bsdSarSize(const storage_t * pStorage, u32 * sar_width,
                    u32 * sar_height);

u32 h264bsdFixFrameNum(u8 *pStream, u32 strmLen, u32 frameNum, u32 maxFrameNum,
    u32 *skippedBytes);

#endif /* #ifdef H264HWD_DECODER_H */
