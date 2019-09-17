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

#ifndef __VP8_DECODER_H__
#define __VP8_DECODER_H__

#include "basetype.h"

#define VP8HWD_VP7             1U
#define VP8HWD_VP8             2U

#define DEC_8190_ALIGN_MASK         0x07U
#define DEC_8190_MODE_VP8           0x09U

#define VP8HWDEC_HW_RESERVED         0x0100
#define VP8HWDEC_SYSTEM_ERROR        0x0200
#define VP8HWDEC_SYSTEM_TIMEOUT      0x0300


#define MAX_NBR_OF_DCT_PARTITIONS       (8)

#define MAX_NBR_OF_SEGMENTS             (4)
#define MAX_NBR_OF_MB_REF_LF_DELTAS     (4)
#define MAX_NBR_OF_MB_MODE_LF_DELTAS    (4)

#define MAX_NBR_OF_VP7_MB_FEATURES      (4)

#define MAX_SNAPSHOT_WIDTH  1024 
#define MAX_SNAPSHOT_HEIGHT 1024

#define VP7_MV_PROBS_PER_COMPONENT      (17)
#define VP8_MV_PROBS_PER_COMPONENT      (19)

#define WEBP_MAX_PIXEL_AMOUNT_NONSLICE (16370688)

enum
{
    HANTRO_NOK =  1,
    HANTRO_OK   = 0
};

#ifndef HANTRO_FALSE
#define HANTRO_FALSE    (0)
#endif 

#ifndef HANTRO_TRUE
#define HANTRO_TRUE     (1)
#endif 

enum {
    VP8_SEG_FEATURE_DELTA,
    VP8_SEG_FEATURE_ABS
};

typedef enum 
{
    VP8_YCbCr_BT601,
    VP8_CUSTOM
} vpColorSpace_e;

typedef struct vp8EntropyProbs_s
{
    u8              probLuma16x16PredMode[4];
    u8              probChromaPredMode[3];
    u8              probMvContext[2][VP8_MV_PROBS_PER_COMPONENT];
    u8              probCoeffs[4][8][3][11];
} vp8EntropyProbs_t;

typedef struct vp8Decoder_s
{
    u32             decMode;

    /* Current frame dimensions */
    u32             width;
    u32             height;
    u32             scaledWidth;
    u32             scaledHeight;   

    u32             vpVersion;
    u32             vpProfile;

    u32             keyFrame;

    u32             coeffSkipMode;

    /* DCT coefficient partitions */
    u32             offsetToDctParts;
    u32             nbrDctPartitions;
    u32             dctPartitionOffsets[MAX_NBR_OF_DCT_PARTITIONS];

    vpColorSpace_e  colorSpace; 
    u32             clamping;   
    u32             showFrame;  


    u32             refreshGolden; 
    u32             refreshAlternate; 
    u32             refreshLast;
    u32             refreshEntropyProbs;
    u32             copyBufferToGolden;
    u32             copyBufferToAlternate;

    u32             refFrameSignBias[2];
    u32             useAsReference;
    u32             loopFilterType;
    u32             loopFilterLevel;
    u32             loopFilterSharpness;

    /* Quantization parameters */
    i32             qpYAc, qpYDc, qpY2Ac, qpY2Dc, qpChAc, qpChDc;

    /* From here down, frame-to-frame persisting stuff */

    u32             vp7ScanOrder[16];
    u32             vp7PrevScanOrder[16];
    
    /* Probabilities */
    u32             probIntra;
    u32             probRefLast;
    u32             probRefGolden;
    u32             probMbSkipFalse;
    u32             probSegment[3];
    vp8EntropyProbs_t entropy,
                      entropyLast;

    /* Segment and macroblock specific values */
    u32             segmentationEnabled;
    u32             segmentationMapUpdate;
    u32             segmentFeatureMode; /* delta/abs */
    i32             segmentQp[MAX_NBR_OF_SEGMENTS];
    i32             segmentLoopfilter[MAX_NBR_OF_SEGMENTS];
    u32             modeRefLfEnabled;
    i32             mbRefLfDelta[MAX_NBR_OF_MB_REF_LF_DELTAS];
    i32             mbModeLfDelta[MAX_NBR_OF_MB_MODE_LF_DELTAS];

    u32             frameTagSize;

    /* Value to remember last frames prediction for hits into most
     * probable reference frame */
    u32             refbuPredHits;

    u32             probsDecoded;
} 
vp8Decoder_t;

struct DecAsicBuffers;

void vp8hwdResetDecoder( vp8Decoder_t * dec);
void vp8hwdPrepareVp7Scan( vp8Decoder_t * dec, u32 * newOrder );
void vp8hwdResetSegMap( vp8Decoder_t * dec, struct DecAsicBuffers *asicBuff, u32 coreID);

#endif /* __VP8_BOOL_H__ */
