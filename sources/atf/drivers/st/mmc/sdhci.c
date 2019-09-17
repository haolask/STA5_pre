/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * SDHC 3.0 mmc driver in SDMA mode only
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <sdhci.h>
#include <errno.h>
#include <mmio.h>
#include <string.h>

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define debug(...)	printf(__VA_ARGS__)
#else
# define debug(...)
#endif

/** @brief SDHC registers offsets */
#define SDHC_DMA_ADDRESS	0x00
#define SDHC_BLOCK_SIZE		0x04
#define SDHC_BLOCK_COUNT	0x06
#define SDHC_ARGUMENT		0x08
#define SDHC_TRANSFER_MODE	0x0C
#define SDHC_COMMAND		0x0E
#define SDHC_RESP0		0x10
#define SDHC_RESP1		0x14
#define SDHC_RESP2		0x18
#define SDHC_RESP3		0x1C
#define SDHC_BUFFER		0x20
#define SDHC_PRESENT_STATE	0x24
#define SDHC_HOST_CONTROL	0x28
#define SDHC_POWER_CONTROL	0x29
#define SDHC_BLOCK_GAP_CONTROL	0x2A
#define SDHC_WAKE_UP_CONTROL	0x2B
#define SDHC_CLOCK_CONTROL	0x2C
#define SDHC_TIMEOUT_CONTROL	0x2E
#define SDHC_SOFTWARE_RESET	0x2F
#define SDHC_INT_STATUS		0x30
#define SDHC_INT_ENABLE		0x34
#define SDHC_SIGNAL_ENABLE	0x38

/* SDMA boundary block size set to 512KB */
#define SDHC_SDMA_BOUNDARY_SIZE		(512 * 1024)
#define SDHC_SDMA_BLKSZ_OFS		12
#define SDHC_SDMA_BOUNDARY_512KB	7

/* Power control reg values */
#define SDHC_POWER_ON		0x01
#define SDHC_POWER_180		0x0A
#define SDHC_POWER_300		0x0C
#define SDHC_POWER_330		0x0E

/* Clock control reg values */
#define SDHC_DIVIDER_SHIFT	8
#define SDHC_DIVIDER_HI_SHIFT	6
#define SDHC_DIV_MASK		0xFF
#define SDHC_DIV_MASK_LEN	8
#define SDHC_DIV_HI_MASK	0x300
#define SDHC_PROG_CLOCK_MODE	BIT(5)
#define SDHC_CLOCK_CARD_EN	BIT(2)
#define SDHC_CLOCK_INT_STABLE	BIT(1)
#define SDHC_CLOCK_INT_EN	BIT(0)

/* Softaware reset reg values */
#define SDHC_RESET_ALL		BIT(0)
#define SDHC_RESET_CMD		BIT(1)
#define SDHC_RESET_DATA		BIT(2)

/* Transfert mode reg values */
#define SDHC_TRANS_DMA		BIT(0)
#define SDHC_TRANS_BLK_CNT_EN	BIT(1)
#define SDHC_TRANS_READ		BIT(4)
#define SDHC_TRANS_MULTI	BIT(5)

#define SDHC_CMD_CRC		0x08
#define SDHC_CMD_INDEX		0x10
#define SDHC_CMD_DATA		0x20

#define SDHC_CMD_RESP_NONE	0x00
#define SDHC_CMD_RESP_LONG	0x01
#define SDHC_CMD_RESP_SHORT	0x02

#define SDHC_TIMEOUT_VALUE	0xE

/* Driver TYPE */
#define SDHC_SIGNALING_1V8	0x0008

/* Int enable register values */
#define SDHC_INT_CMD_COMPLETE	BIT(0)
#define SDHC_INT_DATA_END	BIT(1)
#define SDHC_INT_DMA_END	BIT(3)
#define SDHC_INT_SPACE_AVAIL	BIT(4)
#define SDHC_INT_DATA_AVAIL	BIT(5)
#define SDHC_INT_ERROR		BIT(15)

#define SDHC_ALL_INT_MASK	0xFFFFFFFF

/* Present state register values */
#define SDHC_CMD_INHIBIT	BIT(0)
#define SDHC_DATA_INHIBIT	BIT(1)
#define SDHC_SPACE_AVAILABLE	BIT(10)
#define SDHC_DATA_AVAILABLE	BIT(11)

/* Defines for Host controller */
#define SDHC_HOST_CNT_BW4	0x02
#define SDHC_HOST_CNT_BW8	0x20
#define SDHC_HOST_CNT_HS	0x04

#define SDHC_CMD(c, f)		((((c) & 0xff) << 8) | ((f) & 0xff))

static void sdhc_init(void);
static int sdhc_send_cmd(struct mmc_cmd *cmd);
static int sdhc_set_ios(unsigned int clk, unsigned int width);
static int sdhc_prepare(int lba, uintptr_t buf, size_t size);
static int sdhc_read(int lba, uintptr_t buf, size_t size);
static int sdhc_write(int lba, uintptr_t buf, size_t size);

static const struct mmc_ops sdhc_ops = {
	.init		= sdhc_init,
	.send_cmd	= sdhc_send_cmd,
	.set_ios	= sdhc_set_ios,
	.prepare	= sdhc_prepare,
	.read		= sdhc_read,
	.write		= sdhc_write,
};

static sdhc_params_t sdhc_params;

static void sdhc_set_clk(int clk)
{
	int div;
	uintptr_t base;
	uint16_t clkctrl = 0;
	uint16_t timeout;
	uint8_t hctrl;

	base = sdhc_params.reg_base;

	assert(clk > 0);

	for (div = 1; div < 1024; div++) {
		if ((sdhc_params.clk_rate / (div * 2)) <= clk)
			break;
	}
	assert(div < 1024);

	/* Wait max 20 ms */
	timeout = 200;
	while (mmio_read_32(base + SDHC_PRESENT_STATE) &
	       (SDHC_CMD_INHIBIT | SDHC_DATA_INHIBIT)) {
		if (timeout-- == 0) {
			debug("%s[%d]: timeout\n", __func__, __LINE__);
			return;
		}
		udelay(100);
	}

	hctrl = mmio_read_8(base + SDHC_HOST_CONTROL);
	if (clk > 26000000)
		hctrl |= SDHC_HOST_CNT_HS;
	else
		hctrl &= ~SDHC_HOST_CNT_HS;
	mmio_write_8(base + SDHC_HOST_CONTROL, hctrl);

	/* Disable clocks */
	mmio_write_16(base + SDHC_CLOCK_CONTROL, 0);
	/* Set divisor and enable internal clock */
	clkctrl |= (div & SDHC_DIV_MASK) << SDHC_DIVIDER_SHIFT;
	clkctrl |= ((div & SDHC_DIV_HI_MASK) >> SDHC_DIV_MASK_LEN)
			<< SDHC_DIVIDER_HI_SHIFT;
	clkctrl |= SDHC_CLOCK_INT_EN;
	mmio_write_16(base + SDHC_CLOCK_CONTROL, clkctrl);

	/* Polling for internal clock stable to be set during 20ms */
	timeout = 20;
	while (!(mmio_read_16(SDHC_CLOCK_CONTROL) & SDHC_CLOCK_INT_STABLE)) {
		if (timeout-- == 0) {
			debug("%s[%d]: timeout\n", __func__, __LINE__);
			return;
		}
		mdelay(1);
	}

	/* Enabling SD clock */
	mmio_write_16(base + SDHC_CLOCK_CONTROL,
		      mmio_read_16(SDHC_CLOCK_CONTROL) | SDHC_CLOCK_CARD_EN);

	debug("%s: div=%d, clkctrl=%x\n", __func__, div, clkctrl);
}

static void sdhc_reset(uintptr_t base, uint8_t mask)
{
	unsigned long timeout;

	/* Wait max 100 ms */
	timeout = 100;
	mmio_write_8(base + SDHC_SOFTWARE_RESET, mask);
	while (mmio_read_8(base + SDHC_SOFTWARE_RESET) & mask) {
		if (timeout == 0) {
			ERROR("%s: Reset 0x%x never completed.\n",
			       __func__, (int)mask);
			return;
		}
		timeout--;
		udelay(1000);
	}
}

static void sdhc_init(void)
{
	uintptr_t base;
	uint8_t power = 0;

	base = sdhc_params.reg_base;

	sdhc_reset(base, SDHC_RESET_ALL);

	mmio_write_8(base + SDHC_TIMEOUT_CONTROL, SDHC_TIMEOUT_VALUE);
	mmio_write_8(base + SDHC_HOST_CONTROL, 0); /* Low speed 1bit bus & SDMA */
	/* Enable all IT and mask all */
	mmio_write_32(base + SDHC_INT_ENABLE, SDHC_ALL_INT_MASK);
	mmio_write_32(base + SDHC_SIGNAL_ENABLE, 0);

	switch (sdhc_params.io_volt) {
	case IO_VOLT_330:
		power = SDHC_POWER_330;
		break;
	case IO_VOLT_180:
		power = SDHC_POWER_180;
		break;
	}

	power |= SDHC_POWER_ON;
	do {
		mmio_write_8(base + SDHC_POWER_CONTROL, power); /* Power ON */
	} while (!(mmio_read_8(base + SDHC_POWER_CONTROL) & SDHC_POWER_ON));

	sdhc_set_clk(MMC_BOOT_CLK_RATE);
}

static int sdhc_send_cmd(struct mmc_cmd *cmd)
{
	uintptr_t base;
	uint32_t status = 0;
	uint32_t mask, flags;
	uint32_t timeout;
	int ret = 0;

	assert(cmd);

	debug("%s: CMD%d, ARG: %x\n", __func__, cmd->cmd_idx, cmd->cmd_arg);

	base = sdhc_params.reg_base;

	debug("%s: CMD%d, ARG:%x, ", __func__, cmd->cmd_idx, cmd->cmd_arg);

	mmio_write_32(base + SDHC_INT_STATUS, SDHC_ALL_INT_MASK);

	mask = SDHC_CMD_INHIBIT | SDHC_DATA_INHIBIT;
	if (cmd->cmd_idx == MMC_CMD(12))
		mask &= ~SDHC_DATA_INHIBIT;

	timeout = 2000;  /* 2sec timeout */
	while (mmio_read_32(base + SDHC_PRESENT_STATE) & mask) {
		if (!timeout--) {
			ERROR("%s: SDHC busy timeout\n", __func__);
			return -ETIMEDOUT;
		}
		mdelay(1);
	}

	mask = SDHC_INT_CMD_COMPLETE;
	if (!cmd->resp_type) {
		flags = SDHC_CMD_RESP_NONE;
	} else {
		if (cmd->resp_type == MMC_RESPONSE_R2) {
			flags = SDHC_CMD_RESP_LONG;
		} else {
			flags = SDHC_CMD_RESP_SHORT;
		}
		if ((cmd->resp_type != MMC_RESPONSE_R3) &&
		    (cmd->resp_type != MMC_RESPONSE_R4))
			flags |= SDHC_CMD_CRC;
	}

	/* Manage Data flag following CMD ID */
	switch (cmd->cmd_idx) {
	case MMC_CMD(17):
	case MMC_CMD(18):
	case MMC_CMD(24):
	case MMC_CMD(25):
		flags |= SDHC_CMD_DATA;
		break;
	}

	/* Update tansfert mode if data read */
	if ((cmd->cmd_idx == MMC_CMD(17)) || (cmd->cmd_idx == MMC_CMD(18)))
		mmio_write_16(base + SDHC_TRANSFER_MODE,
			      mmio_read_16(base + SDHC_TRANSFER_MODE) |
			      SDHC_TRANS_READ);


	mmio_write_32(base + SDHC_ARGUMENT, cmd->cmd_arg);
	mmio_write_16(base + SDHC_COMMAND, SDHC_CMD(cmd->cmd_idx, flags));
	timeout = 100000; /* 1sec timeout */
	do {
		status =  mmio_read_32(base + SDHC_INT_STATUS);
		if (status & SDHC_INT_ERROR) {
			debug("%s[%d]: ERROR, status:%x\n",
			      __func__, __LINE__, status);
			mmio_write_32(base + SDHC_INT_STATUS,
				      SDHC_ALL_INT_MASK);
			sdhc_reset(base, SDHC_RESET_CMD | SDHC_RESET_DATA);

			return -EIO;
		}

		if (!timeout--) {
			debug("%s[%d]: TIMEOUT, status:%x\n",
			      __func__, __LINE__, status);
			return -ETIMEDOUT;
		}
		udelay(10);
	} while ((status & mask) != mask);

	if (cmd->resp_type == MMC_RESPONSE_R2) {
		/* CRC is stripped, need to do some shift */
		cmd->resp_data[0] = (mmio_read_32(base + SDHC_RESP3) << 8) |
			(mmio_read_32(base + SDHC_RESP2) >> 24);
		cmd->resp_data[1] = (mmio_read_32(base + SDHC_RESP2) << 8) |
			(mmio_read_32(base + SDHC_RESP1) >> 24);
		cmd->resp_data[2] = (mmio_read_32(base + SDHC_RESP1) << 8) |
			(mmio_read_32(base + SDHC_RESP0) >> 24);
		cmd->resp_data[3] = (mmio_read_32(base + SDHC_RESP0) << 8);

		debug("R0:%x, R1:%x, R2:%x, R3:%x\n",
		      cmd->resp_data[0], cmd->resp_data[1],
		      cmd->resp_data[2], cmd->resp_data[3]);
	} else {
		cmd->resp_data[0] = mmio_read_32(base + SDHC_RESP0);
		debug("\t< R0:%x\n", cmd->resp_data[0]);
	}

	mmio_write_32(base + SDHC_INT_STATUS, SDHC_ALL_INT_MASK);

	return ret;
}

static int sdhc_set_ios(unsigned int clk, unsigned int width)
{
	uint32_t hctrl;

	hctrl = mmio_read_8(sdhc_params.reg_base + SDHC_HOST_CONTROL);
	switch (width) {
	case MMC_BUS_WIDTH_1:
		hctrl &= ~(SDHC_HOST_CNT_BW4 | SDHC_HOST_CNT_BW8);
		break;

	case MMC_BUS_WIDTH_4:
		hctrl |= SDHC_HOST_CNT_BW4;
		hctrl &= ~SDHC_HOST_CNT_BW8;
		break;

	case MMC_BUS_WIDTH_8:
		hctrl &= ~SDHC_HOST_CNT_BW4;
		hctrl |= SDHC_HOST_CNT_BW8;
		break;

	default:
		assert(0);
	}
	mmio_write_8(sdhc_params.reg_base + SDHC_HOST_CONTROL, hctrl);
	sdhc_set_clk(clk);

	return 0;
}

static int sdhc_prepare(int lba, uintptr_t buf, size_t size)
{
	uintptr_t base;
	uint16_t transfert_mode;

	assert(((buf & MMC_BLOCK_MASK) == 0) &&
	       ((size % MMC_BLOCK_SIZE) == 0));

	base = sdhc_params.reg_base;

	/* Always use SDMA  & block count */
	transfert_mode = SDHC_TRANS_BLK_CNT_EN | SDHC_TRANS_DMA;
	if (size > MMC_BLOCK_SIZE) /* Multi block transfert ? */
		transfert_mode |= SDHC_TRANS_MULTI;

	mmio_write_32(base + SDHC_DMA_ADDRESS, buf);
	mmio_write_16(base + SDHC_BLOCK_SIZE, MMC_BLOCK_SIZE |
		      (SDHC_SDMA_BOUNDARY_512KB << SDHC_SDMA_BLKSZ_OFS));
	mmio_write_16(base + SDHC_BLOCK_COUNT, size / MMC_BLOCK_SIZE);
	mmio_write_16(base + SDHC_TRANSFER_MODE, transfert_mode);

	return 0;
}

static int sdhc_read(int lba, uintptr_t buf, size_t size)
{
	uintptr_t base;
	uint32_t status, timeout = 500000;
	int ret = 0;

	base = sdhc_params.reg_base;

	debug("%s: size=%u\n", __func__, size);

	do {
		status = mmio_read_32(base + SDHC_INT_STATUS);
		if (status & SDHC_INT_ERROR) {
			ERROR("Status error: %x\n", status);
			return -EIO;
		}
		if (status & SDHC_INT_DMA_END) {
			mmio_write_32(base + SDHC_INT_STATUS, SDHC_INT_DMA_END);
			buf &= ~(SDHC_SDMA_BOUNDARY_SIZE - 1);
			buf += SDHC_SDMA_BOUNDARY_SIZE;
			mmio_write_32(base + SDHC_DMA_ADDRESS, buf);
		}
		if (timeout-- > 0) {
			udelay(10);
		} else {
			return -ETIMEDOUT;
		}
	} while ((status & SDHC_INT_DATA_END) == 0);

	mmio_write_32(base + SDHC_INT_STATUS, SDHC_ALL_INT_MASK);

	return ret;
}

static int sdhc_write(int lba, uintptr_t buf, size_t size)
{
	uintptr_t base;
	uint32_t status, timeout = 500000;
	int ret = 0;

	base = sdhc_params.reg_base;

	debug("%s: size=%u\n", __func__, size);

	do {
		status = mmio_read_32(base + SDHC_INT_STATUS);
		if (status & SDHC_INT_ERROR) {
			ERROR("Status error: %x\n", status);
			return -EIO;
		}
		if (status & SDHC_INT_DMA_END) {
			mmio_write_32(base + SDHC_INT_STATUS, SDHC_INT_DMA_END);
			buf &= ~(SDHC_SDMA_BOUNDARY_SIZE - 1);
			buf += SDHC_SDMA_BOUNDARY_SIZE;
			mmio_write_32(base + SDHC_DMA_ADDRESS, buf);
		}
		if (timeout-- > 0) {
			udelay(10);
		} else {
			return -ETIMEDOUT;
		}
	} while ((status & SDHC_INT_DATA_END) == 0);

	mmio_write_32(base + SDHC_INT_STATUS, SDHC_ALL_INT_MASK);

	return ret;
}

int sta_sdhc_init(sdhc_params_t *params)
{
	assert((params != 0) &&
	       (params->clk_rate > 0) &&
	       ((params->bus_width == MMC_BUS_WIDTH_1) ||
		(params->bus_width == MMC_BUS_WIDTH_4) ||
		(params->bus_width == MMC_BUS_WIDTH_8)));

	memcpy(&sdhc_params, params, sizeof(sdhc_params_t));
	return mmc_init(&sdhc_ops, 50000000, params->bus_width,
			params->flags, &params->device_info);
}
