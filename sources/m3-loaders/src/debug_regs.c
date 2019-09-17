/**
 * @file debug_regs.c
 * @brief Thread mode low level registers debug
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID Application Team
 */

#include <stdint.h>
#include "trace.h"

void print_process_registers_from_stack(uint32_t *sp)
{
	TRACE_ERR("lr=%08X, pc=%08X, psr=%08X\n",
		      sp[5], sp[6], sp[7]);
}

/*
 * @brief: Dump and print registers from Process Stack Pointer
 * It is a naked function - in effect this is just an assembly function.
 */
__attribute__((naked)) void print_process_registers(void)
{
	__asm volatile
		(
		 " mrs r0, psp                                               \n"
		 " ldr r2, handler_address                                   \n"
		 " bx r2					             \n"
		 " handler_address: .word print_process_registers_from_stack \n"
		);
}
