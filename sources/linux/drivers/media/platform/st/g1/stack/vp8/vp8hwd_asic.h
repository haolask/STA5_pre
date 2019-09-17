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

#ifndef __VP8HWD_ASIC_H__
#define __VP8HWD_ASIC_H__

#include "basetype.h"
#include "vp8hwd_container.h"
#include "regdrv.h"

#define DEC_8190_ALIGN_MASK         0x07U

#define VP8HWDEC_HW_RESERVED         0x0100
#define VP8HWDEC_SYSTEM_ERROR        0x0200
#define VP8HWDEC_SYSTEM_TIMEOUT      0x0300
#define VP8HWDEC_ASYNC_MODE          0xF000

void VP8HwdAsicInit(VP8DecContainer_t * pDecCont);
i32 VP8HwdAsicAllocateMem(VP8DecContainer_t * pDecCont);
void VP8HwdAsicReleaseMem(VP8DecContainer_t * pDecCont);
i32 VP8HwdAsicAllocatePictures(VP8DecContainer_t * pDecCont);
void VP8HwdAsicReleasePictures(VP8DecContainer_t * pDecCont);

void VP8HwdAsicInitPicture(VP8DecContainer_t * pDecCont);
void VP8HwdAsicStrmPosUpdate(VP8DecContainer_t * pDecCont, u32 busAddress);
u32 VP8HwdAsicRun(VP8DecContainer_t * pDecCont);

void VP8HwdAsicProbUpdate(VP8DecContainer_t * pDecCont);

void VP8HwdUpdateRefs(VP8DecContainer_t * pDecCont, u32 corrupted);

void VP8HwdAsicContPicture(VP8DecContainer_t * pDecCont);

DWLLinearMem_t* GetOutput(VP8DecContainer_t *pDecCont);
void VP8HwdSegmentMapUpdate(VP8DecContainer_t * pDecCont);
u32* VP8HwdRefStatusAddress(VP8DecContainer_t * pDecCont);

#endif /* __VP8HWD_ASIC_H__ */
