/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <gicv2.h>
#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include <xlat_tables_v2.h>
#include <sta_private.h>
#include <mailboxes.h>
#include <mbox.h>
#include <string.h>
#include <stdbool.h>

/*
 * Update PM definitions with extrem care.
 * Since this must stay aligned with what is defined
 * in the application processor side.
 */
#define PM_MSG_APXL_REBOOT	0xFFFF0001
#define PM_MSG_SUSPEND		0xFFFF0002
#define PM_MSG_SYSTEM_OFF	0xFFFF0003
#define PM_MSG_SYSTEM_RST	0xFFFF0004

/* mailbox channel id used to send remote message to C-M3 Firmware */
static uint32_t pm_msg_chan_id;

/*
 * Boolean to know whether we must send suspend request to remote sub-system
 * once in sta_pwr_domain_pwr_down_wfi() function.
 */
static bool sta_suspend_request;
static uintptr_t sta_sec_entrypoint;

struct mailbox_t __section(".mailboxes") mailboxes[PLATFORM_CORE_COUNT];

static inline void gicd_write_sgir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_SGIR, val);
}

/*******************************************************************************
 * Handler called when a CPU is about to enter standby.
 * Called by core 1 to enter in wfi
 ******************************************************************************/
static void sta_cpu_standby(plat_local_state_t cpu_state)
{
}

/*******************************************************************************
 * Inline function to send a Power Management request to the remote
 * micro-controller sub-system through hardware mailbox device.
 ******************************************************************************/
static inline void sta_pm_send_msg(uint32_t *request, uint32_t size)
{
	struct mbox_msg msg;

	msg.dsize = size;
	msg.pdata = (uint8_t *)request;
	if (mbox_send_msg(pm_msg_chan_id, &msg) != MBOX_SUCCESS)
		panic();
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned on. The mpidr
 * determines the CPU to be turned on.
 * Called by core 0 to activate core 1
 ******************************************************************************/
#define gicd_make_sgir(int_id, cpu_id) ((int_id) | 1 << (cpu_id) << 16)
mmap_reserve(MAP_ESRAM);
static int sta_pwr_domain_on(u_register_t mpidr)
{
	unsigned int linear_id = plat_core_pos_by_mpidr(mpidr);
	unsigned int request = gicd_make_sgir(WAKING_UP_SGI, linear_id);

	assert(IN_MMAP(MAILBOXES_BASE, MAP_ESRAM));
	mailboxes[linear_id].control_word = WAKE_UP_CONTROL_WORD;
	/* Flush control word and secure entrypoint. */
	flush_dcache_range((uintptr_t)&mailboxes[linear_id],
			   sizeof(mailboxes[linear_id]));
	gicd_write_sgir(GICD_BASE, request);
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void sta_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* Nothing to do */
}

/*******************************************************************************
 * Handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void sta_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/*
	 * Set suspend flag to true so that PM_MSG_SUSPEND will be sent
	 * to remote sub-system just before entering wfi() once in
	 * sta_pwr_domain_pwr_down_wfi() function.
	 */
	sta_suspend_request = true;

}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 * call by core 1 just after wake up
 ******************************************************************************/
static void sta_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned int linear_id = plat_core_pos_by_mpidr(read_mpidr_el1());

	mailboxes[linear_id].control_word = 0;
	flush_dcache_range((uintptr_t)&mailboxes[linear_id].control_word,
			   sizeof(mailboxes[linear_id].control_word));
	sta_gic_init();

	/* init generic timer */
	/* Set CNTFRQ because is not set before */
	write_cntfrq_el0(plat_get_syscnt_freq2());
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
static void sta_pwr_domain_suspend_finish(const psci_power_state_t
					    *target_state)
{
	mbox_resume();

#if defined(SCP_FILE2_BL2_BASE)
	{
	uint32_t *src, *dest, size;
		/*
		 * Restore the SCP_FILE2_BL2 image (aka the M3 OS file2 binary)
		 * that was initially saved during first cold boot by BL2.
		 * The 2 first words are used to get the physical address
		 * and the size of the image to be restored.
		 * See bl2_plat_handle_post_image_load() func for more details.
		 * Data cache still not enabled at this stage so no need to flush it.
		 */
		src = (uint32_t *)(M3OS_SAVED_BASE + 0x10);
		dest = (uint32_t *)mmio_read_32(M3OS_SAVED_BASE + 0x08);
		size = mmio_read_32(M3OS_SAVED_BASE + 0xC);
		memcpy((void *)dest, (void *)src, size);
	}
#endif
#if defined(SCP_BL2_BASE)
	uint32_t *src, *dest, size;

	/*
	 * Restore the SCP_BL2 image (aka the M3 OS binary) that was initially
	 * saved during first cold boot by BL2.
	 * The 2 first words are used to get the physical address
	 * and the size of the image to be restored.
	 * See bl2_plat_handle_post_image_load() func for more details.
	 * Data cache still not enabled at this stage so no need to flush it.
	 */
#if defined(SCP_FILE2_BL2_BASE)
	src = (uint32_t *)(M3OS_SAVED_BASE + 0x10 +
			   mmio_read_32(M3OS_SAVED_BASE + 0xC));
#else
	src = (uint32_t *)(M3OS_SAVED_BASE + 0x10);
#endif
	dest = (uint32_t *)mmio_read_32(M3OS_SAVED_BASE);
	size = mmio_read_32(M3OS_SAVED_BASE + 0x4);
	memcpy((void *)dest, (void *)src, size);

	/*
	 * Make use of C-M3 pen to unblock M3 Xloader so that it can
	 * jump into M3 OS entry code.
	 */
	set_m3_pen((void *)((uint32_t *)SCP_BL2_BASE)[1]);

	/*
	 * Wait for SCP (aka the C-M3 micro-ctrl sub-system) to set sync point
	 * announcing it reached a state so that it's safe to start the AP OS.
	 * This is to make sure critical SCP init code execute before
	 * start of AP OS.
	 */
#endif /* SCP_BL2_BASE */

	sta_scp_wait_sync();

	/* Reset platform suspend flag. */
	sta_suspend_request = false;
}

extern void sta_resume_from_suspend(void);

/*******************************************************************************
 * Handler called at cores power down time.
 *
 * Whenever the application OS wants to suspend:
 *
 * - this function is firstly called by the secondary core following a initial
 *   CPU OFF PSCI request. Since both cores lives in the same power island,
 *   we cannot switch off one core while keeping the other one alive1.
 *   So we just make secondary core enter wfi.
 *
 * - Then, whenever PSCI SYSTEM SUSPEND is finally received, this function is
 *   called again this time by the primary core which will post the
 *   PM_MSG_SUSPEND message to the remote micro-controller sub-system
 *   that will effectively put the system in the suspended state.
 *
 ******************************************************************************/
static void __dead2 sta_pwr_domain_pwr_down_wfi(const psci_power_state_t
						  *target_state)
{
	uint32_t data[2];
	int i;

	/*
	 * Send the system suspend request to remote micro-controller
	 * sub-system and wait forever
	 */
	if (sta_suspend_request) {
		for (int i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++) {
			if (target_state->pwr_domain_state[i] !=
			    PLAT_MAX_OFF_STATE)
				/*
				 * We expect all target state to reach
				 * PLAT_MAX_OFF_STATE before effectively
				 * suspending the system.
				 */
				panic();
		}

		/* FIXME: Why is this required ? */
		for (i = PLATFORM_CORE_COUNT; i--;) {
			mailboxes[i].sec_entrypoint = sta_sec_entrypoint;
		}

		/* flush L1/L2 data caches */
		dcsw_op_all(DC_OP_CISW);
		/*
		 * Set the address of platform low level resume entry function
		 * at magic address in backup ram so that C-M3 bootloader can
		 * read it at system resume time and figure out entry code
		 * where C-A7 should resume.
		 */
		data[0] = PM_MSG_SUSPEND;
		data[1] = (uint32_t)sta_resume_from_suspend;
		sta_pm_send_msg(&data[0], sizeof(data));
	}
	wfi();
	while (1);
}

static void __dead2 sta_system_off(void)
{
	uint32_t data = PM_MSG_SYSTEM_OFF;
	sta_pm_send_msg(&data, sizeof(uint32_t));
	while (1);
}

static void __dead2 sta_system_reset(void)
{
	uint32_t data = PM_MSG_SYSTEM_RST;
	sta_pm_send_msg(&data, sizeof(uint32_t));
	while (1);
}

static int sta_validate_power_state(unsigned int power_state,
				    psci_power_state_t *req_state)
{
	return PSCI_E_SUCCESS;
}

static int sta_validate_ns_entrypoint(uintptr_t entrypoint)
{
	return PSCI_E_SUCCESS;
}

static int sta_node_hw_state(u_register_t target_cpu,
			     unsigned int power_level)
{
	return HW_ON;
}

static void sta_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

/*******************************************************************************
 * Export the platform handlers. The ARM Standard platform layer will take care
 * of registering the handlers with PSCI.
 ******************************************************************************/
static const plat_psci_ops_t sta_psci_ops = {
	.cpu_standby = sta_cpu_standby,
	.pwr_domain_on = sta_pwr_domain_on,
	.pwr_domain_off = sta_pwr_domain_off,
	.pwr_domain_suspend = sta_pwr_domain_suspend,
	.pwr_domain_on_finish = sta_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = sta_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi = sta_pwr_domain_pwr_down_wfi,
	.system_off = sta_system_off,
	.system_reset = sta_system_reset,
	.validate_power_state = sta_validate_power_state,
	.validate_ns_entrypoint = sta_validate_ns_entrypoint,
	.get_node_hw_state = sta_node_hw_state,
	.get_sys_suspend_power_state = sta_get_sys_suspend_power_state,
};


/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	unsigned int i, ret;
	struct mbox_chan_req req;

	ret = mbox_init();
	if (ret)
		return ret;

	req.chan_name = "psci";
	req.user_data = NULL;
	req.rx_cb = NULL;

	pm_msg_chan_id = mbox_request_channel(&req);
	if (pm_msg_chan_id < 0)
		return pm_msg_chan_id;

	sta_sec_entrypoint = sec_entrypoint;

	for (i = PLATFORM_CORE_COUNT; i--;)
		mailboxes[i].sec_entrypoint = sec_entrypoint;
	*psci_ops = &sta_psci_ops;

	NOTICE("%s: sec_entrypoint = 0x%lx\n", __func__, sec_entrypoint);
	return 0;
}
