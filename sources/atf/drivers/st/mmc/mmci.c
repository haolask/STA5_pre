/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmci.h>
#include <errno.h>
#include <mmio.h>
#include <string.h>

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define debug(...)	printf(__VA_ARGS__)
#else
# define debug(...)
#endif

#define CMD_REG_UDELAY		300
#define DATA_REG_UDELAY		30
#define CLK_CHANGE_UDELAY	30

/** @brief MMCI registers offsets */
#define MMCI_POWER		0x00
#define MMCI_CLOCK		0x04
#define MMCI_ARGUMENT		0x08
#define MMCI_COMMAND		0x0C
#define MMCI_RESP_COMMAND	0x10
#define MMCI_RESP0		0x14
#define MMCI_RESP1		0x18
#define MMCI_RESP2		0x1C
#define MMCI_RESP3		0x20
#define MMCI_DATA_TIMER		0x24
#define MMCI_DATA_LENGTH	0x28
#define MMCI_DATA_CTRL		0x2C
#define MMCI_DATA_COUNT		0x30
#define MMCI_STATUS		0x34
#define MMCI_CLEAR		0x38
#define MMCI_MASK0		0x3C
#define MMCI_SELECT		0x44
#define MMCI_FIFO_COUNT		0x48
#define MMCI_FIFO		0X80

/* Power Control register */
#define MMCI_POWERON			0x00000003
#define MMCI_OPENDRAIN			0x00000040

#define MMCI_FDBCLK_DIS			BIT(7)
#define MMCI_CMDDATEN			0x0000003C
#define MMCI_LOW_VOLTAGE_WINDOW		0x00000080
#define MMCI_HIGH_VOLTAGE_WINDOW	0x00FF8000
#define MMCI_HC_WINDOW			0x40000000
#define MMCI_SD_CMD8_HV_WINDOW		0x00000100
#define MMCI_SD_CMD8_LV_WINDOW		0x00000200
#define MMCI_SD_CMD8_CHECK_PATTERN	0x000000AA

/* Clock Control register */
#define MMCI_CLKDIVMASK			0x000000FF
#define MMCI_CLKDIVTRANS		BIT(0)
#define MMCI_CLKEN			BIT(8)
#define MMCI_PWRSAVE			BIT(9)
#define MMCI_BYPASS			BIT(10)
#define MMCI_HWFC_EN			BIT(14)
#define MMCI_CLKGATE_DIS		BIT(16)
#define MMCI_CLKCR_WBUS_MASK		0x00001800
#define MMCI_CLKCR_WBUS_1		0x00000000
#define MMCI_CLKCR_WBUS_4		0x00000800
#define MMCI_CLKCR_WBUS_8		0x00001000

/* Command control register */
#define MMCI_CMD_RESPEXPECTED		BIT(6)
#define MMCI_CMD_LONGRESP		BIT(7)
#define MMCI_CMD_ENABLEINTRREQ		BIT(8)
#define MMCI_CMD_ENABLECMDPEND		BIT(9)
#define MMCI_CMD_CPSMEN			BIT(10)
#define MMCI_CMD_SDIOSUSPEND		BIT(11)

#define MMCI_DATATIMEOUT		0xFFFF0000

/* Data control register */
#define MMCI_DCTRL_DTEN			BIT(0)
#define MMCI_DCTRL_DTREAD		BIT(1)
#define MMCI_DCTRL_DTMODE_STREAM	BIT(2)
#define MMCI_DCTRL_DMAEN		BIT(3)
#define MMCI_DCTRL_DBLKSIZE_MASK	0x000000F0
#define MMCI_DCTRL_BUSYMODE		BIT(14)
#define MMMMCCTRL_DBLOCKSIZE_MASK	0x7FFF0000
#define MMCI_DCTRL_DBLOCKSIZE_SHIFT	16

/* Mask0 register */
#define MMCI_MASK0_MASK		0x1FFFFFFF

/* Status register */
#define MMCI_CMDCRCFAIL		BIT(0)
#define MMCI_DATACRCFAIL	BIT(1)
#define MMCI_CMDTIMEOUT		BIT(2)
#define MMCI_STS_DATATIMEOUT	BIT(3)
#define MMCI_TXUNDERRUN		BIT(4)
#define MMCI_RXOVERRUN		BIT(5)
#define MMCI_CMDRESPEND		BIT(6)
#define MMCI_CMDSENT		BIT(7)
#define MMCI_DATAEND		BIT(8)
#define MMCI_STARTBITERROR	BIT(9)
#define MMCI_DATABLOCKEND	BIT(10)
#define MMCI_CMDACTIVE		BIT(11)
#define MMCI_TXACTIVE		BIT(12)
#define MMCI_RXACTIVE		BIT(13)
#define MMCI_TXFIFOBW		BIT(14)
#define MMCI_RXFIFOBR		BIT(15)
#define MMCI_TXFIFOFULL		BIT(16)
#define MMCI_RXFIFOFULL		BIT(17)
#define MMCI_TXFIFOEMPTY	BIT(18)
#define MMCI_RXFIFOEMPTY	BIT(19)
#define MMCI_TXDATAAVLBL	BIT(20)
#define MMCI_RXDATAAVLBL	BIT(21)
#define MMCI_CARDBUSY		BIT(24)

/* MMCI Interrupt Clear register bits */
#define MMCI_CLEAR_MASK		0x1DC007FF

static void mmci_init(void);
static int mmci_send_cmd(struct mmc_cmd *cmd);
static int mmci_set_ios(unsigned int clk, unsigned int width);
static int mmci_prepare(int lba, uintptr_t buf, size_t size);
static int mmci_read(int lba, uintptr_t buf, size_t size);
static int mmci_write(int lba, uintptr_t buf, size_t size);

static const struct mmc_ops mmci_ops = {
	.init		= mmci_init,
	.send_cmd	= mmci_send_cmd,
	.set_ios	= mmci_set_ios,
	.prepare	= mmci_prepare,
	.read		= mmci_read,
	.write		= mmci_write,
};

static mmci_params_t mmci_params;

static void mmci_set_clk(int clk)
{
	int div;
	uint32_t mmci_clkcr;
	uintptr_t base;

	base = mmci_params.reg_base;

	mmci_clkcr = mmio_read_32(base + MMCI_CLOCK);
	mmci_clkcr |= MMCI_CLKEN | MMCI_HWFC_EN;

	assert(clk > 0);

	for (div = 0; div < 255; div++) {
		if ((mmci_params.clk_rate / (div + 2)) <= clk) {
			break;
		}
	}
	assert(div < 256);

	mmci_clkcr &= ~(MMCI_CLKDIVMASK);
	mmci_clkcr |= div;

	mmio_write_32(base + MMCI_CLOCK, mmci_clkcr);

	udelay(CLK_CHANGE_UDELAY);

	debug("%s: div=%d, mmci_clkcr=%x\n", __func__, div, mmci_clkcr);
}

static void mmci_init(void)
{
	uint32_t data;
	uintptr_t base;

	base = mmci_params.reg_base;
	mmio_write_32(base + MMCI_POWER, MMCI_POWERON
			| (MMCI_OPENDRAIN | MMCI_FDBCLK_DIS)); /* For EMMC */
	mmci_set_clk(MMC_BOOT_CLK_RATE);

	/* Disable mmc interrupts */
	data = mmio_read_32(base + MMCI_MASK0) & ~MMCI_MASK0_MASK;
	mmio_write_32(base + MMCI_MASK0, data);
}

static int mmci_send_cmd(struct mmc_cmd *cmd)
{
	uint32_t data_ctrl = 0, err_mask, op;
	uint32_t hoststatus;
	uintptr_t base;
	int ret = 0;

	assert(cmd);

	debug("%s: CMD%d, ARG: %x\n", __func__, cmd->cmd_idx, cmd->cmd_arg);

	base = mmci_params.reg_base;

	op = MMCI_CMD_CPSMEN;

	/* Manage data control register for data transfert */
	switch(cmd->cmd_idx) {
	case MMC_CMD(17):
	case MMC_CMD(18):
		data_ctrl |= MMCI_DCTRL_DTREAD;
		/* FALLTHROUGH */
	case MMC_CMD(24):
	case MMC_CMD(25):
		data_ctrl |= (MMC_BLOCK_SIZE << MMCI_DCTRL_DBLOCKSIZE_SHIFT);
		data_ctrl |= MMCI_DCTRL_DTEN | MMCI_DCTRL_BUSYMODE;
		mmio_write_32(base + MMCI_DATA_CTRL, data_ctrl);
		break;
	default:
		break;
	}

	if (cmd->resp_type) {
		op |= MMCI_CMD_RESPEXPECTED;
		if (cmd->resp_type == MMC_RESPONSE_R2)
			op |= MMCI_CMD_LONGRESP;
	}

	mmio_write_32(base + MMCI_ARGUMENT, cmd->cmd_arg);
	udelay(CMD_REG_UDELAY);
	mmio_write_32(base + MMCI_COMMAND, op | cmd->cmd_idx);

	err_mask = MMCI_CMDTIMEOUT | MMCI_CMDCRCFAIL;
	if ((op & MMCI_CMD_RESPEXPECTED))
		err_mask |= MMCI_CMDRESPEND;
	else
		err_mask |= MMCI_CMDSENT;

	do
		hoststatus = mmio_read_32(base + MMCI_STATUS) & err_mask;
	while (!hoststatus);

	mmio_write_32(base + MMCI_CLEAR, err_mask);
	if (hoststatus & MMCI_CMDTIMEOUT) {
		WARN("%s: CMD%d time out\n", __func__, cmd->cmd_idx);
		return -ETIMEDOUT;
	} else if ((hoststatus & MMCI_CMDCRCFAIL) &&
			((cmd->resp_type != MMC_RESPONSE_R3)
			&& (op & MMCI_CMD_RESPEXPECTED))) {
		ERROR("%s: CMD%d CRC error\n", __func__, cmd->cmd_idx);
		return -EIO;
	}

	if (op & MMCI_CMD_RESPEXPECTED) {
		cmd->resp_data[0] = mmio_read_32(base + MMCI_RESP0);
		if (cmd->cmd_idx != MMC_CMD(17) && cmd->cmd_idx != MMC_CMD(18)) {
			debug("\t< R0: 0x%x", cmd->resp_data[0]);
			if (op & MMCI_CMD_LONGRESP) {
				cmd->resp_data[1] = mmio_read_32(base
							+ MMCI_RESP1);
				cmd->resp_data[2] = mmio_read_32(base
							+ MMCI_RESP2);
				cmd->resp_data[3] = mmio_read_32(base
							+ MMCI_RESP3);
				debug(", R1: 0x%x, R2: 0x%x, R3: 0x%x",
						cmd->resp_data[1],
						cmd->resp_data[2],
						cmd->resp_data[3]);
			}
			debug("\n");
		}
	}

	/* After CMD3 stop open drain => push pull */
	if (cmd->cmd_idx == MMC_CMD(3)) {
		uint32_t pwr = mmio_read_32(base + MMCI_POWER)
			& ~MMCI_OPENDRAIN;
		mmio_write_32(base + MMCI_POWER, pwr);
	}

	return ret;
}

static int mmci_set_ios(unsigned int clk, unsigned int width)
{
	uint32_t bus_width = 0;
	uint32_t mmci_clkcr;

	mmci_clkcr = mmio_read_32(mmci_params.reg_base + MMCI_CLOCK);

	switch (width) {
	case MMC_BUS_WIDTH_1:
		bus_width |= MMCI_CLKCR_WBUS_1;
		break;
	case MMC_BUS_WIDTH_4:
		bus_width |= MMCI_CLKCR_WBUS_4;
		break;
	case MMC_BUS_WIDTH_8:
		bus_width |= MMCI_CLKCR_WBUS_8;
		break;
	default:
		assert(0);
	}
	mmci_clkcr &= ~(MMCI_CLKCR_WBUS_MASK);
	mmci_clkcr |= bus_width;
	mmio_write_32(mmci_params.reg_base + MMCI_CLOCK, mmci_clkcr);

	udelay(CLK_CHANGE_UDELAY);

	mmci_set_clk(clk);

	return 0;
}

static int mmci_prepare(int lba, uintptr_t buf, size_t size)
{
	uintptr_t base;

	assert(((buf & MMC_BLOCK_MASK) == 0) &&
	       ((size % MMC_BLOCK_SIZE) == 0));

	base = mmci_params.reg_base;

	mmio_write_32(base + MMCI_DATA_TIMER, MMCI_DATATIMEOUT);
	mmio_write_32(base + MMCI_DATA_LENGTH, size);
	udelay(DATA_REG_UDELAY);

	return 0;
}

#define MMCI_FIFO_SIZE 32

static int mmci_read(int lba, uintptr_t buf, size_t size)
{
	uintptr_t base;
	size_t rxcount = size;
	uint32_t status, status_err;
	int ret = 0;

	base = mmci_params.reg_base;

	status = mmio_read_32(base + MMCI_STATUS);
	status_err = status & (MMCI_DATACRCFAIL | MMCI_STS_DATATIMEOUT |
			       MMCI_RXOVERRUN);
	while ((!status_err) && (rxcount >= sizeof(uint32_t))) {
		if ((status & MMCI_RXFIFOBR) && rxcount >= MMCI_FIFO_SIZE) {
			buf = mmc_fast_read(buf, base + MMCI_FIFO);
			rxcount -= MMCI_FIFO_SIZE;
		}
		else if (status & MMCI_RXDATAAVLBL) {
			*((uint32_t *)buf) = mmio_read_32(base + MMCI_FIFO);
			buf += sizeof(uint32_t);
			rxcount -= sizeof(uint32_t);
		}
		status = mmio_read_32(base + MMCI_STATUS);
		status_err = status & (MMCI_DATACRCFAIL | MMCI_STS_DATATIMEOUT |
				       MMCI_RXOVERRUN);
	}

	status_err = status &
		(MMCI_DATACRCFAIL | MMCI_STS_DATATIMEOUT | MMCI_DATABLOCKEND |
		 MMCI_RXOVERRUN);
	while (!status_err) {
		status = mmio_read_32(base + MMCI_STATUS);
		status_err = status &
			(MMCI_DATACRCFAIL | MMCI_STS_DATATIMEOUT |
			 MMCI_DATABLOCKEND | MMCI_RXOVERRUN);
	}

	debug("%s: size=%u\n", __func__, size);

	if (status & MMCI_STS_DATATIMEOUT) {
		ERROR("%s: timed out, rxcount: %u, status: 0x%08X\n",
			__func__, rxcount, status);
		ret = -ETIMEDOUT;
	} else if (status & MMCI_DATACRCFAIL) {
		ERROR("%s: CRC error: 0x%x\n", __func__, status);
		ret = -EIO;
	} else if (status & MMCI_RXOVERRUN) {
		ERROR("%s: RX overflow error\n", __func__);
		ret = -EIO;
	} else if (rxcount) {
		ERROR("%s: error, rxcount: %u\n", __func__, rxcount);
		ret = -EIO;
	}

	mmio_write_32(base + MMCI_CLEAR, MMCI_CLEAR_MASK);

	return ret;
}

static int mmci_write(int lba, uintptr_t buf, size_t size)
{
	uintptr_t base;
	size_t txcount = size;
	uint32_t status, status_err;
	int ret = 0;

	base = mmci_params.reg_base;

	status = mmio_read_32(base + MMCI_STATUS);
	status_err = status & (MMCI_DATACRCFAIL | MMCI_STS_DATATIMEOUT);
	while (!status_err && txcount) {
		if (status & MMCI_TXFIFOBW) {
			if (txcount >= MMCI_FIFO_SIZE) {
				buf = mmc_fast_write(buf,
						base + MMCI_FIFO);
				txcount -= MMCI_FIFO_SIZE;
			} else {
				while (txcount >= sizeof(uint32_t)) {
					mmio_write_32(base + MMCI_FIFO,
							*((uint32_t *)buf));
					buf += sizeof(uint32_t);
					txcount -= sizeof(uint32_t);
				}
			}
		}
		status = mmio_read_32(base + MMCI_STATUS);
		status_err = status & (MMCI_DATACRCFAIL | MMCI_STS_DATATIMEOUT);
	}

	status_err = status &
		(MMCI_DATACRCFAIL | MMCI_STS_DATATIMEOUT | MMCI_DATABLOCKEND);
	while (!status_err) {
		status = mmio_read_32(base + MMCI_STATUS);
		status_err = status & (MMCI_DATACRCFAIL |
				MMCI_STS_DATATIMEOUT | MMCI_DATABLOCKEND);
	}

	debug("%s: size=%u\n", __func__, size);

	if (status & MMCI_STS_DATATIMEOUT) {
		ERROR("%s: timed out, txcount:%u, status:0x%08X\n",
		       __func__, txcount, status);
		ret = -ETIMEDOUT;
	} else if (status & MMCI_DATACRCFAIL) {
		ERROR("%s: CRC error\n", __func__);
		ret = -EIO;
	} else if (txcount) {
		ERROR("%s: error, txcount:%u", __func__, txcount);
		ret = -EIO;
	}


	mmio_write_32(base + MMCI_CLEAR, MMCI_CLEAR_MASK);

	return ret;
}

int sta_mmci_init(mmci_params_t *params)
{
	assert((params != 0) &&
	       (params->clk_rate > 0) &&
	       ((params->bus_width == MMC_BUS_WIDTH_1) ||
		(params->bus_width == MMC_BUS_WIDTH_4) ||
		(params->bus_width == MMC_BUS_WIDTH_8)));

	memcpy(&mmci_params, params, sizeof(mmci_params_t));
	return mmc_init(&mmci_ops, 50000000, params->bus_width,
			params->flags, &params->device_info);
}
