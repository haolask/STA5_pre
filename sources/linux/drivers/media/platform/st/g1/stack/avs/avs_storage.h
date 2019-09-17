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
--  Abstract : Stream decoding storage definition
--
------------------------------------------------------------------------------*/

#ifndef AVSDECSTRMSTORAGE_H_DEFINED
#define AVSDECSTRMSTORAGE_H_DEFINED

#include "basetype.h"
#include "dwl.h"
#include "avs_cfg.h"
#include "avsdecapi.h"
#include "bqueue.h"

typedef struct
{
    DWLLinearMem_t data;
    u32 picType;
    u32 picId;
    u32 picCodeType;
    u32 tf;
    u32 ff[2];
    u32 rff;
    u32 rfc;
    u32 isInter;
    u32 nbrErrMbs;
    u32 pictureDistance;
    AvsDecRet retVal;
    u32 sendToPp;
    AvsDecTime timeCode;

    u32 tiledMode;
} picture_t;

typedef struct
{
    u32 status;
    u32 strmDecReady;

    u32 validPicHeader;
    u32 validSequence;

    picture_t pPicBuf[16];
    u32 outBuf[16];
    u32 outIndex;
    u32 outCount;
    u32 workOut;
    u32 work0;
    u32 work1;
    u32 latestId;   /* current pic id, used for debug */
    u32 skipB;
    u32 prevPicCodingType;
    u32 prevPicStructure;
    u32 fieldToReturn;  /* 0 = First, 1 second */

    u32 fieldIndex;
    i32 fieldOutIndex;

    u32 frameNumber;
    u32 frameWidth;
    u32 frameHeight;
    u32 totalMbsInFrame;

    DWLLinearMem_t directMvs;

    u32 pictureBroken;
    u32 intraFreeze;
    u32 partialFreeze;
    u32 unsupportedFeaturesPresent;

    u32 previousB;
    u32 previousModeFull;

    u32 sequenceLowDelay;

    u32 newHeadersChangeResolution;

    u32 maxNumBuffers;
    u32 numBuffers;
    u32 numPpBuffers;
    bufferQueue_t bq;
    bufferQueue_t bqPp;
    u32 future2prevPastDist;
} DecStrmStorage;

#endif /* #ifndef AVSDECSTRMSTORAGE_H_DEFINED */
