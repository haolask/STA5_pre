/*
* This file is part of VideoCRC
*
* Copyright (C) 2015-2016, STMicroelectronics - All Rights Reserved
* Author: Rajesh Sharma <rajesh-dcg.sharma@st.com> for STMicroelectronics.
*
* License terms: LGPL V2.1.
*
* VideoCRC is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License version 2.1 as
* published by the Free Software Foundation.
*
* VideoCRC is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
* for more details.
*
* You should have received a copy of the GNU Lesser General Public License along
* with this library. If not, see <http://www.gnu.org/licenses/>.
*
*/

/*
 * VideoCRC video checksumming element
 * CRC algo is implemented based on standard CRC computation algorithm
 * available on internet in public domain.
 */

/**
 * SECTION:element-videocrc
 * @short_desc: computes 32 bit CRC (Luma & Chroma) for every video frame
 *
 * This element accepts selected YUV planar formats NV12, I420 and YV12.
 * The element computes 32 bit CRC of luma and chroma for every video frame which can be used
 * to validate the decoded video frames to a exisiting database of pre-computed CRC files.
 * CRC is sent as messsage , so these messages should be processed in application
 * to generate the checksum file to compare with reference CRC files.
 * CRC message to application can be switched off by setting crc-message property to FALSE.
 * The default polynomial used is 0X04c11db7U but it can be changed before processing using crc-mask property.
 * CRC values can also be printed on terminal using --gst-debug=videocrc:4
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -m uridecodebin uri=file:///path/to/video.mp4 ! videocrc ! fakesink
 * gst-launch-1.0 -m filesrc blocksize=622080 location=file.nv12 ! "video/x-raw , width=720 , height=576 , framerate=(fraction)1/1 , format=NV12" ! videocrc ! fakesink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gstvideocrc.h"

#define VIDEO_CAPS \
    GST_VIDEO_CAPS_MAKE("{ NV12, I420, YV12}")

GST_DEBUG_CATEGORY_STATIC (gst_videocrc_debug);
#define GST_CAT_DEFAULT gst_videocrc_debug

enum
{
  PROP_0,
  PROP_CRC_MESSAGE,
  PROP_CRC_MASK
};

#define parent_class gst_videocrc_parent_class
G_DEFINE_TYPE (GstVideocrc, gst_videocrc, GST_TYPE_VIDEO_FILTER);

static void gst_videocrc_finalize (GObject * object);
static void gst_videocrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_videocrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static GstFlowReturn gst_videocrc_transform_frame_ip (GstVideoFilter *
    filter, GstVideoFrame * frame);
static void gst_videocrc_post_message (GstVideocrc * videocrc);
static void gst_videocrc_crc_progressive (GstVideocrc * videocrc,
    GstVideoFrame * frame, gboolean isNV12_format);
static void gst_videocrc_crc_interlaced (GstVideocrc * videocrc,
    GstVideoFrame * frame, gboolean isNV12_format, gboolean top_field);
static void gst_videocrc_init_crc32bit_table (GstVideocrc * videocrc);

#define GST_VIDEO_DEFAULT_CRC_MASK 0X04c11db7U
#define GST_VIDEO_DEFAULT_CRC_MESSAGE TRUE

static void
gst_videocrc_class_init (GstVideocrcClass * klass)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *gstbasetrans_class;
  GstVideoFilterClass *videofilter_class = GST_VIDEO_FILTER_CLASS (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gstbasetrans_class = GST_BASE_TRANSFORM_CLASS (klass);

  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_CAPS)));

  gobject_class->set_property = gst_videocrc_set_property;
  gobject_class->get_property = gst_videocrc_get_property;

  g_object_class_install_property (gobject_class, PROP_CRC_MASK,
      g_param_spec_uint ("crc-mask", "CRC polynomial",
          "CRC computation will use CRC polynomial set by application",
          0, G_MAXUINT, GST_VIDEO_DEFAULT_CRC_MASK, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_CRC_MESSAGE,
      g_param_spec_boolean ("crc-message", "CRC Message",
          "Post CRC messages for every video frame",
          GST_VIDEO_DEFAULT_CRC_MESSAGE, G_PARAM_READWRITE));

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_videocrc_finalize);

  videofilter_class->transform_frame_ip =
      GST_DEBUG_FUNCPTR (gst_videocrc_transform_frame_ip);
  gstbasetrans_class->passthrough_on_same_caps = TRUE;

  gst_element_class_set_metadata (GST_ELEMENT_CLASS (klass),
      "Video CRC Analyzer",
      "Filter/Analyzer/Video",
      "computes Luma & Chroma CRC for every video frame",
      "Rajesh Sharma <rajesh-dcg.sharma@st.com>");
}

static void
gst_videocrc_reset (GstVideocrc * videocrc)
{
  videocrc->crc_mask = GST_VIDEO_DEFAULT_CRC_MASK;
  videocrc->crc_message = GST_VIDEO_DEFAULT_CRC_MESSAGE;
  videocrc->frame_num = 0;
  videocrc->crc_chroma = 0;
  videocrc->crc_luma = 0;
}

static void
gst_videocrc_init (GstVideocrc * videocrc)
{
  gst_videocrc_reset (videocrc);
  gst_videocrc_init_crc32bit_table (videocrc);
  gst_base_transform_set_passthrough (GST_BASE_TRANSFORM (videocrc), TRUE);
}

static void
gst_videocrc_finalize (GObject * object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

#define WIDTH  (8 * sizeof(int))
#define TOPBIT (1 << (WIDTH - 1))

void
gst_videocrc_init_crc32bit_table (GstVideocrc * videocrc)
{
  unsigned int i, j;
  unsigned int remainder;
  unsigned int polynomial = videocrc->crc_mask;

  GST_DEBUG_OBJECT (videocrc, "Initialize CRC table using polynomial %0X",
      videocrc->crc_mask);
  /* Compute the remainder of each possible dividend */
  for (i = 0; i < 256; ++i) {
    /*  Start with the dividend followed by zeros */
    remainder = i << (WIDTH - 8);

    /* Perform modulo-2 division, a bit at a time  */
    for (j = 8; j > 0; --j) {
      /* Try to divide the current data bit */
      if (remainder & TOPBIT) {
        remainder = (remainder << 1) ^ polynomial;
      } else {
        remainder = (remainder << 1);
      }
    }
    /* Store the result into the table */
    videocrc->crc32bit_table[i] = remainder;
  }
}

static void
gst_videocrc_post_message (GstVideocrc * videocrc)
{
  GstMessage *crc_msg;
  crc_msg = gst_message_new_element (GST_OBJECT_CAST (videocrc),
      gst_structure_new ("GstVideoCRC",
          "VideoFrame", G_TYPE_UINT, videocrc->frame_num,
          "crc-chroma", G_TYPE_UINT, videocrc->crc_chroma,
          "crc-luma", G_TYPE_UINT, videocrc->crc_luma, NULL));

  gst_element_post_message (GST_ELEMENT_CAST (videocrc), crc_msg);
}

static void
gst_videocrc_crc_interlaced (GstVideocrc * videocrc,
    GstVideoFrame * frame, gboolean isNV12_format, gboolean top_field)
{
  gint i, j;
  guint8 *LumaAddress, *CbAddress, *CrAddress;
  gint width = frame->info.width;
  gint height = frame->info.height;
  gint strideY, strideU, strideV;
  guint32 CRC_Y;
  guint32 CRC_C;
  guint32 *CRC32Table = videocrc->crc32bit_table;
  gboolean top_field_first = ! !(frame->flags & GST_VIDEO_FRAME_FLAG_TFF);

  CRC_Y = 0xffffffff;
  CRC_C = 0xffffffff;

  LumaAddress = frame->data[0];
  CbAddress = frame->data[1];
  /* CrAddress is dummy in NV12 format */
  CrAddress = frame->data[2];
  strideY = frame->info.stride[0];
  strideU = frame->info.stride[1];
  /* strideV is dummy in NV12 format */
  strideV = frame->info.stride[2];

  /* compute CRC of top field first and then bottom field */
  if (top_field ^ top_field_first)
    LumaAddress += strideY;

  /* compute Luma CRC */
  for (i = 0; i < height; i += 2) {
    /* compute CRC for one line */
    for (j = 0; j < width; j++) {
      CRC_Y = (CRC_Y << 8) ^ CRC32Table[(CRC_Y >> 24) ^ LumaAddress[j]];
    }
    /* jump to next line, it belongs to different field */
    LumaAddress += strideY;
    /* jump to next field */
    LumaAddress += strideY;
  }

  /* compute CRC of top field first and then bottom field */
  if (top_field ^ top_field_first) {
    CbAddress += strideU;
    CrAddress += strideV;
  }

  /* compute Chroma CRC */
  for (i = 0; i < height / 2; i += 2) {
    if (isNV12_format == TRUE) {
      for (j = 0; j < width; j += 2) {
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CbAddress[j]];
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CbAddress[j + 1]];
      }
      /* jump to next line, it belongs to different field */
      CbAddress += strideU;
      /* jump to next line of the field to compute CRC */
      CbAddress += strideU;
    } else {
      for (j = 0; j < width / 2; j++) {
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CbAddress[j]];
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CrAddress[j]];
      }
      /* jump to next line, it belongs to different field */
      CbAddress += strideU;
      CrAddress += strideV;
      /* jump to next line of the field to compute CRC */
      CbAddress += strideU;
      CrAddress += strideV;
    }
  }

  videocrc->crc_luma = CRC_Y;
  videocrc->crc_chroma = CRC_C;

  /* print this info using --gst-debug=videocrc:4 */
  GST_INFO ("VideoFrame %d crc-luma = %u  crc-chroma = %u\n",
      videocrc->frame_num, videocrc->crc_luma, videocrc->crc_chroma);

  if (videocrc->crc_message == TRUE)
    gst_videocrc_post_message (videocrc);
}

static void
gst_videocrc_crc_progressive (GstVideocrc * videocrc,
    GstVideoFrame * frame, gboolean isNV12_format)
{
  gint i, j;
  guint8 *LumaAddress, *CbAddress, *CrAddress;
  gint width = frame->info.width;
  gint height = frame->info.height;
  gint strideY, strideU, strideV;
  guint32 CRC_Y;
  guint32 CRC_C;
  guint32 *CRC32Table = videocrc->crc32bit_table;

  CRC_Y = 0xffffffff;
  CRC_C = 0xffffffff;

  LumaAddress = frame->data[0];
  CbAddress = frame->data[1];
  /* CrAddress is dummy in NV12 format */
  CrAddress = frame->data[2];
  strideY = frame->info.stride[0];
  strideU = frame->info.stride[1];
  /* strideV is dummy in NV12 format */
  strideV = frame->info.stride[2];

  /* compute Luma CRC */
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      CRC_Y = (CRC_Y << 8) ^ CRC32Table[(CRC_Y >> 24) ^ LumaAddress[j]];
    }
    LumaAddress += strideY;
  }

  /* compute Chroma CRC */
  for (i = 0; i < height / 2; i++) {
    if (isNV12_format == TRUE) {
      for (j = 0; j < width; j += 2) {
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CbAddress[j]];
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CbAddress[j + 1]];
      }
      CbAddress += strideU;
    } else {
      for (j = 0; j < width / 2; j++) {
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CbAddress[j]];
        CRC_C = (CRC_C << 8) ^ CRC32Table[(CRC_C >> 24) ^ CrAddress[j]];
      }
      CbAddress += strideU;
      CrAddress += strideV;
    }
  }

  videocrc->crc_luma = CRC_Y;
  videocrc->crc_chroma = CRC_C;

  /* print this info using --gst-debug=videocrc:4 */
  GST_INFO ("VideoFrame %d crc-luma = %u  crc-chroma = %u\n",
      videocrc->frame_num, videocrc->crc_luma, videocrc->crc_chroma);

  if (videocrc->crc_message == TRUE)
    gst_videocrc_post_message (videocrc);
}

static GstFlowReturn
gst_videocrc_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  GstVideocrc *videocrc = GST_VIDEOCRC (filter);
  gboolean isNV12_format = TRUE;

  /* initialize crc table if user has set new polynomial */
  if ((videocrc->frame_num == 0)
      && (videocrc->crc_mask != GST_VIDEO_DEFAULT_CRC_MASK)) {
    gst_videocrc_init_crc32bit_table (videocrc);
  }

  videocrc->frame_num++;

  if (frame->info.finfo->name != NULL) {
    if (!g_strcmp0 (frame->info.finfo->name, "NV12")) {
      isNV12_format = TRUE;
    } else {
      isNV12_format = FALSE;
    }
  } else {
    GST_WARNING_OBJECT (videocrc, "video frame format is not known");
    return GST_FLOW_OK;
  }

  if (frame->flags & GST_VIDEO_FRAME_FLAG_INTERLACED) {
    GST_DEBUG_OBJECT (videocrc,
        "Interlaced VideoFrame Width x Height = %d x %d, Video Format is %s",
        frame->info.width, frame->info.height, frame->info.finfo->name);
    /*  compute CRC of top field */
    gst_videocrc_crc_interlaced (videocrc, frame, isNV12_format, TRUE);
    /*  compute CRC of bottom field */
    gst_videocrc_crc_interlaced (videocrc, frame, isNV12_format, FALSE);
  } else {
    GST_DEBUG_OBJECT (videocrc,
        "Progressive VideoFrame Width x Height = %d x %d, Video Format is %s",
        frame->info.width, frame->info.height, frame->info.finfo->name);
    gst_videocrc_crc_progressive (videocrc, frame, isNV12_format);
  }

  return GST_FLOW_OK;
}

static void
gst_videocrc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVideocrc *videocrc = GST_VIDEOCRC (object);

  switch (prop_id) {
    case PROP_CRC_MESSAGE:
      videocrc->crc_message = g_value_get_boolean (value);
      break;
    case PROP_CRC_MASK:
      videocrc->crc_mask = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_videocrc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstVideocrc *videocrc = GST_VIDEOCRC (object);

  switch (prop_id) {
    case PROP_CRC_MESSAGE:
      g_value_set_boolean (value, videocrc->crc_message);
      break;
    case PROP_CRC_MASK:
      g_value_set_uint (value, videocrc->crc_mask);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  gboolean ret;

  GST_DEBUG_CATEGORY_INIT (gst_videocrc_debug, "videocrc", 0,
      "videocrc element");

  ret = gst_element_register (plugin, "videocrc", GST_RANK_NONE,
      GST_TYPE_VIDEOCRC);

  return ret;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    videocrc,
    "computes video CRC for every video frame",
    plugin_init, VERSION, "LGPL", GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
