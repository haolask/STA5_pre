/*
 * (C) COPYRIGHT 2012 HANTRO PRODUCTS
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
-  Description : Multicore VP8 API
-
------------------------------------------------------------------------------*/

#include "vp8decapi.h"

#include "basetype.h"
#include "dwl.h"
#include "dwlthread.h"
#include "vp8decmc_internals.h"
#include "vp8hwd_asic.h"
#include "vp8hwd_container.h"
#include "vp8hwd_debug.h"
#include "vp8hwd_buffer_queue.h"


#ifndef TRACE_PP_CTRL
#define TRACE_PP_CTRL(...)          do{}while(0)
#else
#include <stdio.h>
#undef TRACE_PP_CTRL
#define TRACE_PP_CTRL(...)          printf(__VA_ARGS__)
#endif

#ifdef VP8DEC_TRACE
#define DEC_API_TRC(str)    VP8DecTrace(str)
#else
#define DEC_API_TRC(str)    do{}while(0)
#endif

#define EOS_MARKER (-1)

static i32 FindIndex(VP8DecContainer_t* pDecCont, const u32* address);
static i32 NextOutput(VP8DecContainer_t *pDecCont);

/*------------------------------------------------------------------------------
    Function name   : VP8DecMCGetCoreCount
    Description     :
    Return type     : VP8DecRet
    Argument        : VP8DecInst decInst
------------------------------------------------------------------------------*/
u32 VP8DecMCGetCoreCount(void)
{
    ASSERT(0);
    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecMCInit
    Description     :
    Return type     : VP8DecRet
    Argument        : VP8DecInst decInst
------------------------------------------------------------------------------*/
VP8DecRet VP8DecMCInit(VP8DecInst * pDecInst, VP8DecMCConfig *pMCInitCfg)
{
    *pDecInst = NULL;   /* return NULL instance for any error */
    if (pMCInitCfg == NULL)
    {
        return VP8DEC_PARAM_ERROR;
    }

    /* Call the common initialization first. */
    VP8DecRet ret = VP8DecInit(pDecInst,
                               VP8DEC_VP8,
                               1,
                               5,
                               DEC_REF_FRM_RASTER_SCAN);
    if (ret != VP8DEC_OK)
    {
        return ret;
    }
    /* Do the multicore specifics. */
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) *pDecInst;
    if (pMCInitCfg->streamConsumedCallback == NULL)
        return VP8DEC_PARAM_ERROR;
    if (fifo_init(VP8DEC_MAX_PIC_BUFFERS, &pDecCont->fifoOut) != FIFO_OK)
        return VP8DEC_MEMFAIL;

    if (fifo_init(VP8DEC_MAX_PIC_BUFFERS, &pDecCont->fifoDisplay) != FIFO_OK)
        return VP8DEC_MEMFAIL;

    pDecCont->numCores = DWLReadAsicCoreCount();
    /* Enable multicore in the VP8 registers. */
    if(pDecCont->numCores>1)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_MULTICORE_E,
                   pDecCont->intraOnly ? 0 : 1);
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_WRITESTAT_E,
                   pDecCont->intraOnly ? 0 : 1);
    }
    pDecCont->streamConsumedCallback = pMCInitCfg->streamConsumedCallback;

    return ret;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecMCDecode
    Description     :
    Return type     : VP8DecRet
    Argument        : VP8DecInst decInst
    Argument        : const VP8DecInput *pInput
    Argument        : VP8DecOutput *pOutput
------------------------------------------------------------------------------*/
VP8DecRet VP8DecMCDecode(VP8DecInst decInst,
                         const VP8DecInput *pInput,
                         VP8DecOutput *pOutput)
{
    VP8DecRet ret;
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;

    DEC_API_TRC("VP8DecMCDecode#\n");

    /* Check that function input parameters are valid */
    if(pInput == NULL || pOutput == NULL || decInst == NULL)
    {
        DEC_API_TRC("VP8DecMCDecode# ERROR: NULL arg(s)\n");
        return (VP8DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("VP8DecMCDecode# ERROR: Decoder not initialized\n");
        return (VP8DEC_NOT_INITIALIZED);
    }

    /* Currently we just call the single core version, but we may change
     * our mind in the future. Do not call directly VP8DecDecode() in any
     * multicore application */
    ret = VP8DecDecode(decInst, pInput, pOutput);

    return ret;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecMCNextPicture
    Description     :
    Return type     : VP8DecRet
    Argument        : VP8DecInst decInst
                      VP8DecPicture pOutput
                      u32 endOfStream
------------------------------------------------------------------------------*/
VP8DecRet VP8DecMCNextPicture(VP8DecInst decInst,
                              VP8DecPicture * pOutput)
{
    i32 i;
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *) decInst;
    if(decInst == NULL || pOutput == NULL)
    {
        DEC_API_TRC("VP8DecNextPicture# ERROR: decInst or pOutput is NULL\n");
        return VP8DEC_PARAM_ERROR;
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("VP8DecNextPicture# ERROR: Decoder not initialized\n");
        return VP8DEC_NOT_INITIALIZED;
    }

    /*  NextOutput will block until there is an output. */
    i = NextOutput(pDecCont);
    if (i == EOS_MARKER)
    {
        DEC_API_TRC("VP8DecNextPicture# H264DEC_END_OF_STREAM\n");
        return VP8DEC_END_OF_STREAM;
    }

    ASSERT(i >= 0 && (u32)i < pDecCont->numBuffers);
    pDecCont->outCount--;
    *pOutput = pDecCont->asicBuff->pictureInfo[i];
    pOutput->picId = pDecCont->picNumber++;

    DEC_API_TRC("VP8DecNextPicture# VP8DEC_PIC_RDY\n");
    return VP8DEC_PIC_RDY;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecPictureConsumed
    Description     :
    Return type     : VP8DecRet
    Argument        : VP8DecInst decInst
------------------------------------------------------------------------------*/
VP8DecRet VP8DecMCPictureConsumed(VP8DecInst decInst,
                                  const VP8DecPicture * pPicture)
{
    if (decInst == NULL || pPicture == NULL)
    {
        return VP8DEC_PARAM_ERROR;
    }
    /* Remove the reference to the buffer. */
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *)decInst;
    VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                               FindIndex(pDecCont, pPicture->pOutputFrame));
    DEC_API_TRC("VP8DecMCPictureConsumed# VP8DEC_OK\n");
    return VP8DEC_OK;
}

/*------------------------------------------------------------------------------
    Function name   : VP8DecMCEndOfStream
    Description     : Used for signalling stream end from the input thread
    Return type     : VP8DecRet
    Argument        : VP8DecInst decInst
------------------------------------------------------------------------------*/
VP8DecRet VP8DecMCEndOfStream(VP8DecInst decInst)
{
    if (decInst == NULL)
    {
        return VP8DEC_PARAM_ERROR;
    }
    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *)decInst;

    /* Don't do end of stream twice. This is not thread-safe, so it must be
     * called from the single input thread that is also used to call
     * VP8DecDecode. */
    if (pDecCont->decStat == VP8DEC_END_OF_STREAM) {
        ASSERT(0);  /* Let the ASSERT kill the stuff in debug mode */
        return VP8DEC_END_OF_STREAM;
    }

    /* If buffer queue has been already initialized, we can use it to track
     * pending cores and outputs safely. */
    if (pDecCont->bq)
    {
        /* if the references and queue were already flushed, cannot do it again. */
        if(pDecCont->asicBuff->outBufferI != VP8_UNDEFINED_BUFFER)
        {
            /* Workaround for ref counting since this buffer is never used. */
            VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                                       pDecCont->asicBuff->outBufferI);
            pDecCont->asicBuff->outBufferI = VP8_UNDEFINED_BUFFER;
            VP8HwdBufferQueueRemoveRef(
                pDecCont->bq, VP8HwdBufferQueueGetPrevRef(pDecCont->bq));
            VP8HwdBufferQueueRemoveRef(
                pDecCont->bq, VP8HwdBufferQueueGetGoldenRef(pDecCont->bq));
            VP8HwdBufferQueueRemoveRef(
                pDecCont->bq, VP8HwdBufferQueueGetAltRef(pDecCont->bq));
            VP8HwdBufferQueueWaitPending(pDecCont->bq);
        }
    }

    pDecCont->decStat = VP8DEC_END_OF_STREAM;
    fifo_push(pDecCont->fifoOut, EOS_MARKER);
    return VP8DEC_OK;
}

/*------------------------------------------------------------------------------
    Function name   : NextOutput
    Description     : Find next picture in output order. If a fifo (fifoDisplay)
                      is used store outputs not yet ready display.
    Return type     : i32 - index to picture buffer
    Argument        : VP8DecContainer_t *
------------------------------------------------------------------------------*/
i32 NextOutput(VP8DecContainer_t *pDecCont)
{
    i32 i;
    u32 j;
    i32 outputI = -1;
    u32 size;


    size = fifo_count(pDecCont->fifoDisplay);

    /* If there are pictures in the display reordering buffer, check them
     * first to see if our next output is there. */
    for(j=0; j<size; j++)
    {
        fifo_pop(pDecCont->fifoDisplay, &i);

        if(pDecCont->asicBuff->displayIndex[i] == pDecCont->picNumber)
        {
            /*  fifoDisplay had the right output. */
            outputI = i;
            break;
        }
        else
        {
            fifo_push(pDecCont->fifoDisplay, i);
        }
    }

    /* Look for output in decode ordered outFifo. */
    while(outputI < 0)
    {
        /* Blocks until next output is available */
        fifo_pop(pDecCont->fifoOut, &i);
        if (i == EOS_MARKER)
            return i;

        if(pDecCont->asicBuff->displayIndex[i] == pDecCont->picNumber)
        {
            /*  fifoOut had the right output. */
            outputI = i;
        }
        else
        {
            /* Until we get the next picture in display order, push the outputs
            * to the display reordering fifo */
            fifo_push(pDecCont->fifoDisplay, i);
        }
    }

    return outputI;
}

#define HWIF_DEC_IRQ_REG      4

void VP8DecIRQCallbackFn(void* arg, i32 core_id)
{
    u32 decRegs[DEC_X170_REGISTERS];

    VP8DecContainer_t *pDecCont = (VP8DecContainer_t *)arg;
    HwRdyCallbackArg info;
    const void *dwl;

    u32 core_status;
    i32 i;

    ASSERT(pDecCont != NULL);
    ASSERT(core_id < MAX_ASIC_CORES);

    info = pDecCont->hwRdyCallbackArg[core_id];

    dwl = pDecCont->dwl;

    /* read all hw regs */
    for (i = 1; i < DEC_X170_REGISTERS; i++)
    {
        decRegs[i] = DWLReadReg(dwl, core_id, i * 4);
    }

    /* React to the HW return value */
    core_status = GetDecRegister(decRegs, HWIF_DEC_IRQ_STAT);

    /* check if DEC_RDY, all other status are errors */
    if (core_status != DEC_8190_IRQ_RDY)
    {
        /* TODO(vmr): Handle unrecoverable system errors properly. */

        /* reset HW if still enabled */
        if (core_status & 0x01)
            DWLDisableHW(dwl, info.coreID, 0x04, 0);
        if(pDecCont->numCores > 1)
        {
            /* Fake reference availability to allow pending cores to go. */
            info.pRefStatus[0] = 0xFF;
            info.pRefStatus[1] = 0xFF;
        }
    }
    /* Remove references this decoded picture needed. */
    VP8HwdBufferQueueRemoveRef(info.bq, info.indexP);
    VP8HwdBufferQueueRemoveRef(info.bq, info.indexA);
    VP8HwdBufferQueueRemoveRef(info.bq, info.indexG);
    DWLReleaseHw(dwl, info.coreID);

    /* Let app know we're done with the buffer. */
    ASSERT(info.streamConsumedCallback);
    info.streamConsumedCallback((u8*)info.pStream, info.pUserData);

    /* If frame is to be outputted, fill in the information before pushing to
     * output fifo. */
    info.pic.nbrOfErrMBs = 0; /* TODO(vmr): Fill correctly. */
    pDecCont->asicBuff->pictureInfo[info.index] = info.pic;
    if(info.showFrame)
    {
        fifo_push(info.fifoOut, info.index);
    }
}


void VP8MCSetHwRdyCallback(VP8DecContainer_t  *pDecCont)
{
    HwRdyCallbackArg *args = &pDecCont->hwRdyCallbackArg[pDecCont->coreID];
    args->pDecCont = pDecCont;
    args->coreID = pDecCont->coreID;
    args->bq = pDecCont->bq;
    args->index = pDecCont->asicBuff->outBufferI;
    args->indexP = VP8HwdBufferQueueGetPrevRef(pDecCont->bq);
    args->indexA = VP8HwdBufferQueueGetAltRef(pDecCont->bq);
    args->indexG = VP8HwdBufferQueueGetGoldenRef(pDecCont->bq);
    args->pStream = pDecCont->pStream;
    args->pUserData = pDecCont->pUserData;
    args->streamConsumedCallback = pDecCont->streamConsumedCallback;
    args->fifoOut = pDecCont->fifoOut;
    if (pDecCont->decoder.showFrame)
    {
        pDecCont->asicBuff->displayIndex[args->index] =
            pDecCont->displayNumber++;
    }
    args->showFrame = pDecCont->decoder.showFrame;
    args->pRefStatus = (u8 *)VP8HwdRefStatusAddress(pDecCont);
    /* Fill in the picture information for everything we know. */
    args->pic.isIntraFrame = pDecCont->decoder.keyFrame;
    args->pic.isGoldenFrame = (pDecCont->decoder.refreshGolden ||
                               pDecCont->decoder.copyBufferToGolden) ? 1 : 0;
    /* Frame size and format information. */
    args->pic.frameWidth = (pDecCont->width + 15) & ~15;
    args->pic.frameHeight = (pDecCont->height + 15) & ~15;
    args->pic.codedWidth = pDecCont->width;
    args->pic.codedHeight = pDecCont->height;
    args->pic.lumaStride = pDecCont->asicBuff->lumaStride ?
        pDecCont->asicBuff->lumaStride : pDecCont->asicBuff->width;
    args->pic.chromaStride = pDecCont->asicBuff->chromaStride ?
        pDecCont->asicBuff->chromaStride : pDecCont->asicBuff->width;
    args->pic.outputFormat = pDecCont->tiledReferenceEnable ?
        DEC_OUT_FRM_TILED_8X4 : DEC_OUT_FRM_RASTER_SCAN;
    /* Buffer addresses. */
    const DWLLinearMem_t *outPic = &pDecCont->asicBuff->pictures[args->index];
    const DWLLinearMem_t *outPicC = &pDecCont->asicBuff->picturesC[args->index];
    args->pic.pOutputFrame = outPic->virtualAddress;
    args->pic.outputFrameBusAddress = outPic->busAddress;
    if(pDecCont->asicBuff->stridesUsed || pDecCont->asicBuff->customBuffers)
    {
        args->pic.pOutputFrameC = outPicC->virtualAddress;
        args->pic.outputFrameBusAddressC = outPicC->busAddress;
    }
    else
    {
        u32 chromaBufOffset = args->pic.frameWidth  * args->pic.frameHeight;
        args->pic.pOutputFrameC = args->pic.pOutputFrame + chromaBufOffset / 4;
        args->pic.outputFrameBusAddressC = args->pic.outputFrameBusAddress +
            chromaBufOffset;
    }
    /* Finally, set the information we don't know yet to 0. */
    args->pic.nbrOfErrMBs = 0;  /* To be set after decoding. */
    args->pic.picId = 0;  /* To be set after output reordering. */
    DWLSetIRQCallback(pDecCont->dwl, pDecCont->coreID,
                      VP8DecIRQCallbackFn, pDecCont);
}


static i32 FindIndex(VP8DecContainer_t* pDecCont, const u32* address)
{
    i32 i;
    for (i = 0; i < (i32)pDecCont->numBuffers; i++)
        if (pDecCont->asicBuff->pictures[i].virtualAddress == address)
            break;
    ASSERT((u32)i < pDecCont->numBuffers);
    return i;
}
