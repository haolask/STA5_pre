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
--  Abstract : API's internal static data storage definition
--
------------------------------------------------------------------------------*/

#ifndef MP4DECAPISTORAGE_H
#define MP4DECAPISTORAGE_H

#include "dwl.h"

typedef struct
{
    enum
    {
        UNINIT,
        INITIALIZED,
        HEADERSDECODED,
        STREAMDECODING,
        HW_VOP_STARTED,
        HW_STRM_ERROR
    } DecStat;

    enum
    {
        NO_BUFFER = 0,
        BUFFER_0,
        BUFFER_1,
        BUFFER_2,
        BUFFER_3,
    } bufferForPp;

    DWLLinearMem_t InternalFrameIn;
    DWLLinearMem_t InternalFrameOut;
    DWLLinearMem_t quantMat;

    DWLLinearMem_t directMvs;

    u32 firstHeaders;
    u32 disableFilter;
    u32 outputOtherField;

} DecApiStorage;

#endif /* MP4DECAPISTORAGE_H */
