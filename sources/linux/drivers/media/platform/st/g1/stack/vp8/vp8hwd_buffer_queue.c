/*
 * (C) COPYRIGHT 2012 HANTRO PRODUCTS
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


#include "vp8hwd_buffer_queue.h"

#include "dwlthread.h"

#include "fifo.h"

/* macro for assertion, used only when _ASSERT_USED is defined */
#ifdef _ASSERT_USED
#ifndef ASSERT
#include <assert.h>
#define ASSERT(expr) assert(expr)
#endif
#else
#define ASSERT(expr)
#endif

#ifdef BUFFER_QUEUE_PRINT_STATUS
#include <stdio.h>
#define PRINT_COUNTS(x) PrintCounts(x)
#else
#define PRINT_COUNTS(x)
#endif /* BUFFER_QUEUE_PRINT_STATUS */

/* Data structure containing the decoder reference frame status at the tail
 * of the queue. */
typedef struct DecoderRefStatus_ {
    i32 iPrev;    /* Index to the previous reference frame. */
    i32 iAlt;     /* Index to the alt reference frame. */
    i32 iGolden;  /* Index to the golden reference frame. */
} DecoderRefStatus;

/* Data structure to hold this picture buffer queue instance data. */
typedef struct BufferQueue_t_
{
    pthread_mutex_t cs;         /* Critical section to protect data. */
    pthread_cond_t pending_cv;/* Sync for DecreaseRefCount and WaitPending. */
    pthread_mutex_t pending;    /* Sync for DecreaseRefCount and WaitPending. */
    i32 nBuffers;         /* Number of buffers contained in total. */
    i32* nReferences;     /* Reference counts on buffers. Index is buffer#.  */
    DecoderRefStatus refStatus; /* Reference status of the decoder. */
    fifo_inst emptyFifo;  /* Queue holding empty, unreferred buffer indices. */
} BufferQueue_t;

static void IncreaseRefCount(BufferQueue_t* q, i32 i);
static void DecreaseRefCount(BufferQueue_t* q, i32 i);
#ifdef BUFFER_QUEUE_PRINT_STATUS
static inline void PrintCounts(BufferQueue_t* q);
#endif

BufferQueue VP8HwdBufferQueueInitialize(i32 nBuffers)
{
    i32 i;
    BufferQueue_t* q;
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("(nBuffers=%i)", nBuffers);
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    ASSERT(nBuffers > 0);
    q = (BufferQueue_t*)DWLcalloc(1, sizeof(BufferQueue_t));
    if (q == NULL)
    {
        return NULL;
    }
    q->nReferences = (i32*)DWLcalloc(nBuffers, sizeof(i32));
    if (q->nReferences == NULL ||
        fifo_init(nBuffers, &q->emptyFifo) != FIFO_OK ||
        pthread_mutex_init(&q->cs, NULL) ||
        pthread_mutex_init(&q->pending, NULL) ||
        pthread_cond_init(&q->pending_cv, NULL))
    {
        VP8HwdBufferQueueRelease(q);
        return NULL;
    }
    /* Add picture buffers among empty picture buffers. */
    for (i = 0; i < nBuffers; i++)
    {
        q->nReferences[i] = 0;
        fifo_push(q->emptyFifo, i);
        q->nBuffers++;
    }
    q->refStatus.iPrev = q->refStatus.iGolden = q->refStatus.iAlt =
        REFERENCE_NOT_SET;
    return q;
}

void VP8HwdBufferQueueRelease(BufferQueue queue)
{
    BufferQueue_t* q = (BufferQueue_t*)queue;
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("()");
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    ASSERT(queue);
    if (q->emptyFifo)
    {   /* Empty the fifo before releasing. */
        i32 i, j;
        for (i = 0; i < q->nBuffers; i++)
            fifo_pop(q->emptyFifo, &j);
        fifo_release(q->emptyFifo);
    }
    pthread_mutex_destroy(&q->cs);
    pthread_cond_destroy(&q->pending_cv);
    pthread_mutex_destroy(&q->pending);
    DWLfree(q->nReferences);
    DWLfree(q);
}

void VP8HwdBufferQueueUpdateRef(BufferQueue queue, u32 refFlags, i32 buffer)
{
    BufferQueue_t* q = (BufferQueue_t*)queue;
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("(refFlags=0x%X, buffer=%i)", refFlags, buffer);
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    ASSERT(queue);
    ASSERT(buffer >= 0 && buffer < q->nBuffers);
    pthread_mutex_lock(&q->cs);
    /* Check for each type of reference frame update need. */
    if (refFlags & BQUEUE_FLAG_PREV && buffer != q->refStatus.iPrev)
    {
        q->refStatus.iPrev = buffer;
    }
    if (refFlags & BQUEUE_FLAG_GOLDEN && buffer != q->refStatus.iGolden)
    {
        q->refStatus.iGolden = buffer;
    }
    if (refFlags & BQUEUE_FLAG_ALT && buffer != q->refStatus.iAlt)
    {
        q->refStatus.iAlt = buffer;
    }
    PRINT_COUNTS(q);
    pthread_mutex_unlock(&q->cs);
}

i32 VP8HwdBufferQueueGetPrevRef(BufferQueue queue)
{
    BufferQueue_t* q = (BufferQueue_t*)queue;
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("()");
    printf(" # %d\n", q->refStatus.iPrev);
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    return q->refStatus.iPrev;
}

i32 VP8HwdBufferQueueGetGoldenRef(BufferQueue queue)
{
    BufferQueue_t* q = (BufferQueue_t*)queue;
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("()");
    printf(" # %d\n", q->refStatus.iGolden);
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    return q->refStatus.iGolden;
}

i32 VP8HwdBufferQueueGetAltRef(BufferQueue queue)
{
    BufferQueue_t* q = (BufferQueue_t*)queue;
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("()");
    printf(" # %d\n", q->refStatus.iAlt);
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    return q->refStatus.iAlt;
}

void VP8HwdBufferQueueAddRef(BufferQueue queue, i32 buffer)
{
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("(buffer=%i)", buffer);
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    BufferQueue_t* q = (BufferQueue_t*)queue;
    ASSERT(buffer >= 0 && buffer < q->nBuffers);
    pthread_mutex_lock(&q->cs);
    IncreaseRefCount(q, buffer);
    pthread_mutex_unlock(&q->cs);
}

void VP8HwdBufferQueueRemoveRef(BufferQueue queue,
                                i32 buffer)
{
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("(buffer=%i)", buffer);
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    BufferQueue_t* q = (BufferQueue_t*)queue;
    ASSERT(buffer >= 0 && buffer < q->nBuffers);
    pthread_mutex_lock(&q->cs);
    DecreaseRefCount(q, buffer);
    pthread_mutex_unlock(&q->cs);
}

i32 VP8HwdBufferQueueGetBuffer(BufferQueue queue)
{
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("()");
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    i32 i;
    BufferQueue_t* q = (BufferQueue_t*)queue;
    fifo_pop(q->emptyFifo, &i);
    pthread_mutex_lock(&q->cs);
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("# %i\n", i);
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    IncreaseRefCount(q, i);
    pthread_mutex_unlock(&q->cs);
    return i;
}

void VP8HwdBufferQueueWaitPending(BufferQueue queue)
{
    BufferQueue_t* q = (BufferQueue_t*)queue;
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("()");
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    ASSERT(queue);

    pthread_mutex_lock(&q->pending);

    while (fifo_count(q->emptyFifo) != (u32)q->nBuffers)
        pthread_cond_wait(&q->pending_cv, &q->pending);

    pthread_mutex_unlock(&q->pending);

#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("#\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
}

static void IncreaseRefCount(BufferQueue_t* q, i32 i)
{
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("(buffer=%i)", i);
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    q->nReferences[i]++;
    ASSERT(q->nReferences[i] >= 0);   /* No negative references. */
    PRINT_COUNTS(q);
}

static void DecreaseRefCount(BufferQueue_t* q, i32 i)
{
#ifdef BUFFER_QUEUE_PRINT_STATUS
    printf(__FUNCTION__);
    printf("(buffer=%i)", i);
    printf("\n");
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
    q->nReferences[i]--;
    ASSERT(q->nReferences[i] >= 0);   /* No negative references. */
    PRINT_COUNTS(q);
    if (q->nReferences[i] == 0)
    {   /* Once picture buffer is no longer referred to, it can be put to
           the empty fifo. */
#ifdef BUFFER_QUEUE_PRINT_STATUS
        printf("Buffer #%i put to empty pool\n", i);
        if(i == q->refStatus.iPrev || i == q->refStatus.iGolden || i == q->refStatus.iAlt)
        {
          printf("released but referenced %d\n", i);
        }
#endif  /* BUFFER_QUEUE_PRINT_STATUS */
        fifo_push(q->emptyFifo, i);

        pthread_mutex_lock(&q->pending);
        if (fifo_count(q->emptyFifo) == (u32)q->nBuffers)
            pthread_cond_signal(&q->pending_cv);
        pthread_mutex_unlock(&q->pending);
    }
}

#ifdef BUFFER_QUEUE_PRINT_STATUS
static inline void PrintCounts(BufferQueue_t* q)
{
    i32 i = 0;
    for (i = 0; i < q->nBuffers; i++)
        printf("%u", q->nReferences[i]);
    printf(" |");
    printf(" P: %i |", q->refStatus.iPrev);
    printf(" G: %i |", q->refStatus.iGolden);
    printf(" A: %i |", q->refStatus.iAlt);
    printf("\n");
}
#endif /* BUFFER_QUEUE_PRINT_STATUS */

