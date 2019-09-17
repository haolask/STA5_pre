/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Benjamin Gaignard <benjamin.gaignard@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/driver.h>
#include <linux/slab.h>
#include <linux/time.h>

#include "g1.h"
#include "g1-cfg.h"

/* define irq registers offset and masks */
#define G1_HW_DEC_IRQ_OFFSET	(1 * 4)
#define G1_HW_PP_IRQ_OFFSET	(60 * 4)
#define G1_HW_DEC_IRQ_MASK	0x100
#define G1_HW_PP_IRQ_MASK	0x100
#define G1_HW_PP_PIPELINE_E_MASK	0x2

/* defines for registers */
#define G1_HW_ASIC_ID                  0

#define G1_HW_PP_SYNTH_CFG       100
#define G1_HW_DEC_SYNTH_CFG       50
#define G1_HW_DEC_SYNTH_CFG_2     54

#define G1_HW_PP_FUSE_CFG         99
#define G1_HW_DEC_FUSE_CFG        57

#define G1_HW_OFFSET_SWREG_0	0x000
#define G1_HW_OFFSET_SWREG_50	0x0C8
#define G1_HW_OFFSET_SWREG_52	0x0D0
#define G1_HW_OFFSET_SWREG_53	0x0D4
#define G1_HW_OFFSET_SWREG_54	0x0D8
#define G1_HW_OFFSET_SWREG_56	0x0E0
#define G1_HW_OFFSET_SWREG_57	0x0E4
#define G1_HW_OFFSET_SWREG_58	0x0E8
#define G1_HW_OFFSET_SWREG_75	0x12C
#define G1_HW_OFFSET_SWREG_76	0x130
#define G1_HW_OFFSET_SWREG_77	0x134
#define G1_HW_OFFSET_SWREG_78	0x138
#define G1_HW_OFFSET_SWREG_95	0x17C

#define to_hw(g1) (&(g1)->hw)

#define HW_PREFIX "[---:----]"

/*
 * debugfs file operation functions
 */

static int g1_hw_debugfs_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t g1_hw_debugfs_registers_dump(struct file *file,
					    char __user *user_buf,
					    size_t size, loff_t *ppos)
{
	struct g1_dev *g1 = file->private_data;
	struct g1_hw *hw = to_hw(g1);
	int ret;

	unsigned long int *regs;
	char *buf;
	unsigned int buffer_size =
	    sizeof("12345678 swreg100: 0x12345678\n") * hw->regs_size / 4;
	unsigned int count = 0;
	size_t left = buffer_size;
	int retparam = 0;
	int cnt = 0;
	char *cur;
	int i;

	regs = kzalloc(hw->regs_size, GFP_KERNEL);
	if (!regs)
		return -ENOMEM;

	buf = kzalloc(buffer_size, GFP_KERNEL);
	if (!buf) {
		kfree(regs);
		return -ENOMEM;
	}
	cur = buf;

	ret = pm_runtime_get_sync(g1->dev);
	if (ret < 0) {
		kfree(regs);
		kfree(buf);
		dev_err(g1->dev,
			"%s    pm_runtime_get_sync failed while dumping registers (err=%d)\n",
			HW_PREFIX, ret);
		return ret;
	}

	mutex_lock(&hw->protect_mutex);
	/* dump registers */
	for (i = 0; i < hw->regs_size; i += 4)
		regs[i / 4] = ioread32(hw->regs + i);

	mutex_unlock(&hw->protect_mutex);

	pm_runtime_put_autosuspend(g1->dev);

	/* write them in humain readable way */
	for (i = 0; i < hw->regs_size; i += 4) {
		unsigned long int val = regs[i / 4];

		if (!val)
			continue;

		cur += cnt;
		left -= cnt;
		retparam = snprintf(cur, left,
				    "%08x swreg%03d: 0x%08lx\n", i, i / 4,
				    val);
		cnt = (left > retparam ? retparam : left);
		dev_dbg(g1->dev, "%s     swreg%03d: 0x%08lx\n", HW_PREFIX,
			i / 4, val);
	}

	count = simple_read_from_buffer(user_buf, strlen(buf), ppos, buf,
					strlen(buf));
	kfree(regs);
	kfree(buf);

	return count;
}

static void g1_hw_performance_init(struct g1_hw_perf *perf)
{
	perf->min = (unsigned long)-1;
	perf->max = 0;
	perf->sum = 0;
	perf->n = 0;
	perf->circular_idx = 0;
}

static void g1_hw_performance_record(struct g1_hw_perf *perf,
				     unsigned long duration)
{
	if (duration < perf->min)
		perf->min = duration;
	if (duration > perf->max)
		perf->max = duration;
	perf->sum += duration;
	perf->n++;
	perf->duration[perf->circular_idx++] = duration;
	if (perf->circular_idx == PERF_CIRCULAR_ARRAY_SIZE)
		perf->circular_idx = 0;
}

static ssize_t g1_hw_debugfs_stats_dump(struct file *file,
					char __user *user_buf,
					size_t size, loff_t *ppos)
{
	struct g1_dev *g1 = file->private_data;
	struct g1_hw *hw = to_hw(g1);
	struct g1_hw_perf *perf = &hw->perf;
	char *buf;
	unsigned int buffer_size =
	    sizeof("Measured on 123456789 samples\n") * 4 +
	    sizeof("  Duration[1234]=0123456789 us\n") *
	    PERF_CIRCULAR_ARRAY_SIZE;
	unsigned int count = 0;
	char *cur;
	size_t left = buffer_size;
	int cnt = 0;
	int ret = 0;

	buf = kzalloc(buffer_size, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;
	cur = buf;

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left, "measured on %4ld samples\n", perf->n);
	cnt = (left > ret ? ret : left);

	if (perf->n) {
		int i;
		int absidx = perf->n - 1;
		int idx = perf->circular_idx - 1;

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, "min duration %lu us\n", perf->min);
		cnt = (left > ret ? ret : left);

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, "max duration %ld us\n", perf->max);
		cnt = (left > ret ? ret : left);

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, "avg duration %ld us\n",
			       perf->sum / perf->n);
		cnt = (left > ret ? ret : left);

		for (i = 0; i < PERF_CIRCULAR_ARRAY_SIZE; i++) {
			cur += cnt;
			left -= cnt;
			ret = snprintf(cur, left, "  duration[%4d]=%ld us\n",
				       absidx, perf->duration[idx]);
			cnt = (left > ret ? ret : left);
			absidx--;
			idx--;
			if (idx < 0)
				idx = PERF_CIRCULAR_ARRAY_SIZE - 1;
			if (absidx < 0)
				break;
		}
	}

	count = simple_read_from_buffer(user_buf, strlen(buf), ppos, buf,
					strlen(buf));
	kfree(buf);

	g1_hw_performance_init(&hw->perf);

	return count;
}

static const struct file_operations g1_hw_debugfs_fops = {
	.open = g1_hw_debugfs_open,
	.read = g1_hw_debugfs_registers_dump,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

static const struct file_operations g1_hw_stats_fops = {
	.open = g1_hw_debugfs_open,
	.read = g1_hw_debugfs_stats_dump,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

static ssize_t g1_hw_debugfs_config_dump(struct file *file,
					 char __user *user_buf,
					 size_t size, loff_t *ppos)
{
	struct g1_dev *g1 = file->private_data;
	char *buf;
	unsigned int buffer_size = 4500;
	unsigned int count = 0;

	buf = kzalloc(buffer_size, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	buf = g1_hw_cfg_dump_str(g1, buf, buffer_size);

	count = simple_read_from_buffer(user_buf, strlen(buf), ppos, buf,
					strlen(buf));

	kfree(buf);

	return count;
}

static const struct file_operations g1_hw_config_fops = {
	.open = g1_hw_debugfs_open,
	.read = g1_hw_debugfs_config_dump,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

/*
 * g1_hw_read_only_register
 * @reg_offset : the offset of reg to check
 * return true if the register is a read only register
 */
static bool g1_hw_read_only_register(int reg_offset)
{
	switch (reg_offset) {
	case G1_HW_OFFSET_SWREG_0:
	case G1_HW_OFFSET_SWREG_50:
	case G1_HW_OFFSET_SWREG_52:
	case G1_HW_OFFSET_SWREG_53:
	case G1_HW_OFFSET_SWREG_54:
	case G1_HW_OFFSET_SWREG_56:
	case G1_HW_OFFSET_SWREG_57:
	case G1_HW_OFFSET_SWREG_58:
	case G1_HW_OFFSET_SWREG_75:
	case G1_HW_OFFSET_SWREG_76:
	case G1_HW_OFFSET_SWREG_77:
	case G1_HW_OFFSET_SWREG_78:
		return true;
	}
	if (reg_offset >= G1_HW_OFFSET_SWREG_95)
		return true;

	return false;
}

int g1_hw_pm_suspend(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);

	dev_dbg(g1->dev, "%s     > %s\n", HW_PREFIX, __func__);

	if (hw->clk)
		clk_disable_unprepare(hw->clk);

	if (hw->regulator)
		regulator_disable(hw->regulator);

	dev_dbg(g1->dev, "%s     < %s\n", HW_PREFIX, __func__);

	return 0;
}

int g1_hw_pm_resume(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);

	dev_dbg(g1->dev, "%s     > %s\n", HW_PREFIX, __func__);

	if (hw->regulator)
		if (regulator_enable(hw->regulator))
			dev_warn(g1->dev, "%s     failed to enable regulator\n",
				 HW_PREFIX);

	if (hw->clk)
		if (clk_prepare_enable(hw->clk))
			dev_warn(g1->dev, "%s     failed to enable clock\n",
				 HW_PREFIX);

	dev_dbg(g1->dev, "%s     < %s\n", HW_PREFIX, __func__);

	return 0;
}

int g1_hw_suspend(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);

	dev_dbg(g1->dev, " > %s\n", __func__);

	if (hw->suspended_state)
		return 0;

	mutex_lock(&hw->protect_mutex);
	/* this mutex insures that no hardware treatment in on going.
	 * keep this mutex locked to forbid access to hardware from
	 * this point.
	 */
	pm_runtime_disable(g1->dev);
	hw->suspended_state = 1;
	dev_dbg(g1->dev, " < %s\n", __func__);
	return 0;
}

int g1_hw_resume(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);

	dev_dbg(g1->dev, " > %s\n", __func__);
	if (g1->state != G1_STATE_FATAL_ERROR) {
		/* After this point, hardware is accessible */
		pm_runtime_enable(g1->dev);
		hw->suspended_state = 0;
	}
	mutex_unlock(&hw->protect_mutex);
	dev_dbg(g1->dev, " < %s\n", __func__);
	return 0;
}

int g1_hw_run(struct g1_ctx *ctx, struct g1_hw_regs *regs)
{
	int i;
	int ret = 0;
	int pending_pp = 0;
	int pending_dec = 0;
	struct timeval start_time, stop_time;
	struct g1_dev *g1 = ctx->dev;
	struct g1_hw *hw = to_hw(g1);

	dev_dbg(g1->dev, "%s     > %s\n", ctx->name, __func__);

	/* reserve hardware */
	mutex_lock(&hw->protect_mutex);

	if (g1->state == G1_STATE_FATAL_ERROR) {
		dev_err(g1->dev,
			"%s fatal error detected - abort IP treatment\n",
			ctx->name);
		mutex_unlock(&hw->protect_mutex);
		return -1;
	}

	/* enable power */
	ret = pm_runtime_get_sync(g1->dev);
	if (ret < 0) {
		dev_err(g1->dev,
			"%s     pm_runtime_get_sync failed while starting hardware (err=%d)\n",
			ctx->name, ret);
		mutex_unlock(&hw->protect_mutex);
		return ret;
	}
	dev_dbg(g1->dev, "%s     hardware reserved\n", ctx->name);

	/* enable irq */
	enable_irq(hw->irq);

	dev_dbg(g1->dev, "%s     decode: %d reg to write\n",
		ctx->name, regs->n);

	/* write data into g1 registers */
	for (i = 0; i < regs->n; i++) {
		/* do not write read-only registers */
		if (!g1_hw_read_only_register((regs->offset[i]))) {
			/* detect start of pp and wait for it
			 * FIXME to rework
			 */
			if ((regs->offset[i] == G1_HW_PP_IRQ_OFFSET) &&
			    (regs->data[i] & 1)) {
				pending_pp = 1;
				do_gettimeofday(&start_time);
				dev_dbg(g1->dev, "%s     fired pp\n",
					ctx->name);
			}
			/* detect start of decoder and wait for it
			 * FIXME to rework
			 */
			if ((regs->offset[i] == G1_HW_DEC_IRQ_OFFSET) &&
			    (regs->data[i] & 1)) {
				pending_dec = 1;
				do_gettimeofday(&start_time);
				dev_dbg(g1->dev, "%s     fired dec\n",
					ctx->name);
			}
			iowrite32(regs->data[i], hw->regs + regs->offset[i]);
		}
	}

	/* now waiting for hardware completion if needed */
	if (pending_pp && !wait_for_completion_timeout
	    (&hw->pp_interrupt, msecs_to_jiffies(2000))) {
		dev_warn(g1->dev, "%s     post proc timeout\n", ctx->name);
	}

	if (pending_dec && !wait_for_completion_timeout
	    (&hw->dec_interrupt, msecs_to_jiffies(2000))) {
		dev_warn(g1->dev, "%s     decoder timeout\n", ctx->name);
	}

	/* to get performance information */
	if (pending_dec || pending_pp) {
		long duration;

		do_gettimeofday(&stop_time);
		duration = stop_time.tv_usec - start_time.tv_usec;
		if (duration < 0)
			duration = 1000000 + duration;
		g1_hw_performance_record(&hw->perf, (unsigned long)duration);
	}

	/* read back all register */
	for (i = 0; i < hw->regs_size / 4; i++) {
		regs->data[i] = ioread32(hw->regs + 4 * i);
		regs->offset[i] = 4 * i;

		if (i == G1_HW_DEC_IRQ_OFFSET / 4) {
			dev_dbg(g1->dev,
				"%s     get decoder interrupt control/status reg[%d] : %x\n",
				ctx->name, i, regs->data[i]);
		}
		if (i == G1_HW_PP_IRQ_OFFSET / 4) {
			/* turn pipeline OFF so it doesn't interfer
			 * with other instances
			 */
			regs->data[i] &= ~G1_HW_PP_PIPELINE_E_MASK;
			iowrite32(regs->data[i],
				  (hw->regs + G1_HW_PP_IRQ_OFFSET));
			dev_dbg(g1->dev,
				"%s     get postproc interrupt control/status reg[%d] : %x\n",
				ctx->name, i, regs->data[i]);
		}
	}

	/* disable irq */
	disable_irq(hw->irq);

	/* disable power */
	pm_runtime_mark_last_busy(g1->dev);
	pm_runtime_put_autosuspend(g1->dev);

	/* unreserve hardware */
	mutex_unlock(&hw->protect_mutex);
	dev_dbg(g1->dev, "%s     hardware released\n", ctx->name);

	dev_dbg(g1->dev, "%s     < %s\n", ctx->name, __func__);
	return 0;
}

/*
 * g1_hw_interrupt - Interrupt handler
 */
static irqreturn_t g1_hw_interrupt(int irq, void *dev)
{
	struct g1_dev *g1 = dev;
	struct g1_hw *hw = to_hw(g1);
	unsigned int irq_dec;
	unsigned int irq_pp;

	/* there is two possible irq source decoder and post-processor */
	irq_dec = ioread32(hw->regs + G1_HW_DEC_IRQ_OFFSET);
	irq_pp = ioread32(hw->regs + G1_HW_PP_IRQ_OFFSET);

	if (irq_dec & G1_HW_DEC_IRQ_MASK) {
		/* clear decoder irq */
		iowrite32(irq_dec & (~G1_HW_DEC_IRQ_MASK),
			  (hw->regs + G1_HW_DEC_IRQ_OFFSET));
		complete(&hw->dec_interrupt);
		dev_dbg(g1->dev, "%s     get an interrupt from decoder, status=0x%x\n",
			HW_PREFIX,
			irq_dec);
	}

	if (irq_pp & G1_HW_PP_IRQ_MASK) {
		/* clear post-processor irq */
		iowrite32(irq_pp & (~G1_HW_PP_IRQ_MASK),
			  (hw->regs + G1_HW_PP_IRQ_OFFSET));
		complete(&hw->pp_interrupt);
		dev_dbg(g1->dev,
			"%s     get an interrupt from post-processor, status=0x%x\n",
			HW_PREFIX,
			irq_pp);
	}

	return IRQ_HANDLED;
}

static int g1_hw_read_config(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);
	int ret;

	dev_dbg(g1->dev, "%s     > %s\n", HW_PREFIX, __func__);

	ret = pm_runtime_get_sync(g1->dev);
	if (ret < 0) {
		dev_err(g1->dev, "%s pm_runtime_get_sync failed while reading config (err=%d)\n",
			HW_PREFIX, ret);
		return ret;
	}

	/* ASIC id */
	hw->asic_id = ioread32(hw->regs + 4 * G1_HW_ASIC_ID);
	if (hw->asic_id == 0) {
		dev_err(g1->dev, "%s    unexpected 0 value for ASIC identifier\n",
			HW_PREFIX);
		return -ENODEV;
	}

	/* DEC configuration */
	hw->dec_cfg = ioread32(hw->regs + 4 * G1_HW_DEC_SYNTH_CFG);
	hw->dec_cfg2 = ioread32(hw->regs + 4 * G1_HW_DEC_SYNTH_CFG_2);
	hw->dec_fuse = ioread32(hw->regs + 4 * G1_HW_DEC_FUSE_CFG);

	/* PP configuration */
	hw->pp_cfg = ioread32(hw->regs + 4 * G1_HW_PP_SYNTH_CFG);
	hw->pp_fuse = ioread32(hw->regs + 4 * G1_HW_PP_FUSE_CFG);

	pm_runtime_put_autosuspend(g1->dev);

	dev_dbg(g1->dev, "%s     < %s\n", HW_PREFIX, __func__);
	return 0;
}

int g1_hw_probe(struct g1_dev *g1)
{
	int ret = 0;
	struct g1_hw *hw = to_hw(g1);
	struct resource *regs;
	void __iomem *iomem;
	int irq;
	unsigned char str[200] = "";

	dev_dbg(g1->dev, "%s     > %s\n", HW_PREFIX, __func__);

	/* get a memory region for mmio */
	regs = platform_get_resource(g1->pdev, IORESOURCE_MEM, 0);
	if (IS_ERR_OR_NULL(regs)) {
		dev_err(g1->dev, "%s     couldn't reserve mmio region\n",
			HW_PREFIX);
		return PTR_ERR(regs);
	}

	dev_dbg(g1->dev, "%s     probe g1 at %x\n", HW_PREFIX, regs->start);
	/* remap g1 registers in kernel space */
	iomem = devm_ioremap_nocache(g1->dev, regs->start,
				     regs->end - regs->start + 1);
	if (IS_ERR_OR_NULL(iomem)) {
		dev_err(g1->dev, "%s     can't ioremap registers region\n",
			HW_PREFIX);
		return PTR_ERR(iomem);
	}
	hw->regs = iomem;
	hw->regs_size = regs->end - regs->start + 1;

	/* retrieve irq number from board resources */
	irq = platform_get_irq(g1->pdev, 0);
	if (irq <= 0) {
		dev_err(g1->dev, "%s     no irq defined\n", HW_PREFIX);
		ret = irq;
		return ret;
	}

	/* request irq */
	dev_dbg(g1->dev, "%s     request irq %d\n", HW_PREFIX, irq);
	ret = devm_request_irq(g1->dev, irq, g1_hw_interrupt,
			       IRQF_SHARED, dev_name(g1->dev), (void *)g1);
	if (ret) {
		dev_err(g1->dev, "%s     can't register irq 0x%x\n", HW_PREFIX,
			irq);
		return ret;
	}
	hw->irq = irq;

	disable_irq(hw->irq);

	/* Get clock resource */
	hw->clk = devm_clk_get(g1->dev, "g1");
	if (IS_ERR(hw->clk)) {
		dev_warn(g1->dev, "%s     can't get clock\n", HW_PREFIX);
		hw->clk = NULL;
	}

	/* get regulator */
	hw->regulator = devm_regulator_get(g1->dev, "v-g1");
	if (IS_ERR(hw->regulator)) {
		dev_warn(g1->dev, "%s     can't get regulator\n", HW_PREFIX);
		hw->regulator = NULL;
	}

	/* initialisation of the protection mutex */
	mutex_init(&hw->protect_mutex);

	/* initialisation of completion signal */
	init_completion(&hw->dec_interrupt);
	init_completion(&hw->pp_interrupt);

	/* create debugfs file */
	hw->debugfs_dir = debugfs_create_dir("hw", g1->debugfs_dir);
	hw->debugfs_regs =
	    debugfs_create_file("regs", 0400, hw->debugfs_dir, g1,
				&g1_hw_debugfs_fops);

	hw->debugfs_stats =
	    debugfs_create_file("stats", 0400, hw->debugfs_dir, g1,
				&g1_hw_stats_fops);

	hw->debugfs_config =
	    debugfs_create_file("config", 0400, hw->debugfs_dir, g1,
				&g1_hw_config_fops);

	/* init pm_runtime used for power management */
	pm_runtime_set_autosuspend_delay(g1->dev, G1_HW_AUTOSUSPEND_DELAY_MS);
	pm_runtime_use_autosuspend(g1->dev);
	pm_runtime_set_suspended(g1->dev);
	pm_runtime_enable(g1->dev);
	hw->suspended_state = 0;
	/* read hw config */
	ret = g1_hw_read_config(g1);
	if (ret)
		return ret;

	/* build config & fuse */
	g1_hw_cfg_build_fuse(g1);
	g1_hw_cfg_build_config(g1);

	/* clear performance  */
	g1_hw_performance_init(&hw->perf);

	dev_info(g1->dev, "found g1 video decoder: asic id=0x%x, %s\n",
		 hw->asic_id, g1_hw_cfg_summary_str(g1, str, sizeof(str)));

	dev_dbg(g1->dev, "%s     < %s\n", HW_PREFIX, __func__);
	return 0;
}

void g1_hw_remove(struct g1_dev *g1)
{
	struct g1_hw *hw = to_hw(g1);

	dev_dbg(g1->dev, "%s     > %s\n", HW_PREFIX, __func__);

	/* remove debugfs file */
	debugfs_remove(hw->debugfs_config);
	debugfs_remove(hw->debugfs_regs);
	debugfs_remove(hw->debugfs_stats);
	debugfs_remove(hw->debugfs_dir);

	/* disable interrupt */
	disable_irq(hw->irq);

	pm_runtime_put_autosuspend(g1->dev);
	pm_runtime_disable(g1->dev);

	dev_dbg(g1->dev, "%s     < %s\n", HW_PREFIX, __func__);
}
