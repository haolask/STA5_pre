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
--  Abstract : 
--
------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Table of context 


------------------------------------------------------------------------------*/

#ifndef STRMDEC_SUSI_H
#define STRMDEC_SUSI_H

#include "basetype.h"
#include "mp4dechwd_container.h"

u32 StrmDec_DecodeCustomHeaders(DecContainer * pDecContainer);
void ProcessUserData(DecContainer * pDecContainer);
void ProcessHwOutput( DecContainer * pDecCont );
void SetStrmFmt( DecContainer * pDecCont, u32 strmFmt );
void SetConformanceFlags( DecContainer * pDecCont );
void SetConformanceRegs( DecContainer * pDecCont );
void SetCustomInfo(DecContainer * pDecCont, u32 width, u32 height ) ;

#endif /* STRMDEC_SUSI_H */
