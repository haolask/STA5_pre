/*
 * ITU-R BT.601 / BT.656 Rear View Camera driver for Accordo5/sta SoC.
 *
 * Copyright (C) 2017 STMicroelectronics Automotive Group
 *
 * author: Mustapha Ghanmi (mustapha.ghanmi@st.com)
 * It's a derivated works from:
 * sta VIP driver by: Philippe Langlais (philippe.langlais@st.com)
 *                    Pierre-Yves MORDRET (pierre-yves.mordre@st.com)
 * original cartesio driver by: Sandeep Kaushik (sandeep-mmc.kaushik@st.com)
 * and sta VIP PCI driver by: Federico Vaga <federico.vaga@gmail.com>
 *                            Andreas Kies <andreas.kies@windriver.com>
 *                            Vlad Lungu   <vlad.lungu@windriver.com>
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/vmalloc.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/rpmsg.h>

#include <media/v4l2-event.h>
#include <media/videobuf2-dma-contig.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-v4l2.h>

#include "sta_rpmsg_mm.h"
#include "sta_rvcam.h"

/* Exported rpmsg_mm services to be informed about
 * RVC usage on M3 core side.
 */
extern void *sta_rpmsg_register_endpoint(char *endpoint_name,
					 t_pfct_rpmsg_mm_cb callback,
					 void *priv);
extern int sta_rpmsg_unregister_endpoint(void *handle);
extern int sta_rpmsg_send_private_message(void *handle, char *remote_endpoint,
					  void *data, int len);

static void rvcam_stop_capture(struct sta_rvcam *rvcam)
{
	const struct v4l2_event eos_event = {
		.type = V4L2_EVENT_EOS
	};

	v4l2_event_queue(rvcam->video_dev, &eos_event);
}

/* Videobuf2 Operations */
static int rvcam_queue_setup(struct vb2_queue *vq, unsigned int *nbuffers,
			     unsigned int *nplanes, unsigned int sizes[],
			     struct device *alloc_devs[])
{
	struct sta_rvcam *rvcam = vb2_get_drv_priv(vq);

	if (rvcam->state == RVCAM_STATE_FATAL_ERROR) {
		dev_dbg(rvcam->v4l2_dev.dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	if (!(*nbuffers) || *nbuffers > MAX_FRAMES)
		*nbuffers = MAX_FRAMES;

	*nplanes = 1;
	sizes[0] = rvcam->format_cap.sizeimage;

	rvcam->sequence = 0;
	rvcam->frame_err = 0;
	rvcam->frame_drop = 0;
	rvcam->incomplete_frame_nb = 0;

	memset(&rvcam->perf, 0, sizeof(rvcam->perf));

	dev_dbg(rvcam->v4l2_dev.dev, "nbuf=%d, size=%u\n", *nbuffers, sizes[0]);

	return 0;
};

static int rvcam_buffer_init(struct vb2_buffer *vb)
{
	//int ret;
	struct rvcam_buffer *rvcam_buf = to_rvcam_buffer(vb);
	struct sta_rvcam *rvcam = vb2_get_drv_priv(vb->vb2_queue);
	unsigned long image_size = rvcam->format_cap.sizeimage;

	if (rvcam->state == RVCAM_STATE_FATAL_ERROR) {
		dev_dbg(rvcam->v4l2_dev.dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	INIT_LIST_HEAD(&rvcam_buf->queue);

	if (vb2_plane_size(vb, 0) < image_size)
		return -ENOMEM;
	return 0;
}

static int rvcam_buffer_prepare(struct vb2_buffer *vb)
{
	struct sta_rvcam *rvcam = vb2_get_drv_priv(vb->vb2_queue);
	struct rvcam_buffer *rvcam_buf = to_rvcam_buffer(vb);
	unsigned long size;

	if (rvcam->state == RVCAM_STATE_FATAL_ERROR) {
		dev_dbg(rvcam->v4l2_dev.dev, "[%s] return -EIO\n", __func__);
		return -EIO;
	}

	size = rvcam->format_cap.sizeimage;
	if (vb2_plane_size(vb, 0) < size) {
		v4l2_err(&rvcam->v4l2_dev, "buffer too small (%lu < %lu)\n",
			 vb2_plane_size(vb, 0), size);
		return -EINVAL;
	}

	vb2_set_plane_payload(&rvcam_buf->vbuf.vb2_buf, 0, size);

#ifdef DEBUG
	/* This can be useful if you want to see if we actually fill
	 * the buffer with something
	 */
	memset(vb2_plane_vaddr(&rvcam_buf->vbuf.vb2_buf, 0), 0xaa, size);
#endif
	return 0;
}

static void rvcam_buffer_queue(struct vb2_buffer *vb)
{
	struct sta_rvcam *rvcam = vb2_get_drv_priv(vb->vb2_queue);
	struct rvcam_buffer *rvcam_buf = to_rvcam_buffer(vb);
	struct rvcam_msg *msg;
	int ret = 0;
	char buffer[MAX_PAYLOAD_SIZE];

	if (rvcam->state == RVCAM_STATE_FATAL_ERROR) {
		dev_dbg(rvcam->v4l2_dev.dev,
			"[%s] return on fatal error\n", __func__);
		return;
	}

	mutex_lock(&rvcam->buf_list_lock);

	list_add_tail(&rvcam_buf->queue, &rvcam->capture);

	/* buffer copied: ready to be used again by M3 */
	msg = (struct rvcam_msg *)buffer;
	msg->info = RVC_FILL_BUFFER;
	msg->m.fill_buffer_msg.buf_phys_addr =
			(u32)virt_to_phys(vb2_plane_vaddr(vb, 0));
	msg->m.fill_buffer_msg.vb2_buf = (u32)vb;

	ret = sta_rpmsg_send_private_message(rvcam->rpmsg_handle,
					     RPSMG_MM_EPT_CORTEXM_RVC,
					     (void *)msg,
					     sizeof(struct rvcam_msg));
	if (ret) {
		dev_err(rvcam->v4l2_dev.dev,
			"sta_rpmsg_send_private_message failed: %d\n",
			ret);
		mutex_unlock(&rvcam->buf_list_lock);
		return;
	}

	mutex_unlock(&rvcam->buf_list_lock);
}

static int rvcam_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct sta_rvcam *rvcam = vb2_get_drv_priv(vq);
	int ret;
	u32 info = RVC_START_STREAMING_REQ;

	dev_dbg(rvcam->v4l2_dev.dev, "rvcam_start_streaming\n");

	rvcam->perf.start_streaming_ns = ktime_get_ns();

	ret = sta_rpmsg_send_private_message(rvcam->rpmsg_handle,
					     RPSMG_MM_EPT_CORTEXM_RVC,
					     (void *)&info, sizeof(u32));
	if (ret) {
		dev_err(rvcam->v4l2_dev.dev,
			"sta_rpmsg_send_private_message failed: %d\n",
			ret);
		return ret;
	}

	rvcam->state = RVCAM_STATE_READY;

	return 0;
}

/* abort streaming */
static void rvcam_stop_streaming(struct vb2_queue *vq)
{
	struct sta_rvcam *rvcam = vb2_get_drv_priv(vq);
	struct rvcam_buffer *rvcam_buf, *node;
	int ret;
	u32 info = RVC_STOP_STREAMING_REQ;

	dev_dbg(rvcam->v4l2_dev.dev, "rvcam_stop_streaming\n");

	mutex_lock(&rvcam->buf_list_lock);

	/* Disable acquisition */
	rvcam_stop_capture(rvcam);

	ret = sta_rpmsg_send_private_message(rvcam->rpmsg_handle,
					     RPSMG_MM_EPT_CORTEXM_RVC,
					     (void *)&info, sizeof(u32));
	if (ret) {
		dev_err(rvcam->v4l2_dev.dev,
			"sta_rpmsg_send_private_message failed: %d\n",
			ret);
	}

	/* Release all active buffers */
	list_for_each_entry_safe(rvcam_buf, node, &rvcam->capture, queue) {
		vb2_buffer_done(&rvcam_buf->vbuf.vb2_buf, VB2_BUF_STATE_ERROR);
		list_del(&rvcam_buf->queue);
	}

	rvcam->state = RVCAM_STATE_STOPPED;

	mutex_unlock(&rvcam->buf_list_lock);
}

static int sta_rvcam_dbg_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int sta_rvcam_device_read(struct file *file, char __user *user_buf,
				 size_t size, loff_t *ppos)
{
	struct sta_rvcam *rvcam = file->private_data;
	unsigned char *cur = rvcam->str;
	unsigned int count = 0;
	size_t left = sizeof(rvcam->str);
	int cnt = 0;
	int ret = 0;

	memset(rvcam->str, 0, sizeof(rvcam->str));

	ret = snprintf(cur, left, "[%s]\n", rvcam->v4l2_dev.name);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "registered as /dev/video%d\n", rvcam->video_dev->num);
	cnt = (left > ret ? ret : left);

	count = simple_read_from_buffer(user_buf, strlen(rvcam->str), ppos,
					rvcam->str, strlen(rvcam->str));

	return count;
}

#define STA_RVCAM_SPRINTF(...) \
do { \
	cur += cnt; \
	left -= cnt; \
	ret = snprintf(cur, left, __VA_ARGS__); \
	cnt = (left > ret ? ret : left); \
} while (0)

static int sta_rvcam_last_read(struct file *file, char __user *user_buf,
			       size_t size, loff_t *ppos)
{
	struct sta_rvcam *rvcam = file->private_data;
	unsigned char *cur = rvcam->str;
	unsigned int count = 0;
	size_t left = sizeof(rvcam->str);
	int cnt = 0;
	int ret = 0;
	u64 duration_ns = 0;
	unsigned int duration = 0;
	int avg_fps = 0;
	u32 std = (u32)rvcam->std;
	u32 input = (u32)rvcam->input;
	u32 field = (u32)rvcam->field;

	memset(rvcam->str, 0, sizeof(rvcam->str));

	STA_RVCAM_SPRINTF("[last session]\n");
	STA_RVCAM_SPRINTF("  |\n  |-[frame infos]\n");
	STA_RVCAM_SPRINTF("  | |- standard: 0x%X\n", std);
	STA_RVCAM_SPRINTF("  | |- input: 0x%X\n", input);
	STA_RVCAM_SPRINTF("  | |- field_type: 0x%X\n", field);
	STA_RVCAM_SPRINTF("  | |- frame_errors: %d\n", rvcam->frame_err);
	STA_RVCAM_SPRINTF("  | |- frame_drop: %d\n", rvcam->frame_drop);
	STA_RVCAM_SPRINTF("  | |- incomplete_frames: %d\n",
			  rvcam->incomplete_frame_nb);
	STA_RVCAM_SPRINTF("  | |- sequence: %d\n", rvcam->sequence);
	STA_RVCAM_SPRINTF("  | |- capture_format: %dx%d\n",
			  rvcam->format_cap.width, rvcam->format_cap.height);

	if (rvcam->perf.end_streaming_ns) {
		STA_RVCAM_SPRINTF("  |\n  |-[performance]\n");

		duration_ns = rvcam->perf.min_duration;
		do_div(duration_ns, 1000ul);
		duration = (unsigned int)duration_ns;

		STA_RVCAM_SPRINTF("  | |- min_duration (us): %d\n", duration);

		duration_ns = rvcam->perf.max_duration;
		do_div(duration_ns, 1000ul);
		duration = (unsigned int)duration_ns;

		STA_RVCAM_SPRINTF("  | |- max_duration (us): %d\n", duration);

		duration_ns = rvcam->perf.end_streaming_ns -
			      rvcam->perf.start_streaming_ns;
		do_div(duration_ns, 1000000ul);
		duration = (unsigned int)duration_ns;

		STA_RVCAM_SPRINTF("  | |- total_duration (ms): %d\n", duration);

		if (duration)
			avg_fps = ((rvcam->sequence - 1) * 10000) / duration;

		STA_RVCAM_SPRINTF("  | |- avg_fps (0.1Hz): %d\n", avg_fps);
	}

	count = simple_read_from_buffer(user_buf, strlen(rvcam->str), ppos,
					rvcam->str, strlen(rvcam->str));

	return count;
}

static const struct file_operations sta_rvcam_device_fops = {
	.owner = THIS_MODULE,
	.open = sta_rvcam_dbg_open,
	.read = sta_rvcam_device_read,
};

static const struct file_operations sta_rvcam_last_fops = {
	.owner = THIS_MODULE,
	.open = sta_rvcam_dbg_open,
	.read = sta_rvcam_last_read,
};

void sta_rvcam_debugfs_create(struct sta_rvcam *rvcam)
{
	rvcam->debugfs_dir = debugfs_create_dir("sta_rvcam", NULL);

	rvcam->debugfs_device = debugfs_create_file("device", 0400,
						    rvcam->debugfs_dir, rvcam,
						    &sta_rvcam_device_fops);

	rvcam->debugfs_last = debugfs_create_file("last", 0400,
						  rvcam->debugfs_dir, rvcam,
						  &sta_rvcam_last_fops);
}

void sta_rvcam_debugfs_remove(struct sta_rvcam *rvcam)
{
	debugfs_remove(rvcam->debugfs_last);
	debugfs_remove(rvcam->debugfs_device);
	debugfs_remove(rvcam->debugfs_dir);
}

static int sta_rvcam_g_v_ctrl(struct v4l2_ctrl *ctrl)
{
	int ret = 0;

	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		ctrl->val = NUM_MIN_BUFFERS;
		break;
	default:
		ret = -EINVAL;
	}
	return 0;
}

static const struct v4l2_ctrl_ops sta_rvcam_ctrl_ops = {
	.g_volatile_ctrl = sta_rvcam_g_v_ctrl,
};

static int sta_rvcam_init_controls(struct sta_rvcam *rvcam)
{
	int i;

	if (rvcam->v4l2_ctrls_is_configured)
		return 0;

	v4l2_ctrl_handler_init(&rvcam->ctrl_hdl, RVCAM_MAX_CTRLS);
	if (rvcam->ctrl_hdl.error) {
		int err = rvcam->ctrl_hdl.error;

		dev_err(rvcam->v4l2_dev.dev, "v4l2_ctrl_handler_init failed\n");
		v4l2_ctrl_handler_free(&rvcam->ctrl_hdl);
		return err;
	}

	rvcam->v4l2_dev.ctrl_handler = &rvcam->ctrl_hdl;
	for (i = 0; i < NUM_CTRLS; i++) {
		rvcam->ctrls[i] =
			v4l2_ctrl_new_std(&rvcam->ctrl_hdl,
					  &sta_rvcam_ctrl_ops,
					  controls[i].id, controls[i].minimum,
					  controls[i].maximum, controls[i].step,
					  controls[i].default_value);
		if (rvcam->ctrl_hdl.error) {
			dev_err(rvcam->v4l2_dev.dev,
				"Adding control (%d) failed\n", i);
			return rvcam->ctrl_hdl.error;
		}
		if (controls[i].is_volatile && rvcam->ctrls[i])
			rvcam->ctrls[i]->flags |= V4L2_CTRL_FLAG_VOLATILE;
	}

	rvcam->v4l2_ctrls_is_configured = true;
	return 0;
}

void sta_rvcam_delete_controls(struct sta_rvcam *rvcam)
{
	int i;

	if (rvcam->v4l2_ctrls_is_configured) {
		v4l2_ctrl_handler_free(&rvcam->ctrl_hdl);
		for (i = 0; i < NUM_CTRLS; i++)
			rvcam->ctrls[i] = NULL;
		rvcam->v4l2_ctrls_is_configured = false;
	}
}

static struct vb2_ops rvcam_video_qops = {
	.queue_setup		= rvcam_queue_setup,
	.buf_init		= rvcam_buffer_init,
	.buf_prepare		= rvcam_buffer_prepare,
	.buf_queue		= rvcam_buffer_queue,
	.start_streaming	= rvcam_start_streaming,
	.stop_streaming		= rvcam_stop_streaming,
	.wait_prepare		= vb2_ops_wait_prepare,
	.wait_finish		= vb2_ops_wait_finish,
};

static int sta_rvcam_v4l2_open(struct file *filp)
{
	struct video_device *video_dev = video_devdata(filp);
	struct sta_rvcam *rvcam = video_get_drvdata(video_dev);

	rvcam->format_cap.width = rvcam->width;
	rvcam->format_cap.height = rvcam->height;
	rvcam->format_cap.pixelformat = RVCAM_PIXFMT;
	rvcam->format_cap.field = rvcam->field;
	rvcam->format_cap.bytesperline = rvcam->width * RVCAM_BPP;
	rvcam->format_cap.sizeimage = rvcam->width * rvcam->height * RVCAM_BPP;
	rvcam->format_cap.colorspace = RVCAM_COLORSPACE;

	return v4l2_fh_open(filp);
}

static int sta_rvcam_v4l2_release(struct file *filp)
{
	int ret;

	ret = vb2_fop_release(filp);

	return ret;
}

/* File Operations */
static const struct v4l2_file_operations sta_rvcam_fops = {
	.owner = THIS_MODULE,
	.open = sta_rvcam_v4l2_open,
	.release = sta_rvcam_v4l2_release,
	.unlocked_ioctl = video_ioctl2,
	.read = vb2_fop_read,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll,
#ifndef CONFIG_MMU
	.get_unmapped_area = vb2_fop_get_unmapped_area,
#endif
};

/**
 * sta_rvcam_querycap - return capabilities of device
 * @file: descriptor of device
 * @cap: contains return values
 *
 * the capabilities of the device are returned
 *
 * return value: 0, no error.
 */
static int sta_rvcam_querycap(struct file *file, void *priv,
			      struct v4l2_capability *cap)
{
	const char rvcam_description[] = "sta Video Input Port";
	char bus_info[32] = "platform:";

	strcat(bus_info, DRV_NAME);
	strlcpy(cap->driver, DRV_NAME, sizeof(cap->driver));
	strlcpy(cap->card, rvcam_description, sizeof(cap->card));
	strlcpy(cap->bus_info, bus_info, sizeof(cap->bus_info));

	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE |
		V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

/**
 * sta_rvcam_s_std - set video standard
 * @file: descriptor of device
 * @std: contains standard to be set
 *
 * the video standard is set
 *
 * return value: 0, no error.
 *
 * -EINVAL, standard not supported
 *
 * other, returned from video decoder.
 */
static int sta_rvcam_s_std(struct file *file, void *priv, v4l2_std_id std)
{
	struct sta_rvcam *rvcam = video_drvdata(file);

	if (rvcam->std != std)
		return -EINVAL;

	return 0;
}

/**
 * sta_rvcam_g_std - get video standard
 * @file: descriptor of device
 * @std: contains return values
 *
 * the current video standard is returned
 *
 * return value: 0, no error.
 */
static int sta_rvcam_g_std(struct file *file, void *priv, v4l2_std_id *std)
{
	struct sta_rvcam *rvcam = video_drvdata(file);

	*std = rvcam->std;
	return 0;
}

/**
 * sta_rvcam_querystd - get possible video standards
 * @file: descriptor of device
 * @std: contains return values
 *
 * only standard detected by M3 is supported
 *
 * return value: delivered by video ADV routine.
 */
static int sta_rvcam_querystd(struct file *file, void *priv, v4l2_std_id *std)
{
	struct sta_rvcam *rvcam = video_drvdata(file);

	*std = rvcam->std;
	return 0;
}

static int sta_rvcam_enum_input(struct file *file, void *priv,
				struct v4l2_input *inp)
{
	struct sta_rvcam *rvcam = video_drvdata(file);

	if (inp->index >= 1)
		return -EINVAL;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = rvcam->std;
	sprintf(inp->name, "Camera %u", inp->index);

	return 0;
}

/**
 * sta_rvcam_s_input - set input line
 * @file: descriptor of device
 * @i: new input line number
 *
 * the current active input line is set
 *
 * return value: 0, no error.
 *
 * -EINVAL, line number out of range
 */
static int sta_rvcam_s_input(struct file *file, void *priv, unsigned int i)
{
	struct sta_rvcam *rvcam = video_drvdata(file);

	if (i != rvcam->input)
		return -EINVAL;

	return 0;
}

/**
 * sta_rvcam_g_input - return input line
 * @file: descriptor of device
 * @i: returned input line number
 *
 * the current active input line is returned
 *
 * return value: always 0.
 */
static int sta_rvcam_g_input(struct file *file, void *priv, unsigned int *i)
{
	struct sta_rvcam *rvcam = video_drvdata(file);

	*i = rvcam->input;
	return 0;
}

/**
 * sta_rvcam_enum_fmt_vid_cap - return video capture format
 * @f: returned format information
 *
 * returns name and format of video capture
 * For now, only RGB565 is supported
 *
 * return value: always 0.
 */
static int sta_rvcam_enum_fmt_vid_cap(struct file *file, void *priv,
				      struct v4l2_fmtdesc *f)
{
	/* No more output */
	if ((f->index < 0) || (f->index >= RVCAM_NUM_FORMATS))
		return -EINVAL;

	strlcpy(f->description, sta_cap_formats[f->index].name,
		sizeof(f->description));
	f->pixelformat = sta_cap_formats[f->index].fourcc;
	f->flags = 0;

	return 0;
}

/**
 * sta_rvcam_try_fmt_vid_cap - set video capture format
 * @file: descriptor of device
 * @f: new format
 *
 * return value: 0, no error
 *
 * -EINVAL, pixel or field format not supported
 *
 */
static int sta_rvcam_try_fmt_vid_cap(struct file *file, void *priv,
				     struct v4l2_format *f)
{
	struct sta_rvcam *rvcam = video_drvdata(file);
	int i;

	for (i = 0; i < RVCAM_NUM_FORMATS; i++) {
		if (sta_cap_formats[i].fourcc == f->fmt.pix.pixelformat)
			break;
	}

	if (i >= RVCAM_NUM_FORMATS) {
		v4l2_warn(&rvcam->v4l2_dev,
			  "Fourcc format (0x%08x) invalid.\n",
			  f->fmt.pix.pixelformat);
		return -EINVAL;
	}

	/* Start/End pixel position must be word aligned =>even width */
	f->fmt.pix.pixelformat = sta_cap_formats[i].fourcc;
	f->fmt.pix.width = rvcam->width;
	f->fmt.pix.height = rvcam->height;
	f->fmt.pix.bytesperline = f->fmt.pix.width * sta_cap_formats[i].bpp;
	f->fmt.pix.sizeimage = f->fmt.pix.bytesperline * f->fmt.pix.height;
	f->fmt.pix.colorspace = RVCAM_COLORSPACE;
	f->fmt.pix.field = rvcam->field;
	f->fmt.pix.priv = 0;
	return 0;
}

/**
 * sta_rvcam_s_fmt_vid_cap - set current video format parameters
 * @file: descriptor of device
 * @f: returned format information
 *
 * set new capture format
 * return value: 0, no error
 *
 * other, delivered by video decoder routine.
 */
static int sta_rvcam_s_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_format *f)
{
	struct sta_rvcam *rvcam = video_drvdata(file);
	int ret;

	ret = sta_rvcam_try_fmt_vid_cap(file, priv, f);
	if (ret)
		return ret;

	if (vb2_is_busy(&rvcam->vq)) {
		/* Can't change format during acquisition */
		v4l2_err(&rvcam->v4l2_dev, "device busy\n");
		return -EBUSY;
	}

	rvcam->format_cap = f->fmt.pix;

	return 0;
}

/**
 * sta_rvcam_g_fmt_vid_cap - get current video format parameters
 * @file: descriptor of device
 * @f: contains format information
 *
 * returns current video format parameters
 *
 * return value: 0, always successful
 */
static int sta_rvcam_g_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_format *f)
{
	struct sta_rvcam *rvcam = video_drvdata(file);

	f->fmt.pix = rvcam->format_cap;

	return 0;
}

static int sta_rvcam_enum_framesizes(struct file *filp, void *priv,
				     struct v4l2_frmsizeenum *sizes)
{
	struct video_device *video_dev = video_devdata(filp);
	struct sta_rvcam *rvcam = video_get_drvdata(video_dev);

	if (sizes->index != 0)
		return -EINVAL;

	sizes->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	sizes->discrete.width  = rvcam->width;
	sizes->discrete.height = rvcam->height;

	return 0;
}

static int sta_rvcam_subscribe_event(struct v4l2_fh *fh,
				     const struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subscribe(fh, sub);
	case V4L2_EVENT_EOS:
		return v4l2_event_subscribe(fh, sub, 2, NULL);
	}
	return v4l2_ctrl_subscribe_event(fh, sub);
}

static const struct v4l2_ioctl_ops sta_rvcam_ioctl_ops = {
	.vidioc_querycap = sta_rvcam_querycap,
	/* FMT handling */
	.vidioc_enum_fmt_vid_cap = sta_rvcam_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap = sta_rvcam_g_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap = sta_rvcam_s_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap = sta_rvcam_try_fmt_vid_cap,
	/* Frame sizes */
	.vidioc_enum_framesizes = sta_rvcam_enum_framesizes,
	/* Buffer handlers */
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_prepare_buf = vb2_ioctl_prepare_buf,
	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_expbuf = vb2_ioctl_expbuf,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	/* Stream on/off */
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,
	/* Standard handling */
	.vidioc_g_std = sta_rvcam_g_std,
	.vidioc_s_std = sta_rvcam_s_std,
	.vidioc_querystd = sta_rvcam_querystd,
	/* Input handling */
	.vidioc_enum_input = sta_rvcam_enum_input,
	.vidioc_g_input = sta_rvcam_g_input,
	.vidioc_s_input = sta_rvcam_s_input,
	/* Log status ioctl */
	.vidioc_log_status = v4l2_ctrl_log_status,
	/* Event handling */
	.vidioc_subscribe_event = sta_rvcam_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
};

static struct video_device sta_rvcam_vdev_template = {
	.name = DRV_NAME,
	.tvnorms = V4L2_STD_ALL,
	.fops = &sta_rvcam_fops,
	.device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_READWRITE |
		       V4L2_CAP_STREAMING,
	.ioctl_ops = &sta_rvcam_ioctl_ops,
	.release = video_device_release,
};

static int sta_rvcam_init_buffer(struct sta_rvcam *rvcam)
{
	int err;

	INIT_LIST_HEAD(&rvcam->capture);
	mutex_init(&rvcam->mutex);
	mutex_init(&rvcam->buf_list_lock);

	memset(&rvcam->vq, 0, sizeof(struct vb2_queue));
	rvcam->vq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rvcam->vq.io_modes = VB2_MMAP | VB2_READ | VB2_USERPTR | VB2_DMABUF;
	rvcam->vq.drv_priv = rvcam;
	rvcam->vq.buf_struct_size = sizeof(struct rvcam_buffer);
	rvcam->vq.ops = &rvcam_video_qops;
	rvcam->vq.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	rvcam->vq.mem_ops = &vb2_dma_contig_memops;
	rvcam->vq.lock = &rvcam->mutex;
	rvcam->vq.dev = &rvcam->pdev->dev;
	err = vb2_queue_init(&rvcam->vq);
	if (err)
		return err;

	return 0;
}

static void sta_rvcam_release_buffer(struct sta_rvcam *rvcam)
{
	vb2_queue_release(&rvcam->vq);
}

static void sta_rvcam_notify(struct v4l2_subdev *sd,
			     unsigned int notification, void *arg)
{
	struct sta_rvcam *rvcam =
		container_of(sd->v4l2_dev, struct sta_rvcam, v4l2_dev);

	switch (notification) {
	case V4L2_DEVICE_NOTIFY_EVENT:
		dev_dbg(rvcam->v4l2_dev.dev, "V4L2_DEVICE_NOTIFY_EVENT\n");
		v4l2_event_queue(rvcam->video_dev, arg);
		break;
	default:
		break;
	}
}

static int sta_rvcam_v4l2_probe(struct sta_rvcam *rvcam)
{
	int ret = 0;

	ret = sta_rvcam_init_controls(rvcam);
	if (ret) {
		dev_err(rvcam->v4l2_dev.dev, "failed to init v4l2 controls\n");
		return ret;
	}

	rvcam->v4l2_dev.notify = sta_rvcam_notify;

	ret = video_register_device(rvcam->video_dev, VFL_TYPE_GRABBER, -1);
	if (ret) {
		rvcam->v4l2_dev.notify = NULL;
		video_device_release(rvcam->video_dev);
		dev_err(rvcam->v4l2_dev.dev, "failed to register rvcam device\n");
		return ret;
	}
	video_set_drvdata(rvcam->video_dev, rvcam);

	dev_info(rvcam->v4l2_dev.dev,
		 "registered as /dev/video%d\n", rvcam->video_dev->num);

	return 0;
}

static void sta_rvcam_v4l2_remove(struct sta_rvcam *rvcam)
{
	dev_dbg(rvcam->v4l2_dev.dev, "Removing %s\n",
		video_device_node_name(rvcam->video_dev));

	v4l2_ctrl_handler_free(&rvcam->ctrl_hdl);
	rvcam->v4l2_dev.notify = NULL;
	video_unregister_device(rvcam->video_dev);
}

static void sta_rvcam_buffer_done(struct sta_rvcam *rvcam,
				  struct vb2_buffer *vb)
{
	struct vb2_v4l2_buffer *vb2_v4l2_buf;
	struct rvcam_buffer *rvcam_buf, *node;
	bool buffer_found = false;
	u64 duration = 0;

	dev_dbg(rvcam->v4l2_dev.dev, "sta_rvcam_buffer_done: vb=%p\n", vb);

	mutex_lock(&rvcam->buf_list_lock);

	if (!vb2_is_streaming(&rvcam->vq)) {
		mutex_unlock(&rvcam->buf_list_lock);
		return;
	}

	/* Delete buffer from the list */
	list_for_each_entry_safe(rvcam_buf, node, &rvcam->capture, queue) {
		if (vb == &rvcam_buf->vbuf.vb2_buf) {
			list_del_init(&rvcam_buf->queue);
			buffer_found = true;
		}
	}

	if (!buffer_found) {
		dev_err(rvcam->v4l2_dev.dev, "BUFFER NOT FOUND !!!\n");
		mutex_unlock(&rvcam->buf_list_lock);
		return;
	}

	if (rvcam->state == RVCAM_STATE_FATAL_ERROR) {
		vb2_buffer_done(vb, VB2_BUF_STATE_ERROR);
		mutex_unlock(&rvcam->buf_list_lock);
		return;
	}

	vb2_v4l2_buf = to_vb2_v4l2_buffer(vb);
	/* TODO: get time from Mx processor when buffer was filled */
	vb2_v4l2_buf->vb2_buf.timestamp = ktime_get_ns();
	vb2_v4l2_buf->sequence = rvcam->sequence++;
	vb2_v4l2_buf->field = rvcam->field;

	switch (rvcam->sequence) {
	case 1:
		rvcam->perf.start_streaming_ns =
				vb2_v4l2_buf->vb2_buf.timestamp;
		break;
	case 2:
		duration = vb2_v4l2_buf->vb2_buf.timestamp -
			   rvcam->perf.end_streaming_ns;
		rvcam->perf.min_duration = duration;
		rvcam->perf.max_duration = duration;
		break;
	default:
		duration = vb2_v4l2_buf->vb2_buf.timestamp -
			   rvcam->perf.end_streaming_ns;
		rvcam->perf.min_duration =
				min(duration, rvcam->perf.min_duration);
		rvcam->perf.max_duration =
				max(duration, rvcam->perf.max_duration);
		break;
	}

	rvcam->perf.end_streaming_ns = vb2_v4l2_buf->vb2_buf.timestamp;

#ifdef DEBUG
	memset(vb2_plane_vaddr(&vb2_v4l2_buf->vb2_buf, 0), 0xc8,
	       vb2_plane_size(vb, 0) / 32);
#endif

	vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
	mutex_unlock(&rvcam->buf_list_lock);
}

static void sta_rvcam_update_config(struct sta_rvcam *rvcam,
				    struct rvcam_configuration_msg config_msg)
{
	dev_dbg(rvcam->v4l2_dev.dev,
		"RVC_GET_CONFIGURATION_ACK %d %d %d %d %d\n",
		config_msg.cam_width,
		config_msg.cam_height,
		config_msg.cam_std,
		config_msg.cam_input,
		config_msg.cam_field);

	/* Camera resolution */
	rvcam->width = config_msg.cam_width;
	rvcam->height = config_msg.cam_height;

	/* Camera standard */
	switch (config_msg.cam_std) {
	case RVC_STD_PAL:
		rvcam->std = V4L2_STD_PAL;
		break;
	case RVC_STD_NTSC:
		rvcam->std = V4L2_STD_NTSC;
		break;
	case RVC_STD_UNKNOWN:
	default:
		rvcam->std = V4L2_STD_UNKNOWN;
		break;
	}

	/* Camera field */
	switch (config_msg.cam_field) {
	case RVC_FIELD_NONE:
		rvcam->field = V4L2_FIELD_NONE;
		break;
	case RVC_FIELD_TOP:
		rvcam->field = V4L2_FIELD_TOP;
		break;
	case RVC_FIELD_BOTTOM:
		rvcam->field = V4L2_FIELD_BOTTOM;
		break;
	case RVC_FIELD_INTERLACED:
		rvcam->field = V4L2_FIELD_INTERLACED;
		break;
	case RVC_FIELD_SEQ_TB:
		rvcam->field = V4L2_FIELD_SEQ_TB;
		break;
	case RVC_FIELD_SEQ_BT:
		rvcam->field = V4L2_FIELD_SEQ_BT;
		break;
	case RVC_FIELD_ALTERNATE:
		rvcam->field = V4L2_FIELD_ALTERNATE;
		break;
	case RVC_FIELD_INTERLACED_TB:
		rvcam->field = V4L2_FIELD_INTERLACED_TB;
		break;
	case RVC_FIELD_INTERLACED_BT:
		rvcam->field = V4L2_FIELD_INTERLACED_BT;
		break;
	default:
		rvcam->field = V4L2_FIELD_NONE;
		break;
	}

	/* Camera input */
	rvcam->input = config_msg.cam_input;
}

int sta_rvcam_rpmsg_endpoint_cb(struct rpmsg_mm_msg_hdr *data, void *priv)
{
	struct sta_rvcam *rvcam = (struct sta_rvcam *)priv;
	struct rvcam_msg *msg;
	struct vb2_buffer *vb2_buf;

	if (!data)
		return -1;

	dev_dbg(rvcam->v4l2_dev.dev,
		"sta_rvcam_rpmsg_endpoint_cb data->info: %d\n",
		data->info);

	switch (data->info) {
	case RPMSG_MM_COMM_AVAILABLE:
		if (rvcam->wait_for_rpmsg_available)
			complete(&rvcam->rpmsg_connected);
		rvcam->wait_for_rpmsg_available = false;
		break;
	case RPMSG_MM_PRIVATE_MESSAGE:
		if (rvcam->wait_for_rpmsg_available)
			complete(&rvcam->rpmsg_connected);
		rvcam->wait_for_rpmsg_available = false;

		msg = (struct rvcam_msg *)data->data;
		switch (msg->info) {
		case RVC_BUFFER_FILLED:
			vb2_buf = (struct vb2_buffer *)
				msg->m.buffer_filled_msg.vb2_buf;
			sta_rvcam_buffer_done(rvcam, vb2_buf);
			break;
		case RVC_GET_CONFIGURATION_ACK:
			sta_rvcam_update_config(rvcam, msg->m.config_msg);
			complete(&rvcam->rvc_config_received);
			break;
		}
		break;
	case RPMSG_MM_COMM_UNAVAILABLE:
	default:
		break;
	}
	return 0;
}

static int sta_rvcam_get_rvc_configuration(struct sta_rvcam *rvcam)
{
	int ret;
	struct rvcam_msg msg;

	msg.info = RVC_GET_CONFIGURATION_REQ;

	ret = sta_rpmsg_send_private_message(rvcam->rpmsg_handle,
					     RPSMG_MM_EPT_CORTEXM_RVC,
					     (void *)&msg,
					     sizeof(struct rvcam_msg));
	if (ret) {
		dev_err(rvcam->v4l2_dev.dev,
			"sta_rpmsg_send_private_message failed: %d\n",
			ret);
		return ret;
	}

	if (!wait_for_completion_timeout(
		&rvcam->rvc_config_received,
		msecs_to_jiffies(1000))) { /* Timeout after 1sec */
		dev_err(rvcam->v4l2_dev.dev, "[RPMSG] : TIMEOUT!!!\n");
		return -EIO;
	}

	return 0;
}

static void sta_rvcam_resume_work(struct work_struct *work)
{
	int ret;
	struct sta_rvcam *rvcam =
		container_of(work, struct sta_rvcam, resume_work);

	if (rvcam->wait_for_rpmsg_available) {
		if (!wait_for_completion_timeout(
			&rvcam->rpmsg_connected,
			msecs_to_jiffies(5000))) { /* Timeout after 5sec */
			dev_err(rvcam->v4l2_dev.dev, "[RPMSG] : TIMEOUT!!!\n");
			goto fatal_error;
		}
	}
	rvcam->wait_for_rpmsg_available = false;

	ret = sta_rvcam_get_rvc_configuration(rvcam);
	if (ret) {
		dev_err(rvcam->v4l2_dev.dev,
			"Failed to get RVC configuration\n");
		rvcam->state = RVCAM_STATE_FATAL_ERROR;
		return;
	}

	dev_dbg(rvcam->v4l2_dev.dev, "[RPMSG] : RVCAM resume OK\n");
	rvcam->probe_status = RVCAM_PROBE_DONE;
	return;

fatal_error:
	dev_dbg(rvcam->v4l2_dev.dev, "[RPMSG] : RVCAM resource unavailable\n");
	rvcam->wait_for_rpmsg_available = false;
	rvcam->state = RVCAM_STATE_FATAL_ERROR;
}

static void sta_rvcam_init_work(struct work_struct *work)
{
	int ret;
	struct sta_rvcam *rvcam =
		container_of(work, struct sta_rvcam, init_work);

	ret = sta_rvcam_get_rvc_configuration(rvcam);
	if (ret) {
		dev_err(rvcam->v4l2_dev.dev,
			"Failed to get RVC configuration\n");
		rvcam->state = RVCAM_STATE_FATAL_ERROR;
		return;
	}

	sta_rvcam_v4l2_probe(rvcam);
	rvcam->probe_status = RVCAM_PROBE_DONE;
}

static int sta_rvcam_probe(struct platform_device *pdev)
{
	struct sta_rvcam *rvcam;
	struct device_node *np = pdev->dev.of_node;
	int ret;

	if (!np) {
		dev_err(&pdev->dev, "Device tree support is mandatory\n");
		return -EINVAL;
	}

	rvcam = devm_kzalloc(&pdev->dev, sizeof(struct sta_rvcam), GFP_KERNEL);

	vb2_dma_contig_set_max_seg_size(&pdev->dev, DMA_BIT_MASK(32));

	if (IS_ERR(rvcam->alloc_ctx)) {
		ret = PTR_ERR(rvcam->alloc_ctx);
		return ret;
	}

	rvcam->pdev = pdev;
	rvcam->probe_status = RVCAM_PROBE_IDLE;

	ret = v4l2_device_register(&pdev->dev, &rvcam->v4l2_dev);
	if (ret)
		goto exit_v4l2_ctrls_setup;

	/* Initialize buffer */
	ret = sta_rvcam_init_buffer(rvcam);
	if (ret)
		goto exit_v4l2_device_unregister;

	/* Alloc, initialize and register video device */
	rvcam->video_dev = video_device_alloc();
	if (!rvcam->video_dev) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "failed to alloc video device\n");
		goto exit_release_buf;
	}

	*rvcam->video_dev = sta_rvcam_vdev_template;
	rvcam->video_dev->v4l2_dev = &rvcam->v4l2_dev;
	rvcam->video_dev->queue = &rvcam->vq;
	rvcam->video_dev->lock = &rvcam->mutex;

	sta_rvcam_debugfs_create(rvcam);

	rvcam->v4l2_ctrls_is_configured = false;

	rvcam->rpmsg_handle = sta_rpmsg_register_endpoint(
						RPSMG_MM_EPT_CORTEXA_RVC,
						sta_rvcam_rpmsg_endpoint_cb,
						(void *)rvcam);

	if (!rvcam->rpmsg_handle) {
		dev_err(&pdev->dev, "Unable to register rpmsg_mm\n");
		goto exit_release_buf;
	}

	rvcam->wait_for_rpmsg_available = false;

	rvcam->rpmsg_wq = create_singlethread_workqueue("rvcam_rpmsg_wq");
	if (!rvcam->rpmsg_wq)
		return -ENOMEM;

	init_completion(&rvcam->rpmsg_connected);
	init_completion(&rvcam->rvc_config_received);

	INIT_WORK(&rvcam->resume_work, sta_rvcam_resume_work);
	INIT_WORK(&rvcam->init_work, sta_rvcam_init_work);

	rvcam->probe_status = RVCAM_PROBE_POSTPONED;
	queue_work(rvcam->rpmsg_wq, &rvcam->init_work);

	return 0;

exit_release_buf:
	sta_rvcam_release_buffer(rvcam);
exit_v4l2_device_unregister:
	v4l2_device_unregister(&rvcam->v4l2_dev);
exit_v4l2_ctrls_setup:
	sta_rvcam_delete_controls(rvcam);
	return ret;
}

static int sta_rvcam_remove(struct platform_device *pdev)
{
	struct v4l2_device *v4l2_dev = platform_get_drvdata(pdev);
	struct sta_rvcam *rvcam =
		container_of(v4l2_dev, struct sta_rvcam, v4l2_dev);

	sta_rpmsg_unregister_endpoint(rvcam->rpmsg_handle);
	rvcam->rpmsg_handle = NULL;

	complete(&rvcam->rpmsg_connected);
	complete(&rvcam->rvc_config_received);
	cancel_work_sync(&rvcam->init_work);
	cancel_work_sync(&rvcam->resume_work);
	destroy_workqueue(rvcam->rpmsg_wq);

	sta_rvcam_debugfs_remove(rvcam);
	sta_rvcam_v4l2_remove(rvcam);
	video_set_drvdata(rvcam->video_dev, NULL);
	sta_rvcam_release_buffer(rvcam);
	v4l2_device_unregister(&rvcam->v4l2_dev);
	sta_rvcam_delete_controls(rvcam);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sta_rvcam_suspend(struct device *dev)
{
	struct sta_rvcam *rvcam = dev_get_drvdata(dev);

	dev_dbg(dev, "suspend\n");

	if (!rvcam) {
		dev_err(dev, "rvcam == NULL\n");
		return 0;
	}

	if (vb2_is_streaming(&rvcam->vq)) {	/* streaming is on */
		const struct v4l2_event eos_event = {
			.type = V4L2_EVENT_EOS
		};

		v4l2_event_queue(rvcam->video_dev, &eos_event);
		rvcam->state = RVCAM_STATE_FATAL_ERROR;
	}

	if (rvcam->probe_status == RVCAM_PROBE_POSTPONED)
		cancel_work_sync(&rvcam->init_work);

	if (rvcam->probe_status == RVCAM_PROBE_RESUME_POSTPONED)
		cancel_work_sync(&rvcam->resume_work);

	return 0;
}

static int sta_rvcam_resume(struct device *dev)
{
	struct sta_rvcam *rvcam = dev_get_drvdata(dev);

	dev_dbg(dev, "resume\n");

	if (!rvcam) {
		dev_err(dev, "rvcam == NULL\n");
		return 0;
	}

	if (rvcam->probe_status == RVCAM_PROBE_DONE) {
		rvcam->probe_status = RVCAM_PROBE_RESUME_POSTPONED;
		rvcam->wait_for_rpmsg_available = true;
		reinit_completion(&rvcam->rpmsg_connected);
		reinit_completion(&rvcam->rvc_config_received);
		queue_work(rvcam->rpmsg_wq, &rvcam->resume_work);
	}

	return 0;
}

static const struct dev_pm_ops sta_rvcam_pm_ops = {
	.suspend = sta_rvcam_suspend,
	.resume = sta_rvcam_resume,
};

#define STA_RVCAM_PM_OPS (&sta_rvcam_pm_ops)

#else /* CONFIG_PM_SLEEP not defined */
#define STA_RVCAM_PM_OPS NULL
#endif

static const struct of_device_id rvcam_of_match[] = {
	{ .compatible = "st,sta_rvcam" },
	{ }
};
MODULE_DEVICE_TABLE(of, rvcam_of_match);

static struct platform_driver sta_rvcam_driver = {
	.driver = {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
		.pm	= STA_RVCAM_PM_OPS,
		.of_match_table = rvcam_of_match,
	},
	.probe		= sta_rvcam_probe,
	.remove		= sta_rvcam_remove,
};

static int __init sta_rvcam_init(void)
{
	return platform_driver_register(&sta_rvcam_driver);
}

static void __exit sta_rvcam_exit(void)
{
	platform_driver_unregister(&sta_rvcam_driver);
}

#ifdef MODULE
module_init(sta_rvcam_init);
module_exit(sta_rvcam_exit);
#else
late_initcall_sync(sta_rvcam_init);
#endif

MODULE_DESCRIPTION("STA Rear View Camera driver");
MODULE_AUTHOR("ST Automotive Group");
MODULE_AUTHOR("Mustapha Ghanmi <mustapha.ghanmi@st.com>");
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");
