/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/slab.h>
#include <linux/dma-mapping.h>

#include "stack/inc/dwl.h"
#include "g1.h"
#include "g1-debug.h"

/* define irq registers offset */
#define G1_HW_DEC_IRQ_OFFSET	(1 * 4)
#define G1_HW_PP_IRQ_OFFSET	(60 * 4)

#define to_dwl(inst) ((struct dwl_ctx *)inst)
#define to_ctx(inst) (((struct dwl_ctx *)inst)->ctx)
#define to_g1(inst)  (((struct dwl_ctx *)inst)->ctx->dev)

struct dwl_ctx {
	struct g1_ctx *ctx;
#ifndef DEC_PP_COMMON_SHADOW_REGS
	/* FIXME shadow registers need to be shared between dec & pp
	 * instead having their own shadowed registers.
	 * This point is not understood and need investigations
	 */
	struct g1_hw_regs regs_w_dec;
	struct g1_hw_regs regs_r_dec;
#endif
};

/* globals needed because of DWL APIs without context in arg */
u32 dwl_asic_id;
struct g1_hw_config dwl_config;
struct g1_hw_fuse dwl_fuse;

/* API added to solve globals initialization */
void DWLSetup(void *userCtx)
{
	struct g1_ctx *ctx;
	struct g1_dev *g1;

	BUG_ON(!userCtx);
	ctx = (struct g1_ctx *)userCtx;
	g1 = ctx->dev;

	dwl_asic_id = g1_hw_cfg_get_asic_id(g1);
	g1_hw_cfg_get_config(g1, &dwl_config);
	g1_hw_cfg_get_fuse(g1, &dwl_fuse);
}

/* API added to let throw context to DWL */
void DWLSetUserContext(const void *instance, void *userCtx)
{
	struct dwl_ctx *dwl = to_dwl(instance);

	BUG_ON(!dwl);
	BUG_ON(!userCtx);

	/* get back g1 instance context from dwl instance */
	dwl->ctx = (struct g1_ctx *)userCtx;
}

u32 DWLReadAsicID(void)
{
	return dwl_asic_id;
};

void DWLReadAsicConfig(DWLHwConfig_t *pHwCfg)
{
	/* check alignment of both structure.
	 * g1-hw.h didn't include dwl.h to not add
	 * fuzzy dependence on g1-hw.c/.h that we want
	 * independent of core legacy code.
	 */
	BUG_ON(sizeof(DWLHwConfig_t) != sizeof(struct g1_hw_config));

	memcpy(pHwCfg, &dwl_config, sizeof(*pHwCfg));
};

const void *DWLInit(DWLInitParam_t *param)
{
	struct dwl_ctx *dwl;
#ifndef DEC_PP_COMMON_SHADOW_REGS
	unsigned int i;
#endif

	BUG_ON(!param);
#if !defined(CONFIG_VIDEO_ST_G1_PP)
	/* post-processor not supported */
	BUG_ON(param->clientType == DWL_CLIENT_TYPE_PP);
#endif
	dwl = kzalloc(sizeof(*dwl), GFP_KERNEL);
	if (!dwl)
		return NULL;

#ifndef DEC_PP_COMMON_SHADOW_REGS
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
#endif

	return dwl;
};

i32 DWLRelease(const void *instance)
{
	kfree(instance);
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

void DWLFreeRefFrm(const void *instance, DWLLinearMem_t *info)
{
	DWLFreeLinear(instance, info);
};

/* SW/HW shared memory */
i32 DWLMallocLinearNamed(const void *instance, u32 size, DWLLinearMem_t *info,
			 const char *name)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);
	dma_addr_t dma_addr;
	void *addr;
	struct mem_info mem_info;

	/* align image size to PAGE_SIZE */
	size = PAGE_ALIGN(size);

	addr = dma_alloc_coherent(g1->dev, size, &dma_addr, GFP_KERNEL);
	if (!addr) {
		dev_err(g1->dev,
			"%s    dma_alloc_coherent failed for size=%d\n",
			ctx->name, size);
		return DWL_ERROR;
	};

	info->size = size;
	info->busAddress = dma_addr;
	info->virtualAddress = addr;
	info->name = name;

	mem_info.size = size;
	snprintf(mem_info.name, sizeof(mem_info.name), "%s", name);

	dev_dbg(g1->dev, "%s    allocate %d bytes of HW memory @(virt=0x%p, phy=0x%p): %s\n",
		ctx->name, size, (void *)info->virtualAddress,
		(void *)info->busAddress, info->name);

	g1_debugfs_track_mem(&mem_info, ctx, "dma");

	return DWL_OK;
};

i32 DWLMallocLinear(const void *instance, u32 size, DWLLinearMem_t *info)
{
	return DWLMallocLinearNamed(instance, size, info, "");
};

void DWLFreeLinear(const void *instance, DWLLinearMem_t *info)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);

	dev_dbg(g1->dev,
		"%s    free %d bytes of HW memory @(virt=0x%p, phy=0x%p): %s\n",
		ctx->name, info->size, (void *)info->virtualAddress,
		(void *)info->busAddress, info->name);

	dma_free_coherent(g1->dev, info->size,
			  info->virtualAddress, info->busAddress);
};

/* Register access */
void DWLWriteReg(const void *instance, i32 coreID, u32 offset, u32 value)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);
#ifndef DEC_PP_COMMON_SHADOW_REGS
	struct dwl_ctx *dwl = to_dwl(instance);
#else
	struct g1_ctx *dwl = to_ctx(instance);
#endif
	u32 regnum;

	dev_vdbg(g1->dev, "%s    write reg %d at offset 0x%02X --> %08X\n",
		 ctx->name, offset / 4, offset, value);

	BUG_ON(offset >= g1->hw.regs_size);

	regnum = offset / 4;

	dwl->regs_w_dec.offset[dwl->regs_w_dec.n] = regnum * 4;
	dwl->regs_w_dec.data[dwl->regs_w_dec.n] = value;
	dwl->regs_w_dec.n++;

	BUG_ON(dwl->regs_w_dec.n > G1_MAX_REGS);
};

u32 DWLReadReg(const void *instance, i32 coreID, u32 offset)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);
#ifndef DEC_PP_COMMON_SHADOW_REGS
	struct dwl_ctx *dwl = to_dwl(instance);
#else
	struct g1_ctx *dwl = to_ctx(instance);
#endif
	u32 val;

	BUG_ON(offset >= g1->hw.regs_size);

	/* get register from shadow read register list */
	val = dwl->regs_r_dec.data[offset / 4];

	dev_vdbg(g1->dev, "%s    read reg %d at offset 0x%02X --> %08X\n",
		 ctx->name, offset / 4, offset, val);
	return val;
};

void DWLWriteRegAll(const void *instance, const u32 *table, u32 size)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);

	dev_warn(g1->dev, "%s DWLWriteRegAll() not implemented\n", ctx->name);
};

void DWLReadRegAll(const void *instance, u32 *table, u32 size)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);

	dev_warn(g1->dev, "%s DWLReadRegAll() not implemented\n", ctx->name);
};

/* HW starting/stopping */
void DWLEnableHW(const void *instance, i32 coreID, u32 offset, u32 value)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);

	dev_dbg(g1->dev, "%s    > %s\n", ctx->name, __func__);
	DWLWriteReg(instance, coreID, offset, value);
	dev_dbg(g1->dev, "%s    < %s\n", ctx->name, __func__);
};

void DWLDisableHW(const void *instance, i32 coreID, u32 offset, u32 value)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);

	dev_dbg(g1->dev, "%s    > %s\n", ctx->name, __func__);
	DWLWriteReg(instance, coreID, offset, value);
	dev_dbg(g1->dev, "%s    < %s\n", ctx->name, __func__);
};

/* HW synchronization */
i32 DWLWaitHwReady(const void *instance, i32 coreID, u32 timeout)
{
	struct g1_dev *g1 = to_g1(instance);
	struct g1_ctx *ctx = to_ctx(instance);
#ifndef DEC_PP_COMMON_SHADOW_REGS
	struct dwl_ctx *dwl = to_dwl(instance);
#else
	struct g1_ctx *dwl = to_ctx(instance);
#endif
	int ret;

	dev_dbg(g1->dev, "%s    > %s\n", ctx->name, __func__);

	dev_dbg(g1->dev, "%s    %d regs to write", ctx->name,
		dwl->regs_w_dec.n);

	if (!dwl->regs_w_dec.n)
		goto out;

	/* run hardware with pending write */
	ret = g1_hw_run(ctx, &dwl->regs_w_dec);
	if (ret) {
		dev_err(g1->dev, "%s    g1_hw_run error\n", ctx->name);
		return DWL_HW_WAIT_ERROR;  /*FIXME*/
	}

	/* save in shadow the last read registers *//*FIXME clarify the need */
	memcpy(&dwl->regs_r_dec, &dwl->regs_w_dec, sizeof(dwl->regs_r_dec));
	/* flush pending write */
	dwl->regs_w_dec.n = 0;

out:
	dev_dbg(g1->dev, "%s    < %s\n", ctx->name, __func__);
	return DWL_OK;
};

void *DWLmalloc(u32 n)
{
	return kzalloc((size_t)n, GFP_KERNEL);
}

void DWLfree(void *p)
{
	kfree(p);
}

void *DWLcalloc(u32 n, u32 s)
{
	return kcalloc((size_t)n, (size_t)s, GFP_KERNEL);
}

void *DWLmemcpy(void *d, const void *s, u32 n)
{
	return memcpy(d, s, (size_t)n);
}

void *DWLmemset(void *d, i32 c, u32 n)
{
	return memset(d, (int)c, (size_t)n);
}

/* FIXME TMP stub pthread */
int pthread_mutex_init(pthread_mutex_t *mutex,
		       const pthread_mutexattr_t *attr) {return 0; };
int pthread_mutex_destroy(pthread_mutex_t *mutex) {return 0; };
int pthread_mutex_lock(pthread_mutex_t *mutex) {return 0; };
int pthread_mutex_unlock(pthread_mutex_t *mutex) {return 0; };

int pthread_cond_init(pthread_cond_t *cond,
		      const pthread_condattr_t *attr) {return 0; };
int pthread_cond_destroy(pthread_cond_t *cond) {return 0; };
int pthread_cond_signal(pthread_cond_t *cond) {return 0; };
int pthread_cond_wait(pthread_cond_t *cond,
		      pthread_mutex_t *mutex) {return 0; };

/* FIXME TMP stub sem */
int sem_init(sem_t *sem, int pshared, unsigned int value) {return 0; };
int sem_post(sem_t *sem) {return 0; };
int sem_wait(sem_t *sem) {return 0; };
int sem_destroy(sem_t *sem) {return 0; };
