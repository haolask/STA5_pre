/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console.h>
#include <debug.h>
#include <libfdt.h>
#include <string.h>

#include "memory_map.h"
#include "shared_data.h"
#include "sta_fdt.h"

/**
 * fdt_find_and_setprop: Find a node and set it's property
 *
 * @fdt: ptr to device tree
 * @node: path of node
 * @prop: property name
 * @val: ptr to new value
 * @len: length of new property value
 * @create: flag to create the property if it doesn't exist
 *
 * Convenience function to directly set a property given the path to the node.
 */
static int fdt_find_and_setprop(void *fdt, const char *node, const char *prop,
				const void *val, int len, int create)
{
	int off = fdt_path_offset(fdt, node);

	if (off < 0)
		return off;

	if ((!create) && (fdt_get_property(fdt, off, prop, NULL) == NULL))
		return 0; /* create flag not set; so exit quietly */

	return fdt_setprop(fdt, off, prop, val, len);
}

/*******************************************************************************
 * Patch DTB following STA Board & SoC
 ******************************************************************************/
static int sta_fdt_setup(void *fdt)
{
	int off, off2;
	uint8_t display_cfg;
	const char *old_mod_name;
	const char *dtb_mmc_path;
	const char *dtb_nand_path;

	if (get_board_id() == BOARD_TC3_EVB) {
		char mod_name[64] = {"ST TELEMACO3 STA1195 EValuation Board"};
		fdt_setprop(fdt, 0, "model", mod_name, strlen(mod_name) + 1);
	}

	old_mod_name = fdt_getprop(fdt, 0, "model", NULL);
	if (old_mod_name) {
		char mod_name[64] = {0};
		int len;
		snprintf(mod_name, sizeof(mod_name), "%s rev", old_mod_name);
		len = strlen(mod_name);
		mod_name[len++] = get_board_rev_id()+'A';
		mod_name[len] = 0;
		fdt_setprop(fdt, 0, "model", mod_name, strlen(mod_name) + 1);
	}

	/* Set DDR memory base & size (BL33) */
	off = fdt_path_offset(fdt, "/memory");
	if (off < 0) {
		ERROR("Missing /memory node\n");
		return -1;
	} else {
		struct fdt_property *prop;
		uint32_t *reg;
		int len;

		prop = fdt_get_property_w(fdt, off, "reg", &len);
		if (prop) {
			reg = (uint32_t *)&prop->data[0];
			reg[0] = cpu_to_fdt32(BL33_BASE);
			reg[1] = cpu_to_fdt32(BL33_SIZE);
		}
	}

#ifdef EARLY_TUNER_FTR // FIXME not yet defined
	off = fdt_path_offset(fdt, "/soc/i2c@50040000");
	if (off >= 0) {
		fdt_setprop_string(fdt, off, "status", "disabled");
		NOTICE("i2c1 disabled in dtb for early tuner usage\n");
	} else {
		NOTICE("failed to disable i2c1\n");
	}

#endif

	dtb_nand_path = "/soc/flash@80000000";
	switch (get_soc_id()) {
		case SOCID_STA1385:
			dtb_mmc_path = "/soc/sdi@50070000";
			break;
		default:
			dtb_mmc_path = "/soc/sdi@500B0000";
			break;
	}

	if (get_boot_dev_type() == BOOT_MMC) {
		/* eMMC is enabled and FSMC NAND disabled */
		off = fdt_path_offset(fdt, dtb_mmc_path);
		if (off >= 0) {
			fdt_setprop_string(fdt, off, "status", "okay");
			NOTICE("dtb: eMMC enabled\n");
		}
		off = fdt_path_offset(fdt, dtb_nand_path);
		if (off >= 0)
			fdt_setprop_string(fdt, off, "status", "disabled");
		if (get_soc_id() == SOCID_STA1385) {
			/* ethernet1 is conflicting with eMMC on sta1385-mtp-mmc machine */
			off = fdt_path_offset(fdt, "/soc/ethernet1");
			fdt_setprop_string(fdt, off, "status", "disabled");
		}
	} else if (get_boot_dev_type() == BOOT_NAND) {
		/* Nand is enabled and eMMC disabled */
		off = fdt_path_offset(fdt, dtb_nand_path);
		if (off >= 0) {
			fdt_setprop_string(fdt, off, "status", "okay");
			NOTICE("dtb: FSMC NAND enabled\n");
		}
		off = fdt_path_offset(fdt, dtb_mmc_path);
		if (off >= 0)
			fdt_setprop_string(fdt, off, "status", "disabled");
	}

#ifdef RAMOOPS_BASE
	/* RAMOOPS feature */
	off = fdt_path_offset(fdt, "/reserved-memory/ramoops");
	if (off >= 0) {
		struct fdt_property *prop;
		uint32_t *reg;
		int len;

		prop = fdt_get_property_w(fdt, off, "reg", &len);
		if (prop) {
			reg = (uint32_t *)&prop->data[0];
			reg[0] = cpu_to_fdt32(RAMOOPS_BASE);
			reg[1] = cpu_to_fdt32(RAMOOPS_SIZE);
		}
	}
#endif /* RAMOOPS_BASE */

#ifdef RAMDUMP_BASE
	/* RAMDUMP feature */
	{
		const void *magic_addr = (const void *)cpu_to_fdt32(RAMDUMP_BASE);
		fdt_find_and_setprop(fdt, "/reserved-memory", "ramdump-magic-addr",
				     &magic_addr, sizeof(uint32_t), 1);
	}
#endif /* RAMDUMP_BASE */

#ifdef APP_OS_MAILBOXES_BASE
	/* Update IPC mailbox mapping */
	off = fdt_path_offset(fdt, "/soc/mailbox@481a0000");
	if (off >= 0) {
		struct fdt_property *prop;
		int len;
		int index;
		uint32_t *reg, *reg2;

		index = fdt_stringlist_search(fdt, off, "reg-names", "mbox-shm");
		if (index >= 0) {
			prop = fdt_get_property_w(fdt, off, "reg", &len);
			if (prop) {
				int idx;
				int tuple_size = 2 * sizeof(reg);

				reg = reg2 = (uint32_t *)&prop->data[0];
				for (idx = 0; idx < (len / tuple_size); idx++) {
					/* Update address and size in reg property */
					if (idx == index) {
						reg[0] = cpu_to_fdt32(APP_OS_MAILBOXES_BASE);
						reg[1] = cpu_to_fdt32(APP_OS_MAILBOXES_SIZE);
					}

					/* Point to next reg tuple */
					reg += 2;
				}

				fdt_setprop(fdt, off, "reg", reg2, len);
			}
		}
	}
#endif /* APP_OS_MAILBOXES_BASE */

	if (get_board_id() == BOARD_A5_EVB) {
#ifdef GCNANO_POOL_BASE
		/* Update GCNANO pool mapping */
		off = fdt_path_offset(fdt, "/soc/gcnano@48F00000");
		if (off >= 0) {
			struct fdt_property *prop;
			int len;
			int index;
			uint32_t *reg, *reg2;

			index = fdt_stringlist_search(fdt, off, "reg-names", "contig_baseaddr");
			if (index >= 0) {
				prop = fdt_get_property_w(fdt, off, "reg", &len);
				if (prop) {
					int idx;
					int tuple_size = 2 * sizeof(reg);

					reg = reg2 = (uint32_t *)&prop->data[0];
					for (idx = 0; idx < (len / tuple_size); idx++) {
						/* Update address and size in reg property */
						if (idx == index) {
							reg[0] = cpu_to_fdt32(GCNANO_POOL_BASE);
							reg[1] = cpu_to_fdt32(GCNANO_POOL_SIZE);
						}

						/* Point to next reg tuple */
						reg += 2;
					}

					fdt_setprop(fdt, off, "reg", reg2, len);
				}
			}
		}
#endif /* GCNANO_POOL_BASE */

		/* Display Management */
		display_cfg = get_display_cfg();
		off2 = fdt_path_offset(fdt, "/soc/panel-rgb1");
		if (off2 < 0) {
			NOTICE("Error to get panel-rgb1\n");
		} else {
			switch (display_cfg) {
			case NO_DETECTION:
			case SINGLE_WVGA:
				/* Nothing to do */
				break;
			case DUAL_WVGA:
				/* enable panel_rgb2 */
				off = fdt_path_offset(fdt, "/soc/panel-rgb2");
				if (off >= 0) {
					fdt_setprop_string(fdt, off, "status", "okay");
					NOTICE("panel-rgb2 enabled\n");
				} else
					NOTICE("Error to enable panel-rgb2\n");
				break;
			case SINGLE_720P:
			case SINGLE_HYBRID_CLUSTER_720P:
			case SINGLE_720P_10INCHES:
				/* modify panel_rgb1 for 720p panel */
				if (fdt_setprop_string(fdt, off2, "compatible", "ampire,am-1280800p2tzqw-t01h"))
					printf("Error to set panel-rgb1 to 720p ampire\n");
				else
					NOTICE("set panel-rgb1 to 720p ampire\n");
				break;
			case SINGLE_CLUSTER:
				/* select panel_rgb1 for cluster chunghwa panel */
				if (fdt_setprop_string(fdt, off2, "compatible", "chunghwa,claa103wa02xn"))
					printf("Error to set panel-rgb1 to cluster chunghwa\n");
				else
					NOTICE("set panel-rgb1 to cluster chunghwa\n");
				break;
			case SINGLE_CLUSTER_HD:
				/* select panel_rgb1 for cluster HD chunghwa panel */
				if (fdt_setprop_string(fdt, off2, "compatible", "chunghwa,claa123fca1xn"))
					printf("Error to set panel-rgb1 to cluster HD chunghwa\n");
				else
					NOTICE("set panel-rgb1 to cluster HD chunghwa\n");
				break;
			case SINGLE_HYBRID_CLUSTER_WVGA:
				/* select panel_rgb1 for cluster WVGA ampire panel */
				if (fdt_setprop_string(fdt, off2, "compatible", "ampire,am-800480aztmqw-54h"))
					printf("Error to set panel-rgb1 to cluster WVGA ampire\n");
				else
					NOTICE("set panel-rgb1 to cluster WVGA ampire\n");
				break;
			default:
				break;
			}
		}

		/* Touchscreen Management */
		/* In case we are not using a WVGA rocktech single/dual display,
		 * disabled ft5336 touchscreen */
		if (!((display_cfg == NO_DETECTION) || (display_cfg == SINGLE_WVGA) ||
		      (display_cfg == DUAL_WVGA))) {
			/* disable ft5336 WVGA touchscreen */
			off = fdt_path_offset(fdt, "/soc/i2c/ft5336");
			if (off >= 0) {
				fdt_setprop_string(fdt, off, "status", "disabled");
				NOTICE("ft5336 disabled\n");
			} else
				NOTICE("Error to disable ft5336\n");
		}

		switch (display_cfg) {
		case SINGLE_720P:
			/* enable exc3132 touchscreen */
			off = fdt_path_offset(fdt, "/soc/i2c/exc3132");
			if (off >= 0) {
				fdt_setprop_string(fdt, off, "status", "okay");
				NOTICE("exc3132 enabled\n");
			} else
				NOTICE("Error to enable exc3132\n");
			break;
		case SINGLE_720P_10INCHES:
			/* enable ft5x26 touchscreen */
			off = fdt_path_offset(fdt, "/soc/i2c/ft5x26");
			if (off >= 0) {
				fdt_setprop_string(fdt, off, "status", "okay");
				NOTICE("ft5x26 enabled\n");
			} else
				NOTICE("Error to enable ft5x26\n");
			break;
		default:
			/* Nothing to do */
			break;
		}
	} else if (get_board_rev_id() >= 2) {
		/* TC3P EVB boards wave C and higher */
		off = fdt_path_offset(fdt, "/soc/msp@48010000/i2s");
		if (off >= 0) {
			if (fdt_setprop_u32(fdt, off, "rx-data-delay", 1) < 0)
				NOTICE("Error to enable rx-data-delay\n");
		} else
			NOTICE("Error to find i2s1@48010000\n");
	}

	return 0;
}

/*******************************************************************************
 * Update Device tree
 ******************************************************************************/
int sta_update_fdt(void *fdt, int max_size)
{
	int ret;

	NOTICE("Updating DTB...\n");
	ret = fdt_open_into(fdt, fdt, max_size);
	if (!ret) {
		ret = sta_fdt_setup(fdt);
		if (ret) {
			ERROR("Failed to apply changes in DTB\n");
			return ret;
		}
		ret = fdt_pack(fdt);
	}

	return ret;
}
