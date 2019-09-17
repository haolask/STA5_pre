/*
 * STA Tuner Contect Backup
 *
 * MISC driver to save context information of the Tuner application
 * copyright (c) 2018 STMicroelectronics
 * license terms: gnu general public license v2
 */

#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mailbox_client.h>
#include <linux/mailbox_controller.h>
#include <linux/mailbox_sta.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

/* Mailbox Payload size. Must be aligned with mailbox driver */
#define MAX_MBOX_SIZE 15

/* STA Tuner Context driver structure */
struct sta_tuner_ctx {
	struct device		*dev;
	struct mbox_client	client;
	struct mbox_chan	*chan;
};

static void sta_tuner_ctx_callback(struct mbox_client *mb_client, void *data)
{
	/* Nothing to do */
}

static ssize_t
sta_tuner_write_ctx(struct device *dev, struct device_attribute *attr,
		    const char *buf, size_t count)
{
	struct sta_tuner_ctx *tctx = dev_get_drvdata(dev);
	struct sta_mbox_msg msg;
	u8 data[MAX_MBOX_SIZE];
	int len = count;
	int ret;

	if (len > MAX_MBOX_SIZE) {
		dev_err(dev, "Error: max size: %d\n", MAX_MBOX_SIZE);
		return -EINVAL;
	}

	memcpy(data, buf, len);

	msg.dsize = len;
	msg.pdata = data;
	ret = mbox_send_message(tctx->chan, (void *)&msg);
	if (ret < 0)
		dev_err(dev, "%s: mbox send failed: %d\n", __func__, ret);

	return count;
}

static DEVICE_ATTR(tuner_ctx, 0200, NULL, sta_tuner_write_ctx);

static int sta_tuner_ctx_probe(struct platform_device *pdev)
{
	struct sta_tuner_ctx *tctx;
	int err;

	err = device_create_file(&pdev->dev, &dev_attr_tuner_ctx);
	if (err)
		dev_warn(&pdev->dev, "error creating sysfs entries\n");

	tctx = devm_kzalloc(&pdev->dev, sizeof(*tctx), GFP_KERNEL);
	if (!tctx)
		return -ENOMEM;

	tctx->client.dev = &pdev->dev;
	tctx->client.tx_block = true;
	tctx->client.tx_tout = 1000;
	tctx->client.knows_txdone = false;
	tctx->client.rx_callback = sta_tuner_ctx_callback;
	tctx->client.tx_done = NULL;
	tctx->client.tx_prepare = NULL;

	tctx->chan = mbox_request_channel(&tctx->client, 0);
	if (IS_ERR(tctx->chan))
		return -EBUSY;

	dev_set_drvdata(&pdev->dev, tctx);

	return 0;
}

static int sta_tuner_ctx_remove(struct platform_device *pdev)
{
	struct sta_tuner_ctx *tctx = dev_get_drvdata(&pdev->dev);

	mbox_free_channel(tctx->chan);

	return 0;
}

static const struct of_device_id sta_tuner_ctx_match[] = {
	{ .compatible = "st,tuner_backup" },
	{},
};

static struct platform_driver sta_tuner_ctx_driver = {
	.driver = {
		.name = "sta_tuner_ctx",
		.of_match_table = sta_tuner_ctx_match,
	},
	.probe  = sta_tuner_ctx_probe,
	.remove = sta_tuner_ctx_remove,
};
module_platform_driver(sta_tuner_ctx_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Tuner Context Backup");
MODULE_AUTHOR("Seraphin Bonnaffe <seraphin.bonnaffe@st.com>");

