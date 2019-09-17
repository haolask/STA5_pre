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

#ifndef __GST_VIDEOCRC_H__
#define __GST_VIDEOCRC_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS
#define GST_TYPE_VIDEOCRC \
  (gst_videocrc_get_type())
#define GST_VIDEOCRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VIDEOCRC,GstVideocrc))
#define GST_VIDEOCRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VIDEOCRC,GstVideocrcClass))
#define GST_IS_VIDEOCRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VIDEOCRC))
#define GST_IS_VIDEOCRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VIDEOCRC))
typedef struct _GstVideocrc GstVideocrc;
typedef struct _GstVideocrcClass GstVideocrcClass;

/**
 * GstVideocrc:
 *
 * Opaque #GstVideocrc element structure
 */
struct _GstVideocrc
{
  GstVideoFilter element;

  /*< private > */
  guint32 crc_chroma;           /* chroma CRC */
  guint32 crc_luma;             /* luma CRC */
  guint32 frame_num;            /* video frame number */
  gboolean crc_message;         /* post message to app if TRUE */
  guint32 crc_mask;             /* CRC POLYNOMIAL */
  guint32 crc32bit_table[256];  /* pre computed CRC table */
};

struct _GstVideocrcClass
{
  GstVideoFilterClass parent_class;
};

GType gst_videocrc_get_type (void);

G_END_DECLS
#endif /* __GST_VIDEOCRC_H__ */
