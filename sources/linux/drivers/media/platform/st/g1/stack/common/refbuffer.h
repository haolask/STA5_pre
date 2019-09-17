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
--  Description : Hardware interface read/write
--
------------------------------------------------------------------------------*/
#ifndef __REFBUFFER_H__
#define __REFBUFFER_H__

#include "basetype.h"

typedef enum {
    REFBU_FRAME,
    REFBU_FIELD,
    REFBU_MBAFF
} refbuMode_e;

/* Feature support flags */
#define REFBU_SUPPORT_GENERIC       (1)
#define REFBU_SUPPORT_INTERLACED    (2)
#define REFBU_SUPPORT_DOUBLE        (4)
#define REFBU_SUPPORT_OFFSET        (8)

/* Buffering info */
#define REFBU_BUFFER_SINGLE_FIELD   (1)
#define REFBU_MULTIPLE_REF_FRAMES   (2)
#define REFBU_DISABLE_CHECKPOINT    (4)
#define REFBU_FORCE_ADAPTIVE_SINGLE (8)
#define REFBU_DONT_USE_STATS        (16)
#define REFBU_DISABLE               (32)

#ifndef HANTRO_TRUE
    #define HANTRO_TRUE     (1)
#endif /* HANTRO_TRUE */

#ifndef HANTRO_FALSE
    #define HANTRO_FALSE    (0)
#endif /* HANTRO_FALSE*/

/* macro to get smaller of two values */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* macro to get greater of two values */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct memAccess {
    u32 latency;
    u32 nonseq;
    u32 seq;
} memAccess_t;

struct refBuffer;
typedef struct refBuffer {
#if 0
    i32 ox[3];
#endif
    i32 decModeMbWeights[2];
    i32 mbWeight;
    i32 oy[3];
    i32 picWidthInMbs;
    i32 picHeightInMbs;
    i32 frmSizeInMbs;
    i32 fldSizeInMbs;
    i32 numIntraBlk[3];
    i32 coverage[3];
    i32 fldHitsP[3][2];
    i32 fldHitsB[3][2];
    i32 fldCnt;
    i32 mvsPerMb;
    i32 filterSize;
    /* Thresholds */
    i32 predIntraBlk;
    i32 predCoverage;
    i32 checkpoint;
    u32 decMode;
    u32 dataExcessMaxPct;

    i32 busWidthInBits;
    i32 prevLatency;
    i32 numCyclesForBuffering;
    i32 totalDataForBuffering;
    i32 bufferPenalty;
    i32 avgCyclesPerMb;
    u32 prevWasField;
    u32 prevUsedDouble;
    i32 thrAdj;
    u32 prevFrameHitSum;
    memAccess_t currMemModel;   /* Clocks per operation, modifiable from 
                                 * testbench. */
    memAccess_t memAccessStats; /* Approximate counts for operations, set
                                 * based on format */
    u32 memAccessStatsFlag;

    /* Support flags */
    u32 interlacedSupport;
    u32 doubleSupport;
    u32 offsetSupport;

    /* Internal test mode */
    void (*testFunction)(struct refBuffer*,u32*regBase,u32 isIntra,u32 mode);

} refBuffer_t;
void RefbuInit( refBuffer_t *pRefbu, u32 decMode, u32 picWidthInMbs, u32 
                picHeightInMbs, u32 supportFlags );

void RefbuMvStatistics( refBuffer_t *pRefbu, u32 *regBase, 
                        u32 *pMv, u32 directMvsAvailable, 
                        u32 isIntraPicture );

void RefbuMvStatisticsB( refBuffer_t *pRefbu, u32 *regBase );

void RefbuSetup( refBuffer_t *pRefbu, u32 *regBase,
                 refbuMode_e mode,
                 u32 isIntraFrame, u32 isBframe, 
                 u32 refPicId0, u32 refpicId1,
                 u32 flags );

i32 RefbuGetHitThreshold( refBuffer_t *pRefbu );
u32 RefbuVpxGetPrevFrameStats( refBuffer_t *pRefbu );

#endif /* __REFBUFFER_H__ */
