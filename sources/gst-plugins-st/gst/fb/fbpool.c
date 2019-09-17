#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "gst/video/video.h"
#include "gst/video/gstvideopool.h"

#include "fbpool.h"

/* Debugging category */
#include <gst/gstinfo.h>

/*
 * GstFbBufferPool:
 */
#define gst_fb_buffer_pool_parent_class parent_class
G_DEFINE_TYPE (GstFbBufferPool, gst_fb_buffer_pool, GST_TYPE_BUFFER_POOL);

GST_DEBUG_CATEGORY_EXTERN (gst_debug_fb_sink);
#define GST_CAT_DEFAULT gst_debug_fb_sink

GType
gst_fb_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { "coordinates", NULL };

  if (g_once_init_enter (&type)) {
    GType _type = gst_meta_api_type_register ("GstFbMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

const GstMetaInfo *
gst_fb_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_fb_meta_api_get_type (), "GstFbMeta",
        sizeof (GstFbMeta), (GstMetaInitFunction) NULL,
        (GstMetaFreeFunction) NULL, (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

/* Buffer Alloc*/
static GstFlowReturn
gst_fb_buffer_pool_alloc_buffer (GstBufferPool * bpool, GstBuffer ** buffer,
    GstBufferPoolAcquireParams * params)
{
  GstFbBufferPool *pool = GST_FB_BUFFER_POOL (bpool);
  GstBuffer *newbuf;
  GstFbMeta *meta;
  guint8 *data;
  /* where the buffer begins and ends in memory space */
  data = ((guint8 *) pool->fbsink->data) + (pool->num_allocated * pool->size);

  /* created a new buffer */
  newbuf = gst_buffer_new_wrapped_full (0, data,
      pool->size, 0, pool->size, NULL, NULL);

  meta = GST_FB_META_ADD (newbuf);

  /* Pan of the current buffer */
  meta->yres = pool->num_allocated * pool->fbsink->fb_varinfo.yres;

  pool->num_allocated++;
  *buffer = newbuf;
  GST_DEBUG_OBJECT (pool, "(alloc_buffer) buffer : %" GST_PTR_FORMAT, *buffer);

  return GST_FLOW_OK;
}


static gboolean
gst_fb_buffer_pool_set_config (GstBufferPool * bpool, GstStructure * config)
{

  GstFbBufferPool *pool = GST_FB_BUFFER_POOL (bpool);
  GstCaps *caps;
  guint size, min_buffers, max_buffers;

  GstAllocationParams params = { 0, 0, 0, 0 };

  /* parse the config and keep around */
  if (!gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
          &max_buffers))
    goto wrong_config;

  pool->size = size;
  pool->num_buffers = pool->fbsink->num_buffers;
  pool->min_buffers = pool->fbsink->num_buffers;
  pool->max_buffers = pool->fbsink->num_buffers;
  pool->params = params;

  gst_buffer_pool_config_set_params (config, caps, size, min_buffers,
      max_buffers);

  GST_DEBUG_OBJECT (pool, "(set_config) Config : %" GST_PTR_FORMAT, config);

  return GST_BUFFER_POOL_CLASS (parent_class)->set_config (bpool, config);

  /* ERRORS */
wrong_config:
  {
    GST_ERROR_OBJECT (pool, "invalid config %" GST_PTR_FORMAT, config);
    return FALSE;
  }
}

/* Finalize pool */
static void
gst_fb_buffer_pool_finalize (GObject * object)
{
  GstFbBufferPool *pool = GST_FB_BUFFER_POOL (object);

  gst_object_unref (pool->fbsink);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_fb_buffer_pool_init (GstFbBufferPool * pool)
{
}

static void
gst_fb_buffer_pool_class_init (GstFbBufferPoolClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstBufferPoolClass *bufferpool_class = GST_BUFFER_POOL_CLASS (klass);

  object_class->finalize = gst_fb_buffer_pool_finalize;

  bufferpool_class->set_config = gst_fb_buffer_pool_set_config;
  bufferpool_class->alloc_buffer = gst_fb_buffer_pool_alloc_buffer;
}

/* Pool new */
GstBufferPool *
gst_fb_buffer_pool_new (GstCaps * caps, GstFbSink * fbsink, guint size)
{
  GstStructure *config;
  GstFbBufferPool *pool;
  /* new bufferPool object */
  pool = (GstFbBufferPool *) g_object_new (GST_TYPE_FB_BUFFER_POOL, NULL);

  GST_DEBUG_OBJECT (pool, "pool : %" GST_PTR_FORMAT, pool);

  pool->fbsink = gst_object_ref (fbsink);
  pool->num_buffers = pool->fbsink->num_buffers;

  g_return_val_if_fail (GST_IS_FB_BUFFER_POOL (pool), NULL);
  g_return_val_if_fail (GST_IS_BUFFER_POOL (pool), NULL);

  config = gst_buffer_pool_get_config (GST_BUFFER_POOL_CAST (pool));

  gst_buffer_pool_config_set_params (config, caps, size,
      pool->num_buffers, pool->num_buffers);

  /* no buffers at this moment */
  pool->num_allocated = 0;

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
