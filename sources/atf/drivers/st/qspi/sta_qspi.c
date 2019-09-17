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
#include <stdbool.h>
#include <utils.h>
#include <hsem.h>

#include "qspi_sta.h"

#define QSPI_CLK_DIV_BY(d)		(d - 1)

/* Commands */
#define XFER_LEN_SHIFT			24
#define XFER_LEN_MASK			0xFF

#define MEM_ADDR_MASK			0xFFFFFF

#define WR_CNFG_OPCD_SHIFT		24
#define WR_CNFG_OPCD_MASK		0xFF

#define READ_OPCODE_SHIFT		16
#define READ_OPCODE_MASK		0xFF

#define CMD_TYPE_SHIFT			12
#define CMD_TYPE_MASK			0x07

#define SQIO_MODE_SHIFT			8
#define SQIO_MODE_MASK			0x03

#define SQIO_CMD_MASK			0xFF

/* Flash memory manufacturer id */
#define QSPI_MACRONIX_ID		0xC2
#define QSPI_MICRON_ID			0x20
#define QSPI_SST_ID			0xBF
#define QSPI_WINBOND_ID			0xEF
#define QSPI_SPANSION_ID		0x01

/* Flash ID */
#define S25FL128S			0x2018

#define WIP_IS_0			0x1
#define WEL_IS_1			0x2
#define WEL_IS_0			0x4
#define QE_IS_1				0x8
#define SPIN_FOREVER			0x10

/* Macronix specific commands */

/* Commands */
#define QSPI_CMD_WRSR			0x01
#define QSPI_CMD_PP			0x02
#define QSPI_CMD_WRDIS			0x04
#define QSPI_CMD_RDSR			0x05
#define QSPI_CMD_WREN			0x06
#define QSPI_CMD_RDCR			0x15
#define QSPI_CMD_SE			0x20
#define QSPI_CMD_EQIO			0x35
#define QSPI_CMD_RSTQIO			0xF5
#define QSPI_CMD_BE_32K			0x52
#define QSPI_CMD_CE			0x60
#define QSPI_CMD_READ_ID		0x9F
#define QSPI_CMD_SBL			0xC0
#define QSPI_CMD_BE_64K			0xD8
#define QSPI_CMD_FASTREAD		0x0B
#define QSPI_CMD_QFASTREAD		0xEB
#define QSPI_CMD_EN4B			0xB7
#define QSPI_CMD_EX4B			0xE9
/* Used for Spansion flashes only. */
#define QSPI_CMD_BRRD			0x16	/* Bank register read */
#define QSPI_CMD_BRWR			0x17	/* Bank register write */

/* Used by Micron flashses only. */
#define QSPI_CMD_WR_EVCR		0x61
#define QSPI_CMD_READ_EVCR		0x65
#define QSPI_CMD_WR_VCR			0x81
#define QSPI_CMD_READ_VCR		0x85
#define QSPI_CMD_WR_NVCR		0xB1
#define QSPI_CMD_READ_NVCR		0xB5

/* Used by Windond flashes */
#define QSPI_CMD_SET_READ_PARAM		0xC0

/* Command type descriptions */
#define QSPI_CMD_TYPE_SHIFT		12
#define QSPI_CMD_TYPE_C			0 /* Command */
#define QSPI_CMD_TYPE_CAD_WR		1 /* Command -> Address -> Data (write) */
#define QSPI_CMD_TYPE_CD_RD		2 /* Command -> Data (read, less than 4 bytes) */
#define QSPI_CMD_TYPE_CD_RD4		3 /* Command -> Data (read, more than 4 bytes) */
#define QSPI_CMD_TYPE_CAD_RD		4 /* Command -> Address -> Data (red, less than 4 bytes) */
#define QSPI_CMD_TYPE_CD_W		5 /* Command -> Data (write) */
#define QSPI_CMD_TYPE_CA			6 /* Command -> Address */

#define QSPI_MACRONIX_SR_QEN		BIT(6)
#define QSPI_MACRONIX_CR_ODS_DEF	0x07
#define QSPI_MACRONIX_CR_DUM_CYC_4	(0x01 << 6)
#define QSPI_MACRONIX_CR_DUM_CYC_6	(0x00 << 6)
#define QSPI_MACRONIX_CR_DUM_CYC_8	(0x02 << 6)
#define QSPI_MACRONIX_CR_DUM_CYC_10	(0x03 << 6)

#define QSPI_WINSPAN_CR_QEN		BIT(1)
#define QSPI_SPANSION_CR_ODS_DEF	0x00
#define QSPI_SPANSION_CR_DUM_CYC_1	(0x03 << 6)
#define QSPI_SPANSION_CR_DUM_CYC_4	(0x01 << 6)
#define QSPI_SPANSION_CR_DUM_CYC_5	(0x02 << 6)

#define QSPI_MICRON_EVCR_QIO_DIS	BIT(7)
#define QSPI_MICRON_CR_WRAP_DEF		(0x0B)  /* No Wrap */
#define QSPI_MICRON_CR_WRAP_32		(0x09)  /* 32 bytes wrap read */
#define QSPI_MICRON_CR_WRAP_64		(0x0A)  /* 64 bytes wrap read */
#define QSPI_MICRON_CR_DUM_CYC_DEF	(0x0F << 4)
#define QSPI_MICRON_CR_DUM_CYC_4	(0x04 << 4)
#define QSPI_MICRON_CR_DUM_CYC_6	(0x06 << 4)
#define QSPI_MICRON_CR_DUM_CYC_8	(0x08 << 4)
#define QSPI_MICRON_CR_DUM_CYC_10	(0x0A << 4)
#define QSPI_MICRON_CR_DUM_CYC_15	(0x0F << 4)

#define QSPI_FIFO_SIZE 256
typedef volatile struct {
    uint32_t page_buffer[QSPI_FIFO_SIZE / 4];	/* addr offset = 0x000 */
    union {
        struct {
            uint32_t sqio_cmd:8;
            uint32_t sqio_mode:2;
            uint32_t res0:2;
            uint32_t cmd_type:3;
            uint32_t res1:1;
            uint32_t read_opcode:8;
            uint32_t wr_cnfg_opcd:8;
        } bit;
        uint32_t reg;
    } cmd_stat_reg;		/* addr offset = 0x100 */

    union {
        struct {
            uint32_t mem_addr:24;
            uint32_t xfer_len:8;
        } bit;
        uint32_t reg;
    } addr_reg;		/* addr offset = 0x104 */

    uint32_t data_reg;		/* addr offset = 0x108 */

    union {
        struct {
            uint32_t divisor:8;
            uint32_t mode:1;
            uint32_t res0:6;
            uint32_t dummy_cycles:3;
            uint32_t dummy_dis:1;
            uint32_t res1:1;
            uint32_t pol_cnt_pre:3;
            uint32_t res2:1;
            uint32_t sw_reset:1;
            uint32_t res3:1;
            uint32_t int_raw_status_poll:1;
            uint32_t int_en_pol:1;
            uint32_t int_raw_status:1;
            uint32_t int_en:1;
            uint32_t res4:1;
            uint32_t dma_req_en:1;
        } bit;
        uint32_t reg;
    } conf_reg;		/* addr offset = 0x10c */

    union {
        struct {
            uint32_t poll_cmd:8;
            uint32_t poll_start:1;
            uint32_t poll_en:1;
            uint32_t poll_status:1;
            uint32_t res0:1;
            uint32_t poll_bsy_bit_index:3;
            uint32_t res1:1;
            uint32_t polling_count:16;
        } bit;
        uint32_t reg;
    } polling_reg;		/* addr offset = 0x110 */

    uint32_t ext_addr_reg;	/* addr offset = 0x114 */

    union {
        struct {
            uint32_t ext_mem_addr_8msb_msk:8;
            uint32_t dummy_cycle_num:6;
            uint32_t dummy_cycle_alt:1;
            uint32_t ext_mem_mode:1;
            uint32_t sqi_para_mode:1;
            uint32_t res0:7;
            uint32_t ext_spi_clock_en:1;
            uint32_t inv_ext_spi_clk:1;
            uint32_t res1:5;
            uint32_t endian:1;
        } bit;
        uint32_t reg;
    } conf_reg2;		/* addr offset = 0x118 */

} sqi_regs_t;


/**
 * @brief SQI context
 * @regs: Registers base address
 * @flash_id: Flash identifier
 * @manuf_id: Manufacturer identifier
 * @mode_rx: read mode
 * @mode_tx: write mode
 * @mode_rac: Register Access Command mode
 */
struct qspi_ctx_t {
	sqi_regs_t *regs;
	uint32_t mmap_base;
	uint16_t flash_id;
	uint16_t jdec_extid;
	uint8_t manuf_id;
	bool four_bytes_address;
	uint8_t mode_rx;
	uint8_t mode_tx;
	uint8_t mode_rac;
};

static void sta_qspi_drvinit(uintptr_t regs_base, uintptr_t mmap_base);
static void sta_qspi_drvdeinit(void);
static int sta_qspi_read(int lba, uintptr_t buf, size_t size);
static int sta_qspi_write(int lba, uintptr_t buf, size_t size);
static int sta_qspi_erase_block(int lba, size_t size);

static const qspi_sf_ops_t sta_qspi_ops = {
	.init		= sta_qspi_drvinit,
	.deinit		= sta_qspi_drvdeinit,
	.read		= sta_qspi_read,
	.write		= sta_qspi_write,
	.erase		= sta_qspi_erase_block
};

enum qspi_access_type {
	QSPI_WRITE_OR_ERASE,
	QSPI_READ
};

static struct qspi_ctx_t qspi_ctx[1];
static struct qspi_ctx_t *ctx;


#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
static void print_buffer(uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf(" %x", buf[i]);
	printf("\n");
}
#endif

static void qspi_get_hsem(enum qspi_access_type type)
{
	/* Always take M3 sem */
	while (!hsem_trylock(HSEM_SQI_NOR_M3_ID, 0))
		;
	if (type == QSPI_WRITE_OR_ERASE) {
		/* And take AP sem also */
		while (!hsem_trylock(HSEM_SQI_NOR_AP_ID, 0))
			;
	}
}

static void qspi_release_hsem(enum qspi_access_type type)
{
	if (type == QSPI_WRITE_OR_ERASE)
		hsem_unlock(HSEM_SQI_NOR_AP_ID);

	hsem_unlock(HSEM_SQI_NOR_M3_ID);
}

static __attribute__((unused)) const char *get_manu_name(uint8_t manuf_id)
{
	switch(manuf_id) {
	case QSPI_MACRONIX_ID:
		return "Macronix";
	case QSPI_MICRON_ID:
		return "Micron";
	case QSPI_SPANSION_ID:
		return "Spansion";
	case QSPI_WINBOND_ID:
		return "Winbond";
	default:
		return "Unknown";
	}
}

/**
 * @brief SQI command description
 * @mem_addr: SQIO direct memory address (24 bits)
 * @xfer_len: number of bytes to write or read (no more than 4) in case of
 * register commands. For data/page commands, xfer_len represents the number
 * of words to be written/read to/from the flash memory.
 * @wr_cnfg_opcd: write configuration op-code
 * @sqio_mode: SQIO controller mode: QSPI_MODE_SPI, QSPI_MODE_QPI or QSPI_MODE_QSPI
 * @cmd_type: syntax command definition, manufacturer dependent
 * @read_opcode: read opcode.
 * @sqio_cmd: the SQIO command to be sent, manufacturer dependent
 */
static void
qspi_runcmd(struct qspi_ctx_t *ctx, uint8_t mode, uint8_t cmd,
		uint32_t type, uint32_t addr, int len)
{
	sqi_regs_t *regs = ctx->regs;
	uint32_t xlen = 0;
	uint8_t read_opcode;

	INFO("%s: to 0x%x, len:%d, cmd:%x, mode:%d\n", __func__,
			addr, len, cmd, mode);

	/* Manage address and length registers */
	if (len) {
		if ((len > 4) || (cmd == QSPI_CMD_PP))
			xlen = ((len + 3) / 4 - 1) << XFER_LEN_SHIFT;
		else
			xlen = (len - 1) << XFER_LEN_SHIFT;
	}
	regs->addr_reg.reg = xlen | (addr & MEM_ADDR_MASK);
	regs->ext_addr_reg = addr;

	regs->conf_reg.bit.dummy_dis = 1;

	read_opcode = regs->cmd_stat_reg.bit.read_opcode;

	/* Manage command register */
	regs->cmd_stat_reg.reg = (type << QSPI_CMD_TYPE_SHIFT) | cmd |
			(mode << SQIO_MODE_SHIFT) |
			(read_opcode << READ_OPCODE_SHIFT);

	/* Active polling */
	while (regs->cmd_stat_reg.reg & SQIO_CMD_MASK)
		;

	regs->conf_reg.bit.dummy_dis = 0;
}

static void
qspi_read_reg(struct qspi_ctx_t *ctx, uint8_t cmd, uint8_t *buf, int len)
{
	sqi_regs_t *regs = ctx->regs;
	int i;
	int shift = 0;
	uint32_t data = 0;

	qspi_runcmd(ctx, ctx->mode_rac, cmd,
		      (len <= 4) ? QSPI_CMD_TYPE_CD_RD : QSPI_CMD_TYPE_CD_RD4,
		      0, len);

	/* Read out data */
	if (len >= 4) {
		/* Result is in page_buffer register (Big Endian) */
		for (i = 0; i < len; i++) {
			if ((i & 3) == 0) {
				data = regs->page_buffer[i / 4];
				shift = 24;
			}
			*buf++ = (data >> shift);
			shift -= 8;
		}
	} else {
		/* Result is in data register (Little Endian) */
		data = regs->data_reg;
		for (i = 0; i < len; i++) {
			*buf++ = (data >> shift);
			shift += 8;
		}
	}
#if (LOG_LEVEL >= LOG_LEVEL_VERBOSE)
	printf("Rx:");
	print_buffer(buf - len, len);
#endif

}

static void
qspi_write_reg(struct qspi_ctx_t *ctx, uint8_t cmd, uint8_t *buf, int len)
{
	sqi_regs_t *regs = ctx->regs;
	int i, shift;
	uint32_t data = 0;
	uint32_t type = QSPI_CMD_TYPE_C;

	if (len) {
		if (len <= 4) {
			/* Write into PAGE_BUFFER[0] in Big endian */
			for (i = 0, shift = 24; i < len; i++) {
				data |= (*buf++ << shift);
				shift -= 8;
			}
			regs->page_buffer[0] = data;
			type = (cmd << WR_CNFG_OPCD_SHIFT) | QSPI_CMD_TYPE_CD_W;
		} else {
			ERROR("Bad length: %d\n", len);
		}
	}

	qspi_runcmd(ctx, ctx->mode_rac, cmd, type, 0, len);
#if (LOG_LEVEL >= LOG_LEVEL_VERBOSE)
	if (len) {
		printf("Tx:");
		print_buffer(buf - len, len);
	}
#endif
}

/**
 * @brief  Read and return a one byte register value
 * @param  ctx: SQI context
 * @param  cmd: The read command to use
 * @return status register value
 */
static inline uint8_t qspi_read_byte_register(struct qspi_ctx_t *ctx, uint8_t cmd)
{
	uint8_t value;

	qspi_read_reg(ctx, cmd, &value, 1);

	return value;
}

/**
 * @brief  Read and return status register value
 * @param  ctx: SQI context
 * @return status register value
 */
static inline uint8_t qspi_read_status_register(struct qspi_ctx_t *ctx)
{
	return qspi_read_byte_register(ctx, QSPI_CMD_RDSR);
}

/**
 * @brief  Execute a SQI command and wait for completion
 *         polling the status register
 * @param  ctx: SQI context
 * @param  event_mask: event mask to poll on
 * @return none
 */
static void qspi_exec_cmd_wait(struct qspi_ctx_t *ctx, uint8_t event_mask)
{
	uint8_t status;

	if (event_mask & WIP_IS_0) {
		/* Loop until Write In Progress becomes 0 */
		do {
			status = qspi_read_status_register(ctx);
		} while (status & 0x1);

		return;
	}

	if (event_mask & WEL_IS_1) {
		/* Loop until Write Latch Enable becomes 1 */
		do {
			status = qspi_read_status_register(ctx);
		} while (!(status & 0x2));

		return;
	}

	if (event_mask & WEL_IS_0) {
		/* Loop until Write Latch Enable becomes 0 */
		do {
			status = qspi_read_status_register(ctx);
		} while (status & 0x2);

		return;
	}

	if (event_mask & QE_IS_1) {
		/* Loop until QE mode is 1 */
		do {
			status = qspi_read_status_register(ctx);
		} while (!(status & 0x40));
	}
}

/**
 * @brief  Read memory credentials
 * @param  ctx: SQI context
 * @return memory id
 */
static inline void qspi_read_credentials(struct qspi_ctx_t *ctx)
{
	uint8_t read_id[6];

	qspi_read_reg(ctx, QSPI_CMD_READ_ID, &read_id[0], sizeof(read_id));

	/* extract the Manufacturer & Flash ID */
	ctx->manuf_id = read_id[0];
	ctx->flash_id = (read_id[1] << 8) + read_id[2];;
	ctx->jdec_extid = (read_id[3] << 8) + read_id[4];
}

/**
 * @brief  Enable the feedback clock
 * @param  ctx: SQI context
 * @return none
 */
static inline void qspi_enable_feedback_clock(struct qspi_ctx_t *ctx)
{
	sqi_regs_t *regs = ctx->regs;

	regs->conf_reg.bit.sw_reset = 1;
	/* Set the CLK prescaler to minimal value.
	 * Aim is to reach 133 MHz max for Macronix MX25L25635F */
	regs->conf_reg.bit.divisor = QSPI_CLK_DIV_BY(1);

	/* Enable feedback clock */
	regs->conf_reg2.bit.ext_spi_clock_en = 1;
}

 /**
 * @brief  Write enable
 * @param  ctx: SQI context
 * @return none
 */
static inline void qspi_write_enable(struct qspi_ctx_t *ctx)
{
	qspi_write_reg(ctx, QSPI_CMD_WREN, NULL, 0);
	qspi_exec_cmd_wait(ctx, WEL_IS_1);
}

/**
 * @brief  Write status register value and
 *         configuration register value
 * @param  ctx: SQI context
 * @param  status: the status register value to be updated
 * @param  config: the configuration register value to be updated
 * @return none
 */
static void qspi_write_status_register(struct qspi_ctx_t *ctx,
		uint8_t status, uint8_t config)
{
	uint8_t data[2];

	qspi_write_enable(ctx);

	data[0] = status;
	data[1] = config;
	qspi_write_reg(ctx, QSPI_CMD_WRSR, &data[0], sizeof(data));
	qspi_exec_cmd_wait(ctx, WIP_IS_0);
}

/**
 * @brief  Enable/disable Quad Read Modes
 * @param  ctx: SQI context
 * @return none
 */
static void qspi_set_quad_mode(struct qspi_ctx_t *ctx, bool enable)
{
#if QSPI_QUAD_MODE == QSPI_MODE_QPI
	if (ctx->manuf_id == QSPI_MICRON_ID) {
		uint8_t evcr = qspi_read_byte_register(ctx, QSPI_CMD_READ_EVCR);
		if (enable)
			evcr &= ~QSPI_MICRON_EVCR_QIO_DIS;
		else
			evcr |= QSPI_MICRON_EVCR_QIO_DIS;
		qspi_write_enable(ctx);
		qspi_write_reg(ctx, QSPI_CMD_WR_EVCR, &evcr, 1);
		ctx->mode_rac = enable ? QSPI_QUAD_MODE : QSPI_MODE_SPI;
		qspi_exec_cmd_wait(ctx, WIP_IS_0);
	} else {
		qspi_write_reg(ctx, enable ? QSPI_CMD_EQIO : QSPI_CMD_RSTQIO,
				NULL, 0);
	}
	ctx->mode_rac = enable ? QSPI_QUAD_MODE : QSPI_MODE_SPI;
#endif
}

/**
 * @brief  Enable fast read (QPI mode already set)
 * @param  ctx: SQI context
 * @return none
 */
static void qspi_enable_fast_read(struct qspi_ctx_t *ctx)
{
	sqi_regs_t *regs = ctx->regs;
	uint8_t mode = ctx->mode_rx;
	uint8_t dc = 8; /* default: 8 dummy cycles */

	/* Enable Dummy Cycles */
	if (ctx->manuf_id == QSPI_SPANSION_ID) {
		/* 7 dummy cycle(s) in QSPI mode, clk <= 104 MHz */
		dc = 7; /* 5 doesn't work ? */
	} else if (ctx->manuf_id == QSPI_WINBOND_ID) {
		switch (ctx->flash_id) {
		case 04016: /* W25Q32JV */
			/* 6 dummy cycle(s) in QSPI mode, clk <= 104 MHz */
			dc = 6;
			break;
		default:
			break;
		}
		{
			uint8_t data = (((dc / 2) - 1) << 4);

			/* Set Dummy cycles in the flash */
			qspi_write_reg(ctx, QSPI_CMD_SET_READ_PARAM, &data, 1);
		}
	} else {
		if (ctx->manuf_id == QSPI_MICRON_ID) {
			switch (mode) {
			case QSPI_MODE_QSPI:
			case QSPI_MODE_QSPI2:
				mode = QSPI_MODE_SPI; /* Workaround force SPI mode */
				break;
			case QSPI_MODE_QPI:
				dc = 10;
				break;
			default:
				break;
			}
		}
	}
	/* Set Dummy cycles for our IP */
	regs->conf_reg2.bit.dummy_cycle_alt = 1;
	regs->conf_reg2.bit.dummy_cycle_num = dc;

	regs->cmd_stat_reg.reg = ((mode == QSPI_MODE_SPI
				? QSPI_CMD_FASTREAD : QSPI_CMD_QFASTREAD) << READ_OPCODE_SHIFT)
		| (mode << SQIO_MODE_SHIFT);
	while (regs->cmd_stat_reg.reg & SQIO_CMD_MASK)
		;

	regs->conf_reg.bit.dummy_dis = 0;
}

/**
 * @brief  Enable extended 32bits address mode
 * @param  ctx: SQI context
 * @return none
 */
static void qspi_enable_4B(struct qspi_ctx_t *ctx)
{
	sqi_regs_t *regs = ctx->regs;

	if (ctx->manuf_id == QSPI_MICRON_ID)
		qspi_write_reg(ctx, QSPI_CMD_WREN, NULL, 0);

	if (ctx->manuf_id != QSPI_SPANSION_ID) {
		/* Set extended mode: 4 Bytes adressing mode */
		qspi_write_reg(ctx, QSPI_CMD_EN4B, NULL, 0);
	} else { /* Spansion flashes, set bit7 of BAR register */
		if (ctx->flash_id != S25FL128S) {
			uint8_t data = 0x80;

			qspi_write_reg(ctx, QSPI_CMD_BRWR, &data, 1);
		} else {
			return;
		}
	}
	regs->conf_reg2.bit.ext_mem_mode = 1;
	regs->conf_reg2.bit.ext_mem_addr_8msb_msk = 0x0F;

	if (ctx->manuf_id == QSPI_MICRON_ID)
		qspi_write_reg(ctx, QSPI_CMD_WRDIS, NULL, 0);
}

/**
 * @brief  Disable extended 32bits address mode
 * @param  mode: the operating mode (SQI/SPI)
 * @return none
 */
static void qspi_disable_4B(struct qspi_ctx_t *ctx)
{
	sqi_regs_t *regs = ctx->regs;

	if (ctx->manuf_id == QSPI_MICRON_ID)
		qspi_write_reg(ctx, QSPI_CMD_WREN, NULL, 0);

	/* Clear extended mode: 4 Bytes adressing mode */
	regs->conf_reg2.bit.ext_mem_mode = 0;
	regs->conf_reg2.bit.ext_mem_addr_8msb_msk = 0;

	if (ctx->manuf_id != QSPI_SPANSION_ID) {
		qspi_write_reg(ctx, QSPI_CMD_EX4B, NULL, 0);
	} else { /* Spansion flashes, clear bit7 of BAR register */
		if (ctx->flash_id != S25FL128S) {
			uint8_t data = 0x0;

			qspi_write_reg(ctx, QSPI_CMD_BRWR, &data, 1);
		} else {
			return;
		}
	}

	if (ctx->manuf_id == QSPI_MICRON_ID)
		qspi_write_reg(ctx, QSPI_CMD_WRDIS, NULL, 0);
}

/**
 * @brief Setup HW to enable SQI peripheral
 * @param  regs_base: Registers base address
 * @param  mmap_base: Memory map base address
 * @return 0 if error, pointer to SQI context otherwise
 */
static void sta_qspi_drvinit(uintptr_t regs_base, uintptr_t mmap_base)
{
	uint32_t status_reg_val = 0;
	uint32_t config_reg_val = 0;

	ctx = &qspi_ctx[0];
	ctx->regs = (sqi_regs_t *)regs_base;
	ctx->mmap_base = mmap_base;
	ctx->mode_rac = QSPI_MODE_SPI;
	ctx->mode_rx = QSPI_QUAD_MODE;
	ctx->mode_tx = QSPI_MODE_SPI;
	ctx->four_bytes_address = true;  /* Enable 4 Bytes address by default */

	if (QSPI_QUAD_MODE == QSPI_MODE_QPI)
		ctx->mode_tx = QSPI_MODE_QPI;

	/* Pin muxing already done by previous stage */

	/* Already initialized ? */
	if (get_sqi_manu_id() != 0) {
		ctx->manuf_id = get_sqi_manu_id();
		ctx->flash_id = get_sqi_flash_id();
		ctx->jdec_extid = get_sqi_jdec_extid();
		/* Some manufacturer dependent specific settings */
		switch (ctx->manuf_id) {
		case QSPI_WINBOND_ID:
			/* Small size Winbond (<16MB)=> no 4B address */
			ctx->four_bytes_address = false;
			switch (ctx->flash_id) {
			case 0x4016: /* W25Q32JV */
				ctx->mode_rx = QSPI_MODE_QSPI;
				break;
			case 0x4017: /* W25Q64FV (SPI) */
				ctx->mode_rx = QSPI_MODE_SPI;
				ctx->mode_tx = QSPI_MODE_SPI;
				break;
			case 0x6017: /* W25Q64FV (QPI) */
				break;
			default:
				ctx->four_bytes_address = true;
				break;
			}
			break;

		case QSPI_SPANSION_ID:
			ctx->mode_rx = QSPI_MODE_QSPI;
			break;

		case QSPI_MACRONIX_ID:
		case QSPI_MICRON_ID:
			ctx->mode_rac = QSPI_QUAD_MODE;
			break;
		}
	} else {
		/* first init */
		qspi_get_hsem(QSPI_WRITE_OR_ERASE);

		qspi_enable_feedback_clock(ctx);

		/* Read the ID and update device and Manufacturer ID
		 * globals variables */
		qspi_read_credentials(ctx);

		switch (ctx->manuf_id) {
		case QSPI_MACRONIX_ID:
			/* Quad Enable bit enable */
			status_reg_val = QSPI_MACRONIX_SR_QEN;
			/* 8 dummy cycles plus signals strength */
			config_reg_val = QSPI_MACRONIX_CR_ODS_DEF |
				QSPI_MACRONIX_CR_DUM_CYC_8;
			/* Write the status register to increase
			 * number of dummy cycles */
			qspi_write_status_register(ctx, status_reg_val,
					config_reg_val);
			break;

		case QSPI_MICRON_ID:
			break;

		case QSPI_WINBOND_ID:
			/* Small size Winbond (<16MB)=> no 4B address */
			ctx->four_bytes_address = false;
			switch (ctx->flash_id) {
			case 0x4016: /* W25Q32JV */
				ctx->mode_rx = QSPI_MODE_QSPI;
				break;
			case 0x4017: /* W25Q64FV (SPI) */
				ctx->mode_rx = QSPI_MODE_SPI;
				ctx->mode_tx = QSPI_MODE_SPI;
				break;
			case 0x6017: /* W25Q64FV (QPI) */
				break;
			default:
				ctx->four_bytes_address = true;
				break;
			}

			/* Enable Quad */
			qspi_write_status_register(ctx, 0, QSPI_WINSPAN_CR_QEN);
			ctx->mode_rx = QSPI_MODE_QSPI;
			break;

		case QSPI_SPANSION_ID:
			status_reg_val = 0;
			/* Quad Enable bit and 5 dummy cycle(s) */
			config_reg_val = QSPI_WINSPAN_CR_QEN | QSPI_SPANSION_CR_ODS_DEF
				| QSPI_SPANSION_CR_DUM_CYC_5;
			/* Write the status register to increase number
			 * of dummy cycles */
			qspi_write_status_register(ctx, status_reg_val,
					config_reg_val);
			ctx->mode_rx = QSPI_MODE_QSPI;
			break;

		default:
			ERROR("%s: Unknown serial NOR manufacturer 0x%x\n",
					__func__, ctx->manuf_id);
			return;
		}

		if (ctx->four_bytes_address)
			qspi_enable_4B(ctx);

		/* Enable Quad mode */
		if (ctx->manuf_id == QSPI_MACRONIX_ID
				|| ctx->manuf_id == QSPI_MICRON_ID)
			qspi_set_quad_mode(ctx, true);

		/* Enable the fast read mode */
		qspi_enable_fast_read(ctx);

		qspi_release_hsem(QSPI_WRITE_OR_ERASE);

		INFO("%s: VCR: %x, EVCR: %x\n", __func__,
			qspi_read_byte_register(ctx, QSPI_CMD_READ_VCR),
			qspi_read_byte_register(ctx, QSPI_CMD_READ_EVCR));
	}

	NOTICE("%s: %x %s SF, mode: %d\n", __func__,
		ctx->flash_id, get_manu_name(ctx->manuf_id), ctx->mode_rx);
}

/**
 * @brief deinit/reset SQI to normal SPI mode
 * @param  ctx: SQI context
 * @return None
 */
static void sta_qspi_drvdeinit(void)
{
	qspi_get_hsem(QSPI_WRITE_OR_ERASE);
#if QSPI_QUAD_MODE == QSPI_MODE_QPI
	if (ctx->manuf_id == QSPI_MACRONIX_ID) {
		/* Disable Quad mode */
		qspi_set_quad_mode(ctx, false);

		/* Reset write status register to default */
		qspi_write_status_register(ctx, 0, QSPI_MACRONIX_CR_ODS_DEF
					   | QSPI_MACRONIX_CR_DUM_CYC_8);
	} else if (ctx->manuf_id == QSPI_MICRON_ID) {
		/* Disable Quad mode */
		qspi_set_quad_mode(ctx, false);
	} else if (ctx->manuf_id == QSPI_SPANSION_ID) {
		/* Reset write config register to default => Stop Quad read */
		qspi_write_status_register(ctx, 0, QSPI_SPANSION_CR_ODS_DEF
					   | QSPI_SPANSION_CR_DUM_CYC_5);
	} else if (ctx->manuf_id == QSPI_WINBOND_ID) {
		/* Reset write config register to default => Stop Quad read */
		qspi_write_status_register(ctx, 0, 0);
	}
#endif /* QSPI_QUAD_MODE == QSPI_MODE_QPI */
	if (ctx->four_bytes_address)
		qspi_disable_4B(ctx);
	qspi_release_hsem(QSPI_WRITE_OR_ERASE);
}

/**
 * @brief  Erase a memory block
 * @param  addr: address belonging to erasable block
 * @param  size: size to erase multiple of 4KBytes
 * @return 0 if no error, not 0 otherwise
 */
static int sta_qspi_erase_block(int addr, size_t size)
{
	uint8_t cmd;

	qspi_get_hsem(QSPI_WRITE_OR_ERASE);

	/* Write Enable */
	qspi_write_enable(ctx);

	switch(size) {
	case 4*1024:
		cmd = QSPI_CMD_SE;
		break;
	case 32*1024:
		cmd = QSPI_CMD_BE_32K;
		break;
	case 64*1024:
		cmd = QSPI_CMD_BE_64K;
		break;
	default:
		return -EINVAL;
	}
	qspi_runcmd(ctx, ctx->mode_rac, cmd, QSPI_CMD_TYPE_C, addr, 0);

	qspi_exec_cmd_wait(ctx, WIP_IS_0);
	qspi_exec_cmd_wait(ctx, WEL_IS_0);

	qspi_release_hsem(QSPI_WRITE_OR_ERASE);

	return 0;
}

/**
 * @brief  Read data
 * @param  addr: address to read
 * @param  buf: output read buffer pointer
 * @param  length: buffer length
 * @return length if no error, < 0 otherwise
 */
static int sta_qspi_read(int addr, uintptr_t buf, size_t length)
{
	VERBOSE("Read: @%x, %d bytes\n", addr, length);
	qspi_get_hsem(QSPI_READ);
	memcpy((void *)buf, (void *)(ctx->mmap_base + addr), length);
	qspi_release_hsem(QSPI_READ);

	return length;
}

/**
 * @brief  Write data
 * @param  ctx: SQI context
 * @param  addr: address to write
 * @param  buf: input write buffer pointer
 * @param  length: buffer length
 * @return 0 if no error, not 0 otherwise
 */
static int sta_qspi_write(int addr, uintptr_t buf, size_t length)
{
	uint32_t loop_size, i;

	assert((buf & 3) == 0);

	VERBOSE("Write: @%x, %d bytes\n", addr, length);

	qspi_get_hsem(QSPI_WRITE_OR_ERASE);
	while (length) {
		if (length < QSPI_FIFO_SIZE)
			loop_size = length;
		else
			loop_size = QSPI_FIFO_SIZE;

		/* Copy into Page Buffer */
		for (i = 0; i < (loop_size / sizeof(uint32_t)); i++) {
			ctx->regs->page_buffer[i] = *(uint32_t *)buf;
			buf += sizeof(uint32_t);
		}

		/* Write Enable */
		qspi_write_enable(ctx);

		qspi_runcmd(ctx, ctx->mode_tx, QSPI_CMD_PP, QSPI_CMD_TYPE_CAD_WR,
					addr, loop_size);

		qspi_exec_cmd_wait(ctx, WIP_IS_0);
		qspi_exec_cmd_wait(ctx, WEL_IS_0);

		addr += loop_size;
		length -= loop_size;
	}

	if (ctx->manuf_id == QSPI_MICRON_ID)
		qspi_write_reg(ctx, QSPI_CMD_WRDIS, NULL, 0);

	qspi_release_hsem(QSPI_WRITE_OR_ERASE);

	return 0;
}

void sta_qspi_init(uintptr_t regs_base, uintptr_t mmap_base)
{
	assert(regs_base == QSPI0_BASE);

	qspi_init(&sta_qspi_ops, regs_base, mmap_base);
}

void sta_qspi_deinit(void)
{
	assert((uintptr_t)ctx->regs == QSPI0_BASE);

	qspi_deinit(&sta_qspi_ops);
}
