/* GStreamer V4L2 video encoder plugin
 *
 * Copyright (C) 2015 STMicroelectronics SA
 *
 * Authors:
 *   Yannick Fertre <yannick.fertre@st.com> for STMicroelectronics
 *   Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_V4L2ENC_BUFFER_POOL_H__
#define __GST_V4L2ENC_BUFFER_POOL_H__

#include <gst/gst.h>
#include "gst/allocators/gstdmabuf.h"

typedef struct _GstV4L2EncBufferPool GstV4L2EncBufferPool;
typedef struct _GstV4L2EncBufferPoolClass GstV4L2EncBufferPoolClass;
typedef struct _GstV4L2EncMeta GstV4L2EncMeta;

#include "gstv4l2enc.h"

G_BEGIN_DECLS
#define GST_TYPE_V4L2ENC_BUFFER_POOL      (gst_v4l2enc_buffer_pool_get_type())
#define GST_IS_V4L2ENC_BUFFER_POOL(obj)   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_V4L2ENC_BUFFER_POOL))
#define GST_V4L2ENC_BUFFER_POOL(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_V4L2ENC_BUFFER_POOL, GstV4L2EncBufferPool))
#define GST_V4L2ENC_BUFFER_POOL_CAST(obj) ((GstV4L2EncBufferPool*)(obj))
    struct _GstV4L2EncBufferPool
{
  GstBufferPool parent;
  GstV4L2Enc *enc;
  GstAllocator *allocator;
  GstAllocationParams params;

  gint video_fd;
  guint num_buffers;            /* number of total buffers in pool */
  guint num_buffers_available;  /* number of buffers available (ie some are reserved for driver) */
  guint num_allocated;          /* number of buffers allocated by the driver */
};

struct _GstV4L2EncBufferPoolClass
{
  GstBufferPoolClass parent_class;
};

struct _GstV4L2EncMeta
{
  GstMeta meta;
  gpointer mem;
  struct v4l2_buffer vbuffer;
  gboolean v4l2_dequeued;       /* is this buffer v4l2-dequeued or not */
};

GType gst_v4l2enc_meta_api_get_type (void);
const GstMetaInfo *gst_v4l2enc_meta_get_info (void);
#define GST_V4L2ENC_META_GET(buf) ((GstV4L2EncMeta *)gst_buffer_get_meta(buf,gst_v4l2enc_meta_api_get_type()))
#define GST_V4L2ENC_META_ADD(buf) ((GstV4L2EncMeta *)gst_buffer_add_meta(buf,gst_v4l2enc_meta_get_info(),NULL))

GType gst_v4l2enc_buffer_pool_get_type (void);

GstBufferPool *gst_v4l2enc_buffer_pool_new (GstV4L2Enc * enc);
GstFlowReturn gst_v4l2enc_buffer_pool_process (GstBufferPool * bpool,
    GstBuffer * buf);

G_END_DECLS
#endif /*__GST_V4L2ENC_BUFFER_POOL_H__ */
