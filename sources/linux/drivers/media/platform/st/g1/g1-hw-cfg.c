/*
 * Copyright (C) STMicroelectronics SA 2015
 * Author: Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include "g1.h"
#include "stack/inc/decapicommon.h"

#define G1_MPEG2_E         31	/* 1 bit */
#define G1_VC1_E           29	/* 2 bits */
#define G1_JPEG_E          28	/* 1 bit */
#define G1_MPEG4_E         26	/* 2 bits */
#define G1_H264_E          24	/* 2 bits */
#define G1_VP6_E           23	/* 1 bit */
#define G1_PJPEG_E         22	/* 1 bit */
#define G1_REF_BUFF_E      20	/* 1 bit */

#define G1_JPEG_EXT_E          31	/* 1 bit */
#define G1_REF_BUFF_ILACE_E    30	/* 1 bit */
#define G1_MPEG4_CUSTOM_E      29	/* 1 bit */
#define G1_REF_BUFF_DOUBLE_E   28	/* 1 bit */
#define G1_RV_E            26	/* 2 bits */
#define G1_VP7_E           24	/* 1 bit */
#define G1_VP8_E           23	/* 1 bit */
#define G1_AVS_E           22	/* 1 bit */
#define G1_MVC_E           20	/* 2 bits */
#define G1_WEBP_E          19	/* 1 bit */
#define G1_DEC_TILED_L     17	/* 2 bits */
#define G1_DEC_PIC_W_EXT   14	/* 2 bits */
#define G1_EC_E            12	/* 2 bits */
#define G1_STRIDE_E        11	/* 1 bit */
#define G1_FIELD_DPB_E     10	/* 1 bit */

#define G1_CFG_E           24	/* 4 bits */
#define G1_PP_E            16	/* 1 bit */
#define G1_PP_IN_TILED_L   14	/* 2 bits */

#define G1_SORENSONSPARK_E 11	/* 1 bit */

#define G1_H264_FUSE_E          31	/* 1 bit */
#define G1_MPEG4_FUSE_E         30	/* 1 bit */
#define G1_MPEG2_FUSE_E         29	/* 1 bit */
#define G1_SORENSONSPARK_FUSE_E 28	/* 1 bit */
#define G1_JPEG_FUSE_E          27	/* 1 bit */
#define G1_VP6_FUSE_E           26	/* 1 bit */
#define G1_VC1_FUSE_E           25	/* 1 bit */
#define G1_PJPEG_FUSE_E         24	/* 1 bit */
#define G1_CUSTOM_MPEG4_FUSE_E  23	/* 1 bit */
#define G1_RV_FUSE_E            22	/* 1 bit */
#define G1_VP7_FUSE_E           21	/* 1 bit */
#define G1_VP8_FUSE_E           20	/* 1 bit */
#define G1_AVS_FUSE_E           19	/* 1 bit */
#define G1_MVC_FUSE_E           18	/* 1 bit */

#define G1_DEC_MAX_1920_FUSE_E  15	/* 1 bit */
#define G1_DEC_MAX_1280_FUSE_E  14	/* 1 bit */
#define G1_DEC_MAX_720_FUSE_E   13	/* 1 bit */
#define G1_DEC_MAX_352_FUSE_E   12	/* 1 bit */
#define G1_REF_BUFF_FUSE_E       7	/* 1 bit */

#define G1_PP_FUSE_E				31	/* 1 bit */
#define G1_PP_DEINTERLACE_FUSE_E   30	/* 1 bit */
#define G1_PP_ALPHA_BLEND_FUSE_E   29	/* 1 bit */
#define G1_PP_MAX_4096_FUSE_E		16	/* 1 bit */
#define G1_PP_MAX_1920_FUSE_E		15	/* 1 bit */
#define G1_PP_MAX_1280_FUSE_E		14	/* 1 bit */
#define G1_PP_MAX_720_FUSE_E		13	/* 1 bit */
#define G1_PP_MAX_352_FUSE_E		12	/* 1 bit */

#define to_hw(g1) (&(g1)->hw)

u32 g1_hw_cfg_get_asic_id(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);

	return hw->asic_id;
};

void g1_hw_cfg_get_config(struct g1_dev *g1, struct g1_hw_config *config)
{
	struct g1_hw *hw = to_hw(g1);

	*config = hw->config;
};

void g1_hw_cfg_get_fuse(struct g1_dev *g1, struct g1_hw_fuse *fuse)
{
	struct g1_hw *hw = to_hw(g1);

	*fuse = hw->fuse;
};

void g1_hw_cfg_build_fuse(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);
	struct g1_hw_fuse *fuse = &hw->fuse;
	u32 *dec_fuse = &hw->dec_fuse;
	u32 *pp_cfg = &hw->pp_cfg;
	u32 *pp_fuse = &hw->pp_fuse;

	dev_dbg(g1->dev, "     > %s\n", __func__);

	memset(fuse, 0, sizeof(*fuse));

	/* Decoder fuse configuration */
	fuse->h264_support_fuse = (*dec_fuse >> G1_H264_FUSE_E) & 0x01U;
	fuse->mpeg4_support_fuse = (*dec_fuse >> G1_MPEG4_FUSE_E) & 0x01U;
	fuse->mpeg2_support_fuse = (*dec_fuse >> G1_MPEG2_FUSE_E) & 0x01U;
	fuse->sorenson_spark_support_fuse =
	    (*dec_fuse >> G1_SORENSONSPARK_FUSE_E) & 0x01U;
	fuse->jpeg_support_fuse = (*dec_fuse >> G1_JPEG_FUSE_E) & 0x01U;
	fuse->vp6_support_fuse = (*dec_fuse >> G1_VP6_FUSE_E) & 0x01U;
	fuse->vc1_support_fuse = (*dec_fuse >> G1_VC1_FUSE_E) & 0x01U;
	fuse->jpeg_prog_support_fuse = (*dec_fuse >> G1_PJPEG_FUSE_E) & 0x01U;
	fuse->rv_support_fuse = (*dec_fuse >> G1_RV_FUSE_E) & 0x01U;
	fuse->avs_support_fuse = (*dec_fuse >> G1_AVS_FUSE_E) & 0x01U;
	fuse->vp7_support_fuse = (*dec_fuse >> G1_VP7_FUSE_E) & 0x01U;
	fuse->vp8_support_fuse = (*dec_fuse >> G1_VP8_FUSE_E) & 0x01U;
	fuse->custom_mpeg4_support_fuse =
	    (*dec_fuse >> G1_CUSTOM_MPEG4_FUSE_E) & 0x01U;
	fuse->mvc_support_fuse = (*dec_fuse >> G1_MVC_FUSE_E) & 0x01U;

	/* check max. decoder output width */
	if (*dec_fuse & 0x10000U)
		fuse->max_dec_pic_width_fuse = 4096;
	else if (*dec_fuse & 0x8000U)
		fuse->max_dec_pic_width_fuse = 1920;
	else if (*dec_fuse & 0x4000U)
		fuse->max_dec_pic_width_fuse = 1280;
	else if (*dec_fuse & 0x2000U)
		fuse->max_dec_pic_width_fuse = 720;
	else if (*dec_fuse & 0x1000U)
		fuse->max_dec_pic_width_fuse = 352;

	fuse->ref_buf_support_fuse = (*dec_fuse >> G1_REF_BUFF_FUSE_E) & 0x01U;

	if ((*pp_cfg >> G1_PP_E) & 0x01U) {
		/* Pp fuse configuration */
		if ((*pp_fuse >> G1_PP_FUSE_E) & 0x01U) {
			fuse->pp_support_fuse = 1;

			/* check max. pp output width */
			if (*pp_fuse & 0x10000U)
				fuse->max_pp_out_pic_width_fuse = 4096;
			else if (*pp_fuse & 0x8000U)
				fuse->max_pp_out_pic_width_fuse = 1920;
			else if (*pp_fuse & 0x4000U)
				fuse->max_pp_out_pic_width_fuse = 1280;
			else if (*pp_fuse & 0x2000U)
				fuse->max_pp_out_pic_width_fuse = 720;
			else if (*pp_fuse & 0x1000U)
				fuse->max_pp_out_pic_width_fuse = 352;

			fuse->pp_config_fuse = *pp_fuse;
		} else {
			fuse->pp_support_fuse = 0;
			fuse->max_pp_out_pic_width_fuse = 0;
			fuse->pp_config_fuse = 0;
		}
	}

	dev_dbg(g1->dev, "     < %s\n", __func__);
}

void g1_hw_cfg_build_config(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);
	struct g1_hw_fuse *fuse = &hw->fuse;
	struct g1_hw_config *config = &hw->config;
	u32 *dec_cfg = &hw->dec_cfg;
	u32 *dec_cfg2 = &hw->dec_cfg2;
	u32 *pp_cfg = &hw->pp_cfg;
	u32 asic_id = hw->asic_id;

	dev_dbg(g1->dev, "     > %s\n", __func__);

	memset(config, 0, sizeof(*config));

	config->h264_support = (*dec_cfg >> G1_H264_E) & 0x3U;
	/* check jpeg */
	config->jpeg_support = (*dec_cfg >> G1_JPEG_E) & 0x01U;
	if (config->jpeg_support && ((*dec_cfg >> G1_PJPEG_E) & 0x01U))
		config->jpeg_support = JPEG_PROGRESSIVE;
	config->mpeg4_support = (*dec_cfg >> G1_MPEG4_E) & 0x3U;
	config->vc1_support = (*dec_cfg >> G1_VC1_E) & 0x3U;
	config->mpeg2_support = (*dec_cfg >> G1_MPEG2_E) & 0x01U;
	config->sorenson_spark_support =
		(*dec_cfg >> G1_SORENSONSPARK_E) & 0x01U;
	config->ref_buf_support = (*dec_cfg >> G1_REF_BUFF_E) & 0x01U;
	config->vp6_support = (*dec_cfg >> G1_VP6_E) & 0x01U;
#ifdef DEC_X170_APF_DISABLE
	if (DEC_X170_APF_DISABLE)
		config->tiled_mode_support = 0;
#endif /* DEC_X170_APF_DISABLE */

	config->max_dec_pic_width = *dec_cfg & 0x07FFU;

	if (config->ref_buf_support) {
		if ((*dec_cfg2 >> G1_REF_BUFF_ILACE_E) & 0x01U)
			config->ref_buf_support |= 2;
		if ((*dec_cfg2 >> G1_REF_BUFF_DOUBLE_E) & 0x01U)
			config->ref_buf_support |= 4;
	}

	config->custom_mpeg4_support = (*dec_cfg2 >> G1_MPEG4_CUSTOM_E) & 0x01U;
	config->vp7_support = (*dec_cfg2 >> G1_VP7_E) & 0x01U;
	config->vp8_support = (*dec_cfg2 >> G1_VP8_E) & 0x01U;
	config->avs_support = (*dec_cfg2 >> G1_AVS_E) & 0x01U;

	/* JPEG extensions */
	if (((asic_id >> 16) >= 0x8190U) || ((asic_id >> 16) == 0x6731U))
		config->jpeg_e_support = (*dec_cfg2 >> G1_JPEG_EXT_E) & 0x01U;
	else
		config->jpeg_e_support = JPEG_EXT_NOT_SUPPORTED;

	if (((asic_id >> 16) >= 0x9170U) || ((asic_id >> 16) == 0x6731U))
		config->rv_support = (*dec_cfg2 >> G1_RV_E) & 0x03U;
	else
		config->rv_support = RV_NOT_SUPPORTED;

	config->mvc_support = (*dec_cfg2 >> G1_MVC_E) & 0x03U;
	config->webp_support = (*dec_cfg2 >> G1_WEBP_E) & 0x01U;
	config->tiled_mode_support = (*dec_cfg2 >> G1_DEC_TILED_L) & 0x03U;
	config->max_dec_pic_width +=
	    ((*dec_cfg2 >> G1_DEC_PIC_W_EXT) & 0x03U) << 11;

	config->ec_support = (*dec_cfg2 >> G1_EC_E) & 0x03U;
	config->stride_support = (*dec_cfg2 >> G1_STRIDE_E) & 0x01U;

	if (config->ref_buf_support && (asic_id >> 16) == 0x6731U)
		config->ref_buf_support |= 8;/* enable HW support for offset */

	if ((*pp_cfg >> G1_PP_E) & 0x01U) {
		config->pp_support = 1;
		/* Theoretical max range 0...65535; actual 48...4096 */
		config->max_pp_out_pic_width = *pp_cfg & 0xFFFFU;
		config->pp_config = *pp_cfg;
	} else {
		config->pp_support = 0;
		config->max_pp_out_pic_width = 0;
		config->pp_config = 0;
	}

	/* check the HW version */
	if (((asic_id >> 16) >= 0x8190U) || ((asic_id >> 16) == 0x6731U)) {
		u32 de_interlace;
		u32 alpha_blend;
		u32 de_interlace_fuse;
		u32 alpha_blend_fuse;

		/* Maximum decoding width supported by the HW */
		if (config->max_dec_pic_width > fuse->max_dec_pic_width_fuse)
			config->max_dec_pic_width =
				fuse->max_dec_pic_width_fuse;
		/* Maximum output width of Post-Processor */
		if (config->max_pp_out_pic_width >
		    fuse->max_pp_out_pic_width_fuse)
			config->max_pp_out_pic_width =
				fuse->max_pp_out_pic_width_fuse;
		/* h264 */
		if (!fuse->h264_support_fuse)
			config->h264_support = H264_NOT_SUPPORTED;
		/* mpeg-4 */
		if (!fuse->mpeg4_support_fuse)
			config->mpeg4_support = MPEG4_NOT_SUPPORTED;
		/* custom mpeg-4 */
		if (!fuse->custom_mpeg4_support_fuse)
			config->custom_mpeg4_support =
				MPEG4_CUSTOM_NOT_SUPPORTED;
		/* jpeg (baseline && progressive) */
		if (!fuse->jpeg_support_fuse)
			config->jpeg_support = JPEG_NOT_SUPPORTED;
		if ((config->jpeg_support == JPEG_PROGRESSIVE) &&
		    !fuse->jpeg_prog_support_fuse)
			config->jpeg_support = JPEG_BASELINE;
		/* mpeg-2 */
		if (!fuse->mpeg2_support_fuse)
			config->mpeg2_support = MPEG2_NOT_SUPPORTED;
		/* vc-1 */
		if (!fuse->vc1_support_fuse)
			config->vc1_support = VC1_NOT_SUPPORTED;
		/* vp6 */
		if (!fuse->vp6_support_fuse)
			config->vp6_support = VP6_NOT_SUPPORTED;
		/* vp7 */
		if (!fuse->vp7_support_fuse)
			config->vp7_support = VP7_NOT_SUPPORTED;
		/* vp8 */
		if (!fuse->vp8_support_fuse)
			config->vp8_support = VP8_NOT_SUPPORTED;
		/* webp */
		if (!fuse->vp8_support_fuse)
			config->webp_support = WEBP_NOT_SUPPORTED;
		/* pp */
		if (!fuse->pp_support_fuse)
			config->pp_support = PP_NOT_SUPPORTED;
		/* check the pp config vs fuse status */
		if ((config->pp_config & 0xFC000000) &&
		    ((fuse->pp_config_fuse & 0xF0000000) >> 5)) {
			/* config */
			de_interlace =
			  ((config->pp_config & PP_DEINTERLACING) >> 25);
			alpha_blend =
			  ((config->pp_config & PP_ALPHA_BLENDING) >> 24);
			/* fuse */
			de_interlace_fuse =
			  (((fuse->pp_config_fuse >> 5) & PP_DEINTERLACING) >>
			   25);
			alpha_blend_fuse =
			  (((fuse->pp_config_fuse >> 5) & PP_ALPHA_BLENDING) >>
			   24);

			/* check if */
			if (de_interlace && !de_interlace_fuse)
				config->pp_config &= 0xFD000000;
			if (alpha_blend && !alpha_blend_fuse)
				config->pp_config &= 0xFE000000;
		}
		/* sorenson */
		if (!fuse->sorenson_spark_support_fuse)
			config->sorenson_spark_support =
			    SORENSON_SPARK_NOT_SUPPORTED;
		/* ref. picture buffer */
		if (!fuse->ref_buf_support_fuse)
			config->ref_buf_support = REF_BUF_NOT_SUPPORTED;

		/* rv */
		if (!fuse->rv_support_fuse)
			config->rv_support = RV_NOT_SUPPORTED;
		/* avs */
		if (!fuse->avs_support_fuse)
			config->avs_support = AVS_NOT_SUPPORTED;
		/* mvc */
		if (!fuse->mvc_support_fuse)
			config->mvc_support = MVC_NOT_SUPPORTED;
	}

	dev_dbg(g1->dev, "     < %s\n", __func__);
}

unsigned char *g1_hw_cfg_summary_str(struct g1_dev *g1,
				     unsigned char *str, unsigned int len)
{
	struct g1_hw *hw = to_hw(g1);
	struct g1_hw_config *config = &hw->config;

	snprintf(str, len,
		 "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s supported up to %d, %s",
		 config->h264_support ? "h264/" : "",
		 config->h264_support && config->mvc_support ? "h264-mvc/" : "",
		 config->vc1_support ? "vc1/" : "",
		 config->mpeg4_support ? "mpeg4/" : "",
		 config->sorenson_spark_support ? "sorenson-spark/" : "",
		 config->mpeg2_support ? "mpeg2/" : "",
		 config->jpeg_support ? "jpeg/" : "",
		 config->jpeg_support && config->jpeg_prog_support ?
			"progressive-jpeg/" : "",
		 config->vp6_support ? "vp6/" : "",
		 config->vp7_support ? "vp7/" : "",
		 config->vp8_support ? "vp8/" : "",
		 config->webp_support ? "webp/" : "",
		 config->avs_support ? "avs/" : "",
		 config->avs_support && config->avs_plus_support ? "avs+/" : "",
		 config->rv_support ? "real-video/" : "",
		 config->max_dec_pic_width,
		 config->pp_support ? "post-processor support" :
			"no post-processor support");

	return str;
}

unsigned char *g1_hw_cfg_dump_str(struct g1_dev *g1,
				  unsigned char *pstr, unsigned int len)
{
	struct g1_hw *hw = to_hw(g1);
	u32 *dec_cfg = &hw->dec_cfg;
	u32 *dec_cfg2 = &hw->dec_cfg2;
	u32 *pp_cfg = &hw->pp_cfg;
	u32 *dec_fuse = &hw->dec_fuse;
	u32 *pp_fuse = &hw->pp_fuse;
	char *cur = pstr;
	size_t left = len;
	int cnt = 0;
	int ret = 0;
	unsigned char str[200] = "";

	ret = snprintf(cur, left,
		       "[hardware config]\n"
		       "|-[sum up]\n"
		       "| |- %s\n"
		       "|\n"
		       "|-[asic id]\n"
		       "| |-id=0x%x\n"
		       "|\n",
		       g1_hw_cfg_summary_str(g1, str, sizeof(str)),
		       hw->asic_id);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf
		(cur, left,
		 "|-[decoder]\n"
		 "| |-[synth cfg]\n"
		 "| | 0x%08x\n"
		 "| |   |||||\\ /  bit(s)  description       support/value\n"
		 "| |   ||||| |\n"
		 "| |   ||||| \\____10..0  max width         %d\n"
		 "| |   |||||   \\_____11  sorenson/spark    %s\n"
		 "| |   |||| \\____15..12  -                 -\n"
		 "| |   ||| \\_____19..16  -                 -\n"
		 "| |   || \\__________20  ref buffer        %s\n"
		 "| |   ||      \\_____21  -                 -\n"
		 "| |   ||      \\_____22  progressive jpeg  %s\n"
		 "| |   ||      \\_____23  vp6               %s\n"
		 "| |   | \\_______25..24  h264              %s\n"
		 "| |   |       \\_27..26  mpeg4             %s\n"
		 "| |    \\____________28  jpeg              %s\n"
		 "| |           \\_30..29  vc1               %s\n"
		 "| |           \\_____31  mpeg2             %s\n"
		 "| |\n",
		 *dec_cfg,
		 *dec_cfg & 0x07FFU,
		 (*dec_cfg >> G1_SORENSONSPARK_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg >> G1_REF_BUFF_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg >> G1_PJPEG_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg >> G1_VP6_E) & 0x01U ? "[yes]" : "[no]",
		 ((*dec_cfg >> G1_H264_E) & 0x3U) == H264_HIGH_PROFILE ?
			"[yes] up to high profile" :
		 ((*dec_cfg >> G1_H264_E) & 0x3U) == H264_MAIN_PROFILE ?
			"[yes] up to main profile" :
		 ((*dec_cfg >> G1_H264_E) & 0x3U) == H264_BASELINE_PROFILE ?
			"[yes] up to baseline profile" : "[no]",
		 ((*dec_cfg >> G1_MPEG4_E) & 0x3U) ==
			 MPEG4_ADVANCED_SIMPLE_PROFILE ?
			"[yes] up to advanced simple profile" :
		 ((*dec_cfg >> G1_MPEG4_E) & 0x3U) == MPEG4_SIMPLE_PROFILE ?
			"[yes] up to simple profile" : "[no]",
		 (*dec_cfg >> G1_JPEG_E) & 0x01U ? "[yes]" : "[no]",
		 ((*dec_cfg >> G1_VC1_E) & 0x3U) == VC1_ADVANCED_PROFILE ?
			"[yes] up to advanced profile" :
		 ((*dec_cfg >> G1_VC1_E) & 0x3U) == VC1_MAIN_PROFILE ?
			"[yes] up to main profile" :
		 ((*dec_cfg >> G1_VC1_E) & 0x3U) == VC1_SIMPLE_PROFILE ?
			"[yes] up to simple profile" : "[no]",
		 (*dec_cfg >> G1_MPEG2_E) & 0x01U ? "[yes]" : "[no]");
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf
		(cur, left,
		 "| |-[synth cfg2]\n"
		 "| | 0x%08x\n"
		 "| |   |||\\/|\\/  bit(s)  description       support/value\n"
		 "| |   |||| ||\n"
		 "| |   |||| | \\____7..0  -                 -\n"
		 "| |   ||||  \\____10..8  -                 -\n"
		 "| |   ||||    \\_____11  stride mode       %s\n"
		 "| |   ||| \\_____13..12  error concealment %s\n"
		 "| |   |||     \\_15..14  extended width    %d\n"
		 "| |   |||     \\_____16  -                 -\n"
		 "| |   |||     \\_18..17  tiled mode        %s\n"
		 "| |   |||     \\_____19  webp              %s\n"
		 "| |   || \\______21..20  h264 mvc          %s\n"
		 "| |   ||      \\_____22  avs               %s\n"
		 "| |   ||      \\_____23  vp8               %s\n"
		 "| |   | \\___________24  vp7               %s\n"
		 "| |   |       \\_26..25  real video        %s\n"
		 "| |   |       \\_____27  -                 -\n"
		 "| |    \\____________28  ref buffer double %s\n"
		 "| |           \\_____29  mpeg4 custom      %s\n"
		 "| |           \\_____30  ref buffer ilace  %s\n"
		 "| |           \\_____31  jpeg extensions   %s\n"
		 "| |\n",
		 *dec_cfg2,
		 (*dec_cfg2 >> G1_STRIDE_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_EC_E) & 0x03U ? "[yes]" : "[no]",
		 ((*dec_cfg2 >> G1_DEC_PIC_W_EXT) & 0x03U) << 11,
		 (*dec_cfg2 >> G1_DEC_TILED_L) & 0x03U ?
			"[yes] tiled 8x4" : "[no]",
		 (*dec_cfg2 >> G1_WEBP_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_MVC_E) & 0x03U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_AVS_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_VP8_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_VP7_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_RV_E) & 0x03U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_REF_BUFF_DOUBLE_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_MPEG4_CUSTOM_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_REF_BUFF_ILACE_E) & 0x01U ? "[yes]" : "[no]",
		 (*dec_cfg2 >> G1_JPEG_EXT_E) & 0x01U ? "[yes]" : "[no]");
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf
		(cur, left,
		 "| |-[fuse]\n"
		 "| | 0x%08x\n"
		 "|     ||||||||  bit(s)  description       fuse\n"
		 "|     ||||||||\n"
		 "|     ||||||| \\___3..0   -                -\n"
		 "|     |||||| \\____6..4   -                -\n"
		 "|     ||||||  \\______7  ref buffer        %s\n"
		 "|     ||||| \\____11..8  -                 -\n"
		 "|     |||| \\________12   352 max          %s\n"
		 "|     ||||    \\_____13   720 max          %s\n"
		 "|     ||||    \\_____14  1280 max          %s\n"
		 "|     ||||    \\_____15  1920 max          %s\n"
		 "|     ||| \\_________16  4096 max          %s\n"
		 "|     |||     \\_____17  -                 -\n"
		 "|     |||     \\_____18  h264 mvc          %s\n"
		 "|     |||     \\_____19  avs               %s\n"
		 "|     || \\__________20  vp8               %s\n"
		 "|     ||      \\_____21  vp7               %s\n"
		 "|     ||      \\_____22  real video        %s\n"
		 "|     ||      \\_____23  custom mpeg4      %s\n"
		 "|     | \\___________24  progressive jpeg  %s\n"
		 "|     |       \\_____25  vc1               %s\n"
		 "|     |       \\_____26  vp6               %s\n"
		 "|     |       \\_____27  jpeg              %s\n"
		 "|      \\____________28  sorenson/spark    %s\n"
		 "|             \\_____29  mpeg2             %s\n"
		 "|             \\_____30  mpeg4             %s\n"
		 "|             \\_____31  h264              %s\n"
		 "|\n",
		 *dec_fuse,
		 (*dec_fuse >> G1_REF_BUFF_FUSE_E) & 0x01U ? "x" : "",
		 *dec_fuse & 0x1000U ? "x" : "",
		 *dec_fuse & 0x2000U ? "x" : "",
		 *dec_fuse & 0x4000U ? "x" : "",
		 *dec_fuse & 0x8000U ? "x" : "",
		 *dec_fuse & 0x10000U ? "x" : "",
		 (*dec_fuse >> G1_MVC_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_AVS_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_VP8_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_VP7_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_RV_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_CUSTOM_MPEG4_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_PJPEG_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_VC1_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_VP6_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_JPEG_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_SORENSONSPARK_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_MPEG2_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_MPEG4_FUSE_E) & 0x01U ? "x" : "",
		 (*dec_fuse >> G1_H264_FUSE_E) & 0x01U ? "x" : "");
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf
		(cur, left,
		 "|-[post processor]\n"
		 "  |-[cfg]\n"
		 "  | 0x%08x\n"
		 "  |   ||||\\  /  bit(s)  description       support/value\n"
		 "  |   |||| |\n"
		 "  |   ||||  \\____12..0  max width         %d\n"
		 "  |   ||||    \\_____13  -                 -\n"
		 "  |   ||||    \\_15..14  tiled input mode  0x%x\n"
		 "  |   ||| \\_________16  post processor    %s\n"
		 "  |   |||     \\_19..17  -                 -\n"
		 "  |   || \\______23..20  -                 -\n"
		 "  |   | \\_______27..24  post proc config  0x%x\n"
		 "  |    \\________31..28  -                 -\n"
		 "  |\n",
		 *pp_cfg,
		 *pp_cfg & 0x1FFFU,
		 (*pp_cfg >> G1_PP_IN_TILED_L) & 0x03U,
		 (*pp_cfg >> G1_PP_E) & 0x01U ? "[yes]" : "[no]",
		 (*pp_cfg >> G1_CFG_E) & 0x0FU);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf
		(cur, left,
		 "  |-[fuse]\n"
		 "    0x%08x\n"
		 "      |\\/||      bit(s)  description       fuse\n"
		 "      || ||\n"
		 "      || | \\________12   352 max          %s\n"
		 "      || |    \\_____13   720 max          %s\n"
		 "      || |    \\_____14  1280 max          %s\n"
		 "      || |    \\_____15  1920 max          %s\n"
		 "      ||  \\_________16  4096 max          %s\n"
		 "      ||      \\_19..17  -                 -\n"
		 "      | \\_______27..20  -                 -\n"
		 "       \\____________28  -                 -\n"
		 "              \\_____29  alpha blending    %s\n"
		 "              \\_____30  deinterlacing     %s\n"
		 "              \\_____31  post processor    %s\n"
		 "\n",
		 *pp_fuse,
		 *pp_fuse & 0x1000U ? "x" : "",
		 *pp_fuse & 0x2000U ? "x" : "",
		 *pp_fuse & 0x4000U ? "x" : "",
		 *pp_fuse & 0x8000U ? "x" : "",
		 *pp_fuse & 0x10000U ? "x" : "",
		 (*pp_fuse >> G1_PP_ALPHA_BLEND_FUSE_E) & 0x01U ? "x" : "",
		 (*pp_fuse >> G1_PP_DEINTERLACE_FUSE_E) & 0x01U ? "x" : "",
		 (*pp_fuse >> G1_PP_FUSE_E) & 0x01U ? "x" : "");
	cnt = (left > ret ? ret : left);

	return pstr;
}

