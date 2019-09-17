/*
 * DMA Router driver for STA1xxx DMA signals routing over DMAC0 or DMAC1
 *
 * Copyright (C) 2017 Jean-Nicolas Graux
 *
 * Based on:
 *
 *  LPC18xx/43xx DMA MUX by:
 *   Copyright (C) 2015 Joachim Eastwood <manabian@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/mfd/syscon.h>
#include <linux/device.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_dma.h>
#include <linux/regmap.h>
#include <linux/spinlock.h>

/* DMA select configuration register offset for dmasel manipulation */
#define MSCREG_DMASEL 0x4

#define DMASEL_NA 0xFF
#define DMASEL_REQUESTS 32
#define STA_DMACS 2

struct sta_request_to_selbit {
	u8 dmac0;
	u8 dmac1;
};

static const struct sta_request_to_selbit sta1295_requests_to_selbits[] = {
	/* dmac0: EFT[0-3], dmac1: DPMailBox, FSRC, CD */
	[0 ... 3] = {DMASEL_NA, DMASEL_NA}, /* dmac0: EFT0-3,  dmac1: DPMbx, FSRC, CD */
	[4] = {DMASEL_NA, 14},   /* dmac0: EFT4,	 dmac1: MSP0 Tx */
	[5] = {0, 15},		 /* dmac0: SSP0 Rx,      dmac1: MSP1 Tx */
	[6] = {1, 16},		 /* dmac0: SSP1 Rx,      dmac1: MSP2 Tx */
	[7] = {DMASEL_NA, 17},	 /* dmac0: SSP2 Rx,      dmac1: MSP0 Rx */
	[8] = {3, 18},		 /* dmac0: SSP0 Tx,      dmac1: MSP1 Rx */
	[9] = {4, 19},		 /* dmac0: SSP1 Tx,      dmac1: MSP2 Rx */
	[10] = {DMASEL_NA, 0},	 /* dmac0: SSP2 Tx,      dmac1: SSP0 Rx */
	[11] = {DMASEL_NA, 1},	 /* dmac0: UART0 Rx,     dmac1: SSP1 Rx */
	[12] = {7, DMASEL_NA},	 /* dmac0: UART1 Rx,     dmac1: TSC FIFO */
	[13] = {8, 3},		 /* dmac0: UART2 Rx,     dmac1: SSP0 Tx */
	[14] = {9, 4},		 /* dmac0: UART3 Rx,     dmac1: SSP1 Tx */
	[15] = {DMASEL_NA, DMASEL_NA}, /* dmac0: UART0 Tx,     dmac1: SQI0 */
	[16] = {11, 22},	 /* dmac0: UART1 Tx,     dmac1: VIP REQ_EVEN */
	[17] = {12, 23},	 /* dmac0: UART2 Tx,     dmac1: VIP REQ_ODD */
	[18] = {13, 20},	 /* dmac0: UART3 Tx,     dmac1: SD/MMC2 */
	[19] = {14, 21},	 /* dmac0: MSP0 Tx,      dmac1: SD/MMC1 */
	[20] = {15, DMASEL_NA},	 /* dmac0: MSP1 Tx,      dmac1: UART0 Rx */
	[21] = {16, 7},		 /* dmac0: MSP2 Tx,      dmac1: UART1 Rx */
	[22] = {17, 8},		 /* dmac0: MSP0 Rx,      dmac1: UART2 Rx */
	[23] = {18, 9},		 /* dmac0: MSP1 Rx,      dmac1: UART3 Rx */
	[24] = {19, DMASEL_NA},	 /* dmac0: MSP2 Rx,      dmac1: UART0 Tx */
	[25] = {DMASEL_NA, 11},	 /* dmac0: I2C0,         dmac1: UART1 Tx */
	[26] = {DMASEL_NA, 12},	 /* dmac0: I2C1,         dmac1: UART2 Tx */
	[27] = {DMASEL_NA, 13},	 /* dmac0: I2C2,         dmac1: UART3 Tx */
	[28] = {20, DMASEL_NA},	 /* dmac0: SD/SDIO/MMC2, dmac1: FSMC FILL_FIFO */
	[29] = {21, DMASEL_NA},	 /* dmac0: SD/SDIO/MMC1, dmac1: EXT REQ (DREQ) */
	[30] = {22, DMASEL_NA},	 /* dmac0: VIP REQ_EVEN, dmac1: SQI1 */
	[31] = {23, DMASEL_NA},	 /* dmac0: VIP REQ_ODD,  dmac1: CD_SUB/SPDIF */
};

static const struct sta_request_to_selbit sta1385_requests_to_selbits[] = {
	/* dmac0: EFT[0-3], dmac1: DPMailBox, FSRC, CD */
	[0 ... 3] = {DMASEL_NA, DMASEL_NA}, /* dmac0: EFT0-3,  dmac1: DPMbx, FSRC, CD */
	[4] = {DMASEL_NA, 14},   /* dmac0: EFT4,	 dmac1: MSP0 Tx */
	[5] = {0, 15},		 /* dmac0: SSP0 Rx,      dmac1: MSP1 Tx */
	[6] = {1, 16},		 /* dmac0: SSP1 Rx,      dmac1: MSP2 Tx */
	[7] = {DMASEL_NA, 17},	 /* dmac0: SSP2 Rx,      dmac1: MSP0 Rx */
	[8] = {3, 18},		 /* dmac0: SSP0 Tx,      dmac1: MSP1 Rx */
	[9] = {4, 19},		 /* dmac0: SSP1 Tx,      dmac1: MSP2 Rx */
	[10] = {DMASEL_NA, 0},	 /* dmac0: SSP2 Tx,      dmac1: SSP0 Rx */
	[11] = {DMASEL_NA, 1},	 /* dmac0: UART0 Rx,     dmac1: SSP1 Rx */
	[12] = {7, DMASEL_NA},	 /* dmac0: UART1 Rx,     dmac1: TSC FIFO */
	[13] = {DMASEL_NA, 3},	 /* dmac0: WLANUART0 rx, dmac1: SSP0 Tx */
	[14] = {9, 4},		 /* dmac0: UART3 Rx,     dmac1: SSP1 Tx */
	[15] = {DMASEL_NA, DMASEL_NA}, /* dmac0: UART0 Tx,     dmac1: SQI0 */
	[16] = {11, DMASEL_NA},	 /* dmac0: UART1 Tx,     dmac1: reserved */
	[17] = {DMASEL_NA, DMASEL_NA}, /* dmac0: WLANUART0 Tx, dmac1: reserved */
	[18] = {13, DMASEL_NA},	 /* dmac0: UART3 Tx,     dmac1: reserved */
	[19] = {14, 21},	 /* dmac0: MSP0 Tx,      dmac1: SD/MMC1 */
	[20] = {15, DMASEL_NA},	 /* dmac0: MSP1 Tx,      dmac1: UART0 Rx */
	[21] = {16, 7},		 /* dmac0: MSP2 Tx,      dmac1: UART1 Rx */
	[22] = {17, 8},		 /* dmac0: MSP0 Rx,      dmac1: UART2 Rx */
	[23] = {18, DMASEL_NA},	 /* dmac0: MSP1 Rx,      dmac1: WLANUART1 Rx */
	[24] = {19, DMASEL_NA},	 /* dmac0: MSP2 Rx,      dmac1: UART0 Tx */
	[25] = {DMASEL_NA, 11},	 /* dmac0: I2C0,         dmac1: UART1 Tx */
	[26] = {DMASEL_NA, 12},	 /* dmac0: I2C1,         dmac1: UART2 Tx */
	[27] = {DMASEL_NA, DMASEL_NA}, /* dmac0: I2C2,   dmac1: WLANUART1 Tx */
	[28] = {DMASEL_NA, DMASEL_NA}, /* dmac0: reserved, dmac1: FSMC FILL_FIFO */
	[29] = {21, DMASEL_NA},	 /* dmac0: SD/SDIO/MMC1, dmac1: EXT REQ (DREQ) */
	[30] = {DMASEL_NA, DMASEL_NA}, /* dmac0: reserved, dmac1: reserved */
	[31] = {DMASEL_NA, DMASEL_NA}, /* dmac0: reserved, dmac1: reserved */
};

struct sta_dmasel {
	unsigned char busy;
	unsigned char val;
};

struct sta_dmasel_data {
	struct dma_router dmarouter;
	struct sta_dmasel *dmasels;
	const struct sta_request_to_selbit *requests_to_selbits;
	u32 dmacs;
	struct regmap *reg;
	/* to protect dmasel register against simultaneous programming */
	spinlock_t lock;
};

#ifdef CONFIG_REGMAP_SMC
static struct regmap_config regmap_cfg = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.val_format_endian = REGMAP_ENDIAN_LITTLE,
	.max_register = 0x7C,
	.fast_io = true,
};
#endif

static void *sta_dmasel_reserve(struct of_phandle_args *dma_spec,
				    struct of_dma *ofdma)
{
	struct platform_device *pdev = of_find_device_by_node(ofdma->of_node);
	struct sta_dmasel_data *dmasel_data = platform_get_drvdata(pdev);
	struct sta_dmasel *dmasel = NULL;
	unsigned long flags;
	unsigned int dmac, request, bit, val;

	if (dma_spec->args_count != 3) {
		dev_err(&pdev->dev, "invalid number of dmac args\n");
		return ERR_PTR(-EINVAL);
	}

	dmac = dma_spec->args[0];
	if (dmac >= STA_DMACS) {
		dev_err(&pdev->dev, "invalid dmac number: %d\n",
			dma_spec->args[0]);
		return ERR_PTR(-EINVAL);
	}

	request = dma_spec->args[1];
	if (request >= DMASEL_REQUESTS) {
		dev_err(&pdev->dev, "invalid request number: %d\n",
			request);
		return ERR_PTR(-EINVAL);
	}

	/* The of_node_put() will be done in the core for the node */
	dma_spec->np = of_parse_phandle(ofdma->of_node, "dmacs", dmac);
	if (!dma_spec->np) {
		dev_err(&pdev->dev, "can't get dma controller\n");
		return ERR_PTR(-EINVAL);
	}

	if (dmac) {
		bit = dmasel_data->requests_to_selbits[request].dmac1;
		val = BIT(bit);
	} else {
		bit = dmasel_data->requests_to_selbits[request].dmac0;
		val = 0;
	}

	if (bit != DMASEL_NA) {
		dmasel = &dmasel_data->dmasels[bit];

		spin_lock_irqsave(&dmasel_data->lock, flags);
		if (dmasel->busy && dmasel->val != dmac) {
			spin_unlock_irqrestore(&dmasel_data->lock, flags);
			dev_err(&pdev->dev,
				"dma request %u failed, dmasel %u is busy\n",
				request, bit);
			of_node_put(dma_spec->np);
			return ERR_PTR(-EBUSY);
		}
		/* if acquiring for the first time, configure it */
		if (!dmasel->busy) {
			regmap_update_bits(dmasel_data->reg, MSCREG_DMASEL,
					   BIT(bit), val);
		}
		dmasel->busy++;
		dmasel->val = dmac;
		spin_unlock_irqrestore(&dmasel_data->lock, flags);

		dev_dbg(&pdev->dev,
			"dmac%u: mapping dmasel %u to dma request %u\n",
			dmac, bit, request);
	}

	dma_spec->args[0] = dma_spec->args[1];
	dma_spec->args[1] = dma_spec->args[2];
	dma_spec->args_count = 2;

	return dmasel;
}

static void sta_dmasel_free(struct device *dev, void *route_data)
{
	struct sta_dmasel_data *dmasel_data = dev_get_drvdata(dev);
	struct sta_dmasel *dmasel = route_data;
	unsigned long flags;

	if (!dmasel)
		return;
	if (!dmasel->busy)
		return;

	spin_lock_irqsave(&dmasel_data->lock, flags);
	dmasel->busy--;
	spin_unlock_irqrestore(&dmasel_data->lock, flags);
}

static int sta_dmasel_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct sta_dmasel_data *dmasel_data;
#ifdef CONFIG_REGMAP_SMC
	struct device_node *syscon_np;
	struct resource res;
	int ret;
#endif

	dmasel_data = devm_kzalloc(&pdev->dev, sizeof(*dmasel_data),
				   GFP_KERNEL);
	if (!dmasel_data)
		return -ENOMEM;

#ifdef CONFIG_REGMAP_SMC
	syscon_np = of_get_parent(np);
	if (syscon_np) {
		ret = of_address_to_resource(syscon_np, 0, &res);
		if (ret)
			return ret;
		dmasel_data->reg = regmap_init_smc(NULL, &res, &regmap_cfg);
		of_node_put(syscon_np);
	} else {
		dev_err(&pdev->dev, "failed to get syscon np\n");
		return -ENODEV;
	}
#else
	dmasel_data->reg = syscon_regmap_lookup_by_compatible(
			   "st,sta1295-mscr-a7");
#endif
	if (IS_ERR(dmasel_data->reg)) {
		dev_err(&pdev->dev, "syscon lookup failed\n");
		return PTR_ERR(dmasel_data->reg);
	}

	/* sanity check on number of dmacs + nodes avaibility */
	if (of_count_phandle_with_args(np, "dmacs", NULL) !=
	    STA_DMACS) {
		dev_err(&pdev->dev, "invalid number of dma controllers\n");
		return -EINVAL;
	}

	dmasel_data->dmasels = devm_kcalloc(&pdev->dev, DMASEL_REQUESTS,
					    sizeof(struct sta_dmasel),
					    GFP_KERNEL);
	if (!dmasel_data->dmasels)
		return -ENOMEM;

	if (of_device_is_compatible(pdev->dev.of_node, "st,sta1385-dmasel"))
		dmasel_data->requests_to_selbits = sta1385_requests_to_selbits;
	else
		dmasel_data->requests_to_selbits = sta1295_requests_to_selbits;

	spin_lock_init(&dmasel_data->lock);
	platform_set_drvdata(pdev, dmasel_data);
	dmasel_data->dmarouter.dev = &pdev->dev;
	dmasel_data->dmarouter.route_free = sta_dmasel_free;

	return of_dma_router_register(np, sta_dmasel_reserve,
				      &dmasel_data->dmarouter);
}

#ifdef CONFIG_PM_SLEEP
/**
 * sta_dmasel_suspend - Suspend method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Put the device in a low power state.
 */
static int sta_dmasel_suspend(struct device *dev)
{
	return 0;
}

/**
 * sta_dmasel_resume - Resume method for the driver
 * @dev:	Address of the device structure
 * Returns 0 on success and error value on error
 *
 * Resume operation after suspend
 */
static int sta_dmasel_resume(struct device *dev)
{
	int i;
	struct sta_dmasel_data *dmasel_data = dev_get_drvdata(dev);
	struct sta_dmasel *dmasel;
	unsigned long flags;

	spin_lock_irqsave(&dmasel_data->lock, flags);
	for (i = 0; i < DMASEL_REQUESTS; i++) {
		dmasel = &dmasel_data->dmasels[i];
		if (!dmasel->busy)
			continue;
		regmap_update_bits(dmasel_data->reg, MSCREG_DMASEL,
				   BIT(i), dmasel->val ? BIT(i) : 0);
	}
	spin_unlock_irqrestore(&dmasel_data->lock, flags);
	return 0;
}
#endif /* ! CONFIG_PM_SLEEP */

static SIMPLE_DEV_PM_OPS(sta_dmasel_dev_pm_ops, sta_dmasel_suspend,
			 sta_dmasel_resume);

static const struct of_device_id sta_dmasel_match[] = {
	{ .compatible = "st,sta1295-dmasel" },
	{ .compatible = "st,sta1385-dmasel" },
	{},
};

static struct platform_driver sta_dmasel_driver = {
	.probe	= sta_dmasel_probe,
	.driver = {
		.name = "sta-dmasel",
		.of_match_table = sta_dmasel_match,
		.pm = &sta_dmasel_dev_pm_ops,
	},
};

static int __init sta_dmasel_init(void)
{
	return platform_driver_register(&sta_dmasel_driver);
}
arch_initcall(sta_dmasel_init);
