/**
 * @file sta_wdt.c
 * @brief Watchdog driver for M3 watchdog modules
 * Copyright (C) 2017 ST Microelectronics
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */

#include <errno.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "utils.h"

#include "sta_common.h"
#include "sta_map.h"
#include "sta_pm.h"
#include "sta_nvic.h"
#include "sta_src.h"
#include "sta_type.h"
#include "sta_wdt.h"
#include "trace.h"
#include "debug_regs.h"

#define WDT_LOCK	0xC00
#define WDT_MAGIC	0x1ACCE551

#define UNLOCK_WDT(reg_base) write_reg(WDT_MAGIC, (uint32_t)reg_base + WDT_LOCK)
#define LOCK_WDT(reg_base) write_reg(0x0, (uint32_t)reg_base + WDT_LOCK)
#define FLUSH_WDT(reg_base) read_reg((uint32_t)reg_base + WDT_LOCK)

struct sta_wdt {
	xSemaphoreHandle sem;
	struct sta_wdt_pdata pdata;
	uint8_t uart_console_no;
};
static struct sta_wdt *wdt;

void WDG_IRQHandler(void) {
	trace_init(wdt->uart_console_no, NULL, 0);

#ifndef NO_SCHEDULER
	TaskHandle_t curr_task = xTaskGetCurrentTaskHandle();
	TRACE_ERR("curr_task=%s\n", pcTaskGetTaskName(curr_task));
#endif
	print_process_registers();

	TRACE_ERR("M3 watchdog reset\n");
	__pm_reboot("M3 Watchdog", NULL);
}

int wdt_init(struct sta_wdt_pdata *pdata) {
	struct nvic_chnl irq_chnl;

	if (wdt)
		return -EEXIST;

	if (!pdata->reg_base) {
		TRACE_ERR("%s: invalid register base address\n",
			  __func__);
		return -EINVAL;
	}

	/* setup NVIC to catch WDT interrupt */
	irq_chnl.id = WDG_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	nvic_chnl_init(&irq_chnl);

	if(pdata->timeout <= pdata->reload_interval) {
		TRACE_ERR("%s: invalid reload interval versus timeout\n",
			  __func__);
		return -EINVAL;
	}

	wdt = pvPortMalloc(sizeof(struct sta_wdt));
	if (!wdt) {
		TRACE_ERR("%s: failed to allocate context\n", __func__);
		return -ENOMEM;

	}
	wdt->pdata.clk_rate = pdata->clk_rate;
	wdt->pdata.timeout = pdata->timeout;
	wdt->pdata.reload_interval = pdata->reload_interval;
	wdt->pdata.reload_auto = true;
	wdt->pdata.reg_base = pdata->reg_base;
	return 0;
}

int wdt_enable(void)
{
	volatile t_wdt *reg_base;
	if (!wdt)
		return -ENODEV;

	reg_base = wdt->pdata.reg_base;
	if (reg_base->wdt_cr.bit.inten) /* already enabled */
		return 0;

	taskENTER_CRITICAL();
	UNLOCK_WDT(reg_base);
	/**
	 * Let set a load value that is half of expected
	 * timeout. This because counter is decremented 2
	 * times: first time, interrupt is raised, second
	 * time reset is triggered.
	 */
	reg_base->wdt_lr.bit.wdtload = (wdt->pdata.clk_rate >> 1) *
		wdt->pdata.timeout - 1;
	reg_base->wdt_cr.bit.resen = 1;
	reg_base->wdt_cr.bit.inten = 1;
	LOCK_WDT(reg_base);
	FLUSH_WDT(reg_base);
	taskEXIT_CRITICAL();
	return 0;
}

int wdt_disable(void)
{
	volatile t_wdt *reg_base;
	if (!wdt)
		return -ENODEV;

	reg_base = wdt->pdata.reg_base;
	taskENTER_CRITICAL();
	UNLOCK_WDT(reg_base);
	reg_base->wdt_cr.bit.inten = 0;
	reg_base->wdt_cr.bit.resen = 0;
	LOCK_WDT(reg_base);
	FLUSH_WDT(reg_base);
	taskEXIT_CRITICAL();
	return 0;
}

int wdt_reload(unsigned int timeout)
{
	volatile t_wdt *reg_base;

	if (!wdt)
		return -ENODEV;

	wdt->pdata.timeout = timeout;
	reg_base = wdt->pdata.reg_base;
	if (!reg_base->wdt_cr.bit.inten) /* not enabled */
		return -EIO;
	xSemaphoreTake(wdt->sem, portMAX_DELAY);
	UNLOCK_WDT(reg_base);
	reg_base->wdt_lr.bit.wdtload = (wdt->pdata.clk_rate >> 1) *
		wdt->pdata.timeout - 1;
	LOCK_WDT(reg_base);
	FLUSH_WDT(reg_base);
	xSemaphoreGive(wdt->sem);
	/*TRACE_INFO("%s\n", __func__);*/
	return 0;
}

void wdt_reload_auto(bool enabled)
{
	wdt->pdata.reload_auto = enabled;
}

void wdt_task(void *cookie)
{
	struct feature_set *fs = (struct feature_set *)cookie;

	if (!wdt)
		goto end;
	wdt->uart_console_no = fs->uart_console_no;
	wdt->sem = xSemaphoreCreateCounting(1, 1);
	if (!wdt->sem) {
		TRACE_ERR("%s: failed to create semaphore\n", __func__);
		wdt_disable();
		goto end;
	}
	TRACE_INFO("%s init done\n", __func__);
	while(1) {
		vTaskDelay(pdMS_TO_TICKS(wdt->pdata.reload_interval * 1000));
		if (wdt->pdata.reload_auto)
			wdt_reload(wdt->pdata.timeout);
	}
end:
	if (wdt)
		vPortFree(wdt);
	vTaskDelete(NULL);
}
