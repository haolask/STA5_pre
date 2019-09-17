/* GStreamer V4L2 video transformer/filter plugin
 *
 * Copyright (C) 2015 STMicroelectronics SA
 *
 * Author: Fabien Dessenne <fabien.dessenne@st.com> for STMicroelectronics.
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

#ifndef __GST_V4L2_OUT_BUF_POOL_H__
#define __GST_V4L2_OUT_BUF_POOL_H__

#include <gst/gst.h>
#include "gst/allocators/gstdmabuf.h"

typedef struct _GstV4L2OutBufPool GstV4L2OutBufPool;
typedef struct _GstV4L2OutBufPoolClass GstV4L2OutBufPoolClass;
typedef struct _GstV4L2TransMeta GstV4L2TransMeta;

#include "gstv4l2trans.h"

G_BEGIN_DECLS
#define GST_TYPE_V4L2_OUT_BUF_POOL (gst_v4l2_out_buf_pool_get_type())
#define GST_IS_V4L2_OUT_BUF_POOL(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_V4L2_OUT_BUF_POOL))
#define GST_V4L2_OUT_BUF_POOL(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_V4L2_OUT_BUF_POOL, \
    GstV4L2OutBufPool))
#define GST_V4L2_OUT_BUF_POOL_CAST(obj) ((GstV4L2OutBufPool*)(obj))

struct _GstV4L2OutBufPool
{
  GstBufferPool parent;
  GstV4L2Trans *trans;
  GstAllocator *allocator;
  gint v4l2_fd;
  guint num_buffers;            /* nb of total buffers in pool */
  guint num_allocated;          /* nb of buffers allocated */
  GstVideoInfo info;
};

struct _GstV4L2OutBufPoolClass
{
  GstBufferPoolClass parent_class;
};

struct _GstV4L2TransMeta
{
  GstMeta meta;
  gpointer mem;
  struct v4l2_buffer vbuffer;   /* v4l2 buffer */
  gboolean v4l2_dequeued;       /* is this buffer v4l2-dequeued or not */
};

GType gst_v4l2trans_meta_api_get_type (void);
const GstMetaInfo *gst_v4l2trans_meta_get_info (void);
#define GST_V4L2TRANS_META_GET(buf) \
    ((GstV4L2TransMeta *) gst_buffer_get_meta (buf, \
    gst_v4l2trans_meta_api_get_type()))
#define GST_V4L2TRANS_META_ADD(buf) \
    ((GstV4L2TransMeta *) gst_buffer_add_meta (buf, \
    gst_v4l2trans_meta_get_info(),NULL))

GType gst_v4l2_out_buf_pool_get_type (void);
GstBufferPool *gst_v4l2_out_buf_pool_new (GstV4L2Trans * trans);
GstFlowReturn gst_v4l2_out_buf_pool_process (GstBufferPool * bpool,
    GstBuffer * buf);

G_END_DECLS
#endif /*__GST_V4L2_OUT_BUF_POOL_H__ */
