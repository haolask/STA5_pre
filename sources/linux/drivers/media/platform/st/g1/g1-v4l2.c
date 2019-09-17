/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Jean-Christophe Trotin <jean-christophe.trotin@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/version.h>

#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-dma-contig.h>
#include "sta_rpmsg_mm.h"

#include "g1.h"
#include "g1-debug.h"

#define G1_NAME	"g1"

#define to_au(__vbuf) container_of(__vbuf, struct g1_au, vbuf)
#define to_frame(__vbuf) container_of(__vbuf, struct g1_frame, vbuf)
#define to_ctx(__fh) container_of(__fh, struct g1_ctx, fh)
#define to_vb2q(ctx, type) (type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? \
				&(ctx)->q_aus : &(ctx)->q_frames)
#define to_type_str(type) (type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "au" : "frame")

#define LOG_G1_API_ENTRY(ctx, dev, str) \
	log_g1_api((ctx), (dev), ">", __func__, (str))

#define LOG_G1_API_EXIT(ctx, dev, str) \
	log_g1_api((ctx), (dev), "<", __func__, (str))

#define LOG_G1_API_ENTRY_MSG(ctx, dev, str, msg) \
	log_g1_api_msg((ctx), (dev), ">", __func__, (str), (msg))

#define LOG_G1_API_EXIT_MSG(ctx, dev, str, msg) \
	log_g1_api_msg((ctx), (dev), "<", __func__, (str), (msg))

static inline void log_g1_api(struct g1_ctx *ctx, struct device *dev,
			      char *dir, const char *fct, char *str) {
	dev_dbg(dev, "%s g1_api[vid-%s] %s%s\n",
		(ctx ? ctx->name : "[   :    ]"),
		str, dir, fct);
}

static inline void log_g1_api_msg(struct g1_ctx *ctx, struct device *dev,
				  char *dir, const char *fct,
				  char *str, char *msg) {
	dev_dbg(dev, "%s g1_api[vid-%s] %s%s [%s]\n",
		(ctx ? ctx->name : "[   :    ]"),
		str, dir, fct, msg);
}

/* offset to differentiate OUTPUT/CAPTURE @mmap, cf g1_mmap/querybuf */
#define MMAP_FRAME_OFFSET (TASK_SIZE / 2)

/* registry of available decoders */
const struct g1_dec *g1_decoders[] = {
#ifdef CONFIG_VIDEO_ST_G1_VP8
	&vp8dec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_H264
	&h264dec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_VC1
	&vc1dec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_MPEG4
	&mp4dec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_MPEG2
	&mpeg2dec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_AVS
	&avsdec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_RAW
	&rawdec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_JPEG
	&jpegdec,
#endif
#ifdef CONFIG_VIDEO_ST_G1_RV
	&rvdec,
#endif
};

enum probe_status {
	IDLE,
	PROBE_POSTPONED,
	RESUME_POSTPONED,
	PROBED
};

/* Exported rpmsg_mm services to be inform about
 * G1 IP usage on M3 core side.
 */
extern void *sta_rpmsg_register_endpoint(char *endpoint_name,
					 t_pfct_rpmsg_mm_cb callback,
					 void *priv);
extern int sta_rpmsg_unregister_endpoint(void *handle);
extern int sta_rpmsg_get_remote_resources_usage(void *handle);

#ifdef CONFIG_VIDEO_ST_G1_RV
extern int g1_rv_update_frame_init_timestamp(struct g1_ctx *pctx, u64 ts);
#endif

static inline int frame_size(__u32 w, __u32 h, __u32 fmt)
{
	switch (fmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_YUV420:
		return ((w * h * 3) >> 1);
	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_XRGB555:
	case V4L2_PIX_FMT_ARGB555:
	case V4L2_PIX_FMT_YYUV:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_YUYV:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_NV16:
		return (w * h * 2);
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_XRGB32:
	case V4L2_PIX_FMT_ABGR32:
	case V4L2_PIX_FMT_ARGB32:
		return (w * h * 4);
	case V4L2_PIX_FMT_NV24:
	case V4L2_PIX_FMT_MJPEG: // Maximum size in YUV
		return (w * h * 3);
	default:
		return 0;
	}
}

static inline int frame_stride(__u32 w, __u32 fmt)
{
	switch (fmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_YUV420:
	case V4L2_PIX_FMT_NV16:
	case V4L2_PIX_FMT_NV24:
		return w; /* in NV12, stride in bytes is width */
	case V4L2_PIX_FMT_RGB565:
	case V4L2_PIX_FMT_XRGB555:
	case V4L2_PIX_FMT_ARGB555:
	case V4L2_PIX_FMT_VYUY:
	case V4L2_PIX_FMT_UYVY:
	case V4L2_PIX_FMT_YVYU:
	case V4L2_PIX_FMT_YUYV:
		return (w * 2);
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_XRGB32:
	case V4L2_PIX_FMT_ABGR32:
	case V4L2_PIX_FMT_ARGB32:
		return (w * 4);
	default:
		return 0;
	}
}

static inline int frame_alignment(__u32 fmt)
{
	switch (fmt) {
	case V4L2_PIX_FMT_NV12:
	case V4L2_PIX_FMT_NV21:
		/* multiple of 2 */
		return 2;
	default:
		return 1;
	}
}

static inline bool err_is_fatal(int ret)
{
	switch (ret) {
	case -ENOMEM:
	case -EINVAL:
	case -EIO:
		return true;
	default:
		return false;
	}
}

static char *dump_au(struct g1_ctx *ctx,
		     struct g1_au *au,
		     unsigned char *str,
		     unsigned int len)
{
	unsigned char *cur = str;
	size_t left = len;
	int cnt = 0;
	int ret = 0;
	unsigned int i;
	__u32 size = 10;	/* dump first & last 10 bytes */
	__u8 *data = (__u8 *)(au->vaddr);

	if (au->size < size)
		size = au->size;

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "au[%d]    picid=%u %s=%lld size=%d data=",
		       au->vbuf.vb2_buf.index, au->picid,
		       (ctx->timestamp_type == G1_DECODING_TIMESTAMP ?
		       "dts" : "pts"),
		       au->ts, au->size);
	cnt = (left > ret ? ret : left);

	for (i = 0; i < size; i++) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, "%.2x ", data[i]);
		cnt = (left > ret ? ret : left);
	}

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "... ");
	cnt = (left > ret ? ret : left);

	if (au->size > size)
		for (i = (au->size - size); i < au->size; i++) {
			cur += cnt;
			left -= cnt;
			ret = snprintf(cur, left, "%.2x ", data[i]);
			cnt = (left > ret ? ret : left);
		}

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "\n");
	cnt = (left > ret ? ret : left);

	return str;
}

static char *dump_frame(struct g1_ctx *ctx,
			struct g1_frame *frame,
			unsigned char *str,
			unsigned int len)
{
	unsigned char *cur = str;
	size_t left = len;
	int cnt = 0;
	int ret = 0;
	unsigned int i;
	__u32 size = 10;	/* dump first 10 bytes */
	__u8 *data = (__u8 *)(frame->vaddr);

	if (frame->pix.sizeimage < size)
		size = frame->pix.sizeimage;

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "frame[%d] picid=%u %s=%lld paddr=0x%x ",
		       frame->index, frame->picid,
		       (ctx->timestamp_type == G1_DECODING_TIMESTAMP ?
		       "dts" : "pts"),
		       frame->ts,
		       frame->paddr);
	cnt = (left > ret ? ret : left);

	if (ctx->mode == G1_MODE_LOW_LATENCY &&
	    ctx->streaminfo.streamformat == V4L2_PIX_FMT_H264) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, "poc:%d ", frame->poc);
		cnt = (left > ret ? ret : left);
	}

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "type=%s data=",
		       frame_type_str(frame->flags));
	cnt = (left > ret ? ret : left);

	for (i = 0; i < size; i++) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, "%.2x ", data[i]);
		cnt = (left > ret ? ret : left);
	}

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "...\n");
	cnt = (left > ret ? ret : left);

	return str;
}

static void g1_push_ts(struct g1_ctx *ctx, u64 val, __u32 index)
{
	struct g1_timestamp *ts;
	struct g1_dev *g1 = ctx->dev;

	if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV30 ||
	    ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV40) {
		/* In case of RV, timestamp coming from demux is not used
		 * Duration of AU is extracted from AU itself.
		 * Frame Timestamp is computed thanks to an initial timestamp +
		 * cumulative frames duration.
		 * In this particular case, we need to transfer the timestamp
		 * so that RV decoder is able to compute the frame timestamp
		 * We don't store anything in the list
		 */
		g1_rv_update_frame_init_timestamp(ctx, val);
		return;
	}

	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts)
		return;

	INIT_LIST_HEAD(&ts->list);

	/* protected by global lock acquired
	 * by V4L2 when calling g1_vb2_au_queue
	 */
	ts->val = val;
	ts->index = index;

	/* whatever the kind of timestamp (decode or
	 * presentation timestamp), we store it at the
	 * end of the list
	 */
	list_add_tail(&ts->list, &ctx->timestamps);
	dev_dbg(g1->dev, "%s timestamp => picid:%d ts:%lld",
		__func__, index, val);
}

static void g1_pop_ts(struct g1_ctx *ctx, u64 *val, __u32 index)
{
	struct g1_dev *g1 = ctx->dev;
	struct g1_timestamp *iter_ts, *next_ts;

	if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV30 ||
	    ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV40)
		/* In case of RV, timestamps have not been stored in the list.
		 * Frame timestamp has already been filled by the decoder
		 * itself.
		 */
		return;

	/* protected by global lock acquired
	 * by V4L2 when calling g1_vb2_au_queue
	 */
	if (list_empty(&ctx->timestamps))
		goto not_found;

	/* FIXME : hack to workaround lack of picId in
	 * G1 VP8, CAVS and JPEG interface.
	 * AU are supposed to arrive in PTS/DTS order
	 */
	if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_VP8 ||
	    ctx->streaminfo.streamformat == V4L2_PIX_FMT_JPEG ||
	    ctx->streaminfo.streamformat == V4L2_PIX_FMT_MJPEG ||
	    ctx->streaminfo.streamformat == V4L2_PIX_FMT_CAVS)
		ctx->timestamp_type = G1_DECODING_TIMESTAMP;

	if (ctx->timestamp_type == G1_DECODING_TIMESTAMP) {
		/* decoding timestamps are consecutively stored
		 * so in this case we return the oldest DTS (first)
		 */
		iter_ts = list_first_entry(&ctx->timestamps,
					   struct g1_timestamp, list);
		goto found;
	} else {
		/* presentation timestamp are received in decode order
		 * to retrieve the good one we use the picture index
		 */
		list_for_each_entry_safe(iter_ts, next_ts,
					 &ctx->timestamps, list) {
			if (iter_ts->index == index)
				goto found;
		}
	}

not_found:
	dev_warn(g1->dev,
		 "%s  no timestamp associated to picid:%d... output timestamp = 0\n",
		 ctx->name, index);
	*val = 0;
	return;

found:
	list_del(&iter_ts->list);
	*val = iter_ts->val;
	kfree(iter_ts);
	dev_dbg(g1->dev, "%s timestamp => picid:%d ts:%lld",
		__func__, index, *val);
}

static void register_decoder(struct g1_dev *g1, const struct g1_dec *dec)
{
	int ret;

	/* those decoder ops are mandatory */
	if (!dec->probe ||
	    !dec->open ||
	    !dec->close ||
	    !dec->setup_frame ||
	    !dec->get_streaminfo ||
	    !dec->set_frameinfo ||
	    !dec->decode ||
	    !dec->get_frame ||
	    !dec->recycle ||
	    !dec->flush ||
	    !dec->drain) {
		dev_warn(g1->dev, "%s not registered : at least one service is missing\n",
			 dec->name);
		return;
	}

	/* probe decoder to check if this codec
	 * is really supported by hardware
	 */
	ret = dec->probe(g1);
	if (ret)
		return;

	if (g1->nb_of_decoders >= G1_MAX_DECODERS) {
		dev_err(g1->dev, "Max decoders number reached : increase G1_MAX_DECODERS value\n");
		return;
	}

	g1->decoders[g1->nb_of_decoders] = dec;
	g1->nb_of_decoders++;

	dev_info(g1->dev, "%s decoder registered\n", dec->name);
}

static void register_all(struct g1_dev *g1)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g1_decoders); i++)
		register_decoder(g1, g1_decoders[i]);
}

static void g1_vb2_lock(struct vb2_queue *q)
{
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;

	mutex_lock(&g1->lock);
}

static void g1_vb2_unlock(struct vb2_queue *q)
{
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;

	mutex_unlock(&g1->lock);
}

static int g1_querycap(struct file *file, void *priv,
		       struct v4l2_capability *cap)
{
	struct g1_dev *g1 = video_drvdata(file);

	LOG_G1_API_ENTRY(NULL, g1->dev, "gen");
	strlcpy(cap->driver, g1->pdev->name, sizeof(cap->driver));
	strlcpy(cap->card, g1->pdev->name, sizeof(cap->card));

	snprintf(cap->bus_info, sizeof(cap->bus_info), "platform:%s", G1_NAME);
	cap->device_caps = V4L2_CAP_STREAMING | V4L2_CAP_VIDEO_M2M;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
	LOG_G1_API_EXIT(NULL, g1->dev, "gen");
	return 0;
}

static int g1_enum_fmt_stream(struct file *file, void *fh,
			      struct v4l2_fmtdesc *f)
{
	struct g1_dev *g1 = video_drvdata(file);
	struct g1_ctx *ctx = to_ctx(fh);
	int stream_nb = 0;
	int i = 0;
	int s = 0;

	LOG_G1_API_ENTRY(ctx, g1->dev, "out");
	while (i < g1->nb_of_decoders) {
		for (s = 0; s < g1->decoders[i]->nb_streams; s++) {
			if (f->index == stream_nb) {
				f->pixelformat =
					g1->decoders[i]->streamformat[s];
				snprintf(f->description,
					 sizeof(f->description),
					 "%4.4s", (char *)&f->pixelformat);
				if (strcmp(g1->decoders[i]->name, "rawdec"))
					f->flags = V4L2_FMT_FLAG_COMPRESSED;
				LOG_G1_API_EXIT_MSG(ctx, g1->dev, "out",
						    f->description);
				return 0;
			}
			stream_nb++;
		}
		i++;
	}
	LOG_G1_API_EXIT_MSG(ctx, g1->dev, "out", "-EINVAL");
	return -EINVAL;
}

static const struct g1_dec *is_supported_streamformat(struct g1_ctx *ctx,
						      __u32 streamformat)
{
	struct g1_dev *g1 = ctx->dev;
	unsigned int i, j;

	for (i = 0; i < g1->nb_of_decoders; i++) {
		if (g1->decoders[i]) {
			for (j = 0; j < g1->decoders[i]->nb_streams; j++) {
				if (g1->decoders[i]->streamformat[j]
					== streamformat)
					return g1->decoders[i];
			}
		}
	}
	return NULL;
}

static int g1_g_fmt_stream(struct file *file, void *fh,
			   struct v4l2_format *f)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;
	const struct g1_dec *dec = ctx->dec;
	char msg[64];

	LOG_G1_API_ENTRY(ctx, g1->dev, "out");
	if (dec)
		dec->get_streaminfo(ctx, streaminfo);

	pix->width = streaminfo->aligned_width;
	pix->height = streaminfo->aligned_height;
	pix->pixelformat = streaminfo->streamformat;
	pix->sizeimage = ctx->max_au_size;
	pix->bytesperline = streaminfo->aligned_width;
	pix->colorspace = streaminfo->colorspace;
	pix->field = streaminfo->field;

	snprintf(msg, sizeof(msg), "%dx%d %4.4s",
		 pix->width, pix->height,
		 (char *)&pix->pixelformat);
	LOG_G1_API_EXIT_MSG(ctx, g1->dev, "out", msg);

	return 0;
}

static int g1_try_fmt_stream(struct file *file, void *fh,
			     struct v4l2_format *f)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	const struct g1_dec *dec = NULL;
	u32 width, height;
	int max_au_size = G1_MAX_RESO;
	u32 g1_max_res = 0;
	u16 g1_min_width = 0, g1_min_height = 0;
	u16 g1_max_width = 0, g1_max_height = 0;
	char msg[64];

	snprintf(msg, sizeof(msg), "%dx%d %4.4s",
		 pix->width, pix->height,
		 (char *)&pix->pixelformat);
	LOG_G1_API_ENTRY_MSG(ctx, g1->dev, "out", msg);

	/**** request validation ****/
	/* stream */
	dec = is_supported_streamformat(ctx, pix->pixelformat);
	if (!dec) {
		dev_dbg(g1->dev,
			"%s V4L2 TRY_FMT(OUTPUT): unsupported format\n",
			ctx->name);
		goto def;
	}

	if (dec->width_range) {
		g1_min_width = dec->width_range[0];
		g1_max_width = dec->width_range[1];
	} else {
		g1_min_width = G1_MIN_WIDTH;
		g1_max_width = G1_MAX_WIDTH;
	}
	if (dec->height_range) {
		g1_min_height = dec->height_range[0];
		g1_max_height = dec->height_range[1];
	} else {
		g1_min_height = G1_MIN_HEIGHT;
		g1_max_height = G1_MAX_HEIGHT;
	}
	if (dec->max_pixel_nb)
		g1_max_res = dec->max_pixel_nb;
	else
		g1_max_res = G1_MAX_RESO;

	/* adjust width & height */
	width = pix->width;
	height = pix->height;
	v4l_bound_align_image
		(&pix->width,
		 g1_min_width,
		 g1_max_width,
		 0,
		 &pix->height,
		 g1_min_height,
		 g1_max_height,
		 0, 0);

	if ((pix->width != width) || (pix->height != height))
		dev_dbg(g1->dev,
			"%s V4L2 TRY_FMT (OUTPUT): resolution updated %dx%d -> %dx%d to fit min/max/alignment\n",
			ctx->name, width, height,
			pix->width, pix->height);

	max_au_size = frame_size(pix->width, pix->height, pix->pixelformat);
	if (!max_au_size)
		max_au_size = g1_max_res;
	/* input buffer size */
	if ((pix->sizeimage <= 0)) {
		dev_dbg(g1->dev,
			"%s V4L2 TRY_FMT(OUTPUT): invalid size: %d size (%dx%d) is 0\n",
			ctx->name,
			pix->sizeimage,
			pix->width, pix->height);
		goto def;
	}
	pix->sizeimage = max_au_size;
	if (pix->field == V4L2_FIELD_ANY)
		pix->field = V4L2_FIELD_NONE;

	LOG_G1_API_EXIT(ctx, g1->dev, "out");
	return 0;

def:
	/* returns the current format parameters as VIDIOC_G_FMT does */
	g1_g_fmt_stream(file, fh, f);

	LOG_G1_API_EXIT(ctx, g1->dev, "out");
	return 0;
}

static int g1_update_fmt_info(struct g1_ctx *ctx, struct g1_streaminfo *stream,
			      struct g1_frameinfo *frame)
{
	struct g1_dev *g1 = ctx->dev;
	const struct g1_dec *dec = NULL;

	dec = is_supported_streamformat(ctx, stream->streamformat);
	if (!dec) {
		dev_dbg(g1->dev, "%s (format %s not supported)", ctx->name,
			(char *)&stream->streamformat);
		return -EINVAL;
	}

	if (ctx->dec && ctx->dec != dec) {
		ctx->dec->close(ctx);
		ctx->dec = NULL;
	}
	if (!ctx->dec) {
		memcpy(&ctx->streaminfo, stream, sizeof(ctx->streaminfo));
		/* open decoder instance */
		if (dec->open(ctx)) {
			dev_err(g1->dev, "%s dec->open failed", ctx->name);
			return -EINVAL;
		}
		ctx->dec = dec;
		snprintf(ctx->name, sizeof(ctx->name), "[%3d:%4s]",
			 ctx->instance_id, dec->name);
	}
	if (dec->set_streaminfo)
		dec->set_streaminfo(ctx, stream);

	if (dec->set_frameinfo)
		dec->set_frameinfo(ctx, frame);

	if (dec->get_streaminfo)
		dec->get_streaminfo(ctx, &ctx->streaminfo);

	if (dec->get_frameinfo)
		dec->get_frameinfo(ctx, &ctx->frameinfo);

	return 0;
}

static int g1_s_fmt_stream(struct file *file, void *fh, struct v4l2_format *f)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct g1_streaminfo streaminfo;
	struct g1_dev *g1 = ctx->dev;

	LOG_G1_API_ENTRY(ctx, g1->dev, "out");
	g1_try_fmt_stream(file, fh, f);
	memset(&streaminfo, 0, sizeof(streaminfo));
	streaminfo.width = pix->width;
	streaminfo.height = pix->height;
	streaminfo.aligned_width = pix->width;
	streaminfo.aligned_height = pix->height;
	streaminfo.streamformat = pix->pixelformat;
	streaminfo.colorspace = pix->colorspace;
	streaminfo.field = pix->field;
	streaminfo.flags &= ~G1_STREAMINFO_FLAG_CROP;
	streaminfo.crop.width = 0;
	streaminfo.crop.height = 0;

	if (!g1_update_fmt_info(ctx, &streaminfo, &ctx->frameinfo))
		ctx->state = G1_STATE_WF_STREAMINFO;

	ctx->max_au_size = frame_size(ctx->streaminfo.aligned_width,
				      ctx->streaminfo.aligned_height,
				      pix->pixelformat);
	if (!ctx->max_au_size)
		ctx->max_au_size = G1_MAX_RESO;

	/* returns the current format parameters as VIDIOC_G_FMT does */
	g1_g_fmt_stream(file, fh, f);
	LOG_G1_API_EXIT(ctx, g1->dev, "out");
	return 0;
}

int g1_reqbufs(struct file *file, void *fh, struct v4l2_requestbuffers *b)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	int ret = 0;
	char video_type[5];

	strncpy(video_type,
		(b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);
	/* request validation */
	if ((b->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) &&
	    (b->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
		dev_err(g1->dev,
			"%s V4L2 REQBUFS: only V4L2_BUF_TYPE_VIDEO_OUTPUT/CAPTURE are supported\n",
			ctx->name);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EINVAL");
		return -EINVAL;
	}

	if (ctx->state == G1_STATE_FATAL_ERROR) {
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EIO");
		return -EIO;
	}

	/* vb2 call
	 * will call g1_vb2_au_queue_setup or g1_vb2_frame_queue_setup
	 */
	ret = vb2_reqbufs(to_vb2q(ctx, b->type), b);
	if (ret) {
		char msg[10];

		sprintf(msg, "%d", ret);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, msg);
		return ret;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_querybuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	int ret;
	char video_type[5];

	strncpy(video_type,
		(b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);
	/* request validation */
	if ((b->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) &&
	    (b->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
		dev_err(g1->dev,
			"%s V4L2 QUERYBUF: only V4L2_BUF_TYPE_VIDEO/CAPTURE are supported\n",
			ctx->name);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EINVAL");
		return -EINVAL;
	}

	if (ctx->state == G1_STATE_FATAL_ERROR) {
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EIO");
		return -EIO;
	}

	/* vb2 call */
	ret = vb2_querybuf(to_vb2q(ctx, b->type), b);
	if (ret) {
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EINVAL");
		return ret;
	}

	/* add an offset to differentiate OUTPUT/CAPTURE @mmap time */
	if ((b->memory == V4L2_MEMORY_MMAP) &&
	    (b->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)) {
		b->m.offset += MMAP_FRAME_OFFSET;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_expbuf(struct file *file, void *fh, struct v4l2_exportbuffer *b)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	int ret = 0;

	char video_type[5];

	strncpy(video_type,
		(b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);

	if (ctx->state == G1_STATE_FATAL_ERROR) {
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EIO");
		return -EIO;
	}

	/* vb2 call */
	ret = vb2_expbuf(to_vb2q(ctx, b->type), b);
	if (ret) {
		char msg[10];

		sprintf(msg, "%d", ret);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, msg);
		return ret;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_qbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	int ret = 0;
	char video_type[5];
	char msg[10];

	strncpy(video_type,
		(b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);
	if (ctx->state == G1_STATE_FATAL_ERROR) {
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EIO");
		return -EIO;
	}
	/* vb2 call
	 * will call g1_vb2_au_queue or g1_vb2_frame_queue
	 */
	ret = vb2_qbuf(to_vb2q(ctx, b->type), b);
	if (ret) {
		snprintf(msg, sizeof(msg), "%d", ret);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, msg);
		return ret;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_streamon(struct file *file, void *fh, enum v4l2_buf_type i)
{
	struct g1_ctx *ctx = to_ctx(fh);
	const struct g1_dec *dec = ctx->dec;
	struct g1_dev *g1 = ctx->dev;
	int ret = 0;
	char video_type[5];

	strncpy(video_type,
		(i == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);
	if (ctx->state == G1_STATE_FATAL_ERROR) {
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EIO");
		return -EIO;
	}

	if (i > 0 && i <= V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		ctx->streamon_done[i] = true;
		if (ctx->streamon_done[V4L2_BUF_TYPE_VIDEO_CAPTURE] &&
		    ctx->streamon_done[V4L2_BUF_TYPE_VIDEO_OUTPUT]) {
			if (dec && dec->validate_config) {
				if (dec->validate_config(ctx) < 0) {
					dev_warn(g1->dev,
						 "%s invalid configuration\n",
						ctx->name);
					LOG_G1_API_EXIT_MSG(ctx, g1->dev,
							    video_type,
							    "-EINVAL");
					return -EINVAL;
				}
			}
		}
	}
	/* vb2 call
	 * will call g1_vb2_au_queue or
	 * g1_vb2_frame_start_streaming & g1_vb2_frame_queue
	 */
	ret = vb2_streamon(to_vb2q(ctx, i), i);
	if (ret) {
		char msg[10];

		sprintf(msg, "%d", ret);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, msg);
		return ret;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	int ret = 0;
	char video_type[5];

	strncpy(video_type,
		(b->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);

	if (ctx->state == G1_STATE_FATAL_ERROR) {
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EIO");
		return -EIO;
	}

	/* vb2 call
	 * will return au or frame buffers returned through vb2_buffer_done()
	 */
	ret = vb2_dqbuf(to_vb2q(ctx, b->type), b, file->f_flags & O_NONBLOCK);
	if (ret) {
		char msg[10];

		sprintf(msg, "%d", ret);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, msg);
		return ret;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_streamoff(struct file *file, void *fh, enum v4l2_buf_type i)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	int ret = 0;
	char video_type[5];

	strncpy(video_type,
		(i == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);

	if (i > 0 && i <= V4L2_BUF_TYPE_VIDEO_OUTPUT)
		ctx->streamon_done[i] = false;

	/* vb2 call
	 * will call g1_vb2_au_stop_streaming or g1_vb2_frame_stop_streaming
	 */
	ret = vb2_streamoff(to_vb2q(ctx, i), i);
	if (ret) {
		char msg[10];

		sprintf(msg, "%d", ret);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, msg);
		return ret;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_vb2_au_queue_setup(struct vb2_queue *q,
				 unsigned int *num_buffers,
				 unsigned int *num_planes,
				 unsigned int sizes[],
				 struct device *alloc_devs[])
{
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	*num_planes = 1;
	if (*num_buffers < 1)
		*num_buffers = 1;
	if (*num_buffers > G1_MAX_AUS)
		*num_buffers = G1_MAX_AUS;

	if (sizes[0])
		dev_warn(g1->dev, "%s psize[0] already set to %d\n",
			 ctx->name, sizes[0]);

	if (alloc_devs[0])
		dev_warn(g1->dev, "%s allocators[0] already set\n", ctx->name);

	if (ctx->max_au_size <= 0) {
		dev_err(g1->dev,
			"%s ctx->max_au_size is 0, verify S_FMT(OUTPUT, sizeimage)\n",
			ctx->name);
		ctx->max_au_size = G1_MAX_RESO;
	}

	sizes[0] = ctx->max_au_size;
	vb2_dma_contig_set_max_seg_size(g1->dev, DMA_BIT_MASK(32));
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return 0;
}

static int g1_vb2_au_prepare(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct g1_au *au = to_au(vbuf);

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	if (!au->prepared) {
		/* get memory addresses */
		au->vaddr = vb2_plane_vaddr(&au->vbuf.vb2_buf, 0);
		au->paddr = vb2_dma_contig_plane_dma_addr(&au->vbuf.vb2_buf, 0);

		au->state = G1_AU_FREE;
		ctx->aus[vb->index] = au;
		ctx->nb_of_aus++;

		au->prepared = 1;
		dev_dbg(g1->dev, "%s au[%d] prepared; virt=0x%p, phy=0x%p\n",
			ctx->name, vb->index,
			(void *)au->vaddr, (void *)au->paddr);
	}

	/* update au size (from user) */
	au->size = vb2_get_plane_payload(vb, 0);
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return 0;
}

static void g1_au_done(struct g1_ctx *ctx, struct g1_au *au, int err)
{
	if (au->state & G1_AU_OUT)
		return;

	au->state |= G1_AU_OUT;

	vb2_buffer_done(&au->vbuf.vb2_buf,
			err ? VB2_BUF_STATE_ERROR : VB2_BUF_STATE_DONE);

	if (ctx->decoded_frames > 0)
		g1_perf_end(ctx);
}

static void g1_frame_done(struct g1_ctx *ctx, struct g1_frame *frame, int err)
{
	struct g1_dev *g1 = ctx->dev;
	unsigned char str[100] = "";

	/* dump frame */
	dev_dbg(g1->dev, "%s dump %s", ctx->name,
		dump_frame(ctx, frame, str, sizeof(str)));

	/* decoded frame is now output to user */
	frame->state |= G1_FRAME_OUT;

	vb2_buffer_done(&frame->vbuf.vb2_buf,
			err ? VB2_BUF_STATE_ERROR : VB2_BUF_STATE_DONE);

	ctx->output_frames++;

	g1_perf_output(ctx);
}

int g1_setup_frame(struct g1_ctx *ctx, struct g1_frame *frame)
{
	struct g1_dev *g1 = ctx->dev;

	if (frame->index >= sizeof(ctx->frames)) {
		dev_err(g1->dev,
			"%s  frame index=%d exceeds output frame count (%d)\n",
			ctx->name,
			frame->index,
			sizeof(ctx->frames));
		return -EINVAL;
	}

	frame->state = G1_FRAME_FREE;
	ctx->frames[frame->index] = frame;
	ctx->nb_of_frames++;

	frame_state_str(frame->state,
			ctx->str, sizeof(ctx->str));
	return 0;
}

static int g1_wait_recycle(struct g1_ctx *ctx)
{
	struct g1_dev *g1 = ctx->dev;
	int ret = 0;
	bool closed = false;

	/* block till out frames recycled */
retry:
	mutex_unlock(&g1->lock);	/* release global lock to let
					 * throw qbuf(frame) or close() or
					 * streamoff(frame)
					 */

	ret = wait_event_interruptible(ctx->wq_recycle,
				       ((ctx->recycled_frames > 0) ||
					(ctx->state == G1_STATE_STOPPED) ||
					(closed =
					 (ctx->state == G1_STATE_CLOSED))));

	/* reacquire global lock */
	mutex_lock(&g1->lock);

	/* if closed, ctx is potentially free-ed at this point,
	 * so exit without any ctx accesses
	 */
	if (closed)
		return -EINTR;

	/* we need to reevaluate conditions again after reacquiring
	 * the lock or return an error if one occurred.
	 */
	if (ret < 0) {
		/* interrupted */
		dev_err(g1->dev,
			"%s  interrupted while waiting for recycled frames\n",
			ctx->name);
		goto err;
	}
	if (ctx->state == G1_STATE_STOPPED) {
		/* stopping */
		dev_dbg(g1->dev,
			"%s  stopped while waiting for recycled frames\n",
			ctx->name);
		ret = -ENODATA;
		goto err;
	}
	if (ctx->recycled_frames <= 0) {
		/* no recycled frames, condition may have changed while
		 * unlocked, retry
		 */
		dev_warn(g1->dev,
			 "%s  exit of recycled frames wait point but no frames, retrying...\n",
			 ctx->name);
		goto retry;
	}

	/* reinitialize exit condition for next call */
	ctx->recycled_frames = 0;

	return 0;

err:
	return ret;
}

bool g1_is_free_frame_available(struct g1_ctx *ctx, struct g1_frame **pframe)
{
	unsigned int i;

	*pframe = NULL;
	/* Try to find a free frame */
	for (i = 0; i < ctx->nb_of_frames; i++) {
		if (ctx->frames[i]->state == G1_FRAME_FREE) {
			*pframe = ctx->frames[i];
			return true;
		}
	}
	return false;
}

int g1_get_free_frame(struct g1_ctx *ctx, struct g1_frame **pframe)
{
	struct g1_dev *g1 = ctx->dev;
	unsigned char str[100] = "";
	unsigned int tries = 3;
	unsigned int i;
	int ret = 0;

	*pframe = NULL;
	if (ctx->nb_of_frames <= 0) {
		dev_err(g1->dev,
			"%s  no output frame registered\n",
			ctx->name);
		/* No frame is registered; go out */
		return -EIO;
	}

	while (!g1_is_free_frame_available(ctx, pframe)) {
		if (tries <= 0) {
			dev_err(g1->dev,
				"%s  no output frame available @ frame %d, access unit is dropped\n",
				ctx->name,
				ctx->decoded_frames);

			/* dump frames status */
			for (i = 0; i < ctx->nb_of_frames; i++)
				dev_info(g1->dev,
					 "%s frame[%d] %s\n",
					 ctx->name, i,
					 frame_state_str(ctx->frames[i]->state,
							 str, sizeof(str)));

			return -ETIMEDOUT;
		}
		tries--;

		dev_dbg(g1->dev,
			"%s  no output frame available, wait for recycle\n",
			ctx->name);

		/* block till frames recycled */
		ret = g1_wait_recycle(ctx);
		if (ret)
			return ret;
	}
	return 0;
}

void g1_recycle(struct g1_ctx *ctx, struct g1_frame *frame)
{
	/* this frame is no more output */
	frame->state &= ~G1_FRAME_OUT;

	/* reset other frame fields */
	frame->flags = 0;
	frame->ts = 0;
	frame->to_drop = false;

	ctx->recycled_frames++;
	wake_up(&ctx->wq_recycle);
}

static void g1_vb2_au_queue(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;
	const struct g1_dec *dec = ctx->dec;
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct g1_au *au = to_au(vbuf);
	struct g1_frame *frame = NULL;
	unsigned char str[100] = "";
	int ret = 0;
	bool discard = false;
	bool loop = false;
	bool au_ts_already_pushed = false;
	bool flush_required = false;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	au->state &= ~G1_AU_OUT;

	if (ctx->state == G1_STATE_STOPPED ||
	    ctx->state == G1_STATE_FATAL_ERROR)
		goto out;

	/* just ignore empty buffers.
	 * this is needed when setting-up
	 * input stream queue; they have been queued
	 * by user with 0-size after each QUERYBUF,
	 * and driver just have to ignore them
	 * in order that they can be dequeued by user
	 * later to put the stream access unit data.
	 */
	if (au->size == 0)
		goto out;

	/* avoid too large au size */
	if (au->size > ctx->max_au_size) {
		dev_err(g1->dev,
			"%s too large au size, got %d while less than %d expected\n",
			ctx->name, au->size, ctx->max_au_size);
		ret = -EINVAL;
		goto err;
	}

	/* update timestamp (from user) :
	 * two options depending on ctx->timestamp_type
	 *   if timestamp_type = G1_DECODING_TIMESTAMP, then ts is DTS
	 *   else ts is PTS
	 */
	au->ts = vb->timestamp;

	/* picture id is used to do the association between a PTS and
	 * the decoded frame.
	 * picid is attached to the AU, send to G1 and retrieve attached
	 * to the decoded frame of this AU. (loop done inside the G1)
	 * so PTS of the AU is then retrieved using this picture id
	 * (as uniq id in a list) when we need to timestamp the decoded
	 * frame
	 * note : only useful in G1_PRESENTATION_TIMESTAMP
	 */
	au->picid = ctx->pic_counter++;

	/* reset number of remaining data to decode */
	au->remaining_data = au->size;

	if (!dec) {
		dev_err(g1->dev, "%s no decoder opened yet\n", ctx->name);
		ret = -EIO;
		goto err;
	}

	g1_perf_begin(ctx, au);

	/* dump access unit */
	dev_dbg(g1->dev, "%s dump %s", ctx->name,
		dump_au(ctx, au, str, sizeof(str)));

	ctx->recycled_frames = 0;

decode_data:
	/* decode this access unit */
	ret = dec->decode(ctx, au);

	/* if interrupted, ctx is potentially free-ed at this point (closing
	 * case), so exit right now without any ctx accesses
	 */
	if (ret == -EINTR) {
		dev_dbg(g1->dev, "%s g1_vb2_api <<<< %s(-EINTR)\n",
			ctx->name, __func__);
		return;
	}

	/* if stopped, return back current au to vb2 and return */
	if (ctx->state == G1_STATE_STOPPED)
		goto out;

	/* if the (-ENODATA) value is returned, it refers to the interlaced
	 * stream case for which 2 access units are needed to get 1 frame.
	 * So, this returned value doesn't mean that the decoding fails, but
	 * indicates that the timestamp information of the access unit shall
	 * not be taken into account, and that the V4L2 buffer associated with
	 * the access unit shall be flagged with V4L2_BUF_FLAG_ERROR to inform
	 * the user of this situation
	 */
	if (ret == -ENODATA) {
		discard = true;
	} else if (err_is_fatal(ret)) {
		goto fatal_err;
	} else if (ret == -EAGAIN) {
		dev_dbg(g1->dev,
			"%s Pending flush flush decoded frames\n",
			ctx->name);
		flush_required = true;
	} else if (ret < 0) {
		dev_dbg(g1->dev, "%s dec->decode failed (%d)\n",
			ctx->name, ret);
		goto err;
	}

	/* get infos from stream if not yet done */
	if (!(ctx->flags & G1_FLAG_STREAMINFO)) {
		struct g1_streaminfo *streaminfo = &ctx->streaminfo;

		ret = dec->get_streaminfo(ctx, streaminfo);
		if (ret) {
			/* infos not yet available, will retry on next stream
			 * au enqueued till stream header decoded.
			 * 2 valid cases can be encountered:
			 * 1) live streaming; first enqueued au may not be
			 *   the header, so each au must be decoded waiting
			 *   for header
			 * 2) corrupted or unsupported stream, in that case
			 *   header may never been recognized. FIXME put
			 *   in place a guard to not parse all bistream
			 *   in that case (warn to conflict with case 1)) !
			 */
			dev_dbg_ratelimited(g1->dev,
					    "%s valid stream header not yet discovered, more video stream needed to be enqueued\n",
					    ctx->name);
			goto out;
		}
		ctx->flags |= G1_FLAG_STREAMINFO;
		ctx->state = G1_STATE_READY;

		dev_info(g1->dev, "%s %s%s\n", ctx->name,
			 (ctx->mode == G1_MODE_LOW_LATENCY  &&
			  ctx->streaminfo.streamformat == V4L2_PIX_FMT_H264 ?
			  "[Low Latency mode] " : ""),
			 g1_streaminfo_str(streaminfo, str, sizeof(str)));

		/* Force remaining_data to 0 : in case of first header detection
		 * AU has to be resent directly from application side)
		 */
		au->remaining_data = 0;
	}

	/* push au timestamp in FIFO */
	if ((!discard || ctx->timestamp_type == G1_PRESENTATION_TIMESTAMP) &&
	    !au_ts_already_pushed) {
		g1_push_ts(ctx, au->ts, au->picid);
		au_ts_already_pushed = true;
	}

	/* try to get frame if and only if at least one frame
	 * has been decoded
	 */
	if (ctx->decoded_frames <= 0)
		goto out;

	loop = flush_required;

#if !defined G1_H264_USE_DISPLAY_SMOOTHING && defined CONFIG_VIDEO_ST_G1_H264
	/*
	 * - it means that it's mandatory to read frames while there are
	 * available.
	 * - it consumes less memory BUT side effect is that AU decode is
	 * blocked while every frames are not copied into output frame buffers
	 * (so depending on their availability) : to avoid that it means that
	 * number of output buffer must be similar to DPB number of frames.
	 */
	if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_H264)
		loop = true;
#else
	/*
	 * - Driver may read only one frame even if several frames are
	 * available, we only get and push one frame.
	 * Remaining decoded frames will be pushed out on the drain.
	 * - Memory consumption is higher as stack doubles number of DPB buffers
	 */
#endif

	if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_H264 &&
	    ctx->mode == G1_MODE_LOW_LATENCY)
		loop = true;

	do {
		ret = dec->get_frame(ctx, &frame);
		if (err_is_fatal(ret))
			goto fatal_err;
		if (ret)
			goto out;

		if (!frame) {
			dev_err(g1->dev,
				"%s  dec->get_frame returned NULL frame\n",
				ctx->name);
			ret = -EIO;
			goto out;
		}
		/* pop timestamp and mark frame with it */
		g1_pop_ts(ctx, &frame->ts, frame->picid);

		if (!frame->to_drop) {
			/* release decoded frame to user only if no error
			 * has been reported.
			 */
			g1_frame_done(ctx, frame, 0);
		} else {
			dev_info(g1->dev, "Drop frame\n");
		}
		frame->to_drop = false;
	} while (loop);
out:
	if (au->remaining_data && flush_required) {
		dev_dbg(g1->dev,
			"%s Retry to decode : %d remains out of %d to decode\n",
			ctx->name, au->remaining_data, au->size);
		goto decode_data;
	}
	g1_au_done(ctx, au, (discard ? -ENODATA : 0));
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return;

fatal_err:
	dev_err(g1->dev, "%s Fatal error (%d)\n",
		ctx->name, ret);
	ctx->state = G1_STATE_FATAL_ERROR;
err:
	g1_au_done(ctx, au, ret);
	dev_dbg(g1->dev, "%s g1_vb2_api <<<< %s(%d)\n",
		ctx->name, __func__, ret);
}

static void g1_flush_ts(struct g1_ctx *ctx)
{
	struct g1_timestamp *ts;
	struct g1_timestamp *next;

	/* protected by global lock acquired
	 * by V4L2 when calling g1_vb2_au_queue
	 */

	/* free all pending ts */
	list_for_each_entry_safe(ts, next, &ctx->timestamps, list) {
		list_del(&ts->list);
		kfree(ts);
	}

	/* reset list */
	INIT_LIST_HEAD(&ctx->timestamps);
}

static void g1_vb2_au_stop_streaming(struct vb2_queue *q)
{
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;
	struct g1_au *au;
	unsigned int i;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	g1_flush_ts(ctx);

	/* force return of all pending
	 * buffers to vb2 (in error state)
	 */
	for (i = 0; i < ctx->nb_of_aus; i++) {
		au = ctx->aus[i];
		if (!(au->state & G1_AU_OUT)) {
			au->state |= G1_AU_OUT;
			vb2_buffer_done(&au->vbuf.vb2_buf, VB2_BUF_STATE_ERROR);
		}
	}

	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
}

static int g1_enum_fmt_frame(struct file *file, void *fh,
			     struct v4l2_fmtdesc *f)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = video_drvdata(file);
	int output_nb = 0;
	int j = 0;
	int i = 0;

	LOG_G1_API_ENTRY(ctx, g1->dev, "cap");
	while (j < g1->nb_of_decoders) {
		for (i = 0; i < g1->decoders[j]->nb_pixels; i++) {
			if (f->index == output_nb) {
				f->pixelformat =
					g1->decoders[j]->pixelformat[i];
				snprintf(f->description,
					 sizeof(f->description),
					 "%4.4s", (char *)&f->pixelformat);
				LOG_G1_API_EXIT_MSG(ctx, g1->dev, "cap",
						    f->description);
				return 0;
			}
			output_nb++;
		}
		j++;
	}
	LOG_G1_API_EXIT_MSG(ctx, g1->dev, "cap", "-EINVAL");
	return -EINVAL;
}

static int g1_vb2_au_queue_init(struct vb2_buffer *vb)
{
	struct mem_info mem_info;
	struct g1_ctx *ctx = container_of(vb->vb2_queue, struct g1_ctx,
					     q_aus);
	struct g1_dev *g1 = ctx->dev;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	mem_info.size = vb->planes[0].length;
	snprintf(mem_info.name, sizeof(mem_info.name), "access unit");

	g1_debugfs_track_mem(&mem_info, ctx, "vb2");

	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return 0;
}

static int g1_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct g1_ctx *ctx = container_of(ctrl->handler, struct g1_ctx,
						ctrl_handler);
	struct g1_dev *g1 = ctx->dev;
	struct g1_frameinfo *frameinfo = &ctx->frameinfo;
	char msg[64];

	if (ctrl->flags & V4L2_CTRL_FLAG_INACTIVE)
		return 0;

	snprintf(msg, sizeof(msg), "CTRL:%d, VAL:%d", ctrl->id, ctrl->val);
	LOG_G1_API_ENTRY_MSG(ctx, g1->dev, "gen", msg);

	switch (ctrl->id) {
	case V4L2_CID_HFLIP:
		frameinfo->ctrls.is_hflip_requested = (ctrl->val != 0);
		break;
	case V4L2_CID_VFLIP:
		frameinfo->ctrls.is_vflip_requested = (ctrl->val != 0);
		break;
	case V4L2_CID_BRIGHTNESS:
		frameinfo->ctrls.brightness = ctrl->val;
		break;
	case V4L2_CID_CONTRAST:
		frameinfo->ctrls.contrast = ctrl->val;
		break;
	case V4L2_CID_SATURATION:
		frameinfo->ctrls.saturation = ctrl->val;
		break;
	case V4L2_CID_ALPHA_COMPONENT:
		frameinfo->ctrls.alpha = (u32)(ctrl->val);
		break;
	case V4L2_CID_USER_STA_VPU_LOW_LATENCY:
		if (ctx->mode != G1_MODE_NOT_SET) {
			dev_err(g1->dev, "Latency can't be updated twice\n");
			return -EINVAL;
		}
		ctx->mode = (ctrl->val != 0 ?
			     G1_MODE_LOW_LATENCY : G1_MODE_NORMAL);
		break;
	default:
		dev_err(g1->dev, "unknown control %d\n", ctrl->id);
		dev_dbg(g1->dev, "g1_api <<<< %s(-EINVAL)\n", __func__);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, "gen", "-EINVAL");
		return -EINVAL;
	}

	LOG_G1_API_EXIT(ctx, g1->dev, "gen");
	return 0;
}

static const struct v4l2_ctrl_ops g1_c_ops = {
	.s_ctrl = g1_s_ctrl,
};

static const struct v4l2_ctrl_config g1_low_latency_ctrl = {
	.ops = &g1_c_ops,
	.id = V4L2_CID_USER_STA_VPU_LOW_LATENCY,
	.name = "low-latency",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.min = 0,
	.max = 1,
	.step = 1,
	.def = 0,
};

static int g1_ctrls_create(struct g1_ctx *ctx)
{
	if (ctx->ctrls.is_configured)
		return 0;

	v4l2_ctrl_handler_init(&ctx->ctrl_handler, G1_MAX_CTRL_NUM);

	ctx->ctrls.hflip = v4l2_ctrl_new_std(&ctx->ctrl_handler,
				&g1_c_ops, V4L2_CID_HFLIP, 0, 1, 1, 0);
	ctx->ctrls.vflip = v4l2_ctrl_new_std(&ctx->ctrl_handler,
				&g1_c_ops, V4L2_CID_VFLIP, 0, 1, 1, 0);

	ctx->ctrls.contrast = v4l2_ctrl_new_std(&ctx->ctrl_handler,
				&g1_c_ops, V4L2_CID_CONTRAST,
				-64, 64, 1, 0);

	ctx->ctrls.brightness = v4l2_ctrl_new_std(&ctx->ctrl_handler,
				&g1_c_ops, V4L2_CID_BRIGHTNESS,
				-128, 127, 1, 0);

	ctx->ctrls.saturation = v4l2_ctrl_new_std(&ctx->ctrl_handler,
				&g1_c_ops, V4L2_CID_SATURATION,
				-64, 128, 1, 0);

	ctx->ctrls.alpha = v4l2_ctrl_new_std(&ctx->ctrl_handler,
				&g1_c_ops, V4L2_CID_ALPHA_COMPONENT,
				0, 255, 1, 0);

	ctx->ctrls.low_latency_mode = v4l2_ctrl_new_custom(&ctx->ctrl_handler,
					&g1_low_latency_ctrl, NULL);

	if (ctx->ctrl_handler.error) {
		int err = ctx->ctrl_handler.error;

		v4l2_ctrl_handler_free(&ctx->ctrl_handler);
		return err;
	}

	ctx->ctrls.is_configured = true;

	return 0;
}

static void g1_ctrls_delete(struct g1_ctx *ctx)
{
	if (ctx->ctrls.is_configured) {
		v4l2_ctrl_handler_free(&ctx->ctrl_handler);
		ctx->ctrls.is_configured = false;
	}
}

int g1_g_crop(struct file *file, void *fh, struct v4l2_crop *a)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	struct g1_frameinfo *frameinfo = &ctx->frameinfo;
	char video_type[5];

	strncpy(video_type,
		(a->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);

	if (a->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) {
		dev_err(g1->dev,
			"%s V4L2 G_CROP: only V4L2_BUF_TYPE_CAPTURE is supported\n",
			ctx->name);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EINVAL");
		return -EINVAL;
	}

	if (!(ctx->flags & G1_FLAG_FRAMEINFO)) {
		/* limit rate because of potential user polling on
		 * availability while stream header is not yet recognized
		 * by decoder
		 */
		dev_warn_ratelimited(g1->dev,
				     "%s valid stream header not yet discovered, more video stream needed to be enqueued\n",
				     ctx->name);
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EBUSY");
		return -EBUSY;
	}

	if (frameinfo->flags & G1_FRAMEINFO_FLAG_CROP) {
		a->c = frameinfo->crop;
	} else {
		/* default to current output frame dimensions */
		a->c.left = 0;
		a->c.top = 0;
		a->c.width = frameinfo->width;
		a->c.height = frameinfo->height;
	}
	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_g_selection(struct file *file, void *fh,
			  struct v4l2_selection *s)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	const struct g1_dec *dec = ctx->dec;
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;
	struct v4l2_crop c;
	char video_type[5];
	char msg[64];

	if (!dec) {
		dev_err(g1->dev, "decoder not yet opened");
		return -EINVAL;
	}

	strncpy(video_type,
		(s->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	LOG_G1_API_ENTRY(ctx, g1->dev, video_type);

	/* get infos from stream if not yet done */
	if (dec->get_streaminfo) {
		if (dec->get_streaminfo(ctx, streaminfo)) {
			LOG_G1_API_EXIT_MSG(ctx, g1->dev,
					    video_type, "-EINVAL");
			return -EINVAL;
		}
	}

	switch (s->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		switch (s->target) {
		case V4L2_SEL_TGT_CROP:
			/* cropped frame */
			if (streaminfo->flags & G1_STREAMINFO_FLAG_CROP) {
				s->r = streaminfo->crop;
			} else {
				s->r.width = streaminfo->width;
				s->r.height = streaminfo->height;
				s->r.left = 0;
				s->r.left = 0;
			}
			break;
		case V4L2_SEL_TGT_CROP_DEFAULT:
		case V4L2_SEL_TGT_CROP_BOUNDS:
			/* complete frame */
			s->r.left = 0;
			s->r.top = 0;
			s->r.width = streaminfo->width;
			s->r.height = streaminfo->height;
			break;
		default:
			dev_err(g1->dev, "Invalid target\n");
			LOG_G1_API_EXIT_MSG(ctx, g1->dev,
					    video_type, "-EINVAL");
			return -EINVAL;
		}
		break;

	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		/* composed (cropped) frame */
		c.type = s->type;
		g1_g_crop(file, fh, &c);
		s->r = c.c;
		break;

	default:
		dev_err(g1->dev, "Invalid type\n");
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EINVAL");
		return -EINVAL;
	}

	snprintf(msg, sizeof(msg), "%dx%d left:%d top:%d",
		 s->r.width, s->r.height, s->r.left, s->r.top);
	LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, msg);
	return 0;
}

static int g1_s_selection(struct file *file, void *fh,
			  struct v4l2_selection *s)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	bool valid = false;
	char video_type[5];
	char msg[64];

	strncpy(video_type,
		(s->type == V4L2_BUF_TYPE_VIDEO_OUTPUT ? "out" : "cap"),
		sizeof(video_type));

	snprintf(msg, sizeof(msg), "%dx%d left:%d top:%d",
		 s->r.width, s->r.height, s->r.left, s->r.top);
	LOG_G1_API_ENTRY_MSG(ctx, g1->dev, video_type, msg);

	if ((s->type == V4L2_BUF_TYPE_VIDEO_OUTPUT) &&
	    (s->target == V4L2_SEL_TGT_CROP)) {
		valid = true;
		/* store crop request coming from user */
		ctx->streaminfo.flags |= G1_STREAMINFO_FLAG_CROP;
		memcpy(&ctx->streaminfo.crop, &s->r,
		       sizeof(struct v4l2_rect));
	}

	if ((s->type == V4L2_BUF_TYPE_VIDEO_CAPTURE) &&
	    (s->target == V4L2_SEL_TGT_COMPOSE)) {
		valid = true;
		/* store crop request coming from user */
		ctx->frameinfo.flags |= G1_FRAMEINFO_FLAG_CROP;
		memcpy(&ctx->frameinfo.crop, &s->r,
		       sizeof(struct v4l2_rect));
	}

	if (!valid) {
		dev_err(g1->dev, "Invalid type / target\n");
		LOG_G1_API_EXIT_MSG(ctx, g1->dev, video_type, "-EINVAL");
		return -EINVAL;
	}
	g1_update_fmt_info(ctx, &ctx->streaminfo, &ctx->frameinfo);
	ctx->max_au_size = frame_size(ctx->streaminfo.aligned_width,
				      ctx->streaminfo.aligned_height,
				      ctx->streaminfo.streamformat);
	if (!ctx->max_au_size)
		ctx->max_au_size = G1_MAX_RESO;

	LOG_G1_API_EXIT(ctx, g1->dev, video_type);
	return 0;
}

static int g1_try_fmt_frame(struct file *file, void *fh,
			    struct v4l2_format *f)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	u32 pixelformat = pix->pixelformat;
	u32 width, height;
	char msg[64];

	snprintf(msg, sizeof(msg), "%dx%d %4.4s",
		 pix->width, pix->height,
		 (char *)&pix->pixelformat);
	LOG_G1_API_ENTRY_MSG(ctx, g1->dev, "cap", msg);

	width = pix->width;
	height = pix->height;

	/* pix->width and pix->height are updated to report 16 pixels
	 * alignement needs to the user
	 */
#if defined G1_FRAME_FIRST_ALIGNED_VALUE_BELOW
	if (pix->width & 0x0F)
		pix->width &= ~0xF;

	if (pix->height & 0x07)
		pix->height &= ~0x7;
#else
	v4l_bound_align_image(&pix->width,
			      G1_MIN_WIDTH, G1_MAX_WIDTH,
			      4, /* 2^4 pixels : PP constraint */
			      &pix->height,
			      G1_MIN_HEIGHT, G1_MAX_HEIGHT,
			      4, /* 2^4 pixels : PP constraint */
			      0);
#endif
	if ((pix->width != width) || (pix->height != height))
		dev_dbg(g1->dev,
			"%s V4L2 TRY_FMT (CAPTURE): resolution updated %dx%d -> %dx%d to fit decoder alignment\n",
			ctx->name, width, height, pix->width, pix->height);

	if (!pix->colorspace)
		pix->colorspace = V4L2_COLORSPACE_REC709;

	pix->bytesperline = frame_stride(pix->width, pixelformat);
	pix->sizeimage = frame_size(pix->width, pix->height, pixelformat);

	if (pix->field == V4L2_FIELD_ANY)
		pix->field = V4L2_FIELD_NONE;

	LOG_G1_API_EXIT(ctx, g1->dev, "cap");
	return 0;
}

static int g1_g_fmt_frame(struct file *file, void *fh, struct v4l2_format *f)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	const struct g1_dec *dec = ctx->dec;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct g1_frameinfo *frameinfo = &ctx->frameinfo;
	unsigned char str[100] = "";

	LOG_G1_API_ENTRY(ctx, g1->dev, "cap");
	if (dec) {
		if (dec->get_frameinfo(ctx, frameinfo)) {
			dev_dbg(g1->dev,
				"%s V4L2 GET_FMT (CAPTURE): no frame information available\n",
				ctx->name);
			LOG_G1_API_EXIT_MSG(ctx, g1->dev, "cap", "-EBUSY");
			return -EBUSY;
		}
		ctx->flags |= G1_FLAG_FRAMEINFO;
	}

	if (!(ctx->flags & G1_FLAG_FRAMEINFO))
		dev_dbg(g1->dev,
			"%s V4L2 GET_FMT (CAPTURE): no frame information available, default to %s\n",
			ctx->name,
			g1_frameinfo_str(frameinfo, str, sizeof(str)));

	pix->pixelformat = frameinfo->pixelformat;
	pix->width = frameinfo->aligned_width;
	pix->height = frameinfo->aligned_height;
	pix->field = frameinfo->field;
	pix->bytesperline = frame_stride(frameinfo->aligned_width,
					       frameinfo->pixelformat);
	pix->sizeimage = frame_size(frameinfo->aligned_width,
				    frameinfo->aligned_height,
				    frameinfo->pixelformat);

	pix->colorspace = frameinfo->colorspace;
	snprintf(str, sizeof(str), "%dx%d %4.4s",
		 pix->width, pix->height,
		 (char *)&pix->pixelformat);
	LOG_G1_API_EXIT_MSG(ctx, g1->dev, "cap", str);
	return 0;
}

static int g1_s_fmt_frame(struct file *file, void *fh, struct v4l2_format *f)
{
	struct g1_ctx *ctx = to_ctx(fh);
	struct g1_dev *g1 = ctx->dev;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct g1_frameinfo frameinfo;

	LOG_G1_API_ENTRY(ctx, g1->dev, "cap");
	g1_try_fmt_frame(file, fh, f);
	/* set frame information to decoder */
	memset(&frameinfo, 0, sizeof(frameinfo));
	frameinfo.pixelformat = pix->pixelformat;
	frameinfo.width = pix->width;
	frameinfo.height = pix->height;
	frameinfo.aligned_width = pix->width;
	frameinfo.aligned_height = pix->height;
	frameinfo.deinterlace_activable = true;

	/* In some cases, rotation request might have been requested
	 * before the s_fmt request. So we retrieve it from local struct
	 * instead of re-init it to false.
	 */
	frameinfo.ctrls.is_hflip_requested =
		ctx->frameinfo.ctrls.is_hflip_requested;
	frameinfo.ctrls.is_vflip_requested =
		ctx->frameinfo.ctrls.is_vflip_requested;
	frameinfo.ctrls.contrast = ctx->frameinfo.ctrls.contrast;
	frameinfo.ctrls.brightness = ctx->frameinfo.ctrls.brightness;
	frameinfo.ctrls.saturation = ctx->frameinfo.ctrls.saturation;
	frameinfo.ctrls.alpha = ctx->frameinfo.ctrls.alpha;

	/* PP output is systematically set to progressive by default */
	frameinfo.field = V4L2_FIELD_NONE;
	if (!g1_update_fmt_info(ctx, &ctx->streaminfo, &frameinfo))
		ctx->flags |= G1_FLAG_FRAMEINFO;
	g1_g_fmt_frame(file, fh, f);
	LOG_G1_API_EXIT(ctx, g1->dev, "cap");
	return 0;
}

static void g1_forward_eos_info(struct g1_ctx *ctx, struct g1_frame *frame)
{
	struct g1_dev *g1 = ctx->dev;
	const struct v4l2_event ev = {
			.type = V4L2_EVENT_EOS
	};

	/* EOS is transferred :
	 * - by returning an empty frame flagged to V4L2_BUF_FLAG_LAST
	 * - and then send EOS event
	 */

	/* set the last buffer flag */
	frame->flags |= V4L2_BUF_FLAG_LAST;
	frame->vbuf.vb2_buf.planes[0].bytesused = 0;

	/* release frame to user */
	vb2_buffer_done(&frame->vbuf.vb2_buf, VB2_BUF_STATE_DONE);

	v4l2_event_queue_fh(&ctx->fh, &ev);
	dev_dbg(g1->dev, "%s EOS event queued\n",
		ctx->name);
	ctx->state = G1_STATE_EOS;
}

static int g1_decoder_stop_cmd(struct g1_ctx *ctx, void *fh)
{
	const struct g1_dec *dec = ctx->dec;
	struct g1_dev *g1 = ctx->dev;
	struct g1_frame *frame = NULL;
	int ret = 0;

	LOG_G1_API_ENTRY(ctx, g1->dev, "gen");
	if (!dec)
		goto out;

	/* drain the decoder */
	dec->drain(ctx);

	/* release to user drained frames */
	while (1) {
		frame = NULL;
		ret = dec->get_frame(ctx, &frame);
		if (ret == -ENODATA) {
			break; /* no more decoded frames */
		}
		if (ret < 0) {
			dev_err(g1->dev,
				"Unable to drain; abort it\n");
			break;
		}
		if (frame && !frame->to_drop) {
			/* pop timestamp and mark frame with it */
			g1_pop_ts(ctx, &frame->ts, frame->picid);

			/* release decoded frame to user */
			g1_frame_done(ctx, frame, 0);
		}
	}

	/* EOS management */
	if (g1_is_free_frame_available(ctx, &frame)) {
		ret = g1_get_free_frame(ctx, &frame);
		if (ret)
			goto out;
		/* - every pending frames have been pushed,
		 * - there is a available free frame
		 * so EOS can now be completed
		 */
		frame->state |= G1_FRAME_OUT;
		g1_forward_eos_info(ctx, frame);
		LOG_G1_API_EXIT(ctx, g1->dev, "gen");
		return 0;
	}
out:
	/* at this point it seems that there is no free empty frame available.
	 * EOS completion is so delayed till next frame_queue() call
	 * to be sure to have a free empty frame available.
	 */
	ctx->state = G1_STATE_WF_EOS;
	LOG_G1_API_EXIT(ctx, g1->dev, "gen");
	return ret;
}

int g1_decoder_cmd(struct file *file, void *fh, struct v4l2_decoder_cmd *cmd)
{
	struct g1_ctx *ctx = to_ctx(fh);

	switch (cmd->cmd) {
	case V4L2_DEC_CMD_STOP:
		if (cmd->flags != 0)
			return -EINVAL;

		g1_decoder_stop_cmd(ctx, fh);

		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int g1_subscribe_event(struct v4l2_fh *fh,
			      const struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_EOS:
		return v4l2_event_subscribe(fh, sub, 2, NULL);
	default:
		return -EINVAL;
	}

	return 0;
}

static int g1_vb2_frame_queue_setup(struct vb2_queue *q,
				    unsigned int *num_buffers,
				    unsigned int *num_planes,
				    unsigned int sizes[],
				    struct device *alloc_devs[])
{
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;
	struct g1_frameinfo frameinfo;
	struct g1_streaminfo streaminfo;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	frameinfo = ctx->frameinfo;
	streaminfo = ctx->streaminfo;

	/* single plane for Y and CbCr */
	*num_planes = 1;

	/* the number of output buffers needed for decoding =
	 * user need (*num_buffers given, usually for display pipeline) +
	 * decoding peak smoothing (depends on G1 IP perf)
	 * dpb value isn't taken into account because with a PP
	 * systematically enabled : G1 allocates it
	 */
	if (*num_buffers < G1_MIN_FRAME_USER) {
		dev_warn(g1->dev,
			 "%s num_buffers too low (%d), increasing to %d\n",
			 ctx->name, *num_buffers, G1_MIN_FRAME_USER);
		*num_buffers = G1_MIN_FRAME_USER;
	}

	*num_buffers += G1_PEAK_FRAME_SMOOTHING;

	if (ctx->mode == G1_MODE_LOW_LATENCY)
		*num_buffers += streaminfo.dpb;

	if (*num_buffers > G1_MAX_FRAMES) {
		dev_warn(g1->dev,
			 "%s Output frame count too high (%d), cut to %d\n",
			 ctx->name, *num_buffers, G1_MAX_FRAMES);
		*num_buffers = G1_MAX_FRAMES;
	}

	ctx->allocated_frames = *num_buffers;

	if (sizes[0])
		dev_warn(g1->dev, "%s psize[0] already set to %d\n",
			 ctx->name, sizes[0]);
	if (alloc_devs[0])
		dev_warn(g1->dev, "%s allocators[0] already set\n", ctx->name);

	sizes[0] = frame_size(frameinfo.aligned_width,
			      frameinfo.aligned_height,
			      frameinfo.pixelformat);

	vb2_dma_contig_set_max_seg_size(g1->dev, DMA_BIT_MASK(32));

	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return 0;
}

static int g1_vb2_frame_prepare(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;
	const struct g1_dec *dec = ctx->dec;
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct g1_frame *frame = to_frame(vbuf);
	int ret = 0;
	struct g1_frameinfo *frameinfo = &ctx->frameinfo;
	unsigned int size;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);

	if (!frame->prepared) {
		frame->index = frame->vbuf.vb2_buf.index;
		frame->vaddr = vb2_plane_vaddr(&frame->vbuf.vb2_buf, 0);
		frame->paddr =
			vb2_dma_contig_plane_dma_addr(&frame->vbuf.vb2_buf, 0);

		frame->info = frameinfo;
		size = frame_size(frameinfo->aligned_width,
				  frameinfo->aligned_height,
				  frameinfo->pixelformat);
		frame->pix.sizeimage = size;
		vb2_set_plane_payload(&frame->vbuf.vb2_buf, 0, size);

		if (dec)
			ret = dec->setup_frame(ctx, frame);
		if (ret) {
			dev_err(g1->dev,
				"%s  dec->setup_frame failed (%d)\n",
				ctx->name, ret);
			dev_dbg(g1->dev, "%s g1_vb2_api <<<< %s(%d)\n",
				ctx->name, __func__, ret);
			return ret;
		}
		frame->prepared = 1;
	}
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return 0;
}

static void g1_vb2_frame_finish(struct vb2_buffer *vb)
{
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct g1_frame *frame = to_frame(vbuf);

	/* update V4L2 timestamp for user */
	vb->timestamp = frame->ts;

	/* update V4L2 field for user */
	vbuf->field = frame->pix.field;

	/* update V4L2 frame flags for user */
	vbuf->flags = frame->flags;
}

static void g1_vb2_frame_queue(struct vb2_buffer *vb)
{
	struct vb2_queue *q = vb->vb2_queue;
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;
	const struct g1_dec *dec = ctx->dec;
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct g1_frame *frame = to_frame(vbuf);

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	if (ctx->state == G1_STATE_WF_EOS) {
		/* new frame available, EOS can now be completed */
		g1_forward_eos_info(ctx, frame);
		/* return, no need to recycle this buffer to decoder */
		dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
		return;
	}
	/* recycle this frame */
	if (dec)
		dec->recycle(ctx, frame);
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
}

static int g1_vb2_frame_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	struct g1_dev *g1 = ctx->dev;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	if (ctx->state == G1_STATE_STOPPED)
		ctx->state = G1_STATE_READY;
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return 0;
}

static void g1_vb2_frame_stop_streaming(struct vb2_queue *q)
{
	struct g1_ctx *ctx = to_ctx(q->drv_priv);
	const struct g1_dec *dec = ctx->dec;
	struct g1_dev *g1 = ctx->dev;
	struct g1_frame *frame;
	int ret;
	unsigned int i;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	/* cancel ongoing decoding (if any, stuck at g1_wait_recycle) */
	ctx->state = G1_STATE_STOPPED;
	wake_up(&ctx->wq_recycle);

	if (dec) {
		/* flush remaining frames */
		while (1) {
			frame = NULL;
			ret = dec->get_frame(ctx, &frame);
			if (ret == -ENODATA)
				break; /* no more decoded frames */
			if (ret < 0) {
				dev_err(g1->dev,
					"Unable to flush; abort it\n");
				goto force_release_buffer;
			}
		}
		g1_flush_ts(ctx);

		dec->flush(ctx);
	}

force_release_buffer:
	/* force return of all pending
	 * buffers to vb2 (in error state)
	 */
	for (i = 0; i < ctx->nb_of_frames; i++) {
		frame = ctx->frames[i];
		if (!(frame->state & G1_FRAME_OUT)) {
			frame->state |= G1_FRAME_OUT;
			vb2_buffer_done(&frame->vbuf.vb2_buf,
					VB2_BUF_STATE_ERROR);
		}
	}
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
}

static int g1_vb2_frame_queue_init(struct vb2_buffer *vb)
{
	struct mem_info mem_info;
	struct g1_ctx *ctx = container_of(vb->vb2_queue, struct g1_ctx,
					     q_frames);
	struct g1_frameinfo *frameinfo = &ctx->frameinfo;
	struct g1_dev *g1 = ctx->dev;

	dev_dbg(g1->dev, "%s g1_vb2_api > %s\n", ctx->name, __func__);
	mem_info.size = vb->planes[0].length;

	/* check if memory resources have been asked by g1 or not */
	if (vb->vb2_queue->memory != V4L2_MEMORY_MMAP) {
		dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
		return 0;
	}

	snprintf(mem_info.name, sizeof(mem_info.name), "frame %4.4s %dx%d",
		 (char *)&frameinfo->pixelformat,
		 frameinfo->aligned_width, frameinfo->aligned_height);

	g1_debugfs_track_mem(&mem_info, ctx, "vb2");
	dev_dbg(g1->dev, "%s g1_vb2_api < %s\n", ctx->name, __func__);
	return 0;
}

static struct vb2_ops g1_vb2_au_ops = {
	.queue_setup = g1_vb2_au_queue_setup,
	.buf_prepare = g1_vb2_au_prepare,
	.buf_queue = g1_vb2_au_queue,
	.buf_init = g1_vb2_au_queue_init,
	.wait_prepare = g1_vb2_unlock,
	.wait_finish = g1_vb2_lock,
	.stop_streaming = g1_vb2_au_stop_streaming,
};

static struct vb2_ops g1_vb2_frame_ops = {
	.queue_setup = g1_vb2_frame_queue_setup,
	.buf_prepare = g1_vb2_frame_prepare,
	.buf_finish = g1_vb2_frame_finish,
	.buf_queue = g1_vb2_frame_queue,
	.buf_init = g1_vb2_frame_queue_init,
	.wait_prepare = g1_vb2_unlock,
	.wait_finish = g1_vb2_lock,
	.start_streaming = g1_vb2_frame_start_streaming,
	.stop_streaming = g1_vb2_frame_stop_streaming,
};

#define FRAME_DEFAULT_WIDTH  32
#define FRAME_DEFAULT_HEIGHT 32
#define STREAM_DEFAULT_WIDTH  1920
#define STREAM_DEFAULT_HEIGHT 1080

static void set_default_params(struct g1_ctx *ctx)
{
	struct g1_frameinfo *frameinfo = &ctx->frameinfo;
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;

	memset(frameinfo, 0, sizeof(*frameinfo));
	frameinfo->pixelformat = V4L2_PIX_FMT_NV12;
	frameinfo->width = FRAME_DEFAULT_WIDTH;
	frameinfo->height = FRAME_DEFAULT_HEIGHT;
	frameinfo->aligned_width = ALIGN(frameinfo->width, 8);
	frameinfo->aligned_height = ALIGN(frameinfo->height, 8);
	frameinfo->field = V4L2_FIELD_NONE;
	frameinfo->colorspace = V4L2_COLORSPACE_REC709;
	frameinfo->ctrls.is_hflip_requested = false;
	frameinfo->ctrls.is_vflip_requested = false;
	frameinfo->ctrls.contrast = 0;
	frameinfo->ctrls.brightness = 0;
	frameinfo->ctrls.saturation = 0;
	frameinfo->ctrls.alpha = 255;
	frameinfo->deinterlace_activable = true;

	memset(streaminfo, 0, sizeof(*streaminfo));
	streaminfo->streamformat = V4L2_PIX_FMT_NV12;
	streaminfo->width = STREAM_DEFAULT_WIDTH;
	streaminfo->height = STREAM_DEFAULT_HEIGHT;
	streaminfo->aligned_width = ALIGN(streaminfo->width, 8);
	streaminfo->aligned_height = ALIGN(streaminfo->height, 8);
	streaminfo->field = V4L2_FIELD_NONE;
	streaminfo->colorspace = V4L2_COLORSPACE_REC709;

	ctx->max_au_size = G1_MAX_AU_SIZE;
	ctx->mode = G1_MODE_NOT_SET;
}

static int g1_open(struct file *file)
{
	struct g1_dev *g1 = video_drvdata(file);
	struct g1_ctx *ctx = NULL;
	struct vb2_queue *q;
	int ret = 0;
	char msg[10];

	LOG_G1_API_ENTRY(ctx, g1->dev, "gen");
	mutex_lock(&g1->lock);

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		ret = -ENOMEM;
		goto err;
	}
	v4l2_fh_init(&ctx->fh, video_devdata(file));

	ret = g1_ctrls_create(ctx);
	if (ret) {
		dev_err(g1->dev, "Failed to create control\n");
		goto err_fh_del;
	}

	/* Use separate control handler per file handle */
	ctx->fh.ctrl_handler = &ctx->ctrl_handler;
	file->private_data = &ctx->fh;
	v4l2_fh_add(&ctx->fh);
	ctx->dev = g1;

	/* setup vb2 queue for stream input */
	q = &ctx->q_aus;
	q->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	q->drv_priv = &ctx->fh;
	q->io_modes = VB2_MMAP | VB2_DMABUF;
	/* overload vb2 buf with private au struct */
	q->buf_struct_size = sizeof(struct g1_au);
	q->ops = &g1_vb2_au_ops;
	q->mem_ops = (struct vb2_mem_ops *)&vb2_dma_contig_memops;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	/* Temporary allow usage of bytesused = 0 */
	q->allow_zero_bytesused = 1;
	q->dev = g1->dev;
	ret = vb2_queue_init(q);
	if (ret) {
		dev_err(g1->dev, "[x:x] vb2_queue_init(aus) failed (%d)\n",
			ret);
		goto error_ctrls;
	}

	/* setup vb2 queue for frame output */
	q = &ctx->q_frames;
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->drv_priv = &ctx->fh;
	q->io_modes = VB2_MMAP | VB2_DMABUF;
	/* overload vb2 buf with private frame struct */
	q->buf_struct_size = sizeof(struct g1_frame)
			     + G1_MAX_FRAME_PRIV_SIZE;
	q->ops = &g1_vb2_frame_ops;
	q->mem_ops = (struct vb2_mem_ops *)&vb2_dma_contig_memops;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	/* Temporary allow usage of bytesused = 0 */
	q->allow_zero_bytesused = 1;
	q->dev = g1->dev;
	ret = vb2_queue_init(q);
	if (ret) {
		dev_err(g1->dev,
			"[x:x] vb2_queue_init(frames) failed (%d)\n", ret);
		goto error_ctrls;/* no cleanup needed for vb2_queue_init */
	}

	/* wait stream format to determine which
	 * decoder to open, cf g1_s_fmt_stream.
	 */
	ctx->state = G1_STATE_WF_FORMAT;

	INIT_LIST_HEAD(&ctx->list);
	INIT_LIST_HEAD(&ctx->timestamps);
	ctx->pic_counter = 0;
	ctx->timestamp_type = G1_DEFAULT_TIMESTAMP;
	ctx->first_au.tv_sec = 0;
	ctx->first_au.tv_usec = 0;
	init_waitqueue_head(&ctx->wq_recycle);

	/* name this instance */
	g1->instance_id++;	/* rolling id to identify this instance */
	ctx->instance_id = g1->instance_id;
	snprintf(ctx->name, sizeof(ctx->name), "[%3d:----]", ctx->instance_id);
	/* default parameters for frame and stream */
	set_default_params(ctx);

	/* update g1 device */
	list_add_tail(&ctx->list, &g1->instances);
	g1->nb_of_instances++;

	g1_debugfs_open(ctx);

	mutex_unlock(&g1->lock);
	LOG_G1_API_EXIT(ctx, g1->dev, "gen");
	return 0;
error_ctrls:
	g1_ctrls_delete(ctx);
err_fh_del:
	v4l2_fh_del(&ctx->fh);
	v4l2_fh_exit(&ctx->fh);
	kfree(ctx);
err:
	mutex_unlock(&g1->lock);
	sprintf(msg, "%d", ret);
	LOG_G1_API_EXIT_MSG(NULL, g1->dev, "gen", msg);
	return ret;
}

static int g1_release(struct file *file)
{
	struct g1_ctx *ctx = to_ctx(file->private_data);
	struct g1_dev *g1 = ctx->dev;
	const struct g1_dec *dec = ctx->dec;
	unsigned char str[200] = "";

	LOG_G1_API_ENTRY(ctx, g1->dev, "gen");
	mutex_lock(&g1->lock);

	/* cancel ongoing decoding (if any, stuck at g1_wait_recycle) */
	ctx->state = G1_STATE_CLOSED;
	wake_up(&ctx->wq_recycle);

	/* close decoder */
	if (dec) {
		dec->close(ctx);
		ctx->dec = NULL;
	}

	/* trace a summary of instance
	 * before closing (debug purpose)
	 */
	if (ctx->flags & G1_FLAG_STREAMINFO)
		dev_info(g1->dev, "%s %s\n", ctx->name,
			 g1_summary_str(ctx, str, sizeof(str)));
	/* update g1 device */
	g1->nb_of_instances--;
	list_del(&ctx->list);

	vb2_queue_release(&ctx->q_frames);	/* will free dma memory
						 * of each frame in queue
						 */

	vb2_queue_release(&ctx->q_aus);	/* will free dma memory
					 * of each aus in queue
					 */

	g1_ctrls_delete(ctx);

	v4l2_fh_del(&ctx->fh);
	v4l2_fh_exit(&ctx->fh);

	g1_debugfs_close(ctx);

	LOG_G1_API_EXIT(ctx, g1->dev, "gen");
	kfree(ctx);
	mutex_unlock(&g1->lock);
	return 0;
}

static unsigned int g1_poll(struct file *file,
			    struct poll_table_struct *wait)
{
	struct g1_ctx *ctx = to_ctx(file->private_data);
	struct g1_dev *g1 = ctx->dev;
	struct vb2_queue *src_q, *dst_q;
	struct vb2_buffer *src_vb = NULL, *dst_vb = NULL;
	unsigned int rc = 0;

	src_q = &ctx->q_aus;
	dst_q = &ctx->q_frames;

	LOG_G1_API_ENTRY(ctx, g1->dev, "gen");
	mutex_lock(&g1->lock);
	/*
	 * There has to be at least one buffer queued on each queued_list, which
	 * means either in driver already or waiting for driver to claim it
	 * and start processing.
	 */
	if ((!vb2_is_streaming(src_q) || list_empty(&src_q->queued_list)) &&
	    (!vb2_is_streaming(dst_q) || list_empty(&dst_q->queued_list))) {
		rc = POLLERR;
		goto end;
	}
	mutex_unlock(&g1->lock);

	poll_wait(file, &ctx->fh.wait, wait);
	if (list_empty(&src_q->done_list))
		poll_wait(file, &src_q->done_wq, wait);
	if (list_empty(&dst_q->done_list))
		poll_wait(file, &dst_q->done_wq, wait);

	mutex_lock(&g1->lock);

	if (v4l2_event_pending(&ctx->fh))
		rc |= POLLPRI;

	if (!list_empty(&src_q->done_list))
		src_vb = list_first_entry(&src_q->done_list,
					  struct vb2_buffer,
					  done_entry);
	if (src_vb && (src_vb->state == VB2_BUF_STATE_DONE ||
		       src_vb->state == VB2_BUF_STATE_ERROR))
		rc |= POLLOUT | POLLWRNORM;

	if (!list_empty(&dst_q->done_list))
		dst_vb = list_first_entry(&dst_q->done_list,
					  struct vb2_buffer,
					  done_entry);
	if (dst_vb && (dst_vb->state == VB2_BUF_STATE_DONE ||
		       dst_vb->state == VB2_BUF_STATE_ERROR))
		rc |= POLLIN | POLLRDNORM;
end:
	mutex_unlock(&g1->lock);
	LOG_G1_API_EXIT(ctx, g1->dev, "gen");
	return rc;
}

static int g1_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct g1_ctx *ctx = to_ctx(file->private_data);
	struct g1_dev *g1 = ctx->dev;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	enum v4l2_buf_type type;
	int ret;

	LOG_G1_API_ENTRY(ctx, g1->dev, "gen");
	mutex_lock(&g1->lock);

	/* offset used to differentiate OUTPUT/CAPTURE */
	if (offset < MMAP_FRAME_OFFSET) {
		type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	} else {
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		vma->vm_pgoff -= (MMAP_FRAME_OFFSET >> PAGE_SHIFT);
	}

	/* vb2 call */
	ret = vb2_mmap(to_vb2q(ctx, type), vma);
	if (ret)
		dev_err(g1->dev, "%s vb2_mmap failed (%d)\n",
			ctx->name, ret);

	mutex_unlock(&g1->lock);
	LOG_G1_API_EXIT(ctx, g1->dev, "gen");
	return ret;
}

/* v4l2 ops */
static const struct v4l2_file_operations g1_fops = {
	.owner = THIS_MODULE,
	.open = g1_open,
	.release = g1_release,
	.unlocked_ioctl = video_ioctl2,
	.mmap = g1_mmap,
	.poll = g1_poll,
};

/* v4l2 ioctl ops */
static const struct v4l2_ioctl_ops g1_ioctl_ops = {
	.vidioc_querycap = g1_querycap,

	/* V4L2 "CAPTURE" = decoder frame output */
	.vidioc_enum_fmt_vid_cap = g1_enum_fmt_frame,
	.vidioc_g_fmt_vid_cap = g1_g_fmt_frame,
	.vidioc_try_fmt_vid_cap = g1_try_fmt_frame,
	.vidioc_s_fmt_vid_cap = g1_s_fmt_frame,

	/* V4L2 "OUTPUT" = decoder stream access units input */
	.vidioc_enum_fmt_vid_out = g1_enum_fmt_stream,
	.vidioc_g_fmt_vid_out = g1_g_fmt_stream,
	.vidioc_try_fmt_vid_out = g1_try_fmt_stream,
	.vidioc_s_fmt_vid_out = g1_s_fmt_stream,

	/* on below ops, buf->type used to switch to au or frame back-end */
	.vidioc_reqbufs = g1_reqbufs,
	.vidioc_querybuf = g1_querybuf,
	.vidioc_expbuf = g1_expbuf,
	.vidioc_qbuf = g1_qbuf,
	.vidioc_dqbuf = g1_dqbuf,
	.vidioc_streamon = g1_streamon,
	.vidioc_streamoff = g1_streamoff,
	.vidioc_g_crop = g1_g_crop,
	.vidioc_decoder_cmd = g1_decoder_cmd,
	.vidioc_subscribe_event = g1_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
	.vidioc_g_selection = g1_g_selection,
	.vidioc_s_selection = g1_s_selection,
};

int g1_rpmsg_endpoint_cb(struct rpmsg_mm_msg_hdr *data, void *priv)
{
	struct g1_dev *g1 = (struct g1_dev *)priv;

	if (!data)
		return -1;

	dev_dbg(g1->dev,
		"[RPMSG] endpoint callback[message:%d][expected message:%d]\n",
		data->info, g1->waited_rpmsg_event);
	switch (data->info) {
	/* M3 is informing about G1 usage */
	case RPMSG_MM_SHARED_RES_LOCKED:
		switch (g1->waited_rpmsg_event) {
		case RPMSG_MM_RESERVED:
			g1->waited_rpmsg_event = RPMSG_MM_SHARED_RES_UNLOCKED;
			break;
		case RPMSG_MM_COMM_AVAILABLE:
			complete(&g1->rpmsg_connected);
			break;
		case RPMSG_MM_SHARED_RES_UNLOCKED:
		default:
			break;
		}
		g1->waited_rpmsg_event = RPMSG_MM_SHARED_RES_UNLOCKED;
		break;
	/* M3 is informing that it doesn't use anymore the G1 */
	case RPMSG_MM_SHARED_RES_UNLOCKED:
		switch (g1->waited_rpmsg_event) {
		case RPMSG_MM_RESERVED:
			break;
		case RPMSG_MM_COMM_AVAILABLE:
			complete(&g1->rpmsg_connected);
			/* No break expected */
		case RPMSG_MM_SHARED_RES_UNLOCKED:
			complete(&g1->resource_unlock);
			break;
		default:
			break;
		}
		g1->waited_rpmsg_event = RPMSG_MM_RESERVED;
		break;
	case RPMSG_MM_COMM_AVAILABLE:
		switch (g1->waited_rpmsg_event) {
		case RPMSG_MM_COMM_AVAILABLE:
			complete(&g1->rpmsg_connected);
			break;
		case RPMSG_MM_RESERVED:
		case RPMSG_MM_SHARED_RES_UNLOCKED:
		default:
			break;
		}
		break;
	case RPMSG_MM_COMM_UNAVAILABLE:
		switch (g1->waited_rpmsg_event) {
		case RPMSG_MM_RESERVED:
		case RPMSG_MM_SHARED_RES_UNLOCKED:
			g1->waited_rpmsg_event = RPMSG_MM_COMM_AVAILABLE;
			break;
		case RPMSG_MM_COMM_AVAILABLE:
		default:
			break;
		}
	default:
		break;
	}
	return 0;
}

static int g1_wait_for_res_availability(struct g1_dev *g1)
{
	int m3_resource_usage = 0;

	dev_dbg(g1->dev,
		"[RPMSG] check resource availability [expected message:%d]\n",
		g1->waited_rpmsg_event);

check_availability:
	if (g1->waited_rpmsg_event == RPMSG_MM_COMM_AVAILABLE) {
		m3_resource_usage = sta_rpmsg_get_remote_resources_usage(
					g1->rpmsg_handle);

		if (m3_resource_usage < 0) {
			dev_dbg(g1->dev,
				"[RPMSG] not available, wait for connection\n");
			if (!wait_for_completion_timeout(
				&g1->rpmsg_connected,
				msecs_to_jiffies(5000))) {
				/* Timeout after 5sec, consider the
				 * resource as available to avoid freeze
				 */
				dev_err(g1->dev,
					"[RPMSG] : TIMEOUT!!!\n");
				goto go_out;
			}
			goto check_availability;
		}
		if ((m3_resource_usage & RPMSG_MM_HW_RES_G1) ==
			RPMSG_MM_HW_RES_G1) {
			dev_dbg(g1->dev,
				"[RPMSG] available, resource booked by M3\n");
			g1->waited_rpmsg_event = RPMSG_MM_SHARED_RES_UNLOCKED;
		} else {
			goto go_out;
		}
	}

	if (g1->waited_rpmsg_event == RPMSG_MM_SHARED_RES_UNLOCKED) {
		dev_dbg(g1->dev, "[RPMSG] wait for 'unlocked' message\n");
		if (wait_for_completion_interruptible(&g1->resource_unlock) ==
			-ERESTARTSYS) {
			/* Wait has been interrupted. It doesn't mean that
			 * resource is free so we go out on error
			 */
			dev_err(g1->dev, "[RPMSG] : Interrupted!!!\n");
			goto fatal_err;
		}
	}

go_out:
	g1->waited_rpmsg_event = RPMSG_MM_RESERVED;
	dev_dbg(g1->dev, "[RPMSG] : G1 resource available\n");
	return 0;

fatal_err:
	dev_err(g1->dev, "Fatal error\n");
	return -1;
}

static void g1_resume_work(struct work_struct *work)
{
	struct g1_dev *g1 = container_of(work, struct g1_dev, resume_work);

	if (g1_wait_for_res_availability(g1) < 0)
		g1->state = G1_STATE_FATAL_ERROR;
	g1_hw_resume(g1);
	g1->probe_status = PROBED;
	dev_dbg(g1->dev, "[RPMSG] : End of resume\n");
}

static void g1_init_work(struct work_struct *work)
{
	struct video_device *vdev;
	struct g1_dev *g1 = container_of(work, struct g1_dev, init_work);
	struct device *dev = g1->dev;
	int ret;

	g1->rpmsg_handle = sta_rpmsg_register_endpoint(RPSMG_MM_EPT_CORTEXA_G1,
						g1_rpmsg_endpoint_cb,
						(void *)g1);
	if (!g1->rpmsg_handle) {
		dev_err(dev, "Unable to register rpmsg_mm\n");
		goto on_error;
	}

	/* Start by checking that G1 IP is not used by M3 */
	if (g1_wait_for_res_availability(g1) < 0) {
		dev_err(dev, "Check resource availability returns an error\n");
		goto on_error;
	}

	INIT_LIST_HEAD(&g1->instances);

	g1_debugfs_create(g1);

	/* probe hardware */
	ret = g1_hw_probe(g1);
	if (ret)
		goto on_error;

	/* register all available decoders */
	register_all(g1);

	/* register on V4L2 */
	ret = v4l2_device_register(dev, &g1->v4l2_dev);
	if (ret) {
		dev_err(dev, "could not register v4l2 device\n");
		goto on_error;
	}

	vdev = video_device_alloc();
	vdev->fops = &g1_fops;
	vdev->ioctl_ops = &g1_ioctl_ops;
	vdev->release = video_device_release;
	vdev->lock = &g1->lock;
	vdev->v4l2_dev = &g1->v4l2_dev;
	snprintf(vdev->name, sizeof(vdev->name), "%s", G1_NAME);
	vdev->vfl_dir = VFL_DIR_M2M;
	ret = video_register_device(vdev, VFL_TYPE_GRABBER, 0);
	if (ret) {
		dev_err(dev, "failed to register video device\n");
		video_device_release(vdev);
		goto on_error;
	}
	g1->vdev = vdev;
	video_set_drvdata(vdev, g1);
	dev_info(dev, "registered as /dev/video%d\n", vdev->num);
	g1->state = G1_STATE_READY;
	g1->probe_status = PROBED;
	mutex_unlock(&g1->lock);
	return;

on_error:
	g1->state = G1_STATE_FATAL_ERROR;
	mutex_unlock(&g1->lock);
}

static int g1_probe(struct platform_device *pdev)
{
	struct g1_dev *g1;
	struct device *dev = &pdev->dev;

	g1 = devm_kzalloc(dev, sizeof(*g1), GFP_KERNEL);
	if (!g1)
		return -ENOMEM;

	g1->dev = dev;
	g1->pdev = pdev;
	platform_set_drvdata(pdev, g1);

	/* By default at probing, we consider the G1 IP
	 * locked by the M3 and communication pipe not
	 * yet established
	 */
	g1->waited_rpmsg_event = RPMSG_MM_COMM_AVAILABLE;
	g1->probe_status = PROBE_POSTPONED;

	g1->init_wq = create_singlethread_workqueue("g1_init_wq");
	if (!g1->init_wq)
		return -ENOMEM;

	g1->resume_wq = create_singlethread_workqueue("g1_resume_wq");
	if (!g1->resume_wq)
		return -ENOMEM;

	init_completion(&g1->rpmsg_connected);
	init_completion(&g1->resource_unlock);

	INIT_WORK(&g1->init_work, g1_init_work);
	queue_work(g1->init_wq, &g1->init_work);
	mutex_init(&g1->lock);
	/* Lock the API to finalize probe before starting something else */
	mutex_lock(&g1->lock);
	return 0;
}

static int g1_remove(struct platform_device *pdev)
{
	struct g1_dev *g1 = platform_get_drvdata(pdev);

	sta_rpmsg_unregister_endpoint(g1->rpmsg_handle);

	mutex_lock(&g1->lock);
	dev_info(g1->dev, "removing %s\n", pdev->name);

	complete(&g1->rpmsg_connected);
	complete(&g1->resource_unlock);
	cancel_work_sync(&g1->init_work);
	cancel_work_sync(&g1->resume_work);
	destroy_workqueue(g1->init_wq);
	destroy_workqueue(g1->resume_wq);

	g1_hw_remove(g1);

	g1_debugfs_remove(g1);

	video_unregister_device(g1->vdev);
	v4l2_device_unregister(&g1->v4l2_dev);

	mutex_unlock(&g1->lock);
	return 0;
}

/*FIXME can we move this in g1-hw.c ? problem around .pm...,
 * can we move this in g1_hw_probe ?
 */
static inline int g1_runtime_suspend(struct device *dev)
{
	struct g1_dev *g1 = dev_get_drvdata(dev);

	return g1_hw_pm_suspend(g1);
}

static inline int g1_runtime_resume(struct device *dev)
{
	struct g1_dev *g1 = dev_get_drvdata(dev);

	return g1_hw_pm_resume(g1);
}

static int g1_resume(struct device *dev)
{
	struct g1_dev *g1 = dev_get_drvdata(dev);

	/* Resume task is postponed in a dedicated work
	 * to be able to communicate with M3 using RPMSG
	 * service : aim => check M3 usage of G1 IP
	 */
	g1->waited_rpmsg_event = RPMSG_MM_COMM_AVAILABLE;

	if (g1->probe_status == PROBED) {
		g1->probe_status = RESUME_POSTPONED;
		init_completion(&g1->rpmsg_connected);
		init_completion(&g1->resource_unlock);
		INIT_WORK(&g1->resume_work, g1_resume_work);
		queue_work(g1->resume_wq, &g1->resume_work);
	}

	return 0;
}

static int g1_suspend(struct device *dev)
{
	struct g1_dev *g1 = dev_get_drvdata(dev);

	if (g1->probe_status == PROBED)
		return g1_hw_suspend(g1);
	dev_info(g1->dev,
		 "Probe not fully finalized, do nothing\n");
	return 0;
}

static const struct dev_pm_ops g1_pm_ops = {
	.suspend = g1_suspend,
	.resume = g1_resume,
	.runtime_suspend = g1_runtime_suspend,
	.runtime_resume = g1_runtime_resume,
};

static const struct of_device_id g1_match_types[] = {
	{
	 .compatible = "st,g1",
	 }, {
	     /* end node */
	     }
};

MODULE_DEVICE_TABLE(of, g1_match_types);

struct platform_driver g1_driver = {
	.probe = g1_probe,
	.remove = g1_remove,
	.driver = {
		   .name = G1_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = g1_match_types,
		   .pm = &g1_pm_ops,
		  },
};

module_platform_driver(g1_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hugues Fruchet <hugues.fruchet@st.com>");
MODULE_DESCRIPTION("STMicroelectronics G1 video decoder V4L2 driver");
