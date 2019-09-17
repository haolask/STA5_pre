/*
 * Copyright (c) 2014-2016, STMicroelectronics International N.V.
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
#include <console.h>
#include <kernel/cache_helpers.h>
#include <kernel/generic_boot.h>
#include <kernel/thread.h>
#include <kernel/misc.h>
#include <kernel/panic.h>
#include <initcall.h>
#include <stdint.h>
#include <string.h>
#include <io.h>
#include <sm/optee_smc.h>
#include <sm/psci.h>
#include <sm/pm.h>
#include <sm/sm.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <sta_helper.h>
#include <mbox.h>
#include <trace.h>
#ifdef CFG_CRYPTO_WITH_HSM
#include <CSE_HAL.h>
#endif

/*
 * BE CAREFUL to align PM_MSG_XXX definitions with what is defined
 * in sta_pm.c driver from remote processor side.
 */
#define PM_MSG_APXL_REBOOT	0xFFFF0001
#define PM_MSG_SUSPEND		0xFFFF0002
#define PM_MSG_SYSTEM_OFF	0xFFFF0003
#define PM_MSG_SYSTEM_RST	0xFFFF0004

/* CPU affinity state */
#define PSCI_CPU_STATE_ON	1
#define PSCI_CPU_STATE_OFF	0

/* mailbox channel id used to send remote message to C-M3 Firmware */
static uint32_t pm_msg_chan_id;

/* CPU affinity state expected for PSCI_AFFINITY_INFO order */
static uint32_t psci_cpu_status[CFG_TEE_CORE_NB_CORE];

/*******************************************************************************
 * Inline function to send a Power Management request to remote processor
 * through hardware mailbox device.
 ******************************************************************************/
static inline void sta_pm_send_msg(void *request, uint32_t size)
{
	struct mbox_msg msg;

	msg.dsize = size;
	msg.pdata = (uint8_t *)request;
	if (mbox_send_msg(pm_msg_chan_id, &msg) != MBOX_SUCCESS)
		panic("mbox message sending failed");
}

/******************************************************************************
 * Wait for SCP (aka the C-M3 micro-ctrl sub-system) to set sync point
 * announcing it reached a state so that it's safe to start the AP OS.
 * This is to make sure critical SCP init code execute before
 * start of AP OS.
 ******************************************************************************/
static void sta_scp_wait_sync(void)
{
	while (!get_sync_point())
		;
}

/*******************************************************************************
 * Handler called at cores suspend time.
 *
 * whenever PSCI SYSTEM SUSPEND is received, this function is called by
 * the primary core which will post the PM_MSG_SUSPEND message to the
 * remote micro-controller sub-system that will effectively put the system
 * in the suspended state.
 ******************************************************************************/
static int sta_cpu_suspend(uint32_t arg __unused)
{
	uint32_t data[2];

	/*
	 * Set the address of platform low level resume entry function
	 * at magic address in backup ram so that C-M3 bootloader can
	 * read it at system resume time and figure out entry code where C-A7
	 * should resume.
	 */
	data[0] = PM_MSG_SUSPEND;
	data[1] = (uint32_t)sta_cpu_resume;

	/*
	 * Send the system suspend request to remote micro-controller
	 * sub-system and wait forever
	 */
	sta_pm_send_msg(&data[0], sizeof(data));

	while (true)
		wfi();

	return PSCI_RET_INTERNAL_FAILURE;
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * having been suspended earlier.
 ******************************************************************************/
static void sta_cpu_resume_finish(void)
{
#ifdef SCP_BL2_BASE
	vaddr_t addr, src, dest;
	uint32_t size;
#endif

	/* Prepare the CPU banked registers for resume into normal world */
	plat_cpu_resume_reset_late();

	/* Restore mbox registers */
	mbox_resume();

#ifdef CFG_CRYPTO_WITH_HSM
	/* eHSM re-initialization */
	hsm_resume();
#endif

#if defined(M3OS_SAVED_BASE) && defined(SCP_BL2_BASE)
	addr = (vaddr_t)phys_to_virt(M3OS_SAVED_BASE, MEM_AREA_RAM_SEC);

#ifdef SCP_FILE2_BL2_BASE
	/*
	 * Restore the SCP_FILE2_BL2 image (aka the M3 OS file2 binary)
	 * that was initially saved during first cold boot by ATF BL2.
	 * The 2 first words are used to get the physical address
	 * and the size of the image to be restored.
	 * Destination memory area is not cacheable so no need to flush it.
	 */
	src = addr + 0x10;
	dest = (vaddr_t)phys_to_virt_io(
				(paddr_t)ALIAS_TO_PHYS(read32(addr + 0x08)));
	size = read32(addr + 0xC);
	memcpy((void *)dest, (void *)src, size);
#endif
	/*
	 * Restore the SCP_BL2 image (aka the M3 OS binary) that was initially
	 * saved during first cold boot by ATF BL2.
	 * The 2 first words are used to get the physical address
	 * and the size of the image to be restored.
	 * Destination memory area is not cacheable so no need to flush it.
	 */
	src = addr + 0x10;
#ifdef SCP_FILE2_BL2_BASE
	src += size;
#endif
	dest = (vaddr_t)phys_to_virt_io(
				(paddr_t)ALIAS_TO_PHYS_M3(read32(addr)));
	size = (vaddr_t)read32(addr + 0x4);
	memcpy((void *)dest, (void *)src, size);

	/*
	 * Make use of C-M3 pen to unblock M3 Xloader so that it can
	 * jump into M3 OS entry code.
	 * Set the C-M3 pen with the M3_OS start address.
	 */
	set_m3_pen((void *)read32(dest + 0x4));

	/*
	 * Wait for SCP (aka the C-M3 micro-ctrl sub-system) to set sync point
	 * announcing it reached a state so that it's safe to start the AP OS.
	 * This is to make sure critical SCP init code execute before
	 * start of AP OS.
	 */
	sta_scp_wait_sync();
#endif /* SCP_BL2_BASE */

	/* Recover CPU mode in progress at suspend state */
	set_cpsr_mode_mon();
	isb();
}

/*******************************************************************************
 * PSCI fonctions supported by sta platform.
 ******************************************************************************/
uint32_t psci_version(void)
{
	return PSCI_VERSION_1_0;
}

int psci_features(uint32_t psci_fid)
{
	switch (psci_fid) {
	case PSCI_PSCI_FEATURES:
	case PSCI_VERSION:
	case PSCI_CPU_ON:
	case PSCI_CPU_OFF:
	case PSCI_AFFINITY_INFO:
	case PSCI_SYSTEM_SUSPEND:
	case PSCI_SYSTEM_OFF:
	case PSCI_SYSTEM_RESET:
		return PSCI_RET_SUCCESS;
	default:
		return PSCI_RET_NOT_SUPPORTED;
	}
}

int psci_cpu_on(uint32_t core_id, uint32_t entry, uint32_t context_id __unused)
{
#if defined(CFG_BOOT_SECONDARY_REQUEST)
	size_t pos = get_core_pos_mpidr(core_id);

	DMSG("core pos: %zu: ns_entry %#" PRIx32, pos, entry);

	if (!generic_boot_core_release(pos, (paddr_t)(entry))) {
		psci_cpu_status[pos] = PSCI_CPU_STATE_ON;
		return PSCI_RET_SUCCESS;
	} else {
		return PSCI_RET_INVALID_PARAMETERS;
	}
#else
	return PSCI_RET_NOT_SUPPORTED;
#endif
}

int psci_cpu_off(void)
{
	uint32_t core_id = get_core_pos();

	if (!core_id || core_id >= CFG_TEE_CORE_NB_CORE)
		return PSCI_RET_INVALID_PARAMETERS;

	/* TODO */
	//DMSG("core_id: %" PRIu32, core_id);

	psci_armv7_cpu_off();

	thread_mask_exceptions(THREAD_EXCP_ALL);

	psci_cpu_status[core_id] = PSCI_CPU_STATE_OFF;

	while (true)
		wfi();

	return PSCI_RET_INTERNAL_FAILURE;
}

int psci_affinity_info(uint32_t affinity,
		       uint32_t lowest_affnity_level __unused)
{
	uint32_t core_id = get_core_pos_mpidr(affinity);
#if 0
	uint32_t wfi_mask = CORE_WFI_MASK(core_id);
	vaddr_t va_base = (vaddr_t)phys_to_virt_io(MSCR_BASE);

	if (core_id >= CFG_TEE_CORE_NB_CORE)
		return PSCI_RET_INVALID_PARAMETERS;

	DMSG("core_id: %" PRIu32 " STATUS: %" PRIx32 " MASK: %" PRIx32,
	     core_id, read32(va_base + MSCR_A7SS_REG), wfi_mask);

	return (read32(va_base + MSCR_A7SS_REG) & wfi_mask) ?
		PSCI_AFFINITY_LEVEL_OFF : PSCI_AFFINITY_LEVEL_ON;
#else
	if (core_id >= CFG_TEE_CORE_NB_CORE)
		return PSCI_RET_INVALID_PARAMETERS;

	return psci_cpu_status[core_id] == PSCI_CPU_STATE_ON ?
		PSCI_AFFINITY_LEVEL_ON : PSCI_AFFINITY_LEVEL_OFF;
#endif
}

int psci_system_suspend(uintptr_t entry, uint32_t context_id __unused,
			struct sm_nsec_ctx *nsec)
{
	int ret = PSCI_RET_INVALID_PARAMETERS;

	/* TODO */
	//DMSG("System suspend request\n");

	/* Store non-sec ctx regs */
	sm_save_modes_regs(&nsec->mode_regs);

	/*
	 * Pass platform specific suspend function
	 */
	ret = sm_pm_cpu_suspend(0, sta_cpu_suspend);
	/*
	 * Sometimes sm_pm_cpu_suspend may not really suspended,
	 * we need to check it's return value to restore reg or not
	 */
	if (ret < 0) {
		DMSG("=== Not suspended ===\n");
		return 0;
	}

	/* Platform init after system resume */
	sta_cpu_resume_finish();

	/* Restore register of different mode in secure world */
	sm_restore_modes_regs(&nsec->mode_regs);

	/* Set entry for back to Linux */
	nsec->mon_lr = (uint32_t)entry;

	main_init_gic();

	DMSG("=== Back from Suspended ===\n");

	return 0;
}

void psci_system_off(void)
{
	uint32_t data = PM_MSG_SYSTEM_OFF;

	sta_pm_send_msg(&data, sizeof(uint32_t));
}

void psci_system_reset(void)
{
	uint32_t data = PM_MSG_SYSTEM_RST;

	sta_pm_send_msg(&data, sizeof(uint32_t));
}

static TEE_Result psci_init(void)
{
	unsigned int i, ret;
	struct mbox_chan_req req;
	vaddr_t addr;

	ret = mbox_init();
	if (ret)
		return ret;

	req.chan_name = "psci";
	req.user_data = NULL;
	req.rx_cb = NULL;

	pm_msg_chan_id = mbox_request_channel(&req);
	if (pm_msg_chan_id < 0)
		panic("PSCI mbox channel request failed");

	return TEE_SUCCESS;
}
driver_init(psci_init);

