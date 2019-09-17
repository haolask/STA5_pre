/*
 * Copyright (C) STMicroelectronics SA 2016
 * Authors:
 *	Stéphane Danieau <stephane.danieau@st.com>
 *	Hugues Fruchet <hugues.fruchet@st.com>
 *	Jean-Christophe Trotin <jean-christophe.trotin@st.com>
 *	for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/slab.h>

#include "g1.h"
#include "stack/inc/on2rvdecapi.h"
#include "stack/inc/rvdecapi.h"
#include "g1-cfg.h"
#include "g1-mem.h"

#include "stack/common/bqueue.h"

#define G1_RV_HEADER_PARSED	1
#define G1_RV_FLUSHING 2
#define G1_MAX_SLICE_OFFSET	128

struct g1_rv_parser_info {
	int slice_count;
	int slice_offset[G1_MAX_SLICE_OFFSET];
	int pts;
};

struct g1_rv_ctx {
	RvDecInst rv;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;
	struct g1_rv_parser_info parserinfo;
	unsigned char *extradata;
	unsigned int extradata_size;
	u64 currentts;
	u32 total_duration;
	u32 lastpid;
	unsigned int state;
	unsigned int eos;
	RvDecSliceInfo sliceInfo[G1_MAX_SLICE_OFFSET];
};

#define to_ctx(ctx) ((struct g1_rv_ctx *)(ctx)->priv)
#define GET_PTS_DIFF(a, b) (((a) - (b) + 8192) & 0x1FFF)

static inline int to_ret(RvDecRet ret)
{
	switch (ret) {
	case RVDEC_OK:
	case RVDEC_STRM_PROCESSED:
	case RVDEC_PIC_RDY:
	case RVDEC_PIC_DECODED:
	case RVDEC_HDRS_RDY:
	case RVDEC_NONREF_PIC_SKIPPED:
		return 0;
	case RVDEC_STRM_ERROR:
		return 0;
	case RVDEC_MEMFAIL:
		return -ENOMEM;
	case RVDEC_HDRS_NOT_RDY:
	case RVDEC_STREAM_NOT_SUPPORTED:
		return -EINVAL;
	default:
		return -EIO;
	}
}

static inline int is_stream_error(RvDecRet ret)
{
	switch (ret) {
	case RVDEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *rvret_str(RvDecRet ret)
{
	switch (ret) {
	case RVDEC_OK:
		return "RVDEC_OK";
	case RVDEC_STRM_PROCESSED:
		return "RVDEC_STRM_PROCESSED";
	case RVDEC_PIC_RDY:
		return "RVDEC_PIC_RDY";
	case RVDEC_PIC_DECODED:
		return "RVDEC_PIC_DECODED";
	case RVDEC_HDRS_RDY:
		return "RvDec_HDRS_RDY";
	case RVDEC_NONREF_PIC_SKIPPED:
		return "RVDEC_NONREF_PIC_SKIPPED";
	case RVDEC_PARAM_ERROR:
		return "RVDEC_PARAM_ERROR";
	case RVDEC_STRM_ERROR:
		return "RVDEC_STRM_ERROR";
	case RVDEC_NOT_INITIALIZED:
		return "RVDEC_NOT_INITIALIZED";
	case RVDEC_MEMFAIL:
		return "RVDEC_MEMFAIL";
	case RVDEC_INITFAIL:
		return "RVDEC_INITFAIL";
	case RVDEC_HDRS_NOT_RDY:
		return "RvDec_HDRS_NOT_RDY";
	case RVDEC_STREAM_NOT_SUPPORTED:
		return "RVDEC_STREAM_NOT_SUPPORTED";
	case RVDEC_HW_RESERVED:
		return "RVDEC_HW_RESERVED";
	case RVDEC_HW_TIMEOUT:
		return "RVDEC_HW_TIMEOUT";
	case RVDEC_HW_BUS_ERROR:
		return "RVDEC_HW_BUS_ERROR";
	case RVDEC_SYSTEM_ERROR:
		return "RVDEC_SYSTEM_ERROR";
	case RVDEC_DWL_ERROR:
		return "RVDEC_DWL_ERROR";
	case RVDEC_FORMAT_NOT_SUPPORTED:
		return "RVDEC_FORMAT_NOT_SUPPORTED";
	default:
		return "!unknown rv return value!";
	}
}

static inline int to_v4l2_pixelformat(RvDecOutFormat fmt)
{
	switch (fmt) {
	case RVDEC_SEMIPLANAR_YUV420:
		return V4L2_PIX_FMT_NV12;
	default:
		return 0;
	}
}

static int g1_rv_probe(struct g1_dev *g1)
{
	/* check if codec is supported by hardware */
	if (!g1->hw.config.rv_support)
		return -EACCES;

	return 0;
}

static int g1_rv_open(struct g1_ctx *pctx)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx;
	RvDecRet rvret;
	int ret;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	pctx->priv = ctx;
	ctx->lastpid = -1;
	ctx->total_duration = 0;

	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));

	rvret = RvDecInit((RvDecInst *)&ctx->rv,
			  0, /* useVideoFreezeConcealment */
			  0,
			  NULL,
			  (ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV30 ?
			   0 : 1),
			  ctx->streaminfo.width, /* u32 maxFrameWidth*/
			  ctx->streaminfo.height, /*, u32 maxFrameHeight */
			  0,
			  DEC_REF_FRM_RASTER_SCAN, (void *)pctx);

	if (rvret) {
		dev_err(g1->dev, "%s  RvDecInit error %s(%d)\n", pctx->name,
			rvret_str(rvret), rvret);
		ret = -EIO;
		goto err_free;
	}

	ret = g1_pp_open(pctx, ctx->rv, ctx->streaminfo.streamformat);
	if (ret)
		goto err_release;

	return 0;

err_release:
	RvDecRelease(ctx->rv);
err_free:
	kfree(ctx);
	return ret;
}

static int g1_rv_close(struct g1_ctx *pctx)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	RvDecRelease(ctx->rv);

	kfree(ctx->extradata);
	kfree(ctx);

	return 0;
}

static int g1_rv_set_streaminfo(struct g1_ctx *pctx, RvDecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;

	streaminfo->width = decinfo->codedWidth;
	streaminfo->height = decinfo->codedHeight;
	streaminfo->aligned_width = decinfo->frameWidth << 4;
	streaminfo->aligned_height = decinfo->frameHeight << 4;

	/* check width/height */
	if ((streaminfo->width == 0) || (streaminfo->height == 0) ||
	    (streaminfo->width * streaminfo->height > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid stream coded resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, streaminfo->width, streaminfo->height,
			G1_MAX_RESO);
		return -EINVAL;
	}

	streaminfo->field = V4L2_FIELD_NONE;
	streaminfo->dpb = 0;

	return 0;
}

static int g1_rv_set_dec_frameinfo(struct g1_ctx *pctx, RvDecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	struct g1_frameinfo *frameinfo = &ctx->dec_frameinfo;
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;
	__u32 fmt;

	/* check decoder output width/height */
	if ((streaminfo->aligned_width == 0) ||
	    (streaminfo->aligned_height == 0) ||
	    (streaminfo->aligned_width * streaminfo->aligned_height
							> G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid decoder output resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, streaminfo->aligned_width,
			streaminfo->aligned_height,
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
	frameinfo->width = streaminfo->width;
	frameinfo->height = streaminfo->height;
	frameinfo->aligned_width = streaminfo->aligned_width;
	frameinfo->aligned_height = streaminfo->aligned_height;
	frameinfo->field = V4L2_FIELD_NONE;

	return 0;
}

static int g1_rv_get_streaminfo(struct g1_ctx *pctx,
				struct g1_streaminfo *streaminfo)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_RV_HEADER_PARSED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_rv_get_frameinfo(struct g1_ctx *pctx,
			       struct g1_frameinfo *frameinfo)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_RV_HEADER_PARSED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_rv_set_frameinfo(struct g1_ctx *pctx,
			       struct g1_frameinfo *frameinfo)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_RV_HEADER_PARSED)
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

#define GET_MAJOR_VER(ver) (((ver) >> 28) & 0x0F)
#define GET_MINOR_VER(ver) (((ver) >> 20) & 0xFF)
#define RPR_BUFFER_SIZE	18

static int g1_rv_parse_opaque_data(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	On2RvMsgSetDecoderRprSizes msg;
	On2RvDecRet rvRet;
	u32 tmpBuf[RPR_BUFFER_SIZE];
	int nb_dim = 0;
	u32 version = 0;
	int i = 0, j = 0;

	if (ctx->extradata)
		return 0;

	if (au->size < 8)
		/* Not enought data to continue */
		return -ENODATA;

	/* In RV8 (RV30) :
	 * Codec Data in case of RV is built as following :
	 *   4 bytes to identify SPO Flags
	 *   4 bytes to identify Bitstream version
	 *   then a variable number of resolution of resampled images
	 * In RV9 (RV40) :
	 *  same organization for two first 32bits
	 *  (SPO + version) but there is no RPR data
	 */
	dev_dbg(g1->dev, "%s %s Extra data detected\n",
		pctx->name, __func__);
	/* Assuming this is the codec_data */
	ctx->extradata = kcalloc(au->size, sizeof(u8),
					GFP_KERNEL);
	if (!ctx->extradata)
		return -ENOMEM;

	memcpy(ctx->extradata, au->vaddr, au->size);
	ctx->extradata_size = au->size;

	version = cpu_to_be32((u32)(*(ctx->extradata + 4)));

	if (!((GET_MAJOR_VER(version) == 3 &&
	       ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV30) ||
	      (GET_MAJOR_VER(version) == 4 &&
	       ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV40))) {
		dev_err(g1->dev,
			"%s %s RV version (M:%d/m:%d) not supported\n",
			pctx->name, __func__,
			GET_MAJOR_VER(version), GET_MINOR_VER(version));
		kfree(ctx->extradata);
		ctx->extradata = NULL;
		return -EINVAL;
	}

	/* No relevant information to catch in case of RV40, go-out */
	if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV40)
		return -ENODATA;

	/* In case of RV30, RPR info have to be sent to G1 stack :
	 * number of possible resampled images sizes
	 *  SPO Flag : RV20_SRV20_SPO_BITS_NUMRESAMPLE_IMAGES
	 *  Mask : 0x00070000 max of 8 RPR images size
	 */
	nb_dim = ctx->extradata[1] & 7;
	if ((nb_dim + 1) << 1 > RPR_BUFFER_SIZE) {
		dev_warn(g1->dev, "more than 8 RPR images => %d !!!\n", nb_dim);
		nb_dim = 8;
	}

	msg.message_id = ON2RV_MSG_ID_Set_RVDecoder_RPR_Sizes;

	/* We add one dimension to store native size */
	msg.num_sizes = nb_dim + 1;
	msg.sizes = tmpBuf;
	i = 0;
	/* Start by sending native resolution */
	msg.sizes[i++] = ctx->streaminfo.width;
	msg.sizes[i++] = ctx->streaminfo.height;

	/* Then complete with res coming from extradata */
	for (j = 8 ; i < (msg.num_sizes << 1); i++, j++) {
		msg.sizes[i] = (ctx->extradata[j] << 2);
		dev_dbg(g1->dev, "%d\n", msg.sizes[i]);
	}

	rvRet = On2RvDecCustomMessage(&msg, (void *)ctx->rv);
	if (rvRet != ON2RVDEC_OK) {
		dev_err(g1->dev,
			"%s %s Unable to send RPR data to the stack\n",
			pctx->name, __func__);
		return -1;
	}
	return -ENODATA;
}

static int g1_rv_find_slices(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	int ret = 0;
	const u8 *pdata = au->vaddr;
	u32 slice_info_size = 0;
	const u32 *pSliceDes;
	int sliceId = 0;
	struct g1_rv_parser_info *pinfo = &ctx->parserinfo;

	/* Check if this AU is "opaque data" (i.e CodecData or extradata) */
	ret = g1_rv_parse_opaque_data(pctx, au);
	if (ret)
		return ret;

	/* Description of beginning of the frame (not sent to G1) :
	 *	First byte stores "Slice count - 1"
	 *	then (SliceCount) times :
	 *		four empty bytes
	 *		then four bytes storing the offset
	 */
	pinfo->slice_count = pdata[0] + 1;
	pdata++;
	pSliceDes = (u32 *)pdata;

	if (pinfo->slice_count >= G1_MAX_SLICE_OFFSET)
		dev_err(g1->dev, "%s  invalid slice count (%d>%d)",
			pctx->name, pinfo->slice_count, G1_MAX_SLICE_OFFSET);

	for (sliceId = 0;
	     sliceId < pinfo->slice_count && sliceId < G1_MAX_SLICE_OFFSET;
	     sliceId++) {
		pSliceDes++;
		pinfo->slice_offset[sliceId] = *(pSliceDes);
		pSliceDes++;
	}
	slice_info_size = 1 + 8 * pinfo->slice_count;

	/* Update frame buffer to remove information linked to number of
	 * slices and offsets.
	 */
	au->size -= slice_info_size;
	memcpy(au->vaddr, au->vaddr + slice_info_size, au->size);
	return 0;
}

static int g1_rv_parse_slice_header_rv8(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	struct g1_rv_parser_info *pinfo = &ctx->parserinfo;
	u8 *pdata = au->vaddr;
	u32 rv_tr;
	u32 tmp;

	/*  Slice Header field description RV8
	 *  ---------------------------------------
	 * bit0 - bit2 : FIELDLEN_RV_BITSTREAM_VERSION
	 * bit3 - bit4 : PicCodType
	 * \-> (00) = RV_INTRAPIC
	 *     (01) = RV_FORCED_INTRAPIC
	 *     (10) = RV_INTERPIC
	 *     (11) = RV_TRUEBPIC
	 * bit5 : ECC - 0
	 * bit6 - bit10 : FIELDLEN_SQUANT Quant
	 * bit11 Deblock Pass Thru
	 * \-> 1 Enable Deblocking filter
	 * bit12 - bit24 : FIELDLEN_TR_RV
	 * then picture size
	 * and MBA data
	 * followed by one bit ; FIELDLEN_RTYPE
	 */

	/* We need to swith to big endian */
	tmp = cpu_to_be32(*((u32 *)pdata));

	/* G1 just need the timestamp : FIELDLEN_TR_RV
	 * bit12 from the left then shift to the right (32-(13+12))
	 */
	rv_tr = (tmp & 0x000FFF80) >> 7;
	pinfo->pts = rv_tr;
	return 0;
}

static int g1_rv_parse_slice_header_rv9(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	struct g1_rv_parser_info *pinfo = &ctx->parserinfo;
	u8 *pdata = au->vaddr;
	u32 rv_tr;
	u32 tmp;

	/*  Slice Header field description RV9
	 *  ---------------------------------------
	 *  bit0  ECC
	 *  \-> 0 if slice contains picture data
	 *      1 if slice contains ECC information
	 * bit1-2 PicCodType
	 * \-> (00) = RV_INTRAPIC
	 *     (01) = RV_FORCED_INTRAPIC
	 *     (10) = RV_INTERPIC
	 *     (11) = RV_TRUEBPIC
	 * bit3 - bit7 SQUANT : Initial Slice Quantization Parameter
	 * bit 8 : Bitstream Version : Reserved – always zero.
	 * bit 9 : Interlaced : Interlaced Slice Coding
	 * bit10 - bit11 : OSV Quant : Super VLC Quantizer
	 * bit12 - Deblock PassThru
	 *   \-> 0 if deblocking filter is to be used
	 *       1 if deblocking filter is to be disabled
	 * bit13- bit25 : RV TR
	 *   \-> Temporal reference (in units of millisecs)
	 * variable size : Decoded Picture size.
	 * variable size : MBA_NumMBs= (width + 15)>>4 * (height + 15)>>4 - 1
	 * -------------------------------------------
	 */

	/* We need to swith to big endian */
	tmp = cpu_to_be32(*((u32 *)pdata));

	/* G1 just need PTS info (RV TR )
	 * bit13 from the left then shift to the right (32-26)
	 */
	rv_tr = (tmp & 0x0007FFC0) >> 6;
	pinfo->pts = rv_tr;

	return 0;
}

static int g1_rv_parse_slice_header(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	int ret = -EINVAL;

	if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV30)
		ret = g1_rv_parse_slice_header_rv8(pctx, au);
	else if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_RV40)
		ret = g1_rv_parse_slice_header_rv9(pctx, au);
	else
		dev_err(g1->dev, "%s  Unknown format", pctx->name);

	return ret;
}

static int g1_rv_decode(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	int ret, i;
	RvDecRet rvret;
	RvDecInput in;
	RvDecOutput out;
	RvDecInfo decinfo;
	struct g1_rv_parser_info *parserinfo = &ctx->parserinfo;

	dev_dbg(g1->dev, "%s  > %s\n", pctx->name, __func__);

	if (ctx->state >= G1_RV_HEADER_PARSED) {
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
	ret = g1_rv_find_slices(pctx, au);
	if (ret < 0)
		return ret;

	ret = g1_rv_parse_slice_header(pctx, au);
	if (ret < 0)
		return ret;

	memset(&in, 0, sizeof(in));
	in.pStream = au->vaddr;
	in.streamBusAddress = au->paddr;
	in.dataLen = au->size;
	in.timestamp = parserinfo->pts;
	in.picId = in.timestamp;
	in.skipNonReference = 0; // Set by default to false

	in.sliceInfoNum = parserinfo->slice_count;

	if (in.sliceInfoNum >= G1_MAX_SLICE_OFFSET)
		dev_err(g1->dev, "%s  invalid slice count (%d>%d)",
			pctx->name, in.sliceInfoNum, G1_MAX_SLICE_OFFSET);

	memset(ctx->sliceInfo, 0, sizeof(RvDecSliceInfo) * G1_MAX_SLICE_OFFSET);

	for (i = 0;
	     i < in.sliceInfoNum && i < G1_MAX_SLICE_OFFSET;
	     i++) {
		ctx->sliceInfo[i].offset = parserinfo->slice_offset[i];
		ctx->sliceInfo[i].isValid = 1;
	}
	in.pSliceInfo = ctx->sliceInfo;

	memset(&out, 0, sizeof(out));

	/* decode this access unit */
	rvret = RvDecDecode(ctx->rv, &in, &out);

	/* check decode status */
	ret = to_ret(rvret);
	if (ret) {
		dev_err(g1->dev, "%s  RvDecDecode error %s(%d)",
			pctx->name, rvret_str(rvret), rvret);
		pctx->decode_errors++;
		return ret;
	}

	if (is_stream_error(rvret)) {
		dev_warn_ratelimited(g1->dev,
				     "%s  stream error @ frame %d, au size=%d\n",
				     pctx->name, pctx->decoded_frames,
				     au->size);
		pctx->stream_errors++;
		return 0;
	}

	if (rvret == RVDEC_HDRS_RDY) {
		/* header detected */
		/* get new stream infos */
		rvret = RvDecGetInfo(ctx->rv, &decinfo);
		if (rvret) {
			dev_err(g1->dev, "%s  RvDecGetInfo error (%d)\n",
				pctx->name,  rvret);
			return -EIO;
		}

		/* new live header ? */
		if (ctx->state >= G1_RV_HEADER_PARSED) {
			/* header already detected previously */
			dev_info(g1->dev,
				 "%s  new header detection while decoding\n",
				 pctx->name);
			return -ENODATA;
		}

		/* first header, store infos */
		ret = g1_rv_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_rv_set_dec_frameinfo(pctx, &decinfo);
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

		ctx->state = G1_RV_HEADER_PARSED;
		return -ENODATA;
	}

	switch (rvret) {
	case RVDEC_OK:
	case RVDEC_STRM_PROCESSED:
	case RVDEC_PIC_RDY:
	case RVDEC_HDRS_NOT_RDY:
	case RVDEC_NONREF_PIC_SKIPPED:
		/* output (decoded data) not expected so return nodata */
		return -ENODATA;
	case RVDEC_PIC_DECODED:
		/* frame decoded */
		pctx->decoded_frames++;
		return ret;
	default:
		/* no error detected at this point but we didn't treat
		 * the returned value ... Strange !!!
		 * log a trace to investigate if needed
		 */
		dev_err_ratelimited(g1->dev,
				    "%s - Untreated decode returned value : %s(%d)\n",
				    pctx->name, rvret_str(rvret), rvret);
		return -EIO;
	}
	return 0;
}

int g1_rv_update_frame_init_timestamp(struct g1_ctx *pctx, u64 ts)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);

	if (ctx->state == G1_RV_FLUSHING && ctx->currentts == 0) {
		ctx->currentts = ts;
		dev_dbg(g1->dev, "Update Initial timestamp to %lld\n", ts);
	}
	return 0;
}

static int g1_rv_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_rv_ctx *ctx = to_ctx(pctx);
	int ret;
	RvDecRet rvret;
	RvDecPicture picture;

	/* systematically check if we need to allocate a free frame
	 * to the PP.
	 * mainly useful in case of drain to store the last reference
	 * frame ( scheduled by g1_decoder_stop_cmd )
	 */
	if (ctx->state >= G1_RV_HEADER_PARSED) {
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

	rvret = RvDecNextPicture(ctx->rv, &picture, ctx->eos);
	ret = to_ret(rvret);
	if (ret) {
		dev_err(g1->dev, "%s  RvDecNextPicture error %s(%d)\n",
			pctx->name, rvret_str(rvret), rvret);
		pctx->decode_errors++;
		return ret;
	}

	if ((rvret != RVDEC_PIC_RDY) && (rvret != RVDEC_PIC_DECODED)) {
		/* no frame decoded */
		goto nodata;
	}

	/* decoded frame available */

	/* get the post-processed frame */
	ret = g1_pp_get_frame(pctx, frame);
	if (ret)
		return ret;

	if (picture.keyPicture)
		(*frame)->flags |= V4L2_BUF_FLAG_KEYFRAME;
	else
		(*frame)->flags |= V4L2_BUF_FLAG_PFRAME;


	/* Seek management, wait for next IDR frame before allowing the frame
	 * pushing.
	 * to_drop equals true means : we don't push the frame.
	 */
	(*frame)->to_drop = (picture.numberOfErrMBs ? true : false);

	if (ctx->state == G1_RV_FLUSHING) {
		if (!picture.keyPicture)
			(*frame)->to_drop = true; // to force drop frame
		else {
			dev_dbg(g1->dev, "FLUSHING STATE : Iframe received\n");
			ctx->state = G1_RV_HEADER_PARSED;
		}
	}

	if (picture.numberOfErrMBs) {
		dev_warn_ratelimited(g1->dev,
				     "%s  Number of Concealed MB : %d\n",
				     pctx->name, picture.numberOfErrMBs);
		pctx->stream_errors++;
	}

	/* Compare to other codecs, in case of RV, it seems we can't rely on
	 * demux info to compute timestamp.
	 * actually, timestamp is computed thanks to the duration of the frame.
	 * (current timestamp + duration)
	 * This duration is retrieved from the difference between the current
	 * pid and the previous pid.
	 * Issue with this approach : in case of seek (we don't have the initial
	 * timestamp neither the previous pid).
	 * To get this initial timestamp, we added a dedicated API to store the
	 * timestamp associated to the first AU received after the seek (flush)
	 * previous pid is set to -1 in case of flush
	 *
	 * Note: As we're not able to flush on G1 stack, potentially after the
	 * seek, we receive a decode frame from 'before_seek' AU. So last_pid
	 * in this case is not correct. As a workaround, we check that frame
	 * duration is in the average of previous frame duration. if not, we
	 * use this average duration.
	 */
	{
		u32 frame_duration = 0;
		u32 avg_frame_duration = 0;

		if (ctx->lastpid != -1)
			frame_duration =
				GET_PTS_DIFF(picture.picId, ctx->lastpid);

		ctx->total_duration += frame_duration;

		if (pctx->output_frames)
			avg_frame_duration =
				ctx->total_duration / pctx->output_frames;

		/* If currentts is invalid, we can't do anything, so we forward
		 * it to upper layer so that it computes the estimated timestamp
		 * by itself.
		 */
		if (ctx->lastpid != -1 &&
		    ctx->currentts != G1_INVALID_TIMESTAMP) {
			if (frame_duration > (avg_frame_duration << 1)) {
				/* Incoherent duration, we switch to the
				 * computed average duration
				 */
				dev_dbg(g1->dev,
					"Average duration usage\n");
				frame_duration = avg_frame_duration;
			}
			ctx->currentts = ctx->currentts +
				(frame_duration * 1000000);
		}
		ctx->lastpid = picture.picId;
		/* Set computed timestamp to the frame */
		(*frame)->ts = ctx->currentts;
	}
	return 0;
nodata:
	if (ctx->eos)
		ctx->eos = 0; /* no more frames to drain: end of EOS */

	return -ENODATA;
}

static void g1_rv_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
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

static void g1_rv_flush(struct g1_ctx *pctx)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);

	/* FIXME find a way to indicate flush to rv stack */
	ctx->state = G1_RV_FLUSHING;
	ctx->lastpid = -1;
	ctx->currentts = 0;
}

static void g1_rv_drain(struct g1_ctx *pctx)
{
	struct g1_rv_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

/* rv decoder can decode VP7 and WEBP contents, add below
 * one decoder struct per supported format
 */
const __u32 rv_stream_formats[] = {
	V4L2_PIX_FMT_RV30,
	V4L2_PIX_FMT_RV40
};

const struct g1_dec rvdec = {
	.name = "rv",
	.streamformat = &rv_stream_formats[0],
	.nb_streams = ARRAY_SIZE(rv_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_rv_probe,
	.open = g1_rv_open,
	.close = g1_rv_close,
	.get_streaminfo = g1_rv_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_rv_get_frameinfo,
	.set_frameinfo = g1_rv_set_frameinfo,
	.decode = g1_rv_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_rv_get_frame,
	.recycle = g1_rv_recycle,
	.flush = g1_rv_flush,
	.drain = g1_rv_drain,
};
