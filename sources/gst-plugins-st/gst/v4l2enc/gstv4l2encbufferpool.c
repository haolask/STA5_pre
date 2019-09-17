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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#include <gstv4l2encbufferpool.h>

GST_DEBUG_CATEGORY_EXTERN (gst_v4l2enc_debug);
#define GST_CAT_DEFAULT gst_v4l2enc_debug

/*
 * GstV4L2EncBuffer:
 */
GType
gst_v4l2enc_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { "memory", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("GstV4L2EncMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

const GstMetaInfo *
gst_v4l2enc_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_v4l2enc_meta_api_get_type (), "GstV4L2EncMeta",
        sizeof (GstV4L2EncMeta), (GstMetaInitFunction) NULL,
        (GstMetaFreeFunction) NULL, (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

/*
 * GstV4L2EncBufferPool:
 */
#define gst_v4l2enc_buffer_pool_parent_class parent_class
G_DEFINE_TYPE (GstV4L2EncBufferPool, gst_v4l2enc_buffer_pool,
    GST_TYPE_BUFFER_POOL);

static const gchar **
gst_v4l2enc_buffer_pool_get_options (GstBufferPool * pool)
{
  static const gchar *options[] = { GST_BUFFER_POOL_OPTION_VIDEO_META,
    GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT, NULL
  };

  return options;
}

static void gst_v4l2enc_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer);

static void
gst_v4l2enc_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);

  GST_DEBUG_OBJECT (pool, "free buffer %p", buffer);

  gst_buffer_unref (buffer);
}

static GstFlowReturn
gst_v4l2enc_buffer_pool_alloc_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);
  GstBuffer *newbuf;
  GstV4L2EncMeta *meta;
  guint index;
  struct v4l2_exportbuffer expbuf;
  GstV4L2Enc *enc = pool->enc;
  GstVideoInfo *info = &enc->info;

  newbuf = gst_buffer_new ();
  meta = GST_V4L2ENC_META_ADD (newbuf);

  index = pool->num_allocated;

  GST_DEBUG_OBJECT (pool, "creating buffer %u, %p for pool %p", index, newbuf,
      pool);

  memset (&meta->vbuffer, 0, sizeof (struct v4l2_buffer));
  meta->vbuffer.index = index;
  meta->vbuffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  meta->vbuffer.memory = V4L2_MEMORY_MMAP;

  if (v4l2_ioctl (pool->video_fd, VIDIOC_QUERYBUF, &meta->vbuffer) < 0)
    goto querybuf_failed;

  GST_LOG_OBJECT (pool, "  index:     %u", meta->vbuffer.index);
  GST_LOG_OBJECT (pool, "  type:      %d", meta->vbuffer.type);
  GST_LOG_OBJECT (pool, "  bytesused: %u", meta->vbuffer.bytesused);
  GST_LOG_OBJECT (pool, "  flags:     %08x", meta->vbuffer.flags);
  GST_LOG_OBJECT (pool, "  field:     %d", meta->vbuffer.field);
  GST_LOG_OBJECT (pool, "  memory:    %d", meta->vbuffer.memory);
  GST_LOG_OBJECT (pool, "  MMAP offset:  %u", meta->vbuffer.m.offset);

  memset (&expbuf, 0, sizeof (struct v4l2_exportbuffer));
  expbuf.type = meta->vbuffer.type;
  expbuf.index = meta->vbuffer.index;
  expbuf.flags = O_CLOEXEC | O_RDWR;
  if (v4l2_ioctl (pool->video_fd, VIDIOC_EXPBUF, &expbuf) < 0)
    goto expbuf_failed;

  gst_buffer_append_memory (newbuf,
      gst_dmabuf_allocator_alloc (pool->allocator, expbuf.fd,
          meta->vbuffer.length));
  pool->num_allocated++;

  /* output buffer ready to be filled in */
  meta->v4l2_dequeued = TRUE;

  gst_buffer_add_video_meta_full (newbuf, GST_VIDEO_FRAME_FLAG_NONE,
      GST_VIDEO_INFO_FORMAT (info), GST_VIDEO_INFO_WIDTH (info),
      GST_VIDEO_INFO_HEIGHT (info), GST_VIDEO_INFO_N_PLANES (info),
      info->offset, info->stride);

  *buffer = newbuf;

  /* Allocation on-demand is not supported:
   * if no buffers are available in pool, pool must wait for
   * buffer release. To trig this behaviour, max limit
   * must be set in pool config after all buffers are allocated.
   * Because allocated buffers are reserved for driver
   * internal usage (cf REQBUFS), max is limited to buffers
   * really available for pool.
   */
  if (pool->num_allocated >= pool->num_buffers) {
    GstStructure *config = gst_buffer_pool_get_config (bpool);
    GstCaps *caps;
    guint size, min_buffers, max_buffers;

    gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
        &max_buffers);
    gst_buffer_pool_config_set_params (config, caps, size,
        pool->num_buffers_available, pool->num_buffers_available);
  }

  return GST_FLOW_OK;

  /* ERRORS */
querybuf_failed:
  {
    GST_WARNING ("Failed QUERYBUF: %s", strerror (errno));
    gst_buffer_unref (newbuf);
    return GST_FLOW_ERROR;
  }
expbuf_failed:
  {
    GST_WARNING ("Failed EXPBUF: %s", strerror (errno));
    gst_buffer_unref (newbuf);
    return GST_FLOW_ERROR;
  }
}

static gboolean
gst_v4l2enc_buffer_pool_set_config (GstBufferPool * bpool,
    GstStructure * config)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);
  GstCaps *caps;
  GstVideoInfo info;
  guint size, min_buffers, max_buffers, num_buffers;
  GstAllocator *allocator;
  GstAllocationParams params;
  struct v4l2_requestbuffers reqbufs;
  struct v4l2_format s_fmt;
  struct v4l2_format g_fmt;
  int ret;
  GstV4L2Enc *enc = pool->enc;
  gboolean need_alignment;
  GstVideoAlignment align;
  guint aligned_width;
  guint aligned_height;

  GST_DEBUG_OBJECT (pool, "set config %" GST_PTR_FORMAT, config);

  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
          &max_buffers))
    goto wrong_config;

  if (caps == NULL)
    goto no_caps;

  if (!gst_buffer_pool_config_get_allocator (config, &allocator, &params))
    goto wrong_config;

  /* now parse the caps from the config */
  if (!gst_video_info_from_caps (&info, caps))
    goto wrong_caps;

  if (enc->caps)
    gst_caps_unref (enc->caps);
  enc->caps = gst_caps_ref (caps);

  /* parse extra alignment info */
  need_alignment =
      gst_buffer_pool_config_has_option (config,
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

  enc->info = info;

  /* Now configure V4L2 input */
  memset (&s_fmt, 0, sizeof s_fmt);
  s_fmt.fmt.pix.width = GST_VIDEO_INFO_WIDTH (&info);
  s_fmt.fmt.pix.height = GST_VIDEO_INFO_HEIGHT (&info);
  s_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

  switch (GST_VIDEO_INFO_FORMAT (&info)) {
    case GST_VIDEO_FORMAT_NV12:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
      s_fmt.fmt.pix.bytesperline = aligned_width;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 3 / 2);
      break;
    case GST_VIDEO_FORMAT_NV21:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV21;
      s_fmt.fmt.pix.bytesperline = aligned_width;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 3 / 2);
      break;
    case GST_VIDEO_FORMAT_UYVY:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
      s_fmt.fmt.pix.bytesperline = aligned_width * 2;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 2);
      break;
      /* FIXME format VYUY missing replace by YUY2 */
    case GST_VIDEO_FORMAT_YUY2:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_VYUY;
      s_fmt.fmt.pix.bytesperline = aligned_width * 2;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 2);
      break;
    case GST_VIDEO_FORMAT_RGB:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
      s_fmt.fmt.pix.bytesperline = aligned_width * 3;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 3);
      break;
    case GST_VIDEO_FORMAT_BGR:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
      s_fmt.fmt.pix.bytesperline = aligned_width * 3;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 3);
      break;
    case GST_VIDEO_FORMAT_RGBx:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
      s_fmt.fmt.pix.bytesperline = aligned_width * 4;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 4);
      break;
    case GST_VIDEO_FORMAT_xRGB:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_XRGB32;
      s_fmt.fmt.pix.bytesperline = aligned_width * 4;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 4);
      break;
    case GST_VIDEO_FORMAT_BGRx:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR32;
      s_fmt.fmt.pix.bytesperline = aligned_width * 4;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 4);
      break;
    case GST_VIDEO_FORMAT_xBGR:
      s_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_XBGR32;
      s_fmt.fmt.pix.bytesperline = aligned_width * 4;
      s_fmt.fmt.pix.sizeimage = (aligned_width * aligned_height * 4);
      break;
    default:
      goto wrong_config;
      break;
  }

  ret = v4l2_ioctl (enc->fd, VIDIOC_S_FMT, &s_fmt);
  if (ret != 0)
    goto error_s_fmt;

  memset (&g_fmt, 0, sizeof g_fmt);
  g_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ret = v4l2_ioctl (enc->fd, VIDIOC_G_FMT, &g_fmt);
  if (ret != 0)
    goto error_g_fmt;

  GST_DEBUG_OBJECT (enc,
      "Format found from V4L2 : fmt %s, width:%d, height:%d, bytesperline:%d, sizeimage:%d",
      v4l2_fmt_str (g_fmt.fmt.pix.pixelformat),
      g_fmt.fmt.pix.width,
      g_fmt.fmt.pix.height, g_fmt.fmt.pix.bytesperline,
      g_fmt.fmt.pix.sizeimage);

  /* check if a new alignment is necessary from driver */
  switch (g_fmt.fmt.pix.pixelformat) {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
      align.padding_right = g_fmt.fmt.pix.bytesperline - g_fmt.fmt.pix.width;
      align.padding_bottom = (g_fmt.fmt.pix.sizeimage /
          (g_fmt.fmt.pix.bytesperline * 3 / 2)) - g_fmt.fmt.pix.height;
      break;
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_VYUY:
      align.padding_right = g_fmt.fmt.pix.bytesperline / 2 -
          g_fmt.fmt.pix.width;
      align.padding_bottom = (g_fmt.fmt.pix.sizeimage /
          g_fmt.fmt.pix.bytesperline) - g_fmt.fmt.pix.height;
      break;
    case V4L2_PIX_FMT_RGB24:
    case V4L2_PIX_FMT_BGR24:
      align.padding_right = g_fmt.fmt.pix.bytesperline / 3 -
          g_fmt.fmt.pix.width;
      align.padding_bottom = (g_fmt.fmt.pix.sizeimage /
          g_fmt.fmt.pix.bytesperline) - g_fmt.fmt.pix.height;
      break;
    case V4L2_PIX_FMT_XRGB32:
    case V4L2_PIX_FMT_XBGR32:
    case V4L2_PIX_FMT_RGB32:
    case V4L2_PIX_FMT_BGR32:
      align.padding_right = g_fmt.fmt.pix.bytesperline / 4 -
          g_fmt.fmt.pix.width;
      align.padding_bottom = (g_fmt.fmt.pix.sizeimage /
          g_fmt.fmt.pix.bytesperline) - g_fmt.fmt.pix.height;
      break;
    default:
      goto wrong_config;
      break;
  }
  need_alignment = ((align.padding_right + align.padding_bottom) != 0);

  if (need_alignment) {
    /* add pool option alingment */
    if (!gst_buffer_pool_config_has_option (config,
            GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT))
      gst_buffer_pool_config_add_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);

    /* add pool option metadata */
    if (!gst_buffer_pool_config_has_option (config,
            GST_BUFFER_POOL_OPTION_VIDEO_META))
      gst_buffer_pool_config_add_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_META);

    /* save align to encoder info & pool config */
    gst_video_info_align (&enc->info, &align);
    gst_buffer_pool_config_set_video_alignment (config, &align);

    GST_LOG_OBJECT (pool, "padding %u-%ux%u-%u", align.padding_top,
        align.padding_left, align.padding_right, align.padding_bottom);
  }

  /* On-demand allocation is not supported, so max=min in config.
   * if max is specified, max buffers must be allocated
   */
  if (max_buffers != 0)
    min_buffers = max_buffers;
  num_buffers = min_buffers;

  /* Request num_buffers to V4L2 */
  /* if buffers already available,
   * set count to zero to request release of all existing buffers
   */
  if (pool->num_buffers_available) {
    memset (&reqbufs, 0, sizeof reqbufs);
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    reqbufs.count = 0;
    reqbufs.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl (pool->video_fd, VIDIOC_REQBUFS, &reqbufs) < 0)
      goto error_ioc_reqbufs;
  }

  memset (&reqbufs, 0, sizeof reqbufs);
  reqbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbufs.count = num_buffers;
  reqbufs.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl (pool->video_fd, VIDIOC_REQBUFS, &reqbufs) < 0)
    goto error_ioc_reqbufs;

  /* V4L2 could request a size image bigger than request
   * adjust size (read size image from g_fmt) and reinject it in pool config
   */
  size = g_fmt.fmt.pix.sizeimage;

  /* V4L2 could request more for internal purpose, so we
   * adjust num_buffers and reinject it in pool config
   */
  pool->num_buffers_available = num_buffers;
  num_buffers = reqbufs.count;
  gst_buffer_pool_config_set_params (config, caps, size, num_buffers,
      num_buffers);

  pool->num_buffers = num_buffers;

  allocator = gst_dmabuf_allocator_new ();
  if (pool->allocator)
    gst_object_unref (pool->allocator);
  if ((pool->allocator = allocator))
    gst_object_ref (allocator);

  GST_DEBUG_OBJECT (pool, "config %" GST_PTR_FORMAT, config);

  return GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);

  /* ERRORS */
error_s_fmt:
  {
    GST_ERROR_OBJECT (pool, "Unable to set input format err=%s",
        strerror (errno));
    return FALSE;
  }
error_g_fmt:
  {
    GST_ERROR_OBJECT (pool, "Unable to get input format err=%s",
        strerror (errno));
    return FALSE;
  }
wrong_config:
  {
    GST_ERROR_OBJECT (pool, "invalid config %" GST_PTR_FORMAT, config);
    return FALSE;
  }
no_caps:
  {
    GST_WARNING_OBJECT (pool, "no caps in config");
    return FALSE;
  }
wrong_caps:
  {
    GST_WARNING_OBJECT (pool,
        "failed getting geometry from caps %" GST_PTR_FORMAT, caps);
    return FALSE;
  }
error_ioc_reqbufs:
  {
    GST_ERROR_OBJECT (pool, "Unable to request buffers err=%s",
        strerror (errno));
    return FALSE;
  }
}

static gboolean
gst_v4l2enc_buffer_pool_start (GstBufferPool * bpool)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);
  GstV4L2Enc *enc = pool->enc;
  gint type;

  GST_DEBUG_OBJECT (pool, "start");
  pool->num_allocated = 0;

  /* allocate the buffers */
  if (!GST_BUFFER_POOL_CLASS (parent_class)->start (bpool))
    goto start_failed;

  /* Start streaming on output */
  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if (v4l2_ioctl (enc->fd, VIDIOC_STREAMON, &type) < 0)
    goto error_ioc_streamon;

  return TRUE;

  /* ERRORS */
start_failed:
  {
    GST_ERROR_OBJECT (pool, "failed to start pool");
    return FALSE;
  }
error_ioc_streamon:
  {
    GST_ERROR_OBJECT (pool, "Streamon failed err=%s", strerror (errno));
    return FALSE;
  }
}

static gboolean
gst_v4l2enc_buffer_pool_stop (GstBufferPool * bpool)
{
  gboolean ret;
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);
  gint type;

  GST_DEBUG_OBJECT (pool, "stop pool");

  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if (v4l2_ioctl (pool->video_fd, VIDIOC_STREAMOFF, &type) < 0)
    goto stop_failed;

  /* free the buffers in the queue */
  ret = GST_BUFFER_POOL_CLASS (parent_class)->stop (bpool);

  if (pool->num_buffers > 0) {
    struct v4l2_requestbuffers breq;
    memset (&breq, 0, sizeof (struct v4l2_requestbuffers));
    breq.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    breq.count = 0;
    breq.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl (pool->video_fd, VIDIOC_REQBUFS, &breq) < 0) {
      GST_ERROR_OBJECT (pool, "error releasing buffers: %s", strerror (errno));
    }
    pool->num_buffers = 0;
  }

  return ret;

  /* ERRORS */
stop_failed:
  {
    GST_ERROR_OBJECT (pool, "error with STREAMOFF %d (%s)", errno,
        g_strerror (errno));
    return FALSE;
  }
}

static GstFlowReturn
gst_v4l2enc_buffer_pool_acquire_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);
  GstBuffer *outbuf;
  GstFlowReturn ret;
  GstV4L2EncMeta *meta;
  struct v4l2_buffer vbuffer;

  GST_DEBUG_OBJECT (pool, "Acquiring buffer");

  /* Get a released buffer by calling the parent class acquire_buffer.
   * This call is blocking and returns when the pool has an available buffer
   */
  ret = GST_BUFFER_POOL_CLASS (parent_class)->acquire_buffer (bpool,
      &outbuf, params);
  if (ret)
    goto no_acquire;

  meta = GST_V4L2ENC_META_GET (outbuf);
  if (!meta)
    goto unknown_buffer;

  /* try to acquire the buffer released directly by gstreamer e.g. by segment clipping API.
   * the buffer released this way is never queued with the driver and therefore don't need dqueue.
   * we will reuse the v4l2 buffer associated with the gst buffer acquired from gstreamer pool.
   */
  if (meta->v4l2_dequeued) {
    /* This buffer does not need to be dequeued */
    GST_DEBUG_OBJECT (pool, "Reusing dequeued buffer %p index %d",
        outbuf, meta->vbuffer.index);
  } else {
    /* Dequeue a free V4L2 buffer from driver */
    memset (&vbuffer, 0x00, sizeof (vbuffer));
    vbuffer.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vbuffer.memory = V4L2_MEMORY_MMAP;

    if (v4l2_ioctl (pool->video_fd, VIDIOC_DQBUF, &vbuffer) < 0)
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
        "problem dequeuing frame %d (index=%d), pool-ct=%d, buf.flags=%d",
        vbuffer.sequence, vbuffer.index,
        GST_MINI_OBJECT_REFCOUNT (pool), vbuffer.flags);

    switch (errno) {
      case EAGAIN:
        GST_WARNING_OBJECT (pool,
            "Non-blocking I/O has been selected using O_NONBLOCK and"
            " no buffer was in the outgoing queue");
        break;
      case EINVAL:
        GST_DEBUG_OBJECT (pool,
            "The buffer type is not supported, or the index is out of bounds, "
            "or no buffers have been allocated yet, or the userptr "
            "or length are invalid");
        break;
      case ENOMEM:
        GST_ERROR_OBJECT (pool,
            "insufficient memory to enqueue a user pointer buffer");
        break;
      case EIO:
        GST_INFO_OBJECT (pool,
            "VIDIOC_DQBUF failed due to an internal error."
            " Can also indicate temporary problems like signal loss."
            " Note the driver might dequeue an (empty) buffer despite"
            " returning an error, or even stop capturing");
        /* have we de-queued a buffer ? */
        if (!(vbuffer.flags & (V4L2_BUF_FLAG_QUEUED | V4L2_BUF_FLAG_DONE))) {
          GST_DEBUG_OBJECT (pool, "reenqueing buffer");
          /* FIXME ... should we do something here? */
        }
        break;
      case EINTR:
        GST_WARNING_OBJECT (pool, "could not sync on a buffer on device");
        break;
      default:
        GST_WARNING_OBJECT (pool,
            "Grabbing frame got interrupted on  unexpectedly. %s.",
            strerror (errno));
        break;
    }
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

static void
gst_v4l2enc_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);
  GstV4L2EncMeta *meta;

  GST_DEBUG_OBJECT (pool, "release buffer %p", buffer);

  meta = GST_V4L2ENC_META_GET (buffer);
  if (meta == NULL) {
    GST_LOG_OBJECT (pool, "unref copied buffer %p", buffer);
    /* no meta, it was a copied buffer that we can unref */
    gst_buffer_unref (buffer);
    return;
  }

  GST_DEBUG_OBJECT (pool,
      "release buffer %p, index %d", buffer, meta->vbuffer.index);

  /* Don't return to the pool the internal buffers,
   * so acquire_buffer will block if available buffers
   * have all been acquired
   */
  if (meta->vbuffer.index < pool->num_buffers_available)
    GST_BUFFER_POOL_CLASS (parent_class)->release_buffer (bpool, buffer);

  return;
}

GstFlowReturn
gst_v4l2enc_buffer_pool_process (GstBufferPool * bpool, GstBuffer * buf)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (bpool);
  GstV4L2EncMeta *meta;
  GstV4L2Enc *enc = pool->enc;

  GST_DEBUG_OBJECT (pool, "process input buffer");

  meta = GST_V4L2ENC_META_GET (buf);
  if (!meta)
    goto no_meta;

  meta->vbuffer.bytesused = gst_buffer_get_size (buf);

  /* Frame timestamp */
  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (buf)))
    GST_TIME_TO_TIMEVAL (GST_BUFFER_TIMESTAMP (buf), meta->vbuffer.timestamp);

  if (v4l2_ioctl (enc->fd, VIDIOC_QBUF, &meta->vbuffer))
    goto error_ioctl_qbuf;

  meta->v4l2_dequeued = FALSE;

  GST_DEBUG_OBJECT (pool, "Queued output buffer %p, index %d",
      buf, meta->vbuffer.index);

  return GST_FLOW_OK;

  /* ERRORS */
no_meta:
  {
    GST_ERROR_OBJECT (enc, "no meta attached to buffer");
    return GST_FLOW_ERROR;
  }
error_ioctl_qbuf:
  {
    GST_ERROR_OBJECT (enc, "QBUF(OUTPUT) failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
}

static void
gst_v4l2enc_buffer_pool_finalize (GObject * object)
{
  GstV4L2EncBufferPool *pool = GST_V4L2ENC_BUFFER_POOL (object);

  GST_DEBUG_OBJECT (pool, "finalize pool");

  if (pool->video_fd != -1) {
    v4l2_close (pool->video_fd);
    pool->video_fd = -1;
  }
  if (pool->allocator)
    gst_object_unref (pool->allocator);
  gst_object_unref (pool->enc);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_v4l2enc_buffer_pool_init (GstV4L2EncBufferPool * pool)
{
}

static void
gst_v4l2enc_buffer_pool_class_init (GstV4L2EncBufferPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_v4l2enc_buffer_pool_finalize;

  bufferpool_class->start = gst_v4l2enc_buffer_pool_start;
  bufferpool_class->stop = gst_v4l2enc_buffer_pool_stop;
  bufferpool_class->get_options = gst_v4l2enc_buffer_pool_get_options;
  bufferpool_class->set_config = gst_v4l2enc_buffer_pool_set_config;
  bufferpool_class->alloc_buffer = gst_v4l2enc_buffer_pool_alloc_buffer;
  bufferpool_class->acquire_buffer = gst_v4l2enc_buffer_pool_acquire_buffer;
  bufferpool_class->release_buffer = gst_v4l2enc_buffer_pool_release_buffer;
  bufferpool_class->free_buffer = gst_v4l2enc_buffer_pool_free_buffer;
}

/**
 * gst_v4l2enc_buffer_pool_new:
 * @enc: the v4l2 encoder owning the pool
 * @max: maximum buffers in the pool
 *
 * Construct a new buffer pool.
 *
 * Returns: the new pool, use gst_object_unref() to free resources
 */
GstBufferPool *
gst_v4l2enc_buffer_pool_new (GstV4L2Enc * enc)
{
  GstV4L2EncBufferPool *pool;
  gint fd;

  GST_DEBUG_OBJECT (enc, "construct a new buffer pool");

  fd = v4l2_dup (enc->fd);
  if (fd < 0)
    goto dup_failed;

  pool =
      (GstV4L2EncBufferPool *) g_object_new (GST_TYPE_V4L2ENC_BUFFER_POOL,
      NULL);
  /* take a reference on v4l2enc to be sure that it will be release after the pool */
  pool->enc = gst_object_ref (enc);
  pool->video_fd = fd;

  return GST_BUFFER_POOL (pool);

  /* ERRORS */
dup_failed:
  {
    GST_DEBUG ("failed to dup fd (%s)", strerror (errno));
    return NULL;
  }
}
