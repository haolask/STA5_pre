/*
 * STA Hardware Semaphore driver
 *
 * Copyright (C) 2014 STMicroelectronics
 *
 * Implements sta semaphore handling, with interrupts.
 *
 * Author: Philippe Langlais <philippe.langlais@st.com>
 * Author: Olivier Lebreton <olivier.lebreton@st.com>
 * Heavily borrowed from the work of :
 *   Mathieu Poirier <mathieu.poirier@linaro.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/hwspinlock.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sta_hsem.h>

#include "hwspinlock_internal.h"

/*
 * Implementation of ST HSem protocol with interrupts.
 */

#define MAX_SEMAPHORE			16	/* a total of 16 semaphores */
#define RESET_SEMAPHORE			(0)	/* free */

/*
 * CPU ID for master running sta kernel.
 * Hswpinlocks should only be used to synchonise operations
 * between the Cortex R4 core and the other CPUs (Cortex M3).
 * Hence forcing the masterID to a preset value.
 */
#define HSEM_MASTER_ID			0x01

#define HSEM_RESET_IRQS			((1 << MAX_SEMAPHORE) - 1)
#define HSEM_INTSX(n)			((n) << 4)

#define HSEM_REGISTER_OFFSET		0x00
#define HSEM_ICRALL			0x90

/* IRQ_A (mapped to R4) register offsets */
#define HSEM_IMSCA			0xA0
#define HSEM_RISA			0xA4
#define HSEM_MISA			0xA8
#define HSEM_ICRA			0xAC

/* IRQ_B (mapped to M3) register offsets */
#define HSEM_IMSCB			0xB0
#define HSEM_RISB			0xB4
#define HSEM_MISB			0xB8
#define HSEM_ICRB			0xBC

static struct clk *sta_hsem_clk;
static DEFINE_SPINLOCK(sta_hsem_irqs_lock);

/**
 * struct sta_hsem - this struct represents a single hsem instance
 * @id:			hsem instance
 * @hsem_reset:		semaphore reset code to use
 * @hsem_base:		base address of the hsem_rx register
 * @priv:		private data, owned by the underlying hsem lock caller
 * @hwlockcb:		Hsem requester callback called in interrupt context
 * @parent:		hsem_device structure which owns this lock
 */
struct sta_hsem {
	int				id;
	u32				hsem_reset;
	void __iomem			*hsem_base;
	void				*priv;
	void (*hsemcb)(void *priv, void *data);
	struct sta_hsem_device	*parent;
};

/**
 * struct sta_hsem_device - STA HSEM device data
 * @dev:		device to which it is attached
 * @hsem_reg_base:	base address of the HSEM register mapping region
 * @hsems:		array of hsem instance attributes
 */
struct sta_hsem_device {
	struct device		*dev;
	void __iomem		*hsem_reg_base;
	struct sta_hsem	*hsems;
};

/**
 * sta_hsem_lock_request() - platform-specific request for a hwspinlock
 * @id:		index of the specific hwspinlock that is requested
 *		if set to HSEM_ID_UNDEF, 1st free semaphore index will be
 *		returned.
 * @irqmode:	Enable interrupt generation when the hw semaphore is freed
 *		0			: No interrupt
 *		HSEM_INTR4		: Generate irq to R4
 *		HSEM_INTM3		: Generate irq to M3
 *		HSEM_INTR4 | HSEM_INTM3 : Generate irq to both R4 and M3
 * @priv:	will be passed over to @hsemcb
 * @hsemcb:	function will be called in HSEM ISR
 *
 * This function should be called by users of the hwspinlock module,
 * in order to assign them a specific hwspinlock.
 * Usually early board code will be calling this function in order to
 * reserve specific hwspinlock ids for predefined purposes.
 *
 * Should be called from a process context (might sleep)
 *
 * Returns the address of the assigned hwspinlock, or NULL on error
 */
struct hwspinlock *sta_hsem_lock_request(unsigned int id,
					 int irqmode, void *priv,
					 void (*hsemcb)(void *priv, void *data))
{
	struct hwspinlock *hwlock;

	if (irqmode && irqmode > (HSEM_INTR4 + HSEM_INTM3)) {
		hwlock = NULL;
		goto out;
	}

	if (id < MAX_SEMAPHORE)
		hwlock = hwspin_lock_request_specific(id);
	else
		hwlock = hwspin_lock_request();

	if (!hwlock)
		goto out;

	if (irqmode) {
		struct sta_hsem *hsem = hwlock->priv;
		struct sta_hsem_device *hsemdev = hsem->parent;
		unsigned long flags;
		u32 bits;

		/* Set INTSA & INTSB bits in the RESET_SEMAPHORE code */
		/* to generate IRQ as soon as the HSEM is freed */
		hsem->hsem_reset |= HSEM_INTSX(irqmode);

		/* Set user HSEM ISR callback */
		hsem->priv = priv;
		hsem->hsemcb = hsemcb;

		/* Enable IRQ for requested HSEM */
		spin_lock_irqsave(&sta_hsem_irqs_lock, flags);

		bits = readl(hsemdev->hsem_reg_base + HSEM_IMSCA)
			| BIT(hsem->id);
		writel(bits, hsemdev->hsem_reg_base + HSEM_IMSCA);

		spin_unlock_irqrestore(&sta_hsem_irqs_lock, flags);
	}

out:
	return hwlock;
}
EXPORT_SYMBOL(sta_hsem_lock_request);

/**
 * sta_hsem_lock_free() - free a specific hwspinlock
 * @hwlock: the specific hwspinlock to free
 *
 * This function mark @hwlock as free again.
 * Should only be called with an @hwlock that was retrieved from
 * an earlier call to sta_hsem_lock_request.
 *
 * Should be called from a process context (might sleep)
 *
 * Returns 0 on success, or an appropriate error code on failure
 */
int sta_hsem_lock_free(struct hwspinlock *hwlock)
{
	struct sta_hsem *hsem;
	struct sta_hsem_device *hsemdev;
	unsigned long flags;
	u32 bits;

	if (!hwlock)
		/* Nothing to do */
		return 0;

	hsem = hwlock->priv;
	hsemdev = hsem->parent;

	hsem->hsem_reset = RESET_SEMAPHORE;
	hsem->hsemcb = NULL;

	/* Disable IRQ for requested HSEM */
	spin_lock_irqsave(&sta_hsem_irqs_lock, flags);

	bits = readl(hsemdev->hsem_reg_base + HSEM_IMSCA) & ~BIT(hsem->id);
	writel(bits, hsemdev->hsem_reg_base + HSEM_IMSCA);

	spin_unlock_irqrestore(&sta_hsem_irqs_lock, flags);

	return hwspin_lock_free(hwlock);
}
EXPORT_SYMBOL(sta_hsem_lock_free);

/**
 * sta_hsem_irq_clr() - clear IRQ of a specific hwspinlock
 * @hwlock:	the specific hwspinlock to clear IRQ
 * @msk:	bits mask to apply on clear register
 *
 * Returns 0 on success, or an appropriate error code on failure
 */
int sta_hsem_irq_clr(struct hwspinlock *hwlock, u32 msk)
{
	struct sta_hsem *hsem;
	struct sta_hsem_device *hsemdev;
	unsigned long flags;
	u32 bits;

	if (!hwlock) {
		pr_err("%s: invalid hwlock\n", __func__);
		return -EINVAL;
	}
	hsem = hwlock->priv;
	hsemdev = hsem->parent;

	/* Disable IRQ for requested HSEM */
	spin_lock_irqsave(&sta_hsem_irqs_lock, flags);

	bits = readl(hsemdev->hsem_reg_base + HSEM_ICRA) | msk;
	writel(bits, hsemdev->hsem_reg_base + HSEM_ICRA);

	spin_unlock_irqrestore(&sta_hsem_irqs_lock, flags);

	return 0;
}
EXPORT_SYMBOL(sta_hsem_irq_clr);

/**
 * sta_hsem_irq_status() - return IRQ of a specific hwspinlock
 * @hwlock:	the specific hwspinlock
 * @irq:	HSEM Interrupt line (IRQ_A or B) requested
 *
 * Returns register content on success, or appropriate error code on failure
 */
u32 sta_hsem_irq_status(struct hwspinlock *hwlock, unsigned int irq)
{
	struct sta_hsem *hsem;
	struct sta_hsem_device *hsemdev;

	if (!hwlock || !irq || irq > HSEM_INTM3) {
		pr_err("%s: invalid hwlock or irq\n", __func__);
		return -EINVAL;
	}
	hsem = hwlock->priv;
	hsemdev = hsem->parent;

	/* Read IRQ status register prior to masking */
	if (irq == HSEM_INTR4)
		return readl(hsemdev->hsem_reg_base + HSEM_RISA);
	else /* irq == HSEM_INTM3 */
		return readl(hsemdev->hsem_reg_base + HSEM_RISB);
}
EXPORT_SYMBOL(sta_hsem_irq_status);

static irqreturn_t sta_hsem_irq_handler(int irq, void *data)
{
	struct sta_hsem_device *hsemdev =
					(struct sta_hsem_device *)data;
	struct sta_hsem *hsem = hsemdev->hsems;
	u32 bits, msk, n;

	bits = readl(hsemdev->hsem_reg_base + HSEM_MISA);

	if (unlikely(!bits))
		return IRQ_NONE;

	for (n = 0; bits; n++) {
		msk = BIT(n);
		if (bits & msk) {
			bits -= msk;
			hsem += n;
			if (readl(hsemdev->hsem_reg_base + HSEM_IMSCA) & msk) {
				if (hsem->hsemcb)
					hsem->hsemcb(hsem->priv, (void *)n);
			} else
				dev_warn(hsemdev->dev,
					 "Unexpected hsem interrupt\n");

			/* Clear IRQ */
			writel(msk, hsemdev->hsem_reg_base + HSEM_ICRA);
		}
	}
	return IRQ_HANDLED;
}

static int sta_hsem_hwspin_trylock(struct hwspinlock *lock)
{
	struct sta_hsem *hsem = lock->priv;

	writel(HSEM_MASTER_ID, hsem->hsem_base);

	/* get only first 4 bit and compare to masterID.
	 * if equal, we have the semaphore, otherwise
	 * someone else has it.
	 */
	return HSEM_MASTER_ID == (0x0F & readl(hsem->hsem_base));
}

static void sta_hsem_hwspin_unlock(struct hwspinlock *lock)
{
	struct sta_hsem *hsem = lock->priv;

	/* Release the lock by writing 0 to it */
	/* According to the INTSx bits value, an IRQ can also be generated */
	writel(hsem->hsem_reset, hsem->hsem_base);
}

/*
 * sta: what value is recommended here ?
 */
static void sta_hsem_hwspin_relax(struct hwspinlock *lock)
{
	ndelay(50);
}

static const struct hwspinlock_ops sta_hwspinlock_ops = {
	.trylock	= sta_hsem_hwspin_trylock,
	.unlock		= sta_hsem_hwspin_unlock,
	.relax		= sta_hsem_hwspin_relax,
};

static int sta_hsem_probe(struct platform_device *pdev)
{
	struct hwspinlock_pdata *pdata = pdev->dev.platform_data;
	struct hwspinlock_device *bank;
	struct hwspinlock *hwlock;
	struct resource *res;
	struct sta_hsem_device *hsemdev;
	struct sta_hsem *hsem, *hsemblk;
	void __iomem *io_base;
	int irq, base_id = 0;
	int i, ret, num_locks = MAX_SEMAPHORE;
#ifdef CONFIG_OF
	struct device_node *np = pdev->dev.of_node;

	if (np)
		of_property_read_u32(np, "st,base-id", &base_id);
	else
#endif
	if (pdata)
		base_id = pdata->base_id;
	else
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	io_base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (!io_base)
		return -ENOMEM;

	bank = devm_kzalloc(&pdev->dev, sizeof(*bank)
			+ num_locks * sizeof(*hwlock), GFP_KERNEL);
	if (!bank)
		return -ENOMEM;

	hsemdev = devm_kzalloc(&pdev->dev, sizeof(*hsemdev), GFP_KERNEL);
	if (!hsemdev)
		return -ENOMEM;

	hsem = devm_kzalloc(&pdev->dev, num_locks * sizeof(*hsem), GFP_KERNEL);
	if (!hsem)
		return -ENOMEM;

	hsemblk = hsem;

	irq = platform_get_irq(pdev, 0);
	ret = devm_request_irq(&pdev->dev, irq, sta_hsem_irq_handler,
			       IRQF_NO_SUSPEND, "sta_hsem", hsemdev);
	if (ret)
		return -EINVAL;

	sta_hsem_clk = devm_clk_get(&pdev->dev, "apb_pclk");
	if (IS_ERR(sta_hsem_clk))
		return PTR_ERR(sta_hsem_clk);

	ret = clk_prepare_enable(sta_hsem_clk);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, bank);

	/* clear all interrupts */
	writel(HSEM_RESET_IRQS, io_base + HSEM_ICRALL);

	for (i = 0, hwlock = &bank->lock[0];
			i < num_locks; i++, hwlock++, hsem++) {
		hsem->id = i;
		hsem->hsem_reset = RESET_SEMAPHORE;
		hsem->hsem_base = io_base + HSEM_REGISTER_OFFSET
							+ sizeof(u32) * i;
		hsem->hsemcb = NULL;
		hsem->parent = hsemdev;

		hwlock->priv = hsem;
	}
	hsemdev->dev = &pdev->dev;
	hsemdev->hsem_reg_base = io_base;
	hsemdev->hsems = hsemblk;

	/* no pm needed for HSem but required to comply with hwspinlock core */
	pm_runtime_enable(&pdev->dev);

	ret = hwspin_lock_register(bank, &pdev->dev, &sta_hwspinlock_ops,
				   base_id, num_locks);
	if (ret)
		pm_runtime_disable(&pdev->dev);

	return ret;
}

static int sta_hsem_remove(struct platform_device *pdev)
{
	struct hwspinlock_device *bank = platform_get_drvdata(pdev);
	struct sta_hsem *hsem = bank->lock[0].priv;
	struct sta_hsem_device *hsemdev = hsem->parent;
	int ret;

	/* clear all interrupts */
	writel(HSEM_RESET_IRQS, hsemdev->hsem_reg_base + HSEM_ICRALL);

	ret = hwspin_lock_unregister(bank);
	if (ret) {
		dev_err(&pdev->dev, "%s failed: %d\n", __func__, ret);
		return ret;
	}

	pm_runtime_disable(&pdev->dev);

	clk_disable_unprepare(sta_hsem_clk);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id sta_hsem_match[] = {
	{ .compatible = "st,sta-hsem" },
	{}
};
MODULE_DEVICE_TABLE(of, sta_hsem_match);
#endif

static struct platform_driver sta_hsem_driver = {
	.probe		= sta_hsem_probe,
	.remove		= sta_hsem_remove,
	.driver		= {
		.name	= "sta-hsem",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(sta_hsem_match),
	},
};

static int __init sta_hsem_init(void)
{
	return platform_driver_register(&sta_hsem_driver);
}

/* board init code might need to reserve hwspinlocks for predefined purposes */
postcore_initcall(sta_hsem_init);

static void __exit sta_hsem_exit(void)
{
	platform_driver_unregister(&sta_hsem_driver);
}
module_exit(sta_hsem_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hardware Spinlock driver for sta");
MODULE_AUTHOR("Philippe Langlais <philippe.langlais@st.com>");
MODULE_AUTHOR("Olivier Lebreton <olivier.lebreton@st.com>");
