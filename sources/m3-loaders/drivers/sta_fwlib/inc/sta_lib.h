/**
 * @file sta_lib.h
 * @brief This file provides all peripherals pointers initialization.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_LIB_H__
#define __STA_LIB_H__

#include "sta_map.h"

struct lib_rbuf_hdr {
	uint32_t used;
	char data[0];
} __attribute__ ((packed));

struct lib_rbuf {
	uint32_t free_elem;
	uint32_t max_elem;
	uint32_t elem_size;
	void *data_pool;
	void *next_free;
	void *data_end;
};

int lib_rbuf_alloc(struct lib_rbuf *, uint32_t, uint32_t);
void *lib_rbuf_get(struct lib_rbuf *, uint32_t *);
void lib_rbuf_put(struct lib_rbuf *, void *);
void lib_rbuf_free(struct lib_rbuf *);

#endif /* __STA_LIB_H__ */
