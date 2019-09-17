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
--  Description : Top level interface for sequence and picture layer decoding
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_DECODER_H
#define VC1HWD_DECODER_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "vc1decapi.h"
#include "vc1hwd_container.h"
#include "vc1hwd_storage.h"
#include "vc1hwd_util.h"
#include "vc1hwd_stream.h"
#include "vc1hwd_headers.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

#define INVALID_ANCHOR_PICTURE  ((u16x)(-1))

#define MIN_PIC_WIDTH   48
#define MIN_PIC_HEIGHT  48
#define MAX_NUM_MBS     ((4096>>4)*(4096>>4))

/* enumerated return values of the functions */
enum {
    VC1HWD_OK,
    VC1HWD_NOT_CODED_PIC,
    VC1HWD_PIC_RDY,
    VC1HWD_SEQ_HDRS_RDY,
    VC1HWD_ENTRY_POINT_HDRS_RDY,
    VC1HWD_END_OF_SEQ,
    VC1HWD_PIC_HDRS_RDY,
    VC1HWD_FIELD_HDRS_RDY,
    VC1HWD_ERROR,
    VC1HWD_METADATA_ERROR,
    VC1HWD_MEMORY_FAIL,
    VC1HWD_USER_DATA_RDY,
    VC1HWD_HDRS_ERROR
};

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

u16x vc1hwdInit( const void *dwl, swStrmStorage_t *pStorage,
    const VC1DecMetaData *pMetaData,
    u32 numFrameBuffers );

u16x vc1hwdDecode( decContainer_t *pDecCont, 
                   swStrmStorage_t *pStorage, 
                   strmData_t *strmData );

u16x vc1hwdRelease( const void *dwl, swStrmStorage_t *pStorage );

u16x vc1hwdUnpackMetaData( const u8 *pBuffer, VC1DecMetaData *MetaData );

void vc1hwdErrorConcealment( const u16x flush, 
                             swStrmStorage_t * pStorage );

u16x vc1hwdNextPicture( swStrmStorage_t * pStorage, u16x * pNextPicture,
                       u32* pFieldToRet,  u16x endOfStream, u32 deinterlace,
                       u32* pPicId, u32* errMbs );

u16x vc1hwdBufferPicture( swStrmStorage_t * pStorage, u16x picToBuffer,
                          u16x bufferB, u16x picId, u16x errMbs );

u32 vc1hwdSeekFrameStart( swStrmStorage_t * pStorage,
                             strmData_t *pStrmData );

void vc1hwdSetPictureInfo( decContainer_t *pDecCont, u32 picId );

void vc1hwdUpdateWorkBufferIndexes( swStrmStorage_t * pStorage, u32 isBPic );

#endif /* #ifndef VC1HWD_DECODER_H */

