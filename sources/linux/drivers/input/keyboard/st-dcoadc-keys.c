/*
 * st-dcoadc-keys.c
 *
 * This driver is used to generate keypad input events from push buttons that
 * are connected to DC-Offset ADC channels.
 * The DCO-ADC is an analog to digital converter that can detect DC offset on
 * ADC input channels.
 *
 * Copyright (C) STMicroelectronics 2016
 * Author: Seraphin Bonnaffe <seraphin.bonnaffe@st.com> for STMicroelectronics
 * License terms:  GNU General Public License (GPL), version 2
 */
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

/* DCO-ADC Registers */
#define REG_IER			0x20
#define REG_ISR			0x24
#define REG_IEADCR		0x28
#define REG_ISADCR		0x2c
#define REG_DCODCR		0x104
#define REG_DCOIER		0x108
#define REG_DCOISR		0x10c
#define REG_ACC_UP_THRESHOLD(channel)	(0x110 + 4 * ((channel) - 1))
#define REG_ACC_LOW_THRESHOLD(channel)	(0x128 + 4 * ((channel) - 1))
#define REG_ACCUMULATED_VAL(channel)	(0x144 + 4 * ((channel) - 1))
#define REG_N_SAMPLES		0x140
#define REG_DCO_TRIG_EN		0x160

/* Bitmaks and constants */
#define IE0_IRQ_MASK			BIT(0)
#define ISADCR_CLEAR_VAL		1
#define MAX_DCO_CHANNELS		6
#define DCODCR_SW_RESET			BIT(16)
#define DCODCR_EN_ALL_CHAN		7
#define DCODCR_TSS0			BIT(3)
#define DCODCR_TSS1			BIT(4)
#define DCODCR_DIS_AUTO_RESTART		BIT(7)
#define DCO_ENABLE_TRIGGER		BIT(0)

#define DEF_UPPER_THLD	0xffffffff
#define DEF_LOWER_THLD	0x175
#define DEFAULT_SAMPLES_NR	0x10

/* Bit Operation */
#define SET_BIT(data, idx)		((data) |= BIT(idx))
#define CLR_BIT(data, idx)		((data) &= ~BIT(idx))
#define CHK_BIT(data, idx)		((data) & BIT(idx))

struct st_dcochannel {
	int num;
	bool in_use;
	bool active_high;
	unsigned int code;
	u32 up_thld;
	u32 low_thld;
};

struct st_dcoadc {
	void __iomem *base;
	struct input_dev *input;
	u32 nb_sample;
	int irq;
	struct clk *pclk;
	struct st_dcochannel channels[MAX_DCO_CHANNELS];
};

enum dcoadc_irq_type {
	DCO_IRQ_NONE		= 0,
	DCO_IRQ_IN_RANGE	= BIT(0),
	DCO_IRQ_OUT_RANGE	= BIT(1)
};

enum dcoadc_trigger_type {
	DCO_TRIGGER_LVL_LOW,
	DCO_TRIGGER_LVL_HIGH
};

static irqreturn_t dcoadc_isr(int irq, void *dev_id)
{
	struct st_dcoadc *dcoadc = dev_id;
	u32 reg, dcoisr, dcoier_new;
	int i;

	/* Check IRQ source = DCO */
	reg = readl(dcoadc->base + REG_ISR);
	if (reg != IE0_IRQ_MASK)
		return IRQ_HANDLED;

	/* Get DCO channel source of the IRQ */
	dcoisr = readl(dcoadc->base + REG_DCOISR);
	dcoier_new = readl(dcoadc->base + REG_DCOIER);
	for (i = 0; i < MAX_DCO_CHANNELS; i++) {
		if (CHK_BIT(dcoisr, i)) {
			CLR_BIT(dcoier_new, i);
			SET_BIT(dcoier_new, i + MAX_DCO_CHANNELS);
			input_report_key(dcoadc->input,
					 dcoadc->channels[i].code,
					 dcoadc->channels[i].active_high ?
					 1 : 0);
			input_sync(dcoadc->input);
		}
		if (CHK_BIT(dcoisr, i + MAX_DCO_CHANNELS)) {
			CLR_BIT(dcoier_new, i + MAX_DCO_CHANNELS);
			SET_BIT(dcoier_new, i);
			input_report_key(dcoadc->input,
					 dcoadc->channels[i].code,
					 dcoadc->channels[i].active_high ?
					 0 : 1);
			input_sync(dcoadc->input);
		}
	}

	/* Clear ISR, ISADCo, ISDCOR */
	writel(IE0_IRQ_MASK, dcoadc->base + REG_ISR);
	writel(ISADCR_CLEAR_VAL, dcoadc->base + REG_ISADCR);
	writel(dcoisr, dcoadc->base + REG_DCOISR);
	/* Set new interrupt mask */
	writel(dcoier_new, dcoadc->base + REG_DCOIER);

	return IRQ_HANDLED;
}

static int dcoadc_setup(struct st_dcoadc *dcoadc)
{
	u32 mask;
	int i;

	/* Reset the DCO controller */
	writel(DCODCR_SW_RESET, dcoadc->base + REG_DCODCR);
	msleep(20);

	/* Configure the number of samples */
	writel(dcoadc->nb_sample, dcoadc->base + REG_N_SAMPLES);

	/* Set Upper and Lower Thresholds */
	for (i = 0; i < MAX_DCO_CHANNELS; i++) {
		int channel = dcoadc->channels[i].num;

		if (!dcoadc->channels[i].in_use)
			continue;
		writel(dcoadc->channels[i].up_thld,
		       dcoadc->base + REG_ACC_UP_THRESHOLD(channel));
		writel(dcoadc->channels[i].low_thld,
		       dcoadc->base + REG_ACC_LOW_THRESHOLD(channel));
	}

	/* Clear the interrupt status register */
	writel(IE0_IRQ_MASK, dcoadc->base + REG_ISR);

	/* Enable DCO Interrupt source */
	writel(IE0_IRQ_MASK, dcoadc->base + REG_IER);

	/* Setup the initial interrupt mask */
	mask = 0;
	for (i = 0; i < MAX_DCO_CHANNELS; i++) {
		if (dcoadc->channels[i].active_high)
			SET_BIT(mask, i);
		else
			SET_BIT(mask, i + MAX_DCO_CHANNELS);
	}
	writel(mask, dcoadc->base + REG_DCOIER);

	/* Enable the DCO Trigger */
	writel(DCO_ENABLE_TRIGGER, dcoadc->base + REG_DCO_TRIG_EN);

	/* Configure DCO Controller */
	mask = DCODCR_EN_ALL_CHAN | DCODCR_TSS1 | DCODCR_DIS_AUTO_RESTART;
	writel(mask, dcoadc->base + REG_DCODCR);

	return 0;
};

static int st_adckeys_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *npc;
	struct st_dcoadc *dcoadc;
	struct resource *res;
	int i, chan, ret;
	u32 prop;

	dcoadc = devm_kzalloc(&pdev->dev, sizeof(*dcoadc), GFP_KERNEL);
	if (!dcoadc)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dcoadc->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(dcoadc->base))
		return -EINVAL;

	/* Parse Device Tree */
	dcoadc->irq = platform_get_irq(pdev, 0);
	if (dcoadc->irq < 0) {
		dev_err(&pdev->dev, "Failed to get irq for DCOADC\n");
		return -EINVAL;
	}

	dcoadc->pclk = devm_clk_get(&pdev->dev, "apb_pclk");
	if (IS_ERR(dcoadc->pclk)) {
		dev_err(&pdev->dev, "Failed to get clock for DCOADC\n");
		return -EINVAL;
	}

	dcoadc->nb_sample = DEFAULT_SAMPLES_NR;

	for_each_child_of_node(np, npc) {
		/* Channel number */
		if (of_property_read_u32(npc, "st,adc-channel", &chan)) {
			dev_err(&pdev->dev, "Key without channel\n");
			return -EINVAL;
		}
		if ((chan < 1) || (chan > MAX_DCO_CHANNELS)) {
			dev_err(&pdev->dev,
				"Channel number must be between 1 and %d\n",
				(int)MAX_DCO_CHANNELS);
			return -EINVAL;
		}
		dcoadc->channels[chan - 1].in_use = true;
		dcoadc->channels[chan - 1].num = chan;

		/* Active-low / active high */
		if (of_property_read_bool(npc, "st,active-high"))
			dcoadc->channels[chan - 1].active_high = true;
		else
			dcoadc->channels[chan - 1].active_high = false;

		/* Upper & Lower thresholds */
		if (of_property_read_u32_array(npc, "st,adc-thresholds",
					       &prop, 1))
			prop = DEF_UPPER_THLD;

		dcoadc->channels[chan - 1].up_thld = prop;

		if (of_property_read_u32_array(npc, "st,adc-thresholds",
					       &prop, 2))
			prop = DEF_LOWER_THLD;

		dcoadc->channels[chan - 1].low_thld = prop;

		/* Key code */
		if (of_property_read_u32(npc, "st,key-code",
					 &dcoadc->channels[chan - 1].code)) {
			dev_err(&pdev->dev, "Key without keycode, chan %d\n",
				dcoadc->channels[chan - 1].num);
			return -EINVAL;
		}
	}

	/* Request threaded IRQ */
	ret = devm_request_threaded_irq(&pdev->dev, dcoadc->irq, NULL,
					dcoadc_isr, IRQF_ONESHOT,
					"adc-keys", dcoadc);
	if (ret) {
		dev_err(&pdev->dev, "Failed to request irq: %d\n", ret);
		return ret;
	}

	/* Request clock */
	ret = clk_prepare_enable(dcoadc->pclk);
	if (ret) {
		dev_err(&pdev->dev, "Failed to request clock: %d\n", ret);
		return ret;
	}

	/* Configure the input device */
	dcoadc->input = devm_input_allocate_device(&pdev->dev);
	if (!dcoadc->input)
		return -ENOMEM;

	dcoadc->input->name = pdev->name;
	dcoadc->input->dev.parent = &pdev->dev;
	dcoadc->input->id.bustype = BUS_HOST;
	dcoadc->input->phys = "adc-keys/input0";

	__set_bit(EV_KEY, dcoadc->input->evbit);
	for (i = 0; i < MAX_DCO_CHANNELS; i++) {
		if (dcoadc->channels[i].in_use)
			input_set_capability(dcoadc->input, EV_KEY,
					     dcoadc->channels[i].code);
	}

	ret = input_register_device(dcoadc->input);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register input device\n");
		return ret;
	}

	dev_set_drvdata(&pdev->dev, dcoadc);

	/* Setup the ADC */
	dcoadc_setup(dcoadc);

	return 0;
}

static int st_adckeys_remove(struct platform_device *pdev)
{
	struct st_dcoadc *dcoadc = dev_get_drvdata(&pdev->dev);
	u32 ier;

	/* Disable DCO Interrupt source and clear ISR */
	ier = readl(dcoadc->base + REG_IER);
	writel(ier & ~IE0_IRQ_MASK, dcoadc->base + REG_IER);
	writel(IE0_IRQ_MASK, dcoadc->base + REG_ISR);

	/* Reset the DCO controller */
	writel(DCODCR_SW_RESET, dcoadc->base + REG_DCODCR);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int st_adckeys_suspend(struct device *dev)
{
	struct st_dcoadc *dcoadc = dev_get_drvdata(dev);
	u32 ier;

	/* Disable DCO Interrupt source and clear ISR */
	ier = readl(dcoadc->base + REG_IER);
	writel(ier & ~IE0_IRQ_MASK, dcoadc->base + REG_IER);
	writel(IE0_IRQ_MASK, dcoadc->base + REG_ISR);

	clk_disable_unprepare(dcoadc->pclk);

	dev_info(dev, "dcoadc-keys driver supended\n");
	return 0;
}

static int st_adckeys_resume(struct device *dev)
{
	struct st_dcoadc *dcoadc = dev_get_drvdata(dev);
	int ret;

	dev_info(dev, "dcoadc-keys driver resume\n");

	ret = clk_prepare_enable(dcoadc->pclk);
	if (ret) {
		dev_err(dev, "Failed to request clock: %d\n", ret);
		return ret;
	}

	dcoadc_setup(dcoadc);

	return 0;
}
#endif

static const struct dev_pm_ops st_adckeys_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(st_adckeys_suspend, st_adckeys_resume)
};

static const struct of_device_id st_adckeys_match[] = {
	{ .compatible = "st,dcoadc-keys", },
	{},
};
MODULE_DEVICE_TABLE(of, st_adckeys_match);

static struct platform_driver st_adckeys_driver = {
	.driver = {
		.name = "ST ADC Keys",
		.owner = THIS_MODULE,
		.of_match_table = st_adckeys_match,
		.pm = &st_adckeys_pm_ops,
	},
	.probe = st_adckeys_probe,
	.remove = st_adckeys_remove,
};

module_platform_driver(st_adckeys_driver);

MODULE_AUTHOR("Seraphin Bonnaffe <seraphin.bonnaffe@st.com>");
MODULE_DESCRIPTION("STMicroelectronics ADC Keys driver");
MODULE_LICENSE("GPL v2");
