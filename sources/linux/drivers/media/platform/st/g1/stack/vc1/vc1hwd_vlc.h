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
--  Description : VLC decoding functionality
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_VLC_H
#define VC1HWD_VLC_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "vc1hwd_stream.h"
#include "vc1hwd_picture_layer.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/* Special cases for VLC value */
#define INVALID_VLC_VALUE   (-1)

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

/* Picture layer codewords */
picType_e vc1hwdDecodePtype( strmData_t * const strmData, const u32 advanced,
                             const u16x maxBframes);
mvmode_e vc1hwdDecodeMvMode( strmData_t * const strmData, const u32 bPic,
                             const u16x pquant, u32 *pIntComp );
mvmode_e vc1hwdDecodeMvModeB( strmData_t * const strmData, const u16x pquant);
u16x vc1hwdDecodeTransAcFrm( strmData_t * const strmData );
void vc1hwdDecodeVopDquant( strmData_t * const strmData, const u16x dquant,
                            pictureLayer_t * const pLayer );
u16x vc1hwdDecodeMvRange( strmData_t * const strmData );
bfract_e vc1hwdDecodeBfraction( strmData_t * const strmData,
    i16x * pScaleFactor );

fcm_e vc1hwdDecodeFcm( strmData_t * const strmData );
u16x vc1hwdDecodeCondOver( strmData_t * const strmData );
u16x vc1hwdDecodeRefDist( strmData_t * const strmData );
u16x vc1hwdDecodeDmvRange( strmData_t * const strmData );
intCompField_e vc1hwdDecodeIntCompField( strmData_t * const strmData );

#endif /* #ifndef VC1HWD_VLC_H */
