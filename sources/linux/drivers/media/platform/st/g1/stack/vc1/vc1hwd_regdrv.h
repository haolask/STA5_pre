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
--  Description : 
--
------------------------------------------------------------------------------*/

#ifndef VC1REGDRV_H
#define VC1REGDRV_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "regdrv.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

#define DEC_X170_MODE_VC1      4

#define DEC_X170_IRQ_DEC_RDY            DEC_8190_IRQ_RDY
#define DEC_X170_IRQ_BUS_ERROR          DEC_8190_IRQ_BUS
#define DEC_X170_IRQ_BUFFER_EMPTY       DEC_8190_IRQ_BUFFER
#define DEC_X170_IRQ_ASO                DEC_8190_IRQ_ASO
#define DEC_X170_IRQ_STREAM_ERROR       DEC_8190_IRQ_ERROR
#define DEC_X170_IRQ_TIMEOUT            DEC_8190_IRQ_TIMEOUT
#define DEC_X170_IRQ_CLEAR_ALL          0xFF


#endif /* #ifndef VC1REGDRV_H */
