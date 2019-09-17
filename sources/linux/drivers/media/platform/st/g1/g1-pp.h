/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef G1_PP_H
#define G1_PP_H

#include "ppapi.h"

#define PP_MIN_W 16
#define PP_MAX_W 4096

#define PP_MIN_H 16
#define PP_MAX_H 4096

static const __u32 pp_output_formats[] = {
	/* XBGR8888. 32 bits. x:B:G:R 8:8:8:8 little endian */
	V4L2_PIX_FMT_XBGR32, /* is actually xRGB => PP_PIX_FMT_RGB32*/
	/* XRGB8888. 32 bits. x:R:G:B 8:8:8:8 little endian */
	V4L2_PIX_FMT_XRGB32, /* is actually xRGB => PP_PIX_FMT_BGR32*/
	/* ABGR8888. 32 bits. A:B:G:R 8:8:8:8 little endian */
	V4L2_PIX_FMT_ABGR32, /* is actually xRGB => PP_PIX_FMT_RGB32*/
	/* ARGB8888. 32 bits. A:R:G:B 8:8:8:8 little endian */
	V4L2_PIX_FMT_ARGB32, /* is actually xRGB => PP_PIX_FMT_BGR32*/
	/* RGB565. [15:0] R:G:B 5:6:5 little endian */
	V4L2_PIX_FMT_RGB565, /* PP_PIX_FMT_RGB16_5_6_5 */
	/* 16  XRGB-1-5-5-5  */
	V4L2_PIX_FMT_XRGB555, /* PP_PIX_FMT_RGB16_5_5_5 */
	/* 16  ARGB-1-5-5-5  */
	V4L2_PIX_FMT_ARGB555, /* PP_PIX_FMT_RGB16_5_5_5 */
	/* NV12. YUV420SP - 1 plane for Y + 1 plane for (CbCr) */
	V4L2_PIX_FMT_NV12, /* PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR */
	/* YUY2 YUV422 Interleaved - 1 plane YCbYCr */
	V4L2_PIX_FMT_YUYV, /* PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED */
};

struct g1_pp {
#ifdef CONFIG_VIDEO_ST_G1_PP
	PPInst pp;
	const void *dec;
#endif
};

struct g1_frame;
struct g1_streaminfo;
struct g1_frameinfo;

int g1_pp_open(struct g1_ctx *ctx, const void *dec, __u32 streamformat);
int g1_pp_close(struct g1_ctx *ctx);
int g1_pp_check_config(struct g1_ctx *ctx, struct g1_frameinfo *in,
		       struct g1_frameinfo *out);
int g1_pp_set_config(struct g1_ctx *ctx, struct g1_frameinfo *dec_frameinfo,
		     struct g1_frame *out_frame);
int g1_pp_get_frame(struct g1_ctx *ctx, struct g1_frame **frame);

#endif /* G1_PP_H */
