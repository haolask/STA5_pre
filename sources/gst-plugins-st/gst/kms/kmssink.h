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

#ifndef __GST_KMSSINK_H__
#define __GST_KMSSINK_H__

#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libkms/libkms.h>

G_BEGIN_DECLS
#define GST_TYPE_KMS_SINK (gst_kms_sink_get_type())
#define GST_KMS_SINK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_KMS_SINK, GstKMSSink))
#define GST_KMS_SINK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_KMS_SINK, GstKMSSinkClass))
#define GST_IS_KMS_SINK(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_KMS_SINK))
#define GST_IS_KMS_SINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_KMS_SINK))
typedef struct _GstKMSSink GstKMSSink;
typedef struct _GstKMSSinkClass GstKMSSinkClass;
typedef struct _GstKMSMeta GstKMSMeta;

/**
 * GstKMSMeta -
 * @handle: drm buffer handle
 * @stride: buffer stride
 * @offset: buffer offset
 * @fb_id:  drm buffer ID
 * @fourcc: drm fourcc
 */
struct _GstKMSMeta
{
  GstMeta meta;
  guint32 handle[4];
  guint32 stride[4];
  guint32 offset[4];
  gint fb_id;
  guint fourcc;
  struct kms_bo *bo;
};

/**
 * GstKMSSink:
 * @videosink: video sink (width, height, ...)
 * @format: pixel format
 * @drm: drm handler
 * @kms: pointer to kms driver
 * @resources: drm ressources (list of crtc/connectors/encoders)
 * @mode: selected mode
 * @display: targeted display
 * @in_plane: shall the sink be in a DRM plane
 * @plane_x: x coordinate of the plane (when in_plane)
 * @plane_y: y coordinate of the plane (when in_plane)
 * @plane_scale: plane scaling if supported
 * @plane_width: plane output width
 * @plane_height: plane output height
 * @plane_zorder: plane z-order (when in_plane)
 * @plane_rotation: plane rotation (when in_plane)
 * @prim_plane_rotation: main plane rotation
 * @max_buffers: maximun buffers to allocate (0 means unlimited)
 * @mode_name: targeted mode
 * @vsync: shall vsync be used or not
 * @dst_w: frame width modified according to pixel aspect ratio
 * @dst_h: frame height modified according to pixel aspect ratio
 * @flow_lock: mutex used to protect data flow from external calls
 * @crtc_id: selected crtc
 * @pipe_id: kernel index of the selected crtc (kernel pipe index)
 * @plane_id: selected plane (sink through DRM plane only)
 * @connector_id: ID of selected connector
 * @backgnd_fb_id: ID of the background FB when a plane is used
 * @mode_set: whether the mode has been set or not
 * @disp_rect: rectangle specifying the display size
 * @cur_buff: buffer being actually displayed
 * @evctx :drm event context used to set the vblank handler
 * @info: frame informations
 * @pool: buffer pool
 */
struct _GstKMSSink
{
  /* the object with are derivated from */
  GstVideoSink videosink;
  GstVideoFormat format;

  gint drm;
  struct kms_driver *kms;
  drmModeResPtr resources;
  drmModeModeInfoPtr mode;

  guint display;
  gboolean in_plane;
  guint plane_x;
  guint plane_y;
  gfloat plane_scale;
  guint plane_width;
  guint plane_height;
  guint plane_number;
  gint plane_zorder;
  gint plane_rotation;
  gint prim_plane_rotation;
  guint max_buffers;
  gchar *mode_name;
  gboolean vsync;
  guint dst_w;
  guint dst_h;

  GMutex flow_lock;
  guint crtc_id;
  guint pipe_id;
  guint plane_id;
  guint connector_id;
  guint backgnd_fb_id;
  gboolean mode_set;
  GstVideoRectangle disp_rect;
  GstBuffer *cur_buff;
  drmEventContext evctx;

  GstVideoInfo info;
  GstBufferPool *pool;
  gchar *driver_name;
};

struct _GstKMSSinkClass
{
  GstVideoSinkClass parent_class;
};

GType gst_kms_sink_get_type (void);

G_END_DECLS
#endif /* __GST_KMSSINK_H__ */
