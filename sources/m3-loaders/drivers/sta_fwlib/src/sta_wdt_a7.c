/**
 * @file sta_wdt.c
 * @brief Watchdog driver for M3 and A7 watchdog modules
 * Copyright (C) 2017 ST Microelectronics
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */

#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "utils.h"

#include "sta_map.h"
#include "sta_pm.h"
#include "sta_nvic.h"
#include "sta_src.h"
#include "sta_type.h"
#include "sta_wdt_a7.h"
#include "trace.h"

static xSemaphoreHandle wdt_sem;

static int irq_init(void)
{
	struct nvic_chnl irq_chnl;

	/* setup NVIC to catch WDT interrupt */
	irq_chnl.id = WDG_A7_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	return nvic_chnl_init(&irq_chnl);
}

void WDG_A7_IRQHandler(void)
{
	/* Put a7 watchdog in reset to avoid branching again in interrupt */
	misc_a7_regs->misc_reg16.bit.a7_wdog_reset = 0;

	portBASE_TYPE xHigherPriorityTaskWoken;
	if (!xSemaphoreGiveFromISR(wdt_sem, &xHigherPriorityTaskWoken))
		return;
	if (xHigherPriorityTaskWoken)
		taskYIELD();
}

void wdt_a7_task(void)
{
	wdt_sem = xSemaphoreCreateCounting(1, 0);
	if (!wdt_sem) {
		TRACE_ERR("%s: failed to create semaphore\n", __func__);
		goto end;
	}
	if (irq_init()) {
		TRACE_ERR("%s: failed to get irq\n", __func__);
		goto end;
	}

	TRACE_INFO("%s: init done\n", __func__);

	while(1) {
		xSemaphoreTake(wdt_sem, portMAX_DELAY);
		TRACE_ERR("%s: A7 Watchdog timeout\n", __func__);
		/* trigger a platform reboot without notifying AP */
		pm_reboot(true);
	}
end:
	while(1)
		vTaskDelay(pdMS_TO_TICKS(10));
}
