/**
 * @file sta_a7.h
 * @brief This file provides all the A7 utilities header
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_A7_H_
#define _STA_A7_H_

#include "sta_src.h"

#define CPU0	0
#define CPU1	1

/**
 * @brief	Set MTUs to input clock divided by 8
 */
static inline void a7_timers_clk_set(void)
{
	srcm3_set_mtu_timers_clk(src_m3_regs, 0xff);
}

/**
 * @brief	A7 Power up sequence, by:
 *	- setting PLL ARM as input clock of A7 sub-system
 *	- set reset trampoline code @0 to jump in start @ != 0
 *	- removing reset
 * @param	start: AP start address
 * @return	NA
 */
void a7_start(uint32_t start);

/**
 * @brief	Stop A7 core
 */
void a7_stop(void);

/**
 * @brief	enter a given CPU to stop
 * @param	cpuid: CPU processor id
 * @param	reset: set to true to trigger reset after, false otherwise
 * @return	0 if no error, not 0 otherwise
 */
int a7_cpu_stop(int cpuid);

/**
 * @brief	start a given CPU
 * @param	cpuid: CPU processor id
 * @return	0 if no error, not 0 otherwise
 */
int a7_cpu_start(int cpuid);

#endif /* _STA_A7_H_ */
