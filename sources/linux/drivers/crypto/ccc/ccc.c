/*
 * ST C3 Channel Controller v3.0
 *
 * Author: Gerald Lejeune <gerald.lejeune@st.com>
 *
 * Copyright (C) 2016 STMicroelectronics Limited
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#include "ccc.h"

#define ARST BIT(16)
static inline void ccc_reset(struct ccc_controller *c3)
{
	writel_relaxed(ARST, c3->base + C3_SYS + SYS_SCR);
	udelay(1);
}

#define OP_ID_LSB 23
struct operation ccc_get_opcode(unsigned char code, unsigned int nr_param, ...)
{
	struct operation op;

	/* Flow type instructions make use of channel index 0. */
	switch (code) {
	case STOP:
		op.code = code << OP_ID_LSB;
		/* Caller must take care of word0 if status is requested. */
		op.wn = nr_param ? 1 : 0;
		op.code |= op.wn << WN_LSB;
		break;
	case NOP:
		/* Fall through. */
	default:
		/* Can't fail. */
		op.code = NOP << OP_ID_LSB;
		op.wn = 0;
		break;
	}
	return op;
}
EXPORT_SYMBOL(ccc_get_opcode);

static struct ccc_dispatcher *
get_available_dispatcher(struct ccc_controller *ccc)
{
	unsigned char nr_disp = ccc->nr_dispatchers;
	struct ccc_dispatcher *disp = ccc->dispatchers;

	while (nr_disp--) {
		if (try_wait_for_completion(&disp->available))
			return disp;
		disp++;
	}
	return NULL;
}

static inline unsigned int get_program_epilog_len(void)
{
	/*
	 * Biggest epilog size is the size of STOP instruction with parameter.
	 */
	return 2 * sizeof(unsigned int);
}

struct ccc_dispatcher *ccc_get_dispatcher(struct ccc_controller *ccc)
{
	struct ccc_dispatcher *disp;

	do {
		disp = get_available_dispatcher(ccc);
		if (!disp) {
			dev_dbg(&ccc->dev, "No available dispatcher yet\n");
			wait_for_completion(&ccc->available);
		}
	} while (!disp);

	if (!ccc_is_dispatcher_available(disp)) {
		dev_err(&ccc->dev, "No available dispatcher\n");
		return NULL;
	}

	disp->pc = disp->program.for_cpu;
	disp->program_free_len = disp->program.size - get_program_epilog_len();
	reinit_completion(&disp->execution);
	return disp;
}
EXPORT_SYMBOL(ccc_get_dispatcher);

void ccc_put_dispatcher(struct ccc_dispatcher *disp)
{
	struct ccc_controller *ccc = disp->controller;

	complete(&disp->available);
	complete(&ccc->available);
}
EXPORT_SYMBOL(ccc_put_dispatcher);

static inline void append_ucode(struct ccc_dispatcher *disp, unsigned int ucode)
{
	struct device *dev = &disp->controller->dev;

	if (!disp->program_free_len) {
		dev_err(dev, "No space left for microcode\n");
		return;
	}
	*disp->pc++ = ucode;
	disp->program_free_len -= sizeof(ucode);
}

void ccc_program(struct ccc_dispatcher *d, unsigned int opcode,
		 unsigned char nr_param, ...)
{
	va_list params;

	if (!d)
		return;

	append_ucode(d, opcode);
	va_start(params, nr_param);
	while (nr_param) {
		unsigned int param = va_arg(params, unsigned int);

		append_ucode(d, param);
		nr_param--;
	}
	va_end(params);
}
EXPORT_SYMBOL(ccc_program);

#define BERR BIT(29)
#define CERR BIT(26)
#define CBSY BIT(25)
#define CDNX BIT(24)
static int report_dispatcher_error(struct ccc_dispatcher *disp)
{
	struct device *dev = &disp->controller->dev;
	unsigned int scr = ccc_read_id_scr(disp);
	int ret = 0;

	if (scr & BERR) {
		ret = -EFAULT;
		dev_err(dev, "BERR ");
	}
	if (scr & CERR) {
		ret = -ENOEXEC;
		dev_err(dev, "CERR ");
	}
	if (scr & CBSY) {
		ret = -EBUSY;
		dev_err(dev, "CBSY ");
	}
	if (scr & CDNX) {
		ret = -ENODEV;
		dev_err(dev, "CDNX ");
	}

	if (!ret) {
		ret = -EIO;
		dev_err(dev, "Error handling error ");
	}

	dev_err(dev, "%d\n", ret);
	return ret;
}

static int check_dispatcher_state(struct ccc_dispatcher *disp)
{
	struct device *dev = &disp->controller->dev;
	unsigned int scr = ccc_read_id_scr(disp);
	unsigned int ids = ccc_get_dispatcher_status(scr);
	int ret = 0;

	switch (ids) {
	case S_ERROR:
		dev_err(dev, "Dispatcher in error state\n");
		/*
		 * Caller does not need to reset dispatcher as Error state
		 * will be exited at next program execution.
		 */
		ret = report_dispatcher_error(disp);
		break;
	case S_NOT_PRESENT:
		dev_err(dev, "Dispatcher not present\n");
		ret = -ENODEV;
		break;
	case S_BUSY:
		dev_err(dev, "Dispatcher busy\n");
		ret = -EBUSY;
		break;
	case S_IDLE:
		break;
	default:
		dev_err(dev, "Error handling error\n");
		ret = -EFAULT;
		break;
	}
	return ret;
}

static int report_channel_error(struct ccc_channel *channel)
{
	struct device *dev = &channel->controller->dev;
	unsigned int scr = ccc_read_channel_scr(channel);
	unsigned int error = scr & channel->error_mask;

	if (error & BERR)
		dev_err(dev, "%s: BERR\n", channel->name);
	if (error & DERR)
		dev_err(dev, "%s: DERR\n", channel->name);
	if (error & PERR)
		dev_err(dev, "%s: PERR\n", channel->name);
	if (error & IERR)
		dev_err(dev, "%s: IERR\n", channel->name);
	if (error & AERR)
		dev_err(dev, "%s: AERR\n", channel->name);
	if (error & OERR)
		dev_err(dev, "%s: OERR\n", channel->name);
	if (error)
		return -EIO;
	return 0;
}

int ccc_check_channel_state(struct ccc_channel *channel)
{
	struct device *dev = &channel->controller->dev;
	unsigned int scr = ccc_read_channel_scr(channel);
	unsigned int cs = ccc_get_channel_status(scr);
	int ret = 0;

	switch (cs) {
	case S_ERROR:
		dev_err(dev, "Channel in error state\n");
		/*
		 * Caller does not need to reset channel as Error state will be
		 * exited at next program execution.
		 */
		ret = report_channel_error(channel);
		break;
	case S_NOT_PRESENT:
		dev_err(dev, "%s: Channel not present\n", channel->name);
		ret = -ENODEV;
		break;
	case S_BUSY:
		dev_err(dev, "%s: Channel busy\n", channel->name);
		ret = -EBUSY;
		break;
	case S_IDLE:
		break;
	default:
		dev_err(dev, "%s: Error handling error\n", channel->name);
		ret = -EFAULT;
		break;
	}

	return ret;
}

int ccc_run_program(struct ccc_dispatcher *disp)
{
	ccc_dma_sync_for_device(&disp->controller->dev, &disp->program,
				DMA_TO_DEVICE);
	ccc_enable_interrupts(disp);
	ccc_set_instruction_pointer(disp);
	wait_for_completion(&disp->execution);
	return check_dispatcher_state(disp);
}
EXPORT_SYMBOL(ccc_run_program);

static irqreturn_t ccc_isr(int irq, void *data)
{
	struct ccc_controller *c3 = (struct ccc_controller *)data;
	struct ccc_dispatcher *disp = c3->dispatchers;
	unsigned char i = 0;
	unsigned int scr = ccc_read_sys_scr(c3);
	unsigned int isdn = ccc_get_sys_scr_isdn(scr);

	isdn &= c3->en_dispatchers;
	for (i = c3->nr_dispatchers; i--;) {
		if (isdn & BIT(i)) {
			ccc_disable_interrupts(disp);
			ccc_clear_interrupt(disp);
			complete(&disp->execution);
		}
		disp++;
	}
	return IRQ_HANDLED;
}

static void ccc_remove_instruction_dispatchers(struct platform_device *pdev,
					       struct ccc_controller *c3)
{
	struct ccc_dispatcher *disp;
	int i;

	disp = c3->dispatchers;
	if (disp) {
		for (i = c3->nr_dispatchers; i--; ) {
			if (disp->program.for_cpu)
				dma_pool_free(c3->pool, disp->program.for_cpu,
					      disp->program.for_dev);
			disp++;
		}
	}
}

static int ccc_probe_instruction_dispatchers(struct platform_device *pdev,
					     struct ccc_controller *c3)
{
	struct device_node *np = pdev->dev.of_node;
	struct ccc_dispatcher *disp;
	unsigned int scr;
	struct resource *prog;
	resource_size_t prog_size;
	int i, ret = 0;

	if (of_property_read_u8(np, "enabled-dispatchers",
				&c3->en_dispatchers)) {
		dev_err(&pdev->dev, "No dispatcher enabled\n");
		ret = -ENODEV;
		goto exit;
	}

	c3->nr_dispatchers = __sw_hweight8(c3->en_dispatchers);
	if (c3->nr_dispatchers > MAX_NR_DISPATCHERS) {
		dev_err(&pdev->dev, "Inconsistent number of dispatchers\n");
		ret = -EINVAL;
		goto exit;
	}

	prog = platform_get_resource_byname(pdev, IORESOURCE_MEM, "c3_prog");
	if (!prog) {
		dev_err(&pdev->dev, "Failed to get coherent memory\n");
		ret = -ENODEV;
		goto exit;
	}

	if (dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32))) {
		dev_err(&pdev->dev, "No suitable DMA available\n");
		ret = -ENODEV;
		goto exit;
	}

#define POOL_NAME "programs"
	prog_size = min_t(resource_size_t, resource_size(prog),
			  MAX_PROGRAM_SIZE);
	if (resource_size(prog) > MAX_PROGRAM_SIZE)
		dev_warn(&pdev->dev, "Truncate DMA pool: " POOL_NAME "\n");

	c3->pool = dmam_pool_create(POOL_NAME, &pdev->dev, prog_size,
				    CCC_ALIGN_SIZE, 0);
	if (!c3->pool) {
		dev_err(&pdev->dev, "No suitable DMA pool for: "
			POOL_NAME "\n");
		ret = -ENOMEM;
		goto exit;
	}

	c3->dispatchers = devm_kzalloc(&pdev->dev, c3->nr_dispatchers *
				       sizeof(struct ccc_dispatcher),
				       GFP_KERNEL);
	if (!c3->dispatchers) {
		ret = -ENOMEM;
		goto exit;
	}

	/*
	 * Assume that enabled Instruction Dispatchers may not be consecutive.
	 */
	init_completion(&c3->available);
	scr = ccc_read_sys_scr(c3);
	disp = c3->dispatchers;
	for (i = MAX_NR_DISPATCHERS; i--; ) {
		if (!(c3->en_dispatchers & BIT(i)))
			continue;
		if (!ccc_is_indexed_dispatcher_present(scr, i)) {
			dev_err(&pdev->dev, "Dispatcher %d does not exist\n",
				i);
			ret = -EINVAL;
			goto exit;
		}

		disp->base = devm_ioremap(&pdev->dev,
					  (phys_addr_t)(c3->physbase
							+ C3_ID(i)),
					  ID_SIZE);
		if (IS_ERR(disp->base)) {
			dev_err(&pdev->dev,
				"Failed to reserve dispatcher mem region\n");
			ret = PTR_ERR(disp->base);
			goto exit;
		}

		disp->program.for_cpu = dma_pool_alloc(c3->pool, GFP_KERNEL,
						       &disp->program.for_dev);
		if (!disp->program.for_cpu) {
			ret = -ENOMEM;
			goto exit;
		}
		disp->program.size = prog_size;
		disp->index = i;
		disp->pc = 0;
		disp->program_free_len = 0;
		init_completion(&disp->execution);
		init_completion(&disp->available);
		complete(&disp->available);
		complete(&c3->available);
		disp->controller = c3;

#ifdef ENABLE_SSC
		ccc_enable_single_step_command(disp);
#endif

		disp++;
	}

exit:
	if (ret)
		ccc_remove_instruction_dispatchers(pdev, c3);
	return ret;
}

static int ccc_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct ccc_controller *c3;
	struct resource *res;
	unsigned int scr, ver, ifcr;
	int ret;

	if (!np) {
		dev_err(&pdev->dev, "No DT found\n");
		return -EINVAL;
	}

	c3 = devm_kzalloc(&pdev->dev, sizeof(struct ccc_controller),
			  GFP_KERNEL);
	if (!c3)
		return -ENOMEM;

	c3->pdev = pdev;

	platform_set_drvdata(pdev, c3);

	if (of_modalias_node(np, c3->name, sizeof(c3->name)) < 0) {
		dev_err(&pdev->dev, "Modalias failure on %s\n", np->full_name);
		return -EINVAL;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "c3_base");
	if (!res) {
		dev_err(&pdev->dev, "C3 base resource not found\n");
		return -ENODEV;
	}

	c3->physbase = res->start;
	c3->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(c3->base)) {
		dev_err(&pdev->dev, "Failed to reserve memory region %p\n",
			res);
		return PTR_ERR(c3->base);
	}

	ret = platform_get_irq_byname(pdev, "c3_irq");
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to find IRQ resource\n");
		return -ENODEV;
	}

	ret = devm_request_threaded_irq(&pdev->dev, ret, NULL, ccc_isr,
					IRQF_ONESHOT, dev_name(&pdev->dev), c3);
	if (ret) {
		dev_err(&pdev->dev, "IRQ request failed\n");
		return ret;
	}

	c3->hclk = devm_clk_get(&pdev->dev, "c3_hclk");
	if (IS_ERR(c3->hclk)) {
		dev_err(&pdev->dev, "Failed to get c3_hclk clock\n");
		return PTR_ERR(c3->hclk);
	}

	ret = clk_prepare_enable(c3->hclk);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable c3_hclk clock\n");
		return ret;
	}

	ccc_reset(c3);
	ccc_disable_hif_memory(c3);

	ret = ccc_probe_instruction_dispatchers(pdev, c3);
	if (ret)
		return ret;

	c3->dev.of_node = np;
	c3->owner = THIS_MODULE;

	ret = ccc_register_controller(c3);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register controller\n");
		return ret;
	}

	dev_info(&c3->dev, "%s registered\n", c3->name);
	ver = ccc_read_sys_ver(c3);
	dev_info(&c3->dev, "Hardware Version: %d.%d.%d\n",
		 ccc_get_sys_ver_v(ver),
		 ccc_get_sys_ver_r(ver),
		 ccc_get_sys_ver_s(ver));
	scr = ccc_read_sys_scr(c3);
	dev_dbg(&c3->dev, "Clear Interrupts on SYS_SCR Read: %s\n",
		ccc_get_sys_scr_cisr(scr) ? "yes" : "no");
	dev_dbg(&c3->dev, "SIF Endianness: %s\n",
		ccc_get_sys_scr_endian(scr) ? "little" : "big");
	dev_dbg(&c3->dev, "%d Instruction Dispatcher%s enabled\n",
		c3->nr_dispatchers, c3->nr_dispatchers > 1 ? "s" : "");
	dev_dbg(&c3->dev, "HIF memory size: %dB\n",
		ccc_read_hif_memory_size(c3));
	ifcr = ccc_read_hif_ifcr(c3);
	dev_dbg(&c3->dev, "HIF Instructions Dispatchers Endianness: %s\n",
		ccc_get_hif_ifcr_id_end(ifcr) ? "little" : "big");
	dev_dbg(&c3->dev, "HIF Channels Endianness: %s\n",
		ccc_get_hif_ifcr_ch_end(ifcr) ? "little" : "big");
	return ret;
}

static int ccc_remove(struct platform_device *pdev)
{
	struct ccc_controller *c3;

	c3 = platform_get_drvdata(pdev);
	ccc_remove_instruction_dispatchers(pdev, c3);
	clk_disable_unprepare(c3->hclk);
	ccc_delete_controller(c3);
	return 0;
}

static const struct of_device_id ccc_match_id[] = {
	{ .compatible = "st,ccc-v3.0" },
	{},
};

MODULE_DEVICE_TABLE(of, ccc_match_id);

static struct platform_driver ccc_controller_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "ccc",
		.of_match_table = ccc_match_id,
	},
	.probe = ccc_probe,
	.remove = ccc_remove,
};
module_platform_driver(ccc_controller_driver);

MODULE_AUTHOR("Gerald Lejeune <gerald.lejeune@st.com>");
MODULE_DESCRIPTION("ST C3 Channel Controller driver");
MODULE_LICENSE("GPL");
