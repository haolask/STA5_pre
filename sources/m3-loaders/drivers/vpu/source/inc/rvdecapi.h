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
--  Description : API for the 8190 RV Decoder
--
------------------------------------------------------------------------------*/

#ifndef __RVDECAPI_H__
#define __RVDECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"
#include "decapicommon.h"

/*------------------------------------------------------------------------------
    API type definitions
------------------------------------------------------------------------------*/

    /* Return values */
    typedef enum
    {
        RVDEC_OK = 0,
        RVDEC_STRM_PROCESSED = 1,
        RVDEC_PIC_RDY = 2,
        RVDEC_HDRS_RDY = 3,
        RVDEC_HDRS_NOT_RDY = 4,
        RVDEC_PIC_DECODED = 5,
        RVDEC_NONREF_PIC_SKIPPED = 6,/* Skipped non-reference picture */

        RVDEC_PARAM_ERROR = -1,
        RVDEC_STRM_ERROR = -2,
        RVDEC_NOT_INITIALIZED = -3,
        RVDEC_MEMFAIL = -4,
        RVDEC_INITFAIL = -5,
        RVDEC_STREAM_NOT_SUPPORTED = -8,

        RVDEC_HW_RESERVED = -254,
        RVDEC_HW_TIMEOUT = -255,
        RVDEC_HW_BUS_ERROR = -256,
        RVDEC_SYSTEM_ERROR = -257,
        RVDEC_DWL_ERROR = -258,
        RVDEC_FORMAT_NOT_SUPPORTED = -1000
    } RvDecRet;

    /* decoder output picture format */
    typedef enum
    {
        RVDEC_SEMIPLANAR_YUV420 = 0x020001,
        RVDEC_TILED_YUV420 = 0x020002
    } RvDecOutFormat;

    typedef struct
    {
        u32 *pVirtualAddress;
        u32 busAddress;
    } RvDecLinearMem;

    /* Decoder instance */
    typedef void *RvDecInst;

    typedef struct
    {
        u32 offset;
        u32 isValid;
    } RvDecSliceInfo;

    /* Input structure */
    typedef struct
    {
        u8 *pStream;             /* Pointer to stream to be decoded              */
        u32 streamBusAddress;    /* DMA bus address of the input stream */
        u32 dataLen;             /* Number of bytes to be decoded                */
        u32 picId;
        u32 timestamp;       /* timestamp of current picture from rv frame header.
                              * NOTE: timestamp of a B-frame should be adjusted referring 
							  * to its forward reference frame's timestamp */

        u32 sliceInfoNum;    /* The number of slice offset entries. */ 
        RvDecSliceInfo *pSliceInfo;     /* Pointer to the sliceInfo.
                                         * It contains offset value of each slice 
                                         * in the data buffer, including start point "0" 
                                         * and end point "dataLen" */
        u32 skipNonReference; /* Flag to enable decoder skip non-reference 
                               * frames to reduce processor load */
    } RvDecInput;

    typedef struct
    {
        u8 *pStrmCurrPos;
        u32 strmCurrBusAddress;          /* DMA bus address location where the decoding
                                          * ended */
        u32 dataLeft;
    } RvDecOutput;

    /* stream info filled by RvDecGetInfo */
    typedef struct
    {
        u32 frameWidth;
        u32 frameHeight;
        u32 codedWidth;
        u32 codedHeight;
        u32 multiBuffPpSize;
        DecDpbMode dpbMode;         /* DPB mode; frame, or field interlaced */       
        RvDecOutFormat outputFormat;
    } RvDecInfo;

    typedef struct
    {
        u8 *pOutputPicture;
        u32 outputPictureBusAddress;
        u32 frameWidth;
        u32 frameHeight;
        u32 codedWidth;
        u32 codedHeight;
        u32 keyPicture;
        u32 picId;
        u32 picCodingType;
        u32 numberOfErrMBs;
        DecOutFrmFormat outputFormat;
    } RvDecPicture;

    /* Version information */
    typedef struct
    {
        u32 major;           /* API major version */
        u32 minor;           /* API minor version */

    } RvDecApiVersion;

    typedef struct DecSwHwBuild_  RvDecBuild;

/*------------------------------------------------------------------------------
    Prototypes of Decoder API functions
------------------------------------------------------------------------------*/

    RvDecApiVersion RvDecGetAPIVersion(void);

    RvDecBuild RvDecGetBuild(void);

    RvDecRet RvDecInit(RvDecInst * pDecInst,
                             DecErrorHandling errorHandling,
                             u32 frameCodeLength,
                             u32 *frameSizes,
                             u32 rvVersion,
                             u32 maxFrameWidth, u32 maxFrameHeight,
                             u32 numFrameBuffers,
                             DecDpbFlags dpbFlags);

    RvDecRet RvDecDecode(RvDecInst decInst,
                               RvDecInput * pInput,
                               RvDecOutput * pOutput);

    RvDecRet RvDecGetInfo(RvDecInst decInst, RvDecInfo * pDecInfo);

    RvDecRet RvDecNextPicture(RvDecInst decInst,
                                    RvDecPicture * pPicture,
                                    u32 endOfStream);

    void RvDecRelease(RvDecInst decInst);

    RvDecRet RvDecPeek(RvDecInst decInst, RvDecPicture * pPicture);

/*------------------------------------------------------------------------------
    Prototype of the API trace funtion. Traces all API entries and returns.
    This must be implemented by the application using the decoder API!
    Argument:
        string - trace message, a null terminated string
------------------------------------------------------------------------------*/
    void RvDecTrace(const char *string);

#ifdef __cplusplus
}
#endif

#endif                       /* __RVDECAPI_H__ */
