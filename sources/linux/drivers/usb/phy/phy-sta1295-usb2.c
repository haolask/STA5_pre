/*
 * phy-sta1295-usb2.c - STA1295 USB2 Phy Driver
 *
 * Copyright (C) 2015 STMicroelectronics
 * Julien Delacou <julien.delacou@st.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/pm.h>

#define GINTSTS				0x14
#define GINTSTS_DISC			BIT(29)
#define XCVER_CNTRL1			0x70
#define XCVER_CNTRL2			0x74
#define XCVER_CDP_PRIMARY		(BIT(8) | BIT(6) | BIT(4))
#define XCVER_SER_DP_DM			(BIT(16) | BIT(15))
#define XCVER_SER_DP			(BIT(16))

#define XCVER_VDATDET			BIT(14)
#define XCVER_I_SDN_SRC			BIT(5)
#define XCVER_I_SDP_SNK			BIT(4)

#define XCVER_TRIM0			0x3C
#define I_INCURRENTENABLE_cUSB0_1V1	BIT(29)
#define I_INTRCC1MA_cUSB0_1V1		BIT(28)
#define I_INHSIPLUS_cUSB0_1V1		BIT(24)
#define I_INHSIPLUSENABLE_cUSB0_1V1	BIT(23)
#define I_INHSRFRED_cUSB0_1V1		BIT(21)
#define I_IHSTX_cUSB0_1V1_3		BIT(20)
#define I_IHSTX_cUSB0_1V1_2		BIT(19)
#define I_IHSTX_cUSB0_1V1_1		BIT(18)
#define I_IHSTX_cUSB0_1V1_0		BIT(17)
#define I_ZHSDRV_cUSB0_1V1_1		BIT(16)
#define I_ZHSDRV_cUSB0_1V1_0		BIT(15)
#define I_INCURRENTENABLE_cUSB1_1V1	BIT(14)
#define I_INTRCC1MA_cUSB1_1V1		BIT(13)
#define I_INHSIPLUS_cUSB1_1V1		BIT(9)
#define I_INHSIPLUSENABLE_cUSB1_1V1	BIT(8)
#define I_INHSRFRED_cUSB1_1V1		BIT(6)
#define I_IHSTX_cUSB1_1V1_3		BIT(5)
#define I_IHSTX_cUSB1_1V1_2		BIT(4)
#define I_IHSTX_cUSB1_1V1_1		BIT(3)
#define I_IHSTX_cUSB1_1V1_0		BIT(2)
#define I_ZHSDRV_cUSB1_1V1_1		BIT(1)
#define I_ZHSDRV_cUSB1_1V1_0		BIT(0)

#define XCVER_TRIM1			0x40
#define I_INSQUETUNE2_cUSB0_1V1		BIT(20)
#define I_INSQUETUNE1_cUSB0_1V1		BIT(19)
#define I_STAGSELECT_cUSB0_1V1		BIT(13)
#define I_INSQUETUNE2_cUSB1_1V1		BIT(7)
#define I_INSQUETUNE1_cUSB1_1V1		BIT(6)
#define I_STAGSELECT_cUSB1_1V1		BIT(0)

#define UTMI_PLL_CLK_SEL_OFFSET		0x84
#define PLL_POWER_DOWN			BIT(4)

#define UTMIOTG_IDDIG_OFFSET		0x80
#define UTMIOTG_DM_PD_USB0		BIT(5)
#define UTMIOTG_DP_PD_USB0		BIT(4)
#define UTMIOTG_IDDIG_USB0		BIT(3)
#define UTMIOTG_IDDIG_USB1		BIT(2)
#define UTMIOTG_DM_PD_USB1		BIT(1)
#define UTMIOTG_DP_PD_USB1		BIT(0)

#define DRVVBUS_USB1_OFFSET		0x78
#define DRVVBUS_USB0_OFFSET		0x8C

#define DRVVBUS				BIT(0)

#define DFT_CFG_REG_OFFSET		0x28
#define PHY_ENABLE			BIT(24)

/* HSIC setting needed through MISC register */
#define MISC_REG8_USB		0x24
#define IF_SELECT_HSIC		BIT(31)
#define HSIC_POR			BIT(11)

#define CDP_TIMEOUT	5000
#define CDP_DP_VALID_DETECT	20000000

enum {
	CDP_OFF = 0,
	CDP_ARMED,
	CDP_TRIGGED,
	CDP_DONE
} phy_sta1295_cdp_mode_e;

enum {
	STA1295_USB0 = 0,
	STA1295_USB1,
	STA1295_USB_MAX
} phy_sta1295_usb_e;

#define DRVVBUS_NAME_LEN 32

#define STA1295_USB_HOST "HOST"
#define STA1295_USB_DEVICE "DEVICE"

#define STA1295_DRVVBUS_ON "ON"
#define STA1295_DRVVBUS_OFF "OFF"

#define STA1295_DRVVBUS_ON_FOR_DEVICE_MODE "ON"
#define STA1295_DRVVBUS_OFF_FOR_DEVICE_MODE "OFF"

enum {
	STA1295_USB_DUAL_HOST_MODE = 0,
	STA1295_USB_DUAL_DEVICE_MODE
} phy_sta1295_dual_mode_e;

#define STA1295_USB_DUAL_DEFAULT_MODE STA1295_USB_DUAL_DEVICE_MODE

/**
 * @id: usb device id
 * @mode: current mode
 * @drvvbus: gpio reference, negative if invalid
 * @drvvbus_state: drvvbus current state
 * @cdp_state: identify if the CDP procedure is completed
 * @irq:controller irq which trigs the CDP procedure
 */
struct sta1295_phy_dev {
	struct sta1295_phy *phy;
	unsigned id;
	unsigned mode;
	struct gpio_desc *drvvbus;
	bool drvvbus_state;
	bool drvvbus_on_device_mode;
	bool cdp_mechanism;
	unsigned int cdp_state;
	struct delayed_work cdp_work;
	ktime_t cdp_start;
	int irq;
	void __iomem *usb_regs;
	bool idline;
	unsigned int idline_irq;
	struct gpio_desc *idline_gpio;
	struct delayed_work idline_work;
};

/**
 * @id: current usb device id for sysfs access
 * @regs: registers
 * @dft_regs: DFT registers
 * @pclk: peripheral clock
 * @xcver_trim0 : desired tuning config for XCVER_TRIM0
 * @xcver_trim1 : desired tuning config for XCVER_TRIM1
 * @devs: list of devices
 */
struct sta1295_phy {
	struct device	*dev;
	unsigned id;
	void __iomem *regs;
	struct clk *pclk;
	u32 xcver_trim0;
	u32 xcver_trim1;
	struct sta1295_phy_dev devs[STA1295_USB_MAX];
	struct regmap *misc_reg_base;
};

static int sta1295_phy_cdp_setup(struct sta1295_phy_dev *phy_dev,
				 bool state)
{
	u32 usb_instance;
	u32 cntrl_reg;
	u32 val;

	if (!phy_dev)
		return -EINVAL;

	usb_instance = phy_dev->id;
	cntrl_reg = usb_instance ? XCVER_CNTRL2 : XCVER_CNTRL1;

	dev_dbg(phy_dev->phy->dev, "%s:%s\n", __func__,  state ? "ON" : "OFF");

	if (state) {
		phy_dev->cdp_state = CDP_ARMED;
		phy_dev->cdp_start = ktime_get();

		val = readl(phy_dev->phy->regs + cntrl_reg);
		val |= XCVER_CDP_PRIMARY;
		writel(val, phy_dev->phy->regs + cntrl_reg);
	} else {
		cancel_delayed_work(&phy_dev->cdp_work);
		phy_dev->cdp_state = CDP_OFF;
		val = readl(phy_dev->phy->regs + cntrl_reg);
		val &= ~(XCVER_CDP_PRIMARY | XCVER_I_SDN_SRC);
		writel(val, phy_dev->phy->regs + cntrl_reg);
	}

	return 0;
}

static int sta1295_usb_phy_drvvbus(struct sta1295_phy *phy, int id, bool state)
{
	u32 val;
	u32 offset;

	if (id != STA1295_USB0 && id != STA1295_USB1)
		return -EINVAL;

	offset = (id == STA1295_USB0 ?
			DRVVBUS_USB0_OFFSET : DRVVBUS_USB1_OFFSET);

	if (phy->devs[id].drvvbus)
		gpiod_set_value_cansleep(phy->devs[id].drvvbus, state);

	val = readl(phy->regs + offset);
	if (!state)
		val &= ~(DRVVBUS);
	else
		val |= (DRVVBUS);
	writel(val, phy->regs + offset);

	phy->devs[id].drvvbus_state = state;

	return 0;
}

static int sta1295_usb_phy_dual_mode(struct sta1295_phy *phy, int id, int mode)
{
	int ret;
	u32 val;
	bool state;

	if (id != STA1295_USB0 && id != STA1295_USB1)
		return -EINVAL;

	if (mode == STA1295_USB_DUAL_HOST_MODE)
		state = true;
	else
		state = phy->devs[id].drvvbus_on_device_mode;

	ret = sta1295_usb_phy_drvvbus(phy, id, state);
	if (ret)
		return ret;

	/* Enable the USB PLL to access to USB OTG registers */
	val = readl(phy->regs + UTMI_PLL_CLK_SEL_OFFSET);
	val &= ~PLL_POWER_DOWN;
	writel(val, phy->regs + UTMI_PLL_CLK_SEL_OFFSET);

	/*
	 * force the ID to device mode, btw disable pulldown resistors on
	 * DP/DM used in host mode
	 */
	val = readl(phy->regs + UTMIOTG_IDDIG_OFFSET);

	switch (id) {
	case STA1295_USB0:
		if (mode != STA1295_USB_DUAL_HOST_MODE) {
			if (phy->devs[id].cdp_mechanism)
				sta1295_phy_cdp_setup(&phy->devs[id], false);
			val |= (UTMIOTG_IDDIG_USB0);
			val &= ~(UTMIOTG_DP_PD_USB0);
			val &= ~(UTMIOTG_DM_PD_USB0);
		} else {
			val &= ~(UTMIOTG_IDDIG_USB0);
			val |= (UTMIOTG_DP_PD_USB0);
			val |= (UTMIOTG_DM_PD_USB0);
			if (phy->devs[id].cdp_mechanism)
				sta1295_phy_cdp_setup(&phy->devs[id], true);
		}
		break;
	case STA1295_USB1:
		if (mode != STA1295_USB_DUAL_HOST_MODE) {
			if (phy->devs[id].cdp_mechanism)
				sta1295_phy_cdp_setup(&phy->devs[id], false);
			val |= (UTMIOTG_IDDIG_USB1);
			val &= ~(UTMIOTG_DP_PD_USB1);
			val &= ~(UTMIOTG_DM_PD_USB1);
		} else {
			val &= ~(UTMIOTG_IDDIG_USB1);
			val |= (UTMIOTG_DP_PD_USB1);
			val |= (UTMIOTG_DM_PD_USB1);
			if (phy->devs[id].cdp_mechanism)
				sta1295_phy_cdp_setup(&phy->devs[id], true);
		}
		break;
	default:
		break;
	}

	writel(val, phy->regs + UTMIOTG_IDDIG_OFFSET);
	phy->devs[id].mode = mode;

	dev_info(phy->dev, "%s: %s switch mode to %s\n", __func__,
		 id == STA1295_USB0 ? "USB0" : "USB1",
		 mode == STA1295_USB_DUAL_HOST_MODE ? "HOST" : "DEVICE");

	return 0;
}

static ssize_t show_usb_id(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", phy->id);
}

static ssize_t set_usb_id(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int err;
	unsigned id;
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	err = kstrtouint(buf, 10, &id);
	if (err)
		return err;

	if (id != STA1295_USB0 && id != STA1295_USB1)
		return -EINVAL;

	phy->id = id;

	return count;
}

static DEVICE_ATTR(usb_id, S_IRUGO | S_IWUSR,
		   show_usb_id, set_usb_id);

static ssize_t show_usb_mode(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	switch (phy->devs[phy->id].mode) {
	case STA1295_USB_DUAL_DEVICE_MODE:
		return sprintf(buf, "%s\n", STA1295_USB_DEVICE);
	case STA1295_USB_DUAL_HOST_MODE:
		return sprintf(buf, "%s\n", STA1295_USB_HOST);
	default:
		return sprintf(buf, "Invalid mode\n");
	}
	return 0;
}

static ssize_t set_usb_mode(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	if (!strncmp(buf, STA1295_USB_HOST, sizeof(STA1295_USB_HOST) - 1) &&
	    phy->devs[phy->id].mode == STA1295_USB_DUAL_DEVICE_MODE) {
		sta1295_usb_phy_dual_mode(phy, phy->id,
					  STA1295_USB_DUAL_HOST_MODE);
		return count;
	}
	if (!strncmp(buf, STA1295_USB_DEVICE, sizeof(STA1295_USB_DEVICE) - 1) &&
	    phy->devs[phy->id].mode == STA1295_USB_DUAL_HOST_MODE) {
		sta1295_usb_phy_dual_mode(phy, phy->id,
					  STA1295_USB_DUAL_DEVICE_MODE);
		return count;
	}
	if (strncmp(buf, STA1295_USB_DEVICE, sizeof(STA1295_USB_DEVICE) - 1) &&
	    strncmp(buf, STA1295_USB_HOST, sizeof(STA1295_USB_HOST) - 1))
		dev_err(dev, "usb_mode: either %s or %s allowed\n",
			STA1295_USB_HOST, STA1295_USB_DEVICE);
	return count;
}

static DEVICE_ATTR(usb_mode, S_IRUGO | S_IWUSR,
		   show_usb_mode, set_usb_mode);

static ssize_t show_usb_drvvbus(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	if (phy->devs[phy->id].drvvbus_state)
		return sprintf(buf, "%s\n", STA1295_DRVVBUS_ON);
	else
		return sprintf(buf, "%s\n", STA1295_DRVVBUS_OFF);
}

static ssize_t set_usb_drvvbus(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	if (!strncmp(buf, STA1295_DRVVBUS_OFF,
		     sizeof(STA1295_DRVVBUS_OFF) - 1) &&
		     phy->devs[phy->id].drvvbus_state) {
		sta1295_usb_phy_drvvbus(phy, phy->id, false);
		return count;
	}
	if (!strncmp(buf, STA1295_DRVVBUS_ON,
		     sizeof(STA1295_DRVVBUS_ON) - 1) &&
		     !phy->devs[phy->id].drvvbus_state) {
		sta1295_usb_phy_drvvbus(phy, phy->id, true);
		return count;
	}
	if (strncmp(buf, STA1295_DRVVBUS_OFF,
		    sizeof(STA1295_DRVVBUS_OFF) - 1) &&
		    strncmp(buf, STA1295_DRVVBUS_ON,
			    sizeof(STA1295_DRVVBUS_ON) - 1))
		dev_err(dev, "usb_drvvbus: either %s or %s allowed\n",
			STA1295_DRVVBUS_OFF, STA1295_DRVVBUS_ON);
	return count;
}

static DEVICE_ATTR(usb_drvvbus, S_IRUGO | S_IWUSR,
		   show_usb_drvvbus, set_usb_drvvbus);

static ssize_t show_drvvbus_on_device_mode(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	if (phy->devs[phy->id].drvvbus_on_device_mode)
		return sprintf(buf, "%s\n",
		STA1295_DRVVBUS_ON_FOR_DEVICE_MODE);
	else
		return sprintf(buf, "%s\n",
		STA1295_DRVVBUS_OFF_FOR_DEVICE_MODE);
}

static ssize_t set_drvvbus_on_device_mode(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	if (!phy) {
		dev_err(dev, "drvvbus_on_device_mode: phy not consistent\n");
		return 0;
	}

	if (!strncmp(buf, STA1295_DRVVBUS_ON_FOR_DEVICE_MODE,
		     sizeof(STA1295_DRVVBUS_ON_FOR_DEVICE_MODE) - 1)) {
		phy->devs[phy->id].drvvbus_on_device_mode = true;
		return count;
	}

	if (!strncmp(buf, STA1295_DRVVBUS_OFF_FOR_DEVICE_MODE,
		     sizeof(STA1295_DRVVBUS_OFF_FOR_DEVICE_MODE) - 1)) {
		phy->devs[phy->id].drvvbus_on_device_mode = false;
		return count;
	}

	dev_err(dev, "drvvbus_on_device_mode: either %s or %s allowed\n",
		STA1295_DRVVBUS_OFF_FOR_DEVICE_MODE,
		STA1295_DRVVBUS_ON_FOR_DEVICE_MODE);
	return 0;
}

static DEVICE_ATTR(drvvbus_on_device_mode, S_IRUGO | S_IWUSR,
		   show_drvvbus_on_device_mode, set_drvvbus_on_device_mode);

static struct phy_ops ops = {
	.init		= NULL,
	.exit		= NULL,
	.power_on	= NULL,
	.power_off	= NULL,
	.owner		= THIS_MODULE,
};

static int sta1295_phy_usb_tuning(struct device *dev, struct sta1295_phy *phy)
{
	u32 val1, val2;

	if (!dev || !phy)
		return -EINVAL;

	/*  PHY tuning*/
	if (!phy->xcver_trim0) {
		val1 = readl(phy->regs + XCVER_TRIM0);
		val1 &= ~(I_INCURRENTENABLE_cUSB0_1V1);
		val1 |= (I_INTRCC1MA_cUSB0_1V1);
		val1 &= ~(I_INHSIPLUS_cUSB0_1V1);
		val1 &= ~(I_INHSIPLUSENABLE_cUSB0_1V1);
		val1 |= (I_INHSRFRED_cUSB0_1V1);
		val1 |= (I_IHSTX_cUSB0_1V1_0);
		val1 |= (I_IHSTX_cUSB0_1V1_1);
		val1 &= ~(I_IHSTX_cUSB0_1V1_2);
		val1 &= ~(I_IHSTX_cUSB0_1V1_3);
		val1 |= (I_ZHSDRV_cUSB0_1V1_0);
		val1 &= ~(I_ZHSDRV_cUSB0_1V1_1);
		val1 &= ~(I_INCURRENTENABLE_cUSB1_1V1);
		val1 |= (I_INTRCC1MA_cUSB1_1V1);
		val1 &= ~(I_INHSIPLUS_cUSB1_1V1);
		val1 &= ~(I_INHSIPLUSENABLE_cUSB1_1V1);
		val1 |= (I_INHSRFRED_cUSB1_1V1);
		val1 |= (I_IHSTX_cUSB1_1V1_0);
		val1 |= (I_IHSTX_cUSB1_1V1_1);
		val1 &= ~(I_IHSTX_cUSB1_1V1_2);
		val1 &= ~(I_IHSTX_cUSB1_1V1_3);
		val1 |= (I_ZHSDRV_cUSB1_1V1_0);
		val1 &= ~(I_ZHSDRV_cUSB1_1V1_1);
	} else {
		val1 = phy->xcver_trim0;
	}
	writel(val1, phy->regs + XCVER_TRIM0);

	if (!phy->xcver_trim1) {
		val2 = readl(phy->regs + XCVER_TRIM1);
		val2 |= (I_INSQUETUNE2_cUSB0_1V1);
		val2 |= (I_INSQUETUNE1_cUSB0_1V1);
		val2 |= (I_STAGSELECT_cUSB0_1V1);
		val2 |= (I_INSQUETUNE2_cUSB1_1V1);
		val2 |= (I_INSQUETUNE1_cUSB1_1V1);
		val2 |= (I_STAGSELECT_cUSB1_1V1);
	} else {
		val2 = phy->xcver_trim1;
	}
	writel(val2, phy->regs + XCVER_TRIM1);

	dev_dbg(dev, "PHY tuning XCVER_TRIM0 :%x  XCVER_TRIM1 :%x\n",
		val1, val2);

	return 0;
}

static void sta1295_phy_cdp_timeout_work_func(struct work_struct *work)
{
	struct sta1295_phy_dev *phy_dev =
		container_of(work, struct sta1295_phy_dev, cdp_work.work);
	u32 usb_instance;
	u32 cntrl_reg;
	u32 val;

	usb_instance = phy_dev->id;
	cntrl_reg = usb_instance ? XCVER_CNTRL2 : XCVER_CNTRL1;

	if (phy_dev->cdp_state != CDP_TRIGGED) {
		dev_err(phy_dev->phy->dev,
			"Abnormal Procedure CDP TimeOut called for USB%d",
			usb_instance);
		return;
	}

	/* Reset All the CDP procedure for this USB port  */
	val = readl(phy_dev->phy->regs + cntrl_reg);

	if ((val & XCVER_CDP_PRIMARY) != XCVER_CDP_PRIMARY)
		dev_dbg(phy_dev->phy->dev,
			"cntrl register not consistent %s",
			"with cdp logical state");

	dev_dbg(phy_dev->phy->dev,
		"CDP procedure TimeOut for USB%d %s",
		usb_instance,
		" Reset D- pull-up\n");
	val &= ~XCVER_I_SDN_SRC;
	writel(val, phy_dev->phy->regs + cntrl_reg);
	phy_dev->phy->devs[usb_instance].cdp_start = ktime_get();
	phy_dev->cdp_state = CDP_ARMED;
	return;
}

static irqreturn_t sta1295_phy_cdp_usb2_irq(int irq, void *dev)
{
	u32 val;
	struct platform_device *pdev = dev;
	struct sta1295_phy *phy = platform_get_drvdata(pdev);
	u32 inst;
	u32 cntrl_reg;
	ktime_t new_cdp_dp_det;


	inst = irq == phy->devs[STA1295_USB0].irq ?
				STA1295_USB0 : STA1295_USB1;

	cntrl_reg = inst ? XCVER_CNTRL2 : XCVER_CNTRL1;

	if (phy->devs[inst].cdp_state == CDP_OFF)
		return IRQ_RETVAL(0);

	if (phy->devs[inst].cdp_state == CDP_DONE) {
		/*Test if disconnect event is detected by USB controller
		 * in order to enable back the CDP primary detection
		 */
		val = readl(phy->devs[inst].usb_regs + GINTSTS);
		if (val & GINTSTS_DISC) {
			dev_dbg(&pdev->dev,
				"USB%d disconnect CDP reactivated %s",
				inst,
				"=> Bit4/6/8 set");
			val = readl(phy->regs + cntrl_reg);
			val &= ~XCVER_I_SDN_SRC;
			val = val | XCVER_CDP_PRIMARY;
			writel(val, phy->regs + cntrl_reg);
			phy->devs[inst].cdp_state = CDP_ARMED;
			cancel_delayed_work(&phy->devs[inst].cdp_work);
			phy->devs[inst].cdp_start = ktime_get();
		}
		return IRQ_RETVAL(0);
		}

	/* Test if the CDP primary detection is enabled */
	val = readl(phy->regs + cntrl_reg);
	if ((val & XCVER_CDP_PRIMARY) != XCVER_CDP_PRIMARY) {
		dev_dbg(&pdev->dev,
			      "cntrl register not consistent %s",
			      "with cdp logical state");
		return IRQ_RETVAL(0);
		}

	dev_dbg(&pdev->dev, "XCVER_CNTRL%d:%x\n", inst + 1, val);

	if (phy->devs[inst].cdp_state == CDP_ARMED) {
		if ((val & XCVER_VDATDET) && !(val & XCVER_I_SDN_SRC)) {
			/* Check determinist D+ signal and not spurious */
			new_cdp_dp_det = ktime_get();
			if (ktime_to_ns(ktime_sub(new_cdp_dp_det,
						  phy->devs[inst].cdp_start))
					< CDP_DP_VALID_DETECT) {
				phy->devs[inst].cdp_state = CDP_TRIGGED;
				dev_dbg(&pdev->dev, "BIT5:0 Bit14:1 ->%s",
					"set BIT 5\n");
				val |=	XCVER_I_SDN_SRC;
				writel(val, phy->regs + cntrl_reg);
				mod_delayed_work(system_wq,
						 &phy->devs[inst].cdp_work,
						 msecs_to_jiffies(CDP_TIMEOUT));
			} else{
				dev_dbg(&pdev->dev, "Spurious Dp detection USB%d\n",
					inst);
				phy->devs[inst].cdp_start =
					new_cdp_dp_det;
			}
		}

		if (val & XCVER_SER_DP_DM) {
			phy->devs[inst].cdp_state = CDP_DONE;
			cancel_delayed_work(&phy->devs[inst].cdp_work);
			dev_dbg(&pdev->dev, "BIT15|BIT16:1 => Reset Bit4/5/6/8 %s",
				"CDP Detection procedure stopped\n");
			val &= ~(XCVER_CDP_PRIMARY | XCVER_I_SDN_SRC);
			writel(val, phy->regs + cntrl_reg);
		}
	}

	if (phy->devs[inst].cdp_state == CDP_TRIGGED) {
		if (!(val & XCVER_VDATDET) && (val & XCVER_I_SDN_SRC)) {
			dev_dbg(&pdev->dev, "BIT5:1 Bit14:0  ->%s",
				"reset BIT 5\n");
			val &= ~(XCVER_I_SDN_SRC);
			writel(val, phy->regs + cntrl_reg);
		}

		if (val & XCVER_SER_DP_DM) {
			phy->devs[inst].cdp_state = CDP_DONE;
			cancel_delayed_work(&phy->devs[inst].cdp_work);
			dev_dbg(&pdev->dev, "BIT15|BIT16:1 =>%s %s",
				"Reset Bit4/5/6/8",
				"CDP Detection procedure stopped\n");
			val &= ~(XCVER_CDP_PRIMARY | XCVER_I_SDN_SRC);
			writel(val, phy->regs + cntrl_reg);
		}
		return IRQ_RETVAL(1);
	}

	return IRQ_RETVAL(0);
}

static void  sta1295_phy_cdp_suspend(struct device *dev,
				     struct sta1295_phy_dev *phy_dev)
{
	u32 usb_instance;
	u32 cntrl_reg;
	u32 val;

	disable_irq(phy_dev->irq);
	cancel_delayed_work(&phy_dev->cdp_work);

	usb_instance = phy_dev->id;
	cntrl_reg = usb_instance ? XCVER_CNTRL2 : XCVER_CNTRL1;
	val = readl(phy_dev->phy->regs + cntrl_reg);
	val &= ~(XCVER_CDP_PRIMARY | XCVER_I_SDN_SRC);
	writel(val, phy_dev->phy->regs + cntrl_reg);
	phy_dev->cdp_state = CDP_OFF;
}

static void  sta1295_phy_cdp_resume(struct device *dev,
				    struct sta1295_phy_dev *phy_dev)
{
	enable_irq(phy_dev->irq);
	/* All correct settings dedicated to CDP resuming
	 * are handled by the flow call
	 * sta1295_usb_phy_dual_mode-> sta1295_phy_cdp_setup
	 * done in primary resume operation
	 */
}

static void  sta1295_phy_cdp_cleanup(struct platform_device *pdev,
				     struct sta1295_phy_dev *phy_dev)
{
	disable_irq(phy_dev->irq);
	cancel_delayed_work(&phy_dev->cdp_work);
	devm_free_irq(&pdev->dev, phy_dev->irq, phy_dev);
}

static int sta1295_phy_cdp_probe(struct platform_device *pdev,
				 struct sta1295_phy_dev *phy_dev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct device_node *np = dev->of_node;
	int ret;
	char property[20];

	if (!phy_dev)
		return -EINVAL;

	phy_dev->cdp_mechanism = false;

	sprintf(property, "st,usb%d_cdp", phy_dev->id);
	if (of_property_read_bool(np, property)) {
		phy_dev->cdp_mechanism = true;
		sprintf(property, "usb%d_regs", phy_dev->id);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						   property);
		if (!res) {
			dev_err(&pdev->dev, "%s resource not defined\n",
				property);
			return -EINVAL;
		}

		phy_dev->usb_regs = devm_ioremap(dev, res->start,
						 resource_size(res));
		if (IS_ERR(phy_dev->usb_regs)) {
			dev_err(&pdev->dev, "%s ioremap error\n", property);
			return PTR_ERR(phy_dev->usb_regs);
		}

		sprintf(property, "usb%d_irq", phy_dev->id);
		res = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
						   property);
		if (!res) {
			dev_err(&pdev->dev, "%s resource not defined\n",
				property);
			return -EINVAL;
		}
		phy_dev->irq = res->start;

		ret = devm_request_irq(&pdev->dev, phy_dev->irq,
				       sta1295_phy_cdp_usb2_irq,
				       IRQF_SHARED,
				       dev_name(&pdev->dev), pdev);
		if (ret) {
			dev_err(&pdev->dev, "failed to allocate USB%d irq%d\n",
				phy_dev->id, ret);
			return ret;
		}

		INIT_DELAYED_WORK(&phy_dev->cdp_work,
				  sta1295_phy_cdp_timeout_work_func);
	}

	return 0;
}

static void sta1295_phy_idline_work_func(struct work_struct *work)
{
	struct sta1295_phy_dev *phy_dev =
		container_of(work, struct sta1295_phy_dev, idline_work.work);
	int level;

	level = gpiod_get_value(phy_dev->idline_gpio);

	switch (level) {
	case 0:
		dev_info(phy_dev->phy->dev,
			 "USB ID Line requests Device Mode\n");
		sta1295_usb_phy_dual_mode(phy_dev->phy,
					  phy_dev->id,
					  STA1295_USB_DUAL_DEVICE_MODE);
		break;

	case 1:
		dev_info(phy_dev->phy->dev,
			 "USB ID Line requests Host Mode\n");
		sta1295_usb_phy_dual_mode(phy_dev->phy,
					  phy_dev->id,
					  STA1295_USB_DUAL_HOST_MODE);
		break;

	default:
		dev_err(phy_dev->phy->dev, "Abnormal ID line level");
		break;
	}
}

static irqreturn_t sta1295_phy_idline_isr(int irq, void *dev_id)
{
	struct sta1295_phy_dev *phy_dev = dev_id;

	WARN_ON(irq != phy_dev->idline_irq);

	mod_delayed_work(system_wq,
			 &phy_dev->idline_work,
			 msecs_to_jiffies(200));

	return IRQ_HANDLED;
}

static int  sta1295_phy_idline_setup(struct device *dev,
				     struct sta1295_phy_dev *phy_dev)
{
	int ret;

	ret = gpiod_to_irq(phy_dev->idline_gpio);
	if (ret < 0) {
		dev_err(dev,
			"%s Unable to get irq number for GPIO %d, error %d\n",
			__func__, desc_to_gpio(phy_dev->idline_gpio), ret);
		return ret;
	}

	phy_dev->idline_irq = (unsigned int)ret;

	INIT_DELAYED_WORK(&phy_dev->idline_work, sta1295_phy_idline_work_func);

	ret = devm_request_any_context_irq(dev, phy_dev->idline_irq,
					   sta1295_phy_idline_isr,
					   IRQF_TRIGGER_RISING |
					   IRQF_TRIGGER_FALLING,
					   "usb_id_line", phy_dev);
	if (ret < 0) {
		dev_err(dev, "%s Unable to claim irq %d; error %d\n",
			__func__, phy_dev->idline_irq, ret);
		return ret;
	}

	mod_delayed_work(system_wq,
			 &phy_dev->idline_work,
			 msecs_to_jiffies(200));

	return 0;
}

static void  sta1295_phy_idline_cleanup(struct platform_device *pdev,
					struct sta1295_phy_dev *phy_dev)
{
	disable_irq(phy_dev->idline_irq);
	cancel_delayed_work(&phy_dev->idline_work);
	devm_free_irq(&pdev->dev, phy_dev->idline_irq, phy_dev);
}

static void  sta1295_phy_idline_suspend(struct sta1295_phy_dev *phy_dev)
{
	disable_irq(phy_dev->idline_irq);
	cancel_delayed_work(&phy_dev->idline_work);
}

static void  sta1295_phy_idline_resume(struct sta1295_phy_dev *phy_dev)
{
	enable_irq(phy_dev->idline_irq);
	mod_delayed_work(system_wq,
			 &phy_dev->idline_work,
			 msecs_to_jiffies(200));
}

static int sta1295_phy_idline_probe(struct device *dev,
				    struct sta1295_phy_dev *phy_dev)
{
	char property[30];

	if (!phy_dev)
		return -EINVAL;

	phy_dev->idline = false;

	sprintf(property, "st,usb%d_idline", phy_dev->id);

	phy_dev->idline_gpio = devm_gpiod_get_optional(dev,
			property,
			GPIOD_IN);
	if (IS_ERR(phy_dev->idline_gpio)) {
		dev_err(dev, "failed to request gpio for USB%d idline\n",
			phy_dev->id);
		return -EINVAL;
	}

	if (!phy_dev->idline_gpio)
		return 0;

	if (sta1295_phy_idline_setup(dev, phy_dev)) {
		dev_err(dev,
			"failed to set idline for controller USB%d\n",
			phy_dev->id);
		return -EFAULT;
	}

	return 0;
}

static int sta1295_phy_usb_hsic_enable(struct sta1295_phy *phy)
{
	int ret;

	if (!phy->misc_reg_base)
		return 0;

	ret = regmap_update_bits(phy->misc_reg_base, MISC_REG8_USB,
				 IF_SELECT_HSIC | HSIC_POR,
				 IF_SELECT_HSIC | HSIC_POR);
	if (ret) {
		dev_err(phy->dev, "%s: failed to update register: %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static int sta1295_phy_usb_hsic_setup(struct sta1295_phy *phy)
{
	int ret;
	struct device_node *node;
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
	struct device *dev = phy->dev;

	phy->misc_reg_base = NULL;

	if (!of_property_read_bool(dev->of_node, "st,usb0_hsic")) {
		dev_dbg(dev, "no hsic settings\n");
		return 0;
	}

	node = of_parse_phandle(dev->of_node, "sta,misc-reg", 0);
	if (!node) {
		dev_err(dev, "Missing property for misc register\n");
		return -ENODEV;
	}

#ifdef CONFIG_REGMAP_SMC
	ret = of_address_to_resource(node, 0, &regmap_res);
	if (ret) {
		dev_err(dev, "%s of_address_to_resource fail %d\n", __func__,
			ret);
		of_node_put(node);
		return ret;
	}
	phy->misc_reg_base = regmap_init_smc(NULL, &regmap_res,
					&regmap_cfg);
#else
	phy->misc_reg_base = syscon_node_to_regmap(node);
#endif
	of_node_put(node);

	if (IS_ERR(phy->misc_reg_base)) {
		dev_err(dev, "misc register not accessible %ld\n",
			PTR_ERR(phy->misc_reg_base));
		return PTR_ERR(phy->misc_reg_base);
	}

	ret = sta1295_phy_usb_hsic_enable(phy);
	if (ret) {
		dev_err(dev, "%s: failed to enable hsic: %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sta1295_usb2_suspend(struct device *dev)
{
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	if (phy->devs[STA1295_USB0].cdp_mechanism)
		sta1295_phy_cdp_suspend(dev, &phy->devs[STA1295_USB0]);
	if (phy->devs[STA1295_USB1].cdp_mechanism)
		sta1295_phy_cdp_suspend(dev, &phy->devs[STA1295_USB1]);

	if (phy->devs[STA1295_USB0].idline)
		sta1295_phy_idline_suspend(&phy->devs[STA1295_USB0]);
	if (phy->devs[STA1295_USB1].idline)
		sta1295_phy_idline_suspend(&phy->devs[STA1295_USB1]);

	clk_disable_unprepare(phy->pclk);

	pinctrl_pm_select_sleep_state(dev);

	return 0;
}

static int sta1295_usb2_resume(struct device *dev)
{
	int ret;
	u32 id;
	struct sta1295_phy *phy = dev_get_drvdata(dev);

	pinctrl_pm_select_default_state(dev);

	ret = clk_prepare_enable(phy->pclk);
	if (ret < 0)
		return ret;

	for (id = STA1295_USB0; id < STA1295_USB_MAX; id++) {
		if (phy->devs[id].idline)
			sta1295_phy_idline_resume(&phy->devs[id]);
		if (phy->devs[id].cdp_mechanism)
			sta1295_phy_cdp_resume(dev, &phy->devs[id]);
		ret = sta1295_usb_phy_dual_mode(phy, id, phy->devs[id].mode);
		if (ret)
			return ret;
	}

	ret = sta1295_phy_usb_tuning(dev, phy);
	if (ret)
		return ret;

	ret = sta1295_phy_usb_hsic_enable(phy);
	if (ret) {
		dev_err(dev, "%s: failed to enable hsic\n",
			__func__);
		return ret;
	}

	return ret;
}
#endif

static int sta1295_phy_usb2_probe(struct platform_device *pdev)
{
	int ret;
	u32 id;
	struct device *dev = &pdev->dev;
	struct sta1295_phy *phy;
	struct resource *res;
	struct phy *gphy;
	struct phy_provider *phy_provider;
	struct device_node *np = dev->of_node;
	const char *str;
	u32 val = 0;

	phy = devm_kzalloc(dev, sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	phy->dev = dev;
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy_regs");
	phy->regs = devm_ioremap_resource(dev, res);
	if (IS_ERR(phy->regs))
			return PTR_ERR(phy->regs);

	/* current id arbitrarily set to USB0 */
	phy->id = STA1295_USB0;

	platform_set_drvdata(pdev, phy);

	gphy = devm_phy_create(dev, NULL, &ops);
	if (IS_ERR(gphy))
		return PTR_ERR(gphy);

	/* The STA1295 PHY supports an 8-bit wide UTMI interface */
	phy_set_bus_width(gphy, 8);

	phy_set_drvdata(gphy, phy);

	phy->pclk = devm_clk_get(dev, "apbreg_clk");
	if (IS_ERR(phy->pclk))
		return PTR_ERR(phy->pclk);

	ret = clk_prepare_enable(phy->pclk);
	if (ret < 0)
		return ret;

	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);
	if (IS_ERR(phy_provider)) {
		clk_disable_unprepare(phy->pclk);
		return PTR_ERR(phy_provider);
	}

	/* USB phy tuning values can be design-specific, so get them from dts */
	phy->xcver_trim0 = 0;
	phy->xcver_trim1 = 0;
	if (!of_property_read_u32(np, "st,xcver_trim0", &val))
		phy->xcver_trim0 = val;

	if (!of_property_read_u32(np, "st,xcver_trim1", &val))
		phy->xcver_trim1 = val;

	ret = sta1295_phy_usb_tuning(dev, phy);
	if (ret)
		return ret;

	for (id = STA1295_USB0; id < STA1295_USB_MAX; id++) {
		phy->devs[id].phy = phy;
		phy->devs[id].id = id;
		phy->devs[id].drvvbus = NULL;
		phy->devs[id].drvvbus_on_device_mode = false;
		phy->devs[id].mode = STA1295_USB_DUAL_DEFAULT_MODE;

		ret = of_property_read_string(np,
					      id == STA1295_USB0 ?
					      "st,usb0_mode" : "st,usb1_mode",
					      &str);
		if (ret >= 0) {
			if (!strncmp(str, STA1295_USB_HOST,
				     sizeof(STA1295_USB_HOST) - 1))
				phy->devs[id].mode = STA1295_USB_DUAL_HOST_MODE;
			else if (!strncmp(str, STA1295_USB_DEVICE,
					  sizeof(STA1295_USB_HOST) - 1))
				phy->devs[id].mode =
					STA1295_USB_DUAL_DEVICE_MODE;
			else
				dev_info(dev,
					 "usb_mode: either %s or %s allowed\n",
					 STA1295_USB_HOST, STA1295_USB_DEVICE);
		}

		phy->devs[id].drvvbus = devm_gpiod_get_optional(dev,
				id == STA1295_USB0 ?
				"st,usb0_drvvbus" : "st,usb1_drvvbus",
				GPIOD_OUT_LOW);
		if (IS_ERR(phy->devs[id].drvvbus)) {
			dev_err(dev,
				"USB%d failed to request %s (%ld)\n",
				id, id == STA1295_USB0 ?
				"st,usb0_drvvbus" : "st,usb1_drvvbus",
				PTR_ERR(phy->devs[id].drvvbus));
			phy->devs[id].drvvbus = NULL;
		}

		if (phy->devs[id].drvvbus)
			dev_err(dev,
				"USB%d %s = GPIO_%d\n",
				id, id == STA1295_USB0 ?
				"st,usb0_drvvbus" : "st,usb1_drvvbus",
				desc_to_gpio(phy->devs[id].drvvbus));

		ret = sta1295_phy_idline_probe(dev, &phy->devs[id]);
		if (ret) {
			dev_err(dev,
				"USB%d: failed to setup idline\n",
				id);
			return ret;
		}

		ret = sta1295_phy_cdp_probe(pdev, &phy->devs[id]);
		if (ret) {
			dev_err(dev,
				"USB%d CDP mechanism probe failure\n",
				id);
			return ret;
		}

		ret = sta1295_usb_phy_dual_mode(phy, id, phy->devs[id].mode);
		if (ret) {
			dev_err(dev, "%s; sta1295_usb_phy_dual_mode fail\n",
				__func__);
			return ret;
		}
	}

	ret = sta1295_phy_usb_hsic_setup(phy);
	if (ret) {
		dev_err(dev, "%s: failed to setup hsic\n",
			__func__);
		return ret;
	}

	device_create_file(&pdev->dev, &dev_attr_usb_mode);
	device_create_file(&pdev->dev, &dev_attr_usb_drvvbus);
	device_create_file(&pdev->dev, &dev_attr_usb_id);
	device_create_file(&pdev->dev, &dev_attr_drvvbus_on_device_mode);

	dev_info(dev, "%s: done\n", __func__);

	return 0;
}

static int sta1295_phy_usb2_remove(struct platform_device *pdev)
{
	struct sta1295_phy *phy = platform_get_drvdata(pdev);

	sta1295_phy_cdp_cleanup(pdev, &phy->devs[STA1295_USB0]);
	sta1295_phy_cdp_cleanup(pdev, &phy->devs[STA1295_USB1]);

	clk_disable_unprepare(phy->pclk);
	sta1295_usb_phy_dual_mode(phy, STA1295_USB0,
				  STA1295_USB_DUAL_DEFAULT_MODE);
	sta1295_usb_phy_dual_mode(phy, STA1295_USB1,
				  STA1295_USB_DUAL_DEFAULT_MODE);

	if (phy->devs[STA1295_USB0].idline)
		sta1295_phy_idline_cleanup(pdev, &phy->devs[STA1295_USB0]);
	if (phy->devs[STA1295_USB1].idline)
		sta1295_phy_idline_cleanup(pdev, &phy->devs[STA1295_USB1]);

	device_remove_file(&pdev->dev, &dev_attr_usb_mode);
	device_remove_file(&pdev->dev, &dev_attr_usb_drvvbus);
	device_remove_file(&pdev->dev, &dev_attr_usb_id);
	device_remove_file(&pdev->dev, &dev_attr_drvvbus_on_device_mode);

	return 0;
}

static const struct of_device_id sta1295_phy_usb2_dt_ids[] = {
	{ .compatible = "st,sta1295-usb2-phy" },
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, sta1295_phy_usb2_dt_ids);

static const struct dev_pm_ops sta1295_phy_usb2_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sta1295_usb2_suspend,
				sta1295_usb2_resume)
};

static struct platform_driver sta1295_usb2_driver = {
	.probe		= sta1295_phy_usb2_probe,
	.remove		= sta1295_phy_usb2_remove,
	.driver		= {
		.name	= "sta1295-phy-usb2",
		.owner	= THIS_MODULE,
		.of_match_table = sta1295_phy_usb2_dt_ids,
		.pm	= &sta1295_phy_usb2_pm_ops,
	},
};

module_platform_driver(sta1295_usb2_driver);

MODULE_ALIAS("platform:sta1295-usb2");
MODULE_AUTHOR("Julien Delacou <julien.delacou@st.com>");
MODULE_DESCRIPTION("STMicroelectronics STA1295 USB 2.0 PHY driver");
MODULE_LICENSE("GPL v2");
