/*
 * Register map access API - Secure Monitor Call support
 *
 * Copyright (c) 2018, STMicroelectronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __SOC_STA_REGMAP_SMC_H
#define __SOC_STA_REGMAP_SMC_H

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

#endif
