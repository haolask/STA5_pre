/*
 * Copyright (C) STMicroelectronics SA 2015
 * Author: Hugues Fruchet <hugues.fruchet@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef G1_H
#define G1_H

#include <linux/slab.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/videobuf2-v4l2.h>

#include "g1-cfg.h"
#include "g1-hw.h"
#include "g1-pp.h"

/*
 * enum g1_state - state of decoding instance
 *
 * @G1_STATE_WF_FORMAT:
 *	wait for compressed format to be set by V4L2 client in order
 *	to know what is the relevant decoder to open.
 *
 * @G1_STATE_WF_STREAMINFO:
 *	wait for stream information to be available (bitstream
 *	header parsing is done).
 *
 * @G1_STATE_READY:
 *	decoding instance is ready to decode compressed access unit.
 *
 * @G1_STATE_CLOSED:
 *	decoding instance is closed.
 *
 * @G1_STATE_STOPPED:
 *	decoding instance is stopped.
 *
 * @G1_STATE_EOS:
 *	EOS (End Of Stream) has been completed (ie signaled to V4L2 client)
 *
 * @G1_STATE_FATAL_ERROR:
 *	A fatal error has been detected in the driver.
 *	Return it to user and cancel every new service except closure
 */
enum g1_state {
	G1_STATE_WF_FORMAT,
	G1_STATE_WF_STREAMINFO,
	G1_STATE_READY,
	G1_STATE_CLOSED,
	G1_STATE_STOPPED,
	G1_STATE_WF_EOS,
	G1_STATE_EOS,
	G1_STATE_FATAL_ERROR
};

/* struct g1_ctrls - g1 control set
 * @hflip:      horizontal flip
 * @vflip:      vertical flip
 * @contrast: control to  adjust contrast of the output picture
 * @brightness: control to  adjust brightness level of the output picture
 * @saturation: control to  adjust color saturation of the output picture
 * @alpha : controls to sets the alpha channel value when the output format
 *          is a predefined 32-bit RGB format
 * @is_configured: ctrl interface enabled or not
 */
struct g1_ctrls {
	struct v4l2_ctrl        *hflip;
	struct v4l2_ctrl        *vflip;
	struct v4l2_ctrl	*contrast;
	struct v4l2_ctrl	*brightness;
	struct v4l2_ctrl	*saturation;
	struct v4l2_ctrl	*alpha;
	struct v4l2_ctrl	*low_latency_mode;
	bool is_configured;
};

/* struct g1_ctrls_value - g1 controls storage
 * @is_hflip_requested:      horizontal flip requested or not
 * @is_vflip_requested:      vertical flip requested or not
 * @contrast: contrast value ; range [-64;64]
 * @brightness: brightness level ; range [-128;127]
 * @saturation: color saturation ; range [-64; 128]
 * @alpha : alpha channel value ; range [0, 255]
 */
struct g1_ctrls_value {
	bool is_hflip_requested;
	bool is_vflip_requested;
	s32 contrast;
	s32 brightness;
	s32 saturation;
	u32 alpha;
};

/*
 * struct g1_streaminfo - information read from stream (header)
 *
 * @flags:		validity of fields (crop, pixelaspect, coding, other)
 * @width:		width of video stream
 * @height:		height ""
 * @aligned_width:	16 pixels aligned value
 * @aligned_height:	8 pixels aligned value
 * @streamformat:	fourcc compressed format of video (VP8, H264, ...)
 * @dpb:		number of frames needed to decode a single frame
 *			(h264 dpb, up to 16)
 * @crop:		cropping window inside output frame (1920x1080@0,0
 *			inside 1920x1088 frame for ex.)
 * @pixelaspect:	pixel aspect ratio of video (4/3, 5/4)
 * @field:		interlaced or not
 * @colorspace:		to store colorspace of the input stream
 * @profile:		profile string
 * @level:		level string
 * @other:		other string information from codec
 */
struct g1_streaminfo {
	__u32 flags;
	__u32 streamformat;
	__u32 width;
	__u32 height;
	__u32 aligned_width;
	__u32 aligned_height;
	__u32 dpb;
	struct v4l2_rect crop;
	struct v4l2_fract pixelaspect;
	enum v4l2_field field;
	enum v4l2_colorspace colorspace;
	__u8 profile[32];
	__u8 level[32];
	__u8 other[32];
};

#define G1_STREAMINFO_FLAG_CROP		0x0001
#define G1_STREAMINFO_FLAG_PIXELASPECT	0x0002
#define G1_STREAMINFO_FLAG_OTHER	0x0004

/*
 * struct g1_frameinfo - frame related information
 *
 * @pixelformat:	pixel format of frame
 * @width:		width of frame
 * @height:		height of frame
 * @aligned_width:	aligned width of frame (encoder or decoder
 *			or post-processor alignment constraint)
 * @aligned_height:	aligned height of frame (encoder or decoder
 *			or post-processor alignment constraint)
 * @crop:		cropping window inside frame (1920x1080@0,0
 *			inside 1920x1088 frame for ex.)
 * @pixelaspect:	pixel aspect ratio of video (4/3, 5/4)
 * @field:		interlaced or not
 * @colorspace:		to store colorspace of the frame
 * @ctrls:		horizontal or vertical flip requests
 * @size:		buffer size of the input buffer
 *			(standalone rawdec only)
 * @vaddr:		virtual address of the input buffer
 *			(standalone rawdec only)
 * @paddr:		physical address of the input buffer
 *			(standalone rawdec only)
 */
struct g1_frameinfo {
	u32 flags;
	u32 pixelformat;
	u32 width;
	u32 height;
	u32 aligned_width;
	u32 aligned_height;
	struct v4l2_rect crop;
	struct v4l2_fract pixelaspect;
	enum v4l2_field field;
	enum v4l2_colorspace colorspace;
	struct g1_ctrls_value ctrls;
	__u32 size;
	void *vaddr;
	dma_addr_t paddr;
	bool deinterlace_activable;
};

#define G1_FRAMEINFO_FLAG_CROP BIT(0)
#define G1_FRAMEINFO_FLAG_PIXELASPECT BIT(1)
#define G1_FRAMEINFO_FLAG_WIDTH_UPDATED BIT(2)
#define G1_FRAMEINFO_FLAG_HEIGHT_UPDATED BIT(3)
#define G1_FRAMEINFO_FLAG_CROP_WIDTH_UPDATED BIT(4)
#define G1_FRAMEINFO_FLAG_CROP_HEIGHT_UPDATED BIT(5)
/*
 * struct g1_au - access unit structure.
 *
 * @vb2:	vb2 struct, to be kept first and not to be wrote by driver.
 *		Allows to get the g1_au fields by just casting a vb2_buffer
 *		with g1_au struct. This is allowed through the use of
 *		vb2 custom buffer mechanism, cf @buf_struct_size of
 *		struct vb2_queue in include/media/videobuf2-core.h
 * @prepared:	boolean, if set vaddr/paddr are resolved
 * @vaddr:	virtual address (kernel can read/write)
 * @paddr:	physical address (for hardware)
 * @flags:	access unit type (V4L2_BUF_FLAG_KEYFRAME/PFRAME/BFRAME)
 * @ts:		timestamp in ns of this frame (DTS or PTS)
 * @state:	access unit state for lifecycle tracking
 *		(G1_AU_FREE/OUT)
 * @picid:	uniq identifier of the AU
 * @remaining_data: indicate number of data not yet been decoded
 */
struct g1_au {
	struct vb2_v4l2_buffer vbuf;	/* keep first */
	int prepared;
	__u32 size;
	void *vaddr;
	dma_addr_t paddr;
	__u32 flags;
	u64 ts;
	__u32 state;
	__u32 picid;
	__u32 remaining_data;
};

#define G1_AU_FREE	0x00
#define G1_AU_OUT	0x01

#define G1_DECODING_TIMESTAMP		0x01
#define G1_PRESENTATION_TIMESTAMP	0x02

/*
 * struct g1_frame - frame structure.
 *
 * @vb2:	vb2 struct, to be kept first and not to be wrote by driver.
 *		Allows to get the g1_frame fields by just casting
 *		vb2_buffer with g1_frame struct. This is allowed through
 *		the use of vb2 custom buffer mechanism, cf @buf_struct_size
 *		of struct vb2_queue in include/media/videobuf2-core.h
 * @prepared:	boolean, if set pix/vaddr/paddr are resolved
 * @index:	frame index, aligned on V4L2 wow
 * @info:	frame information (width, height, format, alignment, ...)
 * @vaddr:	virtual address (kernel can read/write)
 * @paddr:	physical address (for hardware)
 * @flags:	frame type (V4L2_BUF_FLAG_KEYFRAME/PFRAME/BFRAME)
 * @ts:		timestamp of this frame (DTS or PTS)
 * @state:	frame state for frame lifecycle tracking
 *		(G1_FRAME_FREE/DEC/OUT/REC/...)
 * @picid:	uniq identifier of the AU
 */
struct g1_frame {
	struct vb2_v4l2_buffer vbuf;	/* keep first */

	int prepared;
	__u32 index;
	struct v4l2_pix_format pix;
	struct g1_frameinfo *info;
	void *vaddr;
	dma_addr_t paddr;
	__u32 flags;
	u64 ts;
	__u32 state;
	__u32 picid;
	bool to_drop;
	__u32 poc;
	bool isref;
};

#define G1_FRAME_FREE		0x00
#define G1_FRAME_REF		0x01
#define G1_FRAME_BSY		0x02
#define G1_FRAME_DEC		0x04
#define G1_FRAME_OUT		0x08
#define G1_FRAME_RDY		0x10
#define G1_FRAME_QUEUED		0x20

struct g1_timestamp {
	struct list_head list;
	u64 val;
	__u32 index;
};

struct g1_buf {
	__u32 size;
	void *vaddr;
	dma_addr_t paddr;
	const char *name;
	unsigned long attrs;
};

struct g1_ctx;

struct g1_dec {
	struct list_head list;
	const char *name;
	const __u32 *streamformat;
	int nb_streams;
	const __u32 *pixelformat;
	int nb_pixels;
	const __u16 *width_range;
	const __u16 *height_range;
	int max_pixel_nb;
	/*
	 * decoder ops
	 */
	int (*probe)(struct g1_dev *g1);
	int (*open)(struct g1_ctx *ctx);
	int (*close)(struct g1_ctx *ctx);

	/*
	 * setup_frame() - setup frame to be used by decoder
	 * @ctx:	(in) instance
	 * @frame:	(in) frame to use
	 *  @frame.index	(in) identifier of frame
	 *  @frame.vaddr	(in) virtual address (kernel can read/write)
	 *  @frame.paddr	(in) physical address (for hardware)
	 *
	 * Frame is to be allocated by caller, then given
	 * to decoder through this call.
	 * Several frames must be given to decoder (dpb),
	 * each frame is identified using its index.
	 */
	int (*setup_frame)(struct g1_ctx *ctx, struct g1_frame *frame);

	/*
	 * get_streaminfo() - get stream related infos
	 * @ctx:	(in) instance
	 * @streaminfo:	(out) width, height, dpb,...
	 *
	 * Precondition: stream header must have been successfully
	 * parsed to have this call successful & @streaminfo valid.
	 * Header parsing must be done using decode(), giving
	 * explicitly header access unit or first access unit of bitstream.
	 * If no valid header is found, get_streaminfo will return -ENODATA,
	 * in this case the next bistream access unit must be decoded till
	 * get_streaminfo becomes successful.
	 */
	int (*get_streaminfo)(struct g1_ctx *ctx,
			      struct g1_streaminfo *streaminfo);

	int (*set_streaminfo)(struct g1_ctx *ctx,
			      struct g1_streaminfo *streaminfo);

	int (*get_frameinfo)(struct g1_ctx *ctx,
			     struct g1_frameinfo *frameinfo);

	int (*set_frameinfo)(struct g1_ctx *ctx,
			     struct g1_frameinfo *frameinfo);

	/*
	 * decode() - decode a single access unit
	 * @ctx:	(in) instance
	 * @au:		(in/out) access unit
	 *  @au.size	(in) size of au to decode
	 *  @au.vaddr	(in) virtual address (kernel can read/write)
	 *  @au.paddr	(in) physical address (for hardware)
	 *  @au.flags	(out) au type (V4L2_BUF_FLAG_KEYFRAME/
	 *			PFRAME/BFRAME)
	 *
	 * Decode the access unit given. Decode is synchronous;
	 * access unit memory is no more needed after this call.
	 * After this call, none, one or several frames could
	 * have been decoded, which can be retrieved using
	 * get_frame().
	 */
	int (*decode)(struct g1_ctx *ctx, struct g1_au *au);

	/*
	 * get_frame() - get the next decoded frame available
	 * @ctx:	(in) instance
	 * @frame:	(out) frame with decoded data:
	 *  @frame.index	(out) identifier of frame
	 *  @frame.vaddr	(out) virtual address (kernel can read/write)
	 *  @frame.paddr	(out) physical address (for hardware)
	 *  @frame.pix		(out) width/height/format/stride/...
	 *  @frame.flags	(out) frame type (V4L2_BUF_FLAG_KEYFRAME/
	 *			PFRAME/BFRAME)
	 *
	 * Get the next available decoded frame.
	 * If no frame is available, -ENODATA is returned.
	 * If a frame is available, frame structure is filled with
	 * relevant data, frame.index identifying this exact frame.
	 * When this frame is no more needed by upper layers,
	 * recycle() must be called giving this frame identifier.
	 */
	int (*get_frame)(struct g1_ctx *ctx, struct g1_frame **frame);

	/*
	 * recycle() - recycle the given frame
	 * @ctx:	(in) instance
	 * @frame:	(in) frame to recycle:
	 *  @frame.index	(in) identifier of frame
	 *
	 * recycle() is to be called by user when the decoded frame
	 * is no more needed (composition/display done).
	 * This frame will then be reused by decoder to proceed
	 * with next frame decoding.
	 * If not enough frames have been provided through setup_frame(),
	 * or recycle() is not called fast enough, the decoder can run out
	 * of available frames to proceed with decoding (starvation).
	 * This case is guarded by wq_recycle wait queue which ensures that
	 * decoder is called only if at least one frame is available.
	 */
	void (*recycle)(struct g1_ctx *ctx, struct g1_frame *frame);

	/*
	 * flush() - flush decoder
	 * @ctx:	(in) instance
	 *
	 * Reset decoder context and discard all internal buffers.
	 * This allows implementation of seek, which leads to discontinuity
	 * of input bitstream that decoder must know to restart its internal
	 * decoding logic.
	 */
	void (*flush)(struct g1_ctx *ctx);

	/*
	 * drain() - drain decoder
	 * @ctx:	(in) instance
	 *
	 * Mark decoder pending frames (decoded but not yet output) as ready
	 * so that they can be output to client at EOS (End Of Stream).
	 * get_frame() is to be called in a loop right after drain() to
	 * get all those pending frames.
	 */
	void (*drain)(struct g1_ctx *ctx);

	/*
	 * validate_config() - validate post-processing configuration
	 * @ctx:	(in) instance
	 *
	 * check that requested PP configuration is valid.
	 * Only used in case of standalone PP instance usage.
	 * In case of error, instance can't be started.
	 * User has to stopped or reconfigured the instance.
	 */
	int (*validate_config)(struct g1_ctx *ctx);
};

struct g1_dev;

#define STR_LENGTH 1400
#define MEM_STR_LENGTH 400

#define G1_MODE_NORMAL	0
#define G1_MODE_LOW_LATENCY 1
#define G1_MODE_NOT_SET -1

/*  Invalid timestamp value comes from upper layer : gstreamer */
#define G1_INVALID_TIMESTAMP -1000000000

/*
 * struct g1_ctx - instance structure.
 *
 * @flags:		validity of fields (streaminfo)
 * @list:		anchor for list of opened instances maintained
 *			in g1_dev.
 * @fh:			keep track of V4L2 file handle
 * @dev:		keep track of device context
 * @dec:		selected decoder context for this instance
 * @mode:		indicate if stack does the reordering or not
 * @pp:			post-processor for this instance
 * @state:		instance state
 * @q_aus:		V4L2 vb2 queue for access units, allocated by driver
 *			but managed by vb2 framework.
 * @q_frames:		V4L2 vb2 queue for frames, allocated by driver
 *			but managed by vb2 framework.
 * @width:		width given by user through S_FMT(OUTPUT), this is
 *			typically the width claimed by demuxer from container,
 *			which can be different of bitstream width read when
 *			parsing header.
 * @height:		same for height.
 * @streamformat:	format of stream given by user through S_FMT(OUTPUT)
 * @max_au_size:	max size of an access unit
 * @streaminfo:		structure of stream information read by parsing
 *			header
 * @nb_of_aus:		number of access units
 * @aus:		array of access units to keep track of state
 * @wq_recycle:		wait queue to block access unit decoding if
 *			no more frames are available
 * @recycled_frames:	count of recycled (available for decode) frames
 * @nb_of_frames:	number of frames available for decoding
 * @frames:		array of decoding frames to keep track of frame
 *			state and manage frame recycling
 * @decoded_frames:	nb of decoded frames from opening
 * @output_frames:	nb of output frames from opening
 * @dropped_frames:	nb of frames dropped (ie access unit not parsed
 *			or frame decoded but not output)
 * @stream_errors:	nb of stream errors (corrupted, not supported, ...)
 * @decode_errors:	nb of decode errors (firmware error)
 * @sys_errors:		nb of system errors (memory, ipc, ...)
 * @timestamp_type:	to identify type of timestamp : DTS or PTS
 * @pic_counter:	counter to tag each AU with a picture ID.
 * @timestamps:		FIFO of decoding/presentation timestamp.
 *			output frames are timestamped with incoming access
 *			unit timestamps using this fifo.
 * @name:		string naming this instance (debug purpose)
 * @priv:		private decoder context for this instance, allocated
 *			by decoder @open time.
 * @str:		centralized allocated string for debug
 * @debugfs:		debugfs current instance infos
 */
struct g1_ctx {
	__u32 flags;

	struct list_head list;

	struct v4l2_fh fh;

	struct g1_dev *dev;
	const struct g1_dec *dec;

	int mode;

	/* post-processor */
	struct g1_pp pp;

#ifdef DEC_PP_COMMON_SHADOW_REGS
	/* FIXME shadow registers need to be shared between dec & pp
	 * instead having their own shadowed registers.
	 * This point is not understood and need investigations
	 */
	struct g1_hw_regs regs_w_dec;
	struct g1_hw_regs regs_r_dec;
#endif

	enum g1_state state;

	/* vb2 queues */
	struct vb2_queue q_aus;
	struct vb2_queue q_frames;

	size_t max_au_size;

	/* stream */
	struct g1_streaminfo streaminfo;

	/* decoded frame */
	struct g1_frameinfo frameinfo;

	/* input access units */
	__u32 nb_of_aus;
	struct g1_au *aus[G1_MAX_AUS];

	/* output frames */
	wait_queue_head_t wq_recycle;
	u32 recycled_frames;
	__u32 nb_of_frames;
	struct g1_frame *frames[G1_MAX_FRAMES];

	/* used to store that streamon has been (or not)
	 * done for output and capture
	 */
	bool streamon_done[V4L2_BUF_TYPE_VIDEO_OUTPUT + 1];

	/* frames stats */
	u32 decoded_frames;
	u32 output_frames;
	u32 dropped_frames;

	/* errors */
	u32 stream_errors;
	u32 decode_errors;
	u32 sys_errors;

	/* timestamps info */
	int timestamp_type;
	u32 pic_counter;
	struct list_head timestamps;

	u8 instance_id;
	char name[100];

	/* V4L2 ctrl interface to handle flip */
	struct v4l2_ctrl_handler ctrl_handler;
	struct g1_ctrls ctrls;

	/* decoder specific */
	void *priv;

	/* debug: debugfs */
	unsigned char str[STR_LENGTH];
	struct dentry *debugfs;

	/* debug: duration */
	u32 total_duration;
	struct timeval begin;
	u32 min_duration;
	u32 max_duration;
	u32 cnt_duration;
	u32 avg_duration;
	u32 max_fps;
	struct timeval first_au;
	u32 ttf_decode; /* Latency to get the first frame*/

	/* debug: period */
	u32 min_period;
	u32 max_period;
	u32 avg_period;
	u32 total_period;
	u32 cnt_period;
	u32 avg_fps;
	int is_valid_period;

	/* debug output frames period*/
	struct timeval output_tick;
	u32 out_total_period;
	u32 out_cnt_period;
	u32 out_avg_period;
	u32 out_avg_fps;

	/* debug: bitrate */
	u32 window_au_size;
	u32 window_duration;
	u32 window_cnt;
	u32 total_au_size;
	u32 bitrate;
	u32 min_bitrate;
	u32 max_bitrate;
	u32 avg_bitrate;

	/* debug: memory */
	u32 allocated_frames;
	int total_mem;
	unsigned char mem_str[MEM_STR_LENGTH];
	struct list_head mem_infos;
};

#define G1_FLAG_STREAMINFO 0x0001
#define G1_FLAG_FRAMEINFO  0x0002

#define G1_MAX_DECODERS 15

/*
 * struct g1_dev - device struct, 1 per probe (so single one for
 * all platform life)
 *
 * @v4l2_dev:		v4l2 device
 * @vdev:		v4l2 video device
 * @pdev:		platform device
 * @dev:		device
 * @lock:		device lock, for crit section & V4L2 ops serialization.
 * @decoders:		list of registered decoders
 * @nb_of_decoders:	nb of registered decoders
 * @instances:		list of currently opened instances
 * @nb_of_instances:	nb of current opened instances
 * @instance_id:	rolling counter identifying an instance (debug purpose)
 * @debugfs_dir:	debugfs directory
 * @debugfs_device:	debugfs device infos
 * @debugfs_decoders:	debugfs decoders infos
 * @debugfs_last:	debugfs last instance infos
 * @last_ctx:		save of last instance context for debugfs
 * @str:		centralized allocated string for debug
 * @hw:			hardware related structure
 */
struct g1_dev {
	/* device */
	struct v4l2_device v4l2_dev;
	struct video_device *vdev;
	struct platform_device *pdev;
	struct device *dev;
	struct mutex lock; /* checkpatch comment */

	/* handle G1 initialization in a workqueue */
	struct completion rpmsg_connected;
	struct completion resource_unlock;
	struct workqueue_struct *init_wq;
	struct work_struct init_work;
	struct workqueue_struct *resume_wq;
	struct work_struct resume_work;
	enum g1_state state;
	int probe_status;

	/* RPMSG data */
	void *rpmsg_handle;
	u32 waited_rpmsg_event;

	/* decoders */
	const struct g1_dec *decoders[G1_MAX_DECODERS];
	u32 nb_of_decoders;

	/* decoding instances */
	struct list_head instances;
	u32 nb_of_instances;
	u8 instance_id;

	/* debugfs */
	struct dentry *debugfs_dir;
	struct dentry *debugfs_device;
	struct dentry *debugfs_decoders;
	struct dentry *debugfs_last;
	struct g1_ctx last_ctx;
	unsigned char str[STR_LENGTH];

	/* hardware */
	struct g1_hw hw;
};

static inline char *frame_type_str(__u32 flags)
{
	if (flags & V4L2_BUF_FLAG_KEYFRAME)
		return "I";
	if (flags & V4L2_BUF_FLAG_PFRAME)
		return "P";
	if (flags & V4L2_BUF_FLAG_BFRAME)
		return "B";
	return "?";
}

static inline char *frame_state_str(__u32 state, char *str, unsigned int len)
{
	snprintf(str, len, "%s %s %s %s %s",
		 (state & G1_FRAME_REF)  ? "ref" : "   ",
		 (state & G1_FRAME_BSY)  ? "bsy" : "   ",
		 (state & G1_FRAME_DEC)  ? "dec" : "   ",
		 (state & G1_FRAME_OUT)  ? "out" : "   ",
		 (state & G1_FRAME_RDY)  ? "rdy" : "   ");
	return str;
}

int g1_setup_frame(struct g1_ctx *ctx, struct g1_frame *frame);
int g1_get_free_frame(struct g1_ctx *ctx, struct g1_frame **pframe);
void g1_recycle(struct g1_ctx *ctx, struct g1_frame *frame);
bool g1_is_free_frame_available(struct g1_ctx *ctx, struct g1_frame **pframe);
#endif /* G1_H */
