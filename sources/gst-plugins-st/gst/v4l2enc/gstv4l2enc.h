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

#ifndef  GSTV4L2ENC_H
#define  GSTV4L2ENC_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideoencoder.h>
#include <gst/video/gstvideopool.h>
#include <sys/ioctl.h>
#include <pthread.h>
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

/* Number of buffers in input pool  */
#define NB_BUF_INPUT_POOL 21

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_V4L2ENC	(gst_v4l2enc_get_type())
#define GST_V4L2ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_V4L2ENC, GstV4L2Enc))
#define GST_V4L2ENC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_V4L2ENC, GstV4L2EncClass))
#define GST_IS_V4L2ENC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_V4L2ENC))
#define GST_IS_V4L2ENC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_V4L2ENC))
typedef struct _GstV4L2Enc GstV4L2Enc;
typedef struct _GstV4L2EncClass GstV4L2EncClass;
typedef struct _GstV4L2EncControls GstV4L2EncControls;

#include <gstv4l2encbufferpool.h>

struct _GstV4L2EncControls
{
  /* properties */
  guint pending_flags;

  /* static controls:
   * can not be set at runtime */
  guint bitrate_mode;
  guint level;
  guint pixel_aspect_ratio;
  guint profile;
  guint intra_refresh;
  gboolean dct8x8;
  gboolean cabac;

  /* dynamic controls:
   * can be set at runtime */
  guint gop_size;
  guint cpb_size;               /* in kbps */
  guint bitrate;                /* in kbps */
  guint qpmin;
  guint qpmax;
};

struct _GstV4L2Enc
{
  GstVideoEncoder parent;

  gchar *device_name;

  gint fd;

  /* required in case encoder pool is not used */
  gint v4l2_output_fds[NB_BUF_INPUT_POOL];
  gboolean out_no_pool_setup;

  void **mmap_virtual_output;   /* Pointer tab of output frames */
  gint *mmap_size_output;

  GstBufferPool *pool;          /* Pool of input frames */

  gint current_nb_buf_output;   /* used for output munmaping */

  /* Structure representing the state of an incoming or outgoing video stream
     for encoders and decoders. */
  GstVideoCodecState *input_state;
  GstVideoCodecState *output_state;

  gint width;
  gint height;

  /* v4l2 encoder controls */
  GstV4L2EncControls controls;

  GstCaps *caps;
  GstVideoInfo info;
};

struct _GstV4L2EncClass
{

  GstVideoEncoderClass parent_class;
};

GType gst_v4l2enc_get_type (void);

gchar *v4l2_fmt_str (guint32 fmt);

G_END_DECLS
#endif /* GSTV4L2ENC_H */
