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

#ifndef  GSTV4L2TRANS_H
#define  GSTV4L2TRANS_H

#include <gst/gst.h>
#include <gst/video/gstvideofilter.h>

#include <linux/videodev2.h>

#ifdef HAVE_LIBV4L2
#include <libv4l2.h>
#else
#define v4l2_fd_open(fd, flags) (fd)
#define v4l2_close    close
#define v4l2_dup      dup
#define v4l2_ioctl    ioctl
#define v4l2_read     read
#define v4l2_mmap     mmap
#define v4l2_munmap   munmap
#endif

/* Number of buffers in output/capture pool  */
#define NB_BUF_OUTPUT_POOL 30
#define NB_BUF_CAPTURE_POOL 6

/* Color conversion default values */
#define DEFAULT_BRIGHTNESS  0
#define MIN_BRIGHTNESS  -128
#define MAX_BRIGHTNESS  127

#define DEFAULT_CONTRAST    0
#define MIN_CONTRAST  -64
#define MAX_CONTRAST  64

#define DEFAULT_SATURATION  0
#define MIN_SATURATION  -64
#define MAX_SATURATION  128

#define DEFAULT_ALPHA       255
#define MIN_ALPHA  0
#define MAX_ALPHA  255

#define ALIGN_DIM_ON_16_BYTES(x)  (((x) >> 4) << 4)
#define ALIGN_DIM_ON_8_BYTES(x)  (((x) >> 3) << 3)

static inline gint
gstv4l2trans_16_align_dim_up (gint x)
{
  if ((x % 16) == 0)
    return x;
  return ((((x) >> 4) + 1) << 4);
}

static inline gint
gstv4l2trans_8_align_dim_up (gint x)
{
  if ((x % 8) == 0)
    return x;
  return ((((x) >> 3) + 1) << 3);
}

#define ALIGN_DIM_ON_16_BYTES_PLUS(x)  gstv4l2trans_16_align_dim_up(x)
#define ALIGN_DIM_ON_8_BYTES_PLUS(x)  gstv4l2trans_8_align_dim_up(x)

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_V4L2TRANS (gst_v4l2trans_get_type())
#define GST_V4L2TRANS(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_V4L2TRANS, GstV4L2Trans))
#define GST_V4L2TRANS_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_V4L2TRANS, GstV4L2TransClass))
#define GST_IS_V4L2TRANS(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_V4L2TRANS))
#define GST_IS_V4L2TRANS_CLASS(obj) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_V4L2TRANS))
typedef struct _GstV4L2Trans GstV4L2Trans;
typedef struct _GstV4L2TransClass GstV4L2TransClass;
typedef struct _GstV4L2CommonInfo GstV4L2CommonInfo;

/**
 * _GstV4L2CommonInfo:
 * @probed_caps:           caps actually supported by the driver
 * @pool:                  Memory pool for incoming/outgoing frames. We own this pool and share
 *                         it with the upstream/downstream element.
 * @nbbuf:                 Number of buffers in the above pool
 * @is_s_fmt_done:         to track if driver has been informed about the negociated format
 * @v4l2_fd:       Correspondence table between a v4l2 index and a fd
 */

struct _GstV4L2CommonInfo
{
  GstCaps *probed_caps;
  GstBufferPool *pool;
  int nbbuf;
  gboolean is_s_fmt_done;
  gint v4l2_fd[MAX (NB_BUF_OUTPUT_POOL, NB_BUF_CAPTURE_POOL)];
};

struct _GstV4L2Crop
{
  gint left;
  gint right;
  gint top;
  gint bottom;
};
/**
 * _GstV4L2Trans:
 * @parent:                Element parent
 * @fd:                    V4L2 driver file descriptor
 * @capture_info:          info linked to capture pad
 * @output_info:           info linked to capture pad
 * @out_no_pool_setup:     boolean for v4l2 output setup
 *
 * @capture_is_down_pool:  Just to keep in mind if the downstream element owns
 *                         this pool and shares it with us.
 * @capture_start:         Is the capture started
 * @align:                 Used to store capture buffer alignment info.
 * @hflip:                 Horizontal flip
 * @vflip:                 Vertical flip
 */
struct _GstV4L2Trans
{
  GstVideoFilter parent;
  gint fd;

  GstV4L2CommonInfo capture_info;
  GstV4L2CommonInfo output_info;

  gboolean out_no_pool_setup;
  gboolean capture_is_down_pool;
  gboolean capture_start;
  GstVideoAlignment align;

  gboolean hflip;
  gboolean vflip;
  gint brightness;
  gint contrast;
  gint saturation;
  guint alpha;
  struct _GstV4L2Crop crop;
  guint user_crop;
};

struct _GstV4L2TransClass
{
  GstVideoFilterClass parent_class;
};

GType gst_v4l2trans_get_type (void);
GstFlowReturn gst_v4l2trans_update_dyn_properties (GstV4L2Trans * trans);
GstVideoInterlaceMode gst_interlace_mode_from_string (const gchar * mode);
G_END_DECLS
#endif /* GSTV4L2TRANS_H */
