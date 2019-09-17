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

#ifndef MPEG2DECHDRS_H
#define MPEG2DECHDRS_H

#include "basetype.h"

typedef struct DecTimeCode_t
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
    u32 horizontalSize;
    u32 verticalSize;
    u32 aspectRatioInfo;
    u32 parWidth;
    u32 parHeight;
    u32 frameRateCode;
    u32 bitRateValue;
    u32 vbvBufferSize;
    u32 constrParameters;
    u32 loadIntraMatrix;
    u32 loadNonIntraMatrix;
    u8 qTableIntra[64];
    u8 qTableNonIntra[64];
    /* sequence extension header */
    u32 profileAndLevelIndication;
    u32 progressiveSequence;
    u32 chromaFormat;
    u32 horSizeExtension;
    u32 verSizeExtension;
    u32 bitRateExtension;
    u32 vbvBufferSizeExtension;
    u32 lowDelay;
    u32 frameRateExtensionN;
    u32 frameRateExtensionD;
    /* sequence display extension header */
    u32 videoFormat;
    u32 colorDescription;
    u32 colorPrimaries;
    u32 transferCharacteristics;
    u32 matrixCoefficients;
    u32 displayHorizontalSize;
    u32 displayVerticalSize;
    /* GOP (Group of Pictures) header */
    DecTimeCode time;
    u32 closedGop;
    u32 brokenLink;
    /* picture header */
    u32 temporalReference;
    u32 pictureCodingType;
    u32 vbvDelay;
    u32 extraInfoByteCount;
    /* picture coding extension header */
    u32 fCode[2][2];
    u32 intraDcPrecision;
    u32 pictureStructure;
    u32 topFieldFirst;
    u32 framePredFrameDct;
    u32 concealmentMotionVectors;
    u32 quantType;
    u32 intraVlcFormat;
    u32 alternateScan;
    u32 repeatFirstField;
    u32 chroma420Type;
    u32 progressiveFrame;
    u32 compositeDisplayFlag;
    u32 vAxis;
    u32 fieldSequence;
    u32 subCarrier;
    u32 burstAmplitude;
    u32 subCarrierPhase;
    /* picture display extension header */
    u32 frameCentreHorOffset[3];
    u32 frameCentreVerOffset[3];

    /* extra */
    u32 mpeg2Stream;
    u32 frameRate;
    u32 videoRange;
    u32 interlaced;
    u32 repeatFrameCount;
    i32 firstFieldInFrame;
    i32 fieldIndex;
    i32 fieldOutIndex;

    /* for motion vectors */
    u32 fCodeFwdHor;
    u32 fCodeFwdVer;
    u32 fCodeBwdHor;
    u32 fCodeBwdVer;

} DecHdrs;

#endif /* #ifndef MPEG2DECHDRS_H */
