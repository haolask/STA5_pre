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

struct mdl_priv {
    gctPOINTER kvaddr;
    gctPHYS_ADDR_T phys;
};


static gceSTATUS
_Alloc(
    IN gckALLOCATOR Allocator,
    INOUT PVX_MDL Mdl,
    IN gctSIZE_T NumPages,
    IN gctUINT32 Flags
    )
{
    gceSTATUS status;

    struct mdl_priv *mdlPriv = gcvNULL;
    gckOS os = Allocator->os;

    gcmkHEADER_ARG("Mdl=%p NumPages=0x%x Flags=0x%x", Mdl, NumPages, Flags);

    gcmkONERROR(gckOS_Allocate(os, sizeof(struct mdl_priv), (gctPOINTER *) &mdlPriv));

    mdlPriv->kvaddr = valloc(NumPages * PAGE_SIZE);
    if (mdlPriv->kvaddr == gcvNULL)
    {
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    mdlPriv->phys = KM_TO_PHYS(mdlPriv->kvaddr);

    Mdl->priv = mdlPriv;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (mdlPriv)
    {
        gckOS_Free(os, mdlPriv);
    }

    gcmkFOOTER();
    return status;
}

static void
_Free(
    IN gckALLOCATOR Allocator,
    IN OUT PVX_MDL Mdl
    )
{
    gckOS os = Allocator->os;
    struct mdl_priv *mdlPriv = (struct mdl_priv *)Mdl->priv;

    free(mdlPriv->kvaddr);

    gckOS_Free(os, mdlPriv);
}

static gctINT
_MapUser(
    gckALLOCATOR Allocator,
    PVX_MDL Mdl,
    IN PVX_MDL_MAP MdlMap,
    gctBOOL Cacheable
    )
{
    struct mdl_priv *mdlPriv = (struct mdl_priv *)Mdl->priv;

    MdlMap->vmaAddr = mdlPriv->kvaddr;

    return gcvSTATUS_OK;
}

static void
_UnmapUser(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN PVX_MDL_MAP MdlMap,
    IN gctUINT32 Size
    )
{
    return;
}

static gceSTATUS
_MapKernel(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    OUT gctPOINTER *Logical
    )
{
    struct mdl_priv *mdlPriv = (struct mdl_priv *)Mdl->priv;
    *Logical = mdlPriv->kvaddr;
    return gcvSTATUS_OK;
}

static gceSTATUS
_UnmapKernel(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN gctPOINTER Logical
    )
{
    return gcvSTATUS_OK;
}

static gceSTATUS
_Physical(
    IN gckALLOCATOR Allocator,
    IN PVX_MDL Mdl,
    IN gctUINT32 Offset,
    OUT gctPHYS_ADDR_T * Physical
    )
{
    struct mdl_priv *mdlPriv=(struct mdl_priv *)Mdl->priv;

    *Physical = mdlPriv->phys + Offset;

    return gcvSTATUS_OK;
}

static void
_AllocatorDestructor(
    gcsALLOCATOR *Allocator
    )
{
    if (Allocator->privateData)
    {
        free(Allocator->privateData);
    }

    free(Allocator);
}

/* Default allocator operations. */
gcsALLOCATOR_OPERATIONS allocatorOperations = {
    .Alloc              = _Alloc,
    .Free               = _Free,
    .MapUser            = _MapUser,
    .UnmapUser          = _UnmapUser,
    .MapKernel          = _MapKernel,
    .UnmapKernel        = _UnmapKernel,
    .Physical           = _Physical,
};

/* Default allocator entry. */
gceSTATUS
_AllocatorInit(
    IN gckOS Os,
    OUT gckALLOCATOR * Allocator
    )
{
    gceSTATUS status;
    gckALLOCATOR allocator = gcvNULL;

    gcmkONERROR(gckALLOCATOR_Construct(Os, &allocatorOperations, &allocator));

    /* Register private data. */
    allocator->destructor  = _AllocatorDestructor;

    allocator->capability = gcvALLOC_FLAG_CONTIGUOUS
                          | gcvALLOC_FLAG_NON_CONTIGUOUS
                          | gcvALLOC_FLAG_CACHEABLE
                          | gcvALLOC_FLAG_MEMLIMIT
                          | gcvALLOC_FLAG_ALLOC_ON_FAULT
                          ;


    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    if (allocator)
    {
        free(allocator);
    }
    return status;
}

