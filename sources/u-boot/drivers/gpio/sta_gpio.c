/*
 * (C) Copyright 2016 ST-microlectronics APG
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/hardware.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

#define STA_GPIOS_PER_BANK      32      /* Number of GPIOs per bank */

enum gpio_registers {
	GPIO_DAT =	0x00,		/* data register */
	GPIO_DATS =	0x04,		/* data set */
	GPIO_DATC =	0x08,		/* data clear */
	GPIO_PDIS =	0x0c,		/* pull disable */
	GPIO_DIR =	0x10,		/* direction */
	GPIO_DIRS =	0x14,		/* direction set */
	GPIO_DIRC =	0x18,		/* direction clear */
	GPIO_AFSLA =	0x20,		/* alternate function select A */
	GPIO_AFSLB =	0x24,		/* alternate function select B */
};

enum gpio_sta_af { /* alternate function settings */
	GPIO_GPIO = 0,
	GPIO_ALT_A,
	GPIO_ALT_B,
	GPIO_ALT_C
};

struct sta_gpios {
	u32 base;
};

static int sta_gpio_direction_input(struct udevice *dev, unsigned gpio)
{
	const struct sta_gpios *gpios = dev_get_priv(dev);

	writel((1 << gpio), gpios->base + GPIO_DIRC);

	return 0;
}

static int sta_gpio_get_value(struct udevice *dev, unsigned gpio)
{
	const struct sta_gpios *gpios = dev_get_priv(dev);

	return (readl(gpios->base + GPIO_DAT) & (1 << gpio)) ? 1 : 0;
}

static int sta_gpio_set_value(struct udevice *dev, unsigned gpio,
				  int value)
{
	const struct sta_gpios *gpios = dev_get_priv(dev);

	if (value)
		writel((1 << gpio), gpios->base + GPIO_DATS);
	else
		writel((1 << gpio), gpios->base + GPIO_DATC);

	return 0;
}

static int sta_gpio_direction_output(struct udevice *dev, unsigned gpio,
					 int value)
{
	const struct sta_gpios *gpios = dev_get_priv(dev);

	writel((1 << gpio), gpios->base + GPIO_DIRS);
	sta_gpio_set_value(dev, gpio, value);

	return 0;
}

static int sta_gpio_get_function(struct udevice *dev, unsigned gpio)
{
	const struct sta_gpios *gpios = dev_get_priv(dev);
	u32 bit = (1 << gpio);

	/* alternate function is 0..3, with one bit per register */
	switch (((readl(gpios->base + GPIO_AFSLA) & ~bit)  >> gpio) |
		((readl(gpios->base + GPIO_AFSLB) & ~bit) >> (gpio - 1))) {
	case GPIO_GPIO:
		if (readl(gpios->base + GPIO_DIR) & (1 << gpio))
			return GPIOF_OUTPUT;
		else
			return GPIOF_INPUT;

	case GPIO_ALT_A:
		return GPIOF_FUNC_A;
	case GPIO_ALT_B:
		return GPIOF_FUNC_B;
	case GPIO_ALT_C:
		return GPIOF_FUNC_C;

	default:
		return GPIOF_UNKNOWN;
	}
}

static int sta_gpio_set_af(struct udevice *dev,
		unsigned gpio, int function)
{
	u32 afunc, bfunc;
	const struct sta_gpios *gpios = dev_get_priv(dev);
	u32 bit = (1 << gpio);

	/* alternate function is 0..3, with one bit per register */
	afunc = readl(gpios->base + GPIO_AFSLA) & ~bit;
	bfunc = readl(gpios->base + GPIO_AFSLB) & ~bit;
	switch (function) {
	case GPIOF_GPIO:
	case GPIOF_FUNC_A:
	case GPIOF_FUNC_B:
	case GPIOF_FUNC_C:
		function -= GPIOF_GPIO;
		break;

	default:
		function = GPIO_GPIO;
		break;
	}

	if (function & 1)
		afunc |= bit;
	if (function & 2)
		bfunc |= bit;
	writel(afunc, gpios->base + GPIO_AFSLA);
	writel(bfunc, gpios->base + GPIO_AFSLB);

	return 0;
}

static int sta_gpio_probe(struct udevice *dev)
{
	struct sta_gpios *gpios = dev_get_priv(dev);
	struct sta_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->bank_name = plat->port_name;
	uc_priv->gpio_count = STA_GPIOS_PER_BANK;
	gpios->base = plat->base;

	return 0;
}

static int sta_gpio_bind(struct udevice *dev)
{
	struct sta_gpio_platdata *plat = dev->platdata;
	fdt_addr_t base_addr;

	if (plat)
		return 0;

	base_addr = dev_get_addr(dev);
	if (base_addr == FDT_ADDR_T_NONE)
		return -ENODEV;

	/*
	* TODO:
	* When every board is converted to driver model and DT is
	* supported, this can be done by auto-alloc feature, but
	* not using calloc to alloc memory for platdata.
	*/
	plat = calloc(1, sizeof(*plat));
	if (!plat)
		return -ENOMEM;

	plat->base = base_addr;
	plat->port_name = fdt_get_name(gd->fdt_blob, dev->of_offset, NULL);
	dev->platdata = plat;

	return 0;
}

static const struct dm_gpio_ops sta_gpio_ops = {
	.direction_input	= sta_gpio_direction_input,
	.direction_output	= sta_gpio_direction_output,
	.get_value		= sta_gpio_get_value,
	.set_value		= sta_gpio_set_value,
	.get_function		= sta_gpio_get_function,
	.set_af			= sta_gpio_set_af,
};

static const struct udevice_id sta_gpio_ids[] = {
	{ .compatible = "st,sta1xxx" },
	{ }
};

U_BOOT_DRIVER(gpio_sta) = {
	.name	= "gpio_sta",
	.id	= UCLASS_GPIO,
	.ops	= &sta_gpio_ops,
	.of_match = sta_gpio_ids,
	.probe = sta_gpio_probe,
	.bind	= sta_gpio_bind,
	.priv_auto_alloc_size = sizeof(struct sta_gpios),
};

