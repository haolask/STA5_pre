/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __QSPI_H__
#define __QSPI_H__

#define QSPI_MODE_SPI				0x0 /* CAD: 1/1/1 wire */
#define QSPI_MODE_QPI				0x1 /* CAD: 4/4/4 wires */
#define QSPI_MODE_QSPI				0x2 /* CAD: 1/4/4 wires */
#define QSPI_MODE_QSPI2				0x3 /* CAD: 1/1/4 wires */

void sta_qspi_init(uintptr_t regs_base, uintptr_t mmap_base);
void sta_qspi_deinit();

/*
 * QSPI_QUAD_MODE values:
 *	QSPI_MODE_QSPI (CMD: 1 wire, ADDR/DATA: 4 wires)
 *	QSPI_MODE_QPI (CMD/ADDR/DATA: 4 wires) but it's not compatible with ROM CODE
 * after reset or watchdog
 */
#define QSPI_QUAD_MODE	QSPI_MODE_QPI

#endif	/* __QSPI_H__ */
