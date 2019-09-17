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

#ifndef __ON2RVDECAPI_H__
#define __ON2RVDECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "basetype.h"

/* return values */
#define MAKE_RESULT(sev,fac,code) \
    (((u32)sev << 31) | ((u32)4 << 16) | ((fac<<6) | (code)))

#define ON2RVDEC_OK                     MAKE_RESULT(0,0,0x0000)
#define ON2RVDEC_OUTOFMEMORY            MAKE_RESULT(1,7,0x000e)
#define ON2RVDEC_INVALID_PARAMETER      MAKE_RESULT(1,7,0x0057)
#define ON2RVDEC_NOTIMPL                MAKE_RESULT(1,0,0x4001)
#define ON2RVDEC_POINTER                MAKE_RESULT(1,0,0x4003)
#define ON2RVDEC_FAIL                   MAKE_RESULT(1,0,0x4005)

typedef u32 On2RvDecRet;

/* custom message handling */
#define ON2RV_MSG_ID_Set_RVDecoder_RPR_Sizes 36

typedef u32 On2RvCustomMessage_ID;

typedef struct
{
    On2RvCustomMessage_ID message_id;
    u32 num_sizes;
    u32 *sizes;
} On2RvMsgSetDecoderRprSizes;

/* input and output flag definitions */
#define ON2RV_DECODE_MORE_FRAMES    0x00000001
#define ON2RV_DECODE_DONT_DRAW      0x00000002
#define ON2RV_DECODE_KEY_FRAME      0x00000004
#define ON2RV_DECODE_B_FRAME        0x00000008
#define ON2RV_DECODE_LAST_FRAME     0x00000200

/* frame formats */
/* Reference picture format types */
typedef enum 
{
    ON2RV_REF_FRM_RASTER_SCAN          = 0,
    ON2RV_REF_FRM_TILED_DEFAULT        = 1
} On2RvRefFrmFormat;

/* Output picture format types */
typedef enum
{
    ON2RV_OUT_FRM_RASTER_SCAN          = 0,
    ON2RV_OUT_FRM_TILED_8X4            = 1
} On2RvOutFrmFormat;

/* input and output structures */
typedef struct
{
    i32 bIsValid;
    u32 ulSegmentOffset;
} codecSegmentInfo;

typedef struct
{
    u32 dataLength;
    i32 bInterpolateImage;
    u32 numDataSegments;
    codecSegmentInfo *pDataSegments;
    u32 flags;
    u32 timestamp;
    u32 streamBusAddr;
    u32 skipNonReference;
} On2DecoderInParams;

typedef struct
{
    u32 numFrames;
    u32 notes;
    u32 timestamp;
    u32 width;
    u32 height;
    u8 *pOutFrame;
    On2RvOutFrmFormat outputFormat;
} On2DecoderOutParams;

/* decoder initialization structure */
typedef struct
{
    u16 outtype;
    u16 pels;
    u16 lines;
    u16 nPadWidth;
    u16 nPadHeight;
    u16 pad_to_32;
    u32 ulInvariants;
    i32 packetization;
    u32 ulStreamVersion;
} On2DecoderInit;

/* decoding function */
On2RvDecRet On2RvDecDecode(u8 *pRV10Packets,
    u8   *pDecodedFrameBuffer, /* unused */
    void *pInputParams,
    void *pOutputParams,
    void *decInst);

/* initialization function */
On2RvDecRet On2RvDecInit(void *pRV10Init,
    void **pDecInst);

/* release function */
On2RvDecRet On2RvDecFree(void *decInst);

/* custom message handling function. Only Set_RPR_Sizes message implemented */
On2RvDecRet On2RvDecCustomMessage(void *msg_id, void *decInst);

/* unused, always returns DEC_NOTIMPL */
On2RvDecRet On2RvDecHiveMessage(void *msg, void *decInst);

/* function to obtain last decoded picture out from the decoder */
On2RvDecRet On2RvDecPeek(void *pOutputParams, void *decInst);

/* function to specify nbr of picture buffers to decoder */
On2RvDecRet On2RvDecSetNbrOfBuffers( u32 nbrBuffers, void *global );

/* function to specify reference frame format to use */
On2RvDecRet On2RvDecSetReferenceFrameFormat( 
    On2RvRefFrmFormat referenceFrameFormat, void *global );


#ifdef __cplusplus
}
#endif

#endif  /* __ON2RVDECAPI_H__ */
