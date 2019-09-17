/**
 * @file audiolib.h
 * @brief audio header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_AUDIO_H__
#define __STA_AUDIO_H__

#define API_SIGNATURE 0xad10ca11
#define MOD_SIGNATURE 0xad10ca13
#define API_NAME_MAX	33

typedef struct {
	unsigned int id;
	unsigned short size;
	char hsize;
	char name[API_NAME_MAX];
} api_header;

typedef struct {
	unsigned int addr, val;
} setreg;

typedef struct {
	unsigned int xaddr, yaddr;
	char size;
	unsigned int type;
	char name[API_NAME_MAX];
} modinfo;

#endif /* __STA_AUDIO_H__ */
