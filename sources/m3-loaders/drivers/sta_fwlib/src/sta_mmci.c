/**
 * @file sta_mmci.c
 * @brief This file provides all the MMCI ARM PL180 firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>

#undef DEBUG /* Define it for debug traces */

#include "trace.h"
#include "utils.h"
#include "sta_map.h"
#include "sta_mtu.h"
#include "sdmmc.h"
#include "sta_mmci.h"

#ifdef DEBUG
#define trace_debug trace_printf
#else
#define trace_debug(format, ...) (void)0
#endif

#define MMCI_CLOCK_RATE_MAX	100000000 /* ~100MHz */

#define CMD_REG_DELAY    300
#define DATA_REG_DELAY   1000
#define CLK_CHANGE_DELAY 2000

#define MMCI_CLOCK_WIDEBUS_MASK		0x00001800

#define MMCI_DATA_TIMEOUT	0xFFFF0000

/* Data Control register */
#define MMCI_READDIR            BIT(1)
#define MMCI_STREAMMODE         BIT(2)
#define MMCI_DMAENABLE          BIT(3)
#define MMCI_BLOCKSIZE          0x000000F0
#define MMCI_SDIOENABLE         BIT(11)
#define MMCI_DBOOTMODEEN	BIT(13)
#define MMCI_BUSYMODE		BIT(14)

/* Power Control register */
#define MMCI_POWERON		0x00000003
#define MMCI_OPENDRAIN		0x00000040

#define MMCI_FDBCLK_DIS		0x00000080
#define MMCI_CMDDATEN		0x0000003C

/* Command control register */
#define MMCI_RESPEXPECTED	0x00000040
#define MMCI_LONGRESPONSE	0x00000080
#define MMCI_ENABLEINTRREQ	0x00000100
#define MMCI_ENABLECMDPEND	0x00000200
#define MMCI_SDIOSUSPEND	0x00000800

/* Clock Control register */
#define MMCI_CLKDIVINIT	0x000000FF
#define MMCI_CLKDIVTRANS	0x00000001
#define MMCI_CLKENABLE		0x00000100
#define MMCI_PWRSAVE		0x00000200
#define MMCI_BYPASS		0x00000400
#define MMCI_HWFC_EN		0x00004000

/* THERE IS ONE MORE INTERRUPT SOURCE START_BIT_ERROR */
#define MMCI_ALLINTERRUPTS	0x007FFFFF
#define MMCI_CLRSTATICFLAGS	0x1DC007FF

/* USED IN POWER MANAGEMENT APIS AND ENHANCED LAYER APIs */
#define MMCI_CMDPATHENABLE	0x00000400
#define MMCI_DATAPATHENABLE	0x00000001

#define MMCI_CMD0TIMEOUT	0x00FFFFFF

#define MMCI_FAST_PROC_FIFO	0x20

/* Status register */
#define MMCI_CMDCRCFAIL         0x00000001
#define MMCI_DATACRCFAIL        0x00000002
#define MMCI_CMDTIMEOUT         0x00000004
#define MMCI_DATATIMEOUT        0x00000008
#define MMCI_TXUNDERRUN         0x00000010
#define MMCI_RXOVERRUN          0x00000020
#define MMCI_CMDRESPEND         0x00000040
#define MMCI_CMDSENT		0x00000080
#define MMCI_DATAEND		0x00000100
#define MMCI_STARTBITERROR      0x00000200
#define MMCI_DATABLOCKEND       0x00000400
#define MMCI_CMDACTIVE          0x00000800
#define MMCI_TXACTIVE           0x00001000
#define MMCI_RXACTIVE		0x00002000
#define MMCI_TXFIFOBW		0x00004000
#define MMCI_RXFIFOBR		0x00008000
#define MMCI_TXFIFOFULL         0x00010000
#define MMCI_RXFIFOFULL         0x00020000
#define MMCI_TXFIFOEMPTY        0x00040000
#define MMCI_RXFIFOEMPTY        0x00080000
#define MMCI_TXDATAAVLBL        0x00100000
#define MMCI_RXDATAAVLBL        0x00200000
#define MMCI_CARDBUSY           0x01000000

/**
 * @brief	This function will perform the host controller's initialization
 * @param	io_volt - MMC voltage
 * @return	int - status
 */
static int mmci_hw_init(struct t_mmc_ctx *mmc, uint32_t io_volt)
{
	t_mmc *p_mmci = (t_mmc *)mmc->base;

	p_mmci->mmc_data_timer = MMCI_DATA_TIMEOUT;
	p_mmci->mmc_power = MMCI_POWERON | MMCI_FDBCLK_DIS;
	p_mmci->mmc_clock = MMCI_CLKDIVINIT | MMCI_CLKENABLE | MMCI_HWFC_EN;
	p_mmci->mmc_mask0 &= ~MMCI_ALLINTERRUPTS;

	/* Set open drain bus mode */
	p_mmci->mmc_power |= MMCI_OPENDRAIN;

	return 0;
}

/**
 * @brief	This function set the clock frequency
 * @param	clock - clock rate to set
 * @return	int - status
 */
static int mmci_set_clock(struct t_mmc_ctx *mmc, uint32_t clock)
{
	t_mmc *p_mmci = (t_mmc *)mmc->base;
	uint32_t div;

	for (div = 0; div < 255; div++) {
		if ((MMCI_CLOCK_RATE_MAX / (div + 2)) <= clock)
			break;
	}

	/* Set bus witdh and max freq instead if EXT_CSD byte[177] != 0 */
	trace_debug("%s: set clock %dKHz, div=%d\n",
		    __func__, clock / 1000, div);

	p_mmci->mmc_clock = div | MMCI_CLKENABLE | MMCI_HWFC_EN;
	if (clock >= 26000000) /* => Enable feedback clock */
		p_mmci->mmc_power = MMCI_POWERON;
	else
		p_mmci->mmc_power = MMCI_POWERON | MMCI_FDBCLK_DIS;

	p_mmci->mmc_mask0 = 0;
	udelay(CLK_CHANGE_DELAY);

	return 0;
}

static void mmci_set_bus_width(struct t_mmc_ctx *mmc, uint32_t bus_width)
{
	t_mmc *p_mmci = (t_mmc *)mmc->base;

	trace_debug("%s: busWidth=%d\n", __func__,
		    bus_width == 0 ? 1 : (bus_width == 1 ? 4 : 8));
	/* Set wide bus operation for the controller */
	p_mmci->mmc_clock &= ~MMCI_CLOCK_WIDEBUS_MASK;
	p_mmci->mmc_clock |= (MMCI_CLOCK_WIDEBUS_MASK & (bus_width << 11));
}

/**
 * @brief	Send a command
 * @param	uint32_t cmd_id - ID of the command
 * @param	uint32_t arg - argument for the command
 * @param	uint32_t resp_type - waited response type
 * @param	uint32_t size - size of data to exchange or 0
 * @return	int - Status
 */
static int mmci_send_cmd(struct t_mmc_ctx *mmc, uint32_t cmd_id,
			 uint32_t arg, uint32_t resp_type, uint32_t size)
{
	t_mmc *p_mmci = (t_mmc *)mmc->base;
	uint32_t data_ctrl = 0, err_mask, op_cmd;
	uint32_t hoststatus;

	trace_debug("%s: CMD%d, ARG: %x\n", __func__, cmd_id, arg);

	op_cmd = MMCI_CMDPATHENABLE;

	if (resp_type) {
		op_cmd |= MMCI_RESPEXPECTED;
		if (resp_type & MMC_LONG_RSP)
			op_cmd |= MMCI_LONGRESPONSE;
	}

	/* Manage data control registers for data transfert */
	if (size) {
		switch (cmd_id) {
		case SDMMC_CMD0:   /* EMMC READ BOOT ? */
			data_ctrl |=  MMCI_DBOOTMODEEN | MMCI_SDIOENABLE;
			/* FALLTHROUGH */
		case SDMMC_CMD8:
		case SDMMC_CMD17:
		case SDMMC_CMD18:
			data_ctrl |= MMCI_READDIR;
			/* FALLTHROUGH */
		case SDMMC_CMD24:
		case SDMMC_CMD25:
			data_ctrl |= (mmc->blksize << 16);
			data_ctrl |= MMCI_DATAPATHENABLE | MMCI_BUSYMODE;
			p_mmci->mmc_data_timer = MMCI_DATA_TIMEOUT;
			p_mmci->mmc_data_length = size;
			udelay(DATA_REG_DELAY);
			p_mmci->mmc_data_ctrl = data_ctrl;
			break;
		default:
			break;
		}
	}

	p_mmci->mmc_argument = arg;
	udelay(CMD_REG_DELAY);
	p_mmci->mmc_command = op_cmd | cmd_id;

	err_mask = MMCI_CMDTIMEOUT | MMCI_CMDCRCFAIL;
	if (op_cmd & MMCI_RESPEXPECTED)
		err_mask |= MMCI_CMDRESPEND;
	else
		err_mask |= MMCI_CMDSENT;

	do
		hoststatus = p_mmci->mmc_status & err_mask;
	while (!hoststatus);

	p_mmci->mmc_clear = err_mask;

	if (hoststatus & MMCI_CMDTIMEOUT) {
		if (!(resp_type & MMC_BUSY_RSP))
			TRACE_ERR("%s: CMD%d time out\n", __func__, cmd_id);
		return MMC_TIMEOUT_ERROR;
	} else if ((hoststatus & MMCI_CMDCRCFAIL) &&
		   (((resp_type & MMC_NOCRC) == 0) &&
		   (op_cmd & MMCI_RESPEXPECTED))) {
		TRACE_ERR("%s: CMD%d CRC error\n", __func__, cmd_id);
		return MMC_CRC_ERROR;
	}

	if (op_cmd & MMCI_RESPEXPECTED) {
		mmc->resp[0] = p_mmci->mmc_response0;
		trace_debug("\t< R0:%x", mmc->resp[0]);
		if (op_cmd & MMCI_LONGRESPONSE) {
			mmc->resp[1] = p_mmci->mmc_response1;
			mmc->resp[2] = p_mmci->mmc_response2;
			mmc->resp[3] = p_mmci->mmc_response3;
			trace_debug(", R1:%x, R2:%x, R3:%x", mmc->resp[1],
				    mmc->resp[2], mmc->resp[3]);
		}
		trace_debug("\n");
	}

	/* After CMD3 stop open drain => push pull */
	if (cmd_id == SDMMC_CMD3)
		p_mmci->mmc_power &= ~MMCI_OPENDRAIN;

	return 0;
}

/**
 * @brief	Read bytes from MMCI FIFO
 * @param	uint8_t *buffer - Buffer from which data will be written
 * @param	uint32_t count - the total size to write.
 * @return	int - Status
 */
static int mmci_read_bytes(struct t_mmc_ctx *mmc,
			   void *buffer, uint32_t count)
{
	t_mmc *p_mmci = (t_mmc *)mmc->base;
	uint32_t *p_tempbuff = (uint32_t *)buffer;
	int error = 0;

	if ((uint32_t)buffer & 3)
		return MMC_UNALIGNED_ERROR;

	trace_debug("%s: size=%u\n", __func__, count);

	while (!(p_mmci->mmc_status & (MMCI_RXOVERRUN | MMCI_DATACRCFAIL
					| MMCI_DATATIMEOUT | MMCI_DATAEND
					| MMCI_STARTBITERROR))) {
		if (p_mmci->mmc_status & MMCI_RXFIFOBR)
			p_tempbuff = mmc_fast_read(p_tempbuff,
						   (void *)&p_mmci->mmc_fifo);
	}

	if (p_mmci->mmc_status & MMCI_DATATIMEOUT) {
		error = MMC_TIMEOUT_ERROR;
	} else if (p_mmci->mmc_status & MMCI_DATACRCFAIL) {
		error = MMC_CRC_ERROR;
	} else if (p_mmci->mmc_status & MMCI_RXOVERRUN) {
		error = MMC_IO_ERROR;
	} else if (p_mmci->mmc_status & MMCI_STARTBITERROR) {
		error = MMC_START_BIT_ERROR;
	} else {
		while (p_mmci->mmc_status & MMCI_RXDATAAVLBL)
			*p_tempbuff++ = p_mmci->mmc_fifo;
	}

	p_mmci->mmc_clear = MMCI_CLRSTATICFLAGS;

	return error;
}

/**
 * @brief	Write bytes to MMCI FIFO
 * @param	uint8_t *buffer - Buffer from which data will be written
 * @param	uint32_t count - the total size to write.
 * @return	int - Status
 */
static int mmci_write_bytes(struct t_mmc_ctx *mmc,
			    void *buffer, uint32_t count)
{
	t_mmc *p_mmci = (t_mmc *)mmc->base;
	uint32_t *p_tempbuff = (uint32_t *)buffer;
	int error = 0;
	char fifo_buffer[MMCI_FAST_PROC_FIFO];

	if ((uint32_t)buffer & 3)
		return MMC_UNALIGNED_ERROR;

	trace_debug("%s: size=%u\n", __func__, count);

	while (!(p_mmci->mmc_status & (MMCI_TXUNDERRUN | MMCI_DATACRCFAIL
				| MMCI_DATATIMEOUT | MMCI_DATAEND
				| MMCI_STARTBITERROR))) {
		if (p_mmci->mmc_status & MMCI_TXFIFOBW) {
			if (count < MMCI_FAST_PROC_FIFO) {
				memcpy(fifo_buffer, (char *)p_tempbuff, count);
				p_tempbuff = (uint32_t *)fifo_buffer;
			}
			p_tempbuff = mmc_fast_write(p_tempbuff,
						(void *)&p_mmci->mmc_fifo);
			count -= MMCI_FAST_PROC_FIFO;
		}
	}

	if (p_mmci->mmc_status & MMCI_DATATIMEOUT)
		error = MMC_TIMEOUT_ERROR;
	else if (p_mmci->mmc_status & MMCI_DATACRCFAIL)
		error = MMC_CRC_ERROR;
	else if (p_mmci->mmc_status & MMCI_TXUNDERRUN)
		error = MMC_IO_ERROR;
	else if (p_mmci->mmc_status & MMCI_STARTBITERROR)
		error = MMC_START_BIT_ERROR;

	p_mmci->mmc_clear = MMCI_CLRSTATICFLAGS;

	return error;
}

static int mmci_data_cmd(struct t_mmc_ctx *mmc, uint32_t cmd_id,
			 uint32_t sector, void *buffer, uint32_t length)
{
	int error;

	if (mmc->cur_blksize != mmc->blksize) {
		/* Send CMD16 set block size */
		error = mmci_send_cmd(mmc, SDMMC_CMD16,
				      mmc->blksize, MMC_RESPONSE_R1, 0);
		if (error)
			return error;
		mmc->cur_blksize = mmc->blksize;
	}

	error = mmci_send_cmd(mmc, cmd_id, sector, MMC_RESPONSE_R1, length);
	if (error)
		return error;

	if (cmd_id == SDMMC_CMD24 || cmd_id == SDMMC_CMD25)
		return mmci_write_bytes(mmc, buffer, length);
	else
		return mmci_read_bytes(mmc, buffer, length);
}

static int mmci_read_extcsd(struct t_mmc_ctx *mmc, uint8_t *excsdbuf)
{
	int error;

	/* Read extCSD  */
	error = mmci_send_cmd(mmc, SDMMC_CMD8, 0x0, MMC_RESPONSE_R1, 512);
	if (error)
		return error;

	return mmci_read_bytes(mmc, excsdbuf, 512);
}

static int mmci_init_boot(struct t_mmc_ctx *mmc, uint32_t size,
			  uint32_t *buffer, uint32_t clock)
{
	int error = 0;
	t_mmc *p_mmci = (t_mmc *)mmc->base;

	/* Set bus push pull mode */
	p_mmci->mmc_power &= ~MMCI_OPENDRAIN;

	/* Set bus witdh and max freq instead if EXT_CSD byte[177] != 0 */
	mmci_set_clock(mmc, clock);
	mmci_set_bus_width(mmc, mmc->bus_width);

	/* FIXME perhaps use a 500ms protection timeout */
	/*
	 * Send CMD0 GO_IDLE_STATE arg 0xFFFFFFFA
	 * (Try Alternative Boot operation,
	 * see page 38 JEDEC Standard No. 84-A441)
	 */
	error = mmci_send_cmd(mmc, SDMMC_CMD0, 0xFFFFFFFA, 0, size);
	if (error)
		return error;

	return mmci_read_bytes(mmc, (uint8_t *)buffer, size);
}

static const struct mmc_ops mmci_ops = {
	.hw_init = mmci_hw_init,
	.init_boot = mmci_init_boot,
	.send_cmd = mmci_send_cmd,
	.data_cmd = mmci_data_cmd,
	.read_extcsd = mmci_read_extcsd,
	.change_frequency = mmci_set_clock,
	.set_bus_width = mmci_set_bus_width,
};

void mmci_init(struct t_mmc_ctx *mmc, uint32_t regs_base)
{
	trace_debug("%s: MMC base: 0x%08X\n", __func__, regs_base);

	mmc->base = regs_base;
	mmc->ops = &mmci_ops;
}

