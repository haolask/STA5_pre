/**
 * @file sta_sdhc.c
 * @brief This file provides all the SDHC ARM PL180 firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#undef DEBUG /* Define it for debug traces */

#include "trace.h"
#include "utils.h"
#include "sta_map.h"
#include "sta_mtu.h"
#include "sdmmc.h"
#include "sta_sdhc.h"

#define SDHC_CLOCK_RATE_MAX	100000000 /* ~100MHz */

#define SDHC_POWER_3V3		0x07
#define SDHC_POWER_3V0		0x06
#define SDHC_POWER_1V8		0x05

#define SDHC_RESET_ALL		BIT(0)
#define SDHC_RESET_CMD		BIT(1)
#define SDHC_RESET_DATA		BIT(2)

#define SDHC_TRANS_BLK_CNT_EN	BIT(1)
#define SDHC_TRANS_READ		BIT(4)
#define SDHC_TRANS_MULTI	BIT(5)

#define SDHC_CMD_CRC		0x08
#define SDHC_CMD_INDEX		0x10
#define SDHC_CMD_DATA		0x20

#define SDHC_CMD_RESP_NONE	0x00
#define SDHC_CMD_RESP_LONG	0x01
#define SDHC_CMD_RESP_SHORT	0x02
#define SDHC_CMD_RESP_SHORT_BUSY 0x03

#define SDHC_TIMEOUT_VALUE	0xE

/* Driver TYPE */
#define SDHC_SIGNALING_1V8	0x0008

/* Int enable register values */
#define SDHC_INT_CMD_COMPLETE	BIT(0)
#define SDHC_INT_DATA_END	BIT(1)
#define SDHC_INT_DMA_END	BIT(3)
#define SDHC_INT_SPACE_AVAIL	BIT(4)
#define SDHC_INT_DATA_AVAIL	BIT(5)
#define SDHC_INT_CARD_INSERT	BIT(6)
#define SDHC_INT_CARD_REMOVE	BIT(7)
#define SDHC_INT_CARD_INT	BIT(8)
#define SDHC_INT_ERROR		BIT(15)
#define SDHC_INT_TIMEOUT	BIT(16)
#define SDHC_INT_CRC		BIT(17)
#define SDHC_INT_END_BIT	BIT(18)
#define SDHC_INT_INDEX		BIT(19)
#define SDHC_INT_DATA_TIMEOUT	BIT(20)
#define SDHC_INT_DATA_CRC	BIT(21)
#define SDHC_INT_DATA_END_BIT	BIT(22)
#define SDHC_INT_BUS_POWER	BIT(23)
#define SDHC_INT_ACMD12ERR	BIT(24)
#define SDHC_INT_ADMA_ERROR	BIT(25)

#define SDHC_ALL_INT_MASK	0xFFFFFFFF

/* Present state register values */
#define SDHC_CMD_INHIBIT	BIT(0)
#define SDHC_DATA_INHIBIT	BIT(1)
#define SDHC_DOING_WRITE	BIT(8)
#define SDHC_DOING_READ		BIT(9)
#define SDHC_SPACE_AVAILABLE	BIT(10)
#define SDHC_DATA_AVAILABLE	BIT(11)
#define SDHC_CARD_PRESENT	BIT(16)
#define SDHC_CARD_STATE_STABLE	BIT(17)
#define SDHC_CARD_DETECT_PIN_LEVEL	BIT(18)
#define SDHC_WRITE_PROTECT	BIT(19)

/* Defines for Host controller */
#define SDHC_HOST_CNT_BW1	0x00
#define SDHC_HOST_CNT_BW4	0x02
#define SDHC_HOST_CNT_BW8	0x20
#define SDHC_HOST_CNT_HS	0x04
#define SDHC_HOST_CNT_FS	0x00
#define SDHC_HOST_CNT_HS200	0x03
#define SDHC_HOST_CNT2_DDR	0x04
#define SDHC_HOST_CNT2_HS400	0x05

#define SDHC_CMD(c, f)		((((c) & 0xff) << 8) | ((f) & 0xff))

#define BOOT_EN			BIT(5)
#define ALT_BOOT_EN		BIT(6)
#define BOOT_ACK_CHK		BIT(7)

#ifdef DEBUG
#define trace_debug trace_printf
#else
#define trace_debug(format, ...) (void)0
#endif

static void sdhc_reset(struct t_mmc_ctx *mmc, uint8_t val)
{
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;
	uint32_t timeout;

	timeout = 50; /* Wait max 50 ms */
	p_sdhc->softwarereset = val;
	while (p_sdhc->softwarereset & val) {
		if (timeout == 0) {
			TRACE_ERR("%s: Reset %x failed\n", __func__, val);
			return;
		}
		timeout--;
		mdelay(1);
	}
}

/**
 * @brief	This function set the clock frequency of the card
 * @param	clock - the cock rate to set
 * @return	int - status
 */
static int sdhc_set_clock(struct t_mmc_ctx *mmc, uint32_t clock)
{
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;
	uint32_t timeout, div;

	for (div = 1; div < 1024; div++) {
		if ((SDHC_CLOCK_RATE_MAX / (div * 2)) <= clock)
			break;
	}

	trace_debug("%s: set clock %dKHz, div=%d\n", __func__,
		    clock / 1000, div);

	/* Wait max 20 ms */
	timeout = 200;
	while (p_sdhc->presentstate & (SDHC_CMD_INHIBIT | SDHC_DATA_INHIBIT)) {
		if (timeout == 0)
			return MMC_TIMEOUT_ERROR;
		timeout--;
		udelay(100);
	}

	if (clock >= 26000000)
		p_sdhc->hostcontrol1 |= SDHC_HOST_CNT_HS;
	else
		p_sdhc->hostcontrol1 &= ~SDHC_HOST_CNT_HS;

	/* Disable clocks */
	p_sdhc->clockcontrol.reg = 0;

	/* Load value of divider */
	p_sdhc->clockcontrol.bit.sdclkfreqsel = (uint8_t)div;

	/* For SD3.0 host set upper bits */
	p_sdhc->clockcontrol.bit.sdclkfreqsel_upperbits = (div >> 8) & 0x3;

	/* Enable internal clock */
	p_sdhc->clockcontrol.bit.intclkena = 1;

	/* Polling for internal clock stable to be set during 20ms */
	timeout = 20;
	while (!p_sdhc->clockcontrol.bit.intclkstable) {
		if (timeout-- == 0)
			return MMC_TIMEOUT_ERROR;
		mdelay(1);
	}

	/* Enabling SD clock */
	p_sdhc->clockcontrol.bit.sdclkena = 1;

	return 0;
}

/**
 * @brief	This function will perform the mmc controller's initialization
 * @param	io_volt - MMC voltage
 * @return	int - status
 */
static int sdhc_hw_init(struct t_mmc_ctx *mmc, uint32_t io_volt)
{
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;
#ifdef DEBUG
	uint32_t start;

	start = mtu_get_time(0);
#endif

	sdhc_reset(mmc, SDHC_RESET_ALL);

	p_sdhc->timeoutcontrol = SDHC_TIMEOUT_VALUE;
	p_sdhc->hostcontrol1 = 0; /* Low speed 1bit bus */
	p_sdhc->int_sts_ena = SDHC_ALL_INT_MASK; /* Enable all IT */
	p_sdhc->int_sig_ena = 0; /* Mask all IT */

	switch (io_volt) {
	case IO_VOLT_330:
		p_sdhc->powercontrol.bit.sdbusvoltage = SDHC_POWER_3V3;
		p_sdhc->hostcontrol2 = 0;
		break;
	case IO_VOLT_180:
		p_sdhc->powercontrol.bit.sdbusvoltage = SDHC_POWER_1V8;
		p_sdhc->hostcontrol2 = SDHC_SIGNALING_1V8;
		break;
	}

	do {
		p_sdhc->powercontrol.bit.sdbuspower = 1; /* Power ON */
	} while (!p_sdhc->powercontrol.bit.sdbuspower);

	trace_debug("%s: Powerup delay %d us\n", __func__, mtu_get_time(start));

	return sdhc_set_clock(mmc, 400000); /* Set low speed clock 400KHz */
}

/**
 * @brief	This function set the SDMMC bus width
 * @param	bus_width: possible values enum t_mmc_bus_width
 * @return	int - status
 */
static void sdhc_set_bus_width(struct t_mmc_ctx *mmc, uint32_t bus_width)
{
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;
	uint8_t hctrl;

	trace_debug("%s: busWidth=%d\n", __func__,
		    bus_width == 0 ? 1 : (bus_width == 1 ? 4 : 8));

	hctrl = p_sdhc->hostcontrol1;

	switch (bus_width) {
	case MMC_1_BIT_WIDE:
		hctrl &= ~(SDHC_HOST_CNT_BW4 | SDHC_HOST_CNT_BW8);
		break;

	case MMC_4_BIT_WIDE:
		hctrl |= SDHC_HOST_CNT_BW4;
		hctrl &= ~SDHC_HOST_CNT_BW8;
		break;

	case MMC_8_BIT_WIDE:
		hctrl &= ~SDHC_HOST_CNT_BW4;
		hctrl |= SDHC_HOST_CNT_BW8;
		break;
	}

	p_sdhc->hostcontrol1 = hctrl;
}

/**
 * @brief	Send a command
 * @param	uint32_t cmd_id - ID of the command
 * @param	uint32_t arg - argument for the command
 * @param	uint32_t resp_type - waited response type
 * @param	uint32_t size - size of data to exchange or 0
 * @return	int - Status
 */
static int sdhc_send_cmd(struct t_mmc_ctx *mmc, uint32_t cmd_id,
			 uint32_t arg, uint32_t resp_type, uint32_t size)
{
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;

	uint32_t status = 0;
	uint32_t mask, flags;
	uint32_t timeout = 2000;  /* 2sec timeout */

	trace_debug("%s: CMD%d, ARG:%x, ", __func__, cmd_id, arg);

	p_sdhc->int_sts = SDHC_ALL_INT_MASK;

	mask = SDHC_CMD_INHIBIT;
	if (size)
		mask |= SDHC_DATA_INHIBIT;

	while (p_sdhc->presentstate & mask) {
		if (!timeout--) {
			trace_printf("%s: MMC%d busy timeout\n",
				     __func__, mmc->port);
			return MMC_TIMEOUT_ERROR;
		}
		mdelay(1);
	}

	mask = SDHC_INT_CMD_COMPLETE;
	if (!resp_type) {
		flags = SDHC_CMD_RESP_NONE;
	} else {
		if (resp_type & MMC_LONG_RSP)
			flags = SDHC_CMD_RESP_LONG;
		else if (resp_type & MMC_BUSY_RSP)
			flags = SDHC_CMD_RESP_SHORT_BUSY;
		else
			flags = SDHC_CMD_RESP_SHORT;
		if (resp_type & MMC_OPCODE_RSP)
			flags |= SDHC_CMD_INDEX;
		if ((resp_type & MMC_NOCRC) == 0)
			flags |= SDHC_CMD_CRC;
	}

	/* Manage trans mode following CMD ID & size */
	if (size) {
		uint32_t mode;

		flags |= SDHC_CMD_DATA;

		mode = SDHC_TRANS_BLK_CNT_EN;
		if (size > mmc->blksize)
			mode |= SDHC_TRANS_MULTI;

		switch (cmd_id) {
		case SDMMC_CMD0:   /* EMMC READ BOOT ? */
		case SDMMC_CMD8:   /* Read EXT_CSD */
		case SDMMC_CMD17:
		case SDMMC_CMD18:
			mode |= SDHC_TRANS_READ;
			break;
		}

		p_sdhc->blocksize = mmc->blksize;
		p_sdhc->blockcount = size / mmc->blksize;
		p_sdhc->transfermode = mode;
		trace_debug("SZ:%d mode:%x ", size, mode);
	}

	trace_debug("sts_msk:%x cmd:%x\n", mask, SDHC_CMD(cmd_id, flags));

	p_sdhc->argument1 = arg;
	p_sdhc->command = SDHC_CMD(cmd_id, flags);
	timeout = 100000; /* 1sec timout */
	while (((status = p_sdhc->int_sts) & mask) != mask) {
		if (status & SDHC_INT_ERROR) {
			trace_debug("%s[%d]: ERROR, status:%x\n",
				    __func__, __LINE__, status);
			p_sdhc->int_sts = SDHC_ALL_INT_MASK;
			sdhc_reset(mmc, SDHC_RESET_CMD | SDHC_RESET_DATA);

			return MMC_GENERIC_ERROR;
		}

		if (!timeout--) {
			trace_debug("%s[%d]: TIMEOUT, status:%x\n",
				    __func__, __LINE__, status);
			return MMC_TIMEOUT_ERROR;
		}
		udelay(10);
	}
	p_sdhc->int_sts = mask;

	if (resp_type & MMC_LONG_RSP) {
		/* CRC is stripped, need to do some shift */
		mmc->resp[0] = (p_sdhc->response3 << 8) |
				(p_sdhc->response2 >> 24);
		mmc->resp[1] = (p_sdhc->response2 << 8) |
				(p_sdhc->response1 >> 24);
		mmc->resp[2] = (p_sdhc->response1 << 8) |
				(p_sdhc->response0 >> 24);
		mmc->resp[3] = (p_sdhc->response0 << 8);

		trace_debug("R0:%x, R1:%x, R2:%x, R3:%x\n",
			    mmc->resp[0], mmc->resp[1],
			    mmc->resp[2], mmc->resp[3]);
	} else {
		mmc->resp[0] = p_sdhc->response0;
		trace_debug("\t< R0:%x\n", mmc->resp[0]);
	}

	if (size) {
		timeout = 100;  /* 1ms timeout */
		while ((p_sdhc->presentstate &
		        (SDHC_DATA_AVAILABLE | SDHC_SPACE_AVAILABLE)) == 0 ) {
			if (!timeout--) {
				trace_printf("%s: MMC%d data timeout\n",
					     __func__, mmc->port);
				return MMC_TIMEOUT_ERROR;
			}
			udelay(10);
		}
	} else {
		p_sdhc->int_sts = SDHC_ALL_INT_MASK;
	}

	return 0;
}

/**
 * @brief	Read bytes from SDHC FIFO
 * @param	uint8_t *buffer - Buffer from which data will be written
 * @param	uint32_t count - the total size to write.
 * @return	int - Status
 */
static int sdhc_read_bytes(struct t_mmc_ctx *mmc,
			   void *buffer, uint32_t count)
{
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;
	uint32_t *p_tempbuff = (uint32_t *)buffer;
	uint32_t i, status, timeout = 500000;

	while (count && (p_sdhc->presentstate & SDHC_DATA_AVAILABLE)) {
		status = p_sdhc->int_sts;
		if (status & SDHC_INT_ERROR) {
			TRACE_ERR("Status error: %x\n", status);
			return MMC_GENERIC_ERROR;
		}
		if (status & SDHC_INT_DATA_AVAIL) {
			p_sdhc->int_sts = SDHC_INT_DATA_AVAIL;

			for (i = 0; i < mmc->blksize; i += 8) {
				*p_tempbuff++ = p_sdhc->dataport;
				*p_tempbuff++ = p_sdhc->dataport;
			}
			count -= mmc->blksize;
		}
		if (timeout-- > 0)
			udelay(10);
		else
			return MMC_TIMEOUT_ERROR;
	}

	p_sdhc->int_sts = SDHC_ALL_INT_MASK;

	return 0;
}

/**
 * @brief	Write bytes to SDHC FIFO
 * @param	uint8_t *buffer - Buffer from which data will be written
 * @param	uint32_t count - the total size to write.
 * @return	int - Status
 */
static int sdhc_write_bytes(struct t_mmc_ctx *mmc,
			    void *buffer, uint32_t count)
{
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;
	uint32_t *p_tempbuff = (uint32_t *)buffer;
	uint32_t i, status, timeout = 500000;

	while (count && (p_sdhc->presentstate & SDHC_SPACE_AVAILABLE)) {
		status = p_sdhc->int_sts;
		if (status & SDHC_INT_ERROR) {
			TRACE_ERR("Status error: %x\n", status);
			return MMC_GENERIC_ERROR;
		}
		if (status & SDHC_INT_SPACE_AVAIL) {
			p_sdhc->int_sts = SDHC_INT_SPACE_AVAIL;

			for (i = 0; i < mmc->blksize; i += 8) {
				p_sdhc->dataport = *p_tempbuff++;
				p_sdhc->dataport = *p_tempbuff++;
			}
			count -= mmc->blksize;
		}
		if (timeout-- > 0)
			udelay(10);
		else
			return MMC_TIMEOUT_ERROR;
	}

	p_sdhc->int_sts = SDHC_ALL_INT_MASK;

	return 0;
}

static int sdhc_data_cmd(struct t_mmc_ctx *mmc, uint32_t cmd_id,
			 uint32_t sector, void *buffer, uint32_t length)
{
	int error;

	if (mmc->cur_blksize != mmc->blksize) {
		/* Send CMD16 set block size */
		error = sdhc_send_cmd(mmc, SDMMC_CMD16,
				      mmc->blksize, MMC_RESPONSE_R1, 0);
		if (error)
			return error;
		mmc->cur_blksize = mmc->blksize;
	}

	error = sdhc_send_cmd(mmc, cmd_id, sector, MMC_RESPONSE_R1, length);
	if (error != 0)
		return error;

	if (cmd_id == SDMMC_CMD24 || cmd_id == SDMMC_CMD25)
		return sdhc_write_bytes(mmc, buffer, length);
	else
		return sdhc_read_bytes(mmc, buffer, length);
}

static int sdhc_read_extcsd(struct t_mmc_ctx *mmc, uint8_t *excsdbuf)
{
	int error;

	/* Read extCSD  */
	error = sdhc_send_cmd(mmc, SDMMC_CMD8, 0x0, MMC_RESPONSE_R1, 512);
	if (error)
		return error;

	return sdhc_read_bytes(mmc, excsdbuf, 512);
}

static int sdhc_init_boot(struct t_mmc_ctx *mmc, uint32_t size,
			  uint32_t *buffer, uint32_t clock)
{
	int error = 0;
	t_sdhc *p_sdhc = (t_sdhc *)mmc->base;

	/* FIXME to be verified and tested if needed */
	/* # of clocks required to produce 1sec */
	p_sdhc->boottimeoutcnt = 96000000; /* FIXME to adjust */

	p_sdhc->blockgapcontrol = (BOOT_EN | BOOT_ACK_CHK | ALT_BOOT_EN);

	/* Set bus witdh and max freq instead if EXT_CSD byte[177] != 0 */
	sdhc_set_clock(mmc, clock);
	sdhc_set_bus_width(mmc, mmc->bus_width);

	/* FIXME perhaps use a 500ms protection timeout */
	/*
	 * Send CMD0 GO_IDLE_STATE arg 0xFFFFFFFA
	 * (Try Alternative Boot operation,
	 * see page 38 JEDEC Standard No. 84-A441)
	 */
	error = sdhc_send_cmd(mmc, SDMMC_CMD0, 0xFFFFFFFA, 0, size);
	if (error)
		return error;

	return sdhc_read_bytes(mmc, (uint8_t *)buffer, size);
}

static const struct mmc_ops sdhc_ops = {
	.hw_init = sdhc_hw_init,
	.init_boot = sdhc_init_boot,
	.send_cmd = sdhc_send_cmd,
	.data_cmd = sdhc_data_cmd,
	.read_extcsd = sdhc_read_extcsd,
	.change_frequency = sdhc_set_clock,
	.set_bus_width = sdhc_set_bus_width,
};

/**
 * @brief	Initialize Arasan SDHC driver context
 */
void sdhc_init(struct t_mmc_ctx *mmc, uint32_t regs_base)
{
	trace_debug("%s: SDHC base: 0x%08X\n", __func__, regs_base);

	mmc->base = regs_base;
	mmc->ops = &sdhc_ops;
}

