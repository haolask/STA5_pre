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

#include "stack/inc/jpegdecapi.h"
#include "stack/common/bqueue.h"

#define G1_JPEG_IMAGE_INFO_RECEIVED	1

#define JPEGDEC_MIN_WIDTH 48
#define JPEGDEC_MIN_HEIGHT 48
#define JPEGDEC_MAX_WIDTH 4672
#define JPEGDEC_MAX_HEIGHT 4672
#define JPEGDEC_MAX_PIXEL_AMOUNT 16370688

struct g1_jpeg_ctx {
	JpegDecInst jpeg;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;

	unsigned int state;

	bool decoded_frame_not_pushed;

	unsigned int eos;
};

#define to_ctx(ctx) ((struct g1_jpeg_ctx *)(ctx)->priv)

/* debug API of JPEG stack */
void JpegDecTrace(const char *str)
{
#ifdef JPEGDEC_TRACE
	pr_debug("   %s\n", str);
#endif
};

static inline int to_ret(JpegDecRet ret)
{
	switch (ret) {
	case JPEGDEC_SLICE_READY:
	case JPEGDEC_FRAME_READY:
	case JPEGDEC_STRM_PROCESSED:
	case JPEGDEC_SCAN_PROCESSED:
	case JPEGDEC_OK:
		return 0;
	case JPEGDEC_STRM_ERROR:
		return 0;
	case JPEGDEC_MEMFAIL:
		return -ENOMEM;
	case JPEGDEC_INVALID_STREAM_LENGTH:
	case JPEGDEC_INVALID_INPUT_BUFFER_SIZE:
	case JPEGDEC_UNSUPPORTED:
	case JPEGDEC_SLICE_MODE_UNSUPPORTED:
	case JPEGDEC_PARAM_ERROR:
		return -EINVAL;
	default:
		return -EIO;
	}
}

static inline int is_stream_error(JpegDecRet ret)
{
	switch (ret) {
	case JPEGDEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *jpegret_str(JpegDecRet ret)
{
	switch (ret) {
	case JPEGDEC_SLICE_READY:
		return "JPEGDEC_SLICE_READY";
	case JPEGDEC_FRAME_READY:
		return "JPEGDEC_FRAME_READY";
	case JPEGDEC_STRM_PROCESSED:
		return "JPEGDEC_STRM_PROCESSED";
	case JPEGDEC_SCAN_PROCESSED:
		return "JPEGDEC_SCAN_PROCESSED";
	case JPEGDEC_OK:
		return "JPEGDEC_OK";
	case JPEGDEC_ERROR:
		return "JPEGDEC_ERROR";
	case JPEGDEC_UNSUPPORTED:
		return "JPEGDEC_UNSUPPORTED";
	case JPEGDEC_PARAM_ERROR:
		return "JPEGDEC_PARAM_ERROR";
	case JPEGDEC_MEMFAIL:
		return "JPEGDEC_MEMFAIL";
	case JPEGDEC_INITFAIL:
		return "JPEGDEC_INITFAIL";
	case JPEGDEC_INVALID_STREAM_LENGTH:
		return "JPEGDEC_INVALID_STREAM_LENGTH";
	case JPEGDEC_STRM_ERROR:
		return "JPEGDEC_STRM_ERROR";
	case JPEGDEC_INVALID_INPUT_BUFFER_SIZE:
		return "JPEGDEC_INVALID_INPUT_BUFFER_SIZE";
	case JPEGDEC_HW_RESERVED:
		return "JPEGDEC_HW_RESERVED";
	case JPEGDEC_INCREASE_INPUT_BUFFER:
		return "JPEGDEC_INCREASE_INPUT_BUFFER";
	case JPEGDEC_SLICE_MODE_UNSUPPORTED:
		return "JPEGDEC_SLICE_MODE_UNSUPPORTED";
	case JPEGDEC_DWL_HW_TIMEOUT:
		return "JPEGDEC_DWL_HW_TIMEOUT";
	case JPEGDEC_DWL_ERROR:
		return "JPEGDEC_DWL_ERROR";
	case JPEGDEC_HW_BUS_ERROR:
		return "JPEGDEC_HW_BUS_ERROR";
	case JPEGDEC_SYSTEM_ERROR:
		return "JPEGDEC_SYSTEM_ERROR";
	case JPEGDEC_FORMAT_NOT_SUPPORTED:
		return "JPEGDEC_FORMAT_NOT_SUPPORTED";
	default:
		return "!unknown JPEG return value!";
	}
}

static inline int to_v4l2_pixelformat(u32 fmt)
{
	switch (fmt) {
	case JPEGDEC_YCbCr400:
		return V4L2_PIX_FMT_Y4;
	case JPEGDEC_YCbCr420_SEMIPLANAR:
		return V4L2_PIX_FMT_NV12;
	case JPEGDEC_YCbCr422_SEMIPLANAR:
		return V4L2_PIX_FMT_NV16;
	case JPEGDEC_YCbCr411_SEMIPLANAR:
		return V4L2_PIX_FMT_Y41P;
	case JPEGDEC_YCbCr444_SEMIPLANAR:
		return V4L2_PIX_FMT_NV24;
	default:
		return 0;
	}
}

static int g1_jpeg_probe(struct g1_dev *g1)
{
	/* check if codec is supported by hardware */
	if (!g1->hw.config.jpeg_support)
		return -EACCES;

	return 0;
}

static int g1_jpeg_open(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_jpeg_ctx *ctx;
	JpegDecRet jpegret;
	int ret;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	pctx->priv = ctx;

	jpegret = JpegDecInit(&ctx->jpeg,
			      (void *)pctx); /* forward g1 context to DWL */
	if (jpegret) {
		dev_err(g1->dev, "%s  JpegDecInit error %s(%d)\n", pctx->name,
			jpegret_str(jpegret), jpegret);
		ret = -EIO;
		goto err_free;
	}

	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));

	ret = g1_pp_open(pctx, ctx->jpeg, ctx->streaminfo.streamformat);
	if (ret)
		goto err_release;

	return 0;

err_release:
	JpegDecRelease(ctx->jpeg);
err_free:
	kfree(ctx);
	return ret;
}

static int g1_jpeg_close(struct g1_ctx *pctx)
{
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	JpegDecRelease(ctx->jpeg);

	kfree(ctx);

	return 0;
}

static int g1_jpeg_set_streaminfo(struct g1_ctx *pctx,
				  JpegDecImageInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;

	/* check width/height */
	if ((decinfo->outputWidth == 0) || (decinfo->outputHeight == 0) ||
	    (decinfo->outputWidth * decinfo->outputHeight >
	     JPEGDEC_MAX_PIXEL_AMOUNT)) {
		dev_err(g1->dev,
			"%s  invalid stream coded resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, decinfo->outputWidth,
			decinfo->outputHeight,
			JPEGDEC_MAX_PIXEL_AMOUNT);
		return -EINVAL;
	}

	streaminfo->streamformat = V4L2_PIX_FMT_JPEG;
	streaminfo->width = decinfo->outputWidth;
	streaminfo->height = decinfo->outputHeight;
	streaminfo->field = V4L2_FIELD_NONE;
	streaminfo->dpb = 0;
	return 0;
}

static int g1_jpeg_set_dec_frameinfo(struct g1_ctx *pctx,
				     JpegDecImageInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);
	struct g1_frameinfo *frameinfo = &ctx->dec_frameinfo;
	__u32 fmt;

	/* check decoder output width/height */
	if ((decinfo->outputWidth == 0) || (decinfo->outputHeight == 0) ||
	    (decinfo->outputWidth * decinfo->outputHeight >
	     JPEGDEC_MAX_PIXEL_AMOUNT)) {
		dev_err(g1->dev,
			"%s  invalid decoder output resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, decinfo->outputWidth,
			decinfo->outputHeight,
			JPEGDEC_MAX_PIXEL_AMOUNT);
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
	frameinfo->width = decinfo->outputWidth;
	frameinfo->height = decinfo->outputHeight;
	frameinfo->aligned_width = decinfo->outputWidth;
	frameinfo->aligned_height = decinfo->outputHeight;
	frameinfo->field = V4L2_FIELD_NONE;
	/* By default JPEG is systematically in Full range [0-255] */
	frameinfo->colorspace = V4L2_COLORSPACE_JPEG;

	return 0;
}

static int g1_jpeg_get_streaminfo(struct g1_ctx *pctx,
				  struct g1_streaminfo *streaminfo)
{
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_JPEG_IMAGE_INFO_RECEIVED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_jpeg_get_frameinfo(struct g1_ctx *pctx,
				 struct g1_frameinfo *frameinfo)
{
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_JPEG_IMAGE_INFO_RECEIVED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_jpeg_set_frameinfo(struct g1_ctx *pctx,
				 struct g1_frameinfo *frameinfo)
{
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_JPEG_IMAGE_INFO_RECEIVED)
		return -ENODATA;

	frameinfo->colorspace = ctx->dec_frameinfo.colorspace;

	/* let post-proc check & update infos to what it can do */
	ret = g1_pp_check_config(pctx, &ctx->dec_frameinfo,
				 frameinfo);
	if (ret)
		return ret;

	ctx->pp_frameinfo = *frameinfo;

	return 0;
}

/*------------------------------------
 * Print JpegDecGetImageInfo values
 *------------------------------------
 */
void g1_jpeg_display_imageinfo(struct g1_ctx *pctx,
			       JpegDecImageInfo *imageInfo)
{
	struct g1_dev *g1 = pctx->dev;
	/* Select if Thumbnail or full resolution image will be decoded */
	if (imageInfo->thumbnailType == JPEGDEC_THUMBNAIL_JPEG) {
		dev_dbg(g1->dev, "%s\t-THUMBNAIL INFO\n", pctx->name);
		dev_dbg(g1->dev, "%s\t\t- Thumbnail size : %dx%d\n",
			pctx->name, imageInfo->outputWidthThumb,
			imageInfo->outputHeightThumb);
		/* stream type */
		switch (imageInfo->codingModeThumb) {
		case JPEGDEC_BASELINE:
			dev_dbg(g1->dev,
				"%s\t\t- StreamType : BASELINE\n",
				pctx->name);
			break;
		case JPEGDEC_PROGRESSIVE:
			dev_dbg(g1->dev,
				"%s\t\t- StreamType : PROGRESSIVE\n",
				pctx->name);
			break;
		case JPEGDEC_NONINTERLEAVED:
			dev_dbg(g1->dev,
				"%s\t\t- StreamType : NONINTERLEAVED\n",
				pctx->name);
			break;
		}

		if (imageInfo->outputFormatThumb) {
			switch (imageInfo->outputFormatThumb) {
			case JPEGDEC_YCbCr400:
				dev_dbg(g1->dev,
					"%s\t\t- Output format: YCbCr400\n",
					pctx->name);
				break;
			case JPEGDEC_YCbCr420_SEMIPLANAR:
				dev_dbg(g1->dev,
					"%s\t\t- Output format: YCbCr420_SP\n",
					pctx->name);
				break;
			case JPEGDEC_YCbCr422_SEMIPLANAR:
				dev_dbg(g1->dev,
					"%s\t\t- Output format: YCbCr422_SP\n",
					pctx->name);
				break;
			case JPEGDEC_YCbCr440:
				dev_dbg(g1->dev,
					"%s\t\t- Output format: YCbCr440\n",
					pctx->name);
				break;
			case JPEGDEC_YCbCr411_SEMIPLANAR:
				dev_dbg(g1->dev,
					"%s\t\t- Output format: YCbCr411_SP\n",
					pctx->name);
				break;
			case JPEGDEC_YCbCr444_SEMIPLANAR:
				dev_dbg(g1->dev,
					"%s\t\t- Output format: YCbCr444_SP\n",
					pctx->name);
				break;
			}
		}
	} else if (imageInfo->thumbnailType == JPEGDEC_NO_THUMBNAIL) {
		dev_dbg(g1->dev, "%s\t-NO THUMBNAIL\n", pctx->name);
	} else if (imageInfo->thumbnailType ==
		   JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT) {
		dev_dbg(g1->dev, "%s\t-NOT SUPPORTED THUMBNAIL\n", pctx->name);
	}
	/* decode full image */
	dev_dbg(g1->dev, "%s\t- JPEG FULL RESOLUTION INFO\n", pctx->name);
	dev_dbg(g1->dev, "%s\t\t- width: %d\n", pctx->name,
		imageInfo->outputWidth);
	dev_dbg(g1->dev, "%s\t\t- height: %d\n", pctx->name,
		imageInfo->outputHeight);
	if (imageInfo->outputFormat) {
		switch (imageInfo->outputFormat) {
		case JPEGDEC_YCbCr400:
			dev_dbg(g1->dev,
				"%s\t\t- Output format: YCbCr400\n",
				pctx->name);
			break;
		case JPEGDEC_YCbCr420_SEMIPLANAR:
			dev_dbg(g1->dev,
				"%s\t\t- Output format: YCbCr420_SP\n",
				pctx->name);
			break;
		case JPEGDEC_YCbCr422_SEMIPLANAR:
			dev_dbg(g1->dev,
				"%s\t\t- Output format: YCbCr422_SP\n",
				pctx->name);
			break;
		case JPEGDEC_YCbCr440:
			dev_dbg(g1->dev,
				"%s\t\t- Output format: YCbCr440\n",
				pctx->name);
			break;
		case JPEGDEC_YCbCr411_SEMIPLANAR:
			dev_dbg(g1->dev,
				"%s\t\t- Output format: YCbCr411_SP\n",
				pctx->name);
			break;
		case JPEGDEC_YCbCr444_SEMIPLANAR:
			dev_dbg(g1->dev,
				"%s\t\t- Output format: YCbCr444_SP\n",
				pctx->name);
			break;
		}
	}
	/* stream type */
	switch (imageInfo->codingMode) {
	case JPEGDEC_BASELINE:
		dev_dbg(g1->dev,
			"%s\t\t- StreamType : BASELINE\n",
			pctx->name);
		break;
	case JPEGDEC_PROGRESSIVE:
		dev_dbg(g1->dev,
			"%s\t\t- StreamType : PROGRESSIVE\n",
			pctx->name);
		break;
	case JPEGDEC_NONINTERLEAVED:
		dev_dbg(g1->dev,
			"%s\t\t- StreamType : NONINTERLEAVED\n",
			pctx->name);
		break;
	}
}

static int g1_jpeg_decode(struct g1_ctx *pctx, struct g1_au *pau)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);
	int ret;
	JpegDecRet jpegret;
	JpegDecInput in;
	JpegDecOutput out;
	JpegDecImageInfo decinfo;

	memset(&in, 0, sizeof(in));
	/* Full resolution image to be decoded (not thumbnail) */
	in.decImageType = 0;
	in.streamBuffer.pVirtualAddress = pau->vaddr;
	in.streamBuffer.busAddress = (u32)pau->paddr;
	in.streamLength = pau->size;
	in.bufferSize = 0;
	memset(&out, 0, sizeof(out));

	if (ctx->state < G1_JPEG_IMAGE_INFO_RECEIVED) {
		jpegret = JpegDecGetImageInfo(ctx->jpeg, &in, &decinfo);
		if (jpegret != JPEGDEC_OK) {
			dev_err(g1->dev,
				"%s  JpegDecGetImageInfo error %s(%d)\n",
				pctx->name,
				jpegret_str(jpegret), jpegret);
			return jpegret;
		}

		g1_jpeg_display_imageinfo(pctx, &decinfo);

		ret = g1_jpeg_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_jpeg_set_dec_frameinfo(pctx, &decinfo);
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

		ctx->state = G1_JPEG_IMAGE_INFO_RECEIVED;
		return -ENODATA;
	}
	if (ctx->state >= G1_JPEG_IMAGE_INFO_RECEIVED) {
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
	ctx->decoded_frame_not_pushed = false;
	/* decode this access unit */
	jpegret = JpegDecDecode(ctx->jpeg, &in, &out);
	/* check decode status */
	ret = to_ret(jpegret);
	if (ret) {
		dev_err(g1->dev, "%s  JpegDecDecode error %s(%d)\n",
			pctx->name,
			jpegret_str(jpegret), jpegret);
		pctx->decode_errors++;
		return ret;
	}

	if (jpegret != JPEGDEC_FRAME_READY)
		return -ENODATA;

	/* frame decoded */
	pctx->decoded_frames++;
	ctx->decoded_frame_not_pushed = true;
	dev_dbg(g1->dev, "%s  dec picture cnt=%d\n",
		pctx->name,
		pctx->decoded_frames);
	return 0;
}

static int g1_jpeg_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);
	int ret;

	if (!ctx->decoded_frame_not_pushed)
		return -ENODATA;

	/* get the post-processed frame */
	ret = g1_pp_get_frame(pctx, frame);
	if (ret)
		return ret;

	ctx->decoded_frame_not_pushed = false;
	dev_dbg(g1->dev, "%s  out frame[%d] %s cnt=%d %s\n",
		pctx->name,
		(*frame)->index,
		frame_type_str((*frame)->flags),
		pctx->decoded_frames,
		frame_state_str((*frame)->state,
				pctx->str, sizeof(pctx->str)));

	return 0;
}

static void g1_jpeg_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
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

static void g1_jpeg_flush(struct g1_ctx *pctx)
{
	/* FIXME find a way to indicate flush to JPEG stack */
}

static void g1_jpeg_drain(struct g1_ctx *pctx)
{
	struct g1_jpeg_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

const __u32 jpeg_stream_formats[] = {
	V4L2_PIX_FMT_JPEG,
	V4L2_PIX_FMT_MJPEG
};

const __u16 jpeg_width_range[] = {
	JPEGDEC_MIN_WIDTH, JPEGDEC_MAX_WIDTH
};

const __u16 jpeg_height_range[] = {
	JPEGDEC_MIN_WIDTH, JPEGDEC_MAX_HEIGHT
};

const struct g1_dec jpegdec = {
	.name = "jpeg",
	.streamformat = &jpeg_stream_formats[0],
	.nb_streams = ARRAY_SIZE(jpeg_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_jpeg_probe,
	.open = g1_jpeg_open,
	.close = g1_jpeg_close,
	.get_streaminfo = g1_jpeg_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_jpeg_get_frameinfo,
	.set_frameinfo = g1_jpeg_set_frameinfo,
	.decode = g1_jpeg_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_jpeg_get_frame,
	.recycle = g1_jpeg_recycle,
	.flush = g1_jpeg_flush,
	.drain = g1_jpeg_drain,
	.width_range = &jpeg_width_range[0],
	.height_range = &jpeg_height_range[0],
	.max_pixel_nb = JPEGDEC_MAX_PIXEL_AMOUNT
};
