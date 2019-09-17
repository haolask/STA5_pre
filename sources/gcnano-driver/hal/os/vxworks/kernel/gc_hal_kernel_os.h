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


#ifndef __gc_hal_kernel_os_h_
#define __gc_hal_kernel_os_h_

/*
 * This is a simple doubly linked list implementation.
 */

struct list_head {
    struct list_head *next; /* next in chain */
    struct list_head *prev; /* previous in chain */
};


/* Initialise a static list */
#define LIST_HEAD(name) \
struct list_head name = { &(name), &(name)}



/* Initialise a list head to an empty list */
#define INIT_LIST_HEAD(p) \
do { \
    (p)->next = (p);\
    (p)->prev = (p); \
} while (0)

static gcmINLINE
void list_add(struct list_head *new_entry,
                struct list_head *list)
{
    struct list_head *list_next = list->next;

    list->next = new_entry;
    new_entry->prev = list;
    new_entry->next = list_next;
    list_next->prev = new_entry;
}

static gcmINLINE
void list_add_tail(struct list_head *new_entry,
                struct list_head *list)
{
    struct list_head *list_prev = list->prev;

    list->prev = new_entry;
    new_entry->next = list;
    new_entry->prev = list_prev;
    list_prev->next = new_entry;
}


static gcmINLINE
void list_del(struct list_head *entry)
{
    struct list_head *list_next = entry->next;
    struct list_head *list_prev = entry->prev;

    list_next->prev = list_prev;
    list_prev->next = list_next;
}

static gcmINLINE
void list_del_init(struct list_head *entry)
{
    list_del(entry);
    entry->next = entry->prev = entry;
}


static gcmINLINE
int list_empty(struct list_head *entry)
{
    return (entry->next == entry);
}

#define list_entry(entry, type, member) \
    ((type *)((char *)(entry)-(unsigned long)(&((type *)NULL)->member)))

#define list_for_each(itervar, list) \
    for (itervar = (list)->next; itervar != (list); itervar = itervar->next)

#define list_for_each_entry(pos, head, member)                \
    for (pos = list_entry((head)->next, typeof(*pos), member);    \
         &pos->member != (head);     \
         pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_safe(itervar, save_var, list) \
    for (itervar = (list)->next, save_var = (list)->next->next; \
         itervar != (list); \
         itervar = save_var, save_var = save_var->next)

#define list_for_each_entry_safe(pos, n, head, member)            \
    for (pos = list_entry((head)->next, typeof(*pos), member),    \
         n = list_entry(pos->member.next, typeof(*pos), member);    \
         &pos->member != (head);                     \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))


typedef struct _VX_MDL     VX_MDL,     *PVX_MDL;
typedef struct _VX_MDL_MAP VX_MDL_MAP, *PVX_MDL_MAP;

struct _VX_MDL_MAP
{
    gctINT                  pid;

    /* map references. */
    gctUINT32               count;

    struct vm_area_struct * vma;
    gctPOINTER              vmaAddr;
    gctBOOL                 cacheable;

    struct list_head        link;
};

struct _VX_MDL
{
    gckOS                   os;

    atomic_t                refs;

    char *                  addr;

    gctINT                  numPages;
    gctBOOL                 contiguous;

    pthread_mutex_t         mapsMutex;
    struct list_head        mapsHead;

    /* Pointer to allocator which allocates memory for this mdl. */
    void *                  allocator;

    /* Private data used by allocator. */
    void *                  priv;

    gctUINT32               gid;

    struct list_head        link;
};

extern PVX_MDL_MAP
FindMdlMap(
    IN PVX_MDL Mdl,
    IN gctINT PID
    );

typedef struct _DRIVER_ARGS
{
    gctUINT64               InputBuffer;
    gctUINT64               InputBufferSize;
    gctUINT64               OutputBuffer;
    gctUINT64               OutputBufferSize;
}
DRIVER_ARGS;

#endif /* __gc_hal_kernel_os_h_ */
