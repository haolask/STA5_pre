/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Yannick Fertre <yannick.fertre@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */
#include "pp-ctrl.h"
#include "ppapi.h"
#include "dwl.h"

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

static inline u32 to_pp_format(u32 streamformat)
{
	switch (streamformat) {
	case PF_FORMAT_NV12:
		return PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR;
	case PF_FORMAT_YUYV:
		return PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED;
	case PF_FORMAT_YVYU:
		return PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED;
	case PF_FORMAT_UYVY:
		return PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED;
	case PF_RGB565:
		return PP_PIX_FMT_RGB16_5_6_5;
	case PF_ARGB8888:
	case PF_RGBA8888:
		return PP_PIX_FMT_BGR32;
	case PF_ARGB1555:
		return PP_PIX_FMT_RGB16_5_5_5;
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

/**
 * struct pp_frame_addr - Y/Cb/Cr  start address structure
 * @y:	 luminance plane  address
 * @cb:	 Cb plane address
 * @cr:	 Cr plane address
 */
struct g1_pp_frame_addr {
	u32 y;
	u32 cb;
	u32 cr;
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
	case PF_FORMAT_NV12:
		/* decompose Y into Y/Cb */
		paddr->cb = (u32)(paddr->y + pix_size);
		paddr->cr = 0;
		break;
	default:
		paddr->cb = 0;
		paddr->cr = 0;
		break;
	}
}

void* g1_pp_open(void * g1_ctx)
{
	PPInst pp;
	PPResult ppret;

	if (!g1_ctx) {
		TRACE_ERR("%s NULL context error", __func__);
		return NULL;
	}

	ppret = PPInit(&pp, g1_ctx);
	if (ppret) {
		TRACE_ERR("PPInit error %s(%d)",
			ppret_str(ppret), ppret);
		return NULL;
	}

	return (void*)pp;
}

int g1_pp_close(void * inst)
{
	PPInst pp = (PPInst)inst;

	if (pp) {
		PPRelease(pp);
		pp = NULL;
	}
	return 0;
}

int g1_pp_set_config(void * inst, struct user_ppconfig * usr_ppconfig)
{
	struct g1_pp_frame_addr out_addr;
	struct g1_pp_frame_addr in_addr;
	PPInst pp = (PPInst)inst;
	PPResult ppret;
	PPConfig ppconfig;

	memset(&ppconfig, 0, sizeof(ppconfig));
	ppret = PPGetConfig(pp, &ppconfig);

	/* set output image */
	ppconfig.ppOutImg.pixFormat = to_pp_format(usr_ppconfig->output_fmt);
	ppconfig.ppOutImg.width = usr_ppconfig->output_wdt;
	ppconfig.ppOutImg.height = usr_ppconfig->output_hgt;

	g1_pp_prepare_addr(&out_addr, usr_ppconfig->output_paddr,
			   usr_ppconfig->output_fmt,
			   ppconfig.ppOutImg.width, ppconfig.ppOutImg.height);
	ppconfig.ppOutImg.bufferBusAddr = out_addr.y;
	ppconfig.ppOutImg.bufferChromaBusAddr = out_addr.cb;

	/* set input image */
	ppconfig.ppInImg.pixFormat =
		to_pp_format(usr_ppconfig->input_fmt);
	ppconfig.ppInImg.picStruct = PP_PIC_FRAME_OR_TOP_FIELD;
	if (usr_ppconfig->deinterlace_needed)
		ppconfig.ppInImg.picStruct = PP_PIC_TOP_AND_BOT_FIELD_FRAME;

	ppconfig.ppInImg.width = usr_ppconfig->input_wdt;
	ppconfig.ppInImg.height = usr_ppconfig->input_hgt;

	ppconfig.ppInRotation.rotation = PP_ROTATION_NONE;

	g1_pp_prepare_addr(&in_addr, usr_ppconfig->input_paddr,
				usr_ppconfig->input_fmt,
				usr_ppconfig->input_wdt,
				usr_ppconfig->input_hgt);

	ppconfig.ppInImg.bufferBusAddr = in_addr.y;
	ppconfig.ppInImg.bufferCbBusAddr = in_addr.cb;
	ppconfig.ppInImg.bufferCrBusAddr = in_addr.cr;

	if (ppconfig.ppInImg.picStruct ==
		PP_PIC_TOP_AND_BOT_FIELD_FRAME) {
		ppconfig.ppInImg.bufferBusAddrBot =
			in_addr.y + (usr_ppconfig->input_wdt);
		ppconfig.ppInImg.bufferBusAddrChBot =
			in_addr.cb + (usr_ppconfig->input_wdt);
	}

	/* set the dynamic range of input pixel samples */
	ppconfig.ppInImg.videoRange = 0;

	/* check if we need to deinterlace the stream */
	if (usr_ppconfig->deinterlace_needed)
		ppconfig.ppOutDeinterlace.enable = 1;

	//TRACE_INFO("PPSetConfig (%s)\n",
	//	pp_config_str(&ppconfig, str, sizeof(str)));

	ppret = PPSetConfig(pp, &ppconfig);
	if (ppret != PP_OK) {
		TRACE_ERR("PPSetConfig error %s(%d)\n",
			ppret_str(ppret), ppret);
		//	pp_config_str(&ppconfig, str, sizeof(str)));
		return -1;
	}
	return 0;
}

int g1_pp_get_frame(void * inst, u32 ** bufferBusAddr)
{
	PPInst pp = (PPInst)inst;
	PPOutput ppOut;
	PPResult ppret;

	ppret = PPGetResult(pp);
	if (ppret != PP_OK) {
		TRACE_ERR("PPGetResult error %s(%d)\n",
			ppret_str(ppret), ppret);
		return -1;
	}

	ppret = PPGetNextOutput(pp, &ppOut);
	if (ppret != PP_OK) {
		TRACE_ERR("%s PPGetNextOutput error %s(%d)\n",
			ppret_str(ppret), ppret);
		return -1;
	}

	*bufferBusAddr = (u32*)ppOut.bufferBusAddr;
	return 0;
}
