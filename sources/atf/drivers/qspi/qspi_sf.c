/*
 * Copyright (C) ST-Microelectronics SA 2017.
 * @author: ADG-MID team
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Defines a simple and generic interface to access QSPI Serial Flash device.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <qspi_sf.h>
#include <errno.h>
#include <string.h>
#include <utils.h>

static const qspi_sf_ops_t *ops;

size_t qspi_read_blocks(int lba, uintptr_t buf, size_t size)
{
	assert((ops != 0) &&
	       (ops->read != 0));

	inv_dcache_range(buf, size);

	return ops->read(lba, buf, size);
}

size_t qspi_write_blocks(int lba, const uintptr_t buf, size_t size)
{
	int ret;

	assert((ops != 0) &&
	       ((buf & 3) == 0) &&
	       ((size & 3) == 0));

	if (ops->write) {
		clean_dcache_range(buf, size);

		ret = ops->write(lba, buf, size);
		assert(ret == 0);
	} else {
		ERROR("%s: not supported\n", __func__);
		return 0;
	}

	/* Ignore improbable errors in release builds */
	(void)ret;
	return size;
}

size_t qspi_erase_blocks(int lba, size_t size)
{
	int ret;

	assert((ops != 0));

	if (ops->erase) {
		ret = ops->erase(lba, size);
		assert(ret == 0);
	} else {
		ERROR("%s: not supported\n", __func__);
		return 0;
	}

	/* Ignore improbable errors in release builds */
	(void)ret;
	return size;
}

void qspi_init(const qspi_sf_ops_t *ops_ptr,
	       uintptr_t regs_base, uintptr_t mmap_base)
{
	assert((ops_ptr != 0) &&
	       (ops_ptr->init != 0) &&
	       (ops_ptr->read != 0) &&
	       (regs_base != 0) &&
	       (mmap_base != 0));
	ops = ops_ptr;

	ops_ptr->init(regs_base, mmap_base);
}

void qspi_deinit(const qspi_sf_ops_t *ops_ptr)
{
	assert((ops_ptr != 0) &&
	       (ops_ptr->deinit != 0));

	ops_ptr->deinit();
}

