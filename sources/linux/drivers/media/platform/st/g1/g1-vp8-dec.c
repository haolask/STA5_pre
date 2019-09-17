/*
 * Copyright (C) STMicroelectronics SA 2016
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Jean-Christophe Trotin <jean-christophe.trotin@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/slab.h>

#include "g1.h"
#include "g1-cfg.h"
#include "g1-mem.h"

#include "stack/inc/vp8decapi.h"
#include "stack/common/bqueue.h"

#define G1_VP8_HEADER_PARSED	1
#define G1_VP8_FLUSHING 2

struct g1_vp8_ctx {
	VP8DecInst vp8;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;

	unsigned int state;

	struct g1_buf pending_buf_struct;
	struct g1_buf *pending_buf;
	struct g1_au pending_au_struct;
	struct g1_au *pending_au;

	unsigned int eos;
};

#define to_ctx(ctx) ((struct g1_vp8_ctx *)(ctx)->priv)

/* debug API of VP8 stack */
void VP8DecTrace(const char *str)
{
#ifdef VP8DEC_TRACE
	pr_debug("   %s\n", str);
#endif
};

static inline int to_ret(VP8DecRet ret)
{
	switch (ret) {
	case VP8DEC_OK:
	case VP8DEC_STRM_PROCESSED:
	case VP8DEC_PIC_RDY:
	case VP8DEC_PIC_DECODED:
	case VP8DEC_HDRS_RDY:
	case VP8DEC_ADVANCED_TOOLS:
	case VP8DEC_SLICE_RDY:
		return 0;
	case VP8DEC_STRM_ERROR:
		return 0;

	case VP8DEC_MEMFAIL:
		return -ENOMEM;

	case VP8DEC_HDRS_NOT_RDY:
	case VP8DEC_STREAM_NOT_SUPPORTED:
		return -EINVAL;

	default:
		return -EIO;
	}
}

static inline int is_stream_error(VP8DecRet ret)
{
	switch (ret) {
	case VP8DEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *vp8ret_str(VP8DecRet ret)
{
	switch (ret) {
	case VP8DEC_OK:
		return "VP8DEC_OK";
	case VP8DEC_STRM_PROCESSED:
		return "VP8DEC_STRM_PROCESSED";
	case VP8DEC_PIC_RDY:
		return "VP8DEC_PIC_RDY";
	case VP8DEC_PIC_DECODED:
		return "VP8DEC_PIC_DECODED";
	case VP8DEC_HDRS_RDY:
		return "VP8DEC_HDRS_RDY";
	case VP8DEC_ADVANCED_TOOLS:
		return "VP8DEC_ADVANCED_TOOLS";
	case VP8DEC_SLICE_RDY:
		return "VP8DEC_SLICE_RDY";
	case VP8DEC_PARAM_ERROR:
		return "VP8DEC_PARAM_ERROR";
	case VP8DEC_STRM_ERROR:
		return "VP8DEC_STRM_ERROR";
	case VP8DEC_NOT_INITIALIZED:
		return "VP8DEC_NOT_INITIALIZED";
	case VP8DEC_MEMFAIL:
		return "VP8DEC_MEMFAIL";
	case VP8DEC_INITFAIL:
		return "VP8DEC_INITFAIL";
	case VP8DEC_HDRS_NOT_RDY:
		return "VP8DEC_HDRS_NOT_RDY";
	case VP8DEC_STREAM_NOT_SUPPORTED:
		return "VP8DEC_STREAM_NOT_SUPPORTED";
	case VP8DEC_HW_RESERVED:
		return "VP8DEC_HW_RESERVED";
	case VP8DEC_HW_TIMEOUT:
		return "VP8DEC_HW_BUS_ERROR";
	case VP8DEC_HW_BUS_ERROR:
		return "VP8DEC_HW_BUS_ERROR";
	case VP8DEC_SYSTEM_ERROR:
		return "VP8DEC_SYSTEM_ERROR";
	case VP8DEC_DWL_ERROR:
		return "VP8DEC_DWL_ERROR";
	case VP8DEC_EVALUATION_LIMIT_EXCEEDED:
		return "VP8DEC_EVALUATION_LIMIT_EXCEEDED";
	case VP8DEC_FORMAT_NOT_SUPPORTED:
		return "VP8DEC_FORMAT_NOT_SUPPORTED";
	default:
		return "!unknown VP8 return value!";
	}
}

static inline int to_v4l2_pixelformat(VP8DecOutFormat fmt)
{
	switch (fmt) {
	case VP8DEC_SEMIPLANAR_YUV420:
		return V4L2_PIX_FMT_NV12;
	default:
		return 0;
	}
}

static int g1_vp8_probe(struct g1_dev *g1)
{
	/* check if codec is supported by hardware */
	if (!g1->hw.config.vp8_support)
		return -EACCES;

	return 0;
}

static int g1_vp8_open(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vp8_ctx *ctx;
	VP8DecRet vp8ret;
	int ret;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	pctx->priv = ctx;

	vp8ret = VP8DecInit(&ctx->vp8, VP8DEC_VP8,
			    0, /* no video freeze concealment */
			    0, /* no custom buffers */
			    DEC_REF_FRM_RASTER_SCAN, /* no tiled output */
			    (void *)pctx); /* forward g1 context to DWL */
	if (vp8ret) {
		dev_err(g1->dev, "%s  VP8DecInit error %s(%d)\n", pctx->name,
			vp8ret_str(vp8ret), vp8ret);
		ret = -EIO;
		goto err_free;
	}

	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));

	ret = g1_pp_open(pctx, ctx->vp8, ctx->streaminfo.streamformat);
	if (ret)
		goto err_release;

	return 0;

err_release:
	VP8DecRelease(ctx->vp8);
err_free:
	kfree(ctx);
	return ret;
}

static int g1_vp8_close(struct g1_ctx *pctx)
{
	struct g1_vp8_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	VP8DecRelease(ctx->vp8);

	kfree(ctx);

	return 0;
}

static int g1_vp8_set_streaminfo(struct g1_ctx *pctx, VP8DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vp8_ctx *ctx = to_ctx(pctx);
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;

	/* check width/height */
	if ((decinfo->codedWidth == 0) || (decinfo->codedHeight == 0) ||
	    (decinfo->codedWidth * decinfo->codedHeight > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid stream coded resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, decinfo->codedWidth, decinfo->codedHeight,
			G1_MAX_RESO);
		return -EINVAL;
	}

	streaminfo->streamformat = V4L2_PIX_FMT_VP8;
	streaminfo->width = decinfo->codedWidth;
	streaminfo->height = decinfo->codedHeight;
	streaminfo->field = V4L2_FIELD_NONE;

	snprintf(streaminfo->profile, sizeof(streaminfo->profile), "profile %d",
		 decinfo->vpProfile);
	snprintf(streaminfo->level, sizeof(streaminfo->level), "level %d",
		 decinfo->vpVersion);

	streaminfo->dpb = 0;
	return 0;
}

static int g1_vp8_set_dec_frameinfo(struct g1_ctx *pctx, VP8DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vp8_ctx *ctx = to_ctx(pctx);
	struct g1_frameinfo *frameinfo = &ctx->dec_frameinfo;
	__u32 fmt;

	/* check decoder output width/height */
	if ((decinfo->frameWidth == 0) || (decinfo->frameHeight == 0) ||
	    (decinfo->frameWidth * decinfo->frameHeight > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid decoder output resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, decinfo->frameWidth, decinfo->frameHeight,
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
	frameinfo->width = decinfo->codedWidth;
	frameinfo->height = decinfo->codedHeight;
	frameinfo->aligned_width = decinfo->frameWidth;
	frameinfo->aligned_height = decinfo->frameHeight;
	frameinfo->field = V4L2_FIELD_NONE;

	return 0;
}

static int g1_vp8_get_streaminfo(struct g1_ctx *pctx,
				 struct g1_streaminfo *streaminfo)
{
	struct g1_vp8_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_VP8_HEADER_PARSED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_vp8_get_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_vp8_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_VP8_HEADER_PARSED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_vp8_set_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_vp8_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_VP8_HEADER_PARSED)
		return -ENODATA;

	/* let post-proc check & update infos to what it can do */
	ret = g1_pp_check_config(pctx, &ctx->dec_frameinfo,
				 frameinfo);
	if (ret)
		return ret;

	/* If stream is deinterlace, it will be automatically deinterlaced */
	frameinfo->field = V4L2_FIELD_NONE;
	ctx->pp_frameinfo = *frameinfo;

	return 0;
}

static int g1_vp8_decode(struct g1_ctx *pctx, struct g1_au *pau)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vp8_ctx *ctx = to_ctx(pctx);
	int ret;
	VP8DecRet vp8ret;
	VP8DecInput in;
	VP8DecOutput out;
	VP8DecInfo decinfo;
	struct g1_au au;
	struct g1_buf buf;
	unsigned int next_au = 0;
	struct g1_au *curr_au;

next_au:
	if (ctx->pending_au) {
		/* access unit pending, decode it first */
		au = *ctx->pending_au;
		/* FIXME, perhaps to move above in v4l2.c and
		 * ask for several AUs...
		 */
		curr_au = ctx->pending_au;
		ctx->pending_au = NULL;
		next_au = 1;
		dev_dbg(g1->dev, "%s  processing pending au[%p, %d]\n",
			pctx->name, curr_au->vaddr, curr_au->size);
	} else {
		/* free pending buf if any */
		if (ctx->pending_buf) {
			hw_free(pctx, ctx->pending_buf);
			ctx->pending_buf = NULL;
		}

		curr_au = pau;
		next_au = 0;
		dev_dbg(g1->dev, "%s  processing au[%p, %d]\n",
			pctx->name, curr_au->vaddr, curr_au->size);
	}
	au = *curr_au;

	if (ctx->state >= G1_VP8_HEADER_PARSED) {
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

	memset(&in, 0, sizeof(in));
	in.pStream = au.vaddr;
	in.streamBusAddress = au.paddr;
	in.dataLen = au.size;
	memset(&out, 0, sizeof(out));

	/* decode this access unit */
	vp8ret = VP8DecDecode(ctx->vp8, &in, &out);

	/* check decode status */
	ret = to_ret(vp8ret);
	if (ret) {
		dev_err(g1->dev, "%s  VP8DecDecode error %s(%d)\n", pctx->name,
			vp8ret_str(vp8ret), vp8ret);
		pctx->decode_errors++;
		return ret;
	}

	if (is_stream_error(vp8ret)) {
		dev_warn_ratelimited(g1->dev,
				     "%s  stream error @ frame %d, au size=%d\n",
				     pctx->name, pctx->decoded_frames,
				     au.size);
		pctx->stream_errors++;
		return 0;
	}

	if (vp8ret == VP8DEC_HDRS_RDY) {
		/* header detected */

		/* get new stream infos */
		vp8ret = VP8DecGetInfo(ctx->vp8, &decinfo);
		if (vp8ret) {
			dev_err(g1->dev, "%s  VP8DecGetInfo error %s(%d)\n",
				pctx->name, vp8ret_str(vp8ret), vp8ret);
			return -EIO;
		}

		/* new live header ? */
		if (ctx->state >= G1_VP8_HEADER_PARSED) {
			/* header already detected previously */
			dev_info(g1->dev,
				 "%s  new header detection while decoding\n",
				 pctx->name);
			return -ENODATA;
		}

		/* first header, store infos */
		ret = g1_vp8_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_vp8_set_dec_frameinfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* initialize post-proc output frame infos
		 * with decoder output frame infos
		 */
		ctx->pp_frameinfo = ctx->dec_frameinfo;
		/* and let post-proc check & update infos to what it can do */
		ret = g1_pp_check_config(pctx, &ctx->dec_frameinfo,
					 &ctx->pp_frameinfo);
		if (ret)
			return ret;

		ctx->state = G1_VP8_HEADER_PARSED;

		/* FIXME, tmp hack to treat the case where header is given with
		 * a frame. As VP8 stack is not returning the amount of bytes
		 * parsed, the header size cannot be known to eventually detect
		 * that not all input au has been parsed (so potentially a
		 * frame is lost).
		 * To overcome this, the access unit detected as header is
		 * systematically re-decoded.
		 */
		ret = hw_alloc(pctx, au.size,
			       "VP8 header temp access unit", &buf);
		if (ret)
			return -ENOMEM;

		ctx->pending_au_struct  = au;
		memcpy(buf.vaddr, au.vaddr, au.size);
		ctx->pending_au_struct.vaddr = buf.vaddr;
		ctx->pending_au_struct.paddr = buf.paddr;
		ctx->pending_au = &ctx->pending_au_struct;

		ctx->pending_buf_struct = buf;
		ctx->pending_buf = &ctx->pending_buf_struct;

		goto out;
	}

	switch (vp8ret) {
	case VP8DEC_OK:
	case VP8DEC_STRM_PROCESSED:
	case VP8DEC_ADVANCED_TOOLS:
	case VP8DEC_SLICE_RDY:
	case VP8DEC_END_OF_STREAM:
		/* output (decoded data) not expected so return nodata */
		return -ENODATA;
	case VP8DEC_PIC_DECODED:
		/* the frame is decoded, but should not be displayed
		 * ("invisible" VP8 frame)
		 */
		if (!VP8DecShowFrame(ctx->vp8))
			return -ENODATA;

		/* frame decoded */
		pctx->decoded_frames++;

		dev_dbg(g1->dev, "%s  dec picture cnt=%d\n",
			pctx->name,
			pctx->decoded_frames);
		goto out;
	default:
		/* no error detected at this point but we didn't treat
		 * the returned value ... Strange !!!
		 * log a trace to investigate if needed
		 */
		dev_err_ratelimited(g1->dev,
				    "%s - Untreated decode returned value : %s(%d)\n",
				    pctx->name, vp8ret_str(vp8ret), vp8ret);
		return -EIO;
	}

	/* FIXME, implement the VP8 workaround,
	 * cf VP8_HWTIMEOUT_WORKAROUND in 81x0_decoder/codec_vp8.c
	 */

out:
	if (next_au)
		goto next_au;

	return 0;
}

static int g1_vp8_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vp8_ctx *ctx = to_ctx(pctx);
	int ret;
	VP8DecRet vp8ret;
	VP8DecPicture picture;

	/* systematically check if we need to allocate a free frame
	 * to the PP.
	 * mainly useful in case of drain to store the last reference
	 * frame ( scheduled by g1_decoder_stop_cmd )
	 */
	if (ctx->state >= G1_VP8_HEADER_PARSED) {
		struct g1_frame *pframe;
		struct g1_frameinfo *dec_frameinfo = &ctx->dec_frameinfo;

		/* get a free output frame where to decode */
		ret = g1_get_free_frame(pctx, &pframe);
		if (ret) {
			if (ret == -EAGAIN)
				goto nodata;
			return ret;
		}

		/* setup post-processor */
		ret = g1_pp_set_config(pctx, dec_frameinfo, pframe);
		if (ret)
			return ret;
	}

	vp8ret = VP8DecNextPicture(ctx->vp8, &picture, ctx->eos);
	ret = to_ret(vp8ret);
	if (ret) {
		dev_err(g1->dev, "%s  VP8DecNextPicture error %s(%d)\n",
			pctx->name, vp8ret_str(vp8ret), vp8ret);
		pctx->decode_errors++;
		return ret;
	}

	if ((vp8ret != VP8DEC_PIC_RDY) && (vp8ret != VP8DEC_PIC_DECODED)) {
		/* no frame decoded */
		goto nodata;
	}

	/* decoded frame available */

	/* get the post-processed frame */
	ret = g1_pp_get_frame(pctx, frame);
	if (ret)
		return ret;

	/* update frame state */
	if (picture.isIntraFrame)
		(*frame)->flags |= V4L2_BUF_FLAG_KEYFRAME;
	else
		(*frame)->flags |= V4L2_BUF_FLAG_PFRAME;
				/*FIXME no B in VP8 ? */

	if (ctx->state == G1_VP8_FLUSHING) {
		/* Seek case : In flushing state, while there is no
		 * decoded key frame, don't display anything as we're
		 * not sure frames are sane.
		 */
		if (picture.isIntraFrame) {
			dev_dbg(g1->dev,
				"FLUSHING STATE : I frame received\n");
			ctx->state = G1_VP8_HEADER_PARSED;
		} else {
			(*frame)->to_drop = true;
		}
	}

	dev_dbg(g1->dev, "%s  out frame[%d] %s cnt=%d %s\n",
		pctx->name,
		(*frame)->index,
		frame_type_str((*frame)->flags),
		pctx->decoded_frames,
		frame_state_str((*frame)->state,
				pctx->str, sizeof(pctx->str)));

	return 0;

nodata:
	if (ctx->eos)
		ctx->eos = 0; /* no more frames to drain: end of EOS */

	return -ENODATA;
}

static void g1_vp8_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
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

static void g1_vp8_flush(struct g1_ctx *pctx)
{
	struct g1_vp8_ctx *ctx = to_ctx(pctx);

	ctx->state = G1_VP8_FLUSHING;
}

static void g1_vp8_drain(struct g1_ctx *pctx)
{
	struct g1_vp8_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

/* VP8 decoder can decode VP7 and WEBP contents, add below
 * one decoder struct per supported format
 */
const __u32 vp8_stream_formats[] = {
	V4L2_PIX_FMT_VP8
};

const struct g1_dec vp8dec = {
	.name = "vp8",
	.streamformat = &vp8_stream_formats[0],
	.nb_streams = ARRAY_SIZE(vp8_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_vp8_probe,
	.open = g1_vp8_open,
	.close = g1_vp8_close,
	.get_streaminfo = g1_vp8_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_vp8_get_frameinfo,
	.set_frameinfo = g1_vp8_set_frameinfo,
	.decode = g1_vp8_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_vp8_get_frame,
	.recycle = g1_vp8_recycle,
	.flush = g1_vp8_flush,
	.drain = g1_vp8_drain,
};
