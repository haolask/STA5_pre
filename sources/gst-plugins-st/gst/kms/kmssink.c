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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>

#include <gst/video/video.h>

#include "gst/allocators/gstdmabuf.h"

#include "kmssink.h"
#include "kmspool.h"

#define DEFAULT_MODE 0
#define DEFAULT_DISPLAY 0
#define DEFAULT_MAX_BUFFERS 0           /*0 means unlimited */
#define DEFAULT_IN_PLANE 1
#define DEFAULT_PLANE_X (-1)
#define DEFAULT_PLANE_Y (-1)
#define DEFAULT_PLANE_SCALE 1
#define DEFAULT_PLANE_WIDTH 0
#define DEFAULT_PLANE_HEIGHT 0
#define DEFAULT_PLANE_NUMBER 0
#define DEFAULT_PLANE_ZORDER (-1)
#define DEFAULT_PLANE_ROTATION (-1)
#define DEFAULT_PRIM_PLANE_ROTATION (-1)
#define DEFAULT_VSYNC 1

#define DRM_PROP_NAME_PLANE_ZORDER_STIH   "zpos"

#define DRM_PROP_NAME_PLANE_ZORDER_MTB    "renoir_plane_zorder"
#define DRM_PROP_NAME_PLANE_ROTATION_MTB  "renoir_plane_rotation"
#define DRM_PROP_NAME_PRIM_PLANE_ROTATION_MTB "renoir_crtc_rotation"

#define gst_kms_sink_parent_class parent_class
G_DEFINE_TYPE (GstKMSSink, gst_kms_sink, GST_TYPE_VIDEO_SINK);

GST_DEBUG_CATEGORY (gst_debug_kms_sink);
GST_DEBUG_CATEGORY (gst_debug_kms_pool);

#define GST_CAT_DEFAULT gst_debug_kms_sink
enum
{
  PROP_0,
  PROP_DRIVER_NAME,
  PROP_DISPLAY,
  PROP_MODE,
  PROP_MAX_BUFFERS,
  PROP_IN_PLANE,
  PROP_PLANE_X,
  PROP_PLANE_Y,
  PROP_PLANE_SCALE,
  PROP_PLANE_WIDTH,
  PROP_PLANE_HEIGHT,
  PROP_PLANE_NUMBER,
  PROP_PLANE_ZORDER,
  PROP_PLANE_ROTATION,
  PROP_PRIM_PLANE_ROTATION,
  PROP_VSYNC,
};

#define GST_VIDEO_FORMATS "{NV12, BGRx, BGRA, BGR, RGB16}"

static GstStaticPadTemplate kms_sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (
        GST_VIDEO_CAPS_MAKE_WITH_FEATURES (GST_CAPS_FEATURE_MEMORY_DMABUF, GST_VIDEO_FORMATS) ";"
        GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS))
    );

static gboolean gst_kms_sink_prepare_plane (GstKMSSink *kmssink);

static gboolean
gst_kms_sink_propose_allocation (GstBaseSink * bsink, GstQuery * query)
{
  GstKMSSink *sink = GST_KMS_SINK (bsink);
  GstBufferPool *pool;
  GstStructure *config;
  GstCaps *caps;
  guint size;
  gboolean need_pool;
  GstAllocator *allocator;
  GstAllocationParams params;

  gst_allocation_params_init (&params);
  gst_query_parse_allocation (query, &caps, &need_pool);

  if (caps == NULL)
    goto no_caps;

  if ((pool = sink->pool))
    gst_object_ref (pool);

  if (pool != NULL) {
    GstCaps *pcaps;

    /* we had a pool, check caps */
    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_get_params (config, &pcaps, &size, NULL, NULL);

    if (!gst_caps_is_equal (caps, pcaps)) {
      /* different caps, we can't use this pool */
      gst_object_unref (pool);
      GST_DEBUG_OBJECT (sink, "pool has different caps");
      pool = NULL;
    }
    gst_structure_free (config);
  }

  if (pool == NULL && need_pool) {
    GstVideoInfo info;

    if (!gst_video_info_from_caps (&info, caps))
      goto invalid_caps;

    GST_DEBUG_OBJECT (sink, "create new pool");
    pool = gst_kms_pool_new (caps, sink, sink->max_buffers);
    /* the normal size of a frame */
    size = info.size;

    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_set_params (config, caps, size, 3,
        sink->max_buffers);
    gst_buffer_pool_config_set_allocator (config, NULL, &params);
    if (!gst_buffer_pool_set_config (pool, config))
      goto config_failed;
  }

  if (pool) {
    gst_query_add_allocation_pool (query, pool, size, 3,
        sink->max_buffers);
    gst_object_unref (pool);

    /* DMA-BUF allocator */
    allocator = gst_dmabuf_allocator_new ();
    gst_query_add_allocation_param (query, allocator, &params);
    gst_object_unref (allocator);
  }

  /* We also support video metadata (alignment) */
  gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL);

  return TRUE;

  /* ERRORS */
no_caps:
  {
    GST_DEBUG_OBJECT (bsink, "no caps specified");
    return FALSE;
  }
invalid_caps:
  {
    GST_DEBUG_OBJECT (bsink, "invalid caps specified");
    return FALSE;
  }
config_failed:
  {
    GST_DEBUG_OBJECT (bsink, "failed setting config");
    gst_object_unref (pool);
    return FALSE;
  }
}

/**
 * gst_kms_sink_apply_par:
 * @bsink: base sink
 * @info: input video info including width, height and pixel-aspect-ratio
 * @min_w, max_w: minimum width, maximum width
 * @min_h, max_h: minimum height, maximum height
 * @w: output width
 * @h: output height
 *
 * Computes some width and height from an input size, applying
 * pixel-aspect-ratio and fitting in a size range
 *
 * Returns: %TRUE if w and h can be successfully computed
 */
static gboolean
gst_kms_sink_apply_par (GstBaseSink * bsink, GstVideoInfo * info,
    guint min_w, guint max_w, guint min_h, guint max_h,
    guint * w, guint * h)
{
  gint from_w, from_h, from_par_n, from_par_d, from_dar_n, from_dar_d;

  from_w = GST_VIDEO_INFO_WIDTH (info);
  from_h = GST_VIDEO_INFO_HEIGHT (info);
  from_par_n = GST_VIDEO_INFO_PAR_N (info);
  from_par_d = GST_VIDEO_INFO_PAR_D (info);

  GST_DEBUG_OBJECT (bsink, "Applying PAR conversion from %dx%d with PAR=%d/%d "
      "withing range [%d - %d] x [%d - %d]",
      from_w, from_h, from_par_n, from_par_d, min_w, max_w, min_h, max_h);

  if (!from_par_n) {
    from_par_n = 1;
    from_par_d = 1;
  }

  /* Calculate input DAR */
  if (!gst_util_fraction_multiply (from_w, from_h, from_par_n, from_par_d,
          &from_dar_n, &from_dar_d)) {
    GST_DEBUG_OBJECT (bsink, "Cannot calculate the DAR");
    return FALSE;
  }

  /* Try to keep the input height (because of interlacing) */
  *h = CLAMP (from_h, min_h, max_h);
  *w = (guint) gst_util_uint64_scale_int (*h, from_dar_n, from_dar_d);

  if ((*w >= min_w) && (*w <= max_w))
    goto success;

  /* The former failed, try to keep the input width */
  *w = CLAMP (from_w, min_w, max_w);
  *h = (guint) gst_util_uint64_scale_int (*w, from_dar_d, from_dar_n);

  if ((*h >= min_h) && (*h <= max_h))
    goto success;

  /* Could not answer successfully to the request */
  GST_DEBUG_OBJECT (bsink, "Cannot compute");
  return FALSE;

success:
  GST_DEBUG_OBJECT (bsink, "Returning %dx%d", *w, *h);
  return TRUE;
}

static GstCaps *
gst_kms_sink_getcaps (GstBaseSink * bsink, GstCaps * filter)
{
  GstKMSSink *kmssink = GST_KMS_SINK (bsink);
  GstCaps *caps;
  gint i, j;
  gboolean nv12_support = FALSE;
  drmModePlaneResPtr plane_res;
  drmModePlanePtr plane;

  GST_DEBUG_OBJECT (kmssink, "KMS sink get caps");

  /* get a template copy of the sinkpad caps */
  caps = gst_pad_get_pad_template_caps (GST_BASE_SINK (kmssink)->sinkpad);
  caps = gst_caps_make_writable (caps);

  if (kmssink->drm > 0) {
    GValue list = G_VALUE_INIT;
    GValue value = G_VALUE_INIT;

    g_value_init (&list, GST_TYPE_LIST);
    g_value_init (&value, G_TYPE_STRING);

    /* build the caps according to hw caps */
    /* Check NV12 support */
    plane_res = drmModeGetPlaneResources(kmssink->drm);

     for (i = 0; i < plane_res->count_planes; i++) {
       plane = drmModeGetPlane(kmssink->drm, plane_res->planes[i]);
       if (!plane)
         continue;

       for (j = 0; j < plane->count_formats; j++)
         if (!strncmp("NV12", (char *) &plane->formats[j], 4))
           nv12_support = TRUE;

       drmModeFreePlane(plane);
     }

    /* Add NV12 if in-plane and supported by HW */
    if ((nv12_support) && kmssink->in_plane) {
      g_value_set_string (&value, "NV12");
      gst_value_list_append_value (&list, &value);
    }

    g_value_set_string (&value, "BGRx");
    gst_value_list_append_value (&list, &value);

    g_value_set_string (&value, "BGRA");
    gst_value_list_append_value (&list, &value);

    g_value_set_string (&value, "BGR");
    gst_value_list_append_value (&list, &value);

    g_value_set_string (&value, "RGB16");
    gst_value_list_append_value (&list, &value);

    gst_structure_set_value (gst_caps_get_structure (caps, 0), "format", &list);
    gst_structure_set_value (gst_caps_get_structure (caps, 1), "format", &list);
  }

  if (!kmssink->disp_rect.w || !kmssink->disp_rect.w)
    goto out;

  /* change width and height for all format */
  for (i = 0; i < gst_caps_get_size (caps); ++i) {
    GstStructure *structure = gst_caps_get_structure (caps, i);
    if (kmssink->in_plane) {
      gint maxw, maxh;

      maxw = kmssink->disp_rect.w / kmssink->plane_scale;
      /* Note : accept 1088 frames in HD mode */
      maxh = (kmssink->disp_rect.h == 1080) ? 1088 : kmssink->disp_rect.h;
      maxh /= kmssink->plane_scale;

      gst_structure_set (structure,
                         "width", GST_TYPE_INT_RANGE, 1, maxw,
                         "height", GST_TYPE_INT_RANGE, 1, maxh, NULL);
    } else {
      gst_structure_set (structure,
                         "width", G_TYPE_INT, kmssink->disp_rect.w,
                         "height", G_TYPE_INT, kmssink->disp_rect.h, NULL);
    }
  }

out:
  if (filter) {
    GstCaps *intersection =
        gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (caps);
    caps = intersection;
  }

  GST_DEBUG_OBJECT (kmssink, "returned caps %" GST_PTR_FORMAT, caps);
  return caps;
}

static gboolean
gst_kms_sink_setcaps (GstBaseSink * bsink, GstCaps * caps)
{
  GstVideoInfo info;
  GstKMSSink *kmssink = GST_KMS_SINK (bsink);
  guint dst_w = 0, dst_h = 0;
  guint max_w, max_h;

  GST_DEBUG_OBJECT (kmssink, "KMS sink set caps %" GST_PTR_FORMAT, caps);

  if (!gst_video_info_from_caps (&info, caps))
    goto invalid_format;

  /* Size */
  if (kmssink->in_plane) {
    /* accept up to full screen size, including h=1088 for HD */
    max_w = kmssink->disp_rect.w;
    max_h = (kmssink->disp_rect.h == 1080) ? 1088 : kmssink->disp_rect.h;

    /* Apply pixel aspect ratio */
    if (!gst_kms_sink_apply_par (bsink, &info, 0, max_w, 0, max_h, &dst_w, &dst_h))
      goto renegociate;

    /* Check size with plane scaling */
    if ((dst_w * kmssink->plane_scale > max_w) ||
        (dst_h * kmssink->plane_scale > max_h)) {
      GST_DEBUG_OBJECT (kmssink, "Scaled frame won't fit inside display");
      goto renegociate;
    }
  } else {
    /* no scaling and ignore PAR, only accept full screen stretching */
    dst_w = GST_VIDEO_INFO_WIDTH (&info);
    dst_h = GST_VIDEO_INFO_HEIGHT (&info);
    if ((dst_w != kmssink->disp_rect.w) || (dst_h != kmssink->disp_rect.h))
      goto renegociate;
  }

  /* the caps know now, reset kmssink info */
  gst_video_info_from_caps (&kmssink->info, caps);
  GST_VIDEO_SINK_WIDTH (kmssink) = GST_VIDEO_INFO_WIDTH (&info);
  GST_VIDEO_SINK_HEIGHT (kmssink) = GST_VIDEO_INFO_HEIGHT (&info);
  kmssink->dst_w = dst_w;
  kmssink->dst_h = dst_h;
  kmssink->format = GST_VIDEO_INFO_FORMAT (&info);

  /* Get a DRM plane */
  if (kmssink->in_plane)
    if (!gst_kms_sink_prepare_plane (kmssink))
      goto plane_failed;

  if (kmssink->pool)
    gst_object_unref (kmssink->pool);

  kmssink->pool = gst_kms_pool_new (caps, kmssink, kmssink->max_buffers);
  if (!kmssink->pool)
    goto invalid_pool;

  return TRUE;

plane_failed:
  GST_ELEMENT_ERROR (kmssink, CORE, NEGOTIATION, (NULL),
      ("Can't get any suitable DRM plane"));
  return FALSE;

invalid_pool:
  GST_ELEMENT_ERROR (kmssink, CORE, NEGOTIATION, (NULL),
      ("Can't get a valid pool"));
  return FALSE;

invalid_format:
  GST_ELEMENT_ERROR (kmssink, CORE, NEGOTIATION, (NULL),
      ("Could not get image format from caps%" GST_PTR_FORMAT, caps));
  return FALSE;

renegociate:
  GST_ELEMENT_ERROR (kmssink, CORE, NEGOTIATION, (NULL),
      ("mismatch between caps and image format"));
  return FALSE;
}

static gboolean
gst_kms_sink_prepare_plane (GstKMSSink *kmssink)
{
  /* Get a DRM plane */
  guint32 i, j, x, y;
  guint32 handles[4], strides[4], offsets[4] = { 0 };
  guint fourcc_format;
  gint ret;
  gint nb_suitable_planes = 0;
  drmModePlaneResPtr plane_res;
  drmModePlanePtr plane;
  struct kms_bo *bo;
  guint8 *bg_fb;
  unsigned attrs[7] = {
      KMS_WIDTH, 0,
      KMS_HEIGHT, 0,
      KMS_BO_TYPE, KMS_BO_TYPE_SCANOUT_X8R8G8B8,
      KMS_TERMINATE_PROP_LIST,
    };

  /* Get a DRM plane */
  plane_res = drmModeGetPlaneResources(kmssink->drm);
  if (!plane_res)
    return FALSE;

  /* Check supported format */
  fourcc_format = format_gst_to_fourcc (kmssink->format);

  for (i = 0; i < plane_res->count_planes && !kmssink->plane_id; i++) {
    plane = drmModeGetPlane(kmssink->drm, plane_res->planes[i]);
    if (!plane)
      continue;

    for (j = 0; j < plane->count_formats; j++)
      if (fourcc_format == plane->formats[j]) {
        if (nb_suitable_planes < kmssink->plane_number)
          nb_suitable_planes++;
        else {
          kmssink->plane_id = plane->plane_id;
          break;
        }
      }

    drmModeFreePlane(plane);
  }

  if (!kmssink->plane_id)
    return FALSE;
  GST_DEBUG_OBJECT (kmssink, "found DRM plane %d", kmssink->plane_id);

  /* Get background buffer */
  attrs[1] = kmssink->disp_rect.w;
  attrs[3] = kmssink->disp_rect.h;
  ret = kms_bo_create(kmssink->kms, attrs, &bo);
  ret &= kms_bo_map(bo, (void *)&bg_fb);
  ret &= kms_bo_get_prop(bo, KMS_HANDLE, &handles[0]);
  ret &= kms_bo_get_prop(bo, KMS_PITCH, &strides[0]);
  ret &= drmModeAddFB2(kmssink->drm, attrs[1], attrs[3], DRM_FORMAT_ARGB8888,
                       handles, strides, offsets, &kmssink->backgnd_fb_id, 0);
  if (ret < 0) {
    GST_DEBUG_OBJECT (kmssink, "sink_prepare_plane failed : %d", ret);
    return FALSE;
  }

  for (y = 0; y < attrs[3]; y++) {
    for (x = 0; x < attrs[1]; x++)
      ((guint32 *)bg_fb)[x] = (0xE0) << 24 | (x & 0xFF) << 16 | (y & 0xFF) << 8 |
                              ((x + y) & 0xFF);
    bg_fb += strides[0];
  }

  return TRUE;
}

static gint
gst_kms_sink_set_drm_prop (gint fd, guint obj_id, guint obj_type,
    gchar *prop_name, guint prop_val)
{
  drmModeObjectProperties *obj_props;
  drmModePropertyRes *prop;
  gint i, prop_id = -1;

  obj_props = drmModeObjectGetProperties(fd, obj_id, obj_type);

  if (!obj_props)
    return -1;

  for (i = 0; i < obj_props->count_props; i++) {
    prop = drmModeGetProperty(fd, obj_props->props[i]);
    if (prop && !strcmp(prop->name, prop_name)) {
      prop_id = prop->prop_id;
      break;
    }
  }

  drmModeFreeObjectProperties(obj_props);

  if (prop_id < 0)
    return -1;

  return drmModeObjectSetProperty(fd, obj_id, obj_type, prop_id, prop_val);
}

/* Open DRM without specifying the driver name */
static gint
gst_kms_sink_open_drm (GstKMSSink * kmssink)
{
  struct udev *udev;
  struct udev_enumerate *e;
  struct udev_device *drm_device;
  const gchar *path, *filename;
  drm_client_t client;
  gint ret, fd;

  udev = udev_new ();
  if (!udev)
    return -1;

  e = udev_enumerate_new (udev);
  udev_enumerate_add_match_subsystem (e, "drm");
  udev_enumerate_add_match_sysname (e, "card0");
  udev_enumerate_scan_devices (e);
  path = udev_list_entry_get_name (udev_enumerate_get_list_entry (e));
  GST_DEBUG_OBJECT (kmssink, "Found %s\n", path);
  drm_device = udev_device_new_from_syspath (udev, path);
  udev_enumerate_unref (e);

  if (!drm_device) {
    udev_unref (udev);
    return -1;
  }

  /* Open DRM device */
  filename = udev_device_get_devnode (drm_device);

  fd = open (filename, O_RDWR | O_CLOEXEC);
  udev_device_unref (drm_device);
  udev_unref (udev);

  if (fd < 0)
    return -1;

  /* Check that we're the only opener and authed. */
  client.idx = 0;
  ret = drmIoctl (fd, DRM_IOCTL_GET_CLIENT, &client);
  if ((ret) || (!client.auth))
    goto err_close;

  client.idx = 1;
  ret = drmIoctl (fd, DRM_IOCTL_GET_CLIENT, &client);
  if (ret != -1 || errno != EINVAL)
    goto err_close;

  return fd;
err_close:
  close (fd);
  return -1;
}

static gboolean
gst_kms_sink_start (GstBaseSink * bsink)
{
  drmModeEncoderPtr encoder;
  drmModeConnectorPtr connector;
  gchar mode_resolution[DRM_DISPLAY_MODE_LEN];
  guint32 mode_vrefresh = 0;
  gchar *p, *endp;
  guint32 i, n;
  gint ret;

  GstKMSSink *kmssink = GST_KMS_SINK (bsink);

  GST_DEBUG_OBJECT (kmssink, "start KMS sink");

  if (kmssink->driver_name)
    kmssink->drm = drmOpen (kmssink->driver_name, NULL);
  else
    kmssink->drm = gst_kms_sink_open_drm (kmssink);

  if (kmssink->drm < 0)
    goto open_failed;

  ret = kms_create (kmssink->drm, &kmssink->kms);
  if (ret < 0)
    goto kms_failed;

  kmssink->resources = drmModeGetResources (kmssink->drm);
  if (!kmssink->resources)
    goto resources_failed;

  if (!kmssink->resources->count_connectors)
    goto no_connectors;

  if (kmssink->display >= kmssink->resources->count_connectors) {
    GST_WARNING_OBJECT (kmssink, "change display ID to 0");
    GST_WARNING_OBJECT (kmssink, "count connector %d",
        kmssink->resources->count_connectors);
    kmssink->display = 0;
  }

  connector = drmModeGetConnector (kmssink->drm,
      kmssink->resources->connectors[kmssink->display]);
  if (!connector)
    goto no_connectors;
  if (!connector->count_modes)
    goto no_connectors;

  kmssink->connector_id = connector->connector_id;

  kmssink->mode = &connector->modes[0];

  /* Get resolution and vertical refresh frequency of the mode */
  strcpy(mode_resolution, kmssink->mode_name);
  p = strstr(kmssink->mode_name, "-");
  if (p != NULL) {
    mode_resolution[p - kmssink->mode_name] = '\0';
    mode_vrefresh = strtoul(p + 1, &endp, 10);
  }

  for (i = 0; i < connector->count_modes; i++)
    if (!strcmp(connector->modes[i].name, mode_resolution)) {
      kmssink->mode = &connector->modes[i];
      /* If the vertical refresh frequency is not specified then return the
       * first mode that match with the name. Else, return the mode that match
       * the name and the specified vertical refresh frequency.
       */
      if (mode_vrefresh == 0)
        break;
      else if (kmssink->mode->vrefresh == mode_vrefresh)
        break;
    }

  kmssink->disp_rect.w = kmssink->mode->hdisplay;
  kmssink->disp_rect.h = kmssink->mode->vdisplay;

  encoder = drmModeGetEncoder (kmssink->drm, connector->encoders[0]);
  if (!encoder)
    goto no_encoder;

  i = 0;
  n = encoder->possible_crtcs;
  while (!(n & 1)) {
    n >>= 1;
    i++;
  }
  kmssink->crtc_id = kmssink->resources->crtcs[i];
  kmssink->pipe_id = i;

  if (kmssink->plane_scale != 1.0f &&
      (kmssink->plane_width || kmssink->plane_height)) {
    GST_WARNING_OBJECT (kmssink, "ignoring plane-scale");
    kmssink->plane_scale = 1.0f;
  }

  return TRUE;

open_failed:
  GST_ELEMENT_ERROR (kmssink, RESOURCE, FAILED, (NULL),
      ("drmOpen failed: %s (%d)", strerror (errno), errno));
  goto fail;

resources_failed:
  GST_ELEMENT_ERROR (kmssink, RESOURCE, FAILED, (NULL),
      ("drmModeGetResources failed: %s (%d)", strerror (errno), errno));
  goto fail;

no_connectors:
  GST_ELEMENT_ERROR (kmssink, RESOURCE, FAILED, ("no connectors"), (NULL));
  goto fail;

no_encoder:
  GST_ELEMENT_ERROR (kmssink, RESOURCE, FAILED, ("no encoders"), (NULL));
  goto fail;

kms_failed:
  GST_ELEMENT_ERROR (kmssink, RESOURCE, FAILED, (NULL),
      ("kms_create failed: %s (%d)", strerror (errno), errno));
  goto fail;

fail:
  return FALSE;
}

static gboolean
gst_kms_sink_stop (GstBaseSink * bsink)
{
  GstKMSSink *kmssink = GST_KMS_SINK (bsink);
  GST_DEBUG_OBJECT (kmssink, "stop KMS sink");

  /*TODO: add all clean up operations here */
  if (kmssink->cur_buff) {
    gst_buffer_unref (kmssink->cur_buff);
    kmssink->cur_buff = NULL;
  }

  if (kmssink->kms)
    kms_destroy (&kmssink->kms);

  if (kmssink->resources)
    drmModeFreeResources (kmssink->resources);

  if (kmssink->drm != -1) {
    if (kmssink->plane_id)
      drmModeSetPlane(kmssink->drm, kmssink->plane_id, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    drmModeSetCrtc (kmssink->drm, kmssink->crtc_id, 0, 0, 0, 0, 0, NULL);

    close (kmssink->drm);
    kmssink->drm = -1;
  }
  return FALSE;
}

static void
gst_kms_sink_vblank_handler(int fd, unsigned int frame,
    unsigned int sec, unsigned int usec, void *data)
{
  GstKMSSink *kmssink = data;

  /* Free the buffer previously displayed */
  if (kmssink->cur_buff) {
        GST_DEBUG_OBJECT (kmssink, "unreffing %p", kmssink->cur_buff);
        gst_buffer_unref (kmssink->cur_buff);
      }
}

static GstMemory *
get_cached_kmsmeta (GstMemory * mem)
{
  return gst_mini_object_get_qdata (GST_MINI_OBJECT (mem),
      g_quark_from_static_string ("kmsmeta"));
}

static void
set_cached_kmsmeta (GstMemory * mem, GstMemory * kmsmeta)
{
  return gst_mini_object_set_qdata (GST_MINI_OBJECT (mem),
      g_quark_from_static_string ("kmsmeta"), kmsmeta,
      NULL);
}

static GstFlowReturn
gst_kms_sink_show_frame (GstVideoSink * vsink, GstBuffer * buf)
{
  GstKMSSink *kmssink = GST_KMS_SINK (vsink);
  GstKMSMeta *meta = GST_KMS_META_GET (buf);
  GstVideoRectangle src, dst_scaled, result;
  GstMemory *mem;
  gint ret;
  gboolean interlaced;

  GST_DEBUG_OBJECT (kmssink, "KMS sink show frame %p", buf);

  g_mutex_lock (&kmssink->flow_lock);

  if (meta) {
    GST_DEBUG_OBJECT (kmssink, "Buffer has meta (handle %d)", meta->handle[0]);
  } else {
    /* No meta (buffer not from kms pool), try to retrieve some from cache */
    mem = gst_buffer_peek_memory (buf, 0);
    meta = (GstKMSMeta *) get_cached_kmsmeta (mem);
    if (meta)
      GST_DEBUG_OBJECT (kmssink, "Retrieved KMS meta from cache (handle %d)",
          meta->handle[0]);
  }

  if (!meta) {
    /* No meta, adding some */
    gint i, fd, nmem, nplanes;
    GstVideoMeta *vmeta;
    uint32_t handle[4];

    vmeta = gst_buffer_get_video_meta (buf);
    if (!vmeta) {
      GST_ERROR_OBJECT (kmssink, "No video meta for this buffer");
      goto error;
    }

    nmem = gst_buffer_n_memory (buf);
    nplanes = vmeta->n_planes;

    if (nmem == nplanes) {
      /* one fd/handle per plane */
      for (i = 0; i < nplanes; i++) {
        mem = gst_buffer_peek_memory (buf, i);
        fd = gst_dmabuf_memory_get_fd (mem);
        if (fd <= 0) {
          GST_ERROR_OBJECT (kmssink, "Cannot get fd for this buffer");
          goto error;
        }

        if (drmPrimeFDToHandle (kmssink->drm, fd, &handle[i])) {
          GST_ERROR_OBJECT (kmssink, "Cannot get handle for this buffer");
          goto error;
        }
      }
    } else {
      /* one common fd/handle for all planes */
      mem = gst_buffer_peek_memory (buf, 0);
      fd = gst_dmabuf_memory_get_fd (mem);
      if (fd <= 0) {
        GST_ERROR_OBJECT (kmssink, "Cannot get fd for this buffer");
        goto error;
      }

      if (drmPrimeFDToHandle (kmssink->drm, fd, &handle[0])) {
        GST_ERROR_OBJECT (kmssink, "Cannot get handle for this buffer");
        goto error;
      }

      for (i = 1; i < nplanes; i++)
        handle[i] = handle[0];
    }

    /* Create meta and cache it */
    meta = g_slice_new0 (GstKMSMeta);

    meta->fb_id = -1;
    meta->fourcc = format_gst_to_fourcc (vmeta->format);

    for (i = 0; i < nplanes; i++) {
      meta->stride[i] = vmeta->stride[i];
      meta->offset[i] = vmeta->offset[i];
      meta->handle[i] = handle[i];
    }
    set_cached_kmsmeta (mem, GST_MEMORY_CAST (meta));

    GST_DEBUG_OBJECT (kmssink, "Added KMS meta (handle %d) in cache",
        handle[0]);
  }

  interlaced = GST_BUFFER_FLAG_IS_SET (buf, GST_VIDEO_BUFFER_FLAG_INTERLACED);

  if (meta->fb_id == -1) {
    guint flags = 0;

    if (interlaced) {
      GST_DEBUG_OBJECT (kmssink, "Interlaced buffer");
      flags = DRM_MODE_FB_INTERLACED;

      if (!GST_BUFFER_FLAG_IS_SET (buf, GST_VIDEO_BUFFER_FLAG_TFF)) {
        GST_DEBUG_OBJECT (kmssink, "with bottom field first");
        flags |= DRM_MODE_FB_BFF;
      }
    }

    drmModeAddFB2 (kmssink->drm,
        GST_VIDEO_SINK_WIDTH (kmssink), GST_VIDEO_SINK_HEIGHT (kmssink),
        meta->fourcc, meta->handle, meta->stride, meta->offset,
        (uint32_t *) & meta->fb_id, flags);

    GST_DEBUG_OBJECT (kmssink, "Buffer %p has drm fb id %d", buf, meta->fb_id);
  }

  /* Set primary plane rotation */
  if (kmssink->prim_plane_rotation >= 0)
    if (gst_kms_sink_set_drm_prop (kmssink->drm, kmssink->crtc_id,
        DRM_MODE_OBJECT_CRTC, (gchar *) DRM_PROP_NAME_PRIM_PLANE_ROTATION_MTB,
        kmssink->prim_plane_rotation))
      GST_ERROR_OBJECT (kmssink, "Failed to set primary plane rotation");

  if (!kmssink->in_plane) {
    ret = drmModeSetCrtc (kmssink->drm, kmssink->crtc_id, meta->fb_id, 0, 0,
                          &kmssink->connector_id, 1, kmssink->mode);
    if (ret) {
      GST_ERROR_OBJECT (kmssink, "Failed to set mode: %s", kmssink->mode->name);
      goto error;
    }

    kmssink->mode_set = TRUE;
  } else {
    if (!kmssink->mode_set) {
      ret = drmModeSetCrtc (kmssink->drm, kmssink->crtc_id, kmssink->backgnd_fb_id, 0, 0,
	                    &kmssink->connector_id, 1, kmssink->mode);
      if (ret) {
	GST_ERROR_OBJECT (kmssink, "Failed to set mode: %s", kmssink->mode->name);
        goto error;
      }

      kmssink->mode_set = TRUE;
    }

    src.w = GST_VIDEO_SINK_WIDTH (kmssink);
    src.h = GST_VIDEO_SINK_HEIGHT (kmssink);

    dst_scaled.w = kmssink->plane_width ? kmssink->plane_width :
                   kmssink->dst_w * kmssink->plane_scale;
    dst_scaled.h = kmssink->plane_height ? kmssink->plane_height :
                   kmssink->dst_h * kmssink->plane_scale;

    if ((kmssink->plane_x == -1) || (kmssink->plane_y == -1))
      gst_video_sink_center_rect (dst_scaled , kmssink->disp_rect, &result, FALSE);
    else {
      result = dst_scaled;
      result.x = kmssink->plane_x;
      result.y = kmssink->plane_y;
    }

    /* Set plane zorder, trying different DRM prop names */
    if (kmssink->plane_zorder >= 0)
      if (gst_kms_sink_set_drm_prop (kmssink->drm, kmssink->plane_id,
          DRM_MODE_OBJECT_PLANE, (gchar *) DRM_PROP_NAME_PLANE_ZORDER_STIH,
          kmssink->plane_zorder))
        if (gst_kms_sink_set_drm_prop (kmssink->drm, kmssink->plane_id,
            DRM_MODE_OBJECT_PLANE, (gchar *) DRM_PROP_NAME_PLANE_ZORDER_MTB,
            kmssink->plane_zorder))
          GST_ERROR_OBJECT (kmssink, "Failed to set plane zorder");

    /* Set plane rotation */
    if (kmssink->plane_rotation >= 0)
      if (gst_kms_sink_set_drm_prop (kmssink->drm, kmssink->plane_id,
          DRM_MODE_OBJECT_PLANE, (gchar *) DRM_PROP_NAME_PLANE_ROTATION_MTB,
          kmssink->plane_rotation))
        GST_ERROR_OBJECT (kmssink, "Failed to set plane rotation");

    ret = drmModeSetPlane(kmssink->drm, kmssink->plane_id, kmssink->crtc_id, meta->fb_id, 0,
	                  result.x , result.y, result.w, result.h,
	                  0, 0, src.w << 16, src.h << 16);

    if (ret == -EBUSY) {
      GST_WARNING_OBJECT (kmssink, "Failed to set plane (busy)");
    } else if (ret) {
      GST_ERROR_OBJECT (kmssink, "Failed to set plane (%d)", ret);
      goto error;
    }
  }

  if (kmssink->vsync) {
    /* Wait for Next VSYNC */
    drmVBlank vbl;

    vbl.request.type = DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT;
    if (kmssink->pipe_id != 0)
      vbl.request.type |= DRM_VBLANK_SECONDARY;
    vbl.request.sequence = interlaced ? 2 : 1;
    vbl.request.signal = (unsigned long)kmssink;
    ret = drmWaitVBlank(kmssink->drm, &vbl);
    if (ret) {
      GST_ERROR_OBJECT (kmssink, "Failed to wait for vblank");
      goto error;
    }

    ret = drmHandleEvent(kmssink->drm, &kmssink->evctx);
    if (ret) {
      GST_ERROR_OBJECT (kmssink, "Failed to handle vblank event");
      goto error;
    }
    /* Store a reference to the  buff on display */
    GST_DEBUG_OBJECT (kmssink, "reffing %p", buf);
    kmssink->cur_buff = gst_buffer_ref (buf);
  }

  g_mutex_unlock (&kmssink->flow_lock);
  return GST_FLOW_OK;

error:
  g_mutex_unlock (&kmssink->flow_lock);
  return GST_FLOW_ERROR;
}

static gboolean
gst_kms_sink_set_driver (GstKMSSink * kmssink, const gchar * driver,
    GError ** error)
{
  /* we store the filename as we received it from the application. On Windows
   * this should be in UTF8 */
  g_free (kmssink->driver_name);
  kmssink->driver_name = g_strdup (driver);
  GST_INFO ("driver_name : %s", kmssink->driver_name);
  return TRUE;
}


static void
gst_kms_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstKMSSink *kmssink = GST_KMS_SINK (object);
  GST_DEBUG_OBJECT (kmssink, "KMS sink set property");

  switch (prop_id) {
    case PROP_DRIVER_NAME:
      gst_kms_sink_set_driver (kmssink, g_value_get_string (value), NULL);
      break;
    case PROP_DISPLAY:
      kmssink->display = g_value_get_uint (value);
      break;
    case PROP_MODE:
      g_free (kmssink->mode_name);
      kmssink->mode_name = g_strdup (g_value_get_string (value));
      break;
    case PROP_MAX_BUFFERS:
      kmssink->max_buffers = g_value_get_uint (value);
      break;
    case PROP_IN_PLANE:
      kmssink->in_plane = g_value_get_boolean (value);
      break;
    case PROP_PLANE_X:
      kmssink->plane_x = g_value_get_int (value);
      break;
    case PROP_PLANE_Y:
      kmssink->plane_y = g_value_get_int (value);
      break;
    case PROP_PLANE_SCALE:
      kmssink->plane_scale = g_value_get_float (value);
      break;
    case PROP_PLANE_WIDTH:
      kmssink->plane_width = g_value_get_int (value);
      break;
    case PROP_PLANE_HEIGHT:
      kmssink->plane_height = g_value_get_int (value);
      break;
    case PROP_PLANE_NUMBER:
      kmssink->plane_number = g_value_get_int (value);
      break;
    case PROP_PLANE_ZORDER:
      kmssink->plane_zorder = g_value_get_int (value);
      break;
    case PROP_PLANE_ROTATION:
      kmssink->plane_rotation = g_value_get_int (value);
      break;
    case PROP_PRIM_PLANE_ROTATION:
      kmssink->prim_plane_rotation = g_value_get_int (value);
      break;
    case PROP_VSYNC:
      kmssink->vsync = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_kms_sink_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstKMSSink *kmssink = GST_KMS_SINK (object);
  GST_DEBUG_OBJECT (kmssink, "KMS sink get property");

  switch (prop_id) {
    case PROP_DRIVER_NAME:
      g_value_set_string (value, kmssink->driver_name);
      break;
    case PROP_DISPLAY:
      g_value_set_uint (value, kmssink->display);
      break;
    case PROP_MODE:
      g_value_set_string (value, kmssink->mode_name);
      break;
    case PROP_MAX_BUFFERS:
      g_value_set_uint (value, kmssink->max_buffers);
      break;
    case PROP_IN_PLANE:
      g_value_set_boolean (value, kmssink->in_plane);
      break;
    case PROP_PLANE_X:
      g_value_set_int (value, kmssink->plane_x);
      break;
    case PROP_PLANE_Y:
      g_value_set_int (value, kmssink->plane_y);
      break;
    case PROP_PLANE_SCALE:
      g_value_set_float (value, kmssink->plane_scale);
      break;
    case PROP_PLANE_WIDTH:
      g_value_set_int (value, kmssink->plane_width);
      break;
    case PROP_PLANE_HEIGHT:
      g_value_set_int (value, kmssink->plane_height);
      break;
    case PROP_PLANE_NUMBER:
      g_value_set_int (value, kmssink->plane_number);
      break;
    case PROP_PLANE_ZORDER:
      g_value_set_int (value, kmssink->plane_zorder);
      break;
    case PROP_PLANE_ROTATION:
      g_value_set_int (value, kmssink->plane_rotation);
      break;
    case PROP_PRIM_PLANE_ROTATION:
      g_value_set_int (value, kmssink->prim_plane_rotation);
      break;
    case PROP_VSYNC:
      g_value_set_boolean (value, kmssink->vsync);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_kms_sink_init (GstKMSSink * kmssink)
{
  kmssink->drm = -1;
  kmssink->mode_name = g_strdup ("");
  kmssink->display = DEFAULT_DISPLAY;
  kmssink->in_plane = DEFAULT_IN_PLANE;
  kmssink->max_buffers = DEFAULT_MAX_BUFFERS;
  kmssink->plane_x = DEFAULT_PLANE_X;
  kmssink->plane_y = DEFAULT_PLANE_Y;
  kmssink->plane_scale = DEFAULT_PLANE_SCALE;
  kmssink->plane_width = DEFAULT_PLANE_WIDTH;
  kmssink->plane_height = DEFAULT_PLANE_HEIGHT;
  kmssink->plane_number = DEFAULT_PLANE_NUMBER;
  kmssink->plane_zorder = DEFAULT_PLANE_ZORDER;
  kmssink->plane_rotation = DEFAULT_PLANE_ROTATION;
  kmssink->prim_plane_rotation = DEFAULT_PRIM_PLANE_ROTATION;
  kmssink->vsync = DEFAULT_VSYNC;

  kmssink->kms = NULL;
  kmssink->connector_id = 0;
  kmssink->pool = NULL;
  kmssink->driver_name = NULL;
  kmssink->cur_buff = NULL;

  /* Set up event handler */
  memset(&kmssink->evctx, 0, sizeof(kmssink->evctx));
  kmssink->evctx.version = DRM_EVENT_CONTEXT_VERSION;
  kmssink->evctx.vblank_handler = gst_kms_sink_vblank_handler;

  g_mutex_init (&kmssink->flow_lock);
}

static void
gst_kms_sink_class_init (GstKMSSinkClass * klass)
{
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;
  GstVideoSinkClass *videosink_class;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;
  videosink_class = (GstVideoSinkClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_kms_sink_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_kms_sink_get_property);

  g_object_class_install_property (gobject_class, PROP_DRIVER_NAME,
      g_param_spec_string ("driver", "driver name",
          "name of driver to use", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DISPLAY,
      g_param_spec_uint ("display", "targeted display",
          "number of targeted display", 0, UINT_MAX, 0,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MODE,
      g_param_spec_string ("mode", "mode",
          "mode name (ex: 1920x1080 or 1920x1080-24 to force to select the 24Hz mode)", NULL,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_MAX_BUFFERS,
      g_param_spec_uint ("max-buffers", "max buffers allocated",
          "max allocated buffers (0 meaning unlimited)", 0, UINT_MAX,
          DEFAULT_MAX_BUFFERS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_IN_PLANE,
      g_param_spec_boolean ("in-plane", "in-plane",
          "in a DRM plane or not", DEFAULT_IN_PLANE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_X,
      g_param_spec_int ("plane-x", "plane-x",
          "x coordinate (when in-plane / -1 meaning centered)", -1, INT_MAX,
          DEFAULT_PLANE_X, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_Y,
      g_param_spec_int ("plane-y", "plane-y",
          "y coordinate (when in-plane / -1 meaning centered)", -1, INT_MAX,
          DEFAULT_PLANE_Y, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_SCALE,
      g_param_spec_float ("plane-scale", "plane-scale",
          "plane scaling (when in-plane) ex: 0.5", 0, 10,
          DEFAULT_PLANE_SCALE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_WIDTH,
      g_param_spec_int ("plane-width", "plane-width",
          "plane width (when in-plane). Do not use with plane-scale", -1, INT_MAX,
          DEFAULT_PLANE_WIDTH, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_HEIGHT,
      g_param_spec_int ("plane-height", "plane-height",
          "plane height (when in-plane). Do not use with plane-scale", -1, INT_MAX,
          DEFAULT_PLANE_HEIGHT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_NUMBER,
      g_param_spec_int ("plane-number", "plane-number",
          "specifies which plane to use, default 0 is first one", 0, INT_MAX,
          DEFAULT_PLANE_NUMBER, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_ZORDER,
      g_param_spec_int ("plane-zorder", "plane-zorder",
          "plane z-order (when in-plane)", -1, INT_MAX,
          DEFAULT_PLANE_ZORDER, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PLANE_ROTATION,
      g_param_spec_int ("plane-rotation", "plane-rotation",
          "plane rotation (when in-plane)", -1, 359,
          DEFAULT_PLANE_ROTATION, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_PRIM_PLANE_ROTATION,
      g_param_spec_int ("primaryplane-rotation", "primaryplane-rotation",
          "primary plane rotation", -1, 359,
          DEFAULT_PRIM_PLANE_ROTATION, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_VSYNC,
      g_param_spec_boolean ("vsync", "vsync",
          "vertical synchro enabled or not", DEFAULT_VSYNC,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (gstelement_class,
      "KMS sink", "Sink/Video",
      "A standard KMS based on videosink", "STMicroelectronics");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&kms_sink_template));

  gstbasesink_class->start = GST_DEBUG_FUNCPTR (gst_kms_sink_start);
  gstbasesink_class->stop = GST_DEBUG_FUNCPTR (gst_kms_sink_stop);
  gstbasesink_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_kms_sink_propose_allocation);
  gstbasesink_class->get_caps = GST_DEBUG_FUNCPTR (gst_kms_sink_getcaps);
  gstbasesink_class->set_caps = GST_DEBUG_FUNCPTR (gst_kms_sink_setcaps);

  videosink_class->show_frame = GST_DEBUG_FUNCPTR (gst_kms_sink_show_frame);
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  gboolean ret;

  GST_DEBUG_CATEGORY_INIT (gst_debug_kms_sink, "kmssink", 0,
      "kms sink element debug");

  GST_DEBUG_CATEGORY_INIT (gst_debug_kms_pool, "kmspool", 0, "kms pool debug");

  ret = gst_element_register (plugin, "kmssink", GST_RANK_NONE,
      GST_TYPE_KMS_SINK);

  return ret;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    kmssink,
    "kms sink, shall only be used for internal ST tests",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
