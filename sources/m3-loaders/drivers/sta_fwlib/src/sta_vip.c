/**
 * @file sta_vip.c
 * @brief This file provides all the VIP firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#include "sta_vip.h"
#include "sta_cam.h"
#include "sta_tvdec.h"
#include "utils.h"
#include "trace.h"
#include "sta_nvic.h"
#include "sta_pinmux.h"

struct sta_vip sta_vip_ctx;

#ifdef DEBUG
#include "sta_mtu.h"

unsigned int log_idx_frm;
struct log_struct log_frm[LOG_DEPTH];

void vip_line(void)
{
	sta_vip_ctx.line_cnt++;
	*(uint32_t *)(VIP_BASE + VIP_STATUS) = IRQ_STATUS_LINE_END;
}
#endif

void vip_frame(void)
{
	unsigned long intstatus = *(uint32_t *)(VIP_BASE + VIP_STATUS);
	unsigned long vip_mask;

	t_vip_events ev = VIP_FRAME_MAX_EVENTS;

	/* There is a bug on VIP hardware that makes frame interruptions to be
	 * raised at each line instead of each frame.
	 * In order to avoid having too many interrupts, the following
	 * workaround has been done:
	 *   - check that FRAME_START is raised on active frame (TFR flag set)
	 *   - check that FRAME_END is raised on blanking (TFR flag unset)
	 *   - masking the "start frame" interrupt until "end frame" is raised
	 *   - masking the "end frame" interrupt until "start frame" is raised
	 */

	intstatus &=
		(IRQ_STATUS_FRAME_START_RAW | IRQ_STATUS_FRAME_START
		 | IRQ_STATUS_FRAME_END_RAW | IRQ_STATUS_FRAME_END
		 | IRQ_STATUS_FRAME_TYPE
		 | IRQ_STATUS_FRAME_TFR);

	if (((intstatus & IRQ_STATUS_FRAME_START) &&
	     !(intstatus & IRQ_STATUS_FRAME_TFR)) ||
	    ((intstatus & IRQ_STATUS_FRAME_END) &&
	      (intstatus & IRQ_STATUS_FRAME_TFR))) {
		*(uint32_t *)(VIP_BASE + VIP_STATUS) = intstatus;
		return;
	}

	vip_mask = *(uint32_t *)(VIP_BASE + VIP_MASK);

	if (intstatus & IRQ_STATUS_FRAME_START)
		vip_mask = (vip_mask & ~IRQ_FRAME_START) | IRQ_FRAME_END;
	else if (intstatus & IRQ_STATUS_FRAME_END)
		vip_mask = (vip_mask & ~IRQ_FRAME_END) | IRQ_FRAME_START;

	*(uint32_t *)(VIP_BASE + VIP_MASK) = vip_mask;

#ifdef DEBUG
	if (log_idx_frm < LOG_DEPTH) {
		log_frm[log_idx_frm].mtu_s =
				(unsigned int)mtu_get_timebase(TIME_IN_SEC);
		log_frm[log_idx_frm].mtu_u =
				(unsigned int)mtu_get_timebase(TIME_IN_US);
		log_frm[log_idx_frm++].intstatus =  intstatus;
	}
#endif

	if (intstatus & IRQ_STATUS_FRAME_START) {
		if ((intstatus & IRQ_STATUS_FRAME_TYPE) == IRQ_STATUS_FRAME_ODD)
			ev = VIP_FRAME_START_ODD;
		else
			ev = VIP_FRAME_START_EVEN;
	} else if ((intstatus & IRQ_STATUS_FRAME_END) == IRQ_STATUS_FRAME_END) {
		if ((intstatus & IRQ_STATUS_FRAME_TYPE) ==
		    IRQ_STATUS_FRAME_ODD) {
			ev = VIP_FRAME_END_ODD;
		} else {
			ev = VIP_FRAME_END_EVEN;
#ifdef DEBUG
			sta_vip_ctx.line_cnt = 0;
#endif
		}
	}

	*(uint32_t *)(VIP_BASE + VIP_STATUS) = intstatus;
	sta_vip_ctx.intstatus_prev = intstatus;

	if (sta_vip_ctx.notif)
		(*sta_vip_ctx.notif)(ev);
}

/**
 * @brief Video Input Port notification callback
 *
 *  @return		Error Condition (VIP_OK on success)
 */

t_vip_error vip_register_cb(void (*func)(t_vip_events ev))
{
	sta_vip_ctx.notif = func;
	return VIP_OK;
}

/**
 * @brief Video Input Port de-initialization routine
 *
 *  @return		VOID
 */
void vip_deinit(void)
{
	struct nvic_chnl nvic_deinit_struct;

	/* Disable SGA interrupt at nVIC level */
	nvic_deinit_struct.id		= EXT9_IRQChannel;
	nvic_deinit_struct.sub_prio	= 0;
	nvic_deinit_struct.preempt_prio	= 0;
	nvic_deinit_struct.enabled	= false;

	nvic_chnl_init(&nvic_deinit_struct);
#ifdef DEBUG
	nvic_deinit_struct.id		= EXT8_IRQChannel;
	nvic_chnl_init(&nvic_deinit_struct);
#endif
}

/**
 * @brief Video Input Port initialization routine
 *
 *  @return		Error Condition (VIP_OK on success)
 */
t_vip_error vip_init(void)
{
	struct nvic_chnl nvic_init_struct;

	pinmux_request("vip_mux");

	/* Setup VIP registers */
	*(uint32_t *)(VIP_BASE + VIP_CTRL) = 0x881138;
	*(uint32_t *)(VIP_BASE + VIP_CSTARTPR) = TVDEC_NBCOL - TVDEC_ACTIVE_COL;
	*(uint32_t *)(VIP_BASE + VIP_CSTOPPR) = 0xFFFF0000 + TVDEC_NBCOL +
						TVDEC_ACTIVE_COL;
	*(uint32_t *)(VIP_BASE + VIP_EFECR) = ITU656_EMBEDDED_EVEN_CODE;
	*(uint32_t *)(VIP_BASE + VIP_OFECR) = ITU656_EMBEDDED_ODD_CODE;

	sta_vip_ctx.intstatus_prev = 0;
	*(uint32_t *)(VIP_BASE + VIP_MASK) = IRQ_RESET;

	/* Enable SGA interrupt at nVIC level */
	nvic_init_struct.id		= EXT9_IRQChannel;
	nvic_init_struct.sub_prio	= 0;
	nvic_init_struct.preempt_prio	= IRQ_LOW_PRIO;
	nvic_init_struct.enabled	= true;
	nvic_chnl_init(&nvic_init_struct);

#ifdef DEBUG
	sta_vip_ctx.line_cnt = 0;
	log_idx_frm = 0;
	nvic_init_struct.id		= EXT8_IRQChannel;
	nvic_chnl_init(&nvic_init_struct);
#endif

	TRACE_INFO("VIP Initialized\n");

	return VIP_OK;
}

/**
 * @brief Video Input Port start capture by unmasking the interrupts
 *
 *  @return		VOID
 */
void vip_start_capture(void)
{
	TRACE_INFO("VIP Start capture\n");

	*(uint32_t *)(VIP_BASE + VIP_MASK) = IRQ_ENABLE;
}

/**
 * @brief Video Input Port start capture by masking the interrupts
 *
 *  @return		VOID
 */
void vip_stop_capture(void)
{
	TRACE_INFO("VIP Stop capture\n");

	*(uint32_t *)(VIP_BASE + VIP_MASK) = IRQ_RESET;
}

