/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MMCI_H__
#define __MMCI_H__

#include <mmc.h>

typedef struct mmci_params {
	uintptr_t	reg_base;
	int		clk_rate;
	int		bus_width;
	unsigned int	flags;
	struct mmc_device_info	device_info;
} mmci_params_t;

int sta_mmci_init(mmci_params_t *params);

uintptr_t mmc_fast_read(uintptr_t outBuf, uintptr_t sdi_fifo_regs);
uintptr_t mmc_fast_write(uintptr_t inBuf, uintptr_t sdi_fifo_regs);

#endif	/* __MMCI_H__ */
