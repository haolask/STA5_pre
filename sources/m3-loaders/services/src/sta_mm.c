/**
 * @file sta_mm.c
 * @brief MM use-cases handling
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "trace.h"

#include "utils.h"

#include "sta_common.h"
#include "sta_image.h"
#include "FreeRTOS.h"
#include "sta_type.h"
#include "sta_mtu.h"
#include "jpegdecapi.h"
#include "g1-res.h"
#include "dwl.h"
#include "sta_ltdc.h"
#include "sta_lcd.h"
#include "task.h"
#include "sta_rpmsg_mm.h"
#include "sta_mm.h"

void rvc_task(void);
void rpmsg_mm_task(void *p);
void splash_animation_task(void *p);
int rvc_start(void);
int rvc_stop(void);
void rvc_ax_takeover(void);
int splash_start_anim(bool start_allowed);
int splash_stop_anim(bool stopTask);
int splash_end_task(void);
int splash_screen_init(void);
int splash_screen_update(void *shadow_addr);

char *event2str[] = {
	"MM_EVT_RVC_RGE",
	"MM_EVT_RVC_RGD",
	"MM_EVT_RVC_STARTED",
	"MM_EVT_RVC_STOPPED",
	"MM_EVT_ANIM_START",
	"MM_EVT_ANIM_STARTED",
	"MM_EVT_ANIM_STOP",
	"MM_EVT_ANIM_STOPPED",
};

struct mm_queue_elt {
	int event;
};

static struct sta_mm_ctx sta_mm;

int sta_mm_services_handler(uint8_t services, void *priv, int len, void *data)
{
	if ((services | RPMSG_MM_SERV_STOP_ANIM) == RPMSG_MM_SERV_STOP_ANIM) {
		TRACE_INFO("[MM] Service STOP_ANIMATION\n");
		sta_mm.linux_ready = true;
		sta_mm_send_event(MM_EVT_ANIM_STOP);
	}
	return 0;
}

int sta_mm_send_event(int event)
{
	struct mm_queue_elt qelt;

	TRACE_INFO("[MM] Try to queue event %s\n", event2str[event]);
	qelt.event = event;
	if (xQueueSendToBack(sta_mm.mm_queue, &qelt, portMAX_DELAY) !=
	    pdPASS) {
		TRACE_INFO("[MM] Unable to queue event %s\n",
			   event2str[qelt.event]);
		return -1;
	}
	return 0;
}

void sta_mm_task(void *p)
{
	struct mm_queue_elt qelt;
	int registered_services = RPMSG_MM_SERV_STOP_ANIM;
	struct sta *ctxt = (struct sta *)p;

	TRACE_INFO("[MM] MM Task starting\n");

	/* Register to Stop_anim service to be informed
	 * when this message is received on M3 side
	 */
	rpmsg_mm_register_service(registered_services,
				  sta_mm_services_handler,
				  NULL);

	/* First think to do : request animation start at boot time */
	splash_start_anim(ctxt->features.splash_animation);

	while (xQueueReceive(sta_mm.mm_queue, &qelt, portMAX_DELAY)) {
		TRACE_INFO("[MM] Event %s dequeued, current state : %d\n",
			   event2str[qelt.event], sta_mm.state);
		switch (qelt.event) {
		case MM_EVT_RVC_RGE:
			if (sta_mm.state == MM_STATE_RVC)
				break;
			if (sta_mm.state == MM_STATE_ANIM)
				splash_stop_anim(sta_mm.linux_ready);
			rvc_start();
			sta_mm.state = MM_STATE_RVC;
			break;
		case MM_EVT_RVC_STARTED:
			sta_mm.state = MM_STATE_RVC;
			break;
		case MM_EVT_RVC_RGD:
		case MM_EVT_RVC_STOPPED:
			if (sta_mm.state == MM_STATE_RVC) {
				if (qelt.event != MM_EVT_RVC_STOPPED)
					rvc_stop();
				sta_mm.state = MM_STATE_IDLE;
				if (!sta_mm.linux_ready) {
					if (splash_start_anim(
					    ctxt->features.splash_animation))
						sta_mm.state = MM_STATE_ANIM;
				}
			}
			break;
		case MM_EVT_RVC_TAKEOVER:
			/* "rvc_ax_takeover" must be called in the context of
			 * "MM" task.
			 */
			rvc_ax_takeover();
			break;
		case MM_EVT_ANIM_STARTED:
			if (sta_mm.state == MM_STATE_RVC)
				splash_stop_anim(sta_mm.linux_ready);
			else
				sta_mm.state = MM_STATE_ANIM;
			break;
		case MM_EVT_ANIM_START:
			if (sta_mm.state == MM_STATE_IDLE) {
				if (splash_start_anim(
					ctxt->features.splash_animation))
					sta_mm.state = MM_STATE_ANIM;
			}
			break;
		case MM_EVT_ANIM_STOP:
			TRACE_INFO("Stop anim requested\n");
			splash_stop_anim(sta_mm.linux_ready);
		case MM_EVT_ANIM_STOPPED:
			if (sta_mm.linux_ready) {
				/* At this point Linux handover has been done
				 * so we can free resources: splash screen
				 * rgb file and splash animation M-JPEG file.
				 */
				if (sta_mm.splash_shadow_addr)
					free(sta_mm.splash_shadow_addr);
				if (sta_mm.anim_shadow_addr)
					free(sta_mm.anim_shadow_addr);
				sta_mm.splash_shadow_addr = NULL;
				sta_mm.anim_shadow_addr = NULL;
			}
			if (sta_mm.state == MM_STATE_ANIM)
				sta_mm.state = MM_STATE_IDLE;
			break;
		default:
			break;
		}
		TRACE_INFO("[MM] Event %s treated, current state : %d\n",
			   event2str[qelt.event], sta_mm.state);
	}
//on_error:
	vQueueDelete(sta_mm.mm_queue);
}

int sta_mm_init_env(struct sta *ctxt)
{
	int err = 0;
	struct xl1_part_info_t part_info;
	bool have_splash = false;

	memset(&sta_mm, 0, sizeof(sta_mm));
	sta_mm.state = MM_STATE_IDLE;
	sta_mm.mm_queue = xQueueCreate(20, sizeof(struct mm_queue_elt));

	/* read splash image if any before lcd init to avoid blue screen
	 * generating by delay between lcd_init() and lcd_update()
	 */
	err = read_image(SPLASH_ID, ctxt, &part_info,
			 IMAGE_SHADOW_ALLOC_LIBC);
	if (err == 0) {
		have_splash = true;
		sta_mm.splash_shadow_addr = part_info.shadow_address;
	}

	/* for splashscreen and rearcamera features, enable lcd */
	if (ctxt->features.splash_screen ||
	    ctxt->features.splash_animation ||
	    ctxt->features.rear_camera) {
		err = lcd_init();
		if (err)
			return err;
	}

	/* Load splash screen if any */
	if (ctxt->features.splash_screen) {
		if (have_splash) {
			splash_screen_init();
			err = splash_screen_update(sta_mm.splash_shadow_addr);
			if (err)
				return err;
		}
	}

	 /* Mandatory task :
	  * Create RPMSG_MM task used to established a bridge between
	  * M3 and Linux. This bridge is dedicated to MM resource sharing and
	  * specific Multimedia information exchange (Typically : RVC data
	  * transfer)
	  */
	if (ctxt->features.rpmsg_mm)
		err = xTaskCreate((pdTASK_CODE) rpmsg_mm_task,
				  (char *)"MM RPmsg", 222,
				  (void *)&ctxt->features,
				  TASK_PRIO_TEST_RPMSG_MM, NULL);
	if (err != pdPASS) {
		TRACE_INFO("[MM] Unable to RPMSG_MM task\n");
		return -1;
	}

	/* Optional task :
	 * Task used to handle the splash animation. It consists in the decode
	 * and display of jpeg files to simulate an animation while linux OS
	 * is not yet able to manage the display
	 */
	if (ctxt->features.splash_animation) {
		/* Splash screen animation */
		err = read_image(SPLASH_ANIMATION_ID, ctxt, &part_info,
				 IMAGE_SHADOW_ALLOC_LIBC);
		if (err == 0) {
			sta_mm.anim_shadow_addr = part_info.shadow_address;
			sta_mm.anim_size = part_info.size;
			/* Create Boot Flow task */
			err = xTaskCreate((pdTASK_CODE) splash_animation_task,
					  (char *)"Splash animation", 232,
					  (void *)&sta_mm,
					  TASK_PRIO_TEST_SPLASH_ANIM, NULL);
			if (err != pdPASS) {
				TRACE_INFO("[MM] Unable to start anim task\n");
				return -1;
			}
		}
	}

	/* Optional task :
	 * Task used to handle the Rear view camera use-case
	 */
	if (ctxt->features.rear_camera) {
		err = xTaskCreate((pdTASK_CODE) rvc_task,
				  (char *)"RVC", 200,
				  (void *)ctxt,
				  TASK_PRIO_TEST_RVC_CONTROL, NULL);
		if (err == pdPASS) {
			/* Indicate that we want to block the bootflow task
			 * until RVC task has completed the access to shared
			 * resources
			 */
			ctxt->event_bitmask |= RVC_TASK_EVENT_BIT;
		} else {
			TRACE_INFO("[MM] Unable to create RVC task\n");
			return -1;
		}
	}

	/* Background MM task */
	if (ctxt->features.rear_camera ||
	    ctxt->features.splash_animation) {
		err = xTaskCreate((pdTASK_CODE) sta_mm_task,
				  (char *)"MM", 210,
				  (void *)ctxt, TASK_PRIO_TEST_MM, NULL);
		if (err != pdPASS) {
			TRACE_INFO("[MM] Unable to create MM task\n");
			return -1;
		}
	}
	return 0;
}
