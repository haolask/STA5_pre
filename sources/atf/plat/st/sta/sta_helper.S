/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <platform_def.h>
#include <mailboxes.h>
#include <gic_common.h>
#include <gicv2.h>

	.globl	platform_mem_init
	.globl	plat_report_exception
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_reset_handler
	.globl	platform_get_core_pos
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	sta_resume_from_suspend
	.globl  mmc_fast_read
	.globl  mmc_fast_write
	.globl  nand_fast_read_512

func platform_mem_init
	/* nothing to do, don't need to init SYSRAM*/
	bx lr
endfunc platform_mem_init

func plat_report_exception
	/* perhaps we can do something like fvp LED used*/
	bx lr
endfunc plat_report_exception

func plat_reset_handler
	/* FIXME need to see if we need somthing here*/
	bx lr
endfunc plat_reset_handler

	/* ---------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot.
	 *
	 * For a cold boot, return 0.
	 * For a warm boot, read the mailbox and return the address it contains.
	 * ---------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	/* ---------------------------------------------------------------------
	 * Read the control word in the core mailbox. The fact that the
	 * control word is not set denotes a cold boot. Hence return 0.
	 * Else it's a warm boot so read the secure entrypoint in the mailbox
	 * and return it.
	 * This entrypoint must have been saved consequently to a call to
	 * CPU_SUSPEND or to CPU_ON.
	 *
	 * Put the mailbox address of this core in r0.
	 * ---------------------------------------------------------------------
	 */
	mov	r3, lr
	bl	plat_my_core_pos
	mov_imm	r1, MAILBOXES_BASE
	mov_imm	r2, MAILBOX_SIZE
	mla	r0, r0, r2, r1
	/* -----------------------------------------------------
	 * Check control word.
	 * -----------------------------------------------------
	 */
	ldr	r1, [r0, #MAILBOX_CONTROL_WORD]
	mov_imm	r2, WAKE_UP_CONTROL_WORD
	cmp	r1, r2
	bne	pgme_it_is_a_cold_boot
	ldr	r0, [r0, #MAILBOX_SEC_ENTRYPOINT]
	mov	lr, r3
	bx	lr
pgme_it_is_a_cold_boot:
	mov	r0, #0
	mov	lr, r3
	bx	lr
endfunc plat_get_my_entrypoint

	/* --------------------------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * For AArch32, cold-booting secondary CPUs is not yet
	 * implemented and they panic.
	 * --------------------------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	/* -----------------------------------------------------
	 * Program GIC so that a secondary core can wake up on a
	 * SGI coming from primary core thanks to a first call
	 * to CPU_ON PSCI API.
	 * -----------------------------------------------------
	 */
	mov_imm	r0, GICC_BASE
	mov	r1, #GIC_PRI_MASK
	str	r1, [r0, #GICC_PMR]
	mov_imm	r1, CTLR_ENABLE_G0_BIT | FIQ_EN_BIT | \
		FIQ_BYP_DIS_GRP0 | IRQ_BYP_DIS_GRP0 | \
		FIQ_BYP_DIS_GRP1 | IRQ_BYP_DIS_GRP1
	str	r1, [r0, #GICC_CTLR]

pscbs_sleep:
	/* -----------------------------------------------------
	 * Wait for an interrupt.
	 * -----------------------------------------------------
	 */
	mov	r3, #0
	dsb
	wfi

pscbs_loop:
	/* -----------------------------------------------------
	 * Reads the interrupt acknowledge register and
	 * therefore acknowledges the interrupt.
	 * -----------------------------------------------------
	 */
	ldr	r1, [r0, #GICC_IAR]
	ldr	r2, =INT_ID_MASK
	and	r1, r1, r2

	/* -----------------------------------------------------
	 * If the interrupt is non-secure or spurious then there
	 * is no more pending FIQ in GIC distributor then the
	 * current core can go further.
	 * -----------------------------------------------------
	 */
	mov_imm	r2, PENDING_G1_INTID
	cmp	r1, r2
	bge	pscbs_end

	/* -----------------------------------------------------
	 * Check that the interrupt is the expected one.
	 * Originator is not checked here but it is meant to be
	 * the primary core.
	 * -----------------------------------------------------
	 */
	mov	r2, #WAKING_UP_SGI
	cmp	r1, r2
	bne	pscbs_panic
	add	r3, r3, #1

	/* -----------------------------------------------------
	 * Claim interrupt processing completion.
	 * -----------------------------------------------------
	 */
	str	r1, [r0, #GICC_EOIR]
	b       pscbs_loop

pscbs_end:
	/* -----------------------------------------------------
	 * If the current core was woken by a spurious interrupt
	 * then it keeps on sleeping.
	 * -----------------------------------------------------
	 */
	cmp	r3, #0
	beq	pscbs_sleep

	/* -----------------------------------------------------
	 * Else branch to programmed address.
	 * -----------------------------------------------------
	 */
	bl	plat_get_my_entrypoint
	cmp	r0, #0
	bxne	r0

pscbs_panic:
	wfi
	b	pscbs_panic
endfunc plat_secondary_cold_boot_setup

func sta_resume_from_suspend
	bl	plat_set_my_stack
	ldr     r0, =(SCTLR_RESET_VAL & ~(SCTLR_TE_BIT | SCTLR_EE_BIT | SCTLR_V_BIT))
	stcopr	r0, SCTLR
	isb
	bl	plat_is_my_cpu_primary
	mov	r1, #1
	cmp	r0, r1
	bne	secondary_wait

	/* re-enable the uart console for early debug */
	ldr     r0, =CONSOLE_BASE
	ldr     r1, =PL011_UART_CLK_IN_HZ
	ldr	r2, =PL011_BAUDRATE
	bl	console_init

	/*
	 * Primary core initializes GIC and Timers before
	 * calling the generic warm boot func that will
	 * execute the psci suspend finish sequence.
	 */
	bl	generic_delay_timer_init
	bl	sta_gic_driver_init
	bl	sta_gic_init
	bl	sp_min_warm_entrypoint
secondary_wait:
	/*
	 * Secondary core must wait until being woken-up
	 * by primary core and thus just need to call the
	 * platform secondary cold boot function.
	 */
	bl	plat_secondary_cold_boot_setup
endfunc sta_resume_from_suspend

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	ldcopr	r0, MPIDR
	ldr	r1, =(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	and	r0, r1
	cmp	r0, #0x100
	moveq	r0, #1
	movne	r0, #0
	bx	lr
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * unsigned int plat_my_core_pos(void)
	 *
	 * Return CorePos = ((ClusterId - 1) * 4) + CoreId
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	ldcopr	r0, MPIDR
	and	r1, r0, #MPIDR_CPU_MASK
	and	r0, r0, #MPIDR_CLUSTER_MASK
	sub	r0, r0, #(1 << MPIDR_CLUSTER_SHIFT)
	add	r0, r1, r0, LSR #(MPIDR_CLUSTER_SHIFT - 2)
	bx	lr
endfunc plat_my_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 *
	 * Initialize the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	ldr	r0, =CONSOLE_BASE
	ldr	r1, =PL011_UART_CLK_IN_HZ
	ldr	r2, =PL011_BAUDRATE
	b	console_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 *
	 * Print a character on the crash console without a C Runtime stack.
	 * Clobber list : r0 - r3
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	ldr	r1, =CONSOLE_BASE
	b	console_core_putc
endfunc plat_crash_console_putc


/* --------------------------------
 * R0 data pointer
 * R1 SDI fifo reg address
 * --------------------------------
 */
func mmc_fast_read
	push  {r4-r9,r14}
	ldmia r1,{r2-r9}
	stmia r0!,{r2-r9}
	pop   {r4-r9,pc}
endfunc mmc_fast_read

/* --------------------------------
 * R0 data pointer
 * R1 SDI fifo reg address
 * --------------------------------
 */
func mmc_fast_write
	push  {r4-r9,r14}
	ldmia r0!,{r2-r9}
	stmia r1,{r2-r9}
	pop   {r4-r9,pc}
endfunc mmc_fast_write

/* -------------------------------------------
 * Fast read of 128 words (512 bytes)
 * R0 pointer to write data (word align(4))
 * R1 NAND base address to read (word pointer)
 * -------------------------------------------
 */
func nand_fast_read_512
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
endfunc nand_fast_read_512

