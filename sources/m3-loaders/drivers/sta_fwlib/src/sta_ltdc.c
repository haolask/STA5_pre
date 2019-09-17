/**
 * @file sta_ltdc.c
 * @brief  This file provides drivers for CLCD IP
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#include <errno.h>
#include <string.h>
#include "utils.h"

#include "sta_ltdc.h"
#include "sta_map.h"
#include "sta_src.h"
#include "sta_type.h"
#include "sta_pinmux.h"
#include "sta_rpmsg_mm.h"

#define NB_CRTC 1
#define CRTC_MASK GENMASK(NB_CRTC - 1, 0)

#define HWVER_10200 0x010200
#define HWVER_10300 0x010300
#define HWVER_20101 0x020101

#define MAX_LAYERS      4

struct ltdc_caps {
	uint32_t nb_layers;          /* number of supported layers */
	uint32_t rg_ofst;            /* register offset for applicable regs */
	const enum ltdc_pix_fmt *pix_fmt_hw; /* supported pixel formats */
	bool ycbcr_support[MAX_LAYERS]; /* YUV to RGB conversion support */
	bool clut_support[MAX_LAYERS]; /* CLUT support */
};

struct ltdc {
	uint32_t hw_version;
	uint32_t bus_width;
	struct ltdc_caps caps;
	bool dual_display;
};

#define reg_r(reg)		read_reg(CLCD_BASE + (reg))
#define reg_w(val, reg)		write_reg(val, CLCD_BASE + (reg))
#define reg_set(mask, reg)	reg_w(reg_r(reg) | (mask), reg)
#define reg_clear(mask, reg)	reg_w(reg_r(reg) & ~(mask), reg)
#define reg_clear_set(clear_mask, set_mask, reg) \
		reg_w((reg_r(reg) & ~(clear_mask)) | (set_mask), reg)

/*
 * The address of some registers depends on the HW version: such registers have
 * an extra offset specified with rg_ofst.
 */
#define RG_OFST_NONE    0
#define RG_OFST_4	4 /* Insertion of "Layer Configuration 2" reg */

/* LTDC global register offsets in bytes */
#define LTDC_IDR	0x0000 /* IDentification */
#define LTDC_LCR	0x0004 /* Layer Count */
#define LTDC_SSCR	0x0008 /* Synchronization Size */
#define LTDC_BPCR	0x000C /* Back Porch */
#define LTDC_AWCR	0x0010 /* Active Width */
#define LTDC_TWCR	0x0014 /* Total Width */
#define LTDC_GCR	0x0018 /* Global Control */
#define LTDC_GC1R	0x001C /* Global Configuration 1 */
#define LTDC_GC2R	0x0020 /* Global Configuration 1 */
#define LTDC_SRCR	0x0024 /* Shadow Reload */
#define LTDC_GACR	0x0028 /* Gamma Correction */
#define LTDC_BCCR	0x002C /* Background Color */
#define LTDC_IER	0x0034 /* Interrupt Enable */
#define LTDC_ISR	0x0038 /* Interrupt Status */
#define LTDC_ICR	0x003C /* Interrupt Clear */
#define LTDC_LIPCR	0x0040 /* Line Interrupt Position */
#define LTDC_CPSR	0x0044 /* Current Position Status */
#define LTDC_CDSR	0x0048 /* Current Display Status */
#define LTDC_EDCR	0x0060 /* External Display Control */

/* LTDC layer register offsets */
#define LTDC_L1LC1R	(0x0080)                      /* L1 Layer Cfg 1 */
#define LTDC_L1LC2R	(0x0084)                      /* L1 Layer Cfg 2 */
#define LTDC_L1CR	(0x0084 + ltdc.caps.rg_ofst) /* L1 Control */
#define LTDC_L1WHPCR	(0x0088 + ltdc.caps.rg_ofst) /* L1 Window H Pos */
#define LTDC_L1WVPCR	(0x008C + ltdc.caps.rg_ofst) /* L1 Window V Pos */
#define LTDC_L1CKCR	(0x0090 + ltdc.caps.rg_ofst) /* L1 Color Keying */
#define LTDC_L1PFCR	(0x0094 + ltdc.caps.rg_ofst) /* L1 Pixel Format */
#define LTDC_L1CACR	(0x0098 + ltdc.caps.rg_ofst) /* L1 Constant Alpha */
#define LTDC_L1DCCR	(0x009C + ltdc.caps.rg_ofst) /* L1 Default Color */
#define LTDC_L1BFCR	(0x00A0 + ltdc.caps.rg_ofst) /* L1 Blend Factors */
#define LTDC_L1FBBCR	(0x00A4 + ltdc.caps.rg_ofst) /* L1 FB Bus Ctrl */
#define LTDC_L1AFBCR	(0x00A8 + ltdc.caps.rg_ofst) /* L1 AuxFB Ctrl */
#define LTDC_L1CFBAR	(0x00AC + ltdc.caps.rg_ofst) /* L1 ColorFB Address */
#define LTDC_L1CFBLR	(0x00B0 + ltdc.caps.rg_ofst) /* L1 ColorFB Length */
#define LTDC_L1CFBLNR	(0x00B4 + ltdc.caps.rg_ofst) /* L1 ColorFB Line Nb */
#define LTDC_L1AFBAR	(0x00B8 + ltdc.caps.rg_ofst) /* L1 AuxFB Address */
#define LTDC_L1AFBLR	(0x00BC + ltdc.caps.rg_ofst) /* L1 AuxFB Length */
#define LTDC_L1AFBLNR	(0x00C0 + ltdc.caps.rg_ofst) /* L1 AuxFB Line Nb */
#define LTDC_L1CLUTWR	(0x00C4 + ltdc.caps.rg_ofst) /* L1 CLUT Write */
#define LTDC_L1YS1R	(0x00E0 + ltdc.caps.rg_ofst) /* L1 YCbCr Scale 1 */
#define LTDC_L1YS2R	(0x00E4 + ltdc.caps.rg_ofst) /* L1 YCbCr Scale 2 */

#define LAY_OFFSET	0x0080 /* Register Offset between 2 layers */

/* Bit definitions */
#define SSCR_VSH	0x000007FF /* Vertical Synchronization Height */
#define SSCR_HSW	0x0FFF0000 /* Horizontal Synchronization Width */

#define BPCR_AVBP	0x000007FF /* Accumulated Vertical Back Porch */
#define BPCR_AHBP	0x0FFF0000 /* Accumulated Horizontal Back Porch */

#define AWCR_AAH	0x000007FF /* Accumulated Active Heigh */
#define AWCR_AAW	0x0FFF0000 /* Accumulated Active Width */

#define TWCR_TOTALH	0x000007FF /* TOTAL Heigh */
#define TWCR_TOTALW	0x0FFF0000 /* TOTAL Width */

#define GCR_LTDCEN	0x00000001 /* LTDC ENable */
#define GCR_PCPOL	0x10000000 /* Pixel Clock POLarity */
#define GCR_DEPOL	0x20000000 /* Data Enable POLarity */
#define GCR_VSPOL	0x40000000 /* Vertical Synchronization POLarity */
#define GCR_HSPOL	0x80000000 /* Horizontal Synchronization POLarity */
#define GCR_GAMEN	0x00000002 /* GAMma correction ENable */
#define GCR_DITHEN	0x00010000 /* Dithering enable */

#define SRCR_IMR	0x00000001 /* IMmediate Reload */
#define SRCR_VBR	0x00000002 /* Vertical Blanking Reload */

#define BCCR_BCBLUE	0x000000FF /* Background Blue value */
#define BCCR_BCGREEN	0x0000FF00 /* Background Green value */
#define BCCR_BCRED	0x00FF0000 /* Background Red value */

#define GACR_RED (1<<18)
#define GACR_GREEN (1<<17)
#define GACR_BLUE (1<<16)

#define BCCR_BCYELLOW	0x00FFFF00 /* Background yellow value */
#define BCCR_TRANSPARENT	0x00000000 /* Transparent Background value (alpha = 0) */
#define BCCR_HALF_TRANSPARENT	0x7F000000 /* Half Transparent Background value */
#define BCCR_NOT_TRANSPARENT	0xFF000000 /* Not Transparent Background value */


#define IER_LIE		0x00000001 /* Line Interrupt Enable */
#define IER_FUIE	0x00000002 /* Fifo Underrun Interrupt Enable */
#define IER_TERRIE	0x00000004 /* Transfer ERRor Interrupt Enable */
#define IER_RRIE	0x00000008 /* Register Reload Interrupt enable */

#define ISR_LIF		0x00000001 /* Line Interrupt Flag */
#define ISR_FUIF	0x00000002 /* Fifo Underrun Interrupt Flag */
#define ISR_TERRIF	0x00000004 /* Transfer ERRor Interrupt Flag */
#define ISR_RRIF	0x00000008 /* Register Reload Interrupt Flag */

#define EDCR_DVE	0x00100000 /* Dual-View Enable */
#define EDCR_SPME	0x00200000 /* Sub-Pixel Mixing Enable */
#define EDCR_EHFPCE	0x00400000 /* Even Half-Frequency Pixel Clock Enable */
#define EDCR_OHFPCE	0x00800000 /* Odd Half-Frequency Pixel Clock Enable */
#define EDCR_HPCAES	0x01000000 /* Half Pixel Clock Active Edge Shift */

#define LXLC2R_YCBCR	0x40000000 /* YCbCr support */
#define LXLC1R_CLUTCR	0x00000040 /* CLUT support */

#define LXCR_LEN	0x00000001 /* Layer ENable */
#define LXCR_CLUTEN	0x00000010 /* Color Look-Up Table ENable */
#define LXCR_CKEN	0x00000002 /* Color Key ENable */
#define LXCR_DCBEN	0x00000200 /* Default Colour Blending ENable */
#define LXCR_HMIRROR	0x00000100 /* Horizontal Mirroring */

#define LXCR_INSODDEVEN	0x00000000 /* Insert pixel values on both pixels (odd and even) */
#define LXCR_INSODDEVENDUP	0x000000C0 /* Insert pixel values on both pixels (duplicating the same pixel value) */
#define LXCR_INSEVEN	0x00000080 /* Insert pixel values on even pixels only */
#define LXCR_INSODD		0x00000040 /* Insert pixel values on odd pixels only */
#define LXCR_INSMODE	0x000000C0 /* Insert pixel values mask */


#define LXWHPCR_WHSTPOS	0x00000FFF /* Window Horizontal StarT POSition */
#define LXWHPCR_WHSPPOS	0x0FFF0000 /* Window Horizontal StoP POSition */

#define LXWVPCR_WVSTPOS	0x000007FF /* Window Vertical StarT POSition */
#define LXWVPCR_WVSPPOS	0x07FF0000 /* Window Vertical StoP POSition */

#define LXPFCR_PF	0x00000007 /* Pixel Format */

#define LXCACR_CONSTA	0x000000FF /* CONSTant Alpha */
#define CONSTA_MAX	0x000000FF /* Alpha = 1.0 */

#define LXBFCR_BF2	0x00000007 /* Blending Factor 2 */
#define LXBFCR_BF1	0x00000700 /* Blending Factor 1 */
#define BF1_PAXCA	0x00000600 /* Pixel Alpha x Constant Alpha */
#define BF2_1PAXCA	0x00000007 /* 1 - (Pixel Alpha x Constant Alpha) */
#define BF1_CA		0x00000400 /* Constant Alpha */
#define BF2_1CA		0x00000005 /* 1 - Constant Alpha */

#define LXCFBLR_CFBLL	0x00001FFF /* Color Frame Buffer Line Length */
#define LXCFBLR_CFBP	0x1FFF0000 /* Color Frame Buffer Pitch in bytes */

#define LXCFBLNR_CFBLNBR 0x000007FF /* Color Frame Buffer Line NumBeR */

#define LXAFBCR_YCBCR	0x000003F8 /* YCbCr related controls */
#define LXAFBCR_YHE	0x00000200 /* Y Headroom Enable */
#define LXAFBCR_OCYF	0x000001C0 /* Odd, Cb, Y First */
#define OCYF_YUYV	0x000000C0 /* DRM_FORMAT_YUYV */
#define OCYF_UYVY	0x00000080 /* DRM_FORMAT_UYVY */
#define OCYF_YVYU	0x00000040 /* DRM_FORMAT_YVYU */
#define OCYF_VYUY	0x00000000 /* DRM_FORMAT_VYUY */
#define LXAFBCR_YMODE	0x00000030 /* YCbCr mode */
#define YMODE_422I	0x00000000 /* 4:2:2 interleaved */
#define YMODE_420SP	0x00000010 /* 4:2:0 semi planar */
#define LXAFBCR_EN	0x00000008 /* YCbCr conversion enable */

#define MISC_CLCDINTREN 0x00000038 /* MISC CLCD interrupt enable */

#define REG4_FULL_CLK_MASK      BIT(2)
#define CLCDINTREN_ALL  0x0000000F

#define L1YS1R_BT601	0x02040199 /* BT601 coeff (b_cb = 516 / r_cr = 409) */
#define L1YS2R_BT601	0x006400D0 /* BT601 coeff (g_cb = 100 / g_cr = 208) */

#define NB_PF           8       /* Max nb of std (not YCbCr) HW pixel format */

/* The index gives the encoding of the pixel format for an HW version */
static const enum ltdc_pix_fmt ltdc_pix_fmt_a1[NB_PF] = {
		PF_ARGB8888,	/* 0x00 */
		PF_RGB888,	/* 0x01 */
		PF_RGB565,	/* 0x02 */
		PF_RGBA8888,	/* 0x03 */
		PF_AL44,	/* 0x04 */
		PF_L8,		/* 0x05 */
		PF_ARGB1555,	/* 0x06 */
		PF_ARGB4444	/* 0x07 */
};

struct ltdc ltdc;

static void *ltdc_ept;
static uint8_t ltdc_overlay_status[4];

static uint32_t to_ltdc_pixelformat(enum ltdc_pix_fmt pf)
{
	unsigned int i;

	/* check if the pixel format is indicated as supported in CLCD registers */
	for (i = 0; i < NB_PF; i++) {
		if (ltdc.caps.pix_fmt_hw[i] == pf)
			return i;
	}

	/* if the pixel format requested is YUV this info won't be used => we select 0 by default */
	return 0;
}


/* ltdc_fb_to_bypp - determine the bytes per pixel */
static unsigned int ltdc_fb_to_bypp(struct framebuffer *fb)
{
	switch (fb->pixel_format) {
	case PF_FORMAT_NV12:
	case PF_FORMAT_NV21:
		return 1; /* 1 byte per pixel for both planes */
	case PF_FORMAT_YUYV:
	case PF_FORMAT_YVYU:
	case PF_FORMAT_UYVY:
	case PF_FORMAT_VYUY:
		return 2;
	case PF_RGB565:
	case PF_ARGB1555:
	case PF_ARGB4444:
		return 2;
	case PF_RGB888:
		return 3;
	case PF_ARGB8888:
	case PF_RGBA8888:
		return 4;
	case PF_AL44:
	case PF_L8:
		return 1;
	default:
		return 0;
	}
}

/* ltdc_fb_to_num_planes - determine the number of planes */
static unsigned int ltdc_fb_to_num_planes(struct framebuffer *fb)
{
	switch (fb->pixel_format) {
	case PF_FORMAT_NV12:
	case PF_FORMAT_NV21:
		return 2;
	default:
		return 1;
	}
}

static uint32_t get_ycbcr_config(enum ltdc_pix_fmt pf)
{
	uint32_t ycbcr_config;

	switch (pf) {
	case PF_FORMAT_YUYV:
		ycbcr_config = YMODE_422I | OCYF_YUYV;
		break;
	case PF_FORMAT_YVYU:
		ycbcr_config = YMODE_422I | OCYF_YVYU;
		break;
	case PF_FORMAT_UYVY:
		ycbcr_config = YMODE_422I | OCYF_UYVY;
		break;
	case PF_FORMAT_VYUY:
		ycbcr_config = YMODE_422I | OCYF_VYUY;
		break;
	case PF_FORMAT_NV12:
		ycbcr_config = YMODE_420SP | OCYF_UYVY;
		break;
	case PF_FORMAT_NV21:
		ycbcr_config = YMODE_420SP;
		break;
	default:
		/* Not a YUV format */
		return 0;
	}

	/* Enable with limited range */
	ycbcr_config |= LXAFBCR_YHE | LXAFBCR_EN;

	return ycbcr_config;
}

static uint32_t remove_alpha(uint32_t drm)
{
	switch (drm) {
	case PF_ARGB8888:
	case PF_RGBA8888:
	case PF_ARGB1555:
	case PF_ARGB4444:
	case PF_AL44:
		return 1;
	default:
		return 0;
	}
}

static uint32_t ltdc_get_paddr(struct framebuffer *fb, int x, int y,
		unsigned int plane)
{
	uint32_t paddr;
	unsigned int pixsize;

	pixsize = ltdc_fb_to_bypp(fb);

	paddr = fb->paddr + fb->offsets[plane];
	paddr += x * pixsize + y * fb->pitches[plane];

	return paddr;
}

int ltdc_update_gamma_correction(uint16_t *gam_r, uint16_t *gam_g, uint16_t *gam_b)
{
	int i;
	uint32_t val;

	/* Enable clcd register read/write clock */
	srcm3_pclk_enable(src_m3_regs, CLCD_CLK);

	for (i = 0; i < GAMMA_SIZE; i++) {
		/* red value */
		val = GACR_RED | (gam_r[i] << 8) | i;
		reg_w(val, LTDC_GACR);
		/* green value */
		val = GACR_GREEN | (gam_g[i] << 8) | i;
		reg_w(val, LTDC_GACR);
		/* blue value */
		val = GACR_BLUE | (gam_b[i] << 8) | i;
		reg_w(val, LTDC_GACR);
	}

	/* Enable Gamma Correction */
	reg_clear_set(GCR_GAMEN, GCR_GAMEN, LTDC_GCR);

	return 0;
}

int ltdc_layer_update_int(uint32_t lay_id, struct framebuffer *fb,
		uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h,
		uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1,
		enum display_sel display)
{
	uint32_t val, pitch_in_bytes, line_length, paddr;
	uint32_t start_pos, stop_pos, max_pos;

	line_length = (x1 - x0 + 1) * ltdc_fb_to_bypp(fb) +
	      (ltdc.bus_width >> 3) - 1;

	x1++;

	/* if dual display, double horizontal position */
	if (ltdc.dual_display) {
		x0 *= 2;
		x1 *= 2;
	}

	/* Configures the horizontal start and stop position */
	start_pos = x0 + 1 + ((reg_r(LTDC_BPCR) & BPCR_AHBP) >> 16);
	stop_pos = x1 + ((reg_r(LTDC_BPCR) & BPCR_AHBP) >> 16) ;
	max_pos = (reg_r(LTDC_AWCR) & AWCR_AAW) >> 16;
	if ((start_pos > max_pos) || (stop_pos > max_pos))
		return 2;
	val = (stop_pos<<16) | start_pos;
	reg_clear_set(LXWHPCR_WHSTPOS | LXWHPCR_WHSPPOS, val,
			LTDC_L1WHPCR + lay_id * LAY_OFFSET);

	/* Configures the vertical start and stop position */
	start_pos = y0 + 1 + (reg_r(LTDC_BPCR) & BPCR_AVBP);
	stop_pos = y1 + 1 + (reg_r(LTDC_BPCR) & BPCR_AVBP);
	max_pos = reg_r(LTDC_AWCR) & AWCR_AAH;
	if ((start_pos > max_pos) || (stop_pos > max_pos))
		return 3;
	val = (stop_pos<<16) | start_pos;
	reg_clear_set(LXWVPCR_WVSTPOS | LXWVPCR_WVSPPOS, val,
			LTDC_L1WVPCR + lay_id * LAY_OFFSET);

	/* Specifies the pixel format */
	val = to_ltdc_pixelformat(fb->pixel_format);
	reg_clear_set(LXPFCR_PF, val, LTDC_L1PFCR + lay_id * LAY_OFFSET);

	/* Configures the YUV conversion */
	val = get_ycbcr_config(fb->pixel_format);
	reg_clear_set(LXAFBCR_YCBCR, val, LTDC_L1AFBCR + lay_id * LAY_OFFSET);

	if (val) {
		/* Configures the YUV to RGB scale coefficients */
		reg_w(L1YS1R_BT601, LTDC_L1YS1R + lay_id * LAY_OFFSET);
		reg_w(L1YS2R_BT601, LTDC_L1YS2R + lay_id * LAY_OFFSET);
	}

	/* Configures the color frame buffer pitch in bytes & line length */
	pitch_in_bytes = fb->pitches[0];

	val = ((pitch_in_bytes << 16) | line_length);
	reg_clear_set(LXCFBLR_CFBLL | LXCFBLR_CFBP, val,
			LTDC_L1CFBLR + lay_id * LAY_OFFSET);

	/* Specifies the constant alpha value */
	val = CONSTA_MAX;
	reg_clear_set(LXCACR_CONSTA, val, LTDC_L1CACR + lay_id * LAY_OFFSET);

	/* Specifies the blending factors : with or without pixel alpha */
	val = remove_alpha(fb->pixel_format) ?
			BF1_PAXCA | BF2_1PAXCA : BF1_CA | BF2_1CA;
	reg_clear_set(LXBFCR_BF2 | LXBFCR_BF1, val,
			LTDC_L1BFCR + lay_id * LAY_OFFSET);

	/* Configures the frame buffer line number */
	val = y1 - y0 + 1;
	reg_clear_set(LXCFBLNR_CFBLNBR, val,
			LTDC_L1CFBLNR + lay_id * LAY_OFFSET);

	/* Sets the FB address */
	paddr = ltdc_get_paddr(fb, src_x, src_y, 0);
	if (!paddr)
		return 4;
	reg_w(paddr, LTDC_L1CFBAR + lay_id * LAY_OFFSET);

	/* Configure second plane */
	if (ltdc_fb_to_num_planes(fb) >= 2) {
		/* Configures the Aux FB address */
		paddr = ltdc_get_paddr(fb, src_x, src_y, 1);
		if (!paddr)
			return 5;
		reg_w(paddr, LTDC_L1AFBAR + lay_id * LAY_OFFSET);

		/* Configures the frame buffer pitch in bytes & line length */
		pitch_in_bytes = fb->pitches[1];
		/* Same line_length as first plane for supported formats */
		val = ((pitch_in_bytes << 16) | line_length);
		reg_clear_set(LXCFBLR_CFBLL | LXCFBLR_CFBP, val,
				LTDC_L1AFBLR + lay_id * LAY_OFFSET);

		/* Configures the frame buffer line number */
		val = (y1 - y0 + 1) / 2;
		reg_clear_set(LXCFBLNR_CFBLNBR, val,
				LTDC_L1AFBLNR + lay_id * LAY_OFFSET);
	}

	/* Enable layer and CLUT if needed */
	val = ((fb->pixel_format == PF_L8) || (fb->pixel_format == PF_AL44))
		? LXCR_CLUTEN : 0;
	val |= LXCR_LEN;

	if (ltdc.dual_display) {
		switch (display) {
		case ODD:
			val |= LXCR_INSODD;
			break;
		case EVEN:
			val |= LXCR_INSEVEN;
			break;
		case BOTH:
		default:
			val |= LXCR_INSEVEN | LXCR_INSODD;
			break;
		}
		reg_clear_set(LXCR_LEN | LXCR_CLUTEN | LXCR_INSMODE, val,
			      LTDC_L1CR + lay_id * LAY_OFFSET);
	} else {
		reg_clear_set(LXCR_LEN | LXCR_CLUTEN, val,
			      LTDC_L1CR + lay_id * LAY_OFFSET);
	}

	return 0;
}

static int ltdc_set_caps(void)
{
	unsigned int i;
	unsigned int nb_layers = reg_r(LTDC_LCR);

	if (nb_layers != MAX_LAYERS)
		return -6;

	ltdc.caps.nb_layers = nb_layers;
	ltdc.bus_width = 64;

	ltdc.hw_version = reg_r(LTDC_IDR);

	switch (ltdc.hw_version) {
	case HWVER_20101:
		ltdc.caps.rg_ofst = RG_OFST_4;
		ltdc.caps.pix_fmt_hw = ltdc_pix_fmt_a1;
		for (i = 0; i < ltdc.caps.nb_layers; i++) {
			if (LXLC2R_YCBCR & reg_r(LTDC_L1LC2R + i * LAY_OFFSET))
				ltdc.caps.ycbcr_support[i] = true;
			else
				ltdc.caps.ycbcr_support[i] = false;
			if (LXLC1R_CLUTCR & reg_r(LTDC_L1LC1R + i * LAY_OFFSET))
				ltdc.caps.clut_support[i] = true;
			else
				ltdc.caps.clut_support[i] = false;
		}
		break;
	default:
		return -7;
	}

	return 0;
}

int ltdc_rpmsg_mm_callback(struct s_rpmsg_mm_data *data, void *priv)
{
	int index = 0;

	if (!data)
		return -1;

	switch (RPMSG_MM_GET_INFO(data->info)) {
	case RPMSG_MM_SHARED_RES_STATUS_REQ:
		data->len = 1;
		data->info = RPMSG_MM_SHARED_RES_STATUS_ACK;
		data->data[0] = 0;
		for (index = 0; index < 4; index++)
			if (ltdc_overlay_status[index] ==
				RPMSG_MM_SHARED_RES_LOCKED)
				data->data[0] |=
					(RPMSG_MM_HW_RES_OVERLAY_BASE << index);

		TRACE_INFO("return display overlay usage status = 0x%x\n",
			   data->data[0]);
		break;
	default:
		break;
	}
	return 0;
}

int ltdc_rpmsg_mm_update_status(uint32_t layer_id, int status)
{
	uint32_t resource;
	int ret = 0;

	if (!ltdc_ept || layer_id >= ltdc.caps.nb_layers) {
		TRACE_INFO("Unable to book LTDC layer %d resource !!!\n",
			   layer_id);
		return -1;
	}

	resource = (RPMSG_MM_HW_RES_OVERLAY_BASE << layer_id);

	if (status == RPMSG_MM_SHARED_RES_UNLOCKED)
		ret = rpmsg_mm_unlock_resource(ltdc_ept,
					       resource,
					       RPSMG_MM_EPT_CORTEXA_DISPLAY);
	else
		ret = rpmsg_mm_lock_resource(ltdc_ept,
					     status,
					     resource,
					     RPSMG_MM_EPT_CORTEXA_DISPLAY);
	if (!ret)
		ltdc_overlay_status[layer_id] = status;

	TRACE_INFO("%s => %s%s LTDC layer %d\n",
		   __func__,
		   (ret < 0 ? "Warning, Unable to " : ""),
		   (status == RPMSG_MM_SHARED_RES_LOCKED ? "lock" : "unlock"),
		   layer_id);
	return 0;
}

static int layer_lock_count[RPMSG_MM_NB_OVERLAY];
int ltdc_layer_book(uint32_t lay_id)
{
	layer_lock_count[lay_id]++;

	if (layer_lock_count[lay_id] == 1)
		return ltdc_rpmsg_mm_update_status(lay_id,
						   RPMSG_MM_SHARED_RES_LOCKED);

	return 0;
}

int ltdc_layer_release(uint32_t lay_id)
{
	layer_lock_count[lay_id]--;

	if (!layer_lock_count[lay_id])
		return ltdc_rpmsg_mm_update_status(
			lay_id, RPMSG_MM_SHARED_RES_UNLOCKED);

	return 0;
}

/*
 * Provide the top layer that supports a given color format.
 * Lock this layer and upper layer if any.
 */
int ltdc_layer_book_top_layer(enum ltdc_pix_fmt pix_fmt, uint32_t *lay_id)
{
	bool layer_found = false;
	uint32_t i = 0, j = 0;
	int ret = 0;

	switch (pix_fmt) {
	case PF_NONE:
		return -1;
	/* For all YUV formats, check if layer supports it */
	case PF_FORMAT_YUYV:
	case PF_FORMAT_YVYU:
	case PF_FORMAT_UYVY:
	case PF_FORMAT_VYUY:
	case PF_FORMAT_NV12:
	case PF_FORMAT_NV21:
		i = ltdc.caps.nb_layers;
		while (i-- > 0) {
			/* Check if layer supports YUV format */
			if (ltdc.caps.ycbcr_support[i]) {
				*lay_id = i;
				layer_found = true;
				break;
			}
		}
		break;
	/* All layers support non YUV formats: book the top layer */
	default:
		*lay_id = ltdc.caps.nb_layers - 1;
		layer_found = true;
		break;
	}

	if (!layer_found)
		return -1;

	/* Lock this layer and upper layer if any. */
	for (i = *lay_id; i < ltdc.caps.nb_layers; i++) {
		ret = ltdc_layer_book(i);
		if (ret)
			goto err;
	}

	return 0;

err:
	for (j = *lay_id; j < i - 1; j++)
		ltdc_layer_release(j);

	return ret;
}

/* Release this layer and upper layer if any. */
int ltdc_layer_release_top_layer(uint32_t lay_id)
{
	uint32_t i = 0;
	int ret = 0;

	for (i = lay_id; i < ltdc.caps.nb_layers; i++)
		ret |= ltdc_layer_release(i);

	return ret;
}

int ltdc_init(void)
{
	int ret, i;

	/* configure RGB LCD GPIO */
	pinmux_request("lcd_mux");

	/* Enable clcd IP clock for caps reading */
	ret = srcm3_pclk_enable(src_m3_regs, CLCD_CLK);
	if (ret)
		return ret;

	/* we read the CLCD version and associated capabilities */
	ret = ltdc_set_caps();
	if (ret)
		return ret;

	/* disable clcd IP clock */
	ret = srcm3_pclk_disable(src_m3_regs, CLCD_CLK);
	if (ret)
		return ret;

	if (!ltdc_ept)
		ltdc_ept = rpmsg_mm_register_local_endpoint(
				RPSMG_MM_EPT_CORTEXM_DISPLAY,
				ltdc_rpmsg_mm_callback,
				NULL);
	if (!ltdc_ept)
		ret = -1;

	for (i = 0; i < RPMSG_MM_NB_OVERLAY; i++)
		layer_lock_count[i] = 0;

	return ret;
}

int ltdc_mode_set(struct videomode *pps_vm, bool dual_display)
{
	uint32_t hsync, vsync, accum_hbp, accum_vbp, accum_act_w, accum_act_h;
	uint32_t total_width, total_height;
	uint32_t val;
	int ret;

	if (!pps_vm)
		return -1;

	/* double horizontal timings and input pixel clock for dual_display */
	if (dual_display) {
		ltdc.dual_display = 1;
		pps_vm->hactive *= 2;
		pps_vm->hback_porch *= 2;
		pps_vm->hfront_porch *= 2;
		pps_vm->hsync_len *= 2;
	} else
		ltdc.dual_display = 0;

	/* convert video timings to ltdc timings */
	hsync = pps_vm->hsync_len - 1;
	vsync = pps_vm->vsync_len - 1;
	accum_hbp = hsync + pps_vm->hback_porch;
	accum_vbp = vsync + pps_vm->vback_porch;
	accum_act_w = accum_hbp + pps_vm->hactive;
	accum_act_h = accum_vbp + pps_vm->vactive;
	total_width = accum_act_w + pps_vm->hfront_porch;
	total_height = accum_act_h + pps_vm->vfront_porch;


	/* Enable clcd register read/write clock */
	ret = srcm3_pclk_enable(src_m3_regs, CLCD_CLK);
	if (ret)
		return ret;

	/* TODO Enable clcd functional clock
	 * no srca7_pclk_enable available for the moment
	 * but clock is enabled by default in
	 * srca7_init/a7_configure_pllarm/srca7_pclk_enable_all
	 */

	/* Configures HSYNC, VSYNC, DE polarities and Pixel Clock edge */
	val = 0;

	/* HSYNC active low by default (0) */
	if (pps_vm->flags & DISPLAY_FLAGS_HSYNC_HIGH)
		val |= GCR_HSPOL;

	/* VSYNC active low by default (0) */
	if (pps_vm->flags & DISPLAY_FLAGS_VSYNC_HIGH)
		val |= GCR_VSPOL;

	/* DE active high by default (0) */
	if (pps_vm->flags & DISPLAY_FLAGS_DE_LOW)
		val |= GCR_DEPOL;

	/* Pixel Clock on positive edge (no inversion) by default (0) */
	if (pps_vm->flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE)
		val |= GCR_PCPOL;

	reg_clear_set(GCR_HSPOL | GCR_VSPOL | GCR_DEPOL | GCR_PCPOL, val,
		      LTDC_GCR);

	/* Sets Synchronization size */
	val = (hsync << 16) | vsync;
	reg_clear_set(SSCR_VSH | SSCR_HSW, val, LTDC_SSCR);

	/* Sets Accumulated Back porch */
	val = (accum_hbp << 16) | accum_vbp;
	reg_clear_set(BPCR_AVBP | BPCR_AHBP, val, LTDC_BPCR);

	/* Sets Accumulated Active Width */
	val = (accum_act_w << 16) | accum_act_h;
	reg_clear_set(AWCR_AAW | AWCR_AAH, val, LTDC_AWCR);

	/* Sets total width & height */
	val = (total_width << 16) | total_height;
	reg_clear_set(TWCR_TOTALH | TWCR_TOTALW, val, LTDC_TWCR);

	/* Sets the background color value */
	val = BCCR_BCBLUE;
	reg_clear_set(BCCR_BCRED | BCCR_BCGREEN | BCCR_BCBLUE, val, LTDC_BCCR);

	/* Set External Display Control for dual display */
	if (ltdc.dual_display) {
		val =  EDCR_EHFPCE | EDCR_OHFPCE | EDCR_HPCAES;
		reg_clear_set(val, val, LTDC_EDCR);
	}

	/* Enable LTDC by setting LTDCEN bit */
	reg_clear_set(GCR_LTDCEN, GCR_LTDCEN, LTDC_GCR);

	return 0;
}

/* disable interrupts at CLCD level */
void ltdc_disable_interrupts(void)
{
	reg_clear(IER_LIE | IER_RRIE | IER_FUIE | IER_TERRIE, LTDC_IER);
}

int ltdc_disable(void)
{
	unsigned int i;
	int ret;

	/* disable interrupts */
	ltdc_disable_interrupts();

	/* disable layers */
	for (i = 0; i < ltdc.caps.nb_layers; i++)
		reg_clear(LXCR_LEN, LTDC_L1CR + i * LAY_OFFSET);

	/* immediately commit disable of layers before switching off LTDC */
	reg_w(SRCR_IMR, LTDC_SRCR);

	/* disable LTDC */
	reg_clear(GCR_LTDCEN, LTDC_GCR);

	/* Disable clcd register read/write clock */
	ret = srcm3_pclk_disable(src_m3_regs, CLCD_CLK);
	if (ret)
		return ret;

	/* TODO Disable clcd functional clock
	 * Note: no srca7_pclk_disable available for the moment
	 */

	return 0;
}

/* disable a layer */
int ltdc_layer_disable(uint32_t lay_id)
{
	/* disable layer */
	reg_clear(LXCR_LEN, LTDC_L1CR + lay_id * LAY_OFFSET);

	/* commit layer disable */
	reg_set(SRCR_VBR, LTDC_SRCR);

	return 0;
}

/* to update a layer, enable the layer if needed */
int ltdc_layer_update(struct framebuffer *fb, uint32_t lay_id,
		      uint32_t x0, uint32_t y0, enum display_sel display)
{
	uint32_t x1, y1;
	int ret;

	/* Update Layer only if ltdc is enabled */
	if (!(reg_r(LTDC_GCR) & GCR_LTDCEN))
		return -1;

	/* Update the layer only if it has been booked before */
	if (ltdc_overlay_status[lay_id] != RPMSG_MM_SHARED_RES_LOCKED) {
		TRACE_ERR("Error: Impossible to update an unlocked layer");
		return -1;
	}

	if (!fb)
		return -1;

	x1 = x0 + fb->width - 1;
	y1 = y0 + fb->height - 1;

	ret = ltdc_layer_update_int(lay_id, fb, fb->x, fb->y, fb->width,
				    fb->height,	x0, y0, x1, y1, display);
	if (ret)
		return ret;

	reg_set(SRCR_VBR, LTDC_SRCR);

	return 0;
}

