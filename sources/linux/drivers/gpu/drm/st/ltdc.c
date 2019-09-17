/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Philippe Cornu <philippe.cornu@st.com>
 *          Yannick Fertre <yannick.fertre@st.com>
 *          Fabien Dessenne <fabien.dessenne@st.com>
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/clk.h>
#include <linux/component.h>
#include <linux/moduleparam.h>
#include <linux/of_address.h>
#include <linux/of_graph.h>
#include <linux/reset.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_cma_helper.h>
#include <drm/drm_gem_cma_helper.h>
#include <drm/drm_panel.h>
#include <drm/drm_plane_helper.h>

#include <video/videomode.h>

#include "drv.h"
#include "st_hdmi.h"
#include "ltdc.h"
#include "sta_rpmsg_mm.h"

#define HWVER_10200 0x010200
#define HWVER_10300 0x010300
#define HWVER_20101 0x020101

/* clut encoding is 0xIIRRGGBB */
#define RGB_TO_CLUT(i, r, g, b) \
	(((i) << 24) | (((r) >> 8) << 16) | (((g) >> 8) << 8) | ((b) >> 8))

#define FPS_INTERVAL_MS		4000
enum fps_info_id {
	FPS_LAYER1, FPS_LAYER2, FPS_LAYER3, FPS_LAYER4,
	FPS_IRQ_RELOAD, FPS_IRQ_LINE, FPS_INFO_MAX_ENTRIES
};

const char *fps_info_str[FPS_INFO_MAX_ENTRIES] = {
	"LAYER 1", "LAYER 2", "LAYER 3", "LAYER 4",
	"IRQ RELOAD", "IRQ LINE"};

struct fps_info {
	unsigned int frames[FPS_INFO_MAX_ENTRIES];
	ktime_t last_timestamp;
	unsigned int paddr[FPS_INFO_MAX_ENTRIES];
	bool dont_count_duplicated_frame;
};

#define MAX_PLANES	10
#define MAX_CRTC	2
#define MAX_PANELS	2

#define CLUT_SIZE	256

#define LOCKED_LAYER_MASK	GENMASK(RPMSG_MM_NB_OVERLAY - 1, 0)

struct ltdc_caps {
	u32 nb_layers;          /* number of supported layers */
	u32 rg_ofst;            /* register offset for applicable regs */
	const enum ltdc_pix_fmt *pix_fmt_hw; /* supported pixel formats */
	bool ycbcr_support[MAX_PLANES]; /* YUV to RGB conversion support */
	bool dual_display_support; /* Dual display support */
};

/* layer information needed to update layer after unlocked by remote proc */
struct layer_info {
	bool enabled;
	struct drm_rect src;
	struct drm_rect pos;
	struct drm_framebuffer *fb;
	u32 lay_id;
	u32 crtc_id;
};

struct ltdc {
	struct device *dev;
	struct drm_device *drm_dev;

	void __iomem *regs;
	struct clk *clk;     /* ltdc main clock for register configuration */
	struct clk *lcd_clk; /* lcd pixel clock */

	struct drm_panel *panel[MAX_PANELS];
	struct drm_bridge *bridge;

	u32 irq_status;
	u32 error_status;
	struct mutex err_lock; /* protecting error_status */
	struct mutex gamma_lock; /* protecting gamma_status */

	u32 hw_version;
	u32 bus_width;
	struct ltdc_caps caps;

	u32 clut[256];

	struct fps_info fpsi;

	bool dual_display;
	u32 nb_crtc;
	struct drm_atomic_state *state;

	bool wait_for_rpmsg_available;
	struct workqueue_struct *rpmsg_wq;
	struct completion init_rpmsg_complete;
	struct work_struct init_work;
	void *rpmsg_handle;

	u32 locked_layers_status;
	struct mutex layers_info_lock; /* protect layer information */
	struct layer_info lay_infos[RPMSG_MM_NB_OVERLAY];
	bool ltdc_disable_waiting;

	u32 underrun_nb;
	bool gamma_to_update;
	u32 lut_table[CLUT_SIZE * 3]; /* 3 components : Red, Green, Blue */
};

#define reg_r(reg)		readl_relaxed(ltdc->regs + (reg))
#define reg_w(val, reg)		writel_relaxed(val, ltdc->regs + (reg))
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

/* LTDC global register offsets */
#define LTDC_IDR	0x0000 /* IDentification */
#define LTDC_LCR	0x0004 /* Layer Count */
#define LTDC_SSCR	0x0008 /* Synchronization Size */
#define LTDC_BPCR	0x000C /* Back Porch */
#define LTDC_AWCR	0x0010 /* Active Width */
#define LTDC_TWCR	0x0014 /* Total Width */
#define LTDC_GCR	0x0018 /* Global Control */
#define LTDC_GC1R	0x001C /* Global Configuration 1 */
#define LTDC_GC2R	0x0020 /* Global Configuration 2 */
#define LTDC_SRCR	0x0024 /* Shadow Reload */
#define LTDC_GACR	0x0028 /* GAmma Correction */
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
#define LTDC_L1CR	(0x0084 + ltdc->caps.rg_ofst) /* L1 Control */
#define LTDC_L1WHPCR	(0x0088 + ltdc->caps.rg_ofst) /* L1 Window H Pos */
#define LTDC_L1WVPCR	(0x008C + ltdc->caps.rg_ofst) /* L1 Window V Pos */
#define LTDC_L1CKCR	(0x0090 + ltdc->caps.rg_ofst) /* L1 Color Keying */
#define LTDC_L1PFCR	(0x0094 + ltdc->caps.rg_ofst) /* L1 Pixel Format */
#define LTDC_L1CACR	(0x0098 + ltdc->caps.rg_ofst) /* L1 Constant Alpha */
#define LTDC_L1DCCR	(0x009C + ltdc->caps.rg_ofst) /* L1 Default Color */
#define LTDC_L1BFCR	(0x00A0 + ltdc->caps.rg_ofst) /* L1 Blend Factors */
#define LTDC_L1FBBCR	(0x00A4 + ltdc->caps.rg_ofst) /* L1 FB Bus Ctrl */
#define LTDC_L1AFBCR	(0x00A8 + ltdc->caps.rg_ofst) /* L1 AuxFB Ctrl */
#define LTDC_L1CFBAR	(0x00AC + ltdc->caps.rg_ofst) /* L1 ColorFB Address */
#define LTDC_L1CFBLR	(0x00B0 + ltdc->caps.rg_ofst) /* L1 ColorFB Length */
#define LTDC_L1CFBLNR	(0x00B4 + ltdc->caps.rg_ofst) /* L1 ColorFB Line Nb */
#define LTDC_L1AFBAR	(0x00B8 + ltdc->caps.rg_ofst) /* L1 AuxFB Address */
#define LTDC_L1AFBLR	(0x00BC + ltdc->caps.rg_ofst) /* L1 AuxFB Length */
#define LTDC_L1AFBLNR	(0x00C0 + ltdc->caps.rg_ofst) /* L1 AuxFB Line Nb */
#define LTDC_L1CLUTWR	(0x00C4 + ltdc->caps.rg_ofst) /* L1 CLUT Write */
#define LTDC_L1YS1R	(0x00E0 + ltdc->caps.rg_ofst) /* L1 YCbCr Scale 1 */
#define LTDC_L1YS2R	(0x00E4 + ltdc->caps.rg_ofst) /* L1 YCbCr Scale 2 */

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
#define GCR_GAMEN	0x00000002 /* GAMma correction ENable */
#define GCR_PCPOL	0x10000000 /* Pixel Clock POLarity */
#define GCR_DEPOL	0x20000000 /* Data Enable POLarity */
#define GCR_VSPOL	0x40000000 /* Vertical Synchronization POLarity */
#define GCR_HSPOL	0x80000000 /* Horizontal Synchronization POLarity */

#define GC2R_DVIEW	0x00000004 /* Dual View Enabled */

#define GACR_BLUE	0x00010000 /* Select blue CLUT */
#define GACR_GREEN	0x00020000 /* Select green CLUT */
#define GACR_RED	0x00040000 /* Select red CLUT */
#define GACR_MCOLOR	0x0000FF00 /* Mapped Color */


#define SRCR_IMR	0x00000001 /* IMmediate Reload */
#define SRCR_VBR	0x00000002 /* Vertical Blanking Reload */

#define BCCR_BCBLUE	0x000000FF /* Background Blue value */
#define BCCR_BCGREEN	0x0000FF00 /* Background Green value */
#define BCCR_BCRED	0x00FF0000 /* Background Red value */

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

#define LXCR_LEN	0x00000001 /* Layer ENable */
#define LXCR_CLUTEN	0x00000010 /* Color Look-Up Table ENable */

#define LXWHPCR_WHSTPOS	0x00000FFF /* Window Horizontal StarT POSition */
#define LXWHPCR_WHSPPOS	0x0FFF0000 /* Window Horizontal StoP POSition */

#define LXWVPCR_WVSTPOS	0x000007FF /* Window Vertical StarT POSition */
#define LXWVPCR_WVSPPOS	0x07FF0000 /* Window Vertical StoP POSition */

#define LXPFCR_PF	0x00000007 /* Pixel Format */

#define LXCR_INSEVEN	0x00000080 /* Insert pixel value on even pixels only */
#define LXCR_INSODD	0x00000040 /* Insert pixel value on odd pixels only */
#define LXCR_INSMODE	0x000000C0 /* Insert pixel value mask */

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

#define L1YS1R_BT601	0x02040199 /* BT601 coeff (b_cb = 516 / r_cr = 409) */
#define L1YS2R_BT601	0x006400D0 /* BT601 coeff (g_cb = 100 / g_cr = 208) */

#define L1YS1R_FR	0x01C60167 /* Full Range (b_cb = 454 / r_cr = 359) */
#define L1YS2R_FR	0x005800B7 /* Full Range (g_cb = 88 / g_cr = 183) */

enum ltdc_pix_fmt {
	PF_NONE,
	/* RGB formats */
	PF_ARGB8888,    /* ARGB [32 bits] */
	PF_RGBA8888,    /* RGBA [32 bits] */
	PF_RGB888,      /* RGB [24 bits] */
	PF_RGB565,      /* RGB [16 bits] */
	PF_ARGB1555,    /* ARGB A:1 bit RGB:15 bits [16 bits] */
	PF_ARGB4444,    /* ARGB A:4 bits R/G/B: 4 bits each [16 bits] */
	/* Indexed formats */
	PF_L8,          /* Indexed 8 bits [8 bits] */
	PF_AL44,        /* Alpha:4 bits + indexed 4 bits [8 bits] */
	PF_AL88         /* Alpha:8 bits + indexed 8 bits [16 bits] */
};

#define NB_PF           8       /* Max nb of std (not YCbCr) HW pixel format */

/* The index gives the encoding of the pixel format for an HW version */
static const enum ltdc_pix_fmt ltdc_pix_fmt_a0[NB_PF] = {
		PF_ARGB8888,	/* 0x00 */
		PF_RGB888,	/* 0x01 */
		PF_RGB565,	/* 0x02 */
		PF_ARGB1555,	/* 0x03 */
		PF_ARGB4444,	/* 0x04 */
		PF_L8,		/* 0x05 */
		PF_AL44,	/* 0x06 */
		PF_AL88		/* 0x07 */
};

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

static const u32 ycbcr_pix_fmt[] = {
		DRM_FORMAT_NV12, DRM_FORMAT_NV21, DRM_FORMAT_YUYV,
		DRM_FORMAT_YVYU, DRM_FORMAT_UYVY, DRM_FORMAT_VYUY
};

struct ltdc_plane {
	struct drm_plane base;
	unsigned int index;
};

static inline struct ltdc *crtc_to_ltdc(struct drm_crtc *crtc)
{
	return ((struct st_private *)crtc->dev->dev_private)->ltdc;
}

static inline struct ltdc *plane_to_ltdc(struct drm_plane *plane)
{
	return ((struct st_private *)plane->dev->dev_private)->ltdc;
}

static inline struct ltdc *encoder_to_ltdc(struct drm_encoder *encoder)
{
	return ((struct st_private *)encoder->dev->dev_private)->ltdc;
}

static inline struct ltdc *connector_to_ltdc(struct drm_connector *connector)
{
	return ((struct st_private *)connector->dev->dev_private)->ltdc;
}

static inline struct drm_panel *
connector_to_panel(struct drm_connector *connector)
{
	unsigned int i;
	struct ltdc *ltdc = connector_to_ltdc(connector);

	for (i = 0; i < MAX_PANELS; i++)
		if (ltdc->panel[i] && (ltdc->panel[i]->connector == connector))
			return ltdc->panel[i];

	return NULL;
}

static inline struct drm_panel *encoder_to_panel(struct drm_encoder *encoder)
{
	unsigned int i;
	struct ltdc *ltdc = encoder_to_ltdc(encoder);

	for (i = 0; i < MAX_PANELS; i++)
		if (ltdc->panel[i] && ltdc->panel[i]->connector &&
		    (ltdc->panel[i]->connector->encoder == encoder))
			return ltdc->panel[i];

	return NULL;
}

static inline u32 to_ltdc_pixelformat(u32 drm, struct ltdc *ltdc)
{
	enum ltdc_pix_fmt pf;
	unsigned int i;

	switch (drm) {
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_XRGB8888:
		pf = PF_ARGB8888;
		break;
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBX8888:
		pf = PF_RGBA8888;
		break;
	case DRM_FORMAT_RGB888:
		pf = PF_RGB888;
		break;
	case DRM_FORMAT_RGB565:
		pf = PF_RGB565;
		break;
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_XRGB1555:
		pf = PF_ARGB1555;
		break;
	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_XRGB4444:
		pf = PF_ARGB4444;
		break;
	case DRM_FORMAT_C8:
		pf = PF_L8;
		break;
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
		/* Use default (ignored since YUV mode is used) */
		pf = PF_ARGB8888;
		break;
	default:
		DRM_ERROR("unknown pixel format, return default ARGB8888\n");
		pf = PF_ARGB8888;
		break;
	/* Note: There are no DRM_FORMAT for AL44 and AL88 */
	}

	for (i = 0; i < NB_PF; i++) {
		if (ltdc->caps.pix_fmt_hw[i] == pf)
			return i;
	}

	DRM_ERROR("unsupported pixel format, return 0\n");
	return 0;
}

static inline u32 to_drm_pixelformat(enum ltdc_pix_fmt pf)
{
	switch (pf) {
	case PF_ARGB8888:
		return DRM_FORMAT_ARGB8888;
	case PF_RGBA8888:
		return DRM_FORMAT_RGBA8888;
	case PF_RGB888:
		return DRM_FORMAT_RGB888;
	case PF_RGB565:
		return DRM_FORMAT_RGB565;
	case PF_ARGB1555:
		return DRM_FORMAT_ARGB1555;
	case PF_ARGB4444:
		return DRM_FORMAT_ARGB4444;
	case PF_L8:
		return DRM_FORMAT_C8;
	case PF_AL44: /* No DRM support */
	case PF_AL88: /* No DRM support */
	case PF_NONE:
	default:
		return 0;
	}
}

static inline u32 get_ycbcr_config(u32 drm, bool yuv_restricted)
{
	u32 ycbcr_config;

	switch (drm) {
	case DRM_FORMAT_YUYV:
		ycbcr_config = YMODE_422I | OCYF_YUYV;
		break;
	case DRM_FORMAT_YVYU:
		ycbcr_config = YMODE_422I | OCYF_YVYU;
		break;
	case DRM_FORMAT_UYVY:
		ycbcr_config = YMODE_422I | OCYF_UYVY;
		break;
	case DRM_FORMAT_VYUY:
		ycbcr_config = YMODE_422I | OCYF_VYUY;
		break;
	case DRM_FORMAT_NV12:
		ycbcr_config = YMODE_420SP | OCYF_UYVY;
		break;
	case DRM_FORMAT_NV21:
		ycbcr_config = YMODE_420SP;
		break;
	default:
		/* Not a YUV format */
		return 0;
	}

	/* Enable limited range */
	if (yuv_restricted)
		ycbcr_config |= LXAFBCR_YHE;

	/* Enable conversion */
	ycbcr_config |= LXAFBCR_EN;

	return ycbcr_config;
}

static inline u32 remove_alpha(u32 drm)
{
	switch (drm) {
	case DRM_FORMAT_ARGB4444:
		return DRM_FORMAT_XRGB4444;
	case DRM_FORMAT_RGBA4444:
		return DRM_FORMAT_RGBX4444;
	case DRM_FORMAT_ARGB1555:
		return DRM_FORMAT_XRGB1555;
	case DRM_FORMAT_RGBA5551:
		return DRM_FORMAT_RGBX5551;
	case DRM_FORMAT_ARGB8888:
		return DRM_FORMAT_XRGB8888;
	case DRM_FORMAT_RGBA8888:
		return DRM_FORMAT_RGBX8888;
	default:
		return 0;
	}
}

/* ltdc_fb_to_bypp - determine the bytes per pixel */
static unsigned int ltdc_fb_to_bypp(struct drm_framebuffer *fb)
{
	switch (fb->pixel_format) {
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
		return 1; /* 1 byte per pixel for both planes */
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
		return 2;
	default:
		return fb->bits_per_pixel >> 3;
	}
}

static void ltdc_update_fps(struct ltdc *ltdc, enum fps_info_id id,
			    unsigned int paddr)
{
	struct fps_info *fpsi;

	fpsi = &ltdc->fpsi;

	/* if it is a fps for a layer */
	if (id < FPS_IRQ_RELOAD) {
		if ((paddr != fpsi->paddr[id]) ||
		    !fpsi->dont_count_duplicated_frame) {
			/* Compute number of frame updates */
			fpsi->frames[id]++;
			fpsi->paddr[id] = paddr;
		}
	} else {
		fpsi->frames[id]++;
	}

}

static struct drm_crtc *ltdc_crtc_from_pipe(struct drm_device *drm,
					    unsigned int pipe)
{
	struct drm_crtc *crtc;

	list_for_each_entry(crtc, &drm->mode_config.crtc_list, head) {
		if (pipe == drm_crtc_index(crtc))
			return crtc;
	}

	return NULL;
}

/* Enable/increment lcd_clk to read/write reg from pixel clock domain "dp" */
static inline int ltdc_get_dp_regs_clk(struct ltdc *ltdc)
{
	if (clk_prepare_enable(ltdc->lcd_clk)) {
		DRM_ERROR("Failed to enable pix clk\n");
		return -1;
	}

	return 0;
}

/* Disable/decrement lcd_clk after read/write reg from pixel clock domain */
static inline void ltdc_put_dp_regs_clk(struct ltdc *ltdc)
{
	clk_disable_unprepare(ltdc->lcd_clk);
}

/* Allow to know if another crtc is enabled */
static bool is_another_crtc_enabled(struct drm_crtc *crtc)
{
	struct drm_crtc *crtc_tmp;

	list_for_each_entry(crtc_tmp, &crtc->dev->mode_config.crtc_list, head) {
		if ((crtc_tmp != crtc) && crtc_tmp->enabled)
			return true;
	}

	return false;
}

/* Enable/increment lcd_clk to keep pixel clock domain for remote proc */
static inline int ltdc_get_rpc_pix_clk(struct ltdc *ltdc)
{
	return ltdc_get_dp_regs_clk(ltdc);
}

/* Disable/decrement lcd_clk to free pixel clock domain for remote proc */
static inline void ltdc_put_rpc_pix_clk(struct ltdc *ltdc)
{
	ltdc_put_dp_regs_clk(ltdc);
}

static inline bool ltdc_is_layer_locked(struct ltdc *ltdc, u32 lay_id)
{
	if (lay_id >= RPMSG_MM_NB_OVERLAY)
		return false;

	return ((ltdc->locked_layers_status & (1 << lay_id)) == (1 << lay_id));
}

static void ltdc_reset_layer_status(struct ltdc *ltdc, u32 lay_id)
{
	struct layer_info *lay_info;

	if (lay_id >= RPMSG_MM_NB_OVERLAY)
		return;

	lay_info = &ltdc->lay_infos[lay_id];
	lay_info->enabled = false;
	if (lay_info->fb)
		drm_framebuffer_unreference(lay_info->fb);
	lay_info->fb = NULL;
}

static void ltdc_reset_layers_status(struct ltdc *ltdc)
{
	int i;

	for (i = 0; i < RPMSG_MM_NB_OVERLAY; i++)
		ltdc_reset_layer_status(ltdc, i);
}

static void
ltdc_store_layer_infos(struct ltdc *ltdc, u32 lay_id, u32 crtc_id,
		       struct drm_framebuffer *fb,
		       u32 src_x, u32 src_y, u32 src_w, u32 src_h,
		       u32 x0, u32 y0, u32 x1, u32 y1)
{
	struct layer_info *lay_info;

	if (lay_id >= RPMSG_MM_NB_OVERLAY)
		return;

	lay_info = &ltdc->lay_infos[lay_id];
	lay_info->enabled = true;
	lay_info->src.x1 = src_x;
	lay_info->src.x1 = src_y;
	lay_info->src.x2 = src_x + src_w - 1;
	lay_info->src.y2 = src_y + src_h - 1;
	lay_info->pos.x1 = x0;
	lay_info->pos.y1 = y0;
	lay_info->pos.x2 = x1;
	lay_info->pos.y2 = y1;
	if (lay_info->fb)
		drm_framebuffer_unreference(lay_info->fb);
	drm_framebuffer_reference(fb);
	lay_info->fb = fb;
	lay_info->lay_id = lay_id;
	lay_info->crtc_id = crtc_id;
}

static void
ltdc_layer_update(struct ltdc *ltdc, u32 lay_id, u32 crtc_id,
		  struct drm_framebuffer *fb,
		  u32 src_x, u32 src_y, u32 src_w, u32 src_h,
		  u32 x0, u32 y0, u32 x1, u32 y1);

static void ltdc_restore_layer(struct ltdc *ltdc, u32 lay_id)
{
	struct layer_info *lay_info;

	if (lay_id >= RPMSG_MM_NB_OVERLAY)
		return;

	lay_info = &ltdc->lay_infos[lay_id];

	if (!lay_info->enabled || !lay_info->fb) {
		DRM_DEBUG_DRIVER(" NO restore of layer: enabled=%d, fb=%p\n",
				 lay_info->enabled, lay_info->fb);

		return;
	}

	DRM_DEBUG_DRIVER("restore layer %x\n", lay_id);
	ltdc_layer_update(ltdc, lay_id, lay_info->crtc_id, lay_info->fb,
			  lay_info->src.x1, lay_info->src.y1,
			  lay_info->src.x2 - lay_info->src.x1 + 1,
			  lay_info->src.y2 - lay_info->src.y1 + 1,
			  lay_info->pos.x1, lay_info->pos.y1,
			  lay_info->pos.x2, lay_info->pos.y2);

	/* Commit shadow registers = update planes at next vblank */
	reg_set(SRCR_VBR, LTDC_SRCR);
}

static void ltdc_send_vblank(struct ltdc *ltdc)
{
	struct drm_device *drm_dev = ltdc->drm_dev;
	struct drm_crtc *crtc = ltdc_crtc_from_pipe(drm_dev, 0);
	unsigned int pipe;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	list_for_each_entry(crtc, &drm_dev->mode_config.crtc_list, head) {
		pipe = drm_crtc_index(crtc);
		if (drm_dev->vblank[pipe].enabled)
			drm_crtc_handle_vblank(crtc);
	}
}

static void ltdc_update_gamma_registers(struct ltdc *ltdc)
{
	int i;

	if (ltdc_get_dp_regs_clk(ltdc))
		return;

	for (i = (CLUT_SIZE * 3 - 1); i >= 2; i = i - 3) {
		reg_w(ltdc->lut_table[i], LTDC_GACR); /* Blue */
		reg_w(ltdc->lut_table[i - 1], LTDC_GACR); /* Green */
		reg_w(ltdc->lut_table[i - 2], LTDC_GACR); /* Red */
	}

	/* Enable Gamma Correction */
	reg_clear_set(GCR_GAMEN, GCR_GAMEN, LTDC_GCR);

	ltdc_put_dp_regs_clk(ltdc);

	dev_dbg(ltdc->dev, "clut Applied\n");
}

static irqreturn_t ltdc_irq_thread(int irq, void *arg)
{
	struct ltdc *ltdc = arg;

	/* Register Reload IRQ */
	if (ltdc->irq_status & ISR_RRIF)
		ltdc_update_fps(ltdc, FPS_IRQ_RELOAD, 0);

	/* Line IRQ : trigger the vblank event */
	if (ltdc->irq_status & ISR_LIF) {
		mutex_lock(&ltdc->gamma_lock);
		if (ltdc->gamma_to_update) {
			ltdc_update_gamma_registers(ltdc);
			ltdc->gamma_to_update = false;
		}
		mutex_unlock(&ltdc->gamma_lock);
		ltdc_send_vblank(ltdc);
		ltdc_update_fps(ltdc, FPS_IRQ_LINE, 0);
	}

	/* Save FIFO Underrun & Transfer Error status */
	mutex_lock(&ltdc->err_lock);
	if (ltdc->irq_status & ISR_FUIF) {
		ltdc->error_status |= ISR_FUIF;
		ltdc->underrun_nb++;
	}
	if (ltdc->irq_status & ISR_TERRIF)
		ltdc->error_status |= ISR_TERRIF;
	mutex_unlock(&ltdc->err_lock);

	return IRQ_HANDLED;
}

static irqreturn_t ltdc_irq(int irq, void *arg)
{
	struct ltdc *ltdc = arg;

	/* Read & Clear the interrupt status */
	ltdc->irq_status = reg_r(LTDC_ISR);
	reg_w(ltdc->irq_status, LTDC_ICR);

	return IRQ_WAKE_THREAD;
}

/*
 * DRM_CRTC
 */

static void ltdc_crtc_load_cmap(struct drm_crtc *crtc)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);
	unsigned int i, lay;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	if (ltdc_get_dp_regs_clk(ltdc))
		return;

	for (lay = 0; lay < ltdc->caps.nb_layers; lay++)
		for (i = 0; i < 256; i++)
			reg_w(ltdc->clut[i], LTDC_L1CLUTWR + lay * LAY_OFFSET);

	ltdc_put_dp_regs_clk(ltdc);
}

static void ltdc_crtc_update_clut_if_needed(struct drm_crtc *crtc)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);
	struct drm_color_lut *lut;
	int i;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	if (likely(!crtc->state->color_mgmt_changed ||
		   !crtc->state->gamma_lut || !crtc->enabled))
		return;

	lut = (struct drm_color_lut *)crtc->state->gamma_lut->data;

	mutex_lock(&ltdc->gamma_lock);
	for (i = 0; i < min_t(u32, CLUT_SIZE, crtc->state->gamma_lut->length);
	     i++, lut++) {
		ltdc->lut_table[i * 3] =
			GACR_RED | ((lut->red << 8) & GACR_MCOLOR) | i;
		ltdc->lut_table[i * 3 + 1] =
			GACR_GREEN | ((lut->green << 8) & GACR_MCOLOR) | i;
		ltdc->lut_table[i * 3 + 2] =
			GACR_BLUE | ((lut->blue << 8) & GACR_MCOLOR) | i;
	}

	ltdc->gamma_to_update = true;
	mutex_unlock(&ltdc->gamma_lock);
}

static void ltdc_crtc_enable(struct drm_crtc *crtc)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);

	DRM_DEBUG_DRIVER("\n");

	mutex_lock(&ltdc->layers_info_lock);
	ltdc->ltdc_disable_waiting = false;
	mutex_unlock(&ltdc->layers_info_lock);

	/* Enable LTDC only one time, check if already enabled */
	if (!is_another_crtc_enabled(crtc)) {
		/* Enable pixel clock */
		if (clk_prepare_enable(ltdc->lcd_clk))
			DRM_ERROR("Failed to enable pix clk\n");

		/* Sets the background color value */
		reg_w(BCCR_BCBLUE, LTDC_BCCR);

		/* Enable IRQ */
		reg_set(IER_RRIE | IER_FUIE | IER_TERRIE, LTDC_IER);

		/* Commit shadow registers = update planes at next vblank */
		reg_set(SRCR_VBR, LTDC_SRCR);

		/* Enable LTDC */
		reg_clear_set(GCR_LTDCEN, GCR_LTDCEN, LTDC_GCR);
	}

	drm_crtc_vblank_on(crtc);
}

static void ltdc_crtc_disable(struct drm_crtc *crtc)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);
	unsigned int i;

	DRM_DEBUG_DRIVER("\n");

	if (!crtc->enabled) {
		DRM_DEBUG_DRIVER("already disabled\n");
		return;
	}

	drm_crtc_vblank_off(crtc);

	ltdc_reset_layers_status(ltdc);

	/* Check that all others crtc are disabled before disabling LTDC */
	if (is_another_crtc_enabled(crtc))
		return;

	/* Don't disable CRTC if a layer is used by remote proc */
	mutex_lock(&ltdc->layers_info_lock);
	if (ltdc->locked_layers_status != 0) {
		ltdc->ltdc_disable_waiting = true;
		mutex_unlock(&ltdc->layers_info_lock);
		return;
	}
	mutex_unlock(&ltdc->layers_info_lock);

	/* disable LTDC */
	reg_clear(GCR_LTDCEN, LTDC_GCR);

	/* disable IRQ */
	reg_clear(IER_RRIE | IER_FUIE | IER_TERRIE, LTDC_IER);

	/* disable layers */
	for (i = 0; i < ltdc->caps.nb_layers; i++)
		reg_clear(LXCR_LEN, LTDC_L1CR + i * LAY_OFFSET);

	/* immediately commit disable of layers before switching off LTDC */
	reg_w(SRCR_IMR, LTDC_SRCR);

	/* disable pixel clock */
	clk_disable_unprepare(ltdc->lcd_clk);
}

static bool ltdc_crtc_mode_fixup(struct drm_crtc *crtc,
				 const struct drm_display_mode *mode,
				 struct drm_display_mode *adjusted_mode)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);

	dev_dbg(ltdc->dev, "%s\n", __func__);
	/* accept the provided drm_display_mode, do not fix it up */
	return true;
}

static u32 ltdc_get_paddr(struct drm_framebuffer *fb, int x, int y,
			  unsigned int plane)
{
	struct drm_gem_cma_object *cma_obj;
	unsigned int pixsize;
	u32 paddr;

	cma_obj = drm_fb_cma_get_gem_obj(fb, 0);
	if (IS_ERR(cma_obj)) {
		DRM_ERROR("Can't get CMA GEM object for fb\n");
		return 0;
	}

	if (plane >= drm_format_num_planes(fb->pixel_format)) {
		DRM_ERROR("Invalid memory plane (%d)\n", plane);
		return 0;
	}

	pixsize = ltdc_fb_to_bypp(fb);

	paddr = (u32)cma_obj->paddr + fb->offsets[plane];
	paddr += x * pixsize + y * fb->pitches[plane];

	return paddr;
}

static void ltdc_crtc_mode_set_nofb(struct drm_crtc *crtc)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	struct videomode vm;
	int rate = mode->clock * 1000;
	u32 hsync, vsync, accum_hbp, accum_vbp, accum_act_w, accum_act_h;
	u32 total_width, total_height;
	u32 val;

	DRM_DEBUG_DRIVER("CRTC:%d mode:%s\n", crtc->base.id, mode->name);

	/* if one crtc is enabled, ltdc is already configured, do not configure
	 * it again, only one timings configuration is allowed at a time
	 */
	if (is_another_crtc_enabled(crtc))
		return;

	dev_dbg(ltdc->dev,
		"DRM mode: %dx%d (tot %dx%d) hblank %d-%d hsync %d-%d vblank %d-%d vsync %d-%d\n",
		mode->crtc_hdisplay, mode->crtc_vdisplay,
		mode->crtc_htotal, mode->crtc_vtotal,
		mode->crtc_hblank_start, mode->crtc_hblank_end,
		mode->crtc_hsync_start, mode->crtc_hsync_end,
		mode->crtc_vblank_start, mode->crtc_vblank_end,
		mode->crtc_vsync_start, mode->crtc_vsync_end);

	drm_display_mode_to_videomode(mode, &vm);
	dev_dbg(ltdc->dev,
		"Video mode: %dx%d hfp %d hbp %d hsl %d vfp %d vbp %d vsl %d\n",
		vm.hactive, vm.vactive,
		vm.hfront_porch, vm.hback_porch, vm.hsync_len,
		vm.vfront_porch, vm.vback_porch, vm.vsync_len);

	/* double horizontal timings and input pixel clock for dual_display */
	if (ltdc->dual_display) {
		vm.hactive *= 2;
		vm.hback_porch *= 2;
		vm.hfront_porch *= 2;
		vm.hsync_len *= 2;
		rate *= 2;
	}

	/* convert video timings to ltdc timings */
	hsync = vm.hsync_len - 1;
	vsync = vm.vsync_len - 1;
	accum_hbp = hsync + vm.hback_porch;
	accum_vbp = vsync + vm.vback_porch;
	accum_act_w = accum_hbp + vm.hactive;
	accum_act_h = accum_vbp + vm.vactive;
	total_width = accum_act_w + vm.hfront_porch;
	total_height = accum_act_h + vm.vfront_porch;

	dev_dbg(ltdc->dev,
		"ltdc mode (acc): hs %d hbp %d actw %d totw %d vs %d vbp %d acth %d toth %d\n",
		hsync, accum_hbp,  accum_act_w, total_width,
		vsync, accum_vbp, accum_act_h, total_height);

	/* Don't/Can't set rate if remote proc is using LTDC */
	mutex_lock(&ltdc->layers_info_lock);
	if (!ltdc->locked_layers_status) {
		if (clk_set_rate(ltdc->lcd_clk, rate) < 0) {
			DRM_ERROR("Cannot set rate (%dHz) for pix clk\n", rate);
			mutex_unlock(&ltdc->layers_info_lock);
			return;
		}
	} else {
		DRM_DEBUG_DRIVER("Don't set rate as LTDC used by remote proc\n"
				 );
	}
	mutex_unlock(&ltdc->layers_info_lock);

	if (ltdc_get_dp_regs_clk(ltdc))
		return;

	/* Configures HSYNC, VSYNC, DE polarities and Pixel Clock edge */
	val = 0;

	/* HSYNC active low by default (0) */
	if (vm.flags & DISPLAY_FLAGS_HSYNC_HIGH)
		val |= GCR_HSPOL;

	/* VSYNC active low by default (0) */
	if (vm.flags & DISPLAY_FLAGS_VSYNC_HIGH)
		val |= GCR_VSPOL;

	/* DE active high by default (0) */
	if (vm.flags & DISPLAY_FLAGS_DE_LOW)
		val |= GCR_DEPOL;

	/* Pixel Clock on positive edge (no inversion) by default (0) */
	if (vm.flags & DISPLAY_FLAGS_PIXDATA_NEGEDGE)
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

	/* Sets Line Interrupt Position */
	val = accum_act_h + 1;
	reg_w(val, LTDC_LIPCR);

	/* Sets total width & height */
	val = (total_width << 16) | total_height;
	reg_clear_set(TWCR_TOTALH | TWCR_TOTALW, val, LTDC_TWCR);

	/* Set External Display Control for dual display */
	if (ltdc->dual_display) {
		val =  EDCR_EHFPCE | EDCR_OHFPCE | EDCR_HPCAES;
		reg_clear_set(val, val, LTDC_EDCR);
	}

	ltdc_put_dp_regs_clk(ltdc);
}

static void ltdc_crtc_atomic_flush(struct drm_crtc *crtc,
				   struct drm_crtc_state *old_crtc_state)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);
	struct drm_pending_vblank_event *event;
	unsigned long flags;

	DRM_DEBUG_DRIVER("\n");

	ltdc_crtc_update_clut_if_needed(crtc);

	/* Commit shadow registers = update planes at next vblank */
	reg_set(SRCR_VBR, LTDC_SRCR);

	event = crtc->state->event;
	if (event) {
		crtc->state->event = NULL;

		spin_lock_irqsave(&crtc->dev->event_lock, flags);
		if (drm_crtc_vblank_get(crtc) == 0)
			drm_crtc_arm_vblank_event(crtc, event);
		else
			drm_crtc_send_vblank_event(crtc, event);
		spin_unlock_irqrestore(&crtc->dev->event_lock, flags);
	}
}

static struct drm_crtc_helper_funcs ltdc_crtc_helper_funcs = {
	.enable = ltdc_crtc_enable,
	.disable = ltdc_crtc_disable,
	.mode_fixup = ltdc_crtc_mode_fixup,
	.mode_set_nofb = ltdc_crtc_mode_set_nofb,
	.atomic_flush = ltdc_crtc_atomic_flush,
};

static void ltdc_crtc_cmap_set(struct drm_crtc *crtc,
			       u16 *red, u16 *green, u16 *blue,
			       u32 start, u32 size)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);
	unsigned int end = (start + size > 256) ? 256 : start + size;
	unsigned int i;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	if (start > 255) {
		DRM_ERROR("Warning: color map index out of range\n");
		return;
	}

	for (i = start; i < end; i++)
		ltdc->clut[i] = RGB_TO_CLUT(i, red[i], green[i], blue[i]);

	ltdc_crtc_load_cmap(crtc);
}

static void ltdc_crtc_destroy(struct drm_crtc *crtc)
{
	struct ltdc *ltdc = crtc_to_ltdc(crtc);

	dev_dbg(ltdc->dev, "%s\n", __func__);
	drm_crtc_cleanup(crtc);
}

int ltdc_crtc_enable_vblank(struct drm_device *dev, unsigned int crtc_id)
{
	struct st_private *private = dev->dev_private;
	struct ltdc *ltdc = private->ltdc;

	DRM_DEBUG_DRIVER("\n");
	reg_set(IER_LIE, LTDC_IER);
	return 0;
}

void ltdc_crtc_disable_vblank(struct drm_device *dev, unsigned int crtc_id)
{
	struct st_private *private = dev->dev_private;
	struct ltdc *ltdc = private->ltdc;
	struct drm_crtc *crtc;

	DRM_DEBUG_DRIVER("\n");

	/* Check that all others vblank are disabled to disable IP Interrupt */
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		if (crtc_id == drm_crtc_index(crtc))
			continue;

		if (dev->vblank[drm_crtc_index(crtc)].enabled)
			return;
	}

	reg_clear(IER_LIE, LTDC_IER);
}

static struct drm_crtc_funcs ltdc_crtc_funcs = {
	.cmap_set = ltdc_crtc_cmap_set,
	.gamma_set = drm_atomic_helper_legacy_gamma_set,
	.set_config = drm_atomic_helper_set_config,
	.page_flip = drm_atomic_helper_page_flip,
	.destroy = ltdc_crtc_destroy,
	.reset = drm_atomic_helper_crtc_reset,
	.atomic_duplicate_state = drm_atomic_helper_crtc_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_crtc_destroy_state,
};

/*
 * DRM_PLANE
 */
static void
ltdc_layer_update(struct ltdc *ltdc, u32 lay_id, u32 crtc_id,
		  struct drm_framebuffer *fb,
		  u32 src_x, u32 src_y, u32 src_w, u32 src_h,
		  u32 x0, u32 y0, u32 x1, u32 y1)
{
	u32 val, pitch_in_bytes, line_length, paddr;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	line_length = (x1 - x0 + 1) * ltdc_fb_to_bypp(fb) +
		      (ltdc->bus_width >> 3) - 1;

	x1++;

	/* if dual display, double horizontal position */
	if (ltdc->dual_display) {
		x0 *= 2;
		x1 *= 2;
	}

	if (ltdc_get_dp_regs_clk(ltdc))
		return;

	/* Configures the horizontal start and stop position */
	val = ((x1 + ((reg_r(LTDC_BPCR) & BPCR_AHBP) >> 16)) << 16);
	val |= (x0 + 1 + ((reg_r(LTDC_BPCR) & BPCR_AHBP) >> 16));

	reg_clear_set(LXWHPCR_WHSTPOS | LXWHPCR_WHSPPOS, val,
		      LTDC_L1WHPCR + lay_id * LAY_OFFSET);

	/* Configures the vertical start and stop position */
	val = ((y1 + 1 + (reg_r(LTDC_BPCR) & BPCR_AVBP)) << 16);
	val |= (y0 + 1 + (reg_r(LTDC_BPCR) & BPCR_AVBP));
	reg_clear_set(LXWVPCR_WVSTPOS | LXWVPCR_WVSPPOS, val,
		      LTDC_L1WVPCR + lay_id * LAY_OFFSET);

	/* Specifies the pixel format */
	val = to_ltdc_pixelformat(fb->pixel_format, ltdc);
	reg_clear_set(LXPFCR_PF, val, LTDC_L1PFCR + lay_id * LAY_OFFSET);

	/* Configures the YUV conversion */
	val = get_ycbcr_config(fb->pixel_format,
			       !(fb->flags & DRM_MODE_FB_YFCR));
	reg_clear_set(LXAFBCR_YCBCR, val, LTDC_L1AFBCR + lay_id * LAY_OFFSET);

	if (val) {
		if (fb->flags & DRM_MODE_FB_YFCR) {
			/* Configures YUV Full Range to RGB scale coefs */
			reg_w(L1YS1R_FR, LTDC_L1YS1R + lay_id * LAY_OFFSET);
			reg_w(L1YS2R_FR, LTDC_L1YS2R + lay_id * LAY_OFFSET);
		} else {
			/* Configures YUV Reduced Range to RGB scale coefs */
			reg_w(L1YS1R_BT601, LTDC_L1YS1R + lay_id * LAY_OFFSET);
			reg_w(L1YS2R_BT601, LTDC_L1YS2R + lay_id * LAY_OFFSET);
		}
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
	dev_dbg(ltdc->dev, "fb: phys 0x%08x", paddr);
	reg_w(paddr, LTDC_L1CFBAR + lay_id * LAY_OFFSET);

	/* Configure second plane */
	if (drm_format_num_planes(fb->pixel_format) >= 2) {
		/* Configures the Aux FB address */
		paddr = ltdc_get_paddr(fb, src_x, src_y, 1);
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
	val = fb->pixel_format == DRM_FORMAT_C8 ? LXCR_CLUTEN : 0;
	val |= LXCR_LEN;

	if (ltdc->dual_display) {
		val |= crtc_id ? LXCR_INSODD : LXCR_INSEVEN;
		reg_clear_set(LXCR_LEN | LXCR_CLUTEN | LXCR_INSMODE, val,
			      LTDC_L1CR + lay_id * LAY_OFFSET);
	} else {
		reg_clear_set(LXCR_LEN | LXCR_CLUTEN, val,
			      LTDC_L1CR + lay_id * LAY_OFFSET);
	}

	ltdc_put_dp_regs_clk(ltdc);
}

static int ltdc_plane_atomic_check(struct drm_plane *plane,
				   struct drm_plane_state *state)
{
	struct ltdc *ltdc = plane_to_ltdc(plane);
	struct drm_framebuffer *fb = state->fb;
	u32 src_x, src_y, src_w, src_h;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	if (!fb)
		return 0;

	/* convert src_ from 16:16 format */
	src_x = state->src_x >> 16;
	src_y = state->src_y >> 16;
	src_w = state->src_w >> 16;
	src_h = state->src_h >> 16;

	/* Reject scaling */
	if ((src_w != state->crtc_w) || (src_h != state->crtc_h)) {
		DRM_ERROR("Scaling is not supported");
		return -EINVAL;
	}

	if (!ltdc_get_paddr(fb, src_x, src_y, 0)) {
		DRM_ERROR("Cannot get fb address\n");
		return -EFAULT;
	}

	if ((drm_format_num_planes(fb->pixel_format) >= 2) &&
	    (!ltdc_get_paddr(fb, src_x, src_y, 1))) {
		DRM_ERROR("Cannot get fb address (plane 1)\n");
		return -EFAULT;
	}

	return 0;
}

static void ltdc_plane_atomic_update(struct drm_plane *plane,
				     struct drm_plane_state *oldstate)
{
	struct ltdc *ltdc = plane_to_ltdc(plane);
	struct ltdc_plane *ltdc_plane = (struct ltdc_plane *)plane;
	struct drm_plane_state *state = plane->state;
	struct drm_framebuffer *fb = state->fb;
	u32 lay_id = ltdc_plane->index;
	u32 x0 = state->crtc_x;
	u32 x1 = state->crtc_x + state->crtc_w - 1;
	u32 y0 = state->crtc_y;
	u32 y1 = state->crtc_y + state->crtc_h - 1;
	u32 src_x, src_y, src_w, src_h;
	u32 crtc_id;
	u32 paddr;

	if (!state->crtc || !fb) {
		DRM_DEBUG_DRIVER("fb or crtc NULL");
		return;
	}

	/* convert src_ from 16:16 format */
	src_x = state->src_x >> 16;
	src_y = state->src_y >> 16;
	src_w = state->src_w >> 16;
	src_h = state->src_h >> 16;

	DRM_DEBUG_DRIVER(
		"plane:%d fb:%d (%dx%d)@(%d,%d) -> (%dx%d)@(%d,%d)\n",
		plane->base.id, fb->base.id,
		src_w, src_h, src_x, src_y,
		state->crtc_w, state->crtc_h, state->crtc_x, state->crtc_y);

	crtc_id = drm_crtc_index(state->crtc);
	mutex_lock(&ltdc->layers_info_lock);
	ltdc_store_layer_infos(ltdc, lay_id, crtc_id, fb, src_x, src_y, src_w,
			       src_h, x0, y0, x1, y1);

	/* Check that layer is not locked by remote proc */
	if (likely(!ltdc_is_layer_locked(ltdc, lay_id)))
		ltdc_layer_update(ltdc, lay_id, crtc_id, fb, src_x, src_y,
				  src_w, src_h, x0, y0, x1, y1);
	else
		DRM_DEBUG_DRIVER("layer %d (from 0) not updated, locked\n",
				 lay_id);

	mutex_unlock(&ltdc->layers_info_lock);

	paddr = ltdc_get_paddr(fb, src_x, src_y, 0);
	ltdc_update_fps(ltdc, lay_id, paddr);

	mutex_lock(&ltdc->err_lock);
	if (ltdc->error_status & ISR_FUIF) {
		dev_dbg(ltdc->dev, "Fifo underrun\n");
		ltdc->error_status &= ~ISR_FUIF;
	}
	if (ltdc->error_status & ISR_TERRIF) {
		dev_dbg(ltdc->dev, "Transfer error\n");
		ltdc->error_status &= ~ISR_TERRIF;
	}
	mutex_unlock(&ltdc->err_lock);
}

static void ltdc_plane_atomic_disable(struct drm_plane *plane,
				      struct drm_plane_state *oldstate)
{
	struct ltdc *ltdc = plane_to_ltdc(plane);
	struct ltdc_plane *ltdc_plane = (struct ltdc_plane *)plane;
	u32 lay_id = ltdc_plane->index;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	if (!oldstate->crtc) {
		DRM_DEBUG_DRIVER("drm plane:%d not enabled\n", plane->base.id);
		return;
	}

	mutex_lock(&ltdc->layers_info_lock);
	ltdc_reset_layer_status(ltdc, lay_id);

	/* Check if layer is locked by remote proc */
	if (ltdc_is_layer_locked(ltdc, lay_id)) {
		DRM_DEBUG_DRIVER("layer %d (from 0) not disabled, locked\n",
				 lay_id);
		mutex_unlock(&ltdc->layers_info_lock);
		return;
	}

	mutex_unlock(&ltdc->layers_info_lock);

	if (ltdc_get_dp_regs_clk(ltdc))
		return;

	/* disable layer */
	reg_clear(LXCR_LEN, LTDC_L1CR + lay_id * LAY_OFFSET);

	ltdc_put_dp_regs_clk(ltdc);

	DRM_DEBUG_DRIVER("CRTC:%d plane:%d\n",
			 oldstate->crtc->base.id, plane->base.id);
}

/* This function is a forked version of drm_atomic_helper_update_plane.
 * Here we run atomic_commit in nonblocking mode
 */
int ltdc_plane_async_update(struct drm_plane *plane, struct drm_crtc *crtc,
			    struct drm_framebuffer *fb, int crtc_x, int crtc_y,
			    unsigned int crtc_w, unsigned int crtc_h,
			    u32 src_x, u32 src_y, u32 src_w, u32 src_h)
{
	struct drm_atomic_state *state;
	struct drm_plane_state *plane_state;
	struct st_private *private = plane->dev->dev_private;
	int ret = 0;

	state = drm_atomic_state_alloc(plane->dev);
	if (!state)
		return -ENOMEM;

	state->acquire_ctx = drm_modeset_legacy_acquire_ctx(crtc);
retry:
	plane_state = drm_atomic_get_plane_state(state, plane);
	if (IS_ERR(plane_state)) {
		ret = PTR_ERR(plane_state);
		goto fail;
	}

	ret = drm_atomic_set_crtc_for_plane(plane_state, crtc);
	if (ret != 0)
		goto fail;
	drm_atomic_set_fb_for_plane(plane_state, fb);
	plane_state->crtc_x = crtc_x;
	plane_state->crtc_y = crtc_y;
	plane_state->crtc_h = crtc_h;
	plane_state->crtc_w = crtc_w;
	plane_state->src_x = src_x;
	plane_state->src_y = src_y;
	plane_state->src_h = src_h;
	plane_state->src_w = src_w;

	if ((plane == crtc->cursor) || !private->filp->atomic)
		state->legacy_cursor_update = true;

	ret = drm_atomic_commit(state);
	if (ret != 0)
		goto fail;

	/* Driver takes ownership of state on successful commit. */
	return 0;
fail:
	if (ret == -EDEADLK)
		goto backoff;

	drm_atomic_state_free(state);

	return ret;
backoff:
	drm_atomic_state_clear(state);
	drm_atomic_legacy_backoff(state);

	/*
	 * Someone might have exchanged the framebuffer while we dropped locks
	 * in the backoff code. We need to fix up the fb refcount tracking the
	 * core does for us.
	 */
	plane->old_fb = plane->fb;

	goto retry;
}

static void ltdc_plane_destroy(struct drm_plane *plane)
{
	struct ltdc *ltdc = plane_to_ltdc(plane);

	dev_dbg(ltdc->dev, "%s\n", __func__);

	drm_plane_helper_disable(plane);
	drm_plane_cleanup(plane);
}

static struct drm_plane_funcs ltdc_plane_funcs = {
	.update_plane = ltdc_plane_async_update,
	.disable_plane = drm_atomic_helper_disable_plane,
	.destroy = ltdc_plane_destroy,
	.reset = drm_atomic_helper_plane_reset,
	.atomic_duplicate_state = drm_atomic_helper_plane_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_plane_destroy_state,
};

static const struct drm_plane_helper_funcs ltdc_plane_helper_funcs = {
	.atomic_check = ltdc_plane_atomic_check,
	.atomic_update = ltdc_plane_atomic_update,
	.atomic_disable = ltdc_plane_atomic_disable,
};

static struct drm_plane *ltdc_plane_create(struct drm_device *drm_dev,
					   enum drm_plane_type type,
					   unsigned int index)
{
	struct st_private *private = drm_dev->dev_private;
	struct ltdc *ltdc = private->ltdc;
	unsigned long possible_crtcs = GENMASK(ltdc->nb_crtc - 1, 0);
	struct ltdc_plane *ltdc_plane;
	unsigned int i, nb_fmt = 0;
	u32 drm_fmt, drm_fmt_no_alpha;
	int err;
	/* Std pixel formats (w/ and w/o alpha) + YCbCr ones*/
	u32 formats[2 * NB_PF + ARRAY_SIZE(ycbcr_pix_fmt)];

	dev_dbg(ltdc->dev, "%s\n", __func__);

	/* Get supported std (non-YCbCr) pixel formats */
	for (i = 0; i < NB_PF; i++) {
		drm_fmt = to_drm_pixelformat(ltdc->caps.pix_fmt_hw[i]);
		if (!drm_fmt)
			continue;
		formats[nb_fmt++] = drm_fmt;

		drm_fmt_no_alpha = remove_alpha(drm_fmt);
		if (drm_fmt_no_alpha)
			formats[nb_fmt++] = drm_fmt_no_alpha;
	}

	/* Add YCbCr supported pixel formats */
	if (ltdc->caps.ycbcr_support[index]) {
		memcpy(&formats[nb_fmt], ycbcr_pix_fmt, sizeof(ycbcr_pix_fmt));
		nb_fmt += ARRAY_SIZE(ycbcr_pix_fmt);
	}

	ltdc_plane = devm_kzalloc(ltdc->dev, sizeof(*ltdc_plane), GFP_KERNEL);
	if (!ltdc_plane)
		return 0;

	ltdc_plane->index = index;
	err = drm_universal_plane_init(drm_dev, &ltdc_plane->base,
				       possible_crtcs, &ltdc_plane_funcs,
				       formats, nb_fmt, type, NULL);
	if (err < 0)
		return 0;

	drm_plane_helper_add(&ltdc_plane->base, &ltdc_plane_helper_funcs);

	DRM_DEBUG_DRIVER("plane:%d created\n", ltdc_plane->base.base.id);

	return &ltdc_plane->base;
}

static void ltdc_plane_destroy_all(struct ltdc *ltdc)
{
	struct drm_plane *plane, *plane_tmp;

	list_for_each_entry_safe(plane, plane_tmp,
				 &ltdc->drm_dev->mode_config.plane_list, head)
		drm_plane_cleanup(plane);
}

static int ltdc_crtcs_init(struct drm_device *drm_dev, struct drm_crtc *crtc[])
{
	struct st_private *private = drm_dev->dev_private;
	struct ltdc *ltdc = private->ltdc;
	struct drm_plane *primary, *overlay[MAX_PLANES];
	unsigned int i;
	int res;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	/* Initialize crtc(s) and create primary plane(s) */
	for (i = 0; i < min_t(u32, ltdc->nb_crtc, ltdc->caps.nb_layers); i++) {
		primary = ltdc_plane_create(drm_dev, DRM_PLANE_TYPE_PRIMARY, i);
		if (!primary) {
			DRM_ERROR("Can not create primary plane\n");
			res = -ENOMEM;
			goto cleanup;
		}

		res = drm_crtc_init_with_planes(drm_dev, crtc[i], primary, NULL,
						&ltdc_crtc_funcs, NULL);
		if (res) {
			DRM_ERROR("Can not initialize CRTC\n");
			goto cleanup;
		}

		drm_crtc_helper_add(crtc[i], &ltdc_crtc_helper_funcs);
		drm_mode_crtc_set_cmap_size(crtc[i], 256);
		drm_mode_crtc_set_gamma_size(crtc[i], CLUT_SIZE);
		drm_crtc_enable_color_mgmt(crtc[i], 0, false, CLUT_SIZE);

		DRM_DEBUG_DRIVER("CRTC:%d created\n", crtc[i]->base.id);
	}

	/* Add planes for overlay */
	for (i = ltdc->nb_crtc; i < ltdc->caps.nb_layers; i++) {
		overlay[i - ltdc->nb_crtc] = ltdc_plane_create(drm_dev,
						   DRM_PLANE_TYPE_OVERLAY, i);
		if (!overlay[i - ltdc->nb_crtc]) {
			res = -ENOMEM;
			DRM_ERROR("Can not create overlay plane %d\n", i);
			goto cleanup;
		}
	}

	return 0;

cleanup:
	ltdc_plane_destroy_all(ltdc);
	return res;
}

/*
 * DRM_ENCODER
 */

static void ltdc_rgb_encoder_enable(struct drm_encoder *encoder)
{
	struct drm_panel *panel = encoder_to_panel(encoder);

	DRM_DEBUG_DRIVER("\n");

	drm_panel_prepare(panel);
	drm_panel_enable(panel);
}

static void ltdc_rgb_encoder_disable(struct drm_encoder *encoder)
{
	struct drm_panel *panel = encoder_to_panel(encoder);

	DRM_DEBUG_DRIVER("\n");

	drm_panel_disable(panel);
	drm_panel_unprepare(panel);
}

static const struct drm_encoder_helper_funcs ltdc_rgb_encoder_helper_funcs = {
	.enable = ltdc_rgb_encoder_enable,
	.disable = ltdc_rgb_encoder_disable,
};

static void ltdc_rgb_encoder_destroy(struct drm_encoder *encoder)
{
	struct ltdc *ltdc = encoder_to_ltdc(encoder);

	dev_dbg(ltdc->dev, "%s\n", __func__);

	drm_encoder_cleanup(encoder);
}

static const struct drm_encoder_funcs ltdc_rgb_encoder_funcs = {
	.destroy = ltdc_rgb_encoder_destroy,
};

static struct drm_encoder *ltdc_rgb_create_encoder(struct ltdc *ltdc,
						   unsigned int panel_id,
						   struct drm_panel *panel)
{
	struct drm_encoder *encoder;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	encoder = devm_kzalloc(ltdc->dev, sizeof(*encoder), GFP_KERNEL);
	if (!encoder)
		return NULL;

	if (ltdc->dual_display)
		/* Allow only one crtc to an encoder */
		encoder->possible_crtcs = 1 << panel_id;
	else
		encoder->possible_crtcs = GENMASK(ltdc->nb_crtc - 1, 0);

	encoder->possible_clones = 0; /* No cloning support */

	drm_encoder_init(ltdc->drm_dev, encoder, &ltdc_rgb_encoder_funcs,
			 DRM_MODE_ENCODER_LVDS, NULL);

	drm_encoder_helper_add(encoder, &ltdc_rgb_encoder_helper_funcs);

	DRM_DEBUG_DRIVER("RGB encoder:%d created\n", encoder->base.id);

	return encoder;
}

static void ltdc_rgb_encoders_destroy(struct ltdc *ltdc)
{
	struct drm_encoder *encoder, *enc_tmp;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	list_for_each_entry_safe(encoder, enc_tmp,
				 &ltdc->drm_dev->mode_config.encoder_list,
				 head) {
		if (encoder->encoder_type == DRM_MODE_ENCODER_LVDS)
			ltdc_rgb_encoder_destroy(encoder);
	}
}

/*
 * DRM_CONNECTOR
 */

static int ltdc_connector_get_modes(struct drm_connector *connector)
{
	struct ltdc *ltdc = connector_to_ltdc(connector);
	int ret;
	struct drm_panel *panel;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	panel = connector_to_panel(connector);
	if (!panel)
		return 0;

	ret = drm_panel_get_modes(panel);

	return ret < 0 ? 0 : ret;
}

#define CLK_TOLERANCE_PCT 10

static enum drm_mode_status
ltdc_connector_mode_valid(struct drm_connector *connector,
			  struct drm_display_mode *mode)
{
	struct ltdc *ltdc = connector_to_ltdc(connector);
	int target = mode->clock * 1000;
	int target_min;
	int target_max;
	int result;

	/* doubled input pixel clock in case of dual display */
	if (ltdc->dual_display)
		target *= 2;

	target_min = target - (target * CLK_TOLERANCE_PCT) / 100;
	target_max = target + (target * CLK_TOLERANCE_PCT) / 100;

	result = clk_round_rate(ltdc->lcd_clk, target);

	DRM_DEBUG_DRIVER("Target rate = %d => available rate = %d\n",
			 target, result);

	if ((result < target_min) || (result > target_max)) {
		DRM_DEBUG_DRIVER("Rate not supported\n");
		return MODE_BAD;
	}

	return MODE_OK;
}

static struct drm_encoder *
ltdc_connector_best_encoder(struct drm_connector *connector)
{
	int enc_id = connector->encoder_ids[0];

	return enc_id ? drm_encoder_find(connector->dev, enc_id) : NULL;
}

static struct drm_connector_helper_funcs ltdc_connector_helper_funcs = {
	.get_modes = ltdc_connector_get_modes,
	.mode_valid = ltdc_connector_mode_valid,
	.best_encoder = ltdc_connector_best_encoder,
};

static enum drm_connector_status
ltdc_connector_detect(struct drm_connector *connector, bool force)
{
	struct ltdc *ltdc = connector_to_ltdc(connector);
	struct drm_panel *panel;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	panel = connector_to_panel(connector);

	if (!panel) {
		DRM_DEBUG_DRIVER("connector status disconnected");
		return connector_status_disconnected;
	}

	DRM_DEBUG_DRIVER("connector status connected");

	return connector_status_connected;
}

static void ltdc_connector_destroy(struct drm_connector *connector)
{
	struct ltdc *ltdc = connector_to_ltdc(connector);
	struct drm_panel *panel;

	dev_dbg(ltdc->dev, "%s\n", __func__);

	panel = connector_to_panel(connector);

	if (panel)
		drm_panel_detach(panel);

	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);
}

static const struct drm_connector_funcs ltdc_connector_funcs = {
	.dpms = drm_atomic_helper_connector_dpms,
	.fill_modes = drm_helper_probe_single_connector_modes,
	.detect = ltdc_connector_detect,
	.destroy = ltdc_connector_destroy,
	.reset = drm_atomic_helper_connector_reset,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
};

/*
 * PROBE & BINDING
 */

static int ltdc_bind(struct device *dev, struct device *master, void *data)
{
	struct ltdc *ltdc = dev_get_drvdata(dev);
	struct drm_device *drm_dev = data;
	struct drm_encoder *encoder;
	struct st_private *dev_priv = drm_dev->dev_private;
	struct drm_connector *drm_connector_tmp, *drm_connector = NULL;
	struct drm_crtc *crtc[ltdc->nb_crtc];
	int err;
	unsigned int i;
	unsigned int possible_crtcs;

	dev_priv->ltdc = ltdc;
	ltdc->drm_dev = drm_dev;

	dev_dbg(ltdc->dev, "%s drm_device *dev 0x%08x ltdc->regs 0x%08x\n",
		__func__, (u32)(drm_dev), (u32)(ltdc->regs));

	/* Create the ltdc encoders & connectors only if there is a rgb panel */
	for (i = 0; i < MAX_PANELS; i++) {
		if (!ltdc->panel[i])
			continue;

		encoder = ltdc_rgb_create_encoder(ltdc, i, ltdc->panel[i]);
		if (!encoder) {
			DRM_ERROR("Failed to create encoders\n");
			err = -EINVAL;
			goto err_clean_encoders_connectors;
		}

		drm_connector = devm_kzalloc(dev, sizeof(*drm_connector),
					     GFP_KERNEL);
		if (!drm_connector) {
			DRM_ERROR("Failed to allocate connector\n");
			err = -ENOMEM;
			goto err_clean_encoders_connectors;
		}

		drm_connector->polled = DRM_CONNECTOR_POLL_HPD;

		drm_connector_init(drm_dev, drm_connector,
				   &ltdc_connector_funcs,
				   DRM_MODE_CONNECTOR_LVDS);
		drm_connector_helper_add(drm_connector,
					 &ltdc_connector_helper_funcs);

		err = drm_connector_register(drm_connector);
		if (err) {
			DRM_ERROR("Failed to register connector\n");
			goto err_clean_encoders_connectors;
		}

		err = drm_panel_attach(ltdc->panel[i], drm_connector);
		if (err) {
			DRM_ERROR("Failed to attach panel to connector\n");
			goto err_clean_encoders_connectors;
		}

		err = drm_mode_connector_attach_encoder(drm_connector, encoder);
		if (err) {
			DRM_ERROR("Failed to attach connector to encoder\n");
			goto err_clean_encoders_connectors;
		}
	}

	if (ltdc->bridge) {
		if (ltdc->dual_display)
			/* Allow only one crtc to an encoder */
			possible_crtcs = 1 << min_t(int, i, ltdc->nb_crtc - 1);
		else
			possible_crtcs = GENMASK(ltdc->nb_crtc - 1, 0);

#ifdef CONFIG_DRM_ST_LTDC_HDMI
		/* Create HDMI encoder/connector */
		encoder = hdmi_encoder_create(drm_dev,
					      ltdc->bridge,
					      possible_crtcs);
		if (!encoder) {
			DRM_ERROR("Failed to create HDMI encoder-connector\n");
			goto err_clean_encoders_connectors;
		}
#endif
	}

	for (i = 0; i < ltdc->nb_crtc; i++) {
		crtc[i] = devm_kzalloc(dev, sizeof(*crtc[0]), GFP_KERNEL);
		if (!crtc[i]) {
			DRM_ERROR("Failed to allocate crtc\n");
			err = -ENOMEM;
			goto err_clean_hdmi_encoders_connectors;
		}
	}

	err = ltdc_crtcs_init(drm_dev, crtc);
	if (err) {
		DRM_ERROR("Failed to init crtc\n");
		goto err_clean_hdmi_encoders_connectors;
	}

	err = drm_vblank_init(drm_dev, ltdc->nb_crtc);
	if (err) {
		DRM_ERROR("Failed calling drm_vblank_init()\n");
		goto err_clean_planes;
	}

	/* Allow usage of vblank without having to call drm_irq_install */
	drm_dev->irq_enabled = 1;

	return 0;

err_clean_planes:
	ltdc_plane_destroy_all(ltdc);
err_clean_hdmi_encoders_connectors:
#ifdef CONFIG_DRM_ST_LTDC_HDMI
	if (ltdc->bridge)
		hdmi_encoder_destroy(drm_dev, ltdc->bridge);
#endif
err_clean_encoders_connectors:
	for (i = 0; i < MAX_PANELS; i++)
		if (!ltdc->panel[i])
			drm_panel_detach(ltdc->panel[i]);

	list_for_each_entry_safe(drm_connector, drm_connector_tmp,
				 &drm_dev->mode_config.connector_list, head)
		drm_connector_cleanup(drm_connector);

	ltdc_rgb_encoders_destroy(ltdc);
	return err;
}

static void ltdc_unbind(struct device *dev, struct device *master, void *data)
{
	struct ltdc *ltdc = dev_get_drvdata(dev);

	dev_dbg(ltdc->dev, "%s\n", __func__);
}

static const struct component_ops ltdc_ops = {
	.bind = ltdc_bind,
	.unbind = ltdc_unbind,
};

#ifdef CONFIG_DEBUG_FS

int ltdc_drm_fps_filter_get(void *data, u64 *val)
{
	struct drm_device *drm_dev = data;
	struct st_private *private = drm_dev->dev_private;
	struct ltdc *ltdc = private->ltdc;
	struct fps_info *fpsi = &ltdc->fpsi;

	*val = fpsi->dont_count_duplicated_frame;

	return 0;
}

int ltdc_drm_fps_filter_set(void *data, u64 val)
{
	struct drm_device *drm_dev = data;
	struct st_private *private = drm_dev->dev_private;
	struct ltdc *ltdc = private->ltdc;
	struct fps_info *fpsi = &ltdc->fpsi;

	fpsi->dont_count_duplicated_frame = val ? true : false;

	return 0;
}

#define DUMP_REG(name) \
	seq_printf(m, "\n%-16s (%#05x) 0x%08x", #name, name, reg_r(name))

#define DUMP_REG_LAYER(name, lay_id) do {			\
	snprintf(tmp, 256, "\n%-16s (%#05x) 0x%08x", #name,	\
		 name + lay_id * LAY_OFFSET,			\
		 reg_r(name + lay_id * LAY_OFFSET));		\
	tmp[7] = '1' + lay_id;					\
	seq_puts(m, tmp);					\
	} while (0)

static void ltdc_dbg_print_ycbcr_from_regs(struct seq_file *m, unsigned int val)
{
	u32 ycbcr_config;
	u32 drm_format = 0;

	/* If YUV format */
	if (val && LXAFBCR_EN) {
		ycbcr_config = val & ~LXAFBCR_EN & ~LXAFBCR_YHE;

		switch (ycbcr_config) {
		case YMODE_422I | OCYF_YUYV:
			drm_format = DRM_FORMAT_YUYV;
			break;
		case YMODE_422I | OCYF_YVYU:
			drm_format = DRM_FORMAT_YVYU;
			break;
		case YMODE_422I | OCYF_UYVY:
			drm_format = DRM_FORMAT_UYVY;
			break;
		case YMODE_422I | OCYF_VYUY:
			drm_format = DRM_FORMAT_VYUY;
			break;
		case YMODE_420SP | OCYF_UYVY:
			drm_format = DRM_FORMAT_NV12;
			break;
		case YMODE_420SP:
			drm_format = DRM_FORMAT_NV21;
			break;
		default:
			/* Not a YUV format */
			break;
		}

		if (drm_format)
			seq_printf(m, "\tYUV Format:%.4s", (char *)&drm_format);
	}
}

static void ltdc_dbg_act_width(struct seq_file *m, unsigned int val,
			       unsigned int hor_backporch,
			       unsigned int vert_backporch,
			       unsigned int dual_display)
{
	u32 act_width = (((val & BPCR_AHBP) >> 16) - hor_backporch);

	if (dual_display)
		seq_printf(m, "\tAct Width=%d (%d/2)\tAct Height=%d",
			   act_width / 2, act_width,
			   (val & BPCR_AVBP) - vert_backporch);
	else
		seq_printf(m, "\tAct Width=%d\tAct Height=%d", act_width,
			   (val & BPCR_AVBP) - vert_backporch);
}

static void ltdc_dbg_layer_hor_pos(struct seq_file *m, unsigned int val,
				   unsigned int hor_backporch,
				   unsigned int dual_display)
{
	u32 h_offset = (val & LXWHPCR_WHSTPOS) - hor_backporch - 1;
	u32 h_sz = ((val & LXWHPCR_WHSPPOS) >> 16) -
		   (val & LXWHPCR_WHSTPOS) + 1;

	if (dual_display)
		seq_printf(m, "\tHor Offset:%d (%d/2)\tHor Size:%d (%d/2)",
			   h_offset / 2, h_offset,
			   h_sz / 2, h_sz);
	else
		seq_printf(m, "\tHor Offset:%d\tHor Size:%d", h_offset,
			   h_sz);
}

static void ltdc_dbg_layer_vert_pos(struct seq_file *m, unsigned int val,
				    unsigned int vert_backporch)
{
	seq_printf(m, "\tVert Offset:%d\tVert Size:%d",
		   (val & LXWVPCR_WVSTPOS) - vert_backporch - 1,
		   ((val & LXWVPCR_WVSPPOS) >> 16) -
		   (val & LXWVPCR_WVSTPOS) + 1);
}

static void ltdc_dbg_layer_pix_fmt(struct seq_file *m, unsigned int val)
{
	struct drm_info_node *node = (struct drm_info_node *)m->private;
	struct drm_device *drm_dev = node->minor->dev;
	struct st_private *drm_priv = drm_dev->dev_private;
	struct ltdc *ltdc = drm_priv->ltdc;
	u32 pix_fmt = to_drm_pixelformat(ltdc->caps.pix_fmt_hw[val]);

	seq_printf(m, "\tPixel Format:%.4s", (char *)&pix_fmt);
}

static void ltdc_dbg_aux_fb_ctl(struct seq_file *m, unsigned int val)
{
	seq_printf(m, "\tYUVtoRGB Enabled:%d\tLimitedRange:%d", val
		   & LXAFBCR_EN ? 1 : 0, val & LXAFBCR_YHE ? 1 : 0);
}

int ltdc_mm_show(struct seq_file *m, void *arg)
{
	struct drm_info_node *node = (struct drm_info_node *)m->private;
	struct drm_device *dev = node->minor->dev;

	return drm_mm_dump_table(m, &dev->vma_offset_manager->vm_addr_space_mm);
}

int ltdc_fps_show(struct seq_file *m, void *arg)
{
	struct drm_info_node *node = (struct drm_info_node *)m->private;
	struct drm_device *drm_dev = node->minor->dev;
	struct st_private *drm_priv = drm_dev->dev_private;
	struct ltdc *ltdc = drm_priv->ltdc;
	struct fps_info *fpsi = &ltdc->fpsi;
	ktime_t now;
	int ms_since_last;
	unsigned int i, fpks;

	now = ktime_get();
	ms_since_last = ktime_to_ms(ktime_sub(now, fpsi->last_timestamp));

	/* Display all fps */
	for (i = 0; i < FPS_INFO_MAX_ENTRIES; i++) {
		if (ms_since_last && fpsi->frames[i]) {
			fpks = DIV_ROUND_CLOSEST(fpsi->frames[i] * 1000,
						 ms_since_last);
			fpsi->frames[i] = 0;
			if (fpks) /* Filter unused counters */
				seq_printf(m, "LTDC %-10s @ %d fps\n",
					   fps_info_str[i], fpks);
		}
	}

	fpsi->last_timestamp = now;

	return 0;
}

int ltdc_dbg_show_and_reset_underrun(struct seq_file *m, void *arg)
{
	struct drm_info_node *node = (struct drm_info_node *)m->private;
	struct drm_device *drm_dev = node->minor->dev;
	struct st_private *drm_priv = drm_dev->dev_private;
	struct ltdc *ltdc = drm_priv->ltdc;

	/* Read this debugfs entry to get underrun number and reset it */
	seq_printf(m, "Underrun Number:%d\n", ltdc->underrun_nb);
	ltdc->underrun_nb = 0;

	return 0;
}

int ltdc_dbg_show_regs(struct seq_file *m, void *arg)
{
	struct drm_info_node *node = (struct drm_info_node *)m->private;
	struct drm_device *drm_dev = node->minor->dev;
	struct st_private *drm_priv = drm_dev->dev_private;
	struct ltdc *ltdc = drm_priv->ltdc;
	unsigned int i;
	char tmp[256];
	unsigned int hor_backporch, vert_backporch;
	bool layer_enabled;

	if (ltdc_get_dp_regs_clk(ltdc))
		return -1;

	hor_backporch = (reg_r(LTDC_BPCR) & BPCR_AHBP) >> 16;
	vert_backporch = reg_r(LTDC_BPCR) & BPCR_AVBP;

	seq_puts(m, "Global:\n");
	DUMP_REG(LTDC_IDR);
	DUMP_REG(LTDC_LCR);
	DUMP_REG(LTDC_SSCR);
	DUMP_REG(LTDC_BPCR);
	DUMP_REG(LTDC_AWCR);
	ltdc_dbg_act_width(m, reg_r(LTDC_AWCR), hor_backporch, vert_backporch,
			   ltdc->dual_display);
	DUMP_REG(LTDC_TWCR);
	DUMP_REG(LTDC_GCR);
	DUMP_REG(LTDC_GC1R);
	DUMP_REG(LTDC_GC2R);
	DUMP_REG(LTDC_SRCR);
	DUMP_REG(LTDC_GACR);
	DUMP_REG(LTDC_BCCR);
	DUMP_REG(LTDC_IER);
	DUMP_REG(LTDC_ISR);
	DUMP_REG(LTDC_ICR);
	DUMP_REG(LTDC_LIPCR);
	DUMP_REG(LTDC_CPSR);
	DUMP_REG(LTDC_CDSR);
	DUMP_REG(LTDC_EDCR);

	for (i = 0; i < ltdc->caps.nb_layers; i++) {
		layer_enabled =
			(reg_r(LTDC_L1CR + i * LAY_OFFSET) & LXCR_LEN) ? 1 : 0;
		seq_printf(m, "\n\nLayer %d: %s %s\n", i + 1,
			   layer_enabled ? "ENABLED" : "DISABLED",
			   ltdc_is_layer_locked(ltdc, i) ?
			   ", LOCKED BY RPC" : "");

		DUMP_REG_LAYER(LTDC_L1LC1R, i);
		if (ltdc->hw_version == HWVER_20101)
			DUMP_REG_LAYER(LTDC_L1LC2R, i);
		DUMP_REG_LAYER(LTDC_L1CR, i);
		DUMP_REG_LAYER(LTDC_L1WHPCR, i);
		ltdc_dbg_layer_hor_pos(m, reg_r(LTDC_L1WHPCR + i * LAY_OFFSET),
				       hor_backporch, ltdc->dual_display);
		DUMP_REG_LAYER(LTDC_L1WVPCR, i);
		ltdc_dbg_layer_vert_pos(m, reg_r(LTDC_L1WVPCR + i * LAY_OFFSET),
					vert_backporch);
		DUMP_REG_LAYER(LTDC_L1CKCR, i);
		DUMP_REG_LAYER(LTDC_L1PFCR, i);
		ltdc_dbg_layer_pix_fmt(m, reg_r(LTDC_L1PFCR + i * LAY_OFFSET));
		DUMP_REG_LAYER(LTDC_L1CACR, i);
		DUMP_REG_LAYER(LTDC_L1DCCR, i);
		DUMP_REG_LAYER(LTDC_L1BFCR, i);
		DUMP_REG_LAYER(LTDC_L1FBBCR, i);
		DUMP_REG_LAYER(LTDC_L1AFBCR, i);
		ltdc_dbg_aux_fb_ctl(m, reg_r(LTDC_L1AFBCR + i * LAY_OFFSET));
		ltdc_dbg_print_ycbcr_from_regs(m, reg_r(LTDC_L1AFBCR
							+ i * LAY_OFFSET));
		DUMP_REG_LAYER(LTDC_L1CFBAR, i);
		DUMP_REG_LAYER(LTDC_L1CFBLR, i);
		DUMP_REG_LAYER(LTDC_L1CFBLNR, i);
		DUMP_REG_LAYER(LTDC_L1AFBAR, i);
		DUMP_REG_LAYER(LTDC_L1AFBLR, i);
		DUMP_REG_LAYER(LTDC_L1AFBLNR, i);
		DUMP_REG_LAYER(LTDC_L1CLUTWR, i);
		DUMP_REG_LAYER(LTDC_L1YS1R, i);
		DUMP_REG_LAYER(LTDC_L1YS2R, i);
		seq_puts(m, "\n");
	}

	ltdc_put_dp_regs_clk(ltdc);

	return 0;
}
#endif /* CONFIG_DEBUG_FS */

static int ltdc_set_caps(struct ltdc *ltdc)
{
	unsigned int i;

	ltdc->caps.nb_layers = min_t(int, reg_r(LTDC_LCR), MAX_PLANES);

	switch (ltdc->hw_version) {
	case HWVER_10200:
	case HWVER_10300:
		ltdc->caps.rg_ofst = RG_OFST_NONE;
		ltdc->caps.pix_fmt_hw = ltdc_pix_fmt_a0;
		for (i = 0; i < ltdc->caps.nb_layers; i++)
			ltdc->caps.ycbcr_support[i] = false;
		break;
	case HWVER_20101:
		ltdc->caps.rg_ofst = RG_OFST_4;
		ltdc->caps.pix_fmt_hw = ltdc_pix_fmt_a1;
		for (i = 0; i < ltdc->caps.nb_layers; i++)
			ltdc->caps.ycbcr_support[i] = LXLC2R_YCBCR &
				reg_r(LTDC_L1LC2R + i * LAY_OFFSET);
		break;
	default:
		return -ENODEV;
	}

	ltdc->caps.dual_display_support = reg_r(LTDC_GC2R) & (GC2R_DVIEW);

	return 0;
}

static int ltdc_unlock_layer(struct ltdc *ltdc, int lay_id)
{
	unsigned int crtc_id, i;
	struct drm_device *drm_dev;
	struct drm_crtc *crtc;

	/* update layer if used by linux */
	mutex_lock(&ltdc->layers_info_lock);
	ltdc_restore_layer(ltdc, lay_id);

	/* set to 0 the bit of unlocked layer */
	ltdc->locked_layers_status =
		ltdc->locked_layers_status & ~(1 << lay_id);

	/* Balance the ltdc_get_rpc_pix_clk done for remote proc */
	if (!ltdc->locked_layers_status)
		ltdc_put_rpc_pix_clk(ltdc);

	/* Disable the layer if not used by linux */
	if (!ltdc->lay_infos[lay_id].enabled) {
		DRM_DEBUG_DRIVER(
			"Disable layer %d (from 0), not used by linux\n"
				 , lay_id);

		if (ltdc_get_dp_regs_clk(ltdc)) {
			mutex_unlock(&ltdc->layers_info_lock);
			return -1;
		}

		/* disable layer */
		reg_clear(LXCR_LEN, LTDC_L1CR + lay_id * LAY_OFFSET);
		ltdc_put_dp_regs_clk(ltdc);

		reg_set(SRCR_VBR, LTDC_SRCR);
	}

	drm_dev = ltdc->drm_dev;
	crtc = ltdc_crtc_from_pipe(drm_dev, ltdc->lay_infos[lay_id].crtc_id);
	crtc_id = drm_crtc_index(crtc);

	/* Disable LTDC if not used by linux and Remote proc */
	if (!crtc->enabled && (ltdc->locked_layers_status == 0)) {
		/* Check that others crtc are disabled before disabling LTDC */
		if (!is_another_crtc_enabled(crtc)) {
			DRM_DEBUG_DRIVER("Disable CRTC as not used by linux\n");

			if (ltdc_get_dp_regs_clk(ltdc)) {
				mutex_unlock(&ltdc->layers_info_lock);
				return -1;
			}

			/* disable LTDC */
			reg_clear(GCR_LTDCEN, LTDC_GCR);

			/* disable IRQ */
			reg_clear(IER_RRIE | IER_FUIE | IER_TERRIE, LTDC_IER);

			/* disable layers */
			for (i = 0; i < ltdc->caps.nb_layers; i++)
				reg_clear(LXCR_LEN, LTDC_L1CR + i * LAY_OFFSET);

			/* immediately commit disable of layers */
			reg_w(SRCR_IMR, LTDC_SRCR);

			/* disable pixel clock */
			if (ltdc->ltdc_disable_waiting) {
				clk_disable_unprepare(ltdc->lcd_clk);
				ltdc->ltdc_disable_waiting = false;
			}

			ltdc_put_dp_regs_clk(ltdc);
		}
	}

	mutex_unlock(&ltdc->layers_info_lock);
	return 0;
}

int ltdc_rpmsg_endpoint_cb(struct rpmsg_mm_msg_hdr *data, void *priv)
{
	struct ltdc *ltdc = (struct ltdc *)priv;
	unsigned long layer_data;
	u32 lay_id;

	if (!data)
		return -1;

	DRM_DEBUG_DRIVER(
		"[RPMSG] endpoint callback[message:%d]\n", data->info);

	switch (data->info) {
	/* remote proc is informing about layer usage */
	case RPMSG_MM_SHARED_RES_LOCKED:
		if (ltdc->wait_for_rpmsg_available) {
			complete(&ltdc->init_rpmsg_complete);
		} else {
			layer_data = data->data[0];
			lay_id = __ffs(layer_data >>
				       __ffs(RPMSG_MM_HW_RES_OVERLAY_BASE));

			DRM_DEBUG_DRIVER(
				"layer %u (from 0) locked by remote proc\n",
				lay_id);

			mutex_lock(&ltdc->layers_info_lock);

			if (!ltdc->locked_layers_status)
				ltdc_get_rpc_pix_clk(ltdc);

			/* set to 1 the bit of locked layer */
			ltdc->locked_layers_status =
				ltdc->locked_layers_status | (1 << lay_id);
			mutex_unlock(&ltdc->layers_info_lock);
		}
		break;
	/* remote proc is informing that it doesn't use anymore a layer */
	case RPMSG_MM_SHARED_RES_UNLOCKED:
		if (ltdc->wait_for_rpmsg_available) {
			complete(&ltdc->init_rpmsg_complete);
		} else {
			layer_data = data->data[0];
			lay_id = __ffs(layer_data >>
				       __ffs(RPMSG_MM_HW_RES_OVERLAY_BASE));
			DRM_DEBUG_DRIVER(
				"layer %u (from 0) unlocked by remote proc\n",
				lay_id);

			if (ltdc_unlock_layer(ltdc, lay_id) < 0)
				return -1;
		}
		break;
	case RPMSG_MM_COMM_AVAILABLE:
		if (ltdc->wait_for_rpmsg_available)
			complete(&ltdc->init_rpmsg_complete);
		break;
	case RPMSG_MM_COMM_UNAVAILABLE:
	default:
		break;
	}
	return 0;
}

static void ltdc_wait_for_resource_status(struct ltdc *ltdc)
{
	int i, layer_status = 0;

	DRM_DEBUG_DRIVER(
		"[RPMSG] check resource availability\n");

check_availability:
	layer_status = sta_rpmsg_get_remote_resources_usage(
					ltdc->rpmsg_handle);

	if (layer_status < 0) {
		DRM_DEBUG_DRIVER(
			"[RPMSG] not available, wait for connection\n");
		if (!wait_for_completion_timeout(
						 &ltdc->init_rpmsg_complete,
						 msecs_to_jiffies(5000))) {
			/* Timeout after 5sec, consider the
			 * resource as available to avoid freeze
			 */
			dev_err(ltdc->dev, "[RPMSG] : TIMEOUT!!!\n");
			mutex_lock(&ltdc->layers_info_lock);
			ltdc->locked_layers_status = 0;
			mutex_unlock(&ltdc->layers_info_lock);

			ltdc->wait_for_rpmsg_available = false;
			return;
		}
		goto check_availability;
	} else {
		mutex_lock(&ltdc->layers_info_lock);
		ltdc->locked_layers_status =
			(layer_status >> __ffs(RPMSG_MM_HW_RES_OVERLAY_BASE))
			& LOCKED_LAYER_MASK;
		mutex_unlock(&ltdc->layers_info_lock);

		ltdc->wait_for_rpmsg_available = false;

		DRM_DEBUG_DRIVER("locked Layer status = 0x%x\n",
				 ltdc->locked_layers_status);

		/* Update unlocked layers if needed */
		for (i = 0; i < RPMSG_MM_NB_OVERLAY; i++) {
			if (!ltdc_is_layer_locked(ltdc, i))
				ltdc_unlock_layer(ltdc, i);
		}
	}
}

static void ltdc_init_rpmsg_work(struct work_struct *work)
{
	struct ltdc *ltdc = container_of(work, struct ltdc, init_work);
	struct device *dev = ltdc->dev;

	if (!ltdc->rpmsg_handle)
		ltdc->rpmsg_handle =
			sta_rpmsg_register_endpoint(
				RPSMG_MM_EPT_CORTEXA_DISPLAY,
				ltdc_rpmsg_endpoint_cb,
				(void *)ltdc);

	if (!ltdc->rpmsg_handle) {
		dev_err(dev, "Unable to register rpmsg_mm\n");
		return;
	}

	/* Start by checking that CDC IP status */
	ltdc_wait_for_resource_status(ltdc);
}

static bool dual_display_param;
module_param_named(dual_display, dual_display_param, bool, 0600);
MODULE_PARM_DESC(dual_display, "Enable dual display in driver");

static int ltdc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct reset_control *rstc;
	struct resource res;
	struct ltdc *ltdc;
	int irq, ret;
	struct device_node *entity, *remote_port;
	int panel_nb = 0;
	struct drm_panel *panel[MAX_PANELS];
	struct drm_bridge *bridge = NULL;
	unsigned int i;

	DRM_DEBUG_DRIVER("\n");

	/* parse ltdc node to get remote ports */
	for_each_endpoint_of_node(np, entity) {
		remote_port = of_graph_get_remote_port_parent(entity);
		if (!remote_port || !of_device_is_available(remote_port)) {
			of_node_put(remote_port);
			continue;
		}

		if (of_device_is_compatible(remote_port, "adi,adv7513")) {
#ifdef CONFIG_DRM_ST_LTDC_HDMI
			if (!bridge) {
				bridge = of_drm_find_bridge(remote_port);
				of_node_put(remote_port);
				if (bridge) {
					DRM_DEBUG_DRIVER("remote bridge %s\n",
							 remote_port->full_name
							 );
				} else {
					DRM_DEBUG_DRIVER(
						"deferred (HDMI Bridge missing)\n"
						);
					goto deferred;
				}
			}
#endif
		} else if (panel_nb < MAX_PANELS) {
			panel[panel_nb] = of_drm_find_panel(remote_port);
			if (panel[panel_nb]) {
				DRM_DEBUG_DRIVER("remote panel %s\n",
						 remote_port->full_name);
				panel_nb++;
			} else {
				DRM_DEBUG_DRIVER(
						 "deferred (panel missing)\n");
				goto deferred;
			}
		}

		of_node_put(remote_port);
	}

	ltdc = devm_kzalloc(dev, sizeof(*ltdc), GFP_KERNEL);
	if (!ltdc) {
		DRM_ERROR("Cannot allocate memory for ltdc\n");
		return -ENOMEM;
	}

	ltdc->dev = dev;

	ltdc->bridge = bridge;

	for (i = 0; i < panel_nb; i++)
		ltdc->panel[i] = panel[i];

	rstc = of_reset_control_get(np, NULL);

	mutex_init(&ltdc->err_lock);
	mutex_init(&ltdc->gamma_lock);

	ltdc->clk = devm_clk_get(&pdev->dev, "clk");
	if (IS_ERR(ltdc->clk)) {
		DRM_ERROR("Unable to get ltdc clk\n");
		return -ENODEV;
	}

	ltdc->lcd_clk = devm_clk_get(&pdev->dev, "lcd_clk");
	if (IS_ERR(ltdc->lcd_clk)) {
		DRM_ERROR("Unable to get lcd clk\n");
		return -ENODEV;
	}

	if (of_address_to_resource(np, 0, &res)) {
		DRM_ERROR("Unable to get resource\n");
		return -ENODEV;
	}

	ltdc->regs = devm_ioremap_resource(dev, &res);
	if (IS_ERR(ltdc->regs)) {
		DRM_ERROR("Unable to get regs\n");
		return PTR_ERR(ltdc->regs);
	}

	if (!IS_ERR(rstc))
		reset_control_deassert(rstc);

	/* enable clock for registers read/write */
	if (clk_prepare_enable(ltdc->clk)) {
		DRM_ERROR("failed to clock on ltdc\n");
		return -ENODEV;
	}

	ltdc->hw_version = reg_r(LTDC_IDR);
	DRM_INFO("ltdc hw version 0x%08x - ready\n", ltdc->hw_version);
	ret = ltdc_set_caps(ltdc);

	if (ret) {
		DRM_ERROR("hardware identifier (0x%08x) not supported!\n",
			  ltdc->hw_version);
		goto err_disable_clk;
	}

	/* Disable interrupts */
	reg_clear(IER_LIE | IER_RRIE | IER_FUIE | IER_TERRIE, LTDC_IER);

	irq = platform_get_irq(pdev, 0);
	ret = devm_request_threaded_irq(dev, irq, ltdc_irq,
					ltdc_irq_thread, IRQF_ONESHOT,
					dev_name(dev), ltdc);
	if (ret) {
		DRM_ERROR("Failed to register LTDC interrupt\n");
		goto err_disable_clk;
	}

	/* get data bus width */
	if (of_property_read_u32(np, "bus-width", &ltdc->bus_width))
		ltdc->bus_width = 32; /* set to 32 by default */

	ltdc->dual_display = ltdc->caps.dual_display_support &&
			     dual_display_param;
	ltdc->nb_crtc = ltdc->dual_display ? 2 : 1;

	dev_dbg(dev, "ltdc bus width %d\n", ltdc->bus_width);

	mutex_init(&ltdc->layers_info_lock);

	platform_set_drvdata(pdev, ltdc);

	ret = component_add(&pdev->dev, &ltdc_ops);
	if (ret) {
		DRM_ERROR("Cannot add component\n");
		goto err_disable_clk;
	}

	mutex_lock(&ltdc->layers_info_lock);
	/* Lock all layers at probe */
	ltdc->locked_layers_status = LOCKED_LAYER_MASK;
	ltdc_reset_layers_status(ltdc);

	ltdc->ltdc_disable_waiting = false;

	mutex_unlock(&ltdc->layers_info_lock);

	/* Make sure that linux don't disable pixel clock when remote proc
	 * is using it
	 */
	if (ltdc_get_rpc_pix_clk(ltdc))
		return -1;

	ltdc->wait_for_rpmsg_available = true;

	ltdc->rpmsg_handle = NULL;

	ltdc->rpmsg_wq = create_singlethread_workqueue("ltdc_rpmsg_wq");
	if (!ltdc->rpmsg_wq)
		return -ENOMEM;

	init_completion(&ltdc->init_rpmsg_complete);
	INIT_WORK(&ltdc->init_work, ltdc_init_rpmsg_work);
	queue_work(ltdc->rpmsg_wq, &ltdc->init_work);

	return 0;

deferred:
	of_node_put(remote_port);
	of_node_put(entity);
	return -EPROBE_DEFER;
err_disable_clk:
	clk_disable_unprepare(ltdc->clk);
	return ret;
}

static int ltdc_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ltdc *ltdc = dev_get_drvdata(dev);

	DRM_DEBUG_DRIVER("\n");

	sta_rpmsg_unregister_endpoint(ltdc->rpmsg_handle);

	complete(&ltdc->init_rpmsg_complete);
	cancel_work_sync(&ltdc->init_work);
	destroy_workqueue(ltdc->rpmsg_wq);

	component_del(&pdev->dev, &ltdc_ops);

	/* disable clocks */
	clk_disable_unprepare(ltdc->clk);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int ltdc_drm_pm_suspend(struct device *dev)
{
	struct ltdc *ltdc = dev_get_drvdata(dev);
	int ret;

	DRM_DEBUG_DRIVER("\n");

	if (!ltdc)
		return 0;

	drm_kms_helper_poll_disable(ltdc->drm_dev);

	ltdc->state = drm_atomic_helper_suspend(ltdc->drm_dev);
	if (IS_ERR(ltdc->state)) {
		drm_kms_helper_poll_enable(ltdc->drm_dev);
		return PTR_ERR(ltdc->state);
	}

	/* Don't disable Clock if a layer is used by remote proc */
	mutex_lock(&ltdc->layers_info_lock);
	if (!ltdc->locked_layers_status)
		clk_disable_unprepare(ltdc->clk);
	mutex_unlock(&ltdc->layers_info_lock);

	/* decrement pixel clock counter if ltdc disable is waiting */
	if (ltdc->ltdc_disable_waiting) {
		clk_disable_unprepare(ltdc->lcd_clk);
		ltdc->ltdc_disable_waiting = false;
	}

	ret = pinctrl_pm_select_sleep_state(dev);

	complete(&ltdc->init_rpmsg_complete);
	cancel_work_sync(&ltdc->init_work);

	return ret;
}

static int ltdc_drm_pm_resume(struct device *dev)
{
	struct ltdc *ltdc = dev_get_drvdata(dev);
	int ret;

	DRM_DEBUG_DRIVER("\n");

	if (!ltdc)
		return 0;

	mutex_lock(&ltdc->layers_info_lock);

	/* if no layers were locked during suspend, we need to enable clocks
	 * else clocks were kept during suspend
	 */
	if (!ltdc->locked_layers_status) {
		if (clk_prepare_enable(ltdc->clk) < 0) {
			dev_err(dev, "failed to enable ltdc clk\n");
			goto error;
		}
		/* Make sure that linux don't disable pixel clock when remote
		 * proc is using it
		 */
		if (ltdc_get_rpc_pix_clk(ltdc))
			goto error;
	}

	/* Lock all layers at resume */
	ltdc->locked_layers_status = LOCKED_LAYER_MASK;
	ltdc_reset_layers_status(ltdc);

	ltdc->ltdc_disable_waiting = false;

	mutex_unlock(&ltdc->layers_info_lock);

	ltdc->wait_for_rpmsg_available = true;

	ret = pinctrl_pm_select_default_state(dev);
	if (ret < 0) {
		dev_err(dev, "failed to select pinctrl state\n");
		return ret;
	}

	reinit_completion(&ltdc->init_rpmsg_complete);
	queue_work(ltdc->rpmsg_wq, &ltdc->init_work);

	drm_atomic_helper_resume(ltdc->drm_dev, ltdc->state);
	drm_kms_helper_poll_enable(ltdc->drm_dev);

	return 0;

error:
	mutex_unlock(&ltdc->layers_info_lock);
	return -1;
}
#endif
static const struct dev_pm_ops ltdc_drm_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(ltdc_drm_pm_suspend, ltdc_drm_pm_resume)
};

static const struct of_device_id ltdc_of_match[] = {
	{ .compatible = "st,ltdc" },
	{ /* end node */ }
};
MODULE_DEVICE_TABLE(of, ltdc_of_match);

struct platform_driver ltdc_driver = {
	.driver = {
		.name = "st-ltdc",
		.pm	= &ltdc_drm_pm_ops,
		.of_match_table = ltdc_of_match,
	},
	.probe = ltdc_probe,
	.remove = ltdc_remove,
};

MODULE_AUTHOR("Philippe Cornu <philippe.cornu@st.com>");
MODULE_AUTHOR("Yannick Fertre <yannick.fertre@st.com>");
MODULE_DESCRIPTION("STMicroelectronics ST DRM LTDC driver");
MODULE_LICENSE("GPL");
