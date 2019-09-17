/**
 * @file sta_remote_gpio.c
 * @brief Remote gpio driver
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID team
 */

/* Standard includes. */
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Platform includes */
#include "sta_remote_gpio.h"
#include "trace.h"
#include "sta_mbox.h"
#include "sta_gpio.h"
#include "sta_pinmux.h"

struct pinmux_instr_msg {
	struct gpio_mux *groups;
	uint8_t mux_type;
	int size;
};

struct rgpio_instr_msg {
	uint16_t pin;
	uint8_t code;
	uint8_t value;
};

/**************************************************
 *              /!\ WARNING /!\                   *
 * This enumration is shared with linux		  *
 * The order of items is important		  *
 * refer to file: pinctrl-nomadik-remote in linux *
 * modify at your own risk			  *
 **************************************************/
enum rgpio_instr_codes {
	RGPIO_GET,
	RGPIO_SET,
	RGPIO_GET_DIR,
	RGPIO_SET_DIR,
	RGPIO_GET_MODE,
	RGPIO_SET_MODE,
	RGPIO_GET_TRIG,
	RGPIO_SET_TRIG,
	RGPIO_GET_LEVEL,
	RGPIO_SET_LEVEL,
	RGPIO_IRQ_ACK,
	RGPIO_IRQ_HANDLER,
	RGPIO_ERR,
};

static xQueueHandle instr_q;

void remote_gpio_isr(void *context, struct mbox_msg *msg)
{
	struct rgpio_instr_msg *rgpio_req;
	portBASE_TYPE xHigherPriorityTaskWoken;

	if (msg->dsize != sizeof(*rgpio_req))
		return;

	rgpio_req = (struct rgpio_instr_msg *)(msg->pdata);

	/* Enqueue message code and pin */
	xQueueSendToBackFromISR(instr_q, (void *)rgpio_req,
				&xHigherPriorityTaskWoken);
	/*
	 * TODO: check if actual struct rgpio_req is queued
	 * or just pointer ?
	 */

	if (xHigherPriorityTaskWoken)
		taskYIELD();
}

void remote_gpio_task(void *p)
{
	struct mbox_chan_req mbox_req;
	struct rgpio_instr_msg rgpio_req;
	struct gpio_config pin;
	struct mbox_msg msg;
	int chan_id, ioval, m3_pin, ret;

	/* Create a queue for up to 3 rxMsg per mailboxes */
	instr_q = xQueueCreate(3, sizeof(struct rgpio_instr_msg));

	/* Request mailbox channel */
	mbox_req.chan_name = "remote-gpio";
	mbox_req.user_data = NULL;
	mbox_req.rx_cb = remote_gpio_isr;
	chan_id = mbox_request_channel(&mbox_req);
	if (chan_id == MBOX_ERROR) {
		TRACE_ERR("remote_gpio_task: request channel failed\n");
		goto exit;
	}
	TRACE_INFO("remote_gpio_task: Requested channel %s allocated\n",
		   mbox_req.chan_name);

	while (1) {
		if (xQueueReceive(instr_q, &rgpio_req, portMAX_DELAY)) {
			m3_pin = M3_GPIO(rgpio_req.pin);
			/* Check rgpio_req.pin here and filter if necessary */
			switch (rgpio_req.code) {
			case RGPIO_GET:
				ret = gpio_read_gpio_pin(m3_pin, &ioval);
				if (ret >= 0) {
					/* No error. Save value. Reset ret */
					rgpio_req.value = ioval;
					ret = 0;
				}
				break;
			case RGPIO_SET:
				if (rgpio_req.value)
					ret = gpio_set_gpio_pin(m3_pin);
				else
					ret = gpio_clear_gpio_pin(m3_pin);

				break;
			case RGPIO_GET_DIR:
				ret = gpio_get_pin_dir(m3_pin);
				if (ret >= 0) {
					/* No error. Save value. Reset ret */
					rgpio_req.value = ret > 0 ? 1 : 0;
					ret = 0;
				}

				break;
			case RGPIO_SET_DIR:
				pin.mode = GPIO_MODE_SOFTWARE;
				pin.direction = rgpio_req.value ?
						GPIO_DIR_OUTPUT :
						GPIO_DIR_INPUT;
				pin.trig = GPIO_TRIG_LEAVE_UNCHANGED;
				pin.level = GPIO_LEVEL_LEAVE_UNCHANGED;
				ret = gpio_set_pin_config(m3_pin, &pin);
				break;
			case RGPIO_GET_MODE:
				ret = gpio_get_mode(m3_pin);
				if (ret >= 0) {
					rgpio_req.value = ret;
					ret = 0;
				}
				break;
			case RGPIO_SET_MODE:
				pin.mode = rgpio_req.value;
				pin.direction = GPIO_DIR_LEAVE_UNCHANGED;
				pin.trig = GPIO_TRIG_LEAVE_UNCHANGED;
				pin.level = GPIO_LEVEL_LEAVE_UNCHANGED;
				ret = gpio_set_pin_config(m3_pin, &pin);
				break;
			case RGPIO_GET_TRIG:
				ret = gpio_get_trig(m3_pin);
				if (ret >= 0) {
					rgpio_req.value = ret;
					ret = 0;
				}
				break;
			case RGPIO_SET_TRIG:
				pin.mode = GPIO_MODE_SOFTWARE;
				pin.direction = GPIO_TRIG_LEAVE_UNCHANGED;
				pin.trig = rgpio_req.value;
				pin.level = GPIO_TRIG_LEAVE_UNCHANGED;
				ret = gpio_set_pin_config(m3_pin, &pin);
				break;
			case RGPIO_GET_LEVEL:
				ret = gpio_get_level(m3_pin);
				if (ret >= 0) {
					rgpio_req.value = ret;
					ret = 0;
				}
				break;
			case RGPIO_SET_LEVEL:
				pin.mode = GPIO_MODE_SOFTWARE;
				pin.direction = GPIO_LEVEL_LEAVE_UNCHANGED;
				pin.trig = GPIO_LEVEL_LEAVE_UNCHANGED;
				pin.level = rgpio_req.value;
				ret = gpio_set_pin_config(m3_pin, &pin);
				break;
			case RGPIO_IRQ_ACK:
				gpio_irq_ack(m3_pin);
				ret = 0;
				break;
			case RGPIO_IRQ_HANDLER:
				gpio_irq_handler();
				ret = 0;
				break;
			default:
				TRACE_ERR(
					"remote_gpio_task: unknown code: %d\n",
					(uint8_t)(rgpio_req.code));
				ret = -1;
			}
			if (ret)
				rgpio_req.code = RGPIO_ERR;
			msg.dsize = sizeof(struct rgpio_instr_msg);
			msg.pdata = (uint8_t *)&rgpio_req;

			while (mbox_send_msg(chan_id, &msg) == MBOX_BUSY)
				;
		}
	}

exit:
	TRACE_ERR("remote_gpio_task: exit\n");
	vTaskDelete(NULL);
}

// End of file
