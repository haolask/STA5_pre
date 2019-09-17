/**
 * @file shared_data.h
 * @brief Declare variables shared between M3 and AP processors
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

/*
 * ATTENTION!!!: this file is shared between M3 loaders and AP U-boot
 *               and must be identical
 * - in include/ directory of M3 loaders
 * - in board/st/sta1*xx/ directory of AP U-boot
 */

#ifndef __STA_SHARED_DATA_H__
#define __STA_SHARED_DATA_H__

#if !defined(__ASSEMBLY__) && !defined(ASM)

enum display_configs {
	NO_DETECTION,
	SINGLE_WVGA,
	DUAL_WVGA,
	SINGLE_720P,
	SINGLE_CLUSTER,
	SINGLE_CLUSTER_HD,
	SINGLE_HYBRID_CLUSTER_WVGA,
	SINGLE_HYBRID_CLUSTER_720P,
	SINGLE_720P_10INCHES
};

/* STA SoC identifiers as defined in OTP VR4 register */
#define SOCID_STA1295	0
#define SOCID_STA1195	1
#define SOCID_STA1385	2
#define SOCID_STA1275	3
#define SOCID_MAX	4

/* STA Board identifiers */
#define BOARD_A5_EVB        0
#define BOARD_A5_VAB        1
#define BOARD_TC3_EVB       2
#define BOARD_CR2_VAB       3
#define BOARD_TC3P_CARRIER  4
#define BOARD_TC3P_MTP      5
#define BOARD_UNKNOWN       0xFF

/* STA logical cut versions */
enum STA_CUT_REV {
	CUT_10 = 0x10,
	CUT_20 = 0x20,
	CUT_21 = 0x21,
	CUT_22 = 0x22,
	CUT_23 = 0x23,
	CUT_30 = 0x30,
	CUT_UNKNOWN = 0xFF,
};

enum STA_BOOT_DEV {
	BOOT_NAND = 0,    /* Boot on NAND flash */
	BOOT_MMC = 1,     /* Boot on EMMC or SDCard */
	BOOT_SQI = 2,     /* Boot on Serial NOR flash */
};

/* Copy of corresponding enum t_mmc_card_type defined in sta_sdmmc.h */
enum MMC_CARD_TYPE {
	MMC_CARD_MULTIMEDIA,
	MMC_CARD_MULTIMEDIA_HC,
	MMC_CARD_SECURE_DIGITAL,
	MMC_CARD_SECURE_DIGITAL_IO,
	MMC_CARD_SECURE_DIGITAL_HC,
	MMC_CARD_SECURE_DIGITAL_IO_COMBO
};

#define UBOOT_FASTBOOT_MAGIC	0xFA57B007

/**
 * enum wakeup_mode - Here is the different wake-up mode definition
 *
 */
enum boot_mode {
	COLD_BOOT,
	STR_BOOT,
	STD_BOOT
};

enum std_part {
	STD_PART1,
	STD_PART2,
	STD_MAX_PART
};

/*
 * Shared data M3/AP:
 * Page aligned structure with 2 sub parts:
 *  - Boot informations part written only by M3 (M3 has a mirror in ESRAM)
 *  - Read/Write part for others M3/AP RW variables
 */

struct shared_data_bi_t {
	uint32_t otp_regs[8];
	uint8_t soc_id;
	uint8_t erom_version;
	uint8_t board_id;
	uint8_t board_rev_id;
	uint8_t board_extensions;
	uint8_t cut_rev;
	uint8_t boot_mode;
	struct {
		uint32_t base; /* Registers base address */
		uint8_t type;  /* STA_BOOT_DEV */
		uint8_t mmc_num; /* The mmc boot dev number */
		union {
			struct {
				uint16_t flash_id;
				uint16_t jdec_extid;
				uint8_t manuf_id;
			} sqi;
			struct {
				uint32_t ocr;
				uint16_t rca;
				uint8_t csd[16];
				uint8_t bus_width;
				uint8_t card_type; /* Value enum MMC_CARD_TYPE */
			} mmc;
		} dev;
	} boot_dev;
};

struct std_snapshot_info_t {
	uint32_t load;
	uint32_t size;
};

struct shared_data_rw_t {
	void *m3_pen;
	/*
	 * When value is UBOOT_FASTBOOT_MAGIC then start U-boot
	 * in fastboot mode for flash loading purpose
	 */
	uint32_t uboot_fastboot_mode;
	uint8_t sync_point;
	uint8_t display_cfg;
	struct std_snapshot_info_t std_snapshot_info[STD_MAX_PART];
};

struct shared_data_t {
	struct shared_data_bi_t bi;
	struct shared_data_rw_t rw;
} __attribute__ ((aligned (4096)));

#ifndef DECLARE_SHARED_DATA
#define DECLARE_SHARED_DATA extern
#endif
DECLARE_SHARED_DATA struct shared_data_t
	__attribute__((section(".shared_data"))) shared_data;

#ifdef __UBOOT__

/*
 * In U-boot we can't use directly shared_data variable due to relocation
 * We have to pass through gd global data pointer
 */
#include <asm/u-boot.h>		/* boot information for Linux kernel */
#include <asm/global_data.h>	/* global data used for startup functions */
DECLARE_GLOBAL_DATA_PTR;

#ifndef _SHARED_DATA
#define _SHARED_DATA		&(gd->arch.p_shared_data->bi)
#define _SHARED_DATA_RW		&(gd->arch.p_shared_data->rw)
#endif

#else

#ifndef _SHARED_DATA
/*
 * By default, use directly shared_data variable
 * _SHARED_DATA can be defined by specific platform code in order to
 * get shared_data base address according to MMU configuration.
 */
#define _SHARED_DATA		&shared_data.bi
#define _SHARED_DATA_RW		&shared_data.rw
#endif

#endif /* __UBOOT__ */

/*
 * API to access to boot information part written only by M3
 */
static inline uint32_t *get_otp_regs(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->otp_regs;
}

static inline uint8_t get_soc_id(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->soc_id;
}

static inline uint8_t get_cut_rev(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->cut_rev;
}

static inline uint8_t get_board_id(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->board_id;
}

static inline uint8_t get_board_rev_id(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->board_rev_id;
}

static inline uint8_t get_board_extensions(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->board_extensions;
}

static inline uint8_t get_erom_version(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->erom_version;
}

static inline uint8_t get_display_cfg(void)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	return base->display_cfg;
}

static inline void set_display_cfg(uint8_t cfg)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	base->display_cfg = cfg;
}

static inline uint8_t get_boot_dev_type(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.type;
}

static inline void set_boot_dev_type(uint8_t type)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.type = type;
}

static inline uint8_t get_mmc_boot_dev(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.mmc_num;
}

static inline void set_mmc_boot_dev(uint8_t mmc_num)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.mmc_num = mmc_num;
}

static inline uint32_t get_boot_dev_base(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.base;
}

static inline void set_boot_dev_base(uint32_t addr)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.base = addr;
}

static inline uint8_t get_sqi_manu_id(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.sqi.manuf_id;
}

static inline void set_sqi_manu_id(uint8_t manu_id)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.dev.sqi.manuf_id = manu_id;
}

static inline uint16_t get_sqi_flash_id(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.sqi.flash_id;
}

static inline void set_sqi_flash_id(uint16_t flash_id)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.dev.sqi.flash_id = flash_id;
}

static inline uint16_t get_sqi_jdec_extid(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.sqi.jdec_extid;
}

static inline void set_sqi_jdec_extid(uint16_t jdec_extid)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.dev.sqi.jdec_extid = jdec_extid;
}

static inline uint8_t get_mmc_bus_width(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.mmc.bus_width;
}

static inline void set_mmc_bus_width(uint8_t bus_width)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.dev.mmc.bus_width = bus_width;
}

static inline uint8_t get_mmc_card_type(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.mmc.card_type;
}

static inline void set_mmc_card_type(uint8_t card_type)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.dev.mmc.card_type = card_type;
}

static inline uint8_t *get_mmc_csd(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.mmc.csd;
}

static inline void set_mmc_csd(uint8_t *csd)
{
	struct shared_data_bi_t *base = _SHARED_DATA;
	uint32_t i;

	for (i = 0; i < sizeof(base->boot_dev.dev.mmc.csd); i++)
		base->boot_dev.dev.mmc.csd[i] = csd[i];
}

static inline uint32_t get_mmc_ocr(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.mmc.ocr;
}

static inline void set_mmc_ocr(uint32_t ocr)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.dev.mmc.ocr = ocr;
}

static inline uint16_t get_mmc_rca(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_dev.dev.mmc.rca;
}

static inline void set_mmc_rca(uint16_t rca)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_dev.dev.mmc.rca = rca;
}

static inline uint8_t get_boot_mode(void)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	return base->boot_mode;
}

static inline void set_boot_mode(uint8_t boot_mode)
{
	struct shared_data_bi_t *base = _SHARED_DATA;

	base->boot_mode = boot_mode;
}

/*
 * API to access to shared M3/AP RW variables
 */
static inline void *get_m3_pen(void)
{
	volatile struct shared_data_rw_t *base = _SHARED_DATA_RW;

	return base->m3_pen;
}

static inline void set_m3_pen(void *pen)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	base->m3_pen = pen;
}

static inline uint8_t get_sync_point(void)
{
	volatile struct shared_data_rw_t *base = _SHARED_DATA_RW;

	return base->sync_point;
}

static inline void set_sync_point(uint8_t sync_point)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	base->sync_point = sync_point;
}

static inline uint8_t is_uboot_fastboot_mode(void)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	return (base->uboot_fastboot_mode == UBOOT_FASTBOOT_MAGIC);
}

static inline void set_uboot_fastboot_mode(void)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	base->uboot_fastboot_mode = UBOOT_FASTBOOT_MAGIC;
}

static inline void set_std_info(uint32_t std_part, uint32_t addr,
				uint32_t size)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	if (std_part < STD_MAX_PART) {
		base->std_snapshot_info[std_part].load = addr;
		base->std_snapshot_info[std_part].size = size;
	}
}

static inline void get_std_info(uint32_t std_part, uint32_t *addr,
				uint32_t *size)
{
	struct shared_data_rw_t *base = _SHARED_DATA_RW;

	if (std_part < STD_MAX_PART) {
		if (addr)
			*addr = base->std_snapshot_info[std_part].load;

		if (size)
			*size = base->std_snapshot_info[std_part].size;
	}
}

#endif /* __ASSEMBLY__ && ASM */

#endif /* __STA_SHARED_DATA_H__ */
