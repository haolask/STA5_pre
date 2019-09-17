/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <desc_image_load.h>
#ifdef AARCH32_SP_OPTEE
#include <optee_utils.h>
#endif
#include <generic_delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include <string.h>
#include <xlat_tables_v2.h>
#include <qspi_sta.h>

#include "sta_fdt.h"
#include "sta_private.h"

#if defined(AARCH32_SP_OPTEE)
/*******************************************************************************
 * Set image information according to Optee header paramaters.
 ******************************************************************************/
static void set_mem_params_info(entry_point_info_t *ep_info,
				image_info_t *unpaged, image_info_t *paged)
{
	uintptr_t bl32_ep = 0;

	/*
	 * Keep default sram setup if unpaged ep is out of BL32_EXTRA area
	 * reserved in TZDRAM.
	 */
	if (get_optee_header_ep(ep_info, &bl32_ep) &&
	    bl32_ep >= BL32_EXTRA_BASE &&
	    bl32_ep < (BL32_EXTRA_BASE + BL32_EXTRA1_RESERVED_SIZE)) {
		/* Change images info to BL32_EXTRA mem area setup */
		unpaged->image_base = BL32_EXTRA_BASE;
		unpaged->image_max_size = BL32_EXTRA1_RESERVED_SIZE;

		paged->image_base = BL32_EXTRA_BASE + BL32_EXTRA1_RESERVED_SIZE;
		paged->image_max_size = BL32_EXTRA2_RESERVED_SIZE;
	}
}
#endif

mmap_reserve(MAP_CONSOLE);
void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	console_init(CONSOLE_BASE, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);
	/* init generic timer */
	/* Set CNTFRQ because is not set before */
	write_cntfrq_el0(plat_get_syscnt_freq2());
	generic_delay_timer_init();
	/* Initialize the console to provide early debug support */
	assert(IN_MMAP(CONSOLE_BASE, MAP_CONSOLE));
}

/*******************************************************************************
 * Perform platform specific setup.
 ******************************************************************************/
void bl2_platform_setup(void)
{
#if defined(AARCH32_SP_OPTEE)
	INFO("OPTEE support\n");
#else
	INFO("SP_MIN support\n");
#endif
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl2_el3_plat_arch_setup(void)
{
	VERBOSE("BL2 Code region: %p - %p\n",
		(void *)BL_CODE_BASE, (void *)BL_CODE_END);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

#if SEPARATE_CODE_AND_RODATA
	VERBOSE("BL2 RO data region: %p - %p\n",
		(void *)BL_RO_DATA_BASE, (void *)BL_RO_DATA_LIMIT);
	mmap_add_region(BL_RO_DATA_BASE, BL_RO_DATA_BASE,
			BL_RO_DATA_LIMIT - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE);
#endif

	sta_configure_mmu();

	/* Initialize the secure environment */
	sta_security_setup();

	/* Initialize IO here */
	sta_io_setup();
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#ifdef AARCH32_SP_OPTEE
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif

	assert(bl_mem_params);

	switch (image_id) {
	case BL32_IMAGE_ID:
#ifdef AARCH32_SP_OPTEE
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		set_mem_params_info(&bl_mem_params->ep_info,
				    &pager_mem_params->image_info,
				    &paged_mem_params->image_info);

		err = parse_optee_header(&bl_mem_params->ep_info,
					 &pager_mem_params->image_info,
					 &paged_mem_params->image_info);
		if (err != 0)
			WARN("OPTEE header parse error.\n");

		bl_mem_params->ep_info.spsr = sta_get_spsr_for_bl32_entry();
		bl_mem_params->ep_info.lr_svc = plat_get_ns_image_entrypoint();
#if STA_DIRECT_LINUX_BOOT
		/* See bootargs_entry macro in optee generic_entry_a32.S */
		bl_mem_params->ep_info.args.arg1 = ~0U;
		bl_mem_params->ep_info.args.arg2 = (u_register_t) DTB_BASE;
#endif /* STA_DIRECT_LINUX_BOOT */

#endif /* AARCH32_SP_OPTEE */
		break;

#ifdef SCP_BL2_BASE
#ifdef SCP_FILE2_BL2_BASE
	case SCP_FILE2_BL2_IMAGE_ID:
		/*
		 * Copy M3 OS file2 binary in Trusted DDR area so that it can be
		 * restored by ATF following a system resume.
		 * The 2 first words are used to store the physical address
		 * and the size of the image. So that it can be read at resume
		 * time to figure out where to restore it.
		 * See sta_pwr_domain_suspend_finish() func in sta_pm.c.
		 */
		mmio_write_32(M3OS_SAVED_BASE + 0x08,
			      bl_mem_params->image_info.image_base);
		mmio_write_32(M3OS_SAVED_BASE + 0x0C,
			      bl_mem_params->image_info.image_size);
		assert(bl_mem_params->image_info.image_size <= M3OS_SAVED_SIZE);
		memcpy((void *)(M3OS_SAVED_BASE + 0x10), (void *)SCP_FILE2_BL2_BASE,
		       bl_mem_params->image_info.image_size);

		break;

#endif /* SCP_FILE2_BL2_BASE */
	case SCP_BL2_IMAGE_ID:
		/*
		 * Copy M3 OS binary in Trusted DDR area so that it can be
		 * restored by ATF following a system resume.
		 * The 2 first words are used to store the physical address
		 * and the size of the image. So that it can be read at resume
		 * time to figure out where to restore it.
		 * See sta_pwr_domain_suspend_finish() func in sta_pm.c.
		 */
		mmio_write_32(M3OS_SAVED_BASE,
			      bl_mem_params->image_info.image_base);
		mmio_write_32(M3OS_SAVED_BASE + 0x4,
			      bl_mem_params->image_info.image_size);
#ifdef SCP_FILE2_BL2_BASE
		assert((bl_mem_params->image_info.image_size
		        + mmio_read_32(M3OS_SAVED_BASE + 0x0C)) <= M3OS_SAVED_SIZE);
		memcpy((void *)(M3OS_SAVED_BASE + 0x10 +
				mmio_read_32(M3OS_SAVED_BASE + 0x0C)),
#else
		assert(bl_mem_params->image_info.image_size <= M3OS_SAVED_SIZE);
		memcpy((void *)(M3OS_SAVED_BASE + 0x10),
#endif
		       (void *)SCP_BL2_BASE,
		       bl_mem_params->image_info.image_size);

		/*
		 * M3 entry is the reset vector: containt of shadow_address+4
		 * use pen to unblock M3 Xloader to jump into M3 OS
		 */
		set_m3_pen((void *)((uint32_t *)SCP_BL2_BASE)[1]);
		break;
#endif /* SCP_BL2_BASE */

	case BL32_EXTRA1_IMAGE_ID:
	case BL32_EXTRA2_IMAGE_ID:
		break;

	case BL33_IMAGE_ID:
		bl_mem_params->ep_info.args.arg0 = 0;
#if STA_DIRECT_LINUX_BOOT
		/*
		 * According to the file ``Documentation/arm/Booting``
		 * of the Linux kernel tree, Linux expects:
		 * r0 = 0
		 * r1 = machine type number, optional in DT-only platforms (~0 if so)
		 * r2 = Physical address of the device tree blob
		 */
		VERBOSE("STA: Preparing to boot 32-bit Linux kernel\n");
		bl_mem_params->ep_info.args.arg1 = ~0U;
		bl_mem_params->ep_info.args.arg2 = (u_register_t) DTB_BASE;
		/*
		 * Wait for SCP (aka the Cortex-M3 sub-system)
		 * to set sync point announcing it reached a state
		 * so that it's safe to start the AP OS.
		 * This is to make sure critical SCP init code execute before
		 * start of AP OS.
		 */
		sta_scp_wait_sync();
		/* Here, we are able to patch DTB following Board & SoC */
		err = sta_update_fdt((void *)DTB_BASE, DTB_SIZE);
		if (err)
			ERROR("Can't update DTB\n");
		break;

	case NT_FW_CONFIG_ID:
#else
		bl_mem_params->ep_info.spsr =
			SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE,
				    DISABLE_ALL_EXCEPTIONS);
#endif
		break;

	default:
		VERBOSE("Image id=%d not managed by BL2\n", image_id);
		err = -1;
		break;
	}

	return err;
}

/*******************************************************************************
 * Before calling this function BL33 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL33 and set SPSR and security state.
 * On ARM standard platforms we only set the security state of the entrypoint
 ******************************************************************************/
void bl2_plat_set_bl33_ep_info(image_info_t *image,
			       entry_point_info_t *bl33_ep_info)
{
	SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
	bl33_ep_info->spsr = sta_get_spsr_for_bl33_entry();
}

/*******************************************************************************
 * Populate the extents of memory available for loading BL33
 ******************************************************************************/
void bl2_plat_get_bl33_meminfo(meminfo_t *bl33_meminfo)
{
	bl33_meminfo->total_base = BL33_BASE;
	bl33_meminfo->total_size = BL33_SIZE;
}

