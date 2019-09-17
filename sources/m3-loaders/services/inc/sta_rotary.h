/**
 * @file sta_rotary.h
 * @brief This file provides all the roatry driver definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID team
 */

#ifndef _ROTARY_H_
#define _ROTARY_H_

int rotary_enable(QueueHandle_t q);
int rotary_disable(QueueHandle_t q);

#endif /*_ROTARY_H_*/
