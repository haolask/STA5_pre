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
--  Abstract : Stream decoding top header file
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of context 

     1. xxx...
   
 
------------------------------------------------------------------------------*/

#ifndef RV_STRMDEC_H
#define RV_STRMDEC_H

#include "rv_container.h"

enum
{
    DEC_RDY,
    DEC_HDRS_RDY,
    DEC_PIC_HDR_RDY,
    DEC_PIC_HDR_RDY_RPR,
    DEC_PIC_HDR_RDY_ERROR,
    DEC_ERROR,
    DEC_END_OF_STREAM
};

/* function prototypes */
u32 rv_StrmDecode(DecContainer * pDecContainer);

#endif /* #ifndef RV_STRMDEC_H */
