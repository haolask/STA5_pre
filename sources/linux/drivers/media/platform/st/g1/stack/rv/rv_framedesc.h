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

#ifndef RV_FRAMEDESC_H
#define RV_FRAMEDESC_H

#include "basetype.h"

typedef struct
{
    u32 frameNumber;
    u32 frameTimePictures;
    u32 picCodingType;
    u32 totalMbInFrame;
    u32 frameWidth; /* in macro blocks */
    u32 frameHeight;    /* in macro blocks */
    u32 timeCodeHours;
    u32 timeCodeMinutes;
    u32 timeCodeSeconds;
    u32 vlcSet;
    u32 qp;
} DecFrameDesc;

#endif /* #ifndef RV_FRAMEDESC_H */
