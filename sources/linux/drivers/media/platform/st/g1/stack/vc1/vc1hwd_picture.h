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
--  Description : Container for picture and field information
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_PICTURE_H
#define VC1HWD_PICTURE_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "dwl.h"
#include "vc1decapi.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

typedef enum 
{
    NONE = 0,
    PIPELINED = 1,
    PARALLEL = 2,
    STAND_ALONE = 3
} decPpStatus_e;

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

/* Part of picture buffer descriptor. This informaton is unique 
 * for each field of the frame. */
typedef struct field 
{
    intCompField_e intCompF;
    i32 iScaleA;            /* A used for TOP */
    i32 iShiftA;
    i32 iScaleB;            /* B used for BOTTOM */
    i32 iShiftB;
    
    picType_e type;
    u32 picId;

    u32 ppBufferIndex;
    decPpStatus_e decPpStat; /* pipeline / parallel ... */
    VC1DecRet returnValue;
} field_t;


/* Picture buffer descriptor. Used for anchor frames
 * and work buffer and output buffer. */
typedef struct picture 
{
    DWLLinearMem_t data;

    fcm_e fcm;              /* frame coding mode */
    
    u16x codedWidth;        /* Coded height in pixels */
    u16x codedHeight;       /* Coded widht in pixels */
    u16x keyFrame;          /* for field pictures both must be Intra */

    u16x rangeRedFrm;       /* Main profile range reduction information */
    u32 rangeMapYFlag;      /* Advanced profile range mapping parameters */
    u32 rangeMapY;
    u32 rangeMapUvFlag;
    u32 rangeMapUv;
    
    u32 isFirstField;       /* Is current field first or second */
    u32 isTopFieldFirst;    /* Which field is first */
    u32 rff;                /* repeat first field */
    u32 rptfrm;             /* repeat frame count */
    u32 tiledMode;
    u32 picCodeType[2];

    field_t field[2];       /* 0 = first; 1 = second */
} picture_t;

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

#endif /* #ifndef VC1SWD_PICTURE_H */

