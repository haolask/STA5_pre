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

#ifndef  GSTV4L2DEC_H
#define  GSTV4L2DEC_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideodecoder.h>
#include <gst/video/gstvideopool.h>
#include <gst/allocators/gstdmabuf.h>
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

/* Begin Declaration */
G_BEGIN_DECLS
#define GST_TYPE_V4L2DEC	(gst_v4l2dec_get_type())
#define GST_V4L2DEC(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_V4L2DEC, GstV4L2Dec))
#define GST_V4L2DEC_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_V4L2DEC, GstV4L2DecClass))
#define GST_IS_V4L2DEC(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_V4L2DEC))
#define GST_IS_V4L2DEC_CLASS(obj) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_V4L2DEC))
typedef struct _GstV4L2Dec GstV4L2Dec;
typedef struct _GstV4L2DecPoll GstV4L2DecPoll;
typedef struct _GstV4L2DecClass GstV4L2DecClass;
typedef struct _GstV4L2DecDownstreamMeta GstV4L2DecDownstreamMeta;
typedef struct _GstV4L2DecQosElt GstV4L2DecQosElt;

#include <gstv4l2decbufferpool.h>

#define MAX_BUFFERS 30

enum V4L2DecFrameDiscardState
{
  V4L2DEC_DEFAULT = 0,
  V4L2DEC_DESYNCHRO = 1,
  V4L2DEC_WAITFOR_IFRAME = 2,
};

struct _GstV4L2DecPoll
{
  GstPoll *p;                   /* a poll for video_fd */
  GstPollFD fd;
};

/***** QOS parameters *******/
/* Define lenght of history values to identify the trend indicating if there is an underflow or not */
#define HISTORY_LENGHT 10
/* To NOT wait for next keyframe before going out of underflow  */
//#define QOS_DEGRADATION_LEVEL 1

struct _GstV4L2DecQosElt
{
  GstClockTimeDiff diff;
  struct _GstV4L2DecQosElt *pnext;
};

struct _GstV4L2Dec
{
  GstVideoDecoder parent;

  gint fd;
  void **mmap_virtual_input;    /* Pointer tab of input AUs */
  gint *mmap_size_input;
  gint current_nb_buf_input;    /* used for input munmaping */

  /* Structure representing the state of an incoming or outgoing video stream
     for encoders and decoders. */
  GstVideoCodecState *input_state;
  GstVideoCodecState *output_state;

  gchar *format_in_to_str;
  gchar *format_out_to_str;

  GstCaps *caps_in;
  GstCaps *caps_out;

  GstBufferPool *pool;          /* Pool of output frames */
  GstBufferPool *downstream_pool;       /* Pool of output frames */

  GstV4L2DecPoll poll[3];
  gboolean can_poll_device;

  gboolean output_setup;
  gboolean input_setup;

  gboolean input_streaming;
  gboolean output_streaming;

  /* input infos */
  gint width;
  gint height;
  __u32 streamformat;

  GstCaps *preferred_caps;

  /* used for multi-threading */
  GstTask *frame_push_task;
  GRecMutex frame_push_task_mutex;

  GstTask *frame_recycle_task;
  GRecMutex frame_recycle_task_mutex;

  GstBuffer *codec_data;
  GstBuffer *header;

  unsigned char sps_pps_buf[100];
  unsigned int sps_pps_size;

  GstVideoAlignment align;
  gint size_image;

  GstBuffer *downstream_buffers[MAX_BUFFERS];

  gboolean eos_driver;
  GMutex eos_mutex;
  GCond eos_cond;

  /*QOS mechanism */
  enum V4L2DecFrameDiscardState frame_discard_state;
  GstV4L2DecQosElt *qos_trace;
  gboolean qos_underflow;
  gboolean skip_frame;
  gboolean low_latency_mode;
};

struct _GstV4L2DecClass
{
  GstVideoDecoderClass parent_class;
};

struct _GstV4L2DecDownstreamMeta
{
  GstMeta meta;
  gboolean acquired;
};

GType gst_v4l2dec_downstream_meta_api_get_type (void);
const GstMetaInfo *gst_v4l2dec_downstream_meta_get_info (void);
#define GST_V4L2DEC_DOWNSTREAM_META_GET(buf) ((GstV4L2DecDownstreamMeta *)gst_buffer_get_meta(buf,gst_v4l2dec_downstream_meta_api_get_type()))
#define GST_V4L2DEC_DOWNSTREAM_META_ADD(buf) ((GstV4L2DecDownstreamMeta *)gst_buffer_add_meta(buf,gst_v4l2dec_downstream_meta_get_info(),NULL))

GType gst_v4l2dec_get_type (void);


G_END_DECLS
#endif /* __GST_v4l2dec_H__ */
