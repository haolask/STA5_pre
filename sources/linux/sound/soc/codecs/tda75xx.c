/*
 * ALSA SoC driver for STMicroelectronics TDA7569 4 channels amplifier
 *
 * Copyright (C) ST Microelectronics 2014
 *
 * Author:  Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *          for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <sound/soc.h>
#include <linux/of_gpio.h>

#include "tda75xx.h"

enum tda_model {
	TDA7569,
	TDA7577,
};

enum instruction_registers {
	IB1,
	IB2,
};

enum data_registers {
	DB1,
	DB2,
	DB3,
	DB4,
};

/* This struct is used to save the context */
struct tda75xx {
	enum tda_model id;
	int channels;
	unsigned char ireg[TDA75XX_IBNUM];
	unsigned char dreg[TDA75XX_DBNUM];
	int sby_gpio;
	struct mutex mutex; /* protect power on/off */
	struct i2c_client *client;
	int gain;
	int clip;
	int power_on;
};

static int tda75xx_i2c_read(struct tda75xx *tda75xx)
{
	int err;
	struct i2c_client *client = tda75xx->client;

	struct i2c_msg msgs[] = {
		{
		 .addr = client->addr,
		 .flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		 .len = tda75xx->channels,
		 .buf = tda75xx->dreg,
		 },
	};

	err = i2c_transfer(client->adapter, msgs, 1);

	if (err != 1) {
		dev_err(&client->dev, "read transfer error: %d\n", err);
		return -EIO;
	}

	return 0;
}

static int tda75xx_i2c_write(struct tda75xx *tda75xx, int len)
{
	int err;
	struct i2c_client *client = tda75xx->client;

	struct i2c_msg msgs[] = {
		{
		 .addr = client->addr,
		 .flags = (client->flags & I2C_M_TEN),
		 .len = len,
		 .buf = tda75xx->ireg,
		 },
	};

	err = i2c_transfer(client->adapter, msgs, 1);

	if (err != 1) {
		dev_err(&client->dev, "write transfer error: %d\n", err);
		return -EIO;
	}

	return 0;
}

static int tda75xx_power_status(struct tda75xx *tda75xx)
{
	if (tda75xx->id == TDA7569)
		return (tda75xx->dreg[DB3] & DB3_STANDBY_STATUS);
	else
		return (tda75xx->ireg[IB2] & IB2_STANDBY_OFF);
}

static void print_setting_status(struct tda75xx *tda75xx)
{
	struct device *dev = &tda75xx->client->dev;

	dev_info(dev, "ireg %x,%x\n", tda75xx->ireg[0], tda75xx->ireg[1]);
	if (tda75xx->ireg[IB1] & IB1_CD_10)
		dev_info(dev, "Clip detection 10%%\n");
	else
		dev_info(dev, "Clip detection 2%%\n");
	if (tda75xx->ireg[IB1] & IB1_REAR_UNMUTE)
		dev_info(dev, "Rear Unmuted\n");
	else
		dev_info(dev, "Rear Muted\n");
	if (tda75xx->ireg[IB1] & IB1_FRONT_UNMUTE)
		dev_info(dev, "Front Unmuted\n");
	else
		dev_info(dev, "Front Muted\n");
	if (tda75xx->ireg[IB1] & IB1_REAR_GAIN_26DB)
		dev_info(dev, "Rear gain 26DB\n");
	else
		dev_info(dev, "Rear gain 16DB\n");
	if (tda75xx->ireg[IB1] & IB1_FRONT_GAIN_26DB)
		dev_info(dev, "Front gain 26DB\n");
	else
		dev_info(dev, "Front gain 16DB\n");
	if (tda75xx->ireg[IB1] & IB1_OFFSET_DETECTION_ENABLE)
		dev_info(dev, "Offset detection enabled\n");
	else
		dev_info(dev, "Offset detection disabled\n");
	if (tda75xx->ireg[IB1] & IB1_DIAGNOSTIC_ENABLE)
		dev_info(dev, "Diagnostic enabled\n");
	else
		dev_info(dev, "Diagnostic disabled\n");
	if (tda75xx->ireg[IB1] & IB1_MUTE_THRESHOLD_HIGH)
		dev_info(dev, "Voltage Mute threshold high\n");
	else
		dev_info(dev, "Voltage Mute threshold low\n\n");

	if (tda75xx->ireg[IB2] & IB2_LEFT_AMPLI_EFFICIENT)
		dev_info(dev, "Left channel in efficiancy mode\n");
	else
		dev_info(dev, "Left channel in normal mode\n");
	if (tda75xx->ireg[IB2] & IB2_RIGHT_AMPLI_EFFICIENT)
		dev_info(dev, "Right channel in efficiancy mode\n");
	else
		dev_info(dev, "Right channel in normal mode\n");
	if (tda75xx->ireg[IB2] & IB2_CURRENT_DIAG_ENABLED)
		dev_info(dev, "Current diagnosic disabled\n");
	else
		dev_info(dev, "Current diagnosic enabled\n");
	if (tda75xx->ireg[IB2] & IB2_LINE_DRIVER_DIAG)
		dev_info(dev, "Line mode diagnostic\n");
	else
		dev_info(dev, "Power mode diagnostic\n");
	if (tda75xx->ireg[IB2] & IB2_STANDBY_OFF)
		dev_info(dev, "Standby Off\n");
	else
		dev_info(dev, "Standby On\n");
	if (tda75xx->ireg[IB2] & IB2_FAST_MUTING_TIME)
		dev_info(dev, "Fast muting time\n");
	else
		dev_info(dev, "Normal muting time\n");
	if (tda75xx->ireg[IB2] & IB2_CURRENT_THRESHOLD_LOW)
		dev_info(dev, "Current threshold low\n");
	else
		dev_info(dev, "Current threshold high\n");
}

static int print_diagnostic_status(struct tda75xx *tda75xx)
{
	struct device *dev = &tda75xx->client->dev;
	int ret = 0, i;

	ret = tda75xx_i2c_read(tda75xx);
	if (ret) {
		dev_err(dev, "I2C error\n");
		return ret;
	}

	for (i = 0; i < tda75xx->channels; i++) {
		if (tda75xx->dreg[i] & CHX_CURRENT_THRESHOLD)
			dev_info(dev, "Current threshold on CH%d\n", i + 1);
		if (tda75xx->dreg[i] & CHX_SHORT_LOAD)
			dev_info(dev, "Short Load detected on CH%d\n", i + 1);
		if (tda75xx->dreg[i] & CHX_OFFSET)
			dev_info(dev, "Short Load detected on CH%d\n", i + 1);
		if (tda75xx->dreg[i] & CHX_SHORT_VCC)
			dev_info(dev, "Short VCC detected on CH%d\n", i + 1);
		if (tda75xx->dreg[i] & CHX_SHORT_GND)
			dev_info(dev, "Short GND detected on CH%d\n", i + 1);
	}

	if (tda75xx->dreg[DB1] & DB1_THERMAL_WARNING1)
		dev_info(dev, "Thermal warning 160 C\n");
	if (tda75xx->dreg[DB1] & DB1_DIAG_CYCLE_TERMINATED)
		dev_info(dev, "Diag cycle terminated\n");
	if (tda75xx->dreg[DB2] & DB2_OFFSET_ACTIVE)
		dev_info(dev, "Offest diag activated\n");
	if (tda75xx->id == TDA7577) {
		if (tda75xx->dreg[DB2] & DB2_CURRENT_ACTIVE)
			dev_info(dev, "Current diag activated\n");
	}
	if (tda75xx->id == TDA7569) {
		if (tda75xx->dreg[DB3] & DB3_STANDBY_STATUS)
			dev_info(dev, "Ampli is on\n");
		if (tda75xx->dreg[DB3] & DB3_DIAG_STATUS)
			dev_info(dev, "Diagnostic activated\n");
		if (tda75xx->dreg[DB4] & DB4_THERMAL_WARNING2)
			dev_info(dev, "Thermal warning 145 C\n");
		if (tda75xx->dreg[DB4] & DB4_THERMAL_WARNING3)
			dev_info(dev, "Thermal warning 125 C\n");
	}
	return 0;
}

static ssize_t tda75xx_diag_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct tda75xx *tda75xx = (struct tda75xx *)dev_get_drvdata(dev);

	print_diagnostic_status(tda75xx);

	return sprintf(buf, "%s:\nireg %x,%x\ndreg: %02x,%02x,%02x,%02x\n",
		       tda75xx->id == TDA7569 ? "TDA7569" : "TDA7577",
		       tda75xx->ireg[0], tda75xx->ireg[1],
		       tda75xx->dreg[0], tda75xx->dreg[1],
		       tda75xx->dreg[2], tda75xx->dreg[3]);
}

static ssize_t tda75xx_diag_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	struct tda75xx *tda75xx = (struct tda75xx *)dev_get_drvdata(dev);
	unsigned int ib1, ib2;
	int ret;

	ret = sscanf(buf, "%x,%x", &ib1, &ib2);
	if (ret >= 1)
		tda75xx->ireg[0] = (unsigned char)ib1;
	if (ret >= 2)
		tda75xx->ireg[1] = (unsigned char)ib2;
	tda75xx_i2c_write(tda75xx, 2);
	print_setting_status(tda75xx);
	return count;
}

static DEVICE_ATTR(diag, 0644, tda75xx_diag_show, tda75xx_diag_store);

static int tda75xx_initialize(struct tda75xx *tda75xx)
{
	int ret;

	ret = tda75xx_i2c_read(tda75xx);
	if (ret)
		return ret;

	if (tda75xx_power_status(tda75xx))
		/* Already powered on*/
		return 0;

	tda75xx->ireg[IB1] = IB1_MUTE_THRESHOLD_LOW
			   | IB1_DIAGNOSTIC_ENABLE
			   | IB1_OFFSET_DETECTION_ENABLE
			   | ((tda75xx->gain == 26) ?
			      (IB1_FRONT_GAIN_26DB | IB1_REAR_GAIN_26DB)
			    : (IB1_FRONT_GAIN_16DB | IB1_REAR_GAIN_16DB))
			   | IB1_FRONT_MUTE | IB1_REAR_MUTE
			   | ((tda75xx->clip == 2) ? IB1_CD_2 : IB1_CD_10);

	tda75xx->ireg[IB2] = IB2_CURRENT_THRESHOLD_HIGH
			   | IB2_NORMAL_MUTING_TIME | IB2_STANDBY_ON
			   | IB2_POWER_AMPLIFIER_DIAG
			   | IB2_CURRENT_DIAG_ENABLED
			   | IB2_RIGHT_AMPLI_NORMAL | IB2_LEFT_AMPLI_NORMAL;

	ret = tda75xx_i2c_write(tda75xx, 2);
	if (ret)
		return ret;

	return tda75xx_i2c_read(tda75xx);
}

static int tda75xx_power(struct tda75xx *tda75xx, int val)
{
	int ret = 0;

	mutex_lock(&tda75xx->mutex);
	if (val) {
		tda75xx->ireg[IB2] |= IB2_STANDBY_OFF;
		ret = tda75xx_i2c_write(tda75xx, 2);
		if (ret)
			goto power_failure;
		tda75xx->ireg[IB1] |= IB1_FRONT_UNMUTE;
		tda75xx->ireg[IB1] |= IB1_REAR_UNMUTE;
		ret = tda75xx_i2c_write(tda75xx, 1);
		if (ret)
			goto power_failure;
	} else {
		tda75xx->ireg[IB1] &= ~IB1_FRONT_UNMUTE;
		tda75xx->ireg[IB1] &= ~IB1_REAR_UNMUTE;
		ret = tda75xx_i2c_write(tda75xx, 1);
		tda75xx->ireg[IB2] &= ~IB2_STANDBY_OFF;
		ret = tda75xx_i2c_write(tda75xx, 2);
		if (ret)
			goto power_failure;
	}

	ret = tda75xx_i2c_read(tda75xx);
	if (ret)
		goto power_failure;

	ret = tda75xx_power_status(tda75xx);

power_failure:
	mutex_unlock(&tda75xx->mutex);
	return ret;
}

static const char * const enable_mode[] = {"Off", "On"};
static const struct soc_enum enable_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(enable_mode),
			    enable_mode);

static int tda5xx_power_mode_get(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct tda75xx *tda75xx =
		(struct tda75xx *)dev_get_drvdata(component->dev);

	ucontrol->value.enumerated.item[0] = tda75xx_power_status(tda75xx);
	return 0;
}

static int tda5xx_power_mode_put(struct snd_kcontrol *kcontrol,
				 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct tda75xx *tda75xx =
		(struct tda75xx *)dev_get_drvdata(component->dev);
	int val = ucontrol->value.enumerated.item[0];

	return tda75xx_power(tda75xx, val);
}

static const struct snd_kcontrol_new tda7569_controls[] = {
	SOC_ENUM_EXT("4-Ampli", enable_enum,
		     tda5xx_power_mode_get, tda5xx_power_mode_put),
};

static const struct snd_kcontrol_new tda7577_controls[] = {
	SOC_ENUM_EXT("2-Ampli", enable_enum,
		     tda5xx_power_mode_get, tda5xx_power_mode_put),
};

static struct snd_soc_codec_driver tda7569_codec_driver = {
	.component_driver = {
		.controls = tda7569_controls,
		.num_controls = ARRAY_SIZE(tda7569_controls),
	}
};

static struct snd_soc_codec_driver tda7577_codec_driver = {
	.component_driver = {
		.controls = tda7577_controls,
		.num_controls = ARRAY_SIZE(tda7577_controls),
	}
};

struct snd_soc_dai_ops tda75xx_dai_ops = {
};

static struct snd_soc_dai_driver tda75xx_dai_driver[] = {
	{
		.id = 0,
		.ops = &tda75xx_dai_ops,
	},
};

static int tda75xx_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct device *dev;
	struct tda75xx *tda75xx;
	struct device_node *np = client->dev.of_node;
	int ret;
	struct snd_soc_codec_driver *tda75xx_codec_driver;
	int masked_addr;

	dev = &client->dev;

	tda75xx = devm_kzalloc(&client->dev,
			       sizeof(struct tda75xx), GFP_KERNEL);
	if (!tda75xx)
		return -ENOMEM;

	dev_set_drvdata(dev, tda75xx);
	tda75xx->client = client;
	i2c_set_clientdata(client, tda75xx);

	if (np) {
		tda75xx->sby_gpio = of_get_named_gpio(np, "sby-gpio", 0);
	} else {
		dev_err(dev, "Platform data not set\n");
		ret = -EINVAL;
		goto err_gpio;
	}
	ret = of_property_read_u32(np, "gain", &tda75xx->gain);
	if (!ret)
		tda75xx->gain = 26;

	ret = of_property_read_u32(np, "clip", &tda75xx->clip);
	if (!ret)
		tda75xx->clip = 2;

	mutex_init(&tda75xx->mutex);

	tda75xx_dai_driver->name = np->name;

	if (tda75xx->sby_gpio >= 0) {
		dev_info(dev, "standby GPIO (%d)\n", tda75xx->sby_gpio);
		ret = devm_gpio_request(dev, tda75xx->sby_gpio,
					"tda75xx enable");
		if (ret < 0) {
			dev_err(dev, "Failed to request standby GPIO (%d)\n",
				tda75xx->sby_gpio);
			goto err_gpio;
		}
		gpio_direction_output(tda75xx->sby_gpio, 0);
	}

	masked_addr = (client->addr & 0xFC);
	switch (masked_addr) {
	case 0x6C:
		dev_info(dev, "TDA7569\n");
		tda75xx->channels = 4;
		tda75xx_codec_driver = &tda7569_codec_driver;
		tda75xx->id = TDA7569;
		break;
	case 0x68:
		dev_info(dev, "TDA7577\n");
		tda75xx->channels = 2;
		tda75xx_codec_driver = &tda7577_codec_driver;
		tda75xx->id = TDA7577;
		break;
	default:
		dev_err(dev, "Unknown TDA75XX model (%d)\n", tda75xx->id);
		return -EINVAL;
	}

	ret = tda75xx_initialize(tda75xx);
	if (ret != 0)
		goto err_gpio;

	dev_info(dev, "%s: %02x,%02x,%02x,%02x\n",
		 tda75xx->id == TDA7569 ? "TDA7569" : "TDA7569",
		 tda75xx->dreg[0], tda75xx->dreg[1],
		 tda75xx->dreg[2], tda75xx->dreg[3]);

	ret = snd_soc_register_codec(dev,
				     tda75xx_codec_driver,
				     tda75xx_dai_driver,
				     ARRAY_SIZE(tda75xx_dai_driver));
	if (ret)
		dev_err(dev, "Failed to register codec!\n");

	ret = device_create_file(dev, &dev_attr_diag);
	if (ret)
		dev_info(dev, "error creating sysfs files\n");

	return 0;

err_gpio:
	tda75xx->client = NULL;
	return ret;
}

static int tda75xx_remove(struct i2c_client *client)
{
	struct tda75xx *tda75xx = i2c_get_clientdata(client);

	snd_soc_unregister_codec(&client->dev);

	tda75xx_power(tda75xx, 0);
	return 0;
}

static const struct i2c_device_id tda75xx_id[] = {
	{ "tda7569", TDA7569 },
	{ "tda7577", TDA7577 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tda75xx_id);

static const struct of_device_id tda75xx_of_match[] = {
	{ .compatible = "st,tda7569", },
	{ .compatible = "st,tda7577" },
	{},
};
MODULE_DEVICE_TABLE(of, tda75xx_of_match);

#ifdef CONFIG_PM_SLEEP
static int tda75xx_suspend(struct device *dev)
{
	struct tda75xx *tda75xx = (struct tda75xx *)dev_get_drvdata(dev);

	tda75xx->power_on = !!(tda75xx->ireg[IB2] & IB2_STANDBY_OFF);
	tda75xx_power(tda75xx, 0);
	return 0;
}

static int tda75xx_resume(struct device *dev)
{
	struct tda75xx *tda75xx = (struct tda75xx *)dev_get_drvdata(dev);

	gpio_direction_output(tda75xx->sby_gpio, 0);
	tda75xx_initialize(tda75xx);
	tda75xx_power(tda75xx, tda75xx->power_on);
	return 0;
}
#endif

static const struct dev_pm_ops tda75xx_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(tda75xx_suspend, tda75xx_resume)
};

static struct i2c_driver tda75xx_i2c_driver = {
	.driver = {
		.name = "tda75xx",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(tda75xx_of_match),
		.pm = &tda75xx_pm_ops,
	},
	.probe = tda75xx_probe,
	.remove = tda75xx_remove,
	.id_table = tda75xx_id,
};

module_i2c_driver(tda75xx_i2c_driver);

MODULE_AUTHOR("Gian Antonio Sampietro <gianantonio.sampietro@st.com>");
MODULE_DESCRIPTION("TDA75xx amplifier driver");
MODULE_LICENSE("GPL");
