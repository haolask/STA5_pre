/**
 * @file sta_rotary.c
 * @rotary encoder
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID team
 */

#include <errno.h>
#include "FreeRTOS.h"
#include "sta_gpio.h"
#include "sta_pinmux.h"
#include "queue.h"
#include "task.h"
#include "sta_nvic.h"

#define ENC_GPIO_A	S_GPIO(8)
#define ENC_GPIO_B	S_GPIO(9)
#define ENC_IRQ_CHANNEL GPIO_S_IRQChannel

static QueueHandle_t rotary_queue;

static int rotary_hdl(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken;
	int val, step;

	if (!rotary_queue)
		return -EIO;

	gpio_read_gpio_pin(ENC_GPIO_B, &val);
	step = val ? 1 : -1;

	if (xQueueGenericSendFromISR(rotary_queue, &step,
				     &xHigherPriorityTaskWoken,
				     queueSEND_TO_BACK))
		return -1;

	if (xHigherPriorityTaskWoken)
		taskYIELD();

	return 0;
}

int rotary_enable(QueueHandle_t q)
{
	struct gpio_config gpio_config;
	struct nvic_chnl irq_chnl;

	if (rotary_queue)
		return -EBUSY;

	irq_chnl.id = ENC_IRQ_CHANNEL;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	nvic_chnl_init(&irq_chnl);

	gpio_config.direction = GPIO_DIR_INPUT;
	gpio_config.mode	= GPIO_MODE_SOFTWARE;
	gpio_config.level = GPIO_LEVEL_LEAVE_UNCHANGED;
	gpio_config.trig = GPIO_TRIG_FALLING_EDGE;
	gpio_set_pin_config(ENC_GPIO_A, &gpio_config);

	gpio_config.trig = GPIO_TRIG_DISABLE;
	gpio_set_pin_config(ENC_GPIO_B, &gpio_config);

	rotary_queue = q;

	if (gpio_request_irq(ENC_GPIO_A, rotary_hdl)) {
		TRACE_ERR("%s: failed to request irq\n", __func__);
		return -EINVAL;
	}

	return 0;
}

int rotary_disable(QueueHandle_t q)
{
	int ret = 0;

	if (q != rotary_queue)
		return -EINVAL;

	ret = gpio_reset_gpio_pin(ENC_GPIO_A);
	if (ret)
		TRACE_ERR("%s: reset GPIO#%d failed\n", __func__, ENC_GPIO_A);

	ret = gpio_reset_gpio_pin(ENC_GPIO_B);
	if (ret)
		TRACE_ERR("%s: reset GPIO#%d failed\n", __func__, ENC_GPIO_B);

	rotary_queue = NULL;

	return 0;
}
