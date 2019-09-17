/*
 * Copyright (C) STMicroelectronics SA 2016
 * Authors: Yannick Fertre <yannick.fertre@st.com>
 *          Hugues Fruchet <hugues.fruchet@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include "g1.h"
#include "g1-cfg.h"

#include "stack/inc/h264decapi.h"

#define G1_H264_HEADER_PARSED	1
#define G1_H264_FLUSHING 2
#define G1_H264_REORDERING_STEP	0x80
#define G1_MAX_DPB_SIZE	16

//#define DEBUG_REORDERING 1

struct qframe {
	struct g1_frame *frame;
	struct qframe *next;
};

struct g1_h264_reorder {
	unsigned int dpb_size;
	unsigned int nb_qframes;
	struct qframe *qframes;
	u32 num_reorder_frames;
};

struct g1_h264_ctx {
	H264DecInst h264;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;

	unsigned int state;

	unsigned int next_idr_pic;
	unsigned int eos;
	struct g1_h264_reorder order;
	struct mutex reorder_lock; /* To handle concurrency of order field */
};

#define to_ctx(ctx) ((struct g1_h264_ctx *)(ctx)->priv)

/* debug API of h264 stack */
void H264DecTrace(const char *str)
{
#ifdef H264DEC_TRACE
	pr_debug("   %s\n", str);
#endif
};

#if defined DEBUG_REORDERING
static void display_qframes(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	struct g1_h264_reorder *order = &ctx->order;
	struct qframe *elt = order->qframes;

	mutex_lock(&ctx->reorder_lock);
	dev_dbg(g1->dev, "%s [reordering]--- queue (%d elts) ----",
		pctx->name,
		order->nb_qframes);
	while (elt) {
		dev_dbg(g1->dev,
			"%s [reordering] elt:%p   => frame[%d]pid:%d poc:%d next:%p%s",
			pctx->name,
			elt, elt->frame->index,
			elt->frame->picid, elt->frame->poc, elt->next,
			(elt->frame->state & G1_FRAME_QUEUED ? "" :
				 " => ToBeDisplayed"));
		elt = elt->next;
	}
	dev_dbg(g1->dev, "%s [reordering]--------------", pctx->name);
	mutex_unlock(&ctx->reorder_lock);
}
#endif

// Usage MUST BE protected by reorder_lock mutex
static int queue_frame_poc_order(struct g1_h264_ctx *ctx, struct g1_frame *f)
{
	struct g1_h264_reorder *order = NULL;
	struct qframe *qf = NULL;
	struct qframe *elt = NULL;
	struct qframe *prev = NULL;

	qf = kzalloc(sizeof(*qf), GFP_KERNEL);
	if (!qf)
		return -1;

	qf->frame = f;
	f->state |= G1_FRAME_QUEUED;
	qf->next = NULL;

	order = &ctx->order;
	elt = order->qframes;
	order->nb_qframes++;
	if (!elt) {
		order->qframes = qf;
		goto out;
	}
	while (elt) {
		if (elt->frame->poc > f->poc) {
			if (prev)
				prev->next = qf;
			else
				order->qframes = qf;
			qf->next = elt;
			elt = NULL;
		} else {
			prev = elt;
			elt = elt->next;
			if (!elt)
				prev->next = qf;
		}
	}
out:
	return 0;
}

// Usage MUST BE protected by reorder_lock mutex
static int dequeue_first_frame(struct g1_h264_ctx *ctx)
{
	struct g1_h264_reorder *order = NULL;
	struct qframe *elt = NULL;

	order = &ctx->order;
	elt = order->qframes;

	if (!elt)
		return 0;

	elt->frame->state &= ~G1_FRAME_QUEUED;
	return 1;
}

// Usage MUST BE protected by reorder_lock mutex
static
int dequeue_frames_lower_poc(struct g1_h264_ctx *ctx, unsigned int max_poc)
{
	struct g1_h264_reorder *order = NULL;
	struct qframe *elt = NULL;

	order = &ctx->order;
	elt = order->qframes;

	if (!elt)
		return 0;

	while (elt) {
		if (elt->frame->poc <= max_poc)
			elt->frame->state &= ~G1_FRAME_QUEUED;
		elt = elt->next;
	}
	return 0;
}

static int get_frame_to_be_displayed(struct g1_h264_ctx *ctx,
				     struct g1_frame **f)
{
	struct g1_h264_reorder *order = NULL;
	struct qframe *elt = NULL;
	struct qframe *prev = NULL;

	mutex_lock(&ctx->reorder_lock);
	*f = NULL;
	order = &ctx->order;
	elt = order->qframes;

	if (!elt) {
		mutex_unlock(&ctx->reorder_lock);
		return 0;
	}

	while (elt) {
		if (!(elt->frame->state & G1_FRAME_QUEUED) || ctx->eos) {
			*f = elt->frame;
			if (prev)
				prev->next = elt->next;
			else
				order->qframes = elt->next;
			order->nb_qframes--;
			kfree(elt);
			mutex_unlock(&ctx->reorder_lock);
			return 1;
		}
		prev = elt;
		elt = elt->next;
	}
	mutex_unlock(&ctx->reorder_lock);
	return 0;
}

static inline int to_ret(H264DecRet ret)
{
	switch (ret) {
	case H264DEC_OK:
	case H264DEC_STRM_PROCESSED:
	case H264DEC_PIC_RDY:
	case H264DEC_PIC_DECODED:
	case H264DEC_HDRS_RDY:
	case H264DEC_ADVANCED_TOOLS:
	case H264DEC_PENDING_FLUSH:
	case H264DEC_NONREF_PIC_SKIPPED:
	case H264DEC_END_OF_STREAM:
		return 0;
	case H264DEC_STRM_ERROR:
		return 0;

	case H264DEC_MEMFAIL:
		return -ENOMEM;

	case H264DEC_HDRS_NOT_RDY:
	case H264DEC_STREAM_NOT_SUPPORTED:
		return -EINVAL;

	default:
		return -EIO;
	}
}

static inline int is_stream_error(H264DecRet ret)
{
	switch (ret) {
	case H264DEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *h264ret_str(H264DecRet ret)
{
	switch (ret) {
	case H264DEC_OK:
		return "H264DEC_OK";
	case H264DEC_STRM_PROCESSED:
		return "H264DEC_STRM_PROCESSED";
	case H264DEC_PIC_RDY:
		return "H264DEC_PIC_RDY";
	case H264DEC_PIC_DECODED:
		return "H264DEC_PIC_DECODED";
	case H264DEC_HDRS_RDY:
		return "H264DEC_HDRS_RDY";
	case H264DEC_ADVANCED_TOOLS:
		return "H264DEC_ADVANCED_TOOLS";
	case H264DEC_PENDING_FLUSH:
		return "H264DEC_PENDING_FLUSH";
	case H264DEC_NONREF_PIC_SKIPPED:
		return "H264DEC_NONREF_PIC_SKIPPED";
	case H264DEC_END_OF_STREAM:
		return "H264DEC_END_OF_STREAM";
	case H264DEC_PARAM_ERROR:
		return "H264DEC_PARAM_ERROR";
	case H264DEC_STRM_ERROR:
		return "H264DEC_STRM_ERROR";
	case H264DEC_NOT_INITIALIZED:
		return "H264DEC_NOT_INITIALIZED";
	case H264DEC_MEMFAIL:
		return "H264DEC_MEMFAIL";
	case H264DEC_INITFAIL:
		return "H264DEC_INITFAIL";
	case H264DEC_HDRS_NOT_RDY:
		return "H264DEC_HDRS_NOT_RDY";
	case H264DEC_STREAM_NOT_SUPPORTED:
		return "H264DEC_STREAM_NOT_SUPPORTED";
	case H264DEC_HW_RESERVED:
		return "H264DEC_HW_RESERVED";
	case H264DEC_HW_TIMEOUT:
		return "H264DEC_HW_BUS_ERROR";
	case H264DEC_HW_BUS_ERROR:
		return "H264DEC_HW_BUS_ERROR";
	case H264DEC_SYSTEM_ERROR:
		return "H264DEC_SYSTEM_ERROR";
	case H264DEC_DWL_ERROR:
		return "H264DEC_DWL_ERROR";
	case H264DEC_EVALUATION_LIMIT_EXCEEDED:
		return "H264DEC_EVALUATION_LIMIT_EXCEEDED";
	case H264DEC_FORMAT_NOT_SUPPORTED:
		return "H264DEC_FORMAT_NOT_SUPPORTED";
	default:
		return "!unknown H264 return value!";
	}
}

static inline int to_v4l2_pixelformat(H264DecOutFormat fmt)
{
	switch (fmt) {
	case H264DEC_SEMIPLANAR_YUV420:
		return V4L2_PIX_FMT_NV12;
	case H264DEC_YUV400:
		return V4L2_PIX_FMT_Y4;/* TBC with a greyscale bitstream */
	default:
		return 0;
	}
}

static int g1_h264_probe(struct g1_dev *g1)
{
	/* check if codec is supported by hardware */
	if (!g1->hw.config.h264_support)
		return -EACCES;

	return 0;
}

static int g1_h264_init(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	H264DecRet h264ret;
	int ret;
	bool reordering_done_in_driver = false;

	/* In case of low latency mode, we rely on driver reordering instead of
	 * G1 stack reordering. This is done to avoid to wait for DPB value
	 * access units before pushing the first decoded frame.
	 */
	if (pctx->mode == G1_MODE_LOW_LATENCY)
		reordering_done_in_driver = true;
	else
		pctx->mode = G1_MODE_NORMAL;

	h264ret = H264DecInit(&ctx->h264,
			      reordering_done_in_driver,
			      true,  /* video freeze concealment */
#if defined G1_H264_USE_DISPLAY_SMOOTHING
			      true,
#else
			      false,
#endif
			      DEC_REF_FRM_RASTER_SCAN, /* no tiled output */
			      (void *)pctx);
	if (h264ret) {
		dev_err(g1->dev, "%s  H264DecInit error %s(%d)\n", pctx->name,
			h264ret_str(h264ret), h264ret);
		ret = -EIO;
		goto err_free;
	}

	ret = g1_pp_open(pctx, ctx->h264, ctx->streaminfo.streamformat);
	if (ret)
		goto err_release;
	return 0;

err_release:
	H264DecRelease(ctx->h264);
err_free:
	kfree(ctx);
	return ret;
}

static int g1_h264_open(struct g1_ctx *pctx)
{
	struct g1_h264_ctx *ctx;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	pctx->priv = ctx;

	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));

	/* setup reodering context */
	mutex_init(&ctx->reorder_lock);
	ctx->order.dpb_size = G1_MAX_DPB_SIZE;
	ctx->order.qframes = NULL;
	ctx->order.nb_qframes = 0;
	return 0;
}

static int g1_h264_close(struct g1_ctx *pctx)
{
	struct g1_h264_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	H264DecRelease(ctx->h264);

	kfree(ctx);

	return 0;
}

static int g1_h264_set_streaminfo(struct g1_ctx *pctx, H264DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;

	if ((decinfo->picWidth == 0) || (decinfo->picHeight == 0) ||
	    (decinfo->picWidth * decinfo->picHeight > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid stream coded resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, decinfo->picWidth, decinfo->picHeight,
			G1_MAX_RESO);
		return -EINVAL;
	}
	streaminfo->width = decinfo->picWidth;
	streaminfo->height = decinfo->picHeight;

	if (decinfo->cropParams.cropOutWidth != decinfo->picWidth ||
	    decinfo->cropParams.cropOutHeight !=  decinfo->picHeight) {
		/* return native video Width/Height and not the aligned one*/
		/* then store crop info */
		streaminfo->flags |= G1_STREAMINFO_FLAG_CROP;
		streaminfo->crop.left = decinfo->cropParams.cropLeftOffset;
		streaminfo->crop.top = decinfo->cropParams.cropTopOffset;
		streaminfo->crop.width = decinfo->cropParams.cropOutWidth;
		streaminfo->crop.height = decinfo->cropParams.cropOutHeight;
	} else {
		streaminfo->width = decinfo->picWidth;
		streaminfo->height = decinfo->picHeight;
	}
	streaminfo->streamformat = V4L2_PIX_FMT_H264;
	streaminfo->dpb = decinfo->picBuffSize;
	ctx->order.dpb_size = decinfo->picBuffSize;

	/* FIXME how to find profile & version? set 0 by default */
	snprintf(streaminfo->profile, sizeof(streaminfo->profile), "%d", 0);
	snprintf(streaminfo->level, sizeof(streaminfo->level), "%d", 0);

	/* interlaced */
	streaminfo->field = (decinfo->interlacedSequence ?
			    V4L2_FIELD_INTERLACED :
			    V4L2_FIELD_NONE);

	/* FIXME aspect-ratio */

	/* other... */
	if (decinfo->monoChrome) {
		streaminfo->flags |= G1_STREAMINFO_FLAG_OTHER;
		snprintf(streaminfo->other,
			 sizeof(streaminfo->other), "monochrome");
	}

	return 0;
}

static int g1_h264_set_dec_frameinfo(struct g1_ctx *pctx, H264DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	struct g1_frameinfo *frameinfo = &ctx->dec_frameinfo;
	__u32 fmt;

	/* check decoder output width/height */
	if ((decinfo->picWidth == 0) ||
	    (decinfo->picHeight == 0) ||
	    (decinfo->picWidth *
	     decinfo->picHeight > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid decoder output resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name,
			decinfo->picWidth,
			decinfo->picHeight,
			G1_MAX_RESO);
		return -EINVAL;
	}

	/* check decoder output format */
	fmt = to_v4l2_pixelformat(decinfo->outputFormat);
	if (!fmt) {
		dev_err(g1->dev,
			"%s  unsupported decoder output format (%d)\n",
			pctx->name, decinfo->outputFormat);
		return -EINVAL;
	}
	frameinfo->pixelformat = fmt;

	frameinfo->width = decinfo->picWidth;
	frameinfo->height = decinfo->picHeight;
	frameinfo->aligned_width = decinfo->picWidth;
	frameinfo->aligned_height = decinfo->picHeight;

	/* following test allows to handle two cases :
	 * - case where crop info is inside the stream (sps->croppingflag)
	 * - case where G1 imposes its alignment constraints (16 pixels)
	 *   picWidth = aligned value & cropWidth = native stream width
	 *   ==> crop is needed
	 */
	if (decinfo->cropParams.cropOutWidth != decinfo->picWidth ||
	    decinfo->cropParams.cropOutHeight !=  decinfo->picHeight) {
		frameinfo->flags |= G1_FRAMEINFO_FLAG_CROP;
		frameinfo->crop.width = decinfo->cropParams.cropOutWidth;
		frameinfo->crop.height = decinfo->cropParams.cropOutHeight;
		frameinfo->crop.left = decinfo->cropParams.cropLeftOffset;
		frameinfo->crop.top = decinfo->cropParams.cropTopOffset;
	}

	frameinfo->field = (decinfo->interlacedSequence ?
			   V4L2_FIELD_INTERLACED :
			   V4L2_FIELD_NONE);

	/* check colorimetry range :
	 * V4L2_COLORSPACE_REC709 => Range is [16-240]
	 * V4L2_COLORSPACE_JPEG => Full range [0-255]
	 */
	frameinfo->colorspace = V4L2_COLORSPACE_REC709;
	if (decinfo->videoRange)
		frameinfo->colorspace = V4L2_COLORSPACE_JPEG;

	return 0;
}

static int g1_h264_get_streaminfo(struct g1_ctx *pctx,
				  struct g1_streaminfo *streaminfo)
{
	struct g1_h264_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_H264_HEADER_PARSED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_h264_get_frameinfo(struct g1_ctx *pctx,
				 struct g1_frameinfo *frameinfo)
{
	struct g1_h264_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_H264_HEADER_PARSED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_h264_set_frameinfo(struct g1_ctx *pctx,
				 struct g1_frameinfo *frameinfo)
{
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_H264_HEADER_PARSED)
		return -ENODATA;

	/* FIXME : in H264 only, do not change interlaced mode.
	 * to be removed as soon as Deinterlace feature for H264
	 * is fully implemented
	 */
	frameinfo->field = ctx->dec_frameinfo.field;

	/* info not coming from users but from decoder itself so update it */
	frameinfo->colorspace = ctx->dec_frameinfo.colorspace;

	/* let post-proc check & update infos to what it can do */
	ret = g1_pp_check_config(pctx, &ctx->dec_frameinfo,
				 frameinfo);
	if (ret)
		return ret;

	ctx->pp_frameinfo = *frameinfo;

	return 0;
}

static int g1_h264_decode(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	int ret;
	H264DecRet h264ret;
	H264DecInput in;
	H264DecOutput out;
	H264DecInfo decinfo;

	dev_dbg(g1->dev, "%s  > %s\n", pctx->name, __func__);

	if (!ctx->h264) {
		if (g1_h264_init(pctx))
			return -EIO;
	}

	if (ctx->state >= G1_H264_HEADER_PARSED) {
		struct g1_frame *frame;
		struct g1_frameinfo *dec_frameinfo = &ctx->dec_frameinfo;

		/* get a free output frame where to decode */
		ret = g1_get_free_frame(pctx, &frame);
		if (ret)
			return ret;

		/* setup post-processor */
		ret = g1_pp_set_config(pctx, dec_frameinfo, frame);
		if (ret)
			return ret;
	}
retry:
	memset(&in, 0, sizeof(in));
	in.pStream = au->vaddr;
	in.streamBusAddress = au->paddr;
	in.dataLen = au->size;
	in.picId = au->picid;

	memset(&out, 0, sizeof(out));

	/* decode this access unit */
	h264ret = H264DecDecode(ctx->h264, &in, &out);
	/* check decode status */
	ret = to_ret(h264ret);
	if (ret) {
		dev_err(g1->dev, "%s  H264DecDecode error %s(%d)",
			pctx->name, h264ret_str(h264ret), h264ret);
		pctx->decode_errors++;
		return ret;
	}
	au->remaining_data = out.dataLeft;

	if (is_stream_error(h264ret)) {
		dev_warn_ratelimited(g1->dev,
				     "%s  stream error @ frame %d, au size=%d\n",
				     pctx->name, pctx->decoded_frames,
				     au->size);
		pctx->stream_errors++;
		return 0;
	}

	if (h264ret == H264DEC_HDRS_RDY) {
		/* header detected */

		/* get new stream infos */
		h264ret = H264DecGetInfo(ctx->h264, &decinfo);
		if (h264ret) {
			dev_err(g1->dev, "%s  H264DecGetInfo error (%d)\n",
				pctx->name,  h264ret);
			return -EIO;
		}

		/* new live header ? */
		if (ctx->state >= G1_H264_HEADER_PARSED) {
			dev_dbg(g1->dev,
				"%s  new header detection while decoding\n",
				pctx->name);
			if (out.dataLeft) {
				dev_dbg(g1->dev,
					"%s  %d remaining bytes, retry\n",
					pctx->name, out.dataLeft);
				goto retry;
			}
			return -ENODATA;
		}

		/* first header, store infos */
		ret = g1_h264_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_h264_set_dec_frameinfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* initialize post-proc output frame infos
		 * with decoder output frame infos
		 */
		ctx->pp_frameinfo = ctx->dec_frameinfo;
		if (ctx->dec_frameinfo.flags & G1_FRAMEINFO_FLAG_CROP) {
			/* if crop is required and handled by PP, PP output
			 * resolution is the crop resolution
			 *
			 * FIXME : aligned_width and aligned_height shouldn't
			 * be updated at this level. We should update width
			 * and height instead. Aligned values will be set
			 * in pp_check_config to adapt values to G1 constraints
			 */
			ctx->pp_frameinfo.aligned_width =
				ctx->dec_frameinfo.crop.width;
			ctx->pp_frameinfo.aligned_height =
				ctx->dec_frameinfo.crop.height;
			/* clean-up crop info from pp_frameinfo because
			 * crop won't be needed anymore after the PP
			 * execution
			 */
			ctx->pp_frameinfo.flags &= ~G1_FRAMEINFO_FLAG_CROP;
			memset(&ctx->pp_frameinfo.crop, 0,
			       sizeof(struct v4l2_rect));
		}

		/* and let post-proc check & update infos to what it can do */
		ret = g1_pp_check_config(pctx, &ctx->dec_frameinfo,
					 &ctx->pp_frameinfo);
		if (ret)
			return ret;

		ctx->state = G1_H264_HEADER_PARSED;
		return -ENODATA;
	}

	switch (h264ret) {
	case H264DEC_OK:
	case H264DEC_STRM_PROCESSED:
	case H264DEC_PIC_RDY:
	case H264DEC_ADVANCED_TOOLS:
	case H264DEC_NONREF_PIC_SKIPPED:
	case H264DEC_END_OF_STREAM:
		/* output (decoded data) not expected so return nodata */
		return -ENODATA;
	case H264DEC_PENDING_FLUSH:
		return -EAGAIN;
	case H264DEC_PIC_DECODED:
	{
		H264DecPicture decoded_pic_info;
		int ret = 0;

		/* frame decoded */
		pctx->decoded_frames++;

		/* In case of interlaced stream (field mode),
		 * 2 AUs are treated to get one frame, so we
		 * need to inform v4l2 to do discard the push_dts
		 */
		H264DecPeek(ctx->h264, &decoded_pic_info);

		if (decoded_pic_info.isIdrPicture[0])
			ctx->next_idr_pic = decoded_pic_info.picId;

		if (decoded_pic_info.interlaced &&
		    decoded_pic_info.fieldPicture &&
		    decoded_pic_info.topField)
			ret = -ENODATA;

		dev_dbg(g1->dev, "%s  dec picture cnt=%d (return : %d)\n",
			pctx->name,
			pctx->decoded_frames, ret);

		return ret;
	}
	default:
		/* no error detected at this point but we didn't treat
		 * the returned value ... Strange !!!
		 * log a trace to investigate if needed
		 */
		dev_err_ratelimited(g1->dev,
				    "%s - Untreated decode returned value : %s(%d)\n",
				    pctx->name, h264ret_str(h264ret), h264ret);
		return -EIO;
	}
}

static
unsigned int g1_h264_compute_reordering(struct g1_ctx *pctx,
					struct g1_frame *frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	struct g1_h264_reorder *ordering = NULL;
	bool isIDR = false;

	if (pctx->mode != G1_MODE_LOW_LATENCY)
		return 0;

	mutex_lock(&ctx->reorder_lock);

	ordering = &ctx->order;
	isIDR = ((frame->flags & V4L2_BUF_FLAG_KEYFRAME) ==
		 V4L2_BUF_FLAG_KEYFRAME);

#if defined DEBUG_REORDERING
	dev_dbg(g1->dev,
		"%s [reordering][picId:%d][num_reorder_frames:%d][picType:%s][poc:%d][queued frames:%d]",
		pctx->name,
		frame->picid,
		ordering->num_reorder_frames,
		(isIDR ? "IDR" : (frame->isref ? "REF" : "NON_REF")),
		frame->poc, ordering->nb_qframes);
#endif
	/* An IDR frame systematically means that we have to flush queued
	 * decoded frames.
	 * We also display this IDR frame.
	 */
	if (isIDR) {
		/* Force POC of this IDR frame to the max so that it's
		 * queued at the end of the list
		 */
		frame->poc = 0x7FFFFFFF;
		if (queue_frame_poc_order(ctx, frame) < 0)
			dev_warn(g1->dev,
				 "%s [reordering] Unable to queue frame",
				 pctx->name);
		/* Then tag every queued frames to be output */
		dequeue_frames_lower_poc(ctx, 0x7FFFFFFF);
		goto end_reordering;
	}

	/* systematically queue decoded frame before checking if it may be
	 * displayed
	 */
	if (queue_frame_poc_order(ctx, frame) < 0)
		dev_warn(g1->dev, "%s [reordering] Unable to queue frame",
			 pctx->name);

	/* Non-reference frame reception indicates that we can display queued
	 * frames having a lower or equal POC
	 */
	if (frame->isref == 0)
		dequeue_frames_lower_poc(ctx, frame->poc);

	/* In other cases, if number of queued frames is strictly greater
	 * than 'num_reorder_frames' value then we dequeue lower POC
	 * else we don't output any frames
	 */
	else if ((ordering->nb_qframes) > ordering->num_reorder_frames) {
		if (!dequeue_first_frame(ctx)) {
			dev_warn_ratelimited(g1->dev,
					     "%s  Unable to dequeue a frame\n",
					     pctx->name);
		}
	}
end_reordering:
	mutex_unlock(&ctx->reorder_lock);

#if defined DEBUG_REORDERING
	display_qframes(pctx);
#endif
	return G1_H264_REORDERING_STEP;
}

static int g1_h264_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	int ret = 0;
	H264DecRet h264ret;
	H264DecPicture picture;
	u32 pictureId = 0;

	memset(&picture, 0, sizeof(picture));

	/* systematically check if we need to allocate a free frame
	 * to the PP.
	 * mainly useful in case of drain to store the last reference
	 * frame ( scheduled by g1_decoder_stop_cmd )
	 */
	if (ctx->state >= G1_H264_HEADER_PARSED) {
		struct g1_frame *thisframe;
		struct g1_frameinfo *dec_frameinfo = &ctx->dec_frameinfo;

		/* get a free output frame where to decode */
		ret = g1_get_free_frame(pctx, &thisframe);
		if (ret) {
			if (ret == -EAGAIN) {
				ret = -ENODATA;
				goto nodata;
			}
			return ret;
		}
		if (ret == 0) {
			/* setup post-processor */
			ret = g1_pp_set_config(pctx, dec_frameinfo, thisframe);
			if (ret)
				return ret;
		}
	}

	if (!(ctx->state & G1_H264_REORDERING_STEP)) {
		h264ret = H264DecNextPicture(ctx->h264, &picture, ctx->eos);
		ret = to_ret(h264ret);
		if (ret) {
			dev_err(g1->dev, "%s  H264DecNextPicture error %s(%d)",
				pctx->name,
				h264ret_str(h264ret), h264ret);
			pctx->decode_errors++;
			return ret;
		}

		if (h264ret != H264DEC_PIC_RDY) {
			ret = -ENODATA;
			goto parse_queue;/* no frame decoded */
		}

		/* decoded frame available */
		/* get the post-processed frame */
		ret = g1_pp_get_frame(pctx, frame);
		if (ret)
			return ret;

		/* update frame flags */
		if (picture.isIdrPicture[0])
			(*frame)->flags |= V4L2_BUF_FLAG_KEYFRAME;
		else
			(*frame)->flags |= V4L2_BUF_FLAG_PFRAME;

		(*frame)->pix.field = V4L2_FIELD_NONE;
		pictureId = picture.picId;

		(*frame)->picid = pictureId;
		(*frame)->poc = picture.picOrderCnt[0];
		(*frame)->isref = picture.isRef;
		ctx->order.num_reorder_frames = picture.num_reorder_frames;

		if (picture.interlaced) {
			if (picture.fieldPicture)
				if (picture.topField)
					(*frame)->pix.field = V4L2_FIELD_TOP;
				else
					(*frame)->pix.field = V4L2_FIELD_BOTTOM;
			else
				(*frame)->pix.field = V4L2_FIELD_INTERLACED;
		}

		(*frame)->to_drop = (picture.nbrOfErrMBs ? true : false);

		if (ctx->state == G1_H264_FLUSHING) {
			/* Seek case : In flushing state, while there is no
			 * decoded key frame, don't display anything as we're
			 * not sure frames are sane.
			 */
			if (pictureId == ctx->next_idr_pic) {
				dev_dbg(g1->dev,
					"FLUSHING STATE : I frame received\n");
				ctx->state = G1_H264_HEADER_PARSED;
			} else {
				(*frame)->to_drop = true;
			}
		}

		if (picture.nbrOfErrMBs) {
			dev_warn_ratelimited(g1->dev,
					     "%s  Number of Concealed MB : %d\n",
					     pctx->name, picture.nbrOfErrMBs);
			pctx->stream_errors++;
		}
		ctx->state |= g1_h264_compute_reordering(pctx, *frame);
		ret = 0;
	}
parse_queue:
	if (pctx->mode == G1_MODE_LOW_LATENCY) {
		if (ctx->state & G1_H264_REORDERING_STEP || ctx->eos) {
			if (get_frame_to_be_displayed(ctx, frame))
				return 0;
			ret = -ENODATA;
		}
	}
nodata:
	ctx->state &= ~G1_H264_REORDERING_STEP;
	if (ctx->eos)
		ctx->eos = 0; /* no more frames to drain: end of EOS */
	return ret;
}

static void g1_h264_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
{
	struct g1_dev *g1 = pctx->dev;

	dev_dbg(g1->dev,
		"%s  rec frame[%d] %s\n",
		pctx->name,
		frame->index,
		frame_state_str(frame->state,
				pctx->str, sizeof(pctx->str)));

	g1_recycle(pctx, frame);
}

static void g1_h264_flush(struct g1_ctx *pctx)
{
	struct g1_h264_ctx *ctx = to_ctx(pctx);
	struct g1_h264_reorder *order = NULL;
	struct qframe *elt = NULL;
	struct qframe *next = NULL;

	mutex_lock(&ctx->reorder_lock);
	order = &ctx->order;
	elt = order->qframes;
	while (elt) {
		if (elt->frame)
			elt->frame->state = G1_FRAME_FREE;
		next = elt->next;
		kfree(elt);
		elt = next;
	}
	order->qframes = NULL;
	order->nb_qframes = 0;
	mutex_unlock(&ctx->reorder_lock);
	ctx->state = G1_H264_FLUSHING;
}

static void g1_h264_drain(struct g1_ctx *pctx)
{
	struct g1_h264_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

/* H264 decoder can decode H264 contents, add below
 * one decoder struct per supported format
 */
static const __u32 h264_stream_formats[] = {
	V4L2_PIX_FMT_H264,
};

const struct g1_dec h264dec = {
	.name = "h264",
	.streamformat = &h264_stream_formats[0],
	.nb_streams = ARRAY_SIZE(h264_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_h264_probe,
	.open = g1_h264_open,
	.close = g1_h264_close,
	.get_streaminfo = g1_h264_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_h264_get_frameinfo,
	.set_frameinfo = g1_h264_set_frameinfo,
	.decode = g1_h264_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_h264_get_frame,
	.recycle = g1_h264_recycle,
	.flush = g1_h264_flush,
	.drain = g1_h264_drain
};
