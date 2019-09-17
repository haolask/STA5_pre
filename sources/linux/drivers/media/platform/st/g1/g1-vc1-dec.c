/*
 * Copyright (C) STMicroelectronics SA 2016
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Jean-Christophe Trotin <jean-christophe.trotin@st.com>
 *          Stephane Danieau <stephane.danieau@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/slab.h>

#include "g1.h"
#include "g1-cfg.h"
#include "g1-mem.h"
#include "stack/inc/vc1decapi.h"
#include "stack/common/bqueue.h"

#define G1_VC1_HEADER_PARSED 1
#define G1_VC1_FLUSHING 2

#define VC1_METADATA_SIMPLE_PROFILE 0
#define VC1_METADATA_MAIN_PROFILE 4
#define VC1_METADATA_ADVANCED_PROFILE 12

#define VC1_ADVANCED_SC_SIZE 4

enum vc1_startCode_e {
	SC_END_OF_SEQ       = 0x0000010A,
	SC_SLICE            = 0x0000010B,
	SC_FIELD            = 0x0000010C,
	SC_FRAME            = 0x0000010D,
	SC_ENTRY_POINT      = 0x0000010E,
	SC_SEQ              = 0x0000010F,
	SC_SLICE_UD         = 0x0000011B,
	SC_FIELD_UD         = 0x0000011C,
	SC_FRAME_UD         = 0x0000011D,
	SC_ENTRY_POINT_UD   = 0x0000011E,
	SC_SEQ_UD           = 0x0000011F,
	SC_NOT_FOUND        = 0xFFFE
};

struct g1_vc1_ctx {
	VC1DecInst vc1;
	struct g1_streaminfo streaminfo;
	struct g1_frameinfo dec_frameinfo;
	struct g1_frameinfo pp_frameinfo;
	unsigned int state;
	unsigned int eos;
	struct g1_au *local_au;
	struct g1_buf *local_buf;
	__u8 profile;
};

#define to_ctx(ctx) ((struct g1_vc1_ctx *)(ctx)->priv)

/* debug API of VC1 stack */
void VC1DecTrace(const char *str)
{
#ifdef _VC1DEC_TRACE
	pr_debug("   %s\n", str);
#endif
};

static inline int to_ret(VC1DecRet ret)
{
	switch (ret) {
	case VC1DEC_OK:
	case VC1DEC_STRM_PROCESSED:
	case VC1DEC_PIC_RDY:
	case VC1DEC_PIC_DECODED:
	case VC1DEC_HDRS_RDY:
	case VC1DEC_END_OF_SEQ:
		return 0;
	case VC1DEC_STRM_ERROR:
		return 0;

	case VC1DEC_MEMFAIL:
		return -ENOMEM;

	case VC1DEC_PARAM_ERROR:
	case VC1DEC_NOT_INITIALIZED:
	case VC1DEC_INITFAIL:
	case VC1DEC_METADATA_FAIL:
	case VC1DEC_HW_RESERVED:
	case VC1DEC_HW_TIMEOUT:
	case VC1DEC_HW_BUS_ERROR:
	case VC1DEC_SYSTEM_ERROR:
	case VC1DEC_DWL_ERROR:
	case VC1DEC_FORMAT_NOT_SUPPORTED:
		return -EINVAL;

	default:
		return -EIO;
	}
}

static inline int is_stream_error(VC1DecRet ret)
{
	switch (ret) {
	case VC1DEC_STRM_ERROR:
		return 1;
	default:
		return 0;
	}
}

static inline const char *vc1ret_str(VC1DecRet ret)
{
	switch (ret) {
	case VC1DEC_OK:
		return "VC1DEC_OK";
	case VC1DEC_PIC_RDY:
		return "VC1DEC_PIC_RDY";
	case VC1DEC_STRM_PROCESSED:
		return "VC1DEC_STRM_PROCESSED";
	case VC1DEC_HDRS_RDY:
		return "VC1DEC_HDRS_RDY";
	case VC1DEC_END_OF_SEQ:
		return "VC1DEC_END_OF_SEQ";
	case VC1DEC_PIC_DECODED:
		return "VC1DEC_PIC_DECODED";
	case VC1DEC_RESOLUTION_CHANGED:
		return "VC1DEC_RESOLUTION_CHANGED";
	case VC1DEC_NONREF_PIC_SKIPPED:
		return "VC1DEC_NONREF_PIC_SKIPPED";
	case VC1DEC_PARAM_ERROR:
		return "VC1DEC_PARAM_ERROR";
	case VC1DEC_NOT_INITIALIZED:
		return "VC1DEC_NOT_INITIALIZED";
	case VC1DEC_MEMFAIL:
		return "VC1DEC_MEMFAIL";
	case VC1DEC_INITFAIL:
		return "VC1DEC_INITFAIL";
	case VC1DEC_METADATA_FAIL:
		return "VC1DEC_METADATA_FAIL";
	case VC1DEC_STRM_ERROR:
		return "VC1DEC_STRM_ERROR";
	case VC1DEC_HW_RESERVED:
		return "VC1DEC_HW_RESERVED";
	case VC1DEC_HW_TIMEOUT:
		return "VC1DEC_HW_TIMEOUT";
	case VC1DEC_HW_BUS_ERROR:
		return "VC1DEC_HW_BUS_ERROR";
	case VC1DEC_SYSTEM_ERROR:
		return "VC1DEC_SYSTEM_ERROR";
	case VC1DEC_DWL_ERROR:
		return "VC1DEC_DWL_ERROR";
	case VC1DEC_FORMAT_NOT_SUPPORTED:
		return "VC1DEC_FORMAT_NOT_SUPPORTED";
	default:
		return "!unknown VC1 return value!";
	}
}

static inline int to_v4l2_pixelformat(VC1DecOutFormat fmt)
{
	switch (fmt) {
	case VC1DEC_SEMIPLANAR_YUV420:
		return V4L2_PIX_FMT_NV12;
	default:
		return 0;
	}
}

static int g1_vc1_probe(struct g1_dev *g1)
{
	/* check if codec is supported by hardware */
	if (!g1->hw.config.vc1_support)
		return -EACCES;

	return 0;
}

static int g1_vc1_open(struct g1_ctx *pctx)
{
	struct g1_vc1_ctx *ctx;

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	/* G1 decoder instance is not initialized at this
	 * level because for VC1, a first AU is needed to
	 * create VC1 instance.
	 * So decoder init will be done in g1_vc1_decode
	 * service
	 */
	pctx->priv = ctx;
	memcpy(&ctx->streaminfo, &pctx->streaminfo, sizeof(ctx->streaminfo));
	return 0;
}

static int g1_vc1_close(struct g1_ctx *pctx)
{
	struct g1_vc1_ctx *ctx = to_ctx(pctx);

	g1_pp_close(pctx);

	if (ctx->vc1)
		VC1DecRelease(ctx->vc1);

	kfree(ctx->local_au);
	if (ctx->local_buf) {
		hw_free(pctx, ctx->local_buf);
		kfree(ctx->local_buf);
	}
	kfree(ctx);

	return 0;
}

static int g1_vc1_set_streaminfo(struct g1_ctx *pctx, VC1DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vc1_ctx *ctx = to_ctx(pctx);
	struct g1_streaminfo *streaminfo = &ctx->streaminfo;

	if (decinfo->outputFormat != VC1DEC_SEMIPLANAR_YUV420) {
		dev_err(g1->dev,
			"%s  invalid output format: only YUV420 semiplanar is expected\n",
			pctx->name);
		return -EINVAL;
	}
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

	/* pixel-aspect-ratio */
	streaminfo->pixelaspect.numerator = decinfo->parWidth;
	streaminfo->pixelaspect.denominator = decinfo->parHeight;

	/* interlaced */
	streaminfo->field = (decinfo->interlacedSequence ?
			    V4L2_FIELD_INTERLACED :
			    V4L2_FIELD_NONE);

	streaminfo->dpb = 0;

	if (ctx->profile == VC1_METADATA_ADVANCED_PROFILE) {
		snprintf(streaminfo->profile, sizeof(streaminfo->profile),
			 "Advanced");
	} else {
		snprintf(streaminfo->profile, sizeof(streaminfo->profile),
			 (ctx->profile == VC1_METADATA_MAIN_PROFILE) ?
			 "Main" : "Simple");
	}

	snprintf(streaminfo->level, sizeof(streaminfo->level), "%d", 0);

	return 0;
}

static int g1_vc1_set_dec_frameinfo(struct g1_ctx *pctx, VC1DecInfo *decinfo)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vc1_ctx *ctx = to_ctx(pctx);
	struct g1_frameinfo *frameinfo = &ctx->dec_frameinfo;
	__u32 fmt;

	/* check decoder output width/height */
	if ((decinfo->maxCodedWidth == 0) || (decinfo->maxCodedHeight == 0) ||
	    (decinfo->maxCodedWidth * decinfo->maxCodedHeight > G1_MAX_RESO)) {
		dev_err(g1->dev,
			"%s  invalid decoder output resolution: (%dx%d) is 0 or > %d pixels budget\n",
			pctx->name, decinfo->maxCodedWidth,
			decinfo->maxCodedHeight,
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
	frameinfo->aligned_width = decinfo->maxCodedWidth;
	frameinfo->aligned_height = decinfo->maxCodedHeight;

	if (decinfo->maxCodedWidth & 0x0F)
		frameinfo->aligned_width =
			(decinfo->maxCodedWidth + 15) & ~15;

	if (decinfo->maxCodedHeight & 0x0F)
		frameinfo->aligned_height =
			(decinfo->maxCodedHeight + 15) & ~15;

	/* pixel-aspect-ratio */
	frameinfo->pixelaspect.numerator = decinfo->parWidth;
	frameinfo->pixelaspect.denominator = decinfo->parHeight;

	/* interlaced */
	frameinfo->field = (decinfo->interlacedSequence ?
			   V4L2_FIELD_INTERLACED :
			   V4L2_FIELD_NONE);

	return 0;
}

static int g1_vc1_get_streaminfo(struct g1_ctx *pctx,
				 struct g1_streaminfo *streaminfo)
{
	struct g1_vc1_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_VC1_HEADER_PARSED)
		return -ENODATA;

	*streaminfo = ctx->streaminfo;

	return 0;
}

static int g1_vc1_get_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_vc1_ctx *ctx = to_ctx(pctx);

	if (ctx->state < G1_VC1_HEADER_PARSED)
		return -ENODATA;

	*frameinfo = ctx->pp_frameinfo;

	return 0;
}

static int g1_vc1_set_frameinfo(struct g1_ctx *pctx,
				struct g1_frameinfo *frameinfo)
{
	struct g1_vc1_ctx *ctx = to_ctx(pctx);
	unsigned int ret;

	if (ctx->state < G1_VC1_HEADER_PARSED)
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

static int g1_vc1_init(struct g1_ctx *pctx, VC1DecMetaData *pmetadata)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vc1_ctx *ctx = to_ctx(pctx);
	int ret = 0;
	VC1DecRet vc1ret;

	vc1ret = VC1DecInit(&ctx->vc1,
			    pmetadata,
			    false, /* No video freeze concealment */
			    0,
			    /* No tiled output */
			    DEC_REF_FRM_RASTER_SCAN,
			    /* G1 context given to DWL */
			    (void *)pctx);

	if (vc1ret) {
		dev_err(g1->dev, "%s  VC1DecInit error %s(%d)\n", pctx->name,
			vc1ret_str(vc1ret),
			vc1ret);
		ret = -EIO;
		return ret;
	}

	return g1_pp_open(pctx, ctx->vc1, ctx->streaminfo.streamformat);
}

static u32 g1_vc1_get_next_start_code(struct g1_au *au,
				      int start_offset,
				      int stop_offset,
				      int *sc_pos)
{
	u32 sc = 0;
	u8 *pStrm;
	int lpos = start_offset;

	*sc_pos = 0;

	if (!au->vaddr ||
	    au->size < stop_offset ||
	    stop_offset < start_offset ||
	    (stop_offset - start_offset) < 4)
		return SC_NOT_FOUND;

	while (lpos <= (stop_offset - 4)) {
		pStrm = (u8 *)(au->vaddr + lpos);
		sc = (((u32)pStrm[0]) << 16) +
		     (((u32)pStrm[1]) << 8) +
		     (((u32)pStrm[2]));
		if (sc == 0x000001) {
			if ((pStrm[3] >= 0x0A && pStrm[3] <= 0x0F) ||
			    (pStrm[3] >= 0x1B && pStrm[3] <= 0x1F)) {
				sc = (sc << 8) + pStrm[3];
				*sc_pos = lpos;
				return sc;
			}
		}
		lpos++;
	}
	return SC_NOT_FOUND;
}

static int g1_vc1_decode(struct g1_ctx *pctx, struct g1_au *au)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vc1_ctx *ctx = to_ctx(pctx);
	int ret;
	VC1DecRet vc1ret;
	VC1DecInput in;
	VC1DecOutput out;
	VC1DecInfo decinfo;
	bool first_header_received = false;
	u8 AdvancedStartCode[] = {0x00, 0x00, 0x01, 0x0D};
	struct g1_au *lau = au;

	if (!ctx->vc1) {
		/* decoder not yet initialized */
		VC1DecMetaData metadata;

		memset(&metadata, 0, sizeof(metadata));
		if (ctx->streaminfo.streamformat == V4L2_PIX_FMT_VC1_ANNEX_G) {
			/* VC1 Advanced Profile
			 * the G1 decoder expects Sequence Header (SH) and
			 * Entry Point Header (EPH) information packed in
			 * 2 header AUs
			 */
			dev_dbg(g1->dev, "%s : %s : Advanced profile\n",
				pctx->name, __func__);
			ctx->profile = VC1_METADATA_ADVANCED_PROFILE;

			/* FIXME profile not in spec range  :
			 * "Valid values are 0, 4 and 12 for simple, main
			 * and advanced profile respectively. This is the
			 * only variable that is set for advanced profile
			 * video bit stream."
			 */
			metadata.profile = 8;
			if (g1_vc1_init(pctx, &metadata))
				return -EIO;

		} else {
			/* VC1 Main or Simple Profile
			 * the G1 decoder expects STRUCT_C(Sc) information
			 * packed as shown below (4-bytes length):
			 * Sc0 Sc1 Sc2 Sc3
			 */
			dev_dbg(g1->dev, "%s : Main/Simple profile\n",
				pctx->name);
			ctx->profile = (*((__u8 *)au->vaddr) >> 4) & 0x0F;
			dev_dbg(g1->dev, "width: %d, height: %d\n",
				ctx->streaminfo.width, ctx->streaminfo.height);
			metadata.maxCodedWidth = ctx->streaminfo.width;
			metadata.maxCodedHeight = ctx->streaminfo.height;
			if ((metadata.maxCodedWidth == 0) ||
			    (metadata.maxCodedHeight == 0) ||
			    (metadata.maxCodedWidth * metadata.maxCodedHeight >
			     G1_MAX_RESO)) {
				dev_err(g1->dev,
					"%s  invalid decoder output resolution: (%dx%d) is 0 or > %d pixels budget\n",
					pctx->name, metadata.maxCodedWidth,
					metadata.maxCodedHeight,
					G1_MAX_RESO);
				return -EINVAL;
			}
			vc1ret = VC1DecUnpackMetaData((__u8 *)au->vaddr, 4,
						      &metadata);
			/* check metadata unpacking status */
			ret = to_ret(vc1ret);
			if (ret) {
				dev_err(g1->dev, "%s VC1DecUnpackMetaData error %s(%d)\n",
					pctx->name,
					vc1ret_str(vc1ret),
					vc1ret);
				return ret;
			}
			if (g1_vc1_init(pctx, &metadata))
				return -EIO;
			/* no need to decode the first AU,
			 * we just need to retrieve codec info from the Decoder
			 * so we directly jump to header treatment
			 */
			vc1ret = VC1DEC_HDRS_RDY;
			goto hdrs_ready;
		}
	}

	/* decode this access unit */
	if (ctx->state >= G1_VC1_HEADER_PARSED) {
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

	if (ctx->profile == VC1_METADATA_ADVANCED_PROFILE &&
	    ctx->state >= G1_VC1_HEADER_PARSED) {
		/* as soon as Header(s) has(ve) been detected,
		 * we systematically check if a start code (SC) is
		 * available at the beginning of the AU.
		 * If not, we add it
		 */
		int SC_pos = 0;

		if (g1_vc1_get_next_start_code(au, 0, 4, &SC_pos) ==
			SC_NOT_FOUND) {
			dev_dbg(g1->dev,
				"%s Start Code not found, so we add it",
				pctx->name);

			if (!ctx->local_au) {
				ctx->local_au =
					kzalloc(sizeof(*ctx->local_au),
						GFP_KERNEL);
				if (!ctx->local_au)
					return -ENOMEM;
			}
			if (!ctx->local_buf) {
				ctx->local_buf =
					kzalloc(sizeof(*ctx->local_buf),
						GFP_KERNEL);
				if (!ctx->local_buf)
					return -ENOMEM;
			}
			if (!ctx->local_buf->vaddr) {
				ret = hw_alloc(pctx, pctx->max_au_size +
					       VC1_ADVANCED_SC_SIZE,
					       "VC1 local access unit",
					       ctx->local_buf);
				if (ret)
					return -ENOMEM;

				/* start code copy */
				memcpy(ctx->local_buf->vaddr,
				       &AdvancedStartCode,
				       VC1_ADVANCED_SC_SIZE);
			}
			ctx->local_au->vaddr = ctx->local_buf->vaddr;
			ctx->local_au->paddr = ctx->local_buf->paddr;
			ctx->local_au->size = au->size + VC1_ADVANCED_SC_SIZE;
			ctx->local_au->picid = au->picid;
			/* au copy */
			memcpy(ctx->local_buf->vaddr + VC1_ADVANCED_SC_SIZE,
			       au->vaddr, au->size);
			lau = ctx->local_au;
		}
	}
	memset(&in, 0, sizeof(in));
	in.pStream = lau->vaddr;
	in.streamBusAddress = lau->paddr;
	in.streamSize = lau->size;
	in.picId = lau->picid;
	memset(&out, 0, sizeof(out));

retry:
	vc1ret = VC1DecDecode(ctx->vc1, &in, &out);
	/* check decode status */
	ret = to_ret(vc1ret);
	if (ret) {
		dev_err(g1->dev, "%s  VC1DecDecode error %s(%d)", pctx->name,
			vc1ret_str(vc1ret), vc1ret);
		pctx->decode_errors++;

		return ret;
	}

	if (is_stream_error(vc1ret)) {
		dev_warn_ratelimited(g1->dev,
				     "%s  stream error @ frame %d, au size=%d (%s)\n",
				     pctx->name, pctx->decoded_frames,
				     au->size,
				     (first_header_received ?
				      "during headers detection" : ""));
		pctx->stream_errors++;
		return 0;
	}

hdrs_ready:
	if (vc1ret == VC1DEC_HDRS_RDY) {
		if ((ctx->state < G1_VC1_HEADER_PARSED) &&
		    (ctx->profile == VC1_METADATA_ADVANCED_PROFILE) &&
		    (!first_header_received)) {
			/* in G1 stack, there is no way to know which header
			 * has been detected, we assume that first HDRS_READY
			 * means that 'sequence layer' has been detected and
			 * next HDRS_READY means ' entry-point layer'
			 * Headers are supposed to be included in the first
			 * AU ( Sequence Layer then Entry-point Layer )
			 * So after reception of the first HDRS_READY, to
			 * ensure that all headers are processed by the stack,
			 * the first AU is sent back to the decoder after
			 * removing four first data (suppression of the start
			 * code of the first header).
			 *
			 * FIXME : could be done by parsing first AU and
			 * trying to find headers. Next step...
			 */
			if (in.streamSize > VC1_ADVANCED_SC_SIZE) {
				/* skip start code */
				in.pStream += VC1_ADVANCED_SC_SIZE;
				in.streamBusAddress += VC1_ADVANCED_SC_SIZE;
				in.streamSize -= VC1_ADVANCED_SC_SIZE;
			} else {
				dev_err(g1->dev, "%s Unable to find second header, not enought data in the AU",
					pctx->name);
				return -EINVAL;
			}
			first_header_received = true;
			/* restart to decode the AU ( minus the first SC) */
			goto retry;
		}

		/* headers detected */
		/* get new stream infos */
		vc1ret = VC1DecGetInfo(ctx->vc1, &decinfo);
		if (vc1ret) {
			dev_err(g1->dev, "%s  VC1DecGetInfo error (%d)\n",
				pctx->name, vc1ret);
			return -EIO;
		}
		/* new live header */
		if (ctx->state >= G1_VC1_HEADER_PARSED) {
			/* header already detected previously */
			dev_info(g1->dev,
				 "%s  new header detection while decoding\n",
				 pctx->name);
			return -ENODATA;
		}
		/* first header, store infos */
		ret = g1_vc1_set_streaminfo(pctx, &decinfo);
		if (ret)
			return ret;

		/* store decoder output frame infos */
		ret = g1_vc1_set_dec_frameinfo(pctx, &decinfo);
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

		ctx->state = G1_VC1_HEADER_PARSED;
		return -ENODATA;
	}
	switch (vc1ret) {
	case VC1DEC_NONREF_PIC_SKIPPED:
	case VC1DEC_STRM_PROCESSED:
	case VC1DEC_END_OF_SEQ:
	case VC1DEC_RESOLUTION_CHANGED:
	case VC1DEC_OK:
		/* output (decoded data) not expected so return nodata */
		return -ENODATA;
	case VC1DEC_PIC_DECODED:
		/* frame decoded */
		pctx->decoded_frames++;
		return 0;
	default:
		/* no error detected at this point but we didn't treat
		 * the returned value ... Strange !!!
		 * log a trace to investigate if needed
		 */
		dev_err_ratelimited(g1->dev,
				    "%s - Untreated decode returned value : %s(%d)\n",
				    pctx->name, vc1ret_str(vc1ret), vc1ret);
		return -EIO;
	}
}

static int g1_vc1_get_frame(struct g1_ctx *pctx, struct g1_frame **frame)
{
	struct g1_dev *g1 = pctx->dev;
	struct g1_vc1_ctx *ctx = to_ctx(pctx);
	int ret;
	VC1DecRet vc1ret;
	VC1DecPicture picture;
	u32 pictureId = 0;

	/* systematically check if we need to allocate a free frame
	 * to the PP.
	 * mainly useful in case of drain to store the last reference
	 * frame ( scheduled by g1_decoder_stop_cmd )
	 */
	if (ctx->state >= G1_VC1_HEADER_PARSED) {
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

	vc1ret = VC1DecNextPicture(ctx->vc1, &picture, ctx->eos);
	ret = to_ret(vc1ret);
	if (ret) {
		dev_err(g1->dev, "%s  VC1DecNextPicture error %s(%d)\n",
			pctx->name, vc1ret_str(vc1ret), vc1ret);
		pctx->decode_errors++;
		return ret;
	}

	if ((vc1ret != VC1DEC_PIC_RDY) && (vc1ret != VC1DEC_PIC_DECODED)) {
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

	if (ctx->state == G1_VC1_FLUSHING) {
		/* Seek case : In flushing state, while there is no
		 * decoded key frame, don't display anything as we're
		 * not sure frames are sane.
		 */
		if (picture.keyPicture) {
			dev_dbg(g1->dev,
				"FLUSHING STATE : I frame received\n");
			ctx->state = G1_VC1_HEADER_PARSED;
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

static void g1_vc1_recycle(struct g1_ctx *pctx, struct g1_frame *frame)
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

static void g1_vc1_flush(struct g1_ctx *pctx)
{
	struct g1_vc1_ctx *ctx = to_ctx(pctx);

	ctx->state = G1_VC1_FLUSHING;
}

static void g1_vc1_drain(struct g1_ctx *pctx)
{
	struct g1_vc1_ctx *ctx = to_ctx(pctx);

	ctx->eos = 1;
}

/* VC1 decoder can decode SMPTE 421M Annex G compliant stream (relative to the
 * Advanced Profile) and SMPTE 421M Annex L compliant stream (relative to the
 * Simple or Main profile). Add below one decoder structure per supported
 * format
 */
const __u32 vc1_stream_formats[] = {
	V4L2_PIX_FMT_VC1_ANNEX_L,
	V4L2_PIX_FMT_VC1_ANNEX_G,
};

/* bitstream conformant to the Simple or Main profile */
const struct g1_dec vc1dec = {
	.name = "vc1",
	.streamformat = &vc1_stream_formats[0],
	.nb_streams = ARRAY_SIZE(vc1_stream_formats),
	.pixelformat = &pp_output_formats[0],
	.nb_pixels = ARRAY_SIZE(pp_output_formats),
	.probe = g1_vc1_probe,
	.open = g1_vc1_open,
	.close = g1_vc1_close,
	.get_streaminfo = g1_vc1_get_streaminfo,
	.set_streaminfo = NULL,
	.get_frameinfo = g1_vc1_get_frameinfo,
	.set_frameinfo = g1_vc1_set_frameinfo,
	.decode = g1_vc1_decode,
	.setup_frame = g1_setup_frame,
	.get_frame = g1_vc1_get_frame,
	.recycle = g1_vc1_recycle,
	.flush = g1_vc1_flush,
	.drain = g1_vc1_drain,
};
