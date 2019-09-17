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

#ifndef _MPEG2DECCONTAINER_H_
#define _MPEG2DECCONTAINER_H_

#include "basetype.h"
#include "mpeg2hwd_framedesc.h"
#include "mpeg2hwd_mbsetdesc.h"
#include "mpeg2hwd_strmdesc.h"
#include "mpeg2hwd_hdrs.h"
#include "mpeg2hwd_storage.h"
#include "mpeg2hwd_apistorage.h"
#include "mpeg2hwd_cfg.h"
#include "deccfg.h"
#include "decppif.h"
#include "refbuffer.h"
#include "workaround.h"

typedef struct
{
    u32 mpeg2Regs[DEC_X170_REGISTERS];
    DecFrameDesc FrameDesc; /* Frame description */
    DecMbSetDesc MbSetDesc; /* Mb set descriptor */
    DecStrmDesc StrmDesc;
    DecStrmStorage StrmStorage; /* StrmDec storage */
    DecHdrs Hdrs;
    DecHdrs tmpHdrs;    /* for decoding of repeated headers */
    DecApiStorage ApiStorage;   /* Api's internal data storage */
    DecPpInterface ppControl;
    DecPpQuery ppConfigQuery;   /* Decoder asks pp info about setup, info stored here */
    u32 ppStatus;
    u32 asicRunning;
    u32 mbErrorConceal;
    const void *dwl;
    i32 coreID;
    u32 refBufSupport;
    u32 tiledModeSupport;
    u32 tiledReferenceEnable;
    u32 allowDpbFieldOrdering;
    DecDpbMode dpbMode;
    refBuffer_t refBufferCtrl;
    u32 keepHwReserved;
    u32 unpairedField;
    u32 noReallocation;

    workaround_t workarounds;

    const void *ppInstance;
    void (*PPRun) (const void *, DecPpInterface *);
    void (*PPEndCallback) (const void *);
    void (*PPConfigQuery) (const void *, DecPpQuery *);
    void (*PPDisplayIndex)(const void *, u32);
    void (*PPBufferData) (const void *, u32, u32, u32, u32, u32);

} DecContainer;

#endif /* #ifndef _MPEG2DECCONTAINER_H_ */
