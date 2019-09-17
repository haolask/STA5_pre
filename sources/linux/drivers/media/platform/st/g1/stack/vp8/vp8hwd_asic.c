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
#include "dwl.h"
#include "regdrv.h"
#include "vp8decmc_internals.h"
#include "vp8hwd_container.h"
#include "vp8hwd_asic.h"
#include "vp8hwd_buffer_queue.h"
#include "vp8hwd_debug.h"
#include "tiledref.h"
#include "commonconfig.h"

#ifdef _TRACE_PP_CTRL
#ifndef TRACE_PP_CTRL
#include <stdio.h>
#define TRACE_PP_CTRL(...)          printf(__VA_ARGS__)
#endif
#else
#define TRACE_PP_CTRL(...)
#endif

#ifdef ASIC_TRACE_SUPPORT
#endif

#define SCAN(i)         HWIF_SCAN_MAP_ ## i

static const u32 ScanTblRegId[16] = { 0 /* SCAN(0) */ ,
    SCAN(1), SCAN(2), SCAN(3), SCAN(4), SCAN(5), SCAN(6), SCAN(7), SCAN(8),
    SCAN(9), SCAN(10), SCAN(11), SCAN(12), SCAN(13), SCAN(14), SCAN(15)
};

#define BASE(i)         HWIF_DCT_STRM ## i ## _BASE
static const u32 DctBaseId[] = { HWIF_VP6HWPART2_BASE,
    BASE(1), BASE(2), BASE(3), BASE(4), BASE(5), BASE(6), BASE(7) };

#define OFFSET(i)         HWIF_DCT ## i ## _START_BIT
static const u32 DctStartBit[] = { HWIF_STRM_START_BIT,
    OFFSET(1), OFFSET(2), OFFSET(3), OFFSET(4),
    OFFSET(5), OFFSET(6), OFFSET(7) };

#define TAP(i, j)       HWIF_PRED_BC_TAP_ ## i ## _ ## j

static const u32 TapRegId[8][4] = {
    {TAP(0, 0), TAP(0, 1), TAP(0, 2), TAP(0, 3)},
    {TAP(1, 0), TAP(1, 1), TAP(1, 2), TAP(1, 3)},
    {TAP(2, 0), TAP(2, 1), TAP(2, 2), TAP(2, 3)},
    {TAP(3, 0), TAP(3, 1), TAP(3, 2), TAP(3, 3)},
    {TAP(4, 0), TAP(4, 1), TAP(4, 2), TAP(4, 3)},
    {TAP(5, 0), TAP(5, 1), TAP(5, 2), TAP(5, 3)},
    {TAP(6, 0), TAP(6, 1), TAP(6, 2), TAP(6, 3)},
    {TAP(7, 0), TAP(7, 1), TAP(7, 2), TAP(7, 3)}
};

static const u32 mcFilter[8][6] =
{
    { 0,  0,  128,    0,   0,  0 },
    { 0, -6,  123,   12,  -1,  0 },
    { 2, -11, 108,   36,  -8,  1 },
    { 0, -9,   93,   50,  -6,  0 },
    { 3, -16,  77,   77, -16,  3 },
    { 0, -6,   50,   93,  -9,  0 },
    { 1, -8,   36,  108, -11,  2 },
    { 0, -1,   12,  123,  -6,  0 } };

#define probSameAsLastOffset                (0)
#define probModeOffset                      (4*8)
#define probMvIsShortOffset                 (38*8)
#define probMvSignOffset                    (probMvIsShortOffset + 2)
#define probMvSizeOffset                    (39*8)
#define probMvShortOffset                   (41*8)

#define probDCFirstOffset                   (43*8)
#define probACFirstOffset                   (46*8)
#define probACZeroRunFirstOffset            (64*8)
#define probDCRestOffset                    (65*8)
#define probACRestOffset                    (71*8)
#define probACZeroRunRestOffset             (107*8)

#define huffmanTblDCOffset                  (43*8)
#define huffmanTblACZeroRunOffset           (huffmanTblDCOffset + 48)
#define huffmanTblACOffset                  (huffmanTblACZeroRunOffset + 48)

#define PROB_TABLE_SIZE  1208

#define DEC_MODE_VP7  9
#define DEC_MODE_VP8 10

static void VP8HwdAsicRefreshRegs(VP8DecContainer_t * pDecCont);
static void VP8HwdAsicFlushRegs(VP8DecContainer_t * pDecCont);
static void vp8hwdUpdateOutBase(VP8DecContainer_t *pDecCont);
static DWLLinearMem_t* GetPrevRef(VP8DecContainer_t *pDecCont);
static DWLLinearMem_t* GetGoldenRef(VP8DecContainer_t *pDecCont);
static DWLLinearMem_t* GetAltRef(VP8DecContainer_t *pDecCont);

enum {

    VP8HWD_BUFFER_BASIC = 0,
    VP8HWD_BUFFER_STRIDE = 1,
    VP8HWD_BUFFER_CUSTOM = 2

};

/* VP7 QP LUTs */
static const u16 YDcQLookup[128] = 
{
     4,    4,    5,    6,    6,    7,    8,    8,
     9,   10,   11,   12,   13,   14,   15,   16, 
    17,   18,   19,   20,   21,   22,   23,   23,
    24,   25,   26,   27,   28,   29,   30,   31, 
    32,   33,   33,   34,   35,   36,   36,   37,
    38,   39,   39,   40,   41,   41,   42,   43, 
    43,   44,   45,   45,   46,   47,   48,   48,
    49,   50,   51,   52,   53,   53,   54,   56, 
    57,   58,   59,   60,   62,   63,   65,   66,
    68,   70,   72,   74,   76,   79,   81,   84, 
    87,   90,   93,   96,  100,  104,  108,  112,
   116,  121,  126,  131,  136,  142,  148,  154, 
   160,  167,  174,  182,  189,  198,  206,  215,
   224,  234,  244,  254,  265,  277,  288,  301, 
   313,  327,  340,  355,  370,  385,  401,  417,
   434,  452,  470,  489,  509,  529,  550,  572
};

static const u16 YAcQLookup[128] = 
{
     4,    4,    5,    5,    6,    6,    7,    8,
     9,   10,   11,   12,   13,   15,   16,   17, 
    19,   20,   22,   23,   25,   26,   28,   29,
    31,   32,   34,   35,   37,   38,   40,   41, 
    42,   44,   45,   46,   48,   49,   50,   51,
    53,   54,   55,   56,   57,   58,   59,   61, 
    62,   63,   64,   65,   67,   68,   69,   70,
    72,   73,   75,   76,   78,   80,   82,   84, 
    86,   88,   91,   93,   96,   99,  102,  105,
   109,  112,  116,  121,  125,  130,  135,  140, 
   146,  152,  158,  165,  172,  180,  188,  196,
   205,  214,  224,  234,  245,  256,  268,  281, 
   294,  308,  322,  337,  353,  369,  386,  404,
   423,  443,  463,  484,  506,  529,  553,  578, 
   604,  631,  659,  688,  718,  749,  781,  814,
   849,  885,  922,  960, 1000, 1041, 1083, 1127 

};

static const u16 Y2DcQLookup[128] =
{
     7,    9,   11,   13,   15,   17,   19,   21,
    23,   26,   28,   30,   33,   35,   37,   39, 
    42,   44,   46,   48,   51,   53,   55,   57,
    59,   61,   63,   65,   67,   69,   70,   72, 
    74,   75,   77,   78,   80,   81,   83,   84,
    85,   87,   88,   89,   90,   92,   93,   94, 
    95,   96,   97,   99,  100,  101,  102,  104,
   105,  106,  108,  109,  111,  113,  114,  116, 
   118,  120,  123,  125,  128,  131,  134,  137,
   140,  144,  148,  152,  156,  161,  166,  171, 
   176,  182,  188,  195,  202,  209,  217,  225,
   234,  243,  253,  263,  274,  285,  297,  309, 
   322,  336,  350,  365,  381,  397,  414,  432,
   450,  470,  490,  511,  533,  556,  579,  604, 
   630,  656,  684,  713,  742,  773,  805,  838, 
   873,  908,  945,  983, 1022, 1063, 1105, 1148 
};

static const u16 Y2AcQLookup[128] =
{
     7,    9,   11,   13,   16,   18,   21,   24,
    26,   29,   32,   35,   38,   41,   43,   46, 
    49,   52,   55,   58,   61,   64,   66,   69,
    72,   74,   77,   79,   82,   84,   86,   88, 
    91,   93,   95,   97,   98,  100,  102,  104,
   105,  107,  109,  110,  112,  113,  115,  116, 
   117,  119,  120,  122,  123,  125,  127,  128,
   130,  132,  134,  136,  138,  141,  143,  146, 
   149,  152,  155,  158,  162,  166,  171,  175,
   180,  185,  191,  197,  204,  210,  218,  226, 
   234,  243,  252,  262,  273,  284,  295,  308,
   321,  335,  350,  365,  381,  398,  416,  435, 
   455,  476,  497,  520,  544,  569,  595,  622,
   650,  680,  711,  743,  776,  811,  848,  885, 
   925,  965, 1008, 1052, 1097, 1144, 1193, 1244,
  1297, 1351, 1407, 1466, 1526, 1588, 1652, 1719 
};

static const u16 UvDcQLookup[128] = 
{
     4,    4,    5,    6,    6,    7,    8,    8,
     9,   10,   11,   12,   13,   14,   15,   16, 
    17,   18,   19,   20,   21,   22,   23,   23,
    24,   25,   26,   27,   28,   29,   30,   31, 
    32,   33,   33,   34,   35,   36,   36,   37,
    38,   39,   39,   40,   41,   41,   42,   43, 
    43,   44,   45,   45,   46,   47,   48,   48,
    49,   50,   51,   52,   53,   53,   54,   56, 
    57,   58,   59,   60,   62,   63,   65,   66,
    68,   70,   72,   74,   76,   79,   81,   84, 
    87,   90,   93,   96,  100,  104,  108,  112,
   116,  121,  126,  131,  132,  132,  132,  132,  
   132,  132,  132,  132,  132,  132,  132,  132,
   132,  132,  132,  132,  132,  132,  132,  132,  
   132,  132,  132,  132,  132,  132,  132,  132,
   132,  132,  132,  132,  132,  132,  132,  132
};


static const u16 UvAcQLookup[128] = 
{
     4,    4,    5,    5,    6,    6,    7,    8,
     9,   10,   11,   12,   13,   15,   16,   17, 
    19,   20,   22,   23,   25,   26,   28,   29,
    31,   32,   34,   35,   37,   38,   40,   41, 
    42,   44,   45,   46,   48,   49,   50,   51,
    53,   54,   55,   56,   57,   58,   59,   61, 
    62,   63,   64,   65,   67,   68,   69,   70,
    72,   73,   75,   76,   78,   80,   82,   84, 
    86,   88,   91,   93,   96,   99,  102,  105,
   109,  112,  116,  121,  125,  130,  135,  140, 
   146,  152,  158,  165,  172,  180,  188,  196,
   205,  214,  224,  234,  245,  256,  268,  281, 
   294,  308,  322,  337,  353,  369,  386,  404,
   423,  443,  463,  484,  506,  529,  553,  578, 
   604,  631,  659,  688,  718,  749,  781,  814,
   849,  885,  922,  960, 1000, 1041, 1083, 1127
};

#define CLIP3(l, h, v) ((v) < (l) ? (l) : ((v) > (h) ? (h) : (v)))
#define MB_MULTIPLE(x)  (((x)+15)&~15)

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicInit
    Description     : 
    Return type     : void 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicInit(VP8DecContainer_t * pDecCont)
{

    DWLmemset(pDecCont->vp8Regs, 0, sizeof(pDecCont->vp8Regs));

    SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_MODE,
       pDecCont->decMode == VP8HWD_VP7 ?  DEC_MODE_VP7 : DEC_MODE_VP8);

    SetCommonConfigRegs(pDecCont->vp8Regs);

}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicAllocateMem
    Description     : 
    Return type     : i32 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
i32 VP8HwdAsicAllocateMem(VP8DecContainer_t * pDecCont)
{

    const void *dwl = pDecCont->dwl;
    i32 dwl_ret = DWL_OK;
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;
    u32 i = 0;
    u32 numMbs = 0;
    u32 memorySize = 0;
    /*  A segment map memory is allocated at the end of the probabilities for VP8 */

    if(pDecCont->decMode == VP8HWD_VP8)
    {
        numMbs = (pAsicBuff->width>>4)*(pAsicBuff->height>>4);
        memorySize = (numMbs+3)>>2; /* We fit 4 MBs on data into every full byte */
        memorySize = 64*((memorySize + 63) >> 6); /* Round up to next multiple of 64 bytes */
        pAsicBuff->segmentMapSize = memorySize;
    }

    memorySize += PROB_TABLE_SIZE;

    for(i=0; i < pDecCont->numCores; i++)
    {
        dwl_ret = DWLMallocLinearNamed(dwl, memorySize, &pAsicBuff->probTbl[i],
                        "prob table from VP8HwdAsicAllocateMem");
        if(dwl_ret != DWL_OK)
        {
            break;
        }
        if(pDecCont->decMode == VP8HWD_VP8)
        {
            pAsicBuff->segmentMap[i].virtualAddress =
                pAsicBuff->probTbl[i].virtualAddress + PROB_TABLE_SIZE/4;
            DWLmemset(pAsicBuff->segmentMap[i].virtualAddress,
                  0, pAsicBuff->segmentMapSize);
            pAsicBuff->segmentMap[i].busAddress =
                pAsicBuff->probTbl[i].busAddress + PROB_TABLE_SIZE;
        }
    }

    if(dwl_ret != DWL_OK)
    {
        VP8HwdAsicReleaseMem(pDecCont);
        return -1;
    }

    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicReleaseMem
    Description     : 
    Return type     : void 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicReleaseMem(VP8DecContainer_t * pDecCont)
{
    const void *dwl = pDecCont->dwl;
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;
    u32 i;

    for(i=0; i < pDecCont->numCores; i++)
    {
        if(pAsicBuff->probTbl[i].virtualAddress != NULL)
        {
            DWLFreeLinear(dwl, &pAsicBuff->probTbl[i]);
            DWLmemset(&pAsicBuff->probTbl[i], 0, sizeof(pAsicBuff->probTbl[i]));
        }
    }
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicAllocatePictures
    Description     : 
    Return type     : i32 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
i32 VP8HwdAsicAllocatePictures(VP8DecContainer_t * pDecCont)
{
    const void *dwl = pDecCont->dwl;
    i32 dwl_ret;
    u32 i, count;
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;

    u32 pict_buff_size;
    u32 lumaStride;
    u32 chromaStride;
    u32 lumaSize;
    u32 chromaSize;
    u32 height;

    u32 numMbs;
    u32 memorySize;

    /* allocate segment map */

    numMbs = (pAsicBuff->width>>4)*(pAsicBuff->height>>4);

#if 0
    memorySize = (numMbs+3)>>2; /* We fit 4 MBs on data into every full byte */
    memorySize = 64*((memorySize + 63) >> 6); /* Round up to next multiple of 64 bytes */

    if( pDecCont->decoder.decMode != VP8HWD_VP7 )
    {

        dwl_ret = DWLMallocLinearNamed(dwl, memorySize, &pAsicBuff->segmentMap,
                        "segment map from VP8HwdAsicAllocatePictures");

        if(dwl_ret != DWL_OK)
        {
            VP8HwdAsicReleasePictures(pDecCont);
            return -1;
        }

        pAsicBuff->segmentMapSize = memorySize;
        SetDecRegister(pDecCont->vp8Regs, HWIF_SEGMENT_BASE,
                       pAsicBuff->segmentMap.busAddress);
        DWLmemset(pAsicBuff->segmentMap.virtualAddress,
                  0, pAsicBuff->segmentMapSize);
    }
#endif
    DWLmemset(pAsicBuff->pictures, 0, sizeof(pAsicBuff->pictures));

    count = pDecCont->numBuffers;
    
    lumaStride = pAsicBuff->lumaStride ? pAsicBuff->lumaStride : pAsicBuff->width;
    chromaStride = pAsicBuff->chromaStride ? pAsicBuff->chromaStride : pAsicBuff->width;
        
    if( lumaStride < pAsicBuff->width )
    {
        VP8HwdAsicReleasePictures(pDecCont);
        return -1;  
    }

    if( chromaStride < pAsicBuff->width )
    {
        VP8HwdAsicReleasePictures(pDecCont);
        return -1;  
    }
        
    if (!pDecCont->sliceHeight)
        height = pAsicBuff->height;
    else
        height = (pDecCont->sliceHeight + 1) * 16;
                
    lumaSize = lumaStride * height; 
    chromaSize = chromaStride * (height / 2);             
    pAsicBuff->chromaBufOffset = lumaSize;
    
    pict_buff_size = lumaSize + chromaSize;
  
/*        
    if (!pDecCont->sliceHeight)
        pict_buff_size = lumaSize + chromaSize; pAsicBuff->width * pAsicBuff->height * 3 / 2;
    else
        pict_buff_size = pAsicBuff->width *
            (pDecCont->sliceHeight + 1) * 16 * 3 / 2;
            */            

    pAsicBuff->syncMcOffset = pict_buff_size;
    pict_buff_size += 16; /* space for multicore status fields */

    pDecCont->bq = VP8HwdBufferQueueInitialize(pDecCont->numBuffers);
    if( pDecCont->bq == NULL )
    {
        VP8HwdAsicReleasePictures(pDecCont);
        return -1;
    }

    if (pDecCont->pp.ppInstance != NULL)
    {
        pDecCont->pp.ppInfo.tiledMode =
            pDecCont->tiledReferenceEnable;
        pDecCont->pp.PPConfigQuery(pDecCont->pp.ppInstance,
                                &pDecCont->pp.ppInfo);
    }
    if (!pDecCont->userMem &&
        !(pDecCont->intraOnly && pDecCont->pp.ppInfo.pipelineAccepted))
    {
        if(pAsicBuff->customBuffers)
        {
            for (i = 0; i < count; i++)
            {
                pAsicBuff->pictures[i].virtualAddress = pAsicBuff->userMem.pPicBufferY[i];
                pAsicBuff->pictures[i].busAddress = pAsicBuff->userMem.picBufferBusAddrY[i];                
                pAsicBuff->picturesC[i].virtualAddress = pAsicBuff->userMem.pPicBufferC[i];
                pAsicBuff->picturesC[i].busAddress = pAsicBuff->userMem.picBufferBusAddrC[i];                
            }
        }
        else
        {
            for (i = 0; i < count; i++)
            {
                dwl_ret = DWLMallocRefFrmNamed(dwl, pict_buff_size, &pAsicBuff->pictures[i],
                            "picture buffer from VP8HwdAsicAllocatePictures");
                if(dwl_ret != DWL_OK)
                {
                    VP8HwdAsicReleasePictures(pDecCont);
                    return -1;
                }
                /* setup pointers to chroma addresses */
                pAsicBuff->picturesC[i].virtualAddress = 
                    pAsicBuff->pictures[i].virtualAddress +
                    (pAsicBuff->chromaBufOffset/4);
                pAsicBuff->picturesC[i].busAddress = 
                    pAsicBuff->pictures[i].busAddress + 
                    pAsicBuff->chromaBufOffset;

                {
                    void *base = (char*)pAsicBuff->pictures[i].virtualAddress
                            + pAsicBuff->syncMcOffset;
                    (void) DWLmemset(base, ~0, 16);
                }
            }
        }
    }

    /* webp/snapshot -> allocate memory for HW above row storage */
    SetDecRegister(pDecCont->vp8Regs, HWIF_WEBP_E, pDecCont->intraOnly );
    if (pDecCont->intraOnly && 0) /* Not used for HW */
    {
        u32 tmpW = 0;
        ASSERT(count == 1);

        /* width of the memory in macroblocks, has to be multiple of amount
         * of mbs stored/load per access */
        /* TODO: */
        while (tmpW < MAX_SNAPSHOT_WIDTH) tmpW += 256;

        /* TODO: check amount of memory per macroblock for each HW block */
        memorySize = tmpW /* num mbs */ *
                     (  2 + 2 /* streamd (cbf + intra pred modes) */ +
                       16 + 2*8 /* intra pred (luma + cb + cr) */ +
                       16*8 + 2*8*4 /* filterd */ );
        dwl_ret = DWLMallocLinearNamed(dwl, memorySize, &pAsicBuff->pictures[1],
                        "WEBP pictures[1] from "__FILE__);
        if(dwl_ret != DWL_OK)
        {
            VP8HwdAsicReleasePictures(pDecCont);
            return -1;
        }
        DWLmemset(pAsicBuff->pictures[1].virtualAddress, 0, memorySize);
        SetDecRegister(pDecCont->vp8Regs, HWIF_REFER6_BASE, 
            pAsicBuff->pictures[1].busAddress);
        SetDecRegister(pDecCont->vp8Regs, HWIF_REFER2_BASE, 
            pAsicBuff->pictures[1].busAddress + 4*tmpW);
        SetDecRegister(pDecCont->vp8Regs, HWIF_REFER3_BASE, 
            pAsicBuff->pictures[1].busAddress + 36*tmpW);
    }

    if (pDecCont->hwEcSupport)
    {
        /* allocate memory for motion vectors used for error concealment */
        memorySize = numMbs * 16 * sizeof(u32);
        dwl_ret = DWLMallocLinearNamed(dwl, memorySize, &pAsicBuff->mvs[0],
                        "motion vectors 0 from VP8HwdAsicAllocatePictures");
        if(dwl_ret != DWL_OK)
        {
            VP8HwdAsicReleasePictures(pDecCont);
            return -1;
        }
        dwl_ret = DWLMallocLinearNamed(dwl, memorySize, &pAsicBuff->mvs[1],
                        "motion vectors 1 from VP8HwdAsicAllocatePictures");
        if(dwl_ret != DWL_OK)
        {
            VP8HwdAsicReleasePictures(pDecCont);
            return -1;
        }
    }
    pAsicBuff->mvsCurr = pAsicBuff->mvsRef = 0;

    SetDecRegister(pDecCont->vp8Regs, HWIF_PIC_MB_WIDTH, (pAsicBuff->width / 16)&0x1FF);
    SetDecRegister(pDecCont->vp8Regs, HWIF_PIC_MB_HEIGHT_P, (pAsicBuff->height / 16)&0xFF);
    SetDecRegister(pDecCont->vp8Regs, HWIF_PIC_MB_W_EXT, (pAsicBuff->width / 16)>>9);
    SetDecRegister(pDecCont->vp8Regs, HWIF_PIC_MB_H_EXT, (pAsicBuff->height / 16)>>8);

    SetDecRegister(pDecCont->vp8Regs, HWIF_Y_STRIDE_POW2, 
        pAsicBuff->stridesUsed ? pAsicBuff->lumaStridePow2 : 0);
    SetDecRegister(pDecCont->vp8Regs, HWIF_C_STRIDE_POW2, 
        pAsicBuff->stridesUsed ? pAsicBuff->chromaStridePow2 : 0);

    /* Id for first frame. */
    pAsicBuff->outBufferI = VP8HwdBufferQueueGetBuffer(pDecCont->bq);
    pAsicBuff->outBuffer = &pAsicBuff->pictures[pAsicBuff->outBufferI];
    /* These need to point at something so use the output buffer */
    VP8HwdBufferQueueUpdateRef(pDecCont->bq,
        BQUEUE_FLAG_PREV | BQUEUE_FLAG_GOLDEN | BQUEUE_FLAG_ALT,
        pAsicBuff->outBufferI);
    if(pDecCont->intraOnly != HANTRO_TRUE)
    {
        VP8HwdBufferQueueAddRef(pDecCont->bq, VP8HwdBufferQueueGetPrevRef(pDecCont->bq));
        VP8HwdBufferQueueAddRef(pDecCont->bq, VP8HwdBufferQueueGetAltRef(pDecCont->bq));
        VP8HwdBufferQueueAddRef(pDecCont->bq, VP8HwdBufferQueueGetGoldenRef(pDecCont->bq));
    }
    return 0;
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicReleasePictures
    Description     : 
    Return type     : void 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicReleasePictures(VP8DecContainer_t * pDecCont)
{
    u32 i, count;
    const void *dwl = pDecCont->dwl;
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;

    if(pDecCont->streamConsumedCallback == NULL &&
       pDecCont->bq && pDecCont->intraOnly != HANTRO_TRUE)
    {
        /* Legacy single core: remove references made for the next decode. */
        VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                                   VP8HwdBufferQueueGetPrevRef(pDecCont->bq));
        VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                                   VP8HwdBufferQueueGetAltRef(pDecCont->bq));
        VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                                   VP8HwdBufferQueueGetGoldenRef(pDecCont->bq));
        VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                                   pDecCont->asicBuff->outBufferI);
    }

    if(pDecCont->bq)
    {
        if(pDecCont->intraOnly == HANTRO_TRUE)
        {
            VP8HwdBufferQueueRemoveRef(pDecCont->bq,
                                   pDecCont->asicBuff->outBufferI);
        }
        VP8HwdBufferQueueRelease(pDecCont->bq);
        pDecCont->bq = NULL;
    }

    if(!pAsicBuff->customBuffers)
    {
        count = pDecCont->numBuffers;
        for (i = 0; i < count; i++)
        {
            if(pAsicBuff->pictures[i].virtualAddress != NULL)
            {
                DWLFreeRefFrm(dwl, &pAsicBuff->pictures[i]);
            }
        }
    }

    if (pDecCont->intraOnly)
    {
        if(pAsicBuff->pictures[1].virtualAddress != NULL)
        {
            DWLFreeLinear(dwl, &pAsicBuff->pictures[1]);
        }
    }

    DWLmemset(pAsicBuff->pictures, 0, sizeof(pAsicBuff->pictures));

    if(pAsicBuff->mvs[0].virtualAddress != NULL)
        DWLFreeLinear(dwl, &pAsicBuff->mvs[0]);

    if(pAsicBuff->mvs[1].virtualAddress != NULL)
        DWLFreeLinear(dwl, &pAsicBuff->mvs[1]);

}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicInitPicture
    Description     : 
    Return type     : void 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicInitPicture(VP8DecContainer_t * pDecCont)
{

    vp8Decoder_t *dec = &pDecCont->decoder;
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;
    DWLHwConfig_t hwConfig;

    DWLReadAsicConfig(&hwConfig);

#ifdef SET_EMPTY_PICTURE_DATA   /* USE THIS ONLY FOR DEBUGGING PURPOSES */
    {
        i32 bgd = SET_EMPTY_PICTURE_DATA;
        i32 height = pDecCont->sliceHeight ? 16*pDecCont->sliceHeight : 
                     MB_MULTIPLE(dec->height);
        i32 wY = pAsicBuff->lumaStride ? 
            pAsicBuff->lumaStride : MB_MULTIPLE(dec->width);
        i32 wC = pAsicBuff->chromaStride ? 
            pAsicBuff->chromaStride : MB_MULTIPLE(dec->width);
        i32 sY = wY * height;
        i32 sC = wC * height / 2;
        if(pDecCont->userMem)
        {
            DWLmemset(pAsicBuff->userMem.pPicBufferY[0],
                      bgd, sY );
            DWLmemset(pAsicBuff->userMem.pPicBufferC[0],
                      bgd, sC );
        }
        else if (pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
        {
            DWLmemset(pAsicBuff->outBuffer->virtualAddress,
                      bgd, sY);
            DWLmemset(pAsicBuff->picturesC[pAsicBuff->outBufferI].virtualAddress,
                      bgd, sC );
        }
        else
        {
            DWLmemset(pAsicBuff->outBuffer->virtualAddress,
                      bgd, sY + sC);
        }
    }
#endif

    SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUT_DIS, 0);
    
    if (pDecCont->intraOnly && pDecCont->pp.ppInfo.pipelineAccepted)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUT_DIS, 1);
    }
    else if (!pDecCont->userMem && !pDecCont->sliceHeight)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUT_BASE,
                       pAsicBuff->outBuffer->busAddress);
                       
        if(pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_REFER6_BASE,
                    pAsicBuff->picturesC[pAsicBuff->outBufferI].busAddress);
        }

        if (!dec->keyFrame)
            /* previous picture */
            SetDecRegister(pDecCont->vp8Regs, HWIF_REFER0_BASE,
                           GetPrevRef(pDecCont)->busAddress);
        else /* chroma output base address */
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_REFER0_BASE,
                pAsicBuff->picturesC[pAsicBuff->outBufferI].busAddress );
        }
        
        /* Note: REFER1 reg conflicts with slice size, so absolutely not
         * applicable with webp */
        if((pAsicBuff->stridesUsed || pAsicBuff->customBuffers) && !pDecCont->intraOnly )
        {
            i32 idx = VP8HwdBufferQueueGetPrevRef(pDecCont->bq);
            SetDecRegister(pDecCont->vp8Regs, HWIF_REFER1_BASE,
                           pAsicBuff->picturesC[idx].busAddress);
        }

    }
    else
    {
        u32 sliceHeight;

        if (pDecCont->sliceHeight*16 > pAsicBuff->height)
            sliceHeight = pAsicBuff->height/16;
        else
            sliceHeight = pDecCont->sliceHeight;

        SetDecRegister(pDecCont->vp8Regs, HWIF_JPEG_SLICE_H, sliceHeight);

        vp8hwdUpdateOutBase(pDecCont);
    }

    if(pAsicBuff->stridesUsed)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_VP8_STRIDE_E, 1 );
    }
    if(pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_VP8_CH_BASE_E, 1 );
    }

    /* golden reference */
    SetDecRegister(pDecCont->vp8Regs, HWIF_REFER4_BASE,
                   GetGoldenRef(pDecCont)->busAddress);
    if(pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_REFER2_BASE,
            pAsicBuff->picturesC[
                VP8HwdBufferQueueGetGoldenRef(pDecCont->bq)].busAddress);
    }
    SetDecRegister(pDecCont->vp8Regs, HWIF_GREF_SIGN_BIAS,
                   dec->refFrameSignBias[0]);

    /* alternate reference */
    SetDecRegister(pDecCont->vp8Regs, HWIF_REFER5_BASE,
                   GetAltRef(pDecCont)->busAddress);
    if(pAsicBuff->stridesUsed || pAsicBuff->customBuffers)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_REFER3_BASE,
            pAsicBuff->picturesC[
                VP8HwdBufferQueueGetAltRef(pDecCont->bq)].busAddress);
    }
    SetDecRegister(pDecCont->vp8Regs, HWIF_AREF_SIGN_BIAS,
                   dec->refFrameSignBias[1]);

    SetDecRegister(pDecCont->vp8Regs, HWIF_PIC_INTER_E, !dec->keyFrame);


    /* mb skip mode [Codec X] */
    SetDecRegister(pDecCont->vp8Regs, HWIF_SKIP_MODE, !dec->coeffSkipMode );

    /* loop filter */
    SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_TYPE, 
        dec->loopFilterType);
    SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_SHARPNESS, 
        dec->loopFilterSharpness);

    if (!dec->segmentationEnabled)
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_0, 
            dec->loopFilterLevel);
    else if (dec->segmentFeatureMode) /* absolute mode */
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_0,
            dec->segmentLoopfilter[0]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_1,
            dec->segmentLoopfilter[1]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_2,
            dec->segmentLoopfilter[2]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_3,
            dec->segmentLoopfilter[3]);
    }
    else /* delta mode */
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_0,
            CLIP3((i32)0, (i32)63,
                  (i32)dec->loopFilterLevel + dec->segmentLoopfilter[0]));
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_1,
            CLIP3((i32)0, (i32)63,
                  (i32)dec->loopFilterLevel + dec->segmentLoopfilter[1]));
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_2,
            CLIP3((i32)0, (i32)63,
                  (i32)dec->loopFilterLevel + dec->segmentLoopfilter[2]));
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_LEVEL_3,
            CLIP3((i32)0, (i32)63,
                  (i32)dec->loopFilterLevel + dec->segmentLoopfilter[3]));
    }

    SetDecRegister(pDecCont->vp8Regs, HWIF_SEGMENT_E, dec->segmentationEnabled );
    SetDecRegister(pDecCont->vp8Regs, HWIF_SEGMENT_UPD_E, dec->segmentationMapUpdate );

    /* TODO: seems that ref dec does not disable filtering based on version,
     * check */
    /*SetDecRegister(pDecCont->vp8Regs, HWIF_FILTERING_DIS, dec->vpVersion >= 2);*/
    SetDecRegister(pDecCont->vp8Regs, HWIF_FILTERING_DIS,
        dec->loopFilterLevel == 0);

    /* Disable filtering if picture width larger than supported video 
     * decoder width or height larger than 8K */
    if(pDecCont->width > hwConfig.maxDecPicWidth ||
       pDecCont->height >= 8176 )
        SetDecRegister(pDecCont->vp8Regs, HWIF_FILTERING_DIS, 1 );

    /* full pell chroma mvs for VP8 version 3 */
    SetDecRegister(pDecCont->vp8Regs, HWIF_CH_MV_RES,
        dec->decMode == VP8HWD_VP7 || dec->vpVersion != 3);

    SetDecRegister(pDecCont->vp8Regs, HWIF_BILIN_MC_E,
        dec->decMode == VP8HWD_VP8 && (dec->vpVersion & 0x3));

    /* first bool decoder status */
    SetDecRegister(pDecCont->vp8Regs, HWIF_BOOLEAN_VALUE,
                   (pDecCont->bc.value >> 24) & (0xFFU));

    SetDecRegister(pDecCont->vp8Regs, HWIF_BOOLEAN_RANGE,
                   pDecCont->bc.range & (0xFFU));

    /* QP */
    if (pDecCont->decMode == VP8HWD_VP7)
    {
        /* LUT */
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_0, YDcQLookup[dec->qpYDc]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_1, YAcQLookup[dec->qpYAc]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_2, Y2DcQLookup[dec->qpY2Dc]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_3, Y2AcQLookup[dec->qpY2Ac]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_4, UvDcQLookup[dec->qpChDc]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_5, UvAcQLookup[dec->qpChAc]);
    }
    else
    {
        if (!dec->segmentationEnabled)
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_0, dec->qpYAc);
        else if (dec->segmentFeatureMode) /* absolute mode */
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_0, dec->segmentQp[0]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_1, dec->segmentQp[1]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_2, dec->segmentQp[2]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_3, dec->segmentQp[3]);
        }
        else /* delta mode */
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_0,
                CLIP3(0, 127, dec->qpYAc + dec->segmentQp[0]));
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_1,
                CLIP3(0, 127, dec->qpYAc + dec->segmentQp[1]));
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_2,
                CLIP3(0, 127, dec->qpYAc + dec->segmentQp[2]));
            SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_3,
                CLIP3(0, 127, dec->qpYAc + dec->segmentQp[3]));
        }
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_DELTA_0, dec->qpYDc);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_DELTA_1, dec->qpY2Dc);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_DELTA_2, dec->qpY2Ac);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_DELTA_3, dec->qpChDc);
        SetDecRegister(pDecCont->vp8Regs, HWIF_QUANT_DELTA_4, dec->qpChAc);

        if (dec->modeRefLfEnabled)
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_0,  dec->mbRefLfDelta[0]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_1,  dec->mbRefLfDelta[1]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_2,  dec->mbRefLfDelta[2]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_3,  dec->mbRefLfDelta[3]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_0,  dec->mbModeLfDelta[0]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_1,  dec->mbModeLfDelta[1]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_2,  dec->mbModeLfDelta[2]);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_3,  dec->mbModeLfDelta[3]);
        }
        else
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_0,  0);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_1,  0);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_2,  0);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_REF_ADJ_3,  0);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_0,  0);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_1,  0);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_2,  0);
            SetDecRegister(pDecCont->vp8Regs, HWIF_FILT_MB_ADJ_3,  0);
        }
    }

    /* scan order */
    if (pDecCont->decMode == VP8HWD_VP7)
    {
        i32 i;

        for(i = 1; i < 16; i++)
        {
            SetDecRegister(pDecCont->vp8Regs, ScanTblRegId[i],
                           pDecCont->decoder.vp7ScanOrder[i]);
        }
    }

    /* prediction filter taps */
    /* normal 6-tap filters */
    if ((dec->vpVersion & 0x3) == 0 || dec->decMode == VP8HWD_VP7)
    {
        i32 i, j;

        for(i = 0; i < 8; i++)
        {
            for(j = 0; j < 4; j++)
            {
                SetDecRegister(pDecCont->vp8Regs, TapRegId[i][j],
                    mcFilter[i][j+1]);
            }
            if (i == 2)
            {
                SetDecRegister(pDecCont->vp8Regs, HWIF_PRED_TAP_2_M1,
                    mcFilter[i][0]);
                SetDecRegister(pDecCont->vp8Regs, HWIF_PRED_TAP_2_4,
                    mcFilter[i][5]);
            }
            else if (i == 4)
            {
                SetDecRegister(pDecCont->vp8Regs, HWIF_PRED_TAP_4_M1,
                    mcFilter[i][0]);
                SetDecRegister(pDecCont->vp8Regs, HWIF_PRED_TAP_4_4,
                    mcFilter[i][5]);
            }
            else if (i == 6)
            {
                SetDecRegister(pDecCont->vp8Regs, HWIF_PRED_TAP_6_M1,
                    mcFilter[i][0]);
                SetDecRegister(pDecCont->vp8Regs, HWIF_PRED_TAP_6_4,
                    mcFilter[i][5]);
            }
        }
    }
    /* TODO: taps for bilinear case? */

    if (dec->decMode == VP8HWD_VP7)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_COMP0,
            pAsicBuff->dcPred[0]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_COMP1,
            pAsicBuff->dcPred[1]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_MATCH0,
            pAsicBuff->dcMatch[0]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_MATCH1,
            pAsicBuff->dcMatch[1]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_VP7_VERSION,
            dec->vpVersion != 0);
    }

    /* Setup reference picture buffer */
    if( pDecCont->refBufSupport && !pDecCont->intraOnly )
    {
        u32 cntLast = 0,
            cntGolden = 0,
            cntAlt = 0;
        u32 cntBest;
        u32 mul = 0;
        u32 allMbs = 0;
        u32 bufPicId = 0;
        u32 forceDisable = 0;
        u32 tmp;
        u32 cov;
        u32 intraMbs;
        u32 flags = (pDecCont->numCores > 1) ? REFBU_DONT_USE_STATS : 0;
        u32 threshold;

        if(!dec->keyFrame)
        {
            /* Calculate most probable reference frame */

#define CALCULATE_SHARE(range, prob) \
    (((range)*(prob-1))/254)

            allMbs = mul = dec->width * dec->height >> 8;        /* All mbs in frame */

            /* Deduct intra MBs */
            intraMbs = CALCULATE_SHARE(mul, dec->probIntra);
            mul = allMbs - intraMbs;

            cntLast = CALCULATE_SHARE(mul, dec->probRefLast);
            if( pDecCont->decMode == VP8HWD_VP8 )
            {
                mul -= cntLast;     /* What's left is mbs either Golden or Alt */
                cntGolden = CALCULATE_SHARE(mul, dec->probRefGolden);
                cntAlt = mul - cntGolden;
            }
            else
            {
                cntGolden = mul - cntLast; /* VP7 has no Alt ref frame */
                cntAlt = 0;
            }

#undef CALCULATE_SHARE

            /* Select best reference frame */

            if(cntLast > cntGolden)
            {
                tmp = (cntLast > cntAlt);
                bufPicId = tmp ? 0 : 5;
                cntBest  = tmp ? cntLast : cntAlt;
            }
            else
            {
                tmp = (cntGolden > cntAlt);
                bufPicId = tmp ? 4 : 5;
                cntBest  = tmp ? cntGolden : cntAlt;
            }

            /* Check against refbuf-calculated threshold value; if  it
             * seems we'll miss our target, then don't bother enabling
             * the feature at all... */
            threshold = RefbuGetHitThreshold(&pDecCont->refBufferCtrl);
            threshold *= (dec->height/16);
            threshold /= 4;
            if(cntBest < threshold)
                forceDisable = 1;

            /* Next frame has enough reference picture hits, now also take
             * actual hits and intra blocks into calculations... */
            if(!forceDisable)
            {
                cov = RefbuVpxGetPrevFrameStats(&pDecCont->refBufferCtrl);

                /* If we get prediction for coverage, we can disable checkpoint
                 * and do calculations here */
                if( cov > 0 )
                {
                    /* Calculate percentage of hits from last frame, multiply
                     * predicted reference frame referrals by it and compare.
                     * Note: if refbuffer was off for previous frame, we might
                     *    not get an accurate estimation... */

                    tmp = dec->refbuPredHits;
                    if(tmp)     cov = (256*cov)/tmp;
                    else        cov = 0;
                    cov = (cov*cntBest)>>8;

                    if( cov < threshold)
                        forceDisable = 1;
                    else
                        flags |= REFBU_DISABLE_CHECKPOINT;
                }
            }
            dec->refbuPredHits = cntBest;
        }
        else
        {
            dec->refbuPredHits = 0;
        }

        RefbuSetup( &pDecCont->refBufferCtrl, pDecCont->vp8Regs,
                    REFBU_FRAME,
                    dec->keyFrame || forceDisable,
                    HANTRO_FALSE,
                    bufPicId, 0,
                    flags );
    }

    if( pDecCont->tiledModeSupport && !pDecCont->intraOnly)
    {
        pDecCont->tiledReferenceEnable =
            DecSetupTiledReference( pDecCont->vp8Regs,
                pDecCont->tiledModeSupport,
                DEC_DPB_FRAME,
                0 );
    }
    else
    {
        pDecCont->tiledReferenceEnable = 0;
    }

    SetDecRegister(pDecCont->vp8Regs, HWIF_STARTMB_X, 0);
    SetDecRegister(pDecCont->vp8Regs, HWIF_STARTMB_Y, 0);
    SetDecRegister(pDecCont->vp8Regs, HWIF_ERROR_CONC_MODE, 0);

    if (!pDecCont->conceal)
    {
        if (dec->keyFrame || !pDecCont->hwEcSupport)
            SetDecRegister(pDecCont->vp8Regs, HWIF_WRITE_MVS_E, 0);
        else
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_WRITE_MVS_E, 1);
            SetDecRegister(pDecCont->vp8Regs, HWIF_DIR_MV_BASE,
                pAsicBuff->mvs[pAsicBuff->mvsCurr].busAddress);
        }
    }
    /* start HW in error concealment mode */
    else
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_WRITE_MVS_E, 0);
        SetDecRegister(pDecCont->vp8Regs, HWIF_DIR_MV_BASE,
            pAsicBuff->mvs[pAsicBuff->mvsCurr].busAddress);
        SetDecRegister(pDecCont->vp8Regs, HWIF_STARTMB_X, 
            pDecCont->concealStartMbX);
        SetDecRegister(pDecCont->vp8Regs, HWIF_STARTMB_Y,
            pDecCont->concealStartMbY);
        SetDecRegister(pDecCont->vp8Regs, HWIF_ERROR_CONC_MODE, 1);
    }

}

/*------------------------------------------------------------------------------
    Function name : VP8HwdAsicStrmPosUpdate
    Description   : Set stream base and length related registers

    Return type   :
    Argument      : container
------------------------------------------------------------------------------*/
void VP8HwdAsicStrmPosUpdate(VP8DecContainer_t * pDecCont, u32 strmBusAddress)
{
    u32 i, tmp;
    u32 hwBitPos;
    u32 tmpAddr;
    u32 tmp2;
    u32 byteOffset;
    u32 extraBytesPacked = 0;
    vp8Decoder_t *dec = &pDecCont->decoder;

    DEBUG_PRINT(("VP8HwdAsicStrmPosUpdate:\n"));

    /* TODO: miksi bitin tarkkuudella (count) kun kuitenki luetaan tavu
     * kerrallaan? Vai lukeeko HW eri lailla? */
    /* start of control partition */
    tmp = (pDecCont->bc.pos) * 8 + (8 - pDecCont->bc.count);

    if (dec->frameTagSize == 4) tmp+=8;

    if(pDecCont->decMode == VP8HWD_VP8 &&
       pDecCont->decoder.keyFrame)
        extraBytesPacked += 7;

    tmp += extraBytesPacked*8;

    tmpAddr = strmBusAddress + tmp/8;
    hwBitPos = (tmpAddr & DEC_8190_ALIGN_MASK) * 8;
    tmpAddr &= (~DEC_8190_ALIGN_MASK);  /* align the base */

    hwBitPos += tmp & 0x7;

    /* control partition */
    SetDecRegister(pDecCont->vp8Regs, HWIF_VP6HWPART1_BASE, tmpAddr);
    SetDecRegister(pDecCont->vp8Regs, HWIF_STRM1_START_BIT, hwBitPos);

    /* total stream length */
    /*tmp = pDecCont->bc.streamEndPos - (tmpAddr - strmBusAddress);*/

    /* calculate dct partition length here instead */
    tmp = pDecCont->bc.streamEndPos + dec->frameTagSize - dec->dctPartitionOffsets[0];
    tmp += (dec->nbrDctPartitions-1)*3;
    tmp2 = strmBusAddress + extraBytesPacked + dec->dctPartitionOffsets[0];
    tmp += (tmp2 & 0x7);

    SetDecRegister(pDecCont->vp8Regs, HWIF_STREAM_LEN, tmp);
    /* Default length register is 24 bits. If running webp, write extra 
     * length MSBs to extension register. */
    if(pDecCont->intraOnly)
        SetDecRegister(pDecCont->vp8Regs, HWIF_STREAM_LEN_EXT, tmp >> 24);

    /* control partition length */
    tmp = dec->offsetToDctParts;
    /* if total length smaller than indicated control length -> cut */
    if (tmp > pDecCont->bc.streamEndPos)
        tmp = pDecCont->bc.streamEndPos;
    tmp += dec->frameTagSize + extraBytesPacked; 
    /* subtract what was read by SW */
    tmp -= (tmpAddr - strmBusAddress);
    /* give extra byte of data to negotiate "tight" buffers */
    if (!pDecCont->hwEcSupport)
        tmp ++;

    SetDecRegister(pDecCont->vp8Regs, HWIF_STREAM1_LEN, tmp);

    /* number of dct partitions */
    SetDecRegister(pDecCont->vp8Regs, HWIF_COEFFS_PART_AM, 
        dec->nbrDctPartitions-1);

    /* base addresses and bit offsets of dct partitions */
    for (i = 0; i < dec->nbrDctPartitions; i++)
    {
        tmpAddr = strmBusAddress + extraBytesPacked + dec->dctPartitionOffsets[i];
        byteOffset = tmpAddr & 0x7;
        SetDecRegister(pDecCont->vp8Regs, DctBaseId[i], tmpAddr & 0xFFFFFFF8);
        SetDecRegister(pDecCont->vp8Regs, DctStartBit[i], byteOffset * 8);
    }


}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicRefreshRegs
    Description     :
    Return type     : void
    Argument        : decContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicRefreshRegs(VP8DecContainer_t * pDecCont)
{
    i32 i;
    u32 offset = 0x0;

    u32 *decRegs = pDecCont->vp8Regs;

    for(i = DEC_X170_REGISTERS; i > 0; --i)
    {
        *decRegs++ = DWLReadReg(pDecCont->dwl, pDecCont->coreID, offset);
        offset += 4;
    }
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicFlushRegs
    Description     :
    Return type     : void
    Argument        : decContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicFlushRegs(VP8DecContainer_t * pDecCont)
{
    i32 i;
    u32 offset = 0x04;
    u32 *decRegs = pDecCont->vp8Regs + 1;

#ifdef TRACE_START_MARKER
    /* write ID register to trigger logic analyzer */
    DWLWriteReg(pDecCont->dwl, 0x00, ~0);
#endif

    decRegs[0] &= ~decRegs[0];

    for(i = DEC_X170_REGISTERS; i > 1; --i)
    {
        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, offset, *decRegs);
        decRegs++;
        offset += 4;
    }

}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicRun
    Description     : 
    Return type     : u32 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
u32 VP8HwdAsicRun(VP8DecContainer_t * pDecCont)
{
    u32 asic_status = 0;
    i32 ret;

    if (!pDecCont->asicRunning || pDecCont->streamConsumedCallback != NULL)
    {
        pDecCont->asicRunning = 1;
        if(pDecCont->pp.ppInstance != NULL &&
           pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING)
        {
            DecPpInterface *decPpIf = &pDecCont->pp.decPpIf;
            DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;

            TRACE_PP_CTRL("VP8HwdAsicRun: PP Run\n");

            decPpIf->inwidth  = MB_MULTIPLE(pDecCont->width);
            decPpIf->inheight = MB_MULTIPLE(pDecCont->height);
            decPpIf->croppedW = (decPpIf->inwidth + 7) & ~7;
            decPpIf->croppedH = (decPpIf->inheight + 7) & ~7;

            decPpIf->lumaStride = pAsicBuff->lumaStride ? 
                pAsicBuff->lumaStride : pAsicBuff->width;
            decPpIf->chromaStride = pAsicBuff->chromaStride ? 
                pAsicBuff->chromaStride : pAsicBuff->width;
                
            /* forward tiled mode */
            decPpIf->tiledInputMode = pDecCont->tiledReferenceEnable;
            decPpIf->progressiveSequence = 1;

            decPpIf->picStruct = DECPP_PIC_FRAME_OR_TOP_FIELD;
            decPpIf->littleEndian =
                GetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUT_ENDIAN);
            decPpIf->wordSwap =
                GetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUTSWAP32_E);

            if(decPpIf->usePipeline)
            {
                decPpIf->inputBusLuma = decPpIf->inputBusChroma = 0;
            }
            else /* parallel processing */
            {
                decPpIf->inputBusLuma = GetPrevRef(pDecCont)->busAddress;
                decPpIf->inputBusChroma =
                    pAsicBuff->picturesC[
                        VP8HwdBufferQueueGetPrevRef(pDecCont->bq)].busAddress;

                if(!(pAsicBuff->stridesUsed || pAsicBuff->customBuffers))
                {
                    decPpIf->inputBusChroma = decPpIf->inputBusLuma +
                        decPpIf->inwidth * decPpIf->inheight;
                }

            }

            pDecCont->pp.PPDecStart(pDecCont->pp.ppInstance, decPpIf);
        }
        VP8HwdAsicFlushRegs(pDecCont);

        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_E, 1);
        /* If in multicore-mode, prepare the callback. */
        if (pDecCont->streamConsumedCallback)
        {
            VP8MCSetHwRdyCallback(pDecCont);

            /* Manage the reference frame bookkeeping. */
            VP8HwdUpdateRefs(pDecCont, 0);
            /* Make sure the reference statuses (128-bits) are zeroed. */
            /* Use status writing only for actual multicore operation,
               multithreaded decoding can be used with one core also. */
            if(pDecCont->numCores > 1)
            {
                DWLmemset(VP8HwdRefStatusAddress(pDecCont), 0, 4 * sizeof(u32));
            }

        }
        DWLEnableHW(pDecCont->dwl, pDecCont->coreID, 4 * 1,
                    pDecCont->vp8Regs[1]);
    }
    else
    {
        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, 4 * 13,
                    pDecCont->vp8Regs[13]);
        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, 4 * 14,
                    pDecCont->vp8Regs[14]);
        DWLWriteReg(pDecCont->dwl, pDecCont->coreID, 4 * 15,
                    pDecCont->vp8Regs[15]);
        DWLEnableHW(pDecCont->dwl, pDecCont->coreID, 4 * 1,
                    pDecCont->vp8Regs[1]);
    }

    /* If the decoder is run by callback mechanism, no need to wait for HW. */
    if (pDecCont->streamConsumedCallback)
        return VP8HWDEC_ASYNC_MODE;
    else
        ret = DWLWaitHwReady(pDecCont->dwl, pDecCont->coreID,
                             (u32) DEC_X170_TIMEOUT_LENGTH);

    if(ret != DWL_HW_WAIT_OK)
    {
        ERROR_PRINT("DWLWaitHwReady");
        DEBUG_PRINT(("DWLWaitHwReady returned: %d\n", ret));

        /* reset HW */
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_IRQ_STAT, 0);
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_IRQ, 0);
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_E, 0);

        DWLDisableHW(pDecCont->dwl, pDecCont->coreID, 4 * 1,
                     pDecCont->vp8Regs[1]);

        /* Wait for PP to end also */
        if(pDecCont->pp.ppInstance != NULL &&
           pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING)
        {
            pDecCont->pp.decPpIf.ppStatus = DECPP_PIC_READY;

            TRACE_PP_CTRL("VP8HwdAsicRun: PP Wait for end\n");

            pDecCont->pp.PPDecWaitEnd(pDecCont->pp.ppInstance);

            TRACE_PP_CTRL("VP8HwdAsicRun: PP Finished\n");
        }

        pDecCont->asicRunning = 0;
        DWLReleaseHw(pDecCont->dwl, pDecCont->coreID);

        return (ret == DWL_HW_WAIT_ERROR) ?
            VP8HWDEC_SYSTEM_ERROR : VP8HWDEC_SYSTEM_TIMEOUT;
    }

    VP8HwdAsicRefreshRegs(pDecCont);

    /* React to the HW return value */

    asic_status = GetDecRegister(pDecCont->vp8Regs, HWIF_DEC_IRQ_STAT);

    SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_IRQ_STAT, 0);
    SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_IRQ, 0); /* just in case */

    if (pDecCont->decoder.decMode == VP8HWD_VP7)
    {
        pDecCont->asicBuff->dcPred[0] =
            GetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_COMP0);
        pDecCont->asicBuff->dcPred[1] =
            GetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_COMP1);
        pDecCont->asicBuff->dcMatch[0] =
            GetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_MATCH0);
        pDecCont->asicBuff->dcMatch[1] =
            GetDecRegister(pDecCont->vp8Regs, HWIF_INIT_DC_MATCH1);
    }
    if (!(asic_status & DEC_8190_IRQ_SLICE)) /* not slice interrupt */
    {
        /* HW done, release it! */
        DWLDisableHW(pDecCont->dwl, pDecCont->coreID, 4 * 1,
                     pDecCont->vp8Regs[1]);
        pDecCont->asicRunning = 0;

        /* Wait for PP to end also, this is pipeline case */
        if(pDecCont->pp.ppInstance != NULL &&
           pDecCont->pp.decPpIf.ppStatus == DECPP_RUNNING)
        {
            pDecCont->pp.decPpIf.ppStatus = DECPP_PIC_READY;

            TRACE_PP_CTRL("VP8HwdAsicRun: PP Wait for end\n");

            pDecCont->pp.PPDecWaitEnd(pDecCont->pp.ppInstance);

            TRACE_PP_CTRL("VP8HwdAsicRun: PP Finished\n");
        }

        DWLReleaseHw(pDecCont->dwl, pDecCont->coreID);

        if( pDecCont->refBufSupport &&
            (asic_status & DEC_8190_IRQ_RDY) &&
            pDecCont->asicRunning == 0 )
        {
            RefbuMvStatistics( &pDecCont->refBufferCtrl, 
                                pDecCont->vp8Regs,
                                NULL, HANTRO_FALSE,
                                pDecCont->decoder.keyFrame );
        }
    }

    return asic_status;
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdSegmentMapUpdate
    Description     : Set register base for HW, issue new buffer for write
                        reset dor keyframe.
    Return type     : void
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdSegmentMapUpdate(VP8DecContainer_t * pDecCont)
{
    vp8Decoder_t *dec = &pDecCont->decoder;

    /* For map update, provide new segmentation map buffer. There are as many
     * buffers as cores. */
    if(dec->segmentationMapUpdate && pDecCont->streamConsumedCallback)
    {
        pDecCont->segmID = (pDecCont->segmID+1) % pDecCont->numCores;
    }
    SetDecRegister(pDecCont->vp8Regs, HWIF_SEGMENT_BASE,
        pDecCont->asicBuff->segmentMap[pDecCont->segmID].busAddress);

    if(pDecCont->decoder.keyFrame)
    {
        vp8hwdResetSegMap(&pDecCont->decoder, pDecCont->asicBuff, pDecCont->segmID);
    }
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicProbUpdate
    Description     :
    Return type     : void
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicProbUpdate(VP8DecContainer_t * pDecCont)
{
    u32 *dst;
    u32 i, j, k;
    u32 tmp;
    u32 *asicProbBase = pDecCont->asicBuff->probTbl[pDecCont->coreID].virtualAddress;

    SetDecRegister(pDecCont->vp8Regs, HWIF_VP6HWPROBTBL_BASE,
                   pDecCont->asicBuff->probTbl[pDecCont->coreID].busAddress);

    /* first probs */
    dst = asicProbBase;

    tmp = (pDecCont->decoder.probMbSkipFalse << 24) |
          (pDecCont->decoder.probIntra       << 16) |
          (pDecCont->decoder.probRefLast     <<  8) |
          (pDecCont->decoder.probRefGolden   <<  0);
    *dst++ = tmp;
    tmp = (pDecCont->decoder.probSegment[0]  << 24) |
          (pDecCont->decoder.probSegment[1]  << 16) |
          (pDecCont->decoder.probSegment[2]  <<  8) |
          (0 /*unused*/                      <<  0);
    *dst++ = tmp;

    tmp = (pDecCont->decoder.entropy.probLuma16x16PredMode[0] << 24) |
          (pDecCont->decoder.entropy.probLuma16x16PredMode[1] << 16) |
          (pDecCont->decoder.entropy.probLuma16x16PredMode[2] <<  8) |
          (pDecCont->decoder.entropy.probLuma16x16PredMode[3] <<  0);
    *dst++ = tmp;
    tmp = (pDecCont->decoder.entropy.probChromaPredMode[0]    << 24) |
          (pDecCont->decoder.entropy.probChromaPredMode[1]    << 16) |
          (pDecCont->decoder.entropy.probChromaPredMode[2]    <<  8) |
          (0 /*unused*/                                       <<  0);
    *dst++ = tmp;

    /* mv probs */
    tmp = (pDecCont->decoder.entropy.probMvContext[0][0] << 24) | /* is short */
          (pDecCont->decoder.entropy.probMvContext[1][0] << 16) |
          (pDecCont->decoder.entropy.probMvContext[0][1] <<  8) | /* sign */
          (pDecCont->decoder.entropy.probMvContext[1][1] <<  0);
    *dst++ = tmp;
    tmp = (pDecCont->decoder.entropy.probMvContext[0][8+9] << 24) |
          (pDecCont->decoder.entropy.probMvContext[0][9+9] << 16) |
          (pDecCont->decoder.entropy.probMvContext[1][8+9] <<  8) |
          (pDecCont->decoder.entropy.probMvContext[1][9+9] <<  0);
    *dst++ = tmp;
    for( i = 0 ; i < 2 ; ++i )
    {
        for( j = 0 ; j < 8 ; j+=4 )
        {
            tmp = (pDecCont->decoder.entropy.probMvContext[i][j+9+0] << 24) |
                  (pDecCont->decoder.entropy.probMvContext[i][j+9+1] << 16) |
                  (pDecCont->decoder.entropy.probMvContext[i][j+9+2] <<  8) |
                  (pDecCont->decoder.entropy.probMvContext[i][j+9+3] <<  0);
            *dst++ = tmp;
        }
    }
    for( i = 0 ; i < 2 ; ++i )
    {
        tmp = (pDecCont->decoder.entropy.probMvContext[i][0+2] << 24) |
              (pDecCont->decoder.entropy.probMvContext[i][1+2] << 16) |
              (pDecCont->decoder.entropy.probMvContext[i][2+2] <<  8) |
              (pDecCont->decoder.entropy.probMvContext[i][3+2] <<  0);
        *dst++ = tmp;
        tmp = (pDecCont->decoder.entropy.probMvContext[i][4+2] << 24) |
              (pDecCont->decoder.entropy.probMvContext[i][5+2] << 16) |
              (pDecCont->decoder.entropy.probMvContext[i][6+2] <<  8) |
              (0 /* unused */                                  <<  0);
        *dst++ = tmp;
    }

    /* coeff probs (header part) */
    dst = asicProbBase + 8*7/4; 
    for( i = 0 ; i < 4 ; ++i )
    {
        for( j = 0 ; j < 8 ; ++j )
        {
            for( k = 0 ; k < 3 ; ++k )
            {
                tmp = (pDecCont->decoder.entropy.probCoeffs[i][j][k][0] << 24) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][1] << 16) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][2] <<  8) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][3] <<  0);
                *dst++ = tmp;
            }
        }
    }

    /* coeff probs (footer part) */
    dst = asicProbBase + 8*55/4; 
    for( i = 0 ; i < 4 ; ++i )
    {
        for( j = 0 ; j < 8 ; ++j )
        {
            for( k = 0 ; k < 3 ; ++k )
            {
                tmp = (pDecCont->decoder.entropy.probCoeffs[i][j][k][4] << 24) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][5] << 16) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][6] <<  8) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][7] <<  0);
                *dst++ = tmp;
                tmp = (pDecCont->decoder.entropy.probCoeffs[i][j][k][8] << 24) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][9] << 16) |
                      (pDecCont->decoder.entropy.probCoeffs[i][j][k][10] << 8) |
                      (0 /* unused */                                   <<  0);
                *dst++ = tmp;
            }
        }
    }
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdUpdateRefs
    Description     : 
    Return type     : void 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdUpdateRefs(VP8DecContainer_t * pDecCont, u32 corrupted)
{

    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;

    i32 prevP = 0, prevA = 0, prevG = 0;

    if(pDecCont->streamConsumedCallback == NULL)
    {
        /* Store current ref indices but remove only after new refs have been
         * protected. */
        prevP = VP8HwdBufferQueueGetPrevRef(pDecCont->bq);
        prevA = VP8HwdBufferQueueGetAltRef(pDecCont->bq);
        prevG = VP8HwdBufferQueueGetGoldenRef(pDecCont->bq);
    }
    if (pDecCont->decoder.copyBufferToAlternate == 1)
    {
        VP8HwdBufferQueueUpdateRef(pDecCont->bq, BQUEUE_FLAG_ALT,
                                   VP8HwdBufferQueueGetPrevRef(pDecCont->bq));
    }
    else if (pDecCont->decoder.copyBufferToAlternate == 2)
    {
        VP8HwdBufferQueueUpdateRef(pDecCont->bq, BQUEUE_FLAG_ALT,
                                   VP8HwdBufferQueueGetGoldenRef(pDecCont->bq));
    }

    if (pDecCont->decoder.copyBufferToGolden == 1)
    {
        VP8HwdBufferQueueUpdateRef(pDecCont->bq, BQUEUE_FLAG_GOLDEN,
                                   VP8HwdBufferQueueGetPrevRef(pDecCont->bq));
    }
    else if (pDecCont->decoder.copyBufferToGolden == 2)
    {
        VP8HwdBufferQueueUpdateRef(pDecCont->bq, BQUEUE_FLAG_GOLDEN,
                                   VP8HwdBufferQueueGetAltRef(pDecCont->bq));
    }


    if (!corrupted)
    {
        u32 update_flags = 0;
        if(pDecCont->decoder.refreshGolden)
        {
            update_flags |= BQUEUE_FLAG_GOLDEN;
        }

        if(pDecCont->decoder.refreshAlternate)
        {
            update_flags |= BQUEUE_FLAG_ALT;
        }

        if (pDecCont->decoder.refreshLast)
        {
            update_flags |= BQUEUE_FLAG_PREV;
        }
        VP8HwdBufferQueueUpdateRef(pDecCont->bq, update_flags,
                                   pAsicBuff->outBufferI);

        pAsicBuff->mvsRef = pAsicBuff->mvsCurr;
        pAsicBuff->mvsCurr ^= 0x1;
    }
    /* AddRef the pictures for this core run. */
    VP8HwdBufferQueueAddRef(pDecCont->bq,
                            VP8HwdBufferQueueGetPrevRef(pDecCont->bq));
    VP8HwdBufferQueueAddRef(pDecCont->bq,
                            VP8HwdBufferQueueGetAltRef(pDecCont->bq));
    VP8HwdBufferQueueAddRef(pDecCont->bq,
                            VP8HwdBufferQueueGetGoldenRef(pDecCont->bq));

    if(pDecCont->streamConsumedCallback == NULL)
    {
        VP8HwdBufferQueueRemoveRef(pDecCont->bq, prevP);
        VP8HwdBufferQueueRemoveRef(pDecCont->bq, prevA);
        VP8HwdBufferQueueRemoveRef(pDecCont->bq, prevG);
    }
}

void vp8hwdUpdateOutBase(VP8DecContainer_t *pDecCont)
{

    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;

    ASSERT(pDecCont->intraOnly);

    if (pDecCont->userMem)
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUT_BASE,
                       pAsicBuff->userMem.picBufferBusAddrY[0]);
        SetDecRegister(pDecCont->vp8Regs, HWIF_REFER0_BASE,
                       pAsicBuff->userMem.picBufferBusAddrC[0]);
    }
    else/* if (startOfPic)*/
    {
        SetDecRegister(pDecCont->vp8Regs, HWIF_DEC_OUT_BASE,
                       pAsicBuff->outBuffer->busAddress);

        if(!(pAsicBuff->stridesUsed || pAsicBuff->customBuffers))
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_REFER0_BASE,
                pAsicBuff->outBuffer->busAddress +
                pAsicBuff->width *
                (pDecCont->sliceHeight ? (pDecCont->sliceHeight + 1)*16 :
                 pAsicBuff->height));
        }
        else
        {
            SetDecRegister(pDecCont->vp8Regs, HWIF_REFER0_BASE,
                pAsicBuff->outBuffer->busAddress +
                    pAsicBuff->chromaBufOffset);
        }
    }
}

/*------------------------------------------------------------------------------
    Function name   : VP8HwdAsicContPicture
    Description     : 
    Return type     : void 
    Argument        : VP8DecContainer_t * pDecCont
------------------------------------------------------------------------------*/
void VP8HwdAsicContPicture(VP8DecContainer_t * pDecCont)
{

    u32 sliceHeight;

    /* update output picture buffer if not pipeline */
    if(pDecCont->pp.ppInstance == NULL ||
      !pDecCont->pp.decPpIf.usePipeline)
        vp8hwdUpdateOutBase(pDecCont);

    /* slice height */
    if (pDecCont->totDecodedRows + pDecCont->sliceHeight >
        pDecCont->asicBuff->height/16)
        sliceHeight = pDecCont->asicBuff->height/16 - pDecCont->totDecodedRows;
    else
        sliceHeight = pDecCont->sliceHeight;

    SetDecRegister(pDecCont->vp8Regs, HWIF_JPEG_SLICE_H, sliceHeight);

}

DWLLinearMem_t* GetOutput(VP8DecContainer_t *pDecCont)
{
    return pDecCont->asicBuff->pictures + pDecCont->asicBuff->outBufferI;
}

static DWLLinearMem_t* GetPrevRef(VP8DecContainer_t *pDecCont)
{
    return pDecCont->asicBuff->pictures +
        VP8HwdBufferQueueGetPrevRef(pDecCont->bq);
}

static DWLLinearMem_t* GetGoldenRef(VP8DecContainer_t *pDecCont)
{
    return pDecCont->asicBuff->pictures +
        VP8HwdBufferQueueGetGoldenRef(pDecCont->bq);
}

static DWLLinearMem_t* GetAltRef(VP8DecContainer_t *pDecCont)
{
    return pDecCont->asicBuff->pictures +
        VP8HwdBufferQueueGetAltRef(pDecCont->bq);
}

u32 VP8HwdBufferingMode(VP8DecContainer_t * pDecCont)
{
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;

    u32 mode = VP8HWD_BUFFER_BASIC;

    if (pAsicBuff->customBuffers)
    {
        mode = VP8HWD_BUFFER_CUSTOM;
    }
    else if (pAsicBuff->stridesUsed)
    {
        /* In the current implementation stride cannot be
         * enabled without separate buffers. */
        mode = VP8HWD_BUFFER_STRIDE;
        ASSERT(0 && "Error: Stride without custom buffers not implemented.");
    }
    return mode;
}

u32* VP8HwdRefStatusAddress(VP8DecContainer_t * pDecCont)
{
    DecAsicBuffers_t *pAsicBuff = pDecCont->asicBuff;
    u32 chromaWidth;
    u32 *p;

    switch (VP8HwdBufferingMode(pDecCont))
    {
        case VP8HWD_BUFFER_BASIC:
            p = GetOutput(pDecCont)->virtualAddress +
                pDecCont->asicBuff->width *
                pDecCont->asicBuff->height * 3 / 8;
            break;
        case VP8HWD_BUFFER_CUSTOM:
            chromaWidth = (pAsicBuff->chromaStride ?
                pAsicBuff->chromaStride : pAsicBuff->width);
            p = pAsicBuff->picturesC[pAsicBuff->outBufferI].virtualAddress +
                    chromaWidth * pDecCont->asicBuff->height / 8;
            break;
        case VP8HWD_BUFFER_STRIDE:
        default:
            ASSERT(0);
            p = NULL;
            break;
    }
    return p;
}
