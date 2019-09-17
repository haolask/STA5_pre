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
--  Abstract : Stream decoding utilities
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of context

     1. Include headers
     2. External identifiers
     3. Module defines
     4. Module identifiers
     5. Fuctions

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/

#include "bqueue.h"
#include "dwl.h"
#ifndef HANTRO_OK
    #define HANTRO_OK   (0)
#endif /* HANTRO_TRUE */

#ifndef HANTRO_NOK
    #define HANTRO_NOK  (1)
#endif /* HANTRO_FALSE*/

/*------------------------------------------------------------------------------
    2. External identifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Module indentifiers
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    BqueueInit
        Initialize buffer queue
------------------------------------------------------------------------------*/
u32 BqueueInit( bufferQueue_t *bq, u32 numBuffers )
{
    u32 i;

    if(DWLmemset(bq, 0, sizeof(*bq)) != bq)
        return HANTRO_NOK;

    if( numBuffers == 0 )
        return HANTRO_OK;

    bq->picI = (u32*)DWLmalloc( sizeof(u32)*numBuffers);
    if( bq->picI == NULL )
    {
        return HANTRO_NOK;
    }
    for( i = 0 ; i < numBuffers ; ++i )
    {
        bq->picI[i] = 0;
    }
    bq->queueSize = numBuffers;
    bq->ctr = 1;

    return HANTRO_OK;
}

/*------------------------------------------------------------------------------
    BqueueRelease
------------------------------------------------------------------------------*/
void BqueueRelease( bufferQueue_t *bq )
{
    if(bq->picI)
    {
        DWLfree(bq->picI);
        bq->picI = NULL;
    }
    bq->prevAnchorSlot  = 0;
    bq->queueSize       = 0;
}

/*------------------------------------------------------------------------------
    BqueueNext
        Return "oldest" available buffer.
------------------------------------------------------------------------------*/
u32 BqueueNext( bufferQueue_t *bq, u32 ref0, u32 ref1, u32 ref2, u32 bPic )
{
    u32 minPicI = 1<<30;
    u32 nextOut = (u32)0xFFFFFFFFU;
    u32 i;
    /* Find available buffer with smallest index number  */
    i = 0;

    while( i < bq->queueSize )
    {
        if(i == ref0 || i == ref1 || i == ref2) /* Skip reserved anchor pictures */
        {
            i++;
            continue;
        }
        if( bq->picI[i] < minPicI )
        {
            minPicI = bq->picI[i];
            nextOut = i;
        }
        i++;
    }

    if( nextOut == (u32)0xFFFFFFFFU)
    {
        return 0; /* No buffers available, shouldn't happen */
    }

    /* Update queue state */
    if( bPic )
    {
        bq->picI[nextOut] = bq->ctr-1;
        bq->picI[bq->prevAnchorSlot]++;
    }
    else
    {
        bq->picI[nextOut] = bq->ctr;
    }
    bq->ctr++;
    if( !bPic )
    {
        bq->prevAnchorSlot = nextOut;
    }

    return nextOut;
}

/*------------------------------------------------------------------------------
    BqueueDiscard
        "Discard" output buffer, e.g. if error concealment used and buffer
        at issue is never going out.
------------------------------------------------------------------------------*/
void BqueueDiscard( bufferQueue_t *bq, u32 buffer )
{
    bq->picI[buffer] = 0;
}
