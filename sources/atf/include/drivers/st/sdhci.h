/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SDHC_H__
#define __SDHC_H__

#include <mmc.h>

typedef struct sdhc_params {
	uintptr_t	reg_base;
	int		clk_rate;
	int		bus_width;
	unsigned int	flags;
	unsigned int	io_volt;
	struct mmc_device_info	device_info;
} sdhc_params_t;

#define IO_VOLT_180		0x1  /* 1.8V IO voltage */
#define IO_VOLT_330		0x2  /* 3.3V IO voltage */

int sta_sdhc_init(sdhc_params_t *params);

#endif	/* __SDHC_H__ */
