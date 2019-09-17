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
--  Abstract : JPEG decoder and PP pipeline support
--
------------------------------------------------------------------------------*/

#ifndef JPEG_PP_PIPELINE_H
#define JPEG_PP_PIPELINE_H

#include "decppif.h"

i32 jpegRegisterPP(const void *decInst, const void *ppInst,
                   void (*PPRun) (const void *, const DecPpInterface *),
                   void (*PPEndCallback) (const void *),
                   void (*PPConfigQuery) (const void *, DecPpQuery *));

i32 jpegUnregisterPP(const void *decInst, const void *ppInst);

#endif /* #ifdef JPEG_PP_PIPELINE_H */
