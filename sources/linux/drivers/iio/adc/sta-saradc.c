/*
 * STMicroelectronics SAR ADC driver.
 *
 * Copyright (C) 2017 STMicroelectronics
 * Written by:
 *	Gian Antonio Sampietro (gianantonio.sampietro@st.com)
 *	Jean-Nicolas graux (jean-nicolas.graux@st.com)
 *	Nicolas Guion (nicolas.guion@st.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

/* ADC registers */
#define SARADC_VERSION	0x00		/* Version ID Register	         */
#define SARADC_IER	0x20		/* Interrupt Enable Register     */
#define SARADC_ISR	0x24		/* Interrupt Status Register     */
#define SARADC_IEADCR	0x28		/* Interrupt Enable ADC Register */
#define SARADC_ISADCR	0x2C		/* Interrupt Status ADC Register */
#define SARADC_ADCCTRL	0x40		/* ADC Control Register 1        */
#define SARADC_ADCCAPT	0x44		/* ADC Capture Register          */
#define SARADC_ADCDATA0	0x80		/* ADC Data Register 0           */
#define SARADC_ADCDATA1	0x84		/* ADC Data Register 1           */
#define SARADC_ADCDATA2	0x88		/* ADC Data Register 2           */
#define SARADC_ADCDATA3	0x8C		/* ADC Data Register 3           */
#define SARADC_ADCDATA4	0x90		/* ADC Data Register 4           */
#define SARADC_ADCDATA5	0x94		/* ADC Data Register 5           */
#define SARADC_ADCDATA6	0x98		/* ADC Data Register 6           */
#define SARADC_ADCDATA7	0x9C		/* ADC Data Register 7           */
#define SARADC_ADCDATA8	0xA0		/* ADC Data Register 8           */
#define SARADC_ADCDATA9	0xA4		/* ADC Data Register 9           */

#define TSC_TSCCTRL	0xC0		/* Touch Screen Control Register */
#define TSC_TSCCFG	0xC4		/* Touch Screen Config. Register */
#define TSC_TSCWDWTRX	0xC8		/* Tracking Window Top-Right X   */
#define TSC_TSCWDWTRY	0xCC		/* Tracking Window Top-Right Y   */
#define TSC_TSCWDWBLX	0xD0		/* Tracking Window Bottom-Left X */
#define TSC_TSCWDWBLY	0xD4		/* Tracking Window Bottom-Left Y */
#define TSC_TSCFIFOTH	0xD8		/* Touch Screen FIFO Threshold   */
#define TSC_TSCFIFOCS	0xDC		/* TS FIFO Control and Status    */
#define TSC_TSCFIFOSZ	0xE0		/* TS Current FIFO Size Register */
#define TSC_TSCDATAX	0xE4		/* Touch Screen X-Coord Register */
#define TSC_TSCDATAY	0xE8		/* Touch Screen Y-Coord Register */
#define TSC_TSCDATAZ	0xEC		/* Touch Screen Z-Coord Register */
#define TSC_TSCDATAXYZ	0xF0		/* TS X, Y, Z-Coord Register     */
#define TSC_TSCFRACTZ	0xF4		/* TS Z Value Fraction Control   */
#define TSC_IDRIVE	0xFC		/* Touch Screen I Drive Register */

/* SARADC Control Register 1 bits */
#define ADCCTRL_ADCEN	BIT(0)		/* ADC Controller State          */
#define ADCCTRL_ADCEN_IDLE 0		/* ADC Controller Idle state     */
#define ADCCTRL_REFSEL	BIT(1)		/* ADC Reference Clock           */
#define ADCCTRL_REFSEL_EXT 0x0          /* ADC External Reference Enable */
#define ADCCTRL_FREQ	GENMASK(3, 2)	/* ADC Clock Frequency		 */
#define ADCCTRL_FREQ_HCLK26 0		/* ADC_CLK = HCLK divided by 26  */

/* SARADC Interrupt Enable Register bits */
#define IRQ_TOUCH	 BIT(0)		/* Touch Detect Interrupt        */
#define IRQ_FIFOTH	BIT(1)		/* FIFO Threshold Interrupt      */
#define IRQ_FIFOOVR	BIT(2)		/* FIFO Overflow Interrupt       */
#define IRQ_FIFOF	BIT(3)		/* FIFO Full Interrupt           */
#define IRQ_FIFOE	BIT(4)		/* FIFO Empty Interrupt          */
#define IRQ_ADC		BIT(6)		/* ADC Interrupt                 */

/* TSC Control Register bits */
#define TSCCTRL_EN	BIT(0)		/* Touch Screen Enable           */
#define TSCCTRL_MXYZ	0		/* Mode: sample X, Y, Z values   */
#define TSCCTRL_MXY	BIT(1)		/* Mode: sample X, Y values      */
#define TSCCTRL_MX	BIT(2)		/* Mode: sample X values         */
#define TSCCTRL_MY	GENMASK(2, 1)	/* Mode: sample Y values         */
#define TSCCTRL_MZ	BIT(3)		/* Mode: sample Z values         */
#define TSCCTRL_WT(x)	((x) << 4)	/* Window Tracking Index         */
#define TSCCTRL_TOUCHED BIT(7)		/* Touch Detected on Panel       */

/* TSC Configuration Register bits */
#define TSCCFG_VST(x)	((x) << 0)	/* Panel Voltage Settling Time   */
#define TSCCFG_TDD(x)	((x) << 3)	/* Touch Detect Delay            */
#define TSCCFG_AVGS(x)	((x) << 6)	/* Samples Averaging for Event   */

/* TSC FIFO Control and Status bits */
#define TSCFIFOCS_FRES	BIT(0)		/* Touch Screen FIFO Reset       */
#define TSCFIFOCS_FTHT	BIT(4)		/* FIFO Threshold Trigger Flag   */
#define TSCFIFOCS_FIFOE	BIT(5)		/* FIFO Empty Status Flag        */
#define TSCFIFOCS_FIFOF	BIT(6)		/* FIFO Full Status Flag         */
#define TSCFIFOCS_FIFOO	BIT(7)		/* FIFO Overflow Status Flag     */

/* TSC Z value Fraction Control bits */
#define TSCFRACTZ_F0W8 0		/* Z Data: Fractional 0, Whole 8 */
#define TSCFRACTZ_F1W7 1		/* Z Data: Fractional 1, Whole 7 */
#define TSCFRACTZ_F2W6 2		/* Z Data: Fractional 2, Whole 6 */
#define TSCFRACTZ_F3W5 3		/* Z Data: Fractional 3, Whole 5 */
#define TSCFRACTZ_F4W4 4		/* Z Data: Fractional 4, Whole 4 */
#define TSCFRACTZ_F5W3 5		/* Z Data: Fractional 5, Whole 3 */
#define TSCFRACTZ_F6W2 6		/* Z Data: Fractional 6, Whole 2 */
#define TSCFRACTZ_F7W1 7		/* Z Data: Fractional 7, Whole 1 */

#define MAX_12BIT ((1 << 12) - 1)
#define MAX_CHANNEL 10

#define VERSION_ID 1

enum saradc_tsc_averaging {
	TSC_AVG_SAMPLES_1 = 0,
	TSC_AVG_SAMPLES_2,
	TSC_AVG_SAMPLES_4,
	TSC_AVG_SAMPLES_8,
};

enum saradc_tsc_tracking {
	TSC_TRK_INDEX_0 = 0,
	TSC_TRK_INDEX_4,
	TSC_TRK_INDEX_8,
	TSC_TRK_INDEX_16,
	TSC_TRK_INDEX_32,
	TSC_TRK_INDEX_64,
	TSC_TRK_INDEX_92,
	TSC_TRK_INDEX_127,
};

enum saradc_tsc_voltage_settling {
	TSC_VST_TIME_10US = 0,
	TSC_VST_TIME_100US,
	TSC_VST_TIME_500US,
	TSC_VST_TIME_1MS,
	TSC_VST_TIME_5MS,
	TSC_VST_TIME_10MS,
	TSC_VST_TIME_50MS,
	TSC_VST_TIME_100MS,
};

enum saradc_tsc_touch_detect {
	TSC_TDD_TIME_10US = 0,
	TSC_TDD_TIME_50US,
	TSC_TDD_TIME_100US,
	TSC_TDD_TIME_500US,
	TSC_TDD_TIME_1MS,
	TSC_TDD_TIME_5MS,
	TSC_TDD_TIME_10MS,
	TSC_TDD_TIME_50MS,
};

/**
 * struct saradc_platdata: Touch screen configuration data
 * @tsc_enable: If set to false, touchscreen functionality is disabled
 * and ADC channels [0-3] become free for general purpose usage.
 * @averaging_samples: Number of samples averaged for each touch event
 * @tracking_index: Events are reported only if the sum of orizontal
 * movement and vertical movement is greater than this tracking index
 * @volt_settling_time: Delay from the time when the touch screen analog
 * buffers are enabled to the time when the sampling starts
 * @touch_detect_delay: Delay from the time when the touch is detected
 * to the time when an input event is generated
 * @samples_per_second: Set to 0 for max rate supported by hardware
 * @swap_xy: Swap x and y axes
 * @flip_x: Flip x axis
 * @flip_y: Flip y axis
 */
struct saradc_platdata {
	bool tsc_enable;
	u32 averaging_samples;
	u32 tracking_index;
	u32 volt_settling_time;
	u32 touch_detect_delay;
	unsigned int samples_per_second;
	bool swap_xy;
	bool flip_x;
	bool flip_y;
};

/* Touch screen controller status */
struct saradc_tsc {
	struct input_dev *input;
	struct delayed_work touch_detect;
	bool swap_xy;
	bool flip_x;
	bool flip_y;
	u16 x_last;
	u16 y_last;
	u16 z_last;
	int reader_interval;
};

/* SARADC controller */
struct saradc {
	struct device *dev;
	void __iomem *base;
	unsigned int irq;
	struct saradc_platdata *plat;
	struct saradc_tsc tsc;
	struct completion completion;
	struct clk *pclk;
	u32 adcctrl;
};

#define ADC_CHANNEL(chan, name) {			\
	.type = IIO_VOLTAGE,				\
	.indexed = 1,					\
	.channel = chan,				\
	.address = chan,				\
	.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),	\
	.datasheet_name = name,				\
}

static const struct iio_chan_spec saradc_iio_channels[] = {
	ADC_CHANNEL(0, "adc0"),
	ADC_CHANNEL(1, "adc1"),
	ADC_CHANNEL(2, "adc2"),
	ADC_CHANNEL(3, "adc3"),
	ADC_CHANNEL(4, "adc4"),
	ADC_CHANNEL(5, "adc5"),
	ADC_CHANNEL(6, "adc6"),
	ADC_CHANNEL(7, "adc7"),
	ADC_CHANNEL(8, "adc8"),
	ADC_CHANNEL(9, "adc9"),
};

static int saradc_remove_devices(struct device *dev, void *c)
{
	struct platform_device *pdev = to_platform_device(dev);

	platform_device_unregister(pdev);

	return 0;
}

static inline void saradc_set_bits(u32 mask, void __iomem *reg)
{
	writel(readl(reg) | mask, reg);
}

static inline void saradc_clear_bits(u32 mask, void __iomem *reg)
{
	writel(readl(reg) & ~mask, reg);
}

static inline int saradc_tsc_fifo_empty(struct saradc *saradc)
{
	return !!(readl(saradc->base + TSC_TSCFIFOCS) & TSCFIFOCS_FIFOE);
}

static inline int saradc_tsc_pen_down(struct saradc *saradc)
{
	return !!(readl(saradc->base + TSC_TSCCTRL) & TSCCTRL_TOUCHED);
}

static void saradc_tsc_read_data(struct saradc *saradc)
{
	struct saradc_tsc *tsc = &saradc->tsc;

	tsc->x_last = (u16)readl(saradc->base + TSC_TSCDATAX);
	tsc->y_last = (u16)readl(saradc->base + TSC_TSCDATAY);
	tsc->z_last = (u16)readl(saradc->base + TSC_TSCDATAZ);

	if (tsc->flip_x)
		tsc->x_last = MAX_12BIT - tsc->x_last;

	if (tsc->flip_y)
		tsc->y_last = MAX_12BIT - tsc->y_last;

	if (tsc->swap_xy)
		swap(tsc->x_last, tsc->y_last);
}

static inline void saradc_tsc_clear_penirq(struct saradc *saradc)
{
	writel(IRQ_TOUCH, saradc->base + SARADC_ISR);
}

static inline int saradc_tsc_get_penirq(struct saradc *saradc)
{
	return readl(saradc->base + SARADC_ISR) & IRQ_TOUCH;
}

static inline int saradc_tsc_get_fifoirq(struct saradc *saradc)
{
	return readl(saradc->base + SARADC_ISR) & (IRQ_FIFOTH | IRQ_FIFOOVR |
		IRQ_FIFOF | IRQ_FIFOE | IRQ_ADC);
}

static inline void saradc_tsc_clear_fifoirq(struct saradc *saradc)
{
	writel(IRQ_FIFOTH | IRQ_FIFOOVR | IRQ_FIFOF | IRQ_FIFOE | IRQ_ADC,
	       saradc->base + SARADC_ISR);
}

static inline void saradc_tsc_disable(struct saradc *saradc)
{
	saradc_clear_bits(TSCCTRL_EN, saradc->base + TSC_TSCCTRL);
}

static inline void saradc_tsc_fifo_reset(struct saradc *saradc)
{
	saradc_set_bits(TSCFIFOCS_FRES, saradc->base + TSC_TSCFIFOCS);
	saradc_clear_bits(TSCFIFOCS_FRES, saradc->base + TSC_TSCFIFOCS);
}

static void saradc_tsc_config(struct saradc *saradc)
{
	u32 tscctrl = 0, tsccfg = 0;

	/* clear all interrupts */
	writel(~0, saradc->base + SARADC_ISR);

	tscctrl |= TSCCTRL_WT(saradc->plat->tracking_index);
	tscctrl |= TSCCTRL_MXYZ;

	tsccfg |= TSCCFG_VST(saradc->plat->volt_settling_time);
	tsccfg |= TSCCFG_TDD(saradc->plat->touch_detect_delay);
	tsccfg |= TSCCFG_AVGS(saradc->plat->averaging_samples);

	writel(tscctrl, saradc->base + TSC_TSCCTRL);
	writel(tsccfg, saradc->base + TSC_TSCCFG);

	writel(TSCFRACTZ_F5W3, saradc->base + TSC_TSCFRACTZ);
}

static void saradc_tsc_reader(struct work_struct *work)
{
	struct saradc *saradc;
	struct saradc_tsc *tsc;
	bool newdata = false;

	tsc = container_of(work, struct saradc_tsc, touch_detect.work);
	saradc = container_of(tsc, struct saradc, tsc);

	if (saradc_tsc_pen_down(saradc)) {
		while (!saradc_tsc_fifo_empty(saradc)) {
			/* keep only the last sample */
			saradc_tsc_read_data(saradc);
			newdata = true;
		}
		if (newdata) {
			dev_dbg(&tsc->input->dev, "Xr = %d, Yr = %d, Zr = %d\n",
				tsc->x_last, tsc->y_last, tsc->z_last);
			input_report_abs(tsc->input, ABS_X, tsc->x_last);
			input_report_abs(tsc->input, ABS_Y, tsc->y_last);
			input_report_abs(tsc->input, ABS_PRESSURE, tsc->z_last);
			input_report_key(tsc->input, BTN_TOUCH, 1);
			input_sync(tsc->input);
		}
		schedule_delayed_work(&tsc->touch_detect, tsc->reader_interval);
	}
}

static irqreturn_t saradc_tsc_int(int irq, void *dev_id)
{
	struct saradc *saradc = (struct saradc *)dev_id;
	struct saradc_tsc *tsc = &saradc->tsc;

	if (saradc_tsc_get_fifoirq(saradc)) {
		saradc_tsc_clear_fifoirq(saradc);
		while (!saradc_tsc_fifo_empty(saradc)) {
			saradc_tsc_read_data(saradc);

			dev_dbg(&tsc->input->dev, "X = %d, Y = %d, Z = %d\n",
				tsc->x_last, tsc->y_last, tsc->z_last);

			input_report_abs(tsc->input, ABS_X, tsc->x_last);
			input_report_abs(tsc->input, ABS_Y, tsc->y_last);
			input_report_abs(tsc->input, ABS_PRESSURE, tsc->z_last);
			input_report_key(tsc->input, BTN_TOUCH, 1);
			input_sync(tsc->input);
		}
	}

	if (saradc_tsc_get_penirq(saradc)) {
		saradc_tsc_clear_penirq(saradc);
		if (saradc_tsc_pen_down(saradc)) {
			dev_dbg(&tsc->input->dev, "pen down");
			if (tsc->reader_interval)
				schedule_work(&tsc->touch_detect.work);
		} else {
			dev_dbg(&tsc->input->dev, "pen up");
			saradc_tsc_fifo_reset(saradc);
			input_report_abs(tsc->input, ABS_PRESSURE, 0);
			input_report_key(tsc->input, BTN_TOUCH, 0);
			input_sync(tsc->input);
		}
	}

	return IRQ_HANDLED;
}

static int saradc_tsc_open(struct input_dev *idev)
{
	struct saradc *saradc = input_get_drvdata(idev);
	struct saradc_tsc *tsc = &saradc->tsc;

	saradc_tsc_config(saradc);

	/* enable pen down interrupt */
	saradc_set_bits(IRQ_TOUCH, saradc->base + SARADC_IER);

	/* enable FIFO IRQ */
	if (!tsc->reader_interval) {
		writel(1, saradc->base + TSC_TSCFIFOTH);
		saradc_set_bits(IRQ_FIFOTH, saradc->base + SARADC_IER);
	}

	/* enable ADC */
	saradc_set_bits(TSCCTRL_EN, saradc->base + TSC_TSCCTRL);

	return 0;
}

static void saradc_tsc_close(struct input_dev *idev)
{
	struct saradc *saradc = input_get_drvdata(idev);
	struct saradc_tsc *tsc = &saradc->tsc;

	cancel_delayed_work_sync(&tsc->touch_detect);

	/* disable pen down interrupt */
	saradc_clear_bits(IRQ_TOUCH, saradc->base + SARADC_IER);
}

static int saradc_tsc_init(struct platform_device *pdev,
			   struct saradc *saradc)
{
	struct saradc_platdata *plat = saradc->plat;
	int ret;

	if (!plat) {
		dev_err(&pdev->dev, "no platform data found\n");
		return -EINVAL;
	}

	saradc->tsc.input = devm_input_allocate_device(&pdev->dev);
	if (!saradc->tsc.input) {
		dev_err(&pdev->dev, "not enough memory for input device\n");
		return -ENOMEM;
	}

	saradc->tsc.input->name = dev_name(&pdev->dev);
	saradc->tsc.input->dev.parent = &pdev->dev;
	saradc->tsc.input->open = saradc_tsc_open;
	saradc->tsc.input->close = saradc_tsc_close;

	__set_bit(EV_ABS, saradc->tsc.input->evbit);
	__set_bit(EV_KEY, saradc->tsc.input->evbit);
	__set_bit(BTN_TOUCH, saradc->tsc.input->keybit);

	input_set_abs_params(saradc->tsc.input, ABS_X, 0, MAX_12BIT, 0, 0);
	input_set_abs_params(saradc->tsc.input, ABS_Y, 0, MAX_12BIT, 0, 0);
	input_set_abs_params(saradc->tsc.input, ABS_PRESSURE, 0, MAX_12BIT, 0,
			     0);

	input_set_drvdata(saradc->tsc.input, saradc);

	ret = input_register_device(saradc->tsc.input);
	if (ret) {
		dev_err(&pdev->dev, "unable to register input device\n");
		return -ENODEV;
	}

	INIT_DELAYED_WORK(&saradc->tsc.touch_detect, saradc_tsc_reader);

	saradc->tsc.swap_xy = plat->swap_xy;
	saradc->tsc.flip_x = plat->flip_x;
	saradc->tsc.flip_y = plat->flip_y;

	if (plat->samples_per_second)
		saradc->tsc.reader_interval = HZ / plat->samples_per_second;

	ret = devm_request_threaded_irq(&pdev->dev, saradc->irq, NULL,
					saradc_tsc_int, IRQF_ONESHOT,
					dev_name(&pdev->dev), saradc);

	if (ret) {
		dev_err(&pdev->dev, "failed to allocate IRQ %d\n", saradc->irq);
		goto exit_cancel_work;
	}

	dev_info(&pdev->dev, "touchscreen mode enabled\n");
	ret = saradc_tsc_open(saradc->tsc.input);
	if (ret) {
		dev_err(&pdev->dev, "failed to open touchscreen device %d\n",
			ret);
		goto err_irq;
	}

	dev_info(&pdev->dev, "started at 0x%p, IRQ %d\n", saradc->base,
		 saradc->irq);

	return 0;

err_irq:
	free_irq(saradc->irq, saradc);

exit_cancel_work:
	input_unregister_device(saradc->tsc.input);
	cancel_delayed_work(&saradc->tsc.touch_detect);
	flush_scheduled_work();
	return ret;
}

static void saradc_tsc_remove(struct platform_device *pdev,
			      struct saradc *saradc)
{
	input_unregister_device(saradc->tsc.input);
	saradc_tsc_close(saradc->tsc.input);
	free_irq(saradc->irq, saradc);
	cancel_delayed_work(&saradc->tsc.touch_detect);
	flush_scheduled_work();
}

static int saradc_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val, int *val2,
			   long mask)
{
	struct saradc *saradc = iio_priv(indio_dev);
	unsigned int channel_value;
	int ret;

	if (mask != IIO_CHAN_INFO_RAW)
		return -EINVAL;

	if (chan->address >= MAX_CHANNEL ||
	    saradc->plat->tsc_enable ? chan->address < 4 : 0) {
		dev_err(&indio_dev->dev, "Invalid ADC channel %ld\n",
			chan->address);
		return -EINVAL;
	}

	mutex_lock(&indio_dev->mlock);

	channel_value = 1 << chan->address;
	/* Start capture */
	writel(channel_value, saradc->base + SARADC_ADCCAPT);
	while (!(readl(saradc->base + SARADC_ADCCAPT) & channel_value))
		;
	*val = readl(saradc->base + SARADC_ADCDATA0 + (chan->address * 4));
	*val2 = 0;
	ret = IIO_VAL_INT;

	mutex_unlock(&indio_dev->mlock);

	return ret;
}

static int saradc_reg_access(struct iio_dev *indio_dev,
			     unsigned int reg, unsigned int writeval,
			     unsigned int *readval)
{
	struct saradc *saradc = iio_priv(indio_dev);

	if (!readval)
		return -EINVAL;

	*readval = readl(saradc->base + reg);

	return 0;
}

static void saradc_dt_populate_pdata(struct device_node *np,
				     struct saradc_platdata *plat)
{
	u32 val;

	if (np) {
		if (!of_property_read_u32(np, "st,averaging-samples", &val))
			plat->averaging_samples = val;
		if (!of_property_read_u32(np, "st,tracking-index", &val))
			plat->tracking_index = val;
		if (!of_property_read_u32(np, "st,volt-settling-time", &val))
			plat->volt_settling_time = val;
		if (!of_property_read_u32(np, "st,touch-detect-delay", &val))
			plat->touch_detect_delay = val;
		if (!of_property_read_u32(np, "st,samples-per-second", &val))
			plat->samples_per_second = val;
		if (of_property_read_bool(np, "st,swap-xy"))
			plat->swap_xy = true;
		if (of_property_read_bool(np, "st,flip-x"))
			plat->flip_x = true;
		if (of_property_read_bool(np, "st,flip-y"))
			plat->flip_y = true;
		if (of_property_read_bool(np, "st,tsc-enable"))
			plat->tsc_enable = true;
	}
}

static const struct iio_info saradc_iio_info = {
	.read_raw = &saradc_read_raw,
	.debugfs_reg_access = &saradc_reg_access,
	.driver_module = THIS_MODULE,
};

static const struct of_device_id saradc_of_match[] = {
	{ .compatible = "st,sta-saradc", },
	{ },
};
MODULE_DEVICE_TABLE(of, saradc_of_match);

static int saradc_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct saradc_platdata *plat = pdev->dev.platform_data;
	struct saradc *saradc = NULL;
	struct iio_dev *indio_dev = NULL;
	struct resource *res;
	const char *devname;
	int ret;

	/* must have either platform data or DT node */
	if (!plat && !np) {
		dev_err(&pdev->dev, "no platform data or DT found\n");
		return -EINVAL;
	}

	if (!plat) {
		plat = devm_kzalloc(&pdev->dev, sizeof(*plat), GFP_KERNEL);
		if (!plat)
			return -ENOMEM;
	}

	if (np)
		saradc_dt_populate_pdata(np, plat);

	indio_dev = devm_iio_device_alloc(&pdev->dev,
					  sizeof(struct saradc));
	if (!indio_dev) {
		dev_err(&pdev->dev, "failed allocating iio device\n");
		return -ENOMEM;
	}

	saradc = iio_priv(indio_dev);
	saradc->plat = plat;

	devname = dev_name(&pdev->dev);
	platform_set_drvdata(pdev, indio_dev);
	saradc->dev = &pdev->dev;
	init_completion(&saradc->completion);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "MEM resource not defined\n");
		return -EINVAL;
	}

	if (!request_mem_region(res->start, resource_size(res), devname)) {
		dev_err(&pdev->dev, "unable to get device memory regions\n");
		return -EBUSY;
	}

	saradc->base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!saradc->base) {
		dev_err(&pdev->dev, "unable to remap register area\n");
		return -EINVAL;
	}

	saradc->pclk = devm_clk_get(&pdev->dev, "apb_pclk");
	if (IS_ERR(saradc->pclk))
		return PTR_ERR(saradc->pclk);

	clk_prepare_enable(saradc->pclk);

	ret = readl(saradc->base + SARADC_VERSION);
	if (ret == VERSION_ID) {
		dev_info(&pdev->dev, "version %d\n", ret);
	} else {
		dev_err(&pdev->dev, "unknown version %d\n", ret);
		ret = -EINVAL;
		goto err_disable_unprepare_clk;
	}

	saradc->irq = platform_get_irq(pdev, 0);
	if (saradc->irq < 0) {
		dev_err(&pdev->dev, "IRQ resource not defined\n");
		ret = -EINVAL;
		goto err_disable_unprepare_clk;
	}

	indio_dev->name = dev_name(&pdev->dev);
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->dev.of_node = pdev->dev.of_node;
	indio_dev->info = &saradc_iio_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = saradc_iio_channels;
	indio_dev->num_channels = MAX_CHANNEL;

	ret = iio_device_register(indio_dev);
	if (ret)
		goto err_disable_unprepare_clk;

	ret = of_platform_populate(np, saradc_of_match, NULL,
				   &indio_dev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed adding child nodes\n");
		goto err_of_populate;
	}

	saradc->adcctrl = ADCCTRL_ADCEN_IDLE << ADCCTRL_ADCEN |
			  ADCCTRL_REFSEL_EXT << ADCCTRL_REFSEL |
			  ADCCTRL_FREQ_HCLK26 << ADCCTRL_FREQ;

	/* select external reference for ADC */
	writel(saradc->adcctrl, saradc->base + SARADC_ADCCTRL);

	if (saradc->plat->tsc_enable) {
		ret = saradc_tsc_init(pdev, saradc);
		if (ret) {
			dev_err(&pdev->dev, "Touchscreen controller failure\n");
			goto err_of_populate;
		}
	}

	return 0;

err_of_populate:
	device_for_each_child(&indio_dev->dev, NULL,
			      saradc_remove_devices);
	iio_device_unregister(indio_dev);
err_disable_unprepare_clk:
	clk_disable_unprepare(saradc->pclk);

	return ret;
}

static int saradc_remove(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct saradc *saradc;
	struct resource *res;

	if (!indio_dev)
		return -EINVAL;

	saradc = iio_priv(indio_dev);

	if (saradc->plat->tsc_enable)
		saradc_tsc_remove(pdev, saradc);

	device_for_each_child(&indio_dev->dev, NULL,
			      saradc_remove_devices);

	iio_device_unregister(indio_dev);

	clk_disable_unprepare(saradc->pclk);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res)
		release_mem_region(res->start, resource_size(res));

	dev_info(&pdev->dev, "module stopped and unloaded\n");
	return 0;
}

#ifdef CONFIG_PM
static int saradc_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct saradc *saradc;

	saradc = iio_priv(indio_dev);

	saradc->adcctrl = readl(saradc->base + SARADC_ADCCTRL);

	if (!device_may_wakeup(&pdev->dev))
		disable_irq(saradc->irq);

	cancel_delayed_work(&saradc->tsc.touch_detect);
	flush_scheduled_work();
	clk_disable(saradc->pclk);

	return 0;
}

static int saradc_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);
	struct saradc *saradc;

	saradc = iio_priv(indio_dev);

	clk_enable(saradc->pclk);

	if (!device_may_wakeup(&pdev->dev))
		enable_irq(saradc->irq);

	 writel(saradc->adcctrl, saradc->base + SARADC_ADCCTRL);

	return 0;
}

static const struct dev_pm_ops saradc_dev_pm_ops = {
	.suspend = saradc_suspend,
	.resume = saradc_resume,
};

#define SARADC_DEV_PM_OPS (&saradc_dev_pm_ops)
#else
#define SARADC_DEV_PM_OPS NULL
#endif

static struct platform_driver saradc_driver = {
	.probe		= saradc_probe,
	.remove		= saradc_remove,
	.driver = {
		.name	= "sta-saradc",
		.owner	= THIS_MODULE,
		.pm	= SARADC_DEV_PM_OPS,
		.of_match_table = of_match_ptr(saradc_of_match),
	},
};

static int __init saradc_init(void)
{
	int ret;

	ret = platform_driver_register(&saradc_driver);
	return ret;
}

static void __exit saradc_exit(void)
{
	platform_driver_unregister(&saradc_driver);
}

module_init(saradc_init);
module_exit(saradc_exit);

MODULE_AUTHOR("Gian Antonio Sampietro <gianantonio.sampietro@st.com>");
MODULE_AUTHOR("Jean-Nicolas Graux <jean-nicolas.graux@st.com>");
MODULE_AUTHOR("Nicolas Guion <nicolas.guion@st.com>");
MODULE_AUTHOR("STMicroelectronics");
MODULE_DESCRIPTION("STMicroelectronics SAR ADC and resistive Touch Screen driver");
MODULE_LICENSE("GPL");
