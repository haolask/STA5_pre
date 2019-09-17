/**
 * @file sta_sqi_nor_test.c
 * @brief This file provides SQI NOR test functions.
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Platform includes */
#include "trace.h"
#include "sta_cam.h"
#include "sta_tvdec.h"
#include "sta_gpio.h"
#include "sta_nvic.h"
#include "sta_mtu.h"
#include "sta_mbox.h"
#include "sta_sqi.h"
#include "sta_common.h"

#define OP_READ  0
#define OP_WRITE 1
#define OP_ERASE 2

#define WHAT(x) ((x == OP_READ) ?  "READ" : ((x == OP_WRITE) ? "WRITE" : "ERASE"))

/* Sync address and magic number */
#define SYNC_ADDR				0xABBBC0C0
#define HANDSHAKE_SYNC_C_MAGIC	0x00000CAB
#define HANDSHAKE_SYNC_R_MAGIC	0xBABABABA
#define HANDSHAKE_SYNC_W_MAGIC	0xBBBBBBBB
#define HANDSHAKE_SYNC_A_MAGIC	0xBCBCBCBC
#define HANDSHAKE_SYNC_U_MAGIC	0xBDBDBDBD


/* Define here the chunk of memory (bytes) */
#define CHUNK 16*1024

/* We have 1MB for testing purpose */
#define MAX_TASKS 4
#define PARTITION_SIZE	0x100000 / MAX_TASKS

/* Define here the address of the SQI NOR partition for testing purpose */
#define SQI_NOR_ADDR	0x007F0000				/* usually TEST1 */
#define SQI_NOR_ADDR_1	SQI_NOR_ADDR + PARTITION_SIZE
#define SQI_NOR_ADDR_2	SQI_NOR_ADDR + (2 * PARTITION_SIZE)
#define SQI_NOR_ADDR_3	SQI_NOR_ADDR + (3 * PARTITION_SIZE)

/* Define here the number of requests to perform */
#define REQUESTS 10

/* Define here the maximum time to wait between requests */
#define MAX_TIME 500

/* Define here the buffer size (in words) */
#define BUF_SIZE CHUNK/4

static int erase(struct t_sqi_ctx *ctx, uint32_t id, unsigned int addr)
{
	int ret;
	uint32_t timei;
	uint32_t timef;
	uint32_t speed;

	trace_printf("[task:%d] Erasing @%08X... ", id, addr);
	timei = mtu_get_timebase(TIME_IN_SEC) * 1000000
		+ mtu_get_timebase(TIME_IN_US);
	ret = sqi_erase_block(ctx, addr, SQI_64KB_GRANULARITY);
	timef = mtu_get_timebase(TIME_IN_SEC) * 1000000
		+ mtu_get_timebase(TIME_IN_US);
	speed = ((0x10000 >> 10) * 1000000) / (timef - timei);
	if (ret)
		trace_printf("ERROR\n");
	else
		trace_printf("OK at %dKB/s (start:%d end:%d)\n", speed, timef, timei);

	return ret;
}

static int write(struct t_sqi_ctx *ctx, uint32_t id, unsigned int addr, uint32_t buf_size)
{
	unsigned int i;
	int ret;
	uint32_t timei;
	uint32_t timef;
	uint32_t speed;
	uint32_t sqi_buf[buf_size];

	trace_printf("[task:%d] Writing @%08X... ", id, addr);
	for (i = 0; i < buf_size; i++)
		*((char *)sqi_buf + i) = (char)i;
	/* erase first */
	ret = sqi_erase_block(ctx, addr, SQI_64KB_GRANULARITY);
	timei = mtu_get_timebase(TIME_IN_SEC) * 1000000
		+ mtu_get_timebase(TIME_IN_US);
	ret = sqi_write(ctx, addr, sqi_buf, buf_size);
	timef = mtu_get_timebase(TIME_IN_SEC) * 1000000
		+ mtu_get_timebase(TIME_IN_US);
	speed = ((buf_size >> 10) * 1000000) / (timef - timei);
	if (ret)
		trace_printf("ERROR\n");
	else
		trace_printf("OK at %dKB/s (start:%d end:%d)\n", speed, timef, timei);

	return ret;
}

static int read(struct t_sqi_ctx *ctx, uint32_t id, unsigned int addr, uint32_t buf_size, bool check)
{
	unsigned int i;
	int ret;
	uint32_t timei;
	uint32_t timef;
	uint32_t speed;
	uint32_t sqi_buf[buf_size];

	trace_printf("[task:%d] Reading @%08X... ", id, addr);
	memset(sqi_buf, 0, buf_size);
	timei = mtu_get_timebase(TIME_IN_SEC) * 1000000
		+ mtu_get_timebase(TIME_IN_US);
	ret = sqi_read(ctx, addr, sqi_buf, buf_size);
	timef = mtu_get_timebase(TIME_IN_SEC) * 1000000
		+ mtu_get_timebase(TIME_IN_US);
	speed = ((buf_size >> 10) * 1000000) / (timef - timei);
	if (check) {
		/* Test memory */
		for (i = 0; i < buf_size; i++) {
			char val = *((char *)sqi_buf + i);
			if (val != (char)i)
				ret = -1;
		}
	}
	if (ret >= 0)
		trace_printf("OK at %dKB/s (start:%d end:%d)\n", speed, timef, timei);
	else
		trace_printf("ERROR\n");

	return ret;
}

/**
 * sqi_nor_tests - execute SQI/NOR tests
 */
void sqi_nor_tests(void *p)
{
	struct sta *context = (struct sta *)p;
	struct t_sqi_ctx *ctx = context->sqi_ctx;

	int r;
	unsigned int addr;
	uint32_t wait;
	uint32_t prev_op, op;
	void *hdl = xTaskGetCurrentTaskHandle();
	uint32_t prio = uxTaskPriorityGet(hdl);
	uint32_t id = (uint32_t)hdl;

	switch (prio) {
	default:
	case TASK_PRIO_TEST_SQI:
		addr = SQI_NOR_ADDR;
		break;
#if defined MULTIPLE_SQINOR_TEST_TASKS
	case TASK_PRIO_TEST_SQI_1:
		addr = SQI_NOR_ADDR_1;
		break;
	case TASK_PRIO_TEST_SQI_2:
		addr = SQI_NOR_ADDR_2;
		break;
#endif
	}

	trace_printf("[task:%d] %s: start (prio %d)\n", id, __func__, prio);

start:

#if !defined MULTIPLE_SQINOR_TEST_TASKS

	/* Make compiler happy */
	(void) op;
	(void) prev_op;
	(void) wait;

	/* -- time reference --
	 * Nobody is in front of us, we first perform a basic, ERASE, WRITE, READ
	 * operation in order to have a clean time reference
	 * Note: make sure that Linux does not perform any access during that time
	 */

	while (read_reg(SYNC_ADDR) != HANDSHAKE_SYNC_C_MAGIC)
		vTaskDelay(pdMS_TO_TICKS(100));

	trace_printf(" -- TEST #0: time reference -- \n");

	erase(ctx, id, addr);

	write(ctx, id, addr, BUF_SIZE);

	read(ctx, id, addr, BUF_SIZE, true);

	trace_printf(" -- TEST #0: end -- \n");

	/* -- Synchronized testing --
	 * In this section we execute some basic READ, ERASE/WRITE operations with
	 * synchronous accesses to the NOR
	 */

	while (read_reg(SYNC_ADDR) != HANDSHAKE_SYNC_R_MAGIC)
		vTaskDelay(pdMS_TO_TICKS(100));

	trace_printf(" -- TEST #1: synchronized testing -- \n");

	for (r = 0; r < 3; r++)
		read(ctx, id, addr, BUF_SIZE, false);

	/* step2: ERASE/WRITE */
	while (read_reg(SYNC_ADDR) != HANDSHAKE_SYNC_W_MAGIC)
		vTaskDelay(pdMS_TO_TICKS(100));

	for (r = 0; r < 3; r++) {
		/* erase first */
		sqi_erase_block(ctx, addr, SQI_64KB_GRANULARITY);
		write(ctx, id, addr, BUF_SIZE);
	}

	/* step3: READ while cortex A7 is ERASING/WRITING */
	while (read_reg(SYNC_ADDR) != HANDSHAKE_SYNC_A_MAGIC)
		vTaskDelay(pdMS_TO_TICKS(100));

	for (r = 0; r < 3; r++)
		read(ctx, id, addr, BUF_SIZE, false);

	trace_printf(" -- TEST #1: end -- \n");

#else /* !defined MULTIPLE_SQINOR_TEST_TASKS */

	/* -- Random testing --
	 * In this section we execute some different operations with different
	 * sizes at different interval
	 */

	r = 0;
	prev_op = 0xff;

	/* schedule as long as nobody is in front of us */;
	while (read_reg(SYNC_ADDR) != HANDSHAKE_SYNC_U_MAGIC)
		vTaskDelay(pdMS_TO_TICKS(100));

	trace_printf(" -- TEST #2: random testing -- \n");

	srand(mtu_get_timebase(TIME_IN_US));

	while (r < REQUESTS) {

		/* chose randomly an operation */
		op = rand() % (OP_ERASE + 1);

		/* consecutive reads are meaningless */
		if (op == OP_READ && prev_op == OP_READ)
			continue;

		wait = rand() % MAX_TIME;
		vTaskDelay(pdMS_TO_TICKS(wait));

		switch(op) {
		case OP_ERASE:
			erase(ctx, id, addr);
		case OP_WRITE:
			erase(ctx, id, addr);
			write(ctx, id, addr, BUF_SIZE);
		case OP_READ:
			erase(ctx, id, addr);
			write(ctx, id, addr, BUF_SIZE);
			read(ctx, id, addr, BUF_SIZE, true);
		}

		if (REQUESTS > 0)
			r++;
	}

	trace_printf(" -- TEST #2: end -- \n");

#endif

	trace_printf("[task:%d] %s: end\n", id, __func__);

	/* reset magic before new round */
	write_reg(0xFFFFFFFF, SYNC_ADDR);
	goto start;

	vTaskDelete(NULL);
}

