/*
 * startup.s: Xloaders startup entry
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * Author:  APG-MID Application Team
 */

.global reset_handler
.global exit
.syntax unified

.section .init, "ax"
.code 16
.align 2
.thumb_func

@===================================================================
@ Reset handler (entry point)
@===================================================================

reset_handler:

	/* If this is a RAM build, configure vector table offset register to point
	to the RAM vector table. */

	ldr r0, =0xE000ED08
	ldr r1, =_vectors
	str r1, [r0]

	/***************/
	/* Stack setup */
	/***************/

	ldr r1, =__stack_end__
	mov sp, r1

	/* Setup initial call frame */
	mov r0, #0
	mov lr, r0
	mov r12, sp

@===================================================================
@ start function
@===================================================================
	.type start, function
start:
	/* Jump to C bootstrap function */
	ldr r2, =cstartup
	blx r2

.section .text, "ax"
.code 16
.align 2
.thumb_func

@===================================================================
@ exit function
@===================================================================
	.thumb_func
	/* Returned from application entry point, loop forever. */
exit:
	b exit

@===================================================================
@ STAXXXX requires these
@===================================================================
	.global __WFI
	.global __WFE
	.global __SEV
	.global __ISB
	.global __DSB
	.global __DMB
	.global __SVC
	.global __MRS_CONTROL
	.global __MSR_CONTROL
	.global __MRS_PSP
	.global __MSR_PSP
	.global __MRS_MSP
	.global __MSR_MSP
	.global __SETPRIMASK
	.global __RESETPRIMASK
	.global __SETFAULTMASK
	.global __RESETFAULTMASK
	.global __BASEPRICONFIG
	.global __GetBASEPRI
	.global __REV_HalfWord
	.global __REV_Word

.thumb_func
__WFI:
	wfi
	bx r14
.thumb_func
__WFE:
	wfe
	bx r14
.thumb_func
__SEV:
	sev
	bx r14
.thumb_func
__ISB:
	isb
	bx r14
.thumb_func
__DSB:
	dsb
	bx r14
.thumb_func
__DMB:
	dmb
	bx r14
.thumb_func
__SVC:
	svc 0x01
	bx r14
.thumb_func
__MRS_CONTROL:
	mrs r0, control
	bx r14
.thumb_func
__MSR_CONTROL:
	msr control, r0
	isb
	bx r14
.thumb_func
__MRS_PSP:
	mrs r0, psp
	bx r14
.thumb_func
__MSR_PSP:
	msr psp, r0
	bx r14
.thumb_func
__MRS_MSP:
	mrs r0, msp
	bx r14
.thumb_func
__MSR_MSP:
	msr msp, r0
	bx r14
.thumb_func
__SETPRIMASK:
	cpsid i
	bx r14
.thumb_func
__RESETPRIMASK:
	cpsie i
	bx r14
.thumb_func
__SETFAULTMASK:
	cpsid f
	bx r14
.thumb_func
__RESETFAULTMASK:
	cpsie f
	bx r14
.thumb_func
__BASEPRICONFIG:
	msr basepri, r0
	bx r14
.thumb_func
__GetBASEPRI:
	mrs r0, basepri_max
	bx r14
.thumb_func
__REV_HalfWord:
	rev16 r0, r0
	bx r14
.thumb_func
__REV_Word:
	rev r0, r0
	bx r14

	/* Setup attibutes of stack sections so they don't take up room in the elf file */
	.section .stack, "wa", %nobits

