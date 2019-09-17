/**
 * @file sta_sqi.c
 * @brief This file provides all the SQI firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>
#include <errno.h>

//#define DEBUG 0 /* 1 for full data dump */
#define SQI_M3_AP_SHARED /* Uncomment for M3/AP sharing access */

#undef SQI_TESTS /* Define it for tests */

#include "utils.h"
#include "trace.h"

#include "sta_mtu.h"
#include "sta_sqi.h"
#include "sta_pinmux.h"
#include "sta_hsem.h"
#include "rpmx_common.h"

#define SQI_CLK_DIV_BY(d)		(d - 1)

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
#define SQI_MACRONIX_ID			0xC2
#define SQI_MICRON_ID			0x20
#define SQI_SST_ID			0xBF
#define SQI_WINBOND_ID			0xEF
#define SQI_SPANSION_ID			0x01

/* Flash ID */
#define S25FL128S			0x2018
#define W25Q32JV			0x4016
#define W25Q64FV_SPI			0x4017
#define W25Q64FV			0x6017

#define WIP_IS_0			0x1
#define WEL_IS_1			0x2
#define WEL_IS_0			0x4
#define QE_IS_1				0x8
#define SPIN_FOREVER			0x10

/* Macronix specific commands */

/* Commands */
#define SQI_CMD_WRSR			0x01
#define SQI_CMD_PP			0x02
#define SQI_CMD_WRDIS			0x04
#define SQI_CMD_RDSR			0x05
#define SQI_CMD_WREN			0x06
#define SQI_CMD_RDCR			0x15
#define SQI_CMD_SE			0x20
#define SQI_CMD_EQIO			0x35
#define SQI_CMD_RSTQIO			0xF5
#define SQI_CMD_BE_32K			0x52
#define SQI_CMD_CE			0x60
#define SQI_CMD_READ_ID			0x9F
#define SQI_CMD_SBL			0xC0
#define SQI_CMD_BE_64K			0xD8
#define SQI_CMD_FASTREAD		0x0B
#define SQI_CMD_QFASTREAD		0xEB
#define SQI_CMD_EN4B			0xB7
#define SQI_CMD_EX4B			0xE9
/* Used for Spansion flashes only. */
#define SQI_CMD_BRRD			0x16	/* Bank register read */
#define SQI_CMD_BRWR			0x17	/* Bank register write */

/* Used by Micron flashses only. */
#define SQI_CMD_WR_EVCR			0x61
#define SQI_CMD_READ_EVCR		0x65
#define SQI_CMD_WR_VCR			0x81
#define SQI_CMD_READ_VCR		0x85
#define SQI_CMD_WR_NVCR			0xB1
#define SQI_CMD_READ_NVCR		0xB5

/* Used by Windond flashes */
#define SQI_CMD_SET_READ_PARAM		0xC0

/* Command type descriptions */
#define SQI_CMD_TYPE_SHIFT		12
#define SQI_CMD_TYPE_C			0 /* Command */
#define SQI_CMD_TYPE_CAD_WR		1 /* Command -> Address -> Data (write) */
#define SQI_CMD_TYPE_CD_RD		2 /* Command -> Data (read, less than 4 bytes) */
#define SQI_CMD_TYPE_CD_RD4		3 /* Command -> Data (read, more than 4 bytes) */
#define SQI_CMD_TYPE_CAD_RD		4 /* Command -> Address -> Data (red, less than 4 bytes) */
#define SQI_CMD_TYPE_CD_W		5 /* Command -> Data (write) */
#define SQI_CMD_TYPE_CA			6 /* Command -> Address */

#define SQI_MACRONIX_SR_QEN		BIT(6)
#define SQI_MACRONIX_CR_ODS_DEF		0x07
#define SQI_MACRONIX_CR_DUM_CYC_4	(0x01 << 6)
#define SQI_MACRONIX_CR_DUM_CYC_6	(0x00 << 6)
#define SQI_MACRONIX_CR_DUM_CYC_8	(0x02 << 6)
#define SQI_MACRONIX_CR_DUM_CYC_10	(0x03 << 6)

#define SQI_WINSPAN_CR_QEN		BIT(1)
#define SQI_SPANSION_CR_ODS_DEF		0x00
#define SQI_SPANSION_CR_DUM_CYC_1	(0x03 << 6)
#define SQI_SPANSION_CR_DUM_CYC_4	(0x01 << 6)
#define SQI_SPANSION_CR_DUM_CYC_5	(0x02 << 6)

#define SQI_MICRON_EVCR_QIO_DIS		BIT(7)
#define SQI_MICRON_CR_WRAP_DEF		(0x0B)  /* No Wrap */
#define SQI_MICRON_CR_WRAP_32		(0x09)  /* 32 bytes wrap read */
#define SQI_MICRON_CR_WRAP_64		(0x0A)  /* 64 bytes wrap read */
#define SQI_MICRON_CR_DUM_CYC_DEF	(0x0F << 4)
#define SQI_MICRON_CR_DUM_CYC_4		(0x04 << 4)
#define SQI_MICRON_CR_DUM_CYC_6		(0x06 << 4)
#define SQI_MICRON_CR_DUM_CYC_8		(0x08 << 4)
#define SQI_MICRON_CR_DUM_CYC_10	(0x0A << 4)
#define SQI_MICRON_CR_DUM_CYC_15	(0x0F << 4)

#define SQI_RPMC_EXT_STATUS_POWER_ON				0x0
#define SQI_RPMC_EXT_STATUS_OK					0x80
#define SQI_RPMC_EXT_STATUS_SUCCESS_ERR_MASK                    0x8
#define SQI_RPMC_EXT_STATUS_BUSY_ERR_MASK                       0x1
#define SQI_RPMC_EXT_STATUS_PAYLOAD_OK_ERR_MASK                 0x2
#define SQI_RPMC_EXT_STATUS_SIGN_MISMATCH_ERR_MASK              0x4
#define SQI_RPMC_EXT_STATUS_HMAC_KEY_UNINIT_PAYLOAD_OK_ERR_MASK 0x8
#define SQI_RPMC_EXT_STATUS_CNT_MISMATCH_PAYLOAD_OK_ERR_MASK    0x10
#define SQI_RPMC_EST_STATUS_FATAL_ERROR                         0x20

#if defined DEBUG
#undef TRACE_INFO
#define TRACE_INFO	trace_printf
#endif

/**
 * @brief   SQI RPMC serial flash specific parameters
 */
struct sqi_rpmc_cmd_id_params_t
{
	uint8_t op_code_1;
	uint8_t op_code_2;
	uint8_t counter_nr;
	uint8_t counter_size;
	uint8_t busy_polling;
};

static struct t_sqi_ctx sqi_ctx[SQI_INSTANCES_MAX];

static struct sqi_rpmc_cmd_id_params_t s_devf_desc[1] = {
	/* for now only Macronix RPMC serial flash are supported */
	{
		.op_code_1 = 0x9B,
		.op_code_2 = 0x96,
		.counter_nr = 4,
		.counter_size = 32,
		.busy_polling = 1, /* Poll for OP1 busy using Status */
	},
};

#if defined DEBUG && DEBUG >= 1
void print_buffer(uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		TRACE_NOTICE(" %02x", buf[i]);
	TRACE_NOTICE("\n");
}
#endif

enum sqi_access_type {
	SQI_WRITE_OR_ERASE,
	SQI_READ
};

#if defined SQI_M3_AP_SHARED

static struct hsem_lock *hsem_m3;
static struct hsem_lock *hsem_ap;

static void sqi_get_hw_sem(enum sqi_access_type type)
{
	/* Always take M3 sem */
	hsem_lock(hsem_m3);
	if (type == SQI_WRITE_OR_ERASE) {
		/* And take AP sem also */
		hsem_lock(hsem_ap);
	}
}

static void sqi_release_hw_sem(enum sqi_access_type type)
{
	if (type == SQI_WRITE_OR_ERASE)
		hsem_unlock(hsem_ap);

	hsem_unlock(hsem_m3);
}
#else
static void sqi_get_hw_sem(enum sqi_access_type type) {}
static void sqi_release_hw_sem(enum sqi_access_type type) {}
#endif /* SQI_M3_AP_SHARED */

static __maybe_unused const char *get_manu_name(uint8_t manuf_id)
{
	switch(manuf_id) {
	case SQI_MACRONIX_ID:
		return "Macronix";
	case SQI_MICRON_ID:
		return "Micron";
	case SQI_SPANSION_ID:
		return "Spansion";
	case SQI_WINBOND_ID:
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
 * @sqio_mode: SQIO controller mode: SQI_MODE_SPI, SQI_MODE_QPI or SQI_MODE_QSPI
 * @cmd_type: syntax command definition, manufacturer dependent
 * @read_opcode: read opcode.
 * @sqio_cmd: the SQIO command to be sent, manufacturer dependent
 */
static void
sqi_runcmd(struct t_sqi_ctx *ctx, uint8_t mode, uint8_t cmd,
		uint32_t type, uint32_t addr, int len)
{
	t_sqi *regs = ctx->regs;
	uint32_t xlen = 0;
	uint8_t read_opcode;

	TRACE_INFO("%s: to 0x%08x, len:%d, cmd:%02x, mode:%d\n", __func__,
		   addr, len, cmd, mode);

	/* Manage address and length registers */
	if (len) {
		if ((len > 4) || (cmd == SQI_CMD_PP))
			xlen = ((len + 3) / 4 - 1) << XFER_LEN_SHIFT;
		else
			xlen = (len - 1) << XFER_LEN_SHIFT;
	}
	regs->addr_reg.reg = xlen | (addr & MEM_ADDR_MASK);
	regs->ext_addr_reg = addr;

	regs->conf_reg.bit.dummy_dis = 1;

	read_opcode = regs->cmd_stat_reg.bit.read_opcode;

	/* Manage command register */
	regs->cmd_stat_reg.reg = (type << SQI_CMD_TYPE_SHIFT) | cmd |
			(mode << SQIO_MODE_SHIFT) | (read_opcode << READ_OPCODE_SHIFT);

	/* Active polling */
	while (regs->cmd_stat_reg.reg & SQIO_CMD_MASK)
		;

	regs->conf_reg.bit.dummy_dis = 0;
}

static void
sqi_read_reg(struct t_sqi_ctx *ctx, uint8_t cmd, uint8_t *buf, int len)
{
	t_sqi *regs = ctx->regs;
	int i;
	int shift = 0;
	uint32_t data = 0;

	sqi_runcmd(ctx, ctx->mode_rac, cmd,
		   (len <= 4) ? SQI_CMD_TYPE_CD_RD : SQI_CMD_TYPE_CD_RD4,
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
#if defined DEBUG && (DEBUG >= 1)
	TRACE_NOTICE("Rx:");
	print_buffer(buf - len, len);
#endif

}

static void
sqi_write_reg(struct t_sqi_ctx *ctx, uint8_t cmd, uint8_t *buf, int len)
{
	t_sqi *regs = ctx->regs;
	int i, shift;
	uint32_t data = 0;
	uint32_t type = SQI_CMD_TYPE_C;

	if (len) {
		if (len <= 4) {
			/* Write into PAGE_BUFFER[0] in Big endian */
			for (i = 0, shift = 24; i < len; i++) {
				data |= (*buf++ << shift);
				shift -= 8;
			}
			regs->page_buffer[0] = data;
			type = (cmd << WR_CNFG_OPCD_SHIFT) | SQI_CMD_TYPE_CD_W;
		} else {
			TRACE_ERR("Bad length: %d\n", len);
		}
	}

	sqi_runcmd(ctx, ctx->mode_rac, cmd, type, 0, len);
#if defined DEBUG && (DEBUG >= 1)
	if (len) {
		TRACE_NOTICE("Tx:");
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
static inline uint8_t sqi_read_byte_register(struct t_sqi_ctx *ctx, uint8_t cmd)
{
	uint8_t value;

	sqi_read_reg(ctx, cmd, &value, 1);

	return value;
}

/**
 * @brief  Read and return status register value
 * @param  ctx: SQI context
 * @return status register value
 */
static inline uint8_t sqi_read_status_register(struct t_sqi_ctx *ctx)
{
	return sqi_read_byte_register(ctx, SQI_CMD_RDSR);
}

/**
 * @brief  Execute a SQI command and wait for completion
 *         polling the status register
 * @param  ctx: SQI context
 * @param  event_mask: event mask to poll on
 * @return none
 */
static void sqi_exec_cmd_wait(struct t_sqi_ctx *ctx, uint8_t event_mask)
{
	uint8_t status;

	if (event_mask & WIP_IS_0) {
		/* Loop until Write In Progress becomes 0 */
		do {
			status = sqi_read_status_register(ctx);
		} while (status & 0x1);

		return;
	}

	if (event_mask & WEL_IS_1) {
		/* Loop until Write Latch Enable becomes 1 */
		do {
			status = sqi_read_status_register(ctx);
		} while (!(status & 0x2));

		return;
	}

	if (event_mask & WEL_IS_0) {
		/* Loop until Write Latch Enable becomes 0 */
		do {
			status = sqi_read_status_register(ctx);
		} while (status & 0x2);

		return;
	}

	if (event_mask & QE_IS_1) {
		/* Loop until QE mode is 1 */
		do {
			status = sqi_read_status_register(ctx);
		} while (!(status & 0x40));
	}
}

/**
 * @brief  Read memory credentials
 * @param  ctx: SQI context
 * @return memory id
 */
static inline void sqi_read_credentials(struct t_sqi_ctx *ctx)
{
	uint8_t read_id[6];

	sqi_read_reg(ctx, SQI_CMD_READ_ID, &read_id[0], sizeof(read_id));

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
static inline void sqi_enable_feedback_clock(struct t_sqi_ctx *ctx)
{
	t_sqi *regs = ctx->regs;

	regs->conf_reg.bit.sw_reset = 1;
	/* Set the CLK prescaler to minimal value.
	 * Aim is to reach 133 MHz max for Macronix MX25L25635F */
	regs->conf_reg.bit.divisor = SQI_CLK_DIV_BY(1);

	/* Enable feedback clock */
	regs->conf_reg2.bit.ext_spi_clock_en = 1;
}

 /**
 * @brief  Write enable
 * @param  ctx: SQI context
 * @return none
 */
static inline void sqi_write_enable(struct t_sqi_ctx *ctx)
{
	sqi_write_reg(ctx, SQI_CMD_WREN, NULL, 0);
	sqi_exec_cmd_wait(ctx, WEL_IS_1);
}

/**
 * @brief  Write status register value and
 *         configuration register value
 * @param  ctx: SQI context
 * @param  status: the status register value to be updated
 * @param  config: the configuration register value to be updated
 * @return none
 */
static void sqi_write_status_register(struct t_sqi_ctx *ctx,
		uint8_t status, uint8_t config)
{
	uint8_t data[2];

	sqi_write_enable(ctx);

	data[0] = status;
	data[1] = config;
	sqi_write_reg(ctx, SQI_CMD_WRSR, &data[0], sizeof(data));
	sqi_exec_cmd_wait(ctx, WIP_IS_0);
}

/**
 * @brief  Write register value
 * @param  ctx: SQI context
 * @param  cmd: The write command to use
 * @param  data: the register value to be updated
 * @return none
 */
static __maybe_unused void sqi_write_byte_register(struct t_sqi_ctx *ctx,
						   uint8_t cmd, uint8_t data)
{
	sqi_write_enable(ctx);
	sqi_write_reg(ctx, cmd, &data, 1);
	sqi_exec_cmd_wait(ctx, WIP_IS_0);
}

/**
 * @brief  Enable/disable Quad Read Modes
 * @param  ctx: SQI context
 * @return none
 */
void sqi_set_quad_mode(struct t_sqi_ctx *ctx, bool enable)
{
#if SQI_QUAD_MODE == SQI_MODE_QPI
	if (ctx->manuf_id == SQI_MICRON_ID) {
		uint8_t evcr = sqi_read_byte_register(ctx, SQI_CMD_READ_EVCR);
		if (enable)
			evcr &= ~SQI_MICRON_EVCR_QIO_DIS;
		else
			evcr |= SQI_MICRON_EVCR_QIO_DIS;
		sqi_write_enable(ctx);
		sqi_write_reg(ctx, SQI_CMD_WR_EVCR, &evcr, 1);
		ctx->mode_rac = enable ? SQI_QUAD_MODE : SQI_MODE_SPI;
		sqi_exec_cmd_wait(ctx, WIP_IS_0);
	} else {
		sqi_write_reg(ctx, enable ? SQI_CMD_EQIO : SQI_CMD_RSTQIO,
			      NULL, 0);
	}
	ctx->mode_rac = enable ? SQI_QUAD_MODE : SQI_MODE_SPI;
#endif
}

/**
 * @brief  Enable fast read (QPI mode already set)
 * @param  ctx: SQI context
 * @return none
 */
static void sqi_enable_fast_read(struct t_sqi_ctx *ctx)
{
	t_sqi *regs = ctx->regs;
	uint8_t mode = ctx->mode_rx;
	uint8_t dc = 8; /* dummy cycles: 8 dummy cycles in QPI mode, clk <= 104 MHz */

	/* Enable Dummy Cycles */
	switch (ctx->manuf_id) {
	case SQI_SPANSION_ID:
		/* dummy cycles: 7 dummy cycle(s) in QSPI mode, clk<=104 MHz */
		dc = 7; /* 5 doesn't work ? */
		break;

	case SQI_WINBOND_ID:
		switch (ctx->flash_id) {
		case W25Q32JV:
			dc = 6; /* 6 dummy cycle(s) in QSPI mode */
			break;

		default:
			break;
		}
		{
			/* Set Dummy Cycles */
			uint8_t data = (((dc / 2) - 1) << 4);
			sqi_write_reg(ctx, SQI_CMD_SET_READ_PARAM, &data, 1);
		}
		break;

	case SQI_MICRON_ID:
		switch (mode) {
		case SQI_MODE_QSPI:
		case SQI_MODE_QSPI2:
			mode = SQI_MODE_SPI; /* Workaround force SPI mode */
			break;
		case SQI_MODE_QPI:
			dc = 10;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	regs->conf_reg2.bit.dummy_cycle_alt = 1;
	regs->conf_reg2.bit.dummy_cycle_num = dc;
	regs->cmd_stat_reg.reg = ((mode == SQI_MODE_SPI
			? SQI_CMD_FASTREAD : SQI_CMD_QFASTREAD) << READ_OPCODE_SHIFT)
			| (mode << SQIO_MODE_SHIFT);
	while (regs->cmd_stat_reg.reg & SQIO_CMD_MASK)
		;

	regs->conf_reg.bit.dummy_dis = 0;
}

/**
 * @brief Setup HW to enable SQI peripheral
 * @param  instance: instance numero 0 or 1
 * @return 0 if error, pointer to SQI context otherwise
 */
struct t_sqi_ctx *sqi_init(uint8_t instance)
{
	__maybe_unused const char *sf_type;
	struct t_sqi_ctx *ctx;

	if (instance >= SQI_INSTANCES_MAX)
		return NULL;

	ctx = &sqi_ctx[instance];
	if (ctx->regs) /* Already Initialized => just return ctx */
		return ctx;

	ctx->regs = instance ? sqi1_regs : sqi0_regs;
	ctx->mmap_base = instance ? SQI1_NOR_BB_BASE : SQI0_NOR_BB_BASE;
	ctx->mode_rac = SQI_MODE_SPI;
	ctx->mode_rx = SQI_QUAD_MODE;
	ctx->mode_tx = SQI_MODE_SPI;
	ctx->four_bytes_address = true;  /* Enable 4 Bytes address by default */
	ctx->rpmx_family = RPMX_DEV_FAMILY_UNKNOWN;

	if (SQI_QUAD_MODE == SQI_MODE_QPI)
		ctx->mode_tx = SQI_MODE_QPI;

	/* Already initialized by other boot stage ? */
	if (get_boot_dev_base() == (uint32_t)ctx->mmap_base) {
		ctx->manuf_id = get_sqi_manu_id();
		ctx->flash_id = get_sqi_flash_id();
		ctx->jdec_extid = get_sqi_jdec_extid();

		/* Some manufacturer dependent specific settings */
		switch (ctx->manuf_id) {
		case SQI_WINBOND_ID:
			switch (ctx->flash_id) {
			case W25Q32JV:
				ctx->four_bytes_address = false;
				ctx->mode_rx = SQI_MODE_QSPI;
				break;

			case W25Q64FV:
				ctx->four_bytes_address = false;
				break;

			case W25Q64FV_SPI:
				ctx->four_bytes_address = false;
				ctx->mode_rx = SQI_MODE_SPI;
				ctx->mode_tx = SQI_MODE_SPI;
				break;

			default:
				ctx->mode_rac = SQI_QUAD_MODE;
				break;
			}
			break;

		case SQI_SPANSION_ID:
			ctx->mode_rx = SQI_MODE_QSPI;
			break;

		case SQI_MACRONIX_ID:
			ctx->rpmx_family = RPMX_DEV_FAMILY_1;
			/* intentional fallthrough */
		case SQI_MICRON_ID:
			ctx->mode_rac = SQI_QUAD_MODE;
			break;
		}
	} else {
		/* First init */
		pinmux_request("sqi0_mux");

		sqi_enable_feedback_clock(ctx);

		/* Read ID and update device and Manufacturer ID globals variables */
		sqi_read_credentials(ctx);

		switch (ctx->manuf_id) {
		case SQI_MACRONIX_ID:
			/*
			 * Quad Enable bit enable +
			 * 8 dummy cycles + signals strength
			 */
			sqi_write_status_register(ctx, SQI_MACRONIX_SR_QEN,
						  SQI_MACRONIX_CR_ODS_DEF |
						  SQI_MACRONIX_CR_DUM_CYC_8);
			break;

		case SQI_MICRON_ID:
			break;

		case SQI_SPANSION_ID:
			/* Quad Enable bit and 5 dummy cycle(s) */
			sqi_write_status_register(ctx, 0, SQI_WINSPAN_CR_QEN |
						  SQI_SPANSION_CR_ODS_DEF |
						  SQI_SPANSION_CR_DUM_CYC_5);
			ctx->mode_rx = SQI_MODE_QSPI;
			break;

		case SQI_WINBOND_ID:
			switch (ctx->flash_id) {
			case W25Q32JV:
				ctx->four_bytes_address = false;
				ctx->mode_rx = SQI_MODE_QSPI;
				break;

			case W25Q64FV:
				ctx->four_bytes_address = false;
				break;

			case W25Q64FV_SPI:
				ctx->four_bytes_address = false;
				ctx->mode_rx = SQI_MODE_SPI;
				ctx->mode_tx = SQI_MODE_SPI;
				break;

			default:
				break;
			}
			/* Quad Enable bit */
			sqi_write_status_register(ctx, 0, SQI_WINSPAN_CR_QEN);
			break;

		default:
			TRACE_ERR("%s: Unknown serial NOR manufacturer 0x%x\n",
				  __func__, ctx->manuf_id);
			return NULL;
		}

		if (ctx->four_bytes_address)
			sqi_enable_4B(ctx);

		/* Enable Quad mode */
		if (ctx->manuf_id == SQI_MACRONIX_ID || ctx->manuf_id == SQI_MICRON_ID)
			sqi_set_quad_mode(ctx, true);

		TRACE_INFO("%s: VCR: %X, EVCR: %X\n", __func__,
			   sqi_read_byte_register(ctx, SQI_CMD_READ_VCR),
			   sqi_read_byte_register(ctx, SQI_CMD_READ_EVCR));

		/* Enable the fast read mode */
		sqi_enable_fast_read(ctx);

		/* Save minimal context in shared data */
		if (!get_boot_dev_base()) {
			set_boot_dev_base(ctx->mmap_base);
			set_sqi_manu_id(ctx->manuf_id);
			set_sqi_flash_id(ctx->flash_id);
			set_sqi_jdec_extid(ctx->jdec_extid);
		}
	}

#if defined SQI_M3_AP_SHARED
	{
		struct hsem_lock_req lock;

		lock.lock_cb = NULL;

		lock.id = HSEM_SQI_NOR_M3_ID;
		hsem_m3 = hsem_lock_request(&lock);
		if (!hsem_m3)
			return NULL;

		lock.id = HSEM_SQI_NOR_AP_ID;
		hsem_ap = hsem_lock_request(&lock);
		if (!hsem_ap)
			return NULL;
	}
#endif

	TRACE_NOTICE("%s: %04X %s SF, mode: %d\n", __func__, ctx->flash_id,
		     get_manu_name(ctx->manuf_id), ctx->mode_rx);

	return ctx;
}

/**
 * @brief deinit/reset SQI to normal SPI mode
 * @param  ctx: SQI context
 * @return None
 */
void sqi_deinit(struct t_sqi_ctx *ctx)
{
#if SQI_QUAD_MODE == SQI_MODE_QPI
	switch (ctx->manuf_id) {
	case SQI_MACRONIX_ID:
		/* Disable Quad mode */
		sqi_set_quad_mode(ctx, false);

		/* Reset write status register to default */
		sqi_write_status_register(ctx, 0, SQI_MACRONIX_CR_ODS_DEF |
					  SQI_MACRONIX_CR_DUM_CYC_8);
		break;

	case SQI_MICRON_ID:
		/* Disable Quad mode */
		sqi_set_quad_mode(ctx, false);
		break;

	case SQI_SPANSION_ID:
		/* Reset write config register to default => Stop Quad read */
		sqi_write_status_register(ctx, 0, SQI_SPANSION_CR_ODS_DEF |
					  SQI_SPANSION_CR_DUM_CYC_5);
		break;

	case SQI_WINBOND_ID:
		/* Reset write config register to default => Stop Quad read */
		sqi_write_status_register(ctx, 0, 0);
		break;
	}
#endif /* SQI_QUAD_MODE == SQI_MODE_QPI */

	if (ctx->four_bytes_address)
		sqi_disable_4B(ctx);

	/* Reset SQI shared info */
	set_sqi_manu_id(0);
	set_sqi_flash_id(0);

#if defined SQI_M3_AP_SHARED
	hsem_lock_free(hsem_m3);
	hsem_lock_free(hsem_ap);
#endif
	/* Release context */
	ctx->regs = NULL;
}

/**
 * @brief  Enable extended 32bits address mode
 * @param  ctx: SQI context
 * @return none
 */
void sqi_enable_4B(struct t_sqi_ctx *ctx)
{
	t_sqi *regs = ctx->regs;

	if (ctx->manuf_id == SQI_MICRON_ID)
		sqi_write_reg(ctx, SQI_CMD_WREN, NULL, 0);

	if (ctx->manuf_id != SQI_SPANSION_ID) {
		/* Set extended mode: 4 Bytes adressing mode */
		sqi_write_reg(ctx, SQI_CMD_EN4B, NULL, 0);
	} else { /* Spansion flashes, set bit7 of BAR register */
		if (ctx->flash_id != S25FL128S) {
			uint8_t data = 0x80;

			sqi_write_reg(ctx, SQI_CMD_BRWR, &data, 1);
		} else {
			return;
		}
	}
	regs->conf_reg2.bit.ext_mem_mode = 1;
	regs->conf_reg2.bit.ext_mem_addr_8msb_msk = 0x0F;

	if (ctx->manuf_id == SQI_MICRON_ID)
		sqi_write_reg(ctx, SQI_CMD_WRDIS, NULL, 0);
}

/**
 * @brief  Disable extended 32bits address mode
 * @param  mode: the operating mode (SQI/SPI)
 * @return none
 */
void sqi_disable_4B(struct t_sqi_ctx *ctx)
{
	t_sqi *regs = ctx->regs;

	if (ctx->manuf_id == SQI_MICRON_ID)
		sqi_write_reg(ctx, SQI_CMD_WREN, NULL, 0);

	/* Clear extended mode: 4 Bytes adressing mode */
	regs->conf_reg2.bit.ext_mem_mode = 0;
	regs->conf_reg2.bit.ext_mem_addr_8msb_msk = 0;

	if (ctx->manuf_id != SQI_SPANSION_ID) {
		sqi_write_reg(ctx, SQI_CMD_EX4B, NULL, 0);
	} else { /* Spansion flashes, clear bit7 of BAR register */
		if (ctx->flash_id != S25FL128S) {
			uint8_t data = 0x0;

			sqi_write_reg(ctx, SQI_CMD_BRWR, &data, 1);
		} else {
			return;
		}
	}

	if (ctx->manuf_id == SQI_MICRON_ID)
		sqi_write_reg(ctx, SQI_CMD_WRDIS, NULL, 0);
}

/**
 * @brief  Erase a memory block
 * @param  ctx: SQI context
 * @param  addr: address belonging to erasable block
 * @param  granularity: erase granularity @see sqi_erase_size
 * @return 0 if no error, not 0 otherwise
 */
int sqi_erase_block(struct t_sqi_ctx *ctx, uint32_t addr, int granularity)
{
	uint8_t cmd;

	sqi_get_hw_sem(SQI_WRITE_OR_ERASE);

	/* Write Enable */
	sqi_write_enable(ctx);

	switch(granularity) {
	case SQI_4KB_GRANULARITY:
		cmd = SQI_CMD_SE;
		break;
	case SQI_32KB_GRANULARITY:
		cmd = SQI_CMD_BE_32K;
		break;
	case SQI_64KB_GRANULARITY:
		cmd = SQI_CMD_BE_64K;
		break;
	case SQI_CHIP_ERASE:
		cmd = SQI_CMD_CE;
		break;
	default:
		return -EINVAL;
	}
	sqi_runcmd(ctx, ctx->mode_rac, cmd, SQI_CMD_TYPE_C, addr, 0);

	sqi_exec_cmd_wait(ctx, WIP_IS_0);
	sqi_exec_cmd_wait(ctx, WEL_IS_0);

	sqi_release_hw_sem(SQI_WRITE_OR_ERASE);

	return 0;
}

/**
 * @brief  Read data
 * @param  ctx: SQI context
 * @param  addr: address belonging to eresable block
 * @param  buffer: buffer pointer
 * @param  length: buffer length
 * @return length if no error, < 0 otherwise
 */
int sqi_read(struct t_sqi_ctx *ctx, uint32_t addr,
		 void *buffer, uint32_t length)
{
	sqi_get_hw_sem(SQI_READ);

	memcpy(buffer, (char *)(ctx->mmap_base + addr), length);

	sqi_release_hw_sem(SQI_READ);

	return length;
}

/**
 * @brief  Write data
 * @param  ctx: SQI context
 * @param  addr: address belonging to eresable block
 * @param  buffer: buffer pointer
 * @param  length: buffer length
 * @return 0 if no error, not 0 otherwise
 */
int sqi_write(struct t_sqi_ctx *ctx, uint32_t addr,
		 uint32_t *buffer, uint32_t length)
{
	t_sqi *regs = ctx->regs;
	uint32_t loop_size, i;

	sqi_get_hw_sem(SQI_WRITE_OR_ERASE);

	while (length) {
		if (length < SQI_FIFO_SIZE)
			loop_size = length;
		else
			loop_size = SQI_FIFO_SIZE;

		/* Copy into Page Buffer */
		for (i = 0; i < (loop_size / sizeof(uint32_t)); i++)
			regs->page_buffer[i] = *buffer++;

		/* Write Enable */
		sqi_write_enable(ctx);

		sqi_runcmd(ctx, ctx->mode_tx, SQI_CMD_PP, SQI_CMD_TYPE_CAD_WR,
					addr, loop_size);

		sqi_exec_cmd_wait(ctx, WIP_IS_0);
		sqi_exec_cmd_wait(ctx, WEL_IS_0);

		addr += loop_size;
		length -= loop_size;
	}

	if (ctx->manuf_id == SQI_MICRON_ID)
		sqi_write_reg(ctx, SQI_CMD_WRDIS, NULL, 0);

	sqi_release_hw_sem(SQI_WRITE_OR_ERASE);

	return 0;
}

/* RPMC Monotonic counter functions */

/**
 * @brief  This routine queries the extended status register
 * @param  status: pointer to status variable
 * @return None
 */
void sqi_rpmc_read_extended_status_register(struct t_sqi_ctx *ctx,
					    uint8_t *status)
{
	sqi_get_hw_sem(SQI_WRITE_OR_ERASE);

	/* Send OP2 command */
	sqi_runcmd(ctx, SQI_MODE_SPI,
		   s_devf_desc[ctx->rpmx_family].op_code_2,
		   SQI_CMD_TYPE_CD_RD, 0, 2);

	/* First byte is a dummy byte, status is the 2nd byte */
	*status = (ctx->regs->data_reg >> 8);

	sqi_release_hw_sem(SQI_WRITE_OR_ERASE);
}

/**
 * @brief  This routine sends OP1 (write command)
 * @param  ctx: SQI context
 * @param  payload: pointer to OP1 payload buffer
 * @param  size: payload size
 * @return SQI RPMC error condition.
 */
static enum sqi_rpmc_err sqi_rpmc_send_op1(struct t_sqi_ctx *ctx,
					   uint8_t *payload,
					   uint32_t size)
{
	unsigned int i;
	int word_cnt = 0;
	uint8_t saved_mode;
	uint8_t ext_status = 0x0;

	sqi_get_hw_sem(SQI_WRITE_OR_ERASE);

	sqi_runcmd(ctx, SQI_MODE_SPI,
		   s_devf_desc[ctx->rpmx_family].op_code_1,
		   SQI_CMD_TYPE_CAD_WR, 0, size - 1);

	/* Fill the page buffer with correct endianness */
	for (i = 1; i < size; i += sizeof(uint32_t)) {
		ctx->regs->page_buffer[word_cnt++] =
			((payload[i] << 24) | (payload[i + 1] << 16) |
			(payload[i + 2] << 8) | payload[i + 3]);
	}

	if (s_devf_desc[ctx->rpmx_family].busy_polling == 0) {
		do {
			sqi_rpmc_read_extended_status_register(ctx, &ext_status);
		} while (ext_status & SQI_RPMC_EXT_STATUS_BUSY_ERR_MASK);
	}

	saved_mode = ctx->mode_rac;
	ctx->mode_rac = SQI_MODE_SPI;
	sqi_exec_cmd_wait(ctx, WIP_IS_0);
	ctx->mode_rac = saved_mode;

	sqi_release_hw_sem(SQI_WRITE_OR_ERASE);

	return SQI_RPMC_OK;
}

/**
 * @brief  This routine writes the root key register
 * @param  ctx: SQI context
 * @param  palyload: pointer to payload buffer (including opcode, cmd type,
 *                   counter address, rsvd, key, signature)
 * @param  size : payload size
 * @return SQI RPMC error condition.
 */
static enum sqi_rpmc_err
sqi_rpmc_write_root_key_register(struct t_sqi_ctx *ctx,
				 struct sqi_rpmc_wrkr_t *payload,
				 uint32_t size)
{
	const char *err_str;
	enum sqi_rpmc_err err = SQI_RPMC_KO;
	uint8_t ext_status = 0;

	/* Check the counter address is not out of range */
	if (payload->cnt_addr > (s_devf_desc[ctx->rpmx_family].counter_nr - 1)) {
		err_str = "Bad OP1 counter #";
		goto error;
	}

	/* Verify root key has not already been initialized */
	sqi_rpmc_read_extended_status_register(ctx, &ext_status);

	/* Verify we are in the right condition to write the key */
	if (ext_status != SQI_RPMC_EXT_STATUS_POWER_ON) {
		err_str = "Can't overwrite";
		goto error;
	}

	/* Write the root key */
	if (SQI_RPMC_OK != sqi_rpmc_send_op1(ctx, (uint8_t *)payload, size)) {
		err_str = "Can't write";
		goto error;
	}

	/* Verify the extended status register */
	sqi_rpmc_read_extended_status_register(ctx, &ext_status);

	/* Verify the command went fine */
	if (ext_status == SQI_RPMC_EXT_STATUS_OK) {
		return SQI_RPMC_OK;
	} else if (ext_status & SQI_RPMC_EXT_STATUS_BUSY_ERR_MASK) {
		err_str ="Card busy";
	} else if (ext_status & SQI_RPMC_EXT_STATUS_PAYLOAD_OK_ERR_MASK) {
		err_str ="Sign issue or key override";
		err = SQI_RPMC_ROOT_KEY_AVAILABLE;
	} else if (ext_status & SQI_RPMC_EXT_STATUS_SIGN_MISMATCH_ERR_MASK) {
		err_str = "Bad payload sz";
	} else {
		err_str = "Unknown failure";
	}

error :
	TRACE_ERR("%s: %s\n", __func__, err_str);
	return err;
}

/**
 * @brief  This routine writes the root key to the remote RPMC storage
 * @param  ctx: SQI context
 * @param  payload: Write Root key payload
 * @return 0 if OK else error status
 */
enum sqi_rpmc_err sqi_rpmc_init(struct t_sqi_ctx *ctx,
				struct sqi_rpmc_wrkr_t *payload)
{
	if (ctx->rpmx_family >= NELEMS(s_devf_desc))
		return SQI_RPMC_KO;

	/* Write the root key */
	return sqi_rpmc_write_root_key_register(ctx, payload,
						sizeof(struct sqi_rpmc_wrkr_t));
}

/**
 * @brief  Get the extended status register
 * @param  ctx: SQI context
 * @param  payload: pointer to output payload buffer (status, tag, current read data, signature)
 * @return None
 */
void sqi_rpmc_read(struct t_sqi_ctx *ctx,
		   struct sqi_rpmc_read_reply_t *payload)
{
	unsigned int i, shift;
	uint32_t data = 0;
	uint8_t *pd = (uint8_t *)payload;

	sqi_get_hw_sem(SQI_WRITE_OR_ERASE);

	/* Send OP2 command */
	sqi_runcmd(ctx, SQI_MODE_SPI,
		   s_devf_desc[ctx->rpmx_family].op_code_2,
		   SQI_CMD_TYPE_CD_RD4, 0,
		   sizeof(*payload) + 1);

	/* Swap contents and withdraw first dummy byte */
	for (i = 0, shift = 0; i < (sizeof(*payload) + 1); i++) {
		if ((i & 0x03) == 0) {
			data = ctx->regs->page_buffer[i / 4];
			shift = 24;
		}
		if (i != 0) /*  Skip first dummy byte */
			*pd++ = (data >> shift);
		shift -= 8;
	}

	sqi_release_hw_sem(SQI_WRITE_OR_ERASE);
}

/**
 * @brief  This routine discovers the monotonic counter remote device family.
 * @param  ctx: SQI context
 * @param  descriptor: pointer to the descriptor of the family.
 * @return 0 if OK else error status
 */
enum sqi_rpmc_err sqi_rpmc_discover(struct t_sqi_ctx *ctx,
				    uint8_t *descriptor)
{
	if (ctx->rpmx_family >= NELEMS(s_devf_desc)) {
		return SQI_RPMC_KO;
	} else {
		*descriptor = ctx->rpmx_family;
		return SQI_RPMC_OK;
	}
}

/**
 * @brief  Update the HMAC key register
 * @param  ctx: SQI context
 * @param  payload: pointer to payload buffer (including opcode, cmd type, cnt address, rsvd, key data, signature)
 * @return SQI RPMC error condition.
 */
enum sqi_rpmc_err
sqi_rpmc_update_hmac_key_register(struct t_sqi_ctx *ctx,
				  struct sqi_rpmc_uhkr_t *payload)
{
	const char *err_str;
	uint8_t ext_status = 0;

	/* Check the counter address is not out of range */
	if (payload->cnt_addr > (s_devf_desc[ctx->rpmx_family].counter_nr - 1)) {
		err_str = "OP1 counter address out of bounds";
		goto error;
	}

	/* Update HMAC */
	if (SQI_RPMC_OK != sqi_rpmc_send_op1(ctx, (uint8_t *)payload,
					     sizeof(*payload))) {
		err_str = "Can't update HMAC register";
		goto error;
	}

	/* Verify the extended status register */
	sqi_rpmc_read_extended_status_register(ctx, &ext_status);

	if (ext_status == SQI_RPMC_EXT_STATUS_OK)
		return SQI_RPMC_OK;
	else if (ext_status & SQI_RPMC_EXT_STATUS_BUSY_ERR_MASK)
		err_str = "Card is busy";
	else if (ext_status & SQI_RPMC_EXT_STATUS_PAYLOAD_OK_ERR_MASK)
		err_str = "Counter not initialized";
	else if (ext_status & SQI_RPMC_EXT_STATUS_SIGN_MISMATCH_ERR_MASK)
		err_str = "Signature mismatch, cnt address out of range";
	else
		err_str = "Unknown failure";

error :
	TRACE_ERR("%s: %s\n", __func__, err_str);
	return SQI_RPMC_KO;
}

/**
 * @brief  Increment a monotonic counter
 * @param  ctx: SQI context
 * @param  payload: pointer to payload buffer (including opcode, cmd type, cnt address, rsvd, counter data, signature)
 * @return SQI RPMC error condition.
 */
enum sqi_rpmc_err
sqi_rpmc_increment_monotonic_counter(struct t_sqi_ctx *ctx,
				     struct sqi_rpmc_imc_t *payload)
{
	const char *err_str;
	uint8_t  ext_status = 0;

	/* Check the counter address is not out of range */
	if (payload->cnt_addr > (s_devf_desc[ctx->rpmx_family].counter_nr - 1)) {
		err_str = "OP1 counter address out of bounds";
		goto error;
	}

	/* Increment Monotonic Counter */
	if (SQI_RPMC_OK != sqi_rpmc_send_op1(ctx, (uint8_t *)payload,
					     sizeof(*payload))) {
		err_str = "Can't update HMAC register";
		goto error;
	}

	/* Verify the extended status register */
	sqi_rpmc_read_extended_status_register(ctx, &ext_status);

	if (ext_status != SQI_RPMC_EXT_STATUS_OK)
		return SQI_RPMC_OK;
	else if (ext_status & SQI_RPMC_EXT_STATUS_BUSY_ERR_MASK)
		err_str = "Card busy";
	else if (ext_status & SQI_RPMC_EXT_STATUS_PAYLOAD_OK_ERR_MASK)
		err_str = "Sign mismatch, cnt out of range, payload wrong sz";
	else if (ext_status & SQI_RPMC_EXT_STATUS_SIGN_MISMATCH_ERR_MASK)
		err_str = "Counter not initialized";
	else if (ext_status & SQI_RPMC_EXT_STATUS_CNT_MISMATCH_PAYLOAD_OK_ERR_MASK)
		err_str = "Counter value not matching";
	else
		err_str = "Unknown failure";

error :
	TRACE_ERR("%s: %s\n", __func__, err_str);
	return SQI_RPMC_KO;
}

/**
 * @brief  Request a monotonic counter
 * @param  ctx: SQI context
 * @param  payload: pointer to payload buffer (including opcode, cmd type, cnt address, rsvd, tag, signature)
 * @param  size: payload size
 * @return SQI RPMC error condition.
 */
enum sqi_rpmc_err
sqi_rpmc_request_monotonic_counter(struct t_sqi_ctx *ctx,
				   struct sqi_rpmc_rmc_t *payload)
{
	const char *err_str;
	uint8_t  ext_status = 0;

	/* Check the counter address is not out of range */
	if (payload->cnt_addr > (s_devf_desc[ctx->rpmx_family].counter_nr - 1)) {
		err_str = "OP1 counter address out of bounds";
		goto error;
	}

	/* Request Monotonic Counter */
	if (SQI_RPMC_OK != sqi_rpmc_send_op1(ctx, (uint8_t *)payload,
					     sizeof(*payload))) {
		err_str = "Can't update HMAC register";
		goto error;
	}

	/* Verify the extended status register */
	sqi_rpmc_read_extended_status_register(ctx, &ext_status);

	if (ext_status == SQI_RPMC_EXT_STATUS_OK)
		return SQI_RPMC_OK;
	else if (ext_status & SQI_RPMC_EXT_STATUS_BUSY_ERR_MASK)
		err_str = "card is busy";
	else if (ext_status & SQI_RPMC_EXT_STATUS_SIGN_MISMATCH_ERR_MASK)
		err_str = "sign mismatch, cnt out of range, cmd out of range";
	else if (ext_status & SQI_RPMC_EXT_STATUS_CNT_MISMATCH_PAYLOAD_OK_ERR_MASK)
		err_str = "HMAC key not initialized";
	else
		err_str = "unknown failure";

error :
	TRACE_ERR("%s: %s\n", __func__, err_str);
	return SQI_RPMC_KO;
}

#if defined SQI_TESTS

void sqi_tests(struct t_sqi_ctx *ctx)
{
	int ret;
	uint32_t i;
	static uint32_t sqi_buf[4096/4];
	unsigned int addr = 0x00300000;
	uint32_t start;
	uint32_t speed;

	if (ctx->four_bytes_address)
		addr += 0x01000000;  /* test above 16MB */
	TRACE_NOTICE("SQI tests:\n");
	TRACE_NOTICE("Erasing... ");
	start = mtu_get_time(0);
	ret = sqi_erase_block(ctx, addr, SQI_64KB_GRANULARITY);
	speed = ((0x10000 >> 10) * 1000000) / mtu_get_time(start);
	if (ret)
		TRACE_NOTICE("ERROR\n");
	else
		TRACE_NOTICE("OK at %dKB/s\n", speed);
	TRACE_NOTICE("Writing @%08X... ", addr);
	for (i = 0; i < sizeof(sqi_buf); i++)
		*((char *)sqi_buf + i) = (char)i;
	start = mtu_get_time(0);
	ret = sqi_write(ctx, addr, sqi_buf, sizeof(sqi_buf));
	speed = ((sizeof(sqi_buf) >> 10) * 1000000) / mtu_get_time(start);
	if (ret)
		TRACE_NOTICE("ERROR\n");
	else
		TRACE_NOTICE("OK at %dKB/s\n", speed);

	TRACE_NOTICE("Reading @%08X... ", addr);
	memset(sqi_buf, 0, sizeof(sqi_buf));
	start = mtu_get_time(0);
	sqi_read(ctx, addr, sqi_buf, sizeof(sqi_buf));
	speed = ((sizeof(sqi_buf) >> 10) * 1000000) / mtu_get_time(start);
	/* Test memory */
	for (i = 0; i < sizeof(sqi_buf); i++) {
		char val = *((char *)sqi_buf + i);
		if (val != (char)i)
			TRACE_NOTICE("ERROR: @%d: %d\n", i, val);
	}
	TRACE_NOTICE("OK at %dKB/s\n", speed);
}

#else

void sqi_tests(struct t_sqi_ctx *ctx) {}

#endif /* SQI Tests */

