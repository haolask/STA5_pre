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

#ifndef __GST_V4L2_CAP_BUF_POOL_H__
#define __GST_V4L2_CAP_BUF_POOL_H__

#include <gst/gst.h>
#include <gst/allocators/gstdmabuf.h>

typedef struct _GstV4L2CapBufPool GstV4L2CapBufPool;
typedef struct _GstV4L2CapBufPoolClass GstV4L2CapBufPoolClass;

#include "gstv4l2trans.h"

G_BEGIN_DECLS
#define GST_TYPE_V4L2_CAP_BUF_POOL     (gst_v4l2_cap_buf_pool_get_type())
#define GST_IS_V4L2_CAP_BUF_POOL(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GST_TYPE_V4L2_CAP_BUF_POOL))
#define GST_V4L2_CAP_BUF_POOL(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), GST_TYPE_V4L2_CAP_BUF_POOL, \
    GstV4L2CapBufPool))
#define GST_V4L2_CAP_BUF_POOL_CAST(obj) ((GstV4L2CapBufPool*)(obj))

struct _GstV4L2CapBufPool
{
  GstBufferPool parent;
  GstV4L2Trans *trans;
  gint v4l2_fd;
  guint num_allocated;
};

struct _GstV4L2CapBufPoolClass
{
  GstBufferPoolClass parent_class;
};

GType gst_v4l2_cap_buf_pool_get_type (void);
GstBufferPool *gst_v4l2_cap_buf_pool_new (GstV4L2Trans * trans,
    GstCaps * caps, gsize size, guint max);
G_END_DECLS
#endif /*__GST_V4L2_CAP_BUF_POOL_H__ */
