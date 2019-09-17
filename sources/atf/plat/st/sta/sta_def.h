/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __STA_DEF_H__
#define __STA_DEF_H__

#include <tbbr_img_def.h>
#include <gic_common.h>
#include <memory_map.h>

/* Reserved for M3/AP shared data */
#if !defined(SHARED_DATA_BASE) || !defined(SHARED_DATA_SIZE)
#error SHARED_DATA region is undefined
#endif

/* SW Maiboxes for PSCI */
#if !defined(MAILBOXES_BASE) || !defined(MAILBOXES_SIZE)
#error SW MAILBOXES region is undefined
#endif

/* HW Maiboxes for PSCI */
#if !defined(IPC_BASE)
#error shared memory region for HW MAILBOXES is undefined
#endif

#define WAKING_UP_SGI           ARM_IRQ_SEC_SGI_0

/* Possible boot devices */
#define MMC	0
#define SQI	1
#define NAND	2

/* Boot device */
#if !defined(MEMORY_BOOT_DEVICE)
#error MEMORY_BOOT_DEVICE is not defined: possible values MMC, SQI or NAND
#endif

/*
 * Grab enough pages for BL2 and BL32/sp_min.
 *
 * Enable verbose log level and look for "mmap:" then count tables to fine-tune
 * max number.
 * Look for a line such as "Used 6 sub-tables out of 7 (spare: 1)" as well.
 */
#define STA_MAX(a, b) ((a) > (b) ? (a) : (b))

#if REMOTEPROC_CTRL
#if (DRAM_NS_SIZE <= 448 * 1024 * 1024)
#if (MEMORY_BOOT_DEVICE == MMC)
#define MAX_BL2_XLAT_TABLES     9
#define MAX_BL32_XLAT_TABLES    10
#elif (MEMORY_BOOT_DEVICE == SQI)
#define MAX_BL2_XLAT_TABLES     10
#define MAX_BL32_XLAT_TABLES    10
#elif (MEMORY_BOOT_DEVICE == NAND)
#define MAX_BL2_XLAT_TABLES     10
#define MAX_BL32_XLAT_TABLES    10
#endif
#else
#if (MEMORY_BOOT_DEVICE == MMC)
#define MAX_BL2_XLAT_TABLES     10
#define MAX_BL32_XLAT_TABLES    11
#elif (MEMORY_BOOT_DEVICE == SQI)
#define MAX_BL2_XLAT_TABLES     11
#define MAX_BL32_XLAT_TABLES    11
#elif (MEMORY_BOOT_DEVICE == NAND)
#define MAX_BL2_XLAT_TABLES     11
#define MAX_BL32_XLAT_TABLES    11
#endif
#endif
#else /* REMOTEPROC_CTRL */
#if (DRAM_NS_SIZE <= 448 * 1024 * 1024)
#if (MEMORY_BOOT_DEVICE == MMC)
#define MAX_BL2_XLAT_TABLES     7
#define MAX_BL32_XLAT_TABLES    7
#elif (MEMORY_BOOT_DEVICE == SQI)
#define MAX_BL2_XLAT_TABLES     8
#define MAX_BL32_XLAT_TABLES    7
#elif (MEMORY_BOOT_DEVICE == NAND)
#define MAX_BL2_XLAT_TABLES     8
#define MAX_BL32_XLAT_TABLES    7
#endif
#else
#if (MEMORY_BOOT_DEVICE == MMC)
#define MAX_BL2_XLAT_TABLES     8
#define MAX_BL32_XLAT_TABLES    8
#elif (MEMORY_BOOT_DEVICE == SQI)
#define MAX_BL2_XLAT_TABLES     9
#define MAX_BL32_XLAT_TABLES    8
#elif (MEMORY_BOOT_DEVICE == NAND)
#define MAX_BL2_XLAT_TABLES     9
#define MAX_BL32_XLAT_TABLES    8
#endif
#endif
#endif /* REMOTEPROC_CTRL */

#define MAX_XLAT_TABLES		(STA_MAX(MAX_BL2_XLAT_TABLES,\
					 MAX_BL32_XLAT_TABLES))

/*
 * MAX_MMAP_REGIONS is usually:
 * BL mmap_reserve directives + 2 (BL total region + BL code region)
 */
#define MAX_MMAP_REGIONS	(MAX_XLAT_TABLES + 2)

#define BL2_BASE		BL1_BL2_BASE
#define BL2_SIZE		BL1_BL2_SIZE

#if !defined(BL1_BL2_BASE) || !defined(BL1_BL2_SIZE)
#error BL1_BL2 region is undefined
#endif
#if (BL2_SIZE > BL1_BL2_SIZE)
#error Too few memory reserved for BL2
#endif

/* 68 KB for BL32 SP-MIN*/
#define SP_MIN_SIZE		U(0x00011000)
#if !defined(BL32_BASE) || !defined(BL32_SIZE)
#error BL32 region is undefined
#endif
#if ((BL32_SIZE - PLAT_XLAT_SIZE) < SP_MIN_SIZE)
#error Too few memory reserved for BL32 SP-MIN
#endif

#ifdef AARCH32_SP_OPTEE
#if !defined(BL32_EXTRA_BASE) || !defined(BL32_EXTRA_SIZE)
#error BL32_EXTRA region is undefined
#endif
/*
 * By default, BL32 OPTEE OS loading area is set to TZSRAM (config w/ pager).
 * BL2 needs to map an area in the TZSRAM in order to load/authenticate
 * the 1st trusted os extra image (pager image). The whole BL32 area of
 * TZSRAM is reserved for trusted os (OPTEE core unpaged part).
 * BL2 maps an area in the TZDRAM for the 2nd OPTEE extra image loading.
 * This 2nd image is the paged image which only include the paging part
 * using virtual memory but without "init" data.
 * OPTEE core will copy the "init" data (from pager image) to the beginning
 * of the dedicated TZDRAM area, and then copy the extra image behind the
 * "init" data.
 */
#define BL32_EXTRA1_BASE		BL32_BASE
#define BL32_EXTRA1_SIZE		BL32_SIZE
#define BL32_EXTRA2_BASE		BL32_EXTRA_BASE
#define BL32_EXTRA2_SIZE		BL32_EXTRA_SIZE

/*
 * When BL32 OPTEE is configured w/o pager, BL2 maps the whole OPTEE
 * images in TZDRAM. This reserved area in TZDRAM is split to load
 * unpaged and paged images.
 */
#define BL32_EXTRA1_RESERVED_SIZE	U(0x00100000) /* 1 MB for OPTEE core */
#define BL32_EXTRA2_RESERVED_SIZE	U(0x00200000) /* 2 MB for OPTEE pageable */

#if ((BL32_EXTRA1_RESERVED_SIZE + BL32_EXTRA2_RESERVED_SIZE) > BL32_EXTRA_SIZE)
#error Too few memory reserved for BL32_EXTRA1 and BL32_EXTRA2
#endif
#endif /* AARCH32_SP_OPTEE */

/* Set default BL33 executable offset */
#if STA_DIRECT_LINUX_BOOT
#define BL33_EXE_OFFSET		0x1000000 /* Needed for zImage decompression */
#else
#define BL33_EXE_OFFSET		0x0
#endif /* STA_DIRECT_LINUX_BOOT */

/*
 * STA device/io map related constants (used for MMU)
 */
/* Peripherals */
#define DEVICE1_BASE		U(0x40000000)
#define DEVICE1_SIZE		U(0x29000000)
/* NAND */
#define DEVICE2_BASE		U(0x80000000)
#define DEVICE2_SIZE		U(0x00040000)
/* SQI */
#define DEVICE3_BASE		U(0x90000000)
#define DEVICE3_SIZE		U(0x10000000)

/* Memory map size reserved for each peripheral */
#define PERIPH_SIZE		U(0x10000)

/*
 * STA PL011 UART
 */
#define PL011_BAUDRATE		U(115200)
#define PL011_UART_CLK_IN_HZ	U(51200000) /* FIXME or 52000000 on TC3P */

#define UART0_BASE		U(0x50150000)
#define UART1_BASE		U(0x50000000)
#define UART2_BASE		U(0x50010000)
#define UART3_BASE		U(0x50020000)
#ifndef CONSOLE_BASE
/*
 * UART3 for A5 EVB
 * UART2 for TC3P CARRIER
 * UART2 for TC3P MTP
 */
#define CONSOLE_BASE		UART3_BASE
#endif /* CONSOLE_BASE */

/*
 * STA GIC-400
 */
#define GICD_BASE		U(0x48E01000)
#define GICC_BASE		U(0x48E02000)
#define GICH_BASE		U(0x48E04000)
#define GICV_BASE		U(0x48E06000)

/*
 * STA SRC A7 clock control
 */
#define SRC_A7_BASE		U(0x48100000)
#define SRC_A7_RESSTAT		U(0x4810001C)
#define MXTAL_FREQ_SEL_BIT	U(1 << 5)

#define FIP_MAX_SIZE		U(0x00600000)

#if !defined(BLOCKDEV_DATA_BASE) || !defined(BLOCKDEV_DATA_SIZE)
#error BLOCKDEV_DATA region is undefined
#endif

/*
 * SD-MMC
 */
#define MMC0_BASE		U(0x50080000) /* Arasan SDHCI */
#define MMC1_BASE		U(0x50070000) /* ARM PL180 MMCI */
#define MMC2_BASE		U(0x500B0000) /* ARM PL180 MMCI */


/*
 * FSMC-NAND
 */
#define FSMC_BASE		U(0x50300000)
#define NAND_CS0_BASE		U(0x80000000)
/* WARN: must be aligned with flashloader configuration !!! */
#define FIP_NAND_BASE		U(0x180000)

#define EMMC_BASE		U(0)
/* WARN: must be aligned with flashloader configuration !!! */
#define FIP_EMMC_BASE		(EMMC_BASE + U(0x48000))

/*
 * STA QSPI
 */
#define QSPI0_BASE		U(0x50200000)
#define QSPI0_NOR_MEM_MAP_BASE	U(0x90000000)
/* WARN: must be aligned with flashloader configuration !!! */
#define FIP_QSPI_BASE		U(0x50000)

/* HSEM */
#define STA_HSEM_BASE		U(0x481B0000)

/* MBOX */
#define STA_MBOX_M3_BASE	U(0x48190000)
#define STA_MBOX_BASE		U(0x481a0000)
#define IRQ_SEC_SPI_MBOX_TO_M3	(U(63) + MIN_SPI_ID)

/*
 * DEBUG
 */
/*#define  DCACHE_OFF*/
/*#define  MMU_OFF*/

#endif /* __DEF_H__ */
