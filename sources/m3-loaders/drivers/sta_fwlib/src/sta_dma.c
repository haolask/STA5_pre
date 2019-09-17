/**
 * @file sta_dma.c
 * @brief This file provides the DMA functions.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID team
 */

#include <errno.h>

#include "sta_dma.h"
#include "sta_nvic.h"
#include "sta_common.h"
#include "trace.h"

typedef  void (*dma_callback_t)(int);

struct sta_dma {
	uint32_t channel_mask;
	dma_callback_t callback[DMA_CHANNELS];
} sta_dma0;

/* In M3 we only enable DMA0 */
int dma0_request(int chan, void (*callback) (int))
{
	struct nvic_chnl irq_chnl;

	/* bits 19-20 DMAC_CxCFG used to lock a chan in Linux PL080 driver */
	dma0_regs->DMACChannelReg[chan].DMACCConfiguration.reserved2 = 3;
	sta_dma0.callback[chan] = callback;

	if (callback)
		sta_dma0.channel_mask |= (1 << chan);
	else
		sta_dma0.channel_mask &= ~(1 << chan);

	/* EXT4 IRQ is associated to DMA0 cfr sta_m3_irq.c */
	irq_chnl.id = EXT4_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = !!sta_dma0.channel_mask;

	return nvic_chnl_init(&irq_chnl);
}

void dma0_irq_handler(void)
{
	uint32_t err, tc;
	int i, status = 0;

	err = dma0_regs->DMACIntErrorStatus & sta_dma0.channel_mask;
	if (err) {
		TRACE_ERR("dma0 irq err 0x%08x\n", err);
		dma0_regs->DMACIntErrClr = err;
		status = -EIO;
	}

	tc = dma0_regs->DMACIntTCStatus & sta_dma0.channel_mask;
	if (tc)
		dma0_regs->DMACIntTCClear = tc;

	for (i = 0; i < DMA_CHANNELS; i++) {
		if (((1 << i) & err) || ((1 << i) & tc))
			if (sta_dma0.callback[i])
				sta_dma0.callback[i](status);
	}
}
