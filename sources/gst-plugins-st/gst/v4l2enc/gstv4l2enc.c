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
#include "config.h"
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include <linux/videodev2.h>

#include <gst/gst.h>

#include "gstv4l2enc.h"

/* FIXME
 * - crop (1920x1080 example)
 * - error management
 * - errors handling
 * - is a thread needed to dequeue access units ?
 */

GST_DEBUG_CATEGORY (gst_v4l2enc_debug);
#define GST_CAT_DEFAULT gst_v4l2enc_debug

enum
{
  PROP_0,

  PROP_GOP_SIZE,
  PROP_CPB_SIZE,
  PROP_BITRATE,
  PROP_BITRATE_MODE,
  PROP_DCT8x8,
  PROP_CABAC,
  PROP_QPMIN,
  PROP_QPMAX,
  PROP_LAST
};

enum
{
  V4L2ENC_BMODE_VBR,
  V4L2ENC_BMODE_CBR
};

#define V4L2ENC_FLAG_NONE 0x0000
#define V4L2ENC_FLAG_GOP_SIZE 0x0001
#define V4L2ENC_FLAG_CPB_SIZE 0x0002
#define V4L2ENC_FLAG_BITRATE 0x0004
#define V4L2ENC_FLAG_MIN_QP 0x0008
#define V4L2ENC_FLAG_MAX_QP  0x0010

static enum v4l2_mpeg_video_h264_profile
to_v4l2_profile (GstStructure * s)
{
  const gchar *profile;
  gboolean intra_profile = FALSE;

  profile = gst_structure_get_string (s, "profile");
  if (!profile)
    goto bail;

  if (g_str_has_suffix (profile, "-intra"))
    intra_profile = TRUE;

  if (!strcmp (profile, "baseline"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
  else if (!strcmp (profile, "constrained-baseline"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_CONSTRAINED_BASELINE;
  else if (!strcmp (profile, "main"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;
  else if (!strcmp (profile, "extended"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_EXTENDED;
  else if (g_str_has_prefix (profile, "high"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_HIGH;
  else if (g_str_has_prefix (profile, "high-10"))
    return intra_profile ?
        V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10_INTRA :
        V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_10;
  else if (g_str_has_prefix (profile, "high-4:2:2"))
    return intra_profile ?
        V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_422_INTRA :
        V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_422;
  else if (g_str_has_prefix (profile, "high-4:4:4"))
    return intra_profile ?
        V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_444_INTRA :
        V4L2_MPEG_VIDEO_H264_PROFILE_HIGH_444_PREDICTIVE;
  else if (g_str_has_prefix (profile, "cavlc-4:4:4"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_CAVLC_444_INTRA;
  else if (!strcmp (profile, "stereo"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_STEREO_HIGH;
  else if (!strcmp (profile, "multiview"))
    return V4L2_MPEG_VIDEO_H264_PROFILE_MULTIVIEW_HIGH;

bail:
  GST_WARNING ("Invalid profile %s, default to baseline profile", profile);
  return V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE;
}

static enum v4l2_mpeg_video_h264_level
to_v4l2_level (GstStructure * s)
{
  const gchar *level;

  level = gst_structure_get_string (s, "level");
  if (!level)
    goto bail;

  if (!strcmp (level, "1"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_1_0;
  else if (!strcmp (level, "1b"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_1B;
  else if (!strcmp (level, "1.1"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_1_1;
  else if (!strcmp (level, "1.2"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_1_2;
  else if (!strcmp (level, "1.3"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_1_3;
  else if (!strcmp (level, "2"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_2_0;
  else if (!strcmp (level, "2.1"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_2_1;
  else if (!strcmp (level, "2.2"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_2_2;
  else if (!strcmp (level, "3"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_3_0;
  else if (!strcmp (level, "3.1"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_3_1;
  else if (!strcmp (level, "3.2"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_3_2;
  else if (!strcmp (level, "4"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_4_0;
  else if (!strcmp (level, "4.1"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_4_1;
  else if (!strcmp (level, "4.2"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_4_2;
  else if (!strcmp (level, "5"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_5_0;
  else if (!strcmp (level, "5.1"))
    return V4L2_MPEG_VIDEO_H264_LEVEL_5_1;

bail:
  GST_WARNING ("Invalid level %s, default to level 4.2", level);
  return V4L2_MPEG_VIDEO_H264_LEVEL_4_2;
}


static enum v4l2_mpeg_video_h264_vui_sar_idc
to_v4l2_pixel_aspect_ratio (GstVideoInfo * info)
{
  gint num = GST_VIDEO_INFO_PAR_N (info);
  gint den = GST_VIDEO_INFO_PAR_D (info);

  if ((num == 1) && (den == 1))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_1x1;
  else if ((num == 12) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_12x11;
  else if ((num == 10) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_10x11;
  else if ((num == 16) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_16x11;
  else if ((num == 40) && (den == 33))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_40x33;
  else if ((num == 24) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_24x11;
  else if ((num == 20) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_20x11;
  else if ((num == 32) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_32x11;
  else if ((num == 80) && (den == 33))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_80x33;
  else if ((num == 18) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_18x11;
  else if ((num == 15) && (den == 11))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_15x11;
  else if ((num == 64) && (den == 33))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_64x33;
  else if ((num == 160) && (den == 99))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_160x99;
  else if ((num == 4) && (den == 3))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_4x3;
  else if ((num == 3) && (den == 2))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_3x2;
  else if ((num == 2) && (den == 1))
    return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_2x1;

  GST_WARNING ("Invalid pixel aspect ratio %dx%d, default to ratio 1/1",
      num, den);
  return V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_1x1;
}

#define DEFAULT_DCT8x8 FALSE
#define DEFAULT_BMODE V4L2ENC_BMODE_VBR
#define DEFAULT_LEVEL V4L2_MPEG_VIDEO_H264_LEVEL_4_2
#define DEFAULT_PROFILE V4L2_MPEG_VIDEO_H264_PROFILE_BASELINE
#define DEFAULT_PIXEL_ASPECT_RATIO V4L2_MPEG_VIDEO_H264_VUI_SAR_IDC_1x1
#define DEFAULT_GOP_SIZE 15
#define DEFAULT_CPB_SIZE 2400
#define DEFAULT_BITRATE 2000
#define DEFAULT_CABAC 0
#define DEFAULT_QPMIN 18
#define DEFAULT_QPMAX 51

#define V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT	(V4L2_CID_MPEG_BASE+364)

#define MAX_DEVICES 20          /* max V4L2 device instances tried */

#define NB_BUF_INPUT  1
#define NB_BUF_OUTPUT 1

#define WIDTH_MIN 32
#define WIDTH_MAX 1920
#define HEIGHT_MIN 32
#define HEIGHT_MAX 1920

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

/* GstVideoEncoder base class method */
static gboolean gst_v4l2enc_start (GstVideoEncoder * encoder);
static gboolean gst_v4l2enc_stop (GstVideoEncoder * video_encoder);
static gboolean gst_v4l2enc_set_format (GstVideoEncoder * encoder,
    GstVideoCodecState * state);
static GstFlowReturn gst_v4l2enc_handle_frame (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame);
static void gst_v4l2enc_finalize (GObject * object);
static gboolean gst_v4l2enc_propose_allocation (GstVideoEncoder * encoder,
    GstQuery * query);
static void gst_v4l2enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_v4l2enc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static GstFlowReturn gst_v4l2enc_set_static_controls (GstVideoEncoder *
    encoder);
static GstFlowReturn gst_v4l2enc_set_dynamic_controls (GstVideoEncoder *
    encoder);
static GstFlowReturn gst_v4l2enc_set_params (GstVideoEncoder * encoder,
    GstVideoInfo * info);
static int
gst_v4l2enc_open_device (GstV4L2Enc * enc, __u32 sfmt, __u32 pfmt, __u32 width,
    __u32 height);
static gboolean
gst_v4l2enc_setup_input_no_pool (GstVideoEncoder * encoder,
    GstVideoInfo * info);
static GstFlowReturn gst_v4l2enc_encode (GstVideoEncoder * encoder,
    GstVideoCodecFrame * frame);
static GstFlowReturn
gst_v4l2enc_no_pool_process (GstVideoEncoder * encoder, GstBuffer * buf);

void unmap_output_buf (GstV4L2Enc * enc);
gchar *convert_type_2_string (guint32 fmt);
gchar *convert_pxfmt_2_string (guint32 pxfmt);

gboolean plugin_init (GstPlugin * plugin);

struct pixel_format
{
  guint32 pixel_fmt_nb;
  gchar *pixel_fmt_str;
};

struct type_io_v4l2
{
  guint32 type_io_nb;
  gchar *type_io_str;
};

struct type_io_v4l2 type_io[] = {
  {V4L2_BUF_TYPE_VIDEO_OUTPUT, (gchar *) "V4L2_BUF_TYPE_VIDEO_OUTPUT"},
  {V4L2_BUF_TYPE_VIDEO_CAPTURE, (gchar *) "V4L2_BUF_TYPE_VIDEO_CAPTURE"},
};

#define GST_VIDEO_ENC_FORMATS \
    "{ NV12, NV21, UYVY, YUY2, RGB, BGR, RGBx, BGRx, xRGB, xBGR }"

#define GST_VIDEO_ENC_CAPS(features, format) \
    "video/x-raw" features ", "              \
    "format = (string) " format ", "         \
    "width = (int) [ 32, 1920 ], "           \
    "height = (int) [ 32, 1920 ], "          \
    "framerate = (fraction) [0/1, MAX], "    \
    "pixel-aspect-ratio = (fraction) 1/1"

static GstStaticPadTemplate gst_v4l2enc_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_ENC_CAPS
        ("(" GST_CAPS_FEATURE_MEMORY_DMABUF ")", GST_VIDEO_ENC_FORMATS) ";"
        GST_VIDEO_ENC_CAPS ("", GST_VIDEO_ENC_FORMATS))
    );

static GstStaticPadTemplate gst_v4l2enc_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h264, "
        "width  = (int) [ 32, 1920 ], "
        "height = (int) [ 32, 1920 ], "
        "framerate = (fraction) [0/1, MAX], "
        "pixel-aspect-ratio = (fraction) 1/1,"
        "stream-format = (string) { byte-stream }, "
        "alignment = (string) { au }, "
        "profile = (string) { baseline, main, high, stereo},"
        "level = (string) { 4.2, 4.1, 4, 3.2, 3.1, 3, 2.2, 2.1, 2,"
        "1.3, 1.2, 1.1, 1b, 1 }")
    );

#define parent_class gst_v4l2enc_parent_class
G_DEFINE_TYPE (GstV4L2Enc, gst_v4l2enc, GST_TYPE_VIDEO_ENCODER);

struct pixel_format px_formats[] = {
  {V4L2_PIX_FMT_NV12, (gchar *) "V4L2_PIX_FMT_NV12"},
  {V4L2_PIX_FMT_NV21, (gchar *) "V4L2_PIX_FMT_NV21"},
  {V4L2_PIX_FMT_UYVY, (gchar *) "V4L2_PIX_FMT_UYVY"},
  {V4L2_PIX_FMT_VYUY, (gchar *) "V4L2_PIX_FMT_VYUY"},
  {V4L2_PIX_FMT_XRGB32, (gchar *) "V4L2_PIX_FMT_XRGB32"},
  {V4L2_PIX_FMT_XBGR32, (gchar *) "V4L2_PIX_FMT_XBGR32"},
  {V4L2_PIX_FMT_RGB32, (gchar *) "V4L2_PIX_FMT_RGB32"},
  {V4L2_PIX_FMT_BGR32, (gchar *) "V4L2_PIX_FMT_BGR32"},
  {V4L2_PIX_FMT_RGB24, (gchar *) "V4L2_PIX_FMT_RGB24"},
  {V4L2_PIX_FMT_BGR24, (gchar *) "V4L2_PIX_FMT_BGR24"},
  {V4L2_PIX_FMT_H264, (gchar *) "V4L2_PIX_FMT_H264"},
};

/* used for debug pixelformat (v4l2 object) */
gchar *
v4l2_fmt_str (guint32 fmt)
{
  int i = 0;
  for (i = 0; i < ARRAY_SIZE (px_formats); i++) {
    if (px_formats[i].pixel_fmt_nb == fmt)
      return px_formats[i].pixel_fmt_str;
  }
  return NULL;
}

gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "v4l2enc", GST_RANK_PRIMARY + 1,
          GST_TYPE_V4L2ENC))
    return FALSE;
  return TRUE;
}

static void
gst_v4l2enc_class_init (GstV4L2EncClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoEncoderClass *video_encoder_class = GST_VIDEO_ENCODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = gst_v4l2enc_finalize;

  gobject_class->set_property = gst_v4l2enc_set_property;
  gobject_class->get_property = gst_v4l2enc_get_property;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_v4l2enc_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_v4l2enc_sink_template));

  video_encoder_class->start = GST_DEBUG_FUNCPTR (gst_v4l2enc_start);
  video_encoder_class->stop = GST_DEBUG_FUNCPTR (gst_v4l2enc_stop);
  video_encoder_class->set_format = GST_DEBUG_FUNCPTR (gst_v4l2enc_set_format);
  video_encoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_v4l2enc_handle_frame);
  video_encoder_class->propose_allocation =
      GST_DEBUG_FUNCPTR (gst_v4l2enc_propose_allocation);

  g_object_class_install_property (gobject_class, PROP_BITRATE,
      g_param_spec_uint ("bitrate", "bitrate",
          "bitrate in kbps (0 = unlimited)", 0, G_MAXUINT / 1024,
          DEFAULT_BITRATE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DCT8x8,
      g_param_spec_boolean ("dct8x8", "DCT8x8",
          "Adaptive spatial transform size", DEFAULT_DCT8x8,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CABAC,
      g_param_spec_boolean ("cabac", "Use CABAC", "Enable CABAC entropy coding",
          DEFAULT_CABAC, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QPMIN,
      g_param_spec_uint ("qp-min", "Minimum Quantizer",
          "Minimum quantizer", 0, 51, DEFAULT_QPMIN,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_QPMAX,
      g_param_spec_uint ("qp-max", "Maximum Quantizer",
          "Maximum quantizer", 0, 51, DEFAULT_QPMAX,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_GOP_SIZE,
      g_param_spec_uint ("gop-size", "gop size", "gop Size", 1, G_MAXUINT,
          DEFAULT_GOP_SIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_CPB_SIZE,
      g_param_spec_uint ("cpb-size", "cpb size",
          "Cpb size in kb (0 = unlimited)", 0, G_MAXUINT / 1024,
          DEFAULT_CPB_SIZE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_BITRATE_MODE,
      g_param_spec_uint ("bitrate-mode", "bitrate mode",
          "Constant (1) or variable (0) bitrate mode",
          V4L2ENC_BMODE_VBR, V4L2ENC_BMODE_CBR,
          DEFAULT_BMODE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  GST_DEBUG_CATEGORY_INIT (gst_v4l2enc_debug, "v4l2enc", 0,
      "ST v4l2 video encoder");

  gst_element_class_set_static_metadata (element_class,
      "V4L2 encoder", "Encoder/Video", "A V4L2 encoder", "STMicroelectronics");
}

/* Init the v4l2enc structure */
static void
gst_v4l2enc_init (GstV4L2Enc * enc)
{
  enc->fd = -1;

  enc->current_nb_buf_output = 0;
  enc->mmap_virtual_output = NULL;
  enc->mmap_size_output = NULL;
  enc->out_no_pool_setup = FALSE;

  /* Static controls */
  enc->controls.level = DEFAULT_LEVEL;
  enc->controls.profile = DEFAULT_PROFILE;
  enc->controls.pixel_aspect_ratio = DEFAULT_PIXEL_ASPECT_RATIO;
  enc->controls.bitrate_mode = DEFAULT_BMODE;
  enc->controls.dct8x8 = DEFAULT_DCT8x8;
  enc->controls.cabac = DEFAULT_CABAC;

  /* Dynamic controls */
  enc->controls.gop_size = DEFAULT_GOP_SIZE;
  enc->controls.pending_flags |= V4L2ENC_FLAG_GOP_SIZE;
  enc->controls.cpb_size = DEFAULT_CPB_SIZE;
  enc->controls.pending_flags |= V4L2ENC_FLAG_CPB_SIZE;
  enc->controls.bitrate = DEFAULT_BITRATE;
  enc->controls.pending_flags |= V4L2ENC_FLAG_BITRATE;
  enc->controls.qpmin = DEFAULT_QPMIN;
  enc->controls.pending_flags |= V4L2ENC_FLAG_MIN_QP;
  enc->controls.qpmax = DEFAULT_QPMAX;
  enc->controls.pending_flags |= V4L2ENC_FLAG_MAX_QP;
}

static void
gst_v4l2enc_finalize (GObject * object)
{
  GstV4L2Enc *enc = GST_V4L2ENC (object);

  free (enc->device_name);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* Open the device */
static gboolean
gst_v4l2enc_start (GstVideoEncoder * encoder)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);

  GST_DEBUG_OBJECT (enc, "Starting");

  return TRUE;
}

static gboolean
gst_v4l2enc_stop (GstVideoEncoder * encoder)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);
  gint type;
  GST_DEBUG_OBJECT (enc, "Stopping");

  if (enc->out_no_pool_setup) {
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (v4l2_ioctl (enc->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (enc, "Unable to stop stream on output err=%s",
          strerror (errno));
    GST_DEBUG_OBJECT (enc, "STREAM OFF OUTPUT");
  }

  if (enc->fd != -1) {
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl (enc->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (enc, "Unable to stop stream on capture err=%s",
          strerror (errno));
    GST_DEBUG_OBJECT (enc, "STREAM OFF CAPTURE");
  }

  if (enc->input_state) {
    gst_video_codec_state_unref (enc->input_state);
    enc->input_state = NULL;
  }

  if (enc->output_state) {
    gst_video_codec_state_unref (enc->output_state);
    enc->output_state = NULL;
  }

  unmap_output_buf (enc);

  if (enc->mmap_virtual_output) {
    free (enc->mmap_virtual_output);
    enc->mmap_virtual_output = NULL;

    free (enc->mmap_size_output);
    enc->mmap_size_output = NULL;

    enc->current_nb_buf_output = 0;
  }

  if (enc->fd != -1) {
    v4l2_close (enc->fd);
    enc->fd = -1;
  }

  GST_DEBUG_OBJECT (enc, "Stopped !!");

  return TRUE;
}

/* Open the device matching width/height/format as input */
static int
gst_v4l2enc_open_device (GstV4L2Enc * enc, __u32 sfmt, __u32 pfmt, __u32 width,
    __u32 height)
{
  int fd = -1;
  int ret;
  gint i = 0;
  gboolean found;
  gchar path[100];
  struct v4l2_format try_fmt;
  struct v4l2_format s_fmt;
  int libv4l2_fd;

  found = FALSE;
  for (i = 0; i < MAX_DEVICES; i++) {
    snprintf (path, sizeof (path), "/dev/video%d", i);

    fd = open (path, O_RDWR, 0);
    if (fd < 0)
      continue;

    libv4l2_fd = v4l2_fd_open (fd, V4L2_DISABLE_CONVERSION);
    if (libv4l2_fd != -1)
      fd = libv4l2_fd;

    memset (&try_fmt, 0, sizeof try_fmt);
    try_fmt.fmt.pix.width = width;
    try_fmt.fmt.pix.height = height;
    try_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    try_fmt.fmt.pix.sizeimage = (width * height * 3 / 2);
    try_fmt.fmt.pix.pixelformat = pfmt;

    ret = v4l2_ioctl (fd, VIDIOC_TRY_FMT, &try_fmt);
    if (ret < 0) {
      v4l2_close (fd);
      continue;
    }

    memset (&try_fmt, 0, sizeof try_fmt);
    try_fmt.fmt.pix.width = width;
    try_fmt.fmt.pix.height = height;
    try_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    try_fmt.fmt.pix.sizeimage = ((width * height * 3 / 2) / 2);
    try_fmt.fmt.pix.pixelformat = sfmt;

    ret = v4l2_ioctl (fd, VIDIOC_TRY_FMT, &try_fmt);
    if (ret < 0) {
      v4l2_close (fd);
      continue;
    }

    s_fmt = try_fmt;

    ret = v4l2_ioctl (fd, VIDIOC_S_FMT, &s_fmt);
    if (ret < 0) {
      v4l2_close (fd);
      continue;
    }

    found = TRUE;
    break;
  }

  if (!found) {
    GST_ERROR_OBJECT (enc, "No device found matching stream format %s(0x%x)"
        ", pixel format %s(0x%x) and resolution %dx%d",
        v4l2_fmt_str (sfmt), sfmt, v4l2_fmt_str (pfmt), pfmt, width, height);
    return -1;
  }

  GST_INFO_OBJECT (enc, "Device %s opened for format %s and %dx%d resolution",
      path, v4l2_fmt_str (sfmt), width, height);
  return fd;
}

static int
gst_v4l2enc_get_controls_from_caps (GstVideoEncoder * encoder,
    GstVideoInfo * info)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);

  GstCaps *allowed_caps = NULL;

  allowed_caps = gst_pad_get_allowed_caps (GST_VIDEO_ENCODER_SRC_PAD (encoder));

  if (allowed_caps) {
    GstStructure *s;

    if (gst_caps_is_empty (allowed_caps)) {
      gst_caps_unref (allowed_caps);
      return FALSE;
    }

    allowed_caps = gst_caps_make_writable (allowed_caps);
    allowed_caps = gst_caps_fixate (allowed_caps);
    s = gst_caps_get_structure (allowed_caps, 0);

    GST_INFO_OBJECT (enc, "upstream caps: %" GST_PTR_FORMAT, allowed_caps);

    enc->controls.profile = to_v4l2_profile (s);
    enc->controls.level = to_v4l2_level (s);
    enc->controls.pixel_aspect_ratio = to_v4l2_pixel_aspect_ratio (info);
    GST_INFO_OBJECT (enc, "pixel aspect ratio: %d",
        enc->controls.pixel_aspect_ratio);

    if (gst_structure_has_field (s, "bitrate")) {
      gint bitrate;
      gst_structure_get_int (s, "bitrate", &bitrate);
      GST_INFO_OBJECT (enc, "bitrate: %d", bitrate);
      enc->controls.bitrate = (guint) bitrate;
      enc->controls.pending_flags |= V4L2ENC_FLAG_BITRATE;
    }
    gst_caps_unref (allowed_caps);
  }
  return TRUE;
}

static gboolean
gst_v4l2enc_setup_input_no_pool (GstVideoEncoder * encoder, GstVideoInfo * info)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);
  gint type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  struct v4l2_format s_fmt;
  struct v4l2_requestbuffers reqbuf;
  gint i, ret;
  guint aligned_width;
  guint aligned_height;

  aligned_width = GST_VIDEO_INFO_WIDTH (info);
  aligned_height = GST_VIDEO_INFO_HEIGHT (info);

  /* configure V4L2 input */
  memset (&s_fmt, 0, sizeof s_fmt);
  s_fmt.fmt.pix.width = GST_VIDEO_INFO_WIDTH (info);
  s_fmt.fmt.pix.height = GST_VIDEO_INFO_HEIGHT (info);
  s_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

  switch (GST_VIDEO_INFO_FORMAT (info)) {
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

  GST_DEBUG_OBJECT (enc, "set input format %dx%d resolution", aligned_width,
      aligned_height);

  ret = v4l2_ioctl (enc->fd, VIDIOC_S_FMT, &s_fmt);
  if (ret != 0)
    goto error_s_fmt;

  /* Request output buffers  */
  memset (&reqbuf, 0, sizeof reqbuf);
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbuf.count = NB_BUF_INPUT_POOL;
  reqbuf.memory = V4L2_MEMORY_DMABUF;

  if (v4l2_ioctl (enc->fd, VIDIOC_REQBUFS, &reqbuf) < 0)
    goto error_ioc_reqbufs;

  /* Clear V4L2 output fd table */
  for (i = 0; i < ARRAY_SIZE (enc->v4l2_output_fds); i++)
    enc->v4l2_output_fds[i] = -1;

  /* Start stream on OUTPUT */
  if (v4l2_ioctl (enc->fd, VIDIOC_STREAMON, &type) < 0)
    goto error_ioc_streamon;

  enc->out_no_pool_setup = TRUE;

  return TRUE;

  /* ERRORS */
error_s_fmt:
  {
    GST_ERROR_OBJECT (enc, "Unable to set input format err=%s",
        strerror (errno));
    return FALSE;
  }
wrong_config:
  {
    GST_ERROR_OBJECT (enc, "Error: invalid video info");
    return FALSE;
  }
error_ioc_reqbufs:
  {
    GST_ERROR_OBJECT (enc, "Unable to request input buffers err=%s",
        strerror (errno));
    return FALSE;
  }
error_ioc_streamon:
  {
    GST_ERROR_OBJECT (enc, "Unable to start input stream err=%s",
        strerror (errno));
    return FALSE;
  }
}

static gboolean
gst_v4l2enc_set_format (GstVideoEncoder * encoder, GstVideoCodecState * state)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);
  gint i;
  gint type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  int fd;
  __u32 pixelformat = V4L2_PIX_FMT_NV12;
  __u32 streamformat = V4L2_PIX_FMT_H264;
  struct v4l2_requestbuffers reqbuf;
  struct v4l2_buffer querybuf;
  struct v4l2_buffer qbuf;
  GstCaps *caps;
  GstVideoInfo *info = &state->info;
  gint width, height;

  GST_DEBUG_OBJECT (enc, "Setting format: %" GST_PTR_FORMAT, state->caps);

  if (enc->input_state)
    gst_video_codec_state_unref (enc->input_state);
  enc->input_state = gst_video_codec_state_ref (state);

  /* Find profile & level from caps */
  if (!gst_v4l2enc_get_controls_from_caps (encoder, info))
    goto error_controls;

  if (streamformat == V4L2_PIX_FMT_H264)
    caps = gst_caps_new_empty_simple ("video/x-h264");

  caps = gst_caps_make_writable (caps);
  gst_caps_set_simple (caps, "stream-format", G_TYPE_STRING, "byte-stream",
      "alignment", G_TYPE_STRING, "au", NULL);

  enc->output_state =
      gst_video_encoder_set_output_state (GST_VIDEO_ENCODER (enc), caps, state);
  gst_video_codec_state_unref (enc->output_state);
  enc->output_state = NULL;

  width = GST_VIDEO_INFO_WIDTH (info);
  height = GST_VIDEO_INFO_HEIGHT (info);

  if (enc->fd == -1) {
    fd = gst_v4l2enc_open_device (enc, streamformat, pixelformat, width,
        height);
    if (fd == -1)
      goto error_device;

    enc->fd = fd;
    enc->width = width;
    enc->height = height;
  }

  gst_video_encoder_negotiate (GST_VIDEO_ENCODER (enc));

  /* Memory mapping for CAPTURE buffers in V4L2 */
  memset (&reqbuf, 0, sizeof reqbuf);
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.count = NB_BUF_OUTPUT;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl (enc->fd, VIDIOC_REQBUFS, &reqbuf) < 0)
    goto error_ioc_reqbufs;

  enc->mmap_virtual_output = malloc (sizeof (void *) * reqbuf.count);
  enc->mmap_size_output = malloc (sizeof (void *) * reqbuf.count);

  /* Set static controls */
  if (gst_v4l2enc_set_static_controls (encoder) != GST_FLOW_OK)
    goto error_set_ext_control;

  /* Set dynamic controls */
  if (gst_v4l2enc_set_dynamic_controls (encoder) != GST_FLOW_OK)
    goto error_set_ext_control;

  /* set streaming parameters */
  gst_v4l2enc_set_params (encoder, info);

  memset (&querybuf, 0, sizeof querybuf);
  querybuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  querybuf.memory = V4L2_MEMORY_MMAP;
  for (i = 0; i < reqbuf.count; i++) {
    void *ptr;
    querybuf.index = i;
    enc->current_nb_buf_output++;

    /* Memory mapping for output buffers in GStreamer */
    if (v4l2_ioctl (enc->fd, VIDIOC_QUERYBUF, &querybuf) < 0)
      goto error_ioc_querybuf;
    ptr = v4l2_mmap (NULL, querybuf.length, PROT_READ | PROT_WRITE,
        MAP_SHARED, enc->fd, querybuf.m.offset);

    if (ptr == MAP_FAILED)
      goto error_map_fail;

    enc->mmap_virtual_output[i] = ptr;
    enc->mmap_size_output[i] = querybuf.length;

    qbuf = querybuf;            /* index from querybuf */
    qbuf.bytesused = 0;         /* enqueue it with no data */
    if (v4l2_ioctl (enc->fd, VIDIOC_QBUF, &qbuf) < 0)
      goto error_ioc_qbuf;
  }

  /* Start stream on CAPTURE */
  if (v4l2_ioctl (enc->fd, VIDIOC_STREAMON, &type) < 0)
    goto error_ioc_streamon;

  /* activate the OUTPUT pool: the buffers are allocated */
  if (enc->pool && gst_buffer_pool_set_active (enc->pool, TRUE) == FALSE)
    goto error_activate_pool;

  return TRUE;

  /* Errors */
error_device:
  {
    GST_DEBUG_OBJECT (enc, "cannot open device");
    return FALSE;
  }
error_controls:
  {
    GST_ERROR_OBJECT (enc, "Error getting controls from caps");
    return FALSE;
  }
error_ioc_reqbufs:
  {
    GST_ERROR_OBJECT (enc, "Unable to request input buffers err=%s",
        strerror (errno));
    return FALSE;
  }
error_ioc_querybuf:
  {
    GST_ERROR_OBJECT (enc, "Query of input buffer failed err=%s",
        strerror (errno));
    return FALSE;
  }
error_map_fail:
  {
    GST_ERROR_OBJECT (enc, "Failed to map input buffer");
    return FALSE;
  }
error_ioc_qbuf:
  {
    GST_ERROR_OBJECT (enc, "Enqueuing buffer failed err=%s", strerror (errno));
    return FALSE;
  }
error_ioc_streamon:
  {
    GST_ERROR_OBJECT (enc, "Unable to start input stream err=%s",
        strerror (errno));
    return FALSE;
  }
error_set_ext_control:
  {
    GST_ERROR_OBJECT (enc, "Unable to set external control(s) err=%s",
        strerror (errno));
    return FALSE;
  }
error_activate_pool:
  {
    GST_ERROR_OBJECT (enc, "Unable to activate the pool");
    return FALSE;
  }
}

void
unmap_output_buf (GstV4L2Enc * enc)
{
  if (enc->mmap_virtual_output)
    for (int i = 0; i < enc->current_nb_buf_output; i++)
      v4l2_munmap (enc->mmap_virtual_output[i], enc->mmap_size_output[i]);
}

/* used for debug I/O type (v4l2 object) */
gchar *
convert_type_2_string (guint32 fmt)
{
  int i = 0;
  for (i = 0; i < ARRAY_SIZE (type_io); i++) {
    if (type_io[i].type_io_nb == fmt)
      return type_io[i].type_io_str;
  }
  return NULL;
}

/* used for debug pixelformat (v4l2 object) */
gchar *
convert_pxfmt_2_string (guint32 pxfmt)
{
  int i = 0;
  for (i = 0; i < ARRAY_SIZE (px_formats); i++) {
    if (px_formats[i].pixel_fmt_nb == pxfmt)
      return px_formats[i].pixel_fmt_str;
  }
  return NULL;
}

static GstFlowReturn
gst_v4l2enc_handle_frame (GstVideoEncoder * encoder, GstVideoCodecFrame * frame)
{
  GstFlowReturn ret = GST_FLOW_OK;
  gsize gstsize;
  gsize gstoffset = 0;
  struct v4l2_buffer dqbuf;
  struct v4l2_buffer qbuf;

  GstV4L2Enc *enc = GST_V4L2ENC (encoder);

  GST_DEBUG_OBJECT (enc, "gst_v4l2enc_handle_frame");

  /* Set dynamic controls */
  if (gst_v4l2enc_set_dynamic_controls (encoder) != GST_FLOW_OK)
    GST_WARNING_OBJECT (enc, "Unable to set dynamic controls");

  /* For every frame, encode */
  ret = gst_v4l2enc_encode (encoder, frame);

  if (ret)
    goto error_process;

  /* Wait for encoded access unit (will block till available) */
  memset (&dqbuf, 0, sizeof dqbuf);
  dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  dqbuf.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl (enc->fd, VIDIOC_DQBUF, &dqbuf) < 0) {
    goto error_ioctl_dequeue_out;
  }
  gstsize = dqbuf.bytesused;

  ret = gst_video_encoder_allocate_output_frame (encoder, frame, gstsize);
  if (ret)
    goto error_alloc_frame;

  if (dqbuf.flags & V4L2_BUF_FLAG_KEYFRAME) {
    /* mark gst frame as key */
    GST_VIDEO_CODEC_FRAME_SET_SYNC_POINT (frame);
  }

  gst_buffer_fill (frame->output_buffer, gstoffset,
      enc->mmap_virtual_output[dqbuf.index], gstsize);

  ret = gst_video_encoder_finish_frame (encoder, frame);
  if (ret && ret != GST_FLOW_FLUSHING)
    goto error_finish_frame;

  GST_DEBUG_OBJECT (enc, "-->Access unit pushed");

  /* FIXME: refine error section regarding cleanup: for ex. always recycle
   * access unit in case of allocate/finish error... */
  /* recycle access unit */
  qbuf = dqbuf;
  if (v4l2_ioctl (enc->fd, VIDIOC_QBUF, &qbuf) < 0)
    goto error_ioctl_enqueue_out;

  return GST_FLOW_OK;

  /* ERRORS */
error_process:
  {
    GST_ERROR_OBJECT (enc, "queuing input frame in pool failed");
    return ret;
  }
error_ioctl_dequeue_out:
  {
    GST_ERROR_OBJECT (enc, "Dequeuing output failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
error_ioctl_enqueue_out:
  {
    GST_ERROR_OBJECT (enc, "Enqueuing output failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
error_alloc_frame:
  {
    GST_ERROR_OBJECT (enc, "Error when allocating frame err=%s",
        gst_flow_get_name (ret));
    return ret;
  }
error_finish_frame:
  {
    GST_ERROR_OBJECT (enc, "Error when finishing frame err=%s",
        gst_flow_get_name (ret));
    return ret;
  }
}

/* Return the v4l2 index corresponding to a fd.
 * First search if a fd has a corresponding v4l2 index
 * If not, add an element to the table, possibly overwriting an existing
 * element if table full
 */
static gint
gst_v4l2enc_get_v4l2_output_index (GstV4L2Enc * enc, gint fd)
{
  gint i, free = -1;

  for (i = 0; i < ARRAY_SIZE (enc->v4l2_output_fds); i++) {
    if (enc->v4l2_output_fds[i] == fd)
      return i;

    if ((free < 0) && (enc->v4l2_output_fds[i] == -1))
      free = i;
  }

  /* Not found, add to the table */
  if (free < 0) {
    GST_WARNING_OBJECT (enc,
        "v4l2_output_fds table full: will overwrite for fd=%d", fd);
    free = 0;
  }

  enc->v4l2_output_fds[free] = fd;
  return free;
}

static GstFlowReturn
gst_v4l2enc_encode (GstVideoEncoder * encoder, GstVideoCodecFrame * frame)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstBuffer *buf = NULL;
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);

  GST_DEBUG_OBJECT (enc, "encoder input frame");
  buf = frame->input_buffer;

  if (!buf->pool)
    goto error_no_pool;

  if (buf->pool == enc->pool) {
    /* Allocated with our own pool, process it... */
    ret = gst_v4l2enc_buffer_pool_process (enc->pool, frame->input_buffer);
    if (ret != GST_FLOW_OK)
      goto error_encode_process;
  } else {
    /* Not our own pool, prcoess input dmabuf */
    ret = gst_v4l2enc_no_pool_process (encoder, frame->input_buffer);
    if (ret != GST_FLOW_OK)
      goto error_encode_process;
  }

  return GST_FLOW_OK;

  /* ERRORS */
error_no_pool:
  {
    GST_ERROR_OBJECT (enc, "encode buffer %p not from a pool ", buf);
    return ret;
  }
error_encode_process:
  {
    GST_ERROR_OBJECT (enc, "could not process encode buffer %p", buf);
    return ret;
  }
}

static GstFlowReturn
gst_v4l2enc_no_pool_process (GstVideoEncoder * encoder, GstBuffer * buf)
{
  GstMapInfo mapinfo = { 0, };
  gint fd;
  GstMemory *gmem;
  gsize gstsize;
  struct v4l2_buffer qbuf;
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);

  /* Not using our buffer pool. Will use dmabuf fd for 0-copy */
  if (!enc->out_no_pool_setup) {
    GstVideoInfo *info = &enc->input_state->info;
    if (!gst_v4l2enc_setup_input_no_pool (encoder, info))
      goto error_setup_input;
  }

  /* input: get buffer properties */
  gmem = gst_buffer_get_memory (buf, 0);

  if (!gst_is_dmabuf_memory (gmem))
    goto error_no_dmabuf;

  /* DMABUF: use fd */
  fd = gst_dmabuf_memory_get_fd (gmem);
  gst_memory_unref (gmem);

  /* input: queue buffer */
  gst_buffer_map (buf, &mapinfo, GST_MAP_READ);
  gstsize = mapinfo.size;
  memset (&qbuf, 0, sizeof qbuf);
  qbuf.memory = V4L2_MEMORY_DMABUF;
  qbuf.m.fd = fd;
  qbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  qbuf.bytesused = gstsize;
  qbuf.length = qbuf.bytesused;
  /* Frame timestamp */
  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (buf)))
    GST_TIME_TO_TIMEVAL (GST_BUFFER_TIMESTAMP (buf), qbuf.timestamp);

  gst_buffer_unmap (buf, &mapinfo);

  /* We have to keep m.fd <-> index consistency to get V4L2 DMABUF working */
  qbuf.index = gst_v4l2enc_get_v4l2_output_index (enc, fd);

  /* enqueue the input buffer for processing */
  if (v4l2_ioctl (enc->fd, VIDIOC_QBUF, &qbuf) < 0)
    goto error_ioc_qbuf;

  GST_DEBUG_OBJECT (enc, "Enqueued input buffer %p fd= %d index=%d", buf,
      qbuf.m.fd, qbuf.index);

  /* dequeue the processed input buffer */
  if (v4l2_ioctl (enc->fd, VIDIOC_DQBUF, &qbuf) < 0) {
    goto error_ioc_dqbuf;
  }
  GST_DEBUG_OBJECT (enc, "Dequeued input buffer %p fd=%d index=%d", buf,
      qbuf.m.fd, qbuf.index);

  return GST_FLOW_OK;

  /* ERRORS */
error_setup_input:
  {
    GST_ERROR_OBJECT (enc, "Unable to setup input");
    return GST_FLOW_ERROR;
  }
error_no_dmabuf:
  {
    GST_ERROR_OBJECT (enc, "encode buffer %p not a DMA buf ", buf);
    gst_memory_unref (gmem);
    return GST_FLOW_ERROR;
  }
error_ioc_qbuf:
  {
    GST_ERROR_OBJECT (enc, "Enqueuing buffer failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
error_ioc_dqbuf:
  {
    GST_ERROR_OBJECT (enc, "Dequeuing buffer failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
}

static gboolean
gst_v4l2enc_propose_allocation (GstVideoEncoder * encoder, GstQuery * query)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);
  GstBufferPool *pool;
  GstStructure *config;
  GstCaps *caps;
  guint size;
  gboolean need_pool;
  GstAllocator *allocator;
  GstAllocationParams params;
  int fd;
  __u32 streamformat = V4L2_PIX_FMT_H264;
  __u32 pixelformat = V4L2_PIX_FMT_NV12;
  gint width, height;

  gst_allocation_params_init (&params);
  gst_query_parse_allocation (query, &caps, &need_pool);

  if (caps == NULL)
    goto no_caps;

  if ((pool = enc->pool))
    gst_object_ref (pool);

  if (pool != NULL) {
    GstCaps *pcaps;

    /* we had a pool, check caps */
    config = gst_buffer_pool_get_config (pool);
    gst_buffer_pool_config_get_params (config, &pcaps, &size, NULL, NULL);

    if (!gst_caps_is_equal (caps, pcaps)) {
      /* different caps, we can't use this pool */

      /* FIXME renegotiating... stop all */
      gst_v4l2enc_stop (encoder);

      gst_object_unref (pool);
      GST_DEBUG_OBJECT (enc, "pool has different caps");
      pool = NULL;
    }
    gst_structure_free (config);
  }

  if (pool == NULL && need_pool) {
    GstVideoInfo info;

    if (!gst_video_info_from_caps (&info, caps))
      goto invalid_caps;

    width = GST_VIDEO_INFO_WIDTH (&info);
    height = GST_VIDEO_INFO_HEIGHT (&info);

    if (enc->fd == -1) {
      fd = gst_v4l2enc_open_device (enc, streamformat, pixelformat, width,
          height);
      if (fd == -1)
        goto error_device;

      enc->fd = fd;
      enc->width = width;
      enc->height = height;
    }

    GST_DEBUG_OBJECT (enc, "create src buffer pool (input frames)");
    pool = gst_v4l2enc_buffer_pool_new (enc);
    if (pool == NULL)
      goto error_new_pool;
    enc->pool = pool;

    /* The normal size of a frame */
    size = info.size;

    /* Finally add this pool to query,
     * so that it can be selected.
     * We need at least 2 buffers to ensure
     * double-buffering at input of encoder
     * (encode current frame while next is being
     * treated by downstream element) */
    gst_query_add_allocation_pool (query, pool, size, 2, 0);

    gst_query_add_allocation_meta (query, GST_VIDEO_META_API_TYPE, 0);

    /* FIXME to understand, this is needed otherwise no allocator
     * detected on upstream side! */
    gst_object_unref (pool);

    /* DMA-BUF allocator */
    allocator = gst_dmabuf_allocator_new ();
    gst_query_add_allocation_param (query, allocator, &params);
    gst_object_unref (allocator);
  }

  return TRUE;

  /* ERRORS */
no_caps:
  {
    GST_DEBUG_OBJECT (enc, "no caps specified");
    return FALSE;
  }
invalid_caps:
  {
    GST_DEBUG_OBJECT (enc, "invalid caps specified");
    return FALSE;
  }
error_device:
  {
    GST_DEBUG_OBJECT (enc, "cannot open device");
    return FALSE;
  }
error_new_pool:
  {
    GST_ERROR_OBJECT (enc, "Unable to construct a new buffer pool");
    return GST_FLOW_ERROR;
  }
}

static void
gst_v4l2enc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstV4L2Enc *enc = GST_V4L2ENC (object);

  GST_OBJECT_LOCK (enc);
  switch (prop_id) {
    case PROP_DCT8x8:
      enc->controls.dct8x8 = g_value_get_boolean (value);
      break;
    case PROP_BITRATE_MODE:
      enc->controls.bitrate_mode = g_value_get_uint (value);
      break;
    case PROP_CABAC:
      enc->controls.cabac = g_value_get_boolean (value);
      break;
    case PROP_GOP_SIZE:
      enc->controls.gop_size = g_value_get_uint (value);
      enc->controls.pending_flags |= V4L2ENC_FLAG_GOP_SIZE;
      break;
    case PROP_CPB_SIZE:
      enc->controls.cpb_size = g_value_get_uint (value);
      enc->controls.pending_flags |= V4L2ENC_FLAG_CPB_SIZE;
      break;
    case PROP_BITRATE:
      enc->controls.bitrate = g_value_get_uint (value);
      enc->controls.pending_flags |= V4L2ENC_FLAG_BITRATE;
      break;
    case PROP_QPMIN:
      enc->controls.qpmin = g_value_get_uint (value);
      enc->controls.pending_flags |= V4L2ENC_FLAG_MIN_QP;
      break;
    case PROP_QPMAX:
      enc->controls.qpmax = g_value_get_uint (value);
      enc->controls.pending_flags |= V4L2ENC_FLAG_MAX_QP;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (enc);
}

static void
gst_v4l2enc_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstV4L2Enc *enc = GST_V4L2ENC (object);

  switch (prop_id) {
    case PROP_GOP_SIZE:
      g_value_set_uint (value, enc->controls.gop_size);
      break;
    case PROP_CPB_SIZE:
      g_value_set_uint (value, enc->controls.cpb_size);
      break;
    case PROP_BITRATE:
      g_value_set_uint (value, enc->controls.bitrate);
      break;
    case PROP_DCT8x8:
      g_value_set_boolean (value, enc->controls.dct8x8);
      break;
    case PROP_BITRATE_MODE:
      g_value_set_uint (value, enc->controls.bitrate_mode);
      break;
    case PROP_CABAC:
      g_value_set_boolean (value, enc->controls.cabac);
      break;
    case PROP_QPMIN:
      g_value_set_uint (value, enc->controls.qpmin);
      break;
    case PROP_QPMAX:
      g_value_set_uint (value, enc->controls.qpmax);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstFlowReturn
gst_v4l2enc_set_static_controls (GstVideoEncoder * encoder)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);
  struct v4l2_ext_controls ctrls;
  struct v4l2_ext_control ctrl[32];
  gint count = 0;
  gint ret = 0;

  memset (&ctrls, 0, sizeof (struct v4l2_ext_controls));
  ctrls.controls = ctrl;
  ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;

  ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
  ctrl[count].value = enc->controls.level;
  count++;

  ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
  ctrl[count].value = enc->controls.profile;
  count++;

  ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE;
  ctrl[count].value = TRUE;
  count++;

  ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC;
  ctrl[count].value = enc->controls.pixel_aspect_ratio;
  count++;

  if (enc->controls.profile == V4L2_MPEG_VIDEO_H264_PROFILE_STEREO_HIGH ||
      enc->controls.profile == V4L2_MPEG_VIDEO_H264_PROFILE_MULTIVIEW_HIGH) {
    /* enable sei frame packing */
    ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_SEI_FRAME_PACKING;
    ctrl[count].value = TRUE;
    count++;

    /* set frame packing arrangement (support only Top/Bottom) */
    ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_SEI_FP_ARRANGEMENT_TYPE;
    ctrl[count].value = V4L2_MPEG_VIDEO_H264_SEI_FP_ARRANGEMENT_TYPE_TOP_BOTTOM;
    count++;
  }

  ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM;
  ctrl[count].size = sizeof (enc->controls.dct8x8);
  if (enc->controls.profile < V4L2_MPEG_VIDEO_H264_PROFILE_HIGH) {
    if (enc->controls.dct8x8)
      GST_WARNING_OBJECT (enc,
          "DCT8x8 disable, profile must be at least set to high profile");
    ctrl[count].value = 0;
  } else {
    ctrl[count].value = enc->controls.dct8x8;
  }
  count++;

  ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE;
  ctrl[count].size = sizeof (enc->controls.cabac);
  if (enc->controls.profile < V4L2_MPEG_VIDEO_H264_PROFILE_MAIN) {
    if (enc->controls.cabac)
      GST_WARNING_OBJECT (enc,
          "Cabac disable, profile must be at least set to main profile");
    ctrl[count].value = 0;
  } else {
    ctrl[count].value = enc->controls.cabac;
  }
  count++;

  ctrl[count].id = V4L2_CID_MPEG_VIDEO_BITRATE_MODE;
  ctrl[count].size = sizeof (enc->controls.bitrate_mode);
  ctrl[count].value = enc->controls.bitrate_mode;
  count++;

  ctrls.count = count;

  ret = v4l2_ioctl (enc->fd, VIDIOC_S_EXT_CTRLS, &ctrls);
  if (ret < 0) {
    GST_ERROR_OBJECT (enc, "Unable to set static controls: %s (%d)",
        strerror (errno), errno);
    return GST_FLOW_ERROR;
  }

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_v4l2enc_set_dynamic_controls (GstVideoEncoder * encoder)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);
  struct v4l2_ext_controls ctrls;
  struct v4l2_ext_control ctrl[32];
  gint count = 0;
  gint ret = 0;

  GST_OBJECT_LOCK (enc);

  /* return if nothing to set */
  if (!enc->controls.pending_flags) {
    GST_OBJECT_UNLOCK (enc);
    return GST_FLOW_OK;
  }

  memset (&ctrls, 0, sizeof (struct v4l2_ext_controls));
  ctrls.controls = ctrl;
  ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;

  if (enc->controls.pending_flags & V4L2ENC_FLAG_GOP_SIZE) {
    ctrl[count].id = V4L2_CID_MPEG_VIDEO_GOP_SIZE;
    ctrl[count].size = sizeof (enc->controls.gop_size);
    ctrl[count].value = enc->controls.gop_size;
    count++;
  }

  if (enc->controls.pending_flags & V4L2ENC_FLAG_CPB_SIZE) {
    ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE;
    ctrl[count].size = sizeof (enc->controls.cpb_size);
    /* convert from kb in kB */
    ctrl[count].value = enc->controls.cpb_size / 8;
    count++;
  }

  if (enc->controls.pending_flags & V4L2ENC_FLAG_BITRATE) {
    ctrl[count].id = V4L2_CID_MPEG_VIDEO_BITRATE;
    ctrl[count].size = sizeof (enc->controls.bitrate);
    /* convert from kbps in bps */
    ctrl[count].value = enc->controls.bitrate * 1000;
    count++;
  }

  if (enc->controls.pending_flags & V4L2ENC_FLAG_MIN_QP) {
    ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_MIN_QP;
    ctrl[count].size = sizeof (enc->controls.qpmin);
    ctrl[count].value = enc->controls.qpmin;
    count++;
  }

  if (enc->controls.pending_flags & V4L2ENC_FLAG_MAX_QP) {
    ctrl[count].id = V4L2_CID_MPEG_VIDEO_H264_MAX_QP;
    ctrl[count].size = sizeof (enc->controls.qpmax);
    ctrl[count].value = enc->controls.qpmax;
    count++;
  }

  enc->controls.pending_flags = V4L2ENC_FLAG_NONE;
  GST_OBJECT_UNLOCK (enc);

  ctrls.count = count;

  ret = v4l2_ioctl (enc->fd, VIDIOC_S_EXT_CTRLS, &ctrls);
  if (ret < 0) {
    GST_ERROR_OBJECT (enc, "Unable to set controls: %s (%d)",
        strerror (errno), errno);
    return GST_FLOW_ERROR;
  }

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_v4l2enc_set_params (GstVideoEncoder * encoder, GstVideoInfo * info)
{
  GstV4L2Enc *enc = GST_V4L2ENC (encoder);
  struct v4l2_streamparm params;
  struct v4l2_fract timeperframe;
  int ret = 0;

  memset (&params, 0, sizeof (struct v4l2_streamparm));
  memset (&timeperframe, 0, sizeof (struct v4l2_fract));

  /* timeperframe is 1/framerate,so swap numerator and numerator */
  timeperframe.denominator = GST_VIDEO_INFO_FPS_N (info);
  timeperframe.numerator = GST_VIDEO_INFO_FPS_D (info);

  params.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  params.parm.output.timeperframe = timeperframe;
  params.parm.output.capability = V4L2_CAP_TIMEPERFRAME;

  ret = v4l2_ioctl (enc->fd, VIDIOC_S_PARM, &params);
  if (ret < 0) {
    GST_ERROR_OBJECT (enc, "Unable to set params: %s (%d)", strerror (errno),
        errno);
    return GST_FLOW_ERROR;
  } else
    GST_INFO_OBJECT (enc, "set params: timeperframe %d/%d ",
        timeperframe.numerator, timeperframe.denominator);

  return GST_FLOW_OK;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    v4l2enc,
    "V4L2 encoder",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
