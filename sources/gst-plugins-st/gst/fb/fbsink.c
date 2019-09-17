#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Object header */
#include "fbsink.h"
#include "fbpool.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gst/video/video.h>

#include <gst/video/gstvideopool.h>

#define MAX_BUFFER 4

#define FB_DEV_DEFAULT_NAME "/dev/fb0"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define gst_fbsink_parent_class parent_class

enum
{
  PROP_0,
  PROP_LOCATION,
  PROP_BUFFER_MODE,
  PROP_BUFFER_SIZE,
  PROP_APPEND,
  PROP_LAST
};

/* utils */

/* TODO use fb.h struc */
struct var_info_to_string
{
  gint R;
  gint G;
  gint B;
  gint A;
  gchar *format;
};

/* supported color formats which depend on offset: {R, G, B, A} */
struct var_info_to_string v2s[] = {
  {16, 8, 0, 24, (gchar *) "RGBA"},
  {0, 8, 16, 24, (gchar *) "BGRA"},
  {0, 8, 16, 0, (gchar *) "xBGR"},
  {16, 8, 0, 0, (gchar *) "RGBA"},
};

static GstStaticPadTemplate fb_sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE ("{RGBA, BGRA , xBGR, xRGB}"))
    );

#define gst_fbsink_parent_class parent_class
G_DEFINE_TYPE (GstFbSink, gst_fbsink, GST_TYPE_VIDEO_SINK);

GST_DEBUG_CATEGORY (gst_debug_fb_sink);
#define GST_CAT_DEFAULT gst_debug_fb_sink

/* CAPS Negociation Here */
static GstCaps *
gst_fbsink_getcaps (GstBaseSink * bsink, GstCaps * filter)
{
  GstFbSink *fbsink;
  GstCaps *caps = NULL;
  fbsink = GST_FB_SINK (bsink);

  if (fbsink->caps) {
    caps = gst_caps_ref (fbsink->caps);
  } else
    caps = gst_pad_get_pad_template_caps (GST_BASE_SINK (fbsink)->sinkpad);

  if (filter) {
    GstCaps *intersection;
    /* We do the caps negociation here */
    intersection =
        gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
    caps = intersection;
  }
  GST_DEBUG_OBJECT (fbsink, "(getcaps) caps %" GST_PTR_FORMAT, caps);

  return caps;
}

static gboolean
gst_fbsink_setcaps (GstBaseSink * bsink, GstCaps * caps)
{
  GstFbSink *fbsink;
  GstVideoInfo info;
  GstBufferPool *newpool;
  GstBufferPool *oldpool;

  fbsink = GST_FB_SINK (bsink);
  GST_DEBUG_OBJECT (fbsink,
      "(setcaps) sinkconnect possible caps %" GST_PTR_FORMAT
      " with given caps %" GST_PTR_FORMAT, fbsink->caps, caps);

  /* We intersect those caps with our template to make sure they are correct */
  if (!gst_caps_can_intersect (fbsink->caps, caps))
    goto incompatible_caps;

  if (!gst_video_info_from_caps (&info, caps))
    goto invalid_format;

  fbsink->info = info;

  /* pool must be created here */
  newpool =
      (GstBufferPool *) gst_fb_buffer_pool_new (caps, fbsink, fbsink->size);

  GST_DEBUG_OBJECT (fbsink,
      "(setcaps) create new pool %" GST_PTR_FORMAT
      " with given caps %" GST_PTR_FORMAT, newpool, fbsink->caps);

  oldpool = fbsink->pool;
  /* we don't activate the pool yet, this will be done by downstream after it
   * has configured the pool. If downstream does not want our pool we will
   * activate it when we render into it */
  fbsink->pool = newpool;

  /* unref the old sink */
  if (oldpool) {
    /* we don't deactivate, some elements might still be using it, it will be
     * deactivated when the last ref is gone */
    gst_object_unref (oldpool);
  }
  return TRUE;
  /* ERRORS */
incompatible_caps:
  {
    GST_ERROR_OBJECT (fbsink, "caps incompatible");
    return FALSE;
  }
invalid_format:
  {
    GST_ERROR_OBJECT (fbsink, "caps invalid");
    return FALSE;
  }
}

static gboolean
gst_fbsink_propose_allocation (GstBaseSink * bsink, GstQuery * query)
{
  GstFbSink *fbsink = GST_FB_SINK (bsink);
  GstCaps *caps;
  gboolean need_pool;

  /* Management of the bufferpool with gst_query
     Here, we receive the caps supported by the source and if it needs a pool */
  gst_query_parse_allocation (query, &caps, &need_pool);

  if (!caps)
    goto no_caps;

  if (fbsink->pool) {
    GstCaps *pcaps;
    GstStructure *config;
    guint size;

    /* we had a pool, check caps */
    config = gst_buffer_pool_get_config (fbsink->pool);
    gst_buffer_pool_config_get_params (config, &pcaps, &size, NULL, NULL);
    gst_structure_free (config);

    if (!gst_caps_is_equal (caps, pcaps)) {
      /* different caps, we can't use this pool */
      GST_DEBUG_OBJECT (fbsink, "pool has different caps");
      return FALSE;
    }

    /* we need at least 4 buffer. We answer to the src query */
    gst_query_add_allocation_pool (query, fbsink->pool, size,
        1, fbsink->max_buffers);
  }
  return TRUE;

  /* ERRORS */
no_caps:
  {
    GST_DEBUG_OBJECT (bsink, "no caps specified");
    return FALSE;
  }
}

/*Initialization of Framebuffer */
static GstFlowReturn
init_fb (GstFbSink * fbsink)
{
  gint i = 0;
  gchar *format = (gchar *) "RGBA";

  /* Open the device on /dev/fb0 */
  fbsink->fd = open (fbsink->device_name, O_RDWR);

  if (fbsink->fd == -1) {
    goto error_file_opening;
  }

  /* get variable info from the framebuffer */
  if (ioctl (fbsink->fd, FBIOGET_VSCREENINFO,
          &fbsink->fb_varinfo) == (gint) - 1) {
    goto error_ioctl_varinfo;
  }
  /* get fixed info from the framebuffer */
  if (ioctl (fbsink->fd, FBIOGET_FSCREENINFO, &fbsink->fb_fixinfo) == -1) {
    goto error_ioctl_fixinfo;
  }

  /* set the number of buffers we need : yres_virtual / yres */
  fbsink->num_buffers =
      fbsink->fb_varinfo.yres_virtual / fbsink->fb_varinfo.yres;
  /* set our buffers mapsize */
  fbsink->mapsize =
      fbsink->fb_fixinfo.line_length * fbsink->fb_varinfo.yres_virtual;
  /* set the size of one buffer */
  fbsink->size = fbsink->fb_fixinfo.line_length * fbsink->fb_varinfo.yres;

  GST_DEBUG_OBJECT (fbsink, "(init_fb) mapsize: %i", fbsink->mapsize);
  GST_DEBUG_OBJECT (fbsink, "(init_fb) size: %i", fbsink->size);

  fbsink->data = mmap (NULL, fbsink->mapsize, PROT_READ | PROT_WRITE,
      MAP_SHARED, fbsink->fd, 0);

  if (!fbsink->data)
    goto error_mmap;

  if (fbsink->caps)
    gst_caps_unref (fbsink->caps);

  GST_DEBUG_OBJECT (fbsink, "(init_fb) num_buffers: %i", fbsink->num_buffers);
  GST_DEBUG_OBJECT (fbsink,
      "(init_fb) RED offset: %i", fbsink->fb_varinfo.red.offset);
  GST_DEBUG_OBJECT (fbsink,
      "(init_fb) GREEN offset: %i", fbsink->fb_varinfo.green.offset);
  GST_DEBUG_OBJECT (fbsink,
      "(init_fb) BLUE offset: %i", fbsink->fb_varinfo.blue.offset);
  GST_DEBUG_OBJECT (fbsink,
      "(init_fb) TRANS offset: %i", fbsink->fb_varinfo.transp.offset);

  for (i = 0; i < ARRAY_SIZE (v2s); i++) {
    /* G always in the middle. so unecessary tested
       then R and B always opposed. sufficient to test one of them */
    if ((fbsink->fb_varinfo.red.offset == v2s[i].R) &&
        (fbsink->fb_varinfo.transp.offset == v2s[i].A)) {
      format = v2s[i].format;
    }
  }

  /* set our caps with the framebuffer info */
  fbsink->caps = gst_caps_new_simple ("video/x-raw",
      "format", G_TYPE_STRING, format,
      "width", G_TYPE_INT, fbsink->fb_varinfo.xres,
      "height", G_TYPE_INT, fbsink->fb_varinfo.yres,
      "framerate", GST_TYPE_FRACTION_RANGE, 0, 1, G_MAXINT, 1, NULL);

  return GST_FLOW_OK;

  /* ERRORS */
error_file_opening:
  {
    GST_ERROR_OBJECT (fbsink, "failed to open file descriptor.");
    return GST_FLOW_ERROR;
  }
error_ioctl_varinfo:
  {
    GST_ERROR_OBJECT (fbsink, "failed to get varinfo in IOCTL.");
    close (fbsink->fd);
    return GST_FLOW_ERROR;
  }
error_ioctl_fixinfo:
  {
    GST_ERROR_OBJECT (fbsink, "failed to get fixinfo in IOCTL.");
    close (fbsink->fd);
    return GST_FLOW_ERROR;
  }
error_mmap:
  {
    GST_ERROR_OBJECT (fbsink, "error during mmap.");
    close (fbsink->fd);
    return GST_FLOW_ERROR;
  }
}

/* Called for each new frame available for displaying */
static GstFlowReturn
gst_fbsink_show_frame (GstVideoSink * sink, GstBuffer * buf)
{
  GstFbSink *fbsink = GST_FB_SINK (sink);
  GstBufferPool *newpool;
  GstVideoFrame src, dest;
  GstFlowReturn res;
  GstBuffer *to_put;
  GstBufferPoolAcquireParams params = { 0, };
  GstFbMeta *meta;


  if (!fbsink->pool) {
    newpool =
        (GstBufferPool *) gst_fb_buffer_pool_new (fbsink->caps, fbsink,
        fbsink->size);
    fbsink->pool = newpool;
    GST_DEBUG_OBJECT (fbsink, "(show_frame) No pool => Created new pool");
  }

  /* If the buffer come from our Pool */
  if (fbsink->pool == buf->pool) {
    meta = GST_FB_META_GET (buf);
    if (!meta) {
      GST_LOG_OBJECT (fbsink, "unref copied buffer %p", buf);
      /* no meta, it was a copied buffer that we can unref */
      return GST_FLOW_OK;
    }
    /* permit us to navigate with PAN */
    fbsink->fb_varinfo.yoffset = meta->yres;
    goto display_pan;
  }

  /* if no Pool */
  if (!gst_buffer_pool_set_active (fbsink->pool, TRUE))
    goto activate_failed;

  params.flags = GST_BUFFER_POOL_ACQUIRE_FLAG_DONTWAIT;
  res = gst_buffer_pool_acquire_buffer (fbsink->pool, &to_put, &params);

  if (res != GST_FLOW_OK)
    goto no_buffer;
  /* src is pointer on source buffer */
  if (!gst_video_frame_map (&src, &fbsink->info, buf, GST_MAP_READ))
    goto invalid_buffer;
  /* dest is pointer on sink buffer */
  if (!gst_video_frame_map (&dest, &fbsink->info, fbsink->framebuffer,
          GST_MAP_WRITE)) {
    gst_video_frame_unmap (&src);
    goto invalid_buffer;
  }
  /* buf ==> fbsink->framebuffer */
  gst_video_frame_copy (&dest, &src);
  GST_DEBUG_OBJECT (fbsink, "(show_frame) FRAMECOPY MODE");
  gst_video_frame_unmap (&dest);
  gst_video_frame_unmap (&src);

/* common action */
display_pan:
  ioctl (fbsink->fd, FBIOPAN_DISPLAY, &fbsink->fb_varinfo);
  return GST_FLOW_OK;

/* ERRORS */
activate_failed:
  {
    GST_ERROR_OBJECT (fbsink, "failed to activate bufferpool.");
    return GST_FLOW_ERROR;
  }
no_buffer:
  {
    /* No image available. That's very bad ! */
    GST_WARNING_OBJECT (fbsink, "could not create image");
    return GST_FLOW_OK;
  }
invalid_buffer:
  {
    GST_WARNING_OBJECT (fbsink, "invalid buffer");
    return GST_FLOW_ERROR;
  }
}

/* What we do when pipeline state changes */
static GstStateChangeReturn
gst_fbsink_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstFbSink *fbsink;
  GstFlowReturn res = GST_FLOW_OK;

  fbsink = GST_FB_SINK (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      /* Initializing the Frame buffer */
      res = init_fb (fbsink);

      if (res != GST_FLOW_OK)
        goto error_init_fb;
      break;

    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;

    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      break;

    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      break;

    case GST_STATE_CHANGE_PAUSED_TO_READY:
      if (fbsink->pool)
        gst_buffer_pool_set_active (fbsink->pool, FALSE);
      break;

    case GST_STATE_CHANGE_READY_TO_NULL:
      munmap (fbsink->data, fbsink->mapsize);
      close (fbsink->fd);
      break;

    default:
      break;
  }

  return ret;
  /*Errors */
error_init_fb:
  {
    return GST_STATE_CHANGE_FAILURE;
  }
}

static gboolean
gst_fb_sink_set_device (GstFbSink * fbsink, const gchar * device,
    GError ** error)
{
  /* we store the filename as we received it from the application. On Windows
   * this should be in UTF8 */
  g_free (fbsink->device_name);
  fbsink->device_name = g_strdup (device);
  GST_INFO ("device_name : %s", fbsink->device_name);
  return TRUE;
}

static void
gst_fb_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstFbSink *fbsink = GST_FB_SINK (object);
  switch (prop_id) {
    case PROP_LOCATION:
      gst_fb_sink_set_device (fbsink, g_value_get_string (value), NULL);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_fb_sink_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstFbSink *fbsink = GST_FB_SINK (object);
  switch (prop_id) {
    case PROP_LOCATION:
      g_value_set_string (value, fbsink->device_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* Init our fbsink element */
static void
gst_fbsink_init (GstFbSink * fbsink)
{
  fbsink->device_name = g_strdup (FB_DEV_DEFAULT_NAME);
  fbsink->fd = -1;
  fbsink->pool = NULL;
  fbsink->mapsize = 0;
  fbsink->size = 0;
  fbsink->num_buffers = 0;
  fbsink->max_buffers = MAX_BUFFER;
}

static void
gst_fbsink_class_init (GstFbSinkClass * klass)
{
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;
  GstVideoSinkClass *videosink_class;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;
  videosink_class = (GstVideoSinkClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = gst_fb_sink_set_property;
  gobject_class->get_property = gst_fb_sink_get_property;

  g_object_class_install_property (gobject_class, PROP_LOCATION,
      g_param_spec_string ("location", "device Location",
          "Location of the device to write", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (gstelement_class,
      "FB sink", "Sink/Video",
      "A standard FB based videosink", "STMicroelectronics");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&fb_sink_template));

  gstelement_class->change_state = gst_fbsink_change_state;

  gstbasesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_fbsink_getcaps);
  gstbasesink_class->set_caps = GST_DEBUG_FUNCPTR (gst_fbsink_setcaps);
  videosink_class->show_frame = GST_DEBUG_FUNCPTR (gst_fbsink_show_frame);
  gstbasesink_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_fbsink_propose_allocation);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  gboolean ret;

  GST_DEBUG_CATEGORY_INIT (gst_debug_fb_sink, "fbsink", 0,
      "fbsink element debug");

  ret = gst_element_register (plugin, "fbsink", GST_RANK_NONE,
      GST_TYPE_FB_SINK);

  return ret;
}


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    fbsink,
    "framebuffer sink",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
