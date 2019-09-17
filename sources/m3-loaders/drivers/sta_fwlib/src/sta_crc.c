/**
 * @file sta_crc.c
 * @brief This file provides all the CRC routines
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include "sta_crc.h"

#define DO_CRC(x) crc = crc32_tab[(crc ^ (x)) & 255] ^ (crc >> 8)

uint32_t compute_crc32(uint32_t crc, uint8_t *buf, uint32_t len)
{
	const uint32_t *b =(const uint32_t *)buf;
	uint32_t rem_len;

	crc = crc ^ 0xffffffff;

	/* Align it */
	if (((long)b) & 3 && len) {
		uint8_t *p = (uint8_t *)b;
		do {
			DO_CRC(*p++);
		} while ((--len) && ((long)p)&3);
		b = (uint32_t *)p;
	}

	rem_len = len & 3;
	len = len >> 2;
	for (--b; len; --len) {
		/* load data 32 bits wide, xor data 32 bits wide. */
		crc ^= *++b; /* use pre increment for speed */
		DO_CRC(0);
		DO_CRC(0);
		DO_CRC(0);
		DO_CRC(0);
	}
	len = rem_len;
	/* And the last few bytes */
	if (len) {
		uint8_t *p = (uint8_t *)(b + 1) - 1;
		do {
			DO_CRC(*++p); /* use pre increment for speed */
		} while (--len);
	}

	return crc ^ 0xffffffff;
}

