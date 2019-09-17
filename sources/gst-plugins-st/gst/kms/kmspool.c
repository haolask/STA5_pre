/* GStreamer DRM/KMS video sink plugin
 *
 * Copyright (C) 2015 STMicroelectronics SA
 *
 * Author: Benjamin Gaignard <benjamin.gaignard@st.com> for STMicroelectronics.
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
#include "config.h"
#endif

#include <string.h>
#include <fcntl.h>

#include "kmspool.h"
#include "gst/allocators/gstdmabuf.h"

GST_DEBUG_CATEGORY_EXTERN (gst_debug_kms_pool);
#define GST_CAT_DEFAULT gst_debug_kms_pool

#define gst_kms_pool_parent_class parent_class
G_DEFINE_TYPE (GstKMSPool, gst_kms_pool, GST_TYPE_BUFFER_POOL);

GType
gst_kms_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { "memory", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("GstKMSMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

const GstMetaInfo *
gst_kms_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_kms_meta_api_get_type (), "GstKMSMeta",
        sizeof (GstKMSMeta), (GstMetaInitFunction) NULL,
        (GstMetaFreeFunction) NULL, (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

guint
format_gst_to_fourcc (GstVideoFormat gst_format)
{
  switch (gst_format) {
    case GST_VIDEO_FORMAT_NV12:
      return DRM_FORMAT_NV12;
    case GST_VIDEO_FORMAT_BGRA:
      return DRM_FORMAT_ARGB8888;
    case GST_VIDEO_FORMAT_BGR:
      return DRM_FORMAT_RGB888;
    case GST_VIDEO_FORMAT_RGB16:
      return DRM_FORMAT_RGB565;
    case GST_VIDEO_FORMAT_BGRx:
    default:
      return DRM_FORMAT_XRGB8888;
  }
}

static GstFlowReturn
gst_kms_pool_alloc_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  struct kms_bo *bo;
  GstKMSPool *pool = GST_KMS_POOL (bpool);
  GstKMSSink *kmssink = pool->kmssink;
  unsigned attrs[7] = {
    KMS_WIDTH, 0,
    KMS_HEIGHT, 0,
    KMS_BO_TYPE, KMS_BO_TYPE_SCANOUT_X8R8G8B8,
    KMS_TERMINATE_PROP_LIST,
  };
  gint ret;
  guint handle, i;
  GstBuffer *newbuf;
  GstKMSMeta *meta;
  gint prime_fd;

  GST_DEBUG_OBJECT (pool, "KMS pool allocate buffer of size %" G_GSIZE_FORMAT " (%dx%d)",
      GST_VIDEO_INFO_SIZE (&kmssink->info),
      GST_VIDEO_INFO_WIDTH (&kmssink->info), GST_VIDEO_INFO_HEIGHT (&kmssink->info));

  switch (kmssink->format) {
    case GST_VIDEO_FORMAT_NV12:
      attrs[1] = GST_VIDEO_INFO_PLANE_STRIDE (&kmssink->info, 0);
      break;
    case GST_VIDEO_FORMAT_BGRx:
    case GST_VIDEO_FORMAT_BGRA:
      attrs[1] = GST_VIDEO_INFO_PLANE_STRIDE (&kmssink->info, 0) / 4;
      break;
    case GST_VIDEO_FORMAT_BGR:
      attrs[1] = GST_VIDEO_INFO_PLANE_STRIDE (&kmssink->info, 0) / 3;
      break;
    case GST_VIDEO_FORMAT_RGB16:
      attrs[1] = GST_VIDEO_INFO_PLANE_STRIDE (&kmssink->info, 0) / 2;
      break;
    default:
      GST_WARNING_OBJECT (pool, "Unknown format: %d", kmssink->format);
      attrs[1] = GST_VIDEO_INFO_WIDTH (&kmssink->info);
      break;
  }
  /*
   * Allocate buffer according to its info.size, not according to the video size
   * which can be smaller (decoder size alignment). Note that the kms interface allocates
   * memory on a 32 bpp basis
   */
  attrs[3] = GST_VIDEO_INFO_SIZE (&kmssink->info) / attrs[1] / 4;
  attrs[3] = GST_ROUND_UP_4 (attrs[3]);
  if ((attrs[1] * attrs[3] * 4) < GST_VIDEO_INFO_SIZE (&kmssink->info))
    GST_ERROR_OBJECT (pool, "Not allocating enough memory");

  ret = kms_bo_create (kmssink->kms, attrs, &bo);
  if (ret < 0) {
    GST_DEBUG_OBJECT (pool, "kms_bo_create failed %s (%d)", strerror (errno),
        errno);
    return GST_FLOW_ERROR;
  }

  ret = kms_bo_get_prop (bo, KMS_HANDLE, &handle);
  if (ret < 0) {
    GST_DEBUG_OBJECT (pool, "kms_bo_get_prop KMS_HANDLE failed");
    return GST_FLOW_ERROR;
  }

  ret = drmPrimeHandleToFD (kmssink->drm, handle, DRM_CLOEXEC | O_RDWR, &prime_fd);
  if (ret < 0) {
    /* fallback attempt for old kernel */
    ret = drmPrimeHandleToFD (kmssink->drm, handle, DRM_CLOEXEC, &prime_fd);
    if (ret < 0) {
      GST_DEBUG_OBJECT (pool, "drmPrimeHandleToFD failed");
      return GST_FLOW_ERROR;
     }
  }

  newbuf = gst_buffer_new ();
  meta = GST_KMS_META_ADD (newbuf);
  if (!meta) {
    gst_buffer_unref (newbuf);
    GST_DEBUG_OBJECT (pool, "GST_KMS_META_ADD failed");
    return GST_FLOW_ERROR;
  }

  meta->fb_id = -1;
  meta->bo = bo;
  for (i = 0; i < 4; i++) {
    meta->handle[i] = handle;
    meta->stride[i] = GST_VIDEO_INFO_PLANE_STRIDE (&kmssink->info, i);
    meta->offset[i] = GST_VIDEO_INFO_PLANE_OFFSET (&kmssink->info, i);
  }

  meta->fourcc = format_gst_to_fourcc (kmssink->format);

  gst_buffer_append_memory (newbuf,
      gst_dmabuf_allocator_alloc (pool->allocator, prime_fd,
          GST_VIDEO_INFO_SIZE (&kmssink->info)));
  *buffer = newbuf;

  GST_DEBUG_OBJECT (pool,
      "buf %p meta %p meta->handle %d meta->fb_id %d meta->stride %d",
      *buffer, meta, meta->handle[0], meta->fb_id, meta->stride[0]);

  return GST_FLOW_OK;
}

static void
gst_kms_pool_free_buffer (GstBufferPool * bpool, GstBuffer * buffer)
{
  GstKMSPool *pool = GST_KMS_POOL (bpool);
  GstKMSMeta *meta = GST_KMS_META_GET (buffer);

  GST_DEBUG_OBJECT (pool, "KMS pool free buffer");
  kms_bo_destroy (&meta->bo);
  GST_BUFFER_POOL_CLASS (parent_class)->free_buffer (bpool, buffer);
}

static gboolean
gst_kms_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{

  GstKMSPool *pool = GST_KMS_POOL (bpool);
  GstCaps *caps;
  guint size, min_buffers, max_buffers;
  GstAllocator *allocator;
  GstVideoInfo info;
  gboolean has_alignment = FALSE;
  GstVideoAlignment video_align;

  /* parse the config and keep around */
  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
          &max_buffers))
    goto wrong_config;

  /* now parse the caps from the config */
  if (!gst_video_info_from_caps (&info, caps))
    goto wrong_caps;

  /* parse extra alignment info */
  has_alignment = gst_buffer_pool_config_has_option (config, GST_BUFFER_POOL_OPTION_VIDEO_META);
  if (has_alignment) {
    /* get and apply the alignment to info */
    if (gst_buffer_pool_config_get_video_alignment (config, &video_align)) {
      gst_video_info_align (&info, &video_align);
      GST_LOG_OBJECT (pool, "padding %u-%ux%u-%u",
                      video_align.padding_top, video_align.padding_left,
                      video_align.padding_right, video_align.padding_bottom);
    }
  }
  /* Update video info */
  pool->kmssink->info = info;

  allocator = gst_dmabuf_allocator_new ();

  if (pool->allocator)
    gst_object_unref (pool->allocator);
  if ((pool->allocator = allocator))
    gst_object_ref (allocator);

  gst_buffer_pool_config_set_params (config, caps, size, min_buffers,
      max_buffers);

  return GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);

  /* ERRORS */
wrong_config:
  GST_ERROR_OBJECT (pool, "invalid config %" GST_PTR_FORMAT, config);
  return FALSE;

wrong_caps:
  GST_WARNING_OBJECT (pool, "failed getting geometry from caps %" GST_PTR_FORMAT, caps);
  return FALSE;
}

static void
gst_kms_pool_finalize (GObject * object)
{
  GstKMSPool *pool = GST_KMS_POOL (object);

  if (pool->allocator)
    gst_object_unref (pool->allocator);

  gst_object_unref (pool->kmssink);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_kms_pool_init (GstKMSPool * pool)
{
}

static void
gst_kms_pool_class_init (GstKMSPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_kms_pool_finalize;

  bufferpool_class->set_config = gst_kms_pool_set_config;
  bufferpool_class->alloc_buffer = gst_kms_pool_alloc_buffer;
  bufferpool_class->free_buffer = gst_kms_pool_free_buffer;
}

GstBufferPool *
gst_kms_pool_new (GstCaps * caps, GstKMSSink * kmssink, guint max)
{
  GstStructure *config;
  GstVideoInfo info;
  GstKMSPool *pool = (GstKMSPool *) g_object_new (GST_TYPE_KMS_POOL, NULL);
  /* take a reference on sink to be sure that it will
   * be release after the pool */
  pool->kmssink = gst_object_ref (kmssink);

  config = gst_buffer_pool_get_config (GST_BUFFER_POOL_CAST (pool));
  gst_video_info_from_caps (&info, caps);

  gst_buffer_pool_config_set_params (config, caps, GST_VIDEO_INFO_SIZE (&info),
      1, max);

  if (!gst_buffer_pool_set_config (GST_BUFFER_POOL_CAST (pool), config))
    goto config_failed;

  return GST_BUFFER_POOL (pool);

/* ERRORS */
config_failed:
  {
    GST_ERROR_OBJECT (pool, "failed setting config");
    gst_object_unref (pool);
    return GST_BUFFER_POOL (pool);
  }
}
