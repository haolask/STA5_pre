/*
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <asm.S>
#include <arm.h>
#include <arm32_macros.S>
#include <keep.h>
#include <kernel/cache_helpers.h>
#include <kernel/unwind.h>
#include <sm/optee_smc.h>
#include <sm/teesmc_opteed.h>
#include <sm/teesmc_opteed_macros.h>
#include <platform_config.h>

	.globl	get_core_pos_mpidr
	.globl	set_cpsr_mode_mon
	.globl	set_cpsr_mode_svc
	.globl	sta_cpu_resume


/* For sta SoC, the two A7 are implemented in 1 cluster only (cluster ID 1) */
FUNC get_core_pos_mpidr , :
UNWIND(	.fnstart)
	/* Calculate CorePos = CoreId */
	and	r0, r0, #MPIDR_CPU_MASK
	bx	lr
UNWIND(	.fnend)
END_FUNC get_core_pos_mpidr


/* Set processor state to MODE_MON */
/* keep lr and sp of initial mode */
FUNC set_cpsr_mode_mon , :
UNWIND(	.fnstart)
	mov	r0, lr
	mov	r1, sp
	cps	#CPSR_MODE_MON
	mov	lr, r0
	mov	sp, r1
	bx	lr
UNWIND(	.fnend)
END_FUNC set_cpsr_mode_mon


/* Set processor state to MODE_SVC */
/* keep lr and sp of initial mode */
FUNC set_cpsr_mode_svc , :
UNWIND(	.fnstart)
	mov	r0, lr
	mov	r1, sp
	cps	#CPSR_MODE_SVC
	mov	lr, r0
	mov	sp, r1
	bx	lr
UNWIND(	.fnend)
END_FUNC set_cpsr_mode_svc


	.section .text.resume_vect_table
	.align 5
LOCAL_FUNC resume_vect_table , :
	b	.
	b	.	/* Undef */
	b	.	/* Syscall */
	b	.	/* Prefetch abort */
	b	.	/* Data abort */
	b	.	/* Reserved */
	b	.	/* IRQ */
	b	.	/* FIQ */
END_FUNC resume_vect_table

	/*
	 * set_sctlr : Setup some core configuration in CP15 SCTLR
	 *
	 * Setup required by current implementation of the OP-TEE core:
	 * - Disable data and instruction cache.
	 * - MMU is expected off and exceptions trapped in ARM mode.
	 * - Enable or disable alignment checks upon platform configuration.
	 * - Optinally enable write-implies-execute-never.
	 * - Optinally enable round robin strategy for cache replacement.
	 *
	 * Clobbers r0.
	 */
	.macro set_sctlr
		read_sctlr r0
		bic	r0, r0, #(SCTLR_M | SCTLR_C)
		bic	r0, r0, #SCTLR_I
		bic	r0, r0, #SCTLR_TE
#if defined(CFG_SCTLR_ALIGNMENT_CHECK)
		orr	r0, r0, #SCTLR_A
#else
		bic	r0, r0, #SCTLR_A
#endif
#if defined(CFG_HWSUPP_MEM_PERM_WXN) && defined(CFG_CORE_RWDATA_NOEXEC)
		orr	r0, r0, #(SCTLR_WXN | SCTLR_UWXN)
#endif
#if defined(CFG_ENABLE_SCTLR_RR)
		orr	r0, r0, #SCTLR_RR
#endif
		write_sctlr r0
	.endm

	/*
	 * enable_branch_prediction : manually enable branch prediction
	 *
	 * This macro targets only ARMv7 architecture and hence conditionned
	 * by configuration directive CFG_ENABLE_SCTLR_Z. For recent
	 * architectures, the program flow prediction is automatically enable
	 * upon MMU enablement.
	 */
	.macro enable_branch_prediction
#if defined(CFG_ENABLE_SCTLR_Z)
		read_sctlr r0
		/* Some ARMv7 architectures need btac flush with post synchro */
		write_bpiall
		dsb
		isb
		orr	r0, r0, #SCTLR_Z
		write_sctlr r0
#endif
	.endm

	/*
	 * Setup sp to point to the top of the tmp stack for the current CPU:
	 * sp is assigned stack_tmp_export + cpu_id * stack_tmp_stride
	 * r0 is the current cpu_id
	 */
	.macro set_sp
		ldr	r1, =stack_tmp_stride
		ldr	r1, [r1]
		mul	r1, r0, r1
		ldr	r0, =stack_tmp_export
		ldr	r0, [r0]
		add	sp, r1, r0
	.endm

LOCAL_FUNC unhandled_cpu , :
UNWIND(	.fnstart)
	wfi
	b	unhandled_cpu
UNWIND(	.fnend)
END_FUNC unhandled_cpu

/*
 * void sta_cpu_resume(void)
 * Platform specific resume handling
 *
 * return value: none
 */
FUNC sta_cpu_resume, :
UNWIND(	.fnstart)
UNWIND(	.cantunwind)

	/* Early ARM secure MP specific configuration */
	bl	plat_cpu_reset_early

	set_sctlr
	isb

	ldr	r0, =resume_vect_table
	write_vbar r0

	bl	__get_core_pos
	cmp	r0, #CFG_TEE_CORE_NB_CORE
	/* Unsupported CPU, park it before it breaks something */
	bge	unhandled_cpu
	cmp	r0, #0
	bne	resume_secondary

	/* Restore cpu context */
	b	sm_pm_cpu_resume
UNWIND(	.fnend)
END_FUNC sta_cpu_resume
KEEP_PAGER sta_cpu_resume

LOCAL_FUNC resume_secondary , :
UNWIND(	.fnstart)
UNWIND(	.cantunwind)

	/* r0 is the current cpu_id */
	set_sp

	bl	plat_cpu_reset_late

#if defined (CFG_BOOT_SECONDARY_REQUEST)
	/* if L1 is not invalidated before, do it here */
	mov	r0, #DCACHE_OP_INV
	bl	dcache_op_level1
#endif

	bl	core_init_mmu_regs
	bl	cpu_mmu_enable
	bl	cpu_mmu_enable_icache
	bl	cpu_mmu_enable_dcache

	enable_branch_prediction

	/* generic_boot_core_hpen return value (r0) is ns entry point */
	bl	generic_boot_core_hpen

	bl	generic_boot_init_secondary

	mov	r0, #TEESMC_OPTEED_RETURN_ENTRY_DONE
	mov	r1, #0
	mov	r2, #0
	mov	r3, #0
	mov	r4, #0
	smc	#0
	b	.	/* SMC should not return */
UNWIND(	.fnend)
END_FUNC resume_secondary
KEEP_PAGER resume_secondary

