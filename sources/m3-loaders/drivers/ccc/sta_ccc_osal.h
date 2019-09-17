/**
 * @file sta_ccc_osal.h
 * @brief CCC driver private header for OS abstraction layer.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _CCC_OSAL_H_
#define _CCC_OSAL_H_

#ifdef DEBUG
#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LEVEL_VERBOSE
#endif

#include "FreeRTOS.h"
#include "semphr.h"
#include "sta_mtu.h"
#include "sta_nvic.h"
#include "task.h"
#include "trace.h"
#include "utils.h"

#ifndef NO_SCHEDULER
/* Defined as make use of interruption handling instead of polling. */
#define HAVE_INTERRUPT

/* Defined as semaphore API is available. */
#define HAVE_SEM
#endif /* NO_SCHEDULER */

/* Defined as patch hooks must be exported. */
#undef HAVE_HOOKS

/* Implement these macros if requested. */
#ifndef TRACE_ERR
#define TRACE_ERR(format, ...)  (void)0
#endif
#ifndef TRACE_INFO
#define TRACE_INFO(format, ...) (void)0
#endif

#define ASSERT(x) configASSERT(x)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

#ifdef HAVE_INTERRUPT
#define completion_t TaskHandle_t

void init_completion(completion_t *, void *);
void reinit_completion(completion_t *);
void wait_for_completion(completion_t *);
void complete(completion_t *);

#define ccc_isr ccc_irq_handler
#endif /* HAVE_INTERRUPT */

void start_counter(void);

#ifdef HAVE_SEM
#define semaphore_t SemaphoreHandle_t
#else
#define semaphore_t int
#endif /* HAVE_SEM */

void create_sem(semaphore_t *sem);
bool take_sem(semaphore_t *sem);
bool give_sem(semaphore_t *sem);
#endif /* _CCC_OSAL_H_ */
