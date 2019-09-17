/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STA_PRIVATE_H__
#define __STA_PRIVATE_H__

#include <bl_common.h>

/*
 * MMU Management
 */
#define MAP_ESRAM_M3	MAP_REGION_FLAT(ESRAM_M3_BASE, \
					ESRAM_M3_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_ESRAM	MAP_REGION_FLAT(ESRAM_BASE, \
					ESRAM_SIZE, \
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_SHARED_DATA	MAP_REGION_FLAT(SHARED_DATA_BASE, \
					SHARED_DATA_SIZE, \
					MT_NON_CACHEABLE | MT_RW | \
					MT_EXECUTE_NEVER)

/*
 * In case of security isolation some peripherals can only be accessed by a
 * secure bus master, so the related register access requests from REE go
 * through secure monitor.
 * If REMOTEPROC_CTRL is set, then read, write and update register
 * requests are forwarded to the remote processor, relying on "regmap" mailbox
 * channel.
 * Otherwise, register requests are directly handled by the secure monitor and
 * so it requires a MMU configuration allowing access to all peripheral memory
 * map range.
 * Furthermore, in case of safety isolation, peripherals accesses by the
 * secure A7 must be limited to required ones and REMOTEPROC_CTRL has to be
 * set
 */
#if REMOTEPROC_CTRL

#define MAP_CONSOLE	MAP_REGION_FLAT(CONSOLE_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_GIC		MAP_REGION_FLAT(GICD_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_HSEM	MAP_REGION_FLAT(STA_HSEM_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_MBOX_M3	MAP_REGION_FLAT(STA_MBOX_M3_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_MBOX	MAP_REGION_FLAT(STA_MBOX_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_SRC_A7	MAP_REGION_FLAT(SRC_A7_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_MMC0	MAP_REGION_FLAT(MMC0_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_MMC1	MAP_REGION_FLAT(MMC1_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_MMC2	MAP_REGION_FLAT(MMC2_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_NAND	MAP_REGION_FLAT(FSMC_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_SQI		MAP_REGION_FLAT(QSPI0_BASE, \
					PERIPH_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)
#else

#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE, \
					DEVICE1_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)
#define MAP_CONSOLE	MAP_DEVICE1
#define MAP_GIC		MAP_DEVICE1
#define MAP_HSEM	MAP_DEVICE1
#define MAP_MBOX_M3	MAP_DEVICE1
#define MAP_MBOX	MAP_DEVICE1
#define MAP_SRC_A7	MAP_DEVICE1
#define MAP_MMC0	MAP_DEVICE1
#define MAP_MMC1	MAP_DEVICE1
#define MAP_MMC2	MAP_DEVICE1
#define MAP_NAND	MAP_DEVICE1
#define MAP_SQI		MAP_DEVICE1

#endif /* REMOTEPROC_CTRL */

#define MAP_DEVICE2	MAP_REGION_FLAT(DEVICE2_BASE, \
					DEVICE2_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DEVICE3	MAP_REGION_FLAT(DEVICE3_BASE, \
					DEVICE3_SIZE, \
					MT_DEVICE | MT_RO | MT_SECURE | \
					MT_EXECUTE_NEVER)

#define MAP_DRAM	MAP_REGION_FLAT(DRAM_BASE, \
					DRAM_SIZE, \
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_DRAM_NS	MAP_REGION_FLAT(DRAM_NS_BASE, \
					DRAM_NS_SIZE, \
					MT_MEMORY | MT_RW | MT_NS)

#define mmap_reserve(mm) static mmap_region_t __used __mmap_region_##mm \
	__section(".mmap_regions") = mm
#define IN_MMAP(adr, mm) (((adr) >= __mmap_region_##mm.base_va) && \
			  ((adr) < (__mmap_region_##mm.base_va + \
				    __mmap_region_##mm.size)))

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void sta_io_setup(void);
void sta_configure_mmu(void);
#ifdef AARCH32_SP_OPTEE
uint32_t sta_get_spsr_for_bl32_entry(void);
#endif
uint32_t sta_get_spsr_for_bl33_entry(void);
void sta_security_setup(void);
void sta_scp_wait_sync(void);

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31 e.g. while passing control to it from BL2 which is bl31_params
 * and bl31_plat_params and its elements
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	bl_params_t bl31_params;
	image_info_t bl32_image_info;
	image_info_t bl31_image_info;
	image_info_t bl33_image_info;
	entry_point_info_t bl33_ep_info;
	entry_point_info_t bl32_ep_info;
	entry_point_info_t bl31_ep_info;
} bl2_to_bl31_params_mem_t;

/* gic support */
void sta_gic_driver_init(void);
void sta_gic_init(void);
void sta_ic_cpuif_enable(void);
void sta_gic_cpuif_disable(void);
void sta_gic_pcpu_init(void);

#endif /* __STA_PRIVATE_H__ */
