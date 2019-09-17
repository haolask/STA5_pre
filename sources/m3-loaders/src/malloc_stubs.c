/**
 * @file malloc_stubs.c
 * @brief Stubs for libc malloc function for loaders without OS
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>

void * __attribute__((weak)) malloc(size_t xWantedSize)
{
	/* We do not expect to do malloc() calls in m3_xloader.
	 * So just return a null pointer */
	return NULL;
}

void __attribute__((weak)) free(void *pv) {}
