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
--  Description : Jpeg Decoder Internal functions
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents 
   
    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef JPEGDECINTERNAL_H
#define JPEGDECINTERNAL_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "basetype.h"
#include "jpegdecapi.h"
#include "jpegdeccontainer.h"

#ifdef JPEGDEC_ASIC_TRACE
#include <stdio.h>
#endif

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/
#ifdef JPEGDEC_ASIC_TRACE
#define JPEGDEC_TRACE_INTERNAL(args) printf args
#else
#define JPEGDEC_TRACE_INTERNAL(args)
#endif

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/
void JpegDecClearStructs(JpegDecContainer * pJpegDecCont, u32 mode);
JpegDecRet JpegDecInitHW(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWContinue(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWInputBuffLoad(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWProgressiveContinue(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWNonInterleaved(JpegDecContainer * pJpegDecCont);
JpegDecRet JpegDecAllocateResidual(JpegDecContainer * pJpegDecCont);
void JpegDecSliceSizeCalculation(JpegDecContainer * pJpegDecCont);
JpegDecRet JpegDecNextScanHdrs(JpegDecContainer * pJpegDecCont);
void JpegRefreshRegs(JpegDecContainer * pJpegDecCont);
void JpegFlushRegs(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWEmptyScan(JpegDecContainer * pJpegDecCont, u32 componentId);

#endif /* #endif JPEGDECDATA_H */
