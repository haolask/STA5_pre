#define DRV_VERSION	"1.0"
#define DRV_NAME	"sta-rvcam"

/*
 * RVCAM Definitions
 */

#define MAX_FRAMES		8
#define NUM_MIN_BUFFERS		3

enum rvc_rpsmg_private_msg {
	RVC_BUFFER_FILLED,
	/*!< RVC message transferred by M3 to Linux to send information about
	 * buffer filled with camera data.\n
	 * Additional data are composed of:
	 * - 4 bytes to described pointer on a linux vb2_buf struct
	 */

	RVC_FILL_BUFFER,
	/*!< RVC message transferred by Linux to M3 to indicate buffer
	 * described in data is ready to be filled.\n
	 * Additional data are composed of:
	 * - 4 bytes to described physical address of the buffer in which M3 is
	 * allowed to write camera data.
	 * - 4 bytes to described pointer on a linux vb2_buf struct.
	 */

	RVC_START_STREAMING_REQ,
	/*!< RVC message transferred by Linux to M3 to indicate Linux is ready
	 * to handle buffers coming from M3.
	 * At this point of time, we initiate an handover from M3 to Linux
	 */

	RVC_STOP_STREAMING_REQ,
	/*!< RVC message transferred by Linux to M3 to indicate Linux is
	 * stopping the camera use-case.
	 */

	RVC_GET_CONFIGURATION_REQ,
	/*!< RVC message transferred by Linux to M3 to request the camera
	 * configuration (output resolution).
	 */

	RVC_GET_CONFIGURATION_ACK,
	/*!< RVC message transferred by M3 to Linux to get the camera
	 * configuration (output resolution).
	 */
};

enum rvc_standard {
	/* No standard detected */
	RVC_STD_UNKNOWN,
	/* PAL standard */
	RVC_STD_PAL,
	/* NTSC standard */
	RVC_STD_NTSC,
};

enum rvc_field {
	RVC_FIELD_NONE,
	RVC_FIELD_TOP,
	RVC_FIELD_BOTTOM,
	RVC_FIELD_INTERLACED,
	RVC_FIELD_SEQ_TB,
	RVC_FIELD_SEQ_BT,
	RVC_FIELD_ALTERNATE,
	RVC_FIELD_INTERLACED_TB,
	RVC_FIELD_INTERLACED_BT,
};

/*
 * Fill buffer message
 * Transferred from Ax to Mx
 */
struct rvcam_fill_buffer_msg {
	u32 buf_phys_addr;
	u32 vb2_buf;
};

/*
 * Buffer filled message
 * Transferred from Mx to Ax
 */
struct rvcam_buffer_filled_msg {
	u32 vb2_buf;
};

/*
 * RVC configuration message
 * Transferred from Mx to Ax
 */
struct rvcam_configuration_msg {
	u32 cam_width;
	u32 cam_height;
	u32 cam_std;
	u32 cam_input;
	u32 cam_field;
};

/*
 * RVC private message structure
 */
struct rvcam_msg {
	u32 info;
	union {
		struct rvcam_fill_buffer_msg	fill_buffer_msg;
		struct rvcam_buffer_filled_msg	buffer_filled_msg;
		struct rvcam_configuration_msg	config_msg;
	} m;
};

#define RVCAM_PIXFMT		V4L2_PIX_FMT_UYVY
#define RVCAM_BPP		2
#define RVCAM_COLORSPACE	V4L2_COLORSPACE_SMPTE170M

struct sta_rvcam_fmt {
	char *name;
	u32 fourcc;
	u8 bpp;
};

static struct sta_rvcam_fmt sta_cap_formats[] = {
	{
		.name		= "UYVY",
		.fourcc		= RVCAM_PIXFMT,
		.bpp		= RVCAM_BPP,
	},
};

#define RVCAM_NUM_FORMATS ARRAY_SIZE(sta_cap_formats)

/**
 * struct rvcam_control - used to store information about RVCAM controls
 *			it is used to initialize the control framework.
 */
struct rvcam_control {
	__u32			id;
	enum v4l2_ctrl_type	type;
	__u8			name[32];  /* Whatever */
	__s32			minimum;   /* Note signedness */
	__s32			maximum;
	__s32			step;
	__u32			menu_skip_mask;
	__s32			default_value;
	__u32			flags;
	__u32			reserved[2];
	__u8			is_volatile;
};

static struct rvcam_control controls[] = {
	{
		.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE,
		.type = V4L2_CTRL_TYPE_INTEGER,
		.name = "Minimum number of cap bufs",
		.minimum = 1,
		.maximum = 32,
		.step = 1,
		.default_value = 1,
		.is_volatile = 1,
	},
};

#define NUM_CTRLS ARRAY_SIZE(controls)
#define RVCAM_MAX_CTRLS		5

#define STR_LENGTH 1400

/*
 * Buffer for one video frame
 */
struct rvcam_buffer {
	/* common v4l buffer stuff -- must be first */
	struct vb2_v4l2_buffer	vbuf;
	struct list_head	queue;

	unsigned int index;
	unsigned int size;

};

static inline struct rvcam_buffer *to_rvcam_buffer(struct vb2_buffer *vb2)
{
	return container_of(to_vb2_v4l2_buffer(vb2), struct rvcam_buffer, vbuf);
}

/*
 * Performance measurements
 */
struct rvcam_perf {
	u64 start_streaming_ns;
	u64 end_streaming_ns;
	u64 max_duration;
	u64 min_duration;
};

/*
 * enum rvcam_state - state of camera instance
 *
 * @RVCAM_STATE_READY:
 *	camera instance is ready to stream.
 *
 * @RVCAM_STATE_CLOSED:
 *	camera instance is closed.
 *
 * @RVCAM_STATE_STOPPED:
 *	camera instance is stopped.
 *
 * @RVCAM_STATE_EOS:
 *	EOS (End Of Stream) has been completed (ie signaled to V4L2 client)
 *
 * @RVCAM_STATE_FATAL_ERROR:
 *	A fatal error has been detected in the driver.
 *	Return it to user and cancel every new service except closure
 */
enum rvcam_state {
	RVCAM_STATE_READY,
	RVCAM_STATE_CLOSED,
	RVCAM_STATE_STOPPED,
	RVCAM_STATE_EOS,
	RVCAM_STATE_FATAL_ERROR
};

/*
 * enum rvcam_probe_status - status of rvcam probe
 *
 * @RVCAM_PROBE_IDLE:
 *	probe initiale state.
 *
 * @RVCAM_PROBE_POSTPONED:
 *	probe has been postponed.
 *
 * @RVCAM_PROBE_RESUME_POSTPONED:
 *	probe has been postponed after a resume.
 *
 * @RVCAM_PROBE_DONE:
 *	rvcam has been probed successfully
 */
enum rvcam_probe_status {
	RVCAM_PROBE_IDLE,
	RVCAM_PROBE_POSTPONED,
	RVCAM_PROBE_RESUME_POSTPONED,
	RVCAM_PROBE_DONE
};

/**
 * struct sta_rvcam - All internal data for one instance of device
 *
 * @v4l2_dev:			device registered in v4l layer
 * @video_dev:			properties of our device
 * @pdev:			platform device
 * @ctrl_hdl:			handler for control framework
 * @ctrls:			list of v4l2 controls
 * @v4l2_ctrls_is_configured:	true if v4l2 controls configured
 * @ctrl:			Current value of RVCAM CTRL register
 * @state:			camera state
 * @probe_status:		camera probe status
 * @rpmsg_handle:		RPMsg handle
 * @wait_for_rpmsg_available:	true if waiting for RPMsg to be available
 * @rpmsg_wq:			Work queue to wait for RPMsg reply
 * @completion:			used to complete work queue
 * @resume_work:		Work queued after a suspend/resume
 * @init_work:			Work queued to complete probe
 * @format_cap:			V4l2 Capure pixel format
 * @std:			video standard (e.g. PAL/NTSC)
 * @input:			input line for video signal:
 *				[0 => Ain1, 1 => Ain2, 2 => Ain3 ...]
 *				for sta1295 EVB board, this is 2
 *				see ADV7182 datasheet Input Control register
 * @width:			camera width
 * @height:			camera height
 * @field:			field format
 * @vq:				queue maintained by videobuf2 layer
 * @capture:			list of buffer in use
 * @sequence:			sequence number of acquired buffer
 * @frame_err:			number of frame error
 * @frame_drop:			number of frame drop
 * @incomplete_frame_nb:	number of incomplete frames
 * @mutex:			used to access capture device
 * @buf_list_lock:		used to protect vb2 buffer list access
 * @alloc_ctx:			allocator-specific contexts for each plane
 * @debugfs_dir:		debugfs directory
 * @debugfs_device:		debugfs device infos
 * @debugfs_last:		debugfs last instance infos
 * @str:			centralized allocated string for debug
 * @perf:			used to track performance
 *
 * All non-local data is accessed via this structure.
 */
struct sta_rvcam {
	struct v4l2_device v4l2_dev;
	struct video_device *video_dev;
	struct platform_device *pdev;
	struct v4l2_ctrl_handler ctrl_hdl;
	struct v4l2_ctrl *ctrls[RVCAM_MAX_CTRLS];
	bool   v4l2_ctrls_is_configured;
	u32 ctrl;

	enum rvcam_state state;
	enum rvcam_probe_status probe_status;

	/* RPMsg */
	void *rpmsg_handle;
	bool wait_for_rpmsg_available;
	struct workqueue_struct *rpmsg_wq;
	struct completion rpmsg_connected;
	struct completion rvc_config_received;
	struct work_struct resume_work;
	struct work_struct init_work;

	struct v4l2_pix_format format_cap;

	v4l2_std_id std;
	unsigned int input;
	u32 width;
	u32 height;
	enum v4l2_field field;

	struct vb2_queue vq;
	struct list_head capture;
	unsigned int sequence;
	unsigned int frame_err;
	unsigned int frame_drop;
	unsigned int incomplete_frame_nb;

	/* used to access capture device */
	struct mutex mutex;

	/* used to protect vb2 buffer list access */
	struct mutex buf_list_lock;

	/* allocator-specific contexts for each plane */
	struct vb2_alloc_ctx *alloc_ctx;

	/* debugfs */
	struct dentry *debugfs_dir;
	struct dentry *debugfs_device;
	struct dentry *debugfs_last;
	unsigned char str[STR_LENGTH];

	struct rvcam_perf perf;
};
