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
#include "config.h"
#endif

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include "gstv4l2dec.h"
#include <gst/gst.h>

GST_DEBUG_CATEGORY (gst_v4l2dec_debug);
#define GST_CAT_DEFAULT gst_v4l2dec_debug

#define MAX_DEVICES 20          /* Max V4L2 device instances tried */

#define NB_BUF_INPUT 1
#define NB_BUF_OUTPUT 2         /* nb frames necessary for display pipeline */

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))


#define TIMESTAMP_WITH_PTS_INSTEAD_OF_DTS 1

#define parent_class gst_v4l2dec_parent_class
G_DEFINE_TYPE (GstV4L2Dec, gst_v4l2dec, GST_TYPE_VIDEO_DECODER);

enum
{
  PROP_0,
  PROP_PREFERED_CAPS,
  PROP_SKIPFRAME,
  PROP_LOW_LATENCY,
};

/* GstVideoDecoder base class method */
static gboolean gst_v4l2dec_start (GstVideoDecoder * decoder);
static gboolean gst_v4l2dec_stop (GstVideoDecoder * video_decoder);
static void gst_v4l2dec_release_pending_buffers (GstVideoDecoder * decoder);
static gboolean gst_v4l2dec_set_format (GstVideoDecoder * decoder,
    GstVideoCodecState * state);
static gboolean gst_v4l2_video_dec_flush (GstVideoDecoder * decoder);
static GstFlowReturn gst_v4l2dec_handle_frame (GstVideoDecoder * decoder,
    GstVideoCodecFrame * frame);
static void gst_v4l2dec_finalize (GObject * object);
static gboolean gst_v4l2dec_decide_allocation (GstVideoDecoder * decoder,
    GstQuery * query);
static gboolean
gst_v4l2dec_src_event (GstVideoDecoder * decoder, GstEvent * event);
static gboolean gst_v4l2dec_finish (GstVideoDecoder * decoder);
static void gst_v4l2dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_v4l2dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void unmap_input_buf (GstV4L2Dec * dec);
static GstCaps *gst_v4l2dec_par_fixate_caps (GstV4L2Dec * dec, GstCaps * caps,
    GstCaps * othercaps);
static __u32 to_v4l2_streamformat (GstStructure * s);
static gchar *v4l2_type_str (guint32 fmt);
static gchar *v4l2_fmt_str (guint32 pxfmt);

static GstFlowReturn
gst_v4l2dec_decode (GstVideoDecoder * decoder, GstVideoCodecFrame * frame);
static void frame_push_thread (void *arg);
static void frame_recycle_thread (void *arg);
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

static GstStaticPadTemplate gst_v4l2dec_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-h263,"
        "variant = (string) { itu },"
        "parsed = (boolean) true,"
        "profile = (string) 0"
        ";"
        "video/x-flash-video,"
        "flvversion = (int) 1"
        ";"
        "video/x-h264,"
        "stream-format = (string) { byte-stream },"
        "alignment = (string) { au }"
        ";"
        "video/x-h265,"
        "stream-format = (string) { byte-stream },"
        "alignment = (string) { au }"
        ";"
        "video/mpeg,"
        "mpegversion = (int) { 1, 2, 4 },"
        "systemstream = (boolean) false,"
        "parsed = (boolean) true"
        ";"
        "video/x-xvid;"
        "video/x-3ivx;"
        "video/x-divx,"
        "divxversion = (int) {3, 4, 5},"
        "parsed = (boolean) true"
        ";"
        "video/x-vp8"
        ";"
        "video/x-pn-realvideo,"
        "rmversion = (int) { 3, 4 }"
        ";"
        "video/x-wmv,"
        "wmvversion = (int) 3"
        ";"
        "video/x-cavs,"
        "parsed = (boolean) true,"
        "stream-format = (string) { unit-frame }"
        ";" "video/x-jpeg," "parsed = (boolean) true" ";" "image/jpeg" ";")
    );

static GstStaticPadTemplate gst_v4l2dec_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, "
        "format = (string) { NV12, RGB16, BGRx},"
        "width  = (int) [ 32, 4096 ], " "height =  (int) [ 32, 4096 ]"));

struct pixel_format px_formats[] = {
  {V4L2_PIX_FMT_NV12, (gchar *) "V4L2_PIX_FMT_NV12"},
  {V4L2_PIX_FMT_RGB565, (gchar *) "V4L2_PIX_FMT_RGB565"},
  {V4L2_PIX_FMT_XBGR32, (gchar *) "V4L2_PIX_FMT_XBGR32"},
  {V4L2_PIX_FMT_H264, (gchar *) "V4L2_PIX_FMT_H264"},
#ifdef V4L2_PIX_FMT_HEVC
  {V4L2_PIX_FMT_HEVC, (gchar *) "V4L2_PIX_FMT_HEVC"},
#endif
  {V4L2_PIX_FMT_MPEG1, (gchar *) "V4L2_PIX_FMT_MPEG1"},
  {V4L2_PIX_FMT_MPEG2, (gchar *) "V4L2_PIX_FMT_MPEG2"},
  {V4L2_PIX_FMT_MPEG4, (gchar *) "V4L2_PIX_FMT_MPEG4"},
  {V4L2_PIX_FMT_XVID, (gchar *) "V4L2_PIX_FMT_XVID"},
  {V4L2_PIX_FMT_VP8, (gchar *) "V4L2_PIX_FMT_VP8"},
  {V4L2_PIX_FMT_VC1_ANNEX_G, (gchar *) "V4L2_PIX_FMT_VC1_ANNEX_G"},
  {V4L2_PIX_FMT_VC1_ANNEX_L, (gchar *) "V4L2_PIX_FMT_VC1_ANNEX_L"},
  {V4L2_PIX_FMT_MJPEG, (gchar *) "V4L2_PIX_FMT_MJPEG"},
#ifdef V4L2_PIX_FMT_CAVS
  {V4L2_PIX_FMT_CAVS, (gchar *) "V4L2_PIX_FMT_CAVS"},
#endif
#ifdef V4L2_PIX_FMT_RV30
  {V4L2_PIX_FMT_RV30, (gchar *) "V4L2_PIX_FMT_RV30"},
#endif
#ifdef V4L2_PIX_FMT_RV40
  {V4L2_PIX_FMT_RV40, (gchar *) "V4L2_PIX_FMT_RV40"},
#endif
};

static const gchar *interlace_mode[] = {
  "progressive",
  "interleaved",
  "mixed",
  "fields"
};

/*
 * meta data for buffers acquired from downstream pool
 */
GType
gst_v4l2dec_downstream_meta_api_get_type (void)
{
  static volatile GType type;
  static const gchar *tags[] = { "downstream", NULL };

  if (g_once_init_enter (&type)) {
    GType _type =
        gst_meta_api_type_register ("GstV4L2DecDownstreamMetaAPI", tags);
    g_once_init_leave (&type, _type);
  }
  return type;
}

static gboolean
gst_v4l2dec_downstream_meta_init (GstMeta * meta,
    G_GNUC_UNUSED gpointer params, G_GNUC_UNUSED GstBuffer * buffer)
{
  /* Just to avoid a warning */
  return TRUE;
}

const GstMetaInfo *
gst_v4l2dec_downstream_meta_get_info (void)
{
  static const GstMetaInfo *meta_info = NULL;

  if (g_once_init_enter (&meta_info)) {
    const GstMetaInfo *meta =
        gst_meta_register (gst_v4l2dec_downstream_meta_api_get_type (),
        "GstV4L2DecDownstreamMeta",
        sizeof (GstV4L2DecDownstreamMeta),
        (GstMetaInitFunction) gst_v4l2dec_downstream_meta_init,
        (GstMetaFreeFunction) NULL, (GstMetaTransformFunction) NULL);
    g_once_init_leave (&meta_info, meta);
  }
  return meta_info;
}

static GstVideoInterlaceMode
gst_interlace_mode_from_string (const gchar * mode)
{
  gint i;
  for (i = 0; i < G_N_ELEMENTS (interlace_mode); i++) {
    if (g_str_equal (interlace_mode[i], mode))
      return i;
  }
  return GST_VIDEO_INTERLACE_MODE_PROGRESSIVE;
}

static __u32
to_v4l2_streamformat (GstStructure * s)
{
  if (gst_structure_has_name (s, "video/x-h263"))
    return V4L2_PIX_FMT_H263;

#ifdef V4L2_PIX_FMT_FLV1
  /* Sorenson Spark */
  if (gst_structure_has_name (s, "video/x-flash-video ")) {
    gint flvversion = 0;
    if (gst_structure_get_int (s, "flvversion", &flvversion)) {
      if (flvversion == 1)
        return V4L2_PIX_FMT_FLV1;
    }
    return 0;
  }
#endif

  if (gst_structure_has_name (s, "video/x-h264"))
    return V4L2_PIX_FMT_H264;

#ifdef V4L2_PIX_FMT_HEVC
  if (gst_structure_has_name (s, "video/x-h265"))
    return V4L2_PIX_FMT_HEVC;
#endif

#ifdef V4L2_PIX_FMT_CAVS
  if (gst_structure_has_name (s, "video/x-cavs"))
    return V4L2_PIX_FMT_CAVS;
#endif

  if (gst_structure_has_name (s, "video/x-pn-realvideo")) {
    gint rmversion = 0;
    if (gst_structure_get_int (s, "rmversion", &rmversion)) {
      switch (rmversion) {
#ifdef V4L2_PIX_FMT_RV30
        case 3:
          return V4L2_PIX_FMT_RV30;
          break;
#endif
#ifdef V4L2_PIX_FMT_RV40
        case 4:
          return V4L2_PIX_FMT_RV40;
          break;
#endif
        default:
          return 0;
          break;
      }
    }
  }


  if (gst_structure_has_name (s, "video/mpeg")) {
    gint mpegversion = 0;
    if (gst_structure_get_int (s, "mpegversion", &mpegversion)) {
      switch (mpegversion) {
        case 1:
          return V4L2_PIX_FMT_MPEG1;
          break;
        case 2:
          return V4L2_PIX_FMT_MPEG2;
          break;
        case 4:
          return V4L2_PIX_FMT_MPEG4;
          break;
        default:
          return 0;
          break;
      }
    }
  }

  if (gst_structure_has_name (s, "video/x-xvid") ||
      gst_structure_has_name (s, "video/x-3ivx") ||
      gst_structure_has_name (s, "video/x-divx"))
    return V4L2_PIX_FMT_XVID;

  if (gst_structure_has_name (s, "video/x-vp8"))
    return V4L2_PIX_FMT_VP8;

  if (gst_structure_has_name (s, "video/x-wmv")) {
    const gchar *format;

    if ((format = gst_structure_get_string (s, "format"))
        && (g_str_equal (format, "WVC1")))
      return V4L2_PIX_FMT_VC1_ANNEX_G;
    else
      return V4L2_PIX_FMT_VC1_ANNEX_L;
  }

  if (gst_structure_has_name (s, "image/jpeg"))
    return V4L2_PIX_FMT_MJPEG;

  return 0;
}

static __u32
to_v4l2_pixelformat (GstStructure * s)
{
  if (gst_structure_has_name (s, "video/x-raw")) {
    const gchar *format;

    if ((format = gst_structure_get_string (s, "format"))) {
      if (g_str_equal (format, "NV12"))
        return V4L2_PIX_FMT_NV12;
      else if (g_str_equal (format, "RGB16"))
        return V4L2_PIX_FMT_RGB565;
      else if (g_str_equal (format, "BGRx"))
        return V4L2_PIX_FMT_XBGR32;     /* Actually refers to B-G-R-X */
    }
  }

  return V4L2_PIX_FMT_NV12;
}

/* used for debug I/O type (v4l2 object) */
static gchar *
v4l2_type_str (guint32 type)
{
  int i = 0;
  for (i = 0; i < ARRAY_SIZE (type_io); i++) {
    if (type_io[i].type_io_nb == type)
      return type_io[i].type_io_str;
  }
  return NULL;
}

/* used for debug pixelformat (v4l2 object) */
static gchar *
v4l2_fmt_str (guint32 fmt)
{
  int i = 0;
  for (i = 0; i < ARRAY_SIZE (px_formats); i++) {
    if (px_formats[i].pixel_fmt_nb == fmt)
      return px_formats[i].pixel_fmt_str;
  }
  return NULL;
}


static GstVideoFormat
to_gst_pixelformat (__u32 fmt)
{
  switch (fmt) {
    case V4L2_PIX_FMT_NV12:
      return GST_VIDEO_FORMAT_NV12;
    case V4L2_PIX_FMT_RGB565:
      return GST_VIDEO_FORMAT_RGB16;
    case V4L2_PIX_FMT_XBGR32:
      return GST_VIDEO_FORMAT_BGRx;
    default:
      return GST_VIDEO_FORMAT_UNKNOWN;
  }
}

/* Open the device matching width/height/format as input */
static int
gst_v4l2dec_open_device (GstV4L2Dec * dec, __u32 fmt, __u32 width, __u32 height,
    GstVideoInterlaceMode interlace_mode)
{
  int fd = -1;
  int ret;
  gint i = 0;
  gboolean found;
  gchar path[100];
  struct v4l2_format req_fmt;
  struct v4l2_format try_fmt;
  struct v4l2_format s_fmt;
  int libv4l2_fd;

  memset (&req_fmt, 0, sizeof req_fmt);
  req_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  req_fmt.fmt.pix.width = width;
  req_fmt.fmt.pix.height = height;
  req_fmt.fmt.pix.pixelformat = fmt;

  /* particular case of MJPEG : pixel format could be different that YUV420 */
  if (fmt == V4L2_PIX_FMT_MJPEG)
    req_fmt.fmt.pix.sizeimage = (width * height);       /* for MJPEG, let's take YUV422 (w*h*2) largely with /2 compression (so w*h *2 / 2) */
  else
    req_fmt.fmt.pix.sizeimage = ((width * height * 3 / 2) / 2); /* expecting video compression to acheive /2 */

  if (interlace_mode == GST_VIDEO_INTERLACE_MODE_PROGRESSIVE)
    req_fmt.fmt.pix.field = V4L2_FIELD_NONE;
  else
    req_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

  found = FALSE;
  for (i = 0; i < MAX_DEVICES; i++) {
    snprintf (path, sizeof (path), "/dev/video%d", i);
    fd = open (path, O_RDWR, 0);
    if (fd < 0)
      continue;

    libv4l2_fd = v4l2_fd_open (fd, V4L2_DISABLE_CONVERSION);
    if (libv4l2_fd != -1)
      fd = libv4l2_fd;

    try_fmt = req_fmt;
    ret = v4l2_ioctl (fd, VIDIOC_TRY_FMT, &try_fmt);
    if (ret < 0) {
      v4l2_close (fd);
      continue;
    }

    s_fmt = req_fmt;
    ret = v4l2_ioctl (fd, VIDIOC_S_FMT, &s_fmt);
    if (ret < 0) {
      v4l2_close (fd);
      continue;
    }

    /* device can silently accepts (no error returned) any format
     * and change s_fmt values according to its capabilities, so
     * verify that device comply with requested format
     */
    if (!(s_fmt.fmt.pix.pixelformat == req_fmt.fmt.pix.pixelformat &&
            s_fmt.fmt.pix.width == req_fmt.fmt.pix.width &&
            s_fmt.fmt.pix.height == req_fmt.fmt.pix.height)) {
      v4l2_close (fd);
      continue;
    }

    found = TRUE;
    break;
  }

  if (!found) {
    GST_ERROR_OBJECT (dec,
        "No device found matching format %s(0x%x) and resolution %dx%d",
        v4l2_fmt_str (fmt), fmt, width, height);
    return -1;
  }

  /* Update low latency mode at the very beginning if needed */
  if (dec->low_latency_mode) {
    struct v4l2_control ctrl;

    memset (&ctrl, 0, sizeof ctrl);
    ctrl.id = V4L2_CID_USER_STA_VPU_LOW_LATENCY;
    ctrl.value = 1;
    if (v4l2_ioctl (fd, VIDIOC_S_CTRL, &ctrl) < 0) {
      GST_WARNING_OBJECT (dec,
          "Output VIDIOC_S_CTRL Low latency failed."
          " Mode remains in normal mode");
      dec->low_latency_mode = FALSE;
    }
  }

  GST_INFO_OBJECT (dec, "Device %s opened for format %s, %dx%d resolution%s",
      path, v4l2_fmt_str (fmt), width, height,
      (dec->low_latency_mode ? ", Low latency mode" : ""));

  return fd;
}

static void
gst_v4l2dec_process_codec_data (GstV4L2Dec * dec,
    __u32 streamformat, GstBuffer * codec_data)
{
  if (streamformat == V4L2_PIX_FMT_H264) {
    GstBuffer *header = NULL;
    unsigned int sps_size;
    unsigned int pps_size;
    GstMapInfo header_mapinfo = { 0, };
    GstMapInfo mapinfo = { 0, };
    unsigned char *codec_data_ptr;
    unsigned char *sps_pps_ptr;

    header = gst_buffer_new_and_alloc (gst_buffer_get_size (codec_data));
    gst_buffer_map (header, &header_mapinfo, GST_MAP_WRITE);
    sps_pps_ptr = header_mapinfo.data;

    gst_buffer_map (codec_data, &mapinfo, GST_MAP_READ);
    codec_data_ptr = mapinfo.data;

    /* Header decomposition */
    /* <7 bytes><SPS header size><SPS Header><3 bytes><PPS Header> */

    /* <7 bytes> to skip */
    codec_data_ptr += 7;

    /* <SPS header size> on 1 byte */
    sps_size = *codec_data_ptr;
    codec_data_ptr += 1;

    /* 4 bytes SPS startcode to add */
    sps_pps_ptr[0] = 0x00;
    sps_pps_ptr[1] = 0x00;
    sps_pps_ptr[2] = 0x00;
    sps_pps_ptr[3] = 0x01;
    sps_pps_ptr += 4;

    /* <SPS> on <SPS header size> bytes */
    memcpy (sps_pps_ptr, codec_data_ptr, sps_size);
    codec_data_ptr += sps_size;
    sps_pps_ptr += sps_size;

    /* <2 bytes> to skip */
    codec_data_ptr += 2;

    /* <PPS header size> on 1 byte */
    pps_size = *codec_data_ptr;
    codec_data_ptr += 1;

    /* 4 bytes PPS startcode to add */
    sps_pps_ptr[0] = 0x00;
    sps_pps_ptr[1] = 0x00;
    sps_pps_ptr[2] = 0x00;
    sps_pps_ptr[3] = 0x01;
    sps_pps_ptr += 4;

    /* <PPS> on <PPS header size> bytes */
    memcpy (sps_pps_ptr, codec_data_ptr, pps_size);

    sps_size += 4;
    pps_size += 4;

    gst_buffer_unmap (codec_data, &mapinfo);
    gst_buffer_unmap (header, &header_mapinfo);

    gst_buffer_set_size (header, sps_size + pps_size);
    dec->header = header;
  }
}

gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "v4l2dec", GST_RANK_PRIMARY + 1,
          GST_TYPE_V4L2DEC))
    return FALSE;
  return TRUE;
}

static void
gst_v4l2dec_class_init (GstV4L2DecClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoDecoderClass *video_decoder_class = GST_VIDEO_DECODER_CLASS (klass);
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_v4l2dec_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_v4l2dec_get_property);

  g_object_class_install_property (gobject_class, PROP_PREFERED_CAPS,
      g_param_spec_boxed ("preferred-caps", "Prefered caps",
          "Specify the preferred caps to be emited on src pad. "
          "This will potentially enable decoder scaler or color converter stages depending on "
          "decoder capabilities and video bitstream native coding. "
          "If set to NULL, output caps will match video bitstream native coding",
          GST_TYPE_CAPS, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SKIPFRAME,
      g_param_spec_boolean ("skip-frame", "Skip frames",
          "enable QOS : 'drop inter-frames' algorithm in case of underflow",
          FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_LOW_LATENCY,
      g_param_spec_boolean ("low-latency", "Low latency mode (H264 only)",
          "Enable/Disable low latency mode. Only available for H264 codec. "
          "To use in case of streaming use-case only.",
          FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gobject_class->finalize = gst_v4l2dec_finalize;

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_v4l2dec_src_template));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_v4l2dec_sink_template));

  video_decoder_class->start = GST_DEBUG_FUNCPTR (gst_v4l2dec_start);
  video_decoder_class->stop = GST_DEBUG_FUNCPTR (gst_v4l2dec_stop);
  video_decoder_class->set_format = GST_DEBUG_FUNCPTR (gst_v4l2dec_set_format);
  video_decoder_class->flush = GST_DEBUG_FUNCPTR (gst_v4l2_video_dec_flush);
  video_decoder_class->handle_frame =
      GST_DEBUG_FUNCPTR (gst_v4l2dec_handle_frame);
  video_decoder_class->decide_allocation =
      GST_DEBUG_FUNCPTR (gst_v4l2dec_decide_allocation);
  video_decoder_class->finish = GST_DEBUG_FUNCPTR (gst_v4l2dec_finish);
  video_decoder_class->src_event = gst_v4l2dec_src_event;
  GST_DEBUG_CATEGORY_INIT (gst_v4l2dec_debug, "v4l2dec", 0,
      "ST v4l2 video decoder");

  gst_element_class_set_static_metadata (element_class,
      "V4L2 decoder", "Decoder/Video", "A v4l2 decoder", "STMicroelectronics");
}

/* Service used to set underflow state :
 * A list of previous values (number is defined per HISTORY_LENGHT) is
 * systematically kept in memory and analysed to compute the trend.
 * If the trend is negative, it means that we enter underflow state, else
 * we are going out underflow state.
 */
static gboolean
gst_v4l2dec_qos_underflow_detected (GstVideoDecoder * decoder,
    GstClockTimeDiff diff)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstV4L2DecQosElt *pelt = malloc (sizeof (GstV4L2DecQosElt));
  GstV4L2DecQosElt *p = NULL;
  GstV4L2DecQosElt *prev = NULL;
  GstV4L2DecQosElt *listbegin = NULL;
  int previous_trend = 0;
  int trend = 0;
  int nb_elt = 0;
  gboolean previous_underflow_state;

  GST_OBJECT_LOCK (decoder);
  previous_underflow_state = dec->qos_underflow;
  if (pelt) {
    pelt->diff = diff;
    pelt->pnext = NULL;
  }

  if (!dec->qos_trace) {
    dec->qos_trace = pelt;
    dec->qos_underflow = FALSE;
    GST_OBJECT_UNLOCK (decoder);
    return FALSE;
  }

  /* Size of the list is limited to HISTORY_LENGHT elements */
  p = dec->qos_trace;
  listbegin = dec->qos_trace;
  while (p) {
    nb_elt++;
    prev = p;
    p = p->pnext;
    if (nb_elt >= HISTORY_LENGHT) {
      GstV4L2DecQosElt *previous_start = listbegin;
      listbegin = previous_start->pnext;
      free (previous_start);
    }
  }
  prev->pnext = pelt;
  dec->qos_trace = listbegin;

  if (nb_elt < HISTORY_LENGHT) {
    GST_OBJECT_UNLOCK (decoder);
    return FALSE;
  }
  /* Parse the HISTORY_LENGHT elements to identify the trend */
  p = dec->qos_trace;
  nb_elt = 0;
  while (p) {
    nb_elt++;
    trend = (p->diff <= 0 ? -1 : 1);
    GST_DEBUG_OBJECT (decoder,
        "QOS: Elt[%d] => %" G_GINT64_FORMAT
        " [trend = %d, previous_trend = %d]", nb_elt, p->diff, trend,
        previous_trend);
    if (previous_trend == 0)
      previous_trend = trend;
    else if (previous_trend != trend)
      break;
    p = p->pnext;
  }

  if (previous_trend == trend)
    dec->qos_underflow = (diff <= 0);

  if (dec->qos_underflow != previous_underflow_state)
    GST_WARNING_OBJECT (decoder,
        "QOS: Underflow : %s (based on %d consecutive frames) , decoding time %"
        G_GINT64_FORMAT, (dec->qos_underflow ? "yes" : "no"), HISTORY_LENGHT,
        diff);

  GST_OBJECT_UNLOCK (decoder);
  return dec->qos_underflow;
}

static gboolean
gst_v4l2dec_src_event (GstVideoDecoder * decoder, GstEvent * event)
{
  if (GST_EVENT_TYPE (event) == GST_EVENT_QOS) {
    GstQOSType type;
    GstClockTimeDiff diff;
    GstClockTime timestamp;
    gdouble proportion;

    gst_event_parse_qos (event, &type, &proportion, &diff, &timestamp);
    gst_v4l2dec_qos_underflow_detected (decoder, -1 * diff);
  }
  return GST_VIDEO_DECODER_CLASS (parent_class)->src_event (decoder, event);
}

static gboolean
gst_v4l2dec_decide_allocation (GstVideoDecoder * decoder, GstQuery * query)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstCaps *outcaps, *caps;
  guint allocator_nb, i;
  GstAllocator *allocator;
  GstAllocationParams params;
  GstStructure *config;
  struct v4l2_requestbuffers reqbufs;
  GstVideoInfo info;
  GstVideoAlignment align;
  guint width, height, aligned_width, aligned_height;
  struct v4l2_format s_fmt;
  struct v4l2_format g_fmt;
  struct v4l2_selection selection;

restart_allocation:
  gst_query_parse_allocation (query, &outcaps, NULL);

  /* get the allocator array size */
  allocator_nb = gst_query_get_n_allocation_params (query);
  GST_DEBUG_OBJECT (decoder, "Got %d allocator(s)", allocator_nb);

  if (allocator_nb > 0) {
    /* parse the allocators: select the dmabuf allocator if present,
     * else select the first memory allocator in the query
     */
    for (i = 0; i < allocator_nb; i++) {
      gst_query_parse_nth_allocation_param (query, i, &allocator, &params);
      if (allocator) {
        GstBufferPool *pool;
        guint size, min_buffers, max_buffers;
        guint pool_size, pool_min_buffers, pool_max_buffers;

        GST_DEBUG_OBJECT (decoder, "Got %s allocator", allocator->mem_type);

        if (!g_strcmp0 (allocator->mem_type, GST_ALLOCATOR_DMABUF)) {
          /* If already configured/active, simply read back the active configuration */
          /* FIXME not entirely correct, See bug 728268 */
          if (dec->downstream_pool
              && gst_buffer_pool_is_active (dec->downstream_pool)) {
            GST_DEBUG_OBJECT (decoder, "Pool %s is already active",
                dec->downstream_pool->object.name);

            gst_query_parse_nth_allocation_pool (query, 0, &pool, &size,
                &min_buffers, &max_buffers);

            config = gst_buffer_pool_get_config (dec->downstream_pool);
            gst_buffer_pool_config_get_params (config, NULL, &pool_size,
                &pool_min_buffers, &pool_max_buffers);
            gst_structure_free (config);
            gst_object_unref (pool);

            if (size != pool_size) {
              /* Potentially the configuration has been updated so that pool
               * previously allocated is not well appropriate. In this case,
               * we destroy the pool and restart allocation procedure
               */
              GST_WARNING_OBJECT (decoder,
                  "Incoherent size between query (%d) and pool (%d) "
                  "destroy the pool and recreate a new one", size, pool_size);
              gst_buffer_pool_set_active (dec->downstream_pool, FALSE);
              gst_object_unref (dec->downstream_pool);
              dec->downstream_pool = NULL;
              goto restart_allocation;
            }

            gst_query_add_allocation_pool (query, dec->downstream_pool,
                pool_size, pool_min_buffers, pool_max_buffers);
            return TRUE;
          } else {
            gst_query_set_nth_allocation_param (query, 0, allocator, &params);

            gst_query_parse_nth_allocation_pool (query, 0, &pool, &size,
                &min_buffers, &max_buffers);

            GST_DEBUG_OBJECT (decoder,
                "Found %s downstream DMA-Buf pool %p with min=%d max=%d ",
                pool->object.name, pool, min_buffers, max_buffers);

            /* Request output buffers needed (DMABUF mode) */
            memset (&reqbufs, 0, sizeof reqbufs);
            reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            reqbufs.count = (min_buffers == 0 ? NB_BUF_OUTPUT : min_buffers);
            reqbufs.memory = V4L2_MEMORY_DMABUF;

            if (v4l2_ioctl (dec->fd, VIDIOC_REQBUFS, &reqbufs) < 0)
              goto error_ioc_reqbufs;

            /* request that downstream pool allocates at least driver need (reqbufs.count) */
            min_buffers = max_buffers = reqbufs.count;

            /* specify the buffer size */
            size = dec->size_image;

            config = gst_buffer_pool_get_config (pool);

            gst_query_parse_allocation (query, &caps, NULL);
            if (caps)
              gst_buffer_pool_config_set_params (config, caps, size,
                  min_buffers, max_buffers);

            if (gst_query_find_allocation_meta (query, GST_VIDEO_META_API_TYPE,
                    NULL)) {
              gst_buffer_pool_config_add_option (config,
                  GST_BUFFER_POOL_OPTION_VIDEO_META);
              gst_buffer_pool_config_add_option (config,
                  GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);
              GST_LOG_OBJECT (decoder,
                  "Set downstream pool %p padding to %u-%ux%u-%u", pool,
                  dec->align.padding_top, dec->align.padding_left,
                  dec->align.padding_right, dec->align.padding_bottom);
              gst_buffer_pool_config_set_video_alignment (config, &dec->align);
            }
            if (gst_buffer_pool_set_config (pool, config)) {
              /* Check if config has been updated by downstream element */
              config = gst_buffer_pool_get_config (pool);
              /* Get sizes */
              gst_buffer_pool_config_get_params (config, &caps, &size,
                  &min_buffers, &max_buffers);
              gst_video_info_from_caps (&info, caps);
              if (gst_buffer_pool_config_has_option (config,
                      GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT)) {
                gst_buffer_pool_config_get_video_alignment (config, &align);
                GST_DEBUG_OBJECT (decoder, "buffers have padding: %d %d %d %d",
                    align.padding_left, align.padding_top, align.padding_right,
                    align.padding_bottom);
              } else {
                gst_video_alignment_reset (&align);
              }
              gst_structure_free (config);
              width = GST_VIDEO_INFO_WIDTH (&info);
              height = GST_VIDEO_INFO_HEIGHT (&info);
              aligned_width = width + align.padding_left + align.padding_right;
              aligned_height =
                  height + align.padding_top + align.padding_bottom;

              GST_DEBUG_OBJECT (decoder, "Updated Video : %dx%d aligned: %dx%d",
                  width, height, aligned_width, aligned_height);

              /* Set format */
              memset (&s_fmt, 0, sizeof s_fmt);
              s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
              s_fmt.fmt.pix.width = aligned_width;
              s_fmt.fmt.pix.height = aligned_height;
              s_fmt.fmt.pix.pixelformat =
                  to_v4l2_pixelformat (gst_caps_get_structure (caps, 0));
              s_fmt.fmt.pix.sizeimage = size;

              GST_DEBUG_OBJECT (decoder, "Using %dx%d (%dx%d) - size=%d", width,
                  height, aligned_width, aligned_height, size);

              if (v4l2_ioctl (dec->fd, VIDIOC_S_FMT, &s_fmt) < 0) {
                GST_ERROR_OBJECT (decoder, "Capture VIDIOC_S_FMT failed");
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

                if (v4l2_ioctl (dec->fd, VIDIOC_S_SELECTION, &selection) < 0) {
                  GST_ERROR_OBJECT (decoder,
                      "Capture VIDIOC_S_SELECTION failed");
                  return FALSE;
                }
              }
              memset (&g_fmt, 0, sizeof g_fmt);
              g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
              if (v4l2_ioctl (dec->fd, VIDIOC_G_FMT, &g_fmt) < 0) {
                GST_WARNING_OBJECT (decoder,
                    "Capture VIDIOC_G_FMT failed : Format not yet determinated");
                return FALSE;
              }
            }
            gst_query_set_nth_allocation_pool (query, 0, pool, size,
                min_buffers, max_buffers);
            /* Select this downstream pool */
            dec->downstream_pool = pool;
          }
          gst_object_unref (allocator);
        }
      } else {
        GST_DEBUG_OBJECT (decoder, "Allocator is NULL (number of allocator=%d)",
            allocator_nb);
      }
    }
  }

  return GST_VIDEO_DECODER_CLASS (parent_class)->decide_allocation (decoder,
      query);

  /* ERRORS */
error_ioc_reqbufs:
  {
    GST_ERROR_OBJECT (dec, "Unable to request CAPTURE buffers err=%s",
        strerror (errno));
    return FALSE;
  }
}

/* check if we have to skip frame before decoding the next frame.
 *
 * Based on underflow status, we check if we have to drop or not a frame
 * before asking to decode it
 * if you defined QOS_DEGRADATION_LEVEL : we won't wait for next keyframe
 * before re-entering normal mode => you'll probably restart decoding
 * on B or P frames ( meaning macro-blocks )
 * if not defined (by default)  : we wait for IFrame before re-entering
 * normal mode to avoid macro-block
 */
static gboolean
gst_v4l2dec_qos_check_skip_frame (GstVideoDecoder * decoder, gboolean is_intra)
{
  gboolean skip_frame = FALSE;
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);

  GST_OBJECT_LOCK (decoder);
  switch (dec->frame_discard_state) {
    case V4L2DEC_DEFAULT:
      skip_frame = FALSE;
      if (dec->qos_underflow) {
        GST_WARNING_OBJECT (decoder, "QOS: switch to hurry mode");
        dec->frame_discard_state = V4L2DEC_DESYNCHRO;
        skip_frame = TRUE;
      }
      break;
    case V4L2DEC_DESYNCHRO:
      /* skip every frames except Iframe */
      skip_frame = TRUE;
      if (!dec->qos_underflow) {
#ifdef QOS_DEGRADATION_LEVEL
        GST_WARNING_OBJECT (decoder, "QOS: switch to normal mode");
        dec->frame_discard_state = V4L2DEC_DEFAULT;
        skip_frame = FALSE;
#else
        GST_WARNING_OBJECT (decoder,
            "QOS: go out from hurry mode, wait for intra frame");
        dec->frame_discard_state = V4L2DEC_WAITFOR_IFRAME;
#endif
      }
      break;
    case V4L2DEC_WAITFOR_IFRAME:
      skip_frame = TRUE;
      if (dec->qos_underflow) {
        GST_WARNING_OBJECT (decoder, "QOS: switch back to hurry mode");
        dec->frame_discard_state = V4L2DEC_DESYNCHRO;
      } else {
        if (is_intra) {
          GST_WARNING_OBJECT (decoder,
              "QOS: intra frame detected => switch to normal mode");
          dec->frame_discard_state = V4L2DEC_DEFAULT;
          skip_frame = FALSE;
        }
      }
      break;
    default:;
  }
  GST_OBJECT_UNLOCK (decoder);
  return skip_frame;
}

/* Init the v4l2dec structure */
static void
gst_v4l2dec_init (GstV4L2Dec * dec)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) dec;

  dec->current_nb_buf_input = 0;

  gst_video_decoder_set_packetized (decoder, TRUE);

  dec->mmap_virtual_input = NULL;
  dec->mmap_size_input = NULL;

  dec->output_setup = FALSE;
  dec->input_setup = FALSE;

  dec->input_streaming = FALSE;
  dec->output_streaming = FALSE;

  dec->frame_push_task = gst_task_new (frame_push_thread, decoder, NULL);
  g_rec_mutex_init (&dec->frame_push_task_mutex);
  gst_task_set_lock (dec->frame_push_task, &dec->frame_push_task_mutex);

  dec->frame_recycle_task = gst_task_new (frame_recycle_thread, decoder, NULL);
  g_rec_mutex_init (&dec->frame_recycle_task_mutex);
  gst_task_set_lock (dec->frame_recycle_task, &dec->frame_recycle_task_mutex);

  dec->header = NULL;
  dec->fd = -1;
  dec->eos_driver = FALSE;

  dec->preferred_caps = NULL;

  dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].p = gst_poll_new (TRUE);
  dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].p = gst_poll_new (TRUE);
  dec->can_poll_device = TRUE;
  dec->frame_discard_state = V4L2DEC_DEFAULT;
  dec->qos_underflow = FALSE;
  dec->qos_trace = NULL;
  dec->skip_frame = FALSE;
}

static void
gst_v4l2dec_finalize (GObject * object)
{
  GstV4L2Dec *dec = GST_V4L2DEC (object);
  GstV4L2DecQosElt *p = NULL;
  GstV4L2DecQosElt *n = NULL;

  GST_DEBUG_OBJECT (dec, "Finalizing");

  /* wait for all running threads termination */
  if (gst_task_get_state (dec->frame_push_task) != GST_TASK_STOPPED) {
    GST_DEBUG_OBJECT (dec, "task %p should be stopped by now",
        dec->frame_push_task);
    gst_task_stop (dec->frame_push_task);
    g_rec_mutex_lock (&dec->frame_push_task_mutex);
    g_rec_mutex_unlock (&dec->frame_push_task_mutex);
    gst_task_join (dec->frame_push_task);
  }
  GST_DEBUG_OBJECT (dec, "Push task stopped");

  gst_object_unref (dec->frame_push_task);
  dec->frame_push_task = NULL;
  g_rec_mutex_clear (&dec->frame_push_task_mutex);

  if (gst_task_get_state (dec->frame_recycle_task) != GST_TASK_STOPPED) {
    GST_DEBUG_OBJECT (dec, "task %p should be stopped by now",
        dec->frame_recycle_task);
    gst_task_stop (dec->frame_recycle_task);
    g_rec_mutex_lock (&dec->frame_recycle_task_mutex);
    g_rec_mutex_unlock (&dec->frame_recycle_task_mutex);
    gst_task_join (dec->frame_recycle_task);
  }
  GST_DEBUG_OBJECT (dec, "Recycle task stopped");

  gst_poll_free (dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].p);
  dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].p = NULL;
  gst_poll_free (dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].p);
  dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].p = NULL;

  gst_object_unref (dec->frame_recycle_task);
  dec->frame_recycle_task = NULL;
  g_rec_mutex_clear (&dec->frame_recycle_task_mutex);

  if (dec->preferred_caps)
    gst_caps_unref (dec->preferred_caps);

  /* Free QOS list */
  p = dec->qos_trace;
  while (p) {
    n = p->pnext;
    free (p);
    p = n;
  }
  dec->qos_trace = NULL;

  GST_DEBUG_OBJECT (dec, "Finalized");
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_v4l2dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstV4L2Dec *dec = GST_V4L2DEC (object);

  GST_DEBUG_OBJECT (dec, "Set property");

  switch (prop_id) {
    case PROP_PREFERED_CAPS:{
      GstCaps *caps;
      const GstCaps *caps_val = gst_value_get_caps (value);

      if (!caps_val) {
        caps = gst_caps_new_any ();
      } else {
        caps = (GstCaps *) caps_val;
        gst_caps_ref (caps);
      }
      dec->preferred_caps = caps;
      GST_DEBUG_OBJECT (dec, "Prefered caps set to %" GST_PTR_FORMAT,
          dec->preferred_caps);
      break;
    }
    case PROP_SKIPFRAME:
      dec->skip_frame = g_value_get_boolean (value);
      break;
    case PROP_LOW_LATENCY:
      dec->low_latency_mode = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_v4l2dec_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstV4L2Dec *dec = GST_V4L2DEC (object);

  GST_DEBUG_OBJECT (dec, "Get property");

  switch (prop_id) {
    case PROP_PREFERED_CAPS:
      gst_value_set_caps (value, dec->preferred_caps);
      break;
    case PROP_SKIPFRAME:
      g_value_set_boolean (value, dec->skip_frame);
      break;
    case PROP_LOW_LATENCY:
      g_value_set_boolean (value, dec->low_latency_mode);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* Open the device */
static gboolean
gst_v4l2dec_start (GstVideoDecoder * decoder)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  gchar *name;

  GST_DEBUG_OBJECT (dec, "Starting");

  /* task name cannot be set right after task_new() in gst_v4l2dec_init()
   * because object name is set by base classes only after _init() call...
   */
  name =
      g_strconcat (gst_object_get_name (GST_OBJECT (dec)), ":", "push", NULL);
  gst_object_set_name (GST_OBJECT (dec->frame_push_task), name);
  g_free (name);
  name =
      g_strconcat (gst_object_get_name (GST_OBJECT (dec)), ":", "recycle",
      NULL);
  gst_object_set_name (GST_OBJECT (dec->frame_recycle_task), name);
  g_free (name);

  return TRUE;
}

static gboolean
gst_v4l2dec_finish (GstVideoDecoder * decoder)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  struct v4l2_decoder_cmd cmd;

  GST_DEBUG_OBJECT (dec, "Finish handling");

  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  /* send decoder stop command */
  memset (&cmd, 0, sizeof (cmd));
  cmd.cmd = V4L2_DEC_CMD_STOP;
  cmd.flags = 0;
  if (v4l2_ioctl (dec->fd, VIDIOC_DECODER_CMD, &cmd) < 0) {
    GST_WARNING_OBJECT (dec, "Unable to send command to decoder err=%s",
        strerror (errno));
    return TRUE;
  }

  /* block till EOS reached on frame push thread side */
  g_mutex_lock (&dec->eos_mutex);

  while (!dec->eos_driver)
    g_cond_wait (&dec->eos_cond, &dec->eos_mutex);

  dec->eos_driver = FALSE;
  g_mutex_unlock (&dec->eos_mutex);

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);

  GST_DEBUG_OBJECT (dec, "Finished");

  return TRUE;
}

/* Stop Stream + Munmaping + Close thread and device */
static gboolean
gst_v4l2dec_stop (GstVideoDecoder * decoder)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  gint type;
  GST_DEBUG_OBJECT (dec, "Stopping");

  dec->input_setup = FALSE;
  dec->output_setup = FALSE;

  if (dec->header) {
    gst_buffer_unref (dec->header);
    dec->header = NULL;
  }

  if (dec->fd != -1) {
    if (dec->input_streaming) {
      type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
      if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
        GST_WARNING_OBJECT (dec, "Unable to stop stream on OUTPUT err=%s",
            strerror (errno));
      dec->input_streaming = FALSE;
    }

    if (dec->output_streaming) {
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
        GST_WARNING_OBJECT (dec, "Unable to stop stream on CAPTURE err=%s",
            strerror (errno));
      dec->output_streaming = FALSE;
    }
  }

  if (dec->input_state) {
    gst_video_codec_state_unref (dec->input_state);
    dec->input_state = NULL;
  }
  if (dec->output_state) {
    gst_video_codec_state_unref (dec->output_state);
    dec->output_state = NULL;
  }

  if (dec->downstream_pool) {
    gst_buffer_pool_set_active (dec->downstream_pool, FALSE);
    gst_v4l2dec_release_pending_buffers (decoder);
  }

  if (dec->pool) {
    gst_buffer_pool_set_active (dec->pool, FALSE);
  }

  /* wait for all running threads termination */
  if (gst_task_get_state (dec->frame_push_task) != GST_TASK_STOPPED) {
    GST_DEBUG_OBJECT (dec, "task %p should be stopped by now",
        dec->frame_push_task);
    gst_task_stop (dec->frame_push_task);
    g_rec_mutex_lock (&dec->frame_push_task_mutex);
    g_rec_mutex_unlock (&dec->frame_push_task_mutex);
    gst_task_join (dec->frame_push_task);

    /* Raise EOS when stopping push thread
     * in order to not deadlock gst_v4l2dec_finish()... */
    g_mutex_lock (&dec->eos_mutex);
    dec->eos_driver = TRUE;
    g_cond_signal (&dec->eos_cond);
    g_mutex_unlock (&dec->eos_mutex);

    GST_DEBUG_OBJECT (dec, "Push task stopped");
  }

  if (gst_task_get_state (dec->frame_recycle_task) != GST_TASK_STOPPED) {
    GST_DEBUG_OBJECT (dec, "task %p should be stopped by now",
        dec->frame_recycle_task);
    gst_task_stop (dec->frame_recycle_task);
    g_rec_mutex_lock (&dec->frame_recycle_task_mutex);
    g_rec_mutex_unlock (&dec->frame_recycle_task_mutex);
    gst_task_join (dec->frame_recycle_task);
    GST_DEBUG_OBJECT (dec, "Recycle task stopped");
  }

  /* release downstream pool */
  if (dec->downstream_pool) {
    gst_object_unref (dec->downstream_pool);
    dec->downstream_pool = NULL;
  }

  if (dec->pool) {
    gst_object_unref (dec->pool);
    dec->pool = NULL;
  }
  unmap_input_buf (dec);

  if (dec->mmap_virtual_input) {
    free (dec->mmap_virtual_input);
    dec->mmap_virtual_input = NULL;

    free (dec->mmap_size_input);
    dec->mmap_size_input = NULL;

    dec->current_nb_buf_input = 0;
  }

  if (dec->fd != -1) {
    v4l2_close (dec->fd);
    dec->fd = -1;
  }

  GST_DEBUG_OBJECT (dec, "Stopped");

  return TRUE;
}

/* release buffers acquired from downstream pool */
/* should be called after capture stream off */
static void
gst_v4l2dec_release_pending_buffers (GstVideoDecoder * decoder)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstV4L2DecDownstreamMeta *meta = NULL;
  gint i;

  for (i = 0; i < ARRAY_SIZE (dec->downstream_buffers); i++) {
    GstBuffer *buffer = dec->downstream_buffers[i];
    if (buffer) {
      meta = GST_V4L2DEC_DOWNSTREAM_META_GET (buffer);
      if (meta && meta->acquired) {
        GST_DEBUG_OBJECT (dec,
            "pending acquired buffer %p, index %d, force release", buffer, i);
        gst_buffer_unref (buffer);
      }
    }
  }
  return;
}

/* Setup input (OUTPUT for V4L2) */
static gboolean
gst_v4l2dec_set_format (GstVideoDecoder * decoder, GstVideoCodecState * state)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstStructure *structure;
  const gchar *s;
  gint i;
  gint type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  gint retval = 0;
  gint width = 0;
  gint height = 0;
  GstVideoInterlaceMode interlace_mode;

  __u32 streamformat;
  int fd;
  struct v4l2_requestbuffers reqbuf;
  struct v4l2_buffer querybuf;
  struct v4l2_buffer qbuf;

  GST_DEBUG_OBJECT (dec, "Setting format: %" GST_PTR_FORMAT, state->caps);

  structure = gst_caps_get_structure (state->caps, 0);

  streamformat = to_v4l2_streamformat (structure);
  if (!streamformat)
    goto error_format;

  retval = gst_structure_get_int (structure, "width", &width);
  retval &= gst_structure_get_int (structure, "height", &height);
  if (!retval)
    goto error_res;

  if ((s = gst_structure_get_string (structure, "interlace-mode")))
    interlace_mode = gst_interlace_mode_from_string (s);
  else
    interlace_mode = GST_VIDEO_INTERLACE_MODE_PROGRESSIVE;

  if (dec->input_setup) {
    /* Already setup, check to see if something has changed on input caps... */
    if ((dec->streamformat == streamformat) &&
        (dec->width == width) && (dec->height == height)) {
      goto done;                /* Nothing has changed */
    } else {
      /* V4L2_DEC_CMD_STOP: this command should be called on v4l2 decoder
       * to force decoder to decode all pending buffers and put an empty buffer
       * at end with V4L2_BUF_FLAG_LAST flag in V4L2 capture outgoing queue.
       * TBD in Ticket 74968 - require VIDIOC_DECODER_CMD in v4l2 driver
       */
      GST_FIXME_OBJECT (dec, "TBD: require V4L2_DEC_CMD_STOP in v4l2 driver");
      /* Unlock decoder so that frame_push_thread terminates its execution lock
       * free
       */
      GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);
      gst_v4l2dec_stop (decoder);
      GST_VIDEO_DECODER_STREAM_LOCK (decoder);
    }
  }

  fd = gst_v4l2dec_open_device (dec, streamformat, width, height,
      interlace_mode);
  if (fd == -1)
    goto error_device;

  dec->fd = fd;
  dec->streamformat = streamformat;
  dec->width = width;
  dec->height = height;

  gst_poll_fd_init (&dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].fd);
  dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].fd.fd = dec->fd;
  gst_poll_add_fd (dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].p,
      &dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].fd);
  gst_poll_fd_ctl_write (dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].p,
      &dec->poll[V4L2_BUF_TYPE_VIDEO_OUTPUT].fd, TRUE);

  gst_poll_fd_init (&dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].fd);
  dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].fd.fd = dec->fd;
  gst_poll_add_fd (dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].p,
      &dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].fd);
  gst_poll_fd_ctl_write (dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].p,
      &dec->poll[V4L2_BUF_TYPE_VIDEO_CAPTURE].fd, TRUE);

  if (dec->input_state)
    gst_video_codec_state_unref (dec->input_state);
  dec->input_state = gst_video_codec_state_ref (state);

  /* Header */
  dec->codec_data = state->codec_data;
  if (dec->codec_data)
    gst_v4l2dec_process_codec_data (dec, streamformat, dec->codec_data);

  /* Memory mapping for input buffers in V4L2 */
  memset (&reqbuf, 0, sizeof reqbuf);
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  reqbuf.count = NB_BUF_INPUT;
  reqbuf.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl (dec->fd, VIDIOC_REQBUFS, &reqbuf) < 0)
    goto error_ioc_reqbufs;

  dec->mmap_virtual_input = malloc (sizeof (void *) * reqbuf.count);
  dec->mmap_size_input = malloc (sizeof (void *) * reqbuf.count);

  memset (&querybuf, 0, sizeof querybuf);
  querybuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  querybuf.memory = V4L2_MEMORY_MMAP;
  for (i = 0; i < reqbuf.count; i++) {
    void *ptr;
    querybuf.index = i;
    dec->current_nb_buf_input++;

    /* Memory mapping for input buffers in GStreamer */
    if (v4l2_ioctl (dec->fd, VIDIOC_QUERYBUF, &querybuf) < 0)
      goto error_ioc_querybuf;
    ptr = v4l2_mmap (NULL, querybuf.length, PROT_READ | PROT_WRITE,
        MAP_SHARED, dec->fd, querybuf.m.offset);
    if (ptr == MAP_FAILED)
      goto error_map_fail;

    dec->mmap_virtual_input[i] = ptr;
    dec->mmap_size_input[i] = querybuf.length;

    qbuf = querybuf;            /* index from querybuf */
    qbuf.bytesused = 0;         /* enqueue it with no data */
    if (v4l2_ioctl (dec->fd, VIDIOC_QBUF, &qbuf) < 0)
      goto error_ioc_qbuf;
  }

  /* Start stream on input */
  if (v4l2_ioctl (dec->fd, VIDIOC_STREAMON, &type) < 0)
    goto error_ioc_streamon;
  dec->input_streaming = TRUE;

  dec->input_setup = TRUE;

done:
  return TRUE;

  /* Errors */
error_format:
  {
    GST_ERROR_OBJECT (dec, "Unsupported format in caps: %" GST_PTR_FORMAT,
        state->caps);
    return FALSE;
  }
error_res:
  {
    GST_ERROR_OBJECT (dec, "Unable to get width/height value");
    return FALSE;
  }
error_device:
  {
    return FALSE;
  }
error_ioc_reqbufs:
  {
    GST_ERROR_OBJECT (dec, "Unable to request input buffers err=%s",
        strerror (errno));
    return FALSE;
  }
error_ioc_querybuf:
  {
    GST_ERROR_OBJECT (dec, "Query of input buffer failed err=%s",
        strerror (errno));
    return FALSE;
  }
error_map_fail:
  {
    GST_ERROR_OBJECT (dec, "Failed to map input buffer");
    return FALSE;
  }
error_ioc_qbuf:
  {
    GST_ERROR_OBJECT (dec, "Enqueuing buffer failed err=%s", strerror (errno));
    unmap_input_buf (dec);
    return FALSE;
  }
error_ioc_streamon:
  {
    GST_ERROR_OBJECT (dec, "Unable to start input stream err=%s",
        strerror (errno));
    unmap_input_buf (dec);
    return FALSE;
  }
}

static void
unmap_input_buf (GstV4L2Dec * dec)
{
  if (!dec)
    return;
  if (dec->mmap_virtual_input) {
    for (int i = 0; i < dec->current_nb_buf_input; i++) {
      if (dec->mmap_virtual_input[i]) {
        v4l2_munmap (dec->mmap_virtual_input[i], dec->mmap_size_input[i]);
        dec->mmap_virtual_input[i] = NULL;
      }
    }
    dec->current_nb_buf_input = 0;
    dec->mmap_virtual_input = NULL;
  }
}

#define V4L2DEC_DEFAULT_FRAMERATE 25
/* purpose of this service is to detect if the framerate is known and valid.
 * if invalid, the framerate is set from preferred-caps (if provided) or
 * set to a default value.
 * mainly useful in one case : timestamps are not known (stream played without
 * container) and framerate can't be computed (from SPS in case of H264)
 *
 * Note : framerate value set in preferred-caps is used ONLY when input
 * framerate is invalid or not set.
 */
static GstCaps *
gst_v4l2dec_framerate_fixate_caps (GstV4L2Dec * dec, GstCaps * caps,
    GstCaps * othercaps)
{
  GstStructure *ins, *outs;
  const GValue *infps = NULL;
  const GValue *preffps = NULL;
  guint fps_n = 0;
  guint fps_d = 0;
  guint infps_n = 0;
  guint infps_d = 0;

  othercaps = gst_caps_truncate (othercaps);
  othercaps = gst_caps_make_writable (othercaps);

  ins = gst_caps_get_structure (caps, 0);
  outs = gst_caps_get_structure (othercaps, 0);

  infps = gst_structure_get_value (ins, "framerate");
  if (infps) {
    infps_n = gst_value_get_fraction_numerator (infps);
    infps_d = gst_value_get_fraction_denominator (infps);
  }

  preffps = gst_structure_get_value (outs, "framerate");
  if (preffps) {
    fps_n = gst_value_get_fraction_numerator (preffps);
    fps_d = gst_value_get_fraction_denominator (preffps);
  }

  if (!infps_n) {
    /* if framerate is not detected from input caps or
     * framerate is identified as invalid, in this case,
     * we try to find out this information from
     * preferred_caps property
     * if always not found, fix it to a default value
     */
    GST_DEBUG_OBJECT (dec, "invalid input framerate detected: %d/%d",
        infps_n, infps_d);
    if (!fps_n) {
      /* By default, if nothing is specified framerate is set to
       * V4L2DEC_DEFAULT_FRAMERATE/1
       */
      GST_DEBUG_OBJECT (dec, "framerate fixed to %d/1 by default",
          V4L2DEC_DEFAULT_FRAMERATE);
      fps_n = V4L2DEC_DEFAULT_FRAMERATE;
      fps_d = 1;
    } else
      GST_DEBUG_OBJECT (dec, "framerate updated from preferred caps : %d/%d",
          fps_n, fps_d);
  } else {
    fps_n = infps_n;
    fps_d = infps_d;
  }
  gst_structure_set (outs, "framerate", GST_TYPE_FRACTION, fps_n, fps_d, NULL);
  return othercaps;
}

/* The following function is an adapted copy from gst-plugins-base/gst/videoscale/
 * gstvideoscale.c (file under the terms of the GNU Library General Public
 * License version 2 or later).
 * Main purpose is to fix the width, height and pixel-aspect-ratio capabilities for the
 * scaling transform.
 */
static GstCaps *
gst_v4l2dec_par_fixate_caps (GstV4L2Dec * dec, GstCaps * caps,
    GstCaps * othercaps)
{
  GstStructure *ins, *outs;
  GstStructure *tmp;
  const GValue *from_par = NULL, *to_par = NULL;
  GValue fpar = { 0, }, tpar = {
  0,};
  gint w = 0, h = 0;

  othercaps = gst_caps_truncate (othercaps);
  othercaps = gst_caps_make_writable (othercaps);

  GST_DEBUG_OBJECT (dec, "Trying to fixate othercaps %" GST_PTR_FORMAT
      " based on caps %" GST_PTR_FORMAT, othercaps, caps);

  ins = gst_caps_get_structure (caps, 0);
  outs = gst_caps_get_structure (othercaps, 0);

  from_par = gst_structure_get_value (ins, "pixel-aspect-ratio");
  if (!from_par) {
    g_value_init (&fpar, GST_TYPE_FRACTION);
    gst_value_set_fraction (&fpar, 1, 1);
    from_par = &fpar;
  }
  to_par = gst_structure_get_value (outs, "pixel-aspect-ratio");
  if (!to_par) {
    /* By default, if nothing is specified output PAR is 1/1 */
    g_value_init (&tpar, GST_TYPE_FRACTION);
    gst_value_set_fraction (&tpar, 1, 1);
    to_par = &tpar;
    gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
        NULL);
  }

  /* we have both PAR but they might not be fixated */
  {
    gint from_w, from_h, from_par_n, from_par_d, to_par_n, to_par_d;
    gint from_dar_n, from_dar_d;
    gint num, den;

    /* from_par should be fixed */
    g_return_val_if_fail (gst_value_is_fixed (from_par), dec->preferred_caps);

    from_par_n = gst_value_get_fraction_numerator (from_par);
    from_par_d = gst_value_get_fraction_denominator (from_par);
    to_par_n = gst_value_get_fraction_numerator (to_par);
    to_par_d = gst_value_get_fraction_denominator (to_par);

    GST_INFO_OBJECT (dec,
        "Pixel-aspect-ratio is specified : width and height might be adapted");

    gst_structure_get_int (ins, "width", &from_w);
    gst_structure_get_int (ins, "height", &from_h);

    gst_structure_get_int (outs, "width", &w);
    gst_structure_get_int (outs, "height", &h);

    /* Calculate input DAR */
    if (!gst_util_fraction_multiply (from_w, from_h, from_par_n, from_par_d,
            &from_dar_n, &from_dar_d)) {
      GST_ELEMENT_ERROR (dec, CORE, NEGOTIATION, (NULL),
          ("Error calculating the output scaled size - integer overflow"));
      goto no_update;
    }

    /* if both width and height and PAR are already fixed, we can't do anything
     * except to consider width and height as maximal values */
    if (w && h) {
      gint set_h, set_w, f_h, f_w;

      GST_INFO_OBJECT (dec,
          "Preferred caps are set to %dx%d, PAR is %d/%d, try to adapt keeping PAR fixed",
          w, h, to_par_n, to_par_d);

      /* Try first to keep the required input height */
      tmp = gst_structure_copy (outs);
      /* Consider requested width value as a maximum value */
      gst_structure_set (tmp, "width", GST_TYPE_INT_RANGE, 16, w, NULL);
      gst_structure_set (tmp, "height", G_TYPE_INT, h, NULL);
      set_h = h;

      /* Calculate scale factor for the PAR change */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, to_par_d,
              to_par_n, &num, &den)) {
        GST_ELEMENT_ERROR (dec, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto no_update;
      }

      w = (guint) gst_util_uint64_scale_int (set_h, num, den);
      gst_structure_fixate_field_nearest_int (tmp, "width", w);
      gst_structure_get_int (tmp, "width", &set_w);
      GST_DEBUG_OBJECT (dec, "Updated width should be %d", set_w);
      gst_structure_free (tmp);

      /* We kept the DAR and the height is nearest to the original height */
      if (set_w == w) {
        goto done;
      }

      GST_DEBUG_OBJECT (dec,
          "Updated width is out of range, trying to update height");

      f_h = set_h;
      f_w = set_w;

      /* If the former failed, try to keep the input width at least */
      tmp = gst_structure_copy (outs);
      gst_structure_get_int (outs, "width", &w);
      gst_structure_get_int (outs, "height", &h);
      /* Consider requested height value as a maximum value */
      gst_structure_set (tmp, "height", GST_TYPE_INT_RANGE, 16, h, NULL);
      gst_structure_set (tmp, "width", G_TYPE_INT, w, NULL);

      /* This might have failed but try to scale the width
       * to keep the DAR nonetheless */
      h = (guint) gst_util_uint64_scale_int (w, den, num);
      gst_structure_fixate_field_nearest_int (tmp, "height", h);
      gst_structure_get_int (tmp, "height", &set_h);
      GST_DEBUG_OBJECT (dec, "Updated height should be %d", set_h);
      gst_structure_free (tmp);

      /* We kept the DAR and the width is nearest to the original width */
      if (set_h == h) {
        goto done;
      }

      /* If all this failed, keep the height that was nearest to the orignal
       * height and the nearest possible width. This changes the DAR but
       * there's not much else to do here.
       */
      GST_INFO_OBJECT (dec, "Update preferred caps to WxH [%dx%d]", f_w, f_h);
      w = f_w;
      h = f_h;
      goto done;
    }

    GST_DEBUG_OBJECT (dec, "Input DAR is %d/%d", from_dar_n, from_dar_d);

    /* If either width or height are fixed there's not much we
     * can do either except choosing a height or width and PAR
     * that matches the DAR as good as possible
     */
    if (h) {
      GST_DEBUG_OBJECT (dec, "height is fixed (%d)", h);
      GST_DEBUG_OBJECT (dec, "PAR is fixed %d/%d", to_par_n, to_par_d);

      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, to_par_d,
              to_par_n, &num, &den)) {
        GST_ELEMENT_ERROR (dec, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto no_update;
      }
      w = (guint) gst_util_uint64_scale_int (h, num, den);
      goto done;
    } else if (w) {
      GST_WARNING_OBJECT (dec, "width is fixed (%d)", w);
      GST_WARNING_OBJECT (dec, "PAR is fixed %d/%d", to_par_n, to_par_d);
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, to_par_d,
              to_par_n, &num, &den)) {
        GST_ELEMENT_ERROR (dec, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto no_update;
      }

      h = (guint) gst_util_uint64_scale_int (w, den, num);
      GST_INFO_OBJECT (dec,
          "Update preferred caps to WxH [%dx%d] to keep the good PAR", w, h);
      goto done;
    } else {
      GST_DEBUG_OBJECT (dec, "to_par is fixed");
      /* Calculate scale factor for the PAR change */
      if (!gst_util_fraction_multiply (from_dar_n, from_dar_d, to_par_n,
              to_par_d, &num, &den)) {
        GST_ELEMENT_ERROR (dec, CORE, NEGOTIATION, (NULL),
            ("Error calculating the output scaled size - integer overflow"));
        goto no_update;
      }
      w = (guint) gst_util_uint64_scale_int (from_h, num, den);
      h = from_h;
      goto done;
    }
  }

done:
  GST_INFO_OBJECT (dec,
      "Update preferred caps to WxH [%dx%d] to keep the good PAR", w, h);
  gst_structure_set (outs, "pixel-aspect-ratio", GST_TYPE_FRACTION,
      gst_value_get_fraction_numerator (to_par),
      gst_value_get_fraction_denominator (to_par), "width", G_TYPE_INT, w,
      "height", G_TYPE_INT, h, NULL);
no_update:
  if (from_par == &fpar)
    g_value_unset (&fpar);
  if (to_par == &tpar)
    g_value_unset (&tpar);
  return othercaps;
}

static void
gst_v4l2_output_buffer_timestamp (GstVideoCodecFrame * frame,
    struct timeval *ts)
{
  /* In case of invalid PTS timestamp (resp. DTS), instead of using DTS
   * (resp. PTS), we must forward it to driver. We rely on videodecoder
   * class to properly timestamp the frame buffer if associated timestamp
   * is invalid.
   */
#if defined TIMESTAMP_WITH_PTS_INSTEAD_OF_DTS
  /* access unit's timestamp : PTS based */
  if (GST_CLOCK_TIME_IS_VALID (frame->pts))
    GST_TIME_TO_TIMEVAL (frame->pts, *ts);
#else
  /* access unit's timestamp : DTS based */
  if (GST_CLOCK_TIME_IS_VALID (frame->dts))
    GST_TIME_TO_TIMEVAL (frame->dts, *ts);
#endif
  else {
    /* force to an invalid value
     * purpose is to be able to detect, later, in push_thread service,
     * that we're dealing with an invalid timestamp (to return
     * GST_CLOCK_TIME_NONE to gstvideodecoder)
     * in gstvideodecoder, GST_CLOCK_TIME_NONE value forces it
     * to rebuild timestamps from the framerate.
     *
     * note : unable to reuse GST_CLOCK_TIME_NONE because value is -1 in
     * nanosec whereas V4L2 timestamp resolution is usec.
     */
    ts->tv_sec = -1;
    ts->tv_usec = 0;
  }
}

static GstFlowReturn
gst_v4l2_buffer_poll (GstVideoDecoder * decoder, __u32 buffer_type)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstFlowReturn ret = GST_FLOW_OK;
  GstPoll *poll;
  GstPollFD *poll_fd;

  if (dec->can_poll_device)
    return ret;

  poll = dec->poll[buffer_type].p;
  poll_fd = &(dec->poll[buffer_type].fd);

do_retry:
  ret = gst_poll_wait (poll, GST_CLOCK_TIME_NONE);
  if (G_UNLIKELY (ret < 0)) {
    switch (errno) {
      case EBUSY:
        GST_ERROR_OBJECT (dec, "stop called");
        break;
      case EAGAIN:
      case EINTR:
        goto do_retry;
      case ENXIO:
        GST_WARNING_OBJECT (dec,
            "v4l2 device doesn't support polling. Disabling"
            " using libv4l2 in this case may cause deadlocks");
        dec->can_poll_device = FALSE;
        break;
      default:
        GST_DEBUG_OBJECT (dec, "poll error %d: %s (%d)",
            ret, g_strerror (errno), errno);
    }
  }
  if (gst_poll_fd_has_error (poll, poll_fd)) {
    GST_DEBUG_OBJECT (dec, "poll error %d: %s (%d)",
        ret, g_strerror (errno), errno);
  }
  return GST_FLOW_OK;
}

/* setup output (CAPTURE for V4L2) with the Header if not yet done
 * else : go to decode for each AU available */
static GstFlowReturn
gst_v4l2dec_setup_output (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstFlowReturn ret = GST_FLOW_OK;

  GstBuffer *header = NULL;
  GstMapInfo mapinfo = { 0, };

  GstVideoAlignment align;
  struct v4l2_buffer dqbuf;
  struct v4l2_format g_fmt;
  struct v4l2_format s_fmt;
  GstStructure *s;
  gboolean s_fmt_done = FALSE;
  struct v4l2_crop g_crop;
  struct v4l2_buffer qbuf;
  struct v4l2_requestbuffers reqbufs;
  gint type;
  void *au_data;
  int au_size;
  void *v4l2_data;
  __u32 v4l2_size;
  GstBufferPool *pool;
  __u32 width, height, aligned_width, aligned_height;
  __u32 fmt;
  __u32 size;
  gint par_n = 1;
  gint par_d = 1;
  gint fps_n = 0;
  gint fps_d = 0;

do_retry:
  /* Poll until a buffer is available */
  gst_v4l2_buffer_poll (decoder, V4L2_BUF_TYPE_VIDEO_OUTPUT);
  /* Dequeue a V4L2 buffer where to write */
  memset (&dqbuf, 0, sizeof dqbuf);
  dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  dqbuf.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl (dec->fd, VIDIOC_DQBUF, &dqbuf) < 0)
    goto error_ioc_dqbuf;

  v4l2_data = dec->mmap_virtual_input[dqbuf.index];
  v4l2_size = dqbuf.length;

  /* Copy header in V4L2 buffer */
  if (dec->header)
    header = dec->header;       /* Header derived from codec_data processing */
  else if (dec->codec_data)
    header = dec->codec_data;   /* Header is codec_data */
  else
    header = frame->input_buffer;       /* Header is within input gst buffer */

  gst_buffer_map (header, &mapinfo, GST_MAP_READ);
  au_data = mapinfo.data;
  au_size = mapinfo.size;
  if (au_size > v4l2_size) {
    gst_buffer_unmap (header, &mapinfo);
    goto error_au_size;
  }
  memcpy (v4l2_data, au_data, au_size);
  gst_buffer_unmap (header, &mapinfo);

  /* Queue V4L2 buffer */
  qbuf = dqbuf;                 /* index from dqbuf */
  qbuf.bytesused = au_size;
  gst_v4l2_output_buffer_timestamp (frame, &qbuf.timestamp);
  if (v4l2_ioctl (dec->fd, VIDIOC_QBUF, &qbuf) < 0)
    goto error_ioc_qbuf;

do_g_fmt:
  /* Get output frame format from V4L2 (read from header) */
  memset (&g_fmt, 0, sizeof g_fmt);
  g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = v4l2_ioctl (dec->fd, VIDIOC_G_FMT, &g_fmt);
  if (ret != 0) {
    /* Header not yet found */
    if (dec->header || dec->codec_data) {
      /* unable to detect the stream format using codec_data
       * let's try now with the first AU instead
       * to do so, reset dec->header and dec->codec_data
       * and restart sending of data.
       */
      if (dec->header) {
        gst_buffer_unref (dec->header);
        dec->header = NULL;
      }
      if (dec->codec_data)
        dec->codec_data = NULL;
      goto do_retry;
    }
    goto done;
  }
  GST_DEBUG_OBJECT (dec,
      "Format found from V4L2 :fmt:%s, width:%d, height:%d, bytesperline:%d, sizeimage:%d, pixelfmt:%s, field:%d, colorspace:%d",
      v4l2_type_str (g_fmt.type), g_fmt.fmt.pix.width,
      g_fmt.fmt.pix.height, g_fmt.fmt.pix.bytesperline,
      g_fmt.fmt.pix.sizeimage, v4l2_fmt_str (g_fmt.fmt.pix.pixelformat),
      g_fmt.fmt.pix.field, g_fmt.fmt.pix.colorspace);

  aligned_width = g_fmt.fmt.pix.width;
  aligned_height = g_fmt.fmt.pix.height;
  fmt = g_fmt.fmt.pix.pixelformat;
  size = g_fmt.fmt.pix.sizeimage;

  /* Compute padding */
  gst_video_alignment_reset (&align);
  memset (&g_crop, 0, sizeof g_crop);
  g_crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl (dec->fd, VIDIOC_G_CROP, &g_crop) < 0) {
    GST_DEBUG_OBJECT (dec, "Not able to get crop, default to %dx%d",
        aligned_width, aligned_height);
    width = aligned_width;
    height = aligned_height;
  } else {
    GST_DEBUG_OBJECT (dec,
        "Crop found from V4L2 :fmt:%s, left:%d, top:%d, width:%d, height:%d",
        v4l2_type_str (g_crop.type), g_crop.c.left, g_crop.c.top,
        g_crop.c.width, g_crop.c.height);
    width = g_crop.c.width;
    height = g_crop.c.height;

    align.padding_left = g_crop.c.left;
    align.padding_top = g_crop.c.top;
    align.padding_right = aligned_width - align.padding_left - width;
    align.padding_bottom = aligned_height - align.padding_top - height;

    GST_DEBUG_OBJECT (dec,
        "Padding information deduced from V4L2 G_FMT/G_CROP: padding_left:%d, padding_right:%d, "
        "padding_top:%d, padding_bottom:%d",
        align.padding_left, align.padding_right,
        align.padding_top, align.padding_bottom);
  }

  /* Check preferred output caps property; if set, do V4L2 S_FMT */
  if ((!s_fmt_done) && dec->preferred_caps &&
      (s = gst_caps_get_structure (dec->preferred_caps, 0))) {
    gint retval = 0;
    gint preferred_width, preferred_height;
    GstCaps *preferred_caps = gst_caps_ref (dec->preferred_caps);
    const GValue *par;
    const GValue *fps;

    /* In case width and height are forced by user, we do not
     * take into account the PAR so we keep these values
     * In other cases, we have to fix :
     * width, height or both.
     */
    if (!(gst_structure_get_int (s, "width", &preferred_width) &&
            gst_structure_get_int (s, "height", &preferred_height))) {
      preferred_caps =
          gst_v4l2dec_par_fixate_caps (dec, dec->input_state->caps,
          preferred_caps);
    }
    s = gst_caps_get_structure (preferred_caps, 0);

    par = gst_structure_get_value (s, "pixel-aspect-ratio");
    if (par) {
      par_n = gst_value_get_fraction_numerator (par);
      par_d = gst_value_get_fraction_denominator (par);
    }

    preferred_caps =
        gst_v4l2dec_framerate_fixate_caps (dec, dec->input_state->caps,
        preferred_caps);
    s = gst_caps_get_structure (preferred_caps, 0);

    fps = gst_structure_get_value (s, "framerate");
    if (fps) {
      fps_n = gst_value_get_fraction_numerator (fps);
      fps_d = gst_value_get_fraction_denominator (fps);
    }

    fmt = to_v4l2_pixelformat (s);
    retval = gst_structure_get_int (s, "width", &preferred_width);
    if (retval && (preferred_width > 0)) {
      width = preferred_width;
    }
    retval = gst_structure_get_int (s, "height", &preferred_height);
    if (retval && (preferred_height > 0)) {
      height = preferred_height;
    }

    /* Set V4L2 requested output resolution & format */
    memset (&s_fmt, 0, sizeof s_fmt);
    s_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    s_fmt.fmt.pix.width = width;
    s_fmt.fmt.pix.height = height;
    s_fmt.fmt.pix.pixelformat = fmt;
    s_fmt.fmt.pix.colorspace = g_fmt.fmt.pix.colorspace;
    ret = v4l2_ioctl (dec->fd, VIDIOC_S_FMT, &s_fmt);
    if (ret) {
      GST_DEBUG_OBJECT (dec,
          "Not able to set V4L2 CAPTURE format, default to fmt:%s, width:%d, height:%d",
          v4l2_fmt_str (fmt), width, height);
    } else {
      GST_DEBUG_OBJECT (dec,
          "Prefered output caps is set, V4L2 CAPTURE format set to fmt:%s, width:%d, height:%d",
          v4l2_fmt_str (fmt), width, height);
      s_fmt_done = TRUE;
      /* Redo V4L2 G_FMT/G_CROP to get back what V4L2 can really do */
      goto do_g_fmt;
    }
    gst_caps_unref (preferred_caps);
  }

  dec->output_state =
      gst_video_decoder_set_output_state (GST_VIDEO_DECODER (dec),
      to_gst_pixelformat (fmt), width, height, dec->input_state);

  /* Update Pixel-aspect-ratio */
  GST_DEBUG_OBJECT (dec, "Par updated : %d/%d", par_n, par_d);
  dec->output_state->info.par_n = par_n;
  dec->output_state->info.par_d = par_d;

  /* update Framerate info */
  if (fps_n && fps_d) {
    GST_DEBUG_OBJECT (dec, "Framerate updated : %d/%d", fps_n, fps_d);
    dec->output_state->info.fps_n = fps_n;
    dec->output_state->info.fps_d = fps_d;
  }

  switch (g_fmt.fmt.pix.field) {
    case V4L2_FIELD_NONE:
      dec->output_state->info.interlace_mode =
          GST_VIDEO_INTERLACE_MODE_PROGRESSIVE;
      break;
    case V4L2_FIELD_INTERLACED:
    case V4L2_FIELD_INTERLACED_TB:
    case V4L2_FIELD_INTERLACED_BT:
      dec->output_state->info.interlace_mode =
          GST_VIDEO_INTERLACE_MODE_INTERLEAVED;
      break;
    default:
      /* keep the interlace mode as specified by the CAPS */
      break;
  }

  if (g_fmt.fmt.pix.colorspace == V4L2_COLORSPACE_JPEG) {
    /* By default, we do not modify the colorimetry infos.
     * This code deals with a specific case : H264 video stream
     * coded in full range. Video decoder updates V4L2 colorspace
     * when full range is detected.
     * Following colorimetry values come from gstreamer-1.9.1 :
     * gst_v4l2_object_get_colorspace function in
     * [...]/gst-plugins-good/sys/v4l2/gstv4l2object.c
     */
    dec->output_state->info.colorimetry.range = GST_VIDEO_COLOR_RANGE_0_255;
    dec->output_state->info.colorimetry.matrix = GST_VIDEO_COLOR_MATRIX_BT601;
    dec->output_state->info.colorimetry.transfer = GST_VIDEO_TRANSFER_SRGB;
    dec->output_state->info.colorimetry.primaries =
        GST_VIDEO_COLOR_PRIMARIES_BT709;
    GST_DEBUG_OBJECT (dec, "Colorspace updated to full range 'colorimetry=%s'",
        gst_video_colorimetry_to_string (&(dec->output_state->
                info.colorimetry)));
  }

  /* Set buffer alignment */
  gst_video_info_align (&dec->output_state->info, &align);
  dec->align = align;

  /* Set buffer size */
  dec->size_image = size;

  gst_video_decoder_negotiate (decoder);        /* will call gst_v4l2dec_decide_allocation */

  pool = dec->downstream_pool;

  if (!pool) {
    /* Request output buffers needed (MMAP mode) */
    memset (&reqbufs, 0, sizeof reqbufs);
    reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbufs.count = NB_BUF_OUTPUT;
    reqbufs.memory = V4L2_MEMORY_MMAP;

    if (v4l2_ioctl (dec->fd, VIDIOC_REQBUFS, &reqbufs) < 0)
      goto error_ioc_reqbufs;

    /* construct a new buffer pool */
    dec->pool = gst_v4l2dec_buffer_pool_new (dec, NULL, reqbufs.count,  /* driver nb of buffers needed */
        g_fmt.fmt.pix.sizeimage, &align);
    if (dec->pool == NULL)
      goto error_new_pool;

    /* activate the pool: the buffers are allocated */
    if (gst_buffer_pool_set_active (dec->pool, TRUE) == FALSE)
      goto error_activate_pool;

  } else {
    GstStructure *config = gst_buffer_pool_get_config (pool);
    guint size, min_buffers, max_buffers;
    struct v4l2_buffer qbuf;
    GstV4L2DecDownstreamMeta *meta = NULL;
    GstCaps *caps;
    gint i;

    gst_buffer_pool_config_get_params (config, &caps, &size, &min_buffers,
        &max_buffers);
    gst_structure_free (config);

    GST_DEBUG_OBJECT (dec, "Use downstream pool %p min %d max %d", pool,
        min_buffers, max_buffers);

    for (i = 0; i < ARRAY_SIZE (dec->downstream_buffers); i++)
      dec->downstream_buffers[i] = NULL;

    /* Queue output buffers in order that decoder driver
     * is ready to decode up to dpb frames */
    for (i = 0; i < min_buffers; i++) {
      GstBuffer *buffer;
      GstMemory *gmem = NULL;
      GstBufferPoolAcquireParams params = {.flags =
            GST_BUFFER_POOL_ACQUIRE_FLAG_DONTWAIT
      };
      gint fd = -1;

      ret = gst_buffer_pool_acquire_buffer (pool, &buffer, &params);
      if (ret == GST_FLOW_EOS) {        /* No more buffers available (max reached) */
        GST_LOG_OBJECT (dec, "%d buffers available over %d buffers requested",
            i, min_buffers);
        /* FIXME, this situation occurs if the sink (e.g. encoder) reserves some buffers
         * for its internal need */
        break;
      }
      /* In case of buffer pool allocation error, return is FLOW_FLUSHING, we
         go out on error to stop the graph */
      if (ret == GST_FLOW_FLUSHING)
        goto error_activate_pool;

      if (ret != GST_FLOW_OK)   /* logically caused by STREAMOFF */
        goto out;

      dec->downstream_buffers[i] = buffer;
      meta = GST_V4L2DEC_DOWNSTREAM_META_ADD (buffer);
      if (meta) {
        GST_META_FLAG_SET (meta, GST_META_FLAG_POOLED);
        GST_META_FLAG_SET (meta, GST_META_FLAG_LOCKED);
        meta->acquired = TRUE;
        GST_LOG_OBJECT (dec,
            "set acquire TRUE: dmabuf fd %d, buffer %p, index %d", fd, buffer,
            i);
      }

      gmem = gst_buffer_get_memory (buffer, 0);
      if ((gmem != NULL) && (gst_is_dmabuf_memory (gmem) == TRUE)) {
        fd = gst_dmabuf_memory_get_fd (gmem);
        GST_INFO_OBJECT (dec,
            "Setup output buffer: dmabuf fd %d, buffer %p, index %d", fd,
            buffer, i);
      }
      gst_memory_unref (gmem);

      GST_DEBUG_OBJECT (dec, "Queue buffer (capture type)");
      memset (&qbuf, 0, sizeof qbuf);
      qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      qbuf.memory = V4L2_MEMORY_DMABUF;
      qbuf.index = i;
      qbuf.m.fd = fd;
      qbuf.bytesused = dec->size_image;
      qbuf.length = dec->size_image;
      if (v4l2_ioctl (dec->fd, VIDIOC_QBUF, &qbuf) < 0)
        goto queue_failed;
    }
  }

  /* Start streaming on output */
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl (dec->fd, VIDIOC_STREAMON, &type) < 0)
    goto error_ioc_streamon;
  dec->output_streaming = TRUE;

  /* Everything is ready, start the frame push thread */
  dec->eos_driver = FALSE;
  gst_task_start (dec->frame_push_task);

  /* Start the frame recycle thread */
  if (dec->downstream_pool)
    gst_task_start (dec->frame_recycle_task);

  dec->output_setup = TRUE;

done:
  return GST_FLOW_OK;

  /* Errors */
error_ioc_dqbuf:
  {
    GST_ERROR_OBJECT (dec, "Dequeuing failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
error_au_size:
  {
    GST_ERROR_OBJECT (dec, "Input size too large (%d > %d)", au_size,
        v4l2_size);
    return GST_FLOW_ERROR;
  }
error_ioc_qbuf:
  {
    GST_ERROR_OBJECT (dec, "Enqueuing failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
error_ioc_reqbufs:
  {
    GST_ERROR_OBJECT (dec, "Unable to request buffers err=%s",
        strerror (errno));
    return GST_FLOW_ERROR;
  }
error_new_pool:
  {
    GST_ERROR_OBJECT (dec, "Unable to construct a new buffer pool");
    return GST_FLOW_ERROR;
  }
error_activate_pool:
  {
    GST_ERROR_OBJECT (dec, "Unable to activate the pool");
    gst_object_unref (dec->pool);
    return GST_FLOW_ERROR;
  }
error_ioc_streamon:
  {
    GST_ERROR_OBJECT (dec, "Streamon failed err=%s", strerror (errno));
    gst_buffer_pool_set_active (GST_BUFFER_POOL_CAST (dec->pool), FALSE);
    gst_object_unref (dec->pool);
    return GST_FLOW_ERROR;
  }
queue_failed:
  {
    GST_ERROR_OBJECT (dec, "Queuing input failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }

out:
  return GST_FLOW_OK;
}


/* setup output if not yet done
 * else : go to decode for each AU available */
static GstFlowReturn
gst_v4l2dec_handle_frame (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstFlowReturn ret = GST_FLOW_OK;

  if (!dec->input_setup)
    return GST_FLOW_OK;

  /* Setup output if not yet done */
  if (!dec->output_setup) {
    ret = gst_v4l2dec_setup_output (decoder, frame);
    if (ret)
      return ret;
  }

  /* For every frame, decode */
  ret = gst_v4l2dec_decode (decoder, frame);

  return ret;
}

static gboolean
gst_v4l2_video_dec_flush (GstVideoDecoder * decoder)
{
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  struct v4l2_buffer qbuf;
  GstBufferPool *pool;
  guint type;
  guint i;

  GST_DEBUG_OBJECT (dec, "Flushing");

  if (dec->input_streaming) {
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (dec, "Unable to stop stream on OUTPUT err=%s",
          strerror (errno));
    dec->input_streaming = FALSE;
  }

  if (dec->output_streaming) {
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (dec, "Unable to stop stream on CAPTURE err=%s",
          strerror (errno));
    dec->output_streaming = FALSE;
  }

  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  if (dec->downstream_pool)
    pool = dec->downstream_pool;
  else
    pool = dec->pool;

  /* start pool flushing */
  gst_buffer_pool_set_flushing (pool, TRUE);

  /* release already queued buffer back to downstream pool */
  if (dec->downstream_pool)
    gst_v4l2dec_release_pending_buffers (decoder);

  /* wait for all running threads termination */
  if (gst_task_get_state (dec->frame_push_task) != GST_TASK_STOPPED) {
    GST_DEBUG_OBJECT (dec, "task %p should be stopped by now",
        dec->frame_push_task);
    gst_task_stop (dec->frame_push_task);
    g_rec_mutex_lock (&dec->frame_push_task_mutex);
    g_rec_mutex_unlock (&dec->frame_push_task_mutex);
    gst_task_join (dec->frame_push_task);
  }
  GST_DEBUG_OBJECT (dec, "Push task stopped");

  if (gst_task_get_state (dec->frame_recycle_task) != GST_TASK_STOPPED) {
    GST_DEBUG_OBJECT (dec, "task %p should be stopped by now",
        dec->frame_recycle_task);
    gst_task_stop (dec->frame_recycle_task);
    g_rec_mutex_lock (&dec->frame_recycle_task_mutex);
    g_rec_mutex_unlock (&dec->frame_recycle_task_mutex);
    gst_task_join (dec->frame_recycle_task);
  }
  GST_DEBUG_OBJECT (dec, "Recycle task stopped");

  /* stop pool flushing */
  gst_buffer_pool_set_flushing (pool, FALSE);

  /* requeue OUTPUT buffers now and restart */
  for (i = 0; i < dec->current_nb_buf_input; i++) {
    qbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    qbuf.memory = V4L2_MEMORY_MMAP;
    qbuf.index = i;
    qbuf.bytesused = 0;         /* enqueue it with no data */

    if (v4l2_ioctl (dec->fd, VIDIOC_QBUF, &qbuf) < 0)
      GST_WARNING_OBJECT (dec, "QBUF OUTPUT failed (err=%s)", strerror (errno));
  }

  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if (v4l2_ioctl (dec->fd, VIDIOC_STREAMON, &type) < 0)
    GST_WARNING_OBJECT (dec, "STREAMON OUTPUT failed (err=%s)",
        strerror (errno));
  dec->input_streaming = TRUE;

  /* restart capture */
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl (dec->fd, VIDIOC_STREAMON, &type) < 0)
    GST_WARNING_OBJECT (dec, "STREAMON CAPTURE failed (err=%s)",
        strerror (errno));
  dec->output_streaming = TRUE;

  if (gst_task_get_state (dec->frame_push_task) != GST_TASK_STARTED) {
    dec->eos_driver = FALSE;
    gst_task_start (dec->frame_push_task);
  }

  if (dec->downstream_pool)
    if (gst_task_get_state (dec->frame_recycle_task) != GST_TASK_STARTED)
      gst_task_start (dec->frame_recycle_task);

  GST_DEBUG_OBJECT (dec, "Flushed");

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  return TRUE;
}

/* Here we push AU to V4L2, header found so */
static GstFlowReturn
gst_v4l2dec_decode (GstVideoDecoder * decoder, GstVideoCodecFrame * frame)
{
  GstFlowReturn ret = GST_FLOW_OK;
  GstMapInfo mapinfo = { 0, };
  GstBuffer *buf = NULL;
  guint8 *gstdata;
  gsize gstsize;
  struct v4l2_buffer dqbuf;
  struct v4l2_buffer qbuf;
  gboolean is_intra_frame = FALSE;
  gboolean skip_frame = FALSE;
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);

  /* QOS service : check if we have to drop some frame before trying
   * to decode it to decrease VPU usage
   */
  is_intra_frame =
      !((GST_BUFFER_FLAGS (frame->input_buffer) & GST_BUFFER_FLAG_DELTA_UNIT) ==
      GST_BUFFER_FLAG_DELTA_UNIT);
  /* if qos service reports it we skip the frame EXCEPT if this is a intra frame */
  skip_frame = gst_v4l2dec_qos_check_skip_frame (decoder, is_intra_frame);
  if (skip_frame && dec->skip_frame) {
    if (!is_intra_frame) {
      GST_DEBUG_OBJECT (dec, "QOS : skip frame");
      return GST_FLOW_OK;
    } else {
      GST_DEBUG_OBJECT (dec, "QOS : systematically send KeyFrame");
    }
  }
  /* Silently return if not streaming */
  if (!dec->input_streaming)
    return GST_FLOW_OK;

  /* Poll until a buffer is available */
  gst_v4l2_buffer_poll (decoder, V4L2_BUF_TYPE_VIDEO_OUTPUT);
  /* pop an empty buffer */
  memset (&dqbuf, 0, sizeof dqbuf);
  dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  dqbuf.memory = V4L2_MEMORY_MMAP;
  if (v4l2_ioctl (dec->fd, VIDIOC_DQBUF, &dqbuf) < 0)
    goto error_ioctl_dequeue_out;

  /* copy GST -> V4L2 */
  buf = frame->input_buffer;

  gst_buffer_map (buf, &mapinfo, GST_MAP_READ);
  gstdata = mapinfo.data;
  gstsize = mapinfo.size;

  if (gstsize > dec->mmap_size_input[dqbuf.index]) {
    GST_ERROR_OBJECT (dec,
        "Size exceed, dest (%d) smaller than source (%" G_GSIZE_FORMAT ")\n",
        dec->mmap_size_input[dqbuf.index], gstsize);
    gst_buffer_unmap (buf, &mapinfo);
    return GST_FLOW_ERROR;
  } else {
    memcpy (dec->mmap_virtual_input[dqbuf.index], gstdata, gstsize);
  }

  gst_buffer_unmap (buf, &mapinfo);

  /* Unlock decoder before qbuf call:
   * qbuf will eventually block till frames
   * recycled, so frame_push_thread must
   * execute lock free..
   */
  GST_VIDEO_DECODER_STREAM_UNLOCK (decoder);

  /* push AU */
  qbuf = dqbuf;                 /* index from dqbuf */
  qbuf.bytesused = gstsize;     /* access unit size */
  gst_v4l2_output_buffer_timestamp (frame, &qbuf.timestamp);
  if (v4l2_ioctl (dec->fd, VIDIOC_QBUF, &qbuf) < 0) {
    GST_VIDEO_DECODER_STREAM_LOCK (decoder);
    goto error_ioctl_enqueue;
  }

  GST_VIDEO_DECODER_STREAM_LOCK (decoder);
  if (qbuf.flags & V4L2_BUF_FLAG_ERROR)
    /* either the decoded frame is corrupted (decoding error)
     * or it should not be displayed (e.g. "invisible" VP8 frame) */
    GST_DEBUG_OBJECT (dec,
        "! Invisible VP8 frame or non-blocking decoding error");

  return ret;

  /* ERRORS */
error_ioctl_dequeue_out:
  {
    GST_ERROR_OBJECT (dec, "Dequeuing input failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
error_ioctl_enqueue:
  {
    GST_ERROR_OBJECT (dec, "Enqueuing failed err=%s", strerror (errno));
    return GST_FLOW_ERROR;
  }
}

/* The thread is in charge of recycle the displayed frame to V4L2 */
static void
frame_recycle_thread (void *arg)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) arg;
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstMemory *gmem = NULL;
  GstBufferPoolAcquireParams params = {.flags =
        GST_BUFFER_POOL_ACQUIRE_FLAG_NONE
  };
  GstV4L2DecDownstreamMeta *meta = NULL;
  gint fd = -1;
  gboolean found = FALSE;
  struct v4l2_buffer qbuf;
  GstBuffer *buffer;
  int i;
  int ret;
  gint type;

  ret = gst_buffer_pool_acquire_buffer (dec->downstream_pool, &buffer, &params);
  if (ret != GST_FLOW_OK) {     /* logically caused by STREAMOFF */
    GST_LOG_OBJECT (dec, "The pool is no more active %d", ret);
    goto out;
  }

  found = FALSE;
  for (i = 0; i < ARRAY_SIZE (dec->downstream_buffers); i++) {
    if (dec->downstream_buffers[i] == buffer) {
      found = TRUE;
      break;
    }
  }

  if (!found) {
    GST_LOG_OBJECT (dec, "Unexpected buffer from acquire_buffer, buffer %p",
        buffer);
    goto exit;                  /* Additional buffers are allocated, only focus on ours. FIXME: Is unref needed ? */
  }

  meta = GST_V4L2DEC_DOWNSTREAM_META_GET (buffer);
  if (meta) {
    meta->acquired = TRUE;
    GST_LOG_OBJECT (dec, "set acquire TRUE: dmabuf fd %d, buffer %p, index %d",
        fd, buffer, i);
  }

  gmem = gst_buffer_get_memory (buffer, 0);
  if ((gmem != NULL) && (gst_is_dmabuf_memory (gmem) == TRUE)) {
    fd = gst_dmabuf_memory_get_fd (gmem);
    GST_LOG_OBJECT (dec,
        "Recycle output buffer: dmabuf fd %d, buffer %p, index %d", fd, buffer,
        i);
  }
  gst_memory_unref (gmem);

  GST_LOG_OBJECT (dec, "Recycle buffer index %d", i);
  memset (&qbuf, 0, sizeof qbuf);
  qbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  qbuf.memory = V4L2_MEMORY_DMABUF;
  qbuf.index = i;
  qbuf.m.fd = fd;
  qbuf.bytesused = dec->size_image;
  qbuf.length = dec->size_image;

  GST_LOG_OBJECT (dec, "Queue output buffer %p, index %d", buffer, i);
  if (v4l2_ioctl (dec->fd, VIDIOC_QBUF, &qbuf) < 0) {
    GST_ERROR_OBJECT (dec, "Queuing output buffer failed err=%s",
        strerror (errno));
  }
exit:
  return;

out:
  GST_DEBUG_OBJECT (dec, "-->Leaving the frame recycle thread!");

  if (dec->output_streaming) {
    /* Force STREAMOFF on CAPTURE side to
     * unblock potentially blocked QBUF(OUTPUT) call
     */
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (dec, "Unable to stop stream on CAPTURE err=%s",
          strerror (errno));
    dec->output_streaming = FALSE;
  }

  if (dec->input_streaming) {
    /* Force STREAMOFF on OUTPUT side to
     * discard any further QBUF(OUTPUT) call
     */
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (dec, "Unable to stop stream on OUTPUT err=%s",
          strerror (errno));
    dec->input_streaming = FALSE;
  }

  gst_task_pause (dec->frame_recycle_task);
  return;
}

/* The thread is in charge of retrieve the decoded frame from V4L2
 * and push it to the next pad */
static void
frame_push_thread (void *arg)
{
  GstVideoDecoder *decoder = (GstVideoDecoder *) arg;
  GstV4L2Dec *dec = GST_V4L2DEC (decoder);
  GstVideoInfo *info = &dec->output_state->info;
  GstFlowReturn ret = GST_FLOW_OK;
  GstVideoCodecFrame *frame;
  GstBuffer *output_buffer = NULL;
  GstVideoFrameFlags flags = 0;
  gint fd = -1;
  gint index = 0;
  gint type;

  if (dec->pool) {
    ret = gst_buffer_pool_acquire_buffer (dec->pool, &output_buffer, NULL);
    if (ret == GST_FLOW_EOS)
      goto eos;
    else if (ret != GST_FLOW_OK)        /* logically caused by STREAMOFF */
      goto out;
  } else if (dec->downstream_pool) {
    struct v4l2_buffer dqbuf;
    GstV4L2DecDownstreamMeta *meta = NULL;
    GST_DEBUG_OBJECT (dec, "trying to dequeue frame");

    /* Poll until a buffer is available */
    gst_v4l2_buffer_poll (decoder, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    memset (&dqbuf, 0, sizeof dqbuf);
    dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    dqbuf.memory = V4L2_MEMORY_DMABUF;
    GST_DEBUG_OBJECT (dec, "dequeue buffer (capture type)");
    if (v4l2_ioctl (dec->fd, VIDIOC_DQBUF, &dqbuf) < 0) {
      if (errno == EPIPE)
        goto eos;               /* EOS */
      else
        goto out;               /* STREAMOFF */
    }

    if ((dqbuf.flags & V4L2_BUF_FLAG_LAST) && dqbuf.bytesused == 0)
      goto eos;                 /* last empty buffer dqueued */

    GST_DEBUG_OBJECT (dec, "dequeue buffer (capture type) dqbuf.index %d",
        dqbuf.index);

    /* get back GstBuffer from V4L2 index */
    output_buffer = dec->downstream_buffers[dqbuf.index];
    fd = dqbuf.m.fd;
    index = dqbuf.index;

    meta = GST_V4L2DEC_DOWNSTREAM_META_GET (output_buffer);
    if (meta) {
      meta->acquired = FALSE;
      GST_LOG_OBJECT (dec,
          "set acquire FALSE: dmabuf fd %d, buffer %p, index %d", fd,
          output_buffer, index);
    }

    if (dqbuf.timestamp.tv_sec == -1 && dqbuf.timestamp.tv_usec == 0) {
      /* Invalid timestamp, force GST_CLOCK_TIME_NONE */
      GST_BUFFER_TIMESTAMP (output_buffer) = GST_CLOCK_TIME_NONE;
    } else {
      GST_BUFFER_TIMESTAMP (output_buffer) =
          GST_TIMEVAL_TO_TIME (dqbuf.timestamp);
    }

    /* set top/bottom field first if v4l2_buffer has the information */
    if ((dqbuf.field == V4L2_FIELD_INTERLACED_TB)
        || (dqbuf.field == V4L2_FIELD_INTERLACED)) {
      GST_BUFFER_FLAG_SET (output_buffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_SET (output_buffer, GST_VIDEO_BUFFER_FLAG_TFF);
    } else if (dqbuf.field == V4L2_FIELD_INTERLACED_BT) {
      GST_BUFFER_FLAG_SET (output_buffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_UNSET (output_buffer, GST_VIDEO_BUFFER_FLAG_TFF);
    } else {
      /* per default, the frame is considered as progressive */
      GST_BUFFER_FLAG_UNSET (output_buffer, GST_VIDEO_BUFFER_FLAG_INTERLACED);
      GST_BUFFER_FLAG_UNSET (output_buffer, GST_VIDEO_BUFFER_FLAG_TFF);
    }
  }

  frame = gst_video_decoder_get_oldest_frame (decoder);
  if (frame == NULL) {
    if (dec->pool)
      gst_buffer_pool_release_buffer (dec->pool, output_buffer);
    else if (dec->downstream_pool)
      gst_buffer_pool_release_buffer (dec->downstream_pool, output_buffer);
    goto exit;
  }

  /* buffer flags enhance the meta flags */
  if (GST_BUFFER_FLAG_IS_SET (output_buffer, GST_VIDEO_BUFFER_FLAG_INTERLACED))
    flags |= GST_VIDEO_FRAME_FLAG_INTERLACED;
  if (GST_BUFFER_FLAG_IS_SET (output_buffer, GST_VIDEO_BUFFER_FLAG_TFF))
    flags |= GST_VIDEO_FRAME_FLAG_TFF;

  /* Add alignment info */
  gst_buffer_add_video_meta_full (output_buffer,
      flags,
      GST_VIDEO_INFO_FORMAT (info),
      GST_VIDEO_INFO_WIDTH (info),
      GST_VIDEO_INFO_HEIGHT (info),
      GST_VIDEO_INFO_N_PLANES (info), info->offset, info->stride);

  frame->output_buffer = output_buffer;
  frame->pts = output_buffer->pts;

  /* Decrease the refcount of the frame so that the frame is released by the
   * gst_video_decoder_finish_frame function and so that the output buffer is
   * writable when it's pushed downstream */
  gst_video_codec_frame_unref (frame);

  ret = gst_video_decoder_finish_frame (decoder, frame);
  if (ret != GST_FLOW_OK)
    goto out;
  GST_DEBUG_OBJECT (dec, "-->Frame pushed buffer %p", output_buffer);

exit:
  return;

eos:
  GST_DEBUG_OBJECT (dec, "EOS reached");
  goto eos_out;
out:
  GST_DEBUG_OBJECT (dec, "Stop streaming on CAPTURE and OUTPUT");
  if (dec->output_streaming) {
    /* Force STREAMOFF on CAPTURE side to
     * unblock potentially blocked QBUF(CAPTURE) call
     */
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (dec, "Unable to stop stream on CAPTURE err=%s",
          strerror (errno));
    dec->output_streaming = FALSE;
  }

  if (dec->input_streaming) {
    /* Force STREAMOFF on OUTPUT side to
     * discard any further QBUF(OUTPUT) call
     */
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (v4l2_ioctl (dec->fd, VIDIOC_STREAMOFF, &type) < 0)
      GST_WARNING_OBJECT (dec, "Unable to stop stream on OUTPUT err=%s",
          strerror (errno));
    dec->input_streaming = FALSE;
  }

eos_out:
  /* Raise EOS in all cases when leaving frame push thread
   * in order to not deadlock gst_v4l2dec_finish()... */
  GST_DEBUG_OBJECT (dec, "-->Leaving the frame push thread!");
  g_mutex_lock (&dec->eos_mutex);
  dec->eos_driver = TRUE;
  g_cond_signal (&dec->eos_cond);
  g_mutex_unlock (&dec->eos_mutex);

  gst_task_pause (dec->frame_push_task);
  return;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    v4l2dec,
    "V4L2 decoder",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN);
