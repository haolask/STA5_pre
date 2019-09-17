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
--  Abstract : algorithm header file
--
------------------------------------------------------------------------------*/

/*****************************************************************************/

#ifndef DECSVDESC_H_DEFINED
#define DECSVDESC_H_DEFINED

#include "basetype.h"

typedef struct DecSvDesc_t
{
    u32 splitScreenIndicator;
    u32 documentCameraIndicator;
    u32 fullPictureFreezeRelease;
    u32 sourceFormat;
    u32 gobFrameId;
    u32 temporalReference;
    u32 tics;   /* total tics from the beginning */
    u32 numMbsInGob;
    u32 numGobsInVop;

    u32 cpcf;   /* flag indicating if custom picture clock freq used */
    u32 cpcfc;  /* custom picture clock frequency code */
    u32 etr;    /* extended temporal reference */
} DecSvDesc;

#endif
