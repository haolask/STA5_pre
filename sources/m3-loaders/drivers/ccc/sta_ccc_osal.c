/**
 * @file sta_ccc_osal.c
 * @brief CCC driver OS abstraction layer.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

/* #define PERF_MEASUREMENT */

#ifdef PERF_MEASUREMENT
#define DEBUG
#endif

#include "FreeRTOS.h"
#include "semphr.h"
#include "sta_mtu.h"
#include "task.h"
#include "trace.h"
#include "utils.h"

#include "sta_ccc_osal.h"

#ifdef PERF_MEASUREMENT
static unsigned int time;
#endif

#ifdef HAVE_INTERRUPT
inline void init_completion(TaskHandle_t *xTaskToNotify,
			    __maybe_unused void *dummy)
{
	*xTaskToNotify = NULL;
}

inline void reinit_completion(TaskHandle_t *xTaskToNotify)
{
	*xTaskToNotify = xTaskGetCurrentTaskHandle();
	ASSERT(*xTaskToNotify);
}

inline void wait_for_completion(TaskHandle_t *xTaskToNotify)
{
	ASSERT(*xTaskToNotify == xTaskGetCurrentTaskHandle());
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

void complete(TaskHandle_t *xTaskToNotify)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

#ifdef PERF_MEASUREMENT
	TRACE_INFO("%s: time is %d µs\n", __func__, get_elapsed_time());
#endif
	vTaskNotifyGiveFromISR(*xTaskToNotify, &xHigherPriorityTaskWoken);
	*xTaskToNotify = NULL;

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif /* HAVE_INTERRUPT */

void start_counter(void)
{
#ifdef PERF_MEASUREMENT
	time = mtu_get_timebase(TIME_IN_US);
#endif
}

#ifdef PERF_MEASUREMENT
static inline unsigned int get_elapsed_time(void)
{
	return mtu_get_timebase(TIME_IN_US) - time;
}
#endif

#ifdef HAVE_SEM
inline void create_sem(semaphore_t *sem)
{
	*sem = xSemaphoreCreateBinary();
}

inline bool take_sem(semaphore_t *sem)
{
	return (pdTRUE == xSemaphoreTake(*sem, portMAX_DELAY));
}

inline bool give_sem(semaphore_t *sem)
{
	return (pdTRUE == xSemaphoreGive(*sem));
}
#else
inline void create_sem(__maybe_unused semaphore_t *sem)
{
}

inline bool take_sem(__maybe_unused semaphore_t *sem)
{
	return true;
}

inline bool give_sem(__maybe_unused semaphore_t *sem)
{
	return true;
}
#endif /* HAVE_SEM */
