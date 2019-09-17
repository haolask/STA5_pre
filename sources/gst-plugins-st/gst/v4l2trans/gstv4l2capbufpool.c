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
/*
 * Bufferpool using DMABUF allocator and V4L2 capture
 * The buffers of this pool are shared between v4l2trans and the downstream
 * element.
 * Buffers are allocated by V4L2 as MMAP buffers, then are DMABUF exported
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <string.h>

#include "gstv4l2capbufpool.h"

GST_DEBUG_CATEGORY_EXTERN (gst_v4l2trans_debug);
#define GST_CAT_DEFAULT gst_v4l2trans_debug

#define gst_v4l2_cap_buf_pool_parent_class parent_class
G_DEFINE_TYPE (GstV4L2CapBufPool, gst_v4l2_cap_buf_pool, GST_TYPE_BUFFER_POOL);

static GstFlowReturn
gst_v4l2_cap_buf_pool_alloc_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  GstV4L2CapBufPool *pool = GST_V4L2_CAP_BUF_POOL (bpool);
  GstBuffer *newbuf;
  GstStructure *config;
  GstAllocator *allocator;
  GstAllocationParams allocationparams;
  struct v4l2_buffer qbuf;
  struct v4l2_exportbuffer expbuf;
  guint index;

  newbuf = gst_buffer_new ();
  index = pool->num_allocated;

  GST_DEBUG_OBJECT (pool, "Creating buffer %u %p for pool %p", index, newbuf,
      pool);

  memset (&qbuf, 0, sizeof qbuf);
  qbuf.index = index;
  qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  qbuf.memory = V4L2_MEMORY_MMAP;

  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_QUERYBUF, &qbuf) < 0)
    goto querybuf_failed;

  memset (&expbuf, 0, sizeof expbuf);
  expbuf.type = qbuf.type;
  expbuf.index = qbuf.index;
  expbuf.flags = O_CLOEXEC | O_RDWR;
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_EXPBUF, &expbuf) < 0)
    goto expbuf_failed;

  config = gst_buffer_pool_get_config (bpool);
  gst_buffer_pool_config_get_allocator (config, &allocator, &allocationparams);
  gst_structure_free (config);

  gst_buffer_append_memory (newbuf,
      gst_dmabuf_allocator_alloc (allocator, expbuf.fd, qbuf.length));

  GST_LOG_OBJECT (pool, "--index=%u  type=%d  bytesused=%u  flags=%08x"
      "  field=%d  memory=%d  MMAP offset=%u  fd=%d",
      qbuf.index, qbuf.type, qbuf.bytesused, qbuf.flags,
      qbuf.field, qbuf.memory, qbuf.m.offset, expbuf.fd);

  pool->num_allocated++;

  *buffer = newbuf;

  return GST_FLOW_OK;

  /* ERRORS */
querybuf_failed:
  {
    GST_WARNING ("Failed QUERYBUF");
    gst_buffer_unref (newbuf);
    return GST_FLOW_ERROR;
  }
expbuf_failed:
  {
    GST_WARNING ("Failed EXPBUF");
    gst_buffer_unref (newbuf);
    return GST_FLOW_ERROR;
  }
}

static void
gst_v4l2_cap_buf_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buf)
{
  GstMemory *gmem = gst_buffer_get_memory (buf, 0);
  gint fd = gst_dmabuf_memory_get_fd (gmem);

  close (fd);

  gst_memory_unref (gmem);
}

static void
gst_v4l2_cap_buf_pool_finalize (GObject * object)
{
  GstV4L2CapBufPool *pool = GST_V4L2_CAP_BUF_POOL (object);

  if (pool->v4l2_fd > 0)
    v4l2_close (pool->v4l2_fd);

  gst_object_unref (pool->trans);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_v4l2_cap_buf_pool_init (GstV4L2CapBufPool * pool)
{
}

static void
gst_v4l2_cap_buf_pool_class_init (GstV4L2CapBufPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_v4l2_cap_buf_pool_finalize;

  bufferpool_class->alloc_buffer = gst_v4l2_cap_buf_pool_alloc_buffer;
  bufferpool_class->free_buffer = gst_v4l2_cap_buf_pool_free_buffer;
}

GstBufferPool *
gst_v4l2_cap_buf_pool_new (GstV4L2Trans * trans, GstCaps * caps, gsize size,
    guint max)
{
  GstStructure *config;
  GstV4L2CapBufPool *pool;
  GstAllocator *allocator;
  gint v4l2_fd;

  GST_DEBUG_OBJECT (trans, "Creating capture bufferpool");

  v4l2_fd = v4l2_dup (trans->fd);
  if (v4l2_fd < 0) {
    GST_DEBUG ("Failed to dup fd");
    return NULL;
  }

  pool = (GstV4L2CapBufPool *) g_object_new (GST_TYPE_V4L2_CAP_BUF_POOL, NULL);
  allocator = gst_dmabuf_allocator_new ();
  config = gst_buffer_pool_get_config (GST_BUFFER_POOL_CAST (pool));

  gst_buffer_pool_config_set_params (config, caps, size, max, max);
  gst_buffer_pool_config_set_allocator (config, allocator, NULL);

  gst_object_unref (allocator);

  if (!gst_buffer_pool_set_config (GST_BUFFER_POOL_CAST (pool), config))
    goto config_failed;

  /* take a reference on v4l2trans to be sure it is released after the pool */
  pool->trans = gst_object_ref (trans);
  pool->num_allocated = 0;
  pool->v4l2_fd = v4l2_fd;

  return GST_BUFFER_POOL (pool);

config_failed:
  GST_ERROR_OBJECT (pool, "Failed setting config");
  gst_object_unref (pool);
  return NULL;
}
