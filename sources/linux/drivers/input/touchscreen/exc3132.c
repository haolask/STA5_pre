/*
 * Driver for EXC3132 Multiple Touch Controller
 *
 * Copyright (C) 2017 Jean-Nicolas Graux.
 *
 * based on egalax_ts.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * EXC3132 touch screen controller is a I2C based multiple
 * touch screen controller which supports 5 point multiple touches.
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>

#define REPORT_MODE_MTTOUCH 0x6
#define MAX_SUPPORT_POINTS 5
#define EVENT_DOWN_UP (0x1 << 0)
#define MAX_I2C_DATA_LEN 66

#define EXC3132_MAX_X	4096
#define EXC3132_MAX_Y	4096

#define FINGER_DATA_LEN 10

#define LEN_LSB 0
#define LEN_MSB 1

#define DATA_OFFSET 2
#define REPORT_ID 0
#define NUM_FINGERS 1

#define CONTACT_OFFSET 4
#define STATUS 0
#define FINGER_ID 1
#define X_LSB 2
#define X_MSB 3
#define Y_LSB 4
#define Y_MSB 5

struct exc3132 {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct gpio_desc *reset_gpio;
};

static irqreturn_t exc3132_interrupt(int irq, void *dev_id)
{
	struct exc3132 *ts = dev_id;
	struct input_dev *input_dev = ts->input_dev;
	struct i2c_client *client = ts->client;
	u8 buf[MAX_I2C_DATA_LEN];
	u8 *data, nb_fingers, finger_id;
	int ret, x, y, i, len;
	bool down;

	ret = i2c_master_recv(client, buf, MAX_I2C_DATA_LEN);
	if (ret != MAX_I2C_DATA_LEN) {
		dev_dbg(&client->dev, "i2c read returned: %d\n", ret);
		return IRQ_HANDLED;
	}

	len = (buf[LEN_MSB] << 8) | buf[LEN_LSB];
	if (len != MAX_I2C_DATA_LEN) {
		dev_dbg(&client->dev, "unexpected length: %d\n", len);
		return IRQ_HANDLED;
	}

	data = buf + DATA_OFFSET;
	if (data[REPORT_ID] != REPORT_MODE_MTTOUCH) {
		dev_dbg(&client->dev, "unknown report id: %d\n",
			data[REPORT_ID]);
		return IRQ_HANDLED;
	}

	nb_fingers = data[NUM_FINGERS];
	if (nb_fingers > MAX_SUPPORT_POINTS) {
		dev_dbg(&client->dev,
			"%d touch points reported, only %d are supported\n",
			nb_fingers, MAX_SUPPORT_POINTS);
		nb_fingers = MAX_SUPPORT_POINTS;
	}

	data = buf + CONTACT_OFFSET;
	for (i = 0; i < nb_fingers; i++) {
		down = data[STATUS] & EVENT_DOWN_UP;
		finger_id = data[FINGER_ID];
		if (finger_id > MAX_SUPPORT_POINTS) {
			dev_dbg(&client->dev, "invalid finger id: %d\n",
				finger_id);
			return IRQ_HANDLED;
		}

		x = (data[X_MSB] << 8) | data[X_LSB];
		y = (data[Y_MSB] << 8) | data[Y_LSB];

		input_mt_slot(input_dev, finger_id);
		input_mt_report_slot_state(input_dev, MT_TOOL_FINGER, down);

		dev_dbg(&client->dev, "%s id:%d x:%d y:%d",
			down ? "down" : "up", finger_id, x, y);

		if (down) {
			input_report_abs(input_dev, ABS_MT_POSITION_X, x);
			input_report_abs(input_dev, ABS_MT_POSITION_Y, y);
		}
		data += FINGER_DATA_LEN;
	}

	input_mt_sync_frame(input_dev);
	input_sync(input_dev);

	return IRQ_HANDLED;
}

static void exc3132_reset(struct exc3132 *ts)
{
	if (!ts->reset_gpio)
		return;

	gpiod_set_value_cansleep(ts->reset_gpio, 1);
	msleep(4);
	gpiod_set_value_cansleep(ts->reset_gpio, 0);
	msleep(40);
}

static int exc3132_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct exc3132 *ts;
	struct input_dev *input_dev;
	int error;

	ts = devm_kzalloc(&client->dev, sizeof(struct exc3132), GFP_KERNEL);
	if (!ts) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	ts->reset_gpio = devm_gpiod_get_optional(&client->dev, "reset",
						 GPIOD_OUT_LOW);
	if (IS_ERR(ts->reset_gpio)) {
		error = PTR_ERR(ts->reset_gpio);
		if (error != -EPROBE_DEFER)
			dev_err(&client->dev, "error getting reset gpio: %d\n",
				error);
		return error;
	}

	exc3132_reset(ts);

	input_dev = devm_input_allocate_device(&client->dev);
	if (!input_dev) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	ts->client = client;
	ts->input_dev = input_dev;

	input_dev->name = "Ampire exc3132 Touch Screen";
	input_dev->id.bustype = BUS_I2C;

	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
			     EXC3132_MAX_X, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
			     EXC3132_MAX_Y, 0, 0);

	input_mt_init_slots(input_dev, MAX_SUPPORT_POINTS,
			    INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);

	input_set_drvdata(input_dev, ts);

	error = devm_request_threaded_irq(&client->dev, client->irq, NULL,
					  exc3132_interrupt, IRQF_ONESHOT,
					  "exc3132", ts);
	if (error) {
		dev_err(&client->dev, "Failed to register interrupt\n");
		return error;
	}

	error = input_register_device(ts->input_dev);
	if (error)
		return error;

	i2c_set_clientdata(client, ts);
	dev_info(&client->dev, "probed\n");
	return 0;
}

static const struct i2c_device_id exc3132_id[] = {
	{ "exc3132", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, exc3132_id);

static const struct of_device_id exc3132_dt_ids[] = {
	{ .compatible = "ampire,exc3132" },
	{ /* sentinel */ }
};

static struct i2c_driver exc3132_driver = {
	.driver = {
		.name	= "exc3132",
		.owner	= THIS_MODULE,
		.of_match_table	= exc3132_dt_ids,
	},
	.id_table	= exc3132_id,
	.probe		= exc3132_probe,
};

module_i2c_driver(exc3132_driver);

MODULE_AUTHOR("Jean-Nicolas Graux");
MODULE_DESCRIPTION("Touchscreen driver for EXC3132 touch controller");
MODULE_LICENSE("GPL");
