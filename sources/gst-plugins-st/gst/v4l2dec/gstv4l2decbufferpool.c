/* GStreamer V4L2 video decoder plugin
 *
 * Copyright (C) 2015 STMicroelectronics SA
 *
 * Authors:
 *   Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics
 *   Jean-Christophe Trotin <jean-christophe.trotin@st.com> for STMicroelectronics
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

#include <gstv4l2decbufferpool.h>

GST_DEBUG_CATEGORY_EXTERN (gst_v4l2dec_debug);
#define GST_CAT_DEFAULT gst_v4l2dec_debug

/*
 * GstV4L2DecBuffer:
 */
GType
gst_v4l2dec_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { "memory", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("GstV4L2DecMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

static gboolean gst_v4l2dec_meta_init(GstMeta *meta,
	G_GNUC_UNUSED gpointer params,
	G_GNUC_UNUSED GstBuffer *buffer)
{
	/* Just to avoid a warning */
	return TRUE;
}

const GstMetaInfo *
gst_v4l2dec_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_v4l2dec_meta_api_get_type (), "GstV4L2DecMeta",
        sizeof (GstV4L2DecMeta), (GstMetaInitFunction) gst_v4l2dec_meta_init,
        (GstMetaFreeFunction) NULL, (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

/*
 * GstV4L2DecBufferPool:
 */
#define gst_v4l2dec_buffer_pool_parent_class parent_class
G_DEFINE_TYPE (GstV4L2DecBufferPool, gst_v4l2dec_buffer_pool,
    GST_TYPE_BUFFER_POOL);

static void gst_v4l2dec_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer);

static void
gst_v4l2dec_buffer_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);

  GST_DEBUG_OBJECT (pool, "free buffer %p from pool %p", buffer, pool);

#ifndef USE_V4L2_MEMORY_DMABUF
  {
    GstV4L2DecMeta *meta;
    gint index;

    meta = GST_V4L2DEC_META_GET (buffer);
    g_assert (meta != NULL);

    index = meta->vbuffer.index;
    GST_LOG_OBJECT (pool,
        "unmap buffer %p idx %d (data %p, len %u)", buffer,
        index, meta->mem, meta->vbuffer.length);

    v4l2_munmap (meta->mem, meta->vbuffer.length);
    pool->buffers[index] = NULL;
  }
#endif

  gst_buffer_unref (buffer);
}

static GstFlowReturn
gst_v4l2dec_buffer_pool_alloc_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);
  GstBuffer *newbuf;
  GstV4L2DecMeta *meta;
  guint index;
#ifdef USE_V4L2_MEMORY_DMABUF
  struct v4l2_exportbuffer expbuf;
#endif

  newbuf = gst_buffer_new ();
  meta = GST_V4L2DEC_META_ADD (newbuf);

  index = pool->num_allocated;

  GST_DEBUG_OBJECT (pool, "creating buffer %u, %p for pool %p", index, newbuf,
      pool);

  memset (&meta->vbuffer, 0, sizeof (struct v4l2_buffer));
  meta->vbuffer.index = index;
  meta->vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
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

#ifdef USE_V4L2_MEMORY_DMABUF
  memset (&expbuf, 0, sizeof (struct v4l2_exportbuffer));
  expbuf.type = meta->vbuffer.type;
  expbuf.index = meta->vbuffer.index;
  expbuf.flags = O_CLOEXEC | O_RDWR;
  if (v4l2_ioctl (pool->video_fd, VIDIOC_EXPBUF, &expbuf) < 0)
    goto expbuf_failed;

  gst_buffer_append_memory (newbuf,
      gst_dmabuf_allocator_alloc (pool->allocator, expbuf.fd,
          meta->vbuffer.length));
#else
  meta->mem = v4l2_mmap (0, meta->vbuffer.length,
      PROT_READ | PROT_WRITE, MAP_SHARED, pool->video_fd,
      meta->vbuffer.m.offset);
  if (meta->mem == MAP_FAILED)
    goto mmap_failed;

  gst_buffer_append_memory (newbuf,
      gst_memory_new_wrapped (GST_MEMORY_FLAG_NO_SHARE,
          meta->mem, meta->vbuffer.length, 0, meta->vbuffer.length, NULL,
          NULL));
#endif
  pool->num_allocated++;

  *buffer = newbuf;

  return GST_FLOW_OK;

  /* ERRORS */
querybuf_failed:
  {
    gint errnosave = errno;

    GST_WARNING ("Failed QUERYBUF: %s", g_strerror (errnosave));
    gst_buffer_unref (newbuf);
    errno = errnosave;
    return GST_FLOW_ERROR;
  }
#ifdef USE_V4L2_MEMORY_DMABUF
expbuf_failed:
  {
    gint errnosave = errno;

    GST_WARNING ("Failed EXPBUF: %s", g_strerror (errnosave));
    gst_buffer_unref (newbuf);
    errno = errnosave;
    return GST_FLOW_ERROR;
  }
#else
mmap_failed:
  {
    gint errnosave = errno;

    GST_WARNING ("Failed to mmap: %s", g_strerror (errnosave));
    gst_buffer_unref (newbuf);
    errno = errnosave;
    return GST_FLOW_ERROR;
  }
#endif
}

static gboolean
gst_v4l2dec_buffer_pool_set_config (GstBufferPool * bpool,
    GstStructure * config)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);
  GstCaps *caps;
  guint size, min_buffers, max_buffers;
  GstAllocator *allocator;
  GstAllocationParams params;
  gboolean has_alignment = FALSE;
  GstVideoAlignment video_align;

  GST_DEBUG_OBJECT (pool, "set config");

  /* parse the config and keep around */
  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
          &max_buffers))
    goto wrong_config;

  if (!gst_buffer_pool_config_get_allocator (config, &allocator, &params))
    goto wrong_config;

  GST_DEBUG_OBJECT (pool, "config %" GST_PTR_FORMAT, config);

  pool->num_buffers = max_buffers;

  allocator = gst_dmabuf_allocator_new ();

  if (pool->allocator)
    gst_object_unref (pool->allocator);
  if ((pool->allocator = allocator))
    gst_object_ref (allocator);
  pool->params = params;

  gst_buffer_pool_config_set_params (config, caps, size, min_buffers,
      max_buffers);

  /* parse extra alignment info */
  has_alignment = gst_buffer_pool_config_has_option (config,
      GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);

  if (!has_alignment)
    goto missing_video_api;

  /* get and apply the alignment to the output info */
  gst_buffer_pool_config_get_video_alignment (config, &video_align);
  gst_video_info_align (&pool->dec->output_state->info, &video_align);

  GST_LOG_OBJECT (pool, "padding %u-%ux%u-%u",
      video_align.padding_top,
      video_align.padding_left,
      video_align.padding_right, video_align.padding_bottom);

  return GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);

  /* ERRORS */
missing_video_api:
  {
    GST_ERROR_OBJECT (pool,
        "missing GstMetaVideo API in config (alignment needed)");
    return FALSE;
  }
wrong_config:
  {
    GST_ERROR_OBJECT (pool, "invalid config %" GST_PTR_FORMAT, config);
    return FALSE;
  }
}

static gboolean
gst_v4l2dec_buffer_pool_start (GstBufferPool * bpool)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);

  GST_DEBUG_OBJECT (pool, "start pool %p", pool);

  pool->buffers = g_new0 (GstBuffer *, pool->num_buffers);
  pool->num_allocated = 0;

  /* allocate the buffers */
  if (!GST_BUFFER_POOL_CLASS (parent_class)->start (bpool))
    goto start_failed;

  return TRUE;

  /* ERRORS */
start_failed:
  {
    GST_ERROR_OBJECT (pool, "failed to start pool %p", pool);
    return FALSE;
  }
}

static gboolean
gst_v4l2dec_buffer_pool_stop (GstBufferPool * bpool)
{
  gboolean ret;
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);
  guint n;

  GST_DEBUG_OBJECT (pool, "stop pool %p", pool);

  /* free the buffers in the queue */
  ret = GST_BUFFER_POOL_CLASS (parent_class)->stop (bpool);

  /* free the remaining buffers */
  for (n = 0; n < pool->num_buffers; n++) {
    if (pool->buffers[n])
      gst_v4l2dec_buffer_pool_free_buffer (bpool, pool->buffers[n]);
  }
  pool->num_queued = 0;
  g_free (pool->buffers);
  pool->buffers = NULL;

  if (pool->num_buffers > 0) {
    struct v4l2_requestbuffers breq;
    memset (&breq, 0, sizeof (struct v4l2_requestbuffers));
    breq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    breq.count = 0;
    breq.memory = V4L2_MEMORY_MMAP;
    if (v4l2_ioctl (pool->video_fd, VIDIOC_REQBUFS, &breq) < 0) {
      GST_WARNING_OBJECT (pool, "error releasing buffers: %s",
          g_strerror (errno));
    }
    pool->num_buffers = 0;
  }

  return ret;
}

static GstFlowReturn
gst_v4l2dec_buffer_pool_acquire_buffer (GstBufferPool * bpool,
    GstBuffer ** buffer, GstBufferPoolAcquireParams * params)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);
  GstBuffer *outbuf;
  struct v4l2_buffer vbuffer;
  GstClockTime timestamp;

  GST_DEBUG_OBJECT (pool, "acquire buffer from pool %p", pool);
  if (pool->flushing)
    return GST_FLOW_FLUSHING;

  memset (&vbuffer, 0x00, sizeof (vbuffer));
  vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  vbuffer.memory = V4L2_MEMORY_MMAP;

  GST_DEBUG_OBJECT (pool, "dequeue buffer (capture type)");

  if (v4l2_ioctl (pool->video_fd, VIDIOC_DQBUF, &vbuffer) < 0) {
    if (errno == EPIPE)
      return GST_FLOW_EOS; /* EOS */
    else
      goto error;
  }

  if ((vbuffer.flags & V4L2_BUF_FLAG_LAST) &&
    vbuffer.bytesused == 0)
    return GST_FLOW_EOS; /* last empty buffer dqueued */

  /* get from the pool the GstBuffer associated with the index */
  outbuf = pool->buffers[vbuffer.index];
  if (outbuf == NULL)
    goto no_buffer;

  pool->buffers[vbuffer.index] = NULL;
  pool->num_queued--;

  timestamp = GST_TIMEVAL_TO_TIME (vbuffer.timestamp);

  GST_DEBUG_OBJECT (pool,
      "dequeued buffer %p, index =%d, ts %"
      GST_TIME_FORMAT " (pool-queued=%d)", outbuf, vbuffer.index,
      GST_TIME_ARGS (timestamp), pool->num_queued);

  GST_BUFFER_TIMESTAMP (outbuf) = timestamp;

  /* set top/bottom field first if v4l2_buffer has the information */
  if ((vbuffer.field == V4L2_FIELD_INTERLACED_TB)
      || (vbuffer.field == V4L2_FIELD_INTERLACED)) {
    GST_BUFFER_FLAG_SET (outbuf, GST_VIDEO_BUFFER_FLAG_INTERLACED);
    GST_BUFFER_FLAG_SET (outbuf, GST_VIDEO_BUFFER_FLAG_TFF);
  } else if (vbuffer.field == V4L2_FIELD_INTERLACED_BT) {
    GST_BUFFER_FLAG_SET (outbuf, GST_VIDEO_BUFFER_FLAG_INTERLACED);
    GST_BUFFER_FLAG_UNSET (outbuf, GST_VIDEO_BUFFER_FLAG_TFF);
  } else {
    /* per default, the frame is considered as progressive */
    GST_BUFFER_FLAG_UNSET (outbuf, GST_VIDEO_BUFFER_FLAG_INTERLACED);
    GST_BUFFER_FLAG_UNSET (outbuf, GST_VIDEO_BUFFER_FLAG_TFF);
  }

  *buffer = outbuf;

  return GST_FLOW_OK;

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
            "Grabbing frame got interrupted on  unexpectedly. %d: %s.",
            errno, g_strerror (errno));
        break;
    }
    return GST_FLOW_ERROR;
  }
no_buffer:
  {
    GST_ERROR_OBJECT (pool, "No free buffer found in the pool at index %d",
        vbuffer.index);
    return GST_FLOW_ERROR;
  }
}

static void
gst_v4l2dec_buffer_pool_release_buffer (GstBufferPool * bpool,
    GstBuffer * buffer)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);
  GstV4L2DecMeta *meta;

  GST_DEBUG_OBJECT (pool, "release buffer %p from pool %p", buffer, pool);

  meta = GST_V4L2DEC_META_GET (buffer);
  if (meta == NULL) {
    GST_LOG_OBJECT (pool, "unref copied buffer %p", buffer);
    /* no meta, it was a copied buffer that we can unref */
    gst_buffer_unref (buffer);
    return;
  }

  GST_DEBUG_OBJECT (pool,
      "enqueue buffer %p, index %d in pool %p",
      buffer, meta->vbuffer.index, pool);

  if (pool->buffers[meta->vbuffer.index] != NULL)
    goto already_queued;

  pool->buffers[meta->vbuffer.index] = buffer;
  pool->num_queued++;

  GST_DEBUG_OBJECT (pool, "queue buffer (capture type)");

  if (v4l2_ioctl (pool->video_fd, VIDIOC_QBUF, &meta->vbuffer) < 0)
    goto queue_failed;

  return;

  /* ERRORS */
already_queued:
  {
    GST_WARNING_OBJECT (pool, "the buffer was already queued");
    return;
  }
queue_failed:
  {
    GST_WARNING_OBJECT (pool, "could not queue a buffer %d (%s)", errno,
        g_strerror (errno));
    return;
  }
}

static void
gst_v4l2dec_buffer_pool_flush_start (GstBufferPool * bpool)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);
  GST_DEBUG_OBJECT (pool, "flushing start");

  pool->flushing = TRUE;
}

static void
gst_v4l2dec_buffer_pool_flush_stop (GstBufferPool * bpool)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (bpool);
  GstV4L2DecMeta *meta;
  GstBuffer *buffer;
  guint i;

  GST_DEBUG_OBJECT (pool, "flushing stop");

  /* if this is not a application flush then do nothing */
  if (!pool->flushing)
    return;

  for (i = 0; i < pool->num_buffers; i++) {
    /* Re-enqueue buffers */
    if (pool->buffers[i]) {
      buffer = pool->buffers[i];
      meta = GST_V4L2DEC_META_GET (buffer);
      if (meta == NULL) {
        GST_LOG_OBJECT (pool, "unref copied buffer %p", buffer);
        /* no meta, it was a copied buffer that we can unref */
        gst_buffer_unref (buffer);
        continue;
      }

      GST_DEBUG_OBJECT (pool,
          "enqueue buffer %p, index %d in pool %p",
          buffer, meta->vbuffer.index, pool);

      if (v4l2_ioctl (pool->video_fd, VIDIOC_QBUF, &meta->vbuffer) < 0)
        GST_WARNING_OBJECT (pool, "could not queue a buffer %d (%s)", errno,
            g_strerror (errno));
    }
  }

  pool->flushing = FALSE;
  return;
}

static void
gst_v4l2dec_buffer_pool_finalize (GObject * object)
{
  GstV4L2DecBufferPool *pool = GST_V4L2DEC_BUFFER_POOL (object);

  GST_DEBUG_OBJECT (pool, "finalize pool %p", pool);

  if (pool->video_fd != -1) {
    v4l2_close (pool->video_fd);
    pool->video_fd = -1;
  }
  if (pool->allocator)
    gst_object_unref (pool->allocator);
  g_free (pool->buffers);
  gst_object_unref (pool->dec);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_v4l2dec_buffer_pool_init (GstV4L2DecBufferPool * pool)
{
}

static void
gst_v4l2dec_buffer_pool_class_init (GstV4L2DecBufferPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_v4l2dec_buffer_pool_finalize;

  bufferpool_class->start = gst_v4l2dec_buffer_pool_start;
  bufferpool_class->stop = gst_v4l2dec_buffer_pool_stop;
  bufferpool_class->set_config = gst_v4l2dec_buffer_pool_set_config;
  bufferpool_class->alloc_buffer = gst_v4l2dec_buffer_pool_alloc_buffer;
  bufferpool_class->acquire_buffer = gst_v4l2dec_buffer_pool_acquire_buffer;
  bufferpool_class->release_buffer = gst_v4l2dec_buffer_pool_release_buffer;
  bufferpool_class->free_buffer = gst_v4l2dec_buffer_pool_free_buffer;
  bufferpool_class->flush_start = gst_v4l2dec_buffer_pool_flush_start;
  bufferpool_class->flush_stop = gst_v4l2dec_buffer_pool_flush_stop;
}

/**
 * gst_v4l2dec_buffer_pool_new:
 * @dec: the v4l2 decoder owning the pool
 * @max: maximum buffers in the pool
 * @size: size of the buffer
 *
 * Construct a new buffer pool.
 *
 * Returns: the new pool, use gst_object_unref() to free resources
 */
GstBufferPool *
gst_v4l2dec_buffer_pool_new (GstV4L2Dec * dec, GstCaps * caps, guint max,
    guint size, GstVideoAlignment * align)
{
  GstV4L2DecBufferPool *pool;
  GstStructure *s;
  gint fd;

  GST_DEBUG_OBJECT (dec, "construct a new buffer pool (max buffers %u,"
      "buffer size %u)", max, size);

  fd = v4l2_dup (dec->fd);
  if (fd < 0)
    goto dup_failed;

  pool =
      (GstV4L2DecBufferPool *) g_object_new (GST_TYPE_V4L2DEC_BUFFER_POOL,
      NULL);
  /* take a reference on v4l2dec to be sure that it will be release after the pool */
  pool->dec = gst_object_ref (dec);
  pool->dec = dec;
  pool->video_fd = fd;

  s = gst_buffer_pool_get_config (GST_BUFFER_POOL_CAST (pool));
  gst_buffer_pool_config_set_params (s, caps, size, max, max);
  gst_buffer_pool_config_add_option (s, GST_BUFFER_POOL_OPTION_VIDEO_META);
  gst_buffer_pool_config_add_option (s, GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);
  gst_buffer_pool_config_set_video_alignment (s, align);
  gst_buffer_pool_set_config (GST_BUFFER_POOL_CAST (pool), s);

  return GST_BUFFER_POOL (pool);

  /* ERRORS */
dup_failed:
  {
    GST_DEBUG ("failed to dup fd %d (%s)", errno, g_strerror (errno));
    return NULL;
  }
}
