/*
 * Copyright (C) STMicroelectronics SA 2016
 * Authors: Stephane Danieau <stephane.danieau@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/slab.h>

#include "g1.h"
#include "g1-cfg.h"
#include "g1-mem.h"

#include "stack/inc/mpeg2decapi.h"
#include "stack/common/bqueue.h"

#define G1_MPEG2_HEADER_PARSED	1
#define G1_MPEG2_FLUSHING 2

#define FF_PROFILE_MPEG2_422    0
#define FF_PROFILE_MPEG2_HIGH   1
#define FF_PROFILE_MPEG2_SS     2
#define FF_PROFILE_MPEG2_SNR_SCALABLE  3
#define FF_PROFILE_MPEG2_MAIN   4
#define FF_PROFILE_MPEG2_SIMPLE 5
#define FF_PROFILE_UNKNOWN -99
#define FF_PROFILE_RESERVED -100

struct g1_mpeg2_ctx {
	Mpeg2DecInst mpeg2;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;

	unsigned int state;
	unsigned int eos;
};

#define to_ctx(ctx) ((struct g1_mpeg2_ctx *)(ctx)->priv)

/* debug API of MPEG2 stack */
void Mpeg2DecTrace(const char *str)
{
#ifdef MPEG2DEC_TRACE
	pr_debug("   %s\n", str);
#endif
};

static inline const char *mpeg2_video_profiles_to_str(unsigned int pl)
{
	unsigned int p = pl;

	if (pl < 130)
		p = pl >> 4;

	switch (p) {
	case FF_PROFILE_MPEG2_HIGH:
		return "High profile (HP)";
	case FF_PROFILE_MPEG2_SS:
		return "Spatially scalable profile (Spatial)";
	case FF_PROFILE_MPEG2_SNR_SCALABLE:
		return "SNR Scalable profile (SNR)";
	case FF_PROFILE_MPEG2_MAIN:
		return "Main profile (MP)";
	case FF_PROFILE_MPEG2_SIMPLE:
		return "Simple profile (SP)";
	case 130:
		return "422@HL profile";
	case 133:
		return "422@ML profile";
	default:
		return "Unknown profile";
	}
}

static inline const char *mpeg2_video_level_to_str(unsigned int pl)
{
	unsigned int l = 0;

	if (pl < 130)
		l = pl & 0xF;

	switch (l) {
	case 0:
		return "";
	case 4:
		return "High level (HL)";
	case 6:
		return "High 1440 level (H-14)";
	case 8:
		return "Main level (ML)";
	case 10:
		return "Low level (LL)";
	default:
		return "Unknown level";
	}
}

static inline int to_ret(Mpeg2DecRet ret)
{
	switch (ret) {
	case MPEG2DEC_OK:
	case MPEG2DEC_STRM_PROCESSED:
	case MPEG2DEC_PIC_RDY:
	case MPEG2DEC_PIC_DECODED:
	case MPEG2DEC_HDRS_RDY:
	case MPEG2DEC_HDRS_NOT_RDY:
	case MPEG2DEC_NONREF_PIC_SKIPPED:
	case MPEG2DEC_STRM_ERROR:
		return 0;
	case MPEG2DEC_MEMFAIL:
	case MPEG2DEC_INITFAIL:
		return -ENOMEM;
	case MPEG2DEC_STREAM_NOT_SUPPORTED:
	case MPEG2DEC_NOT_INITIALIZED:
	case MPEG2DEC_PARAM_ERROR:
	case MPEG2DEC_HW_RESERVED:
	case MPEG2DEC_HW_TIMEOUT:
	case MPEG2DEC_HW_BUS_ERROR:
	case MPEG2DEC_SYSTEM_ERROR:
	case MPEG2DEC_DWL_ERROR:
	case MPEG2DEC_FORMAT_NOT_SUPPORTED:
		return -EINVAL;
	default:
		return -EIO;
	}
}

static inline int is_stream_error(Mpeg2DecRet ret)
{
	switch (ret) {
	case MPEG2DEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *mpeg2ret_str(Mpeg2DecRet ret)
{
	switch (ret) {
	case MPEG2DEC_OK:
		return "MPEG2DEC_OK";
	case MPEG2DEC_STRM_PROCESSED:
		return "MPEG2DEC_STRM_PROCESSED";
	case MPEG2DEC_PIC_RDY:
		return "MPEG2DEC_PIC_RDY";
	case MPEG2DEC_HDRS_RDY:
		return "MPEG2DEC_HDRS_RDY";
	case MPEG2DEC_HDRS_NOT_RDY:
		return "MPEG2DEC_HDRS_NOT_RDY";
	case MPEG2DEC_PIC_DECODED:
		return "MPEG2DEC_PIC_DECODED";
	case MPEG2DEC_NONREF_PIC_SKIPPED:
		return "MPEG2DEC_NONREF_PIC_SKIPPED";
	case MPEG2DEC_PARAM_ERROR:
		return "MPEG2DEC_PARAM_ERROR";
	case MPEG2DEC_STRM_ERROR:
		return "MPEG2DEC_STRM_ERROR";
	case MPEG2DEC_NOT_INITIALIZED:
		return "MPEG2DEC_NOT_INITIALIZED";
	case MPEG2DEC_MEMFAIL:
		return "MPEG2DEC_MEMFAIL";
	case MPEG2DEC_INITFAIL:
		return "MPEG2DEC_INITFAIL";
	case MPEG2DEC_STREAM_NOT_SUPPORTED:
		return "MPEG2DEC_STREAM_NOT_SUPPORTED";
	case MPEG2DEC_HW_RESERVED:
		return "MPEG2DEC_HW_RESERVED";
	case MPEG2DEC_HW_TIMEOUT:
		return "MPEG2DEC_HW_TIMEOUT";
	case MPEG2DEC_HW_BUS_ERROR:
		return "MPEG2DEC_HW_BUS_ERROR";
	case MPEG2DEC_SYSTEM_ERROR:
		return "MPEG2DEC_SYSTEM_ERROR";
	case MPEG2DEC_DWL_ERROR:
		return "MPEG2DEC_DWL_ERROR";
	case MPEG2DEC_FORMAT_NOT_SUPPORTED:
		return "MPEG2DEC_FORMAT_NOT_SUPPORTED";
	default:
		return "!unknown MPEG2 returned value!";
	}
}

static inline int to_v4l2_pixelformat(Mpeg2DecOutFormat fmt)
{
	switch (fmt) {
	case MPEG2DEC_SEMIPLANAR_YUV420:
		return V4L2_PIX_FMT_NV12;
	default:
		return 0;
	}
}

static int g1_mpeg2_probe(struct g1_dev *g1)
{
	/* check if MPEG2 codec is supported by hardware */
	if (!g1->hw.config.mpeg2_support)
		return -EACCES;

	return 0;
}

static int g1_mpeg2_open(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mpeg2_ctx *ctx;
	Mpeg2DecRet mpeg2ret;
	int ret;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	pctx->priv = ctx;

	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));

	switch (ctx->streaminfo.streamformat) {
	case V4L2_PIX_FMT_MPEG2:
	case V4L2_PIX_FMT_MPEG1:
		break;
	default:
		ret = -EACCES;
		goto err_free;
	}

	mpeg2ret = Mpeg2DecInit(&ctx->mpeg2,
				/* Conceal only the frames having errors */
				DEC_EC_PICTURE_FREEZE,
				0, /* no custom buffers */
				DEC_REF_FRM_RASTER_SCAN, /* no tiled output */
				(void *)pctx); /* forward g1 context to DWL */
	if (mpeg2ret) {
		dev_err(g1->dev, "%s  Mpeg2DecInit error %s(%d)\n", pctx->name,
			mpeg2ret_str(mpeg2ret), mpeg2ret);
		ret = -EIO;
		goto err_free;
	}

	ret = g1_pp_open(pctx, ctx->mpeg2, ctx->streaminfo.streamformat);
	if (ret)
		goto err_release;

	return 0;

err_release:
	Mpeg2DecRelease(ctx->mpeg2);
err_free:
	kfree(ctx);
	return ret;
}

static int g1_mpeg2_close(struct g1_ctx *pctx)
{
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	Mpeg2DecRelease(ctx->mpeg2);

	kfree(ctx);

	return 0;
}

static int g1_mpeg2_set_streaminfo(struct g1_ctx *pctx, Mpeg2DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);
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

	streaminfo->streamformat = V4L2_PIX_FMT_MPEG1;
	if (decinfo->streamFormat)
		streaminfo->streamformat = V4L2_PIX_FMT_MPEG2;

	streaminfo->width = decinfo->codedWidth;
	streaminfo->height = decinfo->codedHeight;

	/* pixel-aspect-ratio */
	switch (decinfo->displayAspectRatio) {
	case MPEG2DEC_4_3:
		streaminfo->pixelaspect.numerator = 4;
		streaminfo->pixelaspect.denominator = 3;
		break;
	case MPEG2DEC_16_9:
		streaminfo->pixelaspect.numerator = 16;
		streaminfo->pixelaspect.denominator = 9;
		break;
	case MPEG2DEC_2_21_1:
		streaminfo->pixelaspect.numerator = 221;
		streaminfo->pixelaspect.denominator = 100;
		break;
	case MPEG2DEC_1_1:
	default:
		streaminfo->pixelaspect.numerator = 1;
		streaminfo->pixelaspect.denominator = 1;
		break;
	}

	/* interlaced */
	streaminfo->field = (decinfo->interlacedSequence ?
			    V4L2_FIELD_INTERLACED :
			    V4L2_FIELD_NONE);

	streaminfo->dpb = 0;

	/* profile & level */
	sprintf(streaminfo->profile, "%s",
		mpeg2_video_profiles_to_str
			(decinfo->profileAndLevelIndication));
	sprintf(streaminfo->level, "%s",
		mpeg2_video_level_to_str
			(decinfo->profileAndLevelIndication));
	return 0;
}

static int g1_mpeg2_set_dec_frameinfo(struct g1_ctx *pctx,
				      Mpeg2DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);
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

	/* pixel-aspect-ratio */
	switch (decinfo->displayAspectRatio) {
	case MPEG2DEC_4_3:
		frameinfo->pixelaspect.numerator = 4;
		frameinfo->pixelaspect.denominator = 3;
		break;
	case MPEG2DEC_16_9:
		frameinfo->pixelaspect.numerator = 16;
		frameinfo->pixelaspect.denominator = 9;
	case MPEG2DEC_2_21_1:
		frameinfo->pixelaspect.numerator = 221;
		frameinfo->pixelaspect.denominator = 100;
		break;
	case MPEG2DEC_1_1:
	default:
		frameinfo->pixelaspect.numerator = 1;
		frameinfo->pixelaspect.denominator = 1;
		break;
	}

	/* interlaced */
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

static int g1_mpeg2_get_streaminfo(struct g1_ctx *pctx,
				   struct g1_streaminfo *streaminfo)
{
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_MPEG2_HEADER_PARSED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_mpeg2_get_frameinfo(struct g1_ctx *pctx,
				  struct g1_frameinfo *frameinfo)
{
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_MPEG2_HEADER_PARSED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_mpeg2_set_frameinfo(struct g1_ctx *pctx,
				  struct g1_frameinfo *frameinfo)
{
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_MPEG2_HEADER_PARSED)
		return -ENODATA;

	/* let post-proc check & update infos to what it can do */
	ret = g1_pp_check_config(pctx, &ctx->dec_frameinfo,
				 frameinfo);
	if (ret)
		return ret;
	/* If stream is deinterlaced, it will be automatically deinterlaced */
	frameinfo->field = V4L2_FIELD_NONE;
	ctx->pp_frameinfo = *frameinfo;

	return 0;
}

static int g1_mpeg2_decode(struct g1_ctx *pctx, struct g1_au *pau)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);
	int ret;
	Mpeg2DecRet mpeg2ret;
	Mpeg2DecInput in;
	Mpeg2DecOutput out;
	Mpeg2DecInfo decinfo;

	dev_dbg(g1->dev, "%s  > %s\n", pctx->name, __func__);

	if (ctx->state >= G1_MPEG2_HEADER_PARSED) {
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
	in.pStream = pau->vaddr;
	in.streamBusAddress = pau->paddr;
	in.dataLen = pau->size;
	in.picId = pau->picid;
	memset(&out, 0, sizeof(out));

	/* decode this access unit */
	mpeg2ret = Mpeg2DecDecode(ctx->mpeg2, &in, &out);
	/* check decode status */
	ret = to_ret(mpeg2ret);
	if (ret) {
		dev_err(g1->dev, "%s  Mpeg2DecDecode error %s(%d)",
			pctx->name, mpeg2ret_str(mpeg2ret), mpeg2ret);
		pctx->decode_errors++;
		return ret;
	}

	if (is_stream_error(mpeg2ret)) {
		dev_warn_ratelimited(g1->dev,
				     "%s  stream error @ frame %d, au size=%d\n",
				     pctx->name, pctx->decoded_frames,
				     pau->size);
		pctx->stream_errors++;
		return 0;
	}
	pau->remaining_data = out.dataLeft;

	if (mpeg2ret == MPEG2DEC_HDRS_RDY) {
		/* header detected */

		/* get new stream infos */
		mpeg2ret = Mpeg2DecGetInfo(ctx->mpeg2, &decinfo);
		if (mpeg2ret) {
			dev_err(g1->dev, "%s  Mpeg2DecGetInfo error (%d)\n",
				pctx->name,  mpeg2ret);
			return -EIO;
		}

		/* new live header ? */
		if (ctx->state >= G1_MPEG2_HEADER_PARSED) {
			/* header already detected previously */
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
		ret = g1_mpeg2_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_mpeg2_set_dec_frameinfo(pctx, &decinfo);
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

		ctx->state = G1_MPEG2_HEADER_PARSED;
		return -ENODATA;
	}

	switch (mpeg2ret) {
	case MPEG2DEC_NONREF_PIC_SKIPPED:
	case MPEG2DEC_STRM_PROCESSED:
	case MPEG2DEC_OK:
	case MPEG2DEC_HDRS_NOT_RDY:
	case MPEG2DEC_PIC_RDY:
		/* output (decoded data) not expected so return nodata */
		return -ENODATA;
	case MPEG2DEC_PIC_DECODED:
		/* frame decoded */
		pctx->decoded_frames++;

		dev_dbg(g1->dev, "%s  dec picture cnt=%d\n",
			pctx->name,
			pctx->decoded_frames);
		return 0;
	default:
		/* no error detected at this point but we didn't treat
		 * the returned value ... Strange !!!
		 * log a trace to investigate if needed
		 */
		dev_err_ratelimited(g1->dev,
				    "%s - Untreated decode returned value : %s(%d)\n",
				    pctx->name, mpeg2ret_str(mpeg2ret),
				    mpeg2ret);
		return -EIO;
	}
}

static int g1_mpeg2_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);
	int ret;
	Mpeg2DecRet mpeg2ret;
	Mpeg2DecPicture picture;
	u32 pictureId = 0;

	/* systematically check if we need to allocate a free frame
	 * to the PP.
	 * mainly useful in case of drain to store the last reference
	 * frame ( scheduled by g1_decoder_stop_cmd )
	 */
	if (ctx->state >= G1_MPEG2_HEADER_PARSED) {
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

	mpeg2ret = Mpeg2DecNextPicture(ctx->mpeg2, &picture, ctx->eos);
	ret = to_ret(mpeg2ret);
	if (ret) {
		dev_err(g1->dev, "%s  Mpeg2DecNextPicture error %s(%d)\n",
			pctx->name, mpeg2ret_str(mpeg2ret), mpeg2ret);
		pctx->decode_errors++;
		return ret;
	}

	if ((mpeg2ret != MPEG2DEC_PIC_RDY) &&
	    (mpeg2ret != MPEG2DEC_PIC_DECODED)) {
		/* no frame decoded */
		goto nodata;
	}

	/* decoded frame available */

	/* get the post-processed frame */
	ret = g1_pp_get_frame(pctx, frame);
	if (ret)
		return ret;

	/* update frame state */
	if (picture.keyPicture)
		(*frame)->flags |= V4L2_BUF_FLAG_KEYFRAME;
	else
		(*frame)->flags |= V4L2_BUF_FLAG_PFRAME;

	(*frame)->pix.field = V4L2_FIELD_NONE;

	pictureId = picture.picId;

	if (picture.interlaced) {
		if (picture.fieldPicture)
			if (picture.topField)
				(*frame)->pix.field = V4L2_FIELD_TOP;
			else
				(*frame)->pix.field = V4L2_FIELD_BOTTOM;
		else
			(*frame)->pix.field = V4L2_FIELD_INTERLACED;
	}

	(*frame)->picid = pictureId;

	if (picture.numberOfErrMBs) {
		dev_warn_ratelimited(g1->dev,
				     "%s  Number of Concealed MB : %d\n",
				     pctx->name, picture.numberOfErrMBs);
		pctx->stream_errors++;
	}
	if (ctx->state == G1_MPEG2_FLUSHING) {
		/* Seek case : In flushing state, while there is no
		 * decoded key frame, don't display anything as we're
		 * not sure frames are sane.
		 */
		if (picture.keyPicture) {
			dev_dbg(g1->dev,
				"FLUSHING STATE : I frame received\n");
			ctx->state = G1_MPEG2_HEADER_PARSED;
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

static void g1_mpeg2_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
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

static void g1_mpeg2_flush(struct g1_ctx *pctx)
{
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);

	ctx->state = G1_MPEG2_FLUSHING;
}

static void g1_mpeg2_drain(struct g1_ctx *pctx)
{
	struct g1_mpeg2_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

/* the decoder conforms to the MPEG-2 [1] main profile
 * and can decode streams constrained to the hardware
 * configuration specific maximum value (up to 1920x1088 pixels).
 * The MPEG-2 decoder is also capable of decoding MPEG-1 streams.
 * Add below one decoder structure per supported format
 */
const __u32 mpeg2_stream_formats[] = {
	V4L2_PIX_FMT_MPEG2,
	V4L2_PIX_FMT_MPEG1
};

const struct g1_dec mpeg2dec = {
	.name = "mpeg2",
	.streamformat = &mpeg2_stream_formats[0],
	.nb_streams = ARRAY_SIZE(mpeg2_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_mpeg2_probe,
	.open = g1_mpeg2_open,
	.close = g1_mpeg2_close,
	.get_streaminfo = g1_mpeg2_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_mpeg2_get_frameinfo,
	.set_frameinfo = g1_mpeg2_set_frameinfo,
	.decode = g1_mpeg2_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_mpeg2_get_frame,
	.recycle = g1_mpeg2_recycle,
	.flush = g1_mpeg2_flush,
	.drain = g1_mpeg2_drain,
};
