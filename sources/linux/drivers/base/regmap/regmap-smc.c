/*
 * Register map access API - Secure Monitor Call support
 *
 * Copyright (c) 2018, STMicroelectronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/arm-smccc.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <soc/sta/regmap_smc.h>

#include "internal.h"

struct regmap_smc_context {
	struct device *dev;
	resource_size_t start;
	int val_bits;
};

static int regmap_smc_regbits_check(size_t reg_bits)
{
	switch (reg_bits) {
	case 8:
	case 16:
	case 32:
#ifdef CONFIG_64BIT
	case 64:
#endif
		return 0;
	default:
		return -EINVAL;
	}
}

static int regmap_smc_get_min_stride(size_t val_bits)
{
	int min_stride;

	switch (val_bits) {
	case STA_REGMAP_SMC_VALBITS_8:
		/* The core treats 0 as 1 */
		min_stride = 0;
		return 0;
	case STA_REGMAP_SMC_VALBITS_16:
		min_stride = 2;
		break;
	case STA_REGMAP_SMC_VALBITS_32:
		min_stride = 4;
		break;
#ifdef CONFIG_64BIT
	case STA_REGMAP_SMC_VALBITS_64:
		min_stride = 8;
		break;
#endif
	default:
		return -EINVAL;
	}

	return min_stride;
}

static int regmap_smc_write(void *context, unsigned int reg, unsigned int val)
{
	struct regmap_smc_context *ctx = context;
	struct arm_smccc_res res;

	/*
	 * SMC write command:
	 * r0: command id: STA_REGMAP_SMC_WRITE
	 * r1: register length
	 * r2: physical address
	 * r3: val
	 */
	arm_smccc_smc(STA_REGMAP_SMC | STA_REGMAP_SMC_WRITE, ctx->val_bits,
		      ctx->start + reg, val, 0, 0, 0, 0, &res);
	if ((int)res.a0) {
		dev_err(ctx->dev, "failed to write @0x%x, err %lu\n",
			ctx->start + reg, res.a0);
		return -EINVAL;
	}

	return 0;
}

static int regmap_smc_read(void *context, unsigned int reg, unsigned int *val)
{
	struct regmap_smc_context *ctx = context;
	struct arm_smccc_res res;

	/*
	 * SMC read command:
	 * r0: command id: STA_REGMAP_SMC_READ
	 * r1: register length
	 * r2: physical address
	 *
	 * res.a0: error if not null
	 * res.a1: returned value
	 */
	arm_smccc_smc(STA_REGMAP_SMC | STA_REGMAP_SMC_READ, ctx->val_bits,
		      ctx->start + reg, 0, 0, 0, 0, 0, &res);
	if ((int)res.a0) {
		dev_err(ctx->dev, "failed to read @0x%x, err %lu\n",
			ctx->start + reg, res.a0);
		return -EINVAL;
	}
	*val = res.a1;

	return 0;
}

static int regmap_smc_update_bits(void *context, unsigned int reg,
				  unsigned int mask, unsigned int val)
{
	struct regmap_smc_context *ctx = context;
	struct arm_smccc_res res;

	/*
	 * SMC update bits command:
	 * r0: command id: STA_REGMAP_SMC_UPDATE_BITS
	 * r1: register length
	 * r2: physical address
	 * r3: bit mask
	 * r4: bit value
	 * res.a0: error if not null
	 */
	arm_smccc_smc(STA_REGMAP_SMC | STA_REGMAP_SMC_UPDATE_BITS,
		      ctx->val_bits, ctx->start + reg, mask, val,
		      0, 0, 0, &res);
	if ((int)res.a0) {
		dev_err(ctx->dev, "failed to update @ 0x%x, err %lu\n",
			ctx->start + reg, res.a0);
		return -EINVAL;
	}

	return 0;
}

static void regmap_smc_free_context(void *context)
{
	kfree(context);
}

static struct regmap_bus regmap_smc = {
	.reg_read = regmap_smc_read,
	.reg_write = regmap_smc_write,
	.reg_update_bits = regmap_smc_update_bits,
	.free_context = regmap_smc_free_context,
	.reg_format_endian_default = REGMAP_ENDIAN_LITTLE,
	.val_format_endian_default = REGMAP_ENDIAN_LITTLE,
};

static struct regmap_smc_context *regmap_smc_gen_context(struct device *dev,
					const struct resource *res,
					const struct regmap_config *config)
{
	struct regmap_smc_context *ctx;
	int min_stride;
	int ret;

	ret = regmap_smc_regbits_check(config->reg_bits);
	if (ret)
		return ERR_PTR(ret);

	if (config->pad_bits)
		return ERR_PTR(-EINVAL);

	min_stride = regmap_smc_get_min_stride(config->val_bits);
	if (min_stride < 0)
		return ERR_PTR(min_stride);

	if (config->reg_stride < min_stride)
		return ERR_PTR(-EINVAL);

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return ERR_PTR(-ENOMEM);

	ctx->dev = dev;
	ctx->start = res->start;

	switch (regmap_get_val_endian(dev, &regmap_smc, config)) {
	case REGMAP_ENDIAN_DEFAULT:
	case REGMAP_ENDIAN_LITTLE:
#ifdef __LITTLE_ENDIAN
	case REGMAP_ENDIAN_NATIVE:
#endif
		switch (config->val_bits) {
		case STA_REGMAP_SMC_VALBITS_8:
		case STA_REGMAP_SMC_VALBITS_16:
		case STA_REGMAP_SMC_VALBITS_32:
#ifdef CONFIG_64BIT
		case STA_REGMAP_SMC_VALBITS_64:
#endif
			ctx->val_bits = config->val_bits;
			break;
		default:
			ret = -EINVAL;
			goto err_free;
		}
		break;
	default:
		ret = -EINVAL;
		goto err_free;
	}

	return ctx;

err_free:
	kfree(ctx);

	return ERR_PTR(ret);
}

struct regmap *__regmap_init_smc(struct device *dev,
				 const struct resource *res,
				 const struct regmap_config *config,
				 struct lock_class_key *lock_key,
				 const char *lock_name)
{
	struct regmap_smc_context *ctx;

	ctx = regmap_smc_gen_context(dev, res, config);
	if (IS_ERR(ctx))
		return ERR_CAST(ctx);

	return __regmap_init(dev, &regmap_smc, ctx, config,
			     lock_key, lock_name);
}
EXPORT_SYMBOL_GPL(__regmap_init_smc);

struct regmap *__devm_regmap_init_smc(struct device *dev,
				      const struct resource *res,
				      const struct regmap_config *config,
				      struct lock_class_key *lock_key,
				      const char *lock_name)
{
	struct regmap_smc_context *ctx;

	ctx = regmap_smc_gen_context(dev, res, config);
	if (IS_ERR(ctx))
		return ERR_CAST(ctx);

	return __devm_regmap_init(dev, &regmap_smc, ctx, config,
				  lock_key, lock_name);
}
EXPORT_SYMBOL_GPL(__devm_regmap_init_smc);

MODULE_DESCRIPTION("Regmap Secure Monitor Call Module");
MODULE_LICENSE("GPL v2");
