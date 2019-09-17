/*
 * nand_fast.s: Provide fast nand access functions (read 128 words)
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * Author: APG-MID team
 */

.global nand_fast_read_512
.syntax unified

.section .text
.code 16
.align 2
.thumb_func

@ Fast read of 128 words (512 bytes)
@ R0 pointer to write data (word align(4))
@ R1 NAND base address to read (word pointer)
nand_fast_read_512:
	push     {r4-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r12,r14} @ Read 48 bytes in one instruction
	stmia r0!,{r2-r12,r14}
	ldmia r1,{r2-r9} @ Read 32 bytes in one instruction
	stmia r0!,{r2-r9}
	pop      {r4-r12,pc}

