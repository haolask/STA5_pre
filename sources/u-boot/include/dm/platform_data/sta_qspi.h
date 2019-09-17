/*
 * Copyright (C) 2016 STMicrolelctronics, <www.st.com>
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _STA_QSPI_PLAT_H
#define _STA_QSPI_PLAT_H

/**
 * struct sta_qspi_platdata - platform data for STA QSPI
 *
 * @speed_hz: Default SCK frequency
 * @reg_base: Base address of QSPI registers
 * @mmap_base: Base address of QSPI memory mapping
 * @mmap_total_size: size of QSPI memory mapping
 */
struct sta_qspi_platdata {
	uint32_t speed_hz;
	fdt_addr_t reg_base;
	fdt_addr_t mmap_base;
	fdt_size_t mmap_total_size;
};

#endif
