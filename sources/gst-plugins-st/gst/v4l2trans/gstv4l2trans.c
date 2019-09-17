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
 * Note that the variables and structures follow the V4L2 naming convention:
 *  - "Output" refers to data & control from the upstream element.
 *  - "Capture" refers to data & control to the downstream element.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <gst/allocators/gstdmabuf.h>

#include "gstv4l2trans.h"
#include "gstv4l2capbufpool.h"
#include "gstv4l2outbufpool.h"

GST_DEBUG_CATEGORY (gst_v4l2trans_debug);
#define GST_CAT_DEFAULT gst_v4l2trans_debug

#define MAX_DEVICES          20 /* Max nb of candidate V4L2 M2M devices */
#define MIN_BUF_OUTPUT_POOL   2 /* Min number of buffers in the output buffer
                                   pool we own */

/* Preference-ordered video formats */
#define GST_VIDEO_TRANS_FORMATS \
    "{ NV12, NV21, NV16, NV61, " \
    "BGRx, BGRA, RGBx, RGBA, xBGR, ABGR, xRGB, ARGB, RGB, BGR, RGB16, BGR16, " \
    "YUY2, YVYU, UYVY, AYUV, YUV9, YVU9, Y41B, I420, YV12, Y42B, v308 }"

static GstStaticCaps gst_v4l2trans_src_format_caps =
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES
    (GST_CAPS_FEATURE_MEMORY_DMABUF,
        GST_VIDEO_TRANS_FORMATS)
    ";" GST_VIDEO_CAPS_MAKE (GST_VIDEO_TRANS_FORMATS)
    );

static GstStaticCaps gst_v4l2trans_sink_format_caps =
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES
    (GST_CAPS_FEATURE_MEMORY_DMABUF,
        GST_VIDEO_TRANS_FORMATS)
    ";" GST_VIDEO_CAPS_MAKE (GST_VIDEO_TRANS_FORMATS)
    );

#define parent_class gst_v4l2trans_parent_class
G_DEFINE_TYPE (GstV4L2Trans, gst_v4l2trans, GST_TYPE_VIDEO_FILTER);

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

gboolean plugin_init (GstPlugin * plugin);

static void gst_v4l2trans_finalize (GObject * object);
void gst_v4l2trans_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec);
void gst_v4l2trans_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec);
static GstVideoFormat gst_v4l2trans_fmt_v4l2_to_gst (gint fmt);

enum
{
  PROP_0,
  PROP_HFLIP,
  PROP_VFLIP,
  PROP_BRIGHTNESS,
  PROP_CONTRAST,
  PROP_SATURATION,
  PROP_ALPHA,
  PROP_CROP_LEFT,
  PROP_CROP_RIGHT,
  PROP_CROP_TOP,
  PROP_CROP_BOTTOM
};


GstVideoInterlaceMode
gst_interlace_mode_from_string (const gchar * mode)
{
  gint i;
  const gchar *interlace_mode[] = {
    "progressive",
    "interleaved",
    "mixed",
    "fields"
  };

  for (i = 0; i < G_N_ELEMENTS (interlace_mode); i++) {
    if (g_str_equal (interlace_mode[i], mode))
      return i;
  }
  return GST_VIDEO_INTERLACE_MODE_PROGRESSIVE;
}

static GstCaps *
gst_v4l2trans_get_src_capslist (void)
{
  static GstCaps *caps = NULL;
  static volatile gsize inited = 0;

  if (g_once_init_enter (&inited)) {
    caps = gst_static_caps_get (&gst_v4l2trans_src_format_caps);
    g_once_init_leave (&inited, 1);
  }
  return caps;
}

static GstCaps *
gst_v4l2trans_get_sink_capslist (void)
{
  static GstCaps *caps = NULL;
  static volatile gsize inited = 0;

  if (g_once_init_enter (&inited)) {
    caps = gst_static_caps_get (&gst_v4l2trans_sink_format_caps);
    g_once_init_leave (&inited, 1);
  }
  return caps;
}

static GstPadTemplate *
gst_v4l2trans_src_template_factory (void)
{
  return gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
      gst_v4l2trans_get_src_capslist ());
}

static GstPadTemplate *
gst_v4l2trans_sink_template_factory (void)
{
  return gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
      gst_v4l2trans_get_sink_capslist ());
}

static void
gst_v4l2trans_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_v4l2trans_set_probed_caps (GstV4L2Trans * trans, enum v4l2_buf_type type)
{
  GValue value = G_VALUE_INIT, formats = G_VALUE_INIT;
  struct v4l2_fmtdesc enum_fmt;
  GstVideoFormat format;
  GstStaticCaps trans_caps =
      GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS_ALL));
  GstCaps *caps, *prefcaps, *tmp;
  guint i, num_structures;
  GstCapsFeatures *f;
  GstStructure *alt, *structure;

  g_value_init (&value, G_TYPE_STRING);
  g_value_init (&formats, GST_TYPE_LIST);

  /* Get the driver supported formats */
  enum_fmt.type = type;
  for (enum_fmt.index = 0;; enum_fmt.index++) {
    if (v4l2_ioctl (trans->fd, VIDIOC_ENUM_FMT, &enum_fmt) < 0) {
      if (errno != EINVAL) {
        GST_ERROR_OBJECT (trans, "enum_fmt failed");
        return FALSE;
      }

      break;                    /* end of enumeration */
    }
    format = gst_v4l2trans_fmt_v4l2_to_gst (enum_fmt.pixelformat);
    if (format) {
      g_value_set_string (&value, gst_video_format_to_string (format));
      gst_value_list_append_value (&formats, &value);
    }
  }

  /* Build caps from the format list */
  caps = gst_static_caps_get (&trans_caps);
  caps = gst_caps_make_writable (caps);
  gst_structure_set_value (gst_caps_get_structure (caps, 0), "format",
      &formats);
  g_value_unset (&value);
  g_value_unset (&formats);

  /* Order formats according to the preferred list */
  prefcaps = (type == V4L2_BUF_TYPE_VIDEO_CAPTURE) ?
      gst_v4l2trans_get_src_capslist () : gst_v4l2trans_get_sink_capslist ();
  tmp = caps;
  caps = gst_caps_intersect_full (prefcaps, tmp, GST_CAPS_INTERSECT_FIRST);
  gst_caps_unref (tmp);

  /* Each format is supported both for SystemMemory and DMABuf caps features */
  num_structures = gst_caps_get_size (caps);
  for (i = 0; i < num_structures; i++) {
    structure = gst_caps_get_structure (caps, i);
    alt = gst_structure_copy (structure);
    gst_caps_append_structure (caps, alt);
    f = gst_caps_get_features (caps, i);
    gst_caps_features_add (f, GST_CAPS_FEATURE_MEMORY_DMABUF);
    gst_caps_features_remove (f, GST_CAPS_FEATURE_MEMORY_SYSTEM_MEMORY);
  }

  /* Store caps */
  if (type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
    trans->capture_info.probed_caps = caps;
  else
    trans->output_info.probed_caps = caps;

  GST_DEBUG_OBJECT (trans, "probed %s caps: %" GST_PTR_FORMAT,
      type == V4L2_BUF_TYPE_VIDEO_CAPTURE ? "src" : "sink", caps);

  return TRUE;
}

static gboolean
gst_v4l2trans_open_device (GstV4L2Trans * trans)
{
  gint ret, libv4l2_fd, devnum, fd;
  gchar device_name[100];
  struct v4l2_capability cap;
  gboolean found = FALSE;

  if (trans->fd > 0)
    return TRUE;

  /* Search for a mem2mem device supporting ARGB capture & output */
  for (devnum = 0; devnum < MAX_DEVICES; devnum++) {
    snprintf (device_name, sizeof device_name, "/dev/video%d", devnum);
    fd = open (device_name, O_RDWR, 0);
    if (fd < 0)
      continue;

    libv4l2_fd = v4l2_fd_open (fd, V4L2_DISABLE_CONVERSION);
    if (libv4l2_fd != -1)
      fd = libv4l2_fd;

    ret = v4l2_ioctl (fd, VIDIOC_QUERYCAP, &cap);
    if (ret != 0) {
      v4l2_close (fd);
      continue;
    }

    /* select first driver which is M2M compatible */
    if ((cap.capabilities & V4L2_CAP_VIDEO_M2M)) {
      found = TRUE;
      goto device_found;
    }
    v4l2_close (fd);
  }

  if (!found) {
    GST_ERROR_OBJECT (trans, "No device found");
    return FALSE;
  }

device_found:
  trans->fd = fd;
  GST_INFO_OBJECT (trans, "Will use %s", device_name);

  /* Get caps from driver */
  if (!gst_v4l2trans_set_probed_caps (trans, V4L2_BUF_TYPE_VIDEO_CAPTURE))
    return FALSE;

  if (!gst_v4l2trans_set_probed_caps (trans, V4L2_BUF_TYPE_VIDEO_OUTPUT))
    return FALSE;

  return TRUE;
}

static void
gst_v4l2trans_close_device (GstV4L2Trans * trans)
{
  GST_DEBUG_OBJECT (trans, "Closing");

  if (trans->fd != -1) {
    v4l2_close (trans->fd);
    trans->fd = -1;
  }
}

static gboolean
gst_v4l2trans_stop (GstBaseTransform * btrans)
{
  gint type;

  GstV4L2Trans *trans = GST_V4L2TRANS (btrans);
  GST_DEBUG_OBJECT (btrans, "Stopping");

  /* Close & free output resources (no bufferpool mode) */
  if (trans->out_no_pool_setup) {
    if (trans->fd != -1) {
      type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
      if (v4l2_ioctl (trans->fd, VIDIOC_STREAMOFF, &type) < 0)
        GST_WARNING_OBJECT (btrans, "Unable to stop stream on output");
    }
  }

  /* Close & free capture resources */
  if (trans->fd != -1) {
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl (trans->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (btrans, "Unable to stop stream on capture");
  }

  /* Release buffer pool */
  if (trans->capture_is_down_pool)
    gst_object_unref (trans->capture_info.pool);

  return TRUE;
}

static gint
gst_v4l2trans_fmt_gst_to_v4l2 (GstVideoInfo * info)
{
  switch (GST_VIDEO_INFO_FORMAT (info)) {
    case GST_VIDEO_FORMAT_RGB:
      return V4L2_PIX_FMT_RGB24;
    case GST_VIDEO_FORMAT_I420:
      return V4L2_PIX_FMT_YUV420;
    case GST_VIDEO_FORMAT_NV12:
      return V4L2_PIX_FMT_NV12;
    case GST_VIDEO_FORMAT_RGB16:
      return V4L2_PIX_FMT_RGB565;
    case GST_VIDEO_FORMAT_RGB15:
      return V4L2_PIX_FMT_XRGB555;
    case GST_VIDEO_FORMAT_BGRx:
      return V4L2_PIX_FMT_XBGR32;
    case GST_VIDEO_FORMAT_xRGB:
      return V4L2_PIX_FMT_XRGB32;
    case GST_VIDEO_FORMAT_BGRA:
      return V4L2_PIX_FMT_ABGR32;
    case GST_VIDEO_FORMAT_YVYU:
      return V4L2_PIX_FMT_YVYU;
    case GST_VIDEO_FORMAT_UYVY:
      return V4L2_PIX_FMT_UYVY;
    case GST_VIDEO_FORMAT_YUY2:
      return V4L2_PIX_FMT_YUYV;
    default:
      return 0;
  }
}

static GstVideoFormat
gst_v4l2trans_fmt_v4l2_to_gst (gint fmt)
{
  switch (fmt) {
    case V4L2_PIX_FMT_RGB24:
      return GST_VIDEO_FORMAT_RGB;
    case V4L2_PIX_FMT_YUV420:
      return GST_VIDEO_FORMAT_I420;
    case V4L2_PIX_FMT_NV12:
      return GST_VIDEO_FORMAT_NV12;
    case V4L2_PIX_FMT_RGB565:
      return GST_VIDEO_FORMAT_RGB16;
    case V4L2_PIX_FMT_XRGB555:
    case V4L2_PIX_FMT_RGB555:
      return GST_VIDEO_FORMAT_RGB15;
    case V4L2_PIX_FMT_XBGR32:
    case V4L2_PIX_FMT_BGR32:
      return GST_VIDEO_FORMAT_BGRx;
    case V4L2_PIX_FMT_XRGB32:
    case V4L2_PIX_FMT_RGB32:
      return GST_VIDEO_FORMAT_xRGB;
    case V4L2_PIX_FMT_ABGR32:
      return GST_VIDEO_FORMAT_BGRA;     /* Actually refers to B-G-R-A */
    case V4L2_PIX_FMT_YVYU:
      return GST_VIDEO_FORMAT_YVYU;
    case V4L2_PIX_FMT_UYVY:
      return GST_VIDEO_FORMAT_UYVY;
    case V4L2_PIX_FMT_YUYV:
      return GST_VIDEO_FORMAT_YUY2;
    default:
      return 0;
  }
}

static gint
gst_v4l2trans_align_width (GstVideoInfo * info)
{
  if (info->finfo->format == GST_VIDEO_FORMAT_NV12 ||
      info->finfo->format == GST_VIDEO_FORMAT_I420)
    return (GST_VIDEO_INFO_PLANE_STRIDE (info, 0));
  else
    return (GST_VIDEO_INFO_WIDTH (info));
}

static gint
gst_v4l2trans_align_height (GstVideoInfo * info)
{
  if (info->finfo->format == GST_VIDEO_FORMAT_NV12 ||
      info->finfo->format == GST_VIDEO_FORMAT_I420)
    return (GST_VIDEO_INFO_PLANE_OFFSET (info, 1) /
        GST_VIDEO_INFO_PLANE_STRIDE (info, 0));
  else
    return (GST_VIDEO_INFO_HEIGHT (info));
}

static gboolean
gst_v4l2trans_capture_setup (GstV4L2Trans * trans)
{
  struct v4l2_format s_fmt;
  struct v4l2_selection selection;
  struct v4l2_requestbuffers reqbuf;
  guint i, width, height, aligned_width, aligned_height, size, minbuf, maxbuf;
  GstVideoInfo info;
  GstVideoAlignment align;
  GstCaps *outcaps;
  GstStructure *config;

  /* Stop stream if already started */
  if (trans->capture_start) {
    gint type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl (trans->fd, VIDIOC_STREAMOFF, &type) < 0) {
      GST_WARNING_OBJECT (trans, "Unable to stop stream on capture");
      return FALSE;
    }
  }
  trans->capture_start = FALSE;

  /* Get sizes */
  config = gst_buffer_pool_get_config (trans->capture_info.pool);
  gst_buffer_pool_config_get_params (config, &outcaps, &size, &minbuf, &maxbuf);
  gst_video_info_from_caps (&info, outcaps);

  if (gst_buffer_pool_config_has_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT)) {
    gst_buffer_pool_config_get_video_alignment (config, &align);
    GST_DEBUG_OBJECT (trans, "buffers have padding: %d %d %d %d",
        align.padding_left, align.padding_top, align.padding_right,
        align.padding_bottom);
  } else {
    gst_video_alignment_reset (&align);
  }
  gst_structure_free (config);

  width = gst_v4l2trans_align_width (&info);
  height = gst_v4l2trans_align_height (&info);
  aligned_width = width + align.padding_left + align.padding_right;
  aligned_height = height + align.padding_top + align.padding_bottom;

  /* Set format */
  memset (&s_fmt, 0, sizeof s_fmt);
  s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  s_fmt.fmt.pix.width = aligned_width;
  s_fmt.fmt.pix.height = aligned_height;
  s_fmt.fmt.pix.pixelformat = gst_v4l2trans_fmt_gst_to_v4l2 (&info);
  s_fmt.fmt.pix.sizeimage = size;

  GST_DEBUG_OBJECT (trans, "Using %dx%d (%dx%d) - size=%d", width, height,
      aligned_width, aligned_height, size);

  if (v4l2_ioctl (trans->fd, VIDIOC_S_FMT, &s_fmt) < 0) {
    GST_ERROR_OBJECT (trans, "Capture VIDIOC_S_FMT failed");
    return FALSE;
  }

  /* Crop if needed */
  if (align.padding_left || align.padding_right ||
      align.padding_top || align.padding_bottom) {
    memset (&selection, 0, sizeof selection);
    selection.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    selection.target = V4L2_SEL_TGT_COMPOSE;
    selection.r.left = align.padding_left;
    selection.r.top = align.padding_top;
    selection.r.width = width;
    selection.r.height = height;

    if (v4l2_ioctl (trans->fd, VIDIOC_S_SELECTION, &selection) < 0) {
      GST_ERROR_OBJECT (trans, "Capture VIDIOC_S_SELECTION failed");
      return FALSE;
    }
  }
  /* Request capture buffers  */
  memset (&reqbuf, 0, sizeof reqbuf);
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.count = trans->capture_info.nbbuf;
  reqbuf.memory =
      (trans->capture_is_down_pool ? V4L2_MEMORY_DMABUF : V4L2_MEMORY_MMAP);

  if (v4l2_ioctl (trans->fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
    GST_ERROR_OBJECT (trans, "Capture VIDIOC_REQBUFS failed");
    return FALSE;
  }

  /* Clear V4L2 capture fd table */
  for (i = 0; i < ARRAY_SIZE (trans->capture_info.v4l2_fd); i++)
    trans->capture_info.v4l2_fd[i] = -1;

  /* Clear V4L2 output fd table */
  for (i = 0; i < ARRAY_SIZE (trans->output_info.v4l2_fd); i++)
    trans->output_info.v4l2_fd[i] = -1;

  trans->capture_info.is_s_fmt_done = TRUE;
  return TRUE;
}


static gboolean
gst_v4l2trans_decide_allocation (GstBaseTransform * btrans, GstQuery * query)
{
  GstV4L2Trans *trans = GST_V4L2TRANS (btrans);
  GstCaps *outcaps;
  GstVideoInfo info;
  GstAllocator *allocator;
  GstAllocationParams params;
  GstBufferPool *pool;
  GstStructure *config;
  guint allocator_nb, i, size, minbuf, maxbuf, nbbuf = 0;
  gboolean dmabuf_found = FALSE;
  gboolean active_pool = FALSE;

  /* Get caps and VideoInfo */
  gst_query_parse_allocation (query, &outcaps, NULL);
  gst_video_info_from_caps (&info, outcaps);

  GST_DEBUG_OBJECT (btrans, "Deciding for outcaps %" GST_PTR_FORMAT, outcaps);

  /* parse the allocators searching for a dmabuf allocator */
  allocator_nb = gst_query_get_n_allocation_params (query);
  GST_DEBUG_OBJECT (btrans, "Got %d allocator(s)", allocator_nb);

  for (i = 0; i < allocator_nb; i++) {
    gst_query_parse_nth_allocation_param (query, i, &allocator, &params);

    if (!allocator)
      continue;

    GST_DEBUG_OBJECT (btrans, "Allocator %d = %s", i, allocator->mem_type);

    /* Looking for a DMABUF allocator */
    if (!g_strcmp0 (allocator->mem_type, GST_ALLOCATOR_DMABUF)) {
      if (trans->capture_is_down_pool &&
          gst_buffer_pool_is_active (trans->capture_info.pool)) {
        /* Pool was already active */
        GST_DEBUG_OBJECT (btrans, "Pool %s already active",
            trans->capture_info.pool->object.name);
        active_pool = TRUE;
      }

      if (!active_pool)
        /* Select the DMABUF allocator */
        gst_query_set_nth_allocation_param (query, 0, allocator, &params);

      /* Configure pool */
      gst_query_parse_nth_allocation_pool (query, 0, &pool, &size,
          &minbuf, &maxbuf);
      GST_DEBUG_OBJECT (btrans, "Found downstream DMABUF pool 0x%p "
          "with min=%d max=%d ", pool, minbuf, maxbuf);

      /* Set the nb of buffers of the downstream pool */
      nbbuf = MAX (minbuf, NB_BUF_CAPTURE_POOL);

      gst_query_add_allocation_pool (query, pool, size, nbbuf, nbbuf);
      gst_query_set_nth_allocation_pool (query, 0, pool, size, nbbuf, nbbuf);
      GST_DEBUG_OBJECT (btrans, "Set downstream DMABUFpool %p to min=%d max=%d",
          pool, nbbuf, nbbuf);

      /* Get possible config updates (eg padding) from the pool */
      config = gst_buffer_pool_get_config (pool);
      gst_buffer_pool_config_set_params (config, outcaps, size, nbbuf, nbbuf);
      gst_buffer_pool_config_set_allocator (config, allocator, &params);
      gst_buffer_pool_set_config (pool, config);


      /* Set downstream pool reference */
      if (active_pool)
        gst_object_unref (trans->capture_info.pool);
      trans->capture_is_down_pool = TRUE;
      trans->capture_info.pool = pool;
      trans->capture_info.nbbuf = nbbuf;
      dmabuf_found = TRUE;

      /* Configure V4L2 format and request DMABUF buffers : one V4L2 buffer for
       * each buffer from the pool.
       * Here (MEMORY_DMABUF) V4L2 does not allocate buffers memory */
      if (!trans->capture_info.is_s_fmt_done)
        if (!gst_v4l2trans_capture_setup (trans))
          return FALSE;

      GST_DEBUG_OBJECT (btrans, "Capture for V4L2 configured");
      break;
    }
    gst_object_unref (allocator);
  }

  if (!dmabuf_found) {
    /* Allocate a pool */
    if (allocator_nb)
      gst_query_parse_nth_allocation_pool (query, 0, &pool, &size,
          &nbbuf, &maxbuf);

    nbbuf = MAX (nbbuf, NB_BUF_CAPTURE_POOL);

    GST_DEBUG_OBJECT (btrans, "No DMABUF allocator found: creating pool with "
        "%d buffers", nbbuf);

    pool = gst_v4l2_cap_buf_pool_new (trans, outcaps,
        GST_VIDEO_INFO_SIZE (&info), nbbuf);

    gst_query_add_allocation_pool (query, pool, GST_VIDEO_INFO_SIZE (&info),
        nbbuf, nbbuf);
    gst_query_set_nth_allocation_pool (query, 0, pool,
        GST_VIDEO_INFO_SIZE (&info), nbbuf, nbbuf);

    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_get_allocator (config, &allocator, &params);
    gst_query_add_allocation_param (query, allocator, &params);
    gst_query_set_nth_allocation_param (query, 0, allocator, &params);

    gst_object_unref (pool);

    /* Configure V4L2 format and request MMAP buffers : one V4L2 buffer for
     * each buffer from the pool.
     * Here (MEMORY_MMAP) V4L2 actually allocate buffers memory */
    trans->capture_info.pool = pool;
    trans->capture_info.nbbuf = nbbuf;

    if (!gst_v4l2trans_capture_setup (trans))
      return FALSE;

    GST_DEBUG_OBJECT (btrans, "Capture for V4L2 configured");
  }

  return GST_BASE_TRANSFORM_CLASS (parent_class)->decide_allocation (btrans,
      query);
}

static gboolean
gst_v4l2trans_set_caps (GstBaseTransform * btrans, GstCaps * incaps,
    GstCaps * outcaps)
{
  gint w, h, ret;
  struct v4l2_format try_fmt_out;
  GstStructure *outs = gst_caps_get_structure (outcaps, 0);
  GstV4L2Trans *trans = GST_V4L2TRANS (btrans);

  /* Service used to validate outcaps : check V4L2 driver is able to
   * handle such width and height in term of data alignment
   */
  gst_structure_get_int (outs, "width", &w);
  gst_structure_get_int (outs, "height", &h);
  /* Try to set this frame resolution to check if driver supports it */
  memset (&try_fmt_out, 0, sizeof try_fmt_out);
  try_fmt_out.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  try_fmt_out.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
  try_fmt_out.fmt.pix.width = w;
  try_fmt_out.fmt.pix.height = h;
  ret = v4l2_ioctl (trans->fd, VIDIOC_TRY_FMT, &try_fmt_out);
  if (ret < 0) {
    GST_DEBUG_OBJECT (trans, "Error returned after trying to set frame format");
    return FALSE;
  }
  if (try_fmt_out.fmt.pix.width != w || try_fmt_out.fmt.pix.height != h) {
    GST_ERROR_OBJECT (trans,
        "Width and height must be aligned on 16 pixels : kindly use %dx%d instead of %dx%d)",
        try_fmt_out.fmt.pix.width, try_fmt_out.fmt.pix.height, w, h);
    return FALSE;
  }
  return GST_BASE_TRANSFORM_CLASS (parent_class)->set_caps (btrans, incaps,
      outcaps);
}

static gboolean
gst_v4l2trans_propose_allocation (GstBaseTransform * btrans,
    GstQuery * decide_query, GstQuery * query)
{
  GstV4L2Trans *trans = GST_V4L2TRANS (btrans);
  GstBufferPool *pool;
  GstStructure *config;
  GstCaps *caps;
  GstAllocator *allocator;
  GstAllocationParams params;
  GstCaps *pcaps;
  GstVideoInfo info;
  guint size;
  gboolean need_pool;

  GST_DEBUG_OBJECT (trans, "Proposing output buffer pool");

  if (decide_query == NULL)
    goto out;

  gst_query_parse_allocation (query, &caps, &need_pool);

  if (caps == NULL)
    goto no_caps;

  pool = trans->output_info.pool;

  if (pool != NULL) {
    gst_object_ref (pool);

    /* we had a pool, check caps */
    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_get_params (config, &pcaps, &size, NULL, NULL);
    GST_DEBUG_OBJECT (trans, "We had a pool with caps %" GST_PTR_FORMAT, pcaps);

    if (!gst_caps_is_equal (caps, pcaps)) {
      /* different caps, we can't use this pool */
      gst_object_unref (pool);
      GST_DEBUG_OBJECT (trans, "Pool has different caps");
      pool = NULL;
    } else {
      GST_DEBUG_OBJECT (trans, "Pool has same caps, proposing again");

      if (!gst_video_info_from_caps (&info, caps))
        goto invalid_caps;

      /* Add buffer pool and VIDEO META */
      gst_query_add_allocation_pool (query, pool, info.size,
          MIN_BUF_OUTPUT_POOL, 0);
      gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);
      gst_object_unref (pool);

      /* Add DMA-BUF allocator */
      allocator = gst_dmabuf_allocator_new ();
      gst_allocation_params_init (&params);
      gst_query_add_allocation_param (query, allocator, &params);
      gst_object_unref (allocator);
    }
    gst_structure_free (config);
  }

  if (pool == NULL && need_pool) {
    if (!gst_video_info_from_caps (&info, caps))
      goto invalid_caps;

    GST_DEBUG_OBJECT (trans, "Creating output buffer pool");

    pool = gst_v4l2_out_buf_pool_new (trans);
    if (pool == NULL)
      goto error_new_pool;

    trans->output_info.pool = pool;

    config = gst_buffer_pool_get_config (pool);
    if (gst_query_find_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL)) {
      gst_buffer_pool_config_add_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_META);
      gst_buffer_pool_config_add_option (config,
          GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);

      GST_LOG_OBJECT (trans,
          "Set output buffer pool %p padding to %u-%ux%u-%u", pool,
          trans->align.padding_top, trans->align.padding_left,
          trans->align.padding_right, trans->align.padding_bottom);
      gst_buffer_pool_config_set_video_alignment (config, &trans->align);
      gst_buffer_pool_set_config (pool, config);
    }
    /* Add buffer pool and VIDEO META */
    gst_query_add_allocation_pool (query, pool, info.size,
        MIN_BUF_OUTPUT_POOL, 0);
    gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);
    if (pool)
      gst_object_unref (pool);

    /* Add DMA-BUF allocator */
    allocator = gst_dmabuf_allocator_new ();
    gst_allocation_params_init (&params);
    gst_query_add_allocation_param (query, allocator, &params);
    gst_object_unref (allocator);
  }

out:
  return GST_BASE_TRANSFORM_CLASS (parent_class)->propose_allocation (btrans,
      decide_query, query);

no_caps:
  {
    GST_DEBUG_OBJECT (trans, "No caps specified");
    return FALSE;
  }
invalid_caps:
  {
    GST_DEBUG_OBJECT (trans, "Invalid caps specified");
    return FALSE;
  }
error_new_pool:
  {
    GST_ERROR_OBJECT (trans, "Unable to construct a new buffer pool");
    return GST_FLOW_ERROR;
  }
}


static gint
gst_v4l2_crop_transform_dimension (gint val, gint delta)
{
  gint64 new_val = (gint64) val + (gint64) delta;

  new_val = CLAMP (new_val, 1, G_MAXINT);

  return (gint) new_val;
}

static gboolean
gst_v4l2_crop_transform_dimension_value (const GValue * src_val,
    gint delta, GValue * dest_val, GstPadDirection direction, gboolean dynamic)
{
  gboolean ret = TRUE;

  if (G_VALUE_HOLDS_INT (src_val)) {
    gint ival = g_value_get_int (src_val);
    ival = gst_v4l2_crop_transform_dimension (ival, delta);

    if (dynamic) {
      if (direction == GST_PAD_SRC) {
        if (ival == G_MAXINT) {
          g_value_init (dest_val, G_TYPE_INT);
          g_value_set_int (dest_val, ival);
        } else {
          g_value_init (dest_val, GST_TYPE_INT_RANGE);
          gst_value_set_int_range (dest_val, ival, G_MAXINT);
        }
      } else {
        if (ival == 1) {
          g_value_init (dest_val, G_TYPE_INT);
          g_value_set_int (dest_val, ival);
        } else {
          g_value_init (dest_val, GST_TYPE_INT_RANGE);
          gst_value_set_int_range (dest_val, 1, ival);
        }
      }
    } else {
      g_value_init (dest_val, G_TYPE_INT);
      g_value_set_int (dest_val, ival);
    }
  } else if (GST_VALUE_HOLDS_INT_RANGE (src_val)) {
    gint min = gst_value_get_int_range_min (src_val);
    gint max = gst_value_get_int_range_max (src_val);

    min = gst_v4l2_crop_transform_dimension (min, delta);
    max = gst_v4l2_crop_transform_dimension (max, delta);

    if (dynamic) {
      if (direction == GST_PAD_SRC)
        max = G_MAXINT;
      else
        min = 1;
    }

    if (min == max) {
      g_value_init (dest_val, G_TYPE_INT);
      g_value_set_int (dest_val, min);
    } else {
      g_value_init (dest_val, GST_TYPE_INT_RANGE);
      gst_value_set_int_range (dest_val, min, max);
    }
  } else if (GST_VALUE_HOLDS_LIST (src_val)) {
    gint i;

    g_value_init (dest_val, GST_TYPE_LIST);

    for (i = 0; i < gst_value_list_get_size (src_val); ++i) {
      const GValue *list_val;
      GValue newval = { 0, };

      list_val = gst_value_list_get_value (src_val, i);
      if (gst_v4l2_crop_transform_dimension_value (list_val, delta, &newval,
              direction, dynamic))
        gst_value_list_append_value (dest_val, &newval);
      g_value_unset (&newval);
    }

    if (gst_value_list_get_size (dest_val) == 0) {
      g_value_unset (dest_val);
      ret = FALSE;
    }
  } else {
    ret = FALSE;
  }

  return ret;
}

static GstCaps *
gst_v4l2trans_crop_transform_caps (GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps)
{
  GstCaps *other_caps;
  gint dy, dx, i, left, right, bottom, top;
  gboolean w_dynamic, h_dynamic;
  GstV4L2Trans *trans = GST_V4L2TRANS (btrans);

  GST_LOG_OBJECT (btrans, "l=%d,r=%d,b=%d,t=%d",
      trans->crop.left, trans->crop.right, trans->crop.bottom, trans->crop.top);

  w_dynamic = (trans->crop.left == -1 || trans->crop.right == -1);
  h_dynamic = (trans->crop.top == -1 || trans->crop.bottom == -1);

  left = (trans->crop.left == -1) ? 0 : trans->crop.left;
  right = (trans->crop.right == -1) ? 0 : trans->crop.right;
  bottom = (trans->crop.bottom == -1) ? 0 : trans->crop.bottom;
  top = (trans->crop.top == -1) ? 0 : trans->crop.top;

  if (direction == GST_PAD_SRC) {
    dx = left + right;
    dy = top + bottom;
  } else {
    dx = 0 - (left + right);
    dy = 0 - (top + bottom);
  }

  GST_LOG_OBJECT (btrans, "transforming caps %" GST_PTR_FORMAT, caps);

  other_caps = gst_caps_new_empty ();

  for (i = 0; i < gst_caps_get_size (caps); ++i) {
    const GValue *v;
    gint wdth, hght;
    GstStructure *structure, *new_structure;
    GValue w_val = { 0, }, h_val = {
    0,};
    gint w_aligned, h_aligned;

    structure = gst_caps_get_structure (caps, i);

    v = gst_structure_get_value (structure, "width");
    wdth = g_value_get_int (v);
    if (!gst_v4l2_crop_transform_dimension_value (v, dx, &w_val, direction,
            w_dynamic)) {
      GST_WARNING_OBJECT (btrans, "could not tranform width value with dx=%d"
          ", caps structure=%" GST_PTR_FORMAT, dx, structure);
      continue;
    }

    v = gst_structure_get_value (structure, "height");
    hght = g_value_get_int (v);
    if (!gst_v4l2_crop_transform_dimension_value (v, dy, &h_val, direction,
            h_dynamic)) {
      g_value_unset (&w_val);
      GST_WARNING_OBJECT (btrans, "could not tranform height value with dy=%d"
          ", caps structure=%" GST_PTR_FORMAT, dy, structure);
      continue;
    }

    w_aligned = (g_value_get_int (&w_val));
    h_aligned = (g_value_get_int (&h_val));

    if (w_aligned != g_value_get_int (&w_val)) {
      trans->crop.right = wdth - trans->crop.left - w_aligned;
      GST_WARNING_OBJECT (btrans,
          "Align cropping to hardware constraint (width %d->%d) "
          "update right cropping value to %d",
          g_value_get_int (&w_val), w_aligned, trans->crop.right);
    }

    if (h_aligned != g_value_get_int (&h_val)) {
      trans->crop.bottom = hght - trans->crop.top - h_aligned;
      GST_WARNING_OBJECT (btrans,
          "Align cropping to hardware constraint (height %d->%d) "
          "update right cropping value to %d",
          g_value_get_int (&h_val), h_aligned, trans->crop.bottom);
    }

    g_value_unset (&w_val);
    g_value_unset (&h_val);

    new_structure = gst_structure_copy (structure);
    gst_structure_set (new_structure,
        "width", G_TYPE_INT, w_aligned, "height", G_TYPE_INT, h_aligned, NULL);

    GST_LOG_OBJECT (btrans, "transformed structure %2d: %" GST_PTR_FORMAT
        " => %" GST_PTR_FORMAT, i, structure, new_structure);
    gst_caps_append_structure (other_caps, new_structure);
  }
  if (gst_caps_is_empty (other_caps)) {
    GST_ERROR ("!!!!!!! no output caps !!!!!");
  }
  return other_caps;
}

static GstCaps *
gst_v4l2trans_transform_caps (GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
  GstCaps *tmp, *result, *res;
  GstStructure *st;
  GstCapsFeatures *f;
  gint i, n;

  GST_DEBUG_OBJECT (btrans,
      "Transforming caps %" GST_PTR_FORMAT " in direction %s", caps,
      (direction == GST_PAD_SINK) ? "sink" : "src");

  res = gst_caps_new_empty ();
  n = gst_caps_get_size (caps);
  for (i = 0; i < n; i++) {
    st = gst_caps_get_structure (caps, i);
    f = gst_caps_get_features (caps, i);

    /* If this is already expressed by the existing caps skip this structure */
    if (i > 0 && gst_caps_is_subset_structure_full (res, st, f))
      continue;

    st = gst_structure_copy (st);

    /* remove the informations (format, width, height...) for the cases
     * when we can actually transform */
    if (!gst_caps_features_is_any (f)) {
      gst_structure_remove_fields (st, "width", "height", NULL);
      gst_structure_remove_fields (st, "format", "pixel-aspect-ratio", NULL);
    }
    gst_caps_append_structure_full (res, st, gst_caps_features_copy (f));
  }

  if (filter) {
    tmp = gst_caps_intersect_full (filter, res, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (res);
    res = tmp;
  }
  result = res;

  GST_DEBUG_OBJECT (btrans, "Transformed %" GST_PTR_FORMAT " into %"
      GST_PTR_FORMAT, caps, result);

  return result;
}

static GstCaps *
gst_v4l2trans_update_interlace_mode (GstBaseTransform * base,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps)
{
  GstStructure *ins, *outs;
  const gchar *s;
  GstVideoInterlaceMode interlace_mode = GST_VIDEO_INTERLACE_MODE_PROGRESSIVE;

  othercaps = gst_caps_truncate (othercaps);
  othercaps = gst_caps_make_writable (othercaps);

  ins = gst_caps_get_structure (caps, 0);
  outs = gst_caps_get_structure (othercaps, 0);

  if ((s = gst_structure_get_string (ins, "format"))) {
    if (g_str_equal (s, "NV12")) {
      /* Input format is NV12 so driver is able to deinterlace if needed */
      if ((s = gst_structure_get_string (ins, "interlace-mode")))
        interlace_mode = gst_interlace_mode_from_string (s);

      if (interlace_mode != GST_VIDEO_INTERLACE_MODE_PROGRESSIVE) {
        /* Input stream is not progressive, driver will deinterlace it */
        GST_DEBUG_OBJECT (base, "Update interlace-mode to progressive");
        gst_structure_set (outs, "interlace-mode",
            G_TYPE_STRING, "progressive", NULL);
      }
    }
  }

  return othercaps;
}

/* The following function is copied from gst-plugins-base/gst/videoscale/
 * gstvideoscale.c (file under the terms of the GNU Library General Public
 * License version 2 or later).
 * It fixates the width, height and pixel-aspect-ratio capabilities for the
 * scaling transform.
 */
static GstCaps *
gst_video_scale_fixate_caps (GstBaseTransform * base, GstPadDirection direction,
    GstCaps * caps, GstCaps * othercaps)
{
  GstStructure *ins, *outs;
  const GValue *from_par, *to_par;
  GValue fpar = { 0, }, tpar = {
  0,};
  GstCaps *input_caps;

  othercaps = gst_caps_truncate (othercaps);
  othercaps = gst_caps_make_writable (othercaps);

  GST_DEBUG_OBJECT (base, "Trying to fixate othercaps %" GST_PTR_FORMAT
      " based on caps %" GST_PTR_FORMAT, othercaps, caps);

  /* width, height and PAR are not fixed but passthrough is not possible */
  input_caps = gst_v4l2trans_crop_transform_caps (base, direction, caps);

  GST_DEBUG_OBJECT (base, "Input caps updated : %" GST_PTR_FORMAT, input_caps);

  ins = gst_caps_get_structure (input_caps, 0);
  outs = gst_caps_get_structure (othercaps, 0);

  from_par = gst_structure_get_value (ins, "pixel-aspect-ratio");

  /* If PAR is not fixed, we force PAR to 1/1 : waylandsink doesn't support
   * other values.
   */
  if (!gst_structure_has_field (outs, "pixel-aspect-ratio")) {
    gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
        1, 1, NULL);
  }
  to_par = gst_structure_get_value (outs, "pixel-aspect-ratio");

  /* If we're fixating from the sinkpad we always set the PAR and
   * assume that missing PAR on the sinkpad means 1/1 and
   * missing PAR on the srcpad means undefined
   */
  if (direction == GST_PAD_SINK) {
    if (!from_par) {
      g_value_init (&fpar, GST_TYPE_FRACTION);
      gst_value_set_fraction (&fpar, 1, 1);
      from_par = &fpar;
    }
    if (!to_par) {
      g_value_init (&tpar, GST_TYPE_FRACTION_RANGE);
      gst_value_set_fraction_range_full (&tpar, 1, G_MAXINT, G_MAXINT, 1);
      to_par = &tpar;
    }
  } else {
    if (!to_par) {
      g_value_init (&tpar, GST_TYPE_FRACTION);
      gst_value_set_fraction (&tpar, 1, 1);
      to_par = &tpar;

      gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
          NULL);
    }
    if (!from_par) {
      g_value_init (&fpar, GST_TYPE_FRACTION);
      gst_value_set_fraction (&fpar, 1, 1);
      from_par = &fpar;
    }
  }

  /* we have both PAR but they might not be fixated */
  {
    gint from_w, from_h, from_par_n, from_par_d, to_par_n, to_par_d;
    gint w = 0, h = 0;
    gint from_dar_n, from_dar_d;
    gint num, den;

    /* from_par should be fixed */
    g_return_val_if_fail (gst_value_is_fixed (from_par), othercaps);

    from_par_n = gst_value_get_fraction_numerator (from_par);
    from_par_d = gst_value_get_fraction_denominator (from_par);

    gst_structure_get_int (ins, "width", &from_w);
    gst_structure_get_int (ins, "height", &from_h);

    gst_structure_get_int (outs, "width", &w);
    gst_structure_get_int (outs, "height", &h);

    /* if both width and height are already fixed, we can't do anything
     * about it anymore */
    if (w && h) {
      guint n, d;

      GST_DEBUG_OBJECT (base, "Dimensions already set to %dx%d, not fixating",
          w, h);
      if (!gst_value_is_fixed (to_par)) {
        if (gst_video_calculate_display_ratio (&n, &d, from_w, from_h,
                from_par_n, from_par_d, w, h)) {
          GST_DEBUG_OBJECT (base, "Fixating to_par to %dx%d", n, d);
          if (gst_structure_has_field (outs, "pixel-aspect-ratio"))
            gst_structure_fixate_field_nearest_fraction (outs,
                "pixel-aspect-ratio", n, d);
          else if (n != d)
            gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
                n, d, NULL);
        }
      }
      goto done;
    }

    /* Calculate input DAR */
    if (!gst_util_fraction_multiply (from_w, from_h, from_par_n, from_par_d,
            &from_dar_n, &from_dar_d)) {
      GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
          ("Error calculating the output scaled size - integer overflow"));
      goto done;
    }

    GST_DEBUG_OBJECT (base, "Input DAR is %d/%d", from_dar_n, from_dar_d);

    /* If either width or height are fixed there's not much we
     * can do either except choosing a height or width and PAR
     * that matches the DAR as good as possible
     */
    if (h) {
      GstStructure *tmp;
      gint set_w, set_par_n, set_par_d;

      GST_DEBUG_OBJECT (base, "Height is fixed (%d)", h);

      /* If the PAR is fixed too, there's not much to do
       * except choosing the width that is nearest to the
       * width with the same DAR */
      if (gst_value_is_fixed (to_par)) {
        to_par_n = gst_value_get_fraction_numerator (to_par);
        to_par_d = gst_value_get_fraction_denominator (to_par);

        GST_DEBUG_OBJECT (base, "PAR is fixed %d/%d", to_par_n, to_par_d);

        if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, to_par_d,
                to_par_n, &num, &den)) {
          GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
              ("Error calculating the output scaled size - integer overflow"));
          goto done;
        }

        w = (guint) gst_util_uint64_scale_int (h, num, den);
        // Adaptation to G1 : Alignment on 16bytes
        w = ALIGN_DIM_ON_16_BYTES (w);
        gst_structure_fixate_field_nearest_int (outs, "width", w);

        goto done;
      }

      /* The PAR is not fixed and it's quite likely that we can set
       * an arbitrary PAR. */

      /* Check if we can keep the input width */
      tmp = gst_structure_copy (outs);
      gst_structure_fixate_field_nearest_int (tmp, "width", from_w);
      gst_structure_get_int (tmp, "width", &set_w);

      /* Might have failed but try to keep the DAR nonetheless by
       * adjusting the PAR */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, h, set_w,
              &to_par_n, &to_par_d)) {
        GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        gst_structure_free (tmp);
        goto done;
      }

      if (!gst_structure_has_field (tmp, "pixel-aspect-ratio"))
        gst_structure_set_value (tmp, "pixel-aspect-ratio", to_par);
      gst_structure_fixate_field_nearest_fraction (tmp, "pixel-aspect-ratio",
          to_par_n, to_par_d);
      gst_structure_get_fraction (tmp, "pixel-aspect-ratio", &set_par_n,
          &set_par_d);
      gst_structure_free (tmp);

      /* Check if the adjusted PAR is accepted */
      if (set_par_n == to_par_n && set_par_d == to_par_d) {
        if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
            set_par_n != set_par_d)
          gst_structure_set (outs, "width", G_TYPE_INT, set_w,
              "pixel-aspect-ratio", GST_TYPE_FRACTION, set_par_n, set_par_d,
              NULL);
        goto done;
      }

      /* Otherwise scale the width to the new PAR and check if the
       * adjusted with is accepted. If all that fails we can't keep
       * the DAR */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, set_par_d,
              set_par_n, &num, &den)) {
        GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto done;
      }

      w = (guint) gst_util_uint64_scale_int (h, num, den);
      // Adaptation to G1 : Alignment on 16bytes
      w = ALIGN_DIM_ON_16_BYTES (w);
      gst_structure_fixate_field_nearest_int (outs, "width", w);
      if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
          set_par_n != set_par_d)
        gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
            set_par_n, set_par_d, NULL);

      goto done;
    } else if (w) {
      GstStructure *tmp;
      gint set_h, set_par_n, set_par_d;

      GST_DEBUG_OBJECT (base, "Width is fixed (%d)", w);

      /* If the PAR is fixed too, there's not much to do
       * except choosing the height that is nearest to the
       * height with the same DAR */
      if (gst_value_is_fixed (to_par)) {
        to_par_n = gst_value_get_fraction_numerator (to_par);
        to_par_d = gst_value_get_fraction_denominator (to_par);

        GST_DEBUG_OBJECT (base, "PAR is fixed %d/%d", to_par_n, to_par_d);

        if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, to_par_d,
                to_par_n, &num, &den)) {
          GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
              ("Error calculating the output scaled size - integer overflow"));
          goto done;
        }

        h = (guint) gst_util_uint64_scale_int (w, den, num);
        // Adaptation to G1 : Alignment on 16bytes
        h = ALIGN_DIM_ON_16_BYTES (h);
        gst_structure_fixate_field_nearest_int (outs, "height", h);

        goto done;
      }

      /* The PAR is not fixed and it's quite likely that we can set
       * an arbitrary PAR. */

      /* Check if we can keep the input height */
      tmp = gst_structure_copy (outs);
      gst_structure_fixate_field_nearest_int (tmp, "height", from_h);
      gst_structure_get_int (tmp, "height", &set_h);

      /* Might have failed but try to keep the DAR nonetheless by
       * adjusting the PAR */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, set_h, w,
              &to_par_n, &to_par_d)) {
        GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        gst_structure_free (tmp);
        goto done;
      }
      if (!gst_structure_has_field (tmp, "pixel-aspect-ratio"))
        gst_structure_set_value (tmp, "pixel-aspect-ratio", to_par);
      gst_structure_fixate_field_nearest_fraction (tmp, "pixel-aspect-ratio",
          to_par_n, to_par_d);
      gst_structure_get_fraction (tmp, "pixel-aspect-ratio", &set_par_n,
          &set_par_d);
      gst_structure_free (tmp);

      /* Check if the adjusted PAR is accepted */
      if (set_par_n == to_par_n && set_par_d == to_par_d) {
        if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
            set_par_n != set_par_d)
          gst_structure_set (outs, "height", G_TYPE_INT, set_h,
              "pixel-aspect-ratio", GST_TYPE_FRACTION, set_par_n, set_par_d,
              NULL);
        gst_structure_set (outs, "height", G_TYPE_INT, set_h, NULL);
        goto done;
      }

      /* Otherwise scale the height to the new PAR and check if the
       * adjusted with is accepted. If all that fails we can't keep
       * the DAR */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, set_par_d,
              set_par_n, &num, &den)) {
        GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto done;
      }

      h = (guint) gst_util_uint64_scale_int (w, den, num);
      // Adaptation to G1 : Alignment on 16bytes
      h = ALIGN_DIM_ON_16_BYTES (h);
      gst_structure_fixate_field_nearest_int (outs, "height", h);
      if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
          set_par_n != set_par_d)
        gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
            set_par_n, set_par_d, NULL);

      goto done;
    } else if (gst_value_is_fixed (to_par)) {
      GstStructure *tmp;
      gint set_h, set_w, f_h, f_w;

      to_par_n = gst_value_get_fraction_numerator (to_par);
      to_par_d = gst_value_get_fraction_denominator (to_par);

      /* Calculate scale factor for the PAR change */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, to_par_n,
              to_par_d, &num, &den)) {
        GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto done;
      }

      /* Try to keep the input height (because of interlacing) */
      tmp = gst_structure_copy (outs);
      gst_structure_fixate_field_nearest_int (tmp, "height", from_h);
      gst_structure_get_int (tmp, "height", &set_h);

      /* This might have failed but try to scale the width
       * to keep the DAR nonetheless */
      w = (guint) gst_util_uint64_scale_int (set_h, num, den);
      // Adaptation to G1 : Alignment on 16bytes
      w = ALIGN_DIM_ON_16_BYTES (w);
      gst_structure_fixate_field_nearest_int (tmp, "width", w);
      gst_structure_get_int (tmp, "width", &set_w);
      gst_structure_free (tmp);

      /* We kept the DAR and the height is nearest to the original height */
      if (set_w == w) {
        gst_structure_set (outs, "width", G_TYPE_INT, set_w, "height",
            G_TYPE_INT, set_h, NULL);
        goto done;
      }

      f_h = set_h;
      f_w = set_w;

      /* If the former failed, try to keep the input width at least */
      tmp = gst_structure_copy (outs);
      gst_structure_fixate_field_nearest_int (tmp, "width", from_w);
      gst_structure_get_int (tmp, "width", &set_w);

      /* This might have failed but try to scale the width
       * to keep the DAR nonetheless */
      h = (guint) gst_util_uint64_scale_int (set_w, den, num);
      // Adaptation to G1 : Alignment on 16bytes
      h = ALIGN_DIM_ON_16_BYTES (h);
      gst_structure_fixate_field_nearest_int (tmp, "height", h);
      gst_structure_get_int (tmp, "height", &set_h);
      gst_structure_free (tmp);

      /* We kept the DAR and the width is nearest to the original width */
      if (set_h == h) {
        gst_structure_set (outs, "width", G_TYPE_INT, set_w, "height",
            G_TYPE_INT, set_h, NULL);
        goto done;
      }

      /* If all this failed, keep the height that was nearest to the orignal
       * height and the nearest possible width. This changes the DAR but
       * there's not much else to do here.
       */
      gst_structure_set (outs, "width", G_TYPE_INT, f_w, "height", G_TYPE_INT,
          f_h, NULL);
      goto done;
    } else {
      GstStructure *tmp;
      gint set_h, set_w, set_par_n, set_par_d, tmp2;

      /* First try to keep the height and width as good as possible
       * and scale PAR */
      tmp = gst_structure_copy (outs);
      gst_structure_fixate_field_nearest_int (tmp, "height", from_h);
      gst_structure_get_int (tmp, "height", &set_h);
      gst_structure_fixate_field_nearest_int (tmp, "width", from_w);
      gst_structure_get_int (tmp, "width", &set_w);

      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, set_h, set_w,
              &to_par_n, &to_par_d)) {
        GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto done;
      }

      if (!gst_structure_has_field (tmp, "pixel-aspect-ratio"))
        gst_structure_set_value (tmp, "pixel-aspect-ratio", to_par);
      gst_structure_fixate_field_nearest_fraction (tmp, "pixel-aspect-ratio",
          to_par_n, to_par_d);
      gst_structure_get_fraction (tmp, "pixel-aspect-ratio", &set_par_n,
          &set_par_d);
      gst_structure_free (tmp);

      if (set_par_n == to_par_n && set_par_d == to_par_d) {
        gst_structure_set (outs, "width", G_TYPE_INT, set_w, "height",
            G_TYPE_INT, set_h, NULL);

        if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
            set_par_n != set_par_d)
          gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
              set_par_n, set_par_d, NULL);
        goto done;
      }

      /* Otherwise try to scale width to keep the DAR with the set
       * PAR and height */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, set_par_d,
              set_par_n, &num, &den)) {
        GST_ELEMENT_ERROR (base, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto done;
      }

      w = (guint) gst_util_uint64_scale_int (set_h, num, den);
      // Adaptation to G1 : Alignment on 16bytes
      w = ALIGN_DIM_ON_16_BYTES (w);
      tmp = gst_structure_copy (outs);
      gst_structure_fixate_field_nearest_int (tmp, "width", w);
      gst_structure_get_int (tmp, "width", &tmp2);
      gst_structure_free (tmp);

      if (tmp2 == w) {
        gst_structure_set (outs, "width", G_TYPE_INT, tmp2, "height",
            G_TYPE_INT, set_h, NULL);
        if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
            set_par_n != set_par_d)
          gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
              set_par_n, set_par_d, NULL);
        goto done;
      }

      /* ... or try the same with the height */
      h = (guint) gst_util_uint64_scale_int (set_w, den, num);
      // Adaptation to G1 : Alignment on 16bytes
      h = ALIGN_DIM_ON_16_BYTES (h);
      tmp = gst_structure_copy (outs);
      gst_structure_fixate_field_nearest_int (tmp, "height", h);
      gst_structure_get_int (tmp, "height", &tmp2);
      gst_structure_free (tmp);

      if (tmp2 == h) {
        gst_structure_set (outs, "width", G_TYPE_INT, set_w, "height",
            G_TYPE_INT, tmp2, NULL);
        if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
            set_par_n != set_par_d)
          gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
              set_par_n, set_par_d, NULL);
        goto done;
      }

      /* If all fails we can't keep the DAR and take the nearest values
       * for everything from the first try */
      gst_structure_set (outs, "width", G_TYPE_INT, set_w, "height",
          G_TYPE_INT, set_h, NULL);
      if (gst_structure_has_field (outs, "pixel-aspect-ratio") ||
          set_par_n != set_par_d)
        gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
            set_par_n, set_par_d, NULL);
    }
  }

done:
  GST_DEBUG_OBJECT (base, "Fixated othercaps to %" GST_PTR_FORMAT, othercaps);

  if (from_par == &fpar)
    g_value_unset (&fpar);
  if (to_par == &tpar)
    g_value_unset (&tpar);

  return othercaps;
}

static GstCaps *
gst_v4l2trans_fixate_caps (GstBaseTransform * btrans,
    GstPadDirection direction, GstCaps * caps, GstCaps * othercaps)
{
  GstCaps *prefcaps, *orderedcaps;
  const GValue *par, *other_par;

  /* fixate the scaling caps (width, height, pixel-aspect-ratio) */
  othercaps = gst_video_scale_fixate_caps (btrans, direction, caps, othercaps);

  /* fixate the interlace mode : G1 specific */
  othercaps =
      gst_v4l2trans_update_interlace_mode (btrans, direction, caps, othercaps);

  /* if needed copy PAR from caps to other caps */
  par = gst_structure_get_value (gst_caps_get_structure (caps, 0),
      "pixel-aspect-ratio");
  other_par = gst_structure_get_value (gst_caps_get_structure (othercaps, 0),
      "pixel-aspect-ratio");
  if (par && !other_par)
    gst_structure_set_value (gst_caps_get_structure (othercaps, 0),
        "pixel-aspect-ratio", par);

  /* fixate any remaining field (e.g. format).
   * Select the format according to the format ordered caps list */
  if (direction == GST_PAD_SRC)
    prefcaps = gst_v4l2trans_get_sink_capslist ();
  else
    prefcaps = gst_v4l2trans_get_src_capslist ();

  orderedcaps = gst_caps_intersect_full (prefcaps, othercaps,
      GST_CAPS_INTERSECT_FIRST);
  gst_caps_unref (othercaps);
  othercaps = gst_caps_fixate (orderedcaps);

  GST_DEBUG_OBJECT (btrans, "Fixated othercaps to %" GST_PTR_FORMAT, othercaps);

  return othercaps;
}

static gboolean
gst_v4l2trans_query (GstBaseTransform * btrans, GstPadDirection direction,
    GstQuery * query)
{
  GstV4L2Trans *trans = GST_V4L2TRANS (btrans);
  GstCaps *caps, *filter, *result, *tmp;
  GstPad *pad, *otherpad;
  gboolean ret = TRUE;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_CAPS:
      /* First get capture_info.probed_caps / output_info.probed_caps from driver capabilities */
      if (!gst_v4l2trans_open_device (trans))
        return GST_STATE_CHANGE_FAILURE;

      /* Get caps from driver */
      if (direction == GST_PAD_SRC) {
        pad = GST_BASE_TRANSFORM_SRC_PAD (trans);
        otherpad = GST_BASE_TRANSFORM_SINK_PAD (trans);
        caps = gst_caps_ref (trans->capture_info.probed_caps);
      } else {
        pad = GST_BASE_TRANSFORM_SINK_PAD (trans);
        otherpad = GST_BASE_TRANSFORM_SRC_PAD (trans);
        caps = gst_caps_ref (trans->output_info.probed_caps);
      }

      /* Filter caps */
      gst_query_parse_caps (query, &filter);
      if (filter) {
        tmp = caps;
        caps = gst_caps_intersect_full (filter, tmp, GST_CAPS_INTERSECT_FIRST);
        gst_caps_unref (tmp);
      }

      result = gst_pad_peer_query_caps (otherpad, caps);
      result = gst_caps_make_writable (result);
      gst_caps_append (result, caps);

      GST_DEBUG_OBJECT (trans, "Query caps for %s, returning %" GST_PTR_FORMAT,
          GST_PAD_NAME (pad), result);

      gst_query_set_caps_result (query, result);
      gst_caps_unref (result);
      break;
    default:
      ret = GST_BASE_TRANSFORM_CLASS (parent_class)->query (btrans, direction,
          query);
      break;
  }

  return ret;
}

GstFlowReturn
gst_v4l2trans_update_dyn_properties (GstV4L2Trans * trans)
{
  struct v4l2_control ctrl;
  /* Flip */
  memset (&ctrl, 0, sizeof ctrl);
  ctrl.id = V4L2_CID_HFLIP;
  ctrl.value = (trans->hflip ? 1 : 0);
  if (v4l2_ioctl (trans->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    GST_ERROR_OBJECT (trans, "Output VIDIOC_S_CTRL V4L2_CID_HFLIP failed");
    return GST_FLOW_ERROR;
  }
  ctrl.id = V4L2_CID_VFLIP;
  ctrl.value = (trans->vflip ? 1 : 0);
  if (v4l2_ioctl (trans->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    GST_ERROR_OBJECT (trans, "Output VIDIOC_S_CTRL V4L2_CID_VFLIP failed");
    return GST_FLOW_ERROR;
  }
  ctrl.id = V4L2_CID_CONTRAST;
  ctrl.value = trans->contrast;
  if (v4l2_ioctl (trans->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    GST_ERROR_OBJECT (trans, "Output VIDIOC_S_CTRL V4L2_CID_CONTRAST failed");
    return GST_FLOW_ERROR;
  }
  ctrl.id = V4L2_CID_BRIGHTNESS;
  ctrl.value = trans->brightness;
  if (v4l2_ioctl (trans->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    GST_ERROR_OBJECT (trans, "Output VIDIOC_S_CTRL V4L2_CID_BRIGHTNESS failed");
    return GST_FLOW_ERROR;
  }
  ctrl.id = V4L2_CID_SATURATION;
  ctrl.value = trans->saturation;
  if (v4l2_ioctl (trans->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    GST_ERROR_OBJECT (trans, "Output VIDIOC_S_CTRL V4L2_CID_SATURATION failed");
    return GST_FLOW_ERROR;
  }
  ctrl.id = V4L2_CID_ALPHA_COMPONENT;
  ctrl.value = trans->alpha;
  if (v4l2_ioctl (trans->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    GST_ERROR_OBJECT (trans,
        "Output VIDIOC_S_CTRL V4L2_CID_ALPHA_COMPONENT failed");
    return GST_FLOW_ERROR;
  }
  return GST_FLOW_OK;
}

static GstFlowReturn
gst_v4l2trans_setup_output_no_pool (GstV4L2Trans * trans, GstVideoFrame * in)
{
  struct v4l2_format s_fmt;
  struct v4l2_selection selection;
  struct v4l2_requestbuffers reqbuf;
  gint width, height, crop_width, crop_height, type;

  /* Set format */
  width = gst_v4l2trans_align_width (&in->info);
  height = gst_v4l2trans_align_height (&in->info);
  crop_width = GST_VIDEO_INFO_WIDTH (&in->info);
  crop_height = GST_VIDEO_INFO_HEIGHT (&in->info);

  memset (&s_fmt, 0, sizeof s_fmt);
  s_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  s_fmt.fmt.pix.width = width;
  s_fmt.fmt.pix.height = height;
  s_fmt.fmt.pix.sizeimage = GST_VIDEO_INFO_SIZE (&in->info);
  s_fmt.fmt.pix.pixelformat = gst_v4l2trans_fmt_gst_to_v4l2 (&in->info);

  if (GST_VIDEO_INFO_IS_INTERLACED (&in->info)) {
    GST_DEBUG_OBJECT (trans, "interlaced video");
    /* ideally we would differentiate between types of interlaced video
     * but there is not sufficient information in the caps..
     */
    s_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
  } else {
    GST_DEBUG_OBJECT (trans, "progressive video");
    s_fmt.fmt.pix.field = V4L2_FIELD_NONE;
  }

  /* Format */
  if (v4l2_ioctl (trans->fd, VIDIOC_S_FMT, &s_fmt) < 0) {
    GST_ERROR_OBJECT (trans, "Output VIDIOC_S_FMT failed");
    return GST_FLOW_ERROR;
  }

  if (gst_v4l2trans_update_dyn_properties (trans) != GST_FLOW_OK)
    return GST_FLOW_ERROR;

  /* Crop if needed */
  if ((width != crop_width) || (height != crop_height)) {
    memset (&selection, 0, sizeof selection);
    selection.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    selection.target = V4L2_SEL_TGT_CROP;
    selection.r.width = crop_width;
    selection.r.height = crop_height;

    if (v4l2_ioctl (trans->fd, VIDIOC_S_SELECTION, &selection) < 0) {
      GST_ERROR_OBJECT (trans, "Output VIDIOC_S_SELECTION failed");
      return GST_FLOW_ERROR;
    }
  }

  /* Set number of buffers */
  memset (&reqbuf, 0, sizeof reqbuf);
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbuf.count = NB_BUF_OUTPUT_POOL;
  reqbuf.memory = V4L2_MEMORY_DMABUF;

  if (v4l2_ioctl (trans->fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
    GST_ERROR_OBJECT (trans, "Output VIDIOC_REQBUFS failed");
    return GST_FLOW_ERROR;
  }

  /* Start stream */
  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if (v4l2_ioctl (trans->fd, VIDIOC_STREAMON, &type) < 0) {
    GST_ERROR_OBJECT (trans, "Output STREAMON FAILED");
    return GST_FLOW_ERROR;
  }

  GST_DEBUG_OBJECT (trans, "Output ready : %dx%d", width, height);
  trans->out_no_pool_setup = TRUE;

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_v4l2trans_capture_start (GstV4L2Trans * trans, GstVideoFrame * out)
{
  gint type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  /* Start stream */
  if (v4l2_ioctl (trans->fd, VIDIOC_STREAMON, &type) < 0) {
    GST_ERROR_OBJECT (trans, "Capture STREAMON FAILED");
    return GST_FLOW_ERROR;
  }

  GST_DEBUG_OBJECT (trans, "Capture started : %dx%d",
      gst_v4l2trans_align_width (&out->info),
      gst_v4l2trans_align_height (&out->info));

  trans->capture_start = TRUE;
  return GST_FLOW_OK;
}

/* Return the v4l2 index corresponding to a fd.
 * First search if a fd has a corresponding v4l2 index
 * If not, add an element to the table, possibly overwriting an existing
 * element if table full
 */
static gint
gst_v4l2trans_get_capture_index (GstV4L2Trans * trans, gint fd)
{
  gint i, free = -1;

  for (i = 0; i < ARRAY_SIZE (trans->capture_info.v4l2_fd); i++) {
    if (trans->capture_info.v4l2_fd[i] == fd)
      return i;

    if ((free < 0) && (trans->capture_info.v4l2_fd[i] == -1))
      free = i;
  }

  /* Not found, add to the table */
  if (free < 0) {
    GST_WARNING_OBJECT (trans,
        "Capture_v4l2_fd table full: will overwrite for fd=%d", fd);
    free = 0;
  }

  trans->capture_info.v4l2_fd[free] = fd;
  return free;
}

/* Return the v4l2 index corresponding to a fd.
 * First search if a fd has a corresponding v4l2 index
 * If not, add an element to the table, possibly overwriting an existing
 * element if table full
 */
static gint
gst_v4l2trans_get_output_index (GstV4L2Trans * trans, gint fd)
{
  gint i, free = -1;

  for (i = 0; i < ARRAY_SIZE (trans->output_info.v4l2_fd); i++) {
    if (trans->output_info.v4l2_fd[i] == fd)
      return i;

    if ((free < 0) && (trans->output_info.v4l2_fd[i] == -1))
      free = i;
  }

  /* Not found, add to the table */
  if (free < 0) {
    GST_WARNING_OBJECT (trans,
        "Output_v4l2_fd table full: will overwrite for fd=%d", fd);
    free = 0;
  }

  trans->output_info.v4l2_fd[free] = fd;
  return free;
}

static GstFlowReturn
gst_v4l2trans_output_no_pool_process (GstV4L2Trans * trans, GstVideoFrame * in,
    gint * v4l2_index)
{
  GstFlowReturn ret;
  GstBuffer *buf;
  struct v4l2_buffer out_buf;
  GstMemory *gmem;
  gint fd;

  /* Not using our buffer pool. Will use dmabuf fd for 0-copy */
  if (!trans->out_no_pool_setup) {
    ret = gst_v4l2trans_setup_output_no_pool (trans, in);
    if (ret != GST_FLOW_OK) {
      GST_ERROR_OBJECT (trans, "Cannot setup output");
      return ret;
    }
  }

  /* Output: inject input dmabuf fd */
  buf = in->buffer;
  gmem = gst_buffer_get_memory (buf, 0);
  if (!gst_is_dmabuf_memory (gmem)) {
    GST_ERROR_OBJECT (trans, "Input buffer is not a DMA-Buf buffer");

    gst_memory_unref (gmem);
    return GST_FLOW_ERROR;
  }

  /* DMABUF: use fd */
  fd = gst_dmabuf_memory_get_fd (gmem);
  gst_memory_unref (gmem);

  /* Output: queue buffer */
  memset (&out_buf, 0, sizeof out_buf);
  out_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  out_buf.memory = V4L2_MEMORY_DMABUF;
  out_buf.m.fd = fd;
  out_buf.index = gst_v4l2trans_get_output_index (trans, fd);
  out_buf.bytesused = gst_buffer_get_size (buf);

  if (v4l2_ioctl (trans->fd, VIDIOC_QBUF, &out_buf) < 0) {
    GST_ERROR_OBJECT (trans, "Output QBUF FAILED");
    return GST_FLOW_ERROR;
  }
  GST_DEBUG_OBJECT (trans, "Enqueued output buffer %p, index %d",
      buf, out_buf.index);

  *v4l2_index = out_buf.index;  /* for DQBUF after transform being done */
  return GST_FLOW_OK;
}

static GstFlowReturn
gst_v4l2trans_output_no_pool_finish (GstV4L2Trans * trans, gint v4l2_index)
{
  /* Output: dequeue buffer */
  struct v4l2_buffer out_buf;

  memset (&out_buf, 0, sizeof out_buf);
  out_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  out_buf.memory = V4L2_MEMORY_DMABUF;
  out_buf.index = v4l2_index;
  if (v4l2_ioctl (trans->fd, VIDIOC_DQBUF, &out_buf) < 0) {
    GST_ERROR_OBJECT (trans, "Output DQBUF FAILED");
    return GST_FLOW_ERROR;
  }
  GST_DEBUG_OBJECT (trans, "Dequeued output buffer index %d", out_buf.index);
  return GST_FLOW_OK;
};

static GstFlowReturn
gst_v4l2trans_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * in, GstVideoFrame * out)
{
  GstV4L2Trans *trans = GST_V4L2TRANS (filter);
  GstBuffer *buf;
  GstFlowReturn ret;
  GstMemory *gmem;
  struct v4l2_buffer cap_buf;
  gint fd;
  gboolean from_our_pool = FALSE;
  guint8 *gstdata;
  gsize gstsize;
  void *ptr;
  gint v4l2_output_index = -1;

  GST_DEBUG_OBJECT (trans, "Transforming");

  /* Capture: get buffer properties */
  buf = out->buffer;
  gmem = gst_buffer_get_memory (buf, 0);
  if (gst_is_dmabuf_memory (gmem)) {
    /* DMABUF: use fd */
    fd = gst_dmabuf_memory_get_fd (gmem);

    /* Check buffer pool of the capture buffer */
    if (buf->pool == trans->capture_info.pool)
      from_our_pool = !trans->capture_is_down_pool;
    else {
      GST_ERROR_OBJECT (trans, "Capture buffer %p from unknown pool %p %s",
          buf, buf->pool, buf->pool->object.name);

      gst_memory_unref (gmem);
      return GST_FLOW_ERROR;
    }
    GST_DEBUG_OBJECT (trans, "FAST path output");
  } else {
    GST_DEBUG_OBJECT (trans, "The capture buffer %p is not a DMABUF one,"
        "will use SLOW path output", buf);
    fd = -1;
  }
  gst_memory_unref (gmem);

  /* Capture: queue buffer */
  memset (&cap_buf, 0, sizeof cap_buf);
  if ((from_our_pool) || (fd == -1))
    cap_buf.memory = V4L2_MEMORY_MMAP;
  else
    cap_buf.memory = V4L2_MEMORY_DMABUF;

  cap_buf.m.fd = fd;
  cap_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  cap_buf.bytesused = gst_buffer_get_size (out->buffer);
  cap_buf.length = cap_buf.bytesused;
  if (fd == -1) {
    /* We can use a unique index as no DMABUF optimization possible here */
    cap_buf.index = 0;
  } else {
    /* We have to keep m.fd <-> index consistency to get V4L2 DMABUF working */
    cap_buf.index = gst_v4l2trans_get_capture_index (trans, fd);
  }

  if (v4l2_ioctl (trans->fd, VIDIOC_QBUF, &cap_buf) < 0) {
    GST_ERROR_OBJECT (trans, "Capture QBUF FAILED");
    return GST_FLOW_ERROR;
  }
  GST_DEBUG_OBJECT (trans, "Enqueued capture buffer %p fd= %d index=%d", buf,
      cap_buf.m.fd, cap_buf.index);

  /* Capture: start (if not done yet) */
  if (!trans->capture_start) {
    ret = gst_v4l2trans_capture_start (trans, out);
    if (ret != GST_FLOW_OK)
      return ret;
  }

  /* Output: queue buffer */
  if (in->buffer->pool != trans->output_info.pool) {
    /* Not using our output buffer pool : will use dmabuf fd of buffer for 0-copy */
    GST_DEBUG_OBJECT (trans, "FAST path input (not our pool)");
    ret = gst_v4l2trans_output_no_pool_process (trans, in, &v4l2_output_index);
    if (ret != GST_FLOW_OK) {
      GST_ERROR_OBJECT (trans, "Failed to process output buffer");
      return ret;
    }
  } else {
    /* Using our output buffer pool: will use dmabuf without copy */
    GST_DEBUG_OBJECT (trans, "FAST path input (our pool)");
    ret = gst_v4l2_out_buf_pool_process (trans->output_info.pool, in->buffer);
    if (ret != GST_FLOW_OK) {
      GST_ERROR_OBJECT (trans, "Queuing output frame failed");
      return ret;
    }
  }

  /* Capture: dequeue the processed buffer. This call blocks until the HW
   * transformation is actually done */
  if (v4l2_ioctl (trans->fd, VIDIOC_DQBUF, &cap_buf) < 0) {
    GST_ERROR_OBJECT (trans, "Capture DQBUF FAILED");
    return GST_FLOW_ERROR;
  }
  GST_DEBUG_OBJECT (trans, "Dequeued capture buffer %p fd=%d index=%d", buf,
      cap_buf.m.fd, cap_buf.index);

  /* Capture non-DMABUF: map and copy V4L2 buffer into GST buffer */
  if (fd == -1) {
    if (v4l2_ioctl (trans->fd, VIDIOC_QUERYBUF, &cap_buf) < 0) {
      GST_ERROR_OBJECT (trans, "Capture VIDIOC_QUERYBUF failed");
      return GST_FLOW_ERROR;
    }

    ptr = v4l2_mmap (NULL, cap_buf.length, PROT_READ | PROT_WRITE,
        MAP_SHARED, trans->fd, cap_buf.m.offset);
    if (ptr == MAP_FAILED) {
      GST_ERROR_OBJECT (trans, "Capture MMAP FAILED");
      return GST_FLOW_ERROR;
    }

    gstdata = GST_VIDEO_FRAME_PLANE_DATA (out, 0);
    gstsize = GST_VIDEO_FRAME_SIZE (out);
    memcpy (gstdata, ptr, gstsize);

    v4l2_munmap (ptr, cap_buf.length);
    GST_DEBUG_OBJECT (trans, "Buffer copied");
  }

  /* Output not from our pool : dequeue buffer */
  if (v4l2_output_index != -1) {
    ret = gst_v4l2trans_output_no_pool_finish (trans, v4l2_output_index);
    if (ret != GST_FLOW_OK) {
      GST_ERROR_OBJECT (trans, "Failed to finish output buffer");
      return ret;
    }
  }

  GST_DEBUG_OBJECT (trans, "Transforming done");
  return GST_FLOW_OK;
}

static gboolean
gst_v4l2trans_check_crop_properties (gint * param,
    const GValue * value, gboolean pix_algnmt_8pix)
{
  gint user_val = g_value_get_int (value);
  gint aligned_value;

  if (pix_algnmt_8pix)
    aligned_value = ALIGN_DIM_ON_8_BYTES_PLUS (user_val);
  else
    aligned_value = ALIGN_DIM_ON_16_BYTES_PLUS (user_val);

  if (aligned_value != user_val) {
    GST_WARNING ("To fit alignment hw constraints, crop parameter has been "
        "fixed to %d instead of %d", aligned_value, user_val);
  }

  if (*param != aligned_value) {
    /* Properties must be 16/8 pixels aligned */
    *param = aligned_value;
    return TRUE;
  }
  return FALSE;
}

void
gst_v4l2trans_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstV4L2Trans *trans = GST_V4L2TRANS (object);
  gboolean passthrough_disable = FALSE;
  guint prev;
  gint sprev;


  GST_DEBUG_OBJECT (trans, "Setting property %d", prop_id);

  switch (prop_id) {
    case PROP_HFLIP:
      trans->hflip = g_value_get_boolean (value);
      break;
    case PROP_VFLIP:
      trans->vflip = g_value_get_boolean (value);
      break;
    case PROP_BRIGHTNESS:
      sprev = trans->brightness;
      trans->brightness = g_value_get_int (value);
      passthrough_disable = (trans->brightness != sprev);
      break;
    case PROP_CONTRAST:
      sprev = trans->contrast;
      trans->contrast = g_value_get_int (value);
      passthrough_disable = (trans->contrast != sprev);
      break;
    case PROP_SATURATION:
      sprev = trans->saturation;
      trans->saturation = g_value_get_int (value);
      passthrough_disable = (trans->saturation != sprev);
      break;
    case PROP_ALPHA:
      prev = trans->alpha;
      trans->alpha = g_value_get_int (value);
      passthrough_disable = (trans->alpha != prev);
      break;
    case PROP_CROP_LEFT:
      if (gst_v4l2trans_check_crop_properties (&trans->crop.left, value, FALSE))
        trans->user_crop |= (1 << prop_id);
      break;
    case PROP_CROP_TOP:
      if (gst_v4l2trans_check_crop_properties (&trans->crop.top, value, FALSE))
        trans->user_crop |= (1 << prop_id);
      break;
    case PROP_CROP_RIGHT:
      /* Even if the hardware allows to support 8 pixels alignment on right and
       * bottom info, we keep 16 pixels alignment to be inline with highest
       * constraint (top and left info)
       */
      if (gst_v4l2trans_check_crop_properties (&trans->crop.right, value,
              FALSE))
        trans->user_crop |= (1 << prop_id);
      break;
    case PROP_CROP_BOTTOM:
      if (gst_v4l2trans_check_crop_properties (&trans->crop.bottom, value,
              FALSE))
        trans->user_crop |= (1 << prop_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }

  if (trans->hflip || trans->vflip || trans->user_crop)
    passthrough_disable = TRUE;

  if (passthrough_disable) {
    GST_DEBUG_OBJECT (trans, "disable passthrough");

    GST_BASE_TRANSFORM_GET_CLASS (trans)->passthrough_on_same_caps = FALSE;
    gst_base_transform_set_passthrough (GST_BASE_TRANSFORM (trans), FALSE);
  }
  gst_v4l2trans_update_dyn_properties (trans);
}

void
gst_v4l2trans_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstV4L2Trans *trans = GST_V4L2TRANS (object);
  GST_DEBUG_OBJECT (trans, "Getting property");

  switch (prop_id) {
    case PROP_HFLIP:
      g_value_set_boolean (value, trans->hflip);
      break;
    case PROP_VFLIP:
      g_value_set_boolean (value, trans->vflip);
      break;
    case PROP_CONTRAST:
      g_value_set_int (value, trans->contrast);
      break;
    case PROP_BRIGHTNESS:
      g_value_set_int (value, trans->brightness);
      break;
    case PROP_SATURATION:
      g_value_set_int (value, trans->saturation);
      break;
    case PROP_ALPHA:
      g_value_set_int (value, trans->alpha);
      break;
    case PROP_CROP_LEFT:
      g_value_set_int (value, trans->crop.left);
      break;
    case PROP_CROP_RIGHT:
      g_value_set_int (value, trans->crop.right);
      break;
    case PROP_CROP_TOP:
      g_value_set_int (value, trans->crop.top);
      break;
    case PROP_CROP_BOTTOM:
      g_value_set_int (value, trans->crop.bottom);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstStateChangeReturn
gst_v4l2trans_change_state (GstElement * element, GstStateChange transition)
{
  GstV4L2Trans *trans = GST_V4L2TRANS (element);
  GstStateChangeReturn ret;

  if (transition == GST_STATE_CHANGE_NULL_TO_READY)
    if (!gst_v4l2trans_open_device (trans))
      return GST_STATE_CHANGE_FAILURE;

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  if (transition == GST_STATE_CHANGE_READY_TO_NULL)
    gst_v4l2trans_close_device (trans);

  return ret;
}

static void
gst_v4l2trans_init (GstV4L2Trans * trans)
{
  if (!trans)
    return;
  memset (&trans->capture_info, 0, sizeof (trans->capture_info));
  memset (&trans->output_info, 0, sizeof (trans->output_info));
  trans->out_no_pool_setup = FALSE;
  trans->capture_is_down_pool = FALSE;
  trans->capture_start = FALSE;
  trans->hflip = FALSE;
  trans->vflip = FALSE;
  trans->contrast = DEFAULT_CONTRAST;
  trans->brightness = DEFAULT_BRIGHTNESS;
  trans->saturation = DEFAULT_SATURATION;
  trans->alpha = DEFAULT_ALPHA;
  trans->crop.left = trans->crop.top = 0;
  trans->crop.right = trans->crop.bottom = 0;
}

static void
gst_v4l2trans_class_init (GstV4L2TransClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseTransformClass *gstbasetransform_class;
  GstVideoFilterClass *gstvideofilter_class;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gstbasetransform_class = (GstBaseTransformClass *) klass;
  gstvideofilter_class = (GstVideoFilterClass *) klass;


  gst_element_class_add_pad_template (element_class,
      gst_v4l2trans_sink_template_factory ());
  gst_element_class_add_pad_template (element_class,
      gst_v4l2trans_src_template_factory ());

  gstbasetransform_class->stop = GST_DEBUG_FUNCPTR (gst_v4l2trans_stop);
  gstbasetransform_class->decide_allocation =
      GST_DEBUG_FUNCPTR (gst_v4l2trans_decide_allocation);
  gstbasetransform_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_v4l2trans_propose_allocation);
  gstbasetransform_class->set_caps = GST_DEBUG_FUNCPTR (gst_v4l2trans_set_caps);
  gstbasetransform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_v4l2trans_transform_caps);
  gstbasetransform_class->fixate_caps =
      GST_DEBUG_FUNCPTR (gst_v4l2trans_fixate_caps);
  gstbasetransform_class->passthrough_on_same_caps = TRUE;
  gstbasetransform_class->query = GST_DEBUG_FUNCPTR (gst_v4l2trans_query);

  gstvideofilter_class->transform_frame =
      GST_DEBUG_FUNCPTR (gst_v4l2trans_transform_frame);

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_v4l2trans_finalize);
  gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_v4l2trans_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_v4l2trans_get_property);

  element_class->change_state = GST_DEBUG_FUNCPTR (gst_v4l2trans_change_state);

  g_object_class_install_property (gobject_class, PROP_HFLIP,
      g_param_spec_boolean ("hflip", "hflip", "Horizontal flip", 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_VFLIP,
      g_param_spec_boolean ("vflip", "vflip", "Vertical flip", 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CONTRAST,
      g_param_spec_int ("contrast", "contrast",
          "Adjust the contrast of the ouput when the output "
          "format is a ARGB/ABGR format. A negative value will decrease "
          "the contrast of the output picture while a positive one will "
          "increase it",
          MIN_CONTRAST, MAX_CONTRAST, DEFAULT_CONTRAST,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BRIGHTNESS,
      g_param_spec_int ("brightness", "brightness",
          "Adjust the brightness level of the ouput when the output "
          "format is a ARGB/ABGR format. A negative value will "
          "decrease the brightness level of the output picture while a "
          "positive one will increase it",
          MIN_BRIGHTNESS, MAX_BRIGHTNESS, DEFAULT_BRIGHTNESS,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SATURATION,
      g_param_spec_int ("saturation", "saturation",
          "Adjusts the color saturation of the output picture when the output "
          "format is a ARGB/ABGR format. A negative "
          "value will decrease the amount of color in the output picture "
          "while a positive one will increase it.",
          MIN_SATURATION, MAX_SATURATION, DEFAULT_SATURATION,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ALPHA,
      g_param_spec_int ("alpha", "alpha",
          "Sets the alpha channel value to the output picture when the output "
          "format is a ARGB/ABGR format",
          MIN_ALPHA, MAX_ALPHA, DEFAULT_ALPHA,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CROP_LEFT,
      g_param_spec_int ("left", "Left",
          "Pixels to crop at left (will be aligned on 16 pixels)", 0, G_MAXINT,
          0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CROP_RIGHT,
      g_param_spec_int ("right", "Right",
          "Pixels to crop at right  (will be aligned on 16 pixels)", 0,
          G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CROP_TOP,
      g_param_spec_int ("top", "Top",
          "Pixels to crop at top (will be aligned on 16 pixels)", 0, G_MAXINT,
          0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CROP_BOTTOM,
      g_param_spec_int ("bottom", "Bottom",
          "Pixels to crop at bottom (will be aligned on 16 pixels)", 0,
          G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  GST_DEBUG_CATEGORY_INIT (gst_v4l2trans_debug, "v4l2trans", 0,
      "ST V4L2 transformer");

  gst_element_class_set_static_metadata (element_class,
      "V4L2 transform", "Video/Transform",
      "A V4L2 transformer", "STMicroelectronics");
}

gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "v4l2trans", GST_RANK_PRIMARY + 1,
          GST_TYPE_V4L2TRANS))
    return FALSE;
  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    v4l2trans,
    "ST V4L2 transform",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
