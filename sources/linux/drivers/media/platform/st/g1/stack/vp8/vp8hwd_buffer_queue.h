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


#ifndef VP8HWD_PICTURE_BUFFER_QUEUE_H_
#define VP8HWD_PICTURE_BUFFER_QUEUE_H_

#include "basetype.h"
#include "dwl.h"

/* BufferQueue is picture queue indexing module, which manages the
 * buffer references on client's behalf. Module will maintain an index of
 * available and used buffers. When free  buffer is not available module
 * will block the thread asking for a buffer index. */

typedef void* BufferQueue;  /* Opaque instance variable. */

/* Special value for index to tell it is unset. */
#define REFERENCE_NOT_SET 0xFFFFFFFF

/* Flags to define previous, golden and alternate frames to be used with
 * bitfields. */
#define BQUEUE_FLAG_PREV   0x01
#define BQUEUE_FLAG_GOLDEN 0x02
#define BQUEUE_FLAG_ALT    0x04

/* Functions to initialize and release the BufferQueue. */
BufferQueue VP8HwdBufferQueueInitialize(i32 nBuffers);
void VP8HwdBufferQueueRelease(BufferQueue queue);

/* Functions to manage the reference picture state. These are to be called
 * only from the decoding thread to get and manipulate the current decoder
 * reference buffer status. When a reference is pointing to a specific buffer
 * BufferQueue will automatically increment the reference counter to the given
 * buffer and decrement the reference counter to the previous reference buffer.
 */
i32 VP8HwdBufferQueueGetAltRef(BufferQueue queue);
i32 VP8HwdBufferQueueGetGoldenRef(BufferQueue queue);
i32 VP8HwdBufferQueueGetPrevRef(BufferQueue queue);
void VP8HwdBufferQueueUpdateRef(BufferQueue queue, u32 refFlags, i32 buffer);

/* Functions to manage references to the picture buffers. Caller is responsible
 * for calling AddRef when somebody will be using the given buffer and
 * RemoveRef each time somebody stops using a given buffer. When reference count
 * reaches 0, buffer is automatically added to the pool of free buffers. */
void VP8HwdBufferQueueAddRef(BufferQueue queue, i32 buffer);
void VP8HwdBufferQueueRemoveRef(BufferQueue queue, i32 buffer);

/* Function to get free buffers from the queue. Blocks until the requested
 * buffer is available. */
i32 VP8HwdBufferQueueGetBuffer(BufferQueue queue); /* Automatic +1 ref. */

/* Function to wait until all buffers are in available status. */
void VP8HwdBufferQueueWaitPending(BufferQueue queue);

#endif  /* VP8HWD_PICTURE_BUFFER_QUEUE_H_ */

