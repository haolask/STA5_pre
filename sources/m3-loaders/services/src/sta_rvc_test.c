/**
 * @file sta_rvc_test.c
 * @brief This file provides Rear View Camera test functions.
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Platform includes */
#include "trace.h"
#include "sta_cam.h"
#include "sta_tvdec.h"
#include "sta_gpio.h"
#include "sta_nvic.h"
#include "sta_common.h"
#include "sta_mtu.h"
#include "sta_mbox.h"
#include "sta_pinmux.h"
#include "sta_rpmsg_mm.h"
#include "sta_lcd.h"
#include "sta_mm.h"

#define TRACE_FUNC_IN()		TRACE_INFO("> %s [%d]\n", __func__, __LINE__)
#define TRACE_FUNC_OUT()	TRACE_INFO("< %s [%d]\n", __func__, __LINE__)
#define TRACE_FUNC_OUT_RET(ret) \
		TRACE_INFO("< %s - ret=%d [%d]\n", __func__, ret, __LINE__)

#define REAR_GEAR_GPIO M3_GPIO(0)

/*#define CAM_TEST_TOGGLING*/

#define OUT_X 0
#define OUT_Y 0

/**
 * To be updated depending on video decoder and how it is connected to
 * CVBS input
 */
#define TVDEC_INPUT ADV7182_INPUT_CVBS_AIN3

/*#define CAM_FULLSCREEN_RESOLUTION*/

#define CUSTOM_OUT_WIDTH	800
#define CUSTOM_OUT_HEIGHT	480

#if !defined(CAM_SCALING)
#define NATIVE_OUT_WIDTH	TVDEC_ACTIVE_COL
#define NATIVE_OUT_HEIGHT	TVDEC_ACTIVE_LINES
#endif

enum rvc_rpsmg_private_msg {
	RVC_BUFFER_FILLED,
	/*!< RVC message transferred by Mx to Linux to send information about
	 * buffer filled with camera data.\n
	 * Additional data are composed of:
	 * - 4 bytes to described pointer on a linux vb2_buf struct
	 */

	RVC_FILL_BUFFER,
	/*!< RVC message transferred by Linux to Mx to indicate buffer
	 * described in data is ready to be filled.\n
	 * Additional data are composed of:
	 * - 4 bytes to described physical address of the buffer in which Mx is
	 * allowed to write camera data.
	 * - 4 bytes to described pointer on a linux vb2_buf struct.
	 */

	RVC_START_STREAMING_REQ,
	/*!< RVC message transferred by Linux to Mx to indicate Linux is ready
	 * to handle buffers coming from Mx.
	 * At this point of time, we initiate an handover from Mx to Linux
	 */

	RVC_STOP_STREAMING_REQ,
	/*!< RVC message transferred by Linux to Mx to indicate Linux is
	 * stopping the camera use-case.
	 */

	RVC_GET_CONFIGURATION_REQ,
	/*!< RVC message transferred by Linux to Mx to request the camera
	 * configuration (output resolution).
	 */

	RVC_GET_CONFIGURATION_ACK,
	/*!< RVC message transferred by Mx to Linux to get the camera
	 * configuration (output resolution).
	 */

	RVC_GET_TAKEOVER_STATUS_ACK,
	/*!< RVC message transferred by Linux to Mx to request the camera
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
	uint32_t buf_phys_addr;
	uint32_t vb2_buf;
};

/*
 * Buffer filled message
 * Transferred from Mx to Ax
 */
struct rvcam_buffer_filled_msg {
	uint32_t vb2_buf;
};

/*
 * RVC configuration message
 * Transferred from Mx to Ax
 */
struct rvcam_configuration_msg {
	uint32_t cam_width;
	uint32_t cam_height;
	uint32_t cam_std;
	uint32_t cam_input;
	uint32_t cam_field;
};

/*
 * RVC private message structure
 */
struct rvcam_msg {
	uint32_t info;
	union {
		struct rvcam_fill_buffer_msg	fill_buffer_msg;
		struct rvcam_buffer_filled_msg	buffer_filled_msg;
		struct rvcam_configuration_msg	config_msg;
	} m;
};

struct rvc_queue_msg {
	uint32_t msg_id;
	void *data;
};

struct cam_rect cam_output;

enum rvc_takeover_state {
	RVC_NOT_MANAGED,
	RVC_MANAGED_BY_Mx,
	RVC_Mx_Ax_TRANSITION,
	RVC_MANAGED_BY_Ax,
};

/**
 * struct rvc_test_context - sta remote processor data
 * @rvc_queue:     RVC Queue Handler
 * @chanid:        Channel ID assigned to RVC Queue
 * @rp_takeover:   Remote Proc takeover state
 *                      remote request.
 * @running:       RVC actually on-going
 */
struct rvc_test_context {
	QueueHandle_t rvc_queue;
	int   chanid;
	bool  running;
	bool  host_loaded;
	void *rpmsg_handle;
	enum rvc_takeover_state rp_takeover;
	enum rpmsg_mm_rvc_debug_mode debug_mode;
} context;

void rvc_test_set_debug_mode(enum rpmsg_mm_rvc_debug_mode mode);
static TaskHandle_t g_rvc_toggling_task;

#ifdef CAM_OVERLAY
#define CURVE_COLOR	0x001F
#define CURVE_WIDTH	10

struct point {
	uint32_t x;
	uint32_t y;
};

struct triangle {
	struct point a;
	struct point b;
	struct point c;
};

static const struct point trace1[4] = {
	{  70, 440 },
	{ 110, 400 },
	{ 224, 284 },
	{ 274, 235 },
};

static const struct point trace2[4] = {
	{ 714, 440 },
	{ 674, 400 },
	{ 560, 284 },
	{ 510, 235 },
};

/* Overlay points are computed relatively to 800 x 480 resolution */
static const uint32_t overlay_width = 800;
static const uint32_t overlay_height = 480;

struct point t1[4];
struct point t2[4];

static uint32_t overlay_buf;
static void draw_line(struct point p0, struct point p1, uint16_t c);
static void draw_triangle(struct triangle t, uint16_t c);
static void bezier(struct point p[4], uint32_t c);

#endif /*CAM_OVERLAY*/

static void rvc_rpmsg_mm_send_config(void);
static int rvc_rear_gear_hdl(void);
static void frame_notif(void *);
static void buffer_filled_notif(void *);
int rvc_services_handler(uint8_t services, void *priv, int len, void *data);

static bool g_rear_gear_id_queued;
static uint32_t rvc_video_standard;

#define REAR_GEAR_ID		0x12340001
#define FRAME_ID		0x12340002
#define INIT_ID			0x12340003
#define RVC_EVENT_UNEXPECTED	0xFFFFFFFF

static int g_rvc_value;

static void rvc_rpmsg_mm_send_takeover_ack(void)
{
	struct rvcam_msg msg;

	TRACE_INFO("rvc_rpmsg_mm_send_takeover_ack\n");

	msg.info = RVC_GET_TAKEOVER_STATUS_ACK;

	rpmsg_send_private_message(context.rpmsg_handle,
				   RPSMG_MM_EPT_CORTEXA_MM,
				   (void *)&msg,
				   sizeof(msg));
}

static void get_rear_gear_status(int *rvc_val)
{
	gpio_read_gpio_pin(REAR_GEAR_GPIO, rvc_val);

	/* GPIO returns 0 if rear gear is engaged
	 * and vice versa, so we invert the result
	 */
	*rvc_val = !(*rvc_val);
}

static int start_camera(void)
{
	if (cam_start_rear_camera()) {
		TRACE_ERR("Can't start Camera.\r\n");
		return -1;
	}

	return 0;
}

static int stop_camera(void)
{
	if (cam_stop_rear_camera()) {
		TRACE_ERR("Can't stop Camera.\r\n");
		return -1;
	}

	return 0;
}

static int manage_trigger(void)
{
	int rvc_value, rvc_value_prev;
	int32_t debounce = 100000; /* 100 ms de-bounce */

	/* De-bounce */
	get_rear_gear_status(&rvc_value);
	rvc_value_prev = rvc_value;
	do {
		mtu_wait_delay(1000);
		debounce -= 1000;
		get_rear_gear_status(&rvc_value);
	} while ((rvc_value == rvc_value_prev) && (debounce > 0));

	if ((debounce > 0) || (g_rvc_value == rvc_value)) {
		TRACE_INFO("Rear Gear bouncing\n");
		return -1;
	}

	g_rvc_value = rvc_value;

	/* Inform remote core about RearGear status */
	rpmsg_mm_send_reargear_status(rvc_value);
	/* Update status on sta_MM side */
	if (g_rvc_value)
		sta_mm_send_event(MM_EVT_RVC_RGE);
	else
		sta_mm_send_event(MM_EVT_RVC_RGD);

	TRACE_INFO("GPIO De-bounce : %d\n", g_rvc_value);
	return 0;
}

static int manage_camera(int status)
{
	int ret = 0;
#ifdef CAM_OVERLAY
	uint32_t i;
#endif

	TRACE_FUNC_IN();

	if ((context.rp_takeover == RVC_NOT_MANAGED) ||
	    (context.rp_takeover == RVC_MANAGED_BY_Ax)) {
		TRACE_INFO("%s: RVC NOT MANAGED BY M3\n", __func__);
		goto end_manage;
	}

	/*  Level Low signal */
	if (status) {
		if (!g_rvc_value)
			TRACE_INFO("%s start req whereas hdw trigger not set\n",
				   __func__);

		ret = start_camera();
		if (ret)
			goto end_manage;

		context.running = true;

#ifdef CAM_OVERLAY
		memcpy(t1, trace2, sizeof(struct point) * 4);
		memcpy(t2, trace2, sizeof(struct point) * 4);

		cam_get_overlay_buffer(&overlay_buf);
		for (i = 0; i < 4; i++) {
			t1[i].x = (trace1[i].x * cam_output.w) / overlay_width;
			t1[i].y = (trace1[i].y * cam_output.h) / overlay_height;
			t2[i].x = (trace2[i].x * cam_output.w) / overlay_width;
			t2[i].y = (trace2[i].y * cam_output.h) / overlay_height;
		}

		for (i = 0; i < cam_output.w * cam_output.h; i++)
			*(uint16_t *)(overlay_buf + i * 2) = TRANSPARENT_COLOR;
#endif
	} else {
		if (g_rvc_value)
			TRACE_INFO("%s / stop req whereas hdw trigger is set\n",
				   __func__);
		ret = stop_camera();
		context.running = false;

		if (context.rp_takeover == RVC_Mx_Ax_TRANSITION) {
			context.rp_takeover = RVC_MANAGED_BY_Ax;
			ret = cam_deinit_rear_camera();

			if (ret)
				context.rp_takeover = RVC_Mx_Ax_TRANSITION;
			else
				rvc_rpmsg_mm_send_takeover_ack();
		}
	}
end_manage:
	return ret;
}

int rvc_start(void)
{
	int ret = 0;

	TRACE_FUNC_IN();

	ret = manage_camera(1);
	if (ret) {
		sta_mm_send_event(MM_EVT_RVC_STOPPED);
		TRACE_FUNC_OUT_RET(ret);
		return ret;
	}
	sta_mm_send_event(MM_EVT_RVC_STARTED);

	TRACE_FUNC_OUT_RET(0);
	return 0;
}

int rvc_stop(void)
{
	int ret = 0;

	TRACE_FUNC_IN();

	ret = manage_camera(0);
	sta_mm_send_event(MM_EVT_RVC_STOPPED);

	TRACE_FUNC_OUT_RET(ret);
	return ret;
}

static int frame_management(void)
{
#ifdef CAM_OVERLAY
	static int dx = -5;

	/*  Clean up old trace */
	bezier(t1, TRANSPARENT_COLOR);
	bezier(t2, TRANSPARENT_COLOR);
	t1[3].x	+= dx;
	t2[3].x	+= dx;
	if (t1[3].x < (20 * cam_output.w) / overlay_width ||
	    t2[3].x > (780 * cam_output.w) / overlay_width) {
		dx = -dx;
	}
	bezier(t1, CURVE_COLOR);
	bezier(t2, CURVE_COLOR);
#endif
	return 0;
}

int rvc_rpmsg_mm_callback(struct s_rpmsg_mm_data *data, void *priv)
{
	TRACE_INFO("rvc_rpmsg_mm_callback\n");

	if (!data)
		return -1;

	switch (RPMSG_MM_GET_INFO(data->info)) {
	case RPMSG_MM_SHARED_RES_STATUS_REQ:
		TRACE_INFO("RPMSG_MM_SHARED_RES_STATUS_REQ\n");
		data->len = 1;
		data->info = RPMSG_MM_SHARED_RES_STATUS_ACK;
		data->data[0] = 0;
		break;
	case RPMSG_MM_REARGEAR_STATUS_REQ:
		TRACE_INFO("return RearGearStatus : %d\n", g_rvc_value);
		return g_rvc_value;
	case RPMSG_MM_PRIVATE_MESSAGE:
	{
		struct rvcam_msg *msg = (struct rvcam_msg *)&data->data[0];

		TRACE_INFO("RPMSG_MM_PRIVATE_MESSAGE (len:%d)\n", data->len);
		if (data->len < (sizeof(uint32_t))) {
			TRACE_ERR("RVC PRIVATE_MSG : bad message length\n");
			break;
		}

		/* First 4 bytes indicate the private command*/
		switch (msg->info) {
		case RVC_FILL_BUFFER:
			TRACE_INFO("RVC_FILL_BUFFER\n");
			if (g_rvc_value)
				cam_new_buffer(
					msg->m.fill_buffer_msg.buf_phys_addr,
					msg->m.fill_buffer_msg.vb2_buf,
					CAM_BUFFER_Ax);
			break;
		case RVC_START_STREAMING_REQ:
			TRACE_INFO("RVC_START_STREAMING_REQ\n");
			cam_start_streaming_ax();
			break;
		case RVC_STOP_STREAMING_REQ:
			TRACE_INFO("RVC_STOP_STREAMING_REQ\n");
			cam_stop_streaming_ax();
			break;
		case RVC_GET_CONFIGURATION_REQ:
			TRACE_INFO("RVC_GET_CONFIGURATION_REQ\n");
			rvc_rpmsg_mm_send_config();
			break;
		default:
			TRACE_INFO("Unknown private message\n");
			break;
		}
		break;
	}
	default:
		break;
	}

	return 0;
}

static void rpmsg_mm_send_buffer_filled(uint32_t vb2_buf)
{
	struct rvcam_msg msg;

	TRACE_INFO("rpmsg_mm_send_buffer_filled\n");

	msg.info = RVC_BUFFER_FILLED;
	msg.m.buffer_filled_msg.vb2_buf = vb2_buf;
	rpmsg_send_private_message(context.rpmsg_handle,
				   RPSMG_MM_EPT_CORTEXA_RVC,
				   (void *)&msg,
				   sizeof(msg));
}

static void rvc_rpmsg_mm_send_config(void)
{
	struct rvcam_msg msg;

	TRACE_INFO("rvc_rpmsg_mm_send_config\n");

	msg.info = RVC_GET_CONFIGURATION_ACK;
	msg.m.config_msg.cam_width = cam_output.w;
	msg.m.config_msg.cam_height = cam_output.h;

	switch (rvc_video_standard) {
	case CAM_NTSC:
		msg.m.config_msg.cam_std = RVC_STD_NTSC;
		break;
	case CAM_PAL:
		msg.m.config_msg.cam_std = RVC_STD_PAL;
		break;
	default:
		msg.m.config_msg.cam_std = RVC_STD_UNKNOWN;
	}

	msg.m.config_msg.cam_input = TVDEC_INPUT;

#ifdef CAM_NO_DEINTERLEAVING
	msg.m.config_msg.cam_field = RVC_FIELD_NONE;
#else
	msg.m.config_msg.cam_field = RVC_FIELD_INTERLACED;
#endif
	rpmsg_send_private_message(context.rpmsg_handle,
				   RPSMG_MM_EPT_CORTEXA_RVC,
				   (void *)&msg,
				   sizeof(msg));
}

/***************************************************************************
 * Function Name  : RVC_task
 * Description    : Rear view Camera Test task routine
 * Input          : None
 * Return         : None
 ***************************************************************************/
void rvc_task(void *p)
{
	struct gpio_config	rvc_pin;
	struct nvic_chnl	irq_chnl;
	int			rvc_value;
	struct rvc_queue_msg	msg;
	int registered_services = (RPMSG_MM_SERV_START_RVC |
				   RPMSG_MM_SERV_STOP_RVC |
				   RPMSG_MM_SERV_RVC_DEBUG_MODE |
				   RPMSG_MM_SERV_RVC_AX_TAKEOVER);
	struct sta *ctxt = (struct sta *)p;
	int ret;

	TRACE_FUNC_IN();

	/* Register to start RVC, stop RVC and RVC debug mode
	 */
	rpmsg_mm_register_service(registered_services,
				  rvc_services_handler,
				  NULL);

#ifdef CAM_OVERLAY
	overlay_buf = 0;
#endif

	cam_output.x = OUT_X;
	cam_output.y = OUT_Y;

#ifdef CAM_SCALING
	#ifdef CAM_FULLSCREEN_RESOLUTION
		lcd_get_display_resolution(&cam_output.w, &cam_output.h);
		if ((!cam_output.w) || (!cam_output.h))
			goto failure;
	#else
		cam_output.w = CUSTOM_OUT_WIDTH;
		cam_output.h = CUSTOM_OUT_HEIGHT;
	#endif
#else
#ifdef PP_USAGE
	#ifdef CAM_FULLSCREEN_RESOLUTION
		lcd_get_display_resolution(&cam_output.w, &cam_output.h);
		if ((!cam_output.w) || (!cam_output.h))
			goto failure;
	#else
		cam_output.w = CUSTOM_OUT_WIDTH;
		cam_output.h = CUSTOM_OUT_HEIGHT;
	#endif
#else
	cam_output.w = NATIVE_OUT_WIDTH;
	cam_output.h = NATIVE_OUT_HEIGHT;
#endif
#endif

	context.running = false;
	context.host_loaded = false;
	context.rp_takeover = RVC_MANAGED_BY_Mx;
	context.rvc_queue = xQueueCreate(20, sizeof(struct rvc_queue_msg));
	context.debug_mode = RVC_DEBUG_MODE_NORMAL;

	rvc_pin.direction	= GPIO_DIR_INPUT;
	rvc_pin.mode		= GPIO_MODE_SOFTWARE;
	rvc_pin.level		= GPIO_LEVEL_LEAVE_UNCHANGED;
	rvc_pin.trig		= GPIO_TRIG_BOTH_EDGES;

	gpio_set_pin_config(REAR_GEAR_GPIO, &rvc_pin);

	/* Register this task as an RPMSG MM endpoint :
	 * - it provides rear gear status
	 * - it shares resources with Linux
	 * - it needs to exchange messages between Mx and Linux
	 */
	context.rpmsg_handle = rpmsg_mm_register_local_endpoint(
					RPSMG_MM_EPT_CORTEXM_RVC,
					rvc_rpmsg_mm_callback, NULL);

	/*  Early detection  */
	/*  Verify if Rear Gear has already been engaged */
	get_rear_gear_status(&rvc_value);

	cam_hw_init(TVDEC_INPUT, &rvc_video_standard);

	/* Access to ADV7182 is all done inside "cam_hw_init".
	 * There is no access to it afterwards.
	 * Release the bootflow task safely.
	 */
	if (ctxt->event_group)
		xEventGroupSetBits(ctxt->event_group, RVC_TASK_EVENT_BIT);

	if (context.rp_takeover == RVC_MANAGED_BY_Mx) {
		/* Initialize the Rear View Camera Service */
		ret = cam_init_rear_camera(
				&cam_output,
				(context.rp_takeover == RVC_MANAGED_BY_Ax));
		if (ret) {
			TRACE_ERR("Can't initialize Camera.\r\n");
			goto failure;
		}
		cam_register_notif(FRAME_EVENT, frame_notif);
		cam_register_notif(BUFFER_FILLED_EVENT, buffer_filled_notif);

		if (rvc_value == 1)
			manage_trigger();
	}

	/* Configure the Rear Gear GPIO in interrupt mode */
	ret = gpio_request_irq(REAR_GEAR_GPIO, rvc_rear_gear_hdl);
	if (ret) {
		TRACE_ERR("%s: failed to request irq\n", __func__);
		goto failure;
	}

	/**
	 * Enable the Mx GPIO interrupt.
	 */
	irq_chnl.id		= GPIO_IRQChannel;
	irq_chnl.preempt_prio	= IRQ_LOW_PRIO;
	irq_chnl.enabled	= true;
	nvic_chnl_init(&irq_chnl);

#ifdef CAM_TEST_TOGGLING
	rvc_test_set_debug_mode(RVC_DEBUG_MODE_AUTO_TOGGLING);
#endif

	TRACE_INFO("TEST: starting RVC service\n");

	while (xQueueReceive(context.rvc_queue, &(msg), portMAX_DELAY)) {
		switch (msg.msg_id) {
		case REAR_GEAR_ID:
			g_rear_gear_id_queued = false;
			manage_trigger();
			break;
		case FRAME_ID:
			frame_management();
			break;
		default:
			TRACE_ERR("%s: Unknown Msg ID: %d\n", __func__,
				  msg.msg_id);
			break;
		}
	}

failure:
	stop_camera();
	sta_mm_send_event(MM_EVT_RVC_STOPPED);
	vQueueDelete(context.rvc_queue);
	while (1)
		vTaskDelay(pdMS_TO_TICKS(10));
}

static int rvc_rear_gear_hdl(void)
{
	struct rvc_queue_msg msg;

	msg.msg_id = REAR_GEAR_ID;

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (g_rear_gear_id_queued)
		return 0;

	g_rear_gear_id_queued = true;

	if (xQueueSendToBackFromISR(context.rvc_queue, &msg,
				    &xHigherPriorityTaskWoken))
		return -1;

	if (xHigherPriorityTaskWoken)
		taskYIELD();

	return 0;
}

static void frame_notif(void *data)
{
	struct rvc_queue_msg msg = {
		.msg_id = FRAME_ID,
	};

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (xQueueSendToBackFromISR(context.rvc_queue, &msg,
				    &xHigherPriorityTaskWoken))
		return;

	if (xHigherPriorityTaskWoken)
		taskYIELD();
}

static void buffer_filled_notif(void *data)
{
	uint32_t *priv = (uint32_t *)data;

	TRACE_INFO("buffer_filled_notif: 0x%08X\n", *priv);
	rpmsg_mm_send_buffer_filled(*priv);
}

#ifdef CAM_OVERLAY
void draw_line(struct point p0, struct point p1, uint16_t c)
{
	int32_t dx	= abs(p1.x - p0.x);
	int32_t sx	= p0.x < p1.x ? 1 : -1;
	int32_t dy	= abs(p1.y - p0.y);
	int32_t sy	= p0.y < p1.y ? 1 : -1;
	int32_t err	= (dx > dy ? dx : -dy) / 2;
	int32_t	e2;

	for (;;) {
		uint32_t addr = (p0.y * cam_output.w + p0.x) * SCREEN_BPP;

		*(uint16_t *)(overlay_buf + addr) = c;

		if (p0.x == p1.x && p0.y == p1.y)
			break;
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			p0.x += sx;
		}
		if (e2 <  dy) {
			err += dx;
			p0.y += sy;
		}
	}
}

void draw_triangle(struct triangle t, uint16_t c)
{
	draw_line(t.a, t.b, c);
	draw_line(t.b, t.c, c);
	draw_line(t.c, t.a, c);
}

void bezier(struct point p[4], uint32_t c)
{
	double		t;
	bool		first = 1;
	double		xt1, yt1;
	struct triangle trg;

	for (t = 0.0; t < 1.0; t += 0.01) {
		double xt = (1 - t) * (1 - t) * (1 - t) * p[0].x +
			3 * t * (1 - t) * (1 - t) * p[1].x +
			3 * t * t * (1 - t) * p[2].x +
			t * t * t * p[3].x;

		double yt = (1 - t) * (1 - t) * (1 - t) * p[0].y +
			3 * t * (1 - t) * (1 - t) * p[1].y +
			3 * t * t * (1 - t) * p[2].y +
			t * t * t * p[3].y;

		if (first) {
			first = 0; /* Skip this iteration */
			xt1 = xt;
			yt1 = yt;
			continue;
		}

		trg.a.x	= (uint32_t)xt1;
		trg.b.x	= (uint32_t)xt1 + CURVE_WIDTH;
		trg.c.x	= (uint32_t)xt;
		trg.a.y	= (uint32_t)yt1;
		trg.b.y	= (uint32_t)yt1;
		trg.c.y	= (uint32_t)yt;
		draw_triangle(trg, c);

		trg.a.x	= (uint32_t)xt;
		trg.b.x	= (uint32_t)xt  + CURVE_WIDTH;
		trg.c.x	= (uint32_t)xt1 + CURVE_WIDTH;
		trg.a.y	= (uint32_t)yt;
		trg.b.y	= (uint32_t)yt;
		trg.c.y	= (uint32_t)yt1;

		xt1	= xt;
		yt1	= yt;
		draw_triangle(trg, c);
	}
}
#endif

/* Handles the takeover of RVC to Ax processor.
 * Must be called in the context of "MM" task to ensure that RVC is not in a
 * transient state.
 */
void rvc_ax_takeover(void)
{
	int ret = 0;

	TRACE_FUNC_IN();

	TRACE_INFO("rp_takeover: %d\n", context.rp_takeover);

	switch (context.rp_takeover) {
	case RVC_MANAGED_BY_Mx:
		if (context.running) {
			context.rp_takeover = RVC_Mx_Ax_TRANSITION;
			TRACE_INFO("rp_takeover = RVC_Mx_Ax_TRANSITION");
		} else {
			context.rp_takeover = RVC_MANAGED_BY_Ax;
			ret = cam_deinit_rear_camera();

			if (ret)
				context.rp_takeover = RVC_Mx_Ax_TRANSITION;
			else
				rvc_rpmsg_mm_send_takeover_ack();
		}

		break;
	case RVC_MANAGED_BY_Ax:
		rvc_rpmsg_mm_send_takeover_ack();
		break;
	case RVC_Mx_Ax_TRANSITION:
	default:
		break;
	}

	TRACE_FUNC_OUT();
}

/*****************
 * DEBUG FUNCTIONS
 *****************/

void rvc_test_update_rear_gear(bool val)
{
	/* Change of rear gear GPIO state is allowed only in manual toggling
	 * mode.
	 */
	if (context.debug_mode != RVC_DEBUG_MODE_MANUAL_TOGGLING)
		return;

	if (val)
		gpio_clear_gpio_pin(REAR_GEAR_GPIO);
	else
		gpio_set_gpio_pin(REAR_GEAR_GPIO);
}

static void rvc_gpio_toggling_task(void)
{
	bool toggle = false;
	int delay_ms = 1500;

	while (context.debug_mode == RVC_DEBUG_MODE_AUTO_TOGGLING) {
		trace_print('I', "GPIO = %d - toggle delay = %d (ms)\n",
			    toggle,
			    delay_ms);

		if (toggle)
			gpio_clear_gpio_pin(REAR_GEAR_GPIO);
		else
			gpio_set_gpio_pin(REAR_GEAR_GPIO);

		vTaskDelay(pdMS_TO_TICKS(delay_ms));

		toggle = !toggle;
	}

	g_rvc_toggling_task = NULL;
	vTaskDelete(NULL);
}

void rvc_test_set_debug_mode(enum rpmsg_mm_rvc_debug_mode mode)
{
	int ret = 0;
	struct gpio_config rvc_pin = {
		.mode	= GPIO_MODE_SOFTWARE,
		.level	= GPIO_LEVEL_LEAVE_UNCHANGED,
		.trig	= GPIO_TRIG_BOTH_EDGES,
	};

	if (context.debug_mode == mode)
		return;

	/* Delete toggling task in case it's not auto mode */
	if ((mode != RVC_DEBUG_MODE_AUTO_TOGGLING) && g_rvc_toggling_task) {
		vTaskDelete(g_rvc_toggling_task);
		g_rvc_toggling_task = NULL;
	}

	/* Configure RVC pin direction */
	switch (mode) {
	case RVC_DEBUG_MODE_NORMAL:
		rvc_pin.direction	= GPIO_DIR_INPUT;
		break;
	case RVC_DEBUG_MODE_MANUAL_TOGGLING:
	case RVC_DEBUG_MODE_AUTO_TOGGLING:
		rvc_pin.direction	= GPIO_DIR_OUTPUT;
		break;
	default:
		TRACE_ERR("Unknown RVC debug mode: %d\n", mode);
		return;
	}
	gpio_set_pin_config(REAR_GEAR_GPIO, &rvc_pin);
	context.debug_mode = mode;

	/* Start toggling task in case of auto mode, stop it otherwise */
	if ((mode == RVC_DEBUG_MODE_AUTO_TOGGLING) &&
	    (!g_rvc_toggling_task)) {
		ret = xTaskCreate((pdTASK_CODE) rvc_gpio_toggling_task,
				  (char *)"RVC test toggling", 98,
				  (void *)NULL, TASK_PRIO_TEST_RVC_CONTROL,
				  &g_rvc_toggling_task);

		if (ret != pdPASS) {
			g_rvc_toggling_task = NULL;
			TRACE_ERR("Cannot create task for test toggling\n");
		}
	}
}

/* Callback called on registered Ax events.
 * This function is excuted in the context of RPMSG_MM task.
 */
int rvc_services_handler(uint8_t services, void *priv, int len, void *data)
{
	if ((services | RPMSG_MM_SERV_START_RVC) == RPMSG_MM_SERV_START_RVC) {
		TRACE_INFO("[RVC] Service START_RVC\n");
		rvc_test_update_rear_gear(1);
	} else if ((services | RPMSG_MM_SERV_STOP_RVC) ==
		   RPMSG_MM_SERV_STOP_RVC) {
		TRACE_INFO("[RVC] Service STOP_RVC\n");
		rvc_test_update_rear_gear(0);
	} else if ((services | RPMSG_MM_SERV_RVC_DEBUG_MODE) ==
		   RPMSG_MM_SERV_RVC_DEBUG_MODE) {
		TRACE_INFO("[RVC] Service RVC_DEBUG_MODE\n");
		if ((len != sizeof(uint32_t)) || (!data)) {
			TRACE_ERR("[RVC] Service RVC_DEBUG_MODE: bad input\n");
			return -1;
		}

		rvc_test_set_debug_mode(*(uint32_t *)data);
	} else if ((services | RPMSG_MM_SERV_RVC_AX_TAKEOVER) ==
		   RPMSG_MM_SERV_RVC_AX_TAKEOVER) {
		TRACE_INFO("[RVC] Service RVC_A7_TAKEOVER\n");
		/* Send "RVC_TAKEOVER" event to MM in order to change the
		 * context of execution.
		 * This will ensure that "rvc_ax_takeover" is not called in the
		 * middle of "rvc_start" or "rvc_stop".
		 */
		sta_mm_send_event(MM_EVT_RVC_TAKEOVER);
	}
	return 0;
}
