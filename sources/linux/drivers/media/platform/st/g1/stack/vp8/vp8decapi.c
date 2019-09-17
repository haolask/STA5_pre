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
#include "basetype.h"
#include "decapicommon.h"
#include "vp8decapi.h"
#include "vp8decmc_internals.h"

#include "version.h"

#include "dwl.h"
#include "vp8hwd_buffer_queue.h"
#include "vp8hwd_container.h"

#include "vp8hwd_debug.h"
#include "tiledref.h"

#include "vp8hwd_asic.h"

#include "regdrv.h"
#include "refbuffer.h"

#include "vp8hwd_asic.h"
#include "vp8hwd_headers.h"

#include "errorhandling.h"

#define VP8DEC_MAJOR_VERSION 1
#define VP8DEC_MINOR_VERSION 0

#ifdef _TRACE_PP_CTRL
#ifndef TRACE_PP_CTRL
#include <stdio.h>
#define TRACE_PP_CTRL(...)          printf(__VA_ARGS__)
#endif
#else
#define TRACE_PP_CTRL(...)
#endif

#ifdef VP8DEC_TRACE
#define DEC_API_TRC(str)    VP8DecTrace(str)
#else
#define DEC_API_TRC(str)    do{}while(0)
#endif

#define MB_MULTIPLE(x)  (((x)+15)&~15)

#define MIN_PIC_WIDTH  48
#define MIN_PIC_HEIGHT 48
#define MAX_PIC_SIZE   4096*4096

extern void vp8hwdPreparePpRun(VP8DecContainer_t *pDecCont);
static u32 vp8hwdCheckSupport( VP8DecContainer_t *pDecCont );
static void vp8hwdFreeze(VP8DecContainer_t *pDecCont);
static u32 CheckBitstreamWorkaround(vp8Decoder_t* dec);
#if 0
static void DoBitstreamWorkaround(vp8Decoder_t* dec, DecAsicBuffers_t *pAsicBuff, vpBoolCoder_t*bc);
#endif
void vp8hwdErrorConceal(VP8DecContainer_t *pDecCont, u32 busAddress,
                        u32 concealEverything);
static DWLLinearMem_t* GetPrevRef(VP8DecContainer_t *pDecCont);
void ConcealRefAvailability(u32 * output, u32 height, u32 width);

/*------------------------------------------------------------------------------
    Function name : VP8DecGetAPIVersion
    Description   : Return the API version information

    Return type   : VP8DecApiVersion
    Argument      : void
------------------------------------------------------------------------------*/
VP8DecApiVersion VP8DecGetAPIVersion(void)
{
    VP8DecApiVersion ver;

    ver.major = VP8DEC_MAJOR_VERSION;
    ver.minor = VP8DEC_MINOR_VERSION;

    DEC_API_TRC("VP8DecGetAPIVersion# OK\n");

    return ver;
}

/*------------------------------------------------------------------------------
    Function name : VP8DecGetBuild
    Description   : Return the SW and HW build information

    Return type   : VP8DecBuild
    Argument      : void
------------------------------------------------------------------------------*/
VP8DecBuild VP8DecGetBuild(void)
{
    VP8DecBuild buildInfo;

    (void)DWLmemset(&buildInfo, 0, sizeof(buildInfo));

    buildInfo.swBuild = HANTRO_DEC_SW_BUILD;
    buildInfo.hwBuild = DWLReadAsicID();

    DWLReadAsicConfig(buildInfo.hwConfig);

    DEC_API_TRC("VP8DecGetBuild# OK\n");

    return buildInfo;
}

/*------------------------------------------------------------------------------
    Function name   : vp8decinit
    Description     : 
    Return type     : VP8DecRet 
    Argument        : VP8DecInst * pDecInst
                      DecErrorHandling errorHandling
------------------------------------------------------------------------------*/
VP8DecRet VP8DecInit(VP8DecInst * pDecInst, VP8DecFormat decFormat,
                     DecErrorHandling errorHandling,
                     u32 numFrameBuffers,
                     DecDpbFlags dpbFlags,
                     void * userCtx)
{
    VP8DecContainer_t *pDecCont;
    const void *dwl;
    u32 i;
    u32 referenceFrameFormat;

    DWLInitParam_t dwlInit;
    DWLHwConfig_t config;

    DEC_API_TRC("VP8DecInit#\n");

    DWLSetup(userCtx);

    /* check that right shift on negative numbers is performed signed */
    /*lint -save -e* following check causes multiple lint messages */
#if (((-1) >> 1) != (-1))
#error Right bit-shifting (>>) does not preserve the sign
#endif
    /*lint -restore */

    if(pDecInst == NULL)
    {
        DEC_API_TRC("VP8DecInit# ERROR: pDecInst == NULL");
        return (VP8DEC_PARAM_ERROR);
    }

    *pDecInst = NULL;   /* return NULL instance for any error */

    /* check that decoding supported in HW */
    {

        DWLHwConfig_t hwCfg;

        DWLReadAsicConfig(&hwCfg);
        if(decFormat == VP8DEC_VP7 && !hwCfg.vp7Support)
        {
            DEC_API_TRC("VP8DecInit# ERROR: VP7 not supported in HW\n");
            return VP8DEC_FORMAT_NOT_SUPPORTED;
        }

        if((decFormat == VP8DEC_VP8 || decFormat == VP8DEC_WEBP) &&
            !hwCfg.vp8Support)
        {
            DEC_API_TRC("VP8DecInit# ERROR: VP8 not supported in HW\n");
            return VP8DEC_FORMAT_NOT_SUPPORTED;
        }

    }

    /* init DWL for the specified client */
    dwlInit.clientType = DWL_CLIENT_TYPE_VP8_DEC;

    dwl = DWLInit(&dwlInit);

    if(dwl == NULL)
    {
        DEC_API_TRC("VP8DecInit# ERROR: DWL Init failed\n");
        return (VP8DEC_DWL_ERROR);
    }

    DWLSetUserContext(dwl, userCtx);

    /* allocate instance */
    pDecCont = (VP8DecContainer_t *) DWLmalloc(sizeof(VP8DecContainer_t));

    if(pDecCont == NULL)
    {
        DEC_API_TRC("VP8DecInit# ERROR: Memory allocation failed\n");

        (void)DWLRelease(dwl);
        return VP8DEC_MEMFAIL;
    }

    (void) DWLmemset(pDecCont, 0, sizeof(VP8DecContainer_t));
    pDecCont->dwl = dwl;

    /* initial setup of instance */

    pDecCont->decStat = VP8DEC_INITIALIZED;
    pDecCont->checksum = pDecCont;  /* save instance as a checksum */

    if( numFrameBuffers > VP8DEC_MAX_PIC_BUFFERS)
        numFrameBuffers = VP8DEC_MAX_PIC_BUFFERS;
    switch(decFormat)
    {
        case VP8DEC_VP7:
            pDecCont->decMode = pDecCont->decoder.decMode = VP8HWD_VP7;
            if(numFrameBuffers < 3)
                numFrameBuffers = 3;
            break;
        case VP8DEC_VP8:
            pDecCont->decMode = pDecCont->decoder.decMode = VP8HWD_VP8;
            if(numFrameBuffers < 4)
                numFrameBuffers = 4;
            break;
        case VP8DEC_WEBP:
            pDecCont->decMode = pDecCont->decoder.decMode = VP8HWD_VP8;
            pDecCont->intraOnly = HANTRO_TRUE;
            numFrameBuffers = 1;
            break;
    }
    pDecCont->numBuffers = numFrameBuffers;
    VP8HwdAsicInit(pDecCont);   /* Init ASIC */

    if(!DWLReadAsicCoreCount())
    {
      DEC_API_TRC("VP8DecInit# ERROR: Unexpected core count\n");
      DWLfree(pDecCont);
      (void)DWLRelease(dwl);
      return (VP8DEC_DWL_ERROR);
    }
    pDecCont->numCores = 1;

    (void)DWLmemset(&config, 0, sizeof(DWLHwConfig_t));

    DWLReadAsicConfig(&config);

    i = DWLReadAsicID() >> 16;
    if(i == 0x8170U)
        errorHandling = 0;
    pDecCont->refBufSupport = config.refBufSupport;
    referenceFrameFormat = dpbFlags & DEC_REF_FRM_FMT_MASK;
    if(referenceFrameFormat == DEC_REF_FRM_TILED_DEFAULT)
    {
        /* Assert support in HW before enabling.. */
        if(!config.tiledModeSupport)
        {
            DEC_API_TRC("VP8DecInit# ERROR: Tiled reference picture format not supported in HW\n");
            DWLfree(pDecCont);
            (void)DWLRelease(dwl);
            return VP8DEC_FORMAT_NOT_SUPPORTED;
        }
        pDecCont->tiledModeSupport = config.tiledModeSupport;
    }
    else
        pDecCont->tiledModeSupport = 0;
    pDecCont->intraFreeze = errorHandling == DEC_EC_VIDEO_FREEZE;
    if (errorHandling == DEC_EC_PARTIAL_FREEZE)
        pDecCont->partialFreeze = 1;
    else if (errorHandling == DEC_EC_PARTIAL_IGNORE)
        pDecCont->partialFreeze = 2;
    pDecCont->pictureBroken = 0;
    pDecCont->decoder.refbuPredHits = 0;

    if (!errorHandling && decFormat == VP8DEC_VP8)
        pDecCont->hwEcSupport = config.ecSupport;

    if ((decFormat == VP8DEC_VP8) || (decFormat == VP8DEC_WEBP))
        pDecCont->strideSupport = config.strideSupport;

    /* return new instance to application */
    *pDecInst = (VP8DecInst) pDecCont;

    DEC_API_TRC("VP8DecInit# OK\n");
    return (VP8DEC_OK);

}

/*------------------------------------------------------------------------------
    Function name   : VP8DecRelease
    Description     : 
    Return type     : void 
    Argument        : VP8DecInst decInst
------------------------------------------------------------------------------*/
void VP8DecRelease(VP8DecInst decInst)
{

    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;
    const void *dwl;

    DEC_API_TRC("VP8DecRelease#\n");

    if(pDecCont == NULL)
    {
        DEC_API_TRC("VP8DecRelease# ERROR: decInst == NULL\n");
        return;
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("VP8DecRelease# ERROR: Decoder not initialized\n");
        return;
    }

    /* PP instance must be already disconnected at this point */
    ASSERT(pDecCont->pp.ppInstance == NULL);

    dwl = pDecCont->dwl;

    if(pDecCont->asicRunning)
    {
        DWLDisableHW(dwl, pDecCont->coreID, 1 * 4, 0);    /* stop HW */
        DWLReleaseHw(dwl, pDecCont->coreID);  /* release HW lock */
        pDecCont->asicRunning = 0;
    }

    VP8HwdAsicReleaseMem(pDecCont);
    VP8HwdAsicReleasePictures(pDecCont);
    if (pDecCont->hwEcSupport)
        vp8hwdReleaseEc(&pDecCont->ec);

    if (pDecCont->fifoOut)
        fifo_release(pDecCont->fifoOut);
    if (pDecCont->fifoDisplay)
        fifo_release(pDecCont->fifoDisplay);

    pDecCont->checksum = NULL;
    DWLfree(pDecCont);

    {
        i32 dwlret = DWLRelease(dwl);

        ASSERT(dwlret == DWL_OK);
        (void) dwlret;
    }

    DEC_API_TRC("VP8DecRelease# OK\n");

    return;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecGetInfo
    Description     : 
    Return type     : VP8DecRet 
    Argument        : VP8DecInst decInst
    Argument        : VP8DecInfo * pDecInfo
------------------------------------------------------------------------------*/
VP8DecRet VP8DecGetInfo(VP8DecInst decInst, VP8DecInfo * pDecInfo)
{
    const VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;

    DEC_API_TRC("VP8DecGetInfo#");

    if(decInst == NULL || pDecInfo == NULL)
    {
        DEC_API_TRC("VP8DecGetInfo# ERROR: decInst or pDecInfo is NULL\n");
        return VP8DEC_PARAM_ERROR;
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("VP8DecGetInfo# ERROR: Decoder not initialized\n");
        return VP8DEC_NOT_INITIALIZED;
    }

    if (pDecCont->decStat == VP8DEC_INITIALIZED)
    {
        return VP8DEC_HDRS_NOT_RDY;
    }

    pDecInfo->vpVersion = pDecCont->decoder.vpVersion;
    pDecInfo->vpProfile = pDecCont->decoder.vpProfile;

    if(pDecCont->tiledReferenceEnable)
    {
        pDecInfo->outputFormat = VP8DEC_TILED_YUV420;
    }
    else
    {
        pDecInfo->outputFormat = VP8DEC_SEMIPLANAR_YUV420;
    }

    /* Fragments have 8 pixels */
    pDecInfo->codedWidth = pDecCont->decoder.width;
    pDecInfo->codedHeight = pDecCont->decoder.height;
    pDecInfo->frameWidth = (pDecCont->decoder.width + 15) & ~15;
    pDecInfo->frameHeight = (pDecCont->decoder.height + 15) & ~15;
    pDecInfo->scaledWidth = pDecCont->decoder.scaledWidth;
    pDecInfo->scaledHeight = pDecCont->decoder.scaledHeight;
    pDecInfo->dpbMode = DEC_DPB_FRAME;

    return VP8DEC_OK;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecDecode
    Description     : 
    Return type     : VP8DecRet 
    Argument        : VP8DecInst decInst
    Argument        : const VP8DecInput * pInput
    Argument        : VP8DecFrame * pOutput
------------------------------------------------------------------------------*/
VP8DecRet VP8DecDecode(VP8DecInst decInst,
                       const VP8DecInput * pInput, VP8DecOutput * pOutput)
{
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;
    DecAsicBuffers_t *pAsicBuff;
    i32 ret;
    u32 asic_status;
    u32 errorConcealment = 0;

    DEC_API_TRC("VP8DecDecode#\n");

    /* Check that function input parameters are valid */
    if(pInput == NULL || pOutput == NULL || decInst == NULL)
    {
        DEC_API_TRC("VP8DecDecode# ERROR: NULL arg(s)\n");
        return (VP8DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("VP8DecDecode# ERROR: Decoder not initialized\n");
        return (VP8DEC_NOT_INITIALIZED);
    }

    if(((pInput->dataLen > DEC_X170_MAX_STREAM) && !pDecCont->intraOnly) ||
       X170_CHECK_VIRTUAL_ADDRESS(pInput->pStream) ||
       X170_CHECK_BUS_ADDRESS(pInput->streamBusAddress))
    {
        DEC_API_TRC("VP8DecDecode# ERROR: Invalid input arg(s)\n");
        return VP8DEC_PARAM_ERROR;
    }

    if ((pInput->pPicBufferY != NULL && pInput->picBufferBusAddressY == 0) ||
        (pInput->pPicBufferY == NULL && pInput->picBufferBusAddressY != 0) ||
        (pInput->pPicBufferC != NULL && pInput->picBufferBusAddressC == 0) ||
        (pInput->pPicBufferC == NULL && pInput->picBufferBusAddressC != 0) ||
        (pInput->pPicBufferY == NULL && pInput->pPicBufferC != 0) ||
        (pInput->pPicBufferY != NULL && pInput->pPicBufferC == 0))
    {
        DEC_API_TRC("VP8DecDecode# ERROR: Invalid picture buffers input arg(s)\n");
        return VP8DEC_PARAM_ERROR;
    }

#ifdef VP8DEC_EVALUATION
    if(pDecCont->picNumber > VP8DEC_EVALUATION)
    {
        DEC_API_TRC("VP8DecDecode# VP8DEC_EVALUATION_LIMIT_EXCEEDED\n");
        return VP8DEC_EVALUATION_LIMIT_EXCEEDED;
    }
#endif

    if(!pInput->dataLen && pDecCont->streamConsumedCallback )
    {
        pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
        return VP8DEC_OK;
    }
    /* aliases */
    pAsicBuff = pDecCont->asicBuff;

    if((!pDecCont->intraOnly) && pInput->sliceHeight )
    {
        DEC_API_TRC("VP8DecDecode# ERROR: Invalid arg value\n");
        return VP8DEC_PARAM_ERROR;
    }

    /* application indicates that slice mode decoding should be used ->
     * disabled unless WebP and PP not used */
    if (pDecCont->decStat != VP8DEC_MIDDLE_OF_PIC &&
        pDecCont->intraOnly && pInput->sliceHeight &&
        pDecCont->pp.ppInstance == NULL)
    {
        DWLHwConfig_t hwConfig;
        u32 tmp;
        /* Slice mode can only be enabled if image width is larger 
         * than supported video decoder maximum width. */
        DWLReadAsicConfig(&hwConfig);
        if( pInput->dataLen >= 5 )
        {
            /* Peek frame width. We make shortcuts and assumptions:
             *  -always keyframe 
             *  -always VP8 (i.e. not VP7)
             *  -keyframe start code and frame tag skipped 
             *  -if keyframe start code invalid, handle it later */
            tmp = (pInput->pStream[7] << 8)|
                  (pInput->pStream[6]); /* Read 16-bit chunk */
            tmp = tmp & 0x3fff;
            if( tmp > hwConfig.maxDecPicWidth )
            {
                if(pInput->sliceHeight > 255)
                {
                    DEC_API_TRC("VP8DecDecode# ERROR: Slice height > max\n");
                    return VP8DEC_PARAM_ERROR;
                }
            
                pDecCont->sliceHeight = pInput->sliceHeight;
            }
            else
            {
                pDecCont->sliceHeight = 0;
            }
        }
        else
        {
            /* Too little data in buffer, let later error management
             * handle it. Disallow slice mode. */
        }
    }

    if (pDecCont->intraOnly && pInput->pPicBufferY)
    {
        pDecCont->userMem = 1;
        pDecCont->asicBuff->userMem.pPicBufferY[0] = pInput->pPicBufferY;
        pDecCont->asicBuff->userMem.picBufferBusAddrY[0] =
            pInput->picBufferBusAddressY;
        pDecCont->asicBuff->userMem.pPicBufferC[0] = pInput->pPicBufferC;
        pDecCont->asicBuff->userMem.picBufferBusAddrC[0] =
            pInput->picBufferBusAddressC;
    }

    if (pDecCont->decStat == VP8DEC_NEW_HEADERS)
    {
        /* if picture size > 16mpix, slice mode mandatory */
        if((pDecCont->asicBuff->width*
            pDecCont->asicBuff->height > WEBP_MAX_PIXEL_AMOUNT_NONSLICE) &&
            pDecCont->intraOnly && 
            (pDecCont->sliceHeight == 0) &&
            (pDecCont->pp.ppInstance == NULL) )
        {
            if(pDecCont->streamConsumedCallback != NULL)
            {
                pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
            }
            return VP8DEC_STREAM_NOT_SUPPORTED;
        }

        if(VP8HwdAsicAllocatePictures(pDecCont) != 0 ||
           (pDecCont->hwEcSupport &&
            vp8hwdInitEc(&pDecCont->ec, pDecCont->width, pDecCont->height, 16)))
        {
            if(pDecCont->streamConsumedCallback != NULL)
            {
                pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
            }
            DEC_API_TRC
                ("VP8DecDecode# ERROR: Picture memory allocation failed\n");
            return VP8DEC_MEMFAIL;
        }

        if(VP8HwdAsicAllocateMem(pDecCont) != 0)
        {
            if(pDecCont->streamConsumedCallback != NULL)
            {
                pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
            }
            DEC_API_TRC("VP8DecInit# ERROR: ASIC Memory allocation failed\n");
            return VP8DEC_MEMFAIL;
        }

        pDecCont->decStat = VP8DEC_DECODING;
    }
    else if (pDecCont->decStat != VP8DEC_MIDDLE_OF_PIC && pInput->dataLen)
    {
        pDecCont->prevIsKey = pDecCont->decoder.keyFrame;
        pDecCont->decoder.probsDecoded = 0;

        /* decode frame tag */
        vp8hwdDecodeFrameTag( pInput->pStream, &pDecCont->decoder );

        /* When on key-frame, reset probabilities and such */
        if( pDecCont->decoder.keyFrame )
        {
            vp8hwdResetDecoder( &pDecCont->decoder);
        }
        /* intra only and non key-frame */
        else if (pDecCont->intraOnly)
        {
            if(pDecCont->streamConsumedCallback != NULL)
            {
                pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
            }
            return VP8DEC_STRM_ERROR;
        }

        if (pDecCont->decoder.keyFrame || pDecCont->decoder.vpVersion > 0)
        {
            pAsicBuff->dcPred[0] = pAsicBuff->dcPred[1] =
            pAsicBuff->dcMatch[0] = pAsicBuff->dcMatch[1] = 0;
        }

        /* Decode frame header (now starts bool coder as well) */
        ret = vp8hwdDecodeFrameHeader( 
            pInput->pStream + pDecCont->decoder.frameTagSize,
            pInput->dataLen - pDecCont->decoder.frameTagSize,
            &pDecCont->bc, &pDecCont->decoder );
        if( ret != HANTRO_OK )
        {
            if(pDecCont->streamConsumedCallback != NULL)
            {
                pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
            }
            DEC_API_TRC("VP8DecDecode# ERROR: Frame header decoding failed\n");
            if (!pDecCont->picNumber || pDecCont->decStat != VP8DEC_DECODING)
                return VP8DEC_STRM_ERROR;
            else
            {
                vp8hwdFreeze(pDecCont);
                DEC_API_TRC("VP8DecDecode# VP8DEC_PIC_DECODED\n");
                return VP8DEC_PIC_DECODED;
            }
        }
        /* flag the stream as non "error-resilient" */
        else if (pDecCont->decoder.refreshEntropyProbs)
            pDecCont->probRefreshDetected = 1;

        if(CheckBitstreamWorkaround(&pDecCont->decoder))
        {
            /* do bitstream workaround */
            /*DoBitstreamWorkaround(&pDecCont->decoder, pAsicBuff, &pDecCont->bc);*/
        }

        ret = vp8hwdSetPartitionOffsets(pInput->pStream, pInput->dataLen,
            &pDecCont->decoder);
        /* ignore errors in partition offsets if HW error concealment used
         * (assuming parts of stream missing -> partition start offsets may
         * be larger than amount of stream in the buffer) */
        if (ret != HANTRO_OK && !pDecCont->hwEcSupport)
        {
            if(pDecCont->streamConsumedCallback != NULL)
            {
                pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
            }
            if (!pDecCont->picNumber || pDecCont->decStat != VP8DEC_DECODING)
                return VP8DEC_STRM_ERROR;
            else
            {
                vp8hwdFreeze(pDecCont);
                DEC_API_TRC("VP8DecDecode# VP8DEC_PIC_DECODED\n");
                return VP8DEC_PIC_DECODED;
            }
        }

        /* check for picture size change */
        if((pDecCont->width != (pDecCont->decoder.width)) ||
           (pDecCont->height != (pDecCont->decoder.height)))
        {

            if (pDecCont->streamConsumedCallback != NULL && pDecCont->bq)
            {
                VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                    pDecCont->asicBuff->outBufferI);
                pDecCont->asicBuff->outBufferI = VP8_UNDEFINED_BUFFER;
                VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                    VP8HwdBufferQueueGetPrevRef(pDecCont->bq));
                VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                    VP8HwdBufferQueueGetGoldenRef(pDecCont->bq));
                VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                    VP8HwdBufferQueueGetAltRef(pDecCont->bq));
                /* Wait for output processing to finish before releasing. */
                VP8HwdBufferQueueWaitPending(pDecCont->bq);
            }

            /* reallocate picture buffers */
            pAsicBuff->width = ( pDecCont->decoder.width + 15 ) & ~15;
            pAsicBuff->height = ( pDecCont->decoder.height + 15 ) & ~15;

            VP8HwdAsicReleasePictures(pDecCont);
            VP8HwdAsicReleaseMem(pDecCont);

            if (pDecCont->hwEcSupport)
                vp8hwdReleaseEc(&pDecCont->ec);

            if (vp8hwdCheckSupport(pDecCont) != HANTRO_OK)
            {
                if(pDecCont->streamConsumedCallback != NULL)
                {
                    pDecCont->streamConsumedCallback((u8*)pInput->pStream, pInput->pUserData);
                }
                pDecCont->decStat = VP8DEC_INITIALIZED;
                return VP8DEC_STREAM_NOT_SUPPORTED;
            }

            pDecCont->width = pDecCont->decoder.width;
            pDecCont->height = pDecCont->decoder.height;

            pDecCont->decStat = VP8DEC_NEW_HEADERS;

            if( pDecCont->refBufSupport && !pDecCont->intraOnly )
            {
                RefbuInit( &pDecCont->refBufferCtrl, 10, 
                           MB_MULTIPLE(pDecCont->decoder.width)>>4,
                           MB_MULTIPLE(pDecCont->decoder.height)>>4,
                           pDecCont->refBufSupport);
            }

            DEC_API_TRC("VP8DecDecode# VP8DEC_HDRS_RDY\n");

            return VP8DEC_HDRS_RDY;
        }

        /* If we are here and dimensions are still 0, it means that we have 
         * yet to decode a valid keyframe, in which case we must give up. */
        if( pDecCont->width == 0 || pDecCont->height == 0 )
        {
            return VP8DEC_STRM_PROCESSED;
        }

        /* If output picture is broken and we are not decoding a base frame, 
         * don't even start HW, just output same picture again. */
        if( !pDecCont->decoder.keyFrame &&
            pDecCont->pictureBroken &&
            (pDecCont->intraFreeze || pDecCont->forceIntraFreeze) )
        {
            vp8hwdFreeze(pDecCont);
            DEC_API_TRC("VP8DecDecode# VP8DEC_PIC_DECODED\n");
            return VP8DEC_PIC_DECODED;
        }

    }
    /* missing picture, conceal */
    else if (!pInput->dataLen)
    {
        if (!pDecCont->hwEcSupport || pDecCont->forceIntraFreeze ||
            pDecCont->prevIsKey)
        {
            pDecCont->decoder.probsDecoded = 0;
            vp8hwdFreeze(pDecCont);
            DEC_API_TRC("VP8DecDecode# VP8DEC_PIC_DECODED\n");
            return VP8DEC_PIC_DECODED;
        }
        else
        {
            pDecCont->concealStartMbX = pDecCont->concealStartMbY = 0;
            vp8hwdErrorConceal(pDecCont, pInput->streamBusAddress,
                /*concealEverything*/ 1);
            /* Assume that broken picture updated the last reference only and
             * also addref it since it will be outputted one more time. */
            VP8HwdBufferQueueAddRef(pDecCont->bq, pAsicBuff->outBufferI);
            VP8HwdBufferQueueUpdateRef(pDecCont->bq, BQUEUE_FLAG_PREV,
                                       pAsicBuff->outBufferI);
            pAsicBuff->prevOutBuffer = pAsicBuff->outBuffer;
            pAsicBuff->outBuffer = NULL;

            pAsicBuff->outBufferI = VP8HwdBufferQueueGetBuffer(pDecCont->bq);
            pAsicBuff->outBuffer = &pAsicBuff->pictures[pAsicBuff->outBufferI];
            pDecCont->picNumber++;
            pDecCont->outCount++;
            ASSERT(pAsicBuff->outBuffer != NULL);
            if (pDecCont->probRefreshDetected)
            {
                pDecCont->pictureBroken = 1;
                pDecCont->forceIntraFreeze = 1;
            }
            return VP8DEC_PIC_DECODED;
        }
    }

    if (pDecCont->decStat != VP8DEC_MIDDLE_OF_PIC)
    {
        pDecCont->refToOut = 0;

        if(pDecCont->pp.ppInstance != NULL &&
           pDecCont->pp.decPpIf.usePipeline)
        {
            /* reserved both DEC and PP hardware for pipeline */
            ret = DWLReserveHwPipe(pDecCont->dwl, &pDecCont->coreID);
        }
        else
        {
            ret = DWLReserveHw(pDecCont->dwl, &pDecCont->coreID);
        }

        if(ret != DWL_OK)
        {
            ERROR_PRINT("DWLReserveHw Failed");
            return VP8HWDEC_HW_RESERVED;
        }

        /* prepare asic */

        VP8HwdAsicProbUpdate(pDecCont);

        VP8HwdSegmentMapUpdate(pDecCont);

        VP8HwdAsicInitPicture(pDecCont);

        VP8HwdAsicStrmPosUpdate(pDecCont, pInput->streamBusAddress);

        /* Store the needed data for callback setup. */
        /* TODO(vmr): Consider parametrizing this. */
        pDecCont->pStream = pInput->pStream;
        pDecCont->pUserData = pInput->pUserData;

        /* PP setup stuff */
        vp8hwdPreparePpRun(pDecCont);
    }
    else
        VP8HwdAsicContPicture(pDecCont);

    if (pDecCont->partialFreeze)
    {
        PreparePartialFreeze((u8*)pAsicBuff->outBuffer->virtualAddress,
                             (pDecCont->width >> 4),(pDecCont->height >> 4));
    }

    /* run the hardware */
    asic_status = VP8HwdAsicRun(pDecCont);

    /* Rollback entropy probabilities if refresh is not set */
    if(pDecCont->decoder.refreshEntropyProbs == HANTRO_FALSE)
    {
        DWLmemcpy( &pDecCont->decoder.entropy, &pDecCont->decoder.entropyLast,
                   sizeof(vp8EntropyProbs_t));
        DWLmemcpy( pDecCont->decoder.vp7ScanOrder, pDecCont->decoder.vp7PrevScanOrder,
                   sizeof(pDecCont->decoder.vp7ScanOrder));
    }
    /* If in asynchronous mode, just return OK  */
    if (asic_status == VP8HWDEC_ASYNC_MODE)
    {
        /* find first free buffer and use it as next output */
        if (!errorConcealment || pDecCont->intraOnly)
        {
            pAsicBuff->prevOutBuffer = pAsicBuff->outBuffer;
            pAsicBuff->prevOutBufferI = pAsicBuff->outBufferI;
            pAsicBuff->outBuffer = NULL;

             /* If we are never going to output the current buffer, we can release
             * the ref for output on the buffer. */
            if (!pDecCont->decoder.showFrame)
                VP8HwdBufferQueueRemoveRef(pDecCont->bq, pAsicBuff->outBufferI);
            /* If WebP, we will be recycling only one buffer and no update is
             * necessary. */
            if (!pDecCont->intraOnly)
            {
                pAsicBuff->outBufferI = VP8HwdBufferQueueGetBuffer(pDecCont->bq);
            }
            pAsicBuff->outBuffer = &pAsicBuff->pictures[pAsicBuff->outBufferI];
            ASSERT(pAsicBuff->outBuffer != NULL);
        }
        return VP8DEC_OK;
    }

    /* Handle system error situations */
    if(asic_status == VP8HWDEC_SYSTEM_TIMEOUT)
    {
        /* This timeout is DWL(software/os) generated */
        DEC_API_TRC("VP8DecDecode# VP8DEC_HW_TIMEOUT, SW generated\n");
        return VP8DEC_HW_TIMEOUT;
    }
    else if(asic_status == VP8HWDEC_SYSTEM_ERROR)
    {
        DEC_API_TRC("VP8DecDecode# VP8HWDEC_SYSTEM_ERROR\n");
        return VP8DEC_SYSTEM_ERROR;
    }
    else if(asic_status == VP8HWDEC_HW_RESERVED)
    {
        DEC_API_TRC("VP8DecDecode# VP8HWDEC_HW_RESERVED\n");
        return VP8DEC_HW_RESERVED;
    }

    /* Handle possible common HW error situations */
    if(asic_status & DEC_8190_IRQ_BUS)
    {
        DEC_API_TRC("VP8DecDecode# VP8DEC_HW_BUS_ERROR\n");
        return VP8DEC_HW_BUS_ERROR;
    }

    /* for all the rest we will output a picture (concealed or not) */
    if((asic_status & DEC_8190_IRQ_TIMEOUT) ||
       (asic_status & DEC_8190_IRQ_ERROR) ||
       (asic_status & DEC_8190_IRQ_ASO) || /* to signal lost residual */
       (asic_status & DEC_8190_IRQ_ABORT))
    {
        u32 concealEverything = 0;

        if (!pDecCont->partialFreeze ||
            !ProcessPartialFreeze((u8*)pAsicBuff->outBuffer->virtualAddress,
                                  (u8*)GetPrevRef(pDecCont)->virtualAddress,
                                  (pDecCont->width >> 4),
                                  (pDecCont->height >> 4),
                                  pDecCont->partialFreeze == 1))
        {
            /* This timeout is HW generated */
            if(asic_status & DEC_8190_IRQ_TIMEOUT)
            {
#ifdef VP8HWTIMEOUT_ASSERT
                ASSERT(0);
#endif
                DEBUG_PRINT(("IRQ: HW TIMEOUT\n"));
            }
            else
            {
                DEBUG_PRINT(("IRQ: STREAM ERROR\n"));
            }

            /* keyframe -> all mbs concealed */
            if (pDecCont->decoder.keyFrame ||
                (asic_status & DEC_8190_IRQ_TIMEOUT) ||
                (asic_status & DEC_8190_IRQ_ABORT))
            {
                pDecCont->concealStartMbX = 0;
                pDecCont->concealStartMbY = 0;
                concealEverything = 1;
            }
            else
            {
                /* concealment start point read from sw/hw registers */
                pDecCont->concealStartMbX =
                    GetDecRegister(pDecCont->vp8Regs, HWIF_STARTMB_X);
                pDecCont->concealStartMbY =
                    GetDecRegister(pDecCont->vp8Regs, HWIF_STARTMB_Y);
                /* error in control partition -> conceal all mbs from start point
                 * onwards, otherwise only intra mbs get concealed */
                concealEverything = !(asic_status & DEC_8190_IRQ_ASO);
            }

            if (pDecCont->sliceHeight && pDecCont->intraOnly)
            {
                pDecCont->sliceConcealment = 1;
            }

            /* PP has to run again for the concealed picture */
            if(pDecCont->pp.ppInstance != NULL && pDecCont->pp.decPpIf.usePipeline)
            {
                /* concealed current, i.e. last  ref to PP */
                TRACE_PP_CTRL
                    ("VP8DecDecode: Concealed picture, PP should run again\n");
                pDecCont->pp.decPpIf.ppStatus = DECPP_RUNNING;
            }

            /* HW error concealment not used if
             * 1) previous frame was key frame (no ref mvs available) AND
             * 2) whole control partition corrupted (no current mvs available) */
            if (pDecCont->hwEcSupport &&
                (!pDecCont->prevIsKey || !concealEverything ||
                 pDecCont->concealStartMbY || pDecCont->concealStartMbX))
                vp8hwdErrorConceal(pDecCont, pInput->streamBusAddress,
                    concealEverything);
            else /* normal picture freeze */
                errorConcealment = 1;
        }
        else
        {
            asic_status &= ~DEC_8190_IRQ_ERROR;
            asic_status &= ~DEC_8190_IRQ_TIMEOUT;
            asic_status &= ~DEC_8190_IRQ_ASO;
            asic_status |= DEC_8190_IRQ_RDY;
            errorConcealment = 0;
        }
    }
    else if(asic_status & DEC_8190_IRQ_RDY)
    {
    }
    else if (asic_status & DEC_8190_IRQ_SLICE)
    {
    }
    else
    {
        ASSERT(0);
    }

    if(asic_status & DEC_8190_IRQ_RDY)
    {
        DEBUG_PRINT(("IRQ: PICTURE RDY\n"));

        if (pDecCont->decoder.keyFrame)
        {
            pDecCont->pictureBroken = 0;
            pDecCont->forceIntraFreeze = 0;
        }

        if (pDecCont->sliceHeight)
        {
            pDecCont->outputRows = pAsicBuff->height -  pDecCont->totDecodedRows*16;
            /* Below code not needed; slice mode always disables loop-filter -->
             * output 16 rows multiple */
            /*
            if (pDecCont->totDecodedRows)
                pDecCont->outputRows += 8;
                */
            return VP8DEC_PIC_DECODED;
        }

    }
    else if (asic_status & DEC_8190_IRQ_SLICE)
    {
        pDecCont->decStat = VP8DEC_MIDDLE_OF_PIC;

        pDecCont->outputRows = pDecCont->sliceHeight * 16;
        /* Below code not needed; slice mode always disables loop-filter -->
         * output 16 rows multiple */
        /*if (!pDecCont->totDecodedRows)
            pDecCont->outputRows -= 8;*/

        pDecCont->totDecodedRows += pDecCont->sliceHeight;

        return VP8DEC_SLICE_RDY;
    }

    if(pDecCont->intraOnly != HANTRO_TRUE)
        VP8HwdUpdateRefs(pDecCont, errorConcealment);

    /* find first free buffer and use it as next output */
    if (!errorConcealment || pDecCont->intraOnly)
    {

        pAsicBuff->prevOutBuffer = pAsicBuff->outBuffer;
        pAsicBuff->prevOutBufferI = pAsicBuff->outBufferI;
        pAsicBuff->outBuffer = NULL;

        /* If WebP, we will be recycling only one buffer and no update is
         * necessary. */
        if (!pDecCont->intraOnly)
        {
            VP8HwdBufferQueueRemoveRef(pDecCont->bq, pAsicBuff->outBufferI);
            pAsicBuff->outBufferI = VP8HwdBufferQueueGetBuffer(pDecCont->bq);
        }
        pAsicBuff->outBuffer = &pAsicBuff->pictures[pAsicBuff->outBufferI];
        ASSERT(pAsicBuff->outBuffer != NULL);
    }
    else
    {
        pDecCont->refToOut = 1;
        pDecCont->pictureBroken = 1;
        if (!pDecCont->picNumber)
        {
            (void) DWLmemset( GetPrevRef(pDecCont)->virtualAddress, 128,
                pAsicBuff->width * pAsicBuff->height * 3 / 2);
        }
    }

    pDecCont->picNumber++;
    if (pDecCont->decoder.showFrame)
        pDecCont->outCount++;

    DEC_API_TRC("VP8DecDecode# VP8DEC_PIC_DECODED\n");
    return VP8DEC_PIC_DECODED;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecNextPicture
    Description     :
    Return type     : VP8DecRet
    Argument        : VP8DecInst decInst
    Argument        : VP8DecPicture * pOutput
    Argument        : u32 endOfStream
------------------------------------------------------------------------------*/
VP8DecRet VP8DecNextPicture(VP8DecInst decInst,
                            VP8DecPicture * pOutput, u32 endOfStream)
{
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;
    u32 picForOutput = 0;

    DEC_API_TRC("VP8DecNextPicture#\n");

    if(decInst == NULL || pOutput == NULL)
    {
        DEC_API_TRC("VP8DecNextPicture# ERROR: decInst or pOutput is NULL\n");
        return (VP8DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("VP8DecNextPicture# ERROR: Decoder not initialized\n");
        return (VP8DEC_NOT_INITIALIZED);
    }

    if (!pDecCont->outCount && !pDecCont->outputRows)
        return VP8DEC_OK;

    if(pDecCont->sliceConcealment)
        return (VP8DEC_OK);

    /* slice for output */
    if (pDecCont->outputRows)
    {
        pOutput->numSliceRows = pDecCont->outputRows;

        if (pDecCont->userMem)
        {
            pOutput->pOutputFrame = pAsicBuff->userMem.pPicBufferY[0];
            pOutput->pOutputFrameC = pAsicBuff->userMem.pPicBufferC[0];
            pOutput->outputFrameBusAddress =
                pAsicBuff->userMem.picBufferBusAddrY[0];
            pOutput->outputFrameBusAddressC =
                pAsicBuff->userMem.picBufferBusAddrC[0];
        }
        else
        {
            u32 offset = 16 * (pDecCont->sliceHeight + 1) * pAsicBuff->width;
            pOutput->pOutputFrame = pAsicBuff->pictures[0].virtualAddress;
            pOutput->outputFrameBusAddress =
                pAsicBuff->pictures[0].busAddress;

            if(pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
            {
                pOutput->pOutputFrameC = pAsicBuff->picturesC[0].virtualAddress;
                pOutput->outputFrameBusAddressC =
                    pAsicBuff->picturesC[0].busAddress;
            }
            else
            {
                pOutput->pOutputFrameC =
                    (u32*)((u32)pOutput->pOutputFrame + offset);
                pOutput->outputFrameBusAddressC =
                    pOutput->outputFrameBusAddress + offset;
            }
        }

        pOutput->picId = 0;
        pOutput->isIntraFrame = pDecCont->decoder.keyFrame;
        pOutput->isGoldenFrame = 0;
        pOutput->nbrOfErrMBs = 0;
        pOutput->frameWidth = (pDecCont->width + 15) & ~15;
        pOutput->frameHeight = (pDecCont->height + 15) & ~15;
        pOutput->codedWidth = pDecCont->width;
        pOutput->codedHeight = pDecCont->height;
        pOutput->lumaStride = pAsicBuff->lumaStride ?
            pAsicBuff->lumaStride : pAsicBuff->width;
        pOutput->chromaStride = pAsicBuff->chromaStride ?
            pAsicBuff->chromaStride : pAsicBuff->width;

        pOutput->outputFormat = pDecCont->tiledReferenceEnable ?
            DEC_OUT_FRM_TILED_8X4 : DEC_OUT_FRM_RASTER_SCAN;

        pDecCont->outputRows = 0;

        return (VP8DEC_PIC_RDY);
    }

    pOutput->numSliceRows = 0;

    if (!pDecCont->pp.ppInstance || pDecCont->pp.decPpIf.ppStatus != DECPP_IDLE)
        picForOutput = 1;
    else if (!pDecCont->decoder.refreshLast || endOfStream)
    {
        picForOutput = 1;
        pDecCont->pp.decPpIf.ppStatus = DECPP_RUNNING;
    }
    else if (pDecCont->pp.ppInstance && pDecCont->pendingPicToPp)
    {
        picForOutput = 1;
        pDecCont->pp.decPpIf.ppStatus = DECPP_RUNNING;
    }

    /* TODO: What if intraFreeze and not pipelined pp? */
    if (pDecCont->pp.ppInstance != NULL &&
        pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING)
    {
        DecPpInterface *decPpIf = &pDecCont->pp.decPpIf;
        TRACE_PP_CTRL("VP8DecNextPicture: PP has to run\n");

        decPpIf->usePipeline = 0;

        decPpIf->inwidth = MB_MULTIPLE(pDecCont->width);
        decPpIf->inheight = MB_MULTIPLE(pDecCont->height);
        decPpIf->croppedW = decPpIf->inwidth;
        decPpIf->croppedH = decPpIf->inheight;

        decPpIf->lumaStride = pAsicBuff->lumaStride ?
            pAsicBuff->lumaStride : pAsicBuff->width;
        decPpIf->chromaStride = pAsicBuff->chromaStride ?
            pAsicBuff->chromaStride : pAsicBuff->width;

        /* forward tiled mode */
        decPpIf->tiledInputMode = pDecCont->tiledReferenceEnable;
        decPpIf->progressiveSequence = 1;

        decPpIf->picStruct = DECPP_PIC_FRAME_OR_TOP_FIELD;
        if (pDecCont->userMem)
        {
            decPpIf->inputBusLuma =
                   pAsicBuff->userMem.picBufferBusAddrY[0];
            decPpIf->inputBusChroma =
                   pAsicBuff->userMem.picBufferBusAddrC[0];
        }
        else
        {
            if (pDecCont->decoder.refreshLast || pDecCont->refToOut)
            {
                decPpIf->inputBusLuma = GetPrevRef(pDecCont)->busAddress;
                decPpIf->inputBusChroma =
                    pAsicBuff->picturesC[
                        VP8HwdBufferQueueGetPrevRef(pDecCont->bq)].busAddress;
            }
            else
            {
                decPpIf->inputBusLuma = pAsicBuff->prevOutBuffer->busAddress;
                decPpIf->inputBusChroma =
                    pAsicBuff->picturesC[pAsicBuff->prevOutBufferI].busAddress;
            }

            if(!(pAsicBuff->stridesUsed || pAsicBuff->customBuffers))
            {
                decPpIf->inputBusChroma = decPpIf->inputBusLuma +
                    decPpIf->inwidth * decPpIf->inheight;
            }
        }

        pOutput->outputFrameBusAddress = decPpIf->inputBusLuma;

        decPpIf->littleEndian =
            GetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUT_ENDIAN);
        decPpIf->wordSwap =
            GetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUTSWAP32_E);

        pDecCont->pp.PPDecStart(pDecCont->pp.ppInstance, decPpIf);

        TRACE_PP_CTRL("VP8DecNextPicture: PP wait to be done\n");

        pDecCont->pp.PPDecWaitEnd(pDecCont->pp.ppInstance);
        pDecCont->pp.decPpIf.ppStatus = DECPP_PIC_READY;

        TRACE_PP_CTRL("VP8DecNextPicture: PP Finished\n");
    }

    if (picForOutput)
    {
        const DWLLinearMem_t *outPic = NULL;
        const DWLLinearMem_t *outPicC = NULL;

        /* no pp -> current output (ref if concealed)
         * pipeline -> the same
         * pp stand-alone (non-reference frame) -> current output
         * pp stand-alone  */
        if ( (pDecCont->pp.ppInstance == NULL ||
              pDecCont->pp.decPpIf.usePipeline ||
              (pDecCont->outCount == 1 &&
               !pDecCont->decoder.refreshLast) ) && !pDecCont->refToOut)
        {
            outPic = pAsicBuff->prevOutBuffer;
            outPicC = &pAsicBuff->picturesC[ pAsicBuff->prevOutBufferI ];
        }
        else
        {
            if (pDecCont->pp.ppInstance != NULL)
            {
                pDecCont->combinedModeUsed = 1;
            }
            outPic = GetPrevRef(pDecCont);
            outPicC = &pAsicBuff->picturesC[
                VP8HwdBufferQueueGetPrevRef(pDecCont->bq)];

        }

        pDecCont->outCount--;

        pOutput->lumaStride = pAsicBuff->lumaStride ?
            pAsicBuff->lumaStride : pAsicBuff->width;
        pOutput->chromaStride = pAsicBuff->chromaStride ?
            pAsicBuff->chromaStride : pAsicBuff->width;

        if (pDecCont->userMem)
        {
            pOutput->pOutputFrame = pAsicBuff->userMem.pPicBufferY[0];
            pOutput->outputFrameBusAddress =
                pAsicBuff->userMem.picBufferBusAddrY[0];
            pOutput->pOutputFrameC = pAsicBuff->userMem.pPicBufferC[0];
            pOutput->outputFrameBusAddressC =
                pAsicBuff->userMem.picBufferBusAddrC[0];

        }
        else
        {
            pOutput->pOutputFrame = outPic->virtualAddress;
            pOutput->outputFrameBusAddress = outPic->busAddress;
            if(pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
            {
                pOutput->pOutputFrameC = outPicC->virtualAddress;
                pOutput->outputFrameBusAddressC = outPicC->busAddress;
            }
            else
            {
                u32 chromaBufOffset = pAsicBuff->width * pAsicBuff->height;
                pOutput->pOutputFrameC = pOutput->pOutputFrame +
                    chromaBufOffset / 4;
                pOutput->outputFrameBusAddressC = pOutput->outputFrameBusAddress +
                    chromaBufOffset;
            }
        }
        pOutput->picId = 0;
        pOutput->isIntraFrame = pDecCont->decoder.keyFrame;
        pOutput->isGoldenFrame = 0;
        pOutput->nbrOfErrMBs = 0;

        pOutput->frameWidth = (pDecCont->width + 15) & ~15;
        pOutput->frameHeight = (pDecCont->height + 15) & ~15;
        pOutput->codedWidth = pDecCont->width;
        pOutput->codedHeight = pDecCont->height;
        pOutput->outputFormat = pDecCont->tiledReferenceEnable ?
            DEC_OUT_FRM_TILED_8X4 : DEC_OUT_FRM_RASTER_SCAN;

        DEC_API_TRC("VP8DecNextPicture# VP8DEC_PIC_RDY\n");

        pDecCont->pp.decPpIf.ppStatus = DECPP_IDLE;

        return (VP8DEC_PIC_RDY);
    }

    DEC_API_TRC("VP8DecNextPicture# VP8DEC_OK\n");
    return (VP8DEC_OK);

}

u32 vp8hwdCheckSupport( VP8DecContainer_t *pDecCont )
{

    DWLHwConfig_t hwConfig;

    DWLReadAsicConfig(&hwConfig);

    if ( (pDecCont->asicBuff->width > hwConfig.maxDecPicWidth) ||
         (pDecCont->asicBuff->width < MIN_PIC_WIDTH) ||
         (pDecCont->asicBuff->height < MIN_PIC_HEIGHT) ||
         (pDecCont->asicBuff->width*pDecCont->asicBuff->height > MAX_PIC_SIZE) )
    {
        /* check if webp support */
        if (pDecCont->intraOnly && hwConfig.webpSupport &&
            pDecCont->asicBuff->width <= MAX_SNAPSHOT_WIDTH*16 &&
            pDecCont->asicBuff->height <= MAX_SNAPSHOT_HEIGHT*16)
        {
            return HANTRO_OK;
        }
        else 
        {
            return HANTRO_NOK;
        }
    }

    return HANTRO_OK;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecPeek
    Description     : 
    Return type     : VP8DecRet 
    Argument        : VP8DecInst decInst
------------------------------------------------------------------------------*/
VP8DecRet VP8DecPeek(VP8DecInst decInst, VP8DecPicture * pOutput)
{
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;
    const DWLLinearMem_t *outPic = NULL;
    const DWLLinearMem_t *outPicC = NULL;        

    DEC_API_TRC("VP8DecPeek#\n");
    
    if(decInst == NULL || pOutput == NULL)
    {
        DEC_API_TRC("VP8DecPeek# ERROR: decInst or pOutput is NULL\n");
        return (VP8DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("VP8DecPeek# ERROR: Decoder not initialized\n");
        return (VP8DEC_NOT_INITIALIZED);
    }

    /* Don't allow peek for webp */
    if(pDecCont->intraOnly)
    {
        DWLmemset(pOutput, 0, sizeof(VP8DecPicture));
        return VP8DEC_OK;
    }

    if (!pDecCont->outCount)
    {
        DWLmemset(pOutput, 0, sizeof(VP8DecPicture));
        return VP8DEC_OK;
    }

    outPic = pAsicBuff->prevOutBuffer;
    outPicC = &pAsicBuff->picturesC[ pAsicBuff->prevOutBufferI ];
    
    pOutput->pOutputFrame = outPic->virtualAddress;
    pOutput->outputFrameBusAddress = outPic->busAddress;
    if(pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
    {
        pOutput->pOutputFrameC = outPicC->virtualAddress;
        pOutput->outputFrameBusAddressC = outPicC->busAddress;
    }
    else
    {
        u32 chromaBufOffset = pAsicBuff->width * pAsicBuff->height;
        pOutput->pOutputFrameC = pOutput->pOutputFrame +
            chromaBufOffset / 4;
        pOutput->outputFrameBusAddressC = pOutput->outputFrameBusAddress +
            chromaBufOffset;
    }

    pOutput->picId = 0;
    pOutput->isIntraFrame = pDecCont->decoder.keyFrame;
    pOutput->isGoldenFrame = 0;
    pOutput->nbrOfErrMBs = 0;

    pOutput->frameWidth = (pDecCont->width + 15) & ~15;
    pOutput->frameHeight = (pDecCont->height + 15) & ~15;
    pOutput->codedWidth = pDecCont->width;
    pOutput->codedHeight = pDecCont->height;
    pOutput->lumaStride = pAsicBuff->lumaStride ? 
        pAsicBuff->lumaStride : pAsicBuff->width;
    pOutput->chromaStride = pAsicBuff->chromaStride ? 
        pAsicBuff->chromaStride : pAsicBuff->width;

    return (VP8DEC_PIC_RDY);


}

void vp8hwdMCFreeze(VP8DecContainer_t *pDecCont)
{
    /*TODO (mheikkinen) Error handling/concealment is still under construction.*/
    /* TODO Output reference handling */
    VP8HwdBufferQueueRemoveRef(pDecCont->bq, pDecCont->asicBuff->outBufferI);
    pDecCont->streamConsumedCallback((u8*)pDecCont->pStream, pDecCont->pUserData);
}

/*------------------------------------------------------------------------------
    Function name   : vp8hwdFreeze
    Description     : 
    Return type     : 
    Argument        : 
------------------------------------------------------------------------------*/
void vp8hwdFreeze(VP8DecContainer_t *pDecCont)
{
    /* for multicore */
    if(pDecCont->streamConsumedCallback)
    {
        vp8hwdMCFreeze(pDecCont);
        return;
    }
    /* Skip */
    pDecCont->picNumber++;
    pDecCont->refToOut = 1;
    if (pDecCont->decoder.showFrame)
        pDecCont->outCount++;

    if(pDecCont->pp.ppInstance != NULL)
    {
        /* last ref to PP */
        pDecCont->pp.decPpIf.usePipeline = 0;
        pDecCont->pp.decPpIf.ppStatus = DECPP_RUNNING;
    }

    /* Rollback entropy probabilities if refresh is not set */
    if (pDecCont->decoder.probsDecoded &&
        pDecCont->decoder.refreshEntropyProbs == HANTRO_FALSE)
    {
        DWLmemcpy( &pDecCont->decoder.entropy, &pDecCont->decoder.entropyLast, 
                   sizeof(vp8EntropyProbs_t));
        DWLmemcpy( pDecCont->decoder.vp7ScanOrder, pDecCont->decoder.vp7PrevScanOrder, 
                   sizeof(pDecCont->decoder.vp7ScanOrder));
    }
    /* lost accumulated coeff prob updates -> force video freeze until next
     * keyframe received */
    else if (pDecCont->hwEcSupport && pDecCont->probRefreshDetected)
    {
        pDecCont->forceIntraFreeze = 1;
    }

    pDecCont->pictureBroken = 1;

    /* reset mv memory to not to use too old mvs in extrapolation */
    if (pDecCont->asicBuff->mvs[pDecCont->asicBuff->mvsRef].virtualAddress)
        DWLmemset(
            pDecCont->asicBuff->mvs[pDecCont->asicBuff->mvsRef].virtualAddress,
            0, pDecCont->width * pDecCont->height / 256 * 16 * sizeof(u32));


}

/*------------------------------------------------------------------------------
    Function name   : CheckBitstreamWorkaround
    Description     : Check if we need a workaround for a rare bug.
    Return type     : 
    Argument        : 
------------------------------------------------------------------------------*/
u32 CheckBitstreamWorkaround(vp8Decoder_t* dec)
{
    /* TODO: HW ID check, P pic stuff */
    if( dec->segmentationMapUpdate &&
        dec->coeffSkipMode == 0 &&
        dec->keyFrame)
    {
        return 1;
    }

    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : DoBitstreamWorkaround
    Description     : Perform workaround for bug.
    Return type     : 
    Argument        : 
------------------------------------------------------------------------------*/
#if 0
void DoBitstreamWorkaround(vp8Decoder_t* dec, DecAsicBuffers_t *pAsicBuff, vpBoolCoder_t*bc)
{
    /* TODO in entirety */
}
#endif

/* TODO(mheikkinen) Work in progress */
void ConcealRefAvailability(u32 * output, u32 height, u32 width)
{
  u8 * pRefStatus = (u8 *)(output +
    (height * width * 3)/2);

  pRefStatus[1] = height & 0xFFU;
  pRefStatus[0] = (height >> 8) & 0xFFU;
}

/*------------------------------------------------------------------------------
    Function name   : vp8hwdErrorConceal
    Description     : 
    Return type     : 
    Argument        : 
------------------------------------------------------------------------------*/
void vp8hwdErrorConceal(VP8DecContainer_t *pDecCont, u32 busAddress,
    u32 concealEverything)
{
    /* force keyframes processed like normal frames (mvs extrapolated for
     * all mbs) */
    if (pDecCont->decoder.keyFrame)
    {
        pDecCont->decoder.keyFrame = 0;
    }

    vp8hwdEc(&pDecCont->ec,
        pDecCont->asicBuff->mvs[pDecCont->asicBuff->mvsRef].virtualAddress,
        pDecCont->asicBuff->mvs[pDecCont->asicBuff->mvsCurr].virtualAddress,
        pDecCont->concealStartMbY * pDecCont->width/16 + pDecCont->concealStartMbX,
        concealEverything);

    pDecCont->conceal = 1;
    if (concealEverything)
        pDecCont->concealStartMbX = pDecCont->concealStartMbY = 0;
    VP8HwdAsicInitPicture(pDecCont);
    VP8HwdAsicStrmPosUpdate(pDecCont, busAddress);

    ConcealRefAvailability(pDecCont->asicBuff->outBuffer->virtualAddress,
    MB_MULTIPLE(pDecCont->asicBuff->height),  MB_MULTIPLE(pDecCont->asicBuff->width));

    pDecCont->conceal = 0;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecSetPictureBuffers
    Description     : 
    Return type     : 
    Argument        : 
------------------------------------------------------------------------------*/
VP8DecRet VP8DecSetPictureBuffers( VP8DecInst decInst,
                                   VP8DecPictureBufferProperties * pPbp )
{

    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;
    u32 i;
    u32 ok;
#if DEC_X170_REFBU_WIDTH == 64
    const u32 maxStride = 1<<18;
#else 
    const u32 maxStride = 1<<17;
#endif /* DEC_X170_REFBU_WIDTH */
    u32 lumaStridePow2 = 0;
    u32 chromaStridePow2 = 0;

    if(!decInst || !pPbp)
    {
        DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: NULL parameter");
        return VP8DEC_PARAM_ERROR;
    }

    /* Allow this only at stream start! */
    if ( ((pDecCont->decStat != VP8DEC_NEW_HEADERS) && 
          (pDecCont->decStat != VP8DEC_INITIALIZED)) ||
         (pDecCont->picNumber > 0))
    {
        DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: Setup allowed at stream"\
                    " start only!");
        return VP8DEC_PARAM_ERROR;
    }

    if( !pDecCont->strideSupport )
    {
        DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: Not supported");
        return VP8DEC_FORMAT_NOT_SUPPORTED;
    }

    /* Tiled mode and custom strides not supported yet */
    if( pDecCont->tiledModeSupport &&
        (pPbp->lumaStride || pPbp->chromaStride))
    {
        DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: tiled mode and picture "\
                    "buffer properties conflict");
        return VP8DEC_PARAM_ERROR;
    }

    /* Strides must be 2^N for some N>=4 */
    if(pPbp->lumaStride || pPbp->chromaStride)
    {
        ok = 0;
        for ( i = 10 ; i < 32 ; ++i )
        {
            if(pPbp->lumaStride == (u32)(1<<i))
            {
                lumaStridePow2 = i;
                ok = 1;
                break;
            }
        }
        if(!ok)
        {
            DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: luma stride must be a "\
                        "power of 2");
            return VP8DEC_PARAM_ERROR;
        }

        ok = 0;
        for ( i = 10 ; i < 32 ; ++i )
        {
            if(pPbp->chromaStride == (u32)(1<<i))
            {
                chromaStridePow2 = i;
                ok = 1;
                break;
            }
        }
        if(!ok)
        {
            DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: luma stride must be a "\
                        "power of 2");
            return VP8DEC_PARAM_ERROR;
        }
    }

    /* Max luma stride check */
    if(pPbp->lumaStride > maxStride)
    {
        DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: luma stride exceeds "\
                    "maximum");
        return VP8DEC_PARAM_ERROR;
    }

    /* Max chroma stride check */
    if(pPbp->chromaStride > maxStride)
    {
        DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: chroma stride exceeds "\
                    "maximum");
        return VP8DEC_PARAM_ERROR;
    }

    pDecCont->asicBuff->lumaStride = pPbp->lumaStride;
    pDecCont->asicBuff->chromaStride = pPbp->chromaStride;
    pDecCont->asicBuff->lumaStridePow2 = lumaStridePow2;
    pDecCont->asicBuff->chromaStridePow2 = chromaStridePow2;
    pDecCont->asicBuff->stridesUsed = 0;
    pDecCont->asicBuff->customBuffers = 0;
    if( pDecCont->asicBuff->lumaStride ||
        pDecCont->asicBuff->chromaStride )
    {
        pDecCont->asicBuff->stridesUsed = 1;
    }

    /* Check custom buffers */
    if( pPbp->numBuffers )
    {
        userMem_t * userMem = &pDecCont->asicBuff->userMem;
        u32 numBuffers = pPbp->numBuffers;

        if( pDecCont->intraOnly )
        {
            DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: custom buffers not "\
                        "applicable for WebP");
            return VP8DEC_PARAM_ERROR;
        }

        /* Check buffer arrays */
        if( !pPbp->pPicBufferY || 
            !pPbp->picBufferBusAddressY ||
            !pPbp->pPicBufferC ||
            !pPbp->picBufferBusAddressC )
        {
            DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: Invalid buffer "\
                        "array(s)");
            return VP8DEC_PARAM_ERROR;
        }

        /* As of now, minimum 5 buffers in multicore, 4 in single core legacy. */
        /* TODO(mheikkinen) Single core should work with 4 buffers
        if( numBuffers < 4 + (pDecCont->numCores > 1) ? 1 : 0 )*/
        if( numBuffers < 5)
        {
            DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: Not enough buffers. " \
                        "Minimum requirement is 5 buffers.");
            return VP8DEC_PARAM_ERROR;
        }

        /* Limit upper boundary to 16 */
        if( numBuffers > VP8DEC_MAX_PIC_BUFFERS )
            numBuffers = VP8DEC_MAX_PIC_BUFFERS;

        /* Check all buffers */
        for( i = 0 ; i < numBuffers ; ++i )
        {
            if ((pPbp->pPicBufferY[i] != NULL && pPbp->picBufferBusAddressY[i] == 0) ||
                (pPbp->pPicBufferY[i] == NULL && pPbp->picBufferBusAddressY[i] != 0) ||
                (pPbp->pPicBufferC[i] != NULL && pPbp->picBufferBusAddressC[i] == 0) ||
                (pPbp->pPicBufferC[i] == NULL && pPbp->picBufferBusAddressC[i] != 0) ||
                (pPbp->pPicBufferY[i] == NULL && pPbp->pPicBufferC[i] != 0) ||
                (pPbp->pPicBufferY[i] != NULL && pPbp->pPicBufferC[i] == 0))
            {
                DEC_API_TRC("VP8DecSetPictureBuffers# ERROR: Invalid buffer "\
                            "supplied");
                return VP8DEC_PARAM_ERROR;
            }
        }

        /* Seems ok, update internal descriptors */
        for( i = 0 ; i < numBuffers ; ++i )
        {
            userMem->pPicBufferY[i] = pPbp->pPicBufferY[i];
            userMem->pPicBufferC[i] = pPbp->pPicBufferC[i];
            userMem->picBufferBusAddrY[i] = pPbp->picBufferBusAddressY[i];
            userMem->picBufferBusAddrC[i] = pPbp->picBufferBusAddressC[i];
        }
        pDecCont->numBuffers = numBuffers;
        pDecCont->asicBuff->customBuffers = 1;

    }

    DEC_API_TRC("VP8DecSetPictureBuffers# OK\n");
    return (VP8DEC_OK);

}

static DWLLinearMem_t* GetPrevRef(VP8DecContainer_t *pDecCont)
{
    return pDecCont->asicBuff->pictures +
        VP8HwdBufferQueueGetPrevRef(pDecCont->bq);
}

u32 VP8DecShowFrame(VP8DecInst decInst)
{
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;

    return pDecCont->decoder.showFrame;
}

/* Stub MC API */
void VP8MCSetHwRdyCallback(VP8DecContainer_t  *pDecCont) {};
