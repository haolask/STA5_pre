/**
 * @file free_RTOS_stubs.c
 * @brief Stubs for free RTOS function for loaders without OS
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define MALLOC_POOL_SIZE	(12 * 1024)

/**
 * FreeRTOS stub functions used in pmu reset and i2c
 */
void __attribute__((weak)) vPortEnterCritical(void){}
void __attribute__((weak)) vPortExitCritical(void) {}
BaseType_t __attribute__((weak)) xQueueGiveFromISR(QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken) { return 0; }

/* Minimal malloc implementation without free: see heap_1.c from FreeRTOS */
void * __attribute__((weak)) pvPortMalloc(size_t xWantedSize)
{
	void *pvReturn = NULL;
	static uint8_t *pucAlignedHeap = NULL;
	static size_t xNextFreeByte;
	static uint32_t malloc_heap[MALLOC_POOL_SIZE / 4];

	/* Ensure that blocks are always aligned to the required number of bytes. */
#if portBYTE_ALIGNMENT != 1
	/* Byte alignment required. */
	if (xWantedSize & portBYTE_ALIGNMENT_MASK)
		xWantedSize += (portBYTE_ALIGNMENT
				- (xWantedSize & portBYTE_ALIGNMENT_MASK));
#endif

	if( pucAlignedHeap == NULL )
		pucAlignedHeap = (uint8_t *)malloc_heap;

	/* Check there is enough room left for the allocation. */
	if (((xNextFreeByte + xWantedSize) < sizeof(malloc_heap)) &&
			((xNextFreeByte + xWantedSize) > xNextFreeByte))
	{
		/* Return the next free byte then increment the index past this
		   block. */
		pvReturn = pucAlignedHeap + xNextFreeByte;
		xNextFreeByte += xWantedSize;
	}

	return pvReturn;
}

void __attribute__((weak)) vPortFree( void *pv )
{
	/* Memory cannot be freed */
	(void) pv;
}

