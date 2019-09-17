/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Yannick Fertre <yannick.fertre@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include "g1.h"
#include "g1-debug.h"

/* debug API of PP stack */
void PPTrace(const char *str)
{
#ifdef PP_TRACE
	pr_debug("   %s\n", str);
#endif
};

static inline const char *ppret_str(PPResult ret)
{
	switch (ret) {
	case PP_OK:
		return "PP_OK";
	case PP_PARAM_ERROR:
		return "PP_PARAM_ERROR";
	case PP_SET_IN_SIZE_INVALID:
		return "PP_SET_IN_SIZE_INVALID";
	case PP_MEMFAIL:
		return "PP_MEMFAIL";
	case PP_SET_IN_ADDRESS_INVALID:
		return "PP_SET_IN_ADDRESS_INVALID";
	case PP_SET_IN_FORMAT_INVALID:
		return "PP_SET_IN_FORMAT_INVALID";
	case PP_SET_CROP_INVALID:
		return "PP_SET_CROP_INVALID";
	case PP_SET_ROTATION_INVALID:
		return "PP_SET_ROTATION_INVALID";
	case PP_SET_OUT_SIZE_INVALID:
		return "PP_SET_OUT_SIZE_INVALID";
	case PP_SET_OUT_ADDRESS_INVALID:
		return "PP_SET_OUT_ADDRESS_INVALID";
	case PP_SET_OUT_FORMAT_INVALID:
		return "PP_SET_OUT_FORMAT_INVALID";
	case PP_SET_VIDEO_ADJUST_INVALID:
		return "PP_SET_VIDEO_ADJUST_INVALID";
	case PP_SET_RGB_BITMASK_INVALID:
		return "PP_SET_RGB_BITMASK_INVALID";
	case PP_SET_FRAMEBUFFER_INVALID:
		return "PP_SET_FRAMEBUFFER_INVALID";
	case PP_SET_MASK1_INVALID:
		return "PP_SET_MASK1_INVALID";
	case PP_SET_MASK2_INVALID:
		return "PP_SET_MASK2_INVALID";
	case PP_SET_DEINTERLACE_INVALID:
		return "PP_SET_DEINTERLACE_INVALID";
	case PP_SET_IN_STRUCT_INVALID:
		return "PP_SET_IN_STRUCT_INVALID";
	case PP_SET_IN_RANGE_MAP_INVALID:
		return "PP_SET_IN_RANGE_MAP_INVALID";
	case PP_SET_ABLEND_UNSUPPORTED:
		return "PP_SET_ABLEND_UNSUPPORTED";
	case PP_SET_DEINTERLACING_UNSUPPORTED:
		return "PP_SET_DEINTERLACING_UNSUPPORTED";
	case PP_SET_DITHERING_UNSUPPORTED:
		return "PP_SET_DITHERING_UNSUPPORTED";
	case PP_SET_SCALING_UNSUPPORTED:
		return "PP_SET_SCALING_UNSUPPORTED";
	case PP_BUSY:
		return "PP_BUSY";
	case PP_HW_BUS_ERROR:
		return "PP_HW_BUS_ERROR";
	case PP_HW_TIMEOUT:
		return "PP_HW_TIMEOUT";
	case PP_DWL_ERROR:
		return "PP_DWL_ERROR";
	case PP_SYSTEM_ERROR:
		return "PP_SYSTEM_ERROR";
	case PP_DEC_COMBINED_MODE_ERROR:
		return "PP_DEC_COMBINED_MODE_ERROR";
	case PP_DEC_RUNTIME_ERROR:
		return "PP_DEC_RUNTIME_ERROR";
	default:
		return "!unknown PP return value!";
	}
}

static inline u32 to_pp_dec_type(u32 streamformat)
{
	switch (streamformat) {
	case V4L2_PIX_FMT_H264:
		return PP_PIPELINED_DEC_TYPE_H264;
	case V4L2_PIX_FMT_MPEG4:
	case V4L2_PIX_FMT_H263:
	case V4L2_PIX_FMT_XVID:
#ifdef V4L2_PIX_FMT_FLV1
	case V4L2_PIX_FMT_FLV1:
#endif
		return PP_PIPELINED_DEC_TYPE_MPEG4;
	case V4L2_PIX_FMT_JPEG:
		return PP_PIPELINED_DEC_TYPE_JPEG;
	case V4L2_PIX_FMT_MJPEG:
		return PP_PIPELINED_DEC_TYPE_JPEG;
	case V4L2_PIX_FMT_VC1_ANNEX_G:
	case V4L2_PIX_FMT_VC1_ANNEX_L:
		return PP_PIPELINED_DEC_TYPE_VC1;
	case V4L2_PIX_FMT_MPEG1:
	case V4L2_PIX_FMT_MPEG2:
		return PP_PIPELINED_DEC_TYPE_MPEG2;
	case V4L2_PIX_FMT_VP8:
		return PP_PIPELINED_DEC_TYPE_VP8;
#ifdef V4L2_PIX_FMT_CAVS
	case V4L2_PIX_FMT_CAVS:
		return PP_PIPELINED_DEC_TYPE_AVS;
#endif
#ifdef V4L2_PIX_FMT_RV30
	case V4L2_PIX_FMT_RV30:
		return PP_PIPELINED_DEC_TYPE_RV;
#endif
#ifdef V4L2_PIX_FMT_RV40
	case V4L2_PIX_FMT_RV40:
		return PP_PIPELINED_DEC_TYPE_RV;
#endif
	default:
		return PP_PIPELINE_DISABLED;
	}
}

static inline u32 to_pp_format(u32 streamformat)
{
	switch (streamformat) {
	case V4L2_PIX_FMT_NV12:
		return PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
	case V4L2_PIX_FMT_YUV420:
		return PP_PIX_FMT_YCBCR_4_2_0_PLANAR;
	case V4L2_PIX_FMT_YUYV:
		return PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	case V4L2_PIX_FMT_YVYU:
		return PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED;
	case V4L2_PIX_FMT_UYVY:
		return PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED;
	case V4L2_PIX_FMT_RGB565:
		return PP_PIX_FMT_RGB16_5_6_5;
	case V4L2_PIX_FMT_XBGR32:
	case V4L2_PIX_FMT_ABGR32:
		return PP_PIX_FMT_RGB32;
	case V4L2_PIX_FMT_XRGB32:
	case V4L2_PIX_FMT_ARGB32:
		return PP_PIX_FMT_BGR32;
	case V4L2_PIX_FMT_XRGB555:
	case V4L2_PIX_FMT_ARGB555:
		return PP_PIX_FMT_RGB16_5_5_5;
	case V4L2_PIX_FMT_NV16:
		return PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR;
	case V4L2_PIX_FMT_NV24:
		return PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR;
	case V4L2_PIX_FMT_Y4:
		return PP_PIX_FMT_YCBCR_4_0_0;
	default:
		return 0;
	}
}

static inline const char *pp_format_str(u32 fmt)
{
	switch (fmt) {
	case PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR:
		return "FMT_YCBCR_4_2_0_SP";
	case PP_PIX_FMT_YCBCR_4_2_0_PLANAR:
		return "FMT_YCBCR_4_2_0_P";
	case PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED:
		return "FMT_YCBCR_4_2_2_I";
	case PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED:
		return "FMT_YCRYCB_4_2_2_I";
	case PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED:
		return "FMT_CBYCRY_4_2_2_I";
	case PP_PIX_FMT_RGB16_5_6_5:
		return "FMT_RGB16_5_6_5";
	case PP_PIX_FMT_RGB32:
		return "FMT_RGB32";
	case PP_PIX_FMT_BGR32:
		return "FMT_BGR32";
	case PP_PIX_FMT_RGB16_5_5_5:
		return "FMT_RGB16_5_5_5";
	case PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR:
		return "FMT_YCBCR_4_2_2_SP";
	case PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR:
		return "FMT_YCBCR_4_4_4_SP";
	case PP_PIX_FMT_YCBCR_4_0_0:
		return "FMT_YCBCR_4_0_0";
	default:
		return "!unknown PP format!";
	}
}

static inline const char *pp_rotation_str(u32 rotation)
{
	switch (rotation) {
	case PP_ROTATION_NONE: return "NONE";
	case PP_ROTATION_RIGHT_90: return "Right 90";
	case PP_ROTATION_LEFT_90: return "Left 90";
	case PP_ROTATION_HOR_FLIP: return "Horizontal flip";
	case PP_ROTATION_VER_FLIP: return "Vertical flip";
	case PP_ROTATION_180: return "Horizontal flip";
	default: return "Unknown rotation value";
	}
}

/**
 * struct pp_frame_addr - Y/Cb/Cr  start address structure
 * @y:	 luminance plane  address
 * @cb:	 Cb plane address
 * @cr:	 Cr plane address
 * @ybot: luminance component base address of a bottom field.
 * @chbot: chrominance component base address of the input bottom field
 */
struct g1_pp_frame_addr {
	u32 y;
	u32 cb;
	u32 cr;
	u32 ybot;
	u32 chbot;
};

static void g1_pp_prepare_addr(struct g1_pp_frame_addr *paddr, u32 base_addr,
			       u32 streamformat, u32 width, u32 height)
{
	u32 pix_size;

	if (!paddr)
		return;

	pix_size = width * height;
	paddr->y = base_addr;
	switch (streamformat) {
	case V4L2_PIX_FMT_NV12:
		/* decompose Y into Y/Cb */
		paddr->cb = (u32)(paddr->y + pix_size);
		paddr->cr = 0;
		break;
	case V4L2_PIX_FMT_YUV420:
		paddr->cb = (u32)(paddr->y + pix_size);
		/* decompose Y into Y/Cb/Cr */
		paddr->cr = (u32)(paddr->cb + (pix_size >> 2));
		break;
	default:
		paddr->cb = 0;
		paddr->cr = 0;
		break;
	}
}

char *pp_config_str(PPConfig *c, char *str, unsigned int len)
{
	char *cur = str;
	size_t left = len;
	int ret = 0;
	int cnt = 0;
	PPInImage *in;
	PPOutImage *out;
	PPOutFrameBuffer *outFrm;
	PPInCropping *in_crop;

	if (!c)
		return NULL;

	in = &c->ppInImg;
	out = &c->ppOutImg;
	outFrm = &c->ppOutFrmBuffer;
	in_crop = &c->ppInCrop;

	ret = snprintf(cur, left,
		       "%s %dx%d",
		       pp_format_str(in->pixFormat),
		       in->width,
		       in->height);
	cnt = (left > ret ? ret : left);

	if (in_crop->enable) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", crop=%dx%d@(%d,%d)",
			       in_crop->width, in_crop->height,
			       in_crop->originX, in_crop->originY);
		cnt = (left > ret ? ret : left);
	}

	cur += cnt;
	left -= cnt;
	if (outFrm->enable) {
		ret = snprintf(cur, left,
			       " => %s %dx%d, crop=%dx%d@(%d,%d)",
			       pp_format_str(out->pixFormat),
			       outFrm->frameBufferWidth,
			       outFrm->frameBufferHeight,
			       out->width, out->height,
			       outFrm->writeOriginX, outFrm->writeOriginY);
		cnt = (left > ret ? ret : left);
	} else {
	ret = snprintf(cur, left,
		       " => %s %dx%d",
		       pp_format_str(out->pixFormat),
		       out->width,
		       out->height);
	}
	cnt = (left > ret ? ret : left);

	if (c->ppInRotation.rotation != PP_ROTATION_NONE) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " rotation=%s",
			       pp_rotation_str(c->ppInRotation.rotation));
		cnt = (left > ret ? ret : left);
	}
	return str;
}

int g1_pp_open(struct g1_ctx *ctx, const void *dec, __u32 streamformat)
{
	struct g1_dev *g1 = ctx->dev;
	PPInst pp;
	PPResult ppret;

	ppret = PPInit(&pp, (void *)ctx);
	if (ppret) {
		dev_err(g1->dev, "%s  PPInit error %s(%d)\n",
			ctx->name, ppret_str(ppret), ppret);
		return -EIO;
	}

	if (dec) {
		ppret = PPDecCombinedModeEnable(pp, dec,
						to_pp_dec_type(streamformat));
		if (ppret) {
			dev_err(g1->dev, "%s failed to enable combined mode\n",
				ctx->name);
			PPRelease(pp);
			return -EIO;
		}
	} else
		dev_dbg(g1->dev, "%s pp instance opened in standalone mode\n",
			ctx->name);

	ctx->pp.pp = pp;
	ctx->pp.dec = dec;

	return 0;
}

int g1_pp_close(struct g1_ctx *ctx)
{
	struct g1_pp *pp = &ctx->pp;

	if (pp->pp) {
		if (pp->dec)
			PPDecCombinedModeDisable(pp->pp, pp->dec);
		PPRelease(pp->pp);
		pp->pp = NULL;
	}

	return 0;
}

void pp_check_scaling_config(struct g1_ctx *ctx, u32 in_wdt, u32 in_hgt,
			     struct g1_frameinfo *out)
{
	struct g1_dev *g1 = ctx->dev;

	/* Scaling limitation */
	/**********************/
	/* -> Limited to x3 in upscale */
	if (out->aligned_width > (3 * in_wdt)) {
		dev_dbg(g1->dev, "%s : A width of %d is requested but upscale is limited to x3 (force it to %d)\n",
			ctx->name, out->aligned_width,
			3 * in_wdt);
		out->aligned_width = 3 * in_wdt;
		out->flags |= G1_FRAMEINFO_FLAG_WIDTH_UPDATED;
	}
	if (out->aligned_height > (3 * (in_hgt - 2))) {
		dev_dbg(g1->dev, "%s : A height of %d is requested but upscale is limited to x3 (force it to %d)\n",
			ctx->name, out->aligned_height,
			3 * (in_hgt - 2));
		out->aligned_height = 3 * (in_hgt - 2);
		out->flags |= G1_FRAMEINFO_FLAG_HEIGHT_UPDATED;
	}

	/* -> Same scaling direction, enlarging or reducing, has to be used
	 *    for both directions of the picture
	 */
	if ((out->aligned_width > in_wdt) &&
	    (out->aligned_height < in_hgt)) {
		dev_dbg(g1->dev, "%s : W[%d->%d] H[%d->%d] Unable to perform a downscale and an upscale at the same time. Disable height downscaling\n",
			ctx->name, in_wdt, out->aligned_width,
			in_hgt, out->aligned_height);
		out->aligned_height = in_hgt;
		out->flags |= G1_FRAMEINFO_FLAG_HEIGHT_UPDATED;
	}

	if ((out->aligned_width < in_wdt) &&
	    (out->aligned_height > in_hgt)) {
		dev_dbg(g1->dev, "%s : W[%d->%d] H[%d->%d] Unable to perform a downscale and an upscale at the same time. Disable width downscaling\n",
			ctx->name, in_wdt, out->aligned_width,
			in_hgt, out->aligned_height);
		out->aligned_width = in_wdt;
		out->flags |= G1_FRAMEINFO_FLAG_WIDTH_UPDATED;
	}
}

int g1_pp_check_config(struct g1_ctx *ctx, struct g1_frameinfo *in,
		       struct g1_frameinfo *out)
{
	struct g1_dev *g1 = ctx->dev;
	struct g1_pp *pp = &ctx->pp;
	int i = 0;
	bool supported_fmt = false;

	/* check if post-proc can handle out frameinfo given in argument
	 * and if not, change argument to the best possible match
	 */
	/* in combined mode, input image size has to be aligned on 16 pixels */
	if (in->width & 0x0F) {
		dev_warn_ratelimited(g1->dev,
				     "%s  input image size (%d) not aligned on 16 pixels\n",
				     ctx->name, in->width);
		in->width &= ~0x0F;
		in->flags |= G1_FRAMEINFO_FLAG_WIDTH_UPDATED;
	}
	if (in->height & 0x0F) {
		dev_warn_ratelimited(g1->dev,
				     "%s  input image size (%d) not aligned on 16 pixels\n",
				     ctx->name, in->height);
		in->height &= ~0x0F;
		in->flags |= G1_FRAMEINFO_FLAG_HEIGHT_UPDATED;
	}

	if (in->flags & G1_FRAMEINFO_FLAG_CROP) {
		/* In case of crop, update values on G1 constraints :
		 * - Cropped picture width & height : aligned on 8 pixels
		 * - Cropping start coordinates (X,Y) : aligned on 16 pixels
		 */
		if (in->crop.width & 7) {
			in->crop.width &= ~7;
			in->flags |= G1_FRAMEINFO_FLAG_CROP_WIDTH_UPDATED;
		}
		if (in->crop.height & 7) {
			in->crop.height &= ~7;
			in->flags |= G1_FRAMEINFO_FLAG_CROP_HEIGHT_UPDATED;
		}
		in->crop.left &= ~15;
		in->crop.top &= ~15;
	}

	/* Scaling limitation */
	/**********************/
	if (in->flags & G1_FRAMEINFO_FLAG_CROP) {
		pp_check_scaling_config(ctx,
					in->crop.width, in->crop.height, out);
	} else {
		pp_check_scaling_config(ctx,
					in->aligned_width, in->height,
					out);
	}

	/* PP output frame alignment constraints:
	 * 8 pixels width & 2 pixels height
	 *
	 * In addition to these PP constraints, we add
	 * those coming from GPU : in NV12
	 * - chroma address has to be aligned on 64bits
	 *   (consequence is that we have to force
	 *    aligned_height on 64bits)
	 * - stride width has to be aligned on 16 bytes
	 */
	if (out->aligned_width & 15) {
		out->aligned_width = out->aligned_width & ~15;
		out->flags |= G1_FRAMEINFO_FLAG_WIDTH_UPDATED;
	}
	if (out->aligned_height & 7) {
		out->aligned_height = out->aligned_height & ~7;
		out->flags |= G1_FRAMEINFO_FLAG_HEIGHT_UPDATED;
	}
	if (pp->dec) {
		/* reaffect width and height to inform appli with
		 * finale PP output size (aligned one)
		 * Not done when there is no decoder as width/height
		 * is used to compute output crop
		 */
		out->width = out->aligned_width;
		out->height = out->aligned_height;
	}
	/* Check that output format is supported by PP.
	 * if not, switch to NV12 by default
	 */
	i = ARRAY_SIZE(pp_output_formats) - 1;
	while (i >= 0) {
		if (out->pixelformat == pp_output_formats[i]) {
			supported_fmt = true;
			break;
		}
		i--;
	}
	if (!supported_fmt) {
		dev_dbg(g1->dev,
			"Unsupported output format, force it to NV12");
		out->pixelformat = V4L2_PIX_FMT_NV12;
	}
	return 0;
}

int g1_pp_set_config(struct g1_ctx *ctx, struct g1_frameinfo *dec_frameinfo,
		     struct g1_frame *out_frame)
{
	struct g1_dev *g1 = ctx->dev;
	struct g1_pp *pp = &ctx->pp;
	struct g1_frameinfo *out_frameinfo = out_frame->info;
	struct g1_pp_frame_addr out_addr;
	PPResult ppret;
	PPConfig ppconfig;
	unsigned char str[100] = "";
	unsigned char str2[100] = "";

	dev_dbg(g1->dev, "%s  set pp config: %s => %s\n",
		ctx->name,
		g1_frameinfo_str(dec_frameinfo, str, sizeof(str)),
		g1_frameinfo_str(out_frameinfo, str2, sizeof(str2)));

	memset(&ppconfig, 0, sizeof(ppconfig));
	ppret = PPGetConfig(pp->pp, &ppconfig);

	/* set output image */
	ppconfig.ppOutImg.pixFormat = to_pp_format(out_frameinfo->pixelformat);
	ppconfig.ppOutImg.width = out_frameinfo->aligned_width;
	ppconfig.ppOutImg.height = out_frameinfo->aligned_height;

	g1_pp_prepare_addr(&out_addr, out_frame->paddr,
			   out_frameinfo->pixelformat,
			   ppconfig.ppOutImg.width, ppconfig.ppOutImg.height);
	ppconfig.ppOutImg.bufferBusAddr = out_addr.y;
	ppconfig.ppOutImg.bufferChromaBusAddr = out_addr.cb;

	/* set input image */
	ppconfig.ppInImg.pixFormat =
		to_pp_format(dec_frameinfo->pixelformat);
	ppconfig.ppInImg.picStruct = PP_PIC_FRAME_OR_TOP_FIELD;
	ppconfig.ppInImg.width = dec_frameinfo->aligned_width;
	ppconfig.ppInImg.height = dec_frameinfo->aligned_height;

	ppconfig.ppInRotation.rotation = PP_ROTATION_NONE;
	if (out_frameinfo->ctrls.is_hflip_requested)
		ppconfig.ppInRotation.rotation = PP_ROTATION_HOR_FLIP;
	else if (out_frameinfo->ctrls.is_vflip_requested)
		ppconfig.ppInRotation.rotation = PP_ROTATION_VER_FLIP;

	/* Update YUV to RGB conversion related parameters */
	ppconfig.ppOutRgb.contrast = out_frameinfo->ctrls.contrast;
	ppconfig.ppOutRgb.brightness = out_frameinfo->ctrls.brightness;
	ppconfig.ppOutRgb.saturation = out_frameinfo->ctrls.saturation;
	ppconfig.ppOutRgb.alpha = out_frameinfo->ctrls.alpha;
	ppconfig.ppOutRgb.transparency = (out_frameinfo->ctrls.alpha != 0);

	if (!pp->dec) {
		struct g1_pp_frame_addr in_addr;
		u32 in_pic_ba = dec_frameinfo->paddr;
		u32 in_pic_struct = ppconfig.ppInImg.picStruct;
		u32 in_pic_fmt = ppconfig.ppInImg.pixFormat;
		u32 in_pic_size = dec_frameinfo->aligned_width *
			       dec_frameinfo->aligned_height;

		memset(&in_addr, 0, sizeof(in_addr));

		if (dec_frameinfo->field == V4L2_FIELD_INTERLACED)
			in_pic_struct = PP_PIC_TOP_AND_BOT_FIELD_FRAME;

		/* Code every cases even if currently everything is not usable.
		 * Actually we force usage of PP_PIC_TOP_AND_BOT_FIELD_FRAME
		 * in case of Interlaced mode to match with camera use-case
		 * To be extend if needed..
		 */
		if (in_pic_struct != PP_PIC_BOT_FIELD) {
			in_addr.y = in_pic_ba;
			in_addr.cb = in_pic_ba + in_pic_size;
			in_addr.cr = in_addr.cb + (in_pic_size >> 2);
		} else {
			in_addr.ybot = in_pic_ba;
			in_addr.chbot = in_pic_ba + in_pic_size;
		}

		if (in_pic_struct == PP_PIC_TOP_AND_BOT_FIELD_FRAME) {
			if (in_pic_fmt == PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR) {
				in_addr.ybot = in_addr.y +
					(dec_frameinfo->aligned_width);
				in_addr.chbot =	in_addr.cb +
					(dec_frameinfo->aligned_width);
			} else {
				in_addr.ybot = in_addr.y +
					2 * (dec_frameinfo->aligned_width);
			}
		} else if (in_pic_struct == PP_PIC_TOP_AND_BOT_FIELD) {
			if (in_pic_fmt == PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR) {
				in_addr.ybot = in_addr.y +
					dec_frameinfo->aligned_width *
					dec_frameinfo->aligned_height / 2;
				in_addr.chbot =	in_addr.cb +
					dec_frameinfo->aligned_width *
					dec_frameinfo->aligned_height / 4;
			} else {
				in_addr.ybot = in_addr.y +
					2 * dec_frameinfo->aligned_width *
					dec_frameinfo->aligned_height / 2;
			}
		}

		ppconfig.ppInImg.bufferBusAddr = in_addr.y;
		ppconfig.ppInImg.bufferCbBusAddr = in_addr.cb;
		ppconfig.ppInImg.bufferCrBusAddr = in_addr.cr;
		ppconfig.ppInImg.bufferBusAddrBot = in_addr.ybot;
		ppconfig.ppInImg.bufferBusAddrChBot = in_addr.chbot;
		ppconfig.ppInImg.picStruct = in_pic_struct;
	}

	/* set the dynamic range of input pixel samples */
	ppconfig.ppInImg.videoRange = 0;
	if (dec_frameinfo->colorspace == V4L2_COLORSPACE_JPEG)
		ppconfig.ppInImg.videoRange = 1;

	if (!memcmp(dec_frameinfo, out_frameinfo, sizeof(*dec_frameinfo))) {
		/* input is same than output => memcpy mode */
		dev_dbg(g1->dev, "%s  pp memcpy mode [%s]\n", ctx->name,
			g1_frameinfo_str(out_frameinfo, str, sizeof(str)));
		goto memcpy_mode;
	}

	/* input crop */
	if ((dec_frameinfo->width < dec_frameinfo->aligned_width) ||
	    (dec_frameinfo->height < dec_frameinfo->aligned_height)) {
		/* alignment is there, crop extra pixels */
		ppconfig.ppInCrop.enable = 1;
		ppconfig.ppInCrop.originX = 0;
		ppconfig.ppInCrop.originY = 0;
		ppconfig.ppInCrop.width = dec_frameinfo->width;
		ppconfig.ppInCrop.height = dec_frameinfo->height;
	}
	if (dec_frameinfo->flags & G1_FRAMEINFO_FLAG_CROP) {
		/* explicit frame crop superseed alignment crop */
		ppconfig.ppInCrop.enable = 1;
		ppconfig.ppInCrop.originX = dec_frameinfo->crop.left;
		ppconfig.ppInCrop.originY = dec_frameinfo->crop.top;
		ppconfig.ppInCrop.width = dec_frameinfo->crop.width;
		ppconfig.ppInCrop.height = dec_frameinfo->crop.height;
	}

	/* output crop */
	if ((out_frameinfo->width < out_frameinfo->aligned_width) ||
	    (out_frameinfo->height < out_frameinfo->aligned_height) ||
	    (out_frameinfo->flags & G1_FRAMEINFO_FLAG_CROP)) {
		/* alignment is there, crop extra pixels */
		ppconfig.ppOutFrmBuffer.enable = 1;
		ppconfig.ppOutFrmBuffer.writeOriginX = 0;
		ppconfig.ppOutFrmBuffer.writeOriginY = 0;
		ppconfig.ppOutFrmBuffer.frameBufferHeight =
			out_frameinfo->aligned_height;
		ppconfig.ppOutFrmBuffer.frameBufferWidth =
			out_frameinfo->aligned_width;

		if (out_frameinfo->flags & G1_FRAMEINFO_FLAG_CROP) {
			ppconfig.ppOutImg.width =
				out_frameinfo->crop.width & ~0x7;
			ppconfig.ppOutImg.height =
				out_frameinfo->crop.height & ~0x3;
		} else {
			ppconfig.ppOutImg.width =
				out_frameinfo->width & ~0x7;
			ppconfig.ppOutImg.height =
				out_frameinfo->height & ~0x3;
		}
	}

	/* check if we need to deinterlace the stream */
	if (out_frameinfo->deinterlace_activable) {
		if (out_frameinfo->field == V4L2_FIELD_NONE &&
		    dec_frameinfo->field == V4L2_FIELD_INTERLACED) {
			ppconfig.ppOutDeinterlace.enable = 1;
		}
	}

memcpy_mode:
	dev_dbg(g1->dev, "%s  PPSetConfig (%s)\n",
		ctx->name,
		pp_config_str(&ppconfig, str, sizeof(str)));

	ppret = PPSetConfig(pp->pp, &ppconfig);
	if (ppret != PP_OK) {
		dev_err(g1->dev, "%s  PPSetConfig error %s(%d) with config: %s\n",
			ctx->name, ppret_str(ppret), ppret,
			pp_config_str(&ppconfig, str, sizeof(str)));
		return -EIO;
	}

	return 0;
}

int g1_pp_get_frame(struct g1_ctx *ctx, struct g1_frame **pframe)
{
	struct g1_dev *g1 = ctx->dev;
	struct g1_pp *pp = &ctx->pp;
	PPOutput ppOut;
	PPResult ppret;
	unsigned int i;
	unsigned int found = 0;
	struct g1_frame *frame;

	ppret = PPGetResult(pp->pp);
	if (ppret != PP_OK) {
		dev_err(g1->dev, "%s PPGetResult error %s(%d)\n", ctx->name,
			ppret_str(ppret), ppret);
		return -EIO;
	}

	ppret = PPGetNextOutput(pp->pp, &ppOut);
	if (ppret != PP_OK) {
		dev_err(g1->dev, "%s PPGetNextOutput error %s(%d)\n", ctx->name,
			ppret_str(ppret), ppret);
		return -EIO;
	}

	for (i = 0; i < ctx->nb_of_frames; i++) {
		frame = ctx->frames[i];
		if (frame->paddr == ppOut.bufferBusAddr) {
			found = 1;
			break;
		}
	}

	if (!found) {
		dev_err(g1->dev, "%s no frame found for post-processed buffer=0x%x\n",
			ctx->name, ppOut.bufferBusAddr);
		return -EIO;
	}

	*pframe = frame;

	return 0;
}
