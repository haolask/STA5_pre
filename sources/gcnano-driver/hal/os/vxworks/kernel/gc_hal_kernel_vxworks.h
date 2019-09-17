/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2014 - 2018 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (C) 2014 - 2018 Vivante Corporation
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#ifndef __gc_hal_kernel_vxworks_h_
#define __gc_hal_kernel_vxworks_h_

#include <stdio.h>

#include <taskLib.h>
#include <spinlockLib.h>
#include <isrLib.h>
#include <vxAtomicLib.h>
#include <tickLib.h>
#include <semLib.h>
#include <iosLib.h>
#include <intLib.h>
#include <memLib.h>
#include <sysLib.h>

#include "gc_hal.h"
#include "gc_hal_driver.h"
#include "gc_hal_kernel.h"
#include "gc_hal_kernel_platform.h"
#include "gc_hal_kernel_device.h"
#include "gc_hal_kernel_os.h"
#include "gc_hal_ta.h"

#ifndef DEVICE_NAME
#   define DEVICE_NAME              "galcore"
#endif

#ifndef CLASS_NAME
#   define CLASS_NAME               "graphics_class"
#endif

/* TODO: */
#define PAGE_SIZE   0x4000
#define PAGE_SHIFT  14
#define PAGE_MASK   (~(PAGE_SIZE - 1))

#define GetPageCount(size, offset) ((((size) + ((offset) & ~PAGE_MASK)) + PAGE_SIZE - 1) >> PAGE_SHIFT)

extern struct device *galcore_device;

/******************************************************************************\
********************************** Structures **********************************
\******************************************************************************/
typedef struct _gcsIOMMU * gckIOMMU;

typedef struct _gcsUSER_MAPPING * gcsUSER_MAPPING_PTR;
typedef struct _gcsUSER_MAPPING
{
    /* Pointer to next mapping structure. */
    gcsUSER_MAPPING_PTR         next;

    /* Physical address of this mapping. */
    gctUINT32                   physical;

    /* Logical address of this mapping. */
    gctPOINTER                  logical;

    /* Number of bytes of this mapping. */
    gctSIZE_T                   bytes;

    /* Starting address of this mapping. */
    gctINT8_PTR                 start;

    /* Ending address of this mapping. */
    gctINT8_PTR                 end;
}
gcsUSER_MAPPING;

struct _gckOS
{
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to device */
    gckGALDEVICE                device;

    /* Memory management */
    pthread_mutex_t             mdlMutex;
    struct list_head            mdlHead;

    /* Kernel process ID. */
    gctUINT32                   kernelProcessID;

    /* Signal management. */

    /* Lock. */
    pthread_mutex_t             signalMutex;

    gcsUSER_MAPPING_PTR         userMap;

    /* Allocate extra page to avoid cache overflow */
    gctPOINTER                  paddingPage;

    /* Detect unfreed allocation. */
    atomic_t                    allocateCount;

    struct list_head            allocatorList;

    /* Lock for register access check. */
    spinlockIsr_t               registerAccessLock;

    /* External power states. */
    gctBOOL                     powerStates[gcdMAX_GPU_COUNT];

    /* External clock states. */
    gctBOOL                     clockStates[gcdMAX_GPU_COUNT];

    /* IOMMU. */
    gckIOMMU                    iommu;
};

typedef struct _gcsSIGNAL * gcsSIGNAL_PTR;
typedef struct _gcsSIGNAL
{
    /* Kernel sync primitive. */
    volatile unsigned int done;
    spinlockTask_t lock;

    SEM_ID sem;

    /* Manual reset flag. */
    gctBOOL manualReset;

    /* The reference counter. */
    atomic_t ref;

    /* The owner of the signal. */
    gctHANDLE process;
}
gcsSIGNAL;

gceSTATUS
gckOS_ImportAllocators(
    gckOS Os
    );

gceSTATUS
gckOS_FreeAllocators(
    gckOS Os
    );

/* Reserved memory. */
gceSTATUS
gckOS_RequestReservedMemory(
    gckOS Os,
    unsigned long Start,
    unsigned long Size,
    const char * Name,
    gctBOOL Requested,
    void ** MemoryHandle
    );

void
gckOS_ReleaseReservedMemory(
    gckOS Os,
    void * MemoryHandle
    );

gceSTATUS
_ConvertLogical2Physical(
    IN gckOS Os,
    IN gctPOINTER Logical,
    IN gctUINT32 ProcessID,
    IN PVX_MDL Mdl,
    OUT gctPHYS_ADDR_T * Physical
    );

gctBOOL
_QuerySignal(
    IN gckOS Os,
    IN gctSIGNAL Signal
    );

static inline gctINT
_GetProcessID(
    void
    )
{
    return taskIdSelf();
}

#ifdef CONFIG_IOMMU_SUPPORT
void
gckIOMMU_Destory(
    IN gckOS Os,
    IN gckIOMMU Iommu
    );

gceSTATUS
gckIOMMU_Construct(
    IN gckOS Os,
    OUT gckIOMMU * Iommu
    );

gceSTATUS
gckIOMMU_Map(
    IN gckIOMMU Iommu,
    IN gctUINT32 DomainAddress,
    IN gctUINT32 Physical,
    IN gctUINT32 Bytes
    );

gceSTATUS
gckIOMMU_Unmap(
    IN gckIOMMU Iommu,
    IN gctUINT32 DomainAddress,
    IN gctUINT32 Bytes
    );
#endif

#endif /* __gc_hal_kernel_vxworks_h_ */
