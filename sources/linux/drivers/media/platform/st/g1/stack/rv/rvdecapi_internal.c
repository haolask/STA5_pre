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
--  Description : API internal functions
--
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Table of contents

    1.  Include headers
    5.  Fuctions
        5.1 ClearDataStructures()

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1.  Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "rv_container.h"
#include "rv_cfg.h"
#include "rvdecapi.h"
#include "rv_utils.h"
#include "rvdecapi_internal.h"
#include "rv_debug.h"
#include "rv_vlc.h"
#include "regdrv.h"

/*------------------------------------------------------------------------------

    5.1 Function name:  rvAPI_InitDataStructures()

        Purpose:        Initialize Data Structures in DecContainer.

        Input:          DecContainer *pDecCont

        Output:         u32

------------------------------------------------------------------------------*/
void rvAPI_InitDataStructures(DecContainer * pDecCont)
{

    pDecCont->StrmStorage.work0 = pDecCont->StrmStorage.work1 =
        INVALID_ANCHOR_PICTURE;
}

/*------------------------------------------------------------------------------

    x.x Function name:  rvAllocateBuffers

        Purpose:        Allocate memory

        Input:          DecContainer *pDecCont

        Output:         RVDEC_MEMFAIL/RVDEC_OK

------------------------------------------------------------------------------*/
RvDecRet rvAllocateBuffers(DecContainer * pDecCont)
{

    u32 i;
    i32 ret = 0;
    u32 sizeTmp = 0;
    u32 buffers = 0;

    ASSERT(pDecCont->StrmStorage.maxMbsPerFrame != 0);

    /* Reference images */
    if(!pDecCont->ApiStorage.externalBuffers)
    {
        /*sizeTmp = 384 * pDecCont->FrameDesc.totalMbInFrame;*/
        sizeTmp = 384 * pDecCont->StrmStorage.maxMbsPerFrame;

        /* Calculate minimum amount of buffers */
        buffers = 3; 

        if( pDecCont->ppInstance ) /* Combined mode used */
        {
            pDecCont->StrmStorage.numPpBuffers = pDecCont->StrmStorage.maxNumBuffers;
            pDecCont->StrmStorage.numBuffers = buffers; /* Use bare minimum in decoder */
            buffers =  2;
            if( pDecCont->StrmStorage.numPpBuffers < buffers )
                pDecCont->StrmStorage.numPpBuffers = buffers;
        }
        else /* Dec only or separate PP */
        {
            pDecCont->StrmStorage.numBuffers = pDecCont->StrmStorage.maxNumBuffers;
            pDecCont->StrmStorage.numPpBuffers = 0;
            if( pDecCont->StrmStorage.numBuffers < buffers )
                pDecCont->StrmStorage.numBuffers = buffers;
        }

        ret = BqueueInit(&pDecCont->StrmStorage.bq, 
                         pDecCont->StrmStorage.numBuffers );
        if( ret != HANTRO_OK )
            return RVDEC_MEMFAIL;

        ret = BqueueInit(&pDecCont->StrmStorage.bqPp, 
                         pDecCont->StrmStorage.numPpBuffers );
        if( ret != HANTRO_OK )
            return RVDEC_MEMFAIL;

        for(i = 0; i < pDecCont->StrmStorage.numBuffers; i++)
        {
            ret |= DWLMallocRefFrmNamed(pDecCont->dwl, sizeTmp,
                                   &pDecCont->StrmStorage.pPicBuf[i].data,
                                   "Allocate Reference Picture buffer");

            RVDEC_DEBUG(("PicBuffer[%d]: %x, %x\n",
                            i,
                            (u32) pDecCont->StrmStorage.pPicBuf[i].data.
                            virtualAddress,
                            pDecCont->StrmStorage.pPicBuf[i].data.busAddress));

            if(pDecCont->StrmStorage.pPicBuf[i].data.busAddress == 0)
            {
                return (RVDEC_MEMFAIL);
            }
        }
        /* initialize first picture buffer (workOut is 1 for the first picture)
         * grey, may be used as reference in certain error cases */
        (void) DWLmemset(pDecCont->StrmStorage.pPicBuf[1].data.virtualAddress,
                         128, 384 * pDecCont->FrameDesc.totalMbInFrame);
    }

    /* shared memory for vlc tables.  Number of table entries is
     * numIntraQpRegions * numTableEntriesPerIntraRegion +
     * numInterQpRegions * numTableEntriesPerInterRegion
     *
     * Each table needs 3*16 entries for "control" and number of different
     * codes entries for symbols */
#if 0
    sizeTmp = 5 * (2 * (1296 + 4*16) + 3*864 + 2*108 + 864 + 2*108 + 32) +
              7 * (1296 + 4*16 + 864 + 2*108 + 864 + 2*108 + 32);
    sizeTmp *= 2; /* each symbol stored as u16 */
    sizeTmp += 3*16*4*(5*(2+8+3+2+1+2+1)+7*(1+4+1+2+1+2+1));
    ret = DWLMallocLinear(pDecCont->dwl, sizeTmp, 
        &pDecCont->StrmStorage.vlcTables);
#endif

    ret |= DWLMallocLinearNamed(pDecCont->dwl,
        ((pDecCont->FrameDesc.totalMbInFrame+1)&~0x1) * 4 * sizeof(u32), 
        &pDecCont->StrmStorage.directMvs,"shared memory for vlc tables");

    if(ret)
        return (RVDEC_MEMFAIL);
    else
        return RVDEC_OK;

}

/*------------------------------------------------------------------------------

    x.x Function name:  rvDecCheckSupport

        Purpose:        Check picture sizes etc

        Input:          DecContainer *pDecCont

        Output:         RVDEC_STRMERROR/RVDEC_OK

------------------------------------------------------------------------------*/

RvDecRet rvDecCheckSupport(DecContainer * pDecCont)
{
    DWLHwConfig_t hwConfig;

    DWLReadAsicConfig(&hwConfig);

    if((pDecCont->FrameDesc.frameHeight > (hwConfig.maxDecPicWidth >> 4)) ||
       (pDecCont->FrameDesc.frameHeight < (RV_MIN_HEIGHT >> 4)))
    {

        RVDEC_DEBUG(("RvDecCheckSupport# Height not supported %d \n",
                        pDecCont->FrameDesc.frameHeight));
        return RVDEC_STREAM_NOT_SUPPORTED;
    }

    if((pDecCont->FrameDesc.frameWidth > (hwConfig.maxDecPicWidth >> 4)) ||
       (pDecCont->FrameDesc.frameWidth < (RV_MIN_WIDTH >> 4)))
    {

        RVDEC_DEBUG(("RvDecCheckSupport# Width not supported %d \n",
                        pDecCont->FrameDesc.frameWidth));
        return RVDEC_STREAM_NOT_SUPPORTED;
    }

    if(pDecCont->FrameDesc.totalMbInFrame > RVAPI_DEC_MBS)
    {
        RVDEC_DEBUG(("Maximum number of macroblocks exceeded %d \n",
                        pDecCont->FrameDesc.totalMbInFrame));
        return RVDEC_STREAM_NOT_SUPPORTED;
    }

    return RVDEC_OK;

}

/*------------------------------------------------------------------------------

    x.x Function name:  rvDecPreparePicReturn

        Purpose:        Prepare return values for PIC returns
                        For use after HW start

        Input:          DecContainer *pDecCont
                        RvDecOutput *outData    currently used out

        Output:         void

------------------------------------------------------------------------------*/

void rvDecPreparePicReturn(DecContainer * pDecCont)
{

    ASSERT(pDecCont != NULL);

    pDecCont->StrmStorage.pPicBuf[pDecCont->StrmStorage.workOut].ff[0] = 0;
    pDecCont->StrmStorage.pPicBuf[pDecCont->StrmStorage.workOut].ff[1] = 0;

    return;

}

/*------------------------------------------------------------------------------

    x.x Function name:  rvDecBufferPicture

        Purpose:        Handles picture buffering

        Input:          
                        

        Output:         

------------------------------------------------------------------------------*/
void rvDecBufferPicture(DecContainer * pDecCont, u32 picId, u32 bufferB,
                           u32 isInter, RvDecRet returnValue, u32 nbrErrMbs)
{
    i32 i, j;
    DecHdrs * pHdrs = &pDecCont->Hdrs;
    u32 frameWidth;
    u32 frameHeight;
    u32 picType;

    ASSERT(pDecCont);

    if(bufferB == 0)    /* Buffer I or P picture */
    {
        i = pDecCont->StrmStorage.outIndex + pDecCont->StrmStorage.outCount;
        if(i >= 16)
            i -= 16;
    }
    else    /* Buffer B picture */
    {
        j = pDecCont->StrmStorage.outIndex + pDecCont->StrmStorage.outCount;
        i = j - 1;
        if(j >= 16)
            j -= 16;
        if(i < 0)
            i += 16;
        pDecCont->StrmStorage.outBuf[j] = pDecCont->StrmStorage.outBuf[i];
    }
    j = pDecCont->StrmStorage.workOut;

    frameWidth = ( 15 + pHdrs->horizontalSize ) & ~15;
    frameHeight = ( 15 + pHdrs->verticalSize ) & ~15;
    pDecCont->StrmStorage.pPicBuf[j].frameWidth = frameWidth;
    pDecCont->StrmStorage.pPicBuf[j].frameHeight = frameHeight;
    pDecCont->StrmStorage.pPicBuf[j].codedWidth = pHdrs->horizontalSize;
    pDecCont->StrmStorage.pPicBuf[j].codedHeight = pHdrs->verticalSize;
    pDecCont->StrmStorage.pPicBuf[j].tiledMode = 
        pDecCont->tiledReferenceEnable;

    pDecCont->StrmStorage.outBuf[i] = j;
    if (!bufferB)
    {
        pDecCont->StrmStorage.pPicBuf[j].picId = picId;
        pDecCont->StrmStorage.prevPicId = pDecCont->StrmStorage.picId;
        pDecCont->StrmStorage.picId = picId;
    }
    else
    {
        pDecCont->StrmStorage.pPicBuf[j].picId = 
            pDecCont->StrmStorage.prevPicId + pDecCont->StrmStorage.trb;
    }
    pDecCont->StrmStorage.pPicBuf[j].retVal = returnValue;
    pDecCont->StrmStorage.pPicBuf[j].isInter = isInter;
    pDecCont->StrmStorage.pPicBuf[j].picType = !isInter && !bufferB;
    if(pDecCont->FrameDesc.picCodingType == RV_I_PIC)
        picType = DEC_PIC_TYPE_I;
    else if(pDecCont->FrameDesc.picCodingType == RV_P_PIC)
        picType = DEC_PIC_TYPE_P;
    else if(pDecCont->FrameDesc.picCodingType == RV_B_PIC)
        picType = DEC_PIC_TYPE_B;
    else
        picType = DEC_PIC_TYPE_FI;
    pDecCont->StrmStorage.pPicBuf[j].picCodeType = picType;

    pDecCont->StrmStorage.pPicBuf[j].nbrErrMbs = nbrErrMbs;

    if(pDecCont->ppInstance != NULL && returnValue == FREEZED_PIC_RDY)
        pDecCont->StrmStorage.pPicBuf[j].sendToPp = 2;

    pDecCont->StrmStorage.outCount++;

}

/*------------------------------------------------------------------------------

    x.x Function name:  rvFreeBuffers

        Purpose:        Allocate memory

        Input:          DecContainer *pDecCont

        Output:         RVDEC_MEMFAIL/RVDEC_OK

------------------------------------------------------------------------------*/
void rvFreeRprBuffer(DecContainer * pDecCont)
{

    if(pDecCont->StrmStorage.pRprBuf.data.virtualAddress != NULL)
    {
        DWLFreeRefFrm(pDecCont->dwl,
                      &pDecCont->StrmStorage.pRprBuf.data);
        pDecCont->StrmStorage.pRprBuf.data.virtualAddress = NULL;
        pDecCont->StrmStorage.pRprBuf.data.busAddress = 0;
    }

    if(pDecCont->StrmStorage.rprWorkBuffer.virtualAddress != NULL )
    {
        DWLFreeLinear( pDecCont->dwl, &pDecCont->StrmStorage.rprWorkBuffer );
        pDecCont->StrmStorage.rprWorkBuffer.virtualAddress = NULL;
        pDecCont->StrmStorage.rprWorkBuffer.busAddress = 0;
    }
}

/*------------------------------------------------------------------------------

    x.x Function name:  rvFreeBuffers

        Purpose:        Allocate memory

        Input:          DecContainer *pDecCont

        Output:         RVDEC_MEMFAIL/RVDEC_OK

------------------------------------------------------------------------------*/
void rvFreeBuffers(DecContainer * pDecCont)
{
    u32 i;

    BqueueRelease(&pDecCont->StrmStorage.bq);

    for(i = 0; i < 16; i++)
        if(pDecCont->StrmStorage.pPicBuf[i].data.virtualAddress != NULL)
        {
            DWLFreeRefFrm(pDecCont->dwl,
                          &pDecCont->StrmStorage.pPicBuf[i].data);
            pDecCont->StrmStorage.pPicBuf[i].data.virtualAddress = NULL;
            pDecCont->StrmStorage.pPicBuf[i].data.busAddress = 0;
        }

    if (pDecCont->StrmStorage.directMvs.virtualAddress != NULL)
        DWLFreeLinear(pDecCont->dwl, &pDecCont->StrmStorage.directMvs);
    if (pDecCont->StrmStorage.vlcTables.virtualAddress != NULL)
        DWLFreeLinear(pDecCont->dwl, &pDecCont->StrmStorage.vlcTables);

    rvFreeRprBuffer( pDecCont );
    BqueueRelease(&pDecCont->StrmStorage.bqPp);
}


/*------------------------------------------------------------------------------

    x.x Function name:  rvAllocateBuffers

        Purpose:        Allocate memory

        Input:          DecContainer *pDecCont

        Output:         RVDEC_MEMFAIL/RVDEC_OK

------------------------------------------------------------------------------*/
RvDecRet rvAllocateRprBuffer(DecContainer * pDecCont)
{

    i32 ret = 0;
    u32 sizeTmp = 0;

    ASSERT(pDecCont->StrmStorage.maxMbsPerFrame != 0);

    if(pDecCont->StrmStorage.pRprBuf.data.virtualAddress != NULL)
        return RVDEC_OK; /* already allocated */

    /* Reference images */
    /*sizeTmp = 384 * pDecCont->FrameDesc.totalMbInFrame;*/
    sizeTmp = 384 * pDecCont->StrmStorage.maxMbsPerFrame;

    ret = DWLMallocRefFrmNamed(pDecCont->dwl, sizeTmp,
                           &pDecCont->StrmStorage.pRprBuf.data,
                           "Allocate Reference images");

    RVDEC_DEBUG(("PicBuffer[%d]: %x, %x\n",
                    0,
                    (u32) pDecCont->StrmStorage.pRprBuf.data.
                    virtualAddress,
                    pDecCont->StrmStorage.pRprBuf.data.busAddress));

    if(pDecCont->StrmStorage.pRprBuf.data.busAddress == 0)
    {
        return (RVDEC_MEMFAIL);
    }

    /* Allocate work buffer for look-up tables:
     *  - 2*ptr*height for row pointers
     *  - u16*width for column offsets
     *  - u8*width for x coeffs
     *  - u8*height for y coeffs
     */
    sizeTmp = 2*sizeof(u8*)*pDecCont->StrmStorage.maxFrameHeight + 
              sizeof(u16)*pDecCont->StrmStorage.maxFrameWidth +
              sizeof(u8)*pDecCont->StrmStorage.maxFrameWidth + 
              sizeof(u8)*pDecCont->StrmStorage.maxFrameHeight;

    ret = DWLMallocLinearNamed(pDecCont->dwl, sizeTmp, 
        &pDecCont->StrmStorage.rprWorkBuffer,
        "work buffer for look-up tables");
    if(pDecCont->StrmStorage.rprWorkBuffer.busAddress == 0 )
    {
        return (RVDEC_MEMFAIL);
    }

    if(ret)
        return (RVDEC_MEMFAIL);
    else
        return RVDEC_OK;

}


