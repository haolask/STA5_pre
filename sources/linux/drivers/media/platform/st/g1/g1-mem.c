/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Chetan Nanda <chetan.nanda@st.com>
 *          Hugues Fruchet <hugues.fruchet@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include "g1.h"
#include "g1-debug.h"
#include "g1-mem.h"

int hw_alloc(struct g1_ctx *ctx, __u32 size, const char *name,
	     struct g1_buf *buf)
{
	struct g1_dev *g1 = ctx->dev;
	dma_addr_t dma_addr;
	void *addr;
	unsigned long attrs;
	struct mem_info mem_info;

	attrs = DMA_ATTR_WRITE_COMBINE;

	addr = dma_alloc_attrs(g1->dev, size, &dma_addr,
			       GFP_KERNEL | __GFP_NOWARN, attrs);
	if (!addr) {
		dev_err(g1->dev,
			"%s hw_alloc:dma_alloc_coherent failed for %s (size=%d)\n",
			ctx->name, name, size);
		ctx->sys_errors++;
		return -ENOMEM;
	};

	buf->size = size;
	buf->paddr = dma_addr;
	buf->vaddr = addr;
	buf->name = name;
	buf->attrs = attrs;

	mem_info.size = size;
	snprintf(mem_info.name, sizeof(mem_info.name), "%s", name);

	dev_dbg(g1->dev,
		"%s allocate %d bytes of HW memory @(virt=0x%p, phy=0x%p): %s\n",
		ctx->name, size, (void *)buf->vaddr,
		(void *)buf->paddr, buf->name);

	g1_debugfs_track_mem(&mem_info, ctx, "dma");
	return 0;
}

void hw_free(struct g1_ctx *ctx, struct g1_buf *buf)
{
	struct g1_dev *g1 = ctx->dev;

	dev_dbg(g1->dev,
		"%s     free %d bytes of HW memory @(virt=0x%p, phy=0x%p): %s\n",
		ctx->name, buf->size, (void *)buf->vaddr,
		(void *)buf->paddr, buf->name);

	dma_free_attrs(g1->dev, buf->size,
		       buf->vaddr, buf->paddr, buf->attrs);
}
