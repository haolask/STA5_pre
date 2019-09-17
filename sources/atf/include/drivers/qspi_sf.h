/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __QSPI_SF_H__
#define __QSPI_SF_H__

#include <stdint.h>

typedef struct qspi_sf_ops {
	void (*init)(uintptr_t regs_base, uintptr_t mmap_base);
	void (*deinit)(void);
	int (*read)(int lba, uintptr_t buf, size_t size);
	int (*write)(int lba, const uintptr_t buf, size_t size);
	int (*erase)(int lba, size_t size);
} qspi_sf_ops_t;

size_t qspi_read_blocks(int lba, uintptr_t buf, size_t size);
size_t qspi_write_blocks(int lba, const uintptr_t buf, size_t size);
size_t qspi_erase_blocks(int lba, size_t size);
void qspi_init(const qspi_sf_ops_t *ops,
	       uintptr_t regs_base, uintptr_t mmap_base);
void qspi_deinit(const qspi_sf_ops_t *ops);
#endif	/* __QSPI_SF_H__ */
