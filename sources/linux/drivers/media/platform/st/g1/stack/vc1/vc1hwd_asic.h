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
--  Description : interface for interaction to hw
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_ASIC_H
#define VC1HWD_ASIC_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "basetype.h"
#include "vc1hwd_container.h"
#include "vc1hwd_stream.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

#define X170_DEC_TIMEOUT        0xFFU
#define X170_DEC_SYSTEM_ERROR   0xFEU
#define X170_DEC_HW_RESERVED    0xFDU

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

u32 VC1RunAsic(decContainer_t *pDecCont, strmData_t *pStrmData,
    u32 strmBusAddr);

void Vc1DecPpSetPpOutpStandalone( vc1DecPp_t *pDecPp, DecPpInterface *pc );
void Vc1DecPpNextInput( vc1DecPp_t *pDecPp, u32 framePic );

#ifdef _DEC_PP_USAGE
void PrintDecPpUsage( decContainer_t *pDecCont, 
                      u32 ff, 
                      u32 picIndex, 
                      u32 decStatus,
                      u32 picId);
#endif

#endif /* #ifndef VC1HWD_ASIC_H */
