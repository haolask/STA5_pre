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
--
--  Abstract : Multicore API
--
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "h264hwd_container.h"
#include "h264decapi.h"
#include "h264hwd_decoder.h"
#include "h264hwd_util.h"
#include "h264hwd_exports.h"
#include "h264hwd_dpb.h"
#include "h264hwd_neighbour.h"
#include "h264hwd_asic.h"
#include "h264hwd_regdrv.h"
#include "h264hwd_byte_stream.h"
#include "deccfg.h"
#include "h264_pp_multibuffer.h"
#include "tiledref.h"
#include "workaround.h"
#include "commonconfig.h"
#include "dwl.h"

#include "h264hwd_dpb_lock.h"
#include "h264decmc_internals.h"

#ifdef _TRACE_PP_CTRL
#ifndef TRACE_PP_CTRL
#include <stdio.h>
#define TRACE_PP_CTRL(...)          printf(__VA_ARGS__)
#endif
#else
#define TRACE_PP_CTRL(...)
#endif

#ifdef H264DEC_TRACE
#define DEC_API_TRC(str)    H264DecTrace(str)
#else
#define DEC_API_TRC(str)    do{}while(0)
#endif


extern void h264InitPicFreezeOutput(decContainer_t *pDecCont, u32 fromOldDpb);
extern const u32 refBase[16];

u32 H264DecMCGetCoreCount(void)
{
    return DWLReadAsicCoreCount();
}

H264DecRet H264DecMCInit(H264DecInst *pDecInst, H264DecMCConfig *pMCInitCfg)
{
    H264DecRet ret;
    decContainer_t *pDecCont;
    u32 i;
    u32 dpbFlags = DEC_REF_FRM_RASTER_SCAN; /* no support for tiled mode */

    DEC_API_TRC("H264DecMCInit#\n");

    if(pDecInst == NULL || pMCInitCfg == NULL)
    {
        DEC_API_TRC("H264DecMCInit# ERROR: pDecInst or pMCInitCfg is NULL\n");
        return (H264DEC_PARAM_ERROR);
    }

    if(pMCInitCfg->dpbFlags & DEC_DPB_ALLOW_FIELD_ORDERING)
        dpbFlags |= DEC_DPB_ALLOW_FIELD_ORDERING;

    ret = H264DecInit( pDecInst,
                       pMCInitCfg->noOutputReordering,
                       0,
                       pMCInitCfg->useDisplaySmoothing,
                       dpbFlags );

    if(ret != H264DEC_OK )
    {
        DEC_API_TRC("H264DecMCInit# ERROR: Failed to create instance\n");
        return ret;
    }

    pDecCont = (decContainer_t *) (*pDecInst);

    pDecCont->bMC = 1;

    pDecCont->nCores = DWLReadAsicCoreCount();

    DWLReadMCAsicConfig(pDecCont->hwCfg);

    /* check how many cores support H264 */
    for(i = 0; i < pDecCont->nCores; i++)
    {
        if(!pDecCont->hwCfg[i].h264Support)
            pDecCont->nCores--;
    }

    pDecCont->streamConsumedCallback.fn = pMCInitCfg->streamConsumedCallback;

    /* enable synchronization writing in multi-core HW */
    if(pDecCont->nCores > 1)
    {
        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_MULTICORE_E, 1);
        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_WRITESTAT_E, 1);
    }

    return ret;
}

H264DecRet H264DecMCDecode(H264DecInst decInst,
                           const H264DecInput *pInput,
                           H264DecOutput *pOutput)
{
    H264DecRet ret;
    decContainer_t *pDecCont = (decContainer_t *) decInst;

    DEC_API_TRC("H264DecMCDecode#\n");

    /* Check that function input parameters are valid */
    if(pInput == NULL || pOutput == NULL || decInst == NULL)
    {
        DEC_API_TRC("H264DecMCDecode# ERROR: NULL arg(s)\n");
        return (H264DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("H264DecMCDecode# ERROR: Decoder not initialized\n");
        return (H264DEC_NOT_INITIALIZED);
    }

    /* Currently we just call the single core version, but we may change
     * our mind in the future. Do not call directly H264DecDecode() in any
     * multicore application */
    ret = H264DecDecode(decInst, pInput, pOutput);

    return ret;
}


/*!\brief Mark last output picture consumed
 *
 * Application calls this after it has finished processing the picture
 * returned by H264DecMCNextPicture.
 */

H264DecRet H264DecMCPictureConsumed(H264DecInst decInst,
                                    const H264DecPicture *pPicture)
{
    decContainer_t *pDecCont = (decContainer_t *) decInst;
    const dpbStorage_t *dpb;
    u32 id = FB_NOT_VALID_ID, i;

    DEC_API_TRC("H264DecMCPictureConsumed#\n");

    if(decInst == NULL || pPicture == NULL)
    {
        DEC_API_TRC("H264DecMCPictureConsumed# ERROR: decInst or pPicture is NULL\n");
        return (H264DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("H264DecMCPictureConsumed# ERROR: Decoder not initialized\n");
        return (H264DEC_NOT_INITIALIZED);
    }

    /* find the mem descriptor for this specific buffer, base view first */
    dpb = pDecCont->storage.dpbs[0];
    for(i = 0; i < dpb->totBuffers; i++)
    {
        if(pPicture->outputPictureBusAddress == dpb->picBuffers[i].busAddress &&
           pPicture->pOutputPicture == dpb->picBuffers[i].virtualAddress)
        {
           id = i;
            break;
        }
    }

    /* if not found, search other view for MVC mode */
    if(id == FB_NOT_VALID_ID && pDecCont->storage.mvc == HANTRO_TRUE)
    {
        dpb = pDecCont->storage.dpbs[1];
       /* find the mem descriptor for this specific buffer */
       for(i = 0; i < dpb->totBuffers; i++)
       {
           if(pPicture->outputPictureBusAddress == dpb->picBuffers[i].busAddress &&
              pPicture->pOutputPicture == dpb->picBuffers[i].virtualAddress)
           {
               id = i;
               break;
           }
       }
    }

    if(id == FB_NOT_VALID_ID)
      return H264DEC_PARAM_ERROR;

    PopOutputPic(&pDecCont->fbList, dpb->picBuffID[id]);

    return H264DEC_OK;
}

/*------------------------------------------------------------------------------

    Function: H264DecMCNextPicture

        Functional description:
            Get next picture in display order if any available.

        Input:
            decInst     decoder instance.
            endOfStream force output of all buffered pictures

        Output:
            pOutput     pointer to output structure

        Returns:
            H264DEC_OK            no pictures available for display
            H264DEC_PIC_RDY       picture available for display
            H264DEC_PARAM_ERROR     invalid parameters

------------------------------------------------------------------------------*/
H264DecRet H264DecMCNextPicture_INTERNAL(H264DecInst decInst,
                                         H264DecPicture * pOutput,
                                         u32 endOfStream)
{
    decContainer_t *pDecCont = (decContainer_t *) decInst;
    const dpbOutPicture_t *outPic = NULL;
    dpbStorage_t *outDpb;
    storage_t *pStorage;
    sliceHeader_t *pSliceHdr;

    DEC_API_TRC("H264DecNextPicture#\n");

    if(decInst == NULL || pOutput == NULL)
    {
        DEC_API_TRC("H264DecNextPicture# ERROR: decInst or pOutput is NULL\n");
        return (H264DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("H264DecNextPicture# ERROR: Decoder not initialized\n");
        return (H264DEC_NOT_INITIALIZED);
    }

    pStorage = &pDecCont->storage;
    pSliceHdr = pStorage->sliceHeader;
    outDpb = pDecCont->storage.dpbs[pDecCont->storage.outView];

    /* if display order is the same as decoding order and PP is used and
     * cannot be used in pipeline (rotation) -> do not perform PP here but
     * while decoding next picture (parallel processing instead of
     * DEC followed by PP followed by DEC...) */
    if (pDecCont->storage.pendingOutPic)
    {
        outPic = pDecCont->storage.pendingOutPic;
        pDecCont->storage.pendingOutPic = NULL;
    }
    else if(outDpb->noReordering == 0)
    {
        if(!outDpb->delayedOut)
        {
            if (pDecCont->pp.ppInstance && pDecCont->pp.decPpIf.ppStatus ==
                DECPP_PIC_READY)
                outDpb->noOutput = 0;

            pDecCont->storage.dpb =
                pDecCont->storage.dpbs[pDecCont->storage.outView];

            outPic = h264bsdNextOutputPicture(&pDecCont->storage);

            if ( (pDecCont->storage.numViews ||
                  pDecCont->storage.outView) && outPic != NULL)
            {
                pOutput->viewId =
                    pDecCont->storage.viewId[pDecCont->storage.outView];
                pDecCont->storage.outView ^= 0x1;
            }
        }
    }
    else
    {
        /* no reordering of output pics AND stereo was activated after base
         * picture was output -> output stereo view pic if available */
        if (pDecCont->storage.numViews &&
            pDecCont->storage.view && pDecCont->storage.outView == 0 &&
            outDpb->numOut == 0 &&
            pDecCont->storage.dpbs[pDecCont->storage.view]->numOut > 0)
        {
            pDecCont->storage.outView ^= 0x1;
            outDpb = pDecCont->storage.dpbs[pDecCont->storage.outView];
        }

        if(outDpb->numOut > 1 || endOfStream ||
           pStorage->prevNalUnit->nalRefIdc == 0 ||
           pDecCont->pp.ppInstance == NULL ||
           pDecCont->pp.decPpIf.usePipeline ||
           pStorage->view != pStorage->outView)
        {
            if(!endOfStream &&
               ((outDpb->numOut == 1 && outDpb->delayedOut) ||
                (pSliceHdr->fieldPicFlag && pStorage->secondField)))
            {
            }
            else
            {
                pDecCont->storage.dpb =
                    pDecCont->storage.dpbs[pDecCont->storage.outView];

                outPic = h264bsdNextOutputPicture(&pDecCont->storage);

                pOutput->viewId =
                    pDecCont->storage.viewId[pDecCont->storage.outView];
                if ( (pDecCont->storage.numViews ||
                      pDecCont->storage.outView) && outPic != NULL)
                    pDecCont->storage.outView ^= 0x1;
            }
        }
    }

    if(outPic != NULL)
    {
        if (!pDecCont->storage.numViews)
            pOutput->viewId = 0;

        pOutput->pOutputPicture = outPic->data->virtualAddress;
        pOutput->outputPictureBusAddress = outPic->data->busAddress;
        pOutput->picId = outPic->picId;
        pOutput->picCodingType[0] = outPic->picCodeType[0];
        pOutput->picCodingType[1] = outPic->picCodeType[1];
        pOutput->isIdrPicture[0] = outPic->isIdr[0];
        pOutput->isIdrPicture[1] = outPic->isIdr[1];
        pOutput->nbrOfErrMBs = outPic->numErrMbs;

        pOutput->interlaced = outPic->interlaced;
        pOutput->fieldPicture = outPic->fieldPicture;
        pOutput->topField = outPic->topField;

        pOutput->picWidth = h264bsdPicWidth(&pDecCont->storage) << 4;
        pOutput->picHeight = h264bsdPicHeight(&pDecCont->storage) << 4;
        pOutput->outputFormat = outPic->tiledMode ?
        DEC_OUT_FRM_TILED_8X4 : DEC_OUT_FRM_RASTER_SCAN;

        pOutput->cropParams = outPic->crop;

        DEC_API_TRC("H264DecNextPicture# H264DEC_PIC_RDY\n");


        PushOutputPic(&pDecCont->fbList, pOutput, outPic->memIdx);

        return (H264DEC_PIC_RDY);
    }
    else
    {
        DEC_API_TRC("H264DecNextPicture# H264DEC_OK\n");
        return (H264DEC_OK);
    }

}

H264DecRet H264DecMCEndOfStream(H264DecInst decInst)
{
    decContainer_t *pDecCont = (decContainer_t *) decInst;
    u32 count = 0;

    DEC_API_TRC("H264DecMCEndOfStream#\n");

    if(decInst == NULL)
    {
        DEC_API_TRC("H264DecMCEndOfStream# ERROR: decInst is NULL\n");
        return (H264DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("H264DecMCEndOfStream# ERROR: Decoder not initialized\n");
        return (H264DEC_NOT_INITIALIZED);
    }

    /* flush any remaining pictures form DPB */
    h264bsdFlushBuffer(&pDecCont->storage);

    FinalizeOutputAll(&pDecCont->fbList);

    {
        H264DecPicture output;

        while(H264DecMCNextPicture_INTERNAL(decInst, &output, 1) == H264DEC_PIC_RDY)
        {
            count++;
        }
    }

    /* After all output pictures were pushed, update decoder status to
     * reflect the end-of-stream situation. This way the H264DecMCNextPicture
     * will not block anymore once all output was handled.
     */
    pDecCont->decStat = H264DEC_END_OF_STREAM;

    /* wake-up output thread */
    PushOutputPic(&pDecCont->fbList, NULL, -1);

    /* TODO(atna): should it be enough to wait until all cores idle and
     *             not that output is empty !?
     */
    h264MCWaitPicReadyAll(pDecCont);

    DEC_API_TRC("H264DecMCEndOfStream# H264DEC_OK\n");
    return (H264DEC_OK);
}

/*------------------------------------------------------------------------------

    Function: H264DecMCNextPicture

        Functional description:
            Get next picture in display order if any available.

        Input:
            decInst     decoder instance.
            endOfStream force output of all buffered pictures

        Output:
            pOutput     pointer to output structure

        Returns:
            H264DEC_OK            no pictures available for display
            H264DEC_PIC_RDY       picture available for display
            H264DEC_PARAM_ERROR     invalid parameters

------------------------------------------------------------------------------*/
H264DecRet H264DecMCNextPicture(H264DecInst decInst, H264DecPicture * pPicture)
{
    decContainer_t *pDecCont = (decContainer_t *) decInst;

    DEC_API_TRC("H264DecMCNextPicture#\n");

    if(decInst == NULL || pPicture == NULL)
    {
        DEC_API_TRC("H264DecMCNextPicture# ERROR: decInst or pOutput is NULL\n");
        return (H264DEC_PARAM_ERROR);
    }

    /* Check for valid decoder instance */
    if(pDecCont->checksum != pDecCont)
    {
        DEC_API_TRC("H264DecMCNextPicture# ERROR: Decoder not initialized\n");
        return (H264DEC_NOT_INITIALIZED);
    }

    if(pDecCont->decStat == H264DEC_END_OF_STREAM &&
       IsOutputEmpty(&pDecCont->fbList))
    {
        DEC_API_TRC("H264DecMCNextPicture# H264DEC_END_OF_STREAM\n");
        return (H264DEC_END_OF_STREAM);
    }

    if(PeekOutputPic(&pDecCont->fbList, pPicture))
    {
        DEC_API_TRC("H264DecMCNextPicture# H264DEC_PIC_RDY\n");
        return (H264DEC_PIC_RDY);
    }
    else
    {
        DEC_API_TRC("H264DecMCNextPicture# H264DEC_OK\n");
        return (H264DEC_OK);
    }
}

void h264MCPushOutputAll(decContainer_t *pDecCont)
{
    u32 ret;
    H264DecPicture output;
    do
    {
        ret = H264DecMCNextPicture_INTERNAL(pDecCont, &output, 0);
    }
    while( ret == H264DEC_PIC_RDY );
}

void h264MCWaitOutFifoEmpty(decContainer_t *pDecCont)
{
    WaitOutputEmpty(&pDecCont->fbList);
}

void h264MCWaitPicReadyAll(decContainer_t *pDecCont)
{
    WaitListNotInUse(&pDecCont->fbList);
}

void h264MCSetRefPicStatus(volatile u8 *pSyncMem, u32 isFieldPic,
                           u32 isBottomField)
{
    if (isFieldPic == 0)
    {
        /* frame status */
        DWLmemset((void*)pSyncMem, 0xFF, 32);
    }
    else if (isBottomField == 0)
    {
        /* top field status */
        DWLmemset((void*)pSyncMem, 0xFF, 16);
    }
    else
    {
        /* bottom field status */
        pSyncMem += 16;
        DWLmemset((void*)pSyncMem, 0xFF, 16);
    }
}

static u32 MCGetRefPicStatus(const u8 *pSyncMem, u32 isFieldPic,
                             u32 isBottomField)
{
    u32 ret;

    if (isFieldPic == 0)
    {
        /* frame status */
        ret = ( pSyncMem[0] << 8) + pSyncMem[1];
    }
    else if (isBottomField == 0)
    {
        /* top field status */
        ret = ( pSyncMem[0] << 8) + pSyncMem[1];
    }
    else
    {
        /* bottom field status */
        pSyncMem += 16;
        ret = ( pSyncMem[0] << 8) + pSyncMem[1];
    }
    return ret;
}

static void MCValidateRefPicStatus(const u32 *h264Regs,
                                   H264HwRdyCallbackArg *info)
{
    const u8* pRefStat;
    const DWLLinearMem_t *pOut;
    const dpbStorage_t *dpb = info->currentDpb;
    u32 status, expected;

    pOut = (DWLLinearMem_t *)GetDataById(dpb->fbList, info->outId);

    pRefStat = (u8*)pOut->virtualAddress + dpb->syncMcOffset;

    status = MCGetRefPicStatus(pRefStat, info->isFieldPic, info->isBottomField);

    expected = GetDecRegister(h264Regs, HWIF_PIC_MB_HEIGHT_P);

    expected *= 16;

    if(info->isFieldPic)
        expected /= 2;

    if(status < expected)
    {
        ASSERT(status == expected);
        h264MCSetRefPicStatus((u8*)pRefStat,
                              info->isFieldPic, info->isBottomField);
    }
}

void h264MCHwRdyCallback(void *args, i32 core_id)
{
    u32 decRegs[DEC_X170_REGISTERS];

    decContainer_t *pDecCont = (decContainer_t *)args;
    H264HwRdyCallbackArg info;

    const void *dwl;
    const dpbStorage_t *dpb;

    u32 core_status, type;
    u32 i;

    ASSERT(pDecCont != NULL);
    ASSERT(core_id < MAX_ASIC_CORES);

    /* take a copy of the args as after we release the HW they
     * can be overwritten.
     */
    info = pDecCont->hwRdyCallbackArg[core_id];

    dwl = pDecCont->dwl;
    dpb = info.currentDpb;

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
        DWLLinearMem_t *pOut = (DWLLinearMem_t *)GetDataById(dpb->fbList,
                                                             info.outId);

#ifdef DEC_PRINT_BAD_IRQ
        fprintf(stderr, "\nCore %d \"bad\" IRQ = 0x%08x\n",
                core_id, core_status);
#endif

        /* reset HW if still enabled */
        if (core_status & DEC_8190_IRQ_BUFFER)
        {
            /*  reset HW; we don't want an IRQ after reset so disable it */
            DWLDisableHW(dwl, core_id, 0x04,
                         core_status | DEC_IRQ_DISABLE | DEC_ABORT);
        }

        /* reset DMV storage for erroneous pictures */
        {
            u32 dvm_mem_size = pDecCont->storage.picSizeInMbs * 64;
            u8 *dvm_base = (u8*)pOut->virtualAddress;

            dvm_base += dpb->dirMvOffset;

            if(info.isFieldPic)
            {
                dvm_mem_size /= 2;
                if(info.isBottomField)
                    dvm_base += dvm_mem_size;
            }

            (void) DWLmemset(dvm_base, 0, dvm_mem_size);
        }

        h264MCSetRefPicStatus((u8*)pOut->virtualAddress + dpb->syncMcOffset,
                              info.isFieldPic, info.isBottomField);

        /* mark corrupt picture in output queue */
        MarkOutputPicCorrupt(dpb->fbList, info.outId,
                             pDecCont->storage.picSizeInMbs);

        /* ... and in DPB */
        i = dpb->dpbSize + 1;
        while((i--) > 0)
        {
            dpbPicture_t *dpbPic = (dpbPicture_t *)dpb->buffer + i;
            if(dpbPic->data == pOut)
            {
                dpbPic->numErrMbs = pDecCont->storage.picSizeInMbs;
                break;
            }
        }
    }
    else
    {
        MCValidateRefPicStatus(decRegs, &info);
    }

    /* clear IRQ status reg and release HW core */
    DWLReleaseHw(dwl, info.coreID);

    H264UpdateAfterHwRdy(pDecCont, decRegs);

    /* release the stream buffer. Callback provided by app */
    if(pDecCont->streamConsumedCallback.fn)
        pDecCont->streamConsumedCallback.fn((u8*)info.pStream,
                                            (void*)info.pUserData);

    if(info.isFieldPic)
    {
        if(info.isBottomField)
            type = FB_HW_OUT_FIELD_BOT;
        else
            type = FB_HW_OUT_FIELD_TOP;
    }
    else
    {
        type = FB_HW_OUT_FRAME;
    }

    ClearHWOutput(dpb->fbList, info.outId, type);

    /* decrement buffer usage in our buffer handling */
    for (i = 0; i < dpb->dpbSize; i++)
    {
        DecrementRefUsage(dpb->fbList, info.refId[i]);
    }
}

void h264MCSetHwRdyCallback(decContainer_t *pDecCont)
{
    dpbStorage_t *dpb = pDecCont->storage.dpb;
    u32 type, i;

    H264HwRdyCallbackArg *arg = &pDecCont->hwRdyCallbackArg[pDecCont->coreID];

    arg->coreID = pDecCont->coreID;
    arg->pStream = pDecCont->streamConsumedCallback.pStrmBuff;
    arg->pUserData = pDecCont->streamConsumedCallback.pUserData;
    arg->isFieldPic = dpb->currentOut->isFieldPic;
    arg->isBottomField = dpb->currentOut->isBottomField;
    arg->outId = dpb->currentOut->memIdx;
    arg->currentDpb = dpb;

    for (i = 0; i < dpb->dpbSize; i++)
    {
        const DWLLinearMem_t *ref;
        ref = (DWLLinearMem_t *)GetDataById(&pDecCont->fbList, dpb->refId[i]);

        ASSERT(ref->busAddress == (pDecCont->asicBuff->refPicList[i] & (~3)));
        (void)ref;

        arg->refId[i] = dpb->refId[i];

    }

    DWLSetIRQCallback(pDecCont->dwl, pDecCont->coreID, h264MCHwRdyCallback,
                      pDecCont);

    if(arg->isFieldPic)
    {
        if(arg->isBottomField)
            type = FB_HW_OUT_FIELD_BOT;
        else
            type = FB_HW_OUT_FIELD_TOP;
    }
    else
    {
        type = FB_HW_OUT_FRAME;
    }

    MarkHWOutput(&pDecCont->fbList, dpb->currentOut->memIdx, type);
}
