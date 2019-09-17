/*
 * dwmac-sta.c - STMicroelectronics DWMAC Glue for STA platforms
 *
 * Copyright (C) 2016 STMicroelectronics (R&D) Limited
 * Author: Seraphin Bonnaffe <seraphin.bonnaffe@st.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/errno.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_net.h>
#include <linux/regmap.h>
#include <linux/stmmac.h>
#include <linux/mfd/syscon.h>

#include "stmmac_platform.h"

/* PHY mode selection and csysreq */
#define MISC_REG10		0x2c
#define PHY_INTF_MASK		0x7
#define PHY_INTF_DWMAC1_SHIFT	4
#define PHY_INTF_RGMII		BIT(0)
#define PHY_INTF_RMII		BIT(2)
/* Interrupt enable register */
#define MISC_ETH_INTR_EN	0x34
#define ETH_INTR_MASK		0x7f
#define ETH_INTR_RESET_VAL	0
#define ETH_SBD_PMT		BIT(6)
#define ETH_SBD_PERCH_RX_2	BIT(5)
#define ETH_SBD_PERCH_TX_1	BIT(4)
#define ETH_SBD_PERCH_TX_0	BIT(3)
#define ETH_SBD_PERCH_TX	BIT(2)
#define ETH_SBD			BIT(1)
#define ETH_LPI			BIT(0)

/* Ethernet controller known base addresses */
#define ETH_DWMAC_0		0x500a0000
#define ETH_DWMAC_1		0x500c0000

struct dwmac_sta {
	struct regmap *misc_reg_base;
	unsigned int phy_shift;
};

int sta_dwmac_set_misc_regs(struct platform_device *pdev,
			    struct plat_stmmacenet_data *plat_dat)
{
	struct dwmac_sta *mac_sta = plat_dat->bsp_priv;
	int ret;

	/* PHY interface: default RGMII */
	if (plat_dat->interface == PHY_INTERFACE_MODE_RMII) {
		ret = regmap_update_bits(mac_sta->misc_reg_base, MISC_REG10,
					 PHY_INTF_MASK << mac_sta->phy_shift,
					 PHY_INTF_RMII << mac_sta->phy_shift);

	} else if ((plat_dat->interface == PHY_INTERFACE_MODE_RGMII) ||
		   (plat_dat->interface == PHY_INTERFACE_MODE_RGMII_ID) ||
		   (plat_dat->interface == PHY_INTERFACE_MODE_RGMII_RXID) ||
		   (plat_dat->interface == PHY_INTERFACE_MODE_RGMII_TXID)) {
		ret = regmap_update_bits(mac_sta->misc_reg_base, MISC_REG10,
					 PHY_INTF_MASK << mac_sta->phy_shift,
					 PHY_INTF_RGMII << mac_sta->phy_shift);
	} else {
		dev_err(&pdev->dev, "Only RMII and RGMII[-ID] mode supported\n");
		return -EINVAL;
	}

	if (ret) {
		dev_err(&pdev->dev, "Failed to set PHY interface");
		return ret;
	}

	/* Enable all Ethernet interrupts */
	ret = regmap_update_bits(mac_sta->misc_reg_base, MISC_ETH_INTR_EN,
				 ETH_INTR_MASK, ETH_INTR_MASK);
	if (ret) {
		dev_err(&pdev->dev, "Failed to update enable ETH interrupts\n");
		return ret;
	}

	return 0;
}

void sta_dwmac_suspend(struct platform_device *pdev, void *priv)
{
	/* Nothing to do */
}

void sta_dwmac_resume(struct platform_device *pdev, void *priv)
{
	struct net_device *ndev = dev_get_drvdata(&pdev->dev);
	struct stmmac_priv *spriv = netdev_priv(ndev);
	struct plat_stmmacenet_data *plat_dat = spriv->plat;

	sta_dwmac_set_misc_regs(pdev, plat_dat);
	spriv->mii->reset(spriv->mii);
}

static int sta_dwmac_probe(struct platform_device *pdev)
{
	struct plat_stmmacenet_data *plat_dat;
	struct stmmac_resources stmmac_res;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *node;
	struct dwmac_sta *mac_sta;
	struct resource *res;
	int ret;
#ifdef CONFIG_REGMAP_SMC
	struct regmap_config regmap_cfg = {
		.reg_bits = 32,
		.val_bits = 32,
		.reg_stride = 4,
		.val_format_endian = REGMAP_ENDIAN_LITTLE,
		.max_register = 0x7C,
		.fast_io = true,
	};
	struct resource regmap_res;
#endif

	ret = stmmac_get_platform_resources(pdev, &stmmac_res);
	if (ret)
		return ret;

	plat_dat = stmmac_probe_config_dt(pdev, &stmmac_res.mac);
	if (IS_ERR(plat_dat))
		return PTR_ERR(plat_dat);

	mac_sta = devm_kzalloc(&pdev->dev, sizeof(*mac_sta), GFP_KERNEL);
	if (!mac_sta)
		return -ENOMEM;

	/* Identify whether it is controller 0 or 1 and adapt bit shifts */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					   STMMAC_RESOURCE_NAME);
	if (!res)
		return -EINVAL;

	if (res->start == ETH_DWMAC_0) {
		mac_sta->phy_shift = 0;
	} else if (res->start == ETH_DWMAC_1) {
		mac_sta->phy_shift = PHY_INTF_DWMAC1_SHIFT;
	} else {
		dev_err(&pdev->dev, "Unrecognised dwmac-sta base address\n");
		return -EINVAL;
	}

	node = of_parse_phandle(np, "sta,misc-reg", 0);
	if (!node) {
		dev_err(&pdev->dev, "Missing property for syscon\n");
		return -ENODEV;
	}

#ifdef CONFIG_REGMAP_SMC
	ret = of_address_to_resource(node, 0, &regmap_res);
	if (ret) {
		of_node_put(node);
		return ret;
	}
	mac_sta->misc_reg_base = regmap_init_smc(NULL, &regmap_res,
						 &regmap_cfg);
#else
	mac_sta->misc_reg_base = syscon_node_to_regmap(node);
#endif
	of_node_put(node);

	if (IS_ERR(mac_sta->misc_reg_base))
		return PTR_ERR(mac_sta->misc_reg_base);

	plat_dat->bsp_priv = mac_sta;

	plat_dat->suspend = sta_dwmac_suspend;
	plat_dat->resume = sta_dwmac_resume;

	ret = sta_dwmac_set_misc_regs(pdev, plat_dat);
	if (ret)
		return ret;

	ret = stmmac_dvr_probe(&pdev->dev, plat_dat, &stmmac_res);

#ifdef CONFIG_REGMAP_SMC
	if (ret < 0)
		regmap_exit(mac_sta->misc_reg_base);
#endif


	return ret;
}

static int sta_dwmac_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct net_device *ndev = dev_get_drvdata(dev);
	struct stmmac_priv *spriv = netdev_priv(ndev);
	struct plat_stmmacenet_data *plat_dat = spriv->plat;
	struct dwmac_sta *mac_sta = plat_dat->bsp_priv;
	int ret;

	/* Disable all Ethernet interrupts */
	ret = regmap_update_bits(mac_sta->misc_reg_base, MISC_ETH_INTR_EN,
				 ETH_INTR_MASK, ETH_INTR_RESET_VAL);
	if (ret) {
		dev_err(dev, "Failed to update disable ETH interrupts\n");
		return ret;
	}

	return stmmac_pltfr_remove(pdev);
}

static const struct of_device_id sta_dwmac_match[] = {
	{ .compatible = "st,sta1295-dwmac", },
	{ }
};
MODULE_DEVICE_TABLE(of, sta_dwmac_match);

static struct platform_driver sta_dwmac_driver = {
	.probe  = sta_dwmac_probe,
	.remove = sta_dwmac_remove,
	.driver = {
		.name           = "sta-dwmac",
		.pm		= &stmmac_pltfr_pm_ops,
		.of_match_table = sta_dwmac_match,
	},
};
module_platform_driver(sta_dwmac_driver);

MODULE_AUTHOR("Seraphin Bonnaffe <seraphin.bonnaffe@st.com>");
MODULE_DESCRIPTION("STMicroelectronics DWMAC Glue for STA platforms");
MODULE_LICENSE("GPL");
