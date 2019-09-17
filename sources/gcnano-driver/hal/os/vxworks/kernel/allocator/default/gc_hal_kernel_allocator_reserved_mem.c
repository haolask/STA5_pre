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
#define _GC_OBJ_ZONE    gcvZONE_OS

/*
 * reserved_mem is for contiguous pool, internal pool and external pool, etc.
 */

/* mdl private. */
struct reserved_mem
{
    unsigned long start;
    unsigned long size;
    char name[32];
    int  release;

    /* Link together. */
    struct list_head link;
};

/* allocator info. */
struct reserved_mem_alloc
{
    /* Record allocated reserved memory regions. */
    struct list_head region;
    pthread_mutex_t lock;
};

static gceSTATUS
reserved_mem_attach(
    IN gckALLOCATOR Allocator,
    IN gcsATTACH_DESC_PTR Desc,
    IN PVX_MDL Mdl
    )
{
    struct reserved_mem_alloc *alloc = Allocator->privateData;
    struct reserved_mem *res;

    res = (struct reserved_mem *)malloc(sizeof(struct reserved_mem));

    if (!res)
        return gcvSTATUS_OUT_OF_MEMORY;

    res->start = Desc->reservedMem.start;
    res->size  = Desc->reservedMem.size;
    strncpy(res->name, Desc->reservedMem.name, sizeof(res->name)-1);

    if (!Desc->reservedMem.requested)
    {
        /*TODO: Request memory region. */
        res->release = 1;
    }

    pthread_mutex_lock(&alloc->lock);
    list_add(&res->link, &alloc->region);
    pthread_mutex_unlock(&alloc->lock);

    Mdl->priv = res;

    return gcvSTATUS_OK;
}

static void
reserved_mem_detach(
    IN gckALLOCATOR Allocator,
    IN OUT PVX_MDL Mdl
    )
{
    struct reserved_mem_alloc *alloc = Allocator->privateData;
    struct reserved_mem *res = Mdl->priv;

    /* unlink from region list. */
    pthread_mutex_lock(&alloc->lock);
    list_del_init(&res->link);
    pthread_mutex_unlock(&alloc->lock);

    if (res->release)
    {
        /*TODO: Release memory region. */
    }

    free(res);
}

static void
reserved_mem_unmap_user(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN PVX_MDL_MAP MdlMap,
    IN gctUINT32 Size
    )
{
    return;
}

static gceSTATUS
reserved_mem_map_user(
    gckALLOCATOR Allocator,
    PVX_MDL Mdl,
    PVX_MDL_MAP MdlMap,
    gctBOOL Cacheable
    )
{
    return gcvSTATUS_OK;
}

static gceSTATUS
reserved_mem_map_kernel(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    OUT gctPOINTER *Logical
    )
{
    return gcvSTATUS_OK;
}

static gceSTATUS
reserved_mem_unmap_kernel(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN gctPOINTER Logical
    )
{
    return gcvSTATUS_OK;
}

static gceSTATUS
reserved_mem_cache_op(
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
reserved_mem_get_physical(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN gctUINT32 Offset,
    OUT gctPHYS_ADDR_T * Physical
    )
{
    struct reserved_mem *res = Mdl->priv;
    *Physical = res->start + Offset;

    return gcvSTATUS_OK;
}

static void
reserved_mem_dtor(
    gcsALLOCATOR *Allocator
    )
{
    if (Allocator->privateData)
    {
        free(Allocator->privateData);
    }

    free(Allocator);
}

/* GFP allocator operations. */
static gcsALLOCATOR_OPERATIONS reserved_mem_ops = {
    .Alloc              = NULL,
    .Attach             = reserved_mem_attach,
    .Free               = reserved_mem_detach,
    .MapUser            = reserved_mem_map_user,
    .UnmapUser          = reserved_mem_unmap_user,
    .MapKernel          = reserved_mem_map_kernel,
    .UnmapKernel        = reserved_mem_unmap_kernel,
    .Cache              = reserved_mem_cache_op,
    .Physical           = reserved_mem_get_physical,
};

/* GFP allocator entry. */
gceSTATUS
_ReservedMemoryAllocatorInit(
    IN gckOS Os,
    OUT gckALLOCATOR * Allocator
    )
{
    gceSTATUS status;
    gckALLOCATOR allocator = gcvNULL;
    struct reserved_mem_alloc *alloc = NULL;

    gcmkONERROR(
        gckALLOCATOR_Construct(Os, &reserved_mem_ops, &allocator));

    alloc = (struct reserved_mem_alloc *) malloc(sizeof(*alloc));

    if (!alloc)
    {
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    INIT_LIST_HEAD(&alloc->region);
    pthread_mutex_init(&alloc->lock, 0);

    /* Register private data. */
    allocator->privateData = alloc;
    allocator->destructor = reserved_mem_dtor;

    allocator->capability = gcvALLOC_FLAG_LINUX_RESERVED_MEM;

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    if (allocator)
    {
        free(allocator);
    }
    return status;
}

