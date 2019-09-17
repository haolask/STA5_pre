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

#ifndef _DECCONTAINER_H_
#define _DECCONTAINER_H_

#include "basetype.h"
#include "mp4dechwd_vopdesc.h"
#include "mp4dechwd_mbsetdesc.h"
#include "mp4dechwd_strmdesc.h"
#include "mp4dechwd_mbdesc.h"
#include "mp4dechwd_hdrs.h"
#include "mp4dechwd_svdesc.h"
#include "mp4dechwd_storage.h"
#include "mp4dechwd_mvstorage.h"
#include "mp4decapihwd_storage.h"
#include "mp4deccfg.h"
#include "deccfg.h"
#include "decppif.h"
#include "refbuffer.h"
#include "workaround.h"

typedef struct DecContainer_t
{
    u32 mp4Regs[DEC_X170_REGISTERS];
    DecVopDesc VopDesc;         /* VOP description */
    DecMbSetDesc MbSetDesc;     /* Mb set descriptor */
    DecMBDesc MBDesc[MP4API_DEC_MBS];
    DecStrmDesc StrmDesc;
    DecStrmStorage StrmStorage; /* StrmDec storage */
    DecHdrs Hdrs;
    DecHdrs tmpHdrs;
    DecSvDesc SvDesc;   /* Short video descriptor */
    DecApiStorage ApiStorage;  /* Api's internal data storage */
    DecPpInterface ppControl;
    DecPpQuery ppConfigQuery; /* Decoder asks pp info about setup, info stored here */
    u32 ppStatus;
    u32 asicRunning;
    u32 rlcMode;
    const void *dwl;
    i32 coreID;
    u32 refBufSupport;
    u32 tiledModeSupport;
    u32 tiledReferenceEnable;
    u32 allowDpbFieldOrdering;
    DecDpbMode dpbMode;    
    refBuffer_t refBufferCtrl;
    workaround_t workarounds;
    u32 packedMode;

    const void *ppInstance;
    void (*PPRun) (const void *, DecPpInterface *);
    void (*PPEndCallback) (const void *);
    void  (*PPConfigQuery)(const void *, DecPpQuery *);
    void (*PPDisplayIndex)(const void *, u32);
    void (*PPBufferData) (const void *, u32, u32, u32, u32, u32);

} DecContainer;

#endif /* _DECCONTAINER_H_ */
