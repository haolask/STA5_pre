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
 * Bufferpool using DMABUF allocator and V4L2 output
 * The buffers of this pool are shared between v4l2trans and the upstream element
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <string.h>

#include "gstv4l2outbufpool.h"

GST_DEBUG_CATEGORY_EXTERN (gst_v4l2trans_debug);
#define GST_CAT_DEFAULT gst_v4l2trans_debug

GType
gst_v4l2trans_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { "memory", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("GstV4L2TransMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

static gboolean
gst_v4l2trans_meta_init (GstMeta * meta,
    G_GNUC_UNUSED gpointer params, G_GNUC_UNUSED GstBuffer * buffer)
{
  /* Just to avoid a warning */
  return TRUE;
}

const GstMetaInfo *
gst_v4l2trans_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_v4l2trans_meta_api_get_type (),
        "GstV4L2TransMeta", sizeof (GstV4L2TransMeta),
        (GstMetaInitFunction) gst_v4l2trans_meta_init,
        (GstMetaFreeFunction) NULL,
        (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

#define gst_v4l2_out_buf_pool_parent_class parent_class
G_DEFINE_TYPE (GstV4L2OutBufPool, gst_v4l2_out_buf_pool, GST_TYPE_BUFFER_POOL);

static const gchar **
gst_v4l2_out_buf_pool_get_options (GstBufferPool * pool)
{
  static const gchar *options[] = { GST_BUFFER_POOL_OPTION_VIDEO_META,
    GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT, NULL
  };

  return options;
}

static GstFlowReturn
gst_v4l2_out_buf_pool_alloc_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  GstV4L2OutBufPool *pool = GST_V4L2_OUT_BUF_POOL (bpool);
  GstBuffer *newbuf;
  GstV4L2TransMeta *meta;
  guint index, size, min_buffers, max_buffers;
  struct v4l2_exportbuffer expbuf;
  GstVideoInfo *info = &pool->info;
  GstStructure *config;
  GstCaps *caps;

  newbuf = gst_buffer_new ();
  meta = GST_V4L2TRANS_META_ADD (newbuf);

  index = pool->num_allocated;

  GST_DEBUG_OBJECT (pool, "Creating buffer %u, %p for pool %p", index, newbuf,
      pool);

  memset (&meta->vbuffer, 0, sizeof meta->vbuffer);
  meta->vbuffer.index = index;
  meta->vbuffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  meta->vbuffer.memory = V4L2_MEMORY_MMAP;

  /* Get a buffer from the driver */
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_QUERYBUF, &meta->vbuffer) < 0)
    goto querybuf_failed;

  /* Export this buffer so it can be used as a DMABUF one */
  memset (&expbuf, 0, sizeof expbuf);
  expbuf.type = meta->vbuffer.type;
  expbuf.index = meta->vbuffer.index;
  expbuf.flags = O_CLOEXEC | O_RDWR;
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_EXPBUF, &expbuf) < 0)
    goto expbuf_failed;

  GST_LOG_OBJECT (pool, "--index=%u  type=%d  bytesused=%u  flags=%08x"
      "  field=%d  memory=%d  MMAP offset=%u  fd=%d",
      meta->vbuffer.index, meta->vbuffer.type,
      meta->vbuffer.bytesused, meta->vbuffer.flags,
      meta->vbuffer.field, meta->vbuffer.memory,
      meta->vbuffer.m.offset, expbuf.fd);

  gst_buffer_append_memory (newbuf,
      gst_dmabuf_allocator_alloc (pool->allocator, expbuf.fd,
          meta->vbuffer.length));
  pool->num_allocated++;

  /* Queue this empty buffer */
  meta->vbuffer.bytesused = 0;
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_QBUF, &meta->vbuffer) < 0)
    goto queue_failed;

  meta->v4l2_dequeued = FALSE;

  GST_DEBUG_OBJECT (pool, "Queued output buffer index %d", meta->vbuffer.index);

  gst_buffer_add_video_meta_full (newbuf, GST_VIDEO_FRAME_FLAG_NONE,
      GST_VIDEO_INFO_FORMAT (info), GST_VIDEO_INFO_WIDTH (info),
      GST_VIDEO_INFO_HEIGHT (info), GST_VIDEO_INFO_N_PLANES (info),
      info->offset, info->stride);

  *buffer = newbuf;

  /* Allocation on-demand is not supported:
   * if no buffers are available in pool, pool must wait for
   * buffer release. To trig this behaviour, max limit
   * must be set in pool config after all buffers are allocated.
   */
  if (pool->num_allocated >= pool->num_buffers) {
    config = gst_buffer_pool_get_config (bpool);
    gst_buffer_pool_config_get_params (config, &caps, &size,
        &min_buffers, &max_buffers);
    gst_buffer_pool_config_set_params (config, caps, size,
        pool->num_buffers, pool->num_buffers);
  }

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
queue_failed:
  {
    GST_WARNING_OBJECT (pool, "Could not queue a buffer");
    gst_buffer_unref (newbuf);
    return GST_FLOW_ERROR;
  }
}

static gboolean
gst_v4l2_out_buf_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{
  GstV4L2OutBufPool *pool = GST_V4L2_OUT_BUF_POOL (bpool);
  GstV4L2Trans *trans = pool->trans;
  GstCaps *caps;
  GstVideoInfo info;
  GstAllocator *allocator;
  GstAllocationParams params;
  GstVideoAlignment align;
  struct v4l2_requestbuffers reqbufs;
  struct v4l2_format s_fmt;
  struct v4l2_format g_fmt;
  struct v4l2_selection selection;
  guint size, min_buf, max_buf, num_buf, aligned_width, aligned_height;
  gint ret;
  gboolean need_alignment;

  GST_DEBUG_OBJECT (pool, "Configuring %" GST_PTR_FORMAT, config);

  if (!gst_buffer_pool_config_get_params (config, &caps, &size,
          &min_buf, &max_buf))
    goto wrong_config;

  if (caps == NULL)
    goto no_caps;

  if (!gst_buffer_pool_config_get_allocator (config, &allocator, &params))
    goto wrong_config;

  /* parse the caps from the config */
  if (!gst_video_info_from_caps (&info, caps))
    goto wrong_caps;

  /* parse extra alignment info */
  need_alignment = gst_buffer_pool_config_has_option (config,
      GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);

  if (need_alignment)
    gst_buffer_pool_config_get_video_alignment (config, &align);
  else
    gst_video_alignment_reset (&align);

  /* add the padding */
  aligned_width =
      GST_VIDEO_INFO_WIDTH (&info) + align.padding_left + align.padding_right;
  aligned_height =
      GST_VIDEO_INFO_HEIGHT (&info) + align.padding_top + align.padding_bottom;
  pool->info = info;

  /* Configure V4L2 output */
  memset (&s_fmt, 0, sizeof s_fmt);
  s_fmt.fmt.pix.width = aligned_width;
  s_fmt.fmt.pix.height = aligned_height;
  s_fmt.fmt.pix.sizeimage = size;
  s_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

  if (GST_VIDEO_INFO_IS_INTERLACED (&info)) {
    GST_DEBUG_OBJECT (trans, "interlaced video");
    /* ideally we would differentiate between types of interlaced video
     * but there is not sufficient information in the caps..
     */
    s_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
  } else {
    GST_DEBUG_OBJECT (trans, "progressive video");
    s_fmt.fmt.pix.field = V4L2_FIELD_NONE;
  }

  switch (GST_VIDEO_INFO_FORMAT (&info)) {
    case GST_VIDEO_FORMAT_RGB:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
      s_fmt.fmt.pix.bytesperline = aligned_width * 3;
      break;
    case GST_VIDEO_FORMAT_NV12:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
      s_fmt.fmt.pix.bytesperline = aligned_width;
      break;
    case GST_VIDEO_FORMAT_I420:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
      s_fmt.fmt.pix.bytesperline = aligned_width;
      break;
    case GST_VIDEO_FORMAT_BGRx:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_XBGR32;
      s_fmt.fmt.pix.bytesperline = aligned_width * 4;
      break;
    case GST_VIDEO_FORMAT_BGRA:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_ABGR32;
      s_fmt.fmt.pix.bytesperline = aligned_width * 4;
      break;
    case GST_VIDEO_FORMAT_RGB16:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
      s_fmt.fmt.pix.bytesperline = aligned_width * 2;
      break;
    case GST_VIDEO_FORMAT_UYVY:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
      s_fmt.fmt.pix.bytesperline = aligned_width * 2;
      break;
    case GST_VIDEO_FORMAT_YUY2:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
      s_fmt.fmt.pix.bytesperline = aligned_width * 2;
      break;
    case GST_VIDEO_FORMAT_YVYU:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU;
      s_fmt.fmt.pix.bytesperline = aligned_width * 2;
      break;
    default:
      goto wrong_config;
      break;
  }

  /* Format */
  ret = v4l2_ioctl (pool->v4l2_fd, VIDIOC_S_FMT, &s_fmt);
  if (ret != 0)
    goto error_s_fmt;

  if (gst_v4l2trans_update_dyn_properties (trans) != GST_FLOW_OK)
    return GST_FLOW_ERROR;

  /* Crop if needed */
  if (align.padding_left || align.padding_right ||
      align.padding_top || align.padding_bottom || trans->user_crop) {
    memset (&selection, 0, sizeof selection);
    selection.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    selection.target = V4L2_SEL_TGT_CROP;
    selection.r.left = align.padding_left + trans->crop.left;
    selection.r.top = align.padding_top + trans->crop.top;

    selection.r.width = aligned_width - selection.r.left -
        (align.padding_right + trans->crop.right);
    if (selection.r.width < 0)
      selection.r.width = 0;

    selection.r.height = aligned_height - selection.r.top -
        (align.padding_bottom + trans->crop.bottom);
    if (selection.r.height < 0)
      selection.r.height = 0;

    GST_DEBUG_OBJECT (trans,
        "Configure V4L2 Set crop => Left:%d Top:%d Width:%d, Height:%d",
        selection.r.left, selection.r.top, selection.r.width,
        selection.r.height);

    ret = v4l2_ioctl (pool->v4l2_fd, VIDIOC_S_SELECTION, &selection);
    if (ret != 0)
      goto error_s_crop;
  }

  memset (&g_fmt, 0, sizeof g_fmt);
  g_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ret = v4l2_ioctl (pool->v4l2_fd, VIDIOC_G_FMT, &g_fmt);
  if (ret != 0)
    goto error_g_fmt;

  GST_DEBUG_OBJECT (trans,
      "Format g_fmt from V4L2 : fmt %.4s, width:%d, height:%d, bytesperline:%d "
      "sizeimage:%d, field:%d",
      (char *) &g_fmt.fmt.pix.pixelformat,
      g_fmt.fmt.pix.width,
      g_fmt.fmt.pix.height, g_fmt.fmt.pix.bytesperline,
      g_fmt.fmt.pix.sizeimage, g_fmt.fmt.pix.field);

  /* check that driver accepted the format parameters without changes */
  if ((s_fmt.fmt.pix.sizeimage != g_fmt.fmt.pix.sizeimage) ||
      (s_fmt.fmt.pix.bytesperline != g_fmt.fmt.pix.bytesperline)) {
    GST_WARNING_OBJECT (pool, "The driver change the format parameters");
  }

  /* Get cropping request from V4L2 driver to adapt buffer config
   * and alignment
   */
  aligned_width = g_fmt.fmt.pix.width;
  aligned_height = g_fmt.fmt.pix.height;

  gst_video_alignment_reset (&trans->align);
  memset (&selection, 0, sizeof selection);
  selection.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  selection.target = V4L2_SEL_TGT_CROP;
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_G_SELECTION, &selection) < 0) {
    GST_DEBUG_OBJECT (trans, "Not able to get output crop, default to %dx%d",
        g_fmt.fmt.pix.width, g_fmt.fmt.pix.height);
  }
  GST_DEBUG_OBJECT (trans, "Get selection returns : l:%d, t:%d, %dx%d",
      selection.r.left, selection.r.top, selection.r.width, selection.r.height);

  if (aligned_width != GST_VIDEO_INFO_WIDTH (&info)) {
    trans->align.padding_left = 0;
    trans->align.padding_right = aligned_width - GST_VIDEO_INFO_WIDTH (&info);
    need_alignment = TRUE;
  }

  if (aligned_height != GST_VIDEO_INFO_HEIGHT (&info)) {
    trans->align.padding_top = 0;
    trans->align.padding_bottom =
        aligned_height - GST_VIDEO_INFO_HEIGHT (&info);
    need_alignment = TRUE;
  }

  if (need_alignment) {
    /* add pool option alignment */
    if (!gst_buffer_pool_config_has_option (config,
            GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT))
      gst_buffer_pool_config_add_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);

    /* add pool option metadata */
    if (!gst_buffer_pool_config_has_option (config,
            GST_BUFFER_POOL_OPTION_VIDEO_META))
      gst_buffer_pool_config_add_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_META);

    /* save align to transformer info & pool config */
    gst_video_info_align (&pool->info, &trans->align);
    gst_buffer_pool_config_set_video_alignment (config, &trans->align);

    GST_LOG_OBJECT (pool, "Adapt pool size : padding %u-%ux%u-%u",
        trans->align.padding_top, trans->align.padding_left,
        trans->align.padding_right, trans->align.padding_bottom);
  }

  /* On-demand allocation is not supported, so max=min in config.
   * if max is specified, max buffers must be allocated */
  if (max_buf != 0)
    min_buf = max_buf;
  num_buf = min_buf;

  /* Request num_buf to V4L2 */
  if (pool->num_buffers) {
    /* Release existing buffers */
    memset (&reqbufs, 0, sizeof reqbufs);
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    reqbufs.count = 0;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_REQBUFS, &reqbufs) < 0)
      goto error_ioc_reqbufs;
  }

  memset (&reqbufs, 0, sizeof reqbufs);
  reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbufs.count = num_buf;
  reqbufs.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_REQBUFS, &reqbufs) < 0)
    goto error_ioc_reqbufs;

  /* V4L2 could request a size image bigger than request
   * adjust size (read size image from g_fmt) and reinject it in pool config */
  size = g_fmt.fmt.pix.sizeimage;

  gst_buffer_pool_config_set_params (config, caps, size, num_buf, num_buf);

  pool->num_buffers = num_buf;

  /* Add dmabuf allocator */
  allocator = gst_dmabuf_allocator_new ();
  if (pool->allocator)
    gst_object_unref (pool->allocator);
  if ((pool->allocator = allocator))
    gst_object_ref (allocator);

  return GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);

  /* ERRORS */
error_s_fmt:
  {
    GST_ERROR_OBJECT (pool, "Unable to set output format");
    return FALSE;
  }
error_s_crop:
  {
    GST_ERROR_OBJECT (pool, "Unable to set crop");
    return FALSE;
  }
error_g_fmt:
  {
    GST_ERROR_OBJECT (pool, "Unable to get output format");
    return FALSE;
  }
wrong_config:
  {
    GST_ERROR_OBJECT (pool, "Invalid config %" GST_PTR_FORMAT, config);
    return FALSE;
  }
no_caps:
  {
    GST_WARNING_OBJECT (pool, "No caps in config");
    return FALSE;
  }
wrong_caps:
  {
    GST_WARNING_OBJECT (pool,
        "Failed getting geometry from caps %" GST_PTR_FORMAT, caps);
    return FALSE;
  }
error_ioc_reqbufs:
  {
    GST_ERROR_OBJECT (pool, "Unable to request buffers");
    return FALSE;
  }
}

static gboolean
gst_v4l2_out_buf_pool_start (GstBufferPool * bpool)
{
  GstV4L2OutBufPool *pool = GST_V4L2_OUT_BUF_POOL (bpool);
  gint type;

  GST_DEBUG_OBJECT (pool, "Starting");

  pool->num_allocated = 0;

  /* allocate the buffers */
  if (!GST_BUFFER_POOL_CLASS (parent_class)->start (bpool))
    goto start_failed;

  /* Start streaming on output */
  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_STREAMON, &type) < 0)
    goto error_ioc_streamon;

  return TRUE;

  /* ERRORS */
start_failed:
  {
    GST_ERROR_OBJECT (pool, "Failed to start pool");
    return FALSE;
  }
error_ioc_streamon:
  {
    GST_ERROR_OBJECT (pool, "Streamon failed");
    return FALSE;
  }
}

static gboolean
gst_v4l2_out_buf_pool_stop (GstBufferPool * bpool)
{
  GstV4L2OutBufPool *pool = GST_V4L2_OUT_BUF_POOL (bpool);
  gboolean ret;
  gint type;

  GST_DEBUG_OBJECT (pool, "Stopping");

  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_STREAMOFF, &type) < 0)
    goto stop_failed;

  /* free the buffers in the queue */
  ret = GST_BUFFER_POOL_CLASS (parent_class)->stop (bpool);

  if (pool->num_buffers > 0) {
    struct v4l2_requestbuffers reqb;
    memset (&reqb, 0, sizeof reqb);

    reqb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    reqb.count = 0;
    reqb.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_REQBUFS, &reqb) < 0) {
      GST_WARNING_OBJECT (pool, "Could not release buffers");
    }
    pool->num_buffers = 0;
  }

  return ret;

  /* ERRORS */
stop_failed:
  {
    GST_ERROR_OBJECT (pool, "Error with STREAMOFF");
    return FALSE;
  }

}

static GstFlowReturn
gst_v4l2_out_buf_pool_acquire_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstV4L2OutBufPool *pool = GST_V4L2_OUT_BUF_POOL (bpool);
  GstBuffer *outbuf;
  GstFlowReturn ret;
  GstV4L2TransMeta *meta;
  struct v4l2_buffer vbuffer;

  GST_DEBUG_OBJECT (pool, "Acquiring buffer");

  /* Get a released buffer by calling the parent class acquire_buffer.
   * This call is blocking and returns when the pool has an available buffer */
  ret = GST_BUFFER_POOL_CLASS (parent_class)->acquire_buffer (bpool,
      &outbuf, params);
  if (ret)
    goto no_acquire;

  /* Check if this acquired buffer is already dequeued : this happens if the
   * upstream element acquired a buffer and decided to release it without any
   * processing (the processing queues back the buffer).
   * If this is the case we will not dequeue this buffer again */
  meta = GST_V4L2TRANS_META_GET (outbuf);
  if (!meta)
    goto unknown_buffer;

  if (meta->v4l2_dequeued) {
    /* This buffer does not need to be dequeued */
    GST_DEBUG_OBJECT (pool, "Reusing dequeued buffer %p index %d",
        outbuf, meta->vbuffer.index);
  } else {
    /* Dequeue a free V4L2 buffer from driver */
    memset (&vbuffer, 0x00, sizeof (vbuffer));
    vbuffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vbuffer.memory = V4L2_MEMORY_MMAP;

    if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_DQBUF, &vbuffer) < 0)
      goto error;

    meta->v4l2_dequeued = TRUE;

    /* Update meta of the buffer, to used at the next QBUF */
    meta->vbuffer = vbuffer;

    GST_DEBUG_OBJECT (pool,
        "Dequeued output buffer %p, index =%d", outbuf, vbuffer.index);
  }

  *buffer = outbuf;

  return ret;

  /* ERRORS */
error:
  {
    GST_WARNING_OBJECT (pool,
        "Problem dequeuing frame %d (index=%d), pool-ct=%d, buf.flags=%d",
        vbuffer.sequence, vbuffer.index,
        GST_MINI_OBJECT_REFCOUNT (pool), vbuffer.flags);
    return GST_FLOW_ERROR;
  }

unknown_buffer:
  {
    GST_ERROR_OBJECT (pool,
        "The acquired buffer %p is not a known one", outbuf);
    return GST_FLOW_ERROR;
  }

no_acquire:
  {
    if (ret == GST_FLOW_FLUSHING)
      GST_WARNING_OBJECT (pool, "Flushing, cannot get buffer");
    else
      GST_ERROR_OBJECT (pool, "Cannot acquire buffer");
    return ret;
  }
}

GstFlowReturn
gst_v4l2_out_buf_pool_process (GstBufferPool * bpool, GstBuffer * buf)
{
  GstV4L2OutBufPool *pool = GST_V4L2_OUT_BUF_POOL (bpool);
  GstV4L2TransMeta *meta;

  GST_DEBUG_OBJECT (pool, "Processing output buffer");

  meta = GST_V4L2TRANS_META_GET (buf);
  if (!meta)
    goto no_meta;

  meta->vbuffer.bytesused = gst_buffer_get_size (buf);

  if (v4l2_ioctl (pool->v4l2_fd, VIDIOC_QBUF, &meta->vbuffer))
    goto error_ioctl_qbuf;

  meta->v4l2_dequeued = FALSE;

  GST_DEBUG_OBJECT (pool, "Queued output buffer %p, index %d",
      buf, meta->vbuffer.index);

  return GST_FLOW_OK;

  /* ERRORS */
no_meta:
  {
    GST_ERROR_OBJECT (pool, "No meta attached to buffer");
    return GST_FLOW_ERROR;
  }
error_ioctl_qbuf:
  {
    GST_ERROR_OBJECT (pool, "QBUF failed");
    return GST_FLOW_ERROR;
  }
}

static void
gst_v4l2_out_buf_pool_finalize (GObject * object)
{
  GstV4L2OutBufPool *pool = GST_V4L2_OUT_BUF_POOL (object);

  if (pool->v4l2_fd > 0)
    v4l2_close (pool->v4l2_fd);
  if (pool->allocator)
    gst_object_unref (pool->allocator);
  gst_object_unref (pool->trans);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_v4l2_out_buf_pool_init (GstV4L2OutBufPool * pool)
{
}

static void
gst_v4l2_out_buf_pool_class_init (GstV4L2OutBufPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_v4l2_out_buf_pool_finalize;

  bufferpool_class->start = gst_v4l2_out_buf_pool_start;
  bufferpool_class->stop = gst_v4l2_out_buf_pool_stop;
  bufferpool_class->get_options = gst_v4l2_out_buf_pool_get_options;
  bufferpool_class->set_config = gst_v4l2_out_buf_pool_set_config;
  bufferpool_class->alloc_buffer = gst_v4l2_out_buf_pool_alloc_buffer;
  bufferpool_class->acquire_buffer = gst_v4l2_out_buf_pool_acquire_buffer;
}

GstBufferPool *
gst_v4l2_out_buf_pool_new (GstV4L2Trans * trans)
{
  GstV4L2OutBufPool *pool;
  gint v4l2_fd;

  GST_DEBUG_OBJECT (trans, "Creating output bufferpool");

  v4l2_fd = v4l2_dup (trans->fd);
  if (v4l2_fd < 0) {
    GST_DEBUG ("Failed to dup fd");
    return NULL;
  }

  pool = (GstV4L2OutBufPool *)
      g_object_new (GST_TYPE_V4L2_OUT_BUF_POOL, NULL);

  if (pool) {
    /* take a reference on v4l2trans to be sure it is released after the pool */
    pool->trans = gst_object_ref (trans);
    pool->v4l2_fd = v4l2_fd;
  }

  return GST_BUFFER_POOL (pool);
}
