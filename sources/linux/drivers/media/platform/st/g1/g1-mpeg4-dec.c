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

#include "stack/inc/mp4decapi.h"
#include "stack/common/bqueue.h"

#define G1_MP4_HEADER_PARSED	1
#define G1_MP4_FLUSHING	2

/* MPEG 4 profile definition
 * retrieved from avcodec.h (libavcodec source code)
 */
#define FF_PROFILE_MPEG4_SIMPLE                     0
#define FF_PROFILE_MPEG4_SIMPLE_SCALABLE            1
#define FF_PROFILE_MPEG4_CORE                       2
#define FF_PROFILE_MPEG4_MAIN                       3
#define FF_PROFILE_MPEG4_N_BIT                      4
#define FF_PROFILE_MPEG4_SCALABLE_TEXTURE           5
#define FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION      6
#define FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE     7
#define FF_PROFILE_MPEG4_HYBRID                     8
#define FF_PROFILE_MPEG4_ADVANCED_REAL_TIME         9
#define FF_PROFILE_MPEG4_CORE_SCALABLE             10
#define FF_PROFILE_MPEG4_ADVANCED_CODING           11
#define FF_PROFILE_MPEG4_ADVANCED_CORE             12
#define FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE 13
#define FF_PROFILE_MPEG4_SIMPLE_STUDIO             14
#define FF_PROFILE_MPEG4_ADVANCED_SIMPLE           15

static inline const char *mp4_video_profiles_to_str(int profile)
{
	switch (profile) {
	case FF_PROFILE_MPEG4_SIMPLE:
		return "Simple Profile";
	case FF_PROFILE_MPEG4_SIMPLE_SCALABLE:
		return "Simple Scalable Profile";
	case FF_PROFILE_MPEG4_CORE:
		return "Core Profile";
	case FF_PROFILE_MPEG4_MAIN:
		return "Main Profile";
	case FF_PROFILE_MPEG4_N_BIT:
		return "N-bit Profile";
	case FF_PROFILE_MPEG4_SCALABLE_TEXTURE:
		return "Scalable Texture Profile";
	case FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION:
		return "Simple Face Animation Profile";
	case FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE:
		return "Basic Animated Texture Profile";
	case FF_PROFILE_MPEG4_HYBRID:
		return "Hybrid Profile";
	case FF_PROFILE_MPEG4_ADVANCED_REAL_TIME:
		return "Advanced Real Time Simple Profile";
	case FF_PROFILE_MPEG4_CORE_SCALABLE:
		return "Code Scalable Profile";
	case FF_PROFILE_MPEG4_ADVANCED_CODING:
		return "Advanced Coding Profile";
	case FF_PROFILE_MPEG4_ADVANCED_CORE:
		return "Advanced Core Profile";
	case FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE:
		return "Advanced Scalable Texture Profile";
	case FF_PROFILE_MPEG4_SIMPLE_STUDIO:
		return "Simple Studio Profile";
	case FF_PROFILE_MPEG4_ADVANCED_SIMPLE:
		return "Advanced Simple Profile";
	default:
		return "Unknown profile";
	}
}

struct g1_mp4_ctx {
	MP4DecInst mp4;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;

	unsigned int state;

	unsigned int eos;
};

#define to_ctx(ctx) ((struct g1_mp4_ctx *)(ctx)->priv)

/* debug API of MP4 stack */
void MP4DecTrace(const char *str)
{
#ifdef MP4DEC_TRACE
	pr_debug("   %s\n", str);
#endif
};

static inline int to_ret(MP4DecRet ret)
{
	switch (ret) {
	case MP4DEC_OK:
	case MP4DEC_STRM_PROCESSED:
	case MP4DEC_PIC_RDY:
	case MP4DEC_PIC_DECODED:
	case MP4DEC_HDRS_RDY:
	case MP4DEC_DP_HDRS_RDY:
	case MP4DEC_HDRS_NOT_RDY:
	case MP4DEC_NONREF_PIC_SKIPPED:
	case MP4DEC_VOS_END:
		return 0;
	case MP4DEC_STRM_ERROR:
		return 0;

	case MP4DEC_MEMFAIL:
	case MP4DEC_DWL_ERROR:
		return -ENOMEM;

	case MP4DEC_STRM_NOT_SUPPORTED:
	case MP4DEC_PARAM_ERROR:
	case MP4DEC_NOT_INITIALIZED:
	case MP4DEC_INITFAIL:
	case MP4DEC_FORMAT_NOT_SUPPORTED:
	case MP4DEC_HW_RESERVED:
	case MP4DEC_HW_TIMEOUT:
	case MP4DEC_HW_BUS_ERROR:
	case MP4DEC_SYSTEM_ERROR:
		return -EINVAL;
	default:
		return -EIO;
	}
}

static inline int is_stream_error(MP4DecRet ret)
{
	switch (ret) {
	case MP4DEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *mp4ret_str(MP4DecRet ret)
{
	switch (ret) {
	case MP4DEC_OK:
		return "MP4DEC_OK";
	case MP4DEC_STRM_PROCESSED:
		return "MP4DEC_STRM_PROCESSED";
	case MP4DEC_PIC_RDY:
		return "MP4DEC_PIC_RDY";
	case MP4DEC_PIC_DECODED:
		return "MP4DEC_PIC_DECODED";
	case MP4DEC_HDRS_RDY:
		return "MP4DEC_HDRS_RDY";
	case MP4DEC_DP_HDRS_RDY:
		return "MP4DEC_DP_HDRS_RDY";
	case MP4DEC_NONREF_PIC_SKIPPED:
		return "MP4DEC_NONREF_PIC_SKIPPED";
	case MP4DEC_VOS_END:
		return "MP4DEC_VOS_END";
	case MP4DEC_HDRS_NOT_RDY:
		return "MP4DEC_HDRS_NOT_RDY";
	case MP4DEC_PARAM_ERROR:
		return "MP4DEC_PARAM_ERROR";
	case MP4DEC_STRM_ERROR:
		return "MP4DEC_STRM_ERROR";
	case MP4DEC_NOT_INITIALIZED:
		return "MP4DEC_NOT_INITIALIZED";
	case MP4DEC_MEMFAIL:
		return "MP4DEC_MEMFAIL";
	case MP4DEC_INITFAIL:
		return "MP4DEC_INITFAIL";
	case MP4DEC_FORMAT_NOT_SUPPORTED:
		return "MP4DEC_FORMAT_NOT_SUPPORTED";
	case MP4DEC_STRM_NOT_SUPPORTED:
		return "MP4DEC_STRM_NOT_SUPPORTED";
	case MP4DEC_HW_RESERVED:
		return "MP4DEC_HW_RESERVED";
	case MP4DEC_HW_TIMEOUT:
		return "MP4DEC_HW_TIMEOUT";
	case MP4DEC_HW_BUS_ERROR:
		return "MP4DEC_HW_BUS_ERROR";
	case MP4DEC_SYSTEM_ERROR:
		return "MP4DEC_SYSTEM_ERROR";
	case MP4DEC_DWL_ERROR:
		return  "MP4DEC_DWL_ERROR";
	default:
		return "!unknown MP4 returned value!";
	}
}

static inline int to_v4l2_pixelformat(MP4DecOutFormat fmt)
{
	switch (fmt) {
	case MP4DEC_SEMIPLANAR_YUV420:
		return V4L2_PIX_FMT_NV12;
	default:
		return 0;
	}
}

static int g1_mp4_probe(struct g1_dev *g1)
{
	/* check if MPEG4 codec is supported by hardware */
	if (!g1->hw.config.mpeg4_support)
		return -EACCES;

	return 0;
}

static int g1_mp4_open(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mp4_ctx *ctx;
	MP4DecRet mp4ret;
	int ret;
	MP4DecStrmFmt strmFormat;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	pctx->priv = ctx;
	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));

	switch (ctx->streaminfo.streamformat) {
	case V4L2_PIX_FMT_MPEG4:
	case V4L2_PIX_FMT_H263:
		strmFormat = MP4DEC_MPEG4;
		break;
	case V4L2_PIX_FMT_XVID:
		/* check if DivX codec is supported by hardware */
		if (!g1->hw.config.custom_mpeg4_support)
			return -EACCES;
		strmFormat = MP4DEC_CUSTOM_1;
		break;
#ifdef V4L2_PIX_FMT_FLV1
	case V4L2_PIX_FMT_FLV1:
		/* check if Sorenson Spark codec is supported by hardware */
		if (!g1->hw.config.sorenson_spark_support)
			return -EACCES;
		strmFormat = MP4DEC_SORENSON;
		break;
#endif
	default:
		return -EACCES;
	}

	mp4ret = MP4DecInit(&ctx->mp4, strmFormat,
			    0, /* no video freeze concealment */
			    0, /* no custom buffers */
			    DEC_REF_FRM_RASTER_SCAN, /* no tiled output */
			    (void *)pctx); /* forward g1 context to DWL */
	if (mp4ret) {
		dev_err(g1->dev, "%s  MP4DecInit error %s(%d)\n", pctx->name,
			mp4ret_str(mp4ret), mp4ret);
		ret = -EIO;
		goto err_free;
	}

	ret = g1_pp_open(pctx, ctx->mp4, ctx->streaminfo.streamformat);
	if (ret)
		goto err_release;

	return 0;

err_release:
	MP4DecRelease(ctx->mp4);
err_free:
	kfree(ctx);
	return ret;
}

static int g1_mp4_close(struct g1_ctx *pctx)
{
	struct g1_mp4_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	MP4DecRelease(ctx->mp4);

	kfree(ctx);

	return 0;
}

static int g1_mp4_set_streaminfo(struct g1_ctx *pctx, MP4DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mp4_ctx *ctx = to_ctx(pctx);
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;
	int profile = 0;
	int level = 0;

	/* check width/height */
	if ((decinfo->codedWidth == 0) || (decinfo->codedHeight == 0) ||
	    (decinfo->codedWidth * decinfo->codedHeight > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid stream coded resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, decinfo->codedWidth, decinfo->codedHeight,
			G1_MAX_RESO);
		return -EINVAL;
	}

	switch (decinfo->streamFormat) {
	case 0: /* MPEG-4 stream */
#ifdef V4L2_PIX_FMT_FLV1
		if ((streaminfo->streamformat != V4L2_PIX_FMT_MPEG4) &&
		    (streaminfo->streamformat != V4L2_PIX_FMT_XVID) &&
		    (streaminfo->streamformat != V4L2_PIX_FMT_FLV1))
#else
		if ((streaminfo->streamformat != V4L2_PIX_FMT_MPEG4) &&
		    (streaminfo->streamformat != V4L2_PIX_FMT_XVID))
#endif
			return -EINVAL;
		break;
	case 1: /* MPEG-4 short video stream */
		streaminfo->streamformat = V4L2_PIX_FMT_MPEG4;
		break;
	case 2: /* H.263 Baseline stream */
		if (streaminfo->streamformat != V4L2_PIX_FMT_H263)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}
	streaminfo->width = decinfo->codedWidth;
	streaminfo->height = decinfo->codedHeight;

	/* pixel-aspect-ratio */
	streaminfo->pixelaspect.numerator = decinfo->parWidth;
	streaminfo->pixelaspect.denominator = decinfo->parHeight;

	/* interlaced */
	streaminfo->field = (decinfo->interlacedSequence ?
			    V4L2_FIELD_INTERLACED :
			    V4L2_FIELD_NONE);

	streaminfo->dpb = 0;

	profile = (decinfo->profileAndLevelIndication & 0xf0) >> 4;
	level = decinfo->profileAndLevelIndication & 0x0f;
	sprintf(streaminfo->profile, "%s", mp4_video_profiles_to_str(profile));
	sprintf(streaminfo->level, "Level %d", level);

	return 0;
}

static int g1_mp4_set_dec_frameinfo(struct g1_ctx *pctx, MP4DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mp4_ctx *ctx = to_ctx(pctx);
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
	frameinfo->pixelaspect.numerator = decinfo->parWidth;
	frameinfo->pixelaspect.denominator = decinfo->parHeight;

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

static int g1_mp4_get_streaminfo(struct g1_ctx *pctx,
				 struct g1_streaminfo *streaminfo)
{
	struct g1_mp4_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_MP4_HEADER_PARSED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_mp4_get_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_mp4_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_MP4_HEADER_PARSED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_mp4_set_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_mp4_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_MP4_HEADER_PARSED)
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

static int g1_mp4_decode(struct g1_ctx *pctx, struct g1_au *pau)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mp4_ctx *ctx = to_ctx(pctx);
	int ret;
	MP4DecRet mp4ret;
	MP4DecInput in;
	MP4DecOutput out;
	MP4DecInfo decinfo;

	dev_dbg(g1->dev, "%s  > %s\n", pctx->name, __func__);

	if (ctx->state >= G1_MP4_HEADER_PARSED) {
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
	mp4ret = MP4DecDecode(ctx->mp4, &in, &out);
	/* check decode status */
	ret = to_ret(mp4ret);
	if (ret) {
		dev_err(g1->dev, "%s  MP4DecDecode error %s(%d)",
			pctx->name, mp4ret_str(mp4ret), mp4ret);
		pctx->decode_errors++;
		return ret;
	}

	if (is_stream_error(mp4ret)) {
		dev_warn_ratelimited(g1->dev,
				     "%s  stream error @ frame %d, au size=%d\n",
				     pctx->name, pctx->decoded_frames,
				     pau->size);
		pctx->stream_errors++;
		return 0;
	}
	pau->remaining_data = out.dataLeft;

	if (mp4ret == MP4DEC_HDRS_RDY ||
	    mp4ret == MP4DEC_DP_HDRS_RDY) {
		/* header detected */

		/* get new stream infos */
		mp4ret = MP4DecGetInfo(ctx->mp4, &decinfo);
		if (mp4ret) {
			dev_err(g1->dev, "%s  MP4DecGetInfo error (%d)\n",
				pctx->name,  mp4ret);
			return -EIO;
		}
		if (ctx->state >= G1_MP4_HEADER_PARSED) {
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
		ret = g1_mp4_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_mp4_set_dec_frameinfo(pctx, &decinfo);
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

		ctx->state = G1_MP4_HEADER_PARSED;
		return -ENODATA;
	}

	switch (mp4ret) {
	case MP4DEC_NONREF_PIC_SKIPPED:
	case MP4DEC_STRM_PROCESSED:
	case MP4DEC_OK:
	case MP4DEC_VOS_END:
	case MP4DEC_HDRS_NOT_RDY:
		/* output (decoded data) not expected so return nodata */
		return -ENODATA;
	case MP4DEC_PIC_DECODED:
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
				    pctx->name, mp4ret_str(mp4ret), mp4ret);
		return -EIO;
	}
}

static int g1_mp4_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_mp4_ctx *ctx = to_ctx(pctx);
	int ret;
	MP4DecRet mp4ret;
	MP4DecPicture picture;
	u32 pictureId = 0;

	/* systematically check if we need to allocate a free frame
	 * to the PP.
	 * mainly useful in case of drain to store the last reference
	 * frame ( scheduled by g1_decoder_stop_cmd )
	 */
	if (ctx->state >= G1_MP4_HEADER_PARSED) {
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

	mp4ret = MP4DecNextPicture(ctx->mp4, &picture, ctx->eos);
	ret = to_ret(mp4ret);
	if (ret) {
		dev_err(g1->dev, "%s  MP4DecNextPicture error %s(%d)\n",
			pctx->name, mp4ret_str(mp4ret), mp4ret);
		pctx->decode_errors++;
		return ret;
	}

	if ((mp4ret != MP4DEC_PIC_RDY) && (mp4ret != MP4DEC_PIC_DECODED)) {
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

	if (picture.nbrOfErrMBs) {
		dev_warn_ratelimited(g1->dev,
				     "%s  Number of Concealed MB : %d\n",
				     pctx->name, picture.nbrOfErrMBs);
		pctx->stream_errors++;
	}

	if (ctx->state == G1_MP4_FLUSHING) {
		/* Seek case : In flushing state, while there is no
		 * decoded key frame, don't display anything as we're
		 * not sure frames are sane.
		 */
		if (picture.keyPicture) {
			dev_dbg(g1->dev,
				"FLUSHING STATE : I frame received\n");
			ctx->state = G1_MP4_HEADER_PARSED;
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

static void g1_mp4_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
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

static void g1_mp4_flush(struct g1_ctx *pctx)
{
	struct g1_mp4_ctx *ctx = to_ctx(pctx);

	ctx->state = G1_MP4_FLUSHING;
}

static void g1_mp4_drain(struct g1_ctx *pctx)
{
	struct g1_mp4_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

/* MP4 decoder can decode following profiles and levels :
 * - MPEG-4 [2] simple profile (levels 0-6) and Main Profile
 * - H.263 [3] with profile 0, 10-70.
 * - DivX 3, 4, 5 and 6
 * - Sorenson Spark  (Not yet supported)
 * Add below one decoder structure per supported format
 */
const __u32 mp4_stream_formats[] = {
	V4L2_PIX_FMT_MPEG4,
	V4L2_PIX_FMT_H263,
	V4L2_PIX_FMT_XVID,
#ifdef V4L2_PIX_FMT_FLV1
	V4L2_PIX_FMT_FLV1,
#endif
};

const struct g1_dec mp4dec = {
	.name = "mp4",
	.streamformat = &mp4_stream_formats[0],
	.nb_streams = ARRAY_SIZE(mp4_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_mp4_probe,
	.open = g1_mp4_open,
	.close = g1_mp4_close,
	.get_streaminfo = g1_mp4_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_mp4_get_frameinfo,
	.set_frameinfo = g1_mp4_set_frameinfo,
	.decode = g1_mp4_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_mp4_get_frame,
	.recycle = g1_mp4_recycle,
	.flush = g1_mp4_flush,
	.drain = g1_mp4_drain,
};
