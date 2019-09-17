/*
 * Copyright (c) 2014-2016, Linaro Limited
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

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <stdint.h>
#include <sta_mem_map.h>

/*
 * Declare variables shared between M3 and AP processors
 * Overwrite _SHARED_DATA macro to get shared_data base address
 * according to the MMU enabling
 */
#ifndef ASM
void *get_shared_data_base(void);
void *get_shared_data_rw_base(void);
#define _SHARED_DATA		get_shared_data_base()
#define _SHARED_DATA_RW		get_shared_data_rw_base()
#include <shared_data.h>
#endif

/* Make stacks aligned to data cache line length */
#define STACK_ALIGNMENT		32

#define CFG_TEE_CORE_NB_CORE	2

#if defined(CFG_WITH_LPAE)
#if defined(CFG_WITH_PAGER)
#define MAX_XLAT_TABLES		3
#else
#define MAX_XLAT_TABLES		10
#endif /* CFG_WITH_PAGER */
#else
/*
 * For safety requirement, only required peripherals have to be configured in
 * MMU. On STA platform, peripheral registers are memory mapped and cover a
 * memory address range of 64KB. By default the IO mem peripheral areas are
 * aligned to CORE_MMU_PGDIR_SIZE (1 or 2MB) MMU page size so a single
 * peripheral isolation is not possible. Therefore the platform peripheral
 * iomem must be individually configured to a new MEM_AREA_IO_SEC_SPG type
 * allowing a 4KB MMU page granularity.
 * When CFG_WITH_LPAE is disable (MMU v7 usage), the physical base addresses
 * must be aligned to CORE_MMU_PGDIR_SIZE for MMU mapping configuration, so
 * this finer page granularity is not possible.
 */
#error CFG_WITH_LPAE must be enabled to be compliant with safety isolation
#endif /* CFG_WITH_LPAE */

/* Below are platform/SoC settings specific to sta platform flavors */

#if defined(PLATFORM_FLAVOR_1385) || defined(PLATFORM_FLAVOR_1295) ||\
	defined(PLATFORM_FLAVOR_1195) || defined(PLATFORM_FLAVOR_1275)

/*
 * Platform peripheral registers definitions
 */
/* GIC registers */
#define GIC_BASE		0x48E00000
#define GIC_CPU_BASE		(GIC_BASE + 0x2000)
#define GIC_DIST_BASE		(GIC_BASE + 0x1000)
#define GICD_CTLR		(0x000)
#define GICD_SGIR		(0xF00)
#define GIC_SGI_OFFSET		0
#define GIC_PPI_OFFSET		16
#define GIC_SPI_OFFSET		32

/* GICD_CTLR bit definitions */
#define ENABLE_GRP1		BIT(1)
#define ENABLE_GRP0		BIT(0)

#define TIMER_CLK_FREQ		52000000

/* UART console */
#define UART0_BASE		0x50150000
#define UART1_BASE		0x50000000
#define UART2_BASE		0x50010000
#define UART3_BASE		0x50020000
#define CONSOLE_UART_BASE	UART1_BASE
#define CONSOLE_BAUDRATE        115200
#define CONSOLE_UART_CLK_IN_HZ	51200000

/* CTX-A7 Miscellaneous registers */
#define MSCR_BASE		0x50700000
#define MSCR_A7SS_REG		0x0058
#define CPU0_WFI_MASK		BIT(4)
#define CPU1_WFI_MASK		BIT(5)
#define CORE_WFI_MASK(core)	SHIFT_U32(CPU0_WFI_MASK, (core))

/* SRC-M3 registers (Peripheral clocks enabling) */
#define SRCM3_BASE		0x40020000
#define SRCM3_PCKEN0_REG	0x0030
#define UART0_CLK_MASK		BIT(2)
#define UART1_CLK_MASK		BIT(6)
#define UART2_CLK_MASK		BIT(12)
#define UART3_CLK_MASK		BIT(16)

/* Mailboxes Interrupt IDs in GIC */
#define MBOX_A7_M3_BASE		0x481a0000
#define MBOX_A7_M3_ID		(63 + GIC_SPI_OFFSET)
#define MBOX_A7_HSM_BASE	0x481e0000
#define MBOX_A7_HSM_ID		(87 + GIC_SPI_OFFSET)

/*
 * Platform peripheral IOMEM definitions
 */
/*
 * In case of security and safety isolation some peripherals can only be
 * accessed by a TEE, so the related register access requests from REE go
 * through secure monitor.
 * If CFG_STA_REMOTEPROC_CTRL is set, then read, write and update register
 * requests are forwarded to the remote processor, relying on "regmap" mailbox
 * channel.
 * Otherwise, register requests are directly handled by the TEE and therefore
 * it require a MMU configuration allowing access to all peripheral memory map
 * range accordingly.
 */
#if defined(CFG_SM_PLATFORM_HANDLER) && !defined(CFG_STA_REMOTEPROC_CTRL)

#define PERIPHS_IOMEM_BASE	0x40000000
#define PERIPHS_IOMEM_SIZE	0x29000000

#else

#define PERIPH_IOMEM_SIZE	0x10000

#define UART2_IOMEM_BASE	ROUNDDOWN(UART2_BASE, SMALL_PAGE_SIZE)
#define UART2_IOMEM_SIZE	ROUNDUP(PERIPH_IOMEM_SIZE + \
					(UART2_BASE - UART2_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

#define UART3_IOMEM_BASE	ROUNDDOWN(UART3_BASE, SMALL_PAGE_SIZE)
#define UART3_IOMEM_SIZE	ROUNDUP(PERIPH_IOMEM_SIZE + \
					(UART3_BASE - UART3_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

#define GIC_IOMEM_BASE		ROUNDDOWN(GIC_BASE, SMALL_PAGE_SIZE)
#define GIC_IOMEM_SIZE		ROUNDUP(PERIPH_IOMEM_SIZE + \
					(GIC_BASE - GIC_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

#define MBOX_M3_IOMEM_BASE	ROUNDDOWN(MBOX_A7_M3_BASE, SMALL_PAGE_SIZE)
#define MBOX_M3_IOMEM_SIZE	ROUNDUP(PERIPH_IOMEM_SIZE + \
					(MBOX_A7_M3_BASE - \
					 MBOX_M3_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

#define MBOX_HSM_IOMEM_BASE	ROUNDDOWN(MBOX_A7_HSM_BASE, SMALL_PAGE_SIZE)
#define MBOX_HSM_IOMEM_SIZE	ROUNDUP(PERIPH_IOMEM_SIZE + \
					(MBOX_A7_HSM_BASE - \
					 MBOX_HSM_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

#endif /* CFG_SM_PLATFORM_HANDLER && !CFG_STA_REMOTEPROC_CTRL */

/*
 * Platform memory definitions
 */
/* SW MAILBOX base for PSCI in BAckup RAM */
#if !defined(ESRAM_A7_ATF_MAILBOXES_BASE)
#error SW MAILBOX region is undefined
#endif
#define MAILBOXES_BASE          ESRAM_A7_ATF_MAILBOXES_BASE

/* M3_OS (SCP) base addresses */
#if defined(ESRAM_M3_XL_SOFTWARE_BASE)
#define SCP_BL2_BASE		ESRAM_M3_XL_SOFTWARE_BASE
#endif
#if defined(ESRAM_A7_M3_CODE_BASE) && defined(ESRAM_A7_M3_CODE_SIZE)
#define SCP_FILE2_BL2_BASE	ESRAM_A7_M3_CODE_BASE
#define SCP_FILE2_BL2_SIZE	ESRAM_A7_M3_CODE_SIZE
#endif

/* M3_OS saved base address */
#if defined(DDRAM_ATF_M3OS_SAVED_BASE)
#define M3OS_SAVED_BASE		DDRAM_ATF_M3OS_SAVED_BASE
#endif

/* IPC shared memory for A7-M3 HW Maiboxes */
#if !defined(ESRAM_A7_ATF_IPC_BASE) ||\
	!defined(ESRAM_A7_ATF_IPC_SIZE)
#error shared memory region for A7-M3 HW MAILBOXES is undefined
#endif
#define IPC_A7_M3_BASE		ESRAM_A7_ATF_IPC_BASE

/* HSM only available STA1385 platform */
#if defined(PLATFORM_FLAVOR_1385)

/* IPC shared memory for A7-HSM HW Maiboxes */
#if !defined(ESRAM_A7_ATF_HSM_MAILBOXES_BASE) ||\
	!defined(ESRAM_A7_ATF_HSM_MAILBOXES_SIZE)
#error shared memory region for A7-HSM HW MAILBOXES is undefined
#endif
#define IPC_A7_HSM_BASE		ESRAM_A7_ATF_HSM_MAILBOXES_BASE

#endif /* PLATFORM_FLAVOR_1385 */

/* Phys addr of DRAM Secure mem */
#if !defined(DDRAM_ATF_TRUSTED_ZONE_BASE) ||\
	!defined(DDRAM_ATF_TRUSTED_ZONE_SIZE)
#error DDRAM_ATF_TRUSTED region is undefined
#endif
#define MEM_DRAM_SEC_BASE	ROUNDDOWN(DDRAM_ATF_TRUSTED_ZONE_BASE, \
					  SMALL_PAGE_SIZE)
#define MEM_DRAM_SEC_SIZE	ROUNDUP((DDRAM_ATF_TEE_BASE - \
					 DDRAM_ATF_TRUSTED_ZONE_BASE) + \
					(DDRAM_ATF_TRUSTED_ZONE_BASE - \
					 MEM_DRAM_SEC_BASE), \
					SMALL_PAGE_SIZE)

/* Phys addr of DRAM NonSecure mem */
/* Shared data area */
#if !defined(DDRAM_SHARED_DATA_BASE) ||\
	!defined(DDRAM_SHARED_DATA_SIZE)
#error SHARED_DATA region is undefined
#endif
#define SHARED_DATA_BASE	DDRAM_SHARED_DATA_BASE
#define SHARED_DATA_SIZE	DDRAM_SHARED_DATA_SIZE

#define SHARED_DATA_IOMEM_BASE	ROUNDDOWN(SHARED_DATA_BASE, SMALL_PAGE_SIZE)
#define SHARED_DATA_IOMEM_SIZE	ROUNDUP(SHARED_DATA_SIZE + \
					(SHARED_DATA_BASE - \
					 SHARED_DATA_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

/* IPC M3 and HSM buffer in SRAM Secure mem */
#define IPC_M3_IOMEM_BASE	ROUNDDOWN(ESRAM_A7_ATF_IPC_BASE, \
					  SMALL_PAGE_SIZE)
#define IPC_M3_IOMEM_SIZE	ROUNDUP(ESRAM_A7_ATF_IPC_SIZE + \
					(ESRAM_A7_ATF_IPC_BASE - \
					 IPC_M3_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

#define IPC_HSM_IOMEM_BASE	ROUNDDOWN(ESRAM_A7_ATF_HSM_MAILBOXES_BASE, \
					  SMALL_PAGE_SIZE)
#define IPC_HSM_IOMEM_SIZE	ROUNDUP(ESRAM_A7_ATF_HSM_MAILBOXES_SIZE + \
					(ESRAM_A7_ATF_HSM_MAILBOXES_BASE - \
					 IPC_HSM_IOMEM_BASE), \
					SMALL_PAGE_SIZE)

/* M3 eSRAM for M3 OS part1 loading at PSCI resume state */
#if !defined(ESRAM_M3_BASE) || \
	!defined(ESRAM_M3_SIZE)
#error ESRAM_M3 region is undefined
#endif
#define ESRAM_M3_IOMEM_BASE	ROUNDDOWN(ESRAM_M3_BASE, CORE_MMU_PGDIR_SIZE)
#define ESRAM_M3_IOMEM_SIZE	ROUNDUP(ESRAM_M3_SIZE + \
					(ESRAM_M3_BASE - \
					 ESRAM_M3_IOMEM_BASE), \
					CORE_MMU_PGDIR_SIZE)
/* A7 eSRAM for M3 OS part2 loading at PSCI resume state */
#if defined(SCP_FILE2_BL2_BASE)
#define ESRAM_A7_IOMEM_BASE	ROUNDDOWN(SCP_FILE2_BL2_BASE, \
					  CORE_MMU_PGDIR_SIZE)
#define ESRAM_A7_IOMEM_SIZE	ROUNDUP(SCP_FILE2_BL2_SIZE + \
					(SCP_FILE2_BL2_BASE - \
					 ESRAM_A7_IOMEM_BASE), \
					CORE_MMU_PGDIR_SIZE)
#endif

#else /* defined(PLATFORM_FLAVOR_xxx) */

#error "Unknown platform flavor"

#endif /* defined(PLATFORM_FLAVOR_xxx) */

/*
 * Below are settings common to sta platform flavors
 */
/* Secure SGI used to wake-up the secondary CPU */
#define WAKING_UP_SGI           8

/*
 * CP15 Auxiliary ConTroL Register (ACTRL)
 *
 * - core always in full SMP (FW bit0=1, SMP bit6=1)
 * - L2 write full line of zero disabled (bit3=0)
 *   (keep WFLZ low. Will be set once outer L2 is ready)
 */
#define CPU_ACTLR_INIT		0x00000041

/*
 * CP15 NonSecure Access Control Register (NSACR)
 *
 * - NSec cannot change ACTRL.SMP (NS_SMP bit18=0)
 * - Nsec can lockdown TLB (TL bit17=1)
 * - NSec cannot access PLE (PLE bit16=0)
 * - NSec can use SIMD/VFP (CP10/CP11) (bit15:14=2b00, bit11:10=2b11)
 */
#define CPU_NSACR_INIT		0x00020C00

/* Convert memory alias to real eSRAM physical address */
#define MEM_ALIAS_AREA		0xF0000000
#define ALIAS_TO_PHYS(a)	(!((a) & MEM_ALIAS_AREA) ? \
				 ((a) + ESRAM_A7_BASE) : (a))
#define ALIAS_TO_PHYS_M3(a)	(!((a) & MEM_ALIAS_AREA) ? \
				 ((a) + ESRAM_M3_BASE) : (a))

/*
 * TEE RAM layout without CFG_WITH_PAGER:
 *
 *  +---------------------------------------+  <- TZDRAM_BASE
 *  | TEE private secure |  TEE_RAM         |   ^
 *  |   external memory  +------------------+   | TZDRAM_SIZE
 *  |                    |  TA_RAM          |   |
 *  +---------------------------------------+   v
 *
 *  +---------------------------------------+   <- CFG_SHMEM_START
 *  |     Non secure     |  SHM             |   ^
 *  |   shared memory    |                  |   | CFG_SHMEM_SIZE
 *  +---------------------------------------+   v
 *
 *  TEE_RAM : default 1MByte
 *  TA_RAM  : all what is left
 *  PUB_RAM : default 2MByte
 *
 * ----------------------------------------------------------------------------
 * TEE RAM layout with CFG_WITH_PAGER=y:
 *
 *  +---------------------------------------+  <- TZSRAM_BASE
 *  | TEE private secure |  TEE_RAM         |   ^
 *  |   internal memory  |                  |   | TZSRAM_SIZE
 *  +---------------------------------------+   v
 *
 *  +---------------------------------------+  <- TZDRAM_BASE
 *  | TEE private secure |  TA_RAM          |   ^
 *  |   external memory  |                  |   | TZDRAM_SIZE
 *  +---------------------------------------+   v
 *
 *  +---------------------------------------+   <- CFG_SHMEM_START
 *  |     Non secure     |  SHM             |   ^
 *  |   shared memory    |                  |   | CFG_SHMEM_SIZE
 *  +---------------------------------------+   v
 *
 *  TEE_RAM : default 256kByte
 *  TA_RAM  : all what is left in DDR TEE reserved area
 *  PUB_RAM : default 2MByte
 */
#if !defined(DDRAM_ATF_TEE_BASE) || !defined(DDRAM_ATF_TEE_SIZE)
#error DDRAM_ATF_TEE region is undefined
#endif
#if !defined(DDRAM_TEE_SHMEM_BASE) || !defined(DDRAM_TEE_SHMEM_SIZE)
#error DDRAM_TEE_SHMEM region is undefined
#endif

#if defined(CFG_WITH_PAGER)

#if !defined(ESRAM_A7_ATF_BL32_BASE) || !defined(ESRAM_A7_ATF_BL32_SIZE)
#error ESRAM_A7_ATF_BL32 region is undefined
#endif

/* Secure internal RAM (eSRAM) */
#define TZSRAM_BASE		(ESRAM_A7_ATF_BL32_BASE - ESRAM_A7_BASE)
#define TZSRAM_SIZE		(ESRAM_A7_ATF_BL32_SIZE)

/* Secure external RAM (DDR) */
#define TZDRAM_BASE		(DDRAM_ATF_TEE_BASE)
#define TZDRAM_SIZE		(DDRAM_ATF_TEE_SIZE)

/* TEE RAM in internal secure RAM (eSRAM) */
#define CFG_TEE_RAM_START	TZSRAM_BASE
#define CFG_TEE_RAM_PH_SIZE	TZSRAM_SIZE

/* TA RAM in external secure RAM (DDR) */
#define CFG_TA_RAM_START	TZDRAM_BASE
#define CFG_TA_RAM_SIZE		TZDRAM_SIZE

#else  /* CFG_WITH_PAGER */

/* Secure external RAM (DDR) */
#define TZDRAM_BASE		(DDRAM_ATF_TEE_BASE)
#define TZDRAM_SIZE		(DDRAM_ATF_TEE_SIZE)

/* TEE RAM in external secure RAM (DDR) */
#define CFG_TEE_RAM_START	TZDRAM_BASE
#define CFG_TEE_RAM_PH_SIZE	CFG_TEE_RAM_VA_SIZE

/* TA RAM in external secure RAM (DDR) */
#define CFG_TA_RAM_START	(TZDRAM_BASE + CFG_TEE_RAM_PH_SIZE)
#define CFG_TA_RAM_SIZE		(TZDRAM_SIZE - CFG_TEE_RAM_PH_SIZE)

#if (CFG_TEE_RAM_PH_SIZE  >= TZDRAM_SIZE)
#error Too few memory reserved for CFG_TEE_RAM and CFG_TA_RAM
#endif

#endif /* !CFG_WITH_PAGER */

/* default locate shared memory at the end of the TEE reserved DDR */
/* external non secure ram */
#define CFG_SHMEM_START		(DDRAM_TEE_SHMEM_BASE)
#define CFG_SHMEM_SIZE		(DDRAM_TEE_SHMEM_SIZE)

/* OPTEE internal keep 1MB */
#define CFG_TEE_RAM_VA_SIZE	(1024 * 1024)

#ifndef CFG_TEE_LOAD_ADDR
/* Keep load address == TEE_RAM start */
#define CFG_TEE_LOAD_ADDR	CFG_TEE_RAM_START
#endif

#endif /* PLATFORM_CONFIG_H */
