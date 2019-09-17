/*
 * This driver supports the Synopsys Designware Ethernet QOS (Quality Of
 * Service) IP block.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/gpio.h>
#include <asm/io.h>

/* Core registers */

#define STMAC4_MAC_REGS_BASE 0x000
struct stmac4_mac_regs {
	uint32_t configuration;				/* 0x000 */
	uint32_t unused_004[(0x070 - 0x004) / 4];	/* 0x004 */
	uint32_t q0_tx_flow_ctrl;			/* 0x070 */
	uint32_t unused_070[(0x090 - 0x074) / 4];	/* 0x074 */
	uint32_t rx_flow_ctrl;				/* 0x090 */
	uint32_t unused_094;				/* 0x094 */
	uint32_t txq_prty_map0;				/* 0x098 */
	uint32_t unused_09c;				/* 0x09c */
	uint32_t rxq_ctrl0;				/* 0x0a0 */
	uint32_t unused_0a4;				/* 0x0a4 */
	uint32_t rxq_ctrl2;				/* 0x0a8 */
	uint32_t unused_0ac[(0x0dc - 0x0ac) / 4];	/* 0x0ac */
	uint32_t us_tic_counter;			/* 0x0dc */
	uint32_t unused_0e0[(0x11c - 0x0e0) / 4];	/* 0x0e0 */
	uint32_t hw_feature0;				/* 0x11c */
	uint32_t hw_feature1;				/* 0x120 */
	uint32_t hw_feature2;				/* 0x124 */
	uint32_t unused_128[(0x200 - 0x128) / 4];	/* 0x128 */
	uint32_t mdio_address;				/* 0x200 */
	uint32_t mdio_data;				/* 0x204 */
	uint32_t unused_208[(0x300 - 0x208) / 4];	/* 0x208 */
	uint32_t address0_high;				/* 0x300 */
	uint32_t address0_low;				/* 0x304 */
};

#define STMAC4_MAC_CONFIGURATION_GPSLCE			BIT(23)
#define STMAC4_MAC_CONFIGURATION_CST			BIT(21)
#define STMAC4_MAC_CONFIGURATION_ACS			BIT(20)
#define STMAC4_MAC_CONFIGURATION_WD			BIT(19)
#define STMAC4_MAC_CONFIGURATION_JD			BIT(17)
#define STMAC4_MAC_CONFIGURATION_JE			BIT(16)
#define STMAC4_MAC_CONFIGURATION_PS			BIT(15)
#define STMAC4_MAC_CONFIGURATION_FES			BIT(14)
#define STMAC4_MAC_CONFIGURATION_DM			BIT(13)
#define STMAC4_MAC_CONFIGURATION_TE			BIT(1)
#define STMAC4_MAC_CONFIGURATION_RE			BIT(0)

#define STMAC4_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT		16
#define STMAC4_MAC_Q0_TX_FLOW_CTRL_PT_MASK		0xffff
#define STMAC4_MAC_Q0_TX_FLOW_CTRL_TFE			BIT(1)

#define STMAC4_MAC_RX_FLOW_CTRL_RFE			BIT(0)

#define STMAC4_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT		0
#define STMAC4_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK		0xff

#define STMAC4_MAC_RXQ_CTRL0_RXQ0EN_SHIFT			0
#define STMAC4_MAC_RXQ_CTRL0_RXQ0EN_MASK			3
#define STMAC4_MAC_RXQ_CTRL0_RXQ0EN_NOT_ENABLED		0
#define STMAC4_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB		2

#define STMAC4_MAC_RXQ_CTRL2_PSRQ0_SHIFT			0
#define STMAC4_MAC_RXQ_CTRL2_PSRQ0_MASK			0xff

#define STMAC4_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT		6
#define STMAC4_MAC_HW_FEATURE1_TXFIFOSIZE_MASK		0x1f
#define STMAC4_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT		0
#define STMAC4_MAC_HW_FEATURE1_RXFIFOSIZE_MASK		0x1f

#define STMAC4_MAC_MDIO_ADDRESS_PA_SHIFT			21
#define STMAC4_MAC_MDIO_ADDRESS_RDA_SHIFT			16
#define STMAC4_MAC_MDIO_ADDRESS_CR_SHIFT			8
#define STMAC4_MAC_MDIO_ADDRESS_CR_20_35			2
#define STMAC4_MAC_MDIO_ADDRESS_SKAP			BIT(4)
#define STMAC4_MAC_MDIO_ADDRESS_GOC_SHIFT			2
#define STMAC4_MAC_MDIO_ADDRESS_GOC_READ			3
#define STMAC4_MAC_MDIO_ADDRESS_GOC_WRITE			1
#define STMAC4_MAC_MDIO_ADDRESS_C45E			BIT(1)
#define STMAC4_MAC_MDIO_ADDRESS_GB			BIT(0)

#define STMAC4_MAC_MDIO_DATA_GD_MASK			0xffff

#define STMAC4_MTL_REGS_BASE 0xd00
struct stmac4_mtl_regs {
	uint32_t txq0_operation_mode;			/* 0xd00 */
	uint32_t unused_d04;				/* 0xd04 */
	uint32_t txq0_debug;				/* 0xd08 */
	uint32_t unused_d0c[(0xd18 - 0xd0c) / 4];	/* 0xd0c */
	uint32_t txq0_quantum_weight;			/* 0xd18 */
	uint32_t unused_d1c[(0xd30 - 0xd1c) / 4];	/* 0xd1c */
	uint32_t rxq0_operation_mode;			/* 0xd30 */
	uint32_t unused_d34;				/* 0xd34 */
	uint32_t rxq0_debug;				/* 0xd38 */
};

#define STMAC4_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT		16
#define STMAC4_MTL_TXQ0_OPERATION_MODE_TQS_MASK		0x1ff
#define STMAC4_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT	2
#define STMAC4_MTL_TXQ0_OPERATION_MODE_TXQEN_MASK		3
#define STMAC4_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED	2
#define STMAC4_MTL_TXQ0_OPERATION_MODE_TSF		BIT(1)
#define STMAC4_MTL_TXQ0_OPERATION_MODE_FTQ		BIT(0)

#define STMAC4_MTL_TXQ0_DEBUG_TXQSTS			BIT(4)
#define STMAC4_MTL_TXQ0_DEBUG_TRCSTS_SHIFT		1
#define STMAC4_MTL_TXQ0_DEBUG_TRCSTS_MASK			3

#define STMAC4_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT		20
#define STMAC4_MTL_RXQ0_OPERATION_MODE_RQS_MASK		0x3ff
#define STMAC4_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT		14
#define STMAC4_MTL_RXQ0_OPERATION_MODE_RFD_MASK		0x3f
#define STMAC4_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT		8
#define STMAC4_MTL_RXQ0_OPERATION_MODE_RFA_MASK		0x3f
#define STMAC4_MTL_RXQ0_OPERATION_MODE_EHFC		BIT(7)
#define STMAC4_MTL_RXQ0_OPERATION_MODE_RSF		BIT(5)

#define STMAC4_MTL_RXQ0_DEBUG_PRXQ_SHIFT			16
#define STMAC4_MTL_RXQ0_DEBUG_PRXQ_MASK			0x7fff
#define STMAC4_MTL_RXQ0_DEBUG_RXQSTS_SHIFT		4
#define STMAC4_MTL_RXQ0_DEBUG_RXQSTS_MASK			3

#define STMAC4_DMA_REGS_BASE 0x1000
struct stmac4_dma_regs {
	uint32_t mode;					/* 0x1000 */
	uint32_t sysbus_mode;				/* 0x1004 */
	uint32_t unused_1008[(0x1100 - 0x1008) / 4];	/* 0x1008 */
	uint32_t ch0_control;				/* 0x1100 */
	uint32_t ch0_tx_control;			/* 0x1104 */
	uint32_t ch0_rx_control;			/* 0x1108 */
	uint32_t unused_110c;				/* 0x110c */
	uint32_t ch0_txdesc_list_haddress;		/* 0x1110 */
	uint32_t ch0_txdesc_list_address;		/* 0x1114 */
	uint32_t ch0_rxdesc_list_haddress;		/* 0x1118 */
	uint32_t ch0_rxdesc_list_address;		/* 0x111c */
	uint32_t ch0_txdesc_tail_pointer;		/* 0x1120 */
	uint32_t unused_1124;				/* 0x1124 */
	uint32_t ch0_rxdesc_tail_pointer;		/* 0x1128 */
	uint32_t ch0_txdesc_ring_length;		/* 0x112c */
	uint32_t ch0_rxdesc_ring_length;		/* 0x1130 */
};

#define STMAC4_DMA_MODE_SWR				BIT(0)

#define STMAC4_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT		16
#define STMAC4_DMA_SYSBUS_MODE_RD_OSR_LMT_MASK		0xf
#define STMAC4_DMA_SYSBUS_MODE_EAME			BIT(11)
#define STMAC4_DMA_SYSBUS_MODE_BLEN16			BIT(3)
#define STMAC4_DMA_SYSBUS_MODE_BLEN8			BIT(2)
#define STMAC4_DMA_SYSBUS_MODE_BLEN4			BIT(1)

#define STMAC4_DMA_CH0_CONTROL_PBLX8			BIT(16)

#define STMAC4_DMA_CH0_TX_CONTROL_TXPBL_SHIFT		16
#define STMAC4_DMA_CH0_TX_CONTROL_TXPBL_MASK		0x3f
#define STMAC4_DMA_CH0_TX_CONTROL_OSP			BIT(4)
#define STMAC4_DMA_CH0_TX_CONTROL_ST			BIT(0)

#define STMAC4_DMA_CH0_RX_CONTROL_RXPBL_SHIFT		16
#define STMAC4_DMA_CH0_RX_CONTROL_RXPBL_MASK		0x3f
#define STMAC4_DMA_CH0_RX_CONTROL_RBSZ_SHIFT		1
#define STMAC4_DMA_CH0_RX_CONTROL_RBSZ_MASK		0x3fff
#define STMAC4_DMA_CH0_RX_CONTROL_SR			BIT(0)

/* These registers are Tegra186-specific */
#define STMAC4_TEGRA186_REGS_BASE 0x8800
struct stmac4_tegra186_regs {
	uint32_t sdmemcomppadctrl;			/* 0x8800 */
	uint32_t auto_cal_config;			/* 0x8804 */
	uint32_t unused_8808;				/* 0x8808 */
	uint32_t auto_cal_status;			/* 0x880c */
};

#define STMAC4_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD	BIT(31)

#define STMAC4_AUTO_CAL_CONFIG_START			BIT(31)
#define STMAC4_AUTO_CAL_CONFIG_ENABLE			BIT(29)

#define STMAC4_AUTO_CAL_STATUS_ACTIVE			BIT(31)

/* Descriptors */

#define STMAC4_DESCRIPTOR_WORDS	4
#define STMAC4_DESCRIPTOR_SIZE	(STMAC4_DESCRIPTOR_WORDS * 4)
/* We assume ARCH_DMA_MINALIGN >= 16; 16 is the STMAC4 HW minimum */
#define STMAC4_DESCRIPTOR_ALIGN	ARCH_DMA_MINALIGN
#define STMAC4_DESCRIPTORS_TX	4
#define STMAC4_DESCRIPTORS_RX	4
#define STMAC4_DESCRIPTORS_NUM	(STMAC4_DESCRIPTORS_TX + STMAC4_DESCRIPTORS_RX)
#define STMAC4_DESCRIPTORS_SIZE	ALIGN(STMAC4_DESCRIPTORS_NUM * \
				      STMAC4_DESCRIPTOR_SIZE, ARCH_DMA_MINALIGN)
#define STMAC4_BUFFER_ALIGN	ARCH_DMA_MINALIGN
#define STMAC4_MAX_PACKET_SIZE	ALIGN(1568, ARCH_DMA_MINALIGN)
#define STMAC4_RX_BUFFER_SIZE	(STMAC4_DESCRIPTORS_RX * STMAC4_MAX_PACKET_SIZE)

/*
 * Warn if the cache-line size is larger than the descriptor size. In such
 * cases the driver will likely fail because the CPU needs to flush the cache
 * when requeuing RX buffers, therefore descriptors written by the hardware
 * may be discarded. Architectures with full IO coherence, such as x86, do not
 * experience this issue, and hence are excluded from this condition.
 *
 * This can be fixed by defining CONFIG_SYS_NONCACHED_MEMORY which will cause
 * the driver to allocate descriptors from a pool of non-cached memory.
 */
#if STMAC4_DESCRIPTOR_SIZE < ARCH_DMA_MINALIGN
#if !defined(CONFIG_SYS_NONCACHED_MEMORY) && \
	!defined(CONFIG_SYS_DCACHE_OFF) && !defined(CONFIG_X86)
#warning Cache line size is larger than descriptor size
#endif
#endif

struct stmac4_desc {
	u32 des0;
	u32 des1;
	u32 des2;
	u32 des3;
};

#define STMAC4_DESC3_OWN		BIT(31)
#define STMAC4_DESC3_FD		BIT(29)
#define STMAC4_DESC3_LD		BIT(28)
#define STMAC4_DESC3_BUF1V	BIT(24)

struct stmac4_config {
	bool reg_access_always_ok;
};

struct stmac4_priv {
	struct udevice *dev;
	const struct stmac4_config *config;
	fdt_addr_t regs;
	struct stmac4_mac_regs *mac_regs;
	struct stmac4_mtl_regs *mtl_regs;
	struct stmac4_dma_regs *dma_regs;
	struct reset_ctl reset_ctl;
	struct gpio_desc phy_reset_gpio;
	struct mii_dev *mii;
	struct phy_device *phy;
	void *descs;
	struct stmac4_desc *tx_descs;
	struct stmac4_desc *rx_descs;
	int tx_desc_idx, rx_desc_idx;
	void *tx_dma_buf;
	void *rx_dma_buf;
	void *rx_pkt;
	bool started;
	bool reg_access_ok;
};

/*
 * TX and RX descriptors are 16 bytes. This causes problems with the cache
 * maintenance on CPUs where the cache-line size exceeds the size of these
 * descriptors. What will happen is that when the driver receives a packet
 * it will be immediately requeued for the hardware to reuse. The CPU will
 * therefore need to flush the cache-line containing the descriptor, which
 * will cause all other descriptors in the same cache-line to be flushed
 * along with it. If one of those descriptors had been written to by the
 * device those changes (and the associated packet) will be lost.
 *
 * To work around this, we make use of non-cached memory if available. If
 * descriptors are mapped uncached there's no need to manually flush them
 * or invalidate them.
 *
 * Note that this only applies to descriptors. The packet data buffers do
 * not have the same constraints since they are 1536 bytes large, so they
 * are unlikely to share cache-lines.
 */
static void *stmac4_alloc_descs(unsigned int num)
{
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	return (void *)noncached_alloc(STMAC4_DESCRIPTORS_SIZE,
				       STMAC4_DESCRIPTOR_ALIGN);
#else
	return memalign(STMAC4_DESCRIPTOR_ALIGN, STMAC4_DESCRIPTORS_SIZE);
#endif
}

static void stmac4_free_descs(void *descs)
{
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	/* FIXME: noncached_alloc() has no opposite */
#else
	free(descs);
#endif
}

static void stmac4_inval_desc(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + STMAC4_DESCRIPTOR_SIZE,
				  ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
#endif
}

static void stmac4_flush_desc(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	flush_cache((unsigned long)desc, STMAC4_DESCRIPTOR_SIZE);
#endif
}

static void stmac4_inval_buffer(void *buf, size_t size)
{
	unsigned long start = (unsigned long)buf & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + size, ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void stmac4_flush_buffer(void *buf, size_t size)
{
	flush_cache((unsigned long)buf, size);
}

static int stmac4_mdio_wait_idle(struct stmac4_priv *stmac4)
{
	return wait_for_bit(__func__, &stmac4->mac_regs->mdio_address,
			    STMAC4_MAC_MDIO_ADDRESS_GB, false, 1000000, true);
}

static int stmac4_mdio_read(struct mii_dev *bus, int mdio_addr,
			    int mdio_devad, int mdio_reg)
{
	struct stmac4_priv *stmac4 = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d):\n", __func__, stmac4->dev,
	      mdio_addr, mdio_reg);

	ret = stmac4_mdio_wait_idle(stmac4);
	if (ret) {
		error("MDIO not idle at entry");
		return ret;
	}

	val = readl(&stmac4->mac_regs->mdio_address);
	val &= STMAC4_MAC_MDIO_ADDRESS_SKAP |
		STMAC4_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << STMAC4_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << STMAC4_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(STMAC4_MAC_MDIO_ADDRESS_CR_20_35 <<
		 STMAC4_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(STMAC4_MAC_MDIO_ADDRESS_GOC_READ <<
		 STMAC4_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		STMAC4_MAC_MDIO_ADDRESS_GB;
	writel(val, &stmac4->mac_regs->mdio_address);

	udelay(10);

	ret = stmac4_mdio_wait_idle(stmac4);
	if (ret) {
		error("MDIO read didn't complete");
		return ret;
	}

	val = readl(&stmac4->mac_regs->mdio_data);
	val &= STMAC4_MAC_MDIO_DATA_GD_MASK;

	debug("%s: val=%x\n", __func__, val);

	return val;
}

static int stmac4_mdio_write(struct mii_dev *bus, int mdio_addr,
			     int mdio_devad, int mdio_reg, u16 mdio_val)
{
	struct stmac4_priv *stmac4 = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d, val=%x):\n", __func__, stmac4->dev,
	      mdio_addr, mdio_reg, mdio_val);

	ret = stmac4_mdio_wait_idle(stmac4);
	if (ret) {
		error("MDIO not idle at entry");
		return ret;
	}

	writel(mdio_val, &stmac4->mac_regs->mdio_data);

	val = readl(&stmac4->mac_regs->mdio_address);
	val &= STMAC4_MAC_MDIO_ADDRESS_SKAP |
		STMAC4_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << STMAC4_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << STMAC4_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(STMAC4_MAC_MDIO_ADDRESS_CR_20_35 <<
		 STMAC4_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(STMAC4_MAC_MDIO_ADDRESS_GOC_WRITE <<
		 STMAC4_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		STMAC4_MAC_MDIO_ADDRESS_GB;
	writel(val, &stmac4->mac_regs->mdio_address);

	udelay(10);

	ret = stmac4_mdio_wait_idle(stmac4);
	if (ret) {
		error("MDIO read didn't complete");
		return ret;
	}

	return 0;
}

static int stmac4_set_full_duplex(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&stmac4->mac_regs->configuration,
		     STMAC4_MAC_CONFIGURATION_DM);

	return 0;
}

static int stmac4_set_half_duplex(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&stmac4->mac_regs->configuration,
		     STMAC4_MAC_CONFIGURATION_DM);

	/* WAR: Flush TX queue when switching to half-duplex */
	setbits_le32(&stmac4->mtl_regs->txq0_operation_mode,
		     STMAC4_MTL_TXQ0_OPERATION_MODE_FTQ);

	return 0;
}

static int stmac4_set_gmii_speed(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&stmac4->mac_regs->configuration,
		     STMAC4_MAC_CONFIGURATION_PS |
		     STMAC4_MAC_CONFIGURATION_FES);

	return 0;
}

static int stmac4_set_mii_speed_100(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&stmac4->mac_regs->configuration,
		     STMAC4_MAC_CONFIGURATION_PS |
		     STMAC4_MAC_CONFIGURATION_FES);

	return 0;
}

static int stmac4_set_mii_speed_10(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrsetbits_le32(&stmac4->mac_regs->configuration,
			STMAC4_MAC_CONFIGURATION_FES,
			STMAC4_MAC_CONFIGURATION_PS);

	return 0;
}

static int stmac4_adjust_link(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	if (stmac4->phy->duplex)
		ret = stmac4_set_full_duplex(dev);
	else
		ret = stmac4_set_half_duplex(dev);
	if (ret < 0) {
		error("stmac4_set_*_duplex() failed: %d", ret);
		return ret;
	}

	switch (stmac4->phy->speed) {
	case SPEED_1000:
		ret = stmac4_set_gmii_speed(dev);
		break;
	case SPEED_100:
		ret = stmac4_set_mii_speed_100(dev);
		break;
	case SPEED_10:
		ret = stmac4_set_mii_speed_10(dev);
		break;
	default:
		error("invalid speed %d", stmac4->phy->speed);
		return -EINVAL;
	}
	if (ret < 0) {
		error("stmac4_set_*mii_speed*() failed: %d", ret);
		return ret;
	}

	return 0;
}

static int stmac4_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	uint32_t val;

	debug("%s(dev=%p):\n", __func__, dev);
	if (!stmac4->config->reg_access_always_ok && !stmac4->reg_access_ok)
		return 0;

	/* Update the MAC address */
	val = (plat->enetaddr[5] << 8) |
		(plat->enetaddr[4]);
	writel(val, &stmac4->mac_regs->address0_high);
	val = (plat->enetaddr[3] << 24) |
		(plat->enetaddr[2] << 16) |
		(plat->enetaddr[1] << 8) |
		(plat->enetaddr[0]);
	writel(val, &stmac4->mac_regs->address0_low);

	return 0;
}

static int stmac4_start(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	int ret, i;
	u32 val, tx_fifo_sz, rx_fifo_sz, tqs, rqs, pbl;
	ulong last_rx_desc;

	debug("%s(dev=%p):\n", __func__, dev);

	stmac4->tx_desc_idx = 0;
	stmac4->rx_desc_idx = 0;

	stmac4->reg_access_ok = true;

	ret = wait_for_bit(__func__, &stmac4->dma_regs->mode,
			   STMAC4_DMA_MODE_SWR, false, 10, false);
	if (ret) {
		error("STMAC4_DMA_MODE_SWR stuck");
		return -1;
	}

	stmac4->phy = phy_connect(stmac4->mii, 0, dev, 0);
	if (!stmac4->phy) {
		error("phy_connect() failed");
		return -1;
	}
	ret = phy_config(stmac4->phy);
	if (ret < 0) {
		error("phy_config() failed: %d", ret);
		goto err_shutdown_phy;
	}
	ret = phy_startup(stmac4->phy);
	if (ret < 0) {
		error("phy_startup() failed: %d", ret);
		goto err_shutdown_phy;
	}

	if (!stmac4->phy->link) {
		error("No link");
		goto err_shutdown_phy;
	}

	ret = stmac4_adjust_link(dev);
	if (ret < 0) {
		error("stmac4_adjust_link() failed: %d", ret);
		goto err_shutdown_phy;
	}

	/* Configure MTL */

	/* Enable Store and Forward mode for TX */
	/* Program Tx operating mode */
	setbits_le32(&stmac4->mtl_regs->txq0_operation_mode,
		     STMAC4_MTL_TXQ0_OPERATION_MODE_TSF |
		     (STMAC4_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED <<
		      STMAC4_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT));

	/* Transmit Queue weight */
	writel(0x10, &stmac4->mtl_regs->txq0_quantum_weight);

	/* Enable Store and Forward mode for RX, since no jumbo frame */
	setbits_le32(&stmac4->mtl_regs->rxq0_operation_mode,
		     STMAC4_MTL_RXQ0_OPERATION_MODE_RSF);

	/* Transmit/Receive queue fifo size; use all RAM for 1 queue */
	val = readl(&stmac4->mac_regs->hw_feature1);
	tx_fifo_sz = (val >> STMAC4_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
		STMAC4_MAC_HW_FEATURE1_TXFIFOSIZE_MASK;
	rx_fifo_sz = (val >> STMAC4_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
		STMAC4_MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

	/*
	 * r/tx_fifo_sz is encoded as log2(n / 128). Undo that by shifting.
	 * r/tqs is encoded as (n / 256) - 1.
	 */
	tqs = (128 << tx_fifo_sz) / 256 - 1;
	rqs = (128 << rx_fifo_sz) / 256 - 1;

	clrsetbits_le32(&stmac4->mtl_regs->txq0_operation_mode,
			STMAC4_MTL_TXQ0_OPERATION_MODE_TQS_MASK <<
			STMAC4_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT,
			tqs << STMAC4_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
	clrsetbits_le32(&stmac4->mtl_regs->rxq0_operation_mode,
			STMAC4_MTL_RXQ0_OPERATION_MODE_RQS_MASK <<
			STMAC4_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT,
			rqs << STMAC4_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT);

	/* Flow control used only if each channel gets 4KB or more FIFO */
	if (rqs >= ((4096 / 256) - 1)) {
		u32 rfd, rfa;

		setbits_le32(&stmac4->mtl_regs->rxq0_operation_mode,
			     STMAC4_MTL_RXQ0_OPERATION_MODE_EHFC);

		/*
		 * Set Threshold for Activating Flow Contol space for min 2
		 * frames ie, (1500 * 1) = 1500 bytes.
		 *
		 * Set Threshold for Deactivating Flow Contol for space of
		 * min 1 frame (frame size 1500bytes) in receive fifo
		 */
		if (rqs == ((4096 / 256) - 1)) {
			/*
			 * This violates the above formula because of FIFO size
			 * limit therefore overflow may occur inspite of this.
			 */
			rfd = 0x3;	/* Full-3K */
			rfa = 0x1;	/* Full-1.5K */
		} else if (rqs == ((8192 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0xa;	/* Full-6K */
		} else if (rqs == ((16384 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x12;	/* Full-10K */
		} else {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x1E;	/* Full-16K */
		}

		clrsetbits_le32(&stmac4->mtl_regs->rxq0_operation_mode,
				(STMAC4_MTL_RXQ0_OPERATION_MODE_RFD_MASK <<
				 STMAC4_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(STMAC4_MTL_RXQ0_OPERATION_MODE_RFA_MASK <<
				 STMAC4_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT),
				(rfd <<
				 STMAC4_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(rfa <<
				 STMAC4_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT));
	}

	/* Configure MAC */

	clrsetbits_le32(&stmac4->mac_regs->rxq_ctrl0,
			STMAC4_MAC_RXQ_CTRL0_RXQ0EN_MASK <<
			STMAC4_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
			STMAC4_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB <<
			STMAC4_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

	/* Set TX flow control parameters */
	/* Set Pause Time */
	setbits_le32(&stmac4->mac_regs->q0_tx_flow_ctrl,
		     0xffff << STMAC4_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);
	/* Assign priority for TX flow control */
	clrbits_le32(&stmac4->mac_regs->txq_prty_map0,
		     STMAC4_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK <<
		     STMAC4_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT);
	/* Assign priority for RX flow control */
	clrbits_le32(&stmac4->mac_regs->rxq_ctrl2,
		     STMAC4_MAC_RXQ_CTRL2_PSRQ0_MASK <<
		     STMAC4_MAC_RXQ_CTRL2_PSRQ0_SHIFT);
	/* Enable flow control */
	setbits_le32(&stmac4->mac_regs->q0_tx_flow_ctrl,
		     STMAC4_MAC_Q0_TX_FLOW_CTRL_TFE);
	setbits_le32(&stmac4->mac_regs->rx_flow_ctrl,
		     STMAC4_MAC_RX_FLOW_CTRL_RFE);

	clrsetbits_le32(&stmac4->mac_regs->configuration,
			STMAC4_MAC_CONFIGURATION_GPSLCE |
			STMAC4_MAC_CONFIGURATION_WD |
			STMAC4_MAC_CONFIGURATION_JD |
			STMAC4_MAC_CONFIGURATION_JE,
			STMAC4_MAC_CONFIGURATION_CST |
			STMAC4_MAC_CONFIGURATION_ACS);

	stmac4_write_hwaddr(dev);

	/* Configure DMA */

	/* Enable OSP mode */
	setbits_le32(&stmac4->dma_regs->ch0_tx_control,
		     STMAC4_DMA_CH0_TX_CONTROL_OSP);

	/* RX buffer size. Must be a multiple of bus width */
	clrsetbits_le32(&stmac4->dma_regs->ch0_rx_control,
			STMAC4_DMA_CH0_RX_CONTROL_RBSZ_MASK <<
			STMAC4_DMA_CH0_RX_CONTROL_RBSZ_SHIFT,
			STMAC4_MAX_PACKET_SIZE <<
			STMAC4_DMA_CH0_RX_CONTROL_RBSZ_SHIFT);

	setbits_le32(&stmac4->dma_regs->ch0_control,
		     STMAC4_DMA_CH0_CONTROL_PBLX8);

	/*
	 * Burst length must be < 1/2 FIFO size.
	 * FIFO size in tqs is encoded as (n / 256) - 1.
	 * Each burst is n * 8 (PBLX8) * 16 (AXI width) == 128 bytes.
	 * Half of n * 256 is n * 128, so pbl == tqs, modulo the -1.
	 */
	pbl = tqs + 1;
	if (pbl > 32)
		pbl = 32;
	clrsetbits_le32(&stmac4->dma_regs->ch0_tx_control,
			STMAC4_DMA_CH0_TX_CONTROL_TXPBL_MASK <<
			STMAC4_DMA_CH0_TX_CONTROL_TXPBL_SHIFT,
			pbl << STMAC4_DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

	clrsetbits_le32(&stmac4->dma_regs->ch0_rx_control,
			STMAC4_DMA_CH0_RX_CONTROL_RXPBL_MASK <<
			STMAC4_DMA_CH0_RX_CONTROL_RXPBL_SHIFT,
			8 << STMAC4_DMA_CH0_RX_CONTROL_RXPBL_SHIFT);

	/* DMA performance configuration */
	val = (2 << STMAC4_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
		STMAC4_DMA_SYSBUS_MODE_EAME | STMAC4_DMA_SYSBUS_MODE_BLEN16 |
		STMAC4_DMA_SYSBUS_MODE_BLEN8 | STMAC4_DMA_SYSBUS_MODE_BLEN4;
	writel(val, &stmac4->dma_regs->sysbus_mode);

	/* Set up descriptors */

	memset(stmac4->descs, 0, STMAC4_DESCRIPTORS_SIZE);
	for (i = 0; i < STMAC4_DESCRIPTORS_RX; i++) {
		struct stmac4_desc *rx_desc = &(stmac4->rx_descs[i]);

		rx_desc->des0 = (u32)(ulong)(stmac4->rx_dma_buf +
					     (i * STMAC4_MAX_PACKET_SIZE));
		rx_desc->des3 |= STMAC4_DESC3_OWN | STMAC4_DESC3_BUF1V;
	}
	flush_cache((unsigned long)stmac4->descs, STMAC4_DESCRIPTORS_SIZE);

	writel(0, &stmac4->dma_regs->ch0_txdesc_list_haddress);
	writel((ulong)stmac4->tx_descs,
	       &stmac4->dma_regs->ch0_txdesc_list_address);
	writel(STMAC4_DESCRIPTORS_TX - 1,
	       &stmac4->dma_regs->ch0_txdesc_ring_length);

	writel(0, &stmac4->dma_regs->ch0_rxdesc_list_haddress);
	writel((ulong)stmac4->rx_descs,
	       &stmac4->dma_regs->ch0_rxdesc_list_address);
	writel(STMAC4_DESCRIPTORS_RX - 1,
	       &stmac4->dma_regs->ch0_rxdesc_ring_length);

	/* Enable everything */

	setbits_le32(&stmac4->mac_regs->configuration,
		     STMAC4_MAC_CONFIGURATION_TE | STMAC4_MAC_CONFIGURATION_RE);

	setbits_le32(&stmac4->dma_regs->ch0_tx_control,
		     STMAC4_DMA_CH0_TX_CONTROL_ST);
	setbits_le32(&stmac4->dma_regs->ch0_rx_control,
		     STMAC4_DMA_CH0_RX_CONTROL_SR);

	/* TX tail pointer not written until we need to TX a packet */
	/*
	 * Point RX tail pointer at last descriptor. Ideally, we'd point at the
	 * first descriptor, implying all descriptors were available. However,
	 * that's not distinguishable from none of the descriptors being
	 * available.
	 */
	last_rx_desc = (ulong)&(stmac4->rx_descs[(STMAC4_DESCRIPTORS_RX - 1)]);
	writel(last_rx_desc, &stmac4->dma_regs->ch0_rxdesc_tail_pointer);

	stmac4->started = true;

	printf("%s: OK\n", __func__);
	return 0;

err_shutdown_phy:
	phy_shutdown(stmac4->phy);
	stmac4->phy = NULL;
	error("FAILED: %d", ret);
	return ret;
}

void stmac4_stop(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	int i;

	debug("%s(dev=%p):\n", __func__, dev);

	if (!stmac4->started)
		return;
	stmac4->started = false;
	stmac4->reg_access_ok = false;

	/* Disable TX DMA */
	clrbits_le32(&stmac4->dma_regs->ch0_tx_control,
		     STMAC4_DMA_CH0_TX_CONTROL_ST);

	/* Wait for TX all packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&stmac4->mtl_regs->txq0_debug);
		u32 trcsts = (val >> STMAC4_MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
			STMAC4_MTL_TXQ0_DEBUG_TRCSTS_MASK;
		u32 txqsts = val & STMAC4_MTL_TXQ0_DEBUG_TXQSTS;

		if ((trcsts != 1) && (!txqsts))
			break;
	}

	/* Turn off MAC TX and RX */
	clrbits_le32(&stmac4->mac_regs->configuration,
		     STMAC4_MAC_CONFIGURATION_TE | STMAC4_MAC_CONFIGURATION_RE);

	/* Wait for all RX packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&stmac4->mtl_regs->rxq0_debug);
		u32 prxq = (val >> STMAC4_MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
			STMAC4_MTL_RXQ0_DEBUG_PRXQ_MASK;
		u32 rxqsts = (val >> STMAC4_MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
			STMAC4_MTL_RXQ0_DEBUG_RXQSTS_MASK;
		if ((!prxq) && (!rxqsts))
			break;
	}

	/* Turn off RX DMA */
	clrbits_le32(&stmac4->dma_regs->ch0_rx_control,
		     STMAC4_DMA_CH0_RX_CONTROL_SR);

	if (stmac4->phy) {
		phy_shutdown(stmac4->phy);
		stmac4->phy = NULL;
	}

	printf("%s: OK\n", __func__);
}

int stmac4_send(struct udevice *dev, void *packet, int length)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	struct stmac4_desc *tx_desc;
	int i;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);

	memcpy(stmac4->tx_dma_buf, packet, length);
	stmac4_flush_buffer(stmac4->tx_dma_buf, length);

	tx_desc = &(stmac4->tx_descs[stmac4->tx_desc_idx]);
	stmac4->tx_desc_idx++;
	stmac4->tx_desc_idx %= STMAC4_DESCRIPTORS_TX;

	tx_desc->des0 = (ulong)stmac4->tx_dma_buf;
	tx_desc->des1 = 0;
	tx_desc->des2 = length;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	tx_desc->des3 = STMAC4_DESC3_OWN | STMAC4_DESC3_FD |
				    STMAC4_DESC3_LD | length;
	stmac4_flush_desc(tx_desc);

	writel((ulong)(tx_desc + 1),
	       &stmac4->dma_regs->ch0_txdesc_tail_pointer);

	for (i = 0; i < 1000000; i++) {
		stmac4_inval_desc(tx_desc);
		if (!(readl(&tx_desc->des3) & STMAC4_DESC3_OWN))
			return 0;
		udelay(1);
	}

	printf("%s: TX timeout\n", __func__);

	return -ETIMEDOUT;
}

int stmac4_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	struct stmac4_desc *rx_desc;
	int length;

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);

	rx_desc = &(stmac4->rx_descs[stmac4->rx_desc_idx]);
	if (rx_desc->des3 & STMAC4_DESC3_OWN) {
		debug("%s: RX packet not available\n", __func__);
		return -EAGAIN;
	}

	*packetp = stmac4->rx_dma_buf +
		(stmac4->rx_desc_idx * STMAC4_MAX_PACKET_SIZE);
	length = rx_desc->des3 & 0x7fff;
	debug("%s: *packetp=%p, length=%d\n", __func__, *packetp, length);

	stmac4_inval_buffer(*packetp, length);
	return length;
}

int stmac4_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	uchar *packet_expected;
	struct stmac4_desc *rx_desc;

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	packet_expected = stmac4->rx_dma_buf +
		(stmac4->rx_desc_idx * STMAC4_MAX_PACKET_SIZE);
	if (packet != packet_expected) {
		printf("%s: Unexpected packet (expected %p)\n", __func__,
		      packet_expected);
		return -EINVAL;
	}

	rx_desc = &(stmac4->rx_descs[stmac4->rx_desc_idx]);
	rx_desc->des0 = (u32)(ulong)packet;
	rx_desc->des1 = 0;
	rx_desc->des2 = 0;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	rx_desc->des3 |= STMAC4_DESC3_OWN | STMAC4_DESC3_BUF1V;
	stmac4_flush_desc(rx_desc);

	writel((ulong)rx_desc, &stmac4->dma_regs->ch0_rxdesc_tail_pointer);

	stmac4->rx_desc_idx++;
	stmac4->rx_desc_idx %= STMAC4_DESCRIPTORS_RX;

	return 0;
}

static int stmac4_probe_resources_core(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	stmac4->descs = stmac4_alloc_descs(STMAC4_DESCRIPTORS_TX +
				       STMAC4_DESCRIPTORS_RX);
	if (!stmac4->descs) {
		printf("%s: stmac4_alloc_descs() failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	stmac4->tx_descs = (struct stmac4_desc *)stmac4->descs;
	stmac4->rx_descs = (stmac4->tx_descs + STMAC4_DESCRIPTORS_TX);
	debug("%s: tx_descs=%p, rx_descs=%p\n", __func__, stmac4->tx_descs,
	      stmac4->rx_descs);

	stmac4->tx_dma_buf = memalign(STMAC4_BUFFER_ALIGN,
			   STMAC4_MAX_PACKET_SIZE);
	if (!stmac4->tx_dma_buf) {
		printf("%s: memalign(tx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_descs;
	}
	debug("%s: tx_dma_buf=%p\n", __func__, stmac4->tx_dma_buf);

	/* Before passing buffers to GMAC we need to make sure zeros
	 * written there were flushed into RAM.
	 * Otherwise there's a chance to get some of them flushed in RAM when
	 * GMAC is already pushing data to RAM via DMA. This way incoming from
	 * GMAC data will be corrupted.
	 */
	flush_dcache_range((ulong)stmac4->tx_dma_buf,
			   (ulong)stmac4->tx_dma_buf + STMAC4_MAX_PACKET_SIZE);

	stmac4->rx_dma_buf = memalign(STMAC4_BUFFER_ALIGN,
			   STMAC4_RX_BUFFER_SIZE);
	if (!stmac4->rx_dma_buf) {
		printf("%s: memalign(rx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_dma_buf;
	}
	debug("%s: rx_dma_buf=%p\n", __func__, stmac4->rx_dma_buf);
	flush_dcache_range((ulong)stmac4->rx_dma_buf,
			   (ulong)stmac4->rx_dma_buf + STMAC4_RX_BUFFER_SIZE);

	stmac4->rx_pkt = malloc(STMAC4_MAX_PACKET_SIZE);
	if (!stmac4->rx_pkt) {
		printf("%s: malloc(rx_pkt) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_rx_dma_buf;
	}
	debug("%s: rx_pkt=%p\n", __func__, stmac4->rx_pkt);

	debug("%s: OK\n", __func__);
	return 0;

err_free_rx_dma_buf:
	free(stmac4->rx_dma_buf);
err_free_tx_dma_buf:
	free(stmac4->tx_dma_buf);
err_free_descs:
	stmac4_free_descs(stmac4->descs);
err:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int stmac4_remove_resources_core(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	free(stmac4->rx_pkt);
	free(stmac4->rx_dma_buf);
	free(stmac4->tx_dma_buf);
	stmac4_free_descs(stmac4->descs);

	debug("%s: OK\n", __func__);
	return 0;
}

static int stmac4_probe(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	stmac4->dev = dev;
	stmac4->config = (void *)dev_get_driver_data(dev);

	stmac4->regs = dev_get_addr(dev);
	if (stmac4->regs == FDT_ADDR_T_NONE) {
		error("dev_get_addr() failed");
		return -ENODEV;
	}

	stmac4->mac_regs = (void *)(stmac4->regs + STMAC4_MAC_REGS_BASE);
	stmac4->mtl_regs = (void *)(stmac4->regs + STMAC4_MTL_REGS_BASE);
	stmac4->dma_regs = (void *)(stmac4->regs + STMAC4_DMA_REGS_BASE);

	ret = stmac4_probe_resources_core(dev);
	if (ret < 0) {
		error("stmac4_probe_resources_core() failed: %d", ret);
		return ret;
	}

	stmac4->mii = mdio_alloc();
	if (!stmac4->mii) {
		error("mdio_alloc() failed");
		return -ENODEV;
	}
	stmac4->mii->read = stmac4_mdio_read;
	stmac4->mii->write = stmac4_mdio_write;
	stmac4->mii->priv = stmac4;
	strcpy(stmac4->mii->name, dev->name);

	ret = mdio_register(stmac4->mii);
	if (ret < 0) {
		error("mdio_register() failed: %d", ret);
		goto err_free_mdio;
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_mdio:
	mdio_free(stmac4->mii);
	stmac4_remove_resources_core(dev);

	printf("%s: returns %d\n", __func__, ret);
	return ret;
}

static int stmac4_remove(struct udevice *dev)
{
	struct stmac4_priv *stmac4 = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	mdio_unregister(stmac4->mii);
	mdio_free(stmac4->mii);
	stmac4_probe_resources_core(dev);

	debug("%s: OK\n", __func__);
	return 0;
}

static const struct eth_ops stmac4_ops = {
	.start = stmac4_start,
	.stop = stmac4_stop,
	.send = stmac4_send,
	.recv = stmac4_recv,
	.free_pkt = stmac4_free_pkt,
	.write_hwaddr = stmac4_write_hwaddr,
};

static const struct stmac4_config stmac4_config = {
	.reg_access_always_ok = false,
};

static const struct udevice_id stmac4_ids[] = {
	{
		.compatible = "st,sta-dwmac-4.10",
		.data = (ulong)&stmac4_config
	},
	{ }
};

U_BOOT_DRIVER(eth_stmac4) = {
	.name = "eth_stmac4",
	.id = UCLASS_ETH,
	.of_match = stmac4_ids,
	.probe = stmac4_probe,
	.remove = stmac4_remove,
	.ops = &stmac4_ops,
	.priv_auto_alloc_size = sizeof(struct stmac4_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
