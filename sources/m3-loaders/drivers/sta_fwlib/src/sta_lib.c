/**
 * @file sta_lib.c
 * @brief This file provides all peripherals pointers initialization.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "FreeRTOS.h"
#include "task.h"

#include "sta_lib.h"
#include "sta_map.h"

/**
 * @brief	This function allocates and initializes ring buffer area.
 * @param	ring_buf: Pointer on ring buffer structure
 * @param	elem_size: Size of each buffer element
 * @param	nb_elem: Number of buffer element
 * @retval:		0 if no error, not 0 otherwise
 */
int lib_rbuf_alloc(struct lib_rbuf *ring_buf, uint32_t elem_size, uint32_t nb_elem)
{
	struct lib_rbuf_hdr *hdr;
	void *data;
	uint32_t i;

	if (!ring_buf)
		return -EINVAL;

	ring_buf->free_elem = ring_buf->max_elem = nb_elem;
	ring_buf->elem_size = elem_size + sizeof(struct lib_rbuf_hdr);

	/* Allocate ring buffer area for data payload */
	ring_buf->data_pool = ring_buf->next_free = data =
	    (void *)pvPortMalloc(ring_buf->elem_size * nb_elem);
	if (!data)
		return -EINVAL;

	/* Initialise Ring buffer */
	for (i = 0; i < nb_elem; i++) {
		hdr = (struct lib_rbuf_hdr *) data;
		hdr->used = 0;

		data += ring_buf->elem_size;
	}
	ring_buf->data_end = data;

	return 0;
}

/**
 * @brief	This function returns a free buffer element from
 * the specified ring buffer area.
 * @param	ring_buf: pointer on Ring buffer structure
 * @retval	buffer address allocated or NULL
 */
void *lib_rbuf_get(struct lib_rbuf *ring_buf, uint32_t *nb_free)
{
	struct lib_rbuf_hdr *hdr;
	void *next_free, *data = NULL;
	uint32_t i;

	/* Get the next free buffer element */
	for (i = 0, next_free = ring_buf->next_free; i < ring_buf->max_elem; i++) {
		hdr = (struct lib_rbuf_hdr *) next_free;
		if (!hdr->used) {
			if (!data) {
				hdr->used = 1;
				data = hdr->data;
				ring_buf->free_elem--;
			} else {
				ring_buf->next_free = next_free;
				break;
			}
		}
		next_free += ring_buf->elem_size;
		if (next_free >= ring_buf->data_end)
			next_free = ring_buf->data_pool;
	}
	if (nb_free)
		*nb_free = ring_buf->free_elem;

	return data;
}

/**
 * @brief	This function frees a buffer element
 * @param	ring_buf: pointer on Ring buffer structure
 * @param	buf:     buffer address to free
 */
void lib_rbuf_put(struct lib_rbuf *ring_buf, void *buf)
{
	struct lib_rbuf_hdr *hdr;

	if (!buf)
		return;

	/* Free required Ring buffer */
	hdr = (struct lib_rbuf_hdr *) (buf - sizeof(struct lib_rbuf_hdr));

	taskENTER_CRITICAL();
	hdr->used = 0;
	ring_buf->free_elem++;
	taskEXIT_CRITICAL();
}

/**
 * @brief	This function frees the ring buffer area.
 * @param	ring_buf: Pointer on ring buffer structure
 */
void lib_rbuf_free(struct lib_rbuf *ring_buf)
{
	/* Free ring buffer area for data payload */
	vPortFree(ring_buf->data_pool);

	ring_buf->data_pool = ring_buf->next_free = ring_buf->data_end = NULL;
	ring_buf->max_elem = ring_buf->free_elem = ring_buf->elem_size = 0;
}
