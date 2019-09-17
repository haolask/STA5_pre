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

#include "basetype.h"
#include "h264hwd_regdrv.h"
#include "h264hwd_asic.h"
#include "h264hwd_container.h"
#include "h264hwd_debug.h"
#include "h264hwd_util.h"
#include "h264hwd_cabac.h"
#include "dwl.h"
#include "refbuffer.h"
#include "h264_pp_multibuffer.h"

#include "h264decmc_internals.h"

#define ASIC_HOR_MV_MASK            0x07FFFU
#define ASIC_VER_MV_MASK            0x01FFFU

#define ASIC_HOR_MV_OFFSET          17U
#define ASIC_VER_MV_OFFSET          4U

static void H264RefreshRegs(decContainer_t * pDecCont);
static void H264FlushRegs(decContainer_t * pDecCont);
static void h264StreamPosUpdate(decContainer_t * pDecCont);
static void H264PrepareCabacInitTables(u32 *cabacInit);
static void h264PreparePOC(decContainer_t *pDecCont);
static void h264PrepareScaleList(decContainer_t *pDecCont);
static void h264CopyPocToHw(decContainer_t *pDecCont);

#ifdef _TRACE_PP_CTRL
#ifndef TRACE_PP_CTRL
#include <stdio.h>
#define TRACE_PP_CTRL(...)          printf(__VA_ARGS__)
#endif
#else
#define TRACE_PP_CTRL(...)
#endif

const u32 refBase[16] = {
    HWIF_REFER0_BASE, HWIF_REFER1_BASE, HWIF_REFER2_BASE,
    HWIF_REFER3_BASE, HWIF_REFER4_BASE, HWIF_REFER5_BASE,
    HWIF_REFER6_BASE, HWIF_REFER7_BASE, HWIF_REFER8_BASE,
    HWIF_REFER9_BASE, HWIF_REFER10_BASE, HWIF_REFER11_BASE,
    HWIF_REFER12_BASE, HWIF_REFER13_BASE, HWIF_REFER14_BASE,
    HWIF_REFER15_BASE
};

const u32 refFieldMode[16] = {
    HWIF_REFER0_FIELD_E, HWIF_REFER1_FIELD_E, HWIF_REFER2_FIELD_E,
    HWIF_REFER3_FIELD_E, HWIF_REFER4_FIELD_E, HWIF_REFER5_FIELD_E,
    HWIF_REFER6_FIELD_E, HWIF_REFER7_FIELD_E, HWIF_REFER8_FIELD_E,
    HWIF_REFER9_FIELD_E, HWIF_REFER10_FIELD_E, HWIF_REFER11_FIELD_E,
    HWIF_REFER12_FIELD_E, HWIF_REFER13_FIELD_E, HWIF_REFER14_FIELD_E,
    HWIF_REFER15_FIELD_E
};

const u32 refTopc[16] = {
    HWIF_REFER0_TOPC_E, HWIF_REFER1_TOPC_E, HWIF_REFER2_TOPC_E,
    HWIF_REFER3_TOPC_E, HWIF_REFER4_TOPC_E, HWIF_REFER5_TOPC_E,
    HWIF_REFER6_TOPC_E, HWIF_REFER7_TOPC_E, HWIF_REFER8_TOPC_E,
    HWIF_REFER9_TOPC_E, HWIF_REFER10_TOPC_E, HWIF_REFER11_TOPC_E,
    HWIF_REFER12_TOPC_E, HWIF_REFER13_TOPC_E, HWIF_REFER14_TOPC_E,
    HWIF_REFER15_TOPC_E
};

const u32 refPicNum[16] = {
    HWIF_REFER0_NBR, HWIF_REFER1_NBR, HWIF_REFER2_NBR,
    HWIF_REFER3_NBR, HWIF_REFER4_NBR, HWIF_REFER5_NBR,
    HWIF_REFER6_NBR, HWIF_REFER7_NBR, HWIF_REFER8_NBR,
    HWIF_REFER9_NBR, HWIF_REFER10_NBR, HWIF_REFER11_NBR,
    HWIF_REFER12_NBR, HWIF_REFER13_NBR, HWIF_REFER14_NBR,
    HWIF_REFER15_NBR
};


/*------------------------------------------------------------------------------
                Reference list initialization
------------------------------------------------------------------------------*/
#define IS_SHORT_TERM_FRAME(a) ((a).status[0] == SHORT_TERM && \
                                (a).status[1] == SHORT_TERM)
#define IS_SHORT_TERM_FRAME_F(a) ((a).status[0] == SHORT_TERM || \
                                (a).status[1] == SHORT_TERM)
#define IS_LONG_TERM_FRAME(a) ((a).status[0] == LONG_TERM && \
                                (a).status[1] == LONG_TERM)
#define IS_LONG_TERM_FRAME_F(a) ((a).status[0] == LONG_TERM || \
                                (a).status[1] == LONG_TERM)
#define IS_REF_FRAME(a) ((a).status[0] && (a).status[0] != EMPTY && \
                         (a).status[1] && (a).status[1] != EMPTY )
#define IS_REF_FRAME_F(a) (((a).status[0] && (a).status[0] != EMPTY) || \
                           ((a).status[1] && (a).status[1] != EMPTY) )

#define FRAME_POC(a) MIN((a).picOrderCnt[0], (a).picOrderCnt[1])
#define FIELD_POC(a) MIN(((a).status[0] != EMPTY) ? \
                          (a).picOrderCnt[0] : 0x7FFFFFFF,\
                         ((a).status[1] != EMPTY) ? \
                          (a).picOrderCnt[1] : 0x7FFFFFFF)

#define INVALID_IDX 0xFFFFFFFF
#define MIN_POC 0x80000000
#define MAX_POC 0x7FFFFFFF

const u32 refPicList0[16] = {
    HWIF_BINIT_RLIST_F0, HWIF_BINIT_RLIST_F1, HWIF_BINIT_RLIST_F2,
    HWIF_BINIT_RLIST_F3, HWIF_BINIT_RLIST_F4, HWIF_BINIT_RLIST_F5,
    HWIF_BINIT_RLIST_F6, HWIF_BINIT_RLIST_F7, HWIF_BINIT_RLIST_F8,
    HWIF_BINIT_RLIST_F9, HWIF_BINIT_RLIST_F10, HWIF_BINIT_RLIST_F11,
    HWIF_BINIT_RLIST_F12, HWIF_BINIT_RLIST_F13, HWIF_BINIT_RLIST_F14,
    HWIF_BINIT_RLIST_F15
};

const u32 refPicList1[16] = {
    HWIF_BINIT_RLIST_B0, HWIF_BINIT_RLIST_B1, HWIF_BINIT_RLIST_B2,
    HWIF_BINIT_RLIST_B3, HWIF_BINIT_RLIST_B4, HWIF_BINIT_RLIST_B5,
    HWIF_BINIT_RLIST_B6, HWIF_BINIT_RLIST_B7, HWIF_BINIT_RLIST_B8,
    HWIF_BINIT_RLIST_B9, HWIF_BINIT_RLIST_B10, HWIF_BINIT_RLIST_B11,
    HWIF_BINIT_RLIST_B12, HWIF_BINIT_RLIST_B13, HWIF_BINIT_RLIST_B14,
    HWIF_BINIT_RLIST_B15
};

const u32 refPicListP[16] = {
    HWIF_PINIT_RLIST_F0, HWIF_PINIT_RLIST_F1, HWIF_PINIT_RLIST_F2,
    HWIF_PINIT_RLIST_F3, HWIF_PINIT_RLIST_F4, HWIF_PINIT_RLIST_F5,
    HWIF_PINIT_RLIST_F6, HWIF_PINIT_RLIST_F7, HWIF_PINIT_RLIST_F8,
    HWIF_PINIT_RLIST_F9, HWIF_PINIT_RLIST_F10, HWIF_PINIT_RLIST_F11,
    HWIF_PINIT_RLIST_F12, HWIF_PINIT_RLIST_F13, HWIF_PINIT_RLIST_F14,
    HWIF_PINIT_RLIST_F15
};

/*------------------------------------------------------------------------------
    Function name : AllocateAsicBuffers
    Description   :

    Return type   : i32
    Argument      : DecAsicBuffers_t * asicBuff
    Argument      : u32 mbs
------------------------------------------------------------------------------*/
u32 AllocateAsicBuffers(decContainer_t * pDecCont, DecAsicBuffers_t * asicBuff,
                        u32 mbs)
{
    i32 ret = 0;
    u32 tmp, i;

    if(pDecCont->rlcMode)
    {
        tmp = ASIC_MB_CTRL_BUFFER_SIZE;
        ret = DWLMallocLinearNamed(pDecCont->dwl, mbs * tmp, &asicBuff[0].mbCtrl,
                "AllocateAsicBuffers mbCtrl");

        tmp = ASIC_MB_MV_BUFFER_SIZE;
        ret |= DWLMallocLinearNamed(pDecCont->dwl, mbs * tmp, &asicBuff[0].mv,
                "AllocateAsicBuffers mv");

        tmp = ASIC_MB_I4X4_BUFFER_SIZE;
        ret |= DWLMallocLinearNamed(pDecCont->dwl, mbs * tmp, &asicBuff[0].intraPred,
                "AllocateAsicBuffers intraPred");

        tmp = ASIC_MB_RLC_BUFFER_SIZE;
        ret |= DWLMallocLinearNamed(pDecCont->dwl, mbs * tmp, &asicBuff[0].residual,
                "AllocateAsicBuffers residual");
    }

    asicBuff[0].buff_status = 0;
    asicBuff[0].picSizeInMbs = mbs;

    if(pDecCont->h264ProfileSupport != H264_BASELINE_PROFILE)
    {
        tmp = ASIC_CABAC_INIT_BUFFER_SIZE + ASIC_SCALING_LIST_SIZE +
            ASIC_POC_BUFFER_SIZE;

        for(i = 0; i < pDecCont->nCores; i++)
        {
            ret |= DWLMallocLinearNamed(pDecCont->dwl, tmp,
                                   asicBuff->cabacInit + i,
                "AllocateAsicBuffers cabacInit");
            if(ret == 0)
            {
                H264PrepareCabacInitTables(
                                    asicBuff->cabacInit[i].virtualAddress);
            }
        }
    }

    if(pDecCont->refBufSupport)
    {
        RefbuInit(&pDecCont->refBufferCtrl, DEC_X170_MODE_H264,
                  pDecCont->storage.activeSps->picWidthInMbs,
                  pDecCont->storage.activeSps->picHeightInMbs,
                  pDecCont->refBufSupport);
    }

    if(ret != 0)
        return 1;
    else
        return 0;
}

/*------------------------------------------------------------------------------
    Function name : ReleaseAsicBuffers
    Description   :

    Return type   : void
    Argument      : DecAsicBuffers_t * asicBuff
------------------------------------------------------------------------------*/
void ReleaseAsicBuffers(const void *dwl, DecAsicBuffers_t * asicBuff)
{
    i32 i;

    if(asicBuff[0].mbCtrl.virtualAddress != NULL)
    {
        DWLFreeLinear(dwl, &asicBuff[0].mbCtrl);
        asicBuff[0].mbCtrl.virtualAddress = NULL;
    }
    if(asicBuff[0].residual.virtualAddress != NULL)
    {
        DWLFreeLinear(dwl, &asicBuff[0].residual);
        asicBuff[0].residual.virtualAddress = NULL;
    }

    if(asicBuff[0].mv.virtualAddress != NULL)
    {
        DWLFreeLinear(dwl, &asicBuff[0].mv);
        asicBuff[0].mv.virtualAddress = NULL;
    }

    if(asicBuff[0].intraPred.virtualAddress != NULL)
    {
        DWLFreeLinear(dwl, &asicBuff[0].intraPred);
        asicBuff[0].intraPred.virtualAddress = NULL;
    }


    for(i = 0; i < MAX_ASIC_CORES; i++)
    {
        if(asicBuff[0].cabacInit[i].virtualAddress != NULL)
        {
            DWLFreeLinear(dwl, asicBuff[0].cabacInit + i);
            asicBuff[0].cabacInit[i].virtualAddress = NULL;
        }
    }

}

/*------------------------------------------------------------------------------
    Function name : H264RunAsic
    Description   :

    Return type   : hw status
    Argument      : DecAsicBuffers_t * pAsicBuff
------------------------------------------------------------------------------*/
u32 H264RunAsic(decContainer_t * pDecCont, DecAsicBuffers_t * pAsicBuff)
{
    const seqParamSet_t *pSps = pDecCont->storage.activeSps;
    const sliceHeader_t *pSliceHeader = pDecCont->storage.sliceHeader;
    const picParamSet_t *pPps = pDecCont->storage.activePps;
    const storage_t *pStorage = &pDecCont->storage;
    dpbStorage_t *dpb = pDecCont->storage.dpb;

    u32 asic_status = 0;
    i32 ret = 0;

    if(!pDecCont->asicRunning)
    {
        u32 reserveRet = 0;
        u32 i;

        for(i = 0; i < 16; i++)
        {
            /* stream decoder may get invalid ref buffer index from errorneus
               stream. Initialize unused base addresses to safe values */
            if(pAsicBuff->refPicList[i] == 0x0)
            {
                SetDecRegister(pDecCont->h264Regs, refBase[i],
                           pAsicBuff->refPicList[0]);
            }
            else
            {
                SetDecRegister(pDecCont->h264Regs, refBase[i],
                               pAsicBuff->refPicList[i]);
            }
        }
        /* inter-view reference picture */
        if (pDecCont->storage.view && !pDecCont->storage.nonInterViewRef)
        {
            u32 tmp;
            tmp = pDecCont->storage.dpbs[0]->currentOut->data->busAddress;
            if (pDecCont->storage.dpbs[0]->currentOut->isFieldPic)
                tmp |= 0x2;
            SetDecRegister(pDecCont->h264Regs, HWIF_INTER_VIEW_BASE, tmp);
            SetDecRegister(pDecCont->h264Regs, HWIF_REFER_VALID_E,
                GetDecRegister(pDecCont->h264Regs, HWIF_REFER_VALID_E) |
                (pSliceHeader->fieldPicFlag ? 0x3 : 0x10000) );
            /* mark interview pic as long term, just to "cheat" mvd to never
             * set colZeroFlag */
            SetDecRegister(pDecCont->h264Regs, HWIF_REFER_LTERM_E,
                GetDecRegister(pDecCont->h264Regs, HWIF_REFER_LTERM_E) |
                (pSliceHeader->fieldPicFlag ? 0x3 : 0x10000) );
        }
        SetDecRegister(pDecCont->h264Regs, HWIF_MVC_E,
            pDecCont->storage.view != 0);

        SetDecRegister(pDecCont->h264Regs, HWIF_FILTERING_DIS,
                       pAsicBuff->filterDisable);

#if 0
        fprintf(stderr, "\n\tCurrently decoding; %s 0x%08x\n",
                pSliceHeader->fieldPicFlag ?
                (pSliceHeader->bottomFieldFlag ? "FIELD BOTTOM" : "FIELD TOP") :
                "FRAME", pAsicBuff->outBuffer->busAddress);
#endif

        if(pSliceHeader->fieldPicFlag && pSliceHeader->bottomFieldFlag)
        {

            if(pDecCont->dpbMode == DEC_DPB_FRAME)
                SetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUT_BASE,
                               pAsicBuff->outBuffer->busAddress +
                               pSps->picWidthInMbs * 16);
            else                               
                SetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUT_BASE,
                               pAsicBuff->outBuffer->busAddress);
        }
        else
        {
            SetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUT_BASE,
                           pAsicBuff->outBuffer->busAddress);
        }
        SetDecRegister( pDecCont->h264Regs, HWIF_DPB_ILACE_MODE, 
                        pDecCont->dpbMode );

        SetDecRegister(pDecCont->h264Regs, HWIF_CH_QP_OFFSET,
                       pAsicBuff->chromaQpIndexOffset);
        SetDecRegister(pDecCont->h264Regs, HWIF_CH_QP_OFFSET2,
                       pAsicBuff->chromaQpIndexOffset2);

        if(pDecCont->rlcMode)
        {
            SetDecRegister(pDecCont->h264Regs, HWIF_RLC_MODE_E, 1);

            SetDecRegister(pDecCont->h264Regs, HWIF_MB_CTRL_BASE,
                           pAsicBuff->mbCtrl.busAddress);

            SetDecRegister(pDecCont->h264Regs, HWIF_RLC_VLC_BASE,
                           pAsicBuff->residual.busAddress);

            SetDecRegister(pDecCont->h264Regs, HWIF_REF_FRAMES,
                           pSps->numRefFrames);

            SetDecRegister(pDecCont->h264Regs, HWIF_INTRA_4X4_BASE,
                           pAsicBuff->intraPred.busAddress);
            SetDecRegister(pDecCont->h264Regs, HWIF_DIFF_MV_BASE,
                           pAsicBuff->mv.busAddress);

            SetDecRegister(pDecCont->h264Regs, HWIF_STREAM_LEN, 0);
            SetDecRegister(pDecCont->h264Regs, HWIF_STRM_START_BIT, 0);
        }
        else
        {
            /* new 8190 stuff (not used/ignored in 8170) */
            if(pDecCont->h264ProfileSupport == H264_BASELINE_PROFILE)
            {
                goto skipped_high_profile;  /* leave high profile stuff disabled */
            }

            /* direct mv writing not enabled if HW config says that
             * SW_DEC_H264_PROF="10" */
            if(pAsicBuff->enableDmvAndPoc && pDecCont->h264ProfileSupport != 2)
            {
                u32 dirMvOffset = dpb->dirMvOffset;

                if (pDecCont->storage.mvc && pDecCont->storage.view == 0)
                    SetDecRegister(pDecCont->h264Regs, HWIF_WRITE_MVS_E,
                        pDecCont->storage.nonInterViewRef == 0 ||
                        pDecCont->storage.prevNalUnit->nalRefIdc != 0);
                else
                    SetDecRegister(pDecCont->h264Regs, HWIF_WRITE_MVS_E,
                        pDecCont->storage.prevNalUnit->nalRefIdc != 0);

                if(pSliceHeader->bottomFieldFlag)
                    dirMvOffset += dpb->picSizeInMbs * 32;

                SetDecRegister(pDecCont->h264Regs, HWIF_DIR_MV_BASE,
                               pAsicBuff->outBuffer->busAddress + dirMvOffset);
            }
            else
                SetDecRegister(pDecCont->h264Regs, HWIF_WRITE_MVS_E, 0);

            /* make sure that output pic sync memory is cleared */
            {
                char *sync_base =
                        (char *) (pAsicBuff->outBuffer->virtualAddress) +
                                  dpb->syncMcOffset;
                u32 sync_size = 16; /* 16 bytes for each field */
                if (!pSliceHeader->fieldPicFlag)
                {
                    sync_size += 16; /* reset all 32 bytes */
                }
                else if (pSliceHeader->bottomFieldFlag)
                {
                    sync_base += 16; /* bottom field sync area*/
                }

                (void) DWLmemset(sync_base, 0, sync_size);
            }

            SetDecRegister(pDecCont->h264Regs, HWIF_DIR_8X8_INFER_E,
                           pSps->direct8x8InferenceFlag);
            SetDecRegister(pDecCont->h264Regs, HWIF_WEIGHT_PRED_E,
                           pPps->weightedPredFlag);
            SetDecRegister(pDecCont->h264Regs, HWIF_WEIGHT_BIPR_IDC,
                           pPps->weightedBiPredIdc);
            SetDecRegister(pDecCont->h264Regs, HWIF_REFIDX1_ACTIVE,
                           pPps->numRefIdxL1Active);
            SetDecRegister(pDecCont->h264Regs, HWIF_FIELDPIC_FLAG_E,
                           !pSps->frameMbsOnlyFlag);
            SetDecRegister(pDecCont->h264Regs, HWIF_PIC_INTERLACE_E,
                           !pSps->frameMbsOnlyFlag &&
                           (pSps->mbAdaptiveFrameFieldFlag || pSliceHeader->fieldPicFlag));

            SetDecRegister(pDecCont->h264Regs, HWIF_PIC_FIELDMODE_E,
                           pSliceHeader->fieldPicFlag);

            DEBUG_PRINT(("framembs %d, mbaff %d, fieldpic %d\n",
                   pSps->frameMbsOnlyFlag,
                   pSps->mbAdaptiveFrameFieldFlag, pSliceHeader->fieldPicFlag));

            SetDecRegister(pDecCont->h264Regs, HWIF_PIC_TOPFIELD_E,
                           !pSliceHeader->bottomFieldFlag);

            SetDecRegister(pDecCont->h264Regs, HWIF_SEQ_MBAFF_E,
                           pSps->mbAdaptiveFrameFieldFlag);

            SetDecRegister(pDecCont->h264Regs, HWIF_8X8TRANS_FLAG_E,
                           pPps->transform8x8Flag);

            SetDecRegister(pDecCont->h264Regs, HWIF_BLACKWHITE_E,
                           pSps->profileIdc >= 100 &&
                           pSps->chromaFormatIdc == 0);

            SetDecRegister(pDecCont->h264Regs, HWIF_TYPE1_QUANT_E,
                           pPps->scalingMatrixPresentFlag);
        }

/***************************************************************************/
      skipped_high_profile:

        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUT_DIS,
                       pAsicBuff->disableOutWriting);

        /* Setup reference picture buffer */
        if(pDecCont->refBufSupport)
        {
            u32 picId0 = 0;
            u32 picId1 = 0;
            u32 flags = pDecCont->bMC ? REFBU_DONT_USE_STATS : 0;
            const u8 *picId0Valid = NULL;
            const u8 *picId1Valid = NULL;
            u32 isIntraFrame = IS_I_SLICE(pSliceHeader->sliceType);
            refbuMode_e refBuffMode;

#ifdef DEC_REFBU_DONT_USE_STATS
            flags = REFBU_DONT_USE_STATS;
#endif

            if(pSliceHeader->fieldPicFlag)
                refBuffMode = REFBU_FIELD;
            else if(pSps->mbAdaptiveFrameFieldFlag)
                refBuffMode = REFBU_MBAFF;
            else
                refBuffMode = REFBU_FRAME;

            /* Find first valid reference picture for picId0 */
            for (i = 0 ; i < 16 ; ++i)
            {
                picId0 = i;
                picId0Valid = h264bsdGetRefPicDataVlcMode(dpb + 1, picId0, 0);
                if(picId0Valid != NULL)
                    break;
            }
            /* Find 2nd valid reference picture for picId1 */
            for (++i ; i < 16 ; ++i)
            {
                picId1 = i;
                picId1Valid = h264bsdGetRefPicDataVlcMode(dpb + 1, picId1, 0);
                if(picId1Valid != NULL)
                    break;
            }

            /* If picId0 is not valid, just tell reference buffer that this is
               an Intra frame, which will implicitly disable buffering. */
            if(picId0Valid == NULL)
            {
                isIntraFrame = 1;
            }
            else if ((pPps->numRefIdxL0Active > 1) && (picId1Valid != NULL))
            {
                flags |= REFBU_MULTIPLE_REF_FRAMES;
            }

            if(picId1Valid == NULL)
            {
                /* make sure that double buffer uses valid picId */
                picId1 = picId0;
            }

#ifdef DEC_REFBU_INTERLACED_DISABLE
            if(pDecCont->bMC && (!pSps->frameMbsOnlyFlag))
                flags = REFBU_DISABLE;
#endif

            RefbuSetup(&pDecCont->refBufferCtrl, pDecCont->h264Regs,
                       refBuffMode, isIntraFrame,
                       IS_B_SLICE(pSliceHeader->sliceType),
                       picId0, picId1, flags );
        }

        if (pDecCont->storage.enable2ndChroma &&
            !pDecCont->storage.activeSps->monoChrome)
        {
            SetDecRegister(pDecCont->h264Regs, HWIF_CH_8PIX_ILEAV_E, 1);
            if(pSliceHeader->fieldPicFlag && pSliceHeader->bottomFieldFlag)
            {
                SetDecRegister(pDecCont->h264Regs, HWIF_DEC_CH8PIX_BASE,
                   pAsicBuff->outBuffer->busAddress + dpb->ch2Offset+
                   pSps->picWidthInMbs * 16);
            }
            else
            {
                SetDecRegister(pDecCont->h264Regs, HWIF_DEC_CH8PIX_BASE,
                   pAsicBuff->outBuffer->busAddress + dpb->ch2Offset);
            }
        }
        else
            SetDecRegister(pDecCont->h264Regs, HWIF_CH_8PIX_ILEAV_E, 0);

        if (!pDecCont->keepHwReserved)
        {

            if(pDecCont->pp.ppInstance != NULL &&
               pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING &&
               pDecCont->pp.decPpIf.usePipeline)
            {
                /* reserved both DEC and PP hardware for pipeline */
                reserveRet = DWLReserveHwPipe(pDecCont->dwl, &pDecCont->coreID);
            }
            else
            {
                reserveRet = DWLReserveHw(pDecCont->dwl, &pDecCont->coreID);
            }

            if(reserveRet != DWL_OK)
            {
                /* Decrement usage for DPB buffers */
                DecrementDPBRefCount(&pStorage->dpb[1]);

                return X170_DEC_HW_RESERVED;
            }
        }

        pDecCont->asicRunning = 1;

        if(pDecCont->pp.ppInstance != NULL &&
           pDecCont->pp.ppInfo.multiBuffer == 0 &&
           pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING)
        {
            /* legacy single buffer mode */
            DecPpInterface *decPpIf = &pDecCont->pp.decPpIf;

            TRACE_PP_CTRL("H264RunAsic: PP Run\n");

            decPpIf->inwidth = pSps->picWidthInMbs * 16;
            decPpIf->inheight = pSps->picHeightInMbs * 16;
            decPpIf->croppedW = decPpIf->inwidth;
            decPpIf->croppedH = decPpIf->inheight;

            if(pSps->frameMbsOnlyFlag)
            {
                decPpIf->picStruct = DECPP_PIC_FRAME_OR_TOP_FIELD;
            }
            else
            {
                /* interlaced */
                if( pDecCont->dpbMode == DEC_DPB_FRAME )                
                    decPpIf->picStruct = DECPP_PIC_TOP_AND_BOT_FIELD_FRAME;
                else if ( pDecCont->dpbMode == DEC_DPB_INTERLACED_FIELD )
                    decPpIf->picStruct = DECPP_PIC_TOP_AND_BOT_FIELD;                    
                /* TODO: missing field? is this OK? */
            }

            decPpIf->littleEndian =
                GetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUT_ENDIAN);
            decPpIf->wordSwap =
                GetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUTSWAP32_E);
            decPpIf->tiledInputMode = pDecCont->tiledReferenceEnable;
            decPpIf->progressiveSequence =
                pDecCont->storage.activeSps->frameMbsOnlyFlag;

            if(decPpIf->usePipeline)
            {
                decPpIf->inputBusLuma = dpb->currentOut->data->busAddress;
            }
            else
            {
                if (pStorage->outView == pStorage->view)
                    decPpIf->inputBusLuma =
                        dpb->outBuf[dpb->outIndexR].data->busAddress;
                else /* starting pp for another view */
                    decPpIf->inputBusLuma =
                        pStorage->dpbs[pStorage->outView]->
                        outBuf[pStorage->dpbs[pStorage->outView]->outIndexR].
                        data->busAddress;
            }

            decPpIf->inputBusChroma =
                decPpIf->inputBusLuma + decPpIf->inwidth * decPpIf->inheight;

            if(decPpIf->picStruct != DECPP_PIC_FRAME_OR_TOP_FIELD)
            { 
                if( pDecCont->dpbMode == DEC_DPB_FRAME )
                {
                    decPpIf->bottomBusLuma = decPpIf->inputBusLuma + 
                        decPpIf->inwidth;
                    decPpIf->bottomBusChroma = decPpIf->inputBusChroma + 
                        decPpIf->inwidth;
                }
                if( pDecCont->dpbMode == DEC_DPB_INTERLACED_FIELD )
                {
                    decPpIf->bottomBusLuma = decPpIf->inputBusLuma + 
                        (decPpIf->inwidth*decPpIf->inheight >> 1);
                    decPpIf->bottomBusChroma = decPpIf->inputBusChroma + 
                        (decPpIf->inwidth*decPpIf->inheight >> 2);
                }
            }
            else
            {
                decPpIf->bottomBusLuma = (u32) (-1);
                decPpIf->bottomBusChroma = (u32) (-1);
            }

            pDecCont->pp.PPDecStart(pDecCont->pp.ppInstance, decPpIf);
        }


        if(pDecCont->pp.ppInstance != NULL &&
           pDecCont->pp.ppInfo.multiBuffer == 1 &&
           pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING)
        {
            /* multibuffer mode */
            DecPpInterface *decPpIf = &pDecCont->pp.decPpIf;

            TRACE_PP_CTRL("H264RunAsic: PP Run\n");

            decPpIf->inwidth = pSps->picWidthInMbs * 16;
            decPpIf->inheight = pSps->picHeightInMbs * 16;
            decPpIf->croppedW = decPpIf->inwidth;
            decPpIf->croppedH = decPpIf->inheight;

            if(pSps->frameMbsOnlyFlag)
            {
                decPpIf->picStruct = DECPP_PIC_FRAME_OR_TOP_FIELD;
            }
            else
            {
                /* interlaced */
                if( pDecCont->dpbMode == DEC_DPB_FRAME )                
                    decPpIf->picStruct = DECPP_PIC_TOP_AND_BOT_FIELD_FRAME;
                else if ( pDecCont->dpbMode == DEC_DPB_INTERLACED_FIELD )
                    decPpIf->picStruct = DECPP_PIC_TOP_AND_BOT_FIELD; 
            }

            decPpIf->littleEndian =
                GetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUT_ENDIAN);
            decPpIf->wordSwap =
                GetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUTSWAP32_E);
            decPpIf->tiledInputMode = pDecCont->tiledReferenceEnable;
            decPpIf->progressiveSequence =
                pDecCont->storage.activeSps->frameMbsOnlyFlag;

            if(decPpIf->usePipeline)
            {
                u32 picID;
                decPpIf->inputBusLuma = pStorage->dpb->currentOut->data->busAddress;
                picID = h264PpMultiAddPic(pDecCont, pStorage->dpb->currentOut->data);

                ASSERT(picID <= pDecCont->pp.multiMaxId);
                TRACE_PP_CTRL("H264RunAsic: PP Multibuffer index = %d\n", picID);
                decPpIf->bufferIndex = picID;
                decPpIf->displayIndex = (u32)(-1);                

            }
            else
            {
                u32 picID;
                u32 view = pStorage->view;
                DWLLinearMem_t *pic =
                   (DWLLinearMem_t *)pDecCont->pp.queuedPicToPp[pStorage->view];

                if (pic == NULL)
                {
                    view = !pStorage->view;
                    pic = (DWLLinearMem_t *)pDecCont->pp.queuedPicToPp[!pStorage->view];
                }

                /* send queued picture to PP */
                ASSERT(pic != NULL);

                decPpIf->inputBusLuma = pic->busAddress;
                picID = h264PpMultiAddPic(pDecCont, pic);

                ASSERT(picID <= pDecCont->pp.multiMaxId);
                TRACE_PP_CTRL("H264RunAsic: PP Multibuffer index = %d\n", picID);
                decPpIf->bufferIndex = picID;
                decPpIf->displayIndex = (u32)(-1);

                /* queue curent picture for next PP processing */
                if(pStorage->view == 0 ? !pStorage->secondField :
                                         !pStorage->baseOppositeFieldPic)
                {
                    pDecCont->pp.queuedPicToPp[pStorage->view] =
                        pStorage->dpbs[pStorage->view]->currentOut->data;
                }
                else
                {
                    /* do not queue first field */
                    pDecCont->pp.queuedPicToPp[view] = NULL;
                }
            }

            decPpIf->inputBusChroma =
                decPpIf->inputBusLuma + decPpIf->inwidth * decPpIf->inheight;

            if(decPpIf->picStruct != DECPP_PIC_FRAME_OR_TOP_FIELD)
            {
                if( pDecCont->dpbMode == DEC_DPB_FRAME )
                {
                    decPpIf->bottomBusLuma = decPpIf->inputBusLuma + 
                        decPpIf->inwidth;
                    decPpIf->bottomBusChroma = decPpIf->inputBusChroma + 
                        decPpIf->inwidth;
                }
                if( pDecCont->dpbMode == DEC_DPB_INTERLACED_FIELD )
                {
                    decPpIf->bottomBusLuma = decPpIf->inputBusLuma + 
                        (decPpIf->inwidth*decPpIf->inheight >> 1);
                    decPpIf->bottomBusChroma = decPpIf->inputBusChroma + 
                        (decPpIf->inwidth*decPpIf->inheight >> 2);
                }
            }
            else
            {
                decPpIf->bottomBusLuma = (u32) (-1);
                decPpIf->bottomBusChroma = (u32) (-1);
            }

            pDecCont->pp.PPDecStart(pDecCont->pp.ppInstance, decPpIf);
        }

        if(pDecCont->h264ProfileSupport != H264_BASELINE_PROFILE)
        {
            u32 coreID = pDecCont->bMC ? pDecCont->coreID : 0;

            h264PrepareScaleList(pDecCont);

            SetDecRegister(pDecCont->h264Regs, HWIF_QTABLE_BASE,
                           pAsicBuff->cabacInit[coreID].busAddress);
        }

        H264FlushRegs(pDecCont);

        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_E, 1);

        if(pDecCont->bMC)
        {
            h264CopyPocToHw(pDecCont);
            h264MCSetHwRdyCallback(pDecCont);
        }

        DWLEnableHW(pDecCont->dwl, pDecCont->coreID, 4 * 1,
                    pDecCont->h264Regs[1]);

        if(pDecCont->bMC)
        {
            /* reset shadow HW status reg values so that we dont end up writing
             * some garbage to next core regs */
            SetDecRegister(pDecCont->h264Regs, HWIF_DEC_E, 0);
            SetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ_STAT, 0);
            SetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ, 0);

            pDecCont->asicRunning = 0;
            pDecCont->keepHwReserved = 0;
            return DEC_8190_IRQ_RDY;
        }
    }
    else    /* buffer was empty and now we restart with new stream values */
    {
        ASSERT(!pDecCont->bMC);

        h264StreamPosUpdate(pDecCont);

        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, 4 * 5, pDecCont->h264Regs[5]);
        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, 4 * 6, pDecCont->h264Regs[6]);
        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, 4 * 12, pDecCont->h264Regs[12]);

        /* start HW by clearing IRQ_BUFFER_EMPTY status bit */
        DWLEnableHW(pDecCont->dwl, pDecCont->coreID, 4 * 1, pDecCont->h264Regs[1]);
    }

    ret = DWLWaitHwReady(pDecCont->dwl, pDecCont->coreID, (u32) DEC_X170_TIMEOUT_LENGTH);

    if(ret != DWL_HW_WAIT_OK)
    {
        ERROR_PRINT("DWLWaitHwReady");
        DEBUG_PRINT(("DWLWaitHwReady returned: %d\n", ret));

        /* reset HW */
        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ_STAT, 0);
        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ, 0);
        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_E, 0);

        DWLDisableHW(pDecCont->dwl, pDecCont->coreID, 4 * 1, pDecCont->h264Regs[1]);

        /* Wait for PP to end also */
        if(pDecCont->pp.ppInstance != NULL &&
           (pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING ||
            pDecCont->pp.decPpIf.ppStatus == DECPP_PIC_NOT_FINISHED))
        {
            pDecCont->pp.decPpIf.ppStatus = DECPP_PIC_READY;

            TRACE_PP_CTRL("H264RunAsic: PP Wait for end\n");

            pDecCont->pp.PPDecWaitEnd(pDecCont->pp.ppInstance);

            TRACE_PP_CTRL("H264RunAsic: PP Finished\n");
        }

        pDecCont->asicRunning = 0;
        pDecCont->keepHwReserved = 0;
        DWLReleaseHw(pDecCont->dwl, pDecCont->coreID);

        /* Decrement usage for DPB buffers */
        DecrementDPBRefCount(&pStorage->dpb[1]);

        return (ret == DWL_HW_WAIT_ERROR) ?
            X170_DEC_SYSTEM_ERROR : X170_DEC_TIMEOUT;
    }

    H264RefreshRegs(pDecCont);

    /* React to the HW return value */

    asic_status = GetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ_STAT);

    SetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ_STAT, 0);
    SetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ, 0);    /* just in case */

    /* any B stuff detected by HW? (note reg name changed from
     * b_detected to pic_inf) */
    if(GetDecRegister(pDecCont->h264Regs, HWIF_DEC_PIC_INF))
    {
        ASSERT(pDecCont->h264ProfileSupport != H264_BASELINE_PROFILE);
        if((pDecCont->h264ProfileSupport != H264_BASELINE_PROFILE) &&
           (pAsicBuff->enableDmvAndPoc == 0))
        {
            DEBUG_PRINT(("HW Detected B slice in baseline profile stream!!!\n"));
            pAsicBuff->enableDmvAndPoc = 1;
        }

        SetDecRegister(pDecCont->h264Regs, HWIF_DEC_PIC_INF, 0);
    }

    if(!(asic_status & DEC_8190_IRQ_BUFFER))
    {
        /* HW done, release it! */
        pDecCont->asicRunning = 0;

        pDecCont->keepHwReserved = 0;
        /* Wait for PP to end also */
        if(pDecCont->pp.ppInstance != NULL &&
           (pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING ||
            pDecCont->pp.decPpIf.ppStatus == DECPP_PIC_NOT_FINISHED))
        {
            /* first field of a frame finished -> decode second one before
             * waiting for PP to finish */
            if ((pDecCont->storage.numViews == 0 ?
                    pDecCont->storage.secondField :
                    pDecCont->storage.sliceHeader->fieldPicFlag &&
                    pDecCont->storage.view == 0) &&
                /*!pDecCont->storage.numViews &&*/
                (asic_status & DEC_8190_IRQ_RDY))
            {
                pDecCont->pp.decPpIf.ppStatus = DECPP_PIC_NOT_FINISHED;
                pDecCont->keepHwReserved = 1;
            }
            else
            {
                pDecCont->pp.decPpIf.ppStatus = DECPP_PIC_READY;

                TRACE_PP_CTRL("H264RunAsic: PP Wait for end\n");

                pDecCont->pp.PPDecWaitEnd(pDecCont->pp.ppInstance);

                TRACE_PP_CTRL("H264RunAsic: PP Finished\n");
            }
        }

        if (!pDecCont->keepHwReserved)
            DWLReleaseHw(pDecCont->dwl, pDecCont->coreID);

        /* Decrement usage for DPB buffers */
        DecrementDPBRefCount(&pStorage->dpb[1]);
    }

    /* update reference buffer stat */
    if(pDecCont->asicRunning == 0 &&
        (asic_status & DEC_8190_IRQ_RDY))
    {
        u32 *pMv = pAsicBuff->outBuffer->virtualAddress;
        u32 tmp = dpb->dirMvOffset;

        if(pSliceHeader->fieldPicFlag && pSliceHeader->bottomFieldFlag)
        {
             tmp += dpb->picSizeInMbs * 32;
        }
        pMv = (u32 *) ((u8*)pMv + tmp);

        if(pDecCont->refBufSupport &&
           !IS_B_SLICE(pDecCont->storage.sliceHeader->sliceType))
        {
            RefbuMvStatistics(&pDecCont->refBufferCtrl, pDecCont->h264Regs,
                              pMv, pAsicBuff->enableDmvAndPoc,
                              IS_IDR_NAL_UNIT(pDecCont->storage.prevNalUnit));
        }
        else if ( pDecCont->refBufSupport )
        {
            RefbuMvStatisticsB( &pDecCont->refBufferCtrl, pDecCont->h264Regs );
        }
    }

    if(!pDecCont->rlcMode)
    {
        u32 last_read_address;
        u32 bytes_processed;
        const u32 start_address =
            pDecCont->hwStreamStartBus & (~DEC_X170_ALIGN_MASK);
        const u32 offset_bytes =
            pDecCont->hwStreamStartBus & DEC_X170_ALIGN_MASK;

        last_read_address =
            GetDecRegister(pDecCont->h264Regs, HWIF_RLC_VLC_BASE);

        bytes_processed = last_read_address - start_address;

        DEBUG_PRINT(("HW updated stream position: %08x\n"
                     "           processed bytes: %8d\n"
                     "     of which offset bytes: %8d\n",
                     last_read_address, bytes_processed, offset_bytes));

        if(asic_status & DEC_8190_IRQ_BUFFER)
        {
            pDecCont->pHwStreamStart += pDecCont->hwLength;
            pDecCont->hwStreamStartBus += pDecCont->hwLength;
            pDecCont->hwLength = 0; /* no bytes left */
        }
        else if(!(asic_status & DEC_8190_IRQ_ASO))
        {
            /* from start of the buffer add what HW has decoded */

            /* end - start smaller or equal than maximum */
            if((bytes_processed - offset_bytes) > pDecCont->hwLength)
            {

                if(!(asic_status & ( DEC_8190_IRQ_ABORT |
                                     DEC_8190_IRQ_ERROR |
                                     DEC_8190_IRQ_TIMEOUT |
                                     DEC_8190_IRQ_BUS )))
                {
                    DEBUG_PRINT(("New stream position out of range!\n"));
                    DEBUG_PRINT(("Buffer size %d, and next start would be %d\n",
                                 pDecCont->hwLength,
                                 (bytes_processed - offset_bytes)));
                    ASSERT(0);
                }

                /* consider all buffer processed */
                pDecCont->pHwStreamStart += pDecCont->hwLength;
                pDecCont->hwStreamStartBus += pDecCont->hwLength;
                pDecCont->hwLength = 0; /* no bytes left */
            }
            else
            {
                pDecCont->hwLength -= (bytes_processed - offset_bytes);
                pDecCont->pHwStreamStart += (bytes_processed - offset_bytes);
                pDecCont->hwStreamStartBus += (bytes_processed - offset_bytes);
            }

        }
        /* else will continue decoding from the beginning of buffer */

        pDecCont->streamPosUpdated = 1;
    }

    return asic_status;
}

/*------------------------------------------------------------------------------
    Function name : PrepareMvData
    Description   :

    Return type   : void
    Argument      : storage_t * pStorage
    Argument      : DecAsicBuffers_t * pAsicBuff
------------------------------------------------------------------------------*/
void PrepareMvData(storage_t * pStorage, DecAsicBuffers_t * pAsicBuff)
{
    const mbStorage_t *pMb = pStorage->mb;

    u32 mbs = pStorage->picSizeInMbs;
    u32 tmp, n;
    u32 *pMvCtrl, *pMvCtrlBase = pAsicBuff->mv.virtualAddress;
    const u32 *pMbCtrl = pAsicBuff->mbCtrl.virtualAddress;

    if(pAsicBuff->wholePicConcealed != 0)
    {
        if(pMb->mbType_asic == P_Skip)
        {
            tmp = pMb->refID[0];

            pMvCtrl = pMvCtrlBase;

            for(n = mbs; n > 0; n--)
            {
                *pMvCtrl = tmp;
                pMvCtrl += (ASIC_MB_MV_BUFFER_SIZE / 4);
            }
        }

        return;
    }

    for(n = mbs; n > 0; n--, pMb++, pMvCtrlBase += (ASIC_MB_MV_BUFFER_SIZE / 4),
        pMbCtrl += (ASIC_MB_CTRL_BUFFER_SIZE / 4))
    {
        const mv_t *mv = pMb->mv;

        pMvCtrl = pMvCtrlBase;

        switch (pMb->mbType_asic)
        {
        case P_Skip:
            tmp = pMb->refID[0];
            *pMvCtrl++ = tmp;
            break;

        case P_L0_16x16:
            tmp = ((u32) (mv[0].hor & ASIC_HOR_MV_MASK)) << ASIC_HOR_MV_OFFSET;
            tmp |= ((u32) (mv[0].ver & ASIC_VER_MV_MASK)) << ASIC_VER_MV_OFFSET;
            tmp |= pMb->refID[0];
            *pMvCtrl++ = tmp;

            break;
        case P_L0_L0_16x8:
            tmp = ((u32) (mv[0].hor & ASIC_HOR_MV_MASK)) << ASIC_HOR_MV_OFFSET;
            tmp |= ((u32) (mv[0].ver & ASIC_VER_MV_MASK)) << ASIC_VER_MV_OFFSET;
            tmp |= pMb->refID[0];

            *pMvCtrl++ = tmp;

            tmp = ((u32) (mv[8].hor & ASIC_HOR_MV_MASK)) << ASIC_HOR_MV_OFFSET;
            tmp |= ((u32) (mv[8].ver & ASIC_VER_MV_MASK)) << ASIC_VER_MV_OFFSET;
            tmp |= pMb->refID[1];

            *pMvCtrl++ = tmp;

            break;
        case P_L0_L0_8x16:
            tmp = ((u32) (mv[0].hor & ASIC_HOR_MV_MASK)) << ASIC_HOR_MV_OFFSET;
            tmp |= ((u32) (mv[0].ver & ASIC_VER_MV_MASK)) << ASIC_VER_MV_OFFSET;
            tmp |= pMb->refID[0];

            *pMvCtrl++ = tmp;

            tmp = ((u32) (mv[4].hor & ASIC_HOR_MV_MASK)) << ASIC_HOR_MV_OFFSET;
            tmp |= ((u32) (mv[4].ver & ASIC_VER_MV_MASK)) << ASIC_VER_MV_OFFSET;
            tmp |= pMb->refID[1];

            *pMvCtrl++ = tmp;

            break;
        case P_8x8:
        case P_8x8ref0:
            {
                u32 i;
                u32 subMbType = *pMbCtrl;

                for(i = 0; i < 4; i++)
                {
                    switch ((subMbType >> (u32) (27 - 2 * i)) & 0x03)
                    {
                    case MB_SP_8x8:
                        tmp =
                            ((u32) ((*mv).hor & ASIC_HOR_MV_MASK)) <<
                            ASIC_HOR_MV_OFFSET;
                        tmp |=
                            ((u32) ((*mv).ver & ASIC_VER_MV_MASK)) <<
                            ASIC_VER_MV_OFFSET;
                        tmp |= pMb->refID[i];
                        *pMvCtrl++ = tmp;
                        mv += 4;
                        break;

                    case MB_SP_8x4:
                        tmp =
                            ((u32) ((*mv).hor & ASIC_HOR_MV_MASK)) <<
                            ASIC_HOR_MV_OFFSET;
                        tmp |=
                            ((u32) ((*mv).ver & ASIC_VER_MV_MASK)) <<
                            ASIC_VER_MV_OFFSET;
                        tmp |= pMb->refID[i];
                        *pMvCtrl++ = tmp;
                        mv += 2;

                        tmp =
                            ((u32) ((*mv).hor & ASIC_HOR_MV_MASK)) <<
                            ASIC_HOR_MV_OFFSET;
                        tmp |=
                            ((u32) ((*mv).ver & ASIC_VER_MV_MASK)) <<
                            ASIC_VER_MV_OFFSET;
                        tmp |= pMb->refID[i];
                        *pMvCtrl++ = tmp;
                        mv += 2;
                        break;

                    case MB_SP_4x8:
                        tmp =
                            ((u32) ((*mv).hor & ASIC_HOR_MV_MASK)) <<
                            ASIC_HOR_MV_OFFSET;
                        tmp |=
                            ((u32) ((*mv).ver & ASIC_VER_MV_MASK)) <<
                            ASIC_VER_MV_OFFSET;
                        tmp |= pMb->refID[i];
                        *pMvCtrl++ = tmp;
                        mv++;
                        tmp =
                            ((u32) ((*mv).hor & ASIC_HOR_MV_MASK)) <<
                            ASIC_HOR_MV_OFFSET;
                        tmp |=
                            ((u32) ((*mv).ver & ASIC_VER_MV_MASK)) <<
                            ASIC_VER_MV_OFFSET;
                        tmp |= pMb->refID[i];
                        *pMvCtrl++ = tmp;
                        mv += 3;

                        break;

                    case MB_SP_4x4:
                        {
                            u32 j;

                            for(j = 4; j > 0; j--)
                            {
                                tmp =
                                    ((u32) ((*mv).hor & ASIC_HOR_MV_MASK)) <<
                                    ASIC_HOR_MV_OFFSET;
                                tmp |=
                                    ((u32) ((*mv).ver & ASIC_VER_MV_MASK)) <<
                                    ASIC_VER_MV_OFFSET;
                                tmp |= pMb->refID[i];
                                *pMvCtrl++ = tmp;
                                mv++;
                            }
                        }
                        break;
                    default:
                        ASSERT(0);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}

/*------------------------------------------------------------------------------
    Function name : PrepareIntra4x4ModeData
    Description   :

    Return type   : void
    Argument      : storage_t * pStorage
    Argument      : DecAsicBuffers_t * pAsicBuff
------------------------------------------------------------------------------*/
void PrepareIntra4x4ModeData(storage_t * pStorage, DecAsicBuffers_t * pAsicBuff)
{
    u32 n;
    u32 mbs = pStorage->picSizeInMbs;
    u32 *pIntraPred, *pIntraPredBase = pAsicBuff->intraPred.virtualAddress;
    const mbStorage_t *pMb = pStorage->mb;

    if(pAsicBuff->wholePicConcealed != 0)
    {
        return;
    }

    /* write out "INTRA4x4 mode" to ASIC */
    for(n = mbs; n > 0;
        n--, pMb++, pIntraPredBase += (ASIC_MB_I4X4_BUFFER_SIZE / 4))
    {
        u32 tmp = 0, block;

        pIntraPred = pIntraPredBase;

        if(h264bsdMbPartPredMode(pMb->mbType_asic) != PRED_MODE_INTRA4x4)
            continue;

        for(block = 0; block < 16; block++)
        {
            u8 mode = pMb->intra4x4PredMode_asic[block];

            tmp <<= 4;
            tmp |= mode;

            if(block == 7)
            {
                *pIntraPred++ = tmp;
                tmp = 0;
            }
        }
        *pIntraPred++ = tmp;
    }
}

/*------------------------------------------------------------------------------
    Function name   : PrepareRlcCount
    Description     :
    Return type     : void
    Argument        : storage_t * pStorage
    Argument        : DecAsicBuffers_t * pAsicBuff
------------------------------------------------------------------------------*/
void PrepareRlcCount(storage_t * pStorage, DecAsicBuffers_t * pAsicBuff)
{
    u32 n;
    u32 mbs = pStorage->picSizeInMbs;
    u32 *pMbCtrl = pAsicBuff->mbCtrl.virtualAddress + 1;

    if(pAsicBuff->wholePicConcealed != 0)
    {
        return;
    }

    for(n = mbs - 1; n > 0; n--, pMbCtrl += (ASIC_MB_CTRL_BUFFER_SIZE / 4))
    {
        u32 tmp = (*(pMbCtrl + (ASIC_MB_CTRL_BUFFER_SIZE / 4))) << 4;

        (*pMbCtrl) |= (tmp >> 23);
    }
}

/*------------------------------------------------------------------------------
    Function name   : H264RefreshRegs
    Description     :
    Return type     : void
    Argument        : decContainer_t * pDecCont
------------------------------------------------------------------------------*/
static void H264RefreshRegs(decContainer_t * pDecCont)
{
    i32 i;
    u32 offset = 0x0;

    u32 *decRegs = pDecCont->h264Regs;

    for(i = DEC_X170_REGISTERS; i > 0; --i)
    {
        *decRegs++ = DWLReadReg(pDecCont->dwl, pDecCont->coreID, offset);
        offset += 4;
    }
}

/*------------------------------------------------------------------------------
    Function name   : H264FlushRegs
    Description     :
    Return type     : void
    Argument        : decContainer_t * pDecCont
------------------------------------------------------------------------------*/
static void H264FlushRegs(decContainer_t * pDecCont)
{
    i32 i;
    u32 offset = 0x04;
    const u32 *decRegs = pDecCont->h264Regs + 1;    /* Don't write ID reg */

#ifdef TRACE_START_MARKER
    /* write ID register to trigger logic analyzer */
    DWLWriteReg(pDecCont->dwl, pDecCont->coreID, 0x00, ~0);
#endif

    /* don not start HW by this flush */
    ASSERT(GetDecRegister(pDecCont->h264Regs, HWIF_DEC_E) == 0);
    ASSERT(GetDecRegister(pDecCont->h264Regs, HWIF_DEC_IRQ_STAT) == 0);

    for(i = DEC_X170_REGISTERS; i > 1; --i)
    {
        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, offset, *decRegs);
        decRegs++;
        offset += 4;
    }
}

/*------------------------------------------------------------------------------
    Function name : H264SetupVlcRegs
    Description   : set up registers for vlc mode

    Return type   :
    Argument      : container
------------------------------------------------------------------------------*/
void H264SetupVlcRegs(decContainer_t * pDecCont)
{
    u32 tmp, i;
    u32 longTermflags = 0;
    u32 validFlags = 0;
    u32 longTermTmp = 0;
    i32 diffPoc, currPoc, itmp;

    const seqParamSet_t *pSps = pDecCont->storage.activeSps;
    const sliceHeader_t *pSliceHeader = pDecCont->storage.sliceHeader;
    const picParamSet_t *pPps = pDecCont->storage.activePps;
    const dpbStorage_t *pDpb = pDecCont->storage.dpb;
    const storage_t *pStorage = &pDecCont->storage;
    const u32 is8190 = pDecCont->is8190;

    SetDecRegister(pDecCont->h264Regs, HWIF_DEC_OUT_DIS, 0);

    SetDecRegister(pDecCont->h264Regs, HWIF_RLC_MODE_E, 0);

    SetDecRegister(pDecCont->h264Regs, HWIF_INIT_QP, pPps->picInitQp);

    SetDecRegister(pDecCont->h264Regs, HWIF_REFIDX0_ACTIVE,
                   pPps->numRefIdxL0Active);

    SetDecRegister(pDecCont->h264Regs, HWIF_REF_FRAMES, pSps->numRefFrames);

    i = 0;
    while(pSps->maxFrameNum >> i)
    {
        i++;
    }
    SetDecRegister(pDecCont->h264Regs, HWIF_FRAMENUM_LEN, i - 1);

    SetDecRegister(pDecCont->h264Regs, HWIF_FRAMENUM,
        pSliceHeader->frameNum & ~(pDecCont->frameNumMask));

    SetDecRegister(pDecCont->h264Regs, HWIF_CONST_INTRA_E,
                   pPps->constrainedIntraPredFlag);

    SetDecRegister(pDecCont->h264Regs, HWIF_FILT_CTRL_PRES,
                   pPps->deblockingFilterControlPresentFlag);

    SetDecRegister(pDecCont->h264Regs, HWIF_RDPIC_CNT_PRES,
                   pPps->redundantPicCntPresentFlag);

    SetDecRegister(pDecCont->h264Regs, HWIF_REFPIC_MK_LEN,
                   pSliceHeader->decRefPicMarking.strmLen);

    SetDecRegister(pDecCont->h264Regs, HWIF_IDR_PIC_E,
                   IS_IDR_NAL_UNIT(pStorage->prevNalUnit));
    SetDecRegister(pDecCont->h264Regs, HWIF_IDR_PIC_ID, pSliceHeader->idrPicId);
    SetDecRegister(pDecCont->h264Regs, HWIF_PPS_ID, pStorage->activePpsId);
    SetDecRegister(pDecCont->h264Regs, HWIF_POC_LENGTH,
                   pSliceHeader->pocLengthHw);

    /* reference picture flags */

    /* TODO separate fields */
    if(pSliceHeader->fieldPicFlag)
    {
        ASSERT(pDecCont->h264ProfileSupport != H264_BASELINE_PROFILE);

        for(i = 0; i < 32; i++)
        {
            longTermTmp = pDpb->buffer[i / 2].status[i & 1] == 3;
            longTermflags = longTermflags << 1 | longTermTmp;

            tmp = h264bsdGetRefPicDataVlcMode(pDpb, i, 1) != NULL;
            validFlags = validFlags << 1 | tmp;
        }
        SetDecRegister(pDecCont->h264Regs, HWIF_REFER_LTERM_E, longTermflags);
        SetDecRegister(pDecCont->h264Regs, HWIF_REFER_VALID_E, validFlags);
    }
    else
    {
        for(i = 0; i < 16; i++)
        {
            u32 n = is8190 ? i : pStorage->dpb[0].list[i];

            longTermTmp = pDpb->buffer[n].status[0] == 3 &&
                pDpb->buffer[n].status[1] == 3;
            longTermflags = longTermflags << 1 | longTermTmp;

            tmp = h264bsdGetRefPicDataVlcMode(pDpb, n, 0) != NULL;
            validFlags = validFlags << 1 | tmp;
        }
        SetDecRegister(pDecCont->h264Regs, HWIF_REFER_LTERM_E,
                       longTermflags << 16);
        SetDecRegister(pDecCont->h264Regs, HWIF_REFER_VALID_E,
                       validFlags << 16);
    }

    if(pSliceHeader->fieldPicFlag)
    {
        currPoc = pStorage->poc->picOrderCnt[pSliceHeader->bottomFieldFlag];
    }
    else
    {
        currPoc = MIN(pStorage->poc->picOrderCnt[0],
                      pStorage->poc->picOrderCnt[1]);
    }
    for(i = 0; i < 16; i++)
    {
        u32 n = is8190 ? i : pDpb->list[i];

        if(pDpb->buffer[n].status[0] == 3 || pDpb->buffer[n].status[1] == 3)
        {
            SetDecRegister(pDecCont->h264Regs, refPicNum[i],
                           pDpb->buffer[n].picNum);
        }
        else
        {
            /* if frameNum workaround activated and bit 12 of frameNum is
             * non-zero -> modify frame numbers of reference pictures so
             * that reference picture list reordering works as usual */
            if (pSliceHeader->frameNum & pDecCont->frameNumMask)
            {
                i32 tmp = pDpb->buffer[n].frameNum - pDecCont->frameNumMask;
                if (tmp < 0) tmp += pSps->maxFrameNum;
                SetDecRegister(pDecCont->h264Regs, refPicNum[i], tmp);
            }
            else
                SetDecRegister(pDecCont->h264Regs, refPicNum[i],
                               pDpb->buffer[n].frameNum);

        }
        diffPoc = ABS(pDpb->buffer[i].picOrderCnt[0] - currPoc);
        itmp = ABS(pDpb->buffer[i].picOrderCnt[1] - currPoc);

        if(pDecCont->asicBuff->refPicList[i])
            pDecCont->asicBuff->refPicList[i] |=
                (diffPoc < itmp ? 0x1 : 0) | (pDpb->buffer[i].isFieldPic ? 0x2 : 0);
    }

    if(pDecCont->h264ProfileSupport != H264_BASELINE_PROFILE)
    {
        h264PreparePOC(pDecCont);

        SetDecRegister(pDecCont->h264Regs, HWIF_CABAC_E,
                       pPps->entropyCodingModeFlag);
    }

    h264StreamPosUpdate(pDecCont);
}

/*------------------------------------------------------------------------------
    Function name   : H264InitRefPicList1
    Description     :
    Return type     : void
    Argument        : decContainer_t *pDecCont
    Argument        : u32 *list0
    Argument        : u32 *list1
------------------------------------------------------------------------------*/
void H264InitRefPicList1(decContainer_t * pDecCont, u32 * list0, u32 * list1)
{
    u32 tmp, i;
    u32 idx, idx0, idx1, idx2;
    i32 refPoc;
    const storage_t *pStorage = &pDecCont->storage;
    const dpbPicture_t *pic;

    refPoc = MIN(pStorage->poc->picOrderCnt[0], pStorage->poc->picOrderCnt[1]);
    i = 0;

    pic = pStorage->dpb->buffer;
    while(IS_SHORT_TERM_FRAME(pic[list0[i]]) &&
          MIN(pic[list0[i]].picOrderCnt[0], pic[list0[i]].picOrderCnt[1]) <
          refPoc)
        i++;

    idx0 = i;

    while(IS_SHORT_TERM_FRAME(pic[list0[i]]))
        i++;

    idx1 = i;

    while(IS_LONG_TERM_FRAME(pic[list0[i]]))
        i++;

    idx2 = i;

    /* list L1 */
    for(i = idx0, idx = 0; i < idx1; i++, idx++)
        list1[idx] = list0[i];

    for(i = 0; i < idx0; i++, idx++)
        list1[idx] = list0[i];

    for(i = idx1; idx < 16; idx++, i++)
        list1[idx] = list0[i];

    if(idx2 > 1)
    {
        tmp = 0;
        for(i = 0; i < idx2; i++)
        {
            tmp += (list0[i] != list1[i]) ? 1 : 0;
        }
        /* lists are equal -> switch list1[0] and list1[1] */
        if(!tmp)
        {
            i = list1[0];
            list1[0] = list1[1];
            list1[1] = i;
        }
    }
}

/*------------------------------------------------------------------------------
    Function name   : H264InitRefPicList1F
    Description     :
    Return type     : void
    Argument        : decContainer_t *pDecCont
    Argument        : u32 *list0
    Argument        : u32 *list1
------------------------------------------------------------------------------*/
void H264InitRefPicList1F(decContainer_t * pDecCont, u32 * list0, u32 * list1)
{
    u32 i;
    u32 idx, idx0, idx1;
    i32 refPoc;
    const storage_t *pStorage = &pDecCont->storage;
    const dpbPicture_t *pic;

    refPoc =
        pStorage->poc->picOrderCnt[pStorage->sliceHeader[0].bottomFieldFlag];
    i = 0;

    pic = pStorage->dpb->buffer;
    while(IS_SHORT_TERM_FRAME_F(pic[list0[i]]) &&
          FIELD_POC(pic[list0[i]]) <= refPoc)
        i++;

    idx0 = i;

    while(IS_SHORT_TERM_FRAME_F(pic[list0[i]]))
        i++;

    idx1 = i;

    /* list L1 */
    for(i = idx0, idx = 0; i < idx1; i++, idx++)
        list1[idx] = list0[i];

    for(i = 0; i < idx0; i++, idx++)
        list1[idx] = list0[i];

    for(i = idx1; idx < 16; idx++, i++)
        list1[idx] = list0[i];

}

/*------------------------------------------------------------------------------
    Function name   : H264InitRefPicList
    Description     :
    Return type     : void
    Argument        : decContainer_t *pDecCont
------------------------------------------------------------------------------*/
void H264InitRefPicList(decContainer_t * pDecCont)
{
    sliceHeader_t *pSliceHeader = pDecCont->storage.sliceHeader;
    pocStorage_t *poc = pDecCont->storage.poc;
    dpbStorage_t *dpb = pDecCont->storage.dpb;
    u32 i, isIdr;
    u32 list0[34] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33
    };
    u32 list1[34] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33
    };
    u32 listP[34] =
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33
    };

    /* determine if alternate view is IDR -> reference only inter-view ref
     * pic */
    isIdr = IS_IDR_NAL_UNIT(pDecCont->storage.prevNalUnit);

    /* B lists */
    if(!pDecCont->rlcMode)
    {
        if(pSliceHeader->fieldPicFlag)
        {
            /* list 0 */
            ShellSortF(dpb, list0, 1,
                       poc->picOrderCnt[pSliceHeader->bottomFieldFlag]);
            if (pDecCont->storage.view && !pDecCont->storage.nonInterViewRef)
            {
                i = 0;
                while (!isIdr && IS_REF_FRAME_F(dpb->buffer[list0[i]])) i++;
                list0[i] = 15;
            }
            /* list 1 */
            H264InitRefPicList1F(pDecCont, list0, list1);
            for(i = 0; i < 16; i++)
            {
                SetDecRegister(pDecCont->h264Regs, refPicList0[i], list0[i]);
                SetDecRegister(pDecCont->h264Regs, refPicList1[i], list1[i]);
            }
        }
        else
        {
            /* list 0 */
            ShellSort(dpb, list0, 1,
                      MIN(poc->picOrderCnt[0], poc->picOrderCnt[1]));
            if (pDecCont->storage.view && !pDecCont->storage.nonInterViewRef)
            {
                i = 0;
                while (!isIdr && IS_REF_FRAME(dpb->buffer[list0[i]])) i++;
                list0[i] = 15;
            }
            /* list 1 */
            H264InitRefPicList1(pDecCont, list0, list1);
            for(i = 0; i < 16; i++)
            {
                SetDecRegister(pDecCont->h264Regs, refPicList0[i], list0[i]);
                SetDecRegister(pDecCont->h264Regs, refPicList1[i], list1[i]);
            }
        }
    }

    /* P list */
    if(pSliceHeader->fieldPicFlag)
    {
        if(!pDecCont->rlcMode)
        {
            ShellSortF(dpb, listP, 0, 0);
            if (pDecCont->storage.view && !pDecCont->storage.nonInterViewRef)
            {
                i = 0;
                while (!isIdr && IS_REF_FRAME_F(dpb->buffer[listP[i]])) i++;
                listP[i] = 15;
            }
            for(i = 0; i < 16; i++)
            {
                SetDecRegister(pDecCont->h264Regs, refPicListP[i], listP[i]);

                /* copy to dpb for error handling purposes */
                dpb[0].list[i] = listP[i];
                dpb[1].list[i] = listP[i];
            }
        }
    }
    else
    {
        ShellSort(dpb, listP, 0, 0);
        if (pDecCont->storage.view && !pDecCont->storage.nonInterViewRef)
        {
            i = 0;
            while (!isIdr && IS_REF_FRAME(dpb->buffer[listP[i]])) i++;
            listP[i] = 15;
        }
        for(i = 0; i < 16; i++)
        {
            if(!pDecCont->rlcMode)
                SetDecRegister(pDecCont->h264Regs, refPicListP[i], listP[i]);
            /* copy to dpb for error handling purposes */
            dpb[0].list[i] = listP[i];
            dpb[1].list[i] = listP[i];
        }
    }
}

/*------------------------------------------------------------------------------
    Function name : h264StreamPosUpdate
    Description   : Set stream base and length related registers

    Return type   :
    Argument      : container
------------------------------------------------------------------------------*/
void h264StreamPosUpdate(decContainer_t * pDecCont)
{
    u32 tmp;
    const u8 *pTmp;

    DEBUG_PRINT(("h264StreamPosUpdate:\n"));
    tmp = 0;

    pTmp = pDecCont->pHwStreamStart;

    /* NAL start prefix in stream start is 0 0 0 or 0 0 1 */
    if(!(*pDecCont->pHwStreamStart + *(pDecCont->pHwStreamStart + 1)))
    {
        if(*(pDecCont->pHwStreamStart + 2) < 2)
        {
            tmp = 1;
        }
    }

    /* stuff related to frameNum workaround, if used -> advance stream by
     * start code prefix and HW is set to decode in NAL unit mode */
    if (tmp && pDecCont->forceNalMode)
    {
        tmp = 0;
        /* skip start code prefix */
        while (*pTmp++ == 0);

        pDecCont->hwStreamStartBus += pTmp - pDecCont->pHwStreamStart;
        pDecCont->pHwStreamStart = pTmp;
        pDecCont->hwLength -= pTmp - pDecCont->pHwStreamStart;
    }

    DEBUG_PRINT(("\tByte stream   %8d\n", tmp));
    SetDecRegister(pDecCont->h264Regs, HWIF_START_CODE_E, tmp);

    /* bit offset if base is unaligned */
    tmp = (pDecCont->hwStreamStartBus & DEC_X170_ALIGN_MASK) * 8;

    DEBUG_PRINT(("\tStart bit pos %8d\n", tmp));

    SetDecRegister(pDecCont->h264Regs, HWIF_STRM_START_BIT, tmp);

    pDecCont->hwBitPos = tmp;

    tmp = pDecCont->hwStreamStartBus;   /* unaligned base */
    tmp &= (~DEC_X170_ALIGN_MASK);  /* align the base */

    DEBUG_PRINT(("\tStream base   %08x\n", tmp));
    SetDecRegister(pDecCont->h264Regs, HWIF_RLC_VLC_BASE, tmp);

    tmp = pDecCont->hwLength;   /* unaligned stream */
    tmp += pDecCont->hwBitPos / 8;  /* add the alignmet bytes */

    DEBUG_PRINT(("\tStream length %8d\n", tmp));
    SetDecRegister(pDecCont->h264Regs, HWIF_STREAM_LEN, tmp);

}

/*------------------------------------------------------------------------------
    Function name : H264PrepareCabacInitTables
    Description   : Prepare CABAC initialization tables

    Return type   : void
    Argument      : u32 *cabacInit
------------------------------------------------------------------------------*/
void H264PrepareCabacInitTables(u32 *cabacInit)
{
    ASSERT(cabacInit != NULL);
    (void) DWLmemcpy(cabacInit, cabacInitValues, 4 * 460 * 2);
}

void h264CopyPocToHw(decContainer_t *pDecCont)
{
    const u32 coreID = pDecCont->bMC ? pDecCont->coreID : 0;
    DWLLinearMem_t *cabacMem = pDecCont->asicBuff->cabacInit + coreID;
    i32 *pocBase = (i32 *) ((u8 *) cabacMem->virtualAddress +
                       ASIC_CABAC_INIT_BUFFER_SIZE);

    if(pDecCont->asicBuff->enableDmvAndPoc)
        DWLmemcpy(pocBase, pDecCont->poc, sizeof(pDecCont->poc));
}

void h264PreparePOC(decContainer_t *pDecCont)
{
    const sliceHeader_t *pSliceHeader = pDecCont->storage.sliceHeader;
    const seqParamSet_t *pSps = pDecCont->storage.activeSps;
    const dpbStorage_t *pDpb = pDecCont->storage.dpb;
    const pocStorage_t *poc = pDecCont->storage.poc;

    int i;

    i32 *pocBase, currPoc;

    if(pDecCont->bMC)
    {
        /* For multicore we cannot write to HW buffer
         * as the core is not yet reserved.
         */
        pocBase = pDecCont->poc;
    }
    else
    {
        DWLLinearMem_t *cabacMem = pDecCont->asicBuff->cabacInit;
        pocBase = (i32 *) ((u8 *) cabacMem->virtualAddress +
                           ASIC_CABAC_INIT_BUFFER_SIZE);
    }

    if(!pDecCont->asicBuff->enableDmvAndPoc)
    {
        SetDecRegister(pDecCont->h264Regs, HWIF_PICORD_COUNT_E, 0);
        return;
    }

    SetDecRegister(pDecCont->h264Regs, HWIF_PICORD_COUNT_E, 1);

    if (pSliceHeader->fieldPicFlag)
    {
        currPoc = poc->picOrderCnt[pSliceHeader->bottomFieldFlag ? 1 : 0];
    }
    else
    {
        currPoc = MIN(poc->picOrderCnt[0], poc->picOrderCnt[1]);
    }

    for (i = 0; i < 32; i++)
    {
        *pocBase++ = pDpb->buffer[i / 2].picOrderCnt[i & 0x1];
    }

    if (pSliceHeader[0].fieldPicFlag || !pSps->mbAdaptiveFrameFieldFlag)
    {
        *pocBase++ = currPoc;
        *pocBase = 0;
    }
    else
    {
        *pocBase++ = poc->picOrderCnt[0];
        *pocBase++ = poc->picOrderCnt[1];
    }
}

void h264PrepareScaleList(decContainer_t *pDecCont)
{
    const picParamSet_t *pPps = pDecCont->storage.activePps;
    const u8 (*scalingList)[64];
    const u32 coreID = pDecCont->bMC ? pDecCont->coreID : 0;
    u32 i, j, tmp;
    u32 *p;

    DWLLinearMem_t *cabacMem = pDecCont->asicBuff->cabacInit + coreID;

    if (!pPps->scalingMatrixPresentFlag)
        return;

    p = (u32 *) ((u8 *) cabacMem->virtualAddress +
                ASIC_CABAC_INIT_BUFFER_SIZE +
                ASIC_POC_BUFFER_SIZE);

    scalingList = pPps->scalingList;
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 4; j++)
        {
            tmp = (scalingList[i][4 * j + 0] << 24) |
                  (scalingList[i][4 * j + 1] << 16) |
                  (scalingList[i][4 * j + 2] << 8) |
                  (scalingList[i][4 * j + 3]);
            *p++ = tmp;
        }
    }
    for (i = 6; i < 8; i++)
    {
        for (j = 0; j < 16; j++)
        {
            tmp = (scalingList[i][4 * j + 0] << 24) |
                  (scalingList[i][4 * j + 1] << 16) |
                  (scalingList[i][4 * j + 2] << 8) |
                  (scalingList[i][4 * j + 3]);
            *p++ = tmp;
        }
    }
}

void H264UpdateAfterHwRdy(decContainer_t *pDecCont, u32 *h264Regs)
{
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;

    /* any B stuff detected by HW? (note reg name changed from
     * b_detected to pic_inf)
     */
    if(GetDecRegister(h264Regs, HWIF_DEC_PIC_INF))
    {
        if((pDecCont->h264ProfileSupport != H264_BASELINE_PROFILE) &&
           (pAsicBuff->enableDmvAndPoc == 0))
        {
            DEBUG_PRINT(("HW Detected B slice in baseline profile stream!!!\n"));
            pAsicBuff->enableDmvAndPoc = 1;
        }

        SetDecRegister(h264Regs, HWIF_DEC_PIC_INF, 0);
    }

    /*  TODO(atna): Do we support RefBuff in multicore? */
}
