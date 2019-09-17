/*
 * sta_tvdec.c SDTV video decoder driver
 *
 * It's a derivated works from driver "adv7180"
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/videodev2.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ctrls.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include "sta_rpmsg_mm.h"

/* Exported rpmsg_mm services to be informed about
 * RVC usage on M3 core side.
 */
extern void *sta_rpmsg_register_endpoint(char *endpoint_name,
					 t_pfct_rpmsg_mm_cb callback,
					 void *priv);
extern int sta_rpmsg_unregister_endpoint(void *handle);
extern int sta_rpmsg_send_private_message(void *handle, char *remote_endpoint,
					  void *data, int len);
/*
 * enum tvdec_state - state of tvdec instance
 *
 * @TVDEC_STATE_READY:
 *	camera instance is ready to stream.
 *
 * @TVDEC_STATE_CLOSED:
 *	camera instance is closed.
 *
 * @TVDEC_STATE_STOPPED:
 *	camera instance is stopped.
 *
 * @TVDEC_STATE_EOS:
 *	EOS (End Of Stream) has been completed (ie signaled to V4L2 client)
 *
 * @TVDEC_STATE_FATAL_ERROR:
 *	A fatal error has been detected in the driver.
 *	Return it to user and cancel every new service except closure
 */
enum tvdec_state {
	TVDEC_STATE_READY,
	TVDEC_STATE_CLOSED,
	TVDEC_STATE_STOPPED,
	TVDEC_STATE_EOS,
	TVDEC_STATE_FATAL_ERROR
};

/*
 * enum tvdec_probe_status - status of tvdec probe
 *
 * @TVDEC_PROBE_IDLE:
 *	probe initiale state.
 *
 * @TVDEC_PROBE_POSTPONED:
 *	probe has been postponed.
 *
 * @TVDEC_PROBE_RESUME_POSTPONED:
 *	probe has been postponed after a resume.
 *
 * @TVDEC_PROBE_DONE:
 *	tvdec has been probed successfully
 */
enum tvdec_probe_status {
	TVDEC_PROBE_IDLE,
	TVDEC_PROBE_POSTPONED,
	TVDEC_PROBE_RESUME_POSTPONED,
	TVDEC_PROBE_DONE
};

enum rvc_rpsmg_private_msg {
	TVDEC_GET_CONFIG_REQ,
	/*!< TVDEC message transferred by Ax to Mx to request current
	 * configuration.
	 */

	TVDEC_GET_CONFIG_ACK,
	/*!< TVDEC message transferred by Mx to Ax to send tvdec configuration
	 */

	TVDEC_SET_INPUT_REQ,
	/*!< TVDEC message transferred by Ax to Mx to set input
	 */

	TVDEC_SET_INPUT_ACK,
	/*!< TVDEC message transferred by Ax to Mx to set input
	 */

	TVDEC_SET_STANDARD_REQ,
	/*!< TVDEC message transferred by Ax to Mx to set standard
	 */

	TVDEC_SET_STANDARD_ACK,
	/*!< TVDEC message transferred by Ax to Mx to set standard
	 */
};

enum rvc_standard {
	/* No standard detected */
	TVDEC_STD_UNKNOWN,
	/* PAL standard */
	TVDEC_STD_PAL,
	/* NTSC standard */
	TVDEC_STD_NTSC,
};

/*
 * TVDEC configuration message
 * Transferred from Mx to Ax
 */
struct tvdec_config_msg {
	u32 std;
	u32 input;
};

/*
 * TVDEC private message structure
 */
struct tvdec_msg {
	u32 info;
	int ret;
	struct tvdec_config_msg	config_msg;
};

/* Contrast */
#define STA_TVDEC_CON_MIN		0
#define STA_TVDEC_CON_DEF		128
#define STA_TVDEC_CON_MAX		255
/* Brightness*/
#define STA_TVDEC_BRI_MIN		-128
#define STA_TVDEC_BRI_DEF		0
#define STA_TVDEC_BRI_MAX		127
/* Saturation */
#define STA_TVDEC_SAT_MIN		0
#define STA_TVDEC_SAT_DEF		128
#define STA_TVDEC_SAT_MAX		255
/* Hue */
#define STA_TVDEC_HUE_MIN		-127
#define STA_TVDEC_HUE_DEF		0
#define STA_TVDEC_HUE_MAX		128

struct sta_tvdec_state {
	struct v4l2_ctrl_handler ctrl_hdl;
	struct v4l2_subdev	sd;
	struct platform_device	*pdev;
	struct device		*dev;
	struct media_pad	pad;
	struct mutex		mutex; /* mutual excl. when accessing chip */
	int			irq;
	struct gpio_desc	*pwdn_gpio;
	v4l2_std_id		curr_norm;
	bool			powered;
	bool			streaming;
	u8			input;

	unsigned int		register_page;
	enum v4l2_field		field;

	enum tvdec_state state;
	enum tvdec_probe_status probe_status;

	/* RPMsg */
	void *rpmsg_handle;
	bool wait_for_rpmsg_available;
	struct workqueue_struct *rpmsg_wq;
	struct completion rpmsg_connected;
	struct completion rpmsg_ack;
	struct work_struct resume_work;
	struct work_struct init_work;
};

#define to_sta_tvdec_sd(_ctrl) (&container_of((_ctrl)->handler,		\
					      struct sta_tvdec_state,	\
					      ctrl_hdl)->sd)

static inline struct sta_tvdec_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct sta_tvdec_state, sd);
}

static int sta_tvdec_querystd(struct v4l2_subdev *sd, v4l2_std_id *std)
{
	struct sta_tvdec_state *state = to_state(sd);
	int err;

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	err = mutex_lock_interruptible(&state->mutex);

	if (err)
		return err;

	if (state->streaming) {
		err = -EBUSY;
		goto unlock;
	}

	/* TODO: forward query standard request to Mx */
	*std = state->curr_norm;

unlock:
	mutex_unlock(&state->mutex);
	return err;
}

static int sta_tvdec_s_routing(struct v4l2_subdev *sd, u32 input,
			       u32 output, u32 config)
{
	struct sta_tvdec_state *state = to_state(sd);
	int ret;
	struct tvdec_msg *msg;
	char buffer[MAX_PAYLOAD_SIZE];

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	ret = mutex_lock_interruptible(&state->mutex);
	if (ret)
		return ret;

	/* message to be sent to Mx */
	msg = (struct tvdec_msg *)buffer;
	msg->info = TVDEC_SET_INPUT_REQ;
	msg->config_msg.input = input;

	ret = sta_rpmsg_send_private_message(state->rpmsg_handle,
					     RPSMG_MM_EPT_CORTEXM_TVDEC,
					     (void *)msg,
					     sizeof(struct tvdec_msg));
	if (ret) {
		dev_err(state->dev,
			"sta_rpmsg_send_private_message failed: %d\n",
			ret);
		goto out;
	}

	if (!wait_for_completion_timeout(
		&state->rpmsg_ack,
		msecs_to_jiffies(1000))) { /* Timeout after 1sec */
		dev_err(state->dev, "[RPMSG] : TIMEOUT!!!\n");
		ret = -EIO;
		goto out;
	}

	state->input = input;

out:
	mutex_unlock(&state->mutex);
	return ret;
}

static int sta_tvdec_g_input_status(struct v4l2_subdev *sd, u32 *status)
{
	struct sta_tvdec_state *state = to_state(sd);

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	/* TODO: if no signal detected, set flag V4L2_IN_ST_NO_SIGNAL */
	*status = 0;
	return 0;
}

static int sta_tvdec_s_std(struct v4l2_subdev *sd, v4l2_std_id std)
{
	struct sta_tvdec_state *state = to_state(sd);
	int ret;
	struct tvdec_msg *msg;
	char buffer[MAX_PAYLOAD_SIZE];

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	ret = mutex_lock_interruptible(&state->mutex);
	if (ret)
		return ret;

	/* message to be sent to Mx */
	msg = (struct tvdec_msg *)buffer;
	msg->info = TVDEC_SET_STANDARD_REQ;
	msg->config_msg.std = std;

	ret = sta_rpmsg_send_private_message(state->rpmsg_handle,
					     RPSMG_MM_EPT_CORTEXM_TVDEC,
					     (void *)msg,
					     sizeof(struct tvdec_msg));
	if (ret) {
		dev_err(state->dev,
			"sta_rpmsg_send_private_message failed: %d\n",
			ret);
		goto out;
	}

	if (!wait_for_completion_timeout(
		&state->rpmsg_ack,
		msecs_to_jiffies(1000))) { /* Timeout after 1sec */
		dev_err(state->dev, "[RPMSG] : TIMEOUT!!!\n");
		ret = -EIO;
		goto out;
	}

	state->curr_norm = std;

out:
	mutex_unlock(&state->mutex);
	return ret;
}

static int sta_tvdec_g_std(struct v4l2_subdev *sd, v4l2_std_id *norm)
{
	struct sta_tvdec_state *state = to_state(sd);

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	*norm = state->curr_norm;

	return 0;
}

static int sta_tvdec_set_power(struct sta_tvdec_state *state, bool on)
{
	/* TODO: forward request to Mx processor */
	return 0;
}

static int sta_tvdec_s_power(struct v4l2_subdev *sd, int on)
{
	struct sta_tvdec_state *state = to_state(sd);
	int ret;

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	ret = mutex_lock_interruptible(&state->mutex);
	if (ret)
		return ret;

	ret = sta_tvdec_set_power(state, on);
	if (ret == 0)
		state->powered = on;

	mutex_unlock(&state->mutex);
	return ret;
}

static int sta_tvdec_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_subdev *sd = to_sta_tvdec_sd(ctrl);
	struct sta_tvdec_state *state = to_state(sd);

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	/* TODO: forward request to Mx processor for these controls:
	 *  - V4L2_CID_BRIGHTNESS
	 *  - V4L2_CID_HUE
	 *  - V4L2_CID_CONTRAST
	 *  - V4L2_CID_SATURATION
	 */
	return 0;
}

static const struct v4l2_ctrl_ops sta_tvdec_ctrl_ops = {
	.s_ctrl = sta_tvdec_s_ctrl,
};

static int sta_tvdec_init_controls(struct sta_tvdec_state *state)
{
	v4l2_ctrl_handler_init(&state->ctrl_hdl, 4);

	v4l2_ctrl_new_std(&state->ctrl_hdl, &sta_tvdec_ctrl_ops,
			  V4L2_CID_BRIGHTNESS, STA_TVDEC_BRI_MIN,
			  STA_TVDEC_BRI_MAX, 1, STA_TVDEC_BRI_DEF);
	v4l2_ctrl_new_std(&state->ctrl_hdl, &sta_tvdec_ctrl_ops,
			  V4L2_CID_CONTRAST, STA_TVDEC_CON_MIN,
			  STA_TVDEC_CON_MAX, 1, STA_TVDEC_CON_DEF);
	v4l2_ctrl_new_std(&state->ctrl_hdl, &sta_tvdec_ctrl_ops,
			  V4L2_CID_SATURATION, STA_TVDEC_SAT_MIN,
			  STA_TVDEC_SAT_MAX, 1, STA_TVDEC_SAT_DEF);
	v4l2_ctrl_new_std(&state->ctrl_hdl, &sta_tvdec_ctrl_ops,
			  V4L2_CID_HUE, STA_TVDEC_HUE_MIN,
			  STA_TVDEC_HUE_MAX, 1, STA_TVDEC_HUE_DEF);

	state->sd.ctrl_handler = &state->ctrl_hdl;
	if (state->ctrl_hdl.error) {
		int err = state->ctrl_hdl.error;

		v4l2_ctrl_handler_free(&state->ctrl_hdl);
		return err;
	}
	v4l2_ctrl_handler_setup(&state->ctrl_hdl);

	return 0;
}

static void sta_tvdec_exit_controls(struct sta_tvdec_state *state)
{
	v4l2_ctrl_handler_free(&state->ctrl_hdl);
}

static int sta_tvdec_enum_mbus_code(struct v4l2_subdev *sd,
				    struct v4l2_subdev_pad_config *cfg,
				    struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index != 0)
		return -EINVAL;

	code->code = MEDIA_BUS_FMT_UYVY8_2X8;

	return 0;
}

static int sta_tvdec_mbus_fmt(struct v4l2_subdev *sd,
			      struct v4l2_mbus_framefmt *fmt)
{
	struct sta_tvdec_state *state = to_state(sd);

	fmt->code = MEDIA_BUS_FMT_UYVY8_2X8;
	fmt->colorspace = V4L2_COLORSPACE_SMPTE170M;
	fmt->width = 720;
	fmt->height = state->curr_norm & V4L2_STD_525_60 ? 480 : 576;

	return 0;
}

static int sta_tvdec_set_field_mode(struct sta_tvdec_state *state)
{
	return 0;
}

static int sta_tvdec_get_pad_format(struct v4l2_subdev *sd,
				    struct v4l2_subdev_pad_config *cfg,
				    struct v4l2_subdev_format *format)
{
	struct sta_tvdec_state *state = to_state(sd);

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	if (format->which == V4L2_SUBDEV_FORMAT_TRY) {
		format->format = *v4l2_subdev_get_try_format(sd, cfg, 0);
	} else {
		sta_tvdec_mbus_fmt(sd, &format->format);
		format->format.field = state->field;
	}

	return 0;
}

static int sta_tvdec_set_pad_format(struct v4l2_subdev *sd,
				    struct v4l2_subdev_pad_config *cfg,
				    struct v4l2_subdev_format *format)
{
	struct sta_tvdec_state *state = to_state(sd);
	struct v4l2_mbus_framefmt *framefmt;
	int ret;

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	switch (format->format.field) {
	case V4L2_FIELD_NONE:
		break;
	default:
		format->format.field = V4L2_FIELD_INTERLACED;
		break;
	}

	ret = sta_tvdec_mbus_fmt(sd,  &format->format);

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE) {
		if (state->field != format->format.field) {
			state->field = format->format.field;
			sta_tvdec_set_power(state, false);
			sta_tvdec_set_field_mode(state);
			sta_tvdec_set_power(state, true);
		}
	} else {
		framefmt = v4l2_subdev_get_try_format(sd, cfg, 0);
		*framefmt = format->format;
	}

	return ret;
}

static int sta_tvdec_g_mbus_config(struct v4l2_subdev *sd,
				   struct v4l2_mbus_config *cfg)
{
	cfg->flags = V4L2_MBUS_MASTER | V4L2_MBUS_PCLK_SAMPLE_RISING |
				V4L2_MBUS_DATA_ACTIVE_HIGH;
	cfg->type = V4L2_MBUS_BT656;

	return 0;
}

static int sta_tvdec_g_pixelaspect(struct v4l2_subdev *sd,
				   struct v4l2_fract *aspect)
{
	struct sta_tvdec_state *state = to_state(sd);

	if (state->curr_norm & V4L2_STD_525_60) {
		aspect->numerator = 11;
		aspect->denominator = 10;
	} else {
		aspect->numerator = 54;
		aspect->denominator = 59;
	}

	return 0;
}

static int sta_tvdec_g_tvnorms(struct v4l2_subdev *sd, v4l2_std_id *norm)
{
	*norm = V4L2_STD_ALL;
	return 0;
}

static int sta_tvdec_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct sta_tvdec_state *state = to_state(sd);
	int ret;

	if (state->state == TVDEC_STATE_FATAL_ERROR) {
		dev_dbg(state->dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	/* It's always safe to stop streaming, no need to take the lock */
	if (!enable) {
		state->streaming = enable;
		return 0;
	}

	/* Must wait until querystd released the lock */
	ret = mutex_lock_interruptible(&state->mutex);
	if (ret)
		return ret;
	state->streaming = enable;
	mutex_unlock(&state->mutex);
	return 0;
}

static int sta_tvdec_subscribe_event(struct v4l2_subdev *sd,
				     struct v4l2_fh *fh,
				     struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subdev_subscribe(sd, fh, sub);
	case V4L2_EVENT_CTRL:
		return v4l2_ctrl_subdev_subscribe_event(sd, fh, sub);
	default:
		return -EINVAL;
	}
}

static const struct v4l2_subdev_video_ops sta_tvdec_video_ops = {
	.s_std = sta_tvdec_s_std,
	.g_std = sta_tvdec_g_std,
	.querystd = sta_tvdec_querystd,
	.g_input_status = sta_tvdec_g_input_status,
	.s_routing = sta_tvdec_s_routing,
	.g_mbus_config = sta_tvdec_g_mbus_config,
	.g_pixelaspect = sta_tvdec_g_pixelaspect,
	.g_tvnorms = sta_tvdec_g_tvnorms,
	.s_stream = sta_tvdec_s_stream,
};

static const struct v4l2_subdev_core_ops sta_tvdec_core_ops = {
	.s_power = sta_tvdec_s_power,
	.subscribe_event = sta_tvdec_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_pad_ops sta_tvdec_pad_ops = {
	.enum_mbus_code = sta_tvdec_enum_mbus_code,
	.set_fmt = sta_tvdec_set_pad_format,
	.get_fmt = sta_tvdec_get_pad_format,
};

static const struct v4l2_subdev_ops sta_tvdec_ops = {
	.core = &sta_tvdec_core_ops,
	.video = &sta_tvdec_video_ops,
	.pad = &sta_tvdec_pad_ops,
};

static int init_device(struct sta_tvdec_state *state)
{
	return 0;
}

static void sta_tvdec_update_config(struct sta_tvdec_state *state,
				    struct tvdec_config_msg config_msg)
{
	dev_dbg(state->dev,
		"RVC_GET_CONFIGURATION_ACK %d %d\n",
		config_msg.std,
		config_msg.input);

	/* Camera standard */
	switch (config_msg.std) {
	case TVDEC_STD_PAL:
		state->curr_norm = V4L2_STD_PAL;
		break;
	case TVDEC_STD_NTSC:
		state->curr_norm = V4L2_STD_NTSC;
		break;
	case TVDEC_STD_UNKNOWN:
	default:
		state->curr_norm = V4L2_STD_UNKNOWN;
		break;
	}

	/* Camera input */
	state->input = config_msg.input;
}

int sta_tvdec_rpmsg_endpoint_cb(struct rpmsg_mm_msg_hdr *data, void *priv)
{
	struct sta_tvdec_state *state = (struct sta_tvdec_state *)priv;
	struct tvdec_msg *msg;

	if (!data)
		return -1;

	dev_dbg(state->dev,
		"sta_tvdec_rpmsg_endpoint_cb data->info: %d\n",
		data->info);

	switch (data->info) {
	case RPMSG_MM_COMM_AVAILABLE:
		break;
	case RPMSG_MM_PRIVATE_MESSAGE:
		msg = (struct tvdec_msg *)data->data;
		switch (msg->info) {
		case TVDEC_GET_CONFIG_ACK:
			sta_tvdec_update_config(state, msg->config_msg);
			complete(&state->rpmsg_ack);
			break;
		case TVDEC_SET_INPUT_ACK:
		case TVDEC_SET_STANDARD_ACK:
			complete(&state->rpmsg_ack);
			break;
		default:
			break;
		}
		break;
	case RPMSG_MM_COMM_UNAVAILABLE:
	default:
		break;
	}
	return 0;
}

static int sta_tvdec_get_rvc_configuration(struct sta_tvdec_state *state)
{
	int ret;
	struct tvdec_msg msg;

	msg.info = TVDEC_GET_CONFIG_REQ;

	ret = sta_rpmsg_send_private_message(state->rpmsg_handle,
					     RPSMG_MM_EPT_CORTEXM_TVDEC,
					     (void *)&msg,
					     sizeof(struct tvdec_msg));
	if (ret) {
		dev_err(state->dev,
			"sta_rpmsg_send_private_message failed: %d\n",
			ret);
		return ret;
	}

	if (!wait_for_completion_timeout(
		&state->rpmsg_ack,
		msecs_to_jiffies(1000))) { /* Timeout after 1sec */
		dev_err(state->dev, "[RPMSG] : TIMEOUT!!!\n");
		return -EIO;
	}

	return 0;
}

static void sta_tvdec_resume_work(struct work_struct *work)
{
	int ret;
	struct sta_tvdec_state *state =
		container_of(work, struct sta_tvdec_state, resume_work);

	if (state->wait_for_rpmsg_available) {
		if (!wait_for_completion_timeout(
			&state->rpmsg_connected,
			msecs_to_jiffies(5000))) { /* Timeout after 5sec */
			dev_err(state->dev, "[RPMSG] : TIMEOUT!!!\n");
			goto fatal_error;
		}
	}
	state->wait_for_rpmsg_available = false;

	ret = sta_tvdec_get_rvc_configuration(state);
	if (ret) {
		dev_err(state->dev,
			"Failed to get RVC configuration\n");
		state->state = TVDEC_STATE_FATAL_ERROR;
		return;
	}

	dev_dbg(state->dev, "[RPMSG] : tvdec resume OK\n");
	state->probe_status = TVDEC_PROBE_DONE;
	return;

fatal_error:
	dev_dbg(state->dev, "[RPMSG] : tvdec resource unavailable\n");
	state->wait_for_rpmsg_available = false;
	state->state = TVDEC_STATE_FATAL_ERROR;
}

static int sta_tvdec_v4l2_probe(struct sta_tvdec_state *state)
{
	int ret = 0;

	ret = v4l2_async_register_subdev(&state->sd);
	if (ret)
		return ret;

	platform_set_drvdata(state->pdev, state);
	return 0;
}

static int sta_tvdec_v4l2_remove(struct sta_tvdec_state *state)
{
	v4l2_async_unregister_subdev(&state->sd);
	return 0;
}

static void sta_tvdec_init_work(struct work_struct *work)
{
	int ret;
	struct sta_tvdec_state *state =
		container_of(work, struct sta_tvdec_state, init_work);

	ret = sta_tvdec_get_rvc_configuration(state);
	if (ret) {
		dev_err(state->dev,
			"Failed to get RVC configuration\n");
		state->state = TVDEC_STATE_FATAL_ERROR;
		return;
	}

	ret = sta_tvdec_v4l2_probe(state);
	if (ret) {
		dev_err(state->dev,
			"Failed to complete probe\n");
		state->state = TVDEC_STATE_FATAL_ERROR;
		return;
	}

	state->probe_status = TVDEC_PROBE_DONE;
}

static int sta_tvdec_probe(struct platform_device *pdev)
{
	struct sta_tvdec_state *state;
	struct v4l2_subdev *sd;
	int ret;

	state = devm_kzalloc(&pdev->dev, sizeof(*state), GFP_KERNEL);
	if (!state)
		return -ENOMEM;

	state->pdev = pdev;
	state->dev = &pdev->dev;
	state->field = V4L2_FIELD_INTERLACED;

	mutex_init(&state->mutex);
	state->curr_norm = V4L2_STD_NTSC;
	state->input = 0;
	sd = &state->sd;

	/* SUBDEV INIT */
	v4l2_subdev_init(sd, &sta_tvdec_ops);
	sd->owner = pdev->dev.driver->owner;
	sd->dev = &pdev->dev;

	v4l2_set_subdevdata(sd, pdev);

	/* set name */
	snprintf(sd->name, sizeof(sd->name), "%s", pdev->dev.driver->name);

	sd->flags = V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS;

	ret = sta_tvdec_init_controls(state);
	if (ret)
		goto err_mutex_destroy;

	state->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.flags |= MEDIA_ENT_F_ATV_DECODER;
	ret = media_entity_pads_init(&sd->entity, 1, &state->pad);
	if (ret)
		goto err_free_ctrl;

	ret = init_device(state);
	if (ret)
		goto err_media_entity_cleanup;

	init_completion(&state->rpmsg_ack);

	state->rpmsg_handle = sta_rpmsg_register_endpoint(
					RPSMG_MM_EPT_CORTEXA_TVDEC,
					sta_tvdec_rpmsg_endpoint_cb,
					(void *)state);

	if (!state->rpmsg_handle) {
		dev_err(&pdev->dev, "Unable to register rpmsg_mm\n");
		return -EIO;
	}

	state->wait_for_rpmsg_available = false;

	state->rpmsg_wq = create_singlethread_workqueue("tvdec_rpmsg_wq");
	if (!state->rpmsg_wq)
		return -ENOMEM;

	init_completion(&state->rpmsg_connected);
	init_completion(&state->rpmsg_ack);

	INIT_WORK(&state->resume_work, sta_tvdec_resume_work);
	INIT_WORK(&state->init_work, sta_tvdec_init_work);

	state->probe_status = TVDEC_PROBE_POSTPONED;
	queue_work(state->rpmsg_wq, &state->init_work);

	return 0;

err_media_entity_cleanup:
	media_entity_cleanup(&sd->entity);
err_free_ctrl:
	sta_tvdec_exit_controls(state);
err_mutex_destroy:
	mutex_destroy(&state->mutex);
	sta_rpmsg_unregister_endpoint(state->rpmsg_handle);
	state->rpmsg_handle = NULL;
	return ret;
}

static int sta_tvdec_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *sd;
	struct sta_tvdec_state *state = platform_get_drvdata(pdev);

	sd = &state->sd;

	complete(&state->rpmsg_connected);
	complete(&state->rpmsg_ack);
	cancel_work_sync(&state->init_work);
	cancel_work_sync(&state->resume_work);
	destroy_workqueue(state->rpmsg_wq);

	if (state->probe_status == TVDEC_PROBE_DONE)
		sta_tvdec_v4l2_remove(state);

	media_entity_cleanup(&sd->entity);
	sta_tvdec_exit_controls(state);

	mutex_destroy(&state->mutex);
	sta_rpmsg_unregister_endpoint(state->rpmsg_handle);
	state->rpmsg_handle = NULL;

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sta_tvdec_suspend(struct device *dev)
{
	struct sta_tvdec_state *state = dev_get_drvdata(dev);

	dev_dbg(dev, "suspend\n");

	if (state->probe_status == TVDEC_PROBE_POSTPONED)
		cancel_work_sync(&state->init_work);

	if (state->probe_status == TVDEC_PROBE_RESUME_POSTPONED)
		cancel_work_sync(&state->resume_work);

	return 0;
}

static int sta_tvdec_resume(struct device *dev)
{
	struct sta_tvdec_state *state = dev_get_drvdata(dev);

	dev_dbg(dev, "resume\n");

	if (state->probe_status == TVDEC_PROBE_DONE) {
		state->probe_status = TVDEC_PROBE_RESUME_POSTPONED;
		state->wait_for_rpmsg_available = true;
		reinit_completion(&state->rpmsg_connected);
		reinit_completion(&state->rpmsg_ack);
		queue_work(state->rpmsg_wq, &state->resume_work);
	}

	return 0;
}

static SIMPLE_DEV_PM_OPS(sta_tvdec_pm_ops, sta_tvdec_suspend, sta_tvdec_resume);
#define STA_TVDEC_PM_OPS (&sta_tvdec_pm_ops)

#else
#define STA_TVDEC_PM_OPS NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id sta_tvdec_of_id[] = {
	{ .compatible = "st,sta_tvdec" },
	{ },
};

MODULE_DEVICE_TABLE(of, sta_tvdec_of_id);
#endif

static struct platform_driver sta_tvdec_driver = {
	.driver = {
		   .name = KBUILD_MODNAME,
		   .pm = STA_TVDEC_PM_OPS,
		   .of_match_table = of_match_ptr(sta_tvdec_of_id),
		   },
	.probe = sta_tvdec_probe,
	.remove = sta_tvdec_remove,
};

module_platform_driver(sta_tvdec_driver);

MODULE_DESCRIPTION("SDTV decoder virtual driver");
MODULE_AUTHOR("ST Automotive Group");
MODULE_LICENSE("GPL v2");
