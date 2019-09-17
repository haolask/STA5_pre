/*
 * Driver for STA QSPI Controller
 *
 * Copyright (C) 2017 ST Microelectronics Automotive
 *
 * Author: Philippe Langlais <philippe.langlais@st.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This driver is based on drivers/mtd/spi-nor/fsl-quadspi.c from Freescale.
 */

#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mtd/spi-nor.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>
#include <linux/hwspinlock.h>
#include <linux/slab.h>
#include <linux/sta_hsem.h>

#define TXRX_BUFFER_SIZE	256

/* STA QSPI register offsets */
#define QSPI_PAGE_BUFFER	0
#define QSPI_CMD_STAT_REG	0x100
#define QSPI_ADDR_REG		0x104
#define QSPI_DATA_REG		0x108
#define	QSPI_CONF_REG1		0x10c
#define QSPI_POLLING_REG	0x110
#define QSPI_EXT_MEM_ADD_REG	0x114
#define QSPI_CONF_REG2		0x118

/* CMD_STAT register fields */
#define OFS_WR_CNFG_OPCD	24
#define WR_CNFG_OPCD		GENMASK(31, OFS_WR_CNFG_OPCD)
#define OFS_READ_OPCODE		16
#define READ_OPCODE		GENMASK(23, OFS_READ_OPCODE)

#define OFS_CMD_TYPE		12
#define CMD_TYPE		GENMASK(14, OFS_CMD_TYPE)
/* Command type descriptions */
#define SQI_CMD_TYPE_C		(0x0 << OFS_CMD_TYPE) /* Cmd */
#define SQI_CMD_TYPE_CAD_WR	(0x1 << OFS_CMD_TYPE) /* Cmd->Addr->Data,WR */
#define SQI_CMD_TYPE_CD_RD	(0x2 << OFS_CMD_TYPE) /* Cmd->Data,RD<=4 */
#define SQI_CMD_TYPE_CD_RD4	(0x3 << OFS_CMD_TYPE) /* Cmd->Data,RD>4 */
#define SQI_CMD_TYPE_CAD_RD	(0x4 << OFS_CMD_TYPE) /* Cmd->Add->Data,RD<=4 */
#define SQI_CMD_TYPE_CD_W	(0x5 << OFS_CMD_TYPE) /* Cmd->Data,WR */
#define SQI_CMD_TYPE_CA		(0x6 << OFS_CMD_TYPE) /* Cmd->Address */

#define OFS_SQIO_MODE		8
#define SQIO_MODE		GENMASK(9, OFS_SQIO_MODE)
/* Modes: */
#define SQI_MODE_SPI		0 /* CAD: 1/1/1 wire */
#define	SQI_MODE_QPI		1 /* CAD: 4/4/4 wires */
#define	SQI_MODE_QSPI1		2 /* CAD: 1/4/4 wires */
#define	SQI_MODE_QSPI2		3 /* CAD: 1/1/4 wires */

#define SQIO_CMD		GENMASK(7, 0)

/* ADDR register fields */
#define OFS_XFER_LEN		24
#define XFER_LEN		GENMASK(31, OFS_XFER_LEN)
#define MEM_ADDR		GENMASK(23, 0)

/* CONF1 register fields */
#define DMA_EN			BIT(31)
#define INT_EN			BIT(29)
#define INT_RAW_STATUS		BIT(28)
#define INT_EN_POL		BIT(27)
#define INT_RAW_STATUS_POL	BIT(26)
#define SW_RESET		BIT(24)
#define POL_CNT_PRE_SCL		GENMASK(22, 20)
#define DUMMY_DIS		BIT(18)
#define DUMMY_CYCLES		GENMASK(17, 15)
#define DUMMYCYCLES_SPI_8	(0x0 << 15)
#define DUMMYCYCLES_SPI_10	(0x4 << 15)
#define DUMMYCYCLES_SPI_6	(0x5 << 15)
#define DUMMYCYCLES_SPI_4	(0x6 << 15)
#define DUMMYCYCLES_QPI_2	(0x0 << 15)
#define DUMMYCYCLES_QPI_4	(0x1 << 15)
#define DUMMYCYCLES_QPI_6	(0x2 << 15)
#define DUMMYCYCLES_QPI_8	(0x3 << 15)
#define DUMMYCYCLES_QPI_10	(0x4 << 15)
#define DUMMYCYCLES_SQI1_4	(0x1 << 15)
#define DUMMYCYCLES_SQI1_6	(0x2 << 15)
#define DUMMYCYCLES_SQI1_8	(0x3 << 15)
#define DUMMYCYCLES_SQI1_10	(0x4 << 15)
#define DUMMYCYCLES_SQI2_4	(0x1 << 15)
#define DUMMYCYCLES_SQI2_6	(0x5 << 15)
#define DUMMYCYCLES_SQI2_8	(0x3 << 15)
#define DUMMYCYCLES_SQI2_10	(0x4 << 15)
#define CLK_MODE		BIT(8)
#define SPI_CLK_DIV		GENMASK(7, 0)
#define SQI_CLK_DIV_BY(d)	((d) - 1)

/* POLLING register fields */
#define POLLING_COUNT		GENMASK(31, 16)
#define POLL_BSY_BIT_INDEX	GENMASK(14, 12)
#define POLL_STATUS		BIT(10)
#define POLL_EN			BIT(9)
#define POLL_START		BIT(8)
#define POLL_CMD		GENMASK(7, 0)

/* CONF2 register fields */
#define BIG_ENDIAN		BIT(31)
#define INV_EXT_SPI_CLK		BIT(25)
#define EXT_SQI_CLK_EN		BIT(24)
#define SQI_PARA_MODE		BIT(16)
#define EXT_MEM_MODE		BIT(15)
#define DUMMY_CYCLE_ALT		BIT(14)
#define DUMMY_CYCLE_NUM_OFS	8
#define DUMMY_CYCLE_NUM		GENMASK(13, DUMMY_CYCLE_NUM_OFS)
#define EXT_MEM_ADDR_8MSB_MSK	GENMASK(7, 0)

/* default quad mode 1/1/4 or 1/4/4 or 4/4/4 */
#ifdef CONFIG_SPI_NOR_QUAD_444_MODE
#define QSPI_QUAD_MODE		SQI_MODE_QPI
#else
#define QSPI_QUAD_MODE		SQI_MODE_QSPI1
#endif

struct sta_qspi {
	void __iomem		*regs;
	void __iomem		*ahb_mem;
	struct clk		*clk;
	struct device		*dev;
	u32			pending;

	struct spi_nor		nor;
	u32			clk_rate;
#if defined CONFIG_SPI_STA_QSPI_SHARED
	struct hwspinlock	*hsem_m3;
	struct hwspinlock	*hsem_ap;
#endif
};

struct sta_qspi_command {
	u8	read_opcode;
	u8	wr_cnfg_opcode;
	u8	instruction;
	u8	mode;
	u8	num_mode_cycles;
	u8	num_dummy_cycles;
	u32	address;

	size_t		buf_len;
	const void	*tx_buf;
	void		*rx_buf;
};

/* Register access functions */
static inline u32 qspi_readl(struct sta_qspi *q, u32 reg)
{
	return readl(q->regs + reg);
}

static inline void qspi_writel(struct sta_qspi *q, u32 reg, u32 value)
{
	writel(value, q->regs + reg);
}

static int
sta_qspi_runcmd(struct spi_nor *nor, u8 cmd, u32 type, u32 addr, int len)
{
	int err = 0;
	u32 xlen = 0;
	struct sta_qspi *q = nor->priv;
	u8 mode = SQI_MODE_SPI;

	if (nor->flash_read == SPI_NOR_QUAD && QSPI_QUAD_MODE == SQI_MODE_QPI)
		mode = QSPI_QUAD_MODE;

	dev_dbg(q->dev, "to 0x%.8x, len:%d, cmd:%.2x, mode:%d\n",
		addr, len, cmd, mode);

	/* Manage address and length registers */
	if (len) {
		if ((len > 4) || (cmd == nor->program_opcode))
			/* xlen = roundup(len in 4 bytes word) - 1 */
			xlen = ((len + 3) / 4 - 1) << OFS_XFER_LEN;
		else
			/* xlen = (len in bytes) - 1 */
			xlen = (len - 1) << OFS_XFER_LEN;
	}
	qspi_writel(q, QSPI_ADDR_REG, xlen | (addr & MEM_ADDR));
	qspi_writel(q, QSPI_EXT_MEM_ADD_REG, addr);

	/* Manage command register */
	qspi_writel(q, QSPI_CMD_STAT_REG, type | cmd
			| (mode << OFS_SQIO_MODE));

	/* Active polling */
	while (qspi_readl(q, QSPI_CMD_STAT_REG) & SQIO_CMD)
		;

	return err;
}

static int
sta_qspi_read_reg(struct spi_nor *nor, u8 opcode, u8 *buf, int len)
{
	int i;
	int shift = 0;
	u32 data = 0;
	struct sta_qspi *q = nor->priv;
	int ret;

	ret = sta_qspi_runcmd(nor, opcode,
			      (len <= 4) ? SQI_CMD_TYPE_CD_RD
					 : SQI_CMD_TYPE_CD_RD4,
			      0, len);
	if (ret)
		return ret;

	if (!len)
		return 0;

	/* Read out data */
	if (len >= 4) {
		/* Result is in PAGE_BUFFER register (words in Big Endian) */
		for (i = 0; i < len; i++) {
			if ((i & 3) == 0) {
				/* Word aligned => read PAGE_BUFFER */
				data = qspi_readl(q, QSPI_PAGE_BUFFER + i);
				shift = 24;
			}
			*buf++ = (data >> shift);
			shift -= 8;
		}
	} else {
		/* Result is in data register (Little Endian) */
		data = qspi_readl(q, QSPI_DATA_REG);
		for (i = 0; i < len; i++) {
			*buf++ = (data >> shift);
			shift += 8;
		}
	}
	print_hex_dump_bytes("sqi Rx: ", DUMP_PREFIX_NONE, buf - len, len);

	return 0;
}

static int
sta_qspi_write_reg(struct spi_nor *nor, u8 opcode, u8 *buf, int len)
{
	struct sta_qspi *q = nor->priv;
	int i, shift;
	u32 data = 0;
	u32 type;

	if (len == 0) {
		type = SQI_CMD_TYPE_C;
	} else if (len <= 4) {
		print_hex_dump_bytes("sqi Tx: ", DUMP_PREFIX_NONE, buf, len);
		/* Write word into PAGE_BUFFER[0] in Big endian */
		for (i = 0, shift = 24; i < len; i++) {
			data |= (*buf++ << shift);
			shift -= 8;
		}
		qspi_writel(q, QSPI_PAGE_BUFFER, data);
		type = (1 << OFS_WR_CNFG_OPCD) | SQI_CMD_TYPE_CD_W;
	} else {
		dev_err(q->dev, "bad len : %d\n", len);
		return -EINVAL;
	}

	return sta_qspi_runcmd(nor, opcode, type, 0, len);
}

static int sta_qspi_erase(struct spi_nor *nor, loff_t offs)
{
	dev_dbg(nor->dev, "Erase %dKiB at 0x%08x\n",
		nor->mtd.erasesize / 1024, (u32)offs);

	return sta_qspi_runcmd(nor, nor->erase_opcode,
			      SQI_CMD_TYPE_C, offs, 0);
}

static ssize_t
sta_qspi_read(struct spi_nor *nor, loff_t from, size_t len, u_char *buf)
{
	struct sta_qspi *q = nor->priv;
	u32 conf_r1 = qspi_readl(q, QSPI_CONF_REG1);
	u32 conf_r2 = qspi_readl(q, QSPI_CONF_REG2);
	u8 mode;

	switch (nor->read_opcode) {
	case SPINOR_OP_READ_1_1_4:
#if QSPI_QUAD_MODE != SQI_MODE_QSPI2
		nor->read_opcode = SPINOR_OP_READ_1_4_4;
#endif
		/* FALLTHRU */
	case SPINOR_OP_READ_1_4_4:
		mode = QSPI_QUAD_MODE;
		break;

	case SPINOR_OP_READ_1_1_2:
		return -EINVAL;

	default:
		mode = SQI_MODE_SPI; /* SPI 1 wire default mode */
		break;
	}

	dev_dbg(q->dev, "cmd [%x] read from 0x%.8x, len:%d, dc=%d, mode=%d\n",
		nor->read_opcode, (u32)from, len, nor->read_dummy, mode);

	qspi_writel(q, QSPI_CMD_STAT_REG,
		    (nor->read_opcode << OFS_READ_OPCODE)
		    | (mode << OFS_SQIO_MODE));

	while (qspi_readl(q, QSPI_CMD_STAT_REG) & SQIO_CMD)
		;

	conf_r2 &= ~DUMMY_CYCLE_NUM;
	conf_r2 |= (nor->read_dummy << DUMMY_CYCLE_NUM_OFS) | DUMMY_CYCLE_ALT;
	qspi_writel(q, QSPI_CONF_REG2, conf_r2);

	conf_r1 &= ~DUMMY_DIS; /* Enable dummy cycles */
	qspi_writel(q, QSPI_CONF_REG1, conf_r1);

	/* Read out the data directly from the AHB buffer.*/
	memcpy(buf, q->ahb_mem + from, len);

#if defined DEBUG && DEBUG > 1
	print_hex_dump_bytes("sqi Rx: ", DUMP_PREFIX_NONE, buf, len);
#endif

	conf_r1 |= DUMMY_DIS; /* Disable dummy cycles */
	qspi_writel(q, QSPI_CONF_REG1, conf_r1);

	return len;
}

static ssize_t sta_qspi_write(struct spi_nor *nor, loff_t to,
			      size_t len, const u_char *buf)
{
	int ret, i;
	struct sta_qspi *q = nor->priv;
	u32 *txbuf = (u32 *)buf;
	u32 *tmp_align_buf = NULL;
	size_t remaining_len = len & (sizeof(u32) - 1);

	if ((u32)txbuf & 3) { /* Buffer not aligned for u32 */
		tmp_align_buf = kmalloc(len, GFP_KERNEL);
		if (!tmp_align_buf)
			return -ENOMEM;
		txbuf = tmp_align_buf;
		memcpy(txbuf, buf, len);
	}

	dev_dbg(q->dev, "Write to 0x%.8x, len:%d\n", (u32)to, len);

	/* Fill the TX data to the PAGE_BUFFER */
	if (len > TXRX_BUFFER_SIZE)
		len = TXRX_BUFFER_SIZE;

	for (i = 0; i < (len / sizeof(u32)); i++)
		qspi_writel(q, QSPI_PAGE_BUFFER + (i * sizeof(u32)), *txbuf++);

	/* Needs to write non 32 bits aligned length ? */
	if (remaining_len) {
		u32 data = *txbuf;

		/* Read from flash relicate data that must not be modified */
		sta_qspi_read(nor, to + len, sizeof(data) - remaining_len,
			      ((char *)&data + remaining_len));

		qspi_writel(q, QSPI_PAGE_BUFFER + (i * sizeof(u32)), data);
	}

	ret = sta_qspi_runcmd(nor, nor->program_opcode, SQI_CMD_TYPE_CAD_WR, to,
			      (len + (sizeof(u32) - 1)) & ~(sizeof(u32) - 1));

#if defined DEBUG && DEBUG > 1
	print_hex_dump_bytes("sqi Tx: ", DUMP_PREFIX_NONE, buf, len);
#endif

	kfree(tmp_align_buf);

	if (ret == 0)
		return len;

	return ret;
}

static int sta_qspi_init(struct sta_qspi *q)
{
	unsigned long src_rate;
	u32 conf_r, divisor;

	/* Enable feedback clock */
	conf_r = qspi_readl(q, QSPI_CONF_REG2);
	conf_r |= EXT_SQI_CLK_EN;
	qspi_writel(q, QSPI_CONF_REG2, conf_r);

	conf_r = qspi_readl(q, QSPI_CONF_REG1);
	conf_r &= ~(SW_RESET | SPI_CLK_DIV);

	/* Reset the QSPI controller & disable DC */
	conf_r |= SW_RESET | DUMMY_DIS;

	/* Use SPI clock MODE 3 by default */
	conf_r |= CLK_MODE;

	src_rate = clk_get_rate(q->clk);
	if (!src_rate)
		return -EINVAL;

	/*
	 * Compute the QSPI baudrate following UM1295 SQIO chapter 25:
	 * CONF_REG1 Bits 7:0   SPI_CLK_DIV (SQIO master clock divider)
	 *                         0x00, 0x01 = divide by 2
	 *                         0x02, 0x03 = divide by 4
	 *                         0x04, 0x05 = divide by 6
	 *                          ...
	 *                         0xFE, 0xFF = divide by 256
	 */
	divisor = DIV_ROUND_UP(src_rate, q->clk_rate);
	conf_r |= SQI_CLK_DIV_BY(divisor);
	q->clk_rate = src_rate / (((divisor - 1) & ~1) + 2); /* True baudrate */

	dev_dbg(q->dev, "clk rate: %uHz\n", q->clk_rate);

	/* Enable the QSPI controller, reset with right parameters */
	qspi_writel(q, QSPI_CONF_REG1, conf_r);

	return 0;
}

#if defined CONFIG_SPI_STA_QSPI_SHARED

static int sta_qspi_prepare(struct spi_nor *nor, enum spi_nor_ops ops)
{
	struct sta_qspi *q = nor->priv;
	int ret = 0;

	if (ops == SPI_NOR_OPS_READ) {
		while ((ret = sta_hsem_trylock(q->hsem_ap)) == -EBUSY)
			;
	} else {
		while ((ret = sta_hsem_trylock(q->hsem_m3)) == -EBUSY)
			;
		while ((ret = sta_hsem_trylock(q->hsem_ap)) == -EBUSY)
			;
	}

	return ret;
}

static void sta_qspi_unprepare(struct spi_nor *nor, enum spi_nor_ops ops)
{
	struct sta_qspi *q = nor->priv;

	sta_hsem_unlock(q->hsem_ap);

	if (ops != SPI_NOR_OPS_READ) /* Hence Write or Erase */
		sta_hsem_unlock(q->hsem_m3);
}

#endif /* CONFIG_SPI_STA_QSPI_SHARED */

static int sta_qspi_probe(struct platform_device *pdev)
{
	struct device_node *child, *np = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	struct sta_qspi *q;
	struct resource *res;
	struct spi_nor *nor;
	struct mtd_info *mtd;
	int err = 0;

	if (of_get_child_count(np) != 1)
		return -ENODEV;
	child = of_get_next_child(np, NULL);

	q = devm_kzalloc(dev, sizeof(*q), GFP_KERNEL);
	if (!q) {
		err = -ENOMEM;
		goto exit;
	}

	platform_set_drvdata(pdev, q);
	q->dev = dev;

	/* Map the registers */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sqi-regs");
	q->regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(q->regs)) {
		dev_err(dev, "missing registers\n");
		err = PTR_ERR(q->regs);
		goto exit;
	}

	/* Map the AHB memory */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "sqi-mmap");
	q->ahb_mem = devm_ioremap_resource(dev, res);
	if (IS_ERR(q->ahb_mem)) {
		dev_err(dev, "missing AHB memory\n");
		err = PTR_ERR(q->ahb_mem);
		goto exit;
	}
	dev_dbg(q->dev, "sqi mmap phy @%x to virt @%p\n",
		res->start, q->ahb_mem);

	/* Get the peripheral clock */
	q->clk = devm_clk_get(dev, NULL);
	if (IS_ERR(q->clk)) {
		dev_err(dev, "missing peripheral clock\n");
		err = PTR_ERR(q->clk);
		goto exit;
	}

	/* Enable the peripheral clock */
	err = clk_prepare_enable(q->clk);
	if (err) {
		dev_err(dev, "failed to enable the peripheral clock\n");
		goto exit;
	}

	/* Setup the spi-nor */
	nor = &q->nor;
	mtd = &nor->mtd;

	nor->dev = dev;
	spi_nor_set_flash_node(nor, child);
	nor->priv = q;
	mtd->priv = nor;

	nor->read_reg = sta_qspi_read_reg;
	nor->write_reg = sta_qspi_write_reg;
	nor->read = sta_qspi_read;
	nor->write = sta_qspi_write;
	nor->erase = sta_qspi_erase;
#if defined CONFIG_SPI_STA_QSPI_SHARED
	q->hsem_m3 = sta_hsem_lock_request(HSEM_SQI_NOR_M3_ID, 0, NULL, NULL);
	q->hsem_ap = sta_hsem_lock_request(HSEM_SQI_NOR_AP_ID, 0, NULL, NULL);
	nor->prepare = sta_qspi_prepare;
	nor->unprepare = sta_qspi_unprepare;
#endif

	err = of_property_read_u32(child, "spi-max-frequency", &q->clk_rate);
	if (err < 0)
		goto disable_clk;

	err = sta_qspi_init(q);
	if (err)
		goto disable_clk;

	err = spi_nor_scan(nor, NULL, SPI_NOR_QUAD);
	if (err)
		goto disable_clk;

	/* Enable 4bytes address mode if necessary */
	if (nor->addr_width == 4) {
		u32 conf_r2;

		conf_r2 = qspi_readl(q, QSPI_CONF_REG2)
			  & ~(EXT_MEM_MODE | EXT_MEM_ADDR_8MSB_MSK);
		conf_r2 |= EXT_MEM_MODE | 0xFF;
		qspi_writel(q, QSPI_CONF_REG2, conf_r2);
	}

	err = mtd_device_register(mtd, NULL, 0);
	if (!err)
		goto exit;

disable_clk:
	clk_disable_unprepare(q->clk);
exit:
	of_node_put(child);

	return err;
}

static int sta_qspi_remove(struct platform_device *pdev)
{
	struct sta_qspi *q = platform_get_drvdata(pdev);

#if defined CONFIG_SPI_STA_QSPI_SHARED
	sta_hsem_lock_free(q->hsem_m3);
	sta_hsem_lock_free(q->hsem_ap);
#endif

	mtd_device_unregister(&q->nor.mtd);
	clk_disable_unprepare(q->clk);
	return 0;
}

static const struct of_device_id sta_qspi_dt_ids[] = {
	{ .compatible = "st,sta-qspi" },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, sta_qspi_dt_ids);

static struct platform_driver sta_qspi_driver = {
	.driver = {
		.name	= "sta_qspi",
		.of_match_table	= sta_qspi_dt_ids,
	},
	.probe		= sta_qspi_probe,
	.remove		= sta_qspi_remove,
};
module_platform_driver(sta_qspi_driver);

MODULE_AUTHOR("Philippe LANGLAIS <philippe.langlais@st.com>");
MODULE_DESCRIPTION("STA QSPI Controller driver");
MODULE_LICENSE("GPL v2");
