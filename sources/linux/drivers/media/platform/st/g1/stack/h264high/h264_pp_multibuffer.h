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
-
-  Description : Dec/PP multibuiffer handling
-
------------------------------------------------------------------------------*/

#ifndef H264_PP_MULTIBUFFER_H
#define H264_PP_MULTIBUFFER_H

#include "basetype.h"
#include "dwl.h"
#include "h264hwd_container.h"

void h264PpMultiInit(decContainer_t * pDecCont, u32 maxBuff);
u32 h264PpMultiAddPic(decContainer_t * pDecCont, const DWLLinearMem_t * data);
u32 h264PpMultiRemovePic(decContainer_t * pDecCont,
                         const DWLLinearMem_t * data);
u32 h264PpMultiFindPic(decContainer_t * pDecCont, const DWLLinearMem_t * data);

void h264PpMultiMvc(decContainer_t * pDecCont, u32 maxBuffId);

#endif /* H264_PP_MULTIBUFFER_H */
