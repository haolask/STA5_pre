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
--  Description : Decoder container
--
------------------------------------------------------------------------------*/

#ifndef VC1HWD_CONTAINER_H
#define VC1HWD_CONTAINER_H

/*------------------------------------------------------------------------------
    Include headers
------------------------------------------------------------------------------*/

#include "vc1hwd_storage.h"
#include "deccfg.h"
#include "decppif.h"
#include "refbuffer.h"

/*------------------------------------------------------------------------------
    Module defines
------------------------------------------------------------------------------*/

/* String length for tracing */
#define VC1DEC_TRACE_STR_LEN 100

/*------------------------------------------------------------------------------
    Data types
------------------------------------------------------------------------------*/

typedef struct
{
    enum {
        VC1DEC_UNINITIALIZED,
        VC1DEC_INITIALIZED,
        VC1DEC_RESOURCES_ALLOCATED
    } decStat;

    swStrmStorage_t storage;
#ifdef VC1DEC_TRACE
    char str[VC1DEC_TRACE_STR_LEN];
#endif
    u32 picNumber;
    u32 asicRunning;
    u32 vc1Regs[DEC_X170_REGISTERS];

    u32 maxWidthHw;

    DecPpInterface ppControl;
    DecPpQuery ppConfigQuery; /* Decoder asks pp info about setup, 
                                 info stored here */
    u32 ppStatus;
    u32 refBufSupport;
    u32 tiledModeSupport;
    u32 tiledReferenceEnable;
    u32 allowDpbFieldOrdering;
    u32 dpbMode;
    refBuffer_t refBufferCtrl;
    DWLLinearMem_t bitPlaneCtrl;
    DWLLinearMem_t directMvs;
    const void *dwl;    /* DWL instance */
    i32 coreID;
    const void *ppInstance;
    void (*PPRun)(const void *, const DecPpInterface *);
    void (*PPEndCallback) (const void *);
    void (*PPConfigQuery)(const void *, DecPpQuery *);
    void (*PPDisplayIndex)(const void *, u32);
    void (*PPBufferData) (const void *, u32, u32, u32, u32, u32);
} decContainer_t;

/*------------------------------------------------------------------------------
    Function prototypes
------------------------------------------------------------------------------*/

#endif /* #ifndef VC1HWD_CONTAINER_H */

