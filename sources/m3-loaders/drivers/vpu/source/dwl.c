/*
 * Copyright (C) STMicroelectronics SA 2017
 * Authors: Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics.
 *          Stephane Danieau <stephane.danieau@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <string.h>
#include <stdlib.h>
#include "dwl.h"
#include "utils.h"
#include "sta_map.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "trace.h"

#define G1_HW_ASIC_ID                  0
#define G1_REGS_SIZE	0x194
#define G1_HW_PP_SYNTH_CFG       100
#define G1_HW_DEC_SYNTH_CFG       50
#define G1_HW_DEC_SYNTH_CFG_2     54

#define G1_HW_PP_FUSE_CFG         99
#define G1_HW_DEC_FUSE_CFG        57

#define G1_HW_OFFSET_SWREG_0	0x000
#define G1_HW_OFFSET_SWREG_50	0x0C8
#define G1_HW_OFFSET_SWREG_52	0x0D0
#define G1_HW_OFFSET_SWREG_53	0x0D4
#define G1_HW_OFFSET_SWREG_54	0x0D8
#define G1_HW_OFFSET_SWREG_56	0x0E0
#define G1_HW_OFFSET_SWREG_57	0x0E4
#define G1_HW_OFFSET_SWREG_58	0x0E8
#define G1_HW_OFFSET_SWREG_75	0x12C
#define G1_HW_OFFSET_SWREG_76	0x130
#define G1_HW_OFFSET_SWREG_77	0x134
#define G1_HW_OFFSET_SWREG_78	0x138
#define G1_HW_OFFSET_SWREG_95	0x17C

/* define irq registers offset */
#define G1_HW_DEC_IRQ_OFFSET	(1 * 4)
#define G1_HW_PP_IRQ_OFFSET	(60 * 4)

#define to_ctx(inst) (((struct ip_instance *)inst)->pctx)

struct ip_instance {
	u32 clientType;
	struct dwl_ctx * pctx;

};

#define G1_MAX_REGS  (60 + 40 + 10)
struct g1_hw_regs {
	int n;
	int offset[G1_MAX_REGS];
	int data[G1_MAX_REGS];
};

struct dwl_ctx {
	SemaphoreHandle_t hw_protect;
	DWLHwFuseStatus_t * fuse;
	DWLHwConfig_t * config;
	struct g1_hw_regs regs_w_dec;
	struct g1_hw_regs regs_r_dec;
};

static u32 dwl_asic_id;
DWLHwConfig_t dwl_config;
DWLHwFuseStatus_t dwl_fuse;



u32 g1_hw_get_asic_id(void)
{
	u32 dwl_asic_id = read_reg(G1_BASE + 4 * G1_HW_ASIC_ID);
	return dwl_asic_id;
};

void g1_hw_cfg_build_fuse(DWLHwFuseStatus_t *fuse)
{	
	u32 dec_fuse = read_reg(G1_BASE + 4 * G1_HW_DEC_FUSE_CFG);
	u32 pp_cfg = read_reg(G1_BASE + 4 * G1_HW_PP_SYNTH_CFG);
	u32 pp_fuse = read_reg(G1_BASE + 4 * G1_HW_PP_FUSE_CFG);

	memset(fuse, 0, sizeof(*fuse));

	/* Decoder fuse configuration */
	fuse->h264SupportFuse = (dec_fuse >> G1_H264_FUSE_E) & 0x01U;
	fuse->mpeg4SupportFuse = (dec_fuse >> G1_MPEG4_FUSE_E) & 0x01U;
	fuse->mpeg2SupportFuse = (dec_fuse >> G1_MPEG2_FUSE_E) & 0x01U;
	fuse->sorensonSparkSupportFuse =
	    (dec_fuse >> G1_SORENSONSPARK_FUSE_E) & 0x01U;
	fuse->jpegSupportFuse = (dec_fuse >> G1_JPEG_FUSE_E) & 0x01U;
	fuse->vp6SupportFuse = (dec_fuse >> G1_VP6_FUSE_E) & 0x01U;
	fuse->vc1SupportFuse = (dec_fuse >> G1_VC1_FUSE_E) & 0x01U;
	fuse->jpegProgSupportFuse = (dec_fuse >> G1_PJPEG_FUSE_E) & 0x01U;
	fuse->rvSupportFuse = (dec_fuse >> G1_RV_FUSE_E) & 0x01U;
	fuse->avsSupportFuse = (dec_fuse >> G1_AVS_FUSE_E) & 0x01U;
	fuse->vp7SupportFuse = (dec_fuse >> G1_VP7_FUSE_E) & 0x01U;
	fuse->vp8SupportFuse = (dec_fuse >> G1_VP8_FUSE_E) & 0x01U;
	fuse->customMpeg4SupportFuse =
	    (dec_fuse >> G1_CUSTOM_MPEG4_FUSE_E) & 0x01U;
	fuse->mvcSupportFuse = (dec_fuse >> G1_MVC_FUSE_E) & 0x01U;

	/* check max. decoder output width */
	if (dec_fuse & 0x10000U)
		fuse->maxDecPicWidthFuse = 4096;
	else if (dec_fuse & 0x8000U)
		fuse->maxDecPicWidthFuse = 1920;
	else if (dec_fuse & 0x4000U)
		fuse->maxDecPicWidthFuse = 1280;
	else if (dec_fuse & 0x2000U)
		fuse->maxDecPicWidthFuse = 720;
	else if (dec_fuse & 0x1000U)
		fuse->maxDecPicWidthFuse = 352;

	fuse->refBufSupportFuse = (dec_fuse >> G1_REF_BUFF_FUSE_E) & 0x01U;

	if ((pp_cfg >> G1_PP_E) & 0x01U) {
		/* Pp fuse configuration */
		if ((pp_fuse >> G1_PP_FUSE_E) & 0x01U) {
			fuse->ppSupportFuse = 1;

			/* check max. pp output width */
			if (pp_fuse & 0x10000U)
				fuse->maxPpOutPicWidthFuse = 4096;
			else if (pp_fuse & 0x8000U)
				fuse->maxPpOutPicWidthFuse = 1920;
			else if (pp_fuse & 0x4000U)
				fuse->maxPpOutPicWidthFuse = 1280;
			else if (pp_fuse & 0x2000U)
				fuse->maxPpOutPicWidthFuse = 720;
			else if (pp_fuse & 0x1000U)
				fuse->maxPpOutPicWidthFuse = 352;

			fuse->ppConfigFuse = pp_fuse;
		} else {
			fuse->ppSupportFuse = 0;
			fuse->maxPpOutPicWidthFuse = 0;
			fuse->ppConfigFuse = 0;
		}
	}
}


void g1_hw_cfg_build_config(DWLHwConfig_t *config, DWLHwFuseStatus_t *fuse)
{
	u32 dec_cfg = read_reg(G1_BASE + 4 * G1_HW_DEC_SYNTH_CFG);
	u32 dec_cfg2 = read_reg(G1_BASE + 4 * G1_HW_DEC_SYNTH_CFG_2);
	u32 pp_cfg = read_reg(G1_BASE + 4 * G1_HW_PP_SYNTH_CFG);
        u32 asic_id = read_reg(G1_BASE + 4 * G1_HW_ASIC_ID);

	memset(config, 0, sizeof(*config));

	config->h264Support = (dec_cfg >> G1_H264_E) & 0x3U;
	/* check jpeg */
	config->jpegSupport = (dec_cfg >> G1_JPEG_E) & 0x01U;
	if (config->jpegSupport && ((dec_cfg >> G1_PJPEG_E) & 0x01U))
		config->jpegSupport = JPEG_PROGRESSIVE;
	config->mpeg4Support = (dec_cfg >> G1_MPEG4_E) & 0x3U;
	config->vc1Support = (dec_cfg >> G1_VC1_E) & 0x3U;
	config->mpeg2Support = (dec_cfg >> G1_MPEG2_E) & 0x01U;
	config->sorensonSparkSupport =
		(dec_cfg >> G1_SORENSONSPARK_E) & 0x01U;
	config->refBufSupport = (dec_cfg >> G1_REF_BUFF_E) & 0x01U;
	config->vp6Support = (dec_cfg >> G1_VP6_E) & 0x01U;
#ifdef DEC_X170_APF_DISABLE
	if (DEC_X170_APF_DISABLE)
		config->tiledModeSupport = 0;
#endif /* DEC_X170_APF_DISABLE */

	config->maxDecPicWidth = dec_cfg & 0x07FFU;

	if (config->refBufSupport) {
		if ((dec_cfg2 >> G1_REF_BUFF_ILACE_E) & 0x01U)
			config->refBufSupport |= 2;
		if ((dec_cfg2 >> G1_REF_BUFF_DOUBLE_E) & 0x01U)
			config->refBufSupport |= 4;
	}

	config->customMpeg4Support = (dec_cfg2 >> G1_MPEG4_CUSTOM_E) & 0x01U;
	config->vp7Support = (dec_cfg2 >> G1_VP7_E) & 0x01U;
	config->vp8Support = (dec_cfg2 >> G1_VP8_E) & 0x01U;
	config->avsSupport = (dec_cfg2 >> G1_AVS_E) & 0x01U;

	/* JPEG extensions */
	if (((asic_id >> 16) >= 0x8190U) || ((asic_id >> 16) == 0x6731U))
		config->jpegESupport = (dec_cfg2 >> G1_JPEG_EXT_E) & 0x01U;
	else
		config->jpegESupport = JPEG_EXT_NOT_SUPPORTED;

	if (((asic_id >> 16) >= 0x9170U) || ((asic_id >> 16) == 0x6731U))
		config->rvSupport = (dec_cfg2 >> G1_RV_E) & 0x03U;
	else
		config->rvSupport = RV_NOT_SUPPORTED;

	config->mvcSupport = (dec_cfg2 >> G1_MVC_E) & 0x03U;
	config->webpSupport = (dec_cfg2 >> G1_WEBP_E) & 0x01U;
	config->tiledModeSupport = (dec_cfg2 >> G1_DEC_TILED_L) & 0x03U;
	config->maxDecPicWidth +=
	    ((dec_cfg2 >> G1_DEC_PIC_W_EXT) & 0x03U) << 11;

	config->ecSupport = (dec_cfg2 >> G1_EC_E) & 0x03U;
	config->strideSupport = (dec_cfg2 >> G1_STRIDE_E) & 0x01U;

	if (config->refBufSupport && (asic_id >> 16) == 0x6731U)
		config->refBufSupport |= 8;/* enable HW support for offset */

	if ((pp_cfg >> G1_PP_E) & 0x01U) {
		config->ppSupport = 1;
		/* Theoretical max range 0...8191; actual 48...4096 */
		config->maxPpOutPicWidth = pp_cfg & 0x1FFFU;
		config->ppConfig = pp_cfg;
	} else {
		config->ppSupport = 0;
		config->maxPpOutPicWidth = 0;
		config->ppConfig = 0;
	}

	/* check the HW version */
	if (((asic_id >> 16) >= 0x8190U) || ((asic_id >> 16) == 0x6731U)) {
		u32 de_interlace;
		u32 alpha_blend;
		u32 de_interlace_fuse;
		u32 alpha_blend_fuse;

		/* Maximum decoding width supported by the HW */
		if (config->maxDecPicWidth > fuse->maxDecPicWidthFuse)
			config->maxDecPicWidth =
				fuse->maxDecPicWidthFuse;
		/* Maximum output width of Post-Processor */
		if (config->maxPpOutPicWidth >
		    fuse->maxPpOutPicWidthFuse)
			config->maxPpOutPicWidth =
				fuse->maxPpOutPicWidthFuse;
		/* h264 */
		if (!fuse->h264SupportFuse)
			config->h264Support = H264_NOT_SUPPORTED;
		/* mpeg-4 */
		if (!fuse->mpeg4SupportFuse)
			config->mpeg4Support = MPEG4_NOT_SUPPORTED;
		/* custom mpeg-4 */
		if (!fuse->customMpeg4SupportFuse)
			config->customMpeg4Support =
				MPEG4_CUSTOM_NOT_SUPPORTED;
		/* jpeg (baseline && progressive) */
		if (!fuse->jpegSupportFuse)
			config->jpegSupport = JPEG_NOT_SUPPORTED;
		if ((config->jpegSupport == JPEG_PROGRESSIVE) &&
		    !fuse->jpegProgSupportFuse)
			config->jpegSupport = JPEG_BASELINE;
		/* mpeg-2 */
		if (!fuse->mpeg2SupportFuse)
			config->mpeg2Support = MPEG2_NOT_SUPPORTED;
		/* vc-1 */
		if (!fuse->vc1SupportFuse)
			config->vc1Support = VC1_NOT_SUPPORTED;
		/* vp6 */
		if (!fuse->vp6SupportFuse)
			config->vp6Support = VP6_NOT_SUPPORTED;
		/* vp7 */
		if (!fuse->vp7SupportFuse)
			config->vp7Support = VP7_NOT_SUPPORTED;
		/* vp8 */
		if (!fuse->vp8SupportFuse)
			config->vp8Support = VP8_NOT_SUPPORTED;
		/* webp */
		if (!fuse->vp8SupportFuse)
			config->webpSupport = WEBP_NOT_SUPPORTED;
		/* pp */
		if (!fuse->ppSupportFuse)
			config->ppSupport = PP_NOT_SUPPORTED;
		/* check the pp config vs fuse status */
		if ((config->ppConfig & 0xFC000000) &&
		    ((fuse->ppConfigFuse & 0xF0000000) >> 5)) {
			/* config */
			de_interlace =
			  ((config->ppConfig & PP_DEINTERLACING) >> 25);
			alpha_blend =
			  ((config->ppConfig & PP_ALPHA_BLENDING) >> 24);
			/* fuse */
			de_interlace_fuse =
			  (((fuse->ppConfigFuse >> 5) & PP_DEINTERLACING) >>
			   25);
			alpha_blend_fuse =
			  (((fuse->ppConfigFuse >> 5) & PP_ALPHA_BLENDING) >>
			   24);

			/* check if */
			if (de_interlace && !de_interlace_fuse)
				config->ppConfig &= 0xFD000000;
			if (alpha_blend && !alpha_blend_fuse)
				config->ppConfig &= 0xFE000000;
		}
		/* sorenson */
		if (!fuse->sorensonSparkSupportFuse)
			config->sorensonSparkSupport =
			    SORENSON_SPARK_NOT_SUPPORTED;
		/* ref. picture buffer */
		if (!fuse->refBufSupportFuse)
			config->refBufSupport = REF_BUF_NOT_SUPPORTED;

		/* rv */
		if (!fuse->rvSupportFuse)
			config->rvSupport = RV_NOT_SUPPORTED;
		/* avs */
		if (!fuse->avsSupportFuse)
			config->avsSupport = AVS_NOT_SUPPORTED;
		/* mvc */
		if (!fuse->mvcSupportFuse)
			config->mvcSupport = MVC_NOT_SUPPORTED;
	}
}

void * g1_hw_init(void)
{
	struct dwl_ctx * pctx = calloc(1,sizeof(*pctx));

	// To be done after IP setup
	dwl_asic_id = g1_hw_get_asic_id();
	g1_hw_cfg_build_fuse(&dwl_fuse);
	g1_hw_cfg_build_config(&dwl_config, &dwl_fuse);
	pctx->hw_protect = xSemaphoreCreateMutex();
	if (!pctx->hw_protect) {
		TRACE_ERR("Unable to allocate a new semaphore\n");
		return NULL;
	}
	return ((void*)pctx);
}

void g1_hw_release(void * ctx)
{
	struct dwl_ctx *pctx = (struct dwl_ctx *)ctx;

	if (pctx && pctx->hw_protect){
		vSemaphoreDelete(pctx->hw_protect );
		pctx->hw_protect = NULL;
	}
	if (pctx) {
		free(pctx);
		pctx = NULL;
	}
}

/*
 * g1_hw_read_only_register
 * @reg_offset : the offset of reg to check
 * return true if the register is a read only register
 */
static bool g1_hw_read_only_register(int reg_offset)
{
	switch (reg_offset) {
	case G1_HW_OFFSET_SWREG_0:
	case G1_HW_OFFSET_SWREG_50:
	case G1_HW_OFFSET_SWREG_52:
	case G1_HW_OFFSET_SWREG_53:
	case G1_HW_OFFSET_SWREG_54:
	case G1_HW_OFFSET_SWREG_56:
	case G1_HW_OFFSET_SWREG_57:
	case G1_HW_OFFSET_SWREG_58:
	case G1_HW_OFFSET_SWREG_75:
	case G1_HW_OFFSET_SWREG_76:
	case G1_HW_OFFSET_SWREG_77:
	case G1_HW_OFFSET_SWREG_78:
		return true;
	}
	if (reg_offset >= G1_HW_OFFSET_SWREG_95)
		return true;

	return false;
}

int g1_hw_run(struct dwl_ctx *ctx, struct g1_hw_regs *regs)
{
	int i;
	int ret = 0;
	int pending_pp = 0;
	int pending_dec = 0;
	u32 timeout=0;
	unsigned int irq_dec;
	unsigned int irq_pp;

	/* reserve hardware */
	xSemaphoreTake( ctx->hw_protect,  portMAX_DELAY );

	/* write data into g1 registers */
	for (i = 0; i < regs->n; i++) {
		/* do not write read-only registers */
		if (!g1_hw_read_only_register((regs->offset[i]))) {
			/* detect start of pp and wait for it
			 * FIXME to rework
			 */
			if ((regs->offset[i] == G1_HW_PP_IRQ_OFFSET) &&
			    (regs->data[i] & 1)) {
				pending_pp = 1;
			}
			/* detect start of decoder and wait for it
			 * FIXME to rework
			 */
			if ((regs->offset[i] == G1_HW_DEC_IRQ_OFFSET) &&
			    (regs->data[i] & 1)) {
				pending_dec = 1;
			}
			write_reg(regs->data[i], G1_BASE + regs->offset[i]);	
		}
	}

	
	timeout = 200; // 200 x 5ms 
	/* now waiting for hardware completion if needed */
	while ((pending_pp || pending_dec) && timeout ) {
		irq_dec = read_reg(G1_BASE + G1_HW_DEC_IRQ_OFFSET);
		irq_pp = read_reg(G1_BASE + G1_HW_PP_IRQ_OFFSET);
		if (pending_pp && (irq_pp & G1_HW_PP_IRQ_MASK)) {
			/* clear decoder irq */
			write_reg(irq_pp & (~G1_HW_DEC_IRQ_MASK),  (G1_BASE + G1_HW_PP_IRQ_OFFSET));
			pending_pp = 0;
		}
		if (pending_dec && (irq_dec & G1_HW_DEC_IRQ_MASK)) {
			/* clear decoder irq */
			write_reg(irq_dec & (~G1_HW_DEC_IRQ_MASK),  (G1_BASE + G1_HW_DEC_IRQ_OFFSET));
			pending_dec = 0;
		}
		vTaskDelay( pdMS_TO_TICKS( 5 ) );
		timeout--;
	}

	if(!timeout && pending_pp) {
		TRACE_ERR("post proc timeout\n");
		ret = -1;
	}
	if(!timeout && pending_dec) {
		TRACE_ERR("decoder timeout\n");
		ret = -1;
	}

	/* read back all register */
	for (i = 0; i < G1_REGS_SIZE / 4; i++) {
		regs->data[i] = read_reg(G1_BASE + 4 * i);
		regs->offset[i] = 4 * i;

		if (i == G1_HW_PP_IRQ_OFFSET / 4) {
			/* turn pipeline OFF so it doesn't interfer
			 * with other instances
			 */
			regs->data[i] &= ~G1_HW_PP_PIPELINE_E_MASK;
			write_reg(regs->data[i], (G1_BASE + G1_HW_PP_IRQ_OFFSET));
		}
	}

	/* unreserve hardware */
	xSemaphoreGive( ctx->hw_protect);
	return ret;
}

/* API added to solve globals initialization  : to call by application at init*/
void DWLSetup(void * ctx)
{
	struct dwl_ctx * pctx = (struct dwl_ctx *)ctx;
	if (!pctx)
	  return;
	pctx->config = &dwl_config;
	pctx->fuse = &dwl_fuse;
}


/* API added to let throw context to DWL to call by application when instance has been created to forward the context*/
void DWLSetUserContext(const void *instance, void *userCtx)
{
	struct dwl_ctx *dwl = NULL;
	int i=0;
	struct ip_instance * my_inst = (struct ip_instance *)instance;
	dwl = (struct dwl_ctx *)userCtx;
	my_inst->pctx = dwl;
	/* init shadow regs */
	dwl->regs_w_dec.n = 0;
	for (i = 0; i < G1_MAX_REGS; i++) {
		dwl->regs_w_dec.offset[i] = 4 * i;
		dwl->regs_w_dec.data[i] = 0;
	}

	/* only force 2 two following registers to be reset,
	 * will be written when first run is called for the first frame
	 * FIXME, to clarify
	 */
	dwl->regs_w_dec.offset[0] = G1_HW_DEC_IRQ_OFFSET;
	dwl->regs_w_dec.data[0] = 0;
	dwl->regs_w_dec.n++;
	dwl->regs_w_dec.offset[1] = G1_HW_PP_IRQ_OFFSET;
	dwl->regs_w_dec.data[1] = 0;
	dwl->regs_w_dec.n++;
}

u32 DWLReadAsicID(void)
{
	return dwl_asic_id;
}

void DWLReadAsicFuseStatus (DWLHwFuseStatus_t * pHwFuseSts)
{
	memcpy(pHwFuseSts, &dwl_fuse, sizeof(*pHwFuseSts));
}

void DWLReadAsicConfig(DWLHwConfig_t *pHwCfg)
{
	memcpy(pHwCfg, &dwl_config, sizeof(*pHwCfg));
};

const void *DWLInit(DWLInitParam_t *param)
{
	struct ip_instance * instance = NULL;
	if (!param)
		return NULL;
	instance = calloc(1,sizeof(*instance));
	if (!instance)
		return NULL;
	instance->clientType = param->clientType;
	instance->pctx = NULL;
	return instance;
};

i32 DWLRelease(const void *instance)
{
	void * ptr = (void*) instance;
	free(ptr);
	return DWL_OK;
};

i32 DWLReserveHw(const void *instance, i32 *coreID)
{
	return DWL_OK;
};

i32 DWLReserveHwPipe(const void *instance, i32 *coreID)
{
	return DWL_OK;
};

void DWLReleaseHw(const void *instance, i32 coreID)
{ 
};

u32 DWLReadAsicCoreCount(void)
{
	return 1;
}


void DWLFreeRefFrm(const void *instance, DWLLinearMem_t *info)
{
	DWLFreeLinear(instance, info);
};

/* SW/HW shared memory */
i32 DWLMallocLinearNamed(const void *instance, u32 size, DWLLinearMem_t *info,
			 const char *name)
{
	void *addr;

	addr = malloc(size); // Take care to addr alignment
	if (!addr) {
		TRACE_ERR("allocation failed for size=%d\n", size);
		return DWL_ERROR;
	};

	info->size = size;
	info->busAddress = (u32)addr;
	info->virtualAddress = addr;
	info->name = name;

	//TRACE_INFO("allocate %d bytes of HW memory @(virt=0x%x, phy=0x%x): \n",
	//	size, (void *)info->virtualAddress,
	//	(void *)info->busAddress);

	return DWL_OK;
};

void DWLFreeLinear(const void *instance, DWLLinearMem_t *info)
{
	//TRACE_INFO("free %d bytes of HW memory @(virt=0x%x, phy=0x%x)\n",
	//	info->size, (void *)info->virtualAddress,
	//	(void *)info->busAddress);

	free(info->virtualAddress);
};


/* Frame buffers memory */
i32 DWLMallocRefFrmNamed(const void *instance, u32 size, DWLLinearMem_t *info,
			 const char *name)
{
	return DWLMallocLinearNamed(instance, size, info, name);
};

i32 DWLMallocRefFrm(const void *instance, u32 size, DWLLinearMem_t *info)
{
	return DWLMallocRefFrmNamed(instance, size, info, "");
};

i32 DWLMallocLinear(const void *instance, u32 size, DWLLinearMem_t *info)
{
	return DWLMallocLinearNamed(instance, size, info, "");
};

/* Register access */
void DWLWriteReg(const void *instance, i32 coreID, u32 offset, u32 value)
{
	struct dwl_ctx *ctx = to_ctx(instance);
	u32 regnum;

	regnum = offset / 4;

	ctx->regs_w_dec.offset[ctx->regs_w_dec.n] = regnum * 4;
	ctx->regs_w_dec.data[ctx->regs_w_dec.n] = value;
	ctx->regs_w_dec.n++;
};

u32 DWLReadReg(const void *instance, i32 coreID, u32 offset)
{
	struct dwl_ctx *ctx = to_ctx(instance);
	u32 val;

	/* get register from shadow read register list */
	val = ctx->regs_r_dec.data[offset / 4];

	return val;
};

void DWLWriteRegAll(const void *instance, const u32 *table, u32 size)
{
//	TRACE_ERR("DWLWriteRegAll() not implemented\n");
};

void DWLReadRegAll(const void *instance, u32 *table, u32 size)
{
//	TRACE_ERR("DWLReadRegAll() not implemented\n");
};

/* HW starting/stopping */
void DWLEnableHW(const void *instance, i32 coreID, u32 offset, u32 value)
{
	DWLWriteReg(instance, coreID, offset, value);
};

void DWLDisableHW(const void *instance, i32 coreID, u32 offset, u32 value)
{
	DWLWriteReg(instance, coreID, offset, value);
};

/* HW synchronization */
i32 DWLWaitHwReady(const void *instance, i32 coreID, u32 timeout)
{
	struct dwl_ctx *ctx = to_ctx(instance);
	int ret;

	if (!ctx->regs_w_dec.n)
		goto out;

	/* run hardware with pending write */
	ret = g1_hw_run(ctx, &ctx->regs_w_dec);
	if (ret) {
		TRACE_ERR("g1_hw_run error\n");
		return DWL_HW_WAIT_ERROR;  /*FIXME*/
	}

	/* save in shadow the last read registers *//*FIXME clarify the need */
	memcpy(&ctx->regs_r_dec, &ctx->regs_w_dec, sizeof(ctx->regs_r_dec));
	/* flush pending write */
	ctx->regs_w_dec.n = 0;

out:
	return DWL_OK;
};

void *DWLmalloc(u32 n)
{
	return calloc(1,(size_t)n);
}

void DWLfree(void *p)
{
	free(p);
}

void *DWLcalloc(u32 n, u32 s)
{
	return calloc((size_t)n, (size_t)s);
}

void *DWLmemcpy(void *d, const void *s, u32 n)
{
	return memcpy(d, s, (size_t)n);
}

void *DWLmemset(void *d, i32 c, u32 n)
{
	return memset(d, (int)c, (size_t)n);
}

