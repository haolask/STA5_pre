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


#include "gc_hal_kernel_vxworks.h"
#include "gc_hal_kernel_allocator.h"

#define _GC_OBJ_ZONE gcvZONE_ALLOCATOR

enum um_desc_type
{
    UM_PHYSICAL_MAP,
    UM_PAGE_MAP,
    UM_PFN_MAP,
};

/* Descriptor of a user memory imported. */
struct um_desc
{
    int type;

    union
    {
        /* UM_PHYSICAL_MAP. */
        unsigned long physical;

        /* UM_PAGE_MAP. */
        struct
        {
            struct page **pages;
        };

        /* UM_PFN_MAP. */
        struct
        {
            unsigned long *pfns;
            int *refs;
        };
    };

    unsigned long user_vaddr;
    size_t size;
    unsigned long offset;

    size_t pageCount;
    size_t extraPage;
};

static gceSTATUS
_Import(
    IN gckOS Os,
    IN gctPOINTER Memory,
    IN gctUINT32 Physical,
    IN gctSIZE_T Size,
    IN struct um_desc * UserMemory
    )
{
    unsigned long start, end, memory;

    gctSIZE_T extraPage;
    gctSIZE_T pageCount;

    gcmkHEADER_ARG("Os=0x%p Memory=%p Physical=0x%x Size=%lu", Os, Memory, Physical, Size);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL || Physical != ~0U);
    gcmkVERIFY_ARGUMENT(Size > 0);

    memory = (unsigned long)Memory;

    /* Get the number of required pages. */
    end = (memory + Size + PAGE_SIZE - 1) >> PAGE_SHIFT;
    start = memory >> PAGE_SHIFT;
    pageCount = end - start;

    /* Allocate extra page to avoid cache overflow */
    extraPage = (((memory + gcmALIGN(Size + 64, 64) + PAGE_SIZE - 1) >> PAGE_SHIFT) > end) ? 1 : 0;

    gcmkTRACE_ZONE(
        gcvLEVEL_INFO, _GC_OBJ_ZONE,
        "%s(%d): pageCount: %d. extraPage: %d",
        __FUNCTION__, __LINE__,
        pageCount, extraPage
        );

    /* Overflow. */
    if ((memory + Size) < memory)
    {
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    UserMemory->physical = Physical & PAGE_MASK;
    UserMemory->user_vaddr = (unsigned long)Memory;
    UserMemory->size  = Size;
    UserMemory->offset = (Physical != gcvINVALID_PHYSICAL_ADDRESS)
                       ? (Physical & ~PAGE_MASK)
                       : (memory & ~PAGE_MASK);

    UserMemory->pageCount = pageCount;
    UserMemory->extraPage = extraPage;

    /* Success. */
    gcmkFOOTER();
    return gcvSTATUS_OK;
}

static gceSTATUS
_UserMemoryAttach(
    IN gckALLOCATOR Allocator,
    IN gcsATTACH_DESC_PTR Desc,
    IN PVX_MDL Mdl
    )
{
    gceSTATUS status;
    struct um_desc * userMemory = gcvNULL;

    gckOS os = Allocator->os;

    gcmkHEADER();

    /* Handle is meangless for this importer. */
    gcmkVERIFY_ARGUMENT(Desc != gcvNULL);

    gcmkONERROR(gckOS_Allocate(os, gcmSIZEOF(struct um_desc), (gctPOINTER *)&userMemory));

    gckOS_ZeroMemory(userMemory, gcmSIZEOF(struct um_desc));

    gcmkONERROR(_Import(os, Desc->userMem.memory, Desc->userMem.physical, Desc->userMem.size, userMemory));

    Mdl->priv = userMemory;
    Mdl->numPages = userMemory->pageCount + userMemory->extraPage;
    Mdl->contiguous = gcvTRUE;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (userMemory != gcvNULL)
    {
        gckOS_Free(os,(gctPOINTER)userMemory);
    }
    gcmkFOOTER();
    return status;
}


static void
_UserMemoryFree(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl
    )
{
    gckOS os = Allocator->os;
    struct um_desc *userMemory = Mdl->priv;

    gcmkHEADER();

    if (userMemory)
    {
        gcmkOS_SAFE_FREE(os, userMemory);
    }

    gcmkFOOTER_NO();
}

static gceSTATUS
_UserMemoryMapUser(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN PVX_MDL_MAP MdlMap,
    IN gctBOOL Cacheable
    )
{
    struct um_desc *userMemory = Mdl->priv;

    MdlMap->vmaAddr = (gctPOINTER)userMemory->user_vaddr;
    MdlMap->cacheable = gcvTRUE;

    return gcvSTATUS_OK;
}

static void
_UserMemoryUnmapUser(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN PVX_MDL_MAP MdlMap,
    IN gctUINT32 Size
    )
{
    return;
}

static gceSTATUS
_UserMemoryMapKernel(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    OUT gctPOINTER *Logical
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
_UserMemoryUnmapKernel(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN gctPOINTER Logical
    )
{
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
_UserMemoryCache(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN gctPOINTER Logical,
    IN gctUINT32 Physical,
    IN gctUINT32 Bytes,
    IN gceCACHEOPERATION Operation
    )
{
    return gcvSTATUS_OK;
}

static gceSTATUS
_UserMemoryPhysical(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN gctUINT32 Offset,
    OUT gctPHYS_ADDR_T * Physical
    )
{
    gckOS os = Allocator->os;
    struct um_desc *userMemory = Mdl->priv;
    unsigned long offset = Offset + userMemory->offset;
    gctUINT32 offsetInPage = offset & ~PAGE_MASK;
    gctUINT32 index = offset / PAGE_SIZE;

    if (index >= userMemory->pageCount)
    {
        if (index < userMemory->pageCount + userMemory->extraPage)
        {
            *Physical = KM_TO_PHYS(os->paddingPage);
        }
        else
        {
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    }
    else
    {
        *Physical = userMemory->physical + index * PAGE_SIZE;
    }

    *Physical += offsetInPage;

    return gcvSTATUS_OK;
}

static void
_UserMemoryAllocatorDestructor(
    gcsALLOCATOR *Allocator
    )
{
    if (Allocator->privateData)
    {
        free(Allocator->privateData);
    }

    free(Allocator);
}

/* User memory allocator (importer) operations. */
static gcsALLOCATOR_OPERATIONS UserMemoryAllocatorOperations =
{
    .Attach             = _UserMemoryAttach,
    .Free               = _UserMemoryFree,
    .MapUser            = _UserMemoryMapUser,
    .UnmapUser          = _UserMemoryUnmapUser,
    .MapKernel          = _UserMemoryMapKernel,
    .UnmapKernel        = _UserMemoryUnmapKernel,
    .Cache              = _UserMemoryCache,
    .Physical           = _UserMemoryPhysical,
};

/* Default allocator entry. */
gceSTATUS
_UserMemoryAlloctorInit(
    IN gckOS Os,
    OUT gckALLOCATOR * Allocator
    )
{
    gceSTATUS status;
    gckALLOCATOR allocator;

    gcmkONERROR(
        gckALLOCATOR_Construct(Os, &UserMemoryAllocatorOperations, &allocator));

    allocator->destructor  = _UserMemoryAllocatorDestructor;

    allocator->capability = gcvALLOC_FLAG_USERMEMORY;

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    return status;
}

