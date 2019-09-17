/*
 * STA Quad Serial Peripheral Interface (QSPI) driver
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

//#define DEBUG 0 /* 1 for full data dump */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/hsem.h>
#include <spi.h>
#include <fdtdec.h>
#include <dm/platform_data/sta_qspi.h>
#include <spi_flash.h>
#include "../mtd/spi/sf_internal.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_SPI
#error "You have to enable CONFIG_DM_SPI"
#endif

#ifndef CONFIG_STA_QSPI_REF_CLK
#define CONFIG_STA_QSPI_REF_CLK (204800000)
#endif
/* default SCK frequency, unit: HZ */
#define STA_QSPI_DEFAULT_SCK_FREQ	CONFIG_STA_QSPI_REF_CLK

#define TXRX_BUFFER_SIZE	256

/* STA qspi register set */
struct sta_qspi_regs {
	uint32_t page_buffer[TXRX_BUFFER_SIZE / 4];
	uint32_t cmd_stat;	/* offset = 0x100 */
	uint32_t addr;		/* offset = 0x104 */
	uint32_t data;		/* offset = 0x108 */
	uint32_t conf;		/* offset = 0x10c */
	uint32_t polling;	/* offset = 0x110 */
	uint32_t ext_addr;	/* offset = 0x114 */
	uint32_t conf2;		/* offset = 0x118 */
};

/* CMD_STAT register fields */
#define OFS_WR_CNFG_OPCD         24
#define WR_CNFG_OPCD             GENMASK(31, OFS_WR_CNFG_OPCD)
#define OFS_READ_OPCODE          16
#define READ_OPCODE              GENMASK(23, OFS_READ_OPCODE)

#define OFS_CMD_TYPE		12
#define CMD_TYPE		GENMASK(14, OFS_CMD_TYPE)
/* Command type descriptions */
#define SQI_CMD_TYPE_C		(0 << OFS_CMD_TYPE) /* Cmd */
#define SQI_CMD_TYPE_CAD_WR	(1 << OFS_CMD_TYPE) /* Cmd->Addr->Data, WR */
#define SQI_CMD_TYPE_CD_RD	(2 << OFS_CMD_TYPE) /* Cmd->Data, RD<=4 */
#define SQI_CMD_TYPE_CD_RD4	(3 << OFS_CMD_TYPE) /* Cmd->Data, RD>4 */
#define SQI_CMD_TYPE_CAD_RD	(4 << OFS_CMD_TYPE) /* Cmd->Addr->Data, RD<=4 */
#define SQI_CMD_TYPE_CD_W	(5 << OFS_CMD_TYPE) /* Cmd->Data, WR */
#define SQI_CMD_TYPE_CA		(6 << OFS_CMD_TYPE) /* Cmd->Address */

#define OFS_SQIO_MODE		8
#define SQIO_MODE		GENMASK(9, OFS_SQIO_MODE)
/* Modes: */
#define SQI_MODE_SPI		0 /* CAD: 1/1/1 wire */
#define	SQI_MODE_QPI		1 /* CAD: 4/4/4 wires */
#define	SQI_MODE_QSPI1		2 /* CAD: 1/4/4 wires */
#define	SQI_MODE_QSPI2		3 /* CAD: 1/1/4 wires */

#define SQIO_CMD		GENMASK(7,0)
#define SQI_CMD_WRSR		0x01
#define SQI_CMD_PP		0x02	/* Page program (up to 256 bytes) */
#define SQI_CMD_WRDIS		0x04	/* Write disable */
#define SQI_CMD_RDSR		0x05	/* Read status register */
#define SQI_CMD_WREN		0x06	/* Write enable */
#define SQI_CMD_FAST_READ	0x0B	/* Read data bytes (high frequency) */
#define SQI_CMD_FAST_QUAD_READ	0x6B
#define SQI_CMD_FAST_444_READ	0xEB
#define SQI_CMD_MACRONIX_RDCR	0x15
#define SQI_CMD_READ_CONFIG	0x35
#define SQI_CMD_CD		0x60
#define SQI_CMD_FLAG_STATUS	0x70
#define SQI_CMD_READ_ID		0x9F	/* Read JEDEC ID */
#define SQI_CMD_SBL		0xC0

/* Macronyx Quad Enable/Disable commands */
#define SQI_CMD_EQIO		0x35
#define SQI_CMD_RSTQIO		0xF5

/* Winbond specific */
/* Enable/Disable commands */
#define SQI_CMD_EQIO_WINBOND	0x38
#define SQI_CMD_RSTQIO_WINBOND	0xFF
/* Set Read Parameters */
#define SQI_CMD_SET_READ_PARAM	0xC0

/* Erase commands */
#define SQI_CMD_BE_4K		0x20
#define SQI_CMD_BE_32K		0x52
#define SQI_CMD_BE_64K		0xD8

/* Used for Micron, winbond and Macronix flashes */
#define	SQI_CMD_WREAR		0xC5	/* EAR register write */
#define	SQI_CMD_RDEAR		0xC8	/* EAR reigster read */
#define	SQI_CMD_WREVCR		0x61	/* EVCR reigster read */
#define	SQI_CMD_RDEVCR		0x65	/* EVCR reigster write */
#define	SQI_CMD_WRVCR		0x81	/* VCR reigster read */
#define	SQI_CMD_RDVCR		0x85	/* VCR reigster write */

/* Enhanced Volatile Configuration Register bits */
#define EVCR_QUAD_DIS_MICRON	BIT(7)	/* Micron Quad I/O */

/* Used for Spansion flashes only. */
#define	SQI_CMD_BRRD		0x16	/* Bank register read */
#define	SQI_CMD_BRWR		0x17	/* Bank register write */

/* Used for Spansion S25FS-S family flash only. */
#define SQI_CMD_RDAR		0x65	/* Read any device register */
#define SQI_CMD_WRAR		0x71	/* Write any device register */

#define SQI_CMD_EN4B		0xB7    /* Enter ext 4 Bytes address mode */
#define SQI_CMD_EX4B		0xE9    /* Exit ext 4 Bytes address mode */

/* 4-byte address QSPI CMD - used on Spansion and some Macronix flashes */
#define SQI_CMD_FAST_READ_4B	0x0C    /* Read data bytes (high frequency) */
#define SQI_CMD_PP_4B		0x12    /* Page program (up to 256 bytes) */
#define SQI_CMD_QUAD_PP		0x32    /* Quad Page Program */
#define SQI_CMD_SE_4B		0xDC    /* Sector erase (usually 64KiB) */

/* ADDR register fields */
#define OFS_XFER_LEN          24
#define XFER_LEN              GENMASK(31, OFS_XFER_LEN)
#define MEM_ADDR              GENMASK(23, 0)

/* CONF register fields */
#define DMA_EN                BIT(31)
#define INT_EN                BIT(29)
#define INT_RAW_STATUS        BIT(28)
#define INT_EN_POL            BIT(27)
#define INT_RAW_STATUS_POL    BIT(26)
#define SW_RESET              BIT(24)
#define POL_CNT_PRE_SCL       GENMASK(22, 20)
#define DUMMY_DIS             BIT(18)
#define DUMMY_CYCLES          GENMASK(17, 15)
#define DUMMYCYCLES_SPI_8     (0 << 15)
#define DUMMYCYCLES_SPI_10    (4 << 15)
#define DUMMYCYCLES_SPI_6     (5 << 15)
#define DUMMYCYCLES_SPI_4     (6 << 15)
#define DUMMYCYCLES_QPI_2     (0 << 15)
#define DUMMYCYCLES_QPI_4     (1 << 15)
#define DUMMYCYCLES_QPI_6     (2 << 15)
#define DUMMYCYCLES_QPI_8     (3 << 15)
#define DUMMYCYCLES_QPI_10    (4 << 15)
#define DUMMYCYCLES_SQI1_4    (1 << 15)
#define DUMMYCYCLES_SQI1_6    (2 << 15)
#define DUMMYCYCLES_SQI1_8    (3 << 15)
#define DUMMYCYCLES_SQI1_10   (4 << 15)
#define DUMMYCYCLES_SQI2_4    (1 << 15)
#define DUMMYCYCLES_SQI2_6    (5 << 15)
#define DUMMYCYCLES_SQI2_8    (3 << 15)
#define DUMMYCYCLES_SQI2_10   (4 << 15)

#define CLK_MODE              BIT(8)
#define SPI_CLK_DIV           GENMASK(7, 0)
#define SQI_CLK_DIV_BY(d)     (d - 1)

/* POLLING register fields */
#define POLLING_COUNT         GENMASK(31, 16)
#define POLL_BSY_BIT_INDEX    GENMASK(14, 12)
#define POLL_STATUS           BIT(10)
#define POLL_EN               BIT(9)
#define POLL_START            BIT(8)
#define POLL_CMD              GENMASK(7, 0)

/* CONF2 register fields */
#define BIG_ENDIAN            BIT(31)
#define INV_EXT_SPI_CLK       BIT(25)
#define EXT_SQI_CLK_EN        BIT(24)
#define SQI_PARA_MODE         BIT(16)
#define EXT_MEM_MODE          BIT(15)
#define DUMMY_CYCLE_ALT       BIT(14)
#define DUMMY_CYCLE_NUM_OFS   8
#define DUMMY_CYCLE_NUM       GENMASK(13, DUMMY_CYCLE_NUM_OFS)
#define EXT_MEM_ADDR_8MSB_MSK GENMASK(7, 0)

/* SPI status fields */
#define SPI_STATUS_WP         BIT(7)
#define SPI_STATUS_QEB_MCIX   BIT(6)
#define SPI_STATUS_QEB_MICRON BIT(7)

/* SPI Config registers fields */
#define STATUS_QEB_WINSPAN    BIT(1) /* For WINBOND & SPANSION */
#define OFS_LC_VALUES         6
#define LC_VALUES             GENMASK(7, OFS_LC_VALUES)

/* Spansion Dummy cycles config values */
#define SPANSION_DC_QF_1      (0x03 << OFS_LC_VALUES)
#define SPANSION_DC_QF_4      (0x01 << OFS_LC_VALUES)
#define SPANSION_DC_QF_5      (0x02 << OFS_LC_VALUES)

/* Macronix Dummy cycles config values */
#define MACRONIX_DC_QF_4      (0x01 << OFS_LC_VALUES)
#define MACRONIX_DC_QF_6      (0x00 << OFS_LC_VALUES)
#define MACRONIX_DC_QF_8      (0x02 << OFS_LC_VALUES)
#define MACRONIX_DC_QF_10     (0x03 << OFS_LC_VALUES)

/* Micron Dummy cycles config values */
#define OFS_MICRON_LC_VALUES   4
#define LC_MICRON_VALUES       GENMASK(7, OFS_MICRON_LC_VALUES)
#define MICRON_DC_QF(X)        ((X) << OFS_MICRON_LC_VALUES)

/* The Quad mode to use for Read Memory Array Commands
 * SQI_MODE_QPI: 4/4/4 the fastest
 * SQI_MODE_QSPI1: 1/4/4 compatible with ROM after Reset and Watchdog
 */
#define SQI_QUAD_MODE         SQI_MODE_QPI /* SQI_MODE_QPI or SQI_MODE_QSPI[12]*/

/**
 * struct sta_qspi_priv - private data for STA QSPI
 *
 * @regs: Point to QSPI register structure for I/O access
 * @speed_hz: Default SCK frequency
 * @mode: Read Memory Array mode
 * @mode_rac: Registers Access Commands mode
 * @mode_qpp: Quad Page Program Command mode
 * @cmd_pp: Page Program Command
 * @dummy_cycles: QPI dummy cycles frequency dependent
 * @cur_cmdid: current command id
 * @sf_addr: flash access offset
 * @mmap_base: Base address of QSPI memory mapping
 * @mmap_total_size: size of QSPI memory mapping
 * @manuf_id: Serial Flash manufacturer ID
 */
struct sta_qspi_priv {
	struct sta_qspi_regs *regs;
	uint32_t speed_hz;
	uint32_t cmd_pp;
	uint32_t mode;
	uint32_t mode_rac;
	uint32_t mode_qpp;
	uint32_t dummy_cycles;
	uint32_t cur_cmdid;
	uint32_t sf_addr;
	void *mmap_base;
	uint32_t mmap_total_size;
	uint8_t bank;
	uint8_t manuf_id;
	uint16_t flash_id;
};

/* Last STA QSPI probed driver to deinitialize if needed */
static struct sta_qspi_priv *sta_probed_priv;

static int sta_qspi_child_pre_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct sta_qspi_priv *priv = dev_get_priv(bus);

	slave->memory_map = priv->mmap_base;
	slave->max_write_size = TXRX_BUFFER_SIZE;

	return 0;
}

static int sta_qspi_probe(struct udevice *bus)
{
	struct sta_qspi_platdata *plat = dev_get_platdata(bus);
	struct sta_qspi_priv *priv = dev_get_priv(bus);
	struct dm_spi_bus *dm_spi_bus;

	dm_spi_bus = bus->uclass_priv;

	dm_spi_bus->max_hz = plat->speed_hz;
	priv->speed_hz = plat->speed_hz;

	priv->regs = (struct sta_qspi_regs *)(uintptr_t)plat->reg_base;

	priv->mmap_total_size = (uint32_t)plat->mmap_total_size;
	priv->mmap_base = map_physmem((uint32_t)plat->mmap_base,
			priv->mmap_total_size, MAP_NOCACHE);
	/* Default SPI starting modes */
	priv->mode = priv->mode_rac = priv->mode_qpp = SQI_MODE_SPI;
	/* Default Page Program command */
	priv->cmd_pp = SQI_CMD_PP;
	priv->bank = 0;

	/* Default dummy cycles value */
	priv->dummy_cycles = 8;

	sta_probed_priv = priv;

	return 0;
}

static int sta_qspi_ofdata_to_platdata(struct udevice *bus)
{
	struct fdt_resource res_regs, res_mem;
	struct sta_qspi_platdata *plat = bus->platdata;
	const void *blob = gd->fdt_blob;
	int node = bus->of_offset;
	int ret;

	ret = fdt_get_named_resource(blob, node, "reg", "reg-names",
				     "sqi-regs", &res_regs);
	if (ret) {
		debug("Error: can't get regs base addresses(ret = %d)!\n", ret);
		return -ENOMEM;
	}
	ret = fdt_get_named_resource(blob, node, "reg", "reg-names",
				     "sqi-mmap", &res_mem);
	if (ret) {
		debug("Error: can't get memory map base addresses(ret = %d)!\n",
				ret);
		return -ENOMEM;
	}

	plat->speed_hz = fdtdec_get_int(blob, node, "spi-max-frequency",
					STA_QSPI_DEFAULT_SCK_FREQ);

	plat->reg_base = res_regs.start;
	plat->mmap_base = res_mem.start;
	plat->mmap_total_size = res_mem.end - res_mem.start + 1;

	debug("%s: regs=<0x%lx> <0x%lx, 0x%lx>, max-frequency=%d\n",
	      __func__, plat->reg_base, plat->mmap_base,
	      plat->mmap_total_size, plat->speed_hz);

	return 0;
}

static void
sqi_runcmd(struct sta_qspi_priv *priv, uint8_t mode, uint8_t cmd,
		uint32_t type, uint32_t addr, int len)
{
	struct sta_qspi_regs *regs = priv->regs;
	uint32_t xlen = 0;

	debug("%s: to 0x%.8x, len:%d, cmd:%.2x, mode:%d\n", __func__,
		addr, len, cmd, mode);

	/* Manage address and length registers */
	if (len) {
		if ((len > 4) || (cmd == priv->cmd_pp))
			xlen = ((len + 3) / 4 - 1) << OFS_XFER_LEN;
		else
			xlen = (len - 1) << OFS_XFER_LEN;
	}
	writel(xlen | (addr & MEM_ADDR), &regs->addr);
	writel(addr, &regs->ext_addr);

	/* Manage command register */
	writel(type | cmd | (mode << OFS_SQIO_MODE), &regs->cmd_stat);

	/* Active polling */
	while (readl(&regs->cmd_stat) & SQIO_CMD)
		;
}

static void
sqi_read_reg(struct sta_qspi_priv *priv, uint8_t opcode, uint8_t *buf, int len)
{
	struct sta_qspi_regs *regs = priv->regs;
	int i;
	int shift = 0;
	uint32_t data = 0;

	sqi_runcmd(priv, priv->mode_rac, opcode,
		      (len <= 4) ? SQI_CMD_TYPE_CD_RD : SQI_CMD_TYPE_CD_RD4,
		      0, len);

	/* Read out data */
	if (len >= 4) {
		/* Result is in page_buffer register (Big Endian) */
		for (i = 0; i < len; i++) {
			if ((i & 3) == 0) {
				data = readl(&regs->page_buffer[i / 4]);
				shift = 24;
			}
			*buf++ = (data >> shift);
			shift -= 8;
		}
	} else {
		/* Result is in data register (Little Endian) */
		data = readl(&regs->data);
		for (i = 0; i < len; i++) {
			*buf++ = (data >> shift);
			shift += 8;
		}
	}
#if defined DEBUG && (DEBUG >= 1)
	debug("Rx: ");
	print_buffer(0, buf - len, 1, len, 0);
#endif
}

static void
sqi_write_reg(struct sta_qspi_priv *priv, uint8_t opcode, uint8_t *buf, int len)
{
	struct sta_qspi_regs *regs = priv->regs;
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
			writel(data, &regs->page_buffer[0]);
			type = (opcode << OFS_WR_CNFG_OPCD) | SQI_CMD_TYPE_CD_W;
		} else {
			debug("Bad length: %d\n", len);
		}
	}

	sqi_runcmd(priv, priv->mode_rac, opcode, type, 0, len);
#if defined DEBUG && (DEBUG >= 1)
	if (len) {
		debug("Tx: ");
		print_buffer(0, buf - len, 1, len, 0);
	}
#endif

}

static void sqi_op_write(struct sta_qspi_priv *priv, uint8_t *txbuf,
		uint32_t len)
{
	struct sta_qspi_regs *regs = priv->regs;

	/* len is always < TXRX_BUFFER_SIZE */
	debug("%s: %d bytes @ 0x%08X\n", __func__, len, priv->sf_addr);

	switch (priv->cur_cmdid) {
	case SQI_CMD_WRSR:
#if defined CONFIG_SPI_FLASH_SPANSION || defined CONFIG_SPI_FLASH_WINBOND
		/* Set new SPI mode, following QEB change in CR */
		if ((priv->manuf_id == SPI_FLASH_CFI_MFR_SPANSION ||
				priv->manuf_id == SPI_FLASH_CFI_MFR_WINBOND)
				&& (txbuf[1] & STATUS_QEB_WINSPAN)) {
			uint8_t config = txbuf[1];

			/* Set Dummy Cycles for Quad I/O Fast Read */
			if (priv->manuf_id == SPI_FLASH_CFI_MFR_WINBOND) {
				uint8_t rp;

				if (priv->speed_hz <= 50000000) {
					priv->dummy_cycles = 2;
				} else if (priv->speed_hz <= 90000000) {
					priv->dummy_cycles = 4;
				} else {
					switch (priv->flash_id) {
					case 0x4016: /* W25Q32JV */
						priv->dummy_cycles = 6;
						break;
					default:
						priv->dummy_cycles = 8;
						break;
					}
				}
				/* Set right dummy cycles with Set Read Parameters */
				rp = ((priv->dummy_cycles / 2) - 1) << 4;
				sqi_write_reg(priv, SQI_CMD_SET_READ_PARAM, &rp, 1);
			} else {
				config &= ~LC_VALUES;
				if (priv->speed_hz <= 50000000) {
					config |= SPANSION_DC_QF_1;
					priv->dummy_cycles = 3;
				} else if (priv->speed_hz <= 90000000) {
					config |= SPANSION_DC_QF_4;
					priv->dummy_cycles = 6;
				} else {
					config |= SPANSION_DC_QF_5;
					priv->dummy_cycles = 7;
				}
				txbuf[1] = config;
			}
			sqi_write_reg(priv, priv->cur_cmdid, txbuf, len);

#if SQI_QUAD_MODE == SQI_MODE_QPI
			if (priv->manuf_id == SPI_FLASH_CFI_MFR_WINBOND) {
				/* Switch to Quad mode for WINBOND */
				sqi_write_reg(priv, SQI_CMD_EQIO_WINBOND, NULL, 0);
				priv->mode = SQI_MODE_QPI;
				priv->mode_qpp = SQI_MODE_QPI;
				priv->cmd_pp = SQI_CMD_PP;
			} else
#endif
			{
				priv->mode = SQI_MODE_QSPI1;
				priv->mode_qpp = SQI_MODE_QSPI2;
				priv->cmd_pp = SQI_CMD_QUAD_PP;
			}
			debug("%s, Enter quad mode: 0x%X, DC=%d\n", __func__,
			      priv->mode, priv->dummy_cycles);
		}
#endif
#if defined CONFIG_SPI_FLASH_MACRONIX
		/* Set new SPI mode, following QEB change in SR */
		if (priv->manuf_id == SPI_FLASH_CFI_MFR_MACRONIX
				&& (txbuf[0] & SPI_STATUS_QEB_MCIX)) {
			uint8_t config;
			uint8_t data[2];

			/* Set dummy cycles for Macronix in config register */
			sqi_read_reg(priv, SQI_CMD_MACRONIX_RDCR, &config, 1);
			config &= ~LC_VALUES;

#if SQI_QUAD_MODE == SQI_MODE_QPI
			/* Set Dummy Cycles for Quad I/O Fast Read */
			if (priv->speed_hz <= 70000000) {
				priv->dummy_cycles = 4;
				config |= MACRONIX_DC_QF_4;
			} else if (priv->speed_hz <= 84000000) {
				priv->dummy_cycles = 6;
				config |= MACRONIX_DC_QF_6;
			} else if (priv->speed_hz <= 104000000) {
				priv->dummy_cycles = 8;
				config |= MACRONIX_DC_QF_8;
			} else {
				priv->dummy_cycles = 10;
				config |= MACRONIX_DC_QF_10;
			}

			/* Switch to Quad mode for Macronix */
			sqi_write_reg(priv, SQI_CMD_EQIO, NULL, 0);

			priv->mode_rac = priv->mode_qpp = SQI_MODE_QPI;
#else
			priv->dummy_cycles = 8;
			config |= MACRONIX_DC_QF_8;
			priv->mode_rac = priv->mode_qpp = SQI_MODE_SPI;
#endif
			priv->cmd_pp = SQI_CMD_PP;
			priv->mode = SQI_QUAD_MODE;
			debug("%s, Enter quad mode: 0x%X, DC: %d\n", __func__,
					priv->mode, priv->dummy_cycles);

			data[0] = txbuf[0];
			data[1] = config;
			sqi_write_reg(priv, priv->cur_cmdid, data, sizeof(data));
			break;
		}
#endif /* CONFIG_SPI_FLASH_MACRONIX */
		sqi_write_reg(priv, priv->cur_cmdid, txbuf, len);
		break;

#ifdef CONFIG_SPI_FLASH_BAR
	case SQI_CMD_BRWR:
	case SQI_CMD_WREAR:
		priv->bank = txbuf[0];

		/* Spansion flashes, set bit7 of BAR register (enable 32bits) */
		if (priv->cur_cmdid == SQI_CMD_BRWR)
			txbuf[0] = 0x80;
		sqi_write_reg(priv, priv->cur_cmdid, txbuf, len);
		break;
#endif

	default: /*  Normal page write */
		/* Round up len to 4 bytes */
		len = (len + 3) & ~3;

		/* Buffer not aligned for uint32_t ? */
		if ((uint32_t)txbuf & 3)
			debug("%s: Address %p not aligned\n", __func__, txbuf);

		/* Fill the TX data to the PAGE_BUFFER */
		if (len > TXRX_BUFFER_SIZE)
			len = TXRX_BUFFER_SIZE;

		memcpy(&regs->page_buffer[0], txbuf, len);

		sqi_runcmd(priv, priv->mode_qpp, priv->cmd_pp,
				SQI_CMD_TYPE_CAD_WR,
				(priv->bank << 24) | priv->sf_addr, len);
		break;
	}
}

static void sqi_write_enable(struct sta_qspi_priv *priv)
{
	uint8_t status;

	sqi_write_reg(priv, SQI_CMD_WREN, NULL, 0);
	do {
		/* Wait WEL set to 1 */
		sqi_read_reg(priv, SQI_CMD_RDSR, &status, 1);
	} while (!(status & 2));
}

static void sqi_do_readid(struct sta_qspi_priv *priv, uint8_t *din,
		uint32_t bytes)
{
	uint32_t data;
	struct sta_qspi_regs *regs = priv->regs;

	/* Already initialized ? */
	if (get_sqi_manu_id() != 0) {
		priv->manuf_id = din[0] = get_sqi_manu_id();
		priv->flash_id = get_sqi_flash_id();
		din[1] = priv->flash_id >> 8;
		din[2] = (uint8_t) priv->flash_id;
		din[3] = get_sqi_jdec_extid() >> 8;
		din[4] = (uint8_t) get_sqi_jdec_extid();
		/* Get dummy cycles from conf_reg2 register */
		priv->dummy_cycles = (readl(&regs->conf2) & DUMMY_CYCLE_NUM)
					>> DUMMY_CYCLE_NUM_OFS;
		/* Some manufacturer dependent specific settings */
		switch (priv->manuf_id) {
		case SPI_FLASH_CFI_MFR_SPANSION:
			/* Already in QPI mode */
			priv->mode = SQI_MODE_QSPI1;
			priv->mode_qpp = SQI_MODE_QSPI2;
			priv->cmd_pp = SQI_CMD_QUAD_PP;
			break;
		case SPI_FLASH_CFI_MFR_WINBOND:
			switch (priv->flash_id) {
			case 0x4017: /* W25Q64FV (SPI) */
				/* Stay in SPI mode */
				break;
			default:
				/* Switch to QSPIx mode */
				priv->mode = SQI_MODE_QSPI1;
				priv->mode_qpp = SQI_MODE_QSPI2;
				priv->cmd_pp = SQI_CMD_QUAD_PP;
				break;
			}
			break;
		case SPI_FLASH_CFI_MFR_STMICRO:
			priv->mode = priv->mode_rac = priv->mode_qpp = SQI_MODE_QPI;
			priv->cmd_pp = SQI_CMD_QUAD_PP;
			break;
		case SPI_FLASH_CFI_MFR_MACRONIX:
			priv->mode = SQI_QUAD_MODE;
#if SQI_QUAD_MODE == SQI_MODE_QPI
			priv->mode_rac = priv->mode_qpp = SQI_MODE_QPI;
#else
			priv->mode_rac = priv->mode_qpp = SQI_MODE_SPI;
#endif
			priv->cmd_pp = SQI_CMD_PP;
		default:
			break;
		}
	} else {
		/* first init */
		sqi_read_reg(priv, SQI_CMD_READ_ID, din, bytes);
		priv->manuf_id = din[0];
		priv->flash_id = (din[1] << 8) | din[2];

#ifdef CONFIG_SPI_FLASH_BAR
		/* Enable 4 bytes address mode */
#if defined CONFIG_SPI_FLASH_SPANSION
		/* Set bit7 of bank register (enable 32bits) */
		if (priv->manuf_id == SPI_FLASH_CFI_MFR_SPANSION) {
			uint8_t data = 0x80;
			sqi_write_reg(priv, SQI_CMD_BRWR, &data, 1);
		}
#endif
#if defined CONFIG_SPI_FLASH_MACRONIX || defined CONFIG_SPI_FLASH_STMICRO \
	    || defined CONFIG_SPI_FLASH_WINBOND
		/* Macronix like serial flashes */
		if (priv->manuf_id == SPI_FLASH_CFI_MFR_MACRONIX
				|| priv->manuf_id == SPI_FLASH_CFI_MFR_STMICRO)
			sqi_write_reg(priv, SQI_CMD_EN4B, NULL, 0);
#endif

		data = readl(&regs->conf2) & ~(EXT_MEM_MODE | EXT_MEM_ADDR_8MSB_MSK);
		data |= EXT_MEM_MODE | 0x0F;
		writel(data, &regs->conf2);

#if defined CONFIG_SPI_FLASH_STMICRO && SQI_QUAD_MODE == SQI_MODE_QPI
		if (priv->manuf_id == SPI_FLASH_CFI_MFR_STMICRO) {
			uint8_t evcr_val;
			uint8_t status;

			priv->dummy_cycles = 10;
			sqi_read_reg(priv, SQI_CMD_RDEVCR, &evcr_val, 1);

			evcr_val &=~ EVCR_QUAD_DIS_MICRON; /* Clear disable QUAD bit */
			sqi_write_enable(priv);
			sqi_write_reg(priv, SQI_CMD_WREVCR, &evcr_val, 1);

			/* Now switch to QPI */
			priv->mode = priv->mode_rac = priv->mode_qpp = SQI_MODE_QPI;
			priv->cmd_pp = SQI_CMD_QUAD_PP;

			do {
				/* Wait WEL set to 0 */
				sqi_read_reg(priv, SQI_CMD_RDSR, &status, 1);
			} while (status & 2);

#if defined DEBUG
			{
				uint8_t vcr_val;

				sqi_read_reg(priv, SQI_CMD_RDVCR, &vcr_val, 1),
					sqi_read_reg(priv, SQI_CMD_RDEVCR, &evcr_val, 1);
				debug("VCR: %02X, EVCR: %02X\n", vcr_val, evcr_val);
			}
#endif
		}
#endif

#endif /* CONFIG_SPI_FLASH_BAR */
	}
}

static int sta_qspi_xfer(struct udevice *dev, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct sta_qspi_priv *priv;
	struct sta_qspi_regs *regs;
	uint32_t bytes = DIV_ROUND_UP(bitlen, 8);
	uint32_t cmd;
	uint32_t conf_r;
	static uint32_t wr_sfaddr;

	priv = dev_get_priv(dev->parent);
	regs = priv->regs;


	if (flags & SPI_XFER_MMAP) {
		switch (priv->mode) {
		case SQI_MODE_SPI:
			cmd = SQI_CMD_FAST_READ;
			break;
		default: /* QUAD modes */
			cmd = SQI_CMD_FAST_444_READ;
			break;
		}
		writel((cmd << OFS_READ_OPCODE) | (priv->mode << OFS_SQIO_MODE),
				&regs->cmd_stat);
		while (readl(&regs->cmd_stat) & SQIO_CMD)
			;

		conf_r = readl(&regs->conf2);
		conf_r &= ~DUMMY_CYCLE_NUM;
		conf_r |= (priv->dummy_cycles << DUMMY_CYCLE_NUM_OFS)
				| DUMMY_CYCLE_ALT;
		writel(conf_r, &regs->conf2);

		conf_r = readl(&regs->conf);
		conf_r &= ~DUMMY_DIS; /* Enable DUMMY cycles */
		writel(conf_r, &regs->conf);
		debug("%s: mmap start, cmd: %02X, mode: %d, dc: %d\n", __func__,
				cmd, priv->mode, priv->dummy_cycles);

		return 0;
	}
	if (flags & SPI_XFER_MMAP_END) {
		/* Disable dummy cycle & reset cmd_stat... */
		conf_r = readl(&regs->conf);
		conf_r |= DUMMY_DIS;
		writel(conf_r, &regs->conf);
		debug("%s: mmap end\n", __func__);

		return 0;
	}

	if (bitlen == 0)
		return -1;

	if (dout) {
		if (flags & SPI_XFER_BEGIN) {
			priv->cur_cmdid = *(uint8_t *)dout;
			if (bytes >= 4) {
				memcpy(&priv->sf_addr, dout, 4);
				priv->sf_addr = swab32(priv->sf_addr) & MEM_ADDR;
			} else {
				priv->sf_addr = 0;
			}
			debug("%s: begin%s: cmd (0x%02X): flash_addr: 0x%08X\n",
				__func__,
				(flags & SPI_XFER_END) ? " and end" : "",
				priv->cur_cmdid, priv->sf_addr);
		} else {
			debug("%s: cmd (out): 0x%02X: len: %u\n",
				__func__, priv->cur_cmdid, bytes);
		}

		if (flags == SPI_XFER_END) {
			priv->sf_addr = wr_sfaddr;
			sqi_op_write(priv, (uint8_t *)dout, bytes);
			return 0;
		}

		switch (priv->cur_cmdid) {
		case SQI_CMD_WREN:
		case SQI_CMD_WRDIS:
			sqi_write_reg(priv, priv->cur_cmdid, NULL, 0);
			break;


		case SQI_CMD_BE_4K:
		case SQI_CMD_BE_32K:
		case SQI_CMD_BE_64K:
			sqi_runcmd(priv, priv->mode_rac,  priv->cur_cmdid, 0,
					(priv->bank << 24) | priv->sf_addr, 0);
			break;

		case SQI_CMD_PP:
		case SQI_CMD_WRAR:
		case SQI_CMD_QUAD_PP:
			wr_sfaddr = priv->sf_addr;
			break;

		default:
			debug("%s: cmd: 0x%02X not managed\n",
					__func__, priv->cur_cmdid);
		case SQI_CMD_WRSR:
		case SQI_CMD_READ_ID:
		case SQI_CMD_READ_CONFIG:
		case SQI_CMD_RDSR:
		case SQI_CMD_FLAG_STATUS:
#ifdef CONFIG_SPI_FLASH_BAR
		case SQI_CMD_BRWR:
		case SQI_CMD_WREAR:
		case SQI_CMD_BRRD:
		case SQI_CMD_RDEAR:
#endif
			break;
		}
	}

	if (din) {
		static bool config_read = false;

		debug("%s: cmd (in): 0x%02X, len: %u\n", __func__,
				priv->cur_cmdid, bytes);
		switch (priv->cur_cmdid) {
		case SQI_CMD_READ_ID:
			sqi_do_readid(priv, din, bytes);
			break;

		case SQI_CMD_READ_CONFIG: /* Read config reg */
			sqi_read_reg(priv,
				(priv->manuf_id == SPI_FLASH_CFI_MFR_MACRONIX) ?
					SQI_CMD_MACRONIX_RDCR : priv->cur_cmdid,
					din, bytes);

			if (((priv->manuf_id == SPI_FLASH_CFI_MFR_SPANSION) ||
			    (priv->manuf_id == SPI_FLASH_CFI_MFR_WINBOND)) &&
			    (*(uint8_t *)din & STATUS_QEB_WINSPAN) && !config_read) {
				debug("Clear QEB\n");
				*(uint8_t *)din &= ~STATUS_QEB_WINSPAN;
			}
			config_read = true;
			break;

		default:
			sqi_read_reg(priv, priv->cur_cmdid, din, bytes);
			if (priv->cur_cmdid == SQI_CMD_BRRD)
				*(uint8_t *)din &= 0x7F;
			break;
		}
	}

	return 0;
}

static int sta_qspi_claim_bus(struct udevice *dev)
{
#if defined CONFIG_STA_QSPI_SHARED
	while (!hsem_trylock(HSEM_SQI_NOR_AP_ID, 0))
		;
#endif

	return 0;
}

static int sta_qspi_release_bus(struct udevice *dev)
{
#if defined CONFIG_STA_QSPI_SHARED
	hsem_unlock(HSEM_SQI_NOR_AP_ID);
#endif

	return 0;
}

static int sta_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct sta_qspi_platdata *plat = dev_get_platdata(bus);
	struct sta_qspi_priv *priv = dev_get_priv(bus);
	struct sta_qspi_regs *regs = priv->regs;
	uint32_t conf_r, divisor;

	if (speed > plat->speed_hz)
		speed = plat->speed_hz;

	/* Enable feedback clock at the right speed */
	conf_r = readl(&regs->conf);
	conf_r &= ~(SW_RESET | SPI_CLK_DIV);
	conf_r |= SW_RESET | DUMMY_DIS;

	/* Set the CLK divisor */
	divisor = CONFIG_STA_QSPI_REF_CLK / speed;
	conf_r |= SQI_CLK_DIV_BY(divisor);
	writel(conf_r, &regs->conf);
	priv->speed_hz = CONFIG_STA_QSPI_REF_CLK / (((divisor - 1) & ~1) + 2);

	/* Enable feedback clock */
	conf_r = readl(&regs->conf2);
	conf_r |= EXT_SQI_CLK_EN;
	writel(conf_r, &regs->conf2);

	debug("%s: manu_id=%X, regs=%p, speed: ask=%d set=%d, div=%d, dummy cycles: %d\n",
		__func__, priv->manuf_id, priv->regs, speed,
		priv->speed_hz, divisor, priv->dummy_cycles);

	return 0;
}

static int sta_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct sta_qspi_priv *priv = dev_get_priv(bus);
	struct sta_qspi_regs *regs = priv->regs;
	uint32_t conf_r;

	/* Set the SPI Clock mode */
	conf_r = readl(&regs->conf);
	conf_r &= ~(CLK_MODE);

	switch (mode & (SPI_CPHA | SPI_CPOL)) {
	case SPI_MODE_3:
		conf_r |= CLK_MODE;
		break;
	case SPI_MODE_0:
	default: /* Others unsupported mode => MODE0 */
		break;
	}

	debug("%s: spi/mode=%d\n", __func__, mode);
	writel(conf_r, &regs->conf);

	return 0;
}

void sta_qspi_deinit(void) {
	struct sta_qspi_priv *priv = sta_probed_priv;

	if (priv && priv->mode == SQI_MODE_QPI) {
		debug("%s: Exit quad mode\n", __func__);

#if defined CONFIG_SPI_FLASH_STMICRO
		if (priv->manuf_id == SPI_FLASH_CFI_MFR_STMICRO) {
			uint8_t evcr_val;
			uint8_t status;

			sqi_read_reg(priv, SQI_CMD_RDEVCR, &evcr_val, 1);

			evcr_val |= EVCR_QUAD_DIS_MICRON; /* Set disable QUAD bit */
			sqi_write_enable(priv);
			sqi_write_reg(priv, SQI_CMD_WREVCR, &evcr_val, 1);

			/* Now switch to SPI */
			priv->mode = priv->mode_rac = priv->mode_qpp
				= SQI_MODE_SPI;
			do {
				/* Wait WEL set to 0 */
				sqi_read_reg(priv, SQI_CMD_RDSR, &status, 1);
			} while (status & 2);
		}
#endif
#if defined CONFIG_SPI_FLASH_MACRONIX
		if (priv->manuf_id == SPI_FLASH_CFI_MFR_MACRONIX)
			sqi_write_reg(priv, SQI_CMD_RSTQIO, NULL, 0);
#endif
#if defined CONFIG_SPI_FLASH_WINBOND
		if (priv->manuf_id == SPI_FLASH_CFI_MFR_WINBOND)
			sqi_write_reg(priv, SQI_CMD_RSTQIO_WINBOND, NULL, 0);
#endif
	}
}

static const struct dm_spi_ops sta_qspi_ops = {
	.claim_bus	= sta_qspi_claim_bus,
	.release_bus	= sta_qspi_release_bus,
	.xfer		= sta_qspi_xfer,
	.set_speed	= sta_qspi_set_speed,
	.set_mode	= sta_qspi_set_mode,
};

static const struct udevice_id sta_qspi_ids[] = {
	{ .compatible = "st,sta-qspi" },
	{ }
};

U_BOOT_DRIVER(sta_qspi) = {
	.name	= "sta_qspi",
	.id	= UCLASS_SPI,
	.of_match = sta_qspi_ids,
	.ops	= &sta_qspi_ops,
	.ofdata_to_platdata = sta_qspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct sta_qspi_platdata),
	.priv_auto_alloc_size = sizeof(struct sta_qspi_priv),
	.probe	= sta_qspi_probe,
	.child_pre_probe = sta_qspi_child_pre_probe,
};
