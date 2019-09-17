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
--  Abstract :
--
------------------------------------------------------------------------------*/

#ifndef AVSDECHDRS_H
#define AVSDECHDRS_H

#include "basetype.h"

typedef struct
{
    u32 dropFlag;
    u32 hours;
    u32 minutes;
    u32 seconds;
    u32 picture;
} DecTimeCode;

typedef struct
{
    /* sequence header */
    u32 profileId;
    u32 levelId;
    u32 progressiveSequence;
    u32 horizontalSize;
    u32 verticalSize;
    u32 chromaFormat;
    u32 aspectRatio;
    u32 frameRateCode;
    u32 bitRateValue;
    u32 lowDelay;
    u32 bbvBufferSize;

    /* sequence display extension header */
    u32 videoFormat;
    u32 sampleRange;
    u32 colorDescription;
    u32 colorPrimaries;
    u32 transferCharacteristics;
    u32 matrixCoefficients;
    u32 displayHorizontalSize;
    u32 displayVerticalSize;

    /* picture header */
    u32 picCodingType;
    u32 bbvDelay;
    DecTimeCode timeCode;
    u32 pictureDistance;
    u32 progressiveFrame;
    u32 pictureStructure;
    u32 advancedPredModeDisable;
    u32 topFieldFirst;
    u32 repeatFirstField;
    u32 fixedPictureQp;
    u32 pictureQp;
    u32 pictureReferenceFlag;
    u32 skipModeFlag;
    u32 loopFilterDisable;
    i32 alphaOffset;
    i32 betaOffset;

    /*AVS Plus stuff */
    /* weighting quant */
    u32 weightingQuantFlag;
    u32 chromaQuantParamDisable;
    i32 chromaQuantParamDeltaCb;
    i32 chromaQuantParamDeltaCr;
    u32 weightingQuantParamIndex;
    u32 weightingQuantModel;
    i32 weightingQuantParamDelta1[6];
    i32 weightingQuantParamDelta2[6];
    u32 weightingQuantParam[6]; // wqP[m][6]

    /* advance entropy coding */
    u32 aecEnable;

    /* picture enhance */
    u32 noForwardReferenceFlag;
    u32 pbFieldEnhancedFlag;
} DecHdrs;

#endif /* #ifndef AVSDECHDRS_H */
