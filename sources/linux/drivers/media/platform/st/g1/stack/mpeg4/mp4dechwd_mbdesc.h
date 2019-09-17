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

#ifndef DECMBDESC_H_DEFINED
#define DECMBDESC_H_DEFINED

#include "basetype.h"

typedef struct DecMBDesc_t
{
    u8 typeOfMB; /* inter,interq,inter4v,intra,intraq,stuffing */
    u8 errorStatus;  /* bit 7 indicates whether mb is concealed or not
                         * bit 1 indicates if whole macro block was lost
                         * bit 0 indicates if macro block texture was lost
                         * (in data partitioned packets) */

} DecMBDesc;

#endif
