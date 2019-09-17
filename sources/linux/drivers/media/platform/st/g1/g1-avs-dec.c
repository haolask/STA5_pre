/*
 * Copyright (C) STMicroelectronics SA 2016
 * Authors: Yannick Fertre <yannick.fertre@st.com>
 *          Hugues Fruchet <hugues.fruchet@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include "g1.h"
#include "g1-cfg.h"

#include "stack/inc/avsdecapi.h"

#define G1_AVS_HEADER_PARSED	1
#define G1_AVS_FLUSHING 2

struct g1_avs_ctx {
	AvsDecInst avs;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;

	unsigned int state;

	unsigned int eos;
};

#define to_ctx(ctx) ((struct g1_avs_ctx *)(ctx)->priv)

/* debug API of avs stack */
void AvsDecTrace(const char *str)
{
#ifdef AVSDEC_TRACE
	pr_debug("   %s\n", str);
#endif
};

#define CAVS_VIDEO_PROFILE_JIZHUN	0x20

#define CAVS_VIDEO_LEVEL_2_0	0x10
#define CAVS_VIDEO_LEVEL_2_1	0x11
#define CAVS_VIDEO_LEVEL_4_0	0x20
#define CAVS_VIDEO_LEVEL_4_2	0x22
#define CAVS_VIDEO_LEVEL_6_0	0x40
#define CAVS_VIDEO_LEVEL_6_0_1	0x41
#define CAVS_VIDEO_LEVEL_6_2	0x42

static inline const char *avs_video_profiles_to_str(unsigned int pl)
{
	switch (pl) {
	case CAVS_VIDEO_PROFILE_JIZHUN:
		return "Jizhun";
	default:
		return "not supported";
	}
}

static inline const char *avs_video_level_to_str(unsigned int pl)
{
	switch (pl) {
	case CAVS_VIDEO_LEVEL_2_0:
		return "2.0";
	case CAVS_VIDEO_LEVEL_2_1:
		return "2.1";
	case CAVS_VIDEO_LEVEL_4_0:
		return "4.0";
	case CAVS_VIDEO_LEVEL_4_2:
		return "4.2";
	case CAVS_VIDEO_LEVEL_6_0:
		return "6.0";
	case CAVS_VIDEO_LEVEL_6_0_1:
		return "6.0.1";
	case CAVS_VIDEO_LEVEL_6_2:
		return "6.2";
	default:
		return "Unknown level";
	}
}

static inline int to_ret(AvsDecRet ret)
{
	switch (ret) {
	case AVSDEC_OK:
	case AVSDEC_STRM_PROCESSED:
	case AVSDEC_PIC_RDY:
	case AVSDEC_PIC_DECODED:
	case AVSDEC_HDRS_RDY:
	case AVSDEC_HDRS_NOT_RDY:
	case AVSDEC_NONREF_PIC_SKIPPED:
		return 0;
	case AVSDEC_STRM_ERROR:
		return 0;

	case AVSDEC_MEMFAIL:
		return -ENOMEM;

	default:
		return -EIO;
	}
}

static inline int is_stream_error(AvsDecRet ret)
{
	switch (ret) {
	case AVSDEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *avsret_str(AvsDecRet ret)
{
	switch (ret) {
	case AVSDEC_OK:
		return "AVSDEC_OK";
	case AVSDEC_STRM_PROCESSED:
		return "AVSDEC_STRM_PROCESSED";
	case AVSDEC_PIC_RDY:
		return "AVSDEC_PIC_RDY";
	case AVSDEC_HDRS_RDY:
		return "AVSDEC_HDRS_RDY";
	case AVSDEC_HDRS_NOT_RDY:
		return "AVSDEC_HDRS_NOT_RDY";
	case AVSDEC_PIC_DECODED:
		return "AVSDEC_PIC_DECODED";
	case AVSDEC_NONREF_PIC_SKIPPED:
		return "AVSDEC_NONREF_PIC_SKIPPED";
	case AVSDEC_PARAM_ERROR:
		return "AVSDEC_PARAM_ERROR";
	case AVSDEC_STRM_ERROR:
		return "AVSDEC_STRM_ERROR";
	case AVSDEC_NOT_INITIALIZED:
		return "AVSDEC_NOT_INITIALIZED";
	case AVSDEC_MEMFAIL:
		return "AVSDEC_MEMFAIL";
	case AVSDEC_INITFAIL:
		return "AVSDEC_INITFAIL";
	case AVSDEC_STREAM_NOT_SUPPORTED:
		return "AVSDEC_STREAM_NOT_SUPPORTED";
	case AVSDEC_HW_RESERVED:
		return "AVSDEC_HW_RESERVED";
	case AVSDEC_HW_TIMEOUT:
		return "AVSDEC_HW_TIMEOUT";
	case AVSDEC_HW_BUS_ERROR:
		return "AVSDEC_HW_BUS_ERROR";
	case AVSDEC_SYSTEM_ERROR:
		return "AVSDEC_SYSTEM_ERROR";
	case AVSDEC_DWL_ERROR:
		return "AVSDEC_DWL_ERROR";
	case AVSDEC_FORMAT_NOT_SUPPORTED:
		return "AVSDEC_FORMAT_NOT_SUPPORTED";
	default:
		return "!unknown AVS return value!";
	}
}

static inline int to_v4l2_pixelformat(AvsDecOutFormat fmt)
{
	switch (fmt) {
	case AVSDEC_SEMIPLANAR_YUV420:
		return V4L2_PIX_FMT_NV12;
	default:
		return 0;
	}
}

static int g1_avs_probe(struct g1_dev *g1)
{
	/* check if codec is supported by hardware */
	if (!g1->hw.config.avs_support)
		return -EACCES;

	return 0;
}

static int g1_avs_open(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_avs_ctx *ctx;
	AvsDecRet avsret;
	int ret;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	pctx->priv = ctx;

	avsret = AvsDecInit(&ctx->avs,
			    false, /* no video freeze concealment */
			    0,
			    DEC_REF_FRM_RASTER_SCAN /* no tiled output */,
			    (void *)pctx);
	if (avsret) {
		dev_err(g1->dev, "%s  AvsDecInit error %s(%d)\n", pctx->name,
			avsret_str(avsret), avsret);
		ret = -EIO;
		goto err_free;
	}

	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));
	ret = g1_pp_open(pctx, ctx->avs, ctx->streaminfo.streamformat);
	if (ret)
		goto err_release;

	return 0;

err_release:
	AvsDecRelease(ctx->avs);
err_free:
	kfree(ctx);
	return ret;
}

static int g1_avs_close(struct g1_ctx *pctx)
{
	struct g1_avs_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	AvsDecRelease(ctx->avs);

	kfree(ctx);

	return 0;
}

static int g1_avs_set_streaminfo(struct g1_ctx *pctx, AvsDecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_avs_ctx *ctx = to_ctx(pctx);
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

	streaminfo->width = decinfo->codedWidth;
	streaminfo->height = decinfo->codedHeight;

	/* interlaced */
	streaminfo->field = (decinfo->interlacedSequence ?
			    V4L2_FIELD_INTERLACED :
			    V4L2_FIELD_NONE);

	/* profile & level */
	sprintf(streaminfo->profile, "profile %s",
		avs_video_profiles_to_str(decinfo->profileId));
	sprintf(streaminfo->level, "level %s",
		avs_video_level_to_str(decinfo->levelId));

	streaminfo->dpb = 1;
	return 0;
}

static int g1_avs_set_dec_frameinfo(struct g1_ctx *pctx, AvsDecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_avs_ctx *ctx = to_ctx(pctx);
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

static int g1_avs_get_streaminfo(struct g1_ctx *pctx,
				 struct g1_streaminfo *streaminfo)
{
	struct g1_avs_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_AVS_HEADER_PARSED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_avs_get_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_avs_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_AVS_HEADER_PARSED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_avs_set_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_avs_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_AVS_HEADER_PARSED)
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

static int g1_avs_decode(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_avs_ctx *ctx = to_ctx(pctx);
	int ret;
	AvsDecRet avsret;
	AvsDecInput in;
	AvsDecOutput out;
	AvsDecInfo decinfo;

	dev_dbg(g1->dev, "%s  > %s\n", pctx->name, __func__);

	if (ctx->state >= G1_AVS_HEADER_PARSED) {
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
	avsret = AvsDecDecode(ctx->avs, &in, &out);

	/* check decode status */
	ret = to_ret(avsret);
	if (ret) {
		dev_err(g1->dev, "%s  AvsDecDecode error %s(%d)",
			pctx->name, avsret_str(avsret), avsret);
		pctx->decode_errors++;
		return ret;
	}

	if (is_stream_error(avsret)) {
		dev_warn_ratelimited(g1->dev,
				     "%s  stream error @ frame %d, au size=%d\n",
				     pctx->name, pctx->decoded_frames,
				     au->size);
		pctx->stream_errors++;
		return 0;
	}
	au->remaining_data = out.dataLeft;

	if (avsret == AVSDEC_HDRS_RDY) {
		/* header detected */

		/* get new stream infos */
		avsret = AvsDecGetInfo(ctx->avs, &decinfo);
		if (avsret) {
			dev_err(g1->dev, "%s  AvsDecGetInfo error (%d)\n",
				pctx->name,  avsret);
			return -EIO;
		}

		/* new live header ? */
		if (ctx->state >= G1_AVS_HEADER_PARSED) {
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
		ret = g1_avs_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_avs_set_dec_frameinfo(pctx, &decinfo);
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

		ctx->state = G1_AVS_HEADER_PARSED;
		return -ENODATA;
	}

	switch (avsret) {
	case AVSDEC_NONREF_PIC_SKIPPED:
	case AVSDEC_STRM_PROCESSED:
	case AVSDEC_OK:
	case AVSDEC_PIC_RDY:
	case AVSDEC_HDRS_NOT_RDY:
		/* output (decoded data) not expected so return nodata */
		return -ENODATA;
	case AVSDEC_PIC_DECODED:
	{
		AvsDecPicture decoded_pic_info;

		/* In case of interlaced stream (field mode),
		 * 2 AUs are treated to get one frame, so we
		 * need to inform v4l2 to do discard the push_dts
		 */
		AvsDecPeek(ctx->avs, &decoded_pic_info);
		if (decoded_pic_info.interlaced &&
		    decoded_pic_info.fieldPicture)
			return -ENODATA;

		/* frame decoded */
		pctx->decoded_frames++;

		dev_dbg(g1->dev, "%s  dec picture cnt=%d (return : %d)\n",
			pctx->name,
			pctx->decoded_frames, ret);

		return 0;
	}
	default:
		/* no error detected at this point but we didn't treat
		 * the returned value ... Strange !!!
		 * log a trace to investigate if needed
		 */
		dev_err_ratelimited(g1->dev,
				    "%s - Untreated decode returned value : %s(%d)\n",
				    pctx->name, avsret_str(avsret), avsret);
		return -EIO;
	}
}

static int g1_avs_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_avs_ctx *ctx = to_ctx(pctx);
	int ret;
	AvsDecRet avsret;
	AvsDecPicture picture;
	u32 pictureId = 0;

	memset(&picture, 0, sizeof(picture));

	/* systematically check if we need to allocate a free frame
	 * to the PP.
	 * mainly useful in case of drain to store the last reference
	 * frame ( scheduled by g1_decoder_stop_cmd )
	 */
	if (ctx->state >= G1_AVS_HEADER_PARSED) {
		struct g1_frame *thisframe;
		struct g1_frameinfo *dec_frameinfo = &ctx->dec_frameinfo;

		/* get a free output frame where to decode */
		ret = g1_get_free_frame(pctx, &thisframe);
		if (ret) {
			if (ret == -EAGAIN)
				goto nodata;
			return ret;
		}
		if (ret == 0) {
			/* setup post-processor */
			ret = g1_pp_set_config(pctx, dec_frameinfo, thisframe);
			if (ret)
				return ret;
		}
	}

	avsret = AvsDecNextPicture(ctx->avs, &picture, ctx->eos);
	ret = to_ret(avsret);
	if (ret) {
		dev_err(g1->dev, "%s  AvsDecNextPicture error %s(%d)",
			pctx->name,
			avsret_str(avsret), avsret);
		pctx->decode_errors++;
		return ret;
	}

	if (avsret != AVSDEC_PIC_RDY)
		goto nodata;/* no frame decoded */

	/* decoded frame available */

	/* get the post-processed frame */
	ret = g1_pp_get_frame(pctx, frame);
	if (ret)
		return ret;

	/* update frame flags */
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

	if (ctx->state == G1_AVS_FLUSHING) {
		/* Seek case : In flushing state, while there is no
		 * decoded key frame, don't display anything as we're
		 * not sure frames are sane.
		 */
		if (picture.keyPicture) {
			dev_dbg(g1->dev,
				"FLUSHING STATE : I frame received\n");
			ctx->state = G1_AVS_HEADER_PARSED;
		} else {
			(*frame)->to_drop = true;
		}
	}

	return 0;

nodata:
	if (ctx->eos)
		ctx->eos = 0; /* no more frames to drain: end of EOS */

	return -ENODATA;
}

static void g1_avs_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
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

static void g1_avs_flush(struct g1_ctx *pctx)
{
	struct g1_avs_ctx *ctx = to_ctx(pctx);

	ctx->state = G1_AVS_FLUSHING;
}

static void g1_avs_drain(struct g1_ctx *pctx)
{
	struct g1_avs_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

/* AVS decoder can decode AVS contents, add below
 *  one decoder struct per supported format
 */

static const __u32 cavs_stream_formats[] = {
	V4L2_PIX_FMT_CAVS,
};

const struct g1_dec avsdec = {
	.name = "avs",
	.streamformat = &cavs_stream_formats[0],
	.nb_streams = ARRAY_SIZE(cavs_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_avs_probe,
	.open = g1_avs_open,
	.close = g1_avs_close,
	.get_streaminfo = g1_avs_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_avs_get_frameinfo,
	.set_frameinfo = g1_avs_set_frameinfo,
	.decode = g1_avs_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_avs_get_frame,
	.recycle = g1_avs_recycle,
	.flush = g1_avs_flush,
	.drain = g1_avs_drain,
};
