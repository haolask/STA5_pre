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
--  Abstract : Picture level data structure
--
------------------------------------------------------------------------------*/

/*****************************************************************************/

#ifndef DECVOPDESC_H_DEFINED
#define DECVOPDESC_H_DEFINED

#include "basetype.h"

typedef struct DecVopDesc_t
{
    u32 vopNumber;
    u32 vopNumberInSeq;
    u32 vopTimeIncrement;
    u32 moduloTimeBase;
    u32 prevVopTimeIncrement;
    u32 prevModuloTimeBase;
    u32 trb;
    u32 trd;
    u32 ticsFromPrev;   /* tics (1/vop_time_increment_resolution
                             * seconds) since previous vop */
    u32 intraDcVlcThr;
    u32 vopCodingType;
    u32 totalMbInVop;
    u32 vopWidth;   /* in macro blocks */
    u32 vopHeight;  /* in macro blocks */
    u32 qP;
    u32 fcodeFwd;
    u32 fcodeBwd;
    u32 vopCoded;
    u32 vopRoundingType;
    /* following three parameters will be read from group of VOPs header
     * and will be updated based on time codes received in VOP header */
    u32 timeCodeHours;
    u32 timeCodeMinutes;
    u32 timeCodeSeconds;
    u32 govCounter; /* number of groups of VOPs */

    /* for interlace support */
    u32 topFieldFirst;
    u32 altVerticalScanFlag;

} DecVopDesc;

#endif
