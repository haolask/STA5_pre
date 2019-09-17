/*
 * nand_fast.s: Provide fast mmc access functions (read/write words)
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * Author: APG-MID team
 */

	.global mmc_fast_read
	.global mmc_fast_write
	.syntax unified

	.text
	.code 16
	.align 2
    .thumb_func

@	R0 data pointer
@	R1 SDI FIFO pointer
mmc_fast_write:
        push  {r4-r9,r14}
	ldmia r0!,{r2-r9}
	stmia r1,{r2-r9}
	pop   {r4-r9,pc}


@	R0 data pointer
@	R1 SDI FIFO pointer
mmc_fast_read:
        push  {r4-r9,r14}
	ldmia r1,{r2-r9}
	stmia r0!,{r2-r9}
	pop   {r4-r9,pc}

