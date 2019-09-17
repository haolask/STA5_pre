/*
 * Embedded iPod Authentication Coprocessor 2.0C driver
 *
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author: Olivier Clergeaud <olivier.clergeaud@st.com> for STMicroelectronics.
 * Author: Guang Xian Yao <guang-xian.yao@st.com> for STMicroelectronics.
 *
 * License type: GPLv2
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEBUG 1
#include <linux/device.h>

#define IAC20_DEVICE_VERSION 0x00
#define IAC20_FW_VERSION     0x01
#define IAC20_AP_MAJOR       0x02
#define IAC20_AP_MINOR       0x03
#define IAC20_DEVICE_ID      0x04
#define IAC20_ERROR_CODE     0x08
#define IAC20_CERTIFICAT_LEN 0x30
#define IAC20_CERTIFICAT_DAT 0x31

#define IAP_AUTH_CP_CERT_PAGE_LEN 128

#define IAC20_MAGIC	'i'
#define IAC20_SET_ADDR	_IOW(IAC20_MAGIC, 0, u8)

static int iac20_major;
static struct class *iac20_class;

#define IAC20_BUF_LEN 128
struct iac20_data {
	struct i2c_client *client;
	struct gpio_desc *reset_gpio;
	int reg;
	int len;
	u8 buf[IAC20_BUF_LEN];
	u8 addr;
};

struct iac20f {
	struct iac20_data *iac20;
	u8 buf[IAC20_BUF_LEN];
};

static int iac20_i2c_read(struct iac20_data *iac20, u8 reg, u8 len, void *data)
{
	struct i2c_client *client = iac20->client;
	struct i2c_msg msgs[1];
	int err;

	msgs[0].addr = client->addr;
	msgs[0].flags = (client->flags & I2C_M_TEN);
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	/* send the write cycle */
	err = i2c_transfer(client->adapter, msgs, 1);

	if (err != 1) {
		/* retry a second time */
		err = i2c_transfer(client->adapter, msgs, 1);

		if (err != 1) {
			dev_err(&client->dev, "write transfer error: %d\n",
				err);
			return -EIO;
		}
	}

	/* send the read cycle */
	msgs[0].addr = client->addr;
	msgs[0].flags = (client->flags & I2C_M_TEN) | I2C_M_RD;
	msgs[0].len = len;
	msgs[0].buf = data;

	mdelay(3);
	err = i2c_transfer(client->adapter, msgs, 1);

	if (err != 1) {
		/* retry a second time */
		err = i2c_transfer(client->adapter, msgs, 1);

		if (err != 1) {
			dev_err(&client->dev, "read transfer error: %d\n", err);
			return -EIO;
		}
	}

	return err;
}

static int iac20_i2c_write(struct iac20_data *iac20, u8 reg, u8 len, void *data)
{
	struct i2c_client *client = iac20->client;
	struct i2c_msg msgs[1];

	int err, i;
	u8 wbuf[IAC20_BUF_LEN + 1];

	if (len > IAC20_BUF_LEN)
		return -ENOMEM;

	wbuf[0] = reg;
	memcpy(&wbuf[1], data, len);

	for (i = 0; i < len + 1; i++)
		dev_dbg(&client->dev, "Write buffer = 0x%02x\n", wbuf[i]);

	msgs[0].addr = client->addr;
	msgs[0].flags = (client->flags & I2C_M_TEN);
	msgs[0].len = len + 1;
	msgs[0].buf = wbuf;

	/* send the write cycle */
	mdelay(3);
	err = i2c_transfer(client->adapter, msgs, 1);

	if (err != 1) {
		/* retry a second time */
		err = i2c_transfer(client->adapter, msgs, 1);

		dev_err(&client->dev, "write transfer error: %d\n", err);
		return -EIO;
	}

	return err;
}

static int iac20_get_device_info(struct iac20_data *iac20)
{
	struct device *dev = &iac20->client->dev;
	u8 buf[IAC20_BUF_LEN];
	int err;

	dev_dbg(dev, "iac20_get_device_info\n");

	err = iac20_i2c_read(iac20, IAC20_DEVICE_VERSION, 1, &buf);
	if (err == -EIO)
		return err;
	dev_dbg(dev, "Device version is 0x%02x\n", buf[0]);

	err = iac20_i2c_read(iac20, IAC20_FW_VERSION, 1, &buf);
	if (err == -EIO)
		return err;
	dev_dbg(dev, "Firmware version 0x%02x\n", buf[0]);

	err = iac20_i2c_read(iac20, IAC20_AP_MAJOR, 2, &buf);
	if (err == -EIO)
		return err;
	dev_dbg(dev, "Authentication Protocol Version %d.%d\n", buf[0], buf[1]);

	err = iac20_i2c_read(iac20, IAC20_DEVICE_ID, 4, &buf);
	if (err == -EIO)
		return err;
	dev_dbg(dev, "Device ID 0x%02x%02x%02x%02x\n",
		buf[0], buf[1], buf[2], buf[3]);

	err = iac20_i2c_read(iac20, IAC20_DEVICE_ID, 1, &buf);
	if (err == -EIO)
		return err;
	dev_dbg(dev, "Error code 0x%02x\n", buf[0]);

	return 0;
}

static int iac20_get_certificate(struct iac20_data *iac20)
{
	struct device *dev = &iac20->client->dev;
	int err;
	u16 certlen;
	u8 buf[IAP_AUTH_CP_CERT_PAGE_LEN];
	u8 certpageaddr = IAP_AUTH_CP_CERT_PAGE_LEN;
	u8 i;

	dev_dbg(dev, "iac20_get_certificate\n");

	err = iac20_i2c_read(iac20, IAC20_CERTIFICAT_LEN, 2, &buf);
	if (err == -EIO)
		return err;
	certlen = (buf[0] << 8) | buf[1];

	dev_dbg(dev, "Certificat length is %d\n", certlen);

	for (i = 0; i < certlen; i += IAP_AUTH_CP_CERT_PAGE_LEN) {
		err = iac20_i2c_read(iac20, certpageaddr,
				     IAP_AUTH_CP_CERT_PAGE_LEN, &buf);

		if (err == -EIO)
			return err;
	}
}

static ssize_t show_device_info(struct device *dev,
				struct device_attribute *attr, char *buffer)
{
	struct iac20_data *iac20 = (struct iac20_data *)dev_get_drvdata(dev);

	iac20_get_device_info(iac20);

	return sprintf(buffer, "End of Diagnostic\n");
}

static ssize_t set_reg_read(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct iac20_data *iac20 = (struct iac20_data *)dev_get_drvdata(dev);
	int err;

	err = sscanf(buf, "%d%d", &iac20->reg, &iac20->len);
	if (err < 0)
		dev_err(dev, "Wrong arguments");

	dev_dbg(dev, "Register address = 0x%02x\n", iac20->reg);
	dev_dbg(dev, "Register length  = %d\n", iac20->len);

	return count;
}

static ssize_t show_reg_read(struct device *dev,
			     struct device_attribute *attr, char *buffer)
{
	struct iac20_data *iac20 = (struct iac20_data *)dev_get_drvdata(dev);
	int err;
	u8 buf[IAC20_BUF_LEN];
	u8 i;

	err = iac20_i2c_read(iac20, iac20->reg, iac20->len, &buf);
	if (err == -EIO) {
		dev_err(dev, "Unable to read register addredd (0x%02X)\n",
			iac20->reg);
		return -EIO;
	}

	dev_dbg(dev, "Register address = 0x%02x\n", iac20->reg);
	dev_dbg(dev, "Register length  = %d\n", iac20->len);

	for (i = 0; i < iac20->len; i++) {
		dev_dbg(dev, "0x%02x", buf[i]);
		sprintf(&buffer[2 * i], "%02x", buf[i]);
		dev_dbg(dev, "%c%c", buffer[2 * i], buffer[2 * i + 1]);
	}

	return (2 * iac20->len);
}

static ssize_t set_reg_write(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct iac20_data *iac20 = (struct iac20_data *)dev_get_drvdata(dev);
	int err;
	int i;
	char val1[2 * IAC20_BUF_LEN];
	u8 val2[3];

	err = sscanf(buf, "%d%d%s", &iac20->reg, &iac20->len, val1);

	if (err < 0)
		dev_err(dev, "Wrong arguments");

	if (iac20->len > IAC20_BUF_LEN) {
		dev_err(dev, "Max length allowed for writing is %02u\n",
			IAC20_BUF_LEN);
		return -ENOMEM;
	}

	for (i = 0; i < iac20->len; i++) {
		val2[0] = val1[2 * i];
		val2[1] = val1[2 * i + 1];
		val2[2] = '\0';
		err = kstrtou8(val2, 16, &iac20->buf[i]);
		if (err)
			dev_err(dev, "Error on kstrtou8");
	}

	dev_dbg(dev, "Register address = 0x%02x\n", iac20->reg);
	dev_dbg(dev, "Register length  = %d\n", iac20->len);
	for (i = 0; i < iac20->len; i++)
		dev_dbg(dev, "Register data    = %x\n", iac20->buf[i]);

	err = iac20_i2c_write(iac20, iac20->reg, iac20->len, &iac20->buf);
	if (err == -EIO) {
		dev_err(dev, "Unable to write at register address (0x%02X)\n",
			iac20->reg);
		return -EIO;
	}

	return count;
}

static DEVICE_ATTR(device_info, 0444, show_device_info, NULL);
static DEVICE_ATTR(reg_read, 0644, show_reg_read, set_reg_read);
static DEVICE_ATTR(reg_write, 0200, NULL, set_reg_write);

static int major_match(struct device *dev, const void *data)
{
	unsigned int major = *(unsigned int *)data;

	return MAJOR(dev->devt) == major;
}

static int iac20_open(struct inode *inode, struct file *file)
{
	struct device *dev;
	struct iac20f *iac20f;
	unsigned int major = imajor(inode);

	dev = class_find_device(iac20_class, NULL, &major, major_match);
	if (!dev)
		return -ENODEV;

	iac20f = kzalloc(sizeof(*iac20f), GFP_KERNEL);
	if (!iac20f) {
		put_device(dev);
		return -ENOMEM;
	}

	iac20f->iac20 = dev_get_drvdata(dev->parent);
	put_device(dev);
	file->private_data = iac20f;

	dev_dbg(dev, "open\n");
	return 0;
}

static ssize_t iac20_read(struct file *file, char __user *buf,
			  size_t count, loff_t *ppos)
{
	struct iac20f *iac20f = file->private_data;
	struct iac20_data *iac20 = iac20f->iac20;
	int err;

	dev_dbg(&iac20->client->dev, "read reg(%d) size(%d)\n",
		iac20->addr, count);

	err = iac20_i2c_read(iac20, iac20->addr, count, iac20f->buf);
	if (err == -EIO) {
		dev_err(&iac20->client->dev, "Unable to read register addredd (0x%02X)\n",
			iac20->addr);
		return -EIO;
	}

	err = copy_to_user(buf, iac20f->buf, count);
	if (err > 0)
		return -EFAULT;

	return count;
}

static ssize_t iac20_write(struct file *file, const char __user *buf,
			   size_t count, loff_t *ppos)
{
	struct iac20f *iac20f = file->private_data;
	struct iac20_data *iac20 = iac20f->iac20;
	int err;

	if (count > IAC20_BUF_LEN) {
		dev_err(&iac20->client->dev, "Max length allowed for writing is %02u\n",
			IAC20_BUF_LEN);
		return -ENOMEM;
	}

	err = copy_from_user(iac20f->buf, buf, count);

	if (err > 0)
		return -EFAULT;

	dev_dbg(&iac20->client->dev, "write reg(%d) size(%d)\n",
		iac20->addr, count);

	err = iac20_i2c_write(iac20, iac20->addr, count, iac20f->buf);
	if (err == -EIO) {
		dev_err(&iac20->client->dev, "Unable to write at register address (0x%02X)\n",
			iac20->addr);
		return -EIO;
	}

	return 0;
}

static long iac20_ioctl(struct file *file, unsigned int cmd,
			unsigned long arg)
{
	struct iac20f *iac20f = file->private_data;
	struct iac20_data *iac20 = iac20f->iac20;
	int err;

	switch (cmd) {
	case IAC20_SET_ADDR:
		err = copy_from_user(&iac20->addr, (void __user *)arg,
				     sizeof(iac20->addr));
		if (err > 0)
			return -EFAULT;

		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int iac20_release(struct inode *inode, struct file *file)
{
	struct iac20f *iac20f = file->private_data;
	struct iac20_data *iac20 = iac20f->iac20;

	kfree(iac20f);
	file->private_data = NULL;

	return 0;
}

static const struct file_operations iac20_fops = {
	.owner		= THIS_MODULE,
	.open		= iac20_open,
	.read		= iac20_read,
	.write		= iac20_write,
	.unlocked_ioctl	= iac20_ioctl,
	.release	= iac20_release,
};

static void iac20_reset(struct iac20_data *iac20)
{
	if (!iac20->reset_gpio)
		return;

	gpiod_set_value_cansleep(iac20->reset_gpio, 1);
	usleep_range(5000, 20000);
	gpiod_set_value_cansleep(iac20->reset_gpio, 0);
	msleep(300);
}

static int iac20_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct iac20_data *iac20;
	int error;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENXIO;

	iac20 = devm_kzalloc(&client->dev, sizeof(*iac20), GFP_KERNEL);
	if (!iac20)
		return -ENOMEM;

	dev_set_drvdata(dev, iac20);

	iac20->client = client;
	iac20->reset_gpio = devm_gpiod_get_optional(dev, "reset",
						     GPIOD_OUT_LOW);
	if (IS_ERR(iac20->reset_gpio)) {
		error = PTR_ERR(iac20->reset_gpio);
		if (error != -EPROBE_DEFER)
			dev_err(dev, "error getting reset gpio: %d\n", error);
		return error;
	}

	iac20_reset(iac20);

	/* verify that the controller is present */
	dev_err(dev, "verify that the controller is present");
	error = i2c_smbus_write_byte(iac20->client, 0x00);
	/* For iPod Auth. Coprocessor need to relaunch the command on error */
	if (error) {
		error = i2c_smbus_write_byte(iac20->client, 0x00);

		if (error) {
			dev_warn(dev, "failed to read from controller: %d\n",
				 error);
			return error;
		}
	}

	iac20_major = register_chrdev(0, "iac20", &iac20_fops);
	if (iac20_major < 0) {
		dev_warn(dev, "cannot register device\n");
		return iac20_major;
	}

	iac20_class = class_create(THIS_MODULE, "iac20");
	if (IS_ERR(iac20_class)) {
		dev_warn(dev, "cannot create class\n");
		unregister_chrdev(iac20_major, "iac20");
		return PTR_ERR(iac20_class);
	}

	device_create(iac20_class, dev, MKDEV(iac20_major, 0), NULL, "iac20");

	error = device_create_file(dev, &dev_attr_device_info);
	if (error) {
		dev_err(dev, "failed: create device info sysfs entry\n");
		goto err;
	}

	error = device_create_file(dev, &dev_attr_reg_read);
	if (error) {
		dev_err(dev, "failed: create register read sysfs entry\n");
		goto err;
	}
	error = device_create_file(dev, &dev_attr_reg_write);
	if (error) {
		dev_err(dev, "failed: create register write sysfs entry\n");
		goto err;
	}
	return 0;

err:
	device_remove_file(dev, &dev_attr_device_info);
	device_remove_file(dev, &dev_attr_reg_read);
	device_remove_file(dev, &dev_attr_reg_write);
	return error;
}

static int iac20_remove(struct i2c_client *client)
{
	struct device *dev = &client->dev;

	device_remove_file(dev, &dev_attr_device_info);
	device_remove_file(dev, &dev_attr_reg_read);
	device_remove_file(dev, &dev_attr_reg_write);
	unregister_chrdev(iac20_major, "iac20");

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id iac20_of_match[] = {
	{ .compatible = "apple,iac20", },
	{ }
};
MODULE_DEVICE_TABLE(of, iac20_of_match);
#endif

static const struct i2c_device_id iac20_id[] = {
	{ "iac20", },
	{ }
};
MODULE_DEVICE_TABLE(i2c, iac20_id);

static struct i2c_driver iac20_driver = {
	.driver = {
		.name = "iac20",
		.of_match_table = of_match_ptr(iac20_of_match),
	},
	.probe = iac20_probe,
	.id_table = iac20_id,
	.remove = iac20_remove,
};
module_i2c_driver(iac20_driver);

MODULE_AUTHOR("Olivier Clergeaud <olivier.clergeaud@st.com>");
MODULE_AUTHOR("Guang Xian Yao <guang-xian.yao@st.com>");
MODULE_AUTHOR("STMicroelectronics");
MODULE_DESCRIPTION("iPod Authentication Coprocessor 2.0C driver");
MODULE_LICENSE("GPL");
