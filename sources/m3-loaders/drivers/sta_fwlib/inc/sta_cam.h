/**
 * @file sta_cam.h
 * @brief Camera Framework
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#ifndef _CAM_H_
#define _CAM_H_

#include <stdlib.h>
#include "sta_type.h"
#include "sta_sga.h"
#include "sta_vip.h"
#include "queue.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

#define SCREEN_BPP		2
#define TRANSPARENT_COLOR	0x0FDF /*0xF87F */

/* CAM_PAL : camera with PAL standard
 * CAM_NTSC : camera with NTSC standard
 * CAM_AUTODETECT: autodetect standard (250ms delay needed for autodetection)
 */
#define CAM_AUTODETECT	0
#define CAM_PAL		1
#define CAM_NTSC	2

#define CAM_STANDARD CAM_AUTODETECT

//#define CAM_SCALING
//#define CAM_NO_DEINTERLEAVING
/*#define CAM_OVERLAY*/
#define PP_USAGE

#if defined(CAM_OVERLAY) && !defined(CAM_SCALING)
	#error "CAM_OVERLAY possible only with CAM_SCALING"
#endif

enum cam_state {
	CAM_UNINITIALIZED,
	CAM_HW_INIT,
	CAM_INIT,
	CAM_INIT_TO_RUNNING,
	CAM_RUNNING_TO_INIT,
	CAM_RUNNING,
	CAM_PAUSED,
	MAX_CAM_STATE
};

enum cam_event {
	INIT_EVENT,
	FRAME_EVENT,
	BUFFER_FILLED_EVENT,
	MAX_CAM_EVENTS
};

struct cam_rect {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};

enum cam_buffer_type {
	CAM_BUFFER_Mx,
	CAM_BUFFER_Ax
};

struct buffer_elem {
	uint32_t		phys_addr;
	uint32_t		priv;
	enum cam_buffer_type	buffer_type;
};

struct cam_context {
	void			*ctx_mem;
	void			*ctx_batch;
	struct cam_rect		out;
	uint32_t		sensor_addr;
	uint32_t		fb_addr[2];
#if defined PP_USAGE
	uint32_t		out_convert;
	uint32_t		out_addr[2];
	uint8_t			idxout;
#endif
#ifdef CAM_TEST_PATTERN
	uint32_t		testbuf_odd_addr;
	uint32_t		testbuf_even_addr;
#endif
#ifdef CAM_OVERLAY
	uint32_t		text0_addr;
#endif
	void *g1_ctx;
#ifdef PP_USAGE
	void *pp;
	void *pp_convert;
#endif
	uint32_t		color_key;
	uint8_t			fb_batch_id;
	struct buffer_elem	current_buf_elem;
	uint8_t			init_batch_id;
	enum cam_state		state;

	void			(*notif[MAX_CAM_EVENTS])(void *data);
	QueueHandle_t		cam_queue;
	QueueHandle_t		mx_buffer_queue;
	QueueHandle_t		ax_buffer_queue;
	bool			sga_running;
	bool			vip_running;
	/* handover:
	 * bit 0: start_streaming received
	 * bit 1: at least one buffer received from Ax processor
	 * bit 2: at least one buffer returned to Ax processor
	 */
	uint32_t		handover;
	uint32_t		nb_buffers_returned;
	bool			ltdc_layer_enabled;
	uint32_t		layer_id;
	t_vip_events		current_vip_event;
};

#ifdef DEBUG
struct log_struct {
	unsigned int mtu_s;
	unsigned int mtu_u;
	unsigned int intstatus;
	unsigned int mask;
};

#define LOG_DEPTH 2000
#endif

/**
 * @brief Push buffer in queue
 *
 * @param phys_addr		buffer physical address
 * @param priv			private info to be given back to Linux
 * @param cam_buffer_type	indicates if it is a Ax or Mx buffer
 */
void cam_new_buffer(uint32_t phys_addr,
		    uint32_t priv,
		    enum cam_buffer_type buffer_type);

/**
 * @brief Handover with Ax/Linux
 *
 * @param  VOID
 * @return			SGA error condition
 */
void cam_start_streaming_ax(void);

/**
 * @brief Handover with Ax/Linux
 *
 * @param  VOID
 * @return			SGA error condition
 */
void cam_stop_streaming_ax(void);

/**
 * @brief  Rear Camera Hardwareand global initialization
 *         This function has to be called once at the
 *         very first boot up to avoid conflicts onto IP whence
 *         apps processor is booting up
 *
 * @param  input		ADV7182 input selected
 * @param  std			ADV7182 detected standard
 * @return			SGA error condition
 */
int8_t cam_hw_init(uint32_t input, uint32_t *std);

/*
 * Parameters:
 *    Output: position and dimension of camera frame output
 * Description :
 *    Initialize SGE, build batches and raise an interrupt at the end of warm-up
 */
int8_t cam_init_rear_camera(
		struct cam_rect *output,
		bool managed_by_ax
		);

/*
 * Parameters:
 *    None
 * Description:
 *   Stop any running batches, cleanup contexts, cleanup SGE/SGA
 */
int8_t cam_deinit_rear_camera(void);

/*
 * Parameters:
 *    g1_ctx : pointer on G1 context
 * Description:
 *    Start Camera batches only if warm-up has been done
 */
int8_t cam_start_rear_camera(void);

/*
 * Parameters:
 *    None
 * Description:
 *    Merely stop Camera. Start can be called afterwards
 */
int8_t cam_stop_rear_camera(void);

/*
 * Parameters:
 *    event : INIT_EVENT, FRAME_EVENT
 *    notif : function pointer void notif(void)
 * Description:
 *    Register a callback on specified event.
 */
int8_t cam_register_notif(enum cam_event event,
			  void (*notif)(void *));

#ifdef CAM_OVERLAY
int8_t cam_get_overlay_buffer(uint32_t *overlay);
#endif

void notif_vip_handler(
		       t_vip_events ev);

#endif /* _CAM_H_ */
