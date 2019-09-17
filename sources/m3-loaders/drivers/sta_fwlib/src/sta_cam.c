/**
 * @file sta_cam.c
 * @brief  This file provides Camera Framework
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "trace.h"
#include "queue.h"

#include "sta_cam.h"
#include "sta_tvdec.h"
#include "sta_common.h"
#include "sta_lcd.h"
#include "sta_ltdc.h"
#include "sta_mtu.h"
#if defined PP_USAGE
#include "pp-ctrl.h"
#include "g1-res.h"
#endif

/* Definition statements for Rear Camera batch */
#define LINES_TO_SKIP	(TVDEC_NBLINES - TVDEC_ACTIVE_LINES)
#define FIELDS_TO_SKIP	8  /*  50 */
#define NB_BATCHES	3

#define SENSOR_SIZE	(TVDEC_ACTIVE_COL * TVDEC_ACTIVE_LINES * \
			TVDEC_BYTES_PER_PIXEL)

#define SGA_ODD_TST_BIT  31
#define SGA_EVEN_TST_BIT 30
/* line duration is always 64 us (on PAL standard as well as on NTSC) */
#define LINE_DURATION_US 64

/* Number of Ax buffers to be displayed in Mx (ltdc_layer_update) before
 * completing the Linux handover
 */
#define CAM_HOVER_NB_BUF 3

/* Maximum elements in Mx buffer queue */
#define MX_QUEUE_MAX_ELEMS 4
/* Maximum elements in Ax buffer queue */
#define AX_QUEUE_MAX_ELEMS 12

/* Enables debug trace while camera is running */
/*#define CAM_DEBUG_RUNNING*/

#ifdef CAM_DEBUG_RUNNING
#define TRACE_INFO_RUNNING TRACE_INFO
#else
#define TRACE_INFO_RUNNING(...)
#endif

#define TRACE_FUNC_IN()		TRACE_INFO("> %s [%d]\n", __func__, __LINE__)
#define TRACE_FUNC_OUT()	TRACE_INFO("< %s [%d]\n", __func__, __LINE__)
#define TRACE_FUNC_OUT_RET(ret) \
		TRACE_INFO("< %s - ret=%d [%d]\n", __func__, ret, __LINE__)

#ifdef DEBUG_RVC_PERF

#define CAM_PERF_NB_FRAMES 100

struct cam_performance {
	uint32_t vip_start_odd;
	uint32_t vip_end_odd;
	uint32_t vip_start_even;
	uint32_t vip_end_even;
	uint32_t sga_it;
};

static uint32_t frame_id;
static struct cam_performance cam_perf[CAM_PERF_NB_FRAMES];
#endif

#ifdef DEBUG_RVC_STATS
struct cam_statistics {
	uint32_t nb_buf_done;
	uint32_t timestamp;
	uint32_t start_ms;
	uint32_t end_ms;
	uint32_t duration;
	uint32_t min_duration;
	uint32_t max_duration;
#if defined PP_USAGE
	uint32_t ts_pp_duration;
#endif
};

static struct cam_statistics cam_stat;
static bool first = true;
#endif

enum cam_sem_type {
	/* rvc task: wait camera to be started */
	CAM_START,
	/* control task: wait camera has been successfully started */
	CAM_START_ACK,
	/* control task: wait camera has been successfully stopped */
	CAM_STOP,
	/* rvc task: wait SGA warmup to be done */
	CAM_WARMUP,
	/* rvc task: wait Ax stop streaming event */
	CAM_STOP_STREAMING_SYNCHRO,
	CAM_MAX_SEM,
};

/* Margin added especially in case other tasks with higher prio are running */
#define MARGIN_DURATION_MS 100

/* PAL: 25 frames per seconds (50 fields per seconds) */
#define FIELD_MAX_DURATION_MS 20
#define FRAME_MAX_DURATION_MS (2 * FIELD_MAX_DURATION_MS)

#define FRAME_DELAY_MS (FRAME_MAX_DURATION_MS + MARGIN_DURATION_MS)

#define WARMUP_DELAY_MS (FIELDS_TO_SKIP * FIELD_MAX_DURATION_MS + \
			 MARGIN_DURATION_MS)
#define START_CAM_DELAY_MS (WARMUP_DELAY_MS + MARGIN_DURATION_MS)

/* Max delay before stop streaming message sent from Cortex Ax CPU */
#define STOP_STREAMING_SYNCHRO_DELAY_MS (500)
#define STOP_CAM_DELAY_MS (STOP_STREAMING_SYNCHRO_DELAY_MS + \
			   FRAME_MAX_DURATION_MS + MARGIN_DURATION_MS)

/* Workaround impossible to close I2C */
static bool skip_sensor_init;
static struct cam_context g_cam_context;
static TaskHandle_t g_cam_main_task_handle;
static SemaphoreHandle_t g_cam_sem[CAM_MAX_SEM];
/* Used to protect against concurrent calls */
static SemaphoreHandle_t g_mutex_synchro;

static void cam_main_task(void);
/* SGA warmup batch callback */
static void cb_init_handler(void);
/* SGA main batch callback */
static void cb_frame_handler(void);

static inline uint32_t WAIT_FOR_COMPLETION(SemaphoreHandle_t sem)
{
	TRACE_INFO("WAIT_FOR_COMPLETION 0x%08X\n", sem);
	return xSemaphoreTake(sem, portMAX_DELAY);
}

static inline uint32_t WAIT_FOR_COMPLETION_TIMEOUT(SemaphoreHandle_t sem,
						   uint32_t delay_ms)
{
	TRACE_INFO("WAIT_FOR_COMPLETION_TIMEOUT 0x%08X - %d\n", sem, delay_ms);
	return xSemaphoreTake(sem, pdMS_TO_TICKS(delay_ms));
}

static inline void COMPLETE_FROM_ISR(SemaphoreHandle_t sem)
{
	TRACE_INFO("COMPLETE_FROM_ISR 0x%08X\n", sem);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static inline void COMPLETE(SemaphoreHandle_t sem)
{
	TRACE_INFO("COMPLETE 0x%08X\n", sem);
	xSemaphoreGive(sem);
}

static inline void cam_delete_semaphores(void)
{
	int i = 0;

	for (i = 0; i < CAM_MAX_SEM; i++) {
		if (g_cam_sem[i])
			vSemaphoreDelete(g_cam_sem[i]);
		g_cam_sem[i] = NULL;
	}
}

static inline uint32_t cam_create_semaphores(void)
{
	int i = 0;

	for (i = 0; i < CAM_MAX_SEM; i++) {
		if (g_cam_sem[i])
			TRACE_ERR("g_cam_sem[%d] already exists\n", i);
		else
			g_cam_sem[i] = xSemaphoreCreateBinary();

		if (!g_cam_sem[i]) {
			TRACE_ERR("Cannot create g_cam_sem[%d]\n", i);
			cam_delete_semaphores();
			return -1;
		}
	}

	return 0;
}

/**
 * @brief  Push a new buffer in the buffer queue
 *
 * @param  phys_addr	(IN)	Buffer physical address
 * @param  priv		(IN)	priv data to be returned to Ax
 */
void cam_new_buffer(uint32_t phys_addr,
		    uint32_t priv,
		    enum cam_buffer_type buffer_type)
{
	struct buffer_elem elem = {
		.phys_addr = phys_addr,
		.priv = priv,
		.buffer_type = buffer_type,
	};

	TRACE_INFO_RUNNING("cam_new_buffer phy: 0x%08X, priv:0x%08X, type:%d\n",
			   phys_addr, priv, buffer_type);

	switch (buffer_type) {
	case CAM_BUFFER_Mx:
		if (xQueueSendToBack(g_cam_context.mx_buffer_queue,
				     (void *)&elem,
				     pdMS_TO_TICKS(10)) != pdPASS) {
			TRACE_ERR("Failed to queue Mx buffer\n");
			return;
		}
		break;
	case CAM_BUFFER_Ax:
		if (xQueueSendToBack(g_cam_context.ax_buffer_queue,
				     (void *)&elem,
				     pdMS_TO_TICKS(10)) != pdPASS) {
			TRACE_ERR("Failed to queue Ax buffer\n");
			return;
		}

		set_bit(g_cam_context.handover, BIT(1));

		g_cam_context.nb_buffers_returned++;

		/* Ax buffer sent back to Mx:
		 * It means that Linux has started to display RVC
		 * LTDC layer can be safely released in Mx
		 */
		if (g_cam_context.ltdc_layer_enabled &&
		    read_bit(g_cam_context.handover, BIT(2)) &&
		    (g_cam_context.nb_buffers_returned == CAM_HOVER_NB_BUF)) {
			g_cam_context.ltdc_layer_enabled = false;
			ltdc_layer_release_top_layer(g_cam_context.layer_id);
		}
		break;
	default:
		TRACE_ERR("Unknown buffer type: %d\n", buffer_type);
	}
}

/**
 * @brief  Rear Camera Hardware and global initialization
 *         This function has to be called once at the
 *         very first boot up to avoid conflicts onto IP whence
 *         apps processor is booting up
 *
 * @param  input		ADV7182 input selected
 * @param  std			ADV7182 detected standard
 * @return			HW error condition
 */
int8_t cam_hw_init(uint32_t input, uint32_t *std)
{
	/*
	 * Initialize the Smart Graphic Accelerator
	 * It will loop in its firmware until a batch is
	 * linked
	 */
	TRACE_INFO("Configuring Smart Graphic Accelerator\n");

	vSemaphoreCreateBinary(g_mutex_synchro);
	if (!g_mutex_synchro) {
		TRACE_ERR("CAM: Cannot create semaphore\n");
		return -1;
	}

	memset(&g_cam_context, 0, sizeof(struct cam_context));
	g_cam_context.state = CAM_UNINITIALIZED;

	/*
	 * Initialize Camera Sensor
	 */
	/*  Workaround  */
	if (!skip_sensor_init) {
		if (tv_dec_setup(input, std) != SENSOR_OK) {
			TRACE_ERR("--> Error initialising Camera Sensor\n");
			return -1;
		}
		TRACE_INFO("--> TVDEC OK\n");
		skip_sensor_init = true;
	}

	g_cam_context.state = CAM_HW_INIT;
	return 0;
}

/**
 * @brief  Build SGA warmup batch
 *
 * @param    void
 */
static void build_sga_warmup_batch(void)
{
	uint32_t *instr_ptr = NULL;
	int s = 0;

	/*
	 *  Warm-up Batch Building
	 */
	sga_pick_batch(&g_cam_context.init_batch_id);
	sga_register_int_cb(g_cam_context.ctx_batch,
			    g_cam_context.init_batch_id,
			    cb_init_handler);
	sga_get_batch_addr(g_cam_context.ctx_batch,
			   g_cam_context.init_batch_id,
			   &instr_ptr);

	/* Pixel Format Image 0 = Monochrome 1bpp */
	*instr_ptr++ = IN0_SET_PIXEL_TYPE;
	/* Pixel Format Image 2 = Monochrome 1bpp */
	*instr_ptr++ = IN2_SET_PIXEL_TYPE;

	/* HProt fixed value, BurstType INCR8, Auto Fetch Mode */
	*instr_ptr++ = AHB | SGA_AHB_CONFIGURATION;

	/* AutoFlushBank Active, Manual Flush, AutoInitCache */
	*instr_ptr++ = CACHE_CTRL  | 0x7FC0;

	/* Transparency is disabled at Input and output  */
	*instr_ptr++ = TRANSP_MODE | 0x00;
	/* Transparency KeyColor */
	*instr_ptr++ = TRANSP_IN_COLOR | 0x00;

	for (s = 0; s < FIELDS_TO_SKIP; s++) {
		*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFF1; /* Wait VIP EOF */
		*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFF0; /* Wait VIP Line */
	}

	sga_commit_batch(g_cam_context.ctx_batch,
			 g_cam_context.init_batch_id,
			 instr_ptr);
}

/**
 * @brief  Build SGA grabbing part of main batch
 *         This is used to grab data from VIP to a DDR buffer
 *
 * @param  is_odd	(IN)	true: grabs odd field lines
 *				false: grabs even field lines
 * @param  deinterlace_needed(IN)	interlacing enabled
 * @param  in		(IN)	input SGA surface
 * @param  out		(IN)	output SGA surface
 * @param  addr		(OUT)	current batch address
 */
static int build_sga_grab_lines(bool is_odd,
				bool deinterlace_needed,
				t_sga_surface *in,
				t_sga_surface *out,
				uint32_t **addr)
{
	uint32_t *instr_ptr = *addr;
	uint32_t line_index = 0;
	uint32_t start_index = deinterlace_needed ? (is_odd ? 1 : 0) : 0;
	uint32_t line_jump = deinterlace_needed ? 2 : 1;
	uint32_t tst_register_bit = is_odd ? SGA_EVEN_TST_BIT : SGA_ODD_TST_BIT;
	/* Unit for WAIT_NEW_SYNCHRO timeout delay is: 256 SGA clock cycle
	 * Set timeout as 2 times the expected line duration
	 */
	uint32_t line_delay = (2 * LINE_DURATION_US * SGA_FREQUENCY_MHZ) / 256;
	uint32_t nb_max_instr_per_line;
	uint32_t nb_lines;
	uint32_t *last_instr;

#ifdef CAM_TEST_PATTERN
	uint32_t in_line_nb = 0;

	if (is_odd) {
		*instr_ptr++ = IN2_BASE_ADD_MSB	|
			((g_cam_context.testbuf_odd_addr & 0xFF000000) >> 24);
		*instr_ptr++ = IN2_BASE_ADD	|
			(g_cam_context.testbuf_odd_addr & 0x00FFFFFF);
	} else {
		*instr_ptr++ = IN2_BASE_ADD_MSB	|
			((g_cam_context.testbuf_even_addr & 0xFF000000) >> 24);
		*instr_ptr++ = IN2_BASE_ADD	|
			(g_cam_context.testbuf_even_addr & 0x00FFFFFF);
	}
#endif

	*instr_ptr++ = WAIT_NEW_SYNCHRO | 0xFFFF1; /* Wait VIP EOF */

#if LINES_TO_SKIP > 0
	for (line_index = 0;
	     line_index < (uint32_t)(LINES_TO_SKIP / 2);
	     line_index++) {
		/* Wait VIP Line */
		*instr_ptr++ = WAIT_NEW_SYNCHRO		| 0xFFFF0;
	}
#endif

	/* Try to predict last instruction after grabbing all lines
	 * Also, take care on address alignment: GOTO instruction is performed
	 * on addresses multiple of 8-bytes (double instructions)
	 */
	nb_max_instr_per_line = 8; /* max instructions inside the for loop */
	nb_lines = out->height / line_jump;
	/* The '+2' is for GOTO instruction outside of for loop and for
	 * address alignment
	 */
	last_instr = instr_ptr + (nb_lines * nb_max_instr_per_line) + 2;
	last_instr = (uint32_t *)((uint32_t)last_instr & ~0x7ul);

	/*
	 * Grab VIP data into output buffer.
	 * Stop grabbing data if field parity is not as expected:
	 * On VIP interrupt, M3 sets SGA test register to SGA_ODD_TST_BIT or
	 * SGA_EVEN_TST_BIT depending on field parity.
	 * Then, this part of SGA batch is testing the test register to stop
	 * grabbing as soon as parity is not as expected
	 */
	for (line_index = start_index;
	     line_index < out->height;
	     line_index += line_jump) {
#ifdef CAM_TEST_PATTERN
		*instr_ptr++ = SET_Y_OFFSET | SGA_TEX_IN2 | (in_line_nb++) << 2;
#endif
		*instr_ptr++ = SET_POINT0	|
			SGA_PACK_VECTOR_SCREEN(out->m_crop.m_x, line_index, 0);
		*instr_ptr++ = SET_POINT1	|
			SGA_PACK_VECTOR_SCREEN(out->m_crop.m_x +
					       out->m_crop.m_w - 1,
					       line_index, 0);
		/* Wait VIP line */
		if (line_index == start_index)
			*instr_ptr++ = WAIT_NEW_SYNCHRO	| 0xFFFF0;
		else
			*instr_ptr++ = WAIT_NEW_SYNCHRO	|
					(line_delay << 4) |
					0x0;
		*instr_ptr++ = DRAW_LINE;
		*instr_ptr++ = TST_INSTR_TEST_REG | tst_register_bit;
		*instr_ptr++ = GOTO | ((uint32_t)last_instr >> 3);
	}

	*instr_ptr++ = GOTO | ((uint32_t)last_instr >> 3);

	if (instr_ptr > last_instr)
		TRACE_ERR("Error in SGA grab batch\n");

	while ((uint32_t)instr_ptr < (uint32_t)last_instr)
		*instr_ptr++ = NO_OP;

	*addr = instr_ptr;
	return 0;
}

/**
 * @brief  Build SGA grabbing configuration batch
 *         This is used to configure SGA for grabbing data from VIP to
 *         a DDR buffer.
 *
 * @param  in		(IN)	input SGA surface
 * @param  out		(IN)	output SGA surface
 * @param  addr		(OUT)	current batch address
 */
static int build_sga_grab_batch(t_sga_surface *in,
				t_sga_surface *out,
				uint32_t **addr)
{
	uint32_t *instr_ptr = *addr;
	uint32_t xx_coeff = 0x0;

	/* Pixel Format Image 0 = Monochrome 1bpp, in0 Off */
	*instr_ptr++ = IN0_SET_PIXEL_TYPE;
	/* Pixel Format Image 0 = Monochrome 1bpp, in1 Off */
	*instr_ptr++ = IN1_SET_PIXEL_TYPE;
	/* Pixel Format Image 0 = Monochrome 1bpp, in2 Off */
	*instr_ptr++ = IN2_SET_PIXEL_TYPE;

	/* HProt fixed value, BurstType INCR8, Auto Fetch Mode */
	*instr_ptr++ = AHB | 0x0120;

	/* AutoFlushBank Active, Manual Flush, AutoInitCache */
	*instr_ptr++ = CACHE_CTRL  | 0x7FD0;

	/* Transparency is disabled at Input and output  */
	*instr_ptr++ = TRANSP_MODE | 0x00;
	/*Transparency KeyColor */
	*instr_ptr++ = TRANSP_IN_COLOR | 0x00;

	/* Allows to configure the Blending Environment of
	 * the Texture Blending Units
	 */
	*instr_ptr++ =
		/* Texture Unit ID #0 */
		SET_TEX_ENV_MSB | SGA_TEX_SOURCE_0 |
		/* We want to bypass this texture unit */
		(SGA_TEX_ENV_REPLACE << SHFT_RGB_FNCN) |
		/* We want to bypass this texture unit */
		(SGA_TEX_ENV_REPLACE << SHFT_ALPHA_FNCN) |
		/* We keep the composed color */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC0) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC1) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC2) |
		/* We keep the composed alpha */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC0) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC1) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC2);

	*instr_ptr++ =
		/* Texture Unit ID #0 */
		SET_TEX_ENV_LSB | SGA_TEX_SOURCE_0 |
		/* Color channel will remain in the color channel */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR0_RGB) |
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR1_RGB) |
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR2_RGB) |
		/* Alpha channel will remain in the color channel */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR0_ALPHA) |
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR1_ALPHA) |
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR2_ALPHA);

	*instr_ptr++ =
		/* Texture Unit ID #1 */
		SET_TEX_ENV_MSB | SGA_TEX_SOURCE_1 |
		/* We want to bypass this texture unit */
		(SGA_TEX_ENV_REPLACE << SHFT_RGB_FNCN) |
		/* We want to bypass this texture unit */
		(SGA_TEX_ENV_REPLACE << SHFT_ALPHA_FNCN) |
		/* We keep the previously composed color */
		(SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_RGB_SRC0) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_RGB_SRC1) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_RGB_SRC2) |
		/* We keep the previously composed alpha */
		(SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_ALPHA_SRC0) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_ALPHA_SRC1) |
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_ALPHA_SRC2);

	*instr_ptr++ =
		/* Texture Unit ID #1 */
		SET_TEX_ENV_LSB | SGA_TEX_SOURCE_1 |
		/* Color channel will remain in the color channel */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR0_RGB) |
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR1_RGB) |
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR2_RGB) |
		/* Alpha channel will remain in the color channel */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR0_ALPHA) |
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR1_ALPHA) |
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR2_ALPHA);

	/* Allows to configure the RGB and
	 * Alpha Scaling in the Texture Units
	 */
	/* Rgb and alpha scales are both FF */
	*instr_ptr++ = SET_TEX_SCALE | SGA_TEX_SOURCE_1 | 0xFFFF;
	/* Rgb and alpha scales are both FF */
	*instr_ptr++ = SET_TEX_SCALE | SGA_TEX_SOURCE_0 | 0xFFFF;

	/*
	 * Allows to configure the Green and Blue component of
	 * the Constant colour in the Texture Units.
	 */
	/* Texture Unit ID #0, Green(0xff) Constant color  */
	*instr_ptr++ = SET_TEX_COLORLSB | SGA_TEX_SOURCE_0 | 0xFF00;
	/* Texture Unit ID #0, Green(0xff) Constant color  */
	*instr_ptr++ = SET_TEX_COLORLSB | SGA_TEX_SOURCE_1 | 0xFF00;
	/*
	 * Allows to configure the Alpha and Red component of
	 * the Constant colour used in the Texture Units.
	 */
	/* Texture Unit ID #0, Alpha(0xff) Constant color  */
	*instr_ptr++ = SET_TEX_COLORMSB | SGA_TEX_SOURCE_0 | 0xFF00;
	/* Texture Unit ID #0, Alpha(0xff) Constant color  */
	*instr_ptr++ = SET_TEX_COLORMSB | SGA_TEX_SOURCE_1 | 0xFF00;

	/* Allows to configure the Blending Environment of
	 * the Frame Blending Unit
	 */
	*instr_ptr++ =
		/* Add Blend Equation */
		SET_BLEND_ENV | SGA_BLEND_OP_ADD |
		/* Coef1 for RGB Frag  */
		(SGA_BLEND_SRC_COEF_1 << SHFT_RGB_FRAG_SRC) |
		/* Coef1 for Alpha Frag  */
		(SGA_BLEND_SRC_COEF_1 << SHFT_ALPHA_FRAG_SRC) |
		/* Coef0 for RGB Frame */
		(SGA_BLEND_SRC_COEF_0 << SHFT_RGB_FRAME_SRC) |
		/* Coef0 for Alpga Frame  */
		(SGA_BLEND_SRC_COEF_0 << SHFT_ALPHA_FRAME_SRC);

	*instr_ptr++ = PIXEL_OP_MODE	| SGA_STOP_CONCAT |
		SGA_ACTIVATE_SCISSOR_OPR |
		SGA_PIXEL_OPERATOR_BYPASS;

	/* Sets the amount of bytes to jump between 2 lines */
	*instr_ptr++ = IN2_SET_LINE_JUMP |
		       ((out->width * out->format.color_depth) & 0x1FFF);

	/* Sets the X and Y size of the picture to process */
	*instr_ptr++ = IN2_SET_SIZE_XY	| out->m_size_xy;

	/*
	 * Sets the signed pixel shift between a coordinate in
	 * the output image and in the input image 1
	 */
	*instr_ptr++ = IN2_SET_DELTA_XY	| 0x0;

	*instr_ptr++ = SET_X_OFFSET | SGA_TEX_IN2;

	*instr_ptr++ = SET_XY_MODE  | SGA_TEX_IN2 |
		       SGA_XYMODE_CLAMPING_X   | SGA_XYMODE_CLAMPING_Y |
		       SGA_XYMODE_ENABLE_ROTATION;

	xx_coeff = (uint32_t)((in->m_crop.m_w << 12) / (out->m_crop.m_w));

	*instr_ptr++ = SET_XX_COEF  | SGA_TEX_IN2 | (xx_coeff & 0x1FFFF);
	*instr_ptr++ = SET_YX_COEF  | SGA_TEX_IN2;
	*instr_ptr++ = SET_XY_COEF  | SGA_TEX_IN2;
	*instr_ptr++ = SET_YY_COEF  | SGA_TEX_IN2 | (1 * 4096);
	*instr_ptr++ = SET_WX_COEF  | SGA_TEX_IN2;
	*instr_ptr++ = SET_WY_COEF  | SGA_TEX_IN2;
	*instr_ptr++ = SET_Y_OFFSET | SGA_TEX_IN2;
	*instr_ptr++ = SET_W_OFFSET | SGA_TEX_IN2;

	/* Sets the pixel format of the input image 1. */
#ifdef CAM_TEST_PATTERN
	*instr_ptr++ = IN2_SET_PIXEL_TYPE	| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_LITTLE_ENDIAN
		| in->format.buffer_fmt
		| in->swap_rb;
#else
	*instr_ptr++ = IN2_SET_PIXEL_TYPE	| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_LITTLE_ENDIAN
		| in->format.buffer_fmt
		| in->swap_rb;
#endif

	*instr_ptr++ = IN2_BASE_ADD_MSB	| ((in->addr & 0xFF000000) >> 24);
	*instr_ptr++ = IN2_BASE_ADD	| (in->addr & 0x00FFFFFF);

	/* Sets the pixel format of the input image 0. */
	*instr_ptr++ = OUT_SET_PIXEL_TYPE	| out->swap_rb
		| out->format.buffer_fmt
		| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_LITTLE_ENDIAN;

	*instr_ptr++ = OUT_SET_LINE_JUMP |
		       ((out->width * out->format.color_depth) & 0x1FFF);
	*instr_ptr++ = OUT_SET_SIZE_XY | out->m_size_xy;
	*instr_ptr++ = OUT_SET_BASE_XY | 0x0;

	sga_set_fb_instr_ptr_addr(g_cam_context.ctx_batch,
				  g_cam_context.fb_batch_id,
				  instr_ptr);

	*instr_ptr++ = OUT_BASE_ADD_MSB	| ((out->addr & 0xFF000000) >> 24);
	*instr_ptr++ = OUT_BASE_ADD	| (out->addr & 0x00FFFFFF);

	*addr = instr_ptr;
	return 0;
}

#ifdef CAM_SCALING
/**
 * @brief  Build SGA scaling batch
 *         This is used to rescale image previoulsy grabbed from VIP
 *
 * @param  in		(IN)	input SGA surface
 * @param  out		(IN)	output SGA surface
 * @param  texture	(IN)	SGA texture surface (only used for overlay)
 * @param  addr		(OUT)	current batch address
 * @param  id		(IN)	batch ID
 */
static int build_sga_rescale_batch(t_sga_surface *in,
				   t_sga_surface *out,
				   t_sga_surface *texture,
				   uint32_t **addr)
{
	uint32_t *instr_ptr = *addr;
	uint32_t xx_coeff	= 0x0;
	uint32_t xy_coeff	= 0x0;
	uint32_t yx_coeff	= 0x0;
	uint32_t yy_coeff	= 0x0;

	/*
	 * Second part of the batch.
	 * From the first intermediate buffer
	 * perform a vertical stretch
	 * activating the bilinear filter
	 * into second intermediate
	 * buffer
	 */
	*instr_ptr++ = IN2_SET_PIXEL_TYPE	| in->format.buffer_fmt
		| in->swap_rb
		| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_ALPHA_255
		| SGA_PIX_FMT_LITTLE_ENDIAN;

	*instr_ptr++ = IN2_SET_LINE_JUMP |
		       ((in->format.color_depth * in->width) & 0x1FFF);
	*instr_ptr++ = IN2_SET_DELTA_XY |
		       SGA_PACK_VECTOR(in->m_crop.m_x, in->m_crop.m_y);
	*instr_ptr++ = IN2_SET_SIZE_XY |
		       SGA_PACK_VECTOR(in->m_crop.m_x + in->m_crop.m_w,
				       in->m_crop.m_y + in->m_crop.m_h);
	*instr_ptr++ = IN2_BASE_ADD_MSB | ((in->addr & 0xFF000000) >> 24);
	*instr_ptr++ = IN2_BASE_ADD | (in->addr & 0x00FFFFFF);

	*instr_ptr++ = SET_TEX_ENV_MSB | SGA_TEX_SOURCE_0	|
		/* We want to bypass this texture unit */
		(SGA_TEX_ENV_REPLACE << SHFT_RGB_FNCN)		|
		/* We want to bypass this texture unit */
		(SGA_TEX_ENV_REPLACE << SHFT_ALPHA_FNCN)	|
		/* Texture */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC0)	|
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC1)	|
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC2)	|
		/* We keep the previous composed alpha */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC0)	|
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC1)	|
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC2);

	*instr_ptr++ = SET_TEX_ENV_LSB | SGA_TEX_SOURCE_0	|
		/* Color channel will remain in the color channel */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR0_RGB)	|
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR1_RGB)	|
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_ALPHA << SHFT_OPR2_RGB)	|
		/* Alpha channel will remain in the color channel */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR0_ALPHA)	|
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR1_ALPHA)	|
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR2_ALPHA);

	*instr_ptr++ = SET_TEX_ENV_MSB | SGA_TEX_SOURCE_1	|
		/* Interpolate */
		(SGA_TEX_ENV_REPLACE << SHFT_RGB_FNCN)		|
		/* Replace */
		(SGA_TEX_ENV_REPLACE << SHFT_ALPHA_FNCN)	|
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC0)	|
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_RGB_SRC1)	|
		(SGA_TEX_ENV_SOURCE_PREVIOUS_COL << SHFT_RGB_SRC2)	|
		/* Alpha from texture */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC0)	|
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC1)	|
		/* Not relevant */
		(SGA_TEX_ENV_SOURCE_TEXTURE << SHFT_ALPHA_SRC2);

	*instr_ptr++ = SET_TEX_ENV_LSB | SGA_TEX_SOURCE_1		|
		/* Color channel will remain in the color channel */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR0_RGB)		|
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_COLOR << SHFT_OPR1_RGB)		|
		/* Not relevant */
		(SGA_TEX_ENV_RGB_OPR_ALPHA << SHFT_OPR2_RGB)		|
		/* Alpha channel will remain in the color channel */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR0_ALPHA)		|
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR1_ALPHA)		|
		/* Not relevant */
		(SGA_TEX_ENV_A_OPR_ALPHA << SHFT_OPR2_ALPHA);

	/* Rgb and alpha scales are both FF */
	*instr_ptr++ = SET_TEX_SCALE | SGA_TEX_SOURCE_0 | 0xFFFF;
	/* Rgb and alpha scales are both FF */
	*instr_ptr++ = SET_TEX_SCALE | SGA_TEX_SOURCE_1 | 0xFFFF;

	*instr_ptr++ = SET_TEX_COLORLSB | SGA_TEX_SOURCE_0 | 0xFFFF;
	*instr_ptr++ = SET_TEX_COLORMSB | SGA_TEX_SOURCE_0 | 0xFFFF;

	*instr_ptr++ = SET_TEX_COLORLSB | SGA_TEX_SOURCE_1 | 0xFFFF;
	*instr_ptr++ = SET_TEX_COLORMSB | SGA_TEX_SOURCE_1 | 0xFFFF;

	/* Allows to configure the Blending Environment of
	 * the Frame Blending Unit
	 */
	*instr_ptr++ =
		/* Add Blend Equation */
		SET_BLEND_ENV | SGA_BLEND_OP_ADD			|
		/* Coef=1-A for RGB Frag */
		(SGA_BLEND_SRC_COEF_1_A_FRAME	<< SHFT_RGB_FRAG_SRC)	|
		/* Coef=1-A for Alpha Frag  */
		(SGA_BLEND_SRC_COEF_1_A_FRAME	<< SHFT_ALPHA_FRAG_SRC)	|
		/* Coef=A for RGB Frame */
		(SGA_BLEND_SRC_COEF_A_FRAME	<< SHFT_RGB_FRAME_SRC)	|
		/* Coef=A for Alpga Frame  */
		(SGA_BLEND_SRC_COEF_A_FRAME	<< SHFT_ALPHA_FRAME_SRC);

	/* Configure ChromaKey */
	*instr_ptr++ = TRANSP_IN_COLOR		| g_cam_context.color_key;
	*instr_ptr++ = TRANSP_MODE		| 0xA8;

	*instr_ptr++ = PIXEL_OP_MODE;

	*instr_ptr++ = OUT_SET_PIXEL_TYPE
		| out->swap_rb
		| out->format.buffer_fmt
		| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_LITTLE_ENDIAN;

	*instr_ptr++ = OUT_SET_LINE_JUMP |
		       ((out->width * out->format.color_depth) & 0x1FFF);
	*instr_ptr++ = OUT_SET_SIZE_XY |
		       SGA_PACK_VECTOR(out->m_crop.m_x + out->m_crop.m_w,
				       out->m_crop.m_y + out->m_crop.m_h);
	*instr_ptr++ = OUT_SET_BASE_XY	|
		       SGA_PACK_VECTOR(out->m_crop.m_x, out->m_crop.m_y);

	sga_set_fb_instr_ptr_addr(g_cam_context.ctx_batch,
				  g_cam_context.fb_batch_id,
				  instr_ptr);

	/*
	 * The next instruction contains the
	 * Frame Buffer address
	 */
	*instr_ptr++ = OUT_BASE_ADD_MSB	| ((out->addr & 0xFF000000) >> 24);
	*instr_ptr++ = OUT_BASE_ADD	| (out->addr & 0x00FFFFFF);

	xx_coeff = (uint32_t)((in->m_crop.m_w << 12) / (out->m_crop.m_w));
	xy_coeff = 0x0;
	yx_coeff = 0x0;
	yy_coeff = (uint32_t)(((in->m_crop.m_h /* LINES_TO_SKIP*/) << 12) /
			      (out->m_crop.m_h));

	*instr_ptr++ = SET_XX_COEF | SGA_TEX_IN2 | (xx_coeff & 0x1FFFF);
	*instr_ptr++ = SET_XY_COEF | SGA_TEX_IN2 | (xy_coeff & 0x1FFFF);
	*instr_ptr++ = SET_YX_COEF | SGA_TEX_IN2 | (yx_coeff & 0x1FFFF);
	*instr_ptr++ = SET_YY_COEF | SGA_TEX_IN2 | (yy_coeff & 0x1FFFF);

	*instr_ptr++ = SET_WX_COEF  | SGA_TEX_IN2 | 0x0;
	*instr_ptr++ = SET_WY_COEF  | SGA_TEX_IN2 | 0x0;

	*instr_ptr++ = SET_Y_OFFSET | SGA_TEX_IN2 | 0x0;
	*instr_ptr++ = SET_W_OFFSET | SGA_TEX_IN2 | 0x0;

#ifdef CAM_OVERLAY
	/*
	 * Set up input overlay surface
	 */
	*instr_ptr++ = IN0_SET_PIXEL_TYPE
		| texture->format.buffer_fmt
		| texture->swap_rb
		| SGA_PIX_TYPE_ACTIVATE_FLOW
		| SGA_PIX_FMT_LITTLE_ENDIAN
		| SGA_PIX_FMT_ALPHA_255;

	*instr_ptr++ = IN0_SET_LINE_JUMP |
		       ((texture->format.color_depth * texture->width) &
			0x1FFF);
	*instr_ptr++ = IN0_SET_DELTA_XY |
		       SGA_PACK_VECTOR(texture->m_crop.m_x,
				       texture->m_crop.m_y);
	*instr_ptr++ = IN0_SET_SIZE_XY |
		       SGA_PACK_VECTOR(texture->m_crop.m_x +
				       texture->m_crop.m_w,
				       texture->m_crop.m_y +
				       texture->m_crop.m_h);

	*instr_ptr++ = IN0_BASE_ADD_MSB | ((texture->addr & 0xFF000000) >> 24);
	*instr_ptr++ = IN0_BASE_ADD | (texture->addr & 0x00FFFFFF);
#endif /* CAM_OVERLAY */

	/*
	 * Finally draw to the candidate frame buffer
	 */
	int DX = out->m_crop.m_x;
	int DTx = DX * (in->width << 2) / (out->width) + 0x2;

	*instr_ptr++ = SET_X_OFFSET | SGA_TEX_IN2 | DTx;
	*instr_ptr++ = SET_POINT0 |
		SGA_PACK_VECTOR_SCREEN(out->m_crop.m_x,
				       out->m_crop.m_y, 0);
	*instr_ptr++ = SET_POINT1 |
		SGA_PACK_VECTOR_SCREEN(out->m_crop.m_x +
				       out->m_crop.m_w - 1,
				       out->m_crop.m_y +
				       out->m_crop.m_h - 1,
				       0);
	*instr_ptr++ = DRAW_RECTANGLE;
	*instr_ptr++ = WAIT_PIPE_EMPTY;

	*addr = instr_ptr;
	return 0;
}
#endif

/**
 * @brief  Build SGA main batch
 *
 * @param  output	(IN)	Position and Size on the screen
 */
static int build_sga_main_batch(struct cam_rect *output)
{
	t_sga_surface		vip_context;
	t_sga_surface		fb_context;

	uint32_t	*instr_ptr			= NULL;

	/*
	 * Create the batch context
	 */
	t_sga_surface	*display				= NULL;
	t_sga_surface	*surf				= NULL;

#ifdef CAM_OVERLAY
	t_sga_surface texture_context;
#endif

#ifdef CAM_SCALING
	t_sga_surface *text0 = NULL;
	t_sga_surface	intermediate_context;
	t_sga_surface	*inter_1				= NULL;

	uint32_t    R, G, B;

	/*  Compute ChromaKey values */
	/* g_cam_context.color_key = TRANSPARENT_COLOR; */
	B	= (((TRANSPARENT_COLOR & 0x1F)   << 3)	| 0x4);
	G	= (((TRANSPARENT_COLOR & 0x07E0) >> 3)	| 0x2);
	R	= (((TRANSPARENT_COLOR & 0xF800) >> 8)	| 0x4);
	g_cam_context.color_key = B | (G << 8) | (R << 16);
#endif

	/*
	 * This is the sensor surface
	 * ADDR:	VIP FIFO Addr
	 * FORMAT:	YUV 422
	 * SIZE:	uSurfWidth * uSurfHeigth
	 */
	surf  = &vip_context;
	surf->addr		= (uint32_t)(VIP_BASE + 0x3000);
	surf->format		= SGA_DIS_FMT_YUV422;
	surf->width		= TVDEC_ACTIVE_COL;
	surf->height		= TVDEC_ACTIVE_LINES;
	surf->swap_rb		= 0;
	surf->m_size_xy		= SGA_PACK_VECTOR_GUARDBAND(surf->width,
							    surf->height,
							    0);
	surf->m_crop.m_x	= 0; /* Define the cropping in the frame */
	surf->m_crop.m_y	= 0;
	surf->m_crop.m_w	= surf->width;
	surf->m_crop.m_h	= surf->height;

	GUARDBAND_SET_ADDRESS(surf);

#ifdef CAM_SCALING
	/*
	 * This is the sensor surface
	 * ADDR:	SENSOR
	 * FORMAT:	RGB 565
	 * SIZE:	uSurfHeigth * disp_width
	 */
	inter_1  = &intermediate_context;

	inter_1->addr		= g_cam_context.sensor_addr;
	inter_1->format		= SGA_DIS_FMT_YUV422;
	inter_1->width		= TVDEC_ACTIVE_COL;
	inter_1->height		= TVDEC_ACTIVE_LINES;
	inter_1->swap_rb	= 0 /*SGA_PIX_FMT_BGR */;
	inter_1->m_size_xy	= SGA_PACK_VECTOR_GUARDBAND(inter_1->width,
							    inter_1->height,
							    0);
	inter_1->m_crop.m_x	= 0;
	inter_1->m_crop.m_y	= 0;
	inter_1->m_crop.m_w	= inter_1->width;
	inter_1->m_crop.m_h	= inter_1->height;

	GUARDBAND_SET_ADDRESS(inter_1);
#endif
	/*
	 * This is the Frame Buffer surface
	 * ADDR:	SGA_FB_SURFACE_ADDRESS
	 * FORMAT:	RGB 565
	 * SIZE:	disp_heigth * disp_width
	 */
	display  = &fb_context;

	display->addr		= 0;	/* Filled in after */
	display->format		= SGA_DIS_FMT_YUV422;
#if !defined PP_USAGE
	display->width		= g_cam_context.out.w;
	display->height		= g_cam_context.out.h;
#else
	display->width		= TVDEC_ACTIVE_COL;
	display->height		= TVDEC_ACTIVE_LINES;
#endif
	display->swap_rb	= 0;	/*SGA_PIX_FMT_BGR */
	display->m_size_xy	= SGA_PACK_VECTOR_GUARDBAND(display->width,
							    display->height,
							    0);
	/* Define the cropping into final image */
	display->m_crop.m_x	= 0;
	/* Define the cropping into final image */
	display->m_crop.m_y	= 0;
	/* Define the cropping into final image */
	display->m_crop.m_w	= display->width;
	/* Define the cropping into final image */
	display->m_crop.m_h	= display->height;

	GUARDBAND_SET_ADDRESS(display);

#ifdef CAM_OVERLAY
	/*
	 * This is the overlay texture surface
	 * ADDR:	depending on scatter region
	 * FORMAT:	RGB565
	 * SIZE:	disp_heigth * disp_width
	 */
	text0  = &texture_context;

	text0->addr		= g_cam_context.text0_addr;
	text0->format		= SGA_DIS_FMT_RGB565;
	text0->width		= display->width;
	text0->height		= display->height;
	text0->swap_rb		= 0;
	text0->m_size_xy	= SGA_PACK_VECTOR_GUARDBAND(text0->width,
							    text0->height,
							    0);
	text0->m_crop.m_x	= 0;
	text0->m_crop.m_y	= 0;
	text0->m_crop.m_w	= text0->width;
	text0->m_crop.m_h	= text0->height;

	GUARDBAND_SET_ADDRESS(text0);
#endif

	sga_pick_batch(&g_cam_context.fb_batch_id);

	/* Register both batch ID on the same Handler */
	sga_register_int_cb(g_cam_context.ctx_batch,
			    g_cam_context.fb_batch_id,
			    cb_frame_handler);
	sga_get_batch_addr(g_cam_context.ctx_batch,
			   g_cam_context.fb_batch_id,
			   &instr_ptr);

#if defined(PP_USAGE)
/* Grab ODD and EVEN lines without rescale */
	build_sga_grab_batch(surf, display, &instr_ptr);
	build_sga_grab_lines(true, true, surf, display, &instr_ptr);
	build_sga_grab_lines(false, true, surf, display, &instr_ptr);
#else
/* Grab ODD lines only */
#if !defined(CAM_SCALING) && defined(CAM_NO_DEINTERLEAVING)
	build_sga_grab_batch(surf, display, &instr_ptr);
	build_sga_grab_lines(true, false, surf, display, &instr_ptr);
#endif

/* Grab ODD lines only and rescale image */
#if defined(CAM_SCALING) && defined(CAM_NO_DEINTERLEAVING)
	build_sga_grab_batch(surf, inter_1, &instr_ptr);
	build_sga_grab_lines(true, false, surf, inter_1, &instr_ptr);
	build_sga_rescale_batch(inter_1, display, text0, &instr_ptr);
#endif

/* Grab ODD and EVEN lines AND rescale */
#if defined(CAM_SCALING) && !defined(CAM_NO_DEINTERLEAVING)
	build_sga_grab_batch(surf, inter_1, &instr_ptr);
	build_sga_grab_lines(true, true, surf, inter_1, &instr_ptr);
	build_sga_grab_lines(false, true, surf, inter_1, &instr_ptr);
	build_sga_rescale_batch(inter_1, display, text0, &instr_ptr);
#endif

/* Grab ODD and EVEN lines without rescale */
#if !defined(CAM_SCALING) && !defined(CAM_NO_DEINTERLEAVING)
	build_sga_grab_batch(surf, display, &instr_ptr);
	build_sga_grab_lines(true, true, surf, display, &instr_ptr);
	build_sga_grab_lines(false, true, surf, display, &instr_ptr);
#endif
#endif

	sga_commit_batch(g_cam_context.ctx_batch,
			 g_cam_context.fb_batch_id,
			 instr_ptr);

	return 0;
}

/**
 * @brief  Rear Camera Batches Creation
 *
 * @param    output	(IN)	Position and Size on the screen
 * @return			SGA error condition
 */
int8_t cam_init_rear_camera(struct cam_rect *output, bool managed_by_ax)
{
	/*
	 * Get the batch number and sizes to allocate
	 * By default 2 batches, allocating the max possible size.
	 * Set all required variables, in particular point P0 and P1
	 * that are used as extremes of the rectangle to fade
	 */
	uint32_t	packed_data		= ((NB_BATCHES & 0xFF) << 28) |
		(SGE_BATCH_DEFAULT_SIZE & 0x00FFFFFF);

	uint32_t num_batchs	= SGE_GET_BATCH_COUNT(packed_data);
	uint32_t batch_size	= SGE_GET_BATCH_SIZE(packed_data);
	uint32_t header_size	= SGE_DO_ALIGN(sizeof(t_sga_memory_context) +
					       sizeof(t_sga_batch_context),
					       SGA_MEMORY_ALIGN_MASK);
	uint32_t ret = 0;
	uint32_t *fb_addr = 0;
#ifdef CAM_OVERLAY
	uint32_t *text_addr = 0;
#endif
#ifdef PP_USAGE
	uint32_t *out_addr = 0;
#endif
#ifdef CAM_SCALING
	uint32_t *sensor_addr = 0;
#endif

	TRACE_INFO("CAM handover=%d\n", g_cam_context.handover);

	if (g_cam_context.state < CAM_HW_INIT) {
		TRACE_ERR("Hardware not initialized\n");
		return -1;
	}

	xSemaphoreTake(g_mutex_synchro, portMAX_DELAY);

	/*
	 * Initialize Video Input Port
	 */
	if (vip_init() != VIP_OK) {
		TRACE_ERR("--> Error initialising VIP\n");
		goto cam_init_failure;
	}
	TRACE_INFO("--> VIP OK\n");

	/*
	 * Initialize the Smart Graphic Accelerator
	 * It will loop in its firmware until a batch is
	 * linked
	 */
	TRACE_INFO("Configuring Smart Graphic Accelerator\n");

	memset(&g_cam_context, 0, sizeof(struct cam_context));
	g_cam_context.state = CAM_UNINITIALIZED;

	if (sge_init() != SGA_OK) {
		TRACE_ERR("--> Error initialising SGA Firmware\n");
		goto vip_deinit;
	}
	TRACE_INFO("--> SGA OK\n");

	vip_register_cb(notif_vip_handler);

	g_cam_context.sga_running = false;
	g_cam_context.vip_running = false;

	/*
	 * Create the batch context
	 */
	batch_size = SGE_DO_ALIGN(batch_size, SGA_MEMORY_ALIGN_MASK);
	sge_do_init_batch(num_batchs,
			  batch_size,
			  header_size,
			  &g_cam_context.ctx_mem,
			  &g_cam_context.ctx_batch);

	if (!g_cam_context.ctx_batch) {
		TRACE_ERR("%s: Error on sge_do_init_batch\n", __func__);
		goto sge_deinit;
	}
#if defined CAM_SCALING
	/* Allocate sensor_addr */
	sensor_addr = memalign(SGA_BUFFER_ALIGNMENT, SENSOR_SIZE);
	if (!sensor_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto sge_deinit;
	}
	memset(sensor_addr, 0, SENSOR_SIZE);
	g_cam_context.sensor_addr = (uint32_t)sensor_addr;
#endif
#ifdef PP_USAGE

	G1ResInit();

	g_cam_context.g1_ctx = G1ResBookResource();
	if (!g_cam_context.g1_ctx) {
		TRACE_ERR("[RVC] Unable to book g1 pp unit\n");
		goto sensor_del;
	}

	/* Allocate frame buffers */
	fb_addr = memalign(SGA_BUFFER_ALIGNMENT, SENSOR_SIZE);
	if (!fb_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto sensor_del;
	}
	memset(fb_addr, 0, SENSOR_SIZE);
	g_cam_context.fb_addr[0] = (uint32_t)fb_addr;

	fb_addr = memalign(SGA_BUFFER_ALIGNMENT, SENSOR_SIZE);
	if (!fb_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto buffers_del;
	}
	memset(fb_addr, 0, SENSOR_SIZE);
	g_cam_context.fb_addr[1] = (uint32_t)fb_addr;

	out_addr = memalign(SGA_BUFFER_ALIGNMENT, SENSOR_SIZE);
	if (!out_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto sensor_del;
	}
	memset(out_addr, 0, SENSOR_SIZE);
	g_cam_context.out_convert = (uint32_t)out_addr;

	/* Allocate output buffers */
	out_addr = memalign(SGA_BUFFER_ALIGNMENT,
			    output->w * output->h * SCREEN_BPP);
	if (!out_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto sensor_del;
	}
	memset(out_addr, 0, output->w * output->h * SCREEN_BPP);
	g_cam_context.out_addr[0] = (uint32_t)out_addr;

	out_addr = memalign(SGA_BUFFER_ALIGNMENT,
			    output->w * output->h * SCREEN_BPP);
	if (!out_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto buffers_del;
	}
	memset(out_addr, 0, output->w * output->h * SCREEN_BPP);
	g_cam_context.out_addr[1] = (uint32_t)out_addr;

#else
	/* Allocate output buffers */
	fb_addr = memalign(SGA_BUFFER_ALIGNMENT,
			   output->w * output->h * SCREEN_BPP);
	if (!fb_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto sensor_del;
	}
	memset(fb_addr, 0, output->w * output->h * SCREEN_BPP);
	g_cam_context.fb_addr[0] = (uint32_t)fb_addr;

	fb_addr = memalign(SGA_BUFFER_ALIGNMENT,
			   output->w * output->h * SCREEN_BPP);
	if (!fb_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto buffers_del;
	}
	memset(fb_addr, 0, output->w * output->h * SCREEN_BPP);
	g_cam_context.fb_addr[1] = (uint32_t)fb_addr;
#endif


#ifdef CAM_TEST_PATTERN
	uint32_t i, j;
	uint8_t y, cb, cr;
	uint8_t *buf;
	uint32_t width = TVDEC_ACTIVE_COL;
	uint32_t height;

#ifdef CAM_NO_DEINTERLEAVING
	height = TVDEC_ACTIVE_LINES;
#else
	height = TVDEC_ACTIVE_LINES / 2;
#endif
	fb_addr = memalign(SGA_BUFFER_ALIGNMENT,
			   width * height * TVDEC_BYTES_PER_PIXEL);
	if (!fb_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto buffers_del;
	}
	g_cam_context.testbuf_odd_addr = (uint32_t)fb_addr;

	fb_addr = memalign(SGA_BUFFER_ALIGNMENT,
			   width * height * TVDEC_BYTES_PER_PIXEL);
	if (!fb_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto buffers_del;
	}
	g_cam_context.testbuf_even_addr = (uint32_t)fb_addr;

	/* ODD */
	buf = (uint8_t *)g_cam_context.testbuf_odd_addr;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			y = (j * 219) / height;
			y = y + 16;
			cb = 128; cr = 128;

			if ((i == 0) || (i == 1)) { /* blue */
				y = 41; cb = 240; cr = 110;
			}
			if (j == 0) { /* yellow */
				y = 210; cb = 16; cr = 146;
			}
			if ((i == width - 1) || (i == width - 2)) { /* green */
				y = 145; cb = 54; cr = 34;
			}
			if (j == height - 1) { /* red */
				y = 82; cb = 90; cr = 240;
			}

			if ((i % 2) == 0)
				*buf++ = cb;
			else
				*buf++ = cr;
			*buf++ = y;
		}
	}

	/* EVEN */
	buf = (uint8_t *)g_cam_context.testbuf_even_addr;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			y = (i * 219) / width;
			y = y + 16;
			cb = 128; cr = 128;
			if ((i == 0) || (i == 1)) { /* blue */
				y = 41; cb = 240; cr = 110;
			}
			if (j == 0) { /* yellow */
				y = 210; cb = 16; cr = 146;
			}
			if ((i == width - 1) || (i == width - 2)) { /* green */
				y = 145; cb = 54; cr = 34;
			}
			if (j == height - 1) { /* red */
				y = 82; cb = 90; cr = 240;
			}

			if ((i % 2) == 0)
				*buf++ = cb;
			else
				*buf++ = cr;
			*buf++ = y;
		}
	}
#endif

#ifdef CAM_OVERLAY
	text_addr = memalign(SGA_BUFFER_ALIGNMENT,
			     output->w * output->h * SCREEN_BPP);
	if (!text_addr) {
		TRACE_ERR("%s: No memory\n", __func__);
		goto buffers_del;
	}
	g_cam_context.text0_addr = (uint32_t)text_addr;
#endif

	g_cam_context.out.x	= output->x;
	g_cam_context.out.y	= output->y;
	g_cam_context.out.w	= output->w;
	g_cam_context.out.h	= output->h;
#ifdef CAM_SCALING
	TRACE_INFO("FRAME Addr : 0x%08x-{%d}\n", sensor_addr, SENSOR_SIZE);
#endif
#if defined PP_USAGE
	TRACE_INFO("FB0 Addr   : 0x%08x-{%d}\n",
		   g_cam_context.fb_addr[0],
		   SENSOR_SIZE);
	TRACE_INFO("FB1 Addr   : 0x%08x-{%d}\n",
		   g_cam_context.fb_addr[1],
		   SENSOR_SIZE);

	TRACE_INFO("OUT0 Addr   : 0x%08x-{%d}\n",
		   g_cam_context.out_addr[0],
		   output->w * output->h * SCREEN_BPP);
	TRACE_INFO("OUT1 Addr   : 0x%08x-{%d}\n",
		   g_cam_context.out_addr[1],
		   output->w * output->h * SCREEN_BPP);
#else
	TRACE_INFO("FB0 Addr   : 0x%08x-{%d}\n",
		   g_cam_context.fb_addr[0],
		   output->w * output->h * SCREEN_BPP);
	TRACE_INFO("FB1 Addr   : 0x%08x-{%d}\n",
		   g_cam_context.fb_addr[1],
		   output->w * output->h * SCREEN_BPP);
#endif
#ifdef CAM_OVERLAY
	TRACE_INFO("TEXT Addr   : 0x%08x-{%d}\n",
		   g_cam_context.text0_addr,
		   output->w * output->h * SCREEN_BPP);
#endif

	build_sga_warmup_batch();
	build_sga_main_batch(output);

	ret = cam_create_semaphores();

	if (ret) {
		TRACE_ERR("%s : Cannot create semaphores\n", __func__);
		goto buffers_del;
	}

	g_cam_context.cam_queue = xQueueCreate(10, sizeof(struct buffer_elem));

	if (!g_cam_context.cam_queue) {
		TRACE_ERR("%s : Cannot create cam queue\n", __func__);
		goto cam_queue_del;
	}

	g_cam_context.mx_buffer_queue =
		xQueueCreate(MX_QUEUE_MAX_ELEMS, sizeof(struct buffer_elem));

	if (!g_cam_context.mx_buffer_queue) {
		TRACE_ERR("%s : Cannot create mx_buffer_queue\n", __func__);
		goto cam_queue_del;
	}

	g_cam_context.ax_buffer_queue =
		xQueueCreate(AX_QUEUE_MAX_ELEMS, sizeof(struct buffer_elem));

	if (!g_cam_context.ax_buffer_queue) {
		TRACE_ERR("%s : Cannot create ax_buffer_queue\n", __func__);
		goto cam_queue_del;
	}

	g_cam_context.state = CAM_INIT;

	ret = xTaskCreate((pdTASK_CODE) cam_main_task,
			  (char *)"CAM",
			  212,
			  (void *)NULL,
			  TASK_PRIO_TEST_RVC,
			  &g_cam_main_task_handle);

	if (ret != pdPASS) {
		TRACE_ERR("Cannot create cam task\n");
		g_cam_context.state = CAM_HW_INIT;
		goto cam_queue_del;
	}

	xSemaphoreGive(g_mutex_synchro);
	return 0;

/* !!!!! TODO: simply call cam_deinit_rear_camera */

cam_queue_del:
	if (g_cam_context.cam_queue)
		vQueueDelete(g_cam_context.cam_queue);
	if (g_cam_context.mx_buffer_queue)
		vQueueDelete(g_cam_context.mx_buffer_queue);
	if (g_cam_context.ax_buffer_queue)
		vQueueDelete(g_cam_context.ax_buffer_queue);
	g_cam_context.cam_queue = NULL;
	g_cam_context.mx_buffer_queue = NULL;
	g_cam_context.mx_buffer_queue = NULL;
	cam_delete_semaphores();
buffers_del:
#ifdef CAM_OVERLAY
	if (g_cam_context.text0_addr) {
		free((uint32_t *)g_cam_context.text0_addr);
		g_cam_context.text0_addr = 0;
	}
#endif
	if (g_cam_context.fb_addr[1]) {
		free((uint32_t *)g_cam_context.fb_addr[1]);
		g_cam_context.fb_addr[1] = 0;
	}

	if (g_cam_context.fb_addr[0]) {
		free((uint32_t *)g_cam_context.fb_addr[0]);
		g_cam_context.fb_addr[0] = 0;
	}
#if defined PP_USAGE
	if (g_cam_context.out_convert) {
		free((uint32_t *)g_cam_context.out_convert);
		g_cam_context.out_convert = 0;
	}
	if (g_cam_context.out_addr[0]) {
		free((uint32_t *)g_cam_context.out_addr[0]);
		g_cam_context.out_addr[0] = 0;
	}
	if (g_cam_context.out_addr[1]) {
		free((uint32_t *)g_cam_context.out_addr[1]);
		g_cam_context.out_addr[1] = 0;
	}
#endif
sensor_del:
	free((uint32_t *)g_cam_context.sensor_addr);
	g_cam_context.sensor_addr = 0;
sge_deinit:
	sge_deinit();
vip_deinit:
	vip_deinit();
cam_init_failure:
	xSemaphoreGive(g_mutex_synchro);
	return -1;
}

/**
 * @brief De-initialize SGA Processing
 *
 * @param  VOID
 * @return			SGA error condition
 */
int8_t cam_deinit_rear_camera(void)
{
	if (g_cam_context.state < CAM_HW_INIT) {
		TRACE_ERR("Cannot deinit: state=%d\n", g_cam_context.state);
		return -1;
	}

	xSemaphoreTake(g_mutex_synchro, portMAX_DELAY);

	if ((g_cam_context.state == CAM_RUNNING) ||
	    (g_cam_context.state == CAM_PAUSED)) {
		if (cam_stop_rear_camera()) {
			TRACE_ERR("%s : Can't STOP camera\n", __func__);
			xSemaphoreGive(g_mutex_synchro);
			return -1;
		}
	}

	if (g_cam_context.state > CAM_INIT) {
		TRACE_ERR("Cannot deinit: state=%d\n", g_cam_context.state);
		xSemaphoreGive(g_mutex_synchro);
		return -1;
	}

	if (g_cam_main_task_handle)
		vTaskDelete(g_cam_main_task_handle);

	g_cam_main_task_handle = NULL;

	if (g_cam_context.cam_queue)
		vQueueDelete(g_cam_context.cam_queue);
	if (g_cam_context.mx_buffer_queue)
		vQueueDelete(g_cam_context.mx_buffer_queue);
	if (g_cam_context.ax_buffer_queue)
		vQueueDelete(g_cam_context.ax_buffer_queue);
	g_cam_context.cam_queue = NULL;
	g_cam_context.mx_buffer_queue = NULL;
	g_cam_context.mx_buffer_queue = NULL;

	cam_delete_semaphores();

	sga_release_batch(g_cam_context.init_batch_id);
	sga_release_batch(g_cam_context.fb_batch_id);

#ifdef CAM_OVERLAY
	if (g_cam_context.text0_addr) {
		free((uint32_t *)g_cam_context.text0_addr);
		g_cam_context.text0_addr = 0;
	}
#endif

#if defined PP_USAGE
	if (g_cam_context.g1_ctx) {
		TRACE_INFO("CAM TASK ====> FREEING G1 RESOURCES\n");
		G1ResFreeResource(g_cam_context.g1_ctx);
	}
	g_cam_context.g1_ctx = NULL;

	if (g_cam_context.out_convert) {
		free((uint32_t *)g_cam_context.out_convert);
		g_cam_context.out_convert = 0;
	}
	if (g_cam_context.out_addr[0]) {
		free((uint32_t *)g_cam_context.out_addr[0]);
		g_cam_context.out_addr[0] = 0;
	}
	if (g_cam_context.out_addr[1]) {
		free((uint32_t *)g_cam_context.out_addr[1]);
		g_cam_context.out_addr[1] = 0;
	}
#endif
	if (g_cam_context.fb_addr[0]) {
		free((void *)g_cam_context.fb_addr[0]);
		g_cam_context.fb_addr[0] = 0;
	}

	if (g_cam_context.fb_addr[1]) {
		free((void *)g_cam_context.fb_addr[1]);
		g_cam_context.fb_addr[1] = 0;
	}

	if (g_cam_context.sensor_addr) {
		free((void *)g_cam_context.sensor_addr);
		g_cam_context.sensor_addr = 0;
	}

	g_cam_context.vip_running = false;
	g_cam_context.sga_running = false;
	vip_register_cb(NULL);

	g_cam_context.state = CAM_UNINITIALIZED;

	/* Workaround */
	/* tv_dec_release(); */

	memset(&g_cam_context, 0, sizeof(struct cam_context));
	g_cam_context.state = CAM_HW_INIT;

	sge_deinit();
	vip_deinit();

	xSemaphoreGive(g_mutex_synchro);
	return 0;
}

/**
 * @brief Start SGA Processing
 *
 * @param  VOID
 * @return			SGA error condition
 */
int8_t cam_start_rear_camera(void)
{
	uint32_t ret = 0;

	TRACE_FUNC_IN();

	if (g_cam_context.state < CAM_INIT) {
		TRACE_ERR("%s : Camera Framework not initialized\n", __func__);
		TRACE_FUNC_OUT_RET(-1);
		return -1;
	}

	xSemaphoreTake(g_mutex_synchro, portMAX_DELAY);

	if (g_cam_context.state > CAM_INIT) {
		/* Do noting just gently return */
		TRACE_ERR("[already running]\n");
		TRACE_FUNC_OUT_RET(0);
		xSemaphoreGive(g_mutex_synchro);
		return 0;
	}
	COMPLETE(g_cam_sem[CAM_START]);

	ret = WAIT_FOR_COMPLETION_TIMEOUT(g_cam_sem[CAM_START_ACK],
					  START_CAM_DELAY_MS);
	if (ret == pdFALSE) {
		TRACE_ERR("Cannot start camera\n");
		TRACE_FUNC_OUT_RET(-1);
		xSemaphoreGive(g_mutex_synchro);
		return -1;
	}

	TRACE_FUNC_OUT_RET(0);
	xSemaphoreGive(g_mutex_synchro);
	return 0;
}

/**
 * @brief Stop SGA Processing
 *
 * @param  VOID
 * @return			SGA error condition
 */
int8_t cam_stop_rear_camera(void)
{
	TRACE_FUNC_IN();

	if (g_cam_context.state <= CAM_HW_INIT) {
		/*  Nothing to do. Return gently */
		TRACE_FUNC_OUT_RET(0);
		return 0;
	}

	xSemaphoreTake(g_mutex_synchro, portMAX_DELAY);

	if ((g_cam_context.state == CAM_RUNNING) ||
	    (g_cam_context.state == CAM_PAUSED))
		g_cam_context.state = CAM_RUNNING_TO_INIT;

	WAIT_FOR_COMPLETION_TIMEOUT(g_cam_sem[CAM_STOP], STOP_CAM_DELAY_MS);

#ifdef PP_USAGE
	if (g_cam_context.pp) {
		g1_pp_close(g_cam_context.pp);
		g_cam_context.pp = NULL;
	}
	if (g_cam_context.pp_convert) {
		g1_pp_close(g_cam_context.pp_convert);
		g_cam_context.pp_convert = NULL;
	}
#endif
	TRACE_FUNC_OUT_RET(0);
	xSemaphoreGive(g_mutex_synchro);
	return 0;
}

/**
 * @brief Handover with Ax/Linux
 *
 * @param  VOID
 * @return			SGA error condition
 */
void cam_start_streaming_ax(void)
{
	TRACE_INFO("STA_CAM: cam_start_streaming_ax\n");

	set_bit(g_cam_context.handover, BIT(0));

	g_cam_context.nb_buffers_returned = 0;

	if (g_cam_context.state == CAM_PAUSED)
		g_cam_context.state = CAM_RUNNING;
}

/**
 * @brief Handover with Ax/Linux
 *
 * @param  VOID
 * @return			SGA error condition
 */
void cam_stop_streaming_ax(void)
{
	uint32_t handover = g_cam_context.handover;

	TRACE_INFO("STA_CAM: cam_stop_streaming_ax\n");

	if (g_cam_context.state == CAM_RUNNING)
		g_cam_context.state = CAM_PAUSED;

	xQueueReset(g_cam_context.ax_buffer_queue);

	clr_bit(g_cam_context.handover, BIT(0));

	if (read_bit(handover, BIT(0)))
		COMPLETE(g_cam_sem[CAM_STOP_STREAMING_SYNCHRO]);
}

/**
 * @brief  Register notification for a particular event
 *
 * @param  state	(IN)	State on which event will fire notification
 * @param  notif	(IN)	Notification function pointer
 * @return			SGA error condition
 */
int8_t cam_register_notif(
		enum cam_event event,
		void (*notif)(void *))
{
	if (event >= MAX_CAM_EVENTS) {
		TRACE_ERR("%s: Bad Camera event\n", __func__);
		return -1;
	}

	g_cam_context.notif[event] = notif;

	return 0;
}

#ifdef CAM_OVERLAY
/**
 * @brief Get Overlay Buffer for SGA
 *
 * @param  VOID
 * @return			SGA error condition
 */
int8_t cam_get_overlay_buffer(uint32_t *overlay)
{
	if (!overlay) {
		TRACE_ERR("%s : overlay parameter is NULL\n", __func__);
		return -1;
	}

	*overlay = g_cam_context.text0_addr;
	return 0;
}
#endif

/**
 * @brief Start pending batch
 *
 * @param batch_id (IN) Batch id
 */
static void cam_start_batch(uint8_t batch_id)
{
	uint32_t fb_addr;
	uint32_t *instr_ptr = NULL;
	BaseType_t ret = 0;
	struct buffer_elem *buf = &g_cam_context.current_buf_elem;

	if (g_cam_context.sga_running)
		return;

	if (read_bit(g_cam_context.handover, BIT(0))) {
		/* start streaming received */
		ret = xQueueReceiveFromISR(g_cam_context.ax_buffer_queue,
					   buf,
					   NULL);
	} else {
		/* start streaming not received */
		ret = xQueueReceiveFromISR(g_cam_context.mx_buffer_queue,
					   buf,
					   NULL);
	}

	if (ret == pdTRUE) {
		fb_addr = buf->phys_addr;
		sga_set_priv_data(g_cam_context.ctx_batch,
				  batch_id,
				  buf->priv);
	} else {
		TRACE_ERR("no buffer in queue - frame skipped\n");
		return;
	}

	sga_get_fb_instr_ptr_addr(g_cam_context.ctx_batch,
				  batch_id,
				  &instr_ptr);

	*instr_ptr++ = OUT_BASE_ADD_MSB | ((fb_addr & 0xFF000000) >> 24);
	*instr_ptr++ = OUT_BASE_ADD     | (fb_addr & 0x00FFFFFF);

	sga_set_surface(g_cam_context.ctx_batch,
			batch_id,
			fb_addr);

	g_cam_context.sga_running = true;
	sge_do_start_batch(g_cam_context.ctx_batch,
			   batch_id);

	TRACE_INFO_RUNNING("[%s] phy: 0x%08X - priv:0x%08X, type:%d\n",
			   __func__,
			   buf->phys_addr, buf->priv, buf->buffer_type);
}

/**
 * @brief  Signals SGA warm-up interrupt has been received
 *
 * @param  VOID
 */
void cb_init_handler(void)
{
	TRACE_INFO("Got Warm-up IT\n");

	COMPLETE_FROM_ISR(g_cam_sem[CAM_WARMUP]);
}

/**
 * @brief  Signals SGA frame interrupt has been received
 *
 * @param  VOID
 */
void cb_frame_handler(void)
{
	uint8_t batch_id;
	struct buffer_elem elem = g_cam_context.current_buf_elem;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
#ifdef DEBUG
	uint32_t nb_lines = 0;
	uint8_t *ptr = (uint8_t *)elem.phys_addr;
#endif

#ifdef DEBUG_RVC_PERF
	cam_perf[frame_id].sga_it = mtu_get_time(0) -
		cam_perf[frame_id].vip_start_odd;
#endif

	sga_get_curr_batch(g_cam_context.ctx_batch, &batch_id);
	sge_do_stop_batch(g_cam_context.ctx_batch, batch_id);
	g_cam_context.sga_running = false;

	if ((g_cam_context.state == CAM_RUNNING) &&
	    (g_cam_context.current_vip_event == VIP_FRAME_END_EVEN)) {
		cam_start_batch(g_cam_context.fb_batch_id);
	}

#ifdef DEBUG
	while (nb_lines < TVDEC_ACTIVE_LINES) {
		if (ptr[nb_lines * TVDEC_ACTIVE_COL * TVDEC_BYTES_PER_PIXEL] ==
		    0x0)
			break;
		nb_lines++;
	}
	TRACE_ERR("nb_lines:%d\n", nb_lines);
#endif

	if (xQueueSendToBackFromISR(g_cam_context.cam_queue, &elem,
				    &xHigherPriorityTaskWoken))
		return;

	if (xHigherPriorityTaskWoken)
		taskYIELD();
}

#ifdef DEBUG_RVC_PERF
static void cam_print_perf(void)
{
	uint32_t i = 0;

	for (i = 0; i < CAM_PERF_NB_FRAMES; i++) {
		trace_printf("[%-2d] %-8d %-8d %-8d %-8d %-8d\n",
			     i,
			     cam_perf[i].vip_start_odd,
			     cam_perf[i].vip_end_odd,
			     cam_perf[i].vip_start_even,
			     cam_perf[i].vip_end_even,
			     cam_perf[i].sga_it);
	}
}
#endif

#ifdef DEBUG_RVC_STATS
static void cam_print_stats(void)
{
	uint32_t avg_fps = 0;

	trace_printf("\n-[performance]\n");
	trace_printf(" |- nb_buf_done: %d\n", cam_stat.nb_buf_done);

	if (cam_stat.end_ms) {
		trace_printf(" |- min_duration (us): %d\n",
			     cam_stat.min_duration);
		trace_printf(" |- max_duration (us): %d\n",
			     cam_stat.max_duration);

		cam_stat.duration = (cam_stat.end_ms - cam_stat.start_ms) /
				   1000;

		trace_printf(" |- duration (ms): %d\n",
			     cam_stat.duration);

		if (cam_stat.duration)
			avg_fps = ((cam_stat.nb_buf_done - 1) * 10000) /
				  cam_stat.duration;

		trace_printf(" |- avg_fps (0.1Hz): %d\n", avg_fps);
	}
#if defined PP_USAGE
	{
		uint32_t avg_duration, avg_convert;

		if (!cam_stat.nb_buf_done)
			return;

		avg_duration = cam_stat.ts_pp_duration / cam_stat.nb_buf_done;
		trace_printf(" |- pp avg duration: %dus\n", avg_duration);
	}
#endif
}
#endif

/**
 * @brief  Signals VIP frame interrupt has been received
 *         Execution context: VIP ISR
 *
 * @param  ev		(IN)	VIP frame event ("start/end" and "even/odd")
 */
void notif_vip_handler(t_vip_events ev)
{
#ifdef DEBUG_RVC_PERF
	switch (ev) {
	case VIP_FRAME_START_ODD:
		cam_perf[frame_id].vip_start_odd = mtu_get_time(0);
		break;
	case VIP_FRAME_END_ODD:
		cam_perf[frame_id].vip_end_odd = mtu_get_time(0) -
			cam_perf[frame_id].vip_start_odd;
		break;
	case VIP_FRAME_START_EVEN:
		cam_perf[frame_id].vip_start_even = mtu_get_time(0) -
			cam_perf[frame_id].vip_start_odd;
		break;
	case VIP_FRAME_END_EVEN:
		cam_perf[frame_id].vip_end_even = mtu_get_time(0) -
			cam_perf[frame_id].vip_start_odd;
		frame_id++;
		break;
	default:
		break;
	}

	if (frame_id >= CAM_PERF_NB_FRAMES) {
		cam_print_perf();
		frame_id = 0;
	}
#endif

	g_cam_context.current_vip_event = ev;

	if ((g_cam_context.state != CAM_RUNNING) ||
	    (!g_cam_context.vip_running)) {
		return;
	}

	switch (ev) {
	case VIP_FRAME_END_ODD:
		sge_clear_tst_register_bit(SGA_ODD_TST_BIT);
		sge_set_tst_register_bit(SGA_EVEN_TST_BIT);
		break;
	case VIP_FRAME_END_EVEN:
		sge_clear_tst_register_bit(SGA_EVEN_TST_BIT);
		sge_set_tst_register_bit(SGA_ODD_TST_BIT);

		if (!g_cam_context.sga_running)
			cam_start_batch(g_cam_context.fb_batch_id);

		break;
	default:
		break;
	}
}

/**
 * @brief Buffer filled: display it or notify Ax RVC driver
 *
 * @param buffer (IN) filled buffer structure
 * @param fb_ptr (IN) framebuffer pointer structure
 */
static void cam_buffer_done(struct buffer_elem buffer,
			    struct framebuffer *fb_ptr)
{
	int32_t ret = 0;
#if defined PP_USAGE
	struct user_ppconfig ppcfg;
	u32 *buffer_addr = 0;
#if defined DEBUG_RVC_STATS
	uint32_t pp_ts_start = 0;
	uint32_t pp_ts_end = 0;
#endif
#endif
	TRACE_INFO_RUNNING("[%s] addr: 0x%08X priv: 0x%08X type:%d\n",
			   __func__,
			   buffer.phys_addr, buffer.priv, buffer.buffer_type);

	if (g_cam_context.state != CAM_RUNNING)
		return;

	switch (buffer.buffer_type) {
	case CAM_BUFFER_Ax:
		if (read_bit(g_cam_context.handover, BIT(0))) {
			/* Start streaming received - Send buffer back to Ax */
			if (g_cam_context.notif[BUFFER_FILLED_EVENT])
				(*g_cam_context.notif[BUFFER_FILLED_EVENT])
					(&buffer.priv);

			set_bit(g_cam_context.handover, BIT(2));
		} else { /* Start streaming not yet received: requeue buffer */
			cam_new_buffer(buffer.phys_addr,
				       buffer.priv,
				       CAM_BUFFER_Ax);
		}
		break;
	case CAM_BUFFER_Mx:
		/* Requeue Mx buffer */
		cam_new_buffer(buffer.phys_addr, 0, CAM_BUFFER_Mx);
		break;
	default:
		TRACE_ERR("Unknown buffer type: %d\n", buffer.buffer_type);
	}
#if defined PP_USAGE
#if defined DEBUG_RVC_STATS
	pp_ts_start = mtu_get_time(0);
#endif
	if (!g_cam_context.pp)
		g_cam_context.pp = g1_pp_open(g_cam_context.g1_ctx);

	memset(&ppcfg, 0, sizeof(struct user_ppconfig));
	ppcfg.input_fmt = fb_ptr->pixel_format;
	ppcfg.input_wdt = fb_ptr->width;
	ppcfg.input_hgt = fb_ptr->height;
	ppcfg.input_paddr = buffer.phys_addr;
	ppcfg.deinterlace_needed = false;

	if (fb_ptr->pixel_format != PF_FORMAT_NV12) {
		if (!g_cam_context.pp_convert)
			g_cam_context.pp_convert =
				g1_pp_open(g_cam_context.g1_ctx);

		// PP is able to deinterlace YUV420 format ONLY
		// so we need to first convert data to YUV420 then scale
		// and deinterlace
		ppcfg.output_fmt = PF_FORMAT_NV12;
		ppcfg.output_wdt = fb_ptr->width;
		ppcfg.output_hgt = fb_ptr->height;
		ppcfg.output_paddr = g_cam_context.out_convert;
		ret = g1_pp_set_config(g_cam_context.pp_convert, &ppcfg);
		if (ret < 0) {
			TRACE_ERR("Unable to convert frame\n");
			goto finalize;
		}
		ret = g1_pp_get_frame(g_cam_context.pp_convert, &buffer_addr);
		if (ret < 0) {
			TRACE_ERR("Unable to retrieve converted frame\n");
			goto finalize;
		}

		memset(&ppcfg, 0, sizeof(struct user_ppconfig));
		ppcfg.input_fmt = PF_FORMAT_NV12;
		ppcfg.input_wdt = fb_ptr->width;
		ppcfg.input_hgt = fb_ptr->height;
		ppcfg.input_paddr = g_cam_context.out_convert;
		ppcfg.deinterlace_needed = true;
	}
	ppcfg.output_fmt = fb_ptr->pixel_format;
	ppcfg.output_wdt = g_cam_context.out.w;
	ppcfg.output_hgt = g_cam_context.out.h;
	g_cam_context.idxout = (g_cam_context.idxout + 1) % 2;
	ppcfg.output_paddr = g_cam_context.out_addr[g_cam_context.idxout];
	ret = g1_pp_set_config(g_cam_context.pp, &ppcfg);
	if (ret < 0) {
		TRACE_ERR("Unable to rescale frame\n");
		goto finalize;
	}

	ret = g1_pp_get_frame(g_cam_context.pp, &buffer_addr);
	if (ret < 0) {
		TRACE_ERR("Unable to retrieve rescaled frame\n");
		goto finalize;
	}
#if defined DEBUG_RVC_STATS
	pp_ts_end = mtu_get_time(0);
#endif

	if (g_cam_context.ltdc_layer_enabled) {
		struct framebuffer fb;

		memcpy(&fb, fb_ptr, sizeof(struct framebuffer));
		fb.paddr = (uint32_t)buffer_addr;
		fb.width = g_cam_context.out.w;
		fb.height = g_cam_context.out.h;
		// !!!!! fb.offsets
		fb.pitches[0] = SCREEN_BPP * g_cam_context.out.w;
		ret = ltdc_layer_update(&fb, g_cam_context.layer_id,
					g_cam_context.out.x,
					g_cam_context.out.y,
					EVEN);
#ifdef DEBUG_RVC_STATS
		if (first) {
			TRACE_ERR("Time To First RVC\n");
			first = false;
		}
#endif
	}
finalize:
	if (ret)
		TRACE_ERR("ltdc_layer_update FAILED\n");

	if (g_cam_context.notif[FRAME_EVENT])
		(*g_cam_context.notif[FRAME_EVENT])(NULL);

#else
	if (g_cam_context.ltdc_layer_enabled) {
		fb_ptr->paddr = buffer.phys_addr;
		ret = ltdc_layer_update(fb_ptr, g_cam_context.layer_id,
					g_cam_context.out.x,
					g_cam_context.out.y,
					EVEN);

#ifdef DEBUG_RVC_STATS
		if (first) {
			TRACE_ERR("Time To First RVC\n");
			first = false;
		}
#endif
	}
	if (ret)
		TRACE_ERR("ltdc_layer_update FAILED\n");

	if (g_cam_context.notif[FRAME_EVENT])
		(*g_cam_context.notif[FRAME_EVENT])(NULL);
#endif

#ifdef DEBUG_RVC_STATS
	cam_stat.nb_buf_done++;
	cam_stat.timestamp = mtu_get_time(0);
	switch (cam_stat.nb_buf_done) {
	case 1:
		cam_stat.start_ms = cam_stat.timestamp;
		break;
	case 2:
		cam_stat.duration = cam_stat.timestamp - cam_stat.end_ms;
		cam_stat.min_duration = cam_stat.duration;
		cam_stat.max_duration = cam_stat.duration;
		break;
	default:
		cam_stat.duration = cam_stat.timestamp - cam_stat.end_ms;
		cam_stat.min_duration = MIN(cam_stat.duration,
					    cam_stat.min_duration);
		cam_stat.max_duration = MAX(cam_stat.duration,
					    cam_stat.max_duration);
		break;
	}
	cam_stat.end_ms = cam_stat.timestamp;

#if defined PP_USAGE
	cam_stat.ts_pp_duration += ((pp_ts_end - pp_ts_start));
#endif

	if ((cam_stat.nb_buf_done % 100) == 0)
		cam_print_stats();
#endif
}

/**
 * @brief Camera main task
 *
 * @param  VOID
 */
static void cam_main_task(void)
{
	struct framebuffer fb;
	uint32_t ret = 0;
	struct buffer_elem buffer;
	uint32_t lcd_w, lcd_h;

	fb.x			= 0;
	fb.y			= 0;
	fb.paddr		= 0;
	fb.offsets[0]		= 0;
	fb.pixel_format		= PF_FORMAT_UYVY;

	lcd_get_display_resolution(&lcd_w, &lcd_h);

	while (g_cam_context.state >= CAM_INIT) {
		/* Wait for camera to be started (cam_start_rear_camera) */
		WAIT_FOR_COMPLETION(g_cam_sem[CAM_START]);
		g_cam_context.state = CAM_INIT_TO_RUNNING;
#if defined PP_USAGE
		fb.width		= TVDEC_ACTIVE_COL;
		fb.height		= TVDEC_ACTIVE_LINES;
		fb.pitches[0]		= SCREEN_BPP * fb.width;
#else
		fb.width		= MIN(lcd_w, g_cam_context.out.w);
		fb.height		= MIN(lcd_h, g_cam_context.out.h);
		fb.pitches[0]		= SCREEN_BPP * g_cam_context.out.w;
#endif
		vip_start_capture();
		g_cam_context.vip_running = true;

		TRACE_INFO("Running Warm-up batch ...\n");
		sge_do_start_batch(g_cam_context.ctx_batch,
				   g_cam_context.init_batch_id);

		/* Wait for SGA warmup interrupt */
		ret = WAIT_FOR_COMPLETION_TIMEOUT(g_cam_sem[CAM_WARMUP],
						  WARMUP_DELAY_MS);
		if (ret == pdFALSE) {
			TRACE_ERR("No Warmup IT\n");
			sge_do_stop_batch(g_cam_context.ctx_batch,
					  g_cam_context.init_batch_id);

			vip_stop_capture();
			g_cam_context.vip_running = false;

			continue;
		}

		if (!g_cam_context.handover) {
			/* Book a display layer to display
			 * images outputed by SGA
			 */
			if (ltdc_layer_book_top_layer(PF_FORMAT_UYVY,
						      &g_cam_context.layer_id))
				continue;

			g_cam_context.ltdc_layer_enabled = true;
			TRACE_INFO("g_cam_context.layer_id: %d\n",
				   g_cam_context.layer_id);

			cam_new_buffer(g_cam_context.fb_addr[0],
				       0,
				       CAM_BUFFER_Mx);
			cam_new_buffer(g_cam_context.fb_addr[1],
				       0,
				       CAM_BUFFER_Mx);
		}

		/* Camera has started successfully:
		 * Unblocks the RVC control task (cam_start_rear_camera)
		 */
		COMPLETE(g_cam_sem[CAM_START_ACK]);

		/* Empty filled buffer Queue */
		xQueueReset(g_cam_context.cam_queue);

		g_cam_context.state = CAM_RUNNING;
		TRACE_INFO("CAM RUNNING\n");

		while ((g_cam_context.state == CAM_RUNNING) ||
		       (g_cam_context.state == CAM_PAUSED)) {
			/* Wait for SGA to complete batch execution
			 */
			ret = xQueueReceive(g_cam_context.cam_queue,
					    &buffer,
					    pdMS_TO_TICKS(FRAME_DELAY_MS));

			if (ret == pdFALSE) {
				TRACE_INFO("IT SGA not received on time\n");
				continue;
			}

			cam_buffer_done(buffer, &fb);
		}

		TRACE_INFO("CAM STOPPING\n");

		/* Wait for SGA to complete batch execution
		 * before being able to complete the stop camera request
		 */
		if (g_cam_context.sga_running) {
			uint8_t batch_id;

			sga_get_curr_batch(g_cam_context.ctx_batch, &batch_id);
			sge_do_stop_batch(g_cam_context.ctx_batch, batch_id);
			g_cam_context.sga_running = false;
		}

		/* Masks VIP interrupts */
		vip_stop_capture();
		g_cam_context.vip_running = false;

		if (read_bit(g_cam_context.handover, BIT(0))) {
			/* Wait for Ax stop streaming event */
			WAIT_FOR_COMPLETION_TIMEOUT(
				g_cam_sem[CAM_STOP_STREAMING_SYNCHRO],
				STOP_STREAMING_SYNCHRO_DELAY_MS);
		}

		if (g_cam_context.ltdc_layer_enabled) {
			g_cam_context.ltdc_layer_enabled = false;
			ltdc_layer_release_top_layer(g_cam_context.layer_id);
		}

		/* Reset filled buffer Queue */
		xQueueReset(g_cam_context.cam_queue);
		/* Reset empty buffer Queues */
		xQueueReset(g_cam_context.mx_buffer_queue);
		xQueueReset(g_cam_context.ax_buffer_queue);
		clr_bit(g_cam_context.handover, BIT(0));
/*
 *		clr_bit(g_cam_context.handover, BIT(1));
 *		clr_bit(g_cam_context.handover, BIT(2));
 */
		g_cam_context.state = CAM_INIT;
		g_cam_context.nb_buffers_returned = 0;

		TRACE_INFO("CAM STOPPED\n");

		COMPLETE(g_cam_sem[CAM_STOP]);
	}

	vTaskDelete(NULL);
}
