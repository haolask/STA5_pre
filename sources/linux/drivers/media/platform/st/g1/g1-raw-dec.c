/*
 * Copyright (C) STMicroelectronics SA 2016
 * Authors: stephane.danieau@st.com
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include "g1.h"
#include "g1-cfg.h"
#include "g1-debug.h"

#define G1_RAW_HEADER_PARSED	1

static const __u32 raw_stream_formats[] = {
	/* NV12. YUV420SP - 1 plane for Y + 1 plane for (CbCr) */
	V4L2_PIX_FMT_NV12,
	/* YU12. YUV420P - 1 plane for Y + 1 plane for Cb + 1 plane for Cr */
	V4L2_PIX_FMT_YUV420,
	/* YUY2. YUV422 Interleaved - 1 plane for YCbYCr */
	V4L2_PIX_FMT_YUYV,
	/* YVYU. YUV422 Interleaved - 1 plane for YCrYCb */
	V4L2_PIX_FMT_YVYU,
	/* UYVY. YUV422 Interleaved - 1 plane CbYCrY */
	V4L2_PIX_FMT_UYVY,
	/* VYUY. YUV422 Interleaved - 1 plane CrYCbY */
	V4L2_PIX_FMT_VYUY
};

struct g1_raw_ctx {
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;

	unsigned int state;

	unsigned int eos;
};

/**
 * struct raw_fmt - driver's internal color format data
 * @pixelformat:fourcc code for this format
 * @bpp:        bits per pixel (general)
 * @bpp_plane0: byte per pixel for the 1st plane
 * @w_align:    width alignment in pixel (multiple of)
 * @h_align:    height alignment in pixel (multiple of)
 */
struct raw_fmt {
	u32                     pixelformat;
	u8                      bpp;
	u8                      bpp_plane0;
	u8                      w_align;
	u8                      h_align;
};

static const struct raw_fmt raw_formats[] = {
	/* NV12. YUV420SP - 1 plane for Y + 1 plane for (CbCr) */
	{
		.pixelformat    = V4L2_PIX_FMT_NV12,
		.bpp            = 12,
		.bpp_plane0     = 8,
		.w_align        = 4, /* 2^4 => 16 pixels */
		.h_align        = 4
	},
	/* YU12. YUV420P - 1 plane for Y + 1 plane for Cb + 1 plane for Cr */
	{
		.pixelformat    = V4L2_PIX_FMT_YUV420,
		.bpp            = 12,
		.bpp_plane0     = 8,
		.w_align        = 4,
		.h_align        = 4
	},
	/* YUYV. YUV422 Interleaved - 1 plane for YCbCr */
	{
		.pixelformat    = V4L2_PIX_FMT_YUYV,
		.bpp            = 16,
		.bpp_plane0     = 16,
		.w_align        = 4,
		.h_align        = 4
	},
	/* YVYU. YUV422 Interleaved - 1 plane for YCrCb */
	{
		.pixelformat    = V4L2_PIX_FMT_YVYU,
		.bpp            = 16,
		.bpp_plane0     = 16,
		.w_align        = 4,
		.h_align        = 4
	},
	/* UYVY. YUV422 Interleaved - 1 plane for CbYCrY */
	{
		.pixelformat    = V4L2_PIX_FMT_UYVY,
		.bpp            = 16,
		.bpp_plane0     = 16,
		.w_align        = 4,
		.h_align        = 4
	},
	/* VYUY. YUV422 Interleaved - 1 plane for CrYCbY */
	{
		.pixelformat    = V4L2_PIX_FMT_VYUY,
		.bpp            = 16,
		.bpp_plane0     = 16,
		.w_align        = 4,
		.h_align        = 4
	}
};

static const struct raw_fmt *raw_find_stream_fmt(u32 pixelformat)
{
	const struct raw_fmt *fmt;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(raw_formats); i++) {
		fmt = &raw_formats[i];
		if (fmt->pixelformat == pixelformat)
			return fmt;
	}
	return NULL;
}

#define to_ctx(ctx) ((struct g1_raw_ctx *)(ctx)->priv)

static int g1_raw_set_dec_frameinfo(struct g1_ctx *pctx)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);
	struct g1_frameinfo *frameinfo = &ctx->dec_frameinfo;
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;

	frameinfo->pixelformat = streaminfo->streamformat;
	frameinfo->pixelaspect = streaminfo->pixelaspect;
	frameinfo->colorspace = streaminfo->colorspace;
	frameinfo->field = streaminfo->field;
	frameinfo->width = streaminfo->aligned_width;
	frameinfo->height = streaminfo->aligned_height;
	frameinfo->aligned_width = streaminfo->aligned_width;
	frameinfo->aligned_height = streaminfo->aligned_height;
	frameinfo->flags &= ~G1_FRAMEINFO_FLAG_CROP;
	if (streaminfo->flags & G1_STREAMINFO_FLAG_CROP) {
		frameinfo->flags |= G1_FRAMEINFO_FLAG_CROP;
		frameinfo->crop = streaminfo->crop;
	}

	return 0;
}

static int g1_raw_valid_config(struct g1_ctx *pctx)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);
	struct g1_dev *g1 = pctx->dev;
	unsigned int ret;

	/* let post-proc check & update infos to what it can do */
	ret = g1_pp_check_config(pctx, &ctx->dec_frameinfo,
				 &ctx->pp_frameinfo);
	if (ret) {
		dev_dbg(g1->dev, "%s configuration check failure\n",
			pctx->name);
		return  -EINVAL;
	}
	if (ctx->pp_frameinfo.flags &
	    (G1_FRAMEINFO_FLAG_WIDTH_UPDATED |
	     G1_FRAMEINFO_FLAG_HEIGHT_UPDATED |
	     G1_FRAMEINFO_FLAG_CROP_WIDTH_UPDATED |
	     G1_FRAMEINFO_FLAG_CROP_HEIGHT_UPDATED)) {
		/* in case of standalone PP usage, we forbid update of output
		 * resolution before starting the PP.
		 * This constraint is mainly due to Gstreamer plugin v4l2trans
		 * which has negociated the caps based on fixed output
		 * resolution :
		 * in this particular case, PP can't output this resolution.
		 */
		dev_warn(g1->dev,
			 "%s Unable to reach requested output resolution without updating it\n",
			 pctx->name);
		return -EINVAL;
	}
	return 0;
}

static int g1_raw_set_streaminfo(struct g1_ctx *pctx,
				 struct g1_streaminfo *userinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_raw_ctx *ctx = to_ctx(pctx);
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;
	struct g1_frameinfo *frameinfo = &ctx->pp_frameinfo;
	const struct raw_fmt *format;

	if ((userinfo->width == 0) || (userinfo->height == 0) ||
	    (userinfo->width * userinfo->height > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid stream coded resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, userinfo->width, userinfo->height,
			G1_MAX_RESO);
		return -EINVAL;
	}

	memcpy(streaminfo, userinfo, sizeof(*streaminfo));

	/* Adjust width & height */
	format = raw_find_stream_fmt(streaminfo->streamformat);
	if (!format) {
		dev_err(g1->dev, "%s Unknown format (%s)\n", pctx->name,
			(char *)&streaminfo->streamformat);
		return -EINVAL;
	}

	v4l_bound_align_image(&streaminfo->aligned_width,
			      streaminfo->width, PP_MAX_W,
			      format->w_align,
			      &streaminfo->aligned_height,
			      streaminfo->height, PP_MAX_H,
			      format->h_align,
			      0);

	if (!(streaminfo->flags & G1_STREAMINFO_FLAG_CROP)) {
		streaminfo->crop.width = 0;
		streaminfo->crop.height = 0;

		if ((streaminfo->aligned_width != streaminfo->width) ||
		    (streaminfo->aligned_height != streaminfo->height)) {
			streaminfo->flags |= G1_STREAMINFO_FLAG_CROP;
			streaminfo->crop.width = streaminfo->width;
			streaminfo->crop.height = streaminfo->height;
		}
	}

	frameinfo->deinterlace_activable = false;
	frameinfo->field = streaminfo->field;
	/* In Raw data, only NV12 format may be deinterlaced by the G1 */
	if (streaminfo->streamformat == V4L2_PIX_FMT_NV12) {
		dev_dbg(g1->dev,
			"%s Enable deinterlace capability\n",
			pctx->name);
		frameinfo->deinterlace_activable = true;
		frameinfo->field = V4L2_FIELD_NONE;
	}

	g1_raw_set_dec_frameinfo(pctx);

	pctx->flags |= G1_FLAG_STREAMINFO;

	return 0;
}

static int g1_raw_open(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_raw_ctx *ctx = to_ctx(pctx);
	int ret;

	if (!ctx) {
		ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
		if (!ctx) {
			dev_err(g1->dev, "%s open failure (-ENOMEM)\n",
				pctx->name);
			return -ENOMEM;
		}
		pctx->priv = ctx;
		ret = g1_pp_open(pctx, NULL, 0);
		if (ret)
			goto err_release;
	}
	pctx->timestamp_type = G1_DECODING_TIMESTAMP;
	return 0;

err_release:
	kfree(ctx);
	dev_err(g1->dev, "%s open failure (%d)\n", pctx->name, ret);
	return ret;
}

static int g1_raw_close(struct g1_ctx *pctx)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);
	kfree(ctx);
	return 0;
}

static int g1_raw_get_streaminfo(struct g1_ctx *pctx,
				 struct g1_streaminfo *streaminfo)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);

	*streaminfo = ctx->streaminfo;
	return 0;
}

static int g1_raw_get_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);

	*frameinfo = ctx->pp_frameinfo;
	return 0;
}

static int g1_raw_set_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);
	struct g1_frameinfo *streaminfo;
	struct g1_dev *g1 = pctx->dev;
	unsigned int ret;

	streaminfo = &ctx->dec_frameinfo;

	frameinfo->deinterlace_activable = false;
	frameinfo->field = streaminfo->field;

	/* In Raw data, only NV12 format may be deinterlaced by the G1 */
	if (streaminfo->pixelformat == V4L2_PIX_FMT_NV12) {
		dev_dbg(g1->dev,
			"%s Enable deinterlace capability\n",
			pctx->name);
		frameinfo->deinterlace_activable = true;
		frameinfo->field = V4L2_FIELD_NONE;
	}

	/* let post-proc check & update infos to what it can do */
	ret = g1_pp_check_config(pctx, streaminfo, frameinfo);
	if (ret) {
		dev_dbg(g1->dev, "%s unable to set frame info\n", pctx->name);
		return ret;
	}
	ctx->pp_frameinfo = *frameinfo;

	return 0;
}

static int g1_raw_decode(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);

	/* frame decoded = frame post-processed  */
	pctx->decoded_frames++;

	/* 'AU' buffer contains raw data there is no decoding
	 * so dec_framinfo format/width/height is equivalent to streaminfo
	 * Difference as there is no decode, is that we have to transmit
	 * input buffer address to PP.
	 */
	ctx->dec_frameinfo.size = au->size;
	ctx->dec_frameinfo.vaddr = au->vaddr;
	ctx->dec_frameinfo.paddr = au->paddr;

	if (ctx->state < G1_RAW_HEADER_PARSED)
		ctx->state = G1_RAW_HEADER_PARSED;

	return 0;
}

static int g1_raw_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);
	int ret;
	struct g1_frame *thisframe;
	struct g1_frameinfo *dec_frameinfo = &ctx->dec_frameinfo;

	if (ctx->state >= G1_RAW_HEADER_PARSED) {
		if (g1_is_free_frame_available(pctx, &thisframe)) {
			ret = g1_get_free_frame(pctx, &thisframe);
			if (ret) {
				if (ret == -EAGAIN)
					goto nodata;
				return ret;
			}
			if (ret == 0) {
				/* setup post-processor */
				ret = g1_pp_set_config(pctx, dec_frameinfo,
						       thisframe);
				if (ret)
					return ret;
			}

			/* get the post-processed frame */
			ret = g1_pp_get_frame(pctx, frame);
			if (ret)
				return ret;
			return 0;
		}
	}

nodata:
	if (ctx->eos)
		ctx->eos = 0; /* no more frames to drain: end of EOS */
	return -ENODATA;
}

static void g1_raw_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
{
	g1_recycle(pctx, frame);
}

static void g1_raw_flush(struct g1_ctx *pctx)
{
	/* FIXME find a way to indicate flush to H264 stack */
}

static void g1_raw_drain(struct g1_ctx *pctx)
{
	struct g1_raw_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

static int g1_raw_probe(struct g1_dev *g1)
{
	return 0;
}

const struct g1_dec rawdec = {
	.name = "rawdec",
	.streamformat = &raw_stream_formats[0],
	.nb_streams = ARRAY_SIZE(raw_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_raw_probe,
	.open = g1_raw_open,
	.close = g1_raw_close,
	.get_streaminfo = g1_raw_get_streaminfo,
	.set_streaminfo = g1_raw_set_streaminfo,
	.get_frameinfo = g1_raw_get_frameinfo,
	.set_frameinfo = g1_raw_set_frameinfo,
	.decode = g1_raw_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_raw_get_frame,
	.recycle = g1_raw_recycle,
	.flush = g1_raw_flush,
	.drain = g1_raw_drain,
	.validate_config = g1_raw_valid_config,
};
