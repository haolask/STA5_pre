/*
 *  arch/arm/mach-sta/sleep.S
 *
 * Copyright (C) 2016 STMicroelectronics (R&D) Limited.
 *		http://www.st.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>

ENTRY(sta_finish_suspend)
	bl	v7_flush_dcache_all
	/**
	 * Send PM_SUSPEND message to the micro-controller
	 * sub-system through the mailbox.
	 */
	bl	sta_send_suspend_request
	cmp	r0, #0
	beq	sleep
	bx	lr
sleep:
	/**
	 * Wait for uCtrl sub-system to move SoC to standby state
	 */
	wfi
	b	.
ENDPROC(sta_finish_suspend)

ENTRY(sta_cpu_resume)
	bl	v7_invalidate_l1
	b	cpu_resume
ENDPROC(sta_cpu_resume)
