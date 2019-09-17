/*
 * Copyright (C) 2016 STMicroelectronics.
 * Common configuration settings for the STA SoC family
 * running on ARMv7 Cortex-A7 processor,
 */

/*
 * Common definitions for all sta configs
 */

#ifndef __CONFIG_STA_H
#define __CONFIG_STA_H

#include <linux/sizes.h>
#ifndef __ASSEMBLY__
#include <linux/types.h>
#endif
#include "sta_mem_map.h"

#define STA_VERSION	"7.7"
#define MANUFACTURER	"STMicroelectronics"

#define CONFIG_SYS_THUMB_BUILD /* Compile in Thumb2 */

/* Cortex-A7 uses a cache line size of 64 bytes */
#define CONFIG_SYS_CACHELINE_SIZE	64

/* Malloc pool size */
#define CONFIG_SYS_MALLOC_LEN	(10 * SZ_1M)   /* 10 MBytes at end of DDRAM */

/* SMP */
#define CONFIG_SMP_PEN_ADDR BACKUP_RAM_PEN_SECONDARY_KERNEL_ADDR_BASE

/* FIT support */
#define CONFIG_IMAGE_FORMAT_LEGACY /* enable also legacy image format */

/**
 * OS Timer stamp frequency: should be equal to PLL2FVCO_BY2_B12
 * in normal mode.
 */
#define CONFIG_TIMER_CLK_FREQ 51200000

#if defined(CONFIG_IDENT_STRING)
#undef CONFIG_IDENT_STRING
#endif

/* boot config: support both device trees and ATAGs. */
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_TAG

/* configure a clustsize smaller than the default 64k */
#define CONFIG_FS_FAT_MAX_CLUSTSIZE (16 * 1024)

/* user interface */
#define CONFIG_SYS_CBSIZE	1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE /* Boot Arg Buffer Size */
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_LOAD_ADDR	DDRAM_BASE	/* default load address */
#define CONFIG_SYS_LOADS_BAUD_CHANGE

/* AP ESRAM mapping */
#define STA_ESRAM_START		0x0
#define STA_ESRAM_SIZE		ESRAM_A7_SIZE

/* memory-related information */
#ifdef CONFIG_ARMV7_NONSEC
/* The M3-Xloader loads us at 0x00000000 (ESRAM) */
#define CONFIG_SYS_INIT_RAM_SIZE	STA_ESRAM_SIZE
#else
/* The M3 core loads us at start of DDRAM */
#define CONFIG_SYS_INIT_RAM_SIZE	(2 * SZ_1M)
#endif

/* Put initial GBL DATA and initial stack at end of SYS_INIT_RAM */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE \
			+ CONFIG_SYS_INIT_RAM_SIZE \
			- GENERATED_GBL_DATA_SIZE)

#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM_1			DDRAM_BASE
#define PHYS_SDRAM_1_SIZE		DDRAM_APP_OS_SYSTEM_MEM_SIZE
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_SDRAM_SIZE		PHYS_SDRAM_1_SIZE

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_SIZE + 2 * SZ_1M)

/* serial port (PL011) configuration */
#define CONFIG_PL01X_SERIAL
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, \
	230400, 460800, 500000, 921600, 1000000, 1152000, \
	1500000, 2000000, 2500000, 3000000, 3500000, 4000000}

/* FLASH and environment organization */
#define CONFIG_ENV_SIZE		0x8000 /* 32 Kb */

/* MMC related configs for one of the 2 sd slots */
#define MMC_BLOCK_SIZE		512
#define CONFIG_ARM_PL180_MMCI

/* this is needed to make hello_world.c and other stuff happy */
#define CONFIG_SYS_MAX_FLASH_SECT	512
#define CONFIG_SYS_MAX_FLASH_BANKS	1

/* QSPI */
# define CONFIG_SF_DEFAULT_SPEED	133000000

#if defined CONFIG_CMD_UBI
/* Additionnal commands UBI... */
#define CONFIG_CMD_UBIFS	/* UBIFS Support		*/

#define CONFIG_RBTREE	/* needed for ubi */
#define CONFIG_LZO	/* needed for UBIFS */
#endif /* CONFIG_CMD_UBI */

/* Define kernel size to 10MB to be compatible with initrmafs */
#define CONFIG_SYS_BOOTM_LEN	0xA00000

/* ENV */
/* See (UBOOT_ENV) placement below */

#if defined CONFIG_QSPI_BOOT /* Serial flash bootloader */

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET		(256 * SZ_1K) /* Offset from 0 */
#define CONFIG_ENV_SECT_SIZE		(64 * SZ_1K)

/* Additionnal commands ... */
#define CONFIG_CMD_MTDPARTS	/* mtd parts support		*/
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS

#define CONFIG_SPI_FLASH_MTD

#if !defined(CONFIG_NAND_BOOT) && !defined(CONFIG_SD_BOOT)
/* SQI flashLoader */
#define CONFIG_IDENT_STRING	" AP-sqi-xlflashLoader V" STA_VERSION

#define CONFIG_BOOTCOMMAND "echo Booting from Serial NOR"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=" __stringify(DDRAM_BASE) "\0" \
	"\0"
#endif

#endif /* CONFIG_QSPI_BOOT */

#if defined CONFIG_NAND_BOOT /* NAND bootloader */

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x80000
#endif

#if defined CONFIG_CMD_FASTBOOT /* It's a NAND flashLoader */
#define CONFIG_IDENT_STRING	" AP-nand-xlflashLoader V" STA_VERSION
#else
#define CONFIG_IDENT_STRING	" AP-nand-xloader V" STA_VERSION
#endif

/* NAND FLASH Configuration */
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_NAND_FSMC
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_FSMC_NAND_8BIT

/* Additionnal commands ... */
#define CONFIG_CMD_MTDPARTS	/* mtd parts support		*/

#define CONFIG_BOOTCOMMAND "run nandload nandloadfdt nandboot"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=n\0" \
	"stdout=serial\0" \
	"stdin=serial\0" \
	"stderr=serial\0" \
	"loadaddr=" __stringify(DDRAM_BASE) "\0" \
	"loadzaddr=A1000000\0" \
	"fdtaddr=A0FE0000\0" \
	"kernel_image=uImage\0" \
	"kernel_sz=800000\0" \
	"nand_page_sz=4096\0" \
	"serial#=01010101\0" \
	"commonargs=rw rootwait earlyprintk quiet\0" \
	"nandloadfdt=nand read ${fdtaddr} DTB 0x20000\0" \
	"nandload=nand read ${loadzaddr} AP_OS ${kernel_sz}\0" \
	"nandargs=setenv bootargs ${console} ${mtdparts} "\
		"ubi.mtd=UBI,${nand_page_sz} root=ubi0:rootfs rootfstype=ubifs ${commonargs}\0" \
	"nandboot=echo Booting from NAND...; " \
		"run nandargs; " \
		"bootm ${loadzaddr} - ${fdtaddr}\0" \
	"\0"

#endif /* CONFIG_NAND_BOOT */

#if defined CONFIG_SD_BOOT /* MMC bootloaders */

#if !defined(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET		(256 * SZ_1K) /* Offset from 0 */
#endif

#define CONFIG_BOOTCOMMAND "mmc dev ${mmcdev}; run mmcload mmcloadfdt mmcboot"

#define CONFIG_SYS_MMC_ENV_DEV		1   /* EMMC by default */

/* Partitions definitions */
#define PART_BOOT		"boot"
#define PART_ROOT		"rootfs"

#define PARTITIONS_DEFAULT  \
	"uuid_disk=${uuid_gpt_disk};" \
	"name="PART_BOOT",start=4MiB,size=16MiB,uuid=${uuid_gpt_"PART_BOOT"};" \
	"name="PART_ROOT",size=-,uuid=${uuid_gpt_"PART_ROOT"}"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"verify=n\0" \
	"stdout=serial\0" \
	"stdin=serial\0" \
	"stderr=serial\0" \
	"loadaddr=" __stringify(DDRAM_BASE) "\0" \
	"loadzaddr=A1000000\0" \
	"fdtaddr=A0FE0000\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcldev=0\0" \
	"mmcbootpart=1\0" \
	"mmcrootpart=2\0" \
	"kernel_image=uImage\0" \
	"fdtfile=evb.dtb\0" \
	"serial#=01010101\0" \
	"partitions="PARTITIONS_DEFAULT"\0" \
	"commonargs=rw rootwait earlyprintk quiet\0" \
	"mmcloadfdt=ext4load mmc ${mmcdev}:${mmcbootpart} ${fdtaddr} ${fdtfile}\0" \
	"mmcload=ext4load mmc ${mmcdev}:${mmcbootpart} ${loadzaddr} ${kernel_image}\0" \
	"mmcargs=setenv bootargs ${console} "\
		"root=/dev/mmcblk${mmcldev}p${mmcrootpart} rootfstype=ext4 ${commonargs}\0" \
	"mmcboot=echo Booting from MMC${mmcdev}...; " \
		"run mmcargs; " \
		"bootm ${loadzaddr} - ${fdtaddr}\0" \
	"\0"

#if defined CONFIG_CMD_FASTBOOT /* It's a MMC flashLoader */
#define CONFIG_IDENT_STRING	" AP-mmc-xlflashLoader V" STA_VERSION

/* Additionnal commands ... */
#define CONFIG_FAT_WRITE
#define CONFIG_SUPPORT_EMMC_BOOT /* emmc Support */
#define CONFIG_RANDOM_UUID
#else
#define CONFIG_IDENT_STRING	" AP-mmc-xloader V" STA_VERSION
#endif /* end CONFIG_CMD_FASTBOOT */

#endif /* CONFIG_SD_BOOT */

/* FASTBOOT */
#if defined CONFIG_CMD_FASTBOOT
/* USB */
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_DWC2_OTG
#define CONFIG_USB_GADGET_DWC2_OTG_PHY
#define CONFIG_USB_GADGET_VBUS_DRAW 2

/* USB Composite download gadget - g_dnl */
#define CONFIG_USB_GADGET_DOWNLOAD
/* USB fastboot Google Nexus generic IDs */
#define CONFIG_G_DNL_VENDOR_NUM 0x18D1
#define CONFIG_G_DNL_PRODUCT_NUM 0x4EE0
#define CONFIG_G_DNL_MANUFACTURER MANUFACTURER

/* Overload configs, to be dynamic */
#if (CONFIG_FASTBOOT_BUF_ADDR == 0) && (CONFIG_FASTBOOT_BUF_SIZE == 0)
#undef CONFIG_FASTBOOT_BUF_ADDR
#undef CONFIG_FASTBOOT_BUF_SIZE
#define CONFIG_FASTBOOT_BUF_ADDR	CONFIG_SYS_LOAD_ADDR
#define CONFIG_FASTBOOT_BUF_SIZE	(CONFIG_SYS_SDRAM_SIZE \
					- (10 * SZ_1M) - CONFIG_SYS_MALLOC_LEN)
#endif
#if defined CONFIG_CMD_NAND
#define CONFIG_FASTBOOT_FLASH_NAND_DEV
#else
#if defined CONFIG_FASTBOOT_FLASH_MMC_DEV
#undef CONFIG_FASTBOOT_FLASH_MMC_DEV
#define CONFIG_FASTBOOT_FLASH_MMC_DEV	mmc_get_env_dev()
#endif
#endif /* CONFIG_CMD_NAND */

#endif /* CONFIG_CMD_FASTBOOT */

/* Additionnal/Specific user interface */
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING

#ifdef CONFIG_STM_GMAC4
#define CONFIG_SYS_NONCACHED_MEMORY    (1 << 20)       /* 1 MiB */
#endif

#include "shared_data.h"

#endif /* __CONFIG_STA_H */
