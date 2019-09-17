/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Fabrice Lecoultre <fabrice.lecoultre@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/debugfs.h>

#include "g1.h"
#include "g1-debug.h"

#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define CLIP(X) (((X) < U32_MAX) ? (X) : 0)

char *g1_streaminfo_str(struct g1_streaminfo *s, char *str, unsigned int len)
{
	char *cur = str;
	size_t left = len;
	int ret = 0;
	int cnt = 0;

	if (!s)
		return NULL;

	ret = snprintf(cur, left,
		       "%4.4s %s %s frame dpb=%d %s  %dx%d",
		       (char *)&s->streamformat,
		       s->profile, s->level, s->dpb,
		       (s->field == V4L2_FIELD_NONE) ?
		       "progressive" : "interlaced",
		       s->width, s->height);
	cnt = (left > ret ? ret : left);

	if ((s->width != s->aligned_width) ||
	    (s->height != s->aligned_height)) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " aligned %dx%d",
			       s->aligned_width, s->aligned_height);
		cnt = (left > ret ? ret : left);
	}

	if (s->flags & G1_STREAMINFO_FLAG_CROP) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " crop=%dx%d@(%d,%d)",
			       s->crop.width, s->crop.height,
			       s->crop.left, s->crop.top);
		cnt = (left > ret ? ret : left);
	}

	if (s->flags & G1_STREAMINFO_FLAG_PIXELASPECT) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " pixel aspect=%d/%d",
			       s->pixelaspect.numerator,
			       s->pixelaspect.denominator);
		cnt = (left > ret ? ret : left);
	}

	if (s->flags & G1_STREAMINFO_FLAG_OTHER) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, " %s", s->other);
		cnt = (left > ret ? ret : left);
	}

	return str;
}

char *g1_frameinfo_str(struct g1_frameinfo *f, char *str, unsigned int len)
{
	char *cur = str;
	size_t left = len;
	int ret = 0;
	int cnt = 0;

	if (!f)
		return NULL;

	ret = snprintf(cur, left,
		       "%4.4s%s%dx%d",
		       (char *)&f->pixelformat,
		       (f->colorspace == V4L2_COLORSPACE_JPEG ?
		       "(full color range) " : " "),
		       f->width, f->height);
	cnt = (left > ret ? ret : left);

	if ((f->width != f->aligned_width) ||
	    (f->height != f->aligned_height)) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " aligned %dx%d",
			       f->aligned_width, f->aligned_height);
		cnt = (left > ret ? ret : left);
	}

	if (f->flags & G1_FRAMEINFO_FLAG_CROP) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " crop=%dx%d@(%d,%d)",
			       f->crop.width, f->crop.height,
			       f->crop.left, f->crop.top);
		cnt = (left > ret ? ret : left);
	}

	if (f->flags & G1_FRAMEINFO_FLAG_PIXELASPECT) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " pixel aspect=%d/%d",
			       f->pixelaspect.numerator,
			       f->pixelaspect.denominator);
		cnt = (left > ret ? ret : left);
	}
	if (f->ctrls.is_hflip_requested || f->ctrls.is_vflip_requested) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       " rotation=%s",
			       f->ctrls.is_hflip_requested ? "hflip" : "vflip");
		cnt = (left > ret ? ret : left);
	}
	return str;
}

char *g1_summary_str(struct g1_ctx *ctx, char *str, unsigned int len)
{
	char *cur = str;
	size_t left = len;
	int cnt = 0;
	int ret = 0;
	unsigned char sstr[200] = "";

	if (!(ctx->flags & G1_FLAG_STREAMINFO))
		return NULL;

	ret = snprintf(cur, left,
		       "%s",
		       g1_streaminfo_str(&ctx->streaminfo, sstr, sizeof(sstr)));
	cnt = (left > ret ? ret : left);

	if (ctx->mode == G1_MODE_LOW_LATENCY &&
	    ctx->streaminfo.streamformat == V4L2_PIX_FMT_H264) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left, ", Low Latency mode");
		cnt = (left > ret ? ret : left);
	}

	if (ctx->flags & G1_FLAG_FRAMEINFO) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", output frame %s",
			       g1_frameinfo_str(&ctx->frameinfo,
						sstr, sizeof(sstr)));
		cnt = (left > ret ? ret : left);
	}

	if (ctx->decoded_frames) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", %d frames decoded", ctx->decoded_frames);
		cnt = (left > ret ? ret : left);
	}

	if (ctx->output_frames) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", %d frames output", ctx->output_frames);
		cnt = (left > ret ? ret : left);
	}

	if (ctx->dropped_frames) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", %d frames dropped", ctx->dropped_frames);
		cnt = (left > ret ? ret : left);
	}

	if (ctx->stream_errors) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", %d stream errors", ctx->stream_errors);
		cnt = (left > ret ? ret : left);
	}

	if (ctx->decode_errors) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", %d decode errors", ctx->decode_errors);
		cnt = (left > ret ? ret : left);
	}

	if (ctx->total_duration && ctx->decoded_frames) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ", max fps(0.1Hz)=%d",
			       100000 * ctx->decoded_frames /
			       ctx->total_duration);
		cnt = (left > ret ? ret : left);
	}

	return str;
}

void g1_perf_begin(struct g1_ctx *ctx, struct g1_au *au)
{
	struct timeval begin, prev;
	u64 period = 0;
	u64 bitrate = 0;
	struct g1_dev *g1 = ctx->dev;

	prev = ctx->begin;
	do_gettimeofday(&begin);
	if (ctx->is_valid_period) {
		/* period for decoding framerate
		 * (! different of rendering framerate)
		 */
		period = ((begin.tv_sec - prev.tv_sec) * 1000000 +
			  (begin.tv_usec - prev.tv_usec));
		do_div(period, 100);
		ctx->min_period = MIN(period, ctx->min_period);
		ctx->max_period = MAX(period, ctx->max_period);
		ctx->total_period += period;
		ctx->cnt_period++;

		/* bitrate min/max are based on
		 * decoding period on a 32 samples window
		 */
		ctx->window_au_size += au->size;
		ctx->window_duration += period;
		ctx->window_cnt++;
		if (ctx->window_cnt >= 32) {
			bitrate = ctx->window_au_size * 80;
			do_div(bitrate, ctx->window_duration);
			ctx->bitrate = bitrate;

			/* instantaneous bitrate */
			dev_dbg(g1->dev, "%s  bitrate[%d] = %lld\n", ctx->name,
				ctx->decoded_frames, bitrate);

			ctx->min_bitrate = MIN(bitrate, ctx->min_bitrate);
			ctx->max_bitrate = MAX(bitrate, ctx->max_bitrate);
			ctx->window_au_size = 0;
			ctx->window_duration = 0;
			ctx->window_cnt = 0;
		}
	}

	/* filter invalid perf, for ex begin/begin
	 * (no frame available) to only have
	 * begin/end/begin/end which correspond
	 * to valid case
	 */
	ctx->is_valid_period = 0;

	/* avg bitrate based on
	 * total AU size & total period
	 */
	if (ctx->cnt_duration > 30)
		ctx->total_au_size += au->size;

	ctx->begin = begin;
	if (ctx->first_au.tv_sec == 0 && ctx->first_au.tv_usec == 0)
		ctx->first_au = begin;
}

void g1_perf_end(struct g1_ctx *ctx)
{
	struct timeval begin, end;
	u64 duration;

	begin = ctx->begin;

	if ((begin.tv_sec * 1000000 + begin.tv_usec) == 0)
		return;

	do_gettimeofday(&end);
	duration = ((end.tv_sec - begin.tv_sec) * 1000000 +
		    (end.tv_usec - begin.tv_usec));
	do_div(duration, 100);
	ctx->min_duration = MIN(duration, ctx->min_duration);
	ctx->max_duration = MAX(duration, ctx->max_duration);
	ctx->total_duration += duration;
	ctx->cnt_duration++;

	/* do not take startup periods (preroll...) */
	if (ctx->cnt_duration > 30)
		ctx->is_valid_period = 1;
}

void g1_perf_output(struct g1_ctx *ctx)
{
	struct timeval tick, previous;
	struct timeval first_au;
	u64 period;

	previous = ctx->output_tick;
	do_gettimeofday(&tick);
	ctx->output_tick = tick;
	if (ctx->output_frames <= 1) {
		ctx->out_avg_fps = 0;
		ctx->out_avg_period = 0;
		ctx->out_total_period = 0;
		/* Compute latency to first available decoded frame */
		first_au = ctx->first_au;
		ctx->ttf_decode = ((tick.tv_sec - first_au.tv_sec) * 1000 +
		    ((tick.tv_usec - first_au.tv_usec) / 1000));
		return;
	}
	period = ((tick.tv_sec - previous.tv_sec) * 1000000 +
		    (tick.tv_usec - previous.tv_usec));
	do_div(period, 100);
	ctx->out_total_period += period;
}

static void perf_compute(struct g1_ctx *ctx)
{
	u64 div;

	if (ctx->total_duration > 0) {
		div = ctx->total_duration;
		do_div(div, ctx->cnt_duration);
		ctx->avg_duration = div;
	} else {
		ctx->avg_duration = 0;
	}

	if (ctx->avg_duration > 0) {
		div = 100000;
		do_div(div, ctx->avg_duration);
		ctx->max_fps = div;
	} else {
		ctx->max_fps = 0;
	}

	if (ctx->total_period > 0) {
		div = ctx->total_period;
		do_div(div, ctx->cnt_period);
		ctx->avg_period = div;
	} else {
		ctx->avg_period = 0;
	}

	if (ctx->avg_period > 0) {
		div = 100000;
		do_div(div, ctx->avg_period);
		ctx->avg_fps = div;
	} else {
		ctx->avg_fps = 0;
	}

	if (ctx->total_period > 0) {
		/* bitrate = video size/video duration */
		div = ctx->total_au_size * 80;	/* 8 bit in 1/10 ms */
		do_div(div, ctx->total_period);
		ctx->avg_bitrate = div;
	} else {
		ctx->avg_bitrate = 0;
	}

	if (ctx->out_total_period > 0 && ctx->output_frames > 1) {
		div = ctx->out_total_period;
		do_div(div, ctx->output_frames - 1);
		ctx->out_avg_period = div;
	} else {
		ctx->out_avg_period = 0;
	}

	if (ctx->out_avg_period > 0) {
		div = 100000;
		do_div(div, ctx->out_avg_period);
		ctx->out_avg_fps = div;
	} else {
		ctx->out_avg_fps = 0;
	}
}

int g1_debugfs_track_mem(struct mem_info *buf, struct g1_ctx *ctx,
			 const unsigned char *type)
{
	struct mem_info *mem_info = NULL;
	struct mem_info *mem_info_e = NULL;

	mem_info_e = kzalloc(sizeof(*mem_info_e), GFP_KERNEL);
	if (!mem_info_e)
		return -ENOMEM;

	INIT_LIST_HEAD(&mem_info_e->list);

	if (buf->size == 0)
		return 0;

	ctx->total_mem += buf->size;

	/* each time an alloc is called, the name is checked to see any
	 * previous occurrence. if there's one, mem_info is freed, else,
	 * we add it at the tail of the list
	 */
	list_for_each_entry(mem_info, &ctx->mem_infos, list) {
		if (!strncmp(mem_info->name, buf->name, strlen(buf->name))) {
			if (buf->size == mem_info->size) {
				mem_info->nb_of_allocations++;
				kfree(mem_info_e);
				return 0;
			}
		}
	}

	mem_info_e->type = type;
	mem_info_e->size = buf->size;
	mem_info_e->nb_of_allocations++;
	strncpy(mem_info_e->name, buf->name, strlen(buf->name));

	list_add_tail(&mem_info_e->list, &ctx->mem_infos);

	return 0;
}

unsigned char *mem_status(struct g1_ctx *ctx)
{
	struct mem_info *mem_info = NULL;
	char *cur = ctx->mem_str;
	size_t left = sizeof(ctx->mem_str);
	int cnt = 0;
	int ret = 0;

	list_for_each_entry(mem_info, &ctx->mem_infos, list) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  | |- (%3s) %3d x %10d  %s\n",
			       mem_info->type,
			       mem_info->nb_of_allocations,
			       mem_info->size, mem_info->name);
		cnt = (left > ret ? ret : left);
	}

	if (ret > left)
		snprintf(cur - 4, 4, "...");

	return ctx->mem_str;
}

static unsigned char *ctx_dump_str(struct g1_ctx *ctx, char *str,
				   unsigned int len, struct g1_dev *g1)
{
	int error = 0;
	char *cur = str;
	size_t left = len;
	int cnt = 0;
	int ret = 0;

	ret = snprintf(cur, left,
		       "|-%s\n"
		       "  |-[config]\n"
		       "  | |- max au size=%d\n"
		       "  | |- allocated frames=%d\n"
		       "  | |- mode=%s\n"
		       "  |\n",
		       ctx->name,
		       ctx->max_au_size,
		       ctx->allocated_frames,
		       (ctx->mode == G1_MODE_LOW_LATENCY ?
			"low latency" : "normal"));
	cnt = (left > ret ? ret : left);

	if (ctx->flags & G1_FLAG_STREAMINFO) {
		struct g1_streaminfo *s = &ctx->streaminfo;

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  |-[stream infos]\n"
			       "  | |- %4.4s %dx%d %s %s\n"
			       "  | |- dpb=%d\n"
			       "  | |- %s\n",
			       (char *)&s->streamformat,
			       s->width,
			       s->height,
			       s->profile,
			       s->level,
			       s->dpb,
			       (s->field == V4L2_FIELD_NONE) ?
			       "progressive" : "interlaced");
		cnt = (left > ret ? ret : left);

		if (s->flags & G1_STREAMINFO_FLAG_CROP) {
			cur += cnt;
			left -= cnt;
			ret = snprintf(cur, left,
				       "  | |- crop=(%dx%d)@(%d,%d)\n",
				       s->crop.width,
				       s->crop.height,
				       s->crop.left, s->crop.top);
			cnt = (left > ret ? ret : left);
		}
	}

	if (ctx->flags & G1_FLAG_FRAMEINFO) {
		struct g1_frameinfo *f = &ctx->frameinfo;

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  |\n"
			       "  |-[frame infos]\n"
			       "  | |- %4.4s %dx%d\n"
			       "  | |- aligned wxh=%dx%d\n"
			       "  | |- contrast=%d\n"
			       "  | |- brightness=%d\n"
			       "  | |- saturation=%d\n"
			       "  | |- alpha=%d\n",
			       (char *)&f->pixelformat,
			       f->width,
			       f->height,
			       f->aligned_width,
			       f->aligned_height,
			       f->ctrls.contrast, f->ctrls.brightness,
			       f->ctrls.saturation, f->ctrls.alpha);
		cnt = (left > ret ? ret : left);

		if (f->flags & G1_FRAMEINFO_FLAG_CROP) {
			cur += cnt;
			left -= cnt;
			ret = snprintf(cur, left,
				       "  | |- crop=(%dx%d)@(%d,%d)\n",
				       f->crop.width, f->crop.height,
				       f->crop.left, f->crop.top);
			cnt = (left > ret ? ret : left);
		}

		if (f->colorspace == V4L2_COLORSPACE_JPEG) {
			cur += cnt;
			left -= cnt;
			ret = snprintf(cur, left,
				       "  | |- colorspace=full_range\n");
			cnt = (left > ret ? ret : left);
		}

		if (f->ctrls.is_hflip_requested ||
		    f->ctrls.is_vflip_requested) {
			cur += cnt;
			left -= cnt;
			ret = snprintf(cur, left,
				       "  | |- rotation=%s\n",
				       f->ctrls.is_hflip_requested ?
				       "hflip" : "vflip");
			cnt = (left > ret ? ret : left);
		}
	}

	error = ctx->sys_errors + ctx->decode_errors + ctx->stream_errors +
		ctx->dropped_frames;

	if (error) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  |\n"
			       "  |-[errors!]\n"
			       "  | |- system    =%d\n"
			       "  | |- decoding  =%d\n"
			       "  | |- stream    =%d\n"
			       "  | |- frame drop=%d\n",
			       ctx->sys_errors,
			       ctx->decode_errors,
			       ctx->stream_errors,
			       ctx->dropped_frames);
		cnt = (left > ret ? ret : left);
	}

	if (ctx->total_mem > 0) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  |\n"
			       "  |-[memory]\n"
			       "  | |- total(KB)=%d\n"
			       "%s", ctx->total_mem / 1000, ctx->mem_str);
		cnt = (left > ret ? ret : left);
	}

	if (ctx->decoded_frames) {
		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       "  |\n"
			       "  |-[performances]\n"
			       "    |- frames decoded=%d\n"
			       "    |- frames output=%d\n"
			       "    |\n"
			       "    |-[ AU frames ]\n"
			       "    | |- avg duration(0.1ms)=%d [min=%d, max=%d]\n"
			       "    | |- au avg period(0.1ms)=%d [min=%d, max=%d]\n"
			       "    | |- au avg fps(0.1Hz)=%d\n"
			       "    | |- max reachable fps=%d\n"
			       "    | |- avg bitrate(Kbit/s)=%d [min=%d, max=%d]\n"
			       "    | |- last bitrate(Kbit/s)=%d\n"
			       "    |\n"
			       "     -[ Decoded frames ]\n"
			       "      |- time to first frame (ms)=%d\n"
			       "      |- out avg period(0.1ms)=%d\n"
			       "      |- out avg fps(0.1Hz)=%d\n",
			       ctx->decoded_frames,
			       ctx->output_frames,
			       ctx->avg_duration,
			       CLIP(ctx->min_duration),
			       ctx->max_duration,
			       ctx->avg_period,
			       CLIP(ctx->min_period),
			       ctx->max_period,
			       ctx->avg_fps,
			       ctx->max_fps,
			       ctx->avg_bitrate,
			       CLIP(ctx->min_bitrate),
			       ctx->max_bitrate, ctx->bitrate,
			       ctx->ttf_decode,
			       ctx->out_avg_period,
			       ctx->out_avg_fps);
		cnt = (left > ret ? ret : left);
	}

	return str;
}

static int device_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int device_dump(struct file *file, char __user *user_buf,
		       size_t size, loff_t *ppos)
{
	struct g1_dev *g1 = file->private_data;
	unsigned char *cur = g1->str;
	unsigned int count = 0;
	size_t left = sizeof(g1->str);
	int cnt = 0;
	int ret = 0;

	memset(g1->str, 0, sizeof(g1->str));

	ret = snprintf(cur, left, "[%s]\n", g1->v4l2_dev.name);
	cnt = (left > ret ? ret : left);

	cur += cnt;
	left -= cnt;
	ret = snprintf(cur, left,
		       "registered as /dev/video%d\n", g1->vdev->num);
	cnt = (left > ret ? ret : left);

	count = simple_read_from_buffer(user_buf, strlen(g1->str), ppos,
					g1->str, strlen(g1->str));

	return count;
}

static const struct file_operations device_fops = {
	.open = device_open,
	.read = device_dump,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

static int decoders_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int decoders_dump(struct file *file, char __user *user_buf,
			 size_t size, loff_t *ppos)
{
	struct g1_dev *g1 = file->private_data;
	unsigned char *cur = g1->str;
	unsigned int count = 0;
	size_t left = sizeof(g1->str);
	u32 stream, pixel;
	int cnt = 0;
	int ret = 0;
	int i = 0;
	int idx;

	memset(g1->str, 0, sizeof(g1->str));

	ret = snprintf(cur, left,
		       "[decoders]\n"
		       "|- %d registered decoders:\n", g1->nb_of_decoders);
	cnt = (left > ret ? ret : left);
	cur += cnt;
	left -= cnt;

	while (g1->decoders[i]) {
		ret = snprintf(cur, left, "|- %s : ", g1->decoders[i]->name);
		cnt = (left > ret ? ret : left);
		cur += cnt;
		left -= cnt;

		for (idx = 0; idx < g1->decoders[i]->nb_streams; idx++) {
			stream = g1->decoders[i]->streamformat[idx];
			ret = snprintf(cur, left, "%4.4s ", (char *)(&stream));
			cnt = (left > ret ? ret : left);
			cur += cnt;
			left -= cnt;
		}

		ret = snprintf(cur, left, "=> ");
		cnt = (left > ret ? ret : left);
		cur += cnt;
		left -= cnt;

		for (idx = 0; idx < g1->decoders[i]->nb_pixels; idx++) {
			pixel = g1->decoders[i]->pixelformat[idx];
			ret = snprintf(cur, left, "%4.4s ", (char *)(&pixel));
			cnt = (left > ret ? ret : left);
			cur += cnt;
			left -= cnt;
		}
		ret = snprintf(cur, left, "\n");
		cnt = (left > ret ? ret : left);
		cur += cnt;
		left -= cnt;
		i++;
	};

	count = simple_read_from_buffer(user_buf, strlen(g1->str), ppos,
					g1->str, strlen(g1->str));

	return count;
}

static const struct file_operations decoders_fops = {
	.open = decoders_open,
	.read = decoders_dump,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

static int last_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int last_dump(struct file *file, char __user *user_buf,
		     size_t size, loff_t *ppos)
{
	struct g1_dev *g1 = file->private_data;
	unsigned char *cur = g1->str;
	unsigned int count = 0;
	size_t left = sizeof(g1->str);
	int cnt = 0;
	int ret = 0;

	memset(g1->str, 0, sizeof(g1->str));

	if (g1->last_ctx.flags & G1_FLAG_STREAMINFO) {
		ret = snprintf(cur, left, "[last decoding]\n");
		cnt = (left > ret ? ret : left);

		perf_compute(&g1->last_ctx);

		cur += cnt;
		left -= cnt;
		ret = snprintf(cur, left,
			       ctx_dump_str(&g1->last_ctx, cur, left,
					    g1));
		cnt = (left > ret ? ret : left);
	}

	count = simple_read_from_buffer(user_buf, strlen(g1->str), ppos,
					g1->str, strlen(g1->str));

	return count;
}

static const struct file_operations last_fops = {
	.open = last_open,
	.read = last_dump,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

void g1_debugfs_create(struct g1_dev *g1)
{
	g1->debugfs_dir = debugfs_create_dir("g1", NULL);

	g1->debugfs_device =
	    debugfs_create_file("device", 0400,
				g1->debugfs_dir, g1, &device_fops);

	g1->debugfs_decoders =
	    debugfs_create_file("decoders", 0400,
				g1->debugfs_dir, g1, &decoders_fops);

	g1->debugfs_last =
	    debugfs_create_file("last", 0400,
				g1->debugfs_dir, g1, &last_fops);
}

void g1_debugfs_remove(struct g1_dev *g1)
{
	debugfs_remove(g1->debugfs_last);
	debugfs_remove(g1->debugfs_device);
	debugfs_remove(g1->debugfs_decoders);
	debugfs_remove(g1->debugfs_dir);
}

static int instance_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static int instance_dump(struct file *file, char __user *user_buf,
			 size_t size, loff_t *ppos)
{
	struct g1_ctx *ctx = file->private_data;
	unsigned char *cur = ctx->str;
	unsigned int count = 0;
	unsigned int left = sizeof(ctx->str);

	memset(ctx->str, 0, sizeof(ctx->str));

	perf_compute(ctx);
	mem_status(ctx);

	snprintf(cur, left,
		 ctx_dump_str(ctx, ctx->str, sizeof(ctx->str), ctx->dev));

	count = simple_read_from_buffer(user_buf, strlen(ctx->str), ppos,
					ctx->str, strlen(ctx->str));

	return count;
}

static const struct file_operations instance_fops = {
	.open = instance_open,
	.read = instance_dump,
	.llseek = default_llseek,
	.owner = THIS_MODULE,
};

void g1_debugfs_open(struct g1_ctx *ctx)
{
	struct g1_dev *g1 = ctx->dev;
	char name[4] = "";

	ctx->min_period = U32_MAX;
	ctx->min_bitrate = U32_MAX;
	ctx->min_duration = U32_MAX;

	snprintf(name, sizeof(name), "%d", g1->instance_id);
	ctx->debugfs =
	    debugfs_create_file(name, 0400,
				g1->debugfs_dir, ctx, &instance_fops);
	INIT_LIST_HEAD(&ctx->mem_infos);
}

void g1_debugfs_close(struct g1_ctx *ctx)
{
	struct g1_dev *g1 = ctx->dev;
	struct mem_info *mem_info = NULL;
	struct mem_info *next = NULL;

	/* format & save memory footprint status from memory linked list
	 * (instead storing the linked list itself).
	 * perf_compute() is not called here to avoid processing
	 * at each end of playback (defered at debugfs invocation).
	 */
	mem_status(ctx);

	/* only save context if something happened */
	if (ctx->flags & G1_FLAG_STREAMINFO)
		/* save current context before removing */
		memcpy(&g1->last_ctx, ctx, sizeof(*ctx));

	list_for_each_entry_safe(mem_info, next, &ctx->mem_infos, list) {
		list_del(&mem_info->list);
		kfree(mem_info);
	}

	debugfs_remove(ctx->debugfs);
}
