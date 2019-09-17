/*
 * PWM (Pulse Width Modulator) driver for STA SoC EFT controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Author: Maxime Gaudron <maxime.gaudron@st.com>
 */

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/resource.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>

#define DEFAULT_OCAR_VALUE 0x1000	/* set PWM value to TIMN_OCBR / 2 */
#define DEFAULT_OCBR_VALUE 0x1000

#define NUM_PWM 1
#define MAX_US_INT 0xffff

#define TIMN_ICAR 0x0000
#define TIMN_ICBR 0x0004
#define TIMN_OCAR 0x0008
#define TIMN_OCBR 0x000C
#define TIMN_CNTR 0x0010
#define TIMN_CR1  0x0014
#define TIMN_CR2  0x0018
#define TIMN_SR   0x001C

#define TIMN_CR1_EN	BIT(15)
#define TIMN_CR1_PWMI	BIT(14)
#define TIMN_CR1_FOLVB	BIT(11)
#define TIMN_CR1_FOLVA	BIT(10)
#define TIMN_CR1_OLVLB	BIT(9)
#define TIMN_CR1_OLVLA	BIT(8)
#define TIMN_CR1_OCBE	BIT(7)
#define TIMN_CR1_OCAE	BIT(6)
#define TIMN_CR1_OPM	BIT(5)
#define TIMN_CR1_PWM	BIT(4)
#define TIMN_CR1_IEDGB	BIT(3)
#define TIMN_CR1_IEDGA	BIT(2)
#define TIMN_CR1_EXEDG	BIT(1)
#define TIMN_CR1_ECKEN	BIT(0)

#define PWM_MODE (TIMN_CR1_OLVLB | TIMN_CR1_OCAE | TIMN_CR1_PWM)

struct sta_eft_chip {
	struct pwm_chip chip;
	void __iomem *base;
	struct clk *clk;
	u32 duty_cycle;
	u32 period;
};

static inline struct sta_eft_chip *to_sta_eft_chip(struct pwm_chip *chip)
{
	return container_of(chip, struct sta_eft_chip, chip);
}

static void sta_eft_set_pwm_mode(struct sta_eft_chip *chip)
{
	writel(PWM_MODE, chip->base + TIMN_CR1);
}

static void sta_eft_disable(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct sta_eft_chip *ec = to_sta_eft_chip(chip);
	u32 cr1;

	cr1 = readl(ec->base + TIMN_CR1);
	writel(cr1 & ~TIMN_CR1_EN, ec->base + TIMN_CR1);

	clk_disable(ec->clk);
}

static int sta_eft_enable(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct sta_eft_chip *ec = to_sta_eft_chip(chip);
	int rc;
	u32 cr1;

	rc = clk_enable(ec->clk);
	if (rc)
		return rc;

	cr1 = readl(ec->base + TIMN_CR1);
	writel(cr1 | TIMN_CR1_EN, ec->base + TIMN_CR1);

	return 0;
}

static void sta_eft_get_state(struct pwm_chip *chip, struct pwm_device *pwm,
			      struct pwm_state *state)
{
	struct sta_eft_chip *ec = to_sta_eft_chip(chip);

	if (!ec)
		return;

	state->period = readl(ec->base + TIMN_OCBR);
	state->duty_cycle = readl(ec->base + TIMN_OCAR);
	state->polarity = PWM_POLARITY_NORMAL;
	state->enabled = readl(ec->base + TIMN_CR1) & TIMN_CR1_EN;
}

static int sta_eft_config(struct pwm_chip *chip, struct pwm_device *pwm,
			  int duty_ns, int period_ns)
{
	struct sta_eft_chip *ec = to_sta_eft_chip(chip);

	if (period_ns <= 0 || period_ns > MAX_US_INT) {
		dev_err(chip->dev, "period value out of bounds\n");
		return -EINVAL;
	}

	if (period_ns != ec->period) {
		writel(period_ns, ec->base + TIMN_OCBR);
		ec->period = period_ns;
	}

	if (duty_ns != ec->duty_cycle) {
		writel(duty_ns, ec->base + TIMN_OCAR);
		ec->duty_cycle = duty_ns;
	}

	return 0;
}

static int sta_eft_apply(struct pwm_chip *chip, struct pwm_device *pwm,
			 struct pwm_state *state)
{
	int ret;

	if (state->enabled)
		sta_eft_enable(chip, pwm);
	else
		sta_eft_disable(chip, pwm);

	ret = sta_eft_config(chip, pwm, state->duty_cycle, state->period);

	return ret;
}

static const struct pwm_ops eft_pwm_ops = {
	.apply = sta_eft_apply,
	.get_state = sta_eft_get_state,
	.owner = THIS_MODULE
};

static int sta_eft_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct pwm_state state = {
		.period		= DEFAULT_OCBR_VALUE,
		.duty_cycle	= DEFAULT_OCAR_VALUE,
		.polarity	= PWM_POLARITY_NORMAL,
		.enabled	= false,
	};
	struct sta_eft_chip *chip;
	int ret;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "eft_regs");
	chip->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(chip->base))
		return PTR_ERR(chip->base);

	chip->chip.dev = &pdev->dev;
	chip->chip.ops = &eft_pwm_ops;
	chip->chip.npwm = NUM_PWM;
	chip->chip.base = -1;

	platform_set_drvdata(pdev, chip);

	sta_eft_set_pwm_mode(chip);
	sta_eft_apply(&chip->chip, NULL, &state);

	ret = pwmchip_add(&chip->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register PWM chip:%d\n", ret);
		return ret;
	}

	dev_info(&pdev->dev, "sta eft PWM registered\n");

	return 0;
}

static int sta_eft_remove(struct platform_device *pdev)
{
	struct sta_eft_chip *chip = platform_get_drvdata(pdev);

	return pwmchip_remove(&chip->chip);
}

static const struct of_device_id sta_eft_dt_ids[] = {
	{
		.compatible = "st,sta-eft"
	}, { /*sentinel */}
};
MODULE_DEVICE_TABLE(of, sta_eft_dt_ids);

static struct platform_driver sta_eft_driver = {
	.probe = sta_eft_probe,
	.remove = sta_eft_remove,
	.driver = {
		.name = "sta-eft",
		.of_match_table = sta_eft_dt_ids
	},
};
module_platform_driver(sta_eft_driver);

MODULE_AUTHOR("Maxime Gaudron <maxime.gaudron@st.com>");
MODULE_DESCRIPTION("STA EFT driver");
MODULE_LICENSE("GPL v2");
