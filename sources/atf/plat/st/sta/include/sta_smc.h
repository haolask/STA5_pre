/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STA_SMC_H__
#define __STA_SMC_H__

#include <platform_def.h>

#define STA_REGMAP_SMC			0x82000000
#define STA_REGMAP_SMC_TYPE_MSK		0xFF
#define STA_REGMAP_SMC_READ		0x0
#define STA_REGMAP_SMC_WRITE		0x1
#define STA_REGMAP_SMC_UPDATE_BITS	0x2

#define STA_REGMAP_SMC_VALBITS_8	8
#define STA_REGMAP_SMC_VALBITS_16	16
#define STA_REGMAP_SMC_VALBITS_32	32
#define STA_REGMAP_SMC_VALBITS_64	64

/* SMC error codes */
#define STA_SMC_OK			0x0
#define STA_SMC_NOT_SUPPORTED		-1
#define STA_SMC_FAILED			-2
#define STA_SMC_INVALID_PARAMS		-3

struct regmap_request {
	uint8_t type;
	uint8_t status;
	uint8_t val_bits;
	uint8_t padding;
	uint32_t reg;
	uint32_t mask;
	uint32_t val;
};

#endif /* __STA_SMC_H__ */
