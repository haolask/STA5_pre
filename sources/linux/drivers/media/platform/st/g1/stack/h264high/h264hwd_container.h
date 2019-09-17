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
--  Abstract : Definition of decContainer_t data structure
--
------------------------------------------------------------------------------*/

#ifndef H264HWD_CONTAINER_H
#define H264HWD_CONTAINER_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "h264hwd_storage.h"
#include "h264hwd_util.h"
#include "refbuffer.h"
#include "deccfg.h"
#include "decppif.h"
#include "workaround.h"

#include "h264hwd_dpb_lock.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

#define H264DEC_UNINITIALIZED   0U
#define H264DEC_INITIALIZED     1U
#define H264DEC_BUFFER_EMPTY    2U
#define H264DEC_NEW_HEADERS     3U

/* asic interface */
typedef struct DecAsicBuffers
{
    u32 buff_status;
    DWLLinearMem_t mbCtrl;
    DWLLinearMem_t mv;
    DWLLinearMem_t intraPred;
    DWLLinearMem_t residual;
    DWLLinearMem_t *outBuffer;
    DWLLinearMem_t cabacInit[MAX_ASIC_CORES];
    u32 refPicList[16];
    u32 maxRefFrm;
    u32 filterDisable;
    i32 chromaQpIndexOffset;
    i32 chromaQpIndexOffset2;
    u32 currentMB;
    u32 notCodedMask;
    u32 rlcWords;
    u32 picSizeInMbs;
    u32 wholePicConcealed;
    u32 disableOutWriting;
    u32 enableDmvAndPoc;
} DecAsicBuffers_t;

typedef struct
{
    u32 coreID;
    const u8 *pStream;
    const void *pUserData;
    u32 isFieldPic;
    u32 isBottomField;
    u32 outId;
    dpbStorage_t *currentDpb;
    u32 refId[16];
} H264HwRdyCallbackArg;

typedef struct decContainer
{
    const void *checksum;
    u32 decStat;
    u32 picNumber;
    u32 asicRunning;
    u32 rlcMode;
    u32 tryVlc;
    u32 reallocate;
    const u8 *pHwStreamStart;
    u32 hwStreamStartBus;
    u32 hwBitPos;
    u32 hwLength;
    u32 streamPosUpdated;
    u32 nalStartCode;
    u32 modeChange;
    u32 gapsCheckedForThis;
    u32 packetDecoded;
    u32 forceRlcMode;        /* by default stays 0, testing can set it to 1 for RLC mode */

    u32 h264Regs[DEC_X170_REGISTERS];
    storage_t storage;       /* h264bsd storage */
    DecAsicBuffers_t asicBuff[1];
    const void *dwl;         /* DWL instance */
    i32 coreID;
    u32 refBufSupport;
    u32 tiledModeSupport;
    u32 tiledReferenceEnable;
    u32 h264ProfileSupport;
    u32 is8190;
    u32 maxDecPicWidth;
    u32 allowDpbFieldOrdering;
    u32 dpbMode;
    refBuffer_t refBufferCtrl;

    u32 keepHwReserved;
    u32 skipNonReference;

    workaround_t workarounds;
    u32 frameNumMask; /* for workaround */
    u32 forceNalMode;

    DWLHwConfig_t hwCfg[MAX_ASIC_CORES];

    FrameBufferList fbList;
    u32 bMC; /* flag to indicate MC mode status */
    u32 nCores;
    struct
    {
        H264DecMCStreamConsumed *fn;
        const u8 *pStrmBuff; /* stream buffer passed in callback */
        const void *pUserData; /* user data to be passed in callback */
    } streamConsumedCallback;

    H264HwRdyCallbackArg hwRdyCallbackArg[MAX_ASIC_CORES];
    i32 poc[34];

    struct pp_
    {
        const void *ppInstance;
        void (*PPDecStart) (const void *, const DecPpInterface *);
        void (*PPDecWaitEnd) (const void *);
        void (*PPConfigQuery) (const void *, DecPpQuery *);
        void (*PPNextDisplayId)(const void *, u32); /* set the next PP outpic ID (multibuffer) */ 
        DecPpInterface decPpIf;
        DecPpQuery ppInfo;
        const DWLLinearMem_t * sentPicToPp[17]; /* list of pictures sent to pp */
        const DWLLinearMem_t * queuedPicToPp[2]; /* queued picture that should be processed next */
        u32 multiMaxId; /* maximum position used in sentPicToPp[] */
    } pp;
} decContainer_t;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

#endif /* #ifdef H264HWD_CONTAINER_H */
