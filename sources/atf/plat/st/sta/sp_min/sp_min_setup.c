/*
 * Copyright (c) 2017, STMicroelectronics - All Rights Reserved
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <platform_sp_min.h>
#include <sta_private.h>
#include <string.h>
#include <xlat_tables_v2.h>

/******************************************************************************
 * Placeholder variables for copying the arguments that have been passed to
 * BL32 from BL2.
 ******************************************************************************/
static entry_point_info_t bl33_image_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info;

	next_image_info = &bl33_image_ep_info;

	if (next_image_info->pc == 0U) {
		return NULL;
	}

	return next_image_info;
}

/*******************************************************************************
 * Perform any BL3-2 specific platform actions.
 ******************************************************************************/
mmap_reserve(MAP_CONSOLE);
void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;

	/* Imprecise aborts can be trapped by NonSecure */
	write_scr(read_scr() | SCR_AW_BIT);

	/* Initialize the console to provide early debug support */
	assert(IN_MMAP(CONSOLE_BASE, MAP_CONSOLE));
	console_init(CONSOLE_BASE, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	/* Check params passed from BL2 should not be NULL */
	assert(params_from_bl2);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params != NULL) {
		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
			break;
		}

		bl_params = bl_params->next_params_info;
	}
}

/*******************************************************************************
 * Secure FIQ handler. To be completed according to the need
 ******************************************************************************/
void sp_min_plat_fiq_handler(uint32_t id)
{
	VERBOSE("[sp_min] interrupt #%d\n", id);
}

/*******************************************************************************
 * Initialize the MMU, security and the GIC.
 ******************************************************************************/
void sp_min_platform_setup(void)
{
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	sta_configure_mmu();

	/* init generic timer */
	generic_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	sta_gic_driver_init();
	sta_gic_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void sp_min_plat_arch_setup(void)
{
}
